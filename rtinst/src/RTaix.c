/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 * 
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */



/************************************************************************
 * RTaix.c: clock access functions for aix.
 *
 * $Log: RTaix.c,v $
 * Revision 1.3  1996/02/13 16:21:57  hollings
 * Fixed timer64 to be time64.
 *
 *
 ************************************************************************/

#include <sys/time.h>
#include <sys/resource.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include "rtinst/h/rtinst.h"


static const double NANO_PER_USEC = 1.0e3;
static const long int MILLION       = 1000000;

/************************************************************************
 * void DYNINSTos_init(void)
 *
 * os initialization function---currently null.
************************************************************************/

void
DYNINSTos_init(void) {
}


/************************************************************************
 * time64 DYNINSTgetCPUtime(void)
 *
 * return value is in usec units.
************************************************************************/

time64
DYNINSTgetCPUtime(void) {
     time64 now;
     static time64 previous=0;
     struct rusage ru;

try_again:    
    if (!getrusage(RUSAGE_SELF, &ru)) {
      now = (time64)ru.ru_utime.tv_sec + (time64)ru.ru_stime.tv_sec;
      now *= (time64)1000000;
      now += (time64)ru.ru_utime.tv_usec + (time64)ru.ru_stime.tv_usec;
      if (now<previous) {
        goto try_again;
      }
      previous=now;
      return(now);
    }
    else {
      perror("getrusage");
      abort();
    }
}





/************************************************************************
 * time64 DYNINSTgetWalltime(void)
 *
 * get the total walltime used by the monitored process.
 * return value is in usec units.
************************************************************************/

time64 DYNINSTgetWalltime(void) {
    time64 now;
    register unsigned int timeSec asm("5");
    register unsigned int timeNano asm("6");
    register unsigned int timeSec2 asm("7");

    /* Need to read the first value twice to make sure it doesn't role
     *   over while we are reading it.
     */
retry:
    asm("mfspr   5,4");		/* read high into register 5 - timeSec */
    asm("mfspr   6,5");		/* read low into register 6 - timeNano */
    asm("mfspr   7,4");		/* read high into register 7 - timeSec2 */

    if (timeSec != timeSec2) goto retry;
    /* convert to correct form. */
    now = (time64)timeSec;
    now *= (time64)MILLION;
    now += (time64)timeNano/(time64)1000;
    return(now);
}


/*
 * Code to trap execvp call and munge command (for SP-2).
 *
 */
void DYNINSTexecvp(char *argv[])
{
     int i;
     int ret;
     char *ch;
     char *cmd;
     int iCount;
     int acount;
     char *pdArgs;
     char **newArgs;
     static int inExecvp;

     if (inExecvp) return;
     inExecvp = 1;

     cmd = argv[0];

     /* this only applies to poe on the SP-2 */
     if (strcmp(cmd, "poe")) return;

     for (iCount=0; argv[iCount]; iCount++);

     pdArgs = (char *) getenv("PARADYN_MASTER_INFO");
     if (!pdArgs) {
	 fprintf(stdout, "unable to get PARADYN_MASTER_INFO\n");
	 fflush(stdout);
	 return;
     }

     /* extras for first arg, command, -runme, and null  */
     for (ch=pdArgs, acount=4; *ch; ch++) if (*ch == ' ') acount++;
     newArgs = calloc(sizeof(char*), iCount+acount);

     newArgs[0] = "poe";
     newArgs[1] = "paradynd";

     /* skip white spave at start */
     while (*pdArgs && *pdArgs == ' ') pdArgs++;
     newArgs[2] = pdArgs;
     for (ch=pdArgs, acount=3; *ch; ch++) {
	 if (*ch == ' ') {
	     *ch = '\0';
	     /* skip over null argument -caused by spaces in environment var */
	     if (!strlen(newArgs[acount-1])) acount--;
	     newArgs[acount++] = ++ch;
	 }
     }
     /* skip over null argument -caused by space at end of environment var */
     if (!strlen(newArgs[acount-1])) acount--;

     newArgs[acount++] = "-runme";
     for (i=1; i < iCount; i++) {
	 newArgs[acount++] = argv[i];
     }

     newArgs[acount] = "";

     /* generate an exit record about the process to paradynd */
     DYNINSTprintCost();

     /* Now call execvp with the correct arguments */
     ret = execvp(cmd, newArgs);

     fprintf(stderr, "execvp failed\n");
     perror("execvp");
     fflush(stderr);

     exit(-1);

     inExecvp = 0;
     return;
}


/*
 * DYNINSTgetRusage(id) - Return the value of various OS stats.
 *
 *    The id is an integer, and when they are changed, any metric that uses
 *        DYNINSTgetRusage will also need to be updated.
 *
 */
int DYNINSTgetRusage(int id)
{
    int ret;
    int value;
    struct rusage rusage;
    struct rusage *DYNINSTrusagePtr;

    ret = getrusage(RUSAGE_SELF, &rusage);
    if (ret) {
	perror("getrusage");
    }
    DYNINSTrusagePtr = &rusage;
    switch (id) {
	case 0:	/* page faults */
	    value = DYNINSTrusagePtr->ru_minflt+DYNINSTrusagePtr->ru_majflt;
	    break;
	case 1:	/* swaps */
	    value = DYNINSTrusagePtr->ru_nswap;
	    break;
	case 2: /* signals received */
	    value = DYNINSTrusagePtr->ru_nsignals;
	    break;
	case 3: /* max rss */
	    value = DYNINSTrusagePtr->ru_maxrss;
	    break;
	case 4: /* context switches */
	    value = DYNINSTrusagePtr->ru_nvcsw + DYNINSTrusagePtr->ru_nivcsw;
	    break;
	default:
	    value = 0;
	    break;
    }
    return value;
}
