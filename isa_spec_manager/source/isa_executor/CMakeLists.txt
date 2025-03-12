cmake_minimum_required(VERSION 3.0)

set(CMAKE_BUILD_TYPE Debug)

set (SRC_DIR ../../source)
set (INC_DIR ../../include)
set (THIRD_PARTY_DIR ../../source/third_party)

# Redirect to single build output location
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

set (ALL_H
    ${SRC_DIR}/isa_executor/isa_executor.h
    ${SRC_DIR}/isa_executor/generic.h
    ${SRC_DIR}/isa_executor/generic_op.h
    ${SRC_DIR}/isa_executor/generated_cpp/inst_execution.hpp
    ${SRC_DIR}/isa_executor/wave_state.h
    ${SRC_DIR}/isa_executor/state_delta.h
    ${SRC_DIR}/common/amdisa_structures.h
    ${SRC_DIR}/common/amdisa_utility.h
    ${SRC_DIR}/common/isa_xml_reader.h
    ${INC_DIR}/amdisa/isa_decoder.h
    ${THIRD_PARTY_DIR}/tinyxml2/tinyxml2.h
)

set (ALL_CPP
    ${SRC_DIR}/isa_executor/isa_executor.cpp
    ${SRC_DIR}/isa_executor/generic.cpp
    ${SRC_DIR}/isa_executor/generic_op.cpp
    ${SRC_DIR}/isa_executor/wave_state.cpp
    ${SRC_DIR}/common/amdisa_utility.cpp
    ${SRC_DIR}/common/isa_xml_reader.cpp
    ${SRC_DIR}/isa_decoder/isa_decoder.cpp
    ${THIRD_PARTY_DIR}/tinyxml2/tinyxml2.cpp
)
# Add your executable
add_executable(isa_eval ./main_test.cpp ${ALL_CPP} ${ALL_H})

include_directories(${INC_DIR})
include_directories(${SRC_DIR}/isa_executor)
include_directories(${SRC_DIR}/isa_executor/generated_cpp)
include_directories(${SRC_DIR}/common)
include_directories(${THIRD_PARTY_DIR}/tinyxml2)
