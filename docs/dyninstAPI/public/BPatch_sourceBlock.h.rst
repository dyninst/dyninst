BPatch_sourceBlock.h
====================

``BPatch_sourceBlock``
----------------------
.. cpp:namespace:: BPatch_sourceBlock

.. cpp:class:: BPatch_sourceBlock
   
   An object of this class represents a source code level block. Each
   source block objects consists of a source file and a set of source lines
   in that source file. This class is used to fill source line information
   for each basic block in the control flow graph. For each basic block in
   the control flow graph there is one or more source block object(s) that
   correspond to the source files and their lines contributing to the
   instruction sequence of the basic block.
   
   .. cpp:function:: const char* getSourceFile()
      
      Returns a pointer to the name of the source file in which this source
      block occurs.
      
   .. cpp:function:: void getSourceLines(std::vector<unsigned short>&)
      
      Fill the given vector with a list of the lines contained within this
      source block.