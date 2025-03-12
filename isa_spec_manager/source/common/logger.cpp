/*
 * Copyright (c) 2021 Advanced Micro Devices, Inc. All rights reserved.
 */
#include "logger.h"

namespace amdisa
{
    Logger::~Logger()
    {
        if (output_file_.is_open())
        {
            output_file_.close();
        }
    }

    bool Logger::Init(const std::string& path, const std::string& file_name)
    {
        if (!output_file_.is_open())
        {
            std::string full_path;
            if (path.empty())
            {
                full_path = file_name;
            }
            else
            {
                full_path = path + "/" + file_name;
            }

            output_file_.open(full_path);
        }

        return output_file_.is_open();
    }

    void Logger::Log(const std::string& message, uint32_t indent)
    {
        std::string indentation;
        while (indent > 0)
        {
            indentation += "    ";
            --indent;
        }
        output_file_ << indentation << message << std::endl;
    }
}  // namespace amdisa