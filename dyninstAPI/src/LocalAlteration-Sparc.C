


#include "dyninstAPI/src/LocalAlteration-Sparc.h"
#include "dyninstAPI/src/LocalAlteration.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/FunctionExpansionRecord.h"
#include "util/h/Vector.h"
#include "dyninstAPI/src/dyninst.h"
#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/inst-sparc.h"
#include <assert.h>

//--------------------------------- SPARC SPECIFIC -----------------------------

// constructor for SPARC LocalAlteration....
SparcLocalAlteration::SparcLocalAlteration
        (pd_Function *f, int beginning_offset, \
	 int ending_offset, Address nBaseAddress) :
	  LocalAlteration(f, beginning_offset, ending_offset) {
    // set (protected data member) baseAddress
    baseAddress = nBaseAddress;


}

// constructor for NOPExpansion....
NOPExpansion::NOPExpansion(pd_Function *f, int 
			   beginning_offset, int ending_offset, \
                           Address nBaseAddress, int size) :
    SparcLocalAlteration(f, beginning_offset, ending_offset, 
			    nBaseAddress)
{
    // Size should correspond to integer # of nop instructions.... 
    assert((size % sizeof(instruction)) == 0);
    sizeNopRegion = size;

    // ending_offset should be same as beginning_offset (at least for nw)....
    // indicating that 0 origional instructions are overwritten by the nops....
    assert(beginningOffset == endingOffset);
}

// update branches :
//  Add extra offset to FunctionExpansionRecord to modify all branches
//  around.  Currently only setup so that footprint has size of 0
//  (in ORIGIONAL CODE) so branches into shouldn't happen, eh???? 
bool NOPExpansion::UpdateExpansions(FunctionExpansionRecord *fer) {
    assert(beginningOffset == endingOffset);
    fer->AddExpansion(beginningOffset, sizeNopRegion);
    return true;
}

// Update location of inst points in function.  In case of NOPExpansion, 
//  changes to inst points same as changes to branch targets....
bool NOPExpansion::UpdateInstPoints(FunctionExpansionRecord *ips) {
    return UpdateExpansions(ips);
}

// rewrite footprint - here simple, just add nops....
bool NOPExpansion::RewriteFootprint(Address &adr, Address newBaseAdr, \
        Address &newAdr, instruction oldInstr[], instruction newInstr[]) {
    int i;

    // silence warnings about param being unused....
    assert(oldInstr);

    // make sure implied offset from beginning of newInstr array is int....
    assert(((newAdr - newBaseAdr) % sizeof(instruction)) == 0);
    int new_offset = (newAdr - newBaseAdr)/sizeof(instruction);
    
    // make sure # of new nops is int....
    assert((sizeNopRegion % sizeof(instruction)) == 0);
    int num_nops = sizeNopRegion/sizeof(instruction);

    // write that many nops to newInstr....
    for (i=0;i<num_nops;i++) {
        generateNOOP(&newInstr[new_offset + i]);
    }

    // adr is updated by difference between beginning and ending offset
    //  (the # of bytes of origional code overwritten by nops)....
    adr += (endingOffset - beginningOffset); 
    // newAdr is updated by Size....
    newAdr += sizeNopRegion;

    return true;
}

TailCallOptimization::TailCallOptimization(pd_Function *f, 
        int beginning_offset, int ending_offset, Address nBaseAddress) :
        SparcLocalAlteration(f, beginning_offset, ending_offset, nBaseAddress)
{

}

//
//  (SPARC-SPECIFIC) PEEPHOLE ALTERATIONS FOR UNWINDING TAIL-CALL OPTIMIZATION
//
JmpNopTailCallOptimization::JmpNopTailCallOptimization(pd_Function *f, 
        int beginning_offset, int ending_offset, Address nBaseAddress) :
  TailCallOptimization(f, beginning_offset, ending_offset, nBaseAddress) {

}

