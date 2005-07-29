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

// $Id: inst-alpha.C,v 1.89 2005/07/29 19:18:33 bernat Exp $

#include "common/h/headers.h"

#ifdef BPATCH_LIBRARY
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#else
#include "rtinst/h/rtinst.h"
#endif
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/arch-alpha.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/inst-alpha.h"
#include <sys/procfs.h>
#include "dyninstAPI/src/stats.h"
#include "dyninstAPI/src/os.h"
#include <sstream>
#include "miniTramp.h"
#include "multiTramp.h"
#include "baseTramp.h"


/*
   This has not been fully tested.
   The Alpha likes to use jsrs much more than the sparc. 
   Paradyn assumes jsrs are jumps to library functions -- this is incorrect.
   It causes timers to be left on for many functions.  These timers are not 
   supposed to overlap.

   gcc compiles code in an interesting manner for the Alpha.  It creates an 
   alternate entry point for most functions.  If gcc -S is run, the alternate
   entry point has ..NG appended to the function name.  The alternate entry 
   point does not show up in the symbol table.  I don't know the rules for 
   choosing which entry point to use.  The alternate entry point is usually
   two instructions after the advertised entry point.  This code will not work
   with gcc compiled applications.  The runtime must also not be compiled with
   gcc.

   I have seen generated code that uses a register other than 'ra' for the return
   address (a temporary register had been used).  I find this disturbing and played
   it mostly safe when it came to saving registers.  At the base tramp, all registers
   that a callee can clobber are saved.  This is done before and after the relocated
   instruction.  One simple optimization is to avoid doing the save and restore
   unless a slot is used on that side of the relocated instruction.

   I have had to split the DYNINSTdata field into DYNINSTcode and DYNINSTdata.
   The branch displacement is not large enough to reach DYNINSTdata from text space.
   Programs compiled using cc should specify '-taso -non_shared -N'

   The 'pv' register must contain the address of the callee when the callee is called.
   I have not seen any documentation on this.  The API does not mention this.

   'TODO' is used to mark loose ends -- this file does not have too many.
   
   Optimizing branch behavior (not implemented)
   * branch to quadword or octaword aligned addresses
   * forward conditional branches are predicted not taken, take advantage of this

   Optimizing branch behavior (implemented)
   * use least significant 16 bits of JSR for branch prediction

 */

// We call into the RT lib and use save/restore sequences there. This funclet
// wraps these calls

void callRTSequence(codeGen &gen,
		    process *proc,
		    const pdstring &funcname,
		    Register scratchReg) {
  // We still use the call-to-RT model.
  // Get the addr -- proc-specific
  pdvector<int_function *> funcs;
  if (!proc->findFuncsByAll(funcname, funcs))
    assert(0);
  assert(funcs.size() == 1);
  Address calladdr = funcs[0]->getAddress();
  
  int remainder = 0;
  
  instruction::generateAddress(gen, scratchReg, calladdr, remainder);
  if (remainder) {
      instruction::generateLDA(gen, scratchReg, scratchReg, remainder, true);
  }
  instruction::generateJump(gen, scratchReg, MD_JSR, REG_RA, remainder);
}

		    


// HACK: This allows the bootstrap code for loading the dyninst library to
//    skip calling save/restore register code.
bool skipSaveCalls = false;

const char *registerNames[] = { "ren", "stimpy" };
registerSpace *regSpace;
Register regList[] = {1, 2, 3, 4, 5, 6, 7, 8};

pdstring getStrOp(int /* op */) {
  assert(0);
  return pdstring("");
}

Address getMaxBranch() { 
  return ((Address)(1 << 22));
}

bool doNotOverflow(int value)
{
  // we are assuming that we have 8 bits to store the immediate operand.
  if ( (value >= 0) && (value <= 255) ) return(true);
  else return(false);
}


// Restore argument registers (a0-a4) or v0
void restoreRegister(codeGen &gen,
		     Register reg,
		     int dest) {
  instruction::generateLoad(gen,dest,REG_SP,(reg<<3),dw_quad, true);
}

unsigned relocatedInstruction::maxSizeRequired() {
  // We aren't very smart.
  unsigned size = instruction::size();
  if (insn.isCall()) {
    // We fake RA...
    // This should only take 5, but hey.
    size += 6*instruction::size();
  }
  return size;
}

unsigned miniTramp::interJumpSize() {
  return instruction::size();
}

unsigned multiTramp::maxSizeRequired() {
  return instruction::size();
}

bool baseTramp::generateSaves(codeGen &gen,
                              registerSpace *) {
    // decrement stack by 16
  instruction::generateLDA(gen, REG_SP, REG_SP, -16, true);

  // push ra onto the stack
  instruction::generateStore(gen, REG_RA, REG_SP, 0, dw_quad);

  // push GP onto the stack
  instruction::generateStore(gen, REG_GP, REG_SP, 8, dw_quad);

  if (isConservative()) {
    callRTSequence(gen, proc(), "DYNINSTsave_conservative",
		   REG_GP);
  }
  else {
    callRTSequence(gen, proc(), "DYNINSTsave_temp",
		   REG_GP);
  }
  return true;
}

