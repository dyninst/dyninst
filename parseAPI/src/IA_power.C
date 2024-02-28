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


#include "IA_power.h"

#include "instructionAPI/h/syscalls.h"

#include "common/src/arch.h"
#include "registers/ppc32_regs.h"
#include "registers/ppc64_regs.h"
#include "parseAPI/src/debug_parse.h"

#include <deque>
#include <iostream>
#include <sstream>
#include <functional>
#include <algorithm>
#include <set>
#include "Register.h"

using namespace Dyninst;
using namespace InstructionAPI;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::InsnAdapter;

static RegisterAST::Ptr ppc32_R11 (new RegisterAST (ppc32::r11));
static RegisterAST::Ptr ppc32_LR  (new RegisterAST (ppc32::lr));
static RegisterAST::Ptr ppc32_SP  (new RegisterAST (ppc32::r1));

static RegisterAST::Ptr ppc64_R11 (new RegisterAST (ppc64::r11));
static RegisterAST::Ptr ppc64_LR  (new RegisterAST (ppc64::lr));
static RegisterAST::Ptr ppc64_SP  (new RegisterAST (ppc64::r1));

IA_power::IA_power(Dyninst::InstructionAPI::InstructionDecoder dec_,
               Address start_, 
	       Dyninst::ParseAPI::CodeObject* o,
	       Dyninst::ParseAPI::CodeRegion* r,
	       Dyninst::InstructionSource *isrc,
	       Dyninst::ParseAPI::Block * curBlk_):
	           IA_IAPI(dec_, start_, o, r, isrc, curBlk_) {
}
IA_power::IA_power(const IA_power& rhs): IA_IAPI(rhs) {}

IA_power* IA_power::clone() const{
    return new IA_power(*this);
}


bool IA_power::isFrameSetupInsn(Instruction) const
{
    return false;
}

bool IA_power::isNop() const
{
    return false;
}

bool IA_power::isThunk() const {
    return false;
}

bool IA_power::isTailCall(const Function* context, EdgeTypeEnum type, unsigned int, const set<Address>& knownTargets) const
{
   // Collapse down to "branch" or "fallthrough"
    switch(type) {
       case CALL:
       case COND_TAKEN:
       case DIRECT:
       case INDIRECT:
       case RET:
          type = DIRECT;
          break;
       case COND_NOT_TAKEN:
       case FALLTHROUGH:
       case CALL_FT:
          type = FALLTHROUGH;
          break;
       default:
          return false;
    }

    parsing_printf("Checking for Tail Call for powerpc\n");
    context->obj()->cs()->incrementCounter(PARSE_TAILCALL_COUNT); 

    if (tailCalls.find(type) != tailCalls.end()) {
        parsing_printf("\tReturning cached tail call check result: %d\n", tailCalls[type]);
        if (tailCalls[type]) {
            context->obj()->cs()->incrementCounter(PARSE_TAILCALL_FAIL);
            return true;
        }
        return false;
    }

    bool valid; Address addr;
    boost::tie(valid, addr) = getCFT();

    Function* callee = _obj->findFuncByEntry(_cr, addr);
    Block* target = _obj->findBlockByEntry(_cr, addr);

    Function *preambleCallee = _obj->findFuncByEntry(_cr, addr - 8);
    Block* preambleTarget = _obj->findBlockByEntry(_cr, addr - 8);

    if (callee == NULL || (preambleCallee != NULL && callee->src() != HINT && preambleCallee->src() == HINT)) {
        callee = preambleCallee;
        target = preambleTarget;
    }
    if(curInsn().getCategory() == c_BranchInsn &&
       valid &&
       callee && 
       callee != context &&
       // We can only trust entry points from hints
       callee->src() == HINT &&
       /* the target can either be not parsed or not within the current context */
       ((target == NULL) || (target && !context->contains(target)))
       )
    {
      parsing_printf("\tjump to 0x%lx, TAIL CALL\n", addr);
      tailCalls[type] = true;
      return true;
    }

    if (valid && addr > 0 && !context->region()->contains(addr)) {
      parsing_printf("\tjump to 0x%lx in other regions, TAIL CALL\n", addr);
      tailCalls[type] = true;
      return true;
    }    

    if (curInsn().getCategory() == c_BranchInsn &&
            valid &&
            !callee) {
    if (knownTargets.find(addr) != knownTargets.end()) {
	    parsing_printf("\tjump to 0x%lx is known target in this function, NOT TAIL CALL\n", addr);
	    tailCalls[type] = false;
	    return false;
	}
    }

    if(allInsns.size() < 2) {
      if(context->addr() == _curBlk->start() && curInsn().getCategory() == c_BranchInsn)
      {
	parsing_printf("\tjump as only insn in entry block, TAIL CALL\n");
	tailCalls[type] = true;
	return true;
      }
      else
      {
        parsing_printf("\ttoo few insns to detect tail call\n");
        context->obj()->cs()->incrementCounter(PARSE_TAILCALL_FAIL);
        tailCalls[type] = false;
        return false;
      }
    }
   
    tailCalls[type] = false;
    context->obj()->cs()->incrementCounter(PARSE_TAILCALL_FAIL);
    return false;
}

