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

// $Id: osf.C,v 1.3 1998/12/25 22:29:02 wylie Exp $

#include "util/h/headers.h"
#include "os.h"
#include "process.h"
#include "stats.h"
#include "util/h/Types.h"
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <filehdr.h>
#include <scnhdr.h>
#include <fcntl.h>
#include <ldfcn.h>
#include "showerror.h"
#ifndef BPATCH_LIBRARY
#include "main.h"
#endif 
#include <sys/procfs.h>
#include <sys/poll.h>
#include <sys/fault.h>

#define PC_REGNUM 31
extern bool exists_executable(const string &fullpathname);


int getNumberOfCPUs()
{
  return(1);
}


bool process::emitInferiorRPCheader(void *insnPtr, unsigned &baseBytes) {

  return true;
}

bool process::emitInferiorRPCtrailer(void *insnPtr, unsigned &baseBytes,
				     unsigned &breakOffset,
				     bool stopForResult,
				     unsigned &stopForResultOffset,
				     unsigned &justAfter_stopForResultOffset) {
  instruction *insn = (instruction *)insnPtr;
  unsigned baseInstruc = baseBytes / sizeof(instruction);

  extern void generateBreakPoint(instruction &);

  if (stopForResult) {
    generateBreakPoint(insn[baseInstruc]);
    stopForResultOffset = baseInstruc * sizeof(instruction);
    baseInstruc++;

    justAfter_stopForResultOffset = baseInstruc * sizeof(instruction);
  }
  
  // Trap instruction (breakpoint):
  generateBreakPoint(insn[baseInstruc]);
  breakOffset = baseInstruc * sizeof(instruction);
  baseInstruc++;

  // And just to make sure that we don't continue, we put an illegal
  // insn here:
  extern void generateIllegalInsn(instruction &);
  generateIllegalInsn(insn[baseInstruc++]);

  baseBytes = baseInstruc * sizeof(instruction); // convert back

  return true;
}

Address process::read_inferiorRPC_result_register(Register) {
  
  gregset_t theIntRegs;
  if (-1 == ioctl(proc_fd, PIOCGREG, &theIntRegs)) {
    perror("process::read_inferiorRPC_result_register PIOCGREG");
    if (errno == EBUSY) {
      cerr << "It appears that the process was not stopped in the eyes of /proc" << endl;
      assert(false);
    }
    return 0; // assert(false)?
  }
  return theIntRegs.regs[0];
}

//void OS::osTraceMe(void) { P_ptrace(PT_TRACE_ME, 0, 0, 0); }


bool process::needToAddALeafFrame(Frame current_frame, Address &leaf_pc){
    return false;
}

//
// return the current frame pointer (fp) and program counter (pc). 
//    returns true if we are able to read the regsiters.
//
bool process::getActiveFrame(Address *fp, Address *pc)
{
  gregset_t theIntRegs;
  bool ok=false;

  if (ioctl (proc_fd, PIOCGREG, &theIntRegs) != -1) {
      *fp=0; /* Don't know the index number for fp */
      *pc=theIntRegs.regs[PC_REGNUM]-4; /* -4 because the PC is updated */
      ok=true;
  }
  return(ok);
}

static inline bool execResult(prstatus_t stat) {
  return  1;
}

#ifndef OPEN_MAX
#define OPEN_MAX 1024
#endif


#ifdef BPATCH_LIBRARY
int process::waitforRPC(int *status,bool block)
{
  static struct pollfd fds[OPEN_MAX];  // argument for poll
  static int selected_fds;             // number of selected
  static int curr;                     // the current element of fds

  if (selected_fds == 0)
    {
      for (unsigned u = 0; u < processVec.size(); u++) {
	if (processVec[u]->status() == running || processVec[u]->status() == neonatal)
	  {
	    fds[u].fd = processVec[u]->proc_fd;
	    selected_fds++;
	  }
	else
	  fds[u].fd = -1;
	fds[u].events = 0xffff;
	fds[u].revents = 0;
      }
      curr = 0;
    }
  if (selected_fds > 0)
    {
      while(fds[curr].fd == -1) ++curr;
      prstatus_t stat;
      int ret = 0;
      int flag = 0;
      // Busy wait till the process actually stops on a fault
      while(!flag)
	{
	  if (ioctl(fds[curr].fd,PIOCSTATUS,&stat) != -1 && (stat.pr_flags & PR_STOPPED || stat.pr_flags & PR_ISTOP)) {
	    switch(stat.pr_why) {
	    case PR_FAULTED:
	    case PR_SIGNALLED:
	  *status = SIGTRAP << 8 | 0177;
	  ret = processVec[curr]->getPid();
	  flag = 1;
	  break;
	    }
	  }
	}
      --selected_fds;
      ++curr;
      if (ret > 0) return ret;
    }
  return 0;
}
#endif

