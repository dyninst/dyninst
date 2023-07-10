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
/* Public Interface */

#ifndef _PATCHAPI_DYNINST_CFG_H_
#define _PATCHAPI_DYNINST_CFG_H_

#include <assert.h>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "CFG.h"
#include "PatchCommon.h"
#include "PatchObject.h"
#include "Point.h"

namespace Dyninst {
namespace PatchAPI {

class PatchParseCallback;

class PatchEdge;
class PatchBlock;
class PatchFunction;
class PatchCallback;

class PATCHAPI_EXPORT PatchEdge {
   friend class PatchBlock;
   friend class PatchFunction;
   friend class PatchObject;
   friend class Callback;
   friend class PatchParseCallback;

  public:
   static PatchEdge *create(ParseAPI::Edge *,
                                            PatchBlock *src,
                                            PatchBlock *trg);
   PatchEdge(ParseAPI::Edge *internalEdge,
             PatchBlock *source,
             PatchBlock *target);
   PatchEdge(const PatchEdge *parent,
             PatchBlock *child_src,
             PatchBlock *child_trg);
   virtual ~PatchEdge();

   // Getters
   ParseAPI::Edge *edge() const;
   PatchBlock *src();
   PatchBlock *trg();
   ParseAPI::EdgeTypeEnum type() const;
   bool sinkEdge() const;
   bool interproc() const;
   bool intraproc() const { return !interproc(); }

   void remove(Point *);
   PatchCallback *cb() const;

   bool consistency() const;

   std::string format() const;

 protected:
    ParseAPI::Edge *edge_;
    PatchBlock *src_;
    PatchBlock *trg_;

    EdgePoints points_;
};

class PATCHAPI_EXPORT PatchBlock {
  friend class PatchEdge;
  friend class PatchFunction;
  friend class PatchObject;
   friend class PatchParseCallback;

  public:
   typedef std::map<Address, InstructionAPI::Instruction> Insns;
    typedef std::vector<PatchEdge*> edgelist;

    static PatchBlock *create(ParseAPI::Block *, PatchFunction *);
    PatchBlock(const PatchBlock *parblk, PatchObject *child);
    PatchBlock(ParseAPI::Block *block, PatchObject *obj);
    virtual ~PatchBlock();

    // Getters
    Address start() const;
    Address end() const;
    Address last() const;
    Address size() const;

    PatchFunction* getFunction(ParseAPI::Function*);
    bool isShared();
    int containingFuncs() const;
    void getInsns(Insns &insns) const;
    InstructionAPI::Instruction getInsn(Address a) const;
    std::string disassemble() const;
    bool containsCall() const { return 0 < numCallEdges(); }
    bool containsDynamicCall();
    std::string format() const;
    std::string long_format() const;
    PatchFunction* getCallee();

    ParseAPI::Block *block() const;
    PatchObject* object() const;
    PatchObject *obj() const { return object(); }
    const edgelist &sources();
    const edgelist &targets();
    
    PatchEdge *findSource(ParseAPI::EdgeTypeEnum type);
    PatchEdge *findTarget(ParseAPI::EdgeTypeEnum type);

    template <class OutputIterator> 
    void getFuncs(OutputIterator result);

    Point *findPoint(Location loc, Point::Type type, bool create = true);

   void remove(Point *);
   PatchCallback *cb() const;

   bool consistency() const;

   bool wasUserAdded() const;

   virtual void markModified()  {}

  protected:
    typedef enum {
      backwards,
      forwards } Direction;

    void removeSourceEdge(PatchEdge *e);
    void removeTargetEdge(PatchEdge *e);
    void destroyPoints();

    void addSourceEdge(PatchEdge *e, bool addIfEmpty = true);
    void addTargetEdge(PatchEdge *e, bool addIfEmpty = true);

    void splitBlock(PatchBlock *succ);
    int numRetEdges() const;
    int numCallEdges() const;

    ParseAPI::Block *block_;
    edgelist srclist_;
    edgelist trglist_;
    PatchObject* obj_;

    BlockPoints points_;
};

class PatchLoop;
class PatchLoopTreeNode;

class PATCHAPI_EXPORT PatchFunction {
   friend class PatchEdge;
   friend class PatchBlock;
   friend class PatchObject;
   friend class PatchParseCallback;
   friend class PatchModifier;

   public:
     struct compare {
       bool operator()(PatchBlock * const &b1,
                       PatchBlock * const &b2) const {
         if(b1->start() < b2->start())
           return true;
         else
           return false;
       }
     };
     typedef std::set<PatchBlock *, compare> Blockset;

