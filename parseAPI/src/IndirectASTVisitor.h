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

//bool PerformTableRead(StridedInterval &target, set<int64_t> & jumpTargets, CodeSource*);

#define SIGNEX_64_32 0xffffffff00000000LL
#define SIGNEX_64_16 0xffffffffffff0000LL
#define SIGNEX_64_8  0xffffffffffffff00LL
#define SIGNEX_32_16 0xffff0000
#define SIGNEX_32_8 0xffffff00



class SimplifyVisitor: public Dyninst::ASTVisitor {
    Dyninst::Address addr;
    bool keepMultiOne;
    SymbolicExpression &se;
public:
    using Dyninst::ASTVisitor::visit;
    virtual ASTPtr visit(Dyninst::DataflowAPI::RoseAST *ast);
    SimplifyVisitor(Dyninst::Address a, bool k, SymbolicExpression &sym): addr(a), keepMultiOne(k), se(sym) {}
};


class BoundCalcVisitor: public Dyninst::ASTVisitor {
     
public:
    using Dyninst::ASTVisitor::visit;
    map<AST*, StridedInterval*> bound;
    BoundFact &boundFact;
    Dyninst::ParseAPI::Block *block;
    bool handleOneByteRead;
    int derefSize;

    BoundCalcVisitor(BoundFact &bf, Dyninst::ParseAPI::Block* b, bool handle, int size): 
        boundFact(bf), block(b), handleOneByteRead(handle), derefSize(size) {}
    ~BoundCalcVisitor();
    virtual ASTPtr visit(Dyninst::DataflowAPI::RoseAST *ast);
    virtual ASTPtr visit(Dyninst::DataflowAPI::ConstantAST *ast);
    virtual ASTPtr visit(Dyninst::DataflowAPI::VariableAST *ast);
    bool IsResultBounded(AST::Ptr ast) {
        return bound.find(ast.get()) != bound.end();
    }
    StridedInterval* GetResultBound(AST::Ptr ast); 
};

class JumpCondVisitor: public Dyninst::ASTVisitor {

public:
    using Dyninst::ASTVisitor::visit;
    bool invertFlag;
    virtual ASTPtr visit(Dyninst::DataflowAPI::RoseAST *ast);
    JumpCondVisitor() : invertFlag(false) {}
};

class ComparisonVisitor: public Dyninst::ASTVisitor {

public:
    using Dyninst::ASTVisitor::visit;
    AST::Ptr subtrahend, minuend;
    virtual ASTPtr visit(Dyninst::DataflowAPI::RoseAST *ast);

    ComparisonVisitor(): subtrahend(AST::Ptr()), minuend(AST::Ptr()) {} 
};

class JumpTableFormatVisitor: public Dyninst::ASTVisitor {

    bool PotentialIndexing(AST::Ptr);
public:
    using Dyninst::ASTVisitor::visit;
    Dyninst::AbsRegion index;
    int numOfVar;
    int memoryReadLayer;
    Dyninst::ParseAPI::Block *b;
    bool findIncorrectFormat;
    bool findTableBase;    
    bool findIndex;
    bool firstAdd;
    virtual ASTPtr visit(Dyninst::DataflowAPI::RoseAST *ast);
    virtual ASTPtr visit(Dyninst::DataflowAPI::VariableAST *ast);
    JumpTableFormatVisitor(Dyninst::ParseAPI::Block *bl);
};

class JumpTableReadVisitor: public Dyninst::ASTVisitor {
public:
    using Dyninst::ASTVisitor::visit;
    Dyninst::AbsRegion index;
    int64_t indexValue;
    Dyninst::ParseAPI::CodeSource* cs;
    Dyninst::ParseAPI::CodeRegion* cr;
    Dyninst::Address targetAddress;
    Dyninst::Address readAddress;
    int memoryReadSize;
    bool valid;
    bool isZeroExtend;


    // This tracks the results of computation for each sub AST
    map<AST*, int64_t> results;
    JumpTableReadVisitor(Dyninst::AbsRegion i, int v, Dyninst::ParseAPI::CodeSource *c, Dyninst::ParseAPI::CodeRegion *r, bool ze, int m);
    virtual ASTPtr visit(Dyninst::DataflowAPI::RoseAST *ast);
    virtual ASTPtr visit(Dyninst::DataflowAPI::ConstantAST *ast);
    virtual ASTPtr visit(Dyninst::DataflowAPI::VariableAST *ast);
    bool PerformMemoryRead(Dyninst::Address addr, int64_t &v);
};
#endif
