# Builds patchAPI.pdf into this directory's counterpart in the build tree.
# The work is in cmake/DyninstDocs.cmake.
dyninst_add_manual(patchAPI)

# Examples-check.C supplies the context the printed examples leave implicit
# and includes each of them, so compiling it compiles them all.
dyninst_validate_listings(patchAPI SOURCES examples/Examples-check.C)
