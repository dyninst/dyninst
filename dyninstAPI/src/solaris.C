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

// $Id: solaris.C,v 1.125 2002/10/18 22:41:13 bernat Exp $

#include "dyninstAPI/src/symtab.h"
#include "common/h/headers.h"
#include "dyninstAPI/src/os.h"
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

#include <sys/procfs.h>
#include <stropts.h>
#include <poll.h>
#include <limits.h>
#include <link.h>
#include <dlfcn.h>

#include "dyn_lwp.h"

#define DLOPEN_MODE (RTLD_NOW | RTLD_GLOBAL)

extern "C" {
extern int ioctl(int, int, ...);
extern long sysconf(int);
};

// The following were defined in process.C
extern debug_ostream attach_cerr;
extern debug_ostream inferiorrpc_cerr;
extern debug_ostream shmsample_cerr;
extern debug_ostream forkexec_cerr;
extern debug_ostream signal_cerr;

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

/*
   osTraceMe is called after we fork a child process to set
   a breakpoint on the exit of the exec system call.
   When /proc is used, this breakpoint **will not** cause a SIGTRAP to 
   be sent to the process. The parent should use PIOCWSTOP to wait for 
   the child.
*/
void OS::osTraceMe(void) {
  sysset_t exitSet;
  char procName[128];

  sprintf(procName,"/proc/%05d", (int)getpid());
  int fd = P_open(procName, O_RDWR, 0);
  if (fd < 0) {
    fprintf(stderr, "osTraceMe: open failed: %s\n", sys_errlist[errno]); 
    fflush(stderr);
    P__exit(-1); // must use _exit here.
  }

  /* set a breakpoint at the exit of exec/execve */
  premptyset(&exitSet);
  praddset(&exitSet, SYS_exec);
  praddset(&exitSet, SYS_execve);

  if (ioctl(fd, PIOCSEXIT, &exitSet) < 0) {
    fprintf(stderr, "osTraceMe: PIOCSEXIT failed: %s\n", sys_errlist[errno]); 
    fflush(stderr);
    P__exit(-1); // must use _exit here.
  }

  errno = 0;
  close(fd);
  return;
}


// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) {
  int ttyfd = open ("/dev/tty", O_RDONLY);
  ioctl (ttyfd, TIOCNOTTY, NULL); 
  P_close (ttyfd);
}

bool process::continueWithForwardSignal(int) {
   if (-1 == ioctl(getDefaultLWP()->get_fd(), PIOCRUN, NULL)) {
      perror("could not forward signal in PIOCRUN");
      return false;
   }

   return true;
}


#ifdef BPATCH_LIBRARY

