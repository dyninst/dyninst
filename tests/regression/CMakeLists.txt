include_guard(GLOBAL)

message(STATUS "Enabling regression tests")

add_subdirectory(symtabAPI)

find_package(Git)

if(NOT Git_FOUND)
  message(FATAL_ERROR "git is required for regression tests")
endif()

set(DYNINST_TEST_BINARIES_DIR "${CMAKE_CURRENT_BINARY_DIR}/test_binaries")

if(NOT IS_DIRECTORY ${DYNINST_TEST_BINARIES_DIR})
  execute_process(
    COMMAND ${GIT_EXECUTABLE} clone --depth=1
            https://github.com/dyninst/test_binaries.git ${DYNINST_TEST_BINARIES_DIR})
endif()
