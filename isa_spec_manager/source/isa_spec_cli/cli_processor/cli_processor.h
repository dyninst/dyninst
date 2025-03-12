/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#ifndef CLI_PROCESSOR_H_
#define CLI_PROCESSOR_H_

// Forward declarations.
namespace cxxopts
{
    class Options;
    class ParseResult;
}  // namespace cxxopts

namespace amdisa
{
    // Forward declarations.
    class IsaDecoder;

    // Interface class for CLI programs.
    struct ICliProcessor
    {
        ICliProcessor() = default;
        virtual ~ICliProcessor(){};

        // Adds options specific to the CLI implementation.
        virtual void AppendOptions(cxxopts::Options& options) = 0;

        // Creates relevant commands based on the options.
        virtual void Configure(const cxxopts::ParseResult& arguments, const std::string& cli_help_msg, IsaDecoder& spec_api) = 0;

        // Run.
        virtual void Run() = 0;
    };
}  // namespace amdisa
#endif  // CLI_PROCESSOR_H_
