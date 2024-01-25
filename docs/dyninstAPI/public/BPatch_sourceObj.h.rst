.. _`sec:BPatch_sourceObj.h`:

BPatch_sourceObj.h
##################

.. cpp:class:: BPatch_sourceObj
   
  The BPatch_sourceObj class is the C++ superclass for the
  BPatch_function, BPatch_module, and BPatch_image classes. It provides a
  set of common methods for all three classes. In addition, it can be used
  to build a "generic" source navigator using the getObjParent and
  getSourceObj methods to get parents and children of a given level (i.e.
  the parent of a module is an image, and the children will be the
  functions).

  .. cpp:enum:: BPatchErrorLevel
  .. cpp:enumerator:: BPatchErrorLevel::BPatchFatal
  .. cpp:enumerator:: BPatchErrorLevel::BPatchSerious
  .. cpp:enumerator:: BPatchErrorLevel::BPatchWarning
  .. cpp:enumerator:: BPatchErrorLevel::BPatchInfo

  .. cpp:enum:: BPatch_sourceType
  .. cpp:enumerator:: BPatch_sourceType::BPatch_sourceUnknown
  .. cpp:enumerator:: BPatch_sourceType::BPatch_sourceProgram
  .. cpp:enumerator:: BPatch_sourceType::BPatch_sourceModule
  .. cpp:enumerator:: BPatch_sourceType::BPatch_sourceFunction
  .. cpp:enumerator:: BPatch_sourceType::BPatch_sourceOuterLoop
  .. cpp:enumerator:: BPatch_sourceType::BPatch_sourceLoop
  .. cpp:enumerator:: BPatch_sourceType::BPatch_sourceStatement

  .. cpp:function:: BPatch_sourceType getSrcType()

    Returns the type of the current source object.

  .. cpp:function:: void getSourceObj(std::vector<BPatch_sourceObj *> &objs)

    Returns the child source objects of the current source object. For
    example, when called on a BPatch_sourceProgram object this will return
    objects of type BPatch_sourceFunction. When called on a
    BPatch_sourceFunction object it may return BPatch_sourceOuterLoop and
    BPatch_sourceStatement objects.

  .. cpp:function:: BPatch_sourceObj *getObjParent()

    Return the parent source object of the current source object. The parent
    of a BPatch_­image is NULL.

  .. cpp:enum:: BPatch_language
  .. cpp:enumerator:: BPatch_language::BPatch_c
  .. cpp:enumerator:: BPatch_language::BPatch_cPlusPlus
  .. cpp:enumerator:: BPatch_language::BPatch_fortran
  .. cpp:enumerator:: BPatch_language::BPatch_fortran77
  .. cpp:enumerator:: BPatch_language::BPatch_fortran90
  .. cpp:enumerator:: BPatch_language::BPatch_f90_demangled_stabstr
  .. cpp:enumerator:: BPatch_language::BPatch_fortran95
  .. cpp:enumerator:: BPatch_language::BPatch_assembly
  .. cpp:enumerator:: BPatch_language::BPatch_mixed
  .. cpp:enumerator:: BPatch_language::BPatch_hpf
  .. cpp:enumerator:: BPatch_language::BPatch_java
  .. cpp:enumerator:: BPatch_language::BPatch_unknownLanguage

  .. cpp:function:: BPatch_language getLanguage()

    Return the source language of the current BPatch_sourceObject. For
    programs that are written in more than one language, BPatch_mixed will
    be returned. If there is insufficient information to determine the
    language, BPatch_unknownLanguage will be returned.
