/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#include "isa_xml_reader.h"

// C++ libraries.
#include <algorithm>
#include <cassert>
#include <deque>
#include <set>
#include <sstream>

// Third party libraries.
#include "tinyxml2.h"

// Local classes.
#include "amdisa_expression_tree_consts.h"
#include "amdisa_structures.h"
#include "amdisa_utility.h"
#include "amdisa_xml_element_consts.h"

namespace amdisa
{
    // Constants.
    // Constants related to html/xml trimming.
    static const char* kSingleTagIdentifier = "/>";
    static const std::set<std::string> kTagsToTrip          = { "Description", "p"};

    // Possible names of the opcode field in AMD GPU ISA: OPX and OPY are
    // used in the case of VOPD encoding, OP is used in all other encodings.
    const std::vector<std::string> kOpFieldNameOptions = {"OP", "OPX", "OPY"};

    // Error string constants.
    static const char* kStringErrorXmlReadError = "Error: Failed to read XML file.";
    static const char* kStringErrorXmlReadErrorExpressionTreeTypeAttributeNotFound =
        "Error: Failed to read expression tree. Type attribute of an expression "
        "not found.";
    static const char* kStringErrorXmlReadErrorExpressionTreeExpressionElementParseIssue =
        "Error: Failed to read expression tree. Failed to parse one of the "
        "elements of expression element.";
    static const char* kStringErrorXmlReadErrorTypeTreeParseIssue =
        "Error: Failed to read expression tree. Failed to parse one of the "
        "elements of the type tree.";
    static const char* kStringErrorXmlEmptyFunctionalGroups = 
        "Error : Failed to read XML file. Functional groups information is missing.";
    static const char* kStringErrorXmlEmptyFunctionalSubgroups = 
        "Error : Failed to read XML file. Functional subgroups information is missing.";

    // Type defintions.
    using XmlComment  = tinyxml2::XMLComment;
    using XmlDocument = tinyxml2::XMLDocument;
    using XmlElement  = tinyxml2::XMLElement;
    using XmlError    = tinyxml2::XMLError;
    using XmlNode     = tinyxml2::XMLNode;

    // XML iterator class for convenient tranversal of elements.
    class XmlIterator
    {
    public:
        XmlIterator(XmlElement* start_element)
        {
            current_element_ = start_element;
        };

        bool IsValid() const
        {
            return current_element_ != nullptr;
        }

        // Excplicitly delete constructor.
        XmlIterator() = delete;

        // Operator to advance iterator to the next sibling XML element. This
        // operator does not advance into children.
        XmlIterator* operator++()
        {
            return Next();
        }

        // Operators to dereference the iterator.
        XmlElement* operator->() const
        {
            return current_element_;
        }

        XmlElement* operator*() const
        {
            return current_element_;
        }

    private:
        XmlElement*  current_element_ = nullptr;
        XmlIterator* Next()
        {
            XmlIterator* next_iterator = nullptr;

            if (current_element_ != nullptr)
            {
                current_element_ = current_element_->NextSiblingElement();
                next_iterator    = this;
            }

            return next_iterator;
        }
    };

    // *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - BEGIN ***
    static XmlElement* GetElementByName(const char* xml_element_name, const XmlElement* parent_element)
    {
        XmlElement* ret = nullptr;

        bool is_found = false;
        for (XmlIterator xml_children_iterator = XmlIterator(const_cast<XmlElement*>(parent_element->FirstChildElement()));
             !is_found && xml_children_iterator.IsValid();
             ++xml_children_iterator)
        {
            if (std::strcmp(xml_children_iterator->Name(), xml_element_name) == 0)
            {
                is_found = true;
                ret      = *xml_children_iterator;
            }
        }

        return ret;
    }

    static void TrimHtml(const std::string& html_string, std::string& trimmed_string)
    {
        // Copy the values.
        trimmed_string = html_string;

        // Trim.
        bool is_string_trimmed = false;
        bool should_skip       = false;
        while (!is_string_trimmed && !should_skip)
        {
            // Find opening tag tag.
            size_t open_bracket_pos  = trimmed_string.find("<");
            size_t close_bracket_pos = trimmed_string.find(">");

            // Check if the string is clean from html tags.
            bool is_open_bracket_found  = (open_bracket_pos != std::string::npos);
            bool is_close_bracket_found = (close_bracket_pos != std::string::npos);

            // Get the tag.
            if (is_open_bracket_found && is_close_bracket_found)
            {
                size_t      tag_length = close_bracket_pos - open_bracket_pos + 1;
                std::string tag        = trimmed_string.substr(open_bracket_pos, tag_length);

                tag.erase(std::remove_if(tag.begin(), tag.end(), [](const char c) { return c == '<' || c == '>' || c == '/'; }), tag.end());
                if (kTagsToTrip.count(tag) > 0)
                {
                    trimmed_string.erase(open_bracket_pos, tag_length);

                    // Lines with "/>" does not have closing tags, usually signal a new line.
                    if (tag.find(kSingleTagIdentifier) != std::string::npos)
                    {
                        trimmed_string.insert(open_bracket_pos, "\n");
                    }
                }
                else
                {
                    should_skip = true;
                }
            }
            else
            {
                is_string_trimmed = true;
            }
        }

        // Remove leading spaces.
        std::string to_remove      = " \n\t";
        size_t      first_char_pos = trimmed_string.find_first_not_of(to_remove);
        if (first_char_pos != std::string::npos)
        {
            trimmed_string.erase(0, first_char_pos);
        }

        // Remove trailing spaces.
        size_t last_char_pos = trimmed_string.find_last_not_of(to_remove);
        if (last_char_pos != std::string::npos)
        {
            trimmed_string.erase(last_char_pos + 1);
        }
    }

    static std::string ExtractText(XmlElement* element, bool is_html = false)
    {
        std::string ret;

        if (element != nullptr)
        {
            if (!is_html)
            {
                const char* text = element->GetText();
                if (text != nullptr)
                {
                    ret = text;
                }
            }
            else
            {
                std::stringstream    formatter;
                tinyxml2::XMLPrinter printer;
                element->Accept(&printer);
                formatter << printer.CStr();
                ret = formatter.str();
            }
        }

        return ret;
    }

    static bool ExtractBool(XmlElement* element)
    {
        std::string xml_text = AmdIsaUtility::ToUpper(ExtractText(element));
        return !xml_text.compare("TRUE");
    }

    static Signedness SignednessStringToEnum(const std::string& signedness)
    {
        Signedness ret      = Signedness::kUnknown;
        bool       is_found = false;

        for (auto map_iterator = kSignednessEnumToString.begin(); !is_found && map_iterator != kSignednessEnumToString.end(); ++map_iterator)
        {
            if (map_iterator->second == signedness)
            {
                ret      = map_iterator->first;
                is_found = true;
            }
        }

        return ret;
    }

