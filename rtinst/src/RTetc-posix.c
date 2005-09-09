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

/************************************************************************
 * $Id: RTetc-posix.c,v 1.76 2005/09/09 18:05:41 legendre Exp $
 * RTposix.c: runtime instrumentation functions for generic posix.
 ************************************************************************/

#ifdef SHM_SAMPLING
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <unistd.h>

#ifdef rs6000_ibm_aix4_1
#else
#include <sys/syscall.h>
#endif

#include <math.h>

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"

/************************************************************************
 * void DYNINST_install_ualarm_interval(unsigned interval)
 *
 * an implementation of "ualarm" using the "setitimer" syscall.
 * interval is in microseconds
************************************************************************/

extern void DYNINSTalarmExpire(int signo);

static int DYNINST_trace_fd = -1; /* low-level version of DYNINSTtraceFp */
static FILE* DYNINSTtraceFp = 0;


/************************************************************************
 * void DYNINSTflushTrace(void)
 *
 * flush any accumulated traces.
************************************************************************/

void
DYNINSTflushTrace(void) {
    if (DYNINSTtraceFp) fflush(DYNINSTtraceFp);
}

int
DYNINSTnullTrace(void) {
	int nullfd;

	if ((nullfd = open("/dev/null", O_WRONLY)) < 0 ||
	    dup2(nullfd, DYNINST_trace_fd) < 0 ||
	    close(nullfd) < 0 ||
	    freopen("/dev/null", "w", DYNINSTtraceFp) == 0) {
		perror("DYNINSTnullTrace");
		return (-1);
	}
	return 0;
}	

/************************************************************************
 * void DYNINSwriteTrace(void)
 *
 * write data on the trace stream
************************************************************************/

int
DYNINSTwriteTrace(void *buffer, unsigned count) {
    int   ret;
    char *b = (char *)buffer;
    assert(DYNINSTtraceFp);
    while (1) {
      errno=0;
      ret = fwrite((const void *)b, 1, count, DYNINSTtraceFp);
      if (errno || ret!=count) {
        if (errno==EINTR) {
          printf("(pid=%d) fwrite interrupted, trying again...\n",(int) getpid());
#if defined(ERESTART)
        } else if (errno==ERESTART) {  /* fwrite partially completed */
          b     += ret;
          count -= ret;
#endif
        } else {
	  perror("unable to write trace record");
          printf("disabling further data logging, pid=%d\n", (int) getpid());
          fflush(stdout);
	  return 0;
        }
      }
      else
	return 1;
    }
}

/***********************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

/************************************************************************
 * static int connectToDaemon(int paradyndPid)
 *
 * get a connection to a paradyn daemon for the trace stream
 *
 * uses a UNIX domain STREAM socket
************************************************************************/

static int connectToDaemon(int paradyndPid) {
  /* uses paradyndPid to obtain a UNIX STREAM socket address to open. */

  int sock_fd;
  struct sockaddr_un sadr;

  char path[100];
  sprintf(path, "%s/paradynd.%d", P_tmpdir, paradyndPid); /* P_tmpdir in <stdio.h> */

  sock_fd = socket(PF_UNIX, SOCK_STREAM, 0);
  if (sock_fd < 0) {
    perror("DYNINST connectToDaemon() socket()");
    abort();
  }

  sadr.sun_family = PF_UNIX;
  strcpy(sadr.sun_path, path);
  if (connect(sock_fd, (struct sockaddr *) &sadr, sizeof(sadr)) < 0) {
    perror("DYNINST connectToDaemon() connect()");
    abort();
  }

  return sock_fd;
}


/************************************************************************
 * DYNINSTinitTrace(int daemon_addr)
 *
 * initializes the trace stream. daemon_addr is a platform dependent value
 * that is used to create a connection. If daemon_addr is -1, we assume
 * that the trace stream is already open (by the daemon, when it started
 * this process). For posix systems, daemon_addr is the pid of the paradynd 
 * daemon
************************************************************************/

void DYNINSTinitTrace(int daemon_addr) {

  if (daemon_addr == -1) {
    /* this process was started by the paradynd, which set up a pipe on fd 3 */
    DYNINST_trace_fd = 3;
#if defined(rs6000_ibm_aix4_1) \
 || defined(mips_sgi_irix6_4)
    DYNINSTtraceFp = fdopen(dup(DYNINST_trace_fd), "w");
#else
    DYNINSTtraceFp = fdopen(syscall(SYS_dup,DYNINST_trace_fd), "w");
#endif
    if (!DYNINSTtraceFp) {
      perror("DYNINSTinitTrace: fdopen()");
      abort();
    }
  } else {
    /* fork or attach, get a connection to the daemon */
    DYNINST_trace_fd = connectToDaemon(daemon_addr);
    assert(DYNINST_trace_fd != -1);

#if defined(rs6000_ibm_aix4_1) \
 || defined(mips_sgi_irix6_4)
    DYNINSTtraceFp = fdopen(dup(DYNINST_trace_fd), "w"); 
#else
    DYNINSTtraceFp = fdopen(syscall(SYS_dup,DYNINST_trace_fd), "w");
#endif
    assert(DYNINSTtraceFp);
  }
  
}


void DYNINSTcloseTrace() {
  fclose(DYNINSTtraceFp);
  close(DYNINST_trace_fd);
}




/*
 * Return the number of bytes in a message hdr iovect.
 *
 */
int
DYNINSTgetMsgVectBytes(struct msghdr *msg)
{
    int i;
    int count;

    for (i=0, count=0; i < msg->msg_iovlen; i++) {
	count += msg->msg_iov[i].iov_len;
    }
    return count;
}

#define stache_blk_align(va)    ((void *)((Address)va & ~(STACHE_BLK_SIZE-1)))
#define STACHE_BLK_SIZE          stache_blk_size

/* Blizzard */
int stache_blk_size ;
void
DYNINSTreportNewMem(char *data_structure, void *va, int memory_size, int blk_size)
/*
 *
 */
{
    void *align_va ;
    struct _traceMemory newRes;

    stache_blk_size = blk_size ;
    align_va = stache_blk_align(va) ;    /* Do I need to align them?*/

#ifdef COSTTEST
    time64 startT,endT;
    startT=DYNINSTgetCPUtime();
#endif
    memset(&newRes, '\0', sizeof(newRes));
    sprintf(newRes.name, "%s", data_structure);
    newRes.va = (int)(Address)align_va ;
    newRes.memSize = memory_size ;
    newRes.blkSize = blk_size ;
    PARADYNgenerateTraceRecord(TR_NEW_MEMORY, 
                               sizeof(struct _traceMemory),  &newRes, 
                               0.0, 0.0);
}

/*Change into DYNINSTfallIn in the future*/
int 
fallIn(int addr, int lower, int upper)
{
        return (addr<=upper && addr >= lower) ;
}

int pipeOK(void) {
  return (DYNINSTtraceFp != NULL);
}


