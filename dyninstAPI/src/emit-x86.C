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
 * emit-x86.C - x86 & AMD64 code generators
 * $Id: emit-x86.C,v 1.64 2008/09/11 20:14:14 mlam Exp $
 */

#include <assert.h>
#include <stdio.h>
#include "compiler_annotations.h"
#include "dyninstAPI/src/codegen.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/emit-x86.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/h/BPatch.h"
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
#include "dyninstAPI/src/registerSpace.h"

#include "dyninstAPI/src/dynProcess.h"

#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/image.h"
// get_index...
#include "dyninstAPI/src/dynThread.h"
#include "ABI.h"
#include "liveness.h"
#include "RegisterConversion.h"
#include "unaligned_memory_access.h"

const int EmitterIA32::mt_offset = -4;
#if defined(arch_x86_64)
const int EmitterAMD64::mt_offset = -8;
#endif

static void emitXMMRegsSaveRestore(codeGen& gen, bool isRestore)
{
   GET_PTR(insn, gen);
   for(int reg = 0; reg <= 7; ++reg)
   {
     registerSlot* r = (*gen.rs())[(REGNUM_XMM0 + reg)];
     if( r && r->liveState == registerSlot::dead) 
     {
       continue;
     }
     unsigned char offset = reg * 16;
     append_memory_as_byte(insn, 0x66);
     append_memory_as_byte(insn, 0x0f);
     // 6f to save, 7f to restore
     if(isRestore) 
     {
       append_memory_as_byte(insn, 0x6f);
     }
     else
     {
       append_memory_as_byte(insn, 0x7f);
     }
     
     if (reg == 0) {
       append_memory_as_byte(insn, 0x00);
     }
     else {
       unsigned char modrm = 0x40 + (0x8 * reg);
       append_memory_as_byte(insn, modrm);
       append_memory_as_byte(insn, offset);
     }
   }
   SET_PTR(insn, gen);
}

static void emitSegPrefix(Register segReg, codeGen& gen)
{
    switch(segReg) {
        case REGNUM_FS:
            emitSimpleInsn(PREFIX_SEGFS, gen);
            return;
        case REGNUM_GS:
            emitSimpleInsn(PREFIX_SEGGS, gen);
            return;
        default:
            assert(0 && "Segment register not handled");
            return;
    }
}


bool EmitterIA32::emitMoveRegToReg(Register src, Register dest, codeGen &gen) {
   RealRegister src_r = gen.rs()->loadVirtual(src, gen);
   RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
   emitMovRegToReg(dest_r, src_r, gen);
   return true;
}

void EmitterIA32::emitLEA(Register base, Register index, unsigned int scale, int disp, Register dest, codeGen& gen)
{
    Register tmp_base = base;
    Register tmp_index = index;
    Register tmp_dest = dest;
    ::emitLEA(RealRegister(tmp_base), RealRegister(tmp_index), scale, disp,
            RealRegister(tmp_dest), gen);
    gen.markRegDefined(dest);
}

codeBufIndex_t EmitterIA32::emitIf(Register expr_reg, Register target, RegControl rc, codeGen &gen)
{
   RealRegister r = gen.rs()->loadVirtual(expr_reg, gen);
   emitOpRegReg(TEST_EV_GV, r, r, gen);
   
   // Retval: where the jump is in this sequence
   codeBufIndex_t retval = gen.getIndex();
   
   // Jump displacements are from the end of the insn, not start. The
   // one we're emitting has a size of 6.
   int32_t disp = 0;
   if (target)
      disp = target - 6;
   
   if (rc == rc_before_jump)
      gen.rs()->pushNewRegState();
   GET_PTR(insn, gen);
   // je dest
   append_memory_as_byte(insn, 0x0F);
   append_memory_as_byte(insn, 0x84);
   write_memory_as(insn, int32_t{disp});
   if (disp == 0) {
     SET_PTR(insn, gen);
     gen.addPatch(gen.getIndex(), NULL, sizeof(int), relocPatch::patch_type_t::pcrel, 
		  gen.used() + sizeof(int));
     REGET_PTR(insn, gen);
   }
   insn += sizeof(int);
   SET_PTR(insn, gen);
   
   return retval;
}

void EmitterIA32::emitOp(unsigned opcode, Register dest, Register src1, Register src2, codeGen &gen)
{
   RealRegister src1_r = gen.rs()->loadVirtual(src1, gen);
   RealRegister src2_r = gen.rs()->loadVirtual(src2, gen);
   RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
   emitMovRegToReg(dest_r, src1_r, gen);
   emitOpRegReg(opcode, dest_r, src2_r, gen);
}

void EmitterIA32::emitRelOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen, bool s)
{
   RealRegister src1_r = gen.rs()->loadVirtual(src1, gen);
   RealRegister src2_r = gen.rs()->loadVirtual(src2, gen);
   RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);   
   Register scratch = gen.rs()->allocateRegister(gen, true);
   RealRegister scratch_r = gen.rs()->loadVirtualForWrite(scratch, gen);

   emitOpRegReg(XOR_R32_RM32, dest_r, dest_r, gen); //XOR dest,dest
   emitMovImmToReg(scratch_r, 0x1, gen);            //MOV $2,scratch
   emitOpRegReg(CMP_GV_EV, src1_r, src2_r, gen);    //CMP src1, src2
   
   unsigned char opcode = cmovOpcodeFromRelOp(op, s); 
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0x0f);
   SET_PTR(insn, gen);
   emitOpRegReg(opcode, dest_r, scratch_r, gen);               //CMOVcc scratch,dest
   gen.rs()->freeRegister(scratch);
}

void EmitterIA32::emitDiv(Register dest, Register src1, Register src2, codeGen &gen, bool s)
{
   Register scratch = gen.rs()->allocateRegister(gen, true);
   gen.rs()->loadVirtualToSpecific(src1, RealRegister(REGNUM_EAX), gen);
   gen.rs()->makeRegisterAvail(RealRegister(REGNUM_EDX), gen);
   gen.rs()->noteVirtualInReal(scratch, RealRegister(REGNUM_EDX));
   RealRegister src2_r = gen.rs()->loadVirtual(src2, gen);                          
   gen.rs()->makeRegisterAvail(RealRegister(REGNUM_EAX), gen);
   emitSimpleInsn(0x99, gen);            //cdq (src1 -> eax:edx)
   if (s)
       emitOpExtReg(0xF7, 0x7, src2_r, gen); //idiv eax:edx,src2 -> eax
   else
       emitOpExtReg(0xF7, 0x6, src2_r, gen); //div eax:edx,src2 -> eax
   gen.rs()->noteVirtualInReal(dest, RealRegister(REGNUM_EAX));
   gen.rs()->freeRegister(scratch);
}

void EmitterIA32::emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, 
                            RegValue src2imm, codeGen &gen)
{
   RealRegister src1_r = gen.rs()->loadVirtual(src1, gen);
   RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
   if (src1 != dest) {
      emitMovRegToReg(dest_r, src1_r, gen);
   }
   emitOpExtRegImm(opcode1, (char) opcode2, dest_r, src2imm, gen);
}

void EmitterIA32::emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm, codeGen &gen, bool s)
{

   Register src2 = gen.rs()->allocateRegister(gen, true);
   emitLoadConst(src2, src2imm, gen);
   emitRelOp(op, dest, src1, src2, gen, s);
   gen.rs()->freeRegister(src2);
}

// where is this defined?
extern bool isPowerOf2(int value, int &result);

void EmitterIA32::emitTimesImm(Register dest, Register src1, RegValue src2imm, codeGen &gen)
{
   int result;
   
   RealRegister src1_r = gen.rs()->loadVirtual(src1, gen);
   RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);

   if (src2imm == 1) {
      emitMovRegToReg(dest_r, src1_r, gen);
      return;
   }


   if (isPowerOf2(src2imm, result) && result <= MAX_IMM8) {
      // sal dest, result
      if (src1 != dest)
         emitMovRegToReg(dest_r, src1_r, gen);
      emitOpExtRegImm8(0xC1, 4, dest_r, static_cast<char>(result), gen);
   }
   else {
      // imul src1 * src2imm -> dest_r
      emitOpRegRegImm(0x69, dest_r, src1_r, src2imm, gen);
   } 
}

void EmitterIA32::emitDivImm(Register dest, Register src1, RegValue src2imm, codeGen &gen, bool s)
{
   int result;
   if (src2imm == 1)
      return;

   if (isPowerOf2(src2imm, result) && result <= MAX_IMM8) {
      RealRegister src1_r = gen.rs()->loadVirtual(src1, gen);
      RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);

      if (src1 != dest)
         emitMovRegToReg(dest_r, src1_r, gen);
      if (s)
          // sar dest, result
          emitOpExtRegImm8(0xC1, 7, dest_r, static_cast<unsigned char>(result), gen);
      else
          // shr dest, result
          emitOpExtRegImm8(0xC1, 5, dest_r, static_cast<unsigned char>(result), gen);

   }
   else {
      Register src2 = gen.rs()->allocateRegister(gen, true);
      emitLoadConst(src2, src2imm, gen);
      emitDiv(dest, src1, src2, gen, s);
      gen.rs()->freeRegister(src2);
   }
}

void EmitterIA32::emitLoad(Register dest, Address addr, int size, codeGen &gen)
{
   RealRegister r = gen.rs()->loadVirtualForWrite(dest, gen);
   if (size == 1) {
      emitMovMBToReg(r, addr, gen);               // movsbl eax, addr
   } else if (size == 2) {
      emitMovMWToReg(r, addr, gen);               // movswl eax, addr
   } else {
      emitMovMToReg(r, addr, gen);               // mov eax, addr
   }
}

void EmitterIA32::emitLoadConst(Register dest, Address imm, codeGen &gen)
{
   RealRegister r = gen.rs()->loadVirtualForWrite(dest, gen);
   emitMovImmToReg(r, imm, gen);
}

void EmitterIA32::emitLoadIndir(Register dest, Register addr_reg, int /*size*/, codeGen &gen)
{
   RealRegister dest_r(-1);
   RealRegister src_r = gen.rs()->loadVirtual(addr_reg, gen);
   if (dest != addr_reg)
      dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
   else
      dest_r = src_r;
   emitMovRMToReg(dest_r, src_r, 0, gen);
}

void EmitterIA32::emitLoadOrigFrameRelative(Register dest, Address offset, codeGen &gen)
{
   if (gen.bt()->createdFrame) {
      Register scratch = gen.rs()->allocateRegister(gen, true);
      RealRegister scratch_r = gen.rs()->loadVirtualForWrite(scratch, gen);
      RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
      emitMovRMToReg(scratch_r, RealRegister(REGNUM_EBP), 0, gen);
      emitMovRMToReg(dest_r, scratch_r, offset, gen);
      gen.rs()->freeRegister(scratch);
      return;
   }

   RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
   emitMovRMToReg(dest_r, RealRegister(REGNUM_EBP), offset, gen);
}

bool EmitterIA32::emitLoadRelative(Register /*dest*/, Address /*offset*/, Register /*base*/, int /*size*/,
                                   codeGen &/*gen*/)
{
    assert(0);
    return false;
}

bool EmitterIA32::emitLoadRelativeSegReg(Register /*dest*/, Address offset, Register base, int /*size*/, codeGen &gen)
{
    // WARNING: dest is hard-coded to EAX currently
    emitSegPrefix(base, gen);
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, 0xa1);
    append_memory_as_byte(insn, offset);
    append_memory_as_byte(insn, 0x00);
    append_memory_as_byte(insn, 0x00);
    append_memory_as_byte(insn, 0x00);
    SET_PTR(insn, gen);
    return true;
}

void EmitterIA32::emitStoreRelative(Register /*src*/, Address /*offset*/, 
                                    Register /*base*/, int /*size*/, codeGen &/*gen*/)
{
    assert(0);
    return;
}

void EmitterIA32::emitLoadOrigRegRelative(Register dest, Address offset,
                                          Register base, codeGen &gen,
                                          bool store)
{

   RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
   restoreGPRtoGPR(RealRegister(base), dest_r, gen);
   // either load the address or the contents at that address
   if(store) 
   {
      // dest = [reg](offset)
      emitMovRMToReg(dest_r, dest_r, offset, gen);
   }
   else //calc address
   {
      //add offset,eax
      emitAddRegImm32(dest_r, offset, gen);
   }
} 

void EmitterIA32::emitLoadFrameAddr(Register dest, Address offset, codeGen &gen)
{
   RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
   restoreGPRtoReg(RealRegister(REGNUM_EBP), gen, &dest_r);
   emitAddRegImm32(dest_r, offset, gen);
}



void EmitterIA32::emitLoadOrigRegister(Address register_num, Register dest, codeGen &gen)
{
   RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
   restoreGPRtoGPR(RealRegister(register_num), dest_r, gen);

}

void EmitterIA32::emitStoreOrigRegister(Address register_num, Register src, codeGen &gen) {

   assert(0); //MATT TODO
   //Previous stack frame register is stored on the stack,
    //it was stored there at the begining of the base tramp.
    
    //Calculate the register's offset from the frame pointer in REGNUM_EBP
    unsigned offset = SAVED_EAX_OFFSET - (register_num * 4);

    emitMovRMToReg(RealRegister(REGNUM_EAX), RealRegister(REGNUM_EBP), -1*(src*4), gen);
    gen.markRegDefined(REGNUM_EAX);
    emitMovRegToRM(RealRegister(REGNUM_EBP), offset, RealRegister(REGNUM_EAX), gen);
}

void EmitterIA32::emitStore(Address addr, Register src, int size, codeGen &gen)
{
   RealRegister r = gen.rs()->loadVirtual(src, gen);
   if (size == 1) {
      emitMovRegToMB(addr, r, gen);
   } else if (size == 2) {
      emitMovRegToMW(addr, r, gen);
   } else {
      emitMovRegToM(addr, r, gen);
   }
}

void EmitterIA32::emitStoreIndir(Register addr_reg, Register src, int /*size*/, codeGen &gen)
{
   RealRegister src_r = gen.rs()->loadVirtual(src, gen);
   RealRegister addr_r = gen.rs()->loadVirtual(addr_reg, gen);
   emitMovRegToRM(addr_r, 0, src_r, gen);
}