bool baseTramp::generateRestores(codeGen &gen,
                                 registerSpace *) {
  if (isConservative()) {
    callRTSequence(gen, proc(), "DYNINSTrestore_conservative",
		   REG_GP);
  }
  else {
    callRTSequence(gen, proc(), "DYNINSTrestore_temp",
		   REG_GP);
  }

  // load ra from the stack
  instruction::generateLoad(gen, REG_RA, REG_SP, 0, dw_quad, true);

  // load GP from the stack
  instruction::generateLoad(gen, REG_GP, REG_SP, 8, dw_quad, true);

  // increment stack by 16
  instruction::generateLDA(gen, REG_SP, REG_SP, 16, true);

  return true;
}

unsigned baseTramp::getBTCost() {
  // FIXME
  return 0;
}

bool baseTramp::generateMTCode(codeGen &,
                               registerSpace *) {
  // No MT...
  return true;
}

bool baseTramp::generateGuardPreCode(codeGen &,
                                     unsigned &,
                                     registerSpace *) {
  // Don't do that funky guard on Alpha
  return false;
}

bool baseTramp::generateGuardPostCode(codeGen &,
				      codeBufIndex_t &,
                                      registerSpace *) {
  // Don't do the guard on Alpha
  return false;
}

bool baseTramp::generateCostCode(codeGen &,
                                 unsigned &,
                                 registerSpace *) {
  // Skip cost for now...
    return false;
}

void baseTrampInstance::updateTrampCost(unsigned) {
  return;
}

bool baseTrampInstance::finalizeGuardBranch(codeGen &, int) {
  // No guard
  return true;
}
//
// Given and instruction, relocate it to a new address, patching up
// any relative addressing that is present.
//
// Note - currently - only 
//

bool relocatedInstruction::generateCode(codeGen &gen,
					Address baseInMutatee) {
    if (generated_) {
        // We already have code... check to see if it's the same addr
        assert(gen.currAddr(baseInMutatee) == relocAddr);
        gen.moveIndex(size_);
        return true;
    }
  
  unsigned origOffset = gen.used();
  relocAddr = gen.currAddr(baseInMutatee);
  int newOffset = 0;

  if (insn.isBranch()) {
    if (!targetOverride_) {
      newOffset = insn.getTarget(origAddr) - relocAddr;
    }
    else {
      newOffset = targetOverride_ - relocAddr;
    }
    if (ABS(newOffset >> 2) > MAX_BRANCH) {
      fprintf(stderr, "newOffset 0x%llx, origAddr 0x%llx, relocAddr 0x%llx, target 0x%llx, override 0x%llx\n",
	      newOffset, origAddr, relocAddr, insn.getTarget(origAddr), targetOverride_);


      
      logLine("A branch too far\n");
      assert(0);
    }
    instruction newBranch(insn);
    (*newBranch).branch.disp = newOffset >> 2;
    newBranch.generate(gen);
  }
  else 
    insn.generate(gen);

  // Calls are ANNOYING. I've seen behavior where RA (the return addr)
  // is later used in a memory calculation... so after all is said and done,
  // set RA to what it would have been.
  // Note: we do this after the original relocation because JSRs don't
  // trigger "isBranch"
  if (insn.isCall()) {
    Address origReturn = origAddr + instruction::size();
    int remainder = 0;
    instruction::generateAddress(gen, REG_RA, origReturn, remainder);
    if (remainder)
      instruction::generateLDA(gen, REG_RA, REG_RA, remainder, true);
  }
  
  size_ = gen.used() - origOffset;
  return true;
}


unsigned trampEnd::maxSizeRequired() {
    // Return branch, illegal.
    return (2*instruction::size());
}

/* Generate a jump to a base tramp. Return the size of the instruction
   generated at the instrumentation point. */

bool multiTramp::generateBranchToTramp(codeGen &gen)
{
    /* There are three ways to get to the base tramp:
       1. Ordinary jump instruction.
       2. Using a short jump to hit nearby space, and long-jumping to the multiTramp. 
       3. Trap instruction.
       
       We currently support #1.
    */
    
    assert(instAddr_);
    assert(trampAddr_);

    instruction::generateBranch(gen,
				instAddr_,
				trampAddr_);

    gen.fillRemaining(codeGen::cgNOP);

    return true;
}

// Emit a func 64bit address for the call
void callAbsolute(codeGen &gen, 
		  Address funcAddr)
{
  int remainder = 0;
  instruction::generateAddress(gen, REG_GP, funcAddr, remainder);
  if (remainder)
    instruction::generateLDA(gen, REG_GP, REG_GP, remainder, true);
  instruction::generateJump(gen, REG_GP, MD_JSR, REG_RA, remainder);
}
 
/*
 * change the insn at fromAddr to be a branch to newAddr.
 *   Used to add multiple tramps to a point.
 */
