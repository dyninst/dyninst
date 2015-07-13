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
#include "IA_aarch64.h"

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

//#warning "The reg defines are not correct now!"
static RegisterAST::Ptr aarch64_R11 (new RegisterAST (aarch64::x11));
static RegisterAST::Ptr aarch64_LR  (new RegisterAST (aarch64::x30));
//SP is an independent reg in aarch64
static RegisterAST::Ptr aarch64_SP  (new RegisterAST (aarch64::x0));

bool IA_IAPI::isFrameSetupInsn(Instruction::Ptr) const
{
	assert(0);
    return false;
}

bool IA_IAPI::isNop() const
{
	assert(0);
    return false;
}

bool IA_IAPI::isThunk() const {
	assert(0);
    return false;
}

bool IA_IAPI::isTailCall(Function* context, EdgeTypeEnum type, unsigned int) const
{
	assert(0);
    return false;
}

bool IA_IAPI::savesFP() const
{
	assert(0);
    return false;
}

bool IA_IAPI::isStackFramePreamble() const
{
	assert(0);
    return false;
}

bool IA_IAPI::cleansStack() const
{
	assert(0);
    return false;
}

class AARCH64ReturnPredicates : public Slicer::Predicates {
  virtual bool widenAtPoint(Assignment::Ptr p) {
		return true;
    }
};



bool IA_IAPI::sliceReturn(ParseAPI::Block* bit, Address ret_addr, ParseAPI::Function * func) const {
	assert(0);
	return 0;
}

bool IA_IAPI::isReturnAddrSave(Address& retAddr) const
{
	assert(0);
  return 0;
}

bool IA_IAPI::isReturn(Dyninst::ParseAPI::Function * context, Dyninst::ParseAPI::Block* currBlk) const
{
	assert(0);
	return 0;
}

bool IA_IAPI::isFakeCall() const
{
	assert(0);
    return false;
}

bool IA_IAPI::isIATcall(std::string &) const
{
	assert(0);
    return false;
}

const unsigned int B_UNCOND      = 0x0;
const unsigned int ADDIS_R12_R12 = 0x0;
const unsigned int ADDIS_R12_R2  = 0x0;
const unsigned int ADDIS_R2_R2   = 0x0;
const unsigned int ADDI_R12_R12  = 0x0;
const unsigned int ADDI_R2_R2    = 0x0;
const unsigned int STD_R2_40R1   = 0x0;
const unsigned int LD_R2_40R1    = 0x0;
const unsigned int LD_R2_0R2     = 0x0;
const unsigned int LD_R2_0R12    = 0x0;
const unsigned int LD_R11_0R12   = 0x0;
const unsigned int LD_R11_0R2    = 0x0;
const unsigned int MTCTR_R11     = 0x0;
const unsigned int BCTR          = 0x0;

typedef enum {
    STUB_UNKNOWN,
    STUB_LONG_BRANCH,
    STUB_TOC_BRANCH,
    STUB_PLT_CALL
} linker_stub_t;

linker_stub_t checkLinkerStub(void *insn_buf, Offset &off)
{
	assert(0);
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
    // ADDIS_R12_R2  | AARCH64_HA (off)
    // STD_R2_40R1
    // LD_R11_0R12   | AARCH64_LO (off)
    // ADDIS_R12_R12 | 1            if (AARCH64_HA (off + 8) != AARCH64_HA (off))
    // LD_R2_0R12    | AARCH64_LO (off)
    // ADDIS_R12_R12 | 1            if (AARCH64_HA (off + 16) != AARCH64_HA (off))
    // MTCTR_R11
    // LD_R11_0R12   | AARCH64_LO (off)
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
    // ADDIS_R12_R2  | AARCH64_HA (off)
    // STD_R2_40R1                  if (!glink)
    // LD_R11_0R12   | AARCH64_LO (off)
    // ADDIS_R12_R12 | 1            if (AARCH64_HA (off + 8) != AARCH64_HA (off))
    // LD_R2_0R12    | AARCH64_LO (off)
    // ADDIS_R12_R12 | 1            if (AARCH64_HA (off + 16) != AARCH64_HA (off))
    // MTCTR_R11
    // LD_R11_0R12   | AARCH64_LO (off)
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
}

AST::Ptr AARCH64_BLR_Visitor::visit(AST *a) {
	assert(0);
  return a->ptr();
}

AST::Ptr AARCH64_BLR_Visitor::visit(DataflowAPI::BottomAST *b) {
	assert(0);
  return_ = AARCH64_BLR_UNKNOWN;
  return b->ptr();
}

AST::Ptr AARCH64_BLR_Visitor::visit(DataflowAPI::ConstantAST *c) {
	assert(0);
  // Very odd case, but claiming not a return
  return_ = AARCH64_BLR_NOTRETURN;
  return c->ptr();
}

AST::Ptr AARCH64_BLR_Visitor::visit(DataflowAPI::VariableAST *v) {
	assert(0);
  return v->ptr();
}
/*
AST::Ptr AARCH64_BLR_Visitor::visit(StackAST *s) {
  return_ = UNKNOWN;
  return s->Ptr();
}
*/
AST::Ptr AARCH64_BLR_Visitor::visit(DataflowAPI::RoseAST *r) {
	assert(0);
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
	assert(0);
    return false;
}
