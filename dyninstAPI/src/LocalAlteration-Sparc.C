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

// $Id: LocalAlteration-Sparc.C,v 1.5 2000/07/28 17:20:41 pcroth Exp $

#include "dyninstAPI/src/LocalAlteration-Sparc.h"
#include "dyninstAPI/src/LocalAlteration.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/FunctionExpansionRecord.h"
#include "common/h/Vector.h"
#include "dyninstAPI/src/dyninst.h"
#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/inst-sparc.h"
#include <assert.h>

//--------------------------------- SPARC SPECIFIC -----------------------------

extern void relocateInstruction(instruction *insn, 
                        Address origAddr, Address targetAddr, process *proc);

// constructor for SPARC LocalAlteration....
SparcLocalAlteration::SparcLocalAlteration
        (pd_Function *f, int beginning_offset,
	 int ending_offset, Address nBaseAddress) :
	  LocalAlteration(f, beginning_offset, ending_offset) {
    // set (protected data member) baseAddress
    baseAddress = nBaseAddress;


}

// constructor for NOPExpansion....
NOPExpansion::NOPExpansion(pd_Function *f, int 
			   beginning_offset, int ending_offset,
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

    int i, oldOffset;

    // silence warnings about param being unused....
    assert(oldInstr);

    // make sure implied offset from beginning of newInstr array is int....
    assert(  (  (newAdr - newBaseAdr) % sizeof(instruction)  ) == 0);
    i = (newAdr - newBaseAdr)/sizeof(instruction);

    // figure out offset of original instruction in oldInstr....
    assert(beginningOffset % sizeof(instruction) == 0);
    oldOffset = beginningOffset / sizeof(instruction);

    // generate save instruction to free up new stack frame for
    //  inserted call....
    // ALERT ALERT - -112 seems like apparently random number
    //  used in solaris specific code where save instructions 
    //  are generated.  Why -112?
    genImmInsn(&newInstr[i++], SAVEop3, REG_SPTR, -112, REG_SPTR);
  
    // Generate : mv %reg %g1.
    // On Sparc, mv %1 %2 is synthetic inst. implemented as orI %1, 0, %2....
    genImmInsn(&newInstr[i++], ORop3, oldInstr[oldOffset].rest.rs1, 0, 1);

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
//                                    add %rs1, reg_or_imm, %rd'
//   call  %reg                       mov %i0 %o0
//   restore %rs1, reg_or_imm, %rd    mov %i1 %o1
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
//                                    add %rs1, reg_or_imm, %rd'
//   call  PC_REL_ADDR                mov %i0 %o0
//   restore %rs1, reg_or_imm, %rd    mov %i1 %o1
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

    //
    // Generate instruction to capture effect of free add from restore op
    //  (the restore op's "side effect" - see Sparc V9 Arch Manual p. 214)
    //  In the (fairly common) case where the restore has no side effect 
    //  (the target of the restore if %g0), this will generate an add
    //  into %g0 - which is a wasted instruction.  This could be avoided
    //  (and the resulting rewritten code made 1 insn shorter) with some 
    //  extra logic in this class, but it doesn't seem worth the extra code
    //  complexity in this class - may want to add if many many functions 
    //  end up needing to be relocated.
    // Note that the semantics of the restore side effect are such that the 
    //  source operands (of the add) are in the register window BEFORE
    //  the restore takes effect, but the destination operand is in the 
    //  register window AFTER the restore takes effect.  Since we are 
    //  adding an explicit add in the original stack frame, can copy the 
    //  source operands directly from the original restore operation, but 
    //  convert the destination operand to account for the fact that it
    //  is still in the same stack frame....
    //

    // Adjust destination register :
    //  o registers -> i registers
    //  g registers -> stay g registers
    //  l registers & i registers ->  ???? W
    //   Would seem to discard the value - return false indicating don't
    //   know how to handle this case....
    Register restore_add_side_effect_destination = oldInstr[oldOffset + 1].resti.rd;
    if (restore_add_side_effect_destination >= REG_O(0) && 
	  restore_add_side_effect_destination <= REG_O(7)) {
        restore_add_side_effect_destination = REG_I(0) + 
	  restore_add_side_effect_destination - REG_O(0);
    } else if (restore_add_side_effect_destination <= REG_G(0) &&
		 restore_add_side_effect_destination <= REG_G(7)) {
        restore_add_side_effect_destination = restore_add_side_effect_destination;
    } else if ((restore_add_side_effect_destination <= REG_L(0) &&
		 restore_add_side_effect_destination <= REG_L(7)) ||
	       ((restore_add_side_effect_destination <= REG_I(0) &&
		 restore_add_side_effect_destination <= REG_I(7)))
	       ) {
        cerr << "WARN : function " << function->prettyName().string_of()
	     << " unable to unroll tail-call optimization in function,"
	     << " restore operation found with side effect which is not"
	     << " preserved across call-restore tail-call optimization "
	     << " unrolling : writing to i or l register" << endl;
        return false;
    } 

    // The dyninstAPI function called to construct the ADD insn is different
    //  based on whether the instruction uses a register and an immeadiate
    //  value or 2 registers as source operands - conceptually, either case
    //  just make an ADD insn with the same source operands as the restore
    //  and a destination which is "adjusted" as described above....
    if (oldInstr[oldOffset + 1].resti.i == 1) {
        genImmInsn(&newInstr[newOffset++], ADDop3, oldInstr[oldOffset + 1].resti.rs1,
		   oldInstr[oldOffset + 1].resti.simm13, 
		   restore_add_side_effect_destination);
    } else {
        genSimpleInsn(&newInstr[newOffset++], ADDop3, 
		    oldInstr[oldOffset + 1].rest.rs1,
		    oldInstr[oldOffset + 1].rest.rs2,
		    restore_add_side_effect_destination);
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
        // if original jmp/call instruction was call to a register, that
        //  register should have been pushed into %g0, so generate a call
        //  to %g0.
        //  generate <call %g1>
        generateJmplInsn(&newInstr[newOffset++], 1, 0, 15);
      
        // If were dealing with inst points in this code, would stick inst point 
	//  here on call site....
      
        // in the case of a jmpl call, 18 instructions are generated
        // and used to replace origional 2 (call, restore), resulting
        // in a new addition of 16 instructions....
        newAdr += 18 * sizeof(instruction); 
    } else {
        // if the original call was a call to an ADDRESS, then want
        //  to copy the original call.  There is, however, a potential
        //  caveat:  The sparc- CALL instruction is apparently PC
        //  relative (even though disassemblers like that in gdb will
        //  show a call to an absolute address).
        //  As such, want to change the call target to account for 
        //  the difference in PCs.
      
        newInstr[newOffset].raw = oldInstr[oldOffset].raw;
        relocateInstruction(&newInstr[newOffset++],
			    adr,
			    newAdr + 7 * sizeof(instruction),
			    NULL);
            
        // in the case of a "true" call, 17 instructions are generated
        //  + replace origional 2, resulting in addition of 15 instrs.
        newAdr += 17 * sizeof(instruction);  
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
        // call ADDR results in 15 extra instructions....
        fer->AddExpansion(beginningOffset, 15 * sizeof(instruction));
    }
    else if (jmpl_call) {
        // call %reg results in 16 extra instructions....
        fer->AddExpansion(beginningOffset, 16 * sizeof(instruction));
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
        ips->AddExpansion(beginningOffset, 7 * sizeof(instruction));
	ips->AddExpansion(beginningOffset + sizeof(instruction), 
			  7 * sizeof(instruction));
	ips->AddExpansion(beginningOffset + 2 * sizeof(instruction), sizeof(instruction));
    } else if (jmpl_call) {
        ips->AddExpansion(beginningOffset, 8 * sizeof(instruction));
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



