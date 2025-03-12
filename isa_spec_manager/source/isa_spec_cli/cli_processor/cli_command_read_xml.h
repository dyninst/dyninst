/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#ifndef CLI_COMMAND_READ_XML_H_
#define CLI_COMMAND_READ_XML_H_

// C++ libraries.
#include <string>

// Local libraries.
#include "cli_command.h"

namespace amdisa
{
    // Forward declarations.
    class IsaDecoder;

    /**
     * Class: CliCommandReadXml
     *
     * Purpose:
     * This class is responsible for handling the CLI (Command Line Interface) command
     * that reads specification details from an XML file and populates internal data
     * structures with the extracted information. This operation facilitates the further
     * processing or utilization of the spec details within the application.
     */
    class CliCommandReadXml : public ICliCommand
    {
    public:
        CliCommandReadXml(const std::string& input_xml_path, IsaDecoder& spec_api)
            : input_xml_path_(input_xml_path)
            , spec_api_(spec_api)
        {
        }
        CliCommandReadXml() = delete;
        bool Execute(std::string& err_message) override;

    private:
        std::string input_xml_path_;
        IsaDecoder& spec_api_;
    };
}  // namespace amdisa
#endif  // CLI_COMMAND_READ_XML_H_