void EmitterIA32::emitStoreFrameRelative(Address offset, Register src, Register scratch, int /*size*/, codeGen &gen)
{
   if (gen.bt()->createdFrame) 
   {
      RealRegister src_r = gen.rs()->loadVirtual(src, gen);
      RealRegister scratch_r = gen.rs()->loadVirtual(scratch, gen);
      emitMovRMToReg(scratch_r, RealRegister(REGNUM_EBP), 0, gen);
      emitMovRegToRM(scratch_r, offset, src_r, gen);
      return;
   }
   RealRegister src_r = gen.rs()->loadVirtual(src, gen);
   emitMovRegToRM(RealRegister(REGNUM_EBP), offset, src_r, gen);
}

void EmitterIA32::emitGetRetVal(Register dest, bool addr_of, codeGen &gen)
{
   RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
   if (!addr_of) {
      restoreGPRtoGPR(RealRegister(REGNUM_EAX), dest_r, gen);
      return;
   }
  
   //EAX isn't really defined here, but this will make the code generator
   //put it onto the stack and thus guarentee that we'll have an
   //address to access it at.
   gen.markRegDefined(REGNUM_EAX);
   stackItemLocation loc = getHeightOf(stackItem::framebase, gen);

   std::vector<registerSlot *> &regs = gen.rs()->trampRegs();
   registerSlot *eax = NULL;
   for (unsigned i=0; i<regs.size(); i++) {
      if (regs[i]->encoding() == REGNUM_EAX) {
         eax = regs[i];
         break;
      }
   }
   assert(eax);

   loc.offset += (eax->saveOffset * 4);
   ::emitLEA(loc.reg, RealRegister(Null_Register), 0, loc.offset, dest_r, gen);
}

void EmitterIA32::emitGetRetAddr(Register dest, codeGen &gen)
{
   // Parameters are addressed by a positive offset from ebp,
   // the first is PARAM_OFFSET[ebp]
   stackItemLocation loc = getHeightOf(stackItem::stacktop, gen);
   RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
   if (!gen.bt() || gen.bt()->alignedStack) {
       // Load the original %esp value into dest_r
       emitMovRMToReg(dest_r, loc.reg, loc.offset, gen);
       loc.offset = 0;
       loc.reg = dest_r;
   }
   emitMovRMToReg(dest_r, loc.reg, loc.offset, gen);
}

void EmitterIA32::emitGetParam(Register dest, Register param_num,
                               instPoint::Type pt_type, opCode op, 
                               bool addr_of, codeGen &gen)
{
   // Parameters are addressed by a positive offset from ebp,
   // the first is PARAM_OFFSET[ebp]
   stackItemLocation loc = getHeightOf(stackItem::stacktop, gen);
   RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
   if (!gen.bt() || gen.bt()->alignedStack) {
       // Load the original %esp value into dest_r
       emitMovRMToReg(dest_r, loc.reg, loc.offset, gen);
       loc.offset = 0;
       loc.reg = dest_r;
   }
   
   switch (op) {
       case getParamOp: 
           // guess whether we're at the call or the function entry point,
           // in which case we need to skip the return value
          if (pt_type == instPoint::FuncEntry) {
             loc.offset += 4;
          }
          break;
       case getParamAtCallOp:
           break;
       case getParamAtEntryOp:
           loc.offset += 4;
           break;
       default:
           assert(0);
           break;
   }

   loc.offset += param_num*4;

   // Prepare a real destination register.

   if (!addr_of)
      emitMovRMToReg(dest_r, loc.reg, loc.offset, gen);
   else
      ::emitLEA(loc.reg, RealRegister(Null_Register),
              0, loc.offset, dest_r, gen);
}

// Moves stack pointer by offset and aligns it to IA32_STACK_ALIGNMENT
// with the following sequence:
//
//     lea    -off(%esp) => %esp           # move %esp down
//     mov    %eax => saveSlot1(%esp)      # save %eax onto stack
//     lahf                                # save %eflags byte into %ah
//     seto   %al                          # save overflow flag into %al
//     mov    %eax => saveSlot2(%esp)      # save flags %eax onto stack
//     lea    off(%esp) => %eax            # store original %esp in %eax
//     and    -$IA32_STACK_ALIGNMENT,%esp  # align %esp
//     mov    %eax => (%esp)               # store original %esp on stack
//     mov    -off+saveSlot2(%eax) => %eax # restore flags %eax from stack
//     add    $0x7f,%al                    # restore overflow flag from %al
//     sahf                                # restore %eflags byte from %ah
//     mov    (%esp) => %eax               # re-load old %esp into %eax to ...
//     mov    -off+saveSlot1(%eax) => %eax # ... restore %eax from stack
//
// This sequence has three important properties:
//     1) It never *directly* writes to memory below %esp.  It always begins
//        by moving %esp down, then writing to locations above it.  This way,
//        if the kernel decides to interrupt, it won't stomp all over our
//        values before we get a chance to use them.
//     2) It is designed to support easy de-allocation of this space by
//        ending with %esp pointing to where we stored the original %esp.
//     3) Care has been taken to properly restore both %eax and %eflags
//        by using "lea" instead of "add" or "sub," and saving the necessary
//        flags around the "and" instruction.
//
// Saving of the flags register can be skipped if the register is not live.

void EmitterIA32::emitStackAlign(int offset, codeGen &gen)
{
    int off = offset + 4 + IA32_STACK_ALIGNMENT;
    int saveSlot1 =    0 + IA32_STACK_ALIGNMENT;
    int saveSlot2 =    4 + IA32_STACK_ALIGNMENT;
    RealRegister esp = RealRegister(REGNUM_ESP);
    RealRegister eax = RealRegister(REGNUM_EAX);
    RealRegister enull = RealRegister(Null_Register);

    bool saveFlags = false;
    if (gen.rs()->checkVolatileRegisters(gen, registerSlot::live)) {
        saveFlags = true;   // We need to save the flags register
        off += 4;           // Allocate stack space to store the flags
    }

    ::emitLEA(esp, enull, 0, -off, esp, gen);
    emitMovRegToRM(esp, saveSlot1, eax, gen);
    if (saveFlags) {
        emitSimpleInsn(0x9f, gen);
        emitSaveO(gen);
        emitMovRegToRM(esp, saveSlot2, eax, gen);
    }
    ::emitLEA(esp, enull, 0, off, eax, gen);
    emitOpExtRegImm8(0x83, EXTENDED_0x83_AND, esp, -IA32_STACK_ALIGNMENT, gen);
    emitMovRegToRM(esp, 0, eax, gen);
    if (saveFlags) {
        emitMovRMToReg(eax, eax, -off + saveSlot2, gen);
        emitRestoreO(gen);
        emitSimpleInsn(0x9e, gen);
        emitMovRMToReg(eax, esp, 0, gen);
    }
    emitMovRMToReg(eax, eax, -off + saveSlot1, gen);
}

static int extra_space_check;
bool EmitterIA32::emitBTSaves(baseTramp* bt, codeGen &gen)
{
    // x86 linux platforms do not allow for writing to memory
    // below the stack pointer.  No need to skip a "red zone."

   gen.setInInstrumentation(true);


    int instFrameSize = 0; // Tracks how much we are moving %rsp
    int funcJumpSlotSize = 0;
    if (bt) {
        funcJumpSlotSize = bt->funcJumpSlotSize() * 4;
    }

    // Align the stack now to avoid having a padding hole in the middle of
    // our instrumentation stack.  Referring to anything on the stack above
    // this point will require an indirect reference.
    //
    // There are two cases that require a 16-byte aligned stack pointer:
    //
    //    - Any time we need to save the FP registers
    //    - Any time we may execute SSE/SSE2 instructions
    //
    // The second case is only possible if we generate a function call
    // so search the ASTs for function call generation.
    //
    bool useFPRs =  BPatch::bpatch->isForceSaveFPROn() ||
                  ( BPatch::bpatch->isSaveFPROn()      &&
                    gen.rs()->anyLiveFPRsAtEntry()     &&
                    bt->saveFPRs() &&
                    bt->makesCall() );
    bool alignStack = useFPRs || !bt || bt->checkForFuncCalls();

    if (alignStack) {
        emitStackAlign(funcJumpSlotSize, gen);

    } else if (funcJumpSlotSize > 0) {
        // Just move %esp to make room for the funcJump.
        // Use LEA to avoid flag modification.
        ::emitLEA(RealRegister(REGNUM_ESP), RealRegister(Null_Register), 0,
                -funcJumpSlotSize, RealRegister(REGNUM_ESP), gen);
        instFrameSize += funcJumpSlotSize;
    }


    bool flags_saved = gen.rs()->saveVolatileRegisters(gen);
    // makesCall was added because our code spills registers around function
    // calls, and needs somewhere for those spills to go
    bool createFrame = !bt || bt->needsFrame() || useFPRs || bt->makesCall();
    bool saveOrigAddr = createFrame && bt->instP();
    bool localSpace = createFrame || useFPRs || 
       (bt && bt->validOptimizationInfo() && bt->spilledRegisters);

    if (bt) {
       bt->savedFPRs = useFPRs;
       bt->createdFrame = createFrame;
       bt->savedOrigAddr = saveOrigAddr;
       bt->createdLocalSpace = localSpace;
       bt->alignedStack = alignStack;
       bt->savedFlags = flags_saved;
    }

    int flags_saved_i = flags_saved ? 1 : 0;
    int base_i = (saveOrigAddr ? 1 : 0) + (createFrame ? 1 : 0);

    int num_saved = 0;
    int numRegsUsed = bt ? bt->numDefinedRegs() : -1;
    if (numRegsUsed == -1 || 
        numRegsUsed > X86_REGS_SAVE_LIMIT)
    {
       emitSimpleInsn(PUSHAD, gen);
       gen.rs()->incStack(8 * 4);
       num_saved = 8;

       gen.rs()->markSavedRegister(RealRegister(REGNUM_EAX), 7 + flags_saved_i + base_i);
       if(flags_saved)
       {
           gen.rs()->markSavedRegister(IA32_FLAG_VIRTUAL_REGISTER, 7 + base_i);
       }
       gen.rs()->markSavedRegister(RealRegister(REGNUM_ECX), 6 + base_i);
       gen.rs()->markSavedRegister(RealRegister(REGNUM_EDX), 5 + base_i);
       gen.rs()->markSavedRegister(RealRegister(REGNUM_EBX), 4 + base_i);
       gen.rs()->markSavedRegister(RealRegister(REGNUM_ESP), 3 + base_i);
       if (!createFrame)
          gen.rs()->markSavedRegister(RealRegister(REGNUM_EBP), 2 + base_i);
       gen.rs()->markSavedRegister(RealRegister(REGNUM_ESI), 1 + base_i);
       gen.rs()->markSavedRegister(RealRegister(REGNUM_EDI), 0 + base_i);
    }
    else
    {
       std::vector<registerSlot *> &regs = gen.rs()->trampRegs();
       for (unsigned i=0; i<regs.size(); i++) {
          registerSlot *reg = regs[i];
          if (bt->definedRegs[reg->encoding()]) {
             ::emitPush(RealRegister(reg->encoding()), gen);
             int eax_flags = (reg->encoding() == REGNUM_EAX) ? flags_saved_i : 0;
             gen.rs()->markSavedRegister(RealRegister(reg->encoding()),
                                         numRegsUsed - num_saved +
                                         base_i - 1 + eax_flags);
             if(eax_flags)
             {
                 gen.rs()->markSavedRegister(IA32_FLAG_VIRTUAL_REGISTER,
                    numRegsUsed - num_saved + base_i - 1);
             }
             num_saved++;
          }
       }
       assert(num_saved == numRegsUsed);
    }

    if (saveOrigAddr) {
       emitPushImm(bt->instP()->addr_compat(), gen);
    }
    if (createFrame)
    {
       // For now, we'll do all saves then do the guard. Could inline
       // Return addr for stack frame walking; for lack of a better idea,
       // we grab the original instPoint address
       emitSimpleInsn(PUSH_EBP, gen);
       gen.rs()->incStack(4);
       emitMovRegToReg(RealRegister(REGNUM_EBP), RealRegister(REGNUM_ESP), gen);
       gen.rs()->markSavedRegister(RealRegister(REGNUM_EBP), 0);
    }

    // Not sure liveness touches this yet, so not using
    //bool liveFPRs = (gen.rs()->FPRs()[0]->liveState == registerSlot:live);

    // Prepare our stack bookkeeping data structures.
    instFrameSize += (flags_saved_i + num_saved + base_i) * 4;
    if (bt) {
       bt->stackHeight = instFrameSize;
    }
    gen.rs()->setInstFrameSize(instFrameSize);
    gen.rs()->setStackHeight(0);

    // Pre-calculate space for temporaries and floating-point state.
    int extra_space = 0;
    if (useFPRs) {
        if (gen.rs()->hasXMM) {
            extra_space += TRAMP_FRAME_SIZE + 512;
        } else {
            extra_space += TRAMP_FRAME_SIZE + FSAVE_STATE_SIZE;
        }

    } else if (localSpace) {
        extra_space += TRAMP_FRAME_SIZE;
    }

    // Make sure that we're still aligned when we add extra_space to the stack.
    if (alignStack) {
        if ((instFrameSize + extra_space) % IA32_STACK_ALIGNMENT)
            extra_space += IA32_STACK_ALIGNMENT -
                ((instFrameSize + extra_space) % IA32_STACK_ALIGNMENT);
    }

    if (extra_space) {
        ::emitLEA(RealRegister(REGNUM_ESP), RealRegister(Null_Register), 0,
                -extra_space, RealRegister(REGNUM_ESP), gen);
        gen.rs()->incStack(extra_space);
    }
    extra_space_check = extra_space;

    if (useFPRs) {
        if (gen.rs()->hasXMM) {
           // need to save the floating point state (x87, MMX, SSE)
           // We're guaranteed to be 16-byte aligned now, so just
           // emit the fxsave.

           // fxsave (%esp) ; 0x0f 0xae 0x04 0x24
           GET_PTR(insn, gen);
           append_memory_as_byte(insn, 0x0f);
           append_memory_as_byte(insn, 0xae);
           append_memory_as_byte(insn, 0x04);
           append_memory_as_byte(insn, 0x24);
           SET_PTR(insn, gen);
        }
        else {
           emitOpRegRM(FSAVE, RealRegister(FSAVE_OP),
                       RealRegister(REGNUM_ESP), 0, gen);
        }
    }

    return true;
}

