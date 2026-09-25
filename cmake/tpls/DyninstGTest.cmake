#================================================================
#
# Configure GoogleTest for DYNINST_BUILD_TESTING
#
#   ----------------------------------------
#
# Provides GTest::gtest / GTest::gtest_main for use by
# `dyninst_add_unit_test` and any test CMakeLists.txt.
#
# Discovery order:
#   1. Reuse a GTest::gtest target already configured by a parent
#      project (e.g. dyninst embedded as a submodule).
#   2. find_package(GTest) for a system/prefix-path installation.
#   3. Fall back to FetchContent, pinned to the same release
#      rocprofiler-systems vendors.
#
#================================================================

include_guard(GLOBAL)

if(NOT DYNINST_BUILD_TESTING)
  return()
endif()

# If GTest::gtest already exists (e.g. configured by a parent project that
# embeds dyninst as a submodule), reuse it instead of searching/fetching.
if(TARGET GTest::gtest)
  message(STATUS "Using pre-configured GTest::gtest target")
  return()
endif()

find_package(GTest CONFIG QUIET)

if(NOT TARGET GTest::gtest)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0)
  # Match the parent project's compiler/runtime settings instead of gtest's own
  # defaults.
  set(gtest_force_shared_crt
      ON
      CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
endif()
