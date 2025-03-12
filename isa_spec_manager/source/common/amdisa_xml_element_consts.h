/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#ifndef AMDISA_XML_ELEMENT_CONSTS_H_
#define AMDISA_XML_ELEMENT_CONSTS_H_

namespace amdisa
{
    // Names of XML elements.
    // Top elements.
    static const char* kElementSpec          = "Spec";
    static const char* kElementDocument      = "Document";
    static const char* kElementCopyright     = "Copyright";
    static const char* kElementSensitivity   = "Sensitivity";
    static const char* kElementDate          = "ReleaseDate";
    static const char* kElementIsa           = "ISA";
    static const char* kElementSchemaVersion = "SchemaVersion";

    // Architecture specific elements.
    static const char* kElementArchitecture     = "Architecture";
    static const char* kElementArchitectureName = "ArchitectureName";
    static const char* kElementArchitectureId   = "ArchitectureId";

    // Encoding specific elements.
    static const char* kElementEncodings              = "Encodings";
    static const char* kElementEncoding               = "Encoding";
    static const char* kElementEncodingIdentifiers    = "EncodingIdentifiers";
    static const char* kElementEncodingIdentifier     = "EncodingIdentifier";
    static const char* kElementEncodingIdentifierMask = "EncodingIdentifierMask";
    static const char* kElementEncodingName           = "EncodingName";
    static const char* kElementEncodingDescription    = "EncodingDescription";
    static const char* kElementEncodingCategory       = "EncodingCategory";
    static const char* kElementMicrocodeFormat        = "MicrocodeFormat";
    static const char* kElementEncodingFields         = "EncodingFields";
    static const char* kElementEncodingConditions     = "EncodingConditions";
    static const char* kElementEncodingCondition      = "EncodingCondition";
    static const char* kElementConditionName          = "ConditionName";
    static const char* kElementConditionExpression    = "CondtionExpression";

    // Field specific elements.
    static const char* kElementField                 = "Field";
    static const char* kElementFieldName             = "FieldName";
    static const char* kElementFieldPredefinedValues = "FieldPredefinedValues";

    // Bit specific elements.
    static const char* kElementBitCount  = "BitCount";
    static const char* kElementBitMap    = "BitMap";
    static const char* kElementBitLayout = "BitLayout";
    static const char* kElementBitOffset = "BitOffset";

    // Expression specific elements.
    static const char* kElementExpression           = "Expression";
    static const char* kElementExpressionOperator   = "Operator";
    static const char* kElementExpressionValue      = "Value";
    static const char* kElementExpressionLabel      = "Label";
    static const char* kElementExpressionComment    = "Comment";
    static const char* kElementSubexpressions       = "Subexpressions";
    static const char* kElementExpressionValueTypes = "ValueTypes";
    static const char* kElementExpressionValueType  = "ValueType";
    static const char* kElementExpressionBaseType   = "BaseType";
    static const char* kElementExpressionArrayType  = "ArrayType";
    static const char* kElementExpressionTypeSize   = "Size";
    static const char* kElementExpressionArraySize  = "ArraySize";
    static const char* kElementExpressionReturnType = "ReturnType";

    // Instruction specific elements.
    static const char* kElementInstructions                 = "Instructions";
    static const char* kElementShaderInstructions           = "ShaderInstructions";
    static const char* kElementInstruction                  = "Instruction";
    static const char* kElementInstructionName              = "InstructionName";
    static const char* kElementAliasedInstructionName       = "AliasedInstructionNames";
    static const char* kElementInstructionDescription       = "InstructionDescription";
    static const char* kElementInstructionOrder             = "InstructionOrder";
    static const char* kElementInstructionSemantics         = "InstructionSemantics";
    static const char* kElementInstructionEncodings         = "InstructionEncodings";
    static const char* kElementInstructionEncoding          = "InstructionEncoding";
    static const char* kElementInstructionBranchInfo        = "InstructionBranchInfo";
    static const char* kElementInstructionBranchOffset      = "InstructionBranchOffset";
    static const char* kElementInstructionBranchTargetPC    = "InstructionBranchTargetPC";
    static const char* kElementInstructionBranchTargetLabel = "InstructionBranchTargetLabel";
    static const char* kElementInstructionBranchTargetIndex = "InstructionBranchTargetIndex";
    static const char* kElementInstructionFlags             = "InstructionFlags";
    static const char* kElementFunctionalGroup              = "FunctionalGroup";
    static const char* kElementExecutionUnit                = "ExecutionUnit";
    static const char* kElementFlagIsBranch                 = "IsBranch";
    static const char* kElementFlagIsConditionalBranch      = "IsConditionalBranch";
    static const char* kElementFlagIsIndirectBranch         = "IsIndirectBranch";
    static const char* kElementFlagIsProgramTerminator      = "IsProgramTerminator";
    static const char* kElementFlagIsImmediatelyExecuted    = "IsImmediatelyExecuted";

