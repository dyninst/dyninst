.. _`sec:symtab-intro`:

=========
SymtabAPI
=========

SymtabAPI is a multi-platform library for parsing symbol tables, object
file headers and debug information. SymtabAPI currently supports the ELF
(IA-32, AMD-64, ARMv8-64, and POWER) and PE (Windows) object file
formats. In addition, it also supports the DWARF debugging format.

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
exception information). Adding a new format requires no changes to the
interface and hence will not affect any of the tools that use the
SymtabAPI.

Our other design goal with SymtabAPI is to allow users and tool
developers to easily extend or add symbol or debug information to the
library through a platform-independent interface. Often times it is
impossible to satify all the requirements of a tool that uses SymtabAPI,
as those requirements can vary from tool to tool. So by providing
extensible structures, SymtabAPI allows tools to modify any structure to
fit their own requirements. Also, tools frequently use more
sophisticated analyses to augment the information available from the
binary directly; it should be possible to make this extra information
available to the SymtabAPI library. An example of this is a tool
operating on a stripped binary. Although the symbols for the majority of
functions in the binary may be missing, many can be determined via more
sophisticated analysis. In our model, the tool would then inform the
SymtabAPI library of the presence of these functions; this information
would be incorporated and available for subsequent analysis. Other
examples of such extensions might involve creating and adding new types
or adding new local variables to certain functions.

.. _`sec:symtab-definitions`:

Definitions
===========

.. cpp:namespace:: Dyninst::SymtabAPI

:cpp:class:`Archive`
   An archive represents a collection of binary objects stored in a
   single file (e.g., a static archive).

:cpp:class:`Exception Blocks <ExceptionBlock>`
   These contain the information necessary for run-time exception
   handling The following definitions deal with members of the Symbol
   class.

Function
   A function represents a code object within the file represented by
   one or more symbols.

Local Variable
   A local variable represents a variable that has been declared within
   the scope of a sub-routine or a parameter to a sub-routine.

Mangled Name
   A mangled name for a symbol provides a way of encoding additional
   information about a function, structure, class or another data type
   in a symbol name. It is a technique used to produce unique names for
   programming entities in many modern programming languages. For
   example, the method *foo* of class C with signature *int C::foo(int,
   int)* has a mangled name *\_ZN1C3fooEii* when compiled with gcc.
   Mangled names may include a sequence of clone suffixes (begins with
   ‘.’ that indicate a compiler synthesized function), and this may be
   followed by a version suffix (begins with ‘@’).

Module
   A module represents a particular source file in cases where multiple
   files were compiled into a single binary object; if this information
   is not present, or if the binary object is a shared library, we use a
   single default module.

Offset
   Offsets represent an address relative to the start address(base) of
   the object file. For executables, the Offset represents an absolute
   address. The following definitions deal with the symbol table
   interface.

Object File
   An object file is the representation of code that a compiler or
   assembler generates by processing a source code file. It represents
   .o’s, a.out’s and shared libraries.

Pretty Name
   A pretty name for a symbol is the demangled user-level symbolic name
   without type information for the function parameters and return
   types. For non-mangled names, the pretty name is the symbol name. Any
   function clone suffixes of the symbol are appended to the result of
   the demangler. For example, a symbol with a mangled name
   *\_ZN1C3fooEii* for the method *int C::foo(int, int)* has a pretty
   name *C::foo*. Version suffixes are removed from the mangled name
   before conversion to the pretty name. The pretty name can be obtained
   by running the command line tool ``c++filt`` as
   ``c++filt -i -p name``, or using the libiberty library function
   ``cplus_demangle`` with options of ``DMGL_AUTO | DMGL_ANSI``.

Region
   A region represents a contiguous area of the file that contains
   executable code or readable data; for example, an ELF section.

Relocations
   These provide the necessary information for inter-object references
   between two object files.

Symbol
   A symbol represents an entry in the symbol table, and may identify a
   function, variable or other structure within the file.

Symbol Linkage
   The symbol linkage for a symbol gives information on the visibility
   (binding) of this symbol, whether it is visible only in the object
   file where it is defined (local), if it is visible to all the object
   files that are being linked (global), or if its a weak alias to a
   global symbol.

Symbol Type
   Symbol type for a symbol represents the category of symbols to which
   it belongs. It can be a function symbol or a variable symbol or a
   module symbol. The following definitions deal with the type and the
   local variable interface.

Type
   A type represents the data type of a variable or a parameter. This
   can represent language pre-defined types (e.g. int, float),
   pre-defined types in the object (e.g., structures or unions), or
   user-defined types.

