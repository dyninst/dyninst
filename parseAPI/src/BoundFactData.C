#include "dyntypes.h"
#include "BoundFactData.h"
#include "debug_parse.h"
#include "Instruction.h"
#include "IndirectASTVisitor.h"
#include "SymEval.h"
#include "CodeSource.h"
#include "CodeObject.h"
#include "CFG.h"
#include "SymbolicExpression.h"
#include <iostream>

#define MAX_TABLE_ENTRY 1000000

using namespace Dyninst::InstructionAPI;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::DataflowAPI;

const StridedInterval StridedInterval::top = StridedInterval(1, StridedInterval::minValue, StridedInterval::maxValue);
const StridedInterval StridedInterval::bottom = StridedInterval();

// Greatest common divisor
static int64_t GCD(int64_t a, int64_t b) {
    if (a == 0 && b == 0) return 0;
    else if (a == 0) return b;
    else if (b == 0) return a;
    int64_t r = a % b;
    while (r != 0) {
        a = b;
	b = r;
	r = a % b;
    }
    return b;
}

// Find out x and y such that
// ax + by = gcd(a,b)
// Check http://en.wikipedia.org/wiki/Extended_Euclidean_algorithm
static int64_t ExtendEuclidean(int64_t a, int64_t b, int64_t &x, int64_t &y) {
    int64_t old_x, old_y, r, old_r, q, tmp;
    x = 0; y = 1; r = b;
    old_x = 1; old_y = 0; old_r = a;
    while (r != 0) {
        q = old_r / r;

	tmp = old_r;
	old_r = r;
	r = tmp - q * r;

	tmp = old_x;
	old_x = x;
	x = tmp - q * x;

	tmp = old_y;
	old_y = y;
	y = tmp - q * y;
    }
    x = old_x;
    y = old_y;
    return old_r;
}

void StridedInterval::Join(const StridedInterval &rhs) {
    // Union of two intervals
    if (*this == top ) return;
    if (rhs == top) {
        *this = top;
	return;
    }
    if (stride < 0 && rhs.stride < 0) {
        // Bottom
        stride = -1;
	low = high = 0;
    } else if (stride < 0) {
        *this = rhs;
    } else if (rhs.stride < 0) {
        // Anything joining bottom stays unchanged
    } else {
        set<int64_t> values;
	values.insert(low);
	values.insert(high);
	values.insert(rhs.low);
	values.insert(rhs.high);
	// if values.size() == 1, then both intervals are the
	// same constants. Nothing needs to be done
	if (values.size() != 1) {
	    // Otherwise, new stride is GCD(stride, rhs.stride, |low - rhs.low|)
	    int64_t newStride = GCD(stride, rhs.stride);
	    set<int64_t>::iterator cur, next;
	    for (cur = values.begin(), next = values.begin(), ++next; next != values.end(); ++cur, ++next)
	        newStride = GCD(newStride, *next - *cur);
	    low = *(values.begin());
	    high = *cur;
	    stride = newStride;
	}	
    }
}

void StridedInterval::Neg() {
    if (stride < 0) return;
    int64_t tmpLow = low;
    low = -high;
    high = -tmpLow;
}

void StridedInterval::Not() {
    if (stride < 0) return;
    // Assume not is always used to 
    // calculates its two's complement to 
    // do a subtraction
    int64_t tmpLow = low;
    low = -high-1;
    high = -tmpLow-1;

}

void StridedInterval::Add(const StridedInterval &rhs) {
    // It is not clear what it means to 
    // add a number with a number in empy set.
    // Assume to be bottom
    if (stride < 0 || rhs.stride < 0) {
        *this = bottom;
	return;
    }
    // If any of the interval is top,
    // then the result of the add can take
    // any value, thus is still top
    if (*this == top || rhs == top) {
        *this = top;
	return;
    }

    // Assume no arithmetic overflow.
    // May need to rewite this
    stride = GCD(stride, rhs.stride);
    low += rhs.low;
    high += rhs.high;

    // This is likely to be caused by arithmetic overflow.
    // In this case, a strided interval cannot accurately 
    // capture the value range. Set to top.
    if (low > high) {
        *this = top;
    }
}

void StridedInterval::Sub(const StridedInterval& minuend) {
   StridedInterval neg(minuend);
   neg.Neg();
   Add(neg);
}

void StridedInterval::And(const StridedInterval &rhs) {
    // Currently only consider the case where at least one of them is constant
    if (stride == 0) {
       // CONSTANT and any thing ==> 1[0, CONSTANT]
       low = 0;
       stride = 1;
    } else if (rhs.stride == 0) {
       stride = 1;
       low = 0;
       high = rhs.low;
    } else {
        // Otherwise, widen
	*this = top;
    }
}

void StridedInterval::Or(const StridedInterval &rhs) {
    // consider
    // case 1: one of them is 1[0, high], the other is a constant
    // case 2: both are 1[0, high]
    if (stride == 0 && rhs.stride == 1 && rhs.low == 0) {
        stride = 1;
	low = 0;
	high |= rhs.high;
    } else if (stride == 1 && low == 0 && rhs.stride == 0) {
        high |= rhs.high;
    } else if (stride == 1 && low == 0 && rhs.stride == 1 && rhs.low == 0) {
        high |= rhs.high;
    } else {
        // Otherwise, widen
	*this = top;
    }
}

void StridedInterval::Xor(const StridedInterval&) {
    parsing_printf("StridedInterval::Xor: not implemented, widen\n");
    *this = top;
}

void StridedInterval::Mul(const StridedInterval &rhs) {
    // Currently only handle the case where
    // one of the interval is a constant
    if (stride == 0 || rhs.stride == 0) {
        if (stride == 0) {
	    int64_t val = low;
	    low = rhs.low * val;
	    high = rhs.high * val;
	    stride = rhs.stride * val;
	} else {
	    int64_t val = rhs.low;
	    low = low * val;
	    high = high * val;
	    stride = stride * val;
	}
	// If the constant value is negative,
	// we need to swap the lower bound and the upper bound
	if (stride < 0) {
	    stride = -stride;
	    int64_t tmp = low;
	    low = high;
	    high = tmp;
	}
    } else {
        // In other case, we widen
        *this = top;
    }
}

void StridedInterval::Div(const StridedInterval &rhs) {
    if (rhs.stride == 0) {
        low /= rhs.low;
	high /= rhs.low;
	stride /= rhs.low;
	if (stride == 0 && low != high) stride = 1;
    } else {
        *this = top;
    }
}