char* process::dumpPatchedImage(string imageFileName){ //ccw 28 oct 2001

	writeBackElf *newElf;
	addLibrary *addLibraryElf;
	void *data, *paddedData;
	unsigned int errFlag=0;
	char name[50];	
	vector<imageUpdate*> compactedUpdates;
	vector<imageUpdate*> compactedHighmemUpdates;
	Address guardFlagAddr= trampGuardAddr();
	char *mutatedSharedObjects=0;
	int mutatedSharedObjectsSize = 0, mutatedSharedObjectsIndex=0;
	char *directoryName = 0;
	shared_object *sh_obj;
	if(!collectSaveWorldData){
		BPatch_reportError(BPatchSerious,122,"dumpPatchedImage: BPatch_thread::startSaveWorld() not called.  No mutated binary saved\n");
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
		// This is going to be a section that looks like this:
		// string
		// addr
		// ...
		// string
		// addr
		
		mutatedSharedObjectsSize += mutatedSharedObjectsNumb * sizeof(unsigned int);
		mutatedSharedObjects = new char[mutatedSharedObjectsSize ];
		for(unsigned i=0;shared_objects && i<shared_objects->size() ; i++) {
			sh_obj = (*shared_objects)[i];
			if(sh_obj->isDirty()){
				memcpy(  & ( mutatedSharedObjects[mutatedSharedObjectsIndex]),
					sh_obj->getName().c_str(),
					strlen(sh_obj->getName().c_str())+1);
				mutatedSharedObjectsIndex += strlen(
					sh_obj->getName().c_str())+1;
				unsigned int baseAddr = sh_obj->getBaseAddress();
				memcpy( & (mutatedSharedObjects[mutatedSharedObjectsIndex]),
					&baseAddr, sizeof(unsigned int));
				mutatedSharedObjectsIndex += sizeof(unsigned int);	
			}
		}	
	}
	char *dyninst_SharedLibrariesData =saveWorldCreateSharedLibrariesSection(dyninst_SharedLibrariesSize);

	newElf = new writeBackElf(( char*) getImage()->file().c_str(),
		"/tmp/dyninstMutatee",errFlag);
	newElf->registerProcess(this);

#ifndef USE_STL_VECTOR
	imageUpdates.sort(imageUpdate::imageUpdateSort);// imageUpdate::mysort ); 
#else
	sort(imageUpdates.begin(), imageUpdates.end(), imageUpdateOrderingRelation());
#endif

	newElf->compactLoadableSections(imageUpdates,compactedUpdates);

#ifndef USE_STL_VECTOR
	highmemUpdates.sort( imageUpdate::imageUpdateSort);
#else
	sort(highmemUpdates.begin(), highmemUpdates.end(), imageUpdateOrderingRelation());
#endif

	newElf->compactSections(highmemUpdates, compactedHighmemUpdates);

	newElf->alignHighMem(compactedHighmemUpdates);

	int sectionsAdded = 0;
	unsigned int newSize, nextPage, paddedDiff;
	unsigned int pageSize = getpagesize();


	//This adds the LOADABLE HEAP TRAMP sections
	//if new platforms require this it will be moved
	//to process.C proper. Right now, only solaris uses it
	for(unsigned int i=0;i<compactedUpdates.size();i++){

		(char*) data = new char[compactedUpdates[i]->size];
		readDataSpace((void*) compactedUpdates[i]->address, compactedUpdates[i]->size, 	
			data, true);	

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


	saveWorldCreateDataSections((void*)newElf);

	if(mutatedSharedObjectsSize){
		newElf->addSection(0 ,mutatedSharedObjects, 
			mutatedSharedObjectsSize, "dyninstAPI_mutatedSO", false);
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


	newElf->createElf();
	char* fullName = new char[strlen(directoryName) + strlen ( (char*)imageFileName.c_str())+1];
	strcpy(fullName, directoryName);
	strcat(fullName, (char*)imageFileName.c_str());
	
	addLibraryElf = new addLibrary();
	elf_update(newElf->getElf(), ELF_C_WRITE);
	addLibraryElf->driver(newElf->getElf(),fullName,"libdyninstAPI_RT.so.1");
	delete [] fullName;
	if(mutatedSharedObjects){
		delete [] mutatedSharedObjects;
	}
	return directoryName;
}
#endif

#ifndef BPATCH_LIBRARY
bool process::dumpImage() {return false;}
#else
bool process::dumpImage(string imageFileName) 
{
    int newFd;
    image *im;
    string command;

    im = getImage();
    string origFile = im->file();


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


    char tempCode[length];


    bool ret = readTextSpace_((void *) baseAddr, length, tempCode);
    if (!ret) {
       // log error
       return false;
    }

    lseek(newFd, offset, SEEK_SET);
    write(newFd, tempCode, length);
    close(newFd);

    return true;
}
#endif

#ifdef BPATCH_LIBRARY
/*
 * Use by dyninst to set events we care about from procfs
 *
 */
bool process::setProcfsFlags()
{

  long flags = PR_BPTADJ;
  if (BPatch::bpatch->postForkCallback) {
      // cause the child to inherit trap-on-exit from exec and other traps
      // so we can learn of the child (if the user cares)
      flags = PR_BPTADJ | PR_FORK | PR_ASYNC | PR_RLC;
  }

  if (ioctl (getDefaultLWP()->get_fd(), PIOCSET, &flags) < 0) {
    fprintf(stderr, "attach: PIOCSET failed: %s\n", sys_errlist[errno]);
    close(getDefaultLWP()->get_fd());
    return false;
  }

  // cause a stop on the exit from fork
  sysset_t sysset;

  if (ioctl(getDefaultLWP()->get_fd(), PIOCGEXIT, &sysset) < 0) {
    fprintf(stderr, "attach: ioctl failed: %s\n", sys_errlist[errno]);
    close(getDefaultLWP()->get_fd());
    return false;
  }

  if (BPatch::bpatch->postForkCallback) {
      praddset (&sysset, SYS_fork);
      praddset (&sysset, SYS_fork1);
      praddset (&sysset, SYS_exec);
      praddset (&sysset, SYS_execve);
  }
  
  if (ioctl(getDefaultLWP()->get_fd(), PIOCSEXIT, &sysset) < 0) {
    fprintf(stderr, "attach: ioctl failed: %s\n", sys_errlist[errno]);
    close(getDefaultLWP()->get_fd());
    return false;
  }

  // now worry about entry too
  if (ioctl(getDefaultLWP()->get_fd(), PIOCGENTRY, &sysset) < 0) {
    fprintf(stderr, "attach: ioctl failed: %s\n", sys_errlist[errno]);
    close(getDefaultLWP()->get_fd());
    return false;
  }

  if (BPatch::bpatch->exitCallback) {
      praddset (&sysset, SYS_exit);
  }

  if (BPatch::bpatch->preForkCallback) {
      praddset (&sysset, SYS_fork);
      praddset (&sysset, SYS_fork1);
      praddset (&sysset, SYS_vfork);
      // praddset (&sysset, SYS_waitsys);
  }

  // should these be for exec callback??
  prdelset (&sysset, SYS_exec);
  prdelset (&sysset, SYS_execve);
  
  if (ioctl(getDefaultLWP()->get_fd(), PIOCSENTRY, &sysset) < 0) {
    fprintf(stderr, "attach: ioctl failed: %s\n", sys_errlist[errno]);
    close(getDefaultLWP()->get_fd());
    return false;
  }

  return true;
}
#endif

/* 
   execResult: return the result of an exec system call - true if succesful.
   The traced processes will stop on exit of an exec system call, just before
   returning to user code. At this point the return value (errno) is already
   written to a register, and we need to check if the return value is zero.
 */
static inline bool execResult(prstatus_t stat) {
  return (stat.pr_reg[RETVAL_REG] == 0);
}

/*
   wait for inferior processes to terminate or stop.
*/
#ifdef BPATCH_LIBRARY
int process::waitProcs(int *status, bool block) {
#else
int process::waitProcs(int *status) {
#endif
   extern vector<process*> processVec;

   static struct pollfd fds[OPEN_MAX];  // argument for poll
   static int selected_fds;             // number of selected
   static int curr;                     // the current element of fds

#ifdef BPATCH_LIBRARY
   do {
#endif

   /* Each call to poll may return many selected fds. Since we only report the
      status of one process per call to waitProcs, we keep the result of the
      last poll buffered, and simply process an element from the buffer until
      all of the selected fds in the last poll have been processed.
   */

#ifdef BPATCH_LIBRARY
   // force a fresh poll each time, processes may have been added/deleted
   //   since the last call. - jkh 1/31/00
   selected_fds = 0;
#endif

    if (selected_fds == 0) {
     // printf("polling for: ");
     for (unsigned u = 0; u < processVec.size(); u++) {
       // printf("checking %d\n", processVec[u]->getPid());
       if (processVec[u] && 
                (processVec[u]->status() == running || 
                 processVec[u]->status() == neonatal)) {
	   // printf("   polling %d\n", processVec[u]->getPid());
	   fds[u].fd = processVec[u]->getDefaultLWP()->get_fd();
       } else {
	   fds[u].fd = -1;
       }	
       fds[u].events = POLLPRI;
       fds[u].revents = 0;
     }
     // printf("\n");

#ifdef BPATCH_LIBRARY
     int timeout;
     if (block) timeout = INFTIM;
     else timeout = 0;
     selected_fds = poll(fds, processVec.size(), timeout);
#else
     selected_fds = poll(fds, processVec.size(), 0);
#endif
     if (selected_fds < 0) {
       fprintf(stderr, "waitProcs: poll failed: %s\n", sys_errlist[errno]);
       selected_fds = 0;
       return 0;
     }

     curr = 0;
   }
   
   if (selected_fds > 0) {
     while (fds[curr].revents == 0)
       ++curr;

     // fds[curr] has an event of interest
     prstatus_t stat;
     int ret = 0;

#ifdef PURE_BUILD
     // explicitly initialize "stat" struct (to pacify Purify)
     // (at least initialize those components which we actually use)
     stat.pr_flags = 0x0000;
     stat.pr_why   = 0;
     stat.pr_what  = 0;
     stat.pr_reg[RETVAL_REG] = 0;
#endif

#ifdef BPATCH_LIBRARY
     if (fds[curr].revents & POLLHUP) {
	 do {
	     ret = waitpid(processVec[curr]->getPid(), status, 0);
	 } while ((ret < 0) && (errno == EINTR));
	 ret = -1;
	 if (ret < 0) {
	     // This means that the application exited, but was not our child
	     // so it didn't wait around for us to get it's return code.  In
	     // this case, we can't know why it exited or what it's return
	     // code was.
	     ret = processVec[curr]->getPid();
	     *status = 0;
	     // is this the bug??
	     // processVec[curr]->continueProc_();
	 }
	 assert(ret == processVec[curr]->getPid());
     } else
#endif
     if (ioctl(fds[curr].fd, PIOCSTATUS, &stat) != -1 
	 && ((stat.pr_flags & PR_STOPPED) || (stat.pr_flags & PR_ISTOP))) {
       switch (stat.pr_why) {
       case PR_SIGNALLED:
	 // return the signal number
	 *status = stat.pr_what << 8 | 0177;
	 ret = processVec[curr]->getPid();
	 if (!processVec[curr]->dyninstLibAlreadyLoaded() && 
	     processVec[curr]->wasCreatedViaAttach()) {
	   // make sure we are stopped in the eyes of paradynd - naim
	   bool wasRunning = (processVec[curr]->status() == running);
	   if (processVec[curr]->status() != stopped)
	     processVec[curr]->Stopped();   
	   if(processVec[curr]->isDynamicallyLinked()) {
	     processVec[curr]->handleIfDueToSharedObjectMapping();
	   }
	   if (processVec[curr]->trapDueToDyninstLib()) {
	     // we need to load libdyninstRT.so.1 - naim
	     processVec[curr]->handleIfDueToDyninstLib();
	   }
	   if (wasRunning) 
	     if (!processVec[curr]->continueProc()) assert(0);
	 }
#if !defined(BPATCH_LIBRARY) //ccw 29 apr 2002 : SPLIT2
	else if(!processVec[curr]->paradynLibAlreadyLoaded() && 
	     processVec[curr]->wasCreatedViaAttach()) {
	  // make sure we are stopped in the eyes of paradynd - naim
	   bool wasRunning = (processVec[curr]->status() == running);
	   if (processVec[curr]->status() != stopped)
	     processVec[curr]->Stopped();   
	   if(processVec[curr]->isDynamicallyLinked()) {
	     processVec[curr]->handleIfDueToSharedObjectMapping();
	   }
	   if (processVec[curr]->trapDueToParadynLib()) {
	     // we need to load libdyninstRT.so.1 - naim
	     processVec[curr]->handleIfDueToDyninstLib();
	   }
	   if (wasRunning) 
	     if (!processVec[curr]->continueProc()) assert(0);

		
	}
#endif
	 break;
       case PR_SYSEXIT: {
	 // exit of a system call.
	 process *p = processVec[curr];

	 if (p->isInSyscall()) {
 	    // reset PIOCSEXIT mask
	    // inferiorrpc_cerr << "solaris got PR_SYSEXIT!" << endl;
	    assert(p->save_exitset_ptr != NULL);
	    if (-1 == ioctl(p->getDefaultLWP()->get_fd(), PIOCSEXIT, p->save_exitset_ptr))
	       assert(false);
	    /*
	     * according to process.h, p->save_exitset_ptr is
	     * of type ( sysset_t * ) on Solaris
	     */
	    delete [] ( sysset_t * )( p->save_exitset_ptr );
	    p->save_exitset_ptr = NULL;

	    // fall through on purpose (so status, ret get set)
	 } else if (((stat.pr_what == SYS_exec) || 
		     (stat.pr_what == SYS_execve)) && !execResult(stat)) {
	   // a failed exec. continue the process
           processVec[curr]->continueProc_();
           break;
         }	    

#ifdef BPATCH_LIBRARY
	 if ((stat.pr_what == SYS_fork) || 
	     (stat.pr_what == SYS_fork1)) {
	     int childPid = stat.pr_reg[RETVAL_REG];
	     if (childPid == getpid()) {
		 // this is a special case where the normal createProcess code
		 // has created this process, but the attach routine runs soon
		 // enough that the child (of the mutator) gets a fork exit
		 // event.  We don't care about this event, so we just continue
		 // the process - jkh 1/31/00
		 processVec[curr]->continueProc_();
		 process *theParent = processVec[curr];
		 theParent->status_ = running;
		 return (0);
	     } else if (childPid > 0) {
		 unsigned int i;

		 for (i=0; i < processVec.size(); i++) {
		     if (processVec[i]->getPid() == childPid) break;
		 }
		 if (i== processVec.size()) {
		     // this is a new child, register it with dyninst
		     int parentPid = processVec[curr]->getPid();
		     process *theParent = processVec[curr];
		     process *theChild = new process(*theParent, (int)childPid, -1);
		     // the parent loaded it!
		     theChild->hasLoadedDyninstLib = true;

		     processVec.push_back(theChild);
		     activeProcesses++;

		     // it's really stopped, but we need to mark it running so
		     //   it can report to us that it is stopped - jkh 1/5/00
		     // or should this be exited???
		     theChild->status_ = neonatal;

		     // parent is stopped too (on exit fork event)
		     theParent->status_ = stopped;

		     theChild->execFilePath = 
			 theChild->tryToFindExecutable("", childPid);
		     theChild->inExec = false;
		     BPatch::bpatch->registerForkedThread(parentPid,
			 childPid, theChild);
		 }
	     } else {
		 printf("fork errno %d\n", stat.pr_reg[RETVAL_REG]);
	     }
	 } else if (((stat.pr_what == SYS_exec) || 
		     (stat.pr_what == SYS_execve)) && execResult(stat)) {
	     process *proc = processVec[curr];
	     proc->execFilePath = proc->tryToFindExecutable("", proc->getPid());

	     // only handle if this is in the child - is this right??? jkh
	     if (proc->reachedFirstBreak) {
		 // mark this for the sig TRAP that will occur soon
		 proc->inExec = true;

		 // leave process stopped until signal handler runs
		 // mark it running so we get the stop signal
		 proc->status_ = stopped;

		 // reset buffer pool to empty (exec clears old mappings)
		 vector<heapItem *> emptyHeap;
		 proc->heap.bufferPool = emptyHeap;
	     }
	 } else {
	     printf("got unexpected PIOCSEXIT\n");
	     printf("  call is return from syscall #%d\n", stat.pr_what);
	 }
#endif

	 *status = SIGTRAP << 8 | 0177;
	 ret = processVec[curr]->getPid();
	 break;
       }

#ifdef BPATCH_LIBRARY
       case PR_SYSENTRY: {
	 bool alreadyCont = false;
	 process *p = processVec[curr];

	 if ((stat.pr_what == SYS_fork) || (stat.pr_what == SYS_vfork) ||
	     (stat.pr_what == SYS_fork1)) {
	     if (BPatch::bpatch->preForkCallback) {
		 assert(p->thread);
		 p->setProcfsFlags();
		 BPatch::bpatch->preForkCallback(p->thread, NULL);
	     }

	     if (stat.pr_what == SYS_vfork)  {
		 unsigned int i;
		 int childPid = 0;
		 alreadyCont = true;
		 struct DYNINST_bootstrapStruct bootRec;

		 (void) processVec[curr]->continueProc_();
		 processVec[curr]->status_ = stopped;
		 do {
		     processVec[curr]->extractBootstrapStruct(&bootRec);

		     childPid = bootRec.pid;
		 } while (bootRec.event != 3);

		 for (i=0; i < processVec.size(); i++) {
		     if (processVec[i]->getPid() == childPid) break;
		 }
		 if (i== processVec.size()) {
		     // this is a new child, register it with dyninst
		     int parentPid = processVec[curr]->getPid();
		     process *theParent = processVec[curr];
		     process *theChild = new process(*theParent, (int)childPid, -1);
		     processVec.push_back(theChild);
		     activeProcesses++;

		     // it's really stopped, but we need to mark it running so
		     //   it can report to us that it is stopped - jkh 1/5/00
		     // or should this be exited???
		     theChild->status_ = running;

		     theChild->execFilePath = 
			 theChild->tryToFindExecutable("", childPid);
		     BPatch::bpatch->registerForkedThread(parentPid,
			 childPid, theChild);
		 }
	     }
	 } else if (stat.pr_what == SYS_exit) {
#ifdef i386_unknown_solaris2_5
	     int code;
	     bool readRet;
	     unsigned long sp;
	     
	     sp = stat.pr_reg[SP_REG];
	     // exit code is at sp+4 when the syscall for exit is invoked
	     readRet = processVec[curr]->readDataSpace_((void *) (sp+4), 
		 sizeof(int), (caddr_t) &code);
	     if (!readRet) {
		 char errorLine[256];
	     	 sprintf(errorLine, "in readDataSpace, unable to read at %lx\n",
		     sp+4);
		 logLine(errorLine);
	     }
#else
	     int code = stat.pr_reg[PARAM1_REG];
#endif
	     process *proc = processVec[curr];

	     proc->status_ = stopped;

	     BPatch::bpatch->registerExit(proc->thread, code);

	     proc->continueProc();
	     alreadyCont = true;

	 } else {
	     printf("got PR_SYSENTRY\n");
	     printf("    unhandeled sys call #%d\n", stat.pr_what);
	 }
	 if (!alreadyCont) (void) processVec[curr]->continueProc_();
	 break;
       }
#endif

       case PR_REQUESTED:
	 assert(0);
	 break;
       case PR_JOBCONTROL:
	 assert(0);
	 break;
       }	
     }

     --selected_fds;
     ++curr;

     if (ret > 0) {
       return ret;
     }
   }

#ifdef BPATCH_LIBRARY
   } while (block);
   return 0;
#else
   return waitpid(0, status, WNOHANG);
#endif
}


static char *extract_string_ptr(int procfd, char **ptr) {
   // we want to return *ptr.

   if (-1 == lseek(procfd, (long)ptr, SEEK_SET))
      assert(false);

   char *result;
   if (-1 == read(procfd, &result, sizeof(result)))
      assert(false);

   return result;   
}

string extract_string(int procfd, const char *inferiorptr) {
   // assuming inferiorptr points to a null-terminated string in the inferior
   // process, extract it and return it.

   if (-1 == lseek(procfd, (long)inferiorptr, SEEK_SET))
      return "";

   string result;
   while (true) {
      char buffer[100];
      if (-1 == read(procfd, &buffer, 80))
	 return "";
      buffer[80] = '\0';
      result += buffer;

      // was there a '\0' anywhere in chars 0 thru 79?  If so then
      // we're done
      for (unsigned lcv=0; lcv < 80; lcv++)
	 if (buffer[lcv] == '\0') {
	    //attach_cerr << "extract_string returning " << result << endl;
	    return result;
	 }
   }
}
 
bool get_ps_stuff(int fd, string &argv0, string &pathenv, string &cwdenv) {
  // Use ps info to obtain argv[0], PATH, and curr working directory of the
  // inferior process designated by fd.  Writes to argv0, pathenv, cwdenv.
  
  prpsinfo the_psinfo;
#ifdef PURE_BUILD
  // explicitly initialize "the_psinfo" struct (to pacify Purify)
  // (at least initialize those components which we actually use)
  the_psinfo.pr_zomb = 0;
  the_psinfo.pr_argc = 0;
  the_psinfo.pr_argv = NULL;
  the_psinfo.pr_envp = NULL;
#endif
  
  if (-1 == ioctl(fd, PIOCPSINFO, &the_psinfo))
    return false;
  
  if (the_psinfo.pr_zomb)
    return false;

   // get argv[0].  It's in the_psinfo.pr_argv[0], but that's a ptr in the inferior
   // space, so we need to /proc read() it out.  Also, the_psinfo.pr_argv is a char **
   // not a char* so we even need to /proc read() to get a pointer value.  Ick.
   assert(the_psinfo.pr_argv != NULL);
   char *ptr_to_argv0 = extract_string_ptr(fd, the_psinfo.pr_argv);
   argv0 = extract_string(fd, ptr_to_argv0);

   // Get the PWD and PATH environment variables from the application.
   char **envptr = the_psinfo.pr_envp;
   while (true) {
      // dereference envptr; check for NULL
      char *env = extract_string_ptr(fd, envptr);
      if (env == NULL)
	 break;

      string env_value = extract_string(fd, env);
      if (env_value.prefixed_by("PWD=") || env_value.prefixed_by("CWD=")) {
	 cwdenv = env_value.c_str() + 4; // skip past "PWD=" or "CWD="
	 attach_cerr << "get_ps_stuff: using PWD value of: " << cwdenv << endl;
      }
      else if (env_value.prefixed_by("PATH=")) {
	 pathenv = env_value.c_str() + 5; // skip past the "PATH="
	 attach_cerr << "get_ps_stuff: using PATH value of: " << pathenv << endl;
      }

      envptr++;
   }

   return true;
}


/*
   Open the /proc file correspoding to process pid, 
   set the signals to be caught to be only SIGSTOP and SIGTRAP,
   and set the kill-on-last-close and inherit-on-fork flags.
*/
extern string pd_flavor ;
bool process::attach() {
  // QUESTION: does this attach operation lead to a SIGTRAP being forwarded
  // to paradynd in all cases?  How about when we are attaching to an
  // already-running process?  (Seems that in the latter case, no SIGTRAP
  // is automatically generated)

  // step 1) /proc open: attach to the inferior process
  // Blow away the existing default LWP handle (if one)

  dyn_lwp *lwp = new dyn_lwp(0, this);
  if (!lwp->openFD()) {
    delete lwp;
    return false;
  }
  lwps[0] = lwp;

  // step 2) /proc PIOCSTRACE: define which signals should be forwarded to daemon
  //   These are (1) SIGSTOP and (2) either SIGTRAP (sparc) or SIGILL (x86), to
  //   implement inferiorRPC completion detection.

  sigset_t sigs;

  premptyset(&sigs);
  praddset(&sigs, SIGSTOP);
  
#ifndef i386_unknown_solaris2_5
  praddset(&sigs, SIGTRAP);
#endif
  
#ifdef i386_unknown_solaris2_5
  praddset(&sigs, SIGILL);
#endif
  
  if (ioctl(lwp->get_fd(), PIOCSTRACE, &sigs) < 0) {
    fprintf(stderr, "attach: ioctl failed: %s\n", sys_errlist[errno]);
    return false;
  }

  // Step 3) /proc PIOCSET:
  // a) turn on the kill-on-last-close flag (kills inferior with SIGKILL when
  //    the last writable /proc fd closes)
  // b) turn on inherit-on-fork flag (tracing flags inherited when
  // child forks) in Paradyn only
  // c) turn on breakpoint trap pc adjustment (x86 only).
  // Also, any child of this process will stop at the exit of an exec call.

  

#ifdef BPATCH_LIBRARY
  setProcfsFlags();

// #endif
#else
  //Tempest, do not need to inherit-on-fork
  long flags ;

  if(process::pdFlavor == string("cow") || process::pdFlavor == string("mpi"))
  	flags = PR_KLC |  PR_BPTADJ;
  else
   	flags = PR_KLC | PR_FORK | PR_BPTADJ;
  if (ioctl (lwp->get_fd(), PIOCSET, &flags) < 0) {
    fprintf(stderr, "attach: PIOCSET failed: %s\n", sys_errlist[errno]);
    return false;
  }
#endif

  if (!get_ps_stuff(lwp->get_fd(), this->argv0, this->pathenv, this->cwdenv))
      return false;

  return true;
}

bool process::trapAtEntryPointOfMain()
{
  prgregset_t regs;
#ifdef PURE_BUILD
  // explicitly initialize "regs" struct (to pacify Purify)
  // (at least initialize those components which we actually use)
  for (unsigned r=0; r<(sizeof(regs)/sizeof(regs[0])); r++) regs[r]=0;
#endif

  if (ioctl (getDefaultLWP()->get_fd(), PIOCGREG, &regs) != -1) {
    // is the trap instr at main_brk_addr?
    if(regs[R_PC] == (int)main_brk_addr){ 
      return(true);
    }
  }
  return(false);
}

#if !defined(BPATCH_LIBRARY) //ccw 18 apr 2002 : SPLIT
bool process::trapDueToParadynLib(){
  if (paradynlib_brk_addr == 0) { // not set yet!
    return(false);
  }
  prgregset_t regs;
#ifdef PURE_BUILD
  // explicitly initialize "regs" struct (to pacify Purify)
  // (at least initialize those components which we actually use)
  for (unsigned r=0; r<(sizeof(regs)/sizeof(regs[0])); r++) regs[r]=0;
#endif
  
  if (ioctl (getDefaultLWP()->get_fd(), PIOCGREG, &regs) != -1) {
    // is the trap instr at dyninstlib_brk_addr?
    if(regs[R_PC] == (int)paradynlib_brk_addr){ 
      return(true);
    }
  }
  return false;
}
#endif


bool process::trapDueToDyninstLib()
{
  if (dyninstlib_brk_addr == 0) // not set yet!
      return(false);

  prgregset_t regs;
#ifdef PURE_BUILD
  // explicitly initialize "regs" struct (to pacify Purify)
  // (at least initialize those components which we actually use)
  for (unsigned r=0; r<(sizeof(regs)/sizeof(regs[0])); r++) regs[r]=0;
#endif

  if (ioctl (getDefaultLWP()->get_fd(), PIOCGREG, &regs) != -1) {
    // is the trap instr at dyninstlib_brk_addr?
    if(regs[R_PC] == (int)dyninstlib_brk_addr){ 
      return(true);
    }
  }
  return(false);
}

void process::handleIfDueToDyninstLib() 
{
  // rewrite original instructions in the text segment we use for 
  // the inferiorRPC - naim
  unsigned count = sizeof(savedCodeBuffer);
  //Address codeBase = getImage()->codeOffset();

  function_base *_startfn = this->findFuncByName("_start");
  Address codeBase = _startfn->getEffectiveAddress(this);
  assert(codeBase);
  writeDataSpace((void *)codeBase, count, (char *)savedCodeBuffer);

  // restore registers
  getDefaultLWP()->restoreRegisters(savedRegs); 
  delete[] (char*)savedRegs;
  savedRegs = NULL;
}

void process::handleTrapAtEntryPointOfMain()
{
  function_base *f_main = findOneFunction("main");
  assert(f_main);
  Address addr = f_main->addr();
  // restore original instruction 
#if defined(sparc_sun_solaris2_4)
  writeDataSpace((void *)addr, sizeof(instruction), (char *)savedCodeBuffer);
#else // x86
  writeDataSpace((void *)addr, 2, (char *)savedCodeBuffer);
#endif
}

void process::insertTrapAtEntryPointOfMain()
{

  function_base *f_main = findOneFunction("main");
  if (!f_main) {
    // we can't instrument main - naim
    showErrorCallback(108,"main() uninstrumentable");
    //extern void cleanUpAndExit(int);
    //cleanUpAndExit(-1); 
    return;
  }
  assert(f_main);
  Address addr = f_main->addr();

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
}

#if !defined(BPATCH_LIBRARY)
/* 	this function is used to dlopen the PARADYN runtime
	library
	ccw 18 apr 2002 : SPLIT
*/


bool process::dlopenPARADYNlib() {
  // we will write the following into a buffer and copy it into the
  // application process's address space
  // [....LIBRARY's NAME...|code for DLOPEN]

  // write to the application at codeOffset. This won't work if we
  // attach to a running process.
  //Address codeBase = this->getImage()->codeOffset();
  // ...let's try "_start" instead

  function_base *_startfn = this->findFuncByName("_start");
  Address codeBase = _startfn->getEffectiveAddress(this);
  assert(codeBase);
  // Or should this be readText... it seems like they are identical
  // the remaining stuff is thanks to Marcelo's ideas - this is what 
  // he does in NT. The major change here is that we use AST's to 
  // generate code for dlopen.

  // savedCodeBuffer[BYTES_TO_SAVE] is declared in process.h
  readDataSpace((void *)codeBase, sizeof(savedCodeBuffer), savedCodeBuffer, true);

  unsigned char scratchCodeBuffer[BYTES_TO_SAVE];
  vector<AstNode*> dlopenAstArgs(2);

  Address count = 0;

//ccw 18 apr 2002 : SPLIT
  dlopenAstArgs[0] = new AstNode(AstNode::Constant, (void*)0); 
  // library name. We use a scratch value first. We will update this parameter
  // later, once we determine the offset to find the string - naim
  dlopenAstArgs[1] = new AstNode(AstNode::Constant, (void*)DLOPEN_MODE); // mode
  AstNode *dlopenAst = new AstNode("dlopen",dlopenAstArgs);
  removeAst(dlopenAstArgs[0]);
  removeAst(dlopenAstArgs[1]);

  // deadList and deadListSize are defined in inst-sparc.C - naim
  extern Register deadList[];
  extern unsigned int deadListSize;
  registerSpace *dlopenRegSpace = new registerSpace(deadListSize/sizeof(Register),
                                deadList, (unsigned)0, NULL);
  dlopenRegSpace->resetSpace();

//ccw 18 apr 2002 : SPLIT
  dlopenAst->generateCode(this, dlopenRegSpace, (char *)scratchCodeBuffer,
			  count, true, true);
  writeDataSpace((void *)codeBase, count, (char *)scratchCodeBuffer);
// the following seems to be a redundant relic
//#if defined(sparc_sun_solaris2_4)
//  count += sizeof(instruction);
//#endif

  // we need to make 2 calls to dlopen: one to load libsocket.so.1 and another
  // one to load libdyninst.so.1 - naim

//ccw 18 apr 2002 : SPLIT
  removeAst(dlopenAst); // to avoid memory leaks - naim
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
  paradynlib_brk_addr = codeBase + count;
  count += sizeof(instruction);
#else //x86
  writeDataSpace((void *)(codeBase + count), 2, insnTrap.ptr());
  paradynlib_brk_addr = codeBase + count;
  count += 2;
#endif

//ccw 18 apr 2002 : SPLIT
#ifdef MT_THREAD
  const char DyninstEnvVar[]="PARADYN_LIB_MT";
#else
  const char DyninstEnvVar[]="PARADYN_LIB";
#endif /*MT_THREAD*/

#if 0 //ccw 22 apr 2002 : SPLIT
  if (dyninstName.length()) {
    // use the library name specified on the start-up command-line
  } else {
#endif

    // check the environment variable
    if (getenv(DyninstEnvVar) != NULL) {
      dyninstName = getenv(DyninstEnvVar);
    } else {
      string msg = string("Environment variable " + string(DyninstEnvVar)
			  + " has not been defined for process ") + string(pid);
      showErrorCallback(101, msg);
      return false;
    }
#if 0  //ccw 22 apr 2002 : SPLIT
  }
#endif

  if (access(dyninstName.c_str(), R_OK)) {
    string msg = string("Runtime library ") + dyninstName
        + string(" does not exist or cannot be accessed!");
    showErrorCallback(101, msg);
    return false;
  }

  Address dyninstlib_addr = codeBase + count;

  writeDataSpace((void *)(codeBase + count), dyninstName.length()+1,
		 (caddr_t)const_cast<char*>(dyninstName.c_str()));
  count += dyninstName.length()+1;
  // we have now written the name of the library after the trap - naim

//ccw 18 apr 2002 : SPLIT
  char socketname[256];
  if (getenv("PARADYN_SOCKET_LIB") != NULL) {
    strcpy((char*)socketname,(char*)getenv("PARADYN_SOCKET_LIB"));
  } else {
    strcpy((char*)socketname,"/usr/lib/libsocket.so.1");
  }
  Address socketlib_addr = codeBase + count;
  writeDataSpace((void *)(codeBase + count), 
		 strlen(socketname)+1, (caddr_t)socketname);
  count += strlen(socketname)+1;
  // we have now written the name of the socket library after the trap - naim
  //
  assert(count<=BYTES_TO_SAVE);

//ccw 18 apr 2002 : SPLIT
  // at this time, we know the offset for the library name, so we fix the
  // call to dlopen and we just write the code again! This is probably not
  // very elegant, but it is easy and it works - naim
  // loading the socket library - naim
  removeAst(dlopenAst); // to avoid leaking memory
  dlopenAstArgs[0] = new AstNode(AstNode::Constant, (void *)(socketlib_addr));
  dlopenAstArgs[1] = new AstNode(AstNode::Constant, (void*)DLOPEN_MODE);
  dlopenAst = new AstNode("dlopen",dlopenAstArgs);
  removeAst(dlopenAstArgs[0]);
  removeAst(dlopenAstArgs[1]);
  count = 0; // reset count
  dlopenAst->generateCode(this, dlopenRegSpace, (char *)scratchCodeBuffer,
			  count, true, true);
  writeDataSpace((void *)codeBase, count, (char *)scratchCodeBuffer);
  removeAst(dlopenAst);

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
  writeDataSpace((void *)(codeBase+count), dyninst_count, (char *)scratchCodeBuffer);
  removeAst(dlopenAst);
  count += dyninst_count;

  // save registers
  savedRegs = getDefaultLWP()->getRegisters();
  assert((savedRegs!=NULL) && (savedRegs!=(void *)-1));

#if defined(i386_unknown_solaris2_5)
  /* Setup a new stack frame large enough for arguments to functions
     called during bootstrap, as generated by the FuncCall AST.  */
  prgregset_t regs; regs = *(prgregset_t*)savedRegs;
  Address theESP = regs[SP_REG];
  if (!changeIntReg(FP_REG, theESP)) {
    logLine("WARNING: changeIntReg failed in dlopenPARADYNlib\n");
    assert(0);
  }
  if (!changeIntReg(SP_REG, theESP-32)) {
    logLine("WARNING: changeIntReg failed in dlopenPARADYNlib\n");
    assert(0);
  }
#endif
  isLoadingParadynLib = true;
  if (!getDefaultLWP()->changePC(codeBase, NULL)) {
    logLine("WARNING: changePC failed in dlopenPARADYNlib\n");
    assert(0);
  }

  return true;
}


#endif
/* 	the following function has been edited to remove
	the ability to load the PARADYN runtime library	
	ccw 18 apr 2002 : SPLIT
*/

bool process::dlopenDYNINSTlib() {
  // we will write the following into a buffer and copy it into the
  // application process's address space
  // [....LIBRARY's NAME...|code for DLOPEN]

  // write to the application at codeOffset. This won't work if we
  // attach to a running process.
  //Address codeBase = this->getImage()->codeOffset();
  // ...let's try "_start" instead
  function_base *_startfn = this->findFuncByName("_start");
  Address codeBase = _startfn->getEffectiveAddress(this);
  assert(codeBase);

  // Or should this be readText... it seems like they are identical
  // the remaining stuff is thanks to Marcelo's ideas - this is what 
  // he does in NT. The major change here is that we use AST's to 
  // generate code for dlopen.

  // savedCodeBuffer[BYTES_TO_SAVE] is declared in process.h
  readDataSpace((void *)codeBase, sizeof(savedCodeBuffer), savedCodeBuffer, true);

  unsigned char scratchCodeBuffer[BYTES_TO_SAVE];
  vector<AstNode*> dlopenAstArgs(2);

  Address count = 0;

//ccw 18 apr 2002 : SPLIT
  AstNode *dlopenAst;

  // deadList and deadListSize are defined in inst-sparc.C - naim
  extern Register deadList[];
  extern unsigned int deadListSize;
  registerSpace *dlopenRegSpace = new registerSpace(deadListSize/sizeof(Register),
                                deadList, (unsigned)0, NULL);
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
  if (access(dyninstName.c_str(), R_OK)) {
    string msg = string("Runtime library ") + dyninstName
        + string(" does not exist or cannot be accessed!");
    showErrorCallback(101, msg);
    return false;
  }

  Address dyninstlib_addr = codeBase + count;

  writeDataSpace((void *)(codeBase + count), dyninstName.length()+1,
		 (caddr_t)const_cast<char*>(dyninstName.c_str()));
  count += dyninstName.length()+1;
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
  writeDataSpace((void *)(codeBase+count), dyninst_count, (char *)scratchCodeBuffer);
  removeAst(dlopenAst);
  count += dyninst_count;

  // save registers
  savedRegs = getDefaultLWP()->getRegisters();
  assert((savedRegs!=NULL) && (savedRegs!=(void *)-1));

#if defined(i386_unknown_solaris2_5)
  /* Setup a new stack frame large enough for arguments to functions
     called during bootstrap, as generated by the FuncCall AST.  */
  prgregset_t regs; regs = *(prgregset_t*)savedRegs;
  Address theESP = regs[SP_REG];
  if (!changeIntReg(FP_REG, theESP)) {
    logLine("WARNING: changeIntReg failed in dlopenDYNINSTlib\n");
    assert(0);
  }
  if (!changeIntReg(SP_REG, theESP-32)) {
    logLine("WARNING: changeIntReg failed in dlopenDYNINSTlib\n");
    assert(0);
  }
#endif
  isLoadingDyninstLib = true;
  if (!getDefaultLWP()->changePC(codeBase, NULL)) {
    logLine("WARNING: changePC failed in dlopenDYNINSTlib\n");
    assert(0);
  }

  return true;
}

Address process::get_dlopen_addr() const {
  if (dyn != NULL)
    return(dyn->get_dlopen_addr());
  else 
    return(0);
}

bool process::isRunning_() const {
   // determine if a process is running by doing low-level system checks, as
   // opposed to checking the 'status_' member vrble.  May assume that attach()
   // has run, but can't assume anything else.
   prstatus theStatus;
#ifdef PURE_BUILD
   // explicitly initialize "theStatus" struct (to pacify Purify)
   memset(&theStatus, '\0', sizeof(prstatus));
#endif
   if (ioctl(getDefaultLWP()->get_fd(), PIOCSTATUS, &theStatus) == -1) {
      perror("process::isRunning_()");
      assert(false);
   }

   if (theStatus.pr_flags & PR_STOPPED)
      return false;
   else
      return true;
}

bool process::attach_() {assert(false); return(false);}
bool process::stop_() {assert(false); return(false);}

/* 
   continue a process that is stopped 
*/

bool dyn_lwp::continueLWP() {
  prrun_t flags;
  prstatus_t stat;
  Address pc;  // PC at which we are trying to continue

#ifdef PURE_BUILD
  // explicitly initialize "flags" struct (to pacify Purify)
  memset(&flags, '\0', sizeof(flags));
  // explicitly initialize "stat" struct (to pacify Purify)
  // (at least initialize those components which we actually use)
  stat.pr_flags = 0x0000;
  stat.pr_why   = 0;
  stat.pr_what  = 0;
#endif

  // a process that receives a stop signal stops twice. We need to run the process
  // and wait for the second stop. (The first run simply absorbs the stop signal;
  // the second one does the actual continue.)
  if (ioctl(fd_, PIOCSTATUS, &stat) == -1) return false;

  if ((0==stat.pr_flags & PR_STOPPED) && (0==stat.pr_flags & PR_ISTOP)) {
      fprintf(stderr, "LWP not stopped, returning false\n");
      return false;
  }
  
  if ((stat.pr_flags & PR_STOPPED)
      && (stat.pr_why == PR_SIGNALLED)
      && (stat.pr_what == SIGSTOP || stat.pr_what == SIGINT)) {
      flags.pr_flags = PRSTOP;
      if (ioctl(fd_, PIOCRUN, &flags) == -1) {
          fprintf(stderr, "continueProc_: PIOCRUN failed: %s\n", sys_errlist[errno]);
          return false;
      }
      if (ioctl(fd_, PIOCWSTOP, 0) == -1) {
          fprintf(stderr, "continueProc_: PIOCWSTOP failed: %s\n", sys_errlist[errno]);
          return false;
      }
  }
  flags.pr_flags = PRCSIG; // clear current signal
  pc = (Address)stat.pr_reg[PC_REG];
  
  if (! (stoppedInSyscall_ && pc == postsyscallpc_)) {
       // Continue the process
      if (ioctl(fd_, PIOCRUN, &flags) == -1) {
          sprintf(errorLine,
                  "continueProc_: PIOCRUN 2 failed: %s\n",
                  sys_errlist[errno]);
          logLine(errorLine);
          return false;
      }
  } else {
      // We interrupted a sleeping system call at some previous pause
      // (i.e. stoppedInSyscall is true), we have not restarted that
      // system call yet, and the current PC is the insn following
      // the interrupted call.  It is time to restart the system
      // call.
      
      // Note that when we make the process runnable, we ignore
      // `flags', set if `hasNewPC' was true in the previous block,
      // because we don't want its PC; we want the PC of the system
      // call trap, which was saved in `syscallreg'.
      
      sysset_t scentry, scsavedentry;
      prrun_t run;
      
      // Restore the registers
      if (0 > ioctl(fd_, PIOCSREG, syscallreg_)) {
          sprintf(errorLine,
		   "Can't restart sleeping syscall (PIOCSREG)\n");
          logLine(errorLine);
          return false;
      }
#if 0      
       // Save current syscall entry traps
       if (0 > ioctl(getDefaultLWP()->get_fd(), PIOCGENTRY, &scsavedentry)) {
	   sprintf(errorLine,
		   "warn: Can't restart sleeping syscall (PIOCGENTRY)\n");
	   logLine(errorLine);
	   return false;
       }
       
       // Set the process to trap on entry to previously stopped syscall
       premptyset(&scentry);
       praddset(&scentry, stoppedSyscall);
       if (0 > ioctl(getDefaultLWP()->get_fd(), PIOCSENTRY, &scentry)) {
	   sprintf(errorLine,
		   "warn: Can't restart sleeping syscall (PIOCSENTRY)\n");
	   logLine(errorLine);
	   return false;
       }
       
       // Continue the process
       run.pr_flags = PRCSIG; // Clear current signal
       if (0 > ioctl(getDefaultLWP()->get_fd(), PIOCRUN, &run)) {
	   sprintf(errorLine,
		   "warn: Can't restart sleeping syscall (PIOCRUN)\n");
	   logLine(errorLine);
	   return false;
       }
       
       // Wait for the process to stop
       if (0 > ioctl(getDefaultLWP()->get_fd(), PIOCWSTOP, &stat)) {
	   sprintf(errorLine,
		   "warn: Can't restart sleeping syscall (PIOCWSTOP)\n");
	   logLine(errorLine);
	   return false;
       }
       
       // Verify we've stopped at the entry of the call we're trying
       // to restart
       if (stat.pr_why != PR_SYSENTRY
 	   || stat.pr_what != stoppedSyscall) {
	   sprintf(errorLine,
		   "warn: Can't restart sleeping syscall (verify)\n");
	   logLine(errorLine);
	   return false;
       }
       
       // Restore the syscall entry traps
       if (0 > ioctl(getDefaultLWP()->get_fd(), PIOCSENTRY, &scsavedentry)) {
	   sprintf(errorLine,
		   "warn: Can't restart sleeping syscall (PIOCSENTRY)\n");
	   logLine(errorLine);
	   return false;
       }
       
#if 0
       // Restore the registers again.
       // Sun told us to do this, but we don't know why.  On the
       // SPARC, it doesn't matter -- the system call can be restarted
       // whether or not we do this.  On the x86, the restart FAILS if
       // we do this.  So Sun can go sit and spin for all I care.
       if (0 > ioctl(getDefaultLWP()->get_fd(), PIOCSREG, syscallreg_)) {
	   sprintf(errorLine,
		   "Can't restart sleeping syscall (PIOCSREG)\n");
	   logLine(errorLine);
	   return false;
       }
#endif
#endif 
       // We are done -- the process is in the kernel for the system
       // call, with the right registers values.  Make the process
       // runnable, restoring its previously blocked signals.
       stoppedInSyscall_ = false;
       run.pr_sighold = sighold_;
       run.pr_flags = PRSHOLD;
       if (0 > ioctl(fd_, PIOCRUN, &run)) {
           sprintf(errorLine,
                   "Can't restart sleeping syscall (PIOCRUN)\n");
           logLine(errorLine);
           return false;
       }
  }
  return true;
}


bool process::continueProc_() {
  ptraceOps++; ptraceOtherOps++;
  return getDefaultLWP()->continueLWP();
#if 0
  // This doesn't work -- dyninst/ST paradyn are okay,
  // since they are equivalent. But MT breaks.
  if (threads.size() == 0) {
      return getDefaultLWP()->continueLWP();
  }
  else {
      bool success = true;
      for (unsigned i = 0; i < threads.size(); i++) {
          dyn_lwp *lwp = threads[i]->get_lwp();
          if (!lwp) continue;
          if (!lwp->continueLWP())
              success = false;
      }
      
      return success;
  }
  return true;
#endif
}

#ifdef BPATCH_LIBRARY
/*
   terminate execution of a process
 */
bool process::terminateProc_()
{
    long flags = PR_KLC;
    if (ioctl (getDefaultLWP()->get_fd(), PIOCSET, &flags) < 0)
	return false;

    Exited();

    return true;
}
#endif

fileDescriptor *getExecFileDescriptor(string filename,
				     int &,
				     bool)
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

// We stopped a process that it is a system call.  We abort the
// system call, so that the process can execute an inferior RPC.  We
// save the process state as it was at the ENTRY of the system call,
// so that the system call can be restarted when we continue the
// process.  Note: this code does not deal with multiple LWPs.

int dyn_lwp::abortSyscall()
{
  prrun_t run;
  sysset_t scexit, scsavedexit;
  sysset_t scentry, scsavedentry;
  prstatus_t prstatus;

  // MT: aborting syscalls does not work. Maybe someone with better knowledge
  // of Solaris can get it working. 
  if (proc_->multithread_capable())
      return 0;
  
  // We do not expect to recursively interrupt system calls.  We could
  // probably handle it by keeping a stack of system call state.  But
  // we haven't yet seen any reason to have this functionality.
  assert(!stoppedInSyscall_);
  stoppedInSyscall_ = true;

  if (ioctl(fd_, PIOCSTATUS, &prstatus) == -1) {
    sprintf(errorLine,
	    "warn : process::pause_ use ioctl to send PICOSTOP returns error : errno = %i\n", errno);
    perror("warn : process::abortSyscall ioctl PICOSTOP: ");
    logLine(errorLine);
  }
  
  // 1. Save the syscall number, registers, and blocked signals
  stoppedSyscall_ = prstatus.pr_syscall;
  memcpy(syscallreg_, prstatus.pr_reg, sizeof(prstatus.pr_reg));
  sighold_ = prstatus.pr_sighold;
#ifdef i386_unknown_solaris2_5
  // From Roger A. Faulkner at Sun (email unknown), 6/29/1997:
  //
  // On Intel and PowerPC machines, the system call trap instruction
  // leaves the PC (program counter, instruction pointer) referring to
  // the instruction that follows the system call trap instruction.
  // On Sparc machines, the system call trap instruction leaves %pc
  // referring to the system call trap instruction itself (the
  // operating system increments %pc on exit from the system call).
  //
  // We have to reset the PC back to the system call trap instruction
  // on Intel and PowerPC machines.
  //
  // This is 7 on Intel, 4 on PowerPC.

  // Note: On x86/Linux this should probably be 2 bytes, because Linux
  // uses "int" to trap, not lcall.

  syscallreg_[PC_REG] -= 7;
#endif

  // 2. Abort the system call

  // Save current syscall exit traps
  if (0 > ioctl(fd_, PIOCGEXIT, &scsavedexit)) {
       sprintf(errorLine,
	       "warn: Can't get status (PIOCGEXIT) of paused process\n");
       logLine(errorLine);
       return 0;
  }

  if (0 > ioctl(fd_, PIOCGENTRY, &scsavedentry)) {
       sprintf(errorLine,
	       "warn: Can't get status (PIOCGENTRY) of paused process\n");
       logLine(errorLine);
       return 0;
  }

  // Set process to trap on exit from this system call
  premptyset(&scexit);
  praddset(&scexit, stoppedSyscall_);
  if (0 > ioctl(fd_, PIOCSEXIT, &scexit)) {
       sprintf(errorLine,
	       "warn: Can't step paused process out of syscall (PIOCSEXIT)\n");
       logLine(errorLine);
       return 0;
  }

  premptyset(&scentry);
  if (0 > ioctl(fd_, PIOCSENTRY, &scentry)) {
       sprintf(errorLine,
	       "warn: Can't step paused process out of syscall (PIOCSENTRY)\n");
       logLine(errorLine);
       return 0;
  }

  // Continue, aborting this system call and blocking all sigs except
  // those needed by DynInst.
  sigfillset(&run.pr_sighold);
  sigdelset(&run.pr_sighold, SIGTRAP);
  sigdelset(&run.pr_sighold, SIGILL);
  run.pr_flags = PRSABORT|PRSHOLD;
  if (0 > ioctl(fd_, PIOCRUN, &run)) {
       sprintf(errorLine,
	       "warn: Can't step paused process out of syscall (PIOCRUN)\n");
       logLine(errorLine);
       return 0;
  }


  // Wait for it to stop (at exit of aborted syscall)
  if (0 > ioctl(fd_, PIOCWSTOP, &prstatus)) {
       sprintf(errorLine,
	       "warn: Can't step paused process out of syscall (PIOCWSTOP)\n");
       logLine(errorLine);
       return 0;
  }

  // Note: We assume that is always safe to restart the call after
  // aborting it.  We are wrong if it turns out that some
  // interruptible system call can make partial progress before we
  // abort it.
  // We think this is impossible because the proc manpage says EINTR
  // would be returned to the process if we didn't trap the syscall
  // exit, and the manpages for interruptible system calls say an
  // EINTR return value means no progress was made.
  // If we are wrong, this is probably the place to decide whether
  // and/or how the syscall should be restarted later.

  // Verify that we're stopped in the right place
  if (((prstatus.pr_flags & (PR_STOPPED|PR_ISTOP))
       != (PR_STOPPED|PR_ISTOP))
      || prstatus.pr_why != PR_SYSEXIT
      || prstatus.pr_syscall != stoppedSyscall_) {
       sprintf(errorLine,
	       "warn: Can't step paused process out of syscall (Verify)\n");
       logLine(errorLine);
       return 0;
  }

  // Reset the syscall exit traps
  if (0 > ioctl(fd_, PIOCSEXIT, &scsavedexit)) {
       sprintf(errorLine,
	       "warn: Can't step paused process out of syscall (PIOCSEXIT)\n");
       logLine(errorLine);
       return 0;
  }

  // Reset the syscall entry traps
  if (0 > ioctl(fd_, PIOCSENTRY, &scsavedentry)) {
       sprintf(errorLine,
	       "warn: Can't step paused process out of syscall (PIOCSENTRY)\n");
       logLine(errorLine);
       return 0;
  }

  // Remember the current PC.  When we continue the process at this PC
  // we will restart the system call.
  postsyscallpc_ = (Address) prstatus.pr_reg[PC_REG];

  return 1;
}

/*
   pause a process that is running
*/

bool dyn_lwp::pauseLWP() {
  int ioctl_ret;
  prstatus_t prstatus;
  
  // /proc PIOCSTOP: direct all LWPs to stop, _and_ wait for them to stop.
  ioctl_ret = ioctl(fd_, PIOCSTOP, &prstatus);
  if (ioctl_ret == -1) {
      // see if the reason it failed is that we were already stopped
      if (ioctl(fd_, PIOCSTATUS, &prstatus) == -1) {
	  sprintf(errorLine,
		  "warn : process::pause_ use ioctl to send PICOSTOP returns error : errno = %i\n", errno);
	  perror("warn : process::pause_ ioctl PICOSTOP: ");
	  logLine(errorLine);
	  return 0;
      } else if (!(prstatus.pr_flags & PR_STOPPED)) {
	  return 0;
      }
  }

  // Determine if the process was in a system call when we stopped it.
  if (! (prstatus.pr_why == PR_REQUESTED
	 && prstatus.pr_syscall != 0
	 && (prstatus.pr_flags & PR_ASLEEP))) {
       // The process was not in a system call.  We're done.
       if (prstatus.pr_why != PR_SYSENTRY) 
	   return true;
  }
  
  return abortSyscall();
}


bool process::pause_() {
  ptraceOps++; ptraceOtherOps++;
  return getDefaultLWP()->pauseLWP();
#if 0
  // This code doesn't work. I'm leaving it here as an example -- bernat
  if (threads.size() == 0) {
      return getDefaultLWP()->pauseLWP();
  }
  else {
      bool success = true;
      for (unsigned i = 0; i < threads.size(); i++) {
          dyn_lwp *lwp = threads[i]->get_lwp();
          if (!lwp) continue;
          if (!lwp->pauseLWP())
              success = false;
      }
      
      return success;
  }
  return true;
#endif
}

/*
   close the file descriptor for the file associated with a process
*/
bool process::detach_() {
  dyn_lwp *lwp = getDefaultLWP();
  if (!lwp)
    return false;
  lwp->closeFD();
  delete lwp;
  return true;
}

#ifdef BPATCH_LIBRARY
/*
   detach from thr process, continuing its execution if the parameter "cont"
   is true.
 */
bool process::API_detach_(const bool cont)
{
  // Remove the breakpoint that we put in to detect loading and unloading of
  // shared libraries.
  // XXX We might want to move this into some general cleanup routine for the
  //     dynamic_linking object.
  if (dyn) {
      dyn->unset_r_brk_point(this);
  }

  // Reset the kill-on-close flag, and the run-on-last-close flag if necessary
  long flags = PR_KLC;
  if (!cont) flags |= PR_RLC;
  if (ioctl (getDefaultLWP()->get_fd(), PIOCRESET, &flags) < 0) {
    fprintf(stderr, "detach: PIOCRESET failed: %s\n", sys_errlist[errno]);
    close(getDefaultLWP()->get_fd());
    return false;
  }
  // Set the run-on-last-close-flag if necessary
  if (cont) {
    flags = PR_RLC;
    if (ioctl (getDefaultLWP()->get_fd(), PIOCSET, &flags) < 0) {
      fprintf(stderr, "detach: PIOCSET failed: %s\n", sys_errlist[errno]);
      close(getDefaultLWP()->get_fd());
      return false;
    }
  }

  sigset_t sigs;
  premptyset(&sigs);
  if (ioctl(getDefaultLWP()->get_fd(), PIOCSTRACE, &sigs) < 0) {
    fprintf(stderr, "detach: PIOCSTRACE failed: %s\n", sys_errlist[errno]);
    close(getDefaultLWP()->get_fd());
    return false;
  }
  if (ioctl(getDefaultLWP()->get_fd(), PIOCSHOLD, &sigs) < 0) {
    fprintf(stderr, "detach: PIOCSHOLD failed: %s\n", sys_errlist[errno]);
    close(getDefaultLWP()->get_fd());
    return false;
  }

  fltset_t faults;
  premptyset(&faults);
  if (ioctl(getDefaultLWP()->get_fd(), PIOCSFAULT, &faults) < 0) {
    fprintf(stderr, "detach: PIOCSFAULT failed: %s\n", sys_errlist[errno]);
    close(getDefaultLWP()->get_fd());
    return false;
  }
  
  sysset_t syscalls;
  premptyset(&syscalls);
  if (ioctl(getDefaultLWP()->get_fd(), PIOCSENTRY, &syscalls) < 0) {
    fprintf(stderr, "detach: PIOCSENTRY failed: %s\n", sys_errlist[errno]);
    close(getDefaultLWP()->get_fd());
    return false;
  }
  if (ioctl(getDefaultLWP()->get_fd(), PIOCSEXIT, &syscalls) < 0) {
    fprintf(stderr, "detach: PIOCSEXIT failed: %s\n", sys_errlist[errno]);
    close(getDefaultLWP()->get_fd());
    return false;
  }

  close(getDefaultLWP()->get_fd());
  return true;
}
#endif

bool process::dumpCore_(const string coreName) 
{
  char command[100];

  sprintf(command, "gcore %d 2> /dev/null; mv core.%d %s", getPid(), getPid(), 
	coreName.c_str());

  detach_();
  system(command);
  attach();

  return false;
}

bool process::writeTextWord_(caddr_t inTraced, int data) {
//  cerr << "writeTextWord @ " << (void *)inTraced << endl; cerr.flush();
  return writeDataSpace_(inTraced, sizeof(int), (caddr_t) &data);
}

bool process::writeTextSpace_(void *inTraced, u_int amount, const void *inSelf) {
//  cerr << "writeTextSpace pid=" << getPid() << ", @ " << (void *)inTraced << " len=" << amount << endl; cerr.flush();
  return writeDataSpace_(inTraced, amount, inSelf);
}

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
bool process::readTextSpace_(void *inTraced, u_int amount, const void *inSelf) {
  return readDataSpace_(inTraced, amount, const_cast<void *>(inSelf));
}
#endif

bool process::writeDataSpace_(void *inTraced, u_int amount, const void *inSelf) {
  ptraceOps++; ptraceBytes += amount;

//  cerr << "process::writeDataSpace_ pid " << getPid() << " writing " << amount << " bytes at loc " << inTraced << endl;

#if defined(BPATCH_LIBRARY)
#if defined (sparc_sun_solaris2_4)
	if(collectSaveWorldData &&  ((Address) inTraced) > getDyn()->getlowestSObaseaddr() ){
		shared_object *sh_obj;
		bool result = false;
		for(int i = 0; shared_objects && !result && i<shared_objects->size();i++){
			sh_obj = (*shared_objects)[i];
			result = sh_obj->isinText((Address) inTraced);
		}
		if( result  ){
		/*	printf(" write at %lx in %s amount %x insn: %x \n", 
				(off_t)inTraced, sh_obj->getName().c_str(), amount,
				 *(unsigned int*) inSelf);
		*/	
			sh_obj->setDirty();	
		}
	}
#endif
#endif

  if (lseek(getDefaultLWP()->get_fd(), (off_t)inTraced, SEEK_SET) != (off_t)inTraced) {
    perror("lseek");
    return false;
  }
  int written = write(getDefaultLWP()->get_fd(), inSelf, amount);
  if ((unsigned) written != amount) {
      printf("write returned %d\n", written);
      printf("attempt to write at %lx\n", (off_t)inTraced);
      perror("write");
      return false;
  } else {
      return true;
  }
}

bool process::readDataSpace_(const void *inTraced, u_int amount, void *inSelf) {
  ptraceOps++; ptraceBytes += amount;
  if((lseek(getDefaultLWP()->get_fd(), (off_t)inTraced, SEEK_SET)) != (off_t)inTraced) {
    fprintf(stderr,"error in lseek addr = 0x%lx amount = %d\n",
                (Address)inTraced,amount);
    return false;
  }
  if (!read(getDefaultLWP()->get_fd(), inSelf, amount) == (int)amount) {
    perror("Error in read");
    return false;
  }
  return true;
}

bool process::loopUntilStopped() {
  assert(0);
  return(false);
}

#ifdef notdef
// TODO -- only call getrusage once per round
static struct rusage *get_usage_data() {
  return NULL;
}
#endif

#ifndef BPATCH_LIBRARY

float OS::compute_rusage_cpu() {
  return 0;
}

float OS::compute_rusage_sys() {
  return 0;
}

float OS::compute_rusage_min() {
  return 0;
}
float OS::compute_rusage_maj() {
  return 0;
}

float OS::compute_rusage_swap() {
  return 0;
}
float OS::compute_rusage_io_in() {
  return 0;
}
float OS::compute_rusage_io_out() {
  return 0;
}
float OS::compute_rusage_msg_send() {
  return 0;
}
float OS::compute_rusage_msg_recv() {
  return 0;
}
float OS::compute_rusage_sigs() {
  return 0;
}
float OS::compute_rusage_vol_cs() {
  return 0;
}
float OS::compute_rusage_inv_cs() {
  return 0;
}

#endif

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

// getActiveFrame(): populate Frame object using toplevel frame
Frame dyn_lwp::getActiveFrame()
{
  Address fp=0, pc=0;
  prgregset_t regs ;

  if (-1 != ioctl(fd_, PIOCGREG, &regs)) {
    fp = regs[FP_REG];
    pc = regs[PC_REG];
  }
  else {
    return Frame();
  }
  return Frame(pc, fp, proc_->getPid(), NULL, this, true);
}

#if 0
// It is the caller's responsibility to do a "delete [] (*IDs)"
bool process::getLWPIDs(vector <unsigned> &LWPids) {
   int nlwp;
   prstatus_t the_psinfo;
   if (-1 !=  ioctl(getDefaultLWP()->get_fd(), PIOCSTATUS, &the_psinfo)) {
     nlwp =  the_psinfo.pr_nlwp ;
     id_t *id_p = new id_t [nlwp+1] ;
     if (-1 != ioctl(getDefaultLWP()->get_fd(), PIOCLWPIDS, id_p)) {
       unsigned i=0; 
       do { 
	 LWPids.push_back(id_p[i]);
       } while (id_p[i++]);
       delete [] id_p;
       return true ;
     }
   }
   return false ;
}

#endif

Frame Frame::getCallerFrame(process *p) const
{
  prgregset_t regs;
  Frame ret;
  ret.lwp_ = lwp_;
  ret.thread_ = thread_;

#ifdef PURE_BUILD
  // explicitly initialize "regs" struct (to pacify Purify)
  // (at least initialize those components which we actually use)
  for (unsigned r=0; r<(sizeof(regs)/sizeof(regs[0])); r++) regs[r]=0;
#endif

  if (uppermost_) {
    function_base *func = p->findFuncByAddr(pc_);
    if (func) {
      if (func->hasNoStackFrame()) { // formerly "isLeafFunc()"
	if (lwp_) { // We have a LWP and are prepared to use it
  	  if (ioctl(lwp_->get_fd(), PIOCGREG, &regs) != -1) {
	    ret.pc_ = regs[R_O7] + 8;
	    ret.fp_ = fp_; // frame pointer unchanged
	    return ret;
	  }
	}
        else if (thread_)
	  cerr << "Not implemented yet" << endl;
	else {
	  if (ioctl(p->getDefaultLWP()->get_fd(), PIOCGREG, &regs) != -1) {
	    Frame ret;
	    ret.pc_ = regs[R_O7] + 8;
	    ret.fp_ = fp_; // frame pointer unchanged
	    return ret;
	  }    
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
  
  if (p->readDataSpace((caddr_t)(fp_ + 56), 2*sizeof(int),
		       (caddr_t)&addrs, true))
  {
    Frame ret;
    ret.fp_ = addrs.fp;
    ret.pc_ = addrs.rtn + 8;
    return ret;
  }
  
  return Frame(); // zero frame
}


#ifdef SHM_SAMPLING
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
  // We use the PIOCUSAGE /proc ioctl
  
  // Other /proc ioctls that should work too: PIOCPSINFO
  // and the lower-level PIOCGETPR and PIOCGETU which return copies of the proc
  // and u areas, respectively.
  // PIOCSTATUS does _not_ work because its results are not in sync
  // with DYNINSTgetCPUtime
  
  rawTime64 result;
  prusage_t theUsage;

#ifdef PURE_BUILD
  // explicitly initialize "theUsage" struct (to pacify Purify)
  memset(&theUsage, '\0', sizeof(prusage_t));
#endif

  // compute the CPU timer for the whole process
  if (ioctl(fd_, PIOCUSAGE, &theUsage) == -1) {
    perror("could not read CPU time of inferior PIOCUSAGE");
    return 0;
  }
  result =  theUsage.pr_utime.tv_sec * 1000000000LL;
  result += theUsage.pr_utime.tv_nsec;

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


bool process::catchupSideEffect(Frame &frame, instReqNode *inst)
{
  return true;
}
#endif // SHM_SAMPLING

void *dyn_lwp::getRegisters()
{
  // Astonishingly, this routine can be shared between solaris/sparc and
  // solaris/x86.  All hail /proc!!!
  
  // assumes the process is stopped (/proc requires it)
  
  prgregset_t theIntRegs;
#ifdef PURE_BUILD
  // explicitly initialize "theIntRegs" struct (to pacify Purify)
  // (at least initialize those components which we actually use)
  for (unsigned r=0; r<(sizeof(theIntRegs)/sizeof(theIntRegs[0])); r++) theIntRegs[r]=0;
#endif
  
  if (ioctl(fd_, PIOCGREG, &theIntRegs) == -1) {
    perror("dyn_lwp::getRegisters PIOCGREG");
    if (errno == EBUSY) {
      cerr << "It appears that the process was not stopped in the eyes of /proc" << endl;
      assert(false);
    }
    
    return NULL;
  }
  
  prfpregset_t theFpRegs;
  
  if (ioctl(fd_, PIOCGFPREG, &theFpRegs) == -1) {
    perror("dyn_lwp::getRegisters PIOCGFPREG");
    if (errno == EBUSY)
      cerr << "It appears that the process was not stopped in the eyes of /proc" << endl;
    else if (errno == EINVAL)
      // what to do in this case?  Probably shouldn't even do a print, right?
      // And it certainly shouldn't be an error, right?
      // But I wonder if any sparcs out there really don't have floating point.
      cerr << "It appears that this machine doesn't have floating-point instructions" << endl;
    
    return NULL;
  }

  const int numbytesPart1 = sizeof(prgregset_t);
  const int numbytesPart2 = sizeof(prfpregset_t);
  assert(numbytesPart1 % 4 == 0);
  assert(numbytesPart2 % 4 == 0);
  void *buffer = new char[numbytesPart1 + numbytesPart2];
  assert(buffer);
  
  memcpy(buffer, &theIntRegs, sizeof(theIntRegs));
  memcpy((char *)buffer + sizeof(theIntRegs), &theFpRegs, sizeof(theFpRegs));
  
  return buffer;
}

bool dyn_lwp::executingSystemCall()
{
  prstatus theStatus;
#ifdef PURE_BUILD
  // explicitly initialize "theStatus" struct (to pacify Purify)
  // (at least initialize those components which we actually use)
  theStatus.pr_syscall = 0;
#endif
  if (ioctl(fd_, PIOCSTATUS, &theStatus) != -1) {
    // PR_SYSEXIT is set when we have aborted a system call but not
    // yet continued the process.  The process in that case is not
    // executing a system call.  (pr_syscall is the aborted call.)
    if (theStatus.pr_syscall > 0 && theStatus.pr_why != PR_SYSEXIT) {
      if ((theStatus.pr_syscall == SYS_exit) && (theStatus.pr_why == PR_SYSENTRY)) {
	// entry to exit is a special case - jkh 3/16/00
	stoppedSyscall_ = SYS_exit;
	abortSyscall();
	inferiorrpc_cerr << "at entry to exit" << endl;
	return(false);
      }
      inferiorrpc_cerr << "pr_syscall=" << theStatus.pr_syscall << endl;
      return(true);
    }
  } else assert(0);
  return(false);
}

bool dyn_lwp::changePC(Address addr, const void *savedRegs)
{
  prgregset_t theIntRegs;
  if (!savedRegs) {
    if (ioctl(fd_, PIOCGREG, &theIntRegs) == -1) {
      perror("dyn_lwp::changePC PIOCGREG");
      if (errno == EBUSY) {
	cerr << "It appears that the process wasn't stopped in the eyes of /proc" << endl;
	assert(false);
      }
      return false;
    }
  }    
  else {
    memcpy(&theIntRegs, savedRegs, sizeof(theIntRegs));
  }
  
  theIntRegs[R_PC] = addr;
#ifdef R_nPC
  theIntRegs[R_nPC] = addr + 4;
#endif
  
  if (ioctl(fd_, PIOCSREG, &theIntRegs) == -1) {
    perror("dyn_lwp::changePC PIOCSREG");
    if (errno == EBUSY) {
      cerr << "It appears that the process wasn't stopped in the eyes of /proc" << endl;
      assert(false);
    }
    return false;
  }
  return true;
}

#if defined(i386_unknown_solaris2_5)
bool process::changeIntReg(int reg, Address val) {
   assert(status_ == stopped); // /proc will require this

   prgregset_t theIntRegs;
   if (ioctl(getDefaultLWP()->get_fd(), PIOCGREG, &theIntRegs) == -1) {
     perror("dyn_lwp::changeIntReg");
      if (errno == EBUSY) {
	 cerr << "It appears that the process wasn't stopped in the eyes of /proc" << endl;
	 assert(false);
      }
      return false;
   }

   theIntRegs[reg] = val;

   if (ioctl(getDefaultLWP()->get_fd(), PIOCSREG, &theIntRegs) == -1) {
      perror("process::changeIntReg PIOCSREG");
      if (errno == EBUSY) {
	 cerr << "It appears that the process wasn't stopped in the eyes of /proc" << endl;
	 assert(false);
      }
      return false;
   }

   return true;
}
#endif

bool dyn_lwp::restoreRegisters(void *regs)
{
  // The fact that this routine can be shared between solaris/sparc and
  // solaris/x86 is just really, really cool.  /proc rules!  

  prgregset_t theIntRegs;
  prfpregset_t theFpRegs;

  memcpy(&theIntRegs, regs, sizeof(theIntRegs));
  memcpy(&theFpRegs, ((char *)regs) + sizeof(theIntRegs), sizeof(theFpRegs));

  if (ioctl(fd_, PIOCSREG, &theIntRegs) == -1) {
    perror("dyn_lwp::restoreRegisters PIOCSREG failed");
    if (errno == EBUSY) {
      cerr << "It appears that the process was not stopped in the eyes of /proc" << endl;
      assert(false);
    }
    return false;
  }
  
  if (ioctl(fd_, PIOCSFPREG, &theFpRegs) == -1) {
    perror("dyn_lwp::restoreRegisters PIOCSFPREG failed");
    if (errno == EBUSY) {
      cerr << "It appears that the process was not stopped in the eyes of /proc" << endl;
      assert(false);
    }
    return false;
  }
  
  return true;
}


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
	      function_base *func = findFuncByAddr(leaf_pc);
	      if(func && func->hasNoStackFrame()) { // formerly "isLeafFunc()"
		  return(true);
	      }
          }
      }
   }
   return false;
}

string process::tryToFindExecutable(const string &iprogpath, int pid) {
   // returns empty string on failure.
   // Otherwise, returns a full-path-name for the file.  Tries every
   // trick to determine the full-path-name, even though "progpath" may be
   // unspecified (empty string).
   
   // Remember, we can always return the empty string...no need to
   // go nuts writing the world's most complex algorithm.

   attach_cerr << "welcome to tryToFindExecutable; progpath=" << iprogpath << ", pid=" << pid << endl;

   const string progpath = expand_tilde_pathname(iprogpath);

   // Trivial case: if "progpath" is specified and the file exists then nothing needed
   if (exists_executable(progpath)) {
      attach_cerr << "tryToFindExecutable succeeded immediately, returning "
	          << progpath << endl;
      return progpath;
   }

   attach_cerr << "tryToFindExecutable failed on filename " << progpath << endl;

   char buffer[128];
   sprintf(buffer, "/proc/%05d", pid);
   int procfd = open(buffer, O_RDONLY, 0);
   if (procfd == -1) {
      attach_cerr << "tryToFindExecutable failed since open of /proc failed" << endl;
      return "";
   }
   attach_cerr << "tryToFindExecutable: opened /proc okay" << endl;

   string argv0, path, cwd;
   if (get_ps_stuff(procfd, argv0, path, cwd)) {
       // the following routine is implemented in the util lib.
       string result;
       if (executableFromArgv0AndPathAndCwd(result, argv0, path, cwd)) {
	  (void)close(procfd);
	  return result;
       }
   }

   attach_cerr << "tryToFindExecutable: giving up" << endl;

   (void)close(procfd);
   return "";
}

bool process::set_breakpoint_for_syscall_completion() {
   /* Can assume: (1) process is paused and (2) in a system call.
      We want to set a TRAP for the syscall exit, and do the
      inferiorRPC at that time.  We'll use /proc PIOCSEXIT.
      Returns true iff breakpoint was successfully set. */

   sysset_t save_exitset;
   if (-1 == ioctl(getDefaultLWP()->get_fd(), PIOCGEXIT, &save_exitset))
      return false;

   sysset_t new_exit_set;
   prfillset(&new_exit_set);
   if (-1 == ioctl(getDefaultLWP()->get_fd(), PIOCSEXIT, &new_exit_set))
      return false;

   assert(save_exitset_ptr == NULL);
   save_exitset_ptr = new sysset_t;
   memcpy(save_exitset_ptr, &save_exitset, sizeof(save_exitset));

   return true;
}

void process::clear_breakpoint_for_syscall_completion() { return; }

Address dyn_lwp::readRegister(Register /*reg*/)
{
   prgregset_t theIntRegs;
#ifdef PURE_BUILD
  // explicitly initialize "theIntRegs" struct (to pacify Purify)
  for (unsigned r=0; r<(sizeof(theIntRegs)/sizeof(theIntRegs[0])); r++)
      theIntRegs[r]=0;
#endif
  
  if (-1 == ioctl(fd_, PIOCGREG, &theIntRegs)) {
    perror("dyn_lwp::readRegister PIOCGREG");
    if (errno == EBUSY) {
      cerr << "It appears that the process was not stopped in the eyes of /proc" << endl;
      assert(false);
    }
    return 0; // assert(false)?
  }
  
  // on x86, the result is always stashed in %EAX; on sparc, it's always %o0.
  // In neither case do we need the argument of this fn.
#ifdef i386_unknown_solaris2_5
  return theIntRegs[EAX];
#else
  return theIntRegs[R_O0];
#endif
}

void print_read_error_info(const relocationEntry entry, 
      pd_Function *&target_pdf, Address base_addr) {

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
	      target_pdf->size());
      logLine(errorLine);
      sprintf(errorLine , "              addr 0x%lx\n",
	      target_pdf->addr());
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
			   pd_Function *&target_pdf, Address base_addr) {

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

    // get address of bound function, and return the corr. pd_Function
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
// instPoint "callee" data member is set to pt to callee's function_base.  
// If the function has not yet been bound, then "target" is set to the 
// function_base associated with the name of the target function (this is 
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
bool process::findCallee(instPoint &instr, function_base *&target){
    
    if((target = const_cast<function_base *>(instr.iPgetCallee()))) {
 	return true; // callee already set
    }

    // find the corresponding image in this process  
    const image *owner = instr.iPgetOwner();
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
//    Address insn_addr = instr.iPgetAddress(); 
    target_addr = instr.getTargetAddress();

    if(!target_addr) {  
	// this is either not a call instruction or an indirect call instr
	// that we can't get the target address
        target = 0;
        return false;
    }

#if defined(sparc_sun_solaris2_4)
    // If this instPoint is from a function that was relocated to the heap
    // then need to get the target address relative to this image   
    if(target_addr && instr.relocated_) {
	assert(target_addr > base_addr);
	target_addr -= base_addr;
    }
#endif

    // see if there is a function in this image at this target address
    // if so return it
    pd_Function *pdf = 0;
    if( (pdf = owner->findFuncByAddr(target_addr,this)) ) {
        target = pdf;
        instr.set_callee(pdf);
	return true; // target found...target is in this image
    }

    // else, get the relocation information for this image
    const Object &obj = owner->getObject();
    const vector<relocationEntry> *fbt;
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
	    pd_Function *target_pdf = 0;
	    if(hasBeenBound((*fbt)[i], target_pdf, base_addr)) {
                target = target_pdf;
                instr.set_callee(target_pdf);
	        return true;  // target has been bound
	    } 
	    else {
		// just try to find a function with the same name as entry 
		target = findFuncByName((*fbt)[i].name());
		if(target){
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
		    // names for pd_Functions

		    string s = string("_");
		    s += (*fbt)[i].name();
		    target = findFuncByName(s);
		    if(target){
	                return true;
		    }
		    s = string("__");
		    s += (*fbt)[i].name();
		    target = findFuncByName(s);
		    if(target){
	                return true;
		    }
		}
	    }
	    target = 0;
	    return false;
	}
    }
    target = 0;
    return false;  
}

#ifndef BPATCH_LIBRARY
void process::initCpuTimeMgrPlt() {
  cpuTimeMgr->installLevel(cpuTimeMgr_t::LEVEL_TWO, &process::yesAvail, 
			   timeUnit::ns(), timeBase::bNone(), 
			   &process::getRawCpuTime_sw, "swCpuTimeFPtrInfo");
}
#endif

bool dyn_lwp::openFD()
{
  if (lwp_) {
    fd_ = ioctl(proc_->getDefaultLWP()->get_fd(), PIOCOPENLWP, &lwp_);
    if (fd_ == (unsigned) -1) {
      return false;
    }
  }
  else {
    char procName[128];    
    sprintf(procName, "/proc/%05d", (int)proc_->getPid());
    fd_ = P_open(procName, O_RDWR, 0);
    if (fd_ == (unsigned) -1) {
      return false;
    }
  }
  return true;
}

void dyn_lwp::closeFD()
{
  if (fd_) close(fd_);
}