    static uint64_t ConstructMask(const MicrocodeFormat& microcode_format, const std::string& field_name)
    {
        uint64_t mask = 0;

        const auto& field_iterator =
            std::find_if(microcode_format.bit_map.begin(), microcode_format.bit_map.end(), [&](const Field& field) { return field.name == field_name; });

        if (field_iterator != microcode_format.bit_map.end())
        {
            // Construct the mask of right size.
            mask = (1 << field_iterator->ranges[0].bit_count) - 1;

            // Shift the mask to the corresponding field location.
            mask <<= field_iterator->ranges[0].bit_offset;
        }

        return mask;
    }

    static void AppendStringNode(const std::string& string_value, std::shared_ptr<GenericExpressionNode>& parent_node)
    {
        parent_node->children.push_back(std::make_shared<GenericExpressionNode>());
        auto& node                = parent_node->children.back();
        node->expression_operator = string_value;
    }

    static bool ReadTypeTree(const XmlElement* input_xml_element, std::shared_ptr<GenericExpressionNode>& output_node, std::string& err_message)
    {
        bool should_abort = false;

        std::deque<const XmlElement*>                      nodes_to_read;
        std::deque<std::shared_ptr<GenericExpressionNode>> nodes_to_write;

        // Initialize.
        nodes_to_read.push_back(input_xml_element);
        nodes_to_write.push_back(output_node);

        while (!should_abort && !nodes_to_read.empty())
        {
            const auto& node_to_read  = nodes_to_read.front();
            auto&       node_to_write = nodes_to_write.front();

            // Determine parse state based on the current type.
            const char* value_type = node_to_read->Attribute(kAttributeTypeName);
            if (value_type != nullptr)
            {
                TypeTreeNodes current_node = TypeTreeNodes::kUndefined;
                if (kMapXmlTypeTreeNodeNamesToEnum.count(value_type) > 0)
                {
                    current_node = kMapXmlTypeTreeNodeNamesToEnum.at(value_type);
                }

                switch (current_node)
                {
                case TypeTreeNodes::kBaseType:
                {
                    node_to_write->expression_operator = kJsonBaseTypeName;

                    // Retrieve elements of base type.
                    XmlElement* base_type_element = GetElementByName(kElementExpressionBaseType, node_to_read);
                    XmlElement* size_element      = GetElementByName(kElementExpressionTypeSize, node_to_read);

                    bool is_type_data_retrieved = base_type_element != nullptr && size_element != nullptr;

                    if (is_type_data_retrieved)
                    {
                        // Save base type.
                        AppendStringNode(ExtractText(base_type_element), node_to_write);

                        // Save the size.
                        AppendStringNode(ExtractText(size_element), node_to_write);
                    }
                    else
                    {
                        should_abort = true;
                        err_message  = kStringErrorXmlReadErrorTypeTreeParseIssue;
                    }
                }
                break;

                case TypeTreeNodes::kArrayType:
                {
                    node_to_write->expression_operator = kJsonArrayTypeName;

                    // Retrieve elements from array type.
                    XmlElement* array_type_element = GetElementByName(kElementExpressionArrayType, node_to_read);
                    XmlElement* array_size_element = GetElementByName(kElementExpressionArraySize, node_to_read);

                    bool is_type_data_retrieved = array_type_element != nullptr && array_size_element != nullptr;

                    if (is_type_data_retrieved)
                    {
                        // Place the type into process queue.
                        XmlElement* value_type_element = GetElementByName(kElementExpressionValueType, array_type_element);
                        if (value_type_element != nullptr)
                        {
                            // Allocate space for type of the array.
                            node_to_write->children.push_back(std::make_shared<GenericExpressionNode>());

                            // Place subtype in queue for further tranversal.
                            nodes_to_write.push_front(node_to_write->children.back());
                            nodes_to_read.push_back(value_type_element);
                        }
                        else
                        {
                            should_abort = true;
                            err_message  = kStringErrorXmlReadErrorTypeTreeParseIssue;
                        }

                        // Save the array size.
                        AppendStringNode(ExtractText(array_size_element), node_to_write);
                    }
                    else
                    {
                        should_abort = true;
                        err_message  = kStringErrorXmlReadErrorTypeTreeParseIssue;
                    }
                }
                break;

                case TypeTreeNodes::kLambdaType:
                {
                    node_to_write->expression_operator = kJsonLambdaTypeName;

                    // Retrieve elements from lambda type.
                    XmlElement* value_types_element = GetElementByName(kElementExpressionValueTypes, node_to_read);

                    bool is_type_data_retrieved = (value_types_element != nullptr);

                    if (is_type_data_retrieved)
                    {
                        // Queue subtypes for further traversal.
                        XmlIterator types_iterator = XmlIterator(value_types_element->FirstChildElement());
                        while (types_iterator.IsValid())
                        {
                            node_to_write->children.push_back(std::make_shared<GenericExpressionNode>());
                            nodes_to_write.push_back(node_to_write->children.back());
                            nodes_to_read.push_back(*types_iterator);

                            ++types_iterator;
                        }
                    }
                    else
                    {
                        should_abort = true;
                        err_message  = kStringErrorXmlReadErrorTypeTreeParseIssue;
                    }
                }
                break;

                default:
                    // Should not be here.
                    assert(false);
                    break;
                }
            }
            // Remove the processed nodes from the deque.
            nodes_to_read.pop_front();
            nodes_to_write.pop_front();
        }

        return !should_abort;
    }