    // Data format specific elements.
    static const char* kElementDataFormats    = "DataFormats";
    static const char* kElementDataFormat     = "DataFormat";
    static const char* kElementDataFormatName = "DataFormatName";
    static const char* kElementDataType       = "DataType";
    static const char* kElementDataAttributes = "DataAttributes";

    // Operand type specific elements.
    static const char* kElementOperands                = "Operands";
    static const char* kElementOperand                 = "Operand";
    static const char* kElementOperandTypes            = "OperandTypes";
    static const char* kElementOperandSubtypes         = "Subtypes";
    static const char* kElementOperandSubtype          = "Subtype";
    static const char* kElementOperandSize             = "OperandSize";
    static const char* kElementOperandType             = "OperandType";
    static const char* kElementOperandTypeName         = "OperandTypeName";
    static const char* kElementOperandPredefinedValues = "OperandPredefinedValues";
    static const char* kElementOperandModifier         = "OperandModifier";
    static const char* kElementOperandModifiers        = "OperandModifiers";
    static const char* kElementPredefinedValue         = "PredefinedValue";

    // Padding specific elements.
    static const char* kElementPadding = "Padding";

    // Generic elements.
    static const char* kElementDescription          = "Description";
    static const char* kElementConstant             = "Constant";
    static const char* kElementRange                = "Range";
    static const char* kElementCondition            = "Condition";
    static const char* kElementOpcode               = "Opcode";
    static const char* kElementIsPartitioned        = "IsPartitioned";
    static const char* kElementValue                = "Value";
    static const char* kElementName                 = "Name";
    static const char* kElementComponentCount       = "ComponentCount";
    static const char* kElementBinaryRepresentation = "BinaryRepresentation";
    static const char* kElementIsInstructionDecoded = "IsInstructionDecoded";

    // Attributes of XML elements.
    static const char* kAttributePublishDate               = "PublishDate";
    static const char* kAttributeRadix                     = "Radix";
    static const char* kAttributeIsConditional             = "IsConditional";
    static const char* kAttributeRangeCount                = "RangeCount";
    static const char* kAttributeOrder                     = "Order";
    static const char* kAttributeOperator                  = "Operator";
    static const char* kAttributeInput                     = "Input";
    static const char* kAttributeOutput                    = "Output";
    static const char* kAttributeIsImplicit                = "IsImplicit";
    static const char* kAttributeIsBinaryMicrocodeRequired = "IsBinaryMicrocodeRequired";
    static const char* kAttributeSignedness                = "Signedness";
    static const char* kAttributeIsPartitioned             = "IsPartitioned";
    static const char* kAttributeType                      = "Type";
    static const char* kAttributeTypeName                  = "Name";
    static const char* kAttributeTypeSubgroup              = "Subgroup";
    static const char* kAttributeTypeFunctionalGroups      = "FunctionalGroups";
    static const char* kAttributeTypeFunctionalGroup       = "FunctionalGroup";
    static const char* kAttributeTypeFunctionalSubgroups   = "FunctionalSubgroups";
    static const char* kAttributeTypeFunctionalSubgroup    = "FunctionalSubgroup";
}  // namespace amdisa
#endif  // AMDISA_XML_ELEMENT_CONSTS_H_
