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

#include "InstructionDecoder-aarch32.h"
#include <boost/assign/list_of.hpp>
#include "../../common/src/singleton_object_pool.h"

#include "aarch32_decoder_autogen.C"

namespace Dyninst {
namespace InstructionAPI {

InstructionDecoder_aarch32::InstructionDecoder_aarch32(Architecture a)
    : InstructionDecoderImpl(a)
{
    std::string condArray[16] = {
        "eq", "ne", "cs", "cc",
        "mi", "pl", "vs", "vc",
        "hi", "ls", "ge", "lt",
        "gt", "le", "al", "nv"
    };
}

InstructionDecoder_aarch32::~InstructionDecoder_aarch32()
{
}

void
InstructionDecoder_aarch32::decodeOpcode(InstructionDecoder::buffer &b)
{
    b.start += 4;
}

using namespace std;

Instruction::Ptr
InstructionDecoder_aarch32::decode(InstructionDecoder::buffer &b)
{
    if (b.start > b.end)
        return Instruction::Ptr();

    rawInsn = (b.start[3] << 0x18 |
               b.start[2] << 0x10 |
               b.start[1] << 0x08 |
               b.start[0]);

#if defined(DEBUG_RAW_INSN)
    cout.width(0);
    cout << "0x";
    cout.width(8);
    cout.fill('0');
    cout << hex << insn << "\t";
#endif

    mainDecode();
    b.start += 4;

    return make_shared(decodedInsn);
}

Result_Type
InstructionDecoder_aarch32::makeSizeType(unsigned int)
{
    assert(0);
    return u32;
}

// ****************
// decoding opcodes
// ****************
void
InstructionDecoder_aarch32::doDelayedDecode(const Instruction* insnToComplete)
{
    InstructionDecoder::buffer b(insnToComplete->ptr(),
                                 insnToComplete->size());

    //insn->m_Operands.reserve(4);
    decode(b);
    decodeOperands(insnToComplete);
}

bool
InstructionDecoder_aarch32::decodeOperands(const Instruction* insnToComplete)
{
    Insn_Entry* insnInfo = identify(rawInsn);

    if (!insnInfo) {
        decodedInsn->getOperation().mnemonic = INVALID_INSN->mnemonic;
        decodedInsn->getOperation().operationID = INVALID_INSN->op;
        decodedInsn->m_Operands.clear();
        decodedInsn->m_Successors.clear();
        return true;
    }

    rawInsn = insnToComplete->m_RawInsn.small_insn;
    decodedInsn = const_cast<Instruction*>(insnToComplete);

    for (int i = 0; insnInfo->operand[i] != OPR_empty; ++i) {
        handle_operand(insnInfo->operand[i]);
    }

    return false;
/*
    rawInsn = insnToComplete->m_RawInsn.small_insn;
    decodedInsn = const_cast<Instruction *>(insnToComplete);

    if (IS_INSN_LDST_REG(insn) ||
        IS_INSN_ADDSUB_EXT(insn) ||
        IS_INSN_ADDSUB_SHIFT(insn) ||
        IS_INSN_LOGICAL_SHIFT(insn))
        skipRm = true;

    for (operandSpec::const_iterator fn = insn_table_entry->operands.begin();
         fn != insn_table_entry->operands.end(); fn++) {
        std::mem_fun(*fn)(this);
    }

    if (insn_table_index == 0)
        isValid = false;

    if (!isValidecodedd) {
        decodedInsn->getOperation().mnemonic = INVALID_ENTRY.mnemonic;
        decodedInsn->getOperation().operationID = INVALID_ENTRY.op;
        decodedInsn->m_Operands.clear();
        decodedInsn->m_Successors.clear();
    }
    else {
        reorderOperands();

        if (IS_INSN_SYSTEM(insn)) {
            processSystemInsn();
        }

        if (IS_INSN_SIMD_MOD_IMM(insn)) {
            processAlphabetImm();
        }

        if (IS_INSN_LDST_SIMD_MULT_POST(insn) || IS_INSN_LDST_SIMD_SING_POST(insn))
            decodedInsn->appendOperand(makeRnExpr(), false, true);

        if (isPstateWritten || isPstateRead)
            decodedInsn->appendOperand(makePstateExpr(), isPstateRead, isPstateWritten);
    }
    return true;
*/
}

void
InstructionDecoder_aarch32::mainDecode(void)
{
    unsigned char* rawPtr = reinterpret_cast<unsigned char*>(&rawInsn);
    Insn_Entry* insnInfo = identify(rawInsn);
    std::string mnemonic(insnInfo->mnemonic);

    mnemonic += insnSuffix[get_condition_field(rawInsn)];
    decodedInsn = makeInstruction(insnInfo->op,
                                  mnemonic.c_str(),
                                  4,
                                  rawPtr);
    decodedInsn->arch_decoded_from = Arch_aarch32;

    // XXX The following test needs to include branch instructions
    //     that write to PC as R15.
    InsnCategory c = entryToCategory(insnInfo->op);
    if (c == c_CallInsn || c == c_BranchInsn) {
        decodeOperands(decodedInsn);
    }

    return;
}

}; // End namespace InstructionAPI
}; // End namespace Dyninst