unsigned generateAndWriteBranch(process *proc, 
				Address fromAddr, 
				Address newAddr,
                                unsigned fillSize) {
  if (fillSize == 0) fillSize = instruction::size();
  codeGen gen(fillSize);

  instruction::generateBranch(gen, fromAddr, newAddr);
  gen.fillRemaining(codeGen::cgNOP);

  proc->writeTextSpace((caddr_t)fromAddr, gen.used(), gen.start_ptr());
  return gen.used();
}

registerSpace *createRegisterSpace()
{
  return new registerSpace(sizeof(regList)/sizeof(Register), regList,
                                0, NULL);
}

//
// We now generate tramps on demand, so all we do is init the reg space.
//
void initTramps(bool is_multithreaded) {
  static bool init_done=false;

  if (init_done) return;
  init_done = true;

  regSpace = new registerSpace(sizeof(regList)/sizeof(Register), regList,
                               0, NULL, is_multithreaded);
}

#if 0

/*
 * Install a base tramp -- fill calls with nop's for now.
 * An obvious optimization is to turn off the save-restore calls when they 
 * are not needed.
 */
baseTramp *installBaseTramp(instPoint *location,
                                process *proc) 
{
//  unsigned words = 0;
  unsigned long words = 0;
  const int MAX_BASE_TRAMP = 128;
  baseTramp *tramp = new baseTramp(location, proc);
  
  // XXX - for now assume base tramp is less than 1K
  instruction *code = new instruction[MAX_BASE_TRAMP]; assert(code);

  int_function *fun_save;
  int_function *fun_restore;

  Symbol info;
  Address baseAddr;

  if (location->getPointType() == otherPoint) {
      fun_save = proc->findOnlyOneFunction("DYNINSTsave_conservative");
      fun_restore = proc->findOnlyOneFunction("DYNINSTrestore_conservative");
      proc->getSymbolInfo("DYNINSTsave_conservative", info, baseAddr);
  } else {
      fun_save = proc->findOnlyOneFunction("DYNINSTsave_temp");
      fun_restore = proc->findOnlyOneFunction("DYNINSTrestore_temp");
      proc->getSymbolInfo("DYNINSTsave_temp", info, baseAddr);
  }

  assert(fun_save && fun_restore);
  Address dyn_save = fun_save->get_address() + baseAddr;
  Address dyn_restore = fun_restore->get_address() + baseAddr;

  // Pre branch
  instruction *skipPreBranch = &code[words];
  tramp->skipPreInsOffset = words*4;
  words += generate_nop(code+words);

  // local_pre
  tramp->localPreOffset = words * 4;
  words += generate_nop(code+words);

  words += generate_nop(code+words);

  // **** When you change these code also look at emitFuncJump ****
  // Call to DYNINSTrestore_temp
  tramp->localPreReturnOffset = words * 4;

  // *** end code cloned in emitFuncJump ****

  // slot for emulate instruction
  tramp->emulateInsOffset = words*4;
  instruction *reloc = &code[words]; 
  *reloc = location->originalInstruction;
  words += 1;
  // Do actual relocation once we know the base address of the tramp

  // compute effective address of this location
  Address pointAddr = location->pointAddr();
  Address baseAddress;
  if(proc->getBaseAddress(location->getOwner(), baseAddress)){
      pointAddr += baseAddress;
  }

  // If the relocated insn is a Jsr or Bsr then 
  // appropriately set Register Ra
  if (isCallInsn(location->originalInstruction)) {
      int remainder;
      words += generate_address(code+words, REG_RA, pointAddr+4,remainder);
      if (remainder)
	words += generate_lda(code+words, REG_RA, REG_RA, remainder, true);
  }

  // Post branch
  instruction *skipPostBranch = &code[words];
  tramp->skipPostInsOffset = words*4;
  words += generate_nop(code+words);

  // decrement stack by 16
  tramp->savePostInsOffset = words*4;
  words += generate_lda(code+words, REG_SP, REG_SP, -16, true);

  // push ra onto the stack
  words += generate_store(code+words, REG_RA, REG_SP, 0, dw_quad);

  // push GP onto the stack
  words += generate_store(code+words, REG_GP, REG_SP, 8, dw_quad);

  // Call to DYNINSTsave_temp
  callAbsolute(code, words, dyn_save);

  // local_post
  tramp->localPostOffset = words * 4;
  words += generate_nop(code+words);

  // Call to DYNINSTrestore_temp
  tramp->localPostReturnOffset = words * 4;
  callAbsolute(code, words, dyn_restore);

  // load ra from the stack
  words += generate_load(code+words, REG_RA, REG_SP, 0, dw_quad);

  // load GP from the stack
  words += generate_load(code+words, REG_GP, REG_SP, 8, dw_quad);

  // increment stack by 16
  tramp->restorePostInsOffset = words*4;
  words += generate_lda(code+words, REG_SP, REG_SP, 16, true);

  // slot for return (branch) instruction
  // actual code after we know its locations
  // branchFromAddr offset from base until we know base of tramp 
  tramp->returnInsOffset = (words * 4);		
  instruction *branchBack = &code[words];
  words += 1;

  words += generate_nop(code+words);

  assert(words < static_cast<const unsigned>(MAX_BASE_TRAMP));

  tramp->size = words * instruction::size();
  tramp->baseAddr = proc->inferiorMalloc(tramp->size, textHeap, pointAddr);
  assert(tramp->baseAddr);

  // pointAddr + 4 is address of instruction after relocated instr
  // branchFromAddr = address of the branch insn that returns to user code
  // branchFromAddr + 4 is updated pc when branch instruction executes
  // we assumed this one was instruction long before

  // update now that we know base
  Address branchFromAddr = tramp->returnInsOffset + tramp->baseAddr;

  int count = generate_branch(branchBack, REG_ZERO,
			   (pointAddr+4) - (branchFromAddr+4), OP_BR);
  assert(count == 1);

  // Do actual relocation once we know the base address of the tramp
  relocateInstruction(reloc, pointAddr, 
       tramp->baseAddr + tramp->emulateInsOffset);

  // Generate skip pre and post instruction branches
  generateBranchInsn(skipPreBranch,
		     tramp->emulateInsOffset - (tramp->skipPreInsOffset+4));
  generateBranchInsn(skipPostBranch,
		     tramp->returnInsOffset - (tramp->skipPostInsOffset+4));

  tramp->prevInstru = false;
  tramp->postInstru = false;

  proc->writeDataSpace((caddr_t)tramp->baseAddr, tramp->size, (caddr_t) code);
  proc->addCodeRange(tramp->baseAddr, tramp);
  
  delete (code);
  return tramp;
}