    static bool ReadExpressionTree(const XmlElement* input_xml_element, std::shared_ptr<GenericExpressionNode>& output_node, std::string& err_message)
    {
        bool should_abort = false;

        std::deque<XmlElement*>                            nodes_to_read;
        std::deque<std::shared_ptr<GenericExpressionNode>> nodes_to_write;

        // Get root expression element.
        XmlElement* root_expression_element = GetElementByName(kElementExpression, input_xml_element);

        // Intialize.
        nodes_to_read.push_back(root_expression_element);
        nodes_to_write.push_back(output_node);

        // Breadth first traversal of the XML nodes.
        while (!should_abort && !nodes_to_read.empty())
        {
            // Firts in first out.
            const auto& node_to_read  = nodes_to_read.front();
            auto&       node_to_write = nodes_to_write.front();

            // Get the expression type.
            const char* expression_type = node_to_read->Attribute(kAttributeType);
            if (expression_type != nullptr)
            {
                ExpressionTreeNodes current_node = ExpressionTreeNodes::kSubexpression;
                if (kMapXmlExpressionTreeNodeNamesToEnum.count(expression_type) > 0)
                {
                    current_node = kMapXmlExpressionTreeNodeNamesToEnum.at(expression_type);
                }

                // Perform action based on the current node type.
                switch (current_node)
                {
                case ExpressionTreeNodes::kSubexpression:
                {
                    // Retrieve elements of expression.
                    XmlElement* operator_element      = GetElementByName(kElementExpressionOperator, node_to_read);
                    XmlElement* subexpression_element = GetElementByName(kElementSubexpressions, node_to_read);

                    bool is_expression_data_retrieved = operator_element != nullptr && subexpression_element != nullptr;

                    if (is_expression_data_retrieved)
                    {
                        // Save operator.
                        node_to_write->expression_operator = ExtractText(operator_element);

                        // Queue subexpression for further traversal.
                        XmlIterator subexpression_xml_iterator = XmlIterator(subexpression_element->FirstChildElement());
                        while (subexpression_xml_iterator.IsValid())
                        {
                            // Allocate space for child node.
                            node_to_write->children.push_back(std::make_shared<GenericExpressionNode>());

                            // Place the nodes for further traversal.
                            nodes_to_write.push_back(node_to_write->children.back());
                            nodes_to_read.push_back(*subexpression_xml_iterator);
                            ++subexpression_xml_iterator;
                        }
                    }
                    else
                    {
                        should_abort = true;
                        err_message  = kStringErrorXmlReadErrorExpressionTreeExpressionElementParseIssue;
                    }
                }
                break;

                case ExpressionTreeNodes::kLiteral:
                {
                    node_to_write->expression_operator = kJsonLiteralIdentifier;

                    // Retrieve elements of literal expression.
                    XmlElement* value_element      = GetElementByName(kElementExpressionValue, node_to_read);
                    XmlElement* value_type_element = GetElementByName(kElementExpressionValueType, node_to_read);

                    bool is_expression_data_retrieved = value_element != nullptr && value_type_element != nullptr;

                    if (is_expression_data_retrieved)
                    {
                        // Save value.
                        AppendStringNode(ExtractText(value_element), node_to_write);

                        // Save type.
                        node_to_write->children.push_back(std::make_shared<GenericExpressionNode>());
                        should_abort = !(ReadTypeTree(value_type_element, node_to_write->children.back(), err_message));
                    }
                    else
                    {
                        should_abort = true;
                        err_message  = kStringErrorXmlReadErrorExpressionTreeExpressionElementParseIssue;
                    }
                }
                break;

                case ExpressionTreeNodes::kId:
                {
                    node_to_write->expression_operator = kJsonIdIdentifier;

                    // Retrieve elements of id expression.
                    XmlElement* label_element      = GetElementByName(kElementExpressionLabel, node_to_read);
                    XmlElement* value_type_element = GetElementByName(kElementExpressionValueType, node_to_read);

                    // Id expression element may not have value type element.
                    bool is_expression_data_retrieved = (label_element != nullptr);
                    bool has_type                     = (value_type_element != nullptr);
                    if (is_expression_data_retrieved)
                    {
                        // Save label.
                        AppendStringNode(ExtractText(label_element), node_to_write);

                        // Save type if present.
                        if (has_type)
                        {
                            node_to_write->children.push_back(std::make_shared<GenericExpressionNode>());
                            should_abort = !(ReadTypeTree(value_type_element, node_to_write->children.back(), err_message));
                        }
                    }
                    else
                    {
                        should_abort = true;
                        err_message  = kStringErrorXmlReadErrorExpressionTreeExpressionElementParseIssue;
                    }
                }
                break;

                case ExpressionTreeNodes::kTypeTree:
                {
                    node_to_write->expression_operator = kJsonValueTypeIdentifier;

                    // Retrieve elements of type expression.
                    XmlElement* value_type_element = GetElementByName(kElementExpressionValueType, node_to_read);

                    bool is_expression_data_retrieved = (value_type_element != nullptr);
                    if (is_expression_data_retrieved)
                    {
                        // Save type.
                        node_to_write->children.push_back(std::make_shared<GenericExpressionNode>());
                        should_abort = !(ReadTypeTree(value_type_element, node_to_write->children.back(), err_message));
                    }
                    else
                    {
                        should_abort = true;
                        err_message  = kStringErrorXmlReadErrorExpressionTreeExpressionElementParseIssue;
                    }
                }
                break;

                case ExpressionTreeNodes::kComment:
                {
                    node_to_write->expression_operator = kJsonCommentIdentifier;

                    // Retrieve elements of comment expression.
                    XmlElement* comment_element = GetElementByName(kElementExpressionComment, node_to_read);

                    bool is_expression_data_retrieved = (comment_element != nullptr);
                    if (is_expression_data_retrieved)
                    {
                        // Save comment.
                        AppendStringNode(ExtractText(comment_element), node_to_write);
                    }
                    else
                    {
                        should_abort = true;
                        err_message  = kStringErrorXmlReadErrorExpressionTreeExpressionElementParseIssue;
                    }
                }
                break;

                default:
                    break;
                }
            }
            else
            {
                should_abort = true;
                err_message  = kStringErrorXmlReadErrorExpressionTreeTypeAttributeNotFound;
            }

            // Remove processed elements.
            nodes_to_read.pop_front();
            nodes_to_write.pop_front();
        }

        return !should_abort;
    }

    // Reads the predefined values information from the provided predefined_values_element XML element and populates the predefined_values
    static bool ReadPredefinedValues(XmlElement* predefined_values_element, std::vector<PredefinedValue>& predefined_values)
    {
        bool        should_abort                   = false;
        XmlIterator predefined_values_xml_iterator = XmlIterator(predefined_values_element->FirstChildElement());
        while (!should_abort && predefined_values_xml_iterator.IsValid())
        {
            // Retrieve.
            XmlElement* predef_value_name_element        = GetElementByName(kElementName, *predefined_values_xml_iterator);
            XmlElement* predef_value_description_element = GetElementByName(kElementDescription, *predefined_values_xml_iterator);
            XmlElement* predef_value_element             = GetElementByName(kElementValue, *predefined_values_xml_iterator);

            // Check the validity.
            bool is_predef_value_elements_retrieved =
                predef_value_name_element != nullptr && predef_value_description_element != nullptr && predef_value_element != nullptr;

            if (is_predef_value_elements_retrieved)
            {
                // Allocate.
                predefined_values.push_back(PredefinedValue());
                auto& single_predef_value = predefined_values.back();

                // Populate.
                single_predef_value.name        = ExtractText(predef_value_name_element);
                single_predef_value.value       = std::stoi(ExtractText(predef_value_element));
                TrimHtml(ExtractText(predef_value_description_element, true), single_predef_value.description);
            }
            else
            {
                should_abort = true;
            }

            // Increment predefined values iterator.
            ++predefined_values_xml_iterator;
        }

        return !should_abort;
    }

