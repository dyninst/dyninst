/* -*- Mode: C; indent-tabs-mode: true; tab-width: 4 -*- */

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
 * $Id: inst-ia64.C,v 1.41 2003/10/28 18:57:36 schendel Exp $
 */

/* Note that these should all be checked for (linux) platform
   independence and the possibility of refactoring their originals
   into in- and de-pendent parts. */

#include <iomanip>
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

// for function relocation, which we don't do.
#include "dyninstAPI/src/func-reloc.h" 
#include "dyninstAPI/src/LocalAlteration.h"

#include "dyninstAPI/src/rpcMgr.h"

/* Assembler-level labels for the dynamic basetramp. */
extern "C" {
	void fetch_ar_pfs();
	void address_of_instrumentation();
	void address_of_call_alloc();
	void address_of_return_alloc();
	void address_of_address_of_call_alloc();
	void address_of_address_of_return_alloc();	
	void address_of_jump_to_emulation();
	} /* end template symbols */

/* Private refactoring function.  Copies and rewrites
   the basetramp template so it'll function in its new home. */
Address relocateBaseTrampTemplateTo( Address buffer, Address target ) {
	/* These offset won't change, so only calculate them once. */
	static Address addressOfTemplate = (Address)&fetch_ar_pfs;
	static Address offsetOfJumpToInstrumentation = 0;
	static Address offsetOfJumpToEmulation = 0;
	static Address offsetOfCallAlloc = 0;
	static Address offsetOfCallAllocMove = 0;	
	static Address offsetOfReturnAlloc = 0;
	static Address offsetOfReturnAllocMove = 0;

	if( offsetOfJumpToInstrumentation == 0 ) {
		offsetOfJumpToInstrumentation = (Address)&address_of_instrumentation - addressOfTemplate;
		offsetOfJumpToEmulation = (Address)&address_of_jump_to_emulation - addressOfTemplate;
		offsetOfCallAlloc = (Address)&address_of_call_alloc - addressOfTemplate;
		offsetOfCallAllocMove = (Address)&address_of_address_of_call_alloc - addressOfTemplate;
		offsetOfReturnAlloc = (Address)&address_of_return_alloc - addressOfTemplate;
		offsetOfReturnAllocMove = (Address)&address_of_address_of_return_alloc - addressOfTemplate;
		} /* end offset initialization */

	/* Our base tramp (the code block 'ia64_tramp_half')
	   needs two helper functions (get_ar_pfs() and
	   preservation_function()).  Since the latter needs
	   to be rewritten, we may as well copy the whole block
	   and avoid rewriting their calls.  (get_ar_pfs() is small.) */
	memmove( (void *)buffer, (void *)addressOfTemplate, offsetOfJumpToEmulation + 8 );

	/* Twiddle the appropriate bits. */
	alterLongMoveAtTo( target + offsetOfCallAllocMove, target + offsetOfCallAlloc );
	alterLongMoveAtTo( target + offsetOfReturnAllocMove, target + offsetOfReturnAlloc );

	/* Always overwrite the jump to instrumentation bundle with a jump
	   to the bundle past the jump to emulation.  This handles the
	   inferior RPC case cleanly. */
	IA64_instruction memoryNOP( NOP_M );
	IA64_instruction_x longJumpInsn = generateLongBranchTo( target + offsetOfJumpToEmulation + 16 );
	IA64_bundle jumpBundle = IA64_bundle( MLXstop, memoryNOP, longJumpInsn );
	ia64_bundle_t rawJumpBundle = jumpBundle.getMachineCode();
	memmove( (void *)(buffer + offsetOfJumpToInstrumentation), & rawJumpBundle, sizeof( ia64_bundle_t ) );

	/* Where do we stick the instrumentation code if we don't rewrite the jump again? */
	return offsetOfJumpToEmulation + 16;
	} /* end relocateTrampTemplate() */

/* Required by process, ast .C, et al */
registerSpace * regSpace;

/* Required by ast.C */
void emitCSload( BPatch_addrSpec_NP as, Register dest, char * baseInsn, Address & base, bool noCost ) {
	assert( 0 );
	} /* end emitCSload() */

/* Required by ast.C */
void emitVload( opCode op, Address src1, Register src2, Register dest,
		char * ibuf, Address & base, bool noCost, int size ) {
	assert( (((Address)ibuf + base) % 16) == 0 );
	ia64_bundle_t * rawBundlePointer = (ia64_bundle_t *)((Address)ibuf + base);

	switch( op ) {
		case loadOp: {
			/* Stick the address src1 in the temporary register src2. */
			emitVload( loadConstOp, src1, 0, src2, ibuf, base, noCost, size );

			/* Load into destination register dest from [src2]. */
			IA64_instruction loadInsn = generateRegisterLoad( dest, src2 );
			IA64_bundle loadBundle( MIIstop, loadInsn, NOP_I, NOP_I );
			
			/* Insert bundle. */
			rawBundlePointer = (ia64_bundle_t *)((Address)ibuf + base);
			* rawBundlePointer = loadBundle.getMachineCode();
			base += 16;
			break; }

		case loadConstOp: {
			// fprintf( stderr, "Loading a constant (0x%lx) into register %d\n", src1, dest );
			// Optimization: if |immediate| <= 22 bits, use generateShortImmediateAdd(), instead.
			IA64_instruction_x lCO = generateLongConstantInRegister( dest, src1 );
			* rawBundlePointer = generateBundleFromLongInstruction( lCO ).getMachineCode();
			base += 16;
			break; }

		case loadFrameRelativeOp:  // FIXME
			assert( 0 );
			break;

		case loadFrameAddr:  // FIXME
			assert( 0 );
			break;

		default:
			fprintf( stderr, "emitVload(): op not a load, aborting.\n" );
			abort();
			break;
		} /* end switch */
	} /* end emitVload() */

/* private function, used by emitFuncCall */
void emitRegisterToRegisterCopy( Register source, Register destination, char * ibuf, Address & base, registerSpace * rs ) {
	if( rs != NULL && ! rs->isFreeRegister( destination ) ) {
		fprintf( stderr, "Destination register %d is (was) not free(d), overwriting.\n", destination );
		} /* end if destination register is not free */

	/* Generate the machine code. (This may need to be refactored out
	   and migrated to arch-ia64.C, which would move ALIGN_RIGHT_SHIFT down
	   out of the arch-ia64.h header.) */
	/* Incidentally, this is 'adds destination = 0, source' */
	uint64_t rawMoveInsn = 0x0000000000000000 |
				(((uint64_t)0x08) << (37 + ALIGN_RIGHT_SHIFT)) |
				(((uint64_t)0x02) << (34 + ALIGN_RIGHT_SHIFT)) |
				(((uint64_t)source) << (20 + ALIGN_RIGHT_SHIFT )) |
				(((uint64_t)destination) << (6 + ALIGN_RIGHT_SHIFT ));

	IA64_instruction moveInsn( rawMoveInsn );
	IA64_instruction memoryNOP( NOP_M );
	IA64_bundle r2rBundle( MMIstop, memoryNOP, memoryNOP, moveInsn );

	/* Write the machine code. */
	assert( (((Address)ibuf + base) % 16) == 0 );
	ia64_bundle_t * rawBundlePointer = (ia64_bundle_t *)((Address)ibuf + base);
	* rawBundlePointer = r2rBundle.getMachineCode();
	base += 16;
	} /* end emitRegisterToRegisterCopy() */

/* private refactoring function */
Register findFreeLocal( registerSpace * rs, char * failure ) {
	Register freeLocalRegister = 0;
	unsigned int localZero = rs->getRegSlot( 0 )->number;
	for( unsigned int i = 0; i < NUM_LOCALS; i++ ) {
		if( rs->isFreeRegister( localZero + i ) ) { freeLocalRegister = localZero + i; break; }
		} /* end freeLocalRegister search */
	if( freeLocalRegister == 0 ) {
		fprintf( stderr, "%s", failure );
		abort();
		} /* end if failed to find a free local register */
	return freeLocalRegister;
	} /* end findFreeLocal() */

/* Required by ast.C */
Register emitFuncCall( opCode op, registerSpace * rs, char * ibuf,
					   Address & base, const pdvector<AstNode *> & operands,
					   process * proc, bool noCost,
					   Address  callee_addr, 
					   const pdvector<AstNode *> &ifForks,
					   const instPoint *location) { 
	/* Consistency check. */
	assert( op == callOp );

	/* We'll get garbage (in debugging) if ibuf isn't aligned. */
	if( (Address)ibuf % 16 != 0 ) { fprintf( stderr, "Instruction buffer (%p) not aligned!\n", ibuf ); }
	

	/*  Sanity check for non NULL address argument */
	if (!callee_addr) {
	  char msg[256];
	  sprintf(msg, "%s[%d]:  internal error:  emitFuncCall called w/out"
			  "callee_addr argument", __FILE__, __LINE__);
	  showErrorCallback(80, msg);
	  assert(0);
	}
 
	Address calleeGP = proc->getTOCoffsetInfo( callee_addr );

	/* Generate the code for the arguments. */
	pdvector< Register > sourceRegisters;
	for( unsigned int i = 0; i < operands.size(); i++ ) {
		sourceRegisters.push_back( operands[i]->generateCode_phase2( proc, rs, ibuf, base, false, ifForks, location ) );
		}

	/* source-to-output register copy */
	unsigned int outputZero = rs->getRegSlot( NUM_LOCALS )->number;
	for( unsigned int i = 0; i < sourceRegisters.size(); i++ ) {
	// fprintf( stderr, "Copying argument in local register %d to output register %d\n", sourceRegisters[i], outputZero + i );
		emitRegisterToRegisterCopy( sourceRegisters[i], outputZero + i, ibuf, base, rs );
		rs->freeRegister( sourceRegisters[i] );
		rs->incRefCount( outputZero + i );
		} /* end source-to-output register copy */

	/* Since the BT was kind enough to save the GP, return value,
	   return pointer, ar.pfs, and both scratch registers for us,
	   we don't bother.  Just set the callee's GP, copy the call
	   target into a scratch branch registers, and indirect to it. */
	emitVload( loadConstOp, calleeGP, 0, REGISTER_GP, ibuf, base, false /* ? */, 0 );

	fprintf( stderr, "* Constructing call to function 0x%lx\n", callee_addr );

	/* FIXME: could use output registers, if past sourceRegisters.size(). */
	/* Grab a register -- temporary for transfer to branch register,
	   and a registerSpace register for the return value. */
	Register rsRegister = findFreeLocal( rs, "Unable to find local register in which to store callee address, aborting.\n" );
	rs->incRefCount( rsRegister );

	IA64_instruction integerNOP( NOP_I );
	IA64_instruction memoryNOP( NOP_M );

	// fprintf( stderr, "Loading function address into register %d\n", rsRegister );
	IA64_instruction_x loadCalleeInsn = generateLongConstantInRegister( rsRegister, callee_addr );
	IA64_bundle loadCalleeBundle( MLXstop, memoryNOP, loadCalleeInsn );

	// fprintf( stderr, "Copying computed branch in general register %d to branch register %d.\n", rsRegister, BRANCH_SCRATCH );
	IA64_instruction setBranchInsn = generateRegisterToBranchMove( rsRegister, BRANCH_SCRATCH );
	IA64_bundle setBranchBundle( MIIstop, memoryNOP, integerNOP, setBranchInsn );

	// fprintf( stderr, "Calling function with offset in branch register %d, return pointer in %d\n", BRANCH_SCRATCH, BRANCH_RETURN );
	IA64_instruction indirectBranchInsn = generateIndirectCallTo( BRANCH_SCRATCH, BRANCH_RETURN );
	IA64_bundle indirectBranchBundle( MMBstop, memoryNOP, memoryNOP, indirectBranchInsn );

	int bundleCount = 0;
	ia64_bundle_t * rawBundlePointer = (ia64_bundle_t *)((Address)ibuf + base);
	rawBundlePointer[bundleCount++] = loadCalleeBundle.getMachineCode();
	rawBundlePointer[bundleCount++] = setBranchBundle.getMachineCode();

	/* According to the software conventions, the stack pointer must have sixteen bytes
	   of scratch space available directly above it.  Give it sixteen bytes nobody's using
	   to make sure we don't smash the stack.  This is NOT done in the preservation header. */
	IA64_instruction moveSPDown = generateShortImmediateAdd( REGISTER_SP, -16, REGISTER_SP );
	rawBundlePointer[bundleCount++] = IA64_bundle( MIIstop, moveSPDown, integerNOP, integerNOP ).getMachineCode();

	rawBundlePointer[bundleCount++] = indirectBranchBundle.getMachineCode();
	for( unsigned int i = 0; i < sourceRegisters.size(); i++ ) {
		/* Free all the output registers. */
		rs->freeRegister( outputZero + i );
		}
	base += bundleCount * 16;

	/* copy the result (r8) to a registerSpace register */
	rs->freeRegister( rsRegister );
	emitRegisterToRegisterCopy( REGISTER_RV, rsRegister, ibuf, base, rs );
	fprintf( stderr, "* emitted function call in buffer at %p\n", ibuf );

	/* Correct the stack pointer. */
	IA64_instruction moveSPUp = generateShortImmediateAdd( REGISTER_SP, 16, REGISTER_SP );
	rawBundlePointer = (ia64_bundle_t *)((Address)ibuf + base);
	rawBundlePointer[0] = IA64_bundle( MIIstop, moveSPUp, integerNOP, integerNOP ).getMachineCode();
	base += 16;	

	/* return that register */
	return rsRegister;
	} /* end emitFuncCall() */

