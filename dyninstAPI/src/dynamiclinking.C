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

// $Id: dynamiclinking.C,v 1.7 2005/03/17 23:27:25 bernat Exp $

// Cross-platform dynamic linking functions

#include "dyninstAPI/src/dynamiclinking.h"
#include "dyninstAPI/src/process.h"

dynamic_linking::dynamic_linking(process *p): proc(p), dynlinked(false),
                                              dlopen_addr(0), 
					      instru_based(false),
                                              force_library_load(false), 
                                              lowestSObaseaddr(0),
                                              dlopenUsed(false) 
{ 
}

dynamic_linking::dynamic_linking(process *p,
                                 dynamic_linking *d): proc(p),
                                                      dynlinked(d->dynlinked),
                                                      dlopen_addr(d->dlopen_addr),
						      instru_based(d->instru_based),
                                                      force_library_load(d->force_library_load),
                                                      lowestSObaseaddr(d->lowestSObaseaddr),
                                                      dlopenUsed(d->dlopenUsed)
{
#if defined(os_linux)
    r_debug_addr = d->r_debug_addr;
    r_brk_target_addr = d->r_brk_target_addr;
    previous_r_state = d->previous_r_state;
#endif
#if defined(os_solaris)
    r_debug_addr = d->r_debug_addr;
    r_state = d->r_state;
#endif
#if defined(os_irix)
    libc_obj= d->libc_obj;
    for (unsigned j = 0; j < d->rld_map.size(); j++) {
        pdElfObjInfo *obj = d->rld_map[j];
        pdElfObjInfo *newobj = new pdElfObjInfo(*obj);
        rld_map.push_back(newobj);
    }
    
#endif
    for (unsigned i = 0; i < d->sharedLibHooks_.size(); i++) {
        sharedLibHooks_.push_back(new sharedLibHook(proc,
                                                    d->sharedLibHooks_[i]));
    }
}


dynamic_linking::~dynamic_linking() {
    uninstallTracing();
}



// And unload the tracing breakpoints inserted
bool dynamic_linking::uninstallTracing() {
    for (unsigned i= 0; i < sharedLibHooks_.size(); i++)
        delete sharedLibHooks_[i];
    sharedLibHooks_.resize(0);
    return true;
}

sharedLibHook *dynamic_linking::reachedLibHook(Address a) {
    for (unsigned i = 0; i < sharedLibHooks_.size(); i++)
        if (sharedLibHooks_[i]->reachedBreakAddr(a))
            return sharedLibHooks_[i];
    return NULL;
}

sharedLibHook::sharedLibHook(process *p, sharedLibHook *h) {
    proc_ = p;
    type_ = h->type_;
    breakAddr_ = h->breakAddr_;
    memcpy(h->saved_, saved_, SLH_SAVE_BUFFER_SIZE);
}

     
