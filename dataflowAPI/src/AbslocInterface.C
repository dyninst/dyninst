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
#include "BinaryFunction.h"
#include "Immediate.h"

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

void AbsRegionConverter::convertAll(const InstructionAPI::Instruction &insn,
				    Address addr,
				    ParseAPI::Function *func,
                                    ParseAPI::Block *block,
				    std::vector<AbsRegion> &used,
				    std::vector<AbsRegion> &defined) {
                        
                        if (!usedCache(addr, func, used)) {
    std::set<RegisterAST::Ptr> regsRead;
    insn.getReadSet(regsRead);


    for (std::set<RegisterAST::Ptr>::const_iterator i = regsRead.begin();
	 i != regsRead.end(); ++i) {
        if(insn.getArch() == Arch_aarch64) {
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
    
    if (insn.readsMemory()) {
      std::set<Expression::Ptr> memReads;
      insn.getMemoryReadOperands(memReads);
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
    insn.getWriteSet(regsWritten);
    for (std::set<RegisterAST::Ptr>::const_iterator i = regsWritten.begin();
	 i != regsWritten.end(); ++i) {
      if(insn.getArch() == Arch_aarch64) {
            MachRegister machReg = (*i)->getID();
            std::vector<MachRegister> flagRegs = {aarch64::n, aarch64::z, aarch64::c, aarch64::v};

            if((machReg & 0xFF) == (aarch64::pstate & 0xFF) && (machReg & 0xFF0000) == (aarch64::SPR)) {
                for(std::vector<MachRegister>::iterator itr = flagRegs.begin(); itr != flagRegs.end(); itr++) {
                    defined.push_back(AbsRegionConverter::convert(RegisterAST::Ptr(new RegisterAST(*itr))));
                }
            } else {
                defined.push_back(AbsRegionConverter::convert(*i));
            }
        } else if (insn.getArch() == Arch_cuda && insn.hasPredicateOperand()) {
            Operand o = insn.getPredicateOperand();
            defined.push_back(AbsRegionConverter::convertPredicatedRegister(*i, o.getPredicate(), o.isTruePredicate()));

        } else {
            defined.push_back(AbsRegionConverter::convert(*i));
        }
    }

    // special case for repeat-prefixed instructions on x86
    // may disappear if Dyninst's representation of these instructions changes
    if (insn.getArch() == Arch_x86) {
      prefixEntryID insnPrefix = insn.getOperation().getPrefixID();
      if ( (prefix_rep == insnPrefix) || (prefix_repnz == insnPrefix) ) {
        defined.push_back(AbsRegionConverter::convert(RegisterAST::Ptr(
          new RegisterAST(MachRegister::getPC(Arch_x86)))));
      }
    }
    
    if (insn.writesMemory()) {
      std::set<Expression::Ptr> memWrites;
      insn.getMemoryWriteOperands(memWrites);
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
  // We do not distinguish partial registers from full register.
  // So, eax and rax are treated the same.
  // But for flags, we want to separate CF, ZF, and so on
  if (reg->getID().isFlag()) {
    return AbsRegion(Absloc(reg->getID()));
  } else {
    return AbsRegion(Absloc(reg->getID().getBaseRegister()));
  }		   
}

AbsRegion AbsRegionConverter::convertPredicatedRegister(RegisterAST::Ptr r, RegisterAST::Ptr p, bool c) {
    return AbsRegion(Absloc(r->getID(), p->getID(), c));
}

class bindKnownRegs : public InstructionAPI::Visitor
{
public:
    bindKnownRegs(Address sp, Address fp, Address ip, bool sdef, bool fdef) :
            defined(true),
            is_stack(false),
            is_frame(false),
            m_sp(sp),
            m_fp(fp),
            m_ip(ip),
            stackDefined(sdef),
            frameDefined(fdef) {}
    virtual ~bindKnownRegs() {}
    bool defined;
    bool is_stack;
    bool is_frame;
    std::deque<long> results;
    Address m_sp;
    Address m_fp;
    Address m_ip;
    bool stackDefined;
    bool frameDefined;
    long getResult() {
        if(results.empty()) return 0;
        return results.front();
    }
    bool isDefined() {
        return defined && (results.size() == 1);
    }
    virtual void visit(BinaryFunction* b)
    {
        if(!defined) return;
        long arg1 = results.back();
        results.pop_back();
        long arg2 = results.back();
        results.pop_back();
        if(b->isAdd())
        {
            results.push_back(arg1+arg2);
        }
        else if(b->isMultiply())
        {
            results.push_back(arg1*arg2);
        }
        else
        {
            defined = false;
        }
    }
    virtual void visit(Immediate* i)
    {
        if(!defined) return;
        results.push_back(i->eval().convert<long>());
    }
    virtual void visit(RegisterAST* r)
    {
        if(!defined) return;
        if(r->getID().isPC())
        {
            results.push_back(m_ip);
            return;
        }
        if(r->getID().isFramePointer() && frameDefined)
        {
            results.push_back(m_fp);
            is_frame = true;
            return;
        }
        if(r->getID().isStackPointer() && stackDefined)
        {
            results.push_back(m_sp);
            is_stack = true;
            return;
        }

        defined = false;
        results.push_back(0);
    }
    virtual void visit(Dereference* )
    {
        //defined = false;
    }

};


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

    // Currently, we only bind sp, fp, and pc.
    // If we decide to also bind aliases of these registers,
    // we need to change bindKnownRegs accordingly.
    Address pcVal = addr;
    if (block->obj()->cs()->getArch() == Arch_x86 || block->obj()->cs()->getArch() == Arch_x86_64) {
        // PC value on x86/64 is post-instruction
        pcVal += block->getInsn(addr).size();
    }

    bindKnownRegs calc(spHeight, fpHeight, pcVal, stackDefined, frameDefined);
    exp->apply(&calc);
    bool isFrame = calc.is_frame;
    bool isStack = calc.is_stack;
    Address res = calc.getResult();

    if (isFrame && stackAnalysisEnabled_) {
      if (calc.isDefined() && frameDefined) {
	return AbsRegion(Absloc(res, 0, func));
      }
      else {
	return AbsRegion(Absloc::Stack);
      }
    }

    if (isStack && stackAnalysisEnabled_) {
      if (calc.isDefined() && stackDefined) {
         return AbsRegion(Absloc(res,
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
    if (calc.isDefined()) {
      return AbsRegion(Absloc(res));
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

void AssignmentConverter::convert(const Instruction &I, 
                                  const Address &addr,
				  ParseAPI::Function *func,
                                  ParseAPI::Block *block,
				  std::vector<Assignment::Ptr> &assignments) {
  assignments.clear();
  if (cache(func, addr, assignments)){ 
      std::cout << "returning cached for " << I.format() << std::endl;
      return;
  }

  // Decompose the instruction into a set of abstract assignments.
  // We don't have the Definition class concept yet, so we'll do the 
  // hard work here. 
  // Two phases:
  // 1) Special-cased for IA32 multiple definition instructions,
  //    based on the opcode of the instruction
  // 2) Generic handling for things like flags and the PC. 

  // Non-PC handling section
  switch(I.getOperation().getID()) {
  case amdgpu_op_s_getpc_b64: {
    // SGPR_PAIR[0] = PC & 0xffffffff
    // SGPR_PARI[1] = PC >> 32
    //
    std::vector<Operand> operands;
    I.getOperands(operands);
    assert(operands.size() == 1);
    RegisterAST::Ptr sgpr_pair = boost::dynamic_pointer_cast<RegisterAST>(operands[0].getValue());
    unsigned int offset = sgpr_pair->getID() - amdgpu_vega::sgpr_vec2_0 ;
    AbsRegion lowpc_dst = AbsRegion(MachRegister(amdgpu_vega::sgpr0+offset)) ;
    AbsRegion highpc_dst = AbsRegion(MachRegister(amdgpu_vega::sgpr0+offset+1)) ;

    //AbsRegion pc = AbsRegion(Absloc::makePC(func->isrc()->getArch()));
    AbsRegion pc = AbsRegion(Absloc(addr));

    Assignment::Ptr lowpcA = Assignment::makeAssignment(I, 
							 addr,
							 func,
                                                         block,
							 lowpc_dst);
    Assignment::Ptr highpcA = Assignment::makeAssignment(I, 
							 addr,
							 func,
                                                         block,
							 highpc_dst);
    //lowpcA->addInput(pc); // treating pc as constant
    //highpcA->addInput(pc); //treating pc as constant

    assignments.push_back(lowpcA);
    assignments.push_back(highpcA);
    // The slicing stops as long as we find at least one of the above assgignment 

    // TODO:
    // DST_SGPR_PAIR = PC+4
    break;
  }
  case amdgpu_op_s_setpc_b64: {
    // TODO:
    // PC = SRC_SGPR_PAIR
    AbsRegion pc = AbsRegion(Absloc::makePC(func->isrc()->getArch()));
    Assignment::Ptr pcA = Assignment::makeAssignment(I, 
							 addr,
							 func,
                                                         block,
							 pc);

    std::vector<Operand> operands;
    I.getOperands(operands);
    // SETPC_B64 should only have one operand, which is the SGPR_PAIR that stores the new PC VALUE
    // Since we don't want to introduce extra Class at the instructionAPI level, we need to break it down ourserlves here
    // So we don't need to deal with reading the value in rose DispatcherAmdgpuVega
    // Depends on the situation, we might also need to differentaite betwwen  vega and rdna, but for now we focus on vega
    assert(operands.size() == 1);
    RegisterAST::Ptr sgpr_pair = boost::dynamic_pointer_cast<RegisterAST>(operands[0].getValue());
    unsigned int offset = sgpr_pair->getID() - amdgpu_vega::sgpr_vec2_0 ;
    AbsRegion oper0 = AbsRegion(MachRegister(amdgpu_vega::sgpr0+offset)) ;
    AbsRegion oper1 = AbsRegion(MachRegister(amdgpu_vega::sgpr0+offset+1)) ;
    pcA->addInput(oper0);
    pcA->addInput(oper1);

    assignments.push_back(pcA);
    break;
  }
  case amdgpu_op_s_swappc_b64: {
    // TODO:DST_SGPR_PAIR= PC + 4
    // PC = SRC_SGPR_PAIR

    //PC = OPR[0] 
    //OPR[1] = OLD_PC + 4
    // => 
    //PC  =  OPR[0:1] 
    //OPR[2:3] = OLD_PC + 4
    //
    std::vector<Operand> operands;
    I.getOperands(operands);
    assert(operands.size() == 2);

    RegisterAST::Ptr store_sgpr_pair = boost::dynamic_pointer_cast<RegisterAST>(operands[1].getValue());
    unsigned int store_offset = store_sgpr_pair->getID() - amdgpu_vega::sgpr_vec2_0 ;
    AbsRegion store_oper0 = AbsRegion(MachRegister(amdgpu_vega::sgpr0+store_offset)) ;
    AbsRegion store_oper1 = AbsRegion(MachRegister(amdgpu_vega::sgpr0+store_offset+1)) ;


    AbsRegion pc = AbsRegion(Absloc::makePC(func->isrc()->getArch()));

    Assignment::Ptr store_pc_lowA = Assignment::makeAssignment(I, 
							 addr,
							 func,
                             block,
							 store_oper0);
    store_pc_lowA->addInput(pc);


    Assignment::Ptr store_pc_highA = Assignment::makeAssignment(I, 
							 addr,
							 func,
                             block,
							 store_oper1);
    store_pc_highA->addInput(pc);

    Assignment::Ptr pcA = Assignment::makeAssignment(I, 
							 addr,
							 func,
                             block,
							 pc);

    RegisterAST::Ptr target_sgpr_pair = boost::dynamic_pointer_cast<RegisterAST>(operands[0].getValue());
    unsigned int target_offset = target_sgpr_pair->getID() - amdgpu_vega::sgpr_vec2_0 ;
    AbsRegion target_oper0 = AbsRegion(MachRegister(amdgpu_vega::sgpr0+target_offset)) ;
    AbsRegion target_oper1 = AbsRegion(MachRegister(amdgpu_vega::sgpr0+target_offset+1)) ;

    pcA->addInput(target_oper0);
    pcA->addInput(target_oper1);

    assignments.push_back(pcA);
    // while the below 2 assignments are also there, we comment it out for now TODO:
    //assignments.push_back(store_pc_lowA); 
    //assignments.push_back(store_pc_highA);


    break;
  }
  case amdgpu_op_s_add_u32: {
    std::vector<Operand> operands;
    I.getOperands(operands);


    assert(operands.size() == 3 && "add_u32 needs 3 operands");

    RegisterAST::Ptr dst_sgpr = boost::dynamic_pointer_cast<RegisterAST>(operands[0].getValue());

    std::vector<AbsRegion> regions;

    aConverter.convertAll(operands[0].getValue(), addr, func, block, regions);
    AbsRegion dst1 = regions[0];
    regions.clear();
    aConverter.convertAll(operands[1].getValue(), addr, func, block, regions);
    AbsRegion src1 = regions[0];
    regions.clear();
    aConverter.convertAll(operands[2].getValue(), addr, func, block, regions);
    AbsRegion src0 = regions[0];
    regions.clear();



    AbsRegion scc = AbsRegion(MachRegister(amdgpu_vega::scc)) ;
    Assignment::Ptr scc_assign = Assignment::makeAssignment(I, 
							 addr,
							 func,
                             block,
							 scc);

   
    Assignment::Ptr add_assign = Assignment::makeAssignment(I, 
							 addr,
							 func,
                             block,
							 dst1);

    add_assign->addInput(src1);
    add_assign->addInput(src0);
    
    scc_assign->addInput(src1);
    scc_assign->addInput(src0);


    assignments.push_back(add_assign);
    assignments.push_back(scc_assign);


    // TODO:
    // D.U = S0.u +S1.U
    // SCC= (S0.u + S1.U >= 0x100000000ULL ? 1 : 0 )
    break;
  }

  case amdgpu_op_s_addc_u32: {
    std::vector<Operand> operands;
    I.getOperands(operands);


    assert(operands.size() == 3 && "add_u32 needs 3 operands");

    RegisterAST::Ptr dst_sgpr = boost::dynamic_pointer_cast<RegisterAST>(operands[0].getValue());

    std::vector<AbsRegion> regions;

    aConverter.convertAll(operands[0].getValue(), addr, func, block, regions);
    AbsRegion dst1 = regions[0];
    regions.clear();
    aConverter.convertAll(operands[1].getValue(), addr, func, block, regions);
    AbsRegion src1 = regions[0];
    regions.clear();
    aConverter.convertAll(operands[2].getValue(), addr, func, block, regions);
    AbsRegion src0 = regions[0];
    regions.clear();


    AbsRegion scc = AbsRegion(MachRegister(amdgpu_vega::scc)) ;

   
    Assignment::Ptr add_assign = Assignment::makeAssignment(I, 
							 addr,
							 func,
                             block,
							 dst1);
    add_assign->addInput(src1);
    add_assign->addInput(src0);
    add_assign->addInput(scc);
    assignments.push_back(add_assign);

    // TODO:
    // D.U = S0.U +S1. U - SCC
    // SCC= (S0.U +S1.U +SCC>= 0x100000000ULL? 1 : 0)
    break;
  }

  case e_push: {
    // SP = SP - 4 
    // *SP = <register>
 
    std::vector<Operand> operands;
    I.getOperands(operands);

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
    I.getOperands(operands);

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
    I.getOperands(operands);

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
    I.getOperands(operands);

    // stwu <a>, <b>, <c>
    // <a> = R1
    // <b> = -16(R1)
    // <c> = R1

    // From this, R1 <= R1 - 16; -16(R1) <= R1
    // So a <= b (without a deref)
    // deref(b) <= c

    std::set<Expression::Ptr> writes;
    I.getMemoryWriteOperands(writes);
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

void AssignmentConverter::handlePushEquivalent(const Instruction I,
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

void AssignmentConverter::handlePopEquivalent(const Instruction I,
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



