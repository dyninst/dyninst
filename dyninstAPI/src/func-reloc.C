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
  target = branchAddress + disp + insn.size();

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
   
  // calculate the target address of the call instruction 
  target = callAddress + disp + insn.size();

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

  instruction *oldInstr = 0;
  Address mutator, mutatee;
  LocalAlterationSet normalized_alteration_set(this);

  return findAlterations(owner, proc, oldInstr, normalized_alteration_set, 
                         mutator, mutatee);
}


/****************************************************************************/
/****************************************************************************/

/* Plarform independent */

// calculate the size and location of all alterations needed to relocate the 
// function. 
// normalized_alteration_set will contain the record of these changes
// Return the sum total of the sizes of the alterations

// oldInstr: buffer of instruction objects
// mutatee: address of actual function to be relocated (located in the mutatee)
// mutator: address of copy of the above function  (located in the mutator)

int pd_Function::findAlterations(const image *owner, 
                                 process *proc, 
                                 instruction *&oldInstr,
                                 LocalAlterationSet &normalized_alteration_set,
                                 Address &mutator, Address &mutatee) {

LocalAlterationSet temp_alteration_set(this);
Address baseAddress;
unsigned numberOfInstructions;
int totalSizeChange = 0;


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
    return false;
  }

  // get baseAddress if this is a shared object
  baseAddress = 0;
  if(!(proc->getBaseAddress(owner,baseAddress))){
    baseAddress = 0;
  }
  // address of function (in mutatee) 
  mutatee = baseAddress + getAddress(0);

  // create a buffer of instruction objects 
  if (!(loadCode(proc, oldInstr, numberOfInstructions, mutatee))) {
    return false;  
  }

  // address of copy of function (in mutator)
  mutator = addressOfMachineInsn(&oldInstr[0]);

#ifdef DEBUG_FUNC_RELOC
  cerr << " mutator = " << mutator << endl;
  cerr << " mutatee = " << mutatee << endl;
#endif

  // discover which instPoints need to be expaned  
  expandInstPoints(&temp_alteration_set, normalized_alteration_set, 
                   baseAddress, mutator, mutatee, oldInstr, 
                   numberOfInstructions); 

  // Check if the expansions discovered in expandInstPoints would require
  // further expansion of the function (this occurs if the target of a 
  // relative branch or call would be moved outside the range of the insn
  updateAlterations(&temp_alteration_set, normalized_alteration_set, oldInstr,
                    baseAddress, mutatee, totalSizeChange); 
 
#ifdef DEBUG_FUNC_RELOC
    cerr << " totalSizeChange = " << totalSizeChange << endl;
#endif

  return totalSizeChange;
}
  
  
/****************************************************************************/
/****************************************************************************/

/* Plarform independent */

// First expand the function, determining all LocalAlterations that need 
// to be applied, and then apply them, storing the expanded function in 
// the buffer NEW_CODE

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
	
  instruction *oldInstr = 0;
  Address mutator, mutatee;
  int totalSizeChange = 0;
  LocalAlterationSet normalized_alteration_set(this); 

  // assumes delete NULL ptr safe....
  oldInstr = NULL;

#ifdef DEBUG_FUNC_RELOC
    cerr << "pd_Function::findAndApplyAlterations called " << endl;
    cerr << " prettyName = " << prettyName().string_of() << endl;
    cerr << " size() = " << size() << endl;
    cerr << " this = " << *this << endl;