bool EmitterIA32::emitBTRestores(baseTramp* bt,codeGen &gen)
{
    bool useFPRs;
    bool createFrame;
    bool saveOrigAddr;
    bool alignStack;
    if (bt) {
       useFPRs = bt->savedFPRs;
       createFrame = bt->createdFrame;
       saveOrigAddr = bt->savedOrigAddr;
       alignStack = bt->alignedStack;
    }
    else {
       useFPRs =  BPatch::bpatch->isForceSaveFPROn() ||
                ( BPatch::bpatch->isSaveFPROn()      &&
                  gen.rs()->anyLiveFPRsAtEntry());
       createFrame = true;
       saveOrigAddr = false;
       alignStack = true;
    }

    if (useFPRs) {
        if (gen.rs()->hasXMM) {
            // restore saved FP state
            // fxrstor (%rsp) ; 0x0f 0xae 0x04 0x24
            GET_PTR(insn, gen);
            append_memory_as_byte(insn, 0x0f);
            append_memory_as_byte(insn, 0xae);
            append_memory_as_byte(insn, 0x0c);
            append_memory_as_byte(insn, 0x24);
            SET_PTR(insn, gen);

        } else
           emitOpRegRM(FRSTOR, RealRegister(FRSTOR_OP),
                       RealRegister(REGNUM_ESP), 0, gen);
    }

    // Remove extra space allocated for temporaries and floating-point state
    int extra_space = gen.rs()->getStackHeight();
    assert(extra_space == extra_space_check);
    if (!createFrame && extra_space) {
        ::emitLEA(RealRegister(REGNUM_ESP), RealRegister(Null_Register), 0,
                extra_space, RealRegister(REGNUM_ESP), gen);
    }

    if (createFrame) {
       emitSimpleInsn(LEAVE, gen);
    }
    if (saveOrigAddr) {
        ::emitLEA(RealRegister(REGNUM_ESP), RealRegister(Null_Register), 0, 4,
                RealRegister(REGNUM_ESP), gen);
    }

    //popa or pop each register, plus optional popf
    emitBTRegRestores32(bt, gen);

    // Restore the (possibly unaligned) stack pointer.
    if (alignStack) {
        emitMovRMToReg(RealRegister(REGNUM_ESP),
                       RealRegister(REGNUM_ESP), 0, gen);
    } else {
       int funcJumpSlotSize = 0;
       if (bt && bt->funcJumpSlotSize()) {
          funcJumpSlotSize = bt->funcJumpSlotSize() * 4;
       }
       if (funcJumpSlotSize) {
          ::emitLEA(RealRegister(REGNUM_ESP),
                  RealRegister(Null_Register), 0, funcJumpSlotSize,
                  RealRegister(REGNUM_ESP), gen);
       }
    }

   gen.setInInstrumentation(false);
    return true;
}

//
// 64-bit code generation helper functions
//

static void emitRex(bool is_64, Register* r, Register* x, Register* b, codeGen &gen)
{
    unsigned char rex = 0x40;

    // need rex for 64-bit ops in most cases
    if (is_64)
       rex |= 0x08;

    // need rex for use of new registers
    // if a new register is used, we mask off the high bit before
    // returning since we account for it in the rex prefix
    
    // "R" register - extension to ModRM reg field
    if (r && *r & 0x08) {
       rex |= 0x04;
       *r &= 0x07;
    }
    
    // "X" register - extension to SIB index field
    if (x && *x & 0x08) {
       rex |= 0x02;
       *x &= 0x07;
    }

    // "B" register - extension to ModRM r/m field, SIB base field,
    // or opcode reg field
    if (b && *b & 0x08) {
       rex |= 0x01;
       *b &= 0x07;
    }
    
    // emit the rex, if needed
    // (note that some other weird cases not covered here
    //  need a "blank" rex, like using %sil or %dil)
    if (rex & 0x0f)
       emitSimpleInsn(rex, gen);
}

#if 0
/* build the MOD/RM byte of an instruction */
static unsigned char makeModRMbyte(unsigned Mod, unsigned Reg, unsigned RM)
{
   return static_cast<unsigned char>(((Mod & 0x3) << 6) + ((Reg & 0x7) << 3) + (RM & 0x7));
}
#endif

void EmitterIA32::emitStoreImm(Address addr, int imm, codeGen &gen, bool /*noCost*/) 
{
   emitMovImmToMem(addr, imm, gen);
}

void emitAddMem(Address addr, int imm, codeGen &gen) {
   //This add needs to encode "special" due to an exception
   // to the normal encoding rules and issues caused by AMD64's
   // pc-relative data addressing mode.  Our helper functions will
   // not correctly emit what we want, and we want this very specific
   // mode for the add instruction.  So I'm just writing raw bytes.

   GET_PTR(insn, gen);
   if (imm < 128 && imm > -127) {
      if (gen.rs()->getAddressWidth() == 8)
         append_memory_as_byte(insn, 0x48); // REX byte for a quad-add
      append_memory_as_byte(insn, 0x83);
      append_memory_as_byte(insn, 0x04);
      append_memory_as_byte(insn, 0x25);

      assert(addr <= numeric_limits<uint32_t>::max() && "addr more than 32-bits");
      append_memory_as(insn, static_cast<uint32_t>(addr)); //Write address

      append_memory_as(insn, static_cast<int8_t>(imm));
      SET_PTR(insn, gen);
      return;
   }

   if (imm == 1) {
      if (gen.rs()->getAddressWidth() == 4)
      {
         append_memory_as_byte(insn, 0xFF); //incl
         append_memory_as_byte(insn, 0x05);
      }
      else {
         assert(gen.rs()->getAddressWidth() == 8);
         append_memory_as_byte(insn, 0xFF); //inlc with SIB
         append_memory_as_byte(insn, 0x04);
         append_memory_as_byte(insn, 0x25);
      }
   }
   else {
      append_memory_as_byte(insn, 0x81); //addl
      append_memory_as_byte(insn, 0x4);
      append_memory_as_byte(insn, 0x25);
   }

   assert(addr <= numeric_limits<uint32_t>::max() && "addr more than 32-bits");
   append_memory_as(insn, static_cast<uint32_t>(addr)); //Write address

   if (imm != 1) {
      append_memory_as(insn, int32_t{imm}); //Write immediate value to add
   }

   SET_PTR(insn, gen);
}

void EmitterIA32::emitAddSignedImm(Address addr, int imm, codeGen &gen,
                                 bool /*noCost*/)
{
   emitAddMem(addr, imm, gen);
}


void emitMovImmToReg64(Register dest, long imm, bool is_64, codeGen &gen)
{
   Register tmp_dest = dest;
   gen.markRegDefined(dest);
   emitRex(is_64, NULL, NULL, &tmp_dest, gen);
   if (is_64) {
      GET_PTR(insn, gen);
      append_memory_as_byte(insn, 0xB8 + tmp_dest);
      append_memory_as(insn, int64_t{imm});
      SET_PTR(insn, gen);
   }
   else
      emitMovImmToReg(RealRegister(tmp_dest), imm, gen);
}

// on 64-bit x86_64 targets, the DWARF register number does not
// correspond to the machine encoding. See the AMD-64 ABI.

// We can only safely map the general purpose registers (0-7 on ia-32,
// 0-15 on amd-64)
#define IA32_MAX_MAP 7
#define AMD64_MAX_MAP 15
static int const amd64_register_map[] =
{ 
    0,  // RAX
    2,  // RDX
    1,  // RCX
    3,  // RBX
    6,  // RSI
    7,  // RDI
    5,  // RBP
    4,  // RSP
    8, 9, 10, 11, 12, 13, 14, 15    // gp 8 - 15
    
    /* This is incomplete. The x86_64 ABI specifies a mapping from
       dwarf numbers (0-66) to ("architecture number"). Without a
       corresponding mapping for the SVR4 dwarf-machine encoding for
       IA-32, however, it is not meaningful to provide this mapping. */
};

int Register_DWARFtoMachineEnc32(int n)
{
    if(n > IA32_MAX_MAP) {
		assert(0);
	}
    
    return n;
}

#if defined(arch_x86_64)

bool isImm64bit(Address imm) {
   return (imm >> 32);
}

void emitMovRegToReg64(Register dest, Register src, bool is_64, codeGen &gen)
{
    if (dest == src) return;

    Register tmp_dest = dest;
    Register tmp_src = src;
    emitRex(is_64, &tmp_dest, NULL, &tmp_src, gen);
    emitMovRegToReg(RealRegister(tmp_dest), RealRegister(tmp_src), gen);
    gen.markRegDefined(dest);
}

void emitMovPCRMToReg64(Register dest, int offset, int size, codeGen &gen, bool deref_result)
{
   GET_PTR(insn, gen);
   if (size == 8)
      append_memory_as_byte(insn, (dest & 0x8)>>1 | 0x48);    // REX prefix
   else {
      append_memory_as_byte(insn, (dest & 0x8)>>1 | 0x40);    // REX prefix
   }
   if (deref_result)
      append_memory_as_byte(insn, 0x8B);                      // MOV instruction
   else
      append_memory_as_byte(insn, 0x8D);                      // LEA instruction
   append_memory_as_byte(insn, ((dest & 0x7) << 3) | 0x5); // ModRM byte
   append_memory_as(insn, int32_t{offset - 7});               // offset
   gen.markRegDefined(dest);
   SET_PTR(insn, gen);
}

static void emitMovRMToReg64(Register dest, Register base, int disp, int size, codeGen &gen)
{
    Register tmp_dest = dest;
    Register tmp_base = base;

    gen.markRegDefined(dest);
    if (size == 1 || size == 2)
    {
       emitRex(true, &tmp_dest, NULL, &tmp_base, gen);
       GET_PTR(insn, gen);
       append_memory_as_byte(insn, 0x0f);
       if (size == 1)
          append_memory_as_byte(insn, 0xb6);
       else if (size == 2)
          append_memory_as_byte(insn, 0xb7);
       SET_PTR(insn, gen);
       emitAddressingMode(tmp_base, 0, tmp_dest, gen);
    }
    if (size == 4 || size == 8)
    {
       emitRex((size == 8), &tmp_dest, NULL, &tmp_base, gen);
       emitMovRMToReg(RealRegister(tmp_dest), RealRegister(tmp_base), disp, gen);
    }
}

static void emitMovSegRMToReg64(Register dest, Register base, int disp, codeGen &gen)
{
    Register tmp_dest = dest;
    Register tmp_base = base;

    gen.markRegDefined(dest);

    emitSegPrefix(base, gen);
    emitRex(true, &tmp_dest, NULL, &tmp_base, gen);
    emitOpSegRMReg(MOV_RM32_TO_R32, RealRegister(tmp_dest), RealRegister(tmp_base), disp, gen);
}

static void emitMovRegToRM64(Register base, int disp, Register src, int size, codeGen &gen)
{
    Register tmp_base = base;
    Register tmp_src = src;
    Register rax = REGNUM_RAX;
    if (size == 1 || size == 2) {
       //mov src, rax
       //mov a[l/x], (dest)
       gen.markRegDefined(REGNUM_RAX);
       if (tmp_src != REGNUM_RAX) {
          emitRex(true, &tmp_src, NULL, &rax, gen);
          emitMovRegToReg(RealRegister(rax), RealRegister(tmp_src), gen);
       }

       // emit prefix
       if (size == 2)
           emitSimpleInsn(0x66, gen);

       emitRex(false, NULL, NULL, &tmp_base, gen);
       GET_PTR(insn, gen);       
       if (size == 1) 
          append_memory_as_byte(insn, 0x88);
       else if (size == 2)
          append_memory_as_byte(insn, 0x89);
       SET_PTR(insn, gen);
       emitAddressingMode(tmp_base, 0, REGNUM_RAX, gen);
    }

    if (size == 4 || size == 8)
    {
      emitRex((size == 8), &tmp_src, NULL, &tmp_base, gen);
      emitMovRegToRM(RealRegister(tmp_base), disp, RealRegister(tmp_src), gen);
    }
}

static void emitOpRegReg64(unsigned opcode, Register dest, Register src, bool is_64, codeGen &gen)
{
    Register tmp_dest = dest;
    Register tmp_src = src;
    emitRex(is_64, &tmp_dest, NULL, &tmp_src, gen);
    emitOpRegReg(opcode, RealRegister(tmp_dest), RealRegister(tmp_src), gen);
    gen.markRegDefined(dest);
}

static void emitOpRegRM64(unsigned opcode, Register dest, Register base, int disp, bool is_64, codeGen &gen)
{
    Register tmp_dest = dest;
    Register tmp_base = base;
    emitRex(is_64, &tmp_dest, NULL, &tmp_base, gen);
    emitOpRegRM(opcode, RealRegister(tmp_dest), RealRegister(tmp_base), disp, gen);
    gen.markRegDefined(dest);
}

void emitOpRegImm64(unsigned opcode, unsigned opcode_ext, Register rm_reg, int imm,
			   bool is_64, codeGen &gen)
{
    Register tmp_rm_reg = rm_reg;
    emitRex(is_64, NULL, NULL, &tmp_rm_reg, gen);

    GET_PTR(insn, gen);
    append_memory_as_byte(insn, opcode);
    append_memory_as_byte(insn, 0xC0 | ((opcode_ext & 0x7) << 3) | tmp_rm_reg);
    append_memory_as(insn, int32_t{imm});
    SET_PTR(insn, gen);
    gen.markRegDefined(rm_reg);
}

// operation on memory location specified with a base register
// (does not work for RSP, RBP, R12, R13)
static void emitOpMemImm64(unsigned opcode, unsigned opcode_ext, Register base,
			  int imm, bool is_64, codeGen &gen)
{
    Register tmp_base = base;
    emitRex(is_64, NULL, NULL, &tmp_base, gen);

    GET_PTR(insn, gen);
    append_memory_as_byte(insn, opcode);
    append_memory_as_byte(insn, ((opcode_ext & 0x7) << 3) | tmp_base);
    append_memory_as(insn, int32_t{imm});
    SET_PTR(insn, gen);
}

static void emitOpRegRegImm64(unsigned opcode, Register dest, Register src1, int imm,
			      bool is_64, codeGen &gen)
{
    emitOpRegReg64(opcode, dest, src1, is_64, gen);
    GET_PTR(insn, gen);
    append_memory_as(insn, int32_t{imm});
    SET_PTR(insn, gen);
    gen.markRegDefined(dest);
}

static void emitOpRegImm8_64(unsigned opcode, unsigned opcode_ext, Register dest,
			     char imm, bool is_64, codeGen &gen)
{
    Register tmp_dest = dest;
    emitRex(is_64, NULL, NULL, &tmp_dest, gen);
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, opcode);
    append_memory_as_byte(insn, 0xC0 | ((opcode_ext & 0x7) << 3) | tmp_dest);
    append_memory_as_byte(insn, imm);
    SET_PTR(insn, gen);
    gen.markRegDefined(dest);
}

