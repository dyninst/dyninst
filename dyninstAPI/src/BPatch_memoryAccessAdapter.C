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

#include "BPatch_memoryAccessAdapter.h"
#include "BPatch_memoryAccess_NP.h"
#include "Instruction.h"
#include "Immediate.h"
#include "Register.h"
#include "Dereference.h"

#include "common/src/arch.h"

#include "legacy-instruction.h"

using namespace Dyninst;
using namespace InstructionAPI;


BPatch_memoryAccess* BPatch_memoryAccessAdapter::convert(Instruction insn,
							 Dyninst::Address current, bool is64)
{
#if defined(arch_x86) || defined(arch_x86_64)
    static unsigned int log2[] = { 0, 0, 1, 1, 2, 2, 2, 2, 3 };
    
  // TODO 16-bit registers
    
  int nac = 0;
    
  ia32_memacc mac_[3];
  ia32_condition cnd;
  ia32_instruction i(mac_, &cnd);
    
  const unsigned char* addr = reinterpret_cast<const unsigned char*>(insn.ptr());
  BPatch_memoryAccess* bmap = BPatch_memoryAccess::none;


    ia32_decode(IA32_DECODE_MEMACCESS | IA32_DECODE_CONDITION, addr, i, is64);
  
  bool first = true;

  for(int j=0; j<3; ++j) {
    ia32_memacc& mac = const_cast<ia32_memacc&>(i.getMac(j));
    const ia32_condition& cond = i.getCond();
    int bmapcond = cond.is ? cond.tttn : -1;
    if(mac.is) {

      // here, we can set the correct address for RIP-relative addressing
      //
      // Xiaozhu: This works for dynamic instrumentation in all cases,
      // and binary rewriting on non-pic code. This will break for rewriting
      // PIE and shared libraries
      if (mac.regs[0] == mRIP) {
	mac.imm = current + insn.size() + mac.imm;
      }

      if(first) {
        if(mac.prefetch) {
          if(mac.prefetchlvl > 0) // Intel
            bmap = new BPatch_memoryAccess(new internal_instruction(NULL), current,
					   false, false,
                                           mac.imm, mac.regs[0], mac.regs[1], mac.scale,
                                           0, -1, -1, 0,
                                           bmapcond, false, mac.prefetchlvl);
          else // AMD
	    bmap = new BPatch_memoryAccess(new internal_instruction(NULL), current,
					   false, false,
                                           mac.imm, mac.regs[0], mac.regs[1], mac.scale,
                                           0, -1, -1, 0,
                                           bmapcond, false, mac.prefetchstt + IA32AMDprefetch);
        }
        else switch(mac.sizehack) { // translation to pseudoregisters
        case 0:
	  bmap = new BPatch_memoryAccess(new internal_instruction(NULL), current,
					 mac.read, mac.write,
                                         mac.size, mac.imm, mac.regs[0], mac.regs[1], mac.scale, 
                                         bmapcond, mac.nt);
          break;
        case shREP: // use ECX register to compute final size as mac.size * ECX
	  bmap = new BPatch_memoryAccess(new internal_instruction(NULL), current,
                                         mac.read, mac.write,
                                         mac.imm, mac.regs[0], mac.regs[1], mac.scale,
                                         0, -1, 1 , log2[mac.size],
                                         bmapcond, false);
          break;
        case shREPESCAS:
	  bmap = new BPatch_memoryAccess(new internal_instruction(NULL), current,
					 mac.read, mac.write,
                                         mac.imm, mac.regs[0], mac.regs[1], mac.scale,
                                         0, -1, IA32_ESCAS, log2[mac.size],
                                         bmapcond, false);
          break;
        case shREPNESCAS:
	  bmap = new BPatch_memoryAccess(new internal_instruction(NULL), current,
					 mac.read, mac.write,
                                         mac.imm, mac.regs[0], mac.regs[1], mac.scale,
                                         0, -1, IA32_NESCAS, log2[mac.size],
                                         bmapcond, false);
          break;
        case shREPECMPS:
	  bmap = new BPatch_memoryAccess(new internal_instruction(NULL), current,
					 mac.read, mac.write,
                                         mac.imm, mac.regs[0], mac.regs[1], mac.scale,
                                         0, -1, IA32_ECMPS, log2[mac.size],
                                         bmapcond, false);
          break;
        case shREPNECMPS:
	  bmap = new BPatch_memoryAccess(new internal_instruction(NULL), current,
					 mac.read, mac.write,
                                         mac.imm, mac.regs[0], mac.regs[1], mac.scale,
                                         0, -1, IA32_NECMPS, log2[mac.size],
                                         bmapcond, false);
          break;
        default:
          assert(!"Unknown size hack");
        }
        first = false;
      }
      else
        switch(mac.sizehack) { // translation to pseudoregisters
        case 0:
          bmap->set2nd(mac.read, mac.write, mac.size, mac.imm,
                       mac.regs[0], mac.regs[1], mac.scale);
          break;
        case shREP: // use ECX register to compute final size as mac.size * ECX
          bmap->set2nd(mac.read, mac.write,
                       mac.imm, mac.regs[0], mac.regs[1], mac.scale,
		       0, -1, 1 , log2[mac.size],
                       bmapcond, false);
          break;
        case shREPESCAS:
        case shREPNESCAS:
          assert(!"Cannot happen");
          break;
        case shREPECMPS:
          bmap->set2nd(mac.read, mac.write,
                       mac.imm, mac.regs[0], mac.regs[1], mac.scale,
                       0, -1, IA32_ECMPS, log2[mac.size],
                       bmapcond, false);
          break;
        case shREPNECMPS:
          //fprintf(stderr, "In set2nd[shREPNECMPS]!!!\n");
          bmap->set2nd(mac.read, mac.write,
                       mac.imm, mac.regs[0], mac.regs[1], mac.scale,
                       0, -1, IA32_NECMPS, log2[mac.size],
                       bmapcond, false);
          break;
        default:
          assert(!"Unknown size hack");
        }
      ++nac;
    }
  }
  assert(nac < 3);
  return bmap;
#elif defined arch_power
    std::vector<Operand> operands;
    insn.getOperands(operands);
    for(std::vector<Operand>::iterator op = operands.begin();
        op != operands.end();
       ++op)
    {
        bool isLoad = op->readsMemory();
        bool isStore = op->writesMemory();
        if(isLoad || isStore)
        {
			op->getValue()->apply(this);
            if(insn.getOperation().getID() == power_op_lmw ||
               insn.getOperation().getID() == power_op_stmw)
            {
                RegisterAST::Ptr byteOverride =
                        boost::dynamic_pointer_cast<RegisterAST>(insn.getOperand(0).getValue());
                assert(byteOverride);
                MachRegister base = byteOverride->getID().getBaseRegister();
                unsigned int converted = base.val() & 0xFFFF;
                bytes = (32 - converted) << 2;
            }
            if(insn.getOperation().getID() == power_op_lswi ||
               insn.getOperation().getID() == power_op_stswi)
            {
                Immediate::Ptr byteOverride =
                        boost::dynamic_pointer_cast<Immediate>(insn.getOperand(2).getValue());
                assert(byteOverride);
                bytes = byteOverride->eval().convert<unsigned int>();
                if(bytes == 0) bytes = 32;
            }
            if(insn.getOperation().getID() == power_op_lswx ||
               insn.getOperation().getID() == power_op_stswx)
            {
                return new BPatch_memoryAccess(new internal_instruction(NULL), current, isLoad, isStore, (long)0, ra, rb, (long)0, 9999, -1);
            }
			return new BPatch_memoryAccess(new internal_instruction(NULL), current, isLoad, isStore,
                                       bytes, imm, ra, rb);
        }
    }
    return NULL;
#elif defined(arch_aarch64) 
    std::vector<Operand> operands;
    insn.getOperands(operands);
    for(std::vector<Operand>::iterator op = operands.begin();
        op != operands.end();
       ++op)
    {
        bool isLoad = op->readsMemory();
        bool isStore = op->writesMemory();
        if(isLoad || isStore)
        {
		
			op->getValue()->apply(this);
      // Xiaozhu: This works for dynamic instrumentation in all cases,
      // and binary rewriting on non-pic code. This will break for rewriting
      // PIE and shared libraries
			if (ra == 32) {
			    imm = imm + current;
			}
			//fprintf(stderr, "instruction: %s, operand %s\n", insn.format().c_str(),op->getValue()->format().c_str());
			//fprintf(stderr, "imm: %d, ra: %d, rb: %d, scale: %d\n", imm, ra, rb, sc);

			return new BPatch_memoryAccess(new internal_instruction(NULL), current, isLoad, isStore,
                                       bytes, imm, ra, rb, sc);
        }
    }
	return NULL;
#else 
    assert(!"Unimplemented architecture");
#endif
    // Silence compiler warnings
    (void)insn;
    (void)current;
    (void)is64;
}


