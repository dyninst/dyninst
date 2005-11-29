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
#include "dyninstAPI/src/debuggerinterface.h"

#include "dyninstAPI/src/arch-ia64.h"
#include "dyninstAPI/src/inst-ia64.h"
#include "dyninstAPI/src/instPoint.h"	// includes instPoint-ia64.h
#include "dyninstAPI/src/instP.h"		// class returnInstance
#include "dyninstAPI/src/InstrucIter.h"
#include "dyninstAPI/src/multiTramp.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/miniTramp.h"

#include "dyninstAPI/src/rpcMgr.h"

#include <sys/ptrace.h>
#include <asm/ptrace_offsets.h>

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
  instruction memoryNOP( NOP_M );
  instruction_x longJumpInsn = generateLongBranchTo( target + offsetOfJumpToEmulation + 16 );
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
void emitCSload( const BPatch_addrSpec_NP *as, Register dest, codeGen &gen, bool noCost ) {
  // /* DEBUG */ fprintf( stderr, "emitCSload: as.imm = %d, as.r1 = %d, as.r2 = %d\n", as.getImm(), as.getReg( 0 ), as.getReg( 1 ) );
	
  /* emitCSload()'s first argument should be a BPatch_countSpec_NP.  It's supposed
	 to set dest to the number calculated by the countSpec, which means that it'll
	 be filled with bogus numnbers.  Don't complicate emitASload() with the bogosity. */
  assert( as->getReg( 0 ) == -1 && as->getReg( 1 ) == -1 );
  emitVload( loadConstOp, as->getImm(), dest, dest, gen, noCost );	
} /* end emitCSload() */

#define SHORT_IMMEDIATE_MIN 	-8191
#define SHORT_IMMEDIATE_MAX		8191

/* Required by ast.C */
void emitVload( opCode op, Address src1, Register src2, Register dest,
				codeGen &gen, bool noCost, registerSpace *rs, 
				int size, const instPoint * location, 
				process * proc) {
  switch( op ) {
  case loadOp: {
	/* Stick the address src1 in the temporary register src2. */
	emitVload( loadConstOp, src1, 0, src2, gen, noCost, NULL, size);

	/* Load into destination register dest from [src2]. */
	instruction loadInsn = generateRegisterLoad( dest, src2, size );
	IA64_bundle loadBundle( MIIstop, loadInsn, NOP_I, NOP_I );
	loadBundle.generate( gen );
  } break;

  case loadConstOp: {
	// Optimization: if |immediate| <= 22 bits, use generateShortImmediateAdd(), instead.
	instruction_x lCO = generateLongConstantInRegister( dest, src1 );
	generateBundleFromLongInstruction( lCO ).generate( gen );
  } break;

  case loadFrameRelativeOp: {
	/* Load size bytes from the address fp + src1 into the register dest. */
	assert( location->func()->framePointerCalculator != NULL );			
			
	/* framePointerCalculator will leave the frame pointer in framePointer. */
	Register framePointer = (Register) location->func()->framePointerCalculator->generateCode( proc, rs, gen, false, true, location );
			
	instruction_x longConstant = generateLongConstantInRegister( src2, src1 );
	instruction calculateAddress = generateArithmetic( plusOp, framePointer, src2, framePointer ); 
	instruction loadFromAddress = generateRegisterLoad( dest, framePointer, size );
	IA64_bundle prepareOffsetBundle( MLXstop, NOP_M, longConstant );
	IA64_bundle calculateAndLoadBundle( MstopMIstop, calculateAddress, loadFromAddress, NOP_I );
	prepareOffsetBundle.generate( gen );
	calculateAndLoadBundle.generate( gen );

	rs->freeRegister( framePointer );
  } break;

  case loadRegRelativeOp: {
	/* Similar to loadFrameRelativeOp, except any general register may be used.
	 *
	 * src1 = offset
	 * src2 = base register
	 */
	Register trueBase = rs->allocateRegister( gen, noCost );

	emitLoadPreviousStackFrameRegister( BP_GR0 + src2, trueBase, gen, size, noCost );
	instruction_x longConstant = generateLongConstantInRegister( dest, src1 );
	instruction calculateAddress = generateArithmetic( plusOp, dest, dest, trueBase );
	instruction loadFromAddress = generateRegisterLoad( dest, dest, size );

	IA64_bundle prepareOffset( MLXstop, NOP_M, longConstant );
	IA64_bundle calculateAndLoadBundle( MstopMIstop, calculateAddress, loadFromAddress, NOP_I );

	prepareOffset.generate(gen);
	calculateAndLoadBundle.generate(gen);

	rs->freeRegister( trueBase );
  } break;

  case loadRegRelativeAddr: {
	/* Similar to loadFrameAddr, except any general register may be used.
	 *
	 * src1 = offset
	 * src2 = base register
	 */
	Register trueBase = rs->allocateRegister( gen, noCost );

	emitLoadPreviousStackFrameRegister( BP_GR0 + src2, trueBase, gen, size, noCost );

	instruction_x longConstant = generateLongConstantInRegister( dest, src1 );
	instruction calculateAddress = generateArithmetic( plusOp, dest, dest, trueBase );
	IA64_bundle prepareOffset( MLXstop, NOP_M, longConstant );
	IA64_bundle calculateBundle( MIIstop, calculateAddress, NOP_I, NOP_I );

	prepareOffset.generate( gen );
	calculateBundle.generate( gen );

	rs->freeRegister( trueBase );
  } break;

  case loadFrameAddr: {
	/* Write the value of the fp + the immediate src1 into the register dest. */
	assert( location->func()->framePointerCalculator != NULL );

	/* framePointerCalculator will leave the frame pointer in framePointer. */
	Register framePointer = (Register) location->func()->framePointerCalculator->generateCode( proc, rs, gen, false, true, location );
			
	instruction memoryNop( NOP_M );
	instruction integerNop( NOP_I );

	if( SHORT_IMMEDIATE_MIN < ((int)src1) && ((int)src1) < SHORT_IMMEDIATE_MAX ) {
	  instruction sumFrameAndOffset = generateShortImmediateAdd( dest, src1, framePointer );
	  IA64_bundle resultBundle( MIIstop, memoryNop, integerNop, sumFrameAndOffset );
	  resultBundle.generate( gen );
	}
	else {
	  instruction_x longConstant = generateLongConstantInRegister( dest, src1 );
	  IA64_bundle setLCBundle( MLXstop, memoryNop, longConstant );

	  instruction addLongConstantToFP = generateArithmetic( plusOp, dest, dest, framePointer );
	  IA64_bundle resultBundle( MIIstop, memoryNop, integerNop, addLongConstantToFP );

	  setLCBundle.generate( gen );
	  resultBundle.generate( gen );
	}

	rs->freeRegister( framePointer );						
  } break;

  default:
	fprintf( stderr, "emitVload(): op not a load, aborting.\n" );
	abort();
	break;
  } /* end switch */
} /* end emitVload() */

/* private function, used by emitFuncCall() */
void emitRegisterToRegisterCopy( Register source, Register destination, codeGen & gen, registerSpace * ) {
  instruction moveInsn( generateRegisterToRegisterMove( source, destination ) );
  instruction memoryNOP( NOP_M );
  IA64_bundle r2rBundle( MMIstop, memoryNOP, memoryNOP, moveInsn );
	
  r2rBundle.generate(gen);
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
Register emitFuncCall( opCode op, registerSpace * rs, codeGen & gen,
					   pdvector< AstNode * > & operands,
					   process * proc, bool /* noCost */,
					   Address callee_addr, 
					   const pdvector< AstNode * > & ifForks,
					   const instPoint *location) { 
  /* Consistency check. */
  assert( op == callOp );

  /* Sanity check for non-NULL address argument */
  if( ! callee_addr ) {
	char msg[256];
	snprintf( msg, 255, "%s[%d]: internal error:  emitFuncCall called without callee_addr argument", __FILE__, __LINE__ );
	showErrorCallback( 80, msg );
	assert(0);
  }
 
  Address calleeGP = proc->getTOCoffsetInfo( callee_addr );

  /* Generate the code for the arguments. */
  pdvector< Register > sourceRegisters;
  for( unsigned int i = 0; i < operands.size(); i++ ) {
	sourceRegisters.push_back( operands[i]->generateCode_phase2( proc, rs, gen, false, ifForks, location ) );
  }

  /* source-to-output register copy */
  unsigned int outputZero = rs->getRegSlot( NUM_LOCALS )->number;
  for( unsigned int i = 0; i < sourceRegisters.size(); i++ ) {
	// fprintf( stderr, "Copying argument in local register %d to output register %d\n", sourceRegisters[i], outputZero + i );
	emitRegisterToRegisterCopy( sourceRegisters[i], outputZero + i, gen, rs );
	rs->freeRegister( sourceRegisters[i] );
	rs->incRefCount( outputZero + i );
  } /* end source-to-output register copy */

	/* Since the BT was kind enough to save the GP, return value,
	   return pointer, ar.pfs, and both scratch registers for us,
	   we don't bother.  Just set the callee's GP, copy the call
	   target into a scratch branch register, and indirect to it. */
  emitVload( loadConstOp, calleeGP, 0, REGISTER_GP, gen, false /* ? */, 0 );

  /* Grab a register -- temporary for transfer to branch register,
	 and a registerSpace register for the return value. */
  Register rsRegister = findFreeLocal( rs, "Unable to find local register in which to store callee address, aborting.\n" );
  rs->incRefCount( rsRegister );

  instruction integerNOP( NOP_I );
  instruction memoryNOP( NOP_M );

  // fprintf( stderr, "Loading function address into register %d\n", rsRegister );
  instruction_x loadCalleeInsn = generateLongConstantInRegister( rsRegister, callee_addr );
  IA64_bundle loadCalleeBundle( MLXstop, memoryNOP, loadCalleeInsn );
	
  loadCalleeBundle.generate(gen);

  // fprintf( stderr, "Copying computed branch in general register %d to branch register %d.\n", rsRegister, BRANCH_SCRATCH );
  instruction setBranchInsn = generateRegisterToBranchMove( rsRegister, BRANCH_SCRATCH );
  IA64_bundle setBranchBundle( MIIstop, memoryNOP, integerNOP, setBranchInsn );

  setBranchBundle.generate(gen);

  // fprintf( stderr, "Calling function with offset in branch register %d, return pointer in %d\n", BRANCH_SCRATCH, BRANCH_RETURN );
  instruction indirectBranchInsn = generateIndirectCallTo( BRANCH_SCRATCH, BRANCH_RETURN );
  IA64_bundle indirectBranchBundle( MMBstop, memoryNOP, memoryNOP, indirectBranchInsn );

  indirectBranchBundle.generate(gen);

  /* According to the software conventions, the stack pointer must have sixteen bytes
	 of scratch space available directly above it.  Give it sixteen bytes nobody's using
	 to make sure we don't smash the stack.  This, however, is done in the preservation header. */
  
  for( unsigned int i = 0; i < sourceRegisters.size(); i++ ) {
	/* Free all the output registers. */
	rs->freeRegister( outputZero + i );
  }

  /* copy the result (r8) to a registerSpace register */
  emitRegisterToRegisterCopy( REGISTER_RV, rsRegister, gen, rs );

  /* return that register (still allocated) */
  return rsRegister;
} /* end emitFuncCall() */

/* Required by ast.C; used for memory instrumentation. */
void emitASload( const BPatch_addrSpec_NP *as, Register dest, codeGen &gen,  bool /*noCost*/ ) {
  /* Convert an addrSpec into a value in the destination register. */
  // /* DEBUG */ fprintf( stderr, "emitASload: as.imm = %d, as.r1 = %d, as.r2 = %d\n", as.getImm(), as.getReg( 0 ), as.getReg( 1 ) );
  emitLoadPreviousStackFrameRegister( as->getReg( 0 ), dest, gen, 0, 0 );
} /* end emitASload() */

/* Required by ast.C */
Register deadRegisterList[] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF };
unsigned int deadRegisterListLength = sizeof( deadRegisterList ) / sizeof( Register );

