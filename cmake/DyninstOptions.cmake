option(USE_OpenMP "Use OpenMP for parallel parsing" ON)

option(
    LIGHTWEIGHT_SYMTAB
    "Use lightweight symtab interface for ParseAPI, ProcControl, and Stackwalker; disables DyninstAPI build"
    OFF)

option(SW_ANALYSIS_STEPPER "Use ParseAPI-based analysis stepper in Stackwalker" ON)

option(BUILD_RTLIB_32 "Build 32-bit runtime library on mixed 32/64 systems" OFF)

option(ENABLE_LTO "Enable Link-Time Optimization" OFF)

option(ENABLE_DEBUGINFOD "Enable debuginfod support" OFF)

set(STERILE_BUILD
    ON
    CACHE BOOL "Do not download/build any third-party dependencies from source")

set(ADD_VALGRIND_ANNOTATIONS
    OFF
    CACHE BOOL "Enable annotations for Valgrind analysis")

set(ENABLE_STATIC_LIBS
    NO
    CACHE STRING "Build static libraries as well?")

option(DYNINST_DISABLE_DIAGNOSTIC_SUPPRESSIONS
       "Disable all warning suppressions and frame size overrides." OFF)

set(DYNINST_EXTRA_WARNINGS
    ""
    CACHE
        STRING
        "Additional warning options to enable if available.  ;-separated without leading '-' (Wopt1[;Wopt2]...)."
    )

option(DYNINST_WARNINGS_AS_ERRORS "Treat compilation warnings as errors" OFF)

option(ENABLE_PARSE_API_GRAPHS "Enable Boost Graph wrappers for parseAPI Functions" OFF)

# Some global on/off switches
if(LIGHTWEIGHT_SYMTAB)
    add_definitions(-DWITHOUT_SYMTAB_API -DWITH_SYMLITE)
else()
    add_definitions(-DWITH_SYMTAB_API -DWITHOUT_SYMLITE)
endif()

if(SW_ANALYSIS_STEPPER)
    add_definitions(-DUSE_PARSE_API)
endif()
