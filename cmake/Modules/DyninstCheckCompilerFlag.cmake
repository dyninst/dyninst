include_guard(GLOBAL)

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

# A generic version of Check{C,CXX}CompilerFlag
#
#   CMake provides the same as of 3.19, but we currently
#   only require 3.14.
#

function(dyninst_check_compiler_flag _lang _flag _var)

  if(_lang STREQUAL "C")
    check_c_compiler_flag(${_flag} ${_var})
  elseif(_lang STREQUAL "CXX")
    check_cxx_compiler_flag(${_flag} ${_var})
  else()
    message(FATAL_ERROR "dyninst_check_compiler_flag: unsupported language '${_lang}'")
  endif()

endfunction()