void initTramps( bool /* is_multithreaded */ ) { 
  /* Initialize the registerSpace pointer regSpace to the state of the
	 registers that will exist at the end of the first part of the base
	 tramp's code.  (That is, for the minitramps.)

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
			codeGen &gen,  bool noCost, registerSpace *rs, int size, 
			const instPoint * location, process * proc) {
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
	instruction comparisonInsn = generateComparison( op, dest % 64, src1, src2 );
	instruction setTrue = predicateInstruction( dest % 64, generateShortImmediateAdd( dest, 1, REGISTER_ZERO ) );
	instruction setFalse = predicateInstruction( (dest + 1) % 64, generateShortImmediateAdd( dest, 0, REGISTER_ZERO ));
	IA64_bundle comparisonBundle( MstopMIstop, comparisonInsn, setTrue, setFalse );
	comparisonBundle.generate(gen);
	break; }

  case plusOp:
  case minusOp:
  case andOp:
  case orOp: {
	instruction arithmeticInsn = generateArithmetic( op, dest, src1, src2 );
	instruction integerNOP( NOP_I );
	IA64_bundle arithmeticBundle( MIIstop, arithmeticInsn, integerNOP, integerNOP );
	arithmeticBundle.generate(gen);
	break; }

  case timesOp: {
	/* Only the floating-point unit has a fixed-point multiplier. */
	/* The stack already has the extra space that we need, thanks to the preservation
	   header, so use (arbitrarily) r3 to generate the pointers. */
	instruction spillFP2 = generateFPSpillTo( REGISTER_SP, 2 );
	instruction secondSpillAddr = generateShortImmediateAdd( 3, +16, REGISTER_SP );
	instruction integerNOP( NOP_I );
	instruction spillFP3 = generateFPSpillTo( 3, 3 );
	IA64_bundle spillFP2Bundle( MMIstop, spillFP2, secondSpillAddr, integerNOP );
	IA64_bundle spillFP3Bundle( MIIstop, spillFP3, integerNOP, integerNOP );
	spillFP2Bundle.generate(gen);
	spillFP3Bundle.generate(gen);
			
	/* Do the multiplication. */
	instruction copySrc1 = generateRegisterToFloatMove( src1, 2 );
	instruction copySrc2 = generateRegisterToFloatMove( src2, 3 );
	instruction fixedMultiply = generateFixedPointMultiply( 2, 2, 3 );
	instruction copyDest = generateFloatToRegisterMove( 2, dest );
	IA64_bundle copySources( MMIstop, copySrc1, copySrc2, integerNOP );
	instruction memoryNOP( NOP_M );
	IA64_bundle doMultiplication( MFIstop, memoryNOP, fixedMultiply, integerNOP );
	IA64_bundle copyToDestination( MMIstop, copyDest, memoryNOP, integerNOP );
	copySources.generate(gen);
	doMultiplication.generate(gen);
	copyToDestination.generate(gen);

	/* Restore the FP registers, SP. */
	instruction fillFP2 = generateFPFillFrom( REGISTER_SP, 2 );
	instruction fillFP3 = generateFPFillFrom( 3, 3 );
	IA64_bundle fillF2Bundle( MIIstop, fillFP2, integerNOP, integerNOP );
	IA64_bundle fillF3Bundle( MIIstop, fillFP3, integerNOP, integerNOP );
	fillF2Bundle.generate(gen);
	fillF3Bundle.generate(gen);
			
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
	int_function *calleefunc = proc->findOnlyOneFunction(callee);
	if (!calleefunc) {
	  char msg[256];
	  sprintf(msg, "%s[%d]:  internal error:  unable to find %s",
			  __FILE__, __LINE__, callee.c_str());
	  showErrorCallback(100, msg);
	  assert(0);  // can probably be more graceful
	}
	Address callee_addr = calleefunc->getAddress();

	/* Might want to use funcCall AstNode here */

	/* Call emitFuncCall(). */
	rs->incRefCount( src1 ); rs->incRefCount( src2 );
	Register returnRegister = emitFuncCall( op, rs, gen, operands, proc, noCost, callee_addr, ifForks, location );
	/* A function call does not require any conditional branches, so ensure
	   that we're not screwing up the current AST by not telling it about stuff. */
	assert( ifForks.size() == 0 );
			
	if( returnRegister != dest ) {
	  emitRegisterToRegisterCopy( returnRegister, dest, gen, rs );
	  rs->freeRegister( returnRegister );
	}
	break; }

  case noOp: {
	IA64_bundle nopBundle( MIIstop, NOP_M, NOP_I, NOP_I );
	nopBundle.generate(gen);
	break; }

  case storeIndirOp: {
	/* Store 'size' bytes out of register src1 into the address in dest. */
	instruction storeInstruction = generateRegisterStore( dest, src1, size );
	IA64_bundle storeBundle( MMIstop, storeInstruction, NOP_M, NOP_I );
	storeBundle.generate(gen);
	break; }
			
  case loadIndirOp: {
	/* Load 'size' bytes from the address in register src1 to the register dest. */
	instruction loadInstruction = generateRegisterLoad( dest, src1, size );
	IA64_bundle loadBundle( MMIstop, loadInstruction, NOP_M, NOP_I );
	loadBundle.generate(gen);
	break; }

  default:
	fprintf( stderr, "emitV(): unrecognized op code %d\n", op );
	assert( 0 );
	break;
  } /* end op switch */
} /* end emitV() */


/* Required by ast.C */
void emitImm( opCode op, Register src1, RegValue src2imm, Register dest,
			  codeGen & gen,  bool /* noCost */, registerSpace * ) {
  switch( op ) {
  case orOp: {
	/* Apparently, the bit-wise operation. */
	if( src2imm == 0 ) {
	  emitRegisterToRegisterCopy( src1, dest, gen, NULL );
	} else {
	  abort();
	}
  } break;
		
  default:
	assert( 0 );
	break;
  } /* end op switch */
} /* end emitImm() */

#define STATE_SAVE_COST     48
#define STATE_RESTORE_COST  48

/* Required by ast.C */
int getInsnCost( opCode op ) {
  /* We'll measure cost mostly in instructions. */
  int cost = 3;

  switch( op ) {
  case saveRegOp:
	/* DEBUG */ fprintf( stderr, "%s[%d]: do I /implement/ saveRegOp?\n", __FILE__, __LINE__ );
	cost = 3;
	break;

  case getSysRetValOp:
	/* DEBUG */ fprintf( stderr, "%s[%d]: do I /implement/ getSysRetValOp?\n", __FILE__, __LINE__ );
	cost = 3;
	break;

  case whileOp:
	/* DEBUG */ fprintf( stderr, "%s[%d]: do I /implement/ whileOp?\n", __FILE__, __LINE__ );
	cost = 3;
	break;

  case doOp:
	/* DEBUG */ fprintf( stderr, "%s[%d]: do I /implement/ doOp?\n", __FILE__, __LINE__ );
	cost = 3;
	break;

  case getSysParamOp:
	/* DEBUG */ fprintf( stderr, "%s[%d]: do I /implement/ getSysParamOp?\n", __FILE__, __LINE__ );
	cost = 3;
	break;

  case getAddrOp:
	/* Turns into loadConstOp, loadFrameAddr, or loadRegRelativeAddr. */
	cost = 6;
	break;

  case noOp:

  case plusOp:
  case minusOp:

  case andOp:
  case orOp:

  case eqOp:
  case neOp:
  case lessOp:
  case greaterOp:
  case leOp:
  case geOp:

  case getParamOp:
  case getRetValOp:

  case loadOp:
  case loadIndirOp:
  case loadConstOp:
  case storeIndirOp:
	cost = 3;
	break;

  case storeOp:
  case storeFrameRelativeOp:
  case loadFrameAddr:
  case loadFrameRelativeOp:

  case ifOp:
  case branchOp:
	cost = 6;
	break;

  case timesOp:
	cost = 21;
	break;

  case divOp:
	/* Incurs the cost of a function call. */
	cost = 30;
  case callOp:
	/* Assuming this doesn't include cost of argument construction
	   or actual delay time, we use (about) thirteen bundles. */
	cost += 39;
	break;

  case updateCostOp:
	cost = 6;
	break;

  case trampPreamble:
	cost = STATE_SAVE_COST;
	break;

  case funcJumpOp:
	/* Mostly the cost of the trampTrailer. */
	cost = 6;
  case trampTrailer:
	cost += STATE_RESTORE_COST;
	break;

  case ifMCOp:
	/* for emitJmpMC(), which isn't implemented. */
	break;

  default:
	fprintf( stderr, "%s[%d]: unrecognized opCode %d\n", __FILE__, __LINE__, op );
	break;
  } /* end op switch */

  return cost;	
} /* end getInsnCost() */

/* private refactoring function */
IA64_bundle generateUnknownRegisterTypeMoveBundle( Address src, Register dest ) {
  if( /* (BP_GR0 <= src) && */ (src <= BP_GR127) ) {
	instruction moveInsn = generateRegisterToRegisterMove( src, dest );
	return IA64_bundle( MIIstop, NOP_M, NOP_I, moveInsn );
  }
  else if( (BP_BR0 <= src) && (src <= BP_BR7) ) {
	instruction moveInsn = generateBranchToRegisterMove( src, dest );
	return IA64_bundle( MIIstop, NOP_M, NOP_I, moveInsn );
  }
  else if( (BP_AR0 <= src) && (src <= BP_AR63) ) {
	/* M-unit */
	instruction moveInsn = generateApplicationToRegisterMove( src, dest );
	return IA64_bundle( MIIstop, moveInsn, NOP_I, NOP_I );
  }
  else if( (BP_AR64 <= src) && (src <= BP_AR127) ) {
	/* I-unit */
	instruction moveInsn = generateApplicationToRegisterMove( src, dest );
	return IA64_bundle( MIIstop, NOP_M, NOP_I, moveInsn );
  }		
  else if( BP_PR == src ) {
	instruction moveInsn = generatePredicatesToRegisterMove( dest );
	return IA64_bundle( MIIstop, NOP_M, NOP_I, moveInsn );
  }
  else {
	fprintf( stderr, "[%s][%d]: illegal register number.\n", __FILE__, __LINE__ );
	assert( 0 );
  }
} /* end generateUnknownRegisterTypeMoveBundle() */

/* Required by ast.C */
void emitLoadPreviousStackFrameRegister( Address register_num, Register dest, 
										 codeGen &gen,
										 int /*size*/, bool /*noCost*/ )
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
  bundle.generate(gen);
}

/* in arch-ia64.C */
extern instruction::unitType INSTRUCTION_TYPE_ARRAY[(0x20 + 1) * 3];

/* private refactoring function */
IA64_bundle generateBundleFor( instruction insnToBundle ) {
  /* The instruction to bundle is always short; the long instruction bundling is trivial. */
  instruction::unitType iUnitType = INSTRUCTION_TYPE_ARRAY[ (insnToBundle.getTemplateID() * 3) + insnToBundle.getSlotNumber() ];

  instruction integerNOP( NOP_I );
  instruction memoryNOP( NOP_M );
	
  switch( iUnitType ) {
  case instruction::M:
	return IA64_bundle( MIIstop, insnToBundle, integerNOP, integerNOP );
	break;
  case instruction::I:
	return IA64_bundle( MIIstop, memoryNOP, insnToBundle, integerNOP );
	break;
  case instruction::B:
	return IA64_bundle( MMBstop, memoryNOP, memoryNOP, insnToBundle );
	break;
  case instruction::F:
	return IA64_bundle( MFIstop, memoryNOP, insnToBundle, integerNOP );
	break;

  case instruction::L:
  case instruction::X:
  case instruction::RESERVED:
  default:
	fprintf( stderr, "Invalid instruction type in generateBundleFor(), aborting.\n" );
	abort();
	break;
  } /* end iUnitType switch */
} /* end generateBundleFor() */

