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

/*
 * $Id: RTrexec.c,v 1.7 2004/03/23 01:12:43 eli Exp $
 * Code to trap rexec call and munge command.
 */
#include <sys/types.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <strings.h>

char *realCommand;

int DYNINSTrexec(char *cmd)
{
     char *pdArgs;

     pdArgs = (char *) getenv("PARADYN_MASTER_INFO");
     if (!pdArgs) {
	 printf("unable to get PARADYN_MASTER_INFO\n");
	 return (-1);
     }
     realCommand = (char *)  malloc(strlen(pdArgs)+strlen(cmd)+20);
     sprintf(realCommand, "paradynd %s -runme %s", pdArgs, cmd);
     printf("Instrumented rexec. This call is not currently supported\n");
     printf("CMD = %s\n", realCommand);

     /* XXX - HACK !!! */
     /* Get Back previous argument list */
     /* We need to load the 5th argument to rexec with our version of cmd */
#if 0
     /* Disable for now, since we don't actually use this code 
	( and it's getting in the way of compiling with xlc) */
#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
     {
	 register int temp asm("r10");
	 register volatile char *cmdPtr asm("r9") = realCommand;
	 asm("oriu  10, 1, 0");
	 asm("cal 1, 80(1)");	/* restore old stack pointer */
	 asm("ai 1, 1, 184");	/* add previous offset back */
	 asm("st 9, -36(1)");	/* get correct value */
	 asm("oriu  1, 10, 0");
      }
#endif
#endif
     return(0);
}
