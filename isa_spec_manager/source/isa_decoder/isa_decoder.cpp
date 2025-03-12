/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#include "amdisa/isa_decoder.h"

// C++ libraries.
#include <algorithm>
#include <cassert>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>

// Local libraries.
#include "amdisa_structures.h"
#include "amdisa_utility.h"
#include "amdisa/api_version.h"
#include "encoding_condition_handler.hpp"
#include "isa_xml_reader.h"

namespace amdisa
{
    // Supported XML version.
    static const char* kMaxSupportedSchemaVersion = "1.0.0";

    // Error string constants.
    static const char* kStringErrorUnsupportedXml              = "Error: Unsupported XML schema version. Check for an updated version of the API.";
    static const char* kStringErrorEmptyRange                  = "Error: Processing empty range.";
    static const char* kStringErrorEncodingNotFound            = "Error: Encoding or opcode not found for the instruction: ";
    static const char* kStringErrorEmptyOperandName            = "Error: Empty operand name was provided.";
    static const char* kStringErrorInstructionNotFoundInSpec   = "Error: Instruction not found in the XML ISA Specification. ";
    static const char* kStringErrorFailedToDecodeOperands      = "Error: Failed to decode operands.";
    static const char* kStringErrorSpecNotInitialized          = "Error: API Implementation is not initialized.";
    static const char* kStringErrorModiferFieldNotFound        = "Error: Defined modifier was not found in the encoding.Check encodings to modifiers mapping.";
    static const char* kStringErrorShaderFileNotOpen           = "Error: Failed to read shader disassembly file.";
    static const char* kStringErrorBranchTargetInfoNotFound    = "Error: Failed to find target information for the direct branch. ";
    static const char* kStringErrorBranchTargetIndexNotFound   = "Error: Failed to find target index for the direct branch: ";
    static const char* kStringErrorBranchTargetLabelNotFound   = "Error: Failed to find target label for the direct branch: ";
    static const char* kStringExceptionShaderFileNotOpen       = "Error: Exception occured while opening shader disassembly for reading: ";
    static const char* kStringErrorShaderTextInvalidDwordByte  = "Error: Invalid DWORD Byte: ";
    static const char* kStringErrorShaderTextDecodeFailed      = "Error: Instruction decoding failed during shader disassembly processing.";
    static const char* kStringErrorShaderTextDecodeNoInst      = "Error: No valid instructions found in the provided shader disassembly file.";
    static const char* kStringErrorMissingTargetResolutionInfo = "Error: Branch target resolution information missing";
    static const char* kStringErrorIsaDecoderNotFound          = "Error: Invalid IsaDecoder provided to decode the instructions";
    static const char* kStringNa                               = "N/A";
    static const char* kStringErrorApiImplAllocationFailed     = "Error: API Implementation object allocation failed";
    static const char* kStringErrorManagerImplAllocationFailed = "Error: Manager Implementation object allocation failed";
    static const char* kStringErrorEmptyRangesInField          = "Error: Range is empty for the field: ";

    // Decoder manager string constants.
    static const char* kStringErrorDecodeManagerUnknownArch  = "Error: Undefined architecture in specification.";
    static const char* kStringErrorDecodeManagerInitFailed   = "Error: Failed to initialize ISA specification. Provided file path: ";
    static const char* kStringErrorDecodeManagerArchNotFound = "Error: Provided architecture was not initialized.";

    // Encoding modifier definitions. Constructed based on the PDF ISA.
    static const char* kModifierNameNegation       = "NEG";
    static const char* kModifierNameOutputModifier = "OMOD";
    static const char* kModifierNameAbsoluteValue  = "ABS";
    static const char* kModifierNameClamp          = "CLAMP";
    static const char* kModifierNameOffset         = "OFFSET";

    // Define which encodings have modifiers.
    static const std::map<std::string, std::vector<std::string>> kEncodingToOperandModifiers = {
        {"ENC_VOP3", {kModifierNameAbsoluteValue, kModifierNameNegation, kModifierNameOutputModifier, kModifierNameClamp}},
        {"ENC_VOP3B", {kModifierNameNegation, kModifierNameOutputModifier, kModifierNameClamp}},
        {"ENC_VOP3P", {kModifierNameNegation, kModifierNameClamp}},
        {"ENC_FLAT", {kModifierNameOffset}},
        {"ENC_FLAT_SCRATCH", {kModifierNameOffset}},
        {"ENC_FLAT_GLBL", {kModifierNameOffset}},
        {"ENC_FLAT_GLOBAL", {kModifierNameOffset}}};

    // SEG bits for FLAT, SCRATCH, and GLOBAL encodings.
    static const std::map<std::string, uint32_t> kSegBits = {{"ENC_FLAT", 0}, {"ENC_FLAT_SCRATCH", 1}, {"ENC_FLAT_GLBL", 2}, {"ENC_FLAT_GLOBAL", 2}};

    // Map architecture IDs to enums.
    static const std::map<uint32_t, GpuArchitecture> kArchitectureIdToEnum = {
        { 0, GpuArchitecture::kCdna1 },
        { 1, GpuArchitecture::kCdna2 },
        { 2, GpuArchitecture::kCdna3 },
        { 5, GpuArchitecture::kRdna1 },
        { 6, GpuArchitecture::kRdna2 },
        { 8, GpuArchitecture::kRdna3 }
    };

    // Masks.
    static const uint32_t kDwordMask = 0xffffffff;

    // Instructions shader disassembly text delimiter
    static const char kShaderTextDelimiter = '\n';

    // Instruction byte size constant (32 bit, 4 bytes)
    static const unsigned int kInstructionBytes32 = 4;
    static const unsigned int kDwordSize          = 32;

    // ISA information that require special handling.
    static const char* kWaveDependentFormat = "FMT_NUM_M64";

    // Type definitions.
    using IdToEncoding             = std::unordered_map<uint64_t, std::shared_ptr<Encoding>>;
    using IdToInstruction          = std::unordered_map<uint64_t, std::shared_ptr<Instruction>>;
    using IdToInstructionEncodings = std::unordered_map<uint64_t, std::vector<std::shared_ptr<InstructionEncoding>>>;
    using EncodingIterator         = std::vector<Encoding>::const_iterator;
    using FieldIterator            = std::vector<Field>::const_iterator;
    struct InstructionPtrs
    {
        std::shared_ptr<Instruction>         instruction_ptr          = nullptr;
        std::shared_ptr<InstructionEncoding> instruction_encoding_ptr = nullptr;
    };

    // *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - BEGIN ***
    class MachineCodeStream
    {
    public:
        MachineCodeStream() = default;
        void Init(const std::vector<uint32_t>& machine_code_stream)
        {
            for (uint32_t machine_code : machine_code_stream)
            {
                machine_code_stream_.push_back(machine_code);
            }
            is_empty_ = machine_code_stream_.empty();
        }

        uint32_t GetNextDword()
        {
            uint32_t ret = 0;
            if (!is_empty_)
            {
                ret = machine_code_stream_.front();
                machine_code_stream_.pop_front();
                is_empty_ = machine_code_stream_.empty();
            }
            else
            {
                std::cerr << "Warning: Nothing to get, the stream is empty." << std::endl;
            }
            return ret;
        }

        bool IsEmpty()
        {
            return is_empty_;
        }

    private:
        std::deque<uint32_t> machine_code_stream_;
        bool                 is_empty_ = true;
    };

    static uint64_t GetFieldValue(const Field& field, const std::vector<uint32_t>& working_dwords)
    {
        Range    range;
        uint64_t field_value = 0;
        if (AmdIsaUtility::GetRange(field, range))
        {
            uint64_t bit_count   = range.bit_count;
            uint64_t bit_offset  = range.bit_offset;
            uint64_t dword_index = bit_offset / 32;
            uint32_t dword       = 0;
            if (dword_index < working_dwords.size())
            {
                dword = working_dwords[dword_index];
            }
            else
            {
                std::cout << "Warning: DWORD is 0" << std::endl;
            }
            uint64_t mask = ((1ULL << bit_count) - 1);
            field_value   = (dword >> (bit_offset - 32 * dword_index)) & mask;
            if (!AmdIsaUtility::GetRange(field, range))
            {
                std::cerr << kStringErrorEmptyRange << std::endl;
                assert(false);
            }
            uint64_t padding_size = range.padding.bit_count;
            if (padding_size > 0)
            {
                uint32_t padding_value = range.padding.value;
                field_value            = (field_value << padding_size) | padding_value;
            }
        }
        else
        {
            std::cerr << kStringErrorEmptyRange << std::endl;
            assert(false);
        }
        return field_value;
    }

