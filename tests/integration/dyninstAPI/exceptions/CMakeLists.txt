include_guard(GLOBAL)

# These are ELF/Linux tests: they depend on .eh_frame / .gcc_except_table
# regeneration and the DT_DYNINST_EH_FRAME + __register_frame delivery, which
# are ELF- and glibc/libgcc-specific. The unwind-info synthesis is implemented
# for x86-64, aarch64, and ppc64le; on any other architecture BinaryEdit does
# not synthesize it (ehFrameArchFor() returns null), so the feature is
# unsupported there and the tests do not apply. Gate on the supported set to
# avoid spurious failures on e.g. RISC-V or AMDGPU.
if(NOT UNIX)
  return()
endif()
if(NOT (DYNINST_HOST_ARCH_X86_64 OR DYNINST_HOST_ARCH_AARCH64 OR DYNINST_HOST_ARCH_PPC64LE))
  return()
endif()
if(NOT (TARGET dyninstAPI AND TARGET dyninstAPI_RT))
  return()
endif()

message(STATUS "Enabling dyninstAPI exception-handling integration tests")

# --- rewriter driver: relocate every function, forcing EH-info synthesis ---
add_executable(eh_rewrite_driver eh_rewrite_driver.cpp)
target_link_libraries(eh_rewrite_driver PRIVATE dyninstAPI)

# --- dynamic driver: create the mutatee as a live process and instrument every
#     basic block, forcing relocation in-process (the original issue reproducer's
#     path) rather than via the static rewriter ---
add_executable(eh_dynamic_driver eh_dynamic_driver.cpp)
target_link_libraries(eh_dynamic_driver PRIVATE dyninstAPI)

# --- mutatees: self-checking C++ programs that throw and catch ---
set(_eh_mutatees
    eh_single    # single class-type try/catch
    eh_multi     # multiple catch clauses; matching (non-first) one fires
    eh_nested    # nested try/catch, type discrimination
    eh_rethrow   # catch + re-throw caught by an outer frame
    eh_cleanup   # destructor runs during unwind before the handler
    eh_basecatch # throw derived, catch by base reference
    eh_deep      # deep (9-frame) non-tail-call unwind
    eh_align)    # alignas(64) -> stack realign + hot/cold split (SPEC omnetpp shape)

# Runtime environment shared by both phases: the rewrite driver must know which
# RT to bind into the output (DYNINSTAPI_RT_LIB), and the rewritten mutatee must
# find that RT at load time (LD_LIBRARY_PATH). All paths come from the build
# tree via generator expressions.
set(_rt_lib "$<TARGET_FILE:dyninstAPI_RT>")
set(_ld_path "$<TARGET_FILE_DIR:dyninstAPI_RT>:$<TARGET_FILE_DIR:dyninstAPI>")
set(_eh_env
    "DYNINSTAPI_RT_LIB=${_rt_lib}"
    "LD_LIBRARY_PATH=${_ld_path}:$ENV{LD_LIBRARY_PATH}")

foreach(m ${_eh_mutatees})
  add_executable(${m} ${m}.cpp)
  set_target_properties(${m} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)
  # -O2 so the compiler hot/cold-splits and stack-realigns (eh_align relies on it).
  target_compile_options(${m} PRIVATE -O2)

  set(_rw "${CMAKE_CURRENT_BINARY_DIR}/${m}_rewritten")

  # Phase 1: rewrite the mutatee (relocate all functions -> regenerate EH info).
  add_test(NAME eh_rewrite_${m}
           COMMAND eh_rewrite_driver "$<TARGET_FILE:${m}>" "${_rw}")
  set_tests_properties(eh_rewrite_${m} PROPERTIES
      LABELS "integration;exceptions"
      FIXTURES_SETUP eh_fx_${m}
      ENVIRONMENT "${_eh_env}")

  # Phase 2: run the rewritten mutatee; it must still CATCH the RIGHT exception
  # and run its cleanups, which each mutatee reports by printing "CAUGHT-<n>"
  # only on full success (a wrong handler prints "WRONG-...", a skipped cleanup
  # "NO-CLEANUP-<n>", an escaped exception "MISSED-<n>", and a bad unwind
  # terminates before printing anything). Matching the success sentinel alone is
  # therefore sufficient: any failure -- wrong catch, crash, terminate, timeout
  # -- simply fails to print it. No FAIL_REGULAR_EXPRESSION is needed, and using
  # one would be a hazard (it takes precedence, so an incidental substring in
  # e.what() could fail a genuinely-passing run). The "-[0-9]" anchor is load-
  # bearing: a bare "CAUGHT" is a substring of the "MISSED-<n>" sentinel's
  # historical spelling, so keep success sentinels disjoint from failure ones.
  add_test(NAME eh_catch_${m} COMMAND "${_rw}")
  set_tests_properties(eh_catch_${m} PROPERTIES
      LABELS "integration;exceptions"
      FIXTURES_REQUIRED eh_fx_${m}
      PASS_REGULAR_EXPRESSION "CAUGHT-[0-9]"
      ENVIRONMENT "${_eh_env}")

  # Dynamic-instrumentation path: create the mutatee live, instrument every basic
  # block (forcing relocation), and run it. Same pass/fail contract as eh_catch;
  # the driver create+instrument+runs in one shot (no fixture). Like any Dyninst
  # mutator it needs the RT at load time (DYNINSTAPI_RT_LIB).
  add_test(NAME eh_dyncatch_${m}
           COMMAND eh_dynamic_driver "$<TARGET_FILE:${m}>")
  set_tests_properties(eh_dyncatch_${m} PROPERTIES
      LABELS "integration;exceptions"
      PASS_REGULAR_EXPRESSION "CAUGHT-[0-9]"
      TIMEOUT 180
      ENVIRONMENT "${_eh_env}")
endforeach()
