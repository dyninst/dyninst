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

// $Id: solaris.C,v 1.173 2005/02/25 07:04:47 jaw Exp $

#include "dyninstAPI/src/symtab.h"
#include "common/h/headers.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/stats.h"
#include "common/h/Types.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/termios.h>
#include <unistd.h>
#include "dyninstAPI/src/showerror.h"
#include "common/h/pathName.h" // concat_pathname_components()
#include "common/h/debugOstream.h"
#include "common/h/solarisKludges.h"

#if defined (sparc_sun_solaris2_4)
#include "dyninstAPI/src/inst-sparc.h"
#else
#include "dyninstAPI/src/inst-x86.h"
#endif

#include "instPoint.h"

#include <procfs.h>
#include <stropts.h>
#include <link.h>
#include <dlfcn.h>

#include "dyn_lwp.h"

#define DLOPEN_MODE (RTLD_NOW | RTLD_GLOBAL)

int (*P_native_demangle)(const char *, char *, size_t);

extern "C" {
extern long sysconf(int);
};

// The following were defined in process.C

extern unsigned enable_pd_attach_detach_debug;

#if ENABLE_DEBUG_CERR == 1
#define attach_cerr if (enable_pd_attach_detach_debug) cerr
#else
#define attach_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_inferior_rpc_debug;

#if ENABLE_DEBUG_CERR == 1
#define inferiorrpc_cerr if (enable_pd_inferior_rpc_debug) cerr
#else
#define inferiorrpc_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_shm_sampling_debug;

#if ENABLE_DEBUG_CERR == 1
#define shmsample_cerr if (enable_pd_shm_sampling_debug) cerr
#else
#define shmsample_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_fork_exec_debug;

#if ENABLE_DEBUG_CERR == 1
#define forkexec_cerr if (enable_pd_fork_exec_debug) cerr
#else
#define forkexec_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_signal_debug;

#if ENABLE_DEBUG_CERR == 1
#define signal_cerr if (enable_pd_signal_debug) cerr
#else
#define signal_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

/*
   Define the indices of some registers to be used with pr_reg.
   These values are different on sparc and x86 platforms.
   RETVAL_REG: the registers that holds the return value of calls ($o0 on sparc,
               %eax on x86).
   PC_REG: program counter
   FP_REG: frame pointer (%i7 on sparc, %ebp on x86) 
*/
#ifdef sparc_sun_solaris2_4
#define PARAM1_REG (R_O0)
#define RETVAL_REG (R_O0)
#define PC_REG (R_PC)
#define FP_REG (R_O6)
#endif
#ifdef i386_unknown_solaris2_5
#define RETVAL_REG (EAX)
#define SP_REG (UESP)
#define PC_REG (EIP)
#define FP_REG (EBP)
#endif


extern void generateBreakPoint(instruction &insn);


// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) {
  //This is the POSIX-compliant way of disconnecting from the terminal 
  setpgrp();
}

/*
 * The set operations (set_entry_syscalls and set_exit_syscalls) are defined
 * in sol_proc.C
 */


// Compatibility for /proc
bool process::get_entry_syscalls(sysset_t *entry)
{
    pstatus_t status;
    if (!get_status(&status)) return false;
    
    memcpy(entry, &(status.pr_sysentry), sizeof(sysset_t));    
    return true;
}

bool process::get_exit_syscalls(sysset_t *exit)
{
    pstatus_t status;
    if (!get_status(&status)) return false;

    memcpy(exit, &(status.pr_sysexit), sizeof(sysset_t));
    return true;
}    

