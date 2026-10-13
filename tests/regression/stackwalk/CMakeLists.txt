include_guard(GLOBAL)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  return()
endif()

add_executable(undefined_ra_target undefined_ra_target.c)
target_link_libraries(undefined_ra_target PRIVATE Threads::Threads)

add_executable(undefined_ra_check undefined_ra_check.cpp)
target_compile_options(undefined_ra_check PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(undefined_ra_check PRIVATE stackwalk)

# Skips unless the host is AArch64 with glibc >= 2.34.
add_test(
  NAME stackwalk_undefined_ra
  COMMAND undefined_ra_check $<TARGET_FILE:undefined_ra_target>
)
set_tests_properties(
  stackwalk_undefined_ra
  PROPERTIES
    LABELS regression
    TIMEOUT 60
    SKIP_RETURN_CODE 77
    SKIP_REGULAR_EXPRESSION "skipping:"
)

add_subdirectory(vdso)