//
//   before:          --->             after
// ---------------------------------------------------
//                                    save  %sp, -96, %sp
//                                    mov %reg %g1
//   jmp  %reg                        mov %i0 %o0
//   nop                              mov %i1 %o1
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
bool JmpNopTailCallOptimization::RewriteFootprint(Address &adr, Address newBaseAdr, 
     Address &newAdr, instruction oldInstr[], instruction newInstr[]) {

    int i;
    instruction instr;

    // silence warnings about param being unused....
    assert(oldInstr);

    // make sure implied offset from beginning of newInstr array is int....
    assert(  (  (newAdr - newBaseAdr) % sizeof(instruction)  ) == 0);
    i = (newAdr - newBaseAdr)/sizeof(instruction);

    // generate save instruction to free up new stack frame for
    //  inserted call....
    // ALERT ALERT - -112 seems like apparently random number
    //  used in solaris specific code where save instructions 
    //  are generated.  Why -112?
    genImmInsn(&newInstr[i++], SAVEop3, REG_SP, -112, REG_SP);
  
    // Generate : mv %reg %g1.
    // On Sparc, mv %1 %2 is synthetic inst. implemented as orI %1, 0, %2....
    genImmInsn(&newInstr[i++], ORop3, instr.rest.rs1, 0, 1);

    // generate mov i0 ... i5 ==> O0 ... 05 instructions....
    // as noted above, mv inst %1 %2 is synthetic inst. implemented as
    //  orI %1, 0, %2  
    genImmInsn(&newInstr[i++], ORop3, REG_I(0), 0, REG_O(0));
    genImmInsn(&newInstr[i++], ORop3, REG_I(1), 0, REG_O(1));
    genImmInsn(&newInstr[i++], ORop3, REG_I(2), 0, REG_O(2));
    genImmInsn(&newInstr[i++], ORop3, REG_I(3), 0, REG_O(3));
    genImmInsn(&newInstr[i++], ORop3, REG_I(4), 0, REG_O(4));
    genImmInsn(&newInstr[i++], ORop3, REG_I(5), 0, REG_O(5));

    // if origional jmp/call instruction was call to a register, that
    //  register should have been pushed into g0, so generate a call
    //  to %g0.
    //  generate <call %g1>
    generateJmplInsn(&newInstr[i], 1, 0, 15);
    // The inst-point at the origional jmp should be moved to that call insn....
    i++;    

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

    //
    // And modify inout params....
    //
    // adr incremented to end of footpirnt....
    adr += (endingOffset - beginningOffset);
    // newAdr incremented by # of instructions written (curr 18)....
    newAdr += 18 * sizeof(instruction);

    return true;
}

// update expansion record based on local re-write....
// Origional sequence:
//    jmp %reg
//    nop
//  A jump to (before) the jmp should go to before the new save....
//  A jump to (before) the nop should go to before the new ret, restore....
//   ==> Assume that never happens in code below....
//  A jump around the footprint should get an extra offset of 16 (change in size)....
bool JmpNopTailCallOptimization::UpdateExpansions(FunctionExpansionRecord *fer) {
    // beginningOffset should fall on instruction word boundary.... 
    assert((beginningOffset % sizeof(instruction)) == 0);
    //  A jump to (before) the jmp should go to before the new save (0 extra offset)....
    //  A jump to (before) the nop should go to after the new ret, retsore (17 extra offset)....
    //   ==> code currently assumes this never happens (why branch to a nop)....
    // Jumps that go around the region get an extra offset of the size change 
    //  (old: 2 instructions, new : 18 instructions)....
    fer->AddExpansion(beginningOffset + sizeof(instruction), 16 * sizeof(instruction));
    return true;
}

//  An inst point previously located at the jmp should be located at the call
//   call point ONLY....
//  An inst point previously located at the nop should be located at the restore
//   return point ONLY.... 
bool JmpNopTailCallOptimization::UpdateInstPoints(FunctionExpansionRecord *ips) {
    // An inst point previously located at the (old) jmp should be moved to the 
    //  (new) call.
    ips->AddExpansion(beginningOffset, 8 * sizeof(instruction));
    // An inst point previously located at the (old) nop should be moved to thw
    //  (new ret)
    ips->AddExpansion(beginningOffset + sizeof(instruction), 7 * sizeof(instruction));
    // One more insn of offset is added to make the total # of bytes of offset agree
    //  with the size change....
    ips->AddExpansion(beginningOffset + 2 * sizeof(instruction), sizeof(instruction));
    return true;
} 

int fubar() {
    return 1;
}
    
// constructor for CallRestoreTailCallOptimization....
CallRestoreTailCallOptimization::CallRestoreTailCallOptimization(pd_Function *f, 
        int beginning_offset, int ending_offset, Address nBaseAddress, 
	instruction callInsn) :
  TailCallOptimization(f, beginning_offset, ending_offset, nBaseAddress)
{
    SetCallType(callInsn);
}