char* process::dumpPatchedImage(pdstring imageFileName){ //ccw 28 oct 2001

	writeBackElf *newElf;
	addLibrary *addLibraryElf;
	void *data, *paddedData;
	unsigned int errFlag=0;
	char name[50];	
	pdvector<imageUpdate*> compactedUpdates;
	pdvector<imageUpdate*> compactedHighmemUpdates;
	Address guardFlagAddr= trampGuardAddr();
	char *mutatedSharedObjects=0;
	int mutatedSharedObjectsSize = 0, mutatedSharedObjectsIndex=0;
	char *directoryName = 0;
	shared_object *sh_obj;
	if(!collectSaveWorldData){
		BPatch_reportError(BPatchSerious,122,"dumpPatchedImage: BPatch_thread::enableDumpPatchedImage() not called.  No mutated binary saved\n");
		return NULL;
	}


	directoryName = saveWorldFindDirectory();

	if(!directoryName){
		return NULL;
	}
	strcat(directoryName, "/");

	unsigned int dl_debug_statePltEntry = 0x00016574;//a pretty good guess
	unsigned int dyninst_SharedLibrariesSize = 0, mutatedSharedObjectsNumb;

	dl_debug_statePltEntry = saveWorldSaveSharedLibs(mutatedSharedObjectsSize, 
		dyninst_SharedLibrariesSize,directoryName, mutatedSharedObjectsNumb);

	if(mutatedSharedObjectsSize){

		//UPDATED: 24 jul 2003 to include flag.
		//the flag denotes whether the shared lib is Dirty (1) or only DirtyCalled (0)
		// This is going to be a section that looks like this:
		// string
		// addr
		// flag
		// ...
		// string
		// addr
		// flag
		
		mutatedSharedObjectsSize += mutatedSharedObjectsNumb * sizeof(unsigned int);
		mutatedSharedObjects = new char[mutatedSharedObjectsSize ];
		for(unsigned i=0;shared_objects && i<shared_objects->size() ; i++) {
			sh_obj = (*shared_objects)[i];
			//i ignore the dyninst RT lib here and in process::saveWorldSaveSharedLibs
			if(sh_obj->isDirty() || sh_obj->isDirtyCalled()&& NULL==strstr(sh_obj->getName().c_str(),"libdyninstAPI_RT")){ //ccw 24 jul 2003
				memcpy(  & ( mutatedSharedObjects[mutatedSharedObjectsIndex]),
					sh_obj->getName().c_str(),
					strlen(sh_obj->getName().c_str())+1);
				mutatedSharedObjectsIndex += strlen(
					sh_obj->getName().c_str())+1;
				unsigned int baseAddr = sh_obj->getBaseAddress();
				memcpy( & (mutatedSharedObjects[mutatedSharedObjectsIndex]),
					&baseAddr, sizeof(unsigned int));
				mutatedSharedObjectsIndex += sizeof(unsigned int);	

				//set flag
				unsigned int tmpFlag = ((sh_obj->isDirty()
							&&  NULL==strstr(sh_obj->getName().c_str(),"libc")) ?1:0);	
				memcpy( &(mutatedSharedObjects[mutatedSharedObjectsIndex]), &tmpFlag, sizeof(unsigned int));
				mutatedSharedObjectsIndex += sizeof(unsigned int);	

			}
		}	
	}
	char *dyninst_SharedLibrariesData =saveWorldCreateSharedLibrariesSection(dyninst_SharedLibrariesSize);

	newElf = new writeBackElf(( char*) getImage()->file().c_str(),
		"/tmp/dyninstMutatee",errFlag);
	newElf->registerProcess(this);

	imageUpdates.sort(imageUpdateSort);// imageUpdate::mysort ); 

	newElf->compactLoadableSections(imageUpdates,compactedUpdates);

	highmemUpdates.sort(imageUpdateSort);

	newElf->compactSections(highmemUpdates, compactedHighmemUpdates);

	newElf->alignHighMem(compactedHighmemUpdates);

	int sectionsAdded = 0;
	unsigned int newSize, nextPage, paddedDiff;
	unsigned int pageSize = getpagesize();


	//This adds the LOADABLE HEAP TRAMP sections
	//if new platforms require this it will be moved
	//to process.C proper. Right now, only solaris uses it
	for(unsigned int i=0;i<compactedUpdates.size();i++) {
		data = new char[compactedUpdates[i]->size];
		readDataSpace((void*) compactedUpdates[i]->address, 
						  compactedUpdates[i]->size, data, true);

		//the TrampGuardFlag must be set to 1 to get the
		//tramps to run. it is not necessary set to 1 yet so
		//we set it to 1 before we save the file.
		if(guardFlagAddr){
			if(compactedUpdates[i]->address < guardFlagAddr &&
                        guardFlagAddr < compactedUpdates[i]->address+compactedUpdates[i]->size){
                                ((char*) data)[guardFlagAddr -
                                        compactedUpdates[i]->address ] = 0x1;
                        }
		}

		sprintf(name,"dyninstAPI_%08x",sectionsAdded);

		// what i want to do is this: 
		// I want MMAP rather than MEMCPY as much into place 
		// as i can. 
		// My assumptions:
		// there can be any number of heap trampoline sections
		// Only the first section could have valid (non heap tramp) data on the
		// beginning of its first page. Any subsequent sections will begin on
		// a different page than the previous page ends on [by the way 
		// compactLoadableSections works] and the heap has been moved
		// beyond the last heap tramp section so the mutatee cannot
		// put data there.
		// 
		// for the first section, i=0, make
		// two sections, where the first one is everything up to the first
		// page break, which will be memcpy'ed to memory
		// the second and subsequent sections will be aligned to be
		// mmaped ..... which means some sections will get bigger.
		
		if( i == 0 ){
			/* create memcpy section */
			nextPage = compactedUpdates[i]->address - (compactedUpdates[i]->address%pageSize);
			nextPage +=pageSize;
			newSize = nextPage - compactedUpdates[i]->address;	
			newElf->addSection(compactedUpdates[i]->address,data ,newSize,name);
			sectionsAdded ++;

			if(compactedUpdates[i]->size > newSize){
				/* only create this section if the size of the update
					spans beyond the first memcpy section*/ 
				/* create mmap section */
				sprintf(name,"dyninstAPI_%08x",sectionsAdded);
				newElf->addSection(nextPage, &(((char*) data)[newSize]) ,
					compactedUpdates[i]->size- newSize,name);
				sectionsAdded ++;
			}
		}else{
			/* create section padded out backwards to a page boundry */
			paddedDiff = (compactedUpdates[i]->address%pageSize);
			paddedData = new char[paddedDiff + compactedUpdates[i]->size];
			memset(paddedData, '\0', paddedDiff + compactedUpdates[i]->size); //necessary?
			memcpy(&( ((char*) paddedData)[paddedDiff]), data, compactedUpdates[i]->size);

			newElf->addSection(compactedUpdates[i]->address-paddedDiff, paddedData,
				paddedDiff + compactedUpdates[i]->size, name);
			delete [] (char*) paddedData;
				
		}
		delete [] (char*) data;
	}

	saveWorldCreateHighMemSections(compactedHighmemUpdates, highmemUpdates, (void*) newElf);

	if(mutatedSharedObjectsSize){
		newElf->addSection(0, mutatedSharedObjects, mutatedSharedObjectsSize,
								 "dyninstAPI_mutatedSO", false);
	}

	unsigned int k;
	
	for( k=0;k<imageUpdates.size();k++){
		delete imageUpdates[k];
	}

	for( k=0;k<highmemUpdates.size();k++){
		delete highmemUpdates[k];
	}
	for(  k=0;k<compactedUpdates.size();k++){
                delete compactedUpdates[k];
        }

        for(  k=0;k<compactedHighmemUpdates.size();k++){
                delete compactedHighmemUpdates[k];
        }

	//the following is for the dlopen problem 
	newElf->addSection(dl_debug_statePltEntry, dyninst_SharedLibrariesData, 
	dyninst_SharedLibrariesSize, "dyninstAPI_SharedLibraries", false);

	delete [] dyninst_SharedLibrariesData;

	//the following reloads any shared libraries loaded into the
	//mutatee using BPatch_thread::loadLibrary
	saveWorldAddSharedLibs((void*)newElf); // ccw 14 may 2002 

	saveWorldCreateDataSections((void*)newElf);
	
	newElf->createElf();
	char* fullName = new char[strlen(directoryName) + strlen ( (char*)imageFileName.c_str())+1];
	strcpy(fullName, directoryName);
	strcat(fullName, (char*)imageFileName.c_str());
	
	addLibraryElf = new addLibrary();
	elf_update(newElf->getElf(), ELF_C_WRITE);
	if(!addLibraryElf->driver(newElf->getElf(), fullName, "libdyninstAPI_RT.so.1")) {
		//ccw 27 jun 2003
		BPatch_reportError(BPatchSerious,122,"dumpPatchedImage: addLibraryElf() failed!  No mutated binary saved\n");
		delete [] directoryName;
                return NULL;

	}
	delete [] fullName;
	if(mutatedSharedObjects){
		delete [] mutatedSharedObjects;
	}
	return directoryName;
}

