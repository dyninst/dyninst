/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: osfDL.C,v 1.29 2003/03/08 01:23:31 bernat Exp $

#include "dyninstAPI/src/sharedobject.h"
#include "dyninstAPI/src/osfDL.h"
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

// getSharedObjects: This routine is called after attaching to
// an already running process p, or when a process reaches the breakpoint at
// the entry point of main().  It gets all shared objects that have been
// mapped into the process's address space, and returns 0 on error or if 
// there are no shared objects.
// The assumptions are that the dynamic linker has already run, and that
// a /proc file descriptor has been opened for the application process.
// TODO: 
// dlclose events should result in a call to removeASharedObject
pdvector< shared_object *> *dynamic_linking::getSharedObjects(process *p) {
    
    // step 1: figure out if this is a dynamic executable. 

    // force load of object
    (void) p->getImage()->getObject();


    // Use the symbol _call_add_pc_range_table as the test for a 
    // dynamically linked obj
    string dyn_str = string("__INIT_00_add_pc_range_table");
    internalSym dyn_sym;
    bool found = p->findInternalSymbol(dyn_str, false, dyn_sym);
    if (!found) {
	// static, nothing to do.
	printf("program is statically linked\n");
	return 0;
    }
   
    int proc_fd = p->getDefaultLWP()->get_fd();
    if(!proc_fd){ return 0;}

    // step 2: find the base address and file descriptor of ld.so.1
    pdvector<shared_object *> *result = new(pdvector<shared_object *>);

    // step 2: get the runtime loader table from the process
    Address ldr_base_addr;
    ldr_context first;
    ldr_module module;

    assert(p->readDataSpace((const void*)LDR_BASE_ADDR,sizeof(Address), &ldr_base_addr, true));
    assert(p->readDataSpace((const void*)ldr_base_addr,sizeof(ldr_context), &first, true));
    assert(p->readDataSpace((const void *) first.head,sizeof(ldr_module), &module, true));
    
    bool first_time = true;
    while (module.next != first.head) {
        if (module.nregions == 0)
	{ 
		assert(p->readDataSpace((const void *) module.next,sizeof(ldr_module), &module,true));
		continue;
        }
        string obj_name = string(readDataString(p, module.name));
	ldr_region *regions;

	regions = (ldr_region *) malloc(module.nregions * sizeof(ldr_region));
	assert(p->readDataSpace((const void *) module.regions, 
	    sizeof(ldr_region)*module.nregions, regions, true));

	long offset = regions[0].mapaddr - regions[0].vaddr;
#ifdef notdef
	if (offset) {
	    fprintf(stderr, "*** shared lib at non-default offset **: ");
	    fprintf(stderr, "    %s\n", obj_name.c_str());
	    fprintf(stderr, "    offset = %ld\n", offset);
	} else {
	    fprintf(stderr, "*** shared lib **: ");
	    fprintf(stderr, "    %s\n", obj_name.c_str());
	    fprintf(stderr, "    at = %ld\n", regions[0].mapaddr);
	}
#endif

	for (int i=0; i < module.nregions; i++) {
	    long newoffset = regions[i].mapaddr - regions[i].vaddr;
	    if (newoffset != offset) {
		fprintf(stderr, "shared lib regions have different offsets\n");
	    }
	    regions[i].name = (long unsigned) readDataString(p,
		(void *) regions[i].name);
	    // printf("  region %d (%s) ", i, regions[i].name);
	    // printf("addr = %lx, ", regions[i].vaddr);
	    // printf("mapped at = %lx, ", regions[i].mapaddr);
	    // printf("size = %x\n", regions[i].size);
	}
	if(obj_name != p->getImage()->file() &&
	   obj_name != p->getImage()->name()) {
	   if((!(p->wasExeced())) || (p->wasExeced() && !first_time)){
	       shared_object *newobj = new shared_object(obj_name,
		       offset,false,true,true,0);
	       result->push_back(newobj);
	   }
	}

	first_time = false;
	free(regions);
	assert(p->readDataSpace((const void *) module.next,sizeof(ldr_module), &module,true));
    }
    p->setDynamicLinking();
    dynlinked = true;

    return result;
}

