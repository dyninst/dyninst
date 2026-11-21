#ifndef BOUND_FACT_DATA_H
#define BOUND_FACT_DATA_H

#include "Absloc.h"
#include "Node.h"
#include "debug_parse.h"

#include "Instruction.h"
#include "CFG.h"
#include "entryIDs.h"

#include <climits>
#include <map>
#include <stdint.h>
#include <string>
#include <utility>
using namespace std;

struct StridedInterval {   
    static const int64_t minValue = LLONG_MIN;
    static const int64_t maxValue = LLONG_MAX;
    static const StridedInterval top;
    static const StridedInterval bottom;
    // stride < 0: bottom (empty set)
    // stride = 0: represent a constant
    // stride > 0: represent an interval
    int64_t stride;
    int64_t low, high;

    // Bottom: empty set
    StridedInterval(): stride(-1), low(0), high(0) {}

    // Construct a constant
    StridedInterval(int64_t x): stride(0), low(x), high(x) {} 

    // Construct an interval
    StridedInterval(unsigned s, int64_t l, int64_t h):
        stride(s), low(l), high(h) {}

    StridedInterval(const StridedInterval &si):
        stride(si.stride), low(si.low), high(si.high) {}

    // Meet is point-wise union
    void Join(const StridedInterval &rhs);

    void Neg();
    void Not();

    void Add(const StridedInterval &rhs);
    void Sub(const StridedInterval &rhs);
    void And(const StridedInterval &rhs);
    void Or(const StridedInterval &rhs);
    void Xor(const StridedInterval &rhs);
    void Mul(const StridedInterval &rhs); 
    void Div(const StridedInterval &rhs);
    void ShiftLeft(const StridedInterval &rhs);
    void ShiftRight(const StridedInterval &rhs);
    std::string format();
    void Print();

    StridedInterval & operator = (const StridedInterval &rhs);
    bool operator == (const StridedInterval &rhs) const;
    bool operator != (const StridedInterval &rhs) const;
    bool operator < (const StridedInterval &rhs) const;

    void Intersect(StridedInterval &rhs);
    void DeleteElement(int64_t val);

    uint64_t size() const;
    bool IsConst(int64_t v) const;
    bool IsConst() const;
};

struct BoundFact {
    typedef map<Dyninst::AST::Ptr, StridedInterval*> FactType;
    FactType fact;

    // Sometimes the bound of a jump table index are derived from 
    // the difference between two values. In this case, it is useful
    // to know that whether there is a certain relation between the two values
    typedef enum {
        Equal,
	NotEqual, 
	UnsignedLessThan,
	UnsignedLargerThan,
	UnsignedLessThanOrEqual,
	UnsignedLargerThanOrEqual,
	SignedLessThan,
	SignedLargerThan,
	SignedLessThanOrEqual,
	SignedLargerThanOrEqual,
    } RelationType;

    struct Relation {
        Dyninst::AST::Ptr left;
	Dyninst::AST::Ptr right;
	RelationType type;
	Relation(Dyninst::AST::Ptr l, Dyninst::AST::Ptr r, RelationType t):
	    left(l), right(r), type(t) {}

        bool operator != (const Relation &rhs) const {
	    if (type != rhs.type) return true;
	    if (!(*left == *rhs.left)) return true;
	    if (!(*right == *rhs.right)) return true;
	    return false;
	}

	Relation& operator = (const Relation &rhs) {
	    left = rhs.left;
	    right = rhs.right;
	    type = rhs.type;
	    return *this;
	}

	Relation(const Relation &r) { *this = r; }
    };

    vector<Relation*> relation;

    // We need to track aliases of each register and memory locations.
    // The left hand side represents an abstract location at the current address
    // and the right hand side represents an AST of input absloc locations.
    // eax at the current location can be different from eax at the input absloc location
    //
    // Register abstract location with address 0 represents an absloc at the current address
    // Register abstract location with address 1 represents an input absloc
    typedef std::map<Dyninst::AST::Ptr, Dyninst::AST::Ptr> AliasMap;
    AliasMap aliasMap;

    struct FlagPredicate {
        bool valid;
	entryID id;
	Dyninst::AST::Ptr e1;
	Dyninst::AST::Ptr e2;

	FlagPredicate():
	    valid(false),
	    id( _entry_ids_max_),
	    e1(),
	    e2() {}
	bool operator != (const FlagPredicate& fp) const {
	    if (!valid && !fp.valid) return false;
	    if (valid != fp.valid) return true;
	    if (id != fp.id) return true;
	    if ((!(*e1 == *fp.e1) || !(*e2 == *fp.e2)) && (!(*e1 == *fp.e2) || !(*e2 == *fp.e1))) return true;
	    return false;
	}

	FlagPredicate& operator = (const FlagPredicate &fp) {
	    valid = fp.valid;
	    if (valid) {
		e1 = fp.e1;
	        e2 = fp.e2;
		id = fp.id;
	    }
	    return *this;
	}
    } pred;

    struct StackTop {
        int64_t value{};
	bool valid;

	StackTop(): valid(false) {}
	StackTop(int64_t v): value(v), valid(true) {}
	bool operator != (const StackTop &st) const {
	    if (!valid && !st.valid) return false;
	    if (valid != st.valid) return true;
	    return value != st.value;
	}

	StackTop& operator = (const StackTop &st) {
	    valid = st.valid;
	    if (valid) value = st.value;
	    return *this;
	}
    } stackTop;

    bool operator< (const BoundFact &bf) const {return fact < bf.fact; }
    bool operator!= (const BoundFact &bf) const;

    StridedInterval* GetBound(const Dyninst::AST::Ptr ast); 
    StridedInterval* GetBound(const Dyninst::AST* ast);
    Dyninst::AST::Ptr GetAlias(const Dyninst::AST::Ptr ast);
    void Meet(BoundFact &bf);


    bool ConditionalJumpBound(Dyninst::InstructionAPI::Instruction insn, Dyninst::ParseAPI::EdgeTypeEnum type);
    void SetPredicate(Dyninst::Assignment::Ptr assign, std::pair<Dyninst::AST::Ptr, bool> expand);
    void GenFact(const Dyninst::AST::Ptr ast, StridedInterval* bv, bool isConditionalJump);
    void KillFact(const Dyninst::AST::Ptr ast, bool isConditionalJump);
    void SetToBottom();
    void Print();
    void AdjustPredicate(Dyninst::AST::Ptr out, Dyninst::AST::Ptr in);

    void IntersectInterval(const Dyninst::AST::Ptr ast, StridedInterval si);
    void DeleteElementFromInterval(const Dyninst::AST::Ptr ast, int64_t val);
    void InsertRelation(Dyninst::AST::Ptr left, Dyninst::AST::Ptr right, RelationType);
    void TrackAlias(Dyninst::AST::Ptr expr, Dyninst::AST::Ptr outAST, bool findBound);

    StridedInterval *ApplyRelations(Dyninst::AST::Ptr outAST);
    StridedInterval *ApplyRelations2(Dyninst::AST::Ptr outAST);

    void PushAConst(int64_t value);
    bool PopAConst(Dyninst::AST::Ptr ast);
    
    void SwapFact(Dyninst::AST::Ptr a, Dyninst::AST::Ptr b);

    BoundFact();
    ~BoundFact();

    BoundFact(const BoundFact& bf);
    BoundFact& operator = (const BoundFact &bf);
};

typedef map<Dyninst::Node::Ptr, BoundFact*> BoundFactsType;


#endif