bool process::dumpImage(pdstring imageFileName) 
{
   int newFd;
   image *im;
   pdstring command;
   
   im = getImage();
   pdstring origFile = im->file();
   
   // first copy the entire image file
   command = "cp ";
   command += origFile;
   command += " ";
   command += imageFileName;
   system(command.c_str());
   
   // now open the copy
   newFd = open(imageFileName.c_str(), O_RDWR, 0);
   if (newFd < 0) {
      // log error
      return false;
   }

   Elf *elfp = elf_begin(newFd, ELF_C_READ, 0);
   Elf_Scn *scn = 0;
   Address baseAddr = 0;
   int length = 0;
   int offset = 0;
   
   Elf32_Ehdr*	ehdrp;
   Elf_Scn* shstrscnp  = 0;
   Elf_Data* shstrdatap = 0;
   Elf32_Shdr* shdrp;
   
   assert(ehdrp = elf32_getehdr(elfp));
   assert(((shstrscnp = elf_getscn(elfp, ehdrp->e_shstrndx)) != 0) &&
          ((shstrdatap = elf_getdata(shstrscnp, 0)) != 0));
   const char* shnames = (const char *) shstrdatap->d_buf;
   
   while ((scn = elf_nextscn(elfp, scn)) != 0) {
      const char* name;
      
      shdrp = elf32_getshdr(scn);
      name = (const char *) &shnames[shdrp->sh_name];
      if (!strcmp(name, ".text")) {
         offset = shdrp->sh_offset;
         length = shdrp->sh_size;
         baseAddr = shdrp->sh_addr;
         break;
      }
   }
   

   char *tempCode = new char[length];
   
   
   bool ret = readTextSpace((void *) baseAddr, length, tempCode);
   if (!ret) {
      // log error
      return false;
   }
   
   lseek(newFd, offset, SEEK_SET);
   write(newFd, tempCode, length);
   delete[] tempCode;
   close(newFd);

   return true;
}

/* Auxiliary function */
bool checkAllThreadsForBreakpoint(process *proc, Address break_addr)
{
   pdvector<Frame> activeFrames;
   if (!proc->getAllActiveFrames(activeFrames)) return false;
   for(unsigned frame_iter = 0; frame_iter < activeFrames.size(); frame_iter++)
   {
      if (activeFrames[frame_iter].getPC() == break_addr) {
         return true;
      }
   }
   
   return false;
}

bool process::trapAtEntryPointOfMain(Address)
{
    if (main_brk_addr == 0x0) return false;
    return checkAllThreadsForBreakpoint(this, main_brk_addr);
}

bool process::handleTrapAtEntryPointOfMain()
{
    assert(main_brk_addr);
    
  // restore original instruction 
#if defined(sparc_sun_solaris2_4)
    writeDataSpace((void *)main_brk_addr, 
                   sizeof(instruction), (char *)savedCodeBuffer);
#else // x86
    writeDataSpace((void *)main_brk_addr, 2, 
                   (char *)savedCodeBuffer);
#endif
    main_brk_addr = 0;
    return true;
}

bool process::insertTrapAtEntryPointOfMain()
{

    int_function *f_main = findOnlyOneFunction("main");
    if (!f_main) {
        // we can't instrument main - naim
        showErrorCallback(108,"main() uninstrumentable");
        return false;
    }
    assert(f_main);
    Address addr = f_main->get_address(); 
    // save original instruction first
#if defined(sparc_sun_solaris2_4)
    readDataSpace((void *)addr, sizeof(instruction), savedCodeBuffer, true);
#else // x86
    readDataSpace((void *)addr, 2, savedCodeBuffer, true);
#endif
    
    // and now, insert trap
    instruction insnTrap;
    generateBreakPoint(insnTrap);
    
#if defined(sparc_sun_solaris2_4)
    writeDataSpace((void *)addr, sizeof(instruction), (char *)&insnTrap);  
#else //x86. have to use SIGILL instead of SIGTRAP
    writeDataSpace((void *)addr, 2, insnTrap.ptr());  
#endif
    main_brk_addr = addr;
    
    char buffer[256];
    readDataSpace((void *)addr, sizeof(instruction), buffer, true);
    return true;
}

bool process::getDyninstRTLibName() {
   if (dyninstRT_name.length() == 0) {
      // Get env variable
      if (getenv("DYNINSTAPI_RT_LIB") != NULL) {
         dyninstRT_name = getenv("DYNINSTAPI_RT_LIB");
      }
      else {
         pdstring msg = pdstring("Environment variable ") +
                        pdstring("DYNINSTAPI_RT_LIB") +
                        pdstring(" has not been defined for process ") +
                        pdstring(pid);
         showErrorCallback(101, msg);
         return false;
      }
   }
   // Check to see if the library given exists.
   if (access(dyninstRT_name.c_str(), R_OK)) {
      pdstring msg = pdstring("Runtime library ") + dyninstRT_name +
                     pdstring(" does not exist or cannot be accessed!");
      showErrorCallback(101, msg);
      return false;
   }
   return true;
}