    static void RetrieveFieldInfo(const std::vector<uint32_t>& working_dwords, const std::vector<Field>& bitmap, InstructionInfo& instruction_info)
    {
        // Get the fields info.
        for (const auto& field : bitmap)
        {
            Range range;
            if (AmdIsaUtility::GetRange(field, range))
            {
                instruction_info.encoding_fields.push_back(EncodingField());
                auto&    encoding_field = instruction_info.encoding_fields.back();
                uint64_t bit_count      = range.bit_count;
                uint64_t bit_offset     = range.bit_offset;
                uint64_t dword_index    = bit_offset / 32;

                encoding_field.bit_count   = bit_count;
                encoding_field.bit_offset  = bit_offset;
                encoding_field.field_name  = field.name;
                encoding_field.field_value = GetFieldValue(field, working_dwords);
            }
            else
            {
                std::cerr << kStringErrorEmptyRange << std::endl;
                assert(false);
            }
        }
    }

    static uint64_t GetFieldMask(const Field& field)
    {
        uint64_t mask = 0;

        for (const auto& range : field.ranges)
        {
            uint64_t current_mask = static_cast<uint64_t>((1 << range.bit_count) - 1) << range.bit_offset;
            mask |= current_mask;
        }

        return mask;
    }

    static std::shared_ptr<Encoding> GetEncodingIterator(const IdToEncoding& id_to_encodings, uint64_t first_dword, const IsaSpec& spec_data)
    {
        // Go over each encoding in the passed spec data.
        std::shared_ptr<Encoding> encoding          = nullptr;
        bool                      is_encoding_found = false;
        for (auto encoding_iterator = spec_data.encodings.begin(); !is_encoding_found && encoding_iterator != spec_data.encodings.end(); ++encoding_iterator)
        {
            // Dereference the iterator.
            const auto& single_encoding = *encoding_iterator;

            // Mask the encoding identifier bits.
            uint64_t masked_machine_code = first_dword & (single_encoding.mask | single_encoding.opcode_mask | single_encoding.seg_mask);
            if (id_to_encodings.find(masked_machine_code) != id_to_encodings.end())
            {
                encoding = id_to_encodings.at(masked_machine_code);
                if (encoding == nullptr)
                {
                    break;
                }
                if (encoding->name == single_encoding.name)
                {
                    is_encoding_found = true;
                }
            }

            // If the instruction is dual, pick the encoding of the second instruction if its encoding size is bigger.
            if (is_encoding_found)
            {
                auto field_iter = std::find_if(single_encoding.microcode_format.bit_map.begin(),
                                               single_encoding.microcode_format.bit_map.end(),
                                               [&](const Field& field) { return field.name == "OPY"; });

                if (field_iter != single_encoding.microcode_format.bit_map.end())
                {
                    uint64_t opy_mask                = GetFieldMask(*field_iter);
                    uint64_t masked_machine_code_opy = first_dword & (single_encoding.mask | opy_mask | single_encoding.seg_mask);
                    if (id_to_encodings.find(masked_machine_code_opy) != id_to_encodings.end())
                    {
                        std::shared_ptr<Encoding> encoding_new = id_to_encodings.at(masked_machine_code_opy);
                        if (encoding_new == nullptr)
                        {
                            break;
                        }
                        if (encoding->bit_count < encoding_new->bit_count)
                        {
                            encoding = encoding_new;
                        }
                    }
                }
            }
        }

        return encoding;
    }

    // Checks if the operand with the specific name present in the instruction encoding.
    static bool IsOperandPresent(const InstructionEncoding& instruction_encoding, const std::string& operand_name)
    {
        const auto& operand_iter = std::find_if(instruction_encoding.operands.begin(), instruction_encoding.operands.end(), [&](const Operand& operand) {
            return operand.encoding_field_name == operand_name;
        });
        return operand_iter != instruction_encoding.operands.end();
    }

    // Gets the iterator to the field from the bitmap with a specific name.
    static FieldIterator GetFieldIterator(const MicrocodeFormat& microcode, const std::string& field_name)
    {
        return std::find_if(microcode.bit_map.begin(), microcode.bit_map.end(), [&](const Field& field) { return field.name == field_name; });
    }

    // Gets the iterator to the field from the bitmap with a specific name and gets the value of
    // the field from the machine code.
    static FieldIterator GetFieldIterator(const std::vector<uint32_t>& machine_code,
                                          const std::string&           field_name,
                                          const MicrocodeFormat&       microcode_format,
                                          uint64_t&                    field_value)
    {
        FieldIterator found_field    = microcode_format.bit_map.end();
        bool          is_field_found = field_name.empty();

        // Go over each field in the passed microcode format structure.
        for (auto field_iterator = microcode_format.bit_map.begin(); !is_field_found && field_iterator != microcode_format.bit_map.end(); ++field_iterator)
        {
            if (field_iterator->name.find(field_name) != std::string::npos)
            {
                is_field_found = true;
                field_value    = GetFieldValue(*field_iterator, machine_code);
                found_field    = field_iterator;
            }
        }

        return found_field;
    }

    static std::string GetNameAsRegisterRange(const std::string& operand_name, uint32_t operand_size)
    {
        std::stringstream reg_name_formatter;
        if (!operand_name.empty())
        {
            const char     base_reg_name = operand_name[0];
            const uint64_t reg_id        = AmdIsaUtility::StringToUnsignedInt(operand_name.substr(1));
            const uint32_t dword_num     = operand_size / kDwordSize;

            reg_name_formatter << base_reg_name << "[" << reg_id << ":" << reg_id + dword_num - 1 << "]";
        }
        else
        {
            std::cerr << kStringErrorEmptyOperandName << std::endl;
            assert(false);
        }
        return reg_name_formatter.str();
    }

    static std::string GeneratePartitionedOperand(const amdisa::MicrocodeFormat& microcode_format, const uint32_t field_value)
    {
        std::stringstream ret;
        ret << "{ ";

        // Package field value for the use with GetFieldValue function.
        std::vector<uint32_t> working_dword;
        working_dword.push_back(field_value);
        for (const auto& field : microcode_format.bit_map)
        {
            ret << field.name << ":";
            uint64_t subvalue             = GetFieldValue(field, working_dword);
            bool     has_predefined_value = false;
            if (field.predefined_values.size() > 0)
            {
                const auto& predefined_value_iterator =
                    std::find_if(field.predefined_values.begin(), field.predefined_values.end(), [&](const amdisa::PredefinedValue& predefined_value) {
                        return static_cast<uint64_t>(predefined_value.value) == subvalue;
                    });
                has_predefined_value = (predefined_value_iterator != field.predefined_values.end());
                if (has_predefined_value)
                {
                    ret << predefined_value_iterator->name << "; ";
                }
            }

            if (!has_predefined_value)
            {
                ret << subvalue << "; ";
            }
        }
        ret << "}";
        return ret.str();
    }

    static bool ExtractModifiers(const std::vector<uint32_t>& machine_code,
                                 const Encoding&              encoding,
                                 std::vector<OperandModifer>& operand_modifiers,
                                 std::string&                 err_message)
    {
        bool is_success = false;

        auto modifier_list_iterator = kEncodingToOperandModifiers.begin();
        bool is_found               = false;
        while (!is_found && modifier_list_iterator != kEncodingToOperandModifiers.end())
        {
            if (encoding.name == modifier_list_iterator->first)
            {
                is_found = true;
            }
            else
            {
                modifier_list_iterator++;
            }
        }

        if (modifier_list_iterator != kEncodingToOperandModifiers.end())
        {
            bool        should_abort        = false;
            const auto& modifiers           = modifier_list_iterator->second;
            uint32_t    processed_modifiers = 0;
            for (auto modifier_name_iterator = modifiers.begin(); !should_abort && modifier_name_iterator != modifiers.end(); ++modifier_name_iterator)
            {
                auto& modifier_name = *modifier_name_iterator;

                // Get modifier value.
                uint64_t    modifier_value = 0;
                const auto& field_iterator = GetFieldIterator(machine_code, modifier_name, encoding.microcode_format, modifier_value);

                // Save the modifier.
                if (field_iterator != encoding.microcode_format.bit_map.end())
                {
                    operand_modifiers.push_back(OperandModifer());
                    auto& instruction_modifier         = operand_modifiers.back();
                    instruction_modifier.modifier_name = modifier_name;
                    instruction_modifier.value         = static_cast<uint32_t>(modifier_value);
                }
                else
                {
                    err_message  = kStringErrorModiferFieldNotFound;
                    should_abort = true;
                }

                // Success condition.
                ++processed_modifiers;
                if (processed_modifiers == modifiers.size())
                {
                    is_success = true;
                }
            }
        }
        else
        {
            // Apply extraction to only defined encodings. Return true for encodings
            // with no modifiers.
            is_success = true;
        }

        return is_success;
    }