    // Reads the microcode format information from the provided microcode_format XML element and populates the microcode_format
    static bool ReadMicrocodeFormat(XmlElement* microcode_format_element, MicrocodeFormat& microcode_format)
    {
        XmlIterator field_xml_iterator = XmlIterator(microcode_format_element->FirstChildElement()->FirstChildElement());
        bool        should_abort       = !field_xml_iterator.IsValid();
        while (!should_abort && field_xml_iterator.IsValid())
        {
            // Retrieve field XML elements and attributes.
            const char* is_conditional_attribute        = field_xml_iterator->Attribute(kAttributeIsConditional);
            XmlElement* field_name_element              = GetElementByName(kElementFieldName, *field_xml_iterator);
            XmlElement* bit_layout_element              = GetElementByName(kElementBitLayout, *field_xml_iterator);
            XmlElement* description_element             = GetElementByName(kElementDescription, *field_xml_iterator);
            XmlElement* field_predefined_values_element = GetElementByName(kElementFieldPredefinedValues, *field_xml_iterator);

            // Check the validity of the retrieved elements.
            bool is_field_elements_retrieved =
                (is_conditional_attribute != nullptr) && (field_name_element != nullptr) && (bit_layout_element != nullptr) && (description_element != nullptr);

            if (is_field_elements_retrieved)
            {
                // Allocate the field entry.
                microcode_format.bit_map.push_back(Field());
                auto& field = microcode_format.bit_map.back();

                // Populate field information.
                field.name           = ExtractText(field_name_element);
                field.is_conditional = std::strcmp(is_conditional_attribute, "true") == 0;
                field.range_count    = std::stoi(bit_layout_element->Attribute(kAttributeRangeCount));
                TrimHtml(ExtractText(description_element), field.description);

                if (field.range_count > 0)
                {
                    XmlIterator ranges_xml_iterator = XmlIterator(bit_layout_element->FirstChildElement());

                    // Check for the validity of the range iterator.
                    should_abort = !ranges_xml_iterator.IsValid();

                    // Go over individual range.
                    while (!should_abort && ranges_xml_iterator.IsValid())
                    {
                        // Retrieve range XML elements.
                        XmlElement* range_bit_count_element  = GetElementByName(kElementBitCount, *ranges_xml_iterator);
                        XmlElement* range_bit_offset_element = GetElementByName(kElementBitOffset, *ranges_xml_iterator);
                        XmlElement* range_padding_element    = GetElementByName(kElementPadding, *ranges_xml_iterator);

                        // Check the validity of the retrieved.
                        bool is_range_elements_retrieved = range_bit_count_element != nullptr && range_bit_offset_element != nullptr;

                        if (is_range_elements_retrieved)
                        {
                            // Allocate range.
                            field.ranges.push_back(Range());
                            auto& range = field.ranges.back();

                            // Populate range information.
                            range.bit_count  = std::stoi(ExtractText(range_bit_count_element));
                            range.bit_offset = std::stoi(ExtractText(range_bit_offset_element));

                            // Get the padding info, if present.
                            if (range_padding_element != nullptr)
                            {
                                XmlElement* padding_bit_count_element = GetElementByName(kElementBitCount, range_padding_element);
                                XmlElement* padding_value_element     = GetElementByName(kElementValue, range_padding_element);

                                // Check the validity of the retrieved.
                                bool is_padding_elements_retrieved = padding_bit_count_element != nullptr && padding_value_element != nullptr;

                                if (is_padding_elements_retrieved)
                                {
                                    range.padding.bit_count = std::stoi(ExtractText(padding_bit_count_element));
                                    range.padding.value     = std::stoul(ExtractText(padding_value_element), 0, 2);
                                }
                            }
                        }
                        else
                        {
                            should_abort = true;
                        }

                        // Increment ranges iterator
                        ++ranges_xml_iterator;
                    }
                }
                else
                {
                    should_abort = true;
                }

                // Get field predefined values if defined.
                if (field_predefined_values_element != nullptr)
                {
                    bool is_read = ReadPredefinedValues(field_predefined_values_element, field.predefined_values);
                    should_abort = !is_read;
                }
            }
            else
            {
                should_abort = true;
            }

            // Increment fields iterator.
            ++field_xml_iterator;
        }

        return !should_abort;
    }

    // Reads the encodings from the XML's <Encodings> and populates spec_data's encodings
    static bool ReadEncodings(const XmlElement* isa_element, IsaSpec& spec_data, std::string& err_message)
    {
        bool should_abort = false;

        // Get encodings structure.
        auto& encodings = spec_data.encodings;

        // Get encodings XML element.
        XmlElement* encodings_element      = GetElementByName(kElementEncodings, isa_element);
        XmlIterator encodings_xml_iterator = XmlIterator(encodings_element->FirstChildElement());

        // Go over all individual encoding XML elements.
        while (!should_abort && encodings_xml_iterator.IsValid())
        {
            // Retrieve encoding XML elements and attributes.
            uint32_t    encoding_order                   = 0;
            XmlError    encoding_order_read_err          = encodings_xml_iterator->QueryUnsignedAttribute(kAttributeOrder, &encoding_order);
            XmlElement* encoding_identifiers_element     = GetElementByName(kElementEncodingIdentifiers, *encodings_xml_iterator);
            XmlElement* encoding_conditions_element      = GetElementByName(kElementEncodingConditions, *encodings_xml_iterator);
            XmlElement* encoding_identifier_mask_element = GetElementByName(kElementEncodingIdentifierMask, *encodings_xml_iterator);
            XmlElement* bit_count_element                = GetElementByName(kElementBitCount, *encodings_xml_iterator);
            XmlElement* encoding_name_element            = GetElementByName(kElementEncodingName, *encodings_xml_iterator);
            XmlElement* description_element              = GetElementByName(kElementDescription, *encodings_xml_iterator);
            XmlElement* microcode_format_element         = GetElementByName(kElementMicrocodeFormat, *encodings_xml_iterator);

            // Check the validity of the retrieved elements.
            bool is_encoding_data_retrieved = encoding_order_read_err == tinyxml2::XML_SUCCESS && encoding_identifier_mask_element != nullptr &&
                                              encoding_identifiers_element != nullptr && encoding_conditions_element != nullptr &&
                                              bit_count_element != nullptr && encoding_name_element != nullptr && description_element != nullptr &&
                                              microcode_format_element != nullptr;

            if (is_encoding_data_retrieved)
            {
                // Allocate the encoding entry.
                encodings.push_back(Encoding());
                auto& single_encoding = encodings.back();

                // Populate encoding information.
                single_encoding.order       = encoding_order;
                single_encoding.bit_count   = std::stoi(ExtractText(bit_count_element));
                single_encoding.name        = ExtractText(encoding_name_element);
                TrimHtml(ExtractText(description_element, true), single_encoding.description);

                // Handle each encoding identifier.
                XmlIterator encoding_identifier_iterator = XmlIterator(encoding_identifiers_element->FirstChildElement());
                should_abort                             = !encoding_identifier_iterator.IsValid();
                while (!should_abort && encoding_identifier_iterator.IsValid())
                {
                    // Get the Radix attribute.
                    uint32_t radix_value        = 0;
                    XmlError radix_read_err     = encoding_identifier_iterator->QueryUnsignedAttribute(kAttributeRadix, &radix_value);
                    bool     is_radix_retrieved = radix_read_err == tinyxml2::XML_SUCCESS;
                    if (is_radix_retrieved)
                    {
                        uint64_t encoding_identifier_value = std::stoul(ExtractText(*encoding_identifier_iterator), 0, radix_value);
                        single_encoding.identifiers.push_back(encoding_identifier_value);
                    }
                    else
                    {
                        should_abort = true;
                    }

                    // Increment the iterator.
                    ++encoding_identifier_iterator;
                }

                // Handle encoding conditions.
                XmlIterator encoding_condition_iterator = XmlIterator(encoding_conditions_element->FirstChildElement());
                while (!should_abort && encoding_condition_iterator.IsValid())
                {
                    // Retrieve encoding condition data.
                    XmlElement* condition_name_element       = GetElementByName(kElementConditionName, *encoding_condition_iterator);
                    XmlElement* condition_expression_element = GetElementByName(kElementConditionExpression, *encoding_condition_iterator);

                    bool is_condition_retrieved = condition_name_element != nullptr && condition_expression_element != nullptr;

                    if (is_condition_retrieved)
                    {
                        // Allocate the condition entry.
                        single_encoding.conditions.push_back(Condition());
                        auto& condition = single_encoding.conditions.back();

                        // Populate condition information.
                        condition.name      = ExtractText(condition_name_element);
                        auto expression_ptr = std::make_shared<GenericExpressionNode>();
                        bool is_read        = ReadExpressionTree(condition_expression_element, expression_ptr, err_message);
                        if (is_read)
                        {
                            condition.expression = *expression_ptr;
                        }
                    }
                    else
                    {
                        should_abort = true;
                    }

                    ++encoding_condition_iterator;
                }

                // Handle fields in microcode format element.
                bool is_read = ReadMicrocodeFormat(microcode_format_element, single_encoding.microcode_format);

                // Reconstruct the encoding and opcode masks.
                if (is_read)
                {
                    // Construct the masks of different fields of the encoding to form encoding identifier.
                    for (const auto& op_field_name : kOpFieldNameOptions)
                    {
                        // Find the opcode field
                        single_encoding.opcode_mask = ConstructMask(single_encoding.microcode_format, op_field_name);
                        if (single_encoding.opcode_mask > 0)
                        {
                            break;
                        }
                    }
                    single_encoding.mask     = ConstructMask(single_encoding.microcode_format, "ENCODING");
                    single_encoding.seg_mask = ConstructMask(single_encoding.microcode_format, "SEG");
                    if (single_encoding.identifiers.size() > 0)
                    {
                        single_encoding.bits = single_encoding.identifiers[0] & single_encoding.mask;
                    }
                }
                else
                {
                    should_abort = true;
                }
            }
            else
            {
                should_abort = true;
            }

            // Increment encodings iterator.
            ++encodings_xml_iterator;
        }

        if (should_abort)
        {
            err_message = kStringErrorXmlReadError;
        }
        else
        {
            // Sort the encodings based on the decode priority order.
            std::sort(spec_data.encodings.begin(), spec_data.encodings.end(), [&](const Encoding& encoding_a, const Encoding& encoding_b) {
                return encoding_a.order < encoding_b.order;
            });
        }

        return !should_abort;
    }

