/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#include <map>
#include <algorithm>
#include <list>
#include "Absloc.h"
#include "Instruction.h"
#include "Graph.h"
#include "Node.h"

#include "boost/tuple/tuple.hpp"

// Example intra-function DDG creator
// Heavily borrows from the Dyninst internal liveness.C file
// Performed over BPatch objects (for now) but designed
// to be pretty agnostic of inputs. 

using namespace Dyninst;
using namespace Dyninst::DDG;
using namespace Dyninst::InstructionAPI;
class BPatch_basicBlock;
class BPatch_flowGraph;
class BPatch_function;

class intraFunctionDDGCreator {
    typedef InstructionAPI::Instruction Insn;
    typedef std::set<Insn> InsnSet;
    typedef Absloc::Ptr AbslocPtr;
    typedef std::set<AbslocPtr> AbslocSet;
    typedef Node::Ptr NodePtr;
    
    typedef BPatch_basicBlock Block;
    typedef BPatch_flowGraph Flowgraph;
    typedef BPatch_function Function;



    // The set of instructions that define an absloc used at the current
    // instruction
    typedef std::set<Block *> BlockSet;

    // This is sufficient information to uniquely identify an instruction:
    // its address (guaranteed unique by model) and instruction information.
    struct InsnInstance {
        InsnInstance(std::pair<Instruction,Address> p) : 
            addr(p.second),
            insn(p.first) {}
        InsnInstance() : addr(0), insn() {}
        Address addr;
        Instruction insn;
        bool operator< (InsnInstance i) const {
            return (addr < i.addr); 
        }
        bool operator==(InsnInstance i) const {
            return (addr == i.addr);
        }
    };

    // a Node in the DDG is an <Instruction,Absloc> pair. We add an Address
    // here because the Instruction doesn't contain its address and we want it
    // to be a) unique and b) findable. Finally, we create a tuple that represents
    // a candidate Node in our analysis data structures. 
    typedef std::pair<AbslocPtr, InsnInstance> cNode;
    typedef std::set<cNode> cNodeSet;

    // Temporary use: a data type for summarizing the definitions of Abslocs
    // from a particular block.
    // We use a map of cNodes to handle aliasing issues; see block comment
    // in intraFunctionCreator.C
    typedef std::map<AbslocPtr, cNodeSet> DefMap;

    // DefMaps for each Block. This is a global type.
    typedef std::map<Block *, DefMap> GenSet;

    // We also want to keep around kill information so we can efficiently
    // do the interprocedural analysis. This turns into a boolean, as we
    // don't really care _who_ kills (that's summarized in the gen set)
    // so long as _someone_ did.
    typedef std::map<AbslocPtr, bool> KillMap;
    typedef std::map<Block *, KillMap> KillSet;

    // A reaching definition is a Node that defines the (Insn,Absloc) pair currently
    // considered. We also tag it with the Block that contains the Node. 
    typedef std::pair<Block *, cNode> ReachingDefEntry;
    
    // We have a collection since there may be multiple reaching definitions. 
    typedef std::set<ReachingDefEntry> ReachingDefSet;
    
    // Given an Absloc, return the ReachingDefSet (reaching defs from predecessor blocks)
    typedef std::map<AbslocPtr, ReachingDefSet> ReachingDefsLocal;

    // A map from each Block to its ReachingDefsLocal data structure.    
    typedef std::map<Block *, ReachingDefsLocal> ReachingDefsGlobal;

 public:
    static intraFunctionDDGCreator create(Function *func);
    Graph::Ptr getDDG();

 private:
    intraFunctionDDGCreator(Function *f) : func(f) {};

    void analyze();

    void generateInterBlockReachingDefs(Flowgraph *CFG);

    void generateIntraBlockReachingDefs(std::set<Block *> &allBlocks);

    void initializeGenKillSets(std::set<Block *> &allBlocks);

    void updateDefSet(const Absloc::Ptr D,
                      DefMap &defMap,
                      cNode &cnode);

    void updateKillSet(const Absloc::Ptr D,
                       KillMap &kills);

    void merge(ReachingDefsLocal &target,
               ReachingDefsLocal &source);

    void calcNewOut(ReachingDefsLocal &out,
                    Block *current,
                    DefMap &gens,
                    KillMap &kills,
                    ReachingDefsLocal &in);
    
    void genSetToReachingDefs(Block *current,
                              const cNodeSet &gens,
                              ReachingDefSet &defs);

    void recordCallState(const Instruction &insn,
                         const Address &a,
                         const DefMap &localDefs,
                         const ReachingDefsLocal &reachingDefs);
    
    void getPredecessors(Block *block,
                         std::vector<Block *> &preds);

    void getSuccessors(Block *block,
                       std::vector<Block *> &succs);

    void debugLocalSet(const ReachingDefsLocal &set, char *str);
    void debugAbslocSet(const AbslocSet &a, char *str);
    void debugDefMap(const DefMap &d, char *str);

    NodePtr makeNodeFromCandidate(cNode cnode);

    bool isCall(Instruction i) const;

    GenSet allGens;
    KillSet allKills;
    ReachingDefsGlobal inSets;
    ReachingDefsGlobal outSets;
    Graph::Ptr DDG;
    Function *func;
};
