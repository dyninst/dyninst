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

#if !defined(REGISTER_IDS_X86_H)
#define REGISTER_IDS_X86_H

#include <vector>
#include <map>
#include "util.h"
#include "Result.h"
#include "dyntypes.h"
namespace Dyninst
{
  namespace InstructionAPI
  {
    /// \enum Dyninst::InstructionAPI::IA32Regs
    /// \brief Registers for IA32 and AMD64 processors.
    ///
    // We REALLY REALLY NEED these definitions to stay aligned as given,
    // so (e.g.) (r_EAX - r_AH) == (r_EDX - r_DH). We use that later
    // for upconverting overlapping/aliased registers.
    enum IA32Regs : unsigned int { r_AH=0, r_BH, r_CH, r_DH,
		    r_AL=10, r_BL, r_CL, r_DL,
		    r_AX=20, r_BX, r_CX, r_DX, r_SI, r_DI,
		    r_eAX=30, r_eBX, r_eCX, r_eDX, r_eSI, r_eDI,
		    r_EAX=40, r_EBX, r_ECX, r_EDX, r_ESI, r_EDI,
		    r_eSP=50, r_eBP,
		    r_ESP=60, r_EBP,
		    r_EDXEAX=70, r_ECXEBX, //133
		    r_EFLAGS=100,
		    r_CS=110, r_DS, r_ES, r_FS, r_GS, r_SS,
		    r_EIP=120,
		    // flags need to be separate registers for proper liveness analysis
		    r_DummyFPR=130, r_Reserved,
		    // and we have a dummy register to make liveness consistent since floating point saves are all/none at present
		    r_rAX=140, r_rBX, r_rCX, r_rDX, r_rSI, r_rDI,
		    r_RAX=150, r_RBX, r_RCX, r_RDX, r_RSI, r_RDI,
		    r_rSP=160, r_rBP,
		    r_RSP=170, r_RBP,
		    r_R8=180, r_R9, r_R10, r_R11, r_R12, r_R13, r_R14, r_R15,
		    // AMD64 GPRs
		    r_RIP=190,
		    r_XMM0=200, r_XMM1, r_XMM2, r_XMM3, r_XMM4, r_XMM5, r_XMM6, r_XMM7,
		    r_MM0, r_MM1, r_MM2, r_MM3, r_MM4, r_MM5, r_MM6, r_MM7,
		    r_CR0, r_CR1, r_CR2, r_CR3, r_CR4, r_CR5, r_CR6, r_CR7,
		    r_DR0, r_DR1, r_DR2, r_DR3, r_DR4, r_DR5, r_DR6, r_DR7,
		    r_TR0, r_TR1, r_TR2, r_TR3, r_TR4, r_TR5, r_TR6, r_TR7,
		    r_SPL, r_BPL, r_SIL, r_DIL,
                    // FP Stack
                    r_ST0, r_ST1, r_ST2, r_ST3, r_ST4, r_ST5, r_ST6, r_ST7,
		    r_ALLGPRS,
                    r_OF=11, r_SF=7, r_ZF=6, r_AF=4, r_PF=2, r_CF=0, r_TF=8, r_IF=9, r_DF=10, r_NT=14, r_RF=16,
                    // NOTE: 11 of these!
    
    };

    /// The %RegInfo struct associates a register ID with its size and name.
    struct RegInfo
    {
      RegInfo(Result_Type t, std::string n) :
	regSize(t), regName(n) 
      {
      }
      RegInfo() :
	regSize(u8), regName("*** UNDEFINED REGISTER ***")
      {
      }
	    
      Result_Type regSize;
      std::string regName;
    };
  }
}
	  
	  
namespace Dyninst
{
  namespace InstructionAPI
  {
    typedef dyn_hash_map<IA32Regs, RegInfo> RegTable;
    /// \brief Register names for disassembly and debugging
    struct IA32RegTable
    {
      IA32RegTable() {}
      RegTable IA32_register_names;
      Result_Type getSize(int id) 
      {
	RegTable::const_iterator found = IA32_register_names.find((IA32Regs)(id));
	if(found != IA32_register_names.end()) {
	  return found->second.regSize;
	}
	// Sane default
	return u32;
      }
      
    };

  }
}



#endif //!defined(REGISTER_IDS_X86_H)
