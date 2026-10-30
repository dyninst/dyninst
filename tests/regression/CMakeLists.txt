include_guard(GLOBAL)

message(STATUS "Enabling regression tests")

add_subdirectory(symtabAPI)
add_subdirectory(parseAPI)
add_subdirectory(dataflowAPI)
add_subdirectory(stackwalk)
add_subdirectory(dyninstAPI)
