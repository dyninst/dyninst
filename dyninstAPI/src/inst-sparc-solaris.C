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

// $Id: inst-sparc-solaris.C,v 1.42 1998/08/26 20:59:04 zhichen Exp $

#include "dyninstAPI/src/inst-sparc.h"
#include "dyninstAPI/src/instPoint.h"
#include "util/h/debugOstream.h"

#include "dyninstAPI/src/FunctionExpansionRecord.h"

static unsigned pfdp_to_pfdp_hash(pd_Function * const &f) {
    pd_Function *pdf = f;
    unsigned l = (unsigned)pdf;
    return addrHash4(l); 
}

// Another constructor for the class instPoint. This one is called
// for the define the instPoints for regular functions which means
// multiple instructions is going to be moved to based trampoline.
// Since we will use the instruction CALL to branch to the base
// tramp(so it doesn't have any code size restriction), things are
// a little more complicated because instruction CALL changes the 
// value in the link register.
instPoint::instPoint(pd_Function *f, const instruction &instr, 
		     const image *owner, Address &adr,
		     bool delayOK,
		     instPointType pointType)
: addr(adr), originalInstruction(instr), inDelaySlot(false), isDelayed(false),
  callIndirect(false), callAggregate(false), callee(NULL), 
  func(f),
  ipType(pointType), image_ptr(owner), firstIsConditional(false),
  relocated_(false), isLongJump(false)
{

  isBranchOut = false;
  size = 0;

  // When the function has a stack frame
  if (!this->hasNoStackFrame()) {

      // we will treat the first instruction after the SAVE instruction
      // in the nonleaf procedure as the function entry.  
      if (ipType == functionEntry) {

	  assert(isInsnType(instr, SAVEmask, SAVEmatch));
	  saveInsn.raw = owner->get_instruction(addr);
	  addr += 4;
	  originalInstruction.raw = owner->get_instruction(addr);
	  delaySlotInsn.raw = owner->get_instruction(addr+4);
	  size += 2*sizeof(instruction);

	  // If the second instruction is DCTI, we need to move the
	  // the instruction in the delayed slot.
	  if (IS_DELAYED_INST(delaySlotInsn)) {
	      isDelayed = true; 
	      isDelayedInsn.raw = owner->get_instruction(addr+8);
	      size += 1*sizeof(instruction);

	      // Life is Hard. If the second instruction is actually
	      // an CALL instruction, we need to move the instruction
	      // after the instruction in the delayed slot if the 
	      // return value of this function is a aggregate value.
	      aggregateInsn.raw = owner->get_instruction(addr+12);
	      if (isCallInsn(delaySlotInsn)) {
		  if (!IS_VALID_INSN(aggregateInsn) && aggregateInsn.raw != 0) {
		      callAggregate = true;
		      size += 1*sizeof(instruction);
		  }
	      }
	  }

      // The following are easier.	  
      } else if (ipType == callSite) {
	  delaySlotInsn.raw = owner->get_instruction(addr+4);
	  size += 2*sizeof(instruction);

	  aggregateInsn.raw = owner->get_instruction(addr+8);
	  if (!IS_VALID_INSN(aggregateInsn) && aggregateInsn.raw != 0) {
	      callAggregate = true;
	      size += 1*sizeof(instruction);
	  }
      } else {
	  delaySlotInsn.raw = owner->get_instruction(addr+4);
	  size += 2*sizeof(instruction);
      }
  }

  // When the function is a leaf function
  else {

      // For the leaf procedure, there are no function calls in
      // this procdure. So we don't need to consider the 
      // aggregate instuction.
      if (ipType == functionEntry) {

	  otherInstruction.raw = owner->get_instruction(addr+4);
	  delaySlotInsn.raw = owner->get_instruction(addr+8);
	  size += 2*sizeof(instruction);

	  if (IS_DELAYED_INST(delaySlotInsn)) {
	      isDelayed = true;
	      isDelayedInsn.raw = owner->get_instruction(addr+12);
	      size += 2*sizeof(instruction);
	  }

      } else if (ipType == functionExit) {
	  
	  addr -= 4;

	  if (owner->isValidAddress(addr-4)) {
	      instruction iplus1;
	      iplus1.raw = owner->get_instruction(addr-4);
	      if (IS_DELAYED_INST(iplus1) && !delayOK) {
		  addr -= 4;
		  inDelaySlot = true;
		  size += 1*sizeof(instruction);
		  if(isCondBranch(iplus1)){
		      instruction previous_inst; 
		      previous_inst.raw = owner->get_instruction(addr-4);
                      firstIsConditional = true;
		      addr -= sizeof(instruction);
		      size += 1*sizeof(instruction);
		  }
	      }
	  }

	  originalInstruction.raw = owner->get_instruction(addr);
	  otherInstruction.raw = owner->get_instruction(addr+4);
	  delaySlotInsn.raw = owner->get_instruction(addr+8);
	  size += 3*sizeof(instruction);

	  if (inDelaySlot) {
	      inDelaySlotInsn.raw = owner->get_instruction(addr+12);
	      if(firstIsConditional) {
		  extraInsn.raw = owner->get_instruction(addr+16);
	      }
	  }

      } else {
	  assert(ipType == callSite);
	  // Usually, a function without a stack frame won't have any call sites
	  extern debug_ostream metric_cerr;
	  metric_cerr << "inst-sparc-solaris.C WARNING: found a leaf fn (no stack frame)" << endl;
	  metric_cerr << "which makes a function call" << endl;
	  metric_cerr << "This fn is " << func->prettyName() << endl;

	  //abort();
      }
  }

  // return the address in the code segment after this instruction
  // sequence. (there's a -1 here because one will be added up later in
  // the function findInstPoints)  
  adr = addr + (size - 1*sizeof(instruction));
}


void AstNode::sysFlag(instPoint *location)
{
    if (location -> ipType == functionEntry) {
	astFlag = (location -> isLongJump)? false:true; 
    } else if (location -> ipType == functionExit) {
       astFlag = location -> hasNoStackFrame(); // formerly "isLeaf()"
    } else
	astFlag = false;

    if (loperand)
	loperand->sysFlag(location);
    if (roperand)
	roperand->sysFlag(location); 

    for (unsigned u = 0; u < operands.size(); u++) {
	operands[u]->sysFlag(location);
    }
}

// Determine if the called function is a "library" function or a "user" function
// This cannot be done until all of the functions have been seen, verified, and
// classified
//
void pd_Function::checkCallPoints() {
  instPoint *p;
  Address loc_addr;

  //cerr << "pd_Function:: checkCallPoints called, *this = " << *this;

  vector<instPoint*> non_lib;

  for (unsigned i=0; i<calls.size(); ++i) {
    /* check to see where we are calling */
    p = calls[i];
    assert(p);

    if (isInsnType(p->originalInstruction, CALLmask, CALLmatch)) {
      //cerr << " isIsinType TRUE" << endl;
      // Direct call
      loc_addr = p->addr + (p->originalInstruction.call.disp30 << 2);
      pd_Function *pdf = (file_->exec())->findFunction(loc_addr);
      if (pdf) {
	p->callee = pdf;
	non_lib += p;
	//cerr << "  pdf (called func?) non-NULL = " << *pdf;
      } else if(!pdf){
	   //cerr << "  pdf (called func) NULL" << endl;
	   // if this is a call outside the fuction, keep it
	   if((loc_addr < getAddress(0))||(loc_addr > (getAddress(0)+size()))){
	        //cerr << "   apparent call outside function, adding p to non_lib"
	        //     << endl;
	        p->callIndirect = true;
                p->callee = NULL; 
                non_lib += p;
	   }
	   else {
	       //cerr << "   apparent call inside function, deleting p" << endl;
	       delete p;
	   }
      } 
    } else {
      //cerr << " isIsinType FALSE, assuming call to unnamed user function" << endl;
      // Indirect call -- be conservative, assume it is a call to 
      // an unnamed user function
      assert(!p->callee); assert(p->callIndirect);
      p->callee = NULL;
      non_lib += p;
    }
  }
  calls = non_lib;
}

// TODO we cannot find the called function by address at this point in time
// because the called function may not have been seen.
// reloc_info is 0 if the function is not currently being relocated
Address pd_Function::newCallPoint(Address &adr, const instruction instr,
				 const image *owner, bool &err, 
				 int &callId, Address &oldAddr,
				 relocatedFuncInfo *reloc_info,
				 const instPoint *&location)
{
    Address ret=adr;
    instPoint *point;

#ifdef DEBUG_CALL_POINTS
    cerr << "pd_Function::newCallPoint called " << endl;
    cerr << " this " << *this << endl;
    cerr << " adr = " << adr << endl;
    cerr << " isTrap = " << isTrap << endl;
    cerr << " reloc_info = " << reloc_info << endl;
#endif

    err = true;
    if (isTrap) {
	point = new instPoint(this, instr, owner, adr, false, callSite, oldAddr);
    } else {
	point = new instPoint(this, instr, owner, adr, false, callSite);
    }

    if (!isInsnType(instr, CALLmask, CALLmatch)) {
      point->callIndirect = true;
      point->callee = NULL;
    } else{
      point->callIndirect = false;
    }

    if (isTrap) {
	if (!reloc_info) {
	    calls += point;
	    calls[callId] -> instId = callId++;
	} else {
	    // calls to a location within the function are not
	    // kept in the calls vector
	    assert(callId >= 0);

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

	    point->relocated_ = true;

	    // Alert!!!!
	    // cannot simply assert that this is true, because of the case
	    //  where (as a hack), the call site in a tail-call optimization
	    //  might not have been previously seeen....
	    assert(((u_int)callId) < calls.size());
	  
	    if(location && (calls[callId] == location)) { 
		assert(calls[callId]->instId  == location->instId);
		location = point; 
	    } 
	   
	    point->instId = callId++;
	    reloc_info->addFuncCall(point);
	}
    } else {
	if (!reloc_info) {
	    calls += point;
	}
	else {
	    point->relocated_ = true;
	    reloc_info->addFuncCall(point);
	}
    }
    err = false;
    return ret;
}

/*
 * Given and instruction, relocate it to a new address, patching up
 *   any relative addressing that is present.
 * 
 */
void relocateInstruction(instruction *insn, u_int origAddr, u_int targetAddr,
			 process *proc)
{
    int newOffset;

    // If the instruction is a CALL instruction, calculate the new
    // offset
    if (isInsnType(*insn, CALLmask, CALLmatch)) {
	newOffset = origAddr  - targetAddr + (insn->call.disp30 << 2);
	insn->call.disp30 = newOffset >> 2;
    } else if (isInsnType(*insn, BRNCHmask, BRNCHmatch)||
	       isInsnType(*insn, FBRNCHmask, FBRNCHmatch)) {

	// If the instruction is a Branch instruction, calculate the 
        // new offset. If the new offset is out of reach after the 
        // instruction is moved to the base Trampoline, we would do
        // the following:
	//    b  address  ......    address: save
	//                                   call new_offset             
	//                                   restore 
	newOffset = origAddr - targetAddr + (insn->branch.disp22 << 2);

	// if the branch is too far, then allocate more space in inferior
	// heap for a call instruction to branch target.  The base tramp 
	// will branch to this new inferior heap code, which will call the
	// target of the branch
	if (!offsetWithinRangeOfBranchInsn(newOffset)) {
//	if (ABS(newOffset) > getMaxBranch1Insn()) {
	    int ret = inferiorMalloc(proc,3*sizeof(instruction), textHeap);
	    u_int old_offset = insn->branch.disp22 << 2;
	    insn->branch.disp22  = (ret - targetAddr)>>2;
	    instruction insnPlus[3];
	    genImmInsn(insnPlus, SAVEop3, REG_SP, -112, REG_SP);
	    generateCallInsn(insnPlus+1, ret+sizeof(instruction), 
			     origAddr+old_offset);
	    genSimpleInsn(insnPlus+2, RESTOREop3, 0, 0, 0); 
	    proc->writeDataSpace((caddr_t)ret, sizeof(insnPlus), 
			 (caddr_t) insnPlus);
	} else {
	    insn->branch.disp22 = newOffset >> 2;
	}
    } else if (isInsnType(*insn, TRAPmask, TRAPmatch)) {
	// There should be no probelm for moving trap instruction
	// logLine("attempt to relocate trap\n");
    } 
    /* The rest of the instructions should be fine as is */
}

