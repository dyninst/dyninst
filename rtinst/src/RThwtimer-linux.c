/*
 * Copyright (c) 1996-2001 Barton P. Miller
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

/* $Id: RThwtimer-linux.c,v 1.2 2001/10/08 20:51:44 zandy Exp $ */

/************************************************************************
 * RThwtimer-linux.c: linux hardware level timer support functions
************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include "rtinst/h/RThwtimer-linux.h"

void int_handler(int errno);

static volatile sig_atomic_t jumpok = 0;
static sigjmp_buf jmpbuf;

void int_handler(int errno)  {
  /* removes warning */  errno = errno;
  if(jumpok == 0)  return;
  siglongjmp(jmpbuf, 1);
}

int isTSCAvail(void)  {
  struct sigaction act, oldact;
  rawTime64 v;
  int retVal=0;
  act.sa_handler = int_handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  if(sigaction(SIGSEGV, &act, &oldact) < 0)  {
    perror("isTSCAvail: Error setting up SIGSEGV handler");
    exit(1);
   }

  if(sigsetjmp(jmpbuf, 1)==0)  {
    jumpok = 1;
    getTSC(v);
    retVal=1;
  } else {
    retVal=0;
  }
  if(sigaction(SIGSEGV, &oldact, NULL) < 0) {
    perror("isTSCAvail: Error resetting the SIGSEGV handler");
    exit(1);
  }
  return retVal;
}


#ifdef HRTIME
#include <unistd.h>

int isLibhrtimeAvail(struct hrtime_struct **hr_cpu_link, int pid) {
  int error;
  
  if(! isTSCAvail()) {
    return 0;
  }
  
  error = hrtime_init();
  if (error < 0) {
    return 0;
  }

  error = get_hrtime_struct(pid, hr_cpu_link);
  if (error < 0) {
    return 0;
  }
  return 1;
}

// this matches the function in RThwtimer-x86.h, used for non-optimizing
// rtinst library in which case the extern inline function is ignored 
rawTime64 hrtimeGetVtime(struct hrtime_struct *hr_cpu_link) {
  hrtime_t current;
  get_hrvtime(hr_cpu_link, &current);
  return (rawTime64)(current);
}

#endif



