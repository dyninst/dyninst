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

BoundValue* BoundFact::GetBound(const Absloc &al) {
        if (fact.find(al) == fact.end())
	    return NULL;
	else
	    return fact.find(al)->second;

}

void BoundValue::Print() {
    parsing_printf("Bound type %d, ",type );
    parsing_printf("Bound value: %lu, ",value);
    parsing_printf("coe %d, ", coe);
    parsing_printf("tableBase %lx, ",tableBase);
    parsing_printf("targetBase %lx, ",targetBase);
    parsing_printf("tableLookup %d, ",tableLookup);
    parsing_printf("tableOffset %d, ",tableOffset);
    parsing_printf("add %d\n", posi);
}

void BoundFact::Intersect(BoundFact &bf) {
        for (auto fit = fact.begin(); fit != fact.end(); ++fit) {
	    if (bf.IsBounded(fit->first)) {
	        BoundValue *val1 = fit->second;
		BoundValue *val2 = bf.GetBound(fit->first);
		if (val1->value != val2->value) val1->type = LessThan;
		if (val1->value < val2->value) val1->value = val2->value;
		if (val1->coe != val2->coe) val1->coe = 1;
		if (val1->tableBase != val2->tableBase) val1->tableBase = 0;
		if (val1->targetBase != val2->targetBase) val1->targetBase = 0;
		val1->tableLookup = val1->tableLookup && val2->tableLookup;
		val1->tableOffset = val1->tableOffset && val2->tableOffset;
		val1->posi = val1->posi | val2->posi;
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

BoundFact::BoundFact():
    cmpAST(AST::Ptr()) {
    cmpBound = 0;
    cmpBoundFactLive = false;
    cmpUsedRegs.clear();
    fact.clear();
}

BoundFact::~BoundFact() {
    for (auto fit = fact.begin(); fit != fact.end(); ++fit)
        if (fit->second != NULL)
	    delete fit->second;
    fact.clear();   
}

BoundFact& BoundFact::operator = (const BoundFact &bf) {
    if (bf.cmpAST == AST::Ptr())
        cmpAST = AST::Ptr();
    else
        cmpAST = bf.cmpAST;
    cmpBound = bf.cmpBound;
    cmpBoundFactLive = bf.cmpBoundFactLive;
    cmpUsedRegs = bf.cmpUsedRegs;
    fact.clear();
    for (auto fit = bf.fact.begin(); fit != bf.fact.end(); ++fit)     
        fact.insert(make_pair(fit->first, new BoundValue(*(fit->second))));
    return *this;
}

BoundFact::BoundFact(const BoundFact &bf) {
    *this = bf;
}    
