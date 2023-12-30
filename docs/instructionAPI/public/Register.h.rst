Register.h
==========

.. cpp:namespace:: Dyninst::InstructionAPI

RegisterAST Class
-----------------

A ``RegisterAST`` object represents a register contained in an operand.
As a ``RegisterAST`` is an ``Expression``, it may contain the physical
register’s contents if they are known.

.. code-block:: cpp

    typedef dyn_detail::boost::shared_ptr<RegisterAST> Ptr

A type definition for a reference-counted pointer to a ``RegisterAST``.

.. code-block:: cpp

    RegisterAST (MachRegister r)

Construct a register using the provided register object ``r``. The
``MachRegister`` datatype is Dyninst’s register representation and
should not be constructed manually.

.. code-block:: cpp

    void getChildren (vector< InstructionAST::Ptr > & children) const

By definition, a ``RegisterAST`` object has no children. Since a
``RegisterAST`` has no children, the ``children`` parameter is unchanged
by this method.

.. code-block:: cpp

    void getUses (set< InstructionAST::Ptr > & uses)

By definition, the use set of a ``RegisterAST`` object is itself. This
``RegisterAST`` will be inserted into ``uses``.

.. code-block:: cpp

    bool isUsed (InstructionAST::Ptr findMe) const

``isUsed`` returns ``true`` if ``findMe`` is a ``RegisterAST`` that
represents the same register as this ``RegisterAST``, and ``false``
otherwise.

.. code-block:: cpp

     std::string format (formatStyle how = defaultStyle) const

The format method on a ``RegisterAST`` object returns the name
associated with its ID.

.. code-block:: cpp
 
    RegisterAST makePC (Dyninst::Architecture arch) [static]

Utility function to get a ``Register`` object that represents the
program counter. ``makePC`` is provided to support platform-independent
control flow analysis.

.. code-block:: cpp

    bool operator< (const RegisterAST & rhs) const

We define a partial ordering on registers by their register number so
that they may be placed into sets or other sorted containers.

.. code-block:: cpp
    
    MachRegister getID () const

The ``getID`` function returns underlying register represented by this
AST.

.. code-block:: cpp

    RegisterAST::Ptr promote (const InstructionAST::Ptr reg) [static]

Utility function to hide aliasing complexity on platforms (IA-32) that
allow addressing part or all of a register