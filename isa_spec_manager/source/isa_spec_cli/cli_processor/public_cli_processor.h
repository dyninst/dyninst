/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#ifndef PUBLIC_CLI_PROCESSOR_H_
#define PUBLIC_CLI_PROCESSOR_H_

// C++ libraries.
#include <memory>
#include <vector>
#include <string>

// Local libraries.
#include "cli_command.h"
#include "cli_processor.h"

// Forward declaration.
namespace cxxopts
{
    class Options;
    class ParseResult;
}  // namespace cxxopts

namespace amdisa
{
    // Forward declarations.
    class IsaDecoder;

    class PublicCliProcessor : public ICliProcessor
    {
    public:
        PublicCliProcessor() = default;
        ~PublicCliProcessor();
        void AppendOptions(cxxopts::Options& options) override;
        void Configure(const cxxopts::ParseResult& arguments, const std::string& cli_help_msg, IsaDecoder& spec_api) override;
        void Run() override;

    private:
        struct PublicCliProcessorImpl;
        PublicCliProcessorImpl* pimpl_ = nullptr;
    };
}  // namespace amdisa
#endif  // PUBLIC_CLI_PROCESSOR_H_
