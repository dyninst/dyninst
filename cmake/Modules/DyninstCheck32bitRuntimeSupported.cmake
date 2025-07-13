#[=======================================================================[.rst:
Dyninst32bitRuntimeSupported
----------------------------

This module provides the ``dyninst_check_32bit_runtime_supported()`` function
to check whether the linker supports dynamically or statically linking to a
32-bit runtime.

.. command:: dyninst_check_32bit_runtime_supported

  .. code-block:: cmake

    dyninst_check_32bit_runtime_supported([LANGUAGES <lang>...])

  Options are:

  ``LANGUAGES <lang>...``
    Check the linkers used for each of the specified languages.
    If this option is not provided, the command checks all enabled languages.

Variables
^^^^^^^^^

For each language checked, ``dyninst_check_32bit_runtime_supported()`` function
defines the follow cache variables:

 ``DYNINST_<lang>_32BIT_RUNTIME_SUPPORTED``
   Set to true if a 32-bit runtime is supported by the linker and false otherwise.

 ``DYNINST_<lang>_32BIT_RUNTIME_FLAG``
   The flag needed to use a 32-bit runtime or blank otherwise.

 ``DYNINST_<lang>_32BIT_STATIC_RUNTIME_SUPPORTED``
   Set to true if a static 32-bit runtime is supported by the linker and false otherwise.

 ``DYNINST_<lang>_32BIT_STATIC_RUNTIME_FLAG``
   The flag needed to use a static 32-bit runtime or blank otherwise.

#]=======================================================================]
include_guard(GLOBAL)

include(DyninstCheckCompilerFlag)

function(dyninst_check_32bit_runtime_supported)

  cmake_parse_arguments(_dyn32rt_args "" "" "LANGUAGES" "${ARGN}")
  if(_dyn32rt_args_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unparsed arguments: ${_dyn32rt_args_UNPARSED_ARGUMENTS}")
  endif()

  if(_dyn32rt_args_LANGUAGES)
    set(_languages "${_dyn32rt_args_LANGUAGES}")
  else()
    # User did not set any languages, use defaults
    get_property(_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
  endif()

  # gcc and clang both use 'm32', but allow for other compilers
  set(_possible_32bit_flags "-m32")

  foreach(_32bit_flag ${_possible_32bit_flags})
    set(_saved_link_opts ${CMAKE_REQUIRED_LINK_OPTIONS})

    set(CMAKE_REQUIRED_LINK_OPTIONS "${_32bit_flag}")

    foreach(_lang ${_languages})
      _dyn32bitrt_int(${_lang} ${_32bit_flag} "")

      # gcc and clang both use '-static', but allow for other compilers
      set(_possible_static_flags "-static")

      foreach(_static_flag ${_possible_static_flags})
        _dyn32bitrt_int(${_lang} ${_32bit_flag} ${_static_flag})
      endforeach()
    endforeach()

    set(CMAKE_REQUIRED_LINK_OPTIONS "${_saved_link_opts}")
  endforeach()

endfunction()

function(_dyn32bitrt_int _lang _32bit_flag _static_flag)
  if(_static_flag)
    set(_sname "STATIC_")
    set(_smsg "static")
  endif()

  set(_32rf DYNINST_${_lang}_32BIT_${_sname}RUNTIME_FLAG)

  # If a flag has already been found, don't re-check.
  if(${_32rf})
    return()
  endif()

  set(_32rs DYNINST_${_lang}_32BIT_${_sname}RUNTIME_SUPPORTED)

  dyninst_check_compiler_flag(${_lang} "${_32bit_flag} ${_static_flag}" ${_32rs})

  set(${_32rs}
      ${${_32rs}}
      CACHE BOOL "${_lang} ${_smsg} 32-bit runtime supported" FORCE)

  set(${_32rf}
      "${_32bit_flag}"
      CACHE STRING "${_lang} ${_smsg} 32-bit runtime flag" FORCE)

endfunction()