bool dynamic_linking::handleIfDueToSharedObjectMapping(process *proc, 
    pdvector<shared_object *> **shObjects, u_int &changeType, bool & /* err */)
{
  Address pc;
  
  if (dlopenRetAddr == 0) return false;

  dyn_saved_regs *regs;

  pc = proc->getDefaultLWP()->getActiveFrame().getPC();

  // dumpMap(proc->getProcFileDescriptor());
  if (pc == dlopenRetAddr) {
      fprintf(stderr, "PC is at 0x%x\n", pc);
      
      regs = proc->getDefaultLWP()->getRegisters();
      
      // We overwrote a return instruction to put the trap in.
      // We need to patch that up with a return to the caller
      
      changeType = SHAREDOBJECT_ADDED;
      
      *shObjects = new pdvector<shared_object *>;
      // get list of current shared objects defined for the process
      pdvector<shared_object *> *curr_list = proc->sharedObjects();
      
      // get the list from the process via /proc
      pdvector<shared_object *> *new_shared_objs = getSharedObjects(proc);
      
      for (unsigned int i=0; i < new_shared_objs->size(); i++) {
          string new_name = ((*new_shared_objs)[i])->getName();
          
          unsigned int j;
          for (j=0; j < curr_list->size(); j++) {
              string curr_name = ((*new_shared_objs)[j])->getName();
              if (curr_name == new_name) {
                  break;
              }
          }
          if (j == curr_list->size()) {
              (*shObjects)->push_back(((*new_shared_objs)[i]));
              (*new_shared_objs)[i] = NULL;
          }
      }
      
      // delete new_shared_objs;
      // Get return address
      Address retAddr = (regs->theIntRegs).regs[REG_RA];
      proc->getDefaultLWP()->changePC(retAddr, NULL);
      fprintf(stderr, "Changing PC to 0x%x\n", retAddr);
      
      return true;
  } else if (pc == dlcloseRetAddr) {
      fprintf(stderr, ">>> dlclose event\n");
      changeType = SHAREDOBJECT_REMOVED;
      
      // this is not yet supported
      return false;
  } else {
      changeType = SHAREDOBJECT_NOCHANGE;
      
      return false;
  }
}

/*
 * put code at the end of dlopen and dlclose to catch events
 */
void dynamic_linking::setMappingHooks(process *proc)
{
    instruction trapInsn;
    instruction tempSpace[1024];
    dlopenRetAddr = 0;
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
            dlopenRetAddr = openAddr + i * INSN_SIZE;
            break;
        }
    }
    assert(dlopenRetAddr);

    generateBreakPoint((instruction &) trapInsn);

    proc->writeDataSpace((caddr_t)dlopenRetAddr, sizeof(instruction),
	 (caddr_t) &trapInsn);
}

bool process::trapDueToDyninstLib()
{
  Address pc;
  prstatus_t stat;

  if (dyninstlib_brk_addr == 0) return false;

  if (ioctl(getDefaultLWP()->get_fd(), PIOCSTATUS, &stat) < 0) {
      perror("ioctl");
  }

  //pc = Frame(this).getPC();
  pc = getDefaultLWP()->getActiveFrame().getPC();

  // printf("testing for trap at entry to DyninstLib, current pc = 0x%lx\n",pc);
  // printf("    breakpoint addr = 0x%lx\n", dyninstlib_brk_addr);

  bool ret = (pc == dyninstlib_brk_addr);

  // XXXX - Hack, Tru64 is giving back an invalid pc here, we check for a pc == 0 and
  //   conclude if we are waiting for a trap for dlopen, then this must be it.
  //   Need to figure out why this happens. - jkh 1/30/02
  if (!ret && (stat.pr_reg.regs[31] == 0)) ret = true;

  return ret;
}

