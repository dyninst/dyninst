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
 * inst-ia64.C - ia64 dependent functions and code generator
 * $Id: inst-ia64.C,v 1.8 2002/06/18 19:48:14 tlmiller Exp $
 */

/* Note that these should all be checked for (linux) platform
   independence and the possibility of refactoring their originals
   into in- and de-pendent parts. */

#include <iomanip.h>
#include <limits.h>

#include "common/h/headers.h"

#ifndef BPATCH_LIBRARY
#include "rtinst/h/rtinst.h"
#endif
#include "common/h/Dictionary.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/stats.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/showerror.h"

#include "dyninstAPI/src/arch-ia64.h"
#include "dyninstAPI/src/inst-ia64.h"
#include "dyninstAPI/src/instPoint.h"	// includes instPoint-ia64.h
#include "dyninstAPI/src/instP.h"	// class returnInstance

// for function relocation
#include "dyninstAPI/src/func-reloc.h" 
#include "dyninstAPI/src/LocalAlteration.h"

/* Required by process, ast .C, et al */
registerSpace * regSpace;

/* Required by ast.C */
void emitVload( opCode op, Address src1, Register src2, Register dest,
		char * ibuf, Address & base, bool noCost, int size ) {}

/* Required by ast.C */
Register emitFuncCall( opCode op, registerSpace * rs, char * ibuf,
			Address & base, const vector<AstNode *> & operands,
			const string & callee, process * proc,
			bool noCost, const function_base * calleefunc, 
			const instPoint *location) { return 0; }

/* Required by symtab.C */
void pd_Function::checkCallPoints() { }

/* Required by symtab.C */
bool pd_Function::findInstPoints(const image *i_owner) { return true; }

/* Required by func-reloc.C */
LocalAlteration *fixOverlappingAlterations( LocalAlteration * alteration, LocalAlteration * tempAlteration ) { return NULL; }

/* Required by LocalAlteration.C */
int InsertNops::sizeOfNop() { return 0; }

/* Required by func-reloc.C */
bool InsertNops::RewriteFootprint( Address oldBaseAdr, Address & oldAdr,
				Address newBaseAdr, Address & newAdr,
				instruction oldInstructions[],
				instruction newInstructions[],
				int & oldOffset, int & newOffset,
				int newDisp, unsigned & codeOffset,
				unsigned char *code ) { return false; }

/* Required by func-reloc.C */
bool ExpandInstruction::RewriteFootprint( Address oldBaseAdr, Address & oldAdr,
		Address newBaseAdr, Address & newAdr,
		instruction oldInstructions[], instruction newInstructions[],
		int &oldOffset, int &newOffset, int newDisp,
		unsigned &codeOffset, unsigned char* code ) { return false; }

/* Required by ast.C */
void emitFuncJump(opCode op, char * i, Address & base,
			const function_base * callee, process * proc ) { }

Register emitFuncCall(opCode op, registerSpace * rs,
                      char * ibuf, Address & base,
                      const vector<AstNode *> &operands,
                      const string & callee, process * proc,
                      bool noCost, const function_base * calleefunc,  
                      const vector<AstNode *> & ifForks,
                      const instPoint * location) { }


/* Required by BPatch_ function, image .C */
BPatch_point *createInstructionInstPoint( process * proc, void * address,
			BPatch_point ** alternative, BPatch_function * bpf ) { return NULL; }

/* Required by ast.C */
void emitASload( BPatch_addrSpec_NP as, Register dest, char * baseInsn,
			Address & base, bool noCost ) { }

/* FIXME: who actually need this? */
bool PushEIP::RewriteFootprint( Address oldBaseAdr, Address & oldAdr,
		Address newBaseAdr, Address & newAdr,
		instruction oldInstructions[], instruction newInstructions[],
		int & oldInsnOffset, int & newInsnOffset,
		int newDisp, unsigned & codeOffset, unsigned char * code ) { return false; }

/* Required by process.C */
void instWaitingList::cleanUp( process *, Address addr ) { }

/* Required by BPatch_thread.C */
bool process::replaceFunctionCall( const instPoint * point, 
			const function_base * func ) { return false; }

/* Required by ast.C */
void emitV( opCode op, Register src1, Register src2, Register dest,
		char * ibuf, Address & base, bool noCost, int size ) { }

/* Required by inst.C */
bool deleteBaseTramp( process *, instPoint *, instInstance * ) { return false; }

/* Required by ast.C */
void emitImm( opCode op, Register src1, RegValue src2imm, Register dest,
		char *ibuf, Address & base, bool noCost ) { }

