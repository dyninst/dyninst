/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

// $Id: dynamiclinking.C,v 1.22 2007/12/12 22:20:42 roundy Exp $

// Cross-platform dynamic linking functions

#include "dyninstAPI/src/dynamiclinking.h"
#include "dyninstAPI/src/process.h"
#include "mapped_object.h"
#include "dyn_thread.h"

dynamic_linking::dynamic_linking(process *p): proc(p), dynlinked(false),
                                              dlopen_addr(0), 
					      instru_based(false),
                                              force_library_load(false), 
                                              lowestSObaseaddr(0),
                                              dlopenUsed(false) 
{ 
}

dynamic_linking::dynamic_linking(const dynamic_linking *pDL,
                                 process *child): proc(child),
                                                  dynlinked(pDL->dynlinked),
                                                  dlopen_addr(pDL->dlopen_addr),
                                                  instru_based(pDL->instru_based),
                                                  force_library_load(pDL->force_library_load),
                                                  lowestSObaseaddr(pDL->lowestSObaseaddr),
                                                  dlopenUsed(pDL->dlopenUsed)
{
#if defined(os_linux)
    r_debug_addr = pDL->r_debug_addr;
    r_brk_target_addr = pDL->r_brk_target_addr;
    previous_r_state = pDL->previous_r_state;
#endif
    for (unsigned i = 0; i < pDL->sharedLibHooks_.size(); i++) {
        sharedLibHooks_.push_back(new sharedLibHook(pDL->sharedLibHooks_[i],
                                                    child));
    }
}


dynamic_linking::~dynamic_linking() {
    uninstallTracing();
}



// And unload the tracing breakpoints inserted
bool dynamic_linking::uninstallTracing() 
{
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

dyn_lwp *dynamic_linking::findLwpAtLibHook(process *proc, sharedLibHook **hook_handle)
{
  pdvector<dyn_thread *>::iterator iter = proc->threads.begin();

  dyn_lwp *brk_lwp = NULL;
  sharedLibHook *hook = NULL;

  while (iter != proc->threads.end()) {
    dyn_thread *thr = *(iter);
    dyn_lwp *cur_lwp = thr->get_lwp();

    if(cur_lwp->status() == running) {
      iter++;
      continue;  // if lwp is running couldn't have hit load library trap
    }

    Frame lwp_frame = cur_lwp->getActiveFrame();
    hook = reachedLibHook(lwp_frame.getPC());
    if (hook) {
      brk_lwp = cur_lwp;
      if (hook_handle)
        *hook_handle = hook;
      break;
    }

    iter++;
  }
  return brk_lwp;
}


bool sharedLibHook::reachedBreakAddr(Address b) const {
    if (breakAddr_ == 0) return false;

#if defined(arch_x86) || defined(arch_x86_64)
    return ((b-1) == breakAddr_);
#else
    return (b == breakAddr_); 
#endif
};

sharedLibHook::sharedLibHook(const sharedLibHook *pSLH, process *child) :
    proc_(child),
    type_(pSLH->type_),
    breakAddr_(pSLH->breakAddr_),
    loadinst_(NULL)
{
    // FIXME
    if (pSLH->loadinst_) {
        loadinst_ = new instMapping(pSLH->loadinst_, child);
    }
    memcpy(saved_, pSLH->saved_, SLH_SAVE_BUFFER_SIZE);
}

// getSharedObjects: gets a complete list of shared objects in the
// process. Normally used to initialize the process-level shared
// objects list.  This function is only called once, and creates
// mapped objects for all shared libraries
bool dynamic_linking::getSharedObjects(pdvector<mapped_object *> &mapped_objects) 
{
    pdvector<fileDescriptor> descs;
    
    if (!processLinkMaps(descs))
        return false;

    // Skip entries that have already been added
    for (unsigned i = 0; i < descs.size(); i++) {
         if (!proc->findObject(descs[i]) && 
             !strstr(descs[i].file().c_str(),"ld.so.cache")) {
#ifdef DEBUG_UNDEFINED
            fprintf(stderr, "DEBUG: match pattern %d, %d, %d, %d, %d\n",
                    descs[i].file() == proc->getAOut()->getFileDesc().file(),
                    descs[i].code() == proc->getAOut()->getFileDesc().code(),
                    descs[i].data() == proc->getAOut()->getFileDesc().data(),
                    descs[i].member() == proc->getAOut()->getFileDesc().member(),
                    descs[i].pid() == proc->getAOut()->getFileDesc().pid());
#endif
            mapped_object *newobj = 
               mapped_object::createMappedObject(descs[i], proc);
            if (newobj == NULL) continue;
            mapped_objects.push_back(newobj);
        }
    }
    return true;
} /* end getSharedObjects() */


#if !defined(os_windows) // Where we don't need it and the compiler complains...

// findChangeToLinkMaps: This routine returns a vector of shared
// objects that have been added or deleted to the link maps
// corresponding to a true or false entry in is_new_object.  
//
// Return false only if there was some problem processing the link maps
bool dynamic_linking::findChangeToLinkMaps(pdvector<mapped_object *> &changed_objects,
					   pdvector<bool> &is_new_object) 
{
  const pdvector<mapped_object *> &curr_list = proc->mappedObjects();
  pdvector<fileDescriptor> new_descs;
  if (!processLinkMaps(new_descs)) {
      return false;
  }

  // match up objects between the two lists, "consumed" tracks objects
  // in new_descs that we've matched against curr_list
  unsigned curr_size = curr_list.size();
  unsigned descs_size = new_descs.size();
  bool consumed [descs_size];
  memset(consumed, 0, sizeof(bool) * descs_size);
  for (unsigned int i = 0; i < curr_size; i++) {
     if (curr_list[i] == proc->getAOut()) {
         continue; // a.out is not listed in the link maps
     }
     bool found = false;
     for (unsigned int j=0; !found && j < descs_size; j++) {
         if (!consumed[j] && new_descs[j] == curr_list[i]->getFileDesc()) {
             found = true;
             consumed[j] = true;
         }
     }
     if (!found) {
        changed_objects.push_back(curr_list[i]);
        is_new_object.push_back(false);
     }
  }
  for (unsigned int i=0; i < descs_size; i++) {
     if (consumed[i] == false 
         && new_descs[i] != proc->getAOut()->getFileDesc()) {
         mapped_object *newobj = 
             mapped_object::createMappedObject(new_descs[i], proc);
         if (newobj != NULL) {
             changed_objects.push_back(newobj);
             is_new_object.push_back(true);
         }
     }
  }
  return true;
}

#else
bool dynamic_linking::findChangeToLinkMaps(pdvector<mapped_object *> &,
					   pdvector<bool> &) 
{
    return true;
}

#endif // defined(os_windows)
     
