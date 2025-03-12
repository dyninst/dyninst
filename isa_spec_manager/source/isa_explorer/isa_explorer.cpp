#include "amdisa/isa_explorer.h"
#include "amdisa_structures.h"
#include "isa_xml_reader.h"

#include <iostream>
#include <tuple>
#include <utility>

namespace amdisa
{
    namespace explorer
    {
        Architecture::Architecture(const std::string& name)
            : name_(name)
        {
        }

        const std::string& Architecture::Name() const noexcept
        {
            return name_;
        }
        class Handle::Impl
        {
        private:
            std::map<std::string, OperandType>        operand_types_;
            std::map<std::string, FunctionalGroup>    func_groups_;
            std::map<std::string, FunctionalSubgroup> func_subgroups_;
            std::map<std::string, Instruction>        instructions_;
            std::map<std::string, DataFormat>         data_formats_;
            std::unique_ptr<Architecture>             architecture_;

        public:
            Impl(const std::string& input_xml_file_path)
            {
                std::map<std::string, DataType> data_types_string_to_enum{{"unknown", DataType::kUnknown},
                                                                          {"integer", DataType::kInteger},
                                                                          {"float", DataType::kFloat},
                                                                          {"bits", DataType::kBinary},
                                                                          {"descriptor", DataType::kDescriptor}};
                std::string                     err_msg;
                amdisa::IsaSpec                 spec;
                if (!amdisa::IsaXmlReader::ReadSpec(input_xml_file_path, spec, err_msg))
                {
                    std::cerr << "Error reading ISA XML: " << err_msg << std::endl;
                    std::abort();
                }

                architecture_ = std::make_unique<Architecture>(spec.architecture.name);

                for (auto& df : spec.data_formats)
                {
                    std::vector<DataAttributes> data_attributes;
                    for (auto& da : df.data_attributes)
                    {
                        std::vector<Field> fields;
                        for (auto& field : da.bit_map)
                        {
                            std::vector<Range> ranges;
                            for (auto& range : field.ranges)
                            {
                                ranges.emplace_back(range.order, range.bit_count, range.bit_offset, Padding(range.padding.bit_count, range.padding.value));
                            }
                            std::vector<PredefinedValue> predefined_values;
                            for (auto& pf : field.predefined_values)
                            {
                                predefined_values.emplace_back(pf.name, pf.description, pf.value);
                            }
                            Signedness signedness = Signedness::kUnknown;
                            switch (field.signedness)
                            {
                            case amdisa::Signedness::kUnknown:
                                signedness = Signedness::kUnknown;
                                break;
                            case amdisa::Signedness::kSigned:
                                signedness = Signedness::kSigned;
                                break;
                            case amdisa::Signedness::kUnsigned:
                                signedness = Signedness::kUnsigned;
                                break;
                            case amdisa::Signedness::kSignedByModifier:
                                signedness = Signedness::kSignedByModifier;
                            }

                            fields.emplace_back(field.name,
                                                field.description,
                                                field.type,
                                                field.is_conditional,
                                                field.is_constant,
                                                field.range_count,
                                                signedness,
                                                ranges,
                                                predefined_values);
                        }
                        data_attributes.emplace_back(da.order, fields);
                    }
                    auto dtype = DataType::kUnknown;
                    switch (df.data_type)
                    {
                    case amdisa::DataType::kUnknown:
                        dtype = DataType::kUnknown;
                        break;
                    case amdisa::DataType::kInteger:
                        dtype = DataType::kInteger;
                        break;
                    case amdisa::DataType::kFloat:
                        dtype = DataType::kFloat;
                        break;
                    case amdisa::DataType::kBits:
                        dtype = DataType::kBinary;
                        break;
                    case amdisa::DataType::kDescriptor:
                        dtype = DataType::kDescriptor;
                        break;
                    }
                    data_formats_.emplace(std::piecewise_construct,
                                          std::forward_as_tuple(df.name),
                                          std::forward_as_tuple(DataFormat(df.name, df.description, dtype, df.bit_count, df.component_count, data_attributes)));
                }

                for (auto& func_group_info : spec.functional_group_info)
                {
                    func_groups_.emplace(std::piecewise_construct,
                                         std::forward_as_tuple(func_group_info.name),
                                         std::forward_as_tuple(FunctionalGroup(func_group_info.name, func_group_info.desc, {})));
                }

                for (auto& func_subgroup_info : spec.functional_subgroup_info)
                {
                    func_subgroups_.emplace(std::piecewise_construct,
                                            std::forward_as_tuple(func_subgroup_info.name),
                                            std::forward_as_tuple(FunctionalSubgroup(func_subgroup_info.name)));
                }
                // FIXME: spec.functional_subgroup_info is not populated, so we will populate it manually
                for (auto& instr : spec.instructions)
                {
                    func_subgroups_.emplace(std::piecewise_construct,
                                            std::forward_as_tuple(instr.functional_subgroup_name),
                                            std::forward_as_tuple(FunctionalSubgroup(instr.functional_subgroup_name)));
                }
                for (auto& operand_type : spec.operand_types)
                {
                    std::vector<PredefinedValue> predefined_values;
                    for (auto& pv : operand_type.predefined_values)
                    {
                        predefined_values.emplace_back(pv.name, pv.description, pv.value);
                    }
                    operand_types_.emplace(
                        std::piecewise_construct,
                        std::forward_as_tuple(operand_type.name),
                        std::forward_as_tuple(OperandType(operand_type.name, operand_type.description, operand_type.is_partitioned, predefined_values)));
                }
                for (auto& instr : spec.instructions)
                {
                    std::vector<InstructionEncoding> encodings;
                    for (auto& encoding : instr.encodings)
                    {
                        std::vector<Operand> operands;
                        for (auto& operand : encoding.operands)
                        {
                            operands.emplace_back(operand.field_name,
                                                  operand.encoding_field_name,
                                                  data_formats_.at(operand.data_format),
                                                  operand_types_.at(operand.type),
                                                  operand.order,
                                                  operand.size,
                                                  operand.input,
                                                  operand.output,
                                                  operand.is_implicit,
                                                  operand.is_in_microcode);
                        }
                        encodings.emplace_back(encoding.name, encoding.opcode, operands);
                    }
                    auto& func_group    = func_groups_.at(instr.functional_group_name);
                    auto& func_subgroup = func_subgroups_.at(instr.functional_subgroup_name);
                    instructions_.emplace(std::piecewise_construct,
                                          std::forward_as_tuple(instr.name),
                                          std::forward_as_tuple(instr.name,
                                                                instr.description,
                                                                instr.is_branch,
                                                                instr.is_conditional_branch,
                                                                instr.is_indirect_branch,
                                                                instr.is_immediately_executed,
                                                                instr.is_program_terminator,
                                                                func_group,
                                                                func_subgroup,
                                                                encodings));

                    auto ins = &instructions_.at(instr.name);
                    func_group.AddInstruction(ins);
                    func_subgroup.AddInstruction(ins);
                }
            }

