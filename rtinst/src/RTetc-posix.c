/*
 * Copyright (c) 1996-2000 Barton P. Miller
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

/************************************************************************
 * $Id: RTetc-posix.c,v 1.71 2003/02/04 14:59:53 bernat Exp $
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

#include "kludges.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"


/************************************************************************
 * void PARADYNbreakPoint(void)
 *
 * stop oneself.
************************************************************************/

/* ccw 22 apr 2002 :  from dyninstAPI_RT/src/RTposix.c*/
void pDYNINSTbreakPoint(void) /* ccw 5 sep 2002 */
{
#ifdef DETACH_ON_THE_FLY
     extern pDYNINSTsigill();  /* ccw 5 sep 2002 */

     pDYNINSTsigill(); /* ccw 5 sep 2002 */

     return;
#endif /* DETACH_ON_THE_FLY */

#ifndef USE_IRIX_FIXES
     fprintf(stderr, "Kill %d\n", getpid());
     kill(getpid(), SIGKILL);
     sleep(10);
#else
     /* there is a bug in all 6.5 versions of IRIX through 6.5.9f that
        cause a PIOCSTRACE on SIGSTOP to starve (at least under the
        conditions that we are throwing it in.)  So on IRIX, we use
        SIGEMT.   -- willb, 10/4/2000 */
     kill(getpid(), SIGEMT);
#endif
     fprintf(stderr, "Returning from pDYNINSTbreadkPoint\n");
}

void
PARADYNbreakPoint(void) {
    int sample;
#ifdef DETACH_ON_THE_FLY
    /* DYNINSTbreakPoint (in dyninstAPI_RT/src/RTposix.c) calls DYNINSTsigill
       which is not strictly necessary here, probably because the trace
       record sent here wakes up the daemon correctly so the daemon does not
       need to see the SIGSTOP event.
    
       Use of SIGILL is complex.  If there's a problem with it here,
       specialize this function to avoid it. */
#endif
    fprintf(stderr, "Calling DYNINSTbreakPoint\n");
    pDYNINSTbreakPoint();/* ccw 5 sep 2002 */
    DYNINSTgenerateTraceRecord(0, TR_SYNC, 0, &sample, 0, 0.0, 0.0);
}



/************************************************************************
 * void DYNINST_install_ualarm_interval(unsigned interval)
 *
 * an implementation of "ualarm" using the "setitimer" syscall.
 * interval is in microseconds
************************************************************************/

extern void DYNINSTalarmExpire(int signo);

#ifndef SHM_SAMPLING
void
DYNINST_install_ualarm(unsigned interval) {
    struct itimerval it;
    struct sigaction act;

    act.sa_handler = DYNINSTalarmExpire;
    act.sa_flags   = 0;

    /* for AIX - default (non BSD) library does not restart - jkh 7/26/95 */
#if defined(SA_RESTART)
    act.sa_flags  |= SA_RESTART;
#endif

    sigfillset(&act.sa_mask);

#if defined(i386_unknown_solaris2_5) || defined(rs6000_ibm_aix4_1) 
    /* we need to catch SIGTRAP inside the alarm handler */    
    sigdelset(&act.sa_mask, SIGTRAP);
#endif

    if (sigaction(SIGALRM, &act, 0) == -1) {
        perror("sigaction(SIGALRM)");
        abort();
    }

    it.it_value.tv_sec     = interval / 1000000;
    it.it_value.tv_usec    = interval % 1000000;
    it.it_interval.tv_sec  = interval / 1000000;
    it.it_interval.tv_usec = interval % 1000000;

    if (setitimer(ITIMER_REAL, &it, 0) == -1) {
        perror("setitimer");
        abort();
    }
}
#endif





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




/**********************************************************************
 * Shared memory sampling functions 
 * TODO: these functions should be prefixed by DYNINST
***********************************************************************/

