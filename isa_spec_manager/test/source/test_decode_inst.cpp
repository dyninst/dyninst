/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#include "amdisa_tests.h"
#include "amdisa/isa_decoder.h"
#include "catch.hpp"

TEST_CASE("Test to decode of a single instruction name", "[decode][single]")
{
    amdisa::IsaDecoder decoder;
    std::string        msg;
    TestConfig&        config = TestConfig::getInstance();
    REQUIRE(decoder.Initialize(config.GetXmlPath(), msg));

    amdisa::InstructionInfo info;
    REQUIRE(decoder.DecodeInstruction("S_MOV_B32", info, msg));
    REQUIRE(info.instruction_name == "S_MOV_B32");
}

TEST_CASE("Test to decode of a single instruction binary", "[decode][single]")
{
    amdisa::IsaDecoder decoder;
    std::string        msg;
    TestConfig&        config = TestConfig::getInstance();
    REQUIRE(decoder.Initialize(config.GetXmlPath(), msg));

    amdisa::InstructionInfoBundle info;
    REQUIRE(decoder.DecodeInstruction(0x80000002, info, msg));
    REQUIRE(info.bundle[0].instruction_name == "S_ADD_U32");
}

TEST_CASE("Test to decode of an instruction stream", "[decode][single]")
{
    amdisa::IsaDecoder decoder;
    std::string        msg;
    TestConfig&        config = TestConfig::getInstance();
    REQUIRE(decoder.Initialize(config.GetXmlPath(), msg));

    std::vector<uint32_t>                      inst_stream = {0x801FFF1F, 0x00000080};
    std::vector<amdisa::InstructionInfoBundle> info;
    REQUIRE(decoder.DecodeInstructionStream(inst_stream, info, msg));
    REQUIRE(info[0].bundle[0].instruction_name == "S_ADD_U32");
}
