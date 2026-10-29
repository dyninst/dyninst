project(tests_dataflowAPI_rose_registers LANGUAGES CXX)

dyninst_add_test(rose_reg_x86 TYPE unit SOURCE_FILES x86.cpp DEPS parseAPI)

dyninst_add_test(rose_reg_x86_64 TYPE unit SOURCE_FILES x86_64.cpp DEPS parseAPI)

dyninst_add_test(rose_reg_aarch64 TYPE unit SOURCE_FILES aarch64.cpp DEPS parseAPI)

dyninst_add_test(rose_reg_ppc32 TYPE unit SOURCE_FILES ppc32.cpp DEPS parseAPI)

dyninst_add_test(rose_reg_ppc64 TYPE unit SOURCE_FILES ppc64.cpp DEPS parseAPI)

dyninst_add_test(rose_reg_riscv64 TYPE unit SOURCE_FILES riscv64.cpp DEPS parseAPI)
