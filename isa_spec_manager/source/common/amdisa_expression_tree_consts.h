/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#ifndef AMDISA_EXPRESSION_TREE_CONSTS_H_
#define AMDISA_EXPRESSION_TREE_CONSTS_H_

// C++ libraries.
#include <map>
#include <string>

namespace amdisa
{
    // Expression tree node type enum.
    enum class ExpressionTreeNodes
    {
        kLiteral,
        kId,
        kComment,
        kReturnType,
        kTypeTree,
        kSubexpression,
        kUndefined
    };

    // Type tree enum.
    enum class TypeTreeNodes
    {
        kBaseType,
        kArrayType,
        kLambdaType,
        kUndefined
    };

    // JSON leaf node identifiers.
    static const char* kJsonLiteralIdentifier   = "'lit";
    static const char* kJsonValueTypeIdentifier = "'ty";
    static const char* kJsonIdIdentifier        = "'id";
    static const char* kJsonCommentIdentifier   = ":comment";

    // Return type.
    static const char* kReturnTypeIdentifier = "'ret";

    // XML leaf node identifiers.
    static const char* kXmlLiteralIdentifier    = "Literal";
    static const char* kXmlValueTypeIdentifier  = "ValueType";
    static const char* kXmlIdIdentifier         = "Id";
    static const char* kXmlCommentIdentifier    = "Comment";
    static const char* kXmlReturnTypeIdentifier = "ReturnType";

    // Map from JSON expression tree string names to enum.
    static const std::map<std::string, ExpressionTreeNodes> kMapJsonExpressionTreeNodeNamesToEnum = {
        {kJsonLiteralIdentifier, ExpressionTreeNodes::kLiteral},
        {kReturnTypeIdentifier, ExpressionTreeNodes::kReturnType},
        {kJsonValueTypeIdentifier, ExpressionTreeNodes::kTypeTree},
        {kJsonIdIdentifier, ExpressionTreeNodes::kId},
        {kJsonCommentIdentifier, ExpressionTreeNodes::kComment},
    };

    // Map from XML expression tree string names to enum.
    static const std::map<std::string, ExpressionTreeNodes> kMapXmlExpressionTreeNodeNamesToEnum = {
        {kXmlLiteralIdentifier, ExpressionTreeNodes::kLiteral},
        {kXmlValueTypeIdentifier, ExpressionTreeNodes::kTypeTree},
        {kXmlIdIdentifier, ExpressionTreeNodes::kId},
        {kXmlCommentIdentifier, ExpressionTreeNodes::kComment},
    };

    // JSON type names.
    static const char* kJsonBaseTypeName   = "!base";
    static const char* kJsonArrayTypeName  = "!array";
    static const char* kJsonLambdaTypeName = "!lambda";

    // XML type names.
    static const char* kXmlBaseTypeName   = "Base";
    static const char* kXmlArrayTypeName  = "Array";
    static const char* kXmlLambdaTypeName = "Lambda";

    // Map from JSON type tree string names to enum.
    static const std::map<std::string, TypeTreeNodes> kMapJsonTypeTreeNodeNamesToEnum = {{kJsonBaseTypeName, TypeTreeNodes::kBaseType},
                                                                                         {kJsonArrayTypeName, TypeTreeNodes::kArrayType},
                                                                                         {kJsonLambdaTypeName, TypeTreeNodes::kLambdaType}};

    // Map from XML type tree string names to enum.
    static const std::map<std::string, TypeTreeNodes> kMapXmlTypeTreeNodeNamesToEnum = {{kXmlBaseTypeName, TypeTreeNodes::kBaseType},
                                                                                        {kXmlArrayTypeName, TypeTreeNodes::kArrayType},
                                                                                        {kXmlLambdaTypeName, TypeTreeNodes::kLambdaType}};
}  // namespace amdisa
#endif  // AMDISA_EXPRESSION_TREE_CONSTANTS_H_
