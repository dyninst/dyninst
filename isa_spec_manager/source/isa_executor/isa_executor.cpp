/*
 * Copyright (c) 2021-2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#include "isa_executor.h"

// C++ libraries.
#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <unordered_map>
#include <set>
#include <sstream>

// Local libraries.
#include "amdisa_utility.h"
#include "amdisa/isa_decoder.h"
#include "inst_execution.hpp"
#include "state_delta.h"
#include "wave_state.h"

namespace amdisa
{
    // IsaExecutor struct implementation.
    struct IsaExecutor::IsaExecutorImpl
    {
        // Wave state.
        std::shared_ptr<WaveState> wave_state_ptr;

        // Execution model of instructions.
        ChangeRecipeFactory recipe_factory;

        // Deltas.
        std::vector<StateDelta> history;
        bool should_track_deltas = false;
    };

    // Error string constants.
    static const char* kStringErrorNoExecutionModel        = "Error: Execution model for the instruction not found.";
    static const char* kStringErrorExecutionTypeNotDefined = "Error: Execution type not defined.";
    static const char* kStringErrorDeltaTrackingOff        = "Error: State delta tracking is off.";
    static const char* kStringErrorDeltaNotAvailable       = "Error: State delta not available. Try executing instruction first.";
    static const char* kStringErrorExecutorInitialized     = "Error: Executor has already been initialized.";
    static const char* kStringErrorIncorrectLiteral        = "Error: Input string format is incorrect";
    static const char* kStringErrorCannotConvertLiteral    = "Error: Can't convert literal. ";
    static const char* kStringErrorInvalidJsonFormat       = "Error: Invalid JSON format. Should be wrapped in {}.";
    static const char* kStringErrorInvalidJsonKey          = "Error: Can't recognize JSON key: ";
    static const char* kStringErrorInvalidJsonValue        = "Error: Can't convert JSON value: ";

    // String constants.
    static const char* kStringPcRegisterName   = "PC";
    static const char* kStringSccRegisterName  = "SCC";
    static const char* kStringVccRegisterName  = "VCC";
    static const char* kStringExecRegisterName = "EXEC";

    // Set of known operand keys.
    const std::set<std::string> kKnownKeys = { "VALUE", "TARGET" };

    // Integer constants.
    static const uint64_t kLo32Mask = 0x00000000ffffffff;

    // Statically linked auxiliary functions.
    namespace aux
    {
        static std::vector<uint32_t> BreakDown64BitInt(const uint64_t value)
        {
            std::vector<uint32_t> result;
            result.push_back(static_cast<uint32_t>(value & kLo32Mask));
            if (value > kLo32Mask)
            {
                result.push_back(static_cast<uint32_t>(value >> 32));
            }
            return result;
        }

        static bool parse_json_operand(const std::string& input, std::map<std::string, uint64_t>& json_operands, std::string& err_message)
        {
            bool should_abort = false;

            // Remove the curly braces
            std::string key_value_pairs;
            if (input.front() == '{' && input.back() == '}')
            {
                key_value_pairs = input.substr(1, input.size() - 2);
            }
            else
            {
                should_abort = true;
                err_message = kStringErrorInvalidJsonFormat;
            }

            // Remove all spaces.
            key_value_pairs.erase(std::remove(key_value_pairs.begin(), key_value_pairs.end(), ' '), key_value_pairs.end());

            // Parse the key-value pairs.
            std::string key, value_str;
            std::stringstream ss_outer;
            ss_outer.str(key_value_pairs);
            std::string segment;
            while (std::getline(ss_outer, segment, ';') && !should_abort)
            {
                std::replace(segment.begin(), segment.end(), ':', ' ');
                std::stringstream ss_inner;
                ss_inner.str(segment);
                ss_inner >> key;
                if (kKnownKeys.count(key) == 0)
                {
                    should_abort = true;
                    std::stringstream err_ss;
                    err_ss << kStringErrorInvalidJsonKey << key;
                    err_message = err_ss.str();
                }

                if (!should_abort)
                {
                    ss_inner >> value_str;
                    try
                    {
                        uint64_t value = std::stoull(value_str);
                        json_operands[key] = value;
                    }
                    catch (const std::exception& e)
                    {
                        should_abort = true;
                        std::stringstream err_ss;
                        err_ss << kStringErrorInvalidJsonValue << " " << value_str << " " << e.what();
                        err_message = err_ss.str();
                    }
                }
            }

            return !should_abort;
        }

        static bool LiteralStringToVector(const std::string& literal_string, std::vector<uint32_t>& value, std::string& err_message)
        {
            bool should_abort = false;

            // Check for the format "lit(0x...)".
            if (literal_string.substr(0, 4) != "lit(" || literal_string.back() != ')')
            {
                should_abort = true;
                err_message = kStringErrorIncorrectLiteral;
            }
            else
            {
                // Extract the hex part from the string.
                // Remove "lit(" at the start and ")" at the end.
                std::string hex_part = literal_string.substr(4, literal_string.size() - 5);

                // Convert hex string to a single integer.
                try
                {
                    uint64_t hex_value = std::stoull(hex_part, nullptr, 16);
                    value = BreakDown64BitInt(hex_value);
                }
                catch (const std::exception& e)
                {
                    should_abort = true;
                    std::stringstream err_ss;
                    err_ss << kStringErrorCannotConvertLiteral << e.what();
                    err_message = err_ss.str();
                }
            }

            return !should_abort;
        }

        static void RecordOldSideEffectValues(const WaveState& wave_state, std::shared_ptr<IStateChangeRecipe> r, StateDelta& delta)
        {
            if (r->UpdatesPc())
            {
                RegisterDelta reg_delta;
                uint32_t pc_lo = wave_state.PC() & kLo32Mask;
                uint32_t pc_hi = (wave_state.PC() >> 32) & kLo32Mask;
                reg_delta.old_value.push_back(pc_hi);
                reg_delta.old_value.push_back(pc_lo);
                delta.deltas[kStringPcRegisterName] = reg_delta;
            }
            if (r->UpdatesScc())
            {
                RegisterDelta reg_delta;
                reg_delta.old_value.push_back(static_cast<uint32_t>(wave_state.SCC()));
                delta.deltas[kStringSccRegisterName] = reg_delta;
            }
        }

        static void RecordNewSideEffectValues(const WaveState& wave_state, std::shared_ptr<IStateChangeRecipe> r, StateDelta& delta)
        {
            if (r->UpdatesPc())
            {
                auto reg_delta_it = delta.deltas.find(kStringPcRegisterName);
                assert(reg_delta_it != delta.deltas.end());
                uint32_t pc_lo = wave_state.PC() & kLo32Mask;
                uint32_t pc_hi = (wave_state.PC() >> 32) & kLo32Mask;
                reg_delta_it->second.new_value.push_back(pc_hi);
                reg_delta_it->second.new_value.push_back(pc_lo);
            }
            if (r->UpdatesScc())
            {
                auto reg_delta_it = delta.deltas.find(kStringSccRegisterName);
                assert(reg_delta_it != delta.deltas.end());
                reg_delta_it->second.new_value.push_back(static_cast<uint32_t>(wave_state.SCC()));
            }
        }

        static bool ExecuteScalar(std::shared_ptr<IStateChangeRecipe> r, const InstructionInfo& inst_info,
            WaveState& wave_state, std::vector<StateDelta>& history, std::string& err_message, bool should_track_deltas)
        {
            bool is_executed = false;

            // Set source operands.
            bool should_abort = false;
            uint32_t src_num = 0;
            auto operand_it = inst_info.instruction_operands.begin();
            while (!should_abort && operand_it != inst_info.instruction_operands.end())
            {
                const auto& operand = *operand_it;
                if (operand.is_input)
                {
                    // Get the value of the operand from the wave state.
                    bool is_retrieved = false;
                    std::vector<uint32_t> src_value;

                    if (operand.operand_name.find("lit(") != std::string::npos)
                    {
                        is_retrieved = LiteralStringToVector(operand.operand_name, src_value, err_message);
                    }
                    else if (operand.operand_name[0] == '{')
                    {
                        std::map<std::string, uint64_t> json_operands;
                        is_retrieved = parse_json_operand(operand.operand_name, json_operands, err_message);
                        assert(is_retrieved);

                        if (json_operands.size() == 1)
                        {
                            auto json_operands_iter = json_operands.begin();
                            src_value = BreakDown64BitInt(json_operands_iter->second);
                        }
                        else
                        {
                            assert(false);
                        }
                    }
                    else
                    {
                        wave_state.GetScalarRegisterValue(operand.operand_name, src_value);
                        is_retrieved = true;
                    }

                    // Set the retrieved value to the execution model of this instruction.
                    bool is_set = false;
                    if (is_retrieved)
                    {
                        is_set = r->SetSource(src_num, src_value);
                    }
                    src_num++;

                    should_abort = !is_retrieved || !is_set;
                }
                operand_it++;
            }
            assert(!should_abort);

            // Compute change.
            StateDelta delta;
            if (should_track_deltas && r->HasSideEffects())
            {
                RecordOldSideEffectValues(wave_state, r, delta);
                r->ComputeChange(wave_state);
                RecordNewSideEffectValues(wave_state, r, delta);
            }
            else
            {
                r->ComputeChange(wave_state);
            }

            // Get destination operands.
            uint32_t dst_num = 0;
            for (const auto& operand : inst_info.instruction_operands)
            {
                if (operand.is_output)
                {
                    // Get the computed destination value.
                    std::vector<uint32_t> dst_value;
                    bool is_retrieved = r->GetDestination(dst_num, dst_value);
                    assert(is_retrieved);

                    // Set the computed value to the wave state.
                    wave_state.SetScalarRegisterValue(operand.operand_name, dst_value, delta, should_track_deltas);
                }
            }

            if (should_track_deltas)
            {
                history.push_back(delta);
            }
            return true;
        }
    }

    bool IsaExecutor::Init(WaveState& wave_state, std::string& err_message, bool should_track_deltas)
    {
        bool ret = false;
        if (pimpl_ == nullptr)
        {
            pimpl_ = std::make_shared<IsaExecutorImpl>();
            pimpl_->recipe_factory.Init();
            pimpl_->wave_state_ptr = std::make_shared<WaveState>(wave_state);
            pimpl_->should_track_deltas = should_track_deltas;
            ret = true;
        }
        else
        {
            err_message = kStringErrorExecutorInitialized;
        }
        return ret;
    }

    bool IsaExecutor::GetLastDelta(StateDelta& delta, std::string& err_message)
    {
        bool is_retrieved = false;
        if (pimpl_->should_track_deltas)
        {
            if (pimpl_->history.size() > 0)
            {
                delta = pimpl_->history.back();
                is_retrieved = true;
            }
            else
            {
                is_retrieved = false;
                err_message = kStringErrorDeltaNotAvailable;
            }
        }
        else
        {
            is_retrieved = false;
            err_message = kStringErrorDeltaTrackingOff;
        }
        return is_retrieved;
    }

    bool IsaExecutor::Execute(const InstructionInfo& inst_info, std::string& err_message)
    {
        bool is_executed = false;

        // Get the recipe for the instruction.
        auto r = pimpl_->recipe_factory.GetModel(AmdIsaUtility::ToLower(inst_info.instruction_name));

        if (r != nullptr)
        {
            switch (r->GetExecutionType())
            {
            case ExecutionType::kBranch:
            case ExecutionType::kScalar:
                is_executed = aux::ExecuteScalar(r, inst_info, *pimpl_->wave_state_ptr, pimpl_->history, err_message,
                    pimpl_->should_track_deltas);
                break;
            default:
                err_message = kStringErrorExecutionTypeNotDefined;
                break;
            }
        }
        else
        {
            std::stringstream err_ss;
            err_ss << kStringErrorNoExecutionModel;
            err_ss << " Instruction: " << inst_info.instruction_name << ".";
            err_message = err_ss.str();
        }

        return is_executed;
    }

    bool IsaExecutor::Execute(const uint64_t instruction, const IsaDecoder& decoder, StateDelta& last_delta, std::string& err_message)
    {
        bool should_abort = false;

        // This method should work only when delta tracking is on.
        if (this->pimpl_->should_track_deltas)
        {
            amdisa::InstructionInfoBundle inst_info_bundle;
            bool is_decoded = decoder.DecodeInstruction(instruction, inst_info_bundle, err_message);
            if (is_decoded)
            {
                auto inst_info_iter = inst_info_bundle.bundle.begin();
                while (!should_abort && inst_info_iter != inst_info_bundle.bundle.end())
                {
                    const auto& inst_info = *inst_info_iter;
                    inst_info_iter++;

                    bool is_executed = this->Execute(inst_info, err_message);
                    if (is_executed)
                    {
                        bool is_retrieved = this->GetLastDelta(last_delta, err_message);
                        if (!is_retrieved)
                        {
                            should_abort = true;
                        }
                    }
                    else
                    {
                        should_abort = true;
                    }
                }
            }
            else
            {
                should_abort = true;
            }
        }
        else
        {
            should_abort = true;
            err_message = kStringErrorDeltaTrackingOff;
        }

        return !should_abort;
    }

    void IsaExecutor::SetWaveState(WaveState& wave_state)
    {
        pimpl_->wave_state_ptr = std::make_shared<WaveState>(wave_state);
    }
}
