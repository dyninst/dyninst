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

/* $Id: RTsolaris.c,v 1.25 1999/06/17 06:19:47 csserra Exp $ */

/************************************************************************
 * RTsolaris.c: clock access functions for solaris-2.
************************************************************************/

#include <signal.h>
#include <sys/ucontext.h>
#include <sys/time.h>
#include <assert.h>
#include <sys/syscall.h>

#include <sys/procfs.h> /* /proc PIOCUSAGE */
#include <stdio.h>
#include <fcntl.h> /* O_RDONLY */
#include <unistd.h> /* getpid() */

#include "rtinst/h/rtinst.h"

#if defined(SHM_SAMPLING) && defined(MT_THREAD)
#include <thread.h>
#endif

/*extern int    gettimeofday(struct timeval *, struct timezone *);*/
extern void perror(const char *);




/************************************************************************
 * symbolic constants.
************************************************************************/

static const double NANO_PER_USEC = 1.0e3;
static const double MILLION       = 1.0e6;




static int procfd = -1;

void DYNINSTgetCPUtimeInitialize(void) {
   /* This stuff is done just once */
   char str[20];

   sprintf(str, "/proc/%d", (int)getpid());
   /* have to use syscall here for applications that have their own
      versions of open, poll...In these cases there is no guarentee that
      things have been initialized so that the application's version of
      open can be used when this open call occurs (in DYNINSTinit)
   */
   procfd = syscall(SYS_open,str, O_RDONLY);
   if (procfd < 0) {
      fprintf(stderr, "open of /proc failed in DYNINSTgetCPUtimeInitialize\n");
      perror("open");
      abort();
   }
}


/************************************************************************
 * void DYNINSTos_init(void)
 *
 * os initialization function---currently null.
************************************************************************/

void
DYNINSTos_init(int calledByFork, int calledByAttach) {

    /*
       Install trap handler.
       This is currently being used only on the x86 platform.
    */
#ifdef i386_unknown_solaris2_5
    void DYNINSTtrapHandler(int sig, siginfo_t *info, ucontext_t *uap);
    struct sigaction act;
    act.sa_handler = DYNINSTtrapHandler;
    act.sa_flags = 0;
    sigfillset(&act.sa_mask);
    if (sigaction(SIGTRAP, &act, 0) != 0) {
        perror("sigaction(SIGTRAP)");
	assert(0);
	abort();
    }
#endif

    /* It is necessary to call DYNINSTgetCPUtimeInitialize here to make sure
       that it is called again for a child process during a fork - naim */
    DYNINSTgetCPUtimeInitialize();
}





/************************************************************************
 * time64 DYNINSTgetCPUtime(void)
 *
 * get the total CPU time used for "an" LWP of the monitored process.
 * this functions needs to be rewritten if a per-thread CPU time is
 * required.  time for a specific LWP can be obtained via the "/proc"
 * filesystem.
 * return value is in usec units.
 *
 * XXXX - This should really return time in native units and use normalize.
 *	conversion to float and division are way too expensive to
 *	do everytime we want to read a clock (slows this down 2x) -
 *	jkh 3/9/95
************************************************************************/


static unsigned long long div1000(unsigned long long in) {
   /* Divides by 1000 without an integer division instruction or library call, both of
    * which are slow.
    * We do only shifts, adds, and subtracts.
    *
    * We divide by 1000 in this way:
    * multiply by 1/1000, or multiply by (1/1000)*2^30 and then right-shift by 30.
    * So what is 1/1000 * 2^30?
    * It is 1,073,742.   (actually this is rounded)
    * So we can multiply by 1,073,742 and then right-shift by 30 (neat, eh?)
    *
    * Now for multiplying by 1,073,742...
    * 1,073,742 = (1,048,576 + 16384 + 8192 + 512 + 64 + 8 + 4 + 2)
    * or, slightly optimized:
    * = (1,048,576 + 16384 + 8192 + 512 + 64 + 16 - 2)
    * for a total of 8 shifts and 6 add/subs, or 14 operations.
    *
    */

   unsigned long long temp = in << 20; /* multiply by 1,048,576 */
   /* beware of overflow; left shift by 20 is quite a lot.
      If you know that the input fits in 32 bits (4 billion) then
      no problem.  But if it's much bigger then start worrying...
   */

   temp += in << 14; /* 16384 */
   temp += in << 13; /* 8192  */
   temp += in << 9;  /* 512   */
   temp += in << 6;  /* 64    */
   temp += in << 4;  /* 16    */
   temp -= in >> 2;  /* 2     */

   return (temp >> 30); /* divide by 2^30 */
}

