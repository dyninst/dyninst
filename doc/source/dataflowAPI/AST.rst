.. _sec:ast:

Class AST
---------

We provide a generic AST framework to represent tree structures. One
example use case is to represent instruction semantics with symbolic
expressions. The AST framework includes the base class definitions for
tree nodes and visitors. Users can inherit tree node classes to create
their own AST structure and AST visitors to write their own analyses for
the AST.

All AST node classes should be derived from the AST class. Currently we
have the following types of AST nodes.

============= ======================
AST::ID       Meaning
============= ======================
V_AST         Base class type
V_BottomAST   Bottom AST node
V_ConstantAST Constant AST node
V_VariableAST Variable AST node
V_RoseAST     ROSEOperation AST node
V_StackAST    Stack AST node
============= ======================

typedef boost::shared_ptr<AST> Ptr;

typedef std::vector<AST::Ptr> Children;

bool operator==(const AST &rhs) const; bool equals(AST::Ptr rhs);

virtual unsigned numChildren() const;

virtual AST::Ptr child(unsigned i) const;

virtual const std::string format() const = 0;

static AST::Ptr substitute(AST::Ptr in, AST::Ptr a, AST::Ptr b);

virtual AST::ID AST::getID() const;

virtual Ptr accept(ASTVisitor \*v);

virtual void AST::setChild(int i, AST::Ptr c);
