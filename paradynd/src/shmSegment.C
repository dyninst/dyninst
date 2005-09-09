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
//----------------------------------------------------------------------------
// $Id: shmSegment.C,v 1.3 2005/09/09 18:08:03 legendre Exp $
//----------------------------------------------------------------------------

#include "common/h/headers.h"
#include "shmSegment.h"
#include "shmMgr.h"

const unsigned ShmSegment::cookie = 0xdeadbeef;


// Cross-platform shmSegment definitions
bool ShmSegment::attach(BPatch_process *appProc, Address baseAddr) {
    BPatch_Vector<BPatch_snippet *>init_args;
    BPatch_constExpr init_key(GetKey());
    BPatch_constExpr init_size(GetSize());
    BPatch_constExpr addr(baseAddr);
    init_args.push_back(&init_key);
    init_args.push_back(&init_size);
    init_args.push_back(&addr);

    assert(appProc->isStopped());
    BPatch_Vector<BPatch_function *>init_func;
    if ((NULL == appProc->getImage()->findFunction("RTsharedAttach",
                                                     init_func)) ||
        init_func.size() == 0) {
        fprintf(stderr, "Failed to find shared init function\n");
        return false;
    }
    assert(appProc->isStopped());
    BPatch_funcCallExpr init_expr(*(init_func[0]), init_args);
    assert(appProc->isStopped());
    
    // We return the address attached to by the child -- set up
    // argument grabbing

    assert(appProc->isStopped());
    baseAddrInApplic = (Address) appProc->oneTimeCode(init_expr);
    assert(appProc->isStopped());

    if (baseAddrInApplic != (Address) -1) {
        if (baseAddr &&
            (baseAddrInApplic != baseAddr)) {
            // We were given a base address, but didn't get it. This is bad.
            fprintf(stderr, "Error: attached to incorrect address\n");
            return false;
        }
        attached = true;
        return true;
    }
    fprintf(stderr, "Attach failed!\n");
    assert(appProc->isStopped());
    
    return false;
}

bool ShmSegment::detach(BPatch_process *appProc) {
    BPatch_Vector<BPatch_snippet *>init_args;
    BPatch_constExpr init_key(GetKey());
    BPatch_constExpr init_size(GetSize());
    BPatch_constExpr addr(baseAddrInApplic);
    init_args.push_back(&init_key);
    init_args.push_back(&init_size);
    init_args.push_back(&addr);

    BPatch_Vector<BPatch_function *>init_func;
    if ((NULL == appProc->getImage()->findFunction("RTsharedDetach",
                                                     init_func)) ||
        init_func.size() == 0) {
        fprintf(stderr, "Failed to find shared init function\n");
        return false;
    }
    BPatch_funcCallExpr init_expr(*(init_func[0]), init_args);
    
    // We return the address attached to by the child -- set up
    // argument grabbing

    baseAddrInApplic = (Address) appProc->oneTimeCode(init_expr);
    
    if (baseAddrInApplic != (Address) -1) {
        attached = true;
        return true;
    }
    return false;
}

Address ShmSegment::malloc(unsigned size) {
    if (!attached) {
        // Could try to attach here...
        return 0;
    }
    if (freespace < size) return 0;
    
    // Just carve a chunk off and update local variables
    Address ret = highWaterMark + baseAddrInDaemon;
    highWaterMark += size;
    freespace -= size;
    return ret;
}

void ShmSegment::free(Address /*addr*/) {
    // We need more sophisticated tracking before this
    // will work
}




