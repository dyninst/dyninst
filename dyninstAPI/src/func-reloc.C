#include "dyninstAPI/src/func-reloc.h"

extern bool isTrueCallInsn(const instruction insn);
extern bool isNearBranchInsn(const instruction insn);
extern void copyInstruction(instruction &newInsn, instruction &oldInsn,  
                                               unsigned &codeOffset);

class LocalAlterationSet;
class LocalAlteration;

/****************************************************************************/
/****************************************************************************/

/* Platform independent */

// update the target of relative branches (near jumps for x86) whose
// destination is outside the function

int pd_Function::fixRelocatedInstruction(bool setDisp, instruction *insn, 
                                         Address origAddr, Address targetAddr) { 

#ifdef DEBUG_FUNC_RELOC
  cerr << "pd_Function::fixRelocatedInstruction:" << endl; 
  cerr << " fixing displacement of insruction relocated" 
       << " from " << hex << origAddr << endl; 
  cerr << " to " << hex << targetAddr  
       << " with setDisp " << setDisp << endl; 
#endif

    int newDisp, disp, extra_bytes = 0;

    // check for relative addressing       
    if (isTrueCallInsn((const instruction) *insn) || 
        isNearBranchInsn((const instruction) *insn)) {
 	
        // get the original displacement
        disp = get_disp(insn);

        // calculate the new displacement 
        newDisp = (origAddr + disp) - targetAddr;

        // update displacement (if set_disp = true, extra_bytes should be 0).
        // extra_bytes = # of extra bytes needed to update insn's displacement 
        extra_bytes = set_disp(setDisp, insn, newDisp, true);
    }

#ifdef DEBUG_FUNC_RELOC 
    cerr << " extra_bytes = " << extra_bytes << endl; 
#endif

  return extra_bytes;
}

/****************************************************************************/
/****************************************************************************/

/* Platform independent */


// Fix displacement of relatvie branch or call instructions.
// Relative branches or calls with targets inside function will be updated by 
// patchOffset.
// Relative branches or calls with targets outside function will be updated by
// fixRelocatedInstruction

int pd_Function::relocateInstructionWithFunction(bool setDisp, 
                                                 instruction *insn, 
                                                 Address origAddr, 
                                                 Address targetAddr, 
                                                 Address oldFunctionAddr, 
                                                 unsigned originalCodeSize) {

#ifdef DEBUG_FUNC_RELOC 
  cerr << "pd_Funciton::relocateInstructionWithFunction " << endl;
#endif

  // check if insn is a relative branch with target inside function 
  if (branchInsideRange(*insn, origAddr, oldFunctionAddr, \
			  oldFunctionAddr + originalCodeSize)) {
    return 0;
  }
  // check if insn is a relative call with target inside function 
  if (trueCallInsideRange(*insn, origAddr, oldFunctionAddr, \
			    oldFunctionAddr + size())) {
    return 0;
  }

  return fixRelocatedInstruction(setDisp, insn, origAddr, targetAddr);
}

/****************************************************************************/
/****************************************************************************/

/* Platform independent */

// check insn is a branch with relative addressing and a target address 
// inside the function

bool pd_Function::branchInsideRange(instruction insn, Address branchAddress, 
                                    Address firstAddress, Address lastAddress) {

#ifdef DEBUG_FUNC_RELOC 
  cerr << "pd_Function::branchInsideRange:" << endl; 
  cerr << " Instruction offset = " << branchAddress - firstAddress << endl; 
  cerr << " function: " << hex << firstAddress << " to " 
       << hex << lastAddress << endl;  
#endif

  int disp;
  Address target;

  // check if insn is a branch with relative addressing 
  if (!isNearBranchInsn(insn)) return false;

  // get the displacement of the target
  disp = get_disp(&insn);

  // get target of branch instruction
#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0)
  target = branchAddress + disp + insn.size();
#elif defined(sparc_sun_solaris2_4)
  target = branchAddress + disp;
#endif

  if (target < firstAddress) return false;
  if (target >= lastAddress) return false;

#ifdef DEBUG_FUNC_RELOC 
  cerr << " branch target = " << hex << target - firstAddress << endl; 
#endif

  return true;
}

/****************************************************************************/
/****************************************************************************/

/* Platform independent */

// check if insn is a call with relative addressing and a target address
// inside the function

bool pd_Function::trueCallInsideRange(instruction insn, Address callAddress, 
                                      Address firstAddress, Address lastAddress) {

#ifdef DEBUG_FUNC_RELOC 
  cerr << "pd_Function::trueCallInsideRange:" << endl; 
  cerr << " Instruction offset = " << callAddress - firstAddress << endl;
  cerr << " function at: " << hex << firstAddress << " to "  << hex << lastAddress << endl;  
#endif

  Address target;
  int disp;
  
  // insn is a call instruction with relative addressing 
  if (!isTrueCallInsn(insn)) return false;

  // get the displacement of the target
  disp = get_disp(&insn);
   
  // get target of call instruction
#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0)
  target = callAddress + disp + insn.size();
#elif defined(sparc_sun_solaris2_4)
  target = callAddress + disp;
#endif

  if (target < firstAddress) return false;
  if (target >= lastAddress) return false;

#ifdef DEBUG_FUNC_RELOC 
  cerr << " call target = " << target - firstAddress << endl; 
#endif

  return true;
}

