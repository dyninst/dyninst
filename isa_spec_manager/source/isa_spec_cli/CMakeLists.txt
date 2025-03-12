# Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
cmake_minimum_required(VERSION 3.0)

# Setting project directories.
set (SRC_DIR ../../source)
set (THIRD_PARTY_DIR ../third_party)
set (INC_DIR ../../include)

# Redirect to single build output location
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

# Local target: IsaSpecCli.
# Setting all header files for this project.
set (ALL_H
    # CLI commands.
    ${SRC_DIR}/isa_spec_cli/cli_processor/cli_command.h
    ${SRC_DIR}/isa_spec_cli/cli_processor/cli_command_decode_machine_code.h
    ${SRC_DIR}/isa_spec_cli/cli_processor/cli_command_decode_shader_file.h
    ${SRC_DIR}/isa_spec_cli/cli_processor/cli_command_generate_inst_desc.h
    ${SRC_DIR}/isa_spec_cli/cli_processor/cli_command_get_inst_info.h
    ${SRC_DIR}/isa_spec_cli/cli_processor/cli_command_print_help.h
    ${SRC_DIR}/isa_spec_cli/cli_processor/cli_command_read_xml.h
    ${SRC_DIR}/isa_spec_cli/cli_processor/cli_processor.h
    ${SRC_DIR}/isa_spec_cli/cli_processor/public_cli_processor.h

    # ISA Spec decoder.
    ${INC_DIR}/amdisa/api_version.h
    ${INC_DIR}/amdisa/isa_decoder.h

    # Third party library headers.
    ${THIRD_PARTY_DIR}/cxxopts/cxxopts.hpp
    ${THIRD_PARTY_DIR}/tinyxml2/tinyxml2.h
)

# Setting all source files for this project.
set (ALL_CPP
    # Local library headers.
    ${SRC_DIR}/isa_spec_cli/cli_processor/cli_command_decode_machine_code.cpp
    ${SRC_DIR}/isa_spec_cli/cli_processor/cli_command_decode_shader_file.cpp
    ${SRC_DIR}/isa_spec_cli/cli_processor/cli_command_get_inst_info.cpp
    ${SRC_DIR}/isa_spec_cli/cli_processor/cli_command_generate_inst_desc.cpp
    ${SRC_DIR}/isa_spec_cli/cli_processor/cli_command_print_help.cpp
    ${SRC_DIR}/isa_spec_cli/cli_processor/cli_command_read_xml.cpp
    ${SRC_DIR}/isa_spec_cli/cli_processor/public_cli_processor.cpp
)

# Adding target.
add_executable(isa_spec_cli main.cpp ${ALL_H} ${ALL_CPP})
target_include_directories(isa_spec_cli PRIVATE
    ${SRC_DIR}/common
    ${SRC_DIR}/isa_spec_cli/cli_processor
    ${THIRD_PARTY_DIR}/cxxopts
    ${THIRD_PARTY_DIR}/tinyxml2
    PUBLIC
    ${INC_DIR}/amdisa
)
target_link_libraries(isa_spec_cli PRIVATE
    isa_decoder
)

install(
TARGETS isa_spec_cli
RUNTIME COMPONENT binary
)
