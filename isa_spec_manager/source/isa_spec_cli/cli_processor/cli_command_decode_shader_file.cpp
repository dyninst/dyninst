/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#include "cli_command_decode_shader_file.h"

// C++ libraries.
#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>

// Local libraries.
#include "amdisa_utility.h"
#include "amdisa_xml_element_consts.h"
#include "amdisa/isa_decoder.h"

// Third party libraries.
#include "tinyxml2.h"

using XMLDocument = tinyxml2::XMLDocument;
using XMLElement  = tinyxml2::XMLElement;

namespace amdisa
{
    // Constants.
    // Information constant strings.
    static const char* kStringInfoDecodeShaderFileStart      = "Info: Decoding Shader file... ";
    static const char* kStringInfoDecodeShaderFileSuccessful = "Info: Decoding Shader file completed successfully.";

    // Error constant strings.
    static const char* kStringErrorDecodeFailed   = "Error: Decoding Shader file failed.";
    static const char* kStringErrorXmlWriteFailed = "Error: Decoding Shader file failed. Could not write XML file.";

    // Creates a new XML element with a string value
    // and attachs to the parent XML element
    void AddToParentXmlElement(XMLElement* parent_element, const char* element_name, const char* value)
    {
        XMLDocument* XmlDoc      = parent_element->GetDocument();
        XMLElement*  element_ptr = XmlDoc->NewElement(element_name);
        assert(element_ptr != nullptr);
        element_ptr->SetText(value);
        parent_element->InsertEndChild(element_ptr);
    }

    // Creates a new XML element with a uint64_t value
    // and attachs to the parent XML element
    void AddToParentXmlElement(XMLElement* parent_element, const char* element_name, const uint64_t& value)
    {
        XMLDocument* XmlDoc      = parent_element->GetDocument();
        XMLElement*  element_ptr = XmlDoc->NewElement(element_name);
        assert(element_ptr != nullptr);
        element_ptr->SetText(value);
        parent_element->InsertEndChild(element_ptr);
    }

    // Removes whitespaces from the given string and returns the modified string
    std::string RemoveWhitespace(const std::string& str)
    {
        std::string str_no_ws  = str;
        auto        space_iter = std::find_if(str_no_ws.begin(), str_no_ws.end(), [](char ch) { return !std::isspace(ch, std::locale::classic()); });
        str_no_ws.erase(str_no_ws.begin(), space_iter);
        auto space_iter_rev = std::find_if(str_no_ws.rbegin(), str_no_ws.rend(), [](char ch) { return !std::isspace(ch, std::locale::classic()); });
        str_no_ws.erase(space_iter_rev.base(), str_no_ws.end());
        return str_no_ws;
    }