            const std::map<std::string, Instruction>& Instructions() const noexcept
            {
                return instructions_;
            }

            const std::map<std::string, OperandType>& OperandTypes() const noexcept
            {
                return operand_types_;
            }

            const Architecture& GetArchitecture() const noexcept
            {
                return *architecture_;
            }
        };

        void FunctionalGroup::AddInstruction(Instruction* instruction)
        {
            instructions_.push_back(instruction);
        }

        Padding::Padding(uint32_t bit_count, uint32_t value)
            : bit_count_(bit_count)
            , value_(value)
        {
        }
        uint32_t Padding::BitCount() const noexcept
        {
            return bit_count_;
        }
        uint32_t Padding::Value() const noexcept
        {
            return value_;
        }
        Range::Range(uint32_t order, uint32_t bit_count, uint32_t bit_offset, const Padding& padding)
            : order_(order)
            , bit_count_(bit_count)
            , bit_offset_(bit_offset)
            , padding_(padding)
        {
        }
        uint32_t Range::Order() const noexcept
        {
            return order_;
        }
        uint32_t Range::BitCount() const noexcept
        {
            return bit_count_;
        }
        uint32_t Range::BitOffset() const noexcept
        {
            return bit_offset_;
        }
        const Padding& Range::GetPadding() const noexcept
        {
            return padding_;
        }
        Field::Field(const std::string&                  name,
                     const std::string&                  description,
                     const std::string&                  type,
                     bool                                is_conditional,
                     bool                                is_constant,
                     uint32_t                            range_count,
                     Signedness                          signedness,
                     const std::vector<Range>&           ranges,
                     const std::vector<PredefinedValue>& predefined_values)
            : name_(name)
            , description_(description)
            , type_(type)
            , is_conditional_(is_conditional)
            , is_constant_(is_constant)
            , range_count_(range_count)
            , signedness_(signedness)
            , ranges_(ranges)
            , predefined_values_(predefined_values)
        {
        }

