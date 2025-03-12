# Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
cmake_minimum_required(VERSION 3.0)

# Setting project directories.
set (SRC_DIR ../../source)
set (INC_DIR ../../include)

# Redirect to single build output location
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

# Adding target for BasicDecoder example
add_executable(basic_decoder ./basic_decoder.cpp)

target_include_directories(basic_decoder PRIVATE
    ${INC_DIR}
)
target_link_libraries(basic_decoder PRIVATE
    isa_decoder
)

# Adding target for Multi-Architecture support example.
add_executable(multiarch_decoder ./multi_arch_decoder.cpp)

target_include_directories(multiarch_decoder PRIVATE
    ${INC_DIR}
)
target_link_libraries(multiarch_decoder PRIVATE
    isa_decoder
)


# Adding target for isa_explorer_tutorial
add_executable(isa_explorer_tutorial ./isa_explorer_tutorial.cpp)

target_include_directories(isa_explorer_tutorial PRIVATE
    ${INC_DIR}
)
target_link_libraries(isa_explorer_tutorial PRIVATE
    isa_explorer
)

# Group these projects under an Examples folder.
set_target_properties(multiarch_decoder basic_decoder isa_explorer_tutorial PROPERTIES FOLDER "examples")