/* Required by ast.C */
int getInsnCost( opCode op ) { return -1; }

/* Required by process.C */
bool process::emitInferiorRPCtrailer( void * void_insnPtr, Address & baseBytes,
			unsigned & breakOffset, bool shouldStopForResult,
			unsigned & stopForResultOffset,
			unsigned & justAfter_stopForResultOffset,
			bool isFunclet ) { return false; }

/* Required by process.C */
bool process::heapIsOk( const vector<sym_data> & find_us ) { return false; }

/* Required by inst.C */
void emitVupdate( opCode op, RegValue src1, Register src2,
		Address dest, char * ibuf, Address & base, bool noCost ) { }

/* Required by ast.C */
void emitLoadPreviousStackFrameRegister( Address register_num,
		Register dest, char * insn, Address & base,
		int size, bool noCost ) { }

/* Required by func-reloc.C */
bool pd_Function::isTrueCallInsn( const instruction insn ) { }

/* Required by BPatch_thread.C */
void returnInstance::installReturnInstance( process * proc ) { }

/* Required by ast.C */
void initTramps() { }

/* Required by func-reloc and ast .C */
void generateBranch( process * proc, Address fromAddr, Address newAddr ) { }

/* Required by func-reloc.C */
bool pd_Function::PA_attachGeneralRewrites( const image * owner,
		LocalAlterationSet * temp_alteration_set,
		Address baseAddress, Address firstAddress,
		instruction * loadedCode,
		unsigned numInstructions, int codeSize ) { return false; }

/* Required by LocalAlteration.C */
int InsertNops::numInstrAddedAfter() { return -1; }

/* Required by inst.C */
trampTemplate * findAndInstallBaseTramp( process * proc, instPoint * & location,
					returnInstance * & retInstance,
					bool trampRecursiveDesired,
					bool noCost, bool & deferred ) { return NULL; }

/* Required by func-reloc.C */
bool pd_Function::isNearBranchInsn( const instruction insn ) { return false; }

/* Required by process.C */
bool process::emitInferiorRPCheader( void * void_insnPtr, Address & baseBytes, bool isFunclet ) { return false; }

/* Required by ast.C */
Register emitR( opCode op, Register src1, Register src2, Register dest,
			char * ibuf, Address & base, bool noCost ) { return 0; }

/* Required by func-reloc.C */
bool pd_Function::loadCode(const image * owner, process * proc,
		instruction * & oldCode, unsigned & numberOfInstructions,
		Address & firstAddress) { return false; }

/* Required by func-reloc.C */
bool pd_Function::PA_attachBranchOverlaps(
		LocalAlterationSet * temp_alteration_set,
		Address baseAddress, Address firstAddress,
		instruction loadedCode[], unsigned numberOfInstructions,
		int codeSize )  { return false; }

/* Required by BPatch_init.C */
void initDefaultPointFrequencyTable() { }

/* Required by inst.C, ast.C */
Address emitA( opCode op, Register src1, Register src2, Register dest,
		char * ibuf, Address & base, bool noCost ) { return NULL; }

/* Required by ast.C */
bool doNotOverflow( int value ) { return false; }

/* Required by func-reloc.C */
bool pd_Function::PA_attachOverlappingInstPoints(
		LocalAlterationSet * temp_alteration_set,
		Address baseAddress, Address firstAddress,
		instruction * loadedCode , int codeSize ) { return false; }

/* Required by inst.C */
void installTramp( instInstance * inst, char * code, int codeSize ) { }

/* Required by func-reloc.C */
void pd_Function::copyInstruction( instruction & newInsn,
	instruction & oldInsn, unsigned & codeOffset) { }

/* Required by func-reloc.C */
bool pd_Function::fillInRelocInstPoints(
		const image * owner, process * proc,   
		instPoint * & location, relocatedFuncInfo * & reloc_info,
		Address mutatee, Address mutator, instruction oldCode[],
		Address newAdr, instruction newCode[],
		LocalAlterationSet & alteration_set ) { return false; }

/* Required by ast.C */
void emitVstore( opCode op, Register src1, Register src2, Address dest,
		char * ibuf, Address & base, bool noCost, int size ) { }

/* Required by inst.C */
void generateNoOp( process * proc, Address addr ) { }

/* Required by func-reloc.C ? */
bool PushEIPmov::RewriteFootprint( Address oldBaseAdr, Address & oldAdr,
				Address newBaseAdr, Address & newAdr,
				instruction oldInstructions[],
				instruction newInstructions[],
				int & oldInsnOffset, int & newInsnOffset,
				int newDisp, unsigned & codeOffset,
				unsigned char * code) { return false; }

