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

// $Id: osfDL.C,v 1.40 2004/04/02 06:34:13 jaw Exp $

#include "dyninstAPI/src/sharedobject.h"
#include "dyninstAPI/src/dynamiclinking.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/arch-alpha.h"
#include "dyninstAPI/src/inst-alpha.h"
#include "common/h/debugOstream.h"

extern unsigned enable_pd_sharedobj_debug;

#if ENABLE_DEBUG_CERR == 1
#define sharedobj_cerr if (enable_pd_sharedobj_debug) cerr
#else
#define sharedobj_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

#include <sys/types.h>
#include <sys/procfs.h>
#include <sys/auxv.h>
#include <sys/signal.h>
#include <sys/ptrace.h>
#include <sys/fault.h>
#include <dlfcn.h>
#include <poll.h>
#include <filehdr.h>

void generateBreakPoint(instruction &insn);

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

    // force load of object
    (void) proc->getImage()->getObject();

    // Use the symbol _call_add_pc_range_table as the test for a 
    // dynamically linked obj
    pdstring dyn_str = pdstring("__INIT_00_add_pc_range_table");
    internalSym dyn_sym;
    bool found = proc->findInternalSymbol(dyn_str, false, dyn_sym);
    if (!found) {
	// static, nothing to do.
        bpinfo("program is statically linked\n");
        return false;
    }

    proc->setDynamicLinking();
    dynlinked = true;

    return true;
}


// getSharedObjects: This routine is called after attaching to
// an already running process p, or when a process reaches the breakpoint at
// the entry point of main().  It gets all shared objects that have been
// mapped into the process's address space, and returns 0 on error or if 
// there are no shared objects.
// The assumptions are that the dynamic linker has already run, and that
// a /proc file descriptor has been opened for the application process.
// TODO: 
// dlclose events should result in a call to removeASharedObject
pdvector< shared_object *> *dynamic_linking::getSharedObjects() {
    
    int proc_fd = proc->getRepresentativeLWP()->get_fd();
    if(!proc_fd){ return 0;}

    // step 2: find the base address and file descriptor of ld.so.1
    pdvector<shared_object *> *result = new(pdvector<shared_object *>);

    // step 2: get the runtime loader table from the process
    Address ldr_base_addr;
    ldr_context first;
    ldr_module module;

    assert(proc->readDataSpace((const void*)LDR_BASE_ADDR,sizeof(Address), &ldr_base_addr, true));
    assert(proc->readDataSpace((const void*)ldr_base_addr,sizeof(ldr_context), &first, true));
    assert(proc->readDataSpace((const void *) first.head,sizeof(ldr_module), &module, true));
    
    bool first_time = true;
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
	if(obj_name != proc->getImage()->file() &&
	   obj_name != proc->getImage()->name()) {
        if((!(proc->wasExeced())) || (proc->wasExeced() && !first_time)){
            shared_object *newobj = new shared_object(obj_name,
                                                      offset,false,true,true,0);
            result->push_back(newobj);
        }
	}
    
	first_time = false;
	free(regions);
	assert(proc->readDataSpace((const void *) module.next,sizeof(ldr_module), &module,true));
    }
    return result;
}

bool dynamic_linking::handleIfDueToSharedObjectMapping(pdvector<shared_object *> **shObjects, u_int &changeType, bool & /* err */)
{
  Address pc;
  
  struct dyn_saved_regs regs;

  pc = proc->getRepresentativeLWP()->getActiveFrame().getPC();

  sharedLibHook *hook = reachedLibHook(pc);
  // dumpMap(proc->getProcFileDescriptor());
  if (force_library_load ||
      hook) {
      int event;
      if (hook) {
          event = hook->eventType();
      }
      else
          event = SLH_INSERT_POST;

      switch(event) {

    case SLH_INSERT_POST:
    {
        changeType = SHAREDOBJECT_ADDED;
        
        *shObjects = new pdvector<shared_object *>;
        // get list of current shared objects defined for the process
        pdvector<shared_object *> *curr_list = proc->sharedObjects();
        
        // get the list from the process via /proc
        pdvector<shared_object *> *new_shared_objs = getSharedObjects();
        
        for (unsigned int i=0; i < new_shared_objs->size(); i++) {
            pdstring new_name = ((*new_shared_objs)[i])->getName();
            
            unsigned int j;
            for (j=0; j < curr_list->size(); j++) {
                pdstring curr_name = ((*new_shared_objs)[j])->getName();
                if (curr_name == new_name) {
                    break;
                }
            }
            if (j == curr_list->size()) {
                (*shObjects)->push_back(((*new_shared_objs)[i]));
                (*new_shared_objs)[i] = NULL;
            }
        }
    }
    break;
    default:
        cerr << "Unhandled case in handleIfDueToSharedObject..." << endl;
        
        break;
      }

      if (force_library_load) return true;
      else {
          // delete new_shared_objs;
          // Get return address
          bool status = proc->getRepresentativeLWP()->getRegisters(&regs);
          
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
    instruction tempSpace[1024];
    
    // Replace the return instruction with a trap. We'll
    // patch up the return mutator-side, just like Solaris.

    // XXX - assume dlopen is less than 1K instructions long
    bool err;
    Address openAddr = proc->findInternalAddress("dlopen", false, err);
    assert(!err);
    proc->readDataSpace((void *)openAddr, INSN_SIZE*1024, tempSpace, true);
    /* Now look for the return address */
    for (int i=0; i < 1024; i++) {
        if (isReturn(tempSpace[i])) {
            ret_addr = openAddr + (i*INSN_SIZE);
            break;
        }
    }
    assert(ret_addr);

    sharedLibHook *sharedHook = new sharedLibHook(proc, SLH_INSERT_POST,
                                                  ret_addr);

    sharedLibHooks_.push_back(sharedHook);
    return true;
}

sharedLibHook::sharedLibHook(process *p, sharedLibHookType t, Address b)
        :proc_(p), type_(t), breakAddr_(b) {
    instruction trapInsn;
    
    proc_->readDataSpace((void *)breakAddr_, SLH_SAVE_BUFFER_SIZE,
                         (void *)saved_, true);

    generateBreakPoint((instruction &) trapInsn);

    proc_->writeDataSpace((caddr_t)breakAddr_, sizeof(instruction),
                         (caddr_t) &trapInsn);
}


sharedLibHook::~sharedLibHook() {
    proc_->writeDataSpace((void *)breakAddr_, sizeof(instruction), saved_);
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

