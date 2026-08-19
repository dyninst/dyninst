include_guard(GLOBAL)

# These are ELF/Linux tests: they depend on .eh_frame / .gcc_except_table
# regeneration and the DT_DYNINST_EH_FRAME + __register_frame delivery, which
# are ELF- and glibc/libgcc-specific. The unwind-info synthesis is implemented
# for x86-64, aarch64, and ppc64le; on any other architecture BinaryEdit does
# not synthesize it (ehFrameArchFor() returns null).
#
# These tests build native mutatees and then rewrite AND execute them, so the
# code generator must target the host architecture. A cross-architecture codegen
# build (the CI "codegen cross-build" matrix retargets the generator -- i386,
# aarch64, ppc64le, ... -- while the mutatees stay native to the x86-64 host)
# cannot rewrite the native mutatees at all: a 32-bit generator has no 64-bit
# register space, and a foreign generator aborts mid-rewrite. Require codegen
# arch == host arch, restricted to the arches whose EH-info synthesis exists.
if(NOT UNIX)
  return()
endif()
if(NOT
   ((DYNINST_CODEGEN_ARCH_X86_64 AND DYNINST_HOST_ARCH_X86_64)
    OR (DYNINST_CODEGEN_ARCH_AARCH64 AND DYNINST_HOST_ARCH_AARCH64)
    OR (DYNINST_CODEGEN_ARCH_PPC64LE AND DYNINST_HOST_ARCH_PPC64LE)))
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

# The dynamic (live-process) EH-delivery path is x86-64 only for now; the static
# rewriter path works on all three supported arches (incl. PIE). Skip the dynamic
# (eh_dyncatch) tests where dynamic delivery is unavailable rather than record
# spurious failures -- each for a DIFFERENT reason:
#   * aarch64: dynamic instrumentation itself is unimplemented (dyninst#449) --
#     BPatch::processCreate aborts in the stackwalker during bootstrap, before any
#     EH code runs.
#   * ppc64le: a pre-existing ppc64le DYNAMIC-RELOCATION bug, not EH. The frames
#     synthesize and register fine, but the relocated code SIGSEGVs BEFORE any
#     exception is thrown: a relocated external/PLT call runs with a wrong TOC
#     (r2), so a TOC-relative indirect call (bctrl) loads a garbage pointer and
#     jumps into ld.so. (Confirmed: the crash NIP is a fixed offset in ld64.so.2
#     across runs; _Unwind_Find_FDE never sees a relocated PC, so the synthesized
#     FDEs are never consulted. Neither #2336 ifunc-exclusion nor #2338 isel
#     changes it.) This is the dynamic analogue of the static TOC-restore fix
#     (the "preserve the TOC-restore after a non-returning call" commit) and is a
#     separate relocation-correctness fix from the EH work. The identical
#     synthesized bytes unwind correctly via static delivery (32/32 above).
set(_eh_dynamic_supported TRUE)
if(DYNINST_HOST_ARCH_AARCH64 OR DYNINST_HOST_ARCH_PPC64LE)
  set(_eh_dynamic_supported FALSE)
endif()

# --- mutatees: self-checking C++ programs that throw and catch ---
set(_eh_mutatees
    eh_single # single class-type try/catch
    eh_multi # multiple catch clauses; matching (non-first) one fires
    eh_nested # nested try/catch, type discrimination
    eh_rethrow # catch + re-throw caught by an outer frame
    eh_cleanup # destructor runs during unwind before the handler
    eh_basecatch # throw derived, catch by base reference
    eh_deep # deep (9-frame) non-tail-call unwind
    eh_align) # alignas(64) -> stack realign + hot/cold split (SPEC omnetpp shape)

# Runtime environment shared by both phases: the rewrite driver must know which
# RT to bind into the output (DYNINSTAPI_RT_LIB), and the rewritten mutatee must
# find that RT at load time (LD_LIBRARY_PATH). All paths come from the build
# tree via generator expressions.
set(_rt_lib "$<TARGET_FILE:dyninstAPI_RT>")
set(_ld_path "$<TARGET_FILE_DIR:dyninstAPI_RT>:$<TARGET_FILE_DIR:dyninstAPI>")
set(_eh_env "DYNINSTAPI_RT_LIB=${_rt_lib}"
            "LD_LIBRARY_PATH=${_ld_path}:$ENV{LD_LIBRARY_PATH}")

