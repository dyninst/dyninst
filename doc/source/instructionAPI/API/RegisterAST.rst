.. _sec:registerAST:

RegisterAST Class
-----------------

A object represents a register contained in an operand. As a is an , it
may contain the physical register’s contents if they are known.

typedef dyn_detail::boost::shared_ptr<RegisterAST> Ptr

RegisterAST (MachRegister r)

void getChildren (vector< InstructionAST::Ptr > & children) const

void getUses (set< InstructionAST::Ptr > & uses)

bool isUsed (InstructionAST::Ptr findMe) const

std::string format (formatStyle how = defaultStyle) const

RegisterAST makePC (Dyninst::Architecture arch) [static]

bool operator< (const RegisterAST & rhs) const

MachRegister getID () const

RegisterAST::Ptr promote (const InstructionAST::Ptr reg) [static]

.. _sec:MaskRegisterAST:

MaskRegisterAST Class
---------------------

Class for mask register operands. This class is the same as the
RegisterAST class except it handles the syntactial differences between
register operands and mask register operands.
