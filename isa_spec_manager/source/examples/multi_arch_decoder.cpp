/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
// multi_arch_decode.cpp: Demonstrates the decoding of instructions
// by loading several architectures at once.
// The following code snippet demonstrates how to use amdisa::DecodeManager.
// The amdisa::DecodeManager can be helpful in case that you need to decode ISA for multiple different AMD GPU architectures
// in a single session, as it allows loading multiple specification files at once. It is merely a convenience interface which
// does not add any additional functionality. Everything can be done using amdisa::IsaDecoder directly, it will just require more code.
// The three specification files that we will decode.

// C++.
#include <iostream>
#include <sstream>
#include <string>

// ISA Spec API.
#include "amdisa/isa_decoder.h"

// Constants.
// Error constant string.
static const char* kStrErrorInvalidArgument    = "Error: Requires at least one program argument.";
static const char* kStrErrorSpecNotInitialized = "Error: Spec not initialized.\n";
static const char* kStrErrorDecodeFail         = "Error: Instruction failed to decode. ";
static const char* kStrErrorRetrieveFail       = "Error: Manager failed to retrieve the decoder.";

// Info constant string.
static const char* kStrInfoInitializingManager = "Info: Initializing DecodeManager from the following specs: ";
static const char* kStrInfoManagerInitialized  = "Info: DecodeManager is now initialized.";
static const char* kStrInfoRetrieveDecoder     = "Info: Retrieving the decoder.";
static const char* kStrInfoDecodingInstruction = "Info: Decoding Instruction ";
static const char* kStrInfoDecodeSuccess       = "Info: Instruction decoded successfully.";

int main(int argc, char* argv[])
{
    // Decoding API example.
    amdisa::DecodeManager decode_manager;

    // bool flags to check for failures.
    bool is_error   = false;
    bool is_success = false;

    // Check for valid Input Argument.
    if (argc < 2)
    {
        std::cerr << kStrErrorInvalidArgument << std::endl;
        is_error = true;
    }

    std::string error_msg;

    // Initialize the API with a spec.
    if (!is_error)
    {
        // Get paths from CLI arguments.
        std::cout << kStrInfoInitializingManager << std::endl;
        std::vector<std::string> xml_spec_paths;
        for (uint32_t i = 1; i < argc; i++)
        {
            xml_spec_paths.push_back(argv[i]);
            std::cout << " - " << argv[i] << std::endl;
        }

        // Initialize the DecodeManager object with all paths.
        is_success = decode_manager.Initialize(xml_spec_paths, error_msg);

        // Check if the ISA spec is initialized.
        if (is_success)
        {
            std::cout << kStrInfoManagerInitialized << std::endl;
        }
        else
        {
            std::cerr << kStrErrorSpecNotInitialized << error_msg << std::endl;
            is_error = true;
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

        // Retrieve decoder.
        std::cout << kStrInfoRetrieveDecoder << std::endl;
        std::shared_ptr<amdisa::IsaDecoder> rdna3_decoder = decode_manager.GetDecoder(amdisa::GpuArchitecture::kRdna3);
        is_success                                            = rdna3_decoder != nullptr;

        // Decode an instruction.
        amdisa::InstructionInfoBundle instruction_info;
        if (is_success)
        {
            is_success = rdna3_decoder->DecodeInstruction(instruction_binary, instruction_info, error_msg);
        }
        else
        {
            std::cerr << kStrErrorRetrieveFail << std::endl;
        }

        if (is_success)
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
        else
        {
            std::cerr << kStrErrorDecodeFail << error_msg << std::endl;
            is_error = true;
        }
    }

    return 0;
}