#endif

  // Find the alterations that need to be applied
  totalSizeChange = findAlterations(owner, proc, oldInstr,
                                    normalized_alteration_set,
                                    mutator, mutatee);  

    // make sure that new function version can fit in newInstr (statically allocated)....
    if (size() + totalSizeChange > NEW_INSTR_ARRAY_LEN) {
       cerr << "WARN : attempting to relocate function " \
       	    << prettyName().string_of() << " with size " \
	    << " > NEW_INSTR_ARRAY_LEN (" << NEW_INSTR_ARRAY_LEN \
	    << "), unable to instrument : " \
            << " size = " << size() << " , totalSizeChange = " \
	    << totalSizeChange << endl;
       delete []oldInstr;
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
        delete []oldInstr; 
        return false;
      }
      reloc_info = new relocatedFuncInfo(proc,newAdr);
      relocatedByProcess += reloc_info;
    }

    // Apply the alterations needed for relocation. The expanded function 
    // will be placed in NEW_CODE.
    if (!(applyAlterations(normalized_alteration_set, mutator, mutatee, 
                           newAdr, oldInstr, size(), newInstr))) {
      delete []oldInstr;
      return false;
    }


    // Fill reloc_info up with relocated (and altered by alterations) inst 
    // points. Do AFTER all alterations are attached AND applied....
    fillInRelocInstPoints(owner, proc, location, reloc_info, 
                          mutatee, mutator, oldInstr, newAdr, newInstr, 
                          normalized_alteration_set);

    
    size_change = totalSizeChange;  

    delete []oldInstr;
    return true;
}

/****************************************************************************/
/****************************************************************************/

// Calulate which instPoints need to be expanded to allow for instrumentation  

// numberOfInstructions: # of insn's in function (as opposed to # of bytes)
// temp_alteration_set: record of the needed expansions 

