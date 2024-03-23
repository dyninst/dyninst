.. _`sec-dev:BPatch_binaryEdit.h`:

BPatch_binaryEdit.h
###################

.. cpp:namespace:: dev

.. cpp:class:: BPatch_binaryEdit : public BPatch_addressSpace

  .. cpp:member:: private std::map<std::string, BinaryEdit *> llBinEdits
  .. cpp:member:: private std::map<std::string, BPatch_object*> loadedLibrary
  .. cpp:member:: private BinaryEdit *origBinEdit
  .. cpp:member:: private std::vector<BinaryEdit *> rtLib
  .. cpp:member:: private bool creation_error

  .. cpp:function:: BPatch_binaryEdit(const char *path, bool openDependencies)

    Creates a new :cpp:class:`BinaryEdit` for the file at ``path`` and associates it with this edit.

    If ``openDependencies`` is ``true``, the dependencies of the original ``BinaryEdit``
    are opened and associated with this edit.

  .. cpp:function:: private bool replaceTrapHandler()

    Here's the story. We may need to install a trap handler for instrumentation to work in the rewritten
    binary. This doesn't play nicely with trap handlers that the binary itself registers. So we're going
    to replace every call to sigaction in the binary with a call to our wrapper. This wrapper (1)
    Ignores attempts to register a SIGTRAP, (2) Passes everything else through to sigaction. It's called
    "dyn_sigaction". This is just a multiplexing function over each child binaryEdit object because
    they're all individual.
