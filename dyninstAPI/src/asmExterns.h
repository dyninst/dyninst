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


/* 
 * $Id: asmExterns.h,v 1.1 2001/08/01 15:39:55 chadd Exp $
*/

//ccw 3 aug 2000 
//this file holds the externs for the assembly code. this allows
//the CE client easy access to it and the NT box as well.
//it is included in baseTrampTemplate.h when appropriate.

extern "C" void baseTramp();
extern "C" void baseTramp_savePreInsn();
extern "C" void baseTramp_skipPreInsn();
extern "C" void baseTramp_globalPreBranch();
extern "C" void baseTramp_localPreBranch();
extern "C" void baseTramp_localPreReturn();
extern "C" void baseTramp_updateCostInsn();
extern "C" void baseTramp_restorePreInsn();
extern "C" void baseTramp_emulateInsn();
extern "C" void baseTramp_skipPostInsn();
extern "C" void baseTramp_savePostInsn();
extern "C" void baseTramp_globalPostBranch();
extern "C" void baseTramp_localPostBranch();
extern "C" void baseTramp_localPostReturn();
extern "C" void baseTramp_restorePostInsn();
extern "C" void baseTramp_returnInsn();
extern "C" void baseTramp_endTramp();

extern "C" void baseNonRecursiveTramp();
extern "C" void baseNonRecursiveTramp_savePreInsn();
extern "C" void baseNonRecursiveTramp_skipPreInsn();
extern "C" void baseNonRecursiveTramp_guardOnPre_begin();
extern "C" void baseNonRecursiveTramp_guardOnPre_end();
extern "C" void baseNonRecursiveTramp_globalPreBranch();
extern "C" void baseNonRecursiveTramp_localPreBranch();
extern "C" void baseNonRecursiveTramp_localPreReturn();
extern "C" void baseNonRecursiveTramp_guardOffPre_begin();
extern "C" void baseNonRecursiveTramp_guardOffPre_end();
extern "C" void baseNonRecursiveTramp_updateCostInsn();
extern "C" void baseNonRecursiveTramp_restorePreInsn();
extern "C" void baseNonRecursiveTramp_emulateInsn();
extern "C" void baseNonRecursiveTramp_skipPostInsn();
extern "C" void baseNonRecursiveTramp_savePostInsn();
extern "C" void baseNonRecursiveTramp_guardOnPost_begin();
extern "C" void baseNonRecursiveTramp_guardOnPost_end();
extern "C" void baseNonRecursiveTramp_globalPostBranch();
extern "C" void baseNonRecursiveTramp_localPostBranch();
extern "C" void baseNonRecursiveTramp_localPostReturn();
extern "C" void baseNonRecursiveTramp_guardOffPost_begin();
extern "C" void baseNonRecursiveTramp_guardOffPost_end();
extern "C" void baseNonRecursiveTramp_restorePostInsn();
extern "C" void baseNonRecursiveTramp_returnInsn();
extern "C" void baseNonRecursiveTramp_endTramp();
