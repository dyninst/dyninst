.. _`sec:compiler_annotations.h`:

compiler_annotations.h
######################

Source-level annotations to provide the compiler with additional information.

Switch Statements
=================

.. c:macro:: DYNINST_FALLTHROUGH

Eliminates "this statement may fall through" warnings when used as the last
statment of a switch case that falls through to the next case.  The macro
is used as shown in indicate that case 2 should fall through to case 1.

.. code:: cpp

   switch (x)  {
       case 2:
           foo();
           DYNINST_FALLTHROUGH;
       case 1:
           foo();
           break;
       default:
           error();
           break;
   }

Deprecation
===========

.. c:macro:: DYNINST_DEPRECATED(msg)

Adds an annotation to a function, method, variable or type that it is
deprecated, and will produce a warning if it used.  The parameter msg
must be a quoted string.

The annotation should be placed before the definition or declaration.
For example:

.. code:: cpp

  DYNINST_DEPRECRATED("Use NewFoo") int Foo();


I/O
===

.. c:macro:: DYNINST_PRINTF_ANNOTATION(fmtIndex, argIndex)
.. c:macro:: DYNINST_SCANF_ANNOTATION(fmtIndex, argIndex)
.. c:macro:: DYNINST_FORMAT_ANNOTATION(fmtType, fmtIndex, argIndex)

Annotates a function as taking a printf for scanf format string and vararg
list of values allowing the compiler to validate the format string and the
supplied arguments.  fmtIndex is the 1-based index of the format string, and
argIndex is the 1-based index that begins the variable argument list used by
printf or scanf.

The annotation should be placed after the declaration of the function.  For
example:

.. code:: cpp

  myprintf(int level, char *fmt, ...) DYNINST_PRINTF_ANNOTATION(2, 3);

Memory Management
=================

.. c:macro:: DYNINST_MALLOC_ANNOTATION
.. c:macro:: DYNINST_MALLOC_DEALLOC_ANNOTATION(freeFunction)
.. c:macro:: DYNINST_MALLOC_DEALLOC_POS_ANNOTATION(freeFunction, pos)

Annotates a function as returning a unique pointer to suitable memory
to hold an object.  The second form names the matching deallocator, and
the third form indicates the position of the pointer in the argument list.

The annotation should be placed after the declaration of the function.  For
example:

.. code:: cpp
 
     Obj* myalloc(int x)  DYNINST_MALLOC_ANNOTATION;
     void mydealloc2(Obj* o);
     Obj* myalloc2(int x) DYNINST_MALLOC_DEALLOC_ANNOTATION(mydealloc2);
     void mydealloc3(int y, Obj* o);
     Obj* myalloc3(int x) DYNINST_MALLOC_DEALLOC_POS_ANNOTATION(mydealloc3, 2);
 