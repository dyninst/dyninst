include_guard(GLOBAL)

# The test attaches to a child process with ptrace. It is compiled with the library's
# platform macros (DYNINST_PLATFORM_CAPABILITIES), so os_linux is defined as in the
# library.
dyninst_add_test(
  stackwalk_vdso_symtab
  TYPE regression
  SOURCE_FILES vdsoSymtab.cpp
  DEPS stackwalk symtabAPI
  DEFINES ${DYNINST_PLATFORM_CAPABILITIES}
  REQUIRES dynamic
)
