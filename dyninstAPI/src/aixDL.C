/*
 * Copyright (c) 1996 Barton P. Miller
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

// $Id: aixDL.C,v 1.25 2002/11/14 20:26:31 bernat Exp $

#include "dyninstAPI/src/sharedobject.h"
#include "dyninstAPI/src/aixDL.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/arch-power.h"
#include "dyninstAPI/src/inst-power.h"
#include "common/h/debugOstream.h"
#include <sys/ptrace.h>
#include <sys/ldr.h>
#include <dlfcn.h> // dlopen constants

/* Getprocs() should be defined in procinfo.h, but it's not */
extern "C" {
extern int getprocs(struct procsinfo *ProcessBuffer,
		    int ProcessSize,
		    struct fdsinfo *FileBuffer,
		    int FileSize,
		    pid_t *IndexPointer,
		    int Count);
extern int getthrds(pid_t, struct thrdsinfo *, int, tid_t, int);
}
#define DLOPEN_MODE (RTLD_NOW | RTLD_GLOBAL)
extern void generateBreakPoint(instruction &);


/* Parse a binary to extract all shared objects it
   contains, and create shared object objects for each
   of them */
vector< shared_object *> *dynamic_linking::getSharedObjects(process *p)
{
  // First things first, get a list of all loader info structures.
  int pid;
  int ret;
  struct ld_info *ptr;
  struct stat ld_stat;
  static bool did_ptrace_multi = false;
  // We hope that we don't get more than 1024 libraries loaded.
  ptr = (struct ld_info *) malloc (1024*sizeof(*ptr));
  pid = p->getPid();
  /* It seems that AIX has some timing problems and
     when the user stack grows, the kernel doesn't update the stack info in time
     and ptrace calls step on user stack. This is the reason why call sleep 
     here, giving the kernel some time to update its internals. */
  usleep (36000);

  ret = 0;
  ret = ptrace(PT_LDINFO, pid, 
	       (int *) ptr, 1024 * sizeof(*ptr), (int *)ptr);
  
  if (ret != 0) {
    perror("PT_LDINFO");
    fprintf(stderr, "For process %d\n", pid);
    statusLine("Unable to get loader info about process, application aborted");
    showErrorCallback(43, "Unable to get loader info about process, application aborted");
    return 0;
  }

  // turn on 'multiprocess debugging', which allows ptracing of both the
  // parent and child after a fork.  In particular, both parent & child will
  // TRAP after a fork.  Also, a process will TRAP after an exec (after the
  // new image has loaded but before it has started to execute at all).
  // Note that turning on multiprocess debugging enables two new values to be
  // returned by wait(): W_SEWTED and W_SFWTED, which indicate stops during
  // execution of exec and fork, respectively.
  // Should do this in loadSharedObjects
  // Note: this can also get called when we incrementally find a shared object.
  // So? :)
  if (!did_ptrace_multi) {
    ptrace(PT_MULTI, pid, 0, 1, 0);
    did_ptrace_multi = true;
  }

  if (!ptr->ldinfo_next)
    {
      // Non-shared object, return 0
      return 0;
    }

  // Skip the first element, which appears to be the executable file.
  ptr = (struct ld_info *)(ptr->ldinfo_next + (char *)ptr);

  // We want to fill in this vector.
  vector<shared_object *> *result = new(vector<shared_object *>);

  // So we have this list of ldinfo structures. This will include the executable and
  // all shared objects it has loaded. Parse it.
  do
    {
      // Gripe if the shared object we have loaded has been deleted
      // If this is so, the file descriptor has been set to -1
      // This can be a problem, since we expect to find symbol table
      // information from the file.
      if (fstat (ptr->ldinfo_fd, &ld_stat) < 0) {
	// We were given a file handle that is invalid. We get this
	// behavior if there's a library that's been loaded into memory,
	// but it's disk equivalent is no longer there. Examples:
	// loaded a library and deleted it, loaded a library and then
	// copied in a fresh copy of that library, i.e. the nightly build.
	// Solution: take the pathname we have and construct a new
	// handle.
	ptr->ldinfo_fd = open(ptr->ldinfo_filename, O_RDONLY);
	if (ptr->ldinfo_fd == -1) {
	  // Sucks to be us
	  //fprintf(stderr, "aixDL.C:getSharedObjects: library %s has disappeared!\n", ptr->ldinfo_filename);
	  perror("aixDL.C:getSharedObjects");
	  // Set the memory offsets to -1 so we don't try to parse it.
	  ptr->ldinfo_textorg = (void *)-1;
	  ptr->ldinfo_dataorg = (void *)-1;
	}
      }

      string obj_name = string(ptr->ldinfo_filename);
      string member = string(ptr->ldinfo_filename + 
			     (strlen(ptr->ldinfo_filename) + 1));
      Address text_org =(Address) ptr->ldinfo_textorg;
      Address data_org =(Address) ptr->ldinfo_dataorg;
#if defined(DEBUG)
      fprintf(stderr, "%s:%s (%x/%x)\n",
	      obj_name.c_str(), member.c_str(),
	      text_org, data_org);
#endif
      // I believe that we need to pass this as a pointer so that
      // subclassing will work
      // "false" == shared library, not exec file
      fileDescriptor_AIX *fda = new fileDescriptor_AIX(obj_name, member,
						       text_org, data_org,
						       pid, false);
      shared_object *newobj = new shared_object(fda,
						false,true,true,0);
      (*result).push_back(newobj);      

      // Close the file descriptor we're given
      close(ptr->ldinfo_fd);
    }
  while ((ptr->ldinfo_next) &&
	 // Pointer arithmetic -- the ldinfo_next data is all relative.
	 (ptr = (struct ld_info *)(ptr->ldinfo_next + (char *)ptr))); 

  p->setDynamicLinking();
  dynlinked = true;

  free(ptr);
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
bool dynamic_linking::handleIfDueToSharedObjectMapping(process *p,
						       vector<shared_object *> **changed_objects,
						       u_int &change_type, 
						       bool &error_occurred) {
  // Well, this is easy, ain't it?
  // List of current shared objects
  vector <shared_object *> *curr_list = p->sharedObjects();
  // List of new shared objects (since we cache parsed objects, we
  // can go overboard safely)
  vector <shared_object *> *new_list = getSharedObjects(p);

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


  *changed_objects = new(vector<shared_object *>);

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
      // So if found_object is true, we don't care. Set it to false and loop. Otherwise,
      // add this to the new list of objects
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
      // So if found_object is true, we don't care. Set it to false and loop. Otherwise,
      // add this to the new list of objects
      if (!found_object) {
	(**changed_objects).push_back(((*new_list)[j]));
      }
      else found_object = false; // reset
    }
  }
  
  // Check to see that there is something in the new list
  if ((*changed_objects)->size() == 0) {
    change_type = 0; // no change after all
    delete changed_objects; // Save memory
    changed_objects = 0;
    return false;
  }
  return true;
}


