project(tests_dataflowAPI_rose_semantics LANGUAGES CXX)

add_executable(rose_semantics_aarch64_branch_shift aarch64_branch_shift.cpp)
target_compile_options(
  rose_semantics_aarch64_branch_shift
  PRIVATE ${UNIT_TESTS_WARNING_FLAGS}
)
target_link_libraries(rose_semantics_aarch64_branch_shift PRIVATE parseAPI)
add_test(
  NAME rose_semantics_aarch64_branch_shift
  COMMAND rose_semantics_aarch64_branch_shift
)
set_tests_properties(rose_semantics_aarch64_branch_shift PROPERTIES LABELS "unit")
