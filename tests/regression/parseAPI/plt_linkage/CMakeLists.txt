include_guard(GLOBAL)

set(_targ plt_linkage)
add_executable(${_targ} plt_linkage.cpp)
target_compile_options(${_targ} PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(${_targ} PRIVATE parseAPI)

if(DYNINST_ENABLE_EXTERNAL_BINARY_TESTS)
  # ppc64le has no .linkage file: its call stubs carry no @plt label for the
  # expectation to be read from.
  set(_binaries
      x86_64/dyn
      x86_64/dyn.cet
      x86_64/dyn.pie
      i686/dyn
      i686/dyn.cet
      i686/dyn.pie
      i686/dyn.pie.cet
      aarch64/dyn
      aarch64/dyn.bti-pac
      riscv64/dyn
      riscv64/dyn.zicfilp)

  foreach(_b ${_binaries})
    string(REPLACE "/" "_" _name "${_b}")
    set(_dir "${DYNINST_TEST_BINARIES_DIR}/symtabAPI/plt_stubs")
    add_test(NAME parseAPI_plt_linkage_${_name}
             COMMAND ${_targ} "${_dir}/${_b}" "${_dir}/${_b}.linkage")
    set_tests_properties(parseAPI_plt_linkage_${_name} PROPERTIES LABELS "regression")
  endforeach()
endif()
