/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#ifndef ISA_DECODER_H_
#define ISA_DECODER_H_

// C++ libraries.
#include <memory>
#include <string>
#include <vector>

namespace amdisa
{
    // Enumuartions of supported architectures.
    enum class GpuArchitecture
    {
        kUnknown,
        kRdna1,
        kRdna2,
        kRdna3,
        kCdna1,
        kCdna2,
        kCdna3
    };

    // Constants.
    static const uint32_t kBranchOffsetOutOfRange = (1 << 16);
    static const uint64_t kInvalidBranchTarget    = UINT64_MAX;

    // Instruction's functional group.
    enum class kFunctionalGroup
    {
        kFunctionalGroupUnknown,      // Unknown
        kFunctionalGroupSalu,         // Scalar ALU
        kFunctionalGroupSmem,         // Scalar Memory
        kFunctionalGroupValu,         // Vector ALU
        kFunctionalGroupVmem,         // Vector Memory
        kFunctionalGroupExport,       // Export
        kFunctionalGroupBranch,       // Branch
        kFunctionalGroupMessage,      // Message
        kFunctionalGroupWaveControl,  // Wave Control
        kFunctionalGroupTrap          // Trap
    };

    static const char* kFunctionalGroupName[] = {
        "Unknown",        // Unknown
        "Scalar ALU",     // Scalar ALU
        "Scalar Memory",  // Scalar Memory
        "Vector ALU",     // Vector ALU
        "Vector Memory",  // Vector Memory
        "Export",         // Export
        "Branch",         // Branch
        "Message",        // Message
        "Wave Control",   // Wave Control
        "Trap"            // Trap
    };

    // Instruction's functional group's subgroup.
    enum class kFunctionalSubgroup
    {
        kFunctionalSubgroupUnknown,        // Unknown
        kFunctionalSubgroupFloatingPoint,  // Floating Point
        kFunctionalSubgroupBuffer,         // Buffer
        kFunctionalSubgroupTexture,        // Texture
        kFunctionalSubgroupLoad,           // Load
        kFunctionalSubgroupStore,          // Store
        kFunctionalSubgroupSample,         // Sample
        kFunctionalSubgroupBvh,            // BVH
        kFunctionalSubgroupAtomic,         // Atomic
        kFunctionalSubgroupFlat,           // Flat
        kFunctionalSubgroupDataShare,      // Data Share
        kFunctionalSubgroupStatic          // Static
    };

    static const char* kFunctionalSubgroupName[] = {
        "Unknown",         // Unknown
        "Floating Point",  // Floating Point
        "Buffer",          // Buffer
        "Texture",         // Texture
        "Load",            // Load
        "Store",           // Store
        "Sample",          // Sample
        "BVH",             // BVH
        "Atomic",          // Atomic
        "Flat",            // Flat
        "Data Share",      // Data Share
        "Static"           // Static
    };

    // OperandModifier provides information about an input/output modifier.
    struct OperandModifer
    {
        std::string modifier_name;
        uint32_t    value = 0;
    };

    // EncodingField provides information about a single field of an instruction's encoding.
    // For example, OP is a field that encodes the opcode of a specific instruction.
    // InstructionInfo object contains a vector of EncodingFields that provide
    // information about every single field of the encoding.
    struct EncodingField
    {
        std::string field_name;
        uint64_t    field_value = 0;

        // Size of the field.
        uint64_t bit_count = 0;

        // Position of the field in the encoding.
        uint64_t bit_offset = 0;
    };

    // InstructionOperand provides information about a single operand of an
    // instruction. InstructionInfo contains a vector of InstructionOperands
    // that provide information about operands involved in a given instruction.
    struct InstructionOperand
    {
        // Name of the operand. For example: v1, s1, or literals.
        std::string operand_name;

        // Size of the operand.
        uint32_t operand_size = 0;

        // True if the operand is an input to an instruction (source).
        bool is_input = false;

        // True if the operand is an output of an instruction (destination).
        bool is_output = false;
    };

    struct BranchInfo
    {
        // True if the instruction is a branch, false otherwise.
        bool is_branch = false;

        // True if the branch is conditional, false otherwise.
        bool is_conditional = false;

