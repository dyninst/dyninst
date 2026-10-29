include_guard(GLOBAL)

# The test attaches to a child process with ptrace. The library's platform macros, so the
# test uses os_linux as the library does.
dyninst_add_test(
  stackwalk_vdso_symtab
  TYPE regression
  SOURCE_FILES vdsoSymtab.cpp
  DEPS stackwalk symtabAPI
  DEFINES ${DYNINST_PLATFORM_CAPABILITIES}
  REQUIRES dynamic
)
