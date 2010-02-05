

#include "Absloc.h"
#include "AbslocInterface.h"

// Pile of InstructionAPI includes
#include "Expression.h"
#include "Register.h"
#include "Result.h"
#include "Dereference.h"


// Dyninst internals...

#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/image-func.h"
#include "dyninstAPI/src/stackanalysis.h"

using namespace Dyninst;
using namespace Dyninst::InstructionAPI;

void AbsRegionConverter::convertAll(InstructionAPI::Expression::Ptr expr,
				    Address addr,
				    image_func *func,
				    std::vector<AbsRegion> &regions) {
  // If we're a memory dereference, then convert us and all
  // used registers.
  if (dyn_detail::boost::dynamic_pointer_cast<Dereference>(expr)) {
    std::vector<InstructionAST::Ptr> tmp;
    // Strip dereference...
    expr->getChildren(tmp);
    for (std::vector<InstructionAST::Ptr>::const_iterator i = tmp.begin();
	 i != tmp.end(); ++i) {
      regions.push_back(convert(dyn_detail::boost::dynamic_pointer_cast<Expression>(*i), addr, func));
    }
  }
  
  // Otherwise just convert registers
  
  std::set<InstructionAST::Ptr> used;
  expr->getUses(used);
  for (std::set<InstructionAST::Ptr>::const_iterator j = used.begin();
       j != used.end(); ++j) {
    regions.push_back(convert(dyn_detail::boost::dynamic_pointer_cast<RegisterAST>(*j)));
  }
}

void AbsRegionConverter::convertAll(InstructionAPI::Instruction::Ptr insn,
				    Address addr,
				    image_func *func,
				    std::vector<AbsRegion> &used,
				    std::vector<AbsRegion> &defined) {
  std::set<RegisterAST::Ptr> regsRead;
  insn->getReadSet(regsRead);

  for (std::set<RegisterAST::Ptr>::const_iterator i = regsRead.begin();
       i != regsRead.end(); ++i) {
    used.push_back(AbsRegionConverter::convert(*i));
  }
  
  if (insn->readsMemory()) {
    std::set<Expression::Ptr> memReads;
    insn->getMemoryReadOperands(memReads);
    for (std::set<Expression::Ptr>::const_iterator r = memReads.begin();
	 r != memReads.end();
	 ++r) {
      used.push_back(AbsRegionConverter::convert(*r, addr, func));
    }
  }
  
  // Defined time
  std::set<RegisterAST::Ptr> regsWritten;
  insn->getWriteSet(regsWritten);
  
  for (std::set<RegisterAST::Ptr>::const_iterator i = regsWritten.begin();
       i != regsWritten.end(); ++i) {
    defined.push_back(AbsRegionConverter::convert(*i));
  }
  
  if (insn->writesMemory()) {
    std::set<Expression::Ptr> memWrites;
    insn->getMemoryWriteOperands(memWrites);
    for (std::set<Expression::Ptr>::const_iterator r = memWrites.begin();
	 r != memWrites.end();
	 ++r) {
      defined.push_back(AbsRegionConverter::convert(*r, addr, func));
    }
  }
}

AbsRegion AbsRegionConverter::convert(RegisterAST::Ptr reg) {
  // FIXME:
  // Upcast register so we can be sure to match things later
  
  Absloc aloc(Absloc::Register,
	      RegisterAST::promote(reg.get())->getID());
  // Get the AbsRegion containing this register (x86 aliasing, TBD)
  return AbsRegion(aloc);
}