/* private refactoring function */
bool instruction_x::generate(codeGen &gen, 
							 process *,
							 Address originalLocation,
							 Address allocatedAddress,
							 Address,
							 Address fallthroughTarget) {
  if (fallthroughTarget) {
	fprintf(stderr, "WARNING: ignoring redirected branch target of 0x%lx!\n",
			fallthroughTarget);
  }

  instruction::insnType instructionType = getType();
  insn_tmpl tmpl = { getMachineCode().high };
  insn_tmpl imm  = { getMachineCode().low };
  int64_t immediate;
	
  /* originalLocation is NOT an encoded address */
  switch( instructionType ) {
  case instruction::DIRECT_BRANCH: immediate = GET_X3_TARGET( &tmpl, &imm ); break;
  case instruction::DIRECT_CALL:   immediate = GET_X4_TARGET( &tmpl, &imm ); break;
  default: {
	instruction memoryNOP( NOP_M );
	IA64_bundle( MLXstop,
				 memoryNOP,
				 *this ).generate(gen);
	return true;
  }
  }
	
  /* Correct the immediate by the difference between originalLocation and (insnPtr + size). */
  /* originalLocation is NOT an encoded address */
  Address target = immediate + originalLocation;
  Address source = allocatedAddress + gen.used();
  int64_t displacement = target - source;
	
  switch( instructionType ) {
  case instruction::DIRECT_BRANCH: SET_X3_TARGET( &tmpl, &imm, displacement); break;
  case instruction::DIRECT_CALL:   SET_X4_TARGET( &tmpl, &imm, displacement); break;
  default: break;
  }
	
  instruction_x alteredLong( imm.raw, tmpl.raw, getTemplateID() );
  IA64_bundle( MLXstop,
			   instruction( NOP_M ),
			   alteredLong ).generate(gen);

  return true;
} /* end emulateLongInstruction */

/* private refactoring function */
void rewriteShortOffset( instruction insnToRewrite, Address originalLocation,
						 codeGen &gen, Address allocatedAddress ) {
  /* We insert a short jump past a long jump to the original target, followed
	 by the instruction rewritten to branch one bundle backwards.
	 It's not very elegant, but it's very straightfoward to implement. :) */
	   
  /* CHECKME: do we need to worry about (dynamic) unwind information for any
	 of the instruction we'll rewrite here?  We special-cased direct calls below. */

  /* Skip the long branch. */
  instruction memoryNOP( NOP_M );
  instruction_x skipInsn = generateLongBranchTo( 32 ); // could be short
  IA64_bundle skipInsnBundle( MLXstop, memoryNOP, skipInsn );
  skipInsnBundle.generate(gen); 

  /* Extract the original target. */
  insn_tmpl tmpl = { insnToRewrite.getMachineCode() };
  bool isSpecCheck = ( insnToRewrite.getType() == instruction::SPEC_CHECK );
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
  // Align originalTarget
  originalTarget -= originalTarget % 16;
  // TODO: handle this better. AllocatedAddress is as of the start
  // of our code generation, but we're a bundle farther. We should
  // be flexible so that if someone adds in another random bundle we
  // don't break.

  // 0x10: sizeof(bundle)
  instruction_x longBranch = generateLongBranchTo( originalTarget - (allocatedAddress + 0x10));
  IA64_bundle longBranchBundle( MLXstop, memoryNOP, longBranch );
	
  longBranchBundle.generate(gen);

  /* Rewrite the short immediate. */
  if( isSpecCheck )
	SET_M20_TARGET( &tmpl, -16 );
  else
	SET_M22_TARGET( &tmpl, -16 );

  /* Emit the rewritten immediate. */
  instruction rewrittenInsn( tmpl.raw, insnToRewrite.getTemplateID(), insnToRewrite.getSlotNumber() );
  generateBundleFor( rewrittenInsn ).generate(gen);
} /* end rewriteShortOffset() */

/* private refactoring function */
bool instruction::generate(codeGen &gen, 
						   process *,
						   Address originalLocation,
						   Address allocatedAddress,
						   Address,
						   Address fallthroughTarget) {
  if (fallthroughTarget) {
	fprintf(stderr, "WARNING: ignoring redirected branch target of 0x%lx!\n",
			fallthroughTarget);
  }
	
  insn_tmpl tmpl = { getMachineCode() };

  switch( getType() ) {
  case instruction::DIRECT_BRANCH:
	rewriteShortOffset( *this, originalLocation, gen,  allocatedAddress);
	break; /* end direct jump handling */
		
  case instruction::DIRECT_CALL: {
	/* Direct calls have to be rewritten as long calls in order to make sure the assertions
	   installBaseTramp() makes about the frame state in the dynamic unwind information
	   are true.  (If we call backwards one instruction, the frame there is not the same as
	   it was at the instruction we're emulating.)  */
	insn_tmpl tmpl = { getMachineCode() };
	Address originalTarget = GET_M22_TARGET( &tmpl ) + originalLocation - (originalLocation % 16);
	instruction_x longCall = generateLongCallTo( originalTarget - allocatedAddress,
												 tmpl.B3.b1, tmpl.B3.qp );
	instruction memoryNOP( NOP_M );			
	IA64_bundle longCallBundle( MLXstop, memoryNOP, longCall );
	longCallBundle.generate(gen);
  } break; /* end direct call handling */		
	
  case instruction::BRANCH_PREDICT:
	/* We can suffer the performance loss. :) */
	IA64_bundle( MIIstop, NOP_M, NOP_I, NOP_I ).generate(gen);
		
	break; /* end branch predict handling */
		
  case instruction::ALAT_CHECK:
  case instruction::SPEC_CHECK:
	/* The advanced load checks can be handled exactly as we 
	   handle direct branches and calls.  The other three checks
	   (I&M unit integer speculation, fp speculation) are handled
	   identically to each other, but their immediates are laid
	   out a little different, so we can't handle them as we do
	   the advanced loads. */
	rewriteShortOffset( *this, originalLocation, gen, allocatedAddress );
		
	/* FIXME: the jump back needs to be fixed.  Implies we need to leave the emulated code in-place. */
	break; /* end speculation check handling */
		
  case instruction::BRANCH_IA:
	assert( 0 );
	break; /* end branch to x86 handling */
		
  case instruction::MOVE_FROM_IP: {
	/* Replace with a movl of the original IP. */
	unsigned int originalRegister = tmpl.I25.r1;
	instruction memoryNOP( NOP_M );
	instruction_x emulatedIPMove = generateLongConstantInRegister( originalRegister, originalLocation );
	IA64_bundle ipMoveBundle( MLXstop, memoryNOP, emulatedIPMove );
	ipMoveBundle.generate(gen);
	break; } /* end ip move handling */
		
  case instruction::INVALID:
	fprintf( stderr, "Not emulating INVALID instruction.\n" );
	break;
		
  case instruction::ALLOC:
	// When emulating an alloc instruction, we must insure that it is the
	// first instruction of an instruction group.
	IA64_bundle( MstopMIstop, NOP_M, *this, NOP_I ).generate(gen);
	break;
		
  case instruction::RETURN:
  case instruction::INDIRECT_CALL:
  case instruction::INDIRECT_BRANCH:
	/* Branch registers hold absolute addresses. */
  default:
	generateBundleFor( *this ).generate(gen);
	break; /* end default case */
  } /* end type switch */

  return true;
} /* end emulate */

/* Required by BPatch_thread.C */
#define NEAR_ADDRESS 0x2000000000000000               /* The lower end of the shared memory segment. */

bool process::replaceFunctionCall(instPoint * point, 
								  const int_function * func ) {
  /* We'll assume that point is a call site, primarily because we don't actually care.
	 However, the semantics seem to be that we can't replace function calls if we've already
	 instrumented them (which is, of course, garbage), so we _will_ check that. */
  // Must be a call site
  if (point->getPointType() != callSite)
	return false;

  /* We'll need this later. */
  instruction memoryNOP( NOP_M );
	
  /* Further implicit semantics indicate that a NULL function should be replace by NOPs. */
  IA64_bundle toInsert;
  if( func == NULL ) {
	toInsert = IA64_bundle( MIIstop, NOP_M, NOP_I, NOP_I );
  } /* end if we're inserting NOPs. */
	
  instPointIter ipIter(point);
  instPointInstance *ipInst;
  while ((ipInst = ipIter++)) {  
	// Multiple replacements. Wheee...
	Address pointAddr = ipInst->addr();
		
	codeRange *range;
	if (modifiedAreas_.find(pointAddr, range)) {
	  multiTramp *multi = range->is_multitramp();
	  if (multi) {
		// We pre-create these guys... so check to see if
		// there's anything there
		if (!multi->generated()) {
		  removeMultiTramp(multi);
		}
		else {
		  // TODO: modify the callsite in the multitramp.
		  assert(0);
		}
	  }
	  if (dynamic_cast<functionReplacement *>(range)) {
		// We overwrote this in a function replacement...
		continue; 
	  }
	}

	/* We'd like to generate the call here, but we need to know the IP it'll be
	   at in order to generate the correct offset.  Grr. */
		
	/* Since we need a full bundle to guarantee we can make the jump, we'll
	   need to insert a minitramp to the emulated instructions.  Gather some
	   of the necessary data. */
	uint8_t slotNo = pointAddr % 16;
	Address alignedAddress = pointAddr - slotNo;
		
	/* We'll construct the minitramp in the mutator's address space and then copy
	   it over once we're done. */
	codeGen codeBuffer(7*instruction::size());
		
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
	instruction_x lbtMT = generateLongBranchTo( offsetToMiniTramp );
	IA64_bundle jumpBundle( MLXstop, memoryNOP, lbtMT );
	ia64_bundle_t rawJumpBundle = jumpBundle.getMachineCode();
	assert( writeTextSpace( (void *)alignedAddress, 16, & rawJumpBundle ) );


	// We keep track of these so that we can disable mutations later.

	// In this case we leave the minitramp around, just
	// overwriting whatever was in the bundle the first time.

	replacedFunctionCall *newRFC = new replacedFunctionCall();
	newRFC->callAddr = alignedAddress;
	newRFC->callSize = 16;
	if (func)
	  newRFC->newTargetAddr = func->getAddress();
	else
	  newRFC->newTargetAddr = 0;

	newRFC->oldCall.allocate(16);
	newRFC->oldCall.copy(&originalBundle, sizeof(originalBundle));

	newRFC->newCall.allocate(16);
	newRFC->newCall.copy(&rawJumpBundle, sizeof(rawJumpBundle));
	replacedFunctionCalls_[alignedAddress] = newRFC;

	/* We don't want to emulate the call instruction itself. */
	if( slotNo != 0 ) {
	  bundleToEmulate.getInstruction0().generate(codeBuffer, this, alignedAddress, allocatedAddress);
	}
	else {
	  if( func != NULL ) {
		/* The SCRAG says that b0 is the return register, but there's no gaurantee of this.
		   If we have problems, we can decode the call instruction. */
		uint8_t branchRegister = 0;
		int64_t callOffset = func->getAddress()- (allocatedAddress + codeBuffer.used());
				
		instruction_x callToFunction = generateLongCallTo( callOffset, branchRegister );
		toInsert = IA64_bundle( MLXstop, memoryNOP, callToFunction );
	  } /* end if we have a function call to insert. */
			
	  toInsert.generate(codeBuffer);
	}
		
	if( bundleToEmulate.hasLongInstruction() ) {
	  if( slotNo == 0 ) {
		bundleToEmulate.getLongInstruction().generate(codeBuffer,
													  this,
													  alignedAddress,
													  allocatedAddress);
	  }
	  else {
		if( func != NULL ) {
		  /* The SCRAG says that b0 is the return register, but there's no gaurantee of this.
			 If we have problems, we can decode the call instruction. */
		  uint8_t branchRegister = 0;
		  int64_t callOffset = func->getAddress() - (allocatedAddress + codeBuffer.used());
					
		  instruction_x callToFunction = generateLongCallTo( callOffset, branchRegister );
		  toInsert = IA64_bundle( MLXstop, memoryNOP, callToFunction );
		} /* end if we have a function call to insert. */
				
		toInsert.generate(codeBuffer);
	  }
	} /* end if we have a long instruction */
	else {
	  /* Handle the instruction in slot 1. */
	  if( slotNo != 1 ) {
		bundleToEmulate.getInstruction1().generate(codeBuffer,
												   this,
												   alignedAddress,
												   allocatedAddress);
	  }
	  else {
		if( func != NULL ) {
		  /* The SCRAG says that b0 is the return register, but there's no gaurantee of this.
			 If we have problems, we can decode the call instruction. */
		  uint8_t branchRegister = 0;
		  int64_t callOffset = func->getAddress() - (allocatedAddress + codeBuffer.used());
					
		  instruction_x callToFunction = generateLongCallTo( callOffset, branchRegister );
		  toInsert = IA64_bundle( MLXstop, memoryNOP, callToFunction );
		} /* end if we have a function call to insert. */
				
		toInsert.generate(codeBuffer);
	  }
			
	  /* Handle the instruction in slot 2. */
	  if( slotNo != 2 ) {
		bundleToEmulate.getInstruction2().generate(codeBuffer,
												   this,
												   alignedAddress,
												   allocatedAddress);
	  }
	  else {
		if( func != NULL ) {
		  /* The SCRAG says that b0 is the return register, but there's no gaurantee of this.
			 If we have problems, we can decode the call instruction. */
		  uint8_t branchRegister = 0;
		  int64_t callOffset = func->getAddress() - (allocatedAddress + codeBuffer.used());
					
		  instruction_x callToFunction = generateLongCallTo( callOffset, branchRegister );
		  toInsert = IA64_bundle( MLXstop, memoryNOP, callToFunction );
		} /* end if we have a function call to insert. */
				
		toInsert.generate(codeBuffer);
	  }
	} /* end if all three are short instructions. */
		
	/* Append the return jump to the minitramp. */
	Address returnOffset = ( alignedAddress + 16 ) /* target */ - 
	  (allocatedAddress + codeBuffer.used()) /* IP */;
	instruction_x lbtOriginal = generateLongBranchTo( returnOffset );
	IA64_bundle jumpBackBundle( MLXstop, memoryNOP, lbtOriginal );
	jumpBackBundle.generate(codeBuffer);
		
	/* Write the minitramp into the mutatee's address space. */
	// /* DEBUG */ fprintf( stderr, "* Wrote minitramp at 0x%lx for %d bytes, from 0x%lx\n", allocatedAddress, size + 16, codeBuffer );
	assert( writeTextSpace( (void *)allocatedAddress, 
							codeBuffer.used(), codeBuffer.start_ptr() ) );
		
	// /* DEBUG */ fprintf( stderr, "* Emitted minitramp at local 0x%lx, for instPoint at 0x%lx and remote 0x%lx\n", (Address) & codeBuffer, alignedAddress, allocatedAddress );
  }
	
  return true;
	
} /* end replaceFunctionCall() */

