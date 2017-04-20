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


#include "Absloc.h"
#include "AbslocInterface.h"

// Pile of InstructionAPI includes
#include "Expression.h"
#include "Register.h"
#include "Result.h"
#include "Dereference.h"

#include "dataflowAPI/h/stackanalysis.h"
#include "common/src/singleton_object_pool.h"
#include "parseAPI/h/CFG.h"
#include "parseAPI/h/CodeObject.h"

using namespace Dyninst;
using namespace Dyninst::InstructionAPI;
extern int df_debug_stackanalysis;

template class std::vector<boost::shared_ptr<Dyninst::Assignment> >;

void AbsRegionConverter::convertAll(InstructionAPI::Expression::Ptr expr,
				    Address addr,
				    ParseAPI::Function *func,
                                    ParseAPI::Block *block,
				    std::vector<AbsRegion> &regions) {
  // If we're a memory dereference, then convert us and all
  // used registers.
  if (boost::dynamic_pointer_cast<Dereference>(expr)) {
    std::vector<Expression::Ptr> tmp;
    // Strip dereference...
    expr->getChildren(tmp);
    for (std::vector<Expression::Ptr>::const_iterator i = tmp.begin();
	 i != tmp.end(); ++i) {
       regions.push_back(convert(*i, addr, func, block));
    }
  }
  
  // Otherwise just convert registers
  
  std::set<InstructionAST::Ptr> used;
  expr->getUses(used);
  for (std::set<InstructionAST::Ptr>::const_iterator j = used.begin();
       j != used.end(); ++j) {
    regions.push_back(convert(boost::dynamic_pointer_cast<RegisterAST>(*j)));
  }
}

void AbsRegionConverter::convertAll(InstructionAPI::Instruction::Ptr insn,
				    Address addr,
				    ParseAPI::Function *func,
                                    ParseAPI::Block *block,
				    std::vector<AbsRegion> &used,
				    std::vector<AbsRegion> &defined) {
                        
                        if (!usedCache(addr, func, used)) {
    std::set<RegisterAST::Ptr> regsRead;
    insn->getReadSet(regsRead);


    for (std::set<RegisterAST::Ptr>::const_iterator i = regsRead.begin();
	 i != regsRead.end(); ++i) {
        if(insn->getArch() == Arch_aarch64) {
            MachRegister machReg = (*i)->getID();
            std::vector<MachRegister> flagRegs = {aarch64::n, aarch64::z, aarch64::c, aarch64::v};

            if((machReg & 0xFF) == (aarch64::pstate & 0xFF) && (machReg & 0xFF0000) == (aarch64::SPR)) {
                for(std::vector<MachRegister>::iterator itr = flagRegs.begin(); itr != flagRegs.end(); itr++) {
                    used.push_back(AbsRegionConverter::convert(RegisterAST::Ptr(new RegisterAST(*itr))));
                }
            } else {
                used.push_back(AbsRegionConverter::convert(*i));
            }
        } else {
            used.push_back(AbsRegionConverter::convert(*i));
        }
    }
    
    if (insn->readsMemory()) {
      std::set<Expression::Ptr> memReads;
      insn->getMemoryReadOperands(memReads);
      for (std::set<Expression::Ptr>::const_iterator r = memReads.begin();
	   r != memReads.end();
	   ++r) {
         used.push_back(AbsRegionConverter::convert(*r, addr, func, block));
      }
    }
  }
  if (!definedCache(addr, func, defined)) {
    // Defined time
    std::set<RegisterAST::Ptr> regsWritten;
    insn->getWriteSet(regsWritten);
    
    for (std::set<RegisterAST::Ptr>::const_iterator i = regsWritten.begin();
	 i != regsWritten.end(); ++i) {
      if(insn->getArch() == Arch_aarch64) {
            MachRegister machReg = (*i)->getID();
            std::vector<MachRegister> flagRegs = {aarch64::n, aarch64::z, aarch64::c, aarch64::v};

            if((machReg & 0xFF) == (aarch64::pstate & 0xFF) && (machReg & 0xFF0000) == (aarch64::SPR)) {
                for(std::vector<MachRegister>::iterator itr = flagRegs.begin(); itr != flagRegs.end(); itr++) {
                    defined.push_back(AbsRegionConverter::convert(RegisterAST::Ptr(new RegisterAST(*itr))));
                }
            } else {
                defined.push_back(AbsRegionConverter::convert(*i));
            }
        } else {
            defined.push_back(AbsRegionConverter::convert(*i));
        }
    }

    // special case for repeat-prefixed instructions on x86
    // may disappear if Dyninst's representation of these instructions changes
    if (insn->getArch() == Arch_x86) {
      prefixEntryID insnPrefix = insn->getOperation().getPrefixID();
      if ( (prefix_rep == insnPrefix) || (prefix_repnz == insnPrefix) ) {
        defined.push_back(AbsRegionConverter::convert(RegisterAST::Ptr(
          new RegisterAST(MachRegister::getPC(Arch_x86)))));
      }
    }
    
    if (insn->writesMemory()) {
      std::set<Expression::Ptr> memWrites;
      insn->getMemoryWriteOperands(memWrites);
      for (std::set<Expression::Ptr>::const_iterator r = memWrites.begin();
	   r != memWrites.end();
	   ++r) {
         defined.push_back(AbsRegionConverter::convert(*r, addr, func, block));
      }
    }
  }

  if (cacheEnabled_) {
    used_cache_[func][addr] = used;
    defined_cache_[func][addr] = defined;
  }
}