void pd_Function::expandInstPoints(LocalAlterationSet *temp_alteration_set, 
                                   LocalAlterationSet &normalized_alteration_set, 
                                   Address baseAddress, Address mutator,
                                   Address mutatee, instruction oldInstr[], 
                                   unsigned numberOfInstructions) {

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

  PA_attachGeneralRewrites(temp_alteration_set, baseAddress, 
                           mutatee, oldInstr, size());
  PA_attachOverlappingInstPoints(&tmp_alt_set1, baseAddress, 
                                 mutatee, oldInstr, size());
  PA_attachBranchOverlaps(&tmp_alt_set2, baseAddress, mutator, 
                          oldInstr, numberOfInstructions, size());

  // merge the LocalAlterations discovered in the above passes, placing
  // them in normalized_alteration_set 

  combineAlterationSets(temp_alteration_set, &tmp_alt_set1);
  combineAlterationSets(temp_alteration_set, &tmp_alt_set2);
  combineAlterationSets(&normalized_alteration_set, temp_alteration_set);    
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

  /* Platform independent */

// Find overlapping instrumentation points 

bool pd_Function::PA_attachOverlappingInstPoints(
                              LocalAlterationSet *temp_alteration_set, 
                              Address baseAddress, Address firstAddress,
                              instruction* /* loadedCode */, int /* codeSize */) {

  int overlap = 0, offset = 0;

#ifdef DEBUG_FUNC_RELOC
    cerr << "pd_Function::PA_attachOverlappingInstPoints called" <<endl;
#endif

    // create and sort vector of instPoints
    vector<instPoint*> foo;
    sorted_ips_vector(foo);

    // loop over all consecutive pairs of instPoints
    for (unsigned i=0;i<foo.size()-1;i++) {
        instPoint *this_inst_point = foo[i];
        instPoint *next_inst_point = foo[i+1];

        overlap = ((this_inst_point->iPgetAddress() + 
                    this_inst_point->sizeOfInstrumentation()) - 
                    next_inst_point->iPgetAddress()); 

        // check if inst point overlaps with next inst point
        if (overlap > 0) {
            offset = ((this_inst_point->iPgetAddress() + baseAddress) - firstAddress);

            // LocalAlteration inserting nops after this_inst_point
       	    InsertNops *nops = new InsertNops(this, offset, overlap);
	    temp_alteration_set->AddAlteration(nops);

#ifdef DEBUG_FUNC_RELOC
    cerr << " detected overlapping inst points: "  << endl;
    cerr << " adding LocalAlteration of size: " << overlap 
         << " at offset: " << offset << endl; 
#endif

	}
    }
    return true;
}

/****************************************************************************/
/****************************************************************************/

 /* Platform independent */

// Locate jumps with targets inside the footprint of an inst points. 

bool pd_Function::PA_attachBranchOverlaps(
                           LocalAlterationSet *temp_alteration_set, 
                           Address /* baseAddress */, Address firstAddress, 
                           instruction loadedCode[],
                           unsigned numberOfInstructions, int codeSize)  {

#ifdef DEBUG_FUNC_RELOC
    cerr << "pd_Function::PA_attachBranchOverlaps called" <<endl;
    cerr << " codeSize = " << codeSize << endl;
    cerr << " numberOfInstructions = " << numberOfInstructions << endl;
#endif

  int instr_address;
  int disp, offset;
  instruction instr;
   
  // create and sort vector of instPoints
  vector<instPoint*> foo;
  sorted_ips_vector(foo);

  // Iterate over function instruction by instruction....
  for(unsigned i = 0; i < numberOfInstructions; i++) {       
    instr = loadedCode[i];
    instr_address = addressOfMachineInsn(&instr);
     
#ifdef DEBUG_FUNC_RELOC
    cerr << " insn address = " << hex << (unsigned ) instr_address << endl;
    cerr << " insn offset = "  << instr_address - firstAddress << endl;
#endif
     
    // look for branch and call insns whose targets are inside the function.
    if (!branchInsideRange(instr, instr_address, firstAddress, 
                                        firstAddress + codeSize) &&
        !trueCallInsideRange(instr, instr_address, firstAddress, 
                                        firstAddress + codeSize)) {
      continue;
    } 

#ifdef DEBUG_FUNC_RELOC
    cerr << " branch at " << hex << (unsigned) instr_address 
         << " has target inside range of function" << endl;
#endif  

    disp = get_disp(&instr);

    // target of branch or call instruction 
    Address target = instr_address + disp;

    // Check if target is in the footprint of an inst point....
    instPoint *overlap = find_overlap(foo, target);
    if (overlap == NULL) continue;

    // offset of instruction from the beginning of the function 
    offset = overlap->address() - firstAddress;

    temp_alteration_set->iterReset();

    // If multiple jumps have their target address within the same 
    // instPoint, we only want to add nops once. To do this we
    // iterate over the known LocalAlterations, checking if any already 
    // are already planning on inserting nops at this instPoint.
    LocalAlteration *alteration = temp_alteration_set->iterNext();
    while (alteration != NULL && alteration->getOffset() < offset) {
      alteration = temp_alteration_set->iterNext();
    }

    if (alteration == NULL || alteration->getOffset() != offset) {
 
      int shift = overlap->followingAddress() - target;

      InsertNops *nops = new InsertNops(this, offset, shift);
      temp_alteration_set->AddAlteration(nops);

#ifdef DEBUG_FUNC_RELOC
   cerr << " detected overlap between branch target and inst point : offset "
        << target - firstAddress << " # bytes " 
        << overlap->firstAddress() - target << endl;
   cerr << " adding LocalAlteration" << endl;        
#endif

    }
  }
  return true;
}

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

// Check if an ExpandInstruction alteration has already been created 
// at an offset, with size, shift.
// This allows us to avoid adding a new ExpandInstruction LocalAlterations for
// an instruction each time we go through discoverAlterations. This prevents
// an infinite looping 

bool alreadyExpanded(int offset, int shift, LocalAlterationSet *alteration_set) {
  bool already_expanded;
  LocalAlteration *alteration = 0;

#ifdef DEBUG_FUNC_RELOC 
	  cerr << " Method alreadyExpanded called " << endl;
#endif  

  alteration_set->iterReset();

  // find the LocalAlteration at offset  
  do {
    alteration = alteration_set->iterNext();
  } while (alteration != NULL && alteration->getOffset() < offset);
  
  if ((alteration != NULL) && (alteration->getOffset() == offset)) {
 
    alteration = dynamic_cast<ExpandInstruction *> (alteration);

    if (alteration == NULL || alteration->getShift() > shift) {
      already_expanded = false;
    }
    else {
      already_expanded = true;
    }
  } else {
      already_expanded = false;
  }
  
  alteration_set->iterReset();
  return already_expanded;
}

/****************************************************************************/
/****************************************************************************/

    /* Platform independent */

// place an expanded copy of the original function in NEW_CODE 

// mutatee: first address of function in mutatee
// mutator: first address of copy of function in mutator
// newAdr: address in mutatee where function is to be relocated to
// codeSize: size of original, unexpanded function
// oldInstr: buffer of insn's corresponding to copy of function in mutator
// newInstr: buffer of insn's corresponding to expanded function located
//          in temporary buffer in mutator

bool pd_Function::applyAlterations(LocalAlterationSet &norm_alt_set,
				   Address mutator, Address mutatee, 
                                   Address newAdr, 
                                   instruction oldInstr[], 
				   unsigned codeSize, 
                                   instruction newInstr[]) {

  // offset of current insn from beginning of original function  
  Address oldOffset = 0;
  // offset of current insn from beginning of expanded function
  Address  newOffset = 0;
  // next alteration
  LocalAlteration *nextAlter = 0; 
  // address at which next alteration begins 
  Address nextAlterBegins;

  // offset of current insn into buffer of instructon objects 
  // (oldInstr and newInstr respectively)
  int oldInsnOffset = 0, newInsnOffset = 0;

  int newDisp = 0;

  // x86-specific, temporary buffer for expanded function
  unsigned char *code = 0; 
  unsigned codeOffset = 0;

#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0)
      code = NEW_CODE;
#else
      code = 0;
#endif
 
  Address mutator_before, mutator_after, newAdr_before, newAdr_after; 

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
      copyInstruction(newInstr[newInsnOffset], oldInstr[oldInsnOffset], 
                                                          codeOffset);
    
      // Sparc will need a version of copyInstruction that does this:
      //    newInstr[newInsnOffset] = oldInstr[oldInsnOffset];

      // mutatee + oldOffset = address in mutatee of current instruction
      // newAdr + newOffset = address in mutatee where current instruction is
      // to be relocated
      // update relative branches and calls to locations outside the function 
      relocateInstructionWithFunction(true,&newInstr[newInsnOffset], 
                                      mutatee + oldOffset, newAdr + newOffset,
                                      mutatee, codeSize);

      // mutatee + oldOffset = address in mutatee of current instruction
      // newInstr[newInsnOffset] = insn in temporary buffer to be patched up
      // update relative branches and calls to locations inside the function
      patchOffset(true, &norm_alt_set, newInstr[newInsnOffset], 
                  mutator + oldOffset, mutator, codeSize);

      // next instruction
      oldOffset += sizeOfMachineInsn(&oldInstr[oldInsnOffset]);
      newOffset += sizeOfMachineInsn(&oldInstr[oldInsnOffset]);
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
      if(isNearBranchInsn(oldInstr[oldInsnOffset])  || 
         isTrueCallInsn(oldInstr[oldInsnOffset])) {
        int oldDisp = get_disp(&oldInstr[oldInsnOffset]);
        int oldInsnSize = sizeOfMachineInsn(&oldInstr[oldInsnOffset]);  

        // mutator + oldOffset = address in mutator of current instruction
        // mutator + codeSize = last address of copy of function in mutator
        if ((branchInsideRange(oldInstr[oldInsnOffset], mutator + oldOffset, 
                               mutator, mutator + codeSize))   ||
            (trueCallInsideRange(oldInstr[oldInsnOffset], mutator + oldOffset, 
                                 mutator, mutator + codeSize))) {

          // updated disp for relative branch or call insn to target 
          // inside function
          newDisp = oldDisp + (norm_alt_set.getShift(oldOffset + oldInsnSize +
                               oldDisp) - norm_alt_set.getShift(oldOffset + oldInsnSize));

        } else {

            // updated disp for relative branch or call insn to target 
            // outside function
            int origAddr = (mutatee + oldOffset + oldInsnSize + oldDisp);
            int targetAddr = (newAdr + newOffset + oldInsnSize + 
                    set_disp(false, &oldInstr[oldInsnOffset], newDisp, true));

            newDisp = origAddr - targetAddr;
	} 

#ifdef DEBUG_FUNC_RELOC 
      cerr << " newDisp " << newDisp << " oldDisp " << oldDisp << endl;
#endif

      }

      // RewriteFootprint makes the appropriate alterations to the  
      // function at the instruction specified by the instPoint. 
      // Unfortunately RewriteFootprint may change certain values, 
      // so we have to make sure we fix any altered values
      // 

      // mutator_before: address of current insn in mutator  
      // mutator_after: address of next instruction to be dealt with
      //                (in mutator), after alteration has been applied

      // newAdr_before: address where current insn will be relocated to   
      // newAdr_after: address where next instruction will be relocated to 

      // oldInstr: buffer of old insn objects
      // newInstr: buffer of new insn objects being made
      // oldInsnOffset: offset of instruction object (in oldInstr) 
      //                corresponding to current insn
      // newInsnOffset: offset of instruction object (in newInstr) 
      //                corresponding to current insn

      mutator_after = mutator_before = mutator + oldOffset;
      newAdr_after = newAdr_before = newAdr + newOffset;

      nextAlter->RewriteFootprint(mutator, mutator_after, 
                                  newAdr, newAdr_after, 
                                  oldInstr, newInstr, oldInsnOffset,  
                                  newInsnOffset, newDisp, 
                                  codeOffset, code);

      // update offsets by the # of bytes RewriteFootprint walked over
      oldOffset += (mutator_after - mutator_before);
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
/* This will become platform specific */

// It may be that in two different passes over a function, we note 
// two different LocalAlterations at the same offset. This function 
// reconciles any conflict between the LocalAlterations and merges them into 
// a single LocalAlteration, or ignores one of them,

LocalAlteration *fixOverlappingAlterations(LocalAlteration *alteration, 
                                           LocalAlteration *tempAlteration) {

  LocalAlteration *casted_alteration = 0, *casted_tempAlteration = 0; 

#ifdef DEBUG_FUNC_RELOC 
	   cerr << "pd_Function::fixOverlappingAlterations " << endl;
#endif

  // assert that there is indeed a conflict  
  assert (alteration->getOffset() == tempAlteration->getOffset()); 

  casted_alteration = dynamic_cast<ExpandInstruction *> (alteration); 
  casted_tempAlteration = dynamic_cast<ExpandInstruction *> (tempAlteration);
  if (casted_alteration != NULL) {
    return alteration;
  } else {
      if (casted_tempAlteration != NULL) {
        return tempAlteration;
      }
  }  
  
  casted_alteration = dynamic_cast<InsertNops *> (alteration); 
  casted_tempAlteration = dynamic_cast<InsertNops *> (tempAlteration); 
  if (casted_alteration != NULL) {
    if (casted_tempAlteration != NULL) {
      if (alteration->getShift() >= tempAlteration->getShift()) {
        return alteration;
      } else {  
          return tempAlteration;
      }
    }
  }

  return NULL;
}

/****************************************************************************/
/****************************************************************************/

/* Platform independent */

// Combine two LocalAlterationSets, making sure the LocalAlterations
// remain in order from smallest to largest offset.
void combineAlterationSets(LocalAlterationSet *combined_alteration_set, 
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
           
            combined_alteration_set->AddAlteration(alteration);
  
            alteration = alteration_set->iterNext();
            tempAlteration = temp_alteration_set.iterNext();
        }
    }      
  }
  combined_alteration_set->iterReset();  
  combined_alteration_set->Collapse(); 
}

