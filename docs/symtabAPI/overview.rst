.. _`sec:symtab-intro`:

SymtabAPI
#########

SymtabAPI is a multi-platform library for parsing symbol tables, object
file headers and debug information. SymtabAPI currently supports ELF
and PE formats. It also supports the DWARF debugging format.

The main goal of this API is to provide an abstract view of binaries and
libraries across multiple platforms. An abstract interface provides two
benefits: it simplifies the development of a tool since the complexity
of a particular file format is hidden, and it allows tools to be easily
ported between platforms. Each binary object file is represented in a
canonical platform independent manner by the API. The canonical format
consists of four components: a header block that contains general
information about the object (e.g., its name and location), a set of
symbol lists that index symbols within the object for fast lookup, debug
information (type, line number and local variable information) present
in the object file and a set of additional data that represents
information that may be present in the object (e.g., relocation or
exception information).

.. _`sec:symtab-definitions`:

Abstractions
************

.. cpp:namespace:: Dyninst::SymtabAPI

.. rubric:: Symbols

The symbol table interface is responsible for parsing the object file and handling the
lookup and addition of new symbols.

Exception Blocks
   :cpp:class:`Exception Blocks <ExceptionBlock>` contain the information necessary for run-time exception
   handling The following definitions deal with members of the Symbol
   class.

Function
  A :cpp:class:`function <Function>` is a code object within the file represented by
  one or more symbols.

Local Variable
  A local :cpp:class:`variable <localVar>` is a variable that has been declared within
  the scope of a sub-routine or a parameter to a sub-routine.

Module
  A :cpp:class:`module <Module>` is a particular source file in cases where multiple
  files were compiled into a single binary object; if this information
  is not present, or if the binary object is a shared library, we use a
  single default module.

Symtab
  A :cpp:class:`symtab <Symtab>` is either an object file on-disk or
  in-memory that the SymtabAPI library operates on.

Symbol
  A :cpp:class:`symbol <Symbol>` is an entry in the symbol table, and may identify a
  function, variable or other structure within the file.

Symbol Linkage
  The symbol :cpp:enum:`linkage <Symbol::SymbolLinkage>` for a symbol gives information on the visibility
  (binding) of this symbol, whether it is visible only in the object
  file where it is defined (local), if it is visible to all the object
  files that are being linked (global), or if its a weak alias to a
  global symbol.

Symbol Type
  Symbol :cpp:enum:`type <Symbol::SymbolType>` for a symbol is the category of symbols to which
  it belongs. It can be a function symbol or a variable symbol or a
  module symbol. The following definitions deal with the type and the
  local variable interface.

Variable
  A :cpp:class:`variable <Variable>` is a data object within the file containing
  one or more symbols.

......

.. rubric:: Symbol Names

Symbol names come in different "flavors" that are different representations
of the same information.