bool process::loadDYNINSTlib() {
  // we will write the following into a buffer and copy it into the
  // application process's address space
  // [....LIBRARY's NAME...|code for DLOPEN]

  // write to the application at codeOffset. This won't work if we
  // attach to a running process.
  //Address codeBase = this->getImage()->codeOffset();
  // ...let's try "_start" instead
  int_function *_startfn = this->findOnlyOneFunction("_start");
  if (NULL == _startfn) {
    pdvector<int_function *> funcs;
    if (!this->findAllFuncsByName("_start", funcs) || !funcs.size()) {
       fprintf(stderr, "%s[%d]:  could not find _start()\n", __FILE__, __LINE__);
       return false;
    }
    fprintf(stderr, "%s[%d]:  WARN:  found %d matches for _start()\n", 
            __FILE__, __LINE__, funcs.size()); 
    int select_fn = 0;
    for (unsigned int i = 0; i < funcs.size(); ++i) {
      const char *modname = funcs[i]->pdmod()->fileName().c_str();
      fprintf(stderr, "\t[%d]\tin module %s\n",i, modname);
      if (strstr(modname, "libc.") || strstr(modname, "libdl."))
         select_fn = i;        
    }
    fprintf(stderr, "%s[%d]: selecting %d\n", __FILE__, __LINE__, select_fn);
    _startfn = funcs[select_fn];
  }

  Address codeBase = _startfn->getEffectiveAddress(this);
  assert(codeBase);

  // Or should this be readText... it seems like they are identical
  // the remaining stuff is thanks to Marcelo's ideas - this is what 
  // he does in NT. The major change here is that we use AST's to 
  // generate code for dlopen.

  // savedCodeBuffer[BYTES_TO_SAVE] is declared in process.h
  readDataSpace((void *)codeBase, sizeof(savedCodeBuffer), savedCodeBuffer, true);

  unsigned char scratchCodeBuffer[BYTES_TO_SAVE];
  pdvector<AstNode*> dlopenAstArgs(2);

  Address count = 0;

//ccw 18 apr 2002 : SPLIT
  AstNode *dlopenAst;

  // deadList and deadListSize are also used in inst-sparc.C
  // registers 8 to 15: out registers 
  // registers 16 to 22: local registers
  Register deadList[10] = { 16, 17, 18, 19, 20, 21, 22, 0, 0, 0 };
  unsigned dead_reg_count = 7;
  if(! multithread_capable()) {
     deadList[7] = 23;
     dead_reg_count++;
  }

  registerSpace *dlopenRegSpace =
     new registerSpace(dead_reg_count, deadList, (unsigned)0,
                       NULL, multithread_capable());
  dlopenRegSpace->resetSpace();

  // we need to make 2 calls to dlopen: one to load libsocket.so.1 and another
  // one to load libdyninst.so.1 - naim

//ccw 18 apr 2002 : SPLIT
  dlopenAstArgs[0] = new AstNode(AstNode::Constant, (void*)0);
  // library name. We use a scratch value first. We will update this parameter
  // later, once we determine the offset to find the string - naim
  dlopenAstArgs[1] = new AstNode(AstNode::Constant, (void*)DLOPEN_MODE); // mode
  dlopenAst = new AstNode("dlopen",dlopenAstArgs);
  removeAst(dlopenAstArgs[0]);
  removeAst(dlopenAstArgs[1]);

  Address dyninst_count = 0;
  dlopenAst->generateCode(this, dlopenRegSpace, (char *)scratchCodeBuffer,
			  dyninst_count, true, true);
  writeDataSpace((void *)(codeBase+count), dyninst_count, (char *)scratchCodeBuffer);
// the following seems to be a redundant relic
//#if defined(sparc_sun_solaris2_4)
//  dyninst_count += sizeof(instruction);
//#endif
  count += dyninst_count;

  instruction insnTrap;
  generateBreakPoint(insnTrap);
#if defined(sparc_sun_solaris2_4)
  writeDataSpace((void *)(codeBase + count), sizeof(instruction), 
		 (char *)&insnTrap);
  dyninstlib_brk_addr = codeBase + count;
  count += sizeof(instruction);
#else //x86
  writeDataSpace((void *)(codeBase + count), 2, insnTrap.ptr());
  dyninstlib_brk_addr = codeBase + count;
  count += 2;
#endif

//ccw 18 apr 2002 : SPLIT
  const char DyninstEnvVar[]="DYNINSTAPI_RT_LIB";

  if (dyninstRT_name.length()) {
    // use the library name specified on the start-up command-line
  } else {
    // check the environment variable
    if (getenv(DyninstEnvVar) != NULL) {
      dyninstRT_name = getenv(DyninstEnvVar);
    } else {
      pdstring msg = pdstring("Environment variable " + pdstring(DyninstEnvVar)
                   + " has not been defined for process ") + pdstring(pid);
      showErrorCallback(101, msg);
      return false;
    }
  }
  if (access(dyninstRT_name.c_str(), R_OK)) {
    pdstring msg = pdstring("Runtime library ") + dyninstRT_name +
                   pdstring(" does not exist or cannot be accessed!");
    showErrorCallback(101, msg);
    return false;
  }

  Address dyninstlib_addr = codeBase + count;

  writeDataSpace((void *)(codeBase + count), dyninstRT_name.length()+1,
		 (caddr_t)const_cast<char*>(dyninstRT_name.c_str()));
  count += dyninstRT_name.length()+1;
  // we have now written the name of the library after the trap - naim

//ccw 18 apr 2002 : SPLIT
  assert(count<=BYTES_TO_SAVE);
  // The dyninst API doesn't load the socket library

//ccw 18 apr 2002 : SPLIT
  count = 0; // reset count

  // at this time, we know the offset for the library name, so we fix the
  // call to dlopen and we just write the code again! This is probably not
  // very elegant, but it is easy and it works - naim
  removeAst(dlopenAst); // to avoid leaking memory
  dlopenAstArgs[0] = new AstNode(AstNode::Constant, (void *)(dyninstlib_addr));
  dlopenAstArgs[1] = new AstNode(AstNode::Constant, (void*)DLOPEN_MODE);
  dlopenAst = new AstNode("dlopen",dlopenAstArgs);
  removeAst(dlopenAstArgs[0]);
  removeAst(dlopenAstArgs[1]);
  dyninst_count = 0; // reset count
  dlopenAst->generateCode(this, dlopenRegSpace, (char *)scratchCodeBuffer,
			  dyninst_count, true, true);
  writeDataSpace((void *)(codeBase+count), dyninst_count,
                 (char *)scratchCodeBuffer);
  removeAst(dlopenAst);
  count += dyninst_count;

  // save registers
  savedRegs = new dyn_saved_regs;
  bool status = getRepresentativeLWP()->getRegisters(savedRegs);
  assert(status == true);

#if defined(i386_unknown_solaris2_5)
  /* Setup a new stack frame large enough for arguments to functions
     called during bootstrap, as generated by the FuncCall AST.  */
  prgregset_t regs;
  regs = *(prgregset_t*)&savedRegs;
  Address theESP = regs[SP_REG];
  if (!changeIntReg(FP_REG, theESP)) {
    logLine("WARNING: changeIntReg failed in loadDYNINSTlib\n");
    assert(0);
  }
  if (!changeIntReg(SP_REG, theESP-32)) {
    logLine("WARNING: changeIntReg failed in loadDYNINSTlib\n");
    assert(0);
  }
#endif
  if (!getRepresentativeLWP()->changePC(codeBase, NULL)) {
    logLine("WARNING: changePC failed in loadDYNINSTlib\n");
    assert(0);
  }
  setBootstrapState(loadingRT);
  return true;
}

