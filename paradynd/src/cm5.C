
/*
 * $Log: cm5.C,v $
 * Revision 1.5  1995/05/18 10:30:23  markc
 * Added dummy functions for getrusage calls
 *
 * Revision 1.4  1995/02/16  08:52:58  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.3  1995/02/16  08:32:52  markc
 * Changed igen interfaces to use strings/vectors rather than char  igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.2  1995/01/26  18:11:50  jcargill
 * Updated igen-generated includes to new naming convention
 *
 * Revision 1.1  1994/11/01  16:49:25  markc
 * Initial files that will provide os support.  This should limit os
 * specific features to these files.
 *
 */

#include "util/h/headers.h"
#include "os.h"
#include "process.h"
#include "stats.h"
#include "util/h/Types.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include "rtinst/h/trace.h"
#include "inst.h"
#include <sys/param.h>
// #include <sys/termios.h>

extern "C" {
extern int ioctl(int, int, ...);
#include <a.out.h>
#include <sys/exec.h>
#include <stab.h>
};

#include <machine/reg.h>
#include <cm/cmmd.h>
#include "ptrace_emul.h"

// TODO 
// TODO 
// PCptrace and nodePtrace need to be cleaned up.
// TODO
// TODO

int inNodePtrace = 0;

/* Prototypes */
int nodePtrace (enum ptracereq request, process *proc, int scalarPid,
		int nodeId, void *addr, int data, void *addr2);

// This is declared to be a friend of class process -- to allow for different ptrace interfaces
class ptraceKludge {
};

// disconnect from controlling terminal 
void OS::osDisconnect(void) {
  int ttyfd = open ("/dev/tty", O_RDONLY);
  ioctl (ttyfd, TIOCNOTTY, NULL); 
  close (ttyfd);
}

bool OS::osAttach(pid_t process_id) {
  return (!P_ptrace(PTRACE_ATTACH, process_id, 0, 0, 0) != -1);
}

bool OS::osStop(pid_t pid) {
  return (!P_kill(pid, SIGSTOP) != -1);
}

bool OS::osDumpCore(pid_t pid, const string fileTo) {
  return (!P_ptrace(PTRACE_DUMPCORE, pid, "core.real", 0, 0));
}

bool OS::osForwardSignal (pid_t pid, int stat) {
  return (!P_ptrace(PTRACE_CONT, pid, (char*)1, stat, 0));
}

bool OS::osDumpImage(const string &imageFileName, int pid, const Address off) {
  logLine("dumpcore not yet available\n");
  return false;
}

void OS::osTraceMe() { P_ptrace(PTRACE_TRACEME, 0, 0, 0, 0);}

/* 
 * The performance consultant's ptrace, it calls CM_ptrace and ptrace as needed.
 * TODO -- I don't want to touch this, and I ain't gonna touch this, but it
 *  really should be cleaned up -- mdc
 */
