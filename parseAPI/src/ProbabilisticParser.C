/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#if defined(cap_stripped_binaries)

#include "ProbabilisticParser.h"

#include <cstdio>
#include <cmath>
#include <algorithm>
#include <queue>
#include <iostream>

#include "entryIDs.h"
#include "registers/x86_64_regs.h"
#include "InstructionDecoder.h"
#include "Instruction.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::InstructionAPI;
using namespace NS_x86;
using namespace hd; 

clock_t ProbabilityCalculator::totalClocks = 0;

// Precision error allowed in double precision float number
#define ZERO 1e-8
static int double_cmp(double a, double b) {
    double delta = a - b;
    if (fabs(delta) < ZERO) return 0;
    else if (delta > ZERO) return 1;
    else return -1;
}

struct ProbAndAddr {
    Address addr;
    double prob;

    ProbAndAddr(Address a, double p): addr(a), prob(p) {}
    bool operator < (const ProbAndAddr &p) const {
        if (double_cmp(prob, p.prob) == 0) return addr > p.addr;
	return prob < p.prob;
    }
};
static bool MatchArgs(unsigned short arg1, unsigned short arg2) {
    return arg1 == arg2;
}

bool IdiomTerm::match(const IdiomTerm &it) const {    
    if (entry_id == WILDCARD_ENTRY_ID || it.entry_id == WILDCARD_ENTRY_ID) return true; // Wildcard matches everything
    if (entry_id == e_nop && it.entry_id == e_nop) return true; // Nops match without considering arguments
    if (entry_id != it.entry_id) return false;
    return MatchArgs(arg1, it.arg1) && MatchArgs(arg2, it.arg2);
}

bool IdiomTerm::matchOpcode(unsigned short eid) const {
    if (entry_id == WILDCARD_ENTRY_ID || eid == WILDCARD_ENTRY_ID) return true; // Wildcard matches everything
    return entry_id == eid;
}

bool IdiomTerm::operator == (const IdiomTerm &it) const {
    return (entry_id == it.entry_id) && (arg1 == it.arg1) && (arg2 == it.arg2);
}

bool IdiomTerm::operator < (const IdiomTerm &it) const {
    if (entry_id != it.entry_id) return entry_id < it.entry_id;
    if (arg1 != it.arg1) return arg1 < it.arg1;
    return arg2 < it.arg2;
}

static string HandleAnOperand(unsigned short arg, int style) {
    if(arg != NOARG) {
	if (arg == MEMARG) {
	    if (style) return "MEM"; else return ":A";
	}
	else if (arg == IMMARG) {
	    if (style) return "IMM"; else return ":B";
	}
	else if (arg == MULTIREG) {
	    if (style) return "MULTIREG"; else return ":C";
	}
	else if (arg == CALLEESAVEREG) {
	    if (style) return "Callee saved reg"; else return ":D";
	} 
	else if (arg == ARGUREG) {
	    if (style) return "Argu passing reg"; else return ":E";
	}
	else if (arg == OTHERREG) {
	    if (style) return "Other reg"; else return ":F";
	}
	else {
	    // the human_readble format is still broken
	    if (style) {
		if (arg == 0x0010) return x86_64::rip.name();
	        switch (arg & 0xf000) {
		    case 0x0000:
		        //GPR
			return MachRegister(arg | x86_64::GPR | Arch_x86_64).name();
		    case 0x1000:
		        //mmx
			return MachRegister(arg | x86_64::MMX | Arch_x86_64).name();
		    case 0x2000:
		        //xxm
			return MachRegister(arg | x86_64::XMM | Arch_x86_64).name();
		    case 0x4000:
		        // flag bit
			return MachRegister(arg | x86_64::FLAG | Arch_x86_64).name();
		}

	      
	        return ":" + MachRegister(arg).name();
	    }
	    char buf[64];
	    snprintf(buf, 64, "%x", arg);
	    return string(":") + string(buf);
	}
    }
    return "";
}


