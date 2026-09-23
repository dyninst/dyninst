# Builds instructionAPI.pdf into this directory's counterpart in the build
# tree.  The work is in cmake/DyninstDocs.cmake.
#
# Every figure in fig/ carries a PDF transparency group and pdfTeX keeps only
# one per output page, so it warns whenever two of them land together.  Which
# figures those are moves with the page flow, so the warning is allowed by
# kind rather than by count.  Regenerating the figures without a /Group would
# let this allowance go.
dyninst_add_manual(
  instructionAPI ALLOW_WARNINGS "multiple pdfs with page group included in a single page")