/* Required by symtab.C */
void pd_Function::checkCallPoints() {
	/* We associate pd_Function*'s with the callee's address.

	   On other architectures, those without explicit call instructions,
	   we winnow out potential call sites whose target resides in the same function. */
  if (call_points_have_been_checked) return;

	instPoint * callSite;
	Address targetAddress;
	pd_Function * calledPdF;
	image * owner = file_->exec();
	pdvector< instPoint * > pdfCalls;

	for( unsigned int i = 0; i < calls.size(); i++ ) {
		callSite = calls[i]; assert( callSite );
		
		/* Extract the address that callSite calls. */
		targetAddress = callSite->getTargetAddress();

		calledPdF = owner->findFuncByOffset( targetAddress );
		if( calledPdF ) {
			callSite->set_callee( calledPdF );
			} else {
			callSite->set_callee( NULL );
			} /* end calledPdF conditional */

		pdfCalls.push_back( callSite );
		} /* end callee loop */

	calls = pdfCalls;
	call_points_have_been_checked = true;
} /* end checkCallPoints() */

/* Required by symtab.C:
   Locate the entry point (funcEntry_, an InstPoint), the call sites
   (calls, vector<instPoint *>), and the return(s) (funcReturns, vector<instPoint *>).
 */
bool pd_Function::findInstPoints( const image * i_owner ) {
	/* We assume the function boundaries are correct.  [Check jump tables, etc.] */
	Address addr = (Address)i_owner->getPtrToInstruction( getAddress( 0 ) );

	IA64_iterator iAddr( addr );
	Address lastI = addr + size();
	Address addressDelta = getAddress( 0 ) - addr;

	/* Generate an instPoint for the function entry. */
	funcEntry_ = new instPoint( getAddress( 0 ), this, i_owner, * iAddr );

	IA64_instruction * currInsn;
	for( ; iAddr < lastI; iAddr++ ) {	
		currInsn = * iAddr;
		switch( currInsn->getType() ) {
			case IA64_instruction::RETURN:
				funcReturns.push_back( new instPoint( iAddr.getEncodedAddress() + addressDelta, this, i_owner, currInsn ) );
				break;
			case IA64_instruction::DIRECT_CALL:
			case IA64_instruction::INDIRECT_CALL:
				calls.push_back( new instPoint( iAddr.getEncodedAddress() + addressDelta, this, i_owner, currInsn ) );
				break;
			case IA64_instruction::ALLOC:
				allocs.push_back( iAddr.getEncodedAddress() - addr );
				break;
			default:
				break;
			} /* end instruction type switch */
		} /* end instruction iteration */

	isInstrumentable_ = 1;
	return true; // if this function can ever return false, make sure the is_instrumentable flag
	// is set properly in this function.
	} /* end findInstPoints() */

/* Required by func-reloc.C */
LocalAlteration *fixOverlappingAlterations( LocalAlteration * alteration, LocalAlteration * tempAlteration ) { assert( 0 ); return NULL; }

/* Required by LocalAlteration.C */
int InsertNops::sizeOfNop() { assert( 0 ); return 0; }

/* Required by func-reloc.C */
bool InsertNops::RewriteFootprint( Address oldBaseAdr, Address & oldAdr,
				Address newBaseAdr, Address & newAdr,
				instruction oldInstructions[],
				instruction newInstructions[],
				int & oldOffset, int & newOffset,
				int newDisp, unsigned & codeOffset,
				unsigned char *code ) { assert( 0 ); return false; }

/* Required by func-reloc.C */
bool ExpandInstruction::RewriteFootprint( Address oldBaseAdr, Address & oldAdr,
		Address newBaseAdr, Address & newAdr,
		instruction oldInstructions[], instruction newInstructions[],
		int &oldOffset, int &newOffset, int newDisp,
		unsigned &codeOffset, unsigned char* code ) { assert( 0 ); return false; }

/* Required by ast.C */
void emitFuncJump(opCode op, char * i, Address & base,
		  const function_base * callee, process * proc,
		  const instPoint *, bool) { assert( 0 ); }

/* Required by BPatch_ function, image .C */
BPatch_point *createInstructionInstPoint( process * proc, void * address,
			BPatch_point ** alternative, BPatch_function * bpf ) { return NULL; assert( 0 ); }

/* Required by ast.C */
void emitASload( BPatch_addrSpec_NP as, Register dest, char * baseInsn,
			Address & base, bool noCost ) { assert( 0 ); }

/* Required by func-reloc.C */
bool PushEIP::RewriteFootprint( Address oldBaseAdr, Address & oldAdr,
		Address newBaseAdr, Address & newAdr,
		instruction oldInstructions[], instruction newInstructions[],
		int & oldInsnOffset, int & newInsnOffset,
		int newDisp, unsigned & codeOffset, unsigned char * code ) { assert( 0 ); return false; }

/* Required by process.C */
void instWaitingList::cleanUp( process *, Address addr ) { assert( 0 ); }

/* Required by BPatch_thread.C */
bool process::replaceFunctionCall( const instPoint * point, 
			const function_base * func ) { assert( 0 ); return false; }

/* Required by ast.C */
Register deadRegisterList[] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF };
void initTramps(bool is_multithreaded) { 
	/* Initialize the registerSpace pointer regSpace to the state of the
	   registers that will exist at the end of the first part of the base
	   tramp's code.  (That is, for the minitramps.

	   The difficulty here is that the register numbers are expected to be
	   physical registers, but we can't determine those until we know which
	   function we're instrumenting...

	   For now, we'll just say 0x1 - 0xF. */

	static bool haveInitializedTramps = false;
	if( haveInitializedTramps ) { return; } else { haveInitializedTramps = true; }

	regSpace = new registerSpace( sizeof( deadRegisterList )/sizeof( Register ), deadRegisterList, 0, NULL );
	} /* end initTramps() */

/* Required by ast.C */
void emitV( opCode op, Register src1, Register src2, Register dest,
		char * ibuf, Address & base, bool noCost, int size, const instPoint * location, process * proc, registerSpace * rs ) {
	assert( (((Address)ibuf + base) % 16) == 0 );
	ia64_bundle_t * rawBundlePointer = (ia64_bundle_t *)((Address)ibuf + base);

	switch( op ) { 
		case eqOp:
		case leOp:
		case greaterOp:
		case lessOp:
		case geOp: 
		case neOp: {
			/* Unless we can verify that a comparison result is never used
			   as an operand, we can't do the right thing and treat the
			   destination as a predicate register.  *Sigh*. */
			IA64_instruction comparisonInsn = generateComparison( op, dest, src1, src2 );
			IA64_instruction setZero = predicateInstruction( dest, generateShortImmediateAdd( dest, 0, 0 ) );
			IA64_instruction setOne = predicateInstruction( dest + 1, generateShortImmediateAdd( dest, 1, 0 ));
			IA64_bundle comparisonBundle( MstopMIstop, comparisonInsn, setZero, setOne );
			* rawBundlePointer = comparisonBundle.getMachineCode();
			base += 16;
			break; }

		case plusOp:
		case minusOp:
		case andOp:
		case orOp: {
			IA64_instruction arithmeticInsn = generateArithmetic( op, dest, src1, src2 );
			IA64_instruction integerNOP( NOP_I );
			IA64_bundle arithmeticBundle( MIIstop, arithmeticInsn, integerNOP, integerNOP );
			* rawBundlePointer = arithmeticBundle.getMachineCode();
			base += 16;
			break; }

		case timesOp: {
			/* We have to get cute.  (Ick!)  'Cause our friends in Intel
			   decided that nobody would want to multiply integers _that_
			   often, they wedged the integer multiplier into the FPU.

			   Yeah.  I'm going to try not to ask, too. */
			unsigned bundleCount = 0;

			/* Since I don't save the FP registers, spill two of them.
			   This usage assumes that the SP is _correct_, that is,
			   that we won't overwrite anything, including the stored predicates
			   from the basetramp, if we subtract the space we use first. */
			IA64_instruction moveSPDown = generateShortImmediateAdd( REGISTER_SP, -16, REGISTER_SP );
			IA64_instruction spillFP2 = generateFPSpillTo( REGISTER_SP, 2 );
			IA64_instruction spillFP3 = generateFPSpillTo( REGISTER_SP, 3 );
			IA64_instruction integerNOP( NOP_I );
			IA64_bundle spillF2Bundle( MstopMIstop, moveSPDown, spillFP2, integerNOP );
			IA64_bundle spillF3Bundle( MstopMIstop, moveSPDown, spillFP3, integerNOP );
			rawBundlePointer[bundleCount++] = spillF2Bundle.getMachineCode();
			rawBundlePointer[bundleCount++] = spillF3Bundle.getMachineCode();

			/* Do the multiplication. */
			IA64_instruction copySrc1 = generateRegisterToFloatMove( src1, 2 );
			IA64_instruction copySrc2 = generateRegisterToFloatMove( src2, 3 );
			IA64_instruction fixedMultiply = generateFixedPointMultiply( 2, 2, 3 );
			IA64_instruction copyDest = generateFloatToRegisterMove( 2, dest );
			IA64_bundle copySources( MMIstop, copySrc1, copySrc2, integerNOP );
			IA64_instruction memoryNOP( NOP_M );
			IA64_bundle doMultiplication( MFIstop, memoryNOP, fixedMultiply, integerNOP );
			IA64_bundle copyToDestination( MMIstop, copyDest, memoryNOP, integerNOP );
			rawBundlePointer[bundleCount++] = copySources.getMachineCode();
			rawBundlePointer[bundleCount++] = doMultiplication.getMachineCode();
			rawBundlePointer[bundleCount++] = copyToDestination.getMachineCode();

			/* Restore the FP registers, SP. */
			IA64_instruction moveSPUp = generateShortImmediateAdd( REGISTER_SP, 16, REGISTER_SP );
			IA64_instruction fillFP2 = generateFPFillFrom( REGISTER_SP, 2 );
			IA64_instruction fillFP3 = generateFPFillFrom( REGISTER_SP, 3 );
			IA64_bundle fillF2Bundle( MstopMIstop, fillFP2, moveSPUp, integerNOP );
			IA64_bundle fillF3Bundle( MstopMIstop, fillFP3, moveSPUp, integerNOP );
			rawBundlePointer[bundleCount++] = fillF2Bundle.getMachineCode();
			rawBundlePointer[bundleCount] = fillF3Bundle.getMachineCode();
			base += (16 * bundleCount);
			break; }

		case divOp: { /* Call libc's __divsi3(). */

			/* Make sure we've got everything we need to work with. */
			assert( location );
			assert( proc );
			assert( rs );
			opCode op( callOp );

			/* Generate the operand ASTs. */
			pdvector< AstNode * > operands;
			operands.push_back( new AstNode( AstNode::DataReg, (void *)(long long int)src1 ) );
			operands.push_back( new AstNode( AstNode::DataReg, (void *)(long long int)src2 ) );

			/* Generate the (empty?) ifForks AST. */
			pdvector< AstNode * > ifForks;

			/* Callers of emitV() expect the register space to be unchanged
			   afterward, so make sure eFC() doesn't use the source registers
			   after copying them to the output. */

			/* Find address for functions __divsi3 */
            pdstring callee = pdstring("__divsi3");
			bool err = false;
			Address callee_addr = proc->findInternalAddress(callee, false, err);
			if (err) {
  
			  function_base *calleefunc = proc->findOnlyOneFunction(callee);
			  if (!calleefunc) {
				char msg[256];
				sprintf(msg, "%s[%d]:  internal error:  unable to find %s",
						__FILE__, __LINE__, callee.c_str());
				showErrorCallback(100, msg);
				assert(0);  // can probably be more graceful
			  }
			  callee_addr = calleefunc->getEffectiveAddress(proc);
			}

			/* Might want to use funcCall AstNode here */

			/* Call emitFuncCall(). */
			rs->incRefCount( src1 ); rs->incRefCount( src2 );
			Register returnRegister = emitFuncCall( op, rs, ibuf, base, operands, proc, noCost, callee_addr, ifForks, location );
			if( returnRegister != dest ) {
				rs->freeRegister( dest );
				emitRegisterToRegisterCopy( returnRegister, dest, ibuf, base, rs );
				rs->incRefCount( dest );
				rs->freeRegister( returnRegister );
				}
			break; }

		case noOp: {
			IA64_bundle nopBundle( MIIstop, NOP_M, NOP_I, NOP_I );
			* rawBundlePointer = nopBundle.getMachineCode();
			base += 16;			
			break; }

		default:
			fprintf( stderr, "emitV(): unrecognized op code %d\n", op );
			assert( 0 );
			break;
		} /* end op switch */
	} /* end emitV() */

