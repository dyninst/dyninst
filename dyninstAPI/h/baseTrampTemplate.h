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
 * $Id: baseTrampTemplate.h,v 1.1 2001/08/01 15:39:52 chadd Exp $
 */
//ccw 2 aug 2000
//created to allow many files to include this
//w/o the overhead of inst-mips.h!

#ifndef baseTrampTemplate__
#define baseTrampTemplate__
#include "common/h/Types.h"

#ifdef mips_unknown_ce2_11 //ccw 2 aug 2000
//in order to get the mips assembly code on the NT box 
//where it needs to be to produce the trampolines,
//the assembly is loaded by the CE client, and passed back
//up to the NT box.  In that case, the assembly functions are
//just defined as Addresses, set when the system initializes.
//
//these values are set by remoteDevice::remoteDevice in file remoteDevice.C
//

extern char *baseTrampMem; //where the allocated memory will go.
extern char *baseNonRecursiveTrampMem;//the the NonRecursive code goes.

extern Address baseTramp;
extern Address baseTemplate_savePreInsOffset;
extern Address baseTemplate_skipPreInsOffset;
extern Address baseTemplate_globalPreOffset;
extern Address baseTemplate_localPreOffset;
extern Address baseTemplate_localPreReturnOffset;
extern Address baseTemplate_updateCostOffset;
extern Address baseTemplate_restorePreInsOffset;
extern Address baseTemplate_emulateInsOffset;
extern Address baseTemplate_skipPostInsOffset;
extern Address baseTemplate_savePostInsOffset;
extern Address baseTemplate_globalPostOffset;
extern Address baseTemplate_localPostOffset;
extern Address baseTemplate_localPostReturnOffset;
extern Address baseTemplate_restorePostInsOffset;
extern Address baseTemplate_returnInsOffset;

extern Address baseTemplate_trampTemp;
extern Address baseTemplate_size;
extern Address baseTemplate_cost;
extern Address baseTemplate_prevBaseCost;
extern Address baseTemplate_postBaseCost;
extern Address baseTemplate_prevInstru;
extern Address baseTemplate_postInstru;
extern Address baseTramp_endTramp;


/////nonRecursive!
extern Address baseNonRecursiveTramp;

extern Address nonRecursiveBaseTemplate_guardOffPost_beginOffset;
extern Address nonRecursiveBaseTemplate_savePreInsOffset;
extern Address nonRecursiveBaseTemplate_skipPreInsOffset;
extern Address nonRecursiveBaseTemplate_globalPreOffset ;
extern Address nonRecursiveBaseTemplate_localPreOffset;
extern Address nonRecursiveBaseTemplate_localPreReturnOffset;
extern Address nonRecursiveBaseTemplate_updateCostOffset;
extern Address nonRecursiveBaseTemplate_restorePreInsOffset;
extern Address nonRecursiveBaseTemplate_emulateInsOffset;
extern Address nonRecursiveBaseTemplate_skipPostInsOffset;
extern Address nonRecursiveBaseTemplate_savePostInsOffset;
extern Address nonRecursiveBaseTemplate_globalPostOffset;
extern Address nonRecursiveBaseTemplate_localPostOffset;
extern Address nonRecursiveBaseTemplate_localPostReturnOffset;
extern Address nonRecursiveBaseTemplate_restorePostInsOffset;
extern Address nonRecursiveBaseTemplate_returnInsOffset;
extern Address nonRecursiveBaseTemplate_guardOnPre_beginOffset;
extern Address nonRecursiveBaseTemplate_guardOffPre_beginOffset;
extern Address nonRecursiveBaseTemplate_guardOnPost_beginOffset;
extern Address nonRecursiveBaseTemplate_guardOffPost_beginOffset;
extern Address nonRecursiveBaseTemplate_guardOnPre_endOffset;
extern Address nonRecursiveBaseTemplate_guardOffPre_endOffset;
extern Address nonRecursiveBaseTemplate_guardOnPost_endOffset;
extern Address nonRecursiveBaseTemplate_guardOffPost_endOffset;
extern Address nonRecursiveBaseTemplate_trampTemp;
extern Address nonRecursiveBaseTemplate_size;
extern Address nonRecursiveBaseTemplate_cost;
extern Address nonRecursiveBaseTemplate_prevBaseCost;
extern Address nonRecursiveBaseTemplate_postBaseCost;
extern Address nonRecursiveBaseTemplate_prevInstru;
extern Address nonRecursiveBaseTemplate_postInstru;

#else

// baseTramp assembly code symbols
#include "asmExterns.h"

#endif

#endif
