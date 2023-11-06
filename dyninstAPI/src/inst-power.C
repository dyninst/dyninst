/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * inst-power.C - Identify instrumentation points for a RS6000/PowerPCs
 * $Id: inst-power.C,v 1.291 2008/06/19 22:13:42 jaw Exp $
 */

#include "common/src/headers.h"
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
#include "dyninstAPI/src/image.h"
#include "dyninstAPI/src/dynProcess.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/inst-power.h"
#include "common/src/arch.h"
#include "dyninstAPI/src/codegen.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "common/src/stats.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/instPoint.h" // class instPoint
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/h/BPatch.h"
#include "dyninstAPI/src/BPatch_collections.h"
#include "dyninstAPI/src/registerSpace.h"
#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/mapped_object.h"

#include "parseAPI/h/CFG.h"

#include "emitter.h"
#include "emit-power.h"

#include <sstream>

#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"

extern bool isPowerOf2(int value, int &result);

#define DISTANCE(x,y)   ((x<y) ? (y-x) : (x-y))

Address getMaxBranch() {
  return MAX_BRANCH;
}

const char *registerNames[] = { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
			"r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
			"r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
			"r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31"};

std::unordered_map<std::string, unsigned> funcFrequencyTable;

void initDefaultPointFrequencyTable()
{
#ifdef notdef
    funcFrequencyTable[EXIT_NAME] = 1;

    FILE *fp;
    float value;
    char name[512];

    funcFrequencyTable["main"] = 1;
    funcFrequencyTable["DYNINSTsampleValues"] = 1;
    // try to read file.
    fp = fopen("freq.input", "r");
    if (!fp) {
	bperr("no freq.input file\n");
	return;
    }
    while (!feof(fp)) {
	fscanf(fp, "%s %f\n", name, &value);
	funcFrequencyTable[name] = (int) value;
	bperr("adding %s %f\n", name, value);
    }
    fclose(fp);
#endif
}

