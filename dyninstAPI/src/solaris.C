
/* 
 * $Log: solaris.C,v $
 * Revision 1.1  1994/11/01 16:49:29  markc
 * Initial files that will provide os support.  This should limit os
 * specific features to these files.
 *
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/ptrace.h>

#include "os.h"
#include "util/h/kludges.h"
#include "ptrace_emul.h"
#include "process.h"
#include "stats.h"
#include "util/h/Types.h"

bool osAttach(int pid) {
  return (!ptrace(PTRACE_ATTACH, pid, 0, 0));
}

bool osStop(int pid) {
  return (!kill(pid, SIGSTOP));
}

bool osDumpCore(int pid, char *fileTo) {
  return false;
}

bool osForwardSignal ( int pid,  int stat) {
  return (!ptrace(PTRACE_CONT, pid, 1, stat));
}

bool osDumpImage(const string &imageFileName,  int pid, const Address off) {
  logLine("dumpcore not yet available\n");
  return false;
}

void osTraceMe() {
  ptrace(PTRACE_TRACEME, 0, 0, 0);
}

/*
 * The performance consultant's ptrace, it calls CM_ptrace and ptrace as needed.
 *
 */
int PCptrace(int request, process *proc, char *addr, int data, char *addr2)
{
    int ret;
    int sig;
    int status;
    int isStopped, wasStopped;

    if (proc->status == exited) {
        printf("attempt to ptrace exited process %d\n", proc->pid);
        return(-1);
    }
	
    ptraceOps++;
    if (request == PTRACE_WRITEDATA)
	ptraceBytes += data;
    else if (request == PTRACE_POKETEXT)
	ptraceBytes += sizeof(int);
    else
	ptraceOtherOps++;

    wasStopped = (proc->status == stopped);
    if (proc->status != neonatal && !wasStopped && 
	request != PTRACE_DUMPCORE) {
	/* make sure the process is stopped in the eyes of ptrace */
	osStop(proc->pid);
	isStopped = 0;
	while (!isStopped) {
	    ret = waitpid(proc->pid, &status, WUNTRACED);
	    if ((ret == -1 && errno == ECHILD) || (WIFEXITED(status))) {
		// the child is gone.
		proc->status = exited;
		return(0);
	    }
	    if (!WIFSTOPPED(status) && !WIFSIGNALED(status)) {
		printf("problem stopping process\n");
		abort();
	    }
	    sig = WSTOPSIG(status);
	    if (sig == SIGSTOP) {
		isStopped = 1;
	    } else {
		ptrace(PTRACE_CONT, proc->pid,1, WSTOPSIG(status));
	    }
	}
    }
    /* INTERRUPT is pseudo request to stop a process. prev lines do this */
    if (request == PTRACE_INTERRUPT) return(0);
    if ((request == PTRACE_READTEXT) || (request == PTRACE_READDATA)) {
	int*     p1 = (int *) addr;
	int*     p2 = (int *) addr2;
	unsigned i;
	int      req = 0;
	int      retval;

	switch (request) {
	case PTRACE_READTEXT: req = PTRACE_PEEKTEXT; break;
	case PTRACE_READDATA: req = PTRACE_PEEKDATA; break;
	}

	data /= 4;
	for (i = 0; i < data; i++) {
	    errno = 0;
	    retval = ptrace(req, proc->pid, (int) p1, 0);
	    assert(errno == 0);
	    memcpy(p2, &retval, sizeof retval);
	    p1++; p2++;
	}

	ret = 0;
    }
    else if ((request == PTRACE_WRITETEXT) || (request == PTRACE_WRITEDATA)) {
	int*     p1 = (int *) addr;
	int*     p2 = (int *) addr2;
	unsigned i;
	int      req = 0;
	int      retval;

	switch (request) {
	case PTRACE_WRITETEXT: req = PTRACE_POKETEXT; break;
	case PTRACE_WRITEDATA: req = PTRACE_POKEDATA; break;
	}

	data /= 4;
	for (i = 0; i < data; i++) {
	    errno = 0;
	    memcpy(&retval, p2, sizeof retval);
	    ptrace(req, proc->pid, (int) p1, retval);
	    assert(errno == 0);
	    p1++; p2++;
	}
	ret = 0;
    }
    else {
        errno = 0;
	int*     p1 = (int *) addr;
        ret = ptrace(request, proc->pid, (int) p1, data);
        assert(errno == 0);
    }

    if ((proc->status != neonatal) && (request != PTRACE_CONT) &&
	(!wasStopped)) {
	(void) ptrace(PTRACE_CONT, proc->pid, 1, SIGCONT);
    }
    errno = 0;
    return(ret);
}


