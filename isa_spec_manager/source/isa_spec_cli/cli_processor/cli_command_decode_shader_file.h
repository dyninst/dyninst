/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#ifndef CLI_COMMAND_DECODE_SHADER_FILE_H_
#define CLI_COMMAND_DECODE_SHADER_FILE_H_

// Local libraries.
#include "cli_command.h"

namespace amdisa
{
    class IsaDecoder;

    /**
     * Class: CliCommandDecodeShaderFile
     *
     * Purpose:
     * This class is responsible for handling the CLI (Command Line Interface) command
     * that decodes shader files into a machine or human readable standardized format.
     * Given a shader file, the class provides the capability to decode its content
     * and save the result in various formats. Additionally, the class allows users
     * to specify the format of the decoding information and whether branch target
     * information should be included.
     */
    class CliCommandDecodeShaderFile : public ICliCommand
    {
    public:
        CliCommandDecodeShaderFile(const std::string& shader_file_path,
                                   const std::string& output_xml_path,
                                   const std::string& info_format,
                                   const IsaDecoder&  spec_api,
                                   bool               is_branch_target_info_set)
            : info_format_(info_format)
            , shader_file_path_(shader_file_path)
            , output_xml_path_(output_xml_path)
            , spec_api_(spec_api)
            , is_branch_target_info_set_(is_branch_target_info_set){};
        CliCommandDecodeShaderFile() = delete;

        bool Execute(std::string& err_message) override;

    private:
        const std::string info_format_;
        const std::string shader_file_path_;
        const std::string output_xml_path_;
        const IsaDecoder& spec_api_;
        const bool        is_branch_target_info_set_;
    };
}  // namespace amdisa
#endif  // CLI_COMMAND_DECODE_SHADER_FILE_H_
