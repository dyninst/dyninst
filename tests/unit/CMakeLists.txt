include_guard(GLOBAL)

message(STATUS "Enabling unit tests")

add_subdirectory(common)
add_subdirectory(dataflowAPI)
add_subdirectory(dyninstAPI/emitter)
add_subdirectory(instructionAPI)
add_subdirectory(instructionAPI/AST)
add_subdirectory(MachRegister)
add_subdirectory(parseAPI)