void BPatch_memoryAccessAdapter::visit(Dereference* d)
{
    bytes = d->size();
}

void BPatch_memoryAccessAdapter::visit(RegisterAST* r)
{
    MachRegister base = r->getID().getBaseRegister();
    //fprintf(stderr, "base: %d\n", base.val());
	    
	unsigned int converted = base.val() & 0xFFFF;
	#if defined arch_power
    if((ra == -1) && !setImm) {
        ra = converted;
        return;
    } else if(rb == -1) {
        rb = converted;
        if(ra == 0) ra = -1;
        return;
    }
    else
    {
        fprintf(stderr, "ASSERT: only two registers used in a power load/store calc!\n");
        assert(0);
    }
	#else
	if(ra == -1) {
		ra = converted;
		return;
	}
	else if(rb == -1) {
		rb = converted;
		return;
	}
    else
    {
        fprintf(stderr, "ASSERT: only two registers used in a power load/store calc!\n");
        assert(0);
    }
	#endif        
}

void BPatch_memoryAccessAdapter::visit(Immediate* i)
{
    imm = i->eval().convert<long>();
	setImm = true;
}

void BPatch_memoryAccessAdapter::visit(BinaryFunction* b)
{	
	if(b->isLeftShift() == true) {
		sc = imm;
		imm = 0;
	}
}