/*************************************************************************/
/***  Code to handle dlopen()ing the runtime library                   ***/
/***                                                                   ***/
/***  get_dlopen_addr() -- return the address of the dlopen function   ***/
/***  Address dyninstlib_brk_addr -- address of the breakpoint at the  ***/
/***                                 end of the RT init function       ***/
/***  Address main_brk_addr -- address when we switch to dlopen()ing   ***/
/***                           the RT lib                              ***/
/***  dlopenDYNINSTlib() -- Write the (string) name of the RT lib,     ***/
/***                        set up and execute a call to dlopen()      ***/
/***  trapDueToDyninstLib() -- returns true if trap addr is at         ***/
/***                          dyninstlib_brk_addr                      ***/
/***  trapAtEntryPointOfMain() -- see above                            ***/
/***  handleIfDueToDyninstLib -- cleanup function                      ***/
/***  handleTrapAtEntryPointOfMain -- cleanup function                 ***/
/***  insertTrapAtEntryPointOfMain -- insert a breakpoint at the start ***/
/***                                  of main                          ***/
/*************************************************************************/


/* Auxiliary function */

bool checkAllThreadsForBreakpoint(process *proc, Address break_addr)
{
  vector<Frame> activeFrames;
  if (!proc->getAllActiveFrames(activeFrames)) return false;
  for (unsigned frame_iter = 0; frame_iter < activeFrames.size(); frame_iter++)
    if (activeFrames[frame_iter].getPC() == break_addr) {
      return true;
    }
  return false;
}

		  //ccw 30 apr 2002 : SPLIT4  