        // True if the branch is indirect, false otherwise.
        bool is_indirect = false;

        // True if the branch is direct.
        bool IsDirect() const;

        // Branch offset for direct branches (offset is embedded in the instruction).
        int32_t branch_offset = kBranchOffsetOutOfRange;

        // Direct branch target's PC (program counter).
        std::string branch_target_pc;

        // Direct branch target's label (if applicable).
        // When the decoded data is retrieved from disassembly,
        // the label representing the branch target will be stored here.
        std::string branch_target_label;

        // Direct branch target's index in the InstructionInfoBundle.
        uint64_t branch_target_index = kInvalidBranchTarget;
    };

    // InstructionSemanticInfo provides behavioral information of an instruction.
    struct InstructionSemanticInfo
    {
        // Branch related information.
        BranchInfo branch_info;

        // True if this instruction indicates the end of the program and false
        // otherwise.
        bool is_program_terminator = false;

        // True if this instruction is executed directly by each wave's instruction
        // buffer.
        bool is_immediately_executed = false;
    };

    struct FunctionalGroupSubgroupInfo
    {
        std::string         description;
        kFunctionalGroup    IsaFunctionalGroup    = kFunctionalGroup::kFunctionalGroupUnknown;
        kFunctionalSubgroup IsaFunctionalSubgroup = kFunctionalSubgroup::kFunctionalSubgroupUnknown;
    };

    // InstructionInfo is a structure that gets populated as a result of API
    // request.
    struct InstructionInfo
    {
        // Name of the instruction.
        std::string instruction_name;

        // Aliased instruction names.
        std::vector<std::string> aliased_names;

        // Description of an instruction.
        std::string instruction_description;

        // Encoding of an instruction.
        std::string encoding_name;

        // Description of the encoding an instruction belongs to.
        std::string encoding_description;

        // Human-readable information about all of the fields and corresponding
        // encoded field values as a single string.
        std::string encoding_layout;

        // Information about the instruction's fields.
        std::vector<EncodingField> encoding_fields;

        // The operands involved in a given instruction.
        std::vector<InstructionOperand> instruction_operands;

        // Modifiers applied to the operands.
        std::vector<OperandModifer> operand_modifiers;

        // Semantic information of an instruction.
        InstructionSemanticInfo instruction_semantic_info;

        // Instruction's functional subgroup.
        FunctionalGroupSubgroupInfo functional_group_subgroup_info;
    };

    // InstructionInfoBundle is one or more InstructionInfo objects
    // that were generated as a result of the decode of a single
    // binary encoded instruction. Certain encodings decode more than
    // a single instruction, and are therefore decoded into multiple
    // instructions. The purpose of this structure is to bundle such
    // instructions together.
    struct InstructionInfoBundle
    {
        std::vector<InstructionInfo> bundle;
    };

    class IsaDecoder
    {
    public:
        /*
     * Initialize --
     *
     * Reads in XML ISA specification and populates the internal structures.
     *
     * Returns true if the spec was successfully read, or false otherwise
     * with the error message in err_message.
     */
        bool Initialize(const std::string& input_xml_file_path, std::string& err_message);

        /*
     * GetVersion --
     *
     * Return a "<major>.<minor>.<patch>" string that represents the version of this API.
     * For more details or preprocessor access to version components, please see api_version.h.
     */
        std::string GetVersion() const;

        /*
     * GetArchitecture --
     *
     * Return the architecture enumaration.
     */
        GpuArchitecture GetArchitecture() const;

        /*
     * DecodeInstruction --
     *
     * Accepts a single machine code. This function is limited to 64-bit wide
     * instructions or shorter. Use DecodeInstructionStream function for longer
     * instructions.
     *
     * Returns true if the provided machine_code (binary representation of the
     * instruction) was decoded successfully and populates instruction_info with
     * corresponding information about the decoded instruction.
     * Returns false otherwise with the error string output.
     */
        bool DecodeInstruction(uint64_t machine_code, InstructionInfoBundle& instruction_info, std::string& err_message) const;

