option(USE_OpenMP "Use OpenMP for parallel parsing" ON)

option(
  LIGHTWEIGHT_SYMTAB
  "Use lightweight symtab interface for ParseAPI, ProcControl, and Stackwalker; disables DyninstAPI build"
  OFF)

option(SW_ANALYSIS_STEPPER "Use ParseAPI-based analysis stepper in Stackwalker" ON)

option(BUILD_RTLIB_32 "Build 32-bit runtime library on mixed 32/64 systems" OFF)

option(DYNINST_ENABLE_LTO "Enable Link-Time Optimization" OFF)

option(ENABLE_DEBUGINFOD "Enable debuginfod support" OFF)

option(STERILE_BUILD "DEPRECATED -- Do not use" OFF)

option(ADD_VALGRIND_ANNOTATIONS "Enable annotations for Valgrind analysis" OFF)

option(ENABLE_STATIC_LIBS "Build static libraries as well?" OFF)

option(DYNINST_DISABLE_DIAGNOSTIC_SUPPRESSIONS
       "Disable all warning suppressions and frame size overrides." OFF)

option(
  DYNINST_EXTRA_WARNINGS
  "Additional warning options to enable if available.  ;-separated without leading '-' (Wopt1[;Wopt2]...)."
  "")

option(DYNINST_WARNINGS_AS_ERRORS "Treat compilation warnings as errors" OFF)

option(ENABLE_PARSE_API_GRAPHS "Enable Boost Graph wrappers for parseAPI Functions" OFF)

option(DYNINST_LINKER "The linker to use" "lld")
mark_as_advanced(DYNINST_LINKER)

option(DYNINST_CXXSTDLIB
       "The C++ standard library to use; only affects LLVM-based compilers" "libstdc++")
mark_as_advanced(DYNINST_CXXSTDLIB)
