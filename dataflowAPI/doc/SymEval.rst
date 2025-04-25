\subsection{Class SymEval}
\label{sec:symeval}

\definedin{SymEval.h}

Class SymEval provides interfaces for expanding an instruction to its symbolic
expression and expanding a slice graph to symbolic expressions for all
abstract locations defined in this slice.

\begin{apient}
typedef std::map<Assignment::Ptr, AST::Ptr, AssignmentPtrValueComp> Result_t;
\end{apient}
\apidesc{This data type represents the results of symbolic expansion of a slice.
Each assignment in the slice has a corresponding AST.}

\begin{apient}
static std::pair<AST::Ptr, bool> expand(const Assignment::Ptr &assignment, 
                                        bool applyVisitors = true);
\end{apient}
\apidesc{This interface expands a single assignment given by \code{assignment}
and returns a \code{std::pair}, in which the first element is the AST after
expansion and the second element is a bool indicating whether the expansion
succeeded or not. \code{applyVisitors} specifies whether or not to perform stack
analysis to precisely track stack variables.}

\begin{apient}
static bool expand(Result_t &res, 
                   std::set<InstructionPtr> &failedInsns,
		   bool applyVisitors = true);
\end{apient}
\apidesc{This interface expands a set of assignment prepared in \code{res}. The
corresponding ASTs are written back into \code{res} and all instructions that
failed during expansion are inserted into \code{failedInsns}. \code{applyVisitors} 
specifies whether or not to perform stack analysis to precisely track stack variables. 
This function returns \code{true} when all assignments in \code{res} are
successfully expanded. }

\begin{center}
\begin{tabular}{ll}
\toprule
Retval\_t & Meaning \\
\midrule
FAILED &  failed \\
WIDEN\_NODE & widen \\
FAILED\_TRANSLATION & failed translation \\
SKIPPED\_INPUT & skipped input \\
SUCCESS & success \\
\bottomrule
\end{tabular}
\end{center}

\begin{apient}
static Retval_t expand(Dyninst::Graph::Ptr slice, DataflowAPI::Result_t &res);
\end{apient}
\apidesc{This interface expands a slice and returns an AST for each assignment in
the slice. This function will perform substitution of ASTs.}

We use an AST to represent the symbolic expressions of an assignment. A symbolic
expression AST contains internal node type \code{RoseAST}, which abstracts the
operations performed with its child nodes, and two leave node types:
\code{VariableAST} and \code{ConstantAST}. 

\code{RoseAST}, \code{VariableAST},
and \code{ConstantAST} all extend class \code{AST}. Besides the methods provided
by class \code{AST}, \code{RoseAST}, \code{VariableAST},
and \code{ConstantAST} each have a different data structure associated with
them.

\begin{apient}
Variable& VariableAST::val() const;
Constant& ConstantAST::val() const;
ROSEOperation & RoseAST::val() const;
\end{apient}

We now describe data structure \code{Variable}, \code{Constant}, and
\code{ROSEOperation}.
 
\begin{apient}
struct Variable;
\end{apient}
\apidesc{A \code{Variable} represents an abstract region at a particular
address.}

\begin{apient}
Variable::Variable();
Variable::Variable(AbsRegion r);
Variable::Variable(AbsRegion r, Address a);
\end{apient}
\apidesc{The constructors of class Variable.}

\begin{apient}
bool Variable::operator==(const Variable &rhs) const;
bool Variable::operator<(const Variable &rhs) const; 

\end{apient}
\apidesc{Two Variable objects are equal when their AbsRegion are equal and their
addresses are equal.}

\begin{apient}
const std::string Variable::format() const;
\end{apient}
\apidesc{Return the string representation of the Variable.}

\begin{apient}
AbsRegion Variable::reg;
Address Variable::addr;
\end{apient}
\apidesc{The abstraction region and the address of this Variable.}

\begin{apient}
struct Constant;
\end{apient}
\apidesc{A \code{Constant} object represents a constant value in code.}

