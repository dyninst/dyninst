# Copyright (c) 2021-2024 Advanced Micro Devices, Inc. All rights reserved.
cmake_minimum_required(VERSION 3.0)

# List of projects.
project(isa_spec_manager)

# Setting compiler flags.
set (CMAKE_CXX_COMPILER g++)
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${FLAGS})

# Enable using folders to group projects on Windows (Visual Studio solution).
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Windows solution generator is a multi config generator, so define
# CMAKE_CONFIGURATION_TYPES, which enables usage of $<$<CONFIG:value1>value2> syntax for Windows settings.
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  # Visual Studio
  set(CMAKE_CONFIGURATION_TYPES "Release;Debug")
endif()

# Setting project directories.
# XML ISA Spec Decoder API: accepts ISA Spec for specific architecture
# and provides interface for decoding of instructions or data retrieval
# for specific instructions in the architecture.
add_subdirectory(./source/isa_decoder)

# ISA Executor API.
add_subdirectory(./source/isa_executor)

# Examples: demonstrates utilities and API features using example apps
add_subdirectory(./source/examples)

# CLI commands: provides command line interface implementation
# for generation of various CLI programs.
add_subdirectory(./source/isa_spec_cli)

# ISA explorer API
add_subdirectory(./source/isa_explorer)

# Unit testing suite
add_subdirectory(./test/source)
