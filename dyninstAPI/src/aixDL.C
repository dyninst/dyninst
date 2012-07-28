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

// $Id: aixDL.C,v 1.79 2008/06/19 19:53:05 legendre Exp $

#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/dynamiclinking.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/symtab.h"
#include "common/h/arch.h"
#include "dyninstAPI/src/inst-power.h"
#include "common/h/debugOstream.h"
#include <sys/ptrace.h>
#include <sys/ldr.h>

#include "dyninstAPI/src/ast.h"


bool dynamic_linking::initialize() {
    // Since we read out of /proc/.../map, there is nothing to do here

    dynlinked = true;
    return true;
}

bool dynamic_linking::installTracing() {
    // We capture dlopen/dlclose by overwriting the return address
    // of their worker functions (load1 for dlopen, dlclose is TODO).
    // We get this from the (already-parsed) listing of libc.

  // Should check only libc.a...

  AstNodePtr retval = AstNode::operandNode(AstNode::ReturnVal, (void *)0);
  
  const char *loadfunc = NULL;
  switch (proc->getAddressWidth()) {
    case 4: loadfunc = "load1"; break;
    case 8: loadfunc = "uload"; break;
    default: assert(0 && "Unknown process address width");
  }

  instMapping *loadInst = new instMapping(loadfunc, "DYNINST_instLoadLibrary",
					  FUNC_EXIT | FUNC_ARG,
					  retval);
  instMapping *unloadInst = new instMapping("unload", "DYNINST_instLoadLibrary",
                                            FUNC_EXIT | FUNC_ARG,
                                            retval);
  loadInst->dontUseTrampGuard();
  unloadInst->dontUseTrampGuard();

  pdvector<instMapping *>instReqs;
  instReqs.push_back(loadInst);
  instReqs.push_back(unloadInst);
  proc->installInstrRequests(instReqs);
  if (loadInst->miniTramps.size()) {
      sharedLibHook *sharedHook = new sharedLibHook(proc, SLH_UNKNOWN,
                                                    loadInst);
      sharedLibHooks_.push_back(sharedHook);
      instru_based = true;
  }
  if (unloadInst->miniTramps.size()) {
      sharedLibHook *sharedHook = new sharedLibHook(proc, SLH_UNKNOWN,
                                                    unloadInst);
      sharedLibHooks_.push_back(sharedHook);
      instru_based = true;
  }

  if (sharedLibHooks_.size())
      return true;
  else
      return false;
}

// If result has entries, use them as a base (don't double-create)
bool dynamic_linking::processLinkMaps(pdvector<fileDescriptor> &result)
{
    // First things first, get a list of all loader info structures.
    int pid;

    pid = proc->getPid();
    prmap_t mapEntry;
    int iter = 0; // Starting with the a.out is _just fine_.

    bool is_aout = true;
    
    // We want to fill in this vector.

    int mapfd = proc->getRepresentativeLWP()->map_fd();
    do {
        if (sizeof(prmap_t) != pread(mapfd, &mapEntry, sizeof(prmap_t), iter*sizeof(prmap_t)))
           fprintf(stderr, "%s[%d]:  pread: %s\n", FILE__, __LINE__, strerror(errno));
        if (mapEntry.pr_size != 0) {
            prmap_t next;
            if (sizeof(prmap_t) != pread(mapfd, &next, sizeof(prmap_t), (iter+1)*sizeof(prmap_t)))
               fprintf(stderr, "%s[%d]:  pread: %s\n", FILE__, __LINE__, strerror(errno));
            if (strcmp(mapEntry.pr_mapname, next.pr_mapname)) {
                // Must only have a text segment (?)
                next.pr_vaddr = 0;
                iter++;
            }
            else {
                iter += 2;
            }
            
            char objname[256];
            objname[0] = '\0';
            if (mapEntry.pr_pathoff) {
               pread(mapfd, objname, 256,
                     mapEntry.pr_pathoff);
               if (objname[0] == '\0')
                  fprintf(stderr, "%s[%d]:  pread: %s\n", FILE__, __LINE__, 
                        strerror(errno));
            }
            else {
                objname[0] = objname[1] = objname[2] = 0;
            }

            Address textOrg = (Address) mapEntry.pr_vaddr;
            Address dataOrg = (Address) next.pr_vaddr;
            
            // Round up to next multiple of wordsize;
            
            if (textOrg % sizeof(unsigned))
                textOrg += sizeof(unsigned) - (textOrg % sizeof(unsigned));
            if (dataOrg % sizeof(unsigned))
                dataOrg += sizeof(unsigned) - (dataOrg % sizeof(unsigned));
            
	    fileDescriptor fda;
	    /* a.out is already added by setAout */
#if 0
	    if(is_aout)
	    {
				//fda = fileDescriptor(objname, textOrg, dataOrg, false);
				//fda.setMember(objname+1);
            			//result.push_back(fda);
	    }
#endif

	    if (!is_aout) {
           		fda = fileDescriptor(objname, 
                                     textOrg, dataOrg,
                                     true);
        	        fda.setMember(objname+strlen(objname)+1);
	        	result.push_back(fda);
            }
            is_aout = false;
            //fda.setPid(pid);
        }
        
    } while (mapEntry.pr_size != 0);

    return true;
}

sharedLibHook::sharedLibHook(process *p, sharedLibHookType t, Address b) 
        : proc_(p), type_(t), breakAddr_(b), loadinst_(NULL) {

    // Before putting in the breakpoint, save what is currently at the
    // location that will be overwritten.
    proc_->readDataSpace((void *)breakAddr_, SLH_SAVE_BUFFER_SIZE,
                         (void *)saved_, true);

    codeGen gen(instruction::size());
    insnCodeGen::generateTrap(gen);
    
    if (!proc_->writeDataSpace((caddr_t)breakAddr_,
                          gen.used(),
                          gen.start_ptr()))
        fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);
    
}

sharedLibHook::sharedLibHook(process *p, sharedLibHookType t, instMapping *inst) 
        : proc_(p), type_(t), breakAddr_(0), loadinst_(inst) {
}

sharedLibHook::~sharedLibHook() {
    if (!proc_->isAttached() || proc_->execing())
        return;
    
    if (breakAddr_)
        if (!proc_->writeDataSpace((void *)breakAddr_, SLH_SAVE_BUFFER_SIZE, saved_))
          fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);
    else if (loadinst_) {
        miniTramp *handle;
        for (unsigned i = 0; i < loadinst_->miniTramps.size(); i++) {
            handle = loadinst_->miniTramps[i];
            handle->uninstrument();
        }
        delete loadinst_;
    }
}

