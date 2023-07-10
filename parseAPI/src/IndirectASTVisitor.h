#ifndef INDIRECT_AST_VISITOR_H
#define INDIRECT_AST_VISITOR_H

#include <stdint.h>
#include <set>

#include "DynAST.h"
#include "SymEval.h"
#include "CodeSource.h"
#include "BoundFactData.h"
#include "SymbolicExpression.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::DataflowAPI;

//bool PerformTableRead(StridedInterval &target, set<int64_t> & jumpTargets, CodeSource*);

#define SIGNEX_64_32 0xffffffff00000000LL
#define SIGNEX_64_16 0xffffffffffff0000LL
#define SIGNEX_64_8  0xffffffffffffff00LL
#define SIGNEX_32_16 0xffff0000
#define SIGNEX_32_8 0xffffff00



class SimplifyVisitor: public ASTVisitor {
    Address addr;
    bool keepMultiOne;
    SymbolicExpression &se;
public:
    using ASTVisitor::visit;
    virtual ASTPtr visit(DataflowAPI::RoseAST *ast);
    SimplifyVisitor(Address a, bool k, SymbolicExpression &sym): addr(a), keepMultiOne(k), se(sym) {}
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
    int memoryReadLayer;
    ParseAPI::Block *b;
    bool findIncorrectFormat;
    bool findTableBase;    
    bool findIndex;
    bool firstAdd;
    virtual ASTPtr visit(DataflowAPI::RoseAST *ast);
    virtual ASTPtr visit(DataflowAPI::VariableAST *ast);
    JumpTableFormatVisitor(ParseAPI::Block *bl);
};

class JumpTableReadVisitor: public ASTVisitor {
public:
    using ASTVisitor::visit;
    AbsRegion index;
    int64_t indexValue;
    CodeSource* cs;
    CodeRegion* cr;
    Address targetAddress;
    Address readAddress;
    int memoryReadSize;
    bool valid;
    bool isZeroExtend;


    // This tracks the results of computation for each sub AST
    map<AST*, int64_t> results;
    JumpTableReadVisitor(AbsRegion i, int v, CodeSource *c, CodeRegion *r, bool ze, int m);
    virtual ASTPtr visit(DataflowAPI::RoseAST *ast);
    virtual ASTPtr visit(DataflowAPI::ConstantAST *ast);
    virtual ASTPtr visit(DataflowAPI::VariableAST *ast);
    bool PerformMemoryRead(Address addr, int64_t &v);
};
#endif
