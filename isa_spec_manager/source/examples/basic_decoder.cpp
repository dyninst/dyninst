/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
// IsaDecoderExamples.cpp: Demonstrates the decoding of an example single
// instruction's binary representation using IsaDecoder - DecodeInstruction().
// Required Argument for the program: Valid ISA Spec XML file path.

// C++.
#include <iostream>
#include <sstream>
#include <string>

// ISA Spec API.
#include "amdisa/isa_decoder.h"

// Constants.
// Error constant string.
static const char* kStrErrorInvalidArgument    = "Error: Requires one program argument.";
static const char* kStrErrorSpecNotInitialized = "Error: Spec not initialized. ";
static const char* kStrErrorDecodeFail         = "Error: Instruction failed to decode. ";

// Info constant string.
static const char* kStrInfoInitializingSpec    = "Info: Initializing AMD ISA Spec from ";
static const char* kStrInfoSpecInitialized     = "Info: ISA Spec is now initialized.";
static const char* kStrInfoDecodingInstruction = "Info: Decoding Instruction ";
static const char* kStrInfoDecodeSuccess       = "Info: Instruction decoded successfully.";

int main(int argc, char* argv[])
{
    // Decoding API example.
    amdisa::IsaDecoder spec_api_example;

    // bool flags to check for failures.
    bool is_error   = false;
    bool is_success = false;

    // Check for valid Input Argument.
    if (argc != 2)
    {
        std::cerr << kStrErrorInvalidArgument << std::endl;
        is_error = true;
    }

    std::string error_msg;

    // Initialize the API with a spec.
    if (!is_error)
    {
        const std::string kPathToSpec = argv[1];
        std::cout << kStrInfoInitializingSpec << kPathToSpec << std::endl;
        is_success = spec_api_example.Initialize(kPathToSpec, error_msg);

        // Check if the ISA spec is initialized.
        if (!is_success)
        {
            std::cerr << kStrErrorSpecNotInitialized << error_msg << std::endl;
            is_error = true;
        }
        else
        {
            std::cout << kStrInfoSpecInitialized << std::endl;
        }
    }

    if (!is_error)
    {
        // Prepare the input sample instruction's binary representation.
        const std::string kStrSampleInstructionBinary = "8BEA7E6A";
        std::cout << kStrInfoDecodingInstruction << kStrSampleInstructionBinary << std::endl;
        std::stringstream kIsaInstructionStream;
        kIsaInstructionStream << kStrSampleInstructionBinary;
        uint64_t instruction_binary = 0;
        kIsaInstructionStream >> std::hex >> instruction_binary;

        // Decode an instruction.
        amdisa::InstructionInfoBundle instruction_info;
        is_success = spec_api_example.DecodeInstruction(instruction_binary, instruction_info, error_msg);

        if (!is_success)
        {
            std::cerr << kStrErrorDecodeFail << error_msg << std::endl;
            is_error = true;
        }
        else
        {
            std::cout << kStrInfoDecodeSuccess << std::endl;

            // Display decoded instruction's information from the instruction_info
            // bundle (InstructionInfo).
            for (const auto& inst : instruction_info.bundle)
            {
                std::cout << "Instruction Name: " << inst.instruction_name << std::endl;
                std::cout << "Instruction Description: " << inst.instruction_description << std::endl;
                std::cout << "Encoding Name: " << inst.encoding_name << std::endl;
                std::cout << "Encoding Description: " << inst.encoding_description << std::endl;
                std::cout << "Functional Group: " << amdisa::kFunctionalGroupName[static_cast<int>(inst.functional_group_subgroup_info.IsaFunctionalGroup)]
                          << std::endl;
                std::cout << "Functional Subgroup: "
                          << amdisa::kFunctionalSubgroupName[static_cast<int>(inst.functional_group_subgroup_info.IsaFunctionalSubgroup)] << std::endl;
                std::cout << "Functional Group Description: " << inst.functional_group_subgroup_info.description << std::endl;
                // Similarly, the following information about the instruction can be
                // fetched:
                // 1. encoding:
                //     encoding_fields - field_name, field_value, bit_count, bit_offset
                //     encoding_layout
                // 2. operands:
                //     instruction_operands - operand_name, operand_size, is_input,
                //                            is_output
                // 3. modifiers:
                //     operand_modifiers - modifier_name, value
                // 4. semantics:
                //     instruction_semantic_info - is_program_terminator,
                //                                 is_immediately_executed,
                //                                 branch_info (is_branch,
                //                                 is_conditional, is_indirect,
                //                                 branch_offset, branch_target_PC,
                //                                 branch_target_index)
            }
        }
    }

    return 0;
}
