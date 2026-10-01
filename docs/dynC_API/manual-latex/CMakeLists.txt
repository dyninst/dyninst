# Builds dynC_API.pdf into this directory's counterpart in the build tree.
# The work is in cmake/DyninstDocs.cmake.
dyninst_add_manual(dynC_API)

# The raw-snippet walkthrough is BPatch code, so it borrows dyninstAPI.
# Examples-check.C includes its eleven steps in order.
dyninst_validate_listings(
  dynC_API
  USES dyninstAPI
  SOURCES examples/Examples-check.C)