        const std::string& Field::Name() const noexcept
        {
            return name_;
        }
        const std::string& Field::Description() const noexcept
        {
            return description_;
        }
        const std::string& Field::Type() const noexcept
        {
            return type_;
        }
        bool Field::IsConditional() const noexcept
        {
            return is_conditional_;
        }
        bool Field::IsConstant() const noexcept
        {
            return is_constant_;
        }
        uint32_t Field::RangeCount() const noexcept
        {
            return range_count_;
        }
        Signedness Field::GetSignedness() const noexcept
        {
            return signedness_;
        }
        const std::vector<Range>& Field::Ranges() const noexcept
        {
            return ranges_;
        }
        const std::vector<PredefinedValue>& Field::PredefinedValues() const noexcept
        {
            return predefined_values_;
        }

        DataAttributes::DataAttributes(uint32_t order, const std::vector<Field>& bit_map)
            : order_(order)
            , bit_map_(bit_map)
        {
        }
        uint32_t DataAttributes::Order() const noexcept
        {
            return order_;
        }
        const std::vector<Field>& DataAttributes::BitMap() const noexcept
        {
            return bit_map_;
        }

        DataFormat::DataFormat(const std::string&                 name,
                               const std::string&                 description,
                               const DataType                     dtype,
                               const uint32_t                     bit_count,
                               const uint32_t                     component_count,
                               const std::vector<DataAttributes>& data_attributes)
            : name_(name)
            , description_(description)
            , data_type_(dtype)
            , bit_count_(bit_count)
            , data_attributes_(data_attributes)
            , component_count_(component_count)
        {
        }
        const std::string& DataFormat::Name() const noexcept
        {
            return name_;
        }
        const std::string& DataFormat::Description() const noexcept
        {
            return description_;
        }
        DataType DataFormat::DType() const noexcept
        {
            return data_type_;
        }
        uint32_t DataFormat::BitCount() const noexcept
        {
            return bit_count_;
        }
        uint32_t DataFormat::ComponentCount() const noexcept
        {
            return component_count_;
        }

        const std::vector<class DataAttributes>& DataFormat::DataAttr() const noexcept
        {
            return data_attributes_;
        }

        FunctionalGroup::FunctionalGroup(const std::string& name, const std::string& description, const std::vector<FunctionalSubgroup>& subgroups)
            : name_(name)
            , description_(description)
            , functional_subgroups_(subgroups)
        {
        }

        const std::string& FunctionalGroup::Name() const noexcept
        {
            return name_;
        }

        const std::string& FunctionalGroup::Description() const noexcept
        {
            return description_;
        }

        const std::vector<Instruction*>& FunctionalGroup::Instructions() const noexcept
        {
            return instructions_;
        }

        const std::vector<FunctionalSubgroup>& FunctionalGroup::FuncSubgroups() const noexcept
        {
            return functional_subgroups_;
        }

        FunctionalSubgroup::FunctionalSubgroup(std::string name)
            : name_(name)
        {
        }
        void FunctionalSubgroup::AddInstruction(Instruction* instruction)
        {
            instructions_.push_back(instruction);
        }

        const std::string& FunctionalSubgroup::Name() const noexcept
        {
            return name_;
        }

        const std::vector<Instruction*>& FunctionalSubgroup::Instructions() const noexcept
        {
            return instructions_;
        }

        PredefinedValue::PredefinedValue(const std::string& name, const std::string& description, uint32_t value)
            : name_(name)
            , description_(description)
            , value_(value)
        {
        }

        const std::string& PredefinedValue::Name() const noexcept
        {
            return name_;
        }

        const std::string& PredefinedValue::Description() const noexcept
        {
            return description_;
        }

        uint32_t PredefinedValue::Value() const noexcept
        {
            return value_;
        }

        OperandType::OperandType(const std::string&                  name,
                                 const std::string&                  description,
                                 bool                                is_partitioned,
                                 const std::vector<PredefinedValue>& predefined_values)
            : name_(name)
            , description_(description)
            , is_partitioned_(is_partitioned)
            , predefined_values_(predefined_values)
        {
        }

        const std::string& OperandType::Name() const noexcept
        {
            return name_;
        }

        const std::string& OperandType::Description() const noexcept
        {
            return description_;
        }

        bool OperandType::IsPartitioned() const noexcept
        {
            return is_partitioned_;
        }

        const std::vector<PredefinedValue>& OperandType::PredefinedValues() const noexcept
        {
            return predefined_values_;
        }