bool process::trapDueToDyninstLib()
{
  if (dyninstlib_brk_addr == 0) return(false);
  return checkAllThreadsForBreakpoint(this, dyninstlib_brk_addr);
}

bool process::loadDYNINSTlibCleanup()
{
  // rewrite original instructions in the text segment we use for 
  // the inferiorRPC - naim
  unsigned count = sizeof(savedCodeBuffer);
  //Address codeBase = getImage()->codeOffset();

  int_function *_startfn = this->findOnlyOneFunction("_start");
  if (NULL == _startfn) {
    pdvector<int_function *> funcs;
    if (!this->findAllFuncsByName("_start", funcs) || !funcs.size()) {
       fprintf(stderr, "%s[%d]:  could not find _start()\n", __FILE__, __LINE__);
       return false;
    }
    fprintf(stderr, "%s[%d]:  WARN:  found %d matches for _start()\n",
            __FILE__, __LINE__, funcs.size());
    int select_fn = 0;
    for (unsigned int i = 0; i < funcs.size(); ++i) {
      const char *modname = funcs[i]->pdmod()->fileName().c_str();
      fprintf(stderr, "\t[%d]\tin module %s\n",i, modname);
      if (strstr(modname, "libc.") || strstr(modname, "libdl."))
         select_fn = i;
    }
    fprintf(stderr, "%s[%d]: selecting %d\n", __FILE__, __LINE__, select_fn);
    _startfn = funcs[select_fn];
  }

  Address codeBase = _startfn->getEffectiveAddress(this);
  assert(codeBase);
  if (!writeDataSpace((void *)codeBase, count, (char *)savedCodeBuffer))
     return false;

  // restore registers
  if (!getRepresentativeLWP()->restoreRegisters(*savedRegs))
     return false;
  delete savedRegs;
  savedRegs = NULL;
  return true;
}

fileDescriptor *getExecFileDescriptor(pdstring filename, int &, bool)
{
    fileDescriptor *desc = new fileDescriptor(filename);
    return desc;
}

#if defined(USES_DYNAMIC_INF_HEAP)
static const Address lowest_addr = 0x0;
void process::inferiorMallocConstraints(Address near, Address &lo, Address &hi,
                                        inferiorHeapType /* type */)
{
  if (near)
    {
      lo = region_lo(near);
      hi = region_hi(near);  
    }
}

void process::inferiorMallocAlign(unsigned &size)
{
     /* 32 byte alignment.  Should it be 64? */
  size = (size + 0x1f) & ~0x1f;
}
#endif

bool process::dumpCore_(const pdstring coreName) 
{
  char command[100];

  sprintf(command, "gcore %d 2> /dev/null; mv core.%d %s", getPid(), getPid(), 
          coreName.c_str());

  detach(false);
  system(command);
  attach();

  return false;
}

int getNumberOfCPUs()
{
  // _SC_NPROCESSORS_CONF is the number of processors configured in the
  // system and _SC_NPROCESSORS_ONLN is the number of those processors that
  // are online.
  int numberOfCPUs;
  numberOfCPUs = (int) sysconf(_SC_NPROCESSORS_ONLN);
  if (numberOfCPUs) 
    return(numberOfCPUs);
  else 
    return(1);
}  

