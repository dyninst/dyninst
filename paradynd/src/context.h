
#ifndef CONTEXT_HDR
#define CONTEXT_HDR

/*
 * $Log: context.h,v $
 * Revision 1.4  1996/05/08 23:54:39  mjrg
 * added support for handling fork and exec by an application
 * use /proc instead of ptrace on solaris
 * removed warnings
 *
 * Revision 1.3  1994/11/10  18:57:50  jcargill
 * The "Don't Blame Me Either" commit
 *
 * Revision 1.2  1994/11/09  18:39:56  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.1  1994/11/02  11:02:55  markc
 * Prototypes for context.C
 *
 */

#include "rtinst/h/trace.h"
#include "dyninst.h"

extern timeStamp startPause;
extern timeStamp elapsedPauseTime;
extern void forkProcess(traceFork *fr);

#endif