    static InstructionPtrs GetInstructionPtrs(uint32_t                        architecture_id,
                                              const IdToInstruction&          id_to_inst,
                                              const IdToInstructionEncodings& id_to_inst_enc,
                                              const Encoding&                 encoding,
                                              const EncodingConditionHandler& condition_handler,
                                              uint32_t                        first_dword,
                                              uint32_t                        second_dword,
                                              bool                            is_vopdx,
                                              bool                            is_vopdy)
    {
        InstructionPtrs found_ptrs;

        uint64_t masked_machine_code = 0;
        uint64_t opcode_mask         = 0;
        if (is_vopdx)
        {
            assert(!is_vopdy);
            const auto& opx_iter = GetFieldIterator(encoding.microcode_format, "OPX");
            assert(opx_iter != encoding.microcode_format.bit_map.end());
            opcode_mask = GetFieldMask(*opx_iter);
        }
        else if (is_vopdy)
        {
            assert(!is_vopdx);
            const auto& opy_iter = GetFieldIterator(encoding.microcode_format, "OPY");
            assert(opy_iter != encoding.microcode_format.bit_map.end());
            opcode_mask = GetFieldMask(*opy_iter);
        }
        else
        {
            opcode_mask = encoding.opcode_mask;
        }

        masked_machine_code = first_dword & (encoding.mask | opcode_mask | encoding.seg_mask);

        // Get instruction pointer.
        if (id_to_inst.find(masked_machine_code) != id_to_inst.end())
        {
            found_ptrs.instruction_ptr = id_to_inst.at(masked_machine_code);
        }

        // Get instruction encoding pointer.
        if (id_to_inst_enc.find(masked_machine_code) != id_to_inst_enc.end())
        {
            // FIXME: Make runtime calculation on the field values.
            const auto& inst_enc_vec = id_to_inst_enc.at(masked_machine_code);
            if (inst_enc_vec.size() == 1)
            {
                found_ptrs.instruction_encoding_ptr = inst_enc_vec[0];
            }
            else
            {
                uint32_t matched_encodings = 0;
                for (const auto& inst_enc_ptr : inst_enc_vec)
                {
                    if (condition_handler.arch_conditions_.find(architecture_id) != condition_handler.arch_conditions_.end())
                    {
                        const auto& conditions = condition_handler.arch_conditions_.at(architecture_id);
                        std::string encoding_name = inst_enc_ptr->name;
                        if (encoding_name.find("ENC_") == 0)
                        {
                            encoding_name = encoding_name.substr(4);
                        }
                        encoding_name += "_" + inst_enc_ptr->condition_name;
                        if (conditions.find(encoding_name) != conditions.end())
                        {
                            auto& IsEncodingMatch = conditions.at(encoding_name);
                            if (IsEncodingMatch((static_cast<uint64_t>(second_dword) << 32) | first_dword))
                            {
                                found_ptrs.instruction_encoding_ptr = inst_enc_ptr;
                                matched_encodings++;
                            }
                        }
                    }
                }
                assert(matched_encodings == 1);
            }
        }

        return found_ptrs;
    }

    static void GetFunctionalGroupSubgroupInfo(InstructionInfo& info, const std::string& functional_group, std::string functional_subgroup)
    {
        // Assign functional group enum
        if (functional_group.compare("SALU") == 0)
        {
            info.functional_group_subgroup_info.IsaFunctionalGroup = kFunctionalGroup::kFunctionalGroupSalu;
        }
        else if (functional_group.compare("SMEM") == 0)
        {
            info.functional_group_subgroup_info.IsaFunctionalGroup = kFunctionalGroup::kFunctionalGroupSmem;
        }
        else if (functional_group.compare("VALU") == 0)
        {
            info.functional_group_subgroup_info.IsaFunctionalGroup = kFunctionalGroup::kFunctionalGroupValu;
        }
        else if (functional_group.compare("VMEM") == 0)
        {
            info.functional_group_subgroup_info.IsaFunctionalGroup = kFunctionalGroup::kFunctionalGroupVmem;
        }
        else if (functional_group.compare("EXPORT") == 0)
        {
            info.functional_group_subgroup_info.IsaFunctionalGroup = kFunctionalGroup::kFunctionalGroupExport;
        }
        else if (functional_group.compare("BRANCH") == 0)
        {
            info.functional_group_subgroup_info.IsaFunctionalGroup = kFunctionalGroup::kFunctionalGroupBranch;
        }
        else if (functional_group.compare("MESSAGE") == 0)
        {
            info.functional_group_subgroup_info.IsaFunctionalGroup = kFunctionalGroup::kFunctionalGroupMessage;
        }
        else if (functional_group.compare("WAVE_CONTROL") == 0)
        {
            info.functional_group_subgroup_info.IsaFunctionalGroup = kFunctionalGroup::kFunctionalGroupWaveControl;
        }
        else if (functional_group.compare("TRAP") == 0)
        {
            info.functional_group_subgroup_info.IsaFunctionalGroup = kFunctionalGroup::kFunctionalGroupTrap;
        }
        else if (functional_group.compare("VALU") == 0)
        {
            info.functional_group_subgroup_info.IsaFunctionalGroup = kFunctionalGroup::kFunctionalGroupValu;
        }
        else
        {
            info.functional_group_subgroup_info.IsaFunctionalGroup = kFunctionalGroup::kFunctionalGroupUnknown;
        }

        // Assign functional subgroup enum
        if (functional_group.compare("FLOATING_POINT") == 0)
        {
            info.functional_group_subgroup_info.IsaFunctionalSubgroup = kFunctionalSubgroup::kFunctionalSubgroupFloatingPoint;
        }
        else if (functional_group.compare("BUFFER") == 0)
        {
            info.functional_group_subgroup_info.IsaFunctionalSubgroup = kFunctionalSubgroup::kFunctionalSubgroupBuffer;
        }
        else if (functional_group.compare("TEXTURE") == 0)
        {
            info.functional_group_subgroup_info.IsaFunctionalSubgroup = kFunctionalSubgroup::kFunctionalSubgroupTexture;
        }
        else if (functional_group.compare("LOAD") == 0)
        {
            info.functional_group_subgroup_info.IsaFunctionalSubgroup = kFunctionalSubgroup::kFunctionalSubgroupLoad;
        }
        else if (functional_group.compare("STORE") == 0)
        {
            info.functional_group_subgroup_info.IsaFunctionalSubgroup = kFunctionalSubgroup::kFunctionalSubgroupStore;
        }
        else if (functional_group.compare("SAMPLE") == 0)
        {
            info.functional_group_subgroup_info.IsaFunctionalSubgroup = kFunctionalSubgroup::kFunctionalSubgroupSample;
        }
        else if (functional_group.compare("BVH") == 0)
        {
            info.functional_group_subgroup_info.IsaFunctionalSubgroup = kFunctionalSubgroup::kFunctionalSubgroupBvh;
        }
        else if (functional_group.compare("ATOMIC") == 0)
        {
            info.functional_group_subgroup_info.IsaFunctionalSubgroup = kFunctionalSubgroup::kFunctionalSubgroupAtomic;
        }
        else if (functional_group.compare("FLAT") == 0)
        {
            info.functional_group_subgroup_info.IsaFunctionalSubgroup = kFunctionalSubgroup::kFunctionalSubgroupFlat;
        }
        else if (functional_group.compare("DATA_SHARE") == 0)
        {
            info.functional_group_subgroup_info.IsaFunctionalSubgroup = kFunctionalSubgroup::kFunctionalSubgroupDataShare;
        }
        else if (functional_group.compare("STATIC") == 0)
        {
            info.functional_group_subgroup_info.IsaFunctionalSubgroup = kFunctionalSubgroup::kFunctionalSubgroupStatic;
        }
        else
        {
            info.functional_group_subgroup_info.IsaFunctionalSubgroup = kFunctionalSubgroup::kFunctionalSubgroupUnknown;
        }
    }

    static bool ExtractDisassembly(std::string&                                  shader_disassembly,
                                   const std::string&                            shader_disassembly_text,
                                   std::string&                                  err_message,
                                   bool                                          resolve_direct_branch_targets,
                                   std::vector<std::string>&                     pc_to_index_map,
                                   std::unordered_map<std::string, std::string>& pc_to_label_map)
    {
        bool is_success = true;

        // Parse and decode the instructions in the disassembly file
        std::stringstream shader_text(shader_disassembly_text);
        std::string       line;
        std::string       label;
        bool              is_prev_line_label = false;
        while (is_success && std::getline(shader_text, line, kShaderTextDelimiter))
        {
            line = AmdIsaUtility::Strip(line);

            // Go to the inline comment in the line read
            const char* kCodeCommentToken = "//";
            const char  kColumnToken      = ':';
            const char  kLlvmCommentToken = ';';
            std::size_t pos_comment       = line.find(kCodeCommentToken);
            std::size_t pos_comment_llvm  = line.find(kLlvmCommentToken);
            std::size_t pos_label_colon   = line.find(kColumnToken);

            // Ignore sp3 program line comments
            bool is_code_line = false;
            if (pos_comment > 0 && pos_comment != std::string::npos)
            {
                // Parse dword/s if match found
                pos_comment = line.find(kColumnToken, pos_comment);
                if (pos_comment != std::string::npos)
                {
                    // Skip through to the first dword (": ", 2 characters)
                    if ((pos_comment + 2) < line.length())
                    {
                        pos_comment += 2;
                        is_code_line = true;
                    }
                }
            }

            // Ignore the code lines with special words that look like assembly.
            is_code_line = is_code_line && (line.find(".long") == std::string::npos);
            is_code_line = is_code_line && (line.find(".ascii") == std::string::npos);
            is_code_line = is_code_line && (line.find(".byte") == std::string::npos);

            // If parsing was successful, then decode the instructions
            std::string pc;
            if (is_code_line)
            {
                // If resolve direct branch targets is enabled by the user
                if (resolve_direct_branch_targets)
                {
                    size_t location = line.find(kCodeCommentToken);
                    pc              = line.substr((location + 3), line.find(":", location) - (location + 3));
                    pc_to_index_map.push_back(pc);
                    if (is_prev_line_label)
                    {
                        is_prev_line_label  = false;
                        pc_to_label_map[pc] = label;
                    }
                }

                // Decode dword/s
                std::stringstream stream;
                stream << line.substr(pos_comment);
                shader_disassembly += stream.str() + kShaderTextDelimiter;
            }
            else
            {
                // Determine if the line-under-process is a label.
                // Save the label and set the flag for the next iteration.
                const bool is_current_line_label = (pos_comment > pos_label_colon) && (pos_comment_llvm > 0);
                if (is_current_line_label)
                {
                    is_prev_line_label = true;
                    line.pop_back();
                    label = line;
                }
            }
        }
        return is_success;
    }