void StridedInterval::ShiftLeft(const StridedInterval &rhs) {
    if (rhs.stride == 0) {
        Mul(StridedInterval(1 << rhs.low));
    } else {
        // In other case, we widen
	*this = top;
    }
}
void StridedInterval::ShiftRight(const StridedInterval &rhs) {
    if (rhs.stride == 0) {
        Div(StridedInterval(1 << rhs.low));
    } else {
        // In other case, we widen
	*this = top;
    }
}
StridedInterval & StridedInterval::operator = (const StridedInterval &rhs) {
    stride = rhs.stride;
    low = rhs.low;
    high = rhs.high;
    return *this;
}


bool StridedInterval::operator == (const StridedInterval &rhs) const {
    if (stride != rhs.stride) return false;
    if (low != rhs.low) return false;
    if (high != rhs.high) return false;
    return true;
}

bool StridedInterval::operator != (const StridedInterval &rhs) const {
    return !(*this == rhs);
}


bool StridedInterval:: operator < (const StridedInterval &rhs) const {
    if (stride != rhs.stride) return stride < rhs.stride;
    if (low != rhs.low) return low < rhs.low;
    return high < rhs.high;
}

void StridedInterval::Intersect(StridedInterval &rhs) {
    if (stride == 0) {
        if (rhs.stride == 0) {
	    if (low != rhs.low) {
	        // Intersect two different constants,
		// we get an empty set
		*this = bottom;
	        parsing_printf("Interval set to bottom!\n");
	    }
	} else {
	    if (low < rhs.low || low > rhs.high || (low - rhs.low) % rhs.stride != 0) {
	        // The constant is not in the rhs's interval
	        *this = bottom;
		parsing_printf("Interval set to bottom!\n");
	    }
	}
    } else {
        if (rhs.stride == 0) {
	    if (low > rhs.low || high < rhs.low  || ((rhs.low - low) % stride != 0)) {
	        // The rhs is a constant and it is not in this interval
		*this = bottom;
		parsing_printf("Interval set to bottom!\n");
	    }
	    else {
	        // The rhs is a constant and it is in this interval,
		// then rhs is the result of the intersection
	        *this = rhs;
	    }
	} else {
	    // Both are intervals
	    int64_t gcd = GCD(stride, rhs.stride);
	    if (low > rhs.high || high < rhs.low || (low - rhs.low) % gcd != 0) {
	        *this = bottom;		
	    } else {
		int64_t x, k1, k2;
		ExtendEuclidean(stride / gcd, rhs.stride / gcd, k1, k2);
	        x = (low - rhs.low) / gcd;
		k1 *= x;
		k2 *= -x;

	    int64_t newStride = stride * rhs.stride / GCD(stride, rhs.stride);
		int64_t newLow = low + k1 * stride, newHigh = high;
		if (newLow < low) newLow += ((low - newLow - 1) / newStride + 1) * newStride;
		if (newLow < rhs.low) newLow += ((rhs.low - newLow - 1) / newStride + 1) * newStride;
		if (rhs.high < newHigh) newHigh = rhs.high;
		if (newHigh < newLow) {
		    *this = bottom;
		} else {
		    newHigh -= (newHigh - newLow) % newStride;
		    stride = newStride;
		    low = newLow;
		    high = newHigh;
		}
	    }
	}
    }
}

void StridedInterval::DeleteElement(int64_t val) {
    // Need to figure out a general way to calculate it
    if (val == low) low += stride;
    if (val == high) low -= stride;
}

string StridedInterval::format() {
    stringstream o;
	o << stride << "[" << low << "," << high << "] ";
	o << hex << stride << "[" << low << "," << high << "]";
    return o.str();
}

uint64_t StridedInterval::size() const {
    if (stride < 0) return 0;
    else if (stride == 0) return 1;
    else return (high - low) / stride;
}

bool StridedInterval::IsConst(int64_t v) const {
    return (stride == 0) && (low == v) && (high == v);
}

bool StridedInterval::IsConst() const{
    return stride == 0;
}

void StridedInterval::Print() {
    parsing_printf("%s\n", format().c_str());
}

void BoundFact::Meet(BoundFact &bf) {
        for (auto fit = fact.begin(); fit != fact.end();) {
	    StridedInterval *val2 = bf.GetBound(fit->first);
	    // if ast fit->first cannot be found in bf,
	    // then fit->first is top on this path.
	    // Anything joins top becomes top
	    if (val2 != NULL) {
	        StridedInterval *val1 = fit->second;
		val1->Join(*val2);
		++fit;
	    } else {
	        auto toErase = fit;
		++fit;
		if (toErase->second != NULL) delete toErase->second;
		fact.erase(toErase);
	    }
	}

	// Meet the relation vector 	
	for (auto rit = relation.begin(); rit != relation.end();) {
	    bool find = false;
	    for (auto it = bf.relation.begin(); it != bf.relation.end(); ++it) {
	        if (*((*rit)->left) == *((*it)->left) && *((*rit)->right) == *((*it)->right) && (*rit)->type == (*it)->type) {
		    find = true;
		    break;
		}
		if (*((*rit)->right) == *((*it)->left) && *((*rit)->left) == *((*it)->right)) {
		   if (((*rit)->type ^ (*it)->type) == 1 && (*it)->type != Equal && (*it)->type != NotEqual) {
		       find = true;
		       break;
		   }
		}

            }
	    if (!find) {
		if (*rit != NULL) delete *rit;
	        rit = relation.erase(rit);
	    } else ++rit;
	}

	// Meet the flag predicate
	if (pred != bf.pred) pred.valid = false;

	// Meet the alias map
	for (auto ait = aliasMap.begin(); ait != aliasMap.end(); ) {
	    auto bit = bf.aliasMap.find(ait->first);
	    if (bit == bf.aliasMap.end() || !(*(ait->second) == *(bit->second))) {
	        auto toErase = ait;
		++ait;
		aliasMap.erase(toErase);
	    } else {
	        ++ait;
	    }
	}

	// Meet the stack top
	if (stackTop != bf.stackTop) stackTop.valid = false;
}