/* Private Refactoring Function

Moves preserved unstacked registers to register stack for basetramp.
FIXME: handling for ar.fpsr (status field 1), ar.rsc (mode), and User Mask.
*/
void generateRegisterStackSave( codeGen &gen, unw_dyn_region_info_t * unwindRegion )
{
  instruction insn[ 3 ];
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
	  if( slot < 2 ) insn[ 1 ] = instruction( NOP_I );
	  if( slot < 3 ) insn[ 2 ] = instruction( NOP_I );
			
	  bundle = IA64_bundle( MII, insn[ 0 ], insn[ 1 ], insn[ 2 ] );
	  bundle.generate(gen);
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
	  if( slot < 3 ) insn[ 2 ] = instruction( NOP_I );

	  bundle = IA64_bundle( MII, NOP_I, insn[ 1 ], insn[ 2 ] );
	  bundle.generate(gen);
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
		insn[ slot++ ] = instruction( NOP_M );
		isPFS = true;

	  } else {
		// Default case.
		insn[ slot++ ] = generateApplicationToRegisterMove( i - BP_AR0, regSpace->storageMap[ i ] );
	  }
	}

	if( slot == 2 || (i == BP_AR0 + 127 && slot) ) {
	  if( slot < 2 ) insn[ 1 ] = instruction( NOP_M );

	  bundle = IA64_bundle( MMI, insn[ 0 ], insn[ 1 ], ( isPFS ? insn[ 2 ] : NOP_I ) );
	  bundle.generate(gen);
	  unwindRegion->insn_count += 3;

	  slot = 0;
	  isPFS = false;
	}
  }

  // Save the predicate registers
  insn[ 0 ] = instruction( NOP_M );
  if( regSpace->storageMap[ BP_PR ] > 0 ) {
	insn[ 1 ] = generatePredicatesToRegisterMove( regSpace->storageMap[ BP_PR ] );
	_U_dyn_op_save_reg( & unwindRegion->op[ unwindRegion->op_count++ ], _U_QP_TRUE,
						unwindRegion->insn_count + (slot-1), UNW_IA64_PR,
						regSpace->storageMap[ BP_PR ] );

  } else {
	// This bundle is needed to end the instruction group, regardless.
	insn[ 1 ] = instruction( NOP_I );
  }
  insn[ 2 ] = instruction( NOP_I );

  bundle = IA64_bundle( MIIstop, insn[ 0 ], insn[ 1 ], insn[ 2 ] );
  bundle.generate(gen);
  unwindRegion->insn_count += 3;
}

/* Private Refactoring Function

Moves preserved registers back to original location for basetramp trailer.
FIXME: handling for ar.fpsr (status field 1), ar.rsc (mode), and User Mask.
*/
void generateRegisterStackRestore( codeGen &gen, unw_dyn_region_info_t * unwindRegion )
{
  instruction insn[ 3 ];
  IA64_bundle bundle;
  int slot;

  // Restore the general registers.
  slot = 0;
  for( Register i = BP_GR0; i < BP_GR0 + 128; ++i ) {
	if( i != BP_GR0 + 12 && regSpace->storageMap[ i ] > 0 )
	  insn[ slot++ ] = generateRegisterToRegisterMove( regSpace->storageMap[ i ], i - BP_GR0 );

	if( slot == 3 || (i == BP_GR0 + 127 && slot) ) {
	  if( slot < 2 ) insn[ 1 ] = instruction( NOP_I );
	  if( slot < 3 ) insn[ 2 ] = instruction( NOP_I );

	  bundle = IA64_bundle( MII, insn[ 0 ], insn[ 1 ], insn[ 2 ] );
	  bundle.generate(gen);
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
	  if( slot < 3 ) insn[ 2 ] = instruction( NOP_I );

	  bundle = IA64_bundle( MII, NOP_I, insn[ 1 ], insn[ 2 ] );
	  bundle.generate(gen);
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
		insn[ slot++ ] = instruction( NOP_M );
		isPFS = true;

	  } else {
		// Default case.
		insn[ slot++ ] = generateRegisterToApplicationMove( regSpace->storageMap[ i ], i - BP_AR0 );
	  }
	}

	if( slot == 2 || (i == BP_AR0 + 127 && slot) ) {
	  if( slot < 2 ) insn[ 1 ] = instruction( NOP_M );

	  bundle = IA64_bundle( MMI, insn[ 0 ], insn[ 1 ], ( isPFS ? insn[ 2 ] : NOP_I ) );
	  bundle.generate(gen);
	  unwindRegion->insn_count += 3;

	  slot = 0;
	  isPFS = false;
	}
  }

  // Restore the predicate registers
  insn[ 0 ] = instruction( NOP_M );
  if( regSpace->storageMap[ BP_PR ] > 0 ) {
	insn[ 1 ] = generateRegisterToPredicatesMove( regSpace->storageMap[ BP_PR ], ~0x1 );
  } else {
	// This bundle is needed to end the instruction group, regardless.
	insn[ 1 ] = instruction( NOP_I );
  }
  insn[ 2 ] = instruction( NOP_I );

  bundle = IA64_bundle( MIIstop, insn[ 0 ], insn[ 1 ], insn[ 2 ] );
  bundle.generate(gen);
  unwindRegion->insn_count += 3;
}

/* Private Refactoring Function

Spills stacked registers to memory stack, if needed.
*/
void generateMemoryStackSave( codeGen &gen, bool * usedFPregs, unw_dyn_region_info_t * unwindRegion )
{
  Register temp_gr[ 2 ] = { 6 };
  Register temp_fr = 6;

  instruction insn[ 2 ];
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
	  bundle.generate(gen);
	  unwindRegion->insn_count += 3;

	  bundle = IA64_bundle( MMIstop,
							generateRegisterToFloatMove( temp_gr[ 0 ], temp_fr ),
							generateApplicationToRegisterMove( AR_UNAT, temp_gr[ 0 ] ),
							NOP_I );
	  bundle.generate(gen);
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
	  bundle.generate(gen);
	  unwindRegion->insn_count += 3;
	}

	// Store ar.unat
	bundle = IA64_bundle( MIIstop,
						  generateSpillTo( REGISTER_SP, temp_gr[ 0 ], 0 ),
						  NOP_I,
						  NOP_I );
	bundle.generate(gen);
	unwindRegion->insn_count += 3;
  }
  /* This is always necessary, because of function calls and the floating-point spills for multiplication. */
  bundle = IA64_bundle( MIIstop,
						generateShortImmediateAdd( REGISTER_SP, -regSpace->sizeOfStack, REGISTER_SP ),
						NOP_I,
						NOP_I );
  _U_dyn_op_add( & unwindRegion->op[ unwindRegion->op_count++ ], _U_QP_TRUE,
				 unwindRegion->insn_count, UNW_IA64_SP, -regSpace->sizeOfStack );
  bundle.generate(gen);
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
	  bundle.generate(gen);
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
	  bundle.generate(gen);
	  unwindRegion->insn_count += 3;

	  bundle = IA64_bundle( MIIstop,
							generateFPFillFrom( temp_gr[ 1 ], temp_fr, 0 ),
							NOP_I,
							NOP_I );
	  bundle.generate(gen);
	  unwindRegion->insn_count += 3;
	}
  }

  // Save the FP regs
  temp_gr[ 0 ] = regSpace->getRegSlot( 0 )->number;
  temp_gr[ 1 ] = regSpace->getRegSlot( 1 )->number;

  /* This should only be necessary if an FP register is used; not sure
	 that two more/fewer GP registers really matter. */
  bundle = IA64_bundle( MIIstop,
						generateShortImmediateAdd( temp_gr[ 0 ], frStackOffset +  0, REGISTER_SP ),
						generateShortImmediateAdd( temp_gr[ 1 ], frStackOffset + 16, REGISTER_SP ),
						NOP_I );
  bundle.generate(gen);
  unwindRegion->insn_count += 3;

  // Save the floating point registers
  int slot = 0;
  for( int i = 0; i < 128; i++ ) {
	if( usedFPregs[i] ) {
	  insn[ slot ] = generateFPSpillTo( temp_gr[ slot ], i, 32 );
	  ++slot;
	}

	if( slot == 2 || (i == 127 && slot) ) {
	  if( slot < 2 ) insn[ 1 ] = instruction( NOP_M );

	  bundle = IA64_bundle( MMIstop, insn[ 0 ], insn[ 1 ], NOP_I );
	  bundle.generate(gen);
	  unwindRegion->insn_count += 3;

	  slot = 0;
	}
  }
}

