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

#include "dyninstAPI/src/inst-sparc.h"

// Another constructor for the class instPoint. This one is called
// for the define the instPoints for regular functions which means
// multiple instructions is going to be moved to based trampoline.
// Since we will use the instruction CALL to branch to the base
// tramp(so it doesn't have any code size restriction), things are
// a little more complicated because instruction CALL changes the 
// value in the link register.
instPoint::instPoint(pd_Function *f, const instruction &instr, 
		     const image *owner, Address &adr,
		     bool delayOK, bool isLeaf, instPointType pointType)
: addr(adr), originalInstruction(instr), inDelaySlot(false), isDelayed(false),
  callIndirect(false), callAggregate(false), callee(NULL), func(f),
  leaf(isLeaf), ipType(pointType), image_ptr(owner), firstIsConditional(false),
  relocated_(false), isLongJump(true)
{

  isBranchOut = false;
  size = 0;

  // When the function is not a leaf function 
  if (!leaf) {

      // we will treat the first instruction after the SAVE instruction
      // in the nonleaf procedure as the function entry.  
      if (ipType == functionEntry) {

	  assert(isInsnType(instr, SAVEmask, SAVEmatch));
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

	      // Life is hard. If the second instruction is actually
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
	      size += 1*sizeof(instruction);
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
	      }
	  }

	  originalInstruction.raw = owner->get_instruction(addr);
	  otherInstruction.raw = owner->get_instruction(addr+4);
	  delaySlotInsn.raw = owner->get_instruction(addr+8);
	  size += 3*sizeof(instruction);

	  if (inDelaySlot) {
	      inDelaySlotInsn.raw = owner->get_instruction(addr+12);
	  }

      } else {
	  // Of course, the leaf function could not have call sites. 
	  logLine("Internal Error: in inst-sparc.C.");
	  abort();
      }
  }

  // return the address in the code segment after this instruction
  // sequence. (there's a -1 here because one will be added up later in
  // the function findInstPoints)  
  adr = addr + (size - 1*sizeof(instruction));
}


