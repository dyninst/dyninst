/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#ifndef AMDISA_UTILITY_H_
#define AMDISA_UTILITY_H_

// C++ libraries.
#include <string>
#include <vector>
#include <cstdint>

namespace amdisa
{
    // Forward declarations.
    struct MicrocodeFormat;
    struct Range;
    struct Field;

    class AmdIsaUtility
    {
    public:
        static void        BitMapToString(const MicrocodeFormat& microcode_format, const std::vector<uint32_t>& machine_code, std::string& string_bitmap);
        static std::string ToUpper(const std::string& str);
        static std::string ToLower(const std::string& str);
        static std::string Strip(const std::string& str);
        static uint64_t    StringToUnsignedInt(const std::string& str_num);
        static bool        GetRange(const Field& field, Range& range);
    };
}  // namespace amdisa
#endif  // AMDISA_UTILITY_H_