AbsRegion AbsRegionConverter::convert(Expression::Ptr exp,
				      Address addr,
				      image_func *func) {
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
    int spRegion = 0;
    bool stackExists = getCurrentStackHeight(func,
					     addr, 
					     spHeight, 
					     spRegion);

    bool frameExists = getCurrentFrameStatus(func,
					     addr);

    bool isStack = false;
    bool isFrame = false;

    static RegisterAST *spRegs32[2] = {NULL, NULL};
    static RegisterAST *fpRegs32[2] = {NULL, NULL};

    static RegisterAST *spRegs64[2] = {NULL, NULL};
    static RegisterAST *fpRegs64[2] = {NULL, NULL};

    static RegisterAST *ipReg32 = NULL;
    static RegisterAST *ipReg64 = NULL;

    if (spRegs32[0] == NULL) {
        spRegs32[0] = new RegisterAST(r_eSP);
        spRegs32[1] = new RegisterAST(r_ESP);

        spRegs64[0] = new RegisterAST(r_rSP);
        spRegs64[1] = new RegisterAST(r_RSP);

        fpRegs32[0] = new RegisterAST(r_eBP);
        fpRegs32[1] = new RegisterAST(r_EBP);

        fpRegs64[0] = new RegisterAST(r_rBP);
        fpRegs64[1] = new RegisterAST(r_RBP);

	ipReg32 = new RegisterAST(r_EIP);
	ipReg64 = new RegisterAST(r_RIP);
    }

    // We currently have to try and bind _every_ _single_ _alias_
    // of the stack pointer...
    if (stackExists) {
      if (exp->bind(spRegs32[0], Result(u32, spHeight)) ||
	  exp->bind(spRegs32[1], Result(u32, spHeight)) ||
	  exp->bind(spRegs64[0], Result(u64, spHeight)) ||
	  exp->bind(spRegs64[1], Result(u64, spHeight))) {
	isStack = true;
      }
    }
    if (frameExists) {
        if (exp->bind(fpRegs32[0], Result(u32, 0)) ||
            exp->bind(fpRegs32[1], Result(u32, 0)) ||
            exp->bind(fpRegs64[0], Result(u64, 0)) ||
            exp->bind(fpRegs64[1], Result(u64, 0))) {
            isFrame = true;
        }
    }

    // Bind the IP, why not...
    exp->bind(ipReg32, Result(u32, addr));
    exp->bind(ipReg64, Result(u64, addr));

    Result res = exp->eval();

    if (!res.defined) {
      return AbsRegion(Absloc::Heap);
    }
    
    Address resAddr;
    if (!convertResultToAddr(res, resAddr))
      return AbsRegion(Absloc::Heap);

    if (isStack) {
      return AbsRegion(Absloc(Absloc::Stack, resAddr, spRegion, func->symTabName()));
    }
    
    // Frame-based accesses are always from region 0...
    if (isFrame) {
      return AbsRegion(Absloc(Absloc::Stack, resAddr, 0, func->symTabName()));
    }


    return AbsRegion(Absloc(Absloc::Heap, resAddr));
}

AbsRegion AbsRegionConverter::stack(Address addr,
				    image_func *func) {
    long spHeight = 0;
    int spRegion = 0;
    bool stackExists = getCurrentStackHeight(func,
					     addr, 
					     spHeight, 
					     spRegion);
    if (!stackExists) {
      return AbsRegion(Absloc::Heap);
    }

    return AbsRegion(Absloc(Absloc::Stack,
			    spHeight,
			    spRegion,
			    func->symTabName()));
}

bool AbsRegionConverter::getCurrentStackHeight(image_func *func,
					       Address addr,
					       long &height,
					       int &region) {
  StackAnalysis sA(func);
  const Dyninst::StackAnalysis::HeightTree *hT = sA.heightIntervals();
  
  StackAnalysis::Height heightSA;
  
  //Offset off = func->lowlevel_func()->addrToOffset(addr);
  Offset off = addr;

  if (!hT->find(off, heightSA)) {
    return false;
  }
  
  // Ensure that analysis has been performed.
  assert(!heightSA.isTop());
  
  if (heightSA.isBottom()) {
    return false;
  }
  
  height = heightSA.height();
  region = heightSA.region()->name();
  
  return true;
}

