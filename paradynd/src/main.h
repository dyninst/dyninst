
#ifndef MAIN_H
#define MAIN_H

/* 
 * $Log: main.h,v $
 * Revision 1.5  1995/12/15 22:26:51  mjrg
 * Merged paradynd and paradyndPVM
 * Get module name for functions from symbol table in solaris
 * Fixed code generation for multiple instrumentation statements
 * Changed syntax of MDL resource lists
 *
 * Revision 1.4  1995/11/22 00:02:30  mjrg
 * Updates for paradyndPVM on solaris
 * Fixed problem with wrong daemon getting connection to paradyn
 * Removed -f and -t arguments to paradyn
 * Added cleanUpAndExit to clean up and exit from pvm before we exit paradynd
 * Fixed bug in my previous commit
 *
 * Revision 1.3  1995/02/16  08:53:39  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.2  1995/02/16  08:33:41  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.1  1994/11/01  16:58:03  markc
 * Prototypes
 *
 */

#include "comm.h"

extern pdRPC *tp;

#ifdef PARADYND_PVM
#include "pvm_support.h"

extern bool pvm_running;
#endif

// Cleanup for pvm and exit.
// This function must be called when we exit, to clean up and exit from pvm.
inline void cleanUpAndExit(int status) {
#ifdef PARADYND_PVM
  if (pvm_running)
    PDYN_exit_pvm();
#endif
  P_exit(status);
}


#endif
