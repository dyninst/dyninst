Symtab.h
========

.. cpp:namespace:: Dyninst::SymtabAPI

Class Symtab
------------

The ``Symtab`` class represents an object file either on-disk or
in-memory. This class is responsible for the parsing of the ``Object``
file information and holding the data that can be accessed through look
up functions.

.. list-table:: The Symtab Class
   :widths: 30  35 35
   :header-rows: 1

   * - Method name
     - Return type
     - Method description
   * - ``file``
     - std::string
     - Full path to the opened file or provided name for the memory image.
   * - ``name``
     - std::string
     - File name without path.
   * - ``memberName``
     - std::string
     - For archive (.a) files, returns the object file (.o) this Symtab represents.
   * - ``getNumberOfRegions``
     - unsigned
     - Number of regions.
   * - ``getNumberOfSymbols``
     - unsigned
     - Total number of symbols in both the static and dynamic tables.
   * - ``mem_image``
     - char *
     - Pointer to memory image for the Symtab; not valid for disk files.
   * - ``imageOffset``
     - Offset
     - Offset at the first code segment from the start of the binary.
   * - ``dataOffset``
     - Offset
     - Offset at the first data segment from the start of the binary.
   * - ``imageLength``
     - Offset
     - Size of the primary code-containing region, typically .text.
   * - ``dataLength``
     - Offset
     - Size of the primary data-containing region, typically .data.
   * - ``isStaticBinary``
     - bool
     - True if the binary was compiled statically.
   * - ``isExecutable``
     - bool
     - True if the file is an executable.
   * - ``isSharedLibrary``
     - bool
     - True if the file is a shared library.
   * - ``isExec``
     - bool
     - True if the file can only be an executable, false otherwise including both executables and shared libraries. Typically files that are bot executables and shared libraries are primarily used as libraries, if you need to determine specifics use the methods ``isExecutable`` and ``isSharedLibrary``.
   * - ``isStripped``
     - bool
     - True if the file was stripped of symbol table information.
   * - ``getAddressWidth``
     - unsigned
     - Size (in bytes) of a pointer value in the Symtab; 4 for 32-bit binaries and 8 for 64-bit binaries.
   * - ``getArchitecture``
     - Architecture
     - Representation of the system architecture for the binary.
   * - ``getLoadOffset``
     - Offset
     - The suggested load offset of the file; typically 0 for shared libraries.
   * - ``getEntryOffset``
     - Offset
     - The entry point (where execution beings) of the binary.
   * - ``getBaseOffset``
     - Offset
     - (Windows only) the OS-specified base offset of the file.

.. code-block:: cpp

    ObjectType getObjectType() const

This method queries information on the type of the object file.

.. code-block:: cpp
 
    bool isExecutable() bool isSharedLibrary() bool isExec()

These methods respectively return true if the Symtab’s object is an
executable, a shared library, and an executable is that is not a shared
library. An object may be both an executable and a shared library.

An Elf Object that can be loaded into memory to form an executable’s
image has one of two types: ET_EXEC and ET_DYN. ET_EXEC type objects are
executables that are loaded at a fixed address determined at link time.
ET_DYN type objects historically were shared libraries that are loaded
at an arbitrary location in memory and are position independent code
(PIC). The ET_DYN object type was reused for position independent
executables (PIE) that allows the executable to be loaded at an
arbitrary location in memory. Although generally not the case an object
can be both a PIE executable and a shared library. Examples of these
include libc.so and the dynamic linker library (ld.so). These objects
are generally used as a shared library so ``isExec()`` will classify
these based on their typical usage. The methods below use heuristics to
classify ET_DYN object types correctly based on the properties of the
Elf Object, and will correctly classify most objects. Due to the
inherent ambiguity of ET_DYN object types, the heuristics may fail to
classify some libraries that are also executables as an executable. This
can happen in object is a shared library and an executable, and its
entry point happens to be at the start of the .text section.

``isExecutable()`` is equivalent to elfutils’ ``elfclassify --program``
test with the refinement of the soname value and entry point tests.
Pseudocode for the algorithm is shown below:

