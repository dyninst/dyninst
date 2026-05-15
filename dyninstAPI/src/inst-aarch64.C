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

#include "common/src/headers.h"
#include "codegen/RegControl.h"
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
#include "dyninstAPI/src/image.h"
#include "dyninstAPI/src/dynProcess.h"
#include "dyninstAPI/src/inst-aarch64.h"
#include "common/src/arch-aarch64.h"
#include "dyninstAPI/src/codegen.h"
#include "dyninstAPI/src/util.h"
#include "common/src/stats.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/instPoint.h" // class instPoint
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/h/BPatch.h"
#include "BPatch_collections.h"
#include "registerSpace.h"
#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/mapped_object.h"
#include "RegisterConversion.h"
#include "parseAPI/h/CFG.h"
#include "codegen/emitters/aarch64/EmitterAarch64Dyn.h"
#include "codegen/emitters/aarch64/EmitterAarch64Stat.h"
#include "Instruction.h"
#include "Register.h"
#include "registers/aarch64_regs.h"

#include "emitter.h"
#include "emit-aarch64.h"

#include <boost/assign/list_of.hpp>
using namespace boost::assign;
#include <sstream>

#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"

using codeGenASTPtr = Dyninst::DyninstAPI::codeGenASTPtr;
using operandAST = Dyninst::DyninstAPI::operandAST;

void emitLoadPreviousStackFrameRegister(Address register_num,
                                        Register dest,
                                        codeGen &gen,
                                        int /*size*/)
{
    gen.codeEmitter()->emitLoadOrigRegister(register_num, dest, gen);
}

// First AST node: target of the call
// Second AST node: source of the call
// This can handle indirect control transfers as well
bool AddressSpace::getDynamicCallSiteArgs(InstructionAPI::Instruction i,
					  Address addr,
					  std::vector<codeGenASTPtr> &args)
{
    namespace di = Dyninst::InstructionAPI;

    auto cft = i.getControlFlowTarget();
    auto target_reg = boost::dynamic_pointer_cast<di::RegisterAST>(cft);
    if(!target_reg) return false;
    auto branch_target = convertRegID(target_reg);

    if(branch_target == registerSpace::ignored) return false;

    //jumping to Xn (BLR Xn)
    args.push_back(operandAST::origRegister((void *)(long)branch_target));
    args.push_back(operandAST::Constant((void *) addr));

    return true;
}

Emitter *AddressSpace::getEmitter() {
    static Dyninst::DyninstAPI::EmitterAarch64Stat emitter64Stat;
    static Dyninst::DyninstAPI::EmitterAarch64Dyn emitter64Dyn;

    if (proc())
        return &emitter64Dyn;

    return &emitter64Stat;
}

#define GET_IP      0x429f0005
#define MFLR_30     0x7fc802a6
#define ADDIS_30_30 0x3fde0000
#define ADDI_30_30  0x3bde0000
#define LWZ_11_30   0x817e0000
#define ADDIS_11_30 0x3d7e0000
