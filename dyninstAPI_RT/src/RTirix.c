/*
 * Copyright (c) 1998 Barton P. Miller
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
 * $Id: RTirix.c,v 1.7 2002/08/09 23:32:38 jaw Exp $
 * RTirix.c: mutatee-side library function specific to IRIX
 ************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>                 /* open() */
#include <fcntl.h>                    /* open() */
#include <unistd.h>                   /* procfs */
#include <sys/procfs.h>               /* procfs */
#include <sys/mman.h>                 /* mmap() */
#include <dlfcn.h>                    /* dlopen() */
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"

/************************************************************************
 * EXPORTED SYMBOLS:
 *   void   DYNINSTos_init(int, int): OS-specific initialization
 *   void  *DYNINSTloadLibrary(char *): dlopen wrapper
 * (below symbols implemented in "RTheap.c" and "RTheap-irix.c")
 *   void  *DYNINSTos_malloc(size_t, void *, void *): heap allocation
 *   int    DYNINSTos_free(void *): heap deallocation
 ************************************************************************/

void DYNINSTos_init(int calledByFork, int calledByAttach)
{
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
    printf("%s[%d]:  DYNINSTloadLibrary -- dlopen\n", __FILE__, __LINE__);
    fflush(NULL);

    if (NULL != (err_str = dlerror()))
      strncpy(gLoadLibraryErrorString, err_str, ERROR_STRING_LENGTH);
    else 
      sprintf(gLoadLibraryErrorString,"unknown error with dlopen");
    
    //fprintf(stderr, "%s[%d]: %s\n",__FILE__,__LINE__,gLoadLibraryErrorString);
    return 0;  
  } else {
    return 1;
  }