-  **if** (**not** loadable()) **return** *false*

-  **if** (object type is ET_EXEC) **return** *true*

-  **if** (has an interpreter (PT_INTERP segment exists)) **return**
   *true*

-  **if** (PIE flag is set in FLAGS_1 of the PT_DYNAMIC segment)
   **return** *true*

-  **if** (DT_DEBUG tag exists in PT_DYNAMIC segment) **return** *true*

-  **if** (has a soname and its value is “linux-gate.so.1”) **return**
   *false*

-  **if** (entry point is in range .text section offset plus 1 to the
   end of the .text section) **return** *true*

-  **if** (has a soname and its value starts with “ld-linux”) **return**
   *true*

-  **otherwise return** *false*

``isSharedLibrary()`` is equivalent to elfutils’
``elfclassify --library``. Pseudocode for the algorithm is shown below:

-  **if** (**not** loadable()) **return** *false*

-  **if** (object type is ET_EXEC) **return** *false*

-  **if** (there is no PT_DYNAMIC segment) **return** *false*

-  **if** (PIE flag is set in FLAGS_1 of the PT_DYNAMIC segment)
   **return** *false*

-  **if** (DT_DEBUG tag exists in PT_DYNAMIC segment) **return** *false*

-  **otherwise return** *true*

Elf files can also store data that is neither an executable nor a shared
library including object files, core files and debug symbol files. To
distinguish these cases the ``loadable()`` function is defined using the
pseudocode shown below and returns true is the file can loaded into a
process’s address space:

-  **if** (object type is neither ET_EXEC nor ET_DYN) **return** *false*

-  **if** (there is are no program segments with the PT_LOAD flag set)
   **return** *false*

-  **if** (contains no sections) **return** *true*

-  **if** (contains a section with the SHF_ALLOC flag set and a section
   type of neither SHT_NOTE nor SHT_NOBITS) **return** *true*

-  **otherwise return** *false*

File opening/parsing
~~~~~~~~~~~~~~~~~~~~

.. code-block:: cpp
    
    static bool openFile(Symtab *&obj, string filename)

Creates a new ``Symtab`` object for an object file on disk. This object
serves as a handle to the parsed object file. ``filename`` represents
the name of the ``Object`` file to be parsed. The ``Symtab`` object is
returned in ``obj`` if the parsing succeeds. Returns ``true`` if the
file is parsed without an error, else returns ``false``.
``getLastSymtabError()`` and ``printError()`` should be called to get
more error details.

.. code-block:: cpp

    static bool openFile(Symtab *&obj, char *mem_image, size_t size, std::string name)

This factory method creates a new ``Symtab`` object for an object file
in memory. This object serves as a handle to the parsed object file.
``mem_image`` represents the pointer to the ``Object`` file in memory to
be parsed. ``size`` indicates the size of the image. ``name`` specifies
the name we will give to the parsed object. The ``Symtab`` object is
returned in ``obj`` if the parsing succeeds. Returns ``true`` if the
file is parsed without an error, else returns ``false``.
``getLastSymtabError()`` and ``printError()`` should be called to get
more error details.

.. code-block:: cpp

    static Symtab *findOpenSymtab(string name)

Find a previously opened ``Symtab`` that matches the provided name.

Module lookup
~~~~~~~~~~~~~

.. code-block:: cpp

    Module *getDefaultModule()

Returns the default module, a collection of all functions, variables,
and symbols that do not have an explicit module specified.

.. code-block:: cpp

    bool findModuleByName(Module *&ret, const string name)

This method searches for a module with name ``name``. If the module
exists returns ``true`` with ``ret`` set to the module handle; otherwise
returns ``false`` with ``ret`` set to ``NULL``.

.. code-block:: cpp

    bool findModuleByOffset(Module *&ret, Offset offset)

This method searches for a module that starts at offset ``offset``. If
the module exists returns ``true`` with ``ret`` set to the module
handle; otherwise returns ``false`` with ``ret`` set to ``NULL``.

.. code-block:: cpp

    bool getAllModules(vector<module *> &ret)

