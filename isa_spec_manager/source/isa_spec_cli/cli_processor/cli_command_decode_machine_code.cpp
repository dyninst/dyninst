/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#include "cli_command_decode_machine_code.h"

// C++ libraries.
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>

// Local libraries.
#include "amdisa_utility.h"
#include "amdisa/isa_decoder.h"

namespace amdisa
{
    // Constants.
    // Information constant strings.
    static const char* kStringInfoDecodeStart      = "Info: Decoding...";
    static const char* kStringInfoDecodeSuccessful = "Info: Decoding completed successfully.";

    // Error constant strings.
    static const char* kStringErrorDecodeFailed = "Error: Decoding failed.";
    static const char* kStringErrorInvalidInput = "Error: Invalid input.";

    bool CliCommandDecodeMachineCode::Execute(std::string& err_message)
    {
        bool is_executed = false;
        bool should_abort = false;

        std::cout << kStringInfoDecodeStart << std::endl;

        // Clean up the input.
        machine_code_.erase(std::remove_if(machine_code_.begin(), machine_code_.end(), [](const char c) {
            return c == '_' || std::isspace(c);
        }), machine_code_.end());

        // Convert the string machine code into uints.
        std::vector<uint32_t> machine_code_dwords;
        if (machine_code_.length() % 8 == 0)
        {
            for (size_t i = 0; i < machine_code_.length(); i += 8)
            {
                std::string single_dword = machine_code_.substr(i, 8);
                machine_code_dwords.push_back(std::strtoul(single_dword.c_str(), nullptr, 16));
            }
        }
        else
        {
            std::stringstream err_stream;
            err_stream << kStringErrorDecodeFailed << " " << kStringErrorInvalidInput;
            err_message  = err_stream.str();
            should_abort = true;
        }

        if (!should_abort)
        {
            // API to decode instruction.
            std::string                        decode_err_message;
            std::vector<InstructionInfoBundle> instruction_info_stream;
            is_executed = spec_api_.DecodeInstructionStream(machine_code_dwords, instruction_info_stream, decode_err_message);

            // Print out.
            if (is_executed)
            {
                for (const auto& instruction_info_bundle : instruction_info_stream)
                {
                    for (const auto& instruction_info : instruction_info_bundle.bundle)
                    {
                        std::cout << kStringInfoDecodeSuccessful << std::endl;
                        std::cout << std::endl;

                        // Print instruction in sp3-like format.
                        std::cout << "Instruction: ";
                        std::cout << instruction_info.instruction_name << std::endl;
                        std::cout << "Operands: ";
                        for (const auto& operand : instruction_info.instruction_operands)
                        {
                            std::cout << operand.operand_name << " ";
                        }
                        std::cout << std::endl;

                        // Print instruction description.
                        std::cout << "Description:" << std::endl;
                        std::cout << instruction_info.instruction_description << std::endl;
                        std::cout << std::endl;

                        // Print encoding information.
                        std::cout << " Encoding: " << instruction_info.encoding_name << std::endl;
                        std::cout << instruction_info.encoding_layout << std::endl;
                        std::cout << "===" << std::endl;
                    }
                }
            }
            else
            {
                std::stringstream final_error;
                final_error << kStringErrorDecodeFailed << std::endl;
                final_error << decode_err_message << ". ";
                final_error << "Provided bits = " << machine_code_ << std::endl;
                err_message = final_error.str();
            }
        }

        return is_executed;
    }
}  // namespace amdisa