void emitPushReg64(Register src, codeGen &gen)
{
    emitRex(false, NULL, NULL, &src, gen);
    emitSimpleInsn(0x50 + src, gen);
    if (gen.rs()) gen.rs()->incStack(8);
}

void emitPopReg64(Register dest, codeGen &gen)
{
    emitRex(false, NULL, NULL, &dest, gen);    
    emitSimpleInsn(0x58 + dest, gen);
    if (gen.rs()) gen.rs()->incStack(-8);
}

void emitMovImmToRM64(Register base, int disp, int imm, bool is_64, 
                      codeGen &gen) 
{
   GET_PTR(insn, gen);
   if (base == Null_Register) {
      append_memory_as_byte(insn, 0xC7);
      append_memory_as_byte(insn, 0x84);
      append_memory_as_byte(insn, 0x25);
      append_memory_as(insn, int32_t{disp});
   }
   else {
      emitRex(is_64, &base, NULL, NULL, gen);
      append_memory_as_byte(insn, 0xC7);
      SET_PTR(insn, gen);
      emitAddressingMode(base, disp, 0, gen);
      REGET_PTR(insn, gen);
   }
   append_memory_as(insn, int32_t{imm});
   SET_PTR(insn, gen);
}

void emitAddRM64(Register dest, int imm, bool is_64, codeGen &gen)
{
   if (imm == 1) {
      emitRex(is_64, &dest, NULL, NULL, gen);
      GET_PTR(insn, gen);
      append_memory_as_byte(insn, 0xFF);
      append_memory_as_byte(insn, dest & 0x7);
      SET_PTR(insn, gen);   
      return;
   }
   emitRex(is_64, &dest, NULL, NULL, gen);
   emitOpMemImm64(0x81, 0x0, dest, imm, true, gen);
   gen.markRegDefined(dest);
   //   *((int*)insn) = imm;
   //insn += sizeof(int);
}


bool EmitterAMD64::emitMoveRegToReg(Register src, Register dest, codeGen &gen) {
    emitMovRegToReg64(dest, src, true, gen);
    gen.markRegDefined(dest);
    return true;
}

bool EmitterAMD64::emitMoveRegToReg(registerSlot *source, registerSlot *dest, codeGen &gen) {
    // TODO: make this work for getting the flag register too.

    return emitMoveRegToReg(source->encoding(), dest->encoding(), gen);
}

void EmitterAMD64::emitLEA(Register base, Register index, unsigned int scale, int disp, Register dest, codeGen& gen)
{
    Register tmp_base = base;
    Register tmp_index = index;
    Register tmp_dest = dest;
    emitRex(/*is_64*/true, &tmp_dest,
	    tmp_index == Null_Register ? NULL : &tmp_index,
	    tmp_base == Null_Register ? NULL : &tmp_base,
	    gen);
   ::emitLEA(RealRegister(tmp_base), RealRegister(tmp_index), scale, disp,
            RealRegister(tmp_dest), gen);
    gen.markRegDefined(dest);
}

codeBufIndex_t EmitterAMD64::emitIf(Register expr_reg, Register target, RegControl, codeGen &gen)
{
    // test %expr, %expr
    emitOpRegReg64(0x85, expr_reg, expr_reg, true, gen);

    // Retval: where the jump is in this sequence
    codeBufIndex_t retval = gen.getIndex();

    // Jump displacements are from the end of the insn, not start. The
    // one we're emitting has a size of 6.
    int32_t disp = target - 6;

    // je target
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, 0x0F);
    append_memory_as_byte(insn, 0x84);
    append_memory_as(insn, int32_t{disp});
    SET_PTR(insn, gen);

    return retval;
}

void EmitterAMD64::emitOp(unsigned opcode, Register dest, Register src1, Register src2, codeGen &gen)
{
    // TODO: optimize this further for ops where order doesn't matter
    if (src1 != dest)
       emitMovRegToReg64(dest, src1, true, gen);
    emitOpRegReg64(opcode, dest, src2, true, gen);
    gen.markRegDefined(dest);
}

void EmitterAMD64::emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm,
			  codeGen &gen)
{
   if (src1 != dest) {
      emitMovRegToReg64(dest, src1, true, gen);
   }
   emitOpRegImm64(opcode1, opcode2, dest, src2imm, true, gen);
   gen.markRegDefined(dest);
}

void EmitterAMD64::emitRelOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen, bool s)
{
    // cmp %src2, %src1
    emitOpRegReg64(0x39, src2, src1, true, gen);

    // mov $0, $dest ; done now in case src1 == dest or src2 == dest
    // (we can do this since mov doesn't mess w/ flags)
    emitMovImmToReg64(dest, 0, false, gen);
    gen.markRegDefined(dest);

    // jcc by two or three, depdending on size of mov
    unsigned char jcc_opcode = jccOpcodeFromRelOp(op, s);
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, jcc_opcode);
    SET_PTR(insn, gen);

    codeBufIndex_t jcc_disp = gen.used();
    gen.fill(1, codeGen::cgNOP);
    codeBufIndex_t after_jcc = gen.used();
    // mov $2,  %dest

    emitMovImmToReg64(dest, 1, false, gen);
    codeBufIndex_t after_mov = gen.used();

    gen.setIndex(jcc_disp);
    REGET_PTR(insn, gen);
    append_memory_as_byte(insn, codeGen::getDisplacement(after_jcc, after_mov));
    SET_PTR(insn, gen);

    gen.setIndex(after_mov);  // overrides previous SET_PTR
}

void EmitterAMD64::emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm,
                                codeGen &gen, bool s)
{
/* disabling hack
   // HACKITY - remove before doing anything else
   // 
   // If the input is a character, then mask off the value in the register so that we're
   // only comparing the low bytes. 
   if (src2imm < 0xff) {
      // Use a 32-bit mask instead of an 8-bit since it sign-extends...
      emitOpRegImm64(0x81, EXTENDED_0x81_AND, src1, 0xff, true, gen); 
  }
*/

   // cmp $src2imm, %src1
   emitOpRegImm64(0x81, 7, src1, src2imm, true, gen);

   // mov $0, $dest ; done now in case src1 == dest
   // (we can do this since mov doesn't mess w/ flags)
   emitMovImmToReg64(dest, 0, false, gen);
   gen.markRegDefined(dest);

   // jcc by two or three, depdending on size of mov
   unsigned char opcode = jccOpcodeFromRelOp(op, s);
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, opcode);
   SET_PTR(insn, gen);
   codeBufIndex_t jcc_disp = gen.used();
   gen.fill(1, codeGen::cgNOP);
   codeBufIndex_t after_jcc = gen.used();

   // mov $2,  %dest
   emitMovImmToReg64(dest, 1, false, gen);
   codeBufIndex_t after_mov = gen.used();

   gen.setIndex(jcc_disp);
   REGET_PTR(insn, gen);
   append_memory_as_byte(insn, codeGen::getDisplacement(after_jcc, after_mov));
   SET_PTR(insn, gen);

   gen.setIndex(after_mov);  // overrides previous SET_PTR
}

void EmitterAMD64::emitDiv(Register dest, Register src1, Register src2, codeGen &gen, bool s)
{
   // TODO: fix so that we don't always use RAX

   // push RDX if it's in use, since we will need it
   bool save_rdx = false;
   if (!gen.rs()->isFreeRegister(REGNUM_RDX) && (dest != REGNUM_RDX)) {
      save_rdx = true;
      emitPushReg64(REGNUM_RDX, gen);
   }
   else {
      gen.markRegDefined(REGNUM_RDX);
   }
   
   // If src2 is RDX we need to move it into a scratch register, as the sign extend
   // will overwrite RDX.
   // Note that this does not imply RDX is not free; both inputs are free if they
   // are not used after this call.
   Register scratchReg = src2;
   if (scratchReg == REGNUM_RDX) {
      std::vector<Register> dontUse;
      dontUse.push_back(REGNUM_RAX);
      dontUse.push_back(src2);
      dontUse.push_back(dest);
      dontUse.push_back(src1);
      scratchReg = gen.rs()->getScratchRegister(gen, dontUse);
      emitMovRegToReg64(scratchReg, src2, true, gen);
   }
   gen.markRegDefined(scratchReg);
   
   // mov %src1, %rax
   emitMovRegToReg64(REGNUM_RAX, src1, true, gen);
   gen.markRegDefined(REGNUM_RAX);
   
   // cqo (sign extend RAX into RDX)
   emitSimpleInsn(0x48, gen); // REX.W
   emitSimpleInsn(0x99, gen);
  
   if (s) {
       // idiv %src2
       emitOpRegReg64(0xF7, 0x7, scratchReg, true, gen);
   } else {
       // div %src2
       emitOpRegReg64(0xF7, 0x6, scratchReg, true, gen);
   }
   
   // mov %rax, %dest
   emitMovRegToReg64(dest, REGNUM_RAX, true, gen);
   gen.markRegDefined(dest);
   
   // pop rdx if it needed to be saved
   if (save_rdx)
      emitPopReg64(REGNUM_RDX, gen);
}

void EmitterAMD64::emitTimesImm(Register dest, Register src1, RegValue src2imm, codeGen &gen)
{
   int result = -1;

   gen.markRegDefined(dest);
   if (isPowerOf2(src2imm, result) && result <= MAX_IMM8) {
      // immediate is a power of two - use a shift
      // mov %src1, %dest (if needed)
      if (src1 != dest) {
         emitMovRegToReg64(dest, src1, true, gen);
      }
      // sal dest, result
      // Note: sal and shl are the same
      emitOpRegImm8_64(0xC1, 4, dest, result, true, gen);
   }
   else {
      // imul %dest, %src1, $src2imm
      emitOpRegRegImm64(0x69, dest, src1, src2imm, true, gen);
   } 
}

void EmitterAMD64::emitDivImm(Register dest, Register src1, RegValue src2imm, codeGen &gen, bool s)
{
   int result = -1;
   gen.markRegDefined(dest);
   if (isPowerOf2(src2imm, result) && result <= MAX_IMM8) {
      // divisor is a power of two - use a shift instruction
      // mov %src1, %dest (if needed)
      if (src1 != dest) {
         emitMovRegToReg64(dest, src1, true, gen);
      }
      if (s) {
          // sar $result, %dest
          emitOpRegImm8_64(0xC1, 7, dest, result, true, gen);
      } else {
          // shr $result, %dest
          emitOpRegImm8_64(0xC1, 5, dest, result, true, gen);
      }
   }
   else {
      
      // push RDX if it's in use, since we will need it
      bool save_rdx = false;
      if (!gen.rs()->isFreeRegister(REGNUM_RDX) && (dest != REGNUM_RDX)) {
         save_rdx = true;
         emitPushReg64(REGNUM_RDX, gen);
      }
      else {
         gen.markRegDefined(REGNUM_RDX);
      }
      // need to put dividend in RDX:RAX
      // mov %src1, %rax
      emitMovRegToReg64(REGNUM_EAX, src1, true, gen);
      gen.markRegDefined(REGNUM_RAX);
      // We either do a sign extension from RAX to RDX or clear RDX
      if (s) {
          emitSimpleInsn(0x48, gen); // REX.W
          emitSimpleInsn(0x99, gen);
      } else {
          emitMovImmToReg64(REGNUM_RDX, 0, true, gen);
      }
      // push immediate operand on the stack (no IDIV $imm)
      emitPushImm(src2imm, gen);
     
      if (s) {
          // idiv (%rsp)
          emitOpRegRM64(0xF7, 0x7 /* opcode extension */, REGNUM_RSP, 0, true, gen);
      }
      else {
          // div (%rsp)
          emitOpRegRM64(0xF7, 0x6 /* opcode extension */, REGNUM_RSP, 0, true, gen);
      }
      
      // mov %rax, %dest ; set the result
      emitMovRegToReg64(dest, REGNUM_RAX, true, gen);
      
      // pop the immediate off the stack
      // add $8, %rsp
      emitOpRegImm8_64(0x83, 0x0, REGNUM_RSP, 8, true, gen);
      gen.rs()->incStack(-8);
      
      // pop rdx if it needed to be saved
      if (save_rdx)
         emitPopReg64(REGNUM_RDX, gen);
   }
}

void EmitterAMD64::emitLoad(Register dest, Address addr, int size, codeGen &gen)
{

   Register scratch = gen.rs()->getScratchRegister(gen);
   
   // mov $addr, %rax
   emitMovImmToReg64(scratch, addr, true, gen);
	
   // mov (%rax), %dest
   emitMovRMToReg64(dest, scratch, 0, size, gen);
   gen.rs()->freeRegister(scratch);
   gen.markRegDefined(dest);
}

void EmitterAMD64::emitLoadConst(Register dest, Address imm, codeGen &gen)
{
   emitMovImmToReg64(dest, imm, true, gen);
   gen.markRegDefined(dest);
}

void EmitterAMD64::emitLoadIndir(Register dest, Register addr_src, int size, codeGen &gen)
{
   emitMovRMToReg64(dest, addr_src, 0, size, gen);
   gen.markRegDefined(dest);
}

void EmitterAMD64::emitLoadOrigFrameRelative(Register dest, Address offset, codeGen &gen)
{
   if (gen.bt()->createdFrame) {
      Register scratch = gen.rs()->getScratchRegister(gen);
      // mov (%rbp), %rax
      emitMovRMToReg64(scratch, REGNUM_RBP, 0, 8, gen);
      
      // mov offset(%rax), %dest
      emitMovRMToReg64(dest, scratch, offset, 4, gen);
      return;
   }
   emitMovRMToReg64(dest, REGNUM_RBP, offset, 4, gen);
}

bool EmitterAMD64::emitLoadRelative(Register dest, Address offset, Register base, int /* size */, codeGen &gen)
{
   // mov offset(%base), %dest
   emitMovRMToReg64(dest, base, offset,
                    gen.addrSpace()->getAddressWidth(), gen);
   gen.markRegDefined(dest);
   return true;
}

bool EmitterAMD64::emitLoadRelativeSegReg(Register dest, Address offset, Register base, int /* size */, codeGen &gen)
{
    emitMovSegRMToReg64(dest, base, offset, gen);
    gen.markRegDefined(dest);
    return true;
}

void EmitterAMD64::emitLoadFrameAddr(Register dest, Address offset, codeGen &gen)
{
   // mov (%rbp), %dest
   if (gen.bt()->createdFrame) {
      emitMovRMToReg64(dest, REGNUM_RBP, 0, 8, gen);
      
      // add $offset, %dest
      emitOpRegImm64(0x81, 0x0, dest, offset, 8, gen);
      gen.markRegDefined(dest);
      return;
   }
   emitLEA(REGNUM_RBP, Null_Register, 0, offset, dest, gen);
}