#ifdef SHM_SAMPLING
int shm_create_existing(key_t theKey, unsigned theSize) {
   /* "create" segment (it already exists; paradynd did the real creating) but don't
      attach; returns seg id */

   int segid = shmget(theKey, theSize, 0666); /* note no IPC_CREAT or IPC_EXCL flags */
   if (segid == -1) {
       perror("shmget");
       fprintf(stderr, "DYNINSTinit cannot shmget for key %d, size %u\n",
               (int)theKey, theSize);
       return -1;
   }

   return segid;
}

void *shm_attach(int shmid) {
   void *result = shmat(shmid, NULL, 0);
   if (result == (void *)-1) {
       perror("DYNINSTinit: cannot shmat");
       return NULL;
   }

   return result;
}

void shm_detach(void *shmSegPtr) {
   (void)shmdt(shmSegPtr);
}

void *shm_detach_reattach_overlap(int newshmid, void *shmSegPtr) {
   /* returns NULL on success (attached at same location),
      (void*)-1 on total failure, and
      a new address on partial failure (attached, but not at same location)
    */

   int success = 1; /* success, so far */
   void *newAttachedAtPtr = NULL;

   int detach_result = shmdt(shmSegPtr);
   if (detach_result == -1) {
      perror("shm_detach_reattach_overlap shmdt");
      success = 0; /* failure; don't try to attach at same loc */
   }

   if (success) {
      newAttachedAtPtr = shmat(newshmid, shmSegPtr, 0);
      if (newAttachedAtPtr == (void*)-1) {
         perror("shm_detach_reattach_overlap: cannot shmat");
	 success = 0; /* couldn't attach at same loc */
      }
   }

   if (success)
      return NULL; /* attached, and at the same location */
   else {
      /* couldn't attach at same location, so attach at a different location */
      newAttachedAtPtr = shmat(newshmid, NULL, 0);
      if (newAttachedAtPtr == (void*)-1)
	 /* this is unexpected...would have though that at least this would work */
	 perror("shm_detach_reattach_overlap: cannot shmat2");

      return newAttachedAtPtr; /* -1 on total failure; an address on partial */
   }
}


void *DYNINST_shm_init(int theKey, int shmSegNumBytes, int *the_id) {

  int the_shmSegShmId;
  void *the_shmSegAttachedPtr;

  /* note: error checking needs to be beefed up here: */
   
  the_shmSegShmId = shm_create_existing(theKey, shmSegNumBytes); /* -1 on error */
  if (the_shmSegShmId == -1) {
     /* note: in theory, when we get a shm error on startup, it would be nice
              to automatically "downshift" into the SIGALRM non-shm-sampling
              code.  Not yet implemented. */
     fprintf(stderr, "DYNINSTinit failed because shm_create_existing failed.\n");
     fprintf(stderr, "DYNINST program startup failed...exiting program now.\n");
     exit(5);
  }

  the_shmSegAttachedPtr = shm_attach(the_shmSegShmId); /* NULL on error */
  if (the_shmSegAttachedPtr == NULL) {
     /* see above note... */
     fprintf(stderr, "DYNINSTinit failed because shm_attach failed.\n");
     fprintf(stderr, "DYNINST program startup failed...exiting program now.\n");
     exit(5);
  }
  *the_id = the_shmSegShmId; 
  return the_shmSegAttachedPtr;

}

extern int DYNINST_shmSegKey;
extern int DYNINST_shmSegNumBytes;
extern int DYNINST_shmSegShmId;
extern void *DYNINST_shmSegAttachedPtr;

void shmsampling_printf(const char *fmt, ...) {
#ifdef SHM_SAMPLING_DEBUG
   va_list args;
   va_start(args, fmt);

   vfprintf(stderr, fmt, args);

   va_end(args);

   fflush(stderr);
#endif
}


