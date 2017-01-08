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


#include "IA_IAPI.h"
#include "IA_power.h"

#include "Register.h"
#include "Dereference.h"
#include "Immediate.h"
#include "BinaryFunction.h"

#include "common/src/arch.h"

#include "parseAPI/src/debug_parse.h"

#include <deque>
#include <iostream>
#include <sstream>
#include <functional>
#include <algorithm>
#include <set>

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

bool IA_IAPI::isFrameSetupInsn(Instruction::Ptr) const
{
    return false;
}

bool IA_IAPI::isNop() const
{
    return false;
}

bool IA_IAPI::isThunk() const {
    return false;
}

bool IA_IAPI::isTailCall(Function* context, EdgeTypeEnum type, unsigned int, const set<Address>& knownTargets) const
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

    parsing_printf("Checking for Tail Call \n");
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
    if (type == DIRECT)
       boost::tie(valid, addr) = getCFT();
    else 
       boost::tie(valid, addr) = getFallthrough();

    if(curInsn()->getCategory() == c_BranchInsn &&
       valid &&
       _obj->findFuncByEntry(_cr,addr))
    {
      parsing_printf("\tjump to 0x%lx, TAIL CALL\n", addr);
      tailCalls[type] = true;
      return true;
    }

    if(allInsns.size() < 2) {
        parsing_printf("\ttoo few insns to detect tail call\n");
        context->obj()->cs()->incrementCounter(PARSE_TAILCALL_FAIL);
        tailCalls[type] = false;
        return false;
    }
    tailCalls[type] = false;
    context->obj()->cs()->incrementCounter(PARSE_TAILCALL_FAIL);
    return false;
}

bool IA_IAPI::savesFP() const
{
    return false;
}

bool IA_IAPI::isStackFramePreamble() const
{
    return false;
}

bool IA_IAPI::cleansStack() const
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



