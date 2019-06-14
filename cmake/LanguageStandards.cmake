#=========================================================================
# LanguageStandards.cmake
#
# Configure C++ and C Language API and ABI standards for Dyninst
#
#   ----------------------------------------
#
# Accepts the following CMake variables
#
# USE_CXX11_ABI - Enable using the GNU C++11 ABI (aka, the post gcc-5 ABI)
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

set (USE_CXX11_ABI "" CACHE STRING "Override the default GNU C++11 ABI setting")
if (NOT ("${USE_CXX11_ABI}" STREQUAL ""))
  if (${USE_CXX11_ABI})
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)
  else()
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
  endif()
endif()


#
# --------  C language features ----------------
#

# Disable compiler-specific C language extensions (e.g., gnu99)
set(CMAKE_C_EXTENSIONS OFF)

# Require C99 support
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
