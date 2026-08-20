include_guard(GLOBAL)

if(POLICY CMP0076)
  # target_sources() converts relative paths to absolute
  cmake_policy(SET CMP0076 NEW)
endif()

add_subdirectory(aarch64)
add_subdirectory(AMDGPU)
add_subdirectory(PowerPC)
add_subdirectory(riscv)
add_subdirectory(x86)

set(_subprojects instructionAPI_aarch64 instructionAPI_amdgpu instructionAPI_powerpc
                 instructionAPI_riscv instructionAPI_x86)

set(_headers type_conversion.h InstructionDecoderImpl.h)

set(_sources InstructionCategories.C InstructionDecoder.C InstructionDecoderImpl.C)

# Optional decoder used by the built-in "unknown instruction" callback to
# recover the length of x86-64 instructions InstructionAPI cannot decode.
if(DYNINST_ENABLE_ZYDIS)
  set(_unknown_insn_deps Dyninst::Zydis)
  set(_unknown_insn_defines DYNINST_ENABLE_ZYDIS)
endif()

# cmake-format: off
dyninst_library(
  instructionAPI_decoder
  INTERNAL_LIBRARY
  SOURCE_FILES ${_sources}
  PRIVATE_HEADER_FILES ${_headers}
  DEFINES ${_unknown_insn_defines}
  DYNINST_INTERNAL_DEPS common
  PRIVATE_DEPS ${_unknown_insn_deps}
  PUBLIC_DEPS ${_subprojects}
)

foreach(s ${_subprojects})
  get_target_property(_type ${s} TYPE)
  if(NOT _type STREQUAL "INTERFACE_LIBRARY")
      target_link_libraries(instructionAPI_decoder PUBLIC $<TARGET_OBJECTS:${s}>)
  endif()
endforeach()

set(_inc_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${PROJECT_SOURCE_DIR}/instructionAPI/h)
# cmake-format: on

target_include_directories(instructionAPI_decoder PRIVATE $<BUILD_INTERFACE:${_inc_dirs}>)

unset(_headers)
unset(_sources)
unset(_unknown_insn_deps)
unset(_unknown_insn_defines)
unset(_subprojects)
