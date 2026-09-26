# Builds dataflowAPI.pdf into this directory's counterpart in the build tree.
# The work is in cmake/DyninstDocs.cmake.
dyninst_add_manual(dataflowAPI)

# dataflowAPI has no library target of its own -- its sources are built into
# parseAPI -- so the examples borrow parseAPI's headers.
dyninst_validate_listings(
  dataflowAPI USES parseAPI
  SOURCES examples/Slicing.C examples/SymbolicEvaluation.C
          examples/LivenessAnalysis.C examples/StackAnalysis.C)
