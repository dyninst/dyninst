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
 * $Id: RTwinnt.c,v 1.6 2003/02/04 14:59:37 bernat Exp $
 * RTwinnt.c: runtime instrumentation functions for Windows NT
 ************************************************************************/
#ifndef mips_unknown_ce2_11 //ccw 29 mar 2001
#include <assert.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#ifndef mips_unknown_ce2_11 //ccw 29 mar 2001
#include <mmsystem.h>
#include <errno.h>
#endif
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"

/************************************************************************
 * void DYNINSTbreakPoint(void)
 *
 * stop oneself.
************************************************************************/

void
DYNINSTbreakPoint(void) {
  /* TODO: how do we stop all threads? */
  DebugBreak();
}


void DYNINSTos_init(int calledByFork, int calledByAttach)
{
#ifndef mips_unknown_ce2_11 //ccw 23 july 2001
  RTprintf("DYNINSTos_init(%d,%d)\n", calledByFork, calledByAttach);
#endif
}

char gLoadLibraryErrorString[ERROR_STRING_LENGTH];
int DYNINSTloadLibrary(char *libname)
{
    HMODULE res;
    gLoadLibraryErrorString[0] = '\0';
    fprintf(stderr, "Attempting to load %s\n", libname);
    
    res = LoadLibrary(libname);
    if (res == NULL) {
        perror("DYNINSTloadLibrary");
        return 0;
    }
    return 1;
}
