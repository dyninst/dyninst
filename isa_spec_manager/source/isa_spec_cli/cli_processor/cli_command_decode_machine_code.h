/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#ifndef CLI_COMMAND_DECODE_MACHINE_CODE_H_
#define CLI_COMMAND_DECODE_MACHINE_CODE_H_

// C++ libraries.
#include <string>

// Local libraries.
#include "cli_command.h"

namespace amdisa
{
    // Forward declarations.
    class IsaDecoder;

    /**
     * Class: CliCommandDecodeMachineCode
     *
     * Purpose:
     * This class is responsible for handling the CLI (Command Line Interface) command
     * that decodes instructions given in binary or machine code format.
     *
     * The instruction is expected to be provided as a string, and upon invocation,
     * the class decodes the instruction and provides a human-readable representation
     * of the corresponding assembly or high-level command.
     */
    class CliCommandDecodeMachineCode : public ICliCommand
    {
    public:
        CliCommandDecodeMachineCode() = delete;
        CliCommandDecodeMachineCode(const std::string& machine_code, const IsaDecoder& spec_api)
            : machine_code_(machine_code)
            , spec_api_(spec_api)
        {
        }

        bool Execute(std::string& err_message) override;

    private:
        std::string       machine_code_;
        const IsaDecoder& spec_api_;
    };
}  // namespace amdisa
#endif  // CLI_COMMAND_DECODE_MACHINE_CODE_H_
