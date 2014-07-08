#ifndef BOUND_FACT_DATA_H
#define BOUND_FACT_DATA_H

#include "Absloc.h"
#include "Node.h"
#include "debug_parse.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::DataflowAPI;


typedef enum {
    Undefined, Equal, LessThan
} BoundType;

struct BoundValue {


    BoundType type;
    int64_t value;
    int coe;
    Address tableBase, targetBase;    
    bool tableLookup, tableOffset;
    bool addIndexing, addOffset;
    BoundValue(BoundType t, int64_t val): 
        type(t), value(val), coe(1), tableBase(0), targetBase(0), tableLookup(false), tableOffset(false), addIndexing(true), addOffset(true) {}
    BoundValue(): 
        type(Undefined), value(0), coe(1), tableBase(0), targetBase(0), tableLookup(false), tableOffset(false), addIndexing(true), addOffset(true) {}
    BoundValue(const BoundValue & bv):
        type(bv.type), value(bv.value), coe(bv.coe),
	tableBase(bv.tableBase), targetBase(bv.targetBase),
	tableLookup(bv.tableLookup), tableOffset(bv.tableOffset),
	addIndexing(bv.addIndexing), addOffset(bv.addOffset) {}

    BoundValue& operator = (const BoundValue &bv) {
        type = bv.type;
	value = bv.value;
	coe = bv.coe;
	tableBase = bv.tableBase;
	targetBase = bv.targetBase;
	tableLookup = bv.tableLookup;
	tableOffset = bv.tableOffset;
	addIndexing = bv.addIndexing;
	addOffset = bv.addOffset;
	return *this;
    }

    bool operator< (const BoundValue &bv) const { return value < bv.value; }
    bool operator== (const BoundValue &bv) const;
    bool CoeBounded() {return (coe == 4) || (coe == 8) || (coe == 2) || (coe == 1); }
    bool HasTableBase() {return tableBase != 0;}
    bool HasTargetBase() {return targetBase != 0; }
    void Print();
};

struct BoundFact {

    // Special fact representation for cmp table guard
    AST::Ptr cmpAST;
    uint64_t cmpBound;
    bool cmpBoundFactLive;
    set<MachRegister> cmpUsedRegs;

    typedef map<Absloc, BoundValue*> FactType;
    FactType fact;
    bool operator< (const BoundFact &bf) const {return fact < bf.fact; }
    bool operator!= (const BoundFact &bf) const;

    bool IsBounded(const Absloc &al) { return fact.find(al) != fact.end();}
    BoundValue* GetBound(const Absloc &al); 
    
    void Intersect(BoundFact &bf);

    void GenFact(const Absloc &al, BoundValue* bv);
    void KillFact(const Absloc &al);
    void Print();
    void CheckCmpValidity(const MachRegister &reg);
    bool CMPBoundMatch(AST* ast);

    BoundFact();
    ~BoundFact();

    BoundFact(const BoundFact& bf);
    BoundFact& operator = (const BoundFact &bf);
};

typedef map<Node::Ptr, BoundFact*> BoundFactsType;


#endif