void pickNewShmSegBaseKey(key_t *resultKey,
			  int *resultSegId) {
   /* picks an avail shm seg and creates it.  Doesn't attach to it, though */

   *resultKey = DYNINST_shmSegKey; /* as good a place to start as any... */

   while (1) {
      /* permissions chosen s.t. the request will fail if already exists */
      int permissions = 0666 | IPC_CREAT | IPC_EXCL;
         /* using IPC_EXCL ensure failure if already-exists */
      int failure;

      *resultSegId = shmget(*resultKey, DYNINST_shmSegNumBytes, permissions);

      /* successful? */
      failure = (*resultSegId == -1);

      /* If successful, then keep the segments and update result vrbles */
      if (!failure)
         return;

      /* On failure, we assume the shmget() didn't create anything. */
      /* So, all we need to do is bump the key and retry... */
      (*resultKey)++;
   }
}

void makeNewShmSegCopy(void) {
   /* detach from old shm segment, create new segment (IN THE SAME LOCATION AS THE OLD
      ONE(*)) */
   /* (*) the new shm segment MUST be in the same location as the old one, or else
          any and all mini-tramps will be pointing to invalid addresses in the child
          process! (the shm seg will appear to have moved) */

   /* question: we detach from the old seg; should we also destroy it?  No, certainly
      not, since paradynd still cares about measuring the old process. */
   /* note: we used to memcpy all data from the old seg to the new one.  Now, since
      we must detach from the old one before creating the new one, we can't do that.
      Instead. we let paradynd do that. */

   key_t new_shmSegKey;
   int new_shmSegShmId;
   void *new_attachedAtPtr;

   /* 1. Pick a new, unused, key: */
   pickNewShmSegBaseKey(&new_shmSegKey, &new_shmSegShmId);
      /* picks & creates new segment; doesn't attach to it though */

   shmsampling_printf("d-f makeNewShmSegsCopy: did pickNewShmSegBaseKey; chose key %d\n",
		      (int)new_shmSegKey);

   /* 2. Attach to the seg, IN THE SAME LOCATION AS THE OLD ONE.  In order to
         achieve this sleight of hand, we must detach from the old one first, and pass
         an address param to shm_attach.  Luckily, this doesn't require destroying the
	 old segment --- a good thing, since we mustn't do that (paradynd still cares
	 about measuring the parent!) */

   new_attachedAtPtr = shm_detach_reattach_overlap(new_shmSegShmId,
						   DYNINST_shmSegAttachedPtr);
   if (new_attachedAtPtr == (void*)-1) {
      /* total failure; could not attach */
      fprintf(stderr, "makeNewShmSegCopy: failed to attach to new segment at all.\n");
      abort();
   }
   else if (new_attachedAtPtr != NULL) {
      /* partial failure; could not attach in the same location; could only attach at a
	 different location.  For now at least, this is as bad as a total failure */
      fprintf(stderr, "makeNewShmSegCopy: could not attach to new segment in the same location as the old one.\n");
      abort();
   }

   shmsampling_printf("d-f makeNewShmSegsCopy pid=%d: attached to new seg key=%d IN SAME LOC!\n", (int)getpid(), new_shmSegKey);
   shmsampling_printf("d-f makeNewShmSegsCopy: also, detached from old seg.\n");

   /* 3. Copy data from the old shm seg the new one */
   /* (We don't [and can't] do this here any more, since we've already detached from the
      old segment.  It was always a toss-up whether to let paradynd do the memcpy.
      Now it must; and it does.) */

   /* 4. update important global variables; the key and shmid change; the sizes and the
         attached location doesn't change */
   DYNINST_shmSegKey = new_shmSegKey;
   DYNINST_shmSegShmId = new_shmSegShmId;
   /* DYNINST_shmSegAttachedPtr = new_shmSegAttachedPtr; MUSTN'T CHANGE */

   shmsampling_printf("d-f makeNewShmSegsCopy: done.\n");
}
#endif







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
    DYNINSTgenerateTraceRecord(0, TR_NEW_MEMORY, 
				sizeof(struct _traceMemory), 
                                &newRes, 1, 0.0, 0.0);
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


