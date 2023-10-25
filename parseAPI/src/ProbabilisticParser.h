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

/*
 * The classes used for probabilistic gap parsing
 */

#ifndef _PROBABILISTIC_PARSER_H_
#define _PROBABILISTIC_PARSER_H_

#if defined(cap_stripped_binaries)

#include "Parser.h"

#include <stdint.h>
#include <utility>
#include <string>
#include <vector>
#include <set>
#include <map>

#include <ctime>

#include "common/h/dyntypes.h"
#include "CodeSource.h"
#include "entryIDs.h"
#include "CFG.h"

#include "Instruction.h"

using Dyninst::Address;
using Dyninst::ParseAPI::CodeRegion;
using Dyninst::ParseAPI::CodeSource;
using Dyninst::ParseAPI::Parser;
using Dyninst::ParseAPI::Function;
using Dyninst::InstructionAPI::Instruction;

namespace hd {
#define WILDCARD_ENTRY_ID 0xaaaa


#define NOARG 0xffff
#define IMMARG (NOARG-1)
#define MEMARG (NOARG-2)
#define MULTIREG (NOARG-3)
#define CALLEESAVEREG (NOARG-4)
#define ARGUREG (NOARG-5)
#define OTHERREG (NOARG-6)

#define JUNK_OPCODE 0xffff

#define ENTRY_SHIFT 32ULL
#define ARG1_SHIFT 16ULL
#define ARG2_SHIFT 0ULL
#define ENTRY_SIZE 16ULL
#define ARG_SIZE 16ULL

#define ENTRY_MASK (((uint64_t)(1<<ENTRY_SIZE)-1) << ENTRY_SHIFT)
#define ARG1_MASK (((uint64_t)(1<<ARG_SIZE)-1) << ARG1_SHIFT)
#define ARG2_MASK (((uint64_t)(1<<ARG_SIZE)-1) << ARG2_SHIFT)

struct IdiomTerm {
    unsigned short entry_id{}, arg1{}, arg2{};
    IdiomTerm() {}
    IdiomTerm(unsigned short a, unsigned short b, unsigned short c):
        entry_id(a), arg1(b), arg2(c) {}
    bool operator == (const IdiomTerm & it) const;
    bool operator < (const IdiomTerm & it) const;
    bool matchOpcode(unsigned short entry_id) const;
    bool match(const IdiomTerm &it) const;
    std::string human_format() const;
};

static IdiomTerm WILDCARD_TERM(WILDCARD_ENTRY_ID, NOARG, NOARG);

struct Idiom {
    std::vector<IdiomTerm> terms;
    double w{};
    bool prefix{};
    Idiom() {}
    Idiom(std::string f, double weight, bool pre);
    bool operator < (const Idiom& i) const;
    std::string human_format() const;
};

class IdiomPrefixTree {
public:
    typedef std::vector<std::pair<IdiomTerm, IdiomPrefixTree*> > ChildrenType;
    typedef dyn_hash_map<unsigned short, ChildrenType> ChildrenByEntryID;
private:
    ChildrenByEntryID childrenClusters{};
    double w{};
    bool feature{};
    void addIdiom(int cur, const Idiom& idiom);


public:
    IdiomPrefixTree();
    void addIdiom(const Idiom& idiom);
    bool findChildrenWithOpcode(unsigned short entry_id, ChildrenType &ret);
    bool findChildrenWithArgs(unsigned short arg1, unsigned short arg2, const ChildrenType &candidate, ChildrenType &ret);
    bool isFeature() {return feature; }
    bool isLeafNode() { return childrenClusters.empty(); }
    double getWeight() {return w;}
    const ChildrenType* getChildrenByEntryID(unsigned short entry_id);
    const ChildrenType* getWildCardChildren();
};

class IdiomModel {
    IdiomPrefixTree normal{};
    IdiomPrefixTree prefix{};

    double bias{};
    double prob_threshold{};
    IdiomModel() {}

public:
    IdiomModel(std::string model_spec);
    double getBias() {return bias; }
    double getProbThreshold() { return prob_threshold; }
    IdiomPrefixTree * getNormalIdiomTreeRoot() { return &normal; }
    IdiomPrefixTree * getPrefixIdiomTreeRoot() { return &prefix; }
};

class ProbabilityCalculator {

    struct DecodeData {
	unsigned short entry_id;
	unsigned short arg1;
	unsigned short arg2;
	unsigned short len;
	DecodeData(unsigned short e, unsigned short a1, unsigned short a2, unsigned short l):
	    entry_id(e), arg1(a1), arg2(a2), len(l) {}
        DecodeData() : entry_id(0), arg1(0), arg2(0), len(0) {}	    
    };

    IdiomModel model;
    CodeRegion* cr;
    CodeSource* cs;
    Parser* parser;
    // The probability of each address to be FEP
    dyn_hash_map<Address, double> FEPProb;
    // The highest probability reaching to this address in enforcing overlapping constraints
    dyn_hash_map<Address, double> reachingProb;
    
    dyn_hash_set<Function *> finalized;

    // save the idiom extraction results for idiom matching at different addresses 
    typedef dyn_hash_map<Address, DecodeData > DecodeCache;
    DecodeCache decodeCache;

    // Recursively mathcing normal idioms and calculate weights
    double calcForwardWeights(int cur, Address addr, IdiomPrefixTree *tree, bool &valid);
    // Recursively mathcing prefix idioms and calculate weights
    double calcBackwardWeights(int cur, Address addr, IdiomPrefixTree *tree, std::set<IdiomPrefixTree*> &matched);
    // Enforce the overlapping constraints and
    // return true if the cur_addr doesn't conflict with other identified functions,
    // otherwise return false
    bool enforceOverlappingConstraints(Function *f, 
                                       Address cur_addr, 
				       double cur_prob,
				       dyn_hash_map<Address, double> &newFEPProb,
				       dyn_hash_map<Address, double> &newReachingProb,
				       dyn_hash_set<Function*> &newDiscoveredFuncs);
    bool decodeInstruction(DecodeData &data, Address addr);

    void Finalize(dyn_hash_map<Address, double> &newFEPProb,
                  dyn_hash_map<Address, double> &newReachingProb,
		  dyn_hash_set<Function*> &newDiscoveredFuncs);
    void Remove(dyn_hash_set<Function*> &newDiscoveredFuncs);
    double getReachingProb(Address addr);
   

public:
    ProbabilityCalculator(CodeRegion *reg, CodeSource *source, Parser *parser, std::string model_spec);
	virtual ~ProbabilityCalculator() {
		FEPProb.clear();
	    reachingProb.clear();
		finalized.clear();
	}
    double calcProbByMatchingIdioms(Address addr);
    void calcProbByEnforcingConstraints();
    double getFEPProb(Address addr);
    bool isFEP(Address addr);
    void prioritizedGapParsing();

    static clock_t totalClocks;
};

}

#endif

#endif