/* -------- implementation of InsnAddr -------- */

InsnAddr InsnAddr::generateFromAlignedDataAddress( Address addr, process * p ) {
	assert( addr % 16 == 0 );  assert( p != NULL );
	InsnAddr generated( addr, p );
	return generated;
	} /* end gFADA() */

/* FIXME: shouldn't these be writeTextSpace() calls? */
bool InsnAddr::writeMyBundleFrom( const unsigned char * savedCodeBuffer ) {
	/* Aligns the instruction address and copies sixteen bytes from
	   the savedCodeBuffer to that address. */

	unsigned short offset = encodedAddress % 16;
	Address alignedAddress = encodedAddress - offset;

	return myProc->writeDataSpace( (void *)alignedAddress, 16, savedCodeBuffer );
	} /* end writeMyBundleFrom() */

bool InsnAddr::saveMyBundleTo( unsigned char * savedCodeBuffer ) {
	/* Aligns the instruction address and copies sixteen bytes from
	   there to the savedCodeBuffer. */

	unsigned short offset = encodedAddress % 16;
	Address alignedAddress = encodedAddress - offset;

	return myProc->readDataSpace( (void *)alignedAddress, 16, savedCodeBuffer, true );
	} /* end saveMyBundleTo() */

bool InsnAddr::replaceBundleWith( const IA64_bundle & bundle ) {
	/* Aligns the instruction address and copies in the bundle. */
	unsigned short offset = encodedAddress % 16;
	Address alignedAddress = encodedAddress - offset;
	ia64_bundle_t theBundle = bundle.getMyBundle();

	return myProc->writeDataSpace( (void *)alignedAddress, 16, & theBundle );
	} /* end replaceInstructionWith() */

bool InsnAddr::saveBundlesTo( unsigned char * savedCodeBuffer, unsigned int numberOfBundles ) {
	/* Align the instruction address and copy (numberOfBundles * 16) bytes 
	   from there to savedCodeBuffer. */
	unsigned short offset = encodedAddress % 16;
	Address alignedAddress = encodedAddress - offset;
	
	return myProc->readDataSpace( (void *)alignedAddress, (numberOfBundles * 16), savedCodeBuffer, true );
	} /* end saveBundlesTo() */

bool InsnAddr::replaceBundlesWith( const IA64_bundle * replacementBundles, unsigned int numberOfReplacementBundles ) {
	/* Align the instruction address and copy (numberOfReplacementBundles * 16) bytes
	   from the savedCodeBuffer to that address. */

	unsigned short offset = encodedAddress % 16;
	Address alignedAddress = encodedAddress - offset;

	return myProc->writeDataSpace( (void *)alignedAddress, (numberOfReplacementBundles * 16), replacementBundles );
	} /* end replaceBundlesWith() */

bool InsnAddr::writeStringAtOffset( unsigned int offsetInBundles, const char * string, unsigned int length ) {
	/* Align the instruction address and compute in the offset. */
	unsigned short offset = encodedAddress % 16;
	Address alignedOffset = encodedAddress - offset + (offsetInBundles * 16);

	return myProc->writeDataSpace( (void *)alignedOffset, length, string );
	} /* end writeStringAtOffset() */


#ifdef ZERO
class InsnAddr {
	public:
		/* prefix increment */
		InsnAddr operator++ ();

		/* prefix decrement */
		InsnAddr operator-- ();

		/* postfix increment */
		InsnAddr operator++ (int dummy);

		/* postfix decrement */
		InsnAddr operator-- (int dummy);

		/* sum of two InsnAddrs */
		friend InsnAddr operator + ( InsnAddr lhs, InsnAddr rhs );

		/* difference of two InsnAddrs */
		friend InsnAddr operator - ( InsnAddr lhs, InsnAddr rhs );

		friend bool operator < ( InsnAddr lhs, InsnAddr rhs );
		friend bool operator <= ( InsnAddr lhs, InsnAddr rhs );
		friend bool operator > ( InsnAddr lhs, InsnAddr rhs );
		friend bool operator >= ( InsnAddr lhs, InsnAddr rhs );
		friend bool operator == ( InsnAddr lhs, InsnAddr rhs );
		friend bool operator != ( InsnAddr lhs, InsnAddr rhs );

	private:
		Address encodedAddress;
}; /* end class InsnAddr */
#endif
