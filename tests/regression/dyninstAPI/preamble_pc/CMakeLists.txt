include_guard(GLOBAL)

# dyninstAPI is not built with LIGHTWEIGHT_SYMTAB
if(LIGHTWEIGHT_SYMTAB)
  return()
endif()

# The skipped TOC preamble exists only on ppc64 (ELFv2 global entry points).
if(DYNINST_HOST_ARCH_PPC64LE AND DYNINST_CODEGEN_ARCH_PPC64LE)
  add_executable(preamble_pc_mutatee preamble_pc_mutatee.c)

  set(_targ preamble_pc)
  add_executable(${_targ} preamble_pc.cpp)
  target_compile_options(${_targ} PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
  target_link_libraries(${_targ} PRIVATE dyninstAPI)

  set(_tname dyninstAPI_${_targ}_test)
  add_test(
    NAME ${_tname}
    COMMAND ${_targ} $<TARGET_FILE:preamble_pc_mutatee> $<TARGET_FILE_DIR:dyninstAPI_RT>
  )
  set_tests_properties(
    ${_tname}
    PROPERTIES
      LABELS "regression"
      TIMEOUT 120
      ENVIRONMENT "DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>"
  )
endif()