Typed Name
   A typed name for a symbol is the demangled user-level symbolic name
   including type information for the function parameters. Typically,
   but not always, function return type information is not included. Any
   function clone information is also included. For non-mangled names,
   the typed name is the symbol name. For example, a symbol with a
   mangled name *\_ZN1C3fooEii* for the method *int C::foo(int, int)*
   has a typed name *C::foo(int, int)*. Version suffixes are removed
   from the mangled name before conversion to the typed name. The typed
   name can be obtained by running the command line tool ``c++filt`` as
   ``c++filt -i name``, or using the libiberty library function
   ``cplus_demangle`` with options of
   ``DMGL_AUTO | DMGL_ANSI | DMGL_PARAMS``.

Variable
   A variable represents a data object within the file represented by
   one or more symbols.

.. _`sec:symtab-abstractions`:

Abstractions
============

SymtabAPI provides a simple set of abstractions over complicated data
structures which makes it straight-forward to use. The SymtabAPI
consists of five classes of interfaces: the symbol table interface, the
type interface, the line map interface, the local variable interface,
and the address translation interface.

.. _symtab-object-ownership:

.. figure:: public/figures/type-hierarchy.png
  :width: 100%

  Type heirarchy in SymtabAPI.

Symbol Table Interface
----------------------

The symbol table interface is responsible for parsing the object file
and handling the look-up and addition of new symbols. It is also
responsible for the emit functionality that SymtabAPI supports. The
Symtab and the Module classes inherit from the LookupInterface class, an
abstract class, ensuring the same lookup function signatures for both
Module and Symtab classes.

Symtab
   A Symtab class object represents either an object file on-disk or
   in-memory that the SymtabAPI library operates on.

Symbol
   A Symbol class object represents an entry in the symbol table.

Module
   A Module class object represents a particular source file in cases
   where multiple files were compiled into a single binary object; if
   this information is not present, we use a single default module.

Archive
   An Archive class object represents a collection of binary objects
   stored in a single file (e.g., a static archive).

ExceptionBlock
   An ExceptionBlock class object represents an exception block which
   contains the information necessary for run-time exception handling.

In addition, we define two symbol aggregates, Function and Variable.
These classes collect multiple symbols with the same address and type
but different names; for example, weak and strong symbols for a single
function.

.. _`subsec:typeInterface`:

Type Interface
--------------

The Type interface is responsible for parsing type information from the
object file and handling the look-up and addition of new type
information. Figure `[fig:class-inherit] <#fig:class-inherit>`__ shows
the class inheritance diagram for the type interface. Class Type is the
base class for all of the classes that are part of the interface. This
class provides the basic common functionality for all the types, such as
querying the name and size of a type. The rest of the classes represent
specific types and provide more functionality based on the type.

Some of the types inherit from a second level of type classes, each
representing a separate category of types.

fieldListType
   - This category of types represent the container types that contain a list of fields. Examples of this category include structure and the union types.

derivedType
   - This category of types represent types derived from a base type. Examples of this category include typedef, pointer and reference types.

rangedType
   - This category represents range types. Examples of this category include the array and the sub-range types.

The enum, function, common block and scalar types do not fall under any
of the above category of types. Each of the specific types is derived
from Type.

Line Number Interface
---------------------

The Line Number interface is responsible for parsing line number
information from the object file debug information and handling the
look-up and addition of new line information. The main classes for this
interface are LineInformation and LineNoTuple.

LineInformation
   A LineInformation class object represents a mapping of line numbers
   to address range within a module (source file).

Statement/LineNoTuple
   A Statement class object represents a location in source code with
   a source file, line number in that source file and start column in
   that line. For backwards compatibility, Statements may also be
   referred to as LineNoTuples.

Local Variable Interface
------------------------

The Local Variable Interface is responsible for parsing local variable
and parameter information of functions from the object file debug
information and handling the look-up and addition of new add new local
variables. All the local variables within a function are tied to the
Symbol class object representing that function.

localVar
   A localVar class object represents a local variable or a parameter
   belonging to a function.

Dynamic Address Translation
---------------------------

The AddressLookup class is a component for mapping between absolute
addresses found in a running process and SymtabAPI objects. This is
useful because libraries can load at different addresses in different
processes. Each AddressLookup instance is associated with, and provides
mapping for, one process.


.. _`sec:symtabapi-defensive`:

Defensive binaries
==================

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
=====

To illustrate the ideas in the API, this section presents several short
examples that demonstrate how the API can be used. SymtabAPI has the
ability to parse files that are on-disk or present in memory. The user
program starts by requesting SymtabAPI to parse an object file.
SymtabAPI returns a handle if the parsing succeeds, whcih can be used
for further interactions with the SymtabAPI library. The following
example shows how to parse a shared object file on disk.