/*
 * Install a base tramp -- fill calls with nop's for now.
 *
 * This one install the base tramp for the regular functions.
 *
 */
trampTemplate *installBaseTramp(instPoint *&location, process *proc)
{
    unsigned baseAddr = inferiorMalloc(proc, baseTemplate.size, textHeap);

    instruction *code = new instruction[baseTemplate.size];
    assert(code);

    memcpy((char *) code, (char*) baseTemplate.trampTemp, baseTemplate.size);

    instruction *temp;
    unsigned currAddr;
    for (temp = code, currAddr = baseAddr; 
	(currAddr - baseAddr) < (unsigned) baseTemplate.size;
	temp++, currAddr += sizeof(instruction)) {

	if (temp->raw == EMULATE_INSN) {

	    // Load the value of link register from stack 
	    // If no stack frame, genereate a RESTORE instruction
            // since there's an instruction SAVE generated and put in the
            // code segment.
	    if (location -> hasNoStackFrame()) {

		Address baseAddress = 0;
		proc->getBaseAddress(location->image_ptr,baseAddress);
		baseAddress += location -> addr;

		if (in1BranchInsnRange(baseAddress, baseAddr) == false) {
		    //cerr << "This happen very rarely, I suppose "<< endl;
		    //cerr << "Lets see if this is going to be executed..." << endl;
		    location -> isLongJump = true;
		    genImmInsn(temp, RESTOREop3, 0, 0, 0);
		} else {
		    generateNOOP(temp);
		}
		temp++;
		currAddr += sizeof(instruction);
	    } 
	    // Same for the leaf and nonleaf functions.
            // First, relocate the "FIRST instruction" in the sequence;  
	    Address fromAddr = location->addr;

	    if (!(location -> hasNoStackFrame())) {
		if (location -> ipType == functionEntry) {
		    *temp = location -> saveInsn;
		    temp++;
		    currAddr += sizeof(instruction);
		}
            }
	    *temp = location->originalInstruction;

	    // compute the real from address if this instrumentation
	    // point is from a shared object image
	    Address baseAddress = 0;
	    if(proc->getBaseAddress(location->image_ptr,baseAddress)){
		fromAddr += baseAddress;		
            }

            // If the instruction is a call instruction to a location somewhere 
	    // within the function, then the 07 regester must be saved and 
	    // resored around the relocated call from the base tramp...the call
	    // instruction changes the value of 07 to be the PC value, and if
	    // we move the call instruction to the base tramp, its value will
	    // be incorrect when we use it in the function.  We generate the
	    // following base tramp code:
	    //		original delay slot instruction 
	    //		save
	    // 		original call instruction
	    //		restore
            // This case should only occur for function entry points in
	    // functions from shared objects, and there should be no append
	    // trampolene code because the relocated call instruction will
	    // not return to the base tramp
	    if (isInsnType(*temp, CALLmask, CALLmatch)) {
		Address offset = fromAddr + (temp->call.disp30 << 2);
		if ((offset > (location->func->getAddress(0)+ baseAddress)) && 
		    (offset < ((location->func->getAddress(0)+ baseAddress)+
				 location->func->size()))) {
		    // offset > adr; "=" means recursive function which is allowed
		    // offset < adr + size; "=" does not apply to this case

		    // TODO: this assumes that the delay slot instruction is not
		    // a call instruction....is this okay?
		    
		    // assume this situation only happens at function entry point 
		    // for the shared library routine. And it is definately nees
		    // long jump support
		    assert(location -> ipType == functionEntry); 
		    location -> isLongJump = true;
 		    
		    // In this situation, save instruction is discarded
		    // Rollback!! 
		    assert(location->hasNoStackFrame() == false);
		    temp--;
		    currAddr -= sizeof(instruction);
		    
		    *temp = location->delaySlotInsn;  
		    temp++; 
		    currAddr += sizeof(instruction);
		    genImmInsn(temp, SAVEop3, REG_SP, -112, REG_SP); 
		    temp++; 
		    currAddr += sizeof(instruction);  
		    *temp = location->originalInstruction;
		    relocateInstruction(temp,fromAddr,currAddr,(process *)proc);
		    temp++; 
		    fromAddr += sizeof(instruction); 
		    currAddr += sizeof(instruction);
		    genImmInsn(temp, RESTOREop3, 0, 0, 0);
		    continue;
		}
	    }   

	    relocateInstruction(temp,fromAddr,currAddr,(process *)proc);

	    // Again, for leaf function, one more is needed to move for one
	    // more spot;
	    if (location->hasNoStackFrame()) {
		// check to see if the otherInstruction is a call instruction
		// to itself, if so then generate the following
		// before 		after 		basetramp
		// ------		-----		---------
		// mov   originalInsn	mov		sethi
		// call  otherInsn 	call 		save
		// sethi delaySlot	nop		call
		//					restore
		// the idea is to not really relocate the originalInsn, and
		// relocate only the call otherInsn and delaySlot instrn
		// then do a save and restore around the relocated call to
		// save the value of the o7 register from the call to base tramp
 	        if (isInsnType(location->otherInstruction,CALLmask,CALLmatch)) {
		  *temp = location->otherInstruction;
		  fromAddr += sizeof(instruction);
 		  Address offset = fromAddr + (temp->call.disp30 << 2);
 		  if ((offset > (location->func->getAddress(0)+baseAddress)) && 
 		    (offset < ((location->func->getAddress(0)+ baseAddress)+
 				 location->func->size()))) {
		       location -> isLongJump = true;
		       // need to replace retore instr with nop 
		       temp--;
		       generateNOOP(temp);
                       // relocate delaySlot instr
		       temp++;
		       *temp = location->delaySlotInsn;
		       fromAddr += sizeof(instruction);
	               relocateInstruction(temp,fromAddr,currAddr, 
					  (process *)proc);
 		       temp++; 
 		       currAddr += sizeof(instruction);
 		       genImmInsn(temp, SAVEop3, REG_SP, -112, REG_SP); 

		       // relocate the call instruction     
 		       temp++; 
 		       currAddr += sizeof(instruction);
		       fromAddr -= sizeof(instruction);
		       *temp = location->otherInstruction;
	               relocateInstruction(temp,fromAddr,currAddr, 
					   (process *)proc);
 		       temp++; 
 		       fromAddr += sizeof(instruction); 
 		       currAddr += sizeof(instruction);
 		       genImmInsn(temp, RESTOREop3, 0, 0, 0);
                       continue;
                  }
                }

                // otherwise relocate the other instruction
		fromAddr += sizeof(instruction);
		currAddr += sizeof(instruction);
		*++temp = location->otherInstruction;
		relocateInstruction(temp, fromAddr, currAddr, 
				    (process *)proc);
	    }	  
	    
	    // Second, relocate the "NEXT instruction";
	    fromAddr += sizeof(instruction);
	    currAddr += sizeof(instruction);
	    *++temp = location->delaySlotInsn;
 
 	    // if the NEXT instruction is a call instruction to a location
 	    // within the function, then the 07 regester must be saved and 
 	    // resored around the relocated call from the base tramp...the call
 	    // instruction changes the value of 07 to be the PC value, and if
 	    // we move the call instruction to the base tramp, its value will
 	    // be incorrect when we use it in the function.  We generate:
 	    //
 	    //  orignial	    relocated to base tramp
 	    //	--------	    -----------------------
 	    // 	save 		    nop	 // SAVE added above, replace w/nop 
 	    // 	original insn	    original instruction // already relocated
 	    //	delaySlotInsn	    isDelayedInsn
 	    //  isDelayedInsn	    save
 	    //			    delaySlotInsn  (call with offset - 4)
 	    //			    restore
 	    //  In the function, the call to the base tramp will have an
 	    //  additional add instruction to adjust the 07 register
 	    //  orignial	    relocated to base tramp
 	    //	--------	    -----------------------
 	    //  save		     save	
 	    //  mov		     call
 	    //  call 		     nop
 	    //  sethi		     add $o7 4   
 	    //
 	    if (isInsnType(*temp, CALLmask, CALLmatch)) {
 		Address offset = fromAddr + (temp->call.disp30 << 2);
 		if ((offset > (location->func->getAddress(0)+ baseAddress)) && 
 		    (offset < ((location->func->getAddress(0)+ baseAddress)+
 				 location->func->size()))) {
 		    
 		    temp--;
 		    temp--;
 		    generateNOOP(temp);  
 		    temp++;
 		    temp++;
 		    location->isLongJump = true;
 		    // assert(location->hasNoStackFrame() == false);
 		    // assume that this is not a delayed instr.
 		    *temp = location->isDelayedInsn;  
 		    temp++; 
 		    currAddr += sizeof(instruction);
 		    genImmInsn(temp, SAVEop3, REG_SP, -112, REG_SP); 
 		    temp++; 
 		    currAddr += sizeof(instruction);  
 		    *temp = location->delaySlotInsn;
 		    Address new_call_addr = fromAddr - sizeof(instruction);
 		    relocateInstruction(temp,new_call_addr,currAddr,proc);
 		    temp++; 
 		    fromAddr += sizeof(instruction); 
 		    currAddr += sizeof(instruction);
 		    genImmInsn(temp, RESTOREop3, 0, 0, 0);
 		    continue;
 		}
 	    }   

            // otherwise relocate the NEXT instruction
	    relocateInstruction(temp, fromAddr, currAddr,
				(process *)proc);
	    
	    // Third, if the "NEXT instruction" is a DCTI, 
	    if (location->isDelayed) {
		fromAddr += sizeof(instruction);
		currAddr += sizeof(instruction);
		*++temp = location->isDelayedInsn;
		relocateInstruction(temp, fromAddr, currAddr,
				    (process *)proc);
		
		// Then, possibly, there's an callAggregate instruction
		// after this. 
		if (location->callAggregate) {
		    currAddr += sizeof(instruction);
		    *++temp = location->aggregateInsn;
		    continue;
		}	
	    }
	    
	    // If the "FIRST instruction" is a DCTI, then our so called 
	    // "NEXT instruction" is in the delayed Slot and this might
	    // happen. (actullay, it happened)
	    if (location->callAggregate) {
		currAddr += sizeof(instruction);
		*++temp = location->aggregateInsn;
		continue;
	    }	
	    
	    // For the leaf function, if there's an inDelaySlot instruction,
            // move this one to the base Tramp.(i.e. at the function exit,
            // if the first instruction is in the delayed slot the previous
            // instruction, we have to move that one too, so we count from 
            // that one and the last one is this sequence is called inDelaySlot
	    // instruction.)
	    // Well, after all these, another SAVE instruction is generated
	    // so we are prepared to handle the returning to our application's
            // code segment. 
	    if (location->hasNoStackFrame()) {
		if (location->inDelaySlot) {
		    fromAddr += sizeof(instruction);
		    currAddr += sizeof(instruction);
		    *++temp = location->inDelaySlotInsn;
		    relocateInstruction(temp,fromAddr,currAddr,(process *)proc);
                    if(location->firstIsConditional){
		        fromAddr += sizeof(instruction);
		        currAddr += sizeof(instruction);
		        *++temp = location->extraInsn;
		        relocateInstruction(temp, fromAddr, currAddr, proc);
		    }
		} 
		
		genImmInsn(temp+1, SAVEop3, REG_SP, -112, REG_SP);
	    }
	    
	} else if (temp->raw == RETURN_INSN) {
	    // compute the real from address if this instrumentation
	    // point is from a shared object image
	    Address baseAddress = 0;
	    if(proc->getBaseAddress(location->image_ptr,baseAddress)){
            }
	    // Back to the code segement of the application.
            // If the location is in the leaf procedure, generate an RESTORE
	    // instruction right after the CALL instruction to restore all
	    // the values in the registers.
	    if (location -> hasNoStackFrame()) {
		generateCallInsn(temp, currAddr, 
				(baseAddress + location->addr)+location->size);
		genImmInsn(temp+1, RESTOREop3, 0, 0, 0);
	    } else {
		generateCallInsn(temp, currAddr, 
				(baseAddress + location->addr)+location->size);
	    }
        } else if (temp->raw == SKIP_PRE_INSN) {
	    unsigned offset;
	    offset = baseAddr+baseTemplate.updateCostOffset-currAddr;
	    generateBranchInsn(temp,offset);

        } else if (temp->raw == SKIP_POST_INSN) {
	    unsigned offset;
	    offset = baseAddr+baseTemplate.returnInsOffset-currAddr;
	    generateBranchInsn(temp,offset);

        } else if (temp->raw == UPDATE_COST_INSN) {
	    
	    baseTemplate.costAddr = currAddr;
	    generateNOOP(temp);
	} else if ((temp->raw == LOCAL_PRE_BRANCH) ||
                   (temp->raw == GLOBAL_PRE_BRANCH) ||
                   (temp->raw == LOCAL_POST_BRANCH) ||
		   (temp->raw == GLOBAL_POST_BRANCH)) {
#if defined(SHM_SAMPLING) && defined(MT_THREAD)
            if ((temp->raw == LOCAL_PRE_BRANCH) ||
                (temp->raw == LOCAL_POST_BRANCH)) {
                temp -= NUM_INSN_MT_PREAMBLE;
                unsigned numIns=0;
                generateMTpreamble((char *)temp, numIns, proc);
                temp += NUM_INSN_MT_PREAMBLE;
            }
#endif
	    /* fill with no-op */
	    generateNOOP(temp);
	}
    }
    // TODO cast
    proc->writeDataSpace((caddr_t)baseAddr, baseTemplate.size,(caddr_t) code);

    delete [] code;

    trampTemplate *baseInst = new trampTemplate;
    *baseInst = baseTemplate;
    baseInst->baseAddr = baseAddr;
    return baseInst;
}