    /*
     * DecodeShaderDisassembly --
     *
     * Accepts a shader disassembly text.
     *
     * Returns true if all the instructions present in the provided shader
     * disassembly text were successfully decoded, or false even if a single
     * instruction fails to be decoded - the failure reason is populated in
     * the err_message. On successful decode, the function outputs a vector
     * of InstructionInfoBundle with the decoded information of all the
     * instructions.
     *
     * Additional Option: resolve_direct_branch_targets
     * (optional, default - false and skipped)
     * Performs the branch target resolution for all direct branches. Stores
     * target PC, branch label name and target info index.
     */
    static bool DecodeShaderDisassembly(const IsaDecoder* const                             spec_decoder,
                                        const std::string&                                  shader_disassembly_text,
                                        std::vector<InstructionInfoBundle>&                 instruction_info_stream,
                                        std::string&                                        err_message,
                                        bool                                                resolve_direct_branch_targets,
                                        const std::vector<std::string>&                     pc_to_index_map,
                                        const std::unordered_map<std::string, std::string>& pc_to_label_map)
    {
        bool is_success = true;

        if (spec_decoder == nullptr)
        {
            is_success  = false;
            err_message = kStringErrorIsaDecoderNotFound;
        }

        if (is_success && resolve_direct_branch_targets && pc_to_index_map.empty() && pc_to_label_map.empty())
        {
            // Cannot perform branch target resolution since the required information is not available
            is_success  = false;
            err_message = kStringErrorMissingTargetResolutionInfo;
        }

        std::vector<uint64_t> direct_branch_indexes;
        std::stringstream     inst_stream(shader_disassembly_text);
        std::string           inst;
        while (is_success && std::getline(inst_stream, inst, kShaderTextDelimiter))
        {
            std::stringstream stream;
            stream << inst;
            static const uint8_t kDwordByte = 8;
            bool                 is_decoded = false;
            if (stream.str().length() == kDwordByte)
            {
                // Single machine code 64-bit
                std::uint64_t                 machine_code_64         = 0;
                amdisa::InstructionInfoBundle instruction_info_bundle = {};

                // Convert instruction DWORD from hex to uint64_t
                stream >> std::hex >> machine_code_64;
                is_decoded = spec_decoder->DecodeInstruction(machine_code_64, instruction_info_bundle, err_message);

                // Store the target pc for direct branch instructions, if
                // enabled
                if (is_decoded && resolve_direct_branch_targets && !instruction_info_bundle.bundle.empty() &&
                    instruction_info_bundle.bundle[0].instruction_semantic_info.branch_info.is_branch &&
                    instruction_info_bundle.bundle[0].instruction_semantic_info.branch_info.IsDirect())
                {
                    // Fetch the branch instruction's PC
                    std::stringstream pc_stream = std::stringstream(pc_to_index_map[instruction_info_stream.size()]);
                    std::uint64_t     target_pc = 0;
                    pc_stream >> std::hex >> target_pc;
                    target_pc += kInstructionBytes32 *
                                 (static_cast<uint64_t>(instruction_info_bundle.bundle[0].instruction_semantic_info.branch_info.branch_offset) + 1);

                    // Store target_pc for branch resolution
                    instruction_info_bundle.bundle[0].instruction_semantic_info.branch_info.branch_target_index = target_pc;
                    direct_branch_indexes.push_back(instruction_info_stream.size());
                }
                instruction_info_stream.push_back(instruction_info_bundle);
            }
            else if (stream.str().length() > kDwordByte)
            {
                // For 32-bit Instructions
                std::uint32_t              machine_code_32  = 0;
                std::vector<std::uint32_t> machine_codes_32 = {};

                // Pack DWORDs in an instruction stream for decoding
                while (stream >> std::hex >> machine_code_32)
                {
                    machine_codes_32.push_back(machine_code_32);
                    machine_code_32 = 0;
                }
                is_decoded = spec_decoder->DecodeInstructionStream(machine_codes_32, instruction_info_stream, err_message);
                machine_codes_32.clear();
            }
            else
            {
                is_success  = false;
                err_message = kStringErrorShaderTextInvalidDwordByte;
                err_message.append(stream.str());
            }

            if (!is_decoded)
            {
                // For debugging - Store information of instructions that failed in decoding
                amdisa::InstructionInfo ErrorInstructionInfo;
                ErrorInstructionInfo.instruction_name        = kStringNa;
                ErrorInstructionInfo.instruction_description = kStringNa;
                ErrorInstructionInfo.encoding_name           = kStringNa;
                ErrorInstructionInfo.encoding_description    = kStringNa;
                ErrorInstructionInfo.encoding_layout         = kStringNa;
                if (instruction_info_stream.empty())
                {
                    amdisa::InstructionInfoBundle ErrorInstructionBundle;
                    ErrorInstructionBundle.bundle.push_back(ErrorInstructionInfo);
                    instruction_info_stream.push_back(ErrorInstructionBundle);
                }
                else
                {
                    amdisa::InstructionInfoBundle& ErrorInstructionBundle = instruction_info_stream.back();
                    ErrorInstructionBundle.bundle.clear();
                    ErrorInstructionBundle.bundle.push_back(ErrorInstructionInfo);
                }

                std::cerr << kStringErrorShaderTextDecodeFailed << std::endl;
            }
        }

        // Replace target pc of direct branches to their respective
        // InstructionInfoBundle vector index, if enabled
        if (is_success && resolve_direct_branch_targets && !instruction_info_stream.empty())
        {
            for (const auto& index : direct_branch_indexes)
            {
                // Fetch InstructionInfo for the index
                amdisa::InstructionInfo& inst_info = instruction_info_stream[index].bundle[0];

                // Target pc conversion and lookup in pc_index_map
                std::stringstream pc_formatter;
                pc_formatter << std::uppercase << std::hex << std::setfill('0') << std::setw(12)
                             << inst_info.instruction_semantic_info.branch_info.branch_target_index;

                // Lookup for the target index in PC to Index Map
                const auto& target_index_iter = std::find(pc_to_index_map.begin(), pc_to_index_map.end(), pc_formatter.str());
                const auto& target_label_iter = pc_to_label_map.find(pc_formatter.str());
                const bool  can_access        = (target_index_iter != pc_to_index_map.end()) && (target_label_iter != pc_to_label_map.end());
                if (can_access)
                {
                    inst_info.instruction_semantic_info.branch_info.branch_target_pc    = pc_formatter.str();
                    inst_info.instruction_semantic_info.branch_info.branch_target_index = uint64_t(target_index_iter - pc_to_index_map.begin());
                    inst_info.instruction_semantic_info.branch_info.branch_target_label = target_label_iter->second;
                }
                else
                {
                    is_success  = false;
                    err_message = kStringErrorBranchTargetInfoNotFound;
                    if (target_index_iter != pc_to_index_map.end())
                    {
                        err_message += kStringErrorBranchTargetIndexNotFound + pc_formatter.str();
                    }
                    if (target_label_iter != pc_to_label_map.end())
                    {
                        err_message += kStringErrorBranchTargetLabelNotFound + pc_formatter.str();
                    }
                    break;
                }
            }
        }

        if (is_success && instruction_info_stream.empty())
        {
            is_success  = false;
            err_message = kStringErrorShaderTextDecodeNoInst;
        }

        return is_success;
    }

    // *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - END ***

    // API implementation definititon.
    class IsaDecoder::IsaDecoderImpl
    {
    public:
        IsaDecoderImpl() = default;
        IsaSpec& GetSpec()
        {
            return spec_data_;
        }

