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
 * $Id: inst-ia64.C,v 1.19 2002/09/27 19:38:12 tlmiller Exp $
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
			/* dest = [src1] */
			IA64_instruction loadInsn = generateRegisterLoad( dest, src1 );
			IA64_bundle loadBundle( MIIstop, loadInsn, NOP_I, NOP_I );
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
			Address & base, const vector<AstNode *> & operands,
			const string & callee, process * proc,
			bool noCost, const function_base * calleefunc,
			const vector<AstNode *> &ifForks,
			const instPoint *location) { 
	/* Consistency check. */
	assert( op == callOp );

	/* We'll get garbage (in debugging) if ibuf isn't aligned. */
	if( (Address)ibuf % 16 != 0 ) { fprintf( stderr, "Instruction buffer (%p) not aligned!\n", ibuf ); }
	
	/* Where are we calling? */
	Address funcEntryAddress;
	if( calleefunc ) {
		funcEntryAddress = calleefunc->getEffectiveAddress(proc);
		} else {
		/* We'll have to look for it. */
		bool err; funcEntryAddress = proc->findInternalAddress( callee, false, err );
		if( err ) { // Why do we do both?
			function_base * func = proc->findOneFunction(callee);
			if( ! func ) { // also stolen from other inst-*.C files.
				ostrstream os(errorLine, 1024, ios::out);
				os << "Internal error: unable to find addr of " << callee << endl;
				logLine(errorLine);
				showErrorCallback(80, (const char *) errorLine);
				P_abort();
				} /* end if not found at all. */
			funcEntryAddress = func->getEffectiveAddress(proc);
			} /* end if not an internal address */
		} /* end if calleefunc not given. */

	Address calleeGP = proc->getTOCoffsetInfo( funcEntryAddress );

	/* Generate the code for the arguments. */
	vector< Register > sourceRegisters;
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

	/* save the caller's GP: find a free register in the locals. */
	unsigned int savedGPRegister = findFreeLocal( rs, "Unable to find free register to save GP, aborting.\n" );
	// fprintf( stderr, "Saving GP in general register %d\n", savedGPRegister );
	emitRegisterToRegisterCopy( REGISTER_GP, savedGPRegister, ibuf, base, rs );
	rs->incRefCount( savedGPRegister );

	/* set the callee's GP */
	// fprintf( stderr, "Setting the callee's GP.\n" );
	emitVload( loadConstOp, calleeGP, 0, REGISTER_GP, ibuf, base, false /* ? */, 0 );

	/* save the caller's RP. */
	unsigned int savedRPRegister = findFreeLocal( rs, "Unable to find free register to save RP, aborting.\n" );
	rs->incRefCount( savedRPRegister );
	// fprintf( stderr, "Saving RP in general register %d\n", savedRPRegister );
	IA64_instruction saveRPInsn = generateBranchToRegisterMove( BRANCH_RETURN, savedRPRegister );

	/* save the caller's ar.pfs */
	unsigned int savedPFSRegister = findFreeLocal( rs, "Unable to find free register to save ar.pfs, aborting.\n" );
	rs->incRefCount( savedPFSRegister );
	// fprinf( stderr, "Saving ar.pfs in general register %d\n", savedPFSRegister );
	IA64_instruction savePFSInsn = generateApplicationToRegisterMove( AR_PFS, savedPFSRegister );

	/* Save the caller's BRANCH_SCRATCH register, since it may have something in it. */
	unsigned int savedBranchRegister = findFreeLocal( rs, "Unable to find free register to save BRANCH_SCRATCH, aborting.\n" );
	rs->incRefCount( savedBranchRegister );
	// fprintf( stderr, "Saving BRANCH_SCRATCH in general register %d\n", savedBranchRegister );
	IA64_instruction saveBranchInsn = generateBranchToRegisterMove( BRANCH_SCRATCH, savedBranchRegister );
	IA64_instruction memoryNOP( NOP_M );	

	IA64_bundle saveBranchRegisters( MIIstop, memoryNOP, saveRPInsn, saveBranchInsn );
	assert( (((Address)ibuf + base) % 16) == 0 );
	ia64_bundle_t * rawBundlePointer = (ia64_bundle_t *)((Address)ibuf + base);
	rawBundlePointer[0] = saveBranchRegisters.getMachineCode();
	base += 16;

	/* Call the callee.  Since br.call is IP-relative, and we don't know
	   where this function call will be installed until later, we would have
	   to calculate its immediate.  However, since we're doing calculations,
	   we have to use an indirect branch.  But if we're doing an indirect branch,
	   we use absolute addresses.  So just movl the funcEntryAddress to a gr,
	   and copy it into the scratch branch register. */

	fprintf( stderr, "*** Constructing call to function 0x%lx\n", funcEntryAddress );

	/* FIXME: could use output registers, if past sourceRegisters.size(). */
	Register tmp1 = findFreeLocal( rs, "Unable to find local register in which to store callee address, aborting.\n" );
	rs->incRefCount( tmp1 );

	// fprintf( stderr, "Loading function address into register %d\n", tmp1 );
	IA64_instruction_x loadCalleeInsn = generateLongConstantInRegister( tmp1, funcEntryAddress );
	IA64_bundle loadCalleeBundle( MLXstop, memoryNOP, loadCalleeInsn );

	// fprintf( stderr, "Copying computed branch in general register %d to branch register %d.\n", tmp1, BRANCH_SCRATCH );
	IA64_instruction setBranchInsn = generateRegisterToBranchMove( tmp1, BRANCH_SCRATCH );
	rs->freeRegister( tmp1 );
	IA64_bundle savePFSAndSetBranchBundle( MIIstop, memoryNOP, savePFSInsn, setBranchInsn );

	// fprintf( stderr, "Calling function with offset in branch register %d, return pointer in %d\n", BRANCH_SCRATCH, BRANCH_RETURN );
	IA64_instruction indirectBranchInsn = generateIndirectCallTo( BRANCH_SCRATCH, BRANCH_RETURN );
	IA64_bundle indirectBranchBundle( MMBstop, memoryNOP, memoryNOP, indirectBranchInsn );

	rawBundlePointer[1] = loadCalleeBundle.getMachineCode();
	rawBundlePointer[2] = savePFSAndSetBranchBundle.getMachineCode();
	rawBundlePointer[3] = indirectBranchBundle.getMachineCode();
	for( unsigned int i = 0; i < sourceRegisters.size(); i++ ) {
		/* Free all the output registers. */
		rs->freeRegister( outputZero + i );
		}
	base += 16 * 3;

	/* restore the caller's GP */
	// fprintf( stderr, "Restoring GP saved in general register %d\n", savedGPRegister );
	emitRegisterToRegisterCopy( savedGPRegister, REGISTER_GP, ibuf, base, rs );
	rs->freeRegister( savedGPRegister );

	/* restore the caller's RP, BRANCH_SCRATCH */
	// fprintf( stderr, "Restoring RP saved in general register %d\n", savedRPRegister );
	IA64_instruction restoreRPInsn = generateRegisterToBranchMove( savedRPRegister, BRANCH_RETURN );
	// fprintf( stderr, "Restoring BRANCH_SCRATCH saved in general register %d\n", savedBranchRegister );
	IA64_instruction restoreBranchInsn = generateRegisterToBranchMove( savedBranchRegister, BRANCH_SCRATCH );
	IA64_bundle restoreBranchRegisters( MIIstop, memoryNOP, restoreRPInsn, restoreBranchInsn );
	rawBundlePointer = (ia64_bundle_t *)((Address)ibuf + base); // reset because of the emitRTRC() call
	rawBundlePointer[0] = restoreBranchRegisters.getMachineCode();
	base += 16;
	rs->freeRegister( savedRPRegister );
	rs->freeRegister( savedBranchRegister );

	/* restore the caller's ar.pfs */
	IA64_instruction restorePFSInsn = generateRegisterToApplicationMove( savedPFSRegister, AR_PFS );
	IA64_bundle restorePFSBundle( MMIstop, memoryNOP, memoryNOP, restorePFSInsn );
	rawBundlePointer[1] = restorePFSBundle.getMachineCode();
	base += 16;
	rs->freeRegister( savedPFSRegister );

	/* copy the result (r8) to a registerSpace register */
	// fprintf( stderr, "Copying function call result to a known-dead register (%d).\n", savedGPRegister );
	emitRegisterToRegisterCopy( REGISTER_RP, savedGPRegister, ibuf, base, rs );
	rs->incRefCount( savedGPRegister );
	fprintf( stderr, "*** emitted function call in buffer at %p\n", ibuf );

	/* return that register */
	return savedGPRegister; 
	} /* end emitFuncCall() */