This method returns all modules in the object file. Returns ``true`` on
success and ``false`` if there are no modules. The error value is set to
``No_Such_Module``.

Function, Variable, and Symbol lookup
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: cpp

    bool findFuncByEntryOffset(Function *&ret, const Offset offset)

This method returns the ``Function`` object that begins at ``offset``.
Returns ``true`` on success and ``false`` if there is no matching
function. The error value is set to ``No_Such_Function``.

.. code-block:: cpp

    bool findFunctionsByName(std::vector<Function *> &ret, const std::string name, NameType nameType = anyName, bool isRegex = false, bool checkCase = true)

This method finds and returns a vector of ``Function``\ s whose names
match the given pattern. The ``nameType`` parameter determines which
names are searched: mangled, pretty, typed, or any. If the ``isRegex``
flag is set a regular expression match is performed with the symbol
names. ``checkCase`` is applicable only if ``isRegex`` has been set.
This indicates if the case be considered while performing regular
expression matching. ``ret`` contains the list of matching Functions, if
any. Returns ``true`` if it finds functions that match the given name,
otherwise returns ``false``. The error value is set to
``No_Such_Function``.

.. code-block:: cpp

    bool getContainingFunction(Offset offset, Function *&ret)

This method returns the function, if any, that contains the provided
``offset``. Returns ``true`` on success and ``false`` on failure. The
error value is set to ``No_Such_Function``. Note that this method does
not parse, and therefore relies on the symbol table for information. As
a result it may return incorrect information if the symbol table is
wrong or if functions are either non-contiguous or overlapping. For more
precision, use the ParseAPI library.

.. code-block:: cpp

    bool getAllFunctions(vector<Function *> &ret)

This method returns all functions in the object file. Returns ``true``
on success and ``false`` if there are no modules. The error value is set
to ``No_Such_Function``.

.. code-block:: cpp

     bool findVariablesByOffset(std::vector<Variable *> &ret, const Offset offset)

This method returns a vector of ``Variable``\ s with the specified
offset. There may be more than one variable at an offset if they have
different sizes. Returns ``true`` on success and ``false`` if there is
no matching variable. The error value is set to ``No_Such_Variable``.

.. code-block:: cpp

   bool findVariablesByName(std::vector<Variable *> &ret, const std::string name, NameType nameType = anyName, bool isRegex = false, bool checkCase = true)

This method finds and returns a vector of ``Variable``\ s whose names
match the given pattern. The ``nameType`` parameter determines which
names are searched: mangled, pretty, typed, or any (note: a ``Variable``
may not have a typed name). If the ``isRegex`` flag is set a regular
expression match is performed with the symbol names. ``checkCase`` is
applicable only if ``isRegex`` has been set. This indicates if the case
be considered while performing regular expression matching. ``ret``
contains the list of matching ``Variable``\ s, if any. Returns ``true``
if it finds variables that match the given name, otherwise returns
``false``. The error value is set to ``No_Such_Variable``.

.. code-block:: cpp

    bool getAllVariables(vector<Variable *> &ret)

This method returns all variables in the object file. Returns ``true``
on success and ``false`` if there are no modules. The error value is set
to ``No_Such_Variable``.

.. code-block:: cpp

    bool findSymbol(vector <Symbol *> &ret, const string name, Symbol::SymbolType sType, NameType nameType = anyName, bool isRegex = false, bool checkCase = false)

This method finds and returns a vector of symbols with type ``sType``
whose names match the given name. The ``nameType`` parameter determines
which names are searched: mangled, pretty, typed, or any. If the
``isRegex`` flag is set a regular expression match is performed with the
symbol names. ``checkCase`` is applicable only if ``isRegex`` has been
set. This indicates if the case be considered while performing regular
expression matching. ``ret`` contains the list of matched symbols if
any. Returns ``true`` if it finds symbols with the given attributes. or
else returns ``false``. The error value is set ``to No_Such_Function`` /
``No_Such_Variable``/ ``No_Such_Module``/ ``No_Such_Symbol`` based on
the type.

.. code-block:: cpp

    const vector<Symbol *> *findSymbolByOffset(Offset offset)