AbsRegion AbsRegionConverter::convert(RegisterAST::Ptr reg) {
  // FIXME:
  // Upcast register so we can be sure to match things later
  AbsRegion tmp = AbsRegion(Absloc(reg->getID().getBaseRegister()));

  //std::cerr << "ARC::convert from " << reg->format() << " to "
  //    << tmp.format() << std::endl;
  return tmp;
}

AbsRegion AbsRegionConverter::convert(Expression::Ptr exp,
				      Address addr,
				      ParseAPI::Function *func,
                                      ParseAPI::Block *block) {
    // We want to simplify the expression as much as possible given 
    // currently known state, and then quantify it as one of the following:
    // 
    // Stack: a memory access based off the current frame pointer (FP) or
    //   stack pointer (SP). If we can determine an offset from the "top"
    //   of the stack we create a stack slot location. Otherwise we create
    //   a "stack" location that represents all stack locations.
    //
    // Heap: a memory access to a generic pointer.
    //
    // Memory: a memory access to a known address. 
    //
    // TODO: aliasing relations. Aliasing SUCKS. 

    // Since we have an Expression as input, we don't have the dereference
    // operator.

    // Here's the logic:
    // If no registers are used:
    //   If only immediates are used:
    //     Evaluate and create a MemLoc.
    //   If a dereference exists:
    //     WTF???
    // If registers are used:
    //   If the only register is the FP AND the function has a stack frame:
    //     Set FP to 0, eval, and create a specific StackLoc.
    //   If the only register is the SP:
    //     If we know the contents of SP:
    //       Eval and create a specific StackLoc
    //     Else create a generic StackLoc.
    //   If a non-stack register is used:
    //     Create a generic MemLoc.

    long spHeight = 0;
    bool stackDefined = getCurrentStackHeight(func,
                                              block,
                                              addr, 
                                              spHeight);
    long fpHeight = 0;
    bool frameDefined = getCurrentFrameHeight(func,
                                              block,
                                              addr,
                                              fpHeight);

    bool isStack = false;
    bool isFrame = false;


    static Expression::Ptr theStackPtr(new RegisterAST(MachRegister::getStackPointer(Arch_x86)));
    static Expression::Ptr theStackPtr64(new RegisterAST(MachRegister::getStackPointer(Arch_x86_64)));
    static Expression::Ptr theStackPtrPPC(new RegisterAST(MachRegister::getStackPointer(Arch_ppc32)));
    
    static Expression::Ptr theFramePtr(new RegisterAST(MachRegister::getFramePointer(Arch_x86)));
    static Expression::Ptr theFramePtr64(new RegisterAST(MachRegister::getFramePointer(Arch_x86_64)));

    static Expression::Ptr thePC(new RegisterAST(MachRegister::getPC(Arch_x86)));
    static Expression::Ptr thePC64(new RegisterAST(MachRegister::getPC(Arch_x86_64)));
    static Expression::Ptr thePCPPC(new RegisterAST(MachRegister::getPC(Arch_ppc32)));
    
    // We currently have to try and bind _every_ _single_ _alias_
    // of the stack pointer...
    if (stackDefined) {
      if (exp->bind(theStackPtr.get(), Result(s32, spHeight)) ||
	  exp->bind(theStackPtr64.get(), Result(s64, spHeight)) ||
	  exp->bind(theStackPtrPPC.get(), Result(s32, spHeight))) {
	isStack = true;
      }
    }
    if (frameDefined) {
      if (exp->bind(theFramePtr.get(), Result(s32, fpHeight)) ||
	  exp->bind(theFramePtr64.get(), Result(s64, fpHeight))) {
	isFrame = true;
      }
    }

    // Bind the IP, why not...
    exp->bind(thePC.get(), Result(u32, addr));
    exp->bind(thePC64.get(), Result(u64, addr));
    exp->bind(thePCPPC.get(), Result(u32, addr));

    Result res = exp->eval();

    if (isFrame && stackAnalysisEnabled_) {
      if (res.defined && frameDefined) {
	return AbsRegion(Absloc(res.convert<Address>(),
                                0,
				func));
      }
      else {
	return AbsRegion(Absloc::Stack);
      }
    }

    if (isStack && stackAnalysisEnabled_) {
      if (res.defined && stackDefined) {
         return AbsRegion(Absloc(res.convert<Address>(),
                                 0,
                                 func));
      }
      else if (func->obj()->defensiveMode()) {
          // SP could point to the heap, we make the worst-case 
          // assumption and will emulate this stack access
          return AbsRegion(Absloc::Heap); 
      } else {
         return AbsRegion(Absloc::Stack);
      }
    }

    // Otherwise we're on the heap
    if (res.defined) {
      return AbsRegion(Absloc(res.convert<Address>()));
    }
    else {
      return AbsRegion(Absloc::Heap);
    }
}

