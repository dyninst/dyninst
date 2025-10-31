include_guard(GLOBAL)

find_package(Git)

if(NOT Git_FOUND)
  message(FATAL_ERROR "git not found")
endif()

set(DYNINST_TEST_BINARIES_DIR "${CMAKE_BINARY_DIR}/tests/test_binaries")

if(NOT IS_DIRECTORY ${DYNINST_TEST_BINARIES_DIR})
  message(STATUS "Installing 'test_binaries' into ${DYNINST_TEST_BINARIES_DIR}")
  execute_process(
    COMMAND ${GIT_EXECUTABLE} clone --depth=1
            https://github.com/dyninst/test_binaries.git ${DYNINST_TEST_BINARIES_DIR})
endif()
