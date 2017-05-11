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

//bool PerformTableRead(StridedInterval &target, set<int64_t> & jumpTargets, CodeSource*);



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
    map<AST*, StridedInterval*> bound;
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
    StridedInterval* GetResultBound(AST::Ptr ast); 
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

    bool PotentialIndexing(AST::Ptr);
public:
    using ASTVisitor::visit;
    AbsRegion index;
    int numOfVar;
    ParseAPI::Block *b;
    bool findIncorrectFormat;
    bool findTableBase;
    bool findIndex;
    virtual ASTPtr visit(DataflowAPI::RoseAST *ast);
    virtual ASTPtr visit(DataflowAPI::VariableAST *ast);
    JumpTableFormatVisitor(ParseAPI::Block *bl);
};
#endif
