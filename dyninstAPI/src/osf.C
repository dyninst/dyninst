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

// $Id: osf.C,v 1.10 2000/02/09 18:43:15 hollings Exp $

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
#define FP_REGNUM 15
#define RA_REGNUM 26
extern bool exists_executable(const string &fullpathname);


int getNumberOfCPUs()
{
  return(1);
}


bool process::emitInferiorRPCheader(void *insnPtr, Address &baseBytes) {

  extern void emitSaveConservative(process *, char *, Address &baseBytes);

  emitSaveConservative(this, (char *) insnPtr, baseBytes);

  return true;
}

bool process::emitInferiorRPCtrailer(void *insnPtr, Address &baseBytes,
				     unsigned &breakOffset,
				     bool stopForResult,
				     unsigned &stopForResultOffset,
				     unsigned &justAfter_stopForResultOffset) {
  instruction *insn = (instruction *)insnPtr;
  Address baseInstruc = baseBytes / sizeof(instruction);


  extern void generateBreakPoint(instruction &);
  extern void emitRestoreConservative(process *, char *, Address &baseBytes);

  emitRestoreConservative(this, (char *) insnPtr, baseBytes);

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


bool process::needToAddALeafFrame(Frame, Address &)
{
    return false;
}

// getActiveFrame(): populate Frame object using toplevel frame
void Frame::getActiveFrame(process *p)
{
  gregset_t theIntRegs;
  int proc_fd = p->getProcFileDescriptor();
  if (ioctl(proc_fd, PIOCGREG, &theIntRegs) != -1) {
    fp_ = theIntRegs.regs[FP_REGNUM];  
    pc_ = theIntRegs.regs[PC_REGNUM]-4; /* -4 because the PC is updated */
  }
}

static inline bool execResult(prstatus_t) 
{
  return  1;
}

#ifndef OPEN_MAX
#define OPEN_MAX 1024
#endif


#ifdef BPATCH_LIBRARY
int process::waitforRPC(int *status, bool /* block */)
{
  static struct pollfd fds[OPEN_MAX];  // argument for poll
  static int selected_fds;             // number of selected
  static int curr;                     // the current element of fds

  if (selected_fds == 0) {
      for (unsigned u = 0; u < processVec.size(); u++) {
	if (processVec[u]->status() == running || 
	    processVec[u]->status() == neonatal) {
	    fds[u].fd = processVec[u]->proc_fd;
	    selected_fds++;
	} else {
	  fds[u].fd = -1;
	}
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
//   return the pid of the process that has stopped or blocked.
int process::waitProcs(int *status, bool block = false) 
{
    int ret = 0;
    static struct pollfd fds[OPEN_MAX];  // argument for poll
    static int selected_fds;             // number of selected
    static int curr;                     // the current element of fds
    bool skipPoll = false;

    do {
         /* 
	 Each call to poll may return many selected fds. Since we only report 
	 the status of one process per each call to waitProcs, we keep the result of 
	 the last poll buffered, and simply process an element from the buffer until 
	 all of the selected fds in the last poll have been processed.
	 */
      
	if (selected_fds == 0) {
	    for (unsigned u = 0; u < processVec.size(); u++) {
		if (processVec[u]->status_ == exited) {
		    fds[u].fd = -1;
		    continue;
		}
	        fds[u].fd = processVec[u]->proc_fd;
	        fds[u].events = POLLPRI | POLLIN;
	        fds[u].revents = 0;
	        if (waitpid(processVec[u]->getPid(), status, WNOHANG|WNOWAIT)) {
		    fds[u].revents = POLLPRI;
		    selected_fds++;
		    skipPoll = true;
		    break;
	        }
	    }

	    if (!skipPoll) {
		int timeout = 0;
		if (block) timeout = 5; 
		// if (block) timeout = INFTIM; 
		//printf("about to poll with timeout = %d, %d processes ", 
		     //timeout, processVec.size());
		fflush(stdout);
		selected_fds = poll(fds, processVec.size(), timeout);
		if (selected_fds < 0) {
		    fprintf(stderr, "waitProcs: poll failed: %s\n", sys_errlist[errno]);
		    selected_fds = 0;
		    return 0;
		}
		//printf("got = %d, \n", selected_fds);

		if (selected_fds <= 0) {
		    if (!block) return 0;
		    continue;
		}
	    }
	    curr = 0;
	}

	while (fds[curr].revents == 0) ++curr;

#if defined(USES_LIBDYNINSTRT_SO)
	if (!processVec[curr]->dyninstLibAlreadyLoaded() &&
	     processVec[curr]->wasCreatedViaAttach()) {
	   bool wasRunning = (processVec[curr]->status() == running);
	   if (processVec[curr]->status() != stopped)
	     processVec[curr]->Stopped();
	   if(processVec[curr]->isDynamicallyLinked()) {
	     processVec[curr]->handleIfDueToSharedObjectMapping();
	   }
	   if (processVec[curr]->trapDueToDyninstLib()) {
	     // we need to load libdyninstRT.so.1 - naim
	     processVec[curr]->handleIfDueToDyninstLib();
	     if (wasRunning) processVec[curr]->continueProc();
	   }
	}
#endif

	// fds[curr] has an event of interest
	prstatus_t stat;
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
	    ret = processVec[curr]->getPid();
	} else if (ioctl(fds[curr].fd, PIOCSTATUS, &stat) != -1) {
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
			    *status = stat.pr_what << 8 | 0177;
			    int ret = processVec[curr]->getPid();
			    break;
		}	
	    }
	}
    
	// clear the processed event
	fds[curr].revents = 0;
	if (selected_fds > 0) --selected_fds;
	++curr;
	
	if (ret > 0) {
	      return ret;
	}
    } while(block);
    return ret;
}

Frame Frame::getCallerFrameNormal(process *p) const
{
  Frame ret;
  Address values[2];
  gregset_t theIntRegs;

#ifdef notdef
  if (fp_ == 0) return false;

  if (uppermost_) {
    int proc_fd = p->getProcFileDescriptor();
    if (ioctl(proc_fd, PIOCGREG, &theIntRegs) != -1) {
      ret.fp_ = theIntRegs.regs[FP_REGNUM];  
      ret.pc_ = theIntRegs.regs[RA_REGNUM];  
    } else {
      return Frame(); // zero frame
    }
  } else {
    if (!p->readDataSpace((void *)fp_, sizeof(Address)*2, values, false)){
      printf("error reading frame at %lx\n", fp_);
      return Frame(); // zero frame
    } else {
      // (*fp) = RA
      // (*fp+8) = saved fp
      ret.pc_ = values[0];
      ret.fp_ = values[1];
      printf("in uppermost fp = %lx, ra = %lx\n", ret.fp_, ret.pc_);
    }
  }
  return ret;
#endif

  return Frame(); // zero frame
}

bool process::dumpCore_(const string coreFile) 
{
  //fprintf(stderr, ">>> process::dumpCore_()\n");
  bool ret;
#ifdef BPATCH_LIBRARY
  ret = dumpImage(coreFile);
#else
  ret = dumpImage();
#endif
  return ret;

}

string process::tryToFindExecutable(const string &progpath, int /* pid */) 
{
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
#ifdef BPATCH_LIBRARY
bool process::dumpImage(string outFile)
{
#else
bool process::dumpImage()
{
    string outFile = getImage()->file() + ".real";
#endif
    int i;
    int rd;
    int ifd;
    int ofd;
    int total;
    int length;
    Address baseAddr;
    extern int errno;
    const int COPY_BUF_SIZE = 4*4096;
    char buffer[COPY_BUF_SIZE];
    struct filehdr hdr;
    struct stat statBuf;
    SCNHDR sectHdr;
    LDFILE      *ldptr = NULL;
    image       *im;
    long text_size , text_start,file_ofs;

    im = getImage();
    string origFile = im->file();

    ifd = open(origFile.string_of(), O_RDONLY, 0);
    if (ifd < 0) {
      sprintf(errorLine, "Unable to open %s\n", origFile.string_of());
      logLine(errorLine);
      showErrorCallback(41, (const char *) errorLine);
      perror("open");
      return true;
    }

    rd = fstat(ifd, &statBuf);
    if (rd != 0) {
      perror("fstat");
      sprintf(errorLine, "Unable to stat %s\n", origFile.string_of());
      logLine(errorLine);
      showErrorCallback(72, (const char *) errorLine);
      return true;
    }
    length = statBuf.st_size;

    sprintf(errorLine, "saving program to %s\n", outFile.string_of());
    logLine(errorLine);

    ofd = open(outFile.string_of(), O_WRONLY|O_CREAT, 0777);
    if (ofd < 0) {
      perror("open");
      exit(-1);
    }

    /* read header and section headers */
    /* Uses ldopen to parse the section headers */
    /* try */ 
    if (!(ldptr = ldopen((char *) origFile.string_of(), ldptr))) {
       perror("Error in Open");
       exit(-1);
     }
     
     if (TYPE(ldptr) != ALPHAMAGIC) {
       printf("%s is not an alpha executable\n", outFile.string_of());
       exit(-1);
     }
     // Read the text and data sections
     hdr = HEADER(ldptr);
     /* Find text segment and then */
     /* compute text segment length and start offset */
     for (int k=0;k<hdr.f_nscns;k++) {
	 if (ldshread(ldptr, k , &sectHdr) == SUCCESS) {
	   // sprintf(errorLine,"Section: %s  Start: %ld ",sectHdr.s_name,
	   //  sectHdr.s_vaddr); 
	   // logLine(errorLine);
	   // cout << "Section: " << sectHdr.s_name << "\tStart: " << sectHdr.s_vaddr 
	   // << "\tEnd: " << sectHdr.s_vaddr + sectHdr.s_size << endl;
	   // cout.flush();
	 } else {
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
    for (i=0; i < length; i += COPY_BUF_SIZE) {
        rd = read(ifd, buffer, COPY_BUF_SIZE);
        write(ofd, buffer, rd);
        total += rd;
    }

    baseAddr = (Address) text_start;
    sprintf(errorLine, "seeking to %d as the offset of the text segment \n",
	file_ofs);
    logLine(errorLine);
    sprintf(errorLine, " code offset= %d\n", baseAddr);
    logLine(errorLine);

    /* seek to the text segment */
    lseek(ofd, file_ofs, SEEK_SET);
    for (i=0; i < text_size; i+= 1024) {
        errno = 0;
        length = ((i + 1024) < text_size) ? 1024 : text_size -i;
	if (lseek(proc_fd, (long)(baseAddr + i), SEEK_SET) != (long)(baseAddr + i)) {
	    fprintf(stderr,"Error_:%s\n",sys_errlist[errno]);
	    fprintf(stderr,"[%d] Couldn't lseek to the designated point\n",i);
	}
	read(proc_fd,buffer,length);
	write(ofd, buffer, length);
    }

    close(ofd);
    close(ifd);

    return true;
}



