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

// $Id: aixDL.C,v 1.57 2005/03/17 17:40:31 bernat Exp $

#include "dyninstAPI/src/sharedobject.h"
#include "dyninstAPI/src/dynamiclinking.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/arch-power.h"
#include "dyninstAPI/src/inst-power.h"
#include "common/h/debugOstream.h"
#include <sys/ptrace.h>
#include <sys/ldr.h>


bool dynamic_linking::initialize() {
    // Since we read out of /proc/.../map, there is nothing to do here

   proc->setDynamicLinking();
   dynlinked = true;

    return true;
}

bool dynamic_linking::installTracing() {
    // We capture dlopen/dlclose by overwriting the return address
    // of their worker functions (load1 for dlopen, dlclose is TODO).
    // We get this from the (already-parsed) listing of libc.
    
    pdvector<shared_object *> *objs = proc->sharedObjects();

    // Get libc
    shared_object *libc = NULL;
    fileDescriptor *libc_desc = NULL;
    for (unsigned i = 0; i < objs->size(); i++) {
        fileDescriptor_AIX *desc = (fileDescriptor_AIX *)(*objs)[i]->getFileDesc();
        if (((*objs)[i]->getName().suffixed_by("libc.a")) &&
            (desc->member() == "shr.o")) {
            libc_desc = (fileDescriptor *)desc;
            libc = ((*objs)[i]);
        }
    }
    assert(libc);

    // Now, libc may have been parsed... in which case we pull the function vector
    // from it. If not, we parse it manually.
    
    pdvector<int_function *> *loadFuncs;
    if (libc->isProcessed()) {
        loadFuncs = libc->findFuncVectorByPretty(pdstring("load1"));
    }
    else {
        image *libc_image = image::parseImage(libc_desc, libc_desc->addr());
        assert(libc_image);
        libc_image->defineModules(proc);
        loadFuncs = libc_image->findFuncVectorByPretty(pdstring("load1"));
    }
    assert(loadFuncs);
    assert(loadFuncs->size() > 0);
    
    int_function *loadfunc = (*loadFuncs)[0];
    assert(loadfunc);

    // There is no explicit place to put a trap, so we'll replace
    // the final instruction (brl) with a trap, and emulate the branch
    // mutator-side
    //
    //  JAW-- Alas this only works on AIX < 5.2.  The AIX 5.2 version of 
    //  load1 has 2 'blr' exit points, and we really want the first one.
    //  the last one is (apparently) the return that is used when there is
    //  is a failure.
    //
    // We used to find multiple exit points in findInstPoints. Now that
    // Laune's got a new version, we don't hack that any more. Instead, we
    // read in the function image, find the blr instructions, and overwrite
    // them by hand. This should be replaced with either a) instrumentation
    // or b) a better hook (aka r_debug in Linux/Solaris)

    Address loadStart = loadfunc->getOffset();
    loadStart += libc_desc->addr();
    Address loadEnd = loadStart + loadfunc->get_size();
    unsigned loadSize = loadEnd - loadStart + sizeof(instruction);
    instruction *func_image = (instruction *)malloc(loadSize);
    proc->readTextSpace((void *)loadStart, loadSize,
			(void *)func_image);

    for (unsigned i = 0; i < (loadSize/sizeof(instruction)); i++) {
      if (func_image[i].raw == BRraw) {
	sharedLibHook *sharedHook = new sharedLibHook(proc, SLH_UNKNOWN, 
						      // not used
						      loadStart + (i*sizeof(instruction)));
	sharedLibHooks_.push_back(sharedHook);
      }
    }

    return true;
    // TODO: handle dlclose as well
}

/* Parse a binary to extract all shared objects it
   contains, and create shared object objects for each
   of them */
pdvector< shared_object *> *dynamic_linking::getSharedObjects()
{
    pdvector <shared_object *> *new_list = processLinkMaps();
    return new_list;
}

pdvector <shared_object *> *dynamic_linking::processLinkMaps()
{
    // First things first, get a list of all loader info structures.
    int pid;
    int ret;

    pid = proc->getPid();
    
    prmap_t mapEntry;
    int iter = 2; // We start off with the third entry. The first two are
    // the executable file.
    
    // We want to fill in this vector.
    pdvector<shared_object *> *result = new(pdvector<shared_object *>);
    int mapfd = proc->getRepresentativeLWP()->map_fd();
    do {
        pread(mapfd, &mapEntry, sizeof(prmap_t), iter*sizeof(prmap_t));
        if (mapEntry.pr_size != 0) {
            prmap_t next;
            pread(mapfd, &next, sizeof(prmap_t), (iter+1)*sizeof(prmap_t));
            if (strcmp(mapEntry.pr_mapname, next.pr_mapname)) {
                // Must only have a text segment (?)
                next.pr_vaddr = 0;
                iter++;
            }
            else {
                iter += 2;
            }
            
            char objname[256];
            if (mapEntry.pr_pathoff) {
                pread(mapfd, objname, 256,
                      mapEntry.pr_pathoff);
            }
            else
            {
                objname[0] = objname[1] = objname[2] = 0;
            }
            
            fileDescriptor_AIX *fda = 
            new fileDescriptor_AIX(objname, objname+strlen(objname)+1,
                                   (Address) mapEntry.pr_vaddr,
                                   (Address) next.pr_vaddr,
                                   pid, false);

#ifdef DEBUG
            bperr( "Adding %s:%s with textorg %llx and dataorg %llx\n",
                    objname, objname+strlen(objname)+1,
                    mapEntry.pr_vaddr,
                    next.pr_vaddr);
#endif

            shared_object *newobj = new shared_object(fda,
                                                      false,
                                                      true,
                                                      true,
                                                      0,
						      proc);
            (*result).push_back(newobj);
        }
        
    } while (mapEntry.pr_size != 0);    
    return result;
}

