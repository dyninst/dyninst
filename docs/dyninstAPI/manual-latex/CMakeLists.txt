# Builds dyninstAPI.pdf into this directory's counterpart in the build tree.
# The work is in cmake/DyninstDocs.cmake.
dyninst_add_manual(dyninstAPI)
dyninst_validate_listings(
  dyninstAPI SOURCES examples/BinaryAnalysis.C examples/InstrumentingFunction.C
                     examples/InstrumentingMemoryAccess.C examples/retee.C)