#if !defined(BPATCH_LIBRARY)
	
bool process::trapDueToParadynLib()
{
  // Since this call requires a PTRACE, optimize it slightly
  if (paradynlib_brk_addr == 0x0) return false;
  // We have multiple threads, right? Well, instead of checking all
  // of 'em all the time, cache the matching kernel thread ID and
  // use it as long as it works :)
  bool result = checkAllThreadsForBreakpoint(this, paradynlib_brk_addr);

  return result;
}
#endif

bool process::trapDueToDyninstLib()
{
  // Since this call requires a PTRACE, optimize it slightly
  if (dyninstlib_brk_addr == 0x0) return false;
  // We have multiple threads, right? Well, instead of checking all
  // of 'em all the time, cache the matching kernel thread ID and
  // use it as long as it works :)
  bool result = checkAllThreadsForBreakpoint(this, dyninstlib_brk_addr);
  if(result){ 
    dyninstlib_brk_addr = 0; //ccw 30 apr 2002 : SPLIT3
    //dyninstlib_brk_addr and paradynlib_brk_addr may be the same
    //if they are we dont want to get them mixed up. once we
    //see this trap is due to dyninst, reset the addr so
    //we can now catch the paradyn trap
  }
  return result;
  
}

bool process::trapAtEntryPointOfMain()
{
  // Since this call requires a PTRACE, optimize it slightly
  // This won't trigger (ever) if we are attaching, btw.
  if (main_brk_addr == 0x0) return false;
  return checkAllThreadsForBreakpoint(this, main_brk_addr);
}

/*
 * Cleanup after dlopen()ing the runtime library. Since we don't overwrite
 * any existing functions, just restore saved registers. Cool, eh?
 */

void process::handleIfDueToDyninstLib()
{
  getDefaultLWP()->restoreRegisters(savedRegs);
  delete savedRegs;
  savedRegs = NULL;
  // We was never here.... 
  
  // But before we go, reset the dyninstlib_brk_addr so we don't
  // accidentally trigger it, eh?
  dyninstlib_brk_addr = 0x0;
}

/*
 * Restore "the original instruction" written into main so that
 * we can proceed after the trap. Saved in "savedCodeBuffer",
 * which is a chunk of space we use for dlopening the RT library.
 */

void process::handleTrapAtEntryPointOfMain()
{
  function_base *f_main = findOneFunction("main");
  assert(f_main);
  Address addr = f_main->addr();
  // Put back the original insn
  writeDataSpace((void *)addr, sizeof(instruction), 
		 (char *)savedCodeBuffer);

  // And zero out the main_brk_addr so we don't accidentally
  // trigger on it.
  main_brk_addr = 0x0;
}

/*
 * Stick a trap at the entry point of main. At this point,
 * libraries are mapped into the proc's address space, and
 * we can dlopen the RT library.
 */