/****************************************************************************/
/****************************************************************************/

/* Platform independent */

// if setDisp is true, update the displacement of the relative call or branch
// instructions if its target address is inside the function 
// Otherwise, calculate by how many bytes the insn needs to be expanded so
// that the branch or call target is within the instructions range

int pd_Function::patchOffset(bool setDisp, LocalAlterationSet *alteration_set, 
                             instruction &insn, Address adr, 
                             Address firstAddress, 
                             unsigned originalCodeSize) {
  
  int disp, extra_offset, extra_bytes;
  int lastAddress = firstAddress + originalCodeSize;
  int insnSize = sizeOfMachineInsn(&insn);

#ifdef DEBUG_FUNC_RELOC 
  cerr << "pd_Function::patchOffset" << endl;
  cerr << " Instruction offset = " << adr - firstAddress << endl;
  cerr << " function at: " << hex << firstAddress << " to " << hex << lastAddress << endl;   
#endif

  // insn is a relative branch or call instruction with target inside function
  if (!branchInsideRange(insn, adr, firstAddress, lastAddress) && 
      !trueCallInsideRange(insn, adr, firstAddress, lastAddress)) return 0;

  //  get the displacement of the instruction
  disp = get_disp(&insn);
 

  // extra_offset: # of bytes that function expansion has further seperated 
  // the address of the insn from its target address 
  alteration_set->Collapse();
  extra_offset = 
         alteration_set->getShift((adr - firstAddress) + insnSize + disp) - 
         alteration_set->getShift((adr - firstAddress)+ insnSize);

  // (if set_disp = true, update the displacement, in which case 
  // extra_bytes = 0).
  // Otherwise extra_bytes = # of extra bytes needed to update insn's 
  // displacement (which could be 0 as well) 
  extra_bytes = set_disp(setDisp, &insn, disp + extra_offset, false);

#ifdef DEBUG_FUNC_RELOC 
    cerr << " extra_bytes = " << extra_bytes << endl; 
#endif

  return extra_bytes;
}


/****************************************************************************/
/****************************************************************************/

/* Plarform independent */

// return the # of bytes by which the function will be expanded, if it is
// relocated
int pd_Function::relocatedSizeChange(const image *owner, process *proc) {

  instruction *oldInstructions = 0;
  Address mutator, mutatee;
  LocalAlterationSet normalized_alteration_set(this);

  return findAlterations(owner, proc, oldInstructions, normalized_alteration_set, 
                         mutator, mutatee);
}


/****************************************************************************/
/****************************************************************************/

/* Plarform independent */

// calculate the size and location of all alterations needed to relocate the 
// function. 
// normalized_alteration_set will contain the record of these changes
// Return the sum total of the sizes of the alterations

// oldInstructions: buffer of instruction objects
// mutatee: address of actual function to be relocated (located in the mutatee)
// mutator: address of copy of the above function  (located in the mutator)

int pd_Function::findAlterations(const image *owner, 
                                 process *proc, 
                                 instruction *&oldInstructions,
                                 LocalAlterationSet &normalized_alteration_set,
                                 Address &mutator, Address &mutatee) {

LocalAlterationSet temp_alteration_set(this);
Address baseAddress;
unsigned numberOfInstructions;
int totalSizeChange = 0;
bool relocate = true;
bool expanded = true;

#ifdef DEBUG_FUNC_RELOC
    cerr << "pd_Function::findAlterations called " << endl;
    cerr << " prettyName = " << prettyName().string_of() << endl;
    cerr << " size() = " << size() << endl;
    cerr << " this = " << *this << endl;
#endif

  if (size() == 0) {
    cerr << "WARN : attempting to relocate function " \
       	 << prettyName().string_of() << " with size 0, unable to instrument" \
         <<  endl;
    return -1;
  }

  // get baseAddress if this is a shared object
  baseAddress = 0;
  if(!(proc->getBaseAddress(owner,baseAddress))){
    baseAddress = 0;
  }
  // address of function (in mutatee) 
  mutatee = baseAddress + getAddress(0);

  // create a buffer of instruction objects 
  if (!(loadCode(owner, proc, oldInstructions, numberOfInstructions, mutatee)))
  {
    return -1;  
  }

  // address of copy of function (in mutator)
  mutator = addressOfMachineInsn(&oldInstructions[0]);

#ifdef DEBUG_FUNC_RELOC
  cerr << " mutator = " << mutator << endl;
  cerr << " mutatee = " << mutatee << endl;
#endif

  // discover which instPoints need to be expaned  
  expanded = expandInstPoints( owner, &temp_alteration_set, 
                               normalized_alteration_set, 
                               baseAddress, mutator, mutatee, 
                               oldInstructions, 
                               numberOfInstructions); 

  if (expanded) { 
  // Check if the expansions discovered in expandInstPoints would require
  // further expansion of the function (this occurs if the target of a 
  // relative branch or call would be moved outside the range of the insn
    relocate = updateAlterations(&temp_alteration_set, 
                                 normalized_alteration_set, 
                                 oldInstructions, baseAddress, 
                                 mutatee, totalSizeChange); 
  } else {
    // Don't relocate
    totalSizeChange = -1;
  }

  // Don't relocate
  if (!relocate) totalSizeChange = -1;

#ifdef DEBUG_FUNC_RELOC
    cerr << " totalSizeChange = " << totalSizeChange << endl;
#endif

  return totalSizeChange;
}
  
  
/****************************************************************************/
/****************************************************************************/