bool AbsRegionConverter::getCurrentFrameStatus(image_func *func,
					       Address addr) {
  StackAnalysis sA(func);
  
  const Dyninst::StackAnalysis::PresenceTree *pT = sA.presenceIntervals();
  Dyninst::StackAnalysis::Presence exists;
  
  //Offset off = func->lowlevel_func()->addrToOffset(addr);
  Offset off = addr;

  if (!pT->find(off, exists)) return false;
  
  assert(!exists.isTop());
  
  return (exists.presence() == StackAnalysis::Presence::frame_t);
}

bool AbsRegionConverter::convertResultToAddr(const InstructionAPI::Result &res, Address &addr) {
    assert(res.defined);
    switch (res.type) {
    case u8:
        addr = (Address) res.val.u8val;
        return true;
    case s8:
        addr = (Address) res.val.s8val;
        return true;
    case u16:
        addr = (Address) res.val.u16val;
        return true;
    case s16:
        addr = (Address) res.val.s16val;
        return true;
    case u32:
        addr = (Address) res.val.u32val;
        return true;
    case s32:
        addr = (Address) res.val.s32val;
        return true;
    case u48:
        addr = (Address) res.val.u48val;
        return true;
    case s48:
        addr = (Address) res.val.s48val;
        return true;
    case u64:
        addr = (Address) res.val.u64val;
        return true;
    case s64:
        addr = (Address) res.val.s64val;
        return true;
    default:
        return false;
    }
}

bool AbsRegionConverter::convertResultToSlot(const InstructionAPI::Result &res, int &addr) {
    assert(res.defined);
    switch (res.type) {
    case u8:
        addr = (int) res.val.u8val;
        return true;
    case s8:
        addr = (int) res.val.s8val;
        return true;
    case u16:
        addr = (int) res.val.u16val;
        return true;
    case s16:
        addr = (int) res.val.s16val;
        return true;
    case u32:
        addr = (int) res.val.u32val;
        return true;
    case s32:
        addr = (int) res.val.s32val;
        return true;
        // I'm leaving these in because they may get used, but
        // we're definitely truncating them down.
    case u48:
        addr = (int) res.val.u48val;
        return true;
    case s48:
        addr = (int) res.val.s48val;
        return true;
    case u64:
        addr = (int) res.val.u64val;
        return true;
    case s64:
        addr = (int) res.val.s64val;
        return true;
    default:
        return false;
    }
}


///////////////////////////////////////////////////////
// Create a set of Assignments from an InstructionAPI
// Instruction.
///////////////////////////////////////////////////////

