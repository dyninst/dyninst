.. _`sec:BPatch_sourceObj.h`:

BPatch_sourceObj.h
##################

.. cpp:class:: BPatch_sourceObj
   
  **A generic source-level object**

  Derived classes include :cpp:class:`BPatch_function`, :cpp:class:`BPatch_module`,
  and :cpp:class:`BPatch_image`. It can be used
  to build a "generic" source navigator using the getObjParent and
  getSourceObj methods to get parents and children of a given level (i.e.
  the parent of a module is an image, and the children will be the
  functions).

  .. cpp:function:: virtual ~BPatch_sourceObj()

  .. cpp:function:: BPatch_sourceType getSrcType()

    Returns the type of the current source object.

  .. cpp:function:: virtual bool getSourceObj(BPatch_Vector<BPatch_sourceObj *> &) = 0

    Returns the child source objects of the current source object. For
    example, when called on a BPatch_sourceProgram object this will return
    objects of type BPatch_sourceFunction. When called on a
    BPatch_sourceFunction object it may return BPatch_sourceOuterLoop and
    BPatch_sourceStatement objects.

  .. cpp:function:: virtual bool getVariables(BPatch_Vector<BPatch_variableExpr *> &) = 0

  .. cpp:function:: virtual BPatch_sourceObj *getObjParent() = 0

    Returns the child source objects of the current source object. For
    example, when called on a BPatch_sourceProgram object this will return
    objects of type BPatch_sourceFunction. When called on a
    BPatch_sourceFunction object it may return BPatch_sourceOuterLoop and
    BPatch_sourceStatement objects.

  .. cpp:function:: BPatch_language getLanguage()

    Return the source language of the current BPatch_sourceObject. For
    programs that are written in more than one language, BPatch_mixed will
    be returned. If there is insufficient information to determine the
    language, BPatch_unknownLanguage will be returned.

  .. cpp:function:: const char *getLanguageStr()


.. cpp:enum:: BPatch_language

  .. cpp:enumerator:: BPatch_c
  .. cpp:enumerator:: BPatch_cPlusPlus
  .. cpp:enumerator:: BPatch_fortran
  .. cpp:enumerator:: BPatch_fortran77
  .. cpp:enumerator:: BPatch_fortran90
  .. cpp:enumerator:: BPatch_f90_demangled_stabstr
  .. cpp:enumerator:: BPatch_fortran95
  .. cpp:enumerator:: BPatch_assembly
  .. cpp:enumerator:: BPatch_mixed
  .. cpp:enumerator:: BPatch_hpf
  .. cpp:enumerator:: BPatch_java
  .. cpp:enumerator:: BPatch_unknownLanguage

.. cpp:enum:: BPatch_sourceType

  .. cpp:enumerator:: BPatch_sourceUnknown
  .. cpp:enumerator:: BPatch_sourceProgram
  .. cpp:enumerator:: BPatch_sourceModule
  .. cpp:enumerator:: BPatch_sourceFunction
  .. cpp:enumerator:: BPatch_sourceOuterLoop
  .. cpp:enumerator:: BPatch_sourceLoop
  .. cpp:enumerator:: BPatch_sourceStatement

