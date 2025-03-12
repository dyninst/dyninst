/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#include "cli_command_get_inst_info.h"

// C++ libraries.
#include <iostream>
#include <string>
#include <sstream>

// Local libraries.
#include "amdisa_utility.h"
#include "amdisa/isa_decoder.h"

namespace amdisa
{
    // Constants.
    // Information constant strings.
    static const char* kStringInfoInstructionRetrievalStart      = "Info: Retrieving instruction information...";
    static const char* kStringInfoInstructionRetrievalSuccessful = "Info: Instruction information retrieved successfully.";

    // Error constant strings.
    static const char* kStringErrorInstructionRetrievalFailed = "Error: Failed to retrieve instruction information.";

    bool CliCommandGetInstInfo::Execute(std::string& err_message)
    {
        bool is_executed = false;
        std::cout << kStringInfoInstructionRetrievalStart << std::endl;

        // API to get instruction information.
        std::string     decode_err_message;
        InstructionInfo instruction_info;
        is_executed = spec_api_.DecodeInstruction(instruction_name_, instruction_info, decode_err_message);

        // Print out.
        if (is_executed)
        {
            std::cout << kStringInfoInstructionRetrievalSuccessful << std::endl;
            std::cout << std::endl;

            // Print instruction name.
            std::cout << "Instruction: " << instruction_info.instruction_name << std::endl;

            // Print instruction description.
            std::cout << "Description:" << std::endl;
            std::cout << instruction_info.instruction_description << std::endl;
            std::cout << std::endl;
        }
        else
        {
            std::stringstream final_error;
            final_error << kStringErrorInstructionRetrievalFailed << ". ";
            final_error << decode_err_message << ". ";
            final_error << "Instruction name = " << instruction_name_ << std::endl;
            err_message = final_error.str();
        }

        return is_executed;
    }
}  // namespace amdisa
