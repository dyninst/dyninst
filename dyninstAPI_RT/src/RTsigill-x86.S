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

/*
   DYNINSTsigill:
	
   DYNINSTsigill executes an illegal instruction, causing SIGILL to be
   sent to the calling process.  It is only needed for
   detach-on-the-fly, where detached mutatees stop to wait for the
   mutator or paradynd to reattach by sending themselves SIGILL.

   A detached mutatee that stops itself with SIGSTOP will not be
   noticed by the mutator or paradynd.  It must send SIGILL to be
   noticed.
    
   There are two ways to send SIGILL to yourself: call kill() or
   execute an illegal instruction.

   We chose to use illegal instructions. The SIGILL handler has a
   simpler implementation if SIGILL is caused by an illegal
   instruction.  We do not use kill() to send SIGILL.

   The test suite calls this function through the dynamic loader to
   avoid duplication of code in the test suite and additional makefile
   complexity.
    
   3000 bytes of source code for 3 bytes of object code.
*/

    .text
    .globl DYNINSTsigill

DYNINSTsigill:
    ud2
    ret