/****************************************************************************/
/****************************************************************************/

// find out if previous LocalAlterations have expanded the function in such
// a way as to require new LocalAlterations to be added.

bool pd_Function::updateAlterations(LocalAlterationSet *temp_alteration_set,
                                    LocalAlterationSet &normalized_alteration_set,
                                    instruction *oldInstr, 
                                    Address baseAddress,
                                    Address firstAddress,
                                    int &totalSizeChange) { 

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
      combineAlterationSets(&normalized_alteration_set, temp_alteration_set);

    }

    temp_alteration_set->Flush();

    // find new alterations that have come about, due to the expansion 
    // of the function. e.g. ExpandInstruction LocalAlterations 
    discoverAlterations(temp_alteration_set, normalized_alteration_set, 
                        baseAddress, firstAddress, oldInstr, size());

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
        if (fn != 0) ret += fn;
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

/* We are not currently executing in this function, so proceed with the relocation */


    Address baseAddress = 0;
    if(!(proc->getBaseAddress(location->owner(),baseAddress))){
      baseAddress = 0;
    }

    // original address of function (before relocation)
    u_int origAddress = baseAddress + getAddress(0);    
 
    // address to which function will be relocated.
    // memory is not allocated until total size change of function is known
    u_int ret = 0;

    if (!reloc_info) {
      // findAndApplyAlterations expands and updates the function, 
      // storing the expanded function with relocated addresses in NEW_CODE.
      if (findAndApplyAlterations(location->iPgetOwner(), location, ret, proc, 
                                  reloc_info, size_change)) {

#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0)

        // Copy the expanded and updated function into the mutatee
        proc->writeDataSpace((caddr_t)ret, size() + size_change,
                             (caddr_t) NEW_CODE);
#else
        proc->writeDataSpace((caddr_t)ret, size() + size_change,
                             (caddr_t) newInstr);
#endif

        // branch from original function to relocated function
        generateBranch(proc, origAddress, ret);
        reloc_info->setInstalled();

#ifdef DEBUG_FUNC_RELOC
        cerr << "pd_Function::relocateFunction " << endl;
        cerr << " prettyName = " << prettyName().string_of() << endl;      
        cerr << " relocated from " << hex << origAddress
	     << " with size " << size() << endl;
        cerr << " to " << hex << ret 
             << " with size " << size()+size_change << endl;
#endif

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
    fill_in += funcEntry_;

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
	    fill_in += calls[calls_idx++];
	} else {
	    fill_in += funcReturns[returns_idx++];
	}
    }
}