string IdiomTerm::human_format() const {
    string entryname;
    if(*this == WILDCARD_TERM)
        entryname = "*";
    else if(entryNames_IAPI.find((entryID)(entry_id)) == entryNames_IAPI.end()) {
        entryname = "[UNMAPED]";
	fprintf(stderr, "Found entryID not mapped in entryNames_IAPI %d\n", entry_id);
    }
    else {
        entryname = entryNames_IAPI[(entryID)(entry_id)];
	if (arg1 != NOARG) entryname += " " + HandleAnOperand(arg1, 1);
	if (arg2 != NOARG) entryname += "," + HandleAnOperand(arg2, 1);
    }

    return entryname;        

}

bool Idiom::operator < (const Idiom &i) const {
    if (terms.size() != i.terms.size()) return terms.size() < i.terms.size();
    for (size_t index = 0; index < terms.size(); ++index)
        if (terms[index] < i.terms[index]) return true;
	else if (i.terms[index] < terms[index]) return false;
    return false;
}

static void split(const char * str, vector<uint64_t> & terms)
{
    const char *s = str, *e = NULL;
    char buf[32];
    
    while((e = strchr(s,'_')) != NULL) {
        assert(e-s < 32);
	strncpy(buf,s,e-s);
	buf[e-s] = '\0';

#if defined (os_windows)
#define dyn_strtoull _strtoui64
#else
#define dyn_strtoull strtoull
#endif
	terms.push_back(dyn_strtoull(buf,NULL,16));
	
	s = e+1;
    }
    // last one
    if(strlen(s)) {
        terms.push_back(dyn_strtoull(s,NULL,16));
    }
#undef dyn_strtoull
}
Idiom::Idiom(string format, double weight, bool pre): 
    w(weight), prefix(pre)
{
    vector<uint64_t> items;
    split(format.c_str(), items);

    unsigned short entry_id, arg1, arg2;
    for (size_t i = 0; i < items.size(); ++i) {
        uint64_t t = items[i];
        if(!(t & ENTRY_MASK)) {
	    t = t << ARG_SIZE;
	    t |= NOARG;
	}
	if(!(t & ENTRY_MASK)) {
	    t = t << ARG_SIZE;
	    t |= NOARG;
	}
	
	entry_id = (t>>32) & 0xffffffff;
	arg1 = (t>>16) & 0xffff;
	arg2 = t & 0xffff;
	terms.push_back(IdiomTerm(entry_id, arg1, arg2));

    }
    if (prefix) reverse(terms.begin(), terms.end());       
}

string Idiom::human_format() const {
    string ret = "";

    //printf("formatting %s\n",format().c_str());
    for(unsigned i=0;i<terms.size();++i) {
        ret += terms[i].human_format();
        if(i<terms.size()-1)
            ret += "|";
    }
    return ret;

}

IdiomPrefixTree::IdiomPrefixTree() {
    feature = false;
}

void IdiomPrefixTree::addIdiom(const Idiom& idiom) {
    addIdiom(0, idiom);
}

void IdiomPrefixTree::addIdiom(int cur, const Idiom& idiom) {
    if (cur == (int)idiom.terms.size()) {
        // If we reach the last term of the idiom, 
        // we set the weight and 
        feature = true;
	w = idiom.w;
    } else {
        // We add the current idiom term.
        ChildrenByEntryID::iterator idit = childrenClusters.find(idiom.terms[cur].entry_id);
	if (idit == childrenClusters.end()) {
	    idit = childrenClusters.insert(make_pair(idiom.terms[cur].entry_id,ChildrenType())).first;
	}
	ChildrenType& children = idit->second;
	ChildrenType::iterator next = children.end();
	for (auto cit = children.begin(); cit != children.end(); ++cit) {
	    if (cit->first == idiom.terms[cur]) {
	        next = cit;
		break;
	    }
	}

	if (next == children.end()) {
	    // If the current term has an opcode that has not been seen before,
	    // we create a new child to present this opcode.
	    children.push_back(make_pair(idiom.terms[cur], new IdiomPrefixTree() ) );
	    next = children.end();
	    --next;
	}
	next->second->addIdiom(cur+1, idiom);
    }
}