#endif

/*
 * emitSaveConservative - generate code to save all registers
 *      used as part of inferrior RPC
 *      We don't know where this will be located, so generate absolute addr
 *          for the function call.
 *
 */
void emitSaveConservative(process *proc, codeGen &gen)
{
  // Matches conservative base tramp... TODO call that function
  // decrement stack by 16
  instruction::generateLDA(gen, REG_SP, REG_SP, -16, true);

  // push T10 onto the stack
  instruction::generateStore(gen, REG_T10, REG_SP, 0, dw_quad);

  // push ra onto the stack
  instruction::generateStore(gen, REG_RA, REG_SP, 8, dw_quad);

  callRTSequence(gen, proc, "DYNINSTsave_conservative", REG_T10);
}

/*
 * emitSaveConservative - generate code to restore all registers
 *      used as part of inferrior RPC
 *      We don't know where this will be located, so generate absolute addr
 *          for the function call.
 *
 */
void emitRestoreConservative(process *proc, codeGen &gen)
{
  callRTSequence(gen, proc, "DYNINSTrestore_conservative", REG_T10);

  // load t10 from the stack
  instruction::generateLoad(gen, REG_T10, REG_SP, 0, dw_quad, true);

  // load ra from the stack
  instruction::generateLoad(gen, REG_RA, REG_SP, 8, dw_quad, true);

  // increment stack by 16
  instruction::generateLDA(gen, REG_SP, REG_SP, 16, true);
}

//
// move the passed parameter into the passed register, or if it is already
//    in a register return the register number.
//
Register getParameter(Register, int param) {
  if (param <= 5) {
    return(16+param);
  }
  assert(0); 
  return(Null_Register);
}

int getInsnCost(opCode op) {

  if (op == loadConstOp) {
    return(1);
  } else if (op ==  loadOp) {
    // assume two cycles to generate address, one to issue load
    return(1+2);
  } else if (op ==  storeOp) {
    // assume two cycles to generate address, one to issue store
    return(1+2);
  } else if (op ==  ifOp) {
    // beq
    return(1);
  } else if (op ==  callOp) {
    //  3  : load address for jsr to DYNINSTsave_misc
    // 12  : cost of DYNINSTsave_misc           
    //  3  : jsr to DYNINSTsave_misc (1 + 2 cycle branch penalty)
    //  3  : load address into register for jsr to called function
    //  1  : move argument to argument register (assume 1 arg on average)
    //  3  : issue jsr to called function (1 + 2 cycle branch penalty)
    //  3  : load address for jsr to DYNINSTrestore_misc
    // 12  : cost of DYNINSTrestore_misc           
    //  3  : jsr to DYNINSTrestore_misc (1 + 2 cycle branch penalty)
    //
    // TODO -- what about the cost of the called code ?
    return 43;
  } else if (op ==  trampPreamble) {
    //  2  : load address of observed_cost
    //  1  : load value of observed_cost
    //  1  : add to observed cost
    //  1  : store updated observed_cost
    return(2+1+1+1);
  } else if (op ==  trampTrailer) {
    // ret
    return(1);
  } else if (op == noOp) {
    // noop
    return(1);
  } else {
    switch (op) {
    case eqOp:
      return 1;                 // 1  : cmpeq
    case neOp:
      return 3;                 // cmpeq, negate result, add 1 to result
    case lessOp:          
    case greaterOp:
    case leOp:
    case geOp:
      return 1;                 // cmpXX, arguments may be reversed
    default:                    // other integer operators
      return(1);
      break;
    }
  }
}

