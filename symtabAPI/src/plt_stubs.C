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

#include "plt_stubs.h"

#include "Dereference.h"
#include "Expression.h"
#include "Instruction.h"
#include "InstructionDecoder.h"
#include "Operand.h"
#include "Register.h"
#include "Result.h"
#include "entryIDs.h"
#include "registers/MachRegister.h"

#include <vector>

namespace ia = Dyninst::InstructionAPI;

namespace Dyninst {
namespace SymtabAPI {

namespace {

// The value of e with reg set to value, or ADDRESS_INVALID.
Address eval_with(ia::Expression::Ptr e, ia::Expression &reg, Address value)
{
    e->bind(&reg, ia::Result(ia::u64, value));
    ia::Result r{e->eval()};
    if (!r.defined)  {
        return ADDRESS_INVALID;
    }
    return r.convert<Address>();
}

Address aarch64_got_slot(const unsigned char *buf, std::size_t len, Address addr)
{
    ia::InstructionDecoder dec{buf, len, Arch_aarch64};
    ia::Instruction adrp{dec.decode()};
    if (!adrp.isValid() || adrp.getOperation().getID() != aarch64_op_adrp)  {
        return ADDRESS_INVALID;
    }
    ia::Instruction ldr{dec.decode()};
    if (!ldr.isValid() || ldr.getOperation().getID() != aarch64_op_ldr_imm_gen)  {
        return ADDRESS_INVALID;
    }

    std::vector<ia::Operand> adrp_ops{adrp.getExplicitOperands()};
    std::vector<ia::Operand> ldr_ops{ldr.getExplicitOperands()};
    if (adrp_ops.size() != 2 || ldr_ops.size() != 2)  {
        return ADDRESS_INVALID;
    }
    auto base = boost::dynamic_pointer_cast<ia::RegisterAST>(adrp_ops[0].getValue());
    auto slot = boost::dynamic_pointer_cast<ia::Dereference>(ldr_ops[1].getValue());
    if (!base || !slot)  {
        return ADDRESS_INVALID;
    }

    // adrp gives the page of pc plus its immediate.  InstructionAPI leaves out
    // the rounding down to the page, so give it the page as the pc.
    ia::RegisterAST pc{MachRegister::getPC(Arch_aarch64)};
    Address page{eval_with(adrp_ops[1].getValue(), pc, addr & ~Address{0xfff})};
    if (page == ADDRESS_INVALID)  {
        return ADDRESS_INVALID;
    }
    // The ldr must load through the register adrp set; if it does not, the
    // address stays undefined.
    return eval_with(slot->getSubexpressions().front(), *base, page);
}

}

Address plt_stub_got_slot(const unsigned char *buf, std::size_t len, Address addr,
                          Architecture arch)
{
    if (arch == Arch_aarch64)  {
        return aarch64_got_slot(buf, len, addr);
    }
    return ADDRESS_INVALID;
}

}
}