    // Reads the instructions from the XML's <Instructions> and populates spec_data's instructions
    static bool ReadInstructions(const XmlElement* isa_element, IsaSpec& spec_data, std::string& err_message)
    {
        bool should_abort = false;

        // Get instructions structure.
        auto& instructions = spec_data.instructions;

        // Get instructions XML element.
        XmlElement* instructions_element      = GetElementByName(kElementInstructions, isa_element);
        XmlIterator instructions_xml_iterator = XmlIterator(instructions_element->FirstChildElement());

        // Go over all individual instruction XML elements.
        while (!should_abort && instructions_xml_iterator.IsValid())
        {
            // Retrieve instruction flags.
            XmlElement* instruction_flags_element     = GetElementByName(kElementInstructionFlags, *instructions_xml_iterator);
            XmlElement* is_branch_element             = GetElementByName(kElementFlagIsBranch, instruction_flags_element);
            XmlElement* is_branch_conditional_element = GetElementByName(kElementFlagIsConditionalBranch, instruction_flags_element);
            XmlElement* is_branch_indirect_element    = GetElementByName(kElementFlagIsIndirectBranch, instruction_flags_element);
            XmlElement* is_imm_exec_element           = GetElementByName(kElementFlagIsImmediatelyExecuted, instruction_flags_element);
            XmlElement* is_prog_term_element          = GetElementByName(kElementFlagIsProgramTerminator, instruction_flags_element);

            // Retrieve instruction XML elements.
            XmlElement* instruction_name_element      = GetElementByName(kElementInstructionName, *instructions_xml_iterator);
            XmlElement* description_element           = GetElementByName(kElementDescription, *instructions_xml_iterator);
            XmlElement* instruction_encodings_element = GetElementByName(kElementInstructionEncodings, *instructions_xml_iterator);
            XmlElement* aliased_names_element         = GetElementByName(kElementAliasedInstructionName, *instructions_xml_iterator);

            // Check the validity of the retrieved elements.
            bool is_instruction_data_retrieved = instruction_name_element != nullptr && description_element != nullptr &&
                                                 instruction_encodings_element != nullptr && is_branch_element != nullptr && is_imm_exec_element != nullptr &&
                                                 is_branch_conditional_element != nullptr && is_branch_indirect_element != nullptr &&
                                                 is_prog_term_element != nullptr;

            if (is_instruction_data_retrieved)
            {
                // Allocated the instruction entry.
                instructions.push_back(Instruction());
                auto& single_instruction = instructions.back();

                // Populate the entry.
                single_instruction.name                    = ExtractText(instruction_name_element);
                single_instruction.is_branch               = ExtractBool(is_branch_element);
                single_instruction.is_conditional_branch   = ExtractBool(is_branch_conditional_element);
                single_instruction.is_indirect_branch      = ExtractBool(is_branch_indirect_element);
                single_instruction.is_immediately_executed = ExtractBool(is_imm_exec_element);
                single_instruction.is_program_terminator   = ExtractBool(is_prog_term_element);
                TrimHtml(ExtractText(description_element, true), single_instruction.description);

                // Get aliased names.
                if (aliased_names_element != nullptr)
                {
                    XmlIterator aliased_names_xml_iterator = XmlIterator(aliased_names_element->FirstChildElement());
                    while (aliased_names_xml_iterator.IsValid())
                    {
                        single_instruction.aliased_names.push_back(ExtractText(*aliased_names_xml_iterator));
                        ++aliased_names_xml_iterator;
                    }
                }

                // Populate functional group and subgroup info
                XmlElement* functional_group_element = GetElementByName(kAttributeTypeFunctionalGroup, *instructions_xml_iterator);
                if (functional_group_element != nullptr)
                {
                    XmlElement* functional_group_name_element = GetElementByName(kAttributeTypeName, functional_group_element);
                    if (functional_group_name_element != nullptr)
                    {
                        single_instruction.functional_group_name = ExtractText(functional_group_name_element);
                    }
                    XmlElement* functional_subgroup_name_element = GetElementByName(kAttributeTypeSubgroup, functional_group_element);
                    if (functional_subgroup_name_element != nullptr)
                    {
                        single_instruction.functional_subgroup_name = ExtractText(functional_subgroup_name_element);
                    }
                }

                // Form the instruction encodings iterator.
                XmlIterator encodings_xml_iterator = XmlIterator(instruction_encodings_element->FirstChildElement());

                // Check the validity of the encodings iterator.
                should_abort = !encodings_xml_iterator.IsValid();

                // Go over individual instruction encodings.
                while (!should_abort && encodings_xml_iterator.IsValid())
                {
                    // Retrieve XML elements in the current instruction encoding element.
                    XmlElement* encoding_name_element      = GetElementByName(kElementEncodingName, *encodings_xml_iterator);
                    XmlElement* encoding_condition_element = GetElementByName(kElementEncodingCondition, *encodings_xml_iterator);
                    XmlElement* opcode_element             = GetElementByName(kElementOpcode, *encodings_xml_iterator);
                    XmlElement* operands_element           = GetElementByName(kElementOperands, *encodings_xml_iterator);

                    // Check the validity of the retrieved elements.
                    bool is_encoding_elements_retrieved = encoding_name_element != nullptr && opcode_element != nullptr && operands_element != nullptr;

                    if (is_encoding_elements_retrieved)
                    {
                        // Allocate the encoding entry.
                        single_instruction.encodings.push_back(InstructionEncoding());
                        auto& instruction_encoding = single_instruction.encodings.back();

                        // Populate instruction encoding information.
                        instruction_encoding.name   = ExtractText(encoding_name_element);
                        instruction_encoding.condition_name = ExtractText(encoding_condition_element);
                        instruction_encoding.opcode = std::stoi(ExtractText(opcode_element));

                        // Form the operands iterator. Instruction encoding maynot have
                        // operands.
                        XmlIterator operands_iterator = XmlIterator(operands_element->FirstChildElement());

                        // Go over individual operands.
                        while (!should_abort && operands_iterator.IsValid())
                        {
                            // Retrieve attributes of the current operand.
                            bool     is_input           = false;
                            bool     is_output          = false;
                            bool     is_implicit        = false;
                            bool     is_in_microcode    = false;
                            uint32_t order              = 0;
                            XmlError input_read_err     = operands_iterator->QueryBoolAttribute(kAttributeInput, &is_input);
                            XmlError output_read_err    = operands_iterator->QueryBoolAttribute(kAttributeOutput, &is_output);
                            XmlError implicit_read_err  = operands_iterator->QueryBoolAttribute(kAttributeIsImplicit, &is_implicit);
                            XmlError singleton_read_err = operands_iterator->QueryBoolAttribute(kAttributeIsBinaryMicrocodeRequired, &is_in_microcode);
                            XmlError order_read_err     = operands_iterator->QueryUnsignedAttribute(kAttributeOrder, &order);

                            // Retrieve XML elements in the current operand.
                            XmlElement* field_name_element      = GetElementByName(kElementFieldName, *operands_iterator);
                            XmlElement* dataformat_name_element = GetElementByName(kElementDataFormatName, *operands_iterator);
                            XmlElement* operand_type_element    = GetElementByName(kElementOperandType, *operands_iterator);
                            XmlElement* operand_size_element    = GetElementByName(kElementOperandSize, *operands_iterator);

                            // Check the validity of the retrieved.
                            bool is_operand_information_retrieved = input_read_err == tinyxml2::XML_SUCCESS && output_read_err == tinyxml2::XML_SUCCESS &&
                                                                    implicit_read_err == tinyxml2::XML_SUCCESS && singleton_read_err == tinyxml2::XML_SUCCESS &&
                                                                    order_read_err == tinyxml2::XML_SUCCESS && dataformat_name_element != nullptr &&
                                                                    operand_type_element != nullptr && operand_size_element != nullptr &&
                                                                    (field_name_element != nullptr || is_implicit || !is_in_microcode);

                            if (is_operand_information_retrieved)
                            {
                                // Allocate the operand entry.
                                instruction_encoding.operands.push_back(Operand());
                                auto& operand = instruction_encoding.operands.back();

                                // Populate the operand information.
                                operand.input               = is_input;
                                operand.output              = is_output;
                                operand.is_implicit         = is_implicit;
                                operand.is_in_microcode     = is_in_microcode;
                                operand.order               = order;
                                operand.encoding_field_name = ExtractText(field_name_element);
                                operand.data_format         = ExtractText(dataformat_name_element);
                                operand.type                = ExtractText(operand_type_element);
                                operand.size                = std::stoi(ExtractText(operand_size_element));
                            }
                            else
                            {
                                should_abort = true;
                            }

                            // Increment the operands iterator.
                            ++operands_iterator;
                        }
                    }
                    else
                    {
                        should_abort = true;
                    }

                    // Increment the instruction encodings iterator.
                    ++encodings_xml_iterator;
                }
            }
            else
            {
                should_abort = true;
            }

            // Increment the instruction iterator.
            ++instructions_xml_iterator;
        }

        if (should_abort)
        {
            err_message = kStringErrorXmlReadError;
        }

        return !should_abort;
    }

