// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




// Do not protect this file with include-once macros.

// This file is the counterpart of <Sawyer/WarningsOff.h> and restores warnings to their state before WarningsOff was
// included. The WarningsRestore should be included once per WarningsOff -- they nest like parentheses.

#ifdef SAWYER_CONFIGURED
#    if _MSC_VER
#        pragma warning(pop)
#    endif
#else
#    error "The <Sawyer/Sawyer.h> file must have been included already."
#endif
