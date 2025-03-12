/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
// Local libraries.
#include "isa_decoder.h"
#include "public_cli_processor.h"

// Third party libraries.
#include "cxxopts.hpp"

// Constants.
// Error messages.
static const char* kStringErrorParsingCommandLineArgs = "Error: Failed to parse command line arguments.";
// Info messages.
static const char* kStringInfoCliDescription = "Command line interface program for ISA Spec API.";

int main(int argc, char* argv[])
{
    amdisa::IsaDecoder         spec_api;
    amdisa::PublicCliProcessor public_cli_proc;
    cxxopts::Options           options(argv[0], kStringInfoCliDescription);

    // Configure CLI.
    bool should_abort = false;
    try
    {
        public_cli_proc.AppendOptions(options);
        public_cli_proc.Configure(options.parse(argc, argv), options.help(), spec_api);
    }
    catch (const cxxopts::OptionException& e)
    {
        std::cerr << kStringErrorParsingCommandLineArgs << std::endl;
        std::cerr << " Exception: " << e.what() << std::endl;
        std::cerr << std::endl;
        std::cerr << options.help() << std::endl;
        should_abort = true;
    }

    // Run CLI.
    if (!should_abort)
    {
        public_cli_proc.Run();
    }
    return 0;
}
