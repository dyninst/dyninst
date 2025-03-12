/*
 * Copyright (c) 2021-2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>

#include "amdisa/isa_decoder.h"
#include "isa_executor.h"
#include "wave_state.h"
#include "state_delta.h"

static const char*    kXmlPath           = "../../../xmls/mi200.xml";
static const uint32_t kWaveLength        = 32;
static const uint32_t kLoHalfMask        = 0x00000000ffffffff;
static const bool     kShouldTrackDeltas = true;

namespace aux
{
    static const char* kStringErrorDeltaSizeMismatch = "Error: Provided delta size are different";
    static const char* kStringErrorDeltaNotFound = "Error: Delta A has a register that is not present in Delta B.";
    static const char* kStringErrorDeltaNewValueMismatch = "Error: New values don't match.";

    static void PrintStateDelta(const amdisa::StateDelta delta)
    {

        for (const auto& reg_delta : delta.deltas)
        {
            std::cout << "Register changed: " << reg_delta.first << std::endl;
            std::cout << "  Old val: ";
            for (uint32_t chunk : reg_delta.second.old_value)
            {
                std::cout << chunk << " ";
            }
            std::cout << std::endl;

            std::cout << "  New val: ";
            for (uint32_t chunk : reg_delta.second.new_value)
            {
                std::cout << chunk << " ";
            }
            std::cout << std::endl;
        }
    }

    static bool IsDeltaEqual(const amdisa::StateDelta& delta_a, const amdisa::StateDelta& delta_b, std::string& err_message)
    {
        if (delta_a.deltas.size() == delta_b.deltas.size())
        {
            for (const auto& delta_info : delta_a.deltas)
            {
                if (delta_b.deltas.find(delta_info.first) != delta_b.deltas.end())
                {
                    if (delta_info.second.new_value.size() == delta_b.deltas.at(delta_info.first).new_value.size())
                    {
                        for (size_t i = 0; i < delta_info.second.new_value.size(); i++)
                        {
                            if (delta_info.second.new_value[i] != delta_b.deltas.at(delta_info.first).new_value[i])
                            {
                                err_message = kStringErrorDeltaNewValueMismatch;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        err_message = kStringErrorDeltaNewValueMismatch;
                        return false;
                    }
                }
                else
                {
                    err_message = kStringErrorDeltaNotFound;
                    return false;
                }
            }
        }
        else
        {
            err_message = kStringErrorDeltaSizeMismatch;
            return false;
        }

        return true;
    }

    // Forms S_MOV_B32 instruction to set the scalar register to a given value.
    static void SetScalarRegister(const uint64_t reg_idx, const uint64_t reg_value,
        const amdisa::IsaDecoder& decoder, amdisa::IsaExecutor& executor)
    {
        assert(reg_idx < 127);

        // Form move instruction.
        uint64_t s_mov_inst = 0;
        if (reg_value < 0xffff)
        {
            // S_MOVK_I32.
            s_mov_inst = 0xb0000000 | reg_value | (reg_idx << 16);
        }
        else
        {
            // S_MOV_B32 with literal.
            s_mov_inst = 0xbe8000ffLL | (reg_value << 32UL) | (reg_idx << 16);
        }

        // Execute the instruction to set the register.
        std::string err_message;
        amdisa::StateDelta delta;
        bool is_executed = executor.Execute(s_mov_inst, decoder, delta, err_message);
        assert(is_executed);
    }

    // Set PC using S_MOV_B32 and S_SETPC_B64. The function uses register s0 and s1.
    // So they will be overwritten.
    static void SetPC(const uint64_t pc_value, const amdisa::IsaDecoder& decoder,
        amdisa::IsaExecutor& executor)
    {
        SetScalarRegister(0, pc_value & kLoHalfMask, decoder, executor);
        SetScalarRegister(1, (pc_value >> 32) & kLoHalfMask, decoder, executor);

        // Execute the instruction to set the register.
        const uint64_t kSetPcB64 = 0xbe801d00;
        std::string err_message;
        amdisa::StateDelta delta;
        bool is_executed = executor.Execute(kSetPcB64, decoder, delta, err_message);
        assert(is_executed);
    }

    static bool TestInstructionExecution(uint64_t instruction, const amdisa::StateDelta& expected_delta,
        amdisa::IsaDecoder& decoder, amdisa::IsaExecutor& executor, std::string& decode_err_message,
        std::string& execute_err_message)
    {
        std::cerr << "HELLO" << std::endl;
        amdisa::InstructionInfoBundle inst_info_bundle;
        bool is_decoded = decoder.DecodeInstruction(instruction, inst_info_bundle, decode_err_message);
        if (is_decoded)
        {
            for (const auto& inst_info : inst_info_bundle.bundle)
            {
                std::cerr << inst_info.encoding_layout << std::endl;
                bool is_executed = executor.Execute(inst_info, execute_err_message);
                if (is_executed)
                {
                    amdisa::StateDelta delta;
                    bool is_retrieved = executor.GetLastDelta(delta, execute_err_message);
                    if (is_retrieved)
                    {
                        PrintStateDelta(delta);
                        return IsDeltaEqual(expected_delta, delta, execute_err_message);
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
        }
        return false;
    }
}

int main()
{
    std::string decoder_err_message;
    std::string execute_err_message;
    amdisa::IsaDecoder decoder;
    amdisa::IsaExecutor executor;
    amdisa::WaveState wave_state;
    wave_state.Init(kWaveLength);

    bool is_decoder_init  = decoder.Initialize(kXmlPath, decoder_err_message);
    bool is_executor_init = executor.Init(wave_state, execute_err_message, kShouldTrackDeltas);

    // Initialize registers.
    //initial_state_ss << "s[10:13]-7777 1111 2222 3333";
    if (is_decoder_init && is_executor_init)
    {
        // Instruction to execute: s_mov_b32 s0, lit(0x12341234)
        {
            const uint64_t kInstruction = 0x12341234be8000ff;

            amdisa::StateDelta expected_delta;
            expected_delta.deltas["s0"].new_value.push_back(0x12341234);
            assert(aux::TestInstructionExecution(kInstruction, expected_delta, decoder, executor, decoder_err_message, execute_err_message));
        }

        // Instruction to execute: s_add_u32 s96, s96, s7
        {
            const uint64_t kInstruction = 0x80600760;

            // Initialize registers that will be involved in the execution.
            aux::SetScalarRegister(7, 21, decoder, executor);
            aux::SetScalarRegister(96, 12, decoder, executor);

            amdisa::StateDelta expected_delta;
            expected_delta.deltas["SCC"].new_value.push_back(0);
            expected_delta.deltas["s96"].new_value.push_back(33);

            assert(aux::TestInstructionExecution(kInstruction, expected_delta, decoder, executor, decoder_err_message, execute_err_message));
        }

        // Instruction to execute: s_add_u32 s96, s96, s7
        // Initialize operands with large numbers to test overflow in SCC.
        {
            const uint64_t kInstruction = 0x80600760;

            // Initialize registers that will be involved in the execution.
            aux::SetScalarRegister(7, 4294967295, decoder, executor);
            aux::SetScalarRegister(96, 4294967295, decoder, executor);

            amdisa::StateDelta expected_delta;
            expected_delta.deltas["SCC"].new_value.push_back(1);
            expected_delta.deltas["s96"].new_value.push_back(4294967294);

            assert(aux::TestInstructionExecution(kInstruction, expected_delta, decoder, executor, decoder_err_message, execute_err_message));
        }

        // Instruction to execute: s_addc_u32 s96, s96, s7
        // Test carry in. SCC value from previous instruction should be added to computation.
        {
            const uint64_t kInstruction = 0x82600760;

            // Initialize registers that will be involved in the execution.
            aux::SetScalarRegister(7, 0, decoder, executor);

            amdisa::StateDelta expected_delta;
            expected_delta.deltas["SCC"].new_value.push_back(0);
            expected_delta.deltas["s96"].new_value.push_back(4294967295);

            assert(aux::TestInstructionExecution(kInstruction, expected_delta, decoder, executor, decoder_err_message, execute_err_message));
        }

        // Instruction to execute: s_addc_u32 s96, s96, s96
        // Test carry out. SCC should record overflow.
        {
            const uint64_t kInstruction = 0x82606060;

            amdisa::StateDelta expected_delta;
            expected_delta.deltas["SCC"].new_value.push_back(1);
            expected_delta.deltas["s96"].new_value.push_back(4294967294);

            assert(aux::TestInstructionExecution(kInstruction, expected_delta, decoder, executor, decoder_err_message, execute_err_message));
        }

        // Instruction to execute: s_addc_u32 s96, s96, s96
        // Test carry in and out. Should SCC and record SCC should record overflow.
        {
            const uint64_t kInstruction = 0x82606060;

            // Initialize registers that will be involved in the execution.
            aux::SetScalarRegister(96, 4294967295, decoder, executor);

            amdisa::StateDelta expected_delta;
            expected_delta.deltas["SCC"].new_value.push_back(1);
            expected_delta.deltas["s96"].new_value.push_back(4294967295);

            assert(aux::TestInstructionExecution(kInstruction, expected_delta, decoder, executor, decoder_err_message, execute_err_message));
        }

        // Instruction to execute: s_setpc_b64 s[12:13]
        // Set PC value to what in the scalar register.
        {
            const uint64_t kInstruction = 0xBE801D0C;

            // Initialize registers that will be involved in the execution.
            aux::SetScalarRegister(12, 2222, decoder, executor);
            aux::SetScalarRegister(13, 3333, decoder, executor);

            amdisa::StateDelta expected_delta;
            expected_delta.deltas["PC"].new_value.push_back(2222);
            expected_delta.deltas["PC"].new_value.push_back(3333);

            assert(aux::TestInstructionExecution(kInstruction, expected_delta, decoder, executor, decoder_err_message, execute_err_message));
        }

        // Instruction to execute: s_swappc_b64 s[12:13], s[10:11]
        // Destination operand s[12:13] get set to PC+4.
        // PC gets set value to the register s[10:11].
        {
            const uint64_t kInstruction = 0xBE8C1E0A;

            // Initialize registers that will be involved in the execution.
            aux::SetScalarRegister(10, 7777, decoder, executor);
            aux::SetScalarRegister(11, 1111, decoder, executor);

            amdisa::StateDelta expected_delta;
            expected_delta.deltas["PC"].new_value.push_back(7777);
            expected_delta.deltas["PC"].new_value.push_back(1111);
            expected_delta.deltas["s[12:13]"].new_value.push_back(2222);
            expected_delta.deltas["s[12:13]"].new_value.push_back(3337);

            assert(aux::TestInstructionExecution(kInstruction, expected_delta, decoder, executor, decoder_err_message, execute_err_message));
        }

        // Instruction to execute: s_getpc_b64 s[0:1]
        // Save PC+4 value to s[0:1].
        {
            const uint64_t kInstruction = 0xBE801C00;

            amdisa::StateDelta expected_delta;
            expected_delta.deltas["s[0:1]"].new_value.push_back(7777);
            expected_delta.deltas["s[0:1]"].new_value.push_back(1115);
            assert(aux::TestInstructionExecution(kInstruction, expected_delta, decoder, executor, decoder_err_message, execute_err_message));
        }

        // Instruction to execute: s_addc_u32 s96, s96, 96
        // Test literal input 96.
        {
            const uint64_t kInstruction = 0x000000608260ff60;

            amdisa::StateDelta expected_delta;
            expected_delta.deltas["s96"].new_value.push_back(96);
            expected_delta.deltas["SCC"].new_value.push_back(1);
            assert(aux::TestInstructionExecution(kInstruction, expected_delta, decoder, executor, decoder_err_message, execute_err_message));
        }

        // Instruction to execute: s_addk_i32 s12, 0x0002
        {
            const uint64_t kInstruction = 0xb70c0002;

            amdisa::StateDelta expected_delta;
            expected_delta.deltas["s12"].new_value.push_back(2224);
            expected_delta.deltas["SCC"].new_value.push_back(0);
            assert(aux::TestInstructionExecution(kInstruction, expected_delta, decoder, executor, decoder_err_message, execute_err_message));
        }

        // Instruction to execute: s_addk_i32 s12, 0xFFFF
        // Signed -1
        {
            const uint64_t kInstruction = 0xb70cffff;

            amdisa::StateDelta expected_delta;
            expected_delta.deltas["s12"].new_value.push_back(2223);
            expected_delta.deltas["SCC"].new_value.push_back(0);
            assert(aux::TestInstructionExecution(kInstruction, expected_delta, decoder, executor, decoder_err_message, execute_err_message));
        }

        // Instruction to execute: s_addk_i32 s12, 0x0001
        // Test signed overflow.
        {
            const uint64_t kInstruction = 0xb70c0001;

            // Initialize registers that will be involved in the execution.
            aux::SetScalarRegister(12, 2147483647, decoder, executor);

            amdisa::StateDelta expected_delta;
            expected_delta.deltas["s12"].new_value.push_back(0x80000000);
            expected_delta.deltas["SCC"].new_value.push_back(1);
            assert(aux::TestInstructionExecution(kInstruction, expected_delta, decoder, executor, decoder_err_message, execute_err_message));
        }

        // Instruction to execute: s_sub_u32 s100, s77, s87
        {
            const uint64_t kInstruction = 0x80e4574d;

            // Initialize registers that will be involved in the execution.
            aux::SetScalarRegister(77, 100, decoder, executor);
            aux::SetScalarRegister(87, 40, decoder, executor);

            amdisa::StateDelta expected_delta;
            expected_delta.deltas["s100"].new_value.push_back(60);
            expected_delta.deltas["SCC"].new_value.push_back(0);
            assert(aux::TestInstructionExecution(kInstruction, expected_delta, decoder, executor, decoder_err_message, execute_err_message));
        }

        // Instruction to execute: s_sub_u32 s100, s77, s87
        // Same as previous, but reverse the values to test overflow.
        {
            const uint64_t kInstruction = 0x80e4574d;

            // Initialize registers that will be involved in the execution.
            aux::SetScalarRegister(77, 40, decoder, executor);
            aux::SetScalarRegister(87, 100, decoder, executor);

            amdisa::StateDelta expected_delta;
            expected_delta.deltas["s100"].new_value.push_back(0xffffffc4);
            expected_delta.deltas["SCC"].new_value.push_back(1);
            assert(aux::TestInstructionExecution(kInstruction, expected_delta, decoder, executor, decoder_err_message, execute_err_message));
        }

        // Instruction to execute: s_call_b64 s14, 0x14
        {
            const uint64_t kInstruction = 0xba8e0014;

            // Set PC to a known value first.
            const uint64_t kInitPc = 0;
            aux::SetPC(kInitPc, decoder, executor);

            amdisa::StateDelta expected_delta;
            expected_delta.deltas["s[14:15]"].new_value.push_back(0);
            expected_delta.deltas["s[14:15]"].new_value.push_back(kInitPc + 4);
            expected_delta.deltas["PC"].new_value.push_back(0);
            expected_delta.deltas["PC"].new_value.push_back(0x14 * 4 + 4);
            assert(aux::TestInstructionExecution(kInstruction, expected_delta, decoder, executor, decoder_err_message, execute_err_message));
        }

        // Instruction to execute: s_movk_i32 s17, 0xff
        {
            const uint64_t kInstruction = 0xb01100ff;

            amdisa::StateDelta expected_delta;
            expected_delta.deltas["s17"].new_value.push_back(0xff);
            assert(aux::TestInstructionExecution(kInstruction, expected_delta, decoder, executor, decoder_err_message, execute_err_message));
        }

        // Instruction to execute: s_movk_i32 s17, -1
        {
            const uint64_t kInstruction = 0xb011ffff;

            amdisa::StateDelta expected_delta;
            expected_delta.deltas["s17"].new_value.push_back(0xffffffff);
            assert(aux::TestInstructionExecution(kInstruction, expected_delta, decoder, executor, decoder_err_message, execute_err_message));
        }
    }
    return 0;
}
