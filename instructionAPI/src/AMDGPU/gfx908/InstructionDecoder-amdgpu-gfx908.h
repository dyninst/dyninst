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

#include <array>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include "InstructionDecoderImpl.h"
#include <iostream>
#include "Immediate.h"
#include "dyn_regs.h"
#include <cstddef>

namespace Dyninst {
    namespace InstructionAPI {

#define insn_printf(format, ...) \
        do{ \
            printf("[%s:%u]insn_debug " format, FILE__, __LINE__, ## __VA_ARGS__); \
        }while(0)

        struct amdgpu_gfx908_insn_entry;
        struct amdgpu_mask_entry;

        class InstructionDecoder_amdgpu_gfx908 : public InstructionDecoderImpl {
            friend struct amdgpu_gfx908_insn_entry;
            friend struct amdgpu_mask_entry;
            enum DecodeFamily {sopp};

            public:
            InstructionDecoder_amdgpu_gfx908(Architecture a) : InstructionDecoderImpl(a) {}

            virtual ~InstructionDecoder_amdgpu_gfx908() = default;

            virtual void decodeOpcode(InstructionDecoder::buffer &b);

            // decode one instruction starting from b.start
            // will advance b.start whenver a instruction is successfully decoded
            virtual Instruction decode(InstructionDecoder::buffer &b);

            virtual void setMode(bool) { }

            virtual bool decodeOperands(const Instruction *insn_to_complete);

            bool decodeOperands(const amdgpu_gfx908_insn_entry & insn_entry);

            virtual void doDelayedDecode(const Instruction *insn_to_complete);

            static const std::array<std::string, 16> condNames;
            static MachRegister sysRegMap(unsigned int);
            static const char* bitfieldInsnAliasMap(entryID);
            static const char* condInsnAliasMap(entryID);

            //Check if the index (2nd arg) is valid for the array (1st arg)
            template <typename ArrayType, std::size_t n, typename IndexType>
                constexpr bool isArrayIndexValid(ArrayType (&)[n], const IndexType& i) const {
                    return 0 <= i && i < n;
                }

            private:
            virtual Result_Type makeSizeType(unsigned int opType);

            bool is64Bit{};

            unsigned int insn_size{}; // size of the instruction that we are currently working on
            unsigned int insn{}; // the first 32 bit
            unsigned int insn_high{}; // the second 32 bit
            unsigned long long int insn_long{}; // the combined 64 bit: insn_high << 32 | insn

            // the main process of decoding an instruciton, won't advance buffer
            void mainDecode(); 

            void mainDecodeOpcode(); 


            void setupInsnWord(InstructionDecoder::buffer &b); 
            // pointer to the instruction that we are currently working on
            boost::shared_ptr<Instruction> insn_in_progress;

            template<int start, int end>
                int field(unsigned int raw) {
#if defined DEBUG_FIELD
                    std::cerr << start << "-" << end << ":" << std::dec << (raw >> (start) &
                            (0xFFFFFFFF >> (31 - (end - start)))) << " ";
#endif
                    return (raw >> (start) & (0xFFFFFFFF >> (31 - (end - start))));
                }

            template<int start, int end>
                int longfield(unsigned long long int raw) {
#if defined DEBUG_FIELD
                    std::cerr << start << "-" << end << ":" << std::dec << (raw >> (start) &
                            (0xFFFFFFFFFFFFFFFF >> (63 - (end - start)))) << " ";
#endif
                    return ( (raw >> (start)) & (0xFFFFFFFFFFFFFFFF >> (63 - (end - start))));
                }

            int32_t sign_extend32(int size_, int in) {
                int32_t val = 0 | in;

                return (val << (32 - size_)) >> (32 - size_);
            }

            int64_t sign_extend64(int size_, int in) {
                int64_t val = 0 | in;

                return (val << (64 - size_)) >> (64 - size_);
            }

            uint32_t unsign_extend32(int size_, int in) {
                uint32_t mask = ~0;

                return (mask >> (32 - size_)) & in;
            }

            uint64_t unsign_extend64(int size_, int in) {
                uint64_t mask = ~0;

                return (mask >> (64 - size_)) & in;
            }

            int highest_set_bit(int32_t val) {
                for (int bit_index = 31; bit_index >= 0; bit_index--)
                    if (((static_cast<uint32_t>(val) >> bit_index) & 0x1) == 0x1)
                        return bit_index + 1;

                return -1;
            }

            int lowest_set_bit(int32_t val) {
                for (int bit_index = 0; bit_index <= 31; bit_index++)
                    if (((static_cast<uint32_t>(val) >> bit_index) & 0x1) == 0x1)
                        return bit_index + 1;

                return -1;
            }

            std::string extension;
            bool hasHw{};
            int hwField{};

            void processHwFieldInsn(int, int);

            bool hasShift{};
            int shiftField{};

            void makeBranchTarget(bool, bool, int, int);

            Expression::Ptr makeFallThroughExpr();

            int _szField{}, size{};
            int _typeField{};
            int cmode{};
            int op{};
            int simdAlphabetImm{};

            void processAlphabetImm();

            void NOTHING();
            bool fix_bitfieldinsn_alias(int, int);
            void fix_condinsn_alias_and_cond(int &);
            void modify_mnemonic_simd_upperhalf_insns();

            MachRegister makeAmdgpuRegID(MachRegister, unsigned int, unsigned int len = 1);

            MachRegister getLoadStoreSimdRegister(int encoding);

            Expression::Ptr makePCExpr();


            template<typename T>
                Expression::Ptr makeLogicalImm(int immr, int imms, int immsLen, Result_Type rT);


            //for load store
            void insnSize(unsigned int insn_size );

            Expression::Ptr decodeSSRC(unsigned int index);
            Expression::Ptr decodeVSRC(unsigned int index);
            Expression::Ptr decodeVDST(unsigned int index);

            Expression::Ptr decodeSGPRorM0(unsigned int offset);


            bool useImm{};
            uint32_t immLen{};
            uint32_t immLiteral{};
            uint32_t imm_at_32{};
            uint32_t imm_at_64{};
            uint32_t imm_at_96{};

            bool isBranch{}; // this is set for all branch instructions,
            bool isConditional{}; // this is set for all conditional branch instruction, will set branchCond
            bool isCall{}; // this is a call function

            // this is set for instructions that directly modify pc
            // namely s_setpc and s_swappc
            bool isModifyPC{};

            // reset the decoder internal state for decoding the next instruction
            void reset();

            Expression::Ptr branchCond;
            Expression::Ptr branchTarget;

            void setBranch(){
                isBranch = true;
            }

            void setConditionalBranch(){
                isConditional = true;
                // TODO : set conditional branch
            }
            void setModifyPC(){
                isModifyPC = true;
            }

            void setCall(){
                isCall =  true;
            }

            inline unsigned int get32bit(InstructionDecoder::buffer &b,unsigned int offset );

            template<unsigned int start,unsigned int end, unsigned int candidate>
                void setUseImm(InstructionDecoder::buffer & b, unsigned int offset){
                    if ( longfield<start,end>(insn_long) == candidate ){
                        useImm = true;
                        immLen = 4;
                        immLiteral = get32bit(b,offset);
                    }

                }

            typedef struct buffer_resource_desc{
                unsigned long long base_address;
                unsigned stride;
                unsigned cache_swizzle;
                unsigned swizzle_enable;
                unsigned num_records;
                unsigned dst_sel_x;
                unsigned dst_sel_y;
                unsigned dst_sel_z;
                unsigned dst_sel_w;
                unsigned num_format;
                unsigned data_format;
                unsigned user_vm_enable;
                unsigned user_vm_mode;
                unsigned index_stride;
                unsigned add_tid_enable;
                unsigned non_volatile;
                unsigned type;
            }buffer_resource_desc;

            void debug_instr();

            uint32_t decodeOPR_LITERAL();
            Expression::Ptr decodeOPR_SDWA();
            Expression::Ptr decodeOPR_LABEL(uint64_t input);
            Expression::Ptr decodeOPR_SIMM4(uint64_t input);
            Expression::Ptr decodeOPR_SIMM8(uint64_t input);
            Expression::Ptr decodeOPR_SIMM16(uint64_t input);
            Expression::Ptr decodeOPR_SIMM32(uint64_t input);
            Expression::Ptr decodeOPR_WAITCNT(uint64_t input);
            using InstructionDecoderImpl::makeRegisterExpression;
            Expression::Ptr makeRegisterExpression(MachRegister registerID, uint32_t num_elements = 1);
            Expression::Ptr makeRegisterExpression(MachRegister registerID, uint32_t low , uint32_t high );
            void specialHandle();
#include "amdgpu_gfx908_decoder_impl.h"    
#include "decodeOperands.h"    
        };
    }
}

