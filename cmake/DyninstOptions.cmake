include_guard(DIRECTORY)

include(DyninstUtilities)

dyninst_add_option(BUILD_SHARED_LIBS "Build shared libraries" ON)
dyninst_add_option(BUILD_STATIC_LIBS "Build static libraries" OFF)

# Use OpenMP?
dyninst_add_option(USE_OpenMP "Use OpenMP for parallel parsing" ON)

# Use SymtabAPI or SymLite?
dyninst_add_option(
    LIGHTWEIGHT_SYMTAB
    "Use lightweight symtab interface for ParseAPI, ProcControl, and Stackwalker; disables DyninstAPI build"
    OFF)

# Use ParseAPI analysis in Stackwalker?
dyninst_add_option(SW_ANALYSIS_STEPPER
                   "Use ParseAPI-based analysis stepper in Stackwalker" ON)
dyninst_add_option(BUILD_TARBALLS
                   "Build Dyninst package tarballs. Requires git archive, tar, gzip." OFF)
dyninst_add_option(BUILD_RTLIB_32 "Build 32-bit runtime library on mixed 32/64 systems"
                   OFF)
dyninst_add_option(
    BUILD_RTLIB
    "Building runtime library (can be disabled safely for component-level builds)" ON)
dyninst_add_option(BUILD_DOCS "Build manuals from LaTeX sources" ON)
dyninst_add_option(ENABLE_LTO "Enable Link-Time Optimization" OFF)
dyninst_add_option(ENABLE_DEBUGINFOD "Enable debuginfod support" OFF)
dyninst_add_option(BUILD_PARSE_THAT "Enable building parseThat executable" ON)
dyninst_add_option(BUILD_TBB "Enable building TBB internally" OFF)
dyninst_add_option(BUILD_BOOST "Enable building Boost internally" OFF)
dyninst_add_option(BUILD_ELFUTILS "Enable building elfutils internally" OFF)
dyninst_add_option(BUILD_LIBIBERTY "Enable building libiberty internally" OFF)

if(STERILE_BUILD)
    foreach(_SUBPACKAGE TBB BOOST ELFUTILS LIBIBERTY)
        dyninst_force_option(BUILD_${_SUBPACKAGE} OFF)
    endforeach()
endif()

# Some global on/off switches
if(LIGHTWEIGHT_SYMTAB)
    add_compile_definitions(WITH_SYMLITE)
else()
    add_compile_definitions(WITH_SYMTAB_API)
endif()

if(SW_ANALYSIS_STEPPER)
    add_compile_definitions(USE_PARSE_API)
endif()