Return a pointer to a vector of ``Symbol``\ s with the specified offset.
The pointer belongs to ``Symtab`` and should not be modified or freed.

.. code-block:: cpp

    bool getAllSymbols(vector<Symbol *> &ret)

This method returns all symbols. Returns ``true`` on success and
``false`` if there are no symbols. The error value is set to
``No_Such_Symbol``.

.. code-block:: cpp

    bool getAllSymbolsByType(vector<Symbol *> &ret, Symbol::SymbolType sType)

This method returns all symbols whose type matches the given type
``sType``. Returns ``true`` on success and ``false`` if there are no
symbols with the given type. The error value is set to
``No_Such_Symbol``.

.. code-block:: cpp

    bool getAllUndefinedSymbols(std::vector<Symbol *> &ret)

This method returns all symbols that reference symbols in other files
(e.g., external functions or variables). Returns ``true`` if there is at
least one such symbol or else returns ``false`` with the error set to
``No_Such_Symbol``.

Region lookup
~~~~~~~~~~~~~

.. code-block:: cpp

    bool getCodeRegions(std::vector<Region *>&ret)

This method finds all the code regions in the object file. Returns
``true`` with ``ret`` containing the code regions if there is at least
one code region in the object file or else returns ``false``.

.. code-block:: cpp

    bool getDataRegions(std::vector<Region *>&ret)

This method finds all the data regions in the object file. Returns
``true`` with ``ret`` containing the data regions if there is at least
one data region in the object file or else returns ``false``.

.. code-block:: cpp

    bool getMappedRegions(std::vector<Region *>&ret)

This method finds all the loadable regions in the object file. Returns
``true`` with ``ret`` containing the loadable regions if there is at
least one loadable region in the object file or else returns ``false``.

.. code-block:: cpp

   bool getAllRegions(std::vector<Region *>&ret)

This method retrieves all the regions in the object file. Returns
``true`` with ``ret`` containing the regions.

.. code-block:: cpp

    bool getAllNewRegions(std::vector<Region *>&ret)

This method finds all the new regions added to the object file. Returns
``true`` with ``ret`` containing the regions if there is at least one
new region that is added to the object file or else returns ``false``.

.. code-block:: cpp

    bool findRegion(Region *&reg, string sname)

Find a region (ELF section) wih name ``sname`` in the binary. Returns
``true`` if found, with ``reg`` set to the region pointer. Otherwise
returns ``false`` with ``reg`` set to ``NULL``.

.. code-block:: cpp
    
    bool findRegion(Region *&reg, const Offset addr, const unsigned long size)

Find a region (ELF section) with a memory offset of ``addr`` and memory
size of ``size``. Returns ``true`` if found, with ``reg`` set to the
region pointer. Otherwise returns ``false`` with ``reg`` set to
``NULL``.

.. code-block:: cpp

    bool findRegionByEntry(Region *&reg, const Offset soff)

Find a region (ELF section) with a memory offset of ``addr``. Returns
``true`` if found, with ``reg`` set to the region pointer. Otherwise
returns ``false`` with ``reg`` set to ``NULL``.

.. code-block:: cpp

    Region *findEnclosingRegion(const Offset offset)

Find the region (ELF section) whose virtual address range contains
``offset``. Returns the region if found; otherwise returns ``NULL``.

Insertion and modification
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: cpp

    bool emit(string file)

Creates a new file using the specified name that contains all changes
made by the user.

.. code-block:: cpp

    bool addLibraryPrereq(string lib)

Add a library dependence to the file such that when the file is loaded,
the library will be loaded as well. Cannot be used for static binaries.

.. code-block:: cpp

    Function *createFunction(std::string name, Offset offset, size_t size, Module *mod = NULL)

This method creates a ``Function`` and updates all necessary data
structures (including creating Symbols, if necessary). The function has
the provided mangled name, offset, and size, and is added to the Module
``mod``. Symbols representing the function are added to the static and
dynamic symbol tables. Returns the pointer to the new ``Function`` on
success or ``NULL`` on failure.

