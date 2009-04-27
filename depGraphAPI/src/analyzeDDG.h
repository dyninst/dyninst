/*
 * Copyright (c) 2007-2008 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#if !defined(DDG_ANALYZER_H)
#define DDG_ANALYZER_H


#include "dyntypes.h"
#include <map>
#include <algorithm>
#include <list>
#include "Absloc.h"
#include "Instruction.h"
#include "Graph.h"
#include "Node.h"
#include "DDG.h"

//#include "Analyzer.h"

#include "dyninstAPI/src/image-func.h"

#include <sys/times.h>

// Example intra-function DDG creator
// Heavily borrows from the Dyninst internal liveness.C file
// Performed over image_funcs (for now)

// Define some hash functions...
class BPatch_basicBlock;
class BPatch_function;


using namespace __gnu_cxx;
namespace __gnu_cxx {
    template<typename T> struct hash<dyn_detail::boost::shared_ptr<T> > {
        hash<char *> h;
        unsigned operator()(const dyn_detail::boost::shared_ptr<T> &p) const {
            return h((char *)p.get());
        }
    };
    template<> struct hash<BPatch_basicBlock *> {
        hash<char *> h;
        unsigned operator()(const BPatch_basicBlock *p) const {
            return h((char *)p);
        }
    };
};

namespace Dyninst {
namespace DepGraphAPI {


class DDGAnalyzer {
 public:
    typedef InstructionAPI::Instruction Insn;
    typedef std::set<Insn> InsnSet;
    //typedef Absloc::Ptr AbslocPtr;
    typedef Absloc::Ptr AbslocPtr;
    typedef std::set<AbslocPtr> AbslocSet;
    typedef Node::Ptr NodePtr;
    typedef std::set<Node::Ptr> odeSet;
    
    // TODO: replace this with ParsingAPI concepts.
    typedef BPatch_basicBlock Block;
    typedef BPatch_function Function;

    typedef std::set<Block *> BlockSet;
    
    // a Node in the DDG is an <Instruction,Absloc> pair. We use an Address instead
    // of an Instruction because the two are logically the same. 
    // Nodes are also typed - a formal parameter node, etc. So we have promoted
    // cnode to a full-fledged struct. 

    typedef enum {
        normal,
        formalParam,
        formalReturn,
        actualParam,
        actualReturn } nodeType;

    class cNode {
    public:
        cNode(Address a, Absloc::Ptr abs) :
            addr(a), absloc(abs), type(normal), func(NULL) {};
        cNode(Address a, Absloc::Ptr abs, nodeType n) :
            addr(a), absloc(abs), type(n), func(NULL) {};
        cNode(Address a, Absloc::Ptr abs, nodeType n, Function *f) : 
            addr(a), absloc(abs), type(n), func(f) {};

        bool operator<(const cNode &rhs) const {
            if (addr < rhs.addr) return true;
            if (addr > rhs.addr) return false;
            if (absloc.get() < rhs.absloc.get()) return true;
            if (absloc.get() > rhs.absloc.get()) return false;
            if (type < rhs.type) return true;
            if (type > rhs.type) return false;
            // Must be equal...
            return false;
        }

        bool operator==(const cNode &rhs) const {
            return ((addr == rhs.addr) &&
                    (absloc == rhs.absloc) &&
                    (type == rhs.type) &&
                    (func == rhs.func));
        }

        Address addr;
        Absloc::Ptr absloc;
        nodeType type;
        Function *func;
    };

    typedef std::vector<cNode> cNodeSet;

    // Temporary use: a data type for summarizing the definitions of Abslocs
    // from a particular block.
    // We use a map of cNodes to handle aliasing issues; see block comment
    // in the source file.
    typedef dyn_hash_map<AbslocPtr, cNodeSet> DefMap;

    // We also want to keep around kill information so we can efficiently
    // do the interprocedural analysis. This turns into a boolean, as we
    // don't really care _who_ kills (that's summarized in the gen set)
    // so long as _someone_ did.
    typedef dyn_hash_map<AbslocPtr, bool> KillMap;
    typedef dyn_hash_map<Block *, KillMap> KillSet;

    // A map from each Block to its DefMap data structure.    
    typedef dyn_hash_map<Block *, DefMap> ReachingDefsGlobal;

    typedef dyn_hash_map<Address, AbslocSet> AbslocMap;

    // Keep a map of nodes we have already built so we don't re-create them
    typedef std::map<cNode, Node::Ptr> NodeMap;

    typedef std::map<Absloc::Ptr, Node::Ptr> NodeCache;

 public:
    DDG::Ptr analyze();
    DDGAnalyzer(Function *f);

 private:

    // Phase 1
    void summarizeGenKillSets(const BlockSet &blocks);
    void summarizeBlockGenKill(Block *curBlock);
    void updateDefSet(const AbslocPtr D,
                      DefMap &defMap,
                      cNode &cnode);

    void updateKillSet(const AbslocPtr D,
                       KillMap &kills);
    void summarizeCallGenKill(const Insn &,
                              const Address &addr,
                              DefMap &gens,
                              KillMap &kills);

    // Phase 2
    void generateInterBlockReachingDefs(Block *entry);
    void merge(DefMap &target,
               const BlockSet &preds);
    void calcNewOut(DefMap &out,
                    DefMap &gens,
                    KillMap &kills,
                    DefMap &in);
    

    // Phase 3
    void generateNodes(const BlockSet &blocks);
    void generateBlockNodes(Block *block);

    void createInsnNodes(const Address &addr,
                         const AbslocSet &used, const AbslocSet &def,
                         DefMap &localReachingDefs);
    void updateReachingDefs(const Address &addr,
                            const AbslocSet &used, const AbslocSet &def,
                            DefMap &localReachingDefs);
    void createCallNodes(const Address &addr,
                         const DefMap &localReachingDefs);
    void createReturnNodes(const Address &addr,
                           const DefMap &localReachingDefs);

    // Utility
    void getPredecessors(Block *block,
                         BlockSet &preds);

    void getSuccessors(Block *block,
                       BlockSet &succs);

    NodePtr makeNode(const cNode &cnode);

    bool isCall(Insn i) const;
    bool isReturn(Insn i) const;

    Function *getCallee(const Address &a);


    // Debugging

    void debugLocalSet(const DefMap &set, char *str);
    void debugAbslocSet(const AbslocSet &a, char *str);
    void debugDefMap(const DefMap &d, char *str);

    // Absloc related

    //static void getAbslocs(AbslocSet &locs);
    
    //static Ptr createAbsloc(const std::string name);
    
    // Translate from an InstructionAPI Expression into an abstract
    // location.
    // We need some sort of "get state" function; for now we're doing
    // Function and address...
    Absloc::Ptr getAbsloc(const InstructionAPI::Expression::Ptr exp,
                                 Address addr);
    Absloc::Ptr getAbsloc(const InstructionAPI::RegisterAST::Ptr reg);

    void getABIDefinedAbslocs(AbslocSet &abslocs);
    void getABIUsedAbslocs(AbslocSet &abslocs);
    
    const AbslocSet &getDefinedAbslocs(const Insn &insn, const Address &a);
    const AbslocSet &getUsedAbslocs(const Insn &insn, const Address &a);

    void getUsedAbslocs(const InstructionAPI::Instruction insn,
                               Address addr,
                               AbslocSet &uses);
    void getDefinedAbslocs(const InstructionAPI::Instruction insn,
                                  Address addr,
                                  AbslocSet &defs);

    static bool convertResultToAddr(const InstructionAPI::Result &res, Address &addr);
    static bool convertResultToSlot(const InstructionAPI::Result &res, int &slot);

    bool isFramePointer(const InstructionAPI::InstructionAST::Ptr &reg, Address addr);
    bool isStackPointer(const InstructionAPI::InstructionAST::Ptr &reg, Address addr);

    void bindFP(InstructionAPI::InstructionAST::Ptr &reg, Address addr);
    void bindSP(InstructionAPI::InstructionAST::Ptr &reg, Address addr);

    // Returns false if the current height is unknown.
    bool getCurrentStackHeight(Address addr, int &height);

    InstructionAPI::RegisterAST::Ptr makeRegister(int id);
 private:

    ReachingDefsGlobal allGens;

    KillSet allKills;

    ReachingDefsGlobal inSets;
    ReachingDefsGlobal outSets;

    AbslocMap globalUsed;
    AbslocMap globalDef;

    DDG::Ptr ddg;
    
    Function *func_;

    NodeMap nodeMap;

    NodePtr virtualNode;

    NodeCache formalParamNodes_;
    NodeCache formalReturnNodes_;

    unsigned addr_width;

};


};
};

#endif
