project(tests_dataflowAPI LANGUAGES CXX)

add_subdirectory(rose)

dyninst_add_test(
  dataflowAPI_stack_analysis_concurrent_cache
  TYPE unit
  SOURCE_FILES stack_analysis/concurrent_cache.cpp
  DEPS parseAPI Threads::Threads
)
