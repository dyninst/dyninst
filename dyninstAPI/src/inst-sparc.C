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

// $Id: inst-sparc.C,v 1.122 2002/04/22 17:39:44 bernat Exp $

#include "dyninstAPI/src/inst-sparc.h"
#include "dyninstAPI/src/instPoint.h"

#include "dyninstAPI/src/FunctionExpansionRecord.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

static dictionary_hash<string, unsigned> funcFrequencyTable(string::hash);

trampTemplate baseTemplate;
NonRecursiveTrampTemplate nonRecursiveBaseTemplate;

//declaration of conservative base trampoline template

trampTemplate conservativeBaseTemplate;
NonRecursiveTrampTemplate nonRecursiveConservativeBaseTemplate;



registerSpace *regSpace;

#if defined(SHM_SAMPLING) && defined(MT_THREAD)
// registers 8 to 15: out registers 
// registers 16 to 22: local registers
Register deadList[] = {16, 17, 18, 19, 20, 21, 22 };
#else
Register deadList[] = {16, 17, 18, 19, 20, 21, 22, 23 };
#endif

int deadListSize = sizeof(deadList);

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// Constructor for the instPoint class. 
instPoint::instPoint(pd_Function *f, const image *owner, Address &adr, 
                     bool delayOK, instPointType pointType)