        bool IsInitialized() const
        {
            return is_initialized_;
        }

        void SetInitialized(bool is_initialized)
        {
            is_initialized_ = is_initialized;
        }

        void MapIdentifierToEncoding(uint64_t identifier, std::shared_ptr<Encoding> encoding_ptr)
        {
            if (identifier_to_encoding_.find(identifier) == identifier_to_encoding_.end())
            {
                identifier_to_encoding_[identifier] = encoding_ptr;
            }
        }

        IdToEncoding& GetEncodingMap()
        {
            return identifier_to_encoding_;
        }

        void MapIdentifierToInstruction(uint64_t identifier, std::shared_ptr<Instruction> instr_ptr)
        {
            if (identifier_to_instruction_.find(identifier) == identifier_to_instruction_.end())
            {
                identifier_to_instruction_[identifier] = instr_ptr;
            }
            else
            {
                if (identifier_to_instruction_[identifier]->name != instr_ptr->name)
                {
                    std::cout << "Warning: Id is mapping to the same instruction." << std::endl;
                }
            }
        }

        IdToInstruction& GetInstructionMap()
        {
            return identifier_to_instruction_;
        }

        void MapIdentifierToInstructionEncoding(uint64_t identifier, std::shared_ptr<InstructionEncoding> instr_enc_ptr)
        {
            auto&       instr_enc_vec  = identifier_to_instruction_encoding_vec_[identifier];
            const auto& instr_enc_iter = std::find_if(instr_enc_vec.begin(), instr_enc_vec.end(), [&](std::shared_ptr<InstructionEncoding> pushed_instr_enc) {
                return (pushed_instr_enc->name == instr_enc_ptr->name)
                    && (pushed_instr_enc->condition_name == instr_enc_ptr->condition_name);
            });
            if (instr_enc_iter == instr_enc_vec.end())
            {
                identifier_to_instruction_encoding_vec_[identifier].push_back(instr_enc_ptr);
            }
            else if ((instr_enc_ptr->name.find("VOPD") == std::string::npos) && (instr_enc_ptr->name.find("FLAT") == std::string::npos))
            {
                std::cout << "Warning: Id is mapping the same instruction encoding." << std::endl;
            }
        }

        IdToInstructionEncodings& GetInstructionEncodingMap()
        {
            return identifier_to_instruction_encoding_vec_;
        }

        EncodingConditionHandler& GetEncodingConditionHandler()
        {
            return condition_handler_;
        }

    private:
        // Internal representation of the spec.
        IsaSpec spec_data_;

        // Is set to true once XML was successfully read.
        bool is_initialized_ = false;

        // Map from id to the encoding spec, provides constant time access to the encodings.
        IdToEncoding identifier_to_encoding_;

        // Map from id to the instruction spec, provides constant time access to the instructions.
        IdToInstruction identifier_to_instruction_;

        // Map from id to the vector of operand layout spec, provides constant time access to the operand layouts.
        IdToInstructionEncodings identifier_to_instruction_encoding_vec_;

        // Encoding conditions handler. The class contains functions that are needed for runtime
        // evaluation of encodings. For example, should we pick an encoding with literal, dpp or sdwa.
        EncodingConditionHandler condition_handler_;
    };

    bool IsaDecoder::Initialize(const std::string& input_xml_file_path, std::string& err_message)
    {
        bool is_xml_read_successful = true;

        // Allocate implementation.
        if (api_impl_ == nullptr)
        {
            api_impl_ = new IsaDecoderImpl();
        }
        else
        {
            delete api_impl_;
            api_impl_ = new IsaDecoderImpl();
        }

        if (api_impl_ == nullptr)
        {
            is_xml_read_successful = false;
            err_message            = kStringErrorApiImplAllocationFailed;
        }

        // Read spec.
        if (is_xml_read_successful)
        {
            is_xml_read_successful = IsaXmlReader::ReadSpec(input_xml_file_path, api_impl_->GetSpec(), err_message);
            api_impl_->SetInitialized(is_xml_read_successful);
        }

        // Check version.
        if (is_xml_read_successful)
        {
            std::string& xml_schema_version = api_impl_->GetSpec().info.schema_version;
            if (xml_schema_version > kMaxSupportedSchemaVersion)
            {
                is_xml_read_successful = false;
                std::stringstream err_stream;
                err_stream << kStringErrorUnsupportedXml << " ";
                err_stream << "Version of XML being read: " << xml_schema_version << ". ";
                err_stream << "API supports version " << kMaxSupportedSchemaVersion << " and lower.";
                err_message = err_stream.str();
            }
        }

        if (is_xml_read_successful)
        {
            const IsaSpec& spec_data = api_impl_->GetSpec();
            
            // Map identifiers to encodings.
            for (const Encoding& encoding : spec_data.encodings)
            {
                if (encoding.bit_count <= 64)
                {
                    for (uint64_t identifier : encoding.identifiers)
                    {
                        api_impl_->MapIdentifierToEncoding(identifier, std::make_shared<Encoding>(encoding));
                    }
                }
            }

            // Map identifiers to instructions.
            for (const Instruction& instruction : spec_data.instructions)
            {
                for (uint64_t encoding_itr = 0; (is_xml_read_successful && encoding_itr < instruction.encodings.size()); encoding_itr++)
                {
                    const InstructionEncoding& instruction_encoding = instruction.encodings[encoding_itr];
                    // VOPD handling, determine if x or y layout.
                    bool is_x_layout = IsOperandPresent(instruction_encoding, "SRCX0");
                    bool is_y_layout = IsOperandPresent(instruction_encoding, "SRCY0");

                    // Form encoding identifier.
                    const auto& found_encoding_iterator = std::find_if(spec_data.encodings.begin(), spec_data.encodings.end(), [&](const Encoding& encoding) {
                        return encoding.name == instruction_encoding.name;
                    });
                    assert(found_encoding_iterator != spec_data.encodings.end());

                    const auto& microcode    = found_encoding_iterator->microcode_format;
                    const auto& op_iter      = GetFieldIterator(microcode, "OP");
                    const auto& opx_iter     = GetFieldIterator(microcode, "OPX");
                    const auto& opy_iter     = GetFieldIterator(microcode, "OPY");
                    const auto& seg_iter     = GetFieldIterator(microcode, "SEG");
                    bool        is_op_found  = op_iter != microcode.bit_map.end();
                    bool        is_opx_found = opx_iter != microcode.bit_map.end();
                    bool        is_opy_found = opy_iter != microcode.bit_map.end();
                    bool        is_seg_found = seg_iter != microcode.bit_map.end();
                    uint64_t    identifier   = found_encoding_iterator->bits;

                    Range range;
                    if (is_op_found)
                    {
                        if (!AmdIsaUtility::GetRange(*op_iter, range))
                        {
                            is_xml_read_successful = false;
                            err_message            = kStringErrorEmptyRangesInField + op_iter->name;
                        }
                        else
                        {
                            const uint64_t positioned_op = static_cast<uint64_t>(instruction_encoding.opcode) << range.bit_offset;
                            identifier |= positioned_op;
                            if (is_seg_found)
                            {
                                uint64_t positioned_seg = 0;
                                if (kSegBits.find(instruction_encoding.name) != kSegBits.end())
                                {
                                    if (!AmdIsaUtility::GetRange(*seg_iter, range))
                                    {
                                        is_xml_read_successful = false;
                                        err_message            = kStringErrorEmptyRangesInField + seg_iter->name;
                                    }
                                    else
                                    {
                                        positioned_seg = (static_cast<uint64_t>(kSegBits.at(instruction_encoding.name)) << range.bit_offset);
                                        identifier |= positioned_seg;
                                    }
                                }
                                else
                                {
                                    std::cout << "Warning: Marked with SEG field, but doesn't match with constants." << std::endl;
                                }
                            }
                        }
                    }
                    else if (is_x_layout)
                    {
                        assert(is_opx_found && !is_y_layout);
                        if (!AmdIsaUtility::GetRange(*opx_iter, range))
                        {
                            is_xml_read_successful = false;
                            err_message            = kStringErrorEmptyRangesInField + opx_iter->name;
                        }
                        else
                        {
                            const uint64_t positioned_opx = static_cast<uint64_t>(instruction_encoding.opcode) << range.bit_offset;
                            identifier |= positioned_opx;
                        }
                    }
                    else if (is_y_layout)
                    {
                        assert(is_opy_found && !is_x_layout);
                        if (!AmdIsaUtility::GetRange(*opy_iter, range))
                        {
                            is_xml_read_successful = false;
                            err_message            = kStringErrorEmptyRangesInField + opy_iter->name;
                        }
                        else
                        {
                            const uint64_t positioned_opy = static_cast<uint64_t>(instruction_encoding.opcode) << range.bit_offset;
                            identifier |= positioned_opy;
                        }
                    }
                    else
                    {
                        assert(instruction.name.find("EXP") != std::string::npos);
                    }

                    // Map.
                    api_impl_->MapIdentifierToInstruction(identifier, std::make_shared<Instruction>(instruction));
                    api_impl_->MapIdentifierToInstructionEncoding(identifier, std::make_shared<InstructionEncoding>(instruction_encoding));
                }
            }
        }

        return is_xml_read_successful && (api_impl_ != nullptr);
    }

