Function.h
==========

.. cpp:namespace:: Dyninst::SymtabAPI

Class FunctionBase
------------------

The ``FunctionBase`` class provides a common interface that can
represent either a regular function or an inlined function.

.. list-table:: FunctionBase Class
   :widths: 30  35 35
   :header-rows: 1

   * - Method name
     - Return type
     - Method description
   * - getModule
     - const Module *
     - Module this function belongs to.
   * - getSize
     - unsigned
     - Size encoded in the symbol table; may not be actual function size.
   * - getRegion
     - Region *
     - Region containing this function.
   * - getReturnType
     - Type *
     - Type representing the return type of the function.
   * - getName
     - std::string
     - Returns primary name of the function (first mangled name or DWARF name).


.. code-block:: cpp

    bool setModule (Module *module)

This function changes the module to which the function belongs to
``module``. Returns ``true`` if it succeeds.

.. code-block:: cpp

    bool setSize (unsigned size)

This function changes the size of the function to ``size``. Returns
``true`` if it succeeds.

.. code-block:: cpp

    bool setOffset (Offset offset)

The method changes the offset of the function to ``offset``. Returns
``true`` if it succeeds.

.. code-block:: cpp

    bool addMangledName(string name, bool isPrimary)

This method adds a mangled name ``name`` to the function. If
``isPrimary`` is ``true`` then it becomes the default name for the
function. This method returns ``true`` on success and ``false`` on
failure.

.. code-block:: cpp

    bool addPrettyName(string name, bool isPrimary)

This method adds a pretty name ``name`` to the function. If
``isPrimary`` is ``true`` then it becomes the default name for the
function. This method returns ``true`` on success and ``false`` on
failure.

.. code-block:: cpp

    bool addTypedName(string name, bool isPrimary)

This method adds a typed name ``name`` to the function. If ``isPrimary``
is ``true`` then it becomes the default name for the function. This
method returns ``true`` on success and ``false`` on failure.

.. code-block:: cpp

    bool getLocalVariables(vector<localVar *> &vars)

This method returns the local variables in the function. ``vars``
contains the list of variables found. If there is no debugging
information present then it returns ``false`` with the error code set to
``NO_DEBUG_INFO`` accordingly. Otherwise it returns ``true``.

.. code-block:: cpp

    std::vector<VariableLocation> &getFramePtr()

This method returns a list of frame pointer offsets (abstract top of the
stack) for the function. See the ``VariableLocation`` class description
for more information.

.. code-block:: cpp
    
    bool getParams(vector<localVar *> &params)

This method returns the parameters to the function. ``params`` contains
the list of parameters. If there is no debugging information present
then it returns ``false`` with the error code set to ``NO_DEBUG_INFO``
accordingly. Returns ``true`` on success.

.. code-block:: cpp

    bool findLocalVariable(vector<localVar *> &vars, string name)

This method returns a list of local variables within a function that
have name ``name``. ``vars`` contains the list of variables found.
Returns ``true`` on success and ``false`` on failure.

.. code-block:: cpp

    FunctionBase* getInlinedParent()

Gets the function that this function is inlined into, if any. Returns
``NULL`` if there is no parent.

.. code-block:: cpp

    const InlineCollection& getInlines()

Gets the set of functions inlined into this one (possibly empty).

.. _Function:

Symbtab Class Function
----------------------

The ``Function`` class represents a collection of symbols that have the
same address and a type of ``ST_FUNCTION``. When appropriate, use this
representation instead of the underlying ``Symbol`` objects.

.. list-table:: Class Function
   :widths: 30  35 35
   :header-rows: 1

   * - Method name
     - Return type
     - Method description
   * - getModule
     - const Module *
     - Module this function belongs to.
   * - getOffset
     - Offset
     - Offset in the file associated with the function.
   * - getSize
     - unsigned
     - Size encoded in the symbol table; may not be actual function size.
   * - mangled_names_begin
     - Aggregate::name_iter
     - Beginning of a range of unique names of symbols pointing to this function.
   * - mangled_names_end
     - Aggregate::name_iter
     - End of a range of symbols pointing to this function.
   * - pretty_names_begin
     - Aggregate::name_iter
     - As above, but prettified with the demangler.
   * - pretty_names_end
     - Aggregate::name_iter
     - As above, but prettified with the demangler.
   * - typed_names_begin
     - Aggregate::name_iter
     - As above, but including full type strings.
   * - typed_names_end
     - Aggregate::name_iter
     - As above, but including full type strings.
   * - getRegion
     - Region *
     - Region containing this function
   * - getReturnType
     - Type *
     - Type representing the return type of the function.

.. code-block:: cpp

    bool getSymbols(vector<Symbol *> &syms) const

This method returns the vector of ``Symbol``\ s that refer to the
function.

.. code-block:: cpp

    bool setModule (Module *module)

This function changes the module to which the function belongs to
``module``. Returns ``true`` if it succeeds.

.. code-block:: cpp

    bool setSize (unsigned size)

This function changes the size of the function to ``size``. Returns
``true`` if it succeeds.

.. code-block:: cpp

    bool setOffset (Offset offset)

The method changes the offset of the function to ``offset``. Returns
``true`` if it succeeds.

.. code-block:: cpp

    bool addMangledName(string name, bool isPrimary)

This method adds a mangled name ``name`` to the function. If
``isPrimary`` is ``true`` then it becomes the default name for the
function. This method returns ``true`` on success and ``false`` on
failure.

.. code-block:: cpp

    bool addPrettyName(string name, bool isPrimary)

This method adds a pretty name ``name`` to the function. If
``isPrimary`` is ``true`` then it becomes the default name for the
function. This method returns ``true`` on success and ``false`` on
failure.

.. code-block:: cpp

    bool addTypedName(string name, bool isPrimary)

This method adds a typed name ``name`` to the function. If ``isPrimary``
is ``true`` then it becomes the default name for the function. This
method returns ``true`` on success and ``false`` on failure.

.. code-block:: cpp

    bool getLocalVariables(vector<localVar *> &vars)

This method returns the local variables in the function. ``vars``
contains the list of variables found. If there is no debugging
information present then it returns ``false`` with the error code set to
``NO_DEBUG_INFO`` accordingly. Otherwise it returns ``true``.

.. code-block:: cpp
    
    std::vector<VariableLocation> &getFramePtr()

This method returns a list of frame pointer offsets (abstract top of the
stack) for the function. See the ``VariableLocation`` class description
for more information.

.. code-block:: cpp

    bool getParams(vector<localVar *> &params)

This method returns the parameters to the function. ``params`` contains
the list of parameters. If there is no debugging information present
then it returns ``false`` with the error code set to ``NO_DEBUG_INFO``
accordingly. Returns ``true`` on success.

.. code-block:: cpp

    bool findLocalVariable(vector<localVar *> &vars, string name)

This method returns a list of local variables within a function that
have name ``name``. ``vars`` contains the list of variables found.
Returns ``true`` on success and ``false`` on failure.

.. _InlinedFunction:

Class InlinedFunction
---------------------

The ``InlinedFunction`` class represents an inlined function, as found
in DWARF information. Its interface is almost entirely inherited from
``FunctionBase``.

.. code-block:: cpp

    std::pair<std::string, Dyninst::Offset> getCallsite()

Returns the file and line corresponding to the call site of an inlined
function.