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

#if !defined(INSTRUCTION_DECODER_X86_H)
#define INSTRUCTION_DECODER_X86_H

#include "InstructionDecoderImpl.h"

#include "common/src/ia32_locations.h"

namespace NS_x86 {
struct ia32_operand;
class ia32_instruction;
}

namespace Dyninst
{
    namespace InstructionAPI
    {
    
    /// The %InstructionDecoder class decodes instructions, given a buffer of bytes and a length,
    /// and constructs an %Instruction.
    /// The %InstructionDecoder will, by default, be constructed to decode machine language
    /// on the platform on which it has been compiled.  The buffer
    /// will be treated as if there is an instruction stream starting at the beginning of the buffer.
    /// %InstructionDecoder objects are given a buffer from which to decode at construction.
    /// Calls to \c decode will proceed to decode instructions sequentially from that buffer until its
    /// end is reached.  At that point, all subsequent calls to \c decode will return an invalid
    /// %Instruction object.
        ///
    /// An %InstructionDecoder object may alternately be constructed without designating a buffer,
    /// and the buffer may be specified at the time \c decode is called.  This method of use may be
    /// more convenient for users who are decoding non-contiguous instructions.

        class InstructionDecoder_x86 : public InstructionDecoderImpl
        {
            friend class Instruction;
            public:
                INSTRUCTION_EXPORT InstructionDecoder_x86(Architecture a);
      
                INSTRUCTION_EXPORT virtual ~InstructionDecoder_x86();
            private:
                INSTRUCTION_EXPORT InstructionDecoder_x86(const InstructionDecoder_x86& o);
            public:
                INSTRUCTION_EXPORT virtual Instruction decode(InstructionDecoder::buffer& b);
      
                INSTRUCTION_EXPORT virtual void setMode(bool is64);
                virtual void doDelayedDecode(const Instruction* insn_to_complete);

            protected:
      
                virtual bool decodeOperands(const Instruction* insn_to_complete);

                bool decodeOneOperand(const InstructionDecoder::buffer& b,
				      const NS_x86::ia32_operand& operand,
                                      int & imm_index,
                                      const Instruction* insn_to_complete, 
                                      bool isRead, bool isWritten, bool isImplicit);
                virtual void decodeOpcode(InstructionDecoder::buffer& b);
      
                Expression::Ptr makeSIBExpression(const InstructionDecoder::buffer& b);
                Expression::Ptr makeModRMExpression(const InstructionDecoder::buffer& b,
						    unsigned int opType);
                Expression::Ptr getModRMDisplacement(const InstructionDecoder::buffer& b);
                MachRegister makeRegisterID(unsigned int intelReg, unsigned int opType, bool isExtendedReg = false);
                Expression::Ptr decodeImmediate(unsigned int opType, const unsigned char* immStart, bool isSigned = false);
                virtual Result_Type makeSizeType(unsigned int opType);

            private:
                void doIA32Decode(InstructionDecoder::buffer& b);
		        bool isDefault64Insn();
		
                ia32_locations* locs;
                NS_x86::ia32_instruction* decodedInstruction;
                bool sizePrefixPresent;
                bool addrSizePrefixPresent;
                bool is64BitMode;
        };
    }
}


#endif // !defined(INSTRUCTION_DECODER_X86_H)
