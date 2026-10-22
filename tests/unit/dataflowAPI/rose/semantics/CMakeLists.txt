project(tests_dataflowAPI_rose_semantics LANGUAGES CXX)

# One test per source file: rose_semantics_<file name>
file(GLOB _sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
foreach(_source ${_sources})
  get_filename_component(_name "${_source}" NAME_WE)
  set(_test rose_semantics_${_name})
  add_executable(${_test} ${_source})
  target_compile_options(${_test} PRIVATE ${UNIT_TESTS_WARNING_FLAGS})
  target_link_libraries(${_test} PRIVATE parseAPI)
  add_test(NAME ${_test} COMMAND ${_test})
  set_tests_properties(${_test} PROPERTIES LABELS "unit")
endforeach()
