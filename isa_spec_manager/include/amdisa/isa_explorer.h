#ifndef _AMDISA_ISA_EXPLORER_H_
#define _AMDISA_ISA_EXPLORER_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace amdisa
{
    namespace explorer
    {

        /// @brief Enumeration of the data types supported by the ISA
        enum class DataType : int
        {
            kUnknown,
            kInteger,
            kFloat,
            kBinary,
            kDescriptor
        };

        /// @brief Enumeration of the signedness of the data format
        enum class Signedness : int
        {
            kUnknown,
            kDoesNotApply,
            kUnsigned,
            kSigned,
            kSignedByModifier
        };

        class Instruction;         // forward declaration
        class FunctionalSubgroup;  // forward declaration
        class Handle;              // forward declaration

        /// @brief This class represents the ISA Architecture
        class Architecture
        {
        public:
            /// @brief Constructor
            Architecture();

            /// @brief Constructor
            Architecture(const std::string& name);

            /// @brief Get the name of the architecture
            ///
            /// @returns Name of the architecture
            const std::string& Name() const noexcept;

        private:
            const std::string name_;  /// Name of the architecture
        };

        /// @brief The class represents the predefined value of an operand type
        class PredefinedValue
        {
        public:
            /// @brief Constructor
            PredefinedValue(const std::string& name, const std::string& description, uint32_t value);

            /// @brief Get the name of the predefined value
            ///
            /// @returns Name of the predefined value
            const std::string& Name() const noexcept;

            /// @brief Get the description of the predefined value
            ///
            /// @returns Description of the predefined value
            const std::string& Description() const noexcept;

            /// @brief Get the value of the predefined value
            ///
            /// @returns Value of the predefined value
            uint32_t Value() const noexcept;

        private:
            const std::string name_;         ///< Name of the predefined value
            const std::string description_;  ///< Description of the predefined value
            const uint32_t    value_;        ///< Value of the predefined value
        };

        /// @brief The class represents the padding of a range
        class Padding
        {
        public:
            /// @brief Constructor
            Padding(uint32_t bit_count, uint32_t value);
            /// @brief Get the number of bits in the padding
            ///
            /// @returns Number of bits in the padding
            uint32_t BitCount() const noexcept;
            /// @brief Get the value of the padding
            ///
            /// @returns Value of the padding
            uint32_t Value() const noexcept;

        private:
            const uint32_t bit_count_;  /// @brief Number of bits in the padding
            const uint32_t value_;      /// @brief Value of the padding
        };

        /// @brief The class represents the range of a field
        class Range
        {
        public:
            /// @brief Constructor
            Range(uint32_t order, uint32_t bit_count, uint32_t bit_offset, const Padding& padding);

            /// @brief Get the order of the range
            ///
            /// @returns Order of the range
            uint32_t Order() const noexcept;
            /// @brief Get the number of bits in the range
            ///
            /// @returns Number of bits in the range
            uint32_t BitCount() const noexcept;
            /// @brief Get the offset of the range
            ///
            /// @returns Offset of the range
            uint32_t BitOffset() const noexcept;
            /// @brief Get the padding of the range
            ///
            /// @returns Padding of the range
            const Padding& GetPadding() const noexcept;

        private:
            const uint32_t order_;       /// Order of the range
            const uint32_t bit_count_;   /// Number of bits in the range
            const uint32_t bit_offset_;  /// Offset of the range
            const Padding  padding_;     /// Padding of the range
        };

        /// @brief The class represents the field of a data attribute
        class Field
        {
        public:
            /// @brief Constructor
            Field(const std::string&                  name,
                  const std::string&                  description,
                  const std::string&                  type,
                  bool                                is_conditional,
                  bool                                is_constant,
                  uint32_t                            range_count,
                  Signedness                          signedness,
                  const std::vector<Range>&           ranges,
                  const std::vector<PredefinedValue>& predefined_values);

            /// @brief Get the name of the field
            ///
            /// @returns Name of the field
            const std::string& Name() const noexcept;

            /// @brief Get the description of the field
            ///
            /// @returns Description of the field
            const std::string& Description() const noexcept;

            /// @brief Get the data type of the field
            ///
            /// @returns Data type of the field
            const std::string& Type() const noexcept;

            /// @brief Whether the field is conditional
            ///
            /// @returns Whether the field is conditional
            bool IsConditional() const noexcept;

            /// @brief Whether the field is constant
            ///
            /// @returns Whether the field is constant
            bool IsConstant() const noexcept;

            /// @brief Get the number of ranges
            ///
            /// @returns Number of ranges
            uint32_t RangeCount() const noexcept;

            /// @brief Get the signedness of the field
            ///
            /// @returns Signedness of the field
            Signedness GetSignedness() const noexcept;

            /// @brief Get the ranges of the field
            ///
            /// @returns Ranges of the field
            const std::vector<Range>& Ranges() const noexcept;

            /// @brief Get the predefined values of the field
            ///
            /// @returns Predefined values of the field
            const std::vector<PredefinedValue>& PredefinedValues() const noexcept;

        private:
            const std::string                  name_;               ///< Name of the field
            const std::string                  description_;        ///< Description of the field
            const std::string                  type_;               ///< Type of the field
            const bool                         is_conditional_;     ///< Whether the field is conditional
            const bool                         is_constant_;        ///< Whether the field is constant
            const uint32_t                     range_count_;        ///< Number of ranges
            const Signedness                   signedness_;         ///< Signedness of the field
            const std::vector<Range>           ranges_;             ///< Ranges of the field
            const std::vector<PredefinedValue> predefined_values_;  ///< Predefined values of the field
        };

        /// @brief The class represents the data attributes of a data format
        class DataAttributes
        {
        public:
            /// @brief Constructor
            DataAttributes(uint32_t order, const std::vector<Field>& bit_map);

            /// @brief Get the order of the data attribute
            uint32_t Order() const noexcept;

            /// @brief Get the bit map of the data attribute
            const std::vector<Field>& BitMap() const noexcept;

        private:
            const uint32_t           order_ = 0;  ///< Order of the data attribute
            const std::vector<Field> bit_map_;    ///< Bit map of the data attribute
        };

        /// @brief The class represents the data format of an operand
        class DataFormat
        {
        public:
            /// @brief Constructor
            DataFormat(const std::string&                 name,
                       const std::string&                 description,
                       const DataType                     dtype,
                       const uint32_t                     bit_count,
                       const uint32_t                     component_count,
                       const std::vector<DataAttributes>& data_attributes);

            /// @brief Get the name of the DataFormat
            ///
            /// @returns Name of the DataFormat
            const std::string& Name() const noexcept;

            /// @brief Get the description of the DataFormat
            ///
            /// @returns Description of the DataFormat
            const std::string& Description() const noexcept;

            /// @brief Get the data type of the DataFormat
            ///
            /// @returns Data type of the DataFormat
            DataType DType() const noexcept;

            /// @brief Get bit_count of the DataFormat
            ///
            /// @returns bit_count of the DataFormat
            uint32_t BitCount() const noexcept;

            /// @brief Get component_count of the DataFormat
            ///
            /// @returns component_count of the DataFormat
            uint32_t ComponentCount() const noexcept;

            /// @brief Get the data attributes of the DataFormat
            ///
            /// @returns Data attributes of the DataFormat
            const std::vector<DataAttributes>& DataAttr() const noexcept;

        private:
            const std::string                 name_;             ///< Name of the data format
            const std::string                 description_;      ///< Description of the data format
            const DataType                    data_type_;        ///< Data type of the data format
            const uint32_t                    bit_count_;        ///< Number of bits in the data format
            const uint32_t                    component_count_;  ///< Number of components in the data format
            const std::vector<DataAttributes> data_attributes_;  ///< Data attributes of the data format
        };

        /// @brief The class represents the functional group of instructions
        class FunctionalGroup
        {
        public:
            /// @brief Constructor
            FunctionalGroup(const std::string& name, const std::string& description, const std::vector<FunctionalSubgroup>& subgroups);

            /// @brief Get the name of the functional group
            ///
            /// @returns Name of the functional group
            const std::string& Name() const noexcept;

            /// @brief Get the description of the functional group
            ///
            /// @returns Description of the functional group
            const std::string& Description() const noexcept;

            /// @brief Get the instructions in the functional group
            ///
            /// @returns Instructions in the functional group
            const std::vector<Instruction*>& Instructions() const noexcept;

            ///@brief Get the subgroups in the functional group
            ///
            /// @returns Subgroups in the functional group
            const std::vector<FunctionalSubgroup>& FuncSubgroups() const noexcept;

        private:
            /// @brief Add the instruction to the functional group
            void AddInstruction(Instruction* instruction);
            friend class Handle;

            const std::string                     name_;                  ///< Name of the functional group
            const std::string                     description_;           ///< Description of the functional group
            std::vector<Instruction*>             instructions_;          ///< Instructions in the functional group
            const std::vector<FunctionalSubgroup> functional_subgroups_;  ///< Subgroups in the functional group
        };

        /// @brief The class represents the functional subgroup of instructions
        class FunctionalSubgroup
        {
        public:
            /// @brief Constructor
            FunctionalSubgroup(std::string name);

            /// @brief Get the name of the functional subgroup
            ///
            /// @returns Name of the functional subgroup
            const std::string& Name() const noexcept;

            /// @brief Get the instructions in the functional subgroup
            ///
            /// @returns Instructions in the functional subgroup
            const std::vector<Instruction*>& Instructions() const noexcept;

        private:
            /// @brief Add the instruction to the functional group
            void AddInstruction(Instruction* instruction);
            friend class Handle;

            std::string               name_;          ///< Name of the functional subgroup
            std::vector<Instruction*> instructions_;  ///< Instructions in the functional subgroup
        };

        /// @brief The class represents the operand type
        class OperandType
        {
        public:
            /// @brief Constructor
            OperandType(const std::string& name, const std::string& description, bool is_partitioned, const std::vector<PredefinedValue>& predefined_values);

            /// @brief Get the name of the operand type
            ///
            /// @returns Name of the operand type
            const std::string& Name() const noexcept;

            /// @brief Get the description of the operand type
            ///
            /// @returns Description of the operand type
            const std::string& Description() const noexcept;

            /// @brief Whether the operand type is partitioned
            ///
            /// @returns Whether the operand type is partitioned
            bool IsPartitioned() const noexcept;

            /// @brief Get the predefined values of the operand type
            ///
            /// @returns Predefined values of the operand type
            const std::vector<PredefinedValue>& PredefinedValues() const noexcept;

        private:
            const std::string                  name_;               ///< Name of the operand type
            const std::string                  description_;        ///< Description of the operand type
            const bool                         is_partitioned_;     ///< Whether the operand type is partitioned
            const std::vector<PredefinedValue> predefined_values_;  ///< Predefined values of the operand type
        };

        /// @brief The class represents the operand of the instruction
        class Operand
        {
        public:
            /// @brief Constructor
            Operand(const std::string& field_name,
                    const std::string& encoding_field_name,
                    const DataFormat&  data_format,
                    const OperandType& operand_type,
                    uint32_t           order,
                    uint32_t           size,
                    bool               is_input,
                    bool               is_output,
                    bool               is_implicit,
                    bool               is_ucode);

            /// @brief Get the name of the field
            ///
            /// @returns Name of the field
            const std::string& FieldName() const noexcept;

            /// @brief Get the name of the encoding field
            ///
            /// @returns Name of the encoding field
            const std::string& EncodingFieldName() const noexcept;

            /// @brief Get the DataFormat of the operand
            ///
            /// @returns DataFormat of the operand
            const DataFormat& DataFmt() const noexcept;

            /// @brief Get the type of the operand
            ///
            /// @returns Type of the operand
            const OperandType& Type() const noexcept;

            /// @brief Get the order of the operand
            ///
            /// @returns Order of the operand
            uint32_t Order() const noexcept;

            /// @brief Get the size of the operand
            ///
            /// @returns Size of the operand
            uint32_t Size() const noexcept;

            /// @brief Whether the operand is an input
            ///
            /// @returns Whether the operand is an input
            bool IsInput() const noexcept;

            /// @brief Whether the operand is an output
            ///
            /// @returns Whether the operand is an output
            bool IsOutput() const noexcept;

            /// @brief Whether the operand is implicit
            ///
            /// @returns Whether the operand is implicit
            bool IsImplicit() const noexcept;

            /// @brief Whether the operand is in microcode
            ///
            /// @returns Whether the operand is in microcode
            bool IsInMicrocode() const noexcept;

        private:
            const std::string  field_name_;           ///< Name of the field
            const std::string  encoding_field_name_;  ///< Name of the encoding field
            const DataFormat*  data_format_;          /// Data format of the operand
            const OperandType* operand_type_;         ///< Type of the operand
            const uint32_t     order_;                ///< Order of the operand
            const uint32_t     size_;                 ///< Size of the operand
            const bool         is_input_;             ///< Whether the operand is an input
            const bool         is_output_;            ///< Whether the operand is an output
            const bool         is_implicit_;          ///<whether the operand is implicit
            const bool         is_ucode_;             ///< Whether the operand is in microcode
        };

        /// @brief The class represents the encoding of the instruction
        class InstructionEncoding
        {
        public:
            /// @brief Constructor
            InstructionEncoding(const std::string& name, uint32_t opcode, const std::vector<Operand>& operands);

            /// @brief Get the name of the encoding
            ///
            /// @returns Name of the encoding
            const std::string& Name() const noexcept;

            /// @brief Get the opcode of the encoding
            ///
            /// @returns Opcode of the encoding
            uint32_t Opcode() const noexcept;

            /// @brief Get the operands of the encoding
            ///
            /// @returns Operands of the encoding
            const std::vector<Operand>& Operands() const noexcept;

        private:
            const std::string          name_;      ///< Name of the encoding
            const uint32_t             opcode_;    ///< Opcode of the encoding
            const std::vector<Operand> operands_;  ///< Operands of the encoding
        };

        /// @brief The class represents the instruction
        class Instruction
        {
        public:
            ///@brief Constructor
            Instruction(const std::string&                      name,
                        const std::string&                      description,
                        bool                                    is_branch,
                        bool                                    is_conditional_branch,
                        bool                                    is_indirect_branch,
                        bool                                    is_immediately_executed,
                        bool                                    is_program_terminator,
                        FunctionalGroup&                        functional_group,
                        FunctionalSubgroup&                     functional_subgroup,
                        const std::vector<InstructionEncoding>& encodings);
            /// @brief Get the name of the instruction
            ///
            /// @returns Name of the instruction
            const std::string& Name() const noexcept;

            /// @brief Get the description of the instruction
            ///
            /// @returns Description of the instruction
            const std::string& Description() const noexcept;

            /// @brief Whether the instruction is a branch
            ///
            /// @returns Whether the instruction is a branch
            bool IsBranch() const noexcept;

            /// @brief Whether the instruction is a conditional branch
            ///
            /// @returns Whether the instruction is a conditional branch
            bool IsConditionalBranch() const noexcept;

            /// @brief Whether the instruction is an indirect branch
            ///
            /// @returns Whether the instruction is an indirect branch
            bool IsIndirectBranch() const noexcept;

            /// @brief Whether the instruction is immediately executed
            ///
            /// @returns Whether the instruction is immediately executed
            bool IsImmediatelyExecuted() const noexcept;

            /// @brief Whether the instruction is a program terminator
            ///
            /// @returns Whether the instruction is a program terminator
            bool IsProgramTerminator() const noexcept;

            /// @brief Get the functional group to which the instruction belongs
            ///
            /// @returns Functional group to which the instruction belongs
            const FunctionalGroup* const FuncGroup() const noexcept;

            /// @brief Get the functional subgroup to which the instruction belongs
            ///
            /// @returns Functional subgroup to which the instruction belongs
            const FunctionalSubgroup* const FuncSubgroup() const noexcept;

            /// @brief Get the encodings of the instruction
            ///
            /// @returns Encodings of the instruction
            const std::vector<InstructionEncoding>& Encodings() const noexcept;

        private:
            const std::string                      name_;                     ///< Name of the instruction
            const std::string                      description_;              ///< Description of the instruction
            const bool                             is_branch_;                ///< Whether the instruction is a branch
            const bool                             is_conditional_branch_;    ///< Whether the instruction is a conditional branch
            const bool                             is_indirect_branch_;       ///< Whether the instruction is an indirect branch
            const bool                             is_immediately_executed_;  ///< Whether the instruction is immediately executed
            const bool                             is_program_terminator_;    ///< Whether the instruction is a program terminator
            FunctionalGroup* const                 functional_group_;         ///< Functional group to which the instruction belongs
            FunctionalSubgroup* const              functional_subgroup_;      ///< Functional subgroup to which the instruction belongs
            const std::vector<InstructionEncoding> encodings_;                ///< Encodings of the instruction
        };

        /// @brief The class represents the handle to the ISA explorer
        class Handle
        {
        public:
            /// @brief Constructor
            Handle(const std::string& input_xml_file_path);
            Handle();
            ~Handle();
            Handle(const Handle&) = delete;
            Handle(Handle&&);

            //// @brief Get the Architecture of this ISA
            const Architecture& Arch() const noexcept;

            /// @brief Get the DataFormats of this ISA
            const std::map<std::string, DataFormat>& DataFormats() const noexcept;

            /// @brief Get the instructions of this ISA
            const std::map<std::string, Instruction>& Instructions() const noexcept;

            //// @brief Get the OperandTypes of this ISA
            const std::map<std::string, OperandType>& OperandTypes() const noexcept;

        private:
            class Impl;
            std::unique_ptr<Impl> impl_;  ///< Private implementation
        };

    }  // namespace explorer
}  // namespace amdisa

#endif  // _AMDISA_ISA_EXPLORER_H_