void EmitterAMD64::emitLoadOrigRegRelative(Register dest, Address offset,
                                           Register base, codeGen &gen,
                                           bool store)
{
   Register scratch = gen.rs()->getScratchRegister(gen);
   gen.markRegDefined(scratch);
   gen.markRegDefined(dest);
   // either load the address or the contents at that address
   if(store) 
   {
      // load the stored register 'base' into RAX
      emitLoadOrigRegister(base, scratch, gen);
      // move offset(%rax), %dest
      emitMovRMToReg64(dest, scratch, offset, 4, gen);
   }
   else
   {
      // load the stored register 'base' into dest
      emitLoadOrigRegister(base, dest, gen);
      // add $offset, %dest
      emitOpRegImm64(0x81, 0x0, dest, offset, true, gen);
   }
} 

// this is the distance on the basetramp stack frame from the
// start of the GPR save region to where the base pointer is,
// in 8-byte quadwords
#define GPR_SAVE_REGION_OFFSET 18

void EmitterAMD64::emitLoadOrigRegister(Address register_num, Register destination, codeGen &gen)
{
   registerSlot *src = (*gen.rs())[register_num];
   assert(src);
   registerSlot *dest = (*gen.rs())[destination];
   assert(dest);

   if (register_num == REGNUM_ESP) {
      stackItemLocation loc = getHeightOf(stackItem::stacktop, gen);
      if (!gen.bt() || gen.bt()->alignedStack)
         emitMovRMToReg64(destination, loc.reg.reg(), loc.offset, 8, gen);
      else
         emitLEA(loc.reg.reg(), Null_Register, 0, loc.offset,
                   destination, gen);
      return;
   }

   if (src->spilledState == registerSlot::unspilled)
   {
      assert(register_num != REGNUM_EFLAGS);
      emitMoveRegToReg((Register) register_num, destination, gen);
      return;
   }

   stackItemLocation loc = getHeightOf(stackItem(RealRegister(register_num)), gen);
   registerSlot *stack = (*gen.rs())[loc.reg.reg()];
   emitLoadRelative(dest->encoding(), loc.offset, stack->encoding(), gen.addrSpace()->getAddressWidth(), gen);
   gen.markRegDefined(destination);
   return;
}

void EmitterAMD64::emitStoreOrigRegister(Address register_num, Register src, codeGen &gen) {
   assert(gen.addrSpace());
   unsigned size = (gen.addrSpace()->getAddressWidth());
   gen.rs()->writeProgramRegister(gen, register_num, src, size);
}

void EmitterAMD64::emitStore(Address addr, Register src, int size, codeGen &gen)
{
   Register scratch = gen.rs()->getScratchRegister(gen);
   gen.markRegDefined(scratch);

   // mov $addr, %rax
   emitMovImmToReg64(scratch, addr, true, gen);

   // mov %src, (%rax)
   emitMovRegToRM64(scratch, 0, src, size, gen);
}

void EmitterAMD64::emitStoreIndir(Register addr_reg, Register src, int size, codeGen &gen)
{
   emitMovRegToRM64(addr_reg, 0, src, size, gen);
}

void EmitterAMD64::emitStoreFrameRelative(Address offset, Register src, Register /*scratch*/, int size, codeGen &gen)
{
   if (gen.bt()->createdFrame) {
      Register scratch = gen.rs()->getScratchRegister(gen);
      gen.markRegDefined(scratch);
      // mov (%rbp), %rax
      emitMovRMToReg64(scratch, REGNUM_RBP, 0, 8, gen);
      // mov %src, offset(%rax)
      emitMovRegToRM64(scratch, offset, src, size, gen);
      gen.rs()->freeRegister(scratch);
      return;
   }
   emitMovRegToRM64(REGNUM_RBP, offset, src, size, gen);
}

void EmitterAMD64::emitStoreRelative(Register src, Address offset, Register base, int /* size */, codeGen &gen) {
   emitMovRegToRM64(base, 
                    offset,
                    src, 
                    gen.addrSpace()->getAddressWidth(),
                    gen);
}

void EmitterAMD64::setFPSaveOrNot(const int * liveFPReg,bool saveOrNot)
{
   if (liveFPReg != NULL)
   {
      if (liveFPReg[0] == 0 && saveOrNot)
      {
         int * temp = const_cast<int *>(liveFPReg);
         temp[0] = 1;
      }
   }
}



/* Recursive function that goes to where our instrumentation is calling
   to figure out what registers are clobbered there, and in any function
   that it calls, to a certain depth ... at which point we clobber everything

   Update-12/06, njr, since we're going to a cached system we are just going to 
   look at the first level and not do recursive, since we would have to also
   store and reexamine every call out instead of doing it on the fly like before*/
bool EmitterAMD64::clobberAllFuncCall( registerSpace *rs,
                                       func_instance *callee)
		   
{
   if (callee == NULL) return false;
   
   /* This will calculate the values if the first time around, otherwise
      will check preparsed, stored values.
      True - FP Writes are present
      False - No FP Writes
   */

   stats_codegen.startTimer(CODEGEN_LIVENESS_TIMER);  
   if (callee->ifunc()->writesFPRs()) {
      for (unsigned i = 0; i < rs->FPRs().size(); i++) {
         // We might want this to be another flag, actually
         rs->FPRs()[i]->beenUsed = true;
      }
   }

   // Since we are making a call, mark all caller-saved registers
   // as used (therefore we will save them if they are live)
   for (int i = 0; i < rs->numGPRs(); i++) {
      rs->GPRs()[i]->beenUsed = true;
   }
   
   stats_codegen.stopTimer(CODEGEN_LIVENESS_TIMER);
   return true;
}



static Register amd64_arg_regs[] = {REGNUM_RDI, REGNUM_RSI, REGNUM_RDX, REGNUM_RCX, REGNUM_R8, REGNUM_R9};
#define AMD64_ARG_REGS (sizeof(amd64_arg_regs) / sizeof(Register))
Register EmitterAMD64::emitCall(opCode op, codeGen &gen, const std::vector<AstNodePtr> &operands,
                                bool noCost, func_instance *callee)
{
   assert(op == callOp);
   std::vector <Register> srcs;

   bool inInstrumentation = true;

   //  Sanity check for NULL address arg
   if (!callee) {
      char msg[256];
      sprintf(msg, "%s[%d]:  internal error:  emitFuncCall called w/out"
              "callee argument", __FILE__, __LINE__);
      showErrorCallback(80, msg);
      assert(0);
   }

   // Before we generate argument code, save any register that's live across
   // the call. 
   std::vector<pair<unsigned,int> > savedRegsToRestore;
   if (inInstrumentation) {
      bitArray regsClobberedByCall = ABI::getABI(8)->getCallWrittenRegisters();
      for (int i = 0; i < gen.rs()->numGPRs(); i++) {
         registerSlot *reg = gen.rs()->GPRs()[i];
         Register r = reg->encoding();
         static LivenessAnalyzer live(8);
         bool callerSave = 
            regsClobberedByCall.test(live.getIndex(regToMachReg64.equal_range(r).first->second));
         if (!callerSave) {
            // We don't care!
            regalloc_printf("%s[%d]: pre-call, skipping callee-saved register %u\n", FILE__, __LINE__,
                     reg->number);
            continue;
         }

         regalloc_printf("%s[%d]: pre-call, register %u has refcount %d, keptValue %d, liveState %s\n",
                         FILE__, __LINE__, reg->number,
                         reg->refCount,
                         reg->keptValue,
                         (reg->liveState == registerSlot::live) ? "live" : ((reg->liveState == registerSlot::spilled) ? "spilled" : "dead"));

         if (reg->refCount > 0 ||  // Currently active
             reg->keptValue || // Has a kept value
             (reg->liveState == registerSlot::live)) { // needs to be saved pre-call
            regalloc_printf("%s[%d]: \tsaving reg\n", FILE__, __LINE__);
            pair<unsigned, unsigned> regToSave;
            regToSave.first = reg->number;
            
            regToSave.second = reg->refCount;
            // We can have both a keptValue and a refCount - so I invert
            // the refCount if there's a keptValue
            if (reg->keptValue)
               regToSave.second *= -1;
            
            savedRegsToRestore.push_back(regToSave);
            
            // The register is live; save it. 
            emitPushReg64(reg->encoding(), gen);
            // And now that it's saved, nuke it
            reg->refCount = 0;
            reg->keptValue = false;
         }
         else {
	    // mapping from Register to MachRegister, then to index in liveness bitArray
	    if (regsClobberedByCall.test(live.getIndex(regToMachReg64.equal_range(r).first->second))){	  
               gen.markRegDefined(r);
            }
         }
      }
   }

   // Make sure we'll be adding exactly enough to the stack to maintain
   // alignment required by the AMD64 ABI.
   //
   // We must make sure this matches the number of push operations
   // in the operands.size() loop below.
   int stack_operands = operands.size() - AMD64_ARG_REGS;
   if (stack_operands < 0)
      stack_operands = 0;

   int alignment = (savedRegsToRestore.size() + stack_operands) * 8;
   if (alignment % AMD64_STACK_ALIGNMENT)
      alignment = AMD64_STACK_ALIGNMENT - (alignment % AMD64_STACK_ALIGNMENT);

   if (alignment) {
      emitLEA(REGNUM_RSP, Null_Register, 0, -alignment,
                REGNUM_RSP, gen);
      gen.rs()->incStack(alignment);
   }

   // generate code for arguments
   // Now, it would be _really_ nice to emit into 
   // the correct register so we don't need to move it. 
   // So try and allocate the correct one. 
   // We should be able to - we saved them all up above.
   int frame_size = 0;
   for (int u = operands.size() - 1; u >= 0; u--) {
      Address unused = ADDR_NULL;
      unsigned reg = Null_Register;
      if(u >= (int)AMD64_ARG_REGS)
      {
         if (!operands[u]->generateCode_phase2(gen,
                                               noCost,
                                               unused,
                                               reg)) assert(0);
         assert(reg != Null_Register);
         emitPushReg64(reg, gen);
         gen.rs()->freeRegister(reg);
         frame_size++;
      }
      else
      {
          if (gen.rs()->allocateSpecificRegister(gen, (unsigned) amd64_arg_regs[u], true))
            reg = amd64_arg_regs[u];
         else {
            cerr << "Error: tried to allocate register " << amd64_arg_regs[u] << " and failed!" << endl;
            assert(0);
         }
         gen.markRegDefined(reg);
         if (!operands[u]->generateCode_phase2(gen,
                                               noCost,
                                               unused,
                                               reg)) assert(0);
	 if (reg != amd64_arg_regs[u]) {
	   // Code generator said "we've already got this one in a different
	   // register, so just reuse it"
	   emitMovRegToReg64(amd64_arg_regs[u], reg, true, gen);
	 }	
      }
   }

   // RAX = number of FP regs used by varargs on AMD64 (also specified as caller-saved).
   //Clobber it to 0.
   emitMovImmToReg64(REGNUM_RAX, 0, true, gen);
   gen.markRegDefined(REGNUM_RAX);

   emitCallInstruction(gen, callee, Null_Register);

   // Now clear whichever registers were "allocated" for a return value
   // Don't do that for stack-pushed operands; they've already been freed.
   for (unsigned i = 0; i < operands.size(); i++) {
      if (i == AMD64_ARG_REGS) break;

      if (operands[i]->decRefCount())
         gen.rs()->freeRegister(amd64_arg_regs[i]);
   }
   if(frame_size)
   {
      emitAdjustStackPointer(frame_size, gen);
      //emitOpRegImm64(0x81, EXTENDED_0x81_ADD, REGNUM_RSP, frame_size * 8, gen); // add esp, frame_size
   }

   if (alignment) {
      // Skip past the stack alignment.
      emitLEA(REGNUM_RSP, Null_Register, 0, alignment,
                REGNUM_RSP, gen);
      gen.rs()->incStack(-alignment);
   }

   if (!inInstrumentation) return Null_Register;

   // We now have a bit of an ordering problem.
   // The RS thinks all registers are free; this is not the case
   // We've saved the incoming registers, and it's likely that
   // the return value is co-occupying one. 
   // We need to restore the registers, but _first_ we need to 
   // restore the RS state and allocate a keeper register.
   // Now restore any registers live over the call

   for (int i = savedRegsToRestore.size() - 1; i >= 0; i--) {
      registerSlot *reg = (*gen.rs())[savedRegsToRestore[i].first];
        
      if (savedRegsToRestore[i].second < 1) {
         reg->refCount = -1*(savedRegsToRestore[i].second);
         reg->keptValue = true;
      }
      else
         reg->refCount = savedRegsToRestore[i].second;
   }

   // allocate a (virtual) register to store the return value
   // We do this now because the state is correct again in the RS.

   Register ret = gen.rs()->allocateRegister(gen, noCost);
   gen.markRegDefined(ret);
   emitMovRegToReg64(ret, REGNUM_EAX, true, gen);

    
   // Now restore any registers live over the call
   for (int i = savedRegsToRestore.size() - 1; i >= 0; i--) {
      registerSlot *reg = (*gen.rs())[savedRegsToRestore[i].first];

      emitPopReg64(reg->encoding(), gen);
   }

   return ret;
}

bool EmitterAMD64Dyn::emitCallInstruction(codeGen &gen, func_instance *callee, Register) {
   // make the call (using an indirect call)
   //emitMovImmToReg64(REGNUM_EAX, callee->addr(), true, gen);
   //emitSimpleInsn(0xff, gen); // group 5
   //emitSimpleInsn(0xd0, gen); // mod = 11, reg = 2 (call Ev), r/m = 0 (RAX)

   
   if (gen.startAddr() != (Address) -1) {
      signed long disp = callee->addr() - (gen.currAddr() + 5);
      int disp_i = (int) disp;
      if (disp == (signed long) disp_i) {
         emitCallRel32(disp_i, gen);
         return true;
      }
   }
   
   std::vector<Register> excluded;
   excluded.push_back(REGNUM_RAX);
   
   Register ptr = gen.rs()->getScratchRegister(gen, excluded);
   gen.markRegDefined(ptr);
   Register effective = ptr;
   emitMovImmToReg64(ptr, callee->addr(), true, gen);
   if(ptr >= REGNUM_R8) {
      emitRex(false, NULL, NULL, &effective, gen);
   }
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0xFF);
   append_memory_as_byte(insn, static_cast<uint8_t>(0xD0 | effective));
   SET_PTR(insn, gen);

   return true;
}


