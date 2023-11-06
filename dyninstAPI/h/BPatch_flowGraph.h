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

#ifndef _BPatch_flowGraph_h_
#define _BPatch_flowGraph_h_

#include <string>
#include <set>
#include <map>
#include "Annotatable.h"
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_Set.h"
#include "BPatch_basicBlock.h"
#include "BPatch_basicBlockLoop.h"
#include "BPatch_loopTreeNode.h"
#include "BPatch_edge.h"
#include "dyntypes.h"

class func_instance;
class AddressSpace;
class BPatch_edge;
class edge_instance;

typedef BPatch_basicBlockLoop BPatch_loop;

/** class which represents the control flow graph of a function
  * in a executable code. 
  *
  * @see BPatch_basicBlock
  * @see BPatch_basicBlockLoop
  */
namespace Dyninst {
    namespace PatchAPI{
        class PatchLoop;
    }
}    
class BPATCH_DLL_EXPORT BPatch_flowGraph : 
      public Dyninst::AnnotatableSparse 
{
  friend class BPatch_basicBlock;
  friend class BPatch_edge;
  friend class BPatch_function;
  friend class dominatorCFG;
  friend class func_instance; // This is illegal here... keeps us from having to
                            // have a public constructor...  PDSEP
  friend void dfsCreateLoopHierarchy(BPatch_loopTreeNode * parent,
                                     BPatch_Vector<BPatch_basicBlockLoop *> &loops,
                                     std::string level);
 
  BPatch_flowGraph (BPatch_function *func, bool &valid); 

  func_instance *ll_func() const;
  bool isValid_;

  std::map<const block_instance *, BPatch_basicBlock *> blockMap_;
  std::map<const edge_instance *, BPatch_edge *> edgeMap_;

public:

  //BPatch_process *getBProcess() const { return bproc; }
  BPatch_addressSpace *getAddSpace() const { return addSpace; }
  AddressSpace *getllAddSpace() const;
  BPatch_function *getFunction() const { return func_; }
  BPatch_module *getModule() const { return mod; }

  BPatch_basicBlock *findBlock(block_instance *b);
  BPatch_edge *findEdge(edge_instance *e);
  void invalidate(); // invoked when additional parsing takes place

  //  Functions for use by Dyninst users

  ~BPatch_flowGraph();

  /** returns the set of all basic blocks in the CFG */
  bool getAllBasicBlocks(BPatch_Set<BPatch_basicBlock*> &blocks); 
  bool getAllBasicBlocks(std::set<BPatch_basicBlock *> &blocks);
  
  /** returns the vector of entry basic blocks to CFG */
  bool getEntryBasicBlock(BPatch_Vector<BPatch_basicBlock*> &blocks);
  
  /** returns the vector of exit basic blocks to CFG */
  bool getExitBasicBlock(BPatch_Vector<BPatch_basicBlock*> &blocks);

  /** Finds the block containing a specific instruction. Warning:
      this method is slow! **/
  BPatch_basicBlock *findBlockByAddr(Dyninst::Address addr);
  
  /** returns the vector of loops in CFG */
  bool getLoops(BPatch_Vector<BPatch_basicBlockLoop*> &loops);

  /** returns a vector of outer loops in the CFG */
  bool getOuterLoops(BPatch_Vector<BPatch_basicBlockLoop*> &loops);

  /** creates the source line blocks of all blocks in CFG.
   * without calling this method line info is not available
   */
  bool createSourceBlocks();
  
  /** fills the dominator and immediate-dom information of basic blocks.
   * without calling this method dominator info is not available
   */
  void fillDominatorInfo();

  /** same as above, but for postdominator/immediate-postdom info 
   */
  void fillPostDominatorInfo();

  /** return root of loop hierarchy  */
  BPatch_loopTreeNode * getLoopTree();

  /** returns true if the cfg contains dynamic callsites */
  bool containsDynamicCallsites();

  // for debugging, print loops with line numbers to stderr
  void printLoops();

  BPatch_basicBlockLoop * findLoop(const char *name);

  bool isValid(); 

  /** find instrumentation points specified by loc, add to points*/
  BPatch_Vector<BPatch_point*> * 
      findLoopInstPoints(const BPatch_procedureLocation loc, 
                          BPatch_basicBlockLoop *loop);

 private:

  BPatch_function *func_;
  BPatch_addressSpace *addSpace;
  //  BPatch_process *bproc;
  BPatch_module *mod;

  /** set of loops contained in control flow graph */
  std::set<BPatch_basicBlockLoop*> *loops;
  
  /** set of all basic blocks that control flow graph has */
  std::set<BPatch_basicBlock*> allBlocks;

  /** root of the tree of loops */
  BPatch_loopTreeNode *loopRoot;

  /** set of back edges */
  std::set<BPatch_edge*> backEdges;
  
  /** flag that keeps whether dominator info is initialized*/
  bool isDominatorInfoReady;

  /** flag that keeps whether postdominator info is initialized*/
  bool isPostDominatorInfoReady;
  
  /** flag that keeps whether source block info is initialized*/
  bool isSourceBlockInfoReady;
  
  bool createBasicBlocks();
  
  void dfsVisitWithTargets(BPatch_basicBlock*,int*); 

  void dfsVisitWithSources(BPatch_basicBlock*,int*); 
  
  void findAndDeleteUnreachable();
  
  static void findBBForBackEdge(BPatch_edge*,
				std::set<BPatch_basicBlock*>&);


  void getLoopsByNestingLevel(BPatch_Vector<BPatch_basicBlockLoop*>&, 
			      bool outerMostOnly);
  
  void dfsPrintLoops(BPatch_loopTreeNode *n);

  std::map<Dyninst::PatchAPI::PatchLoop*, BPatch_basicBlockLoop*> _loop_map;  
  void createLoops();

  void dump();


  void findLoopExitInstPoints(BPatch_basicBlockLoop *loop,
                              BPatch_Vector<BPatch_point*> *points);

};


#endif /* _BPatch_flowGraph_h_ */
