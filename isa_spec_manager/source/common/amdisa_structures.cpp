#include "amdisa_structures.h"

#include <sstream>

namespace amdisa
{

    std::string ToString(const DataType data_type)
    {
        switch (data_type)
        {
        case DataType::kInteger:
            return "Integer";

        case DataType::kFloat:
            return "Float";

        case DataType::kBits:
            return "Bits";

        case DataType::kDescriptor:
            return "Descriptor";

        default:
            return "Unknown";
        }
    }

    std::string ToString(const Signedness signedness)
    {
        switch (signedness)
        {
        case Signedness::kDoesNotApply:
            return "Does not apply";

        case Signedness::kUnsigned:
            return "Unsigned";

        case Signedness::kSigned:
            return "Signed";

        case Signedness::kSignedByModifier:
            return "Signed by modifier";

        default:
            return "Unknown";
        }
    }

    Padding::operator std::string() const noexcept
    {
        std::stringstream s_stream;
        s_stream << "padding bit count : " << bit_count << ' ';
        s_stream << "padding value : " << value;
        return s_stream.str();
    }

    std::ostream& operator<<(std::ostream& out_stream, const Padding& pad)
    {
        out_stream << std::string(pad);
        return out_stream;
    }

    Range::operator std::string() const noexcept
    {
        std::stringstream s_stream;
        s_stream << "-- range-begin--\n";

        s_stream << "order : " << order << '\n';
        s_stream << "bit count : " << bit_count << '\n';
        s_stream << "bit offset : " << bit_offset << '\n';
        s_stream << "padding : ";
        s_stream << padding;
        s_stream << '\n';

        s_stream << "-- range-end --\n";
        return s_stream.str();
    }
    std::ostream& operator<<(std::ostream& out_stream, const Range& range)
    {
        out_stream << std::string(range);
        return out_stream;
    }

    Field::operator std::string() const noexcept
    {
        std::stringstream s_stream;
        s_stream << "-- field-begin --\n";

        s_stream << "field name : " << name << '\n';
        s_stream << "description : " << description << '\n';
        s_stream << "conditional : " << (is_conditional ? "yes" : "no") << '\n';
        s_stream << "constant : " << (is_constant ? "yes" : "no") << '\n';
        s_stream << "range count : " << range_count << '\n';

        s_stream << "-- ranges-begin --\n";
        for (auto& range : ranges)
        {
            s_stream << range;
            s_stream << '\n';
        }
        s_stream << "-- ranges-end --\n";

        s_stream << "signedness : " << amdisa::ToString(signedness) << '\n';

        s_stream << "-- field-end --\n";
        return s_stream.str();
    }

    std::ostream& operator<<(std::ostream& out_stream, const Field& field)
    {
        out_stream << std::string(field);
        return out_stream;
    }

    MicrocodeFormat::operator std::string() const noexcept
    {
        std::stringstream s_stream;
        s_stream << "-- microcode-format-begin --\n";

        for (auto field : bit_map)
        {
            s_stream << std::string(field);
            s_stream << '\n';
        }

        s_stream << "-- microcode-format-end --\n";
        return s_stream.str();
    }

    std::ostream& operator<<(std::ostream& out_stream, const MicrocodeFormat& uformat)
    {
        out_stream << std::string(uformat);
        return out_stream;
    }

    Encoding::operator std::string() const noexcept
    {
        std::stringstream s_stream;
        s_stream << "-- encoding-begin --\n";

        s_stream << "name : " << name << '\n';
        s_stream << "description : " << description << '\n';
        s_stream << "category : " << category << '\n';

        s_stream << "encoding identifiers : begin\n";
        s_stream << std::hex;
        for (auto encoding_identifier : identifiers)
        {
            s_stream << encoding_identifier << '\n';
        }
        s_stream << "encoding identifiers : end\n";

        s_stream << "encoding bits : " << bits << '\n';
        s_stream << "encoding mask : " << mask << '\n';
        s_stream << "opcode mask : " << opcode_mask << '\n';

        s_stream << std::dec;
        s_stream << "bit count : " << bit_count << '\n';
        s_stream << "encoding order : " << order << '\n';
        s_stream << "microcode format : " << microcode_format << "\n";

        s_stream << "-- encoding-end --\n";
        return s_stream.str();
    }

    std::ostream& operator<<(std::ostream& out_stream, const Encoding& encoding)
    {
        out_stream << std::string(encoding);
        return out_stream;
    }

    Operand::operator std::string() const noexcept
    {
        std::stringstream s_stream;
        s_stream << "-- operand-begin --\n";

        s_stream << "field name : " << field_name << '\n';
        s_stream << "data format : " << data_format << '\n';
        s_stream << "operand type : " << type << '\n';
        s_stream << "encoding field name : " << encoding_field_name << '\n';
        s_stream << "order : " << order << '\n';
        s_stream << "input : " << (input ? "yes" : "no") << '\n';
        s_stream << "output : " << (output ? "yes" : "no") << '\n';
        s_stream << "implicit " << (is_implicit ? "yes" : "no") << '\n';
        s_stream << "binary microcode required : " << (is_in_microcode ? "yes" : "no") << '\n';

        s_stream << "-- operand-end --\n";
        return s_stream.str();
    }
    std::ostream& operator<<(std::ostream& out_stream, const Operand& operand)
    {
        out_stream << std::string(operand);
        return out_stream;
    }