/* Required by inst.C */
bool deleteBaseTramp( process */*proc*/, trampTemplate *) {
	return false;
}


/* Required by ast.C */
void emitImm( opCode op, Register src1, RegValue src2imm, Register dest,
		char *ibuf, Address & base, bool noCost ) {
	switch( op ) {
		case orOp: {
			/* Apparently, the bit-wise operation. */
			if( src2imm == 0 ) {
				emitRegisterToRegisterCopy( src1, dest, ibuf, base, NULL );
				} else {
				abort();
				}
		} break;

		default:
			assert( 0 );
		break;
		} /* end op switch */
	} /* end emitImm() */

/* Required by ast.C */
int getInsnCost( opCode op ) {
	/* FIXME: correct costs. */

	int cost = 0;
	switch( op ) {
		case noOp:
		case loadIndirOp:
		case saveRegOp:
		case plusOp:
		case minusOp:
		case timesOp:
		case divOp:
		case loadConstOp:
		case loadOp:
		case loadFrameRelativeOp:
		case loadFrameAddr:
		case storeOp:
		case storeIndirOp:
		case storeFrameRelativeOp:
		case ifOp:
		case eqOp:
		case neOp:
		case lessOp:
		case greaterOp:
		case leOp:
		case geOp:
		case branchOp:
		case callOp:
		case whileOp:
		case doOp:
		case updateCostOp:
		case trampPreamble:
		case trampTrailer:
		case orOp:
		case andOp:
		case getRetValOp:
		case getSysRetValOp:
		case getSysParamOp:
		case getParamOp:
		case getAddrOp:
		case funcJumpOp:
			cost = 1;
			break;

		default: cost = 0; break;
		} /* end op switch */

	return cost;
	} /* end getInsnCost() */

/* Required by process.C */
bool process::heapIsOk( const pdvector<sym_data> & find_us ) {
	/* Set .mainFunction; in the interest of civility, I will
	   not speculate why this is done here. */
	if( ! (mainFunction = findOnlyOneFunction( "main" )) ) {
		statusLine( "Failed to find main()." );
		showErrorCallback( 50, "Failed to find main()." );
		return false;
		}

	/* I will also not speculate why we ask after symbols that
	   don't need to be found if we don't do anything with them... */
	pdstring str; Symbol sym; Address baseAddress;
	for( unsigned int i = 0; i < find_us.size(); i++ ) {
		if( ! find_us[i].must_find ) { break; }
		str = find_us[i].name;
		if( ! getSymbolInfo( str, sym, baseAddress ) && ! getSymbolInfo( "_" + str, sym, baseAddress ) ) {
			str = "Cannot find symbol \"" + str + "\".";
			statusLine( str.c_str() );
			showErrorCallback( 50, str );
			} /* end a must-find symbol isn't found */
		} /* end look-up loop */
			
	return true;
	} /* end of heapIsOk() */

/* Required by inst.C */
void emitVupdate( opCode op, RegValue src1, Register src2,
		Address dest, char * ibuf, Address & base, bool noCost ) {
	switch( op ) {
		case updateCostOp:
fprintf( stderr, "FIXME: no-op'ing emitVupdate(updateCostOp)\n" );
			break;

		default:
			assert( 0 );
			break;
		} /* end op switch */
	} /* end emitVupdate() */

/* Required by ast.C */
void emitLoadPreviousStackFrameRegister( Address register_num,
		Register dest, char * insn, Address & base,
		int size, bool noCost ) { assert( 0 ); }

/* Required by func-reloc.C */
bool pd_Function::isTrueCallInsn( const instruction insn ) { assert( 0 ); }

/* Required by BPatch_thread.C */
void returnInstance::installReturnInstance( process * proc ) { 
	IA64_instruction memoryNOP( NOP_M );
	IA64_bundle installationBundle( MLXstop, memoryNOP, * (IA64_instruction_x *) instructionSeq );	
	InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( addr_, proc );
	iAddr.replaceBundleWith( installationBundle );
	installed = true;
	} /* end installReturnInstance() */

/* Required by inst.C */
void generateBranch( process * proc, Address fromAddr, Address newAddr ) {
	/* Write a branch from fromAddr to newAddr in process proc. */
	IA64_instruction memoryNOP( NOP_M );
	IA64_instruction_x longBranch = generateLongBranchTo( newAddr - fromAddr );
	IA64_bundle branchBundle( MLXstop, memoryNOP, longBranch );
	InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( fromAddr, proc );
	iAddr.replaceBundleWith( branchBundle );
	} /* end generateBranch */

/* Required by func-reloc.C */
bool pd_Function::PA_attachGeneralRewrites( const image * owner,
		LocalAlterationSet * temp_alteration_set,
		Address baseAddress, Address firstAddress,
		instruction * loadedCode,
		unsigned numInstructions, int codeSize ) { assert( 0 ); return false; }

/* Required by LocalAlteration.C */
int InsertNops::numInstrAddedAfter() { assert( 0 ); return -1; }

/* in arch-ia64.C */
extern IA64_instruction::unitType INSTRUCTION_TYPE_ARRAY[(0x20 + 1) * 3];

/* left-aligned constants for decoding the PSR */
#define PSR_RI_MASK	0x0000060000000000

/* left-aligned constants for instruction emulation */
#define BIT_36		0x0800000000000000	
#define BIT_13_32	0x00FFFFF000000000
#define IMMEDIATE_MASK	(~( 0 | BIT_36 | BIT_13_32 ))
#define BIT_2_40	0xFFFFFFFFF0000000
#define BIT_20_32	0x00FFF80000000000
#define BIT_06_12	0x0000000FE0000000
#define X3_MASK         0x0700000000000000

/* right-aligned constants for instruction emulation */
#define RIGHT_IMM20	0x00000000000FFFFF
#define RIGHT_IMM39	0x07FFFFFFFFF00000

/* private refactoring function */
IA64_bundle generateBundleFor( IA64_instruction insnToBundle ) {
	/* The instruction to bundle is always short; the long instruction bundling is trivial. */
	IA64_instruction::unitType iUnitType = INSTRUCTION_TYPE_ARRAY[ (insnToBundle.getTemplateID() * 3) + insnToBundle.getSlotNumber() ];

	IA64_instruction integerNOP( NOP_I );
	IA64_instruction memoryNOP( NOP_M );
	
	switch( iUnitType ) {
		case IA64_instruction::M:
			return IA64_bundle( MIIstop, insnToBundle, integerNOP, integerNOP );
			break;
		case IA64_instruction::I:
			return IA64_bundle( MIIstop, memoryNOP, insnToBundle, integerNOP );
			break;
		case IA64_instruction::B:
			return IA64_bundle( MMBstop, memoryNOP, memoryNOP, insnToBundle );
			break;
		case IA64_instruction::F:
			return IA64_bundle( MFIstop, memoryNOP, insnToBundle, integerNOP );
			break;

		case IA64_instruction::L:
		case IA64_instruction::X:
		case IA64_instruction::RESERVED:
		default:
			fprintf( stderr, "Invalid instruction type in generateBundleFor(), aborting.\n" );
			abort();
			break;
		} /* end iUnitType switch */
	} /* end generateBundleFor() */

/* private refactoring function */
void emulateLongInstruction( IA64_instruction_x insnToEmulate, Address originalLocation, ia64_bundle_t * insnPtr, unsigned int & offset, unsigned int & size, Address allocatedAddress ) {
	/* originalLocation is NOT an encoded address */
	switch( insnToEmulate.getType() ) {
		case IA64_instruction::DIRECT_BRANCH:
		case IA64_instruction::DIRECT_CALL: {
			/* Correct the immediate by the difference between originalLocation and (insnPtr + size). */
			uint64_t lowWord = insnToEmulate.getMachineCode().low;
			uint64_t highWord = insnToEmulate.getMachineCode().high;

			uint64_t immediate = 	( 0 |
			(((highWord & BIT_36) >> (36 + ALIGN_RIGHT_SHIFT)) << 59) |
			(((lowWord & BIT_2_40) >> (2 + ALIGN_RIGHT_SHIFT)) << 20) |
			(((highWord & BIT_13_32) >> (13 + ALIGN_RIGHT_SHIFT)))
						) << 4;

			Address target = immediate + originalLocation;
			Address source = allocatedAddress + size;
			long long int displacement64 = target - source;
			uint64_t displacement60 = displacement64 >> 4;

			highWord =	( highWord & IMMEDIATE_MASK ) |
					( ((uint64_t)(displacement64 < 0)) << (36 + ALIGN_RIGHT_SHIFT)) |   
					( (displacement60 & RIGHT_IMM20) << (13 + ALIGN_RIGHT_SHIFT));
			lowWord = 0x0000000000000000 |
					( (displacement60 & RIGHT_IMM39) << (-20 + 2 + ALIGN_RIGHT_SHIFT) );

			IA64_instruction memoryNOP( NOP_M );
			IA64_instruction_x alteredLong( lowWord, highWord, insnToEmulate.getTemplateID() );
			IA64_bundle emulationBundle( MLXstop, memoryNOP, alteredLong );
			insnPtr[offset++] = emulationBundle.getMachineCode(); size += 16;
			break; } /* end branch handling */

		default: {
			IA64_instruction memoryNOP( NOP_M );
			insnPtr[offset++] = IA64_bundle( MLXstop, memoryNOP, insnToEmulate ).getMachineCode(); size += 16;
			break; } /* end default case */
		} /* end type switch */
	} /* end emulateLongInstruction */

/* private refactoring function */
#define TYPE_20B 0
#define TYPE_13C 1
void rewriteShortOffset( IA64_instruction insnToRewrite, Address originalLocation, ia64_bundle_t * insnPtr, unsigned int & offset, unsigned int & size, unsigned int immediateType, Address allocatedAddress ) {
	/* We insert a short jump past a long jump to the original target, followed
	   by the instruction rewritten to branch one bundle backwards.
	   It's not very elegant, but it's very straightfoward to implement. :) */
	
	/* Skip the long branch. */
	IA64_instruction memoryNOP( NOP_M );
	IA64_instruction_x skipInsn = generateLongBranchTo( 32 ); // could be short
	IA64_bundle skipInsnBundle( MLXstop, memoryNOP, skipInsn );
	insnPtr[offset++] = skipInsnBundle.getMachineCode(); size += 16;

	/* Extract the short immediate. */
	uint64_t immediate;
	uint64_t machineCode = insnToRewrite.getMachineCode();
	switch( immediateType ) {
		case TYPE_20B:
			immediate = 	0 |
					( (machineCode & BIT_36) >> (36 + ALIGN_RIGHT_SHIFT - 63) ) |
					( (machineCode & BIT_13_32) >> (13 + ALIGN_RIGHT_SHIFT) );
			break;

		case TYPE_13C:
			immediate = 	0 |
					( (machineCode & BIT_36) >> (36 + ALIGN_RIGHT_SHIFT - 63) ) |
					( (machineCode & BIT_20_32) >> (20 + ALIGN_RIGHT_SHIFT - 7) ) |
					( (machineCode & BIT_06_12) >> (6 + ALIGN_RIGHT_SHIFT) );
			break;

		default:
			fprintf( stderr, "Unrecognized immediate type in rewriteShortImmediate(), aborting.\n" );
			abort();
			break;
		} /* end immediateType switch */

	/* The long branch. */
	Address originalTarget = (immediate << 4 ) + originalLocation;
	IA64_instruction_x longBranch = generateLongBranchTo( originalTarget - (allocatedAddress + size) );
	IA64_bundle longBranchBundle( MLXstop, memoryNOP, longBranch );
	insnPtr[offset++] = longBranchBundle.getMachineCode(); size += 16;

	/* Rewrite the short immediate. */
	switch( immediateType ) {
		case TYPE_20B:
			machineCode =	( machineCode & IMMEDIATE_MASK ) |
					( ((uint64_t)1) << (36 + ALIGN_RIGHT_SHIFT) ) |
					( ((uint64_t)1) << (13 + ALIGN_RIGHT_SHIFT) );
			break;

		case TYPE_13C:
			machineCode =	( machineCode & IMMEDIATE_MASK ) |
					( ((uint64_t)1) << (36 + ALIGN_RIGHT_SHIFT) ) |
					( ((uint64_t)1) << (6 + ALIGN_RIGHT_SHIFT) );
			break;

		default:
			fprintf( stderr, "Unrecognized immediate type in rewriteShortImmediate(), aborting.\n" );
			abort();
			break;
		} /* end immediateType switch */

	/* Emit the rewritten immediate. */
	IA64_instruction rewrittenInsn( machineCode, insnToRewrite.getTemplateID(), NULL, insnToRewrite.getSlotNumber() );
	insnPtr[offset++] = generateBundleFor( rewrittenInsn ).getMachineCode(); size += 16;
	} /* end rewriteShortOffset() */