/*
 * Install the base Tramp for the function relocated.
 * (it means the base tramp that don't need to bother with long jump and
 *  is the one we used before for all the functions(since there's no
 *  long jumps)
 *  for system calls
 */ 
trampTemplate *installBaseTrampSpecial(const instPoint *&location,
				       process *proc,
				       vector<instruction> &extra_instrs) 
{
    unsigned currAddr;
    instruction *code;
    instruction *temp;

    unsigned baseAddr = inferiorMalloc(proc, baseTemplate.size, textHeap);

    if(!(location->func->isInstalled(proc))) {
        location->func->relocateFunction(proc,location,extra_instrs);
    }
    else if(!location->relocated_){
	// need to find new instPoint for location...it has the pre-relocated
	// address of the instPoint
        location->func->modifyInstPoint(location,proc);	     
    }

    code = new instruction[baseTemplate.size];
    memcpy((char *) code, (char*) baseTemplate.trampTemp, baseTemplate.size);

    for (temp = code, currAddr = baseAddr; 
        (currAddr - baseAddr) < (unsigned) baseTemplate.size;
        temp++, currAddr += sizeof(instruction)) {

        if (temp->raw == EMULATE_INSN) {
	    if (location->isBranchOut) {
                // the original instruction is a branch that goes out of a 
		// function.  We don't relocate the original instruction. We 
		// only get to the tramp if the branch is taken, so we generate
		// an unconditional branch to the target of the original 
		// instruction here 
                assert(location->branchTarget);
                int disp = location->branchTarget - currAddr;

                if (in1BranchInsnRange(currAddr,location->branchTarget)) {
		  generateBranchInsn(temp, disp);
		  disp = temp->branch.disp22;
		} else {
		  generateCallInsn(temp, currAddr, disp);
		}
                continue;
            }
	    else {
		*temp = location->originalInstruction;
		Address fromAddress = location->addr;
		relocateInstruction(temp, fromAddress, currAddr, proc);
		if (location->isDelayed) {
		    /* copy delay slot instruction into tramp instance */
		    currAddr += sizeof(instruction);  
		    *++temp = location->delaySlotInsn;
		}
		if (location->callAggregate) {
		    /* copy invalid insn with aggregate size in it */
		    currAddr += sizeof(instruction);  
		    *++temp = location->aggregateInsn;
		}
	    }
        } else if (temp->raw == RETURN_INSN) {
            generateBranchInsn(temp, 
		(location->addr+ sizeof(instruction) - currAddr));
            if (location->isDelayed) {
                /* skip the delay slot instruction */
                temp->branch.disp22 += 1;
            }
            if (location->callAggregate) {
                /* skip the aggregate size slot */
                temp->branch.disp22 += 1;
            }
        } else if (temp->raw == SKIP_PRE_INSN) {
          unsigned offset;
          offset = baseAddr+baseTemplate.updateCostOffset-currAddr;
          generateBranchInsn(temp,offset);
        } else if (temp->raw == SKIP_POST_INSN) {
          unsigned offset;
          offset = baseAddr+baseTemplate.returnInsOffset-currAddr;
          generateBranchInsn(temp,offset);
        } else if (temp->raw == UPDATE_COST_INSN) {
	    
	    baseTemplate.costAddr = currAddr;
	    generateNOOP(temp);
	} else if ((temp->raw == LOCAL_PRE_BRANCH) ||
                   (temp->raw == GLOBAL_PRE_BRANCH) ||
                   (temp->raw == LOCAL_POST_BRANCH) ||
		   (temp->raw == GLOBAL_POST_BRANCH)) {
#if defined(SHM_SAMPLING) && defined(MT_THREAD)
            if ((temp->raw == LOCAL_PRE_BRANCH) ||
                (temp->raw == LOCAL_POST_BRANCH)) {
                temp -= NUM_INSN_MT_PREAMBLE;
                unsigned numIns=0;
                generateMTpreamble((char *)temp, numIns, proc);
		temp += NUM_INSN_MT_PREAMBLE;
            }
#endif
            /* fill with no-op */
            generateNOOP(temp);
        }
    }
    // TODO cast
    proc->writeDataSpace((caddr_t)baseAddr, baseTemplate.size,(caddr_t) code);

    delete [] code;

    trampTemplate *baseInst = new trampTemplate;
    *baseInst = baseTemplate;
    baseInst->baseAddr = baseAddr;
    return baseInst;
}

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
				 bool)
{
    Address adr = location->addr;
    retInstance = NULL;

    
    trampTemplate *ret;
    if (proc->baseMap.find((const instPoint *)location, ret)) // writes to ret if found
       // This base tramp already exists; nothing to do.
       return ret;

    if (location->func->isTrapFunc()) {
       // get the base Address of this function if it is a 
       // shared object
       Address baseAddress = 0;
       if(!proc->getBaseAddress(location->image_ptr,baseAddress)){
	  // TODO: what should be done here? 	
	  logLine("Error:findAndInstallBaseTramp call getBaseAddress\n"); 
       }
       // Install Base Tramp for the functions which are 
       // relocated to the heap.
       vector<instruction> extra_instrs;

       ret = installBaseTrampSpecial(location, proc,extra_instrs);

       // add a branch from relocated function to the base tramp
       // if function was just relocated then location has old address
       // otherwise location will have address in already relocated func
       if (!location->func->isInstalled(proc)){
	  if (location->isBranchOut){
	     changeBranch(proc, location->addr, 
		          (int) ret->baseAddr, location->originalInstruction);
	   } else {
	     generateBranch(proc, location->addr, (int)ret->baseAddr);
	   }
       }
       else {  // location's address is correct...it is in the heap
	  if (location->isBranchOut){
	     changeBranch(proc, location->addr, 
		          (int) ret->baseAddr, location->originalInstruction);
	  } else {
	     generateBranch(proc, location->addr, (int)ret->baseAddr);
	  }
       }

       // If for this process, a call to the relocated function has not
       // yet be installed in its original location, then genterate the
       // following instructions at the begining of the function:
       //   SAVE;             CALL;         RESTORE.
       // so that it would jump the start of the relocated function
       // which is in heap.
       if (!location->func->isInstalled(proc)){
	  location->func->setInstalled(proc);
	  u_int e_size = extra_instrs.size();
	  instruction *insn = new instruction[3 + e_size];
	  Address adr = location-> func -> getAddress(0);
	  genImmInsn(insn, SAVEop3, REG_SP, -112, REG_SP);
	  generateCallInsn(insn+1, adr+baseAddress+4, 
			   location->func->getAddress(proc));
	  genSimpleInsn(insn+2, RESTOREop3, 0, 0, 0); 
	  for(u_int i=0; i < e_size; i++){
	     insn[3+i] = extra_instrs[i];
	  }
	  retInstance = new returnInstance((instructUnion *)insn, 
					   (3+e_size)*sizeof(instruction), 
					   adr+baseAddress, 
					   location->func->size());
	  assert(retInstance);

	  //cerr << "created a new return instance (relocated fn)!" << endl;
       }
    } else {
       // It's not a trap-function; it's a "normal" function
       // compute the real from address if this instrumentation
       // point is from a shared object image
       Address baseAddress = 0;
       if (proc->getBaseAddress(location->image_ptr,baseAddress)){
	  adr += baseAddress;		
       }

       ret = installBaseTramp(location, proc);
       // check to see if this is an entry point and if the delay 
       // slot instruction is a call insn, if so, then if the 
       // call is to a location within the function, then we need to 
       // add an extra instruction after the restore to correctly
       // set the o7 register
       bool need_to_add = false;
       if (location->ipType==functionEntry &&
	   isInsnType(location->delaySlotInsn,CALLmask,CALLmatch)) {
	  Address call_offset = location->addr + 8 + 
	                        (location->delaySlotInsn.call.disp30<<2);
	  Address fun_addr = location->func->getAddress(0);
	  u_int fun_size = location->func->size();
	  if (call_offset>fun_addr && call_offset<(fun_addr+fun_size)) {
	     assert(location->isLongJump);
	     need_to_add = true;
	  }
       }	

       if (location->hasNoStackFrame()) {
	  // if it is the leaf function, we need to generate
	  // the following instruction sequence:
	  //     SAVE;      CALL;      NOP.

	  if (location -> isLongJump == false) {
	     instruction *insn = new instruction;
	     generateBranchInsn(insn, (int)(ret->baseAddr-adr));
	     retInstance = new returnInstance((instructUnion *)insn,
					      sizeof(instruction), adr, 
					      sizeof(instruction));
	  } else if (need_to_add) {
	     // generate  original; call; add $o7 imm4 
	     instruction *insn = new instruction[2];
	     generateCallInsn(insn, adr+4, (int) ret->baseAddr);
	     genImmInsn(insn+1,ADDop3,REG_O7,4,REG_O7);
	     retInstance = new returnInstance((instructUnion *)insn,
				 2*sizeof(instruction), adr+4,
			         2*sizeof(instruction));
	  } else {
	    bool already_done = false; 
	    // check to see if the otherInstruction is a call instruction
	    // to itself, if so then generate the following
	    // before 			after 		basetramp
	    // ------			-----		---------
	    // mov     originalInsn	mov		sethi
	    // call    otherInsn	call 		save
	    // sethi   delaySlot	nop		call
	    //						restore
	    // only generate a call and nop...leave the originalInsn
	    //
 	    if (isInsnType(location->otherInstruction, CALLmask, CALLmatch)) {
 	      Address offset = location-> func -> getAddress(0)+4 + 
			       (location->otherInstruction.call.disp30 << 2);
 	      if ((offset > (location->func->getAddress(0))) && 
 	          (offset < ((location->func->getAddress(0))+
 			 location->func->size()))) {
	             instruction *insn = new instruction[2];
	             generateCallInsn(insn, adr+4, (int) ret->baseAddr);
	             generateNOOP(insn+1);
	             retInstance = new returnInstance((instructUnion *)insn, 
					     2*sizeof(instruction), adr+4, 
					     2*sizeof(instruction));

                     already_done = true;
                 }
             }

             if(!already_done) {
	         instruction *insn = new instruction[3];
	         genImmInsn(insn, SAVEop3, REG_SP, -112, REG_SP);
	         generateCallInsn(insn+1, adr+4, (int) ret->baseAddr);
	         generateNOOP(insn+2);
	         retInstance = new returnInstance((instructUnion *)insn, 
					     3*sizeof(instruction), adr, 
					     3*sizeof(instruction));
	     }	
	  }
		
	  assert(retInstance);
       } else {
	  // It's not a leaf.
	  // Generate branch instruction from the application to the
	  // base trampoline and no SAVE instruction is needed
		
	  if (in1BranchInsnRange(adr, ret->baseAddr)) {
	    // make sure that the isLongJump won't be true
	    // which only is possible for shlib entry point 
	    //assert(location->isLongJump == false);
	    if (location->isLongJump) {
	      instruction *insn = new instruction[2];	
	      generateCallInsn(insn, adr, (int) ret->baseAddr);
	      assert(location->ipType == functionEntry);
	      generateNOOP(insn+1);
	      retInstance = new returnInstance((instructUnion *)insn, 
					      2*sizeof(instruction), adr, 
					      2*sizeof(instruction));
	      assert(retInstance);
	    } else {
	      instruction *insn = new instruction;
	      if (location -> ipType == functionEntry) {
	          generateBranchInsn(insn, (int)(ret->baseAddr-adr+sizeof(instruction))); 
		  retInstance = new returnInstance((instructUnion *)insn,
						   sizeof(instruction), 
						   adr - sizeof(instruction), 
						   sizeof(instruction));
	      } else {
	          generateBranchInsn(insn,(int)(ret->baseAddr-adr));
		  retInstance = new returnInstance((instructUnion *)insn,
						   sizeof(instruction), 
						   adr, 
						   sizeof(instruction));
	      }
	    }
	  } else if(need_to_add) {
	     // the delay slot instruction is is a call to a location
	     // within the same function, then need to generate 3 instrs
	     //    call
	     //    nop          // delay slot (originally call insn)
	     //    add o7 imm4  // sets o7 register to correct value
	     instruction *insn = new instruction[3];	
	     generateCallInsn(insn, adr, (int) ret->baseAddr);
	     generateNOOP(insn+1);
	     genImmInsn(insn+2,ADDop3,REG_O7,4,REG_O7);
	     retInstance = new returnInstance((instructUnion *)insn, 
					      3*sizeof(instruction), adr, 
					      3*sizeof(instruction));
	  } else {
	     instruction *insn = new instruction[2];	
	     generateCallInsn(insn, adr, (int) ret->baseAddr);
	     if (location -> ipType == functionEntry) {
	        if (location -> isLongJump)
		   generateNOOP(insn+1);
		else
		   genSimpleInsn(insn+1, RESTOREop3, 0, 0, 0);
	     } else
	        generateNOOP(insn+1);

	     retInstance = new returnInstance((instructUnion *)insn, 
					      2*sizeof(instruction), adr, 
					      2*sizeof(instruction));
	     assert(retInstance);
	  }
       }
    }

    proc->baseMap[(const instPoint *)location] = ret;
	
    return(ret);
       // remember, ret was the result of either installBaseTramp() or
       // installBaseTrampSpecial()
}