bool process::loadDYNINSTlibCleanup()
{
    dyninstlib_brk_addr = 0x0;
    
  // restore code and registers
  bool err;
  Address code = findInternalAddress("_start", false, err);
  assert(code);
  writeDataSpace((void *)code, sizeof(savedCodeBuffer), savedCodeBuffer);

  getDefaultLWP()->restoreRegisters(savedRegs);

  delete [] (char*)savedRegs;
  savedRegs = NULL;
  dyninstlib_brk_addr = 0;

  return true;
  /* No idea what everything beyond here is doing... */
#if 0
  const char *DyninstEnvVar="DYNINSTAPI_RT_LIB";
  const char *DyninstLibName="libdyninstAPI_RT";

  (void) findInternalAddress("DYNINSTbreakPoint", false, err);
  if (!err) {
      // found the library already
      string msg = string("Do not statically link ") + string(DyninstLibName) 
                 + string(".\n   Use ")              + string(DyninstEnvVar) 
                 + string(" environment variable to specify library.");
      showErrorCallback(101, msg);
      return;
  }

  if (dyninstRT_name.length()) {
    // use the library name specified on the start-up command-line
    sprintf(errorLine,"Pre-specified library %s\n", dyninstRT_name.c_str());
    logLine(errorLine);
  } else {
    // check the environment variable
    if (getenv(DyninstEnvVar) != NULL) {
      dyninstRT_name = getenv(DyninstEnvVar);
      sprintf(errorLine,"Environment variable %s=%s\n", DyninstEnvVar,
              dyninstRT_name.c_str());
      logLine(errorLine);
    } else {
      string msg = string("Environment variable " + string(DyninstEnvVar)
                   + " has not been defined for process ") + string(pid);
      showErrorCallback(101, msg);
      return;
    }
  }
  if (access(dyninstRT_name.c_str(), R_OK)) {
    string msg = string("Runtime library ") + dyninstRT_name
        + string(" does not exist or cannot be accessed!");
    showErrorCallback(101, msg);
    return;
  }

  pdvector<shared_object *> *new_shared_objs = dyn->getSharedObjects(this);
  if (!new_shared_objs) return;

  // get the address of the new shared objects
  for (unsigned int i=0; i < new_shared_objs->size(); i++) {
      if (((*new_shared_objs)[i])->getName() == dyninstRT_name) {
	  if (addASharedObject(*((*new_shared_objs)[i]))) {
	      shared_objects->push_back(((*new_shared_objs)[i]));
	  }
      }
  }
  return true;
#endif  
}

#ifdef DEBUG
static void dumpMap(int proc_fd) 
{
  int ret;
  int numMaps;
  prmap_t *maps;

  ret = ioctl(proc_fd, PIOCNMAP, &numMaps);
  assert(ret == 0);

  printf("%d segments mapped\n", numMaps);

  maps = (prmap_t *) calloc(sizeof(prmap_t), numMaps+1);

  ret = ioctl(proc_fd, PIOCMAP, maps);
  assert(ret == 0);

  for (int i = 0; i < numMaps; i++) {
        printf("map %d:\n", i);
        printf("    start: %lx\n", maps[i].pr_vaddr);
        printf("    size: %lx\n", maps[i].pr_size);
        printf("    protection: ");
        if (maps[i].pr_mflags & MA_READ) printf("READ ");
        if (maps[i].pr_mflags & MA_WRITE) printf("WRITE ");
        if (maps[i].pr_mflags & MA_EXEC) printf("EXEC ");
        printf("\n");
  }
}
#endif

bool osfTestProc(int proc_fd, const void *mainLoc)
// This function is used to test if the child program is
// ready to be read or written to.  mainLoc should be the
// address of main() in the mutatee.
//
// See process::insertTrapAtEntryPointOfMain() below for a
// detailed explination of why this function is needed.
//
// Ray Chen 6/18/2002
{
    return (lseek(proc_fd, reinterpret_cast<off_t>(mainLoc), SEEK_SET) == (off_t)mainLoc);
}

void osfWaitProc(int fd)
{
    int ret;
    struct pollfd pollFD;
    struct prstatus status;

    // now wait for the signal
    memset(&pollFD, '\0', sizeof(pollFD));
    pollFD.fd = fd;
    pollFD.events = POLLPRI | POLLNORM;
    pollFD.revents = 0;
    ret = poll(&pollFD, 1, -1);
    if (ret < 0) {
	 string msg("poll failed");
	 showErrorCallback(101, msg);
	 return;
    }

    if (ioctl(fd, PIOCSTATUS, &status) < 0) {
	 string msg("PIOCSTATUS failed");
	 showErrorCallback(101, msg);
	 return;
    }
#ifdef DEBUG
    printf("status = %d\n", status.pr_why);
    if (status.pr_flags & PR_STOPPED) {
        if (status.pr_why == PR_SIGNALLED) {
            printf("stopped for signal %d\n", status.pr_what);
        } else if (status.pr_why == PR_FAULTED) {
            printf("stopped for fault %d\n", status.pr_what);
        } else if (status.pr_why == PR_SYSEXIT) {
            printf("stopped for exist system call %d\n", status.pr_what);
        } else {
            printf("stopped for pr+why = %d\n", status.pr_why);
        }
    } else {
        printf("process is *not* stopped\n");
    }
#endif
}