/* private refactoring function */
void emulateShortInstruction( IA64_instruction insnToEmulate, Address originalLocation, ia64_bundle_t * insnPtr, unsigned int & offset, unsigned int & size, Address allocatedAddress ) {
	switch( insnToEmulate.getType() ) {
		case IA64_instruction::DIRECT_BRANCH:
		case IA64_instruction::DIRECT_CALL: {
			rewriteShortOffset( insnToEmulate, originalLocation, insnPtr, offset, size, TYPE_20B, allocatedAddress );
			break; } /* end direct jump handling */

		case IA64_instruction::BRANCH_PREDICT: {
			/* We can suffer the performance loss. :) */
			insnPtr[offset++] = IA64_bundle( MIIstop, NOP_M, NOP_I, NOP_I ).getMachineCode(); size += 16;
			break; } /* end branch predict handling */

		case IA64_instruction::CHECK: {
			/* The advanced load checks can be handled exactly as we 
			   handle direct branches and calls.  The other three checks
			   (I&M unit integer speculation, fp speculation) are handled
			   identically to each other, but their immediates are laid
			   out a little different, so we can't handle them as we do
			   the advanced loads. */

			uint64_t instruction = insnToEmulate.getMachineCode();
			uint8_t x3 = (instruction & X3_MASK) >> (ALIGN_RIGHT_SHIFT + 33);
			if( x3 >= 4 && x3 <= 7 ) { /* advanced load check */
				rewriteShortOffset( insnToEmulate, originalLocation, insnPtr, offset, size, TYPE_20B, allocatedAddress );
				} /* end if M22 or M23 */
			else if( x3 == 1 || x3 == 3 ) { /* speculation check */
				rewriteShortOffset( insnToEmulate, originalLocation, insnPtr, offset, size, TYPE_13C, allocatedAddress );
				} /* end if I20, M20, or M21. */
			break; } /* end speculation check handling */

		case IA64_instruction::BRANCH_IA: {
			assert( 0 );
			break; } /* end branch to x86 handling */

		case IA64_instruction::MOVE_FROM_IP: {
			/* Replace with a movl of the original IP. */
			unsigned int originalRegister = (insnToEmulate.getMachineCode() & 0x0000000FE0000000) >> 6; // bits 06 - 12
			IA64_instruction memoryNOP( NOP_M );
			IA64_instruction_x emulatedIPMove = generateLongConstantInRegister( originalRegister, originalLocation );
			IA64_bundle ipMoveBundle( MLXstop, memoryNOP, emulatedIPMove );
			insnPtr[offset++] = ipMoveBundle.getMachineCode(); size += 16;
			break; } /* end ip move handling */

		case IA64_instruction::RETURN:
		case IA64_instruction::INDIRECT_CALL:
		case IA64_instruction::INDIRECT_BRANCH:
			/* Branch registers hold absolute addresses. */
		default: {
			insnPtr[offset++] = generateBundleFor( insnToEmulate ).getMachineCode(); size += 16;
			break; } /* end default case */
		} /* end type switch */
	} /* end emulateShortInstruction() */

/* private refactoring function */
void emulateBundle( IA64_bundle bundleToEmulate, Address originalLocation, ia64_bundle_t * insnPtr, unsigned int & offset, unsigned int & size, Address allocatedAddress ) {
	/* We need to alter all IP-relative instructions to do the Right Thing.  In particular:
	
		mov rX = ip	->	movl rX = <originalLocation>
		brp.*		->	nop.m	; Is an architectural nop, and we've already destroyed performance by doing the instrumentation.
		brl.*		->	brl.*	; correct the offset by the difference between originalLocation and (allocatedAddress + size)
		check.*		->	check.*	; replace target25 with an offset to a brl to the original target, as adjusted for brl.*
		br.[cond|call]	->	brl.*	; direct only; fix as brl.*
		br.cloop	->	
		br.c[top|exit]	->
		br.w[top|exit]	->	br.*	; can handle as per the check.* instructions.
		br.[cond|call]i	->	br.*	; 
		br.ret		->	br.ret	; The three indirect branches can be safely duplicated:
						; the branch registers hold absolute addresses.
		br.ia		->		; barf: maybe invoke the x86 port? :)
	*/

	emulateShortInstruction( * bundleToEmulate.getInstruction( 0 ), originalLocation, insnPtr, offset, size, allocatedAddress );

	if( bundleToEmulate.hasLongInstruction() ) {
		emulateLongInstruction( * bundleToEmulate.getLongInstruction(), originalLocation, insnPtr, offset, size, allocatedAddress );
		} else {
		emulateShortInstruction( * bundleToEmulate.getInstruction( 1 ), originalLocation, insnPtr, offset, size, allocatedAddress );
		emulateShortInstruction( * bundleToEmulate.getInstruction( 2 ), originalLocation, insnPtr, offset, size, allocatedAddress );
		}
	} /* end emulateBundle() */

/* Required by func-reloc.C */
bool pd_Function::isNearBranchInsn( const instruction insn ) { assert( 0 ); return false; }

#define BIT_32_37 0x3F00000000
#define BIT_25_31 0x00FE000000
#define BIT_18_24 0x0001FC0000
#define BIT_14_17 0x000003C000
#define BIT_7_13  0x0000003F80
#define BIT_0_6   0x000000007F
#define INVALID_CFM 0x10000000000

/* private refactoring function */
bool generatePreservationHeader( ia64_bundle_t * insnPtr, Address & count ) {
	/* How many bundles did we use?  Update the (byte)'count' at the end. */
	int bundleCount = 0;

	/* Preserve the stacked integer registers. */
	IA64_instruction allocInsn = generateAllocInstructionFor( regSpace,
		NUM_LOCALS, NUM_OUTPUT, regSpace->originalOutputs );
	IA64_bundle allocBundle( MIIstop, allocInsn, NOP_I, NOP_I );
	insnPtr[bundleCount++] = allocBundle.getMachineCode();

	/* We'll be using spills, so go ahead and preserve the UNAT. */
	IA64_instruction unatMove = generateApplicationToRegisterMove( AR_UNAT, deadRegisterList[0] );
	IA64_instruction moveSPDown = generateShortImmediateAdd( REGISTER_SP, -8, REGISTER_SP );
	IA64_instruction unatStore = generateRegisterStore( REGISTER_SP, deadRegisterList[0], -8 );
	IA64_instruction memoryNOP( NOP_M );
	IA64_bundle unatMoveBundle( MIIstop, unatMove, memoryNOP, moveSPDown );
	IA64_instruction integerNOP( NOP_I );
	IA64_bundle unatStoreBundle( MIIstop, unatStore, integerNOP, integerNOP );
	insnPtr[bundleCount++] = unatMoveBundle.getMachineCode();
	insnPtr[bundleCount++] = unatStoreBundle.getMachineCode();

	/* Preserve the unstacked integer registers. */
	IA64_instruction r1spill = generateSpillTo( REGISTER_SP, 1, -8 );
	IA64_instruction r2spill = generateSpillTo( REGISTER_SP, 2, -8 );
	IA64_bundle spillBundle( MstopMIstop, r1spill, r2spill, integerNOP );
	insnPtr[bundleCount++] = spillBundle.getMachineCode();
	IA64_instruction r3spill = generateSpillTo( REGISTER_SP, 3, -8 );

	IA64_instruction r8spill = generateSpillTo( REGISTER_SP, 8, -8 );
	spillBundle = IA64_bundle( MstopMIstop, r3spill, r8spill, integerNOP );
	insnPtr[bundleCount++] = spillBundle.getMachineCode();
	IA64_instruction r9spill = generateSpillTo( REGISTER_SP, 9, -8 );
	IA64_instruction r10spill = generateSpillTo( REGISTER_SP, 10, -8 );
	spillBundle = IA64_bundle( MstopMIstop, r9spill, r10spill, integerNOP );
	insnPtr[bundleCount++] = spillBundle.getMachineCode();
	IA64_instruction r11spill = generateSpillTo( REGISTER_SP, 11, -8 );
	spillBundle = IA64_bundle( MstopMIstop, r11spill, memoryNOP, integerNOP );
	insnPtr[bundleCount++] = spillBundle.getMachineCode();

	for( int i = 14, j = 15; i <= 30 && j <= 31; i += 2, j += 2 ) {
		IA64_instruction rIspill = generateSpillTo( REGISTER_SP, i, -8 );
		IA64_instruction rJspill = generateSpillTo( REGISTER_SP, j, -8 );
		spillBundle = IA64_bundle( MstopMIstop, rIspill, rJspill, integerNOP );
		insnPtr[bundleCount++] = spillBundle.getMachineCode();
		} /* end r14 - r31 spill loop */

	/* FIXME: Preserve the floating-point registers.  FIXME: do static analyis. */

	/* Preserve the branch and predicate registers. */
	IA64_instruction moveB0 = generateBranchToRegisterMove( BRANCH_RETURN, deadRegisterList[0] );
	IA64_instruction storeB0 = generateRegisterStore( REGISTER_SP, deadRegisterList[0], -8 );
	IA64_instruction moveB6 = generateBranchToRegisterMove( BRANCH_SCRATCH, deadRegisterList[1] );
	IA64_instruction storeB6 = generateRegisterStore( REGISTER_SP, deadRegisterList[1], -8 );
	IA64_instruction moveB7 = generateBranchToRegisterMove( BRANCH_SCRATCH_ALT, deadRegisterList[2] );
	IA64_instruction storeB7 = generateRegisterStore( REGISTER_SP, deadRegisterList[2], -8 );

	IA64_instruction movePredicates = generatePredicatesToRegisterMove( deadRegisterList[3] );
	IA64_instruction storePredicates = generateRegisterStore( REGISTER_SP, deadRegisterList[3], -8 );

	IA64_bundle moveB0bundle( MIIstop, memoryNOP, movePredicates, moveB0 );
	IA64_bundle storeB0moveB6( MstopMIstop, storeB0, memoryNOP, moveB6 );
	IA64_bundle storeB6moveB7( MstopMIstop, storeB6, memoryNOP, moveB7 );
	IA64_bundle storeB7bundle( MstopMIstop, storeB7, storePredicates, integerNOP );
	insnPtr[bundleCount++] = moveB0bundle.getMachineCode();
	insnPtr[bundleCount++] = storeB0moveB6.getMachineCode();
	insnPtr[bundleCount++] = storeB6moveB7.getMachineCode();
	insnPtr[bundleCount++] = storeB7bundle.getMachineCode();

	/* FIXME: Handle the other application registers and the user mask. */

	/* Preserve PFS.  Note that the -8 in generateRegisterStore is _required_:
	   if we don't 16-byte align the stack, and someone tries to store a float
	   to it later on, we're screwed. */
	IA64_instruction movePFS = generateApplicationToRegisterMove( AR_PFS, deadRegisterList[0] );
	IA64_instruction storePFS = generateRegisterStore( REGISTER_SP, deadRegisterList[0], -8 );
	IA64_bundle pfsBundle( MIIstop, memoryNOP, movePFS, integerNOP );
	insnPtr[bundleCount++] = pfsBundle.getMachineCode();
	pfsBundle = IA64_bundle( MIIstop, storePFS, integerNOP, integerNOP );
	insnPtr[bundleCount++] = pfsBundle.getMachineCode();

	// /* DEBUG */ fprintf( stderr, "Emitted %d-bundle preservation header at 0x%lx\n", bundleCount, (Address)insnPtr ); 

	/* Update the offset. */
	count += (bundleCount * 16);
	return true;
	} /* end generatePreservationHeader() */

