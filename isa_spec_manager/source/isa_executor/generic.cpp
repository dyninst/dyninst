// Copyright (c) 2021-2023 Advanced Micro Devices, Inc. All rights reserved.
#include "generic.h"

// C++ libraries.
#include <cassert>
#include <stdint.h>

// Local libraries.
#include "generic_op.h"

namespace amdisa
{
    Generic::Generic(const uint8_t& data)
    {
        data_.u8 = data;
        bit_width_ = 8;
        is_signed_ = false;
        type_ = Type::kInteger;
    }

    Generic::Generic(const uint16_t& data)
    {
        data_.u16 = data;
        bit_width_ = 16;
        is_signed_ = false;
        type_ = Type::kInteger;
    }

    Generic::Generic(const uint32_t& data)
    {
        data_.u32 = data;
        bit_width_ = 32;
        is_signed_ = false;
        type_ = Type::kInteger;
    }

    Generic::Generic(const uint64_t& data)
    {
        data_.u64 = data;
        bit_width_ = 64;
        is_signed_ = false;
        type_ = Type::kInteger;
    }

    Generic::Generic(const int8_t& data)
    {
        data_.i8 = data;
        bit_width_ = 8;
        is_signed_ = true;
        type_ = Type::kInteger;
    }

    Generic::Generic(const int16_t& data)
    {
        data_.i16 = data;
        bit_width_ = 16;
        is_signed_ = true;
        type_ = Type::kInteger;
    }

    Generic::Generic(const int32_t& data)
    {
        data_.i32 = data;
        bit_width_ = 32;
        is_signed_ = true;
        type_ = Type::kInteger;
    }

    Generic::Generic(const int64_t& data)
    {
        data_.i64 = data;
        bit_width_ = 64;
        is_signed_ = true;
        type_ = Type::kInteger;
    }

    Generic::Generic(const float& data)
    {
        data_.f32 = data;
        bit_width_ = 32;
        is_signed_ = false;
        type_ = Type::kFloat;
    }

    Generic::Generic(const double& data)
    {
        data_.f64 = data;
        bit_width_ = 64;
        is_signed_ = false;
        type_ = Type::kFloat;
    }

    void Generic::SetData(const uint64_t& data)
    {
        data_.u64 = data;
    }

    void Generic::SetData(const float& data)
    {
        data_.f32 = data;
    }

    void Generic::SetData(const double& data)
    {
        data_.f64 = data;
    }

    void Generic::SetData(const std::vector<uint32_t>& data)
    {
        // TODO: Add support for data width > 64.
        assert(data.size() < 3);

        // Merge 32-bit chunks into single variable.
        data_.u64 = 0;
        uint32_t shift = 0;
        for (const auto& chunk : data)
        {
            data_.u64 = (data_.u64 << (shift * 32)) | chunk;
            ++shift;
        }
        bit_width_ = shift * 32;
    }

    void Generic::SetType(const Type& type, bool is_signed)
    {
        type_ = type;
        is_signed_ = is_signed;
    }

    Type Generic::GetType() const
    {
        return type_;
    }

    uint32_t Generic::GetBitWidth() const
    {
        return bit_width_;
    }

    void Generic::GetData(std::vector<uint32_t>& data) const
    {
        data.clear();
        if (bit_width_ > 32)
        {
            data.push_back((data_.u64 >> 32) & 0xffffffffUL);
            data.push_back(data_.u32);
        }
        else
        {
            data.push_back(data_.u32);
        }
    }

    Generic Generic::operator[](const uint32_t& index)
    {
        // Checks.
        Generic ret;
        GenericOp::Subscript(*this, index, ret);
        return ret;
    }

    Generic Generic::operator+(const Generic& rhs)
    {
        // Define binary operations.
        Generic ret;
        GenericOp::BinOp(*this, rhs, Operation::kAdd, ret);
        return ret;
    }

    Generic Generic::operator+(const uint64_t& rhs)
    {
        // Define binary operations.
        Generic ret;
        GenericOp::BinOp(*this, U64(rhs), Operation::kAdd, ret);
        return ret;
    }

    void Generic::operator+=(const Generic& rhs)
    {
        Generic temp;
        GenericOp::BinOp(*this, rhs, Operation::kAdd, temp);
        *this = temp;
    }

    Generic Generic::operator-(const Generic& rhs)
    {
        Generic ret;
        GenericOp::BinOp(*this, rhs, Operation::kSubtract, ret);
        return ret;
    }

    void Generic::operator-=(const Generic& rhs)
    {
        Generic temp;
        GenericOp::BinOp(*this, rhs, Operation::kSubtract, temp);
        *this = temp;
    }

    Generic Generic::operator*(const Generic& rhs)
    {
        Generic ret;
        GenericOp::BinOp(*this, rhs, Operation::kMultiply, ret);
        return ret;
    }

    void Generic::operator*=(const Generic& rhs)
    {
        Generic temp;
        GenericOp::BinOp(*this, rhs, Operation::kMultiply, temp);
        *this = temp;
    }

    bool Generic::operator==(const Generic& rhs)
    {
        return GenericOp::CompareEqual(*this, rhs);
    }

