#=========================================================================
# LanguageStandards.cmake
#
# Configure C++ and C Language API and ABI standards for Dyninst
#
#=========================================================================

#
# C/C++ language standard cmake options.
#

set(DYNINST_CXX_LANGUAGE_STANDARD
    "17"
    CACHE STRING "C++ language standard version.")
set(DYNINST_C_LANGUAGE_STANDARD
    "11"
    CACHE STRING "C language standard version.")

#
# --------  C++ language features ----------------
#

# Disable compiler-specific C++ language extensions (e.g., gnu++17)
set(CMAKE_CXX_EXTENSIONS OFF)

# Require C++17 support
set(CMAKE_CXX_STANDARD ${DYNINST_CXX_LANGUAGE_STANDARD})
message(STATUS "C++ language standard:  ${DYNINST_CXX_LANGUAGE_STANDARD}")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Require a compiler with usable C++17 support.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "10.0")
    message(FATAL_ERROR "Dyninst requires gcc >= 10.0")
  endif()
endif()

#
# --------  C language features ----------------
#

# Disable compiler-specific C language extensions (e.g., gnu99)
set(CMAKE_C_EXTENSIONS OFF)

# Require C11 support
set(CMAKE_C_STANDARD ${DYNINST_C_LANGUAGE_STANDARD})
message(STATUS "C language standard:  ${DYNINST_C_LANGUAGE_STANDARD}")
set(CMAKE_C_STANDARD_REQUIRED ON)
