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
// x86-specific methods for generating control flow

#include "CFWidget.h"
#include "Widget.h"
#include "../CFG/RelocTarget.h"

#include "instructionAPI/h/Instruction.h"

#include "../dyninstAPI/src/debug.h"

#include "../CodeTracker.h"
#include "../CodeBuffer.h"
#include "dyninstAPI/src/addressSpace.h"
#include "dyninstAPI/src/emit-x86.h"
#include "dyninstAPI/src/registerSpace.h"
#include "dyninstAPI/src/BPatch_memoryAccessAdapter.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"

#if defined(cap_mem_emulation)
#include "dyninstAPI/src/MemoryEmulator/memEmulatorWidget.h"
#endif

using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

using namespace NS_x86;

bool CFWidget::generateIndirect(CodeBuffer &buffer,
                              Register reg,
                              const RelocBlock *trace,
                              Instruction::Ptr insn) {
   // Two possibilities here: either copying an indirect jump w/o
   // changes, or turning an indirect call into an indirect jump because
   // we've had the isCall_ flag overridden.

   if (reg != Null_Register) {
      // Whatever was there doesn't matter.
      // Only thing we can handle right now is a "we left the destination
      // at the top of the stack, go get 'er Tiger!"
      assert(reg == REGNUM_ESP);
      codeGen gen(1);
      //gen.fill(1, codeGen::cgTrap);
      GET_PTR(insn, gen);
      *insn++ = 0xC3; // RET
      SET_PTR(insn, gen);
      buffer.addPIC(gen, tracker(trace));
      return true;
   }
   ia32_locations loc;
   ia32_memacc memacc[3];
   ia32_condition cond;

   ia32_instruction orig_instr(memacc, &cond, &loc);
   ia32_decode(IA32_FULL_DECODER, (const unsigned char *)insn->ptr(), orig_instr);
   const unsigned char *ptr = (const unsigned char *)insn->ptr();

   std::vector<unsigned char> raw (ptr,
                                   ptr + insn->size());

   // Opcode might get modified;
   // 0xe8 -> 0xe9 (call Jz -> jmp Jz)
   // 0xff xx010xxx -> 0xff xx100xxx (call Ev -> jmp Ev)
   // 0xff xx011xxx -> 0xff xx101xxx (call Mp -> jmp Mp)

   bool fiddle_mod_rm = false;
   for (unsigned i = loc.num_prefixes; 
        i < loc.num_prefixes + (unsigned) loc.opcode_size;
        ++i) {
      switch(raw[i]) {
         case 0xE8:
            raw[i] = 0xE9;
            break;
         case 0xFF:
            fiddle_mod_rm = true;
            break;
         default:
            break;
      }
   }

   for (int i = loc.num_prefixes + (int) loc.opcode_size; 
        i < (int) insn->size(); 
        ++i) {
      if ((i == loc.modrm_position) &&
          fiddle_mod_rm) {
         raw[i] |= 0x20;
         raw[i] &= ~0x10;
      }
   } 
  
   // TODO: don't ignore reg...
   // Indirect branches don't use the PC and so are
   // easy - we just copy 'em.
   buffer.addPIC(raw, tracker(trace));

   return true;
}



bool CFWidget::generateIndirectCall(CodeBuffer &buffer,
                                    Register reg,
                                    Instruction::Ptr insn,
                                    const RelocBlock *trace,
                                    Address /*origAddr*/)
{
   // I'm pretty sure that anything that can get translated will be
   // turned into a push/jump combo already. 
   assert(reg == Null_Register);
   // Check this to see if it's RIP-relative
   NS_x86::instruction ugly_insn(insn->ptr());
   if (ugly_insn.type() & REL_D_DATA) {
      // This was an IP-relative call that we moved to a new location.
      assert(origTarget_);

      CFPatch *newPatch = new CFPatch(CFPatch::Data, insn, 
                                      new Target<Address>(origTarget_),
                                      trace->func(),
                                      addr_);
      buffer.addPatch(newPatch, tracker(trace));
   }
   else {
      buffer.addPIC(insn->ptr(), insn->size(), tracker(trace));
   }
   
   return true;
}