/* private refactoring function; assumes that regSpace and
   deadRegisterList are as they were in the corresponding call
   to generatePreservationHeader(). */
bool generatePreservationTrailer( ia64_bundle_t * insnPtr, Address & count ) {
	/* How many bundles did we use?  Update the (byte)'count' at the end. */
	int bundleCount = 0;

	/* We'll need these later. */
	IA64_instruction memoryNOP( NOP_M );
	IA64_instruction integerNOP( NOP_I );

	/* Restore the PFS. */
	IA64_instruction moveSP = generateShortImmediateAdd( REGISTER_SP, 8, REGISTER_SP );
	insnPtr[ bundleCount++ ] = IA64_bundle( MIIstop, memoryNOP, moveSP, integerNOP ).getMachineCode();
	IA64_instruction loadPFS = generateRegisterLoad( deadRegisterList[0], REGISTER_SP, 8 );
	insnPtr[ bundleCount++ ] = IA64_bundle( MIIstop, loadPFS, integerNOP, integerNOP ).getMachineCode();
	IA64_instruction movePFS = generateRegisterToApplicationMove( deadRegisterList[0], AR_PFS );
	insnPtr[ bundleCount++ ] = IA64_bundle( MIIstop, memoryNOP, movePFS, integerNOP ).getMachineCode();

	/* FIXME: Handle the other application registers and the user mask. */

	/* Restore the predicate and branch registers. */ 
	IA64_instruction loadPredicates = generateRegisterLoad( deadRegisterList[3], REGISTER_SP, 8 );
	IA64_instruction movePredicates = generateRegisterToPredicatesMove( deadRegisterList[3], 0x1FFFF );
	
	IA64_instruction loadB7 = generateRegisterLoad( deadRegisterList[2], REGISTER_SP, 8 );
	IA64_instruction moveB7 = generateRegisterToBranchMove( deadRegisterList[2], BRANCH_SCRATCH_ALT );
	IA64_instruction loadB6 = generateRegisterLoad( deadRegisterList[1], REGISTER_SP, 8 );
	IA64_instruction moveB6 = generateRegisterToBranchMove( deadRegisterList[1], BRANCH_SCRATCH );
	IA64_instruction loadB0 = generateRegisterLoad( deadRegisterList[0], REGISTER_SP, 8 );
	IA64_instruction moveB0 = generateRegisterToBranchMove( deadRegisterList[0], BRANCH_RETURN );

	IA64_bundle predicatesBundle( MstopMIstop, loadPredicates, memoryNOP, movePredicates );
	IA64_bundle b7bundle( MstopMIstop, loadB7, memoryNOP, moveB7 );
	IA64_bundle b6bundle( MstopMIstop, loadB6, memoryNOP, moveB6 );
	IA64_bundle b0bundle( MstopMIstop, loadB0, memoryNOP, moveB0 );

	insnPtr[ bundleCount++ ] = predicatesBundle.getMachineCode();
	insnPtr[ bundleCount++ ] = b7bundle.getMachineCode();
	insnPtr[ bundleCount++ ] = b6bundle.getMachineCode();
	insnPtr[ bundleCount++ ] = b0bundle.getMachineCode();

	/* FIXME: Restore the floating-point registers. */

	/* Restore registers 31 to 14, 11-8, and 3-1. */
	IA64_bundle fillBundle;
	for( int i = 31, j = 30; i >= 15 && j >= 14; i -= 2, j -= 2 ) {
		IA64_instruction rIfill = generateFillFrom( i, REGISTER_SP, 8 );
		IA64_instruction rJfill = generateFillFrom( j, REGISTER_SP, 8 );
		fillBundle = IA64_bundle( MstopMIstop, rIfill, rJfill, integerNOP );
		insnPtr[ bundleCount++ ] = fillBundle.getMachineCode();
		} /* end r31 - r14 fill loop */
	for( int i = 11, j = 10; i >= 9 && j >= 8; i -=2, j -= 2 ) {
		IA64_instruction rIfill = generateFillFrom( i, REGISTER_SP, 8 );
		IA64_instruction rJfill = generateFillFrom( j, REGISTER_SP, 8 );
		fillBundle = IA64_bundle( MstopMIstop, rIfill, rJfill, integerNOP );
		insnPtr[ bundleCount++ ] = fillBundle.getMachineCode();
		} /* end r11 - r8 fill loop */
	for( int i = 3; i >= 1; i-- ) {
		IA64_instruction rIfill = generateFillFrom( i, REGISTER_SP, 8 );
		fillBundle = IA64_bundle( MstopMIstop, rIfill, memoryNOP, integerNOP );
		insnPtr[ bundleCount++ ] = fillBundle.getMachineCode();
		} /* end r3 - r1 fill loop */

	/* Restore the UNAT. */	
	IA64_instruction unatLoad = generateRegisterLoad( deadRegisterList[0], REGISTER_SP, 8 );
	IA64_instruction unatMove = generateRegisterToApplicationMove( deadRegisterList[0], AR_UNAT );
	IA64_bundle unatBundle( MstopMIstop, unatLoad, unatMove, integerNOP );
	insnPtr[ bundleCount++ ] = unatBundle.getMachineCode();

	/* Restore the original frame. */
	IA64_instruction allocInsn = generateOriginalAllocFor( regSpace );
	IA64_bundle allocBundle( MIIstop, allocInsn, NOP_I, NOP_I );
	insnPtr[ bundleCount++ ] = allocBundle.getMachineCode();

	// /* DEBUG */ fprintf( stderr, "Emitted %d-bundle preservation trailer at 0x%lx\n", bundleCount, (Address)insnPtr ); 

	/* Update the offset. */
	count += (bundleCount * 16);
	return true;
	} /* end generatePreservationTrailer() */

/* Originally from linux-ia64.C's executingSystemCall(). */
bool needToHandleSyscall( process * proc ) {
	/* This checks if the previous instruction is a break 0x100000. */
	const uint64_t SYSCALL_MASK = 0x01000000000 >> 5;
	uint64_t iip, instruction, rawBundle[2];
	int64_t ipsr_ri, pr;
	IA64_bundle origBundle;
	
	// Bad things happen if you use ptrace on a running process.
	assert( proc->status() == stopped );
	errno = 0;
	
	// Find the correct bundle.
	iip = getPC( proc->getPid() );
	ipsr_ri = P_ptrace( PTRACE_PEEKUSER, proc->getPid(), PT_CR_IPSR, 0 );
	if( errno && (ipsr_ri == -1) ) {
		// Error reading process information.  Should we assert here?
		assert(0);
		}
	ipsr_ri = (ipsr_ri & 0x0000060000000000) >> 41;
	if (ipsr_ri == 0) iip -= 16;  // Get previous bundle, if necessary.
		
	// Read bundle data
	if( ! proc->readDataSpace( (void *)iip, 16, (void *)rawBundle, true ) ) {
		// Could have gotten here because the mutatee stopped right
		// after a jump to the beginning of a memory segment (aka,
		// no previous bundle).  But, that can't happen from a syscall.
		return false;
		}
	
	// Isolate previous instruction.
	origBundle = IA64_bundle( rawBundle[0], rawBundle[1] );
	ipsr_ri = (ipsr_ri + 2) % 3;
	instruction = origBundle.getInstruction(ipsr_ri)->getMachineCode();
	instruction = instruction >> ALIGN_RIGHT_SHIFT;
	
	// Determine predicate register and remove it from instruction.
	pr = P_ptrace( PTRACE_PEEKUSER, proc->getPid(), PT_PR, 0 );
	if (errno && pr == -1) assert(0);
	pr = (pr >> (0x1F & instruction)) & 0x1;
	instruction = instruction >> 5;
	
	return (pr && instruction == SYSCALL_MASK);
	} /* end needToHandleSyscall() */

#include "dlfcn.h"
bool emitSyscallHeader( process * proc, void * insnPtr, Address & baseBytes ) {
	/* Extract the current slotNo. */
	errno = 0;
	uint64_t ipsr = P_ptrace( PTRACE_PEEKUSER, proc->getPid(), PT_CR_IPSR, 0 );
	assert( ! errno );
	uint64_t slotNo = (ipsr & PSR_RI_MASK) >> 41;
	assert( slotNo <= 2 );
	
	/* When the kernel continues execution of the now-stopped process,
	   it may or may not decrement the slot counter to restart the
	   interrupted system call.  Since we need to resume after the iRPC
	   where the kernel thinks we should, we need to change the slot of 
	   the iRPC's terminal break, since we can't alter ipsr.ri otherwise.
	   
	   The header code is templated; we just replace the first bundle with
	   the one appropriate for the known slotNo.  Copy from 'syscallPrefix'
	   to 'syscallPrefixCommon', exclusive.  (The last bundle is an spacing-
	   only NOP bundle.  The iRPC header proper should start there.) */

	assert( ( ((Address)insnPtr) + baseBytes ) % 16 == 0 );
	ia64_bundle_t * bundlePtr = (ia64_bundle_t *)( ((Address)insnPtr) + baseBytes );

	/* FIXME: (static variable) cache the Addresses below. */
	/* Calling dlopen() on a NULL string dlopen()s the current executable.
	   This neatly sidesteps the problem of making sure we're looking at the right library
	   when we look for our code templates.
	   
	   It's also worth noting that labels also marked as functions return pointers to the
	   function pointer, not the function pointer proper.  Don't ask me, I don't know. */
	char * dlErrorString = NULL;
	void * handle = dlopen( NULL, RTLD_NOW );
	assert( handle != NULL );

	void * syscallPrefix = dlsym( handle, "syscallPrefix" );
	if( (dlErrorString = dlerror()) != NULL ) {
		fprintf( stderr, "%s\n", dlErrorString ); assert( 0 );
		}
	assert( syscallPrefix != NULL );

	void * prefixNotInSyscall = dlsym( handle, "prefixNotInSyscall" );
	if( (dlErrorString = dlerror()) != NULL ) {
		fprintf( stderr, "%s\n", dlErrorString ); assert( 0 );
		}
	assert( prefixNotInSyscall != NULL );

	void * prefixInSyscall = dlsym( handle, "prefixInSyscall" );
	if( (dlErrorString = dlerror()) != NULL ) {
		fprintf( stderr, "%s\n", dlErrorString ); assert( 0 );
		}
	assert( prefixInSyscall != NULL );

	void * prefixCommon = dlsym( handle, "prefixCommon" );
	if( (dlErrorString = dlerror()) != NULL ) {
		fprintf( stderr, "%s\n", dlErrorString ); assert( 0 );
		}
	assert( prefixCommon != NULL );		

	void * jumpFromNotInSyscallToPrefixCommon = dlsym( handle, "jumpFromNotInSyscallToPrefixCommon" );
	if( (dlErrorString = dlerror()) != NULL ) {
		fprintf( stderr, "%s\n", dlErrorString ); assert( 0 );
		}
	assert( jumpFromNotInSyscallToPrefixCommon != NULL );	
	
	dlclose( handle );

	/* Since the linker will indirect functions one level,
	   we don't declare any of the assembly blocks functions. */
	Address syscallPrefixAddress = (Address) syscallPrefix;
	Address prefixNotInSyscallAddress = (Address) prefixNotInSyscall;
	Address prefixInSyscallAddress = (Address) prefixInSyscall;
	Address prefixCommonAddress = (Address) prefixCommon;

	Address lengthWithoutTrailingNOPs = prefixCommonAddress - syscallPrefixAddress;
	memcpy( bundlePtr, (const void * )syscallPrefixAddress, lengthWithoutTrailingNOPs );
	baseBytes += lengthWithoutTrailingNOPs;

	/* Likewise, we need to rewrite the nop.b bundle at jumpFromNotInSyscallToPrefixCommon
	   because the assembler jumps are indirected.  (Why can't I force a relative jump?!) */
	Address jumpAddress = (Address) jumpFromNotInSyscallToPrefixCommon;
	IA64_instruction memoryNOP( NOP_M );
	IA64_instruction_x branchPastInSyscall = generateLongBranchTo( prefixCommonAddress - jumpAddress );
	IA64_bundle branchPastInSyscallBundle( MLXstop, memoryNOP, branchPastInSyscall );
	* (bundlePtr + ( (jumpAddress - syscallPrefixAddress) / 16 )) = branchPastInSyscallBundle.getMachineCode();

	/* Construct the slot-specific jump bundle. */
	IA64_bundle entryJumpBundle;
	IA64_instruction jumpToInSyscall = generateShortImmediateBranch( prefixInSyscallAddress - syscallPrefixAddress );
	IA64_instruction jumpToNotInSyscall = generateShortImmediateBranch( prefixNotInSyscallAddress - syscallPrefixAddress );
	switch( slotNo ) {
		case 0:
			entryJumpBundle = IA64_bundle( BBBstop, jumpToNotInSyscall, NOP_B, jumpToInSyscall );
			break;
		case 1:
			entryJumpBundle = IA64_bundle( BBBstop, jumpToInSyscall, jumpToNotInSyscall, NOP_B );
			break;
		case 2:
			entryJumpBundle = IA64_bundle( BBBstop, jumpToNotInSyscall, NOP_B, jumpToInSyscall );
			break;
		}
	bundlePtr[ 0 ] = entryJumpBundle.getMachineCode();

fprintf( stderr, "* iRPC system call handler (prefix) generated at 0x%lx\n", (Address) bundlePtr );
	return true;
	} /* end emitSystemCallHeader() */

