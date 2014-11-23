#include "dyntypes.h"
#include "BoundFactData.h"
#include "debug_parse.h"
#include "Instruction.h"
#include "IndirectASTVisitor.h"
#include "SymEval.h"

using namespace Dyninst::InstructionAPI;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::DataflowAPI;

const StridedInterval StridedInterval::top = StridedInterval(1, StridedInterval::minValue, StridedInterval::maxValue);
const StridedInterval StridedInterval::bottom = StridedInterval();

const BoundValue BoundValue::top = BoundValue(StridedInterval::top);
const BoundValue BoundValue::bottom = BoundValue();

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
	    int newStride = GCD(stride, rhs.stride);
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
    int64_t tmpLow = low;
    low = ~high;
    high = ~tmpLow;

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
}

void StridedInterval::Sub(const StridedInterval& minuend) {
   StridedInterval neg(minuend);
   neg.Neg();
   Add(neg);
}

void StridedInterval::And(const StridedInterval &rhs) {
    // Currently only consider the case where at least one of them is constant
    if (stride == 0) {
       // CONSTANT and any thing ==> 1[1, CONSTANT]
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
    // currently only consider the case where
    // one of them is 1[0, high], the other is a constant
    if (stride == 0 && rhs.stride == 1 && rhs.low == 0) {
        stride = 1;
	low = 0;
	high |= rhs.high;
    } else if (stride == 1 && low == 0 && rhs.stride == 0) {
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

void StridedInterval::ShiftLeft(const StridedInterval &rhs) {
    if (rhs.stride == 0) {
        Mul(StridedInterval(1 << rhs.low));
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

	        int newStride = stride * rhs.stride / GCD(stride, rhs.stride);
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
    char buf[1024];
    sprintf(buf, "%d[%ld,%ld] %x[%lx,%lx]", stride, low, high, stride, low, high);
    return string(buf);
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

BoundValue::BoundValue(int64_t val):
        interval(val), 
	targetBase(0), 
	isTableRead(false), 
	isInverted(false),
	isSubReadContent(false) {}

BoundValue::BoundValue(const StridedInterval &si):
        interval(si), 
	targetBase(0), 
	isTableRead(false), 
	isInverted(false),
	isSubReadContent(false) {}

BoundValue::BoundValue():
        interval(),
	targetBase(0),
	isTableRead(false),
	isInverted(false),
	isSubReadContent(false) {}

BoundValue::BoundValue(const BoundValue & bv):
	targetBase(bv.targetBase),
	isTableRead(bv.isTableRead),
	isInverted(bv.isInverted),
	isSubReadContent(bv.isSubReadContent) 
{
    interval = bv.interval;
}


bool BoundValue::operator == (const BoundValue &bv) const {
    return (interval == bv.interval) &&
	   (targetBase == bv.targetBase) &&
	   (isTableRead == bv.isTableRead) &&
	   (isInverted == bv.isInverted) &&
	   (isSubReadContent == bv.isSubReadContent);
}

bool BoundValue::operator != (const BoundValue &bv) const {
    return !(*this == bv);
}

BoundValue & BoundValue::operator = (const BoundValue &bv) {

    interval = bv.interval;
    targetBase = bv.targetBase;
    isTableRead = bv.isTableRead;
    isInverted = bv.isInverted;
    isSubReadContent = bv.isSubReadContent;
    return *this;

}

void BoundValue::Print() {
    parsing_printf("Interval %s, ", interval.format().c_str() );
    parsing_printf("targetBase %lx, ",targetBase);
    parsing_printf("isTableRead %d, ", isTableRead);
    parsing_printf("isInverted %d, ", isInverted);
    parsing_printf("isSubReadContent %d\n", isSubReadContent);
}

BoundValue* BoundFact::GetBound(const Absloc &al) {
        if (fact.find(al) == fact.end())
	    return NULL;
	else
	    return fact.find(al)->second;

}

void BoundValue::IntersectInterval(StridedInterval &si) {
    if (isTableRead) {
        // We are not going to continue tracking
	// how the memory read is used.
	// The read contents can be anything, so set to top
	*this = top;
	
    }
    interval.Intersect(si);
}

void BoundValue::DeleteElementFromInterval(int64_t val) {
    interval.DeleteElement(val);
}

void BoundValue::Join(BoundValue &bv) {
    if (isTableRead != bv.isTableRead) {
        // Unless boths are table reads, we stop trakcing
	// how the read is used.
	// Also, since a memory read can be any value,
	// we set to top.
	*this = top;
    } else {
        interval.Join(bv.interval);
	if (targetBase != bv.targetBase) targetBase = 0;
	if (isInverted != bv.isInverted) isInverted = false;
	if (isSubReadContent != bv.isSubReadContent) isSubReadContent = false;
    }
}

void BoundValue::ClearTableCheck(){
    isTableRead = false;
    targetBase = 0;
    isInverted = false;
    isSubReadContent = false;
}

void BoundValue::Add(const BoundValue &rhs) {
    // First consider the case for: Imm - [table read address]
    // where the isInverted is true
    if (isTableRead && isInverted && rhs.interval.IsConst(1)) {
        isInverted = false;
	isSubReadContent = true;
    } else if (rhs.isTableRead && rhs.isInverted && interval.IsConst(1)) {
        *this = rhs;
        isInverted = false;
	isSubReadContent = true;
    }
    // Then check if we find the target base
    else if (isTableRead && !isInverted && rhs.interval.IsConst() && !rhs.isTableRead) {
        targetBase = rhs.interval.low;
    } else if (rhs.isTableRead && !rhs.isInverted && interval.IsConst() && !isTableRead) {
        int64_t val = interval.low;
        *this = rhs;
	targetBase = val;
    } 
    // In other case, we only track the values
    else {
        interval.Add(rhs.interval);
	ClearTableCheck();
    }
}

void BoundValue::And(const BoundValue &rhs) { 
    if (isTableRead) {        
        // The memory read content can be anything
        *this = top;
    }
    if (rhs.isTableRead)
        interval.And(StridedInterval::top);
    else
        interval.And(rhs.interval);
    
    // The result is not a table read
    ClearTableCheck();
}

void BoundValue::Mul(const BoundValue &rhs) { 
    if (isTableRead) {        
        // The memory read content can be anything
        *this = top;
    }
    if (rhs.isTableRead)
        interval.Mul(StridedInterval::top);
    else
        interval.Mul(rhs.interval);
    
    // The result is not a table read
    ClearTableCheck();
}

void BoundValue::ShiftLeft(const BoundValue &rhs) {
    if (isTableRead) {        
        // The memory read content can be anything
        *this = top;
    }
    if (rhs.isTableRead)
        interval.ShiftLeft(StridedInterval::top);
    else
        interval.ShiftLeft(rhs.interval);
    
    // The result is not a table read
    ClearTableCheck();
}

void BoundValue::Or(const BoundValue &rhs) { 
    if (isTableRead) {        
        // The memory read content can be anything
        *this = top;
    }
    if (rhs.isTableRead)
        interval.Or(StridedInterval::top);
    else
        interval.Or(rhs.interval);
    
    // The result is not a table read
    ClearTableCheck();
}

void BoundFact::Meet(BoundFact &bf) {
        for (auto fit = fact.begin(); fit != fact.end(); ++fit) {
	    if (bf.IsBounded(fit->first)) {
	        BoundValue *val1 = fit->second;
		BoundValue *val2 = bf.GetBound(fit->first);
		val1->Join(*val2);
	    }
	}

	// Meet the relation map
	RelationMap newMap;
	for (auto rit = relation.begin(); rit != relation.end(); ++rit) {
	    auto it = bf.relation.find(rit->first);
	    // The absloc pair has the same relation on both paths
	    if (it != bf.relation.end() && it->second == rit->second)
	        newMap.insert(make_pair(rit->first, rit->second));
	}
	relation = newMap;

	// Meet the flag predicate
	if (pred != bf.pred) pred.valid = false;
}

void BoundFact::Print() {
    if (pred.valid) {
        parsing_printf("\t\t\tCurrent predicate:");
	if (pred.e1_aloc.type() == Absloc::Unknown)
	    parsing_printf(" element 1 is %ld;", pred.e1_const);
	else 
	    parsing_printf(" element 1 is %s;", pred.e1_aloc.format().c_str());

        if (pred.e2_aloc.type() == Absloc::Unknown)
	    parsing_printf(" element 2 is %ld\n", pred.e2_const);
	else 
	    parsing_printf(" element 2 is %s\n", pred.e2_aloc.format().c_str());
    } else
        parsing_printf("\t\t\tDo not track predicate\n");
    for (auto fit = fact.begin(); fit != fact.end(); ++fit) {
        parsing_printf("\t\t\tVar: %s, ", fit->first.format().c_str());
	fit->second->Print();
    }
}

void BoundFact::GenFact(const Absloc &al, BoundValue* bv) {
    KillFact(al);
    fact.insert(make_pair(al,bv));
}

void BoundFact::KillFact(const Absloc &al) {
    if (fact.find(al) != fact.end() && fact[al] != NULL)
        delete fact[al];
    fact.erase(al); 
    // Need to think about how this affects relation map
}


bool BoundFact::operator != (const BoundFact &bf) const {
    if (pred != bf.pred) return true; 
    if (fact.size() != bf.fact.size()) return true;
    return !equal(fact.begin(), fact.end(), bf.fact.begin());
}

BoundFact::BoundFact() {
    fact.clear();
}

BoundFact::~BoundFact() {
    for (auto fit = fact.begin(); fit != fact.end(); ++fit)
        if (fit->second != NULL)
	    delete fit->second;
    fact.clear();   
}

BoundFact& BoundFact::operator = (const BoundFact &bf) {
    pred = bf.pred;
    fact.clear();
    for (auto fit = bf.fact.begin(); fit != bf.fact.end(); ++fit)     
        fact.insert(make_pair(fit->first, new BoundValue(*(fit->second))));
    return *this;
}

BoundFact::BoundFact(const BoundFact &bf) {
    *this = bf;
}   


void BoundFact::ConditionalJumpBound(Instruction::Ptr insn, EdgeTypeEnum type) {
    entryID id = insn->getOperation().getID();
    if (!pred.valid) {
        parsing_printf("WARNING: We reach a conditional jump, but have not tracked the flag! Do nothing and return\n");
	return;
    }
    parsing_printf("\t\tproduce conditional bound for %s, edge type %d\n", insn->format().c_str(), type);
    if (type == COND_TAKEN) {
        switch (id) {
	    // unsigned 
	    case e_jnbe: {
	        if (pred.e1_aloc.type() == Absloc::Unknown) {
		    if (pred.e2_aloc.type() == Absloc::Unknown) {
		        // If both elements are constant,
			// it means the conditional jump is actually unconditional.
			// It is possible to happen, but should be unlikely 
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        IntersectInterval(pred.e2_aloc, StridedInterval(1, 0, pred.e1_const - 1));
		    }
		} else if (pred.e2_aloc.type() == Absloc::Unknown) {
		    IntersectInterval(pred.e1_aloc, StridedInterval(1, pred.e2_const + 1, StridedInterval::maxValue));
		} else {
		    relation[make_pair(pred.e1_aloc, pred.e2_aloc)] = UnsignedLargerThan;
		}
		break;
	    }
	    case e_jnb: {
	        if (pred.e1_aloc.type() == Absloc::Unknown) {
		    if (pred.e2_aloc.type() == Absloc::Unknown) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        IntersectInterval(pred.e2_aloc, StridedInterval(1, 0, pred.e1_const));
		    }
		} else if (pred.e2_aloc.type() == Absloc::Unknown) {
		    IntersectInterval(pred.e1_aloc, StridedInterval(1, pred.e2_const, StridedInterval::maxValue));
		} else {
		    relation[make_pair(pred.e1_aloc, pred.e2_aloc)] = UnsignedLargerThanOrEqual;
		}
		break;
	    }
	    case e_jb: {
	        if (pred.e1_aloc.type() == Absloc::Unknown) {
		    if (pred.e2_aloc.type() == Absloc::Unknown) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        IntersectInterval(pred.e2_aloc, StridedInterval(1, pred.e1_const + 1, StridedInterval::maxValue));
		    }
		} else if (pred.e2_aloc.type() == Absloc::Unknown) {
		    // Assuming a-loc pred.e1_aloc is always used as 
		    // unsigned value before it gets rewritten.
		    IntersectInterval(pred.e1_aloc, StridedInterval(1, 0 , pred.e2_const - 1));
		} else {
		    relation[make_pair(pred.e1_aloc, pred.e2_aloc)] = UnsignedLessThan;
		}
		break;
	    }
	    case e_jbe: {
	        if (pred.e1_aloc.type() == Absloc::Unknown) {
		    if (pred.e2_aloc.type() == Absloc::Unknown) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        IntersectInterval(pred.e2_aloc, StridedInterval(1, pred.e1_const, StridedInterval::maxValue));
		    }
		} else if (pred.e2_aloc.type() == Absloc::Unknown) {
		    // Assuming a-loc pred.e1_aloc is always used as 
		    // unsigned value before it gets rewritten.
		    IntersectInterval(pred.e1_aloc, StridedInterval(1, 0 , pred.e2_const));
		} else {
		    relation[make_pair(pred.e1_aloc, pred.e2_aloc)] = UnsignedLessThanOrEqual;
		}
		break;
	    }
	    case e_jz: {
	        if (pred.e1_aloc.type() == Absloc::Unknown) {
		    if (pred.e2_aloc.type() == Absloc::Unknown) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        // the predicate sometimes is between the low 8 bits of a register
			// and a constant. If I simply extends the predicate to the whole
			// 64 bits of a register. I may get wrong constant value. 
		        // IntersectInterval(pred.e2_aloc, StridedInterval(1, pred.e1_const, pred.e1_const));
			parsing_printf("WARNING: do not track equal predicate\n");
		    }
		} else if (pred.e2_aloc.type() == Absloc::Unknown) {
		    parsing_printf("WARNING: do not track equal predicate\n");
		    //IntersectInterval(pred.e1_aloc, StridedInterval(0, pred.e2_const , pred.e2_const));
		} else {
		    relation[make_pair(pred.e1_aloc, pred.e2_aloc)] = Equal;
		}
		break;
	    }
	    case e_jnz: {
	        if (pred.e1_aloc.type() == Absloc::Unknown) {
		    if (pred.e2_aloc.type() == Absloc::Unknown) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        DeleteElementFromInterval(pred.e2_aloc, pred.e1_const);
		    }
		} else if (pred.e2_aloc.type() == Absloc::Unknown) {
		    DeleteElementFromInterval(pred.e1_aloc, pred.e2_const);
		} else {
		    relation[make_pair(pred.e1_aloc, pred.e2_aloc)] = NotEqual;
		}
		break;
	    }

	    // signed
	    case e_jnle: {
	        if (pred.e1_aloc.type() == Absloc::Unknown) {
		    if (pred.e2_aloc.type() == Absloc::Unknown) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        IntersectInterval(pred.e2_aloc, StridedInterval(1, StridedInterval::minValue, pred.e1_const - 1));
		    }
		} else if (pred.e2_aloc.type() == Absloc::Unknown) {
		    IntersectInterval(pred.e1_aloc, StridedInterval(1, pred.e2_const + 1, StridedInterval::maxValue));
		} else {
		    relation[make_pair(pred.e1_aloc, pred.e2_aloc)] = SignedLargerThan;
		}
		break;
	    }
	    case e_jnl: {
	        if (pred.e1_aloc.type() == Absloc::Unknown) {
		    if (pred.e2_aloc.type() == Absloc::Unknown) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        IntersectInterval(pred.e2_aloc, StridedInterval(1, StridedInterval::minValue, pred.e1_const));
		    }
		} else if (pred.e2_aloc.type() == Absloc::Unknown) {
		    IntersectInterval(pred.e1_aloc, StridedInterval(1, pred.e2_const, StridedInterval::maxValue));
		} else {
		    relation[make_pair(pred.e1_aloc, pred.e2_aloc)] = SignedLargerThanOrEqual;
		}
		break;
	    }
	    case e_jl: {
	        if (pred.e1_aloc.type() == Absloc::Unknown) {
		    if (pred.e2_aloc.type() == Absloc::Unknown) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        IntersectInterval(pred.e2_aloc, StridedInterval(1, pred.e1_const + 1, StridedInterval::maxValue));
		    }
		} else if (pred.e2_aloc.type() == Absloc::Unknown) {
		    IntersectInterval(pred.e1_aloc, StridedInterval(1, StridedInterval::minValue , pred.e2_const - 1));
		} else {
		    relation[make_pair(pred.e1_aloc, pred.e2_aloc)] = SignedLessThan;
		}
		break;

	    }
	    case e_jle: {
	        if (pred.e1_aloc.type() == Absloc::Unknown) {
		    if (pred.e2_aloc.type() == Absloc::Unknown) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        IntersectInterval(pred.e2_aloc, StridedInterval(1, pred.e1_const, StridedInterval::maxValue));
		    }
		} else if (pred.e2_aloc.type() == Absloc::Unknown) {
		    IntersectInterval(pred.e1_aloc, StridedInterval(1, StridedInterval::minValue , pred.e2_const));
		} else {
		    relation[make_pair(pred.e1_aloc, pred.e2_aloc)] = SignedLessThanOrEqual;
		}
		break;
	    }
	    default:
	        fprintf(stderr, "Unhandled conditional jump type. entry id is %d\n", id);
	}

    } else if (type == COND_NOT_TAKEN) {
        // the following switch statement is almost
	// the same as the above one, except case label
	// all cases of e_jnxx corresponds to cases of e_jxx
	// and cases of e_jxx corresponds to cases of e_jnxx
        switch (id) {
	    // unsigned 
	    case e_jbe: {
	        if (pred.e1_aloc.type() == Absloc::Unknown) {
		    if (pred.e2_aloc.type() == Absloc::Unknown) {
		        // If both elements are constant,
			// it means the conditional jump is actually unconditional.
			// It is possible to happen, but should be unlikely 
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        IntersectInterval(pred.e2_aloc, StridedInterval(1, 0, pred.e1_const - 1));
		    }
		} else if (pred.e2_aloc.type() == Absloc::Unknown) {
		    IntersectInterval(pred.e1_aloc, StridedInterval(1, pred.e2_const + 1, StridedInterval::maxValue));
		} else {
		    relation[make_pair(pred.e1_aloc, pred.e2_aloc)] = UnsignedLargerThan;
		}
		break;
	    }
	    case e_jb: {
	        if (pred.e1_aloc.type() == Absloc::Unknown) {
		    if (pred.e2_aloc.type() == Absloc::Unknown) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        IntersectInterval(pred.e2_aloc, StridedInterval(1, 0, pred.e1_const));
		    }
		} else if (pred.e2_aloc.type() == Absloc::Unknown) {
		    IntersectInterval(pred.e1_aloc, StridedInterval(1, pred.e2_const, StridedInterval::maxValue));
		} else {
		    relation[make_pair(pred.e1_aloc, pred.e2_aloc)] = UnsignedLargerThanOrEqual;
		}
		break;
	    }
	    case e_jnb: {
	        if (pred.e1_aloc.type() == Absloc::Unknown) {
		    if (pred.e2_aloc.type() == Absloc::Unknown) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        IntersectInterval(pred.e2_aloc, StridedInterval(1, pred.e1_const + 1, StridedInterval::maxValue));
		    }
		} else if (pred.e2_aloc.type() == Absloc::Unknown) {
		    // Assuming a-loc pred.e1_aloc is always used as 
		    // unsigned value before it gets rewritten.
		    IntersectInterval(pred.e1_aloc, StridedInterval(1, 0 , pred.e2_const - 1));
		} else {
		    relation[make_pair(pred.e1_aloc, pred.e2_aloc)] = UnsignedLessThan;
		}
		break;
	    }
	    case e_jnbe: {
	        if (pred.e1_aloc.type() == Absloc::Unknown) {
		    if (pred.e2_aloc.type() == Absloc::Unknown) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        IntersectInterval(pred.e2_aloc, StridedInterval(1, pred.e1_const, StridedInterval::maxValue));
		    }
		} else if (pred.e2_aloc.type() == Absloc::Unknown) {
		    // Assuming a-loc pred.e1_aloc is always used as 
		    // unsigned value before it gets rewritten.
		    IntersectInterval(pred.e1_aloc, StridedInterval(1, 0 , pred.e2_const));
		} else {
		    relation[make_pair(pred.e1_aloc, pred.e2_aloc)] = UnsignedLessThanOrEqual;
		}
		break;
	    }
	    case e_jnz: {
	        if (pred.e1_aloc.type() == Absloc::Unknown) {
		    if (pred.e2_aloc.type() == Absloc::Unknown) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        parsing_printf("WARNING: do not track equal predicate\n");
		        //IntersectInterval(pred.e2_aloc, StridedInterval(1, pred.e1_const, pred.e1_const));
		    }
		} else if (pred.e2_aloc.type() == Absloc::Unknown) {
		    // the predicate sometimes is between the low 8 bits of a register
		    // and a constant. If I simply extends the predicate to the whole
		    // 64 bits of a register. I may get wrong constant value. 
		    // IntersectInterval(pred.e2_aloc, StridedInterval(1, pred.e1_const, pred.e1_const));
		    parsing_printf("WARNING: do not track equal predicate\n");
		    //IntersectInterval(pred.e1_aloc, StridedInterval(0, pred.e2_const , pred.e2_const));
		} else {
		    relation[make_pair(pred.e1_aloc, pred.e2_aloc)] = Equal;
		}
		break;
	    }
	    case e_jz: {
	        if (pred.e1_aloc.type() == Absloc::Unknown) {
		    if (pred.e2_aloc.type() == Absloc::Unknown) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        DeleteElementFromInterval(pred.e2_aloc, pred.e1_const);
		    }
		} else if (pred.e2_aloc.type() == Absloc::Unknown) {
		    DeleteElementFromInterval(pred.e1_aloc, pred.e2_const);
		} else {
		    relation[make_pair(pred.e1_aloc, pred.e2_aloc)] = NotEqual;
		}
		break;
	    }

	    // signed
	    case e_jle: {
	        if (pred.e1_aloc.type() == Absloc::Unknown) {
		    if (pred.e2_aloc.type() == Absloc::Unknown) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        IntersectInterval(pred.e2_aloc, StridedInterval(1, StridedInterval::minValue, pred.e1_const - 1));
		    }
		} else if (pred.e2_aloc.type() == Absloc::Unknown) {
		    IntersectInterval(pred.e1_aloc, StridedInterval(1, pred.e2_const + 1, StridedInterval::maxValue));
		} else {
		    relation[make_pair(pred.e1_aloc, pred.e2_aloc)] = SignedLargerThan;
		}
		break;
	    }
	    case e_jl: {
	        if (pred.e1_aloc.type() == Absloc::Unknown) {
		    if (pred.e2_aloc.type() == Absloc::Unknown) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        IntersectInterval(pred.e2_aloc, StridedInterval(1, StridedInterval::minValue, pred.e1_const));
		    }
		} else if (pred.e2_aloc.type() == Absloc::Unknown) {
		    IntersectInterval(pred.e1_aloc, StridedInterval(1, pred.e2_const, StridedInterval::maxValue));
		} else {
		    relation[make_pair(pred.e1_aloc, pred.e2_aloc)] = SignedLargerThanOrEqual;
		}
		break;
	    }
	    case e_jnl: {
	        if (pred.e1_aloc.type() == Absloc::Unknown) {
		    if (pred.e2_aloc.type() == Absloc::Unknown) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        IntersectInterval(pred.e2_aloc, StridedInterval(1, pred.e1_const + 1, StridedInterval::maxValue));
		    }
		} else if (pred.e2_aloc.type() == Absloc::Unknown) {
		    IntersectInterval(pred.e1_aloc, StridedInterval(1, StridedInterval::minValue , pred.e2_const - 1));
		} else {
		    relation[make_pair(pred.e1_aloc, pred.e2_aloc)] = SignedLessThan;
		}
		break;

	    }
	    case e_jnle: {
	        if (pred.e1_aloc.type() == Absloc::Unknown) {
		    if (pred.e2_aloc.type() == Absloc::Unknown) {
		        parsing_printf("WARNING: both predicate elements are constants!\n");
		    } else {
		        IntersectInterval(pred.e2_aloc, StridedInterval(1, pred.e1_const, StridedInterval::maxValue));
		    }
		} else if (pred.e2_aloc.type() == Absloc::Unknown) {
		    IntersectInterval(pred.e1_aloc, StridedInterval(1, StridedInterval::minValue , pred.e2_const));
		} else {
		    relation[make_pair(pred.e1_aloc, pred.e2_aloc)] = SignedLessThanOrEqual;
		}
		break;
	    }
	    default:
	        fprintf(stderr, "Unhandled conditional jump type. entry id is %d\n", id);
	}

    } else {
        assert(0 && "type should be either COND_TAKEN or COND_NOT_TAKEN");
    }
}


