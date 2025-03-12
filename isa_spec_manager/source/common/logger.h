/*
 * Copyright (c) 2021 Advanced Micro Devices, Inc. All rights reserved.
 */
#ifndef LOGGER_H_
#define LOGGER_H_

// C++ libraries.
#include <fstream>
#include <string>

namespace amdisa
{
    class Logger
    {
    public:
        void Log(const std::string& message, uint32_t indent = 0);
        bool Init(const std::string& path, const std::string& file_name);
        Logger() = default;
        ~Logger();

    private:
        std::ofstream output_file_;
    };
}  // namespace amdisa
#endif  // LOGGER_H_