int PCptrace(enum ptracereq request, process *proc, char *addr,
	     int data, char *addr2)
{
    int ret;
//    int code;
    int cmPid;
    int scalarPid;
//    struct regs regs;
    extern int errno;

    errno = 0;

    if (proc->status() == exited) {
	sprintf (errorLine, 
		 "attempt to ptrace exited process %d\n", proc->pid);
	logLine(errorLine);
	return(-1);
    }

    /* stats */
    ptraceOps++;
    if (request == PTRACE_WRITEDATA) 
	ptraceBytes += data;
    else if (request == PTRACE_POKETEXT) 
	ptraceBytes += sizeof(int);
    else 
	ptraceOtherOps++;


    if (proc == nodePseudoProcess) {
	// do a broadcast ptrace.
	nodePtrace (request, proc, proc->pid%MAXPID, 0xffffffff, addr, 
		    data, addr2);
    } else if (proc->pid < MAXPID) {
	/* normal process */
	int sig;
	int status;
	int isStopped, wasStopped;

	wasStopped = (proc->status() == stopped);
	if (proc->status() != neonatal && !wasStopped &&
	    request != PTRACE_DUMPCORE) {
	    /* make sure the process is stopped in the eyes of ptrace */
	    kill(proc->pid, SIGSTOP);
	    isStopped = 0;
	    while (!isStopped) {
		ret = waitpid(proc->pid, &status, WUNTRACED);
		if ((ret == -1 && errno == ECHILD) || (WIFEXITED(status))) {
		    // the child is gone.
		    proc->kludgeStatus(exited);
		    return(0);
		}
		if (!WIFSTOPPED(status) && !WIFSIGNALED(status)) {
		    logLine("problem stopping process\n");
		    abort();
		}
		sig = WSTOPSIG(status);
		if (sig == SIGSTOP) {
		    isStopped = 1;
		} else {
		    sprintf(errorLine, "PCptrace forwarded signal %d while waiting for stop\n",
			    WSTOPSIG(status));
		    logLine(errorLine);
		    P_ptrace(PTRACE_CONT, proc->getPid(),(char*)1, WSTOPSIG(status),0);
		}
	    }
	}
	/* INTERRUPT is pseudo request to stop a process. prev lines do this */
	if (request == PTRACE_INTERRUPT) return(0);
	errno = 0;
	ret = P_ptrace(request, proc->pid,(char*) addr, data, (char*) addr2);
	assert(errno == 0);

	if ((proc->status != neonatal) && (request != PTRACE_CONT) &&
	    (!wasStopped)) {
	    (void) P_ptrace(PTRACE_CONT, proc->pid,(char*) 1, 0, (char*) 0);
	}
	errno = 0;
	return(ret);
    } else {
	cmPid = (proc->pid / MAXPID) - 1;
	scalarPid = proc->pid % MAXPID;

	/*
	 * Keeping this continue code around for a while, since we may need 
         * to do the same type of thing on the nodes in the future...
	 */

//	    case PTRACE_CONT:
//		/* need to advance pc past break point if at one */
//		code = CM_ptrace(cmPid, PE_PTRACE_GETSTATUS, scalarPid,0,0,0);
//		if (code == PE_STATUS_BREAK) {
//		    code = CM_ptrace(cmPid, PE_PTRACE_GETREGS, 
//			scalarPid, (char *) &regs,0,0);
//		    regs.r_pc += 4;
//		    code = CM_ptrace(cmPid, PE_PTRACE_SETREGS, scalarPid,
//			(char *) &regs,0,0);
//		}

	return (nodePtrace (request, proc, proc->pid%MAXPID, cmPid, addr, 
			    data, addr2));
    }
    return(0);
}

extern int pendingSampleNodes;
extern void sampleNodes();

/*
 * This routine send ptrace requests via CMMD to the nodes for processing.
 */
int nodePtrace (enum ptracereq request, process *proc, int scalarPid, int nodeId, 
		void *addr, int data, void *addr2)
{
    ptraceReqHeader header;
    extern int errno;

    inNodePtrace = 1;

#ifdef notdef
    {
      /* Check for node I/O */
      int i;
      for (i=0; i< 1000; i++)
	while (CMMD_poll_for_services() == 1)
	  ;			/* TEMPORARY:    XXXXXX */
    }
#endif

    /* Create the request header */
    header.request = request;
    header.pid = scalarPid;
    header.nodeNum = nodeId;
    header.addr = (char *) addr;
    header.data = data;
    header.addr2 = (char *) addr2;

    /* Send the ptrace request to the nodes for execution */
    CMMD_bc_from_host (&header, sizeof(header));

    /* Send optional addition data for the request */
    switch (request) {
      case PTRACE_WRITEDATA:
	CMMD_bc_from_host (addr2, data);
	break;
      default:
	// No "optional" data gets sent by default...
	break;
    }  

#ifdef DEBUG_PRINTS
    logLine ("Sent ptrace request (%d, %d, %d)\n", nodeId, addr, header.length);
#endif
   
    inNodePtrace = 0;
#ifdef notdef
    if (pendingSampleNodes)
	sampleNodes();
#endif

    errno = 0;			/* can be hosed by CMMD_poll_for_services() */
    return 0;			/* WHAT SHOULD WE RETURN?  XXX */
}

// TODO is this safe here ?
bool process::continueProc_() {
#ifdef notdef
  if (!checkStatus()) 
    return false;
  ptraceOps++; ptraceOtherOps++;
  return (P_ptrace(PTRACE_CONT, pid, (char*)1, 0, (char*)NULL) != -1);
#endif
  return (PCptrace(PTRACE_CONT, this, (char*)1, 0, (char*)NULL) != -1);
}

