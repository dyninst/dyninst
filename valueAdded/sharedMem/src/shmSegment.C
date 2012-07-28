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
//----------------------------------------------------------------------------
// $Id: shmSegment.C,v 1.1 2006/11/22 21:45:04 bernat Exp $
//----------------------------------------------------------------------------

#include "shmSegment.h"
#include "../h/SharedMem.h"
#include "sharedMemInternal.h"
#include "BPatch_process.h"
#include "BPatch_image.h"

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