bool IdiomPrefixTree::findChildrenWithOpcode(unsigned short entry_id, ChildrenType &ret) {
    ChildrenByEntryID::iterator idit = childrenClusters.find(entry_id);
    if (idit != childrenClusters.end()) {
        ret.insert(ret.end(), idit->second.begin(), idit->second.end());
    }
    idit = childrenClusters.find(WILDCARD_ENTRY_ID);
    if (idit != childrenClusters.end()) {
        ret.insert(ret.end(), idit->second.begin(), idit->second.end());
    } 
    return !ret.empty();
}

bool IdiomPrefixTree::findChildrenWithArgs(unsigned short arg1, unsigned short arg2, const ChildrenType &candidate, ChildrenType &ret) {
    for (auto cit = candidate.begin(); cit != candidate.end(); ++cit) {
        if (cit->first.match(IdiomTerm(cit->first.entry_id, arg1, arg2)))
	    ret.push_back(*cit);
    }
    return !ret.empty();
}

const IdiomPrefixTree::ChildrenType* IdiomPrefixTree::getChildrenByEntryID(unsigned short entry_id) {
    ChildrenByEntryID::iterator iter = childrenClusters.find(entry_id);
    if (iter == childrenClusters.end())
        return NULL;
    else
        return &iter->second;
}
const IdiomPrefixTree::ChildrenType* IdiomPrefixTree::getWildCardChildren() {
    return getChildrenByEntryID(WILDCARD_ENTRY_ID);
}
ProbabilityCalculator::ProbabilityCalculator(CodeRegion *reg, CodeSource *source, Parser* p, string model_spec):
    model(model_spec), cr(reg), cs(source), parser(p) 
{
}

static bool PassPreCheck(unsigned char *buf) {
    if (buf == NULL) return false;
    if (*buf == 0 || *buf == 0x90) return false;
    return true;
}

double ProbabilityCalculator::calcProbByMatchingIdioms(Address addr) {
    if (FEPProb.find(addr) != FEPProb.end())
        return FEPProb[addr];
    unsigned char *buf = (unsigned char*)(cs->getPtrToInstruction(addr));
    if (!PassPreCheck(buf)) return 0;
    double w = model.getBias();  
    bool valid = true;
    parsing_printf("Idiom matching at %lx, before forward matching w = %.6lf\n", addr, w);
    w += calcForwardWeights(0, addr, model.getNormalIdiomTreeRoot(), valid);
    parsing_printf("after forward matching w = %.6lf\n", w);

    if (valid) {
	set<IdiomPrefixTree*> matched;
	w += calcBackwardWeights(0, addr, model.getPrefixIdiomTreeRoot(), matched);
	parsing_printf("after backward matching w = %.6lf\n", w);
        double prob = ((double)1) / (1 + exp(-w));
        return FEPProb[addr] = reachingProb[addr] = prob;	
    } else return FEPProb[addr] = reachingProb[addr] = 0;
}