        /*
     * DecodeInstruction --
     *
     * Accepts a string that represents an instruction name.
     *
     * Returns true if the provided instruction was decoded successfully and
     * populates instruction_info with corresponding information about the
     * decoded instruction. Returns false otherwise with the error string output.
     */
        bool DecodeInstruction(const std::string& instruction_name, InstructionInfo& instruction_info, std::string& err_message) const;

        /*
     * DecodeInstructionStream --
     *
     * Accepts instruction stream as a vector of DWORDs.
     *
     * Returns true if all of the instructions were successfully decoded, or false
     * even if a single DWORD from instruction stream fails to be decoded. On a
     * successful decode, the function outputs a vector of InstructionInfoGroup.
     * Each of the objects in the vector correspond to a decoded instruction in
     * the input instruction stream.
     */
        bool DecodeInstructionStream(const std::vector<uint32_t>&        machine_code_stream,
                                     std::vector<InstructionInfoBundle>& instruction_info_stream,
                                     std::string&                        err_message) const;

        /*
     * DecodeShaderDisassemblyText --
     *
     * Accepts a shader disassembly text (SP3/LLVM).
     *
     * Returns true if all the instructions present in the provided shader
     * disassembly text were successfully extracted and decoded, or false
     * otherwise (even if the decoding of a single instruction fails - false
     * is returned). The failure reason is populated in the err_message.
     * On successful decode, the function outputs a vector of
     * InstructionInfoBundle with the decoded information of all the instructions.
     *
     * Additional Option: resolve_direct_branch_targets
     * (optional, default - false and skipped)
     * Setting the resolve_direct_branch_targets to true enables determining
     * the direct branch's target instruction's index in the
     * instruction_info_bundle vector which will be available in the
     * InstructionInfo.instruction_semantic_info.branch_info's struct member
     * branch_target_index
     */
        bool DecodeShaderDisassemblyText(const std::string&                  shader_disassembly_text,
                                         std::vector<InstructionInfoBundle>& instruction_info_stream,
                                         std::string&                        err_message,
                                         bool                                resolve_direct_branch_targets) const;

        /*
     * DecodeShaderDisassemblyFile --
     *
     * Accepts a shader disassembly file (SP3/LLVM).
     *
     * Returns true if all the instructions present in the provided shader
     * disassembly file were successfully extracted and decoded, or false
     * otherwise (even if the decoding of a single instruction fails) and
     * the failure reason is populated in the err_message. On successful decode,
     * the function outputs a vector of InstructionInfoBundle with the decoded
     * information of all the instructions.
     *
     * Additional Option: resolve_direct_branch_targets
     * (optional, default - false and skipped)
     * Setting the resolve_direct_branch_targets to true enables determining
     * the direct branch's target instruction's index in the
     * instruction_info_bundle vector which will be available in the
     * InstructionInfo.instruction_semantic_info.branch_info's struct member
     * branch_target_index
     */
        bool DecodeShaderDisassemblyFile(const std::string&                  shader_disassembly_file,
                                         std::vector<InstructionInfoBundle>& instruction_info_stream,
                                         std::string&                        err_message,
                                         bool                                resolve_direct_branch_targets) const;

        IsaDecoder() = default;
        ~IsaDecoder();

    private:
        class IsaDecoderImpl;
        IsaDecoderImpl* api_impl_ = nullptr;
    };

    // Convenience API for handling multiple architectures.
    class DecodeManager
    {
    public:
        /**
     * Initialize --
     *
     * Accepts the vector of file paths to specs and
     * initializes the API with the provided spec files.
     *
     * Returns 'true' if the initialization is successful.
     * Returns 'false' if any error occurs during initialization.
     */
        bool Initialize(const std::vector<std::string>& input_spec_file_paths, std::string& err_message);

        /**
     * GetDecoder --
     *
     * Accepts the architecture enum for which amdisa::IsaDecoder is requested.
     *
     * Returns pointer to the decoder if the provided architecture was found, and the nullptr otherwise.
     */
        std::shared_ptr<IsaDecoder> GetDecoder(GpuArchitecture architecture) const;

        DecodeManager() = default;
        ~DecodeManager();

    private:
        struct DecodeManagerImpl;
        DecodeManagerImpl* manager_impl_ = nullptr;
    };
}  // namespace amdisa
#endif  // ISA_DECODER_H_