/*
 * Install a single tramp.
 *
 */
void installTramp(instInstance *inst, char *code, int codeSize) 
{
    totalMiniTramps++;
    insnGenerated += codeSize/sizeof(int);
    
    // TODO cast
    (inst->proc)->writeDataSpace((caddr_t)inst->trampBase, codeSize, code);

    unsigned atAddr;
    if (inst->when == callPreInsn) {
	if (inst->baseInstance->prevInstru == false) {
	    atAddr = inst->baseInstance->baseAddr+baseTemplate.skipPreInsOffset;
	    inst->baseInstance->cost += inst->baseInstance->prevBaseCost;
	    inst->baseInstance->prevInstru = true;
	    generateNoOp(inst->proc, atAddr);
	}
    } else {
	if (inst->baseInstance->postInstru == false) {
	    atAddr = inst->baseInstance->baseAddr+baseTemplate.skipPostInsOffset; 
	    inst->baseInstance->cost += inst->baseInstance->postBaseCost;
	    inst->baseInstance->postInstru = true;
	    generateNoOp(inst->proc, atAddr);
	}
    }
}


unsigned emitFuncCall(opCode op, 
		      registerSpace *rs,
		      char *i, unsigned &base, 
		      const vector<AstNode *> &operands, 
		      const string &callee, process *proc,
		      bool noCost)
{
        assert(op == callOp);
        unsigned addr;
	bool err;
	vector <reg> srcs;
	void cleanUpAndExit(int status);

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
	    addr = func->getAddress(0);
	}
	
	for (unsigned u = 0; u < operands.size(); u++)
	    srcs += operands[u]->generateCode(proc, rs, i, base, noCost, false);

	// TODO cast
	instruction *insn = (instruction *) ((void*)&i[base]);

        for (unsigned u=0; u<srcs.size(); u++){
            if (u >= 5) {
	         string msg = "Too many arguments to function call in instrumentation code: only 5 arguments can be passed on the sparc architecture.\n";
		 fprintf(stderr, msg.string_of());
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

        // return value is the register with the return value from the
        //   function.
        // This needs to be %o0 since it is back in the callers scope.
        return(8);
}
 
unsigned emit(opCode op, reg src1, reg src2, reg dest, char *i, unsigned &base,
	      bool noCost)
{
    // TODO cast
    instruction *insn = (instruction *) ((void*)&i[base]);

    if (op == loadConstOp) {
      // dest = src1:imm    TODO
      if (src1 > MAX_IMM13 || src1 < MIN_IMM13) {
            // src1 is out of range of imm13, so we need an extra instruction
	    generateSetHi(insn, src1, dest);
	    base += sizeof(instruction);
	    insn++;

	    // or regd,imm,regd

            // Chance for optimization: we should check for LOW10(src1)==0, and
            // if so, don't generate the following bitwise-or instruction, since
            // in that case nothing would be done.

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
    } else if (op ==  loadIndirOp) {
	generateLoad(insn, src1, 0, dest);
	base += sizeof(instruction);
    } else if (op ==  storeOp) {
	insn->sethi.op = FMT2op;
	insn->sethi.rd = src2;
	insn->sethi.op2 = SETHIop2;
	insn->sethi.imm22 = HIGH22(dest);
	insn++;

	generateStore(insn, src1, src2, LOW10(dest));

	base += sizeof(instruction)*2;
    } else if (op ==  storeIndirOp) {
	generateStore(insn, src1, dest, 0);
	base += sizeof(instruction);
    } else if (op ==  ifOp) {
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
    } else if (op == branchOp) {
	// Unconditional branch
	generateBranchInsn(insn, dest); insn++;

	generateNOOP(insn);
	base += sizeof(instruction)*2;
	return(base - 2*sizeof(instruction));
    } else if (op ==  updateCostOp) {
        // generate code to update the observed cost.
	if (!noCost) {
	   // sethi %hi(dest), %l0
	   generateSetHi(insn, dest, REG_L0);
	   base += sizeof(instruction);
	   insn++;
  
	   // ld [%l0+ lo(dest)], %l1
	   generateLoad(insn, REG_L0, LOW10(dest), REG_L1);
	   base += sizeof(instruction);
	   insn++;
  
	   // update value (src1 holds the cost, in cycles; e.g. 19)
	   if (src1 <= MAX_IMM13) {
	      genImmInsn(insn, ADDop3, REG_L1, src1, REG_L1);
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
	      generateSetHi(insn, src1, REG_L2);
	      base += sizeof(instruction);
	      insn++;

	      // or regd,imm,regd
	      genImmInsn(insn, ORop3, REG_L2, LOW10(src1), REG_L2);
	      base += sizeof(instruction);
	      insn++;

	      // now add it
	      genSimpleInsn(insn, ADDop3, REG_L1, REG_L2, REG_L1);
	      base += sizeof(instruction);
	      insn++;
	   }
  
	   // store result st %l1, [%l0+ lo(dest)];
	   generateStore(insn, REG_L1, REG_L0, LOW10(dest));
	   base += sizeof(instruction);
	   insn++;
	} // if (!noCost)
    } else if (op ==  trampPreamble) {
#ifdef ndef
        // save and restore are done inthe base tramp now
        genImmInsn(insn, SAVEop3, REG_SP, -112, REG_SP);
	base += sizeof(instruction);
        insn++;

	// generate code to save global registers
	for (unsigned u = 0; u < 4; u++) {
	  genStoreD(insn, 2*u, REG_FP, - (8 + 8*u));
	  base += sizeof(instruction);
	  insn++;
	}
#endif
    } else if (op ==  trampTrailer) {
#ifdef ndef
        // save and restore are done inthe base tramp now
	// generate code to restore global registers
	for (unsigned u = 0; u < 4; u++) {
	  genLoadD(insn, REG_FP, - (8 + 8*u), 2*u);
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
    } else if (op == noOp) {
	generateNOOP(insn);
	base += sizeof(instruction);
    } else if (op == getParamOp) {
#if defined(SHM_SAMPLING) && defined(MT_THREAD)
        // saving CT/vector address on the stack
        generateStore(insn, REG_MT, REG_FP, -40);
        insn++;
#endif
	// first 8 parameters are in register 24 ....
	genSimpleInsn(insn, RESTOREop3, 0, 0, 0);
	insn++;

	generateStore(insn, 24+src1, REG_SP, 68+4*src1); 
	insn++;
	      
	genImmInsn(insn, SAVEop3, REG_SP, -112, REG_SP);
	insn++;

	generateLoad(insn, REG_SP, 112+68+4*src1, 24+src1); 
	insn++;

#if defined(SHM_SAMPLING) && defined(MT_THREAD)
        // restoring CT/vector address back in REG_MT
        generateLoad(insn, REG_FP, -40, REG_MT);
        insn++;
        base += 6*sizeof(instruction);
#else
	base += 4*sizeof(instruction);
#endif
	
	if (src1 <= 8) {
	    return(24+src1);
	}
	
	abort();
    } else if (op == getSysParamOp) {
	
	if (src1 <= 8) {
	    return(24+src1);
	}	
    } else if (op == getRetValOp) {
	// return value is in register 24
	genSimpleInsn(insn, RESTOREop3, 0, 0, 0);
	insn++;

	generateStore(insn, 24, REG_SP, 68); 
	insn++;
	      
	genImmInsn(insn, SAVEop3, REG_SP, -112, REG_SP);
	insn++;

	generateLoad(insn, REG_SP, 112+68, 24); 
	insn++;

	base += 4*sizeof(instruction);

	return(24);

    } else if (op == getSysRetValOp) {

	return(24);
    } else if (op == saveRegOp) {
	// should never be called for this platform.
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
                genImmInsn(insn, WRYop3, REG_G0, 0, 0);
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
		return(0);
		break;

            case neOp:
                genRelOp(insn, BEcond, src1, src2, dest, base);
                return(0);
                break;

	    case lessOp:
                genRelOp(insn, BGEcond, src1, src2, dest, base);
                return(0);
                break;

            case leOp:
                genRelOp(insn, BGTcond, src1, src2, dest, base);
                return(0);
                break;

            case greaterOp:
                genRelOp(insn, BLEcond, src1, src2, dest, base);
                return(0);
                break;

            case geOp:
                genRelOp(insn, BLTcond, src1, src2, dest, base);
                return(0);
                break;

	    default:
		abort();
		break;
	}
	genSimpleInsn(insn, op3, src1, src2, dest);

	base += sizeof(instruction);
      }
    return(0);
}

static inline bool CallRestoreTC(instruction instr, instruction nexti) {
    return (isCallInsn(instr) && 
           (nexti.rest.op == 2
	       && ((nexti.rest.op3 == ORop3 && nexti.rest.rd == 15)
		       || nexti.rest.op3 == RESTOREop3)));
}

/*
    Return integer value indicating whether instruction sequence
     found signals tail call 
     jmp 
     nop 
    sequence.  Note that this should NOT include jmpl nop, ret nop, retl
     nop....
    Current heuristic to detect such sequences :
     look for jmp %reg, nop in function w/ no stack frame, if jmp, nop
     are last 2 instructions, return 1 (definate TC), at any other point,
     return 0 (not TC).  Otherwise, return 0 (no TC).
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
    //  NOT %i7 + 8 or %o7 + 8 (ret and retl synthetic instructions, respectively)
    if (instr.resti.i == 1) {
        if (instr.resti.rs1 == REG_I(7) || instr.resti.rs1 == REG_O(7)) {
	    // NOTE : some return and retl instructions jump to {io}7 + 12,
	    //  not + 8, to have some extra space to store the size of a 
	    //  return structure....
            if (instr.resti.simm13 == 0x8 || instr.resti.simm13 == 12) {
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
    

/*
 * Find the instPoints of this function.
 */
bool pd_Function::findInstPoints(const image *owner) {
    bool call_restore_tc;
    int jmp_nop_tc;
    instPoint *blah = 0;
    bool err;
    int dummyId;

    //cerr << "pd_Function::findInstPoints called " << *this;
   if (size() == 0) {
     //cerr << " size = 0, returning FALSE" << endl;
     return false;
   }

   noStackFrame = true;

   Address adr;
   Address adr1 = getAddress(0);
   instruction instr, nexti;
   instr.raw = owner->get_instruction(adr1);
   if (!IS_VALID_INSN(instr)) {
     //cerr << " IS_VALID_ISIN(adr1) == 0, returning FALSE" << endl;
     return false;
   }

   // If it contains an instruction, I assume it would be s system call
   // which will be treat differently. 
   isTrap = false;
   bool func_entry_found = false;

   for ( ; adr1 < getAddress(0) + size(); adr1 += 4) {
       instr.raw = owner->get_instruction(adr1);
       nexti.raw = owner->get_instruction(adr1+4);

       // If there's an TRAP instruction in the function, we 
       // assume that it is an system call and will relocate it 
       // to the heap
       if (isInsnType(instr, TRAPmask, TRAPmatch)) {
	   isTrap = true;
	   //cerr << " TRAP instrcution detected, returning findInstPoints" << endl;
	   return findInstPoints(owner, getAddress(0), 0);
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
       call_restore_tc = CallRestoreTC(instr, nexti);
       jmp_nop_tc = JmpNopTC(instr, nexti, adr1, this);
       if (call_restore_tc || jmp_nop_tc) {
           isTrap = true;
	   return findInstPoints(owner, getAddress(0), 0);
       }
       

       // The function Entry is defined as the first SAVE instruction plus
       // the instructions after this.
       // ( The first instruction for the nonleaf function is not 
       //   necessarily a SAVE instruction. ) 
       if (isInsnType(instr, SAVEmask, SAVEmatch) && !func_entry_found) {
	   //cerr << " save instruction found" << endl;
	   noStackFrame = false;

	   func_entry_found = true;
	   funcEntry_ = new instPoint(this, instr, owner, adr1, true,
				      functionEntry);
	   adr = adr1;
	   assert(funcEntry_);
       }
  }

   // If there's no SAVE instruction found, this is a leaf function and
   // and function Entry will be defined from the first instruction
   if (noStackFrame) {
       //cerr << " noStackFrame, apparently leaf function" << endl;
       adr = getAddress(0);
       instr.raw = owner->get_instruction(adr);
       funcEntry_ = new instPoint(this, instr, owner, adr, true,
				  functionEntry);
       assert(funcEntry_);
   }

   for ( ; adr < getAddress(0) + size(); adr += sizeof(instruction)) {

     instr.raw = owner->get_instruction(adr);

     bool done;

     // check for return insn and as a side affect decide if we are at the
     //   end of the function.
     if (isReturnInsn(owner, adr, done)) {
       // define the return point
       funcReturns += new instPoint(this, instr, owner, adr, false,
				    functionExit);

     } else if (instr.branch.op == 0 
		&& (instr.branch.op2 == 2 || instr.branch.op2 == 6) 
		&& (instr.branch.cond == 0 ||instr.branch.cond == 8)) {
       // find if this branch is going out of the function
       int disp = instr.branch.disp22;
       Address target = adr +  (disp << 2);
       if ((target < (getAddress(0)))  
	   || (target >= (getAddress(0) + size()))) {
	 instPoint *point = new instPoint(this, instr, owner, adr, false,
					  functionExit);
	 funcReturns += point;
       }

     } else if (isCallInsn(instr)) {

       // if the call target is the address of the call instruction
       // then this is not something that we can instrument...
       // this occurs in functions with code that is modifined when 
       // they are loaded by the run-time linker, or when the .init
       // section is executed.  In this case the instructions in the
       // parsed image file are different from the ones in the executable
       // process.
       if(instr.call.op == CALLop) { 
           Address call_target = adr + (instr.call.disp30 << 2);
           if(call_target == adr){ 
	        cerr << "WARN : function " << prettyName().string_of()
		  << " has call to same location as call, NOT instrumenting"
		  << endl;
	        return false;
	   }
       }
       // first, check for tail-call optimization: a call where the instruction 
       // in the delay slot write to register %o7(15), usually just moving
       // the caller's return address, or doing a restore
       // Tail calls are instrumented as return points, not call points.


       instruction nexti; 
       nexti.raw = owner->get_instruction(adr+4);

       if (CallRestoreTC(instr, nexti)) {
	 // Alert!!!!
	 adr = newCallPoint(adr, instr, owner, err, dummyId, adr,0,blah);
	 funcReturns += new instPoint(this, instr, owner, adr, false,
				      functionExit);

       } else {
	 // define a call point
	 // this may update address - sparc - aggregate return value
	 // want to skip instructions
	 adr = newCallPoint(adr, instr, owner, err, dummyId, adr,0,blah);
       }
     }
     else if (JmpNopTC(instr, nexti, adr, this)) {
         // Alert!!!! 
         adr = newCallPoint(adr, instr, owner, err, dummyId, adr,0,blah);

	 funcReturns += new instPoint(this, instr, owner, adr, false,
				      functionExit);
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

       reg jumpreg = instr.rest.rs1;
       instruction prev1;
       instruction prev2;

       prev1.raw = owner->get_instruction(adr-4);
       prev2.raw = owner->get_instruction(adr-8);

       unsigned targetAddr;

       if (instr.rest.rd == 0 && (instr.rest.i == 1 || instr.rest.rs2 == 0)
	   && prev2.sethi.op == FMT2op && prev2.sethi.op2 == SETHIop2 
	   && prev2.sethi.rd == (unsigned)jumpreg
	   && prev1.rest.op == RESTop 
	   && prev1.rest.rd == (unsigned)jumpreg && prev1.rest.i == 1
	   && prev1.rest.op3 == ORop3 && prev1.rest.rs1 == (unsigned)jumpreg) {

	 targetAddr = (prev2.sethi.imm22 << 10) & 0xfffffc00;
	 targetAddr |= prev1.resti.simm13;
	 if ((targetAddr<getAddress(0))||(targetAddr>=(getAddress(0)+size()))){
	   instPoint *point = new instPoint(this, instr, owner, adr, false, 
					    functionExit);
	   funcReturns += point;
	 }
       }

     }
 }
   
 return (checkInstPoints(owner)); 
}

/*
 * Check all the instPoints within this function to see if there's 
 * any conficts happen.
 */
bool pd_Function::checkInstPoints(const image *owner) {

    // Our own library function, skip the test.
    if (prettyName().prefixed_by("DYNINST")) 
	return true;

#ifndef BPATCH_LIBRARY /* XXX Users of libdyninstAPI might not agree. */
    // The function is too small to be worthing instrumenting.
    if (size() <= 12){
        //cerr << "WARN : function " << prettyName().string_of()
        //	     << " too small (size <= 12), can't instrument" << endl;
	return false;
    }
#endif

    // No function return! return false;
    if (sizeof(funcReturns) == 0) {
        //cerr << "WARN : function " << prettyName().string_of()
        //	     << " no return point found, can't instrument" << endl;
	return false;
    }

    instruction instr;
    Address adr = getAddress(0);

    bool retl_inst = false;
    // Check if there's any branch instruction jump to the middle
    // of the instruction sequence in the function entry point
    // and function exit point.
    for ( ; adr < getAddress(0) + size(); adr += sizeof(instruction)) {

	instr.raw = owner->get_instruction(adr);
	if(isInsnType(instr, RETLmask, RETLmatch)) retl_inst = true;

	if (isInsnType(instr, BRNCHmask, BRNCHmatch)||
	    isInsnType(instr, FBRNCHmask, FBRNCHmatch)) {

	    int disp = instr.branch.disp22;
	    Address target = adr + (disp << 2);

	    if ((target > funcEntry_->addr)&&
		(target < (funcEntry_->addr + funcEntry_->size))) {
		if (adr > (funcEntry_->addr+funcEntry_->size)){
		    //cerr << "WARN : function " << prettyName().string_of()
		    //	 << " has branch target inside fn entry point, can't instrument" << endl;
		    return false;
	    } }

	    for (u_int i = 0; i < funcReturns.size(); i++) {
		if ((target > funcReturns[i]->addr)&&
		    (target < (funcReturns[i]->addr + funcReturns[i]->size))) {
		    if ((adr < funcReturns[i]->addr)||
			(adr > (funcReturns[i]->addr + funcReturns[i]->size))){
		        //cerr << "WARN : function " << prettyName().string_of()
		        //  << " has branch target inside fn return point, "
		        //  << "can't instrument" << endl;
		        return false;
		} }
	    }
	}
    }

    // if there is a retl instruction and we don't think this is a leaf
    // function then this is a way messed up function...well, at least we
    // we can't deal with this...the only example I can find is _cerror
    // and _cerror64 in libc.so.1
    if(retl_inst && !noStackFrame){ 
        //cerr << "WARN : function " << prettyName().string_of()
        //     << " retl instruction in non-leaf function, can't instrument"
        //      << endl;
	return false;
    }

    // check that no instrumentation points could overlap
    Address func_entry = funcEntry_->addr + funcEntry_->size; 
    for (u_int i = 0; i < funcReturns.size(); i++) {
	if(func_entry >= funcReturns[i]->addr){
	   return false;
        }
	if(i >= 1){ // check if return points overlap
	    Address prev_exit = funcReturns[i-1]->addr+funcReturns[i-1]->size;  
	    if(funcReturns[i]->addr < prev_exit) {
	        //cerr << "WARN : function " << prettyName().string_of()
	        //     << " overlapping instrumentation points, can't instrument"
	        //     << endl;
		return false;
	    } 
	}
    }

    return true;	
}


/* The maximum length of relocatable function is 1k instructions */  
// This function is to find the inst Points for a function
// that will be relocated if it is instrumented. 
bool pd_Function::findInstPoints(const image *owner, Address newAdr, process*){
   int i, jmp_nop_tc;
   instruction second_instr;
   instruction nexti; 

   bool err;
   instPoint *blah = 0;

   if (size() == 0) {
     return false;
   }
   relocatable_ = true;

   Address adr = getAddress(0);
   instruction instr;
   instr.raw = owner->get_instruction(adr);
   if (!IS_VALID_INSN(instr))
     return false;
   
   if (size() <= 3*sizeof(instruction)) 
       return false;

   instPoint *point = new instPoint(this, instr, owner, newAdr, true, 
				    functionEntry, adr);

   funcEntry_ = point;

   // if the second instruction in a relocated function is a call instruction
   // or a branch instruction, then we can't deal with this.
   // New: only problem if call is to location outside of function, or
   //  a jump to itself....
   if(size() > sizeof(instruction)){
       Address second_adr = adr + sizeof(instruction);
       second_instr.raw =  owner->get_instruction(second_adr); 

       if (isCallInsn(second_instr)) {
           Address call_target = second_adr + (second_instr.call.disp30 << 2);
	   // if call dest. is outside of function, assume real
	   //  call site.  Assuming cant deal with this case!!!!
           if (!(call_target >= adr && call_target <= adr + size()) ||
	       (call_target == second_adr)) {
	       return false;
	   }
       }

       if (second_instr.branch.op == 0 && 
   		      (second_instr.branch.op2 == 2 || 
   		      second_instr.branch.op2 == 6)) {
   	   return false;
       }
   }    

   assert(funcEntry_);
   int retId = 0;
   int callsId = 0; 

   for (i = 0; adr < getAddress(0) + size(); adr += sizeof(instruction),  
	newAdr += sizeof(instruction), i++) {

     instr.raw = owner->get_instruction(adr);
     newInstr[i] = instr;
     nexti.raw = owner->get_instruction(adr+4);

     bool done;

     // check for return insn and as a side affect decide if we are at the
     //   end of the function.
     if (isReturnInsn(owner, adr, done)) {
       // define the return point
       instPoint *point	= new instPoint(this, instr, owner, newAdr, false, 
					functionExit, adr);
       funcReturns += point;
       funcReturns[retId] -> instId = retId++;
     } else if (instr.branch.op == 0 
		&& (instr.branch.op2 == 2 || instr.branch.op2 == 6)) {
       // find if this branch is going out of the function
       int disp = instr.branch.disp22;
       Address target = adr + (disp << 2);
       if (target < getAddress(0) || target >= getAddress(0) + size()) {
	   instPoint *point = new instPoint(this, newInstr[i], owner, 
					    newAdr, false, 
					    functionExit, adr);
	   if ((instr.branch.cond != 0) && (instr.branch.cond != 8)) {  
	       point->isBranchOut = true;
	       point->branchTarget = target;
	   }
	   funcReturns += point;
	   funcReturns[retId] -> instId = retId++;
       }

     } else if (isCallInsn(instr)) {

       // SNIP - incorrect comment removed)
       //  check for 2 types of tail-call optimization.  The first
       //  (seen on Solaris 2.4, 2.5 and 2.6) is a CALL, RESTORE
       //  anywhere inside function.  The second (seen to date on
       //  Solaris 2.6 only) is a JMP, NOP in the last 2 instructions 
       //  of a function w/o a stack frame.
       //  In both cases, the sequence is marked here as a function
       //   return point (not call site).  This is possibly meaningless
       //   however, as the first time the function is called, 
       //   it is relocated, and (theoretically) the tail-call sequence
       //   is opened up and seperate call site and exit point
       //   should be falgged.
       if (CallRestoreTC(instr, nexti)) {
	   // Alert!!!!
	   adr = newCallPoint(newAdr, instr, owner, err, callsId, adr,0,blah);
	   if (err) return false;

           instPoint *point = new instPoint(this, instr, owner, newAdr, false,
				      functionExit, adr);
           funcReturns += point;
           funcReturns[retId] -> instId = retId++;

       } else {
         // if this is a call instr to a location within the function, and if 
         // the offest is not 8 then do not define this function 
         if(instr.call.op == CALLop){ 
           Address call_target = adr + (instr.call.disp30 << 2);
           if((call_target >= getAddress(0)) 
	      && (call_target <= (getAddress(0)+size()))){ 
	      if((instr.call.disp30 << 2) != 2*sizeof(instruction)) {
	        return false;
	      }
           }
	 }
	 // define a call point
	 // this may update address - sparc - aggregate return value
	 // want to skip instructions
	 adr = newCallPoint(newAdr, instr, owner, err, callsId, adr,0,blah);
	 if (err)
	   return false;
       }
     }
     else if ((jmp_nop_tc = JmpNopTC(instr, nexti, adr, this)) == 1) {
           // Alert!!!!
           adr = newCallPoint(newAdr, instr, owner, err, callsId, adr,0,blah);
	   if (err) return false;

           instPoint *point = new instPoint(this, instr, owner, newAdr, false,
				      functionExit, adr);
           funcReturns += point;
           funcReturns[retId] -> instId = retId++;
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

	 reg jumpreg = instr.rest.rs1;
	 instruction prev1;
	 instruction prev2;
	 
	 prev1.raw = owner->get_instruction(adr-4);
	 prev2.raw = owner->get_instruction(adr-8);

	 unsigned targetAddr;

	 if (instr.rest.rd == 0 && (instr.rest.i == 1 || instr.rest.rs2 == 0)
	     && prev2.sethi.op == FMT2op && prev2.sethi.op2 == SETHIop2 
	     && prev2.sethi.rd == (unsigned)jumpreg
	     && prev1.rest.op == RESTop 
	     && prev1.rest.rd == (unsigned)jumpreg && prev1.rest.i == 1
	     && prev1.rest.op3 == ORop3 && prev1.rest.rs1 == (unsigned)jumpreg){
	     
	     targetAddr = (prev2.sethi.imm22 << 10) & 0xfffffc00;
	     targetAddr |= prev1.resti.simm13;
	     if ((targetAddr < getAddress(0)) 
		 || (targetAddr >= (getAddress(0)+size()))) {
		 instPoint *point = new instPoint(this, instr, owner, 
						  newAdr, false,
						  functionExit, adr);
		 funcReturns += point;
		 funcReturns[retId] -> instId = retId++;
	     }
	 }
     }
 }
 return true;
}

// This function assigns new address to instrumentation points of  
// a function that has been relocated
bool pd_Function::findNewInstPoints(const image *owner, 
				const instPoint *&location,
				Address newAdr,
				process *proc,
				vector<instruction> &callInstrs,
				relocatedFuncInfo *reloc_info, 
				FunctionExpansionRecord *fer, 
				int size_change) {

   int i, extra_offset, disp;
   instruction nexti; 
   bool call_restore_tc, jmp_nop_tc, err;

   if (size() == 0) {
       cerr << "WARN : attempting to relocate function "
       	    << prettyName().string_of() << " with size 0, unable to instrument"
            <<  endl;
       return false;
   }
   assert(reloc_info);

   // Note : newInstr defined as static array 1024 long in inst-sparc.h
   //  for some reason. (why static????)????
   if (size() + size_change > NEW_INSTR_ARRAY_LEN) {
       cerr << "WARN : attempting to relocate function "
       	    << prettyName().string_of() << " with size "
	    << " > NEW_INSTR_ARRAY_LEN, unable to instrument";
       cerr << " size = " << size() << " , size_change = "
	    << size_change << endl;
       return FALSE;
   }


   Address adr = getAddress(0);
   Address start_adr = adr;
   instruction instr;
   instr.raw = owner->get_instruction(adr);
   if (!IS_VALID_INSN(instr)) {
       cerr << "WARN : attempting to relocate function "
	    << prettyName().string_of() << " first instruction could not "
	    << " be correctly parsed, unable to instrument" << endl;
       return false;
   }

   instPoint *point = new instPoint(this, instr, owner, newAdr, true, 
				    functionEntry, adr);
   point->relocated_ = true;
   // if location was the entry point then change location's value to new pt
   if(location == funcEntry_) { 
	location = point;
   }

   reloc_info->addFuncEntry(point);
   assert(reloc_info->funcEntry());
   int retId = 0;
   int callsId = 0; 

   // get baseAddress if this is a shared object
   Address baseAddress = 0;
   if(!(proc->getBaseAddress(owner,baseAddress))){
	baseAddress =0;
   }

   // if we have call instructions that need to be added after the instrs
   // to call the relocated instruction, the first address we can use is
   // the address of the 4th instruction in the function
   Address call_start_addr = getAddress(0)+baseAddress + 3*sizeof(instruction);

   for (i = 0; adr < getAddress(0) + size(); adr += 4,  newAdr += 4, i++) {
    
     instr.raw = owner->get_instruction(adr);
     newInstr[i] = instr;
     nexti.raw = owner->get_instruction(adr+4);

     bool done;

     // check for return insn and as a side affect decide if we are at the
     //   end of the function.
     if (isReturnInsn(owner, adr, done)) {
       // define the return point
       instPoint *point	= new instPoint(this, instr, owner, newAdr, false, 
					functionExit, adr);
       point->relocated_ = true;
       // if location was this point, change it to new point
       if(location == funcReturns[retId]) { 
	   location = point;
       }
       retId++;
       reloc_info->addFuncReturn(point);
     } else if (instr.branch.op == 0 
		&& (instr.branch.op2 == 2 || instr.branch.op2 == 6)) {
       // --- If the instruction being relocated is a branch instruction ---

       // find if this branch is going out of the function
       disp = instr.branch.disp22;
       Address target = adr + baseAddress + (disp << 2);

       // getAddress(0) gives the addr of the fn before it's relocated
       if ((target < (getAddress(0) + baseAddress)) 
	   || (target >= (getAddress(0) + baseAddress + size()))) {
	   // If the branch was to a target outside of the function
	   //  then mark the point as an exit point, and patch the
	   //  jump target - the jump target needs to be patched because
	   //  the jump target is NOT REALLY AN ABSOLUTE ADDRESS.  It is 
	   //  actually a PC relative offset - and the relocated function
	   //  will have a different PC. -matt

	   relocateInstruction(&newInstr[i],adr+baseAddress,newAdr,proc);
	   instPoint *point = new instPoint(this, newInstr[i], owner, 
					    newAdr, false, 
					    functionExit, adr);
           point->relocated_ = true;
	   disp = newInstr[i].branch.disp22;
	   if ((instr.branch.cond != 0) && (instr.branch.cond != 8)) {  
	       point->isBranchOut = true;
	       point->branchTarget = adr + (disp<<2);
	   }
           // if location was this point, change it to new point
           if(location == funcReturns[retId]) { 
	       location = point;
           }
           retId++;
           reloc_info->addFuncReturn(point);
       } else {
	   // If the branch was to a target INSIDE the function, then
	   // the branch target also needs to be updated. 
	   // a. it should point to the relocated function
	   // - because the branch is PC relative, the target DOESN'T
	   //  need to be updated for this (it will be the same relative 
           //  to the PC location inside the relocated function as it was
	   //  to the PC location inside the non-relocated function.)
	   // b. however, it may need to point to a slightly different location
	   //  inside the relocated function - this is because pieces of the 
	   //  function code may be expanded during the instrumentation 
           //  process....
	   // -matt

	   //  branch how far....
	   disp = newInstr[i].branch.disp22;
	   //  by how many instructions is code between the src and 
           //   destination expanded....
	   extra_offset = fer->GetShift(adr - start_adr +
		 disp * sizeof(instruction)) -
	         fer->GetShift(adr - start_adr);
	   
	   //if (extra_offset != 0) {
	   //    cerr << "WARNING : function " << prettyName().string_of()
	   //	  << " using addition FER offset of " << extra_offset
	   //	  << " at instruction " << adr - start_adr << endl;
	   //}

	   //  Dont want to use relocateInstruction function here, as
	   //   the branching is PC-relative, and thus not changed by
	   //   relocation....
	   assert(extra_offset % 4 == 0);
	   newInstr[i].branch.disp22 += extra_offset >> 2;
       }

     } 
     else if ((call_restore_tc = CallRestoreTC(instr, nexti))
	      || (jmp_nop_tc = JmpNopTC(instr, nexti, adr, this))) {
        // First, check for tail-call optimization: a call where the instruction 
        // in the delay slot write to register %o7(15), usually just moving
        // the caller's return address, or doing a restore
        // Tail calls are instrumented as return points, not call points.
        // New Tail-Call Optimization sequence (found by pd grp 
        //  to-date only in Solaris 2.6, not in 2.5 or 2.4)
        //  jmp %reg, nop as last 2 instructions in function w/o
        //  own stack frame.  This sequence gets rewritten so
        //  as to seperate the call site and exit point, similar
        //  to what's done in the call, restore TC opt sequence
        //  above.
	    // Undoing the tail-call optimazation when the function
	    // is relocated. Here is an example:
	    //   before:          --->             after
	    // ---------------------------------------------------
	    //                                    mov %reg %g1
	    //   call  %reg                       mov %i0 %o0
	    //   restore                          mov %i1 %o1
	    //                                    mov %i2 %o2
	    //                                    mov %i3 %o3
	    //                                    mov %i4 %o4
	    //                                    mov %i5 %o5
	    //                                    call %g1
	    //                                    nop
	    //                                    mov %o0 %i0
	    //                                    mov %o1 %i1
	    //                                    mov %i2 %o2
	    //                                    mov %i3 %o3
	    //                                    mov %i4 %o4
	    //                                    mov %i5 %o5
	    //                                    ret
	    //                                    restore
	    //   before:          --->             after
	    // ---------------------------------------------------
	    //   call  PC_REL_ADDR                mov %i0 %o0
	    //   restore                          mov %i1 %o1
	    //                                    mov %i2 %o2
	    //                                    mov %i3 %o3
	    //                                    mov %i4 %o4
	    //                                    mov %i5 %o5
	    //                                    call PC_REL_ADDR'
	    //                                    nop
	    //                                    mov %o0 %i0
	    //                                    mov %o1 %i1
	    //                                    mov %i2 %o2
	    //                                    mov %i3 %o3
	    //                                    mov %i4 %o4
	    //                                    mov %i5 %o5
	    //                                    ret
	    //                                    restore
            //   before:          --->             after
	    // ---------------------------------------------------
            //                                    save  %sp, -96, %sp
	    //                                    mov %reg %g1
	    //   jmp  %reg                        mov %i0 %o0
	    //   restore                          mov %i1 %o1
	    //                                    mov %i2 %o2
	    //                                    mov %i3 %o3
	    //                                    mov %i4 %o4
	    //                                    mov %i5 %o5
	    //                                    call %g1
	    //                                    nop
	    //                                    mov %o0 %i0
	    //                                    mov %o1 %i1
	    //                                    mov %i2 %o2
	    //                                    mov %i3 %o3
	    //                                    mov %i4 %o4
	    //                                    mov %i5 %o5
	    //                                    ret
	    //                                    restore

	 //    Note : Assuming that %g1 is safe to use, since g1 is scratch
	 //     register that is defined to be volatile across procedure
	 //     calls.
         //    My barf for hand written assembly code which violates this
         //     assumption.
	    // Q: Here the assumption that register i1 is available 
	    //    might be an question, is it?
	    // A: I think it is appropriate because:
	    //      (in situation A calls B and B calls C)
	    //      The procedure C called via tail call is a leaf 
	    //      procedure, the value arguments and return value between
	    //      A and C are passed by register (o1...o5, o7)
	    //      So even If B mess up the value of i0, it won't affect the
	    //      commnucation between A and C. Also, we saved the value of
	    //      i0 on stack and when we return from B, the value of i0
	    //      won't be affected.
	    //      If C is not a leaf procedure, it should be fine
	    //      as it is.
	    //    ( If you could give an counter-example, please
	    //      let me know.                         --ling )

            //  Should both be false for jmp, nop tail call sequences........
	    bool true_call = isTrueCallInsn(instr);
	    bool jmpl_call = isJmplCallInsn(instr);

            if (call_restore_tc) {
	        if (!true_call && !jmpl_call) {
	            cerr << "WARN : attempting to unwind tail-call optimization, call instruction appears to be neither TRUE call nor JMPL call, bailing.  Function is : " << prettyName().string_of() << endl;
	            return FALSE;
	        }

	        // if the call instruction was a call to a register, stick in extra
	        //  initial mov as above....
	        if (jmpl_call) {
	            // added extra mv *, g1
	            // translation : mv inst %1 %2 is synthetic inst. implemented as
	            //  orI %1, 0, %2  
	            genImmInsn(&newInstr[i++], ORop3, instr.rest.rs1, 0, 1);
	        }
	    } else {
	        assert(jmp_nop_tc);
		
		// generate save instruction to free up new stack frame for
		//  inserted call....
		// ALERT ALERT - -112 seems like apparently random number
		//  used in solaris specific code where save instructions 
		//  are generated.  Why -112?
		genImmInsn(&newInstr[i++], SAVEop3, REG_SP, -112, REG_SP);
		 
		// mv %reg %g1
		genImmInsn(&newInstr[i++], ORop3, instr.rest.rs1, 0, 1);
	    }

	    // generate mov i0 ... i5 ==> O0 ... 05 instructions....
	    // as noted above, mv inst %1 %2 is synthetic inst. implemented as
	    //  orI %1, 0, %2  
	    genImmInsn(&newInstr[i++], ORop3, REG_I(0), 0, REG_O(0));
	    genImmInsn(&newInstr[i++], ORop3, REG_I(1), 0, REG_O(1));
	    genImmInsn(&newInstr[i++], ORop3, REG_I(2), 0, REG_O(2));
	    genImmInsn(&newInstr[i++], ORop3, REG_I(3), 0, REG_O(3));
	    genImmInsn(&newInstr[i++], ORop3, REG_I(4), 0, REG_O(4));
	    genImmInsn(&newInstr[i++], ORop3, REG_I(5), 0, REG_O(5));

	    err = false;

	    if (jmp_nop_tc) {
	        // if origional jmp/call instruction was call to a register, that
	        //  register should have been pushed into g0, so generate a call
	        //  to %g0.
	        //  generate <call %g1>
	        generateJmplInsn(&newInstr[i], 1, 0, 15);

		newCallPoint(newAdr += 8 *sizeof(instruction), newInstr[i], owner,
			     err, callsId, adr,
			     reloc_info, location);
		i++;

		// here, 18 instructions are generated and used to replace
		//  the origional 2 (jmp, nop), resulting in an addition 
		//  of 16 instructions....
		// 8 instructions added above, so add 8 more here....
		newAdr += 8 * sizeof(instruction);
	        
	    } else {
	        assert(call_restore_tc);
	        if (jmpl_call) { 
	            // if origional jmp/call instruction was call to a register, that
	            //  register should have been pushed into g0, so generate a call
	            //  to %g0.
	            //  generate <call %g1>
	            generateJmplInsn(&newInstr[i], 1, 0, 15);
		    
		    newCallPoint(newAdr += 7 *sizeof(instruction), newInstr[i],
			     owner, err, callsId, adr,
			     reloc_info, location);

		    i++;
		    // in the case of a jmpl call, 17 instructions are generated
	            // and used to replace origional 2 (call, restore), resulting
	            // in a new addition of 15 instructions....
		    // 7 instructions added above, so add 8 more here....
	            newAdr += 8 * sizeof(instruction); 
	        } else {
	            // if the origional call was a call to an ADDRESS, then want
	            //  to copy the origional call.  There is, however, a potential
	            //  caveat:  The sparc- CALL instruction is apparently PC
	            //  relative (even though disassemblers like that in gdb will
	            //  show a call to an absolute address).
	            //  As such, want to change the call target to account for 
	            //  the difference in PCs.

		    newAdr += 6 * sizeof(instruction);

	            newInstr[i].raw = owner->get_instruction(adr);
	            relocateInstruction(&newInstr[i],
		        adr+baseAddress,
		        newAdr,
			proc);
		    
		    // mark the call instruction as a call site....
		    //  alert -
		    //  callsId looks ok, only used in calls to newInstPoint
		    //  reloc_info looks ok as long as new call site added in 
		    //   newCallPoint? - CHECK
		    //  location looks ok as long as compared with funcReturns[id?}
		    //   ? - CHECK
		    newCallPoint(newAdr,
                    		 newInstr[i], owner, err, callsId, adr,
                    		 reloc_info, location); 

		    i++;

		    // in the case of a "true" call, 16 instructions are generated
	            //  + replace origional 2, resulting in addition of 18 instrs.
		    // Addition of 14 total should make newAdr point to
		    //  the retl instruction 2nd to last in unwound tail-call 
		    //  opt. sequence....
		    // 6 instructions were added above, so add 8 more here....
	            newAdr += 8 * sizeof(instruction);
	        }
	    }

	    // err should have been set in call to newCallPoint - to mark
	    //  relocated call insn as cal site....
	    if (err) return false;

	    // generate NOP following call instruction (for delay slot)
	    //  ....
	    generateNOOP(&newInstr[i++]);

	    // generate mov instructions moving %o0 ... %o5 ==> 
	    //     %i0 ... %i5.  
            genImmInsn(&newInstr[i++], ORop3, REG_O(0), 0, REG_I(0));
	    genImmInsn(&newInstr[i++], ORop3, REG_O(1), 0, REG_I(1));
	    genImmInsn(&newInstr[i++], ORop3, REG_O(2), 0, REG_I(2));
	    genImmInsn(&newInstr[i++], ORop3, REG_O(3), 0, REG_I(3));
	    genImmInsn(&newInstr[i++], ORop3, REG_O(4), 0, REG_I(4));
	    genImmInsn(&newInstr[i++], ORop3, REG_O(5), 0, REG_I(5));
	    
	    // generate ret instruction
            generateJmplInsn(&newInstr[i++], REG_I(7), 8 ,0);

	    // generate restore operation....
	    genSimpleInsn(&newInstr[i], RESTOREop3, 0, 0, 0);

	    // and note retl location as return point....
	    instPoint *point = new instPoint(this, instr, owner, newAdr, false,
				      functionExit, adr);
	    point-> originalInstruction = newInstr[i-1];
	    point-> delaySlotInsn = newInstr[i];
	    point-> isDelayed = true;
            point->relocated_ = true;
            // if location was this point, change it to new point
            if(location == funcReturns[retId]) { 
		location = point;
            }
            retId++;
            reloc_info->addFuncReturn(point);

	    // skip the restore instruction (in nexti)....
	    adr += 4;
	    newAdr += 4;
     }
     else if (isCallInsn(instr)) {
	 // if the second instruction in the function is a call instruction
         // then this cannot go in the delay slot of the branch to the
         // base tramp, so add a noop between first and second instructions
	 // in the relocated function (check out write in libc.so.1 for
	 // and example of this):
	 //
	 //     save  %sp, -96, %sp             brach to base tramp
	 //     call  0x73b70                   nop
	 //                                     call 0x73b70
	 if(adr == (getAddress(0)+4)){
	     newInstr[i+1] = instr;
	     generateNOOP(&newInstr[i]);
	     i++;
	     newAdr += 4;
	 }

	 // if this is a call to an address within the same function, then
	 // we need to set the 07 register to have the same value as it
	 // would before the function was relocated
	 // to do this we generate a call instruction back to the original
	 // function location, and then at this location we generate a call 
	 // instruction back to the relocated instruction.  In the delay 
	 // slot of the second instruction the value of 07 is changed by 
	 // the difference between the origninal call instruction, and 
	 // the location of the call instruction back to the relocated
	 // function.  This way the 07 register will contain the address
	 // of the original call instruction
         //
	 // Assumes call is "true" call, NOT jmpl call....
	 Address call_target = adr + (instr.call.disp30 << 2);
         if((call_target >= getAddress(0)) 
		&& (call_target <= (getAddress(0) + size()))){ 
	    assert((newInstr[i].call.disp30 << 2) == 8);

	    // generating call instruction to orginal function address
	    // after the SAVE, CALL, RESTORE instr.s that call the relocated
	    // function 
	    disp = newInstr[i].call.disp30;
            newInstr[i].call.disp30 = ((call_start_addr -newAdr) >> 2); 


	    // generate call to relocated function from original function 
	    // (this will get almost correct value for register 07)
	    // NOTE - also want to keep track of any code expansion which
	    //  occurs in function between origional call location and 
	    //  target, and patch call targets appropriately....
            instruction new_inst;
	    extra_offset = fer->GetShift((i + disp) * sizeof(instruction)) -
	      fer->GetShift(i * sizeof(instruction));
	    
	    //if (extra_offset != 0) {
	    //    cerr << "WARNING : function " << prettyName().string_of()
	    //	    << " using addition FER offset of " << extra_offset
	    //	    << " at instruction " << i * 4 << endl;
	    //}

	    // the call (stuck in the origional function) should go to
	    // 2 instructions after newAdr (which will be a call back to 
	    // the patched call instruction).  Call to newAdr + 2 (instead
	    // of newAdr + 1) because the call at newAdr has a delay slot,
	    // calling to newAdr + 1 would end up executing the instruction
	    // in the delay slot twice.
	    generateCallInsn(&new_inst,call_start_addr,
			     newAdr + 2 * sizeof(instruction) + extra_offset);
            callInstrs += new_inst;
	   

	    // generate add isntruction to get correct value for 07 register 
	    // this will go in delay slot of previous call instr.
	    // NOTE 1 - looks to me like this will be correct ONLY when 
	    //  this trick of generating a call to the same place in the 
	    //  function to set the 07 register is done from the beginning
	    //  of the function (2nd instruction)????
	    //  -matt
	    genImmInsn(&new_inst,ADDop3,REG_O7,
		       (adr+baseAddress-call_start_addr),REG_O7);
            callInstrs += new_inst;
	    call_start_addr += 2*sizeof(instruction);
         }
	 else {
	    // otherwise, this is a call instruction to a location
	    // outside the function
	    relocateInstruction(&newInstr[i],adr+baseAddress,newAdr,proc);
	    (void)newCallPoint(newAdr, newInstr[i], owner, err, 
			       callsId, adr,reloc_info,location);
	    if (err) return false;
         }
     }

     else if (isInsnType(instr, JMPLmask, JMPLmatch)) {
	 reg jumpreg = instr.rest.rs1;
         nexti.raw = owner->get_instruction(adr+4);

       /* A register indirect jump. Some jumps may exit the function 
          (e.g. read/write on SunOS). In general, the only way to 
	  know if a jump is exiting the function is to instrument
	  the jump to test if the target is outside the current 
	  function. Instead 
of doing this, we just check the 
	  previous two instructions, to see if they are loading
	  an address that is out of the current function.
	  This should catch the most common cases (e.g. read/write).
	  For other cases, we would miss a return point.

	  This is the case considered:

	     sethi addr_hi, r
	     or addr_lo, r, r
	     jump r
	*/
	 instruction prev1;
	 instruction prev2;
	 
	 prev1.raw = owner->get_instruction(adr-4);
	 prev2.raw = owner->get_instruction(adr-8);

	 unsigned targetAddr;

	 if (instr.rest.rd == 0 && (instr.rest.i == 1 || instr.rest.rs2 == 0)
	     && prev2.sethi.op == FMT2op && prev2.sethi.op2 == SETHIop2 
	     && prev2.sethi.rd == (unsigned)jumpreg
	     && prev1.rest.op == RESTop 
	     && prev1.rest.rd == (unsigned)jumpreg && prev1.rest.i == 1
	     && prev1.rest.op3 == ORop3 && prev1.rest.rs1 == (unsigned)jumpreg){
	     
	     targetAddr = (prev2.sethi.imm22 << 10) & 0xfffffc00;
	     targetAddr |= prev1.resti.simm13;
	     if ((targetAddr < getAddress(0)) 
		 || (targetAddr >= (getAddress(0)+size()))) {
		 instPoint *point = new instPoint(this, instr, owner, 
						  newAdr, false,
						  functionExit, adr);
                 point->relocated_ = true;
                 // if location was this point, change it to new point
                 if(location == funcReturns[retId]) { 
		     location = point;
                 }
                 retId++;
                 reloc_info->addFuncReturn(point);
	     }
	 }
     }
 }
   
   return true;
}

// Take an (empty) FunctionExpansionRecord, and fill it with data describing
//  how blocks of instructions inside function are shifted around by expansion
//  or contraction of rewritten code blocks.
// Also fills in a variable describing the total amount by which function size 
//  changes.... 
// return value - boolean indicating whether operations could be successfully
//  performed.  Shoudl return TRU2E unless something is foudn to indicate that
//  function cannot be successfully relocated given current impl. of relocation
//  and instrumentation code.
//  Currently, in the solaris implementation, logical code blocks are expanded
//   when a function is relocated in the following cases:
//   1. when unwinding tail-call optimizations (call, restore sequence).
//   2. when the 2nd instruction in the function is a call instruction.
bool pd_Function::calcRelocationExpansions(const image *owner,
        FunctionExpansionRecord *fer, int *size_change) {


    int i, total_shift = 0;
    Address adr = getAddress(0);
    instruction instr, nexti;

    for(i=0; adr < getAddress(0) + size(); adr += 4, i++) {
        instr.raw = owner->get_instruction(adr);
	nexti.raw = owner->get_instruction(adr+4);

	if (isCallInsn(instr)) {
	    // if 2nd instruction is call instruction, code adds 1 extra
	    //  nop instruction in the delay slot....
   	    if (i == 1) {
	        // add 1 instruction (4 bytes long) of offset to address of any
	        // logical block of function starting *after* that origional call....
	        fer->AddExpansion(8, 4);
	        total_shift += 4 * sizeof(instruction);
	    }

	    // if next instruction is "restore", then sequence is recognised as
	    //  a tail-call optimization, and unwound....
	    if (CallRestoreTC(instr, nexti)) {
	        // adds either 7 or 8 extra instructions, depending on which type
	        // of call the call instruction is (e.g. call %REGISTER, call ADDRESS,
	        // etc....)....

	        // call %REGISTER is actually a "JMPL" instruction in SPARC arch, 
                //  although gdb's "disass" will show "call %REGISTER"....
	        //  This current manner in which this is unwound results in 17 instructions
	        //  where the origional had 2 (call + restore)....
	        if (isJmplCallInsn(instr)) {
		    fer->AddExpansion(i*sizeof(instruction) + 8,
				      15 * sizeof(instruction));
		    total_shift += 15 * sizeof(instruction);
	        } else if (isTrueCallInsn(instr)) {
		    // call ADDR results in 16 instructions where origional code
		    //  had 2 (net increase of 14 instructions).... 
	 	    fer->AddExpansion(i*sizeof(instruction) + 8,
				      14 * sizeof(instruction));
		    total_shift += 14 * sizeof(instruction);
	        } else {
		    // relocation code currently does NOT deal with other types of
		    //  call ; restore sequence (e.g. call %REGISTER + OFFSET; restore).
		    //   signal dont know how much rewriting of such code sequence will
		    //  expand it....
		    return FALSE;
	        }
    	    }
	}
	// Also search for a new form of tail-call optimization 
	//  (to date only seen in Solaris 2.6) : a jump %reg, nop
	//  at the end of a function which doesn't have a stack
	//  frame....
        // This type of tail-call optimization adds HOW_BIG
	//  instructions to function size....
	if (JmpNopTC(instr, nexti, adr, this)) {
	    fer->AddExpansion(i * sizeof(instruction), 16);
	    total_shift += 16 * sizeof(instruction);
	}
    }

    *size_change = total_shift;
    return TRUE;
}













