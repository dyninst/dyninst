include_guard(GLOBAL)

set(
  _tests
  # find_main doesn't work for stripped aarch64 binaries
  #  "aarch64/dyn.nopie"
  #  "aarch64/dyn.pie"
  #  "aarch64/stat.nopie"
  #  "aarch64/stat.pie"
  # find_main doesn't work for stripped 32-bit x86 binaries
  #  "i686/dyn.nopie"
  #  "i686/dyn.pie"
  #  "i686/stat.nopie"
  #  "i686/stat.pie"
  # glibc for ppc64le doesn't support static PIE
  "ppc64le/dyn.nopie"
  "ppc64le/dyn.pie"
  "ppc64le/stat.nopie"
  "x86_64/dyn.nopie"
  "x86_64/dyn.pie"
  "x86_64/stat.nopie"
  "x86_64/stat.pie"
)

if(DYNINST_ENABLE_CAPSTONE)
  # riscv64 doesn't support static PIE until glibc-2.39
  list(
    APPEND _tests
    # find_main doesn't work for stripped riscv64 binaries
    #  "riscv64/dyn.nopie"
    #  "riscv64/stat.nopie"
    # 'main' is in .dynsym so it's present in stripped PIE binaries
    "riscv64/dyn.pie"
  )
endif()

set(_test_dir "${DYNINST_TEST_BINARIES_DIR}/dyninstAPI/findMain")

dyninst_add_test_executable(
  find_main
  SOURCE_FILES find_main.cpp
  DEPS dyninstAPI
  REQUIRES test_binaries
)

# This is clunky, but will go away with #2535
dyninst_test_requirement(no_i386_codegen WHEN "FALSE" REASON "i386 codegen ABI bug")

foreach(_t IN ITEMS ${_tests})
  # Convert "arch/link.pic" into "arch_link_pic"
  string(REPLACE "/" "_" _tmp ${_t})
  string(REPLACE "." "_" _tmp ${_tmp})

  set(_name "find_main_${_tmp}")
  set(_file "${_test_dir}/${_t}")

  # Parsing an x86_64 binary when i386 codegen is enabled causes
  # a crash in the ABI class. Should be fixed by #2535.
  if(DYNINST_CODEGEN_ARCH STREQUAL "i386" AND "${_t}" MATCHES "x86_64")
    set(_extra_requires no_i386_codegen)
  endif()

  dyninst_add_test(
    "${_name}"
    TYPE regression
    COMMAND "$<TARGET_FILE:find_main>" "${_file}.nostripped" "${_file}.stripped"
    REQUIRES test_binaries "${_extra_requires}"
  )
endforeach()
