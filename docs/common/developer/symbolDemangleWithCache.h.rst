.. _`sec:symbolDemangleWithCache.h`:

symbolDemangleWithCache.h
#########################

.. cpp:function:: std::string const& symbol_demangle_with_cache(const std::string &symName, bool includeParams)

  Returns a demangled symbol using symbol_demangle with a per thread, single-entry cache of the previous demangling.
