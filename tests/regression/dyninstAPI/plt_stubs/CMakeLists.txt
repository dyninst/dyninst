include_guard(GLOBAL)

set(_targ plt_stubs_image)
add_executable(${_targ} plt_stubs_image.cpp)
target_compile_options(${_targ} PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(${_targ} PRIVATE dyninstAPI)

if(DYNINST_ENABLE_EXTERNAL_BINARY_TESTS)
  # Unlike the symtabAPI and parseAPI tests over these binaries, this one goes
  # through BPatch, which can only open an object for the architecture it is
  # running on.  Every binary is still registered, so that a runner reports the
  # ones it cannot open as skipped rather than appearing not to have them: a
  # test that is absent looks the same as a test nobody wrote.
  # Which architecture this build generates code for, which is what decides
  # what it can open -- not the machine it was built on.  A build configured
  # with DYNINST_CODEGEN_ARCH for another architecture runs that one's tests
  # instead, and an ordinary build runs its own.
  #
  # x86-64 covers both x86 architectures: it decodes and parses 32-bit objects
  # as well, and all four i686 binaries pass against such a build.  It happens
  # to manage the aarch64 ones too, since these are small enough that parsing
  # never reaches the dataflow that would need an aarch64 build, but that is
  # luck rather than support and is not relied on here.
  set(_this_arch "")
  if(DYNINST_CODEGEN_ARCH_X86_64)
    set(_this_arch "x86_64;i686")
  elseif(DYNINST_CODEGEN_ARCH_I386)
    set(_this_arch "i686")
  elseif(DYNINST_CODEGEN_ARCH_AARCH64)
    set(_this_arch "aarch64")
  elseif(DYNINST_CODEGEN_ARCH_RISCV64)
    set(_this_arch "riscv64")
  endif()

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
    get_filename_component(_arch "${_b}" DIRECTORY)
    set(_dir "${DYNINST_TEST_BINARIES_DIR}/symtabAPI/plt_stubs")
    add_test(NAME dyninstAPI_plt_stubs_${_name}
             COMMAND ${_targ} "${_dir}/${_b}" "${_dir}/${_b}.linkage")
    set_tests_properties(dyninstAPI_plt_stubs_${_name} PROPERTIES LABELS "regression")
    # Nothing here instruments, so the runtime library is never loaded, but
    # BPatch's constructor reports it missing and a SERIOUS line in a CI log
    # reads like a broken test.  Ask the target rather than deriving a path.
    set_tests_properties(
      dyninstAPI_plt_stubs_${_name} PROPERTIES ENVIRONMENT
      "DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>")
    if(NOT "${_arch}" IN_LIST _this_arch)
      set_tests_properties(dyninstAPI_plt_stubs_${_name} PROPERTIES DISABLED TRUE)
    endif()
  endforeach()
endif()
