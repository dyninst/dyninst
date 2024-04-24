.. _`sec:parseauxv.h`:

parseauxv.h
###########

.. cpp:class:: AuxvParser

  .. cpp:function:: static AuxvParser *createAuxvParser(int pid, unsigned asize)
  .. cpp:function:: void deleteAuxvParser()
  .. cpp:function:: Dyninst::Address getInterpreterBase()
  .. cpp:function:: bool parsedVsyscall()
  .. cpp:function:: Dyninst::Address getVsyscallBase()
  .. cpp:function:: Dyninst::Address getVsyscallText()
  .. cpp:function:: Dyninst::Address getVsyscallEnd()
  .. cpp:function:: Dyninst::Address getProgramBase()
  .. cpp:function:: Dyninst::Address getPageSize()
