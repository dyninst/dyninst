
INCLUDE(CheckCSourceCompiles)

MACRO (CHECK_MUTATEE_COMPILER _COMPILER _COMP_FLAG _LINK_FLAG _RESULT)
   SET(${_RESULT})
   SET(SAFE_CMAKE_C_COMPILER "${CMAKE_C_COMPILER}")
   SET(SAFE_CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
   SET(SAFE_CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

   SET(CMAKE_C_COMPILER "${_COMPILER}")
   SET(CMAKE_EXE_LINKER_FLAGS "${_LINK_FLAG}")
   SET(CMAKE_C_FLAGS "${_COMP_FLAG}")

   CHECK_C_SOURCE_COMPILES("#include <signal.h> \n int main(void) { return 0; }" ${_RESULT}

     # Some compilers do not fail with a bad flag
     FAIL_REGEX "warning: command line option .* is valid for .* but not for C"
                                                            # Apple gcc
     FAIL_REGEX "unrecognized .*option"                     # GNU
     FAIL_REGEX "unknown .*option"                          # Clang
     FAIL_REGEX "ignoring unknown option"                   # MSVC
     FAIL_REGEX "warning D9002"                             # MSVC, any lang
     FAIL_REGEX "[Uu]nknown option"                         # HP
     FAIL_REGEX "[Ww]arning: [Oo]ption"                     # SunPro
     FAIL_REGEX "command option .* is not recognized"       # XL
     FAIL_REGEX "cannot find"                               # Missing libraries
     )
   SET (CMAKE_C_COMPILER "${SAFE_CMAKE_C_COMPILER}")
   SET (CMAKE_EXE_LINKER_FLAGS "${SAFE_CMAKE_EXE_LINKER_FLAGS}")
   SET (CMAKE_C_FLAGS "${SAFE_CMAKE_C_FLAGS}")
ENDMACRO (CHECK_MUTATEE_COMPILER)