void process::insertTrapAtEntryPointOfMain()
{
  function_base *f_main = findOneFunction("main");
  if (!f_main) {
    // we can't instrument main - naim
    showErrorCallback(108,"main() uninstrumentable");
    extern void cleanUpAndExit(int);
    cleanUpAndExit(-1); 
    return;
  }
  assert(f_main);
  Address addr = f_main->addr();
  
  // save original instruction first
  readDataSpace((void *)addr, sizeof(instruction), savedCodeBuffer, true);

  // and now, insert trap
  instruction insnTrap;
  generateBreakPoint(insnTrap);
  writeDataSpace((void *)addr, sizeof(instruction), (char *)&insnTrap);  
  main_brk_addr = addr;
}

/*
 * getRTLibraryName()
 *
 * Return the name of the runtime library, grabbed from the environment
 * or the command line, as appropriate.
 *
 * Updates process::dyninstName
 */

bool getRTLibraryName(string &dyninstName, int pid)
{
  // get library name... 
  // Get the name of the appropriate runtime library
  const char DyninstEnvVar[]="DYNINSTAPI_RT_LIB";//ccw 1 jun 2002 SPLIT

  if (dyninstName.length()) {
    // use the library name specified on the start-up command-line
  } else {
    // check the environment variable
    if (getenv(DyninstEnvVar) != NULL) {
      dyninstName = getenv(DyninstEnvVar);
    } else {
      string msg = string("Environment variable " + string(DyninstEnvVar)
                   + " has not been defined for process ") + string(pid);
      showErrorCallback(101, msg);
      return false;
    }
  }

  // Check to see if the library given exists.
  if (access(dyninstName.c_str(), R_OK)) {
    string msg = string("Runtime library ") + dyninstName
        + string(" does not exist or cannot be accessed!");
    showErrorCallback(101, msg);
    return false;
  }
  return true;
}


/*
 * dlopenDYNINSTlib()
 *
 * The evil black magic function. What we want: for the runtime
 * library to show up in the process' address space. Magically.
 * No such luck. What we do: patch in a call to dlopen(RTLIB)
 * at the entry of main, then restore the original instruction
 * and continue.
 */

extern void pushStack(char *i, Address &base);
extern void popStack(char *i, Address &base);

