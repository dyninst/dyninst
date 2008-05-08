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

#define BPATCH_FILE

#include "util.h"
#include "symtab.h"

#include "BPatch_edge.h"
#include "BPatch_flowGraph.h"
#include "BPatch_basicBlock.h"
#include "instPoint.h"
#include "process.h"
#include "InstrucIter.h"
#include "BPatch_process.h"

string 
edge_type_string(BPatch_edgeType t)
{ 
    string ts = "Invalid Edge Type";
    switch (t) {
    case CondJumpTaken: { ts = "CondJumpTaken"; break; }
    case CondJumpNottaken: { ts = "CondJumpNottaken"; break; }
    case UncondJump: { ts = "UncondJump"; break; } 
    case NonJump: { ts = "NonJump"; break; }
    }
    return ts;
}

BPatch_edgeType 
BPatch_edge::getTypeInt()
{
    EdgeTypeEnum lltype;
    lltype = source->lowlevel_block()->getTargetEdgeType(target->lowlevel_block());

    switch(lltype) {
        case ET_NOEDGE: { return NonJump; break; }
        case ET_COND_TAKEN: { return CondJumpTaken; break; }
        case ET_COND_NOT_TAKEN: { return CondJumpNottaken; break; }
        case ET_DIRECT:
        case ET_INDIR: { return UncondJump; break; }
        default: { return NonJump; break; }
    }
}


void BPatch_edge::BPatch_edgeInt(BPatch_basicBlock *s, 
                                 BPatch_basicBlock *t, 
                                 BPatch_flowGraph *fg)
    
{
    assert(s != NULL);
    assert(t != NULL);
    
    source = s;
    target = t;
    flowGraph = fg;
    // point is set when this edge is instrumented. instAddr is set
    // when either this edge or its conditional buddy is instrumented
    point = NULL;    

    type = getType();
}

 
void BPatch_edge::BPatch_edge_dtor()
{
    //fprintf(stderr,"~BPatch_edge\n");
}


void BPatch_edge::dumpInt()
{
    string ts = edge_type_string(type);

//     fprintf(stderr," %3u --> %3u\n",
//             source->blockNo(),
//             target->blockNo());

//     fprintf(stderr,"  (b%u 0x%x 0x%x) --> (b%u 0x%x 0x%x)\n",
//             source->blockNo(),
//             source->getRelStart(),
//             source->getRelLast(),
//             target->blockNo(),
//             target->getRelStart(),
//             target->getRelLast());

    fprintf(stderr," 0x%p, 0x%p --> 0x%p, 0x%p %s\n",
            (void *)source->getStartAddress(),
            (void *)source->getEndAddress(),
            (void *)target->getStartAddress(),
            (void *)target->getEndAddress(),
	    edge_type_string(type).c_str());

}


#if 0
// Only edges created by conditional jumps need edge trampolines
bool BPatch_edge::needsEdgeTrampInt()
{
    return type == CondJumpNottaken || type == CondJumpTaken;
}
#endif

BPatch_basicBlock *BPatch_edge::getSourceInt() {
  return source;
}

BPatch_basicBlock *BPatch_edge::getTargetInt() {
  return target;
}

BPatch_point *BPatch_edge::getPointInt()
{
   if (!point) {
      // BPatch_points typically represent an instruction. This doesn't;
      // we can have two per instruction (fallthrough edge and target
      // edge). Argh. 
      assert(source);
      assert(target);
      Address lastInsnAddr = (Address) source->getLastInsnAddress();
      
      
      AddressSpace *as = flowGraph->getAddSpace()->getAS();
      assert(as);
      int_function *f = flowGraph->getBFunction()->lowlevel_func();

      instPoint *ip = instPoint::createArbitraryInstPoint(lastInsnAddr, as, f);
      
      if (ip == NULL) {
         fprintf(stderr, "Failed to find inst point at address 0x%lx\n",
                 lastInsnAddr);
         return NULL;
      }
      
      BPatch_point *newPoint = new BPatch_point(flowGraph->getAddSpace(),
                                                flowGraph->getBFunction(),
                                                this,
                                                ip);
      if (newPoint) {
         point = newPoint;
      }
      else {
         fprintf(stderr, "BPatch_edge: didn't create point!\n");
      }
   }
   return point;
}