AbsRegion AbsRegionConverter::stack(Address addr,
				    ParseAPI::Function *func,
                                    ParseAPI::Block *block,
				    bool push) {
    if(!stackAnalysisEnabled_) {
//        std::cerr << "Stack analysis disabled, returning Stack absregion" << std::endl;
        return AbsRegion(Absloc::Stack);
    }
    long spHeight = 0;
    bool stackExists = getCurrentStackHeight(func,
                                             block,
					     addr, 
					     spHeight);
    if (!stackExists) {
      return AbsRegion(Absloc::Stack);
    }

    if (push) {
      int word_size = func->isrc()->getAddressWidth();
      spHeight -= word_size;
    }

    return AbsRegion(Absloc(spHeight,
                            0,
			    func));
}

AbsRegion AbsRegionConverter::frame(Address addr,
				    ParseAPI::Function *func,
                                    ParseAPI::Block *block,
				    bool push) {
    long fpHeight = 0;
    bool frameExists = getCurrentFrameHeight(func,
                                             block,
					     addr, 
					     fpHeight);

    if (!frameExists) {
      return AbsRegion(Absloc::Heap);
    }

    if (push) {
      int word_size = func->isrc()->getAddressWidth();
      fpHeight -= word_size;
    }
    
    return AbsRegion(Absloc(fpHeight,
                            0,
			    func));
}

bool AbsRegionConverter::getCurrentStackHeight(ParseAPI::Function *func,
                                               ParseAPI::Block *block,
					       Address addr,
					       long &height) {
  if (!stackAnalysisEnabled_) return false;
  StackAnalysis sA(func);
 
  StackAnalysis::Height heightSA = sA.findSP(block, addr);

  // Ensure that analysis has been performed.
  assert(!heightSA.isTop());
  
  if (heightSA.isBottom()) {
    return false;
  }
  
  height = heightSA.height();
  
  return true;
}

bool AbsRegionConverter::getCurrentFrameHeight(ParseAPI::Function *func,
                                               ParseAPI::Block *block,
                                               Address addr,
					       long &height) {
  if (!stackAnalysisEnabled_) return false;					       
  StackAnalysis sA(func);

  StackAnalysis::Height heightSA = sA.find(block, addr, MachRegister::getFramePointer(func->isrc()->getArch()));;

  // Ensure that analysis has been performed.
  assert(!heightSA.isTop());
  
  if (heightSA.isBottom()) {
    return false;
  }
  
  height = heightSA.height();
  
  return true;
}