void ProbabilityCalculator::calcProbByEnforcingConstraints() {

    // Initialize our knowledge about non-gap functions
    finalized.clear();
    for (auto fit = parser->obj().funcs().begin(); fit != parser->obj().funcs().end(); ++fit) {    
        finalized.insert(*fit);
	for (auto bit = (*fit)->blocks().begin(); bit != (*fit)->blocks().end(); ++bit) {
	    for (Address addr = (*bit)->start(); addr != (*bit)->end(); ++addr)
	        reachingProb[addr] = 2;
	}
    }

    priority_queue<ProbAndAddr> q;
    double threshold = model.getProbThreshold();
    // Use the prob from matching idioms and push all the FEP candidates into the priority queue
    for (auto pit = FEPProb.begin(); pit != FEPProb.end(); ++pit)
        if (pit->second >= threshold) 
	    q.push(ProbAndAddr(pit->first, pit->second));
    
    while (!q.empty()) {
        ProbAndAddr pa = q.top();
	q.pop();

	//This is an out-dated item. The address has either improved prob by
	//applying call consistency constraints or 0 prob by applying overlapping constraints.
	if (double_cmp(FEPProb[pa.addr], pa.prob) != 0) continue;

	parser->parse_at(cr, pa.addr, true, GAP);

	Function *f = parser->findFuncByEntry(cr, pa.addr);
	assert(f != NULL);

	fprintf(stderr, "Enforcing constraints at %lx with prob %.6lf\n", pa.addr, pa.prob);
	// Enforce the overlapping constraints
	dyn_hash_map<Address, double> newFEPProb, newReachingProb;
	dyn_hash_set<Function *> newDiscoveredFuncs;
        if (enforceOverlappingConstraints(f, pa.addr, pa.prob, newFEPProb, newReachingProb, newDiscoveredFuncs)) {
	    Finalize(newFEPProb, newReachingProb, newDiscoveredFuncs);
	    // Enforce the call consistency constraints
	} else {
	    Remove(newDiscoveredFuncs);
	}
    }

}

double ProbabilityCalculator::getFEPProb(Address addr) {
    if (FEPProb.find(addr) != FEPProb.end()) return FEPProb[addr];
    return 0;
}

double ProbabilityCalculator::getReachingProb(Address addr) {
    if (reachingProb.find(addr) != reachingProb.end()) return reachingProb[addr];
    return 0;
}

bool ProbabilityCalculator::isFEP(Address addr) {
    double prob = getFEPProb(addr);
    if (prob >= model.getProbThreshold()) return true; else return false;
}

double ProbabilityCalculator::calcForwardWeights(int cur, Address addr, IdiomPrefixTree *tree, bool &valid) {
    if (addr >= cr->high()) return 0;
    parsing_printf("\tStart matching at %lx for %dth idiom term\n", addr, cur);
    double w = 0;
    if (tree->isFeature()) {
        w = tree->getWeight();
	parsing_printf("\t\tMatch forward idiom with weight %.6lf\n", tree->getWeight());
    }

    if (tree->isLeafNode()) return w;
    
    DecodeData data;
    if (!decodeInstruction(data, addr)) {
        valid = false;
	return 0;
    }

    const IdiomPrefixTree::ChildrenType* children = tree->getChildrenByEntryID(data.entry_id);
    if (children != NULL) {
	for (auto cit = children->begin(); cit != children->end() && valid; ++cit)
	    if (cit->first.match(IdiomTerm(cit->first.entry_id, data.arg1, data.arg2))) {
	        w += calcForwardWeights(cur + 1, addr + data.len, cit->second, valid);
	    }
    }
    if (!valid) return 0;
    // Wildcard terms also match the current instruction
    children = tree->getWildCardChildren();
    if (children != NULL) {
        // Note that for a wildcard term,
	// there is no need to really check whether the operands match or not,
	// but at least we know that the current address can
	// be decoded into a valid instruction.
	for (auto cit = children->begin(); cit != children->end() && valid; ++cit)
	    w += calcForwardWeights(cur + 1, addr + data.len, cit->second, valid);
    }
           
    // the return value is not important if "valid" becomes false
    return w;
}