/* Required by symtab.C */
void pd_Function::checkCallPoints() {
	/* We associate pd_Function*'s with the callee's address.

	   On other architectures, those without explicit call instructions,
	   we winnow out potential call sites whose target resides in the same function. */

	instPoint * callSite;
	Address targetAddress;
	pd_Function * calledPdF;
	image * owner = file_->exec();
	vector< instPoint * > pdfCalls;

	for( unsigned int i = 0; i < calls.size(); i++ ) {
		callSite = calls[i]; assert( callSite );
		
		/* Extract the address that callSite calls. */
		targetAddress = callSite->getTargetAddress();

		calledPdF = owner->findFuncByAddr( targetAddress );
		if( calledPdF ) {
			callSite->set_callee( calledPdF );
			pdfCalls.push_back( callSite );
			} else {
			callSite->set_callee( NULL );
			pdfCalls.push_back( callSite );
			} /* end calledPdF conditional */
		} /* end callee loop */

	calls = pdfCalls;
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
			default:
				break;
			} /* end instruction type switch */
		} /* end instruction iteration */

	return true;
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
			const function_base * callee, process * proc ) { assert( 0 ); }

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
void emitV( opCode op, Register src1, Register src2, Register dest,
		char * ibuf, Address & base, bool noCost, int size ) {
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

		case divOp: {
			/* We would just call out to __udivsi3, but emitV() isn't given
			   enough information to call emitFuncCall().  So we generate
			   the equivalent in-line. */
			
			
			
			break; }

		default:
			fprintf( stderr, "emitV(): unrecognized op code %d\n", op );
			assert( 0 );
			break;
		} /* end op switch */
	} /* end emitV() */

