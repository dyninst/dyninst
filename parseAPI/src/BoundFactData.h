#ifndef BOUND_FACT_DATA_H
#define BOUND_FACT_DATA_H

#include "Absloc.h"
#include "Node.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::DataflowAPI;


typedef enum {
    Equal, LessThan
} BoundType;

struct BoundValue {


    BoundType type;
    uint64_t value;
    int coe;
    uint64_t tableBase, targetBase;
    bool tableLookup, tableOffset, posi;
    BoundValue(BoundType t, uint64_t val, int c, uint64_t tableB, uint64_t tarB, bool tl, bool to, bool p = true): 
        type(t), value(val), coe(c), tableBase(tableB), targetBase(tarB), tableLookup(tl), tableOffset(to), posi(p) {}
    BoundValue(): 
        type(Equal), value(0), coe(1), tableBase(0), targetBase(0), tableLookup(false), tableOffset(false), posi(false) {}

    bool operator< (const BoundValue &bv) const { return value < bv.value; }
    bool operator== (const BoundValue &bv) const;
    bool CoeBounded() {return (coe == 4) || (coe == 8); }
    bool HasTableBase() {return tableBase != 0;}
    bool HasTargetBase() {return targetBase != 0; }
};

struct BoundFact {

    // Special fact representation for cmp table guard
    AST::Ptr cmpAST;
    uint64_t cmpBound;
    bool cmpBoundFactLive;
    set<MachRegister> cmpUsedRegs;

    typedef map<Absloc, BoundValue > FactType;
    FactType fact;
    bool operator< (const BoundFact &bf) const {return fact < bf.fact; }
    bool operator!= (const BoundFact &bf) const;

    bool IsBounded(const Absloc &al) { return fact.find(al) != fact.end();}
    BoundValue GetBound(const Absloc &al); 
    
    void Intersect(BoundFact &bf);

    void GenFact(const Absloc &al, BoundValue bv);
    void KillFact(const Absloc &al);
    void Print();
    void CheckCmpValidity(const MachRegister &reg);
    bool CMPBoundMatch(AST* ast);
};

typedef map<Node*, BoundFact> BoundFactsType;


#endif
