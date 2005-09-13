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

#ifndef _BPatch_flowGraph_h_
#define _BPatch_flowGraph_h_

#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_Set.h"
#include "BPatch_basicBlock.h"
#include "BPatch_basicBlockLoop.h"
#include "BPatch_eventLock.h"
#include "BPatch_loopTreeNode.h"

class int_function;
class process;
class pdstring;

class BPatch_edge;

typedef BPatch_basicBlockLoop BPatch_loop;

/** class which represents the control flow graph of a function
  * in a executable code. 
  *
  * @see BPatch_basicBlock
  * @see BPatch_basicBlockLoop
  */
#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_flowGraph

class BPATCH_DLL_EXPORT BPatch_flowGraph : public BPatch_eventLock {
  friend class BPatch_basicBlock;
  friend class BPatch_function;
  friend class int_function; // This is illegal here... keeps us from having to
                            // have a public constructor...  PDSEP
  friend std::ostream& operator<<(std::ostream&,BPatch_flowGraph&);
  friend void dfsCreateLoopHierarchy(BPatch_loopTreeNode * parent,
                                     BPatch_Vector<BPatch_basicBlockLoop *> &loops,
                                     pdstring level);
 
  BPatch_flowGraph (BPatch_function *func, 
		    bool &valid); 

  process *ll_proc() const; // Not implemented here to cut down on header files
  int_function *ll_func() const;
public:

  BPatch_process *getBProcess() const { return bproc; }
  BPatch_function *getBFunction() const { return func_; }
  BPatch_module *getModule() const { return mod; }
  //  End of deprecated function

  //  Functions for use by Dyninst users

  API_EXPORT_DTOR(_dtor, (),
  ~,BPatch_flowGraph,());

  /** returns the set of all basic blocks in the CFG */
  API_EXPORT(Int, (blocks),
  bool,getAllBasicBlocks,(BPatch_Set<BPatch_basicBlock*> &blocks)); 
  
  /** returns the vector of entry basic blocks to CFG */
  API_EXPORT(Int, (blocks),
  bool,getEntryBasicBlock,(BPatch_Vector<BPatch_basicBlock*> &blocks));
  
  /** returns the vector of exit basic blocks to CFG */
  API_EXPORT(Int, (blocks),
  bool,getExitBasicBlock,(BPatch_Vector<BPatch_basicBlock*> &blocks));
  
  /** returns the vector of loops in CFG */
  API_EXPORT(Int, (loops),
  bool,getLoops,(BPatch_Vector<BPatch_basicBlockLoop*> &loops));

  /** returns a vector of outer loops in the CFG */
  API_EXPORT(Int, (loops),
  bool,getOuterLoops,(BPatch_Vector<BPatch_basicBlockLoop*> &loops));

  /** creates the source line blocks of all blocks in CFG.
   * without calling this method line info is not available
   */
  API_EXPORT(Int, (),
  bool,createSourceBlocks,());
  
  /** fills the dominator and immediate-dom information of basic blocks.
   * without calling this method dominator info is not available
   */
  API_EXPORT_V(Int, (),
  void,fillDominatorInfo,());

  /** same as above, but for postdominator/immediate-postdom info 
   */
  API_EXPORT_V(Int, (),
  void,fillPostDominatorInfo,());

  /** return root of loop hierarchy  */
  API_EXPORT(Int, (),
  BPatch_loopTreeNode *,getLoopTree,());

  /** returns true if the cfg contains dynamic callsites */
  API_EXPORT(Int, (),
  bool,containsDynamicCallsites,());

  // for debugging, print loops with line numbers to stderr
  API_EXPORT_V(Int, (),
  void,printLoops,());

  API_EXPORT(Int, (name),
  BPatch_basicBlockLoop *,findLoop,(const char *name));

  /*
  API_EXPORT(Int, (edge),
  BPatch_point *,createInstPointAtEdge,(BPatch_edge *edge));
  */
  // Deprecated... use BPatch_edge->point() instead
  
  /** find instrumentation points specified by loc, add to points*/
  API_EXPORT(Int, (loc, loop),
  BPatch_Vector<BPatch_point*> *,
      findLoopInstPoints,(CONST_EXPORT BPatch_procedureLocation loc, 
                          BPatch_basicBlockLoop *loop));

 private:

  BPatch_function *func_;
  BPatch_process *bproc;
  BPatch_module *mod;

  /** set of basic blocks that are entry to the control flow graph*/
  BPatch_Set<BPatch_basicBlock*> entryBlock;

  /** set of basic blocks that are exit from the control flow graph */
  BPatch_Set<BPatch_basicBlock*> exitBlock;
  
  /** set of loops contained in control flow graph */
  BPatch_Set<BPatch_basicBlockLoop*> *loops;
  
  /** set of all basic blocks that control flow graph has */
  BPatch_Set<BPatch_basicBlock*, BPatch_basicBlock::compare> allBlocks;

  /** root of the tree of loops */
  BPatch_loopTreeNode *loopRoot;

  /** set of back edges */
  BPatch_Set<BPatch_edge*> *backEdges;
  
  /** three colors used in depth first search algorithm */
  static const int WHITE;
  static const int GRAY;
  static const int BLACK;
  
  /** flag that keeps whether dominator info is initialized*/
  bool isDominatorInfoReady;

  /** flag that keeps whether postdominator info is initialized*/
  bool isPostDominatorInfoReady;
  
  /** flag that keeps whether source block info is initialized*/
  bool isSourceBlockInfoReady;
  
  bool createBasicBlocks();
  
  /** create the tree of loops/callees for this flow graph */
  void createLoopHierarchy();
  
  void dfsVisitWithTargets(BPatch_basicBlock*,int*); 

  void dfsVisitWithSources(BPatch_basicBlock*,int*); 
  
  void findAndDeleteUnreachable();
  
  static void findBBForBackEdge(BPatch_edge*,
				BPatch_Set<BPatch_basicBlock*>&);


  void getLoopsByNestingLevel(BPatch_Vector<BPatch_basicBlockLoop*>&, 
			      bool outerMostOnly);
  
  bool dfsInsertCalleeIntoLoopHierarchy(BPatch_loopTreeNode *node, 
                                        int_function *func,
                                        unsigned long addr);

  void insertCalleeIntoLoopHierarchy(int_function * func, unsigned long addr);

  void dfsPrintLoops(BPatch_loopTreeNode *n);

  void assignAnExitBlockIfNoneExists();

  void createEdges();
  void createLoops();

  void dump();

  void findLoopExitInstPoints(BPatch_basicBlockLoop *loop,
                              BPatch_Vector<BPatch_point*> *points);

};


#endif /* _BPatch_flowGraph_h_ */
