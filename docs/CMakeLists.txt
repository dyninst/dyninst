# The manuals.  Everything they need is here: DyninstDocs defines the two
# functions the manuals call, and each manual is added as a directory of its
# own so that it has a scope to hang its targets on.  dataflowAPI needs that
# most, being include()d by the libraries rather than added, so it has no
# CMakeLists scope anywhere else.
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

# What 'docs-install' installs, one file name per line, for anything that has
# to tell a complete install from a partial one -- the Documentation workflow
# does exactly that.  The list is accumulated by the install() rule in
# dyninst_add_latex_document, so it describes what the build will really
# produce rather than what the source tree happens to look like.  It is
# written here because it is only complete once every manual has been added.
get_property(_installed GLOBAL PROPERTY DYNINST_DOCS_INSTALLED_FILES)
list(SORT _installed)
list(JOIN _installed "\n" _installed)
file(
  GENERATE
  OUTPUT "${PROJECT_BINARY_DIR}/docs-manuals.txt"
  CONTENT "${_installed}\n")
