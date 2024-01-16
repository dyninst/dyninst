symutil.h
=========

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:function:: const char *supportedLanguages2Str(supportedLanguages s)

.. cpp:enum:: NameType

  .. cpp:enumerator:: mangledName = 1
  .. cpp:enumerator:: prettyName = 2
  .. cpp:enumerator:: typedName = 4
  .. cpp:enumerator:: anyName = 7


.. cpp:enum:: supportedLanguages 

  .. cpp:enumerator:: lang_Unknown

    Unknown source language

  .. cpp:enumerator:: lang_Assembly

    Raw assembly code

  .. cpp:enumerator:: lang_C

    C source code

  .. cpp:enumerator:: lang_CPlusPlus

    C++ source code

  .. cpp:enumerator:: lang_GnuCPlusPlus

    C++ with GNU extensions

  .. cpp:enumerator:: lang_Fortran

    Fortran source code

  .. cpp:enumerator:: lang_Fortran_with_pretty_debug

    Fortran with debug annotations

  .. cpp:enumerator:: lang_CMFortran

    Fortran with CM extensions


.. cpp:enum:: ObjectType

  .. cpp:enumerator:: obj_Unknown
  .. cpp:enumerator:: obj_SharedLib
  .. cpp:enumerator:: obj_Executable
  .. cpp:enumerator:: obj_RelocatableFile


.. cpp:enum:: SymtabError

  .. cpp:enumerator:: Obj_Parsing = 0
  .. cpp:enumerator:: Syms_To_Functions
  .. cpp:enumerator:: Build_Function_Lists
  .. cpp:enumerator:: No_Such_Function
  .. cpp:enumerator:: No_Such_Variable
  .. cpp:enumerator:: No_Such_Module
  .. cpp:enumerator:: No_Such_Region
  .. cpp:enumerator:: No_Such_Symbol
  .. cpp:enumerator:: No_Such_Member
  .. cpp:enumerator:: Not_A_File
  .. cpp:enumerator:: Not_An_Archive
  .. cpp:enumerator:: Duplicate_Symbol
  .. cpp:enumerator:: Export_Error
  .. cpp:enumerator:: Emit_Error
  .. cpp:enumerator:: Invalid_Flags
  .. cpp:enumerator:: Bad_Frame_Data
  .. cpp:enumerator:: No_Frame_Entry
  .. cpp:enumerator:: Frame_Read_Error
  .. cpp:enumerator:: Multiple_Region_Matches
  .. cpp:enumerator:: No_Error

.. cpp:struct:: Segment

  .. cpp:member:: void *data
  .. cpp:member:: Offset loadaddr
  .. cpp:member:: unsigned long size
  .. cpp:member:: std::string name
  .. cpp:member:: unsigned segFlags
