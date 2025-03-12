/*
 * Copyright (c) 2021-2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#ifndef WAVE_STATE_H_
#define WAVE_STATE_H_

// C++ libraries.
#include <memory>
#include <string>
#include <vector>

namespace amdisa
{
    // Forward declrations.
    struct StateDelta;

    class WaveState
    {
    public:
        void Init(uint32_t wave_length);
        void GetScalarRegisterValue(const std::string& operand_name, std::vector<uint32_t>& value) const;
        void SetScalarRegisterValue(const std::string& operand_name, const std::vector<uint32_t>& value,
            StateDelta& delta, bool should_track_delta);
        void SetScalarRegisterValue(const std::string& operand_name, const std::vector<uint32_t>& value);
        void GetVectorRegisterValue(const std::string& operand_name, const uint32_t lane_id, std::vector<uint32_t>& value) const;
        void SetVectorRegisterValue(const std::string& operand_name, const uint32_t lane_id, const std::vector<uint32_t>& value);
        uint32_t GetWaveLength() const;

        // State registers which can be accessed directly.
        bool& SCC() const;
        uint64_t& PC() const;

        // Constructor and destructor control.
        WaveState() = default;

    private:
        struct WaveStateImpl;
        std::shared_ptr<WaveStateImpl> pimpl_;
    };
}
#endif