void AstNode::sysFlag(instPoint *location)
{
    // astFlag = location->func->isTrapFunc();
    if (astFlag == false)
	astFlag = (location -> isLongJump)? false:true; 
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

  vector<instPoint*> non_lib;

  for (unsigned i=0; i<calls.size(); ++i) {
    /* check to see where we are calling */
    p = calls[i];
    assert(p);

    if (isInsnType(p->originalInstruction, CALLmask, CALLmatch)) {
      // Direct call
      loc_addr = p->addr + (p->originalInstruction.call.disp30 << 2);
      pd_Function *pdf = (file_->exec())->findFunction(loc_addr);
      if (pdf && !pdf->isLibTag()) {
	p->callee = pdf;
	non_lib += p;
      } else {
          delete p;
      }
    } else {
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

    err = true;
    if (isTrap) {
	point = new instPoint(this, instr, owner, adr, false, callSite, oldAddr);
    } else {
	point = new instPoint(this, instr, owner, adr, false, false, callSite);
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
	    assert(((u_int)callId) < calls.size());
	    point->relocated_ = true;
	    // if the location was this call site, then change its value
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
    u_int newOffset;

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
	    // TODO: this is the wrong branch offset
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
trampTemplate *installBaseTramp(const instPoint *&location, process *proc)
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
	    // If it is a leaf function, genereate a RESTORE instruction
            // since there's an instruction SAVE generated and put in the
            // code segment.
	    if (location -> leaf) {
		genImmInsn(temp, RESTOREop3, 0, 0, 0);
		temp++;
		currAddr += sizeof(instruction);
	    } 

	    // Same for the leaf and nonleaf functions.
            // First, relocate the "FIRST instruction" in the sequence;  
	    *temp = location->originalInstruction;
	    Address fromAddr = location->addr;
	    relocateInstruction(temp,fromAddr,currAddr,(process *)proc);

	    // Again, for leaf function, one more is needed to move for one
	    // more spot;
	    if (location->leaf) {
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
	    if (location->leaf) {
		if (location->inDelaySlot) {
		    fromAddr += sizeof(instruction);
		    currAddr += sizeof(instruction);
		    *++temp = location->inDelaySlotInsn;
		    relocateInstruction(temp,fromAddr,currAddr,(process *)proc);
		} 
		
		genImmInsn(temp+1, SAVEop3, REG_SP, -112, REG_SP);
	    }
	    
	} else if (temp->raw == RETURN_INSN) {
	    // Back to the code segement of the application.
            // If the location is in the leaf procedure, generate an RESTORE
	    // instruction right after the CALL instruction to restore all
	    // the values in the registers.
	    if (location -> leaf) {
		generateCallInsn(temp, currAddr, location->addr+location->size);
		genImmInsn(temp+1, RESTOREop3, 0, 0, 0);
	    } else {
		generateCallInsn(temp, currAddr, location->addr+location->size);
	    }
        } else if (temp->raw == SKIP_PRE_INSN) {
	    unsigned offset;
	    offset = baseAddr+baseTemplate.updateCostOffset-currAddr;
	    generateAnnulledBranchInsn(temp,offset);

        } else if (temp->raw == SKIP_POST_INSN) {
	    unsigned offset;
	    offset = baseAddr+baseTemplate.returnInsOffset-currAddr;
	    generateAnnulledBranchInsn(temp,offset);

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
		// only get to the tramp is the branch is taken, so we generate
		// a unconditional branch to the target of the original 
		// instruction here 
                assert(location->branchTarget);
                int disp = location->branchTarget - currAddr;
                generateAnnulledBranchInsn(temp, disp);
                disp = temp->branch.disp22;
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
            generateAnnulledBranchInsn(temp, 
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
          generateAnnulledBranchInsn(temp,offset);
        } else if (temp->raw == SKIP_POST_INSN) {
          unsigned offset;
          offset = baseAddr+baseTemplate.returnInsOffset-currAddr;
          generateAnnulledBranchInsn(temp,offset);
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
 * we need for modifying the code segment
 *
 */
trampTemplate *findAndInstallBaseTramp(process *proc, 
				 instPoint *&location,
				 returnInstance *&retInstance,
				 bool )
{
    Address adr = location->addr;
    trampTemplate *ret;
    retInstance = NULL;
    if (!proc->baseMap.defines((const instPoint *)location)) {
	if (location->func->isTrapFunc()) {
	    // Install Base Tramp for the functions which are 
	    // relocated to the heap.
            vector<instruction> extra_instrs;
	    ret = installBaseTrampSpecial(location,proc,extra_instrs);
	    if (location->isBranchOut){
	        changeBranch(proc, location->addr, (int) ret->baseAddr, 
			     location->originalInstruction);
            } else {
	        generateBranch(proc, location->addr, (int)ret->baseAddr);
            }

	    // If for this process, a call to the relocated function has not
	    // yet be installed in its original location, then genterate the
	    // following instructions at the begining of the function:
	    //   SAVE;             CALL;         RESTORE.
	    // so that it would jump the start of the relocated function
	    // which is in heap.
	    if(!(location->func->isInstalled(proc))){
	    	location->func->setInstalled(proc);
		u_int e_size = extra_instrs.size();
		instruction *insn = new instruction[3 + e_size];
		Address adr = location-> func -> getAddress(0);
		genImmInsn(insn, SAVEop3, REG_SP, -112, REG_SP);
		generateCallInsn(insn+1, adr+4, 
				 location->func->getAddress(proc));
		genSimpleInsn(insn+2, RESTOREop3, 0, 0, 0); 
		for(u_int i=0; i < e_size; i++){
		    insn[3+i] = extra_instrs[i];
		}
		retInstance = new returnInstance((instructUnion *)insn, 
					 (3+e_size)*sizeof(instruction), 
					 adr, location->func->size());
                assert(retInstance);

                //cerr << "created a new return instance (relocated fn)!" << endl;
	    }

	} else {
	    // Install base tramp for all the other regular functions. 
	    ret = installBaseTramp(location, proc);
	    if (location->leaf) {
		// if it is the leaf function, we need to generate
		// the following instruction sequence:
		//     SAVE;      CALL;      NOP.
		instruction *insn = new instruction[3];
		genImmInsn(insn, SAVEop3, REG_SP, -112, REG_SP);
		generateCallInsn(insn+1, adr+4, (int) ret->baseAddr);
		generateNOOP(insn+2);
		retInstance = new returnInstance((instructUnion *)insn, 
						 3*sizeof(instruction), adr, 
						 3*sizeof(instruction));
                assert(retInstance);
	    } else {
		// Otherwise,
		// Generate branch instruction from the application to the
		// base trampoline and no SAVE instruction is needed
		instruction *insn = new instruction[2];	
		assert(insn);

		generateCallInsn(insn, adr, (int) ret->baseAddr);
		generateNOOP(insn+1);

		retInstance = new returnInstance((instructUnion *)insn, 
						 2*sizeof(instruction), adr, 
						 2*sizeof(instruction));
                assert(retInstance);
	    }
	}

	proc->baseMap[(const instPoint *)location] = ret;

    } else {
        ret = proc->baseMap[(const instPoint *)location];
    }
      
    return(ret);
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
	    atAddr = inst->baseInstance->baseAddr+baseTemplate.skipPostInsOffset
	    ;
	    inst->baseInstance->cost += inst->baseInstance->postBaseCost;
	    inst->baseInstance->postInstru = true;
	    generateNoOp(inst->proc, atAddr);
        }
    }
    // What does this next statement do? atAddr could be not initialized here!
    //  jkh 3/12/97
    //generateNoOp(inst->proc, atAddr);
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
		logLine(errorLine);
		showErrorCallback(80, (const char *) errorLine);
		P_abort();
	    }
	    // TODO: is this correct or should we get relocated address?
	    addr = func->getAddress(0);
	}
	
	for (unsigned u = 0; u < operands.size(); u++)
	    srcs += operands[u]->generateCode(proc, rs, i, base, noCost);

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
	generateSetHi(insn, dest, src2);
	insn++;

	generateStore(insn, src1, src2, LOW10(dest));

	base += sizeof(instruction)*2;
    } else if (op ==  storeIndirOp) {
	generateStore(insn, src1, dest, 0);
	base += sizeof(instruction);
    } else if (op ==  ifOp) {
	// cmp src1,0
	genSimpleInsn(insn, SUBop3cc, src1, 0, 0); insn++;
	genBranch(insn, dest, BEcond, false); insn++;

	generateNOOP(insn);
	base += sizeof(instruction)*3;
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
    } else if (op ==  trampTrailer) {
	// dest is in words of offset and generateBranchInsn is bytes offset
	generateAnnulledBranchInsn(insn, dest << 2);
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

/*
 * Find the instPoints of this function.
 */
bool pd_Function::findInstPoints(const image *owner) {

   if (size() == 0) {
     return false;
   }

   leaf = true;
   Address adr;
   Address adr1 = getAddress(0);
   instruction instr;
   instr.raw = owner->get_instruction(adr1);
   if (!IS_VALID_INSN(instr))
     return false;

   // If it contains an instruction, I assume it would be s system call
   // which will be treat differently. 
   isTrap = false;
   bool func_entry_found = false;

   for ( ; adr1 < getAddress(0) + size(); adr1 += 4) {
       instr.raw = owner->get_instruction(adr1);

       // If there's an TRAP instruction in the function, we 
       // assume that it is an system call and will relocate it 
       // to the heap
       if (isInsnType(instr, TRAPmask, TRAPmatch)) {
	   isTrap = true;
	   return findInstPoints(owner, getAddress(0), 0);
       } 

       // The function Entry is defined as the first SAVE instruction plus
       // the instructions after this.
       // ( The first instruction for the nonleaf function is not 
       //   necessarily a SAVE instruction. ) 
       if (isInsnType(instr, SAVEmask, SAVEmatch) && !func_entry_found) {

	   leaf = false;
	   func_entry_found = true;
	   funcEntry_ = new instPoint(this, instr, owner, adr1, true, leaf, 
				      functionEntry);
	   adr = adr1;
	   assert(funcEntry_);
       }
   }

   // If there's no SAVE instruction found, this is a leaf function and
   // and function Entry will be defined from the first instruction
   if (leaf) {
       adr = getAddress(0);
       instr.raw = owner->get_instruction(adr);
       funcEntry_ = new instPoint(this, instr, owner, adr, true, leaf,
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
       funcReturns += new instPoint(this, instr, owner, adr, false, leaf, 
				    functionExit);

     } else if (instr.branch.op == 0 
		&& (instr.branch.op2 == 2 || instr.branch.op2 == 6) 
		&& (instr.branch.cond == 0 ||instr.branch.cond == 8)) {
       // find if this branch is going out of the function
       int disp = instr.branch.disp22;
       Address target = adr +  (disp << 2);
       if ((target < (getAddress(0)))  
	   || (target >= (getAddress(0) + size()))) {
	 instPoint *point = new instPoint(this, instr, owner, adr, false, leaf, 
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
	        return false;
       }}
       // first, check for tail-call optimization: a call where the instruction 
       // in the delay slot write to register %o7(15), usually just moving
       // the caller's return address, or doing a restore
       // Tail calls are instrumented as return points, not call points.


       instruction nexti; 
       nexti.raw = owner->get_instruction(adr+4);

       if (nexti.rest.op == 2 
	   && ((nexti.rest.op3 == ORop3 && nexti.rest.rd == 15)
	      || nexti.rest.op3 == RESTOREop3)) {
	 funcReturns += new instPoint(this, instr, owner, adr, false, leaf, 
				      functionExit);

       } else {
	 // define a call point
	 // this may update address - sparc - aggregate return value
	 // want to skip instructions
	 bool err;
	 int dummyId;
	 instPoint *blah = 0;
	 adr = newCallPoint(adr, instr, owner, err, dummyId, adr,0,blah);
       }
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
					    leaf, functionExit);
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

    // The function is too small to be worthing instrumenting.
    if (size() <= 12)
	return false;

    // No function return! return false;
    if (sizeof(funcReturns) == 0)
	return false;

    instruction instr;
    Address adr = getAddress(0);

    // Check if there's any branch instruction jump to the middle
    // of the instruction sequence in the function entry point
    // and function exit point.
    for ( ; adr < getAddress(0) + size(); adr += sizeof(instruction)) {

	instr.raw = owner->get_instruction(adr);

	if (isInsnType(instr, BRNCHmask, BRNCHmatch)||
	    isInsnType(instr, FBRNCHmask, FBRNCHmatch)) {

	    int disp = instr.branch.disp22;
	    Address target = adr + (disp << 2);

	    if ((target > funcEntry_->addr)&&
		(target < (funcEntry_->addr + funcEntry_->size))) {
		if (adr > (funcEntry_->addr+funcEntry_->size))
		    return false;
	    }

	    for (u_int i = 0; i < funcReturns.size(); i++) {
		if ((target > funcReturns[i]->addr)&&
		    (target < (funcReturns[i]->addr + funcReturns[i]->size))) {
		    if ((adr < funcReturns[i]->addr)||
			(adr > (funcReturns[i]->addr + funcReturns[i]->size)))
			return false;
		}
	    }
	}
    }

    return true;	
}

// This function is to find the inst Points for a function
// that will be relocated if it is instrumented. 
bool pd_Function::findInstPoints(const image *owner, Address newAdr, 
				process *proc){

   int i;
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
   // or a branch instruction, then we can't deal with this 
   if(size() > sizeof(instruction)){
       Address second_adr = adr + sizeof(instruction);
       instruction second_instr;
       second_instr.raw =  owner->get_instruction(second_adr); 
       if ((isCallInsn(second_instr)) || 
		      (second_instr.branch.op == 0 && 
		      (second_instr.branch.op2 == 2 || 
		      second_instr.branch.op2 == 6))) {
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
	   relocateInstruction(&newInstr[i], adr, newAdr, proc);
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

       // first, check for tail-call optimization: a call where the instruction 
       // in the delay slot write to register %o7(15), usually just moving
       // the caller's return address, or doing a restore
       // Tail calls are instrumented as return points, not call points.
       instruction nexti; 
       nexti.raw = owner->get_instruction(adr+4);

       if (nexti.rest.op == 2 
	   && ((nexti.rest.op3 == ORop3 && nexti.rest.rd == 15)
	      || nexti.rest.op3 == RESTOREop3)) {

           instPoint *point = new instPoint(this, instr, owner, newAdr, false,
				      functionExit, adr);
           funcReturns += point;
           funcReturns[retId] -> instId = retId++;

       } else {

	 // define a call point
	 // this may update address - sparc - aggregate return value
	 // want to skip instructions
	 bool err;
	 instPoint *blah = 0;
	 adr = newCallPoint(newAdr, instr, owner, err, callsId, adr,0,blah);
	 if (err)
	   return false;
       }
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
				vector<instruction> &,
				relocatedFuncInfo *reloc_info) {

   int i;
   if (size() == 0) {
     return false;
   }
   assert(reloc_info);

   Address adr = getAddress(0);
   instruction instr;
   instr.raw = owner->get_instruction(adr);
   if (!IS_VALID_INSN(instr))
     return false;

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

   for (i = 0; adr < getAddress(0) + size(); adr += 4,  newAdr += 4, i++) {
    
     instr.raw = owner->get_instruction(adr);
     newInstr[i] = instr;

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
       // find if this branch is going out of the function
       int disp = instr.branch.disp22;
       Address target = adr + (disp << 2);
       if ((target < (getAddress(0))) 
	   || (target >= (getAddress(0) + size()))) {
	   relocateInstruction(&newInstr[i],adr,newAdr,proc);
	   instPoint *point = new instPoint(this, newInstr[i], owner, 
					    newAdr, false, 
					    functionExit, adr);
           point->relocated_ = true;
	   // TODO is this the correct displacement???
	   disp = newInstr[i].branch.disp22;
	   if ((instr.branch.cond != 0) && (instr.branch.cond != 8)) {  
	       point->isBranchOut = true;
	       // TODO is this the correct target???
	       point->branchTarget = target;
	   }
           // if location was this point, change it to new point
           if(location == funcReturns[retId]) { 
	       location = point;
           }
           retId++;
           reloc_info->addFuncReturn(point);
       }

     } else if (isCallInsn(instr)) {

       // first, check for tail-call optimization: a call where the instruction 
       // in the delay slot write to register %o7(15), usually just moving
       // the caller's return address, or doing a restore
       // Tail calls are instrumented as return points, not call points.
       instruction nexti; 
       nexti.raw = owner->get_instruction(adr+4);

       if (nexti.rest.op == 2 
	   && ((nexti.rest.op3 == ORop3 && nexti.rest.rd == 15)
	      || nexti.rest.op3 == RESTOREop3)) {

	    // Undoing the tail-call optimazation when the function
	    // is relocated. Here is an example:
	    //   before:          --->             after
	    // ---------------------------------------------------
	    //   call  %g1                        restore    
	    //   restore                          st  %i0, [ %fp + 0x44 ]
	    //                                    mov %o7 %i0
	    //                                    call %g1 
	    //                                    nop
	    //                                    mov %i0,%o7
	    //                                    st  [ %fp + 0x44 ], %i0
	    //         			    retl
            //                                    nop
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

	    genSimpleInsn(&newInstr[i++], RESTOREop3, 0, 0, 0);
	    generateStore(&newInstr[i++], 24, REG_FP, 0x44); 
	    genImmInsn(&newInstr[i++], ORop3, 15, 0, 24); 
	    newInstr[i++].raw = owner->get_instruction(adr);
	    generateNOOP(&newInstr[i++]);
	    genImmInsn(&newInstr[i++], ORop3, 24, 0, 15);
	    generateLoad(&newInstr[i++], REG_FP, 0x44, 24);  	    
	    generateJmplInsn(&newInstr[i++], 15, 8 ,0);
	    newAdr += 28;
	    generateNOOP(&newInstr[i]);
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
       } else {

	   // otherwise, this is a call instruction to a location
           // outside the function
	   bool err;
           // relocateInstruction(&newInstr[i],adr+baseAddress,newAdr,proc);
	   adr = newCallPoint(newAdr, newInstr[i], owner, err,
				   callsId, adr,reloc_info,location);
           if (err) return false;
       }
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