     static PatchFunction *create(ParseAPI::Function *, PatchObject*);
     PatchFunction(ParseAPI::Function *f, PatchObject* o);
     PatchFunction(const PatchFunction* parFunc, PatchObject* child);
     virtual ~PatchFunction();

     const std::string &name() const { return func_->name(); }
     Address addr() const { return addr_;  }
     ParseAPI::Function *function() const { return func_; }
     PatchObject *obj() const { return obj_; }

     PatchBlock *entry();
     const Blockset &blocks();
     const Blockset &callBlocks();
     //const Blockset &returnBlocks();
     const Blockset &exitBlocks();

     Point *findPoint(Location loc, Point::Type type, bool create = true);

     // Moved to source code because we treat non-returning calls as exits
     bool verifyExit(PatchBlock *block) { return exitBlocks().find(block) != exitBlocks().end(); }
     bool verifyCall(PatchBlock *block) { return callBlocks().find(block) != callBlocks().end(); }

     // Const : no building the exit/call sets (returns true if set is empty)
     bool verifyExitConst(const PatchBlock *block) const { 
        return exit_blocks_.empty() || 
            exit_blocks_.find(const_cast<PatchBlock *>(block)) != exit_blocks_.end(); 
     }
     bool verifyCallConst(const PatchBlock *block) const { 
        return call_blocks_.empty() || 
            call_blocks_.find(const_cast<PatchBlock *>(block)) != call_blocks_.end(); 
     }

     // Fast access to a range of instruction points
     bool findInsnPoints(Point::Type type, PatchBlock *block,
                                         InsnPoints::const_iterator &start,
                                         InsnPoints::const_iterator &end);

   void remove(Point *);
   PatchCallback *cb() const;

   bool consistency() const;

   virtual void markModified() {}

    /* Loops */    
    PatchLoopTreeNode* getLoopTree();
    PatchLoop* findLoop(const char *name);
    bool getLoops(vector<PatchLoop*> &loops);
    bool getOuterLoops(vector<PatchLoop*> &loops);

    /* Dominator info */

    /* Return true if A dominates B in this function */
    bool dominates(PatchBlock* A, PatchBlock *B);
    PatchBlock* getImmediateDominator(PatchBlock *A);
    void getImmediateDominates(PatchBlock *A, set<PatchBlock*> &);
    void getAllDominates(PatchBlock *A, set<PatchBlock*> &);

    /* Post-dominator info */

    /* Return true if A post-dominates B in this function */
    bool postDominates(PatchBlock* A, PatchBlock *B);
    PatchBlock* getImmediatePostDominator(PatchBlock *A);
    void getImmediatePostDominates(PatchBlock *A, set<PatchBlock*> &);
    void getAllPostDominates(PatchBlock *A, set<PatchBlock*> &);

   protected:
     // For callbacks from ParseAPI to PatchAPI
     void removeBlock(PatchBlock *);
     void addBlock(PatchBlock *);
     void splitBlock(PatchBlock *first, PatchBlock *second);
     void destroyPoints();
     void destroyBlockPoints(PatchBlock *block);
     void invalidateBlocks();

     ParseAPI::Function *func_;
     PatchObject* obj_;
     Address addr_;

     Blockset all_blocks_;
     Blockset call_blocks_;
     Blockset return_blocks_;
     Blockset exit_blocks_;

     FuncPoints points_;
     // For context-specific
     std::map<PatchBlock *, BlockPoints> blockPoints_;
     std::map<PatchEdge *, EdgePoints> edgePoints_;

    /* Loop details*/
    bool _loop_analyzed; // true if loops in the function have been found and stored in _loops
    std::set<PatchLoop*> _loops;
    map<ParseAPI::Loop*, PatchLoop*> _loop_map;
    PatchLoopTreeNode *_loop_root; // NULL if the tree structure has not be calculated    
    void getLoopsByNestingLevel(vector<PatchLoop*>& lbb, bool outerMostOnly);
    void createLoops();
    void createLoopHierarchy();

    /* Dominator and post-dominator info details */
    bool isDominatorInfoReady;
    bool isPostDominatorInfoReady;
    void fillDominatorInfo();
    void fillPostDominatorInfo();
    /** set of basic blocks that this basicblock dominates immediately*/
    std::map<PatchBlock*, std::set<PatchBlock*>*> immediateDominates;
    /** basic block which is the immediate dominator of the basic block */
    std::map<PatchBlock*, PatchBlock*> immediateDominator;
    /** same as previous two fields, but for postdominator tree */
    std::map<PatchBlock*, std::set<PatchBlock*>*> immediatePostDominates;
    std::map<PatchBlock*, PatchBlock*> immediatePostDominator;

};


class PATCHAPI_EXPORT PatchLoop  
{
	friend class PatchFunction;
private:
        std::set<PatchEdge*> backEdges;

