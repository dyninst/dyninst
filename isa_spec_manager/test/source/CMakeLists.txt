# Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
cmake_minimum_required(VERSION 3.0)

# Setting project directories.
set (SRC_DIR ../source)
set (THIRD_PARTY_DIR ../third_party)

# Redirect to single build output location
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

# Local target: IsaSpecCli.
# Setting all header files for this project.
set (ALL_H
    ${SRC_DIR}/include/amdisa_tests.h
    # Third party library headers.
    ${THIRD_PARTY_DIR}/catch2/catch.hpp
)

# Setting all test files for this project.
set (ALL_TESTS_CPP
   # Test files
   test_initialize.cpp
   test_decode_inst.cpp
   test_decode_text.cpp
)

# Adding target.
add_executable(amdisa_test main.cpp ${ALL_H} ${ALL_TESTS_CPP})
target_include_directories(amdisa_test PRIVATE
    ${SRC_DIR}/include
    ${THIRD_PARTY_DIR}/catch2
)
target_link_libraries(amdisa_test PRIVATE
    isa_decoder
)

install(
TARGETS amdisa_test
RUNTIME COMPONENT binary
)
