include_guard(GLOBAL)

# The test attaches to a child process with ptrace, so it is only built when it can run;
# the program's own check of the option is then always true.
dyninst_add_test(
  stackwalk_vdso_symtab
  TYPE regression
  SOURCE_FILES vdsoSymtab.cpp
  DEPS stackwalk symtabAPI
  DEFINES DYNINST_ENABLE_DYNAMIC_TESTS=1
  REQUIRES dynamic
)
