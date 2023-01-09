include_guard(GLOBAL)

if(USE_OpenMP)
  find_package(OpenMP REQUIRED)
else()
  # Dummy targets so we don't have to check 'USE_OpenMP' everywhere
  add_library(OpenMP::OpenMP_C INTERFACE IMPORTED)
  add_library(OpenMP::OpenMP_CXX INTERFACE IMPORTED)
  add_library(OpenMP::OpenMP_Fortran INTERFACE IMPORTED)
endif()