double ProbabilityCalculator::calcBackwardWeights(int cur, Address addr, IdiomPrefixTree *tree, set<IdiomPrefixTree*> &matched) {
    double w = 0;
    if (tree->isFeature()) {
        if (matched.find(tree) == matched.end()) {
	    matched.insert(tree);
	    w += tree->getWeight();
	    parsing_printf("\t\tBackward match idiom with weight %.6lf\n", tree->getWeight());
	}
    }
    parsing_printf("\tStart matching at %lx for %dth idiom term\n", addr, cur);

    if (tree->isLeafNode()) return w;

    for (Address prevAddr = addr - 1; prevAddr >= cr->low() && addr - prevAddr <= 15; --prevAddr) {
	DecodeData data;
	if (!decodeInstruction(data, prevAddr)) continue;
	if (prevAddr + data.len != addr) continue;

	// Look for idioms that match the exact current instruction
	const IdiomPrefixTree::ChildrenType* children = tree->getChildrenByEntryID(data.entry_id);
	if (children != NULL) {
	    for (auto cit = children->begin(); cit != children->end(); ++cit)
	        if (cit->first.match(IdiomTerm(cit->first.entry_id, data.arg1, data.arg2))) {
		    w += calcBackwardWeights(cur + 1, prevAddr , cit->second, matched);
		}
	}
        // Wildcard terms also match the current instruction
	children = tree->getWildCardChildren();
	if (children != NULL) {
	    for (auto cit = children->begin(); cit != children->end(); ++cit)
	        w += calcBackwardWeights(cur + 1, prevAddr , cit->second, matched);
	}

    }
    return w;
}

bool ProbabilityCalculator::decodeInstruction(DecodeData &data, Address addr) {
    DecodeCache::iterator iter = decodeCache.find(addr);
    if (iter != decodeCache.end()) {
        data = iter->second;
	if (data.len == 0) return false;
    } else {
	unsigned char *buf = (unsigned char*)(cs->getPtrToInstruction(addr));
	if (buf == NULL) { 
	    decodeCache.insert(make_pair(addr, DecodeData(JUNK_OPCODE, 0,0,0)));
	    return false;
	}
	InstructionDecoder dec( buf ,  30, cs->getArch()); 
        Instruction insn = dec.decode();
	if (!insn.isValid()) {
	    decodeCache.insert(make_pair(addr, DecodeData(JUNK_OPCODE, 0,0,0)));
	    return false;
	}
	data.len = (unsigned short)insn.size();
	if (data.len == 0) {
	    decodeCache.insert(make_pair(addr, DecodeData(JUNK_OPCODE, 0,0,0)));
	    return false;
	}
	
	data.entry_id = insn.getOperation().getID();

	vector<Operand> ops;
	insn.getOperands(ops);
	int args[2] = {NOARG,NOARG};
	for(unsigned int i=0;i<2 && i<ops.size();++i) {
	    Operand & op = ops[i];
	    if (op.getValue()->size() == 0) {
		// This is actually an invalid instruction with valid opcode
    		// so modify the opcode cache to make it invalid
		decodeCache.insert(make_pair(addr, DecodeData(JUNK_OPCODE, 0,0,0)));
		return false;
	    }

	    if(!op.readsMemory() && !op.writesMemory()) {
	        // register or immediate
                set<RegisterAST::Ptr> regs;
		op.getReadSet(regs);
		op.getWriteSet(regs);  
    	        
		if(!regs.empty()) {
		    if (regs.size() > 1) {
		        args[i] = MULTIREG;
		    } else {
		        args[i] = (*regs.begin())->getID();
		    }
		} else {
		    // immediate
                    args[i] = IMMARG;
                }
            } else {
	        args[i] = MEMARG; 
            }
        }
        data.arg1 = args[0];
        data.arg2 = args[1];
	decodeCache.insert(make_pair(addr, data));
    }
    return true;
}					      