    InstructionEncoding::operator std::string() const noexcept
    {
        std::stringstream s_stream;
        s_stream << "-- instruction-encoding-begin --\n";

        s_stream << "name : " << name << '\n';
        s_stream << "opcode : " << opcode << '\n';

        s_stream << "operands : begin\n";
        for (auto& operand : operands)
        {
            s_stream << operand;
            s_stream << '\n';
        }
        s_stream << "operands : end\n";

        s_stream << "-- instruction-encoding-end --\n";
        return s_stream.str();
    }

    std::ostream& operator<<(std::ostream& out_stream, const InstructionEncoding& encoding)
    {
        out_stream << std::string(encoding);
        return out_stream;
    }

    Instruction::operator std::string() const noexcept
    {
        std::stringstream s_stream;
        s_stream << "-- instruction-Dump-begin --\n";

        s_stream << "name : " << name << '\n';
        s_stream << "description : " << description << '\n';

        s_stream << "is branch : " << (is_branch ? "yes" : "no") << '\n';
        s_stream << "is immediately executed : " << (is_immediately_executed ? "yes" : "no") << '\n';
        s_stream << "is program terminator : " << (is_program_terminator ? "yes" : "no") << '\n';

        s_stream << "encodings : begin\n";
        for (const auto& encoding : encodings)
        {
            s_stream << encoding << '\n';
        }
        s_stream << "encodings : end\n";

        // TODO: semantics

        s_stream << "-- instruction-Dump-end --\n";
        return s_stream.str();
    }

    std::ostream& operator<<(std::ostream& out_stream, const Instruction& instruction)
    {
        out_stream << std::string(instruction);
        return out_stream;
    }

    DataAttributes::operator std::string() const noexcept
    {
        std::stringstream s_stream;
        s_stream << "-- data-attributes-begin --\n";

        s_stream << "bit map : begin\n";
        for (const auto& field : bit_map)
        {
            s_stream << field << '\n';
        }
        s_stream << "bit map : end\n";

        s_stream << "-- data-attributes-end --\n";
        return s_stream.str();
    }

    std::ostream& operator<<(std::ostream& out_stream, const DataAttributes& attributes)
    {
        out_stream << std::string(attributes);
        return out_stream;
    }

    DataFormat::operator std::string() const noexcept
    {
        std::stringstream s_stream;
        s_stream << "-- data-format-begin --";

        s_stream << "name : " << name << '\n';
        s_stream << "description : " << description << '\n';

        s_stream << "data type : ";
        s_stream << amdisa::ToString(data_type);
        s_stream << '\n';

        s_stream << "bit count : " << bit_count << '\n';
        s_stream << "component count : " << component_count << '\n';

        s_stream << "data attributes : begin\n";
        for (const auto& attribute : data_attributes)
        {
            s_stream << attribute << '\n';
        }
        s_stream << "data attributes : end\n";

        s_stream << "-- data-format-end --";
        return s_stream.str();
    }

    std::ostream& operator<<(std::ostream& out_stream, const DataFormat& data_format)
    {
        out_stream << std::string(data_format);
        return out_stream;
    }

    PredefinedValue::operator std::string() const noexcept
    {
        std::stringstream s_stream;
        s_stream << "-- operand-predefined-value-begin --\n";

        s_stream << "name : " << name << '\n';
        s_stream << "description : " << description << '\n';
        s_stream << "value : " << value << '\n';

        s_stream << "-- operand-predefined-value-begin --\n";
        return s_stream.str();
    }

    std::ostream& operator<<(std::ostream& out_stream, const PredefinedValue& predefined_value)
    {
        out_stream << std::string(predefined_value);
        return out_stream;
    }

    OperandType::operator std::string() const noexcept
    {
        std::stringstream s_stream;
        s_stream << "-- operand-type-begin --\n";

        s_stream << "operand type name : " << name << '\n';
        s_stream << "description : " << description << '\n';
        s_stream << "predefined values : begin\n";
        for (const auto& value : predefined_values)
        {
            s_stream << value << '\n';
        }
        s_stream << "predefined values : end\n";

        s_stream << "-- operand-type-end --\n";
        return s_stream.str();
    }

    std::ostream& operator<<(std::ostream& out_stream, const OperandType& operand_type)
    {
        out_stream << std::string(operand_type);
        return out_stream;
    }

    IsaSpec::operator std::string() const noexcept
    {
        std::stringstream s_stream;
        s_stream << "-- isa-spec-internal-begin --\n";

        s_stream << "architecture : " << architecture.name;

        s_stream << "encodings : begin\n";
        for (const auto& encoding : encodings)
        {
            s_stream << encoding << '\n';
        }
        s_stream << "encodings : end\n";

        s_stream << "instructions : begin\n";
        for (const auto& inst : instructions)
        {
            s_stream << inst << '\n';
        }
        s_stream << "instructions : end\n";

        s_stream << "data formats : begin\n";
        for (const auto& format : data_formats)
        {
            s_stream << format << '\n';
        }
        s_stream << "data formats : end\n";

        s_stream << "operand types : begin\n";
        for (const auto& type : operand_types)
        {
            s_stream << type << '\n';
        }
        s_stream << "operand types : end\n";

        s_stream << "-- isa-spec-internal-end --\n";
        return s_stream.str();
    }

    std::ostream& operator<<(std::ostream& out_stream, const IsaSpec& isa_spec)
    {
        out_stream << std::string(isa_spec);
        return out_stream;
    }
}  // namespace amdisa
