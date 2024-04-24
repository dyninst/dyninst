.. _`sec:BPatch_binaryEdit.h`:

BPatch_binaryEdit.h
###################

.. cpp:class:: BPatch_binaryEdit : public BPatch_addressSpace

  **A set of executable and library files for binary rewriting**

  .. cpp:function:: void getAS(std::vector<AddressSpace *> &as)
  .. cpp:function:: BinaryEdit *lowlevel_edit() const
  .. cpp:function:: bool isMultiThreadCapable() const
  .. cpp:function:: processType getType()
  .. cpp:function:: bool getTerminated()
  .. cpp:function:: bool getMutationsActive()

  .. cpp:function:: bool writeFile(const char * outFile)

    Rewrite a BPatch_binaryEdit to disk. The original file opened with this
    BPatch_binaryEdit is written to the current working directory with the
    name outFile. If any dependent libraries were also opened and have
    instrumentation or other modifications, then those libraries will be
    written to disk in the current working directory under their original
    names.

    A rewritten dependency library should only be used with the original
    file that was opened for rewriting. For example, if the file a.out and
    its dependent library libfoo.so were opened for rewriting, and both had
    instrumentation inserted, then the rewritten libfoo.so should not be
    used without the rewritten a.out. To build a rewritten libfoo.so that
    can load into any process, libfoo.so must be the original file opened by
    BPatch::openBinary.

    This function returns true if it successfully wrote a file, or false
    otherwise.

  .. cpp:function:: ~BPatch_binaryEdit()
  .. cpp:function:: BPatch_image * getImage()
  .. cpp:function:: void beginInsertionSet()

    Start the batch insertion of multiple points all calls to insertSnippet
    after this call will not actually instrument until finalizeInsertionSet is called

  .. cpp:function:: bool finalizeInsertionSet(bool atomic, bool *modified = NULL)

    Finalizes all instrumentation logically added since a call to beginInsertionSet.

    Returns true if all instrumentation was successfully inserted otherwise, none  was.
    Individual instrumentation can be manipulated via the BPatchSnippetHandles returned
    from individual calls to insertSnippet.  atomic: if true, all instrumentation will be
    removed if any fails to go in.  modified: if provided, and set to true by
    finalizeInsertionSet, additional steps were taken to make the installation work, such
    as modifying process state. Note that such steps will be taken whether or not a variable
    is provided.

  .. cpp:function:: virtual BPatch_object * loadLibrary(const char *libname, bool reload = false)

    Load a shared library into the mutatee's address space.

    Returns true if successful