bool ProbabilityCalculator::enforceOverlappingConstraints(Function *f, 
                                                          Address cur_addr, 
							  double cur_prob,
							  dyn_hash_map<Address, double> &newFEPProb,
							  dyn_hash_map<Address, double> &newReachingProb,
							  dyn_hash_set<Function*> &newDiscoveredFuncs) {
    // Only apply the overlapping constraints to un-finalized functions
    if (finalized.find(f) != finalized.end()) return true;    
    if (newDiscoveredFuncs.find(f) != newDiscoveredFuncs.end()) return true;
    newDiscoveredFuncs.insert(f);
    fprintf(stderr,"  in function at %lx\n", f->addr());
    for (auto bit = f->blocks().begin(); bit != f->blocks().end(); ++bit) {       
        if ((*bit)->region() != cr) continue;
        fprintf(stderr, "  block [%lx,%lx)\n", (*bit)->start(), (*bit)->end());
        for (Address addr = (*bit)->start(); addr < (*bit)->end(); ++addr) {

	    // Meet a byte that is in a function with higher probability: conflict
	    if (double_cmp(cur_prob, getReachingProb(addr)) < 0) {
	        fprintf(stderr, "    conflict with %lx with reaching prob %.6lf\n", addr, getReachingProb(addr));
		return false;
	    } else if (double_cmp(cur_prob, getReachingProb(addr)) > 0) {
	        newReachingProb[addr] = cur_prob;
		newFEPProb[addr] = 0;
	    } else {
	        // TODO: meet a byte with same prob
	    }
	}
    }
    
    auto call_edges = f->callEdges();
    for (auto eit = call_edges.begin(); eit != call_edges.end(); ++eit) {
        if ((*eit)->type() == CALL_FT) continue;
	Address target = (*eit)->trg_addr();
	if (reachingProb.find(target) == reachingProb.end()) continue;
	if (double_cmp(cur_prob, getFEPProb(target)) > 0) {
	    newFEPProb[target] = cur_prob;
	}
	if (double_cmp(cur_prob, getReachingProb(target)) > 0){
	    newReachingProb[target] = cur_prob;
	}
	Function *callee = parser->findFuncByEntry(cr, target);
//	fprintf(stderr, "    find call target from %lx to %lx\n", (*eit)->src()->last(), target);
	if (callee == NULL) {
	    parser->parse_at(cr, target, true, GAP);
	    callee = parser->findFuncByEntry(cr, target);
//	    assert(callee != NULL);  
	}
	if (callee == NULL) continue;
	if (!enforceOverlappingConstraints(callee, cur_addr, cur_prob, newFEPProb, newReachingProb, newDiscoveredFuncs)) return false;
    }
    return true;
}

void ProbabilityCalculator::Finalize(dyn_hash_map<Address, double> &newFEPProb,
                                     dyn_hash_map<Address, double> &newReachingProb,
				     dyn_hash_set<Function*> &newDiscoveredFuncs) {
    for (auto nit = newFEPProb.begin(); nit != newFEPProb.end(); ++nit) {
        FEPProb[nit->first] = nit->second;
    }
    for (auto nit = newReachingProb.begin(); nit != newReachingProb.end(); ++nit) {
        reachingProb[nit->first] = nit->second;
    }
    finalized.insert(newDiscoveredFuncs.begin(), newDiscoveredFuncs.end());
}

void ProbabilityCalculator::Remove(dyn_hash_set<Function*> &newDiscoveredFuncs) {
    for (auto fit = newDiscoveredFuncs.begin(); fit != newDiscoveredFuncs.end(); ++fit) {
       parser->remove_func(*fit); 
    }
}

void ProbabilityCalculator::prioritizedGapParsing() {
    priority_queue<ProbAndAddr> q;
    double threshold = model.getProbThreshold();

    // Use the prob from matching idioms and push all the FEP candidates into the priority queue
    for (auto pit = FEPProb.begin(); pit != FEPProb.end(); ++pit)
        if (pit->second >= threshold) 
	    q.push(ProbAndAddr(pit->first, pit->second));
    
    while (!q.empty()) {
        ProbAndAddr pa = q.top();
	q.pop();
	parser->parse_at(cr, pa.addr, true, GAP);

    }

}
#endif

