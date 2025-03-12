/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#ifndef CLI_COMMAND_PRINT_HELP_H_
#define CLI_COMMAND_PRINT_HELP_H_

// C++ libraries.
#include <string>

// Local libraries.
#include "cli_command.h"

namespace amdisa
{
    /**
     * Class: CliCommandPrintHelp
     *
     * Purpose:
     * This class is responsible for handling the CLI (Command Line Interface) command
     * that displays help or usage information to the user. It provides an overview
     * of available commands, their descriptions, and proper syntax or format for invoking them.
     */
    class CliCommandPrintHelp : public ICliCommand
    {
    public:
        CliCommandPrintHelp() = delete;
        CliCommandPrintHelp(const std::string& help_message)
            : help_message_(help_message)
        {
        }
        bool Execute(std::string& err_message) override;

    private:
        std::string help_message_;
    };
}  // namespace amdisa
#endif  // CLI_COMMAND_PRINT_HELP_H_