// handleIfDueToSharedObjectMapping: returns true if the trap was caused
// by a change to the link maps
// p - process we're dealing with
// changed_objects -- set to list of new objects
// change_type -- set to 1 if added, 2 if removed
// error_occurred -- duh
// return value: true if there was a change to the link map,
// false otherwise
bool dynamic_linking::handleIfDueToSharedObjectMapping(pdvector<shared_object *> **changed_objects,
                                                       u_int &change_type, 
                                                       bool &error_occurred) {
    // Check to see if any thread hit the breakpoint we inserted
    pdvector<Frame> activeFrames;
    if (!proc->getAllActiveFrames(activeFrames)) {
        return false;
    }
    
    dyn_lwp *brk_lwp = NULL;
    sharedLibHook *hook;
    
    for (unsigned frame_iter = 0; frame_iter < activeFrames.size();frame_iter++)
    {
        hook = reachedLibHook(activeFrames[frame_iter].getPC());
        if (hook) {
            brk_lwp = activeFrames[frame_iter].getLWP();
            break;
        }
    }
    
    if (brk_lwp ||
        force_library_load) {
        // A thread hit the breakpoint, so process a change in the dynamic objects
        // List of current shared objects
        pdvector <shared_object *> *curr_list = proc->sharedObjects();
        // List of new shared objects (since we cache parsed objects, we
        // can go overboard safely)
        pdvector <shared_object *> *new_list = processLinkMaps();
        
        error_occurred = false; // Boy, we're optimistic.
        change_type = 0; // Assume no change
        
        // I've seen behavior where curr_list should be null, but instead has zero size
        if (!curr_list || (curr_list->size() == 0)) {
            change_type = 0;
            return false;
        }
        
        // Check to see if something was returned by getSharedObjects
        // They all went away? That's odd
        if (!new_list) {
            error_occurred = true;
            change_type = 2;
            return false;
        }
        
        if (new_list->size() == curr_list->size())
            change_type = 0;
        else if (new_list->size() > curr_list->size())
            change_type = 1; // Something added
        else
            change_type = 2; // Something removed
        
        
        *changed_objects = new(pdvector<shared_object *>);
        
        // if change_type is add, figure out what is new
        if (change_type == 1) {
            // Compare the two lists, and stick anything new on
            // the added_list vector (should only be one, but be general)
            bool found_object = false;
            for (u_int i = 0; i < new_list->size(); i++) {
                for (u_int j = 0; j < curr_list->size(); j++) {
                    // Check for equality -- file descriptor equality, nothing
                    // else is good enough.
                    shared_object *sh1 = (*new_list)[i];
                    shared_object *sh2 = (*curr_list)[j];
                    fileDescriptor *fd1 = sh1->getFileDesc();
                    fileDescriptor *fd2 = sh2->getFileDesc();
                    
                    if (*fd1 == *fd2) {
                        found_object = true;
                        break;
                    }
                }
                // So if found_object is true, we don't care. Set it to false and
                // loop. Otherwise, add this to the new list of objects
                if (!found_object) {
                    (**changed_objects).push_back(((*new_list)[i]));
                }
                else found_object = false; // reset
            }
        }
        else if (change_type == 2) {
            // Compare the two lists, and stick anything deleted on
            // the removed_list vector (should only be one, but be general)
            bool found_object = false;
            // Yes, this almost identical to the previous case. The for loops
            // are reversed, but that's it. Basically, find items in the larger
            // list that aren't in the smaller. 
            for (u_int j = 0; j < curr_list->size(); j++) {
                for (u_int i = 0; i < new_list->size(); i++) {
                    // Check for equality -- file descriptor equality, nothing
                    // else is good enough.
                    shared_object *sh1 = (*new_list)[i];
                    shared_object *sh2 = (*curr_list)[j];
                    fileDescriptor *fd1 = sh1->getFileDesc();
                    fileDescriptor *fd2 = sh2->getFileDesc();
                    
                    if (*fd1 == *fd2) {
                        found_object = true;
                        break;
                    }
                }
                // So if found_object is true, we don't care. Set it to false and
                // loop. Otherwise, add this to the new list of objects
                if (!found_object) {
                    (**changed_objects).push_back((*curr_list)[j]);
                }
                else found_object = false; // reset
            }
        }
        if (force_library_load)
            return true;
        else {
            // Now we need to fix the PC. We overwrote the return instruction,
            // so grab the value in the link register and set the PC to it.
            dyn_saved_regs regs;
            bool status = brk_lwp->getRegisters(&regs);
            assert(status == true);
            brk_lwp->changePC(regs.theIntRegs.__lr, NULL);
            return true;
        }
    }
    return false;
}

sharedLibHook::sharedLibHook(process *p, sharedLibHookType t, Address b) 
        : proc_(p), type_(t), breakAddr_(b) {

    // Before putting in the breakpoint, save what is currently at the
    // location that will be overwritten.
    proc_->readDataSpace((void *)breakAddr_, SLH_SAVE_BUFFER_SIZE,
                         (void *)saved_, true);

    instruction trap_insn;
    trap_insn.raw = BREAK_POINT_INSN;

    proc_->writeDataSpace((caddr_t)breakAddr_,
                         sizeof(instruction),&trap_insn.raw);
    
}

sharedLibHook::~sharedLibHook() {
    proc_->writeDataSpace((void *)breakAddr_, SLH_SAVE_BUFFER_SIZE, saved_);
}
