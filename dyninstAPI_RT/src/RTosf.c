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
 * RTosf.c: mutatee-side library function specific to OSF
************************************************************************/
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#if !defined (EXPORT_SPINLOCKS_AS_HEADER)
/* everything should be under this flag except for the assembly code
   that handles the runtime spinlocks  -- this is imported into the
   test suite for direct testing */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dlfcn.h>                    /* dlopen() */

/* The alpha does not have a divide instruction */
/* Division is emulated in software */
int divide(int a,int b)
{
  return (a/b);
}

char gLoadLibraryErrorString[ERROR_STRING_LENGTH];
int DYNINSTloadLibrary(char *libname)
{
  void *res;
  char *err_str;
  gLoadLibraryErrorString[0]='\0';
  
  if (NULL == (res = dlopen(libname, RTLD_NOW | RTLD_GLOBAL))) {
    // An error has occurred
    perror( "DYNINSTloadLibrary -- dlopen" );
    
    if (NULL != (err_str = dlerror()))
      strncpy(gLoadLibraryErrorString, err_str, ERROR_STRING_LENGTH);
    else 
      sprintf(gLoadLibraryErrorString,"unknown error with dlopen");
    
    //fprintf(stderr, "%s[%d]: %s\n",__FILE__,__LINE__,gLoadLibraryErrorString);
    return 0;  
  } else
    return 1;
}

void DYNINSTos_init(int calledByFork, int calledByAttach)
{
}
#endif /* EXPORT SPINLOCKS */
void DYNINSTlock_spinlock(dyninst_spinlock *mut)
{

 asm (
         "  ldq         $3, 16($15)  # R3 <- mut\n"

         "  1:                       # Loop 1: try to get and store lock\n"
         "  ldl_l       $4, 0($3)    # R4 <- mut->lock, (locked operation)\n"
         "  blbs        $4, 2f       # if lock bit set, spin in loop 2\n"
         "  bne         $4, 1b       # R4 != 0, someone else has lock, spin.\n"
         "  addl        $4, 1, $4    # R4 == 0, add 1 ( 1 = mutex locked)\n"
         "  stl_c       $4, 0($3)    # R4 -> mut->lock\n"
         "  beq         $4, 2f       # if R4 == 0, store failed, lock contention, spin\n"
         "                           # in Loop2 until lock is released\n"
         "  br          3f           # have lock, jump to end. \n"

         "  2:                       # Loop 2: spin until lock bit unset\n"
         "  ldl         $4, 0($3)    # R4 <- mut->lock\n"
         "  blbs        $4, 2b       # if lock bit set, spin in loop 2\n"
         "  br          1b           # if lock bit unset, start over in Loop\n"

         "  3:          mb           # memory barrier, and we're done\n"
     );



}