\begin{apient}
Constant::Constant();
Constant::Constant(uint64_t v);
Constant::Constant(uint64_t v, size_t s);
\end{apient}
\apidesc{Construct Constant objects.}

\begin{apient}
bool Constant::operator==(const Constant &rhs) const;
bool Constant::operator<(const Constant &rhs) const;
\end{apient}
\apidesc{Comparison operators for Constant objects. Comparison is based on the
value and size.}

\begin{apient}
const std::string Constant::format() const;
\end{apient}
\apidesc{Return the string representation of the Constant object.}

\begin{apient}
uint64_t Constant::val;
size_t Constant::size;
\end{apient}
\apidesc{The numerical value and bit size of this value.}

\begin{apient}
struct ROSEOperation;
\end{apient}

\code{ROSEOperation} defines the following operations and we represent the
semantics of all instructions with these operations.

\begin{center}
\begin{tabular}{ll}
\toprule
ROSEOperation::Op & Meaning \\
\midrule
    nullOp  & No operation \\
    extractOp & Extract bit ranges from a value \\
    invertOp & Flip every bit \\
    negateOp & Negate the value \\
    signExtendOp & Sign-extend the value \\
    equalToZeroOp & Check whether the value is zero or not \\
    generateMaskOp & Generate mask \\
    LSBSetOp & LSB set op\\
    MSBSetOp & MSB set op \\
    concatOp & Concatenate two values to form a new value \\
    andOp & Bit-wise and operation \\
    orOp & Bit-wise or operation \\
    xorOp & Bit-wise xor operation \\
    addOp & Add operation \\
    rotateLOp & Rotate to left operation \\
    rotateROp & Rotate to right operation \\
    shiftLOp & Shift to left operation \\
    shiftROp & Shift to right operation \\
    shiftRArithOp & Arithmetic shift to right operation \\
    derefOp  & Dereference memory operation \\
    writeRepOp & Write rep operation\\
    writeOp & Write operation\\
    ifOp & If operation \\
    sMultOp & Signed multiplication operation \\
    uMultOp & Unsigned multiplication operation \\
    sDivOp & Signed division operation \\
    sModOp & Signed modular operation \\
    uDivOp & Unsigned division operation \\    
    uModOp & Unsigned modular operation \\
    extendOp & Zero extend operation \\
    extendMSBOp & Extend the most significant bit operation \\
\bottomrule
\end{tabular}
\end{center}

\begin{apient}
ROSEOperation::ROSEOperation(Op o) : op(o);
ROSEOperation::ROSEOperation(Op o, size_t s);
\end{apient}
\apidesc{Constructors for ROSEOperation}

\begin{apient}
bool ROSEOperation::operator==(const ROSEOperation &rhs) const;
\end{apient}
\apidesc{Equal operator}

\begin{apient}
const std::string ROSEOperation::format() const;
\end{apient}
\apidesc{Return the string representation.}

\begin{apient}
ROSEOperation::Op ROSEOperation::op;
size_t ROSEOperation::size;
\end{apient}

\subsection{Class ASTVisitor}

The ASTVisitor class defines callback functions to apply during visiting an AST for
each AST node type. Users can inherit from this class to write customized analyses
for ASTs.

\begin{apient}
typedef boost::shared_ptr<AST> ASTVisitor::ASTPtr;
virtual ASTVisitor::ASTPtr ASTVisitor::visit(AST *);
virtual ASTVisitor::ASTPtr ASTVisitor::visit(DataflowAPI::BottomAST *);
virtual ASTVisitor::ASTPtr ASTVisitor::visit(DataflowAPI::ConstantAST *);
virtual ASTVisitor::ASTPtr ASTVisitor::visit(DataflowAPI::VariableAST *);
virtual ASTVisitor::ASTPtr ASTVisitor::visit(DataflowAPI::RoseAST *);
virtual ASTVisitor::ASTPtr ASTVisitor::visit(StackAST *);
\end{apient}
\apidesc{Callback functions for visiting each type of AST node. The default
behavior is to return the input parameter.}