/* Required by inst.C */
bool deleteBaseTramp( process *, instPoint *, trampTemplate *,
		      instInstance * lastMT ) { assert( 0 ); return false; }

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
bool process::emitInferiorRPCtrailer( void * void_insnPtr, Address & baseBytes,
			unsigned & breakOffset, bool shouldStopForResult,
			unsigned & stopForResultOffset,
			unsigned & justAfter_stopForResultOffset,
			bool isFunclet ) { assert( 0 ); return false; }

/* Required by process.C */
bool process::heapIsOk( const vector<sym_data> & find_us ) {
	/* Set .mainFunction; in the interest of civility, I will
	   not speculate why this is done here. */
	if( ! (mainFunction = findOneFunction( "main" )) ) {
		statusLine( "Failed to find main()." );
		showErrorCallback( 50, "Failed to find main()." );
		return false;
		}

	/* I will also not speculate why we ask after symbols that
	   don't need to be found if we don't do anything with them... */
	string str; Symbol sym; Address baseAddress;
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

/* Required by ast.C */
Register deadRegisterList[] = { 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF };
void initTramps() { 
	/* Initialize the registerSpace pointer regSpace to the state of the registers
	   that will exist at the end of the first part of the base tramp's code.  (That
	   is, for the minitramps.

	   The difficulty here is that the register numbers are expected to be physical
	   registers, but we can't determine those until we know which function we're
	   instrumenting...

	   For now, we'll just say 0x1 - 0xF. */

	static bool haveInitializedTramps = false;
	if( haveInitializedTramps ) { return; } else { haveInitializedTramps = true; }

	regSpace = new registerSpace( sizeof( deadRegisterList )/sizeof( Register ), deadRegisterList, 0, NULL );
	} /* end initTramps() */

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

/* private refactoring function */
#define MAX_BASE_TRAMP_SIZE (16 * 128)
#define NEAR_ADDRESS 0x2000000000000000		/* The lower end of the shared memory segment. */
trampTemplate * installBaseTramp( instPoint * & location, process * proc ) { // FIXME: updatecost
	fprintf( stderr, "*** Installing base tramp.\n" );
	/* Allocate memory and align as needed. */
	trampTemplate * baseTramp = new trampTemplate();

	bool allocErr; Address allocatedAddress = proc->inferiorMalloc( MAX_BASE_TRAMP_SIZE, anyHeap, NEAR_ADDRESS, & allocErr );
	if( allocErr ) { fprintf( stderr, "Unable to allocate base tramp, aborting.\n" ); abort(); }
	if( allocatedAddress % 16 != 0 ) { allocatedAddress = allocatedAddress - (allocatedAddress % 16); }

	/* Initialize the baseTramp. */
	baseTramp->baseAddr = allocatedAddress;
	baseTramp->size = 0;

	/* Acquire the base address of the object we're instrumenting. */
	Address baseAddress; assert( proc->getBaseAddress( location->iPgetOwner(), baseAddress ) );

	/* Generate the base tramp in insnPtr, and copy to allocatedAddress. */
	unsigned int bundleCount = 0;
	ia64_bundle_t insnPtr[ MAX_BASE_TRAMP_SIZE ];
	
	/* Initially, we decide if we'll be alloc()ing or saving and restoring
	   to obtain free (dead) general registers. */
	bool useSaveAndRestoreMethod = false;
	registerSpace baseRegisterSpace( 0, NULL, 0, NULL );
	defineBaseTrampRegisterSpaceFor( location, & baseRegisterSpace );
	if( baseRegisterSpace.getRegisterCount() == 0 ) {
		/* We either couldn't find an alloc(), or they didn't all have the same
		   inputs + locals size. */
		useSaveAndRestoreMethod = true;
		}

	IA64_instruction allocInsn;

	/* Insert the skipPreInsn nop bundle. */
	IA64_bundle nopBundle( MIIstop, NOP_M, NOP_I, NOP_I );
	baseTramp->skipPreInsOffset = baseTramp->size;
	insnPtr[bundleCount++] = nopBundle.getMachineCode(); baseTramp->size += 16;

	/* Generate the free registers.  Note that we always save the predicate
	   register to the memory stack, regardless of which method we use,
	   and that we do NOT automatically save and restore branch registers;
	   it's the responsibility of the function using them to do so. */
	baseTramp->savePreInsOffset = baseTramp->size;
	if( useSaveAndRestoreMethod ) {
		assert( 0 );
		} /* end if using save and restore method */
	else {
		// FIXME: see defineBTRF().
		Address fnEntryOffset = location->iPgetFunction()->addr();
		const unsigned char * fnEntryAddress = location->iPgetOwner()->getPtrToInstruction( fnEntryOffset );
		assert( (Address)fnEntryAddress % 16 == 0 );
		const ia64_bundle_t * rawBundles = (const ia64_bundle_t *)(Address)fnEntryAddress;
		IA64_bundle initialBundle( rawBundles[0] );
		allocInsn = * initialBundle.getInstruction( 0 );

		IA64_instruction alteredAlloc = generateAlteredAlloc( allocInsn.getMachineCode(), NUM_LOCALS, NUM_OUTPUT, 0 );
		IA64_bundle allocBundle( MIIstop, alteredAlloc, NOP_I, NOP_I );
		insnPtr[bundleCount++] = allocBundle.getMachineCode(); baseTramp->size += 16;
		} /* end if using alloc method. */

	/* Save the predicate registers to the memory stack. */
	Register localZero = baseRegisterSpace.getRegSlot( 0 )->number;
	IA64_instruction moveSPDown = generateShortImmediateAdd( REGISTER_SP, -8, REGISTER_SP );
	IA64_instruction movePRtoReg = generatePredicatesToRegisterMove( localZero );
	IA64_instruction storePRCopy = generateRegisterStore( REGISTER_SP, localZero );
	IA64_bundle savePredicateMoves( MIIstop, moveSPDown, NOP_M, movePRtoReg );
	IA64_bundle savePredicateStore( MIIstop, storePRCopy, NOP_I, NOP_I );	
	insnPtr[bundleCount++] = savePredicateMoves.getMachineCode(); baseTramp->size += 16;
	insnPtr[bundleCount++] = savePredicateStore.getMachineCode(); baseTramp->size += 16;

	/* FIXME: Insert the recursion check here, if desired.  If we _are_ recursing, predicate
	   a jump to a copy of the state-restoration code followed by an unconditional jump
	   back to the mutatee.  (If we restore the predicate registers, we lose _our_
	   predicate!)  The following instruction(s) (after the predicated jump) should be
	   reverse-predicated to set the gaurd bit. */

	/* Insert the localPre nop bundle. */
	baseTramp->localPreOffset = baseTramp->size;
	insnPtr[bundleCount++] = nopBundle.getMachineCode(); baseTramp->size += 16;

	/* Restore state for the relocated instructions.  This always includes the predicates. */
	baseTramp->localPreReturnOffset = baseTramp->size;
	IA64_instruction loadPRCopy = generateRegisterLoad( localZero, REGISTER_SP );
	IA64_instruction moveSPUp = generateShortImmediateAdd( REGISTER_SP, 8, REGISTER_SP );
	IA64_instruction moveRegToPr = generateRegisterToPredicatesMove( localZero, 0x1FFFF );
	IA64_bundle restorePRBundle( MstopMIstop, loadPRCopy, moveSPUp, moveRegToPr );
	baseTramp->restorePreInsOffset = baseTramp->size;
	insnPtr[bundleCount++] = restorePRBundle.getMachineCode(); baseTramp->size += 16;

	if( useSaveAndRestoreMethod ) {
		assert( 0 );
		} /* end if using save and restore method. */
	else {
		/* Restore the register frame:
		   rather than saving & restoring a register around the alloc
		   (since we have to stick its return value somewhere),
		   we'll just allocate an extra output register and store it there. */
		
		IA64_instruction alteredAlloc = generateRestoreAlloc( allocInsn.getMachineCode() );
		IA64_bundle allocBundle( MIIstop, alteredAlloc, NOP_I, NOP_I );
		insnPtr[bundleCount++] = allocBundle.getMachineCode(); baseTramp->size += 16;
		} /* end if using alloc method */

	/* Emulate the relocated instructions.  Since we're using the owner to read the
	   instructions, don't adjust by the base pointer. */
	Address installationPoint = location->iPgetAddress();
	installationPoint = installationPoint - (installationPoint % 16);
	Address installationPointAddress = (Address)location->iPgetOwner()->getPtrToInstruction( installationPoint );
	assert( installationPointAddress % 16 == 0 );
	IA64_bundle bundleToEmulate( * (const ia64_bundle_t *) installationPointAddress );
	baseTramp->emulateInsOffset = baseTramp->size;
	emulateBundle( bundleToEmulate, installationPoint, insnPtr, bundleCount, (unsigned int &)(baseTramp->size), allocatedAddress );
	
	/* Replace the skipPre nop bundle with a jump from it to baseTramp->emulateInsOffset. */
	unsigned int skipPreJumpBundleOffset = baseTramp->skipPreInsOffset / 16;
	IA64_instruction memoryNOP( NOP_M );
	IA64_instruction_x skipPreJump = generateLongBranchTo( baseTramp->emulateInsOffset - baseTramp->skipPreInsOffset ); 
	IA64_bundle skipPreJumpBundle( MLXstop, memoryNOP, skipPreJump );
	insnPtr[skipPreJumpBundleOffset] = skipPreJumpBundle.getMachineCode();

	/* Insert the skipPost nop bundle. */
	baseTramp->skipPostInsOffset = baseTramp->size;
	insnPtr[bundleCount++] = nopBundle.getMachineCode(); baseTramp->size += 16;

	/* Regenerate the free registers. */
	baseTramp->savePostInsOffset = baseTramp->size;
	if( useSaveAndRestoreMethod ) {
		assert( 0 );
		} /* end if using save and restore method */
	else {
		IA64_instruction alteredAlloc = generateAlteredAlloc( allocInsn.getMachineCode(), NUM_LOCALS, NUM_OUTPUT, 0 );
		IA64_bundle allocBundle( MIIstop, alteredAlloc, NOP_I, NOP_I );
		insnPtr[bundleCount++] = allocBundle.getMachineCode(); baseTramp->size += 16;
		} /* end if using alloc method */

	/* Save the predicate registers to the memory stack again. */
	insnPtr[bundleCount++] = savePredicateMoves.getMachineCode(); baseTramp->size += 16;
	insnPtr[bundleCount++] = savePredicateStore.getMachineCode(); baseTramp->size += 16;

	/* Insert the localPost nop bundle. */
	baseTramp->localPostOffset = baseTramp->size;
	insnPtr[bundleCount++] = nopBundle.getMachineCode(); baseTramp->size += 16;

	/* FIXME: if we're using the recursion gaurd, this is the place to unset it. */

	/* Restore state for the jump back to normal execution.  Always includes the predicates. */
	baseTramp->localPostReturnOffset = baseTramp->size;
	baseTramp->restorePostInsOffset = baseTramp->size;
	insnPtr[bundleCount++] = restorePRBundle.getMachineCode(); baseTramp->size += 16;

	if( useSaveAndRestoreMethod ) {
		assert( 0 );
		} /* end if using save and restore method */
	else {
		/* Restore the register frame:
		   rather than saving & restoring a register around the alloc
		   (since we have to stick its return value somewhere),
		   we'll just allocate an extra output register and store it there. */
		IA64_instruction alteredAlloc = generateRestoreAlloc( allocInsn.getMachineCode() );
		IA64_bundle allocBundle( MIIstop, alteredAlloc, NOP_I, NOP_I );
		insnPtr[bundleCount++] = allocBundle.getMachineCode(); baseTramp->size += 16;
		} /* end if using alloc method */

	/* Insert the jump back to normal execution.
	   Question: why do we do this if we generate a returnInstance? */
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
	insnPtr[skipPostJumpBundleOffset] = skipPostJumpBundle.getMachineCode();

	/* Copy the instructions from hither to thither. */
	InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( allocatedAddress, proc );
	iAddr.writeBundlesFrom( (unsigned char *)insnPtr, bundleCount );

	/* Install the jump to the base tramp. */
	Address originatingAddress = returnAddress - 16;
	IA64_instruction_x jumpToBaseInstruction = generateLongBranchTo( allocatedAddress - originatingAddress );
	IA64_bundle jumpToBaseBundle( MLXstop, memoryNOP, jumpToBaseInstruction );
	InsnAddr jAddr = InsnAddr::generateFromAlignedDataAddress( originatingAddress, proc );
	jAddr.replaceBundleWith( jumpToBaseBundle );

	fprintf( stderr, "*** Installed base tramp at 0x%lx, from 0x%lx\n", baseTramp->baseAddr, originatingAddress );
	return baseTramp;
	} /* end installBaseTramp() */

/* Required by inst.C */
trampTemplate * findAndInstallBaseTramp( process * proc, instPoint * & location,
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
fprintf( stderr, "Generating return instance from 0x%lx to 0x%lx\n", returnFrom, returnTo );
	* longBranchInstruction = generateLongBranchTo( returnTo - returnFrom );
	retInstance = new returnInstance( 3, longBranchInstruction, 16, returnFrom, 16 );

	return installedBaseTramp;
	} /* end findAndInstallBaseTramp() */

/* Required by func-reloc.C */
bool pd_Function::isNearBranchInsn( const instruction insn ) { assert( 0 ); return false; }

/* Required by process.C */
bool process::emitInferiorRPCheader( void * void_insnPtr, Address & baseBytes, bool isFunclet ) { assert( 0 ); return false; }

void generateMTpreamble(char *, Address &, process *) {
	assert( 0 );	// We don't yet handle multiple threads.
	} /* end generateMTpreamble() */

/* Required by ast.C */
Register emitR( opCode op, Register src1, Register src2, Register dest,
			char * ibuf, Address & base, bool noCost ) {
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
void installTramp( instInstance * inst, process * proc, char * code, int codeSize,
		  instPoint * /* location */, callWhen when ) {
	/* Book-keeping. */
	totalMiniTramps++; insnGenerated += codeSize / 16;

	/* Write the minitramp. */
	fprintf( stderr, "*** Installing minitramp at 0x%lx\n", inst->trampBase );
	proc->writeDataSpace( (caddr_t)inst->trampBase, codeSize, code );

	/* Make sure that it's not skipped. */
	Address insertNOPAt = inst->baseInstance->baseAddr;
	if( when == callPreInsn && inst->baseInstance->prevInstru == false ) {
		inst->baseInstance->cost += inst->baseInstance->prevBaseCost;
		insertNOPAt += inst->baseInstance->skipPreInsOffset;
		inst->baseInstance->prevInstru = true;
		} 
	else if( when == callPostInsn && inst->baseInstance->postInstru == false ) {
		inst->baseInstance->cost += inst->baseInstance->postBaseCost;
		insertNOPAt += inst->baseInstance->skipPostInsOffset;
		inst->baseInstance->postInstru = true;
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
			IA64_instruction storeInsn = generateRegisterStore( dest, src1 );
			IA64_bundle storeBundle( MIIstop, storeInsn, NOP_I, NOP_I );
			* rawBundlePointer = storeBundle.getMachineCode();
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

bool InsnAddr::writeStringAtOffset( unsigned int offsetInBundles, const char * string, unsigned int length ) {
	/* Align the instruction address and compute in the offset. */
	unsigned short offset = encodedAddress % 16;
	Address alignedOffset = encodedAddress - offset + (offsetInBundles * 16);

	return myProc->writeTextSpace( (void *)alignedOffset, length + 1, string );
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

/* I haven't the slightest idea what needs this. */
int BPatch_point::getDisplacedInstructions( int maxSize, void * insns ) {
	assert( 0 );
	} /* end getDisplacedInstructions() */
