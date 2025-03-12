// Copyright (c) 2021-2023 Advanced Micro Devices, Inc. All rights reserved.
#include "generic_op.h"

// C++ libraries.
#include <cassert>
#include <functional>
#include <map>

// Local libraries.
#include "generic.h"

namespace amdisa
{
    // Define binary operator functions.
    struct Computation
    {
        std::function<float(float, float)> f32;
        std::function<double(double, double)> f64;
        std::function<uint64_t(uint64_t, uint64_t)> u64;
        std::function<int64_t(int64_t, int64_t)> i64;
    };

    template <typename T>
    T Add(T a, T b)
    {
        return a + b;
    }

    template <typename T>
    T Subtract(T a, T b)
    {
        return a - b;
    }

    template <typename T>
    T Multiply(T a, T b)
    {
        return a * b;
    }

    const static Computation kComputeAdd = { Add<float>, Add<double>, Add<uint64_t>, Add<int64_t> };
    const static Computation kComputeSubtract = { Subtract<float>, Subtract<double>, Subtract<uint64_t>, Subtract<int64_t> };
    const static Computation kComputeMultiply = { Multiply<float>, Multiply<double>, Multiply<uint64_t>, Multiply<int64_t> };

    const static std::map<Operation, Computation> kComputes = {
        { Operation::kAdd, kComputeAdd },
        { Operation::kSubtract, kComputeSubtract },
        { Operation::kMultiply, kComputeMultiply }
    };

    void GenericOp::BinOp(const Generic& a, const Generic& b, const Operation op, Generic& c)
    {
        // Checks.
        assert(a.type_ != Type::kUndefined && b.type_ != Type::kUndefined);

        // Get the compute function for the operation.
        const auto& compute_iter = kComputes.find(op);

        // Perform operation.
        if (a.type_ == b.type_ && a.bit_width_ == b.bit_width_ && compute_iter != kComputes.end())
        {
            const auto& compute = compute_iter->second;
            c.type_ = a.type_;
            c.bit_width_ = a.bit_width_;
            if (a.type_ == Type::kFloat)
            {
                switch (a.bit_width_)
                {
                case 32:
                    c.data_.f32 = compute.f32(a.data_.f32, b.data_.f32);
                    break;
                case 64:
                    c.data_.f64 = compute.f64(a.data_.f64, b.data_.f64);
                default:
                    assert(false);
                    break;
                }
            }
            else
            {
                if (a.is_signed_ || b.is_signed_)
                {
                    c.data_.i64 = compute.i64(a.data_.i64, b.data_.i64);
                    c.is_signed_ = true;
                }
                else
                {
                    c.data_.u64 = compute.u64(a.data_.u64, b.data_.u64);
                }
            }
        }
        else
        {
            // [Generic Semantic Rule]
            // If types differ, return undefined type. It should
            // be explicitely casted.
            c.type_ = Type::kUndefined;
            assert(false);
        }
    }

    void GenericOp::Subscript(const Generic& a, const uint32_t index, Generic& c)
    {
        // Checks.
        assert(a.type_ != Type::kUndefined);

        // Calculation.
        c.data_.u1 = static_cast<bool>(a.data_.u64 >> index) & 1;

        // [Generic Semantic Rule]
        // Subscript returns one bit.
        c.type_ = Type::kInteger;
        c.bit_width_ = 1;
    }

    void GenericOp::Slice(const uint32_t hi, const uint32_t lo, Generic& c)
    {
        // Checks.
        assert(hi < c.bit_width_);

        // Compute slicing information.
        uint32_t slice_width = hi - lo + 1;
        uint64_t slice_mask = (1ULL << slice_width) - 1;

        // Set the type and proper bit width.
        c.bit_width_ = 8;
        while (slice_width > c.bit_width_)
        {
            c.bit_width_ *= 2;
        }

        // Set the data.
        c.data_.u64 = (c.data_.u64 >> lo) & slice_mask;
    }

    void GenericOp::SignExtend(const uint32_t width, Generic& c)
    {
        // Kyuudou AST operations define only 64 bit sign extension.
        // Add support for other width if ever needed.
        assert(width == 64);
        switch (c.bit_width_)
        {
        case 8:
            c.data_.i64 = static_cast<int64_t>(c.data_.i8);
            break;
        case 16:
            c.data_.i64 = static_cast<int64_t>(c.data_.i16);
            break;
        case 32:
            c.data_.i64 = static_cast<int64_t>(c.data_.i32);
            break;
        case 64:
            break;
        default:
            assert(false);
            break;
        }
        c.bit_width_ = width;
        c.is_signed_ = true;
    }

    bool GenericOp::CompareEqual(const Generic& a, const Generic& b)
    {
        // Checks.
        assert(a.type_ != Type::kUndefined && b.type_ != Type::kUndefined);

        // Calculation.
        bool ret = false;

        // [Generic Semantic Rule]
        // Comparison of float and integers returns false.
        bool is_different_types = (a.type_ == Type::kInteger && b.type_ == Type::kFloat)
            || (a.type_ == Type::kFloat && b.type_ == Type::kInteger);
        if (!is_different_types)
        {
            ret = a.data_.u64 == b.data_.u64;
        }

        return ret;
    }
}
