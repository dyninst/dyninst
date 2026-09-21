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

#ifndef CODEGEN_LOONGARCH64_H
#define CODEGEN_LOONGARCH64_H

#include "common/src/Types.h"
#include "dyninstAPI/src/codegen.h"
#include "common/h/dyn_regs.h"

class codeGen;

namespace NS_loongarch64 {
    class instruction;
}

class insnCodeGen {
public:
    enum LoadStore { Load, Store };
    enum ArithOp { Add, Sub };
    enum IndexMode { Post, Pre, Offset };

    static void generateNOOP(codeGen &gen, unsigned size = 4);
    static void generateTrap(codeGen &gen);
    static void generateIllegal(codeGen &gen);
    static void generateBranch(codeGen &gen, long jump_off, bool link = false);
    static void generateBranch(codeGen &gen, Address from, Address to, bool link = false);
    static void generateCall(codeGen &gen, Address from, Address to);
    static void generateLongBranch(codeGen &gen, Address from, Address to, bool isCall);
    static void generateBranchViaTrap(codeGen &gen, Address from, Address to, bool isCall);
    static void generateConditionalBranch(codeGen& gen, Address to, unsigned opcode, bool s);
    static void generateMemAccess(codeGen &gen, LoadStore accType, Register r1, Register r2, int immd, unsigned size, IndexMode im=Post);
    template<typename T>
    static void loadImmIntoReg(codeGen &gen, Register rt, T value);
    static void saveRegister(codeGen &gen, Register r, int sp_offset, IndexMode im=Offset);
    static void restoreRegister(codeGen &gen, Register r, int sp_offset, IndexMode im=Offset);
    static void generateMove(codeGen &gen, Register rd, Register rm, bool is64bit = true);
    static void generateMoveFromLR(codeGen &gen, Register rt);
    static void generateMoveToLR(codeGen &gen, Register rs);
    static void generate(codeGen &gen, instruction &insn);
    static void generateAddress(codeGen &gen, Address addr);
    static bool modifyJump(Address targetAddr, instruction &insn, codeGen &gen);
    static bool modifyJcc(Address targetAddr, instruction &insn, codeGen &gen);
    static bool modifyCall(Address targetAddr, instruction &insn, codeGen &gen);
    static bool modifyData(Address targetAddr, instruction &insn, codeGen &gen);
};

bool insn_hasLDst(unsigned int insn);
bool codeGen_generate_gap(codeGen &gen, int offset);
void saveRegister(codeGen &gen, Register reg, int offset);
void restoreRegister(codeGen &gen, Register reg, int offset);

#endif // CODEGEN_LOONGARCH64_H