// modifyInstPoint: if the function associated with the process was 
// recently relocated, then the instPoint may have the old pre-relocated
// address (this can occur because we are getting instPoints in mdl routines 
// and passing these to routines that do the instrumentation, it would
// be better to let the routines that do the instrumenting find the points)

void pd_Function::modifyInstPoint(instPoint *&location, process *proc) {
    
    unsigned retId = 0, callId = 0;
    bool found = false;

    if(relocatable_ && !(location->getRelocated())){
        for(u_int i=0; i < relocatedByProcess.size(); i++){
           if((relocatedByProcess[i])->getProcess() == proc){
               if(location == funcEntry_){
                 instPoint *new_entry = const_cast<instPoint *>
                                        ((relocatedByProcess[i])->funcEntry());
                 location = new_entry;
               } 
               else {
                 for(retId=0;retId < funcReturns.size(); retId++) {
                    if(location == funcReturns[retId]){
                       const vector<instPoint *> new_returns = 
                          (relocatedByProcess[i])->funcReturns(); 
                       location = (new_returns[retId]);
                       found = true;
                       break;
		    }
		 }
                 if (found) break;
         
                 for(callId=0;callId < calls.size(); callId++) {
                    if(location == calls[callId]){
                       vector<instPoint *> new_calls = 
                          (relocatedByProcess[i])->funcCallSites(); 
                       location = (new_calls[callId]);
                       break;
                    }
		 }
               break;
	       }
	   }
	}
    }
}
