static unsigned long long divMillion(unsigned long long in) {
   /* Divides by 1,000,000 without an integer division instruction or library call,
    * both of which are slow.
    * We do only shifts, adds, and subtracts.
    *
    * We divide by 1,000,000 in this way:
    * multiply by 1/1,000,000, or multiply by (1/1,000,000)*2^30 and then right-shift
    * by 30.  So what is 1/1,000,000 * 2^30?
    * It is 1,074.   (actually this is rounded)
    * So we can multiply by 1,074 and then right-shift by 30 (neat, eh?)
    *
    * Now for multiplying by 1,074
    * 1,074 = (1024 + 32 + 16 + 2)
    * for a total of 4 shifts and 4 add/subs, or 8 operations.
    *
    * Note: compare with div1000 -- it's cheaper to divide by a million than
    *       by a thousand (!)
    *
    */

   unsigned long long temp = in << 10; /* multiply by 1024 */
   /* beware of overflow...if the input arg uses more than 52 bits
      than start worrying about whether (in << 10) plus the smaller additions
      we're gonna do next will fit in 64...
   */

   temp += in << 5; /* 32 */
   temp += in << 4; /* 16 */
   temp += in << 1; /* 2  */

   return (temp >> 30); /* divide by 2^30 */
}

static unsigned long long mulMillion(unsigned long long in) {
   unsigned long long result = in;

   /* multiply by 125 by multiplying by 128 and subtracting 3x */
   result = (result << 7) - result - result - result;

   /* multiply by 125 again, for a total of 15625x */
   result = (result << 7) - result - result - result;

   /* multiply by 64, for a total of 1,000,000x */
   result <<= 6;

   /* cost was: 3 shifts and 6 subtracts
    * cost of calling mul1000(mul1000()) would be: 6 shifts and 4 subtracts
    *
    * Another algorithm is to multiply by 2^6 and then 5^6.
    * The former is super-cheap (one shift); the latter is more expensive.
    * 5^6 = 15625 = 16384 - 512 - 256 + 8 + 1
    * so multiplying by 5^6 means 4 shift operations and 4 add/sub ops
    * so multiplying by 1000000 means 5 shift operations and 4 add/sub ops.
    * That may or may not be cheaper than what we're doing (3 shifts; 6 subtracts);
    * I'm not sure.  --ari
    */

   return result;
}

time64
DYNINSTgetCPUtime(void) {
  hrtime_t lwpTime;
  time64 now;
  lwpTime = gethrvtime();
  now = div1000(lwpTime);  /* nsec to usec */
  return(now);  
}





/************************************************************************
 * time64 DYNINSTgetWalltime(void)
 *
 * get the total walltime used by the monitored process.
 * return value is in usec units.
************************************************************************/

time64
DYNINSTgetWalltime(void) {
  static time64 previous=0;
  time64 now;

  while (1) {
    struct timeval tv;
    if (gettimeofday(&tv,NULL) == -1) {
        perror("gettimeofday");
	assert(0);
        abort();
    }

    now = mulMillion(tv.tv_sec) + tv.tv_usec;
    /*    now = (time64)tv.tv_sec*(time64)1000000 + (time64)tv.tv_usec; */

    if (now < previous) continue;
    previous = now;
    return(now);
  }
}

#if defined(SHM_SAMPLING) && defined(MT_THREAD)
extern unsigned DYNINST_hash_lookup(unsigned key);
extern unsigned DYNINST_initialize_done;
extern void DYNINST_initialize_hash(unsigned total);
extern void DYNINST_initialize_free(unsigned total);
extern unsigned DYNINST_hash_insert(unsigned k);

int DYNINSTthreadSelf(void) {
  return(thr_self());
}

int DYNINSTthreadPos(void) {
  if (initialize_done) {
    return(DYNINST_hash_lookup(DYNINSTthreadSelf()));
  } else {
    DYNINST_initialize_free(MAX_NUMBER_OF_THREADS);
    DYNINST_initialize_hash(MAX_NUMBER_OF_THREADS);
    DYNINST_initialize_done=1;
    return(DYNINST_hash_insert(DYNINSTthreadSelf()));
  }
}
#endif


/****************************************************************************
   The trap handler. Currently being used only on x86 platform.

   Traps are used when we can't insert a jump at a point. The trap
   handler looks up the address of the base tramp for the point that
   uses the trap, and set the pc to this base tramp.
   The paradynd is responsible for updating the tramp table when it
   inserts instrumentation.
*****************************************************************************/

#ifdef i386_unknown_solaris2_5
trampTableEntry DYNINSTtrampTable[TRAMPTABLESZ];
unsigned DYNINSTtotalTraps = 0;

static unsigned lookup(unsigned key) {
    unsigned u;
    unsigned k;
    for (u = HASH1(key); 1; u = (u + HASH2(key)) % TRAMPTABLESZ) {
      k = DYNINSTtrampTable[u].key;
      if (k == 0)
        return 0;
      else if (k == key)
        return DYNINSTtrampTable[u].val;
    }
    /* not reached */
    assert(0);
    abort();
}

void DYNINSTtrapHandler(int sig, siginfo_t *info, ucontext_t *uap) {
    unsigned pc = uap->uc_mcontext.gregs[PC];
    unsigned nextpc = lookup(pc);

    if (!nextpc) {
      /* kludge: maybe the PC was not automatically adjusted after the trap */
      /* this happens for a forked process */
      pc--;
      nextpc = lookup(pc);
    }

    if (nextpc) {
      uap->uc_mcontext.gregs[PC] = nextpc;
    } else {
      assert(0);
      abort();
    }
    DYNINSTtotalTraps++;
}
#endif



