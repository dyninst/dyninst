// Copyright (c) 2023 Advance Micro Devices. All rights reserved.
#include "wave_state.h"

// C++ libraries.
#include <array>
#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <unordered_map>

// Local libraries.
#include "generic.h"
#include "state_delta.h"

namespace amdisa
{
    // Constatnt error messages.
    static const char* kStringErrorUnknownRegister = "Error: Unsupported register.";
    static const char* kStringErrorConversionFailure = "Error: Failed to convert string to integer. ";

    // Auxiliary types and functions.
    namespace aux
    {
        struct RegInfo
        {
            bool is_scalar = false;
            bool is_vector = false;
            uint32_t reg_idx = 0;
            uint32_t reg_size = 0;
        };

        static bool BreakdownStringRegName(const std::string& reg_name, RegInfo& reg_info, std::string& err_message)
        {
            bool should_abort = false;

            // Determine if scalar or vector.
            if (reg_name[0] == 's')
            {
                reg_info.is_scalar = true;
            }
            else if (reg_name[0] == 'v')
            {
                reg_info.is_vector = true;
            }
            else
            {
                err_message = kStringErrorUnknownRegister;
                should_abort = true;
            }

            // Get register index.
            if (!should_abort)
            {
                std::string sub_reg_name;
                std::string sub_reg_name_up;
                // Get positions.
                size_t open_brac_pos = reg_name.find("[");
                size_t semi_pos = reg_name.find(":");
                size_t close_brac_pos = reg_name.find("]");

                // Register access with subscript, i.e. s[0:1], s[2:3], etc
                if (open_brac_pos != std::string::npos)
                {
                    sub_reg_name = reg_name.substr(2, semi_pos - open_brac_pos - 1);
                    sub_reg_name_up = reg_name.substr(reg_name.find(":") + 1, close_brac_pos - semi_pos - 1);
                }
                // Register access with no subscript, i.e. s0, s1, etc.
                else
                {
                    sub_reg_name = reg_name.substr(1);
                    sub_reg_name_up = reg_name.substr(1);
                }

                try
                {
                    reg_info.reg_idx = std::stol(sub_reg_name);
                    uint32_t reg_idx_up = std::stol(sub_reg_name_up);
                    reg_info.reg_size = reg_idx_up - reg_info.reg_idx + 1;
                }
                catch(const std::exception& e)
                {
                    std::stringstream err_ss;
                    err_ss << kStringErrorConversionFailure << e.what() << std::endl;
                    err_message = err_ss.str();
                    should_abort = true;
                }
            }

            return !should_abort;
        }
    }

    // WaveState struct implementation.
    using RegisterMap = std::unordered_map<uint32_t, uint32_t>;
    struct WaveState::WaveStateImpl
    {
        // Wave length.
        uint32_t wave_length = 0;

        // Registers.
        RegisterMap sgprs;
        std::unordered_map<uint32_t, RegisterMap> vgprs;

        // Scalar conditional code.
        bool scc = false;

        // Program Counter.
        uint64_t pc = 0;
    };

    void WaveState::Init(uint32_t wave_length)
    {
        if (pimpl_ == nullptr)
        {
            pimpl_ = std::make_shared<WaveStateImpl>();
            pimpl_->wave_length = wave_length;
        }
    }

    bool& WaveState::SCC() const
    {
        return pimpl_->scc;
    }

    uint64_t& WaveState::PC() const
    {
        return pimpl_->pc;
    }

    void WaveState::GetScalarRegisterValue(const std::string& operand_name, std::vector<uint32_t>& value) const
    {
        aux::RegInfo reg_info;
        std::string err_message;
        bool is_valid = aux::BreakdownStringRegName(operand_name, reg_info, err_message);

        if (is_valid && reg_info.is_scalar && !reg_info.is_vector)
        {
            value.clear();
            for (uint32_t i = 0; i < reg_info.reg_size; i++)
            {
                const uint32_t kRegNumToRead = reg_info.reg_idx + i;
                value.push_back(pimpl_->sgprs[kRegNumToRead]);
            }
        }
        else
        {
            std::cout << err_message << std::endl;
            assert(false);
        }
    }

    void WaveState::SetScalarRegisterValue(const std::string& operand_name, const std::vector<uint32_t>& value,
        StateDelta& delta, bool should_track_delta)
    {
        aux::RegInfo reg_info;
        std::string err_message;
        bool is_valid = aux::BreakdownStringRegName(operand_name, reg_info, err_message);

        if (is_valid && reg_info.is_scalar && !reg_info.is_vector)
        {
            assert(value.size() == reg_info.reg_size);

            // Record the value.
            uint32_t i = 0;
            RegisterDelta reg_delta;
            for (uint32_t chunk : value)
            {
                const uint32_t kRegNumToWrite = reg_info.reg_idx + i;
                if (should_track_delta)
                {
                    reg_delta.old_value.push_back(pimpl_->sgprs[kRegNumToWrite]);
                    pimpl_->sgprs[kRegNumToWrite] = chunk;
                    reg_delta.new_value.push_back(pimpl_->sgprs[kRegNumToWrite]);
                }
                else
                {
                    pimpl_->sgprs[kRegNumToWrite] = chunk;
                }
                ++i;
            }
            if (should_track_delta)
            {
                delta.deltas[operand_name] = reg_delta;
            }
        }
        else
        {
            std::cout << err_message << std::endl;
            assert(false);
        }
    }

    void WaveState::SetScalarRegisterValue(const std::string& operand_name, const std::vector<uint32_t>& value)
    {
        StateDelta delta;
        SetScalarRegisterValue(operand_name, value, delta, false);
    }

    void WaveState::GetVectorRegisterValue(const std::string& operand_name, const uint32_t lane_id, std::vector<uint32_t>& value) const
    {
        aux::RegInfo reg_info;
        std::string err_message;
        bool is_valid = aux::BreakdownStringRegName(operand_name, reg_info, err_message);

        if (is_valid && reg_info.is_vector && !reg_info.is_scalar)
        {
            value.clear();

            // Get lane registers.
            auto& vregs = pimpl_->vgprs[lane_id];

            // Get the relevant registers from the lane.
            for (uint32_t i = 0; i < reg_info.reg_size; i++)
            {
                const uint32_t kRegNumToRead = reg_info.reg_idx + i;
                value.push_back(vregs[kRegNumToRead]);
            }
        }
    }

    void WaveState::SetVectorRegisterValue(const std::string& operand_name, const uint32_t lane_id, const std::vector<uint32_t>& value)
    {
        aux::RegInfo reg_info;
        std::string err_message;
        bool is_valid = aux::BreakdownStringRegName(operand_name, reg_info, err_message);

        if (is_valid && reg_info.is_vector && !reg_info.is_scalar)
        {
            // Get lane registers.
            auto& vregs = pimpl_->vgprs[lane_id];

            assert(value.size() == reg_info.reg_size);
            uint32_t i = 0;
            for (const auto& chunk : value)
            {
                const uint32_t kRegNumToWrite = reg_info.reg_idx + i;
                vregs[kRegNumToWrite] = chunk;
                ++i;
            }
        }
    }

    uint32_t WaveState::GetWaveLength() const
    {
        return pimpl_->wave_length;
    }
}