bool EmitterAMD64Stat::emitCallInstruction(codeGen &gen, func_instance *callee, Register) {
   //fprintf(stdout, "at emitCallInstruction: callee=%s\n", callee->prettyName().c_str());

   AddressSpace *addrSpace = gen.addrSpace();
   Address dest;

   // find func_instance reference in address space
   // (refresh func_map)
   std::vector<func_instance *> funcs;
   addrSpace->findFuncsByAll(callee->prettyName(), funcs);

   // test to see if callee is in a shared module
   assert(gen.func());
   if (gen.func()->obj() != callee->obj()) {
      emitPLTCall(callee, gen);
   } else {
      dest = callee->addr();
      signed long disp = dest - (gen.currAddr() + 5);
      int disp_i = (int) disp;
      assert(disp == (signed long) disp_i);
      emitCallRel32(disp_i, gen);
      return true;
   }
   return true;
}

bool EmitterAMD64Stat::emitPLTJump(func_instance *callee, codeGen &gen) {
   // create or retrieve jump slot
   Address dest = getInterModuleFuncAddr(callee, gen);
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0xFF);
   // Note: this is a combination of 00 (MOD), 100 (opcode extension), and 101
   // (disp32)
   append_memory_as_byte(insn, 0x25);
   int64_t offset = dest - (gen.currAddr() + sizeof(int32_t) + 2);
   assert(numeric_limits<int32_t>::lowest() <= offset && offset <= numeric_limits<int32_t>::max() && "offset more than 32 bits");
   append_memory_as(insn, static_cast<int32_t>(offset));
   SET_PTR(insn, gen);
   return true;
}

bool EmitterAMD64Stat::emitPLTCall(func_instance *callee, codeGen &gen) {
   // create or retrieve jump slot
   Address dest = getInterModuleFuncAddr(callee, gen);
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0xFF);
   append_memory_as_byte(insn, 0x15);
   int64_t offset = dest - (gen.currAddr() + sizeof(int32_t) + 2);
   assert(numeric_limits<int32_t>::lowest() <= offset && offset <= numeric_limits<int32_t>::max() && "offset more than 32 bits");
   append_memory_as(insn, static_cast<int32_t>(offset));
   SET_PTR(insn, gen);
   return true;
}


// FIXME: comment here on the stack layout
void EmitterAMD64::emitGetRetVal(Register dest, bool addr_of, codeGen &gen)
{
   if (!addr_of) {
      emitLoadOrigRegister(REGNUM_RAX, dest, gen);
      gen.markRegDefined(dest);
      return;
   }

   //RAX isn't defined here.  See comment in EmitterIA32::emitGetRetVal
   gen.markRegDefined(REGNUM_RAX);
   stackItemLocation loc = getHeightOf(stackItem::framebase, gen);
   registerSlot *rax = (*gen.rs())[REGNUM_RAX];
   assert(rax);
   loc.offset += (rax->saveOffset * 8);
   emitLEA(loc.reg.reg(), Null_Register, 0, loc.offset, dest, gen);
}


void EmitterAMD64::emitGetRetAddr(Register dest, codeGen &gen)
{
   stackItemLocation loc = getHeightOf(stackItem::stacktop, gen);
   emitLEA(loc.reg.reg(), Null_Register, 0, loc.offset, dest, gen);
}


void EmitterAMD64::emitGetParam(Register dest, Register param_num, instPoint::Type pt_type, opCode op, bool addr_of, codeGen &gen)
{
   if (!addr_of && param_num < 6) {
      emitLoadOrigRegister(amd64_arg_regs[param_num], dest, gen);
      gen.markRegDefined(dest);
      return;
   }
   else if (addr_of && param_num < 6) {
      Register reg = amd64_arg_regs[param_num];
      gen.markRegDefined(reg);
      stackItemLocation loc = getHeightOf(stackItem::framebase, gen);
      registerSlot *regSlot = (*gen.rs())[reg];
      assert(regSlot);
      loc.offset += (regSlot->saveOffset * 8);
      emitLEA(loc.reg.reg(), Null_Register, 0, loc.offset, dest, gen);
      return;
   }
   assert(param_num >= 6);
   stackItemLocation loc = getHeightOf(stackItem::stacktop, gen);
   if (!gen.bt() || gen.bt()->alignedStack) {
      // Load the original %rsp value into dest
      emitMovRMToReg64(dest, loc.reg.reg(), loc.offset, 8, gen);
      loc.reg = RealRegister(dest);
      loc.offset = 0;
   }

   switch (op) {
      case getParamOp:
         if (pt_type == instPoint::FuncEntry) {
            //Return value before any parameters
            loc.offset += 8;
         }
         break;
      case getParamAtCallOp:
         break;
      case getParamAtEntryOp:
         loc.offset += 8;
         break;
      default:
         assert(0);
         break;
   }

   loc.offset += (param_num-6)*8;
   if (!addr_of)
      emitMovRMToReg64(dest, loc.reg.reg(), loc.offset, 8, gen);
   else 
      emitLEA(loc.reg.reg(), Null_Register, 0, loc.offset, dest, gen);
}

// Commented out until we need it to avoid warnings
#if 0
static void emitPushImm16_64(unsigned short imm, codeGen &gen)
{
   GET_PTR(insn, gen);

   // operand-size prefix
   append_memory_as_byte(insn, 0x66);

   // PUSH imm opcode
   append_memory_as_byte(insn, 0x68);

   // and the immediate
   *(unsigned short*)insn = imm;
   insn += 2;

   SET_PTR(insn, gen);
}
#endif

void EmitterAMD64::emitASload(int ra, int rb, int sc, long imm, Register dest, int stackShift, codeGen &gen)
{
   // Support for using ESP that has been moved is unimplemented.

   assert(stackShift == 0);
   Register use_a = Null_Register;
   Register use_b = Null_Register;

   bool havera = ra > -1, haverb = rb > -1;


   // if ra is specified, move its inst-point value into our
   // destination register
   gen.markRegDefined(dest);
   if(havera) {
      if (ra == mRIP) {
         // special case: rip-relative data addressing
         // the correct address has been stuffed in imm
         emitMovImmToReg64(dest, imm, true, gen);
         return;
      }
      if (gen.inInstrumentation()) {
         use_a = dest;
         emitLoadOrigRegister(ra, dest, gen);
      }
      else {
         use_a = ra;
      }
   }
  
   // if rb is specified, move its inst-point value into RAX
   if(haverb) {
      if (gen.inInstrumentation()) {
         use_b = gen.rs()->getScratchRegister(gen);
         gen.markRegDefined(use_b);
         emitLoadOrigRegister(rb, use_b, gen);
      }
      else {
         use_b = rb;
      }
   }
   // emitLEA will not handle the [disp32] case properly, so
   // we special case that
   if (!havera && !haverb)
      emitMovImmToReg64(dest, imm, false, gen);
   else {
      emitLEA(use_a, use_b,
                sc, (int)imm, 
                dest, gen);
   }
}

void EmitterAMD64::emitCSload(int ra, int rb, int sc, long imm, Register dest, codeGen &gen)
{
   // count is at most 1 register or constant or hack (aka pseudoregister)
   assert((ra == -1) &&
          ((rb == -1) ||
           ((imm == 0) && (rb == 1 /*REGNUM_ECX */ || rb >= IA32_EMULATE))));
   
   gen.markRegDefined(dest);
   if(rb >= IA32_EMULATE) {      
      // need to emulate repeated SCAS or CMPS to figure out byte count
      
      // TODO: firewall code to ensure that direction is up
      
      bool neg = false;
      unsigned char opcode_small, opcode_large;
      bool restore_rax = false;
      bool restore_rsi = false;
      
      bool rax_wasUsed = false;
      bool rsi_wasUsed = false;
      bool rdi_wasUsed = false;
      bool rcx_wasUsed = false;
      
      switch(rb) {
         case IA32_NESCAS:
            neg = true;
	    DYNINST_FALLTHROUGH;
         case IA32_ESCAS:
            opcode_small = 0xAE;
            opcode_large = 0xAF;
            restore_rax = true;
            break;
         case IA32_NECMPS:
            neg = true;
	    DYNINST_FALLTHROUGH;
         case IA32_ECMPS:
            opcode_small = 0xA6;
            opcode_large = 0xA7;
            restore_rsi = true;
            break;
         default:
            assert(!"Wrong emulation!");
      }
      
      // restore flags (needed for direction flag)
      gen.codeEmitter()->emitRestoreFlagsFromStackSlot(gen);

      // restore needed registers to values at the inst point
      // (push current values on the stack in case they're in use)
      if (restore_rax) {
         // We often use RAX as a destination register - in this case,
         // it's allocated but by us. And we really don't want to save 
         // it and then restore...
         if (!gen.rs()->isFreeRegister(REGNUM_RAX) && (dest != REGNUM_RAX)) {
            rax_wasUsed = true;
            emitPushReg64(REGNUM_RAX, gen);
         }
         emitLoadOrigRegister(REGNUM_RAX, REGNUM_RAX, gen);
      }
      if (restore_rsi) {
         if (!gen.rs()->isFreeRegister(REGNUM_RSI) && (dest != REGNUM_RSI)) {
            rsi_wasUsed = true;
            emitPushReg64(REGNUM_RSI, gen);
         }
         emitLoadOrigRegister(REGNUM_RSI, REGNUM_RSI, gen);
      }
      if (!gen.rs()->isFreeRegister(REGNUM_RDI) && (dest != REGNUM_RDI)) {
         rdi_wasUsed = true;
         emitPushReg64(REGNUM_RDI, gen);
      }
      emitLoadOrigRegister(REGNUM_RDI, REGNUM_RDI, gen);
      if (!gen.rs()->isFreeRegister(REGNUM_RCX) && (dest != REGNUM_RCX)) {
         rcx_wasUsed = true;
         emitPushReg64(REGNUM_RCX, gen);
      }
      emitLoadOrigRegister(REGNUM_RCX, REGNUM_RCX, gen);

      // emulate the string instruction
      emitSimpleInsn(neg ? 0xF2 : 0xF3, gen); // rep(n)e
      if (sc == 0)
         emitSimpleInsn(opcode_small, gen);
      else {
         if (sc == 1)
            emitSimpleInsn(0x66, gen); // operand size prefix
         else if (sc == 3)
            emitSimpleInsn(0x48, gen); // REX.W
         emitSimpleInsn(opcode_large, gen);
      }

      // RCX has now been decremented by the number of repititions
      // load old RCX into RAX and compute difference
      emitLoadOrigRegister(REGNUM_RCX, dest, gen);
      emitOp(0x2B, dest, dest, REGNUM_RCX, gen);

      // restore registers we stomped on
      if (rcx_wasUsed)
         emitPopReg64(REGNUM_RCX, gen);
      if (rdi_wasUsed)
         emitPopReg64(REGNUM_RDI, gen);
      if (rsi_wasUsed)
         emitPopReg64(REGNUM_RSI, gen);       
      if (rax_wasUsed)
         emitPopReg64(REGNUM_RAX, gen);
   }
   else if(rb > -1) {

      // count spec is simple register with scale
      // TODO: 16-bit pseudoregisters
      assert(rb < 16);

      // store the register into RAX
      Register scratch = gen.rs()->getScratchRegister(gen);
      gen.markRegDefined(scratch);
      emitLoadOrigRegister(rb, scratch, gen);

      // shift left by the given scale
      // emitTimesImm will do the right thing
      if(sc > 0)
         emitTimesImm(dest, scratch, 1 << sc, gen);
   }
   else
      emitMovImmToReg64(dest, (int)imm, true, gen);       
}

// this is the distance in 8-byte quadwords from the frame pointer
// in our basetramp's stack frame to the saved value of RFLAGS
// (1 qword for our false return address, 16 for the saved registers, 1 more for the flags)
#define SAVED_RFLAGS_OFFSET 18

void EmitterAMD64::emitRestoreFlags(codeGen &gen, unsigned offset)
{
   if (offset)
      emitOpRMReg(PUSH_RM_OPC1, RealRegister(REGNUM_EBP), offset*8, RealRegister(PUSH_RM_OPC2), gen);
   emitSimpleInsn(0x9D, gen);
}

void EmitterAMD64::emitPushFlags(codeGen &gen) {
   // save flags (PUSHFQ)
   emitSimpleInsn(0x9C, gen);
}

void EmitterAMD64::emitRestoreFlagsFromStackSlot(codeGen &gen)
{
   stackItemLocation loc = getHeightOf(stackItem(RealRegister(REGNUM_OF)), gen);
   emitOpRMReg(PUSH_RM_OPC1, RealRegister(loc.reg.reg()), loc.offset, RealRegister(PUSH_RM_OPC2), gen);
   emitSimpleInsn(0x9D, gen);
}

bool shouldSaveReg(registerSlot *reg, baseTramp *inst, bool saveFlags)
{ 
  if (reg->encoding() == REGNUM_RSP) {
    return false;
  }
  
   if (inst->point()) {
      regalloc_printf("\t shouldSaveReg for BT %p, from 0x%lx\n", (void*)inst, inst->point()->insnAddr() );
   }
   else {
      regalloc_printf("\t shouldSaveReg for iRPC\n");
   }
   if (reg->liveState != registerSlot::live) {
      regalloc_printf("\t Reg %u not live, concluding don't save\n", reg->number);
      return false;
   }
   if (saveFlags) {
      // Saving flags takes up EAX/RAX, and so if they're live they must
      // be saved even if we don't explicitly use them
      DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_LOGICAL_OP
      if (reg->number == REGNUM_EAX ||
          reg->number == REGNUM_RAX) return true;
      DYNINST_DIAGNOSTIC_END_SUPPRESS_LOGICAL_OP
   }
   if (inst && inst->validOptimizationInfo() && !inst->definedRegs[reg->encoding()]) {
      regalloc_printf("\t Base tramp instance doesn't have reg %u (num %u) defined; concluding don't save\n",
                      reg->encoding(), reg->number);
      return false;
   }
   return true;
}

