include_guard(GLOBAL)

# Commented-out tests don't work because find_main
# doesn't work for stripped binaries on that arch.
set(
  _tests
  #  "aarch64/dyn.nopie"
  #  "aarch64/dyn.pie"
  #  "aarch64/stat.nopie"
  #  "aarch64/stat.pie"
  #
  #  "i686/dyn.nopie"
  #  "i686/dyn.pie"
  #  "i686/stat.nopie"
  #  "i686/stat.pie"
  #
  #  # ppc64le doesn't support static PIE
  "ppc64le/dyn.nopie"
  "ppc64le/dyn.pie"
  "ppc64le/stat.nopie"
  # riscv64 doesn't support static PIE until glibc-2.39
  #  "riscv64/dyn.nopie"
  #  "riscv64/stat.nopie"
  "riscv64/dyn.pie"
  "x86_64/dyn.nopie"
  "x86_64/dyn.pie"
  "x86_64/stat.nopie"
  "x86_64/stat.pie"
)

if(NOT LIGHTWEIGHT_SYMTAB)
  add_executable(find_main find_main.cpp)
  target_compile_options(find_main PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
  target_link_libraries(find_main PRIVATE dyninstAPI)
endif()

set(_test_dir "${DYNINST_TEST_BINARIES_DIR}/dyninstAPI/findMain")

foreach(_t IN ITEMS ${_tests})
  string(REPLACE "/" "_" _tmp ${_t})
  string(REPLACE "." "_" _tmp ${_tmp})

  set(_name "find_main_${_tmp}")
  set(_file "${_test_dir}/${_t}")

  if(LIGHTWEIGHT_SYMTAB)
    # dyninstAPI is not built with LIGHTWEIGHT_SYMTAB, so the test cannot be built.
    set(
      _command
      sh
      -c
      "echo 'skipping: dyninstAPI is not built with LIGHTWEIGHT_SYMTAB' && exit 77"
    )
  elseif(NOT DYNINST_ENABLE_EXTERNAL_BINARY_TESTS)
    set(_command find_main --skip "needs the test binaries (DYNINST_ENABLE_TESTS=ALL)")
  elseif(_t MATCHES "^riscv64/" AND NOT DYNINST_ENABLE_CAPSTONE)
    set(_command find_main --skip "riscv64 code needs Capstone")
  else()
    set(_command find_main "${_file}.nostripped" "${_file}.stripped")
  endif()

  add_test(NAME ${_name} COMMAND ${_command})
  set_tests_properties(
    ${_name}
    PROPERTIES LABELS "regression" SKIP_RETURN_CODE 77 SKIP_REGULAR_EXPRESSION "skipping:"
  )
endforeach()
