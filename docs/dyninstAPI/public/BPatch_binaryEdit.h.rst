BPatch_binaryEdit.h
===================

``BPatch_binaryEdit``
---------------------
.. cpp:namespace:: BPatch_binaryEdit

.. cpp:class:: BPatch_binaryEdit
   
   The BPatch_binaryEdit class represents a set of executable files and
   library files for binary rewriting. BPatch_binaryEdit inherits from the
   BPatch_addressSpace class, where most functionality for binary rewriting
   is found.
   
   .. cpp:function:: bool writeFile(const char *outFile)
      
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