/* Required by process.C */
bool rpcMgr::emitInferiorRPCheader( void * insnPtr, Address & baseBytes ) {
	/* Extract the CFM. */
	errno = 0;
	uint64_t currentFrameMarker = P_ptrace( PTRACE_PEEKUSER, proc_->getPid(), PT_CFM, 0 );
	assert( ! errno );

	/* Construct the corresponding registerSpace. */
	int rrb_pr = (currentFrameMarker & BIT_32_37 ) >> 32;
	int rrb_fr = (currentFrameMarker & BIT_25_31 ) >> 25;
	int rrb_gr = (currentFrameMarker & BIT_18_24 ) >> 18;
	int soRotating = (currentFrameMarker & BIT_14_17 ) >> 14;
	int soLocals = (currentFrameMarker & BIT_7_13 ) >> 7;
	int soFrame = (currentFrameMarker & BIT_0_6 ) >> 0;
	int soOutputs = soFrame - soLocals;

	/* FIXME: */ if( ! ( 0 == rrb_pr == rrb_fr == rrb_gr ) ) { assert( 0 ); }
	/* FIXME: */ if( currentFrameMarker == INVALID_CFM ) { assert( 0 ); }

	/* Set regSpace for the code generator. */
	Register deadRegisterList[NUM_LOCALS + NUM_OUTPUT];
	for( int i = 0; i < NUM_LOCALS + NUM_OUTPUT; i++ ) {
		deadRegisterList[i] = 32 + soFrame + i;
		} /* end deadRegisterList population */
	registerSpace rs( NUM_LOCALS + NUM_OUTPUT, deadRegisterList, 0, NULL );
	rs.originalLocals = soLocals;
	rs.originalOutputs = soOutputs;
	rs.originalRotates = soRotating;

	/* The code generator needs to know about the register space
	   as well, so just take advantage of the existing globals. */
	* regSpace = rs;
	memcpy( ::deadRegisterList, deadRegisterList, 16 * sizeof( Register ) );

	assert( ( ((Address)insnPtr) + baseBytes ) % 16 == 0 );
	ia64_bundle_t * bundlePtr = (ia64_bundle_t *)( ((Address)insnPtr) + baseBytes );

	if( needToHandleSyscall( proc_ ) ) {
		if( ! emitSyscallHeader( proc_, insnPtr, baseBytes ) ) { return false; }
		bundlePtr = (ia64_bundle_t *)( ((Address)insnPtr) + baseBytes );
		}
	else {
		/* We'll be adjusting the PC to the start of the preservation code,
		   but we can't change the slot number ([i]psr.ri), so we need to soak
		   up the extra slots with nops. */
		bundlePtr[0] = IA64_bundle( MIIstop, NOP_M, NOP_I, NOP_I ).getMachineCode();
		baseBytes += 16; bundlePtr++;
		}

	/* Generate the preservation header. */
	return generatePreservationHeader( bundlePtr, baseBytes );
	} /* end emitInferiorRPCheader() */

bool emitSyscallTrailer( void * insnPtr, Address & baseBytes, uint64_t slotNo ) {
	/* Copy the template from 'syscallSuffix' to 'suffixExitPoint' (inclusive),
	   and replace the bundle at 'suffixExitPoint' with one which has 
	   predicated SIGILLs in the right places. */

	assert( ( ((Address)insnPtr) + baseBytes ) % 16 == 0 );
	ia64_bundle_t * bundlePtr = (ia64_bundle_t *)( ((Address)insnPtr) + baseBytes );

	/* Grab those two addresses. */
	char * dlErrorString = NULL;
	void * handle = dlopen( NULL, RTLD_NOW );
	assert( handle != NULL );

	void * syscallSuffix = dlsym( handle, "syscallSuffix" );
	if( (dlErrorString = dlerror()) != NULL ) {
		fprintf( stderr, "%s\n", dlErrorString ); assert( 0 );
		}
	assert( syscallSuffix != NULL );
	
	void * suffixExitPoint= dlsym( handle, "suffixExitPoint" );
	if( (dlErrorString = dlerror()) != NULL ) {
		fprintf( stderr, "%s\n", dlErrorString ); assert( 0 );
		}
	assert( suffixExitPoint != NULL );
	
	dlclose( handle );
	
	/* Copy the code template. */
	Address syscallSuffixAddress = (Address) syscallSuffix;
	Address suffixExitPointAddress = (Address) suffixExitPoint;
	
	/* Trailing NOPs are replaced by the constructed SIGILLs. */
	Address lengthWithTrailingNOPs = ( suffixExitPointAddress + 0x10 ) - syscallSuffixAddress;
	memcpy( bundlePtr, (const void *)syscallSuffixAddress, lengthWithTrailingNOPs );
	baseBytes += lengthWithTrailingNOPs;
	
	/* Construct the predicated SIGILLs for the bundle at suffixExitPointAddress. */
	IA64_bundle predicatedBreakBundle;
	switch( slotNo ) {
		case 0: {
			/* slot 0 NOT, slot 2 IS;; p1 and p2 from assembler */
			IA64_instruction notInSyscallBreak( TRAP_M );
			IA64_instruction inSyscallBreak( TRAP_I );
			notInSyscallBreak = predicateInstruction( 1, notInSyscallBreak );
			inSyscallBreak = predicateInstruction( 2, inSyscallBreak );
			predicatedBreakBundle = IA64_bundle( MMIstop, notInSyscallBreak, NOP_M, inSyscallBreak );
			} break;
			
		case 1: {
			/* slot 1 NOT, slot 0 IS */
			IA64_instruction notInSyscallBreak( TRAP_M );
			IA64_instruction inSyscallBreak( TRAP_M );
			notInSyscallBreak = predicateInstruction( 1, notInSyscallBreak );
			inSyscallBreak = predicateInstruction( 2, inSyscallBreak );
			predicatedBreakBundle = IA64_bundle( MMIstop, inSyscallBreak, notInSyscallBreak, NOP_I );
			} break;
			
		case 2: {
			/* slot 2 NOT, slot 1 IS */
			IA64_instruction memoryNOP( NOP_M );
			IA64_instruction notInSyscallBreak( TRAP_I );
			IA64_instruction inSyscallBreak( TRAP_M );
			notInSyscallBreak = predicateInstruction( 1, notInSyscallBreak );
			inSyscallBreak = predicateInstruction( 2, inSyscallBreak );
			predicatedBreakBundle = IA64_bundle( MMIstop, memoryNOP, inSyscallBreak, notInSyscallBreak );
			} break;
			
		default:
			assert( 0 );
		} /* end slotNo switch */		
	* (bundlePtr + ((lengthWithTrailingNOPs - 0x10) / 16) ) = predicatedBreakBundle.getMachineCode();

fprintf( stderr, "* iRPC system call handler (suffix) generated at 0x%lx\n", (Address) bundlePtr );
	return true;
	} /* end emitSyscallTrailer() */

/* Required by process.C */
bool rpcMgr::emitInferiorRPCtrailer( void * insnPtr, Address & offset,
			unsigned & breakOffset, bool shouldStopForResult,
			unsigned & stopForResultOffset,
			unsigned & justAfter_stopForResultOffset ) {
	/* We'll need two of these. */
	IA64_bundle trapBundle = generateTrapBundle();

	/* Get a real instruction pointer. */
	assert( ( ((Address)insnPtr) + offset ) % 16 == 0 );
	ia64_bundle_t * bundlePtr = (ia64_bundle_t *)( ((Address)insnPtr) + offset);
	unsigned int bundleCount = 0;

	if( shouldStopForResult ) {
		fprintf( stderr, "* iRPC will stop for result.\n" );
		stopForResultOffset = offset + (bundleCount * 16);
		bundlePtr[ bundleCount++ ] = trapBundle.getMachineCode();
		justAfter_stopForResultOffset = offset + (bundleCount * 16);
		/* Make sure that we eat the ipsr.ri with a nop. */
		bundlePtr[ bundleCount++ ] = IA64_bundle( MIIstop, NOP_M, NOP_I, NOP_I ).getMachineCode();
		offset += bundleCount * 16;
		} /* end if we're interested in the result. */

	/* Generate the restoration code. */
	bundlePtr = (ia64_bundle_t *)( ((Address)insnPtr) + offset );
	generatePreservationTrailer( bundlePtr, offset );

	/* Regenerate the bundle ptr, etc, to reflect the preservation trailer. */
	assert( ( ((Address)insnPtr) + offset ) % 16 == 0 );
	bundlePtr = (ia64_bundle_t *)( ((Address)insnPtr) + offset );
	bundleCount = 0;
	
	/* The SIGILL for the demon needs to happen in the instruction slot
	   corresponding to ipsr.ri so that the mutatee resumes in the correct
	   location after the daemon adjusts its PC. */
	errno = 0;
	uint64_t ipsr = P_ptrace( PTRACE_PEEKUSER, proc_->getPid(), PT_CR_IPSR, 0 );
	assert( ! errno );
	uint64_t slotNo = (ipsr & PSR_RI_MASK) >> 41;
	assert( slotNo <= 2 );

	if( needToHandleSyscall( proc_ ) ) {
		if( ! emitSyscallTrailer( insnPtr, offset, slotNo ) ) { return false; }
		breakOffset = offset - 16;
		}
	else {
		IA64_bundle slottedTrapBundle;
		switch( slotNo ) {
			case 0:
				slottedTrapBundle = IA64_bundle( MIIstop, TRAP_M, NOP_I, NOP_I );
				break;
			case 1:
				slottedTrapBundle = IA64_bundle( MIIstop, NOP_M, TRAP_I, NOP_I );
				break;
			case 2:
				slottedTrapBundle = IA64_bundle( MIIstop, NOP_M, NOP_I, TRAP_I );
				break;
			} /* end slotNo switch */

		breakOffset = offset + (bundleCount * 16);
		bundlePtr[ bundleCount++ ] = slottedTrapBundle.getMachineCode();
		offset += 16;
		} /* end if we don't need to handle a syscall */

	/* If the demon drops the ball, make sure the mutatee suicides. */
	bundlePtr = (ia64_bundle_t *)( ((Address)insnPtr) + offset );		
	bundlePtr[ 0 ] = trapBundle.getMachineCode();
	offset += 16;
	return true;
	} /* end emitInferiorRPCtrailer() */

/**
 * We use the same assembler for the basetramp and the header
 * and trailer of an inferior RPC; a basetramp is two header/trailer
 * pairs, with inside each pair and the emulated instructions between
 * them.  The same registerSpace must be used throughout the code
 * generation (including the minitramps) for the results to be sensible.
 */

