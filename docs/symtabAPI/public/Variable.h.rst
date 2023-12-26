Variable.h
==========

.. cpp:namespace:: Dyninst::SymtabAPI

Class Variable
--------------

The ``Variable`` class represents a collection of symbols that have the
same address and represent data.

.. list-table:: Variable Class
   :widths: 30  35 35
   :header-rows: 1

   * - Method name
     - Return type
     - Method description
   * - getOffset
     - Offset
     - Offset associated with this variable.
   * - getSize
     - unsigned
     - Size of this variable encoded in the symbol table.
   * - mangled_names_begin
     - Aggregate::name_iter
     - Beginning of a range of unique names of symbols pointing to this variable.
   * - mangled_names_end
     - Aggregate::name_iter
     - End of a range of unique names of symbols pointing to this variable.
   * - getType
     - Type *
     - Type of this variable, if known.
   * - getModule
     - const Module *
     - Module this variable belongs to.
   * - getRegion
     - Region *
     - Region that contains this variable.

.. code-block:: cpp

    bool getSymbols(vector<Symbol *> &syms) const

This method returns the vector of ``Symbol``\ s that refer to the
variable.

.. code-block:: cpp

    bool setModule (Module *module)

This method changes the module to which the variable belongs. Returns
``true`` if it succeeds.

.. code-block:: cpp
   
    bool setSize (unsigned size)

This method changes the size of the variable to ``size``. Returns
``true`` if it succeeds.

.. code-block:: cpp

    bool setOffset (Offset offset)

The method changes the offset of the variable. Returns ``true`` if it
succeeds.

.. code-block:: cpp

    bool addMangledName(string name, bool isPrimary)

This method adds a mangled name ``name`` to the variable. If
``isPrimary`` is ``true`` then it becomes the default name for the
variable. This method returns ``true`` on success and ``false`` on
failure.

.. code-block:: cpp

    bool addPrettyName(string name, bool isPrimary)

This method adds a pretty name ``name`` to the variable. If
``isPrimary`` is ``true`` then it becomes the default name for the
variable. This method returns ``true`` on success and ``false`` on
failure.

.. code-block:: cpp

    bool addTypedName(string name, bool isPrimary)

This method adds a typed name ``name`` to the variable. If ``isPrimary``
is ``true`` then it becomes the default name for the variable. This
method returns ``true`` on success and ``false`` on failure.

.. code-block:: cpp

    bool setType(Type *type)

Sets the type of the variable to ``type``.

Class localVar
--------------

This represents a local variable or parameter of a function.

.. list-table:: Class localVar
   :widths: 30  35 35
   :header-rows: 1

   * - Method name
     - Return type
     - Method description
   * - getName
     - string &
     - Name of the local variable or parameter.
   * - getType
     - Type *
     - Type associated with the variable.
   * - getFileName
     - string &
     - File where the variable was declared, if known.
   * - getLineNum
     - int
     - Line number where the variable was declared, if known.

.. code-block:: cpp

    vector<VariableLocation> &getLocationLists()

A local variable can be in scope at different positions and based on
that it is accessible in different ways. Location lists provide a way to
encode that information. The method retrieves the location list,
specified in terms of ``VariableLocation`` structures (section
`6.13 <#VariableLocation>`__) where the variable is in scope.