	// The entries of the loop
	std::set<PatchBlock*> entries;

        // the function this loop is part of
        PatchFunction * func;


	/** set of loops that are contained (nested) in this loop. */
        std::set<PatchLoop*> containedLoops;

	/** the basic blocks in the loop */
        std::set<PatchBlock*> basicBlocks;

        ParseAPI::Loop *loop_;

public:

	PatchLoop* parent;

        bool containsAddress(Address addr);
	bool containsAddressInclusive(Address addr);


        /** Sets edges to the set of back edges that define this loop,
            returns the number of back edges that define this loop */
        int getBackEdges(vector<PatchEdge*> &edges);

        /* returns the entry blocks of the loop.
	 * A natural loop has a single entry block
	 * and an irreducible loop has mulbile entry blocks
	 * */
	int getLoopEntries(vector<PatchBlock*>&);

	/** returns vector of contained loops */
        bool getContainedLoops(vector<PatchLoop*> &loops);

	/** returns vector of outer contained loops */
	bool getOuterLoops(vector<PatchLoop*> &loops);

	/** returns all basic blocks in the loop */
        bool getLoopBasicBlocks(vector<PatchBlock*> &blocks);

	/** returns all basic blocks in this loop, exluding the blocks
	    of its sub loops. */
        bool getLoopBasicBlocksExclusive(vector<PatchBlock*> &blocks);

        /** does this loop or its subloops contain the given block? */
        bool hasBlock(PatchBlock *b);

        /** does this loop contain the given block? */
        bool hasBlockExclusive(PatchBlock *b);

	/** returns true if this loop is a descendant of the given loop */
        bool hasAncestor(PatchLoop *loop);

	/** returns the function this loop is in */
        PatchFunction * getFunction();

        ~PatchLoop() { }

        std::string format() const;

private:
	/** constructor of class */
	PatchLoop(PatchObject* obj, ParseAPI::Loop *);

	/** get either contained or outer loops, determined by outerMostOnly */
	bool getLoops(vector<PatchLoop*>&, bool outerMostOnly) const;
}; // class PatchLoop


class PATCHAPI_EXPORT PatchLoopTreeNode {

 public:
    // A loop node contains a single Loop instance
    PatchLoop *loop;

    // The LoopTreeNode instances nested within this loop.
    vector<PatchLoopTreeNode *> children;

    //  LoopTreeNode::LoopTreeNode
    //  Create a loop tree node for Loop with name n 
    PatchLoopTreeNode(PatchObject *obj, ParseAPI::LoopTreeNode *l, std::map<ParseAPI::Loop*, PatchLoop*>&);

    //  Destructor
    ~PatchLoopTreeNode();

    //  LoopTreeNode::name
    //  Return the name of this loop. 
    const char * name(); 

    //  LoopTreeNode::getCalleeName
    //  Return the function name of the ith callee. 
    const char * getCalleeName(unsigned int i);

    //  LoopTreeNode::numCallees
    //  Return the number of callees contained in this loop's body. 
    unsigned int numCallees();

    //Returns a vector of the functions called by this loop.
    bool getCallees(vector<PatchFunction *> &v);
    

    //  find loop by hierarchical name
    PatchLoop * findLoop(const char *name);

 private:

    /** name which indicates this loop's relative nesting */
    char *hierarchicalName;

    // A vector of functions called within the body of this loop (and
    // not the body of sub loops). 
    vector<PatchFunction *> callees;

}; // class LoopTreeNode 




template <class OutputIterator>
void PatchBlock::getFuncs(OutputIterator result) {
  std::vector<ParseAPI::Function *> pFuncs;
  block()->getFuncs(pFuncs);
  for (unsigned i = 0; i < pFuncs.size(); ++i) {
    PatchFunction *func = getFunction(pFuncs[i]);
    *result = func;
    ++result;
  }
}

#define ASSERT_CONSISTENCY_FAILURE 1
#define CONSIST_FAIL {if (ASSERT_CONSISTENCY_FAILURE) assert(0); return false;}

}
}


#endif /* _PATCHAPI_DYNINST_CFG_H_ */