/* private refactoring function */
#define MAX_BASE_TRAMP_SIZE (16 * 128)
#define NEAR_ADDRESS 0x2000000000000000		/* The lower end of the shared memory segment. */
trampTemplate * installBaseTramp( instPoint * & location, process * proc ) { // FIXME: updatecost
	fprintf( stderr, "* Installing base tramp.\n" );
	/* Allocate memory and align as needed. */
	  trampTemplate * baseTramp = new trampTemplate(location, proc);

	bool allocErr; Address allocatedAddress = proc->inferiorMalloc( MAX_BASE_TRAMP_SIZE, anyHeap, NEAR_ADDRESS, & allocErr );
	if( allocErr ) { fprintf( stderr, "Unable to allocate base tramp, aborting.\n" ); abort(); }
	if( allocatedAddress % 16 != 0 ) { allocatedAddress = allocatedAddress - (allocatedAddress % 16); }

	/* Initialize the baseTramp. */
	baseTramp->baseAddr = allocatedAddress;
	baseTramp->size = 0;

	/* Acquire the base address of the object we're instrumenting. */
	Address baseAddress; assert( proc->getBaseAddress( location->iPgetOwner(), baseAddress ) );

	/* Determine this instrumentation point's regSpace and deadRegisterList (global variables)
	   for use by the rest of the code generator.  The generatePreservation*() functions need
	   this information as well. */
	bool staticallyAnalyzed = defineBaseTrampRegisterSpaceFor( location, regSpace, deadRegisterList );
	if( ! staticallyAnalyzed ) {
		fprintf( stderr, "FIXME: Dynamic determination of register frame required but not yet implemented, aborting.\n" );
		assert( 0 );
		}

	/* Generate the base tramp in insnPtr, and copy to allocatedAddress. */
	unsigned int bundleCount = 0;
	ia64_bundle_t instructions[ MAX_BASE_TRAMP_SIZE ];
	ia64_bundle_t * insnPtr = instructions;

	/* CODEGEN: Insert the skipPreInsn nop bundle. */
	IA64_bundle nopBundle( MIIstop, NOP_M, NOP_I, NOP_I );
	baseTramp->skipPreInsOffset = baseTramp->size;
	insnPtr[ bundleCount++ ] = nopBundle.getMachineCode();
	baseTramp->size += 16;

	/* Insert the preservation header. */
	insnPtr = (ia64_bundle_t *)( ((Address)instructions) + baseTramp->size );
	generatePreservationHeader( insnPtr, (Address &) baseTramp->size );

	/* Update insnPtr to reflect the size of the preservation header. */
	insnPtr = (ia64_bundle_t *)( ((Address)instructions) + baseTramp->size );
	bundleCount = 0;

	/* FIXME: Insert the recursion header.  If we don't do any instrumentation (that is,
	   we took the jump at skipPreInsOffset), we don't need to avoid recursing in it.  If
	   we are doing instrumentation, we don't have to worry about stomping on the
	   predicate bits.  If the recursion gaurd is set, jump to the second copy of the 
	   preservation trailer.  If it isn't, use reverse-predicated instructions to set the
	   recursion guard here, and unset just before the emulated instructions.  (So that if we
	   skip the instrumentation, we don't unset the guard.) */
	
	/* CODEGEN: Insert the localPre nop bundle.  (Jump to first minitramp.) */
	baseTramp->localPreOffset = baseTramp->size;
	insnPtr[bundleCount++] = nopBundle.getMachineCode(); baseTramp->size += 16;
	baseTramp->localPreReturnOffset = baseTramp->size;

	/* Insert the preservation trailer. */
	insnPtr = (ia64_bundle_t *)( ((Address)instructions) + baseTramp->size );
	generatePreservationTrailer( insnPtr, (Address &) baseTramp->size );

	/* Update insnPtr to reflect the size of the preservation trailer. */
	insnPtr = (ia64_bundle_t *)( ((Address)instructions) + baseTramp->size );
	bundleCount = 0;

	/* Emulate the relocated instructions.  Since we're using the owner to read the
	   instructions, don't adjust by the base pointer. */
	Address installationPoint = location->iPgetAddress();
	installationPoint = installationPoint - (installationPoint % 16);
	Address installationPointAddress = (Address)location->iPgetOwner()->getPtrToInstruction( installationPoint );
	assert( installationPointAddress % 16 == 0 );
	IA64_bundle bundleToEmulate( * (const ia64_bundle_t *) installationPointAddress );
	baseTramp->emulateInsOffset = baseTramp->size;
	emulateBundle( bundleToEmulate, installationPoint, insnPtr, bundleCount, (unsigned int &)(baseTramp->size), allocatedAddress );

	/* CODEGEN: Replace the skipPre nop bundle with a jump from it to baseTramp->emulateInsOffset. */
	unsigned int skipPreJumpBundleOffset = baseTramp->skipPreInsOffset / 16;
	IA64_instruction memoryNOP( NOP_M );
	IA64_instruction_x skipPreJump = generateLongBranchTo( baseTramp->emulateInsOffset - baseTramp->skipPreInsOffset ); 
	IA64_bundle skipPreJumpBundle( MLXstop, memoryNOP, skipPreJump );
	instructions[skipPreJumpBundleOffset] = skipPreJumpBundle.getMachineCode();

	/* CODEGEN: Insert the skipPost nop bundle. */
	baseTramp->skipPostInsOffset = baseTramp->size;
	insnPtr[bundleCount++] = nopBundle.getMachineCode(); baseTramp->size += 16;

	/* Insert the preservation header. */
	insnPtr = (ia64_bundle_t *)( ((Address)instructions) + baseTramp->size );
	generatePreservationHeader( insnPtr, (Address &) baseTramp->size );

	/* Update insnPtr to reflect the size of the preservation header. */
	insnPtr = (ia64_bundle_t *)( ((Address)instructions) + baseTramp->size );
	bundleCount = 0;

	/* FIXME: Insert the recursion guard. */

	/* CODEGEN: Insert the localPost nop bundle. */
	baseTramp->localPostOffset = baseTramp->size;
	insnPtr[bundleCount++] = nopBundle.getMachineCode(); baseTramp->size += 16;
	baseTramp->localPostReturnOffset = baseTramp->size;

	/* Insert the preservation trailer. */
	insnPtr = (ia64_bundle_t *)( ((Address)instructions) + baseTramp->size );
	generatePreservationTrailer( insnPtr, (Address &) baseTramp->size );

	/* Update insnPtr to reflect the size of the preservation trailer. */
	insnPtr = (ia64_bundle_t *)( ((Address)instructions) + baseTramp->size );
	bundleCount = 0;

	/* Insert the jump back to normal execution. */
	baseTramp->returnInsOffset = baseTramp->size;
	Address returnAddress = installationPoint + baseAddress;
	returnAddress = returnAddress - (returnAddress % 16) + 16;
	IA64_instruction_x returnFromTrampoline = generateLongBranchTo( returnAddress - (baseTramp->baseAddr + baseTramp->size) );
	IA64_bundle returnBundle( MLXstop, memoryNOP, returnFromTrampoline );
	insnPtr[bundleCount++] = returnBundle.getMachineCode(); baseTramp->size += 16;

	/* Replace the skipPost nop bundle with a jump from it to baseTramp->returnInsOffset. */
	unsigned int skipPostJumpBundleOffset = baseTramp->skipPostInsOffset / 16;
	IA64_instruction_x skipPostJump = generateLongBranchTo( baseTramp->returnInsOffset - baseTramp->skipPostInsOffset ); 
	IA64_bundle skipPostJumpBundle( MLXstop, memoryNOP, skipPostJump );
	instructions[skipPostJumpBundleOffset] = skipPostJumpBundle.getMachineCode();

	/* Copy the instructions from hither to thither. */
	InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( allocatedAddress, proc );
	iAddr.writeBundlesFrom( (unsigned char *)instructions, baseTramp->size / 16  );

	/* Install the jump to the base tramp. */
	Address originatingAddress = returnAddress - 16;
	IA64_instruction_x jumpToBaseInstruction = generateLongBranchTo( allocatedAddress - originatingAddress );
	IA64_bundle jumpToBaseBundle( MLXstop, memoryNOP, jumpToBaseInstruction );
	InsnAddr jAddr = InsnAddr::generateFromAlignedDataAddress( originatingAddress, proc );
	jAddr.replaceBundleWith( jumpToBaseBundle );

	fprintf( stderr, "* Installed base tramp at 0x%lx, from 0x%lx (local 0x%lx)\n", baseTramp->baseAddr, originatingAddress, (Address)instructions );
	return baseTramp;
	} /* end installBaseTramp() */

/* Required by inst.C */
trampTemplate * findOrInstallBaseTramp( process * proc, instPoint * & location,
					returnInstance * & retInstance,
					bool trampRecursiveDesired,
					bool noCost, bool & deferred ) {
	/* TODO: handle if trampRecursiveDesired; handle if noCast, handle if deferred. */
 
	/* proc->baseMap is in the relevant variable here; check to see if the given
	   instPoint already has a base tramp ("find"), and if not, "install" one. */
	
	/* "find" */
	if( proc->baseMap.defines( location ) ) { return proc->baseMap[location]; }

	/* "install" */
	trampTemplate * installedBaseTramp = installBaseTramp( location, proc );
	proc->baseMap[location] = installedBaseTramp;

	/* Generate the returnInstance, which will be written to
	   the mutatee by installReturnInstance().  Note that we do NOT
	   store a full bundle, because of how things are declared. */
	IA64_instruction_x * longBranchInstruction = new IA64_instruction_x();
	Address returnFrom = installedBaseTramp->baseAddr + installedBaseTramp->size;
	Address returnTo; assert( proc->getBaseAddress( location->iPgetOwner(), returnTo ) );
	returnTo += location->iPgetAddress();
	fprintf( stderr, "* Generating return instance from 0x%lx to 0x%lx\n", returnFrom, returnTo );
	* longBranchInstruction = generateLongBranchTo( returnTo - returnFrom );
	retInstance = new returnInstance( 3, longBranchInstruction, 16, returnFrom, 16 );

	return installedBaseTramp;
	} /* end findOrInstallBaseTramp() */

void generateMTpreamble(char *, Address &, process *) {
	assert( 0 );	// We don't yet handle multiple threads.
	} /* end generateMTpreamble() */

/* Required by ast.C */
Register emitR( opCode op, Register src1, Register src2, Register dest,
					 char * ibuf, Address & base, bool noCost,
					 const instPoint * /* location */, bool for_multithreaded) {
	/* FIXME: handle noCost */
	switch( op ) {
		case getParamOp: {
			/* src1 is the (incoming) parameter we want. */
			if( src1 >= 8 ) {
				assert( 0 );
				} else {
				/* FIXME: is the global registerSpace the right one? */
				unsigned int outputZero = regSpace->getRegSlot( 0 )->number;
				emitRegisterToRegisterCopy( src1 + outputZero, dest, ibuf, base, NULL );
				return dest;	// Why do we bother doing this?
				} /* end if it's a parameter in a register. */
		} break;

                case getRetValOp: {
                        /* WARNING: According to the Itanium Software Conventions
                           Guide, the return value can use multiple registers (r8-11).
                           However, since Dyninst can't handle anything larger than
                           64 bits (for now), we'll just return r8.

                           This should be valid for the general case.
                        */
                        Register retVal = (Register)8;
                        emitRegisterToRegisterCopy(retVal, dest, ibuf, base, NULL);
                        return dest;
                } break;

		default:
			assert( 0 ) ;
		break;
		} /* end switch */
	} /* end emitR() */

/* Required by func-reloc.C */
bool pd_Function::loadCode(const image * owner, process * proc,
		instruction * & oldCode, unsigned & numberOfInstructions,
		Address & firstAddress) { assert( 0 ); return false; }

/* Required by func-reloc.C */
bool pd_Function::PA_attachBranchOverlaps(
		LocalAlterationSet * temp_alteration_set,
		Address baseAddress, Address firstAddress,
		instruction loadedCode[], unsigned numberOfInstructions,
		int codeSize )  { assert( 0 ); return false; }

/* Required by BPatch_init.C */
void initDefaultPointFrequencyTable() {
	/* On other platforms, this loads data into a table that's only used
	   in the function getPointFrequency, which is never called, so we'll do nothing. */
	} /* end initDefaultPointFrequencyTable() */