    // Reads the data formats from the XML's <DataFormats> to spec_data's data formats
    static bool ReadDataFormats(const XmlElement* isa_element, IsaSpec& spec_data, std::string& err_message)
    {
        bool should_abort = false;

        // Get data formats structure.
        auto& dataformats = spec_data.data_formats;

        // Get the data format XML element.
        XmlElement* dataformats_element      = GetElementByName(kElementDataFormats, isa_element);
        XmlIterator dataformats_xml_iterator = XmlIterator(dataformats_element->FirstChildElement());

        // Go over all individual data format XML elements.
        while (!should_abort && dataformats_xml_iterator.IsValid())
        {
            // Retrieve XML elements of the current data format.
            XmlElement* dataformat_name_element = GetElementByName(kElementDataFormatName, *dataformats_xml_iterator);
            XmlElement* description_element     = GetElementByName(kElementDescription, *dataformats_xml_iterator);
            XmlElement* data_type_element       = GetElementByName(kElementDataType, *dataformats_xml_iterator);
            XmlElement* bit_count_element       = GetElementByName(kElementBitCount, *dataformats_xml_iterator);
            XmlElement* component_count_element = GetElementByName(kElementComponentCount, *dataformats_xml_iterator);
            XmlElement* data_attributes_element = GetElementByName(kElementDataAttributes, *dataformats_xml_iterator);
            XmlIterator bitmap_xml_iterator     = XmlIterator(data_attributes_element->FirstChildElement());

            // Check the validity of the retrieved elements.
            bool is_dataformat_elements_retrieved = dataformat_name_element != nullptr && description_element != nullptr && data_type_element != nullptr &&
                                                    bit_count_element != nullptr && component_count_element != nullptr && data_attributes_element != nullptr &&
                                                    bitmap_xml_iterator.IsValid();

            if (is_dataformat_elements_retrieved)
            {
                // Allocate the data format entry.
                dataformats.push_back(DataFormat());
                auto& single_dataformat = dataformats.back();

                // Populate the data format entry.
                single_dataformat.name            = ExtractText(dataformat_name_element);
                TrimHtml(ExtractText(description_element, true), single_dataformat.description);
                single_dataformat.data_type       = kStringDataTypeToEnum.at(ExtractText(data_type_element));
                single_dataformat.bit_count       = std::stoi(ExtractText(bit_count_element));
                single_dataformat.component_count = std::stoi(ExtractText(component_count_element));

                // Go over individual bitmaps.
                while (!should_abort && bitmap_xml_iterator.IsValid())
                {
                    // Retrieve.
                    uint32_t    order;
                    XmlError    order_read_err      = bitmap_xml_iterator->QueryUnsignedAttribute(kAttributeOrder, &order);
                    XmlIterator fields_xml_iterator = XmlIterator(bitmap_xml_iterator->FirstChildElement());

                    // Check the validity of the retrieved.
                    bool is_bitmap_data_retrieved = order_read_err == tinyxml2::XML_SUCCESS && fields_xml_iterator.IsValid();

                    if (is_bitmap_data_retrieved)
                    {
                        // Allocate.
                        single_dataformat.data_attributes.push_back(DataAttributes());
                        auto& data_attributes = single_dataformat.data_attributes.back();

                        // Populate
                        data_attributes.order = order;

                        // Go over individual fields.
                        while (!should_abort && fields_xml_iterator.IsValid())
                        {
                            // Retrieve.
                            const char* signedness_attribute;
                            XmlError    read_signedness_err = fields_xml_iterator->QueryStringAttribute(kAttributeSignedness, &signedness_attribute);
                            XmlElement* field_name_element  = GetElementByName(kElementFieldName, *fields_xml_iterator);
                            XmlElement* bit_layout_element  = GetElementByName(kElementBitLayout, *fields_xml_iterator);
                            XmlIterator range_xml_iterator  = XmlIterator(bit_layout_element->FirstChildElement());

                            // Check the validity of the retrieved.
                            bool is_field_data_retrieved = read_signedness_err == tinyxml2::XML_SUCCESS && field_name_element != nullptr &&
                                                           bit_layout_element != nullptr && range_xml_iterator.IsValid();

                            if (is_field_data_retrieved)
                            {
                                // Allocate.
                                data_attributes.bit_map.push_back(Field());
                                auto& field = data_attributes.bit_map.back();

                                // Populate.
                                field.signedness = SignednessStringToEnum(signedness_attribute);
                                field.name       = ExtractText(field_name_element);

                                // Go over individual range.
                                while (!should_abort && range_xml_iterator.IsValid())
                                {
                                    // Retrieve.
                                    uint32_t    range_order;
                                    XmlError    range_order_read_err = range_xml_iterator->QueryUnsignedAttribute(kAttributeOrder, &range_order);
                                    XmlElement* bit_count_element    = GetElementByName(kElementBitCount, *range_xml_iterator);
                                    XmlElement* bit_offset_element   = GetElementByName(kElementBitOffset, *range_xml_iterator);

                                    // Check the validity of the retrieved.
                                    bool is_range_data_retrieved =
                                        range_order_read_err == tinyxml2::XML_SUCCESS && bit_count_element != nullptr && bit_offset_element != nullptr;

                                    if (is_range_data_retrieved)
                                    {
                                        // Allocate.
                                        field.ranges.push_back(Range());
                                        auto& range = field.ranges.back();

                                        // Populate.
                                        range.order      = range_order;
                                        range.bit_count  = std::stoi(ExtractText(bit_count_element));
                                        range.bit_offset = std::stoi(ExtractText(bit_offset_element));
                                    }
                                    else
                                    {
                                        should_abort = true;
                                    }

                                    // Increment range iterator.
                                    ++range_xml_iterator;
                                }
                            }
                            else
                            {
                                should_abort = true;
                            }

                            // Increment fields iterator.
                            ++fields_xml_iterator;
                        }
                    }
                    else
                    {
                        should_abort = true;
                    }

                    // Increment the bitmap iterator.
                    ++bitmap_xml_iterator;
                }
            }
            else
            {
                should_abort = true;
            }

            // Increment data formats iterator.
            ++dataformats_xml_iterator;
        }

        if (should_abort)
        {
            err_message = kStringErrorXmlReadError;
        }

        return !should_abort;
    }