    std::string IsaDecoder::GetVersion() const
    {
        return amdisa::ApiVersion::GetVersion();
    }

    bool IsaDecoder::DecodeShaderDisassemblyText(const std::string&                  shader_disassembly_text,
                                                 std::vector<InstructionInfoBundle>& instruction_info_stream,
                                                 std::string&                        err_message,
                                                 bool                                resolve_direct_branch_targets) const
    {
        bool        is_success = true;
        std::string disassembly_text;

        // PC and index pair map for branch target resolution
        // Indexes are of instruction_info_stream
        std::vector<std::string> pc_to_index_map = {};

        // PC and label pair.
        std::unordered_map<std::string, std::string> pc_to_label_map = {};

        if (is_success)
        {
            is_success =
                ExtractDisassembly(disassembly_text, shader_disassembly_text, err_message, resolve_direct_branch_targets, pc_to_index_map, pc_to_label_map);
        }

        // Decode the extracted instructions
        if (is_success)
        {
            is_success = DecodeShaderDisassembly(
                this, disassembly_text, instruction_info_stream, err_message, resolve_direct_branch_targets, pc_to_index_map, pc_to_label_map);
        }
        return is_success;
    }

    bool IsaDecoder::DecodeShaderDisassemblyFile(const std::string&                  shader_disassembly_file,
                                                 std::vector<InstructionInfoBundle>& instruction_info_stream,
                                                 std::string&                        err_message,
                                                 bool                                resolve_direct_branch_targets) const
    {
        bool is_success = true;

        // PC and index pair map for branch target resolution
        // Indexes are of instruction_info_stream
        std::vector<std::string> pc_to_index_map = {};

        // PC and label pair.
        std::unordered_map<std::string, std::string> pc_to_label_map = {};

        // Read from file and prepare for extract
        std::string shader_disassembly_file_text;
        try
        {
            std::ifstream disassembly_file;
            disassembly_file.open(shader_disassembly_file, std::ios::in);
            if (disassembly_file.is_open())
            {
                while (!disassembly_file.eof() && is_success)
                {
                    std::string line;
                    getline(disassembly_file, line);
                    line = AmdIsaUtility::Strip(line);
                    shader_disassembly_file_text += (line + kShaderTextDelimiter);
                }
            }
        }
        catch (const std::exception& e)
        {
            is_success = false;
            err_message.append(e.what());
        }

        // Parse the instructions from the shader disassembly file
        std::string shader_disassembly_text;
        if (is_success)
        {
            is_success = ExtractDisassembly(
                shader_disassembly_text, shader_disassembly_file_text, err_message, resolve_direct_branch_targets, pc_to_index_map, pc_to_label_map);
        }

        // Decode the extracted instructions
        if (is_success)
        {
            is_success = DecodeShaderDisassembly(
                this, shader_disassembly_text, instruction_info_stream, err_message, resolve_direct_branch_targets, pc_to_index_map, pc_to_label_map);
        }
        return is_success;
    }

