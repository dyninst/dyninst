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

// $Id: instPoint-ia64.h,v 1.4 2002/08/23 01:56:03 tlmiller Exp $

#ifndef _INST_POINT_IA64_H_
#define _INST_POINT_IA64_H_

#include "dyninstAPI/src/symtab.h"

class instPoint {
	public:
		instPoint( Address addr, pd_Function * pdfn, const image * owner, IA64_instruction * theInsn );

		const function_base * iPgetCallee() const {
			return myCallee;
			} /* required by linux.C */

		const image * iPgetOwner() const { 
			return (myPDFunction) ? ( (myPDFunction->file()) ? myPDFunction->file()->exec() : NULL ) : NULL;
			} /* required by linux.C */

		Address getTargetAddress() {
			return myTargetAddress;
			} /* required by linux.C */

		void set_callee( pd_Function * callee ) {
			myCallee = callee;
			} /* required by linux.C */

		Address iPgetAddress() const {
			return myAddress;
			} /* required by func-reloc.C */

		Address firstAddress() const {
			assert( 0 );
			return 0;
			} /* required by func-reloc.C */

		Address followingAddress() const {
			assert( 0 );
			return 0;
			} /* required by func-reloc.C */

		Address insnAddress() const {
			assert( 0 );
			return 0;
			} /* required by func-reloc.C */

		const function_base * iPgetFunction() const {
			return myPDFunction;
			} /* required by inst.C */

#ifdef BPATCH_LIBRARY
	private:
		// We need this here because BPatch_point gets dropped before
		// we get to generate code from the AST, and we glue info needed  
		// to generate code for the effective address snippet/node to the
		// BPatch_point rather than here.
		friend class BPatch_point;

		// unfortunately the correspondig BPatch_point
		// is created afterwards, so it needs to set this
		BPatch_point *bppoint; 

	public:
		const BPatch_point* getBPatch_point() const { return bppoint; }
#endif

	private:

		Address myAddress;
		pd_Function * myPDFunction;
		const image * myOwner;
		IA64_instruction * myInstruction;

		pd_Function * myCallee;			/* if I'm a call */
		Address myTargetAddress;		/* if I'm a call or branch */

	}; /* end class instPoint */

#endif
