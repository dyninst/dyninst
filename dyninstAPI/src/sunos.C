
/* 
 * $Log: sunos.C,v $
 * Revision 1.1  1994/11/01 16:49:27  markc
 * Initial files that will provide os support.  This should limit os
 * specific features to these files.
 *
 */


/*
 * The performance consultant's ptrace, it calls CM_ptrace and ptrace as needed.
 *
 */

#include "os.h"
#include "util/h/kludges.h"
#include "ptrace_emul.h"
#include "process.h"
#include "stats.h"
#include "util/h/Types.h"

#include "util/h/kludges.h"

#include <fcntl.h>
#include <a.out.h>
#include <stab.h>
#include <strstream.h>

bool osAttach(const int process_id) {
  return (!ptrace(PTRACE_ATTACH, process_id, 0, 0, 0));
}

bool osStop(int pid) {
  return (!kill(pid, SIGSTOP));
}

bool osDumpCore(int pid, char *dumpTo) {
  return (!ptrace(PTRACE_DUMPCORE, pid, dumpTo, 0, 0));
}

bool osForwardSignal (int pid,  int stat) {
  return (!ptrace(PTRACE_CONT, pid, (char*)1, stat, 0));
}

void osTraceMe() {
  ptrace(PTRACE_TRACEME, 0, 0, 0, 0);
}

int PCptrace(int request, process *proc, char *addr, int data, char *addr2)
{
  int ret;
  int sig;
  int status;
  int isStopped, wasStopped;
  extern int errno;

  if (proc->status == exited) {
    sprintf(errorLine, "attempt to ptrace exited process %d\n", proc->pid);
    logLine(errorLine);
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
  if (proc->status != neonatal && !wasStopped && request != PTRACE_DUMPCORE) {
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
	ptrace(PTRACE_CONT, proc->pid,(char*)1, WSTOPSIG(status),0);
      }
    }
  }
  /* INTERRUPT is pseudo request to stop a process. prev lines do this */
  if (request == PTRACE_INTERRUPT) return(0);
  errno = 0;
  ret = ptrace(request, proc->pid, addr, data, addr2);
  assert(errno == 0);

  if ((proc->status != neonatal) && (request != PTRACE_CONT) &&
      (!wasStopped)) {
    (void) ptrace(PTRACE_CONT, proc->pid,(char*) 1, SIGCONT, (char*) 0);
  }
  errno = 0;
  return(ret);
}

bool osDumpImage(const string &imageFileName,  int pid, const Address codeOff)
{
  int i;
  int rd;
  int ifd;
  int ofd;
  int total;
  int length;
  struct exec my_exec;
  char buffer[4096];
  char outFile[256];
  extern int errno;
  struct stat statBuf;

  ostrstream os;

  os << imageFileName << ends;
  char *temp = os.str();
  ifd = open(temp, O_RDONLY, 0);
  if (ifd < 0) {
    perror("open");
    exit(-1);
  }

  rd = read(ifd, (char *) &my_exec, sizeof(struct exec));
  if (rd != sizeof(struct exec)) {
    perror("read");
    exit(-1);
  }

  rd = fstat(ifd, &statBuf);
  if (rd != 0) {
    perror("fstat");
    exit(-1);
  }
  length = statBuf.st_size;
  sprintf(outFile, "%s.real", temp);
  delete temp;
  sprintf(errorLine, "saving program to %s\n", outFile);
  logLine(errorLine);

  ofd = open(outFile, O_WRONLY|O_CREAT, 0777);
  if (ofd < 0) {
    perror("open");
    exit(-1);
  }

  /* now copy the rest */

  lseek(ofd, 0, SEEK_SET);
  write(ofd, (char *) &my_exec, sizeof(struct exec));

  if (!stopped) {
    // make sure it is stopped.
    osStop(pid);
    waitpid(pid, NULL, WUNTRACED);
  }

  lseek(ofd, N_TXTOFF(my_exec), SEEK_SET);
  for (i=0; i < my_exec.a_text; i+= 4096) {
    errno = 0;
    ptrace(PTRACE_READTEXT, pid, (char*) (codeOff + i), 4096, buffer);
    if (errno) {
      perror("ptrace");
      abort();
    }
    write(ofd, buffer, 4096);
  }

  ptrace(PTRACE_CONT, pid, (char*) 1, SIGCONT, 0);

  rd = lseek(ofd, N_DATOFF(my_exec), SEEK_SET);
  if (rd != N_DATOFF(my_exec)) {
    perror("lseek");
    exit(-1);
  }

  rd = lseek(ifd, N_DATOFF(my_exec), SEEK_SET);
  if (rd != N_DATOFF(my_exec)) {
    perror("lseek");
    exit(-1);
  }

  total = N_DATOFF(my_exec);
  for (i=N_DATOFF(my_exec); i < length; i += 4096) {
    rd = read(ifd, buffer, 4096);
    write(ofd, buffer, rd);
    total += rd;
  }
  if (total != length) {
    sprintf(errorLine, "tried to write %d bytes, only %d written\n",
	    length, total);
    logLine(errorLine);
  }

  close(ofd);
  close(ifd);
  return true;
}