    // Reads the operand types from the XML's <OperandTypes> and populates spec_data's operand types
    static bool ReadOperandTypes(const XmlElement* isa_element, IsaSpec& spec_data, std::string& err_message)
    {
        bool should_abort = false;

        // Get operand types structure.
        auto& operand_types = spec_data.operand_types;

        // Get the data operand type XML element.
        XmlElement* operand_types_element      = GetElementByName(kElementOperandTypes, isa_element);
        XmlIterator operand_types_xml_iterator = XmlIterator(operand_types_element->FirstChildElement());

        // Go over all individual operand types.
        while (!should_abort && operand_types_xml_iterator.IsValid())
        {
            // Retrieve XML elements of the current operand type.
            const char* is_partitioned_attribute  = operand_types_xml_iterator->Attribute(kAttributeIsPartitioned);
            XmlElement* operand_type_name_element = GetElementByName(kElementOperandTypeName, *operand_types_xml_iterator);
            XmlElement* description_element       = GetElementByName(kElementDescription, *operand_types_xml_iterator);
            XmlElement* subtypes_element          = GetElementByName(kElementOperandSubtypes, *operand_types_xml_iterator);

            // Check the validity of the retrieved elements.
            bool is_optype_elements_retrieved = is_partitioned_attribute != nullptr && operand_type_name_element != nullptr && description_element != nullptr;

            if (is_optype_elements_retrieved)
            {
                // Allocate.
                operand_types.push_back(OperandType());
                auto& single_operand_type = operand_types.back();

                // Populate.
                single_operand_type.name           = ExtractText(operand_type_name_element);
                TrimHtml(ExtractText(description_element, true), single_operand_type.description);
                single_operand_type.is_partitioned = std::strcmp(is_partitioned_attribute, "true") == 0;

                // Subtype names.
                if (subtypes_element != nullptr)
                {
                    XmlIterator subtype_iterator = XmlIterator(subtypes_element->FirstChildElement());
                    while (subtype_iterator.IsValid())
                    {
                        single_operand_type.subtype_names.push_back(ExtractText(*subtype_iterator));
                        ++subtype_iterator;
                    }
                }

                // Go over individual predefined values.
                XmlElement* operand_predefined_values_element = GetElementByName(kElementOperandPredefinedValues, *operand_types_xml_iterator);
                if (operand_predefined_values_element != nullptr)
                {
                    bool is_read = ReadPredefinedValues(operand_predefined_values_element, single_operand_type.predefined_values);
                    should_abort = !is_read;
                }

                if (single_operand_type.is_partitioned)
                {
                    XmlElement* microcode_format_element = GetElementByName(kElementMicrocodeFormat, *operand_types_xml_iterator);
                    if (microcode_format_element != nullptr)
                    {
                        bool is_read = ReadMicrocodeFormat(microcode_format_element, single_operand_type.microcode_format);
                        should_abort = !is_read;
                    }
                    else
                    {
                        should_abort = true;
                    }
                }
            }
            else
            {
                should_abort = true;
            }

            // Increment operand type iterator.
            ++operand_types_xml_iterator;
        }

        if (should_abort)
        {
            err_message = kStringErrorXmlReadError;
        }

        return !should_abort;
    }

