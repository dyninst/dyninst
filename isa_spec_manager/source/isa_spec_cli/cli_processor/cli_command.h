/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#ifndef I_CLI_COMMAND_H_
#define I_CLI_COMMAND_H_

// C++ libraries.
#include <string>

namespace amdisa
{
    // Interface class for commands.
    struct ICliCommand
    {
        ICliCommand() = default;
        virtual ~ICliCommand(){};
        virtual bool Execute(std::string& err_message) = 0;
    };
}  // namespace amdisa
#endif  // I_CLI_COMMAND_H_
