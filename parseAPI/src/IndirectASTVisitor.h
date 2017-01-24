#ifndef INDIRECT_AST_VISITOR_H
#define INDIRECT_AST_VISITOR_H

#include <set>

#include "DynAST.h"
#include "SymEval.h"
#include "CodeSource.h"
#include "BoundFactData.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::DataflowAPI;

AST::Ptr SimplifyRoot(AST::Ptr ast, Address addr);
AST::Ptr SimplifyAnAST(AST::Ptr ast, Address addr);
AST::Ptr SubstituteAnAST(AST::Ptr ast, const BoundFact::AliasMap &aliasMap);
AST::Ptr DeepCopyAnAST(AST::Ptr ast);
bool ContainAnAST(AST::Ptr root, AST::Ptr check);
bool PerformTableRead(BoundValue &target, set<int64_t> & jumpTargets, boost::shared_ptr<CodeSource>);


// On x86 and x86-64, the value of PC is post-instruction, 
// which is the current address plus the length of the instruction.
// On ARMv8, the value of PC is pre-instruction,
// which is the current address
Address PCValue(Address cur, size_t insnSize, Architecture a);

class SimplifyVisitor: public ASTVisitor {
    Address addr;
public:
    using ASTVisitor::visit;
    virtual ASTPtr visit(DataflowAPI::RoseAST *ast);
    SimplifyVisitor(Address a): addr(a) {}
};



class BoundCalcVisitor: public ASTVisitor {
     
public:
    using ASTVisitor::visit;
    map<AST*, BoundValue*> bound;
    BoundFact &boundFact;
    ParseAPI::Block *block;
    bool handleOneByteRead;
    int derefSize;

    BoundCalcVisitor(BoundFact &bf, ParseAPI::Block* b, bool handle, int size): 
        boundFact(bf), block(b), handleOneByteRead(handle), derefSize(size) {}
    ~BoundCalcVisitor();
    virtual ASTPtr visit(DataflowAPI::RoseAST *ast);
    virtual ASTPtr visit(DataflowAPI::ConstantAST *ast);
    virtual ASTPtr visit(DataflowAPI::VariableAST *ast);
    bool IsResultBounded(AST::Ptr ast) {
        return bound.find(ast.get()) != bound.end();
    }
    BoundValue* GetResultBound(AST::Ptr ast); 
};

class JumpCondVisitor: public ASTVisitor {

public:
    using ASTVisitor::visit;
    bool invertFlag;
    virtual ASTPtr visit(DataflowAPI::RoseAST *ast);
    JumpCondVisitor() : invertFlag(false) {}
};

class ComparisonVisitor: public ASTVisitor {

public:
    using ASTVisitor::visit;
    AST::Ptr subtrahend, minuend;
    virtual ASTPtr visit(DataflowAPI::RoseAST *ast);

    ComparisonVisitor(): subtrahend(AST::Ptr()), minuend(AST::Ptr()) {} 
};

class JumpTableFormatVisitor: public ASTVisitor {

public:
    using ASTVisitor::visit;
    ParseAPI::Block *b;
    bool format;
    virtual ASTPtr visit(DataflowAPI::RoseAST *ast);
    JumpTableFormatVisitor(ParseAPI::Block *bl): b(bl), format(true) {}
};
#endif
