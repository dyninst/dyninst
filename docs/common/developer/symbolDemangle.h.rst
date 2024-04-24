.. _`sec:symbolDemangle.h`:

symbolDemangle.h
################

.. cpp:function:: char *symbol_demangle(const char *symName, int includeParams)

  Returns a malloc'd string that is the demangled symbol name.  THe caller is
  responsible for the freeing this memory.  Returns NULL on malloc failure.
  The symbol name symName is demangled using the cplus_demangle function after
  first removing any versioning suffixes (first '@')
  to create the mangled name.  If cplus_demangle fails, the mangled name
  is returned unmodified.  If cplus_demangle succeeds and includeParams is
  false, then any clone suffixes (the first '.' to the end of the mangled
  name) are appended to the value from cplus_demangle.

  Other than the removal of versioning suffixes, and appending any
  clone suffixes if includeParams is false to the result, the result should be
  equivalent to using c++filt.

.. csv-table:: c++filt and cplus_demangle options
  :header: "includeParams", "c++ filt opts", "cplus_demangle opts"

  "false","-i -p","DMGL_AUTO | DMGL_ANSI"
  "true","-i","DMGL_AUTO | DMGL_ANSI | DMGL_PARAMS"
