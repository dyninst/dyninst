.. _sec:immediate:

Immediate Class
---------------

The Immediate class represents an immediate value in an operand.

Since an Immediate represents a constant value, the and interface are
disabled on Immediate objects. If an immediate value is being modified,
a new Immediate object should be created to represent the new value.

virtual bool isUsed(InstructionAST::Ptr findMe) const

void getChildren(vector<InstructionAST::Ptr> &) const

void getUses(set<InstructionAST::Ptr> &)

bool isUsed(InstructionAPI::Ptr findMe) const
