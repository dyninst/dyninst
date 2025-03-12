/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#include "amdisa_utility.h"

// C++ libraries.
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

// Local libraries.
#include "amdisa_structures.h"

namespace amdisa
{
    // Warning string constants.
    static const char* kStringWarningCouldNotConvertStrToUnsigned = "Warning: could not convert string to integer: ";

    bool AmdIsaUtility::GetRange(const Field& field, Range& range)
    {
        bool is_range_retrieved = false;
        if (!field.ranges.empty())
        {
            range              = field.ranges[0];
            is_range_retrieved = true;
        }

        return is_range_retrieved;
    }

    void AmdIsaUtility::BitMapToString(const MicrocodeFormat& microcode_format, const std::vector<uint32_t>& machine_code, std::string& string_bitmap)
    {
        std::stringstream ret;
        for (const auto& field : microcode_format.bit_map)
        {
            Range range;
            if (GetRange(field, range))
            {
                uint32_t bit_offset  = range.bit_offset;
                uint32_t bit_count   = range.bit_count;
                uint32_t upper       = bit_offset + bit_count - 1;
                uint32_t lower       = bit_offset;
                uint32_t dword_index = bit_offset / 32;

                ret << "    " << std::left << std::setw(13) << field.name << "[" << std::right << std::dec << std::setw(2) << upper << ":" << std::setw(2)
                    << lower << "]";

                uint32_t dword = 0;
                if (dword_index < machine_code.size())
                {
                    dword = machine_code[dword_index];
                }

                uint64_t field_val = (dword >> (bit_offset - 32 * dword_index)) & ((1 << bit_count) - 1);
                ret << " = " << std::hex << field_val;
                ret << std::endl;
            }
        }

        string_bitmap = ret.str();
    }

    std::string AmdIsaUtility::ToUpper(const std::string& str)
    {
        std::string ustr = str;
        std::transform(ustr.begin(), ustr.end(), ustr.begin(), [](const char& c) { return std::toupper(c); });
        return ustr;
    }

    std::string AmdIsaUtility::ToLower(const std::string& str)
    {
        std::string lstr = str;
        std::transform(lstr.begin(), lstr.end(), lstr.begin(), [](const char& c) { return std::tolower(c); });
        return lstr;
    }

    std::string AmdIsaUtility::Strip(const std::string& str)
    {
        std::string stripped_str;
        int32_t     pos_start = 0;
        size_t      pos_end   = str.length() - 1;

        // Skip leading whitespaces and special chars.
        auto should_skip = [](const char& c) { return c == ' ' || c == '\n' || c == '\r' || c == '\t'; };
        while (pos_start < str.length() && should_skip(str[pos_start]))
        {
            ++pos_start;
        }

        // Skip trailing whitespaces and special chars.
        while (pos_end - pos_start >= 0 && should_skip(str[pos_end]))
        {
            --pos_end;
        }

        // Get the middle.
        for (size_t i = pos_start; i <= pos_end && pos_end != std::string::npos; i++)
        {
            stripped_str += str[i];
        }

        return stripped_str;
    }

    uint64_t AmdIsaUtility::StringToUnsignedInt(const std::string& str)
    {
        uint64_t result = 0;
        try
        {
            result = std::stoull(str);
        }
        catch (std::exception& err)
        {
            std::cout << kStringWarningCouldNotConvertStrToUnsigned << str << ". Exception: " << err.what() << std::endl;
        }
        return result;
    }
}  // namespace amdisa