/*
 * Place a trap at the entry point to main.  We need to prod the program
 *    along a bit since at the entry to this function, the program is in
 *    the dynamic loader and has not yet loaded the segment that contains
 *    main.  All we need to do is wait for a SIGTRAP that the loader gives
 *    us just after it completes loading.
 */
void process::insertTrapAtEntryPointOfMain()
{
  // XXX - Should check if it's statically linked and skip the prod. - jkh
  // continueProc_();
  // waitProc(proc_fd, SIGTRAP);

  // continueProc_();
  // waitProc(proc_fd, SIGTRAP);

  // save trap address: start of main()
  // TODO: use start of "_main" if exists?
  bool err;
  int countdown = 10;

  main_brk_addr = findInternalAddress("main", false, err);
  if (!main_brk_addr) {
      // failed to locate main
      showErrorCallback(108,"Failed to locate main().\n");
      extern void cleanUpAndExit(int);
      cleanUpAndExit(-1);
      return;
  }
  assert(main_brk_addr);

  // dumpMap(proc_fd);

  while (!osfTestProc(getDefaultLWP()->get_fd(), (void *)main_brk_addr)) {
      // POSSIBLE BUG:  We expect the first SIGTRAP to occur after a
      // successful exec call, but we seem to get an early signal.
      // At the time of the first SIGTRAP, attempts to read or write the
      // child data space fail.
      //
      // If the child is instructed to continue, it will eventually stop
      // in a useable state (before the first instruction of main).  However,
      // a SIGTRAP will *NOT* be generated on the second stop.  PROCFS also
      // stops in a strange state (prstatus_t.pr_info.si_code == 0).
      //
      // Looks like this code was in place before.  I don't know why it was
      // removed. (I renamed waitProc() to osfWaitProc() to avoid confusion
      // with process' waitProcs() class method)
      //
      // Ray Chen 03/22/02
      if (--countdown < 0) {
	  // looped too many times.
	  showErrorCallback(108, "Could not access mutatee (even after 10 tries).\n");
	  extern void cleanUpAndExit(int);
	  cleanUpAndExit(-1);
	  return;
      }

      continueProc_();
      osfWaitProc(getDefaultLWP()->get_fd());
  }
  readDataSpace((void *)main_brk_addr, INSN_SIZE, savedCodeBuffer, true);

  // insert trap instruction
  instruction trapInsn;
  generateBreakPoint(trapInsn);

  writeDataSpace((void *)main_brk_addr, INSN_SIZE, &trapInsn);

}

bool process::trapAtEntryPointOfMain(Address)
{
  Address pc;

  if (main_brk_addr == 0) return false;

  //pc = Frame(this).getPC();
  pc = getDefaultLWP()->getActiveFrame().getPC();

  // printf("testing for trap at enttry to main, current pc = %lx\n", pc);

  bool ret = (pc == main_brk_addr);
  // if (ret) fprintf(stderr, ">>> process::trapAtEntryPointOfMain()\n");
  return ret;
}

void process::handleTrapAtEntryPointOfMain()
{
    // restore original instruction to entry point of main()
    writeDataSpace((void *)main_brk_addr, INSN_SIZE, savedCodeBuffer);

    // set PC to be value at the address.
   gregset_t theIntRegs;
   if (ioctl(getDefaultLWP()->get_fd(), PIOCGREG, &theIntRegs) == -1) {
      perror("dyn_lwp::getRegisters PIOCGREG");
      if (errno == EBUSY) {
         cerr << "It appears that the process was not stopped in the eyes of /pr
oc" << endl;
         assert(false);
      }
      return;
   }
   theIntRegs.regs[PC_REGNUM] -= 4;
   getDefaultLWP()->changePC(theIntRegs.regs[PC_REGNUM], NULL);

  prstatus info;
  ioctl(getDefaultLWP()->get_fd(), PIOCSTATUS,  &info);
  while (!prismember(&info.pr_flags, PR_STOPPED))
  {
     sleep(1);
     ioctl(getDefaultLWP()->get_fd(), PIOCSTATUS,  &info);
  }
   if (ioctl(getDefaultLWP()->get_fd(), PIOCSREG, &theIntRegs) == -1) {
      perror("dyn_lwp::getRegisters PIOCGREG");
      if (errno == EBUSY) {
         cerr << "It appears that the process was not stopped in the eyes of /pr
oc" << endl;
         assert(false);
      }
      return;
    }
}


