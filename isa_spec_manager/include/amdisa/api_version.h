/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#ifndef API_VERSION_H_
#define API_VERSION_H_

// C++ libraries.

#include <string>
#include <sstream>

#define AMDISA_DECODE_API_VERSION_MAJOR 1
#define AMDISA_DECODE_API_VERSION_MINOR 0
#define AMDISA_DECODE_API_VERSION_PATCH 0

namespace amdisa
{
    // Structure to return API version.
    class ApiVersion
    {
    public:
        static int GetMajor()
        {
            return AMDISA_DECODE_API_VERSION_MAJOR;
        }

        static int GetMinor()
        {
            return AMDISA_DECODE_API_VERSION_MINOR;
        }

        static int GetPatch()
        {
            return AMDISA_DECODE_API_VERSION_PATCH;
        }

        static std::string GetVersion()
        {
            std::stringstream api_version;
            api_version << GetMajor() << "." << GetMinor() << "." << GetPatch();
            return api_version.str();
        }

        static bool IsAtLeast(int major, int minor, int patch)
        {
            bool is_older = false;
            is_older =
                (major < GetMajor() || (major == GetMajor() && minor < GetMinor()) || (major == GetMajor() && minor == GetMinor() && patch < GetPatch()));
            return !is_older;
        }
    };

}  // namespace amdisa

#endif  // API_VERSION_H_