.. code-block:: cpp

    Variable *createVariable(std::string name, Offset offset, size_t size, Module *mod = NULL)

This method creates a ``Variable`` and updates all necessary data
structures (including creating Symbols, if necessary). The variable has
the provided mangled name, offset, and size, and is added to the Module
``mod``. Symbols representing the variable are added to the static and
dynamic symbol tables. Returns the pointer to the new ``Variable`` on
success or ``NULL`` on failure.

.. code-block:: cpp

    bool addSymbol(Symbol *newsym)

This method adds a new symbol ``newsym`` to all of the internal data
structures. The primary name of the ``newsym`` must be a mangled name.
Returns ``true`` on success and ``false`` on failure. A new copy of
``newsym`` is not made. ``newsym`` must not be deallocated after adding
it to symtabAPI. We suggest using ``createFunction`` or
``createVariable`` when possible.

.. code-block:: cpp

    bool addSymbol(Symbol *newsym, Symbol *referringSymbol)

This method adds a new dynamic symbol ``newsym`` which refers to
``referringSymbol`` to all of the internal data structures. ``newsym``
must represent a dynamic symbol. The primary name of the newsym must be
a mangled name. All the required version names are allocated
automatically. Also if the ``referringSymbol`` belongs to a shared
library which is not currently a dependency, the shared library is added
to the list of dependencies implicitly. Returns ``true`` on success and
``false`` on failure. A new copy of ``newsym`` is not made. ``newsym``
must not be deallocated after adding it to symtabAPI.

.. code-block:: cpp
    
    bool deleteFunction(Function *func)

This method deletes the ``Function`` ``func`` from all of symtab’s data
structures. It will not be available for further queries. Return
``true`` on success and ``false`` if ``func`` is not owned by the
``Symtab``.

.. code-block:: cpp

    bool deleteVariable(Variable *var)

This method deletes the variable ``var`` from all of symtab’s data
structures. It will not be available for further queries. Return
``true`` on success and ``false`` if ``var`` is not owned by the
``Symtab``.

.. code-block:: cpp

    bool deleteSymbol(Symbol *sym)

This method deletes the symbol ``sym`` from all of symtab’s data
structures. It will not be available for further queries. Return
``true`` on success and ``false`` if func is not owned by the
``Symtab``.

.. code-block:: cpp

    bool addRegion(Offset vaddr, void *data, unsigned int dataSize, std::string name, Region::RegionType rType_, bool loadable = false, unsigned long memAlign = sizeof(unsigned), bool tls = false)

Creates a new region using the specified parameters and adds it to the
file.

.. code-block:: cpp

    Offset getFreeOffset(unsigned size)

Find a contiguous region of unused space within the file (which may be
at the end of the file) of the specified size and return an offset to
the start of the region. Useful for allocating new regions.

.. code-block:: cpp
    
    bool addRegion(Region *newreg);

Adds the provided region to the file.

Catch and Exception block lookup
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. cpp:class:: ExceptionBlock

.. code-block:: cpp

    bool getAllExceptions(vector<ExceptionBlock *> &exceptions)

This method retrieves all the exception blocks in the ``Object`` file.
Returns ``false`` if there are no exception blocks else returns ``true``
with exceptions containing a vector of ``ExceptionBlock``\ s.

.. code-block:: cpp

    bool findException(ExceptionBlock &excp, Offset addr)

This method returns the exception block in the binary at the offset
``addr``. Returns ``false`` if there is no exception block at the given
offset else returns ``true`` with ``excp`` containing the exception
block.

.. code-block:: cpp

    bool findCatchBlock(ExceptionBlock &excp, Offset addr, unsigned size = 0)

This method returns ``true`` if the address range ``[addr, addr+size]``
contains a catch block, with ``excp`` pointing to the appropriate block,
else returns ``false``.

Symtab information
~~~~~~~~~~~~~~~~~~

.. code-block:: cpp

   typedef enum obj_Unknown, obj_SharedLib, obj_Executable, obj _RelocatableFile, ObjectType; bool isCode(const Offset where) const

This method checks if the given offset ``where`` belongs to the text
section. Returns ``true`` if that is the case or else returns ``false``.

