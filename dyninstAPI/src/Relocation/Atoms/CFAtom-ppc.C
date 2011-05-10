// ppc-specific methods for generating control flow

#include "CFAtom.h"
#include "Atom.h"
#include "Target.h"

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

using namespace NS_power;

bool CFAtom::generateIndirect(CodeBuffer &buffer,
                              Register,
                              const Trace *trace,
                              Instruction::Ptr insn) {
  // Copying an indirect jump; unlike x86 we don't do
  // call -> indirect conversion yet. 
  // ... though that would be really freaking easy. 

  NS_power::instruction ugly_insn(insn->ptr());
  IFORM_LK_SET(ugly_insn, 0);
  codeGen gen(4);
  insnCodeGen::generate(gen, ugly_insn);

  // TODO don't ignore the register parameter
   buffer.addPIC(gen, tracker(trace));

   return true;
}



bool CFAtom::generateIndirectCall(CodeBuffer &buffer,
                                  Register reg,
                                  Instruction::Ptr insn,
                                  const Trace *trace,
				  Address origAddr) 
{
  NS_power::instruction ugly_insn(insn->ptr());
  IFORM_LK_SET(ugly_insn, 1);
  codeGen gen(4);
  insnCodeGen::generate(gen, ugly_insn);

  // TODO don't ignore the register parameter
   buffer.addPIC(gen, tracker(trace));

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

   if (isSpecialCase(gen)) {
     if (!handleSpecialCase(gen)) {
       return false;
     }
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
      // Either a Trace (which _must_ be local)
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

   // We need a RegisterSpace for this. Amusingly,
   // we want to use the RegisterSpace corresponding to the
   // entry of the callee, as it doesn't matter what's live
   // here. 
   instPoint *calleeEntry = instPoint::funcEntry(callee);
   gen.setRegisterSpace(registerSpace::actualRegSpace(calleeEntry));

   if (type == Call) 
      gen.codeEmitter()->emitPLTCall(callee, gen);
   else if (type == Jump)
      gen.codeEmitter()->emitPLTJump(callee, gen);
   else
      assert(0);

   return true;
}

bool CFPatch::isSpecialCase(codeGen &gen) {
  // 64-bit check
  if (gen.addrSpace()->getAddressWidth() != 8) return false;

  // See if this is inter-module, according to the target
  // and gen
  // Assuming an address target is not inter-module...
  if (target->type() != TargetInt::BlockTarget) return false;

  Target<block_instance *> *t = static_cast<Target<block_instance *> *>(target);
  if (t->t()->obj() == gen.func()->obj()) return false;
  
  return true;
}

bool CFPatch::handleSpecialCase(codeGen &gen) {
  // Annoying, pain in the butt case...
  assert(target->type() == TargetInt::BlockTarget);
  Target<block_instance *> *t = static_cast<Target<block_instance *> *>(target);


  if (type == Jump)
    return gen.codeEmitter()->emitTOCJump(t->t(), gen);
  else if (type == Call)
    return gen.codeEmitter()->emitTOCCall(t->t(), gen);
  else {
    assert(0);
    return false;
  }
}
    