bool process::loadDYNINSTlib()
{
    //fprintf(stderr, ">>> process::loadDYNINSTlib()\n");

  // use "_start" as scratch buffer to invoke dlopen() on DYNINST
  bool err;
  extern bool skipSaveCalls;
  Address baseAddr = findInternalAddress("_start", false, err);
  assert(baseAddr);
  char buf_[BYTES_TO_SAVE];
  char *buf = buf_;
  instruction illegalInsn;
  Address bufSize = 0;

  memset(buf, '\0', BYTES_TO_SAVE);

  // step 0: illegal instruction (code)
  extern void generateIllegalInsn(instruction &);
  generateIllegalInsn((instruction &) illegalInsn);
  bcopy((char *) &illegalInsn, buf, INSN_SIZE);
  bufSize += INSN_SIZE;

  // step 1: DYNINST library string (data)
  Address libAddr = baseAddr + bufSize;
#ifdef BPATCH_LIBRARY
  char *libVar = "DYNINSTAPI_RT_LIB";
#else
  char *libVar = "PARADYN_LIB";
#endif
  char *libName = getenv(libVar);
  if (!libName) {
    string msg = string("Environment variable DYNINSTAPI_RT_LIB is not defined,"
        " should be set to the pathname of the dyninstAPI_RT runtime library.");
    showErrorCallback(101, msg);
    return false;
  }

  int libSize = strlen(libName) + 1;
  strcpy((char *) &buf[bufSize], libName);

  int npad = INSN_SIZE - (libSize % INSN_SIZE);
  bufSize += (libSize + npad);

  // step 2: inferior dlopen() call (code)
  Address codeAddr = baseAddr + bufSize;

  extern registerSpace *createRegisterSpace();
  registerSpace *regs = createRegisterSpace();

  pdvector<AstNode*> args(2);
  AstNode *call;
  string callee = "dlopen";
  // inferior dlopen(): build AstNodes
  args[0] = new AstNode(AstNode::Constant, (void *)libAddr);
  args[1] = new AstNode(AstNode::Constant, (void *)RTLD_NOW);
  call = new AstNode(callee, args);
  removeAst(args[0]);
  removeAst(args[1]);

  // inferior dlopen(): generate code
  regs->resetSpace();

  skipSaveCalls = true;		// don't save register, we've done it!
  call->generateCode(this, regs, buf, bufSize, true, true);
  skipSaveCalls = false;

  removeAst(call);

  // save registers and "_start" code
  readDataSpace((void *)baseAddr, BYTES_TO_SAVE, (void *) savedCodeBuffer,true);
  savedRegs = getDefaultLWP()->getRegisters();
  assert(savedRegs);

  // step 3: trap instruction (code)
  Address trapAddr = baseAddr + bufSize;
  instruction bkpt;
  generateBreakPoint((instruction &) bkpt);
  bcopy((char *) &bkpt, &buf[bufSize], INSN_SIZE);
  bufSize += INSN_SIZE;

  // step 4: write inferior dlopen code and set PC
  assert(bufSize <= BYTES_TO_SAVE);
  // fprintf(stderr, "writing %ld bytes to <0x%08lx:_start>, $pc = 0x%lx\n",
      // bufSize, baseAddr, codeAddr);
  // fprintf(stderr, ">>> loadDYNINSTlib <0x%lx(_start): %ld insns>\n",
      // baseAddr, bufSize/INSN_SIZE);
  writeDataSpace((void *)baseAddr, bufSize, (void *)buf);
  bool ret = getDefaultLWP()->changePC(codeAddr, savedRegs);
  assert(ret);

  dyninstlib_brk_addr = trapAddr;
  setBootstrapState(loadingRT);
  
  return true;
}