bool process::dlopenDYNINSTlib()
{
  // This is actually much easier than on other platforms, since
  // by now we have the DYNINSTstaticHeap symbol to work with.
  // Basically, we can ptrace() anywhere in the text heap we want to,
  // so go somewhere untouched. Unfortunately, we haven't initialized
  // the tramp space yet (no point except on AIX) so we can't simply
  // call inferiorMalloc(). 

  // However, if we can get code_len_ + code_off_ from the object file,
  // then we can use the area above that point freely.

  // Steps: Get the library name (command line or ENV)
  //        Get the address for dlopen()
  //        Write in a call to dlopen()
  //        Write in a trap after the call
  //        Write the library name somewhere where dlopen can find it.
  // Actually, why not write the library name first?

  const Object binaryFile = symbols->getObject();
  Address codeBase = binaryFile.code_off() + binaryFile.code_len();
  // Round it up to the nearest instruction. 
  codeBase += sizeof(instruction) - (codeBase % sizeof(instruction));


  int count = 0; // how much we've written
  unsigned char scratchCodeBuffer[BYTES_TO_SAVE]; // space
  Address dyninstlib_addr;
  Address dlopencall_addr;
  Address dlopentrap_addr;

  // Do we want to save whatever is there? Can't see a reason why...

  // Write out the name of the library to the codeBase area
  if (!getRTLibraryName(dyninstName, pid))
    return false;
  
  // write library name...
  dyninstlib_addr = (Address) (codeBase + count);
  writeDataSpace((void *)(codeBase + count), dyninstName.length()+1,
		 (caddr_t)const_cast<char*>(dyninstName.c_str()));
  count += dyninstName.length()+sizeof(instruction); // a little padding

  // Actually, we need to bump count up to a multiple of insnsize
  count += sizeof(instruction) - (count % sizeof(instruction));

  // Need a register space
  // make sure this syncs with inst-power.C
  Register liveRegList[] = {10, 9, 8, 7, 6, 5, 4, 3};
  Register deadRegList[] = {11, 12};
  unsigned liveRegListSize = sizeof(liveRegList)/sizeof(Register);
  unsigned deadRegListSize = sizeof(deadRegList)/sizeof(Register);

  registerSpace *dlopenRegSpace = new registerSpace(deadRegListSize, deadRegList, 
						    liveRegListSize, liveRegList);
  dlopenRegSpace->resetSpace();

  Address dyninst_count = 0; // size of generated code
  vector<AstNode*> dlopenAstArgs(2);
  AstNode *dlopenAst;

  dlopenAstArgs[0] = new AstNode(AstNode::Constant, (void *)(dyninstlib_addr));
  dlopenAstArgs[1] = new AstNode(AstNode::Constant, (void*)DLOPEN_MODE);
  dlopenAst = new AstNode("dlopen", dlopenAstArgs);
  removeAst(dlopenAstArgs[0]);
  removeAst(dlopenAstArgs[1]);
  // We need to push down the stack before we call this
  pushStack((char *)scratchCodeBuffer, dyninst_count);
  dlopenAst->generateCode(this, dlopenRegSpace, (char *)scratchCodeBuffer,
			  dyninst_count, true, true);
  popStack((char *)scratchCodeBuffer, dyninst_count);
  dlopencall_addr = codeBase + count;
  writeDataSpace((void *)dlopencall_addr, dyninst_count, 
		 (char *)scratchCodeBuffer);
  removeAst(dlopenAst);
  count += dyninst_count;

  instruction insnTrap;
  generateBreakPoint(insnTrap);
  dlopentrap_addr = codeBase + count;
  writeDataSpace((void *)dlopentrap_addr, sizeof(instruction),
		 (void *)(&insnTrap.raw));
  count += sizeof(instruction);

  dyninstlib_brk_addr = dlopentrap_addr;

  // save registers
  savedRegs = getDefaultLWP()->getRegisters();
  assert((savedRegs!=NULL) && (savedRegs!=(void *)-1));

  isLoadingDyninstLib = true;
  if (!getDefaultLWP()->changePC(dlopencall_addr, NULL)) {
    logLine("WARNING: changePC failed in dlopenDYNINSTlib\n");
    assert(0);
  }
  return true;
}



		  //ccw 30 apr 2002 : SPLIT4  
#if !defined(BPATCH_LIBRARY)

