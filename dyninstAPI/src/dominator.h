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

class BPatch_basicBlock;
class BPatch_flowGraph;
class dominatorCFG;
class dominatorBB;

#include "dyninstAPI/h/BPatch_Set.h"
#include "dyninstAPI/h/BPatch_Vector.h"
#include "common/h/Dictionary.h"

class dominatorBB {
   friend class dominatorCFG;
protected:
   int dfs_no;
   int size;
   dominatorBB *semiDom;
   dominatorBB *immDom;
   dominatorBB *label;
   dominatorBB *ancestor;
   dominatorBB *parent;
   dominatorBB *child;

   BPatch_basicBlock *bpatchBlock;
   dominatorCFG *dom_cfg;

   BPatch_Set<dominatorBB *> bucket;
   BPatch_Vector<dominatorBB *> pred;
   BPatch_Vector<dominatorBB *> succ;   
 public:
   dominatorBB(BPatch_basicBlock *bb, dominatorCFG *dc);
   ~dominatorBB();
   dominatorBB *eval();
   void compress();
   void dominatorPredAndSucc();
   void postDominatorPredAndSucc();
   int sdno();
};

class dominatorCFG {
   friend class dominatorBB;
 protected:
   dictionary_hash<unsigned, dominatorBB *> map;
   BPatch_flowGraph *fg;
   BPatch_Vector<dominatorBB *> all_blocks;
   BPatch_Vector<dominatorBB *> sorted_blocks;
   int currentDepthNo;

   dominatorBB *entryBlock;
   dominatorBB *nullNode;

   void performComputation();
   void storeDominatorResults();
   void depthFirstSearch(dominatorBB *v);
   void eval(dominatorBB *v);
   void link(dominatorBB *v, dominatorBB *w);
   dominatorBB *bpatchToDomBB(BPatch_basicBlock *bb);

 public:
   dominatorCFG(BPatch_flowGraph *flowgraph);
   ~dominatorCFG();

   void calcDominators();
   void calcPostDominators();
};

