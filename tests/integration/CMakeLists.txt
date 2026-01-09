include_guard(GLOBAL)

message(STATUS "Enabling integration tests")

add_subdirectory(InstructionAPI/decoder/x86)

if(DYNINST_ENABLE_CAPSTONE)
  add_subdirectory(InstructionAPI/decoder/riscv)
endif()