Mangled Name
  A mangled name for a symbol provides a way of encoding additional
  information about a function, structure, class or another data type
  in a symbol name. It is a technique used to produce unique names for
  programming entities in many modern programming languages. For
  example, the method ``foo`` of class ``C`` with signature ```int C::foo(int, int)``
  has a mangled name ``_ZN1C3fooEii`` when compiled with gcc.
  Mangled names may include a sequence of clone suffixes (begins with
  ‘.’ that indicate a compiler synthesized function), and this may be
  followed by a version suffix (begins with ‘@’).

Pretty Name
  A pretty name for a symbol is the demangled user-level symbolic name
  without type information for the function parameters and return
  types. For non-mangled names, the pretty name is the symbol name. Any
  function clone suffixes of the symbol are appended to the result of
  the demangler. For example, a symbol with a mangled name
  ``_ZN1C3fooEii`` for the method ``int C::foo(int, int)`` has a pretty
  name ``C::foo``. Version suffixes are removed from the mangled name
  before conversion to the pretty name. The pretty name can be obtained
  by running the command line tool ``c++filt`` as
  ``c++filt -i -p name``, or using the libiberty library function
  ``cplus_demangle`` with options of ``DMGL_AUTO | DMGL_ANSI``.

Typed Name
  A typed name for a symbol is the demangled user-level symbolic name
  including type information for the function parameters. Typically,
  but not always, function return type information is not included. Any
  function clone information is also included. For non-mangled names,
  the typed name is the symbol name. For example, a symbol with a
  mangled name ``_ZN1C3fooEii`` for the method ``int C::foo(int, int)``
  has a typed name ``C::foo(int, int)``. Version suffixes are removed
  from the mangled name before conversion to the typed name. The typed
  name can be obtained by running the command line tool ``c++filt`` as
  ``c++filt -i name``, or using the libiberty library function
  ``cplus_demangle`` with options of
  ``DMGL_AUTO | DMGL_ANSI | DMGL_PARAMS``.

......

.. _`topic:symtabapi-object-files`:
.. rubric:: Object Files

Object files are the basic data structure for Symtab. They represent either an on-disk file
or an in-memory representation. In both cases, they are treated as a structured sequence of
bytes dictate by standards like ELF or PE. Symtab attempts to abstract away the details of
these formats to provide a uniform interface for interacting with any object file. Examples
of object files are ``.so`` files on Linux or ``.exe`` files on Windows.

Archive
   An :cpp:class:`archive <Archive>` is a collection of binary objects stored in a
   single file (e.g., a static archive).

Object File
   An :cpp:class:`object <Object>` file is the representation of code that a compiler or
   assembler generates by processing a source code file. It represents
   .o’s, a.out’s and shared libraries.

Region
   A :cpp:class:`region <Region>` is a contiguous area of the file that contains
   executable code or readable data; for example, an ELF section.

......

.. rubric:: Type Information

The type interface is responsible for parsing type information from the
object file and handling the look-up and addition of new type information.
All type information is derived from debugging information present in the
:ref:`object file <topic:symtabapi-object-files>`.

Type
   A :cpp:class:`type <Type>` is the data type of a variable or a parameter. This
   can represent language pre-defined types (e.g. int, float),
   pre-defined types in the object (e.g., structures or unions), or
   user-defined types.

Array
  An :cpp:class:`array <typeArray>` sequence of values contiguous in memory

Common
  A :cpp:class:`common <typeCommon>` block type in Fortran

CBlock
  An :cpp:class:`element <CBlock>` of a common block in Fortran

Derived
  A :cpp:class:`reference <derivedType>` to another type. Examples are pointers, references, and typedefs.

Enum
  A :cpp:class:`collection <typeEnum>` of named constants with values.

Function
  A :cpp:class:`block <typeFunction>` of executable code with a return type and an optional list of parameters

Field List
  A :cpp:class:`container <fieldListType>` type like C++ ``struct``, ``union``, and ``class``.

Field
  A data :cpp:class:`member <Field>` of a struct or union type.

Pointer
  A :cpp:class:`pointer <typePointer>`.

Ranged
  A :cpp:class:`range <rangedType>` with a lower and upper bound.

Reference
  A C++ :cpp:class:`reference <typeRef>`.

Scalar
  :cpp:class:`Integral <typeScalar>` and floating-point types.

Struct
  An algebraic :cpp:class:`product <typeStruct>` type like a C ``struct``.

Subrange
  A :cpp:class:`subsequence <typeSubrange>` of a range. This could be a subrange of an array.

Typedef
  An type :cpp:class:`alias <typeTypedef>` like a C ``typedef`` or C++ ``using`` alias.

Union
  An algebraic :cpp:class:`sum <typeUnion>` type like a C ``union``.

......

.. rubric:: Line Numbers

The line number interface is responsible for parsing line number
information from the object file debug information and handling the
look-up and addition of new line information.

LineInformation
   A collection of statements mapping source :cpp:class:`lines <LineInformation>`
   to a range of addresses in the object file.

Statement
   A tuple for a single :cpp:class:`location <Statement>` in a source code file containing
   a source file, line number in that source file and start column in
   that line. For backwards compatibility, Statements may also be
   referred to as LineNoTuples.

......

.. rubric:: Dynamic Address Translation

A mapping between absolute addresses found in a running process and symtabs. Object
files can load at different addresses in different processes. Each AddressLookup instance
is associated with, and provides mapping for, one process.

Offset
  An :cpp:type:`offset <Offset>` is an address relative to the start address(base) of
  the object file. For executables, the Offset is an absolute address.

Relocations
   Provides a description for inter-object references between two object files.


.. _`sec:symtabapi-defensive`:

Defensive binaries
******************

Code reuse attacks are an increasingly popular technique for circumventing tra-
ditional program protection mechanisms such as W ``xor`` X (e.g., Data Execution
Prevention (DEP)), and the security community has proposed a wide range of
approaches to protect against these attacks. However, many of these approaches
provide ad hoc solutions, relying on observed attack characteristics that are not
intrinsic to the class of attacks. In the continuing arms race against code reuse
attacks, we must construct defenses using a more systematic approach: good
engineering practices must combine with the best security techniques.
Any such approach must be engineered to cover the complete spectrum of
attack surfaces. While more general defensive techniques, such as Control Flow
2 Detecting Code Reuse Attacks
Integrity or host-based intrusion detection, provide good technical solutions,
each is lacking in one or more features necessary to provide a comprehensive
and adoptable solution. We must develop defenses that can be effectively
applied to real programs.

See `Jacboson et al. 2014 <https://paradyn.org/papers/Jacobson14ROPStop.pdf>`_ for details.

.. _symtabapi-usage:

Usage
*****

:cpp:func:`Symtab::openFile` is the canonical entry for parsing
on-disk files or in-memory images without user interaction. If more control
of the parsing process is needed, see :ref:`ParseAPI <sec:parseapi-intro>`.

Symbols can be looked up using a specific type and name as well as by regular expression.

.. rli:: https://raw.githubusercontent.com/dyninst/examples/master/symtabAPI/printSymbols.cpp
  :language: cpp
  :linenos:


Line number information for source files used to create a binary can be queried.

.. rli:: https://raw.githubusercontent.com/dyninst/examples/master/symtabAPI/printLineInfo.cpp
  :language: cpp
  :linenos:


Local variables from within the entire object file or just within a function can be looked up.

.. rli:: https://raw.githubusercontent.com/dyninst/examples/master/symtabAPI/printLocalVars.cpp
  :language: cpp
  :linenos:

