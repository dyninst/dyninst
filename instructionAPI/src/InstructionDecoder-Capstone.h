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

#if !defined(INSTRUCTION_DECODER_CAPSTONE_H)
#define INSTRUCTION_DECODER_CAPSTONE_H

#include "Expression.h"
#include "dyn_regs.h"
#include "Operation_impl.h"
#include "entryIDs.h"
#include "Instruction.h"
#include "InstructionDecoder.h" // buffer...anything else?
#include "InstructionDecoderImpl.h"

#include "capstone/capstone.h"
#include "capstone/x86.h"
#include "capstone/ppc.h"
#include "capstone/arm64.h"

namespace Dyninst
{
namespace InstructionAPI
{
class InstructionDecoder_Capstone : public InstructionDecoderImpl
{
    public:
      
        InstructionDecoder_Capstone(Architecture a);
        virtual void doDelayedDecode(const Instruction* insn_to_complete);
        virtual void setMode(bool is64) {};
    protected:
      
        virtual bool decodeOperands(const Instruction* insn_to_complete);

        virtual void decodeOpcode(InstructionDecoder::buffer&);
        virtual Result_Type makeSizeType(unsigned int opType) {};


    private:
        static dyn_tls bool handle_init;
        static dyn_tls cs_insn* capstone_ins_no_detail;
        static dyn_tls cs_insn* capstone_ins_with_detail;
        static dyn_tls csh handle_no_detail;
        static dyn_tls csh handle_with_detail;
        static dyn_tls std::map<std::string, std::string> *opcode_alias;
        bool openCapstoneHandle();
        std::string mnemonicNormalization(std::string);

        entryID opcodeTranslation(unsigned int);
        entryID opcodeTranslation_x86(unsigned int);
        entryID opcodeTranslation_ppc(unsigned int);
        entryID opcodeTranslation_aarch64(unsigned int);

        void decodeOperands_x86(const Instruction* insn, cs_detail*);
        void decodeOperands_ppc(const Instruction* insn, cs_detail*);
        void decodeOperands_aarch64(const Instruction* insn, cs_detail*);

        MachRegister registerTranslation_x86(x86_reg);

        Result_Type operandSizeTranslation(uint8_t);
        bool checkCapstoneGroup(cs_detail*, uint8_t);
};

};
};

#endif //!defined(INSTRUCTION_DECODER_CAPSTONE_H)