Frame Frame::getCallerFrame()
{
   Frame ret;
   ret.lwp_ = lwp_;
   ret.thread_ = thread_;
   ret.proc_ = proc_;
   ret.pid_ = pid_;
   ret.thread_ = thread_;
   ret.lwp_ = lwp_;
   if (uppermost_) {
     codeRange *range = getRange();
       int_function *func = range->is_function();
       if (func) {
           if (func->hasNoStackFrame()) { // formerly "isLeafFunc()"
               if (lwp_) { // We have a LWP and are prepared to use it
                   struct dyn_saved_regs regs;
                   bool status = lwp_->getRegisters(&regs);
                   assert(status == true);
                   ret.pc_ = regs.theIntRegs[R_O7] + 8;
                   ret.fp_ = fp_; // frame pointer unchanged
                   ret.frameType_ = FRAME_normal;
                   return ret;
                   
               }
               else if (thread_)
                   cerr << "Not implemented yet" << endl;
               else {
                   struct dyn_saved_regs regs;
                   bool status = getProc()->getRepresentativeLWP()->getRegisters(&regs);
                   assert(status == true);
                   ret.pc_ = regs.theIntRegs[R_O7] + 8;
                   ret.fp_ = fp_;
                   ret.frameType_ = FRAME_normal;
                   return ret;
               }
           }
       }
   }
   //
   // For the sparc, register %i7 is the return address - 8 and the fp is
   // register %i6. These registers can be located in %fp+14*5 and
   // %fp+14*4 respectively, but to avoid two calls to readDataSpace,
   // we bring both together (i.e. 8 bytes of memory starting at %fp+14*4
   // or %fp+56).
   // These values are copied to the stack when the application is paused,
   // so we are assuming that the application is paused at this point
   
   struct {
      Address fp;
      Address rtn;
   } addrs;
   
   if (getProc()->readDataSpace((caddr_t)(fp_ + 56), 2*sizeof(int),
                        (caddr_t)&addrs, true))
   {
      ret.fp_ = addrs.fp;
      ret.pc_ = addrs.rtn + 8;
      
      // Check if we're in a sig handler, since we don't know if the
      // _current_ frame has its type set at all.
      if (getProc()->isInSignalHandler(pc_)) {
         // get the value of the saved PC: this value is stored in the
         // address specified by the value in register i2 + 44. Register i2
         // must contain the address of some struct that contains, among
         // other things, the saved PC value.
         u_int reg_i2;
         if (getProc()->readDataSpace((caddr_t)(fp_+40), sizeof(u_int),
                              (caddr_t)&reg_i2,true)) {
            Address saved_pc;
            if (getProc()->readDataSpace((caddr_t) (reg_i2+44), sizeof(int),
                                 (caddr_t) &saved_pc,true)) {
               
               int_function *func = getProc()->findFuncByAddr(saved_pc);
               
               ret.pc_ = saved_pc;
               if (func && func->hasNoStackFrame())
                  ret.fp_ = fp_;
               return ret;
            }
         }
         return Frame();
      }
      
      // If we're in a base tramp, skip this frame (return getCallerFrame)
      // as we only return minitramps
      codeRange *range = getRange();
      if (range->is_basetramp())
          return ret.getCallerFrame();

      if(getProc()->multithread_capable()) {
         // MT thread adds another copy of the start function
         // to the top of the stack... this breaks instrumentation
         // since we think we're at a function entry.
         if (ret.fp_ == 0) ret.pc_ = 0;
      }

      // Check if the _current_ PC is in a signal handler, and if so set the type
      if (getProc()->isInSignalHandler(ret.pc_)) {
          ret.frameType_ = FRAME_signalhandler;
      }

      return ret;
   }
  
   return Frame(); // zero frame
}

bool Frame::setPC(Address newpc) {
  fprintf(stderr, "Implement me! Changing frame PC from %x to %x\n",
	  pc_, newpc);
  return true;
}


#if !defined(BPATCH_LIBRARY)
rawTime64 dyn_lwp::getRawCpuTime_hw()
{
  return 0;
}

/* return unit: nsecs */
rawTime64 dyn_lwp::getRawCpuTime_sw() 
{
  // returns user time from the u or proc area of the inferior process,
  // which in turn is presumably obtained by using a /proc ioctl to obtain
  // it (solaris).  It must not stop the inferior process in order to obtain
  // the result, nor can it assure that the inferior has been stopped.  The
  // result MUST be "in sync" with rtinst's DYNINSTgetCPUtime().
  
  rawTime64 result;
  prusage_t theUsage;

#ifdef PURE_BUILD
  // explicitly initialize "theUsage" struct (to pacify Purify)
  memset(&theUsage, '\0', sizeof(prusage_t));
#endif

  // compute the CPU timer for the whole process
  if(is_attached()) {
     if(pread(usage_fd(), &theUsage, sizeof(prusage_t), 0) 
        != sizeof(prusage_t))
     {
        perror("getInfCPU: read");
        return -1;  // perhaps the process ended
     }
  }
  else return -1; // perhaps the process ended

  result =  (theUsage.pr_utime.tv_sec + theUsage.pr_stime.tv_sec) * 1000000000LL;
  result += (theUsage.pr_utime.tv_nsec+ theUsage.pr_stime.tv_nsec);

  if (result < sw_previous_) // Time ran backwards?
  {
      // When the process exits we often get a final time call.
      // If the result is 0(.0), don't print an error.
      if (result) {
          char errLine[150];
          sprintf(errLine,"process::getRawCpuTime_sw - time going backwards in "
                  "daemon - cur: %lld, prev: %lld\n", result, sw_previous_);
          cerr << errLine;
          logLine(errLine);
      }
      result = sw_previous_;
  }
  else sw_previous_=result;
  
  return result;
}
#endif // BPATCH_LIBRARY


bool process::instrSideEffect(Frame &frame, instPoint *inst)
{
  int_function *instFunc = inst->pointFunc();
  if (!instFunc) return false;

  codeRange *range = frame.getRange();
  if (range->is_function() != instFunc) {
    return true;
  }

  if (inst->getPointType() == callSite) {
    Address insnAfterPoint = inst->absPointAddr(this) + 2*sizeof(instruction);

    if (frame.getPC() == insnAfterPoint) {
      frame.setPC(baseMap[inst]->baseAddr + baseMap[inst]->skipPostInsOffset);
    }
  }

  return true;
}

#if 0
// needToAddALeafFrame: returns true if the between the current frame 
// and the next frame there is a leaf function (this occurs when the 
// current frame is the signal handler and the function that was executing
// when the sighandler was called is a leaf function)
bool process::needToAddALeafFrame(Frame current_frame, Address &leaf_pc){

   // check to see if the current frame is the signal handler 
   Address frame_pc = current_frame.getPC();
   Address sig_addr = 0;
   const image *sig_image = (signal_handler->file())->exec();
   if(getBaseAddress(sig_image, sig_addr)){
       sig_addr += signal_handler->getAddress(0);
   } else {
       sig_addr = signal_handler->getAddress(0);
   }
   u_int sig_size = signal_handler->size();
   if(signal_handler&&(frame_pc >= sig_addr)&&(frame_pc < (sig_addr+sig_size))){
       // get the value of the saved PC: this value is stored in the address
       // specified by the value in register i2 + 44. Register i2 must contain
       // the address of some struct that contains, among other things, the 
       // saved PC value.  
       u_int reg_i2;
       int fp = current_frame.getFP();
       if (readDataSpace((caddr_t)(fp+40),sizeof(u_int),(caddr_t)&reg_i2,true)){
          if (readDataSpace((caddr_t) (reg_i2+44), sizeof(int),
			    (caddr_t) &leaf_pc,true)){
	      // if the function is a leaf function return true
	      int_function *func = findFuncByAddr(leaf_pc);
	      if(func && func->hasNoStackFrame()) { // formerly "isLeafFunc()"
		  return(true);
	      }
          }
      }
   }
   return false;
}
#endif