.. code-block:: cpp

   using namespace Dyninst;
   using namespace SymtabAPI;

   //Name the object file to be parsed:
   std::string file = "libfoo.so";

   //Declare a pointer to an object of type Symtab; this represents the file.
   Symtab *obj = NULL;

   // Parse the object file
   bool err = Symtab::openFile(obj, file);

Once the object file is parsed successfully and the handle is obtained,
symbol look up and update operations can be performed in the following
way:

.. code-block:: cpp

   using namespace Dyninst;
   using namespace SymtabAPI;
   std::vector <Symbol *> syms;
   std::vector <Function *> funcs;

   // search for a function with demangled (pretty) name "bar".
   if (obj->findFunctionsByName(funcs, "bar")) {
          // Add a new (mangled) primary name to the first function
          funcs[0]->addMangledName("newname", true);
   }

   // search for symbol of any type with demangled (pretty) name "bar".
   if (obj->findSymbol(syms, "bar", Symbol::ST_UNKNOWN)) { 

       // change the type of the found symbol to type variable(ST_OBJECT)
       syms[0]->setType(Symbol::ST_OBJECT);

       // These changes are automatically added to symtabAPI; no further
       // actions are required by the user.
   }

New symbols, functions, and variables can be created and added to the
library at any point using the handle returned by successful parsing of
the object file. When possible, add a function or variable rather than a
symbol directly.

.. code-block:: cpp

   using namespace Dyninst;
   using namespace SymtabAPI;

   //Module for the symbol
   Module *mod;

   // obj represents a handle to a parsed object file.
   obj->findModuleByName(mod, "/path/to/foo.c");

   // Create a new function symbol
   Variable *newVar = mod->createVariable("newIntVar",  // Name of new variable
                                          0x12345,      // Offset from data section
                                          sizeof(int)); // Size of symbol 

SymtabAPI gives the ability to query type information present in the
object file. Also, new user defined types can be added to SymtabAPI. The
following example shows both how to query type information after an
object file is successfully parsed and also add a new structure type.

.. code-block:: cpp

   // create a new struct Type
   // typedef struct{
   //int field1,
   //int field2[10]
   // } struct1;

   using namespace Dyninst;
   using namespace SymtabAPI;

   // Find a handle to the integer type; obj represents a handle to a parsed object file
   Type *lookupType;
   obj->findType(lookupType, "int");

   // Convert the generic type object to the specific scalar type object
   typeScalar *intType = lookupType->getScalarType();

   // container to hold names and types of the new structure type
   vector<pair<string, Type *> >fields;

   //create a new array type(int type2[10])
   typeArray *intArray = typeArray::create("intArray",intType,0,9, obj);

   //types of the structure fields
   fields.push_back(pair<string, Type *>("field1", intType));
   fields.push_back(pair<string, Type *>("field2", intArray));

   //create the structure type
   typeStruct *struct1 = typeStruct::create("struct1", fields, obj);

Users can also query line number information present in an object file.
The following example shows how to use SymtabAPI to get the address
range for a line number within a source file.

.. code-block:: cpp

   using namespace Dyninst;
   using namespace SymtabAPI;

   // obj represents a handle to a parsed object file using symtabAPI
   // Container to hold the address range
   vector< pair< Offset, Offset > > ranges;

   // Get the address range for the line 30 in source file foo.c
   obj->getAddressRanges(ranges, "foo.c", 30);

Local variable information can be obtained using symtabAPI. You can
query for a local variable within the entire object file or just within
a function. The following example shows how to find local variable foo
within function bar.

.. code-block:: cpp

   using namespace Dyninst;
   using namespace SymtabAPI;

   // Obj represents a handle to a parsed object file using symtabAPI
   // Get the Symbol object representing function bar
   vector<Symbol *> syms;
   obj->findSymbol(syms, "bar", Symbol::ST_FUNCTION);

   // Find the local var foo within function bar
   vector<localVar *> *vars = syms[0]->findLocalVarible("foo");

The ``LineInformation`` class also provides the ability for iterating
over its data (line numbers and their corresponding address ranges). The
following example shows how to iterate over the line information for a
given module using SymtabAPI.

.. code-block:: cpp

   //Example showing how to iterate over the line information for a given module.
   using namespace Dyninst;
   using namespace SymtabAPI;

   //Obj represents a handle to a parsed object file using symtabAPI
   //Module handle for the module
   Module *mod;

   //Find the module \lq foo\rq within the object.
   obj->findModuleByName(mod, "foo");

   // Get the Line Information for module foo.
   LineInformation *info = mod->getLineInformation();

   //Iterate over the line information
   LineInformation::const_iterator iter;
   for( iter = info->begin(); iter != info->end(); iter++)
   {
   // First component represents the address range for the line
   const std::pair<Offset, Offset> addrRange = iter->first;

   //Second component gives information about the line itself.
   LineNoTuple lt = iter->second;
   }