/* Plarform independent */

// First expand the function, determining all LocalAlterations that need 
// to be applied, and then apply them, writing the rewritten function to
// the buffer relocatedCode

// newAdr: address in mutatee where function is to be relocated to
// reloc_info: info about relocated functions
// size_change: # bytes by which relocated function is larger than original

/* IMPORTANT: The function is not actually relocated in this method */

bool pd_Function::findAndApplyAlterations(const image *owner, 
			                  instPoint *&location,
      				          u_int &newAdr, 
                                          process *proc, 
                                          relocatedFuncInfo *&reloc_info, 
                                          unsigned &size_change) {
	
  instruction *oldInstructions = 0, *newInstructions = 0;
  Address mutator, mutatee;
  int totalSizeChange = 0;
  LocalAlterationSet normalized_alteration_set(this); 

  // assumes delete NULL ptr safe....
  oldInstructions = NULL;

#ifdef DEBUG_FUNC_RELOC
    cerr << "pd_Function::findAndApplyAlterations called " << endl;
    cerr << " prettyName = " << prettyName().string_of() << endl;
    cerr << " size() = " << size() << endl;
    cerr << " this = " << *this << endl;
#endif

  // Find the alterations that need to be applied
  totalSizeChange = findAlterations(owner, proc, oldInstructions,
                                    normalized_alteration_set,
                                    mutator, mutatee);  

  if (totalSizeChange == -1) {

    // Do not relocate function
    delete []oldInstructions;
    return false;
  }

    // make sure that new function version can fit in newInstructions (statically allocated)....
    if (size() + totalSizeChange > NEW_INSTR_ARRAY_LEN) {
       cerr << "WARN : attempting to relocate function " \
       	    << prettyName().string_of() << " with size " \
	    << " > NEW_INSTR_ARRAY_LEN (" << NEW_INSTR_ARRAY_LEN \
	    << "), unable to instrument : " \
            << " size = " << size() << " , totalSizeChange = " \
	    << totalSizeChange << endl;
       delete []oldInstructions;
       return FALSE;
    }

#ifdef DEBUG_FUNC_RELOC
    cerr << " Allocate memory: " << size() + totalSizeChange << endl;
#endif

    // Allocate the memory on the heap to which the function will be relocated
    if(!reloc_info){  
      Address ipAddr = 0;
      proc->getBaseAddress(owner,ipAddr);
      ipAddr += location -> iPgetAddress();
      u_int ret = inferiorMalloc(proc, size() + totalSizeChange, textHeap, ipAddr);
      newAdr = ret;
      if(!newAdr) {
        delete []oldInstructions; 
        return false;
      }
      reloc_info = new relocatedFuncInfo(proc,newAdr);
      relocatedByProcess.push_back(reloc_info);
    }

    // Allocate the memory in paradyn to hold a copy of the rewritten function
    relocatedCode = new unsigned char[size() + totalSizeChange];
    if(!relocatedCode) {
      cerr << "WARNING: Allocation of space for relocating function "
           << prettyName() << " failed." << endl;
       delete []oldInstructions;  
       return false;
    } 

#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0)
      newInstructions = NEW_INSTR;
#elif defined(sparc_sun_solaris2_4)
      newInstructions = reinterpret_cast<instruction *> (relocatedCode);
#endif

    // Apply the alterations needed for relocation. The expanded function 
    // will be written to relocatedCode.
    if (!(applyAlterations(normalized_alteration_set, mutator, mutatee, 
                           newAdr, oldInstructions, size(), newInstructions) )) {
      delete []oldInstructions;
      return false;
    }


    // Fill reloc_info up with relocated (and altered by alterations) inst 
    // points. Do AFTER all alterations are attached AND applied....
    fillInRelocInstPoints(owner, proc, location, reloc_info, 
                          mutatee, mutator, oldInstructions, newAdr, newInstructions, 
                          normalized_alteration_set);

    
    size_change = totalSizeChange;  

    delete []oldInstructions;
    return true;
}

/****************************************************************************/
/****************************************************************************/

// Calulate which instPoints need to be expanded to allow for instrumentation  

