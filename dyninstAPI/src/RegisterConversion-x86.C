/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#include "RegisterConversion-x86.h"
#include "inst-x86.h"

#include <map>
#include <boost/assign/list_of.hpp>
using namespace Dyninst::InstructionAPI;
using namespace std;
using namespace boost::assign;

map<IA32Regs, Register> reverseRegisterMap = map_list_of
        (r_EAX, REGNUM_RAX)
        (r_ECX, REGNUM_RCX)
        (r_EDX, REGNUM_RDX)
        (r_EBX, REGNUM_RBX)
        (r_ESP, REGNUM_RSP)
        (r_EBP, REGNUM_RBP)
        (r_ESI, REGNUM_RSI)
        (r_EDI, REGNUM_RDI)
        (r_R8, REGNUM_R8)
        (r_R9, REGNUM_R9)
        (r_R10, REGNUM_R10)
        (r_R11, REGNUM_R11)
        (r_R12, REGNUM_R12)
        (r_R13, REGNUM_R13)
        (r_R14, REGNUM_R14)
        (r_R15, REGNUM_R15)
        (r_DummyFPR, REGNUM_DUMMYFPR)
        (r_OF, REGNUM_OF)
        (r_SF, REGNUM_SF)
        (r_ZF, REGNUM_ZF)
        (r_AF, REGNUM_AF)
        (r_PF, REGNUM_PF)
        (r_CF, REGNUM_CF)
        (r_TF, REGNUM_TF)
        (r_IF, REGNUM_IF)
        (r_DF, REGNUM_DF)
        (r_NT, REGNUM_NT)
        (r_RF, REGNUM_RF)
        (r_AH, REGNUM_RAX)
        (r_BH, REGNUM_RBX)
        (r_CH, REGNUM_RCX)
        (r_DH, REGNUM_RDX)
        (r_AL, REGNUM_RAX)
        (r_BL, REGNUM_RBX)
        (r_CL, REGNUM_RCX)
        (r_DL, REGNUM_RDX)
        (r_SPL, REGNUM_RSP)
        (r_BPL, REGNUM_RBP)
        (r_SIL, REGNUM_RSI)
        (r_DIL, REGNUM_RDI)
        (r_eAX, REGNUM_RAX)
        (r_eBX, REGNUM_RBX)
        (r_eCX, REGNUM_RCX)
        (r_eDX, REGNUM_RDX)
        (r_AX, REGNUM_RAX)
        (r_DX, REGNUM_RDX)
        (r_eSP, REGNUM_RSP)
        (r_eBP, REGNUM_RBP)
        (r_eSI, REGNUM_RSI)
        (r_eDI, REGNUM_RDI)
    // These are wrong, need to extend to make cmpxch8b work right
        (r_EDXEAX, REGNUM_RAX)
        (r_ECXEBX, REGNUM_RCX)
        (r_CS, REGNUM_IGNORED)
        (r_DS, REGNUM_IGNORED)
        (r_ES, REGNUM_IGNORED)
        (r_FS, REGNUM_IGNORED)
        (r_GS, REGNUM_IGNORED)
        (r_SS, REGNUM_IGNORED)
        (r_rAX, REGNUM_RAX)
        (r_rCX, REGNUM_RCX)
        (r_rDX, REGNUM_RDX)
        (r_rBX, REGNUM_RBX)
        (r_rSP, REGNUM_RSP)
        (r_rBP, REGNUM_RBP)
        (r_rSI, REGNUM_RSI)
        (r_rDI, REGNUM_RDI)
        (r_EFLAGS, REGNUM_IGNORED)
        (r_EIP, REGNUM_IGNORED)
        (r_RIP, REGNUM_IGNORED)
        (r_RAX, REGNUM_RAX)
        (r_RCX, REGNUM_RCX)
        (r_RDX, REGNUM_RDX)
        (r_RBX, REGNUM_RBX)
        (r_RSP, REGNUM_RSP)
        (r_RBP, REGNUM_RBP)
        (r_RSI, REGNUM_RSI)
        (r_RDI, REGNUM_RDI)
        (r_SI, REGNUM_RSI)
        (r_DI, REGNUM_RDI)
        (r_XMM0, REGNUM_XMM0)
        (r_XMM1, REGNUM_XMM1)
        (r_XMM2, REGNUM_XMM2)
        (r_XMM3, REGNUM_XMM3)
        (r_XMM4, REGNUM_XMM4)
        (r_XMM5, REGNUM_XMM5)
        (r_XMM6, REGNUM_XMM6)
        (r_XMM7, REGNUM_XMM7)
        (r_MM0, REGNUM_MM0)
        (r_MM1, REGNUM_MM1)
        (r_MM2, REGNUM_MM2)
        (r_MM3, REGNUM_MM3)
        (r_MM4, REGNUM_MM4)
        (r_MM5, REGNUM_MM5)
        (r_MM6, REGNUM_MM6)
        (r_MM7, REGNUM_MM7)
        (r_CR0, REGNUM_IGNORED)
        (r_CR1, REGNUM_IGNORED)
        (r_CR2, REGNUM_IGNORED)
        (r_CR3, REGNUM_IGNORED)
        (r_CR4, REGNUM_IGNORED)
        (r_CR5, REGNUM_IGNORED)
        (r_CR6, REGNUM_IGNORED)
        (r_CR7, REGNUM_IGNORED)
        (r_DR0, REGNUM_IGNORED)
        (r_DR1, REGNUM_IGNORED)
        (r_DR2, REGNUM_IGNORED)
        (r_DR3, REGNUM_IGNORED)
        (r_DR4, REGNUM_IGNORED)
        (r_DR5, REGNUM_IGNORED)
        (r_DR6, REGNUM_IGNORED)
        (r_DR7, REGNUM_IGNORED)
        (r_ALLGPRS, REGNUM_IGNORED)
  (r_ST0, REGNUM_IGNORED)
  (r_ST1, REGNUM_IGNORED)
  (r_ST2, REGNUM_IGNORED)
  (r_ST3, REGNUM_IGNORED)
  (r_ST4, REGNUM_IGNORED)
  (r_ST5, REGNUM_IGNORED)
  (r_ST6, REGNUM_IGNORED)
  (r_ST7, REGNUM_IGNORED)
        ;

Register convertRegID(IA32Regs toBeConverted)
{
    map<IA32Regs, Register>::const_iterator found =
            reverseRegisterMap.find(toBeConverted);
    if(found == reverseRegisterMap.end()) {
        fprintf(stderr, "Register ID %d not found in reverseRegisterLookup!\n", toBeConverted);
        assert(!"Bad register ID");
    }
    return found->second;
}