bool IA_power::savesFP() const
{
    return false;
}

bool IA_power::isStackFramePreamble() const
{
    return false;
}

bool IA_power::cleansStack() const
{
    return false;
}

class PPCReturnPredicates : public Slicer::Predicates {
  virtual bool widenAtPoint(Assignment::Ptr p) {
    for (std::vector<AbsRegion>::const_iterator iter = p->inputs().begin();
	 iter != p->inputs().end(); ++iter) {
      if ((*iter).type() != Absloc::Unknown)
	return true;
    }
    return false;
  }
};



bool IA_power::sliceReturn(ParseAPI::Block* bit, Address ret_addr, ParseAPI::Function * func) const {

  parsing_printf(" sliceReturn ret 0x%lx address 0x%lx func %s addr 0x%lx \n", ret_addr, bit->lastInsnAddr(), func->name().c_str(), func->addr() );
  AST::Ptr pcDef;
  AssignmentConverter converter(true, true);
  vector<Assignment::Ptr>::iterator ait;
  vector<Assignment::Ptr> assgns;
  PPCReturnPredicates preds;

  Address retnAddr = bit->lastInsnAddr();
  InstructionDecoder retdec( _isrc->getPtrToInstruction( retnAddr ), 
                             InstructionDecoder::maxInstructionLength, 
                             _cr->getArch() );
  Instruction retn = retdec.decode();
  converter.convert(retn, retnAddr, func, bit, assgns);
  for (ait = assgns.begin(); assgns.end() != ait; ait++) {
      AbsRegion & outReg = (*ait)->out();
      if ( outReg.absloc().isPC() ) {
          Slicer slicer(*ait,bit,func);
          Graph::Ptr slGraph = slicer.backwardSlice(preds);
          DataflowAPI::Result_t slRes;
          DataflowAPI::SymEval::expand(slGraph,slRes);
          pcDef = slRes[*ait];
          /*
          for (DataflowAPI::SymEval::Result_t::const_iterator r_iter = slRes.begin();
               r_iter != slRes.end(); ++r_iter) {
              cout << "-----------------" << endl;
              cout << r_iter->first->format();
              cout << " == ";
              cout << (r_iter->second ? r_iter->second->format() : "<NULL>") << endl;
          }
          */
          break;
      }
  }

  if (!pcDef) { return false; }
  PPC_BLR_Visitor checker(ret_addr);
  pcDef->accept(&checker);
  if (checker.returnState() == PPC_BLR_Visitor::PPC_BLR_RETURN) {
    return true;
  } else {
            return false;
  }
}

