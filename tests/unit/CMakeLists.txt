include_guard(GLOBAL)

message(STATUS "Enabling unit tests")

add_subdirectory(common)
add_subdirectory(dataflowAPI)
add_subdirectory(dyninstAPI)
add_subdirectory(instructionAPI)
add_subdirectory(MachRegister)
add_subdirectory(parseAPI)
add_subdirectory(symtabAPI)
add_subdirectory(static_link)