// The Alpha does not have a divide instruction
// The divide is performed in software by calling __divl
// This is actually never used; we assert in ast.C instead.
// Isn't that funny?
int software_divide(int src1,int src2,int dest,
		    codeGen &gen,
		    bool,Address divl_addr,bool Imm)
{
  //int words;
  int remainder;

  // or src1,src1,a0
  instruction::generateOperate(gen, src1, src1, REG_A0, OP_BIS, FC_BIS);
  if (Imm)
    // load constant into register a1
    instruction::generateLitOperate(gen, REG_ZERO, src2, REG_A1, OP_ADDL, FC_ADDL);
  else
    // or src2,src2,a1
    instruction::generateOperate(gen, src2, src2, REG_A1, OP_BIS, FC_BIS);

  // jsr __divl
  instruction::generateAddress(gen, REG_PV,divl_addr, remainder);
  if (remainder)
    instruction::generateLDA(gen, REG_PV, REG_PV, remainder, true);
  instruction::generateJump(gen, REG_PV, MD_JSR, REG_RA, remainder);

  // or v0,v0,dest
  instruction::generateOperate(gen,REG_V0,REG_V0,dest,OP_BIS,FC_BIS);
  return 0;
}

/*
    ra only has to be saved when a procedure call is added using
    instrumentation.
    the Alpha API.
    ra must be saved because it will be clobbered by a procedure call since 
    it will get the return address. 
 */


// The value returned by emitA can be
// 
// ifOp: address to calculate branch
// 
// TODO -- if an offset is returned, it is the offset to the instruction
//         where the branch occurs.  This offset cannot be used to
//         calculate branches, the offset of the next instruction should
//         be used.  The alpha uses the UPDATED pc for branches.
// 

codeBufIndex_t emitA(opCode op, Register src1, Register /*src2*/, Register dest,
	      codeGen &gen, bool /*noCost*/) {

  // Return the index before a jump (if any)
  codeBufIndex_t retval = 0;

  //bperr("emitA(op=%d,src1=%d,src2=XX,dest=%d)\n",op,src1,dest);

  switch (op) {
  case ifOp: {
    // beq src1
    // return address used to calculate branch offset
    assert(src1 < Num_Registers);
    // branch calculations use the updated pc - thus the -4
    retval = gen.getIndex();
    instruction::generateBranch(gen, src1, dest-instruction::size(), OP_BEQ);
    // return offset of branch instruction
    break;
    }
  case branchOp: {
    retval = gen.getIndex();
    instruction::generateBranch(gen, dest-instruction::size());
    break;
    }
  case trampTrailer: {
    // dest is in words of offset and generateBranchInsn is bytes offset
    assert(!dest);
    //    unsigned words = generate_branch(insn, REG_ZERO, dest << 2, OP_BR);
    retval = gen.getIndex();
    instruction::generateBranch(gen, REG_ZERO, dest << 2, OP_BR);
    instruction::generateIllegal(gen);
    // return the offset of the branch instruction -- not the updated pc
    break;
    }
  case trampPreamble: {
    // Don't do this anymore; we update cost once in the base tramp.
    //generate_tramp_preamble(insn, src1, dest, base);
    return(0);          // let's hope this is expected!
  }
  default:
    abort();            // unexpected op for this emit!
  }
  return retval;
}

Register emitR(opCode op, Register src1, Register /*src2*/, Register dest,
               codeGen &gen, bool /*noCost*/,
               const instPoint * /* location */, bool /*for_multithreaded*/) {

  //bperr("emitR(op=%d,src1=%d,src2=XX,dest=%d)\n",op,src1,dest);

  switch (op) {
  case getRetValOp:
    {
      // Return value register v0 is the 12th register saved in the base tramp
      restoreRegister(gen,12,dest);
      return(dest);
    }
  case getParamOp:
    {
      if (src1 >5) assert(0);
      /*
       * We don't save the parameter registers unless we make a function call,
       * so we can read the values directly out of the registers.
       */
      instruction::generateOperate(gen,REG_A0+src1,REG_A0+src1,dest,OP_BIS, FC_BIS);
      return(dest);
    }
  default:
    abort();                    // unexpected op for this emit!
  }
  return(Null_Register);        // should never get here!
}


#ifdef BPATCH_LIBRARY
void emitJmpMC(int /*condition*/, int /*disp*/,codeGen & /*gen*/)
{
  // Not needed for memory instrumentation, otherwise TBD
}

// VG(11/07/01): Load in destination the effective address given
// by the address descriptor. Used for memory access stuff.
void emitASload(const BPatch_addrSpec_NP * /*as*/, unsigned /*dest*/,
		codeGen & /*gen*/, bool /*noCost*/)
{
  // TODO ...
}

void emitCSload(const BPatch_addrSpec_NP *as, unsigned dest, codeGen &gen,
		bool noCost)
{
  emitASload(as, dest, gen, noCost);
}
#endif


