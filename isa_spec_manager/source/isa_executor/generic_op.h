// Copyright (c) 2021-2023 Advanced Micro Devices, Inc. All rights reserved.
#ifndef GENERIC_OP_H_
#define GENERIC_OP_H_

// C++ libraries.
#include <functional>
#include <stdint.h>

namespace amdisa
{
    // Binary operations defined in semantics library.
    enum class Operation
    {
        kUndefined,
        kAdd,
        kSubtract,
        kMultiply,
        kDivide
    };

    // Forward declarations.
    class Generic;

    class GenericOp
    {
    public:
        static void BinOp(const Generic& a, const Generic& b, const Operation op, Generic& c);
        static void Subscript(const Generic& a, const uint32_t index, Generic& c);
        static void Slice(const uint32_t hi, const uint32_t lo, Generic& c);
        static void SignExtend(const uint32_t width, Generic& c);
        static bool CompareEqual(const Generic& a, const Generic& b);

        // Disable constructors and assignment operator.
        GenericOp() = delete;
        GenericOp(const GenericOp&) = delete;
        GenericOp& operator=(const GenericOp&) = delete;
    };
}

#endif