bool process::dlopenPARADYNlib()
{
  // This is actually much easier than on other platforms, since
  // by now we have the DYNINSTstaticHeap symbol to work with.
  // Basically, we can ptrace() anywhere in the text heap we want to,
  // so go somewhere untouched. Unfortunately, we haven't initialized
  // the tramp space yet (no point except on AIX) so we can't simply
  // call inferiorMalloc(). 

  // However, if we can get code_len_ + code_off_ from the object file,
  // then we can use the area above that point freely.

  // Steps: Get the library name (command line or ENV)
  //        Get the address for dlopen()
  //        Write in a call to dlopen()
  //        Write in a trap after the call
  //        Write the library name somewhere where dlopen can find it.
  // Actually, why not write the library name first?

  const Object binaryFile = symbols->getObject();
  Address codeBase = binaryFile.code_off() + binaryFile.code_len();
  // Round it up to the nearest instruction. 
  codeBase += sizeof(instruction) - (codeBase % sizeof(instruction));


  int count = 0; // how much we've written
  unsigned char scratchCodeBuffer[BYTES_TO_SAVE]; // space
  Address dyninstlib_addr;
  Address dlopencall_addr;
  Address dlopentrap_addr;

  // Do we want to save whatever is there? Can't see a reason why...
#ifdef MT_THREAD
  const char DyninstEnvVar[]="PARADYN_LIB_MT";
#else
  const char DyninstEnvVar[]="PARADYN_LIB";
#endif MT_THREAD

    // check the environment variable
    if (getenv(DyninstEnvVar) != NULL) {
      dyninstName = getenv(DyninstEnvVar);
    } else {
      string msg = string("Environment variable " + string(DyninstEnvVar)
                   + " has not been defined for process ") + string(pid);
      showErrorCallback(101, msg);
      return false;
  }

  // Check to see if the library given exists.
  if (access(dyninstName.c_str(), R_OK)) {
    string msg = string("Runtime library ") + dyninstName
        + string(" does not exist or cannot be accessed!");
    showErrorCallback(101, msg);
    return false;
  }
  
  // write library name...
  dyninstlib_addr = (Address) (codeBase + count);
  writeDataSpace((void *)(codeBase + count), dyninstName.length()+1,
		 (caddr_t)const_cast<char*>(dyninstName.c_str()));
  count += dyninstName.length()+sizeof(instruction); // a little padding

  // Actually, we need to bump count up to a multiple of insnsize
  count += sizeof(instruction) - (count % sizeof(instruction));

  // Need a register space
  // make sure this syncs with inst-power.C
  Register liveRegList[] = {10, 9, 8, 7, 6, 5, 4, 3};
  Register deadRegList[] = {11, 12};
  unsigned liveRegListSize = sizeof(liveRegList)/sizeof(Register);
  unsigned deadRegListSize = sizeof(deadRegList)/sizeof(Register);

  registerSpace *dlopenRegSpace = new registerSpace(deadRegListSize, deadRegList, 
						    liveRegListSize, liveRegList);
  dlopenRegSpace->resetSpace();

  Address dyninst_count = 0; // size of generated code
  vector<AstNode*> dlopenAstArgs(2);
  AstNode *dlopenAst;

  dlopenAstArgs[0] = new AstNode(AstNode::Constant, (void *)(dyninstlib_addr));
  dlopenAstArgs[1] = new AstNode(AstNode::Constant, (void*)DLOPEN_MODE);
  dlopenAst = new AstNode("dlopen", dlopenAstArgs);
  removeAst(dlopenAstArgs[0]);
  removeAst(dlopenAstArgs[1]);
  // We need to push down the stack before we call this
  pushStack((char *)scratchCodeBuffer, dyninst_count);
  dlopenAst->generateCode(this, dlopenRegSpace, (char *)scratchCodeBuffer,
			  dyninst_count, true, true);
  popStack((char *)scratchCodeBuffer, dyninst_count);
  dlopencall_addr = codeBase + count;
  writeDataSpace((void *)dlopencall_addr, dyninst_count, 
		 (char *)scratchCodeBuffer);
  removeAst(dlopenAst);
  count += dyninst_count;

  instruction insnTrap;
  generateBreakPoint(insnTrap);
  dlopentrap_addr = codeBase + count;
  writeDataSpace((void *)dlopentrap_addr, sizeof(instruction),
		 (void *)(&insnTrap.raw));
  count += sizeof(instruction);

  paradynlib_brk_addr = dlopentrap_addr; //ccw 30 apr 2002 : SPLIT4

  // save registers
  savedRegs = getDefaultLWP()->getRegisters();
  assert((savedRegs!=NULL) && (savedRegs!=(void *)-1));
  isLoadingParadynLib = true; //ccw 30 apr 2002 : SPLIT4
  if (!getDefaultLWP()->changePC(dlopencall_addr, NULL)) {
    logLine("WARNING: changePC failed in dlopenPARADYNlib\n");
    assert(0);
  }
  return true;
}

#endif
/*
 * Return the entry point of dlopen(). Used (I think) when we generate
 * a call to dlopen in an AST
 */

Address process::get_dlopen_addr() const {

  function_base *pdf = findOneFunction("dlopen");

  if (pdf)
    return pdf->addr();

  return 0;

}
