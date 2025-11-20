include_guard(GLOBAL)

message(STATUS "Enabling integration tests")

add_subdirectory(InstructionAPI/decoder/x86)
add_subdirectory(InstructionAPI/decoder/riscv)