bool CFPatch::apply(codeGen &gen, CodeBuffer *buf) {
   // Question 1: are we doing an inter-module static control transfer?
   // If so, things get... complicated
   if (isPLT(gen)) {
      relocation_cerr << "CFPatch::apply, PLT jump" << endl;
      if (!applyPLT(gen, buf)) {
         cerr << "Failed to apply patch (PLT req'd)" << endl;
         return false;
      }
      return true;
   }

   // Otherwise this is a classic, and therefore easy.
   int targetLabel = target->label(buf);

   relocation_cerr << "\t\t CFPatch::apply, type " << type << ", origAddr " << hex << origAddr_ 
                   << ", and label " << dec << targetLabel << endl;
   if (orig_insn) {
      relocation_cerr << "\t\t\t Currently at " << hex << gen.currAddr() << " and targeting predicted " << buf->predictedAddr(targetLabel) << dec << endl;
      switch(type) {
         case CFPatch::Jump: {
            relocation_cerr << "\t\t\t Generating CFPatch::Jump from " 
                            << hex << gen.currAddr() << " to " << buf->predictedAddr(targetLabel) << dec << endl;
            if (!insnCodeGen::modifyJump(buf->predictedAddr(targetLabel), *ugly_insn, gen)) {
               cerr << "Failed to modify jump" << endl;
               return false;
            }
            return true;
         }
         case CFPatch::JCC: {
            relocation_cerr << "\t\t\t Generating CFPatch::JCC from " 
                            << hex << gen.currAddr() << " to " << buf->predictedAddr(targetLabel) << dec << endl;            
            if (!insnCodeGen::modifyJcc(buf->predictedAddr(targetLabel), *ugly_insn, gen)) {
               cerr << "Failed to modify conditional jump" << endl;
               return false;
            }
            return true;            
         }
         case CFPatch::Call: {
            if (!insnCodeGen::modifyCall(buf->predictedAddr(targetLabel), *ugly_insn, gen)) {
               cerr << "Failed to modify call" << endl;
               return false;
            }
            return true;
         }
         case CFPatch::Data: {
            if (!insnCodeGen::modifyData(buf->predictedAddr(targetLabel), *ugly_insn, gen)) {
               cerr << "Failed to modify data" << endl;
               return false;
            }
            return true;
         }
      }
   }
   else {
      switch(type) {
         case CFPatch::Jump:
            insnCodeGen::generateBranch(gen, gen.currAddr(), buf->predictedAddr(targetLabel));
            break;
         case CFPatch::Call:
            insnCodeGen::generateCall(gen, gen.currAddr(), buf->predictedAddr(targetLabel));
            break;
         default:
            assert(0);
      }
   }
   
   return true;
}

bool CFPatch::isPLT(codeGen &gen) {
   if (!gen.addrSpace()->edit()) return false;

   // We need to PLT if we're in two different 
   // AddressSpaces - the current codegen and
   // the target.

   // First check the target type.
   if (target->type() != TargetInt::BlockTarget) {
      // Either a RelocBlock (which _must_ be local)
      // or an Address (which has to be local to be
      // meaningful); neither reqs PLT
      return false;
   }

   Target<block_instance *> *t = static_cast<Target<block_instance *> *>(target);
   block_instance *tb = t->t();
   if (tb->proc() != gen.addrSpace())
      return true;
   else
      return false;
}

bool CFPatch::applyPLT(codeGen &gen, CodeBuffer *) {
   // We should try and keep any prefixes that were on the instruction. 
   // However... yeah, right. I'm not that good with x86. So instead
   // I'm copying the code from emitCallInstruction...
   
   if (target->type() != TargetInt::BlockTarget) {
      cerr << "Target type is " << target->type() << ", not block target" << endl;
      return false;
   }
   // And can only handle calls right now. That's a TODO...
   if (type != Call &&
       type != Jump) {
      cerr << "Attempt to make PLT of type " << type << " and can only handle calls or jumps" << endl;
      return false;
   }

   Target<block_instance *> *t = static_cast<Target<block_instance *> *>(target);
   block_instance *tb = t->t();

   // We can (for now) only jump to functions
   func_instance *callee = tb->entryOfFunc();
   if (!callee) {
      cerr << "No callee, ret false" << endl;
      return false;
   }

   relocation_cerr << "Emitting a PLT jump/call, targeting " << callee->name() << endl;

   // We need a registerSpace for this. For now, assume we're at
   // a call boundary (as that's _really_ the only place we can
   // be for now) and set it to the optimistic register space.
   gen.setRegisterSpace(registerSpace::optimisticRegSpace(gen.addrSpace()));

   if (type == Call) 
      gen.codeEmitter()->emitPLTCall(callee, gen);
   else if (type == Jump)
      gen.codeEmitter()->emitPLTJump(callee, gen);
   else
      assert(0);

   return true;
}


