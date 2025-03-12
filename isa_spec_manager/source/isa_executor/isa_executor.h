/*
 * Copyright (c) 2021-2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#ifndef ISA_EXECUTOR_H_
#define ISA_EXECUTOR_H_

// C++ libraries.
#include <memory>
#include <string>
#include <vector>

namespace amdisa
{
    // Forward declarations.
    struct InstructionInfo;
    struct StateDelta;
    class IsaDecoder;
    class WaveState;

    class IsaExecutor
    {
    public:
        bool Init(WaveState& wave_state, std::string& err_message, bool should_track_deltas = false);
        bool GetLastDelta(StateDelta& delta, std::string& err_message);
        bool Execute(const InstructionInfo& inst_info, std::string& err_message);
        bool Execute(const uint64_t instruction, const IsaDecoder& decoder, StateDelta& delta, std::string& err_message);
        void SetWaveState(WaveState& wave_state);

        // Constructor and destructor control.
        IsaExecutor() = default;

    private:
        struct IsaExecutorImpl;
        std::shared_ptr<IsaExecutorImpl> pimpl_;
    };
}
#endif