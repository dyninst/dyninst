include_guard(GLOBAL)

if(DYNINST_OS_UNIX)
  set(_required REQUIRED)
endif()

# Prefer the compiler's -pthread flag over a bare -lpthread. On GCC and Clang,
# -pthread also defines _REENTRANT, which Dyninst's bundled Sawyer/ROSE relies
# on to compile its pool allocator and shared-pointer reference counting as
# thread-safe (see dataflowAPI/rose/util/Sawyer.h). GCC's -fopenmp defines
# _REENTRANT implicitly, but Clang's -fopenmp=libomp does not; using -pthread
# makes the thread-safe path explicit and compiler-independent.
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads ${_required})

if(NOT Threads_FOUND)
  # make a dummy
  add_library(Threads::Threads INTERFACE IMPORTED)
endif()
