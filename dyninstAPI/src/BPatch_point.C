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

#include <stdio.h>
#ifdef rs6000_ibm_aix4_1
#include <memory.h>
#endif

#include "BPatch_point.h"
#include "BPatch_snippet.h"
#include "process.h"
#include "symtab.h"
#include "instPoint.h"


/*
 * BPatch_point::getCalledFunction
 *
 * For a BPatch_point representing a call site, returns a pointer to a
 * BPatch_function that represents the function being called.  If the point
 * isn't a call site, returns NULL.
 */
BPatch_function *BPatch_point::getCalledFunction()
{
    assert(point);

    // XXX Should get rid of the machine-dependent stuff here
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)
    if (point->ipType != callSite)
	return NULL;
#elif defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
    if (point->ipLoc != ipFuncCallPoint)
	return NULL;
#elif defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0)
    if (!point->insnAtPoint().isCall())
	return NULL;
#endif

    function_base *func;
    if (!proc->findCallee(*point, func))
    	return NULL;

    if (func != NULL)
    	return new BPatch_function(proc, func);
    else
	return NULL;
}


/*
 * BPatch_point::getAddress
 *
 * Returns the original address of the first instruction at this point.
 */
void *BPatch_point::getAddress()
{
    return point->iPgetAddress();
}


/*
 * BPatch_point::getDisplacedInstructions
 *
 * Returns the instructions to be relocated when instrumentation is inserted
 * at this point.  Returns the number of bytes taken up by these instructions.
 *
 * maxSize	The maximum number of bytes of instructions to return.
 * insns	A pointer to a buffer in which to return the instructions.
 */
int BPatch_point::getDisplacedInstructions(int maxSize, void *insns)
{
#ifdef rs6000_ibm_aix4_1
    if (maxSize >= sizeof(instruction))
	memcpy(insns, &point->originalInstruction.raw, sizeof(instruction));

    return sizeof(instruction);
#else
    // Not implemented except on AIX
    return -1;
#endif
}


/*
 * BPatch_point::usesTrap_NP
 *
 * Returns true if this point is or would be instrumented with a trap, rather
 * than a jump to the base tramp, false otherwise.  On platforms that do not
 * use traps (everything other than x86), it always returns false;
 */
bool BPatch_point::usesTrap_NP()
{
#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0)
    assert(point);
    assert(proc);

    return point->usesTrap(proc);
#else
    return false;
#endif
}
