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

#ifndef _BPatch_edge_h_
#define _BPatch_edge_h_

#include "BPatch_dll.h"

class BPatch_flowGraph;
class BPatch_basicBlock;
class BPatch_point;


// XXX ignores indirect jumps
typedef enum { 
    CondJumpTaken, CondJumpNottaken, UncondJump, NonJump 
} BPatch_edgeType;


/** An edge between two blocks
 */
class BPATCH_DLL_EXPORT BPatch_edge {

 public:

    BPatch_edge(BPatch_basicBlock *, BPatch_basicBlock *, 
                BPatch_flowGraph *, const unsigned char *);

    ~BPatch_edge();

    bool needsEdgeTramp();
    void dump();

    BPatch_edgeType type;
    BPatch_basicBlock *source;
    BPatch_basicBlock *target;

    BPatch_flowGraph *flowGraph;

    // conditional jumps create two edges, the taken edge and the
    // nottaken (fallthrough) edge. these edges point to each other
    // through this conditionalBuddy pointer
    BPatch_edge *conditionalBuddy;

    BPatch_point *point;
    void *instAddr;

    const unsigned char *relocationPointer;

    BPatch_point *instPoint();

    unsigned long addrInFunc;
    
 private:

    BPatch_edgeType BPatch_edge::getType();
    BPatch_point *createInstPointAtEdge();

};


#endif /* _BPatch_edge_h_ */