// Moves stack pointer by offset and aligns it to AMD64_STACK_ALIGNMENT
// with the following sequence:
//
//     lea    -off(%rsp) => %rsp           # move %rsp down
//     mov    %rax => saveSlot1(%rsp)      # save %rax onto stack
//     lahf                                # save %rflags byte into %ah
//     seto   %al                          # save overflow flag into %al
//     mov    %rax => saveSlot2(%rsp)      # save flags %rax onto stack
//     lea    off(%rsp) => %rax            # store original %rsp in %rax
//     and    -$AMD64_STACK_ALIGNMENT,%rsp # align %rsp
//     mov    %rax => (%rsp)               # store original %rsp on stack
//     mov    -off+saveSlot2(%rax) => %rax # restore flags %rax from stack
//     add    $0x7f,%al                    # restore overflow flag from %al
//     sahf                                # restore %rflags byte from %ah
//     mov    (%rsp) => %rax               # re-load old %rsp into %rax to ...
//     mov    -off+saveSlot1(%rax) => %rax # ... restore %rax from stack
//
// This sequence has four important properties:
//     1) It never writes to memory within offset bytes below the original
//        %rsp.  This is to make it compatible with red zone skips.
//     2) It never *directly* writes to memory below %rsp.  It always begins
//        by moving %rsp down, then writing to locations above it.  This way,
//        if the kernel decides to interrupt, it won't stomp all over our
//        values before we get a chance to use them.
//     3) It is designed to support easy de-allocation of this space by
//        ending with %rsp pointing to where we stored the original %rsp.
//     4) Care has been taken to properly restore both %eax and %eflags
//        by using "lea" instead of "add" or "sub," and saving the necessary
//        flags around the "and" instruction.
//
// Saving of the flags register can be skipped if the register is not live.

void EmitterAMD64::emitStackAlign(int offset, codeGen &gen)
{
   int off = offset + 8 + AMD64_STACK_ALIGNMENT;
   int saveSlot1 =    0 + AMD64_STACK_ALIGNMENT;
   int saveSlot2 =    8 + AMD64_STACK_ALIGNMENT;

   bool saveFlags = false;
   Register scratch = REGNUM_RAX;

   if (gen.rs()->checkVolatileRegisters(gen, registerSlot::live)) {
      saveFlags = true;   // We need to save the flags register
      off += 8;           // Allocate stack space to store the flags
   }

   emitLEA(REGNUM_RSP, Null_Register, 0, -off, REGNUM_RSP, gen);
   emitStoreRelative(scratch, saveSlot1, REGNUM_RSP, 8, gen);
   if (saveFlags) {
      emitSimpleInsn(0x9f, gen);
      emitSaveO(gen);
      emitStoreRelative(scratch, saveSlot2, REGNUM_RSP, 8, gen);
   }
   emitLEA(REGNUM_RSP, Null_Register, 0, off, scratch, gen);
 
   emitOpRegImm8_64(0x83, EXTENDED_0x83_AND, REGNUM_RSP,
                    -AMD64_STACK_ALIGNMENT, true, gen);
   emitStoreRelative(scratch, 0, REGNUM_RSP, 8, gen);
   if (saveFlags) {
      emitLoadRelative(scratch, -off+saveSlot2, scratch, 8, gen);
      emitRestoreO(gen);
      emitSimpleInsn(0x9e, gen);
      emitLoadRelative(scratch, 0, REGNUM_RSP, 8, gen);
   }
   emitLoadRelative(scratch, -off+saveSlot1, scratch, 8, gen);


}

bool EmitterAMD64::emitBTSaves(baseTramp* bt,  codeGen &gen)
{
   gen.setInInstrumentation(true);

   int instFrameSize = 0; // Tracks how much we are moving %rsp

   // Align the stack now to avoid having a padding hole in the middle of
   // our instrumentation stack.  Referring to anything on the stack above
   // this point will require an indirect reference.
   //
   // There are four cases that require a AMD64_STACK_ALIGNMENT aligned
   // stack pointer:
   //
   //    - Any time we need to save the FP registers
   //    - Any time we call a function (Required by the AMD64 ABI)
   //    - Any time we may execute SSE/SSE2 instructions
   //
   // The third case is equivalent to the second case, so search the
   // ASTs for function call generation.
   //
   bool useFPRs =  BPatch::bpatch->isForceSaveFPROn() ||
      ( BPatch::bpatch->isSaveFPROn()      &&
        gen.rs()->anyLiveFPRsAtEntry()     &&
        //bt->saveFPRs()               &&
        bt->makesCall() );
   bool alignStack = useFPRs || !bt || bt->checkForFuncCalls();
   bool saveFlags = gen.rs()->checkVolatileRegisters(gen, registerSlot::live);
   bool createFrame = !bt || bt->needsFrame() || useFPRs;
   bool saveOrigAddr = createFrame && bt->instP();
   // Stores the offset to the location of the previous SP stored 
   // in the stack when a frame is created. 
   uint64_t sp_offset = 0;
   int num_saved = 0;
   int num_to_save = 0;
   //Calculate the number of registers we'll save
   for (int i = 0; i < gen.rs()->numGPRs(); i++) {
      registerSlot *reg = gen.rs()->GPRs()[i];
      if (!shouldSaveReg(reg, bt, saveFlags))
         continue;
      if (createFrame && reg->encoding() == REGNUM_RBP)
         continue;
      num_to_save++;
   }
   if (createFrame) {
      num_to_save++; //will save rbp
      num_to_save++; //Saving SP
      num_to_save++; //Saving Flag Variable
   }
   if (saveOrigAddr) {
      num_to_save++; //Stack slot for return value, no actual save though
   }
   if (saveFlags) {
      num_to_save++;
   }
         
   bool skipRedZone = (num_to_save > 0) || alignStack || saveOrigAddr || createFrame;


   if (alignStack) {
      emitStackAlign(AMD64_RED_ZONE, gen);
   } else if (skipRedZone) {
      // Just move %rsp past the red zone 
      // Use LEA to avoid flag modification.
      emitLEA(REGNUM_RSP, Null_Register, 0,
                -AMD64_RED_ZONE, REGNUM_RSP, gen);
      instFrameSize += AMD64_RED_ZONE;
      // In cases where redzone offset is skipped without alignment
      // the previous frame's SP is just the redzone skip + REG_COUNT * 8
      sp_offset += AMD64_RED_ZONE;
   }

   

   // Save the live ones
   for (int i = 0; i < gen.rs()->numGPRs(); i++) {
      registerSlot *reg = gen.rs()->GPRs()[i];

      if (!shouldSaveReg(reg, bt, saveFlags))
           continue; 
      if (createFrame && reg->encoding() == REGNUM_RBP)
           continue;
      emitPushReg64(reg->encoding(),gen);
      // We move the FP down to just under here, so we're actually
      // measuring _up_ from the FP. 
      assert((18-num_saved) > 0);
      num_saved++;
      gen.rs()->markSavedRegister(reg->encoding(), num_to_save-num_saved);
   }

   // Save flags if we need to
   if (saveFlags) {
      gen.rs()->saveVolatileRegisters(gen);
      emitPushReg64(REGNUM_RAX, gen); 

      num_saved++;
      gen.rs()->markSavedRegister(REGNUM_EFLAGS, num_to_save-num_saved);
      // Need a "defined, but not by us silly"
      gen.markRegDefined(REGNUM_RAX);
   }

   if (createFrame){
      Register itchy = gen.rs()->getScratchRegister(gen);

      // add an offset each register saved so far.
      sp_offset += (8 * (num_saved));
      // If stack alignment is used, pull the original SP from the stack
      // this location is sp_offset.
      if (alignStack) {
        emitLoadRelative(itchy, sp_offset, REGNUM_RSP, 8, gen);
        emitPushReg64(itchy, gen);
      } else {
        // Otherwise, the previous SP is exactly SP+sp_offset away
        emitLEA(REGNUM_RSP, Null_Register, 0, sp_offset, itchy, gen);
        emitPushReg64(itchy, gen);
      }
      // Special Word to help stackwalker know in First Party mode that
      // it is attempting to walk out of an inst frame.
      emitMovImmToReg64(itchy, 0xBEEFDEAD, true, gen);
      emitPushReg64(itchy, gen);
      gen.rs()->freeRegister(itchy);
   }

   // push a return address for stack walking
   if (saveOrigAddr) {
      Register origTmp = gen.rs()->getScratchRegister(gen);
      emitMovImmToReg64(origTmp, bt->instP()->addr_compat(), true, gen);
      emitPushReg64(origTmp, gen);
      gen.markRegDefined(origTmp);
      num_saved++;
   }

   // Push RBP...
   if (createFrame)
   {


        // set up a fresh stack frame
      // pushl %rbp        (0x55)
      // movl  %rsp, %rbp  (0x48 0x89 0xe5)
      emitSimpleInsn(0x55, gen);
      gen.rs()->markSavedRegister(REGNUM_RBP, 0);
      num_saved++;
      num_saved++;
      num_saved++;
      // And track where it went
      (*gen.rs())[REGNUM_RBP]->liveState = registerSlot::spilled;
      (*gen.rs())[REGNUM_RBP]->spilledState = registerSlot::framePointer;
      (*gen.rs())[REGNUM_RBP]->saveOffset = 0;

      emitMovRegToReg64(REGNUM_RBP, REGNUM_RSP, true, gen);

   }

   assert(num_saved == num_to_save);

   // Prepare our stack bookkeeping data structures.
   instFrameSize += num_saved * 8;
   if (bt) {
      bt->stackHeight = instFrameSize;
   }
   gen.rs()->setInstFrameSize(instFrameSize);
   gen.rs()->setStackHeight(0);

   // Pre-calculate space for re-alignment and floating-point state.
   int extra_space = 0;
   if (useFPRs) {
      extra_space += 512;
   }

   // Make sure that we're still 32-byte aligned when we add extra_space
   // to the stack.
   if (alignStack) {
      if ((instFrameSize + extra_space) % 32)
         extra_space += 32 - ((instFrameSize + extra_space) % 32);
   }

   if (extra_space) {
      emitLEA(REGNUM_RSP, Null_Register, 0, -extra_space,
                REGNUM_RSP, gen);
      gen.rs()->incStack(extra_space);
   }
   extra_space_check = extra_space;


   bool needFXsave = false;
   if (useFPRs) {
      // need to save the floating point state (x87, MMX, SSE)
      // Since we're guarenteed to be at least 16-byte aligned
      // now, the following sequence does the job:
      //
      //   fxsave (%rsp)           ; 0x0f 0xae 0x04 0x24

      // Change to REGET if we go back to magic LEA emission
     
     for(auto curReg = gen.rs()->FPRs().begin();
	 curReg != gen.rs()->FPRs().end();
	 ++curReg)
     {
       if((*curReg)->liveState != registerSlot::dead)
       {
	 switch ((*curReg)->number) 
	 {
	 case REGNUM_XMM0:
	 case REGNUM_XMM1:
	 case REGNUM_XMM2:
	 case REGNUM_XMM3:
	 case REGNUM_XMM4:
	 case REGNUM_XMM5:
	 case REGNUM_XMM6:
	 case REGNUM_XMM7:
	   continue;
	 default:
	   needFXsave = true;
	   break;
	 }
       }
     }
     
     if(needFXsave)
     {
       GET_PTR(buffer, gen);
       *buffer++ = 0x0f;
       *buffer++ = 0xae;
       *buffer++ = 0x04;
       *buffer++ = 0x24;
       SET_PTR(buffer, gen);
     } else 
     {
       emitMovRegToReg64(REGNUM_RAX, REGNUM_RSP, true, gen);
       emitXMMRegsSaveRestore(gen, false);
     }
   }

   if (bt) {
      bt->savedFPRs = useFPRs;
      bt->wasFullFPRSave = needFXsave;
      
      bt->createdFrame = createFrame;
      bt->savedOrigAddr = saveOrigAddr;
      bt->createdLocalSpace = false;
      bt->alignedStack = alignStack;
      bt->savedFlags = saveFlags;
      bt->skippedRedZone = skipRedZone; 
   }

   return true;
}

bool EmitterAMD64::emitBTRestores(baseTramp* bt, codeGen &gen)
{
   bool useFPRs = false;
   bool createFrame = false;
   bool saveOrigAddr = false;
   bool alignStack = false;
   bool skippedRedZone = false;
   bool restoreFlags = false;


   if (bt) {
      useFPRs = bt->savedFPRs;
      createFrame = bt->createdFrame;
      saveOrigAddr = bt->savedOrigAddr;
      alignStack = bt->alignedStack;
      skippedRedZone = bt->skippedRedZone;
      restoreFlags = bt->savedFlags;
   }
   else {
      useFPRs =  BPatch::bpatch->isForceSaveFPROn() ||
         ( BPatch::bpatch->isSaveFPROn()      &&
           gen.rs()->anyLiveFPRsAtEntry());
      createFrame = true;
      saveOrigAddr = false;
      alignStack = true;
      skippedRedZone = true; // Obviated by alignStack, but hey
      restoreFlags = true;
   }

   if (useFPRs) {
      // restore saved FP state
      // fxrstor (%rsp) ; 0x0f 0xae 0x04 0x24
     if(bt && bt->wasFullFPRSave)
     {
       GET_PTR(buffer, gen);
       *buffer++ = 0x0f;
       *buffer++ = 0xae;
       *buffer++ = 0x0c;
       *buffer++ = 0x24;
       SET_PTR(buffer, gen);
     }
     else
     {
       emitMovRegToReg64(REGNUM_RAX, REGNUM_RSP, true, gen);
       emitXMMRegsSaveRestore(gen, true);
     }
     
   }

   int extra_space = gen.rs()->getStackHeight();
   assert(extra_space == extra_space_check);
   if (!createFrame && extra_space) {
      emitLEA(REGNUM_RSP, Null_Register, 0, extra_space,
                REGNUM_RSP, gen);
   }

   if (createFrame) {
      // tear down the stack frame (LEAVE)
      emitSimpleInsn(0xC9, gen);
      // Pop the Previous SP and Special Word off of the stack, discard them
      Register itchy = gen.rs()->getScratchRegister(gen);
      emitPopReg64(itchy, gen);
      emitPopReg64(itchy, gen);
      gen.rs()->freeRegister(itchy);
   }

   // pop "fake" return address
   if (saveOrigAddr)
      emitPopReg64(REGNUM_RAX, gen);

   // Restore flags
   if (restoreFlags) {
      emitPopReg64(REGNUM_RAX, gen);
      gen.rs()->restoreVolatileRegisters(gen);
   }

   // restore saved registers
   for (int i = gen.rs()->numGPRs() - 1; i >= 0; i--) {
      registerSlot *reg = gen.rs()->GPRs()[i];
      if (reg->encoding() == REGNUM_RBP && createFrame) {
	// Although we marked it saved, we already restored it
	// above. 
	continue;
      }

      if (reg->liveState == registerSlot::spilled) {
         emitPopReg64(reg->encoding(),gen);
      }
   }

   // Restore the (possibly unaligned) stack pointer.
   if (alignStack) {
      emitLoadRelative(REGNUM_RSP, 0, REGNUM_RSP, 0, gen);
   } else if (skippedRedZone) {
      emitLEA(REGNUM_ESP, Null_Register, 0,
                  AMD64_RED_ZONE, REGNUM_ESP, gen);
    }

   gen.setInInstrumentation(false);
    return true;
}

