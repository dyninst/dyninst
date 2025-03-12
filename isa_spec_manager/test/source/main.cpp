/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */

#include "amdisa_tests.h"

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

int main(int argc, char* argv[])
{
    Catch::Session session;

    std::string xml_path;
    std::string disassembly_file;

    using namespace Catch::clara;
    auto cli = session.cli() | Opt(xml_path, "spec")["-I"]["--spec"]("Path to the AMDISA XML spec") |
               Opt(disassembly_file, "disassembly_file")["-F"]["--disassembly"]("Path to the binary disassembly file");

    session.cli(cli);

    int returnCode = session.applyCommandLine(argc, argv);
    if (returnCode != 0)
        return returnCode;

    TestConfig& config = TestConfig::getInstance();
    if (!xml_path.empty())
    {
        config.SetXmlPath(xml_path);
        std::cout << "Testing: " << xml_path << std::endl;
    }

    return session.run();
}