    bool IsaDecoder::DecodeInstructionStream(const std::vector<uint32_t>&        machine_code_stream,
                                             std::vector<InstructionInfoBundle>& instruction_info_stream,
                                             std::string&                        err_message) const
    {
        bool is_decode_failed = true;

        bool is_api_init = false;
        if (api_impl_ != nullptr)
        {
            is_api_init = api_impl_->IsInitialized();
        }

        if (is_api_init)
        {
            const amdisa::IsaSpec& spec_data = api_impl_->GetSpec();

            MachineCodeStream stream;
            stream.Init(machine_code_stream);
            std::vector<uint32_t> working_dwords;
            is_decode_failed = false;
            while (!is_decode_failed && !stream.IsEmpty())
            {
                // Get words from the machine code stream for the current instruction.
                working_dwords.clear();
                working_dwords.push_back(stream.GetNextDword());

                // Get the encoding.
                auto encoding_ptr = GetEncodingIterator(api_impl_->GetEncodingMap(), working_dwords[0], spec_data);

                if (encoding_ptr != nullptr)
                {
                    // Insert one instruction info group.
                    instruction_info_stream.push_back(InstructionInfoBundle());
                    auto& instruction_info_bundle = instruction_info_stream.back();

                    // Get next dword from instruction stream for wider encodings.
                    while (working_dwords.size() * 32 < encoding_ptr->bit_count)
                    {
                        working_dwords.push_back(stream.GetNextDword());
                    }

                    // VOPD encoding requires special handling due to two opcodes.
                    uint32_t opcode_count      = 1;
                    bool     is_vopd_encoding  = false;
                    bool     is_smem_encoding  = false;
                    bool     is_mubuf_encoding = false;
                    bool     is_flat_encoding  = false;
                    if (encoding_ptr->name.find("VOPD") != std::string::npos)
                    {
                        opcode_count     = 2;
                        is_vopd_encoding = true;
                    }
                    else if (encoding_ptr->name.find("SMEM") != std::string::npos)
                    {
                        is_smem_encoding = true;
                    }
                    else if (encoding_ptr->name.find("MUBUF") != std::string::npos)
                    {
                        is_mubuf_encoding = true;
                    }

                    // Dual Ops common src_literal case
                    bool        is_prev_operand_lit = false;
                    std::string prev_lit_operand_name;
                    while (opcode_count > 0)
                    {
                        // Determine the name of the OP field based on the encoding.
                        std::string opcode_field_name = "OP";
                        bool        is_vopdx          = false;
                        bool        is_vopdy          = false;
                        if (is_vopd_encoding)
                        {
                            if (opcode_count == 2)
                            {
                                opcode_field_name = "OPX";
                                is_vopdx          = true;
                            }
                            else
                            {
                                opcode_field_name = "OPY";
                                is_vopdy          = true;
                            }
                        }
                        --opcode_count;

                        // Get the opcode.
                        uint64_t    opcode_value   = 0;
                        const auto& field_iterator = GetFieldIterator(working_dwords, opcode_field_name, encoding_ptr->microcode_format, opcode_value);

                        // EXP encoding has no OP field.
                        bool is_exp                  = encoding_ptr->name.find("EXP") != std::string::npos;
                        bool is_retrieval_successful = (field_iterator != encoding_ptr->microcode_format.bit_map.end() || is_exp);

                        // Get the instruction name.
                        if (is_retrieval_successful)
                        {
                            instruction_info_bundle.bundle.push_back(InstructionInfo());
                            auto& instruction_info = instruction_info_bundle.bundle.back();

                            // Get the modifiers.
                            bool is_extracted = ExtractModifiers(working_dwords, *encoding_ptr, instruction_info.operand_modifiers, err_message);
                            if (is_extracted)
                            {
                                // Get the fields info into the container.
                                RetrieveFieldInfo(working_dwords, encoding_ptr->microcode_format.bit_map, instruction_info);
                                uint32_t second_dword = 0;
                                if (working_dwords.size() > 1)
                                {
                                    second_dword = working_dwords[1];
                                }
                                const auto& instruction_ptrs = GetInstructionPtrs(spec_data.architecture.id,
                                                                                  api_impl_->GetInstructionMap(),
                                                                                  api_impl_->GetInstructionEncodingMap(),
                                                                                  *encoding_ptr,
                                                                                  api_impl_->GetEncodingConditionHandler(),
                                                                                  working_dwords[0],
                                                                                  second_dword,
                                                                                  is_vopdx,
                                                                                  is_vopdy);

                                // Get the conditional encoding.
                                if (encoding_ptr->name != instruction_ptrs.instruction_encoding_ptr->name)
                                {
                                    const auto& encoding_iter = std::find_if(spec_data.encodings.begin(), spec_data.encodings.end(), [&](const Encoding& encoding) {
                                        return encoding.name == instruction_ptrs.instruction_encoding_ptr->name;
                                    });
                                    assert(encoding_iter != spec_data.encodings.end());
                                    encoding_ptr = std::make_shared<Encoding>(*encoding_iter);

                                    // Retrieve all required dwords from the instruction stream.
                                    while (working_dwords.size() * 32 < encoding_ptr->bit_count)
                                    {
                                        working_dwords.push_back(stream.GetNextDword());
                                    }
                                }

                                // Encoding info retrieve successfully -- save return values.
                                instruction_info.encoding_description = encoding_ptr->description;
                                instruction_info.encoding_name = encoding_ptr->name;

                                // Get the fields info in a formated single string for printing.
                                amdisa::AmdIsaUtility::BitMapToString(encoding_ptr->microcode_format, working_dwords, instruction_info.encoding_layout);

                                // Find the instruction.
                                bool is_instruction_retrieval_successful =
                                    instruction_ptrs.instruction_encoding_ptr != nullptr && instruction_ptrs.instruction_ptr != nullptr;

                                if (is_instruction_retrieval_successful)
                                {
                                    // Dereference pointers for local use.
                                    const InstructionEncoding& inst_enc = *(instruction_ptrs.instruction_encoding_ptr);
                                    const Instruction&         inst     = *(instruction_ptrs.instruction_ptr);

                                    // Instruction info retrieved successfully -- save return values.
                                    instruction_info.instruction_name                                     = inst.name;
                                    instruction_info.aliased_names                                        = inst.aliased_names;
                                    instruction_info.instruction_semantic_info.branch_info.is_branch      = inst.is_branch;
                                    instruction_info.instruction_semantic_info.branch_info.is_conditional = inst.is_conditional_branch;
                                    instruction_info.instruction_semantic_info.branch_info.is_indirect    = inst.is_indirect_branch;
                                    instruction_info.instruction_semantic_info.is_immediately_executed    = inst.is_immediately_executed;
                                    instruction_info.instruction_semantic_info.is_program_terminator      = inst.is_program_terminator;
                                    instruction_info.instruction_description                              = inst.description;

                                    // Get Functional Group and Subgroup Information
                                    GetFunctionalGroupSubgroupInfo(instruction_info, inst.functional_group_name, inst.functional_subgroup_name);

                                    // Get Functional Group Description
                                    instruction_info.functional_group_subgroup_info.description = "Functional group description not found!";
                                    for (auto group_itr = spec_data.functional_group_info.begin(); group_itr != spec_data.functional_group_info.end();
                                         ++group_itr)
                                    {
                                        if (group_itr->name.compare(inst.functional_group_name) == 0)
                                        {
                                            instruction_info.functional_group_subgroup_info.description = group_itr->desc;
                                            break;
                                        }
                                    }

                                    // Get the operands.
                                    Encoding encoding = *encoding_ptr;
                                    for (auto operands_iterator = inst_enc.operands.begin(); !is_decode_failed && operands_iterator != inst_enc.operands.end();
                                         ++operands_iterator)
                                    {
                                        // Dereference the iterator.
                                        const auto& operand = *operands_iterator;

                                        // No need to process implicit operands as they are not in the
                                        // machine code.
                                        if (!operand.is_implicit)
                                        {
                                            std::string field_name = operand.encoding_field_name;

                                            // Convert field name to upper case.
                                            field_name = AmdIsaUtility::ToUpper(field_name);

                                            bool is_implied_literal = false;
                                            if (encoding.name.find("LITERAL") != std::string::npos && field_name.empty())
                                            {
                                                field_name         = "SIMM32";
                                                is_implied_literal = true;
                                            }

                                            // Get the value of the field.
                                            uint64_t    field_value    = 0;
                                            const auto& field_iterator = GetFieldIterator(working_dwords, field_name, encoding.microcode_format, field_value);
                                            bool        is_field_found = field_iterator != encoding.microcode_format.bit_map.end();

                                            // Known cases when the field name may not be present in the
                                            // encoding.
                                            bool is_operand_literal = false;
                                            if (!is_field_found)
                                            {
                                                if (field_name == "LITERAL")
                                                {
                                                    is_field_found     = true;
                                                    is_operand_literal = true;
                                                }
                                                else if (!operand.is_in_microcode)
                                                {
                                                    is_field_found = true;
                                                }
                                                else
                                                {
                                                    assert(false);
                                                }
                                            }

                                            // Get the operand type.
                                            const auto& operand_type_iterator = std::find_if(
                                                spec_data.operand_types.begin(), spec_data.operand_types.end(), [&](const OperandType& operand_type) {
                                                    return operand_type.name == operand.type;
                                                });

                                            bool is_operand_retrieval_successful = is_field_found && operand_type_iterator != spec_data.operand_types.end();

                                            // Get the operand names.
                                            if (is_operand_retrieval_successful)
                                            {
                                                instruction_info.instruction_operands.push_back(InstructionOperand());
                                                auto& instruction_operand        = instruction_info.instruction_operands.back();
                                                instruction_operand.is_input     = operand.input;
                                                instruction_operand.is_output    = operand.output;
                                                instruction_operand.operand_size = operand.size;

                                                const auto& predefined_value_iterator = std::find_if(
                                                    operand_type_iterator->predefined_values.begin(),
                                                    operand_type_iterator->predefined_values.end(),
                                                    [&](const PredefinedValue& predefined_value) { return predefined_value.value == field_value; });

                                                // Save branch offset if branch.
                                                if (instruction_info.instruction_semantic_info.branch_info.is_branch)
                                                {
                                                    if (instruction_info.instruction_semantic_info.branch_info.IsDirect())
                                                    {
                                                        instruction_info.instruction_semantic_info.branch_info.branch_offset =
                                                            static_cast<int16_t>(field_value);
                                                    }
                                                }

                                                if (field_name.find("SIMM") == 0 && encoding.name.find("LITERAL") != std::string::npos)
                                                {
                                                    std::stringstream formatter;
                                                    formatter << std::hex << "lit(0x" << field_value << ")";
                                                    instruction_operand.operand_name = formatter.str();
                                                }
                                                else if (predefined_value_iterator != operand_type_iterator->predefined_values.end())
                                                {
                                                    if (!predefined_value_iterator->name.empty())
                                                    {
                                                        // Get operand name. Expand the name to range format if operand size
                                                        // is greater than 32 bits.
                                                        const std::string operand_name   = predefined_value_iterator->name;
                                                        instruction_operand.operand_name = operand_name;
                                                        if (operand_name.length() > 1)
                                                        {
                                                            const bool is_size_wave_dependent = (operand.data_format == kWaveDependentFormat) ||
                                                                                                (is_mubuf_encoding && operand.encoding_field_name == "VADDR");
                                                            const bool is_next_digit = std::isdigit(static_cast<uint8_t>(operand_name[1]));
                                                            const bool is_sgpr       = (operand_name[0] == 's') && (is_next_digit);
                                                            const bool is_vgpr       = (operand_name[0] == 'v') && (is_next_digit);
                                                            if (!is_size_wave_dependent && (instruction_operand.operand_size > kDwordSize) &&
                                                                (is_sgpr || is_vgpr))
                                                            {
                                                                instruction_operand.operand_name =
                                                                    GetNameAsRegisterRange(operand_name, instruction_operand.operand_size);
                                                            }
                                                        }
                                                        // Add constant offset modifier for SMEM encoding instructions.
                                                        if ((is_smem_encoding || is_mubuf_encoding) && field_name == "SOFFSET")
                                                        {
                                                            uint64_t    const_offset = 0;
                                                            const auto& field_iterator =
                                                                GetFieldIterator(working_dwords, "OFFSET", encoding.microcode_format, const_offset);
                                                            if (const_offset > 0)
                                                            {
                                                                std::stringstream formatter;
                                                                formatter << predefined_value_iterator->name << " offset:0x" << std::hex << const_offset;
                                                                instruction_operand.operand_name = formatter.str();
                                                            }
                                                        }
                                                    }
                                                    else
                                                    {
                                                        instruction_operand.operand_name = std::to_string(predefined_value_iterator->value);
                                                    }
                                                }
                                                else if (operand.type.find("VCC") != std::string::npos)
                                                {
                                                    instruction_operand.operand_name = "vcc";
                                                }
                                                else if (operand.type.find("EXEC") != std::string::npos)
                                                {
                                                    instruction_operand.operand_name = "exec";
                                                }
                                                else if (operand_type_iterator->is_partitioned)
                                                {
                                                    assert(field_value <= UINT32_MAX);
                                                    instruction_operand.operand_name =
                                                        GeneratePartitionedOperand(operand_type_iterator->microcode_format, static_cast<uint32_t>(field_value));
                                                    if (is_implied_literal)
                                                    {
                                                        is_prev_operand_lit   = true;
                                                        prev_lit_operand_name = instruction_operand.operand_name;
                                                    }
                                                }
                                                else
                                                {
                                                    // Return raw bit value if no predefined values were
                                                    if (!is_operand_literal)
                                                    {
                                                        std::stringstream formatter;
                                                        formatter << std::hex << "0x" << field_value;
                                                        instruction_operand.operand_name = formatter.str();
                                                    }
                                                }

                                                // Handle  the implied literal case and the source literal case.
                                                if (is_operand_literal || instruction_operand.operand_name == "src_literal")
                                                {
                                                    if (!is_operand_literal)
                                                    {
                                                        assert(field_value == 255);
                                                    }
                                                    if (is_vopd_encoding && opcode_count == 0 && is_prev_operand_lit)
                                                    {
                                                        instruction_operand.operand_name = prev_lit_operand_name;

                                                        // Set to false since the second iteration of decoding
                                                        // dual operations type instructions is complete.
                                                        // opcode_count == 0
                                                        is_prev_operand_lit = false;
                                                    }
                                                    else
                                                    {
                                                        std::stringstream formatter;
                                                        uint32_t lit = working_dwords[working_dwords.size() - 1];
                                                        if (encoding.name.find("LITERAL") == std::string::npos)
                                                        {
                                                            lit = stream.GetNextDword();
                                                        }
                                                        formatter << "lit(0x" << std::hex << lit << ")";
                                                        instruction_operand.operand_name = formatter.str();
                                                        is_prev_operand_lit              = true;
                                                        prev_lit_operand_name            = instruction_operand.operand_name;
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                is_decode_failed = true;
                                                err_message      = kStringErrorFailedToDecodeOperands;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    is_decode_failed = true;
                                    std::stringstream err_stream;
                                    err_stream << kStringErrorInstructionNotFoundInSpec;
                                    if (instruction_ptrs.instruction_encoding_ptr == nullptr)
                                    {
                                        err_stream << " " << kStringErrorEncodingNotFound;
                                    }
                                    err_stream << kStringErrorEncodingNotFound;
                                    for (const auto& kDword : working_dwords)
                                    {
                                        err_stream << std::hex << "0x" << kDword << " ";
                                    }
                                    err_message = err_stream.str();
                                }

                                // Apply modifiers on the assembly operands.
                                uint32_t neg_modifier_value = 0;
                                uint32_t abs_modifier_value = 0;
                                uint32_t offset_value       = 0;
                                for (const auto& operand_modifier : instruction_info.operand_modifiers)
                                {
                                    // Negation.
                                    if (operand_modifier.modifier_name.find(kModifierNameNegation) != std::string::npos)
                                    {
                                        neg_modifier_value = operand_modifier.value;
                                    }
                                    else if (operand_modifier.modifier_name.find(kModifierNameAbsoluteValue) != std::string::npos)
                                    {
                                        abs_modifier_value = operand_modifier.value;
                                    }
                                    else if (operand_modifier.modifier_name.find(kModifierNameOffset) != std::string::npos)
                                    {
                                        offset_value = operand_modifier.value;
                                    }
                                }

                                uint32_t check_bit_pos = 1;
                                for (uint32_t i = 0; i < instruction_info.instruction_operands.size(); i++)
                                {
                                    InstructionOperand& current_operand = instruction_info.instruction_operands[i];
                                    if (current_operand.is_input)
                                    {
                                        bool is_neg_bit_set = (neg_modifier_value & check_bit_pos) > 0;
                                        bool is_abs_bit_set = (abs_modifier_value & check_bit_pos) > 0;
                                        if (is_neg_bit_set)
                                        {
                                            current_operand.operand_name = "-" + current_operand.operand_name;
                                        }
                                        if (is_abs_bit_set)
                                        {
                                            current_operand.operand_name = "abs(" + current_operand.operand_name + ")";
                                        }

                                        check_bit_pos <<= 1;
                                    }
                                }
                            }
                            else
                            {
                                is_decode_failed = true;
                                err_message      = kStringErrorModiferFieldNotFound;
                            }
                        }
                        else
                        {
                            is_decode_failed = true;
                        }
                    }

                    // Pop the inserted InstructionGroup if the decode failed.
                    if (is_decode_failed)
                    {
                        instruction_info_stream.pop_back();
                    }
                }
                else
                {
                    is_decode_failed = true;
                    std::stringstream err_stream;
                    err_stream << kStringErrorEncodingNotFound;
                    for (const auto& kDword : working_dwords)
                    {
                        err_stream << std::hex << "0x" << kDword << " ";
                    }
                    err_message = err_stream.str();
                }
            }
        }
        else
        {
            err_message = kStringErrorSpecNotInitialized;
        }

        return !is_decode_failed;
    }

    bool IsaDecoder::DecodeInstruction(uint64_t machine_code, InstructionInfoBundle& instruction_info_bundle, std::string& err_message) const
    {
        // Pack into stream.
        std::vector<uint32_t> machine_code_stream;
        while (machine_code > 0)
        {
            machine_code_stream.push_back(machine_code & kDwordMask);
            machine_code = machine_code >> 32;
        }

        // Decode the stream.
        std::vector<InstructionInfoBundle> instruction_info_stream;
        bool                               is_decoded = DecodeInstructionStream(machine_code_stream, instruction_info_stream, err_message);
        assert(is_decoded);
        if (is_decoded)
        {
            if (instruction_info_stream.empty())
            {
                is_decoded  = false;
                err_message = kStringErrorInstructionNotFoundInSpec;
            }
            else
            {
                instruction_info_bundle = instruction_info_stream[0];
            }
        }

        return is_decoded;
    }

    bool IsaDecoder::DecodeInstruction(const std::string& instruction_name, InstructionInfo& instruction_info, std::string& err_message) const
    {
        bool is_retrieval_failed = false;

        bool is_api_init = false;
        if (api_impl_ != nullptr)
        {
            is_api_init = api_impl_->IsInitialized();
        }
        else
        {
            is_retrieval_failed = true;
        }

        if (is_api_init)
        {
            const amdisa::IsaSpec& spec_data = api_impl_->GetSpec();

            // Convert to upper case.
            std::string instruction_name_all_caps = AmdIsaUtility::ToUpper(instruction_name);

            // Find the instruction by name.
            const auto& instructions_iterator = std::find_if(spec_data.instructions.begin(), spec_data.instructions.end(), [&](const Instruction& instruction) {
                return instruction.name == instruction_name_all_caps;
            });

            if (instructions_iterator != spec_data.instructions.end())
            {
                // Instruction found -- save return values.
                instruction_info.instruction_name        = instructions_iterator->name;
                instruction_info.instruction_description = instructions_iterator->description;
            }
            else
            {
                is_retrieval_failed = true;
                err_message         = kStringErrorInstructionNotFoundInSpec + instruction_name;
            }
        }
        else
        {
            err_message = kStringErrorSpecNotInitialized;
        }

        return !is_retrieval_failed;
    }

    GpuArchitecture IsaDecoder::GetArchitecture() const
    {
        GpuArchitecture ret = GpuArchitecture::kUnknown;
        auto arch_enum_iter = kArchitectureIdToEnum.find(api_impl_->GetSpec().architecture.id);
        if (arch_enum_iter != kArchitectureIdToEnum.end())
        {
            ret = arch_enum_iter->second;
        }
        return ret;
    }

    IsaDecoder::~IsaDecoder()
    {
        if (api_impl_ != nullptr)
        {
            delete api_impl_;
            api_impl_ = nullptr;
        }
    }

    // Decoder manager implementation.
    struct DecodeManager::DecodeManagerImpl
    {
        std::map<GpuArchitecture, std::shared_ptr<IsaDecoder>> arch_to_decoder;
    };

    bool DecodeManager::Initialize(const std::vector<std::string>& input_spec_file_paths, std::string& err_message)
    {
        bool should_abort = false;

        if (manager_impl_ == nullptr)
        {
            manager_impl_ = new DecodeManagerImpl();
        }
        else
        {
            delete manager_impl_;
            manager_impl_ = new DecodeManagerImpl();
        }

        if (manager_impl_ == nullptr)
        {
            should_abort = true;
            err_message  = kStringErrorManagerImplAllocationFailed;
        }

        // Initialize IsaDecoder for each XML spec paths provided.
        for (auto path_iter = input_spec_file_paths.begin(); !should_abort && path_iter != input_spec_file_paths.end(); path_iter++)
        {
            std::string                 init_err_message;
            std::shared_ptr<IsaDecoder> decoder         = std::make_shared<IsaDecoder>();
            bool                        is_decoder_init = decoder->Initialize(*path_iter, init_err_message);
            if (is_decoder_init)
            {
                GpuArchitecture architecture = decoder->GetArchitecture();
                if (architecture != GpuArchitecture::kUnknown)
                {
                    manager_impl_->arch_to_decoder[architecture] = decoder;
                }
                else
                {
                    should_abort = true;
                    err_message  = kStringErrorDecodeManagerUnknownArch;
                }
            }
            else
            {
                should_abort = true;
                std::stringstream error_ss;
                error_ss << kStringErrorDecodeManagerInitFailed << *path_iter;
                err_message = error_ss.str();
            }
        }

        return !should_abort;
    }

    std::shared_ptr<IsaDecoder> DecodeManager::GetDecoder(GpuArchitecture architecture) const
    {
        std::shared_ptr<IsaDecoder> ret = nullptr;

        // Get the decoder from the map.
        auto decoder_iter = manager_impl_->arch_to_decoder.find(architecture);
        if (decoder_iter != manager_impl_->arch_to_decoder.end())
        {
            ret = decoder_iter->second;
        }
        return ret;
    }

    DecodeManager::~DecodeManager()
    {
        if (manager_impl_ != nullptr)
        {
            delete manager_impl_;
            manager_impl_ = nullptr;
        }
    }

    bool BranchInfo::IsDirect() const
    {
        return !is_indirect;
    }

}  // namespace amdisa