void EmitterAMD64::emitStoreImm(Address addr, int imm, codeGen &gen, bool noCost) 
{
   if (!isImm64bit(addr) && !isImm64bit(imm)) {
      emitMovImmToMem(addr, imm, gen);
   }
   else {
      Register r = gen.rs()->allocateRegister(gen, noCost);
      gen.markRegDefined(r);
      emitMovImmToReg64(r, addr, true, gen);
      emitMovImmToRM64(r, 0, imm, true, gen);
      gen.rs()->freeRegister(r);
   }
}

void EmitterAMD64::emitAddSignedImm(Address addr, int imm, codeGen &gen,bool noCost)
{
   if (!isImm64bit(addr) && !isImm64bit(imm)) {
      emitAddMem(addr, imm, gen);
   }
   else {
      Register r = gen.rs()->allocateRegister(gen, noCost);      
      gen.markRegDefined(r);
      emitMovImmToReg64(r, addr, true, gen);
      emitAddRM64(r, imm, true, gen);
      gen.rs()->freeRegister(r);
   }
}

      
int Register_DWARFtoMachineEnc64(int n)
{
    if(n <= AMD64_MAX_MAP)
        return amd64_register_map[n];
    else {
		assert(0);
		return n;

    }
}

bool EmitterAMD64::emitPush(codeGen &gen, Register reg) {
    emitPushReg64(reg, gen);
    return true;
}
   
bool EmitterAMD64::emitPop(codeGen &gen, Register reg) {
    emitPopReg64(reg, gen);
    return true;
}

bool EmitterAMD64::emitAdjustStackPointer(int index, codeGen &gen) {
	// The index will be positive for "needs popped" and negative
	// for "needs pushed". However, positive + SP works, so don't
	// invert.
	int popVal = index * gen.addrSpace()->getAddressWidth();
	emitOpRegImm64(0x81, EXTENDED_0x81_ADD, REGNUM_ESP, popVal, true, gen);
   gen.rs()->incStack(-1 * popVal);
	return true;
}

#endif /* end of AMD64-specific functions */

Address Emitter::getInterModuleFuncAddr(func_instance *func, codeGen& gen)
{
    AddressSpace *addrSpace = gen.addrSpace();
    BinaryEdit *binEdit = addrSpace->edit();
    Address relocation_address;
    unsigned int jump_slot_size = 4;
#if defined(arch_x86_64)
    jump_slot_size = 8;
#endif

    if (!binEdit || !func) {
        assert(!"Invalid function call (function info is missing)");
    }

    SymtabAPI::Symbol *referring = func->getRelocSymbol();

    // have we added this relocation already?
    relocation_address = binEdit->getDependentRelocationAddr(referring);

    if (!relocation_address) {
        // inferiorMalloc addr location and initialize to zero
        relocation_address = binEdit->inferiorMalloc(jump_slot_size);
        unsigned char* dat = (unsigned char*) malloc(jump_slot_size);
        memset(dat,0,jump_slot_size);
        binEdit->writeDataSpace((void*)relocation_address, jump_slot_size, dat);
        free(dat);

        // add write new relocation symbol/entry
        binEdit->addDependentRelocation(relocation_address, referring);
    }

    return relocation_address;
}

Address Emitter::getInterModuleVarAddr(const image_variable *var, codeGen& gen)
{
    AddressSpace *addrSpace = gen.addrSpace();
    BinaryEdit *binEdit = addrSpace->edit();
    Address relocation_address;
    unsigned int jump_slot_size = 4;
#if defined(arch_x86_64)
    jump_slot_size = 8;
#endif

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
        unsigned int dat = 0;
        binEdit->writeDataSpace((void*)relocation_address, jump_slot_size, &dat);

        // add write new relocation symbol/entry
        binEdit->addDependentRelocation(relocation_address, referring);
    }

    return relocation_address;
}

bool EmitterIA32Dyn::emitCallInstruction(codeGen &gen, func_instance *callee, Register ret) 
{
   // make the call
   // we are using an indirect call here because we don't know the
   // address of this instruction, so we can't use a relative call.
	// The only direct, absolute calls available on x86 are far calls,
	// which require the callee to be aware that they're being far-called.
	// So we grit our teeth and deal with the indirect call.
	// Physical register
   //Same regs on Win/x86 and linux/x86
   gen.rs()->makeRegisterAvail(RealRegister(REGNUM_EAX), gen); //caller saved regs
   gen.rs()->makeRegisterAvail(RealRegister(REGNUM_ECX), gen);
   gen.rs()->makeRegisterAvail(RealRegister(REGNUM_EDX), gen);

   Register placeholder1 = gen.rs()->allocateRegister(gen, true);
   Register placeholder2 = gen.rs()->allocateRegister(gen, true);
   gen.rs()->noteVirtualInReal(ret, RealRegister(REGNUM_EAX));
   gen.rs()->noteVirtualInReal(placeholder1, RealRegister(REGNUM_ECX));
   gen.rs()->noteVirtualInReal(placeholder2, RealRegister(REGNUM_EDX));

   if (gen.startAddr() == (Address) -1) {
      emitMovImmToReg(RealRegister(REGNUM_EAX), callee->addr(), gen);
      emitOpExtReg(CALL_RM_OPC1, CALL_RM_OPC2, RealRegister(REGNUM_EAX), gen);
   }
   else {
      Address dest = callee->addr();
      Address src = gen.currAddr() + 5;
      emitCallRel32(dest - src, gen);
   }

   gen.rs()->freeRegister(placeholder1);
   gen.rs()->freeRegister(placeholder2);
   return true;
}

bool EmitterIA32Stat::emitCallInstruction(codeGen &gen, func_instance *callee, Register ret) {
   AddressSpace *addrSpace = gen.addrSpace();
   Address dest;

   gen.rs()->makeRegisterAvail(RealRegister(REGNUM_EAX), gen); //caller saved regs
   gen.rs()->makeRegisterAvail(RealRegister(REGNUM_ECX), gen);
   gen.rs()->makeRegisterAvail(RealRegister(REGNUM_EDX), gen);

   //Put some dummy virtual registers in ECX and EDX so that the
   // emitMovPCRMToReg below doesn't try to allocate them.  
   // Associate the return value into EAX now for the same reason.
   // These shouldn't generate to any acutal code.  
   Register placeholder1 = gen.rs()->allocateRegister(gen, true);
   Register placeholder2 = gen.rs()->allocateRegister(gen, true);
   gen.rs()->noteVirtualInReal(ret, RealRegister(REGNUM_EAX));
   gen.rs()->noteVirtualInReal(placeholder1, RealRegister(REGNUM_ECX));
   gen.rs()->noteVirtualInReal(placeholder2, RealRegister(REGNUM_EDX));

   // find func_instance reference in address space
   // (refresh func_map)
   std::vector<func_instance *> funcs;
   addrSpace->findFuncsByAll(callee->prettyName(), funcs);
   
   // test to see if callee is in a shared module
   if (gen.func()->obj() != callee->obj()) {
      emitPLTCall(callee, gen);
   } else {
      dest = callee->addr();
      Address src = gen.currAddr() + 5;
      emitCallRel32(dest - src, gen);
   }

   gen.rs()->freeRegister(placeholder1);
   gen.rs()->freeRegister(placeholder2);
   return true;
}

bool EmitterIA32Stat::emitPLTCall(func_instance *callee, codeGen &gen) {
   // create or retrieve jump slot
   Address dest = getInterModuleFuncAddr(callee, gen);
   // load register with address from jump slot
   emitMovPCRMToReg(RealRegister(REGNUM_EAX), dest-gen.currAddr(), gen);
   // emit call *(e_x)
   emitOpRegReg(CALL_RM_OPC1, RealRegister(CALL_RM_OPC2), 
                RealRegister(REGNUM_EAX), gen);
   return true;
}

bool EmitterIA32Stat::emitPLTJump(func_instance *callee, codeGen &gen) {
  // create or retrieve jump slot
   Address dest = getInterModuleFuncAddr(callee, gen);
   // load register with address from jump slot
   emitMovPCRMToReg(RealRegister(REGNUM_EAX), dest-gen.currAddr(), gen);
   // emit jump *(e_x)
   emitOpRegReg(JUMP_RM_OPC1, RealRegister(JUMP_RM_OPC2), 
                RealRegister(REGNUM_EAX), gen);
   return true;
}

void EmitterIA32::emitLoadShared(opCode op, Register dest, const image_variable *var, bool is_local, int /*size*/, codeGen &gen, Address offset)
{
   RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);

   // create or retrieve jump slot
   Address addr;
   if(var == NULL) {
      addr = offset;
   }
   else if(!is_local) {
      addr = getInterModuleVarAddr(var, gen);
   }  
   else {
      addr = (Address)var->getOffset();
   }
  
   emitMovPCRMToReg(dest_r, addr - gen.currAddr(), gen, (!is_local && var != NULL));
   if (op == loadOp) {
      emitLoadIndir(dest, dest, 4, gen);
   }
}

void EmitterIA32::emitStoreShared(Register source, const image_variable *var, bool is_local, 
                                  int /*size*/, codeGen &gen)
{
   // create or retrieve jump slot
   //Address addr = getInterModuleVarAddr(var, gen);
   Address addr;
   if(!is_local)
      addr = getInterModuleVarAddr(var, gen);
   else
      addr = (Address)var->getOffset();
   
   // temporary virtual register for storing destination address
   Register dest = gen.rs()->allocateRegister(gen, false);
   RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
   emitMovPCRMToReg(dest_r, addr-gen.currAddr(), gen, !is_local);
   emitStoreIndir(dest, source, 4, gen);
   gen.rs()->freeRegister(dest);
}

bool EmitterIA32::emitXorRegRM(Register dest, Register base, int disp, codeGen& gen)
{
    RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
    emitOpRegRM(XOR_R32_RM32/*0x33*/, dest_r, RealRegister(base), disp, gen);
    return true;
}

bool EmitterIA32::emitXorRegReg(Register dest, Register base, codeGen& gen)
{
    RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
    emitOpRegReg(XOR_R32_RM32, dest_r, RealRegister(base), gen);
    return true;
}

bool EmitterIA32::emitXorRegImm(Register dest, int imm, codeGen& gen)
{
    RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
    emitOpRegImm(0x6, dest_r, imm, gen);
    return true;
}

bool EmitterIA32::emitXorRegSegReg(Register /*dest*/, Register base, int disp, codeGen& gen)
{
    // WARNING: dest is hard-coded to EDX currently
    emitSegPrefix(base, gen);
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, 0x33);
    append_memory_as_byte(insn, 0x15);
    append_memory_as_byte(insn, disp);
    append_memory_as_byte(insn, 0x00);
    append_memory_as_byte(insn, 0x00);
    append_memory_as_byte(insn, 0x00);
    SET_PTR(insn, gen);
    return true;
}

#if defined(arch_x86_64)
void EmitterAMD64::emitLoadShared(opCode op, Register dest, const image_variable *var, bool is_local, int size, codeGen &gen, Address offset)
{
  Address addr;
  gen.markRegDefined(dest);
  if(!var)
  {
    addr = offset;
  }
  else if(is_local)
  {
      addr = (Address)var ->getOffset();
  }
  else
  {
    // create or retrieve jump slot
    addr = getInterModuleVarAddr(var, gen);
  }
  
  if(op == loadConstOp) {
    int addr_offset = addr - gen.currAddr();
    // Brutal hack for IP-relative: displacement operand on 32-bit = IP-relative on 64-bit.
    if(is_local || !var)
      emitLEA(Null_Register, Null_Register, 0, addr_offset - 7, dest, gen);
    else
       emitMovPCRMToReg64(dest, addr - gen.currAddr(), 8, gen, true);
    
    return;
  }
  
  // load register with address from jump slot
  if(!is_local) {
     emitMovPCRMToReg64(dest, addr - gen.currAddr(), 8, gen, true);
     emitLoadIndir(dest, dest, size, gen);
  }
  else {
     emitMovPCRMToReg64(dest, addr - gen.currAddr(), size, gen, true);
  }
}

void EmitterAMD64::emitStoreShared(Register source, const image_variable *var, bool is_local, int size, codeGen &gen)
{
  Address addr;
  
  if(is_local) {
     addr = (Address)var->getOffset();
  }
  else {
     addr = getInterModuleVarAddr(var, gen);
  }
  
  // temporary virtual register for storing destination address
  Register dest = gen.rs()->allocateRegister(gen, false); 
  gen.markRegDefined(dest);
 
  // load register with address from jump slot
  emitLEA(Null_Register, Null_Register, 0, addr-gen.currAddr() - 7, dest, gen);
  //emitMovPCRMToReg64(dest, addr-gen.currAddr(), gen, true);
  if(!is_local)
     emitLoadIndir(dest, dest, 8, gen);
    
  // get the variable with an indirect load
  emitStoreIndir(dest, source, size, gen);
  
  gen.rs()->freeRegister(dest);
}

bool EmitterAMD64::emitXorRegRM(Register dest, Register base, int disp, codeGen& gen)
{
    emitOpRegRM64(XOR_R32_RM32, dest, base, disp, true, gen);
    gen.markRegDefined(dest);
    return true;
}

bool EmitterAMD64::emitXorRegReg(Register dest, Register base, codeGen& gen)
{
    emitOpRegReg64(XOR_R32_RM32, dest, base, true, gen);
    gen.markRegDefined(dest);
    return true;
}

bool EmitterAMD64::emitXorRegImm(Register dest, int imm, codeGen& gen)
{
    emitOpRegImm64(0x81, 6, dest, imm, false, gen);
    gen.markRegDefined(dest);
    return true;
}

bool EmitterAMD64::emitXorRegSegReg(Register dest, Register base, int disp, codeGen& gen)
{
    Register tmp_dest = dest;
    Register tmp_base = base;

    emitSegPrefix(base, gen);
    emitRex(true, &tmp_dest, NULL, &tmp_base, gen);
    emitOpSegRMReg(XOR_R32_RM32, RealRegister(tmp_dest), RealRegister(tmp_base), disp, gen);
    gen.markRegDefined(dest);
    return true;
}


#endif
