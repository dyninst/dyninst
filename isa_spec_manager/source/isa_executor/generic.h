// Copyright (c) 2021-2024 Advanced Micro Devices, Inc. All rights reserved.
#ifndef GENERIC_H_
#define GENERIC_H_

// C++ libraries.
#include <cassert>
#include <stdint.h>
#include <vector>

namespace amdisa
{
    enum class Type
    {
        kUndefined,
        kInteger,
        kFloat
    };

    union Data {
        uint8_t u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64 = 0;
        int8_t i8;
        int16_t i16;
        int32_t i32;
        int64_t i64;
        float f32;
        double f64;
        bool u1;
    };

    /*
    * Kyuudou AST Types and operations
    * The shim layer class has all the information about the
    * types in Kyuudou AST. Ideally, this class should be auto
    * generated from the XML ISA Spec.
    */
    class Generic
    {
    public:
        Generic() = default;
        explicit Generic(const uint8_t& data);
        explicit Generic(const uint16_t& data);
        explicit Generic(const uint32_t& data);
        explicit Generic(const uint64_t& data);
        explicit Generic(const int8_t& data);
        explicit Generic(const int16_t& data);
        explicit Generic(const int32_t& data);
        explicit Generic(const int64_t& data);
        explicit Generic(const float& data);
        explicit Generic(const double& data);

        // Setters and getters.
        void SetData(const uint64_t& data);
        void SetData(const float& data);
        void SetData(const double& data);
        void SetData(const std::vector<uint32_t>& data);
        void SetType(const Type& type, bool is_signed = false);
        Type GetType() const;
        uint32_t GetBitWidth() const;
        void GetData(std::vector<uint32_t>& data) const;

        // Operator overloads.
        Generic& operator=(const Generic& rhs) = default;
        Generic operator[](const uint32_t& index);
        Generic operator+(const Generic& rhs);
        Generic operator+(const uint64_t& rhs);
        void operator+=(const Generic& rhs);
        Generic operator-(const Generic& rhs);
        void operator-=(const Generic& rhs);
        Generic operator*(const Generic& rhs);
        void operator*=(const Generic& rhs);
        bool operator==(const Generic& rhs);
        bool operator!=(const Generic& rhs);

        // Conversion definitions.
        operator uint64_t() const;

        // Other operators.
        Generic slice(const uint32_t hi, const uint32_t lo);
        Generic signext();

        // Casting.
        Generic u8() const;
        Generic u16() const;
        Generic u32() const;
        Generic u64() const;
        Generic i8() const;
        Generic i16() const;
        Generic i32() const;
        Generic i64() const;
        Generic f32() const;
        Generic f64() const;
        Generic b32() const;

        // GenericOp class performs operatrions on the Generic.
        friend class GenericOp;

    private:
        Data data_;
        Type type_          = Type::kUndefined;
        uint32_t bit_width_ = 0;
        bool is_signed_     = false;
    };

    // AUX functions for the execution model: begin
    static Generic I32(const uint64_t& data)
    {
        Generic ret;
        ret.SetData(data);
        ret.SetType(Type::kInteger, true);
        return ret.i32();
    }

    static bool U1(const uint64_t& data)
    {
        return (data & 0b1) == 1;
    }

    static Generic U32(const uint64_t& data)
    {
        Generic ret;
        ret.SetData(data);
        ret.SetType(Type::kInteger);
        return ret.u32();
    }

    static Generic I16(const uint64_t& data)
    {
        Generic ret;
        ret.SetData(data);
        ret.SetType(Type::kInteger, true);
        return ret.i16();
    }

    static Generic I64(const uint64_t& data)
    {
        Generic ret;
        ret.SetData(data);
        ret.SetType(Type::kInteger, true);
        return ret.i64();
    }

    static Generic U64(const uint64_t& data)
    {
        Generic ret;
        ret.SetData(data);
        ret.SetType(Type::kInteger);
        return ret.u64();
    }

    static Generic i8_to_i32(const Generic& data)
    {
        assert(data.GetBitWidth() == 8);
        assert(data.GetType() == Type::kInteger);
        Generic temp = data;
        return temp.i32();
    }
    // AUX functions for the execution model: end
}
#endif // GENERIC_H_