// numberOfInstructions: # of insn's in function (as opposed to # of bytes)
// temp_alteration_set: record of the needed expansions 

bool pd_Function::expandInstPoints(
                               const image *owner,
                               LocalAlterationSet *temp_alteration_set, 
                               LocalAlterationSet &normalized_alteration_set, 
                               Address baseAddress, Address mutator,
                               Address mutatee, instruction oldInstructions[], 
                               unsigned numberOfInstructions ) {

  bool combined1, combined2, combined3;

#ifdef DEBUG_FUNC_RELOC
    cerr << "pd_Function::expandInstPoints called "<< endl;
    cerr << " baseAddress = " << hex << baseAddress << endl;
    cerr << " mutator = " << hex << mutator << endl;
    cerr << " mutatee = " << hex << mutatee << endl;
    cerr << " numberOfInstructions = " << numberOfInstructions << endl;
#endif

  LocalAlterationSet tmp_alt_set1(this);
  LocalAlterationSet tmp_alt_set2(this);

  // Perform three passes looking for instPoints that need expansion

  PA_attachGeneralRewrites(owner, temp_alteration_set, baseAddress, 
                           mutatee, oldInstructions, size());
  PA_attachOverlappingInstPoints(&tmp_alt_set1, baseAddress, 
                                 mutatee, oldInstructions, size());
  PA_attachBranchOverlaps(&tmp_alt_set2, baseAddress, mutator, 
                          oldInstructions, numberOfInstructions, size());

  // merge the LocalAlterations discovered in the above passes, placing
  // them in normalized_alteration_set 

  combined1 = combineAlterationSets(temp_alteration_set, &tmp_alt_set1);
  combined2 = combineAlterationSets(temp_alteration_set, &tmp_alt_set2);
  combined3 = combineAlterationSets(&normalized_alteration_set, temp_alteration_set);    

  if (!combined1 || !combined2 || !combined3) {
    return false;
  }

  return true;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

  /* Platform independent */

// Check if targetAddress (the target of a branch or call insn) 
// is in the footprint of an inst point. 

instPoint *pd_Function::find_overlap(vector<instPoint*> v, Address targetAddress) {

#ifdef DEBUG_FUNC_RELOC
  cerr << " find_overlap:" 
       << " of target address " << hex << targetAddress 
       << " with an instrumentation point " << endl;
#endif
    
  for (unsigned u = 0; u < v.size(); u++) {
    instPoint *i = v[u];
     
    if (targetAddress <= i->firstAddress()) {
      return NULL;
    }
    if (targetAddress > i->firstAddress() && targetAddress < i->followingAddress()) {
	return i;
    }
  }
  return NULL;
}

/****************************************************************************/
/****************************************************************************/

/* Platform independent */

// Alterations needed to increase the range of relative branch and call 
// instructions, are located in this function. i.e. alterations may need
// to be applied if the target of a 2 byte jmp instruction is no longer 
// within 128 bytes of the insn, due to expanding other instPoints. To deal 
// with this, we expand the jmp instruciton into a 5 byte jump.

bool pd_Function::discoverAlterations(LocalAlterationSet *temp_alteration_set, 
                                      LocalAlterationSet &norm_alt_set,
				      Address baseAddress, Address firstAddress, 
                                      instruction code[], 
				      int codeSize) {

  int oldOffset = 0, newOffset = 0, codeOffset = 0; 
  int size_of_expansion1, size_of_expansion2;
 
  // set to true if a new LocalAlteration is discovered 
  bool foundAlteration = false;  

  LocalAlterationSet discover_alteration_set(this);

#ifdef DEBUG_FUNC_RELOC
    cerr << "pd_Function::discoverAlterations called" <<endl;
    cerr << " firstAddress = " << hex << firstAddress
         << " codeSize = " << codeSize << endl;
#endif 

  norm_alt_set.iterReset();  
  norm_alt_set.Collapse(); 

  // iterate over all instructions
  while (oldOffset < codeSize) {

#ifdef DEBUG_FUNC_RELOC 
      cerr << " oldOffset = " << oldOffset << endl;
#endif
 
    // # of bytes needed to expand instruction so that its target 
    // is still within its range.
    size_of_expansion1 = relocateInstructionWithFunction(
                                     false, &code[codeOffset], 
                                     baseAddress + firstAddress + oldOffset, 
                                     baseAddress + firstAddress + newOffset, 
                                     baseAddress + firstAddress, codeSize);

    // if instruction needs to be expanded 
    if (size_of_expansion1) {

#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0)

      // expansion was not already found
      if (!alreadyExpanded(oldOffset, size_of_expansion1, &norm_alt_set)) {

        // new LocalAlteration
        ExpandInstruction *exp = new ExpandInstruction(this, oldOffset,       
                                                       size_of_expansion1);
        discover_alteration_set.AddAlteration(exp);
        foundAlteration = true;

#ifdef DEBUG_FUNC_RELOC 
	  cerr << " ExpandInstruction alteration found " << endl;
          cerr << "     offset: " << oldOffset << endl; 
          cerr << "     size: " << size_of_expansion1 << endl;
#endif
      }

#elif defined(sparc_sun_solaris2_4)
        // Don't expand instruction on sparc. i.e. if the new branch target is 
        // greater than 2^21 bytes away, don't relocate the function 
        return false;
#endif

    }

    // # of bytes needed to expand instruction so that its target 
    // is still within its range.
    size_of_expansion2 = patchOffset(false, &norm_alt_set, 
                                     code[codeOffset], 
                                     firstAddress + oldOffset, 
                                     firstAddress, codeSize);


    // if instruction needs to be expanded      
    if (size_of_expansion2) {
      // insn target is either inside reange of function or outside range
      // of function, not both
      assert(!size_of_expansion1);

#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0)

      // expansion was not already found
      if (!alreadyExpanded(oldOffset, size_of_expansion2, &norm_alt_set)) {

        // new LocalAlteration
	ExpandInstruction *exp = new ExpandInstruction(this, oldOffset, 
                                                         size_of_expansion2);
        discover_alteration_set.AddAlteration(exp);
        foundAlteration = true;        

#ifdef DEBUG_FUNC_RELOC 
	  cerr << " ExpandInstruction alteration found " << endl;
          cerr << "     offset: " << oldOffset << endl; 
          cerr << "     size: " << size_of_expansion2 << endl;
#endif
      }

#elif defined(sparc_sun_solaris2_4)
        // Don't expand instruction on sparc. i.e. if the new branch target is 
        // greater than 2^21 bytes away, don't relocate the function 
        return false;
#endif

    }

    // next intsruction
    oldOffset += sizeOfMachineInsn(&code[codeOffset]);
    newOffset += sizeOfMachineInsn(&code[codeOffset]); 

    codeOffset++;      
  }

  // If any new LocalAlterations were found, pass them out of this function 
  // through temp_alteration_set
  discover_alteration_set.Collapse();

  if (foundAlteration) *temp_alteration_set = discover_alteration_set;

  return true;
}
/****************************************************************************/
/****************************************************************************/

    /* Platform independent */

// place an expanded copy of the original function in relocatedCode 

// mutatee: first address of function in mutatee
// mutator: first address of copy of function in mutator
// newAdr: address in mutatee where function is to be relocated to
// codeSize: size of original, unexpanded function
// oldInstructions: buffer of insn's corresponding to copy of function in mutator
// newInstructions: buffer of insn's corresponding to expanded function located
//          in temporary buffer in mutator

bool pd_Function::applyAlterations(LocalAlterationSet &norm_alt_set,
				   Address mutator, Address mutatee, 
                                   Address newAdr, 
                                   instruction oldInstructions[], 
				   unsigned codeSize, 
                                   instruction newInstructions[]) {

  // offset of current insn from beginning of original function  
  Address oldOffset = 0;
  // offset of current insn from beginning of expanded function
  Address  newOffset = 0;
  // next alteration
  LocalAlteration *nextAlter = 0; 
  // address at which next alteration begins 
  Address nextAlterBegins;

  // offset of current insn into buffer of instructon objects 
  // (oldInstructions and newInstructions respectively)
  int oldInsnOffset = 0, newInsnOffset = 0;

  int newDisp = 0;

  // offset into buffer for rerwitten function
  unsigned codeOffset = 0;

  Address oldAdr_before, oldAdr_after, newAdr_before, newAdr_after; 

#ifdef DEBUG_FUNC_RELOC
    cerr << "pd_Function::applyAlterations called" <<endl;
    cerr << " mutator = " << hex << mutator << endl;
    cerr << " mutatee = " << hex << mutatee << endl;
    cerr << " newAdr = " << hex << newAdr << endl;
    cerr << " codeSize = " << codeSize << endl;
#endif 

  norm_alt_set.iterReset();  
  norm_alt_set.Collapse(); 

  // iterate over all instructions in function....
  while (oldOffset < codeSize) {

    // next alteration
    nextAlter = norm_alt_set.iterNext();
  
    if (nextAlter == NULL) {
      // no more alterations 
      nextAlterBegins = codeSize;
    } else {
        // offset at which next alteration begins
	nextAlterBegins = nextAlter->getOffset();
    }

    // iterate over all instructions before the next alteration, 
    while (oldOffset < nextAlterBegins) {

#ifdef DEBUG_FUNC_RELOC 
      cerr << " oldOffset = " << oldOffset << endl;
      cerr << " nextAlterBegins = " << nextAlterBegins << endl;
#endif
         
      // copy current instruction into temporary buffer in mutator
      copyInstruction(newInstructions[newInsnOffset], oldInstructions[oldInsnOffset], 
                                                          codeOffset);
    
      // mutatee + oldOffset = address in mutatee of current instruction
      // newAdr + newOffset = address in mutatee where current instruction is
      // to be relocated
      // update relative branches and calls to locations outside the function 
      relocateInstructionWithFunction(true,&newInstructions[newInsnOffset], 
                                      mutatee + oldOffset, newAdr + newOffset,
                                      mutatee, codeSize);

      // mutatee + oldOffset = address in mutatee of current instruction
      // newInstructions[newInsnOffset] = insn in temporary buffer to be patched up
      // update relative branches and calls to locations inside the function
      patchOffset(true, &norm_alt_set, newInstructions[newInsnOffset], 
                  mutator + oldOffset, mutator, codeSize);

      // next instruction
      oldOffset += sizeOfMachineInsn(&oldInstructions[oldInsnOffset]);
      newOffset += sizeOfMachineInsn(&oldInstructions[oldInsnOffset]);
      oldInsnOffset++; 
      newInsnOffset++;
    }

#ifdef DEBUG_FUNC_RELOC 
      cerr << " oldOffset = " << oldOffset << endl;
      cerr << " nextAlterBegins = " << nextAlterBegins << endl;
#endif

    assert(oldOffset == nextAlterBegins);
  
    // next alteration to be applied
    if (nextAlter != NULL) {

      // branch or call instruction with relative addressing
      if(isNearBranchInsn(oldInstructions[oldInsnOffset])  || 
         isTrueCallInsn(oldInstructions[oldInsnOffset])) {
        int oldDisp = get_disp(&oldInstructions[oldInsnOffset]);
        int oldInsnSize = sizeOfMachineInsn(&oldInstructions[oldInsnOffset]);  

        // mutator + oldOffset = address in mutator of current instruction
        // mutator + codeSize = last address of copy of function in mutator
        if ((branchInsideRange(oldInstructions[oldInsnOffset], mutator + oldOffset, 
                               mutator, mutator + codeSize))   ||
            (trueCallInsideRange(oldInstructions[oldInsnOffset], mutator + oldOffset, 
                                 mutator, mutator + codeSize))) {

          // updated disp for relative branch or call insn to target 
          // inside function
          newDisp = oldDisp + (norm_alt_set.getShift(oldOffset + oldInsnSize +
                               oldDisp) - norm_alt_set.getShift(oldOffset + oldInsnSize));

        } else {

            // updated disp for relative branch or call insn to target 
            // outside function
#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0)
            int origAddr = (mutatee + oldOffset + oldInsnSize + oldDisp);
            int targetAddr = (newAdr + newOffset + oldInsnSize + 
                    set_disp(false, &oldInstructions[oldInsnOffset], newDisp, true));
#elif defined(sparc_sun_solaris2_4)
            int origAddr = (mutatee + oldOffset + oldDisp);
            int targetAddr = (newAdr + newOffset + 
                    set_disp(false, &oldInstructions[oldInsnOffset], newDisp, true));
#endif

            newDisp = origAddr - targetAddr;
	} 

#ifdef DEBUG_FUNC_RELOC 
      cerr << " newDisp " << newDisp << " oldDisp " << oldDisp << endl;
#endif

      }

      // RewriteFootprint makes the appropriate alterations to the  
      // function at the instruction specified by the instPoint. 
      // Unfortunately RewriteFootprint may change the offset values, 
      // so we have to make sure we fix any altered values
      // 

      // oldAdr_before: address of current insn in mutatee  
      // oldAdr_after: address of next instruction to be dealt with
      //                (in mutatee), after alteration has been applied

      // newAdr_before: address where current insn will be relocated to   
      // newAdr_after: address where next instruction will be relocated to 

      // oldInstructions: buffer of old insn objects
      // newInstructions: buffer of new insn objects being made
      // oldInsnOffset: offset of instruction object (in oldInstructions) 
      //                corresponding to current insn
      // newInsnOffset: offset of instruction object (in newInstructions) 
      //                corresponding to current insn

      oldAdr_after = oldAdr_before = mutatee + oldOffset;
      newAdr_after = newAdr_before = newAdr + newOffset;

      nextAlter->RewriteFootprint(mutatee, oldAdr_after, 
                                  newAdr, newAdr_after, 
                                  oldInstructions, newInstructions, oldInsnOffset,  
                                  newInsnOffset, newDisp, 
                                  codeOffset, relocatedCode);

      // update offsets by the # of bytes RewriteFootprint walked over
      oldOffset += (oldAdr_after - oldAdr_before);
      newOffset += (newAdr_after - newAdr_before);
    }
  } 
  
  return true;
}

/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/****************************************************************************/

/* Platform independent */

// Combine two LocalAlterationSets, making sure the LocalAlterations
// remain in order from smallest to largest offset.
bool combineAlterationSets(LocalAlterationSet *combined_alteration_set, 
                          LocalAlterationSet *alteration_set) {

  assert (combined_alteration_set != NULL);
  assert (alteration_set != NULL);

  LocalAlterationSet temp_alteration_set = *combined_alteration_set;
  combined_alteration_set->Flush();

#ifdef DEBUG_FUNC_RELOC 
  cerr << "pd_Function::combineAlterationSets " << endl;
#endif

  alteration_set->iterReset();  
  LocalAlteration *alteration = alteration_set->iterNext(); 
  
  temp_alteration_set.iterReset();
  LocalAlteration *tempAlteration = temp_alteration_set.iterNext();
 
  // While there are still LocalAlterations
  while (tempAlteration != NULL || alteration != NULL) { 

    // if alteration is NULL or tempAlteration has the smaller offset
    if ((alteration == NULL) || ((tempAlteration != NULL) && 
         alteration->getOffset() > tempAlteration->getOffset())) {

      // tempAlteration is the next LocalAlteration to be added (     
      combined_alteration_set->AddAlteration(tempAlteration);
      tempAlteration = temp_alteration_set.iterNext();     
    } else {
        if ((tempAlteration == NULL) || 
             alteration->getOffset() < tempAlteration->getOffset()) { 

          // alteration is the next LocalAlteration to be added
          combined_alteration_set->AddAlteration(alteration);
          alteration = alteration_set->iterNext();
        } else {

            assert (alteration->getOffset() == tempAlteration->getOffset());

            // reconcile the conflict of overlapping LocalAlterations
            alteration = fixOverlappingAlterations(alteration, tempAlteration);
  
            if (alteration == NULL) return false;
            
            combined_alteration_set->AddAlteration(alteration);
  
            alteration = alteration_set->iterNext();
            tempAlteration = temp_alteration_set.iterNext();
        }
    }      
  }
  combined_alteration_set->iterReset();  
  combined_alteration_set->Collapse(); 
  return true;
}

/****************************************************************************/
/****************************************************************************/

// find out if previous LocalAlterations have expanded the function in such
// a way as to require new LocalAlterations to be added.

bool pd_Function::updateAlterations(LocalAlterationSet *temp_alteration_set,
                                    LocalAlterationSet &normalized_alteration_set,
                                    instruction *oldInstructions, 
                                    Address baseAddress,
                                    Address firstAddress,
                                    int &totalSizeChange) { 

  // false if discoverAlterations failed
  bool relocate = true;
  bool combined = true;
  assert (temp_alteration_set != NULL);

  temp_alteration_set->iterReset();
  LocalAlteration *iterator = temp_alteration_set->iterNext();

#ifdef DEBUG_FUNC_RELOC 
  cerr << " updateAlterations: " << endl;
  cerr << " address of process " << hex << baseAddress << endl;
  cerr << " address of function being relocated" << hex << firstAddress << endl;
  cerr << " bytes of expansion of function " << totalSizeChange << endl; 
#endif

  // while discoverAlterations discovers new LocalAlterations
  for (int i = 0; iterator != NULL && i < 2 ; i++) {
    
    // normalized_alteration_set already contains 
    // temp_alteration_set is already normalized the first time through
    if (i != 0) {   
      
      // merge normalized_alteration_set and temp_alteration_set
      combined = combineAlterationSets(&normalized_alteration_set, temp_alteration_set);
 
      if (!combined) return false;
    }

    temp_alteration_set->Flush();

    // find new alterations that have come about, due to the expansion 
    // of the function. e.g. ExpandInstruction LocalAlterations 
    relocate = discoverAlterations(temp_alteration_set, normalized_alteration_set, 
                                     baseAddress, firstAddress, oldInstructions, size());
 
    // Don't relocate the function
    if (!relocate) return false;

    temp_alteration_set->iterReset();

    // NULL if no new alterations were found
    iterator = temp_alteration_set->iterNext();
  }

  totalSizeChange = normalized_alteration_set.sizeChange();  

#ifdef DEBUG_FUNC_RELOC 
  cerr << " Number of bytes of expansion = " << totalSizeChange << endl;  
#endif

  return true;
} 

/****************************************************************************/
/****************************************************************************/

// Function relocation requires a version of process::convertPCsToFuncs 
// in which null functions are not passed into ret. - Itai 
vector<pd_Function *> process::pcsToFuncs(vector<Address> pcs) {
    vector <pd_Function *> ret;
    unsigned i;
    pd_Function *fn;
    for(i=0;i<pcs.size();i++) {
        fn = (pd_Function *)findFunctionIn(pcs[i]);
        // no reason to add a null function to ret
        if (fn != 0) ret.push_back(fn);
    }
    return ret;
}

/****************************************************************************/
/****************************************************************************/

/* Platform independent */

// Relocate "this" function

bool pd_Function::relocateFunction(process *proc, instPoint *&location) {

    relocatedFuncInfo *reloc_info = 0;

    // how many bytes the function was expanded by
    unsigned size_change;

#ifdef DEBUG_FUNC_RELOC 
    cerr << "pd_Function::relocateFunction " << endl;
    cerr << " prettyName = " << prettyName().string_of() << endl;
    cerr << " size() = " << size() << endl;
    cerr << " this = " << *this << endl;
#endif

    // check if this process already has a relocation record for this 
    // function, meaning that the.function has already been relocated
    for(u_int j=0; j < relocatedByProcess.size(); j++){
        if((relocatedByProcess[j])->getProcess() == proc){        
            reloc_info = relocatedByProcess[j];
	    
#ifdef DEBUG_FUNC_RELOC
            cerr << "pd_Function::relocateFunction " << endl;
            cerr << " prettyName = " << prettyName().string_of() << endl;      
            cerr << " previously relocated." << endl; 
#endif
        }
    }

   
  /* Check if we are currently executing inside the function we want to */ 
  /* instrument. If so, don't relocate the function.                    */
  /* this code was copied from metricDefinitionNode::adjustManuallyTrigger() */
  /* in metric.C -itai                                                  */

    unsigned i;
    pd_Function *stack_func;
#if defined(MT_THREAD)
    vector<Address> stack_pcs;
    vector<vector<Address> > pc_s = proc->walkAllStack();
    for (i=0; i< pc_s.size(); i++) {
      stack_pcs += pc_s[i];
    }
#else
    vector<Address> stack_pcs = proc->walkStack();
#endif
    if( stack_pcs.size() == 0 )
      cerr << "WARNING -- process::walkStack returned an empty stack" << endl;
    vector<pd_Function *> stack_funcs = proc->pcsToFuncs(stack_pcs);
    for(i=0;i<stack_funcs.size();i++) {
      stack_func = stack_funcs[i];      
      if( stack_func == this ) {
#ifdef DEBUG_FUNC_RELOC
        cerr << "pd_Function::relocateFunction" << endl;
        cerr << "currently in Function" << endl;
#endif
        return false;
      }
    }

    /* We are not currently executing in this function, 
       so proceed with the relocation */


    Address baseAddress = 0;
    if(!(proc->getBaseAddress(location->iPgetOwner(),baseAddress))){
      baseAddress = 0;
    }

    // original address of function (before relocation)
    u_int origAddress = baseAddress + getAddress(0);    
 
    // address to which function will be relocated.
    // memory is not allocated until total size change of function is known
    u_int ret = 0;

    if (!reloc_info) {
      // findAndApplyAlterations expands and updates the function, 
      // storing the expanded function with relocated addresses in 
      // the buffer relocatedCode.
      if (findAndApplyAlterations(location->iPgetOwner(), location, ret, proc, 
                                  reloc_info, size_change)) {

        // Copy the expanded and updated function into the mutatee's 
        // address space
        proc->writeDataSpace((caddr_t)ret, size() + size_change,
                             relocatedCode);

        // branch from original function to relocated function
        generateBranch(proc, origAddress, ret);
        reloc_info->setInstalled();

#ifdef DEBUG_FUNC_RELOC
        cerr << "pd_Function::relocateFunction " << endl;
        cerr << " prettyName = " << prettyName().string_of() << endl;      
        cerr << " relocated from 0x" << hex << origAddress
	     << " with size 0x" << size() << endl;
        cerr << " to 0x" << hex << ret 
             << " with size 0x" << size()+size_change << endl;
#endif

      } else {

#ifdef DEBUG_FUNC_RELOC
        cerr << "Warning: Unable to relocate function"
             << prettyName() << endl;
#endif
          return false;
      }

      return true;
    }
    return false;
}

/****************************************************************************/
/****************************************************************************/

// 32 bit addresses
#ifndef MAX_ADDRESS
#define MAX_ADDRESS 0xffffffff;
#endif

// Fill up vector with instPoints and then sort it 

void pd_Function::sorted_ips_vector(vector<instPoint*>&fill_in) {
    unsigned int returns_idx, calls_idx;
    Address returns_ip_addr, calls_ip_addr;


#ifdef DEBUG_FUNC_RELOC 
    cerr << " sorted_ips_vector: " << endl;
    cerr << "sort vector of instPoints " << endl;
#endif

    // sorted vector of inst points starts with funcEntry_  ....
    fill_in.push_back(funcEntry_);

    returns_idx = calls_idx = 0;

    // step through funcReturns and calls, popping element off of each and
    //  looking at iPgetAddress() to see which one to stick onto fill_in next....
    while (returns_idx < funcReturns.size() || calls_idx < calls.size()) {
        if (returns_idx < funcReturns.size()) {
	    returns_ip_addr = funcReturns[returns_idx]->iPgetAddress();
        } else {
	    returns_ip_addr = MAX_ADDRESS;
	}
	
        if (calls_idx < calls.size()) {
	    calls_ip_addr = calls[calls_idx]->iPgetAddress();
        } else {
	    calls_ip_addr = MAX_ADDRESS;
	}

	// 2 inst points at same location????
	assert(returns_ip_addr != calls_ip_addr);

	// if next call inst point comes before next return inst point, add
	//  the call inst point....
	if (calls_ip_addr < returns_ip_addr) {
	    fill_in.push_back(calls[calls_idx++]);
	} else {
	    fill_in.push_back(funcReturns[returns_idx++]);
	}
    }
}
