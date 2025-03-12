/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#ifndef CLI_COMMAND_GET_INST_INFO_H_
#define CLI_COMMAND_GET_INST_INFO_H_

// C++ libraries.
#include <string>

// Local libraries.
#include "cli_command.h"

namespace amdisa
{
    // Forward declarations.
    class IsaDecoder;

    /**
     * Class: CliCommandGetInstInfo
     *
     * Purpose:
     * This class is responsible for handling the CLI (Command Line Interface) command
     * that generates a detailed description or information about a given instruction.
     * Upon receiving an instruction, the class provides a comprehensive breakdown or
     * overview of what the instruction does, its operands, semantics, and other relevant details.
     */
    class CliCommandGetInstInfo : public ICliCommand
    {
    public:
        CliCommandGetInstInfo(const std::string& instruction_name, const IsaDecoder& spec_api)
            : instruction_name_(instruction_name)
            , spec_api_(spec_api)
        {
        }
        CliCommandGetInstInfo() = delete;
        bool Execute(std::string& err_message) override;

    private:
        const std::string instruction_name_;
        const IsaDecoder& spec_api_;
    };
}  // namespace amdisa
#endif  // CLI_COMMAND_GET_INST_INFO_H_
