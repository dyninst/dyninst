# The manuals.  Each is a directory of its own so that it has a scope to hang
# its targets on; dataflowAPI has none anywhere else, being include()d by the
# libraries rather than added.
include(DyninstDocs)

add_subdirectory(dataflowAPI/manual-latex)
add_subdirectory(dynC_API/manual-latex)
add_subdirectory(dyninstAPI/manual-latex)
add_subdirectory(instructionAPI/manual-latex)
add_subdirectory(parseAPI/manual-latex)
add_subdirectory(patchAPI/manual-latex)
add_subdirectory(proccontrol/manual-latex)
add_subdirectory(stackwalk/manual-latex)
add_subdirectory(symtabAPI/manual-latex)

# What 'docs-install' installs, one name per line, for the Documentation
# workflow to check against.  Accumulated by the install() rule in
# dyninst_add_latex_document, and written here because the list is only
# complete once every manual has been added.
get_property(_installed GLOBAL PROPERTY DYNINST_DOCS_INSTALLED_FILES)
list(SORT _installed)
list(JOIN _installed "\n" _installed)
file(
  GENERATE
  OUTPUT "${PROJECT_BINARY_DIR}/docs-manuals.txt"
  CONTENT "${_installed}\n")
