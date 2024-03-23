.. _`sec-dev:BPatch_dll.h`:

BPatch_dll.h
############


**TEMPORARY PARADYND FLOWGRAPH KLUGE**

If we are building BPatch classes into paradynd we want BPATCH_DLL_EXPORT
to be defined as the empty string (for all platforms). This currently tests
SHM_SAMPLING because it is defined for paradynd and not for the dyninst
dll or dyninst clients.

.. code:: cpp

  #ifdef SHM_SAMPLING
  # define BPATCH_DLL_EXPORT
  #else
  # if defined(_MSC_VER)
  #   ifdef BPATCH_DLL_BUILD
        // we are building the dyninstAPI DLL
  #     define BPATCH_DLL_EXPORT __declspec(dllexport)
  # else
      // we are not building the dyninstAPI DLL
  #   define BPATCH_DLL_EXPORT __declspec(dllimport)
  #   if _MSC_VER >= 1300
  #     define BPATCH_DLL_IMPORT   1
  #   endif
  # endif
  #else
    // we are not building for a Windows target
  # define BPATCH_DLL_EXPORT  __attribute__((visibility ("default")))
  #endif

.. cpp:var:: const char V_libdyninstAPI[]

  Declare our version string
