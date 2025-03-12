/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#include "amdisa_tests.h"
#include "amdisa/isa_decoder.h"
#include "catch.hpp"

TEST_CASE("Test to fail decode of a shader disassembly text", "[decode][text][fail]")
{
    amdisa::IsaDecoder decoder;
    std::string        msg;
    TestConfig&        config = TestConfig::getInstance();
    REQUIRE(decoder.Initialize(config.GetXmlPath(), msg));

    std::vector<amdisa::InstructionInfoBundle> info;
    REQUIRE(decoder.DecodeShaderDisassemblyText("", info, msg, false) == false);
}

TEST_CASE("Test to decode of a shader disassembly text", "[decode][text]")
{
    amdisa::IsaDecoder decoder;
    std::string        msg;
    TestConfig&        config = TestConfig::getInstance();
    REQUIRE(decoder.Initialize(config.GetXmlPath(), msg));

    std::vector<amdisa::InstructionInfoBundle> info;
    REQUIRE(decoder.DecodeShaderDisassemblyText("s_cmp_eq_u32  s9, 0 // 000000000000: BF068009", info, msg, false));
    REQUIRE(info[0].bundle[0].instruction_name == "S_CMP_EQ_U32");
}

TEST_CASE("Test to fail decode of a shader disassembly file", "[decode][file][fail]")
{
    amdisa::IsaDecoder decoder;
    std::string        msg;
    TestConfig&        config = TestConfig::getInstance();
    REQUIRE(decoder.Initialize(config.GetXmlPath(), msg));

    std::vector<amdisa::InstructionInfoBundle> info;
    REQUIRE(decoder.DecodeShaderDisassemblyFile("./invalid_disassembly_file.sp3", info, msg, false) == false);
}
