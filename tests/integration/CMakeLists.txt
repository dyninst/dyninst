include_guard(GLOBAL)

message(STATUS "Enabling integration tests")

add_subdirectory(InstructionAPI/decoder/aarch64)
add_subdirectory(InstructionAPI/decoder/ppc64le)
add_subdirectory(InstructionAPI/decoder/x86)

if(DYNINST_ENABLE_CAPSTONE)
  add_subdirectory(InstructionAPI/decoder/riscv)
endif()

# C++ exception handling through relocated code (issue #1437)
add_subdirectory(dyninstAPI/exceptions)