bool AbsRegionConverter::usedCache(Address addr,
				   ParseAPI::Function *func,
				   std::vector<AbsRegion> &used) {
  if (!cacheEnabled_) return false;
  FuncCache::iterator iter = used_cache_.find(func);
  if (iter == used_cache_.end()) return false;
  AddrCache::iterator iter2 = iter->second.find(addr);
  if (iter2 == iter->second.end()) return false;
  used = iter2->second;
  return true;
}

bool AbsRegionConverter::definedCache(Address addr,
				      ParseAPI::Function *func,
				      std::vector<AbsRegion> &defined) {
  if (!cacheEnabled_) return false;
  FuncCache::iterator iter = defined_cache_.find(func);
  if (iter == defined_cache_.end()) return false;
  AddrCache::iterator iter2 = iter->second.find(addr);
  if (iter2 == iter->second.end()) return false;
  defined = iter2->second;
  return true;
}

///////////////////////////////////////////////////////
// Create a set of Assignments from an InstructionAPI
// Instruction.
///////////////////////////////////////////////////////

void AssignmentConverter::convert(const Instruction::Ptr I, 
                                  const Address &addr,
				  ParseAPI::Function *func,
                                  ParseAPI::Block *block,
				  std::vector<Assignment::Ptr> &assignments) {
  assignments.clear();
  if (cache(func, addr, assignments)) return;

  // Decompose the instruction into a set of abstract assignments.
  // We don't have the Definition class concept yet, so we'll do the 
  // hard work here. 
  // Two phases:
  // 1) Special-cased for IA32 multiple definition instructions,
  //    based on the opcode of the instruction
  // 2) Generic handling for things like flags and the PC. 

  // Non-PC handling section
  switch(I->getOperation().getID()) {
  case e_push: {
    // SP = SP - 4 
    // *SP = <register>
 
    std::vector<Operand> operands;
    I->getOperands(operands);

    // According to the InstructionAPI, the first operand will be the argument, the second will be ESP.
    assert(operands.size() == 2);

    // The argument can be any of the following:
    // 1) a register (push eax);
    // 2) an immediate value (push $deadbeef)
    // 3) a memory location. 

    std::vector<AbsRegion> oper0;
    aConverter.convertAll(operands[0].getValue(),
                          addr,
                          func,
                          block,
                          oper0);

    handlePushEquivalent(I, addr, func, block, oper0, assignments);
    break;
  }
  case e_call: {
    // This can be seen as a push of the PC...

    std::vector<AbsRegion> pcRegion;
    pcRegion.push_back(Absloc::makePC(func->isrc()->getArch()));
    Absloc sp = Absloc::makeSP(func->isrc()->getArch());
    
    handlePushEquivalent(I, addr, func, block, pcRegion, assignments);

    // Now for the PC definition
    // Assume full intra-dependence of non-flag and non-pc registers. 
    std::vector<AbsRegion> used;
    std::vector<AbsRegion> defined;

    aConverter.convertAll(I,
			  addr,
			  func,
                          block,
			  used,
			  defined);

    Assignment::Ptr a = Assignment::makeAssignment(I, addr, func, block, pcRegion[0]);
    if (!used.empty()) {
        for(std::vector<AbsRegion>::const_iterator u = used.begin();
            u != used.end();
            ++u)
        {
            if(!(u->contains(pcRegion[0])) &&
                 !(u->contains(sp)))
            {
                a->addInput(*u);
            }
        }
    }
    else {
      a->addInputs(pcRegion);
    }
    assignments.push_back(a);
    break;
  }
  case e_pop: {
    // <reg> = *SP
    // SP = SP + 4/8
    // Amusingly... this doesn't have an intra-instruction dependence. It should to enforce
    // the order that <reg> = *SP happens before SP = SP - 4, but since the input to both 
    // uses of SP in this case are the, well, input values... no "sideways" edges. 
    // However, we still special-case it so that SP doesn't depend on the incoming stack value...
    // Also, we use the same logic for return, defining it as
    // PC = *SP
    // SP = SP + 4/8

    // As with push, eSP shows up as operand 1. 

    std::vector<Operand> operands;
    I->getOperands(operands);

    // According to the InstructionAPI, the first operand will be the explicit register, the second will be ESP.
    assert(operands.size() == 2);

    std::vector<AbsRegion> oper0;
    aConverter.convertAll(operands[0].getValue(),
                          addr,
                          func,
                          block,
                          oper0);

    handlePopEquivalent(I, addr, func, block, oper0, assignments);
    break;
  }
  case e_leave: {
    // a leave is equivalent to:
    // mov ebp, esp
    // pop ebp
    // From a definition POV, we have the following:
    // SP = BP
    // BP = *SP
        
    // BP    STACK[newSP]
    //  |    |
    //  v    v
    // SP -> BP
        
    // This is going to give the stack analysis fits... for now, I think it just reverts the
    // stack depth to 0. 

    // TODO FIXME update stack analysis to make this really work. 
        
    AbsRegion sp(Absloc::makeSP(func->isrc()->getArch()));
    AbsRegion fp(Absloc::makeFP(func->isrc()->getArch()));

    // Should be "we assign SP using FP"
    Assignment::Ptr spA = Assignment::makeAssignment(I,
							 addr,
							 func,
                                                         block,
							 sp);
    spA->addInput(fp);

    // And now we want "FP = (stack slot -2*wordsize)"
    /*
      AbsRegion stackTop(Absloc(0,
      0,
      func));
    */
    // Actually, I think this is ebp = pop esp === ebp = pop ebp
    Assignment::Ptr fpA = Assignment::makeAssignment(I,
							 addr,
							 func,
                                                         block,
							 fp);
    //fpA->addInput(aConverter.stack(addr + I->size(), func, false));
    fpA->addInput(aConverter.frame(addr, func, block, false));

    assignments.push_back(spA);
    assignments.push_back(fpA);
    break;
  }
  case e_ret_near:
  case e_ret_far: {
    // PC = *SP
    // SP = SP + 4/8
    // Like pop, except it's all implicit.

    AbsRegion pc = AbsRegion(Absloc::makePC(func->isrc()->getArch()));
    Assignment::Ptr pcA = Assignment::makeAssignment(I, 
							 addr,
							 func,
                                                         block,
							 pc);
    pcA->addInput(aConverter.stack(addr, func, block, false));

    AbsRegion sp = AbsRegion(Absloc::makeSP(func->isrc()->getArch()));
    Assignment::Ptr spA = Assignment::makeAssignment(I,
							 addr,
							 func,
                                                         block,
							 sp);
    spA->addInput(sp);

    assignments.push_back(pcA);
    assignments.push_back(spA);
    break;
  }

  case e_xchg: {
    // xchg defines two abslocs, and uses them as appropriate...

    std::vector<Operand> operands;
    I->getOperands(operands);

    // According to the InstructionAPI, the first operand will be the argument, the second will be ESP.
    assert(operands.size() == 2);

    // We use the first to define the second, and vice versa
    std::vector<AbsRegion> oper0;
    aConverter.convertAll(operands[0].getValue(),
                          addr,
                          func,
                          block,
                          oper0);
    
    std::vector<AbsRegion> oper1;
    aConverter.convertAll(operands[1].getValue(),
                          addr,
                          func,
                          block,
                          oper1);

    // Okay. We may have a memory reference in here, which will
    // cause either oper0 or oper1 to have multiple entries (the
    // remainder will be registers). So. Use everything from oper1
    // to define oper0[0], and vice versa.
    
    Assignment::Ptr a = Assignment::makeAssignment(I, addr, func, block, oper0[0]);
    a->addInputs(oper1);

    Assignment::Ptr b = Assignment::makeAssignment(I, addr, func, block, oper1[0]);
    b->addInputs(oper0);

    assignments.push_back(a);
    assignments.push_back(b);
    break;
  }


  case power_op_stwu: {
    std::vector<Operand> operands;
    I->getOperands(operands);

    // stwu <a>, <b>, <c>
    // <a> = R1
    // <b> = -16(R1)
    // <c> = R1

    // From this, R1 <= R1 - 16; -16(R1) <= R1
    // So a <= b (without a deref)
    // deref(b) <= c

    std::set<Expression::Ptr> writes;
    I->getMemoryWriteOperands(writes);
    assert(writes.size() == 1);

    Expression::Ptr tmp = *(writes.begin());
    AbsRegion effAddr = aConverter.convert(tmp,
					   addr, 
					   func,
                                           block);
    std::vector<AbsRegion> regions;
    aConverter.convertAll(operands[0].getValue(), addr, func, block, regions);
    AbsRegion RS = regions[0];
    regions.clear();
    aConverter.convertAll(operands[2].getValue(), addr, func, block, regions);
    AbsRegion RA = regions[0];

    Assignment::Ptr mem = Assignment::makeAssignment(I, 
							 addr,
							 func,
                                                         block,
							 effAddr);
    mem->addInput(RS);
    
    Assignment::Ptr ra = Assignment::makeAssignment(I,
							addr,
							func,
                                                        block,
							RA);
    ra->addInput(RS);
    assignments.push_back(mem);
    assignments.push_back(ra);
    break;
  }      
        
  default:
    // Assume full intra-dependence of non-flag and non-pc registers. 
    std::vector<AbsRegion> used;
    std::vector<AbsRegion> defined;

    aConverter.convertAll(I,
			  addr,
			  func,
                          block,
			  used,
			  defined);
    // PC should be regarded as a constant		
    AbsRegion pc(Absloc::makePC(func->isrc()->getArch()));
    for (auto uit = used.begin(); uit != used.end(); ++uit)
        if (*uit == pc) {
	    used.erase(uit);			 
	    break;
	}
    for (std::vector<AbsRegion>::const_iterator i = defined.begin();
	 i != defined.end(); ++i) {
       Assignment::Ptr a = Assignment::makeAssignment(I, addr, func, block, *i);
       a->addInputs(used);
       assignments.push_back(a);
    }
    break;
  }
    

  // Now for flags...
  // According to Matt, the easiest way to represent dependencies for flags on 
  // IA-32/AMD-64 is to have them depend on the inputs to the instruction and 
  // not the outputs of the instruction; therefore, there's no intra-instruction
  // dependence. 

  // PC-handling section
  // Most instructions use the PC to set the PC. This includes calls, relative branches,
  // and the like. So we're basically looking for indirect branches or absolute branches.
  // (are there absolutes on IA-32?).
  // Also, conditional branches and the flag registers they use. 

  if (cacheEnabled_) {
    cache_[func][addr] = assignments;
  }

}

