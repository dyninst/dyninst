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

#ifdef __GNUC__
#if (__GNUC__ < 4) || \
	((__GNUC__ == 4) && (__GNUC_MINOR__ < 3))
using namespace __gnu_cxx;
namespace __gnu_cxx {
    template<typename T> struct hash<dyn_shared_ptr<T> > {
        hash<char *> h;
        unsigned operator()(const dyn_shared_ptr<T> &p) const {
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
#endif // GNUC < 4.3.0
#endif // GNUC

namespace Dyninst {
namespace DepGraphAPI {


class DDGAnalyzer {
 public:
    typedef InstructionAPI::Instruction Insn;
    typedef InstructionAPI::Instruction::Ptr InsnPtr;
    typedef std::set<InsnPtr> InsnSet;
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

    // We need a data structure to represent definitions, since we need to handle
    // flags and the PC separately from anything else...

    class DefSet {
public:
        AbslocSet gprs;
        AbslocSet flags;
        AbslocSet sprs;
        AbslocSet mem;

        bool defPC_;

        class iterator {
            AbslocSet::iterator gI;
            AbslocSet::iterator fI;
            AbslocSet::iterator sI;
            AbslocSet::iterator mI;

            AbslocSet::iterator gE;
            AbslocSet::iterator fE;
            AbslocSet::iterator sE;
            AbslocSet::iterator mE;

            // I'm going to gine these inline so that 
            // we don't have enormous :: nesting in a .C file...
        public:

            iterator &operator++() {
                if (gI != gE) {
                    gI++; 
                    return *this;
                }
                else if (fI != fE) {
                    fI++;
                    return *this;
                }
                else if (sI != sE) {
                    sI++;
                    return *this;
                }
                else {
                    mI++;
                    return *this;
                }
            }

            bool operator==(const iterator &rhs) const {
                return ((rhs.gI == gI) &&
                        (rhs.fI == fI) &&
                        (rhs.sI == sI) &&
                        (rhs.mI == mI) &&
                        (rhs.gE == gE) &&
                        (rhs.fE == fE) &&
                        (rhs.sE == sE) &&
                        (rhs.mE == mE));
            }

            bool operator!=(const iterator &rhs) const {
                return ((rhs.gI != gI) ||
                        (rhs.fI != fI) ||
                        (rhs.sI != sI) ||
                        (rhs.mI != mI) ||
                        (rhs.gE != gE) ||
                        (rhs.fE != fE) ||
                        (rhs.sE != sE) ||
                        (rhs.mE != mE));
            }

            iterator &operator=(const iterator &rhs) {
                gI = rhs.gI;
                fI = rhs.fI;
                sI = rhs.sI;
                mI = rhs.mI;
                gE = rhs.gE;
                fE = rhs.fE;
                sE = rhs.sE;
                mE = rhs.mE;
                return *this;
            }

            const Absloc::Ptr &operator*() const {
                if (gI != gE) {
                    return *gI;
                }
                else if (fI != fE) {
                    return *fI;
                }
                else if (sI != sE) {
                    return *sI;
                }
                else return *mI;
            }

            iterator() {};
                
            iterator(const iterator &rhs) :
                gI(rhs.gI),
                fI(rhs.fI),
                sI(rhs.sI),
                mI(rhs.mI),
                gE(rhs.gE),
                fE(rhs.fE),
                sE(rhs.sE),
                mE(rhs.mE) {};

            iterator(AbslocSet::iterator g1,
                     AbslocSet::iterator g2,
                     AbslocSet::iterator f1,
                     AbslocSet::iterator f2,
                     AbslocSet::iterator s1,
                     AbslocSet::iterator s2,
                     AbslocSet::iterator m1,
                     AbslocSet::iterator m2) :
                gI(g1),
                fI(f1),
                sI(s1),
                mI(m1),
                gE(g2), 
                fE(f2),
                sE(s2),
                mE(m2) {};
        };
        
        iterator begin() const { return iterator(gprs.begin(), gprs.end(),
                                                 flags.begin(), flags.end(),
                                                 sprs.begin(), sprs.end(),
                                                 mem.begin(), mem.end()); }
        
        iterator end() const { return iterator(gprs.end(), gprs.end(), 
                                               flags.end(), flags.end(),
                                               sprs.end(), sprs.end(),
                                               mem.end(), mem.end());}

        iterator beginGprsMem() const { return iterator(gprs.begin(), gprs.end(),
                                                        flags.end(), flags.end(),
                                                        sprs.end(), sprs.end(),
                                                        mem.begin(), mem.end()); }
        iterator beginFlags() const {
            return iterator(gprs.end(), gprs.end(),
                            flags.begin(), flags.end(),
                            sprs.end(), sprs.end(),
                            mem.end(), mem.end());
        }
        iterator beginSprs() const { return iterator(gprs.end(), gprs.end(),
                                                     flags.end(), flags.end(),
                                                     sprs.begin(), sprs.end(),
                                                     mem.end(), mem.end()); }
        DefSet() : defPC_(false) {};
        bool defPC() const { return defPC_; };
    };


    typedef std::set<cNode> cNodeSet;

    // Temporary use: a data type for summarizing the definitions of Abslocs
    // from a particular block.
    // We use a map of cNodes to handle aliasing issues; see block comment
    // in the source file.
    typedef map<AbslocPtr, cNodeSet> DefMap;

    // We also want to keep around kill information so we can efficiently
    // do the interprocedural analysis. This turns into a boolean, as we
    // don't really care _who_ kills (that's summarized in the gen set)
    // so long as _someone_ did.
    typedef map<AbslocPtr, bool> KillMap;
    typedef map<Block *, KillMap> KillSet;

    // A map from each Block to its DefMap data structure.    
    typedef map<Block *, DefMap> ReachingDefsGlobal;

    typedef map<Address, AbslocSet> AbslocMap;

    // Keep a map of nodes we have already built so we don't re-create them
    typedef std::map<cNode, Node::Ptr> CNodeMap;

    typedef std::map<Absloc::Ptr, Node::Ptr> NodeCache;

    typedef std::map<Address, cNodeSet> ActualReturnMap;

    typedef std::vector<Node::Ptr> NodeVec;

    typedef std::map<Absloc::Ptr, NodeVec> NodeMap;

    typedef std::map<Address, DefSet> DefCache;

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
    void summarizeCallGenKill(const InsnPtr,
                              const Address &addr,
                              DefMap &gens,
                              KillMap &kills);

    // Phase 2
    void generateInterBlockReachingDefs(Block *entry);
    void merge(DefMap &target,
               const BlockSet &preds);
    void mergeAliases(const Absloc::Ptr &A,
                      DefMap &source,
                      DefMap &target);

    void calcNewOut(DefMap &out,
                    DefMap &gens,
                    KillMap &kills,
                    DefMap &in);
    

    // Phase 3
    void generateNodes(const BlockSet &blocks);
    void generateBlockNodes(Block *block);

    void createInsnNodes(const InsnPtr I, 
                         const Address &addr,
                         const DefSet &def,
                         DefMap &localReachingDefs);
    void updateReachingDefs(const Address &addr,
                            const DefSet &def,
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

    bool isCall(InsnPtr i) const;
    bool isReturn(InsnPtr i) const;

    Function *getCallee(const Address &a);


    // Debugging

    void debugLocalSet(const DefMap &set, char *str);
    void debugAbslocSet(const AbslocSet &a, char *str);
    void debugDefMap(const DefMap &d, char *str);
    void debugBlockDefs();

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


    
    const DefSet &getDefinedAbslocs(const InsnPtr insn, const Address &a);
    const AbslocSet &getUsedAbslocs(const InsnPtr insn, const Address &a);

    void getUsedAbslocs(const InsnPtr insn,
                        Address addr,
                        AbslocSet &uses);
    void getDefinedAbslocsInt(const InsnPtr insn,
                              Address addr,
                              DefSet &defs);

    static bool convertResultToAddr(const InstructionAPI::Result &res, Address &addr);
    static bool convertResultToSlot(const InstructionAPI::Result &res, int &slot);

    // Returns false if the current height is unknown.
    bool getCurrentStackHeight(Address addr, long &height, int &region);
    bool getCurrentFrameStatus(Address addr);

    InstructionAPI::RegisterAST::Ptr makeRegister(int id);

    //////////////////////
    // Handling callees
    //////////////////////
    //
    // Option 1: trust the ABI 
    void summarizeABIGenKill(Address, Function *, DefMap &, KillMap &);
    void summarizeABIUsed(Address, Function *, const DefMap &, NodeVec &);

    // Option 2: conservative
    void summarizeConservativeGenKill(Address, Function *, DefMap &, KillMap &);
    void summarizeConservativeUsed(Address, Function *, const DefMap &, NodeVec &);
    
    // Option 3: linear scan
    void summarizeLinearGenKill(Address, Function *, int, int, DefMap &, KillMap &);
    void summarizeLinearUsed(Address, Function *, int, int, const DefMap &, NodeVec &);

    // Option 4: recursive
    void summarizeAnalyzeGenKill(Address, Function *, int, int, DefMap &, KillMap &);
    void summarizeAnalyzeUsed(Address, Function *, int, int, const DefMap &, NodeVec &);

    
    ///////////////////////////
    void getUsedToDefine(const InsnPtr I, 
                         const Address &addr, 
                         AbslocPtr D,
                         AbslocSet &used);

    void handlePushEquivalent(Address addr,
                              Absloc::Ptr read,
                              InstructionAPI::RegisterAST::Ptr SP,
                              NodeMap &worklist);

    void handlePopEquivalent(Address addr,
                             InstructionAPI::RegisterAST::Ptr readReg,
                             InstructionAPI::RegisterAST::Ptr SP,
                             NodeMap &worklist);

 private:

    ReachingDefsGlobal allGens;

    KillSet allKills;

    ReachingDefsGlobal inSets;
    ReachingDefsGlobal outSets;

    AbslocMap globalUsed;
    DefCache defCache;
    
    DDG::Ptr ddg;
    
    Function *func_;

    CNodeMap nodeMap;

    NodeCache formalParamNodes_;
    NodeCache formalReturnNodes_;

    unsigned addr_width;

    ActualReturnMap actualReturnMap_;
};


};
};

#endif
