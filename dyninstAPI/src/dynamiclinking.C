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

// $Id: dynamiclinking.C,v 1.10 2005/08/08 20:23:33 gquinn Exp $

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
#if defined(os_solaris)
    r_debug_addr = pDL->r_debug_addr;
    r_state = pDL->r_state;
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

bool sharedLibHook::reachedBreakAddr(Address b) const {
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
// process. Normally used to initialize the process-level shared objects list.

bool dynamic_linking::getSharedObjects(pdvector<mapped_object *> &mapped_objects) {
    pdvector<fileDescriptor> descs;
    
    if (!processLinkMaps(descs))
        return false;
    
    // Skip first entry: always the a.out
    for (unsigned i = 0; i < descs.size(); i++) {
        if (descs[i] != proc->getAOut()->getFileDesc()) {
#if 0
            fprintf(stderr, "DEBUG: match pattern %d, %d, %d, %d, %d\n",
                    descs[i].file() == proc->getAOut()->getFileDesc().file(),
                    descs[i].code() == proc->getAOut()->getFileDesc().code(),
                    descs[i].data() == proc->getAOut()->getFileDesc().data(),
                    descs[i].member() == proc->getAOut()->getFileDesc().member(),
                    descs[i].pid() == proc->getAOut()->getFileDesc().pid());
#endif
            mapped_object *newobj = mapped_object::createMappedObject(descs[i], proc);
            mapped_objects.push_back(newobj);
#if defined(cap_save_the_world)
            setlowestSObaseaddr(descs[i].code());
#endif
        }           
    }
    return true;
} /* end getSharedObjects() */


#if !defined(os_windows) // Where we don't need it and the compiler complains...

// findChangeToLinkMaps: This routine returns a vector of shared objects
// that have been deleted or added to the link maps as indicated by
// change_type.  If an error occurs it sets error_occured to true.
bool dynamic_linking::findChangeToLinkMaps(u_int &change_type,
					   pdvector<mapped_object *> &changed_objects) {
  
  // get list of current shared objects
  const pdvector<mapped_object *> &curr_list = proc->mappedObjects();
  if((change_type == SHAREDOBJECT_REMOVED) && (curr_list.size() == 0)) {
    return false;
  }

  // get the list from the process via /proc
  pdvector<fileDescriptor> new_descs;
  if (!processLinkMaps(new_descs))
      return false;

  // Some platforms we set here
  if (!change_type) {
      if (curr_list.size() > new_descs.size())
          change_type = SHAREDOBJECT_REMOVED;
      else if (curr_list.size() < new_descs.size())
          change_type = SHAREDOBJECT_ADDED;
      else
          change_type = SHAREDOBJECT_NOCHANGE;
  }
  
  
  // if change_type is add then figure out what has been added
  if(change_type == SHAREDOBJECT_ADDED) {
      // Look for the one that doesn't match
      for (unsigned int i=0; i < new_descs.size(); i++) {
	  bool found = false;
	  for (unsigned int j = 0; j < curr_list.size(); j++) {
#if 0
              fprintf(stderr, "Comparing %s/0x%x/0x%x/%s/%d to %s/0x%x/0x%x/%s/%d\n",
                      new_descs[i].file().c_str(),
                      new_descs[i].code(),
                      new_descs[i].data(),
                      new_descs[i].member().c_str(),
                      new_descs[i].pid(),
                      curr_list[j]->getFileDesc().file().c_str(),
                      curr_list[j]->getFileDesc().code(),
                      curr_list[j]->getFileDesc().data(),
                      curr_list[j]->getFileDesc().member().c_str(),
                      curr_list[j]->getFileDesc().pid());
#endif
              if (new_descs[i] == curr_list[j]->getFileDesc()) {
                  found = true;
                  break;
              }
	  }
	  if (!found) {
              mapped_object *newobj = mapped_object::createMappedObject(new_descs[i],
                                                                        proc);
              changed_objects.push_back(newobj);


          // SaveTheWorld bookkeeping
#if defined(cap_save_the_world)
              char *tmpStr = new char[1+strlen(newobj->fileName().c_str())];
              strcpy(tmpStr, newobj->fileName().c_str());
              if( !strstr(tmpStr, "libdyninstAPI_RT.so") && 
                  !strstr(tmpStr, "libelf.so")){
                  //bperr(" dlopen: %s \n", tmpStr);
                  newobj->openedWithdlopen();
              }
              setlowestSObaseaddr(newobj->codeBase());
              delete [] tmpStr;	              
              // SaveTheWorld bookkeeping
#endif

	  }
      }
  }
  // if change_type is remove then figure out what has been removed
  else if((change_type == SHAREDOBJECT_REMOVED) && (curr_list.size())) {
      // Look for the one that's not in descs
      bool stillThere[curr_list.size()];
      for (unsigned k = 0; k < curr_list.size(); k++) 
          stillThere[k] = false;
      
      for (unsigned int i=0; i < new_descs.size(); i++) {
	  for (unsigned int j = 0; j < curr_list.size(); j++) {
              if (new_descs[i] == curr_list[j]->getFileDesc()) {
                  stillThere[j] = true;
                  break;
              }
	  }
      }

      for (unsigned l = 0; l < curr_list.size(); l++) {
          if (!stillThere[l]) {
              changed_objects.push_back(curr_list[l]);
          }
      }
  }
  return true;
}

#else
bool dynamic_linking::findChangeToLinkMaps(u_int &,
                                           pdvector<mapped_object *> &) {
    return true;
}

#endif // defined(os_windows)
     
