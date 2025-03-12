/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#include "amdisa_tests.h"
#include "amdisa/isa_decoder.h"
#include "catch.hpp"

TEST_CASE("Perform Initialization with an invalid path", "[initialize][fail]")
{
    amdisa::IsaDecoder decoder;
    std::string        msg;
    REQUIRE(decoder.Initialize("./invalid_path.xml", msg) == false);
}

TEST_CASE("Perform Initialization with an user specified amdisa spec file", "[initialize]")
{
    amdisa::IsaDecoder decoder;
    std::string        msg;
    TestConfig& config = TestConfig::getInstance();
    REQUIRE(decoder.Initialize(config.GetXmlPath(), msg));
}