void BoundFact::Print() {
    if (pred.valid) {
        parsing_printf("\t\t\tCurrent predicate:");
	parsing_printf(" element 1 is %s;", pred.e1->format().c_str());
	parsing_printf(" element 2 is %s\n", pred.e2->format().c_str());
    } else
        parsing_printf("\t\t\tDo not track predicate\n");
    for (auto fit = fact.begin(); fit != fact.end(); ++fit) {
        parsing_printf("\t\t\tVar: %s, ", fit->first->format().c_str());
	fit->second->Print();
    }
    parsing_printf("\t\t\tRelations:\n");
    for (auto rit = relation.begin(); rit != relation.end(); ++rit) {
        parsing_printf("\t\t\t\t%s and %s, relation: %d\n", (*rit)->left->format().c_str(), (*rit)->right->format().c_str(), (*rit)->type);
    }
    parsing_printf("\t\t\tAliasing:\n");
    for (auto ait = aliasMap.begin(); ait != aliasMap.end(); ++ait) {
        parsing_printf("\t\t\t\t%s = %s\n", ait->first->format().c_str(), ait->second->format().c_str());
    }
    if (stackTop.valid) {
        parsing_printf("\t\t\tStack top is %ld\n", stackTop.value);
    } else {
        parsing_printf("\t\t\tNo known value at the top of the stack\n");
    }
}

void BoundFact::GenFact(const AST::Ptr ast, StridedInterval* bv, bool isConditionalJump) {
    bv->Print();
    KillFact(ast, isConditionalJump);
    fact.insert(make_pair(ast, bv));
    for (auto rit = relation.begin(); rit != relation.end(); ++rit) {
	if ((*rit)->type == Equal) {
	    if (*((*rit)->left) == *ast) {
	        KillFact((*rit)->right, isConditionalJump);
	        fact.insert(make_pair((*rit)->right, new StridedInterval(*bv)));
	    }
	    if (*((*rit)->right) == *ast) {
	        KillFact((*rit)->left, isConditionalJump);
	        fact.insert(make_pair((*rit)->left, new StridedInterval(*bv))); 
	    }
	}
    }

    // Only check alias for bound produced by conditinal jumps.
    if (isConditionalJump) {
        AST::Ptr subAST = SymbolicExpression::DeepCopyAnAST(ast);
	parsing_printf("Before substitute %s\n", ast->format().c_str());
	subAST = SymbolicExpression::SubstituteAnAST(subAST, aliasMap);
	parsing_printf("After  substitute %s\n", subAST->format().c_str());
	if (!(*subAST == *ast)) {
	    KillFact(subAST, true);
	    fact.insert(make_pair(subAST, new StridedInterval(*bv)));

	}
    }
}

/*
static bool IsSubTree(AST::Ptr tree, AST::Ptr sub) {
    if (*tree == *sub) return true;
    bool ret = false;
    unsigned totalChildren = tree->numChildren();
    for (unsigned i = 0 ; i < totalChildren && !ret; ++i) {
        ret |= IsSubTree(tree->child(i), sub);
    } 
    return ret;
}
*/

void BoundFact::KillFact(const AST::Ptr ast, bool isConditionalJump) {
    for (auto fit = fact.begin(); fit != fact.end();)
//        if (IsSubTree(fit->first, ast)) {
        if (*fit->first == *ast) {
	    auto toErase = fit;
	    ++fit;
	    if (toErase->second != NULL) delete toErase->second;
	    fact.erase(toErase);
	} else ++fit;

    // Conditional jump bounds do not change
    // relations between two ASTs
    if (isConditionalJump) return;

    // It also affects the relation map
    for (auto rit = relation.begin(); rit != relation.end();) {
	// If the one of them is changed,
	// we no longer know their relationship
    if (*((*rit)->left) == *ast || *((*rit)->right) == *ast) {
	    delete *rit;
	    rit = relation.erase(rit);
	} else ++rit;
    }
}


bool BoundFact::operator != (const BoundFact &bf) const {    
    if (pred != bf.pred) return true; 
    if (stackTop != bf.stackTop) return true;
    if (fact.size() != bf.fact.size()) return true;
    if (relation.size() != bf.relation.size()) return true;
    for (size_t i = 0; i < relation.size(); ++i)
        if (*relation[i] != *bf.relation[i]) return true;
    if (aliasMap.size() != bf.aliasMap.size()) return true;
    for (auto ait = aliasMap.begin(); ait != aliasMap.end(); ++ait) {
        auto bit = bf.aliasMap.find(ait->first);
	if (bit == bf.aliasMap.end()) return true;
	if (!(*(ait->second) == *(bit->second))) return true;
    }
    return !equal(fact.begin(), fact.end(), bf.fact.begin());
}

BoundFact::BoundFact() {
    fact.clear();
    relation.clear();
    aliasMap.clear();
}

BoundFact::~BoundFact() {
    for (auto fit = fact.begin(); fit != fact.end(); ++fit)
        if (fit->second != NULL)
	    delete fit->second;
    fact.clear();   
    for (auto rit = relation.begin(); rit != relation.end(); ++rit)
        if (*rit != NULL)
	    delete *rit;
    relation.clear();
    aliasMap.clear();
}

BoundFact& BoundFact::operator = (const BoundFact &bf) {
    pred = bf.pred;
    stackTop = bf.stackTop;
    for (auto fit = fact.begin(); fit != fact.end(); ++fit)
        if (fit->second != NULL) delete fit->second;
    fact.clear();
    for (auto fit = bf.fact.begin(); fit != bf.fact.end(); ++fit)     
        fact.insert(make_pair(fit->first, new StridedInterval(*(fit->second))));

    for (auto rit = relation.begin(); rit != relation.end(); ++rit)
        if (*rit != NULL) delete *rit;
    relation.clear();
    for (auto rit = bf.relation.begin(); rit != bf.relation.end(); ++rit)
        relation.push_back(new Relation(**rit));
    aliasMap = bf.aliasMap;

    return *this;
}

BoundFact::BoundFact(const BoundFact &bf) {
    *this = bf;
}   