Register floatingLiveRegList[] = {13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
unsigned int floatingLiveRegListSize = 14;

// Note that while we have register definitions for r13..r31, we only
// use r0..r12 (well, r3..r12). I believe this is to reduce the number
// of saves that we execute. 


void registerSpace::initialize32() {
    static bool done = false;
    if (done) return;
    done = true;

    std::vector<registerSlot *> registers;

    // At ABI boundary: R0 and R12 are dead, others are live.
    // Also, define registers in reverse order - it helps with
    // function calls
    
    registers.push_back(new registerSlot(r12,
                                         "r12",
                                         false,
                                         registerSlot::deadABI,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r11,
                                         "r11",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r10,
                                         "r10",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r9,
                                         "r9",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r8,
                                         "r8",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r7,
                                         "r7",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r6,
                                         "r6",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r5,
                                         "r5",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r4,
                                         "r4",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r3,
                                         "r3",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));

    // Everyone else
    for (unsigned i = r13; i <= r31; ++i) {
      char name[32];
      sprintf(name, "r%u", i-r0);
      registers.push_back(new registerSlot(i, name,
					   false, 
					   registerSlot::liveAlways,
					   registerSlot::GPR));
    }

    /// Aaaand the off-limits ones.

    registers.push_back(new registerSlot(r0,
                                         "r0",
                                         true, // Don't use r0 - it has all sorts
                                         // of implicit behavior.
                                         registerSlot::deadABI,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r1,
                                         "r1",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r2,
                                         "r2",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));

    for (unsigned i = fpr0; i <= fpr13; i++) {
        char buf[128];
        sprintf(buf, "fpr%u", i - fpr0);
        registers.push_back(new registerSlot(i,
                                             buf,
                                             false,
                                             registerSlot::liveAlways,
                                             registerSlot::FPR));
    }
    registers.push_back(new registerSlot(xer,
                                         "xer",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
 
    registers.push_back(new registerSlot(lr,
                                         "lr",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(cr,
                                         "cr",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(ctr,
                                         "ctr",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(mq,
                                         "mq",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registerSpace::createRegisterSpace(registers);

    // Initialize the sets that encode which registers
    // are used/defined by calls, returns, and syscalls. 
    // These all assume the ABI, of course. 

    // TODO: Linux/PPC needs these set as well.
    

}

void registerSpace::initialize64() {
    static bool done = false;
    if (done) return;
    done = true;

    std::vector<registerSlot *> registers;

    // At ABI boundary: R0 and R12 are dead, others are live.
    // Also, define registers in reverse order - it helps with
    // function calls
    
    registers.push_back(new registerSlot(r12,
                                         "r12",
                                         false,
                                         registerSlot::deadABI,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r11,
                                         "r11",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r10,
                                         "r10",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r9,
                                         "r9",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r8,
                                         "r8",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r7,
                                         "r7",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r6,
                                         "r6",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r5,
                                         "r5",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r4,
                                         "r4",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r3,
                                         "r3",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));


    // Everyone else
    for (unsigned i = r13; i <= r31; ++i) {
      char name[32];
      sprintf(name, "r%u", i-r0);
      registers.push_back(new registerSlot(i, name,
					   false, 
					   registerSlot::liveAlways,
					   registerSlot::GPR));
    }


    /// Aaaand the off-limits ones.

    registers.push_back(new registerSlot(r0,
                                         "r0",
                                         true, // Don't use r0 - it has all sorts
                                         // of implicit behavior.
                                         registerSlot::deadABI,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r1,
                                         "r1",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r2,
                                         "r2",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));

    for (unsigned i = fpr0; i <= fpr13; i++) {
        char buf[128];
        sprintf(buf, "fpr%u", i - fpr0);
        registers.push_back(new registerSlot(i,
                                             buf,
                                             false,
                                             registerSlot::liveAlways,
                                             registerSlot::FPR));
    }
    registers.push_back(new registerSlot(xer,
                                         "xer",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(lr,
                                         "lr",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(cr,
                                         "cr",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(ctr,
                                         "ctr",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(mq,
                                         "mq",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registerSpace::createRegisterSpace64(registers);

    // Initialize the sets that encode which registers
    // are used/defined by calls, returns, and syscalls. 
    // These all assume the ABI, of course. 

    // TODO: Linux/PPC needs these set as well.
    

}

void registerSpace::initialize() {
    initialize32();
    initialize64();
}

unsigned registerSpace::SPR(Register x) {
    // Encodings from architecture manual
    switch ((powerRegisters_t) x) {
    case xer:
        return SPR_XER;
        break;
    case lr:
        return SPR_LR;
        break;
    case ctr:
        return SPR_CTR;
        break;
    case mq:
        return SPR_MQ;
        break;
    case cr:
        fprintf(stderr, "Error: condition register has no encoding!\n");
        return Null_Register;
        break;
    default:
        assert(0);
        return Null_Register;
        break;
    }
}
            

/*
 * Saving and restoring registers
 * We create a new stack frame in the base tramp and save registers
 * above it. Currently, the plan is this:
 *          < 220 bytes as per system spec      > + 4 for 64-bit alignment
 *          < 14 GPR slots @ 4 bytes each       >
 *          < 14 FPR slots @ 8 bytes each       >
 *          < 6 SPR slots @ 4 bytes each        >
 *          < 1 FP SPR slot @ 8 bytes           >
 *          < Space to save live regs at func call >
 *          < Func call overflow area, 32 bytes > 
 *          < Linkage area, 24 bytes            >
 *
 * Of course, change all the 4's to 8's for 64-bit mode.
 */

    ////////////////////////////////////////////////////////////////////
    //Generates instructions to save a special purpose register onto
    //the stack.
    //  Returns the number of bytes needed to store the generated
    //    instructions.
    //  The instruction storage pointer is advanced the number of 
    //    instructions generated.
    //
// NOTE: the bit layout of the mfspr instruction is as follows:
// opcode:6 ; RT: 5 ; SPR: 10 ; const 339:10 ; Rc: 1
// However, the two 5-bit halves of the SPR field are reversed
// so just using the xfxform will not work
void saveSPR(codeGen &gen,     //Instruction storage pointer
             Register    scratchReg, //Scratch register
             int         sprnum,     //SPR number
             int         stkOffset) //Offset from stack pointer
{
    instruction insn;

    // mfspr:  mflr scratchReg
    insn.clear();
    XFORM_OP_SET(insn, EXTop);
    XFORM_RT_SET(insn, scratchReg);
    XFORM_RA_SET(insn, sprnum & 0x1f);
    XFORM_RB_SET(insn, (sprnum >> 5) & 0x1f);
    XFORM_XO_SET(insn, MFSPRxop);
    insnCodeGen::generate(gen,insn);

    if (gen.width() == 4) {
	insnCodeGen::generateImm(gen, STop,
                                 scratchReg, REG_SP, stkOffset);
    } else /* gen.width() == 8 */ {
	insnCodeGen::generateMemAccess64(gen, STDop, STDxop,
                                         scratchReg, REG_SP, stkOffset);
    }
}

    ////////////////////////////////////////////////////////////////////
    //Generates instructions to restore a special purpose register from
    //the stack.
    //  Returns the number of bytes needed to store the generated
    //    instructions.
    //  The instruction storage pointer is advanced the number of 
    //    instructions generated.
    //
void restoreSPR(codeGen &gen,       //Instruction storage pointer
                Register      scratchReg, //Scratch register
                int           sprnum,     //SPR number
                int           stkOffset)  //Offset from stack pointer
{
    if (gen.width() == 4) {
        insnCodeGen::generateImm(gen, Lop,
                                 scratchReg, REG_SP, stkOffset);
    } else /* gen.width() == 8 */ {
        insnCodeGen::generateMemAccess64(gen, LDop, LDxop,
                                         scratchReg, REG_SP, stkOffset);
    }

    instruction insn;
    insn.clear();

    //mtspr:  mtlr scratchReg
    XFORM_OP_SET(insn, EXTop);
    XFORM_RT_SET(insn, scratchReg);
    XFORM_RA_SET(insn, sprnum & 0x1f);
    XFORM_RB_SET(insn, (sprnum >> 5) & 0x1f);
    XFORM_XO_SET(insn, MTSPRxop);
    insnCodeGen::generate(gen,insn);
}

           ////////////////////////////////////////////////////////////////////
	   //Generates instructions to save link register onto stack.
	   //  Returns the number of bytes needed to store the generated
	   //    instructions.
	   //  The instruction storage pointer is advanced the number of 
	   //    instructions generated.
void saveLR(codeGen &gen,       //Instruction storage pointer
            Register      scratchReg, //Scratch register
            int           stkOffset)  //Offset from stack pointer
{
    saveSPR(gen, scratchReg, SPR_LR, stkOffset);
    gen.rs()->markSavedRegister(registerSpace::lr, stkOffset);
}

           ////////////////////////////////////////////////////////////////////
           //Generates instructions to restore link register from stack.
           //  Returns the number of bytes needed to store the generated
	   //    instructions.
	   //  The instruction storage pointer is advanced the number of 
	   //    instructions generated.
	   //
void restoreLR(codeGen &gen,       //Instruction storage pointer
               Register      scratchReg, //Scratch register
               int           stkOffset)  //Offset from stack pointer
{
    restoreSPR(gen, scratchReg, SPR_LR, stkOffset);
}

           ////////////////////////////////////////////////////////////////////
           //Generates instructions to place a given value into link register.
	   //  The entire instruction sequence consists of the generated
	   //    instructions followed by a given (tail) instruction.
	   //  Returns the number of bytes needed to store the entire
	   //    instruction sequence.
	   //  The instruction storage pointer is advanced the number of 
	   //    instructions in the sequence.
	   //
void setBRL(codeGen &gen,        //Instruction storage pointer
            Register      scratchReg,  //Scratch register
            long          val,         //Value to set link register to
            instruction   ti)          //Tail instruction
{
    insnCodeGen::loadImmIntoReg(gen, scratchReg, val);

    instruction insn;

    //mtspr:  mtlr scratchReg
    insn.clear();
    XFORM_OP_SET(insn, EXTop);
    XFORM_RT_SET(insn, scratchReg);
    XFORM_RA_SET(insn, SPR_LR);
    XFORM_XO_SET(insn, MTSPRxop);
    insnCodeGen::generate(gen,insn);

    insn = ti;
    insnCodeGen::generate(gen,insn);
}

     //////////////////////////////////////////////////////////////////////////
     //Writes out instructions to place a value into the link register.
     //  If val == 0, then the instruction sequence is followed by a `nop'.
     //  If val != 0, then the instruction sequence is followed by a `brl'.
     //
void resetBRL(AddressSpace  *p,   //Process to write instructions into
	      Address   loc, //Address in process to write into
	      unsigned  val) //Value to set link register
{
    codeGen gen(10*instruction::size());
    Register scratch = 10;
    if (val) {
        setBRL(gen, scratch, val, instruction(BRLraw));
    }
    else {
        setBRL(gen, scratch, val, instruction(NOOPraw));
    }
    p->writeTextSpace((void *)loc, gen.used(), gen.start_ptr());
}

    /////////////////////////////////////////////////////////////////////////
    //Generates instructions to save the condition codes register onto stack.
    //  Returns the number of bytes needed to store the generated
    //    instructions.
    //  The instruction storage pointer is advanced the number of 
    //    instructions generated.
    //
void saveCR(codeGen &gen,       //Instruction storage pointer
            Register      scratchReg, //Scratch register
            int           stkOffset)  //Offset from stack pointer
{
    instruction insn;

    //mfcr:  mflr scratchReg
    insn.clear();
    XFXFORM_OP_SET(insn, EXTop);
    XFXFORM_RT_SET(insn, scratchReg);
    XFXFORM_XO_SET(insn, MFCRxop);
    insnCodeGen::generate(gen,insn);

    if (gen.width() == 4) {
        insnCodeGen::generateImm(gen, STop,
                                 scratchReg, REG_SP, stkOffset);
    } else /* gen.width() == 8 */ {
        insnCodeGen::generateMemAccess64(gen, STDop, STDxop,
                                         scratchReg, REG_SP, stkOffset);
    }
}

    ///////////////////////////////////////////////////////////////////////////
    //Generates instructions to restore the condition codes register from stack.
    //  Returns the number of bytes needed to store the generated
    //    instructions.
    //  The instruction storage pointer is advanced the number of 
    //    instructions generated.
    //
void restoreCR(codeGen &gen,       //Instruction storage pointer
               Register      scratchReg, //Scratch register
               int           stkOffset)  //Offset from stack pointer
{
    instruction insn;

    if (gen.width() == 4) {
        insnCodeGen::generateImm(gen, Lop,
                                 scratchReg, REG_SP, stkOffset);
    } else /* gen.width() == 8 */ {
        insnCodeGen::generateMemAccess64(gen, LDop, LDxop,
                                         scratchReg, REG_SP, stkOffset);
    }

    //mtcrf:  scratchReg
    insn.clear();
    XFXFORM_OP_SET(insn, EXTop);
    XFXFORM_RT_SET(insn, scratchReg);
    XFXFORM_SPR_SET(insn, 0xff << 1);
    XFXFORM_XO_SET(insn, MTCRFxop);
    insnCodeGen::generate(gen,insn);
}

    /////////////////////////////////////////////////////////////////////////
    //Generates instructions to save the floating point status and control
    //register on the stack.
    //  Returns the number of bytes needed to store the generated
    //    instructions.
    //  The instruction storage pointer is advanced the number of 
    //    instructions generated.
    //
void saveFPSCR(codeGen &gen,       //Instruction storage pointer
               Register      scratchReg, //Scratch fp register
               int           stkOffset)  //Offset from stack pointer
{
    instruction mffs;

    //mffs scratchReg
    mffs.clear();
    XFORM_OP_SET(mffs, X_FP_EXTENDEDop);
    XFORM_RT_SET(mffs, scratchReg);
    XFORM_XO_SET(mffs, MFFSxop);
    insnCodeGen::generate(gen,mffs);

    //st:     st scratchReg, stkOffset(r1)
    insnCodeGen::generateImm(gen, STFDop, scratchReg, REG_SP, stkOffset);
}

    ///////////////////////////////////////////////////////////////////////////
    //Generates instructions to restore the floating point status and control
    //register from the stack.
    //  Returns the number of bytes needed to store the generated
    //    instructions.
    //  The instruction storage pointer is advanced the number of 
    //    instructions generated.
    //
void restoreFPSCR(codeGen &gen,       //Instruction storage pointer
                  Register      scratchReg, //Scratch fp register
                  int           stkOffset)  //Offset from stack pointer
{
    insnCodeGen::generateImm(gen, LFDop, scratchReg, REG_SP, stkOffset);

    instruction mtfsf;

    //mtfsf:  scratchReg
    mtfsf.clear();
    XFLFORM_OP_SET(mtfsf, X_FP_EXTENDEDop);
    XFLFORM_FLM_SET(mtfsf, 0xff);
    XFLFORM_FRB_SET(mtfsf, scratchReg);
    XFLFORM_XO_SET(mtfsf, MTFSFxop);
    insnCodeGen::generate(gen,mtfsf);
}

     //////////////////////////////////////////////////////////////////////////
     //Writes out a `br' instruction
     //
void resetBR(AddressSpace  *p,    //Process to write instruction into
	     Address   loc)  //Address in process to write into
{
    instruction i = BRraw;
    if (!p->writeDataSpace((void *)loc, instruction::size(), i.ptr()))
        fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);
}

void saveRegisterAtOffset(codeGen &gen,
                          Register reg,
                          int save_off) {
    if (gen.width() == 4) {
        insnCodeGen::generateImm(gen, STop,
                                 reg, REG_SP, save_off);
    } else /* gen.width() == 8 */ {
        insnCodeGen::generateMemAccess64(gen, STDop, STDxop,
                                         reg, REG_SP, save_off);
    }
}

// Dest != reg : optimizate away a load/move pair
void saveRegister(codeGen &gen,
                  Register source,
                  Register dest,
                  int save_off)
{
    saveRegisterAtOffset(gen, source, save_off + (dest * gen.width()));
    //  bperr("Saving reg %d at 0x%x off the stack\n", reg, offset + reg*GPRSIZE);
}

void saveRegister(codeGen &gen,
                  Register reg,
                  int save_off)
{
    saveRegister(gen, reg, reg, save_off);
}

void restoreRegisterAtOffset(codeGen &gen,
                             Register dest,
                             int saved_off) {
    if (gen.width() == 4) {
        insnCodeGen::generateImm(gen, Lop, 
                                 dest, REG_SP, saved_off);
    } else /* gen.width() == 8 */ {
        insnCodeGen::generateMemAccess64(gen, LDop, LDxop,
                                         dest, REG_SP, saved_off);
    }
}

// Dest != reg : optimizate away a load/move pair
void restoreRegister(codeGen &gen,
                     Register source,
                     Register dest, 
                     int saved_off)
{
    return restoreRegisterAtOffset(gen, dest, saved_off + (source * gen.width()));
    //bperr( "Loading reg %d (into reg %d) at 0x%x off the stack\n", 
    //  reg, dest, offset + reg*GPRSIZE);
}

void restoreRegister(codeGen &gen,
                     Register reg,
                     int save_off)
{
    restoreRegister(gen, reg, reg, save_off);
}

void saveFPRegister(codeGen &gen, 
                    Register reg,
                    int save_off)
{
    assert("WE SHOULD NOT BE HERE" == 0);

    insnCodeGen::generateImm(gen, STFDop, 
                             reg, REG_SP, save_off + reg*FPRSIZE);
    //fprintf(stderr, "Save offset: %d\n", save_off + reg*FPRSIZE);
    //bperr( "Saving FP reg %d at 0x%x off the stack\n", 
    //  reg, offset + reg*FPRSIZE);
}

void restoreFPRegister(codeGen &gen,
                       Register source,
                       Register dest,
                       int save_off)
{
    assert("WE SHOULD NOT BE HERE" == 0);
    
    insnCodeGen::generateImm(gen, LFDop, 
                             dest, REG_SP, save_off + source*FPRSIZE);
    //  bperr("Loading FP reg %d (into %d) at 0x%x off the stack\n", 
    //  reg, dest, offset + reg*FPRSIZE);
}

void restoreFPRegister(codeGen &gen,
                       Register reg,
                       int save_off)
{
    restoreFPRegister(gen, reg, reg, save_off);
}	

/*
 * Emit code to push down the stack, AST-generate style
 */
void pushStack(codeGen &gen)
{
    if (gen.width() == 4) {
	insnCodeGen::generateImm(gen, STUop,
				 REG_SP, REG_SP, -TRAMP_FRAME_SIZE_32);
    } else /* gen.width() == 8 */ {
	insnCodeGen::generateMemAccess64(gen, STDop, STDUxop,
                                  REG_SP, REG_SP, -TRAMP_FRAME_SIZE_64);
    }
}

void popStack(codeGen &gen)
{
    if (gen.width() == 4) {
	insnCodeGen::generateImm(gen, CALop, 
				 REG_SP, REG_SP, TRAMP_FRAME_SIZE_32);

    } else /* gen.width() == 8 */ {
	insnCodeGen::generateImm(gen, CALop,
                                 REG_SP, REG_SP, TRAMP_FRAME_SIZE_64);
    }
}

/*
 * Save necessary registers on the stack
 * insn, base: for code generation. Offset: regs saved at offset + reg
 * Returns: number of registers saved.
 * Side effects: instruction pointer and base param are shifted to 
 *   next free slot.
 */
unsigned saveGPRegisters(codeGen &gen,
                         registerSpace *theRegSpace,
                         int save_off, int numReqGPRs)
{
    int numRegs = 0;
    if (numReqGPRs == -1 ) numReqGPRs = theRegSpace->numGPRs();
    for(int i = 0; i < theRegSpace->numGPRs(); i++) {
        registerSlot *reg = theRegSpace->GPRs()[i];
        if (reg->liveState == registerSlot::live) {
	    saveRegister(gen, reg->encoding(), save_off);
            // saveRegister implicitly adds in (reg * word size)
            // Do that by hand here.
            
            int actual_save_off = save_off;

            actual_save_off += (reg->encoding() * gen.width());

	    gen.rs()->markSavedRegister(reg->number, actual_save_off);
	    numRegs++;
	    if (numRegs == numReqGPRs) break;
	}
    }
    return numRegs;
}

/*
 * Restore necessary registers from the stack
 * insn, base: for code generation. Offset: regs restored from offset + reg
 * Returns: number of registers restored.
 * Side effects: instruction pointer and base param are shifted to 
 *   next free slot.
 */

unsigned restoreGPRegisters(codeGen &gen,
                            registerSpace *theRegSpace,
                            int save_off)
{
    unsigned numRegs = 0;
    for(int i = 0; i < theRegSpace->numGPRs(); i++) {
        registerSlot *reg = theRegSpace->GPRs()[i];
        if (reg->liveState == registerSlot::spilled) {
	    restoreRegister(gen, reg->encoding(), save_off);
	    numRegs++;
        }
    }

    return numRegs;
}

/*
 * Save FPR registers on the stack. (0-13)
 * insn, base: for code generation. Offset: regs saved at offset + reg
 * Returns: number of regs saved.
 */

unsigned saveFPRegisters(codeGen &gen,
                         registerSpace *,
                         int save_off)
{
  insnCodeGen::saveVectors(gen, save_off);

  // for(int i = 0; i < theRegSpace->numFPRs(); i++) {
  //     registerSlot *reg = theRegSpace->FPRs()[i];
  //     if (reg->liveState == registerSlot::live) {
  //         saveFPRegister(gen, reg->encoding(), save_off);
  //         reg->liveState = registerSlot::spilled;
  //         numRegs++;
  //     }
  // }  
  
  return 32;
}

/*
 * Restore FPR registers from the stack. (0-13)
 * insn, base: for code generation. Offset: regs restored from offset + reg
 * Returns: number of regs restored.
 */

unsigned restoreFPRegisters(codeGen &gen, 
                            registerSpace *,
                            int save_off)
{
  
  insnCodeGen::restoreVectors(gen, save_off);
  // for(int i = 0; i < theRegSpace->numFPRs(); i++) {
  //     registerSlot *reg = theRegSpace->FPRs()[i];
  //     if (reg->liveState == registerSlot::spilled) {
  //         restoreFPRegister(gen, reg->encoding(), save_off);
  //         numRegs++;
  //     }
  // }
  
  return 32;
}

/*
 * Save the special purpose registers (for Dyninst conservative tramp)
 * CTR, CR, XER, SPR0, FPSCR
 */
unsigned saveSPRegisters(codeGen &gen,
                         registerSpace *,
                         int save_off,
			 int force_save)
{
    unsigned num_saved = 0;
    int cr_off, ctr_off, xer_off, fpscr_off;
    
    if (gen.width() == 4) {
	cr_off    = STK_CR_32;
	ctr_off   = STK_CTR_32;
	xer_off   = STK_XER_32;
	fpscr_off = STK_FP_CR_32;
    } else /* gen.width() == 8 */ {
	cr_off    = STK_CR_64;
	ctr_off   = STK_CTR_64;
	xer_off   = STK_XER_64;
	fpscr_off = STK_FP_CR_64;
    }

    registerSlot *regCR = (*(gen.rs()))[registerSpace::cr]; 
    assert (regCR != NULL); 
    if (force_save || regCR->liveState == registerSlot::live) 
    {
    saveCR(gen, 10, save_off + cr_off); num_saved++;
    gen.rs()->markSavedRegister(registerSpace::cr, save_off + cr_off);
    }
    registerSlot *regCTR = (*(gen.rs()))[registerSpace::ctr]; 
    assert (regCTR != NULL); 
    if (force_save || regCTR->liveState == registerSlot::live) 
    {
    saveSPR(gen, 10, SPR_CTR, save_off + ctr_off); num_saved++;
    gen.rs()->markSavedRegister(registerSpace::ctr, save_off + ctr_off);
    }

    registerSlot *regXER = (*(gen.rs()))[registerSpace::xer]; 
    assert (regXER != NULL); 
    if (force_save || regXER->liveState == registerSlot::live) 
   {
    saveSPR(gen, 10, SPR_XER, save_off + xer_off); num_saved++;
    gen.rs()->markSavedRegister(registerSpace::xer, save_off + xer_off);
    }

    saveFPSCR(gen, 10, save_off + fpscr_off); num_saved++;

    return num_saved;
}

/*
 * Restore the special purpose registers (for Dyninst conservative tramp)
 * CTR, CR, XER, SPR0, FPSCR
 */

unsigned restoreSPRegisters(codeGen &gen,
                            registerSpace *,
                            int save_off,
			    int force_save)
{
    int cr_off, ctr_off, xer_off, fpscr_off;
    unsigned num_restored = 0;

    if (gen.width() == 4) {
	cr_off    = STK_CR_32;
	ctr_off   = STK_CTR_32;
	xer_off   = STK_XER_32;
	fpscr_off = STK_FP_CR_32;
    } else /* gen.width() == 8 */ {
	cr_off    = STK_CR_64;
	ctr_off   = STK_CTR_64;
	xer_off   = STK_XER_64;
	fpscr_off = STK_FP_CR_64;
    }

    restoreFPSCR(gen, 10, save_off + fpscr_off); num_restored++;

    registerSlot *regXER = (*(gen.rs()))[registerSpace::xer];
    assert (regXER != NULL);
    if (force_save || regXER->liveState == registerSlot::spilled)
    {
    restoreSPR(gen, 10, SPR_XER, save_off + xer_off); num_restored++;
    }
    registerSlot *regCTR = (*(gen.rs()))[registerSpace::ctr]; 
    assert (regCTR != NULL); 
    if (force_save || regCTR->liveState == registerSlot::spilled) 
    {
    restoreSPR(gen, 10, SPR_CTR, save_off + ctr_off); num_restored++;
    }
    registerSlot *regCR = (*(gen.rs()))[registerSpace::cr];
    assert (regCR != NULL);
    if (force_save || regCR->liveState == registerSlot::spilled)
    {
    restoreCR(gen, 10, save_off + cr_off); num_restored++;
    }

    return num_restored;
}


bool baseTramp::generateSaves(codeGen &gen,
                              registerSpace *)
{
    regalloc_printf("========== baseTramp::generateSaves\n");
    unsigned int width = gen.width();

    int gpr_off, fpr_off;
    gpr_off = TRAMP_GPR_OFFSET(width);
    fpr_off = TRAMP_FPR_OFFSET(width);

    // Make a stack frame.
    pushStack(gen);

    // Save GPRs
    saveGPRegisters(gen, gen.rs(), gpr_off);

    if(BPatch::bpatch->isSaveFPROn() ||  // Save FPRs
        BPatch::bpatch->isForceSaveFPROn() ) 
	saveFPRegisters(gen, gen.rs(), fpr_off);

    // Save LR            
    saveLR(gen, REG_SCRATCH /* register to use */, TRAMP_SPR_OFFSET(width) + STK_LR);

    saveSPRegisters(gen, gen.rs(), TRAMP_SPR_OFFSET(width), true); // FIXME get liveness fixed
    return true;
}

bool baseTramp::generateRestores(codeGen &gen,
                                 registerSpace *)
{
    unsigned int width = gen.width();

    regalloc_printf("========== baseTramp::generateRestores\n");

    int gpr_off, fpr_off;
    gpr_off = TRAMP_GPR_OFFSET(width);
    fpr_off = TRAMP_FPR_OFFSET(width);

    // Restore possible SPR saves
    restoreSPRegisters(gen, gen.rs(), TRAMP_SPR_OFFSET(width), false);

    // LR
    restoreLR(gen, REG_SCRATCH, TRAMP_SPR_OFFSET(width) + STK_LR);

    if (BPatch::bpatch->isSaveFPROn() || // FPRs
        BPatch::bpatch->isForceSaveFPROn() ) 
	restoreFPRegisters(gen, gen.rs(), fpr_off);

    // GPRs
    restoreGPRegisters(gen, gen.rs(), gpr_off);

    /*
    // Multithread GPR -- always save
    restoreRegister(gen, REG_MT_POS, TRAMP_GPR_OFFSET);
    */

    popStack(gen);

    return true;
}


void emitImm(opCode op, Register src1, RegValue src2imm, Register dest, 
             codeGen &gen, bool noCost, registerSpace * /* rs */, bool s)
{
        //bperr("emitImm(op=%d,src=%d,src2imm=%d,dest=%d)\n",
        //        op, src1, src2imm, dest);
    int iop=-1;
    int result=-1;
    switch (op) {
        // integer ops
    case plusOp:
        iop = CALop;
        insnCodeGen::generateImm(gen, iop, dest, src1, src2imm);
        return;
        break;
        
    case minusOp:
        iop = SIop;
        insnCodeGen::generateImm(gen, iop, dest, src1, src2imm);
        return;
        break;
        
    case timesOp:
       if (isPowerOf2(src2imm,result) && (result < (int) (gen.width() * 8))) {
            insnCodeGen::generateLShift(gen, src1, result, dest);
            return;
        }
        else {
            Register dest2 = gen.rs()->getScratchRegister(gen, noCost);
            emitVload(loadConstOp, src2imm, dest2, dest2, gen, noCost);
            emitV(op, src1, dest2, dest, gen, noCost, gen.rs(), 
                  gen.width(), gen.point(), gen.addrSpace(), s);
            return;
        }
        break;
        
    case divOp: {
            Register dest2 = gen.rs()->getScratchRegister(gen, noCost);
            emitVload(loadConstOp, src2imm, dest2, dest2, gen, noCost);
            emitV(op, src1, dest2, dest, gen, noCost, gen.rs(),
                  gen.width(), gen.point(), gen.addrSpace(), s);
            return;
        }
        // Bool ops
    case orOp:
        iop = ORILop;
        // For some reason, the destField is 2nd for ORILop and ANDILop
        insnCodeGen::generateImm(gen, iop, src1, dest, src2imm);
        return;
        break;
        
    case andOp:
        iop = ANDILop;
        // For some reason, the destField is 2nd for ORILop and ANDILop
        insnCodeGen::generateImm(gen, iop, src1, dest, src2imm);
        return;
        break;
    default:
        Register dest2 = gen.rs()->getScratchRegister(gen, noCost);
        emitVload(loadConstOp, src2imm, dest2, dest2, gen, noCost);
        emitV(op, src1, dest2, dest, gen, noCost, gen.rs(),
              gen.width(), gen.point(), gen.addrSpace(), s);
        return;
        break;
    }
}


/* Recursive function that goes to where our instrumentation is calling
to figure out what registers are clobbered there, and in any function
that it calls, to a certain depth ... at which point we clobber everything

Update-12/06, njr, since we're going to a cached system we are just going to 
look at the first level and not do recursive, since we would have to also
store and reexamine every call out instead of doing it on the fly like before*/
bool EmitterPOWER::clobberAllFuncCall( registerSpace *rs,
                                       func_instance * callee)
		   
{
  if (!callee) return true;

  stats_codegen.startTimer(CODEGEN_LIVENESS_TIMER);
    
  /* usedRegs does calculations if not done before and returns
     whether or not the callee is a leaf function.
     if it is, we use the register info we gathered,
     otherwise, we punt and save everything */
  //     bool isLeafFunc = callee->ifunc()->usedRegs();

  if (callee->ifunc()->isLeafFunc()) {
      std::set<Register> * gprs = callee->ifunc()->usedGPRs();
      std::set<Register>::iterator It = gprs->begin();
      for(unsigned i = 0; i < gprs->size(); i++) {
          //while (It != gprs->end()){
          rs->GPRs()[*(It++)]->beenUsed = true;
      }
      
      std::set<Register> * fprs = callee->ifunc()->usedFPRs();
      std::set<Register>::iterator It2 = fprs->begin();
      for(unsigned i = 0; i < fprs->size(); i++)
      {
          //while (It2 != fprs->end()){
          rs->FPRs()[*(It2++)]->beenUsed = true;
      }
    }
  else {
      for (int i = 0; i < rs->numGPRs(); i++) {
          rs->GPRs()[i]->beenUsed = true;
      }
      for (int i = 0; i < rs->numFPRs(); i++) {
          rs->FPRs()[i]->beenUsed = true;
      }
  }
  stats_codegen.stopTimer(CODEGEN_LIVENESS_TIMER);
  return false;
}


//
// Author: Jeff Hollingsworth (3/26/96)
//
// Emit a function call.
//   It saves registers as needed.
//   copy the passed arguments into the canonical argument registers (r3-r10)
//   64-bit ELF Linux ONLY: 
//     Locate the TOC entry of the callee module and copy it into R2
//   generate a branch and link the destination
//   64-bit ELF Linux ONLY:
//     Restore the original TOC into R2
//   restore the saved registers.
//
// Parameters:
//   op - unused parameter (to be compatible with sparc)
//   srcs - vector of ints indicating the registers that contain the parameters
//   dest - the destination address (should be Address not reg). 
//   insn - pointer to the code we are generating
//   based - offset into the code generated.
//

Register emitFuncCall(opCode, codeGen &, std::vector<AstNodePtr> &, bool, Address) {
	assert(0);
        return 0;
}

Register emitFuncCall(opCode op,
                      codeGen &gen,
                      std::vector<AstNodePtr> &operands, bool noCost,
                      func_instance *callee) {
    //fprintf(stderr, "[DEBUG_CRAP] Generaating function call to %p\n", callee->entryBlock()->GetBlockStartingAddress());
    return gen.emitter()->emitCall(op, gen, operands, noCost, callee);
}

void EmitterPOWER::emitCallWithSaves(codeGen &gen, Address dest, bool saveToc, bool saveLR, bool saveR12) {
    // Save the values onto the stack.... (might be needed).
    if (saveToc) {}
    if (saveLR) {}
    if (saveR12) {}

    emitVload(loadConstOp, dest, 0, 0, gen, false);
    insnCodeGen::generateMoveToLR(gen, 0);
    emitVload(loadConstOp, dest, 12, 12, gen, false);
    instruction brl(BRLraw);
    insnCodeGen::generate(gen,brl);
    inst_printf("Generated BRL\n");
    // Retore the original
    if (saveToc) {}
    if (saveLR) {}
    if (saveR12) {}
}


Register EmitterPOWER::emitCallReplacement(opCode ocode,
                                           codeGen &gen,
                                           bool /* noCost */,
                                           func_instance *callee) {
    // This takes care of the special case where we are replacing an existing
    // linking branch instruction.
    //
    // This code makes two crucial assumptions:
    // 1) LR is free: Linking branch instructions place pre-branch IP in LR.
    // 2) TOC (r2) is free: r2 should hold TOC of destination.  So use it
    //    as scratch, and set it to destination module's TOC upon return.
    //    This works for both the inter and intra module call cases.
    // In the 32-bit case where we can't use r2, stomp on r0 and pray...

    //  Sanity check for opcode.
    assert(ocode == funcJumpOp);

    Register freeReg = 0;
    instruction mtlr(MTLR0raw);

    // 64-bit Mutatees
    if (gen.addrSpace()->proc()->getAddressWidth() == 8) {
        freeReg = 2;
        mtlr = instruction(MTLR2raw);
    }

    // Load register with address.
    emitVload(loadConstOp, callee->addr(), freeReg, freeReg, gen, false);

    // Move to link register.
    insnCodeGen::generate(gen,mtlr);

    Address toc_new = gen.addrSpace()->proc()->getTOCoffsetInfo(callee);
    if (toc_new) {
        // Set up the new TOC value
        emitVload(loadConstOp, toc_new, freeReg, freeReg, gen, false);
    }

    // blr - branch through the link reg.
    instruction blr(BRraw);
    insnCodeGen::generate(gen,blr);

    func_instance *caller = gen.point()->func();
    Address toc_orig = gen.addrSpace()->proc()->getTOCoffsetInfo(caller);
    if (toc_new) {
        // Restore the original TOC value.
        emitVload(loadConstOp, toc_orig, freeReg, freeReg, gen, false);
    }

    // What to return here?
    return Null_Register;
}


// Register EmitterPOWER::emitCallReplacementLR(opCode ocode,
//                                            codeGen &gen,
//                                            bool /* noCost */,
//                                            func_instance *callee) {
//     // This takes care of the special case where we are replacing an existing
//     // linking branch instruction.
//     //
//     // This code makes two crucial assumptions:
//     // 1) LR is free: Linking branch instructions place pre-branch IP in LR.
//     // 2) TOC (r2) is free: r2 should hold TOC of destination.  So use it
//     //    as scratch, and set it to destination module's TOC upon return.
//     //    This works for both the inter and intra module call cases.
//     // In the 32-bit case where we can't use r2, stomp on r0 and pray...

//     //  Sanity check for opcode.
//     assert(ocode == funcJumpOp);

//     Register freeReg = 0;
//     instruction mtlr(MTLR0raw);

//     // 64-bit Mutatees
//     if (gen.addrSpace()->proc()->getAddressWidth() == 8) {
//         freeReg = 2;
//         mtlr = instruction(MTLR2raw);
//     }

//     // Load register with address.
//     emitVload(loadConstOp, callee->addr(), freeReg, freeReg, gen, false);

//     // Move to link register.
//     insnCodeGen::generate(gen,mtlr);

//     Address toc_new = gen.addrSpace()->proc()->getTOCoffsetInfo(callee);
//     if (toc_new) {
//         // Set up the new TOC value
//         emitVload(loadConstOp, toc_new, freeReg, freeReg, gen, false);
//     }

//     // blr - branch through the link reg.
//     instruction blr(BRraw);
//     insnCodeGen::generate(gen,blr);

//     func_instance *caller = gen.point()->func();
//     Address toc_orig = gen.addrSpace()->proc()->getTOCoffsetInfo(caller);
//     if (toc_new) {
//         // Restore the original TOC value.
//         emitVload(loadConstOp, toc_orig, freeReg, freeReg, gen, false);
//     }

//     // What to return here?
//     return Null_Register;
// }
// There are four "axes" going on here:
// 32 bit vs 64 bit  
// Instrumentation vs function call replacement
// Static vs. dynamic 

Register EmitterPOWER::emitCall(opCode ocode,
                                codeGen &gen,
                                const std::vector<AstNodePtr> &operands,
                                bool noCost,
                                func_instance *callee) {
    bool inInstrumentation = true;

    //fprintf(stderr, "[EmitterPOWER::emitCall] making call to: %llx\n", callee-> );
    // If inInstrumentation is true we're in instrumentation;
    // if false we're in function call replacement

    //fprintf(stderr, "%s %p\n", "[DEBUG_CRAP] In emit call for ", callee->entryBlock()->GetBlockStartingAddress());
    if (ocode == funcJumpOp)
	return emitCallReplacement(ocode, gen, noCost, callee);

    //  Sanity check for NULL address argument
    if (!callee) {
        char msg[256];
        sprintf(msg, "%s[%d]:  internal error:  emitFuncCall called w/out"
                "callee argument", __FILE__, __LINE__);
        showErrorCallback(80, msg);
        assert(0);
    }

    // Now that we have the destination address (unique, hopefully) 
    // get the TOC anchor value for that function
    // The TOC offset is stored in the Object. 
    // file() -> pdmodule "parent"
    // exec() -> image "parent"
 
    Address toc_anchor = 0;
    Address caller_toc = 0;
    std::vector <Register> srcs;

    // Linux, 64, static/dynamic, inst/repl
    // DYN
    toc_anchor = gen.addrSpace()->getTOCoffsetInfo(callee);
    
    // Instead of saving the TOC (if we can't), just reset it afterwards.
    if (gen.func()) {
//      fprintf(stderr, "info: %s:%d: \n", __FILE__, __LINE__); 
      caller_toc = gen.addrSpace()->getTOCoffsetInfo(gen.func());
    }
    else if (gen.point()) {
      //fprintf(stderr, "info: %s:%d: \n", __FILE__, __LINE__);
      caller_toc = gen.addrSpace()->getTOCoffsetInfo(gen.point()->func());
    }
    else {
      // Don't need it, and this might be an iRPC
    }
    
    inst_printf("Caller TOC 0x%lx; callee 0x%lx\n",
		caller_toc, toc_anchor);
    // ALL
    bool needToSaveLR = false;
    registerSlot *regLR = (*(gen.rs()))[registerSpace::lr];
    if (regLR && regLR->liveState == registerSlot::live) {
        needToSaveLR = true;
        //fprintf(stderr, "info: %s:%d: \n", __FILE__, __LINE__);
        inst_printf("... need to save LR\n");
    }

    // Note: For 32-bit ELF PowerPC Linux (and other SYSV ABI followers)
    // r2 is described as "reserved for system use and is not to be 
    // changed by application code".
    // On these platforms, we return 0 when getTOCoffsetInfo is called.

    std::vector<int> savedRegs;

    //  Save the link register.
    // mflr r0
    // Linux, 32/64, stat/dynamic, instrumentation
    if (needToSaveLR) {
        //fprintf(stderr, "info: %s:%d: \n", __FILE__, __LINE__);
        assert(inInstrumentation);
        insnCodeGen::generateMoveFromLR(gen, 0);
        saveRegister(gen, 0, FUNC_CALL_SAVE(gen.width()));
        savedRegs.push_back(0);
        inst_printf("saved LR in 0\n");
    }

    if (inInstrumentation &&
        (toc_anchor != caller_toc)) {
        // Save register 2 (TOC)
        //fprintf(stderr, "info: %s:%d: \n", __FILE__, __LINE__); 
      saveRegister(gen, 2, FUNC_CALL_SAVE(gen.width()));
        savedRegs.push_back(2);
    }

    // see what others we need to save.
    for (int i = 0; i < gen.rs()->numGPRs(); i++) {
        //fprintf(stderr, "info: %s:%d: \n", __FILE__, __LINE__);
       registerSlot *reg = gen.rs()->GPRs()[i];

       // We must save if:
       // refCount > 0 (and not a source register)
       // keptValue == true (keep over the call)
       // liveState == live (technically, only if not saved by the callee) 
       
       if (inInstrumentation &&
           ((reg->refCount > 0) || 
            reg->keptValue ||
            (reg->liveState == registerSlot::live))) {
        //fprintf(stderr, "info: %s:%d: Register: %d \n", __FILE__, __LINE__, reg->number); 
	 saveRegister(gen, reg->number, FUNC_CALL_SAVE(gen.width()));
          savedRegs.push_back(reg->number);
       }
    }

    // Generate the code for all function parameters, and keep a list
    // of what registers they're in.
    for (unsigned u = 0; u < operands.size(); u++) {
    // Note: if we're in function replacement, we can assert operands.empty()
/*
	if (operands[u]->getSize() == 8) {
	    // What does this do?
	    bperr( "in weird code\n");
	    Register dummyReg = gen.rs()->allocateRegister(gen, noCost);
	    srcs.push_back(dummyReg);

	    insnCodeGen::generateImm(gen, CALop, dummyReg, 0, 0);
	}
*/
	//Register src = Null_Register;
        // Try to target the code generation
        
        Register reg = Null_Register;
        // Try to allocate the correct parameter register
        if (gen.rs()->allocateSpecificRegister(gen, registerSpace::r3 + u, true))
            reg = registerSpace::r3 + u;
             //fprintf(stderr, "info: %s:%d: Register: %d \n", __FILE__, __LINE__, reg); 
	Address unused = ADDR_NULL;
	if (!operands[u]->generateCode_phase2( gen, false, unused, reg)) assert(0);
	assert(reg != Null_Register);
	srcs.push_back(reg);
	//bperr( "Generated operand %d, base %d\n", u, base);
    }
  
    if(srcs.size() > 8) {
	// This is not necessarily true; more then 8 arguments could be passed,
	// the first 8 need to be in registers while the others need to be on
	// the stack, -- sec 3/1/97
       std::string msg = "Too many arguments to function call in instrumentation code:"
	    " only 8 arguments can (currently) be passed on the POWER architecture.\n";
	bperr("%s", msg.c_str());
	showErrorCallback(94,msg);
	exit(-1);
    }

    // If we got the wrong register, we may need to do a 3-way swap. 

    int scratchRegs[8];
    for (int a = 0; a < 8; a++) {
	scratchRegs[a] = -1;
    }

    // Now load the parameters into registers.
    for (unsigned u=0; u<srcs.size(); u++){

        // Parameters start at register 3 - so we're already done
        // in this case
	if (srcs[u] == (registerSpace::r3+u)) {
         //fprintf(stderr, "info: %s:%d: Register: %d \n", __FILE__, __LINE__, srcs[u]); 
	    gen.rs()->freeRegister(srcs[u]);
	    continue;
	}

	int whichSource = -1;
	bool hasSourceBeenCopied = true;
        

        // If the parameter we want exists in a scratch register...
	if (scratchRegs[u] != -1) {
         //fprintf(stderr, "info: %s:%d: Register: %d \n", __FILE__, __LINE__,u+3); 
	    insnCodeGen::generateImm(gen, ORILop, scratchRegs[u], u+3, 0);
	    gen.rs()->freeRegister(scratchRegs[u]);
            // We should check to make sure the one we want isn't occupied?
	} else {
	    for (unsigned v=u; v < srcs.size(); v++) {
		if (srcs[v] == u+3) {
                    // Okay, so the source we want is actuall in srcs[v]
		    hasSourceBeenCopied = false;
		    whichSource = v;
                    break;
		}
	    }
            // Ummm... we didn't find it? Ah, so copying us (since we're wrong)
            // into scratch.
	    if (!hasSourceBeenCopied) {
                Register scratch = gen.rs()->getScratchRegister(gen);
		insnCodeGen::generateImm(gen, ORILop, u+3, scratch, 0);
		gen.rs()->freeRegister(u+3);
		scratchRegs[whichSource] = scratch;
		hasSourceBeenCopied = true;

		insnCodeGen::generateImm(gen, ORILop, srcs[u], u+3, 0);
		gen.rs()->freeRegister(srcs[u]);

	    } else {
		insnCodeGen::generateImm(gen, ORILop, srcs[u], u+3, 0);
		gen.rs()->freeRegister(srcs[u]);
                // Not sure why this was needed
		//gen.rs()->clobberRegister(u+3);
	    }
	} 
    }

    // Call generation time.
    bool setTOC = false;

	// Linux, 64, stat/dyn, inst/repl
    if (toc_anchor != caller_toc) {
         //fprintf(stderr, "info: %s:%d: \n", __FILE__, __LINE__); 
        setTOC = true;
    }
    
    emitCallInstruction(gen, callee, setTOC, toc_anchor);
    // ALL instrumentation
    Register retReg = Null_Register;
    if (inInstrumentation) {
        // get a register to keep the return value in.
        retReg = gen.rs()->allocateRegister(gen, noCost);        
        // put the return value from register 3 to the newly allocated register.
        insnCodeGen::generateImm(gen, ORILop, 3, retReg, 0);
    }

        
    // Otherwise we're replacing a call and so we don't want to move
    // anything. 

    // restore saved registers.
    // If inInstrumentation == false then this vector should be empty...
 	// ALL instrumentation
 
    if (!inInstrumentation) assert(savedRegs.size() == 0);
    for (u_int ui = 0; ui < savedRegs.size(); ui++) {
      restoreRegister(gen, savedRegs[ui], FUNC_CALL_SAVE(gen.width()));
    }
  
    // mtlr	0 (aka mtspr 8, rs) = 0x7c0803a6
    // Move to link register
    // Reused from above. instruction mtlr0(MTLR0raw);
    if (needToSaveLR) {
        // We only use register 0 to save LR. 
        insnCodeGen::generateMoveToLR(gen, 0);
    }
    
    if (!inInstrumentation && setTOC) {
        // Need to reset the TOC
        emitVload(loadConstOp, caller_toc, 2, 2, gen, false);

        // Also store toc_orig [r2] into the TOC save area [40(r1)].
        // Subsequent code will look for it there.
        saveRegisterAtOffset(gen, 2, 40);
    }        

    /*
      gen = (instruction *) gen;
      for (unsigned foo = initBase/4; foo < base/4; foo++)
      bperr( "0x%x,\n", gen[foo].raw);
    */ 
    // return value is the register with the return value from the called function
    return(retReg);
}

 
codeBufIndex_t emitA(opCode op, Register src1, Register /*src2*/, long dest,
	      codeGen &gen, RegControl, bool /*noCost*/)
{
    //bperr("emitA(op=%d,src1=%d,src2=XX,dest=%d)\n",op,src1,dest);
    codeBufIndex_t retval = 0;
    switch (op) {
      case ifOp: {
	// cmpi 0,0,src1,0
          instruction insn;
          insn.clear();
          DFORM_OP_SET(insn, CMPIop);
          DFORM_RA_SET(insn, src1);
          DFORM_SI_SET(insn, 0);
          insnCodeGen::generate(gen,insn);
          retval = gen.getIndex();
          
          // be 0, dest
          insn.clear();
          BFORM_OP_SET(insn, BCop);
          BFORM_BO_SET(insn, BTRUEcond);
          BFORM_BI_SET(insn, EQcond);
          BFORM_BD_SET(insn, dest/4);
          BFORM_AA_SET(insn, 0);
          BFORM_LK_SET(insn, 0);
          
          insnCodeGen::generate(gen,insn);
          break;
      }
    case branchOp: {
        retval = gen.getIndex();
        insnCodeGen::generateBranch(gen, dest);
        break;
    }
    case trampPreamble: {
        // nothing to do in this platform
        return(0);              // let's hope this is expected!
    }        
    default:
        assert(0);        // unexpected op for this emit!
    }
    return retval;
}

Register emitR(opCode op, Register src1, Register src2, Register dest,
               codeGen &gen, bool /*noCost*/,
               const instPoint * /*location*/, bool /*for_MT*/)
{
    //bperr("emitR(op=%d,src1=%d,src2=XX,dest=%d)\n",op,src1,dest);

    registerSlot *regSlot = NULL;
    unsigned addrWidth = gen.width();

    switch (op) {
    case getRetValOp: {
        regSlot = (*(gen.rs()))[registerSpace::r3];
        break;
    }

    case getParamOp: {
        // The first 8 parameters (0-7) are stored in registers (r3-r10) upon
        // entering the function and then saved above the stack upon entering
        // the trampoline; in emit functional call the stack pointer is moved
        // so the saved registers are not over-written the other parameters >
        // 8 are stored on the caller's stack at an offset.
        // 
        // src1 is the argument number 0..X, the first 8 are stored in regs
        // src2 (if not Null_Register) holds the value to be written into src1

        if(src1 < 8) {
            // src1 is 0..8 - it's a parameter number, not a register
            regSlot = (*(gen.rs()))[registerSpace::r3 + src1];
            break;

        } else {
            // Registers from 11 (src = 8) and beyond are saved on the stack.

            int stkOffset;
            if (addrWidth == 4) {
                stkOffset = TRAMP_FRAME_SIZE_32 +
                            (src1 - 8) * sizeof(int) +
                            PARAM_OFFSET(addrWidth);
            } else {
	      // Linux ABI says:
	      // Parameters go in the "argument save area", which starts at
	      // PARAM_OFFSET(...). However, we'd save argument _0_ at the base
	      // of it, so the first 8 slots are normally empty (as they go in
	      // registers). To get the 9th, etc. argument you want
	      // PARAM_OFFSET(...) + (8 * arg number) instead of
	      // 8 * (arg_number - 8)
	      int stackSlot = src1;
	      stkOffset = TRAMP_FRAME_SIZE_64 +
                            stackSlot * sizeof(long) +
                            PARAM_OFFSET(addrWidth);
            }

            if (src2 != Null_Register) saveRegisterAtOffset(gen, src2, stkOffset);
            restoreRegisterAtOffset(gen, dest, stkOffset);
            return(dest);
      }
      break;
    }

    case getRetAddrOp: {
      regSlot = (*(gen.rs()))[registerSpace::lr];
      break;
    }

    default:
        assert(0);
        break;
    }

    assert(regSlot);
    Register reg = regSlot->number;

    switch(regSlot->liveState) {
    case registerSlot::spilled: {
      int offset = TRAMP_GPR_OFFSET(addrWidth);
      
      // its on the stack so load it.
      if (src2 != Null_Register) saveRegister(gen, src2, reg, offset);
      restoreRegister(gen, reg, dest, offset);
      return(dest);
    }
    case registerSlot::live: {
        // its still in a register so return the register it is in.
        
        return(reg);
    }
    case registerSlot::dead: {
        // Uhhh... wha?
        assert(0);
    }
    }

    assert(0);
    return Null_Register;
}

void emitJmpMC(int /*condition*/, int /*offset*/, codeGen &)
{
  // Not needed for memory instrumentation, otherwise TBD
}


// VG(11/16/01): Say if we have to restore a register to get its original value
static inline bool needsRestore(Register x)
{
  //return (x == 0) || ((x >= 3) && (x <= 12)) || (x == POWER_XER2531);
  return ((x <= 12) && !(x==2)) || (x == POWER_XER2531);
}

// VG(03/15/02): Restore mutatee value of GPR reg to dest GPR
static inline void restoreGPRtoGPR(codeGen &gen,
                                   Register reg, Register dest)
{
    int frame_size, gpr_size, gpr_off;
    if (gen.width() == 4) {
	frame_size = TRAMP_FRAME_SIZE_32;
	gpr_size   = GPRSIZE_32;
	gpr_off    = TRAMP_GPR_OFFSET_32;
    } else /* gen.width() == 8 */ {
	frame_size = TRAMP_FRAME_SIZE_64;
	gpr_size   = GPRSIZE_64;
	gpr_off    = TRAMP_GPR_OFFSET_64;
    }

    if (reg == 1) // SP is in a different place, but we don't need to
                  // restore it, just subtract the stack frame size
        insnCodeGen::generateImm(gen, CALop, dest, REG_SP, frame_size);

    else if((reg == 0) || ((reg >= 3) && (reg <=12)))
	 insnCodeGen::generateMemAccess64(gen, LDop, LDxop, dest, REG_SP, gpr_off + reg*gpr_size);
// Past code restoring 32bit's of register instead of entire register
//        insnCodeGen::generateImm(gen, Lop, dest, REG_SP,
//                                 gpr_off + reg*gpr_size);
    else {
        bperr( "GPR %u should not be restored...", reg);
        assert(0);
    }
    //bperr( "Loading reg %d (into reg %d) at 0x%x off the stack\n", 
    //  reg, dest, offset + reg*GPRSIZE);
}

// VG(03/15/02): Restore mutatee value of XER to dest GPR
static inline void restoreXERtoGPR(codeGen &gen, Register dest)
{
    if (gen.width() == 4) {
        insnCodeGen::generateImm(gen, Lop, dest, REG_SP,
                                 TRAMP_SPR_OFFSET(4) + STK_XER_32);
    } else /* gen.width() == 8 */ {
        insnCodeGen::generateMemAccess64(gen, LDop, LDxop, dest, REG_SP,
                                         TRAMP_SPR_OFFSET(8) + STK_XER_64);
    }
}

// VG(03/15/02): Move bits 25:31 of GPR reg to GPR dest
static inline void moveGPR2531toGPR(codeGen &gen,
                                    Register reg, Register dest)
{
  // keep only bits 32+25:32+31; extrdi dest, reg, 7 (n bits), 32+25 (start at b)
  // which is actually: rldicl dest, reg, 32+25+7 (b+n), 64-7 (64-n)
  // which is the same as: clrldi dest,reg,57 because 32+25+7 = 64
    instruction rld;
    rld.clear();
    MDFORM_OP_SET( rld, RLDop);
    MDFORM_RS_SET( rld, reg);
    MDFORM_RA_SET( rld, dest);
    MDFORM_SH_SET( rld, 0);  //(32+25+7) % 32;
    MDFORM_MB_SET( rld, (64-7) % 32);
    MDFORM_MB2_SET(rld, (64-7) / 32);
    MDFORM_XO_SET( rld, ICLxop);
    MDFORM_SH2_SET(rld, 0); //(32+25+7) / 32;
    MDFORM_RC_SET( rld, 0);
    insnCodeGen::generate(gen,rld);
}

// VG(11/16/01): Emit code to add the original value of a register to
// another. The original value may need to be restored from stack...
// VG(03/15/02): Made functionality more obvious by adding the above functions
static inline void emitAddOriginal(Register src, Register acc, 
                                   codeGen &gen, bool noCost)
{
    bool nr = needsRestore(src);
    Register temp;
    
    if(nr) {
        // this needs gen because it uses emitV...
        temp = gen.rs()->allocateRegister(gen, noCost);
        
        // Emit code to restore the original ra register value in temp.
        // The offset compensates for the gap 0, 3, 4, ...
        // This writes at insn, and updates insn and base.
        
        if(src == POWER_XER2531) { // hack for XER_25:31
            //bperr( "XER_25:31\n");
            restoreXERtoGPR(gen, temp);
            moveGPR2531toGPR(gen, temp, temp);
        }
        else
            restoreGPRtoGPR(gen, src, temp);
    }
    else
        temp = src;
    
    // add temp to dest;
    // writes at gen+base and updates base, we must update insn...
    emitV(plusOp, temp, acc, acc, gen, noCost, 0);
    
    if(nr)
        gen.rs()->freeRegister(temp);
}

// VG(11/07/01): Load in destination the effective address given
// by the address descriptor. Used for memory access stuff.
void emitASload(const BPatch_addrSpec_NP *as, Register dest, int stackShift,
		codeGen &gen,
		bool noCost)
{
  // Haven't implemented non-zero shifts yet
  assert(stackShift == 0);
  //instruction *insn = (instruction *) ((void*)&gen[base]);
  int imm = as->getImm();
  int ra  = as->getReg(0);
  int rb  = as->getReg(1);
  // TODO: optimize this to generate the minimum number of
  // instructions; think about schedule

  // emit code to load the immediate (constant offset) into dest; this
  // writes at gen+base and updates base, we must update insn...
  emitVload(loadConstOp, (Address)imm, dest, dest, gen, noCost);
  
  // If ra is used in the address spec, allocate a temp register and
  // get the value of ra from stack into it
  if(ra > -1)
      emitAddOriginal(ra, dest, gen, noCost);

  // If rb is used in the address spec, allocate a temp register and
  // get the value of ra from stack into it
  if(rb > -1)
    emitAddOriginal(rb, dest, gen, noCost);

}

void emitCSload(const BPatch_addrSpec_NP *as, Register dest, codeGen &gen,               
		bool noCost)
{
  emitASload(as, dest, 0, gen, noCost);
}

void emitVload(opCode op, Address src1, Register src2, Register dest,
               codeGen &gen, bool /*noCost*/, 
               registerSpace * /*rs*/, int size,
               const instPoint * /* location */, AddressSpace *proc)
{
  switch(op) {
  case loadConstOp:
    insnCodeGen::loadImmIntoReg(gen, dest, (long)src1);
    break;
  case loadOp:
    insnCodeGen::loadPartialImmIntoReg(gen, dest, (long)src1);
    
    // really load dest, (dest)imm
    if (size == 1) {
      insnCodeGen::generateImm(gen, LBZop, dest, dest, LOW(src1));
    }
    else if (size == 2) {
      insnCodeGen::generateImm(gen, LHZop, dest, dest, LOW(src1));
    }
    else if ((size == 4) ||
	     (size == 8 && proc->getAddressWidth() == 4)) // Override bogus size
      insnCodeGen::generateImm(gen, Lop,   dest, dest, LOW(src1));
    else if (size == 8)
      insnCodeGen::generateMemAccess64(gen, LDop, LDxop,
				       dest, dest, (int16_t)LOW(src1));
    else assert(0 && "Incompatible loadOp size");
    break;
  case loadFrameRelativeOp: {
	long offset = (long)src1;
	if (gen.width() == 4)
	    offset += TRAMP_FRAME_SIZE_32;
	else /* gen.width() == 8 */
	    offset += TRAMP_FRAME_SIZE_64;

	// return the value that is FP offset from the original fp
	if (size == 1)
	    insnCodeGen::generateImm(gen, LBZop, dest, REG_SP, offset);
	else if (size == 2)
	    insnCodeGen::generateImm(gen, LHZop, dest, REG_SP, offset);
	else if ((size == 4) ||
		 (size == 8 && proc->getAddressWidth() == 4)) // Override bogus size
	    insnCodeGen::generateImm(gen, Lop,   dest, REG_SP, offset);
	else if (size == 8)
	    insnCodeGen::generateMemAccess64(gen, LDop, LDxop,
					     dest, REG_SP, offset);
	else assert(0 && "Incompatible loadFrameRelativeOp size");
  }
    break;
  case loadFrameAddr: {
    // offsets are signed!
    long offset = (long)src1;
    offset += (gen.width() == 4 ? TRAMP_FRAME_SIZE_32
	       : TRAMP_FRAME_SIZE_64);
    
    if (offset < MIN_IMM16 || MAX_IMM16 < offset) assert(0);
    insnCodeGen::generateImm(gen, CALop, dest, REG_SP, offset);
  }
    break;
  case loadRegRelativeAddr:
    // (readReg(src2) + src1)
    gen.rs()->readProgramRegister(gen, src2, dest, size);
    emitImm(plusOp, dest, src1, dest, gen, false);
    break;
  case loadRegRelativeOp:
    // *(readReg(src2) + src1)
    gen.rs()->readProgramRegister(gen, src2, dest, size);

    if (size == 1)
      insnCodeGen::generateImm(gen, LBZop, dest, dest, src1);
    else if (size == 2)
      insnCodeGen::generateImm(gen, LHZop, dest, dest, src1);
    else if ((size == 4) ||
	     (size == 8 && proc->getAddressWidth() == 4)) // Override bogus size
      insnCodeGen::generateImm(gen, Lop,   dest, dest, src1);
    else if (size == 8)
      insnCodeGen::generateMemAccess64(gen, LDop, LDxop,
				       dest, dest, src1);
    break;
  default:


    cerr << "Unknown op " << op << endl;
    assert(0);
  }
}

void emitVstore(opCode op, Register src1, Register /*src2*/, Address dest,
		codeGen &gen, bool noCost, 
                registerSpace * /* rs */, int size,
                const instPoint * /* location */, AddressSpace *proc)
{
    if (op == storeOp) {
	// temp register to hold base address for store (added 6/26/96 jkh)
	Register temp = gen.rs()->getScratchRegister(gen, noCost);

        insnCodeGen::loadPartialImmIntoReg(gen, temp, (long)dest);
        if (size == 1)
            insnCodeGen::generateImm(gen, STBop, src1, temp, LOW(dest));
        else if (size == 2)
            insnCodeGen::generateImm(gen, STHop, src1, temp, LOW(dest));
        else if ((size == 4) ||
		 (size == 8 && proc->getAddressWidth() == 4)) // Override bogus size
            insnCodeGen::generateImm(gen, STop,  src1, temp, LOW(dest));
        else if (size == 8)
            insnCodeGen::generateMemAccess64(gen, STDop, STDxop,
                                             src1, temp, (int16_t)BOT_LO(dest));
        else assert(0 && "Incompatible storeOp size");

    } else if (op == storeFrameRelativeOp) {
        long offset = (long)dest;
        offset += (gen.width() == 4
		   ? TRAMP_FRAME_SIZE_32
		   : TRAMP_FRAME_SIZE_64);

        if (size == 1)
            insnCodeGen::generateImm(gen, STBop, src1, REG_SP, offset);
        else if (size == 2)
            insnCodeGen::generateImm(gen, STHop, src1, REG_SP, offset);
        else if ((size == 4) ||
		 (size == 8 || proc->getAddressWidth() == 4)) // Override bogus size
            insnCodeGen::generateImm(gen, STop,  src1, REG_SP, offset);
        else if (size == 8)
            insnCodeGen::generateMemAccess64(gen, STDop, STDxop, src1,
                                             REG_SP, offset);
        else assert(0 && "Incompatible storeFrameRelativeOp size");

    } else assert(0 && "Unknown op passed to emitVstore");
}

void emitV(opCode op, Register src1, Register src2, Register dest,
           codeGen &gen, bool /*noCost*/,
           registerSpace * /*rs*/, int size,
           const instPoint * /* location */, AddressSpace *proc, bool s)
{
    //bperr("emitV(op=%d,src1=%d,src2=%d,dest=%d)\n",op,src1,src2,dest);

    assert ((op!=branchOp) && (op!=ifOp) && 
            (op!=trampPreamble));         // !emitA
    assert ((op!=getRetValOp) && (op!=getParamOp));             // !emitR
    assert ((op!=loadOp) && (op!=loadConstOp));                 // !emitVload
    assert ((op!=storeOp));                                     // !emitVstore
    assert ((op!=updateCostOp));                                // !emitVupdate

    instruction insn;

    if (op == loadIndirOp) {
       // really load dest, (dest)imm
       assert(src2 == 0); // Since we don't use it.
       if (!size)
          size = proc->getAddressWidth();
       if (size == 1)
          insnCodeGen::generateImm(gen, LBZop, dest, src1, 0);
       else if (size == 2)
          insnCodeGen::generateImm(gen, LHZop, dest, src1, 0);
       else if ((size == 4) ||
                (size == 8 && proc->getAddressWidth() == 4)) // Override bogus size
          insnCodeGen::generateImm(gen, Lop,   dest, src1, 0);
       else if (size == 8) {
          insnCodeGen::generateMemAccess64(gen, LDop, LDxop,
                                           dest, src1, 0);
       } 
       else 
          assert(0 && "Incompatible loadOp size");
    } else if (op == storeIndirOp) {
        // generate -- st src1, dest
        if (size == 1)
            insnCodeGen::generateImm(gen, STBop, src1, dest, 0);
        else if (size == 2)
            insnCodeGen::generateImm(gen, STHop, src1, dest, 0);
        else if ((size == 4) ||
		 (size == 8 && proc->getAddressWidth() == 4)) // Override bogus size
            insnCodeGen::generateImm(gen, STop,  src1, dest, 0);
        else if (size == 8)
            insnCodeGen::generateMemAccess64(gen, STDop, STDxop,
                                             src1, dest, 0);
        else assert(0 && "Incompatible storeOp size");

    } else if (op == noOp) {
        insnCodeGen::generateNOOP(gen);

    } else if (op == saveRegOp) {
        saveRegister(gen,src1,8);

    } else {
        int instXop=-1;
        int instOp=-1;
        switch (op) {
            // integer ops
            case plusOp:
                instOp = CAXop;
                instXop = CAXxop;
                break;

            case minusOp:
                Register temp;
                // need to flip operands since this computes ra-rb not rb-ra
                temp = src1;
                src1 = src2;
                src2 = temp;
                instOp = SFop;
                instXop = SFxop;
                break;

            case timesOp:
                instOp = MULSop;
                // For 64-bit integer multiplication, 
                // signed and unsigned are the same.
                //
                // Signed and unsigned are different if we start to 
                // psas upper 64-bit of the results back to users.
                instXop = MULLxop;
                break;

            case divOp:
                instOp = DIVSop;   // POWER divide instruction
                                   // Same as DIVWop for PowerPC
                if (s)
                    instXop = DIVLSxop; // Extended opcode for signed division
                else
                    instXop = DIVLUxop; // Extended opcode for unsigned division
                break;

            // Bool ops
            case orOp:
                //genSimple(gen, ORop, src1, src2, dest);
                insn.clear();
                XOFORM_OP_SET(insn, ORop);
                // operation is ra <- rs | rb (or rs,ra,rb)
                XOFORM_RA_SET(insn, dest);
                XOFORM_RT_SET(insn, src1);
                XOFORM_RB_SET(insn, src2);
                XOFORM_XO_SET(insn, ORxop);
                insnCodeGen::generate(gen,insn);
                return;
                break;

            case andOp:
                //genSimple(gen, ANDop, src1, src2, dest);
                // This is a Boolean and with true == 1 so bitwise is OK
                insn.clear();
                XOFORM_OP_SET(insn, ANDop);
                // operation is ra <- rs & rb (and rs,ra,rb)
                XOFORM_RA_SET(insn, dest);
                XOFORM_RT_SET(insn, src1);
                XOFORM_RB_SET(insn, src2);
                XOFORM_XO_SET(insn, ANDxop);
                insnCodeGen::generate(gen,insn);
                return;
		break;

	    case xorOp:
		insn.clear();
		XOFORM_OP_SET(insn, XORop);
		// operation is ra <- rs ^ rb (xor rs,ra,rb)
		XOFORM_RA_SET(insn, dest);
		XOFORM_RT_SET(insn, src1);
		XOFORM_RB_SET(insn, src2);
		XOFORM_XO_SET(insn, XORxop);
		insnCodeGen::generate(gen, insn);
		return;
		break;

            // rel ops
            case eqOp:
                insnCodeGen::generateRelOp(gen, EQcond, BTRUEcond, src1, src2, dest, s);
                return;
                break;

            case neOp:
                insnCodeGen::generateRelOp(gen, EQcond, BFALSEcond, src1, src2, dest, s);
                return;
                break;

            case lessOp:
                insnCodeGen::generateRelOp(gen, LTcond, BTRUEcond, src1, src2, dest, s);
                return;
                break;

            case greaterOp:
                insnCodeGen::generateRelOp(gen, GTcond, BTRUEcond, src1, src2, dest, s);
                return;
                break;

            case leOp:
                insnCodeGen::generateRelOp(gen, GTcond, BFALSEcond, src1, src2, dest, s);
                return;
                break;

            case geOp:
                insnCodeGen::generateRelOp(gen, LTcond, BFALSEcond, src1, src2, dest, s);
                return;
                break;

            default:
                // internal error, invalid op.
                bperr( "Invalid op passed to emit, instOp = %d\n", instOp);
                assert(0 && "Invalid op passed to emit");
                break;
        }

        
        assert((instOp != -1) && (instXop != -1));
        insn.clear();
        XOFORM_OP_SET(insn, instOp);
        XOFORM_RT_SET(insn, dest);
        XOFORM_RA_SET(insn, src1);
        XOFORM_RB_SET(insn, src2);
        XOFORM_XO_SET(insn, instXop);
        insnCodeGen::generate(gen,insn);
    }
  return;
}


//
// I don't know how to compute cycles for POWER instructions due to 
//   multiple functional units.  However, we can compute the number of
//   instructions and hope that is fairly close. - jkh 1/30/96
//
int getInsnCost(opCode op)
  {
    int cost = 0;

    /* XXX Need to add branchOp */
    switch (op) {
      
    case noOp:
    case loadIndirOp:
    case saveRegOp:

      // worse case is it is on the stack and takes one instruction.
    case getParamOp:

      // integer ops
    case plusOp:
    case minusOp:
    case timesOp:
    case divOp:
      cost = 1;
      break;

    case loadConstOp:
      // worse case is addi followed by ori

    case loadOp:
      // addis, l
      
    case storeOp:
    case storeIndirOp:
      cost = 2;
      break;

    case ifOp:
      // cmpi 0,0,src1,0
      // be 0, dest
      // nop
      cost = 3;
      break;

      // rel ops
    case eqOp:
    case neOp:
    case lessOp:
    case greaterOp:
    case leOp:
    case geOp:
      cost = 4;
      break;
      
    case callOp:
      // mflr r0
      // st r0, (r1)
      cost += 2;
      
      // Should compute the cost to save registers here.  However, we lack 
      //   sufficient information to compute this value. We need to be 
      //   inside the code generator to know this amount.
      //
      // We know it is at *least* every live register (i.e. parameter reg)
      cost += 13;
      
      // clr r5
      // clr r6
      // decrement stack pointer -- STUop
      // load r0 with address, then move to link reg and branch and link.
      // ori dest,dest,LOW(src1)
      // mtlr	0 (aka mtspr 8, rs) = 0x7c0803a6
      // brl - branch and link through the link reg.
      cost += 7;
      
      // now cleanup.
      
      // increment stack pointer -- CALop
      // restore the saved register 0.
      cost += 2;
      
      // Should compute the cost to restore registers here.  However, we lack 
      //   sufficient information to compute this value. We need to be 
      //   inside the code generator to know this amount.
      //
      // We know it is at *least* every live register (i.e. parameter reg)
      cost += 13;
      
      // mtlr	0 
      cost++;
      break;

    case updateCostOp:
      // In most cases this cost is 4, but if we can't use CALop because
      // the value is to large, then we'll need 2 additional operations to
      // load that value - naim
      // Why the +=? Isn't it equivalent to an = here? -- bernat
      cost += 4;
      break;
      
    case trampPreamble:
      // Generate code to update the observed cost.
      // generated code moved to the base trampoline.
      cost += 0;
      break;

    default:
      cost = 0;
      break;
    }
    return (cost);
}

#if 0
// What does this do???
void registerSpace::saveClobberInfo(const instPoint *location)
{
  registerSlot *regSlot = NULL;
  registerSlot *regFPSlot = NULL;
  if (location == NULL)
    return;
  if (location->actualGPRLiveSet_ != NULL && location->actualFPRLiveSet_ != NULL)
    {
      
      // REG guard registers, if live, must be saved
      if (location->actualGPRLiveSet_[ REG_GUARD_ADDR ] == LIVE_REG)
	location->actualGPRLiveSet_[ REG_GUARD_ADDR ] = LIVE_CLOBBERED_REG;
      
      if (location->actualGPRLiveSet_[ REG_GUARD_OFFSET ] == LIVE_REG)
	location->actualGPRLiveSet_[ REG_GUARD_OFFSET ] = LIVE_CLOBBERED_REG;

      // GPR and FPR scratch registers, if live, must be saved
      if (location->actualGPRLiveSet_[ REG_SCRATCH ] == LIVE_REG)
	location->actualGPRLiveSet_[ REG_SCRATCH ] = LIVE_CLOBBERED_REG;

      if (location->actualFPRLiveSet_[ REG_SCRATCH ] == LIVE_REG)
	location->actualFPRLiveSet_[ REG_SCRATCH ] = LIVE_CLOBBERED_REG;

      // Return func call register, since we make a call because
      // of multithreading (regardless if it's threaded) from BT
      // we must save return register
      if (location->actualGPRLiveSet_[ 3 ] == LIVE_REG)
	location->actualGPRLiveSet_[ 3 ] = LIVE_CLOBBERED_REG;

    
      for (u_int i = 0; i < getRegisterCount(); i++)
	{
	  regSlot = getRegSlot(i);

	  if (  location->actualGPRLiveSet_[ (int) registers[i].number ] == LIVE_REG )
	    {
	      if (!registers[i].beenClobbered)
		location->actualGPRLiveSet_[ (int) registers[i].number ] = LIVE_UNCLOBBERED_REG;
	      else
		location->actualGPRLiveSet_[ (int) registers[i].number ] = LIVE_CLOBBERED_REG;
	    }


	  if (  location->actualGPRLiveSet_[ (int) registers[i].number ] == LIVE_UNCLOBBERED_REG ) 
	    {
	      if (registers[i].beenClobbered)
		location->actualGPRLiveSet_[ (int) registers[i].number ] = LIVE_CLOBBERED_REG;
	    }
	}
	  
      for (u_int i = 0; i < getFPRegisterCount(); i++)
	{
	  regFPSlot = getFPRegSlot(i);
	  
	  if (  location->actualFPRLiveSet_[ (int) fpRegisters[i].number ] == LIVE_REG )
	    {
	      if (!fpRegisters[i].beenClobbered)
		location->actualFPRLiveSet_[ (int) fpRegisters[i].number ] = LIVE_UNCLOBBERED_REG;
	      else
		location->actualFPRLiveSet_[ (int) fpRegisters[i].number ] = LIVE_CLOBBERED_REG;
	    }
	  
	  if (  location->actualFPRLiveSet_[ (int) fpRegisters[i].number ] == LIVE_UNCLOBBERED_REG )
	    {
	      if (fpRegisters[i].beenClobbered)
		location->actualFPRLiveSet_[ (int) fpRegisters[i].number ] = LIVE_CLOBBERED_REG;
	    }
	}
    }
}
#endif

bool doNotOverflow(int64_t value) {
    if ( (value <= 32767) && (value >= -32768) ) return(true);
    else return(false);

}

// hasBeenBound: returns true if the runtime linker has bound the
// function symbol corresponding to the relocation entry in at the address
// specified by entry and base_addr.  If it has been bound, then the callee 
// function is returned in "target_pdf", else it returns false.
bool PCProcess::hasBeenBound(const SymtabAPI::relocationEntry &entry, 
		func_instance *&target_pdf, Address base_addr) 
{

	if (isTerminated()) return false;

	// if the relocationEntry has not been bound yet, then the value
	// at rel_addr is the address of the instruction immediately following
	// the first instruction in the PLT entry (which is at the target_addr) 
	// The PLT entries are never modified, instead they use an indirrect 
	// jump to an address stored in the _GLOBAL_OFFSET_TABLE_.  When the 
	// function symbol is bound by the runtime linker, it changes the address
	// in the _GLOBAL_OFFSET_TABLE_ corresponding to the PLT entry

	Address got_entry = entry.rel_addr() + base_addr;
	Address bound_addr = 0;
	if (!readDataSpace((const void*)got_entry, sizeof(Address),
				&bound_addr, true)){
		sprintf(errorLine, "read error in PCProcess::hasBeenBound addr 0x%x, pid=%d\n (readDataSpace returns 0)",(unsigned)got_entry,getPid());
		logLine(errorLine);
		//print_read_error_info(entry, target_pdf, base_addr);
		fprintf(stderr, "%s[%d]: %s\n", FILE__, __LINE__, errorLine);
		return false;
	}

   //fprintf(stderr, "%s[%d]:  hasBeenBound:  %p ?= %p ?\n", FILE__, __LINE__, bound_addr, entry.target_addr() + 6 + base_addr);
	if ( !( bound_addr == (entry.target_addr()+6+base_addr)) ) {
	  // the callee function has been bound by the runtime linker
	  // find the function and return it
	  target_pdf = findFuncByEntry(bound_addr);
	  if(!target_pdf){
	    return false;
	  }
	  return true;
	}
	return false;
}

bool PCProcess::bindPLTEntry(const SymtabAPI::relocationEntry &entry, Address base_addr, 
                           func_instance * origFunc, Address target_addr) {
   fprintf(stderr, "[PCProcess::bindPLTEntry] Relocation Entry location target: %lx, relocation: %lx - base_addr: %lx, original_function: %lx, original_name: %s, new_target: %lx\n", entry.target_addr(), entry.rel_addr(), base_addr, origFunc->getPtrAddress(), origFunc->name().c_str(), target_addr);
   //Address got_entry = entry.rel_addr() + base_addr;
   return true;//writeDataSpace((void *)got_entry, sizeof(Address), &target_addr);

   //assert(0 && "TODO!");
   // return false;
}
void emitLoadPreviousStackFrameRegister(Address register_num, 
                                        Register dest,
                                        codeGen &gen,
                                        int /*size*/,
                                        bool noCost)
{
    // As of 10/24/2007, the size parameter is still incorrect.
    // Luckily, we know implicitly what size they actually want.

    // Offset if needed
    int offset;
    // Unused, 3OCT03
    //instruction *insn_ptr = (instruction *)insn;
    // We need values to define special registers.

    switch ( (int) register_num) {
    case registerSpace::lr:
        // LR is saved on the stack
        // Note: this is only valid for non-function entry/exit instru. 
        // Once we've entered a function, the LR is stomped to point
        // at the exit tramp!
      offset = TRAMP_SPR_OFFSET(gen.width()) + STK_LR; 

        // Get address (SP + offset) and stick in register dest.
        emitImm(plusOp ,(Register) REG_SP, (RegValue) offset, dest,
                gen, noCost, gen.rs());
        // Load LR into register dest
        emitV(loadIndirOp, dest, 0, dest, gen, noCost, gen.rs(),
              gen.width(), gen.point(), gen.addrSpace());
        break;

    case registerSpace::ctr:
        // CTR is saved down the stack
        if (gen.width() == 4)
	  offset = TRAMP_SPR_OFFSET(gen.width()) + STK_CTR_32;
        else
	  offset = TRAMP_SPR_OFFSET(gen.width()) + STK_CTR_64;

        // Get address (SP + offset) and stick in register dest.
        emitImm(plusOp ,(Register) REG_SP, (RegValue) offset, dest,
                gen, noCost, gen.rs());
        // Load LR into register dest
        emitV(loadIndirOp, dest, 0, dest, gen, noCost, gen.rs(),
              gen.width(), gen.point(), gen.addrSpace());
      break;

    default:
        cerr << "Fallthrough in emitLoadPreviousStackFrameRegister" << endl;
        cerr << "Unexpected register " << register_num << endl;
        assert(0);
        break;
    }
}

void emitStorePreviousStackFrameRegister(Address,
                                         Register,
                                         codeGen &,
                                         int,
                                         bool) {
    assert(0);
}

using namespace Dyninst::InstructionAPI; 
bool AddressSpace::getDynamicCallSiteArgs(InstructionAPI::Instruction i,
					  Address addr, 
					  std::vector<AstNodePtr> &args)
{
  static RegisterAST::Ptr ctr32(new RegisterAST(ppc32::ctr));
  static RegisterAST::Ptr ctr64(new RegisterAST(ppc64::ctr));
  static RegisterAST::Ptr lr32(new RegisterAST(ppc32::lr));
  static RegisterAST::Ptr lr64(new RegisterAST(ppc64::lr));
    Register branch_target = registerSpace::ignored;

    // Is this a branch conditional link register (BCLR)
    // BCLR uses the xlform (6,5,5,5,10,1)
    for(Instruction::cftConstIter curCFT = i.cft_begin();
        curCFT != i.cft_end();
        ++curCFT)
    {
      if(curCFT->target->isUsed(ctr32) ||
	 curCFT->target->isUsed(ctr64))
        {
            branch_target = registerSpace::ctr;
            break;
        }
      else if(curCFT->target->isUsed(lr32) ||
	      curCFT->target->isUsed(lr64))
        {
	  fprintf(stderr, "setting lr\n");
            branch_target = registerSpace::lr;
            break;
        }
    }
    if(branch_target != registerSpace::ignored)
    {
        // Where we're jumping to (link register, count register)
        args.push_back( AstNode::operandNode(AstNode::operandType::origRegister,
                        (void *)(long)branch_target));

        // Where we are now
        args.push_back( AstNode::operandNode(AstNode::operandType::Constant,
                        (void *) addr));

        return true;
    }
    else
    {
      return false;
    }
}

bool writeFunctionPtr(AddressSpace *p, Address addr, func_instance *f)
{
    // 64-bit ELF PowerPC Linux uses r2 for TOC base register
    if (p->getAddressWidth() == sizeof(uint64_t)) {
        Address val_to_write = f->addr();
        // Use function descriptor address, if available.
        if (f->getPtrAddress()) val_to_write = f->getPtrAddress();
        return p->writeDataSpace((void *) addr,
                                 sizeof(val_to_write), &val_to_write);
    }
    else {
        // Originally copied from inst-x86.C
        // 32-bit ELF PowerPC Linux mutatee
        uint32_t val_to_write = (uint32_t)f->addr();
        return p->writeDataSpace((void *) addr,
                                 sizeof(val_to_write), &val_to_write);
    }
}

Emitter *AddressSpace::getEmitter() 
{
    static EmitterPOWER32Dyn emitter32Dyn;
    static EmitterPOWER64Dyn emitter64Dyn;
    static EmitterPOWER32Stat emitter32Stat;
    static EmitterPOWER64Stat emitter64Stat;

    if (getAddressWidth() == 8) {
        if (proc()) {
            return &emitter64Dyn;
        }
        else return &emitter64Stat;
    }
    if (proc())
        return &emitter32Dyn;
    else
        return &emitter32Stat;

    assert(0);
    return NULL;
}

#define GET_IP      0x429f0005
#define MFLR_30     0x7fc802a6
#define ADDIS_30_30 0x3fde0000
#define ADDI_30_30  0x3bde0000
#define LWZ_11_30   0x817e0000
#define ADDIS_11_30 0x3d7e0000

/*
 * If the target stub_addr is a glink stub, try to determine the actual
 * function called (through the GOT) and fill in that information.
 *
 * The function at stub_addr may not have been created when this method
 * is called.
 *
 * XXX Is this a candidate to move into general parsing code, or is
 *     this properly a Dyninst-only technique?
 */
bool image::updatePltFunc(parse_func *caller_func, Address stub_addr)
{
    unsigned int *stub;
    Address got2 = 0;

    if(!getPltFuncs())
        return false;

    // If we're calling an empty glink stub.
    unordered_map<Address, std::string>::iterator entry = pltFuncs->find(stub_addr);
    if (entry != pltFuncs->end() && entry->second == "@plt")
    {
        int state = 0;

        // Find GOT2 value
        auto bl = caller_func->blocks();
        auto bit = bl.begin();
        for( ; bit != bl.end(); ++bit) {
          ParseAPI::Block * b = *bit;
          for(Address addr = b->start(); addr < b->end();
              addr += instruction::size()) // XXX 4
          {
            unsigned int * caller_insn = 
                (unsigned int *)caller_func->isrc()->getPtrToInstruction(addr);

            if (state == 0 && *caller_insn == GET_IP) {
                got2 = addr + instruction::size();
                ++state;
            } else if (state == 1 && *caller_insn == MFLR_30) {
                ++state;
            } else if (state == 4) {
                break;
            } 
            else if (state >= 2 && (*caller_insn & 0xffff0000) == ADDIS_30_30)
            {
                got2 += ((signed short)(*caller_insn & 0x0000ffff)) << 16;
                ++state;
            } 
            else if (state >= 2 && (*caller_insn & 0xffff0000) == ADDI_30_30)
            {
                got2 += (signed short)(*caller_insn & 0x0000ffff);
                ++state;
            }
          }
        }
        if (state != 4) return false;

        // Find stub offset
        stub = (unsigned int *)
            caller_func->isrc()->getPtrToInstruction(stub_addr);
        int offset = 0;
        if ( (stub[0] & 0xffff0000) == LWZ_11_30) {
            offset = (signed short)(stub[0] & 0x0000ffff);
        } else if ( (stub[0] & 0xffff0000) == ADDIS_11_30) {
            offset &= (stub[0] & 0x0000ffff) << 16;
            offset &= (stub[1] & 0x0000ffff);
        }

        // Update all PLT based structures
        Address plt_addr = got2 + offset;
        (*pltFuncs)[stub_addr] = (*pltFuncs)[plt_addr];
        getObject()->updateFuncBindingTable(stub_addr, plt_addr);
    }
    return true;
}

bool EmitterPOWER::emitCallRelative(Register dest, Address offset, Register base, codeGen &gen){
    // Loads a saved register from the stack. 
    int imm = offset;
    if (gen.width() == 4) {
      if (((signed)MIN_IMM16 <= (signed)imm) && ((signed)imm <= (signed)MAX_IMM16))
        {
          insnCodeGen::generateImm (gen, CALop, dest, base, imm);

        }
      else if (((signed)MIN_IMM32 <= (signed)imm) && ((signed)imm <= (signed)MAX_IMM32))
        {
          insnCodeGen::generateImm (gen, CAUop, dest, 0, BOT_HI (offset));
          insnCodeGen::generateImm (gen, ORILop, dest, dest, BOT_LO (offset));
          insnCodeGen::generateAddReg (gen, CAXop, dest, dest, base);
        }
	else {
		assert(0);
	}
	
    }
    else {
/*        insnCodeGen::generateMemAccess64(gen, LDop, LDxop,
                                         dest,
                                         base,
                                         offset);
*/
    }
    return true;
}

bool EmitterPOWER::emitLoadRelative(Register dest, Address offset, Register base, int size, codeGen &gen){ 
  if (((long)MIN_IMM16 <= (long)offset) && ((long) offset <= (long)MAX_IMM16)) {
    int ocode = Lop;
    switch (size) {
    case 1:
      ocode = LBZop;
      break;
    case 2:
      ocode = LHZop;
      break;
    case 4:
      ocode = Lop;
      break;
    case 8:
      ocode = LDop;
      break;
    default:
      return false;
      break;
    }
    insnCodeGen::generateImm (gen, ocode, dest, base, offset);    
  }
  else {
    // Add the offset to the base register, which holds 
    // the current PC
    insnCodeGen::generateImm (gen, CAUop, base, base, HA (offset));
    insnCodeGen::generateImm (gen, CALop, base, base, LOW (offset));

    int ocode = LXop;
    int xcode = 0;
    switch (size) {
    case 1:
      xcode = LBZXxop;
      break;
    case 2:
      xcode = LHZXxop;
      break;
    case 4:
      xcode = LXxop;
      break;
    case 8:
      xcode = LDXxop;
      break;
    default:
      printf(" Unrecognized size for load operation(%d). Assuming size of 4 \n", size);
      return false;
      break;
    }
    
    instruction insn; insn.clear();
    XFORM_OP_SET(insn, ocode);
    XFORM_RT_SET(insn, dest);
    XFORM_RA_SET(insn, 0);
    XFORM_RB_SET(insn, base);
    XFORM_XO_SET(insn, xcode);
    XFORM_RC_SET(insn, 0);
    insnCodeGen::generate(gen, insn);
  }
  return true;
}


void EmitterPOWER::emitStoreRelative(Register source, Address offset, Register base, int size, codeGen &gen){
  if (((long)MIN_IMM16 <= (long)offset) && ((long) offset <= (long)MAX_IMM16)) {
    int ocode = STop;
    switch (size) {
    case 1:
      ocode = STBop;
      break;
    case 2:
      ocode = STHop;
      break;
    case 4:
      ocode = STop;
      break;
    case 8:
      ocode = STDop;
      break;
    default:
      //return false;
      assert(0);
      break;
    }
    insnCodeGen::generateImm (gen, ocode, source, base, offset);    
  }
  else {

    // Add the offset to the base register, which holds 
    // the current PC
    insnCodeGen::generateImm (gen, CAUop, base, base, HA (offset));
    insnCodeGen::generateImm (gen, CALop, base, base, LOW (offset));

    int ocode = STXop;
    int xcode = 0;
    switch (size) {
    case 1:
      xcode = STBXxop;
      break;
    case 2:
      xcode = STHXxop;
      break;
    case 4:
      xcode = STXxop;
      break;
    case 8:
      xcode = STDXxop;
      break;
    default:
      printf(" Unrecognized size for load operation(%d). Assuming size of 4 \n", size);
      //return false;
      assert(0);
      break;
    }
    
    instruction insn; insn.clear();
    XFORM_OP_SET(insn, ocode);
    XFORM_RT_SET(insn, source);
    XFORM_RA_SET(insn, 0);
    XFORM_RB_SET(insn, base);
    XFORM_XO_SET(insn, xcode);
    XFORM_RC_SET(insn, 0);
    insnCodeGen::generate(gen, insn);
  }
  //return true;
}

bool EmitterPOWER::emitMoveRegToReg(registerSlot *src,
                                    registerSlot *dest,
                                    codeGen &gen) {
    assert(dest->type == registerSlot::GPR);

    switch (src->type) {
    case registerSlot::GPR:
        insnCodeGen::generateImm(gen, ORILop, src->encoding(), dest->encoding(), 0);
        break;
    case registerSlot::SPR: {
        instruction insn;
        
        switch (src->number) {
        case registerSpace::lr:
        case registerSpace::xer:
        case registerSpace::ctr:
        case registerSpace::mq:
            insn.clear();
            XFORM_OP_SET(insn, EXTop);
            XFORM_RT_SET(insn, dest->encoding());
            XFORM_RA_SET(insn, src->encoding() & 0x1f);
            XFORM_RB_SET(insn, (src->encoding() >> 5) & 0x1f);
            XFORM_XO_SET(insn, MFSPRxop);
            insnCodeGen::generate(gen,insn);
            break;
        case registerSpace::cr:
            insn.clear();                    //mtcrf:  scratchReg
            XFXFORM_OP_SET(insn, EXTop);
            XFXFORM_RT_SET(insn, dest->encoding());
            XFXFORM_XO_SET(insn, MFCRxop);
            insnCodeGen::generate(gen,insn);
            break;
        default:
            assert(0);
            break;
        }
	break;
    }

    default:
        assert(0);
        break;
    }
    return true;
}

/*
bool EmitterPOWER32Stat::emitPIC(codeGen& gen, Address origAddr, Address relocAddr) {

      Register scratchPCReg = gen.rs()->getScratchRegister(gen, true);
      std::vector<Register> excludeReg;
      excludeReg.push_back(scratchPCReg);
      Register scratchReg = gen.rs()->getScratchRegister(gen, excludeReg, true);
      bool newStackFrame = false;
      int stack_size = 0;
      int gpr_off, fpr_off, ctr_off;
      //fprintf(stderr, " emitPIC origAddr 0x%lx reloc 0x%lx Registers PC %d scratch %d \n", origAddr, relocAddr, scratchPCReg, scratchReg);
      if ((scratchPCReg == Null_Register) || (scratchReg == Null_Register)) {
		//fprintf(stderr, " Creating new stack frame for 0x%lx to 0x%lx \n", origAddr, relocAddr);

		newStackFrame = true;
		//create new stack frame
	        gpr_off = TRAMP_GPR_OFFSET_32;
	        fpr_off = TRAMP_FPR_OFFSET_32;
	        ctr_off = STK_CTR_32;

		// Make a stack frame.
	    	pushStack(gen);

    		// Save GPRs
	      stack_size = saveGPRegisters(gen, gen.rs(), gpr_off, 2);

	      scratchPCReg = gen.rs()->getScratchRegister(gen, true);
	      assert(scratchPCReg != Null_Register);
	      excludeReg.clear();
	      excludeReg.push_back(scratchPCReg);
	      scratchReg = gen.rs()->getScratchRegister(gen, excludeReg, true);
	      assert(scratchReg != Null_Register);
	      // relocaAddr has moved since we added instructions to setup a new stack frame
	      relocAddr = relocAddr + ((stack_size + 1)*(gen.width()));
              //fprintf(stderr, " emitPIC origAddr 0x%lx reloc 0x%lx stack size %d Registers PC %d scratch %d \n", origAddr, relocAddr, stack_size, scratchPCReg, scratchReg);

	} 
	emitMovePCToReg(scratchPCReg, gen);	
	Address varOffset = origAddr - relocAddr;
	emitCallRelative(scratchReg, varOffset, scratchPCReg, gen);
      	insnCodeGen::generateMoveToLR(gen, scratchReg);
	if(newStackFrame) {
	      // GPRs
	      restoreGPRegisters(gen, gen.rs(), gpr_off);
	      popStack(gen);
	}

      return 0;
}

bool EmitterPOWER64Stat::emitPIC(codeGen& gen, Address origAddr, Address relocAddr) {
	assert(0);
	return false;
}
bool EmitterPOWERDyn::emitPIC(codeGen &gen, Address origAddr, Address relocAddr) {

	Address origRet = origAddr + 4;
	Register scratch = gen.rs()->getScratchRegister(gen, true);
	assert(scratch != Null_Register);
	instruction::loadImmIntoReg(gen, scratch, origRet);
	insnCodeGen::generateMoveToLR(gen, scratch);
	return true;

}
*/


// It seems like we should be able to do a better job...
bool EmitterPOWER32Stat::emitCallInstruction(codeGen& gen, func_instance* callee, bool /* setTOC */, Address) {
// 32 - No TOC 
// if inter module, gen PIC code
  if (gen.func()->obj() != callee->obj()) {
    return emitPLTCall(callee, gen);
  }
  //fprintf(stderr, "info: %s:%d: \n", __FILE__, __LINE__); 
  insnCodeGen::generateCall(gen, gen.currAddr(), callee->addr());
  return true;
}

bool EmitterPOWER32Stat::emitPLTCommon(func_instance *callee, bool call, codeGen &gen) {
  Register scratchReg = gen.rs()->getScratchRegister(gen, true);
  if (scratchReg == Null_Register) return false;

  Register scratchLR = Null_Register;
  std::vector<Register> excluded; excluded.push_back(scratchReg);
  scratchLR = gen.rs()->getScratchRegister(gen, excluded, true);
  if (scratchLR == Null_Register) {
    if (scratchReg == registerSpace::r0) return false;
    // We can use r0 for this, since it's volatile. 
    scratchLR = registerSpace::r0;
  }

  if (!call) {
    // Save the LR in scratchLR
    insnCodeGen::generateMoveFromLR(gen, scratchLR);
  }

  // Generate the PLT call

  Address dest = getInterModuleFuncAddr(callee, gen);  
  Address pcVal = emitMovePCToReg(scratchReg, gen);

  if (!call) {
    insnCodeGen::generateMoveToLR(gen, scratchLR);
  }

  // We can now use scratchLR

  Address varOffset = dest - pcVal;
  emitLoadRelative(scratchLR, varOffset, scratchReg, gen.width(), gen);
  
  insnCodeGen::generateMoveToCR(gen, scratchLR);

  if (!call) {
    instruction br(BCTRraw);
    insnCodeGen::generate(gen, br);
  }
  else {
    instruction brl(BCTRLraw);
    insnCodeGen::generate(gen, brl);
  }

  return true;
}


// TODO 32/64-bit? 
bool EmitterPOWER32Stat::emitPLTCall(func_instance *callee, codeGen &gen) {
  return emitPLTCommon(callee, true, gen);
}

bool EmitterPOWER32Stat::emitPLTJump(func_instance *callee, codeGen &gen) {
  return emitPLTCommon(callee, false, gen);
}

bool EmitterPOWER32Stat::emitTOCCall(block_instance *block, codeGen &gen) {
  return emitTOCCommon(block, true, gen);
}

bool EmitterPOWER32Stat::emitTOCJump(block_instance *block, codeGen &gen) {
  return emitTOCCommon(block, false, gen);
}


bool EmitterPOWER32Stat::emitTOCCommon(block_instance *block, bool call, codeGen &gen) {
  Register scratchReg = gen.rs()->getScratchRegister(gen, true);
  if (scratchReg == Null_Register) return false;

  Register scratchLR = Null_Register;
  std::vector<Register> excluded; excluded.push_back(scratchReg);
  scratchLR = gen.rs()->getScratchRegister(gen, excluded, true);
  if (scratchLR == Null_Register) {
    if (scratchReg == registerSpace::r0) return false;
    // We can use r0 for this, since it's volatile. 
    scratchReg = registerSpace::r0;
  }

  if (!call) {
    // Save the LR in scratchLR
    insnCodeGen::generateMoveFromLR(gen, scratchLR);
  }

  // Generate the PLT call

  Address dest = block->llb()->firstInsnOffset();
  Address pcVal = emitMovePCToReg(scratchReg, gen);

  if (!call) {
    insnCodeGen::generateMoveToLR(gen, scratchLR);
  }

  // We can now use scratchLR

  Address varOffset = dest - pcVal;
  emitLoadRelative(scratchLR, varOffset, scratchReg, gen.width(), gen);
  
  insnCodeGen::generateMoveToCR(gen, scratchLR);

  if (!call) {
    instruction br(BCTRraw);
    insnCodeGen::generate(gen, br);
  }
  else {
    instruction brl(BCTRLraw);
    insnCodeGen::generate(gen, brl);
  }

  return true;
}

bool EmitterPOWER64Stat::emitPLTCommon(func_instance *callee, bool call, codeGen &gen) {
/* This function generates code to call/jump to an external function.
 * The Power ABI v2 specifies that R12 should contain the callee address.
 *
 * The steps include:
 * 1. Save registers
 * 2. Create/Load the PLT entry for the callee
 * 3. Make the call
 * 4. Restore registers
 * 5. Generate a return for jump case
 */

  const unsigned TOCreg = 2;
  const unsigned wordsize = gen.width();
  assert(wordsize == 8);

  Address func_desc = getInterModuleFuncAddr(callee, gen);


  // We need a scratch register and a place to store the LR. 
  // Because modification can also call this function, there may not
  // be an instrumentation frame. So, we move down the stack before the
  // call and move up the stack after the call
  pushStack(gen);

  unsigned r_tmp = 12; // R12 ; We need to put callee address into R12 

  // Save R12
  insnCodeGen::generateMemAccess64(gen, STDop, STDxop, r_tmp, REG_SP, 3*wordsize);
  
  // Save LR
  insnCodeGen::generateMoveFromLR(gen, r_tmp);
  insnCodeGen::generateMemAccess64(gen, STDop, STDxop, r_tmp, REG_SP, 4*wordsize);

  // Save R2
  insnCodeGen::generateMemAccess64(gen, STDop, STDxop, TOCreg, REG_SP, 5*wordsize);

  // r_tmp := func_desc
  // We first load PC into R12
  Address pcVal = emitMovePCToReg(r_tmp, gen);

  Address func_desc_from_cur = func_desc - pcVal;

  // Here we use R2 as a temp register:
  // We load the offset into R2
  insnCodeGen::loadImmIntoReg(gen, TOCreg, func_desc_from_cur);
  
  // Add the offset to R12, which contains the PC
  insnCodeGen::generateAddReg(gen, CAXop, r_tmp, r_tmp, TOCreg);

  // r_tmp := *(r_tmp)
  insnCodeGen::generateMemAccess64(gen, LDop, LDxop, r_tmp, r_tmp, 0);
  
  // lr := r_tmp; Loading the call target
  insnCodeGen::generateMoveToLR(gen, r_tmp);

  // blrl
  instruction branch_insn(BRLraw);
  insnCodeGen::generate(gen, branch_insn);

  // Restore LR, R2, and R12
  insnCodeGen::generateMemAccess64(gen, LDop, LDxop, r_tmp, REG_SP, 4*wordsize);
  insnCodeGen::generateMoveToLR(gen, r_tmp);
  insnCodeGen::generateMemAccess64(gen, LDop, LDxop, r_tmp, REG_SP, 3*wordsize);
  insnCodeGen::generateMemAccess64(gen, LDop, LDxop, TOCreg, REG_SP, 5*wordsize);

  // Move back the stack
  popStack(gen);

  if (!call) {
    // We genearte a return here for jump case, 
    // because Dyninst will relocate the original function.
    // We do not want to execute the original code
    // in cases like function replacement and function wrapping.
    instruction ret(BRraw);
    insnCodeGen::generate(gen, ret);
  }

  return true;
}

bool EmitterPOWER64Dyn::emitTOCCommon(block_instance *block, bool call, codeGen &gen) {
  // This code is complicated by the need to set the new TOC and restore it
  // post-(call/branch). That means we can't use a branch if asked, since we won't
  // regain control. Fun. 
  //
  // 1. Move down the stack and save R12 and LR
  // 2. R2 := TOC of callee
  // 3. Load callee into R12 (V2 ABI requires the callee address should be in R12)
  // 4. LR := R12 
  // 5. Call LR
  // 6. Restore R12 and LR
  // 7. R2 := TOC of caller
  // 8. Move up the stack
  // IF (!call)
  //   Return
  
  const unsigned TOCreg = 2;
  const unsigned wordsize = gen.width();
  assert(wordsize == 8);
  Address dest = block->start();

  // We need the callee TOC, which we find by function, not by block. 
  std::vector<func_instance *> funcs;
  block->getFuncs(std::back_inserter(funcs));
  Address callee_toc = gen.addrSpace()->getTOCoffsetInfo(funcs[0]);
  
  Address caller_toc = 0;
  if (gen.func()) {
    caller_toc = gen.addrSpace()->getTOCoffsetInfo(gen.func());
  }
  else if (gen.point()) {
    assert(gen.point()->func());
    caller_toc = gen.addrSpace()->getTOCoffsetInfo(gen.point()->func());
  }
  else {
    // Don't need it, and this might be an iRPC
  }
  unsigned r12 = 12;

  // Move down the stack to create space for saving registers
  pushStack(gen);

  // Save R12 and LR
  insnCodeGen::generateMoveFromLR(gen, TOCreg);
  insnCodeGen::generateMemAccess64(gen, STDop, STDxop, TOCreg, REG_SP, 3*wordsize);
  insnCodeGen::generateMemAccess64(gen, STDop, STDxop, r12, REG_SP, 4*wordsize);
				     
  // Use the R12 to generate the destination address
  insnCodeGen::loadImmIntoReg(gen, r12, dest);
  insnCodeGen::generateMoveToLR(gen, r12);
  
  // Load the callee TOC
  insnCodeGen::loadImmIntoReg(gen, TOCreg, callee_toc);
  
  instruction branch_insn(BRLraw);
  insnCodeGen::generate(gen, branch_insn);

  // Restore R12 and LR
  insnCodeGen::generateMemAccess64(gen, LDop, LDxop, TOCreg, REG_SP, 3*wordsize);
  insnCodeGen::generateMoveToLR(gen, TOCreg);
  insnCodeGen::generateMemAccess64(gen, LDop, LDxop, r12, REG_SP, 4*wordsize);

  // Load caller TOC
  insnCodeGen::loadImmIntoReg(gen, TOCreg, caller_toc);

  // Move up the stack
  popStack(gen);

  if (!call) {
    instruction ret(BRraw);
    insnCodeGen::generate(gen, ret);
  }
  
  return true;
}

// TODO 32/64-bit? 
bool EmitterPOWER64Stat::emitPLTCall(func_instance *callee, codeGen &gen) {
  return emitPLTCommon(callee, true, gen);
}

bool EmitterPOWER64Stat::emitPLTJump(func_instance *callee, codeGen &gen) {
  return emitPLTCommon(callee, false, gen);
}

bool EmitterPOWER64Stat::emitTOCCall(block_instance *block, codeGen &gen) {
  return emitTOCCommon(block, true, gen);
}

bool EmitterPOWER64Stat::emitTOCJump(block_instance *block, codeGen &gen) {
  return emitTOCCommon(block, false, gen);
}

bool EmitterPOWER64Stat::emitTOCCommon(block_instance *block, bool call, codeGen &gen) {
  // Right now we can only jump to a block if it's the entry of a function
  // since it needs a relocation entry, which implies symbols, which implies... a function.
  // Theoretically, we could create symbols for this block, if anyone ever cares.
  std::vector<func_instance *> funcs;
  block->getFuncs(std::back_inserter(funcs));
  for (unsigned i = 0; i < funcs.size(); ++i) {
    if (block == funcs[i]->entry()) {
      return emitPLTCommon(funcs[i], call, gen);
    }
  }
  return false;
}

bool EmitterPOWER64Stat::emitCallInstruction(codeGen &gen,
                                             func_instance *callee,
                                             bool , Address) {
    // if the TOC changes, generate a PIC call
    Address dest =  callee->addr();
    if( dest == 0)
    	dest = getInterModuleFuncAddr(callee, gen);

 

//    if (setTOC) {
    if (gen.func()->obj() != callee->obj()) {
        return emitPLTCall(callee, gen);
    }
    // For local calls, we should not need to set R2.
    //
    // If the callee has a preamble to set up R2, we skip these two instructions.
    // For PIE code, the preamble uses R12 to set up R2; calling the global entry
    // will require setting R12 to be the global entry of the callee. So, we just
    // skip the preabmle.
    //
    // TODO: if the premable changes, the amount of bytes to skip should change as well.
    //
    if (callee->ifunc()->containsPowerPreamble())
        insnCodeGen::generateCall(gen, gen.currAddr(), dest + 8);
    else
    // For functions that do not have the R2 preabmle,
    // it means it will not change R2, we just call it.
        insnCodeGen::generateCall(gen, gen.currAddr(), dest);
    return true;
}

// Generates call instruction sequence for all POWER-based systems
// under dynamic instrumentation.
//
// This should be able to stomp on the link register (LR) and TOC
// register (r2), as they were saved by Emitter::emitCall() as necessary.
bool EmitterPOWER::emitCallInstruction(codeGen &gen, func_instance *callee, bool setTOC, Address toc_anchor) {

    //fprintf(stderr, "info: %s:%d: \n", __FILE__, __LINE__); 
    bool needLongBranch = false;
    if (gen.startAddr() == (Address) -1) { // Unset...
        needLongBranch = true;
        //fprintf(stderr, "info: %s:%d: \n", __FILE__, __LINE__); 
        inst_printf("Unknown generation addr, long call required\n");
    }
    else {
        long displacement = callee->addr() - gen.currAddr();
        // Increase the displacement to be conservative. 
        // We use fewer than 6 instructions, too. But again,
        // conservative.
        //fprintf(stderr, "info: %s:%d: %lu\n", __FILE__, __LINE__, displacement); 
        if ((ABS(displacement) + 6*instruction::size()) > MAX_BRANCH) {
            needLongBranch = true;
            //fprintf(stderr, "info: %s:%d: \n", __FILE__, __LINE__); 
            inst_printf("Need long call to get from 0x%lx to 0x%lx\n",
                    gen.currAddr(), callee->addr());
        }
    }
    // Need somewhere to put the destination calculation...
    int scratchReg = 12;
    if (needLongBranch) {
        // Use scratchReg to set destination of the call...
        inst_printf("[EmitterPOWER::EmitCallInstruction] needLongBranch, Emitting VLOAD  Callee: 0x%lx, ScratchReg: %u\n",
                    callee->addr(), (unsigned) scratchReg);
        emitVload(loadConstOp, callee->addr(), scratchReg, scratchReg, gen, false);
        //fprintf(stderr, "info: %s:%d: \n", __FILE__, __LINE__); 
        insnCodeGen::generateMoveToLR(gen, scratchReg);

        inst_printf("Generated LR value in %d\n", scratchReg);
    }


    // Linux 64
    if (setTOC) {
        inst_printf("[EmitterPOWER::EmitCallInstruction] Setting TOC anchor, toc_anchor: %lx, register: 2(fixed)\n",
                    (uint64_t)toc_anchor);
        // Set up the new TOC value
        emitVload(loadConstOp, toc_anchor, 2, 2, gen, false);
        //fprintf(stderr, "info: %s:%d: \n", __FILE__, __LINE__); 
        //inst_printf("toc setup (%d)...");
        inst_printf("Set new TOC\n");
    }

    // ALL dynamic; call instruction generation
    if (needLongBranch) {
        //insnCodeGen::generateCall(gen, gen.currAddr(), callee->addr());
        //fprintf(stderr, "info: %s:%d: \n", __FILE__, __LINE__); 
        emitVload(loadConstOp, callee->addr(), 12, 12, gen, false);
        instruction brl(BRLraw);
        insnCodeGen::generate(gen,brl);
        inst_printf("Generated BRL\n");
    }
    else {
        inst_printf("[EmitterPOWER::EmitCallInstruction] Generating Call, curAddress: %lx, calleeAddr: %lx\n",
                     gen.currAddr(), callee->addr());
        //fprintf(stderr, "info: %s:%d: \n", __FILE__, __LINE__); 
        emitVload(loadConstOp, callee->addr(), 12, 12, gen, false);
        // if (gen.currAddr() == 0x100000120f80) {
        //     fprintf(stderr, "we are here, break now\n");
        // } else {
        //     emitVload(loadConstOp, callee->addr(), 12, 12, gen, false);
        // }
        insnCodeGen::generateCall(gen, gen.currAddr(), callee->addr());

        inst_printf("Generated short call from 0x%lx to 0x%lx\n",
                gen.currAddr(), callee->addr());
    }

    return true;
}

void EmitterPOWER::emitLoadShared(opCode op, Register dest, const image_variable* var, bool is_local, int size, codeGen &gen, Address offset)
{
   // create or retrieve jump slot
   Address addr;
   int stackSize = 0;

   if(var == NULL) {
      addr = offset;
   }
   else if(!is_local) {
      addr = getInterModuleVarAddr(var, gen);
   }
   else {
      addr = (Address)var->getOffset();
   }

   // load register with address from jump slot

   inst_printf("emitLoadShared addr 0x%lx curr adress 0x%lx offset %lu 0x%lx size %d\n", 
   	addr, gen.currAddr(), addr - gen.currAddr()+4, addr - gen.currAddr()+4, size);
   Register scratchReg = gen.rs()->getScratchRegister(gen, true);

   if (scratchReg == Null_Register) {
   	std::vector<Register> freeReg;
        std::vector<Register> excludeReg;
   	stackSize = insnCodeGen::createStackFrame(gen, 1, freeReg, excludeReg);
   	assert (stackSize == 1);
   	scratchReg = freeReg[0];
   	inst_printf("emitLoadrelative - after new stack frame - addr 0x%lx curr adress 0x%lx offset %lu 0x%lx size %d\n", 
		addr, gen.currAddr(), addr - gen.currAddr()+4, addr - gen.currAddr()+4, size);
   }

   emitMovePCToReg(scratchReg, gen);
   Address varOffset = addr - gen.currAddr()+4;
   
   if (op ==loadOp) {
   	if(!is_local && (var != NULL)){

	  emitLoadRelative(dest, varOffset, scratchReg, gen.width(), gen);
	  // Deference the pointer to get the variable
	  emitLoadRelative(dest, 0, dest, size, gen);
   	} else {

	  emitLoadRelative(dest, varOffset, scratchReg, size, gen);
   	}
   } else { //loadConstop
     if(!is_local && (var != NULL)){

       emitLoadRelative(dest, varOffset, scratchReg, gen.width(), gen);
     } else {

       // Move address of the variable into the register - load effective address
       //dest = effective address of pc+offset ;
       insnCodeGen::generateImm (gen, CAUop, dest, 0, BOT_HI (varOffset));
       insnCodeGen::generateImm (gen, ORILop, dest, dest, BOT_LO (varOffset));
       insnCodeGen::generateAddReg (gen, CAXop, dest, dest, scratchReg);
     }
   }
   
   if (stackSize > 0)
   	insnCodeGen::removeStackFrame(gen);

  return;
}

void EmitterPOWER::emitStoreShared(Register source, const image_variable * var, bool is_local, int size, codeGen & gen)
{
   // create or retrieve jump slot
   Address addr;
   int stackSize = 0;
   if(!is_local) {
      addr = getInterModuleVarAddr(var, gen);
   }
   else {
      addr = (Address)var->getOffset();
   }

   inst_printf("emitStoreRelative addr 0x%lx curr adress 0x%lx offset %lu 0x%lx size %d\n",
   		addr, gen.currAddr(), addr - gen.currAddr()+4, addr - gen.currAddr()+4, size);

   // load register with address from jump slot
   Register scratchReg = gen.rs()->getScratchRegister(gen, true);
   if (scratchReg == Null_Register) {
   	std::vector<Register> freeReg;
        std::vector<Register> excludeReg;
   	stackSize = insnCodeGen::createStackFrame(gen, 1, freeReg, excludeReg);
	assert (stackSize == 1);
	scratchReg = freeReg[0];
	
   	inst_printf("emitStoreRelative - after new stack frame- addr 0x%lx curr adress 0x%lx offset %lu 0x%lx size %d\n",
   		addr, gen.currAddr(), addr - gen.currAddr()+4, addr - gen.currAddr()+4, size);
   }
   
   emitMovePCToReg(scratchReg, gen);
   Address varOffset = addr - gen.currAddr()+4;
   
   if(!is_local) {
        std::vector<Register> exclude;
        exclude.push_back(scratchReg);
   	Register scratchReg1 = gen.rs()->getScratchRegister(gen, exclude, true);
   	if (scratchReg1 == Null_Register) {
   		std::vector<Register> freeReg;
        	std::vector<Register> excludeReg;
   		stackSize = insnCodeGen::createStackFrame(gen, 1, freeReg, excludeReg);
		assert (stackSize == 1);
		scratchReg1 = freeReg[0];
	
   		inst_printf("emitStoreRelative - after new stack frame- addr 0x%lx curr adress 0x%lx offset %lu 0x%lx size %d\n",
   		addr, gen.currAddr(), addr - gen.currAddr()+4, addr - gen.currAddr()+4, size);
   	}
     	emitLoadRelative(scratchReg1, varOffset, scratchReg, gen.width(), gen);
   	emitStoreRelative(source, 0, scratchReg1, size, gen);
   } else {
   	emitStoreRelative(source, varOffset, scratchReg, size, gen);
   }
   
   if (stackSize > 0)
   	insnCodeGen::removeStackFrame(gen);
  
  return;
}

Address Emitter::getInterModuleVarAddr(const image_variable *var, codeGen& gen)
{
    AddressSpace *addrSpace = gen.addrSpace();
    if (!addrSpace)
        assert(0 && "No AddressSpace associated with codeGen object");

    BinaryEdit *binEdit = addrSpace->edit();
    Address relocation_address;

    unsigned int jump_slot_size;
    switch (addrSpace->getAddressWidth()) {
    case 4: jump_slot_size = 4; break;
    case 8: jump_slot_size = 8; break;
    default: assert(0 && "Encountered unknown address width");
    }

    if (!binEdit || !var) {
        assert(!"Invalid variable load (variable info is missing)");
    }

    // find the Symbol corresponding to the int_variable
    std::vector<SymtabAPI::Symbol *> syms;
    var->svar()->getSymbols(syms);

    if (syms.size() == 0) {
        char msg[256];
        sprintf(msg, "%s[%d]:  internal error:  cannot find symbol %s"
                , __FILE__, __LINE__, var->symTabName().c_str());
        showErrorCallback(80, msg);
        assert(0);
    }

    // try to find a dynamic symbol
    // (take first static symbol if none are found)
    SymtabAPI::Symbol *referring = syms[0];
    for (unsigned k=0; k<syms.size(); k++) {
        if (syms[k]->isInDynSymtab()) {
            referring = syms[k];
            break;
        }
    }

    // have we added this relocation already?
    relocation_address = binEdit->getDependentRelocationAddr(referring);

    if (!relocation_address) {
        // inferiorMalloc addr location and initialize to zero
        relocation_address = binEdit->inferiorMalloc(jump_slot_size);
        unsigned char dat[8] = {0};
        binEdit->writeDataSpace((void*)relocation_address, jump_slot_size, dat);

        // add write new relocation symbol/entry
        binEdit->addDependentRelocation(relocation_address, referring);
    } 

    return relocation_address;
}

Address EmitterPOWER::emitMovePCToReg(Register dest, codeGen &gen)
{
         insnCodeGen::generateBranch(gen, gen.currAddr(),  gen.currAddr()+4, true); // blrl
	 Address ret = gen.currAddr();
         insnCodeGen::generateMoveFromLR(gen, dest); // mflr
	 return ret;
}

Address Emitter::getInterModuleFuncAddr(func_instance *func, codeGen& gen)
{
    AddressSpace *addrSpace = gen.addrSpace();
    if (!addrSpace)
        assert(0 && "No AddressSpace associated with codeGen object");

    BinaryEdit *binEdit = addrSpace->edit();
    Address relocation_address;
    
    unsigned int jump_slot_size;
    switch (addrSpace->getAddressWidth()) {
    case 4: jump_slot_size =  4; break;
    case 8: 
      jump_slot_size = 24;
      break;
    default: assert(0 && "Encountered unknown address width");
    }

    if (!binEdit || !func) {
        assert(!"Invalid function call (function info is missing)");
    }

    // find the Symbol corresponding to the func_instance
    std::vector<SymtabAPI::Symbol *> syms;
    func->ifunc()->func()->getSymbols(syms);

    if (syms.size() == 0) {
        char msg[256];
        sprintf(msg, "%s[%d]:  internal error:  cannot find symbol %s"
                , __FILE__, __LINE__, func->symTabName().c_str());
        showErrorCallback(80, msg);
        assert(0);
    }

    // try to find a dynamic symbol
    // (take first static symbol if none are found)
    SymtabAPI::Symbol *referring = syms[0];
    for (unsigned k=0; k<syms.size(); k++) {
        if (syms[k]->isInDynSymtab()) {
            referring = syms[k];
            break;
        }
    }
    // have we added this relocation already?
    relocation_address = binEdit->getDependentRelocationAddr(referring);

    if (!relocation_address) {
        // inferiorMalloc addr location and initialize to zero
        relocation_address = binEdit->inferiorMalloc(jump_slot_size);
        unsigned char dat[24] = {0};
        binEdit->writeDataSpace((void*)relocation_address, jump_slot_size, dat);
        // add write new relocation symbol/entry
        binEdit->addDependentRelocation(relocation_address, referring);
    }
    return relocation_address;
}
