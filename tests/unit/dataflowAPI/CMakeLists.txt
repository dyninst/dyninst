project(tests_dataflowAPI LANGUAGES CXX)

add_subdirectory(rose)

dyninst_add_test(
  dataflowAPI_stack_visitor
  TYPE unit
  SOURCE_FILES stack_visitor.cpp
  DEPS parseAPI
  # SymEvalVisitors.h includes ROSE's BitVector.h, whose checkRange has an unused
  # parameter when assertions are off.
  COMPILE_OPTIONS -Wno-unused-parameter
)