bool IA_power::isReturnAddrSave(Address& retAddr) const
{
  RegisterAST::Ptr regLR, regSP;
  regLR = ppc32_LR; regSP = ppc32_SP;

 /* FIXME: InstructionAPI doesn't handle ppc64:LR correctly. 
  * For now, use ppc32:LR for ppc64 also.

  switch (_isrc->getArch()) {
  case Arch_ppc32: regLR = ppc32_LR; regSP = ppc32_SP; break;
  case Arch_ppc64: regLR = ppc64_LR; regSP = ppc64_SP; break;
  default: assert(0 && "Inappropriate _isrc architechture.");
  }
  */

  std::set < RegisterAST::Ptr > regs;
  RegisterAST::Ptr destLRReg;
  bool foundMFLR = false;
  Address ret = 0;
  int cnt = 1;
  Instruction ci = curInsn ();
   parsing_printf(" Examining address 0x%lx to check if LR is saved on stack \n", getAddr());
    parsing_printf("\t\tchecking insn %s \n", ci.format().c_str());

  if (ci.getOperation().getID () == power_op_mfspr &&
      ci.isRead (regLR))
    {
      foundMFLR = true;
      ci.getWriteSet (regs);
      if (regs.size () != 1)
	{
	  parsing_printf ("mfspr wrote %lu registers instead of 1. insn: %s\n",
			  regs.size (), ci.format ().c_str ());
	  return 0;
	}
      destLRReg = *(regs.begin ());
      retAddr = getAddr();
      parsing_printf ("Found MFLR saved in %s at 0x%lx\n",
                      destLRReg->format ().c_str (), getAddr());
    }

  if (foundMFLR)
    {

      // walk to first control flow transfer instruction, looking
      // for a save of destLRReg
      IA_power copy (dec, getAddr (), _obj, _cr, _isrc, _curBlk);
      while (!copy.hasCFT ())
	{
	  ci = copy.curInsn ();
	  if (ci.writesMemory () &&
	      ci.isRead (regSP) && ci.isRead (destLRReg))
	    {
	      ret = true;
	      break;
	    }
	  else if (ci.isWritten (destLRReg))
	    {
	      ret = false;
	      break;
	    }
	  copy.advance ();
	  ++cnt;
	}
    }
  parsing_printf ("[%s:%d] isReturnAddrSave examined %d instructions - returning %lu\n", FILE__, __LINE__, cnt, ret);
  return ret;
}

bool IA_power::isReturn(Dyninst::ParseAPI::Function * context, Dyninst::ParseAPI::Block* currBlk) const
{
  /* Check for leaf node or lw - mflr - blr pattern */
  if (curInsn().getCategory() != c_ReturnInsn) {
	parsing_printf(" Not BLR - returning false \n");
	return false;
   }
  Function *func = context;
  parsing_printf
    ("isblrReturn at 0x%lx Addr 0x%lx 0x%lx Function addr 0x%lx leaf %d \n",
     current, getAddr (), currBlk->start (), context->addr (),
     func->_is_leaf_function);
  if (!func->_is_leaf_function)
    {
      parsing_printf ("\t LR saved for %s \n", func->name().c_str());
      // Check for lwz from Stack - mtlr - blr 
      RegisterAST::Ptr regLR, regSP, reg11;
		regLR = ppc32_LR; regSP = ppc32_SP; reg11 = ppc32_R11;

 /* FIXME: InstructionAPI doesn't handle ppc64:LR correctly. 
  * For now, use ppc32:LR for ppc64 also.

      switch (_isrc->getArch()) {
      case Arch_ppc32:
          regLR = ppc32_LR; regSP = ppc32_SP; reg11 = ppc32_R11; break;
      case Arch_ppc64:
          regLR = ppc64_LR; regSP = ppc64_SP; reg11 = ppc64_R11; break;
      default: assert(0 && "Inappropriate _isrc architechture.");
      }
*/
      std::set < RegisterAST::Ptr > regs;
      RegisterAST::Ptr sourceLRReg;

      Instruction ci = curInsn ();
      bool foundMTLR = false;
      allInsns_t::reverse_iterator iter;
      Address blockStart = currBlk->start ();
      const unsigned char *b =
	(const unsigned char *) (this->_isrc->
				 getPtrToInstruction (blockStart));
      InstructionDecoder decCopy (b, currBlk->size (),
				  this->_isrc->getArch ());
      IA_power copy (decCopy, blockStart, _obj, _cr, _isrc, _curBlk);
      while (!copy.hasCFT ())
	{
	  copy.advance ();
	}
      for(iter = copy.allInsns.rbegin(); iter != copy.allInsns.rend(); iter++)
	{
	  parsing_printf ("\t\tchecking insn 0x%lx: %s \n", iter->first,
		  iter->second.format ().c_str ());
	  if (iter->second.getOperation().getID () == power_op_mtspr &&
	      iter->second.isWritten (regLR))
	    {
	      iter->second.getReadSet (regs);
	      if (regs.size () != 1)
		{
		  parsing_printf
		    ("expected mtspr to read 1 register, insn is %s\n",
		     ci.format ().c_str ());
		  return false;
		}
	      sourceLRReg = *(regs.begin ());
	      parsing_printf ("\t\t\t **** Found MTLR saved from %s \n",
		      sourceLRReg->format ().c_str ());
	      foundMTLR = true;
	    }
	  else if (foundMTLR &&
		   iter->second.readsMemory () &&
		   (iter->second.isRead (regSP) ||
		    (iter->second.isRead (reg11))) &&
		   iter->second.isWritten (sourceLRReg))
	    {
	      parsing_printf ("\t\t\t **** Found lwz - RETURNING TRUE\n");
	      return true;
	    }
	}

      parsing_printf (" Slicing for Addr 0x%lx startAddr 0x%lx ret addr 0x%lx func %s\n",
	      getAddr (), currBlk->start (), func->_ret_addr, func->name().c_str());
      bool ret = sliceReturn(currBlk, func->_ret_addr, func);

      func->invalidateCache();
      if (ret) {
	parsing_printf ("\t\t\t **** Slicing - is a return instruction\n");
	return true;
      } else {
	parsing_printf ("\t\t\t **** Slicing - is not a return instruction\n");
	return false;	
      }
    }
  else
    {
      parsing_printf ("\t leaf node  RETURNING TRUE \n");
      return true;
    }
}

