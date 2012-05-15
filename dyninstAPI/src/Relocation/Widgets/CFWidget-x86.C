// x86-specific methods for generating control flow

#include "CFWidget.h"
#include "Widget.h"
#include "../CFG/RelocTarget.h"

#include "instructionAPI/h/Instruction.h"

#include "../patchapi_debug.h"

#include "../CodeTracker.h"
#include "../CodeBuffer.h"
#include "dyninstAPI/src/addressSpace.h"
#include "dyninstAPI/src/emit-x86.h"
#include "dyninstAPI/src/registerSpace.h"

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
   ia32_decode(IA32_FULL_DECODER, (unsigned char *)insn->ptr(), orig_instr);
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
                                    Address) 
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

