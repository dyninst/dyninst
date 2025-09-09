.. _sec:operand:

Operand Class
-------------

An Operand object contains an AST built from RegisterAST and Immediate
leaves, and information about whether the Operand is read, written, or
both. This allows us to determine which of the registers that appear in
the Operand are read and which are written, as well as whether any
memory accesses are reads, writes, or both. An Operand, given full
knowledge of the values of the leaves of the AST, and knowledge of the
logic associated with the tree’s internal nodes, can determine the
result of any computations that are encoded in it. It will rarely be the
case that an Instruction is built with its Operands’ state fully
specified. This mechanism is instead intended to allow a user to fill in
knowledge about the state of the processor at the time the Instruction
is executed.

Operand(Expression::Ptr val, bool read, bool written)

void getReadSet(std::set<RegisterAST::Ptr> & regsRead) const

void getWriteSet(std::set<RegisterAST::Ptr> & regsWritten) const

bool isRead() const

bool isWritten() const

bool isRead(Expression::Ptr candidate) const

bool isWritten(Expression::Ptr candidate) const

bool readsMemory() const

bool writesMemory() const

void addEffectiveReadAddresses(std::set<Expression::Ptr> & memAccessors)
const

void addEffectiveWriteAddresses(std::set<Expression::Ptr> &
memAccessors) const

std::string format(Architecture arch, Address addr = 0) const

Expression::Ptr getValue() const
