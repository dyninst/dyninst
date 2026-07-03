include_guard(GLOBAL)

if(USE_OpenMP)
  find_package(OpenMP REQUIRED)

  # Dyninst's bundled Sawyer/ROSE only compiles its pool allocator and
  # shared-pointer reference counting as thread-safe when _REENTRANT is defined
  # (see dataflowAPI/rose/util/Sawyer.h). GCC's -fopenmp defines _REENTRANT
  # implicitly, but Clang's -fopenmp/-fopenmp=libomp does not. Without it, the
  # parallel parser races on the allocator and crashes. Define it globally
  # (matching GCC's behavior) so the setting is consistent across all TUs.
  add_compile_definitions(_REENTRANT)
else()
  # Dummy targets so we don't have to check 'USE_OpenMP' everywhere
  add_library(OpenMP::OpenMP_C INTERFACE IMPORTED)
  add_library(OpenMP::OpenMP_CXX INTERFACE IMPORTED)
  add_library(OpenMP::OpenMP_Fortran INTERFACE IMPORTED)
endif()