void emitVload(opCode op, Address src1, Register, Register dest,
	       codeGen &gen, bool, 
	       registerSpace *,
	       int size,
	       const instPoint * /* location */, process * /* proc */)
{
  if (op == loadConstOp) {
    // lda dest, src1 (LITERAL)     --> put the literal value of src1 into dest
    // it may take several instructions to do this
    assert(dest < Num_Registers);
    int remainder;
    //unsigned long words = 0;
    // if the constant is a zero -- or it into place
    if (!src1) { 
      instruction::generateOperate(gen, REG_ZERO, REG_ZERO, dest,
				   OP_BIS, FC_BIS);
    } else {
      instruction::generateAddress(gen, dest, src1, remainder);
      if (remainder)
	instruction::generateLDA(gen, dest, dest, remainder, true);
    }
    return;
  } else if (op ==  loadOp) {
	// ld? dest, [src1]             --> src1 is a literal
	// src1 = address to load
	// src2 = 
	// dest = register to load
	int remainder;
	instruction::generateAddress(gen, dest, src1, remainder);
	if (size == 1) {
	  instruction::generateLoad(gen, dest, dest, remainder, dw_byte, true);
	} else if (size == 2) {
	  instruction::generateLoad(gen, dest, dest, remainder, dw_word, true);
	} else if (size == 4) {
	  instruction::generateLoad(gen, dest, dest, remainder, dw_long, true);
	} else if (size == 8) {
	  instruction::generateLoad(gen, dest, dest, remainder, dw_quad, true);
	} else {
	    abort();
	}
	return;
    } else if (op == loadFrameRelativeOp) {
      //unsigned long words = 0;
	// frame offset is negative of actual offset.
	long stack_off = (long) src1;

	// the saved sp is 16 less than original sp (due to base tramp code)
	stack_off += 16;		

	assert(ABS(stack_off) < 32767);
	instruction::generateLoad(gen, dest, REG_SP, 112, dw_quad, true);

	instruction::generateLoad(gen, dest, dest, stack_off, dw_long, true);
    } else if (op == loadFrameAddr) {
      //unsigned long words = 0;
	// frame offset is signed.
	long stack_offset = (long) src1;

	// the saved sp is 16 less than original sp (due to base tramp code)
	stack_offset += 16;		

	// load fp into dest
	instruction::generateLoad(gen, dest, REG_SP, 112, dw_quad,true);

	// addd the offset
	instruction::generateLDA(gen, dest, dest, stack_offset, true);
    } else {
	abort();       // unexpected op for this emit!
    }
}

void emitVstore(opCode op, Register src1, Register src2, Address dest,
		codeGen &gen, bool /* noCost */, 
		registerSpace *,
		int size,
		const instPoint * /* location */, process * /* proc */)
{
  if (op ==  storeOp) {
    // st? dest, [src1]
    // src1 = value to store
    // src2 = register to hold address
    // dest = address
    int remainder;
    instruction::generateAddress(gen, src2, dest, remainder);
    if (size == 8) {
      instruction::generateStore(gen, src1, src2, remainder, dw_quad);
    } else if (size == 4) {
      instruction::generateStore(gen, src1, src2, remainder, dw_long);
    } else {
	abort();
    }
    return;
  } else if (op ==  storeFrameRelativeOp) {
    // frame offset is signed.
    // the saved sp is 16 less than original sp (due to base tramp code)
    dest += 16;		

    assert(ABS(dest) < 32767);
    instruction::generateLoad(gen, src2, REG_SP, 112, dw_quad, true);
    if (size == 8) {
      instruction::generateStore(gen, src1, src2, dest, dw_quad);
    } else if (size == 4) {
      instruction::generateStore(gen, src1, src2, dest, dw_long);
    } else {
      abort();
    }
  } else {
    abort();       // unexpected op for this emit!
  }
}
void emitV(opCode op, Register src1, Register src2, Register dest,
	   codeGen &gen, bool /*noCost*/,
	   registerSpace *,
	   int size,
	   const instPoint * /* location */, process * /* proc */)
{
  //bperr("emitV(op=%d,src1=%d,src2=%d,dest=%d)\n",op,src1,src2,dest);

    assert ((op!=branchOp) && (op!=ifOp) && 
            (op!=trampTrailer) && (op!=trampPreamble));         // !emitA
    assert ((op!=getRetValOp) && (op!=getParamOp));             // !emitR
    assert ((op!=loadOp) && (op!=loadConstOp));                 // !emitVload
    assert ((op!=storeOp));                                     // !emitVstore
    assert ((op!=updateCostOp));                                // !emitVupdate

  data_width width;
  if (size == 4) {
    width = dw_long;
  } else if (size == 8) {
    width = dw_quad;
  } else {
    // should not happen
    abort();
  }

  if (op == noOp) {
    instruction::generateNOOP(gen);
    return;
  } else if (op == loadIndirOp) {
    instruction::generateLoad(gen, dest, src1, 0, width, true);
    return;
  } else if (op == storeIndirOp) {
    instruction::generateStore(gen, src1, dest, 0, width);
    return;
  } else {
    instruction::generateIntegerOp(gen, src1, src2, dest, op, false);
    return;
  }
}


/************************************************************************
 * void restore_original_instructions(process* p, instPoint* ip)
************************************************************************/

// The only non read only registers are those that are allocated - t0...t7

