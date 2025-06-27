include_guard(GLOBAL)

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

# Check if the CMAKE_<LANG>_COMPILER supports static linking
#
# NOTE: This is only needed when creating executables because
#
#         add_executable(foo STATIC foo.cpp)
#
#       doesn't (currently) work. Libraries should use the STATIC
#       keyword as usual.

# gcc and clang both use '-static', but allow for other compilers
set(_possible_static_flags "-static")

foreach(_flag ${_possible_static_flags})
  set(_saved_link_opts ${CMAKE_REQUIRED_LINK_OPTIONS})

  set(CMAKE_REQUIRED_LINK_OPTIONS "${_flag}")

  check_c_compiler_flag("${_flag}" DYNINST_C_HAVE_STATIC_LINK_FLAG)

  if(DYNINST_C_HAVE_STATIC_LINK_FLAG AND NOT DYNINST_C_STATIC_LINK_FLAG)
    set(DYNINST_C_STATIC_LINK_FLAG
        "${_flag}"
        CACHE STRING "C static link flag" FORCE)
  endif()

  check_cxx_compiler_flag("${_flag}" DYNINST_CXX_HAVE_STATIC_LINK_FLAG)

  if(DYNINST_CXX_HAVE_STATIC_LINK_FLAG AND NOT DYNINST_CXX_STATIC_LINK_FLAG)
    set(DYNINST_CXX_STATIC_LINK_FLAG
        "${_flag}"
        CACHE STRING "C++ static link flag" FORCE)
  endif()

  set(CMAKE_REQUIRED_LINK_OPTIONS "${_saved_link_opts}")
endforeach()