#if !defined(cap_mem_emulation)
bool CFWidget::generateAddressTranslator(CodeBuffer &,const codeGen &,Register &,const RelocBlock *) {
   return true;
}
#else
bool CFWidget::generateAddressTranslator(CodeBuffer &buffer,
                                       const codeGen &templ,
                                       Register &reg,
                                       const RelocBlock *trace) 
{
   if (!templ.addrSpace()->isMemoryEmulated() ||
       BPatch_defensiveMode != trace->block()->obj()->hybridMode())
      return true;

   if (insn_->getOperation().getID() == e_ret_near ||
       insn_->getOperation().getID() == e_ret_far) {
      // Oops!
      return true;
   }
   if (!insn_->readsMemory()) {
      return true;
   }
   
   BPatch_memoryAccessAdapter converter;
   BPatch_memoryAccess *acc = converter.convert(insn_, addr_, false);
   if (!acc) {
      reg = Null_Register;
      return true;
   }
   
   codeGen patch(128);
   patch.applyTemplate(templ);
   
   // TODO: we probably want this in a form that doesn't stomp the stack...
   // But we can probably get away with this for now. Check that.

   // step 1: create space on the stack. 
   ::emitPush(RealRegister(REGNUM_EAX), patch);
   
   // step 2: save registers that will be affected by the call
   ::emitPush(RealRegister(REGNUM_ECX), patch);
   ::emitPush(RealRegister(REGNUM_EDX), patch);
   ::emitPush(RealRegister(REGNUM_EAX), patch);
   
   // Step 3: LEA this sucker into ECX.
   const BPatch_addrSpec_NP *start = acc->getStartAddr(0);
   if (start->getReg(0) == REGNUM_ESP ||
       start->getReg(1) == REGNUM_ESP) {
      cerr << "ERROR: CF insn that uses the stack pointer! " << insn_->format() << endl;
   }

   int stackShift = -16;
   // If we are a call _instruction_ but isCall is false, then we've got an extra word
   // on the stack from an emulated return address
   if (!isCall_ && insn_->getCategory() == c_CallInsn) stackShift -= 4;

   emitASload(start, REGNUM_ECX, stackShift, patch, true);
   
   // Step 4: save flags post-LEA
   emitSimpleInsn(0x9f, patch);
   emitSaveO(patch);
   ::emitPush(RealRegister(REGNUM_EAX), patch);
   
   // This might look a lot like a memEmulatorWidget. That's, well, because it
   // is. 
   buffer.addPIC(patch, tracker(trace));
   
   // Where are we going?
   func_instance *func = templ.addrSpace()->findOnlyOneFunction("RTtranslateMemory");
   // FIXME for static rewriting; this is a dynamic-only hack for proof of concept.
   assert(func);
   
   // Now we start stealing from memEmulatorWidget. We need to call our translation function,
   // which means a non-PIC patch to the CodeBuffer. I don't feel like rewriting everything,
   // so there we go.
   buffer.addPatch(new MemEmulatorPatch(REGNUM_ECX, REGNUM_ECX, addr_, func->addr()),
                   tracker(trace));
   patch.setIndex(0);
   
   // Restore flags
   ::emitPop(RealRegister(REGNUM_EAX), patch);
   emitRestoreO(patch);
   emitSimpleInsn(0x9E, patch);
   ::emitPop(RealRegister(REGNUM_EAX), patch);
   ::emitPop(RealRegister(REGNUM_EDX), patch);
   
   // ECX now holds the pointer to the destination...
   // Dereference
   ::emitMovRMToReg(RealRegister(REGNUM_ECX),
                    RealRegister(REGNUM_ECX),
                    0,
                    patch);
   
   // ECX now holds the _actual_ destination, so move it on to the stack. 
   // We've got ECX saved
   ::emitMovRegToRM(RealRegister(REGNUM_ESP),
                    1*4, 
                    RealRegister(REGNUM_ECX),
                    patch);
   ::emitPop(RealRegister(REGNUM_ECX), patch);
   // And tell our people to use the top of the stack
   // for their work.
   // TODO: trust liveness and leave this in a register. 

   buffer.addPIC(patch, tracker(trace));
   reg = REGNUM_ESP;
   return true;
}
#endif
