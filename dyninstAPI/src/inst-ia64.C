/* -*- Mode: C; indent-tabs-mode: true; tab-width: 4 -*- */

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
 * inst-ia64.C - ia64 dependent functions and code generator
 * $Id: inst-ia64.C,v 1.70 2005/02/09 03:27:44 jaw Exp $
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
#include "dyninstAPI/src/instP.h"		// class returnInstance

#include "dyninstAPI/src/rpcMgr.h"

/* For unwinding through instrumentation. */
#include <libunwind.h>
#define PRESERVATION_UNWIND_OPERATION_COUNT	32

/* Assembler-level labels for the dynamic basetramp. */
extern "C" {
	void fetch_ar_pfs();
	void address_of_instrumentation();
	void address_of_call_alloc();
	void address_of_return_alloc();
	void address_of_address_of_call_alloc();
	void address_of_address_of_return_alloc();	
	void address_of_jump_to_emulation();
	
	/* For FP work. */
	void address_of_jump_to_spills();
	void address_of_jump_to_fills();
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

	static Address offsetOfJumpToSpills = 0;
	static Address offsetOfJumpToFills = 0;

	if( offsetOfJumpToInstrumentation == 0 ) {
		offsetOfJumpToInstrumentation = (Address)&address_of_instrumentation - addressOfTemplate;
		offsetOfJumpToEmulation = (Address)&address_of_jump_to_emulation - addressOfTemplate;
		offsetOfCallAlloc = (Address)&address_of_call_alloc - addressOfTemplate;
		offsetOfCallAllocMove = (Address)&address_of_address_of_call_alloc - addressOfTemplate;
		offsetOfReturnAlloc = (Address)&address_of_return_alloc - addressOfTemplate;
		offsetOfReturnAllocMove = (Address)&address_of_address_of_return_alloc - addressOfTemplate;

		offsetOfJumpToSpills = (Address)&address_of_jump_to_spills - addressOfTemplate;
		offsetOfJumpToFills = (Address)&address_of_jump_to_fills - addressOfTemplate;
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

	/* FIXME: jumpToSpills and jumpToFills need to be over-written to actually jump to the
	   appropriate code blocks, whose mutatee addresses the caller will supply (and populate).
	   Also take the addresses to the jumps back as parameters and rewrite those as well. */

	/* Where do we stick the instrumentation code if we don't rewrite the jump again? */
	return offsetOfJumpToEmulation + 16;
	} /* end relocateTrampTemplate() */

/* Required by process, ast .C, et al */
registerSpace * regSpace;

/* Required by ast.C; used for memory instrumentation. */
void emitCSload( BPatch_addrSpec_NP as, Register dest, char * baseInsn, Address & base, bool noCost ) {
	// /* DEBUG */ fprintf( stderr, "emitCSload: as.imm = %d, as.r1 = %d, as.r2 = %d\n", as.getImm(), as.getReg( 0 ), as.getReg( 1 ) );
	
	/* emitCSload()'s first argument should be a BPatch_countSpec_NP.  It's supposed
	   to set dest to the number calculated by the countSpec, which means that it'll
	   be filled with bogus numnbers.  Don't complicate emitASload() with the bogosity. */
	assert( as.getReg( 0 ) == -1 && as.getReg( 1 ) == -1 );
	emitVload( loadConstOp, as.getImm(), dest, dest, baseInsn, base, noCost );	
	} /* end emitCSload() */

#define SHORT_IMMEDIATE_MIN 	-8191
#define SHORT_IMMEDIATE_MAX		8191

/* Required by ast.C */
void emitVload( opCode op, Address src1, Register src2, Register dest,
		char * ibuf, Address & base, bool noCost, int size, const instPoint * location, process * proc, registerSpace * rs ) {
	assert( (((Address)ibuf + base) % 16) == 0 );
	ia64_bundle_t * rawBundlePointer = (ia64_bundle_t *)((Address)ibuf + base);

	switch( op ) {
		case loadOp: {
			/* Stick the address src1 in the temporary register src2. */
			emitVload( loadConstOp, src1, 0, src2, ibuf, base, noCost, size );

			/* Load into destination register dest from [src2]. */
			IA64_instruction loadInsn = generateRegisterLoad( dest, src2, size );
			IA64_bundle loadBundle( MIIstop, loadInsn, NOP_I, NOP_I );
			
			/* Insert bundle. */
			rawBundlePointer = (ia64_bundle_t *)((Address)ibuf + base);
			* rawBundlePointer = loadBundle.getMachineCode();
			base += 16;
			break; }

		case loadConstOp: {
			// Optimization: if |immediate| <= 22 bits, use generateShortImmediateAdd(), instead.
			IA64_instruction_x lCO = generateLongConstantInRegister( dest, src1 );
			* rawBundlePointer = generateBundleFromLongInstruction( lCO ).getMachineCode();
			base += 16;
			break; }

		case loadFrameRelativeOp: {
			/* Load size bytes from the address fp + src1 into the register dest. */
			assert( location->pointFunc()->framePointerCalculator != NULL );			

			/* framePointerCalculator will leave the frame pointer in framePointer. */
			pdvector< AstNode * > ifForks;
			Register framePointer = (Register) location->pointFunc()->framePointerCalculator->generateCode_phase2( proc, rs, ibuf, base, false, ifForks, location );
			/* For now, make sure what we don't know can't hurt us. */
			assert( ifForks.size() == 0 );
			
			/* Recalculate this because of the generateCode_phase2() call. */
			rawBundlePointer = (ia64_bundle_t *)((Address)ibuf + base);
			
			IA64_instruction_x longConstant = generateLongConstantInRegister( src2, src1 );
			IA64_instruction calculateAddress = generateArithmetic( plusOp, framePointer, src2, framePointer ); 
			IA64_instruction loadFromAddress = generateRegisterLoad( dest, framePointer, size );
			IA64_bundle prepareOffsetBundle( MLXstop, NOP_M, longConstant );
			IA64_bundle calculateAndLoadBundle( MstopMIstop, calculateAddress, loadFromAddress, NOP_I );

			rawBundlePointer[0] = prepareOffsetBundle.getMachineCode();
			rawBundlePointer[1] = calculateAndLoadBundle.getMachineCode();
			base += 32;

			rs->freeRegister( framePointer );
			} break;

		case loadRegRelativeOp: {
			/* Similar to loadFrameRelativeOp, except any general register may be used.
			 *
			 * src1 = offset
			 * src2 = base register
			 */
			Register trueBase = rs->allocateRegister(ibuf, base, noCost);

			emitLoadPreviousStackFrameRegister( BP_GR0 + src2, trueBase, ibuf, base, size, noCost );
			rawBundlePointer = (ia64_bundle_t *)((Address)ibuf + base);

			IA64_instruction_x longConstant = generateLongConstantInRegister( dest, src1 );
			IA64_instruction calculateAddress = generateArithmetic( plusOp, dest, dest, trueBase );
			IA64_instruction loadFromAddress = generateRegisterLoad( dest, dest, size );
			IA64_bundle prepareOffset( MLXstop, NOP_M, longConstant );
			IA64_bundle calculateAndLoadBundle( MstopMIstop, calculateAddress, loadFromAddress, NOP_I );

			rawBundlePointer[0] = prepareOffset.getMachineCode();
			rawBundlePointer[1] = calculateAndLoadBundle.getMachineCode();
			base += 32;

			rs->freeRegister( trueBase );
			} break;

		case loadRegRelativeAddr: {
			/* Similar to loadFrameAddr, except any general register may be used.
			 *
			 * src1 = offset
			 * src2 = base register
			 */
			Register trueBase = rs->allocateRegister(ibuf, base, noCost);

			emitLoadPreviousStackFrameRegister( BP_GR0 + src2, trueBase, ibuf, base, size, noCost );
			rawBundlePointer = (ia64_bundle_t *)((Address)ibuf + base);

			IA64_instruction_x longConstant = generateLongConstantInRegister( dest, src1 );
			IA64_instruction calculateAddress = generateArithmetic( plusOp, dest, dest, trueBase );
			IA64_bundle prepareOffset( MLXstop, NOP_M, longConstant );
			IA64_bundle calculateBundle( MIIstop, calculateAddress, NOP_I, NOP_I );

			rawBundlePointer[0] = prepareOffset.getMachineCode();
			rawBundlePointer[1] = calculateBundle.getMachineCode();
			base += 32;

			rs->freeRegister( trueBase );
			} break;

		case loadFrameAddr: {
			/* Write the value of the fp + the immediate src1 into the register dest. */
			assert( location->pointFunc()->framePointerCalculator != NULL );

			/* framePointerCalculator will leave the frame pointer in framePointer. */
			pdvector< AstNode * > ifForks;
			Register framePointer = (Register) location->pointFunc()->framePointerCalculator->generateCode_phase2( proc, rs, ibuf, base, false, ifForks, location );
			/* For now, make sure what we don't know can't hurt us. */
			assert( ifForks.size() == 0 );
			
			/* Recalculate this because of the generateCode_phase2() call. */
			rawBundlePointer = (ia64_bundle_t *)((Address)ibuf + base);
			
			IA64_instruction memoryNop( NOP_M );
			IA64_instruction integerNop( NOP_I );
			if( SHORT_IMMEDIATE_MIN < ((int)src1) && ((int)src1) < SHORT_IMMEDIATE_MAX ) {
				IA64_instruction sumFrameAndOffset = generateShortImmediateAdd( dest, src1, framePointer );
				IA64_bundle resultBundle( MIIstop, memoryNop, integerNop, sumFrameAndOffset );
				* rawBundlePointer = resultBundle.getMachineCode();
				base += 16;
				}
			else {
				IA64_instruction_x longConstant = generateLongConstantInRegister( dest, src1 );
				IA64_bundle setLCBundle( MLXstop, memoryNop, longConstant );
				IA64_instruction addLongConstantToFP = generateArithmetic( plusOp, dest, dest, framePointer );
				IA64_bundle resultBundle( MIIstop, memoryNop, integerNop, addLongConstantToFP );

				rawBundlePointer[0] = setLCBundle.getMachineCode();
				rawBundlePointer[1] = resultBundle.getMachineCode();
				base += 32;
				}

			rs->freeRegister( framePointer );						
			} break;

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

	IA64_instruction moveInsn( generateRegisterToRegisterMove( source, destination ) );
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
					   process * proc, bool /* noCost */,
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
	   target into a scratch branch register, and indirect to it. */
	emitVload( loadConstOp, calleeGP, 0, REGISTER_GP, ibuf, base, false /* ? */, 0 );

	// /* DEBUG */ fprintf( stderr, "* Constructing call to function 0x%lx\n", callee_addr );

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
	   to make sure we don't smash the stack.  This, however, is done in the preservation header. */
  
	rawBundlePointer[bundleCount++] = indirectBranchBundle.getMachineCode();
	for( unsigned int i = 0; i < sourceRegisters.size(); i++ ) {
		/* Free all the output registers. */
		rs->freeRegister( outputZero + i );
		}
	base += bundleCount * 16;

	/* copy the result (r8) to a registerSpace register */
	rs->freeRegister( rsRegister );
	emitRegisterToRegisterCopy( REGISTER_RV, rsRegister, ibuf, base, rs );
	// /* DEBUG */ fprintf( stderr, "* emitted function call in buffer at %p\n", ibuf );

	/* return that register */
	return rsRegister;
	} /* end emitFuncCall() */

/* Required by symtab.C */
void int_function::checkCallPoints() {
	/* We associate int_function*'s with the callee's address.

	   On other architectures, those without explicit call instructions,
	   we winnow out potential call sites whose target resides in the same function. */
  if (call_points_have_been_checked) return;

	instPoint * callSite;
	Address targetAddress;
	int_function * calledPdF;
	image * owner = pdmod()->exec();
	pdvector< instPoint * > pdfCalls;

	for( unsigned int i = 0; i < calls.size(); i++ ) {
		callSite = calls[i]; assert( callSite );
		
		/* Extract the address that callSite calls. */
		targetAddress = callSite->getTargetAddress();

		calledPdF = owner->findFuncByOffset( targetAddress );
		if( calledPdF ) {
			callSite->setCallee( calledPdF );
			} else {
			callSite->setCallee( NULL );
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
bool int_function::findInstPoints( const image * i_owner ) {
  parsed_ = true;

	/* We assume the function boundaries are correct.  [Check jump tables, etc.] */
	Address addr = (Address)i_owner->getPtrToInstruction( getAddress( 0 ) );

	IA64_iterator iAddr( addr );
	Address lastI = addr + get_size();
	Address addressDelta = getAddress( 0 ) - addr;

	/* Generate an instPoint for the function entry. */
	funcEntry_ = new instPoint( getAddress( 0 ), this, * iAddr, functionEntry );

	IA64_instruction * currInsn;
	for( ; iAddr < lastI; iAddr++ ) {	
		currInsn = * iAddr;
		switch( currInsn->getType() ) {
			case IA64_instruction::DIRECT_BRANCH: {
				/* Some system call -wrapping functions (write(), read()) jump to common code
				   (at __libc_start_main+560) to handle system calls which return errors,
				   but this common code returns directly, instead of jumping back to its
				   callee, which means our exit instrumentation may not be run.
				   
				   Laune's new parsing code will handle cases like this, so for now,
				   use the same wrong assumption as the rest of the code (that symbol
				   table sizes are correct) to determine the boundaries of the function,
				   and claim that jumps outside these boundaries are exits. */
				Address targetAddress = currInsn->getTargetAddress() + iAddr.getEncodedAddress() + addressDelta;
				targetAddress =  targetAddress - (targetAddress % 16);
				Address lowAddress = addr + addressDelta;
				Address highAddress = addr + get_size() + addressDelta;
				
				if( ! ( lowAddress <= targetAddress && targetAddress < highAddress ) ) { 
					// /* DEBUG */ fprintf( stderr, "%s[%d]: 0x%lx targets 0x%lx, outside of 0x%lx to 0x%lx\n", __FILE__, __LINE__, iAddr.getEncodedAddress() + addressDelta, targetAddress, lowAddress, highAddress );
					funcReturns.push_back( new instPoint( iAddr.getEncodedAddress() + addressDelta, this, currInsn, functionExit ) );
					}			
				} break;				
				
			case IA64_instruction::RETURN:
				funcReturns.push_back( new instPoint( iAddr.getEncodedAddress() + addressDelta, this, currInsn, functionExit ) );
				break;
				
			case IA64_instruction::DIRECT_CALL:
			case IA64_instruction::INDIRECT_CALL:
				calls.push_back( new instPoint( iAddr.getEncodedAddress() + addressDelta, this, currInsn, callSite) );
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

/* Required by BPatch_function, image.C */
BPatch_point *createInstructionInstPoint( process * proc, void * address,
										  BPatch_point ** /*alternative*/, BPatch_function * bpf ) { 
	/* Since IA-64 instpoints never conflict, we can safely ignore 'alternative'.
	   Simply verify that the given address is valid and do the construction routines. */
	uint64_t slotNo = ((Address)address) % 0x10;
	if( slotNo >= 3 ) { return NULL; }	

	Address alignedAddress = ((Address)address) - slotNo;
	
	BPatch_function * bpFunction = bpf;

	int_function * containingFunction = NULL;
	if( bpf != NULL ) {
		containingFunction = (int_function *)(bpf->func);
		}
	else {
		containingFunction = proc->findFuncByAddr( alignedAddress );
		if( containingFunction == NULL ) { return NULL; }
		bpFunction = proc->findOrCreateBPFunc( containingFunction );
		}

	/* Acquire the owner. */
	const image * owner = containingFunction->pdmod()->exec();

	/* Acquire the instruction. */
	Address instructionAddress = (Address)owner->getPtrToInstruction( containingFunction->getAddress( NULL ) );
	instructionAddress += alignedAddress - containingFunction->getAddress( NULL );
	ia64_bundle_t * bundlePtr = (ia64_bundle_t *) instructionAddress;
	IA64_bundle theBundle( * bundlePtr );
	
	/* Construct the instPoint. */
	Address instPointAddr = (Address)address;
	if( theBundle.hasLongInstruction() && slotNo == 2 ) { instPointAddr--; }
	
	IA64_instruction * theInstruction = theBundle.getInstruction( slotNo );
	instPoint * arbitraryInstPoint = new instPoint( instPointAddr, containingFunction, theInstruction, otherPoint );
	containingFunction->addArbitraryPoint( arbitraryInstPoint, proc );
	return proc->findOrCreateBPPoint( bpFunction, arbitraryInstPoint, BPatch_arbitrary );
	} /* end createInstructionInstPoint() */
	
/* Required by ast.C; used for memory instrumentation. */
void emitASload( BPatch_addrSpec_NP as, Register dest, char * ibuf, Address & base, bool /*noCost*/ ) {
	/* Convert an addrSpec into a value in the destination register. */
	// /* DEBUG */ fprintf( stderr, "emitASload: as.imm = %d, as.r1 = %d, as.r2 = %d\n", as.getImm(), as.getReg( 0 ), as.getReg( 1 ) );
	emitRegisterToRegisterCopy( as.getReg( 0 ), dest, ibuf, base, NULL );
	} /* end emitASload() */

/* Required by process.C */
void instWaitingList::cleanUp( process *, Address /*addr*/ ) { assert( 0 ); }

/* Required by ast.C */
Register deadRegisterList[] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF };
void initTramps(bool /*is_multithreaded*/) { 
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

			/* What's worse, since there is no way to allocate predicate
			   registers, the variable dest must represent a general register.
			   To deal with a the range difference between to two types of
			   registers (General: 0-127, Predicate: 0-63), we simply use
			   dest % 64.

			   That method is a hack at best, and creates a number of potential
			   bugs.  But, a correct solution would involve a complete overhaul
			   of our regSpace class, moving it into architecture specific files.
			   Perhaps for the next release.
			*/
			IA64_instruction comparisonInsn = generateComparison( op, dest % 64, src1, src2 );
			IA64_instruction setTrue = predicateInstruction( dest % 64, generateShortImmediateAdd( dest, 1, REGISTER_ZERO ) );
			IA64_instruction setFalse = predicateInstruction( (dest + 1) % 64, generateShortImmediateAdd( dest, 0, REGISTER_ZERO ));
			IA64_bundle comparisonBundle( MstopMIstop, comparisonInsn, setTrue, setFalse );
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
			/* Only the floating-point unit has a fixed-point multiplier. */
			unsigned bundleCount = 0;

			/* The stack already has the extra space that we need, thanks to the preservation
			   header, so use (arbitrarily) r3 to generate the pointers. */
			IA64_instruction spillFP2 = generateFPSpillTo( REGISTER_SP, 2 );
			IA64_instruction secondSpillAddr = generateShortImmediateAdd( 3, +16, REGISTER_SP );
			IA64_instruction integerNOP( NOP_I );
			IA64_instruction spillFP3 = generateFPSpillTo( 3, 3 );
			IA64_bundle spillFP2Bundle( MMIstop, spillFP2, secondSpillAddr, integerNOP );
			IA64_bundle spillFP3Bundle( MIIstop, spillFP3, integerNOP, integerNOP );
			rawBundlePointer[ bundleCount++ ] = spillFP2Bundle.getMachineCode();
			rawBundlePointer[ bundleCount++ ] = spillFP3Bundle.getMachineCode();
			
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
			IA64_instruction fillFP2 = generateFPFillFrom( REGISTER_SP, 2 );
			IA64_instruction fillFP3 = generateFPFillFrom( 3, 3 );
			IA64_bundle fillF2Bundle( MIIstop, fillFP2, integerNOP, integerNOP );
			IA64_bundle fillF3Bundle( MIIstop, fillFP3, integerNOP, integerNOP );
			rawBundlePointer[bundleCount++] = fillF2Bundle.getMachineCode();
			rawBundlePointer[bundleCount++] = fillF3Bundle.getMachineCode();
			
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

			/* Generate the empty ifForks AST. */
			pdvector< AstNode * > ifForks;

			/* Callers of emitV() expect the register space to be unchanged
			   afterward, so make sure eFC() doesn't use the source registers
			   after copying them to the output. */

			/* Find address for functions __divsi3 */
            pdstring callee = pdstring("__divsi3");
			bool err = false;
			Address callee_addr = proc->findInternalAddress(callee, false, err);
			if (err) {
			  int_function *calleefunc = proc->findOnlyOneFunction(callee);
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
			/* A function call does not require any conditional branches, so ensure
			   that we're not screwing up the current AST by not telling it about stuff. */
			assert( ifForks.size() == 0 );
			
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

		case storeIndirOp: {
			/* Store 'size' bytes out of register src1 into the address in dest. */
			IA64_instruction storeInstruction = generateRegisterStore( dest, src1, size );
			IA64_bundle storeBundle( MMIstop, storeInstruction, NOP_M, NOP_I );
			* rawBundlePointer = storeBundle.getMachineCode();
			base += 16;		
			break; }
			
		case loadIndirOp: {
			/* Load 'size' bytes from the address in register src1 to the register dest. */
			IA64_instruction loadInstruction = generateRegisterLoad( dest, src1, size );
			IA64_bundle loadBundle( MMIstop, loadInstruction, NOP_M, NOP_I );
			* rawBundlePointer = loadBundle.getMachineCode();
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
			  char *ibuf, Address & base, bool /*noCost*/ ) {
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
void emitVupdate( opCode op, RegValue /*src1*/, Register /*src2*/,
				  Address /*dest*/, char * /*ibuf*/, Address & /*base*/, bool /*noCost*/ ) {
	switch( op ) {
		case updateCostOp:
			// fprintf( stderr, "FIXME: no-op'ing emitVupdate(updateCostOp)\n" );
			break;

		default:
			assert( 0 );
			break;
		} /* end op switch */
	} /* end emitVupdate() */

/* private refactoring function */
IA64_bundle generateUnknownRegisterTypeMoveBundle( Address src, Register dest ) {
		if( BP_GR0 <= src <= BP_GR127 ) {
			IA64_instruction moveInsn = generateRegisterToRegisterMove( src, dest );
			return IA64_bundle( MIIstop, NOP_M, NOP_I, moveInsn );
			}
		else if( BP_BR0 <= src <= BP_BR7 ) {
			IA64_instruction moveInsn = generateBranchToRegisterMove( src, dest );
			return IA64_bundle( MIIstop, NOP_M, NOP_I, moveInsn );
			}
		else if( BP_AR0 <= src <= BP_AR63 ) {
			/* M-unit */
			IA64_instruction moveInsn = generateApplicationToRegisterMove( src, dest );
			return IA64_bundle( MIIstop, moveInsn, NOP_I, NOP_I );
			}
		else if( BP_AR64 <= src <= BP_AR127 ) {
			/* I-unit */
			IA64_instruction moveInsn = generateApplicationToRegisterMove( src, dest );
			return IA64_bundle( MIIstop, NOP_M, NOP_I, moveInsn );
			}		
		else if( BP_PR == src ) {
			IA64_instruction moveInsn = generatePredicatesToRegisterMove( dest );
			return IA64_bundle( MIIstop, NOP_M, NOP_I, moveInsn );
			}
		else {
			fprintf( stderr, "[%s][%d]: illegal register number.\n", __FILE__, __LINE__ );
			assert( 0 );
			}
	} /* end generateUnknownRegisterTypeMoveBundle() */

/* Required by ast.C */
void emitLoadPreviousStackFrameRegister( Address register_num, Register dest, char * insn,
										 Address & base, int /*size*/, bool /*noCost*/ )
{
    /* Ray: why adjust 'base' instead of 'size' (as before)? */
	IA64_bundle bundle;

	if( regSpace->storageMap[ register_num ] == 0 ) {
		if( register_num == dest ) return;
		bundle = generateUnknownRegisterTypeMoveBundle( register_num, dest );
	} else if( regSpace->storageMap[ register_num ] > 0 ) {
		bundle = generateUnknownRegisterTypeMoveBundle( regSpace->storageMap[ register_num ], dest );
	} else {
		// Register was stored on the memory stack.
	
		int stackOffset = 32 + ( (-regSpace->storageMap[ register_num ] - 1) * 8 );
		
		bundle = IA64_bundle( MstopMIstop,
							  generateShortImmediateAdd( dest, stackOffset, REGISTER_SP ),
							  generateFillFrom( dest, dest, 0 ),
							  NOP_I );
	}

	/* Insert bundle. */
	ia64_bundle_t *rawBundlePointer = (ia64_bundle_t *)((Address)insn + base);
	*rawBundlePointer = bundle.getMachineCode();
	base += 16;
}

/* Required by BPatch_thread.C */
void returnInstance::installReturnInstance( process * proc ) { 
	IA64_instruction_x ** ia64Sequence = (IA64_instruction_x **) instructionSeq;
	IA64_instruction memoryNOP( NOP_M );
	IA64_bundle installationBundle( MLXstop, memoryNOP, ** ia64Sequence );	
	// /* DEBUG */ fprintf( stderr, "Installing return instance at 0x%lx\n", addr_ );
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

/* in arch-ia64.C */
extern IA64_instruction::unitType INSTRUCTION_TYPE_ARRAY[(0x20 + 1) * 3];

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
	IA64_instruction::insnType instructionType = insnToEmulate.getType();
	insn_tmpl tmpl = { insnToEmulate.getMachineCode().high };
	insn_tmpl imm  = { insnToEmulate.getMachineCode().low };
	int64_t immediate;

	/* originalLocation is NOT an encoded address */
	switch( instructionType ) {
		case IA64_instruction::DIRECT_BRANCH: immediate = GET_X3_TARGET( &tmpl, &imm ); break;
		case IA64_instruction::DIRECT_CALL:   immediate = GET_X4_TARGET( &tmpl, &imm ); break;
		default: {
            IA64_instruction memoryNOP( NOP_M );
            insnPtr[offset++] = IA64_bundle( MLXstop,
											 memoryNOP,
											 insnToEmulate ).getMachineCode();
			size += 16;
            return;
		}
	}

	/* Correct the immediate by the difference between originalLocation and (insnPtr + size). */
	/* originalLocation is NOT an encoded address */
	Address target = immediate + originalLocation;
	Address source = allocatedAddress + size;
	int64_t displacement = target - source;

    switch( instructionType ) {
		case IA64_instruction::DIRECT_BRANCH: SET_X3_TARGET( &tmpl, &imm, displacement); break;
		case IA64_instruction::DIRECT_CALL:   SET_X4_TARGET( &tmpl, &imm, displacement); break;
		default: break;
    }

    IA64_instruction_x alteredLong( imm.raw, tmpl.raw, insnToEmulate.getTemplateID() );
    insnPtr[offset++] = IA64_bundle( MLXstop,
									 IA64_instruction( NOP_M ),
									 alteredLong ).getMachineCode();
    size += 16;
	} /* end emulateLongInstruction */

/* private refactoring function */
void rewriteShortOffset( IA64_instruction insnToRewrite, Address originalLocation, ia64_bundle_t * insnPtr, unsigned int & offset, unsigned int & size, Address allocatedAddress ) {
	/* We insert a short jump past a long jump to the original target, followed
	   by the instruction rewritten to branch one bundle backwards.
	   It's not very elegant, but it's very straightfoward to implement. :) */
	   
	/* CHECKME: do we need to worry about (dynamic) unwind information for any
	   of the instruction we'll rewrite here?  We special-cased direct calls below. */

	/* Skip the long branch. */
	IA64_instruction memoryNOP( NOP_M );
	IA64_instruction_x skipInsn = generateLongBranchTo( 32 ); // could be short
	IA64_bundle skipInsnBundle( MLXstop, memoryNOP, skipInsn );
	insnPtr[offset++] = skipInsnBundle.getMachineCode(); size += 16;

	/* Extract the original target. */
	insn_tmpl tmpl = { insnToRewrite.getMachineCode() };
	bool isSpecCheck = ( insnToRewrite.getType() == IA64_instruction::SPEC_CHECK );
	Address originalTarget;

	if( isSpecCheck )
		originalTarget = GET_M20_TARGET( &tmpl ) + originalLocation;
	else {
		// This is cheating a bit, but all non-SPEC_CHECK instructions
		// that flow through this function share the same immediate
		// encoding, so using the M22 template should work.
		originalTarget = GET_M22_TARGET( &tmpl ) + originalLocation;
	}

	/* The long branch. */
	// /* DEBUG */ fprintf( stderr, "originalTarget 0x%lx = 0x%lx + 0x%lx\n", originalTarget, ( signExtend( signBit, immediate ) << 4 ), originalLocation );
	IA64_instruction_x longBranch = generateLongBranchTo( originalTarget - (allocatedAddress + size) );
	IA64_bundle longBranchBundle( MLXstop, memoryNOP, longBranch );
	insnPtr[offset++] = longBranchBundle.getMachineCode(); size += 16;

	/* Rewrite the short immediate. */
	if( isSpecCheck )
		SET_M20_TARGET( &tmpl, -16 );
	else
		SET_M22_TARGET( &tmpl, -16 );

	/* Emit the rewritten immediate. */
	IA64_instruction rewrittenInsn( tmpl.raw, insnToRewrite.getTemplateID(), insnToRewrite.getSlotNumber() );
	insnPtr[offset++] = generateBundleFor( rewrittenInsn ).getMachineCode(); size += 16;
	} /* end rewriteShortOffset() */

/* private refactoring function */
void emulateShortInstruction( IA64_instruction insnToEmulate, Address originalLocation, ia64_bundle_t * insnPtr, unsigned int & offset, unsigned int & size, Address allocatedAddress ) {
	insn_tmpl tmpl = { insnToEmulate.getMachineCode() };

	switch( insnToEmulate.getType() ) {
		case IA64_instruction::DIRECT_BRANCH:
			rewriteShortOffset( insnToEmulate, originalLocation, insnPtr, offset, size, allocatedAddress);
			break; /* end direct jump handling */
			
		case IA64_instruction::DIRECT_CALL: {
			/* Direct calls have to be rewritten as long calls in order to make sure the assertions
			   installBaseTramp() makes about the frame state in the dynamic unwind information
			   are true.  (If we call backwards one instruction, the frame there is not the same as
			   it was at the instruction we're emulating.)  */
			insn_tmpl tmpl = { insnToEmulate.getMachineCode() };
			Address originalTarget = GET_M22_TARGET( &tmpl ) + originalLocation;
			IA64_instruction_x longCall = generateLongCallTo( originalTarget - (allocatedAddress + size), tmpl.B3.b1, tmpl.B3.qp );
            IA64_instruction memoryNOP( NOP_M );			
			IA64_bundle longCallBundle( MLXstop, memoryNOP, longCall );
			insnPtr[offset++] = longCallBundle.getMachineCode(); size += 16;
			} break; /* end direct call handling */		

		case IA64_instruction::BRANCH_PREDICT:
			/* We can suffer the performance loss. :) */
			insnPtr[offset++] = IA64_bundle( MIIstop, NOP_M, NOP_I, NOP_I ).getMachineCode(); size += 16;
			break; /* end branch predict handling */

		case IA64_instruction::ALAT_CHECK:
		case IA64_instruction::SPEC_CHECK:
			/* The advanced load checks can be handled exactly as we 
			   handle direct branches and calls.  The other three checks
			   (I&M unit integer speculation, fp speculation) are handled
			   identically to each other, but their immediates are laid
			   out a little different, so we can't handle them as we do
			   the advanced loads. */
			rewriteShortOffset( insnToEmulate, originalLocation, insnPtr, offset, size, allocatedAddress );

			/* FIXME: the jump back needs to be fixed.  Implies we need to leave the emulated code in-place. */
			break; /* end speculation check handling */

		case IA64_instruction::BRANCH_IA:
			assert( 0 );
			break; /* end branch to x86 handling */

		case IA64_instruction::MOVE_FROM_IP: {
			/* Replace with a movl of the original IP. */
			unsigned int originalRegister = tmpl.I25.r1;
			IA64_instruction memoryNOP( NOP_M );
			IA64_instruction_x emulatedIPMove = generateLongConstantInRegister( originalRegister, originalLocation );
			IA64_bundle ipMoveBundle( MLXstop, memoryNOP, emulatedIPMove );
			insnPtr[offset++] = ipMoveBundle.getMachineCode(); size += 16;
			break; } /* end ip move handling */

		case IA64_instruction::INVALID:
			fprintf( stderr, "Not emulating INVALID instruction.\n" );
			break;

		case IA64_instruction::ALLOC:
			// When emulating an alloc instruction, we must insure that it is the
			// first instruction of an instruction group.
			insnPtr[offset++] = IA64_bundle( MstopMIstop, NOP_M, insnToEmulate, NOP_I ).getMachineCode(); size += 16;
			break;

		case IA64_instruction::RETURN:
		case IA64_instruction::INDIRECT_CALL:
		case IA64_instruction::INDIRECT_BRANCH:
			/* Branch registers hold absolute addresses. */
		default:
			insnPtr[offset++] = generateBundleFor( insnToEmulate ).getMachineCode(); size += 16;
			break; /* end default case */
		} /* end type switch */
	} /* end emulateShortInstruction() */

/* private refactoring function.  The first argument is self-explanatory; originalLocation refers to the address
   of the bundle in the remote process.  The insnPtr is a pointer to (start of) the buffer into which the emulated instruction
   (identified by slotNo) is to be written.  'offset' is the index of the last bundle written to the buffer, and 'size'
   is the size in bytes of those bundles.  (Granted, one could be calculated from the other, but they should both be
   readily available.)  Since offset and size are both references, the same variables should be used in each call
   to emulateBundle() in a given function.  allocatedAddress is where in the remote process the base tramp will be copied. */
void emulateBundle( IA64_bundle bundleToEmulate, Address originalLocation, ia64_bundle_t * insnPtr, unsigned int & offset, unsigned int & size, Address allocatedAddress, int slotNo ) {
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

	if( slotNo == 0 ) {
		emulateShortInstruction( bundleToEmulate.getInstruction0(), originalLocation, insnPtr, offset, size, allocatedAddress );
		return;
		}
		
	/* By convention, an MLX bundle at 0x...0 has its slot 0 instruction at 0x...0, and
	   its slot 1 instruction at 0x...1; by ignoring requests at slot 2, we make sure that
	   long instructions are always emulated in the middle, which makes post-position
	   instrumentation work. */
	if( bundleToEmulate.hasLongInstruction() && slotNo == 1 ) {
		emulateLongInstruction( bundleToEmulate.getLongInstruction(), originalLocation, insnPtr, offset, size, allocatedAddress );
		}
	else if( bundleToEmulate.hasLongInstruction() && slotNo == 2 ) {
		fprintf( stderr, "Malformed emulation request: second slot of long instruction, ignoring.\n" );
		return;
		}
	else {
		/* Which short instruction do we emulate? */
		if( slotNo == 1 ) {
			emulateShortInstruction( bundleToEmulate.getInstruction1(), originalLocation, insnPtr, offset, size, allocatedAddress );
			}
		else if( slotNo == 2 ) {
			emulateShortInstruction( bundleToEmulate.getInstruction2(), originalLocation, insnPtr, offset, size, allocatedAddress );
			}
		else {
			assert( 0 );
			}
		}
	} /* end emulateBundle() */

/* Required by BPatch_thread.C */
#define NEAR_ADDRESS 0x2000000000000000               /* The lower end of the shared memory segment. */
bool process::replaceFunctionCall( const instPoint * point, const int_function * function ) {
	/* We'll assume that point is a call site, primarily because we don't actually care.
	   However, the semantics seem to be that we can't replace function calls if we've already
	   instrumented them (which is, of course, garbage), so we _will_ check that. */
	if( baseMap.defines( point ) ) {
		return false;
		}

	/* We'll need this later. */
	IA64_instruction memoryNOP( NOP_M );

	/* Further implicit semantics indicate that a NULL function should be replace by NOPs. */
	IA64_bundle toInsert;
	if( function == NULL ) {
		toInsert = IA64_bundle( MIIstop, NOP_M, NOP_I, NOP_I );
		} /* end if we're inserting NOPs. */

	/* We'd like to generate the call here, but we need to know the IP it'll be
	   at in order to generate the correct offset.  Grr. */

	/* Since we need a full bundle to guarantee we can make the jump, we'll
	   need to insert a minitramp to the emulated instructions.  Gather some
	   of the necessary data. */
	uint8_t slotNo = point->pointAddr() % 16;
	Address alignedAddress = point->pointAddr() - slotNo;

	/* We'll construct the minitramp in the mutator's address space and then copy
	   it over once we're done. */
	ia64_bundle_t codeBuffer[ 7 ];

	/* However, we need to know the IP we'll be jumping to/from, so allocate
	   the minitramp now. */
	bool allocErr; Address allocatedAddress = inferiorMalloc( 7 * sizeof( ia64_bundle_t ), anyHeap, NEAR_ADDRESS, & allocErr );
	assert( ! allocErr );
	assert( allocatedAddress % 16 == 0 );

	/* Acquire the bundle we'll be emulating before we overwrite it. */
	ia64_bundle_t originalBundle;
	assert( readTextSpace( (void *)alignedAddress, 16, & originalBundle ) );
	IA64_bundle bundleToEmulate( originalBundle );

	/* Overwrite the original instrumentation point with a jump to the minitramp. */
	Address offsetToMiniTramp = allocatedAddress /* target */ - alignedAddress /* ip */;
	IA64_instruction_x lbtMT = generateLongBranchTo( offsetToMiniTramp );
	IA64_bundle jumpBundle( MLXstop, memoryNOP, lbtMT );
	ia64_bundle_t rawJumpBundle = jumpBundle.getMachineCode();
	assert( writeTextSpace( (void *)alignedAddress, 16, & rawJumpBundle ) );

	/* We're not sure how many bundles emulation will take up ahead of time, so offset is an output parameter. */
	unsigned int offset = 0;
	unsigned int size = 0;

	/* We don't want to emulate the call instruction itself. */
	if( slotNo != 0 ) {
		emulateShortInstruction( bundleToEmulate.getInstruction0(), alignedAddress, codeBuffer, offset, size, allocatedAddress );
		}
	else {
		if( function != NULL ) {
			/* The SCRAG says that b0 is the return register, but there's no gaurantee of this.
			   If we have problems, we can decode the call instruction. */
			uint8_t branchRegister = 0;
			int64_t callOffset = function->get_address() - (allocatedAddress + (offset * 16));

			IA64_instruction_x callToFunction = generateLongCallTo( callOffset, branchRegister );
			toInsert = IA64_bundle( MLXstop, memoryNOP, callToFunction );
			} /* end if we have a function call to insert. */

		codeBuffer[ offset++ ] = toInsert.getMachineCode(); size += 16;
		}

	if( bundleToEmulate.hasLongInstruction() ) {
		if( slotNo == 0 ) {
			emulateLongInstruction( bundleToEmulate.getLongInstruction(), alignedAddress, codeBuffer, offset, size, allocatedAddress );
			}
		else {
			if( function != NULL ) {
				/* The SCRAG says that b0 is the return register, but there's no gaurantee of this.
				   If we have problems, we can decode the call instruction. */
				uint8_t branchRegister = 0;
				int64_t callOffset = function->get_address() - (allocatedAddress + (offset * 16));

				IA64_instruction_x callToFunction = generateLongCallTo( callOffset, branchRegister );
				toInsert = IA64_bundle( MLXstop, memoryNOP, callToFunction );
				} /* end if we have a function call to insert. */

			codeBuffer[ offset++ ] = toInsert.getMachineCode(); size += 16;			
			}
		} /* end if we have a long instruction */
	else {
		/* Handle the instruction in slot 1. */
		if( slotNo != 1 ) {
			emulateShortInstruction( bundleToEmulate.getInstruction1(), alignedAddress, codeBuffer, offset, size, allocatedAddress );
			}
		else {
			if( function != NULL ) {
				/* The SCRAG says that b0 is the return register, but there's no gaurantee of this.
				   If we have problems, we can decode the call instruction. */
				uint8_t branchRegister = 0;
				int64_t callOffset = function->get_address() - (allocatedAddress + (offset * 16));

				IA64_instruction_x callToFunction = generateLongCallTo( callOffset, branchRegister );
				toInsert = IA64_bundle( MLXstop, memoryNOP, callToFunction );
				} /* end if we have a function call to insert. */

			codeBuffer[ offset++ ] = toInsert.getMachineCode(); size += 16;			
			}

		/* Handle the instruction in slot 2. */
		if( slotNo != 2 ) {
			emulateShortInstruction( bundleToEmulate.getInstruction2(), alignedAddress, codeBuffer, offset, size, allocatedAddress );
			}
		else {
			if( function != NULL ) {
				/* The SCRAG says that b0 is the return register, but there's no gaurantee of this.
				   If we have problems, we can decode the call instruction. */
				uint8_t branchRegister = 0;
				int64_t callOffset = function->get_address() - (allocatedAddress + (offset * 16));

				IA64_instruction_x callToFunction = generateLongCallTo( callOffset, branchRegister );
				toInsert = IA64_bundle( MLXstop, memoryNOP, callToFunction );
				} /* end if we have a function call to insert. */

			codeBuffer[ offset++ ] = toInsert.getMachineCode(); size += 16;		
			}
		} /* end if all three are short instructions. */

	/* Append the return jump to the minitramp. */
	Address returnOffset = ( alignedAddress + 16 ) /* target */ - (allocatedAddress + (offset * 16)) /* IP */;
	IA64_instruction_x lbtOriginal = generateLongBranchTo( returnOffset );
	IA64_bundle jumpBackBundle( MLXstop, memoryNOP, lbtOriginal );
	codeBuffer[ offset++ ] = jumpBackBundle.getMachineCode(); size += 16;

	/* Write the minitramp into the mutatee's address space. */
	// /* DEBUG */ fprintf( stderr, "* Wrote minitramp at 0x%lx for %d bytes, from 0x%lx\n", allocatedAddress, size + 16, codeBuffer );
	assert( writeTextSpace( (void *)allocatedAddress, size + 16, & codeBuffer ) );

	// /* DEBUG */ fprintf( stderr, "* Emitted minitramp at local 0x%lx, for instPoint at 0x%lx and remote 0x%lx\n", (Address) & codeBuffer, alignedAddress, allocatedAddress );
	return false;
	} /* end replaceFunctionCall() */

/* Private Refactoring Function

   Moves preserved unstacked registers to register stack for basetramp.
   FIXME: handling for ar.fpsr (status field 1), ar.rsc (mode), and User Mask.
*/
void generateRegisterStackSave( ia64_bundle_t * insnPtr, int & bundleCount, unw_dyn_region_info_t * unwindRegion )
{
	IA64_instruction insn[ 3 ];
	IA64_bundle bundle;
	int slot;

	int extraStackSize = 0;
	for( int i = 0; i < BP_R_MAX; ++i )
		if( regSpace->storageMap[ i ] < 0 ) {
			// If the memory stack was needed, ar.unat was stashed,
			// and the memory stack will be shifted down by 16 bytes.
			extraStackSize += 16;
			break;
		}
	int originalFrameSize = regSpace->originalLocals + regSpace->originalOutputs;
	if( originalFrameSize == 96 ) {
		// If the original register frame was full, an fp register will be spilled,
		// and the memory stack will be shifted down by another 16 bytes.
		extraStackSize += 16;
	}

	// Save the general registers.
	slot = 0;
	for( Register i = BP_GR0; i < BP_GR0 + 128; ++i ) {
		if( regSpace->storageMap[ i ] > 0 ) {
			if( i == BP_GR0 + 12 ) {
				// SP is a special case.
				insn[ slot++ ] = generateShortImmediateAdd( regSpace->storageMap[ i ],
															regSpace->sizeOfStack + extraStackSize,
															REGISTER_SP );
			} else {
				// Default case.
				insn[ slot++ ] = generateRegisterToRegisterMove( i - BP_GR0, regSpace->storageMap[ i ] );
			}
		}
		if( slot == 3 || (i == BP_GR0 + 127 && slot) ) {
			if( slot < 2 ) insn[ 1 ] = IA64_instruction( NOP_I );
			if( slot < 3 ) insn[ 2 ] = IA64_instruction( NOP_I );

			bundle = IA64_bundle( MII, insn[ 0 ], insn[ 1 ], insn[ 2 ] );
			insnPtr[ bundleCount++ ] = bundle.getMachineCode();
			unwindRegion->insn_count += 3;

			slot = 0;
		}
	}

	// Save the branch registers
	slot = 1;
	for( Register i = BP_BR0; i < BP_BR0 + 8; ++i ) {
		if( regSpace->storageMap[ i ] > 0 ) {
			insn[ slot++ ] = generateBranchToRegisterMove( i - BP_BR0, regSpace->storageMap[ i ] );

			// Register unwind information for RP.
			if( i == BP_BR0 )
				_U_dyn_op_save_reg( & unwindRegion->op[ unwindRegion->op_count++ ], _U_QP_TRUE,
									unwindRegion->insn_count + (slot-1), UNW_IA64_RP,
									regSpace->storageMap[ i ] );
		}

		if( slot == 3 || (i == BP_BR0 + 7 && slot != 1) ) {
			if( slot < 3 ) insn[ 2 ] = IA64_instruction( NOP_I );

			bundle = IA64_bundle( MII, NOP_I, insn[ 1 ], insn[ 2 ] );
			insnPtr[ bundleCount++ ] = bundle.getMachineCode();
			unwindRegion->insn_count += 3;

			slot = 1;
		}
	}

	// Save the application registers.
	slot = 0;
	bool isPFS = false;
	for( Register i = BP_AR0; i < BP_AR0 + 128; ++i ) {
		if( regSpace->storageMap[ i ] > 0 ) {
			if( i == BP_AR_PFS ) {
				// Special case for ar.pfs:  Account for I-type move instruction.
				// ASSUMPTION:  ar.pfs does not change between instrumentation point
				//              and basetramp state preservation.
				insn[ 2 ] = generateApplicationToRegisterMove( i - BP_AR0, regSpace->storageMap[ i ] );
				insn[ slot++ ] = IA64_instruction( NOP_M );
				isPFS = true;

			} else {
				// Default case.
				insn[ slot++ ] = generateApplicationToRegisterMove( i - BP_AR0, regSpace->storageMap[ i ] );
			}
		}

		if( slot == 2 || (i == BP_AR0 + 127 && slot) ) {
			if( slot < 2 ) insn[ 1 ] = IA64_instruction( NOP_M );

			bundle = IA64_bundle( MMI, insn[ 0 ], insn[ 1 ], ( isPFS ? insn[ 2 ] : NOP_I ) );
			insnPtr[ bundleCount++ ] = bundle.getMachineCode();
			unwindRegion->insn_count += 3;

			slot = 0;
			isPFS = false;
		}
	}

	// Save the predicate registers
	insn[ 0 ] = IA64_instruction( NOP_M );
	if( regSpace->storageMap[ BP_PR ] > 0 ) {
		insn[ 1 ] = generatePredicatesToRegisterMove( regSpace->storageMap[ BP_PR ] );
		_U_dyn_op_save_reg( & unwindRegion->op[ unwindRegion->op_count++ ], _U_QP_TRUE,
							unwindRegion->insn_count + (slot-1), UNW_IA64_PR,
							regSpace->storageMap[ BP_PR ] );

	} else {
		// This bundle is needed to end the instruction group, regardless.
		insn[ 1 ] = IA64_instruction( NOP_I );
	}
	insn[ 2 ] = IA64_instruction( NOP_I );

	bundle = IA64_bundle( MIIstop, insn[ 0 ], insn[ 1 ], insn[ 2 ] );
	insnPtr[ bundleCount++ ] = bundle.getMachineCode();
	unwindRegion->insn_count += 3;
}

/* Private Refactoring Function

   Moves preserved registers back to original location for basetramp trailer.
   FIXME: handling for ar.fpsr (status field 1), ar.rsc (mode), and User Mask.
*/
void generateRegisterStackRestore( ia64_bundle_t * insnPtr, int & bundleCount, unw_dyn_region_info_t * unwindRegion )
{
	IA64_instruction insn[ 3 ];
	IA64_bundle bundle;
	int slot;

	// Restore the general registers.
	slot = 0;
	for( Register i = BP_GR0; i < BP_GR0 + 128; ++i ) {
		if( i != BP_GR0 + 12 && regSpace->storageMap[ i ] > 0 )
			insn[ slot++ ] = generateRegisterToRegisterMove( regSpace->storageMap[ i ], i - BP_GR0 );

		if( slot == 3 || (i == BP_GR0 + 127 && slot) ) {
			if( slot < 2 ) insn[ 1 ] = IA64_instruction( NOP_I );
			if( slot < 3 ) insn[ 2 ] = IA64_instruction( NOP_I );

			bundle = IA64_bundle( MII, insn[ 0 ], insn[ 1 ], insn[ 2 ] );
			insnPtr[ bundleCount++ ] = bundle.getMachineCode();
			unwindRegion->insn_count += 3;

			slot = 0;
		}
	}

	// Restore the branch registers
	slot = 1;
	for( Register i = BP_BR0; i < BP_BR0 + 8; ++i ) {
		if( regSpace->storageMap[ i ] > 0 )
			insn[ slot++ ] = generateRegisterToBranchMove( regSpace->storageMap[ i ], i - BP_BR0 );

		if( slot == 3 || (i == BP_BR0 + 7 && slot != 1) ) {
			if( slot < 3 ) insn[ 2 ] = IA64_instruction( NOP_I );

			bundle = IA64_bundle( MII, NOP_I, insn[ 1 ], insn[ 2 ] );
			insnPtr[ bundleCount++ ] = bundle.getMachineCode();
			unwindRegion->insn_count += 3;

			slot = 1;
		}
	}

	// Restore the application registers.
	slot = 0;
	bool isPFS = false;
	for( Register i = BP_AR0; i < BP_AR0 + 128; ++i ) {
		if( regSpace->storageMap[ i ] > 0 ) {
			if( i == BP_AR_PFS ) {
				// Special case for ar.pfs:  Account for I-type move instruction.
				insn[ 2 ] = generateRegisterToApplicationMove( regSpace->storageMap[ i ], i - BP_AR0 );
				insn[ slot++ ] = IA64_instruction( NOP_M );
				isPFS = true;

			} else {
				// Default case.
				insn[ slot++ ] = generateRegisterToApplicationMove( regSpace->storageMap[ i ], i - BP_AR0 );
			}
		}

		if( slot == 2 || (i == BP_AR0 + 127 && slot) ) {
			if( slot < 2 ) insn[ 1 ] = IA64_instruction( NOP_M );

			bundle = IA64_bundle( MMI, insn[ 0 ], insn[ 1 ], ( isPFS ? insn[ 2 ] : NOP_I ) );
			insnPtr[ bundleCount++ ] = bundle.getMachineCode();
			unwindRegion->insn_count += 3;

			slot = 0;
			isPFS = false;
		}
	}

	// Restore the predicate registers
	insn[ 0 ] = IA64_instruction( NOP_M );
	if( regSpace->storageMap[ BP_PR ] > 0 ) {
		insn[ 1 ] = generateRegisterToPredicatesMove( regSpace->storageMap[ BP_PR ], ~0x1 );
	} else {
		// This bundle is needed to end the instruction group, regardless.
		insn[ 1 ] = IA64_instruction( NOP_I );
	}
	insn[ 2 ] = IA64_instruction( NOP_I );

	bundle = IA64_bundle( MIIstop, insn[ 0 ], insn[ 1 ], insn[ 2 ] );
	insnPtr[ bundleCount++ ] = bundle.getMachineCode();
	unwindRegion->insn_count += 3;
}

/* Private Refactoring Function

   Spills stacked registers to memory stack, if needed.
*/
void generateMemoryStackSave( ia64_bundle_t * insnPtr, int & bundleCount, bool * usedFPregs, unw_dyn_region_info_t * unwindRegion )
{
	Register temp_gr[ 2 ] = { 6 };
	Register temp_fr = 6;

	IA64_instruction insn[ 2 ];
	IA64_bundle bundle;
	int grStackOffset = 0;
	int frStackOffset = 0;

	bool grSpillNeeded = false;
	for( int i = 0; i < BP_R_MAX; ++i )
		if( regSpace->storageMap[ i ] < frStackOffset ) {
			grSpillNeeded = true;
			frStackOffset = regSpace->storageMap[ i ];
		}
	frStackOffset  = 32 + (-frStackOffset * 8);
	frStackOffset += frStackOffset % 16;

	int originalFrameSize = regSpace->originalLocals + regSpace->originalOutputs;
	if( grSpillNeeded ) {
		if( originalFrameSize == 96 ) {
			// Free a general register when all 96 stacked registers are in use.
			// We also do this when 95 registers are in use because our new alloc
			// instruction uses the only free register.
			//
			// Algorithm taken from:
			// V. Ramasamy, R. Hundt, "Dynamic Binary Instrumentation for Intel Itanium
			// Processor Family," EPIC1 Workshop, MICRO34, Dec 1-5, 2001

			bundle = IA64_bundle( MstopMI,
								  generateShortImmediateAdd( REGISTER_SP, -16, REGISTER_SP ),
								  generateFPSpillTo( REGISTER_SP, temp_fr, -16 ),
								  NOP_I );
			_U_dyn_op_add( & unwindRegion->op[ unwindRegion->op_count++ ], _U_QP_TRUE,
						   unwindRegion->insn_count + 0, UNW_IA64_SP, (unw_word_t)-16 );
			_U_dyn_op_add( & unwindRegion->op[ unwindRegion->op_count++ ], _U_QP_TRUE,
						   unwindRegion->insn_count + 1, UNW_IA64_SP, (unw_word_t)-16 );
			insnPtr[ bundleCount++ ] = bundle.getMachineCode();
			unwindRegion->insn_count += 3;

			bundle = IA64_bundle( MMIstop,
								  generateRegisterToFloatMove( temp_gr[ 0 ], temp_fr ),
								  generateApplicationToRegisterMove( AR_UNAT, temp_gr[ 0 ] ),
								  NOP_I );
			insnPtr[ bundleCount++ ] = bundle.getMachineCode();
			unwindRegion->insn_count += 3;

		} else {
			// Since at least one stacked register is unused, the highest
			// register allocated for basetramp must be free.
			temp_gr[ 0 ] = regSpace->getRegSlot( regSpace->getRegisterCount() - 1 )->number;

			bundle = IA64_bundle( MMIstop,
								  generateShortImmediateAdd( REGISTER_SP, -16, REGISTER_SP ),
								  generateApplicationToRegisterMove( AR_UNAT, temp_gr[ 0 ] ),
								  NOP_I );
			_U_dyn_op_add( & unwindRegion->op[ unwindRegion->op_count++ ], _U_QP_TRUE,
						   unwindRegion->insn_count + 0, UNW_IA64_SP, (unw_word_t)-16 );
			insnPtr[ bundleCount++ ] = bundle.getMachineCode();
			unwindRegion->insn_count += 3;
		}

		// Store ar.unat
		bundle = IA64_bundle( MIIstop,
							  generateSpillTo( REGISTER_SP, temp_gr[ 0 ], 0 ),
							  NOP_I,
							  NOP_I );
		insnPtr[ bundleCount++ ] = bundle.getMachineCode();
		unwindRegion->insn_count += 3;
	}
	bundle = IA64_bundle( MIIstop,
						  generateShortImmediateAdd( REGISTER_SP, -regSpace->sizeOfStack, REGISTER_SP ),
						  NOP_I,
						  NOP_I );
	_U_dyn_op_add( & unwindRegion->op[ unwindRegion->op_count++ ], _U_QP_TRUE,
				   unwindRegion->insn_count, UNW_IA64_SP, -regSpace->sizeOfStack );
	insnPtr[ bundleCount++ ] = bundle.getMachineCode();
	unwindRegion->insn_count += 3;

	// Save the general registers, if needed.
	// FIXME: This could be optimized to use more free registers, if they exist.
	for( Register i = BP_GR0; i < BP_GR0 + 128; ++i ) {
		if( regSpace->storageMap[ i ] < 0 ) {
			grStackOffset = 32 + ( ( -regSpace->storageMap[ i ] - 1 ) * 8 );

			bundle = IA64_bundle( MstopMIstop,
								  generateShortImmediateAdd( temp_gr[ 0 ], grStackOffset, REGISTER_SP ),
								  generateSpillTo( temp_gr[ 0 ], i, 0 ),
								  NOP_I );
			insnPtr[ bundleCount++ ] = bundle.getMachineCode();
			unwindRegion->insn_count += 3;
		}
	}

	//
	// At this point, we have NUM_LOCALS + NUM_OUTPUT + NUM_PRESERVED
	// general registers free.
	//
	if( grSpillNeeded ) {
		if( originalFrameSize == 96 ) {
			temp_gr[ 1 ] = regSpace->getRegSlot( 1 )->number;
			// Undo register swapping to maintain uniform register state.
			bundle = IA64_bundle( MMIstop,
								  generateFloatToRegisterMove( temp_fr, temp_gr[ 0 ] ),
								  generateShortImmediateAdd( temp_gr[ 1 ], regSpace->sizeOfStack + 16, REGISTER_SP ),
								  NOP_I );
			insnPtr[ bundleCount++ ] = bundle.getMachineCode();
			unwindRegion->insn_count += 3;

			bundle = IA64_bundle( MIIstop,
								  generateFPFillFrom( temp_gr[ 1 ], temp_fr, 0 ),
								  NOP_I,
								  NOP_I );
			insnPtr[ bundleCount++ ] = bundle.getMachineCode();
			unwindRegion->insn_count += 3;
		}
	}

	// Save the FP regs
	temp_gr[ 0 ] = regSpace->getRegSlot( 0 )->number;
	temp_gr[ 1 ] = regSpace->getRegSlot( 1 )->number;

	bundle = IA64_bundle( MIIstop,
						  generateShortImmediateAdd( temp_gr[ 0 ], frStackOffset +  0, REGISTER_SP ),
						  generateShortImmediateAdd( temp_gr[ 1 ], frStackOffset + 16, REGISTER_SP ),
						  NOP_I );
	insnPtr[ bundleCount++ ] = bundle.getMachineCode();
	unwindRegion->insn_count += 3;

	// Save the floating point registers
	int slot = 0;
	for( int i = 0; i < 128; i++ ) {
		if( usedFPregs[i] ) {
			insn[ slot ] = generateFPSpillTo( temp_gr[ slot ], i, 32 );
			++slot;
		}

		if( slot == 2 || (i == 127 && slot) ) {
			if( slot < 2 ) insn[ 1 ] = IA64_instruction( NOP_M );

			bundle = IA64_bundle( MMIstop, insn[ 0 ], insn[ 1 ], NOP_I );
			insnPtr[ bundleCount++ ] = bundle.getMachineCode();
			unwindRegion->insn_count += 3;

			slot = 0;
		}
	}
}

/* Private Refactoring Function
   Fills stacked registers from memory stack, if needed.
*/
void generateMemoryStackRestore( ia64_bundle_t * insnPtr, int & bundleCount, bool * usedFPregs, unw_dyn_region_info_t * unwindRegion )
{
	Register temp_gr[ 2 ] = { 6 };
	Register temp_fr = 6;

	IA64_instruction insn[ 2 ];
	IA64_bundle bundle;
	int grStackOffset = 0;
	int frStackOffset = 0;

	bool grSpillNeeded = false;
	for( int i = 0; i < BP_R_MAX; ++i )
		if( regSpace->storageMap[ i ] < frStackOffset ) {
			grSpillNeeded = true;
			frStackOffset = regSpace->storageMap[ i ];
		}
	frStackOffset  = 32 + (-frStackOffset * 8);
	frStackOffset += frStackOffset % 16;

	int originalFrameSize = regSpace->originalLocals + regSpace->originalOutputs;

	// Prepare offsets for the FP regs
	temp_gr[ 0 ] = regSpace->getRegSlot( 0 )->number;
	temp_gr[ 1 ] = regSpace->getRegSlot( 1 )->number;

	bundle = IA64_bundle( MIIstop,
						  generateShortImmediateAdd( temp_gr[ 0 ], frStackOffset +  0, REGISTER_SP ),
						  generateShortImmediateAdd( temp_gr[ 1 ], frStackOffset + 16, REGISTER_SP ),
						  NOP_I );
	insnPtr[ bundleCount++ ] = bundle.getMachineCode();
	unwindRegion->insn_count += 3;

	// Restore the FP registers
	int slot = 0;
	for( int i = 0; i < 128; i++ ) {
		if( usedFPregs[i] ) {
			insn[ slot ] = generateFPFillFrom( temp_gr[ slot ], i, 32 );
			++slot;
		}

		if( slot == 2 || (i == 127 && slot) ) {
			if( slot < 2 ) insn[ 1 ] = IA64_instruction( NOP_M );

			bundle = IA64_bundle( MMIstop, insn[ 0 ], insn[ 1 ], NOP_I );
			insnPtr[ bundleCount++ ] = bundle.getMachineCode();
			unwindRegion->insn_count += 3;

			slot = 0;
		}
	}

	// Now we need to find the one guaranteed free register.
	if( grSpillNeeded ) {
		if( originalFrameSize == 96 ) {
			temp_gr[ 0 ] = 6;
			temp_gr[ 1 ] = regSpace->getRegSlot( 1 )->number;
			// Redo register swapping to free up one extra general register..
			bundle = IA64_bundle( MstopMI,
								  generateShortImmediateAdd( temp_gr[ 1 ], regSpace->sizeOfStack + 16, REGISTER_SP ),
								  generateFPSpillTo( temp_gr[ 1 ], temp_fr, 0 ),
								  NOP_I );
			insnPtr[ bundleCount++ ] = bundle.getMachineCode();
			unwindRegion->insn_count += 3;

			bundle = IA64_bundle( MIIstop,
								  generateRegisterToFloatMove( temp_gr[ 0 ], temp_fr ),
								  NOP_I,
								  NOP_I );
			insnPtr[ bundleCount++ ] = bundle.getMachineCode();
			unwindRegion->insn_count += 3;

		} else {
			temp_gr[ 0 ] = regSpace->getRegSlot( regSpace->getRegisterCount() - 1 )->number;
		}

		if( originalFrameSize != 96 && regSpace->storageMap[ BP_AR_PFS ] != 32 + originalFrameSize ) {
			// Move ar.pfs to the expected location.
			bundle = IA64_bundle( MIIstop,
								  generateRegisterToRegisterMove( regSpace->storageMap[ BP_AR_PFS ], 32 + originalFrameSize ),
								  NOP_I,
								  NOP_I );
			insnPtr[ bundleCount++ ] = bundle.getMachineCode();
			unwindRegion->insn_count += 3;
		}
	}

	// Restore the general registers, if needed.
	// FIXME: This could be optimized to use more free registers, if they exist.
	for( Register i = BP_GR0; i < BP_GR0 + 128; ++i ) {
		if( regSpace->storageMap[ i ] < 0 ) {
			grStackOffset = 32 + ( ( -regSpace->storageMap[ i ] - 1 ) * 8 );

			bundle = IA64_bundle( MstopMIstop,
								  generateShortImmediateAdd( temp_gr[ 0 ], grStackOffset, REGISTER_SP ),
								  generateFillFrom( temp_gr[ 0 ], i, 0 ),
								  NOP_I );
			insnPtr[ bundleCount++ ] = bundle.getMachineCode();
			unwindRegion->insn_count += 3;
		}
	}

	//
	// At this point, temp_gr[ 0 ] is the only safe register to use.
	//
	bundle = IA64_bundle( MIIstop,
						  generateShortImmediateAdd( REGISTER_SP, regSpace->sizeOfStack, REGISTER_SP ),
						  NOP_I,
						  NOP_I );
	_U_dyn_op_pop_frames( & unwindRegion->op[ unwindRegion->op_count++ ], _U_QP_TRUE, unwindRegion->insn_count, 1 );
	insnPtr[ bundleCount++ ] = bundle.getMachineCode();
	unwindRegion->insn_count += 3;

	if( grSpillNeeded ) {
		// Restore ar.unat
		bundle = IA64_bundle( MstopMIstop,
							  generateFillFrom( REGISTER_SP, temp_gr[ 0 ], 0 ),
							  generateRegisterToApplicationMove( temp_gr[ 0 ], AR_UNAT ),
							  NOP_I );
		insnPtr[ bundleCount++ ] = bundle.getMachineCode();
		unwindRegion->insn_count += 3;

		bundle = IA64_bundle( MIIstop,
							  generateShortImmediateAdd( REGISTER_SP, 16, REGISTER_SP ),
							  NOP_I,
							  NOP_I );
		_U_dyn_op_pop_frames( & unwindRegion->op[ unwindRegion->op_count++ ], _U_QP_TRUE, unwindRegion->insn_count, 1 );
		insnPtr[ bundleCount++ ] = bundle.getMachineCode();
		unwindRegion->insn_count += 3;

		if( originalFrameSize == 96 ) {
			bundle = IA64_bundle( MMIstop,
								  generateFloatToRegisterMove( temp_fr, temp_gr[ 0 ] ),
								  generateFPFillFrom( REGISTER_SP, temp_fr, 16 ),
								  NOP_I );
			_U_dyn_op_pop_frames( & unwindRegion->op[ unwindRegion->op_count++ ], _U_QP_TRUE, unwindRegion->insn_count + 1, 1 );
			insnPtr[ bundleCount++ ] = bundle.getMachineCode();
			unwindRegion->insn_count += 3;
		}
	}
}

/* private refactoring function */
bool generatePreservationHeader( ia64_bundle_t * insnPtr, Address & count, bool * whichToPreserve, unw_dyn_region_info_t * unwindRegion ) {
	/* For clarity (in the callers, too), handle NULL unwindRegions here. */
	bool freeUnwindRegion = false;
	if( unwindRegion == NULL ) {
		unwindRegion = (unw_dyn_region_info_t *)malloc( _U_dyn_region_info_size( PRESERVATION_UNWIND_OPERATION_COUNT ) );
		assert( unwindRegion != NULL );
		unwindRegion->op_count = 0;
		unwindRegion->insn_count = 0;
		
		freeUnwindRegion = true;
		} /* end if unwindRegion was NULL */

	/* How many bundles did we use?  Update the (byte)'count' at the end. */
	int bundleCount = 0;

	IA64_instruction integerNOP( NOP_I );
	IA64_instruction memoryNOP( NOP_M );
	IA64_bundle bundle;

	/* Assume, in the absence of information, that we wanted to preserve everything. */
	if( whichToPreserve == NULL ) {
		whichToPreserve = (bool *)malloc( 128 * sizeof( bool ) );
		for( int i = 0; i < 128; i++ ) {
			whichToPreserve[i] = true;
			} /* end initialization loop */

		/* Never try to preserve fp 0, 1, 2 - 5, or 16 - 31, because we don't use
		   them and the function we're calling will preserve them if it uses them. */
		whichToPreserve[ 0 ] = false;
		whichToPreserve[ 1 ] = false;

		whichToPreserve[ 2 ] = false;
		whichToPreserve[ 3 ] = false;
		whichToPreserve[ 4 ] = false;
		whichToPreserve[ 5 ] = false;

		whichToPreserve[ 16 ] = false;
		whichToPreserve[ 17 ] = false;
		whichToPreserve[ 18 ] = false;
		whichToPreserve[ 19 ] = false;
		whichToPreserve[ 20 ] = false;
		whichToPreserve[ 21 ] = false;
		whichToPreserve[ 22 ] = false;
		whichToPreserve[ 23 ] = false;
		whichToPreserve[ 24 ] = false;
		whichToPreserve[ 25 ] = false;
		whichToPreserve[ 26 ] = false;
		whichToPreserve[ 27 ] = false;
		whichToPreserve[ 28 ] = false;
		whichToPreserve[ 29 ] = false;
		whichToPreserve[ 30 ] = false;
		whichToPreserve[ 31 ] = false;		
		} /* end if whichToPreserve is NULL */

	/* The alloc instruction here generated both preserves the stacked registers
	   and provides us enough dead stacked registers to preserve the unstacked
	   general registers, several application registers, and branch registers
	   we need in order to ensure that we can make arbitrary function calls from
	   the minitramps without (further) altering the mutatee's semantics.

	   If the original alloc already uses all 96 registers, don't generate
	   a new alloc.
	*/
	int originalFrameSize = regSpace->originalLocals + regSpace->originalOutputs;
	if( originalFrameSize < 96 ) {
		bundle = IA64_bundle( MIIstop,
							  generateAllocInstructionFor( regSpace, NUM_LOCALS, NUM_OUTPUT, regSpace->originalRotates ),
							  integerNOP,
							  integerNOP );
		insnPtr[ bundleCount++ ] = bundle.getMachineCode();

		// ar.pfs is initially stashed in the only register known dead register.
		_U_dyn_op_save_reg( & unwindRegion->op[ unwindRegion->op_count++ ], _U_QP_TRUE,
							unwindRegion->insn_count, UNW_IA64_AR_PFS, 32 + originalFrameSize );
		unwindRegion->insn_count += 3;
	}

	// generateMemoryStackSave() *MUST* be called before generateRegisterStackSave()
	generateMemoryStackSave( insnPtr, bundleCount, whichToPreserve, unwindRegion );
	generateRegisterStackSave( insnPtr, bundleCount, unwindRegion );

	// /* DEBUG */ fprintf( stderr, "Emitted %d-bundle preservation header at 0x%lx\n", bundleCount, (Address)insnPtr ); 

	/* Update the offset. */
	assert( unwindRegion->op_count < PRESERVATION_UNWIND_OPERATION_COUNT );
	if( freeUnwindRegion ) { free( unwindRegion ); }
	count += (bundleCount * 16);
	return true;
	} /* end generatePreservationHeader() */

/* private refactoring function; assumes that regSpace and
   deadRegisterList are as they were in the corresponding call
   to generatePreservationHeader(). */
bool generatePreservationTrailer( ia64_bundle_t * insnPtr, Address & count, bool * whichToPreserve, unw_dyn_region_info_t * unwindRegion ) {
	/* For clarity (in the callers, too), handle NULL unwindRegions here. */
	bool freeUnwindRegion = false;
	if( unwindRegion == NULL ) {
		unwindRegion = (unw_dyn_region_info_t *)malloc( _U_dyn_region_info_size( PRESERVATION_UNWIND_OPERATION_COUNT ) );
		assert( unwindRegion != NULL );
		unwindRegion->op_count = 0;
		unwindRegion->insn_count = 0;
		
		freeUnwindRegion = true;
		} /* end if unwindRegion was NULL */

	/* How many bundles did we use?  Update the (byte)'count' at the end. */
	int bundleCount = 0;
	int originalFrameSize = regSpace->originalLocals + regSpace->originalOutputs;

	IA64_instruction memoryNOP( NOP_M );
	IA64_instruction integerNOP( NOP_I );

	/* Assume, in the absence of information, that we wanted to preserve everything. */
	if( whichToPreserve == NULL ) {
		whichToPreserve = (bool *)malloc( 128 * sizeof( bool ) );
		for( int i = 0; i < 128; i++ ) {
			whichToPreserve[i] = true;
			} /* end initialization loop */

		/* Never try to preserve fp 0, 1, 2 - 5, or 16 - 31, because we don't use
		   them and the function we're calling will preserve them if it uses them. */
		whichToPreserve[ 0 ] = false;
		whichToPreserve[ 1 ] = false;

		whichToPreserve[ 2 ] = false;
		whichToPreserve[ 3 ] = false;
		whichToPreserve[ 4 ] = false;
		whichToPreserve[ 5 ] = false;

		whichToPreserve[ 16 ] = false;
		whichToPreserve[ 17 ] = false;
		whichToPreserve[ 18 ] = false;
		whichToPreserve[ 19 ] = false;
		whichToPreserve[ 20 ] = false;
		whichToPreserve[ 21 ] = false;
		whichToPreserve[ 22 ] = false;
		whichToPreserve[ 23 ] = false;
		whichToPreserve[ 24 ] = false;
		whichToPreserve[ 25 ] = false;
		whichToPreserve[ 26 ] = false;
		whichToPreserve[ 27 ] = false;
		whichToPreserve[ 28 ] = false;
		whichToPreserve[ 29 ] = false;
		whichToPreserve[ 30 ] = false;
		whichToPreserve[ 31 ] = false;		
		} /* end if whichToPreserve is NULL */

	/* generateRegisterStackRestore() *MUST* be called before generateMemoryStackRestore() */
	generateRegisterStackRestore( insnPtr, bundleCount, unwindRegion );
	generateMemoryStackRestore( insnPtr, bundleCount, whichToPreserve, unwindRegion );

	/* Restore the original frame. */
	if( originalFrameSize < 96 ) {
		/* Restore ar.pfs; preserved by the header's alloc instruction. */
		IA64_instruction movePFS = generateRegisterToApplicationMove( 32 + originalFrameSize, AR_PFS );
		IA64_bundle fifthMoveBundle( MIIstop, memoryNOP, movePFS, integerNOP );
		insnPtr[ bundleCount ++ ] = fifthMoveBundle.getMachineCode();
		unwindRegion->insn_count += 3;

		_U_dyn_op_stop( & unwindRegion->op[ unwindRegion->op_count ] );

		IA64_instruction allocInsn = generateOriginalAllocFor( regSpace );
		IA64_bundle allocBundle( MIIstop, allocInsn, NOP_I, NOP_I );
		insnPtr[ bundleCount++ ] = allocBundle.getMachineCode();
		unwindRegion->insn_count += 3;
	} else {
		_U_dyn_op_stop( & unwindRegion->op[ unwindRegion->op_count ] );
	}
	/* FIXME: It's only after the above alloc executes that anything changes from the
	   POV of the unwinder, because the preserved values stay where they were
	   preserved until then. */

	// /* DEBUG */ fprintf( stderr, "Emitted %d-bundle preservation trailer at 0x%lx\n", bundleCount, (Address)insnPtr ); 

	/* Update the offset. */
	assert( unwindRegion->op_count < PRESERVATION_UNWIND_OPERATION_COUNT );
	if( freeUnwindRegion ) { free( unwindRegion ); }
	count += (bundleCount * 16);
	return true;
	} /* end generatePreservationTrailer() */

/* Originally from linux-ia64.C's executingSystemCall(). */
bool needToHandleSyscall( process * proc, bool * pcMayHaveRewound ) {
	ia64_bundle_t rawBundle;
	uint64_t iip, ri;
	int64_t pr;

	// Bad things happen if you use ptrace on a running process.
	assert( proc->status() == stopped );
	errno = 0;

	// Find the correct bundle.
	iip = getPC( proc->getPid() );
	reg_tmpl reg = { P_ptrace( PTRACE_PEEKUSER, proc->getPid(), PT_CR_IPSR, 0 ) };
	if( errno && (reg.raw == -1) ) {
		// Error reading process information.  Should we assert here?
		assert(0);
		}
	ri = reg.PSR.ri;
	if (ri == 0) iip -= 16;  // Get previous bundle, if necessary.

	/* As above; if the syscall rewinds the PC, we must as well. */
	if( pcMayHaveRewound != NULL ) {
		if( ri == 0 ) { * pcMayHaveRewound = true; }
		else { * pcMayHaveRewound = false; }
		}

	// Read bundle data
	if( ! proc->readDataSpace( (void *)iip, 16, (void *)&rawBundle, true ) ) {
		// Could have gotten here because the mutatee stopped right
		// after a jump to the beginning of a memory segment (aka,
		// no previous bundle).  But, that can't happen from a syscall.
		return false;
		}

	// Isolate previous instruction.
	ri = (ri + 2) % 3;
	IA64_bundle origBundle( rawBundle );
	IA64_instruction *insn = origBundle.getInstruction(ri);

	// Determine predicate register and remove it from instruction.
	pr = P_ptrace( PTRACE_PEEKUSER, proc->getPid(), PT_PR, 0 );
	if (errno && pr == -1) assert(0);
	pr = ( pr >> insn->getPredicate() ) & 0x1;

	return (insn->getType() == IA64_instruction::SYSCALL && !pr);
	} /* end needToHandleSyscall() */

#include "dlfcn.h"
bool emitSyscallHeader( process * proc, void * insnPtr, Address & baseBytes ) {
	/* Extract the current slotNo. */
	errno = 0;
	reg_tmpl reg = { P_ptrace( PTRACE_PEEKUSER, proc->getPid(), PT_CR_IPSR, 0 ) };
	assert( ! errno );
	uint64_t slotNo = reg.PSR.ri;
	assert( slotNo <= 2 );

	// /* DEBUG */ fprintf( stderr, "emitSyscallHeader() thinks slotNo = %ld\n", slotNo );

	/* When the kernel continues execution of the now-stopped process,
	   it may or may not decrement the slot counter to restart the
	   interrupted system call.  Since we need to resume after the iRPC
	   where the kernel thinks we should, we need to change the slot of 
	   the iRPC's terminal break, since we can't alter ipsr.ri otherwise.
	   
	   The header code is templated; we just replace the first bundle(s) with
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
	   function pointer, not the function pointer proper.  This is probably PIC breakage. */
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
	IA64_bundle firstJumpBundle;
	IA64_bundle secondJumpBundle;
	IA64_instruction branchNOP( NOP_B );
	/* Probably wouldn't hurt to assert that the immediates here are small enough. */
	IA64_instruction jumpToInSyscall = generateShortImmediateBranch( prefixInSyscallAddress - (syscallPrefixAddress + 0x10 ) );
	IA64_instruction jumpToNotInSyscall = generateShortImmediateBranch( prefixNotInSyscallAddress - (syscallPrefixAddress + 0x10) );
	switch( slotNo ) {
		case 0:
			jumpToInSyscall = generateShortImmediateBranch( prefixInSyscallAddress - syscallPrefixAddress );
			firstJumpBundle = IA64_bundle( BBBstop, branchNOP, branchNOP, jumpToInSyscall );
			secondJumpBundle = IA64_bundle( BBBstop, jumpToNotInSyscall, branchNOP, branchNOP );
			break;
		case 1:
			firstJumpBundle = IA64_bundle( BBBstop, branchNOP, branchNOP, branchNOP );
			secondJumpBundle = IA64_bundle( BBBstop, jumpToInSyscall, jumpToNotInSyscall, branchNOP );
			break;
		case 2:
			firstJumpBundle = IA64_bundle( BBBstop, branchNOP, branchNOP, branchNOP );
			secondJumpBundle = IA64_bundle( BBBstop, branchNOP, jumpToInSyscall, jumpToNotInSyscall );
			break;
		default:
			assert( 0 );
			break;
		}
	bundlePtr[ 0 ] = firstJumpBundle.getMachineCode();
	bundlePtr[ 1 ] = secondJumpBundle.getMachineCode();

	// /* DEBUG */ fprintf( stderr, "* iRPC system call handler (prefix) generated at 0x%lx\n", (Address) bundlePtr );
	return true;
	} /* end emitSystemCallHeader() */

/* private refactoring function */
bool * doFloatingPointStaticAnalysis( const instPoint * location ) {
	/* Cast away const-ness rather than fix broken int_function::getAddress(). */
	int_function * functionBase = location->pointFunc();
	Address mutatorAddress = (Address) location->getOwner()->getPtrToInstruction( functionBase->getAddress( NULL ) ) ;

	IA64_iterator iAddr( mutatorAddress );
	Address lastI = mutatorAddress + functionBase->get_size();
	// /* DEBUG */ fprintf( stderr, "Mutator claims to have mutatee machine code from 0x%lx to 0x%lx\n", mutatorAddress, lastI );
	// /* DEBUG */ fprintf( stderr, "(claims to be function '%s')\n", functionBase->symTabName().c_str() );

	bool fpUsed = false;
	bool registersRotated = false;
	bool * whichToPreserve = (bool *)calloc( 128, sizeof( bool ) );

	for( int slotNo = 0; iAddr < lastI; iAddr ++ ) {
		IA64_instruction * currInsn = * iAddr;
		insn_tmpl tmpl = { currInsn->getMachineCode() };

		bool instructionIsFP = false;
		bool instructionIsMem = false;
		uint8_t templateID = currInsn->getTemplateID();
		switch( templateID ) {
			case 0x00:
			case 0x01:
			case 0x02:
			case 0x03:
				/* MII */
			case 0x04:
			case 0x05:
				/* MLX */
				instructionIsMem = (slotNo == 0);
				break;

			case 0x08:
			case 0x09:
			case 0x0A:
			case 0x0B:
				/* MMI */
				instructionIsMem = (slotNo == 0 || slotNo == 1);
				break;

			case 0x10:
			case 0x11:
				/* MIB */
			case 0x12:
			case 0x13:
				/* MBB */
				instructionIsMem = (slotNo == 0);
				break;

			case 0x18:
			case 0x19:
				/* MMB */
				instructionIsMem = (slotNo == 0 || slotNo == 1);
				break;

			case 0x0C:
			case 0x0D:
				/* MFI */
			case 0x1C:
			case 0x1D:
				/* MFB */
				instructionIsFP = (slotNo == 1);
				instructionIsMem = (slotNo == 0);
				break;

			case 0x0E:
			case 0x0F:
				/* MMF */
				instructionIsFP = (slotNo == 2);
				instructionIsMem = (slotNo == 0 || slotNo == 1);
				break;

			default:
				break;
				} /* end switch */

		if( instructionIsMem ) {
			/* Decide which fields in the instruction actually contain register numbers. */
			switch( GET_OPCODE( &tmpl ) ) {
				case 0x0: {
					int8_t x2 = tmpl.M_SYS.x2;
					int8_t x3 = tmpl.M_SYS.x3;
					int8_t x4 = tmpl.M_SYS.x4;

					/* M23 */ if( x3 == 0x6 || x3 == 0x7 )
						whichToPreserve[ tmpl.M23.f1 ] = true;

					/* M27 */ if( x3 == 0x0 && x4 == 0x3 && x2 == 0x1 )
						whichToPreserve[ tmpl.M27.f1 ] = true;
				} break;

				case 0x1: {
					int8_t x3 = tmpl.M_SYS.x3;

					/* M21 */ if( x3 == 0x3 )
						whichToPreserve[ tmpl.M21.f2 ] = true;
				} break;

				case 0x4: {
					int8_t m = tmpl.M_LD_ST.m;
					int8_t x = tmpl.M_LD_ST.x;
					int8_t x6 = tmpl.M_LD_ST.x6;

					/* M19 */ if( m == 0x0 && x == 0x1 && 0x1C <= x6 && x6 <= 0x1F )
						whichToPreserve[ tmpl.M19.f2 ] = true;
				} break;

				case 0x6: {
					int8_t m = tmpl.M_LD_ST.m;
					int8_t x = tmpl.M_LD_ST.x;
					int8_t x6 = tmpl.M_LD_ST.x6;

					if( x == 0x0 && m == 0x0 ) {
						/* M6  */ if( (0x00 <= x6 && x6 <= 0x0F) || (0x20 <= x6 && x6 <= 0x27) || x6 == 0x1B )
							whichToPreserve[ tmpl.M6.f1 ] = true;

						/* M9  */ if( (0x30 <= x6 && x6 <= 0x32) || x6 == 0x3B )
							whichToPreserve[ tmpl.M9.f2 ] = true;
					}
					if( x == 0x0 && m == 0x1 ) {
						/* M7  */ if( (0x00 <= x6 && x6 <= 0x0F) || (0x20 <= x6 && x6 <= 0x27) || x6 == 0x1B )
							whichToPreserve[ tmpl.M7.f1 ] = true;

						/* M10 */ if( (0x30 <= x6 && x6 <= 0x33) || (x6 == 0x3B) )
							whichToPreserve[ tmpl.M10.f2 ] = true;
					}
					if( x == 0x1 && m == 0x0 ) {
						/* M11 */ if( (0x01 <= x6 && x6 <= 0x0F && x6 != 0x04 && x6 != 0x08 && x6 != 0x0C && x6 != 0x24) ||
									  (0x21 <= x6 && x6 <= 0x27 && x6 != 0x24) ) {
							whichToPreserve[ tmpl.M11.f1 ] = true;
							whichToPreserve[ tmpl.M11.f2 ] = true;
						}
						/* M18 */ if( 0x1C <= x6 && x6 <= 0x1F )
							whichToPreserve[ tmpl.M18.f1 ] = true;
					}
					if( x == 0x1 && m == 0x1 ) {
						/* M12 */ if( (0x01 <= x6 && x6 <= 0x0F && x6 != 0x04 && x6 != 0x08 && x6 != 0x0C && x6 != 0x24) ||
									  (0x21 <= x6 && x6 <= 0x27 && x6 != 0x24) ) {
							whichToPreserve[ tmpl.M12.f1 ] = true;
							whichToPreserve[ tmpl.M12.f2 ] = true;
						}
					}
				} break;

				case 0x7: {
					int8_t x6 = tmpl.M_LD_ST.x6;

					/* M8  */ if( (0x00 <= x6 && x6 <= 0x0F) || (0x20 <= x6 && x6 <= 0x27) || x6 == 0x1B )
						whichToPreserve[ tmpl.M8.f1 ] = true;

					/* M10 */ if( (0x30 <= x6 && x6 <= 0x32) || x6 == 0x3B )
						whichToPreserve[ tmpl.M10.f2 ] = true;
				} break;
			}
		}

		if( instructionIsFP ) {
			// /* DEBUG */ fprintf( stderr, "Instruction at mutator address 0x%lx uses an FPU.\n", iAddr.getEncodedAddress() );

			/* Decide which fields in the instruction actually contain register numbers. */
			switch( GET_OPCODE( &tmpl ) ) {
				case 0x8:
				case 0x9:
				case 0xA:
				case 0xB:
				case 0xC:
				case 0xD:
				case 0xE:
					/* F1, F2, F3 */
					whichToPreserve[ tmpl.F1.f1 ] = true;
					whichToPreserve[ tmpl.F1.f2 ] = true;
					whichToPreserve[ tmpl.F1.f3 ] = true;
					whichToPreserve[ tmpl.F1.f4 ] = true;
					fpUsed = true;
					break;
					
				case 0x4:
					/* F4 */
					whichToPreserve[ tmpl.F4.f2 ] = true;
					whichToPreserve[ tmpl.F4.f3 ] = true;
					fpUsed = true;
					break;				
					
				case 0x5:
					/* F5 */
					whichToPreserve[ tmpl.F5.f2 ] = true;
					fpUsed = true;
					break;
					
				case 0x0:
				case 0x1: {
					/* F6, F7, F8, F9, F10, F11 */
					uint64_t bitX = tmpl.F6.x;
					uint64_t bitQ = tmpl.F6.q;

					if( bitX == 0x1 && bitQ == 0x0 ) {
						/* F6 */
						whichToPreserve[ tmpl.F6.f2 ] = true;
						whichToPreserve[ tmpl.F6.f3 ] = true;
						fpUsed = true;
						}
					if( bitX == 0x1 && bitQ == 0x1 ) {
						/* F7 */
						whichToPreserve[ tmpl.F7.f3 ] = true;
						fpUsed = true;
						}
					if( bitX == 0x0 ) {
						uint64_t x6 = tmpl.F8.x6;

						if( ( 0x14 <= x6 && x6 <= 0x17 ) || ( 0x30 <= x6 && x6 <= 0x37 ) ) {
							/* F8 */
							whichToPreserve[ tmpl.F8.f1 ] = true;
							whichToPreserve[ tmpl.F8.f2 ] = true;
							whichToPreserve[ tmpl.F8.f3 ] = true;
							fpUsed = true;
							}
						if( ( 0x10 <= x6 && x6 <= 0x12 ) || ( 0x2C <= x6 && x6 <= 0x2F ) || 
							( 0x34 <= x6 && x6 <= 0x36 ) || ( 0x39 <= x6 && x6 <= 0x3D ) || x6 == 0x28 ) {
							/* F9 */
							whichToPreserve[ tmpl.F9.f1 ] = true;
							whichToPreserve[ tmpl.F9.f2 ] = true;
							whichToPreserve[ tmpl.F9.f3 ] = true;
							fpUsed = true;
							}
						if( 0x18 <= x6 && x6 <= 0x1B ) {
							/* F10 */
							whichToPreserve[ tmpl.F10.f1 ] = true;
							whichToPreserve[ tmpl.F10.f2 ] = true;
							fpUsed = true;
							}
						if( 0x1C == x6 ) {
							/* F11 */
							whichToPreserve[ tmpl.F11.f1 ] = true;
							whichToPreserve[ tmpl.F11.f2 ] = true;
							fpUsed = true;
							}
						} /* end if bitX is 0 .*/
					} break;

				default:
					/* F12, F13, F14, and F15 are actually in case 0, but they don't use FP registers. */
					break;
				} /* end opcode switch */
			} /* end if instructionIsFP */

		/* For simplicity's sake, look for the register-rotating loops separately. */
		switch( templateID ) {
			case 0x10:
			case 0x11:
				/* MIB */
			case 0x12:
			case 0x13:
				/* MBB */
			case 0x16:
			case 0x17:
				/* BBB */
			case 0x18:
			case 0x19:
				/* MMB */
			case 0x1C:
			case 0x1D: {
				/* MFB */
				if( slotNo == 2 ) {
					if( GET_OPCODE( &tmpl ) == 0x4 ) {
						uint64_t btype = tmpl.B1.btype;
						if( btype == 0x02 || btype == 0x03 || btype == 0x06 || btype == 0x07 ) {
							// /* DEBUG */ fprintf( stderr, "Instruction at mutator address 0x%lx rotates registers.\n", iAddr.getEncodedAddress() );
							registersRotated = true;
							} /* end if it's the right btype */
						} /* end if it's the right opcode */
					} /* end if it could be ctop, cexit, wtop, wexit */
				} break;
				
			default:
				break;
			} /* end templateID switch for rotating loops */

		/* Increment the slotNo. */
		slotNo = (slotNo + 1) % 3;
		} /* end instruction iteration */

	if( registersRotated && fpUsed ) {
		for( int i = 32; i < 128; i++ ) { whichToPreserve[i] = true; }
		} /* end if we have to preserve all the stacked registers. */

	/* Never try to preserve fp 0, 1, 2 - 5, or 16 - 31, because we don't use
	   them and the function we're calling will preserve them if it uses them. */
	whichToPreserve[ 0 ] = false;
	whichToPreserve[ 1 ] = false;

	whichToPreserve[ 2 ] = false;
	whichToPreserve[ 3 ] = false;
	whichToPreserve[ 4 ] = false;
	whichToPreserve[ 5 ] = false;

	whichToPreserve[ 16 ] = false;
	whichToPreserve[ 17 ] = false;
	whichToPreserve[ 18 ] = false;
	whichToPreserve[ 19 ] = false;
	whichToPreserve[ 20 ] = false;
	whichToPreserve[ 21 ] = false;
	whichToPreserve[ 22 ] = false;
	whichToPreserve[ 23 ] = false;
	whichToPreserve[ 24 ] = false;
	whichToPreserve[ 25 ] = false;
	whichToPreserve[ 26 ] = false;
	whichToPreserve[ 27 ] = false;
	whichToPreserve[ 28 ] = false;
	whichToPreserve[ 29 ] = false;
	whichToPreserve[ 30 ] = false;
	whichToPreserve[ 31 ] = false;
																	
	functionBase->usedFPregs = whichToPreserve;
	return whichToPreserve;
 	} /* end doFloatingPointStaticAnalysis() */ 

#define INVALID_CFM 0x10000000000
extern void initBaseTrampStorageMap( registerSpace *, int, bool * );

/* Required by process.C */
bool rpcMgr::emitInferiorRPCheader( void * insnPtr, Address & baseBytes ) {
	/* Extract the CFM. */
	errno = 0;
	reg_tmpl reg = { P_ptrace( PTRACE_PEEKUSER, proc_->getPid(), PT_CFM, 0 ) };
	assert( ! errno );

	/* FIXME: */ if( ! ( 0 == reg.CFM.rrb_pr == reg.CFM.rrb_fr == reg.CFM.rrb_gr ) ) { assert( 0 ); }
	/* FIXME: */ if( reg.raw == INVALID_CFM ) { assert( 0 ); }

	/* Set regSpace for the code generator. */
	Register deadRegisterList[NUM_LOCALS + NUM_OUTPUT];
	for( int i = 0; i < NUM_LOCALS + NUM_OUTPUT; i++ ) {
		deadRegisterList[i] = 32 + reg.CFM.sof + i + NUM_PRESERVED;
		} /* end deadRegisterList population */
	registerSpace rs( NUM_LOCALS + NUM_OUTPUT, deadRegisterList, 0, NULL );

	initBaseTrampStorageMap( &rs, reg.CFM.sof, NULL );
	rs.originalLocals = reg.CFM.sol;
	rs.originalOutputs = reg.CFM.sof - reg.CFM.sol;
	rs.originalRotates = reg.CFM.sor;

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
		   up the extra slots with nops.  (Because the syscall header may require
		   a bundle _before_ the jump target, add two NOPs; the installation routine(s)
		   will compensate.) */
		bundlePtr[0] = IA64_bundle( MIIstop, NOP_M, NOP_I, NOP_I ).getMachineCode();
		baseBytes += 16; bundlePtr++;
		bundlePtr[0] = IA64_bundle( MIIstop, NOP_M, NOP_I, NOP_I ).getMachineCode();		
		baseBytes += 16; bundlePtr++;
		}

	/* Determine which of the scratch (f6 - f15, f32 - f127) floating-point
	   registers need to be preserved.  It's been brought to my attention that 
	   figuring this out will probably take longer than just spilling all the fp
	   registers, considering that an iRPC only runs once. */
	// Address interruptedAddress = getPC( proc_->getPid() );
	// int_function * interruptedFunction = proc_->findFuncByAddr( interruptedAddress );
	bool * whichToPreserve = NULL; // doFloatingPointStaticAnalysis( interruptedFunction->funcEntry( proc_ ) );

	/* Generate the preservation header; don't bother with unwind information
	   for an inferior RPC.  (It must be the top of the stack.) */
	return generatePreservationHeader( bundlePtr, baseBytes, whichToPreserve, NULL );
	} /* end emitInferiorRPCheader() */

bool emitSyscallTrailer( void * insnPtr, Address & baseBytes, uint64_t slotNo ) {
	/* Copy the template from 'syscallSuffix' to 'suffixExitPoint' (inclusive),
	   and replace the bundle at 'suffixExitPoint' with one which has 
	   predicated SIGILLs in the right places. */

	// /* DEBUG */ fprintf( stderr, "emitSyscallTrailer() thinks slotNo = %ld\n", slotNo );

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
			// PROBLEM: in this case, the PC may need to be backed up!
			predicatedBreakBundle = IA64_bundle( MMIstop, notInSyscallBreak, NOP_M, inSyscallBreak );
			assert( 0 );
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

	// /* DEBUG */ fprintf( stderr, "* iRPC system call handler (suffix) generated at 0x%lx\n", (Address) bundlePtr );
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
		// * DEBUG */ fprintf( stderr, "* iRPC will stop for result.\n" );
		stopForResultOffset = offset + (bundleCount * 16);
		bundlePtr[ bundleCount++ ] = trapBundle.getMachineCode();
		justAfter_stopForResultOffset = offset + (bundleCount * 16);
		/* Make sure that we eat the ipsr.ri with a nop. */
		bundlePtr[ bundleCount++ ] = IA64_bundle( MIIstop, NOP_M, NOP_I, NOP_I ).getMachineCode();
		offset += bundleCount * 16;
		} /* end if we're interested in the result. */

	/* Determine which of the scratch (f6 - f15, f32 - f127) floating-point
	   registers need to be preserved.  See comment in emitInferiorRPCHeader()
	   for why we don't actually do this. */
	// Address interruptedAddress = getPC( proc_->getPid() );
	// int_function * interruptedFunction = proc_->findFuncByAddr( interruptedAddress );
	bool * whichToPreserve = NULL; // doFloatingPointStaticAnalysis( interruptedFunction->funcEntry( proc_ ) );

	/* Generate the restoration code. */
	bundlePtr = (ia64_bundle_t *)( ((Address)insnPtr) + offset );
	generatePreservationTrailer( bundlePtr, offset, whichToPreserve, NULL );

	/* Regenerate the bundle ptr, etc, to reflect the preservation trailer. */
	assert( ( ((Address)insnPtr) + offset ) % 16 == 0 );
	bundlePtr = (ia64_bundle_t *)( ((Address)insnPtr) + offset );
	bundleCount = 0;
	
	/* The SIGILL for the demon needs to happen in the instruction slot
	   corresponding to ipsr.ri so that the mutatee resumes in the correct
	   location after the daemon adjusts its PC. */
	errno = 0;
	reg_tmpl reg = { P_ptrace( PTRACE_PEEKUSER, proc_->getPid(), PT_CR_IPSR, 0 ) };
	assert( ! errno );
	uint64_t slotNo = reg.PSR.ri;
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


unsigned int installTrampGuardOn(registerSpace *rs, process *proc, ia64_bundle_t *insnPtr)
{
	// Assumptions: No need to "allocate" registers from registerSpace.  They are
	//				scratch registers which just came into existence.
	//
	//				The branch in guardOnBundle3 must jump past 3 bundles (Offset +4).
	//					1 for the miniTramp
	//					2 for the guardOff bundles

	int maxRegister = rs->getRegisterCount();
	Register trampGuardFlagAddr = rs->getRegSlot( maxRegister - 1 )->number;
	Register trampGuardFlagValue = rs->getRegSlot( maxRegister - 2 )->number;

	IA64_bundle guardOnBundle0( MLXstop,
								IA64_instruction( NOP_M ),
								generateLongConstantInRegister( trampGuardFlagAddr, proc->trampGuardAddr() ));

	IA64_bundle guardOnBundle1( MstopMIstop,
								generateRegisterLoad( trampGuardFlagValue, trampGuardFlagAddr, 4 ),
								generateComparison( eqOp, 6, trampGuardFlagValue, REGISTER_ZERO ),
								IA64_instruction( NOP_I ));

	IA64_bundle guardOnBundle2( MLXstop,
								generateRegisterStore( trampGuardFlagAddr, REGISTER_ZERO, 4, 7),
								generateLongBranchTo( +4 << 4, 6 ));

	insnPtr[0] = guardOnBundle0.getMachineCode();
	insnPtr[1] = guardOnBundle1.getMachineCode();
	insnPtr[2] = guardOnBundle2.getMachineCode();

	return 3; // Number of bundles added to insnPtr
}

unsigned int installTrampGuardOff(registerSpace *rs, process *proc, ia64_bundle_t *insnPtr)
{
	int maxRegister = rs->getRegisterCount();
    Register trampGuardFlagAddr = rs->getRegSlot( maxRegister - 1 )->number;
	Register trampGuardFlagValue = rs->getRegSlot( maxRegister - 2 )->number;

    IA64_bundle guardOffBundle0( MLXstop,
								 generateShortConstantInRegister( trampGuardFlagValue, 1 ),
								 generateLongConstantInRegister( trampGuardFlagAddr, proc->trampGuardAddr() ));

    IA64_bundle guardOffBundle1( MMIstop,
								 generateRegisterStore( trampGuardFlagAddr, trampGuardFlagValue, 4 ),
								 IA64_instruction( NOP_M ),
								 IA64_instruction( NOP_I ));

    insnPtr[0] = guardOffBundle0.getMachineCode();
    insnPtr[1] = guardOffBundle1.getMachineCode();

	return 2; // Number of bundles added to insnPtr
}

void registerRemoteUnwindInformation( unw_word_t di, unw_word_t udil, pid_t pid ) {
	unw_dyn_info_list_t dummy_dil;
	unw_word_t generationOffset = ((unw_word_t)(& dummy_dil.generation)) - ((unw_word_t)(& dummy_dil));
	unw_word_t firstOffset = ((unw_word_t)(& dummy_dil.first)) - ((unw_word_t)(& dummy_dil));
  
	unw_dyn_info_t dummy_di;
	unw_word_t nextOffset = ((unw_word_t)(& dummy_di.next)) - ((unw_word_t)(& dummy_di));
	unw_word_t prevOffset = ((unw_word_t)(& dummy_di.prev)) - ((unw_word_t)(& dummy_di));
  
	errno = 0;
  
	/* Notionally, we should lock from here to the last POKETEXT.  However,
	   locking in the local process will not suffice if the remote process
	   is also using libunwind.  In that case, the right thing to do is use
	   the remote process's lock.  Since the remote process is stopped, if
	   the lock isn't held, we don't have to do anything; if it is, we need
	   to single-step out of dyn_register().  However, this may do Very Strange
	   Things to our process control model, so I'm ignoring it for now. */
	unw_word_t generation, first;
  	
	generation = P_ptrace( PTRACE_PEEKTEXT, pid, (udil + generationOffset), (Address)NULL );
	assert( errno == 0 );
    
	++generation;
    
	P_ptrace( PTRACE_POKETEXT, pid, (udil + generationOffset), generation );
	assert( errno == 0 );
    
    
	first = P_ptrace( PTRACE_PEEKTEXT, pid, (udil + firstOffset), (Address)NULL );
	assert( errno == 0 );
    
	P_ptrace( PTRACE_POKETEXT, pid, (di + nextOffset), first );
	assert( errno == 0 );
	P_ptrace( PTRACE_POKETEXT, pid, (di + prevOffset), (Address)NULL );
	assert( errno == 0 );
    
	if ( first ) {
		P_ptrace( PTRACE_POKETEXT, pid, (first + prevOffset), di );
		assert( errno == 0 );
		}
    
	P_ptrace( PTRACE_POKETEXT, pid, (udil + firstOffset), di );
	assert( errno == 0 );  
	} /* end registerRemoteUnwindInformation() */

/* FIXME: copy bTDI and its pointed-to regions into the remote process, adjusting the pointers as we go.
   Assume that the region's op_count fields are accurate, and only copy over the correct amount. */
bool insertAndRegisterDynamicUnwindInformation( unw_dyn_info_t * baseTrampDynamicInfo, process * proc ) {
	/* Note: assumes no 'handler' routine(s) in baseTrampDynamicInfo->u.pi. */
	Address addressOfbTDI = proc->inferiorMalloc( sizeof( unw_dyn_info_t ) );
	assert( addressOfbTDI != (Address)NULL );
	// /* DEBUG */ fprintf( stderr, "address of baseTrampDynamicInfo = 0x%lx\n", addressOfbTDI );
	
	/* Copying the string over every time is wasteful, but simple. */
	Address lengthOfName = strlen( (char *) baseTrampDynamicInfo->u.pi.name_ptr );
	Address addressOfName = proc->inferiorMalloc( lengthOfName + 1 );
	assert( addressOfName != (Address)NULL );
	// /* DEBUG */ fprintf( stderr, "address of name = 0x%lx\n", addressOfName );
	
	assert( proc->writeDataSpace( (void *)addressOfName, lengthOfName + 1, (void *)baseTrampDynamicInfo->u.pi.name_ptr ) );
	baseTrampDynamicInfo->u.pi.name_ptr = addressOfName;

	/* Allocate the first region. */
	unw_dyn_region_info_t * nextRegion = baseTrampDynamicInfo->u.pi.regions;
	Address sizeOfNextRegion = _U_dyn_region_info_size( nextRegion->op_count );
	Address addressOfNextRegion = proc->inferiorMalloc( sizeOfNextRegion );
	baseTrampDynamicInfo->u.pi.regions = (unw_dyn_region_info_t *)addressOfNextRegion;
	// /* DEBUG */ fprintf( stderr, "nextRegion = %p, sizeOfNextRegion = 0x%lx, addressOfNextRegion = 0x%lx\n", nextRegion, sizeOfNextRegion, addressOfNextRegion );

	unw_dyn_region_info_t * currentRegion = nextRegion;	
	Address sizeOfCurrentRegion = sizeOfNextRegion;
	Address addressOfCurrentRegion = addressOfNextRegion;
	// /* DEBUG */ fprintf( stderr, "currentRegion = %p, sizeOfCurrentRegion = 0x%lx, addressOfCurrentRegion = 0x%lx\n", currentRegion, sizeOfCurrentRegion, addressOfCurrentRegion );	
	
	/* Copy baseTrampDynamicInfo itself over, now that we've updated all of its pointers. */
	assert( proc->writeDataSpace( (void *)addressOfbTDI, sizeof( unw_dyn_info_t ), baseTrampDynamicInfo ) );
	
	/* Iteratively allocate region (n + 1), update region n, and then copy it over. */
	// /* DEBUG */ fprintf( stderr, "Beginning iterations over region list.\n" );
	while( currentRegion->next != NULL ) {
		nextRegion = currentRegion->next;
		sizeOfNextRegion = _U_dyn_region_info_size( nextRegion->op_count );
		
		/* Allocate. */
		addressOfNextRegion = proc->inferiorMalloc( sizeOfNextRegion );
		assert( addressOfNextRegion != (Address)NULL );
		
		/* Update. */
		currentRegion->next = (unw_dyn_region_info_t *)addressOfNextRegion;
		
		/* Copy. */
		// /* DEBUG */ fprintf( stderr, "(at copy) nextRegion = %p, sizeOfNextRegion = 0x%lx, addressOfNextRegion = 0x%lx\n", nextRegion, sizeOfNextRegion, addressOfNextRegion );
		// /* DEBUG */ fprintf( stderr, "(at copy) currentRegion = %p, sizeOfCurrentRegion = 0x%lx, addressOfCurrentRegion = 0x%lx\n", currentRegion, sizeOfCurrentRegion, addressOfCurrentRegion );	
		assert( proc->writeDataSpace( (void *)addressOfCurrentRegion, sizeOfCurrentRegion, currentRegion ) );

		/* Iterate. */
		currentRegion = nextRegion;
		sizeOfCurrentRegion = sizeOfNextRegion;
		addressOfCurrentRegion = addressOfNextRegion;
		} /* end region iteration */
	
	/* Copy the n + 1 region. */
	// /* DEBUG */ fprintf( stderr, "(at last copy) currentRegion = %p, sizeOfCurrentRegion = 0x%lx, addressOfCurrentRegion = 0x%lx\n", currentRegion, sizeOfCurrentRegion, addressOfCurrentRegion );	
	assert( proc->writeDataSpace( (void *)addressOfCurrentRegion, sizeOfCurrentRegion, currentRegion ) );
	
	/* We need the address of the _U_dyn_info_list in the remote process in order
	   to register the baseTrampDynamicInfo. */
	bool couldNotFind = false;
	Address addressOfuDIL = proc->findInternalAddress( "_U_dyn_info_list", false, couldNotFind );
	if( couldNotFind ) {
		return ! proc->isBootstrappedYet();
		}
	
	/* Register baseTrampDynamicInfo in remote process. */
	// /* DEBUG */ fprintf( stderr, "%s[%d]: registering remote address range [0x%lx - 0x%lx) with gp = 0x%lx with uDIL at 0x%lx\n", __FILE__, __LINE__, baseTrampDynamicInfo->start_ip, baseTrampDynamicInfo->end_ip, baseTrampDynamicInfo->gp, addressOfuDIL );
	registerRemoteUnwindInformation( addressOfbTDI, addressOfuDIL, proc->getPid() );
	return true;
	} /* end insertAndRegisterDynamicUnwindInformation() */

/**
 * We use the same assembler for the basetramp and the header
 * and trailer of an inferior RPC; a basetramp is two header/trailer
 * pairs, with inside each pair and the emulated instructions between
 * them.  The same registerSpace must be used throughout the code
 * generation (including the minitramps) for the results to be sensible.
 */

/* private refactoring function */
#define MAX_BASE_TRAMP_SIZE (16 * 512)	// FIXME: it would be a lot cleaner to determine this dynamically.
trampTemplate * installBaseTramp( Address installationPoint, instPoint * & location, process * proc, bool trampRecursiveDesired = false )
{ // FIXME: updatecost
	// /* DEBUG */ fprintf( stderr, "* Installing base tramp.\n" );
	/* Allocate memory and align as needed. */
	trampTemplate * baseTramp = new trampTemplate(location, proc);

	bool allocErr; Address allocatedAddress = proc->inferiorMalloc( MAX_BASE_TRAMP_SIZE, anyHeap, NEAR_ADDRESS, & allocErr );
	if( allocErr ) { fprintf( stderr, "Unable to allocate base tramp, aborting.\n" ); abort(); }
	if( allocatedAddress % 16 != 0 ) { allocatedAddress = allocatedAddress - (allocatedAddress % 16); }

	/* Initialize the tramp template. */
	baseTramp->baseAddr = allocatedAddress;
	baseTramp->size = 0;

	/* Acquire the base address of the object we're instrumenting. */
	Address baseAddress; assert( proc->getBaseAddress( location->getOwner(), baseAddress ) );
	assert( baseAddress % 16 == 0 );

	Address origBundleAddr = location->pointAddr() & ~0xF;
	int slotNo = location->pointAddr() % 16;

	// /* DEBUG */ fprintf( stderr, "* Installing base tramp at 0x%lx, emulating instruction at 0x%lx slotNo %d.\n", installationPoint, origBundleAddr + baseAddress, slotNo );

	/* Begin constructing the dynamic unwind information for libunwind. */
	unw_dyn_info_t * baseTrampDynamicInfo = (unw_dyn_info_t *)calloc( 1, sizeof( unw_dyn_info_t ) );
	assert( baseTrampDynamicInfo != NULL );
	
	baseTrampDynamicInfo->next = NULL;
	baseTrampDynamicInfo->prev = NULL;
	
	/* The first region will simply ALIAS to the instrumentation point. */
	unw_dyn_region_info_t * regionZero = (unw_dyn_region_info_t *)malloc( _U_dyn_region_info_size( 2 ) );
	assert( regionZero != NULL );
	
	regionZero->insn_count = 0;
	regionZero->op_count = 2;
	// /* DEBUG */ fprintf( stderr, "aliasing basetramp (beginning) to 0x%lx\n", origBundleAddr + baseAddress );
	_U_dyn_op_alias( & regionZero->op[0], _U_QP_TRUE, -1, origBundleAddr + baseAddress );
	_U_dyn_op_stop( & regionZero->op[1] );
	
	/* Operations for the base tramp proper; this (probably) grossly overestimates the number of operations. */
	unw_dyn_region_info_t * regionOne = (unw_dyn_region_info_t *)malloc( _U_dyn_region_info_size( PRESERVATION_UNWIND_OPERATION_COUNT ) );
	assert( regionOne != NULL );
	regionZero->next = regionOne;
	regionOne->next = NULL;

	regionOne->insn_count = 0;
	regionOne->op_count = 0;

	/* Determine this instrumentation point's regSpace and deadRegisterList (global variables)
	   for use by the rest of the code generator.  The generatePreservation*() functions need
	   this information as well. */
	bool staticallyAnalyzed = defineBaseTrampRegisterSpaceFor( location, regSpace, deadRegisterList, proc );
	if( ! staticallyAnalyzed ) {
		fprintf( stderr, "FIXME: Dynamic determination of register frame required but not yet implemented, aborting.\n" );
		fprintf( stderr, "FIXME: mutatee instrumentation point 0x%lx\n", location->pointAddr() + baseAddress );
		return NULL;
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
	regionOne->insn_count += 3;	

	int_function * pdf = location->pointFunc();

	/* Insert the preservation header. */
	insnPtr = (ia64_bundle_t *)( ((Address)instructions) + baseTramp->size );
	generatePreservationHeader( insnPtr, (Address &) baseTramp->size, pdf->usedFPregs, regionOne );

	/* Update insnPtr to reflect the size of the preservation header. */
	insnPtr = (ia64_bundle_t *)( ((Address)instructions) + baseTramp->size );
	bundleCount = 0;

	/* Insert tramp guard (3 bundles) */
	unsigned int guardCount;
	if (!trampRecursiveDesired) {
		guardCount = installTrampGuardOn(regSpace, proc, insnPtr + bundleCount);
		baseTramp->size += guardCount * 16;
		bundleCount += guardCount;
		regionOne->insn_count += guardCount * 3;	
		}

	/* CODEGEN: Insert the localPre nop bundle.  (Jump to first minitramp.) */
	baseTramp->localPreOffset = baseTramp->size;
	insnPtr[bundleCount++] = nopBundle.getMachineCode(); baseTramp->size += 16; regionOne->insn_count += 3;
	baseTramp->localPreReturnOffset = baseTramp->size;

	if (!trampRecursiveDesired) {
		guardCount = installTrampGuardOff(regSpace, proc, insnPtr + bundleCount);
		baseTramp->size += guardCount * 16;
		bundleCount += guardCount;
		regionOne->insn_count += guardCount * 3;	
	}

	/* Insert the preservation trailer. */
	insnPtr = (ia64_bundle_t *)( ((Address)instructions) + baseTramp->size );
	generatePreservationTrailer( insnPtr, (Address &) baseTramp->size, pdf->usedFPregs, regionOne );

	/* generatePreservationTrailer closed off regionOne for me.  Open a new region. */
	unw_dyn_region_info_t * regionTwoA = (unw_dyn_region_info_t *)malloc( _U_dyn_region_info_size( 2 ) );
	assert( regionTwoA != NULL );
	regionOne->next = regionTwoA;
	regionTwoA->next = NULL;
	
	/* This third region is the same as the first, in that its state is the same as the instrumented function's. */
	regionTwoA->insn_count = 0;
	regionTwoA->op_count = 2;
	// /* DEBUG */ fprintf( stderr, "aliasing basetramp (middle) to 0x%lx\n", origBundleAddr + baseAddress );
	_U_dyn_op_alias( & regionTwoA->op[0], _U_QP_TRUE, -1, origBundleAddr + baseAddress );
	_U_dyn_op_stop( & regionTwoA->op[1] );

	/* Update insnPtr to reflect the size of the preservation trailer. */
	insnPtr = (ia64_bundle_t *)( ((Address)instructions) + baseTramp->size );
	bundleCount = 0;

	/* Removing the last pre-position minitramp will insert a jump
	   from skipPreInsnOffset to updateCostOffset, in attempt to make
	   sure the cost is still updated... not sure why.  See inst.C's
	   clearBaseBranch(). */
	baseTramp->updateCostOffset = baseTramp->size;
	
	/* Prepare bundle for emulation */
	const ia64_bundle_t *origBundlePtr =
		(const ia64_bundle_t *)location->getOwner()->getPtrToInstruction( origBundleAddr );
	assert( (Address)origBundlePtr % 16 == 0 );
	IA64_bundle bundleToEmulate( *origBundlePtr );

	unw_dyn_region_info_t * regionTwoB = (unw_dyn_region_info_t *)malloc( _U_dyn_region_info_size( 1 ) );
	assert( regionTwoB != NULL );
	regionTwoA->next = regionTwoB;
	regionTwoB->next = NULL;
	
	regionTwoB->insn_count = 0;
	regionTwoB->op_count = 1;
	_U_dyn_op_stop( & regionTwoB->op[0] );
	
	/* Emulate the instrumented instruction. */
	baseTramp->emulateInsOffset = baseTramp->size;	
	unsigned int preEmulationSize = baseTramp->size;
	emulateBundle( bundleToEmulate, origBundleAddr + baseAddress, insnPtr, bundleCount, (unsigned int &)(baseTramp->size), allocatedAddress, slotNo );	
	regionTwoB->insn_count = ( (baseTramp->size - preEmulationSize) / 16 ) * 3;
	
	/* CODEGEN: Replace the skipPre nop bundle with a jump from it to baseTramp->emulateInsOffset. */
	unsigned int skipPreJumpBundleOffset = baseTramp->skipPreInsOffset / 16;
	IA64_instruction memoryNOP( NOP_M );
	IA64_instruction_x skipPreJump = generateLongBranchTo( baseTramp->emulateInsOffset - baseTramp->skipPreInsOffset ); 
	IA64_bundle skipPreJumpBundle( MLXstop, memoryNOP, skipPreJump );
	instructions[skipPreJumpBundleOffset] = skipPreJumpBundle.getMachineCode();

	/* CODEGEN: Insert the skipPost nop bundle. */
	baseTramp->skipPostInsOffset = baseTramp->size;
	insnPtr[bundleCount++] = nopBundle.getMachineCode(); baseTramp->size += 16;
	regionTwoB->insn_count += 3;	
	
	/* Open region three. */
	unw_dyn_region_info_t * regionThree = (unw_dyn_region_info_t *)malloc( _U_dyn_region_info_size( PRESERVATION_UNWIND_OPERATION_COUNT ) );
	assert( regionThree != NULL );
	regionTwoB->next = regionThree;
	regionThree->next = NULL;
	
	regionThree->insn_count = 0;
	regionThree->op_count = 0;

	/* Insert the preservation header. */
	insnPtr = (ia64_bundle_t *)( ((Address)instructions) + baseTramp->size );
	generatePreservationHeader( insnPtr, (Address &) baseTramp->size, pdf->usedFPregs, regionThree );

	/* Update insnPtr to reflect the size of the preservation header. */
	insnPtr = (ia64_bundle_t *)( ((Address)instructions) + baseTramp->size );
	bundleCount = 0;

	/* Insert the recursion guard. */
	if (!trampRecursiveDesired) {
		guardCount = installTrampGuardOn(regSpace, proc, insnPtr + bundleCount);
		baseTramp->size += guardCount * 16;
		bundleCount += guardCount;
		regionThree->insn_count += guardCount * 3;	
		}

	/* CODEGEN: Insert the localPost nop bundle. */
	baseTramp->localPostOffset = baseTramp->size;
	insnPtr[bundleCount++] = nopBundle.getMachineCode(); baseTramp->size += 16; regionThree->insn_count += 3;	
	baseTramp->localPostReturnOffset = baseTramp->size;

	if (!trampRecursiveDesired) {
		guardCount = installTrampGuardOff(regSpace, proc, insnPtr + bundleCount);
		baseTramp->size += guardCount * 16;
		bundleCount += guardCount;
		regionThree->insn_count += guardCount * 3;
		}

	/* Insert the preservation trailer. */
	insnPtr = (ia64_bundle_t *)( ((Address)instructions) + baseTramp->size );
	generatePreservationTrailer( insnPtr, (Address &) baseTramp->size, pdf->usedFPregs, regionThree );

	/* generatePreservationTrailer closed off regionThree for me.  Open a new region. */
	unw_dyn_region_info_t * regionFourA = (unw_dyn_region_info_t *)malloc( _U_dyn_region_info_size( 2 ) );
	assert( regionFourA != NULL );
	regionThree->next = regionFourA;
	regionFourA->next = NULL;
	
	/* This fifth region is the same as the first and third, in that its state is the same as the instrumented function's. */
	regionFourA->insn_count = 0;
	regionFourA->op_count = 2;
	// /* DEBUG */ fprintf( stderr, "aliasing basetramp (end) to 0x%lx\n", origBundleAddr + baseAddress );
	_U_dyn_op_alias( & regionFourA->op[0], _U_QP_TRUE, -1, origBundleAddr + baseAddress );
	_U_dyn_op_stop( & regionFourA->op[1] );	

	/* Update insnPtr to reflect the size of the preservation trailer. */
	insnPtr = (ia64_bundle_t *)( ((Address)instructions) + baseTramp->size );
	bundleCount = 0;

	/* The jump to skip post-instrumentation should land here. */
	baseTramp->returnInsOffset = baseTramp->size;

	unw_dyn_region_info_t * regionFourB = (unw_dyn_region_info_t *)malloc( _U_dyn_region_info_size( 1 ) );
	assert( regionFourB != NULL );
	regionFourA->next = regionFourB;
	regionFourB->next = NULL;

	regionFourB->insn_count = 0;
	regionFourB->op_count = 1;
	_U_dyn_op_stop( & regionFourB->op[0] );	

	/* Insert the jump back to the multi-tramp. */
	Address returnAddress = installationPoint + 16;
	IA64_instruction_x returnFromTrampoline = generateLongBranchTo( returnAddress - (baseTramp->baseAddr + baseTramp->size) );
	IA64_bundle returnBundle( MLXstop, memoryNOP, returnFromTrampoline );
	insnPtr[bundleCount++] = returnBundle.getMachineCode(); baseTramp->size += 16;
	regionFourB->insn_count += 3;

	/* Replace the skipPost nop bundle with a jump from it to baseTramp->returnInsOffset. */
	unsigned int skipPostJumpBundleOffset = baseTramp->skipPostInsOffset / 16;
	IA64_instruction_x skipPostJump = generateLongBranchTo( baseTramp->returnInsOffset - baseTramp->skipPostInsOffset );
	IA64_bundle skipPostJumpBundle( MLXstop, memoryNOP, skipPostJump );
	instructions[skipPostJumpBundleOffset] = skipPostJumpBundle.getMachineCode();

	/* Copy the instructions from hither to thither. */
	InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( allocatedAddress, proc );
	iAddr.writeBundlesFrom( (unsigned char *)instructions, baseTramp->size / 16  );

	/* Insert the jump from the multi-tramp.  Do NOT execute at instPoint's instrumentation
	   if the corresponding instruction will not execute because of predication. */
	// /* DEBUG */ if( location->iPgetInstruction()->getPredicate() != 0 ) { fprintf( stderr, "%s[%d]: Predicating jump at 0x%lx (for 0x%lx) with PR %d, machine code 0x%lx\n", __FILE__, __LINE__, installationPoint, location->pointAddr() + baseAddress, location->iPgetInstruction()->getPredicate(), location->iPgetInstruction()->getMachineCode() ); }
	IA64_instruction_x jumpToBaseInstruction = generateLongBranchTo( allocatedAddress - installationPoint, location->iPgetInstruction()->getPredicate() );
	IA64_bundle jumpToBaseBundle( MLXstop, memoryNOP, jumpToBaseInstruction );
	InsnAddr jAddr = InsnAddr::generateFromAlignedDataAddress( installationPoint, proc );
	jAddr.replaceBundleWith( jumpToBaseBundle );

	/* Fill out the dynamic unwind accounting information. */	
	baseTrampDynamicInfo->start_ip = baseTramp->baseAddr;
	baseTrampDynamicInfo->end_ip = baseTramp->baseAddr + baseTramp->size + 0x10;
	baseTrampDynamicInfo->gp = proc->getTOCoffsetInfo( origBundleAddr + baseAddress );
	baseTrampDynamicInfo->format = UNW_INFO_FORMAT_DYNAMIC;

	baseTrampDynamicInfo->u.pi.name_ptr = (unw_word_t) "dynamic instrumentation: base tramp";
	baseTrampDynamicInfo->u.pi.handler = (Address)NULL;
	baseTrampDynamicInfo->u.pi.regions = regionZero;

	/* DEBUG: Walk through the region list and dump insn/op counts. */
	// unw_dyn_region_info_t * region = regionZero;
	// for( int i = 0; region != NULL; i++, region = region->next ) {
		// fprintf( stderr, "%s[%d]: region[%d]: ops: %u, instructions: %d\n", __FILE__, __LINE__, i, region->op_count, region->insn_count );
		// } /* end DEBUG loop */
		
	/* Insert the dynamic unwind information in the remote process and register it. */
	assert( insertAndRegisterDynamicUnwindInformation( baseTrampDynamicInfo, proc ) );
	
	free( regionFourB );
	free( regionFourA );
	free( regionThree );
	free( regionTwoB );
	free( regionTwoA );
	free( regionOne );
	free( regionZero );
	free( baseTrampDynamicInfo );

	// /* DEBUG */ fprintf( stderr, "* Installed base tramp at 0x%lx, from 0x%lx (local 0x%lx)\n", baseTramp->baseAddr, installationPoint, (Address)instructions );
	// /* DEBUG */ fprintf( stderr, "Installed base tramp of size 0x%x (of 0x%x) at 0x%lx.\n", baseTramp->size, MAX_BASE_TRAMP_SIZE, allocatedAddress );
	assert( baseTramp->size < MAX_BASE_TRAMP_SIZE );
	return baseTramp;
	} /* end installBaseTramp() */

/* Expands a single bundle to 10 bundles.  Three for each instruction,
 * (the maximum possible emulated size) and one for the long jump back
 * to continue normal execution.  Aids per-instruction (as opposed to
 * per-bundle) instrumentation.
 */
Address installMultiTramp(instPoint * & location, process *proc)
{
	bool allocErr;
	Address origBundleAddr = location->pointAddr() & ~0xF;
	Address allocatedAddress = proc->inferiorMalloc( sizeof(struct ia64_bundle_t) * 10,
													 anyHeap,
													 NEAR_ADDRESS,
													 &allocErr );
	if( allocErr ) { fprintf( stderr, "Unable to allocate multi tramp, aborting.\n" ); abort(); }
	allocatedAddress -= allocatedAddress % 16;

    // Acquire the base address of the object we're instrumenting.
    Address baseAddress; assert( proc->getBaseAddress( location->getOwner(), baseAddress ) );
    assert( baseAddress % 16 == 0 );

	const ia64_bundle_t *origBundlePtr =
		(const ia64_bundle_t *)location->getOwner()->getPtrToInstruction( origBundleAddr );
	assert( (Address)origBundlePtr % 16 == 0 );
	IA64_bundle bundleToEmulate( *origBundlePtr );
	origBundleAddr += baseAddress;

	unsigned int bundleCount = 0;
	unsigned int size = 0;
	ia64_bundle_t insnBuffer[10];
	IA64_bundle nopBundle( MMI, NOP_M, NOP_M, NOP_I );

	// Prepare multi-tramp slot 0
	emulateBundle( bundleToEmulate, origBundleAddr, insnBuffer,
				   bundleCount, size, allocatedAddress, 0 );
	while( bundleCount < 3 ) {
		insnBuffer[ bundleCount++ ] = nopBundle.getMachineCode(); size += 16;
		}

	// Prepare multi-tramp slot 1
	emulateBundle( bundleToEmulate, origBundleAddr, insnBuffer,
				   bundleCount, size, allocatedAddress, 1 );
	while( bundleCount < 6 ) {
		insnBuffer[ bundleCount++ ] = nopBundle.getMachineCode(); size += 16;
		}

	// Prepare multi-tramp slot 2 (if needed)
	if (!bundleToEmulate.hasLongInstruction())
		emulateBundle( bundleToEmulate, origBundleAddr, insnBuffer,
					   bundleCount, size, allocatedAddress, 2 );
	while( bundleCount < 9 ) {
		insnBuffer[ bundleCount++ ] = nopBundle.getMachineCode(); size += 16;
		}

    IA64_instruction_x returnToOrig = generateLongBranchTo( (origBundleAddr + 0x10) -
															(allocatedAddress + 0x90) );
    IA64_bundle returnBundle( MLXstop, IA64_instruction(NOP_M), returnToOrig );
    insnBuffer[ bundleCount++ ] = returnBundle.getMachineCode(); size += 16;

	// Copy the instructions into mutatee.
    InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( allocatedAddress, proc );
    iAddr.writeBundlesFrom( (unsigned char *)insnBuffer, 10 );

	// Replace original bundle
	IA64_instruction_x jumpToBaseInstruction = generateLongBranchTo( allocatedAddress - origBundleAddr );
	IA64_bundle jumpToBaseBundle( MLXstop, IA64_instruction(NOP_M), jumpToBaseInstruction );
	InsnAddr jAddr = InsnAddr::generateFromAlignedDataAddress( origBundleAddr, proc );
	jAddr.replaceBundleWith( jumpToBaseBundle );
	
	/* Take care of unwind information book-keeping. */
	unw_dyn_info_t * multiTrampDynamicInfo = (unw_dyn_info_t *)calloc( 1, sizeof( unw_dyn_info_t ) );
	assert( multiTrampDynamicInfo != NULL );
	
	multiTrampDynamicInfo->next = NULL;
	multiTrampDynamicInfo->prev = NULL;

	/* This region will simply ALIAS to the instrumentation point. */
	unw_dyn_region_info_t * regionZero = (unw_dyn_region_info_t *)malloc( _U_dyn_region_info_size( 2 ) );
	assert( regionZero != NULL );
	
	regionZero->op_count = 2;
	regionZero->insn_count = 30;
	_U_dyn_op_alias( & regionZero->op[0], _U_QP_TRUE, -1, origBundleAddr );
	_U_dyn_op_stop( & regionZero->op[1] );

	regionZero->next = NULL;
	
	multiTrampDynamicInfo->start_ip = allocatedAddress;
	multiTrampDynamicInfo->end_ip = allocatedAddress + (10 * 0x10) + 0x10;
	multiTrampDynamicInfo->gp = proc->getTOCoffsetInfo( origBundleAddr );
	multiTrampDynamicInfo->format = UNW_INFO_FORMAT_DYNAMIC;	
			
    multiTrampDynamicInfo->u.pi.name_ptr = (unw_word_t) "dynamic instrumentation: multi tramp";
    multiTrampDynamicInfo->u.pi.regions = regionZero;
	
	insertAndRegisterDynamicUnwindInformation( multiTrampDynamicInfo, proc );

	return allocatedAddress;
}

/* Required by inst.C */
trampTemplate * findOrInstallBaseTramp( process * proc, instPoint * & location,
										returnInstance * & retInstance,
										bool trampRecursiveDesired,
										bool /*noCost*/, bool & /*deferred*/, bool /*allowTrap*/ ) {
	/* TODO: handle if trampRecursiveDesired; handle if noCast, handle if deferred. */
 
	/* proc->baseMap is in the relevant variable here; check to see if the given
	   instPoint already has a base tramp ("find"), and if not, "install" one. */
	
	/* "find" */
	if( proc->baseMap.defines( location ) ) { return proc->baseMap[location]; }

	/* Pre-"install" */
	Address multiTrampAddress;
	/* FIXME: should this be pointAddr() + baseAddr? */
	if( proc->multiTrampMap.defines( location->pointAddr() & ~0xF )) {
		multiTrampAddress = proc->multiTrampMap[ location->pointAddr() & ~0xF ];
	} else {
		multiTrampAddress = installMultiTramp(location, proc);
		if (!multiTrampAddress) return NULL;
		proc->multiTrampMap[ location->pointAddr() & ~0xF ] = multiTrampAddress;
	}

	/* Clear the multi-tramp slot (3 bundles) to prepare for basetramp install */
	ia64_bundle_t emptySlot[3];
	IA64_bundle nopBundle( MMI, NOP_M, NOP_M, NOP_I);
	emptySlot[0] = nopBundle.getMachineCode();
	emptySlot[1] = nopBundle.getMachineCode();
	emptySlot[2] = nopBundle.getMachineCode();

	int slotNo = (location->pointAddr() % 16);
	Address installAddr = multiTrampAddress + (slotNo * 0x30);
	InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( installAddr, proc );
	iAddr.writeBundlesFrom( (unsigned char *)emptySlot, 3 );

	/* "install" */
	trampTemplate * installedBaseTramp = installBaseTramp( installAddr, location,
														   proc, trampRecursiveDesired );
	/* FIXME */ if( installedBaseTramp == NULL ) { return NULL; }
	proc->baseMap[location] = installedBaseTramp;

	/* We've already written the return instruction.  No need for
	   addInstFunc() to write it again. */
	retInstance = NULL;

	return installedBaseTramp;
	} /* end findOrInstallBaseTramp() */

void generateMTpreamble(char *, Address &, process *) {
	assert( 0 );	// We don't yet handle multiple threads.
	} /* end generateMTpreamble() */

/* Required by ast.C */
void emitFuncJump(opCode op, char *buf, Address &base, const int_function *callee,
				  process *proc, const instPoint *location, bool) {

	assert(op == funcJumpOp);
	IA64_bundle bundle;

	assert( ((Address)buf) % 16 == 0 );
	ia64_bundle_t *insnPtr = (ia64_bundle_t *)(Address)buf;

	// Remove basetramp's frame on memory stack.
	/* FIXME: we should make use of the unwind_region information here, but this 
	   is the only minitramp which alters the frame. */
	generatePreservationTrailer( insnPtr, base, location->pointFunc()->usedFPregs, NULL );

	int outputRegisters = 8;
	int extraOuts = outputRegisters % 3;
	/* FIXME: the knowledge about NUM_PRESERVED is spread all over creation, and should
	   probably be localized to generatePreservation*(). */
	int offset = 32 + outputRegisters + 5 + NUM_PRESERVED;

	// Generate a new register frame with an output size equal to the
	// original local size.
	// NOTE:	What we really want is the post-call, pre-alloc local size.
	//			Unfortunatly, that is difficult to get, so we'll use the
	//			post-call, post-alloc local instead.  While that may be
	//			larger than is strictly necessary, it should still work
	//			correctly.
	bundle = IA64_bundle( MII,
						  generateAllocInstructionFor( regSpace, 5, outputRegisters, 0 ),
						  (extraOuts >= 1 ? generateRegisterToRegisterMove( 32, offset )
										  : IA64_instruction( NOP_I )),
						  (extraOuts >= 2 ? generateRegisterToRegisterMove( 33, offset + 1 )
										  : IA64_instruction( NOP_I )));
	insnPtr[base/16] = bundle.getMachineCode();
	base += 16;

	// Move local regs into output area.
	for (int i = extraOuts; i < outputRegisters; i += 3) {
		bundle = IA64_bundle( MII,
							  generateRegisterToRegisterMove( 32 + (i+0), offset + (i+0) ),
							  generateRegisterToRegisterMove( 32 + (i+1), offset + (i+1) ),
							  generateRegisterToRegisterMove( 32 + (i+2), offset + (i+2) ));
		insnPtr[base/16] = bundle.getMachineCode();
		base += 16;
	}

	// Save system state into local registers.  AR.PFS saved earlier by alloc.
	bundle = IA64_bundle( MII,
						  generateRegisterToRegisterMove( REGISTER_GP, offset - 4 ),
						  generateBranchToRegisterMove( BRANCH_RETURN, offset - 3 ),
						  generateBranchToRegisterMove( BRANCH_SCRATCH, offset - 2 ));
	insnPtr[base/16] = bundle.getMachineCode();
	base += 16;

	// Ready the callee address.
	Address callee_addr = callee->getEffectiveAddress(proc);
	bundle = IA64_bundle( MLXstop,
						  IA64_instruction( NOP_M ),
						  generateLongConstantInRegister( offset - 1, callee_addr ));
	insnPtr[base/16] = bundle.getMachineCode();
	base += 16;

	// Install GP for callee.
	bundle = IA64_bundle( MLX,
						  IA64_instruction( NOP_M ),
						  generateLongConstantInRegister( REGISTER_GP, proc->getTOCoffsetInfo( callee_addr )));
	insnPtr[base/16] = bundle.getMachineCode();
	base += 16;

	// Copy the callee address and jump.
	bundle = IA64_bundle( MIBstop,
						  IA64_instruction( NOP_M ),
						  generateRegisterToBranchMove( offset - 1, BRANCH_SCRATCH ),
						  generateIndirectCallTo( BRANCH_SCRATCH, BRANCH_RETURN ));
	insnPtr[base/16] = bundle.getMachineCode();
	base += 16;

	// Restore system state.
	bundle = IA64_bundle( MII,
						  generateRegisterToRegisterMove( offset - 4, REGISTER_GP ),
						  generateRegisterToBranchMove( offset - 3, BRANCH_RETURN ),
						  generateRegisterToApplicationMove( 32 + regSpace->originalLocals + regSpace->originalOutputs, AR_PFS ));
	insnPtr[base/16] = bundle.getMachineCode();
	base += 16;

	bundle = IA64_bundle( MII,
						  (extraOuts >= 1 ? generateRegisterToRegisterMove( offset, 32 )
										  : IA64_instruction( NOP_I )),
						  (extraOuts >= 2 ? generateRegisterToRegisterMove( offset + 1, 33 )
										  : IA64_instruction( NOP_I )),
						  generateRegisterToBranchMove( offset - 2, BRANCH_SCRATCH ));
	insnPtr[base/16] = bundle.getMachineCode();
	base += 16;

	// Move output regs back into local area.
	for (int i = extraOuts; i < outputRegisters; i += 3) {
		bundle = IA64_bundle( MII,
							  generateRegisterToRegisterMove( offset + (i+0), 32 + (i+0) ),
							  generateRegisterToRegisterMove( offset + (i+1), 32 + (i+1) ),
							  generateRegisterToRegisterMove( offset + (i+2), 32 + (i+2) ));
		insnPtr[base/16] = bundle.getMachineCode();
		base += 16;
	}

	// Restore original register frame.
	bundle = IA64_bundle( MstopMIstop,
						  IA64_instruction( NOP_M ),
						  generateOriginalAllocFor( regSpace ),
						  IA64_instruction( NOP_M ));
	insnPtr[base/16] = bundle.getMachineCode();
	base += 16;

	// Ignore rest of tramp, and return directly to original caller.
	bundle = IA64_bundle( MMBstop,
						  IA64_instruction( NOP_M ),
						  IA64_instruction( NOP_M ),
						  generateReturnTo( BRANCH_RETURN ));
	insnPtr[base/16] = bundle.getMachineCode();
	base += 16;
}

/* Required by ast.C */
Register emitR( opCode op, Register src1, Register /*src2*/, Register dest,
				char * ibuf, Address & base, bool /*noCost*/,
				const instPoint *location, bool /*for_multithreaded*/) {
	/* FIXME: handle noCost */
	switch( op ) {
		case getParamOp: {
			/* src1 is the (incoming) parameter we want. */
			if( src1 >= 8 ) {
				/* emitR is only called from within generateTramp, which sets the global
				   variable regSpace to the correct value.  Use it with reckless abandon.

				   Also, REGISTER_SP will always be stored in a register, so we are free
				   to use regSpage->storageMap directly (as opposed to going through
				   emitLoadPreviousStackFrameRegister).
				*/
				int spReg = regSpace->storageMap[ BP_GR0 + REGISTER_SP ];
				int memStackOffset = (src1 - 8 + 2) * 8;
				ia64_bundle_t * rawBundlePointer = (ia64_bundle_t *)((Address)ibuf + base);

				IA64_bundle bundle = IA64_bundle( MstopMIstop,
												  generateShortImmediateAdd(dest, memStackOffset, spReg),
												  generateRegisterLoad(dest, dest),
												  IA64_instruction(NOP_M) );
				*rawBundlePointer = bundle.getMachineCode();
				base += 16;
				return dest;

				} else {
				/* Due to register renaming, the requested parameter location depends
				   on the instPoint. */

				int regStackOffset = 0;
				if (location->getPointType() == callSite)
					regStackOffset = regSpace->originalLocals;

				emitRegisterToRegisterCopy( 32 + regStackOffset + src1, dest, ibuf, base, NULL );
				return dest;
				} /* end if it's a parameter in a register. */
			} break;
		
		case getRetValOp: {
			/* WARNING: According to the Itanium Software Conventions
			   Guide, the return value can use multiple registers (r8-11).
			   However, since Dyninst can't handle anything larger than
			   64 bits (for now), we'll just return r8.

			   This should be valid for the general case. */
			   
			Register retVal = (Register)8;
			emitRegisterToRegisterCopy(retVal, dest, ibuf, base, NULL);
			return dest;
			} break;

		default:
			assert( 0 );
			break;
		} /* end switch */
	} /* end emitR() */

/* Required by BPatch_init.C */
void initDefaultPointFrequencyTable() {
	/* On other platforms, this loads data into a table that's only used
	   in the function getPointFrequency, which is never called, so we'll do nothing. */
	} /* end initDefaultPointFrequencyTable() */

#define DEFAULT_MAGIC_FREQUENCY 100.0
float getPointFrequency( instPoint * point ) {
	int_function * func = point->getCallee();
                                                                                                                                   
	if( !func ) { func = point->pointFunc(); }

	#if defined( USE_DEFAULT_FREQUENCY_TABLE )
	if( ! funcFrequencyTable.defines( func->prettyName() ) ) { return DEFAULT_MAGIC_FREQUENCY; }
  	else{ return (float) funcFrequencyTable[ func->prettyName() ]; }
	#else
	return DEFAULT_MAGIC_FREQUENCY;
	#endif /* defined( USE_DEFAULT_FREQUENCY_TABLE ) */
	} /* end getPointFrequency() */

/* Required by inst.C, ast.C */
Address emitA( opCode op, Register src1, Register /*src2*/, Register dest,
			   char * ibuf, Address & base, bool /*noCost*/ ) {  // FIXME: cost?
	/* Emit the given opcode, returning its relative offset.  For
	   multi-bundle opcodes, return the offset of the branch itself;
	   the code generator will insert the fall-through case directly
	   after this address. */
	assert( (((Address)ibuf + base) % 16) == 0 );
	ia64_bundle_t * rawBundlePointer = (ia64_bundle_t *)((Address)ibuf + base);

	switch( op ) {
		case trampTrailer: {
			rawBundlePointer[0] = IA64_bundle( MIIstop, NOP_M, NOP_I, NOP_I ).getMachineCode();

			base += 16;
			return (base - 16);
			break; }

		case ifOp: {
			/* Branch by offset dest if src1 is zero. */

			/* See note in eqOp case of emitV() about predicate registers */
			IA64_instruction compareInsn = generateComparison( eqOp, src1 % 64, src1, REGISTER_ZERO );
			IA64_instruction_x branchInsn = predicateLongInstruction( src1 % 64, generateLongBranchTo( dest ) );
			IA64_bundle compareBundle( MIIstop, compareInsn, NOP_I, NOP_I );
			IA64_instruction memoryNOP( NOP_M );
			IA64_bundle branchBundle( MLXstop, memoryNOP, branchInsn );
			rawBundlePointer[0] = compareBundle.getMachineCode();
			rawBundlePointer[1] = branchBundle.getMachineCode();

			base += 32;
			return (base - 16);
			break; }
			
		case branchOp: {
			/* Why are we passing 64-bit numbers through a Register type? */
			IA64_instruction memoryNOP( NOP_M );
			IA64_instruction_x branchInsn = generateLongBranchTo( dest );
			IA64_bundle branchBundle( MLXstop, memoryNOP, branchInsn );
			rawBundlePointer[0] = branchBundle.getMachineCode();
			
			base += 16;
			return base - 16;
			} break;

		default:
			assert( 0 );
			return 0;
		} /* end op switch */
	} /* end emitA() */

/* Required by ast.C */
bool doNotOverflow( int /*value*/ ) { 
	/* To be on the safe side, we'll say it always overflows,
	   since it's not clear to me which immediate size(s) are going to be used. */
	return false;
	} /* end doNotOverflow() */

/* Required by inst.C; install a single mini-tramp */
void installTramp( miniTrampHandle *mtHandle,
				   process *proc,
				   char *code, int codeSize) {
	/* Book-keeping. */
	totalMiniTramps++; insnGenerated += codeSize / 16;
	
	/* Write the minitramp. */
	// /* DEBUG */ fprintf( stderr, "* Installing minitramp at 0x%lx\n", mtHandle->miniTrampBase );
	proc->writeDataSpace( (caddr_t)mtHandle->miniTrampBase, codeSize, code );
	
	/* Update the dynamic unwind information. */
	unw_dyn_info_t * miniTrampDynamicInfo = (unw_dyn_info_t *)calloc( 1, sizeof( unw_dyn_info_t ) );
	assert( miniTrampDynamicInfo != NULL );
	
	miniTrampDynamicInfo->next = NULL;
	miniTrampDynamicInfo->prev = NULL;
	
	/* The first and only region will simply ALIAS to the jump to it. */
	unw_dyn_region_info_t * regionZero = (unw_dyn_region_info_t *)malloc( _U_dyn_region_info_size( 2 ) );
	assert( regionZero != NULL );
	
	regionZero->insn_count = 0;
	regionZero->op_count = 2;
	
	unw_dyn_region_info_t * regionOne = (unw_dyn_region_info_t *)malloc( _U_dyn_region_info_size( 2 ) );
	assert( regionOne != NULL );

	regionZero->next = regionOne;
	regionOne->next = NULL;
	
	regionOne->insn_count = ( codeSize / 16 ) * 3;
	regionOne->op_count = 1;
	_U_dyn_op_stop( & regionOne->op[ 0 ] );

	/* The pre- and post- jumps to the minitramps have the same frame state. */
	Address jumpToMiniTrampAddress = mtHandle->baseTramp->baseAddr + mtHandle->baseTramp->localPreOffset;
	// /* DEBUG */ fprintf( stderr, "aliasing minitramp to 0x%lx\n", jumpToMiniTrampAddress );
	_U_dyn_op_alias( & regionZero->op[0], _U_QP_TRUE, -1, jumpToMiniTrampAddress );
	_U_dyn_op_stop( & regionZero->op[1] );
	
	/* Acquire the remote address from which one jumps to this minitramp's basetramp. */
	Address installationPoint = mtHandle->baseTramp->location->pointAddr();
	int slotNo = installationPoint % 16;
	installationPoint = installationPoint - slotNo;
	
	Address baseAddress; assert( proc->getBaseAddress( mtHandle->baseTramp->location->getOwner(), baseAddress ) );
	assert( baseAddress % 16 == 0 );
	installationPoint += baseAddress;

	/* Fill out the rest of the book-keeping. */
	miniTrampDynamicInfo->start_ip = mtHandle->miniTrampBase;
	miniTrampDynamicInfo->end_ip = mtHandle->miniTrampBase + codeSize + 0x10;
	miniTrampDynamicInfo->gp = proc->getTOCoffsetInfo( installationPoint );
	miniTrampDynamicInfo->format = UNW_INFO_FORMAT_DYNAMIC;	
			
    miniTrampDynamicInfo->u.pi.name_ptr = (unw_word_t) "dynamic instrumentation: mini tramp";
    miniTrampDynamicInfo->u.pi.regions = regionZero;

	assert( insertAndRegisterDynamicUnwindInformation( miniTrampDynamicInfo, proc ) );

	free( regionZero );
	free( miniTrampDynamicInfo );
    
	/* Make sure that it's not skipped. */
	Address insertNOPAt = 0;
	if( mtHandle->when == callPreInsn && mtHandle->baseTramp->prevInstru == false ) {
		mtHandle->baseTramp->cost += mtHandle->baseTramp->prevBaseCost;
		insertNOPAt = mtHandle->baseTramp->baseAddr + mtHandle->baseTramp->skipPreInsOffset;
		mtHandle->baseTramp->prevInstru = true;
		} 
	else if( mtHandle->when == callPostInsn && mtHandle->baseTramp->postInstru == false ) {
		mtHandle->baseTramp->cost += mtHandle->baseTramp->postBaseCost;
		insertNOPAt = mtHandle->baseTramp->baseAddr + mtHandle->baseTramp->skipPostInsOffset;
		mtHandle->baseTramp->postInstru = true;
		}
	
	/* Only write the NOP if appropriate. */
	if( insertNOPAt != 0 ) {
		InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( insertNOPAt, proc );
		iAddr.replaceBundleWith( IA64_bundle( MIIstop, NOP_M, NOP_I, NOP_I ) );
		}
} /* end installTramp() */

/* Required by ast.C */
void emitVstore( opCode op, Register src1, Register src2, Address dest,
				 char * ibuf, Address & base, bool /*noCost*/, int size,
				 const instPoint * location, process * proc, registerSpace * rs ) {
	assert( (((Address)ibuf + base) % 16) == 0 );
	ia64_bundle_t * rawBundlePointer = (ia64_bundle_t *)((Address)ibuf + base);

	switch( op ) {
		case storeOp: {
			IA64_instruction memoryNOP( NOP_M );
			IA64_instruction_x loadAddressInsn = generateLongConstantInRegister( src2, dest );
			IA64_bundle loadBundle( MLXstop, memoryNOP, loadAddressInsn );
			rawBundlePointer[0] = loadBundle.getMachineCode();
			base += 16;

			IA64_instruction storeInsn = generateRegisterStore( src2, src1, size );
			IA64_bundle storeBundle( MIIstop, storeInsn, NOP_I, NOP_I );
			rawBundlePointer[1] = storeBundle.getMachineCode();
			base += 16;
			} break;

		case storeFrameRelativeOp: {
			/* Store size bytes at the address fp + the immediate dest from the register src1,
			   using the scratch register src2. */
			assert( location->pointFunc()->framePointerCalculator != NULL );			

			/* framePointerCalculator will leave the frame pointer in framePointer. */
			pdvector< AstNode * > ifForks;
			Register framePointer = (Register) location->pointFunc()->framePointerCalculator->generateCode_phase2( proc, rs, ibuf, base, false, ifForks, location );
			/* For now, make sure what we don't know can't hurt us. */
			assert( ifForks.size() == 0 );

			/* Recalculate this because of the generateCode_phase2() call. */
			rawBundlePointer = (ia64_bundle_t *)((Address)ibuf + base);
			
			/* See the frameAddr case in emitVload() for an optimization. */
			IA64_instruction memoryNop( NOP_M );
			IA64_instruction_x loadOffset = generateLongConstantInRegister( src2, dest );
			IA64_bundle loadOffsetBundle( MLXstop, memoryNop, loadOffset );
			IA64_instruction computeAddress = generateArithmetic( plusOp, src2, src2, framePointer );
			IA64_instruction storeToAddress = generateRegisterStore( src2, src1, size );
			IA64_instruction integerNop( NOP_I );
			IA64_bundle computeAndStoreBundle( MstopMIstop, computeAddress, storeToAddress, integerNop );
			rawBundlePointer[0] = loadOffsetBundle.getMachineCode();
			rawBundlePointer[1] = computeAndStoreBundle.getMachineCode();
			base += 32;
			
			rs->freeRegister( framePointer );
			} break;

		default:
			fprintf( stderr, "emitVstore(): unexpected opcode, aborting.\n" );
			abort();
			break;
		} /* end switch */
	} /* end emitVstore() */

/* Required by ast.C */
void emitJmpMC( int /*condition*/, int /*offset*/, char * /*baseInsn*/, Address & /*base*/ ) { assert( 0 ); }

/* Required by inst.C */
void generateNoOp( process * proc, Address addr ) { 
	InsnAddr iAddr = InsnAddr::generateFromAlignedDataAddress( addr, proc );
	iAddr.replaceBundleWith( IA64_bundle( MIIstop, NOP_M, NOP_I, NOP_I ) );
	} /* end generateNoOp */

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
		// currentBundle = IA64_bundle( *(ia64_bundle_t*)encodedAddress );
		new( & currentBundle ) IA64_bundle( *(ia64_bundle_t*)encodedAddress );
		} else {
		encodedAddress++;
		}

	/* Return the proper value. */
	return rv;
	} /* end operator ++ */

/* Implementation of instPoint */
instPoint::instPoint( Address encodedAddress, int_function * pdfn, IA64_instruction * theInsn, instPointType type )  : instPointBase(type, encodedAddress, pdfn), myInstruction( theInsn ) {
	/* Does not account for base addresses. */
	myTargetAddress = myInstruction->getTargetAddress() + pointAddr();
	/* 16-byte align the target to compensate for pointAddr() being encoded. */
	myTargetAddress = myTargetAddress - (myTargetAddress % 16);
	} /* end instPoint constructor */

/* The IA-64 instrumentation code always displaces a single three-instruction bundle.
   We'll return the machine code, since that's easier and the type of insns isn't specificed. */ 
int BPatch_point::getDisplacedInstructionsInt( int maxSize, void * insns ) {
	/* If there's not enough room, don't do anything. */
	if( ((unsigned)maxSize) < sizeof( ia64_bundle_t ) ) { return 0; }

	/* Acquire the machine-code bundle. */
	Address mutatorAddress = (Address) point->getOwner()->getPtrToInstruction( point->pointAddr() ) ;
	Address alignedBundleAddress = mutatorAddress - (mutatorAddress % 0x10 );

	/* Copy things over. */
	* ( ia64_bundle_t *) insns = * (ia64_bundle_t *) alignedBundleAddress;

	return sizeof( ia64_bundle_t );
	} /* end getDisplacedInstructions() */

//XXX loop port
BPatch_point *
createInstructionEdgeInstPoint(process* proc, 
			       int_function *func, 
			       BPatch_edge *edge)
{
    return NULL;
}

//XXX loop port
void 
createEdgeTramp(process *proc, image *img, BPatch_edge *edge)
{

}


bool registerSpace::clobberRegister(Register reg) 
{
  return false;
}

unsigned saveGPRegister(char *baseInsn, Address &base, Register reg)
{
}

#if ! defined( BPATCH_LIBRARY )

bool returnInstance::checkReturnInstance( const pdvector< pdvector<Frame> > &stackWalks ) {
	/* Looted from inst-x86.C */
	for( unsigned walk_iter = 0; walk_iter < stackWalks.size(); walk_iter++ ) {
		for( u_int i=0; i < stackWalks[ walk_iter ].size(); i++ ) {
			/* 27FEB03: we no longer return true if we are at the
			   exact same address as the return instance. In this case
			   writing a jump is safe. -- bernat */
			if(	(stackWalks[walk_iter][i].getPC() > addr_) &&
				(stackWalks[walk_iter][i].getPC() < addr_+size_) ) {
				bperr( "PC at 0x%lx (thread %d, frame %d) conflicts with inst point 0x%lx\n", stackWalks[walk_iter][i].getPC(), walk_iter, i, addr_);
				return false;
				}
			}
		}
	return true;
	} /* end checkReturnInstance() */

#define BASE_TRAMP_MAGIC_COST 1000 /* in 'cycles' */ 
int getPointCost( process * proc, const instPoint * point ) {
	if( proc->baseMap.defines( point ) ) {
   		return 0;
   		}
   	else {
   		return BASE_TRAMP_MAGIC_COST;
   		}
   	} /* end getPointCost() */

#endif /* ! defined( BPATCH_LIBRARY ) */

bool process::isDynamicCallSite( instPoint * callSite ) {
	switch( callSite->iPgetInstruction()->getType() ) {
		case IA64_instruction::INDIRECT_CALL: return true;
		default: return false;
		}
	} /* end isDynamicCallSite() */

#if defined( OLD_DYNAMIC_CALLSITE_MONITORING )
bool process::MonitorCallSite( instPoint * callSite ) {
	pdvector< AstNode * > arguments;
#else
/* This is a really horribly-named function. */
bool process::getDynamicCallSiteArgs( instPoint * callSite, pdvector<AstNode *> & arguments ) {
#endif
	insn_tmpl tmpl = { callSite->iPgetInstruction()->getMachineCode() };
	uint64_t targetAddrRegister = tmpl.B4.b2;			
	// /* DEBUG */ fprintf( stderr, "%s[%d]: monitoring call site for branch register %d\n", __FILE__, __LINE__, targetAddrRegister );
	
	/* This should be the only place on the IA-64 using this poorly-named constant. */
	AstNode * target = new AstNode( AstNode::PreviousStackFrameDataReg, BP_BR0 + targetAddrRegister );
	assert( target != NULL );
	arguments[0] = target;
	
	AstNode * source = new AstNode( AstNode::Constant, callSite->pointAddr() );
	assert( source != NULL );
	arguments[1] = source;

#if defined( OLD_DYNAMIC_CALLSITE_MONITORING )
	AstNode * callToMonitor = new AstNode( "DYNINSTRegisterCallee", arguments );
	assert( callToMonitor != NULL );
	
	miniTrampHandle * mtHandle = NULL;
	addInstFunc(	this, mtHandle, callSite, callToMonitor,
					callPreInsn, orderFirstAtPoint, true, false, true );
#endif
						
	return true;
	} /* end MonitorCallSite() */