bool registerSpace::readOnlyRegister(Register reg_number) {
  if ((reg_number >= REG_T0) && (reg_number <= REG_T7))
    return false;
  else 
    return true;
}

//
// return cost in cycles of executing at this point.  This is the cost
//   of the base tramp if it is the first at this point or 0 otherwise.
//
// This was in inst.C, but the base tramp cost is machine dependent
// 

int instPoint::getPointCost()
{
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
	    worstCost = 81; // Magic constant from before time
          }
      }
      else {
          // No multiTramp, so still free (we're not instrumenting here).
      }
  }
  return worstCost;
}

dictionary_hash<pdstring, unsigned> funcFrequencyTable(pdstring::hash);

void initDefaultPointFrequencyTable()
{
#ifdef notdef
    FILE *fp;
    float value;
    char name[512];

    funcFrequencyTable["main"] = 1;
    funcFrequencyTable["DYNINSTsampleValues"] = 1;
    funcFrequencyTable[EXIT_NAME] = 1;

    // try to read file.
    fp = fopen("freq.input", "r");
    if (!fp) {
        return;
    } else {
        bperr("found freq.input file\n");
    }
    while (!feof(fp)) {
        fscanf(fp, "%s %f\n", name, &value);
        funcFrequencyTable[name] = (int) value;
        bperr("adding %s %f\n", name, value);
    }
    fclose(fp);
#endif
}


float getPointFrequency(instPoint *point)
{

    int_function *func;

    func = point->findCallee();
    if (!func)
        func = point->func();
    
    if (!funcFrequencyTable.defines(func->prettyName())) {
        // Changing this value from 250 to 100 because predictedCost was
        // too high - naim 07/18/96
        return(100);
    } else {
        return (funcFrequencyTable[func->prettyName()]);
    }
}

void emitImm(opCode op, Register src1, RegValue src2imm, Register dest, 
             codeGen &gen, bool /* noCost */, registerSpace *)
{
  instruction::generateIntegerOp(gen, src1, src2imm, dest, op, true);
}

Register
emitFuncCall(opCode /* op */, 
	     registerSpace *rs,
	     codeGen &gen, 
	     pdvector<AstNode *> &operands,
	     process *proc, bool noCost,
	     Address callee_addr,
	     const pdvector<AstNode *> &ifForks,
             const instPoint * /*location*/) // FIXME: pass it!
{
  pdvector <Register> srcs;
  
  // First, generate the parameters
  for (unsigned u = 0; u < operands.size(); u++)
    srcs.push_back(operands[u]->generateCode_phase2(proc, rs, gen,
						    false, ifForks));

  // put parameters in argument registers
  // register move is "bis src, src, dest"  (bis is logical or)
  // save and restore the values currently in the argument registers
  //  unsigned long words = 0;

  void cleanUpAndExit(int status);

  //  Sanity check for NULL address argument
  if (!callee_addr) {
     char msg[256];
     sprintf(msg, "%s[%d]:  internal error:  emitFuncCall called w/out"
             "callee_addr argument", __FILE__, __LINE__);
     showErrorCallback(80, msg);
     assert(0);
   }
 
  int remainder;

  if (!skipSaveCalls) {
    callRTSequence(gen, proc, "DYNINSTsave_misc", REG_T10);
  }

  for (unsigned u=0; u<srcs.size(); u++){
    if (u >= 5) {
       pdstring msg = "Too many arguments to function call in instrumentation code: only 5 arguments can be passed on the alpha architecture.\n";
       fprintf(stderr, msg.c_str());
       showErrorCallback(94,msg);
       cleanUpAndExit(-1);
    }
    instruction::generateOperate(gen, srcs[u], srcs[u], u+REG_A0, 
				 OP_BIS, FC_BIS);
  }

  // Set the value of  pv/t12/r27
  // I have not found any documentation on this
  // But the alpha appears to put the callee's address in this register
  // this register must be saved and restored

  // Jump to the function
  instruction::generateAddress(gen, REG_PV, callee_addr, remainder);
  if (remainder)
    {
      instruction::generateLDA(gen, REG_PV, REG_PV, remainder, true);
    }

  // TODO -- clear other argument registers ?
  instruction::generateJump(gen, REG_PV, MD_JSR, REG_RA, remainder);
  
  if (!skipSaveCalls) {
    callRTSequence(gen, proc, "DYNINSTrestore_misc", REG_T10);
  }

  Register dest = rs->allocateRegister(gen, noCost);

  // or v0,v0,dest
  instruction::generateOperate(gen, REG_V0, REG_V0, dest, OP_BIS, FC_BIS);

  for (unsigned u=0; u<srcs.size(); u++){
    rs->freeRegister(srcs[u]);
  }

  return dest;
}


