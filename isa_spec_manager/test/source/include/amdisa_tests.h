/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMDISA_TESTS_H_
#define AMDISA_TESTS_H_

#include "amdisa/isa_decoder.h"

class TestConfig
{
public:
    TestConfig(const TestConfig&)            = delete;
    TestConfig& operator=(const TestConfig&) = delete;

    static TestConfig& getInstance()
    {
        static TestConfig instance;
        return instance;
    }

    const std::string& GetXmlPath()
    {
        return xml_path_;
    };

    void SetXmlPath(std::string path){
        xml_path_ = path;
    };

private:
    TestConfig() = default;
    std::string xml_path_;
};

#endif  // AMDISA_TESTS_H_