// rewrite code to unwind CallRestoreTailCallOptimization....
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
void relocateInstruction(instruction *insn, u_int origAddr, u_int targetAddr,
			 process *proc);
bool CallRestoreTailCallOptimization::RewriteFootprint(Address &adr, 
        Address newBaseAdr, Address &newAdr, instruction oldInstr[], 
        instruction newInstr[]) {

    assert(jmpl_call || true_call);

    int oldOffset = beginningOffset/sizeof(instruction);
    int newOffset = (newAdr - newBaseAdr)/sizeof(instruction);

    // if the call instruction was a call to a register, stick in extra
    //  initial mov as above....
    if (jmpl_call) {
        // added extra mv *, g1
        // translation : mv inst %1 %2 is synthetic inst. implemented as
        //  orI %1, 0, %2  
        genImmInsn(&newInstr[newOffset++], ORop3, oldInstr[oldOffset].rest.rs1, 0, 1);
    }

    // generate mov i0 ... i5 ==> O0 ... 05 instructions....
    // as noted above, mv inst %1 %2 is synthetic inst. implemented as
    //  orI %1, 0, %2  
    genImmInsn(&newInstr[newOffset++], ORop3, REG_I(0), 0, REG_O(0));
    genImmInsn(&newInstr[newOffset++], ORop3, REG_I(1), 0, REG_O(1));
    genImmInsn(&newInstr[newOffset++], ORop3, REG_I(2), 0, REG_O(2));
    genImmInsn(&newInstr[newOffset++], ORop3, REG_I(3), 0, REG_O(3));
    genImmInsn(&newInstr[newOffset++], ORop3, REG_I(4), 0, REG_O(4));
    genImmInsn(&newInstr[newOffset++], ORop3, REG_I(5), 0, REG_O(5));

    if (jmpl_call) { 
        // if origional jmp/call instruction was call to a register, that
        //  register should have been pushed into g0, so generate a call
        //  to %g0.
        //  generate <call %g1>
        generateJmplInsn(&newInstr[newOffset++], 1, 0, 15);
      
        // If were dealing with inst points in this code, would stick inst point 
	//  here on call site....
      
        // in the case of a jmpl call, 17 instructions are generated
        // and used to replace origional 2 (call, restore), resulting
        // in a new addition of 15 instructions....
        newAdr += 17 * sizeof(instruction); 
    } else {
        // if the origional call was a call to an ADDRESS, then want
        //  to copy the origional call.  There is, however, a potential
        //  caveat:  The sparc- CALL instruction is apparently PC
        //  relative (even though disassemblers like that in gdb will
        //  show a call to an absolute address).
        //  As such, want to change the call target to account for 
        //  the difference in PCs.
      
        newInstr[newOffset].raw = oldInstr[oldOffset].raw;
        relocateInstruction(&newInstr[newOffset++],
			    adr,
			    newAdr + 6 * sizeof(instruction), \
			    NULL);
            
        // in the case of a "true" call, 16 instructions are generated
        //  + replace origional 2, resulting in addition of 18 instrs.
        // Addition of 14 total should make newAdr point to
        //  the retl instruction 2nd to last in unwound tail-call 
        //  opt. sequence....
        newAdr += 16 * sizeof(instruction);  
    }
 
    // generate NOP following call instruction (for delay slot)
    //  ....
    generateNOOP(&newInstr[newOffset++]);
    
    // generate mov instructions moving %o0 ... %o5 ==> 
    //     %i0 ... %i5.  
    genImmInsn(&newInstr[newOffset++], ORop3, REG_O(0), 0, REG_I(0));
    genImmInsn(&newInstr[newOffset++], ORop3, REG_O(1), 0, REG_I(1));
    genImmInsn(&newInstr[newOffset++], ORop3, REG_O(2), 0, REG_I(2));
    genImmInsn(&newInstr[newOffset++], ORop3, REG_O(3), 0, REG_I(3));
    genImmInsn(&newInstr[newOffset++], ORop3, REG_O(4), 0, REG_I(4));
    genImmInsn(&newInstr[newOffset++], ORop3, REG_O(5), 0, REG_I(5));
    
    // generate ret instruction
    generateJmplInsn(&newInstr[newOffset++], REG_I(7), 8 ,0);
    
    // generate restore operation....
    genSimpleInsn(&newInstr[newOffset++], RESTOREop3, 0, 0, 0);

    // alteration covers 2 instructions in original code....
    adr += 2 * sizeof(instruction);

    return true;
}