// TODO ??
bool process::pause_() {
#ifdef notdef
  if (!checkStatus()) 
    return false;
  ptraceOps++; ptraceOtherOps++;
  bool wasStopped = (status() == stopped);
  if (status() != neonatal && !wasStopped)
    return (loopUntilStopped());
  else
    return true;
#endif
  return (PCptrace(PTRACE_INTERRUPT, this, (char*)1, 0, (char*)NULL) != -1);
}

bool process::detach_() {
#ifdef notdef
  if (!checkStatus())
    return false;
  ptraceOps++; ptraceOtherOps++;
  return (ptraceKludge::deliverPtrace(this, PTRACE_DETACH, (char*)1, SIGCONT, NULL));
#endif
  return (PCptrace(PTRACE_DETACH, this, (char*) 1, SIGCONT, NULL) != -1);
}

// temporarily unimplemented, PTRACE_DUMPCORE is specific to sunos4.1
bool process::dumpCore_(const string coreFile) {
#ifdef notdef
  if (!checkStatus()) 
    return false;
  ptraceOps++; ptraceOtherOps++;

  assert(0);
  errno = 0;
  // int ret = P_ptrace(request, pid, coreFile, 0, (char*) NULL);
  int ret = 0;
  assert(errno == 0);
  return ret;
#endif
  return (PCptrace(PTRACE_DUMPCORE, this, (char*)"core.out", 0, (char*)NULL) != -1);
}

bool process::writeTextWord_(caddr_t inTraced, int data) {
#ifdef notdef
  if (!checkStatus()) 
    return false;
  ptraceBytes += sizeof(int); ptraceOps++;
  return (ptraceKludge::deliverPtrace(this, PTRACE_POKETEXT, inTraced, data, NULL));
#endif
  return (PCptrace(PTRACE_POKETEXT, this, (char*) inTraced, data, NULL) != -1);
}

bool process::writeTextSpace_(caddr_t inTraced, int amount, caddr_t inSelf) {
#ifdef notdef
  if (!checkStatus()) 
    return false;
  ptraceBytes += amount; ptraceOps++;
  return (ptraceKludge::deliverPtrace(this, PTRACE_WRITETEXT, inTraced, amount, inSelf));
#endif
  abort();
  return false;
}

bool process::writeDataSpace_(caddr_t inTraced, int amount, caddr_t inSelf) {
#ifdef notdef
  if (!checkStatus())
    return false;
  ptraceOps++; ptraceBytes += amount;
  return (ptraceKludge::deliverPtrace(this, PTRACE_WRITEDATA, inTraced, amount, inSelf));
#endif
  return (PCptrace(PTRACE_WRITEDATA, this, inTraced, amount, inSelf) != -1);
}

bool process::readDataSpace_(caddr_t inTraced, int amount, caddr_t inSelf) {
#ifdef notdef
  if (!checkStatus())
    return false;
  ptraceOps++; ptraceBytes += amount;
  return (ptraceKludge::deliverPtrace(this, PTRACE_READDATA, inTraced, amount, inSelf));
#endif
  abort();
  return false;
}

bool process::loopUntilStopped() {
#ifdef notdef
  /* make sure the process is stopped in the eyes of ptrace */
  OS::osStop(pid);
  bool isStopped = false;
  int waitStatus;
  while (!isStopped) {
    int ret = P_waitpid(pid, &waitStatus, WUNTRACED);
    if ((ret == -1 && errno == ECHILD) || (WIFEXITED(waitStatus))) {
      // the child is gone.
      status_ = exited;
      return(false);
    }
    if (!WIFSTOPPED(waitStatus) && !WIFSIGNALED(waitStatus)) {
      printf("problem stopping process\n");
      assert(0);
    }
    int sig = WSTOPSIG(waitStatus);
    if (sig == SIGSTOP) {
      isStopped = true;
    } else {
      if (P_ptrace(PTRACE_CONT, pid, (char*)1, WSTOPSIG(waitStatus), 0) == -1) {
	cerr << "Ptrace error\n";
	assert(0);
      }
    }
  }
  return true;
#endif
  abort();
  return false;
}

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