# Build each mutatee BOTH position-independent (PIE, ET_DYN -- the default on
# modern distros) and not (ET_EXEC), and run each variant through both the static
# rewriter and dynamic instrumentation. The EH delivery genuinely differs by
# type: a PIE loads at a runtime bias, so the dynamic path must rebase the
# DWARF/LSDA addresses (link -> runtime) and the static path must shift the
# DT_DYNINST_EH_FRAME tag by library_adjust (the PIE "PHDR in the first page"
# workaround). Testing only one type hides an entire class of bug -- the PIE gap
# was invisible for exactly this reason: every earlier mutatee was ET_EXEC. The
# -pie / -no-pie flags force the type explicitly so the matrix is identical on a
# PIE-default and a non-PIE-default toolchain (i.e. reproducible across machines).
#
# Pass/fail contract (all eh_catch_/eh_dyncatch_ tests): each mutatee prints
# "CAUGHT-<n>" only on full success (right handler + cleanups run); a wrong
# handler prints "WRONG-...", a skipped cleanup "NO-CLEANUP-<n>", an escaped
# exception "MISSED-<n>", and a bad unwind terminates before printing anything.
# So matching the success sentinel alone suffices -- any failure simply fails to
# print it. No FAIL_REGULAR_EXPRESSION (it takes precedence, so an incidental
# substring in e.what() could fail a passing run). The "-[0-9]" anchor is load-
# bearing: a bare "CAUGHT" is a substring of "MISSED-<n>"'s historical spelling.
# eh_register_mutatee(<target> <source> <opt> <nopie|pie>): build one mutatee
# variant (its binary under mutatees/) and register its test chain --
# eh_prep_<t> (clean the artifact dir) -> eh_rewrite_<t> -> eh_catch_<t> (static),
# and eh_dyncatch_<t> (dynamic, where supported). Reads _eh_env and
# _eh_dynamic_supported from the enclosing scope.
function(eh_register_mutatee t src opt variant)
  add_executable(${t} ${src})
  set_target_properties(
    ${t}
    PROPERTIES CXX_STANDARD 11
               CXX_STANDARD_REQUIRED ON
               RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/mutatees")
  target_compile_options(${t} PRIVATE ${opt})
  if(variant STREQUAL "pie")
    set_target_properties(${t} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_compile_options(${t} PRIVATE -fPIE)
    target_link_options(${t} PRIVATE -pie)
  else()
    set_target_properties(${t} PROPERTIES POSITION_INDEPENDENT_CODE OFF)
    target_compile_options(${t} PRIVATE -fno-pie)
    target_link_options(${t} PRIVATE -no-pie)
  endif()

  # Each test's rewritten binary (and any future per-test debug output) lives in
  # its own artifacts/<t>/ directory, kept after the run for inspection.
  set(_adir "${CMAKE_CURRENT_BINARY_DIR}/artifacts/${t}")
  set(_rw "${_adir}/${t}_rewritten")

  # Phase 0 (setup): clean+create the artifact dir so a failed/partial rewrite
  # can't leave a stale binary for the catch phase to false-pass on. Unlabeled --
  # fixture plumbing, kept out of the -L exceptions count but still run
  # automatically whenever a test requiring eh_fx_${t} runs.
  add_test(NAME eh_prep_${t} COMMAND ${CMAKE_COMMAND} -DDIR=${_adir} -P
                                     "${CMAKE_CURRENT_SOURCE_DIR}/eh_clean_dir.cmake")
  set_tests_properties(eh_prep_${t} PROPERTIES FIXTURES_SETUP eh_fx_${t})

  # Phase 1: rewrite the mutatee (relocate all functions -> regenerate EH info).
  add_test(NAME eh_rewrite_${t} COMMAND eh_rewrite_driver "$<TARGET_FILE:${t}>" "${_rw}")
  set_tests_properties(
    eh_rewrite_${t}
    PROPERTIES LABELS
               "integration;exceptions"
               FIXTURES_SETUP
               eh_fx_${t}
               DEPENDS
               eh_prep_${t}
               ENVIRONMENT
               "${_eh_env}")

  # Phase 2 (static): run the rewritten mutatee; it must still CATCH.
  add_test(NAME eh_catch_${t} COMMAND "${_rw}")
  set_tests_properties(
    eh_catch_${t}
    PROPERTIES LABELS
               "integration;exceptions"
               FIXTURES_REQUIRED
               eh_fx_${t}
               PASS_REGULAR_EXPRESSION
               "CAUGHT-[0-9]"
               ENVIRONMENT
               "${_eh_env}")

  # Dynamic path: create the mutatee live, instrument every basic block (forcing
  # relocation), and run it -- the issue reproducer's path. Skipped where dynamic
  # delivery is unavailable (aarch64 #449; ppc64le unported -- see above).
  if(_eh_dynamic_supported)
    add_test(NAME eh_dyncatch_${t} COMMAND eh_dynamic_driver "$<TARGET_FILE:${t}>")
    set_tests_properties(
      eh_dyncatch_${t}
      PROPERTIES LABELS
                 "integration;exceptions"
                 PASS_REGULAR_EXPRESSION
                 "CAUGHT-[0-9]"
                 TIMEOUT
                 180
                 ENVIRONMENT
                 "${_eh_env}")
  endif()
endfunction()

# The standard mutatees at -O2 (enough for their hot/cold + stack-realign shapes),
# each built PIE and non-PIE.
foreach(m ${_eh_mutatees})
  foreach(variant nopie pie)
    eh_register_mutatee("${m}_${variant}" "${m}.cpp" "-O2" "${variant}")
  endforeach()
endforeach()

# Hot/cold split exercised at BOTH -O2 and -O3: the block-partitioning heuristics
# -- and thus which fragment the throwing call-site vs. the landing pad end up in
# -- differ between the two levels, so each is worth its own coverage.
foreach(opt O2 O3)
  foreach(variant nopie pie)
    eh_register_mutatee("eh_hotcold_${opt}_${variant}" "eh_hotcold.cpp" "-${opt}"
                        "${variant}")
  endforeach()
endforeach()
