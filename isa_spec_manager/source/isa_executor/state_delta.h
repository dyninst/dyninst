/*
 * Copyright (c) 2021-2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#ifndef STATE_DELTA_H_
#define STATE_DELTA_H_

// C++ libraries.
#include <string>
#include <unordered_map>

namespace amdisa
{
    // Defines the delta for a single register.
    struct RegisterDelta
    {
        std::vector<uint32_t> old_value;
        std::vector<uint32_t> new_value;
    };

    // Defines the delta for all registers for a single instruction.
    struct StateDelta
    {
        std::unordered_map<std::string, RegisterDelta> deltas;
    };
}
#endif // STATE_DELTA_H_