/* Private Refactoring Function
   Fills stacked registers from memory stack, if needed.
*/
void generateMemoryStackRestore( codeGen &gen, bool * usedFPregs, unw_dyn_region_info_t * unwindRegion )
{
  Register temp_gr[ 2 ] = { 6 };
  Register temp_fr = 6;

  instruction insn[ 2 ];
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
  bundle.generate(gen);
  unwindRegion->insn_count += 3;

  // Restore the FP registers
  int slot = 0;
  for( int i = 0; i < 128; i++ ) {
	if( usedFPregs[i] ) {
	  insn[ slot ] = generateFPFillFrom( temp_gr[ slot ], i, 32 );
	  ++slot;
	}

	if( slot == 2 || (i == 127 && slot) ) {
	  if( slot < 2 ) insn[ 1 ] = instruction( NOP_M );

	  bundle = IA64_bundle( MMIstop, insn[ 0 ], insn[ 1 ], NOP_I );
	  bundle.generate(gen);
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
	  bundle.generate(gen);
	  unwindRegion->insn_count += 3;

	  bundle = IA64_bundle( MIIstop,
							generateRegisterToFloatMove( temp_gr[ 0 ], temp_fr ),
							NOP_I,
							NOP_I );
	  bundle.generate(gen);
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
	  bundle.generate(gen);
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
	  bundle.generate(gen);
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
  bundle.generate(gen);
  unwindRegion->insn_count += 3;

  if( grSpillNeeded ) {
	// Restore ar.unat
	bundle = IA64_bundle( MstopMIstop,
						  generateFillFrom( REGISTER_SP, temp_gr[ 0 ], 0 ),
						  generateRegisterToApplicationMove( temp_gr[ 0 ], AR_UNAT ),
						  NOP_I );
	bundle.generate(gen);
	unwindRegion->insn_count += 3;

	bundle = IA64_bundle( MIIstop,
						  generateShortImmediateAdd( REGISTER_SP, 16, REGISTER_SP ),
						  NOP_I,
						  NOP_I );
	_U_dyn_op_pop_frames( & unwindRegion->op[ unwindRegion->op_count++ ], _U_QP_TRUE, unwindRegion->insn_count, 1 );
	bundle.generate(gen);
	unwindRegion->insn_count += 3;

	if( originalFrameSize == 96 ) {
	  bundle = IA64_bundle( MMIstop,
							generateFloatToRegisterMove( temp_fr, temp_gr[ 0 ] ),
							generateFPFillFrom( REGISTER_SP, temp_fr, 16 ),
							NOP_I );
	  _U_dyn_op_pop_frames( & unwindRegion->op[ unwindRegion->op_count++ ], _U_QP_TRUE, unwindRegion->insn_count + 1, 1 );
	  bundle.generate(gen);
	  unwindRegion->insn_count += 3;
	}
  }
}

/* private refactoring function */
bool generatePreservationHeader(codeGen &gen, bool * whichToPreserve, unw_dyn_region_info_t * unwindRegion ) {
  /* For clarity (in the callers, too), handle NULL unwindRegions here. */
  bool freeUnwindRegion = false;
  if( unwindRegion == NULL ) {
	unwindRegion = (unw_dyn_region_info_t *)malloc( _U_dyn_region_info_size( PRESERVATION_UNWIND_OPERATION_COUNT ) );
	assert( unwindRegion != NULL );
	unwindRegion->op_count = 0;
	unwindRegion->insn_count = 0;
		
	freeUnwindRegion = true;
  } /* end if unwindRegion was NULL */

  instruction integerNOP( NOP_I );
  instruction memoryNOP( NOP_M );
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
	bundle.generate(gen);
		
	_U_dyn_op_save_reg( & unwindRegion->op[ unwindRegion->op_count++ ], _U_QP_TRUE,
						unwindRegion->insn_count, UNW_IA64_AR_PFS, 32 + originalFrameSize );
	unwindRegion->insn_count += 3;
  }

  // generateMemoryStackSave() *MUST* be called before generateRegisterStackSave()
  generateMemoryStackSave( gen, whichToPreserve, unwindRegion );
  generateRegisterStackSave( gen, unwindRegion );

  // /* DEBUG */ fprintf( stderr, "Emitted %d-bundle preservation header at 0x%lx\n", bundleCount, (Address)insnPtr ); 

  /* Update the offset. */
  assert( unwindRegion->op_count < PRESERVATION_UNWIND_OPERATION_COUNT );
  if( freeUnwindRegion ) { free( unwindRegion ); }
  return true;
} /* end generatePreservationHeader() */

/* private refactoring function; assumes that regSpace and
   deadRegisterList are as they were in the corresponding call
   to generatePreservationHeader(). */
bool generatePreservationTrailer( codeGen &gen, bool * whichToPreserve, unw_dyn_region_info_t * unwindRegion ) {
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
  int originalFrameSize = regSpace->originalLocals + regSpace->originalOutputs;

  instruction memoryNOP( NOP_M );
  instruction integerNOP( NOP_I );

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
  generateRegisterStackRestore( gen, unwindRegion );
  generateMemoryStackRestore( gen, whichToPreserve, unwindRegion );

  /* Restore the original frame. */
  if( originalFrameSize < 96 ) {
	/* Restore ar.pfs; preserved by the header's alloc instruction. */
	instruction movePFS = generateRegisterToApplicationMove( 32 + originalFrameSize, AR_PFS );
	IA64_bundle fifthMoveBundle( MIIstop, memoryNOP, movePFS, integerNOP );
	fifthMoveBundle.generate(gen);
	unwindRegion->insn_count += 3;
		
	/* FIXME: Update for save-and-restore around alloc. */
	_U_dyn_op_stop( & unwindRegion->op[ unwindRegion->op_count++ ] );

	IA64_bundle saveBundle = IA64_bundle( MstopMIstop, 
										  generateShortImmediateAdd( REGISTER_SP, -16, REGISTER_SP ),
										  generateRegisterStoreImmediate( REGISTER_SP, REGISTER_GP, 0 ),
										  integerNOP );
	saveBundle.generate(gen);
	
	instruction allocInsn = generateOriginalAllocFor( regSpace );
	IA64_bundle allocBundle( MIIstop, allocInsn, NOP_I, NOP_I );
	allocBundle.generate(gen);
		
	IA64_bundle restoreBundle = IA64_bundle( MIIstop,
											 generateRegisterLoadImmediate( REGISTER_GP, REGISTER_SP, +16 ),
											 integerNOP,
											 integerNOP );
	restoreBundle.generate(gen);

	unwindRegion->insn_count += 9;
  } else {
	_U_dyn_op_stop( & unwindRegion->op[ unwindRegion->op_count++ ] );
  }
  /* FIXME: It's only after the above alloc executes that anything changes from the
	 POV of the unwinder, because the preserved values stay where they were
	 preserved until then. */
	
  // /* DEBUG */ fprintf( stderr, "Emitted %d-bundle preservation trailer at 0x%lx\n", bundleCount, (Address)insnPtr ); 

  /* Update the offset. */
  assert( unwindRegion->op_count < PRESERVATION_UNWIND_OPERATION_COUNT );
  if( freeUnwindRegion ) { free( unwindRegion ); }
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
  reg_tmpl reg = { getDBI()->ptrace( PTRACE_PEEKUSER, proc->getPid(), PT_CR_IPSR, 0 ) };
  if( errno && (reg.raw == ((unsigned long)-1)) ) {
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
  instruction *insn = origBundle.getInstruction(ri);

  // Determine predicate register and remove it from instruction.
  pr = getDBI()->ptrace( PTRACE_PEEKUSER, proc->getPid(), PT_PR, 0 );
  if (errno && pr == -1) assert(0);
  pr = ( pr >> insn->getPredicate() ) & 0x1;

  return (insn->getType() == instruction::SYSCALL && pr);
} /* end needToHandleSyscall() */

#include "dlfcn.h"

/* From ia64-template.s. */
extern void (* syscallPrefix)();
extern void (* prefixNotInSyscall)();
extern void (* prefixInSyscall)();
extern void (* prefixCommon)();
extern void (* jumpFromNotInSyscallToPrefixCommon)();

bool emitSyscallHeader( process * proc, codeGen &gen) {
  /* Extract the current slotNo. */
  errno = 0;
  reg_tmpl reg = { getDBI()->ptrace( PTRACE_PEEKUSER, proc->getPid(), PT_CR_IPSR, 0 ) };
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

  /* The linker is doing Something Funky, but what we want is the address of
	 what we're claiming are a bunch of function pointers. */
  Address syscallPrefixAddress = (Address)(& syscallPrefix);
  Address prefixNotInSyscallAddress = (Address)(& prefixNotInSyscall);
  Address prefixInSyscallAddress = (Address)(& prefixInSyscall);
  Address prefixCommonAddress = (Address)(& prefixCommon);
  Address jumpAddress = (Address)(& jumpFromNotInSyscallToPrefixCommon);
	
  Address lengthWithoutTrailingNOPs = prefixCommonAddress - syscallPrefixAddress;
  gen.copy( (void *)syscallPrefixAddress, lengthWithoutTrailingNOPs );
  int endOfTemplateIndex = gen.getIndex();

  /* Likewise, we need to rewrite the nop.b bundle at jumpFromNotInSyscallToPrefixCommon
	 because the assembler jumps are indirected.  (Why can't I force a relative jump?!) */
  instruction memoryNOP( NOP_M );
  instruction_x branchPastInSyscall = generateLongBranchTo( prefixCommonAddress - jumpAddress );
  IA64_bundle branchPastInSyscallBundle( MLXstop, memoryNOP, branchPastInSyscall );

  gen.setIndex( (jumpAddress - syscallPrefixAddress) / 16 );
  branchPastInSyscallBundle.generate(gen);

  /* Construct the slot-specific jump bundle. */
  IA64_bundle firstJumpBundle;
  IA64_bundle secondJumpBundle;
  instruction branchNOP( NOP_B );
	
  /* Probably wouldn't hurt to assert that the immediates here are small enough. */
  instruction jumpToInSyscall = generateShortImmediateBranch( prefixInSyscallAddress - (syscallPrefixAddress + 0x10 ) );
  instruction jumpToNotInSyscall = generateShortImmediateBranch( prefixNotInSyscallAddress - (syscallPrefixAddress + 0x10) );
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
		
  gen.setIndex( 0 );
  firstJumpBundle.generate( gen );
  secondJumpBundle.generate( gen );
  gen.setIndex( endOfTemplateIndex );

  // /* DEBUG */ fprintf( stderr, "* iRPC system call handler (prefix) generated at 0x%lx\n", (Address) bundlePtr );
  return true;
} /* end emitSystemCallHeader() */

/* private refactoring function */
bool * doFloatingPointStaticAnalysis( const instPoint * location ) {
  /* Cast away const-ness rather than fix broken int_function::getAddress(). */
  int_function * functionBase = location->func();

  InstrucIter iAddr(functionBase);

  // /* DEBUG */ fprintf( stderr, "Mutator claims to have mutatee machine code from 0x%lx to 0x%lx\n", mutatorAddress, lastI );
  // /* DEBUG */ fprintf( stderr, "(claims to be function '%s')\n", functionBase->symTabName().c_str() );

  bool fpUsed = false;
  bool registersRotated = false;
  bool * whichToPreserve = (bool *)calloc( 128, sizeof( bool ) );
	
  while (iAddr.hasMore()) {
	instruction currInsn = iAddr.getInstruction();
	int slotNo = currInsn.getSlotNumber();
	iAddr++;

	insn_tmpl tmpl = { currInsn.getMachineCode() };
		
	bool instructionIsFP = false;
	bool instructionIsMem = false;
	uint8_t templateID = currInsn.getTemplateID();
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
bool rpcMgr::emitInferiorRPCheader( codeGen &gen ) {
  /* Extract the CFM. */
  errno = 0;
  reg_tmpl reg = { getDBI()->ptrace( PTRACE_PEEKUSER, proc_->getPid(), PT_CFM, 0 ) };
  assert( ! errno );

  /* FIXME: */ if( ! ( 0 == reg.CFM.rrb_pr == reg.CFM.rrb_fr == reg.CFM.rrb_gr ) ) { assert( 0 ); }
  /* FIXME: */ if( reg.raw == INVALID_CFM ) { assert( 0 ); }

  /* Set regSpace for the code generator. */
  int baseReg = 32 + reg.CFM.sof + NUM_PRESERVED;
  if( baseReg > 128 - (NUM_LOCALS + NUM_OUTPUT) ) {
	baseReg = 128 - (NUM_LOCALS + NUM_OUTPUT); // Never allocate over 128 registers.
  }

  Register deadRegisterList[NUM_LOCALS + NUM_OUTPUT];
  for( int i = 0; i < NUM_LOCALS + NUM_OUTPUT; ++i ) {
	deadRegisterList[i] = baseReg + i;
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

  if( needToHandleSyscall( proc_ ) ) {
	if( ! emitSyscallHeader( proc_, gen) ) { return false; }
  }
  else {
	/* We'll be adjusting the PC to the start of the preservation code,
	   but we can't change the slot number ([i]psr.ri), so we need to soak
	   up the extra slots with nops.  (Because the syscall header may require
	   a bundle _before_ the jump target, add two NOPs; the installation routine(s)
	   will compensate.) */
	IA64_bundle( MIIstop, NOP_M, NOP_I, NOP_I ).generate(gen);
	IA64_bundle( MIIstop, NOP_M, NOP_I, NOP_I ).generate(gen);		
  }

  /* It'll probably be faster just to spill all the FP registers,
	 given that this code'll only be running once. */
  bool * whichToPreserve = NULL;

  /* Generate the preservation header; don't bother with unwind information
	 for an inferior RPC.  (It must be the top of the stack.) */
  return generatePreservationHeader( gen, whichToPreserve, NULL );
} /* end emitInferiorRPCheader() */

/* From ia64-template.s. */
extern void (* syscallSuffix)();
extern void (* suffixExitPoint)();
bool emitSyscallTrailer( codeGen &gen, uint64_t slotNo ) {
  /* Copy the template from 'syscallSuffix' to 'suffixExitPoint' (inclusive),
	 and replace the bundle at 'suffixExitPoint' with one which has 
	 predicated SIGILLs in the right places. */

  /* Copy the code template. */
  Address syscallSuffixAddress = (Address)(& syscallSuffix );
  Address suffixExitPointAddress = (Address)(& suffixExitPoint );
	
  /* Trailing NOPs are replaced by the constructed SIGILLs. */
  Address lengthWithTrailingNOPs = ( suffixExitPointAddress + 0x10 ) - syscallSuffixAddress;
  gen.copy( (void *)syscallSuffixAddress, lengthWithTrailingNOPs );
  int predicatedBreakBundleIndex = gen.getIndex() - 1;
	
  /* Construct the predicated SIGILLs for the bundle at suffixExitPointAddress. */
  IA64_bundle predicatedBreakBundle;
  switch( slotNo ) {
  case 0: {
	/* slot 0 NOT, slot 2 IS;; p1 and p2 from assembler */
	instruction notInSyscallBreak( TRAP_M );
	instruction inSyscallBreak( TRAP_I );
	notInSyscallBreak = predicateInstruction( 1, notInSyscallBreak );
	inSyscallBreak = predicateInstruction( 2, inSyscallBreak );
	// PROBLEM: in this case, the PC may need to be backed up!
	predicatedBreakBundle = IA64_bundle( MMIstop, notInSyscallBreak, NOP_M, inSyscallBreak );
	assert( 0 );
  } break;
			
  case 1: {
	/* slot 1 NOT, slot 0 IS */
	instruction notInSyscallBreak( TRAP_M );
	instruction inSyscallBreak( TRAP_M );
	notInSyscallBreak = predicateInstruction( 1, notInSyscallBreak );
	inSyscallBreak = predicateInstruction( 2, inSyscallBreak );
	predicatedBreakBundle = IA64_bundle( MMIstop, inSyscallBreak, notInSyscallBreak, NOP_I );
  } break;
			
  case 2: {
	/* slot 2 NOT, slot 1 IS */
	instruction memoryNOP( NOP_M );
	instruction notInSyscallBreak( TRAP_I );
	instruction inSyscallBreak( TRAP_M );
	notInSyscallBreak = predicateInstruction( 1, notInSyscallBreak );
	inSyscallBreak = predicateInstruction( 2, inSyscallBreak );
	predicatedBreakBundle = IA64_bundle( MMIstop, memoryNOP, inSyscallBreak, notInSyscallBreak );
  } break;
			
  default:
	assert( 0 );
  } /* end slotNo switch */
		
  codeBufIndex_t index = gen.getIndex();
  gen.setIndex( predicatedBreakBundleIndex );
  predicatedBreakBundle.generate( gen );
  gen.setIndex( index );

  // /* DEBUG */ fprintf( stderr, "* iRPC system call handler (suffix) generated at 0x%lx\n", (Address) bundlePtr );
  return true;
} /* end emitSyscallTrailer() */

/* Required by process.C */
bool rpcMgr::emitInferiorRPCtrailer( codeGen &gen,
									 unsigned & breakOffset, bool shouldStopForResult,
									 unsigned & stopForResultOffset,
									 unsigned & justAfter_stopForResultOffset ) {
  /* We'll need two of these. */
  IA64_bundle trapBundle = generateTrapBundle();

  /* Get a real instruction pointer. */
  if( shouldStopForResult ) {
	// * DEBUG */ fprintf( stderr, "* iRPC will stop for result.\n" );
	stopForResultOffset = gen.used();
	trapBundle.generate(gen);
	justAfter_stopForResultOffset = gen.used();
	/* Make sure that we eat the ipsr.ri with a nop. */
	IA64_bundle( MIIstop, NOP_M, NOP_I, NOP_I ).generate(gen);
  } /* end if we're interested in the result. */

	/* Determine which of the scratch (f6 - f15, f32 - f127) floating-point
	   registers need to be preserved.  See comment in emitInferiorRPCHeader()
	   for why we don't actually do this. */
	// Address interruptedAddress = getPC( proc_->getPid() );
	// int_function * interruptedFunction = proc_->findFuncByAddr( interruptedAddress );
  bool * whichToPreserve = NULL; // doFloatingPointStaticAnalysis( interruptedFunction->funcEntry( proc_ ) );

  /* Generate the restoration code. */
  generatePreservationTrailer( gen, whichToPreserve, NULL );

  /* The SIGILL for the demon needs to happen in the instruction slot
	 corresponding to ipsr.ri so that the mutatee resumes in the correct
	 location after the daemon adjusts its PC. */
  errno = 0;
  reg_tmpl reg = { getDBI()->ptrace( PTRACE_PEEKUSER, proc_->getPid(), PT_CR_IPSR, 0 ) };
  assert( ! errno );
  uint64_t slotNo = reg.PSR.ri;
  assert( slotNo <= 2 );

  if( needToHandleSyscall( proc_ ) ) {
	if( ! emitSyscallTrailer(gen, slotNo ) ) { return false; }
	breakOffset = gen.used() - 16;
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

	breakOffset = gen.used();
	slottedTrapBundle.generate(gen);
  } /* end if we don't need to handle a syscall */

	/* If the demon drops the ball, make sure the mutatee suicides. */
  trapBundle.generate(gen);
  return true;
} /* end emitInferiorRPCtrailer() */

void baseTrampInstance::updateTrampCost( unsigned cost ) {
  if( baseT->costSize == 0 ) {
	return;
  }
  assert( baseT->costSize != 0 );		
  assert( baseT->valueRegister != 0 );
  assert( baseT->addressRegister != 0 );
    
  codeGen gen( baseT->costSize );
	
  instruction_x setCost = generateLongConstantInRegister( baseT->addressRegister, cost );
  generateBundleFromLongInstruction( setCost ).generate( gen );

  instruction addInstruction = generateArithmetic( plusOp, baseT->valueRegister, baseT->addressRegister, baseT->valueRegister );
  IA64_bundle addBundle( MIIstop, addInstruction, NOP_I, NOP_I );
  addBundle.generate( gen );

  Address trampCostAddr = trampPreAddr() + baseT->costValueOffset;
  proc()->writeDataSpace( (void *) trampCostAddr, gen.used(), (void *) gen.start_ptr() );
} /* end updateTrampCost() */

#define GUARD_PREDICATE_TRUE	6
#define GUARD_PREDICATE_FALSE	7
bool baseTramp::generateGuardPreCode(codeGen &gen,
									 codeBufIndex_t &guardJumpIndex,
									 registerSpace *rs) {

  // Assumptions: No need to "allocate" registers from registerSpace.  They are
  //				scratch registers which just came into existence.
  //
  //				The branch in guardOnBundle3 must jump past 3 bundles (Offset +4).
  //					1 for the miniTramp
  //					2 for the guardOff bundles

  assert(rs);

  Address trampGuardBase = proc()->trampGuardBase();
  if (!trampGuardBase) return false;

  int maxRegister = rs->getRegisterCount();
  trampGuardFlagAddr = rs->getRegSlot( maxRegister - 1 )->number;
  trampGuardFlagValue = rs->getRegSlot( maxRegister - 2 )->number;
	
  IA64_bundle guardOnBundle0( MLXstop,
							  instruction( NOP_M ),
							  generateLongConstantInRegister( trampGuardFlagAddr, trampGuardBase ));

  IA64_bundle guardOnBundle05(	MstopMIstop,
								generateShiftLeftAndAdd( trampGuardFlagAddr, rs->getRegSlot( 0 )->number - 1, 3, trampGuardFlagAddr ),
								instruction( NOP_M ),
								instruction( NOP_I ) );
			
  IA64_bundle guardOnBundle1( MstopMIstop,
							  generateRegisterLoad( trampGuardFlagValue, trampGuardFlagAddr, 4 ),
							  generateComparison( eqOp, GUARD_PREDICATE_TRUE, trampGuardFlagValue, REGISTER_ZERO ),
							  instruction( NOP_I ));

  IA64_bundle guardOnBundle2( MIIstop,
							  generateRegisterStore( trampGuardFlagAddr, REGISTER_ZERO, 4, GUARD_PREDICATE_FALSE ),
							  instruction( NOP_I ),
							  instruction( NOP_I ));

  /* This displacement is fixed up in finalizeGuardBranch(). */
  IA64_bundle guardOnBundle3( MLXstop,
							  instruction( NOP_M ),
							  generateLongBranchTo( 0 << 4, GUARD_PREDICATE_TRUE ));

  guardOnBundle0.generate(gen);
  guardOnBundle05.generate(gen);
  guardOnBundle1.generate(gen);
  guardOnBundle2.generate(gen);
  guardJumpIndex = gen.getIndex();
  guardOnBundle3.generate(gen); 

  return true; 
}

bool baseTramp::generateGuardPostCode(codeGen &gen, codeBufIndex_t &postIndex,
									  registerSpace *rs) 
{
  Address trampGuardBase = proc()->trampGuardBase();
  if (!trampGuardBase) return false;

  int maxRegister = rs->getRegisterCount();
  Register trampGuardFlagAddr = rs->getRegSlot( maxRegister - 1 )->number;
  Register trampGuardFlagValue = rs->getRegSlot( maxRegister - 2 )->number;

  IA64_bundle guardOffBundle0( MLXstop,
							   generateShortConstantInRegister( trampGuardFlagValue, 1 ),
							   generateLongConstantInRegister( trampGuardFlagAddr, proc()->trampGuardBase() ));
								 
  IA64_bundle guardOnBundle05(	MstopMIstop,
								generateShiftLeftAndAdd( trampGuardFlagAddr, rs->getRegSlot( 0 )->number - 1, 3, trampGuardFlagAddr ),
								instruction( NOP_M ),
								instruction( NOP_I ) );

  IA64_bundle guardOffBundle1( MMIstop,
							   generateRegisterStore( trampGuardFlagAddr, trampGuardFlagValue, 4 ),
							   instruction( NOP_M ),
							   instruction( NOP_I ));

  guardOffBundle0.generate(gen);
  guardOffBundle1.generate(gen);
  postIndex = gen.getIndex();
  return true;
}

bool baseTramp::generateCostCode( codeGen & gen, unsigned & costUpdateOffset, registerSpace * rSpace ) {
  Address costAddr = proc()->getObservedCostAddr();
  if( ! costAddr ) {
	return false;
  }
    
  assert( rSpace != NULL );    
  addressRegister = rSpace->allocateRegister( gen, false );
  valueRegister = rSpace->allocateRegister( gen, false );
    
    
  emitVload( loadOp, costAddr, addressRegister, valueRegister, gen, false, rSpace );
    
  /* In the general case, we need a bundle to set addressRegister to the cost,
	 and another to add it to valueRegister. */    
  costUpdateOffset = gen.used();
  IA64_bundle nopBundle( MIIstop, NOP_M, NOP_I, NOP_I );
  nopBundle.generate( gen );
  nopBundle.generate( gen );
    
  emitVstore( storeOp, valueRegister, addressRegister, costAddr, gen, false, rSpace );
    
    
  rSpace->freeRegister( valueRegister );
  rSpace->freeRegister( addressRegister );
    
  return true;
} /* end generateCostCode() */


bool baseTramp::generateMTCode( codeGen & gen, registerSpace * rs ) {
  pdvector< AstNode * > dummy;

  if( this->threaded() ) {
	AstNode * threadPos = new AstNode( "DYNINSTthreadIndex", dummy );
	assert( threadPos != NULL );
		
	Register src = threadPos->generateCode(	this->proc(), rs, gen,
											false /* no cost */, true /* root node */ );
										
	/* Ray: I'm asserting that we don't use the 35th preserved register for anything. */
	emitRegisterToRegisterCopy( src, rs->getRegSlot( 0 )->number - 1, gen, rs );
		
	removeAst( threadPos );
  }
  else {
	/* Stick a zero in the Known Thread Index register. */
	emitRegisterToRegisterCopy( BP_GR0, rs->getRegSlot( 0 )->number - 1, gen, rs );		
  }
		
  return true;
} /* end generateMTCode() */

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
  	
  generation = getDBI()->ptrace( PTRACE_PEEKTEXT, pid, (udil + generationOffset), (Address)NULL );
  assert( errno == 0 );
    
  ++generation;
    
  getDBI()->ptrace( PTRACE_POKETEXT, pid, (udil + generationOffset), generation );
  assert( errno == 0 );
    
    
  first = getDBI()->ptrace( PTRACE_PEEKTEXT, pid, (udil + firstOffset), (Address)NULL );
  assert( errno == 0 );
    
  getDBI()->ptrace( PTRACE_POKETEXT, pid, (di + nextOffset), first );
  assert( errno == 0 );
  getDBI()->ptrace( PTRACE_POKETEXT, pid, (di + prevOffset), (Address)NULL );
  assert( errno == 0 );
    
  if ( first ) {
	getDBI()->ptrace( PTRACE_POKETEXT, pid, (first + prevOffset), di );
	assert( errno == 0 );
  }
    
  getDBI()->ptrace( PTRACE_POKETEXT, pid, (udil + firstOffset), di );
  assert( errno == 0 );  
} /* end registerRemoteUnwindInformation() */

char * unwindOpTagToString( int8_t op ) {
  switch( op ) {
  case UNW_DYN_STOP: return "UNW_DYN_STOP";
  case UNW_DYN_SAVE_REG: return "UNW_DYN_SAVE_REG";
  case UNW_DYN_SPILL_FP_REL: return "UNW_DYN_SPILL_FP_REL";
  case UNW_DYN_SPILL_SP_REL: return "UNW_DYN_SPILL_SP_REL";
  case UNW_DYN_ADD: return "UNW_DYN_ADD";
  case UNW_DYN_POP_FRAMES: return "UNW_DYN_POP_FRAMES";
  case UNW_DYN_LABEL_STATE: return "UNW_DYN_LABEL_STATE";
  case UNW_DYN_COPY_STATE: return "UNW_DYN_COPY_STATE";
  case UNW_DYN_ALIAS: return "UNW_DYN_ALIAS";
		
  default:
	return "INVALID";
  }
} /* end unwindOpTagToString() */

void dumpDynamicUnwindInformation( unw_dyn_info_t * unwindInformation, process * ) {
  assert( unwindInformation != NULL );

  Address currentIP = unwindInformation->start_ip;
  unw_dyn_region_info_t * currentRegion = unwindInformation->u.pi.regions;
	
  dyn_unw_printf( "unwind covers: [0x%lx - 0x%lx)\n", unwindInformation->start_ip, unwindInformation->end_ip );
  for( uint32_t i = 0; currentRegion != NULL; currentRegion = currentRegion->next, ++i ) {
	Address nextIP = currentIP + ((currentRegion->insn_count / 3) * 16);
	assert( currentRegion->insn_count % 3 == 0 ); // 0, 1, or 2 is legal, but we only generate bundles.
	dyn_unw_printf( "region %d: [0x%lx - 0x%lx), %d ops", i, currentIP, nextIP, currentRegion->op_count );
	for( unsigned int j = 0; j < currentRegion->op_count; ++j ) {
	  dyn_unw_printf( " %s", unwindOpTagToString( currentRegion->op[j].tag ) );
	}
	dyn_unw_printf( "\n" );
	if( currentRegion->op[currentRegion->op_count - 1].tag != UNW_DYN_STOP ) {
	  fprintf( stderr, "*** format error!  last operation in region not UNW_DYN_STOP!\n" );
	}
	currentIP = nextIP;
  }

			
  if( currentIP != unwindInformation->end_ip ) {
	fprintf( stderr, "*** consistency failure!  last instruction not last IP! (0x%lx != 0x%lx)\n", currentIP, unwindInformation->end_ip );
  }
} /* end dumpDynamicUnwindInformation() */

/* Copies bTDI and its pointed-to regions into the remote process, adjusting the pointers as it goes.
   Assumes that the region's op_count fields are accurate, and only copies over the correct amount. */
bool process::insertAndRegisterDynamicUnwindInformation( unw_dyn_info_t * baseTrampDynamicInfo ) {
  /* This also does a simple consistency check. */
  dumpDynamicUnwindInformation( baseTrampDynamicInfo, this );
  process * proc = this;

  /* Note: assumes no 'handler' routine(s) in baseTrampDynamicInfo->u.pi. */
  Address addressOfbTDI = proc->inferiorMalloc( sizeof( unw_dyn_info_t ) );
  assert( addressOfbTDI != (Address)NULL );
  // dyn_unw_printf( "%s[%d]: address of baseTrampDynamicInfo = 0x%lx\n", __FILE__, __LINE__, addressOfbTDI );
	
  /* Copying the string over every time is wasteful, but simple. */
  Address lengthOfName = strlen( (char *) baseTrampDynamicInfo->u.pi.name_ptr );
  Address addressOfName = proc->inferiorMalloc( lengthOfName + 1 );
  assert( addressOfName != (Address)NULL );
  // dyn_unw_printf( "%s[%d]: address of name = 0x%lx\n", __FILE__, __LINE__, addressOfName );
	
  assert( proc->writeDataSpace( (void *)addressOfName, lengthOfName + 1, (void *)baseTrampDynamicInfo->u.pi.name_ptr ) );
  baseTrampDynamicInfo->u.pi.name_ptr = addressOfName;

  /* Allocate the first region. */
  unw_dyn_region_info_t * nextRegion = baseTrampDynamicInfo->u.pi.regions;
  Address sizeOfNextRegion = _U_dyn_region_info_size( nextRegion->op_count );
  Address addressOfNextRegion = proc->inferiorMalloc( sizeOfNextRegion );
  baseTrampDynamicInfo->u.pi.regions = (unw_dyn_region_info_t *)addressOfNextRegion;
  dyn_unw_printf( "%s[%d]: nextRegion = %p, sizeOfNextRegion = 0x%lx, addressOfNextRegion = 0x%lx\n", __FILE__, __LINE__, nextRegion, sizeOfNextRegion, addressOfNextRegion );

  unw_dyn_region_info_t * currentRegion = nextRegion;	
  Address sizeOfCurrentRegion = sizeOfNextRegion;
  Address addressOfCurrentRegion = addressOfNextRegion;
  dyn_unw_printf( "%s[%d]: currentRegion = %p, sizeOfCurrentRegion = 0x%lx, addressOfCurrentRegion = 0x%lx\n", __FILE__, __LINE__, currentRegion, sizeOfCurrentRegion, addressOfCurrentRegion );	
	
  /* Copy baseTrampDynamicInfo itself over, now that we've updated all of its pointers. */
  assert( proc->writeDataSpace( (void *)addressOfbTDI, sizeof( unw_dyn_info_t ), baseTrampDynamicInfo ) );
	
  /* Iteratively allocate region (n + 1), update region n, and then copy it over. */
  dyn_unw_printf( "%s[%d]: beginning iterations over region list.\n", __FILE__, __LINE__ );
  while( currentRegion->next != NULL ) {
	nextRegion = currentRegion->next;
	sizeOfNextRegion = _U_dyn_region_info_size( nextRegion->op_count );
		
	/* Allocate. */
	addressOfNextRegion = proc->inferiorMalloc( sizeOfNextRegion );
	assert( addressOfNextRegion != (Address)NULL );
		
	/* Update. */
	currentRegion->next = (unw_dyn_region_info_t *)addressOfNextRegion;
		
	/* Copy. */
	dyn_unw_printf( "%s[%d]: (at copy) nextRegion = %p, sizeOfNextRegion = 0x%lx, addressOfNextRegion = 0x%lx\n", __FILE__, __LINE__, nextRegion, sizeOfNextRegion, addressOfNextRegion );
	dyn_unw_printf( "%s[%d]: (at copy) currentRegion = %p, sizeOfCurrentRegion = 0x%lx, addressOfCurrentRegion = 0x%lx\n", __FILE__, __LINE__, currentRegion, sizeOfCurrentRegion, addressOfCurrentRegion );	
	assert( proc->writeDataSpace( (void *)addressOfCurrentRegion, sizeOfCurrentRegion, currentRegion ) );

	/* Iterate. */
	currentRegion = nextRegion;
	sizeOfCurrentRegion = sizeOfNextRegion;
	addressOfCurrentRegion = addressOfNextRegion;
  } /* end region iteration */
	
	/* Copy the n + 1 region. */
  dyn_unw_printf( "%s[%d]: (at last copy) currentRegion = %p, sizeOfCurrentRegion = 0x%lx, addressOfCurrentRegion = 0x%lx\n", __FILE__, __LINE__, currentRegion, sizeOfCurrentRegion, addressOfCurrentRegion );	
  assert( proc->writeDataSpace( (void *)addressOfCurrentRegion, sizeOfCurrentRegion, currentRegion ) );
	
  /* We need the address of the _U_dyn_info_list in the remote process in order
	 to register the baseTrampDynamicInfo. */
  Symbol dyn_info_list;
  if (!proc->getSymbolInfo( "_U_dyn_info_list", dyn_info_list))
	return ! proc->isBootstrappedYet();
  Address addressOfuDIL = dyn_info_list.addr();
	
  /* Register baseTrampDynamicInfo in remote process. */
  dyn_unw_printf( "%s[%d]: registering remote address range [0x%lx - 0x%lx) with gp = 0x%lx with uDIL at 0x%lx\n", __FILE__, __LINE__, baseTrampDynamicInfo->start_ip, baseTrampDynamicInfo->end_ip, baseTrampDynamicInfo->gp, addressOfuDIL );
  registerRemoteUnwindInformation( addressOfbTDI, addressOfuDIL, proc->getPid() );
  return true;
} /* end insertAndRegisterDynamicUnwindInformation() */

/* Required by ast.C */
void emitFuncJump(opCode op, codeGen &gen, const int_function *callee,
				  process *proc, const instPoint *location, bool) {

  assert(op == funcJumpOp);
  IA64_bundle bundle;

  // Remove basetramp's frame on memory stack.
  /* FIXME: we should make use of the unwind_region information here, but this 
	 is the only minitramp which alters the frame. */
  generatePreservationTrailer( gen, location->func()->usedFPregs, NULL );

  int outputRegisters = 8;
  int extraOuts = outputRegisters % 3;
  /* FIXME: the knowledge about NUM_PRESERVED is spread all over creation, and should
	 probably be localized to generatePreservation*(). */
  int offset = regSpace->getRegSlot( 0 )->number + 5;

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
						 : instruction( NOP_I )),
						(extraOuts >= 2 ? generateRegisterToRegisterMove( 33, offset + 1 )
						 : instruction( NOP_I )));
  bundle.generate(gen);

  // Move local regs into output area.
  for (int i = extraOuts; i < outputRegisters; i += 3) {
	bundle = IA64_bundle( MII,
						  generateRegisterToRegisterMove( 32 + (i+0), offset + (i+0) ),
						  generateRegisterToRegisterMove( 32 + (i+1), offset + (i+1) ),
						  generateRegisterToRegisterMove( 32 + (i+2), offset + (i+2) ));
	bundle.generate(gen);
  }

  // Save system state into local registers.  AR.PFS saved earlier by alloc.
  bundle = IA64_bundle( MII,
						generateRegisterToRegisterMove( REGISTER_GP, offset - 4 ),
						generateBranchToRegisterMove( BRANCH_RETURN, offset - 3 ),
						generateBranchToRegisterMove( BRANCH_SCRATCH, offset - 2 ));
  bundle.generate(gen);

  // Ready the callee address.
  Address callee_addr = callee->getAddress();
  bundle = IA64_bundle( MLXstop,
						instruction( NOP_M ),
						generateLongConstantInRegister( offset - 1, callee_addr ));
  bundle.generate(gen);

  // Install GP for callee.
  bundle = IA64_bundle( MLX,
						instruction( NOP_M ),
						generateLongConstantInRegister( REGISTER_GP, proc->getTOCoffsetInfo( callee_addr )));
  bundle.generate(gen);

  // Copy the callee address and jump.
  bundle = IA64_bundle( MIBstop,
						instruction( NOP_M ),
						generateRegisterToBranchMove( offset - 1, BRANCH_SCRATCH ),
						generateIndirectCallTo( BRANCH_SCRATCH, BRANCH_RETURN ));
  bundle.generate(gen);

  // Restore system state.
  bundle = IA64_bundle( MII,
						generateRegisterToRegisterMove( offset - 4, REGISTER_GP ),
						generateRegisterToBranchMove( offset - 3, BRANCH_RETURN ),
						generateRegisterToApplicationMove( 32 + regSpace->originalLocals + regSpace->originalOutputs, AR_PFS ));
  bundle.generate(gen);

  bundle = IA64_bundle( MII,
						(extraOuts >= 1 ? generateRegisterToRegisterMove( offset, 32 )
						 : instruction( NOP_I )),
						(extraOuts >= 2 ? generateRegisterToRegisterMove( offset + 1, 33 )
						 : instruction( NOP_I )),
						generateRegisterToBranchMove( offset - 2, BRANCH_SCRATCH ));
  bundle.generate(gen);

  // Move output regs back into local area.
  for (int i = extraOuts; i < outputRegisters; i += 3) {
	bundle = IA64_bundle( MII,
						  generateRegisterToRegisterMove( offset + (i+0), 32 + (i+0) ),
						  generateRegisterToRegisterMove( offset + (i+1), 32 + (i+1) ),
						  generateRegisterToRegisterMove( offset + (i+2), 32 + (i+2) ));
	bundle.generate(gen);
  }

  // Restore original register frame.
  bundle = IA64_bundle( MstopMIstop,
						instruction( NOP_M ),
						generateOriginalAllocFor( regSpace ),
						instruction( NOP_M ));
  bundle.generate(gen);

  // Ignore rest of tramp, and return directly to original caller.
  bundle = IA64_bundle( MMBstop,
						instruction( NOP_M ),
						instruction( NOP_M ),
						generateReturnTo( BRANCH_RETURN ));
  bundle.generate(gen);
}


/* Required by ast.C */
Register emitR( opCode op, Register src1, Register /*src2*/, Register dest,
				codeGen &gen,  bool /*noCost*/,
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

	  IA64_bundle bundle = IA64_bundle( MstopMIstop,
										generateShortImmediateAdd(dest, memStackOffset, spReg),
										generateRegisterLoad(dest, dest),
										instruction(NOP_M) );
	  bundle.generate(gen);
	  return dest;

	} else {
	  /* Due to register renaming, the requested parameter location depends
		 on the instPoint. */

	  int regStackOffset = 0;
	  if (location->getPointType() == callSite)
		regStackOffset = regSpace->originalLocals;

	  emitRegisterToRegisterCopy( 32 + regStackOffset + src1, dest, gen, NULL );
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
	emitRegisterToRegisterCopy(retVal, dest, gen, NULL);
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
  int_function * func = point->findCallee();
	
  if( !func ) { func = point->func(); }
	
#if defined( USE_DEFAULT_FREQUENCY_TABLE )
  if( ! funcFrequencyTable.defines( func->prettyName() ) ) { return DEFAULT_MAGIC_FREQUENCY; }
  else{ return (float) funcFrequencyTable[ func->prettyName() ]; }
#else
  return DEFAULT_MAGIC_FREQUENCY;
#endif /* defined( USE_DEFAULT_FREQUENCY_TABLE ) */
} /* end getPointFrequency() */

/* Required by inst.C, ast.C */
codeBufIndex_t emitA( opCode op, Register src1, Register /*src2*/, Register dest,
					  codeGen &gen,  bool /*noCost*/ ) {  // FIXME: cost?
  /* Emit the given opcode, returning its relative offset.  For
	 multi-bundle opcodes, return the offset of the branch itself;
	 the code generator will insert the fall-through case directly
	 after this address. */
	
  codeBufIndex_t retval;

  switch( op ) {
  case trampTrailer: {
	retval = gen.getIndex();
	IA64_bundle( MIIstop, NOP_M, NOP_I, NOP_I ).generate(gen);
	break; }

  case ifOp: {
	/* Branch by offset dest if src1 is zero. */
			
	/* See note in eqOp case of emitV() about predicate registers */
	instruction compareInsn = generateComparison( eqOp, src1 % 64, src1, REGISTER_ZERO );
	instruction_x branchInsn = predicateLongInstruction( src1 % 64, generateLongBranchTo( dest ) );
	IA64_bundle compareBundle( MIIstop, compareInsn, NOP_I, NOP_I );
	instruction memoryNOP( NOP_M );
	IA64_bundle branchBundle( MLXstop, memoryNOP, branchInsn );
	compareBundle.generate(gen);
	retval = gen.getIndex();
	branchBundle.generate(gen);
	break; }
			
  case branchOp: {
	/* Why are we passing 64-bit numbers through a Register type? */
	instruction memoryNOP( NOP_M );
	instruction_x branchInsn = generateLongBranchTo( dest );
	IA64_bundle branchBundle( MLXstop, memoryNOP, branchInsn );
	retval = gen.getIndex();
	branchBundle.generate(gen);
  } break;

  default:
	assert( 0 );
	return 0;
  } /* end op switch */
  return retval;
} /* end emitA() */

/* Required by ast.C */
bool doNotOverflow( int /*value*/ ) { 
  /* To be on the safe side, we'll say it always overflows,
	 since it's not clear to me which immediate size(s) are going to be used. */
  return false;
} /* end doNotOverflow() */

/* Required by ast.C */
void emitVstore( opCode op, Register src1, Register src2, Address dest,
				 codeGen &gen,  bool /*noCost*/, registerSpace * rs, int size,
				 const instPoint * location, process * proc) {

  switch( op ) {
  case storeOp: {
	instruction memoryNOP( NOP_M );
	instruction_x loadAddressInsn = generateLongConstantInRegister( src2, dest );
	IA64_bundle loadBundle( MLXstop, memoryNOP, loadAddressInsn );
	loadBundle.generate(gen);

	instruction storeInsn = generateRegisterStore( src2, src1, size );
	IA64_bundle storeBundle( MIIstop, storeInsn, NOP_I, NOP_I );
	storeBundle.generate(gen);
  } break;

  case storeFrameRelativeOp: {
	/* Store size bytes at the address fp + the immediate dest from the register src1,
	   using the scratch register src2. */
	assert( location->func()->framePointerCalculator != NULL );			

	/* framePointerCalculator will leave the frame pointer in framePointer. */
	Register framePointer = (Register) location->func()->framePointerCalculator->generateCode( proc, rs, gen, false, true, location );

	/* See the frameAddr case in emitVload() for an optimization. */
	instruction memoryNop( NOP_M );
	instruction_x loadOffset = generateLongConstantInRegister( src2, dest );
	IA64_bundle loadOffsetBundle( MLXstop, memoryNop, loadOffset );
	instruction computeAddress = generateArithmetic( plusOp, src2, src2, framePointer );
	instruction storeToAddress = generateRegisterStore( src2, src1, size );
	instruction integerNop( NOP_I );
	IA64_bundle computeAndStoreBundle( MstopMIstop, computeAddress, storeToAddress, integerNop );
	loadOffsetBundle.generate(gen);
	computeAndStoreBundle.generate(gen);
	rs->freeRegister( framePointer );
  } break;

  default:
	fprintf( stderr, "emitVstore(): unexpected opcode, aborting.\n" );
	abort();
	break;
  } /* end switch */
} /* end emitVstore() */

/* Required by ast.C */
void emitJmpMC( int /*condition*/, int /*offset*/, codeGen &) { assert( 0 ); }

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

/* For AIX "on the fly" register allocation. */
bool registerSpace::clobberRegister( Register ) {
  return false;
} /* end clobberRegister() */

/* For AIX "on the fly" register allocation. */
bool registerSpace::clobberFPRegister( Register ) {
  return false;
} /* end clobberFPRegister() */

/* For AIX "on the fly" register allocation. */
unsigned saveRestoreRegistersInBaseTramp( process *, baseTramp *, registerSpace * ) {
  return 0;
} /* end saveRestoreRegistersInBaseTramp() */


#if defined( OLD_DYNAMIC_CALLSITE_MONITORING )
bool process::MonitorCallSite( instPoint * callSite ) {
  pdvector< AstNode * > arguments;
#else
  /* This is a really horribly-named function. */
  bool process::getDynamicCallSiteArgs( instPoint * callSite, pdvector<AstNode *> & arguments ) {
#endif
	insn_tmpl tmpl = { callSite->insn().getMachineCode() };
	uint64_t targetAddrRegister = tmpl.B4.b2;			
	// /* DEBUG */ fprintf( stderr, "%s[%d]: monitoring call site for branch register %d\n", __FILE__, __LINE__, targetAddrRegister );
	
	/* This should be the only place on the IA-64 using this poorly-named constant. */
	AstNode * target = new AstNode( AstNode::PreviousStackFrameDataReg, BP_BR0 + targetAddrRegister );
	assert( target != NULL );
	arguments[0] = target;
	
	AstNode * source = new AstNode( AstNode::Constant, callSite->addr() );
	assert( source != NULL );
	arguments[1] = source;

#if defined( OLD_DYNAMIC_CALLSITE_MONITORING )
	AstNode * callToMonitor = new AstNode( "DYNINSTRegisterCallee", arguments );
	assert( callToMonitor != NULL );
	
	miniTramp * mtHandle = NULL;
	addInstFunc(	this, mtHandle, callSite, callToMonitor,
					callPreInsn, orderFirstAtPoint, true, false, true );
#endif
						
	return true;
  } /* end MonitorCallSite() */

  unsigned relocatedInstruction::maxSizeRequired() {
	// This can be pruned to be a better estimate.
	return 48; // bundles. I think. Might be 3.
  }

  bool baseTrampInstance::finalizeGuardBranch( codeGen & gen, int displacement ) {
	/* Displacement is in bytes. */
	IA64_bundle guardOnBundle3( MLXstop,
								instruction( NOP_M ),
								generateLongBranchTo( displacement, GUARD_PREDICATE_TRUE ));
	guardOnBundle3.generate( gen );
	
	return true;
  }

  bool baseTramp::generateSaves( codeGen & gen, registerSpace * rs ) {
	assert( baseTrampRegion == NULL );
	assert( rs != NULL );

	/* Operations for the base tramp proper; this (probably) grossly overestimates the number of operations. */
	baseTrampRegion = (unw_dyn_region_info_t *)malloc( _U_dyn_region_info_size( PRESERVATION_UNWIND_OPERATION_COUNT ) );
	assert( baseTrampRegion != NULL );
	
	baseTrampRegion->insn_count = 0;
	baseTrampRegion->op_count = 0;
	baseTrampRegion->next = NULL;

	/* Determine this instrumentation point's regSpace and deadRegisterList (global variables)
	   for use by the rest of the code generator.  The generatePreservation*() functions need
	   this information as well. */
	bool staticallyAnalyzed = defineBaseTrampRegisterSpaceFor( point(), regSpace, deadRegisterList );
	if( ! staticallyAnalyzed ) {
	  fprintf( stderr, "FIXME: Dynamic determination of register frame required but not yet implemented, aborting.\n" );
	  fprintf( stderr, "FIXME: mutatee instrumentation point 0x%lx\n", point()->addr() );
	  return false;
	} 

	return generatePreservationHeader( gen, point()->func()->usedFPregs, baseTrampRegion );
  } /* end generateSaves() */

  bool baseTramp::generateRestores( codeGen & gen, registerSpace * rs ) {
	assert( baseTrampRegion != NULL );
	assert( rs != NULL );
	
	/* Determine this instrumentation point's regSpace and deadRegisterList (global variables)
	   for use by the rest of the code generator.  The generatePreservation*() functions need
	   this information as well. */
	bool staticallyAnalyzed = defineBaseTrampRegisterSpaceFor( point(), regSpace, deadRegisterList );
	if( ! staticallyAnalyzed ) {
	  fprintf( stderr, "FIXME: Dynamic determination of register frame required but not yet implemented, aborting.\n" );
	  fprintf( stderr, "FIXME: mutatee instrumentation point 0x%lx\n", point()->addr() );
	  return false;
	} 
	
	return generatePreservationTrailer( gen, point()->func()->usedFPregs, baseTrampRegion );
  }  /* end generateRestores() */

  unsigned baseTramp::getBTCost() {
	/* This seems to be defined to return a random constant.  Lovely. */
	return STATE_SAVE_COST + STATE_RESTORE_COST;
  } /* end getBTCost() */

 
  int instPoint::getPointCost() {
	unsigned worstCost = 0;
	for (unsigned i = 0; i < instances.size(); i++) {
      if (instances[i]->multi()) {
		if (instances[i]->multi()->usesTrap()) {
		  // Stop right here
		  // Actually, probably don't want this if the "always
		  // delivered" instrumentation happens
		  return 9000; // Estimated trap cost
		}
		else {
		  worstCost = 105; // Magic constant from before time
		}
      }
      else {
		// No multiTramp, so still free (we're not instrumenting here).
      }
	}
	return worstCost;
  }

  /**
   * Fills in an indirect function pointer at 'addr' to point to 'f'.
   **/
  bool writeFunctionPtr( process *, Address, int_function * ) {
	/* FIXME */ fprintf( stderr, "%s[%d]: FIXME writeFunctionPtr() unimplemented.\n", __FILE__, __LINE__ );
	return true;
  } /* end writeFunctionPtr() */
