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

// $Id: osf.C,v 1.58 2003/10/22 16:00:50 schendel Exp $

#include "common/h/headers.h"
#include "os.h"
#include "process.h"
#include "dyn_lwp.h"
#include "stats.h"
#include "common/h/Types.h"
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
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

#include "common/h/osfKludges.h"
#include "dyninstAPI/src/rpcMgr.h"
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/dyn_thread.h"

#define V0_REGNUM 0	/* retval from integer funcs */
#define PC_REGNUM 31
#define SP_REGNUM 30
#define FP_REGNUM 15
#define A0_REGNUM 16	/* first param to funcs and syscalls */
#define RA_REGNUM 26
extern bool exists_executable(const pdstring &fullpathname);

extern unsigned enable_pd_attach_detach_debug;

#if ENABLE_DEBUG_CERR == 1
#define attach_cerr if (enable_pd_attach_detach_debug) cerr
#else
#define attach_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

int getNumberOfCPUs()
{
  return(1);
}


bool rpcMgr::emitInferiorRPCheader(void *insnPtr, Address &baseBytes) {

  extern void emitSaveConservative(process *, char *, Address &baseBytes);

  emitSaveConservative(proc_, (char *) insnPtr, baseBytes);

  return true;
}

bool rpcMgr::emitInferiorRPCtrailer(void *insnPtr, Address &baseBytes,
				     unsigned &breakOffset,
				     bool stopForResult,
				     unsigned &stopForResultOffset,
				     unsigned &justAfter_stopForResultOffset) {
  instruction *insn = (instruction *)insnPtr;
  Address baseInstruc = baseBytes / sizeof(instruction);


  extern void generate_nop(instruction*);
  extern void generateBreakPoint(instruction &);
  extern void emitRestoreConservative(process *, char *, Address &baseBytes);

  emitRestoreConservative(proc_, (char *) insnPtr, baseBytes);

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

Address dyn_lwp::readRegister(Register /*reg*/)
{  
  gregset_t theIntRegs;
  if (-1 == ioctl(fd_, PIOCGREG, &theIntRegs)) {
    perror("process::readRegister PIOCGREG");
    if (errno == EBUSY) {
      cerr << "It appears that the process was not stopped in the eyes of /proc" << endl;
      assert(false);
    }
    return 0; // assert(false)?
  }
  return theIntRegs.regs[0];
}

//void OS::osTraceMe(void) { P_ptrace(PT_TRACE_ME, 0, 0, 0); }


// getActiveFrame(): populate Frame object using toplevel frame
Frame dyn_lwp::getActiveFrame()
{
  Address pc, fp;
  Frame theFrame;
  gregset_t theIntRegs;
//  int proc_fd = p->getProcFileDescriptor();
  if (ioctl(fd_, PIOCGREG, &theIntRegs) != -1) {
    fp = theIntRegs.regs[SP_REGNUM];  
    pc = theIntRegs.regs[PC_REGNUM]-4; /* -4 because the PC is updated */
    theFrame = Frame(pc, fp, proc_->getPid(), NULL, this, true);
  }
  return theFrame;
}

/*
 * Use by dyninst to set events we care about from procfs
 *
 */
bool process::installSyscallTracing()
{

  long flags = 0;
  if (BPatch::bpatch->postForkCallback) {
      // cause the child to inherit trap-on-exit from exec and other traps
      // so we can learn of the child (if the user cares)
      flags = PR_FORK | PR_ASYNC | PR_RLC;
  }

  dyn_lwp *replwp = getRepresentativeLWP();
  if (ioctl(replwp->get_fd(), PIOCSET, &flags) < 0) {
    fprintf(stderr, "attach: PIOCSET failed: %s\n", sys_errlist[errno]);
    return false;
  }

  // cause a stop on the exit from fork
  sysset_t sysset;

  if (ioctl(replwp->get_fd(), PIOCGEXIT, &sysset) < 0) {
    fprintf(stderr, "attach: ioctl failed: %s\n", sys_errlist[errno]);
    return false;
  }

  if (BPatch::bpatch->postForkCallback) {
      praddset (&sysset, SYS_fork);
      praddset (&sysset, SYS_execv);
      praddset (&sysset, SYS_execve);
  }
  
  if (ioctl(replwp->get_fd(), PIOCSEXIT, &sysset) < 0) {
    fprintf(stderr, "attach: ioctl failed: %s\n", sys_errlist[errno]);
    return false;
  }

  // now worry about entry too
  if (ioctl(replwp->get_fd(), PIOCGENTRY, &sysset) < 0) {
    fprintf(stderr, "attach: ioctl failed: %s\n", sys_errlist[errno]);
    return false;
  }

  if (BPatch::bpatch->exitCallback) {
      praddset (&sysset, SYS_exit);
  }

  if (BPatch::bpatch->preForkCallback) {
      praddset (&sysset, SYS_fork);
      praddset (&sysset, SYS_vfork);
      // praddset (&sysset, SYS_waitsys);
  }

  // should these be for exec callback??
  prdelset (&sysset, SYS_execv);
  prdelset (&sysset, SYS_execve);
  
  if (ioctl(replwp->get_fd(), PIOCSENTRY, &sysset) < 0) {
    fprintf(stderr, "attach: ioctl failed: %s\n", sys_errlist[errno]);
    return false;
  }

  return true;
}


static inline bool execResult(prstatus_t stat) 
{
  return (stat.pr_reg.regs[V0_REGNUM] == 0);
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
	    fds[u].fd = processVec[u]->getRepresentativeLWP()->get_fd();
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

procSyscall_t decodeSyscall(process *p, procSignalWhat_t syscall)
{
    if (syscall == SYS_fork ||
        syscall == SYS_vfork)
        return procSysFork;
    if (syscall == SYS_execv ||
        syscall == SYS_execve)
        return procSysExec;
    if (syscall == SYS_exit)
        return procSysExit;
    return procSysOther;
}

int decodeProcStatus(process *proc,
                     procProcStatus_t status,
                     procSignalWhy_t &why,
                     procSignalWhat_t &what,
                     procSignalInfo_t &info) {
    
    switch (status.pr_why) {
  case PR_SIGNALLED:
      why = procSignalled;
      what = status.pr_what;
      break;
  case PR_SYSENTRY:
      why = procSyscallEntry;
      what = status.pr_what;
      info = status.pr_reg.regs[A0_REGNUM];
      break;
  case PR_SYSEXIT:
      why = procSyscallExit;
      what = status.pr_what;
      info = status.pr_reg.regs[V0_REGNUM];
      break;
  case PR_REQUESTED:
      // We don't expect PR_REQUESTED in the signal handler
      assert(0 && "PR_REQUESTED not handled");
      break;
  case PR_JOBCONTROL:
  case PR_FAULTED:
  default:
      assert(0);
      break;
    }
    return 1;
}


// Get and decode a signal for a process
// We poll /proc for process events, and so it's possible that
// we'll get multiple hits. In this case we return one and queue
// the rest. Further calls of decodeProcessEvent will burn through
// the queue until there are none left, then re-poll.
// Return value: 0 if nothing happened, or process pointer

// the pertinantLWP and wait_options are ignored on Solaris, AIX

process *decodeProcessEvent(dyn_lwp **pertinantLWP, int wait_arg, 
                            procSignalWhy_t &why, procSignalWhat_t &what,
                            procSignalInfo_t &info, bool block,
                            int wait_options)
{
    why = procUndefined;
    what = 0;
    info = 0;

    extern pdvector<process*> processVec;
    static struct pollfd fds[OPEN_MAX];  // argument for poll
    // Number of file descriptors with events pending
    static int selected_fds = 0; 
    // The current FD we're processing.
    static int curr = 0;
    prstatus_t stat;

    if (selected_fds == 0) {
        for (unsigned u = 0; u < processVec.size(); u++) {
            //printf("checking %d\n", processVec[u]->getPid());
            if (processVec[u] && 
                (processVec[u]->status() == running || 
                 processVec[u]->status() == neonatal)) {
                if (wait_arg == -1 ||
                    processVec[u]->getPid() == wait_arg) {
                    fds[u].fd =processVec[u]->getRepresentativeLWP()->get_fd();
                    // Apparently, exit doesn't cause a poll event. Odd...
                    int status;
                    int retWait = waitpid(processVec[u]->getPid(), &status, WNOHANG|WNOWAIT);
                    if (retWait == -1) {
                        perror("Initial waitpid");
                    }
                    else if (retWait > 1) {
                        decodeWaitPidStatus(processVec[u], status, why, what);
                        // In this case we return directly
                        return processVec[u];
                    }
                }
            } else {
                fds[u].fd = -1;
            }	
            fds[u].events = POLLPRI;
            fds[u].revents = 0;
        }
        if (selected_fds == 0) {
            int timeout;
            // Exit doesn't provide an event, so we need a timeout eventually
            if (block) timeout = 5;
            else timeout = 0;
            selected_fds = poll(fds, processVec.size(), timeout);
        }
        if (selected_fds <= 0) {
            if (selected_fds < 0) {
                fprintf(stderr, "decodeProcessEvent: poll failed: %s\n",
                        sys_errlist[errno]);
                selected_fds = 0;
            }
            return NULL;
        }
        
        // Reset the current pointer to the beginning of the poll list
        curr = 0;
    } // if selected_fds == 0
    // We have one or more events to work with.
    while (fds[curr].revents == 0) {
        // skip
        ++curr;
    }
    // fds[curr] has an event of interest
    prstatus_t procstatus;
    process *currProcess = processVec[curr];
    
    if (fds[curr].revents & POLLHUP) {
        // True if the process exited out from under us
        int status;
        int ret;
        // Process exited, get its return status
        do {
            ret = waitpid(currProcess->getPid(), &status, 0);
        } while ((ret < 0) && (errno == EINTR));
        if (ret < 0) {
            // This means that the application exited, but was not our child
            // so it didn't wait around for us to get it's return code.  In
            // this case, we can't know why it exited or what it's return
            // code was.
            ret = currProcess->getPid();
            status = 0;
            // is this the bug??
            // processVec[curr]->continueProc_();
        }
        if (!decodeWaitPidStatus(currProcess, status, why, what)) {
            cerr << "decodeProcessEvent: failed to decode waitpid return" << endl;
            return NULL;
        }
    } else {
        // Real return from poll
        if (ioctl(currProcess->getRepresentativeLWP()->get_fd(), 
                  PIOCSTATUS, &procstatus) != -1) {
            // Check if the process is stopped waiting for us
            if (procstatus.pr_flags & PR_STOPPED ||
                procstatus.pr_flags & PR_ISTOP) {
                if (!decodeProcStatus(currProcess, procstatus, why, what, info))
                    return NULL;
            }
        }
        else {
            // Process exited on us
        }
    }
    // Skip this FD the next time through

    if (currProcess) {
        currProcess->savePreSignalStatus();
        currProcess->set_status(stopped);
    }
    

    --selected_fds;
    ++curr;    
    return currProcess;
    
} 

Frame Frame::getCallerFrame(process *p) const
{
  Frame ret;
  Address values[2];
  gregset_t theIntRegs;
  pd_Function *currFunc;
  if (fp_ == 0) return Frame();

  if (uppermost_) {
      int proc_fd = p->getRepresentativeLWP()->get_fd();
      if (ioctl(proc_fd, PIOCGREG, &theIntRegs) != -1) {
          ret.pc_ = theIntRegs.regs[PC_REGNUM];  

          currFunc = p->findFuncByAddr(ret.pc_);
          if (currFunc && currFunc->frame_size) {
              ret.fp_ = theIntRegs.regs[SP_REGNUM] + currFunc->frame_size;  
              ret.sp_ = theIntRegs.regs[SP_REGNUM];
              //printf(" %s fp=%lx\n",currFunc->prettyName().c_str(), ret.fp_);
          } else {
              ret.fp_ = 0;
              sprintf(errorLine, "pc %lx, not in a known function\n", ret.pc_);
              logLine(errorLine);
          }
          
      } else {
          return Frame(); // zero frame
      }
  } else {
      if (!p->readDataSpace((void *)sp_, sizeof(Address), values, false)){
          printf("error reading frame at %lx\n", fp_);
          return Frame(); // zero frame
      } else {
          // (*sp_) = RA
          // fp_ + frame_size = saved fp
          ret.pc_ = values[0];
          
          currFunc = p->findFuncByAddr(ret.pc_);
          if (currFunc && currFunc->frame_size) {
              ret.sp_ = fp_;		/* current stack pointer is old fp */
              ret.fp_ = fp_ + currFunc->frame_size;  
              //printf(" %s fp=%lx\n",currFunc->prettyName().c_str(), ret.fp_);
          } else {
              sprintf(errorLine, "pc %lx, not in a known function\n", ret.pc_);
              logLine(errorLine);
              ret.fp_ = 0;
          }
      }
  }
  return ret;
}

bool process::dumpCore_(const pdstring coreFile) 
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

pdstring process::tryToFindExecutable(const pdstring &progpath, int pid) 
{
   // returns empty string on failure

   if (exists_executable(progpath)) // util lib
      return progpath;

   char buffer[128];
   sprintf(buffer, "/proc/%05d", pid);
   int procfd = open(buffer, O_RDONLY, 0);
   if (procfd == -1) {
      attach_cerr << "tryToFindExecutable failed since open of /proc failed" << endl;
      return "";
   }
   attach_cerr << "tryToFindExecutable: opened /proc okay" << endl;

   struct prpsinfo the_psinfo;

   if (ioctl(procfd, PIOCPSINFO, &the_psinfo) == -1) {
       close(procfd);
       return "";
   }

   char commandName[256];
   strcpy(commandName, the_psinfo.pr_psargs);
   if (strchr(commandName, ' ')) *(strchr(commandName, ' ')) = '\0';

   if (!access(commandName, X_OK)) {
       // found the file, return the results
       (void)close(procfd);
       return commandName;
   }

   printf("access to  %s failed \n", commandName);
   attach_cerr << "tryToFindExecutable: giving up" << endl;

   (void)close(procfd);

   return ""; // failure
}


//
// Write out the current contents of the text segment to disk.  This is useful
//    for debugging dyninst.
//
#ifdef BPATCH_LIBRARY
bool process::dumpImage(pdstring outFile)
#else
bool process::dumpImage()
#endif
{
#if !defined(BPATCH_LIBRARY)
  pdstring outFile = getImage()->file() + ".real";
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
    pdstring origFile = im->file();

    ifd = open(origFile.c_str(), O_RDONLY, 0);
    if (ifd < 0) {
      sprintf(errorLine, "Unable to open %s\n", origFile.c_str());
      logLine(errorLine);
      showErrorCallback(41, (const char *) errorLine);
      perror("open");
      return true;
    }

    rd = fstat(ifd, &statBuf);
    if (rd != 0) {
      perror("fstat");
      sprintf(errorLine, "Unable to stat %s\n", origFile.c_str());
      logLine(errorLine);
      showErrorCallback(72, (const char *) errorLine);
      return true;
    }
    length = statBuf.st_size;

    sprintf(errorLine, "saving program to %s\n", outFile.c_str());
    logLine(errorLine);

    ofd = open(outFile.c_str(), O_WRONLY|O_CREAT, 0777);
    if (ofd < 0) {
      perror("open");
      exit(-1);
    }

    /* read header and section headers */
    /* Uses ldopen to parse the section headers */
    /* try */ 
    if (!(ldptr = ldopen((char *) origFile.c_str(), ldptr))) {
       perror("Error in Open");
       exit(-1);
     }
     
     if (TYPE(ldptr) != ALPHAMAGIC) {
       printf("%s is not an alpha executable\n", outFile.c_str());
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
    sprintf(errorLine, "seeking to %ld as the offset of the text segment \n",
            file_ofs);
    logLine(errorLine);
    sprintf(errorLine, " code offset= %ld\n", baseAddr);
    logLine(errorLine);

    /* seek to the text segment */
    lseek(ofd,(off_t)file_ofs, SEEK_SET);
    for (i=0; i < text_size; i+= 1024) {
       errno = 0;
       length = ((i + 1024) < text_size) ? 1024 : text_size -i;
       dyn_lwp *replwp = getRepresentativeLWP();
       if (lseek(replwp->get_fd(),(off_t)(baseAddr + i), SEEK_SET) !=
           (long)(baseAddr + i))
       {
          fprintf(stderr,"Error_:%s\n",sys_errlist[errno]);
          fprintf(stderr,"[%d] Couldn't lseek to the designated point\n",i);
       }
       read(replwp->get_fd(),buffer,length);
       write(ofd, buffer, length);
    }

    close(ofd);
    close(ifd);

    return true;
}

/*
   terminate execution of a process
 */
bool process::terminateProc_()
{
    long flags = PRFS_KOLC;
    if (ioctl (getRepresentativeLWP()->get_fd(), PIOCSSPCACT, &flags) < 0)
        return false;

    // just to make sure it is dead
    kill(getPid(), 9);

    handleProcessExit(0);

    return true;
}

dyn_lwp *process::createRepresentativeLWP() {
   // don't register the representativeLWP in real_lwps since it's not a true
   // lwp
   representativeLWP = createFictionalLWP(0);
   return representativeLWP;
}

#if !defined(BPATCH_LIBRARY)
rawTime64 dyn_lwp::getRawCpuTime_hw()
{
  return 0;
}

/* return unit: nsecs */
rawTime64 dyn_lwp::getRawCpuTime_sw() 
{
  // returns user+sys time from the u or proc area of the inferior process,
  // which in turn is presumably obtained by mmapping it (sunos)
  // or by using a /proc ioctl to obtain it (solaris).
  // It must not stop the inferior process in order to obtain the result,
  // nor can it assue that the inferior has been stopped.
  // The result MUST be "in sync" with rtinst's DYNINSTgetCPUtime().
  
  // We use the PIOCUSAGE /proc ioctl
  
  // Other /proc ioctls that should work too: PIOCPSINFO and the
  // lower-level PIOCGETPR and PIOCGETU which return copies of the proc
  // and u areas, respectively.
  // PIOCSTATUS does _not_ work because its results are not in sync
  // with DYNINSTgetCPUtime
  
  rawTime64 now;
  
  prpsinfo_t procinfo;
  
  if (ioctl(fd_, PIOCPSINFO, &procinfo) == -1) {
    perror("process::getInferiorProcessCPUtime - PIOCPSINFO");
    abort();
  }
  
  /* Put secs and nsecs into usecs */
  now = procinfo.pr_time.tv_sec;
  now *= I64_C(1000000000);
  now += procinfo.pr_time.tv_nsec;
  
  if (now<sw_previous_) {
    // time shouldn't go backwards, but we have seen this happening
    // before, so we better check it just in case - naim 5/30/97
    logLine("********* time going backwards in paradynd **********\n");
    now=sw_previous_;
  }
  else {
    sw_previous_=now;
  }
  
  return now;
}
#endif

fileDescriptor *getExecFileDescriptor(pdstring filename,
				     int &,
				     bool)
{
  fileDescriptor *desc = new fileDescriptor(filename);
  return desc;
}

bool dyn_lwp::realLWP_attach_() {
   assert( false && "threads not yet supported on OSF");
   return false;
}

bool dyn_lwp::representativeLWP_attach_() {
   /*
     Open the /proc file correspoding to process pid, 
     set the signals to be caught to be only SIGSTOP,
     and set the kill-on-last-close and inherit-on-fork flags.
   */

   char procName[128];    
   sprintf(procName, "/proc/%d", (int)getPid());
   fd_ = P_open(procName, O_RDWR, 0);
   if (fd_ == (unsigned) -1) {
      perror("Error opening process file descriptor");
      return false;
   }

   /* we don't catch any child signals, except SIGSTOP */
   sigset_t sigs;
   fltset_t faults;
   premptyset(&sigs);
   praddset(&sigs, SIGSTOP);
   praddset(&sigs, SIGTRAP);
   praddset(&sigs, SIGSEGV);
   
   if (ioctl(fd_, PIOCSTRACE, &sigs) < 0) {
      fprintf(stderr, "attach: ioctl failed: %s\n", sys_errlist[errno]);
      close(fd_);
      return false;
   }
   
   premptyset(&faults);
   praddset(&faults,FLTBPT);
   if (ioctl(fd_, PIOCSFAULT, &faults) <0) {
      fprintf(stderr, "attach: ioctl failed: %s\n", sys_errlist[errno]);
      close(fd_);
      return false;
  }
   
   /* turn on the kill-on-last-close and inherit-on-fork flags. This will cause
      the process to be killed when paradynd exits.
      Also, any child of this process will stop at the exit of an exec call.
   */
#if defined(PIOCSET)
   {
      long flags = PR_KLC | PR_FORK;
      if (ioctl (fd_, PIOCSET, &flags) < 0) {
         fprintf(stderr, "attach: PIOCSET failed: %s\n", sys_errlist[errno]);
         close(fd_);
         return false;
      }
   }
#else
#if defined(PRFS_KOLC)
   {
      long pr_flags;
      if (ioctl(fd_, PIOCGSPCACT, &pr_flags) < 0) {
         sprintf(errorLine, "Cannot get status\n");
         logLine(errorLine);
         close(fd_);
         return false;
      }
      pr_flags |= PRFS_KOLC;
      ioctl(fd_, PIOCSSPCACT, &pr_flags);
   }
#endif
#endif

   return true;
}

void dyn_lwp::realLWP_detach_()
{
   assert(is_attached());  // dyn_lwp::detach() shouldn't call us otherwise
}

void dyn_lwp::representativeLWP_detach_()
{
   assert(is_attached());  // dyn_lwp::detach() shouldn't call us otherwise
   if (fd_) close(fd_);
}


void loadNativeDemangler() {}
