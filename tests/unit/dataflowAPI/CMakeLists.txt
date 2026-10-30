project(tests_dataflowAPI LANGUAGES CXX)

add_subdirectory(rose)

dyninst_add_test(
  dataflowAPI_stack_analysis_concurrent_cache
  TYPE unit
  SOURCE_FILES stack_analysis/concurrent_cache.cpp
  DEPS parseAPI Threads::Threads
  # The test analyzes the host's libc, which needs code generation for the
  # host. Parsing x86_64 code with i386 code generation crashes (#2397).
  REQUIRES native
)

dyninst_add_test(
  dataflowAPI_stack_visitor
  TYPE unit
  SOURCE_FILES stack_visitor.cpp
  DEPS parseAPI
  # SymEvalVisitors.h includes ROSE's BitVector.h, whose checkRange has an unused
  # parameter when assertions are off.
  COMPILE_OPTIONS -Wno-unused-parameter
)
