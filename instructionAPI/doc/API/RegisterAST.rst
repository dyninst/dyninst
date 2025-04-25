.. _sec:registerAST:

RegisterAST Class
-----------------

A object represents a register contained in an operand. As a is an , it
may contain the physical register’s contents if they are known.

typedef dyn_detail::boost::shared_ptr<RegisterAST> Ptr

.. code::
  RegisterAST (MachRegister r)

Constructor

.. code::
  void getChildren (vector< Expression::Ptr > & children) const

By definition, a ``RegisterAST`` object has no children. Since a ``RegisterAST`` has no children,
the ``children`` parameter is unchanged by this method.

.. code::
  void getUses (set< Expression::Ptr > & uses)

By definition, the use set of a ``RegisterAST`` object is itself. This ``RegisterAST`` will be
inserted into ``uses``.

.. code::
  bool isUsed (Expression::Ptr findMe) const

``isUsed`` returns ``true`` if ``findMe`` is a ``RegisterAST`` that represents the same
register as this ``RegisterAST``, and ``false`` otherwise.

.. code::
  std::string format (formatStyle how = defaultStyle) const

.. code::
  RegisterAST makePC (Dyninst::Architecture arch) [static]

.. code::
  bool operator< (const RegisterAST & rhs) const

.. code::
  RegisterAST::Ptr promote (const Expression::Ptr reg) [static]

Utility function to hide aliasing complexity on platforms (IA-32) that allow addressing part
or all of a register

.. _sec:MaskRegisterAST:

MaskRegisterAST Class
---------------------

Class for mask register operands. This class is the same as the
RegisterAST class except it handles the syntactial differences between
register operands and mask register operands.