void AssignmentConverter::convert(const Instruction::Ptr I, 
                                  const Address &addr,
				  image_func *func,
				  std::set<Assignment::Ptr> &assignments) {
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
    AbsRegionConverter::convertAll(operands[0].getValue(),
				   addr,
				   func,
				   oper0);

    handlePushEquivalent(I, addr, func, oper0, assignments);
    break;
  }
  case e_call: {
    // This can be seen as a push of the PC...

    std::vector<AbsRegion> pcRegion;
    pcRegion.push_back(Absloc::makePC());

    handlePushEquivalent(I, addr, func, pcRegion, assignments);
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
    AbsRegionConverter::convertAll(operands[0].getValue(),
				   addr,
				   func,
				   oper0);

    handlePopEquivalent(I, addr, func, oper0, assignments);
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
        
    AbsRegion sp(Absloc::makeSP());
    AbsRegion fp(Absloc::makeFP());

    // Should be "we assign SP using FP"
    Assignment::Ptr spA = Assignment::Ptr(new Assignment(I,
							 addr,
							 sp));
    spA->addInput(fp);

    // And now we want "FP = (stack slot 0)"
    AbsRegion stackTop(Absloc(Absloc::Stack,
			      0,
			      0,
			      func->symTabName()));

    Assignment::Ptr fpA = Assignment::Ptr(new Assignment(I,
							 addr,
							 fp));
    fpA->addInput(AbsRegionConverter::stack(addr + I->size(), func));

    assignments.insert(spA);
    assignments.insert(fpA);
    break;
  }
  case e_ret_near:
  case e_ret_far: {
    // PC = *SP
    // SP = SP + 4/8
    // Like pop, except it's all implicit.

    AbsRegion pc = AbsRegion(Absloc::makePC());
    Assignment::Ptr pcA = Assignment::Ptr(new Assignment(I, 
							 addr,
							 pc));
    pcA->addInput(AbsRegionConverter::stack(addr, func));

    AbsRegion sp = AbsRegion(Absloc::makeSP());
    Assignment::Ptr spA = Assignment::Ptr(new Assignment(I,
							 addr,
							 sp));
    spA->addInput(sp);

    assignments.insert(pcA);
    assignments.insert(spA);
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
    AbsRegionConverter::convertAll(operands[0].getValue(),
				   addr,
				   func,
				   oper0);
    
    std::vector<AbsRegion> oper1;
    AbsRegionConverter::convertAll(operands[1].getValue(),
				   addr,
				   func,
				   oper1);

    // Okay. We may have a memory reference in here, which will
    // cause either oper0 or oper1 to have multiple entries (the
    // remainder will be registers). So. Use everything from oper1
    // to define oper0[0], and vice versa.
    
    Assignment::Ptr a = Assignment::Ptr(new Assignment(I, addr, oper0[0]));
    a->addInputs(oper1);

    Assignment::Ptr b = Assignment::Ptr(new Assignment(I, addr, oper1[0]));
    b->addInputs(oper0);

    assignments.insert(a);
    assignments.insert(b);
    break;
  }
        
  default:
    // Assume full intra-dependence of non-flag and non-pc registers. 
    std::vector<AbsRegion> used;
    std::vector<AbsRegion> defined;

    AbsRegionConverter::convertAll(I,
				   addr,
				   func,
				   used,
				   defined);

    for (std::vector<AbsRegion>::const_iterator i = defined.begin();
	 i != defined.end(); ++i) {
      Assignment::Ptr a = Assignment::Ptr(new Assignment(I, addr, *i));
      a->addInputs(used);
      assignments.insert(a);
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

}

void AssignmentConverter::handlePushEquivalent(const Instruction::Ptr I,
					       Address addr,
					       image_func *func,
					       std::vector<AbsRegion> &operands,
					       std::set<Assignment::Ptr> &assignments) {
  // The handled-in operands are used to define *SP
  // And then we update SP

  AbsRegion stackTop = AbsRegionConverter::stack(addr, func);
  AbsRegion sp(Absloc::makeSP());

  Assignment::Ptr spA = Assignment::Ptr(new Assignment(I,
						       addr,
						       stackTop));
  spA->addInputs(operands);
  spA->addInput(sp);

  Assignment::Ptr spB = Assignment::Ptr(new Assignment(I, addr, sp));
  spB->addInput(sp);

  assignments.insert(spA);
  assignments.insert(spB);
}

void AssignmentConverter::handlePopEquivalent(const Instruction::Ptr I,
					      Address addr,
					      image_func *func,
					      std::vector<AbsRegion> &operands,
					      std::set<Assignment::Ptr> &assignments) {
  // We use the top of the stack and any operands beyond the first.
  // (Can you pop into memory?)

  AbsRegion stackTop = AbsRegionConverter::stack(addr, func);
  AbsRegion sp(Absloc::makeSP());
  
  Assignment::Ptr spA = Assignment::Ptr(new Assignment(I,
						       addr,
						       operands[0]));
  spA->addInput(stackTop);
  spA->addInput(sp);

  for (unsigned i = 1; i < operands.size(); i++) {
    spA->addInput(operands[i]);
  }

  // Now stack assignment
  Assignment::Ptr spB = Assignment::Ptr(new Assignment(I, addr, sp));
  spB->addInput(sp);

  assignments.insert(spA);
  assignments.insert(spB);
}
