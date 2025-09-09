.. _sec:symeval:

Class SymEval
-------------

Class SymEval provides interfaces for expanding an instruction to its
symbolic expression and expanding a slice graph to symbolic expressions
for all abstract locations defined in this slice.

typedef std::map<Assignment::Ptr, AST::Ptr, AssignmentPtrValueComp>
Result_t;

static std::pair<AST::Ptr, bool> expand(const Assignment::Ptr
&assignment, bool applyVisitors = true);

static bool expand(Result_t &res, std::set<InstructionPtr> &failedInsns,
bool applyVisitors = true);

================== ==================
Retval_t           Meaning
================== ==================
FAILED             failed
WIDEN_NODE         widen
FAILED_TRANSLATION failed translation
SKIPPED_INPUT      skipped input
SUCCESS            success
================== ==================

static Retval_t expand(Dyninst::Graph::Ptr slice, DataflowAPI::Result_t
&res);

We use an AST to represent the symbolic expressions of an assignment. A
symbolic expression AST contains internal node type , which abstracts
the operations performed with its child nodes, and two leave node types:
and .

, , and all extend class . Besides the methods provided by class , , ,
and each have a different data structure associated with them.

Variable& VariableAST::val() const; Constant& ConstantAST::val() const;
ROSEOperation & RoseAST::val() const;

We now describe data structure , , and .

struct Variable;

Variable::Variable(); Variable::Variable(AbsRegion r);
Variable::Variable(AbsRegion r, Address a);

bool Variable::operator==(const Variable &rhs) const; bool
Variable::operator<(const Variable &rhs) const;

const std::string Variable::format() const;

AbsRegion Variable::reg; Address Variable::addr;

struct Constant;

Constant::Constant(); Constant::Constant(uint64_t v);
Constant::Constant(uint64_t v, size_t s);

bool Constant::operator==(const Constant &rhs) const; bool
Constant::operator<(const Constant &rhs) const;

const std::string Constant::format() const;

uint64_t Constant::val; size_t Constant::size;

struct ROSEOperation;

defines the following operations and we represent the semantics of all
instructions with these operations.

================= ==========================================
ROSEOperation::Op Meaning
================= ==========================================
nullOp            No operation
extractOp         Extract bit ranges from a value
invertOp          Flip every bit
negateOp          Negate the value
signExtendOp      Sign-extend the value
equalToZeroOp     Check whether the value is zero or not
generateMaskOp    Generate mask
LSBSetOp          LSB set op
MSBSetOp          MSB set op
concatOp          Concatenate two values to form a new value
andOp             Bit-wise and operation
orOp              Bit-wise or operation
xorOp             Bit-wise xor operation
addOp             Add operation
rotateLOp         Rotate to left operation
rotateROp         Rotate to right operation
shiftLOp          Shift to left operation
shiftROp          Shift to right operation
shiftRArithOp     Arithmetic shift to right operation
derefOp           Dereference memory operation
writeRepOp        Write rep operation
writeOp           Write operation
ifOp              If operation
sMultOp           Signed multiplication operation
uMultOp           Unsigned multiplication operation
sDivOp            Signed division operation
sModOp            Signed modular operation
uDivOp            Unsigned division operation
uModOp            Unsigned modular operation
extendOp          Zero extend operation
extendMSBOp       Extend the most significant bit operation
================= ==========================================

ROSEOperation::ROSEOperation(Op o) : op(o);
ROSEOperation::ROSEOperation(Op o, size_t s);

bool ROSEOperation::operator==(const ROSEOperation &rhs) const;

const std::string ROSEOperation::format() const;

ROSEOperation::Op ROSEOperation::op; size_t ROSEOperation::size;

Class ASTVisitor
----------------

The ASTVisitor class defines callback functions to apply during visiting
an AST for each AST node type. Users can inherit from this class to
write customized analyses for ASTs.

typedef boost::shared_ptr<AST> ASTVisitor::ASTPtr; virtual
ASTVisitor::ASTPtr ASTVisitor::visit(AST \*); virtual ASTVisitor::ASTPtr
ASTVisitor::visit(DataflowAPI::BottomAST \*); virtual ASTVisitor::ASTPtr
ASTVisitor::visit(DataflowAPI::ConstantAST \*); virtual
ASTVisitor::ASTPtr ASTVisitor::visit(DataflowAPI::VariableAST \*);
virtual ASTVisitor::ASTPtr ASTVisitor::visit(DataflowAPI::RoseAST \*);
virtual ASTVisitor::ASTPtr ASTVisitor::visit(StackAST \*);
