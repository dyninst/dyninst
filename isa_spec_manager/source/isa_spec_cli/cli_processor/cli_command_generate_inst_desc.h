/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#ifndef CLI_COMMAND_GENERATE_INST_DESC_H_
#define CLI_COMMAND_GENERATE_INST_DESC_H_

// C++ libraries.
#include <string>

// Local libraries.
#include "cli_command.h"

namespace amdisa
{
    // Forward declarations.
    class IsaDecoder;

    /**
     * Class: CliCommandGenerateInstDesc
     *
     * Purpose:
     * This class is responsible for handling the CLI (Command Line Interface) command
     * that produces tables mapping instruction names to their corresponding descriptions.
     * Given the spec, the class can generate tables in various formats that provide a
     * description of each instruction.
     */
    class CliCommandGenerateInstDesc : public ICliCommand
    {
    public:
        CliCommandGenerateInstDesc() = delete;
        CliCommandGenerateInstDesc(const std::string& output_desc_path, const std::string& input_xml_path, const std::string& desc_format)
            : output_desc_path_(output_desc_path)
            , input_xml_path_(input_xml_path)
            , desc_format_(desc_format)
        {
        }
        bool Execute(std::string& err_message) override;

    private:
        const std::string output_desc_path_;
        const std::string input_xml_path_;
        const std::string desc_format_;
    };
}  // namespace amdisa
#endif  // CLI_COMMAND_GENERATE_INST_DESC_H_
