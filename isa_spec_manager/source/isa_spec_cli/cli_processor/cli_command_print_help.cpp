/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#include "cli_command_print_help.h"

// C++ libraries.
#include <iostream>
#include <string>

namespace amdisa
{
    // Error constant strings.
    static const char* kStringErrorCommandNotConfigured = "Error: Print help command not configured.";

    bool CliCommandPrintHelp::Execute(std::string& err_message)
    {
        bool is_executed = false;
        if (!help_message_.empty())
        {
            std::cout << help_message_ << std::endl;
            is_executed = true;
        }
        else
        {
            std::cerr << kStringErrorCommandNotConfigured << std::endl;
        }

        return is_executed;
    }
}  // namespace amdisa