// wait for a process to terminate or stop
#ifdef BPATCH_LIBRARY
int process::waitProcs(int *status,bool block = false) {
#else
int process::waitProcs(int *status) {
#endif
  static struct pollfd fds[OPEN_MAX];  // argument for poll
  static int selected_fds;             // number of selected
  static int curr;                     // the current element of fds

#ifdef BPATCH_LIBRARY
   do {
#endif

  /* Each call to poll may return many selected fds. Since we only report the status
     of one process per each call to waitProcs, we keep the result of the last
     poll buffered, and simply process an element from the buffer until all of
     the selected fds in the last poll have been processed.
     */
  
  if (selected_fds == 0) {
    for (unsigned u = 0; u < processVec.size(); u++) {
      if (processVec[u]->status() == running || processVec[u]->status() == neonatal)
	fds[u].fd = processVec[u]->proc_fd;
      else
	fds[u].fd = -1;
      fds[u].events = POLLPRI | POLLRDNORM;
      fds[u].revents = 0;
    }
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
#ifdef BPATCH_LIBRARY
    if (fds[curr].revents & POLLHUP) {
      do {
	ret = waitpid(processVec[curr]->getPid(), status, 0);
      } while ((ret < 0) && (errno == EINTR));
      if (ret < 0) {
	// This means that the application exited, but was not our child
	// so it didn't wait around for us to get it's return code.  In
	// this case, we can't know why it exited or what it's return
	// code was.
	ret = processVec[curr]->getPid();
	*status = 0;
      }
      assert(ret == processVec[curr]->getPid());
    } else {
#endif
      if (ioctl(fds[curr].fd, PIOCSTATUS, &stat) != -1) {
	if (stat.pr_why == PR_DEAD) {
	  do {
	    ret = waitpid(processVec[curr]->getPid(), status, 0);
	  } while ((ret < 0) && (errno == EINTR));
	  if (ret < 0) {
	    // This means that the application exited, but was not our child
	    // so it didn't wait around for us to get it's return code.  In
	    // this case, we can't know why it exited or what it's return
	    // code was.
	    ret = processVec[curr]->getPid();
	    *status = 0;
	  }
	  assert(ret == processVec[curr]->getPid());
	} else if (stat.pr_flags & PR_STOPPED || stat.pr_flags & PR_ISTOP) {
	  switch (stat.pr_why) {
	  case PR_SIGNALLED:
	    // return the signal number
	    *status = stat.pr_what << 8 | 0177;
	    ret = processVec[curr]->getPid();
	    break;
	  case PR_SYSEXIT:
	    // exit of exec
	    if (!execResult(stat)) {
	      // a failed exec. continue the process
	      processVec[curr]->continueProc_();
	      break;
	    }	    
	    
	    *status = SIGTRAP << 8 | 0177;
	    ret = processVec[curr]->getPid();
	    break;
	  case PR_REQUESTED:
	      stat.pr_flags = PRCSIG;
	      if (ioctl(fds[curr].fd, PIOCRUN, &stat) == -1) {
		fprintf(stderr, "attach: PIOCRUN failed: %s\n",
			sys_errlist[errno]);
		
		return false;
	      }
	      break;
	  case PR_FAULTED:
	    *status = SIGTRAP << 8 | 0177;
	    ret = processVec[curr]->getPid();
	    break;
	  case PR_JOBCONTROL:
	    assert(0);
	    break;
	  }	
	}
      }
#ifdef BPATCH_LIBRARY
    }
#endif
    
    --selected_fds;
    ++curr;
    
    if (ret > 0) {
      return ret;
    }
  }
#ifdef BPATCH_LIBRARY
   } while(block);
   return 0;
#else
  return waitpid(0, status, WNOHANG);
#endif
}


//
// given the pointer to a frame (currentFP), return 
//     (1) the saved frame pointer (fp)
//            NULL -> that currentFP is the bottom (last) frame.	
//     (2) the return address of the function for that frame (rtn).
//     (3) return true if we are able to read the frame.
//
bool process::readDataFromFrame(Address currentFP, Address *fp, Address *rtn,
                                bool uppermost = False)
{
	return false;
}

bool process::dumpCore_(const string coreFile) {
  if (!checkStatus()) 
    return false;
  ptraceOps++; ptraceOtherOps++;
  char temp[255];
  char name[] = "./test1.mutatee";
  sprintf(temp,"file = %s",coreFile.string_of());
  logLine(temp);
  assert(osDumpImage(name, pid, NULL));
  //  assert(OS::osDumpImage(symbols->file(), pid, symbols->codeOffset()));
  errno = 0;
  (void) ptrace(PT_CONTINUE, pid, (Address*)1, SIGBUS);
  assert(errno == 0);
  return true;
}

string process::tryToFindExecutable(const string &progpath, int pid) {
   // returns empty string on failure

   if (progpath.length() == 0)
      return "";

   if (exists_executable(progpath)) // util lib
      return progpath;

   return ""; // failure
}


//
// Write out the current contents of the text segment to disk.  This is useful
//    for debugging dyninst.
//
bool process::osDumpImage(const string &imageFileName,  pid_t pid, Address codeOff)
{
    int i;
    int rd;
    int ifd;
    int ofd;
    int cnt;
    int ret;
    int total;
    int length;
    Address baseAddr;
    extern int errno;
    char buffer[4096];
    char outFile[256];
    struct filehdr hdr;
    struct stat statBuf;
    bool needsCont = false;
    SCNHDR sectHdr;
    LDFILE      *ldptr = NULL;
    long text_size , text_start,file_ofs;

    ifd = open(imageFileName.string_of(), O_RDONLY, 0);
    if (ifd < 0) {
      sprintf(errorLine, "Unable to open %s\n", outFile);
      logLine(errorLine);
      showErrorCallback(41, (const char *) errorLine);
      perror("open");
      return true;
    }

    rd = fstat(ifd, &statBuf);
    if (rd != 0) {
      perror("fstat");
      sprintf(errorLine, "Unable to stat %s\n", outFile);
      logLine(errorLine);
      showErrorCallback(72, (const char *) errorLine);
      return true;
    }
    length = statBuf.st_size;
    sprintf(outFile, "%s.real", imageFileName.string_of());
    sprintf(errorLine, "saving program to %s\n", outFile);
    logLine(errorLine);

    ofd = open(outFile, O_WRONLY|O_CREAT, 0777);
    if (ofd < 0) {
      perror("open");
      exit(-1);
    }

    /* read header and section headers */
    /* Uses ldopen to parse the section headers */
    /* try */ 
     if (!(ldptr = ldopen(imageFileName.string_of(), ldptr))) {
       perror("Error in Open");
       exit(-1);
     }
     
     if (TYPE(ldptr) != ALPHAMAGIC) {
       printf("%s is not an alpha executable\n", imageFileName.string_of());
       exit(-1);
     }
     // Read the text and data sections
     hdr = HEADER(ldptr);
     /* Find text segment and then */
     /* compute text segment length and start offset */
     for (int k=0;k<hdr.f_nscns;k++)
       {
	 if (ldshread(ldptr, k , &sectHdr) == SUCCESS) {
	   sprintf(errorLine,"Section: %s  Start: %ld ",sectHdr.s_name,sectHdr.s_vaddr);
	   logLine(errorLine);
	   cout << "Section: " << sectHdr.s_name << "\tStart: " << sectHdr.s_vaddr 
		<< "\tEnd: " << sectHdr.s_vaddr + sectHdr.s_size << endl;
	   cout.flush();
	 }
	 else
	   {
	     perror("Error reading section header");
	     exit(-1);
	   }

	 if (!P_strcmp(sectHdr.s_name, ".text")) {
	   text_size = sectHdr.s_size;
	   text_start = sectHdr.s_vaddr;
	   file_ofs = sectHdr.s_scnptr;
	 }
       }
     ldclose(ldptr);
    /* ---------end section header read ------------*/

    /* now copy the entire file */
    lseek(ofd, 0, SEEK_SET);
    lseek(ifd, 0, SEEK_SET);
    for (i=0; i < length; i += 4096) {
        rd = read(ifd, buffer, 4096);
        write(ofd, buffer, rd);
        total += rd;
    }

    if (!stopped) {
        // make sure it is stopped.
        stop_();
        waitpid(pid, NULL, WUNTRACED);
	needsCont = true;
    }

    baseAddr = (Address) text_start;
    sprintf(errorLine, "seeking to %d as the offset of the text segment \n",
	file_ofs);
    logLine(errorLine);
    sprintf(errorLine, " code offset= %d\n", baseAddr);
    logLine(errorLine);

#ifdef notdef
    // Use the /proc file system to read in the text segment from memory
	char procName[128];

	sprintf(procName,"/proc/%05d", (int)pid);
	int proc_fd = P_open(procName, O_RDWR, 0);
	if (proc_fd < 0) {
	  fprintf(stderr, "attach: open failed: %s\n", sys_errlist[errno]);
	  return false;
	}
#endif
	
    /* seek to the text segment */
    lseek(ofd, file_ofs, SEEK_SET);
    for (i=0; i < text_size; i+= 1024) {
        errno = 0;
        length = ((i + 1024) < text_size) ? 1024 : text_size -i;
	if (lseek(proc_fd, (long)(baseAddr + i), SEEK_SET) != (long)(baseAddr + i))
	  {
	    fprintf(stderr,"Error_:%s\n",sys_errlist[errno]);
	    fprintf(stderr,"[%d] Couldn't lseek to the designated point\n",i);
	  }
	read(proc_fd,buffer,length);
	write(ofd, buffer, length);
    }

    
    if (needsCont) {
	ptrace(PT_CONTINUE, pid, (Address*) 1, SIGCONT);
    }

    close(ofd);
    close(ifd);
    close(proc_fd);

    return true;
}



