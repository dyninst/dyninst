#=========================================================================
# LanguageStandards.cmake
#
# Configure C++ and C Language API and ABI standards for Dyninst
#
#=========================================================================

#
# --------  C++ language features ----------------
#

# Disable compiler-specific C++ language extensions (e.g., gnu++11)
set(CMAKE_CXX_EXTENSIONS OFF)

# Require C++11 support
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Require the standards-compliant C++11 ABI for gcc
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.1")
    message(FATAL_ERROR "Dyninst requires gcc >= 5.1")
  endif()
  add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)
endif()


#
# --------  C language features ----------------
#

# Disable compiler-specific C language extensions (e.g., gnu99)
set(CMAKE_C_EXTENSIONS OFF)

# Require C99 support
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
