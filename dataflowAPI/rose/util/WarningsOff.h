// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




// Do not protect this file with include-once macros.

// This file can be included to turn off certain warnings and re-enable them by including WarningsRestore.h

#ifdef SAWYER_CONFIGURED
#   if _MSC_VER
#       pragma warning(push)

        // Warning 4251: class 'XXX' needs to have dll-interface to be used by clients of class 'YYY'.
        // See http://unknownroad.com/rtfm/VisualStudio/warningC4251.html for a discussion of this warning.
        // We are turning this warning off within Sawyer header files because we are not trying to have portability over
        // different versions of its prerequisite libraries (or even over its own *.C components, which also produce these
        // warnings).  In fact, we also don't try to be portable across different versions of Sawyer or compilers.
#       pragma warning(disable:4251)

        // Warning 4996: This function or variable may be unsafe.
        // This warning is emitted even for safe usage of functions like fopen, strerror, localtime, getenv, sprintf...
#       pragma warning(disable:4996)

#   endif
#else
#   error "The <Sawyer/Sawyer.h> file must have been included already."
#endif