    // Reads the functional group and subgroup information from the XML's <FunctionalGroups> and populates spec_data's functional_group_info
    bool ReadFunctionalGroupInfo(const XmlElement* isa_element, IsaSpec& spec_data, std::string& err_message)
    {
        bool should_abort = false;

        XmlElement* functionalgroups_element     = GetElementByName(kAttributeTypeFunctionalGroups, isa_element);
        XmlIterator functionalgroup_xml_iterator = nullptr;
        if (functionalgroups_element != nullptr)
        {
            functionalgroup_xml_iterator = XmlIterator(functionalgroups_element->FirstChildElement());
        }
        else
        {
            should_abort = true;
            err_message  = kStringErrorXmlEmptyFunctionalGroups;
            assert(false);
        }

        // Go over all individual data format XML elements.
        while (!should_abort && functionalgroup_xml_iterator.IsValid())
        {
            FunctionalGroupInfo info;
            XmlElement*         name_element        = GetElementByName(kElementName, *functionalgroup_xml_iterator);
            XmlElement*         description_element = GetElementByName(kElementDescription, *functionalgroup_xml_iterator);

            info.name = ExtractText(name_element);
            info.desc = ExtractText(description_element);

            spec_data.functional_group_info.push_back(info);

            // Increment operand type iterator.
            ++functionalgroup_xml_iterator;
        }

        XmlElement* functionalsubgroups_element     = GetElementByName(kAttributeTypeFunctionalSubgroups, isa_element);
        XmlIterator functionalsubgroup_xml_iterator = nullptr;
        if (functionalsubgroups_element != nullptr)
        {
            functionalsubgroup_xml_iterator = XmlIterator(functionalsubgroups_element->FirstChildElement());
        }
        else {
            should_abort = true;
            err_message  = kStringErrorXmlEmptyFunctionalSubgroups;
            assert(false);
        }

        // Go over all individual data format XML elements.
        while (!should_abort && functionalsubgroup_xml_iterator.IsValid())
        {
            FunctionalSubgroupInfo info;
            XmlElement*            name_element     = GetElementByName(kElementName, *functionalsubgroup_xml_iterator);

            info.name = ExtractText(name_element);

            spec_data.functional_subgroup_info.push_back(info);

            // Increment operand type iterator.
            ++functionalsubgroup_xml_iterator;
        }

        return !should_abort;
    }

    // *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - END ***

    bool IsaXmlReader::ReadSpec(const std::string& path_to_input_xml, IsaSpec& spec_data, std::string& err_message)
    {
        bool is_read_successful = false;

        // Load the XML.
        XmlDocument    xml_document;
        const XmlError err = xml_document.LoadFile(path_to_input_xml.c_str());
        if (err == tinyxml2::XML_SUCCESS)
        {
            is_read_successful = true;
        }
        else
        {
            is_read_successful = false;
            err_message        = kStringErrorXmlReadError;
        }

        // Get non-ISA related information.
        if (is_read_successful)
        {
            XmlElement* document_element       = GetElementByName(kElementDocument, xml_document.FirstChildElement());
            XmlElement* copyright_element      = GetElementByName(kElementCopyright, document_element);
            XmlElement* sensitivity_element    = GetElementByName(kElementSensitivity, document_element);
            XmlElement* date_element           = GetElementByName(kElementDate, document_element);
            XmlElement* schema_version_element = GetElementByName(kElementSchemaVersion, document_element);

            bool is_retrieve_successful = document_element != nullptr && copyright_element != nullptr && sensitivity_element != nullptr &&
                                          date_element != nullptr && schema_version_element != nullptr;

            if (is_retrieve_successful)
            {
                spec_data.info.copyright      = ExtractText(copyright_element);
                spec_data.info.sensitivity    = ExtractText(sensitivity_element);
                spec_data.info.date           = ExtractText(date_element);
                spec_data.info.schema_version = ExtractText(schema_version_element);
            }
            else
            {
                is_read_successful = false;
                err_message        = kStringErrorXmlReadError;
            }
        }

        // Get ISA element.
        XmlElement* isa_element;
        if (is_read_successful)
        {
            isa_element = GetElementByName(kElementIsa, xml_document.FirstChildElement());
            if (isa_element == nullptr)
            {
                is_read_successful = false;
                err_message        = kStringErrorXmlReadError;
            }
        }

        // Get architecture name.
        if (is_read_successful)
        {
            XmlElement* architecture_element      = GetElementByName(kElementArchitecture, isa_element);
            XmlElement* architecture_name_element = GetElementByName(kElementArchitectureName, architecture_element);
            XmlElement* architecture_id_element = GetElementByName(kElementArchitectureId, architecture_element);

            bool is_retrieve_successful = architecture_element != nullptr && architecture_name_element != nullptr &&
                architecture_id_element != nullptr;

            if (is_retrieve_successful)
            {
                spec_data.architecture.name = AmdIsaUtility::ToUpper(ExtractText(architecture_name_element));
                spec_data.architecture.id = std::stoi(ExtractText(architecture_id_element));
            }
            else
            {
                is_read_successful = false;
                err_message        = kStringErrorXmlReadError;
            }
        }

        // Extract encodings.
        if (is_read_successful)
        {
            is_read_successful = ReadEncodings(isa_element, spec_data, err_message);
        }

        // Extract instructions.
        if (is_read_successful)
        {
            is_read_successful = ReadInstructions(isa_element, spec_data, err_message);
        }

        // Extract data formats.
        if (is_read_successful)
        {
            is_read_successful = ReadDataFormats(isa_element, spec_data, err_message);
        }

        // Extract operand types.
        if (is_read_successful)
        {
            is_read_successful = ReadOperandTypes(isa_element, spec_data, err_message);
        }

        if (is_read_successful)
        {
            is_read_successful = ReadFunctionalGroupInfo(isa_element, spec_data, err_message);
        }

        return is_read_successful;
    }
}  // namespace amdisa
