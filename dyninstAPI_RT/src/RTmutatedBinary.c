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

/* $Id: RTmutatedBinary.c,v 1.8 2005/04/05 16:45:22 jodom Exp $ */

/* this file contains the code to restore the necessary
   data for a mutated binary 
 */


#include <unistd.h>
#include <stdlib.h>

extern int isMutatedExec;
char *buffer;

/* checkMutatedFile() is defined in RTmutatedBinary_<fileformat>.c */

extern int checkMutatedFile();

/*
 * This function sets up pre-initialization
 * data structures for SaveTheWorld. Ensure
 * it is called _before_ DYNINSTinit
 */ 
void RTmutatedBinary_init(){

/* this buffer is allocated to clear
   the first page on the heap. This is necessary
   because loading the heap tramps uses mmap, which
   is going to eat the heap if the heap begins on 
   the same page the heap tramps end on (almost certain)
*/
    /* Call-once protection */
    static int init = 0;
    
    if (!init) {
        buffer = (char*) malloc(getpagesize());
        isMutatedExec =checkMutatedFile();
        free(buffer);
        init++;
    }
    
}