bool BoundFact::ConditionalJumpBound(Instruction insn, EdgeTypeEnum type) {
    if (!pred.valid) {
        parsing_printf("WARNING: We reach a conditional jump, but have not tracked the flag! Do nothing and return\n");
	return true;
    }
    entryID id = insn.getOperation().getID();
    parsing_printf("\t\tproduce conditional bound for %s, edge type %d\n", insn.format().c_str(), type);
    if (type == COND_TAKEN) {
        switch (id) {
	    // unsigned 
	    case e_ja: {
	        if (pred.e1->getID() == AST::V_ConstantAST) {
		    if (pred.e2->getID() == AST::V_ConstantAST) {
		        // If both elements are constant,
			// it means the conditional jump is actually unconditional.
			// It is possible to happen, but should be unlikely 
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e1);
			IntersectInterval(pred.e1, StridedInterval(1, 0, constAST->val().val - 1));
		    }
		} else if (pred.e2->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e2);
		    IntersectInterval(pred.e1, StridedInterval(1, constAST->val().val + 1, StridedInterval::maxValue));
		} else {
		    InsertRelation(pred.e1, pred.e2,UnsignedLargerThan);
		}
		break;
	    }
	    case e_jae:
	    case e_jnb_jae_j: {
	        if (pred.e1->getID() == AST::V_ConstantAST) {
		    if (pred.e2->getID() == AST::V_ConstantAST) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e1);
			parsing_printf("XXX\n");
		        IntersectInterval(pred.e2, StridedInterval(1, 0, constAST->val().val));
		    }
		} else if (pred.e2->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e2);
		    parsing_printf("YYY\n");
		    IntersectInterval(pred.e1, StridedInterval(1, constAST->val().val, StridedInterval::maxValue));
		} else {
		    InsertRelation(pred.e1, pred.e2, UnsignedLargerThanOrEqual);
		}
		break;
	    }
	    case e_jb: 
	    case e_jb_jnaej_j: {
	        if (pred.e1->getID() == AST::V_ConstantAST) {
		    if (pred.e2->getID() == AST::V_ConstantAST) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e1);
		        IntersectInterval(pred.e2, StridedInterval(1, constAST->val().val + 1, StridedInterval::maxValue));
		    }
		} else if (pred.e2->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e2);
		    // Assuming a-loc pred.e1 is always used as 
		    // unsigned value before it gets rewritten.
		    IntersectInterval(pred.e1, StridedInterval(1, 0 , constAST->val().val - 1));
		} else {
		    InsertRelation(pred.e1, pred.e2, UnsignedLessThan);
		}
		break;
	    }
	    case aarch64_op_b_cond:
	    case power_op_bc:
	    case e_jbe: {
	        if (pred.e1->getID() == AST::V_ConstantAST) {
		    if (pred.e2->getID() == AST::V_ConstantAST) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e1);
		        IntersectInterval(pred.e2, StridedInterval(1, constAST->val().val, StridedInterval::maxValue));
		    }
		} else if (pred.e2->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e2);
		    // Assuming a-loc pred.e1 is always used as 
		    // unsigned value before it gets rewritten.
		    IntersectInterval(pred.e1,StridedInterval(1, 0 , constAST->val().val));
		} else {
		    InsertRelation(pred.e1, pred.e2,UnsignedLessThanOrEqual);
		}
		break;
	    }
	    case e_je: {
	        if (pred.e1->getID() == AST::V_ConstantAST) {
		    if (pred.e2->getID() == AST::V_ConstantAST) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        // the predicate sometimes is between the low 8 bits of a register
			// and a constant. If I simply extends the predicate to the whole
			// 64 bits of a register. I may get wrong constant value. 
		        // IntersectInterval(pred.e2, StridedInterval(1, constAST->val().val, constAST->val().val));
			parsing_printf("WARNING: do not track equal predicate\n");
		    }
		} else if (pred.e2->getID() == AST::V_ConstantAST) {
		    parsing_printf("WARNING: do not track equal predicate\n");
		    //IntersectInterval(pred.e1, StridedInterval(0, constAST->val().val , constAST->val().val));
		} else {
		    InsertRelation(pred.e1, pred.e2,Equal);
		}
		break;
	    }
	    case e_jne: {
	        if (pred.e1->getID() == AST::V_ConstantAST) {
		    if (pred.e2->getID() == AST::V_ConstantAST) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e1);
		        DeleteElementFromInterval(pred.e2, constAST->val().val);
		    }
		} else if (pred.e2->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e2);
		    DeleteElementFromInterval(pred.e1, constAST->val().val);
		} else {
		    InsertRelation(pred.e1, pred.e2, NotEqual);
		}
		break;
	    }

	    // signed
	    case e_jg: {
	        if (pred.e1->getID() == AST::V_ConstantAST) {
		    if (pred.e2->getID() == AST::V_ConstantAST) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e1);
		        IntersectInterval(pred.e2, StridedInterval(1, 0 , constAST->val().val - 1));
		    }
		} else if (pred.e2->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e2);
		    IntersectInterval(pred.e1, StridedInterval(1, constAST->val().val + 1, StridedInterval::maxValue));
		} else {
		    InsertRelation(pred.e1, pred.e2, SignedLargerThan);
		}
		break;
	    }
	    case e_jge: {
	        if (pred.e1->getID() == AST::V_ConstantAST) {
		    if (pred.e2->getID() == AST::V_ConstantAST) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e1);
		        IntersectInterval(pred.e2, StridedInterval(1, 0, constAST->val().val));
		    }
		} else if (pred.e2->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e2);
		    IntersectInterval(pred.e1, StridedInterval(1, constAST->val().val, StridedInterval::maxValue));
		} else {
		    InsertRelation(pred.e1, pred.e2,SignedLargerThanOrEqual);
		}
		break;
	    }
	    case e_jl: {
	        if (pred.e1->getID() == AST::V_ConstantAST) {
		    if (pred.e2->getID() == AST::V_ConstantAST) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e1);
		        IntersectInterval(pred.e2, StridedInterval(1, constAST->val().val + 1, StridedInterval::maxValue));
		    }
		} else if (pred.e2->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e2);
		    IntersectInterval(pred.e1, StridedInterval(1,  0 , constAST->val().val - 1));
		} else {
		    InsertRelation(pred.e1, pred.e2,SignedLessThan);
		}
		break;

	    }
	    case e_jle: {
	        if (pred.e1->getID() == AST::V_ConstantAST) {
		    if (pred.e2->getID() == AST::V_ConstantAST) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e1);
		        IntersectInterval(pred.e2, StridedInterval(1, constAST->val().val, StridedInterval::maxValue));
		    }
		} else if (pred.e2->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e2);
		    IntersectInterval(pred.e1, StridedInterval(1, 0 , constAST->val().val));
		} else {
		    InsertRelation(pred.e1, pred.e2,SignedLessThanOrEqual);
		}
		break;
	    }
	    default:
	        parsing_printf("Unhandled conditional jump type. entry id is %u\n", id);
	}

    } else if (type == COND_NOT_TAKEN) {
        // the following switch statement is almost
	// the same as the above one, except case label
	// all cases of e_jnxx corresponds to cases of e_jxx
	// and cases of e_jxx corresponds to cases of e_jnxx
        switch (id) {
	    // unsigned 
	    case e_jbe: {
	        if (pred.e1->getID() == AST::V_ConstantAST) {
		    if (pred.e2->getID() == AST::V_ConstantAST) {
		        // If both elements are constant,
			// it means the conditional jump is actually unconditional.
			// It is possible to happen, but should be unlikely 
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e1);
		        IntersectInterval(pred.e2, StridedInterval(1, 0, constAST->val().val - 1));
		    }
		} else if (pred.e2->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e2);
		    IntersectInterval(pred.e1, StridedInterval(1, constAST->val().val + 1, StridedInterval::maxValue));
		} else {
		    InsertRelation(pred.e1, pred.e2,UnsignedLargerThan);
		}
		break;
	    }
	    case e_jb: 
	    case e_jb_jnaej_j: {
	        if (pred.e1->getID() == AST::V_ConstantAST) {
		    if (pred.e2->getID() == AST::V_ConstantAST) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e1);
		        IntersectInterval(pred.e2,StridedInterval(1, 0, constAST->val().val));
		    }
		} else if (pred.e2->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e2);
		    IntersectInterval(pred.e1, StridedInterval(1, constAST->val().val, StridedInterval::maxValue));
		} else {
		    InsertRelation(pred.e1, pred.e2 ,UnsignedLargerThanOrEqual);
		}
		break;
	    }
	    case e_jae:
	    case e_jnb_jae_j: {
	        if (pred.e1->getID() == AST::V_ConstantAST) {
		    if (pred.e2->getID() == AST::V_ConstantAST) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e1);
			parsing_printf("!!!\n");
		        IntersectInterval(pred.e2, StridedInterval(1, constAST->val().val + 1, StridedInterval::maxValue));
		    }
		} else if (pred.e2->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e2);
		    // Assuming a-loc pred.e1 is always used as 
		    // unsigned value before it gets rewritten.
		    parsing_printf("@@@\n");
		    IntersectInterval(pred.e1, StridedInterval(1, 0 , constAST->val().val - 1));
		} else {
		    InsertRelation(pred.e1, pred.e2, UnsignedLessThan);
		}
		break;
	    }
	    case aarch64_op_b_cond:
	    case power_op_bc:
	    case e_ja: {
	        if (pred.e1->getID() == AST::V_ConstantAST) {
		    if (pred.e2->getID() == AST::V_ConstantAST) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e1);
		        IntersectInterval(pred.e2, StridedInterval(1, constAST->val().val, StridedInterval::maxValue));
		    }
		} else if (pred.e2->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e2);
		    // Assuming a-loc pred.e1 is always used as 
		    // unsigned value before it gets rewritten.
		    IntersectInterval(pred.e1, StridedInterval(1, 0 , constAST->val().val));
		} else {
		    InsertRelation(pred.e1, pred.e2, UnsignedLessThanOrEqual);
		}
		break;
	    }
	    case e_jne: {
	        if (pred.e1->getID() == AST::V_ConstantAST) {
		    if (pred.e2->getID() == AST::V_ConstantAST) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        parsing_printf("WARNING: do not track equal predicate\n");
		        //IntersectInterval(pred.e2, StridedInterval(1, constAST->val().val, constAST->val().val));
		    }
		} else if (pred.e2->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e2);
		    // the predicate sometimes is between the low 8 bits of a register
		    // and a constant. If I simply extends the predicate to the whole
		    // 64 bits of a register. I may get wrong constant value. 
		    // IntersectInterval(pred.e2, StridedInterval(1, constAST->val().val, constAST->val().val));
		    parsing_printf("WARNING: do not track equal predicate\n");
		    //IntersectInterval(pred.e1, StridedInterval(0, constAST->val().val , constAST->val().val));
		} else {
		    InsertRelation(pred.e1, pred.e2, Equal);
		}
		break;
	    }
	    case e_je: {
	        if (pred.e1->getID() == AST::V_ConstantAST) {
		    if (pred.e2->getID() == AST::V_ConstantAST) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e1);
		        DeleteElementFromInterval(pred.e2, constAST->val().val);
		    }
		} else if (pred.e2->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e2);
		    DeleteElementFromInterval(pred.e1, constAST->val().val);
		} else {
		    InsertRelation(pred.e1, pred.e2,NotEqual);
		}
		break;
	    }

	    // signed
	    case e_jle: {
	        if (pred.e1->getID() == AST::V_ConstantAST) {
		    if (pred.e2->getID() == AST::V_ConstantAST) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e1);
		        IntersectInterval(pred.e2, StridedInterval(1, 0, constAST->val().val - 1));
		    }
		} else if (pred.e2->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e2);
		    IntersectInterval(pred.e1, StridedInterval(1, constAST->val().val + 1, StridedInterval::maxValue));
		} else {
		    InsertRelation(pred.e1, pred.e2, SignedLargerThan);
		}
		break;
	    }
	    case e_jl: {
	        if (pred.e1->getID() == AST::V_ConstantAST) {
		    if (pred.e2->getID() == AST::V_ConstantAST) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e1);
		        IntersectInterval(pred.e2, StridedInterval(1, 0, constAST->val().val));
		    }
		} else if (pred.e2->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e2);
		    IntersectInterval(pred.e1, StridedInterval(1, constAST->val().val, StridedInterval::maxValue));
		} else {
		    InsertRelation(pred.e1, pred.e2, SignedLargerThanOrEqual);
		}
		break;
	    }
	    case e_jge: {
	        if (pred.e1->getID() == AST::V_ConstantAST) {
		    if (pred.e2->getID() == AST::V_ConstantAST) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e1);
		        IntersectInterval(pred.e2, StridedInterval(1, constAST->val().val + 1, StridedInterval::maxValue));
		    }
		} else if (pred.e2->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e2);
		    IntersectInterval(pred.e1, StridedInterval(1, 0 , constAST->val().val - 1));
		} else {
		    InsertRelation(pred.e1, pred.e2,SignedLessThan);
		}
		break;

	    }
	    case e_jg: {
	        if (pred.e1->getID() == AST::V_ConstantAST) {
		    if (pred.e2->getID() == AST::V_ConstantAST) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
//		        ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e1);
//		        IntersectInterval(pred.e2, StridedInterval(1, constAST->val().val, StridedInterval::maxValue));
		    }
		} else if (pred.e2->getID() == AST::V_ConstantAST) {
		    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e2);
		    IntersectInterval(pred.e1, StridedInterval(1, 0 , constAST->val().val));
		} else {
		    InsertRelation(pred.e1, pred.e2,SignedLessThanOrEqual);
		}
		break;
	    }
	    default:
	        fprintf(stderr, "Unhandled conditional jump type. entry id is %u\n", id);
		assert(0);
	}

    } else {
        parsing_printf("Instruction %s\n", insn.format().c_str());
	parsing_printf("type should be either COND_TAKEN or COND_NOT_TAKEN, but it is %d\n", type);
	return false;
    }

    if (pred.id == e_sub) {
        if (pred.e2->getID() == AST::V_ConstantAST) {
	    StridedInterval *val = GetBound(pred.e1);
	    if (val != NULL) {
	        ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e2);
	        val->Sub(StridedInterval(constAST->val().val));
	    }
	} else if (pred.e1->getID() == AST::V_ConstantAST) {
	    StridedInterval *val = GetBound(pred.e2);
	    if (val != NULL) {
	        ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(pred.e1);
	        val->Sub(StridedInterval(constAST->val().val));
	    }

	}
    }
    return true;
}