    void PrintInstructionInfo(XMLElement* XmlInstructionInfo, const InstructionInfo& inst, bool is_branch_target_info_set = false)
    {
        XMLDocument* XmlDoc = XmlInstructionInfo->GetDocument();
        XMLElement*  parent_element;
        XMLElement*  child_element;

        // Add Instruction Name XML Element
        AddToParentXmlElement(XmlInstructionInfo, kElementInstructionName, inst.instruction_name.c_str());

        // Add Instruction Description XML Element
        AddToParentXmlElement(XmlInstructionInfo, kElementInstructionDescription, RemoveWhitespace(inst.instruction_description).c_str());

        // Add Encoding Name XML Element
        AddToParentXmlElement(XmlInstructionInfo, kElementEncodingName, inst.encoding_name.c_str());

        // Add Encoding Description XML Element
        AddToParentXmlElement(XmlInstructionInfo, kElementEncodingDescription, RemoveWhitespace(inst.encoding_description).c_str());

        // Add Encoding Fields XML Element
        parent_element = XmlDoc->NewElement(kElementEncodingFields);
        assert(parent_element != nullptr);
        XmlInstructionInfo->InsertEndChild(parent_element);
        if (inst.encoding_fields.size())
        {
            for (const auto& field : inst.encoding_fields)
            {
                child_element = XmlDoc->NewElement(kElementEncoding);
                assert(child_element != nullptr);
                AddToParentXmlElement(child_element, kElementName, field.field_name.c_str());
                AddToParentXmlElement(child_element, kElementValue, field.field_value);
                AddToParentXmlElement(child_element, kElementBitCount, field.bit_count);
                AddToParentXmlElement(child_element, kElementBitOffset, field.bit_offset);
                parent_element->InsertEndChild(child_element);
                child_element = nullptr;
            }
        }
        else
        {
            parent_element->SetText("None");
        }

        // Add Instruction Operands XML Element
        parent_element = XmlDoc->NewElement(kElementOperands);
        assert(parent_element != nullptr);
        XmlInstructionInfo->InsertEndChild(parent_element);
        if (inst.instruction_operands.size())
        {
            for (const auto& operand : inst.instruction_operands)
            {
                child_element = XmlDoc->NewElement(kElementOperand);
                assert(child_element != nullptr);

                // Get operand name.
                std::string operand_to_print = operand.operand_name.c_str();

                // Overwrite operand name with label if this is the branch.
                if (!inst.instruction_semantic_info.branch_info.branch_target_label.empty())
                {
                    operand_to_print = inst.instruction_semantic_info.branch_info.branch_target_label;
                }
                AddToParentXmlElement(child_element, kElementName, operand_to_print.c_str());
                AddToParentXmlElement(child_element, kElementOperandSize, operand.operand_size);
                AddToParentXmlElement(child_element, kAttributeInput, (operand.is_input ? "yes" : "no"));
                AddToParentXmlElement(child_element, kAttributeOutput, (operand.is_output ? "yes" : "no"));
                parent_element->InsertEndChild(child_element);
                child_element = nullptr;
            }
        }
        else
        {
            parent_element->SetText("None");
        }

        // Add Instruction Operand Modifiers XML Element
        parent_element = XmlDoc->NewElement(kElementOperandModifiers);
        assert(parent_element != nullptr);
        XmlInstructionInfo->InsertEndChild(parent_element);
        if (inst.operand_modifiers.size())
        {
            for (const auto& modifier : inst.operand_modifiers)
            {
                child_element = XmlDoc->NewElement(kElementOperandModifier);
                AddToParentXmlElement(child_element, kElementName, modifier.modifier_name.c_str());
                AddToParentXmlElement(child_element, kElementValue, modifier.value);
                parent_element->InsertEndChild(child_element);
                child_element = nullptr;
            }
        }
        else
        {
            parent_element->SetText("None");
        }

        // Add Instruction Semantics XML Element
        parent_element = XmlDoc->NewElement(kElementInstructionSemantics);
        assert(parent_element != nullptr);
        XmlInstructionInfo->InsertEndChild(parent_element);
        AddToParentXmlElement(parent_element, kElementFlagIsBranch, (inst.instruction_semantic_info.branch_info.is_branch ? "yes" : "no"));
        AddToParentXmlElement(parent_element, kElementFlagIsProgramTerminator, (inst.instruction_semantic_info.is_program_terminator ? "yes" : "no"));
        AddToParentXmlElement(parent_element, kElementFlagIsImmediatelyExecuted, (inst.instruction_semantic_info.is_immediately_executed ? "yes" : "no"));

        child_element = XmlDoc->NewElement(kElementInstructionBranchInfo);
        assert(child_element != nullptr);
        XmlInstructionInfo->InsertEndChild(child_element);
        const BranchInfo* info      = &inst.instruction_semantic_info.branch_info;
        bool              is_branch = ((info != nullptr) && info->is_branch);
        AddToParentXmlElement(
            child_element, kElementFlagIsConditionalBranch, (((info != nullptr) && info->is_conditional) ? "yes" : (is_branch ? "no" : "N/A")));
        AddToParentXmlElement(child_element, kElementFlagIsIndirectBranch, (((info != nullptr) && info->is_indirect) ? "yes" : (is_branch ? "no" : "N/A")));
        AddToParentXmlElement(child_element,
                              kElementInstructionBranchOffset,
                              ((info != nullptr) && (info->branch_offset != kBranchOffsetOutOfRange)) ? std::to_string(info->branch_offset).c_str() : "N/A");
        AddToParentXmlElement(child_element,
                              kElementInstructionBranchTargetPC,
                              (((info != nullptr) && !info->branch_target_pc.empty()) ? info->branch_target_pc.c_str() : "N/A"));
        AddToParentXmlElement(child_element,
                              kElementInstructionBranchTargetLabel,
                              (((info != nullptr) && !info->branch_target_label.empty()) ? info->branch_target_label.c_str() : "N/A"));
        AddToParentXmlElement(
            child_element,
            kElementInstructionBranchTargetIndex,
            (((info != nullptr) && info->branch_target_index != kInvalidBranchTarget) ? std::to_string(info->branch_target_index).c_str() : "N/A"));

        // Add Instruction's functional group and subgroup info
        parent_element = XmlDoc->NewElement(kElementFunctionalGroup);
        assert(parent_element != nullptr);
        XmlInstructionInfo->InsertEndChild(parent_element);
        AddToParentXmlElement(parent_element, kElementName, kFunctionalGroupName[static_cast<int>(inst.functional_group_subgroup_info.IsaFunctionalGroup)]);
        AddToParentXmlElement(
            parent_element, kAttributeTypeSubgroup, kFunctionalSubgroupName[static_cast<int>(inst.functional_group_subgroup_info.IsaFunctionalSubgroup)]);
        AddToParentXmlElement(parent_element, kElementDescription, inst.functional_group_subgroup_info.description.c_str());
    }