    bool Generic::operator!=(const Generic& rhs)
    {
        return !(GenericOp::CompareEqual(*this, rhs));
    }

    Generic::operator uint64_t() const
    {
        return data_.u64;
    }

    Generic Generic::slice(const uint32_t hi, const uint32_t lo)
    {
        Generic temp;
        temp = *this;
        GenericOp::Slice(hi, lo, temp);
        return temp;
    }

    Generic Generic::signext()
    {
        Generic temp;
        temp = *this;
        // Sign extend to 64 bits per rules set in Kyuudou.
        GenericOp::SignExtend(64, temp);
        return temp;
    }

    Generic Generic::u8() const
    {
        assert(type_ != Type::kFloat);
        Generic ret;
        ret = Generic(static_cast<uint8_t>(data_.u64));
        ret.bit_width_ = 8;
        ret.type_ = Type::kInteger;
        ret.is_signed_ = false;
        return ret;
    }

    Generic Generic::u16() const
    {
        assert(type_ != Type::kFloat);
        Generic ret;
        ret = Generic(static_cast<uint16_t>(data_.u64));
        ret.bit_width_ = 16;
        ret.type_ = Type::kInteger;
        ret.is_signed_ = false;
        return ret;
    }

    Generic Generic::u32() const
    {
        assert(type_ != Type::kFloat);
        Generic ret;
        ret = Generic(static_cast<uint32_t>(data_.u64));
        ret.bit_width_ = 32;
        ret.type_ = Type::kInteger;
        ret.is_signed_ = false;
        return ret;
    }

    Generic Generic::u64() const
    {
        assert(type_ != Type::kFloat);
        Generic ret;
        ret = Generic(static_cast<uint64_t>(data_.u64));
        ret.bit_width_ = 64;
        ret.type_ = Type::kInteger;
        ret.is_signed_ = false;
        return ret;
    }

    Generic Generic::i8() const
    {
        assert(type_ != Type::kFloat);
        assert(is_signed_ || type_ == Type::kUndefined);
        Generic ret;
        ret.data_.i8 = static_cast<int8_t>(data_.u64);
        ret.bit_width_ = 8;
        ret.type_ = Type::kInteger;
        ret.is_signed_ = true;
        return ret;
    }

    Generic Generic::i16() const
    {
        assert(type_ != Type::kFloat);
        assert(is_signed_ || type_ == Type::kUndefined);
        Generic ret;
        int16_t value = static_cast<int16_t>(data_.u64);
        if (bit_width_ == 8)
        {
            value = static_cast<int16_t>(data_.i8);
        }
        ret.data_.i16 = value;
        ret.bit_width_ = 16;
        ret.type_ = Type::kInteger;
        ret.is_signed_ = true;
        return ret;
    }

    Generic Generic::i32() const
    {
        assert(type_ != Type::kFloat);
        assert(is_signed_ || type_ == Type::kUndefined);
        Generic ret;
        int32_t value = static_cast<int32_t>(data_.u64);
        if (bit_width_ == 8)
        {
            value = static_cast<int32_t>(data_.i8);
        }
        else if (bit_width_ == 16)
        {
            value = static_cast<int32_t>(data_.i16);
        }
        ret.data_.i32 = value;
        ret.bit_width_ = 32;
        ret.type_ = Type::kInteger;
        ret.is_signed_ = true;
        return ret;
    }

    Generic Generic::i64() const
    {
        assert(type_ != Type::kFloat);
        assert(is_signed_ || type_ == Type::kUndefined);
        Generic ret;
        int64_t value = static_cast<int64_t>(data_.u64);
        if (bit_width_ == 8)
        {
            value = static_cast<int64_t>(data_.i8);
        }
        else if (bit_width_ == 16)
        {
            value = static_cast<int64_t>(data_.i16);
        }
        else if (bit_width_ == 32)
        {
            value = static_cast<int64_t>(data_.i32);
        }
        ret.data_.i64 = value;
        ret.bit_width_ = 64;
        ret.type_ = Type::kInteger;
        ret.is_signed_ = true;
        return ret;
    }

    Generic Generic::f32() const
    {
        assert(type_ != Type::kInteger);
        Generic ret;
        float value = data_.f32;
        if (bit_width_ == 64)
        {
            value = static_cast<float>(data_.f64);
        }
        ret.data_.f32 = value;
        ret.bit_width_ = 32;
        ret.type_ = Type::kFloat;
        ret.is_signed_ = false;
        return ret;
    }

    Generic Generic::f64() const
    {
        assert(type_ != Type::kInteger);
        Generic ret;
        double value = data_.f64;
        if (bit_width_ == 32)
        {
            value = static_cast<double>(data_.f32);
        }
        ret.data_.f64 = value;
        ret.bit_width_ = 64;
        ret.type_ = Type::kFloat;
        ret.is_signed_ = false;
        return ret;
    }

    Generic Generic::b32() const
    {
        assert(type_ != Type::kFloat);
        Generic ret;
        ret = Generic(static_cast<uint32_t>(data_.u64));
        ret.bit_width_ = 32;
        ret.type_ = Type::kInteger;
        ret.is_signed_ = false;
        return ret;
    }
}
