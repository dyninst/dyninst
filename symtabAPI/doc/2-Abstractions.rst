.. _sec:abstractions:

Abstractions
============

= [rectangle, draw, fill=green!25] = [rectangle, draw, rounded corners,
fill=yellow!25] = [ellipse, draw, fill=red!25] = [trapezium, trapezium
left angle=70, trapezium right angle=70, draw, fill=cyan!25] = []

SymtabAPI provides a simple set of abstractions over complicated data
structures which makes it straight-forward to use. The SymtabAPI
consists of five classes of interfaces: the symbol table interface, the
type interface, the line map interface, the local variable interface,
and the address translation interface.

Figure \ `[fig:object-ownership] <#fig:object-ownership>`__ shows the
ownership hierarchy for the SymtabAPI classes. Ownership here is a
“contains” relationship; if one class owns another, then instances of
the owner class maintain an exclusive instance of the other. For
example, each Symtab class instance contains multiple instances of class
Symbol and each Symbol class instance belongs to one Symtab class
instance. Each of four interfaces and the classes belonging to these
interfaces are described in the rest of this section. The API functions
in each of the classes are described in detail in Section
`[sec:symtabAPI] <#sec:symtabAPI>`__.

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

.. _subsec:typeInterface:

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

= [rectangle, draw, rounded corners, fill=yellow!100] = [rectangle,
draw, rounded corners, fill=yellow!100, node distance=.65cm] =
[rectangle, draw, rounded corners, pattern=north west lines, pattern
color=yellow] = []

Some of the types inherit from a second level of type classes, each
representing a separate category of types.

fieldListType
   - This category of types represent the container types that contain a
   list of fields. Examples of this category include structure and the
   union types.

derivedType
   - This category of types represent types derived from a base type.
   Examples of this category include typedef, pointer and reference
   types.

rangedType
   - This category represents range types. Examples of this category
   include the array and the sub-range types.

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
   - A LineInformation class object represents a mapping of line numbers
   to address range within a module (source file).

Statement/LineNoTuple
   - A Statement class object represents a location in source code with
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
   - A localVar class object represents a local variable or a parameter
   belonging to a function.

Dynamic Address Translation
---------------------------

The AddressLookup class is a component for mapping between absolute
addresses found in a running process and SymtabAPI objects. This is
useful because libraries can load at different addresses in different
processes. Each AddressLookup instance is associated with, and provides
mapping for, one process.