void BoundFact::SetPredicate(Assignment::Ptr assign ) {   
    Instruction::Ptr insn = assign->insn();
    entryID id = insn->getOperation().getID();
    pred.valid = true;
    pred.e1_aloc = Absloc();
    pred.e2_aloc = Absloc();
    parsing_printf("\t\tLook for predicates for instruction %s, assign %s\n", insn->format().c_str(), assign->format().c_str());
    pair<AST::Ptr, bool> expandRet = SymEval::expand(assign, false);
    if (expandRet.first == NULL) {
        // If the instruction is outside the set of instrutions we
        // add instruction semantics. We assume this instruction
        // kills all bound fact.
        parsing_printf("\t\t (Should not happen here) No semantic support for this instruction. Kill all bound fact\n");
	SetToBottom();
	return;
    }
    parsing_printf("\t\texpand to %s\n", expandRet.first->format().c_str());
    AST::Ptr simplifiedAST = SimplifyAnAST(expandRet.first, insn->size());
    parsing_printf("\t\tafter simplifying %s\n", simplifiedAST->format().c_str());
    switch (id) {
        case e_cmp: {	
	    ComparisonVisitor cv;
	    expandRet.first->accept(&cv);
	    //parsing_printf("\t\t\tsubtrahend: %s, minuend: %s\n", cv.subtrahend->format().c_str(), cv.minuend->format().c_str());
	    if (cv.subtrahend->getID() == AST::V_ConstantAST) {	       
	        ConstantAST::Ptr subtrahendAST = boost::static_pointer_cast<ConstantAST>(cv.subtrahend);
	        pred.e1_const = subtrahendAST->val().val;
	    } else if (cv.subtrahend->getID() == AST::V_VariableAST) {
	        VariableAST::Ptr subtrahendAST = boost::static_pointer_cast<VariableAST>(cv.subtrahend);
		pred.e1_aloc = subtrahendAST->val().reg.absloc();
	    } else {
	        parsing_printf("\t\t The source cmp operand is neither a constant or an a-loc. Bottom it\n");
		pred.valid = false;
		return;
	    }
	    if (cv.minuend->getID() == AST::V_ConstantAST) {	       
	        ConstantAST::Ptr minuendAST = boost::static_pointer_cast<ConstantAST>(cv.minuend);
	        pred.e2_const = minuendAST->val().val;
	    } else if (cv.minuend->getID() == AST::V_VariableAST) {
	        VariableAST::Ptr minuendAST = boost::static_pointer_cast<VariableAST>(cv.minuend);
		pred.e2_aloc = minuendAST->val().reg.absloc();
	    } else {
	        parsing_printf("\t\t The dest cmp operand is neither a constant or an a-loc. Bottom it\n");
		pred.valid = false;
		return;
	    }

	    break;
	}
	case e_test: {
	    parsing_printf("\t\t To be handled\n");
	    pred.valid = false;
	    break;
	}
	case e_sub: {
	    parsing_printf("\t\t To be handled\n");
	    pred.valid = false;
	    break;

	}
	default:
	    parsing_printf("Not tracking this instruction that sets flags: %s\n", insn->format().c_str());
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
}

void BoundFact::IntersectInterval(Absloc aloc, StridedInterval si) {
    if (fact.find(aloc) != fact.end()) {
        fact[aloc]->IntersectInterval(si);
    } else {
        fact.insert(make_pair(aloc, new BoundValue(si)));
    }
}

void BoundFact::DeleteElementFromInterval(Absloc aloc, int64_t val) {
    if (fact.find(aloc) != fact.end()) {
        fact[aloc]->DeleteElementFromInterval(val);
    }
}
