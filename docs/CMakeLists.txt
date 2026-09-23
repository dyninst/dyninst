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
