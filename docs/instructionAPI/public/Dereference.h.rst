Dereference.h
=============

.. cpp:namespace:: Dyninst::InstructionAPI

Dereference Class
-----------------

A ``Dereference`` object is an ``Expression`` that dereferences another
``ValueComputation``.

A ``Dereference`` contains an ``Expression`` representing an effective
address computation. Its use set is the same as the use set of the
``Expression`` being dereferenced.

It is not possible, given the information in a single instruction, to
evaluate the result of a dereference. ``eval`` may still be called on an
``Expression`` that includes dereferences, but the expected use case is
as follows:

-  Determine the address being used in a dereference via the ``eval``
   mechanism

-  Perform analysis to determine the contents of that address

-  If necessary, fill in the ``Dereference`` node with the contents of
   that addresss, using ``setValue``

The type associated with a ``Dereference`` node will be the type of the
value *read* *from* *memory*, not the type used for the address
computation. Two ``Dereference``\ s that access the same address but
interpret the contents of that memory as different types will produce
different values. The children of a ``Dereference`` at a given address
are identical, regardless of the type of dereference being performed at
that address. For example, the ``Expression`` shown in Figure 6 could
have its root ``Dereference``, which interprets the memory being
dereferenced as a unsigned 16-bit integer, replaced with a
``Dereference`` that interprets the memory being dereferenced as any
other type. The remainder of the ``Expression`` tree would, however,
remain unchanged.

.. code-block:: cpp

    Dereference (Expression::Ptr addr, Result_Type result_type)

A ``Dereference`` is constructed from an ``Expression`` pointer (raw or
shared) representing the address to be dereferenced and a type
indicating how the memory at the address in question is to be
interpreted.

.. code-block:: cpp

    virtual void getChildren (vector< InstructionAST::Ptr > & children) const

A ``Dereference`` has one child, which represents the address being
dereferenced. Appends the child of this ``Dereference`` to ``children``.

.. code-block:: cpp

    virtual void getUses (set< InstructionAST::Ptr > & uses)

The use set of a ``Dereference`` is the same as the use set of its
children. The use set of this ``Dereference`` is inserted into ``uses``.

.. code-block:: cpp

    virtual bool isUsed (InstructionAST::Ptr findMe) const

An ``InstructionAST`` is used by a ``Dereference`` if it is equivalent
to the ``Dereference`` or it is used by the lone child of the
``Dereference``