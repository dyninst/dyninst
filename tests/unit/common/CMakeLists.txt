include_guard(GLOBAL)

# One test per case; the program runs the case named by its argument.
# The library's platform macros, so os_linux selects the cases its Linux code needs
dyninst_add_test_executable(
  common_filesystem
  SOURCE_FILES filesystem.cpp
  DEPS common
  DEFINES ${DYNINST_PLATFORM_CAPABILITIES}
)
foreach(
  _case
  canonicalize
  canonicalize_user
  resolve_in_root_fs
  canonicalize_procfs
  executable_path
  exists
  replace_extension
  append_filename_suffix
  strip_all_extensions
)
  dyninst_add_test(
    common_filesystem_${_case}
    TYPE unit
    COMMAND common_filesystem ${_case}
  )
endforeach()
foreach(
  _case
  canonicalize_procfs_namespace
  executable_path_namespace
  executable_path_chroot
)
  dyninst_add_test(
    common_filesystem_${_case}
    TYPE unit
    COMMAND common_filesystem ${_case}
    REQUIRES userns
  )
endforeach()

dyninst_add_test(common_bitmath_test TYPE unit SOURCE_FILES bitmath.cpp DEPS common)

dyninst_add_test(
  common_arch_aarch64_test
  TYPE unit
  SOURCE_FILES arch_aarch64.cpp
  DEPS common
)

add_executable(arch_power_test arch_power.cpp)
target_compile_options(arch_power_test PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(arch_power_test PRIVATE common)
add_test(NAME common_arch_power_test COMMAND arch_power_test)
set_tests_properties(common_arch_power_test PROPERTIES LABELS "unit")

add_subdirectory(registers)