void BoundFact::SetPredicate(Assignment::Ptr assign,std::pair<AST::Ptr, bool> expandRet ) {   
    Instruction insn = assign->insn();
    entryID id = insn.getOperation().getID();
    pred.valid = true;
    parsing_printf("\t\tLook for predicates for instruction %s, assign %s\n", insn.format().c_str(), assign->format().c_str());
    if (expandRet.first == NULL) {
        // If the instruction is outside the set of instrutions we
        // add instruction semantics. We assume this instruction
        // kills all bound fact.
        parsing_printf("\t\tNo semantic support for this instruction. Invalidate current predicate\n");
	pred.valid = false;
	//SetToBottom();
	return;
    }
    AST::Ptr simplifiedAST = expandRet.first;
    parsing_printf("\t\t semanic expansions: %s\n", simplifiedAST->format().c_str());
    
    // Special handling of test and and instructions on x86
    switch (id) {
	case e_test: {
	    if (simplifiedAST->getID() == AST::V_RoseAST) {
	        RoseAST::Ptr rootRoseAST = boost::static_pointer_cast<RoseAST>(simplifiedAST);
		if (rootRoseAST->val().op == ROSEOperation::equalToZeroOp && 
		    rootRoseAST->child(0)->getID() == AST::V_RoseAST) {
		    RoseAST::Ptr childAST = boost::static_pointer_cast<RoseAST>(rootRoseAST->child(0));
		    if (childAST->val().op == ROSEOperation::andOp) {
		        if (*childAST->child(0) == *childAST->child(1)) {
			    pred.e1 = childAST->child(0);
			    pred.e2 = ConstantAST::create(Constant(0));
			    pred.id = id;
			    break;
			}
			else {
			    parsing_printf("\t\t For test instruction, now only handle the case where two operands are the same\n");
			}
		    }
		}
	    }
	    pred.valid = false;
	    break;
	}
	case e_and: {
	    if (simplifiedAST->getID() == AST::V_RoseAST) {
	        RoseAST::Ptr rootRoseAST = boost::static_pointer_cast<RoseAST>(simplifiedAST);
		if (rootRoseAST->val().op == ROSEOperation::equalToZeroOp && 
		    rootRoseAST->child(0)->getID() == AST::V_RoseAST) {
		    RoseAST::Ptr childAST = boost::static_pointer_cast<RoseAST>(rootRoseAST->child(0));
		    if (childAST->val().op == ROSEOperation::andOp) {
			// The effect of the and instruction can be 
			// evaluated now. And the predicate is actually
			// simply comparing value to 0
		        if (childAST->child(1)->getID() == AST::V_ConstantAST) {
			    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(childAST->child(1));
			    StridedInterval si(1,0,constAST->val().val);
			    IntersectInterval(childAST->child(0), si);
			    pred.e1 = childAST->child(0);
			    pred.e2 = ConstantAST::create(Constant(0));
			    pred.id = id;
			    break;
			} else if (childAST->child(0)->getID() == AST::V_ConstantAST){
			    ConstantAST::Ptr constAST = boost::static_pointer_cast<ConstantAST>(childAST->child(0));
			    StridedInterval si(1,0,constAST->val().val);
			    IntersectInterval(childAST->child(1), si);
			    pred.e1 = ConstantAST::create(Constant(0));
			    pred.e2 = childAST->child(0);
			    pred.id = id;
			    break;
			} else {
			    parsing_printf("\t\t None of the operands is constant, do not handle this case\n");
			}
		    }
		}
	    }
	    pred.valid = false;
	    break;
	}
	default:
	    break;
    }


    ComparisonVisitor cv;
    expandRet.first->accept(&cv);
    pred.e1 = cv.subtrahend;
    pred.e2 = cv.minuend; 
    pred.id = id;
    if (pred.e1 == AST::Ptr() || pred.e2 == AST::Ptr()) {
        pred.valid = false;
    }
}