bool IA_power::isFakeCall() const
{
    return false;
}

bool IA_power::isIATcall(std::string &) const
{
    return false;
}

bool IA_power::isLinkerStub() const
{
  // Disabling this code because it ends with an
  // incorrect CFG. 

  return false;
}

AST::Ptr PPC_BLR_Visitor::visit(AST *a) {
  return a->ptr(); 
}

AST::Ptr PPC_BLR_Visitor::visit(DataflowAPI::BottomAST *b) {
  return_ = PPC_BLR_UNKNOWN;
  return b->ptr();
}

AST::Ptr PPC_BLR_Visitor::visit(DataflowAPI::ConstantAST *c) {
  // Very odd case, but claiming not a return
  return_ = PPC_BLR_NOTRETURN;
  return c->ptr();
}

AST::Ptr PPC_BLR_Visitor::visit(DataflowAPI::VariableAST *v) {
  if ((v->val().reg == AbsRegion(ppc32::lr)) &&
      (v->val().addr == ret_)) {
    return_ = PPC_BLR_RETURN;
  }
  // Check the stack
  else if ((v->val().reg.type() == Absloc::Unknown) &&
      (v->val().reg.absloc().type() == Absloc::Stack) &&
      (v->val().reg.absloc().off() == 4)) {
    // FIXME WORDSIZE
    return_ = PPC_BLR_RETURN;
  }
  else {
    return_ = PPC_BLR_UNKNOWN;
  }
  return v->ptr();
}
/*
AST::Ptr PPC_BLR_Visitor::visit(StackAST *s) {
  return_ = UNKNOWN;
  return s->Ptr();
}
*/
AST::Ptr PPC_BLR_Visitor::visit(DataflowAPI::RoseAST *r) {
  if (return_ != PPC_BLR_UNSET) {
    return r->ptr();
  }

  switch(r->val().op) {
  case DataflowAPI::ROSEOperation::andOp: {
    assert(r->numChildren() == 2);
    if (r->child(1)->getID() != AST::V_ConstantAST) {
      return_ = PPC_BLR_UNKNOWN;
      return r->ptr();
    }
    DataflowAPI::ConstantAST::Ptr mask = DataflowAPI::ConstantAST::convert(r->child(1));
    if (mask->val().val != 0xfffffffc) {
      return_ = PPC_BLR_UNKNOWN;
      return r->ptr();
    }

    r->child(0)->accept(this);
    break;
  }
  default:
    return_ = PPC_BLR_UNKNOWN;
    break;
  }
  return r->ptr();
}
      

 

#if 0
ParseAPI::StackTamper
IA_power::tampersStack(ParseAPI::Function *, Address &) const
{
    return TAMPER_NONE;
}
#endif

bool IA_power::isNopJump() const
{
    return false;
}