: insnAddr(adr), addr(adr), 
  firstPriorIsDCTI(false), 
  secondPriorIsDCTI(false),
  thirdPriorIsDCTI(false), 
  firstIsDCTI(false), 
  secondIsDCTI(false), 
  thirdIsDCTI(false),
  firstPriorIsAggregate(false),
  thirdIsAggregate(false), 
  fourthIsAggregate(false), 
  fifthIsAggregate(false),
  usesPriorInstructions(false),
  numPriorInstructions(0),
  callIndirect(false), callee(NULL), func(f), isBranchOut(false),
  ipType(pointType), image_ptr(owner),
  relocated_(false), needsLongJump(false)
{

  // If the base tramp is too far away from the function for a branch, 
  // we will need to relocate at least two instructions to the base tramp
  firstInstruction.raw   = owner->get_instruction(adr);

  // For function call sites 
  if (ipType == callSite) {

      // Grab the second instruction
      secondInstruction.raw  = owner->get_instruction(adr + 4);

      firstIsDCTI = true;

      // Only need to relocate two instructions as the destination
      // of the original call is changed to the base tramp, and a call
      // to the real target is generated in the base tramp
      size = 2 * sizeof(instruction);


      // we may need to move the instruction after the instruction in the
      // call's delay slot, to deal with the case where the return value
      // of the called function is a structure.
      thirdInstruction.raw = owner->get_instruction(adr + 8);

      if ( !IS_VALID_INSN(thirdInstruction) &&
            thirdInstruction.raw != 0 ) {

          thirdIsAggregate = true;
          size = 3 * sizeof(instruction);
      }


  // For arbitrary instPoints
  } else if (ipType == otherPoint) {

      // Grab the second instruction
      secondInstruction.raw  = owner->get_instruction(adr + 4);

      size = 2 * sizeof(instruction);

      if ( isDCTI(firstInstruction) ) {
          firstIsDCTI = true;
      }


      // Instruction sequence looks like this:
      //
      // adr:      insn
      // adr + 4:  insn

      // If the second instruction is a delayed, control transfer
      // instruction, we need to also move the instruction in its
      // delay slot (if it is relocated to the base tramp).
      if ( isDCTI(secondInstruction) ) {

          secondIsDCTI = true;

          // Will need to relocate a third instruction to the base trampoline
          // if the base trampoline is outside of the range of a branch
          thirdInstruction.raw   = owner->get_instruction(adr + 8);

          size = 3 * sizeof(instruction);

          // If the second instruction is a CALL instruction, we may 
          // need to move the instruction after the instruction in the 
          // delay slot, to deal with the case where the return value 
          // of the called function is a structure.
          if ( isCallInsn(secondInstruction) ) {

              fourthInstruction.raw = owner->get_instruction(adr + 12);

	      if ( !IS_VALID_INSN(fourthInstruction) && 
                    fourthInstruction.raw != 0 ) {

	          fourthIsAggregate = true;
	          size = 4 * sizeof(instruction);
	      }
	  }
      }

  // For function entry instPoints
  } else if (ipType == functionEntry) {

      // Grab the second instruction
      secondInstruction.raw  = owner->get_instruction(adr + 4);

      // Will need to relocate a third instruction to the base trampoline
      // if the base trampoline is outside of the range of a branch
      thirdInstruction.raw   = owner->get_instruction(adr + 8);

      size = 3 * sizeof(instruction);

      // Instruction sequence looks like this:
      //
      // adr:      insn
      // adr + 4:  insn
      // adr + 8:  insn


      // If the first instruction is a delayed, control transfer
      // instruction, we need to also move the instruction in its
      // delay slot (if it is relocated to the base tramp).
      if ( isDCTI(firstInstruction) ) {

          firstIsDCTI = true;

          // If the first instruction is a CALL instruction, we may 
          // need to move the instruction after the instruction in its 
          // delay slot, to deal with the case where the return value 
          // of the called function is a structure.
          if ( isCallInsn(firstInstruction) ) {

              if ( !IS_VALID_INSN(thirdInstruction) && 
                    thirdInstruction.raw != 0 ) {

  	          thirdIsAggregate = true;
	      }
	  }
      }


      // If the second instruction is a delayed, control transfer
      // instruction, we need to also move the instruction in its
      // delay slot (if it is relocated to the base tramp).
      if ( isDCTI(secondInstruction) ) {

          secondIsDCTI = true;

          // If the second instruction is a CALL instruction, we may 
          // need to move the instruction after the instruction in the 
          // delay slot, to deal with the case where the return value 
          // of the called function is a structure.
          if ( isCallInsn(secondInstruction) ) {

              fourthInstruction.raw = owner->get_instruction(adr + 12);

	      if ( !IS_VALID_INSN(fourthInstruction) && 
                    fourthInstruction.raw != 0 ) {

	          fourthIsAggregate = true;
	          size = 4 * sizeof(instruction);
	      }
	  }
      }


      // If the third instruction is a delayed, control transfer
      // instruction, we need to also move the instruction in its
      // delay slot (if it is relocated to the base tramp).
      if ( isDCTI(thirdInstruction) ) {

          thirdIsDCTI = true;

  	  fourthInstruction.raw = owner->get_instruction(adr + 12);

	  // If the third instruction is a CALL instruction, we may 
          // need to move the instruction after the instruction in the 
          // delay slot, to deal with the case where the return value 
          // of the called function is a structure.
          if ( isCallInsn(thirdInstruction) ) {

              fifthInstruction.raw = owner->get_instruction(adr + 16);

	      if ( !IS_VALID_INSN(fifthInstruction) && 
                    fifthInstruction.raw != 0 ) {

	          fifthIsAggregate = true;
		  size = 5 * sizeof(instruction);
	      }
	  }
      }


  // For function exit points
  } else {

      assert(ipType == functionExit);

      firstIsDCTI = true;

      // if the function has no stack frame
      if (this->hasNoStackFrame()) {

  	  // If the base trampoline is too far away to use a branch 
          // instruction, we will need to claim the instructions before
          // the exit instructions, to be able to transfer to the base
          // trampoline.
	  usesPriorInstructions = true;

	  // Grab the instruction just before the exit instructions  
          firstPriorInstruction.raw = owner->get_instruction(adr - 4);
          numPriorInstructions      = 1;
	  size                      = 3 * sizeof(instruction);


          // If the first Prior instruction is a DCTI, then this point is part
          // of a tail call optimization and we need to have firstInstruction
          // pointing to the DCTI, not the delay slot instruction.
	  if ( isDCTI(firstPriorInstruction) ) {

              firstInstruction.raw  = owner->get_instruction(adr - 4);
              secondInstruction.raw = owner->get_instruction(adr);
              numPriorInstructions  = 0;
	      size                  = 2 * sizeof(instruction);


	  } else {

              // Grab the second instruction
              secondInstruction.raw  = owner->get_instruction(adr + 4);


              // If the 'firstPriorInstruction' is in the delay slot of the 
              // instruction just before it, we have to copy both of those
              // instructions to the base tramp
	      if (owner->isValidAddress(adr - 8)) {

  	          // Grab the instruction before the 'firstPriorInstruction' 
	          secondPriorInstruction.raw = owner->get_instruction(adr - 8);

	          if ( isDCTI(secondPriorInstruction) && !delayOK ) {

		      secondPriorIsDCTI    = true;
                      numPriorInstructions = 2;
		      size                 = 4 * sizeof(instruction);

		  }
	      }


              // If the 'firstPriorInstruction' is the aggregate instruction
              // for a call, we need to copy the three prior instructions to
              // the base trampoline. 
	      if (owner->isValidAddress(adr - 12)) {

  	          // Grab the instruction just before secondPriorInstruction 
	          thirdPriorInstruction.raw = owner->get_instruction(adr - 12);

	          if ( isDCTI(thirdPriorInstruction) && !delayOK ) {

  		      // If the 'firstPriorInstruction' is an aggregate
                      // instruction for the 'thirdPriorInstruction', copy
		      // all three prior instructions to the base tramp
	              if ( !IS_VALID_INSN(firstPriorInstruction) && 
                            firstPriorInstruction.raw != 0 ) {

  		          thirdPriorIsDCTI      = true;
	                  firstPriorIsAggregate = true;
                          numPriorInstructions  = 3;
	                  size                  = 5 * sizeof(instruction);
		      }
		  }
	      }
	  }

      // Function has a stack frame.
      } else {

          // Grab the second instruction
          secondInstruction.raw  = owner->get_instruction(adr + 4);

	  // No need to save registers before branching to exit 
	  // instrumentation
	  size = 2 * sizeof(instruction);
      }
  }


  // return the address in the code segment after this instruction
  // sequence. (there's a -1 here because one will be added up later in
  // the function findInstPoints)  
  adr = addr + (size - sizeof(instruction));
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// constructor for instPoint class for functions that have been relocated
instPoint::instPoint(pd_Function *f, const instruction instr[], 
                     int arrayOffset, const image *owner, Address &adr, 
                     bool delayOK, instPointType pointType)
: insnAddr(adr), addr(adr), 
  firstPriorIsDCTI(false), 
  secondPriorIsDCTI(false),
  thirdPriorIsDCTI(false), 
  firstIsDCTI(false), 
  secondIsDCTI(false), 
  thirdIsDCTI(false),
  firstPriorIsAggregate(false),
  thirdIsAggregate(false), 
  fourthIsAggregate(false), 
  fifthIsAggregate(false),
  usesPriorInstructions(false),
  numPriorInstructions(0),
  callIndirect(false), callee(NULL), func(f), isBranchOut(false),
  ipType(pointType), image_ptr(owner),
  relocated_(true), needsLongJump(false)
{

  // If the base tramp is too far away from the function for a branch, 
  // we will need to relocate at least two instructions to the base tramp
  firstInstruction.raw   = instr[arrayOffset].raw;
  secondInstruction.raw  = instr[arrayOffset + 1].raw;


  // For function call sites 
  if (ipType == callSite) {

    //assert( isCallInsn(firstInstruction) );

      firstIsDCTI = true;

      // Only need to relocate two instructions as the destination
      // of the original call is changed to the base tramp, and a call
      // to the real target is generated in the base tramp
      size = 2 * sizeof(instruction);


      // we may need to move the instruction after the instruction in the
      // call's delay slot, to deal with the case where the return value
      // of the called function is a structure.
      thirdInstruction.raw = instr[arrayOffset + 2].raw;

      if ( !IS_VALID_INSN(thirdInstruction) &&
            thirdInstruction.raw != 0 ) {

          thirdIsAggregate = true;
          size = 3 * sizeof(instruction);
      }

  // For arbitrary instPoints
  } else if (ipType == otherPoint) {

      // Grab the second instruction
      secondInstruction.raw  = instr[arrayOffset + 1].raw;

      size = 2 * sizeof(instruction);

      if ( isDCTI(firstInstruction) ) {
          firstIsDCTI = true;
      }


      // Instruction sequence looks like this:
      //
      // adr:      insn
      // adr + 4:  insn

      // If the second instruction is a delayed, control transfer
      // instruction, we need to also move the instruction in its
      // delay slot (if it is relocated to the base tramp).
      if ( isDCTI(secondInstruction) ) {

          secondIsDCTI = true;

          // Will need to relocate a third instruction to the base trampoline
          // if the base trampoline is outside of the range of a branch
          thirdInstruction.raw = instr[arrayOffset + 2].raw;

          size = 3 * sizeof(instruction);

          // If the second instruction is a CALL instruction, we may 
          // need to move the instruction after the instruction in the 
          // delay slot, to deal with the case where the return value 
          // of the called function is a structure.
          if ( isCallInsn(secondInstruction) ) {

              fourthInstruction.raw = instr[arrayOffset + 3].raw;

	      if ( !IS_VALID_INSN(fourthInstruction) && 
                    fourthInstruction.raw != 0 ) {

	          fourthIsAggregate = true;
	          size = 4 * sizeof(instruction);
	      }
	  }
      }

  // For function entry instPoints
  } else if (ipType == functionEntry) {

      // Will to relocate a third instruction to the base trampoline
      // if the base trampoline is outside of the range of a branch
      thirdInstruction.raw = instr[arrayOffset + 2].raw;

      size = 3 * sizeof(instruction);

      // Instruction sequence looks like this:
      //
      // adr:      insn
      // adr + 4:  insn
      // adr + 8:  insn


      // If the first instruction is a delayed, control transfer
      // instruction, we need to also move the instruction in its
      // delay slot (if it is relocated to the base tramp).
      if ( isDCTI(firstInstruction) ) {

          firstIsDCTI = true;

          // If the first instruction is a CALL instruction, we may 
          // need to move the instruction after the instruction in its 
          // delay slot, to deal with the case where the return value 
          // of the called function is a structure.
          if ( isCallInsn(firstInstruction) ) {

              if ( !IS_VALID_INSN(thirdInstruction) && 
                    thirdInstruction.raw != 0 ) {

  	          thirdIsAggregate = true;
	      }
	  }
      }


      // If the second instruction is a delayed, control transfer
      // instruction, we need to also move the instruction in its
      // delay slot (if it is relocated to the base tramp).
      if ( isDCTI(secondInstruction) ) {

          secondIsDCTI = true;

          // If the second instruction is a CALL instruction, we may 
          // need to move the instruction after the instruction in the 
          // delay slot, to deal with the case where the return value 
          // of the called function is a structure.
          if ( isCallInsn(secondInstruction) ) {

              fourthInstruction.raw = instr[arrayOffset + 3].raw;

	      if ( !IS_VALID_INSN(fourthInstruction) && 
                    fourthInstruction.raw != 0 ) {

	          fourthIsAggregate = true;
	          size = 4 * sizeof(instruction);
	      }
	  }
      }


      // If the third instruction is a delayed, control transfer
      // instruction, we need to also move the instruction in its
      // delay slot (if it is relocated to the base tramp).
      if ( isDCTI(thirdInstruction) ) {

          thirdIsDCTI = true;

  	  fourthInstruction.raw = instr[arrayOffset + 3].raw;

	  // If the third instruction is a CALL instruction, we may 
          // need to move the instruction after the instruction in the 
          // delay slot, to deal with the case where the return value 
          // of the called function is a structure.
          if ( isCallInsn(thirdInstruction) ) {

              fifthInstruction.raw = instr[arrayOffset + 4].raw;

	      if ( !IS_VALID_INSN(fifthInstruction) && 
                    fifthInstruction.raw != 0 ) {

	          fifthIsAggregate = true;
		  size = 5 * sizeof(instruction);
	      }
	  }
      }

  // For function exit points
  } else {

      assert(ipType == functionExit);

      firstIsDCTI = true;

      // if the function has no stack frame
      if (this->hasNoStackFrame()) {

        // Nops were added to the end of the function, so there is no
	// need to use prior instructions 
        size = 3 * sizeof(instruction);

      // Function has a stack frame.
      } else {

	  // No need to save registers before branching to exit 
	  // instrumentation
	  size = 2 * sizeof(instruction);
      }
  }


  // return the address in the code segment after this instruction
  // sequence. (there's a -1 here because one will be added up later in
  // the function findInstPoints)  
  adr = addr + (size - sizeof(instruction));
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// Add the astNode opt to generate one instruction to get the 
// return value for the compiler optimazed case
void
AstNode::optRetVal(AstNode *opt) {

    if (oType == ReturnVal) {
        cout << "Optimazed Return." << endl;
        if (loperand == 0) {
            loperand = opt;
            return;
        } else if (opt == 0) {
            delete loperand;
            loperand = NULL;
            return; 
        }
    }
    if (loperand) loperand->optRetVal(opt);
    if (roperand) roperand->optRetVal(opt);
    for (unsigned i = 0; i < operands.size(); i++) 
        operands[i] -> optRetVal(opt);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

bool 
processOptimaRet(instPoint *location, AstNode *&ast) {

    // For optimazed return code
    if (location->ipType == functionExit) {

        if ((isInsnType(location->firstInstruction, RETmask, RETmatch)) ||
            (isInsnType(location->firstInstruction, RETLmask, RETLmatch))) {

            if (isInsnType(location->secondInstruction, RESTOREmask, 
                                                        RESTOREmatch) &&
		(location->secondInstruction.raw | 0xc1e82000) != 0xc1e82000) {

                /* cout << "Optimazed Retrun Value:  Addr " << hex << 
                    location->addr << " in "
                        << location -> func -> prettyName() << endl; */
                AstNode *opt = new AstNode(AstNode::Constant,
                                   (void *)location->secondInstruction.raw);
                ast->optRetVal(opt);
                return true;
            }
        }
    }

    return false;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

Register
emitOptReturn(instruction i, Register src, char *insn, Address &base, bool noCost) {
    
    unsigned instr = i.raw;

    cout << "Handling a special case for optimized return value." << endl;

    assert(((instr&0x3e000000)>>25) <= 16);

    if ((instr&0x02000)>>13)
        emitImm(plusOp, (instr&0x07c000)>>14, instr&0x01fff,
                ((instr&0x3e000000)>>25)+16, insn, base, noCost);
    else
        (void) emitV(plusOp, (instr&0x07c000)>>14, instr&0x01fff,
             ((instr&0x3e000000)>>25)+16, insn, base, noCost);
    
    return emitR(getSysRetValOp, 0, 0, src, insn, base, noCost);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

//
// initDefaultPointFrequencyTable - define the expected call frequency of
//    procedures.  Currently we just define several one shots with a
//    frequency of one, and provide a hook to read a file with more accurate
//    information.
//
void initDefaultPointFrequencyTable()
{
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
        printf("found freq.input file\n");
    }
    while (!feof(fp)) {
        fscanf(fp, "%s %f\n", name, &value);
        funcFrequencyTable[name] = (int) value;
        printf("adding %s %f\n", name, value);
    }
    fclose(fp);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
 * Get an estimate of the frequency for the passed instPoint.  
 *    This is not (always) the same as the function that contains the point.
 * 
 *  The function is selected as follows:
 *
 *  If the point is an entry or an exit return the function name.
 *  If the point is a call and the callee can be determined, return the called
 *     function.
 *  else return the funcation containing the point.
 *
 *  WARNING: This code contins arbitray values for func frequency (both user 
 *     and system).  This should be refined over time.
 *
 * Using 1000 calls sec to be one SD from the mean for most FPSPEC apps.
 *      -- jkh 6/24/94
 *
 */
float getPointFrequency(instPoint *point)
{

    pd_Function *func;

    if (point->callee)
        func = point->callee;
    else
        func = point->func;

    if (!funcFrequencyTable.defines(func->prettyName())) {
      // Changing this value from 250 to 100 because predictedCost was
      // too high - naim 07/18/96
      return(100); 
      
    } else {
      return (funcFrequencyTable[func->prettyName()]);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

//
// return cost in cycles of executing at this point.  This is the cost
//   of the base tramp if it is the first at this point or 0 otherwise.
//
int getPointCost(process *proc, const instPoint *point)
{
    if (proc->baseMap.defines(point)) {
        return(0);
    } else {
        // 70 cycles for base tramp (worst case)
        return(70);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void initATramp (trampTemplate *thisTemp, Address tramp,
		 bool isConservative=false)
{
    instruction *temp;

    if(!isConservative){

    	thisTemp->savePreInsOffset = 
			((Address)baseTramp_savePreInsn - tramp);
    	thisTemp->restorePreInsOffset = 
			((Address)baseTramp_restorePreInsn - tramp);
    	thisTemp->savePostInsOffset = 
			((Address)baseTramp_savePostInsn - tramp);
    	thisTemp->restorePostInsOffset = 
			((Address)baseTramp_restorePostInsn - tramp);
    }else{
	thisTemp->savePreInsOffset =
		((Address)conservativeBaseTramp_savePreInsn - tramp);
	thisTemp->restorePreInsOffset =
		((Address)conservativeBaseTramp_restorePreInsn - tramp);
	thisTemp->savePostInsOffset =
		((Address)conservativeBaseTramp_savePostInsn - tramp);
	thisTemp->restorePostInsOffset =
		((Address)conservativeBaseTramp_restorePostInsn - tramp);
    }

    // TODO - are these offsets always positive?
    thisTemp->trampTemp = (void *) tramp;
    for (temp = (instruction*)tramp; temp->raw != END_TRAMP; temp++) {
        const Address offset = (Address)temp - tramp;
        switch (temp->raw) {
            case LOCAL_PRE_BRANCH:
                thisTemp->localPreOffset = offset;
                thisTemp->localPreReturnOffset = thisTemp->localPreOffset 
                                                + sizeof(temp->raw);
                break;
            case GLOBAL_PRE_BRANCH:
                thisTemp->globalPreOffset = offset;
                break;
            case LOCAL_POST_BRANCH:
                thisTemp->localPostOffset = offset;
                thisTemp->localPostReturnOffset = thisTemp->localPostOffset
                                                + sizeof(temp->raw);
                break;
            case GLOBAL_POST_BRANCH:
                thisTemp->globalPostOffset = offset;
                break;
            case SKIP_PRE_INSN:
                thisTemp->skipPreInsOffset = offset;
                break;
            case UPDATE_COST_INSN:
                thisTemp->updateCostOffset = offset;
                break;
            case SKIP_POST_INSN:
                thisTemp->skipPostInsOffset = offset;
                break;
            case RETURN_INSN:
                thisTemp->returnInsOffset = offset;
                break;
            case EMULATE_INSN:
                thisTemp->emulateInsOffset = offset;
                break;
	    case CONSERVATIVE_TRAMP_READ_CONDITION:
		if(isConservative)
			temp->raw = 0x83408000; /*read condition codes to g1*/
		break;
	    case CONSERVATIVE_TRAMP_WRITE_CONDITION:
		if(isConservative)
			temp->raw = 0x85806000; /*write condition codes fromg1*/
		break;
        }       
    }

    // Cost with the skip branches.
    thisTemp->cost = 14;  
    thisTemp->prevBaseCost = 20 +
      RECURSIVE_GUARD_ON_CODE_SIZE + RECURSIVE_GUARD_OFF_CODE_SIZE;
    thisTemp->postBaseCost = 22 +
      RECURSIVE_GUARD_ON_CODE_SIZE + RECURSIVE_GUARD_OFF_CODE_SIZE;
    thisTemp->prevInstru = thisTemp->postInstru = false;
    thisTemp->size = (int) temp - (int) tramp;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void initATramp(NonRecursiveTrampTemplate *thisTemp, Address tramp,
		bool isConservative=false)
{
  initATramp((trampTemplate *)thisTemp, tramp,isConservative);

  instruction *temp;

  for (temp = (instruction*)tramp; temp->raw != END_TRAMP; temp++) {
    const Address offset = (Address)temp - tramp;
    switch (temp->raw)
      {

      case RECURSIVE_GUARD_ON_PRE_INSN:
	thisTemp->guardOnPre_beginOffset = offset;
	thisTemp->guardOnPre_endOffset = thisTemp->guardOnPre_beginOffset
                + RECURSIVE_GUARD_ON_CODE_SIZE * INSN_SIZE;
	break;

      case RECURSIVE_GUARD_OFF_PRE_INSN:
	thisTemp->guardOffPre_beginOffset = offset;
	thisTemp->guardOffPre_endOffset = thisTemp->guardOffPre_beginOffset
                + RECURSIVE_GUARD_OFF_CODE_SIZE * INSN_SIZE;
	break;

      case RECURSIVE_GUARD_ON_POST_INSN:
	thisTemp->guardOnPost_beginOffset = offset;
	thisTemp->guardOnPost_endOffset = thisTemp->guardOnPost_beginOffset
                + RECURSIVE_GUARD_ON_CODE_SIZE * INSN_SIZE;
	break;

      case RECURSIVE_GUARD_OFF_POST_INSN:
	thisTemp->guardOffPost_beginOffset = offset;
	thisTemp->guardOffPost_endOffset = thisTemp->guardOffPost_beginOffset
                + RECURSIVE_GUARD_OFF_CODE_SIZE * INSN_SIZE;
	break;

      }
  }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void initTramps()
{
    static bool inited=false;

    if (inited) return;
    inited = true;

    initATramp(&baseTemplate, (Address) baseTramp);
    initATramp(&nonRecursiveBaseTemplate, (Address)baseTramp);


    initATramp(&conservativeBaseTemplate,
	       (Address)conservativeBaseTramp,true);
    initATramp(&nonRecursiveConservativeBaseTemplate,
	       (Address)conservativeBaseTramp,true);

    regSpace = new registerSpace(sizeof(deadList)/sizeof(Register), deadList,
                                         0, NULL);
    assert(regSpace);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#ifndef BPATCH_LIBRARY
#if defined(MT_THREAD)
//
// For multithreaded applications and shared memory sampling, this routine 
// will compute the address where the corresponding counter/timer vector for
// level 0 is (by default). In the mini-tramp, if the counter/timer is at a
// different level, we will add the corresponding offset - naim 4/18/97
//
// NUM_INSN_MT_PREAMBLE
void generateMTpreamble(char *insn, Address &base, process *proc)
{
  AstNode *t1 ;
  Address tableAddr;
  int value; 
  bool err;
  unsigned offset = 19*sizeof(instruction);
  Register src = Null_Register;

  vector<AstNode *> dummy;

  t1 = new AstNode("DYNINSTthreadPos", dummy);

  // t2=DYNINSTthreadPos()*sizeof(unsigned)
  value = sizeof(unsigned);
  AstNode* t4 = new AstNode(AstNode::Constant,(void *)value);
  AstNode* t2 = new AstNode(timesOp, t1, t4);
  removeAst(t1) ;
  removeAst(t4) ;

  // t3=DYNINSTthreadTable+t2
  //
  tableAddr = proc->findInternalAddress("DYNINSTthreadTable",true,err);
  assert(!err);
  AstNode* t5 = new AstNode(AstNode::Constant, (void *)tableAddr);
  AstNode* t3 = new AstNode(plusOp, t2, t5);
  removeAst(t2);
  removeAst(t5);

  AstNode* t7 = new AstNode(AstNode::Constant,(void *)-2);
  AstNode* t8 = new AstNode(eqOp, t1, t7);
  removeAst(t7);
  AstNode* t9 = new AstNode(AstNode::Constant, (void *)(offset));
  //goto offset
  AstNode* t10 = new AstNode(branchOp, t9);
  removeAst(t9);
  //t11 = "if(t1 ==-2) goto offset"
  AstNode* t11 = new AstNode(ifOp, t8, t10);
  removeAst(t8);
  removeAst(t10) ;

  //t6 = " if (t1 == -2) goto Offset; t3=DYNINSTthreadTable+t2"
  AstNode *t6= new AstNode(t11, t3);
  removeAst(t11);
  removeAst(t3);
  src = t6->generateCode(proc, regSpace, insn, base, false, true);
  removeAst(t6);
  (void) emitV(orOp, src, 0, REG_MT_BASE, insn, base, false);
  regSpace->freeRegister(src);
}
#endif

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// Fake the POS calculation

Address process::generateMTRPCCode(void *insnPtr, Address &base, 
				   unsigned int tid, unsigned pos)
{
  AstNode *t1 ;
  Address tableAddr;
  int value; 
  bool err;
  Register src = Null_Register;

  if (tid != -1)  {
    vector<AstNode *> param;
    param.push_back( new AstNode(AstNode::Constant,(void *)tid) );
    param.push_back( new AstNode(AstNode::Constant,(void *)pos) );
    t1 = new AstNode("DYNINSTthreadPosTID", param);
    for (unsigned i=0; i<param.size(); i++) 
      removeAst(param[i]) ;

    value = sizeof(unsigned);
    AstNode* t4 = new AstNode(AstNode::Constant,(void *)value);
    AstNode* t2 = new AstNode(timesOp, t1, t4);
    removeAst(t1) ;
    removeAst(t4) ;

    // t3=DYNINSTthreadTable[thr_self()]
    //
    tableAddr = findInternalAddress("DYNINSTthreadTable",true,err);
    assert(!err);
    AstNode* t5 = new AstNode(AstNode::Constant, (void *)tableAddr);
    AstNode* t3 = new AstNode(plusOp, t2, t5);
    removeAst(t2);
    removeAst(t5);

    AstNode* t7 = new AstNode(AstNode::Constant,(void *)-2);
    AstNode* t8 = new AstNode(eqOp, t1, t7);
    removeAst(t7);
    AstNode* t9 = new AstNode(AstNode::Constant, (void *)28);
    AstNode* t10 = new AstNode(branchOp, t9);
    removeAst(t9);
    AstNode* t11 = new AstNode(ifOp, t8, t10);
    removeAst(t8);
    removeAst(t10);

    AstNode *t6= new AstNode(t11, t3);
    removeAst(t11);
    removeAst(t3);
    src = t6->generateCode(this, regSpace, (char *)insnPtr, base, false, true);
    removeAst(t6);
    unsigned first_insn=base;
    (void) emitV(orOp, src, 0, REG_MT_BASE, (char *)insnPtr, base, false);
    regSpace->freeRegister(src);
  }
  return 0;
}
#endif

void generateMTSkipBranch(void *insnPtr, Address addr, Address offset)
{
  instruction *temp = (instruction *) (&((instruction *)insnPtr)[addr/sizeof(instruction)]);

  // Conditional branch (on equal).
  temp->branch.op = 0;
  temp->branch.cond = BEcond;
  temp->branch.op2 = BICCop2;
  temp->branch.anneal = false;
  temp->branch.disp22 = (offset - addr) >> 2;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void generateNoOp(process *proc, Address addr)
{
    instruction insn;

    /* fill with no-op */
    insn.raw = 0;
    insn.branch.op = 0;
    insn.branch.op2 = NOOPop2;

    proc->writeTextWord((caddr_t)addr, insn.raw);
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
 * change the insn at addr to be a branch to newAddr.
 *   Used to add multiple tramps to a point.
 */
void generateBranch(process *proc, Address fromAddr, Address newAddr)
{
    int disp;
    instruction insn;

    disp = newAddr-fromAddr;
    generateBranchInsn(&insn, disp);

    proc->writeTextWord((caddr_t)fromAddr, insn.raw);
}
void generateBranchOrCall(process *proc, Address fromAddr, Address newAddr)
{
    int disp;
    instruction insn;

    disp = newAddr-fromAddr;
    if (offsetWithinRangeOfBranchInsn(disp)){
    	generateBranchInsn(&insn, disp);
        proc->writeTextWord((caddr_t)fromAddr, insn.raw);
    }
    else{
	genImmInsn(&insn,SAVEop3,14,-112,14);
        proc->writeTextWord((caddr_t)fromAddr, insn.raw);

	fromAddr += sizeof(Address);
	generateCallInsn(&insn,fromAddr,newAddr);
        proc->writeTextWord((caddr_t)fromAddr, insn.raw);

	fromAddr += sizeof(Address);
	genSimpleInsn(&insn,RESTOREop3,0,0,0);
        proc->writeTextWord((caddr_t)fromAddr, insn.raw);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void generateCall(process *proc, Address fromAddr, Address newAddr)
{
    instruction insn; 
    generateCallInsn(&insn, fromAddr, newAddr);

    proc->writeTextWord((caddr_t)fromAddr, insn.raw);

}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void genImm(process *proc, Address fromAddr, int op, 
                        Register rs1, int immd, Register rd)
{
    instruction insn;
    genImmInsn(&insn, op, rs1, immd, rd);

    proc->writeTextWord((caddr_t)fromAddr, insn.raw);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
 *  change the target of the branch at fromAddr, to be newAddr.
 */
void changeBranch(process *proc, Address fromAddr, Address newAddr, 
                  instruction originalBranch) {
    int disp = newAddr-fromAddr;
    instruction insn;
    insn.raw = originalBranch.raw;
    insn.branch.disp22 = disp >> 2;
    proc->writeTextWord((caddr_t)fromAddr, insn.raw);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

int callsTrackedFuncP(instPoint *point)
{
    if (point->callIndirect) {
        return(true);
    } else {
        if (point->callee) {
            return(true);
        } else {
            return(false);
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/*
 * return the function asociated with a point.
 *
 *     If the point is a funcation call, and we know the function being called,
 *          then we use that.  Otherwise it is the function that contains the
 *          point.
 *  
 *   This is done to return a better idea of which function we are using.
 */
pd_Function *getFunction(instPoint *point)
{
    return(point->callee ? point->callee : point->func);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

bool process::emitInferiorRPCheader(void *insnPtr, Address &baseBytes, bool /*isFunclet*/) 
{
   instruction *insn = (instruction *)insnPtr;
   Address baseInstruc = baseBytes / sizeof(instruction);

   genImmInsn(&insn[baseInstruc++], SAVEop3, 14, -112, 14);

   baseBytes = baseInstruc * sizeof(instruction); // convert back
   return true;
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

bool process::emitInferiorRPCtrailer(void *insnPtr, Address &baseBytes,
                                     unsigned &breakOffset,
                                     bool stopForResult,
                                     unsigned &stopForResultOffset,
				     unsigned &justAfter_stopForResultOffset,
				     bool isFunclet)
{

   // Sequence: restore, trap, illegal

   instruction *insn = (instruction *)insnPtr;
   Address baseInstruc = baseBytes / sizeof(instruction);

   if (stopForResult) {
      // trap insn:
      genBreakpointTrap(&insn[baseInstruc]);
      stopForResultOffset = baseInstruc * sizeof(instruction);
      baseInstruc++;
      justAfter_stopForResultOffset = baseInstruc * sizeof(instruction);
   }


   if (isFunclet) { 
     //ret instruction
     genImmInsn(&insn[baseInstruc++], JMPLop3, REG_I(7), 0x08, REG_G(0)) ;
   }

   genSimpleInsn(&insn[baseInstruc++], RESTOREop3, 0, 0, 0);

   // Now that the inferior has executed the 'restore' instruction, the %in 
   // and %local registers have been restored.  We mustn't modify them after
   // this point!! (reminder: the %in and %local registers aren't saved and 
   // set with ptrace GETREGS/SETREGS call)


   // If non-threaded, or if threaded and not safe RPC
   if (!isFunclet) {
     // Trap instruction:
     genBreakpointTrap(&insn[baseInstruc]); // ta 1
     breakOffset = baseInstruc * sizeof(instruction);
     baseInstruc++;
     
     // And just to make sure that we don't continue from the trap:
     genUnimplementedInsn(&insn[baseInstruc++]); // UNIMP 0
   }

   baseBytes = baseInstruc * sizeof(instruction); // convert back

   return true; // success
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void emitImm(opCode op, Register src1, RegValue src2imm, Register dest, 
             char *i, Address &base, bool noCost)
{
        instruction *insn = (instruction *) ((void*)&i[base]);
        RegValue op3 = -1;
        int result = -1;
        switch (op) {
            // integer ops
            case plusOp:
                op3 = ADDop3;
                genImmInsn(insn, op3, src1, src2imm, dest);
                break;

            case minusOp:
                op3 = SUBop3;
                genImmInsn(insn, op3, src1, src2imm, dest);
                break;

            case timesOp:
                op3 = SMULop3;
                if (isPowerOf2(src2imm,result) && (result<32))
                  generateLShift(insn, src1, (Register)result, dest);           
                else 
                  genImmInsn(insn, op3, src1, src2imm, dest);
                break;

            case divOp:
                op3 = SDIVop3;
                if (isPowerOf2(src2imm,result) && (result<32))
                  generateRShift(insn, src1, (Register)result, dest);           
                else { // needs to set the Y register to zero first
                  // Set the Y register to zero: Zhichen
                  genImmInsn(insn, WRYop3, REG_G(0), 0, 0);
                  base += sizeof(instruction);
                  insn = (instruction *) ((void*)&i[base]);
                  genImmInsn(insn, op3, src1, src2imm, dest);
                }

                break;

            // Bool ops
            case orOp:
                op3 = ORop3;
                genImmInsn(insn, op3, src1, src2imm, dest);
                break;

            case andOp:
                op3 = ANDop3;
                genImmInsn(insn, op3, src1, src2imm, dest);
                break;

            // rel ops
            // For a particular condition (e.g. <=) we need to use the
            // the opposite in order to get the right value (e.g. for >=
            // we need BLTcond) - naim
            case eqOp:
                genImmRelOp(insn, BNEcond, src1, src2imm, dest, base);
                return;
                break;

            case neOp:
                genImmRelOp(insn, BEcond, src1, src2imm, dest, base);
                return;
                break;

            case lessOp:
                genImmRelOp(insn, BGEcond, src1, src2imm, dest, base);
                return;
                break;

            case leOp:
                genImmRelOp(insn, BGTcond, src1, src2imm, dest, base);
                return;
                break;

            case greaterOp:
                genImmRelOp(insn, BLEcond, src1, src2imm, dest, base);
                return;
                break;

            case geOp:
                genImmRelOp(insn, BLTcond, src1, src2imm, dest, base);
                return;
                break;

            default:
                Register dest2 = regSpace->allocateRegister(i, base, noCost);
                (void) emitV(loadConstOp, src2imm, dest2, dest2, i, base, noCost);
                (void) emitV(op, src1, dest2, dest, i, base, noCost);
                regSpace->freeRegister(dest2);
                return;
                break;
        }
        base += sizeof(instruction);
        return;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

//
// All values based on Cypress0 && Cypress1 implementations as documented in
//   SPARC v.8 manual p. 291
//
int getInsnCost(opCode op)
{
    /* XXX Need to add branchOp */
    if (op == loadConstOp) {
        return(1);
    } else if (op ==  loadOp) {
        // sethi + load single
        return(1+1);
    } else if (op ==  loadIndirOp) {
        return(1);
    } else if (op ==  storeOp) {
        // sethi + store single
        // return(1+3); 
        // for SS-5 ?
        return(1+2); 
    } else if (op ==  storeIndirOp) {
        return(2); 
    } else if (op ==  ifOp) {
        // subcc
        // be
        // nop
        return(1+1+1);
    } else if (op ==  callOp) {
        int count = 0;

        // mov src1, %o0
        count += 1;

        // mov src2, %o1
        count += 1;

        // clr i2
        count += 1;

        // clr i3
        count += 1;

        // sethi
        count += 1;

        // jmpl
        count += 1;

        // noop
        count += 1;

        return(count);
    } else if (op ==  updateCostOp) {
        // sethi %hi(obsCost), %l0
        // ld [%lo + %lo(obsCost)], %l1
        // add %l1, <cost>, %l1
        // st %l1, [%lo + %lo(obsCost)]
        return(1+2+1+3);
    } else if (op ==  trampPreamble) {
        return(0);
    } else if (op ==  trampTrailer) {
        // retl
        return(2);
    } else if (op == noOp) {
        // noop
        return(1);
    } else if (op == getParamOp) {
        return(0);
    } else {
        switch (op) {
            // rel ops
            case eqOp:
            case neOp:
            case lessOp:
            case leOp:
            case greaterOp:
            case geOp:
                // bne -- assume taken
                return(2);
                break;
            default:
                return(1);
                break;
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

bool isReturnInsn(instruction instr, Address addr, string name) {
    if (isInsnType(instr, RETmask, RETmatch) ||
        isInsnType(instr, RETLmask, RETLmatch)) {
        //  Why 8 or 12?
        //  According to the sparc arch manual (289), ret or retl are
        //   synthetic instructions for jmpl %i7+8, %g0 or jmpl %o7+8, %go.
        //  Apparently, the +8 is not really a hard limit though, as 
        //   sometimes some extra space is allocated after the jump 
        //   instruction for various reasons.
        //  1 possible reason is to include information on the size of
        //   returned structure (4 bytes).
        //  So, 8 or 12 here is a heuristic, but doesn't seem to 
        //   absolutely have to be true.
        //  -matt
        if ((instr.resti.simm13 != 8) && (instr.resti.simm13 != 12) 
                    && (instr.resti.simm13 != 16)) {
          sprintf(errorLine,"WARNING: unsupported return at address 0x%lx"
                        " in function %s - appears to be return to PC + %i", 
                  addr, name.string_of(), (int)instr.resti.simm13);
          showErrorCallback(55, errorLine);
        } else { 
          return true;
        }
    }
    return false;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

bool isReturnInsn(const image *owner, Address adr, bool &lastOne, string name)
{
    instruction instr;

    instr.raw = owner->get_instruction(adr);
    lastOne = false;

    return isReturnInsn(instr, adr, name);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

bool isBranchInsn(instruction instr) {
    if (instr.branch.op == 0 
                && (instr.branch.op2 == 2 || instr.branch.op2 == 6)) 
          return true;
    return false;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// modifyInstPoint: if the function associated with the process was 
// recently relocated, then the instPoint may have the old pre-relocated
// address (this can occur because we are getting instPoints in mdl routines 
// and passing these to routines that do the instrumentation, it would
// be better to let the routines that do the instrumenting find the points)
void pd_Function::modifyInstPoint(const instPoint *&location,process *proc)
{

    if(relocatable_ && !(location->relocated_)){
        for(u_int i=0; i < relocatedByProcess.size(); i++){
            if((relocatedByProcess[i])->getProcess() == proc){
                if(location->ipType == functionEntry){
                    const instPoint *new_entry = 
                                (relocatedByProcess[i])->funcEntry();
                    location = new_entry;
                } 
                else if(location->ipType == functionExit){
                    const vector<instPoint *> new_returns = 
                        (relocatedByProcess[i])->funcReturns(); 
                    if(funcReturns.size() != new_returns.size()){
                        printf("funcReturns = %d new_returns = %d\n",
                                funcReturns.size(),new_returns.size());
                        fflush(stdout);
                    }
                    assert(funcReturns.size() == new_returns.size());
                    for(u_int j=0; j < new_returns.size(); j++){
                        if(funcReturns[j] == location){
                            location = (new_returns[j]);
                            break;
                        }
                    }
                }
                else if(location->ipType == otherPoint) {
                    const vector<instPoint *> new_arbitrary = 
                        (relocatedByProcess[i])->funcArbitraryPoints(); 

                    assert(arbitraryPoints.size() == new_arbitrary.size());
                    for(u_int j=0; j < new_arbitrary.size(); j++){
                        if(arbitraryPoints[j] == location){
                            location = (new_arbitrary[j]);
                            break;
                        }
                    }
		}
		else {
                    const vector<instPoint *> new_calls = 
                                (relocatedByProcess[i])->funcCallSites(); 
                    assert(calls.size() == new_calls.size());
                    for(u_int j=0; j < new_calls.size(); j++){
                        if(calls[j] == location){
                            location = (new_calls[j]);
                            break;
                        }
                    }

                }
                break;
    } } }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// The exact semantics of the heap are processor specific.
//
// find all DYNINST symbols that are data symbols
//
bool process::heapIsOk(const vector<sym_data> &find_us) {
  Symbol sym;
  Address baseAddr;

  // find the main function
  // first look for main or _main
  if (!((mainFunction = findOneFunction("main")) 
        || (mainFunction = findOneFunction("_main")))) {
     string msg = "Cannot find main. Exiting.";
     statusLine(msg.string_of());
#if defined(BPATCH_LIBRARY)
     BPatch_reportError(BPatchWarning, 50, msg.string_of());
#else
     showErrorCallback(50, msg);
#endif
     return false;
  }

  for (unsigned i=0; i<find_us.size(); i++) {
    const string &str = find_us[i].name;
    if (!getSymbolInfo(str, sym, baseAddr)) {
      string str1 = string("_") + str.string_of();
      if (!getSymbolInfo(str1, sym, baseAddr) && find_us[i].must_find) {
        string msg;
        msg = string("Cannot find ") + str + string(". Exiting");
        statusLine(msg.string_of());
        showErrorCallback(50, msg);
        return false;
      }
    }
  }

//  string ghb = GLOBAL_HEAP_BASE;
//  if (!getSymbolInfo(ghb, sym, baseAddr)) {
//    ghb = U_GLOBAL_HEAP_BASE;
//    if (!linkedFile.get_symbol(ghb, sym)) {
//      string msg;
//      msg = string("Cannot find ") + ghb + string(". Exiting");
//      statusLine(msg.string_of());
//      showErrorCallback(50, msg);
//      return false;
//    }
//  }
//  Address instHeapEnd = sym.addr()+baseAddr;
//  addInternalSymbol(ghb, instHeapEnd);


#ifdef ndef
  /* Not needed with the new heap type system */

  string ihb = INFERIOR_HEAP_BASE;
  if (!getSymbolInfo(ihb, sym, baseAddr)) {
    ihb = UINFERIOR_HEAP_BASE;
    if (!getSymbolInfo(ihb, sym, baseAddr)) {
      string msg;
      msg = string("Cannot find ") + ihb + string(". Cannot use this application");
      statusLine(msg.string_of());
      showErrorCallback(50, msg);
      return false;
    }
  }

  Address curr = sym.addr()+baseAddr;
  // Check that we can patch up user code to jump to our base trampolines
  // (Perhaps this code is no longer needed for sparc platforms, since we use full
  // 32-bit jumps)
  const Address instHeapStart = curr;
  const Address instHeapEnd = instHeapStart + SYN_INST_BUF_SIZE - 1;

  if (instHeapEnd > getMaxBranch3Insn()) {
    logLine("*** FATAL ERROR: Program text + data too big for dyninst\n");
    sprintf(errorLine, "    heap starts at %x and ends at %x; maxbranch=%x\n",
            instHeapStart, instHeapEnd, getMaxBranch3Insn());
    logLine(errorLine);
    return false;
  }
#endif

  return true;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// Certain registers (i0-i7 on a SPARC) may be available to be read
// as an operand, but cannot be written.
bool registerSpace::readOnlyRegister(Register /*reg_number*/) {
// -- this code removed, since it seems incorrect
//if ((reg_number < REG_L(0)) || (reg_number > REG_L(7)))
//    return true;
//else
      return false;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

bool returnInstance::checkReturnInstance(const vector<Frame> &stackWalk, u_int &index) {
    // If false (unsafe) is returned, then 'index' is set to the first unsafe call stack
    // index.
  for (u_int i=0; i < stackWalk.size(); i++) {
    index = i;
    
    // Is the following check correct?  Shouldn't the ">" be changed to ">=",
    // and the "<=" be changed to "<" ??? --ari 6/11/97
    if (stackWalk[i].getPC() > addr_ && stackWalk[i].getPC() <= addr_+size_)
      return false;
  }
  
  return true;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void returnInstance::installReturnInstance(process *proc) {
    proc->writeTextSpace((caddr_t)addr_, instSeqSize, 
                         (caddr_t) instructionSeq); 
    installed = true;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void generateBreakPoint(instruction &insn) {
    insn.raw = BREAK_POINT_INSN;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void returnInstance::addToReturnWaitingList(Address pc, process *proc) {
    // if there already is a TRAP set at this pc for this process don't
    // generate a trap instruction again...you will get the wrong original
    // instruction if you do a readDataSpace
    bool found = false;
    instruction insn;
    for (u_int i=0; i < instWList.size(); i++) {
         if (instWList[i]->pc_ == pc && instWList[i]->which_proc == proc) {
             found = true;
             insn = instWList[i]->relocatedInstruction;
             break;
         }
    }
    if(!found) {
        instruction insnTrap;
        generateBreakPoint(insnTrap);
        proc->readDataSpace((caddr_t)pc, sizeof(insn), (char *)&insn, true);
        proc->writeTextSpace((caddr_t)pc, sizeof(insnTrap), (caddr_t)&insnTrap);
    }
    else {
    }

    instWaitingList *instW = new instWaitingList(instructionSeq,instSeqSize,
                                                 addr_,pc,insn,pc,proc);
    instWList.push_back(instW);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

bool doNotOverflow(int value)
{
  // we are assuming that we have 13 bits to store the immediate operand.
  //if ( (value <= 16383) && (value >= -16384) ) return(true);
  if ( (value <= MAX_IMM13) && (value >= MIN_IMM13) ) return(true);
  else return(false);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void instWaitingList::cleanUp(process *proc, Address pc) {
    proc->writeTextSpace((caddr_t)pc, sizeof(relocatedInstruction),
                    (caddr_t)&relocatedInstruction);
    proc->writeTextSpace((caddr_t)addr_, instSeqSize, (caddr_t)instructionSeq);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// process::replaceFunctionCall
//
// Replace the function call at the given instrumentation point with a call to
// a different function, or with a NOOP.  In order to replace the call with a
// NOOP, pass NULL as the parameter "func."
// Returns true if sucessful, false if not.  Fails if the site is not a call
// site, or if the site has already been instrumented using a base tramp.
bool process::replaceFunctionCall(const instPoint *point,
                                  const function_base *func) {
    // Must be a call site
    if (point->ipType != callSite)
        return false;

    // Cannot already be instrumented with a base tramp
    if (baseMap.defines(point))
        return false;

    // Replace the call
    if (func == NULL)
        generateNoOp(this, point->addr);
    else
        generateCall(this, point->addr,
                     func->getEffectiveAddress(this));

    return true;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#ifndef BPATCH_LIBRARY
bool process::isDynamicCallSite(instPoint *callSite){
  function_base *temp;
  if(!findCallee(*(callSite),temp)){
    //True call instructions are not dynamic on sparc,
    //they are always to a pc relative offset
    if(!isTrueCallInsn(callSite->firstInstruction))
      return true;
  }
  return false;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

bool process::MonitorCallSite(instPoint *callSite){
 
  if(isJmplInsn(callSite->firstInstruction)){
    vector<AstNode *> the_args(2);
    
    //this instruction is a jmpl with i == 1, meaning it
    //calling function register rs1+simm13
    if(callSite->firstInstruction.rest.i == 1){
      
      AstNode *base =  new AstNode(AstNode::PreviousStackFrameDataReg,
			  (void *) callSite->firstInstruction.rest.rs1);
      AstNode *offset = new AstNode(AstNode::Constant, 
			(void *) callSite->firstInstruction.resti.simm13);
      the_args[0] = new AstNode(plusOp, base, offset);
    } 
    
    //This instruction is a jmpl with i == 0, meaning its
    //two operands are registers
    else if(callSite->firstInstruction.rest.i == 0){
      //Calculate the byte offset from the contents of the %fp reg
      //that the registers from the previous stack frame 
      //specified by rs1 and rs2 are stored on the stack
      AstNode *callee_addr1 = 
	new AstNode(AstNode::PreviousStackFrameDataReg,
		    (void *) callSite->firstInstruction.rest.rs1);
      AstNode *callee_addr2 = 
	new AstNode(AstNode::PreviousStackFrameDataReg, 
		    (void *) callSite->firstInstruction.rest.rs2);
      the_args[0] = new AstNode(plusOp, callee_addr1, callee_addr2);
    }
    else assert(0);
    
    the_args[1] = new AstNode(AstNode::Constant,
			      (void *) callSite->iPgetAddress());
    AstNode *func = new AstNode("DYNINSTRegisterCallee", 
				the_args);
    addInstFunc(this, callSite, func, callPreInsn,
		orderFirstAtPoint,
		true,
		false);
  }
  else if(isTrueCallInsn(callSite->firstInstruction)){
    //True call destinations are always statically determinable.
    return true;
  }
  else return false;

  return true;
}
#endif

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// Emit code to jump to function CALLEE without linking.  (I.e., when
// CALLEE returns, it returns to the current caller.)  On SPARC, we do
// this by ensuring that the register context upon entry to CALLEE is
// the register context of function we are instrumenting, popped once.
void emitFuncJump(opCode op, char *i, Address &base, 
		  const function_base *callee, process *proc)
{
        assert(op == funcJumpOp);
        Address addr;
	void cleanUpAndExit(int status);

	addr = callee->getEffectiveAddress(proc);
	// TODO cast
	instruction *insn = (instruction *) ((void*)&i[base]);

        generateSetHi(insn, addr, 13); insn++;
	// don't want the return address to be used
        genImmInsn(insn, JMPLop3, 13, LOW10(addr), 0); insn++;
        genSimpleInsn(insn, RESTOREop3, 0, 0, 0); insn++;
        base += 3 * sizeof(instruction);
}


// If there is a base trampoline installed for this instPoint delete it
// and undo the operations done in findAndInstallBaseTramp. (i.e. replace
// the ba, a and save; call; restore; sequences with the instructions
// originally at those locations in the function
bool deleteBaseTramp(process *proc, instPoint* location,
		     instInstance* instance)
{
  Address ipAddr, baseAddress;
  trampTemplate* currentBaseTramp;

  if(!proc->baseMap.find((const instPoint*)location,currentBaseTramp)) {
    return false;
  }

  assert(currentBaseTramp == instance->baseInstance);

  // If the function has been relocated and the instPoint is from
  // the original instPoint, change the instPoint to be the one 
  // in the corresponding relocated function instead
  if(location->func->isInstalled(proc) && !location->relocated_) {

    location->func->modifyInstPoint( (const instPoint *)(location), proc );
  }

  // Get the base address of the shared object
  proc->getBaseAddress(location->image_ptr, baseAddress);

  // Get the address of the instPoint
  ipAddr = location->iPgetAddress() + baseAddress;


  // Replace the branch instruction with the instruction that was
  // originally there
  if( !location->needsLongJump ) {

    proc->writeTextWord( (caddr_t)(ipAddr), location->firstInstruction.raw );

  } else {

      // A call was used to transfer to the base tramp, clear out the
      // call and any other instructions that were written into the
      // function

      // Address of the first instruction in the instPoint's footprint
      Address firstAddress = ipAddr;

      if( location->hasNoStackFrame() ) {

          if (location->usesPriorInstructions) {

            firstAddress = ipAddr - 
                           location->numPriorInstructions*sizeof(instruction);
	  }

          // Replace the instruction overwritten by the save instruction
   	  proc->writeTextWord( (caddr_t)(firstAddress), 
                               location->firstInstruction.raw );

          // Replace the instruction overwritten by the call instruction
	  proc->writeTextWord( (caddr_t)(firstAddress) + sizeof(instruction), 
                               location->secondInstruction.raw );

	  if (location->ipType != otherPoint) {

              // Replace the instruction overwritten by the nop instruction
	      proc->writeTextWord( (caddr_t)(firstAddress) + 
                                     2*sizeof(instruction),
                                    location->thirdInstruction.raw );
	  }

      } else {

          if (location->ipType == functionEntry) {

            // Replace the instruction overwritten by the save instruction
	    proc->writeTextWord( (caddr_t)(firstAddress),
	      		         location->firstInstruction.raw);

            // Replace the instruction overwritten by the call instruction
	    proc->writeTextWord( (caddr_t)(firstAddress) + 
                                   sizeof(instruction),
	      		         location->secondInstruction.raw);
  
            // Replace the instruction overwritten by the nop instruction
	    proc->writeTextWord( (caddr_t)(firstAddress) + 
                                   2*sizeof(instruction),
				 location->thirdInstruction.raw);

	  } else if ( location->ipType == callSite || 
                      location->ipType == functionExit ) {

              // Replace the instruction overwritten by the call instruction
	      proc->writeTextWord( (caddr_t)(firstAddress),
				   location->firstInstruction.raw);
  
              // Replace the instruction overwritten by the nop instruction
	      proc->writeTextWord( (caddr_t)(firstAddress) + 
                                     sizeof(instruction),
				   location->secondInstruction.raw);


	  } else if (location->ipType == otherPoint) {

              // Replace the instruction overwritten by the call instruction
   	      proc->writeTextWord( (caddr_t)(firstAddress), 
                                   location->firstInstruction.raw );

              // Replace the instruction overwritten by the nop instruction
	      proc->writeTextWord( (caddr_t)(firstAddress) + 
                                     sizeof(instruction), 
                                   location->secondInstruction.raw );

	  }
      }
  }

  vector<Address> pointsToCheck;
  pointsToCheck.push_back( currentBaseTramp->baseAddr );
  pointsToCheck.push_back( instance->trampBase );

  vector< vector<Address> > checkPointHolder;
  checkPointHolder.push_back( (vector<Address>)(pointsToCheck) );

  // Free up the base trampoline
  inferiorFree( proc, currentBaseTramp->baseAddr, checkPointHolder );

  return true;
}


#ifdef BPATCH_LIBRARY

#include "BPatch_flowGraph.h"
#include "BPatch_function.h"

#include <sys/systeminfo.h>

/*
 * function which check whether the architecture is 
 * sparcv8plus or not. For the earlier architectures 
 * it is not possible to support random instrumentation 
 */
bool isV8plusISA()
{
    char isaOptions[256];

    if (sysinfo(SI_ISALIST, isaOptions, 256) < 0)
        return false;
    if (strstr(isaOptions, "sparcv8plus"))
        return true;
    return false;
}

/*
 * createInstructionInstPoint
 *
 * Create a BPatch_point instrumentation point at the given address, which
 * is guaranteed not be one of the "standard" inst points.
 *
 * proc         The process in which to create the inst point.
 * address      The address for which to create the point.
 */

BPatch_point *createInstructionInstPoint(process *proc, void *address,
					 BPatch_point** alternative,
					 BPatch_function* bpf)
{
    unsigned i;
    Address begin_addr,end_addr,curr_addr;

    //the method to check whether conservative base tramp can be installed
    //or not since it contains condition code instructions which is
    //available after version8plus of sparc

    if(!isV8plusISA()){
	cerr << "BPatch_image::createInstPointAtAddr : is not supported for";
	cerr << " sparc architecture earlier than v8plus\n";
	return NULL;
    }

    curr_addr = (Address)address;

    //if the address is not aligned then there is a problem
    if(!isAligned(curr_addr))	
	return NULL;

    function_base *func = NULL;
    if(bpf)
	func = bpf->func;
    else
	func = proc->findFuncByAddr(curr_addr);

    pd_Function* pointFunction = (pd_Function*)func;
    Address pointImageBase = 0;
    image* pointImage = pointFunction->file()->exec();
    proc->getBaseAddress((const image*)pointImage,pointImageBase);

    BPatch_function *bpfunc = proc->findOrCreateBPFunc((pd_Function*)func);

    BPatch_flowGraph *cfg = bpfunc->getCFG();
    BPatch_Set<BPatch_basicBlock*> allBlocks;
    cfg->getAllBasicBlocks(allBlocks);

    BPatch_basicBlock** belements =
                new BPatch_basicBlock*[allBlocks.size()];
    allBlocks.elements(belements);

    for(i=0; i< (unsigned)allBlocks.size(); i++) {
	void *bbsa, *bbea;
	if (belements[i]->getAddressRange(bbsa,bbea)) {
	    begin_addr = (Address)bbsa;
	    if ((begin_addr - INSN_SIZE) == curr_addr) {
		delete[] belements;
		BPatch_reportError(BPatchSerious, 118,
				   "point uninstrumentable");
		return NULL;
	    }
	}
    }
    delete[] belements;

    curr_addr -= pointImageBase;

    if (func != NULL) {
	instPoint *entry = const_cast<instPoint *>(func->funcEntry(NULL));
	assert(entry);

	begin_addr = entry->iPgetAddress();
	end_addr = begin_addr + entry->Size();

	if(((begin_addr - INSN_SIZE) <= curr_addr) && 
	   (curr_addr < end_addr)){ 
	    BPatch_reportError(BPatchSerious, 117,
			       "instrumentation point conflict 1");
	    if(alternative)
		*alternative = proc->findOrCreateBPPoint(bpfunc, entry, BPatch_entry);
	    return NULL;
	}

	const vector<instPoint*> &exits = func->funcExits(NULL);
	for (i = 0; i < exits.size(); i++) {
	    assert(exits[i]);

	    begin_addr = exits[i]->iPgetAddress();
	    end_addr = begin_addr + exits[i]->Size();

	    if (((begin_addr - INSN_SIZE) <= curr_addr) &&
		(curr_addr < end_addr)){
		BPatch_reportError(BPatchSerious, 117,
				   "instrumentation point conflict 2");
		if(alternative)
			*alternative = proc->findOrCreateBPPoint(bpfunc,exits[i],BPatch_exit);
		return NULL;
	    }
	}

	const vector<instPoint*> &calls = func->funcCalls(NULL);
	for (i = 0; i < calls.size(); i++) {
	    assert(calls[i]);

	    begin_addr = calls[i]->iPgetAddress();
	    end_addr = begin_addr + calls[i]->Size();

	    if (((begin_addr - INSN_SIZE) <= curr_addr) &&
		(curr_addr < end_addr)){
		BPatch_reportError(BPatchSerious, 117,
				   "instrumentation point conflict3 ");
		if(alternative)
			*alternative = proc->findOrCreateBPPoint(bpfunc,calls[i],BPatch_subroutine);
		return NULL;
	    }
	}
    }

    curr_addr += pointImageBase;

    /* Check for conflict with a previously created inst point. */
    if (proc->instPointMap.defines(curr_addr - INSN_SIZE)) {
	BPatch_reportError(BPatchSerious,117,"instrumentation point conflict 4");
	if(alternative)
		*alternative = (proc->instPointMap)[curr_addr-INSN_SIZE];
	return NULL;
    } else if (proc->instPointMap.defines(curr_addr + INSN_SIZE)) {
	BPatch_reportError(BPatchSerious,117,"instrumentation point conflict 5");
	if(alternative)
		*alternative = (proc->instPointMap)[curr_addr+INSN_SIZE];
	return NULL;
    }

    /* Check for instrumenting just before or after a branch. */

    bool decrement = false;
    if ((Address)address > func->getEffectiveAddress(proc)) {
	instruction prevInstr;
	proc->readTextSpace((char *)address - INSN_SIZE,
			    sizeof(instruction),
			    &prevInstr.raw);
	if (isDCTI(prevInstr)){
          if((prevInstr.call.op == CALLop) ||
             ((prevInstr.call.op != CALLop) && !prevInstr.branch.anneal))
          {
              BPatch_reportError(BPatchSerious, 118, "point uninstrumentable");
              return NULL;
          }
          decrement = true;
	}
    }

    if ((Address)address + INSN_SIZE <
	func->getEffectiveAddress(proc) + func->size()) {
	instruction nextInstr;
	proc->readTextSpace((char *)address + INSN_SIZE,
			    sizeof(instruction),
			    &nextInstr.raw);
	if (isDCTI(nextInstr)){
          proc->readTextSpace((char *)address + 2*INSN_SIZE,
                            sizeof(instruction),
                            &nextInstr.raw);
          if(!isNopInsn(nextInstr)){
              BPatch_reportError(BPatchSerious, 118,
                                 "point uninstrumentable");
              return NULL;
          }
	}
    }

    if(decrement){
      address = (void*)((Address)address - INSN_SIZE);
      curr_addr -= INSN_SIZE;
    }

    instruction instr;
    proc->readTextSpace(address, sizeof(instruction), &instr.raw);
	
    curr_addr -= pointImageBase;
    //then create the instrumentation point object for the address
    instPoint *newpt = new instPoint(pointFunction,
				     (const image*)pointImage,
				     (Address &)curr_addr,
				     false, // bool delayOk - ignored,
				     otherPoint);

    pointFunction->addArbitraryPoint(newpt,proc);

    return proc->findOrCreateBPPoint(bpfunc, newpt, BPatch_instruction);
}

/*
 * BPatch_point::getDisplacedInstructions
 *
 * Returns the instructions to be relocated when instrumentation is inserted
 * at this point.  Returns the number of bytes taken up by these instructions.
 *
 * maxSize      The maximum number of bytes of instructions to return.
 * insns        A pointer to a buffer in which to return the instructions.
 */
int BPatch_point::getDisplacedInstructions(int maxSize, void* insns)
{
    int count = 0;
    instruction copyOut[10];	// I think 7 is the max - jkh 8/3/00

    //
    // This function is based on what is contained in the instPoint 
    //    constructor in the file inst-sparc-solaris.C
    //
    if (!point->hasNoStackFrame()) {
	if (point->ipType == functionEntry) {
	    copyOut[count++].raw = point->secondInstruction.raw;
	    copyOut[count++].raw = point->thirdInstruction.raw;

	    if (point->secondIsDCTI) {
		if (point->fourthIsAggregate) {
		    copyOut[count++].raw = point->fourthInstruction.raw;
		}
	    }

	    if (point->thirdIsDCTI) {
		copyOut[count++].raw = point->fourthInstruction.raw;
		if (point->fifthIsAggregate) {
		    copyOut[count++].raw = point->fifthInstruction.raw;
		}
	    }

	} else if (point->ipType == callSite) {

	    copyOut[count++].raw = point->firstInstruction.raw;
	    copyOut[count++].raw = point->secondInstruction.raw;

	    if (point->thirdIsAggregate) {
		copyOut[count++].raw = point->thirdInstruction.raw;
	    }

	} else {

	    copyOut[count++].raw = point->firstInstruction.raw;
	    copyOut[count++].raw = point->secondInstruction.raw;
	}

    } else {

	if (point->ipType == functionEntry) {

	    copyOut[count++].raw = point->firstInstruction.raw;
	    copyOut[count++].raw = point->secondInstruction.raw;
	    copyOut[count++].raw = point->thirdInstruction.raw;
	    if (point->thirdIsDCTI) {
		copyOut[count++].raw = point->fourthInstruction.raw;
	    }

	} else if (point->ipType == functionExit) {

	    if (point->thirdPriorIsDCTI && point->firstPriorIsAggregate) {
		copyOut[count++].raw = point->thirdPriorInstruction.raw;
		copyOut[count++].raw = point->secondPriorInstruction.raw;
	    }

	    if (point->secondPriorIsDCTI) {
		copyOut[count++].raw = point->secondPriorInstruction.raw;
	    }

            copyOut[count++].raw = point->firstPriorInstruction.raw;
	    copyOut[count++].raw = point->firstInstruction.raw;
	    copyOut[count++].raw = point->secondInstruction.raw;

	} else if(point->ipType == otherPoint) {

	   copyOut[count++].raw = point->firstInstruction.raw;
	   copyOut[count++].raw = point->secondInstruction.raw;
	   if (point->secondIsDCTI) {
	       copyOut[count++].raw = point->thirdInstruction.raw;

	       if (point->thirdIsAggregate) {
	           copyOut[count++].raw = point->fourthInstruction.raw;
	       }
	   }

	} else {

	   assert(point->ipType == callSite);
	   copyOut[count++].raw = point->firstInstruction.raw;
	   copyOut[count++].raw = point->secondInstruction.raw;
	   if (point->thirdIsAggregate) {
	       copyOut[count++].raw = point->thirdInstruction.raw;
	   }
	}
    }

    if (count * sizeof(instruction) > (unsigned) maxSize) {
	return -1;
    } else {
	memcpy(insns, copyOut, count * sizeof(instruction));
	return count * sizeof(instruction);
    }
}

#endif

// needed in metric.C
bool instPoint::match(instPoint *p)
{
  if (this == p)
    return true;
  
  // should we check anything else?
  if (addr == p->addr)
    return true;
  
  return false;
}