void BoundFact::SetToBottom() {
    pred.valid = false;
    relation.clear();
    for (auto fit = fact.begin(); fit != fact.end(); ++fit) 
        if (fit->second != NULL)
	    delete fit->second;
    fact.clear();
    aliasMap.clear();
}

StridedInterval* BoundFact::GetBound(const AST::Ptr ast) {
    return GetBound(ast.get());
}

StridedInterval* BoundFact::GetBound(const AST* ast) {
    StridedInterval *ret = NULL;
    for (auto fit = fact.begin(); fit != fact.end(); ++fit)
        if (*(fit->first) == *ast) {
	    ret = fit->second;
	    break;
	}
    return ret;
}


void BoundFact::IntersectInterval(const AST::Ptr ast, StridedInterval si) {
    StridedInterval *bv = GetBound(ast);
    if (bv != NULL) {
        bv->Intersect(si); 
        GenFact(ast, new StridedInterval(*bv), true);
    } else {
        // If the fact value does not exist,
	// it means it is top and can be any value.
        GenFact(ast, new StridedInterval(si), true);
    }
}

void BoundFact::DeleteElementFromInterval(const AST::Ptr ast, int64_t val) {
    StridedInterval *bv = GetBound(ast);
    if (bv != NULL) {
        bv->DeleteElement(val);
    }
}

