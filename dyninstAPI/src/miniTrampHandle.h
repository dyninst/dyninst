
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

// $Id: miniTrampHandle.h,v 1.3 2004/03/23 01:12:06 eli Exp $

#ifndef MINI_TRAMP_HANDLE_H
#define MINI_TRAMP_HANDLE_H

#include "common/h/Types.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/codeRange.h"

// Callback func for deletion of a minitramp
class miniTrampHandle;
typedef void (*miniTrampHandleFreeCallback)(void *, miniTrampHandle *);

// The new miniTrampHandle class -- description of a particular minitramp.
class miniTrampHandle : public codeRange {
    // Global numbering of minitramps
    static int _id;
  public:

    // IDs
    enum { uninitialized_id = -1 };
    static int get_new_id() { return _id++; }

    miniTrampHandle() : ID(uninitialized_id), miniTrampBase(0), returnAddr(0), 
    baseTramp(NULL), cost(0), callback(NULL), callbackData(NULL) {
    }

    // Fork constructor
    miniTrampHandle(const miniTrampHandle *m, trampTemplate *t) : ID(m->ID),
    miniTrampBase(m->miniTrampBase), returnAddr(m->returnAddr),
    baseTramp(t), when(m->when), cost(m->cost),
    // Uhh... not sure whether to clone callback data or not!
    callback(m->callback), callbackData(m->callbackData) {};
    
    
    ~miniTrampHandle() {
        if (callback)
            (*callback)(callbackData, this);  
    }

    // Get address of the branch in the base tramp to the current minitramp
    // Defined in inst.C
    Address getBaseBranchAddr() const;
    Address getBaseReturnAddr() const;
    Address get_address() const {
       return miniTrampBase;
    }
    unsigned get_size() const {
       return (returnAddr - miniTrampBase);
    }
  
  void registerCallback(miniTrampHandleFreeCallback cb, void *data) {
    callback = cb;
    callbackData = data;
  };

  int ID;                    // used to match up miniTrampHandles in forked procs
  Address miniTrampBase;     // base of code
  Address returnAddr;      // Offset from base to the return addr


  // "Parent" instPoint.
  trampTemplate *baseTramp;
  
  callWhen when; /* Pre or post */

  int cost;		     // cost in cycles of this inst req.

  // Material to check when deleting this miniTrampHandle
  miniTrampHandleFreeCallback callback; /* Callback to be made when
					instance is deleted */
  void *callbackData;                /* Associated data */
};

#endif