void print_read_error_info(const relocationEntry entry, 
                           int_function *&target_pdf, Address base_addr) {

   sprintf(errorLine, "  entry      : target_addr 0x%lx\n",
           entry.target_addr());
   logLine(errorLine);
   sprintf(errorLine, "               rel_addr 0x%lx\n", entry.rel_addr());
   logLine(errorLine);
   sprintf(errorLine, "               name %s\n", (entry.name()).c_str());
   logLine(errorLine);
   
   if (target_pdf) {
      sprintf(errorLine, "  target_pdf : symTabName %s\n",
              (target_pdf->symTabName()).c_str());
      logLine(errorLine);    
      sprintf(errorLine , "              prettyName %s\n",
              (target_pdf->symTabName()).c_str());
      logLine(errorLine);
      sprintf(errorLine , "              size %i\n",
              target_pdf->get_size());
      logLine(errorLine);
      sprintf(errorLine , "              addr 0x%lx\n",
              target_pdf->get_address());
      logLine(errorLine);
   }

   sprintf(errorLine, "  base_addr  0x%lx\n", base_addr);
   logLine(errorLine);
}

// hasBeenBound: returns true if the runtime linker has bound the
// function symbol corresponding to the relocation entry in at the address
// specified by entry and base_addr.  If it has been bound, then the callee 
// function is returned in "target_pdf", else it returns false.
bool process::hasBeenBound(const relocationEntry entry, 
			   int_function *&target_pdf, Address base_addr) {

// TODO: the x86 and sparc versions should really go in seperate files 
#if defined(i386_unknown_solaris2_5)

    if (status() == exited) return false;

    // if the relocationEntry has not been bound yet, then the value
    // at rel_addr is the address of the instruction immediately following
    // the first instruction in the PLT entry (which is at the target_addr) 
    // The PLT entries are never modified, instead they use an indirrect 
    // jump to an address stored in the _GLOBAL_OFFSET_TABLE_.  When the 
    // function symbol is bound by the runtime linker, it changes the address
    // in the _GLOBAL_OFFSET_TABLE_ corresponding to the PLT entry

    Address got_entry = entry.rel_addr() + base_addr;
    Address bound_addr = 0;
    if(!readDataSpace((const void*)got_entry, sizeof(Address), 
			&bound_addr, true)){
        sprintf(errorLine, "read error in process::hasBeenBound "
		"addr 0x%lx, pid=%d\n (readDataSpace returns 0)",
		got_entry, pid);
	logLine(errorLine);
	print_read_error_info(entry, target_pdf, base_addr);
        return false;
    }

    if( !( bound_addr == (entry.target_addr()+6+base_addr)) ) {
        // the callee function has been bound by the runtime linker
	// find the function and return it
        target_pdf = findFuncByAddr(bound_addr);
	if(!target_pdf){
            return false;
	}
        return true;	
    }
    return false;

#else
    // if the relocationEntry has not been bound yet, then the second instr 
    // in this PLT entry branches to the fist PLT entry.  If it has been   
    // bound, then second two instructions of the PLT entry have been changed 
    // by the runtime linker to jump to the address of the function.  
    // Here is an example:   
    //     before binding			after binding
    //	   --------------			-------------
    //     sethi  %hi(0x15000), %g1		sethi  %hi(0x15000), %g1
    //     b,a  <_PROCEDURE_LINKAGE_TABLE_>     sethi  %hi(0xef5eb000), %g1
    //	   nop					jmp  %g1 + 0xbc ! 0xef5eb0bc

    instruction next_insn;
    Address next_insn_addr = entry.target_addr() + base_addr + 4; 
    if( !(readDataSpace((caddr_t)next_insn_addr, sizeof(next_insn), 
		       (char *)&next_insn, true)) ) {
        sprintf(errorLine, "read error in process::hasBeenBound addr 0x%lx"
                " (readDataSpace next_insn_addr returned 0)\n",
		next_insn_addr);
	logLine(errorLine);
	print_read_error_info(entry, target_pdf, base_addr);
    }
    // if this is a b,a instruction, then the function has not been bound
    if((next_insn.branch.op == FMT2op)  && (next_insn.branch.op2 == BICCop2) 
       && (next_insn.branch.anneal == 1) && (next_insn.branch.cond == BAcond)) {
	return false;
    } 

    // if this is a sethi instruction, then it has been bound...get target_addr
    instruction third_insn;
    Address third_addr = entry.target_addr() + base_addr + 8; 
    if( !(readDataSpace((caddr_t)third_addr, sizeof(third_insn), 
		       (char *)&third_insn, true)) ) {
        sprintf(errorLine, "read error in process::hasBeenBound addr 0x%lx"
                " (readDataSpace third_addr returned 0)\n",
		third_addr);
	logLine(errorLine);
	print_read_error_info(entry,target_pdf, base_addr);
    }

    // get address of bound function, and return the corr. int_function
    if((next_insn.sethi.op == FMT2op) && (next_insn.sethi.op2 == SETHIop2)
	&& (third_insn.rest.op == RESTop) && (third_insn.rest.i == 1)
	&& (third_insn.rest.op3 == JMPLop3)) {
        
	Address new_target = (next_insn.sethi.imm22 << 10) & 0xfffffc00; 
	new_target |= third_insn.resti.simm13;

        target_pdf = findFuncByAddr(new_target);
	if(!target_pdf){
            return false;
	}
	return true;
    }
    // this is a messed up entry
    return false;
#endif

}



