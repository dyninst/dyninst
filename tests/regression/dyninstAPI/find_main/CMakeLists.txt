include_guard(GLOBAL)

add_executable(find_main find_main.cpp)
target_compile_options(find_main PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(find_main PRIVATE dyninstAPI)

# Commented-out tests don't work because find_main
# doesn't work for stripped binaries on that arch.
set(_tests
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
#  "ppc64le/dyn.nopie"
#  "ppc64le/dyn.pie"
#  "ppc64le/stat.nopie"

  "x86_64/dyn.nopie"
  "x86_64/dyn.pie"
  "x86_64/stat.nopie"
  "x86_64/stat.pie"
)

if(DYNINST_ENABLE_CAPSTONE)
  # riscv64 doesn't support static PIE until glibc-2.39
  list(APPEND _tests
#  "riscv64/dyn.nopie"
#  "riscv64/stat.nopie"
  "riscv64/dyn.pie")
endif()

set(_test_dir "${DYNINST_TEST_BINARIES_DIR}/dyninstAPI/findMain")

foreach(_t IN ITEMS ${_tests})
  string(REPLACE "/" "_" _tmp ${_t})
  string(REPLACE "." "_" _tmp ${_tmp})
  
  set(_name "find_main_${_tmp}")
  set(_file "${_test_dir}/${_t}")

  add_test(NAME ${_name} COMMAND find_main "${_file}.nostripped" "${_file}.stripped")
  set_tests_properties(${_name} PROPERTIES LABELS "regression")
endforeach()