void AssignmentConverter::handlePushEquivalent(const Instruction::Ptr I,
					       Address addr,
					       ParseAPI::Function *func,
                                               ParseAPI::Block *block,
					       std::vector<AbsRegion> &operands,
					       std::vector<Assignment::Ptr> &assignments) {
  // The handled-in operands are used to define *SP
  // And then we update SP
  
   AbsRegion stackTop = aConverter.stack(addr, func, block, true);
  AbsRegion sp(Absloc::makeSP(func->isrc()->getArch()));

  Assignment::Ptr spA = Assignment::makeAssignment(I,
						       addr,
						       func,
                                                       block,
						       stackTop);
  spA->addInputs(operands);
  spA->addInput(sp);

  Assignment::Ptr spB = Assignment::makeAssignment(I, addr, func, block, sp);
  spB->addInput(sp);

  assignments.push_back(spA);
  assignments.push_back(spB);
}

void AssignmentConverter::handlePopEquivalent(const Instruction::Ptr I,
					      Address addr,
					      ParseAPI::Function *func,
                                              ParseAPI::Block *block,
					      std::vector<AbsRegion> &operands,
					      std::vector<Assignment::Ptr> &assignments) {
  // We use the top of the stack and any operands beyond the first.
  // (Can you pop into memory?)

   AbsRegion stackTop = aConverter.stack(addr, func, block, false);
  AbsRegion sp(Absloc::makeSP(func->isrc()->getArch()));
  
  Assignment::Ptr spA = Assignment::makeAssignment(I,
						       addr,
						       func,
                                                       block,
						       operands[0]);
  spA->addInput(stackTop);
  spA->addInput(sp);

  for (unsigned i = 1; i < operands.size(); i++) {
    spA->addInput(operands[i]);
  }

  // Now stack assignment
  Assignment::Ptr spB = Assignment::makeAssignment(I, addr, func, block, sp);
  spB->addInput(sp);

  assignments.push_back(spA);
  assignments.push_back(spB);
}

bool AssignmentConverter::cache(ParseAPI::Function *func, 
				Address addr, 
				std::vector<Assignment::Ptr> &assignments) {
  if (!cacheEnabled_) {
    return false;
  }
  FuncCache::iterator iter = cache_.find(func);
  if (iter == cache_.end()) {
    return false;
  }
  AddrCache::iterator iter2 = iter->second.find(addr);
  if (iter2 == iter->second.end()) {
    return false;
  }
  assignments = iter2->second;
  return true;
}



