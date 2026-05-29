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

#include "codegen/RegControl.h"
#include "common/src/headers.h"
#include "common/src/bitmath.h"
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
#include "dyninstAPI/src/image.h"
#include "dynproc/dynProcess.h"
#include "dyninstAPI/src/inst-power.h"
#include "common/src/arch-power.h"
#include "codegen/codegen.h"
#include "common/src/stats.h"
#include "dyninstAPI/src/os.h"
#include "patching/instPoint.h" // class instPoint
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/h/BPatch.h"
#include "BPatch/BPatch_collections.h"
#include "registerSpace/registerSpace.h"
#include "dyninstAPI/src/binaryEdit.h"
#include "patching/function.h"
#include "dyninstAPI/src/mapped_object.h"
#include "parsing/parse_func.h"
#include "parseAPI/h/CFG.h"
#include "registerSpace/RegisterConversion.h"
#include "emitter.h"
#include "emit-power.h"
#include "codegen/emitters/PowerPC/generators.h"
#include "codegen/emitters/PowerPC/ppc32/EmitterPowerPC32Dyn.h"
#include "codegen/emitters/PowerPC/ppc32/EmitterPowerPC32Stat.h"
#include "codegen/emitters/PowerPC/ppc64/EmitterPowerPC64Dyn.h"
#include "codegen/emitters/PowerPC/ppc64/EmitterPowerPC64Stat.h"
#include "codegen/emitters/PowerPC/ppc64/generators.h"

#include <sstream>

#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"

using codeGenASTPtr = Dyninst::DyninstAPI::codeGenASTPtr;
using operandAST = Dyninst::DyninstAPI::operandAST;


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
             Dyninst::Register    scratchReg, //Scratch register
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
                Dyninst::Register      scratchReg, //Scratch register
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
            Dyninst::Register      scratchReg, //Scratch register
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
               Dyninst::Register      scratchReg, //Scratch register
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
            Dyninst::Register      scratchReg,  //Scratch register
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

    /////////////////////////////////////////////////////////////////////////
    //Generates instructions to save the condition codes register onto stack.
    //  Returns the number of bytes needed to store the generated
    //    instructions.
    //  The instruction storage pointer is advanced the number of 
    //    instructions generated.
    //
void saveCR(codeGen &gen,       //Instruction storage pointer
            Dyninst::Register      scratchReg, //Scratch register
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
               Dyninst::Register      scratchReg, //Scratch register
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
               Dyninst::Register      scratchReg, //Scratch fp register
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
                  Dyninst::Register      scratchReg, //Scratch fp register
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

void saveFPRegister(codeGen &gen, 
                    Dyninst::Register reg,
                    int save_off)
{
    assert("WE SHOULD NOT BE HERE" == 0);

    insnCodeGen::generateImm(gen, STFDop, 
                             reg, REG_SP, save_off + reg*FPRSIZE);
}

void restoreFPRegister(codeGen &gen,
                       Dyninst::Register source,
                       Dyninst::Register dest,
                       int save_off)
{
    assert("WE SHOULD NOT BE HERE" == 0);
    
    insnCodeGen::generateImm(gen, LFDop, 
                             dest, REG_SP, save_off + source*FPRSIZE);
}

void restoreFPRegister(codeGen &gen,
                       Dyninst::Register reg,
                       int save_off)
{
    restoreFPRegister(gen, reg, reg, save_off);
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
	    Dyninst::DyninstAPI::ppc::saveRegister(gen, reg->encoding(), save_off);
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
	    Dyninst::DyninstAPI::ppc::restoreRegister(gen, reg->encoding(), save_off);
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
