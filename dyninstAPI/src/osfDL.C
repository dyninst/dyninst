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

// $Id: osfDL.C,v 1.44 2005/08/03 05:28:16 bernat Exp $

#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/dynamiclinking.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/arch-alpha.h"
#include "dyninstAPI/src/inst-alpha.h"
#include "common/h/debugOstream.h"
#include "dyninstAPI/src/InstrucIter.h"

#include <sys/types.h>
#include <sys/procfs.h>
#include <sys/auxv.h>
#include <sys/signal.h>
#include <sys/ptrace.h>
#include <sys/fault.h>
#include <dlfcn.h>
#include <poll.h>
#include <filehdr.h>

#ifdef DEBUG
static void dumpMap(int proc_fd);
#endif

// PC register index into the gregset_t array for the alphas
#define PC_REGNUM 31
#define GP_REGNUM 27

char *readDataString(process *p, void *address)
{
    static char temp[1024];
    p->readDataSpace(address, sizeof(temp), temp, true);
    temp[1023] = '\0';
    return(temp);
}

typedef struct {
    Address unused1;
    Address unused2;
    Address head;
    Address tail;
} ldr_context;

typedef struct {
    Address next;
    Address previous;
    Address unused1;
    char *name;
    Address moduleInfoAddr;
    long moduleID;
    Address unused2;
    Address unused3;
    long nregions;
    Address regions;
} ldr_module;

typedef struct {
    long unused1;
    Address name;
    long protection;
    Address vaddr;
    Address mapaddr;
    long size;
    long unused2[5];
} ldr_region;

#define LDR_BASE_ADDR 0x3ffc0000000

bool dynamic_linking::initialize() {

    // step 1: figure out if this is a dynamic executable. 

    // Use the symbol _call_add_pc_range_table as the test for a 
  // dynamically linked obj
  pdstring dyn_str = pdstring("__INIT_00_add_pc_range_table");
  Symbol dyn_sym;
  if (!proc->getSymbolInfo(dyn_str, dyn_sym)) {
    // static, nothing to do.
    bpinfo("program is statically linked\n");
    return false;
  }
  
  dynlinked = true;
  
  return true;
}


// processLinkMaps: get a list of all loaded objects in fileDescriptor form.
bool dynamic_linking::processLinkMaps(pdvector<fileDescriptor> &descs) {
    
    int proc_fd = proc->getRepresentativeLWP()->get_fd();
    if(!proc_fd){ return false;}

    // step 2: get the runtime loader table from the process
    Address ldr_base_addr;
    ldr_context first;
    ldr_module module;

    assert(proc->readDataSpace((const void*)LDR_BASE_ADDR,sizeof(Address), &ldr_base_addr, true));
    assert(proc->readDataSpace((const void*)ldr_base_addr,sizeof(ldr_context), &first, true));
    assert(proc->readDataSpace((const void *) first.head,sizeof(ldr_module), &module, true));
    
    while (module.next != first.head) {
      if (module.nregions == 0)
	{ 
	  assert(proc->readDataSpace((const void *) module.next,sizeof(ldr_module), &module,true));
	  continue;
        }
      pdstring obj_name = pdstring(readDataString(proc, module.name));
      ldr_region *regions;
      
      regions = (ldr_region *) malloc(module.nregions * sizeof(ldr_region));
      assert(proc->readDataSpace((const void *) module.regions, 
				 sizeof(ldr_region)*module.nregions, regions, true));
      
      long offset = regions[0].mapaddr - regions[0].vaddr;
#ifdef notdef
      if (offset) {
	bperr("*** shared lib at non-default offset **: ");
	bperr("    %s\n", obj_name.c_str());
	bperr("    offset = %ld\n", offset);
      } else {
	bperr("*** shared lib **: ");
	bperr("    %s\n", obj_name.c_str());
	bperr("    at = %ld\n", regions[0].mapaddr);
      }
#endif
      
      for (int i=0; i < module.nregions; i++) {
	long newoffset = regions[i].mapaddr - regions[i].vaddr;
	if (newoffset != offset) {
	  bperr( "shared lib regions have different offsets\n");
	}
	regions[i].name = (long unsigned) readDataString(proc,
							 (void *) regions[i].name);
	// bperr("  region %d (%s) ", i, regions[i].name);
	// bperr("addr = %lx, ", regions[i].vaddr);
	// bperr("mapped at = %lx, ", regions[i].mapaddr);
	// bperr("size = %x\n", regions[i].size);
      }
      descs.push_back(fileDescriptor(obj_name, 
				     offset,
				     offset,
				     true));

      free(regions);
      assert(proc->readDataSpace((const void *) module.next,sizeof(ldr_module), &module,true));
    }
    return true;
}


