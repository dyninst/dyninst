include_guard(GLOBAL)

if(USE_OpenMP)
  # Clang needs the OpenMP runtime that came with it: a newer Clang calls
  # entry points an older libomp lacks.  FindOpenMP takes the runtime from
  # the link directories of a test program, and the one beside the compiler
  # can come after the system directories there.  So when more than one Clang
  # is installed, it can pick another's libomp, and the link fails.  Prefer
  # the libomp in the compiler's own lib directory, two levels above its
  # resource directory, when there is one; set OpenMP_omp_LIBRARY to choose
  # another.
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT OpenMP_omp_LIBRARY)
    execute_process(
      COMMAND ${CMAKE_CXX_COMPILER} -print-resource-dir
      OUTPUT_VARIABLE _dyninst_res_dir
      OUTPUT_STRIP_TRAILING_WHITESPACE
      RESULT_VARIABLE _dyninst_clang_rc
      ERROR_QUIET
    )
    set(_dyninst_libomp "${_dyninst_res_dir}/../../libomp${CMAKE_SHARED_LIBRARY_SUFFIX}")
    # EXISTS follows symbolic links, so a dangling one -- libomp-N-dev not
    # installed -- is passed over
    if(_dyninst_clang_rc EQUAL 0 AND EXISTS "${_dyninst_libomp}")
      get_filename_component(_dyninst_libomp "${_dyninst_libomp}" ABSOLUTE)
      set(
        OpenMP_omp_LIBRARY
        "${_dyninst_libomp}"
        CACHE FILEPATH
        "Path to the omp library for OpenMP"
      )
      message(STATUS "Using the compiler's own libomp: ${_dyninst_libomp}")
    endif()
    unset(_dyninst_res_dir)
    unset(_dyninst_clang_rc)
    unset(_dyninst_libomp)
  endif()

  find_package(OpenMP REQUIRED)
else()
  # Dummy targets so we don't have to check 'USE_OpenMP' everywhere
  add_library(OpenMP::OpenMP_C INTERFACE IMPORTED)
  add_library(OpenMP::OpenMP_CXX INTERFACE IMPORTED)
  add_library(OpenMP::OpenMP_Fortran INTERFACE IMPORTED)
endif()
