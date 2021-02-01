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

#include "SymbolicExpansion.h"
#include "SymEvalPolicy.h"

#include "../rose/SgAsmInstruction.h"
#include "../rose/SgAsmPowerpcInstruction.h"
#include "../rose/SgAsmx86Instruction.h"
#include "../rose/SgAsmAmdgpuVegaInstruction.h"

#include "../rose/x86InstructionSemantics.h"
#include "../rose/x86_64InstructionSemantics.h"

#include "../rose/semantics/DispatcherARM64.h"
#include "../rose/semantics/DispatcherAmdgpuVega.h"
#include "../rose/semantics/DispatcherPowerpc.h"

using namespace Dyninst;
using namespace DataflowAPI;

bool SymbolicExpansion::expandX86(SgAsmInstruction *rose_insn,
                                  SymEvalPolicy &policy) {
    SgAsmx86Instruction *insn = static_cast<SgAsmx86Instruction *>(rose_insn);

    X86InstructionSemantics<SymEvalPolicy, Handle> t(policy);
    t.processInstruction(insn);
    return true;
}

bool SymbolicExpansion::expandX86_64(SgAsmInstruction *rose_insn,
                                     SymEvalPolicy_64 &policy) {
    try {
        SgAsmx86Instruction *insn = static_cast<SgAsmx86Instruction *>(rose_insn);
        X86_64InstructionSemantics<SymEvalPolicy_64, Handle> t(policy);
        t.processInstruction(insn);
    } catch (rose::BinaryAnalysis::InstructionSemantics2::BaseSemantics::Exception &e) {
        // fprintf(stderr, "Instruction processing threw exception for instruction: %s\n", insn_dump.c_str());
    }

    return true;
}

bool SymbolicExpansion::expandPPC32(SgAsmInstruction *rose_insn,
                                    BaseSemantics::RiscOperatorsPtr ops, 
				    const std::string &insn_dump) {
    SgAsmPowerpcInstruction *insn = static_cast<SgAsmPowerpcInstruction *>(rose_insn);

    BaseSemantics::DispatcherPtr cpu = DispatcherPowerpc::instance(ops, 32);

    try {
        cpu->processInstruction(insn);
    } catch (rose::BinaryAnalysis::InstructionSemantics2::BaseSemantics::Exception &e) {
        // fprintf(stderr, "Instruction processing threw exception for instruction: %s\n", insn_dump.c_str());
    }

    return true;
}
bool SymbolicExpansion::expandPPC64(SgAsmInstruction *rose_insn,
                                    BaseSemantics::RiscOperatorsPtr ops, 
				    const std::string &insn_dump) {
    SgAsmPowerpcInstruction *insn = static_cast<SgAsmPowerpcInstruction *>(rose_insn);

    BaseSemantics::DispatcherPtr cpu = DispatcherPowerpc::instance(ops, 64);

    try {
        cpu->processInstruction(insn);
    } catch (rose::BinaryAnalysis::InstructionSemantics2::BaseSemantics::Exception &e) {
//         fprintf(stderr, "Instruction processing threw exception for instruction: %s\n", insn_dump.c_str());
//	 std::cerr << e << std::endl;
    }

    return true;
}

bool SymbolicExpansion::expandAarch64(SgAsmInstruction *rose_insn, BaseSemantics::RiscOperatorsPtr ops, const std::string &insn_dump) {
    SgAsmArmv8Instruction *insn = static_cast<SgAsmArmv8Instruction *>(rose_insn);

    BaseSemantics::DispatcherPtr cpu = DispatcherARM64::instance(ops, 64);

    try {
        cpu->processInstruction(insn);
    } catch (rose::BinaryAnalysis::InstructionSemantics2::BaseSemantics::Exception &e) {
        // fprintf(stderr, "Instruction processing threw exception for instruction: %s\n", insn_dump.c_str());
    }

    return false;
}

bool SymbolicExpansion::expandAmdgpuVega(SgAsmInstruction *rose_insn, BaseSemantics::RiscOperatorsPtr ops, const std::string &insn_dump) {
    SgAsmAmdgpuVegaInstruction *insn = static_cast<SgAsmAmdgpuVegaInstruction *>(rose_insn);

    BaseSemantics::DispatcherPtr cpu = DispatcherAmdgpuVega::instance(ops, 64);

    try {
        cpu->processInstruction(insn);
    } catch (rose::BinaryAnalysis::InstructionSemantics2::BaseSemantics::Exception &e) {
        // fprintf(stderr, "Instruction processing threw exception for instruction: %s\n", insn_dump.c_str());
    }

    return false;
}