bool dynamic_linking::handleIfDueToSharedObjectMapping(pdvector<mapped_object *> &changed_objects,
						       u_int &changeType)
{
  Address pc;
  
  struct dyn_saved_regs regs;
  
  pc = proc->getRepresentativeLWP()->getActiveFrame().getPC();
  
  sharedLibHook *hook = reachedLibHook(pc);
  // dumpMap(proc->getProcFileDescriptor());
  if (force_library_load ||
      hook) {
    fprintf(stderr, "Reached dlopen trap point, force %d\n", force_library_load);
    // findChangeToLinkMaps figures out the change type.

    if (!findChangeToLinkMaps(changeType,
			      changed_objects)) {
      fprintf(stderr, "findChange failed\n");
      return false;
    }
    
    if (force_library_load) return true;
    else {
      // Get return address
      proc->getRepresentativeLWP()->getRegisters(&regs);
      
      // We overwrote a return instruction to put the trap in.
      // We need to patch that up with a return to the caller
      
      Address retAddr = (regs.theIntRegs).regs[REG_RA];
      proc->getRepresentativeLWP()->changePC(retAddr, NULL);
      return true;
    }
  }
  return false;
}

/*
 * put code at the end of dlopen and dlclose to catch events
 */
bool dynamic_linking::installTracing()
{
    Address ret_addr = 0;
    instruction trapInsn;

    // Replace the return instruction with a trap. We'll
    // patch up the return mutator-side, just like Solaris.

    // There's actually two returns - handled now.

    pdvector<int_function *> dlopen_funcs;
    if (!proc->findFuncsByAll("dlopen", dlopen_funcs))
      return false;
    if (dlopen_funcs.size() > 1) {
      fprintf(stderr, "Warning, found multiple copies of dlopen, using the first\n");
    }
    Address openAddr = dlopen_funcs[0]->getAddress();

    InstrucIter dlopen_iter(dlopen_funcs[0]);
    while (dlopen_iter.hasMore()) {
      if (dlopen_iter.getInstruction().isReturn()) {
	ret_addr = *dlopen_iter;
	sharedLibHook *sharedHook = new sharedLibHook(proc, SLH_INSERT_POST,
						      ret_addr);
	sharedLibHooks_.push_back(sharedHook);
      }
      dlopen_iter++;
    }
    
    if (sharedLibHooks_.size() == 0)
      return false;
    return true;
}

sharedLibHook::sharedLibHook(process *p, sharedLibHookType t, Address b)
        :proc_(p), type_(t), breakAddr_(b), loadinst_(NULL) {
    instruction trapInsn;
    
    proc_->readDataSpace((void *)breakAddr_, SLH_SAVE_BUFFER_SIZE,
                         (void *)saved_, true);

    codeGen gen(instruction::size());
    instruction::generateTrap(gen);

    proc_->writeDataSpace((caddr_t)breakAddr_, gen.used(),
                          gen.start_ptr());
}


sharedLibHook::~sharedLibHook() {
    proc_->writeDataSpace((void *)breakAddr_, instruction::size(), saved_);
}

#ifdef DEBUG

static void dumpMap(int proc_fd) 
{
  int ret;
  int numMaps;
  prmap_t *maps;

  ret = ioctl(proc_fd, PIOCNMAP, &numMaps);
  assert(ret == 0);

  bperr("%d segments mapped\n", numMaps);

  maps = (prmap_t *) calloc(sizeof(prmap_t), numMaps+1);

  ret = ioctl(proc_fd, PIOCMAP, maps);
  assert(ret == 0);

  for (int i = 0; i < numMaps; i++) {
        bperr("map %d:\n", i);
        bperr("    start: %lx\n", maps[i].pr_vaddr);
        bperr("    size: %lx\n", maps[i].pr_size);
        bperr("    protection: ");
        if (maps[i].pr_mflags & MA_READ) bperr("READ ");
        if (maps[i].pr_mflags & MA_WRITE) bperr("WRITE ");
        if (maps[i].pr_mflags & MA_EXEC) bperr("EXEC ");
  }
}
#endif