void BoundFact::InsertRelation(AST::Ptr left, AST::Ptr right, RelationType r) {
    if (r == Equal && *left == *right) return;
    parsing_printf("\t\t\t inserting relation %s and %s, type %d\n",left->format().c_str(), right->format().c_str(), r); 
    if (r == Equal) {
        // We have to consider transitive relations
	size_t n = relation.size();
	for (size_t i = 0; i < n; ++i)
	    if (relation[i]->type == Equal) {
	        if (*(relation[i]->left) == *left && !(*(relation[i]->right) == *right))
		    relation.push_back(new Relation(relation[i]->right, right, r));
	        if (*(relation[i]->left) == *right && !(*(relation[i]->right) == *left))
		    relation.push_back(new Relation(relation[i]->right, left, r));
		if (*(relation[i]->right) == *left && !(*(relation[i]->left) == *right))
		    relation.push_back(new Relation(relation[i]->left, right, r));
	        if (*(relation[i]->right) == *right && !(*(relation[i]->left) == *left))
		    relation.push_back(new Relation(relation[i]->left, left, r));
	    }
    } else if (r == NotEqual) {
       // The new added NotEqual relation with an existing relation like UnsignedLessThanOrEqual 
       // can be combined to a more strict relation like UnsignedLessThan
       for (auto rit = relation.begin(); rit != relation.end(); ++rit) {
           Relation * re = *rit;
	   if ((*(re->left) == *left && *(re->right) == *right) || 
	       (*(re->left) == *right && *(re->right) == *left)) {
	           if (re->type == UnsignedLessThanOrEqual) re->type = UnsignedLessThan;
		   else if (re->type == UnsignedLargerThanOrEqual) re->type = UnsignedLargerThan;
		   else if (re->type == SignedLessThanOrEqual) re->type = SignedLessThan;
		   else if (re->type == SignedLargerThanOrEqual) re->type = SignedLargerThan;
		   else continue;
		   // If the current relation is combined with an existing one,
		   // we can return without inserting a new relation
		   return; 
	   }
       }
    } else if (r == UnsignedLessThanOrEqual || r == UnsignedLargerThanOrEqual || 
               r == SignedLessThanOrEqual || r == SignedLargerThanOrEqual) {
        // Similar to the above case
       for (auto rit = relation.begin(); rit != relation.end(); ++rit) {
           Relation * re = *rit;
	   if ((*(re->left) == *left && *(re->right) == *right) || 
	       (*(re->left) == *right && *(re->right) == *left)) {
	       if (re->type == NotEqual) {
	           r = (RelationType)((int)r - 2);
		   relation.erase(rit);
		   break;
	       }
           }
       }          
    }
    relation.push_back(new Relation(left, right, r));

}

void BoundFact::AdjustPredicate(AST::Ptr out, AST::Ptr in) {
    if (!pred.valid) return;
    if (*out == *pred.e1 && pred.e2->getID() == AST::V_ConstantAST) {
        parsing_printf("\t\t\t Adjust predicate\n");
	if (in->getID() == AST::V_RoseAST) {
	    RoseAST::Ptr root = boost::static_pointer_cast<RoseAST>(in);
	    if (root->val().op == ROSEOperation::addOp && *pred.e1 == *(root->child(0)) && in->child(1)->getID() == AST::V_ConstantAST) {
	        ConstantAST::Ptr v1 = boost::static_pointer_cast<ConstantAST>(pred.e2);
	        ConstantAST::Ptr v2 = boost::static_pointer_cast<ConstantAST>(in->child(1));
	        uint64_t newV = v1->val().val + v2->val().val;
		if (v1->val().size != 64)
		    newV = newV & ((1ULL << v1->val().size) - 1);
		pred.e2 = ConstantAST::create(Constant(newV, v1->val().size));
	    }
	}

    } else if (*out == *pred.e2 && pred.e1->getID() == AST::V_ConstantAST) {
        parsing_printf("\t\t\t Adjust predicate\n");
	if (in->getID() == AST::V_RoseAST) {
	    RoseAST::Ptr root = boost::static_pointer_cast<RoseAST>(in);
	    if (root->val().op == ROSEOperation::addOp && *pred.e2 == *(root->child(0)) && in->child(1)->getID() == AST::V_ConstantAST) {
	        ConstantAST::Ptr v1 = boost::static_pointer_cast<ConstantAST>(pred.e1);
	        ConstantAST::Ptr v2 = boost::static_pointer_cast<ConstantAST>(in->child(1));
	        uint64_t newV = v1->val().val + v2->val().val;
		if (v1->val().size != 64)
		    newV = newV & ((1ULL << v1->val().size) - 1);
		pred.e1 = ConstantAST::create(Constant(newV, v1->val().size));
	    }
	}

    }
}

AST::Ptr BoundFact::GetAlias(const AST::Ptr ast) {
    for (auto ait = aliasMap.begin(); ait != aliasMap.end(); ++ait) {
        if (*(ait->first) == *ast) {
	    return ait->second;
	}
    }
    return AST::Ptr();
}

void BoundFact::TrackAlias(AST::Ptr expr, AST::Ptr outAST, bool findBound) {
    expr = SymbolicExpression::SubstituteAnAST(expr, aliasMap);
    bool find = false;    
    for (auto ait = aliasMap.begin(); ait != aliasMap.end(); ++ait) {
        if (*(ait->first) == *outAST) {
	    ait->second = expr;
	    find = true;
	    break;
	}
    }
    if (!find) {
        aliasMap.insert(make_pair(outAST, expr));
    }
    StridedInterval *substiBound = GetBound(expr);
    if (substiBound != NULL && !findBound) {
        GenFact(outAST, new StridedInterval(*substiBound), false);
    }
}

void BoundFact::PushAConst(int64_t value) {
    stackTop.valid = true;
    stackTop.value = value;
}

bool BoundFact::PopAConst(AST::Ptr ast) {
    if (!stackTop.valid) return false;
    GenFact(ast, new StridedInterval(stackTop.value), false);
    stackTop.valid = false;
    return true;
}