bool process::replaceFunctionCall(instPoint *point,
				  const int_function *newFunc) {
   // Must be a call site
   if (point->getPointType() != callSite)
      return false;
   
   inst_printf("Function replacement, point func %s, new func %s, point primary addr 0x%x\n",
               point->func()->symTabName().c_str(), newFunc ? newFunc->symTabName().c_str() : "<NULL>",
               point->addr());
   
   instPointIter ipIter(point);
   instPointInstance *ipInst;
   while ((ipInst = ipIter++)) {  
     // Multiple replacements. Wheee...
     Address pointAddr = ipInst->addr();
     inst_printf("... replacing 0x%x", pointAddr);
     codeRange *range;
     if (multiTramps_.find(pointAddr, range)) {
       // We instrumented this... not handled yet
       assert(0);
     }
     else {  
         codeGen gen(instruction::size());
         if (newFunc == NULL) {	// Replace with a NOOP
             inst_printf("... nulling call");
             instruction::generateBranch(gen, REG_RA, 0, OP_BR);
         } else {			// Replace with a new call instruction
             instruction::generateBSR(gen, 
                                      newFunc->getAddress() + instruction::size() - pointAddr);
         }

          // Before we replace, track the code.
          // We could be clever with instpoints keeping instructions around, but
          // it's really not worth it.
          replacedFunctionCall *newRFC = new replacedFunctionCall();
          newRFC->callAddr = pointAddr;

          codeGen old(instruction::size());
          old.copy(point->insn().ptr(), instruction::size());
          newRFC->oldCall = old;
          newRFC->newCall = gen;
          
          replacedFunctionCalls_[pointAddr] = newRFC;

         
         writeTextSpace((caddr_t)pointAddr, gen.used(),
                        gen.start_ptr());
     }
   }
   return true;
}

static const Address lowest_addr = 0x00400000;
void process::inferiorMallocConstraints(Address near, Address &lo, Address &hi,
					inferiorHeapType /*type*/)
{
  if (near)
    {
      // Avoid wrapping issues
      if (near < lowest_addr + MAX_BRANCH)
	lo = lowest_addr;
      else
	lo = near - MAX_BRANCH;
      hi = near + MAX_BRANCH;
    }
  else // near == 0
    {
      lo = lowest_addr;
    }
}

void process::inferiorMallocAlign(unsigned &size)
{
  // quadword-aligned (stack alignment)
  unsigned align = 16;
  if (size % align) size = ((size/align)+1)*align;
}

// Emit code to jump to function CALLEE without linking.  (I.e., when
// CALLEE returns, it returns to the current caller.)
void emitFuncJump(opCode /*op*/, 
		  codeGen &gen, 
		  const int_function *callee, process *proc,
		  const instPoint *, bool)
{
  int remainder = 0;
  Address addr = callee->getAddress();

  callRTSequence(gen, proc, "DYNINSTrestore_temp",
		 REG_GP);
  
  // load ra from the stack
  instruction::generateLoad(gen, REG_RA, REG_SP, 0, dw_quad, true);
  
  // load GP from the stack
  instruction::generateLoad(gen, REG_GP, REG_SP, 8, dw_quad, true);
  
  // increment stack by 16
  instruction::generateLDA(gen, REG_SP, REG_SP, 16, true);
  
  // save gp and ra in special location
  // **** Warning this fails in the case of replacing a mutually recursive
  //    function
  Address saveArea = proc->inferiorMalloc(2*sizeof(long), dataHeap, 0);
  
  instruction::generateAddress(gen, REG_T12, saveArea, remainder);
  instruction::generateStore(gen, REG_GP, REG_T12, remainder, 
			     dw_quad); 
  
  instruction::generateStore(gen, REG_RA, REG_T12, 
			     remainder+sizeof(long), dw_quad);
  
  // calling convention seems to expect t12 to contain the address of the
  //    suboutine being called, so we use t12 to build the address
  remainder = 0;
  instruction::generateAddress(gen, REG_T12, addr, remainder);
  if (remainder)
    instruction::generateLDA(gen, REG_T12, REG_T12, remainder, true);
  instruction::generateJump(gen, REG_T12, MD_JSR, REG_RA, remainder);
  
  instruction::generateNOOP(gen);
  
  // back after function, restore everything
  remainder = 0;
  instruction::generateAddress(gen, REG_RA, saveArea, remainder);
  instruction::generateLoad(gen, REG_GP, REG_RA, remainder, 
			    dw_quad, true); 
  
  instruction::generateLoad(gen, REG_RA, REG_RA, 
			    remainder+sizeof(long), dw_quad, true);
  instruction::generateJump(gen, REG_RA, MD_JMP, REG_ZERO, remainder);
  instruction::generateNOOP(gen);
}

void emitLoadPreviousStackFrameRegister(Address, Register,
					codeGen &, int, bool){
  assert(0);
}
 
bool process::getDynamicCallSiteArgs(instPoint * /*callSite*/,
                                     pdvector<AstNode *> & /*args*/)
{
  return false;
}

#ifdef NOTDEF // PDSEP
bool process::MonitorCallSite(instPoint *callSite){
  return false;
}
#endif

bool registerSpace::clobberRegister(Register /*reg*/) 
{
  return false;
}

bool registerSpace::clobberFPRegister(Register /*reg*/)
{
  return false;
}

unsigned saveRestoreRegistersInBaseTramp(process * /*proc*/, baseTramp * /*bt*/,
					 registerSpace * /*rs*/)
{
  return 0;
}