// used internally to set true_call and jmpl_call based on exact call insn
//  type...
void CallRestoreTailCallOptimization::SetCallType(instruction callInsn) { 
    // was call insn call %reg or call ADDR....
    true_call = isTrueCallInsn(callInsn);
    jmpl_call = isJmplCallInsn(callInsn); 
    assert(jmpl_call || true_call);
}

// update expansions:
//  Assumes no branches directly to restore op (in tc opt....)....
bool CallRestoreTailCallOptimization::UpdateExpansions(FunctionExpansionRecord *fer) {
    assert(jmpl_call || true_call);
    if (true_call) {
        // call ADDR results in 14 extra instructions....
        fer->AddExpansion(beginningOffset, 14 * sizeof(instruction));
    }
    else if (jmpl_call) {
        // call %reg results in 15 extra instructions....
        fer->AddExpansion(beginningOffset, 15 * sizeof(instruction));
    }
    return true;
}

// update inst points:
//  OLD                     NEW
//   before call             before call (offset same)
//   before restore          before ret
//   after restore           after restore
bool CallRestoreTailCallOptimization::UpdateInstPoints(FunctionExpansionRecord *ips) {
    assert(jmpl_call || true_call);
    if (true_call) {
        ips->AddExpansion(beginningOffset, 6 * sizeof(instruction));
	ips->AddExpansion(beginningOffset + sizeof(instruction), 
			  7 * sizeof(instruction));
	ips->AddExpansion(beginningOffset + 2 * sizeof(instruction), sizeof(instruction));
    } else if (jmpl_call) {
        ips->AddExpansion(beginningOffset, 7 * sizeof(instruction));
        ips->AddExpansion(beginningOffset + sizeof(instruction), 7 * sizeof(instruction));
	ips->AddExpansion(beginningOffset + 2 * sizeof(instruction), sizeof(instruction));
    } 
    return true;
}

//
//  CODE FOR Set07 CLASS....
//
SetO7::SetO7(pd_Function *f, int beginning_offset, int ending_offset, 
	     Address nBaseAddress) :
    SparcLocalAlteration(f, beginning_offset, ending_offset, nBaseAddress)
{
    //  NOTHING EXTRA BEYOND SPARCLocalAlteration CONSTR....
}

//
// Origional Sequence:
//  call PC + 8
// becomes
//  sethi
//  or
// Branches to call go to sethi, branches to anything after get bumped forward
//  by 1 slot....
bool SetO7::UpdateExpansions(FunctionExpansionRecord *fer) {
    assert(fer);
    fer->AddExpansion(beginningOffset + sizeof(instruction), sizeof(instruction));
    return true;
}

// Inst points previously located at call should stay in same place, and those
//  located after call get bumped up by 1 instruction....
bool SetO7::UpdateInstPoints(FunctionExpansionRecord *ips) {
    assert(ips);
    ips->AddExpansion(beginningOffset + sizeof(instruction), sizeof(instruction));
    return true;
}

// Parameters:
//  adr is address at which original (07 setting) call was found....
//  newBaseAdr is address at which relocated version of function will
//   begin....
//  generates sequence like:
//    sethi %07, high 22 bits of adr (also zeros low order 10 bits)
//    or %07, low order 10 bits of adr, %07
bool SetO7::RewriteFootprint(Address &adr, Address newBaseAdr, \
			     Address &newAdr, instruction oldInstr[], 
			     instruction newInstr[]) {
    assert(oldInstr);

    // offset into newInstr 
    int newOffset = (newAdr - newBaseAdr)/sizeof(instruction);
    // write 
    //  sethi %07, HIGH_22_BITS(adr) 
    // into newInstr array....
    generateSetHi(&newInstr[newOffset], adr, REG_O(7));
    // write 
    //  or, %07, low order 10 bits of adr, %07
    genImmInsn(&newInstr[newOffset+1], ORop3, REG_O(7), LOW10(adr), REG_O(7));
 
    //  alteration covers original call instruction....
    adr += sizeof(instruction);

    //  and writes 2 new instructions....
    newAdr += 2 * sizeof(instruction);
    return true;
}