.. code-block:: cpp

    bool isData(const Offset where) const

This method checks if the given offset ``where`` belongs to the data
section. Returns ``true`` if that is the case or else returns ``false``.

.. code-block:: cpp

    bool isValidOffset(const Offset where) const

This method checks if the given offset ``where`` is valid. For an offset
to be valid it should be aligned and it should be a valid code offset or
a valid data offset. Returns ``true`` if it succeeds or else returns
``false``.

Line number information
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: cpp

    bool getAddressRanges(vector<pair<Offset, Offset> > & ranges, string lineSource, unsigned int LineNo)

This method returns the address ranges in ``ranges`` corresponding to
the line with line number ``lineNo`` in the source file ``lineSource``.
Searches all modules for the given source. Return ``true`` if at least
one address range corresponding to the line number was found and returns
``false`` if none found.

.. code-block:: cpp

    bool getSourceLines(vector<LineNoTuple> &lines, Offset addressInRange)

This method returns the source file names and line numbers corresponding
to the given address ``addressInRange``. Searches all modules for the
given source. Return ``true`` if at least one tuple corresponding to the
offset was found and returns ``false`` if none found.

.. code-block:: cpp

    bool addLine(string lineSource, unsigned int lineNo, unsigned int lineOffset, Offset lowInclusiveAddr, Offset highExclusiveAddr)

This method adds a new line to the line map. ``lineSource`` represents
the source file name. ``lineNo`` represents the line number. Returns
``true`` on success and ``false`` on error.

.. code-block:: cpp

    bool addAddressRange(Offset lowInclusiveAddr, Offset highExclusiveAddr, string lineSource, unsigned int lineNo, unsigned int lineOffset = 0);

This method adds an address range
``[lowInclusiveAddr, highExclusiveAddr)`` for the line with line number
``lineNo`` in source file ``lineSource`` at offset ``lineOffset``.
Returns ``true`` on success and ``false`` on error.

Type information
~~~~~~~~~~~~~~~~


.. code-block:: cpp

    void parseTypesNow()

Forces SymtabAPI to perform type parsing instead of delaying it to when
needed.

.. code-block:: cpp

    bool findType(Type *&type, string name)

Performs a look up among all the built-in types, standard types and
user-defined types and returns a handle to the found type with name
``name``. Returns ``true`` if a type is found with type containing the
handle to the type, else return ``false``.

.. code-block:: cpp

    bool addType(Type * type)

Adds a new type ``type`` to symtabAPI. Return ``true`` on success.

.. code-block:: cpp

    static std::vector<Type *> * getAllstdTypes()

Returns all the standard types that normally occur in a program.

.. code-block:: cpp

    static std::vector<Type *> * getAllbuiltInTypes()

Returns all the built-in types defined in the binary.

.. code-block:: cpp

    bool findLocalVariable(vector<localVar *> &vars, string name)

The method returns a list of local variables named name within the
object file. Returns ``true`` with ``vars`` containing a list of
``localVar`` objects corresponding to the local variables if found or
else returns ``false``.

.. code-block:: cpp

    bool findVariableType(Type *&type, std::string name)

This method looks up a global variable with name ``name`` and returns
its type attribute. Returns ``true`` if a variable is found or returns
``false`` with type set to ``NULL``.

.. code-block:: cpp

    typedef enum ... SymtabError

``SymtabError`` can take one of the following values.

