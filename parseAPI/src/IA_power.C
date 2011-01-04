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

#include "Register.h"
#include "Dereference.h"
#include "Immediate.h"
#include "BinaryFunction.h"

#include "common/h/arch.h"

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

bool IA_IAPI::isTailCall(Function*,unsigned int) const
{
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


bool IA_IAPI::isReturnAddrSave() const
{
    // FIXME it seems as though isReturnAddrSave won't work for PPC64
    static RegisterAST::Ptr ppc_theLR(new RegisterAST(ppc32::lr));
    static RegisterAST::Ptr ppc_stackPtr(new RegisterAST(ppc32::r1));
    static RegisterAST::Ptr ppc_gpr0(new RegisterAST(ppc32::r0));

    bool foundMFLR = false;   
    bool ret = false;
    Instruction::Ptr ci = curInsn();
    if(ci->getOperation().getID() == power_op_mfspr &&
       ci->isRead(ppc_theLR) &&
       ci->isWritten(ppc_gpr0))
    {
        foundMFLR = true;
    }

    if(!foundMFLR)
        return false;

    // walk to first control flow transfer instruction, looking
    // for a save of gpr0
    int cnt = 1;
    IA_IAPI copy(dec,getAddr(),_obj,_cr,_isrc);
    while(!copy.hasCFT() && copy.curInsn()) {
        ci = copy.curInsn();
        if(ci->writesMemory() &&
           ci->isRead(ppc_stackPtr) &&
           ci->isRead(ppc_gpr0))
        {
            ret = true;
            break;
        }
        copy.advance();
        ++cnt;
    }
    parsing_printf("[%s:%d] isReturnAddrSave examined %d instructions\n",   
        FILE__,__LINE__,cnt);
    return ret;
}

bool IA_IAPI::isFakeCall() const
{
    return false;
}

bool IA_IAPI::isIATcall() const
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
    if (validLinkerStubState)
        return cachedLinkerStubState;

    if (!validCFT)
        return false;

    if (!isCall()) {
        cachedLinkerStubState = false;
        validLinkerStubState = true;
        return cachedLinkerStubState;
    }

    void *insn_buf = _isrc->getPtrToInstruction(cachedCFT);
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
        cachedCFT += off;
        break;

      case STUB_TOC_BRANCH:
        cachedCFT += off;
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
        cachedCFT = _obj->cs()->getTOC(current) + off;
        break;
    }

    cachedLinkerStubState = (stub_type != STUB_UNKNOWN);
    validLinkerStubState = true;

    return cachedLinkerStubState;
}

ParseAPI::StackTamper 
IA_IAPI::tampersStack(ParseAPI::Function *, Address &) const
{
    return TAMPER_NONE;
}