        Operand::Operand(const std::string& field_name,
                         const std::string& encoding_field_name,
                         const DataFormat&  data_format,
                         const OperandType& operand_type,
                         uint32_t           order,
                         uint32_t           size,
                         bool               is_input,
                         bool               is_output,
                         bool               is_implicit,
                         bool               is_ucode)
            : field_name_(field_name)
            , encoding_field_name_(encoding_field_name)
            , data_format_(&data_format)
            , operand_type_(&operand_type)
            , order_(order)
            , size_(size)
            , is_input_(is_input)
            , is_output_(is_output)
            , is_implicit_(is_implicit)
            , is_ucode_(is_ucode)
        {
        }

        const std::string& Operand::FieldName() const noexcept
        {
            return field_name_;
        }

        const std::string& Operand::EncodingFieldName() const noexcept
        {
            return encoding_field_name_;
        }

        const DataFormat& Operand::DataFmt() const noexcept
        {
            return *data_format_;
        }

        const OperandType& Operand::Type() const noexcept
        {
            return *operand_type_;
        }

        uint32_t Operand::Order() const noexcept
        {
            return order_;
        }
        uint32_t Operand::Size() const noexcept
        {
            return size_;
        }

        bool Operand::IsInput() const noexcept
        {
            return is_input_;
        }

        bool Operand::IsOutput() const noexcept
        {
            return is_output_;
        }

        bool Operand::IsImplicit() const noexcept
        {
            return is_implicit_;
        }

        bool Operand::IsInMicrocode() const noexcept
        {
            return is_ucode_;
        }

        InstructionEncoding::InstructionEncoding(const std::string& name, uint32_t opcode, const std::vector<Operand>& operands)
            : name_(name)
            , opcode_(opcode)
            , operands_(operands)
        {
        }
        const std::string& InstructionEncoding::Name() const noexcept
        {
            return name_;
        }

        uint32_t InstructionEncoding::Opcode() const noexcept
        {
            return opcode_;
        }

        const std::vector<Operand>& InstructionEncoding::Operands() const noexcept
        {
            return operands_;
        }
        Instruction::Instruction(const std::string&                      name,
                                 const std::string&                      description,
                                 bool                                    is_branch,
                                 bool                                    is_conditional_branch,
                                 bool                                    is_indirect_branch,
                                 bool                                    is_immediately_executed,
                                 bool                                    is_program_terminator,
                                 class FunctionalGroup&                  functional_group,
                                 class FunctionalSubgroup&               functional_subgroup,
                                 const std::vector<InstructionEncoding>& encodings)
            : name_(name)
            , description_(description)
            , is_branch_(is_branch)
            , is_conditional_branch_(is_conditional_branch)
            , is_indirect_branch_(is_indirect_branch)
            , is_immediately_executed_(is_immediately_executed)
            , is_program_terminator_(is_program_terminator)
            , functional_group_(&functional_group)
            , functional_subgroup_(&functional_subgroup)
            , encodings_(encodings)
        {
        }

        const std::string& Instruction::Name() const noexcept
        {
            return name_;
        }

        const std::string& Instruction::Description() const noexcept
        {
            return description_;
        }

        bool Instruction::IsBranch() const noexcept
        {
            return is_branch_;
        }

        bool Instruction::IsConditionalBranch() const noexcept
        {
            return is_conditional_branch_;
        }

        bool Instruction::IsIndirectBranch() const noexcept
        {
            return is_indirect_branch_;
        }

        bool Instruction::IsImmediatelyExecuted() const noexcept
        {
            return is_immediately_executed_;
        }

        bool Instruction::IsProgramTerminator() const noexcept
        {
            return is_program_terminator_;
        }

        const FunctionalGroup* const Instruction::FuncGroup() const noexcept
        {
            return functional_group_;
        }

        const FunctionalSubgroup* const Instruction::FuncSubgroup() const noexcept
        {
            return functional_subgroup_;
        }

        const std::vector<InstructionEncoding>& Instruction::Encodings() const noexcept
        {
            return encodings_;
        }

        Handle::Handle()         = default;
        Handle::~Handle()        = default;
        Handle::Handle(Handle&&) = default;

        Handle::Handle(const std::string& input_xml_file_path)
            : impl_(std::make_unique<Impl>(input_xml_file_path))
        {
        }

        const std::map<std::string, Instruction>& Handle::Instructions() const noexcept
        {
            return impl_->Instructions();
        }

        const std::map<std::string, OperandType>& Handle::OperandTypes() const noexcept
        {
            return impl_->OperandTypes();
        }

        const Architecture& Handle::Arch() const noexcept
        {
            return impl_->GetArchitecture();
        }

    }  // namespace explorer

}  // namespace amdisa