.. container:: center

   +-------------------------+-------------------------------------------+
   | SymtabError enum        | Meaning                                   |
   +=========================+===========================================+
   | Obj_Parsing             | An error occurred during object           |
   |                         | parsing(internal error).                  |
   +-------------------------+-------------------------------------------+
   | Syms_To_Functions       | An error occurred in converting symbols   |
   |                         | to functions(internal error).             |
   +-------------------------+-------------------------------------------+
   | Build_Function_Lists    | An error occurred while building function |
   |                         | lists(internal error).                    |
   +-------------------------+-------------------------------------------+
   | No_Such_Function        | No matching function exists with the      |
   |                         | given inputs.                             |
   +-------------------------+-------------------------------------------+
   | No_Such_Variable        | No matching variable exists with the      |
   |                         | given inputs.                             |
   +-------------------------+-------------------------------------------+
   | No_Such_Module          | No matching module exists with the given  |
   |                         | inputs.                                   |
   +-------------------------+-------------------------------------------+
   | No_Such_Symbol          | No matching symbol exists with the given  |
   |                         | inputs.                                   |
   +-------------------------+-------------------------------------------+
   | No_Such_Region          | No matching region exists with the given  |
   |                         | inputs.                                   |
   +-------------------------+-------------------------------------------+
   | No_Such_Member          | No matching member exists in the archive  |
   |                         | with the given inputs.                    |
   +-------------------------+-------------------------------------------+
   | Not_A_File              | Binary to be parsed may be an archive and |
   |                         | not a file.                               |
   +-------------------------+-------------------------------------------+
   | Not_An_Archive          | Binary to be parsed is not an archive.    |
   +-------------------------+-------------------------------------------+
   | Duplicate_Symbol        | Duplicate symbol found in symbol table.   |
   +-------------------------+-------------------------------------------+
   | Export_Error            | Error occurred during export of modified  |
   |                         | symbol table.                             |
   +-------------------------+-------------------------------------------+
   | Emit_Error              | Error occurred during generation of       |
   |                         | modified binary.                          |
   +-------------------------+-------------------------------------------+
   | Invalid_Flags           | Flags passed are invalid.                 |
   +-------------------------+-------------------------------------------+
   | Bad_Frame_Data          | Stack walking DWARF information has bad   |
   |                         | frame data.                               |
   +-------------------------+-------------------------------------------+
   | No_Frame_Entry          | No stack walking frame data found in      |
   |                         | debug information for this location.      |
   +-------------------------+-------------------------------------------+
   | Frame_Read_Error        | Failed to read stack frame data.          |
   +-------------------------+-------------------------------------------+
   | Multiple_Region_Matches | Multiple regions match the provided data. |
   +-------------------------+-------------------------------------------+
   | No_Error                | Previous operation did not result in      |
   |                         | failure.                                  |
   +-------------------------+-------------------------------------------+

.. code-block:: cpp

    static SymtabError getLastSymtabError()

This method returns an error value for the previously performed
operation that resulted in a failure. SymtabAPI sets a global error
value in case of error during any operation. This call returns the last
error that occurred while performing any operation.

.. code-block:: cpp

    static string printError(SymtabError serr)

This method returns a detailed description of the enum value serr in
human readable format.

Class ExceptionBlock
--------------------

This class represents an exception block present in the object file.
This class gives all the information pertaining to that exception block.

=========== =========== ============================================
Method name Return type Method description
=========== =========== ============================================
hasTry      bool        True if the exception block has a try block.
tryStart    Offset      Start of the try block if it exists, else 0.
tryEnd      Offset      End of the try block if it exists, else 0.
trySize     Offset      Size of the try block if it exists, else 0.
catchStart  Offset      Start of the catch block.
=========== =========== ============================================

.. code-block:: cpp

    bool contains(Offset addr) const

This method returns ``true`` if the offset ``addr`` is contained with in
the try block. If there is no try block associated with this exception
block or the offset does not fall within the try block, it returns
``false``.

relocationEntry
---------------

This class represents object relocation information.

.. code-block:: cpp

    Offset target_addr() const

Specifies the offset that will be overwritten when relocations are
processed.

.. code-block:: cpp

    Offset rel_addr() const

Specifies the offset of the relocation itself.

.. code-block:: cpp

    Offset addend() const

Specifies the value added to the relocation; whether this value is used
or not is specific to the relocation type.

.. code-block:: cpp

    const std::string name() const

Specifies the user-readable name of the relocation.

.. code-block:: cpp

    Symbol *getDynSym() const

Specifies the symbol whose final address will be used in the relocation
calculation. How this address is used is specific to the relocation
type.

.. code-block:: cpp

    unsigned long getRelType() const

Specifies the platform-specific relocation type.
