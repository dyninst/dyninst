#ifndef _BPatch_flowGraph_h_
#define _BPatch_flowGraph_h_

#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_Set.h"
#include "BPatch_basicBlock.h"
#include "BPatch_basicBlockLoop.h"

class function_base;
class process;
class pdmodule;


// A class to represent the tree of nested loops and 
// functions (callees) in the flow graph
class BPATCH_DLL_EXPORT LoopTreeNode {
    friend class BPatch_flowGraph;

 public:
    BPatch_basicBlockLoop * loop;

    BPatch_Vector<LoopTreeNode *> children;

    BPatch_Vector<function_base *> callees;


    LoopTreeNode(BPatch_basicBlockLoop * l): loop(l) {}

    ~LoopTreeNode();

    bool containsAddress(unsigned long addr);

    const char * getFuncName(unsigned int i);
    
    unsigned int numCallees() { return callees.size(); }
};



/** class which represents the control flow graph of a function
  * in a executable code. 
  *
  * @see BPatch_basicBlock
  * @see BPatch_basicBlockLoop
  */
class BPATCH_DLL_EXPORT BPatch_flowGraph {
  friend class BPatch_basicBlock;
  friend ostream& operator<<(ostream&,BPatch_flowGraph&);

public:
 
  BPatch_flowGraph (function_base *func, 
		    process *proc, 
		    pdmodule *mod, 
		    bool &valid); 

  ~BPatch_flowGraph();

  const function_base *getFunction() const { return func; }
  process *getProcess() const { return proc; }
  const pdmodule *getModule() const { return mod; }

  /** returns the set of all basic blocks in the CFG */
  void getAllBasicBlocks(BPatch_Set<BPatch_basicBlock*>&);
  
  /** returns the vector of entry basic blocks to CFG */
  void getEntryBasicBlock(BPatch_Vector<BPatch_basicBlock*>&);
  
  /** returns the vector of exit basic blocks to CFG */
  void getExitBasicBlock(BPatch_Vector<BPatch_basicBlock*>&);
  
  /** returns the vector of loops in CFG */
  void getLoops(BPatch_Vector<BPatch_basicBlockLoop*>&);

  /** returns a vector of outer loops in the CFG */
  void getOuterLoops(BPatch_Vector<BPatch_basicBlockLoop*>&);
  
  /** creates the source line blocks of all blocks in CFG.
   * without calling this method line info is not available
   */
  void createSourceBlocks();
  
  /** fills the dominator and immediate-dom information of basic blocks.
   * without calling this method dominator info is not available
   */
  void fillDominatorInfo();

  /** same as above, but for postdominator/immediate-postdom info 
   */
  void fillPostDominatorInfo();

  /** return root of loop hierarchy  */
  LoopTreeNode * getLoopHierarchy();

  /** print the loops in this FG to stderr  */
  void printLoops();

  /** create the tree of loops/callees for this flow graph */
  void createLoopHierarchy();

 private:

  function_base *func;
  process *proc;
  pdmodule *mod;

  /** set of basic blocks that are entry to the control flow graph*/
  BPatch_Set<BPatch_basicBlock*> entryBlock;

  /** set of basic blocks that are exit from the control flow graph */
  BPatch_Set<BPatch_basicBlock*> exitBlock;
  
  /** set of loops contained in control flow graph */
  BPatch_Set<BPatch_basicBlockLoop*>* loops;
  
  /** set of all basic blocks that control flow graph has */
  BPatch_Set<BPatch_basicBlock*> allBlocks;

  /** root of the tree of loops */
  LoopTreeNode * loopRoot;
  
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
  
  void fillLoopInfo(BPatch_Set<BPatch_basicBlock*>**,BPatch_basicBlock**);
  
  void dfsVisit(BPatch_basicBlock*,int*,
		BPatch_Set<BPatch_basicBlock*>**,
		int* which=NULL,BPatch_basicBlock** order=NULL); 
  
  void findBackEdges(BPatch_Set<BPatch_basicBlock*>**);
  
  void findAndDeleteUnreachable();
  
  static void findBBForBackEdge(BPatch_basicBlock*,BPatch_basicBlock*,
				BPatch_Set<BPatch_basicBlock*>&);


  void getLoopsByNestingLevel(BPatch_Vector<BPatch_basicBlockLoop*>&, 
			      bool outerMostOnly);

  void insertCalleeIntoLoopHierarchy(function_base * func, unsigned long addr);

};


#endif /* _BPatch_flowGraph_h_ */