    bool CliCommandDecodeShaderFile::Execute(std::string& err_message)
    {
        bool is_executed = false;
        std::cout << kStringInfoDecodeShaderFileStart << shader_file_path_ << std::endl;

        // Capitalize info format for consistency.
        const std::string info_format_cap = AmdIsaUtility::ToUpper(info_format_);

        // API to decode instruction.
        std::string                        api_error_message;
        std::vector<InstructionInfoBundle> instruction_info_stream;
        bool                               is_decode_successful =
            spec_api_.DecodeShaderDisassemblyFile(shader_file_path_, instruction_info_stream, api_error_message, is_branch_target_info_set_);

        // Print out.
        if (is_decode_successful)
        {
            std::cout << kStringInfoDecodeShaderFileSuccessful << std::endl;
            if (info_format_cap == "XML")
            {
                // Variables used for generating output in XML format
                XMLDocument XmlOutput;
                XMLElement* XmlInfo = XmlOutput.NewElement("DecodedOutput");
                XmlOutput.InsertFirstChild(XmlInfo);
                XMLElement* XmlInstructionInfos = XmlOutput.NewElement(kElementShaderInstructions);
                XmlInfo->InsertFirstChild(XmlInstructionInfos);
                uint32_t inst_order = 0;
                for (const auto& instruction_info_bundle : instruction_info_stream)
                {
                    XMLElement* XmlInstructionInfo = XmlOutput.NewElement(kElementInstruction);
                    AddToParentXmlElement(XmlInstructionInfo, kElementInstructionOrder, inst_order++);
                    for (const auto& instruction_info : instruction_info_bundle.bundle)
                    {
                        PrintInstructionInfo(XmlInstructionInfo, instruction_info, is_branch_target_info_set_);
                    }
                    XmlInstructionInfos->InsertEndChild(XmlInstructionInfo);
                    XmlInstructionInfo = nullptr;
                }
                if (!output_xml_path_.empty())
                {
                    tinyxml2::XMLError status = XmlOutput.SaveFile(output_xml_path_.c_str());

                    if (status == tinyxml2::XML_SUCCESS)
                    {
                        is_executed = true;
                    }
                    else
                    {
                        err_message = kStringErrorXmlWriteFailed;
                    }
                }
                else
                {
                    XmlOutput.Print();
                    is_executed = true;
                }
            }
            else
            {
                for (const auto& instruction_info_bundle : instruction_info_stream)
                {
                    for (const auto& instruction_info : instruction_info_bundle.bundle)
                    {
                        std::cout << std::endl;

                        // Print instruction in sp3-like format.
                        std::cout << "Instruction: ";
                        std::cout << std::setw(17) << std::left << instruction_info.instruction_name << " ";
                        for (const auto& operand : instruction_info.instruction_operands)
                        {
                            std::cout << operand.operand_name << " ";
                        }
                        std::cout << std::endl;

                        // Print instruction description.
                        std::cout << "HTML Description:" << std::endl;
                        std::cout << instruction_info.instruction_description << std::endl;
                        std::cout << std::endl;

                        // Print encoding information.
                        std::cout << "Encoding: " << instruction_info.encoding_name << std::endl;
                        std::cout << instruction_info.encoding_layout << std::endl;

                        // Print about target if it's a direct branch.
                        uint64_t target_index = instruction_info.instruction_semantic_info.branch_info.branch_target_index;
                        if (target_index != amdisa::kInvalidBranchTarget)
                        {
                            std::cout << "Branch Target Information" << std::endl;
                            amdisa::InstructionInfo target_inst_info;
                            target_inst_info = instruction_info_stream[target_index].bundle[0];

                            // Print target instruction in sp3-like format.
                            std::cout << "Target PC: " << instruction_info.instruction_semantic_info.branch_info.branch_target_pc << std::endl;
                            std::cout << "Instruction: ";
                            std::cout << std::setw(17) << std::left << target_inst_info.instruction_name << " ";
                            for (const auto& operand : target_inst_info.instruction_operands)
                            {
                                std::cout << operand.operand_name << " ";
                            }
                            std::cout << std::endl;
                        }

                        // Print instruction's functional group and subgroup info
                        std::cout << "Functional Group: "
                                  << kFunctionalGroupName[static_cast<int>(instruction_info.functional_group_subgroup_info.IsaFunctionalGroup)] << std::endl;
                        std::cout << "Functional Subgroup: "
                                  << kFunctionalSubgroupName[static_cast<int>(instruction_info.functional_group_subgroup_info.IsaFunctionalSubgroup)]
                                  << std::endl;
                        std::cout << "Functional Group Description: " << instruction_info.functional_group_subgroup_info.description << std::endl;

                        std::cout << "===" << std::endl;
                    }
                }
                is_executed = true;
            }
        }
        else
        {
            std::stringstream final_error;
            std::cerr << kStringErrorDecodeFailed << std::endl;
            std::cerr << api_error_message;
        }

        return is_executed;
    }
}  // namespace amdisa
