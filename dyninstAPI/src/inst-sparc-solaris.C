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

// $Id: inst-sparc-solaris.C,v 1.126 2003/03/21 20:58:31 jodom Exp $

#include "dyninstAPI/src/inst-sparc.h"
#include "dyninstAPI/src/instPoint.h"
#include "common/h/debugOstream.h"

// Needed for function relocation
#include "dyninstAPI/src/func-reloc.h"

#include <sys/utsname.h>
#include <stdlib.h>
#include <strstream.h>

extern bool relocateFunction(process *proc, instPoint *&location);
extern bool branchInsideRange(instruction insn, Address branchAddress, 
                              Address firstAddress, Address lastAddress); 
extern instPoint* find_overlap(pdvector<instPoint*> v, Address targetAddress);
extern void sorted_ips_vector(pdvector<instPoint*>&fill_in);

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


void AstNode::sysFlag(instPoint *location)
{
    if (location -> ipType == functionEntry) {
        astFlag = true;
    } else if (location->ipType == functionExit) {
        astFlag = location->hasNoStackFrame();
    } else {
	astFlag = false;
    }

    if (loperand)
	loperand->sysFlag(location);
    if (roperand)
	roperand->sysFlag(location); 

    for (unsigned u = 0; u < operands.size(); u++) {
	operands[u]->sysFlag(location);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// Determine if the called function is a "library" function or a "user" 
// function. This cannot be done until all of the functions have been 
// seen, verified, and classified.
void pd_Function::checkCallPoints() {
  instPoint *p;
  Address loc_addr;

  //cerr << "pd_Function:: checkCallPoints called, *this = " << *this;

  pdvector<instPoint*> non_lib;

  for (unsigned i=0; i<calls.size(); ++i) {
    /* check to see where we are calling */
    p = calls[i];
    assert(p);

    if (isInsnType(p->firstInstruction, CALLmask, CALLmatch)) {

      loc_addr = p->addr + (p->firstInstruction.call.disp30 << 2);
      pd_Function *pdf = (file_->exec())->findFuncByAddr(loc_addr);

      if (pdf) {
	p->callee = pdf;
	non_lib.push_back(p);

      } else if(!pdf){

	   if((loc_addr < getAddress(0))||(loc_addr > (getAddress(0)+size()))){

	        p->callIndirect = true;
                p->callee = NULL; 
                non_lib.push_back(p);
	   }
	   else {

	       delete p;
	   }
      } 
    } else {

      // Indirect call -- be conservative, assume it is a call to 
      // an unnamed user function
      assert(!p->callee); assert(p->callIndirect);
      p->callee = NULL;
      non_lib.push_back(p);
    }
  }
  calls = non_lib;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// Add the instPoint 'point' to the array of call instPoints

// TODO we cannot find the called function by address at this point in time
// because the called function may not have been seen.
// reloc_info is 0 if the function is not currently being relocated
// Note that this may be called even when instr is NOT a CAlL inst, e.g.
//  in the case of a jmp instruction where paradynd/dyninstAPI knows
//  (really guesses) that control flow will return to the point following
//  the jmp, or wants
//  to mark the jmp as a call to preserve its logical structure of 
//  synchronous call + return (which is violated by tail-call optimization -
//  including a function (w/o stack frame) which ends w/ jmp, nop....
void pd_Function::addCallPoint(const instruction instr, 
				  unsigned &callId,
				  relocatedFuncInfo *reloc_info,
				  instPoint *&point,
				  const instPoint *&location)
{
 
#ifdef DEBUG_CALL_POINTS
    cerr << "pd_Function::addCallPoint called " << endl;
    cerr << " this " << *this << endl;
    cerr << " adr = " << adr << endl;
    cerr << " relocatable_ = " << relocatable_ << endl;
    cerr << " reloc_info = " << reloc_info << endl;
#endif

    if (!isInsnType(instr, CALLmask, CALLmatch)) {
      point->callIndirect = true;
      point->callee = NULL;
    } else{
      point->callIndirect = false;
    }

    if (relocatable_) {

	if (!reloc_info) {
	    calls.push_back(point);
	    calls[callId] -> instId = callId; 
            callId++;

	} else {

#ifdef DEBUG_CALL_POINTS
	    cerr << " *this = " << *this;
	    cerr << " callId = " << callId;
	    cerr << " (u_int)callId = " << (u_int)callId;
	    cerr << " calls.size() = " << calls.size() << endl;
	    cerr << " calls = " << endl;
	    for(unsigned un=0;un<calls.size();un++) {
	        cerr << calls[un] << " , ";
	    }
	    cerr << endl;
#endif

	    assert((callId) < calls.size());
	  
	    if(location && (calls[callId] == location)) { 
		assert(calls[callId]->instId  == location->instId);
		location = point; 
	    } 
	   
	    point->instId = callId++;
	    reloc_info->addFuncCall(point);
	}

    } else {

	if (!reloc_info) {
	    calls.push_back(point);
	} else {
	    reloc_info->addFuncCall(point);
	}
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
 * Given an instruction, relocate it to a new address, patching up
 *   any relative addressing that is present.
 * 
 */
void relocateInstruction(instruction*& insn, Address origAddr, 
                         Address& targetAddr, process *proc)
{
    long long newLongOffset = 0;

    // If the instruction is a CALL instruction, calculate the new
    // offset
    if (isInsnType(*insn, CALLmask, CALLmatch)) {
    
      newLongOffset = origAddr;
      newLongOffset -= targetAddr;
      newLongOffset += (insn->call.disp30 << 2);

      insn->call.disp30 = (int)(newLongOffset >> 2);
    
    } else if (isInsnType(*insn, BRNCHmask, BRNCHmatch)||
	       isInsnType(*insn, FBRNCHmask, FBRNCHmatch)) {

	// If the instruction is a Branch instruction, calculate the 
        // new offset. If the new offset is out of reach after the 
        // instruction is moved to the base Trampoline, we would do
        // the following:
	//    b  address  ......    address: save
	//                                   call new_offset             
	//                                   restore 
	newLongOffset = origAddr;
	newLongOffset -= targetAddr;
	newLongOffset +=  (insn->branch.disp22 << 2);

	// if the branch is too far, then allocate more space in inferior
	// heap for a call instruction to branch target.  The base tramp 
	// will branch to this new inferior heap code, which will call the
	// target of the branch

	if ((newLongOffset < (long long)(-0x7fffffff-1)) ||
	    (newLongOffset > (long long)0x7fffffff) ||
	    !offsetWithinRangeOfBranchInsn((int)newLongOffset)){

	    /** we need to check whether the new allocated space is **/
	    /** in the range of 1 branch instruction from reloacted inst **/

	    unsigned ret = proc->inferiorMalloc(3*sizeof(instruction), textHeap,
					   targetAddr);
	    assert(ret);

	    int old_offset = insn->branch.disp22 << 2;

	    long long assertCheck = ret;
	    assertCheck -= targetAddr;
	    assertCheck = assertCheck >> 2;

	    /** will it really fit to branch offset **/
	    if ((assertCheck < (long long)(-0x1FFFFF-1)) ||
		(assertCheck > (long long)0x1FFFFF))
			assert( 0 && "We could not create nearby space");


	    insn->branch.disp22  = (int)(assertCheck);

	    instruction insnPlus[3];
	    genImmInsn(insnPlus, SAVEop3, REG_SPTR, -112, REG_SPTR);
	    generateCallInsn(insnPlus+1, ret+sizeof(instruction), 
			     origAddr+old_offset);
	    genSimpleInsn(insnPlus+2, RESTOREop3, 0, 0, 0); 
	    proc->writeDataSpace((caddr_t)ret, sizeof(insnPlus), 
			 (caddr_t) insnPlus);
	} else {
	    insn->branch.disp22 = (int)(newLongOffset >> 2);
	}
    } else if (isInsnType(*insn, TRAPmask, TRAPmatch)) {
	// There should be no probelm for moving trap instruction
	// logLine("attempt to relocate trap\n");
    } 
    /* The rest of the instructions should be fine as is */
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void generate_base_tramp_recursive_guard_code( process & p,
					       instruction * code,
					       Address base_addr,
					       NonRecursiveTrampTemplate & templ )
{
  /* prepare guard flag memory, if needed */
  Address guard_flag_address = p.trampGuardAddr();
  
  instruction * curr_instr;
  Address curr_addr;

  /* Assignments: L0 holds the tramp guard addr, L1 holds the loaded value
     and L2 holds the compare/set to value. Oh, and L3 has the shifted POS
     for MT calcs
  */

  /* fill the 'guard on' pre-instruction instrumentation */
  curr_instr = code + templ.guardOnPre_beginOffset / sizeof( instruction );
  curr_addr = base_addr + templ.guardOnPre_beginOffset;
  generateSetHi( curr_instr, guard_flag_address, REG_L(0) );
  curr_instr++; curr_addr += sizeof( instruction );
  if (p.multithread_capable()) {
    int shift_val;
    isPowerOf2(sizeof(unsigned), shift_val);
    generateLShift(curr_instr, REG_MT_POS, (Register)shift_val, REG_L(3));
    curr_instr++; curr_addr += sizeof(instruction);
    genSimpleInsn(curr_instr, ADDop3, REG_L(3), REG_L(0), REG_L(0));
    curr_instr++; curr_addr += sizeof(instruction);
  }    
  genSimpleInsn( curr_instr, ADDop3, REG_G(0), REG_G(0), REG_L(1) );
  curr_instr++; curr_addr += sizeof( instruction );
  generateLoad( curr_instr, REG_L(0), LOW10( guard_flag_address ), REG_L(2) );
  curr_instr++; curr_addr += sizeof( instruction );
  generateStore( curr_instr, REG_L(1), REG_L(0), LOW10( guard_flag_address ) );
  curr_instr++; curr_addr += sizeof( instruction );
  genSimpleInsn( curr_instr, SUBop3cc, REG_L(2), REG_G(0), REG_G(0) );
  curr_instr++; curr_addr += sizeof( instruction );
  int branch_offset_in_bytes =
    ( base_addr + templ.guardOffPre_endOffset )
    -
    curr_addr
    ;
  genBranch( curr_instr,
	     branch_offset_in_bytes,
	     BEcond,
	     false );
  curr_instr++; curr_addr += sizeof( instruction );
  generateNOOP ( curr_instr );

  /* fill the 'guard off' pre-instruction instrumentation */
  curr_instr = code + templ.guardOffPre_beginOffset / sizeof( instruction );
  curr_addr = base_addr + templ.guardOffPre_beginOffset;
  generateSetHi( curr_instr, guard_flag_address, REG_L(0) );
  curr_instr++; curr_addr += sizeof( instruction );
  // We should store this value... but where?
  if (p.multithread_capable()) {
    int shift_val;
    isPowerOf2(sizeof(unsigned), shift_val);
    generateLShift(curr_instr, REG_MT_POS, (Register)shift_val, REG_L(3));
    curr_instr++; curr_addr += sizeof(instruction);
    genSimpleInsn(curr_instr, ADDop3, REG_L(3), REG_L(0), REG_L(0));
    curr_instr++; curr_addr += sizeof(instruction);
  }    

  genImmInsn( curr_instr, ADDop3, REG_G(0), 1, REG_L(1) );
  curr_instr++; curr_addr += sizeof( instruction );
  generateStore( curr_instr, REG_L(1), REG_L(0), LOW10( guard_flag_address ) );

  /* fill the 'guard on' post-instruction instrumentation */
  curr_instr = code + templ.guardOnPost_beginOffset / sizeof( instruction );
  curr_addr = base_addr + templ.guardOnPost_beginOffset;
  generateSetHi( curr_instr, guard_flag_address, REG_L(0) );
  curr_instr++; curr_addr += sizeof( instruction );

  if (p.multithread_capable()) {
    int shift_val;
    isPowerOf2(sizeof(unsigned), shift_val);
    generateLShift(curr_instr, REG_MT_POS, (Register)shift_val, REG_L(3));
    curr_instr++; curr_addr += sizeof(instruction);
    genSimpleInsn(curr_instr, ADDop3, REG_L(3), REG_L(0), REG_L(0));
    curr_instr++; curr_addr += sizeof(instruction);
  }    

  genSimpleInsn( curr_instr, ADDop3, REG_G(0), REG_G(0), REG_L(1) );
  curr_instr++; curr_addr += sizeof( instruction );
  generateLoad( curr_instr, REG_L(0), LOW10( guard_flag_address ), REG_L(2) );
  curr_instr++; curr_addr += sizeof( instruction );
  generateStore( curr_instr, REG_L(1), REG_L(0), LOW10( guard_flag_address ) );
  curr_instr++; curr_addr += sizeof( instruction );
  genSimpleInsn( curr_instr, SUBop3cc, REG_L(2), REG_G(0), REG_G(0) );
  curr_instr++; curr_addr += sizeof( instruction );
  branch_offset_in_bytes =
    ( base_addr + templ.guardOffPost_endOffset )
    -
    curr_addr
    ;
  genBranch( curr_instr,
	     branch_offset_in_bytes,
	     BEcond,
	     false );
  curr_instr++; curr_addr += sizeof( instruction );
  generateNOOP ( curr_instr );

  /* fill the 'guard off' post-instruction instrumentation */
  curr_instr = code + templ.guardOffPost_beginOffset / sizeof( instruction );
  curr_addr = base_addr + templ.guardOffPost_beginOffset;
  generateSetHi( curr_instr, guard_flag_address, REG_L(0) );
  curr_instr++; curr_addr += sizeof( instruction );

  if (p.multithread_capable()) {
    int shift_val;
    isPowerOf2(sizeof(unsigned), shift_val);
    generateLShift(curr_instr, REG_MT_POS, (Register)shift_val, REG_L(3));
    curr_instr++; curr_addr += sizeof(instruction);
    genSimpleInsn(curr_instr, ADDop3, REG_L(3), REG_L(0), REG_L(0));
    curr_instr++; curr_addr += sizeof(instruction);
  }    

  genImmInsn( curr_instr, ADDop3, REG_G(0), 1, REG_L(1) );
  curr_instr++; curr_addr += sizeof( instruction );
  generateStore( curr_instr, REG_L(1), REG_L(0), LOW10( guard_flag_address ) );
}

//
// For multithreaded applications and shared memory sampling, this routine 
// will compute the address where the corresponding counter/timer vector for
// level 0 is (by default). In the mini-tramp, if the counter/timer is at a
// different level, we will add the corresponding offset - naim 4/18/97
//
// NUM_INSN_MT_PREAMBLE
void generateMTpreamble(char *insn, Address &base, process *proc)
{
  AstNode *threadPOS;
  Address returnVal;
  pdvector<AstNode *> dummy;
  bool err;
  Register src = Null_Register;

  // registers cleanup
  regSpace->resetSpace();

  /* Get the hashed value of the thread */
  if (!proc->multithread_ready()) {
    // Uh oh... we're not ready to build a tramp yet!
    cerr << "WARNING: tramp constructed without RT multithread support!" << endl;
    threadPOS = new AstNode("DYNINSTreturnZero", dummy);
  }
  else 
    threadPOS = new AstNode("DYNINSTthreadPos", dummy);
  src = threadPOS->generateCode(proc, regSpace, (char *)insn,
				base, 
				false, // noCost 
				true); // root node
  if ((src) != REG_MT_POS) {
    // This is always going to happen... we reserve REG_MT_POS, so the
    // code generator will never use it as a destination
    emitV(orOp, src, 0, REG_MT_POS, insn, base, false);
  }
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
 * Install a base tramp -- fill calls with nop's for now.
 *
 * This one install the base tramp for the regular functions.
 *
 */
trampTemplate * installBaseTramp( instPoint * & location,
				  process * proc,
                                  trampTemplate *&current_template,
                                  Address baseTrampAddress,
				  bool trampRecursiveDesired = false )
{
    Address ipAddr = 0;
    Address baseAddress = 0;
    Address currAddr;
    instruction *temp;
    // Records the number of instructions copied over to the base trampoline
    int numInsnsCopied = 0;

    // Get the base address of the so 
    proc->getBaseAddress( location->image_ptr, baseAddress );

    // Determine the address of the instPoint
    ipAddr = location->iPgetAddress() + baseAddress;

    // Determine if the base trampoline is within the range of a branch
    // instruction from the instrumentation point. This is used to determine
    // How we transfer between the function and instrumentation, as well
    // as how many instructions get copied to the base trampoline 
    if ( !offsetWithinRangeOfBranchInsn(baseTrampAddress - ipAddr) ) {
      location->needsLongJump = true;
    }

//     if(location->dontUseCall)
//       fprintf(stderr, "dontUseCall@%p\n", ipAddr);

    // VG(4/25/2002): refuse installation if call is needed, but not safe
    if(location->needsLongJump && location->dontUseCall)
      return NULL;

    // very conservative installation as o7 can be live 
    // at this arbitrary inst point
    if( (location->ipType == otherPoint) &&
         location->func && location->func->is_o7_live() &&
         location->needsLongJump)
    {
        // Free up the space allocated for the base tramp
	proc->inferiorFree(baseTrampAddress);
        location->needsLongJump = false;

        // Creation of base tramp failed
	return NULL;
    }

    // Generate a buffer for writing the base tramp to
    instruction * code = new instruction[ current_template->size ];
    assert( code );

    // Set up the template of the base trampoline in the buffer
    memcpy( (char *)code,
	    (char *)current_template->trampTemp,
	    current_template->size );



    // Iterate over the trampoline template buffer, filling in the details
    for (temp = code, currAddr = baseTrampAddress; 
	(currAddr - baseTrampAddress) < (unsigned) current_template->size;
	temp++, currAddr += sizeof(instruction)) {

      unsigned offset;
      Address numIns;
      Address retAddress;

      switch (temp->raw) {
      case EMULATE_INSN:
	// Branch instruction (i.e.  ba, a) will do for transfering 
	// between function and base trampoline, thus only one 
	// instruction needs to be copied to the base tramp. (unless
	// of course that instruction is a DCTI).
	if ( !location->needsLongJump ) {
	  
	  // Need to copy the first instruction to the base trampoline
	  *temp = location->firstInstruction;
	  relocateInstruction(temp, ipAddr, currAddr, proc);
	  numInsnsCopied = 1;
	  
	  // If the instruction that was copied over to the 
	  // base tramp was a DCTI, then we need to copy over the
	  // instruction in its delay slot, and perhaps also an
	  // aggregate instruction. (An aggregate instruction is a
	  // byte that follows the delay slot of a call, where the 
	  // called function returns a structure, and the aggregate
	  // instruction records the size of the structure returned).
	  
	  if ( location->firstIsDCTI ) {
	    
	    // copy delay slot instruction to trampoline
	    currAddr += sizeof(instruction);  
	    *++temp = location->secondInstruction;
	    numInsnsCopied = 2;
	  }
	  
	  if ( location->thirdIsAggregate ) {
	    
	    // copy invalid insn with aggregate size in it to
	    // base trampoline
	    currAddr += sizeof(instruction);  
	    *++temp = location->thirdInstruction;
	    numInsnsCopied = 3;
	  }
	  
	  // call sequence is needed to transfer to base trampoline
	} else {
	  
	  // For arbitrary instPoints. call; nop; used to transfer 
	  if (location->ipType == otherPoint) {
	    
	    // relocate the FIRST instruction
	    *temp = location->firstInstruction;
	    relocateInstruction(temp, ipAddr, currAddr, proc);
	    numInsnsCopied = 1;
	    
	    
	    // relocate the SECOND instruction
	    temp++;
	    currAddr += sizeof(instruction);
	    *temp = location->secondInstruction;
	    relocateInstruction(temp,
				ipAddr + 1*sizeof(instruction), 
				currAddr, proc);
	    numInsnsCopied = 2;
	    
	    // if the SECOND instruction is a DCTI, then the 
	    // third instruction needs to be copied, and the fourth 
	    // instruction may be an aggregate 
	    if (location->secondIsDCTI) {
	      
	      // relocate the third instruction
	      temp++;
	      currAddr += sizeof(instruction);
	      *temp = location->thirdInstruction;
	      relocateInstruction(temp, 
				  ipAddr + 2*sizeof(instruction),
				  currAddr, proc);
	      numInsnsCopied = 3;
	      
	      if (location->fourthIsAggregate) {
		
		// relocate the FOURTH instruction
		temp++;
		currAddr += sizeof(instruction);
		*temp = location->fourthInstruction;
		relocateInstruction(temp, 
				    ipAddr + 3*sizeof(instruction),
				    currAddr, proc);
		numInsnsCopied = 4;
	      }
	    }
	    
	  } else {
	    
	    // the instructions prior to the instPoint don't need to 
	    // be used
	    if ( !location->usesPriorInstructions ) {
	      
	      
	      // For leaf functions
	      if (location->hasNoStackFrame()) {
		
		// relocate the FIRST instruction
		*temp = location->firstInstruction;
		relocateInstruction(temp,
				    ipAddr, 
				    currAddr, proc);
		numInsnsCopied = 1;
		
		
		// relocate the SECOND instruction
		temp++;
		currAddr += sizeof(instruction);
		*temp = location->secondInstruction;
		relocateInstruction(temp,
				    ipAddr + 1*sizeof(instruction), 
				    currAddr, proc);
		numInsnsCopied = 2;
		
		
		// relocate the THIRD instruction
		temp++;
		currAddr += sizeof(instruction);
		*temp = location->thirdInstruction;
		relocateInstruction(temp, 
				    ipAddr + 2*sizeof(instruction),
				    currAddr, proc);
		numInsnsCopied = 3;
		
		
		// if the THIRD instruction is a DCTI, then the fourth
		// instruction must be copied to the base tramp as well 
		if (location->thirdIsDCTI) {
		  
		  // Copy the instruction in the delay slot to the 
		  // base trampoline
		  temp++;
		  currAddr += sizeof(instruction);
		  *temp = location->fourthInstruction;
		  relocateInstruction(temp,
				      ipAddr + 3*sizeof(instruction),
				      currAddr, proc);
		  temp++;
		  numInsnsCopied = 4;
		}
		
		// Function has a stack frame
	      } else {
		
		// No need for a save instruction, only need to copy
		// the original instruction and the delay slot insn
		if ( location->ipType == callSite || 
		     location->ipType == functionExit ) {
		  
		  // relocate the FIRST instruction
		  *temp = location->firstInstruction;
		  relocateInstruction(temp,
				      ipAddr, 
				      currAddr, proc);
		  temp++;
		  currAddr += sizeof(instruction);
		  numInsnsCopied = 1;
		  
		  // relocate the SECOND instruction
		  *temp = location->secondInstruction;
		  relocateInstruction(temp,
				      ipAddr + 1*sizeof(instruction), 
				      currAddr, proc);
		  numInsnsCopied = 2;
		  
		  
		  // since the FIRST instruction is a DCTI, the 
		  // third instruction may be an aggregate
		  assert (location->firstIsDCTI);
		  
		  if (location->thirdIsAggregate) {
		    
		    // relocate the THIRD instruction
		    temp++;
		    currAddr += sizeof(instruction);
		    *temp = location->thirdInstruction;
		    relocateInstruction(temp, 
					ipAddr + 2*sizeof(instruction),
					currAddr, proc);
		    numInsnsCopied = 3;
		  }
		  
		} else {
		  
		  assert (location->ipType == functionEntry);
		  
		  // relocate the FIRST instruction
		  *temp = location->firstInstruction;
		  relocateInstruction(temp, ipAddr, currAddr, proc);
		  temp++;
		  currAddr += sizeof(instruction);
		  numInsnsCopied = 1;
		  
		  // relocate the SECOND instruction
		  *temp = location->secondInstruction;
		  relocateInstruction(temp,
				      ipAddr + 1*sizeof(instruction),
				      currAddr, proc);
		  numInsnsCopied = 2;
		  
		  
		  // relocate the THIRD instruction
		  temp++;
		  currAddr += sizeof(instruction);
		  *temp = location->thirdInstruction;
		  relocateInstruction(temp, 
				      ipAddr + 2*sizeof(instruction),
				      currAddr, proc);
		  numInsnsCopied = 3;
		  
		  
		  // if the SECOND instruction is a DCTI, then the 
		  // fourth instruction may be an aggregate 
		  if (location->secondIsDCTI) {
		    
		    if (location->fourthIsAggregate) {
		      
                                // relocate the FOURTH instruction
		      temp++;
		      currAddr += sizeof(instruction);
		      *temp = location->fourthInstruction;
		      relocateInstruction(temp, 
					  ipAddr + 3*sizeof(instruction),
					  currAddr, proc);
		      numInsnsCopied = 4;
		    }
		  }
		  
		  // if the THIRD instruction is a DCTI, then the 
		  // fourth instruction is in the delay slot and the
		  // fifth instruction may be an aggregate (for 
		  // entry and other points only). 
		  if (location->thirdIsDCTI) {
		    
		    // relocate the FOURTH instruction
		    temp++;
		    currAddr += sizeof(instruction);
		    *temp = location->fourthInstruction;
		    relocateInstruction(temp, 
					ipAddr + 3*sizeof(instruction),
					currAddr, proc);
		    numInsnsCopied = 4;
		    
		    // Then, possibly, there's a callAggregate
		    // instruction after this. 
		    if (location->fifthIsAggregate) {
		      
                                // relocate the FIFTH instruction
		      temp++;
		      currAddr += sizeof(instruction);
		      *temp = location->fifthInstruction;
		      relocateInstruction(temp, 
					  ipAddr + 4*sizeof(instruction),
					  currAddr, proc);
		      
		      numInsnsCopied = 5;
		    }
		  }
		}
	      }
	      
	      // the instructions prior to the instPoint need to be used
	    } else {
	      
	      // At the exit of leaf function, we need to over-write
	      // three instructions for a save; call; restore; sequence
	      // but we only have two instructions. So we claim the
	      // the instructions preceding the exit point
	      assert(location->hasNoStackFrame());
	      assert(location->ipType == functionExit);
	      
	      numInsnsCopied = 2;
	      
	      // 'firstPriorInstruction' is the aggregate after a call,
	      // so copy the two instructions that precede it.
	      if (location->numPriorInstructions == 3) {
		
		currAddr += sizeof(instruction);
		*++temp = location->thirdPriorInstruction;
		relocateInstruction(temp, ipAddr, currAddr, proc);
		
		currAddr += sizeof(instruction);
		*++temp = location->secondPriorInstruction;
		
	      }
	      
	      // 'firstPriorInstruction' is the delay slot of a call,
	      // so copy the instruction that precedes it.
	      if (location->numPriorInstructions == 2) {
		
		currAddr += sizeof(instruction);
		*++temp = location->secondPriorInstruction;
		relocateInstruction(temp, ipAddr, currAddr, proc);
		
	      }
	      
	      currAddr += sizeof(instruction);
	      *++temp = location->firstPriorInstruction;
	      
	      currAddr += sizeof(instruction);
	      *++temp = location->firstInstruction;
	      
	      currAddr += sizeof(instruction);
	      *++temp = location->secondInstruction;
	      
	      // Well, after all these, another SAVE instruction is 
	      // generated so we are prepared to handle the returning 
	      // to our application's code segment.
	      genImmInsn(temp+1, SAVEop3, REG_SPTR, -120, REG_SPTR);
	    }
	  }
	  
	}
	break;
      case RETURN_INSN:
	
	if ( offsetWithinRangeOfBranchInsn((ipAddr + 
					    numInsnsCopied*sizeof(instruction)) - 
					   currAddr) ) {
	  
	  generateBranchInsn(temp, ipAddr + 
			     numInsnsCopied*sizeof(instruction) - 
			     currAddr);
	  
	} else {
	  
	  retAddress =  baseAddress + 
	    location->iPgetAddress() + 
	    numInsnsCopied*sizeof(instruction);
	  
	  if (location->func->hasNoStackFrame() ||
	      ((location->ipType == otherPoint) &&
	       location->func->is_o7_live())){
		
	      /* to save value of live o7 register we save and call*/
	      genImmInsn(temp, SAVEop3, REG_SPTR, -120, REG_SPTR);
	      generateCallInsn(temp + 1, 
			       currAddr + sizeof(instruction), 
			       retAddress);
	      genImmInsn(temp+2, RESTOREop3, 0, 0, 0);
	      
	  } else {
	      generateCallInsn(temp, currAddr, retAddress);
	  }
	}
	break;
      case SKIP_PRE_INSN:
	offset = baseTrampAddress+current_template->updateCostOffset-currAddr;
	generateBranchInsn(temp,offset);
	break;
      case SKIP_POST_INSN:
	offset = baseTrampAddress+current_template->returnInsOffset-currAddr;
	generateBranchInsn(temp,offset);
	break;
      case UPDATE_COST_INSN:
	current_template->costAddr = currAddr;
	generateNOOP(temp);
	break;
      case LOCAL_PRE_BRANCH:
      case LOCAL_POST_BRANCH:
	generateNOOP(temp);
	break;
      case MT_POS_CALC:
	if (proc->multithread_capable()) {
	  numIns = 0;
	  generateMTpreamble((char *)temp, numIns, proc);
	  //temp += (numIns/sizeof(instruction));
	}
	else
	  generateNOOP(temp); /* Not yet ready for MT tramp */
	break;
      case RECURSIVE_GUARD_ON_PRE_INSN:
      case RECURSIVE_GUARD_OFF_PRE_INSN:
      case RECURSIVE_GUARD_ON_POST_INSN:
      case RECURSIVE_GUARD_OFF_POST_INSN:
	generateNOOP( temp );
	break;
      default: /* May catch an unknown instruction (or a noop) */
	break;
      }
    }
    
    if( ! trampRecursiveDesired ) {
      generate_base_tramp_recursive_guard_code( *proc, code, baseTrampAddress,
						(NonRecursiveTrampTemplate &) *current_template);
    }
    /*
    fprintf(stderr, "------------\n");
    for (int i = 0; i < (current_template->size/4); i++)
      fprintf(stderr, "0x%x,\n", code[i].raw);
    fprintf(stderr, "------------\n");
    fprintf(stderr, "\n\n\n");
    */

    // TODO cast
    proc->writeDataSpace( ( caddr_t )baseTrampAddress,
			  current_template->size,
			  ( caddr_t )code );
    delete [] code;

    trampTemplate * baseInst;
    if( trampRecursiveDesired )
      {
	baseInst = new trampTemplate;
      }
    else
      {
	baseInst = new NonRecursiveTrampTemplate;
      }
    * baseInst = *current_template;
    baseInst->baseAddr = baseTrampAddress;

    return baseInst;
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
 * Allocate the space for the base Trampoline, and generate the instruction
 * we need for modifying the code segment.
 *
 * 'retInstance' tells you how to modify the code to jump to the base tramp
 *
 */
trampTemplate *findAndInstallBaseTramp(process *proc, 
				       instPoint *&location,
				       returnInstance *&retInstance,
				       bool trampRecursionDesired,
				       bool, 
                                       bool &deferred)
{
    Address baseAddress;
    Address ipAddr;
    Address baseTrampAddress = 0;

    retInstance = NULL;

    const instPoint *&cLocation = const_cast<const instPoint *&>(location);
 
    trampTemplate *ret;

    if (proc->baseMap.find(cLocation, ret)) {

       // This base tramp already exists; nothing to do.
       return ret;
    }


    // Generate the template for the trampoline
    trampTemplate *current_template = &nonRecursiveBaseTemplate;

    if(location->ipType == otherPoint) {
      current_template = &nonRecursiveConservativeBaseTemplate;
    }

    if( trampRecursionDesired ) {
      current_template = &baseTemplate;

      if(location->ipType == otherPoint) {
        current_template = &conservativeBaseTemplate;
      }
    }


    // Get the base address of the shared object 
    proc->getBaseAddress( location->image_ptr, baseAddress );

    // Determine the address of the instPoint
    ipAddr = location->iPgetAddress() + baseAddress;


    // For functions that MAY need relocation, check to see if the 
    // base tramp is within the range of a branch of the instPoint.
    // If the base tramp is too far away, force relocation, and rewrite
    // the function so that a save; call; restore; sequence can be used
    // to transfer to the base trampoline.
    if ( location->func->mayNeedRelocation() ) { 

      // Allocate space for the base trampoline
      baseTrampAddress = proc->inferiorMalloc(current_template->size, 
					      textHeap, ipAddr);
      assert( baseTrampAddress );

      // Determine if the base trampoline is within the range of a branch
      // instruction from the instrumentation point. This is used to determine
      // How we transfer between the function and instrumentation, as well
      // as how many instructions get copied to the base trampoline
      if ( !offsetWithinRangeOfBranchInsn(baseTrampAddress - ipAddr) ) {

        // The function needs to be relocated to be instrumentable
        location->func->setRelocatable(true);

        // Free up the space allocated for the base tramp
        // (Since we are relocating the function, we want to allocate
        // the base tramp near the NEW instPoint location).
	proc->inferiorFree(baseTrampAddress);
        baseTrampAddress = 0;

      }
    }


    // Relocate the function if needed 
    if (location->func->needsRelocation()) {

#ifdef BPATCH_LIBRARY
      const BPatch_point *old_bppoint = location->getBPatch_point();
#endif
      if(!(location->func->isInstalled(proc))) {

        // Relocate the function
        bool relocated = location->func->relocateFunction(proc, location, 
                                                                deferred);

        // Unable to relocate function
        if (relocated == false) {
          return NULL;
	}
#ifdef BPATCH_LIBRARY
	location->setBPatch_point(old_bppoint);
#endif

      } else {

          if(!location->relocated_){

            // need to find new instPoint for location...
            // it has the pre-relocated address of the instPoint
            location->func->modifyInstPoint(cLocation,proc);
#ifdef BPATCH_LIBRARY
	    location->setBPatch_point(old_bppoint);
#endif
	  }
      }
    }


    // Grab the address of the instPoint (this may have changed if the
    // function was relocated).
    ipAddr = location->iPgetAddress() + baseAddress;


    // Allocate space for the base tramp if it has not been allocated yet
    if (baseTrampAddress == 0) {

      // Allocate space for the base trampoline
      baseTrampAddress = proc->inferiorMalloc(current_template->size, 
					      textHeap, ipAddr);
      assert( baseTrampAddress );
    }


    // Set up the base tramp.
    ret = installBaseTramp(location, proc, 
                           current_template, 
                           baseTrampAddress, 
                           trampRecursionDesired);
    if(!ret) return NULL;


    // If the base trampoline is within the range of a branch instruction
    // from the function, use a branch to transfer control from the 
    // function to the base tramp
    if ( !location->needsLongJump ) {

      // Place branch at the exact location of the instruction to be 
      // instrumented (insnAddress()), rather than at the
      // first Address of the instPoint's footprint, iPgetAddress(),
      // (these may differ).
      // The latter address is used when we use a call instruction 
      // instead of a branch to get to the base tramp.
      Address branchAddress = location->insnAddress() + baseAddress;

      instruction *insn = new instruction;
      generateBranchInsn(insn, ret->baseAddr - branchAddress);
      retInstance = new returnInstance(1, (instructUnion *)insn,
                                       1 * sizeof(instruction), branchAddress, 
                                       1 * sizeof(instruction));
    } else {


        if ( location->ipType == otherPoint ) {

          // Generate call; nop; sequence
          // (no need to generate a save since instrumentation will only go
          // in if the o7 register is not live
	  instruction *insn = new instruction[2];

          // Generate the call instruction over the ret
	  generateCallInsn(insn, ipAddr, ret->baseAddr);

          // Generate the nop over the restore
	  generateNOOP(insn+1);

	  retInstance = new returnInstance(2, (instructUnion *)insn, 
	              		           2*sizeof(instruction), 
                                           ipAddr, 
			 	           2*sizeof(instruction));
	} else {

          // For functions that have no stack frame
          if (location->hasNoStackFrame()) {

            // Generate a save; call; nop sequence
            instruction *insn = new instruction[3];

            // The address the save will be written to
            Address saveAddr = ipAddr;

            // If prior instructions need to be claimed, determine the
            // address of the first instruction that will be moved to the
            // base tramp (and thus replaced by the save).
            if (location->usesPriorInstructions) {

              saveAddr = 
                ipAddr - location->numPriorInstructions * sizeof(instruction);
	    }

            // Generate the save
	    genImmInsn(insn, SAVEop3, REG_SPTR, -120, REG_SPTR);

            // Generate the call
	    generateCallInsn(insn + 1, 
                             saveAddr + sizeof(instruction), 
                             ret->baseAddr);

            // Generate the restore
	    genImmInsn(insn + 2, RESTOREop3, 0, 0, 0);

    	    retInstance = new returnInstance(3, (instructUnion *)insn, 
	               		             3 * sizeof(instruction), 
                                             saveAddr, 
					     3 * sizeof(instruction));

          // Not a leaf function
	  } else {

	    if ( location->ipType == callSite || 
                 location->ipType == functionExit ) {

                // Generate call; nop; sequence
                // (no need to generate a save since the o7 register is 
                // going to be clobbered by the ret; restore).
	        instruction *insn = new instruction[2];

                // Generate the call instruction over the ret
	        generateCallInsn(insn, ipAddr, ret->baseAddr);

                // Generate the nop over the restore
	        generateNOOP(insn+1);

	        retInstance = new returnInstance(2, (instructUnion *)insn, 
		        		         2*sizeof(instruction), 
                                                 ipAddr, 
			 		         2*sizeof(instruction));

	    } else if ( location->ipType == functionEntry ) {

                // Generate save; call; restore; sequence
                instruction *insn = new instruction[3];

                // Generate the save
                genImmInsn(insn, SAVEop3, REG_SPTR, -120, REG_SPTR);

                // Generate the call
                generateCallInsn(insn + 1, 
                                 ipAddr + sizeof(instruction), 
                                 ret->baseAddr);

                // Generate the restore
                genImmInsn(insn + 2, RESTOREop3, 0, 0, 0);

                retInstance = new returnInstance(3, (instructUnion *)insn, 
                                                 3 * sizeof(instruction), 
                                                 ipAddr, 
                                                 3 * sizeof(instruction));
	    }
	  }
	}
    }

    assert(retInstance);
    proc->baseMap[(const instPoint *)location] = ret;
	
    return(ret);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
 * Install a single tramp.
 *
 */
void installTramp(instInstance *inst, process *proc, char *code, int codeSize,
		  instPoint *location, callWhen when) 
{
    //the default base trampoline template is the regular base trampoline.
    //However if the location iptype is  randomPoint then we have to use
    //the conservatibve base trampoline which saves the condition codes

    trampTemplate* current_template = &baseTemplate;

    if(location->ipType == otherPoint)
        current_template = &conservativeBaseTemplate;

    totalMiniTramps++;
    insnGenerated += codeSize/sizeof(int);
    
    // TODO cast
    proc->writeDataSpace((caddr_t)inst->trampBase, codeSize, code);

    Address atAddr;
    if(when == callPreInsn) {
	if (inst->baseInstance->prevInstru == false) {
	    atAddr = inst->baseInstance->baseAddr+current_template->skipPreInsOffset;
	    inst->baseInstance->cost += inst->baseInstance->prevBaseCost;
	    inst->baseInstance->prevInstru = true;
	    generateNoOp(proc, atAddr);
	}
    } else {
	if (inst->baseInstance->postInstru == false) {
	    atAddr = inst->baseInstance->baseAddr+current_template->skipPostInsOffset; 
	    inst->baseInstance->cost += inst->baseInstance->postBaseCost;
	    inst->baseInstance->postInstru = true;
	    generateNoOp(proc, atAddr);
	}
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

//This function returns true if the processor on which the daemon is running
//is an ultra SPARC, otherwise returns false.
bool isUltraSparc(){
  struct utsname u;
  if(uname(&u) < 0){
    cerr <<"Trouble in uname(), inst-sparc-solaris.C\n";
    return false;
  }
  if(!strcmp(u.machine, "sun4u")){
    return 1;
  }
  return false;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void emitLoadPreviousStackFrameRegister(Address register_num,
					Register dest,
					char *insn,
					Address &base,
					int size,
					bool noCost){
  if(register_num > 31)
    assert(0);
  else if(register_num > 15){
    /*Need to find it on the stack*/
    unsigned frame_offset = (register_num-16) * 4;
    /*generate a FLUSHW instruction, in order to make sure that
      the registers from the caller are on the caller's stack
      frame*/
    instruction *in = (instruction *) ((void*)&insn[base]);
    if(isUltraSparc())
      generateFlushw(in);
    else 
      generateTrapRegisterSpill(in);
    base+=sizeof(instruction);
    
    if(frame_offset == 0){
      emitV(loadIndirOp, 30, 0, dest, insn, base, noCost, size);
    }	    
    else {
      emitImm(plusOp,(Register) 30,(RegValue)frame_offset, 
	      dest, insn, base, noCost);
      emitV(loadIndirOp, dest, 0, dest, insn, base, noCost, size);
    }
  }	  
  else if(register_num > 7) { 
    //out registers become in registers, so we add 16 to the register
    //number to find it's value this stack frame. We move it's value
    //into the destination register
    emitV(orOp, (Register) register_num + 16, 0,  dest, insn, base, false);
  }
  else /* if(register_num >= 0) */ {
    int frame_offset;
    if(register_num % 2 == 0) 
      frame_offset = (register_num * -4) - 8;
    else 
      frame_offset = (register_num * -4);
    //read globals from the stack, they were saved in tramp-sparc.S
    emitImm(plusOp,(Register) 30,(RegValue)frame_offset, 
	    dest, insn, base, noCost);
    emitV(loadIndirOp, dest, 0, dest, insn, base, noCost, size);
  }
  /* else assert(0); */
  
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

Register emitFuncCall(opCode op, 
		      registerSpace *rs,
		      char *i, Address &base, 
		      const pdvector<AstNode *> &operands, 
		      const string &callee, process *proc,
		      bool noCost, const function_base *calleefunc,
		      const pdvector<AstNode *> &ifForks,
		      const instPoint *location)
{
   assert(op == callOp);
   Address addr;
   bool err;
   pdvector <Register> srcs;
   void cleanUpAndExit(int status);
   
   if (calleefunc) {
      addr = calleefunc->getEffectiveAddress(proc);
   }
   else {      
      addr = proc->findInternalAddress(callee, false, err);
      
      if (err) {
         function_base *func = proc->findOneFunction(callee);
         if (!func) {
            ostrstream os(errorLine, 1024, ios::out);
            os << "Internal error: unable to find addr of " << callee << endl;
            showErrorCallback(80, (const char *) errorLine);
            P_abort();
         }
         // TODO: is this correct or should we get relocated address?
         //addr = func->getAddress(0);
	 addr = func->getEffectiveAddress(proc);
      }
   }
   for (unsigned u = 0; u < operands.size(); u++)
      srcs.push_back(operands[u]->generateCode_phase2(proc, rs, i, base,
                                                      noCost, ifForks,
                                                      location));
   
   // TODO cast
   instruction *insn = (instruction *) ((void*)&i[base]);
   
   for (unsigned u=0; u<srcs.size(); u++){
      if (u >= 5) {
         string msg = "Too many arguments to function call in instrumentation code: only 5 arguments can be passed on the sparc architecture.\n";
         fprintf(stderr, msg.c_str());
         showErrorCallback(94,msg);
         cleanUpAndExit(-1);
      }
      genSimpleInsn(insn, ORop3, 0, srcs[u], u+8); insn++;
      base += sizeof(instruction);
      rs->freeRegister(srcs[u]);
   }
   
   // As Ling pointed out to me, the following is rather inefficient.  It does:
   //   sethi %hi(addr), %o5
   //   jmpl %o5 + %lo(addr), %o7   ('call' pseudo-instr)
   //   nop
   // We can do better:
   //   call <addr>    (but note that the call true-instr is pc-relative jump)
   //   nop
   generateSetHi(insn, addr, 13); insn++;
   genImmInsn(insn, JMPLop3, 13, LOW10(addr), 15); insn++;
   generateNOOP(insn);
        
   base += 3 * sizeof(instruction);
   
   // return value is the register with the return value from the function.
   // This needs to be %o0 since it is back in the caller's scope.
   return(REG_O(0));
}
 
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

Address emitA(opCode op, Register src1, Register /*src2*/, Register dest, 
              char *i, Address &base, bool /*noCost*/)
{
    //fprintf(stderr,"emitA(op=%d,src1=%d,src2=XX,dest=%d)\n",op,src1,dest);

    instruction *insn = (instruction *) ((void*)&i[base]);

    switch (op) {
      case ifOp: {
	// cmp src1,0
        genImmInsn(insn, SUBop3cc, src1, 0, 0); insn++;
	//genSimpleInsn(insn, SUBop3cc, src1, 0, 0); insn++;

	insn->branch.op = 0;
	insn->branch.cond = BEcond;
	insn->branch.op2 = BICCop2;
	insn->branch.anneal = false;
	insn->branch.disp22 = dest/4;
	insn++;

	generateNOOP(insn);
	base += sizeof(instruction)*3;
	return(base - 2*sizeof(instruction));
        }
      case branchOp: {
	// Unconditional branch
	generateBranchInsn(insn, dest); insn++;

	generateNOOP(insn);
	base += sizeof(instruction)*2;
	return(base - 2*sizeof(instruction));
        }
      case trampPreamble: {
#ifdef ndef
        // save and restore are done in the base tramp now
        genImmInsn(insn, SAVEop3, REG_SPTR, -112, REG_SPTR);
	base += sizeof(instruction);
        insn++;

	// generate code to save global registers
	for (unsigned u = 0; u < 4; u++) {
	  genStoreD(insn, 2*u, REG_FPTR, - (8 + 8*u));
	  base += sizeof(instruction);
	  insn++;
	}
#endif
        return(0);      // let's hope this is expected!
        }
      case trampTrailer: {
#ifdef ndef
        // save and restore are done in the base tramp now
	// generate code to restore global registers
	for (unsigned u = 0; u < 4; u++) {
	  genLoadD(insn, REG_FPTR, - (8 + 8*u), 2*u);
	  base += sizeof(instruction);
	  insn++;
	}

        // sequence: restore; nop; b,a back to base tramp; nop
        // we can do better.  How about putting the restore in
        // the delay slot of the branch instruction, as in:
        // b <back to base tramp>; restore
        genSimpleInsn(insn, RESTOREop3, 0, 0, 0); 
	base += sizeof(instruction);
	insn++;

	generateNOOP(insn);
	base += sizeof(instruction);
	insn++;
#endif
	// dest is in words of offset and generateBranchInsn is bytes offset
	generateBranchInsn(insn, dest << 2);
	base += sizeof(instruction);
	insn++;

        // add no-op, SS-5 sometimes seems to try to decode this insn - jkh 2/14
        generateNOOP(insn);
        insn++;
        base += sizeof(instruction);

	return(base -  2 * sizeof(instruction));
        }
      default:
        abort();        // unexpected op for this emit!
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

Register emitR(opCode op, Register src1, Register /*src2*/, Register dest, 
               char *i, Address &base, bool /*noCost*/,
               const instPoint * /* location */ )
{
    //fprintf(stderr,"emitR(op=%d,src1=%d,src2=XX,dest=XX)\n",op,src1);

    instruction *insn = (instruction *) ((void*)&i[base]);

    switch(op) {
      case getParamOp: {
#if defined(SHM_SAMPLING) && defined(MT_THREAD)
        // saving CT/vector address on the stack
	generateStore(insn, REG_MT_POS, REG_FPTR, -40);
        insn++; base += sizeof(instruction);
#endif
	// We have managed to emit two saves since the entry point of
	// the function, so the first 6 parameters are in the previous
	// frame's I registers, and we need to pick them off the stack

	// generate the FLUSHW instruction to make sure that previous
	// windows are written to the register save area on the stack
	if (isUltraSparc()) {
	    generateFlushw(insn);
	}
	else {
	    generateTrapRegisterSpill(insn);
	}
	insn++; base += sizeof(instruction);
	
	if (src1 < 6) {
	    // The arg is in a previous frame's I register. Get it from
	    // the register save area
	    generateLoad(insn, REG_FPTR, 4*8 + 4*src1, dest);
	    insn++; base += sizeof(instruction);
	}
	else {
	    // The arg is on the stack, two frames above us. Get the previous
	    // FP (i6) from the register save area
	    generateLoad(insn, REG_FPTR, 4*8 + 4*6, dest);
            // old fp is in dest now
	    insn++; base += sizeof(instruction);

	    // Finally, load the arg from the stack
	    generateLoad(insn, dest, 92 + 4 * (src1 - 6), dest);
	    insn++; base += sizeof(instruction);
	}

#if defined(SHM_SAMPLING) && defined(MT_THREAD)
        // restoring CT/vector address back in REG_MT_POS
        generateLoad(insn, REG_FPTR, -40, REG_MT_POS);
        insn++; base += sizeof(instruction);
#endif
	
	return dest;
      }
    case getSysParamOp: {
	// We have emitted one save since the entry point of
	// the function, so the first 6 parameters are in this
	// frame's I registers
	if (src1 < 6) {
	    // Param is in an I register
	    return(REG_I(src1));
	}	
	else {
	    // Param is on the stack
	    generateLoad(insn, REG_FPTR, 92 + 4 * (src1 - 6), dest);
	    insn++; base += sizeof(instruction);
	    return dest;
	}
    }
    case getRetValOp: {
	// We have emitted one save since the exit point of
	// the function, so the return value is in the previous frame's
	// I0, and we need to pick it from the stack

	// generate the FLUSHW instruction to make sure that previous
	// windows are written to the register save area on the stack
	if (isUltraSparc()) {
	    generateFlushw(insn);
	}
	else {
	    generateTrapRegisterSpill(insn);
	}
	insn++; base += sizeof(instruction);

	generateLoad(insn, REG_FPTR, 4*8, dest); 
	insn++; base += sizeof(instruction);

	return dest;
    }
    case getSysRetValOp:
	return(REG_I(0));
    default:
        abort();        // unexpected op for this emit!
    }
}

#ifdef BPATCH_LIBRARY
void emitJmpMC(int condition, int offset, char* baseInsn, Address &base)
{
  // Not needed for memory instrumentation, otherwise TBD
}

// VG(12/02/01): Emit code to add the original value of a register to
// another. The original value may need to be restored somehow...
static inline void emitAddOriginal(Register src, Register acc, 
			    char* baseInsn, Address &base, bool noCost)
{
// Plan:
// ignore g0 (r0), since we would be adding 0.
// get    g1-g4 (r1-r4) from stack (where the base tramp saved them)
// get    g5-g7 (r5-r7) also from stack, but issue an warning because
// the SPARC psABI cleary states that G5-G7 SHOULD NOT BE EVER WRITTEN
// in userland, as they may be changed arbitrarily by signal handlers!
// So AFAICT it doesn't make much sense to read them...
// get o0-o7 (r8-r15) by "shifting" the window back (i.e. get r24-r31)
// get l0-l7 (r16-23) by flushing the register window to stack
//                    and geeting them from there
// get i0-i7 (r24-31) by --"--
// Note: The last two may seem slow, but they're async. trap safe this way

// All this is coded as a binary search.

  bool mustFree = false;
  instruction *insn = (instruction *) ((void*)&baseInsn[base]);
  Register temp;

  if (src >= 16) {
    // VG(12/06/01): Currently saving registers on demand is NOT
    // implemented on SPARC (dumps assert), so we can safely ignore it
    temp = regSpace->allocateRegister(baseInsn, base, noCost);
    mustFree = true;

    // Cause repeated spills, till all windows but current are clear
    generateFlushw(insn); // only ultraSPARC supported here
    insn++;
    base += sizeof(instruction);

    // the spill trap puts these at offset from the previous %sp now %fp (r30)
    unsigned offset = (src-16) * sizeof(long); // FIXME for 64bit mutator/32bit mutatee
    generateLoad(insn, 30, offset, temp);
    insn++;
    base += sizeof(instruction);
  }
  else if (src >= 8)
    temp = src + 16;
  else if (src >= 1) {
    // VG(12/06/01): Currently saving registers on demand is NOT
    // implemented on SPARC (dumps assert), so we can safely ignore it
    temp = regSpace->allocateRegister(baseInsn, base, noCost);
    mustFree = true;

    // the base tramp puts these at offset from %fp (r30)
    unsigned offset = ((src%2) ? src : (src+2)) * -sizeof(long); // FIXME too
    generateLoad(insn, 30, offset, temp);
    insn++;
    base += sizeof(instruction);

    if (src >= 5)
      logLine("SPARC WARNING: Restoring original value of g5-g7 is unreliable!");
  }
  else // src == 0
    return;

  // add temp to dest;
  // writes at baseInsn+base and updates base, we must update insn...
  emitV(plusOp, temp, acc, acc, baseInsn, base, noCost, 0);
  insn = (instruction *) ((void*)&baseInsn[base]);

  if(mustFree)
    regSpace->freeRegister(temp);
}

// VG(11/30/01): Load in destination the effective address given
// by the address descriptor. Used for memory access stuff.
void emitASload(BPatch_addrSpec_NP as, Register dest, char* baseInsn,
		Address &base, bool noCost)
{
  //instruction *insn = (instruction *) ((void*)&baseInsn[base]);
  int imm = as.getImm();
  int ra  = as.getReg(0);
  int rb  = as.getReg(1);
  
  // TODO: optimize this to generate the minimum number of
  // instructions; think about schedule

  // emit code to load the immediate (constant offset) into dest; this
  // writes at baseInsn+base and updates base, we must update insn...
  emitVload(loadConstOp, (Address)imm, dest, dest, baseInsn, base, noCost);
  //insn = (instruction *) ((void*)&baseInsn[base]);
  
  // If ra is used in the address spec, allocate a temp register and
  // get the value of ra from stack into it
  if(ra > -1)
    emitAddOriginal(ra, dest, baseInsn, base, noCost);

  // If rb is used in the address spec, allocate a temp register and
  // get the value of ra from stack into it
  if(rb > -1)
    emitAddOriginal(rb, dest, baseInsn, base, noCost);
}

void emitCSload(BPatch_addrSpec_NP as, Register dest, char* baseInsn,
		Address &base, bool noCost)
{
  emitASload(as, dest, baseInsn, base, noCost);
}
#endif

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

//
// load the original FP (before the dyninst saves) into register dest
//
int getFP(instruction *insn, Register dest)
{
    genSimpleInsn(insn, RESTOREop3, 0, 0, 0);
    insn++;

    generateStore(insn, REG_FPTR, REG_SPTR, 68); 
    insn++;
	  
    genImmInsn(insn, SAVEop3, REG_SPTR, -112, REG_SPTR);
    insn++;

    generateLoad(insn, REG_SPTR, 112+68, dest); 
    insn++;

    return(4*sizeof(instruction));
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void emitVload(opCode op, Address src1, Register src2, Register dest, 
              char *i, Address &base, bool /*noCost*/, int /* size */)
{
    instruction *insn = (instruction *) ((void*)&i[base]);

    if (op == loadConstOp) {
      // dest = src1:imm    TODO

      if ((src1) > ( unsigned )MAX_IMM13 || (src1) < ( unsigned )MIN_IMM13) {
            // src1 is out of range of imm13, so we need an extra instruction
	    generateSetHi(insn, src1, dest);
	    base += sizeof(instruction);
	    insn++;

	    // or regd,imm,regd

            // Chance for optimization: we should check for LOW10(src1)==0,
            // and if so, don't generate the following bitwise-or instruction,
            // since in that case nothing would be done.

	    genImmInsn(insn, ORop3, dest, LOW10(src1), dest);
	    base += sizeof(instruction);
	} else {
	    // really or %g0,imm,regd
	    genImmInsn(insn, ORop3, 0, src1, dest);

	    base += sizeof(instruction);
	}
    } else if (op ==  loadOp) {
	// dest = [src1]   TODO
	generateSetHi(insn, src1, dest);
	insn++;

	generateLoad(insn, dest, LOW10(src1), dest);

	base += sizeof(instruction)*2;
    } else if (op ==  loadFrameRelativeOp) {
	// return the value that is FP offset from the original fp
	//   need to restore old fp and save it on the stack to get at it.

	base += getFP(insn, dest);
	insn = (instruction *) ((void*)&i[base]);
	if (((int) src1 < MIN_IMM13) || ((int) src1 > MAX_IMM13)) {
	    // offsets are signed!
	    int offset = (int) src1;

	    // emit sethi src2, offset
	    generateSetHi(insn, offset, src2);
	    base += sizeof(instruction);
	    insn++;

	    // or src2, offset, src2
	    genImmInsn(insn, ORop3, src2, LOW10(offset), src2);
	    base += sizeof(instruction);
	    insn++;

	    // add dest, src2, dest
	    genSimpleInsn(insn, ADDop3, dest, src2, src2);
	    base += sizeof(instruction);
	    insn++;

	    generateLoad(insn, src2, 0, dest);
	    insn++;
	    base += sizeof(instruction);
	}  else {
	    generateLoad(insn, dest, src1, dest);
	    insn++;
	    base += sizeof(instruction);
	}
    } else if (op == loadFrameAddr) {
	// offsets are signed!
	int offset = (int) src1;

	base += getFP(insn, dest);
	insn = (instruction *) ((void*)&i[base]);

	if (((int) offset < MIN_IMM13) || ((int) offset > MAX_IMM13)) {
	    // emit sethi src2, offset
	    generateSetHi(insn, offset, src2);
	    base += sizeof(instruction);
	    insn++;

	    // or src2, offset, src2
	    genImmInsn(insn, ORop3, src2, LOW10(offset), src2);
	    base += sizeof(instruction);
	    insn++;

	    // add dest, src2, dest
	    genSimpleInsn(insn, ADDop3, dest, src2, dest);
	    base += sizeof(instruction);
	    insn++;
	}  else {
	    // fp is in dest, just add the offset
	    genImmInsn(insn, ADDop3, dest, offset, dest);
	    insn++;
	    base += sizeof(instruction);
	}
    } else {
        abort();       // unexpected op for this emit!
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void emitVstore(opCode op, Register src1, Register src2, Address dest, 
              char *i, Address &base, bool /*noCost*/, int /* size */)
{
    instruction *insn = (instruction *) ((void*)&i[base]);

    if (op == storeOp) {
	insn->sethi.op = FMT2op;
	insn->sethi.rd = src2;
	insn->sethi.op2 = SETHIop2;
	insn->sethi.imm22 = HIGH22(dest);
	insn++;

	generateStore(insn, src1, src2, LOW10(dest));

	base += sizeof(instruction)*2;
    } else if (op == storeFrameRelativeOp) {
	// offsets are signed!
	int offset = (int) dest;

	base += getFP(insn, src2);
	insn = (instruction *) ((void*)&i[base]);

	if ((offset < MIN_IMM13) || (offset > MAX_IMM13)) {
	    // We are really one regsiter short here, so we put the
	    //   value to store onto the stack for part of the sequence
	    generateStore(insn, src1, REG_SPTR, 112+68);
	    base += sizeof(instruction);
	    insn++;

	    generateSetHi(insn, offset, src1);
	    base += sizeof(instruction);
	    insn++;

	    genImmInsn(insn, ORop3, src1, LOW10(offset), src1);
	    base += sizeof(instruction);
	    insn++;

	    genSimpleInsn(insn, ADDop3, src1, src2, src2);
	    base += sizeof(instruction);
	    insn++;

	    generateLoad(insn, REG_SPTR, 112+68, src1); 
	    base += sizeof(instruction);
	    insn++;

	    generateStore(insn, src1, src2, 0);
	    base += sizeof(instruction);
	    insn++;
	} else {
	    generateStore(insn, src1, src2, offset);
	    insn++;
	    base += sizeof(instruction);
	}
    } else {
        abort();       // unexpected op for this emit!
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void emitVupdate(opCode op, RegValue src1, Register /*src2*/, Address dest, 
              char *i, Address &base, bool noCost)
{
    instruction *insn = (instruction *) ((void*)&i[base]);

    if (op == updateCostOp) {
        // generate code to update the observed cost.
	if (!noCost) {
	   // sethi %hi(dest), %l0
	   generateSetHi(insn, dest, REG_L(0));
	   base += sizeof(instruction);
	   insn++;
  
	   // ld [%l0+ lo(dest)], %l1
	   generateLoad(insn, REG_L(0), LOW10(dest), REG_L(1));
	   base += sizeof(instruction);
	   insn++;
  
	   // update value (src1 holds the cost, in cycles; e.g. 19)
	   if (src1 <= MAX_IMM13) {
	      genImmInsn(insn, ADDop3, REG_L(1), src1, REG_L(1));
	      base += sizeof(instruction);
	      insn++;

	      generateNOOP(insn);
	      base += sizeof(instruction);
	      insn++;

	      generateNOOP(insn);
	      base += sizeof(instruction);
	      insn++;
	   } else {
	      // load in two parts
	      generateSetHi(insn, src1, REG_L(2));
	      base += sizeof(instruction);
	      insn++;

	      // or regd,imm,regd
	      genImmInsn(insn, ORop3, REG_L(2), LOW10(src1), REG_L(2));
	      base += sizeof(instruction);
	      insn++;

	      // now add it
	      genSimpleInsn(insn, ADDop3, REG_L(1), REG_L(2), REG_L(1));
	      base += sizeof(instruction);
	      insn++;
	   }
  
	   // store result st %l1, [%l0+ lo(dest)];
	   generateStore(insn, REG_L(1), REG_L(0), LOW10(dest));
	   base += sizeof(instruction);
	   insn++;
	}
    } else {
        abort();       // unexpected op for this emit!
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void emitV(opCode op, Register src1, Register src2, Register dest, 
              char *i, Address &base, bool /*noCost*/, int /* size */,
              const instPoint * /* location */, process * /* proc */,
              registerSpace * /* rs */ )
{
    //fprintf(stderr,"emitV(op=%d,src1=%d,src2=%d,dest=%d)\n",op,src1,src2,dest);

    assert ((op!=branchOp) && (op!=ifOp) && 
            (op!=trampTrailer) && (op!=trampPreamble));         // !emitA
    assert ((op!=getRetValOp) && (op!=getSysRetValOp) &&
            (op!=getParamOp) && (op!=getSysParamOp));           // !emitR
    assert ((op!=loadOp) && (op!=loadConstOp));                 // !emitVload
    assert ((op!=storeOp));                                     // !emitVstore
    assert ((op!=updateCostOp));                                // !emitVupdate

    instruction *insn = (instruction *) ((void*)&i[base]);

    if (op == loadIndirOp) {
	generateLoad(insn, src1, 0, dest);
	base += sizeof(instruction);
    } else if (op == storeIndirOp) {
	generateStore(insn, src1, dest, 0);
	base += sizeof(instruction);
    } else if (op == noOp) {
	generateNOOP(insn);
	base += sizeof(instruction);
    } else if (op == saveRegOp) {
	// should never be called for this platform.
        // VG(12/02/01): Unfortunately allocateRegister *may* call this
	abort();
    } else {
      int op3=-1;
	switch (op) {
	    // integer ops
	    case plusOp:
		op3 = ADDop3;
		break;

	    case minusOp:
		op3 = SUBop3;
		break;

	    case timesOp:
		op3 = SMULop3;
		break;

	    case divOp:
		op3 = SDIVop3;
                //need to set the Y register to Zero, Zhichen
                genImmInsn(insn, WRYop3, REG_G(0), 0, 0);
                base += sizeof(instruction);
                insn = (instruction *) ((void*)&i[base]);
		break;

	    // Bool ops
	    case orOp:
		op3 = ORop3;
		break;

	    case andOp:
		op3 = ANDop3;
		break;

	    // rel ops
	    // For a particular condition (e.g. <=) we need to use the
            // the opposite in order to get the right value (e.g. for >=
            // we need BLTcond) - naim
	    case eqOp:
		genRelOp(insn, BNEcond, src1, src2, dest, base);
		return;
		break;

            case neOp:
                genRelOp(insn, BEcond, src1, src2, dest, base);
                return;
                break;

	    case lessOp:
                genRelOp(insn, BGEcond, src1, src2, dest, base);
                return;
                break;

            case leOp:
                genRelOp(insn, BGTcond, src1, src2, dest, base);
                return;
                break;

            case greaterOp:
                genRelOp(insn, BLEcond, src1, src2, dest, base);
                return;
                break;

            case geOp:
                genRelOp(insn, BLTcond, src1, src2, dest, base);
                return;
                break;

	    default:
		abort();
		break;
	}
	genSimpleInsn(insn, op3, src1, src2, dest);

	base += sizeof(instruction);
      }
   return;
}

/****************************************************************************/
/****************************************************************************/

static inline bool isRestoreInsn(instruction i) {
    return (i.rest.op == 2 \
	       && ((i.rest.op3 == ORop3 && i.rest.rd == 15)
		       || i.rest.op3 == RESTOREop3));
}

/****************************************************************************/
/****************************************************************************/

static inline bool CallRestoreTC(instruction instr, instruction nexti) {
    return (isCallInsn(instr) && isRestoreInsn(nexti));
}

/****************************************************************************/
/****************************************************************************/


/*
    Return bool value indicating whether instruction sequence
     found signals tail-call jmp; nop; sequence.  Note that this should 
     NOT include jmpl; nop;, ret; nop;, retl; nop;....

    Current heuristic to detect such sequences :
     look for jmp %reg, nop in function w/ no stack frame, if jmp, nop
     are last 2 instructions, return true (definate TC), at any other point,
     return false (not TC).  Otherwise, return false (no TC).
     w/ no stack frame....
    instr is instruction being examioned.
    nexti is instruction after
    addr is address of <instr>
    func is pointer to function class object describing function
     instructions come from....
 */
static inline bool JmpNopTC(instruction instr, instruction nexti,
			    Address addr, pd_Function *func) {

    if (!isInsnType(instr, JMPLmask, JMPLmatch)) {
        return 0;
    }

    assert(instr.resti.op3 == 0x38);

    // only looking for jump instructions which don't overwrite a register
    //  with the PC which the jump comes from (g0 is hardwired to 0, so a write
    //  there has no effect?)....  
    //  instr should have gdb disass syntax : 
    //      jmp  %reg, 
    //  NOT jmpl %reg1, %reg2
    if (instr.resti.rd != REG_G(0)) {
        return 0;
    }

    // only looking for jump instructions in which the destination is
    //  NOT %i7 + 8/12/16 or %o7 + 8/12/16 (ret and retl synthetic 
    //  instructions, respectively)
    if (instr.resti.i == 1) {
        if (instr.resti.rs1 == REG_I(7) || instr.resti.rs1 == REG_O(7)) {
	    // NOTE : some return and retl instructions jump to {io}7 + 12,
	    //  or (io)7 + 16, not + 8, to have some extra space to store the size of a 
	    //  return structure....
            if (instr.resti.simm13 == 0x8 || instr.resti.simm13 == 12 ||
		    instr.resti.simm13 == 16) {
	        return 0;
	    }
        }
    }  

    // jmp, foloowed by NOP....
    if (!isNopInsn(nexti)) {
        return 0;
    }

    // in function w/o stack frame....
    if (!func->hasNoStackFrame()) {
        return 0;
    }

    // if sequence is detected, but not at end of fn 
    //  (last 2 instructions....), return value indicating possible TC.
    //  This should (eventually) mark the fn as uninstrumenatble....
    if (addr != (func->getAddress(0) + func->size() - 8)) {
        return 0;
    }

    return 1;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
  Is the specified call instruction one whose goal is to set the 07 register
  (the sequence of execution is as if the call instruction did not change the
  control flow, and the O7 register is set)?

  here, we define a call whose goal is to set the 07 regsiter
    as one where the target is the call address + 8, AND where that
    target is INSIDE the same function (need to make sure to check for that
    last case also, c.f. function DOW, which ends with):
     0xef601374 <DOW+56>:    call  0xef60137c <adddays>
     0xef601378 <DOW+60>:    restore 

  instr - raw instruction....
  functionSize - size of function (in bytes, NOT # instructions)....
  instructionOffset - BYTE offset in function at which instr occurs....
 */        
static inline bool is_set_O7_call(instruction instr, unsigned functionSize, 
			      unsigned instructionOffset) {
    // if the instruction is call %register, assume that it is NOT a 
    //  call designed purely to set %O7....
    if(instr.call.op != CALLop) {
        return false;
    }
    if (((instr.call.disp30 << 2) == 8) && 
             (instructionOffset < (functionSize - 2 * sizeof(instruction)))) {
        return true; 
    }
    return false; 
}  

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
    Does the specified call instruction call to target inside function
    or outside - may be indeterminate if insn is call %reg instead of 
    call <address> (really call PC + offset)
    Note: (recursive) calls back to the beginning of the function are OK
    since we really want to consider these as instrumentable call sites!
 */
enum fuzzyBoolean {eFalse = 0, eTrue = 1, eDontKnow = 2}; 

static enum fuzzyBoolean is_call_outside_function(const instruction instr,
                const Address functionStarts, const Address instructionAddress, 
		const unsigned int functionSize) 
{
    // call %register - don't know if target inside function....
    if(instr.call.op != CALLop) {
        return eDontKnow;
    }
    const Address call_target = instructionAddress + (instr.call.disp30 << 2);
    if ((call_target > functionStarts) && 
        (call_target < (functionStarts + functionSize))) {
        return eFalse;
    }
    return eTrue;
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
 * Find the instPoints of this function.
 */
bool pd_Function::findInstPoints(const image *owner) {

  Address firstAddress = getAddress(0);
  Address lastAddress = getAddress(0) + size();
  Address adr;
  Address target;
  Address entry;
  Address disp;

  instruction instr; 
  instruction nexti;

  instPoint *point = 0;

  // For determining if function needs relocation to be instrumented
  relocatable_ = false;
  mayNeedRelocation_ = false;
  canBeRelocated_ = true;

  // Initially assume function has no stack frame 
  noStackFrame = true;

  // variables for function parameters
  const instPoint *blah = 0;
  
  bool dummyParam;

  // Ids for instPoints
  unsigned retId = 0;
  unsigned callsId = 0; 

  if (size() == 0) {
    return false;
  } 

  instr.raw = owner->get_instruction(firstAddress);
  if (!IS_VALID_INSN(instr)) {
    return false;
  }

  // Determine if function needs to be relocated when instrumented
  for ( adr = firstAddress; adr < lastAddress; adr += 4) { 
    instr.raw = owner->get_instruction(adr);
    nexti.raw = owner->get_instruction(adr+4);

    // If there's an TRAP instruction in the function, we assume
    // that it is an system call and will relocate it to the heap
    if (isInsnType(instr, TRAPmask, TRAPmatch)) {
      relocatable_ = true;
    } 

    // TODO: This is a hacking for the solaris(solaris2.5 actually)
    // We will relocate that function if the function has been 
    // tail-call optimazed.
    // (Actully, the reason of this is that the system calls like 
    //  read, write, etc have the tail-call optimazation to call
    //  the _read, _write etc. which contain the TRAP instruction 
    //  This is only done if libc is statically linked...if the
    //  libTag is set, otherwise we instrument read and _read
    //  both for the dynamically linked case
    // New for Solaris 2.6 support - new form of tail-call opt-
    //  imization found:
    //   jmp %register
    //   nop
    //  as last 2 instructions in function which does not have
    //  own register frame.
    if (CallRestoreTC(instr, nexti) || JmpNopTC(instr, nexti, adr, this)) {
      relocatable_ = true;
    }

    // if call is directly to a retl, this is not a real call, but
    // is instead used to set the o7 register. Set the function to be
    // relocated when instrumented.
    if (isCallInsn(instr)) {

      // find target address of call
      disp = instr.call.disp30 << 2;
      target = adr + disp;

      // get target instruction of the call   
      instruction tmpInsn;
      tmpInsn.raw = owner->get_instruction( target );

      if((tmpInsn.raw & 0xfffff000) == 0x81c3e000) {
        mayNeedRelocation_ = true;
      }

      // if this is a call instr to a location within the function, 
      // and if the offest is 8 then this is used to set the o7 register
      // with the pc and we may need to relocate the function 
      if ( is_set_O7_call(instr, size(), adr - firstAddress)) {
        mayNeedRelocation_ = true;
      }
    }
  }


  /* FIND FUNCTION ENTRY */


  entry = firstAddress;
  for (adr = firstAddress; adr < lastAddress; adr += 4) { 

    // The function Entry is defined as the first SAVE instruction plus
    // the instructions after this.
    // ( The first instruction for the nonleaf function is not 
    //   necessarily a SAVE instruction. ) 
    instr.raw = owner->get_instruction(adr);

    if (isInsnType(instr, SAVEmask, SAVEmatch)) {
      entry = adr;
      noStackFrame = false;
      continue;
    }
  }


  // If there's no SAVE instruction found, this is a leaf function
  // and function Entry will be defined from the first instruction
  if (noStackFrame) {

    // noStackFrame, apparently leaf function
    adr = firstAddress;
    entry = adr;
  }


  /* CHECK IF FUNCTION SHOULD NOT BE RELOCATED WHEN INSTRUMENTED */

  // FUNCTION TOO SMALL
  if (size() <= 3*sizeof(instruction)) {
    canBeRelocated_ = false;
  }


  // Can't handle function
  if (canBeRelocated_ == false && relocatable_ == true) {
    return false;
  }


#ifdef BPATCH_LIBRARY
  if (BPatch::bpatch->hasForcedRelocation_NP()) {
    if (canBeRelocated_ == true) {
      relocatable_ = true;
    }
  }
#endif

  o7_live = false;

  /* CREATE ENTRY INSTPOINT */
  instr.raw = owner->get_instruction(entry);

  funcEntry_ = new instPoint(this, owner, entry, true, functionEntry);
  assert(funcEntry_);

  // ITERATE OVER INSTRUCTIONS, locating instPoints
  adr = firstAddress;

  instructions = new instruction[size()/sizeof(instruction)];
 
  for (int i=0; adr < lastAddress; adr += sizeof(instruction), i++) {

    instr.raw = owner->get_instruction(adr);
    instructions[i] = instr;
    nexti.raw = owner->get_instruction(adr+4);

    if(!o7_live){
      InsnRegister rd,rs1,rs2;
      get_register_operands(instr,&rd,&rs1,&rs2);

      if(rs1.is_o7() || rs2.is_o7())// ||
        // (rd.is_o7() && 
	//  ((instr.raw & 0xc1f80000) != 0x81c00000))) /*indirect call*/
              o7_live = true;
    }

    // check for return insn and as a side, decide if we are at the
    // end of the function.
    if (isReturnInsn(owner, adr, dummyParam, prettyName())) {
      // define the return point

      instPoint *point;
      point = new instPoint(this, owner, adr, false, functionExit);
      funcReturns.push_back(point);
      funcReturns[retId] -> instId = retId; retId++;
    } 
    
    else if (instr.branch.op == 0      
              &&  (instr.branch.op2 == 2 || instr.branch.op2 == 6) 
	      && (instr.branch.cond == 0 || instr.branch.cond == 8)) {

      // find if this branch is going out of the function
      disp = instr.branch.disp22;
      Address target = adr +  (disp << 2);
         
      if (target < firstAddress || target >= lastAddress) {

        instPoint *point;
        point = new instPoint(this, owner, adr, false, functionExit);

        if ((instr.branch.cond != 0) && (instr.branch.cond != 8)) {  

          point->isBranchOut = true;
	  point->branchTarget = target;
	}

	funcReturns.push_back(point);
	funcReturns[retId] -> instId = retId; retId++;

      }
    } 
    
    else if (isCallInsn(instr)) {

      // if the call target is the address of the call instruction
      // then this is not something that we can instrument...
      // this occurs in functions with code that is modifined when 
      // they are loaded by the run-time linker, or when the .init
      // section is executed.  In this case the instructions in the
      // parsed image file are different from the ones in the executable
      // process.
      Address call_target = adr + (instr.call.disp30 << 2);
      if(instr.call.op == CALLop) { 
        if(call_target == adr){ 
          cerr << "WARN : function " << prettyName().c_str()
               << " has call to same location as call, NOT instrumenting"
               << endl;
	  return false;
	}
      }

      // first, check for tail-call optimization: a call where the 
      // instruction in the delay slot write to register %o7(15), usually 
      // just moving the caller's return address, or doing a restore
      // Tail calls are instrumented as return points, not call points.

      if (CallRestoreTC(instr, nexti)) {

        // generate a call instPoint for the call instruction
        point = new instPoint(this, owner, adr, false, callSite);

        // record the call instPoint in pdFunction's calls vector 
        addCallPoint(instr, callsId, 0, point, blah);
  
        // generate a functionExit instPoint for the tail-call sequence  
        // (mark the instruction in the delay slot of the call as the exit)
        point = new instPoint(this, owner, adr, false, functionExit);
        funcReturns.push_back(point);
        funcReturns[retId] -> instId = retId; retId++;

      } else {

	  // check if the call is to inside the function - if definately
	  // inside function (meaning that thew destination can be determined
	  // statically because its a call to an address, not to a register 
          // or register + offset) then don't instrument as call site, 
          // otherwise (meaning that the call destination is known statically 
          // to be outside the function, or is not known statically), then 
          // instrument as a call site....
          enum fuzzyBoolean is_inst_point;
          is_inst_point = is_call_outside_function(instr, firstAddress, 
                                                           adr, size());
          if (is_inst_point == eFalse) {

            // if this is a call instr to a location within the function, 
            // and if the offest is not 8 then do not define this function 
	    if (!is_set_O7_call(instr, size(), adr - firstAddress)) {
	      return false;
	    }

            // generate a call instPoint for the call instruction
            point = new instPoint(this, owner, adr, false, callSite);

            // record the call instPoint in pdFunction's calls vector 
            addCallPoint(instr, callsId, 0, point, blah);

  	  } else {
 
              // get call target instruction   
              Address call_target = adr + (instr.call.disp30 << 2);
              instruction tmpInsn;
              tmpInsn.raw = owner->get_instruction( call_target );

              // check that call is not directly to a retl instruction,
              // and thus a real call
              if((tmpInsn.raw & 0xfffff000) != 0x81c3e000) {

                // generate a call instPoint for the call instruction
                point = new instPoint(this,  owner, adr, false, callSite);

                // record the call instPoint in pdFunction's calls vector 
                addCallPoint(instr, callsId, 0, point, blah);

	      } 
	  }         
      }
    }

    else if (JmpNopTC(instr, nexti, adr, this)) {

      // generate a call instPoint for the jump instruction
      point = new instPoint(this, owner, adr, false, callSite);

      // record the call instPoint in pdFunction's calls vector 
      addCallPoint(instr, callsId, 0, point, blah);

      // generate a functionExit instPoint for the tail-call sequence  
      // (mark the instruction in the delay slot of the jump as the exit)
      point = new instPoint(this,  owner, adr, false, functionExit);
      funcReturns.push_back(point);
      funcReturns[retId] -> instId = retId; retId++;
    }

    else if (isInsnType(instr, JMPLmask, JMPLmatch)) {
       /* A register indirect jump. Some jumps may exit the function 
          (e.g. read/write on SunOS). In general, the only way to 
	  know if a jump is exiting the function is to instrument
	  the jump to test if the target is outside the current 
	  function. Instead of doing this, we just check the 
	  previous two instructions, to see if they are loading
	  an address that is out of the current function.
	  This should catch the most common cases (e.g. read/write).
	  For other cases, we would miss a return point.

	  This is the case considered:

	     sethi addr_hi, r
	     or addr_lo, r, r
	     jump r
	*/

      Register jumpreg = instr.rest.rs1;
      instruction prev1;
      instruction prev2;

      prev1.raw = owner->get_instruction(adr-4);
      prev2.raw = owner->get_instruction(adr-8);

      Address targetAddr;

      if (instr.rest.rd == 0 && (instr.rest.i == 1 || instr.rest.rs2 == 0)
	  && prev2.sethi.op == FMT2op && prev2.sethi.op2 == SETHIop2 
	  && prev2.sethi.rd == (unsigned)jumpreg
	  && prev1.rest.op == RESTop 
          && prev1.rest.rd == (unsigned)jumpreg && prev1.rest.i == 1
          && prev1.rest.op3 == ORop3 && prev1.rest.rs1 == (unsigned)jumpreg) {

        targetAddr = (prev2.sethi.imm22 << 10) & 0xfffffc00;
        targetAddr |= prev1.resti.simm13;

        if ( (targetAddr < firstAddress) || (targetAddr >= lastAddress) ){

          instPoint *point;
          point = new instPoint(this, owner, adr, false, functionExit);

	  funcReturns.push_back(point);
	  funcReturns[retId] -> instId = retId; retId++;
	}
      }
    }
  }


  // For leaf functions, beware of exit points with a DCTI just prior.
  // If we can't use a branch instruction (i.e. ba,a) at the exit, we 
  // will need to relocate the function.  
  if ( hasNoStackFrame() ) {

    // Check each return point
    for (unsigned j=0; j < funcReturns.size(); j++) {

      // Leaf exits always require prior instructions to instrument with
      // a save; call; restore; sequence
      assert(funcReturns[j]->usesPriorInstructions);

      // Check if the first prior instruction is in the delay slot of a
      // DCTI, or is an aggregate for a DCTI
      if ( (funcReturns[j]->secondPriorIsDCTI) ||
           (funcReturns[j]->thirdPriorIsDCTI && 
            funcReturns[j]->firstPriorIsAggregate) ) {

        mayNeedRelocation_ = true;
        continue;
      }
    }


    // Check for branches to the exit point. If there is such a branch,
    // we will need to relocate the function and add nops after exit point
    // instead of claiming prior instructions
    Address insnAddr    = firstAddress;
    int num_instructions = size() / sizeof(instruction);
 
    // Iterate over all instructions
    for (int j=0; j < num_instructions; j++) {

      // Grab the instruction
      instr = instructions[j];

      // Check if the instruction is a branch
      if ( instr.branch.op == 0 ) {

        // find if this branch is going to an exit point
        int displacement = instr.branch.disp22;
        Address target   = insnAddr + (displacement << 2);

        // Check each return point address
        for (unsigned k=0; k < funcReturns.size(); k++) {

          // Check if the branch target matches the exit point address
          if (target == funcReturns[k]->iPgetAddress()) {
            mayNeedRelocation_ = true;
            continue;
	  }
	}
      }

      insnAddr += sizeof(instruction);
    }
  }

  bool checkPoints = checkInstPoints(owner);

  if ( (checkPoints == false) || (!canBeRelocated_ && relocatable_) ){
    return false;
  }

  return true;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
 * Check all the instPoints within this function to see if there's 
 * any conficts happen.
 */
bool pd_Function::checkInstPoints(const image *owner) {

    // Our own library function, skip the test.
    if (prettyName().prefixed_by("DYNINST")) 
	return true;

#ifndef BPATCH_LIBRARY /* XXX Users of libdyninstAPI might not agree. */
    // The function is too small to be worth instrumenting.
    if (size() <= 12){
	return false;
    }
#endif

    // No function return! return false;
    if (sizeof(funcReturns) == 0) {
	return false;
    }

    instruction instr;
    Address adr = getAddress(0);

    bool retl_inst = false;
    bool restore_inst = false;
    // Check if there's any branch instruction jump to the middle
    // of the instruction sequence in the function entry point
    // and function exit point.
    for ( ; adr < getAddress(0) + size(); adr += sizeof(instruction)) {

	instr.raw = owner->get_instruction(adr);
	if(isInsnType(instr, RETLmask, RETLmatch)) retl_inst = true;
	if(isInsnType(instr, RESTOREmask, RESTOREmatch)) restore_inst = true;
	if (isInsnType(instr, BRNCHmask, BRNCHmatch)||
	    isInsnType(instr, FBRNCHmask, FBRNCHmatch)) {

	    int disp = instr.branch.disp22;
	    Address target = adr + (disp << 2);

	    if ((target > funcEntry_->addr)&&
		(target < (funcEntry_->addr + funcEntry_->size))) {
		if (adr > (funcEntry_->addr+funcEntry_->size)){

                  // function can be instrumented if we relocate it
                  relocatable_ = true;
		}
	    }

	    for (u_int i = 0; i < funcReturns.size(); i++) {
		if ((target > funcReturns[i]->addr)&&
		    (target < (funcReturns[i]->addr + funcReturns[i]->size))) {
		    if ((adr < funcReturns[i]->addr)||
			(adr > (funcReturns[i]->addr + funcReturns[i]->size))){

                      // function can be instrumented if we relocate it
                      relocatable_ = true; 
		    }
		}
	    }
	}
    }

    // if there is a retl instruction and we don't think this is a leaf
    // function then this is a way messed up function...well, at least we
    // we can't deal with this...the only example I can find is _cerror
    // and _cerror64 in libc.so.1
    if(retl_inst && !noStackFrame && !restore_inst){ 
        //cerr << "WARN : function " << prettyName().c_str()
        //     << " retl instruction in non-leaf function, can't instrument"
        //      << endl;
	return false;
    }

    // check that no instrumentation points could overlap
    Address func_entry = funcEntry_->addr + funcEntry_->size; 
    for (u_int i = 0; i < funcReturns.size(); i++) {
	if(func_entry >= funcReturns[i]->addr){

          // function can be instrumented if we relocate it 
          relocatable_ = true; 
        }
	if(i >= 1){ // check if return points overlap
	    Address prev_exit = funcReturns[i-1]->addr+funcReturns[i-1]->size;  
	    if(funcReturns[i]->addr < prev_exit) {

              // function can be instrumented if we relocate it 
              relocatable_ = true;
	    } 
	}
    }

    return true;	
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// used for sorting inst points - typecast void *s to instPoint **s, then
//  do {-1, 0, 1} comparison by address....
int sort_inst_points_by_address(const void *arg1, const void *arg2) {
    instPoint * const *a = static_cast<instPoint* const *>(arg1);
    instPoint * const *b = static_cast<instPoint* const *>(arg2);
    if ((*a)->iPgetAddress() > (*b)->iPgetAddress()) {
        return 1;
    } else if ((*a)->iPgetAddress() < (*b)->iPgetAddress()) {
        return -1;
    }
    return 0;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
  First pass, handle cases where general-purpose re-writing needs to be
  done to preserve paradynd/dyninstAPI's assumptions about function
  structure.  
    In current sparc-solaris version, this is ONLY the following cases:
      If a call site and return point overlap and are located directly
       next to eachother, and the return point is located on a 
       restore operation, then a TailCallPA is applied whose footprint
       covers the call and the restore.
       The TailCallPA should rewrite the call; restore, and update the 
       locations of both inst points as necessary.
      If the 2nd instruction in a function is a CALL, then a single nop
       is inserted before the call - to clear out the delay slot of the
       branch which is inserted at the first instruction (for entry point
       instrumentation).
       If the dyninstAPI is seperated from paradyn, or when converting this 
       code to support instrumentation at arbitrary points (instead of assuming
       entry, exit, and call site instrumentation), then the check should be 
       changed from:
         look at 2nd insn, see if its a call
        TO
	 look at all inst points, see if any of them have a call in what
	 corresponds to the delay slot of the instruction which is going to
	 get stomped by a branch/call to tramp....
      If the code has any calls to address (of call) + 8, replace the call with
        a sequence which sets the 07 register to the (original) address....
  Function:
    Attaches LocalAlterations related to general rewrites of function
     (by adding them to LocalAlterationSet p).
    returns boolean value indicating whether it was able to figure out
    sequence of LocalAlterations to apply to perform specified rewrites....
 */
bool pd_Function::PA_attachGeneralRewrites( const image *owner,
                                            LocalAlterationSet *p, 
                                            Address baseAddress, 
                                            Address firstAddress, 
                                            instruction loadedCode[], 
                                            unsigned /* numInstructions */,
                                            int codeSize ) {

#ifdef DEBUG_PA_INST
    cerr << "pd_Function::PA_attachGeneralRewrites called" <<endl;
    cerr << " prettyName = " << prettyName() << endl;
#endif

    assert((codeSize % sizeof(instruction)) == 0);

    // Iterate over function instruction by instruction, looking for calls
    // made to set the pc
    for(unsigned i=0; i < codeSize / sizeof(instruction); i++) {

        //  want CALL %address, NOT CALL %register
        if (isTrueCallInsn(loadedCode[i])) {

	    // figure out destination of call....
	    if (is_set_O7_call(loadedCode[i], codeSize, 
                               i * sizeof(instruction))) {

	        SetO7 *seto7 = new SetO7(this, i * sizeof(instruction));
		p->AddAlteration(seto7);

#ifdef DEBUG_PA_INST
	        cerr << " detected call designed to set 07 register at offset "
		     <<  i * sizeof(instruction) << endl;
#endif

	    } else {

  	        // Check for a call to a location outside of the function, 
	        // where the target of the call is a retl instruction. This 
	        // sequence is used to set the o7 register with the PC.

	        // Get target of call instruction
                Address callAddress = firstAddress + i*sizeof(instruction);
       	        Address callTarget  = callAddress + 
                                      (loadedCode[i].call.disp30 << 2);

                // If call is to location outside of function              
                if ( (callTarget < firstAddress) || 
                     (callTarget > firstAddress + size()) ) { 

                  // get target instruction
                  instruction tmpInsn;
                  tmpInsn.raw = owner->get_instruction(callTarget - baseAddress);
                  // If call target instruction is a retl instruction
                  if((tmpInsn.raw & 0xfffff000) == 0x81c3e000) {

                    // Retrieve the instruction in the delay slot of the retl,
                    // so that it can be copied into the relocated function
                    tmpInsn.raw = 
                    owner->get_instruction( (callTarget - baseAddress) + sizeof(instruction) );

                    RetlSetO7 *retlSetO7 = 
                        new RetlSetO7(this, i * sizeof(instruction), tmpInsn);
                    p->AddAlteration(retlSetO7);

#ifdef DEBUG_PA_INST
                    cerr << " detected call to retl instruction"
                         << " designed to set 07 register at offset " 
                         <<  i * sizeof(instruction) << endl;
#endif
		  }
		}
	    }
        }
    }


    // Special cases for leaf functions
    if (hasNoStackFrame()) {

      // Since we are already relocating the function, add a nop after the
      // return point so that we don't have to use prior instructions to
      // instrument the exit.
      for(unsigned i=0; i < funcReturns.size(); i++) {

        instPoint *point = funcReturns[i];

        if (!JmpNopTC(point->firstInstruction, point->secondInstruction,
                      point->iPgetAddress() - sizeof(instruction), this) &&
            !CallRestoreTC(point->firstInstruction, point->secondInstruction)){

            // Offset to insert nop after
            int offset;
 
            offset = ( point->iPgetAddress() + baseAddress + 
                       sizeof(instruction) ) - firstAddress;

            // Insert a single nop after the exit to the function
            InsertNops *exitNops = 
                new InsertNops(this, offset, sizeof(instruction));

            p->AddAlteration(exitNops);

#ifdef DEBUG_PA_INST
            cerr << "Function: " << prettyName() << " added a single nop "
                 << " after exit at offset " << offset << "." << endl;
#endif

	}
      }
    }

    return true;
}


bool pd_Function::PA_attachTailCalls(LocalAlterationSet *p) {
    instruction instr, nexti;
    TailCallOptimization *tail_call;

    // previously referred to calls[i] directly, but gdb seems to be having
    //  trouble with templated types with the new compiler - making debugging
    //  difficult - so, directly assign calls[i] to the_call so can use gdb
    //  to get at info about it....
    instPoint *the_call;

#ifdef DEBUG_PA_INST
    cerr << "pd_Function::PA_tailCallOptimizations called" <<endl;
    cerr << " prettyName = " << prettyName() << endl;
#endif


    // Look for an instPoint in funcCalls where the instruction is 
    //  a call instruction and the next instruction is a restore op
    //  or where the instruction is a jmp (out of function?), and the next 
    //  instruction is a nop....
    for(unsigned i=0; i < calls.size(); i++) {

        // this should return the offset at which the FIRST instruction which
        //  is ACTUALLY OVEWRITTEN BY INST POINT is located....
        the_call = calls[i];
        int offset = (the_call->iPgetAddress() - getAddress(0));
	instr = the_call->insnAtPoint();
	nexti = the_call->insnAfterPoint();
	if (CallRestoreTC(instr, nexti)) {
	    tail_call = new CallRestoreTailCallOptimization(this, offset, 
				   offset + 2 * sizeof(instruction), instr);
	    p->AddAlteration(tail_call);

#ifdef DEBUG_PA_INST
	    cerr << " detected call-restore tail-call optimization at offset "
		 << offset << endl;
#endif

	}
	if (JmpNopTC(instr, nexti, the_call->iPgetAddress(), this)) {
	    tail_call = new JmpNopTailCallOptimization(this, offset, 
				   offset + 2 * sizeof(instruction));
	    p->AddAlteration(tail_call);

#ifdef DEBUG_PA_INST
	    cerr << " detected jmp-nop tail-call optimization at offset " 
		 << offset << endl;
#endif

	}
    }

    return true;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/* 
  Second pass, handle cases where inst points overlap eachother (and
  thus prevent the normal instrumentation technique from being used).
    In current sparc-solaris version, the following algorithm is used:
     Check all inst points.  If any overlap, figure out the instruction
     that each is trying to get and insert nops so that they no longer
     overlap.  
*/
bool pd_Function::PA_attachOverlappingInstPoints(
		        LocalAlterationSet *p, Address /* baseAddress */, 
                        Address /* firstAddress */,
	                instruction loadedCode[], int /* codeSize */) {

    instruction instr, nexti;

#ifdef DEBUG_PA_INST
    cerr << "pd_Function::PA_attachOverlappingInstPoints called" <<endl;
    cerr << " prettyName = " << prettyName() << endl;
#endif

    // Make a list of all inst-points attached to function, and sort
    //  by address.  Then check for overlaps....
    pdvector<instPoint*> foo;    
    sorted_ips_vector(foo);

    // should hopefully have inst points for fn sorted by address....
    // check for overlaps....
    for (unsigned i=0;i<foo.size()-1;i++) {
        instPoint *this_inst_point = foo[i];
        instPoint *next_inst_point = foo[i+1];
        
        if ((this_inst_point->ipType == callSite) && 
	          (next_inst_point->ipType == functionExit)) {
          instr = this_inst_point->insnAtPoint();
          nexti = this_inst_point->insnAfterPoint();
          if (CallRestoreTC(instr, nexti) || 
      	    JmpNopTC(instr, nexti, this_inst_point->iPgetAddress(), this)) {

             // This tail call optimization will be rewritten, eliminating the
             // overlap, so we don't have to worry here about rewriting this
             // as overlapping instPoints.
             continue;
          }
	}

	// check if inst point overlaps with next inst point....
	int overlap = ((this_inst_point->iPgetAddress() + 
 	  this_inst_point->Size()) - next_inst_point->iPgetAddress());
 	if (overlap > 0) {

	    // Inst point overlaps with next one.  Insert 
	    //  InsertNops into PA Set AFTER instruction pointed to
	    //  by inst point (Making sure that this does NOT break up 
	    //  an insn and its delay slot).....
	    // ALRT ALRT : This is NOT designed to handle the case where
	    //  2 inst points are located at exactly the same place or 
	    //  1 is located in the delay slot of the other - it will NOT
	    //  break up the 2 inst points in that case....

 	    int offset = (this_inst_point->insnAddress() - getAddress(0));
            
            if (isDCTI(loadedCode[offset/sizeof(instruction)])) {
              offset += sizeof(instruction);
            } 
 	    InsertNops *nops = new InsertNops(this, offset, overlap);
 	    p->AddAlteration(nops);

#ifdef DEBUG_PA_INST
            cerr << " detected overlapping inst points : offset " << offset <<
 	      " overlap " << overlap << endl;
#endif
	}
    }
    return true;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
  Third pass, handle cases where inst points overlap branch/jump targets.
    In current sparc-solaris version, the following algorithm is used:
     Check all branch/jump operations.  If they go into footprint of 
     any inst points, then try to insert nops so that the branch targets
     move outside of the the footprint....
  Note that the algorithm given below should be attached AFTER alterations
   which unwind tail-call optimizations and seperate overlapping inst points
   have been BOTH attached and applied....
 */
bool pd_Function::PA_attachBranchOverlaps(
			     LocalAlterationSet *p, Address /* baseAddress */, 
			     Address firstAddress, instruction loadedCode[],
                             unsigned /* numberOfInstructions */, 
                             int codeSize)  {

#ifdef DEBUG_PA_INST
    cerr << "pd_Function::PA_attachBranchOverlaps called" <<endl;
    cerr << " prettyName = " << prettyName() << endl;
#endif

    // Make a list of all inst-points attached to function, and sort
    //  by address.  Then check for overlaps....
    pdvector<instPoint*> foo;
    foo.push_back(funcEntry_);
    VECTOR_APPEND(foo,funcReturns);
    VECTOR_APPEND(foo,calls);

    // Sort inst points list by address....
    VECTOR_SORT(foo, sort_inst_points_by_address);

    // Iterate over function instruction by instruction....
    assert((codeSize % sizeof(instruction)) == 0);
    for(unsigned i=0;i<(codeSize/sizeof(instruction));i++) {
        // looking for branch instructions inside function....
        if (!branchInsideRange(loadedCode[i], 
			       firstAddress + (i * sizeof(instruction)),
			       firstAddress, 
			       firstAddress + size())) {
	    continue;
	}
							      
        int disp = loadedCode[i].branch.disp22;
        Address target = firstAddress + (i * sizeof(instruction))
							    + (disp << 2);

	// branch insn inside function.  Check target versus known inst points....
	instPoint *overlap = find_overlap(foo, target);
	if (overlap == NULL) continue;

	if (target <= overlap->iPgetAddress()) {
	    InsertNops *nops = new InsertNops(this, 
                               (target - firstAddress) - sizeof(instruction), 
			       target - overlap->firstAddress());
	    p->AddAlteration(nops);

#ifdef DEBUG_PA_INST
          cerr << " detected overlap between branch target and inst point:"
               << " offset "  << target - firstAddress 
               << " # bytes " << target - overlap->firstAddress() << endl;
#endif

	} else {
	    InsertNops *nops = new InsertNops(this, 
                                 (target - firstAddress) - sizeof(instruction),
				 overlap->followingAddress() - target);
	    p->AddAlteration(nops);

#ifdef DEBUG_PA_INST
	  cerr << " detected overlap between branch target and inst point:"
               << " offset " << (target - firstAddress) - sizeof(instruction)
               << " # bytes " << overlap->firstAddress() - target << endl;
#endif

	}
    }

    return true;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

bool pd_Function::PA_attachBasicBlockEndRewrites(LocalAlterationSet *p,
						 Address baseAddress,
						 Address firstAddress,
						 process *proc) {
#ifdef BPATCH_LIBRARY

  BPatch_function *bpfunc = proc->findOrCreateBPFunc(this);

  BPatch_flowGraph *cfg = bpfunc->getCFG();
  BPatch_Set<BPatch_basicBlock*> allBlocks;
  cfg->getAllBasicBlocks(allBlocks);

  BPatch_basicBlock** belements =
    new BPatch_basicBlock*[allBlocks.size()];
  allBlocks.elements(belements);

  BPatch_Set<Address> blockEnds;
  for (int i = 0; i < allBlocks.size(); i++) {
    void *bbsa, *bbea;
    belements[i]->getAddressRange(bbsa, bbea);
    blockEnds.insert((Address) bbea);
  }
  delete[] belements;

  pdvector<instPoint*> ips;
  sorted_ips_vector(ips);

  instPoint *entry = const_cast<instPoint*>(funcEntry_);
  Address entry_begin, entry_end;
  entry_begin = entry->iPgetAddress();
  entry_end = entry_begin + entry->Size();

  for (unsigned i = 0; i < ips.size(); i++) {
    Address curr_addr = ips[i]->iPgetAddress();
    // Skip inst points inside entry block
    if (entry_begin <= curr_addr && curr_addr < entry_end)
      continue;

    // We do a special check to see if we're the restore in a tail-call
    // optimization.  This should be probably be addressed in
    // fixOverlappingAlterations
    if (blockEnds.contains(curr_addr) &&
	!isInsnType(ips[i]->insnAtPoint(), RESTOREmask, RESTOREmatch)) {
      
      InsertNops *blockNop = 
	new InsertNops(this, curr_addr + baseAddress +
		       sizeof(instruction) - firstAddress, 
		       sizeof(instruction));
      p->AddAlteration(blockNop);
      
    }
  }

#endif BPATCH_LIBRARY
  return true;
}



/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// Return the offset (in bytes) of the next instruction at or after
//  offset (again bytes) which is NOT in the delay slot of another
//  instruction....
int pd_Function::moveOutOfDelaySlot(int offset, instruction loadedCode[],
	  int codeSize) {
    assert(offset >= 0 && offset < codeSize);
    if (isDCTI(loadedCode[offset/sizeof(instruction)-1])) {
        return offset + sizeof(instruction);
    }
    return offset;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#ifdef NOT_DEFINED
#define UPDATE_ADDR(point) \
    if (point != NULL) { \
        offset = point->Addr() - oldAdr; \
	shift = p->GetInstPointShift();  \
        if (shift != 0) {                \
	    p->SetAddr(p->Addr() = shift); \
	} \
    }
bool pd_Function::applyAlterationsToInstPoints(LocalAlterationSet *p, 
        relocatedFuncInfo *info, Address oldAdr) {
    instPoint *point;
    int offset, shift, i;    

    // try to handle entry point....
    point = info->funcEntry();
    UPDATE_ADDR(point);

    // ditto for calls....
    for(i=0;i<funcCalls().size();i++) {
        point = funcCalls[i];
	UPDATE_ADDR(point);
    }

    // and for exit points....
    for(i=0;i<funcReturns().size();i++) {
        point = funcReturns[i];
	UPDATE_ADDR(point);
    }
}
#endif

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// Fill in relocatedFuncInfo <func> with inst points from function.
//  Notes:
//    Assumes that function inst points are : 1 entry, multiple call site, and
//     multiple exit points, and that all should be translated into inst
//     points in relocated function (in reloc_info).  This probably shld get 
//     changed if Trash-A-Dyn/Dyninst moves to a model of instrumenting arbitrary
//     points in function....
//    When this function finishes, inst points in reloc info should have their
//     NEW (relocated) address, including changes made by alterations applied
//     to function.
//    3 LocalAlterationSet s are used because that is the number which a relatively
//     straight-forward coding of the logic to find alterations used - other
//     platforms which have different logic for attaching alterations will 
//     probably have more or less than 3 alteration sets....
//    This function preserves the (probably memory leaking) semantics of Tia's original
//     code for filling in reloc_info in findNewInstPoints - it does NOT delete 
//     created inst points on error....
#define CALC_OFFSETS(ip) \
    originalOffset = ((ip->insnAddress() + imageBaseAddr) - mutatee); \
    newOffset = alteration_set.getInstPointShift(originalOffset + 1); \
    assert(((originalOffset + newOffset) % sizeof(instruction)) == 0); \
    arrayOffset = (originalOffset + newOffset) / sizeof(instruction); \
    tmp = (newAdr + originalOffset + newOffset) - imageBaseAddr; \
    tmp2 = ip->iPgetAddress();

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void pd_Function::addArbitraryPoint(instPoint* location,
				    process* proc,
				    relocatedFuncInfo* reloc_info)
{

    if(!isInstalled(proc))
	return;

    instPoint *point;
    int originalOffset, newOffset, arrayOffset;
    Address tmp, tmp2,mutatee,newAdr,mutator;
    LocalAlterationSet alteration_set(this);

    instruction *oldInstructions = NULL, *newCode = NULL;


    const image* owner = location->iPgetOwner();

    findAlterations(owner,proc,oldInstructions,alteration_set,
                                    mutator, mutatee);
    Address imageBaseAddr;
    if (!proc->getBaseAddress(owner,imageBaseAddr))
        abort();

    newAdr = reloc_info->address();
    CALC_OFFSETS(location);

    newCode  = reinterpret_cast<instruction *> (relocatedCode);
    point = new instPoint(this, newCode, arrayOffset, owner,
			             tmp, true, otherPoint);

    reloc_info->addArbitraryPoint(point);

    delete[] oldInstructions;
}


bool pd_Function::fillInRelocInstPoints(
                            const image *owner, process *proc, 
                            instPoint *&location, 
                            relocatedFuncInfo *&reloc_info, Address mutatee,
                            Address /* mutator */ ,instruction /* oldCode */[],
                            Address newAdr, instruction newCode[], 
                            LocalAlterationSet &alteration_set) { 

    unsigned retId = 0;
    unsigned callId = 0; 
    unsigned tmpId = 0;
    unsigned arbitraryId = 0;
    int originalOffset, newOffset, arrayOffset;
    instPoint *point;
    Address tmp, tmp2;

    assert(reloc_info);

#ifdef DEBUG_PA_INST
    cerr << "pd_Function::fillInRelocInstPoints called" <<endl;
    cerr << " mutatee = " << mutatee << " newAdr = " << newAdr << endl;
#endif
 
    Address imageBaseAddr;
    if (!proc->getBaseAddress(owner, imageBaseAddr))
        abort();

    alteration_set.Collapse();

    //  Add inst point corresponding to func entry....
    //   Assumes function has single entry point  
    if (funcEntry_ != NULL) {

        //  figure out how far entry inst point is from beginning of function
        originalOffset = ((funcEntry_->iPgetAddress() + imageBaseAddr) - 
                                                              mutatee);
        arrayOffset = originalOffset / sizeof(instruction);
        tmp         = (newAdr + originalOffset) - imageBaseAddr;
        tmp2        = funcEntry_->iPgetAddress();

        point = new instPoint(this, newCode, arrayOffset, owner, 
	                         tmp, true, functionEntry);



#ifdef DEBUG_PA_INST
        cerr << " added entry point at originalOffset " << originalOffset
	     << " newOffset " << originalOffset << endl;
#endif

	assert(point != NULL);

	if (location == funcEntry_) {
	    location = point;
	}

	reloc_info->addFuncEntry(point);
        assert(reloc_info->funcEntry());
    }

    // Add inst points corresponding to func exits....
    for(retId=0;retId < funcReturns.size(); retId++) {
        CALC_OFFSETS(funcReturns[retId])
        point = new instPoint(this, newCode, arrayOffset, owner, 
                                 tmp, false, functionExit);
#ifdef DEBUG_PA_INST
        cerr << " added return point at originalOffset " << originalOffset
	     << " newOffset " << newOffset << endl;
#endif
        assert(point != NULL);

	if (location == funcReturns[retId]) {
	    location = point;
	}
	reloc_info->addFuncReturn(point);
    } 

    // Add inst points corresponding to func call sites....
    for(callId=0;callId<calls.size();callId++) {
        CALC_OFFSETS(calls[callId])
	tmpId = callId;
        point = new instPoint(this, newCode, arrayOffset, owner, tmp, false,
                                                             callSite);
        addCallPoint(newCode[arrayOffset], tmpId, reloc_info, point, 
                     const_cast<const instPoint *&> (location));

#ifdef DEBUG_PA_INST
        cerr << " added call site at originalOffset " << originalOffset
	     << " newOffset " << newOffset << endl;
#endif

    }

    for(arbitraryId=0;arbitraryId<arbitraryPoints.size();arbitraryId++){

	CALC_OFFSETS(arbitraryPoints[arbitraryId]);

	point = new instPoint(this, newCode, arrayOffset, owner,
			            tmp, true, otherPoint);

        assert(point != NULL);

	if (location == arbitraryPoints[arbitraryId]) 
		location = point;

	reloc_info->addArbitraryPoint(point);
    }

    return true;    
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

//
// Read instructions in function code from image into <into> (array)....
bool pd_Function::readFunctionCode(const image *owner, instruction *into) {
    int i = 0;

    Address firstAddress = getAddress(0);
    Address lastAddress = firstAddress + size();

    while (firstAddress < lastAddress) {
        into[i++].raw = owner->get_instruction(firstAddress);
	firstAddress += sizeof(instruction);
    }

    return true;
}


/****************************************************************************/

/*
 * Return the displacement of the relative call or branch instruction.
 * 
 */
int get_disp(instruction *insn) 
{
  int disp = 0;
  
  // If the instruction is a CALL instruction
  if (isInsnType(*insn, CALLmask, CALLmatch)) {
    disp = (insn->call.disp30 << 2);
  } 
  else {

    // If the instruction is a Branch instruction
    if (isInsnType(*insn, BRNCHmask, BRNCHmatch)||
	isInsnType(*insn, FBRNCHmask, FBRNCHmatch)) {
      disp = (insn->branch.disp22 << 2);
    }
  }
  return disp;
}

/****************************************************************************/

/****************************************************************************/

/*
 * Upon setDisp being true, set the target of a relative jump or call insn.
 * Otherwise, return 0
 * 
 */
int set_disp(bool setDisp, instruction *insn, int newOffset, bool /* outOfFunc */ ) 
{
  
  if ( setDisp ) {
    // If the instruction is a CALL instruction
    if (isInsnType(*insn, CALLmask, CALLmatch)) {
      insn->call.disp30 = newOffset >> 2;
    }
    else {

      // If the instruction is a Branch instruction
      if (isInsnType(*insn, BRNCHmask, BRNCHmatch)||
	  isInsnType(*insn, FBRNCHmask, FBRNCHmatch)) {
        insn->branch.disp22 = newOffset >> 2;
      }
    }
  }

  return 0;
}

/****************************************************************************/

bool pd_Function::loadCode(const image *owner, process* /* proc */, 
                           instruction *&oldCode, 
                           unsigned &numberOfInstructions, 
                           Address& /* firstAddress */) {

#ifdef DEBUG_PA_INST
    cerr << "pd_Function::loadCode called " << endl;
    cerr << " prettyName = " << prettyName().c_str() << endl;
    cerr << " size() = " << size() << endl;
#endif


    // check that function has size > 0.... 
    if (size() == 0) {
      cerr << "WARN : attempt to relocate function " <<  prettyName()  
           << " with size 0, unable to instrument"   <<  endl;
        return false;
    }

    //
    //  LOAD CODE FROM ADDRESS SPACE INTO ARRAY OF INSTRUCTIONS....
    //
    // allocate array of instructions to original code to be read from
    //  addr space

    assert(size() % sizeof(instruction) == 0);
    numberOfInstructions = size() / sizeof(instruction);

    oldCode = new instruction[numberOfInstructions];

    // if unable to allocate array, dump, warn and return false.... 
    if (oldCode == NULL) {
      cerr << "WARN : unable to allocate array (" << size()
           << " bytes) to read in code for function" 
           << prettyName() << ": unable to instrument." << endl;

      return false;
    }

    // read function code from image....
    if (!readFunctionCode(owner, oldCode)) {
      cerr << "WARN : unable to read code for function " << prettyName()
	   << " from address space : unable to instrument" << endl;

      delete []oldCode;
      return false;
    }

    return true;
}

/****************************************************************************/
/****************************************************************************/

// return the address of the sparc instruction
int addressOfMachineInsn(instruction *insn) {
  return (int)insn;
}

// Return the size of the sparc instruction (4 bytes)
int sizeOfMachineInsn(instruction * /* insn */) {
  return sizeof(instruction);
}

/***************************************************************************/
/***************************************************************************/

// Copy machine code from one location (in mutator) to another location
// (also in the mutator)
void pd_Function::copyInstruction(instruction &newInsn, instruction &oldInsn, 
                                                        unsigned &codeOffset) {
  newInsn = oldInsn;
  codeOffset += sizeof(instruction);
}

bool pd_Function::isNearBranchInsn(const instruction insn) {
  return ( isBranchInsn(insn) );
}

/***************************************************************************/
/***************************************************************************/

// It may be that in two different passes over a function, we note 
// two different LocalAlterations at the same offset. This function 
// reconciles any conflict between the LocalAlterations and merges them into 
// a single LocalAlteration, or discards one of the alterations, returning 
// the relevant alteration

LocalAlteration *fixOverlappingAlterations(LocalAlteration *alteration, 
                                           LocalAlteration *tempAlteration) {

  LocalAlteration *alt = 0, *tmpAlt = 0; 

#ifdef DEBUG_FUNC_RELOC 
	   cerr << "fixOverlappingAlterations " << endl;
#endif

  // assert that there is indeed a conflict  
  assert (alteration->getOffset() == tempAlteration->getOffset()); 

  alt = dynamic_cast<InsertNops *> (alteration); 
  tmpAlt = dynamic_cast<InsertNops *> (tempAlteration); 
  if (alt != NULL) {
    if (tmpAlt != NULL) {
      if (alteration->getShift() >= tempAlteration->getShift()) {
        return alteration;
      } else {  
          return tempAlteration;
      }
    }
  }

  /* Check for tail call optimization and set o7 sequence at same location */

  alt = dynamic_cast<CallRestoreTailCallOptimization *>(alteration); 
  tmpAlt = dynamic_cast<SetO7 *> (tempAlteration); 
  if (alt != NULL) {
    if (tmpAlt != NULL) {
       return alteration;
    }
  }

  alt = dynamic_cast<CallRestoreTailCallOptimization *>(alteration); 
  tmpAlt = dynamic_cast<RetlSetO7 *> (tempAlteration); 
  if (alt != NULL) {
    if (tmpAlt != NULL) {
       return alteration;
    }
  }

  alt = dynamic_cast<JmpNopTailCallOptimization *>(alteration); 
  tmpAlt = dynamic_cast<SetO7 *> (tempAlteration); 
  if (alt != NULL) {
    if (tmpAlt != NULL) {
        return alteration;
    }
  }
  alt = dynamic_cast<JmpNopTailCallOptimization *>(alteration); 
  tmpAlt = dynamic_cast<RetlSetO7 *> (tempAlteration); 
  if (alt != NULL) {
    if (tmpAlt != NULL) {
        return alteration;
    }
  }

  cerr << "WARNING: Conflicting Alterations in function relocation" << endl;
  return NULL;
}

InsnRegister::InsnRegister() 
	: wordCount(0),
	  regType(InsnRegister::None),
	  regNumber(-1) {};

InsnRegister::InsnRegister(char isD,InsnRegister::RegisterType rt,
			   unsigned short rn)
	: wordCount(isD),
	  regType(rt),
	  regNumber(rn) {};

void InsnRegister::setWordCount(char isD) { wordCount = isD; }

void InsnRegister::setType(InsnRegister::RegisterType rt) { regType = rt ; }

void InsnRegister::setNumber(short rn) { regNumber = rn ; }

bool InsnRegister::is_o7(){
	if(regType == InsnRegister::GlobalIntReg)
		for(int i=0;i<wordCount;i++)
			if((regNumber+i) == 0xf)
				return true;
	return false;
}

void InsnRegister::print(){
	switch(regType){
		case InsnRegister::GlobalIntReg: 
			cerr << "R[ "; break;
		case InsnRegister::FloatReg    : 
			cerr << "F[ "; break;
		case InsnRegister::CoProcReg   : 
			cerr << "C[ "; break;
		case InsnRegister::SpecialReg  : 
			cerr << "S[ "; break;
		default : 
			cerr << "NONE[ "; break;
	}

	if((regType != InsnRegister::SpecialReg) &&
	   (regType != InsnRegister::None))
		for(int i=0;i<wordCount;i++)
			cerr << regNumber+i;
	cerr << "] ";
}


void get_register_operands(const instruction& insn,
		  InsnRegister* rd,InsnRegister* rs1,InsnRegister* rs2)
{
	*rd = InsnRegister();
	*rs1 = InsnRegister();
	*rs2 = InsnRegister();
	
	if(!IS_VALID_INSN(insn))
		return;

	switch(insn.call.op){
		case 0x0:
		    {
			if((insn.sethi.op2 == 0x4) &&
		   	   (insn.sethi.rd || insn.sethi.imm22))
				*rd = InsnRegister(1,
						  InsnRegister::GlobalIntReg,
						  (short)(insn.sethi.rd));
			break;
		    }
		case 0x2:
		    {
			unsigned firstTag = insn.rest.op3 & 0x30;
			unsigned secondTag = insn.rest.op3 & 0xf;

			if((firstTag == 0x00) ||
			   (firstTag == 0x10) ||
			   ((firstTag == 0x20) && (secondTag < 0x8)) ||
			   ((firstTag == 0x30) && (secondTag >= 0x8)) ||
			   ((firstTag == 0x30) && (secondTag < 0x4)))
			{
				*rs1 = InsnRegister(1,
                                 		    InsnRegister::GlobalIntReg,
                                		    (short)(insn.rest.rs1));
				if(!insn.rest.i)
					*rs2 = InsnRegister(1,
						    InsnRegister::GlobalIntReg,
					            (short)(insn.rest.rs2));

				if((firstTag == 0x30) && (secondTag < 0x4))
					*rd = InsnRegister(1,
                                 		    InsnRegister::SpecialReg,
                                		    -1);
				else if((firstTag != 0x30) || 
					(secondTag <= 0x8) || 
				        (secondTag >= 0xc))
					*rd = InsnRegister(1,
                                 		    InsnRegister::GlobalIntReg,
                                		    (short)(insn.rest.rd));
			}
			else if((firstTag == 0x20) && (secondTag >= 0x8))
			{
				*rs1 = InsnRegister(1,
                                                    InsnRegister::SpecialReg,
                                                    -1);
				*rd = InsnRegister(1,
                                                    InsnRegister::GlobalIntReg,
                                                    (short)(insn.rest.rd));
			}
			else if((secondTag == 0x6) || (secondTag == 0x7))
			{
				*rs1 = InsnRegister(1,
                                 		    InsnRegister::CoProcReg,
                                		    (short)(insn.restix.rs1));
				*rs2 = InsnRegister(1,
                                 		    InsnRegister::CoProcReg,
                                		    (short)(insn.restix.rs2));
				*rd = InsnRegister(1,
                                 		    InsnRegister::CoProcReg,
                                		    (short)(insn.restix.rd));
			}
			else if((secondTag == 0x4) || (secondTag == 0x5))
			{
				char wC = 0;
				switch(insn.rest.unused & 0x03){
					case 0x0:
					case 0x1: wC = 1; break;
					case 0x2: wC = 2; break;
					case 0x3: wC = 4; break;
					default: break;	
				}
				
				*rs2 = InsnRegister(wC,
                                 		    InsnRegister::FloatReg,
                                		    (short)(insn.rest.rs2));

				firstTag = insn.rest.unused & 0xf0;
				secondTag = insn.rest.unused & 0xf;
				if((firstTag == 0x40) || (firstTag == 0x60)){
					*rs1 = *rs2;
					rs1->setNumber((short)(insn.rest.rs1));
				}

				if(firstTag < 0x60){
					*rd = *rs2;
					rd->setNumber((short)(insn.rest.rd));
				}
				else{
					if(secondTag < 0x8)
						wC = 1;
					else if(secondTag < 0xc)
						wC = 2;
					else
						wC = 4;

					*rd = InsnRegister(wC,
                                                    InsnRegister::FloatReg,
                                                    (short)(insn.rest.rd));
				}
			}

			break;
		    }
		case 0x3:
		    {
			*rs1 = InsnRegister(1,
					    InsnRegister::GlobalIntReg,
					    (short)(insn.rest.rs1));
			if(!insn.rest.i)
				*rs2 = InsnRegister(1,
						    InsnRegister::GlobalIntReg,
					            (short)(insn.rest.rs2));
			char wC = 1;
			InsnRegister::RegisterType rt;
			short rn = -1;

			unsigned firstTag = insn.rest.op3 & 0x30;
			unsigned secondTag = insn.rest.op3 & 0xf;

			switch(firstTag){
				case 0x00:
				case 0x10:
					rt = InsnRegister::GlobalIntReg;
					rn = (short)(insn.rest.rd);
					break;
				case 0x20:
				case 0x30:
					if((secondTag == 0x1) ||
					   (secondTag == 0x5) ||
					   (secondTag == 0x6))
						rt = InsnRegister::SpecialReg;
					else{
					    if(firstTag == 0x20)
						rt = InsnRegister::FloatReg;
					    else if(firstTag == 0x30)
						rt = InsnRegister::CoProcReg;
					    rn = (short)(insn.rest.rd);
				     	}
					break;
				default: break;
			}
			if((secondTag == 0x3) ||
			   (secondTag == 0x7))
				wC = 2;

			rd->setNumber(rn);
			rd->setType(rt);
			rd->setWordCount(wC);
			break;
		    }
		default:
			break;
	}
	return;
}
