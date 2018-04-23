add_subdirectory(unstrip)

add_subdirectory(codeCoverage)

add_executable(cfg_to_dot ../parseAPI/doc/example.cc)
add_dependencies(cfg_to_dot parseAPI symtabAPI)
target_link_libraries(cfg_to_dot parseAPI symtabAPI)
   

