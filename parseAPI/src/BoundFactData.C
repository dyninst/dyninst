#include "BoundFactData.h"
#include "debug_parse.h"

using namespace Dyninst::ParseAPI;

bool BoundValue::operator == (const BoundValue &bv) const {
    return (type == bv.type) &&
           (value == bv.value) &&
	   (tableBase == bv.tableBase) &&
	   (targetBase == bv.targetBase) &&
	   (coe == bv.coe) &&
	   (posi == bv.posi) &&
	   (tableLookup == bv.tableLookup) &&
	   (tableOffset == bv.tableOffset);
}

BoundValue BoundFact::GetBound(const Absloc &al) {
        if (fact.find(al) == fact.end())
	    return BoundValue();
	else
	    return fact[al];

}

void BoundFact::Intersect(BoundFact &bf) {
        for (auto fit = fact.begin(); fit != fact.end(); ++fit) {
	    if (bf.IsBounded(fit->first)) {
	        BoundValue &val1 = fit->second;
		BoundValue val2 = bf.GetBound(fit->first);
		if (val1.value != val2.value) val1.type = LessThan;
		if (val1.value < val2.value) val1.value = val2.value;
		if (val1.coe != val2.coe) val1.coe = 1;
		if (val1.tableBase != val2.tableBase) val1.tableBase = 0;
		if (val1.targetBase != val2.targetBase) val1.targetBase = 0;
		val1.tableLookup = val1.tableLookup && val2.tableLookup;
		val1.tableOffset = val1.tableOffset && val2.tableOffset;
	    }
	}

    cmpBoundFactLive = cmpBoundFactLive && bf.cmpBoundFactLive;
    if (cmpBoundFactLive) {
        if (cmpAST->equals(bf.cmpAST)) {
	    if (bf.cmpBound > cmpBound) cmpBound = bf.cmpBound;
	}
	else
	    cmpBoundFactLive = false;
    }
}

void BoundFact::Print() {
    if (cmpBoundFactLive) {
        parsing_printf("\tcmp bound fact live");
	parsing_printf(", cmpAST = %s", cmpAST->format().c_str());
	parsing_printf(", cmpBound = %lu", cmpBound);
	parsing_printf(", used regs:");
	for (auto rit = cmpUsedRegs.begin(); rit != cmpUsedRegs.end(); ++rit)
	    parsing_printf(" %s", rit->name().c_str());
	parsing_printf("\n");
    } else {
        parsing_printf("\tcmp bound fact dead\n");
    }
    for (auto fit = fact.begin(); fit != fact.end(); ++fit) {
        parsing_printf("\tVar: %s, ", fit->first.format().c_str());
	parsing_printf("Bound type %d, ",fit->second.type );
	parsing_printf("Bound value: %lu, ",fit->second.value);
	parsing_printf("coe %d, ", fit->second.coe);
	parsing_printf("tableBase %lu, ",fit->second.tableBase);
	parsing_printf("targetBase %lu, ",fit->second.targetBase);
	parsing_printf("tableLookup %d, ",fit->second.tableLookup);
	parsing_printf("tableOffset %d, ",fit->second.tableOffset);
	parsing_printf("add %d\n", fit->second.posi);
    }
}

void BoundFact::GenFact(const Absloc &al, BoundValue bv) {
    fact[al] = bv;
    if (al.type() == Absloc::Register)
        CheckCmpValidity(al.reg());
}

void BoundFact::KillFact(const Absloc &al) { 
    fact.erase(al); 
    if (al.type() == Absloc::Register)
        CheckCmpValidity(al.reg());
}

void BoundFact::CheckCmpValidity(const MachRegister &reg) {
    if (cmpBoundFactLive) {
        if (cmpUsedRegs.find(reg) != cmpUsedRegs.end()) cmpBoundFactLive = false;
    }
}

bool BoundFact::CMPBoundMatch(AST *ast) {
    if (!cmpBoundFactLive) return false;
    return ast->equals(cmpAST);
}

bool BoundFact::operator != (const BoundFact &bf) const {
    if (cmpBoundFactLive != bf.cmpBoundFactLive) return true;
    if (cmpBoundFactLive)
        if (cmpBound != bf.cmpBound || !cmpAST->equals(bf.cmpAST)) return true;
    
    if (fact.size() != bf.fact.size()) return true;
    return !equal(fact.begin(), fact.end(), bf.fact.begin());
}


