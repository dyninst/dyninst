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

// $Id: inst-sparc-solaris.C,v 1.93 2001/09/07 21:15:08 tikir Exp $

#include "dyninstAPI/src/inst-sparc.h"
#include "dyninstAPI/src/instPoint.h"
#include "common/h/debugOstream.h"

// Needed for function relocation
#include "dyninstAPI/src/func-reloc.h"

#include <sys/utsname.h>
#include <stdlib.h>

extern bool relocateFunction(process *proc, instPoint *&location);
extern bool branchInsideRange(instruction insn, Address branchAddress, 
                              Address firstAddress, Address lastAddress); 
extern instPoint* find_overlap(vector<instPoint*> v, Address targetAddress);
extern void sorted_ips_vector(vector<instPoint*>&fill_in);

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// static unsigned pfdp_to_pfdp_hash(pd_Function * const &f) {
//     pd_Function *pdf = f;
//     unsigned l = (unsigned)pdf;
//     return addrHash4(l); 
// }

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

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
: insnAddr(adr), addr(adr), originalInstruction(instr), 
  inDelaySlot(false), isDelayed(false),
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
      } else if(ipType == otherPoint) {
	  delaySlotInsn.raw = owner->get_instruction(addr+4);
	  size += 2*sizeof(instruction);
      } else {
	  assert(ipType == callSite);
	  // Usually, a function without a stack frame won't have any call sites
	  //cerr << "inst-sparc-solaris.C WARNING: found a leaf fn (no stack frame)" 
	  //     << "which makes a function call : " << func->prettyName()
	  //     << " at address " << adr << endl;
	  //abort();
	  // Actually - that is incorrect.  It confuses a leaf function
	  //  (one without a stack frame of its own) with a function which
	  //  does not make any calls).  It is possible for a function without
	  //  a stack frae to make calls in the case of e.g. tail-call 
	  //  optimization (in this case, the function could end with
	  //  e.g. jmp, nop)....
	  delaySlotInsn.raw = owner->get_instruction(addr+4);
	  size += 2*sizeof(instruction);

	  aggregateInsn.raw = owner->get_instruction(addr+8);
	  if (!IS_VALID_INSN(aggregateInsn) && aggregateInsn.raw != 0) {
	      callAggregate = true;
	      size += 1*sizeof(instruction);
	  }
      }
  }

  // return the address in the code segment after this instruction
  // sequence. (there's a -1 here because one will be added up later in
  // the function findInstPoints)  
  adr = addr + (size - 1*sizeof(instruction));
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// return the instruction after originalInstruction....
const instruction instPoint::insnAfterPoint() const {
    if (this->hasNoStackFrame()) {
      switch (ipType) {
      case functionEntry: 
	return otherInstruction;
	break;
      case callSite:
	return delaySlotInsn;
	break;
      case functionExit:
	return otherInstruction;
	break;
      default:
	assert(false);
      }
    } else {
      switch (ipType) {
      case functionEntry:
	return delaySlotInsn;
	break;
      case callSite:
	return delaySlotInsn;
	break;
      case functionExit:
	return delaySlotInsn;
      default:
	assert(false);
      }
    }

    // should never be reached....
    assert(false);
    // prevent warning about lack of return value....
    return delaySlotInsn;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

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

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

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
      pd_Function *pdf = (file_->exec())->findFuncByAddr(loc_addr);
      if (pdf) {
	p->callee = pdf;
	non_lib.push_back(p);
	//cerr << "  pdf (called func?) non-NULL = " << *pdf;
      } else if(!pdf){
	   //cerr << "  pdf (called func) NULL" << endl;
	   // if this is a call outside the fuction, keep it
	   if((loc_addr < getAddress(0))||(loc_addr > (getAddress(0)+size()))){
	        //cerr << "   apparent call outside function, adding p to non_lib"
	        //     << endl;
	        p->callIndirect = true;
                p->callee = NULL; 
                non_lib.push_back(p);
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
      non_lib.push_back(p);
    }
  }
  calls = non_lib;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

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
Address pd_Function::newCallPoint(Address &adr, const instruction instr,
				  const image * /*owner*/, bool &err, 
				  unsigned &callId, Address &/*oldAddr*/,
				  relocatedFuncInfo *reloc_info,
				  instPoint *&point,
				  const instPoint *&location)
{
    Address ret=adr;
    //instPoint *point;
 
#ifdef DEBUG_CALL_POINTS
    cerr << "pd_Function::newCallPoint called " << endl;
    cerr << " this " << *this << endl;
    cerr << " adr = " << adr << endl;
    cerr << " isTrap = " << isTrap << endl;
    cerr << " reloc_info = " << reloc_info << endl;
#endif

    err = true;

#ifdef notdefined     
    if (isTrap) {
        point = new instPoint(this, instr, owner, adr, false, callSite, oldAddr);
    } else {
        point = new instPoint(this, instr, owner, adr, false, callSite);
    }
    //point = new instPoint(this, instr, owner, adr, false, callSite);
#endif

    if (!isInsnType(instr, CALLmask, CALLmatch)) {
      point->callIndirect = true;
      point->callee = NULL;
    } else{
      point->callIndirect = false;
    }

    if (isTrap) {
	if (!reloc_info) {
	    calls.push_back(point);
	    calls[callId] -> instId = callId; callId++;
	} else {
	    // calls to a location within the function are not
	    // kept in the calls vector

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
	    //  might not have been previously seen....
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
	}
	else {
	    point->relocated_ = true;
	    reloc_info->addFuncCall(point);
	}
    }
    err = false;
    return ret;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
 * Given an instruction, relocate it to a new address, patching up
 *   any relative addressing that is present.
 * 
 */
void relocateInstruction(instruction*& insn, 
                        Address origAddr, Address& targetAddr, process *proc)
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
	    int ret = inferiorMalloc(proc,3*sizeof(instruction), textHeap,targetAddr);
	    assert(ret);
	    u_int old_offset = insn->branch.disp22 << 2;
	    insn->branch.disp22  = (ret - targetAddr)>>2;
	    instruction insnPlus[3];
	    genImmInsn(insnPlus, SAVEop3, REG_SPTR, -112, REG_SPTR);
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

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void generate_base_tramp_recursive_guard_code( process & p,
					       instruction * code,
					       Address base_addr,
					       NonRecursiveTrampTemplate & templ )
{
  /* prepare guard flag memory, if needed */
  Address guard_flag_address = p.getTrampGuardFlagAddr();
  if( guard_flag_address == 0 )
    {
	int initial_value = 1;
	guard_flag_address = inferiorMalloc( & p, sizeof( int ), dataHeap );
	/* initialize the new value */
	p.writeDataSpace( ( void * )guard_flag_address, sizeof( int ), & initial_value );

	p.setTrampGuardFlagAddr( guard_flag_address );
    }

  instruction * curr_instr;
  Address curr_addr;

  /* fill the 'guard on' pre-instruction instrumentation */
  curr_instr = code + templ.guardOnPre_beginOffset / sizeof( instruction );
  curr_addr = base_addr + templ.guardOnPre_beginOffset;
  generateSetHi( curr_instr, guard_flag_address, REG_L(0) );
  curr_instr++; curr_addr += sizeof( instruction );
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
  genImmInsn( curr_instr, ADDop3, REG_G(0), 1, REG_L(1) );
  curr_instr++; curr_addr += sizeof( instruction );
  generateStore( curr_instr, REG_L(1), REG_L(0), LOW10( guard_flag_address ) );

  /* fill the 'guard on' post-instruction instrumentation */
  curr_instr = code + templ.guardOnPost_beginOffset / sizeof( instruction );
  curr_addr = base_addr + templ.guardOnPost_beginOffset;
  generateSetHi( curr_instr, guard_flag_address, REG_L(0) );
  curr_instr++; curr_addr += sizeof( instruction );
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
  genImmInsn( curr_instr, ADDop3, REG_G(0), 1, REG_L(1) );
  curr_instr++; curr_addr += sizeof( instruction );
  generateStore( curr_instr, REG_L(1), REG_L(0), LOW10( guard_flag_address ) );
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
				  bool trampRecursiveDesired = false )
{
  trampTemplate*  current_template = &nonRecursiveBaseTemplate;

  if(location->ipType == otherPoint)
        current_template = &nonRecursiveConservativeBaseTemplate;

  if( trampRecursiveDesired )
    {
      current_template = &baseTemplate;

      if(location->ipType == otherPoint)
                current_template = &conservativeBaseTemplate;
    }

    Address ipAddr = 0;
    proc->getBaseAddress( location->image_ptr, ipAddr );
    ipAddr += location->addr;

    Address baseAddr = inferiorMalloc( proc, current_template->size, textHeap, ipAddr );
    assert( baseAddr );

    /* very conservative installation as o7 can be live at 
      this arbitrary inst point */

    if((location->ipType == otherPoint) &&
       location->func && location->func->is_o7_live() &&
       !in1BranchInsnRange(ipAddr, baseAddr))
    {
	vector<addrVecType> pointsToCheck;
	inferiorFree(proc,baseAddr,pointsToCheck);
	return NULL;
    }

    instruction * code = new instruction[ current_template->size ];
    assert( code );

    memcpy( ( char * )code,
	    ( char * )current_template->trampTemp,
	    current_template->size );

    instruction * temp;
    Address currAddr;
    for (temp = code, currAddr = baseAddr; 
	(currAddr - baseAddr) < (unsigned) current_template->size;
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
		    genImmInsn(temp, SAVEop3, REG_SPTR, -112, REG_SPTR); 
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
 		       genImmInsn(temp, SAVEop3, REG_SPTR, -112, REG_SPTR); 

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
 		    genImmInsn(temp, SAVEop3, REG_SPTR, -112, REG_SPTR); 
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
		
		genImmInsn(temp+1, SAVEop3, REG_SPTR, -112, REG_SPTR);
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
	    } else if(location->ipType == otherPoint){
		/** to save the value of live o7 register we save and call*/
		genImmInsn(temp, SAVEop3, REG_SPTR, -120, REG_SPTR);
		generateCallInsn(temp+1, currAddr+sizeof(instruction), 
				(baseAddress + location->addr)+location->size);
		genImmInsn(temp+2, RESTOREop3, 0, 0, 0);
	    } else {
		generateCallInsn(temp, currAddr, 
				(baseAddress + location->addr)+location->size);
	    }
        } else if (temp->raw == SKIP_PRE_INSN) {
	    unsigned offset;
	    offset = baseAddr+current_template->updateCostOffset-currAddr;
	    generateBranchInsn(temp,offset);
        } else if (temp->raw == SKIP_POST_INSN) {

	    unsigned offset;
	    offset = baseAddr+current_template->returnInsOffset-currAddr;
	    generateBranchInsn(temp,offset);

        } else if (temp->raw == UPDATE_COST_INSN) {	    
	    current_template->costAddr = currAddr;
	    generateNOOP(temp);
	} else if ((temp->raw == LOCAL_PRE_BRANCH) ||
                   (temp->raw == GLOBAL_PRE_BRANCH) ||
                   (temp->raw == LOCAL_POST_BRANCH) ||
		   (temp->raw == GLOBAL_POST_BRANCH)) {
#if defined(MT_THREAD)
            if ((temp->raw == LOCAL_PRE_BRANCH) ||
                (temp->raw == LOCAL_POST_BRANCH)) 
	    {
	      temp -= NUM_INSN_MT_PREAMBLE;
	      Address numIns=0;
	      generateMTpreamble((char *)temp, numIns, proc);
	      temp += NUM_INSN_MT_PREAMBLE;
            }
#endif
	    /* fill with no-op */
	    generateNOOP(temp);
	}
	else if( temp->raw == RECURSIVE_GUARD_ON_PRE_INSN )
	  {
	    generateNOOP( temp );
	  }
	else if( temp->raw == RECURSIVE_GUARD_OFF_PRE_INSN )
	  {
	    generateNOOP( temp );
	  }
	else if( temp->raw == RECURSIVE_GUARD_ON_POST_INSN )
	  {
	    generateNOOP( temp );
	  }
	else if( temp->raw == RECURSIVE_GUARD_OFF_POST_INSN )
	  {
	    generateNOOP( temp );
	  }
    }

  if( ! trampRecursiveDesired )
    {
      generate_base_tramp_recursive_guard_code( * proc,
						code,
						baseAddr,
						( NonRecursiveTrampTemplate & )*current_template );
    }

    // TODO cast
    proc->writeDataSpace( ( caddr_t )baseAddr,
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
    baseInst->baseAddr = baseAddr;

    return baseInst;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
 * Install the base Tramp for the function relocated.
 * (it means the base tramp that don't need to bother with long jump and
 *  is the one we used before for all the functions(since there's no
 *  long jumps)
 *  for system calls
 */ 
trampTemplate *installBaseTrampSpecial(const instPoint *&location,
				       process *proc, bool &deferred,
				       bool trampRecursiveDesired = false)
{
  trampTemplate* current_template = &nonRecursiveBaseTemplate;

  if(location->ipType == otherPoint)
        current_template = &nonRecursiveConservativeBaseTemplate;

  if( trampRecursiveDesired )
    {
      current_template = &baseTemplate;

      if(location->ipType == otherPoint)
                current_template = &conservativeBaseTemplate;
    }

  Address currAddr;
  instruction *code;
  instruction *temp;

  bool relocated;

  if(!(location->func->isInstalled(proc))) {
    relocated = location->func->relocateFunction(proc, const_cast<instPoint *>(location), deferred);

    // Unable to relocate function
    if (relocated == false) {
      return NULL;
    }
  }
  else if(!location->relocated_){
    // need to find new instPoint for location...it has the pre-relocated
    // address of the instPoint
    location->func->modifyInstPoint(location,proc);
  }

  code = new instruction[current_template->size];
  memcpy((char *) code, (char*) current_template->trampTemp, current_template->size);

  Address baseAddr = inferiorMalloc(proc, current_template->size, textHeap, location->addr);
  assert(baseAddr);

  if((location->ipType == otherPoint) &&
     location->func && location->func->is_o7_live() &&
     !in1BranchInsnRange(location->addr, baseAddr))
  {
      vector<addrVecType> pointsToCheck;
      inferiorFree(proc,baseAddr,pointsToCheck);
      return NULL;
  }

  for (temp = code, currAddr = baseAddr; 
       (currAddr - baseAddr) < (unsigned) current_template->size;
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
          offset = baseAddr+current_template->updateCostOffset-currAddr;
          generateBranchInsn(temp,offset);
        } else if (temp->raw == SKIP_POST_INSN) {
          unsigned offset;
          offset = baseAddr+current_template->returnInsOffset-currAddr;
          generateBranchInsn(temp,offset);
        } else if (temp->raw == UPDATE_COST_INSN) {	    

	    current_template->costAddr = currAddr;
	    generateNOOP(temp);
	} else if ((temp->raw == LOCAL_PRE_BRANCH) ||
                   (temp->raw == GLOBAL_PRE_BRANCH) ||
                   (temp->raw == LOCAL_POST_BRANCH) ||
		   (temp->raw == GLOBAL_POST_BRANCH)) {
#if defined(MT_THREAD)
            if ((temp->raw == LOCAL_PRE_BRANCH) ||
                (temp->raw == LOCAL_POST_BRANCH)) 
	    {
	      temp -= NUM_INSN_MT_PREAMBLE;
	      Address numIns=0;
	      generateMTpreamble((char *)temp, numIns, proc);
	      temp += NUM_INSN_MT_PREAMBLE;
            }
#endif
            /* fill with no-op */
            generateNOOP(temp);
        }
	else if( temp->raw == RECURSIVE_GUARD_ON_PRE_INSN )
	  {
	    generateNOOP( temp );
	  }
	else if( temp->raw == RECURSIVE_GUARD_OFF_PRE_INSN )
	  {
	    generateNOOP( temp );
	  }
	else if( temp->raw == RECURSIVE_GUARD_ON_POST_INSN )
	  {
	    generateNOOP( temp );
	  }
	else if( temp->raw == RECURSIVE_GUARD_OFF_POST_INSN )
	  {
	    generateNOOP( temp );
	  }
  }

  if( ! trampRecursiveDesired )
    {
      generate_base_tramp_recursive_guard_code( * proc,
						code,
						baseAddr,
						( NonRecursiveTrampTemplate & )*current_template );
    }

    // TODO cast
    proc->writeDataSpace((caddr_t)baseAddr, current_template->size,(caddr_t) code);

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
    baseInst->baseAddr = baseAddr;

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
    Address adr = location->addr;
    retInstance = NULL;

    const instPoint *&cLocation = const_cast<const instPoint *&>(location);
 
    trampTemplate *ret;
    if (proc->baseMap.find(cLocation, ret)) // writes to ret if found
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
       // vector<instruction> extra_instrs;  // not any more

       ret = installBaseTrampSpecial(cLocation, proc, deferred, 
                                     trampRecursionDesired);
       if(!ret) return NULL;

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
       // yet be installed in its original location, then genterate either
       //   BA,A
       // or
       //   SAVE;             CALL;         RESTORE.
       // so that it would jump to the start of the relocated function
       // which is in heap.
       if (!location->func->isInstalled(proc)){
          location->func->setInstalled(proc);

          Address adr = location-> func -> getAddress(0);
          instruction *insn;
          unsigned branchSize ;
          if (in1BranchInsnRange(adr+baseAddress, location->func->getAddress(proc))) {
            branchSize = 1 ;
            insn = new instruction[branchSize];
            generateBranchInsn(insn,(int)(location->func->getAddress(proc)-(adr+baseAddress)));
          } else {
            branchSize = 3 ;
            insn = new instruction[branchSize];
            genImmInsn(insn, SAVEop3, REG_SPTR, -112, REG_SPTR);
            generateCallInsn(insn+1, adr+baseAddress+4, location->func->getAddress(proc));
            genSimpleInsn(insn+2, RESTOREop3, 0, 0, 0);
          }

	  // set unknown the number of instructions to be overwritten 
          retInstance = new returnInstance(0/*branchSize*/, (instructUnion *)insn,
                                           branchSize*sizeof(instruction),
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

       ret = installBaseTramp(location, proc, trampRecursionDesired);
       if(!ret) return NULL;
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
	     retInstance = new returnInstance(1, (instructUnion *)insn,
					      sizeof(instruction), adr, 
					      sizeof(instruction));
	  } else if (need_to_add) {
	     // generate  original; call; add $o7 imm4 
	     instruction *insn = new instruction[2];
	     generateCallInsn(insn, adr+4, (int) ret->baseAddr);
	     genImmInsn(insn+1,ADDop3,REG_O(7),4,REG_O(7));
	     retInstance = new returnInstance(2, (instructUnion *)insn,
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
	             retInstance = new returnInstance(2, (instructUnion *)insn, 
					     2*sizeof(instruction), adr+4, 
					     2*sizeof(instruction));

                     already_done = true;
                 }
             }

             if(!already_done) {
	         instruction *insn = new instruction[3];
	         genImmInsn(insn, SAVEop3, REG_SPTR, -112, REG_SPTR);
	         generateCallInsn(insn+1, adr+4, (int) ret->baseAddr);
	         generateNOOP(insn+2);
	         retInstance = new returnInstance(3, (instructUnion *)insn, 
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
	      retInstance = new returnInstance(2, (instructUnion *)insn, 
					      2*sizeof(instruction), adr, 
					      2*sizeof(instruction));
	      assert(retInstance);
	    } else {
	      instruction *insn = new instruction;
	      if (location -> ipType == functionEntry) {
	          generateBranchInsn(insn, (int)(ret->baseAddr-adr+sizeof(instruction))); 
		  retInstance = new returnInstance(1, (instructUnion *)insn,
						   sizeof(instruction), 
						   adr - sizeof(instruction), 
						   sizeof(instruction));
	      } else {
	          generateBranchInsn(insn,(int)(ret->baseAddr-adr));
		  retInstance = new returnInstance(1, (instructUnion *)insn,
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
	     genImmInsn(insn+2,ADDop3,REG_O(7),4,REG_O(7));
	     retInstance = new returnInstance(3, (instructUnion *)insn, 
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

	     retInstance = new returnInstance(2, (instructUnion *)insn, 
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

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
 * Install a single tramp.
 *
 */
void installTramp(instInstance *inst, char *code, int codeSize) 
{
    //the default base trampoline template is the regular base trampoline.
    //However if the location iptype is  randomPoint then we have to use
    //the conservatibve base trampoline which saves the condition codes

    trampTemplate* current_template = &baseTemplate;

    if(inst->location->ipType == otherPoint)
        current_template = &conservativeBaseTemplate;

    totalMiniTramps++;
    insnGenerated += codeSize/sizeof(int);
    
    // TODO cast
    (inst->proc)->writeDataSpace((caddr_t)inst->trampBase, codeSize, code);

    Address atAddr;
    if (inst->when == callPreInsn) {
	if (inst->baseInstance->prevInstru == false) {
	    atAddr = inst->baseInstance->baseAddr+current_template->skipPreInsOffset;
	    inst->baseInstance->cost += inst->baseInstance->prevBaseCost;
	    inst->baseInstance->prevInstru = true;
	    generateNoOp(inst->proc, atAddr);
	}
    } else {
	if (inst->baseInstance->postInstru == false) {
	    atAddr = inst->baseInstance->baseAddr+current_template->skipPostInsOffset; 
	    inst->baseInstance->cost += inst->baseInstance->postBaseCost;
	    inst->baseInstance->postInstru = true;
	    generateNoOp(inst->proc, atAddr);
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
		      const vector<AstNode *> &operands, 
		      const string &callee, process *proc,
		      bool noCost, const function_base *calleefunc)
{
        assert(op == callOp);
        Address addr;
	bool err;
	vector <Register> srcs;
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
		  addr = func->getAddress(0);
	     }
	}
	for (unsigned u = 0; u < operands.size(); u++)
	    srcs.push_back(operands[u]->generateCode(proc, rs, i, base, noCost, false));

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

Register emitR(opCode op, Register src1, Register /*src2*/, Register /*dest*/, 
              char *i, Address &base, bool /*noCost*/)
{
    //fprintf(stderr,"emitR(op=%d,src1=%d,src2=XX,dest=XX)\n",op,src1);

    instruction *insn = (instruction *) ((void*)&i[base]);

    switch(op) {
      case getParamOp: {
#if defined(SHM_SAMPLING) && defined(MT_THREAD)
        // saving CT/vector address on the stack
        generateStore(insn, REG_MT, REG_FPTR, -40);
        insn++;
#endif
	// first 8 parameters are in register bank I (24..31)
	genSimpleInsn(insn, RESTOREop3, 0, 0, 0);
	insn++;

	generateStore(insn, REG_I(src1), REG_SPTR, 68+4*src1); 
	insn++;
	      
	genImmInsn(insn, SAVEop3, REG_SPTR, -112, REG_SPTR);
	insn++;

	generateLoad(insn, REG_SPTR, 112+68+4*src1, REG_I(src1)); 
	insn++;

#if defined(SHM_SAMPLING) && defined(MT_THREAD)
        // restoring CT/vector address back in REG_MT
        generateLoad(insn, REG_FPTR, -40, REG_MT);
        insn++;
        base += 6*sizeof(instruction);
#else
	base += 4*sizeof(instruction);
#endif
	
	if (src1 <= 8) {
	    return(REG_I(src1));
	}
	abort();
      }
    case getSysParamOp: {
	if (src1 <= 8) {
	    return(REG_I(src1));
	}	
        abort();
      }
    case getRetValOp: {
	// return value is in register REG_I(0)==24
	genSimpleInsn(insn, RESTOREop3, 0, 0, 0);
	insn++;

	generateStore(insn, REG_I(0), REG_SPTR, 68); 
	insn++;
	      
	genImmInsn(insn, SAVEop3, REG_SPTR, -112, REG_SPTR);
	insn++;

	generateLoad(insn, REG_SPTR, 112+68, REG_I(0)); 
	insn++;

	base += 4*sizeof(instruction);

	return(REG_I(0));
      }
    case getSysRetValOp:
	return(REG_I(0));
    default:
        abort();        // unexpected op for this emit!
    }
}

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
              char *i, Address &base, bool /*noCost*/, int /* size */)
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
/****************************************************************************/

static inline bool isRestoreInsn(instruction i) {
    return (i.rest.op == 2 \
	       && ((i.rest.op3 == ORop3 && i.rest.rd == 15)
		       || i.rest.op3 == RESTOREop3));
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

static inline bool CallRestoreTC(instruction instr, instruction nexti) {
    return (isCallInsn(instr) && isRestoreInsn(nexti));
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
  isTrap = false;
  relocatable_ = false;
  bool canBeRelocated = true;

  // Initially assume function has no stack frame 
  noStackFrame = true;

  // variables for function parameters
  const instPoint *blah = 0;
  
  bool err;
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
      isTrap = true;
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
      isTrap = true;
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
        isTrap = true;
        relocatable_ = true;
      }
    }
  }


  /* FIND FUNCTION ENTRY */

  entry = firstAddress;
  for ( adr = firstAddress; adr < lastAddress; adr += 4) { 

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
    canBeRelocated = false;
  }


  // if the second instruction in a function that needs relocation is a call
  // instruction or a branch instruction, then we can't deal with this.
  // New: only a problem if the call is to a location outside the function, 
  // or is a jump to itself....

  // Grab second instruction
  Address addrSecondInstr = firstAddress + sizeof(instruction);
  instr.raw = owner->get_instruction(addrSecondInstr); 

  if ( isCallInsn(instr) ) {

    // target of call
    target = addrSecondInstr + (instr.call.disp30 << 2);

    // if call dest. is outside of function, assume real
    // call site.  Assuming cant deal with this case!!!!
    if ( !(target >= firstAddress && target <= lastAddress) ||
                                    (target == addrSecondInstr) ) {
      canBeRelocated = false;
    }

    // Branch instruction  
    if ( instr.branch.op == 0 && 
        (instr.branch.op2 == 2 || instr.branch.op2 == 6) ) {
      canBeRelocated = false;
    }
  }


  // Can't handle function
  if (canBeRelocated == false && isTrap == true) {
    return false;
  }


#ifdef BPATCH_LIBRARY
  if (BPatch::bpatch->hasForcedRelocation_NP()) {
    if (canBeRelocated == true) {
      isTrap = true;
      relocatable_ = true;
    }
  }
#endif


  /* CREATE ENTRY INSTPOINT */
  instr.raw = owner->get_instruction(entry);

  if (relocatable_ == true) {
    funcEntry_ = new instPoint(this, instr, owner, entry, true, 
                                          functionEntry, entry);
  } else {
      funcEntry_ = new instPoint(this, instr, owner, entry, true, 
                                                 functionEntry);
  }

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

      if(rs1.is_o7() || rs2.is_o7() ||
         (rd.is_o7() && 
	  ((instr.raw & 0xc1f80000) != 0x81c00000))) /*indirect call*/
              o7_live = true;
    }

    // check for return insn and as a side affect decide if we are at the
    //   end of the function.
    if (isReturnInsn(owner, adr, dummyParam, prettyName())) {
      // define the return point

      instPoint *point;
      if (relocatable_ == true) {
        point = new instPoint(this, instr, owner, adr, false, 
                                           functionExit, adr);
      } else {
          point = new instPoint(this, instr, owner, adr, false, 
					          functionExit);
      }

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
        if (relocatable_ == true) {
          point = new instPoint(this, instructions[i], owner, adr, 
                                     false, functionExit, adr);
	} else {
            point = new instPoint(this, instructions[i], owner, adr, 
                                            false, functionExit);
	}

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
          cerr << "WARN : function " << prettyName().string_of()
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

        if (isTrap) {
          point = new instPoint(this, instr, owner, adr, false, callSite, adr);
        } else {
            point = new instPoint(this, instr, owner, adr, false, callSite);
        }

        adr = newCallPoint(adr, instr, owner, err, callsId, adr, 0, point, blah);
        if (err) {
          return false;
	}

	disp = adr + sizeof(instruction);
        instPoint *point = new instPoint(this, instr, owner, disp, 
                                         false, functionExit, adr);
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

            if (isTrap) {
              point = new instPoint(this, instr, owner, adr, false, callSite, 
                                                                         adr);
            } else {
                point = new instPoint(this, instr, owner, adr, false, 
                                                            callSite);
            }
            adr = newCallPoint(adr, instr, owner, err, callsId, adr, 0, point,
                                                                         blah);

  	  } else {
 
              // get call target instruction   
              Address call_target = adr + (instr.call.disp30 << 2);
              instruction tmpInsn;
              tmpInsn.raw = owner->get_instruction( call_target );

              // check that call is not directly to a retl instruction,
              // and thus a real call
              if((tmpInsn.raw & 0xfffff000) != 0x81c3e000) {
                if (isTrap) {
                  point = new instPoint(this, instr, owner, adr, false, 
                                                         callSite, adr);
                } else {
                    point = new instPoint(this, instr, owner, adr, false, 
                                                                callSite);
                }
                adr = newCallPoint(adr, instr, owner, err, callsId, 
                                                      adr, 0, point, blah);
  	        if (err) {
                  return false;
		}
	      } 
	  }         
      }
    }

    else if (JmpNopTC(instr, nexti, adr, this)) {

      if (isTrap) {
        point = new instPoint(this, instr, owner, adr, false, callSite, adr);
      } else {
          point = new instPoint(this, instr, owner, adr, false, callSite);
      }

      adr = newCallPoint(adr, instr, owner, err, callsId, adr, 0, point, blah);
      if (err) {
        return false;
      }

      disp = adr + sizeof(instruction);
      instPoint *point = new instPoint(this, instr, owner, disp, false, 
                                                     functionExit, adr);
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
          if (relocatable_ == true) {
            point = new instPoint(this, instr, owner, adr, false, 
          	  			       functionExit, adr);
	  } else {
              point = new instPoint(this, instr, owner, adr, false, 
               			                      functionExit);
	  }

	  funcReturns.push_back(point);
	  funcReturns[retId] -> instId = retId; retId++;
	}
      }
    }
  }
     
  bool checkPoints = checkInstPoints(owner);

  if ( (checkPoints == false) || (!canBeRelocated && isTrap) ){
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
		    //cerr << "WARN : function " << prettyName().string_of()
		    //	 << " has branch target inside fn entry point, can't instrument" << endl;
		  //return false;

                  // function can be instrumented if we relocate it
                  isTrap = true; 
                  relocatable_ = true;
	    } }

	    for (u_int i = 0; i < funcReturns.size(); i++) {
		if ((target > funcReturns[i]->addr)&&
		    (target < (funcReturns[i]->addr + funcReturns[i]->size))) {
		    if ((adr < funcReturns[i]->addr)||
			(adr > (funcReturns[i]->addr + funcReturns[i]->size))){
		        //cerr << "WARN : function " << prettyName().string_of()
		        //  << " has branch target inside fn return point, "
		        //  << "can't instrument" << endl;
		      //return false;

                      // function can be instrumented if we relocate it
                      isTrap = true;
                      relocatable_ = true; 
		} }
	    }
	}
    }

    // if there is a retl instruction and we don't think this is a leaf
    // function then this is a way messed up function...well, at least we
    // we can't deal with this...the only example I can find is _cerror
    // and _cerror64 in libc.so.1
    if(retl_inst && !noStackFrame && !restore_inst){ 
        //cerr << "WARN : function " << prettyName().string_of()
        //     << " retl instruction in non-leaf function, can't instrument"
        //      << endl;
	return false;
    }

    // check that no instrumentation points could overlap
    Address func_entry = funcEntry_->addr + funcEntry_->size; 
    for (u_int i = 0; i < funcReturns.size(); i++) {
	if(func_entry >= funcReturns[i]->addr){
	  //return false;

          // function can be instrumented if we relocate it 
          isTrap = true;
          relocatable_ = true; 
        }
	if(i >= 1){ // check if return points overlap
	    Address prev_exit = funcReturns[i-1]->addr+funcReturns[i-1]->size;  
	    if(funcReturns[i]->addr < prev_exit) {
	        //cerr << "WARN : function " << prettyName().string_of()
	        //     << " overlapping instrumentation points, can't instrument"
	        //     << endl;
	      //return false;
              // function can be instrumented if we relocate it 
              isTrap = true; 
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
    instruction instr, nexti;
    TailCallOptimization *tail_call;
    // previously referred to calls[i] directly, but gdb seems to be having
    //  trouble with templated types with the new compiler - making debugging
    //  difficult - so, directly assign calls[i] to the_call so can use gdb
    //  to get at info about it....
    instPoint *the_call;

#ifdef DEBUG_PA_INST
    cerr << "pd_Function::PA_attachGeneralRewrites called" <<endl;
    cerr << " prettyName = " << prettyName() << endl;
#endif

    // Look at the 2nd instruction in function.  If its a call, then
    //  stick a single nop before it....
    // The comment in the old inst-sparc-solaris.C version describing the rationale
    // for thw change is :
    //   if the second instruction in the function is a call instruction
    //   then this cannot go in the delay slot of the branch to the
    //   base tramp, so add a noop between first and second instructions
    //   in the relocated function (check out write in libc.so.1 for
    //   and example of this):
    //
    //     save  %sp, -96, %sp             brach to base tramp
    //     call  0x73b70                   nop
    //                                     call 0x73b70
    //    Note that if this call insn is a call to address + 8, it will actually
    //     be replaced by a sequence which does NOT include a call, so don't need to
    //     worry about inserting the extra nop....
    if (isCallInsn(loadedCode[1]) && 
                !is_set_O7_call(loadedCode[1], codeSize, sizeof(instruction))) {
        // Insert one no-op after the first instruction
        // (RewriteFootprint will copy the instruction at offset 0, and then
        // place the no-op after that instruction)   
        InsertNops *nop = new InsertNops(this, 0, sizeof(instruction));
	p->AddAlteration(nop);
#ifdef DEBUG_PA_INST
	cerr << " added single NOP in 2nd instruction" << endl;
#endif
    }

    // Iterate over function instruction by instruction, looking for calls to
    //  address + 8....
    assert((codeSize % sizeof(instruction)) == 0);
    for(unsigned i=0;i<(codeSize/sizeof(instruction));i++) {
        //  want CALL %address, NOT CALL %register
        if (isTrueCallInsn(loadedCode[i])) {
	    // figure out destination of call....
	    if (is_set_O7_call(loadedCode[i], codeSize, i * sizeof(instruction))) {
	        SetO7 *seto7 = new SetO7(this, i * sizeof(instruction));
		p->AddAlteration(seto7);
#ifdef DEBUG_PA_INST
	        cerr << " detected call pattern designed to set 07 register at offset " 
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
    // There is an unfortunate dependence on the method for detecting tail-call
    //  optimizations in the code for detecting overlapping inst-points, below.
    //  If change this code, may need to update that code correspondingly....
    for(unsigned i=0;i<calls.size();i++) {
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
	    cerr << " detected call, restore tail-call optimization at offset " 
		 << offset << endl;
#endif

	}
	if (JmpNopTC(instr, nexti, the_call->iPgetAddress(), this)) {
	    tail_call = new JmpNopTailCallOptimization(this, offset, 
				   offset + 2 * sizeof(instruction));
	    p->AddAlteration(tail_call);

#ifdef DEBUG_PA_INST
	    cerr << " detected jmp, nop tail-call optimization at offset " 
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
    vector<instPoint*> foo;
    //foo += funcEntry_;
    //foo += funcReturns;
    //foo += calls;
    // define sort_inst_points_by_address as function with following
    //  interface:
    //  void             sort (int (*)(const void *, const void *))
    //   - takes 2 void *'s, typecasts to instPoint *'s, then does
    //   {-1, 0, 1} comparison based on ip->addr....
    // foo.sort(sort_inst_points_by_address);
    // qsort((void *) data_, sz_, sizeof(T), cmpfunc);
    //qsort(foo.data(), foo.size(), sizeof(instPoint*), sort_inst_points_by_address);
    
    sorted_ips_vector(foo);

    // should hopefully have inst points for fn sorted by address....
    // check for overlaps....
    for (unsigned i=0;i<foo.size()-1;i++) {
        instPoint *this_inst_point = foo[i];
        instPoint *next_inst_point = foo[i+1];
	// This is kind of a hack - strictly speaking, the peephole alteration 
	//  abstraction for relocating inst points should be applied to the set of
	//  inst points in the function after every independent set of alterations is
	//  applied.  This is nto done for performance reasons - rather all the inst
	//  points in the function are relocated at once - based on the alteration sets.
	// As such, the peephole alterations are NOT implemented so as to be strictly
	//  independent.  An example of this is the interaction of the code for attaching
	//  tail-call optimization PAs based on inst points (NOT function instructions),
	//  above.  Anyway, the net efffect is that by the time flow-of-control reaches 
	//  here, the tail-call optimization has (hopefully) been rewritten, but the 
	//  inst points pointing to it have not been updated.  As such, check here to
	//  make sure that the overalpping inst points aren't really part of a tail-call
	//  optimization.  This introduces some lack of locality of reference - sorry....
        
        if ((this_inst_point->ipType == callSite) && 
	          (next_inst_point->ipType == functionExit)) {
          instr = this_inst_point->insnAtPoint();
          nexti = this_inst_point->insnAfterPoint();
          if (CallRestoreTC(instr, nexti) || 
      	    JmpNopTC(instr, nexti, this_inst_point->iPgetAddress(), this)) {

             // This tail call optimization will be rewritten, eliminating the
             // overlap, so we don't have to worry here about rewriting this
             // as overlapping instPoints.
             // Also, I added the i++ because we don't have to bother looking 
             // for an overlap between the next_inst_point, and any instPoints 
             // that may follow it (even one that is located at the very next 
             // instruction). The reason for this is that when we relocate the 
             // function, we will call installBaseTrampSpecial to generate the
             // base tramp. The distance between the base tramp and the 
             // relocated function will then be within the range
             // of a branch insn, and only one instruction at the instPoint 
             // will be relocated to the baseTramp (since the restore or nop
             // will not also have a delay slot insn) which means that there 
             // will be no conflict with next_inst_point overlapping another
             // instPoint.  itai 
             i++;

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
            
            if (IS_DELAYED_INST(loadedCode[offset/sizeof(instruction)])) {
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
    vector<instPoint*> foo;
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

	//
	// ALRT ALRT - What happens if you have multiple barcnhes to the same spot?
	//  They will reuslt in multiple identical NOP expansions added here.  Need
	//  LocalAlterationSet to ignore multiple identical alterations (identical
	//  in type, location, and all fields) beyond first....
	//
	// branch target inside function and inside inst point footprint....
	//  if target is before actual inst point target instruction, add
	//  (target - 1st addres) nops, where 1st address is address of 1st
	//  instruction actually inside the footprint....
	if (target <= overlap->iPgetAddress()) {
	    InsertNops *nops = new InsertNops(this, 
                               (target - firstAddress) - sizeof(instruction), 
			       target - overlap->firstAddress());
	    p->AddAlteration(nops);

#ifdef DEBUG_PA_INST
	    cerr << " detected overlap between branch target and inst point : offset "
		 << target - firstAddress << " # bytes " 
		 << target - overlap->firstAddress() << endl;
#endif

	} else {
	    InsertNops *nops = new InsertNops(this, 
                                 (target - firstAddress) - sizeof(instruction),
				 overlap->followingAddress() - target);
	    p->AddAlteration(nops);

#ifdef DEBUG_PA_INST
	    cerr << " detected overlap between branch target and inst point : offset "
		 << (target - firstAddress) - sizeof(instruction) 
                 << " # bytes " 
		 << overlap->firstAddress() - target << endl;
#endif

	}
    }

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
    if (IS_DELAYED_INST(loadedCode[offset/sizeof(instruction)-1])) {
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
    originalOffset = ((ip->iPgetAddress() + imageBaseAddr) - mutatee); \
    newOffset = alteration_set.getInstPointShift(originalOffset + 1); \
    assert(((originalOffset + newOffset) % sizeof(instruction)) == 0); \
    arrayOffset = ((originalOffset + newOffset) / sizeof(instruction)); \
    tmp = newAdr + originalOffset + newOffset; \
    tmp2 = ip->iPgetAddress();

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void pd_Function::addArbitraryPoint(instPoint* location,
				    process* proc,
				    relocatedFuncInfo* reloc_info)
{

    if(!reloc_info->funcArbitraryPoints().size())
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
    point = new instPoint(this,newCode[arrayOffset],owner,
			  tmp,true,otherPoint,tmp2);
    point->relocated_ = true;
    point->originalInstruction = newCode[arrayOffset];
    point->delaySlotInsn = newCode[arrayOffset+1];

    reloc_info->addArbitraryPoint(point);

    delete[] oldInstructions;
}

// oldAdr corresponds to x86 variable mutatee

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
    bool err;
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
        //  figure out how far entry inst point is from beginning of function....
        CALC_OFFSETS(funcEntry_)
        point = new instPoint(this, newCode, arrayOffset, owner, 
	                         tmp, true, functionEntry, tmp2);
#ifdef DEBUG_PA_INST
        cerr << " added entry point at originalOffset " << originalOffset
	     << " newOffset " << newOffset << endl;
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
                                 tmp, false, functionExit, tmp2);
#ifdef DEBUG_PA_INST
        cerr << " added return point at originalOffset " << originalOffset
	     << " newOffset " << newOffset << endl;
#endif
        assert(point != NULL);

#ifdef notdefined
	// ALERT ALERT
	//  Setting point->delaySlotInsn necessary because instPoint
	//  constructor used above reads program address space (at address
	//  adr) to set point->delaySlotInsn (and aggregateInsn, 	
	//  - should that also be set here?).  That's really a design screw-up
	//  in the inst point constructor - which should take additional
	//  arguments....
        point-> originalInstruction = newCode[arrayOffset];
        point-> delaySlotInsn = newCode[arrayOffset+1];
#endif

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
                                                            callSite, tmp2);
        newCallPoint(tmp, newCode[arrayOffset], owner, err, tmpId, tmp2, 
                     reloc_info, point, 
                     const_cast<const instPoint *&> (location));
#ifdef DEBUG_PA_INST
        cerr << " added call site at originalOffset " << originalOffset
	     << " newOffset " << newOffset << endl;
#endif
	if (err) return false;
    }

    for(arbitraryId=0;arbitraryId<arbitraryPoints.size();arbitraryId++){

	CALC_OFFSETS(arbitraryPoints[arbitraryId]);

	point = new instPoint(this, newCode, arrayOffset, owner,
			            tmp, true, otherPoint, tmp2);

        assert(point != NULL);

#ifdef notdefined
	// ALERT ALERT
	//  Setting point->delaySlotInsn necessary because instPoint
	//  constructor used above reads program address space (at address
	//  adr) to set point->delaySlotInsn (and aggregateInsn,
	//  - should that also be set here?).  That's really a design screw-up
	//  in the inst point constructor - which should take additional
	//  arguments....
	point-> originalInstruction = newCode[arrayOffset];
	point-> delaySlotInsn = newCode[arrayOffset+1];
#endif

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
    cerr << " prettyName = " << prettyName().string_of() << endl;
    cerr << " size() = " << size() << endl;
#endif

    //
    //  SOME PRELIMINARY CHECKS....
    //
    // check that function has size > 0.... 
    if (size() == 0) {
        cerr << "WARN : attempting to relocate function " \
       	     << prettyName().string_of() << " with size 0, unable to instrument" \
             <<  endl;
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
    // if unable to allocate array, dump warn and return false.... 
    if (oldCode == NULL) {
        cerr << "WARN : unable to allocate array (" << size() << " bytes) to read in " \
             << "code for function" << prettyName().string_of() << " unable to instrument" \
             << endl;
	return false;
    }
    // read function code from image....
    if (!readFunctionCode(owner, oldCode)) {
        cerr << "WARN : unable to read code for function " 
	     << prettyName().string_of() << " from address space : unable to instrument" \
             << endl;
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