bool IA_IAPI::sliceReturn(ParseAPI::Block* bit, Address ret_addr, ParseAPI::Function * func) const {

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
  Instruction::Ptr retn = retdec.decode();
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

bool IA_IAPI::isReturnAddrSave(Address& retAddr) const
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
  Instruction::Ptr ci = curInsn ();
   parsing_printf(" Examining address 0x%lx to check if LR is saved on stack \n", getAddr());
    parsing_printf("\t\tchecking insn %s \n", ci->format().c_str());

  if (ci->getOperation ().getID () == power_op_mfspr &&
      ci->isRead (regLR))
    {
      foundMFLR = true;
      ci->getWriteSet (regs);
      if (regs.size () != 1)
	{
	  parsing_printf ("mfspr wrote %d registers instead of 1. insn: %s\n",
			  regs.size (), ci->format ().c_str ());
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
      IA_IAPI copy (dec, getAddr (), _obj, _cr, _isrc, _curBlk);
      while (!copy.hasCFT () && copy.curInsn ())
	{
	  ci = copy.curInsn ();
	  if (ci->writesMemory () &&
	      ci->isRead (regSP) && ci->isRead (destLRReg))
	    {
	      ret = true;
	      break;
	    }
	  else if (ci->isWritten (destLRReg))
	    {
	      ret = false;
	      break;
	    }
	  copy.advance ();
	  ++cnt;
	}
    }
  parsing_printf ("[%s:%d] isReturnAddrSave examined %d instructions - returning %d \n", FILE__, __LINE__, cnt, ret);
  return ret;
}

bool IA_IAPI::isReturn(Dyninst::ParseAPI::Function * context, Dyninst::ParseAPI::Block* currBlk) const
{
  /* Check for leaf node or lw - mflr - blr pattern */
  if (curInsn()->getCategory() != c_ReturnInsn) {
	parsing_printf(" Not BLR - returning false \n");
	return false;
   }
  Instruction::Ptr ci = curInsn ();
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

      Instruction::Ptr ci = curInsn ();
      bool foundMTLR = false;
      allInsns_t::reverse_iterator iter;
      Address blockStart = currBlk->start ();
      const unsigned char *b =
	(const unsigned char *) (this->_isrc->
				 getPtrToInstruction (blockStart));
      InstructionDecoder decCopy (b, currBlk->size (),
				  this->_isrc->getArch ());
      IA_IAPI copy (decCopy, blockStart, _obj, _cr, _isrc, _curBlk);
      while (copy.getInstruction () && !copy.hasCFT ())
	{
	  copy.advance ();
	}
      for(iter = copy.allInsns.rbegin(); iter != copy.allInsns.rend(); iter++)
	{
	  parsing_printf ("\t\tchecking insn 0x%x: %s \n", iter->first,
		  iter->second->format ().c_str ());
	  if (iter->second->getOperation ().getID () == power_op_mtspr &&
	      iter->second->isWritten (regLR))
	    {
	      iter->second->getReadSet (regs);
	      if (regs.size () != 1)
		{
		  parsing_printf
		    ("expected mtspr to read 1 register, insn is %s\n",
		     ci->format ().c_str ());
		  return false;
		}
	      sourceLRReg = *(regs.begin ());
	      parsing_printf ("\t\t\t **** Found MTLR saved from %s \n",
		      sourceLRReg->format ().c_str ());
	      foundMTLR = true;
	    }
	  else if (foundMTLR &&
		   iter->second->readsMemory () &&
		   (iter->second->isRead (regSP) ||
		    (iter->second->isRead (reg11))) &&
		   iter->second->isWritten (sourceLRReg))
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

bool IA_IAPI::isFakeCall() const
{
    return false;
}

bool IA_IAPI::isIATcall(std::string &) const
{
    return false;
}

const unsigned int B_UNCOND      = 0x48000000;
const unsigned int ADDIS_R12_R12 = 0x3d8c0000;
const unsigned int ADDIS_R12_R2  = 0x3d820000;
const unsigned int ADDIS_R2_R2   = 0x3c420000;
const unsigned int ADDI_R12_R12  = 0x398c0000;
const unsigned int ADDI_R2_R2    = 0x38420000;
const unsigned int STD_R2_40R1   = 0xf8410028;
const unsigned int LD_R2_40R1    = 0xe8410028;
const unsigned int LD_R2_0R2     = 0xe8420000;
const unsigned int LD_R2_0R12    = 0xe84c0000;
const unsigned int LD_R11_0R12   = 0xe96c0000;
const unsigned int LD_R11_0R2    = 0xe9620000;
const unsigned int MTCTR_R11     = 0x7d6903a6;
const unsigned int BCTR          = 0x4e800420;

typedef enum {
    STUB_UNKNOWN,
    STUB_LONG_BRANCH,
    STUB_TOC_BRANCH,
    STUB_PLT_CALL
} linker_stub_t;

linker_stub_t checkLinkerStub(void *insn_buf, Offset &off)
{
    instruction *insn = static_cast<instruction *>(insn_buf);

#if defined(ppc64_linux)
    /*
     * Linker stubs seen from GNU's binutils.
     * (see the following functions in binutils' bfd/elf64-ppc.c:
     *     ppc_build_one_stub()
     *     build_plt_stub()
     *     build_tls_get_addr_stub()
     *
     * We could be clever and create some sort of state machine that will
     * determine the correct signature by only reading each instruction
     * once.  However, I assume this will also make the code harder to
     * maintain, and so I've gone the dumb route.  We can re-code this
     * section if it's determined to be a performance bottleneck.
     *
     * Add stub signatures as we see more.
     */

    // ----------------------------------------------
    // ppc_stub_plt_call:
    //

    // binutils >= 2.18 PLT stub signatures look like this:
    // if (PPC_HA (off) != 0)
    //   ADDIS_R12_R2 | PPC_HA (off)
    //   STD_R2_40R1
    //   LD_R11_0R12  | PPC_LO (off)
    //   ADDI_R12_R12 | PPC_LO (off) if (PPC_HA (off + 16) != PPC_HA (off))
    //   MTCTR_R11
    //   LD_R2_0R12   | PPC_LO (off + 8)
    //   LD_R11_0R12  | PPC_LO (off + 16)
    //   BCTR
    // else
    //   STD_R2_40R1
    //   LD_R11_0R2   | PPC_LO (off)
    //   ADDI_R2_R2   | PPC_LO (off)
    //   MTCTR_R11
    //   LD_R11_0R2   | PPC_LO (off + 16)
    //   LD_R2_0R2    | PPC_LO (off + 8)
    //   BCTR
    // endif
    //
    // This results in three possible stubs:

    if (   (insn[0].asInt() & 0xffff0000) == ADDIS_R12_R2
        &&  insn[1].asInt()               == STD_R2_40R1
        && (insn[2].asInt() & 0xffff0000) == LD_R11_0R12
        && (insn[2].asInt() & 0xffff0000) == ADDI_R12_R12
        &&  insn[4].asInt()               == MTCTR_R11
        && (insn[3].asInt() & 0xffff0000) == LD_R2_0R12
        && (insn[5].asInt() & 0xffff0000) == LD_R11_0R12
        &&  insn[6].asInt()               == BCTR)
    {
        off = (DFORM_SI(insn[0]) << 16) + DFORM_SI(insn[2]);
        return STUB_PLT_CALL;
    }

    if (   (insn[0].asInt() & 0xffff0000) == ADDIS_R12_R2
        &&  insn[1].asInt()               == STD_R2_40R1
        && (insn[2].asInt() & 0xffff0000) == LD_R11_0R12
        &&  insn[4].asInt()               == MTCTR_R11
        && (insn[3].asInt() & 0xffff0000) == LD_R2_0R12
        && (insn[5].asInt() & 0xffff0000) == LD_R11_0R12
        &&  insn[6].asInt()               == BCTR)
    {
        off = (DFORM_SI(insn[0]) << 16) + DFORM_SI(insn[2]);
        return STUB_PLT_CALL;
    }
    
    if (    insn[1].asInt()               == STD_R2_40R1
        && (insn[2].asInt() & 0xffff0000) == LD_R11_0R2
        && (insn[2].asInt() & 0xffff0000) == ADDI_R2_R2
        &&  insn[4].asInt()               == MTCTR_R11
        && (insn[3].asInt() & 0xffff0000) == LD_R11_0R12
        && (insn[5].asInt() & 0xffff0000) == LD_R2_0R12
        &&  insn[6].asInt()               == BCTR)
    {
        off = DFORM_SI(insn[1]);
        return STUB_PLT_CALL;
    }

    // binutils from 1.15 -> 2.18 PLT stub signatures look like this:
    // ADDIS_R12_R2  | PPC_HA (off)
    // STD_R2_40R1
    // LD_R11_0R12   | PPC_LO (off)
    // ADDIS_R12_R12 | 1            if (PPC_HA (off + 8) != PPC_HA (off))
    // LD_R2_0R12    | PPC_LO (off)
    // ADDIS_R12_R12 | 1            if (PPC_HA (off + 16) != PPC_HA (off))
    // MTCTR_R11
    // LD_R11_0R12   | PPC_LO (off)
    // BCTR
    //
    // This results in three possible stubs:

    if (   (insn[0].asInt() & 0xffff0000) ==  ADDIS_R12_R2
        &&  insn[1].asInt()               ==  STD_R2_40R1
        && (insn[2].asInt() & 0xffff0000) ==  LD_R11_0R12
        && (insn[3].asInt() & 0xffff0000) ==  LD_R2_0R12
        &&  insn[4].asInt()               ==  MTCTR_R11
        && (insn[5].asInt() & 0xffff0000) ==  LD_R11_0R12
        &&  insn[6].asInt()               ==  BCTR)
    {
        off = (DFORM_SI(insn[0]) << 16) + DFORM_SI(insn[2]);
        return STUB_PLT_CALL;
    }

    if (   (insn[0].asInt() & 0xffff0000) ==  ADDIS_R12_R2
        &&  insn[1].asInt()               ==  STD_R2_40R1
        && (insn[2].asInt() & 0xffff0000) ==  LD_R11_0R12
        && (insn[3].asInt() & 0xffff0000) ==  LD_R2_0R12
        &&  insn[4].asInt()               == (ADDIS_R12_R12 | 1)
        &&  insn[5].asInt()               ==  MTCTR_R11
        && (insn[6].asInt() & 0xffff0000) ==  LD_R11_0R12
        &&  insn[7].asInt()               ==  BCTR)
    {
        off = (DFORM_SI(insn[0]) << 16) + DFORM_SI(insn[2]);
        return STUB_PLT_CALL;
    }

    if (   (insn[0].asInt() & 0xffff0000) ==  ADDIS_R12_R2
        &&  insn[1].asInt()               ==  STD_R2_40R1
        && (insn[2].asInt() & 0xffff0000) ==  LD_R11_0R12
        &&  insn[3].asInt()               == (ADDIS_R12_R12 | 1)
        && (insn[4].asInt() & 0xffff0000) ==  LD_R2_0R12
        &&  insn[5].asInt()               == (ADDIS_R12_R12 | 1)
        &&  insn[6].asInt()               ==  MTCTR_R11
        && (insn[7].asInt() & 0xffff0000) ==  LD_R11_0R12
        &&  insn[8].asInt()               ==  BCTR)
    {
        off = (DFORM_SI(insn[0]) << 16) + DFORM_SI(insn[2]);
        return STUB_PLT_CALL;
    }

    // binutils < 1.15 PLT stub signatures look like this:
    // LD_R2_40R1                   if (glink)
    // ADDIS_R12_R2  | PPC_HA (off)
    // STD_R2_40R1                  if (!glink)
    // LD_R11_0R12   | PPC_LO (off)
    // ADDIS_R12_R12 | 1            if (PPC_HA (off + 8) != PPC_HA (off))
    // LD_R2_0R12    | PPC_LO (off)
    // ADDIS_R12_R12 | 1            if (PPC_HA (off + 16) != PPC_HA (off))
    // MTCTR_R11
    // LD_R11_0R12   | PPC_LO (off)
    // BCTR
    //
    // The non-glink case is identical to the cases above, so we need only
    // handle the three glink cases:

    /* Ugg.  The toc register is pulled off the stack for these cases.
       This is most likely the toc for the callee, but we don't know
       who the callee is yet.
    */

    if (    insn[0].asInt()               ==  LD_R2_40R1
        && (insn[1].asInt() & 0xffff0000) ==  ADDIS_R12_R2
        && (insn[2].asInt() & 0xffff0000) ==  LD_R11_0R12
        && (insn[3].asInt() & 0xffff0000) ==  LD_R2_0R12
        &&  insn[4].asInt()               ==  MTCTR_R11
        && (insn[5].asInt() & 0xffff0000) ==  LD_R11_0R12
        &&  insn[6].asInt()               ==  BCTR)
    {
        fprintf(stderr, "WARNING: Pre-binutils 1.15 linker detected. PLT call stubs may not be handled properly.\n");
        return STUB_UNKNOWN;
        //off = (DFORM_SI(insn[0]) << 16) + DFORM_SI(insn[2]);
        //return STUB_PLT_CALL;
    }

    if (    insn[0].asInt()               ==  LD_R2_40R1
        && (insn[1].asInt() & 0xffff0000) ==  ADDIS_R12_R2
        && (insn[2].asInt() & 0xffff0000) ==  LD_R11_0R12
        && (insn[3].asInt() & 0xffff0000) ==  LD_R2_0R12
        &&  insn[4].asInt()               == (ADDIS_R12_R12 | 1)
        &&  insn[5].asInt()               ==  MTCTR_R11
        && (insn[6].asInt() & 0xffff0000) ==  LD_R11_0R12
        &&  insn[7].asInt()               ==  BCTR)
    {
        fprintf(stderr, "WARNING: Pre-binutils 1.15 linker detected. PLT call stubs may not be handled properly.\n");
        return STUB_UNKNOWN;
        //off = (DFORM_SI(insn[0]) << 16) + DFORM_SI(insn[2]);
        //return STUB_PLT_CALL;
    }

    if (    insn[0].asInt()               ==  LD_R2_40R1
        && (insn[1].asInt() & 0xffff0000) ==  ADDIS_R12_R2
        && (insn[2].asInt() & 0xffff0000) ==  LD_R11_0R12
        &&  insn[3].asInt()               == (ADDIS_R12_R12 | 1)
        && (insn[4].asInt() & 0xffff0000) ==  LD_R2_0R12
        &&  insn[5].asInt()               == (ADDIS_R12_R12 | 1)
        &&  insn[6].asInt()               ==  MTCTR_R11
        && (insn[7].asInt() & 0xffff0000) ==  LD_R11_0R12
        &&  insn[8].asInt()               ==  BCTR)
    {
        fprintf(stderr, "WARNING: Pre-binutils 1.15 linker detected. PLT call stubs may not be handled properly.\n");
        return STUB_UNKNOWN;
        //off = (DFORM_SI(insn[0]) << 16) + DFORM_SI(insn[2]);
        //return STUB_PLT_CALL;
    }

    // ----------------------------------------------
    // ppc_stub_long_branch:
    // ppc_stub_long_branch_r2off:
    if (   (insn[0].asInt() & 0xfc000000) == B_UNCOND)
    {
        off = IFORM_LI(insn[0]) << 2;
        return STUB_LONG_BRANCH;
    }

    if (    insn[0].asInt()               == STD_R2_40R1
        && (insn[1].asInt() & 0xffff0000) == ADDIS_R2_R2
        && (insn[2].asInt() & 0xffff0000) == ADDI_R2_R2
        && (insn[3].asInt() & 0xfc000003) == B_UNCOND)
    {
        off = (3 * 4) + (IFORM_LI(insn[3]) << 2);
        return STUB_LONG_BRANCH;
    }

    if (    insn[0].asInt()               == STD_R2_40R1
        && (insn[1].asInt() & 0xffff0000) == ADDI_R2_R2
        && (insn[2].asInt() & 0xfc000003) == B_UNCOND)
    {
        off = (2 * 4) + (IFORM_LI(insn[2]) << 2);
        return STUB_LONG_BRANCH;
    }

    // ----------------------------------------------
    // ppc_stub_plt_branch:
    //
    if (   (insn[0].asInt() & 0xffff0000) == ADDIS_R12_R2
        && (insn[1].asInt() & 0xffff0000) == LD_R11_0R12
        &&  insn[2].asInt()               == MTCTR_R11
        &&  insn[3].asInt()               == BCTR)
    {
        off = (DFORM_SI(insn[0]) << 16) + DFORM_SI(insn[1]);
        return STUB_TOC_BRANCH;
    }

    if (   (insn[0].asInt() & 0xffff0000) == LD_R11_0R2
        &&  insn[1].asInt()               == MTCTR_R11
        &&  insn[2].asInt()               == BCTR)
    {
        off = DFORM_SI(insn[0]);
        return STUB_TOC_BRANCH;
    }

    // ----------------------------------------------
    // ppc_stub_plt_branch_r2off:
    //

    // With offset > 16 bits && r2offset > 16 bits
    if (    insn[0].asInt()               == STD_R2_40R1
        && (insn[1].asInt() & 0xffff0000) == ADDIS_R12_R2
        && (insn[2].asInt() & 0xffff0000) == LD_R11_0R12
        && (insn[3].asInt() & 0xffff0000) == ADDIS_R2_R2
        && (insn[4].asInt() & 0xffff0000) == ADDI_R2_R2
        &&  insn[5].asInt()               == MTCTR_R11
        &&  insn[6].asInt()               == BCTR)
    {
        off = (DFORM_SI(insn[1]) << 16) + DFORM_SI(insn[2]);
        return STUB_TOC_BRANCH;
    }

    // With offset > 16 bits && r2offset <= 16 bits
    if (    insn[0].asInt()               == STD_R2_40R1
        && (insn[1].asInt() & 0xffff0000) == ADDIS_R12_R2
        && (insn[2].asInt() & 0xffff0000) == LD_R11_0R12
        && (insn[3].asInt() & 0xffff0000) == ADDI_R2_R2
        &&  insn[4].asInt()               == MTCTR_R11
        &&  insn[5].asInt()               == BCTR)
    {
        off = (DFORM_SI(insn[1]) << 16) + DFORM_SI(insn[2]);
        return STUB_TOC_BRANCH;
    }

    // With offset <= 16 bits && r2offset > 16 bits
    if (    insn[0].asInt()               == STD_R2_40R1
        && (insn[1].asInt() & 0xffff0000) == LD_R11_0R2
        && (insn[2].asInt() & 0xffff0000) == ADDIS_R2_R2
        && (insn[3].asInt() & 0xffff0000) == ADDI_R2_R2
        &&  insn[4].asInt()               == MTCTR_R11
        &&  insn[5].asInt()               == BCTR)
    {
        off = DFORM_SI(insn[1]);
        return STUB_TOC_BRANCH;
    }

    // With offset <= 16 bits && r2offset <= 16 bits
    if (    insn[0].asInt()               == STD_R2_40R1
        && (insn[1].asInt() & 0xffff0000) == LD_R11_0R2
        && (insn[2].asInt() & 0xffff0000) == ADDI_R2_R2
        &&  insn[3].asInt()               == MTCTR_R11
        &&  insn[4].asInt()               == BCTR)
    {
        off = DFORM_SI(insn[1]);
        return STUB_TOC_BRANCH;
    }
#endif

    off = 0;
    return STUB_UNKNOWN;
}

bool IA_IAPI::isLinkerStub() const
{
  // Disabling this code because it ends with an
  // incorrect CFG. 

  return false;

    if (validLinkerStubState)
        return cachedLinkerStubState;

    if (!validCFT)
        return false;

    if (!isCall()) {
        cachedLinkerStubState = false;
        validLinkerStubState = true;
        return cachedLinkerStubState;
    }

    if (!cachedCFT.first) return false; 

    void *insn_buf = _isrc->getPtrToInstruction(cachedCFT.second);
    if (!insn_buf)
        return false;

    Offset off;
    linker_stub_t stub_type = checkLinkerStub(insn_buf, off);

    switch (stub_type) {
      case STUB_UNKNOWN:
        // It's not a linker stub (that we know of).  Allow processing to
        // continue unmodified, probably leading to the eventual creation
        // of a targXXXXX function.
        break;

      case STUB_LONG_BRANCH:
        cachedCFT.second += off;
        break;

      case STUB_TOC_BRANCH:
        cachedCFT.second += off;
        assert(0 && "STUB_TOC_BRANCH not implemented yet.");

        // Although tempting, we cannot just read the word directly from the
        // mutatee, and find the symbol that matches.  There may be no
        // child process to read from.
        //
        // In theory, we can use the relocations to determine the final
        // address/symbol.  But, I can't get binutils to actually generate
        // this kind of stub.  Let's deal with this once we find a binary
        // that uses it.
        break;

      case STUB_PLT_CALL:
        cachedCFT.second = _obj->cs()->getTOC(current) + off;
        break;
    }

    cachedLinkerStubState = (stub_type != STUB_UNKNOWN);
    validLinkerStubState = true;

    return cachedLinkerStubState;
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
IA_IAPI::tampersStack(ParseAPI::Function *, Address &) const
{
    return TAMPER_NONE;
}
#endif

bool IA_IAPI::isNopJump() const
{
    return false;
}