// findCallee: finds the function called by the instruction corresponding
// to the instPoint "instr". If the function call has been bound to an
// address, then the callee function is returned in "target" and the 
// instPoint "callee" data member is set to pt to callee's int_function.  
// If the function has not yet been bound, then "target" is set to the 
// int_function associated with the name of the target function (this is 
// obtained by the PLT and relocation entries in the image), and the instPoint
// callee is not set.  If the callee function cannot be found, (ex. function
// pointers, or other indirect calls), it returns false.
// Returns false on error (ex. process doesn't contain this instPoint).
//
// The assumption here is that for all processes sharing the image containing
// this instPoint they are going to bind the call target to the same function. 
// For shared objects this is always true, however this may not be true for
// dynamic executables.  Two a.outs can be identical except for how they are
// linked, so a call to fuction foo in one version of the a.out may be bound
// to function foo in libfoo.so.1, and in the other version it may be bound to 
// function foo in libfoo.so.2.  We are currently not handling this case, since
// it is unlikely to happen in practice.
bool process::findCallee(instPoint &instr, int_function *&target){
   if((target = instr.getCallee())) {
      return true; // callee already set
   }

   // find the corresponding image in this process  
   image *owner = instr.getOwner();
   bool found_image = false;
   Address base_addr = 0;
   if(symbols == owner) {  found_image = true; } 
   else if(shared_objects){
      for(u_int i=0; i < shared_objects->size(); i++){
         if(owner == ((*shared_objects)[i])->getImage()) {
            found_image = true;
            base_addr = ((*shared_objects)[i])->getBaseAddress();
            break;
         }
      }
   } 
   if(!found_image) {
      target = 0;
      return false; // image not found...this is bad
   }

   // get the target address of this function
   Address target_addr = 0;
   //    Address insn_addr = instr.pointAddr(); 
   target_addr = instr.getTargetAddress();

   if(!target_addr) {  
      // this is either not a call instruction or an indirect call instr
      // that we can't get the target address
      target = 0;
      return false;
   }

#if defined(sparc_sun_solaris2_4)
   /*
   // I don't see how this is possible -- we relocate after we've determined static
   // callees -- bernat, 13OCT03
   // If this instPoint is from a function that was relocated to the heap
   // then need to get the target address relative to this image   
   if(target_addr && instr.relocated_) {
   assert(target_addr > base_addr);
   target_addr -= base_addr;
   }
   */
#endif

   // see if there is a function in this image at this target address
   // if so return it
   int_function *pdf = 0;
   if( (pdf = owner->findFuncByEntry(target_addr)) ) {
      target = pdf;
      instr.setCallee(pdf);
      return true; // target found...target is in this image
   }

   // else, get the relocation information for this image
   const Object &obj = owner->getObject();
   const pdvector<relocationEntry> *fbt;
   if(!obj.get_func_binding_table_ptr(fbt)) {
      target = 0;
      return false; // target cannot be found...it is an indirect call.
   }

   // find the target address in the list of relocationEntries
   for(u_int i=0; i < fbt->size(); i++) {
      if((*fbt)[i].target_addr() == target_addr) {
         // check to see if this function has been bound yet...if the
         // PLT entry for this function has been modified by the runtime
         // linker
         int_function *target_pdf = 0;
         if(hasBeenBound((*fbt)[i], target_pdf, base_addr)) {
            target = target_pdf;
            instr.setCallee(target_pdf);
            return true;  // target has been bound
         } 
         else {
            // just try to find a function with the same name as entry 
            pdvector<int_function *> pdfv;
            bool found = findAllFuncsByName((*fbt)[i].name(), pdfv);
            if(found) {
               assert(pdfv.size());
#ifdef BPATCH_LIBRARY
               if(pdfv.size() > 1)
                  cerr << __FILE__ << ":" << __LINE__ 
                       << ": WARNING:  findAllFuncsByName found " 
                       << pdfv.size() << " references to function " 
                       << (*fbt)[i].name() << ".  Using the first.\n";
#endif
               target = pdfv[0];
               return true;
            }
            else {  
               // KLUDGE: this is because we are not keeping more than
               // one name for the same function if there is more
               // than one.  This occurs when there are weak symbols
               // that alias global symbols (ex. libm.so.1: "sin" 
               // and "__sin").  In most cases the alias is the same as 
               // the global symbol minus one or two leading underscores,
               // so here we add one or two leading underscores to search
               // for the name to handle the case where this string 
               // is the name of the weak symbol...this will not fix 
               // every case, since if the weak symbol and global symbol
               // differ by more than leading underscores we won't find
               // it...when we parse the image we should keep multiple
               // names for int_functions

               pdstring s("_");
               s += (*fbt)[i].name();
               found = findAllFuncsByName(s, pdfv);
               if(found) {
                  assert(pdfv.size());
#ifdef BPATCH_LIBRARY
                  if(pdfv.size() > 1)
                     cerr << __FILE__ << ":" << __LINE__ 
                          << ": WARNING: findAllFuncsByName found " 
                          << pdfv.size() << " references to function " 
                          << s << ".  Using the first.\n";
#endif
                  target = pdfv[0];
                  return true;
               }
		    
               s = pdstring("__");
               s += (*fbt)[i].name();
               found = findAllFuncsByName(s, pdfv);
               if(found) {
                  assert(pdfv.size());
#ifdef BPATCH_LIBRARY
                  if(pdfv.size() > 1)
                     cerr << __FILE__ << ":" << __LINE__ 
                          << ": WARNING: findAllFuncsByName found " 
                          << pdfv.size() << " references to function "
                          << s << ".  Using the first.\n";
#endif
                  target = pdfv[0];
                  return true;
               }
               //		    else
               // 		        cerr << __FILE__ << ":" << __LINE__
               // 			     << ": WARNING: findAllFuncsByName found no "
               // 			     << "matches for function " << (*fbt)[i].name() 
               // 			     << " or its possible aliases\n";
            }
         }
         target = 0;
         return false;
      }
   }
   target = 0;
   return false;  
}

void loadNativeDemangler() {
  
  P_native_demangle = NULL;
  void *hDemangler = dlopen("libdemangle.so", DLOPEN_MODE);
  if (hDemangler != NULL)
    P_native_demangle = (int (*) (const char *, char *, size_t)) 
      dlsym(hDemangler, "cplus_demangle");
}
