.. _`sec:compiler_diagnostics.h`:

compiler_diagnostics.h
######################

Macros to suppress compiler diagnostics for a region of code.

They are used to suppress diagnostic that are due to

  1) non-standard code
  2) the compiler produced false positives.

  They expand to nothing if not applicable with the current compiler.

The macros to begin and end the region take the form:

.. code:: cpp

     DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_<code>
     DYNINST_DIAGNOSTIC_END_SUPPRESS_<code>

They should be place on a lines of their own without trailing '()' or ';'.

Currently defined value for ``<code>`` are

.. c:macro:: FLEX_ARRAY

  C flexible arrays in C++

.. c:macro:: VLA

  C VLAs (variable length arrays) in C++

.. c:macro:: VLA_EXTENSION

  Clang warning about C VLAs in C++, if VLA is suppressed

.. c:macro:: VLA_ALL

  Enable both :c:macro:`VLA` and :c:macro:`VLA_EXTENSION`

.. c:macro:: VLA_GCC_PRAGMA_BUG

  gcc <9, 11.0, and 11.1 workaround

.. c:macro:: LOGICAL_OP

  warning about duplicate subexpressions in a logical expression
  Is a false positive due compiler checks after macro/constant
  propagation (eg. (x == a && x == b) if a and b are distinct
  constants with the same physical value. Only gcc 6-8.

.. c:macro:: DUPLICATED_BRANCHES

  similar to LOGICAL_OP except the expressions are the
  conditionals of a chain of if/then/else's. Only gcc 7-8.

.. c:macro:: UNUSED_VARIABLE

  clang <10 warns about variables defined solely for RIAA (locks)

.. c:macro:: MAYBE_UNINITIALIZED

  gcc 12 warns that boost::optional::value_or may use an
  unitialized value when value_or checks if it is initialized.

Silence Warnings
================

.. c:macro:: DYNINST_SUPPRESS_UNUSED_VARIABLE(var)

  Indicates that variable ``var`` is OK to be unused.

.. c:macro:: DYNINST_DIAGNOSTIC_NO_SUPPRESSIONS

  Define to prevent suppressions.