StridedInterval * BoundFact::ApplyRelations(AST::Ptr outAST) {
    AST::Ptr cal;
    for (auto ait = aliasMap.begin(); ait != aliasMap.end(); ++ait)
        if ( *(ait->first) == *outAST) {
	    cal = ait->second;
	    break;
	}
    parsing_printf("\t\tApply relations to %s\n", cal->format().c_str());
    if (cal->getID() != AST::V_RoseAST) return NULL;
    RoseAST::Ptr root = boost::static_pointer_cast<RoseAST>(cal);
    if (root->val().op != ROSEOperation::addOp) return NULL;
    if (root->child(0)->getID() != AST::V_RoseAST || root->child(1)->getID() != AST::V_RoseAST) return NULL;
    RoseAST::Ptr leftChild = boost::static_pointer_cast<RoseAST>(root->child(0));
    RoseAST::Ptr rightChild = boost::static_pointer_cast<RoseAST>(root->child(1));
    if (leftChild->val().op != ROSEOperation::addOp || rightChild->val().op != ROSEOperation::addOp) return NULL;
    AST::Ptr leftOp = leftChild->child(0);
    if (leftChild->child(1)->getID() != AST::V_ConstantAST) return NULL;
    ConstantAST::Ptr baseAST = boost::static_pointer_cast<ConstantAST>(leftChild->child(1));
    int64_t baseValue = baseAST->val().val;
    if (rightChild->child(0)->getID() != AST::V_RoseAST || rightChild->child(1)->getID() != AST::V_ConstantAST) return NULL;
    RoseAST::Ptr invertAST = boost::static_pointer_cast<RoseAST>(rightChild->child(0));
    if (invertAST->val().op != ROSEOperation::invertOp) return NULL;
    ConstantAST::Ptr subAST = boost::static_pointer_cast<ConstantAST>(rightChild->child(1));
    if (subAST->val().val != 1) return NULL;
    AST::Ptr rightOp = invertAST->child(0);
    bool matched = false;
    for (auto rit = relation.begin(); rit != relation.end(); ++rit) {
        if ( (*((*rit)->left) == *(leftOp) && *((*rit)->right) == *(rightOp))
	   ||(*((*rit)->right) == *(leftOp) && *((*rit)->left) == *(rightOp)))
	      if ((*rit)->type != Equal || (*rit)->type != NotEqual) {
	          matched = true;
		  break;
	      }
    }
    parsing_printf("\t\tApply relation matched: %d\n", matched);
    if (matched) return new StridedInterval(StridedInterval(1,0,baseValue-1));
    return NULL;
}

StridedInterval * BoundFact::ApplyRelations2(AST::Ptr outAST) {
    AST::Ptr cal;
    for (auto ait = aliasMap.begin(); ait != aliasMap.end(); ++ait)
        if ( *(ait->first) == *outAST) {
	    cal = ait->second;
	    break;
	}
    parsing_printf("\t\tApply relations2 to %s\n", cal->format().c_str());
    if (cal->getID() != AST::V_RoseAST) return NULL;
    RoseAST::Ptr root = boost::static_pointer_cast<RoseAST>(cal);
    if (root->val().op != ROSEOperation::addOp) return NULL;
    if (root->child(0)->getID() != AST::V_VariableAST || root->child(1)->getID() != AST::V_RoseAST) return NULL;
    VariableAST::Ptr leftChild = boost::static_pointer_cast<VariableAST>(root->child(0));
    RoseAST::Ptr rightChild = boost::static_pointer_cast<RoseAST>(root->child(1));
    if (rightChild->val().op != ROSEOperation::addOp) return NULL;
    if (rightChild->child(0)->getID() != AST::V_RoseAST || rightChild->child(1)->getID() != AST::V_ConstantAST) return NULL;
    RoseAST::Ptr invertAST = boost::static_pointer_cast<RoseAST>(rightChild->child(0));
    if (invertAST->val().op != ROSEOperation::invertOp) return NULL;
    ConstantAST::Ptr subAST = boost::static_pointer_cast<ConstantAST>(rightChild->child(1));
    if (subAST->val().val != 1) return NULL;
    if (invertAST->child(0)->getID() != AST::V_RoseAST) return NULL;
    RoseAST::Ptr shlAST = boost::static_pointer_cast<RoseAST>(invertAST->child(0));
    if (shlAST->val().op != ROSEOperation::shiftLOp) return NULL;
    ConstantAST::Ptr shlBit = boost::static_pointer_cast<ConstantAST>(shlAST->child(1));
    if (shlAST->child(0)->getID() != AST::V_RoseAST) return NULL;
    RoseAST::Ptr shrAST = boost::static_pointer_cast<RoseAST>(shlAST->child(0));
    if (shrAST->val().op != ROSEOperation::shiftROp) return NULL;
    ConstantAST::Ptr shrBit = boost::static_pointer_cast<ConstantAST>(shrAST->child(1));
    if (shrBit->val().val != shlBit->val().val) return NULL;
    if (shrAST->child(0)->getID() != AST::V_VariableAST) return NULL;
    VariableAST::Ptr leftChild2 = boost::static_pointer_cast<VariableAST>(shrAST->child(0));
    if (leftChild->val().reg != leftChild2->val().reg) return NULL;
    return new StridedInterval(StridedInterval(1,0,(1 << shlBit->val().val)-1));
}
void BoundFact::SwapFact(AST::Ptr a, AST::Ptr b) {
    auto aIter = fact.end();
    auto bIter = fact.end();
    for (auto fit = fact.begin(); fit != fact.end(); ++fit) {
        if (*(fit->first) == *a) aIter = fit;
	if (*(fit->first) == *b) bIter = fit;
    }

    if (aIter != fact.end() && bIter != fact.end()) {
        StridedInterval * tmp = aIter->second;
        aIter->second = bIter->second;
	bIter->second = tmp;
    } else if (aIter != fact.end() && bIter == fact.end()) {
        fact.insert(make_pair(b, aIter->second));
	fact.erase(aIter);
    } else if (aIter == fact.end() && bIter != fact.end()) {
        fact.insert(make_pair(a, bIter->second));
	fact.erase(bIter);
    }
    for (auto rit = relation.begin(); rit != relation.end(); ++rit)
        if ( (*((*rit)->left) == *a && *((*rit)->right) == *b)
	   ||(*((*rit)->left) == *b && *((*rit)->right) == *a)) {
	       AST::Ptr tmp = (*rit)->left;
	       (*rit)->left = (*rit)->right;
	       (*rit)->right = tmp;
	   }
}
