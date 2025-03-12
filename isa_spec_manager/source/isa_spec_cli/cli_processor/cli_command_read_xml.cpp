/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#include "cli_command_read_xml.h"

// C++ libraries.
#include <iostream>
#include <string>
#include <sstream>

// Local libraries.
#include "amdisa/isa_decoder.h"

namespace amdisa
{
    // Constants.
    // Information constant strings.
    static const char* kStringInfoXmlReadStart      = "Info: Parsing XML file...";
    static const char* kStringInfoXmlReadSuccessful = "Info: XML parsing completed successfully.";

    // Error constant strings.
    static const char* kStringErrorXmlReadFailed = "Error: Failed to parse XML file.";

    bool CliCommandReadXml::Execute(std::string& err_message)
    {
        bool is_executed = false;
        std::cout << kStringInfoXmlReadStart << std::endl;
        std::string init_err_message;
        is_executed = spec_api_.Initialize(input_xml_path_, init_err_message);

        if (is_executed)
        {
            std::cout << kStringInfoXmlReadSuccessful << std::endl;
        }
        else
        {
            std::stringstream final_error;
            final_error << kStringErrorXmlReadFailed << std::endl;
            final_error << init_err_message << std::endl;
            err_message = final_error.str();
        }

        return is_executed;
    }
}  // namespace amdisa