/* Required by inst.C, ast.C */
Address emitA( opCode op, Register src1, Register src2, Register dest,
		char * ibuf, Address & base, bool noCost ) {  // FIXME: cost?
	/* Emit the give opcode, returning its relative offset. */
        assert( (((Address)ibuf + base) % 16) == 0 );
        ia64_bundle_t * rawBundlePointer = (ia64_bundle_t *)((Address)ibuf + base);

	switch( op ) {
		case trampTrailer: {
			rawBundlePointer[0] = IA64_bundle( MIIstop, NOP_M, NOP_I, NOP_I ).getMachineCode();
			base += 16;
			return (base - 16);
			break; }

		case ifOp: {
			/* If src1 == 0, branch relative by immediate 'dest'. */
			IA64_instruction compareInsn = generateComparison( eqOp, src1, src1, REGISTER_ZERO );
			IA64_instruction_x branchInsn = predicateLongInstruction( src1, generateLongBranchTo( dest ) );
			IA64_bundle compareBundle( MIIstop, compareInsn, NOP_I, NOP_I );
			IA64_instruction memoryNOP( NOP_M );
			IA64_bundle branchBundle( MLXstop, memoryNOP, branchInsn );
			rawBundlePointer[0] = compareBundle.getMachineCode();
			rawBundlePointer[1] = branchBundle.getMachineCode();
			base += 32;
			return (base - 32);
			break; }

		default:
			assert( 0 );
			return 0;
		} /* end op switch */
	} /* end emitA() */

/* Required by ast.C */
bool doNotOverflow( int value ) { 
	/* To be on the safe side, we'll say it always overflows,
	   since it's not clear to me which immediate size(s) are going to be used. */
	return false;
	} /* end doNotOverflow() */

/* Required by func-reloc.C */
bool pd_Function::PA_attachOverlappingInstPoints(
		LocalAlterationSet * temp_alteration_set,
		Address baseAddress, Address firstAddress,
		instruction * loadedCode , int codeSize ) { assert( 0 ); return false; }

/* Required by inst.C; install a single mini-tramp */
void installTramp( miniTrampHandle *mtHandle,
				   process *proc,
				   char *code, int codeSize) {
	/* Book-keeping. */
	totalMiniTramps++; insnGenerated += codeSize / 16;
	
	/* Write the minitramp. */
	fprintf( stderr, "* Installing minitramp at 0x%lx\n", mtHandle->miniTrampBase );
	proc->writeDataSpace( (caddr_t)mtHandle->miniTrampBase, codeSize, code );
	
	/* Make sure that it's not skipped. */
	Address insertNOPAt = mtHandle->baseTramp->baseAddr;
	if( mtHandle->when == callPreInsn && mtHandle->baseTramp->prevInstru == false ) {
		mtHandle->baseTramp->cost += mtHandle->baseTramp->prevBaseCost;
		insertNOPAt += mtHandle->baseTramp->skipPreInsOffset;
		mtHandle->baseTramp->prevInstru = true;
		} 
	else if( mtHandle->when == callPostInsn && mtHandle->baseTramp->postInstru == false ) {
		mtHandle->baseTramp->cost += mtHandle->baseTramp->postBaseCost;
		insertNOPAt += mtHandle->baseTramp->skipPostInsOffset;
		mtHandle->baseTramp->postInstru = true;
		}
	
	/* Write the NOP */
	InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( insertNOPAt, proc );
	iAddr.replaceBundleWith( IA64_bundle( MIIstop, NOP_M, NOP_I, NOP_I ) );
} /* end installTramp() */

/* Required by func-reloc.C */
void pd_Function::copyInstruction( instruction & newInsn,
	instruction & oldInsn, unsigned & codeOffset) { assert( 0 ); }

/* Required by func-reloc.C */
bool pd_Function::fillInRelocInstPoints(
		const image * owner, process * proc,   
		instPoint * & location, relocatedFuncInfo * & reloc_info,
		Address mutatee, Address mutator, instruction oldCode[],
		Address newAdr, instruction newCode[],
		LocalAlterationSet & alteration_set ) { assert( 0 ); return false; }

/* Required by ast.C */
void emitVstore( opCode op, Register src1, Register src2, Address dest,
		char * ibuf, Address & base, bool noCost, int size ) {
	assert( (((Address)ibuf + base) % 16) == 0 );
	ia64_bundle_t * rawBundlePointer = (ia64_bundle_t *)((Address)ibuf + base);

	switch( op ) {
		case storeOp: {
			IA64_instruction memoryNOP( NOP_M );
			IA64_instruction_x loadAddressInsn = generateLongConstantInRegister( src2, dest );
			IA64_bundle loadBundle( MLXstop, memoryNOP, loadAddressInsn );
			rawBundlePointer[0] = loadBundle.getMachineCode();
			base += 16;

			IA64_instruction storeInsn = generateRegisterStore( src2, src1 );
			IA64_bundle storeBundle( MIIstop, storeInsn, NOP_I, NOP_I );
			rawBundlePointer[1] = storeBundle.getMachineCode();
			base += 16;
			break; }

		case storeFrameRelativeOp:  // FIXME
			assert( 0 );
			break;

		case storeIndirOp:  // FIXME
			assert( 0 );
			break;

                default:
			fprintf( stderr, "emitVstore(): op not a load, aborting.\n" );
			abort();
			break;
		} /* end switch */
	} /* end emitVstore() */

/* Required by ast.C */
void emitJmpMC( int condition, int offset, char * baseInsn, Address & base ) { assert( 0 ); }

/* Required by inst.C */
void generateNoOp( process * proc, Address addr ) { assert( 0 ); }

/* Required by func-reloc.C */
bool PushEIPmov::RewriteFootprint( Address oldBaseAdr, Address & oldAdr,
				Address newBaseAdr, Address & newAdr,
				instruction oldInstructions[],
				instruction newInstructions[],
				int & oldInsnOffset, int & newInsnOffset,
				int newDisp, unsigned & codeOffset,
				unsigned char * code) { assert( 0 ); return false; }

/* -------- implementation of InsnAddr -------- */

InsnAddr InsnAddr::generateFromAlignedDataAddress( Address addr, process * p ) {
	assert( addr % 16 == 0 );  assert( p != NULL );
	InsnAddr generated( addr, p );
	return generated;
	} /* end gFADA() */

bool InsnAddr::writeMyBundleFrom( const unsigned char * savedCodeBuffer ) {
	/* Aligns the instruction address and copies sixteen bytes from
	   the savedCodeBuffer to that address. */

	unsigned short offset = encodedAddress % 16;
	Address alignedAddress = encodedAddress - offset;

	return myProc->writeTextSpace( (void *)alignedAddress, 16, savedCodeBuffer );
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
	ia64_bundle_t theBundle = bundle.getMachineCode();

	return myProc->writeTextSpace( (void *)alignedAddress, 16, & theBundle );
	} /* end replaceInstructionWith() */

bool InsnAddr::saveBundlesTo( unsigned char * savedCodeBuffer, unsigned int numberOfBundles ) {
	/* Align the instruction address and copy (numberOfBundles * 16) bytes 
	   from there to savedCodeBuffer. */
	unsigned short offset = encodedAddress % 16;
	Address alignedAddress = encodedAddress - offset;
	
	return myProc->readDataSpace( (void *)alignedAddress, (numberOfBundles * 16), savedCodeBuffer, true );
	} /* end saveBundlesTo() */

bool InsnAddr::writeBundlesFrom( unsigned char * savedCodeBuffer, unsigned int numberOfBundles ) {
	/* Align the instruction address and copy (numberOfBundles * 16) bytes 
	   from there to savedCodeBuffer. */
	unsigned short offset = encodedAddress % 16;
	Address alignedAddress = encodedAddress - offset;
	
	return myProc->writeTextSpace( (void *)alignedAddress, (numberOfBundles * 16), savedCodeBuffer );
	} /* end saveBundlesTo() */

bool InsnAddr::replaceBundlesWith( const IA64_bundle * replacementBundles, unsigned int numberOfReplacementBundles ) {
	/* Align the instruction address and copy (numberOfReplacementBundles * 16) bytes
	   from the savedCodeBuffer to that address. */

	unsigned short offset = encodedAddress % 16;
	Address alignedAddress = encodedAddress - offset;

	ia64_bundle_t * rawReplacementBundles = (ia64_bundle_t *)malloc( sizeof( ia64_bundle_t ) * numberOfReplacementBundles );
	for( unsigned int i = 0; i < numberOfReplacementBundles; i++ ) {
		rawReplacementBundles[i] = replacementBundles[i].getMachineCode();
		}

	return myProc->writeTextSpace( (void *)alignedAddress, (numberOfReplacementBundles * 16), rawReplacementBundles );
	} /* end replaceBundlesWith() */

bool InsnAddr::writeStringAtOffset( unsigned int offsetInBundles, const char * pdstring, unsigned int length ) {
	/* Align the instruction address and compute in the offset. */
	unsigned short offset = encodedAddress % 16;
	Address alignedOffset = encodedAddress - offset + (offsetInBundles * 16);

	return myProc->writeTextSpace( (void *)alignedOffset, length + 1, pdstring );
	} /* end writeStringAtOffset() */

uint64_t InsnAddr::operator * () {
	/* Align the instruction address and compute in the offset. */
	unsigned short offset = encodedAddress % 16;
	Address alignedOffset = encodedAddress - offset;

	/* Construct a decodable bundle from the machine code. */
	ia64_bundle_t rawBundle;
	if( ! myProc->readDataSpace( (const void *)alignedOffset, 16, (void *)& rawBundle, true ) ) {
		fprintf( stderr, "Failed to read from instruction address 0x%lX, aborting.\n", alignedOffset );
		abort();
		}
	IA64_bundle bundle( rawBundle );

	/* Return the appropriate left-aligned machine code. */
	return bundle.getInstruction( offset )->getMachineCode();
	} /* end operator * () */

/* Implementation of IA64_iterator. */

IA64_iterator::IA64_iterator( Address addr ) : encodedAddress( addr ) {
	currentBundle = IA64_bundle( *(ia64_bundle_t*)addr );
        } /* end constructor */

bool operator < ( IA64_iterator lhs, IA64_iterator rhs ) {
	return ( lhs.encodedAddress < rhs.encodedAddress );
	} /* end operator < () */

IA64_instruction * IA64_iterator::operator * () {
	/* Compute the instruction slot. */
	unsigned short offset = encodedAddress % 16;

	/* Are we looking at a long instruction? */
	if( currentBundle.hasLongInstruction() && offset != 0 ) {
		return currentBundle.getLongInstruction();
		}

	/* Return the appropriate instruction. */
	return currentBundle.getInstruction( offset );
	} /* end operator * */

/* FIXME: rewrite in terms of prefix increment. */
const IA64_iterator IA64_iterator::operator++ ( int ) {
	/* Preserve the return value. */
	IA64_iterator rv = * this;

	/* Align the instruction address and compute in the offset. */
	unsigned short offset = encodedAddress % 16;
	Address alignedOffset = encodedAddress - offset;

	/* Advance an instruction. */
	if( currentBundle.hasLongInstruction() && offset == 0 ) { offset += 2; }
	else { offset++; }
	
	/* Maintain the invariant. */
	if( offset > 2 ) {
		encodedAddress = alignedOffset + 16;
		currentBundle = IA64_bundle( *(ia64_bundle_t*)encodedAddress );
		} else {
		encodedAddress++;
		}

	/* Return the proper value. */
	return rv;
	} /* end operator ++ */

/* Implementation of instPoint */
instPoint::instPoint( Address encodedAddress, pd_Function * pdfn, const image * owner, IA64_instruction * theInsn )  : myAddress( encodedAddress ), myPDFunction( pdfn ), myOwner( owner ), myInstruction( theInsn ) {
	/* Extract from that its target address. */
	myTargetAddress = myInstruction->getTargetAddress() + myPDFunction->addr();
	} /* end instPoint constructor */

// Get the absolute address of an instPoint
Address instPoint::iPgetAddress(process *p) const {
    if (!p) return myAddress;
    Address baseAddr;
    p->getBaseAddress(iPgetOwner(), baseAddr);
    return myAddress + baseAddr;
}


/* The IA-64 instrumentation code always displaces a single three-instruction bundle.
   We'll return the machine code, since that's easier and the type of insns isn't specificed. */ 
int BPatch_point::getDisplacedInstructions( int maxSize, void * insns ) {
	const IA64_instruction * insn = point->iPgetInstruction();
	const IA64_bundle * bundle = insn->getMyBundle();
	
	/* We could probably generate the bundle from the instruction's address. */
	assert( bundle != NULL );

	/* If there's not enough room, don't do anything. */
	if( ((unsigned)maxSize) < sizeof( ia64_bundle_t ) ) { return 0; }

	/* Copy things over. */
	* ( ia64_bundle_t *) insns = bundle->getMachineCode();

	return sizeof( ia64_bundle_t );
	} /* end getDisplacedInstructions() */
