# Builds symtabAPI.pdf into this directory's counterpart in the build tree.
# The work is in cmake/DyninstDocs.cmake.
dyninst_add_manual(symtabAPI)

# Examples-check.C supplies the context the printed examples leave implicit
# and includes each of them, so compiling it compiles them all.
dyninst_validate_listings(symtabAPI SOURCES examples/Examples-check.C)
