include_guard(GLOBAL)

set(_targ plt_stubs)
add_executable(${_targ} plt_stubs.cpp)
target_compile_options(${_targ} PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(${_targ} PRIVATE symtabAPI)

if(DYNINST_ENABLE_EXTERNAL_BINARY_TESTS)
  # None of these is built for the host: SymtabAPI reads any architecture, and
  # the bugs they cover were all invisible to a runner of a single one.
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
      ppc64le/dyn
      riscv64/dyn
      riscv64/dyn.zicfilp)

  foreach(_b ${_binaries})
    string(REPLACE "/" "_" _name "${_b}")
    set(_dir "${DYNINST_TEST_BINARIES_DIR}/symtabAPI/plt_stubs")
    add_test(NAME symtabAPI_plt_stubs_${_name}
             COMMAND ${_targ} "${_dir}/${_b}" "${_dir}/${_b}.expected")
    set_tests_properties(symtabAPI_plt_stubs_${_name} PROPERTIES LABELS "regression")
  endforeach()
endif()
