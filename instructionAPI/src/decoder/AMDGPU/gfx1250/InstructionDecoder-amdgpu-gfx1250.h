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
#ifndef INSTRUCTION_DECODER_GFX1250_H
#define INSTRUCTION_DECODER_GFX1250_H
#include <array>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include "decoder/InstructionDecoderImpl.h"
#include <iostream>
#include "Immediate.h"
#include "Architecture.h"
#include <cstddef>

namespace Dyninst {
    namespace InstructionAPI {

#define insn_printf(format, ...) \
        do{ \
            printf("[%s:%u]insn_debug " format, FILE__, __LINE__, ## __VA_ARGS__); \
        }while(0)

        struct amdgpu_gfx1250_insn_entry;

        class InstructionDecoder_amdgpu_gfx1250 : public InstructionDecoderImpl {
            friend struct amdgpu_gfx1250_insn_entry;
            friend struct amdgpu_mask_entry;

            public:
    		InstructionDecoder_amdgpu_gfx1250(Architecture a) : InstructionDecoderImpl(a)  {}

            virtual ~InstructionDecoder_amdgpu_gfx1250() = default;

            // decode one instruction starting from b.start
            // will advance b.start whenver a instruction is successfully decoded
            virtual Instruction decode(InstructionDecoder::buffer &b);

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

            bool is64Bit{};

            uint32_t insn_size{}; // size of the instruction that we are currently working on
            uint32_t insn{}; // the first 32 bit
            uint32_t insn_high{}; // the second 32 bit
            uint64_t insn_long{}; // the combined 64 bit: insn_high << 32 | insn
            uint64_t insn_long2{}; // bits 127:64, only used by 128-bit encodings (ENC_VOP3PX2)

            // the main process of decoding an instruciton, won't advance buffer
            void mainDecode();



            void setupInsnWord(InstructionDecoder::buffer &b);
            // pointer to the instruction that we are currently working on
            boost::shared_ptr<Instruction> insn_in_progress;

            // start/end are INCLUSIVE bit positions, matching the ISA
            // manual's [end:start] field notation; a single bit is
            // field<n,n>. RegisterAST bit ranges are exclusive-high.
            template<int start, int end>
                int field(uint32_t raw)  {
#if defined DEBUG_FIELD
                    std::cerr << start << "-" << end << ":" << std::dec << (raw >> (start) &
                            (0xFFFFFFFF >> (31 - (end - start)))) << " ";
#endif
                    return (raw >> (start) & (0xFFFFFFFF >> (31 - (end - start))));
                }

            template<int start, int end>
                uint64_t longfield(uint64_t raw)  {
#if defined DEBUG_FIELD
                    std::cerr << start << "-" << end << ":" << std::dec << (raw >> (start) &
                            (0xFFFFFFFFFFFFFFFF >> (63 - (end - start)))) << " ";
#endif
                    return ( (raw >> (start)) & (0xFFFFFFFFFFFFFFFF >> (63 - (end - start))));
                }

            int32_t sign_extend32(int size_, int in)  {
                int32_t val = 0 | in;

                return (val << (32 - size_)) >> (32 - size_);
            }

            int64_t sign_extend64(int size_, int in)  {
                int64_t val = 0 | in;

                return (val << (64 - size_)) >> (64 - size_);
            }

            uint32_t unsign_extend32(int size_, int in)  {
                uint32_t mask = ~0;

                return (mask >> (32 - size_)) & in;
            }

            uint64_t unsign_extend64(int size_, int in)  {
                uint64_t mask = ~0;

                return (mask >> (64 - size_)) & in;
            }

            int highest_set_bit(int32_t val)  {
                for (int bit_index = 31; bit_index >= 0; bit_index--)
                    if (((static_cast<uint32_t>(val) >> bit_index) & 0x1) == 0x1)
                        return bit_index + 1;

                return -1;
            }

            int lowest_set_bit(int32_t val)  {
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

            void makeBranchTarget(bool, bool, int, int immLen = 16);

            Expression::Ptr makeFallThroughExpr();

            int _szField{}, size{};
            int _typeField{};
            int cmode{};
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
            uint32_t immLen{}; // extra 4 bytes included for decoding instruction
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

            void setBranch()  {
                isBranch = true;
            }

            void setConditionalBranch()  {
                isConditional = true;
                // TODO : set conditional branch
            }
            void setModifyPC()  {
                isModifyPC = true;
            }

            void setCall()  {
                isCall =  true;
            }

            inline unsigned int get32bit(InstructionDecoder::buffer &b,unsigned int offset );

            template<unsigned int start,unsigned int end, unsigned int candidate>
                void setUseImm(InstructionDecoder::buffer & b, unsigned int offset)
                {
                    if (longfield<start,end>(insn_long) == candidate)  {
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
            Expression::Ptr decodeOPR_LABEL(uint64_t input);
            using InstructionDecoderImpl::makeRegisterExpression;
            Expression::Ptr makeRegisterExpression(MachRegister registerID, uint32_t num_elements = 1);
            Expression::Ptr makeRegisterExpression(MachRegister registerID, uint32_t low , uint32_t high );
            void specialHandle();

            static bool IS_ENC_SOP1(uint64_t I);
            static bool IS_ENC_SOPC(uint64_t I);
            static bool IS_ENC_SOPP(uint64_t I);
            static bool IS_ENC_SOPK(uint64_t I);
            static bool IS_ENC_SOP2(uint64_t I);
            static bool IS_ENC_SMEM(uint64_t I);
            static bool IS_ENC_VOP1(uint64_t I);
            static bool IS_ENC_VOPC(uint64_t I);
            static bool IS_ENC_VOP2(uint64_t I);
            static bool IS_ENC_VOP3(uint64_t I);
            static bool IS_ENC_VOP3PX2(uint64_t I);
            static bool IS_ENC_VOP3PX3(uint64_t I);
            static bool IS_ENC_VOP3P(uint64_t I);
            static bool IS_ENC_VDS(uint64_t I);
            static bool IS_ENC_VBUFFER(uint64_t I);
            static bool IS_ENC_VIMAGE(uint64_t I);
            static bool IS_ENC_VFLAT(uint64_t I);
            static bool IS_ENC_VSCRATCH(uint64_t I);
            static bool IS_ENC_VGLOBAL(uint64_t I);
            static bool IS_SOP1_INST_LITERAL(uint64_t I);
            static bool IS_SOP1_INST_LITERAL64(uint64_t I);
            static bool IS_SOP2_INST_LITERAL(uint64_t I);
            static bool IS_SOP2_INST_LITERAL64(uint64_t I);
            static bool IS_SOPC_INST_LITERAL(uint64_t I);
            static bool IS_SOPC_INST_LITERAL64(uint64_t I);
            static bool IS_SOPK_INST_LITERAL(uint64_t I);
            static bool IS_VOP1_INST_LITERAL(uint64_t I);
            static bool IS_VOP1_INST_LITERAL64(uint64_t I);
            static bool IS_VOP1_VOP_DPP8(uint64_t I);
            static bool IS_VOP1_VOP_DPP16(uint64_t I);
            static bool IS_VOP3_INST_LITERAL(uint64_t I);
            static bool IS_VOP3_VOP_DPP8(uint64_t I);
            static bool IS_VOP3_VOP_DPP16(uint64_t I);
            static bool IS_VOP2_INST_LITERAL(uint64_t I);
            static bool IS_VOP2_INST_LITERAL64(uint64_t I);
            static bool IS_VOP2_VOP_DPP8(uint64_t I);
            static bool IS_VOP2_VOP_DPP16(uint64_t I);
            static bool IS_VOP3_SDST_ENC(uint64_t I);
            static bool IS_VOP3_SDST_ENC_INST_LITERAL(uint64_t I);
            static bool IS_VOP3_SDST_ENC_VOP_DPP8(uint64_t I);
            static bool IS_VOP3_SDST_ENC_VOP_DPP16(uint64_t I);
            static bool IS_VOP3P_INST_LITERAL(uint64_t I);
            static bool IS_VOP3P_VOP_DPP8(uint64_t I);
            static bool IS_VOP3P_VOP_DPP16(uint64_t I);
            static bool IS_VOPC_INST_LITERAL(uint64_t I);
            static bool IS_VOPC_INST_LITERAL64(uint64_t I);
            static bool IS_VOPC_VOP_DPP8(uint64_t I);
            static bool IS_VOPC_VOP_DPP16(uint64_t I);
            static bool IS_VOPD3XY_X(uint64_t I);
            static bool IS_VOPD3XY_Y(uint64_t I);
            static bool IS_VOPDXY_X(uint64_t I);
            static bool IS_VOPDXY_INST_LITERAL_X(uint64_t I);
            static bool IS_VOPDXY_Y(uint64_t I);
            static bool IS_VOPDXY_INST_LITERAL_Y(uint64_t I);
            bool COND_ENC_SOP1_not_has_lit64_0_and_not_has_lit64_1_and_not_has_lit_0_and_not_has_lit_1(uint64_t I);
            bool COND_ENC_SOPC_default(uint64_t I);
            bool COND_ENC_SOP2_default(uint64_t I);
            bool COND_ENC_VOP1_default(uint64_t I);
            bool COND_ENC_VOPC_default(uint64_t I);
            bool COND_ENC_VOP2_default(uint64_t I);
            bool COND_ENC_VOP3_default(uint64_t I);
            bool COND_ENC_VOP3P_default(uint64_t I);
            bool COND_SOP1_INST_LITERAL_has_lit_0(uint64_t I);
            bool COND_SOP1_INST_LITERAL64_has_lit64_0(uint64_t I);
            bool COND_SOP2_INST_LITERAL_has_lit_0(uint64_t I);
            bool COND_SOP2_INST_LITERAL_has_lit_1(uint64_t I);
            bool COND_SOP2_INST_LITERAL_has_lit_0_and_has_lit_1(uint64_t I);
            bool COND_SOP2_INST_LITERAL_default(uint64_t I);
            bool COND_SOP2_INST_LITERAL64_has_lit64_0(uint64_t I);
            bool COND_SOP2_INST_LITERAL64_has_lit64_1(uint64_t I);
            bool COND_SOP2_INST_LITERAL64_has_lit64_0_and_has_lit64_1(uint64_t I);
            bool COND_SOPC_INST_LITERAL_has_lit_0(uint64_t I);
            bool COND_SOPC_INST_LITERAL_has_lit_1(uint64_t I);
            bool COND_SOPC_INST_LITERAL_has_lit_0_and_has_lit_1(uint64_t I);
            bool COND_SOPC_INST_LITERAL64_has_lit64_0(uint64_t I);
            bool COND_SOPC_INST_LITERAL64_has_lit64_1(uint64_t I);
            bool COND_SOPC_INST_LITERAL64_has_lit64_0_and_has_lit64_1(uint64_t I);
            bool COND_VOP1_INST_LITERAL_has_lit(uint64_t I);
            bool COND_VOP1_INST_LITERAL64_has_lit64(uint64_t I);
            bool COND_VOP1_VOP_DPP8_has_dpp8(uint64_t I);
            bool COND_VOP1_VOP_DPP16_has_dpp16(uint64_t I);
            bool COND_VOP3_INST_LITERAL_has_lit_0(uint64_t I);
            bool COND_VOP3_INST_LITERAL_has_lit_1(uint64_t I);
            bool COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_1(uint64_t I);
            bool COND_VOP3_INST_LITERAL_has_lit_2(uint64_t I);
            bool COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_2(uint64_t I);
            bool COND_VOP3_INST_LITERAL_has_lit_1_and_has_lit_2(uint64_t I);
            bool COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_1_and_has_lit_2(uint64_t I);
            bool COND_VOP3_VOP_DPP8_has_dpp8(uint64_t I);
            bool COND_VOP3_VOP_DPP16_has_dpp16(uint64_t I);
            bool COND_VOP2_INST_LITERAL_has_lit(uint64_t I);
            bool COND_VOP2_INST_LITERAL_default(uint64_t I);
            bool COND_VOP2_INST_LITERAL64_has_lit64(uint64_t I);
            bool COND_VOP2_INST_LITERAL64_default(uint64_t I);
            bool COND_VOP2_VOP_DPP8_has_dpp8(uint64_t I);
            bool COND_VOP2_VOP_DPP16_has_dpp16(uint64_t I);
            bool COND_VOP3_SDST_ENC_default(uint64_t I);
            bool COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_0(uint64_t I);
            bool COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_1(uint64_t I);
            bool COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_0_and_has_lit_1(uint64_t I);
            bool COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_2(uint64_t I);
            bool COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_0_and_has_lit_2(uint64_t I);
            bool COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_1_and_has_lit_2(uint64_t I);
            bool COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_0_and_has_lit_1_and_has_lit_2(uint64_t I);
            bool COND_VOP3_SDST_ENC_VOP_DPP8_has_dpp8(uint64_t I);
            bool COND_VOP3_SDST_ENC_VOP_DPP16_has_dpp16(uint64_t I);
            bool COND_VOP3P_INST_LITERAL_has_lit_0(uint64_t I);
            bool COND_VOP3P_INST_LITERAL_has_lit_1(uint64_t I);
            bool COND_VOP3P_INST_LITERAL_has_lit_0_and_has_lit_1(uint64_t I);
            bool COND_VOP3P_INST_LITERAL_has_lit_2(uint64_t I);
            bool COND_VOP3P_INST_LITERAL_has_lit_0_and_has_lit_2(uint64_t I);
            bool COND_VOP3P_INST_LITERAL_has_lit_1_and_has_lit_2(uint64_t I);
            bool COND_VOP3P_INST_LITERAL_has_lit_0_and_has_lit_1_and_has_lit_2(uint64_t I);
            bool COND_VOP3P_VOP_DPP8_has_dpp8(uint64_t I);
            bool COND_VOP3P_VOP_DPP16_has_dpp16(uint64_t I);
            bool COND_VOPC_INST_LITERAL_has_lit(uint64_t I);
            bool COND_VOPC_INST_LITERAL64_has_lit64(uint64_t I);
            bool COND_VOPC_VOP_DPP8_has_dpp8(uint64_t I);
            bool COND_VOPC_VOP_DPP16_has_dpp16(uint64_t I);
            bool COND_VOPDXY_X_default(uint64_t I);
            bool COND_VOPDXY_INST_LITERAL_X_has_lit(uint64_t I);
            bool COND_VOPDXY_INST_LITERAL_X_default(uint64_t I);
            bool COND_VOPDXY_Y_default(uint64_t I);
            bool COND_VOPDXY_INST_LITERAL_Y_has_lit(uint64_t I);
            bool COND_VOPDXY_INST_LITERAL_Y_default(uint64_t I);
            enum InstructionFamily
            {
                ENC_SOP1 = -1,
                ENC_SOPC = 0,
                ENC_SOPP = 1,
                ENC_SOPK = 2,
                ENC_SOP2 = 3,
                ENC_SMEM = 4,
                ENC_VOP1 = 5,
                ENC_VOPC = 6,
                ENC_VOP2 = 7,
                ENC_VOP3 = 8,
                ENC_VOP3PX2 = 9,
                ENC_VOP3PX3 = 10,
                ENC_VOP3P = 11,
                ENC_VDS = 20,
                ENC_VBUFFER = 21,
                ENC_VIMAGE = 22,
                ENC_VFLAT = 25,
                ENC_VSCRATCH = 26,
                ENC_VGLOBAL = 27,
                SOP1_INST_LITERAL = 28,
                SOP1_INST_LITERAL64 = 29,
                SOP2_INST_LITERAL = 30,
                SOP2_INST_LITERAL64 = 31,
                SOPC_INST_LITERAL = 32,
                SOPC_INST_LITERAL64 = 33,
                SOPK_INST_LITERAL = 34,
                VOP1_INST_LITERAL = 35,
                VOP1_INST_LITERAL64 = 36,
                VOP1_VOP_DPP8 = 37,
                VOP1_VOP_DPP16 = 38,
                VOP3_INST_LITERAL = 39,
                VOP3_VOP_DPP8 = 40,
                VOP3_VOP_DPP16 = 41,
                VOP2_INST_LITERAL = 42,
                VOP2_INST_LITERAL64 = 43,
                VOP2_VOP_DPP8 = 44,
                VOP2_VOP_DPP16 = 45,
                VOP3_SDST_ENC = 46,
                VOP3_SDST_ENC_INST_LITERAL = 47,
                VOP3_SDST_ENC_VOP_DPP8 = 48,
                VOP3_SDST_ENC_VOP_DPP16 = 49,
                VOP3P_INST_LITERAL = 50,
                VOP3P_VOP_DPP8 = 51,
                VOP3P_VOP_DPP16 = 52,
                VOPC_INST_LITERAL = 53,
                VOPC_INST_LITERAL64 = 54,
                VOPC_VOP_DPP8 = 55,
                VOPC_VOP_DPP16 = 56,
                VOPD3XY_X = 57,
                VOPD3XY_Y = 57,
                VOPDXY_X = 58,
                VOPDXY_INST_LITERAL_X = 59,
                VOPDXY_Y = 60,
                VOPDXY_INST_LITERAL_Y = 61,
            };
            InstructionFamily instr_family;
            struct layout_ENC_SOP1 {
                uint16_t ENCODING : 9;
                uint8_t OP : 8;
                uint8_t SDST : 7;
                uint8_t SSRC0 : 8;
            };
            struct layout_ENC_SOPC {
                uint16_t ENCODING : 9;
                uint8_t OP : 7;
                uint8_t SSRC0 : 8;
                uint8_t SSRC1 : 8;
            };
            struct layout_ENC_SOPP {
                uint16_t ENCODING : 9;
                uint8_t OP : 7;
                uint16_t SIMM16 : 16;
            };
            struct layout_ENC_SOPK {
                uint8_t ENCODING : 4;
                uint8_t OP : 5;
                uint8_t SDST : 7;
                uint16_t SIMM16 : 16;
            };
            struct layout_ENC_SOP2 {
                uint8_t ENCODING : 2;
                uint8_t OP : 7;
                uint8_t SDST : 7;
                uint8_t SSRC0 : 8;
                uint8_t SSRC1 : 8;
            };
            struct layout_ENC_SMEM {
                uint8_t ENCODING : 6;
                uint32_t IOFFSET : 24;
                uint8_t NV : 1;
                uint8_t OP : 6;
                uint8_t SBASE : 7;
                uint8_t SCALE_OFFSET : 1;
                uint8_t SCOPE : 2;
                uint8_t SDATA : 7;
                uint8_t SOFFSET : 7;
                uint8_t TH : 2;
            };
            struct layout_ENC_VOP1 {
                uint8_t ENCODING : 7;
                uint8_t OP : 7;
                uint16_t SRC0 : 9;
                uint8_t VDST : 8;
            };
            struct layout_ENC_VOPC {
                uint8_t ENCODING : 7;
                uint8_t OP : 8;
                uint16_t SRC0 : 9;
                uint8_t VSRC1 : 8;
            };
            struct layout_ENC_VOP2 {
                uint8_t ENCODING : 1;
                uint8_t OP : 6;
                uint16_t SRC0 : 9;
                uint8_t VDST : 8;
                uint8_t VSRC1 : 8;
            };
            struct layout_ENC_VOP3 {
                uint8_t ABS : 3;
                uint8_t CLAMP : 1;
                uint8_t ENCODING : 6;
                uint8_t NEG : 3;
                uint8_t OMOD : 2;
                uint16_t OP : 10;
                uint8_t OPSEL : 4;
                uint16_t SRC0 : 9;
                uint16_t SRC1 : 9;
                uint16_t SRC2 : 9;
                uint8_t VDST : 8;
            };
            struct layout_ENC_VOP3PX2 {
                uint8_t CLAMP : 1;
                uint8_t ENCODING : 8;
                uint8_t NEG : 3;
                uint8_t NEG_HI : 3;
                uint8_t OP : 8;
                uint8_t OPSEL : 3;
                uint8_t OPSEL_HI : 4;
                uint8_t SCALE_CLAMP : 1;
                uint8_t SCALE_NEG : 3;
                uint8_t SCALE_NEG_HI : 3;
                uint8_t SCALE_OPSEL : 3;
                uint8_t SCALE_OPSEL_HI : 2;
                uint16_t SCALE_SRC0 : 9;
                uint16_t SCALE_SRC1 : 9;
                uint16_t SCALE_SRC2 : 9;
                uint16_t SRC0 : 9;
                uint16_t SRC1 : 9;
                uint16_t SRC2 : 9;
                uint8_t VDST : 8;
                uint16_t X2ENCODING : 16;
            };
            struct layout_ENC_VOP3PX3 {
                uint8_t CLAMP : 1;
                uint8_t ENCODING : 8;
                uint8_t NEG : 3;
                uint8_t NEG_HI : 3;
                uint8_t OP : 8;
                uint8_t OPSEL : 3;
                uint8_t OPSEL_HI : 4;
                uint8_t SCALE_CLAMP : 1;
                uint8_t SCALE_NEG : 3;
                uint8_t SCALE_NEG_HI : 3;
                uint8_t SCALE_OPSEL : 3;
                uint8_t SCALE_OPSEL_HI : 2;
                uint16_t SCALE_SRC0 : 9;
                uint16_t SCALE_SRC1 : 9;
                uint16_t SCALE_SRC2 : 9;
                uint16_t SRC0 : 9;
                uint16_t SRC1 : 9;
                uint16_t SRC2 : 9;
                uint8_t VDST : 8;
                uint16_t X3ENCODING : 16;
            };
            struct layout_ENC_VOP3P {
                uint8_t CLAMP : 1;
                uint8_t ENCODING : 8;
                uint8_t NEG : 3;
                uint8_t NEG_HI : 3;
                uint8_t OP : 8;
                uint8_t OPSEL : 3;
                uint8_t OPSEL_HI : 3;
                uint16_t SRC0 : 9;
                uint16_t SRC1 : 9;
                uint16_t SRC2 : 9;
                uint8_t VDST : 8;
            };
            struct layout_ENC_VDS {
                uint8_t ADDR : 8;
                uint8_t DATA0 : 8;
                uint8_t DATA1 : 8;
                uint8_t ENCODING : 6;
                uint8_t OFFSET0 : 8;
                uint8_t OFFSET1 : 8;
                uint8_t OP : 8;
                uint8_t VDST : 8;
            };
            struct layout_ENC_VBUFFER {
                uint8_t ENCODING : 6;
                uint8_t FORMAT : 7;
                uint8_t IDXEN : 1;
                uint32_t IOFFSET : 24;
                uint8_t NV : 1;
                uint8_t OFFEN : 1;
                uint8_t OP : 8;
                uint16_t RSRC : 9;
                uint8_t SCOPE : 2;
                uint8_t SOFFSET : 7;
                uint8_t TFE : 1;
                uint8_t TH : 3;
                uint8_t VADDR : 8;
                uint8_t VDATA : 8;
            };
            struct layout_ENC_VIMAGE {
                uint8_t A16 : 1;
                uint8_t D16 : 1;
                uint8_t DIM : 3;
                uint8_t DMASK : 4;
                uint8_t ENCODING : 6;
                uint8_t NV : 1;
                uint8_t OP : 8;
                uint8_t R128 : 1;
                uint16_t RSRC : 9;
                uint8_t SCOPE : 2;
                uint8_t TFE : 1;
                uint8_t TH : 3;
                uint8_t VADDR0 : 8;
                uint8_t VADDR1 : 8;
                uint8_t VADDR2 : 8;
                uint8_t VADDR3 : 8;
                uint8_t VADDR4 : 8;
                uint8_t VDATA : 8;
            };
            struct layout_ENC_VFLAT {
                uint8_t ENCODING : 8;
                uint32_t IOFFSET : 24;
                uint8_t NV : 1;
                uint8_t OP : 8;
                uint8_t SADDR : 7;
                uint8_t SCALE_OFFSET : 1;
                uint8_t SCOPE : 2;
                uint8_t SVE : 1;
                uint8_t TH : 3;
                uint8_t VADDR : 8;
                uint8_t VDST : 8;
                uint8_t VSRC : 8;
            };
            struct layout_ENC_VSCRATCH {
                uint8_t ENCODING : 8;
                uint32_t IOFFSET : 24;
                uint8_t NV : 1;
                uint8_t OP : 8;
                uint8_t SADDR : 7;
                uint8_t SCALE_OFFSET : 1;
                uint8_t SCOPE : 2;
                uint8_t SVE : 1;
                uint8_t TH : 3;
                uint8_t VADDR : 8;
                uint8_t VDST : 8;
                uint8_t VSRC : 8;
            };
            struct layout_ENC_VGLOBAL {
                uint8_t ENCODING : 8;
                uint32_t IOFFSET : 24;
                uint8_t NV : 1;
                uint8_t OP : 8;
                uint8_t SADDR : 7;
                uint8_t SCALE_OFFSET : 1;
                uint8_t SCOPE : 2;
                uint8_t SVE : 1;
                uint8_t TH : 3;
                uint8_t VADDR : 8;
                uint8_t VDST : 8;
                uint8_t VSRC : 8;
            };
            struct layout_SOP1_INST_LITERAL {
                uint16_t ENCODING : 9;
                uint32_t LITERAL : 32;
                uint8_t OP : 8;
                uint8_t SDST : 7;
                uint8_t SSRC0 : 8;
            };
            struct layout_SOP1_INST_LITERAL64 {
                uint16_t ENCODING : 9;
                uint64_t LITERAL64 : 64;
                uint8_t OP : 8;
                uint8_t SDST : 7;
                uint8_t SSRC0 : 8;
            };
            struct layout_SOP2_INST_LITERAL {
                uint8_t ENCODING : 2;
                uint32_t LITERAL : 32;
                uint8_t OP : 7;
                uint8_t SDST : 7;
                uint8_t SSRC0 : 8;
                uint8_t SSRC1 : 8;
            };
            struct layout_SOP2_INST_LITERAL64 {
                uint8_t ENCODING : 2;
                uint64_t LITERAL64 : 64;
                uint8_t OP : 7;
                uint8_t SDST : 7;
                uint8_t SSRC0 : 8;
                uint8_t SSRC1 : 8;
            };
            struct layout_SOPC_INST_LITERAL {
                uint16_t ENCODING : 9;
                uint32_t LITERAL : 32;
                uint8_t OP : 7;
                uint8_t SSRC0 : 8;
                uint8_t SSRC1 : 8;
            };
            struct layout_SOPC_INST_LITERAL64 {
                uint16_t ENCODING : 9;
                uint64_t LITERAL64 : 64;
                uint8_t OP : 7;
                uint8_t SSRC0 : 8;
                uint8_t SSRC1 : 8;
            };
            struct layout_SOPK_INST_LITERAL {
                uint8_t ENCODING : 4;
                uint32_t LITERAL : 32;
                uint8_t OP : 5;
                uint8_t SDST : 7;
                uint16_t SIMM16 : 16;
            };
            struct layout_VOP1_INST_LITERAL {
                uint8_t ENCODING : 7;
                uint32_t LITERAL : 32;
                uint8_t OP : 7;
                uint16_t SRC0 : 9;
                uint8_t VDST : 8;
            };
            struct layout_VOP1_INST_LITERAL64 {
                uint8_t ENCODING : 7;
                uint64_t LITERAL64 : 64;
                uint8_t OP : 7;
                uint16_t SRC0 : 9;
                uint8_t VDST : 8;
            };
            struct layout_VOP1_VOP_DPP8 {
                uint8_t ENCODING : 7;
                uint8_t LANE_SEL_0 : 3;
                uint8_t LANE_SEL_1 : 3;
                uint8_t LANE_SEL_2 : 3;
                uint8_t LANE_SEL_3 : 3;
                uint8_t LANE_SEL_4 : 3;
                uint8_t LANE_SEL_5 : 3;
                uint8_t LANE_SEL_6 : 3;
                uint8_t LANE_SEL_7 : 3;
                uint8_t OP : 7;
                uint16_t SRC0 : 9;
                uint8_t VDST : 8;
                uint8_t VSRC0 : 8;
            };
            struct layout_VOP1_VOP_DPP16 {
                uint8_t BANK_MASK : 4;
                uint8_t BOUND_CTRL : 1;
                uint16_t DPP_CTRL : 9;
                uint8_t ENCODING : 7;
                uint8_t FI : 1;
                uint8_t OP : 7;
                uint8_t ROW_MASK : 4;
                uint16_t SRC0 : 9;
                uint8_t SRC0_ABS : 1;
                uint8_t SRC0_NEG : 1;
                uint8_t SRC1_ABS : 1;
                uint8_t SRC1_NEG : 1;
                uint8_t VDST : 8;
                uint8_t VSRC0 : 8;
            };
            struct layout_VOP3_INST_LITERAL {
                uint8_t ABS : 3;
                uint8_t CLAMP : 1;
                uint8_t ENCODING : 6;
                uint32_t LITERAL : 32;
                uint8_t NEG : 3;
                uint8_t OMOD : 2;
                uint16_t OP : 10;
                uint8_t OPSEL : 4;
                uint16_t SRC0 : 9;
                uint16_t SRC1 : 9;
                uint16_t SRC2 : 9;
                uint8_t VDST : 8;
            };
            struct layout_VOP3_VOP_DPP8 {
                uint8_t ABS : 3;
                uint8_t CLAMP : 1;
                uint8_t ENCODING : 6;
                uint8_t LANE_SEL_0 : 3;
                uint8_t LANE_SEL_1 : 3;
                uint8_t LANE_SEL_2 : 3;
                uint8_t LANE_SEL_3 : 3;
                uint8_t LANE_SEL_4 : 3;
                uint8_t LANE_SEL_5 : 3;
                uint8_t LANE_SEL_6 : 3;
                uint8_t LANE_SEL_7 : 3;
                uint8_t NEG : 3;
                uint8_t OMOD : 2;
                uint16_t OP : 10;
                uint8_t OPSEL : 4;
                uint16_t SRC0 : 9;
                uint16_t SRC1 : 9;
                uint16_t SRC2 : 9;
                uint8_t VDST : 8;
                uint8_t VSRC0 : 8;
            };
            struct layout_VOP3_VOP_DPP16 {
                uint8_t ABS : 3;
                uint8_t BANK_MASK : 4;
                uint8_t BOUND_CTRL : 1;
                uint8_t CLAMP : 1;
                uint16_t DPP_CTRL : 9;
                uint8_t ENCODING : 6;
                uint8_t FI : 1;
                uint8_t NEG : 3;
                uint8_t OMOD : 2;
                uint16_t OP : 10;
                uint8_t OPSEL : 4;
                uint8_t ROW_MASK : 4;
                uint16_t SRC0 : 9;
                uint8_t SRC0_ABS : 1;
                uint8_t SRC0_NEG : 1;
                uint16_t SRC1 : 9;
                uint8_t SRC1_ABS : 1;
                uint8_t SRC1_NEG : 1;
                uint16_t SRC2 : 9;
                uint8_t VDST : 8;
                uint8_t VSRC0 : 8;
            };
            struct layout_VOP2_INST_LITERAL {
                uint8_t ENCODING : 1;
                uint32_t LITERAL : 32;
                uint8_t OP : 6;
                uint16_t SRC0 : 9;
                uint8_t VDST : 8;
                uint8_t VSRC1 : 8;
            };
            struct layout_VOP2_INST_LITERAL64 {
                uint8_t ENCODING : 1;
                uint64_t LITERAL64 : 64;
                uint8_t OP : 6;
                uint16_t SRC0 : 9;
                uint8_t VDST : 8;
                uint8_t VSRC1 : 8;
            };
            struct layout_VOP2_VOP_DPP8 {
                uint8_t ENCODING : 1;
                uint8_t LANE_SEL_0 : 3;
                uint8_t LANE_SEL_1 : 3;
                uint8_t LANE_SEL_2 : 3;
                uint8_t LANE_SEL_3 : 3;
                uint8_t LANE_SEL_4 : 3;
                uint8_t LANE_SEL_5 : 3;
                uint8_t LANE_SEL_6 : 3;
                uint8_t LANE_SEL_7 : 3;
                uint8_t OP : 6;
                uint16_t SRC0 : 9;
                uint8_t VDST : 8;
                uint8_t VSRC0 : 8;
                uint8_t VSRC1 : 8;
            };
            struct layout_VOP2_VOP_DPP16 {
                uint8_t BANK_MASK : 4;
                uint8_t BOUND_CTRL : 1;
                uint16_t DPP_CTRL : 9;
                uint8_t ENCODING : 1;
                uint8_t FI : 1;
                uint8_t OP : 6;
                uint8_t ROW_MASK : 4;
                uint16_t SRC0 : 9;
                uint8_t SRC0_ABS : 1;
                uint8_t SRC0_NEG : 1;
                uint8_t SRC1_ABS : 1;
                uint8_t SRC1_NEG : 1;
                uint8_t VDST : 8;
                uint8_t VSRC0 : 8;
                uint8_t VSRC1 : 8;
            };
            struct layout_VOP3_SDST_ENC {
                uint8_t CLAMP : 1;
                uint8_t ENCODING : 6;
                uint8_t NEG : 3;
                uint8_t OMOD : 2;
                uint16_t OP : 10;
                uint8_t SDST : 7;
                uint16_t SRC0 : 9;
                uint16_t SRC1 : 9;
                uint16_t SRC2 : 9;
                uint8_t VDST : 8;
            };
            struct layout_VOP3_SDST_ENC_INST_LITERAL {
                uint8_t CLAMP : 1;
                uint8_t ENCODING : 6;
                uint32_t LITERAL : 32;
                uint8_t NEG : 3;
                uint8_t OMOD : 2;
                uint16_t OP : 10;
                uint8_t SDST : 7;
                uint16_t SRC0 : 9;
                uint16_t SRC1 : 9;
                uint16_t SRC2 : 9;
                uint8_t VDST : 8;
            };
            struct layout_VOP3_SDST_ENC_VOP_DPP8 {
                uint8_t CLAMP : 1;
                uint8_t ENCODING : 6;
                uint8_t LANE_SEL_0 : 3;
                uint8_t LANE_SEL_1 : 3;
                uint8_t LANE_SEL_2 : 3;
                uint8_t LANE_SEL_3 : 3;
                uint8_t LANE_SEL_4 : 3;
                uint8_t LANE_SEL_5 : 3;
                uint8_t LANE_SEL_6 : 3;
                uint8_t LANE_SEL_7 : 3;
                uint8_t NEG : 3;
                uint8_t OMOD : 2;
                uint16_t OP : 10;
                uint8_t SDST : 7;
                uint16_t SRC0 : 9;
                uint16_t SRC1 : 9;
                uint16_t SRC2 : 9;
                uint8_t VDST : 8;
                uint8_t VSRC0 : 8;
            };
            struct layout_VOP3_SDST_ENC_VOP_DPP16 {
                uint8_t BANK_MASK : 4;
                uint8_t BOUND_CTRL : 1;
                uint8_t CLAMP : 1;
                uint16_t DPP_CTRL : 9;
                uint8_t ENCODING : 6;
                uint8_t FI : 1;
                uint8_t NEG : 3;
                uint8_t OMOD : 2;
                uint16_t OP : 10;
                uint8_t ROW_MASK : 4;
                uint8_t SDST : 7;
                uint16_t SRC0 : 9;
                uint8_t SRC0_ABS : 1;
                uint8_t SRC0_NEG : 1;
                uint16_t SRC1 : 9;
                uint8_t SRC1_ABS : 1;
                uint8_t SRC1_NEG : 1;
                uint16_t SRC2 : 9;
                uint8_t VDST : 8;
                uint8_t VSRC0 : 8;
            };
            struct layout_VOP3P_INST_LITERAL {
                uint8_t CLAMP : 1;
                uint8_t ENCODING : 8;
                uint32_t LITERAL : 32;
                uint8_t NEG : 3;
                uint8_t NEG_HI : 3;
                uint8_t OP : 8;
                uint8_t OPSEL : 3;
                uint8_t OPSEL_HI : 3;
                uint16_t SRC0 : 9;
                uint16_t SRC1 : 9;
                uint16_t SRC2 : 9;
                uint8_t VDST : 8;
            };
            struct layout_VOP3P_VOP_DPP8 {
                uint8_t CLAMP : 1;
                uint8_t ENCODING : 8;
                uint8_t LANE_SEL_0 : 3;
                uint8_t LANE_SEL_1 : 3;
                uint8_t LANE_SEL_2 : 3;
                uint8_t LANE_SEL_3 : 3;
                uint8_t LANE_SEL_4 : 3;
                uint8_t LANE_SEL_5 : 3;
                uint8_t LANE_SEL_6 : 3;
                uint8_t LANE_SEL_7 : 3;
                uint8_t NEG : 3;
                uint8_t NEG_HI : 3;
                uint8_t OP : 8;
                uint8_t OPSEL : 3;
                uint8_t OPSEL_HI : 3;
                uint16_t SRC0 : 9;
                uint16_t SRC1 : 9;
                uint16_t SRC2 : 9;
                uint8_t VDST : 8;
                uint8_t VSRC0 : 8;
            };
            struct layout_VOP3P_VOP_DPP16 {
                uint8_t BANK_MASK : 4;
                uint8_t BOUND_CTRL : 1;
                uint8_t CLAMP : 1;
                uint16_t DPP_CTRL : 9;
                uint8_t ENCODING : 8;
                uint8_t FI : 1;
                uint8_t NEG : 3;
                uint8_t NEG_HI : 3;
                uint8_t OP : 8;
                uint8_t OPSEL : 3;
                uint8_t OPSEL_HI : 3;
                uint8_t ROW_MASK : 4;
                uint16_t SRC0 : 9;
                uint8_t SRC0_ABS : 1;
                uint8_t SRC0_NEG : 1;
                uint16_t SRC1 : 9;
                uint8_t SRC1_ABS : 1;
                uint8_t SRC1_NEG : 1;
                uint16_t SRC2 : 9;
                uint8_t VDST : 8;
                uint8_t VSRC0 : 8;
            };
            struct layout_VOPC_INST_LITERAL {
                uint8_t ENCODING : 7;
                uint32_t LITERAL : 32;
                uint8_t OP : 8;
                uint16_t SRC0 : 9;
                uint8_t VSRC1 : 8;
            };
            struct layout_VOPC_INST_LITERAL64 {
                uint8_t ENCODING : 7;
                uint64_t LITERAL64 : 64;
                uint8_t OP : 8;
                uint16_t SRC0 : 9;
                uint8_t VSRC1 : 8;
            };
            struct layout_VOPC_VOP_DPP8 {
                uint8_t ENCODING : 7;
                uint8_t LANE_SEL_0 : 3;
                uint8_t LANE_SEL_1 : 3;
                uint8_t LANE_SEL_2 : 3;
                uint8_t LANE_SEL_3 : 3;
                uint8_t LANE_SEL_4 : 3;
                uint8_t LANE_SEL_5 : 3;
                uint8_t LANE_SEL_6 : 3;
                uint8_t LANE_SEL_7 : 3;
                uint8_t OP : 8;
                uint16_t SRC0 : 9;
                uint8_t VSRC0 : 8;
                uint8_t VSRC1 : 8;
            };
            struct layout_VOPC_VOP_DPP16 {
                uint8_t BANK_MASK : 4;
                uint8_t BOUND_CTRL : 1;
                uint16_t DPP_CTRL : 9;
                uint8_t ENCODING : 7;
                uint8_t FI : 1;
                uint8_t OP : 8;
                uint8_t ROW_MASK : 4;
                uint16_t SRC0 : 9;
                uint8_t SRC0_ABS : 1;
                uint8_t SRC0_NEG : 1;
                uint8_t SRC1_ABS : 1;
                uint8_t SRC1_NEG : 1;
                uint8_t VSRC0 : 8;
                uint8_t VSRC1 : 8;
            };
            struct layout_VOPD3XY_X {
                uint8_t ENCODING : 8;
                uint8_t NEGX : 3;
                uint8_t NEGY : 3;
                uint8_t OPX : 6;
                uint8_t OPY : 6;
                uint16_t SRCX0 : 9;
                uint16_t SRCY0 : 9;
                uint8_t VDSTX : 8;
                uint8_t VDSTY : 8;
                uint8_t VSRCX1 : 8;
                uint8_t VSRCX2 : 8;
                uint8_t VSRCY1 : 8;
                uint8_t VSRCY2 : 8;
            };
            struct layout_VOPD3XY_Y {
                uint8_t ENCODING : 8;
                uint8_t NEGX : 3;
                uint8_t NEGY : 3;
                uint8_t OPX : 6;
                uint8_t OPY : 6;
                uint16_t SRCX0 : 9;
                uint16_t SRCY0 : 9;
                uint8_t VDSTX : 8;
                uint8_t VDSTY : 8;
                uint8_t VSRCX1 : 8;
                uint8_t VSRCX2 : 8;
                uint8_t VSRCY1 : 8;
                uint8_t VSRCY2 : 8;
            };
            struct layout_VOPDXY_X {
                uint8_t ENCODING : 6;
                uint8_t OPX : 4;
                uint8_t OPY : 5;
                uint16_t SRCX0 : 9;
                uint16_t SRCY0 : 9;
                uint8_t VDSTX : 8;
                uint8_t VDSTY : 8;
                uint8_t VSRCX1 : 8;
                uint8_t VSRCY1 : 8;
            };
            struct layout_VOPDXY_INST_LITERAL_X {
                uint8_t ENCODING : 6;
                uint32_t LITERAL : 32;
                uint8_t OPX : 4;
                uint8_t OPY : 5;
                uint16_t SRCX0 : 9;
                uint16_t SRCY0 : 9;
                uint8_t VDSTX : 8;
                uint8_t VDSTY : 8;
                uint8_t VSRCX1 : 8;
                uint8_t VSRCY1 : 8;
            };
            struct layout_VOPDXY_Y {
                uint8_t ENCODING : 6;
                uint8_t OPX : 4;
                uint8_t OPY : 5;
                uint16_t SRCX0 : 9;
                uint16_t SRCY0 : 9;
                uint8_t VDSTX : 8;
                uint8_t VDSTY : 8;
                uint8_t VSRCX1 : 8;
                uint8_t VSRCY1 : 8;
            };
            struct layout_VOPDXY_INST_LITERAL_Y {
                uint8_t ENCODING : 6;
                uint32_t LITERAL : 32;
                uint8_t OPX : 4;
                uint8_t OPY : 5;
                uint16_t SRCX0 : 9;
                uint16_t SRCY0 : 9;
                uint8_t VDSTX : 8;
                uint8_t VDSTY : 8;
                uint8_t VSRCX1 : 8;
                uint8_t VSRCY1 : 8;
            };
            union insn_layout{

                layout_ENC_SOP1 ENC_SOP1;
                layout_ENC_SOPC ENC_SOPC;
                layout_ENC_SOPP ENC_SOPP;
                layout_ENC_SOPK ENC_SOPK;
                layout_ENC_SOP2 ENC_SOP2;
                layout_ENC_SMEM ENC_SMEM;
                layout_ENC_VOP1 ENC_VOP1;
                layout_ENC_VOPC ENC_VOPC;
                layout_ENC_VOP2 ENC_VOP2;
                layout_ENC_VOP3 ENC_VOP3;
                layout_ENC_VOP3PX2 ENC_VOP3PX2;
                layout_ENC_VOP3PX3 ENC_VOP3PX3;
                layout_ENC_VOP3P ENC_VOP3P;
                layout_ENC_VDS ENC_VDS;
                layout_ENC_VBUFFER ENC_VBUFFER;
                layout_ENC_VIMAGE ENC_VIMAGE;
                layout_ENC_VFLAT ENC_VFLAT;
                layout_ENC_VSCRATCH ENC_VSCRATCH;
                layout_ENC_VGLOBAL ENC_VGLOBAL;
                layout_SOP1_INST_LITERAL SOP1_INST_LITERAL;
                layout_SOP1_INST_LITERAL64 SOP1_INST_LITERAL64;
                layout_SOP2_INST_LITERAL SOP2_INST_LITERAL;
                layout_SOP2_INST_LITERAL64 SOP2_INST_LITERAL64;
                layout_SOPC_INST_LITERAL SOPC_INST_LITERAL;
                layout_SOPC_INST_LITERAL64 SOPC_INST_LITERAL64;
                layout_SOPK_INST_LITERAL SOPK_INST_LITERAL;
                layout_VOP1_INST_LITERAL VOP1_INST_LITERAL;
                layout_VOP1_INST_LITERAL64 VOP1_INST_LITERAL64;
                layout_VOP1_VOP_DPP8 VOP1_VOP_DPP8;
                layout_VOP1_VOP_DPP16 VOP1_VOP_DPP16;
                layout_VOP3_INST_LITERAL VOP3_INST_LITERAL;
                layout_VOP3_VOP_DPP8 VOP3_VOP_DPP8;
                layout_VOP3_VOP_DPP16 VOP3_VOP_DPP16;
                layout_VOP2_INST_LITERAL VOP2_INST_LITERAL;
                layout_VOP2_INST_LITERAL64 VOP2_INST_LITERAL64;
                layout_VOP2_VOP_DPP8 VOP2_VOP_DPP8;
                layout_VOP2_VOP_DPP16 VOP2_VOP_DPP16;
                layout_VOP3_SDST_ENC VOP3_SDST_ENC;
                layout_VOP3_SDST_ENC_INST_LITERAL VOP3_SDST_ENC_INST_LITERAL;
                layout_VOP3_SDST_ENC_VOP_DPP8 VOP3_SDST_ENC_VOP_DPP8;
                layout_VOP3_SDST_ENC_VOP_DPP16 VOP3_SDST_ENC_VOP_DPP16;
                layout_VOP3P_INST_LITERAL VOP3P_INST_LITERAL;
                layout_VOP3P_VOP_DPP8 VOP3P_VOP_DPP8;
                layout_VOP3P_VOP_DPP16 VOP3P_VOP_DPP16;
                layout_VOPC_INST_LITERAL VOPC_INST_LITERAL;
                layout_VOPC_INST_LITERAL64 VOPC_INST_LITERAL64;
                layout_VOPC_VOP_DPP8 VOPC_VOP_DPP8;
                layout_VOPC_VOP_DPP16 VOPC_VOP_DPP16;
                layout_VOPD3XY_X VOPD3XY_X;
                layout_VOPD3XY_Y VOPD3XY_Y;
                layout_VOPDXY_X VOPDXY_X;
                layout_VOPDXY_INST_LITERAL_X VOPDXY_INST_LITERAL_X;
                layout_VOPDXY_Y VOPDXY_Y;
                layout_VOPDXY_INST_LITERAL_Y VOPDXY_INST_LITERAL_Y;
            }insn_layout;
            void decodeENC_SOP1();
            void finalizeENC_SOP1Operands();
            void decodeENC_SOPC();
            void finalizeENC_SOPCOperands();
            void decodeENC_SOPP();
            void finalizeENC_SOPPOperands();
            void decodeENC_SOPK();
            void finalizeENC_SOPKOperands();
            void decodeENC_SOP2();
            void finalizeENC_SOP2Operands();
            void decodeENC_SMEM();
            void finalizeENC_SMEMOperands();
            void decodeENC_VOP1();
            void finalizeENC_VOP1Operands();
            void decodeENC_VOPC();
            void finalizeENC_VOPCOperands();
            void decodeENC_VOP2();
            void finalizeENC_VOP2Operands();
            void decodeENC_VOP3();
            void finalizeENC_VOP3Operands();
            void decodeENC_VOP3PX2();
            void finalizeENC_VOP3PX2Operands();
            void decodeENC_VOP3PX3();
            void finalizeENC_VOP3PX3Operands();
            void decodeENC_VOP3P();
            void finalizeENC_VOP3POperands();
            void decodeENC_VDS();
            void finalizeENC_VDSOperands();
            void decodeENC_VBUFFER();
            void finalizeENC_VBUFFEROperands();
            void decodeENC_VIMAGE();
            void finalizeENC_VIMAGEOperands();
            void decodeENC_VFLAT();
            void finalizeENC_VFLATOperands();
            void decodeENC_VSCRATCH();
            void finalizeENC_VSCRATCHOperands();
            void decodeENC_VGLOBAL();
            void finalizeENC_VGLOBALOperands();
            void decodeSOP1_INST_LITERAL();
            void finalizeSOP1_INST_LITERALOperands();
            void decodeSOP1_INST_LITERAL64();
            void finalizeSOP1_INST_LITERAL64Operands();
            void decodeSOP2_INST_LITERAL();
            void finalizeSOP2_INST_LITERALOperands();
            void decodeSOP2_INST_LITERAL64();
            void finalizeSOP2_INST_LITERAL64Operands();
            void decodeSOPC_INST_LITERAL();
            void finalizeSOPC_INST_LITERALOperands();
            void decodeSOPC_INST_LITERAL64();
            void finalizeSOPC_INST_LITERAL64Operands();
            void decodeSOPK_INST_LITERAL();
            void finalizeSOPK_INST_LITERALOperands();
            void decodeVOP1_INST_LITERAL();
            void finalizeVOP1_INST_LITERALOperands();
            void decodeVOP1_INST_LITERAL64();
            void finalizeVOP1_INST_LITERAL64Operands();
            void decodeVOP1_VOP_DPP8();
            void finalizeVOP1_VOP_DPP8Operands();
            void decodeVOP1_VOP_DPP16();
            void finalizeVOP1_VOP_DPP16Operands();
            void decodeVOP3_INST_LITERAL();
            void finalizeVOP3_INST_LITERALOperands();
            void decodeVOP3_VOP_DPP8();
            void finalizeVOP3_VOP_DPP8Operands();
            void decodeVOP3_VOP_DPP16();
            void finalizeVOP3_VOP_DPP16Operands();
            void decodeVOP2_INST_LITERAL();
            void finalizeVOP2_INST_LITERALOperands();
            void decodeVOP2_INST_LITERAL64();
            void finalizeVOP2_INST_LITERAL64Operands();
            void decodeVOP2_VOP_DPP8();
            void finalizeVOP2_VOP_DPP8Operands();
            void decodeVOP2_VOP_DPP16();
            void finalizeVOP2_VOP_DPP16Operands();
            void decodeVOP3_SDST_ENC();
            void finalizeVOP3_SDST_ENCOperands();
            void decodeVOP3_SDST_ENC_INST_LITERAL();
            void finalizeVOP3_SDST_ENC_INST_LITERALOperands();
            void decodeVOP3_SDST_ENC_VOP_DPP8();
            void finalizeVOP3_SDST_ENC_VOP_DPP8Operands();
            void decodeVOP3_SDST_ENC_VOP_DPP16();
            void finalizeVOP3_SDST_ENC_VOP_DPP16Operands();
            void decodeVOP3P_INST_LITERAL();
            void finalizeVOP3P_INST_LITERALOperands();
            void decodeVOP3P_VOP_DPP8();
            void finalizeVOP3P_VOP_DPP8Operands();
            void decodeVOP3P_VOP_DPP16();
            void finalizeVOP3P_VOP_DPP16Operands();
            void decodeVOPC_INST_LITERAL();
            void finalizeVOPC_INST_LITERALOperands();
            void decodeVOPC_INST_LITERAL64();
            void finalizeVOPC_INST_LITERAL64Operands();
            void decodeVOPC_VOP_DPP8();
            void finalizeVOPC_VOP_DPP8Operands();
            void decodeVOPC_VOP_DPP16();
            void finalizeVOPC_VOP_DPP16Operands();
            void decodeVOPD3XY_X();
            void finalizeVOPD3XY_XOperands();
            void decodeHalfVOPD3XY_X();
            void decodeVOPD3XY_Y();
            void finalizeVOPD3XY_YOperands();
            void decodeHalfVOPD3XY_Y();
            void decodeVOPDXY_X();
            void finalizeVOPDXY_XOperands();
            void decodeHalfVOPDXY_X();
            void decodeVOPDXY_INST_LITERAL_X();
            void finalizeVOPDXY_INST_LITERAL_XOperands();
            void decodeHalfVOPDXY_INST_LITERAL_X();
            void decodeVOPDXY_Y();
            void finalizeVOPDXY_YOperands();
            void decodeHalfVOPDXY_Y();
            void decodeVOPDXY_INST_LITERAL_Y();
            void finalizeVOPDXY_INST_LITERAL_YOperands();
            void decodeHalfVOPDXY_INST_LITERAL_Y();
            bool TRIVIAL_ENC_SOP1(uint64_t I);
            bool FORCED_SOP2_INST_LITERAL(uint64_t I);
            bool FORCED_SOPK_INST_LITERAL(uint64_t I);
            bool FORCED_VOP2_INST_LITERAL(uint64_t I);
            bool FORCED_VOP2_INST_LITERAL64(uint64_t I);


            Expression::Ptr decodeOPR_DELAY_INSTID0(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_DELAY_INSTID1(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_DELAY_INSTSKIP(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_DSMEM(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_EXEC(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_GPUMEM(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_HWREG_ID(uint64_t input, uint32_t start, uint32_t end);
            Expression::Ptr decodeOPR_PC(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SDST(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SDST_EXEC(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SDST_M0(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SENDMSG_RTN_MSG(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SGPR(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SMEM_OFFSET_NOK(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SRC(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SRC_NOINLINE(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SRC_SIMPLE(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SRC_VGPR(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SRC_VGPR_OR_INLINE(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SREG(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SREG_M0(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SSRC(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SSRC_BARRIER_ID(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SSRC_LANESEL(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SSRC_SPECIAL_SCC(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_VCC(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_VERSION_VERSION(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_VGPR(uint64_t input, uint32_t output_vec_len = 1 );


            void appendOPR_CLAUSE(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_DELAY(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_HWREG(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_LABEL(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SENDMSG(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SENDMSG_RTN(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SET_VGPR_MSB(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SIMM16(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SIMM24(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SIMM32(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SIMM5(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SIMM64(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SIMM8(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SLEEP(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void processOPR_SMEM_OFFSET(layout_ENC_SMEM & layout  );

            void appendOPR_VERSION(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_WAIT_ALU(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_WAIT_MEM_DS(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_DSMEM(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_EXEC(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_GPUMEM(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_PC(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SDST(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SDST_EXEC(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SDST_M0(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SGPR(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SMEM_OFFSET_NOK(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SRC(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SRC_NOINLINE(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SRC_SIMPLE(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SRC_VGPR(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SRC_VGPR_OR_INLINE(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SREG(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SREG_M0(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SSRC(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SSRC_BARRIER_ID(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SSRC_LANESEL(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SSRC_SPECIAL_SCC(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_VCC(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_VGPR(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);



            struct amdgpu_gfx1250_insn_entry {
                entryID op;
                const char *mnemonic;
            };


            const amdgpu_gfx1250_insn_entry ENC_SMEM_insn_table[41] =
            {
                {amdgpu_gfx1250_op_S_LOAD_B32,"S_LOAD_B32"}, // 0
                {amdgpu_gfx1250_op_S_LOAD_B64,"S_LOAD_B64"}, // 1
                {amdgpu_gfx1250_op_S_LOAD_B128,"S_LOAD_B128"}, // 2
                {amdgpu_gfx1250_op_S_LOAD_B256,"S_LOAD_B256"}, // 3
                {amdgpu_gfx1250_op_S_LOAD_B512,"S_LOAD_B512"}, // 4
                {amdgpu_gfx1250_op_S_LOAD_B96,"S_LOAD_B96"}, // 5
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 6
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 7
                {amdgpu_gfx1250_op_S_LOAD_I8,"S_LOAD_I8"}, // 8
                {amdgpu_gfx1250_op_S_LOAD_U8,"S_LOAD_U8"}, // 9
                {amdgpu_gfx1250_op_S_LOAD_I16,"S_LOAD_I16"}, // 10
                {amdgpu_gfx1250_op_S_LOAD_U16,"S_LOAD_U16"}, // 11
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 12
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 13
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_S_BUFFER_LOAD_B32,"S_BUFFER_LOAD_B32"}, // 16
                {amdgpu_gfx1250_op_S_BUFFER_LOAD_B64,"S_BUFFER_LOAD_B64"}, // 17
                {amdgpu_gfx1250_op_S_BUFFER_LOAD_B128,"S_BUFFER_LOAD_B128"}, // 18
                {amdgpu_gfx1250_op_S_BUFFER_LOAD_B256,"S_BUFFER_LOAD_B256"}, // 19
                {amdgpu_gfx1250_op_S_BUFFER_LOAD_B512,"S_BUFFER_LOAD_B512"}, // 20
                {amdgpu_gfx1250_op_S_BUFFER_LOAD_B96,"S_BUFFER_LOAD_B96"}, // 21
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 22
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 23
                {amdgpu_gfx1250_op_S_BUFFER_LOAD_I8,"S_BUFFER_LOAD_I8"}, // 24
                {amdgpu_gfx1250_op_S_BUFFER_LOAD_U8,"S_BUFFER_LOAD_U8"}, // 25
                {amdgpu_gfx1250_op_S_BUFFER_LOAD_I16,"S_BUFFER_LOAD_I16"}, // 26
                {amdgpu_gfx1250_op_S_BUFFER_LOAD_U16,"S_BUFFER_LOAD_U16"}, // 27
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 28
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 29
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 30
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 31
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 32
                {amdgpu_gfx1250_op_S_DCACHE_INV,"S_DCACHE_INV"}, // 33
                {amdgpu_gfx1250_op_S_ATC_PROBE,"S_ATC_PROBE"}, // 34
                {amdgpu_gfx1250_op_S_ATC_PROBE_BUFFER,"S_ATC_PROBE_BUFFER"}, // 35
                {amdgpu_gfx1250_op_S_PREFETCH_INST,"S_PREFETCH_INST"}, // 36
                {amdgpu_gfx1250_op_S_PREFETCH_INST_PC_REL,"S_PREFETCH_INST_PC_REL"}, // 37
                {amdgpu_gfx1250_op_S_PREFETCH_DATA,"S_PREFETCH_DATA"}, // 38
                {amdgpu_gfx1250_op_S_BUFFER_PREFETCH_DATA,"S_BUFFER_PREFETCH_DATA"}, // 39
                {amdgpu_gfx1250_op_S_PREFETCH_DATA_PC_REL,"S_PREFETCH_DATA_PC_REL"}, // 40
            }; // end ENC_SMEM_insn_table
            const amdgpu_gfx1250_insn_entry ENC_SOP1_insn_table[111] =
            {
                {amdgpu_gfx1250_op_S_MOV_B32,"S_MOV_B32"}, // 0
                {amdgpu_gfx1250_op_S_MOV_B64,"S_MOV_B64"}, // 1
                {amdgpu_gfx1250_op_S_CMOV_B32,"S_CMOV_B32"}, // 2
                {amdgpu_gfx1250_op_S_CMOV_B64,"S_CMOV_B64"}, // 3
                {amdgpu_gfx1250_op_S_BREV_B32,"S_BREV_B32"}, // 4
                {amdgpu_gfx1250_op_S_BREV_B64,"S_BREV_B64"}, // 5
                {amdgpu_gfx1250_op_S_GET_SHADER_CYCLES_U64,"S_GET_SHADER_CYCLES_U64"}, // 6
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 7
                {amdgpu_gfx1250_op_S_CTZ_I32_B32,"S_CTZ_I32_B32"}, // 8
                {amdgpu_gfx1250_op_S_CTZ_I32_B64,"S_CTZ_I32_B64"}, // 9
                {amdgpu_gfx1250_op_S_CLZ_I32_U32,"S_CLZ_I32_U32"}, // 10
                {amdgpu_gfx1250_op_S_CLZ_I32_U64,"S_CLZ_I32_U64"}, // 11
                {amdgpu_gfx1250_op_S_CLS_I32,"S_CLS_I32"}, // 12
                {amdgpu_gfx1250_op_S_CLS_I32_I64,"S_CLS_I32_I64"}, // 13
                {amdgpu_gfx1250_op_S_SEXT_I32_I8,"S_SEXT_I32_I8"}, // 14
                {amdgpu_gfx1250_op_S_SEXT_I32_I16,"S_SEXT_I32_I16"}, // 15
                {amdgpu_gfx1250_op_S_BITSET0_B32,"S_BITSET0_B32"}, // 16
                {amdgpu_gfx1250_op_S_BITSET0_B64,"S_BITSET0_B64"}, // 17
                {amdgpu_gfx1250_op_S_BITSET1_B32,"S_BITSET1_B32"}, // 18
                {amdgpu_gfx1250_op_S_BITSET1_B64,"S_BITSET1_B64"}, // 19
                {amdgpu_gfx1250_op_S_BITREPLICATE_B64_B32,"S_BITREPLICATE_B64_B32"}, // 20
                {amdgpu_gfx1250_op_S_ABS_I32,"S_ABS_I32"}, // 21
                {amdgpu_gfx1250_op_S_BCNT0_I32_B32,"S_BCNT0_I32_B32"}, // 22
                {amdgpu_gfx1250_op_S_BCNT0_I32_B64,"S_BCNT0_I32_B64"}, // 23
                {amdgpu_gfx1250_op_S_BCNT1_I32_B32,"S_BCNT1_I32_B32"}, // 24
                {amdgpu_gfx1250_op_S_BCNT1_I32_B64,"S_BCNT1_I32_B64"}, // 25
                {amdgpu_gfx1250_op_S_QUADMASK_B32,"S_QUADMASK_B32"}, // 26
                {amdgpu_gfx1250_op_S_QUADMASK_B64,"S_QUADMASK_B64"}, // 27
                {amdgpu_gfx1250_op_S_WQM_B32,"S_WQM_B32"}, // 28
                {amdgpu_gfx1250_op_S_WQM_B64,"S_WQM_B64"}, // 29
                {amdgpu_gfx1250_op_S_NOT_B32,"S_NOT_B32"}, // 30
                {amdgpu_gfx1250_op_S_NOT_B64,"S_NOT_B64"}, // 31
                {amdgpu_gfx1250_op_S_AND_SAVEEXEC_B32,"S_AND_SAVEEXEC_B32"}, // 32
                {amdgpu_gfx1250_op_S_AND_SAVEEXEC_B64,"S_AND_SAVEEXEC_B64"}, // 33
                {amdgpu_gfx1250_op_S_OR_SAVEEXEC_B32,"S_OR_SAVEEXEC_B32"}, // 34
                {amdgpu_gfx1250_op_S_OR_SAVEEXEC_B64,"S_OR_SAVEEXEC_B64"}, // 35
                {amdgpu_gfx1250_op_S_XOR_SAVEEXEC_B32,"S_XOR_SAVEEXEC_B32"}, // 36
                {amdgpu_gfx1250_op_S_XOR_SAVEEXEC_B64,"S_XOR_SAVEEXEC_B64"}, // 37
                {amdgpu_gfx1250_op_S_NAND_SAVEEXEC_B32,"S_NAND_SAVEEXEC_B32"}, // 38
                {amdgpu_gfx1250_op_S_NAND_SAVEEXEC_B64,"S_NAND_SAVEEXEC_B64"}, // 39
                {amdgpu_gfx1250_op_S_NOR_SAVEEXEC_B32,"S_NOR_SAVEEXEC_B32"}, // 40
                {amdgpu_gfx1250_op_S_NOR_SAVEEXEC_B64,"S_NOR_SAVEEXEC_B64"}, // 41
                {amdgpu_gfx1250_op_S_XNOR_SAVEEXEC_B32,"S_XNOR_SAVEEXEC_B32"}, // 42
                {amdgpu_gfx1250_op_S_XNOR_SAVEEXEC_B64,"S_XNOR_SAVEEXEC_B64"}, // 43
                {amdgpu_gfx1250_op_S_AND_NOT0_SAVEEXEC_B32,"S_AND_NOT0_SAVEEXEC_B32"}, // 44
                {amdgpu_gfx1250_op_S_AND_NOT0_SAVEEXEC_B64,"S_AND_NOT0_SAVEEXEC_B64"}, // 45
                {amdgpu_gfx1250_op_S_OR_NOT0_SAVEEXEC_B32,"S_OR_NOT0_SAVEEXEC_B32"}, // 46
                {amdgpu_gfx1250_op_S_OR_NOT0_SAVEEXEC_B64,"S_OR_NOT0_SAVEEXEC_B64"}, // 47
                {amdgpu_gfx1250_op_S_AND_NOT1_SAVEEXEC_B32,"S_AND_NOT1_SAVEEXEC_B32"}, // 48
                {amdgpu_gfx1250_op_S_AND_NOT1_SAVEEXEC_B64,"S_AND_NOT1_SAVEEXEC_B64"}, // 49
                {amdgpu_gfx1250_op_S_OR_NOT1_SAVEEXEC_B32,"S_OR_NOT1_SAVEEXEC_B32"}, // 50
                {amdgpu_gfx1250_op_S_OR_NOT1_SAVEEXEC_B64,"S_OR_NOT1_SAVEEXEC_B64"}, // 51
                {amdgpu_gfx1250_op_S_AND_NOT0_WREXEC_B32,"S_AND_NOT0_WREXEC_B32"}, // 52
                {amdgpu_gfx1250_op_S_AND_NOT0_WREXEC_B64,"S_AND_NOT0_WREXEC_B64"}, // 53
                {amdgpu_gfx1250_op_S_AND_NOT1_WREXEC_B32,"S_AND_NOT1_WREXEC_B32"}, // 54
                {amdgpu_gfx1250_op_S_AND_NOT1_WREXEC_B64,"S_AND_NOT1_WREXEC_B64"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 59
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 60
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 61
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_S_MOVRELS_B32,"S_MOVRELS_B32"}, // 64
                {amdgpu_gfx1250_op_S_MOVRELS_B64,"S_MOVRELS_B64"}, // 65
                {amdgpu_gfx1250_op_S_MOVRELD_B32,"S_MOVRELD_B32"}, // 66
                {amdgpu_gfx1250_op_S_MOVRELD_B64,"S_MOVRELD_B64"}, // 67
                {amdgpu_gfx1250_op_S_MOVRELSD_2_B32,"S_MOVRELSD_2_B32"}, // 68
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 69
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 70
                {amdgpu_gfx1250_op_S_GET_PC_I64,"S_GET_PC_I64"}, // 71
                {amdgpu_gfx1250_op_S_SET_PC_I64,"S_SET_PC_I64"}, // 72
                {amdgpu_gfx1250_op_S_SWAP_PC_I64,"S_SWAP_PC_I64"}, // 73
                {amdgpu_gfx1250_op_S_RFE_I64,"S_RFE_I64"}, // 74
                {amdgpu_gfx1250_op_S_ADD_PC_I64,"S_ADD_PC_I64"}, // 75
                {amdgpu_gfx1250_op_S_SENDMSG_RTN_B32,"S_SENDMSG_RTN_B32"}, // 76
                {amdgpu_gfx1250_op_S_SENDMSG_RTN_B64,"S_SENDMSG_RTN_B64"}, // 77
                {amdgpu_gfx1250_op_S_BARRIER_SIGNAL,"S_BARRIER_SIGNAL"}, // 78
                {amdgpu_gfx1250_op_S_BARRIER_SIGNAL_ISFIRST,"S_BARRIER_SIGNAL_ISFIRST"}, // 79
                {amdgpu_gfx1250_op_S_GET_BARRIER_STATE,"S_GET_BARRIER_STATE"}, // 80
                {amdgpu_gfx1250_op_S_BARRIER_INIT,"S_BARRIER_INIT"}, // 81
                {amdgpu_gfx1250_op_S_BARRIER_JOIN,"S_BARRIER_JOIN"}, // 82
                {amdgpu_gfx1250_op_S_ALLOC_VGPR,"S_ALLOC_VGPR"}, // 83
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 84
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 85
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 86
                {amdgpu_gfx1250_op_S_WAKEUP_BARRIER,"S_WAKEUP_BARRIER"}, // 87
                {amdgpu_gfx1250_op_S_SLEEP_VAR,"S_SLEEP_VAR"}, // 88
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 89
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 90
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 91
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 92
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 93
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 94
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 95
                {amdgpu_gfx1250_op_S_CEIL_F32,"S_CEIL_F32"}, // 96
                {amdgpu_gfx1250_op_S_FLOOR_F32,"S_FLOOR_F32"}, // 97
                {amdgpu_gfx1250_op_S_TRUNC_F32,"S_TRUNC_F32"}, // 98
                {amdgpu_gfx1250_op_S_RNDNE_F32,"S_RNDNE_F32"}, // 99
                {amdgpu_gfx1250_op_S_CVT_F32_I32,"S_CVT_F32_I32"}, // 100
                {amdgpu_gfx1250_op_S_CVT_F32_U32,"S_CVT_F32_U32"}, // 101
                {amdgpu_gfx1250_op_S_CVT_I32_F32,"S_CVT_I32_F32"}, // 102
                {amdgpu_gfx1250_op_S_CVT_U32_F32,"S_CVT_U32_F32"}, // 103
                {amdgpu_gfx1250_op_S_CVT_F16_F32,"S_CVT_F16_F32"}, // 104
                {amdgpu_gfx1250_op_S_CVT_F32_F16,"S_CVT_F32_F16"}, // 105
                {amdgpu_gfx1250_op_S_CVT_HI_F32_F16,"S_CVT_HI_F32_F16"}, // 106
                {amdgpu_gfx1250_op_S_CEIL_F16,"S_CEIL_F16"}, // 107
                {amdgpu_gfx1250_op_S_FLOOR_F16,"S_FLOOR_F16"}, // 108
                {amdgpu_gfx1250_op_S_TRUNC_F16,"S_TRUNC_F16"}, // 109
                {amdgpu_gfx1250_op_S_RNDNE_F16,"S_RNDNE_F16"}, // 110
            }; // end ENC_SOP1_insn_table
            const amdgpu_gfx1250_insn_entry SOP1_INST_LITERAL_insn_table[111] =
            {
                {amdgpu_gfx1250_op_S_MOV_B32,"S_MOV_B32"}, // 0
                {amdgpu_gfx1250_op_S_MOV_B64,"S_MOV_B64"}, // 1
                {amdgpu_gfx1250_op_S_CMOV_B32,"S_CMOV_B32"}, // 2
                {amdgpu_gfx1250_op_S_CMOV_B64,"S_CMOV_B64"}, // 3
                {amdgpu_gfx1250_op_S_BREV_B32,"S_BREV_B32"}, // 4
                {amdgpu_gfx1250_op_S_BREV_B64,"S_BREV_B64"}, // 5
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 6
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 7
                {amdgpu_gfx1250_op_S_CTZ_I32_B32,"S_CTZ_I32_B32"}, // 8
                {amdgpu_gfx1250_op_S_CTZ_I32_B64,"S_CTZ_I32_B64"}, // 9
                {amdgpu_gfx1250_op_S_CLZ_I32_U32,"S_CLZ_I32_U32"}, // 10
                {amdgpu_gfx1250_op_S_CLZ_I32_U64,"S_CLZ_I32_U64"}, // 11
                {amdgpu_gfx1250_op_S_CLS_I32,"S_CLS_I32"}, // 12
                {amdgpu_gfx1250_op_S_CLS_I32_I64,"S_CLS_I32_I64"}, // 13
                {amdgpu_gfx1250_op_S_SEXT_I32_I8,"S_SEXT_I32_I8"}, // 14
                {amdgpu_gfx1250_op_S_SEXT_I32_I16,"S_SEXT_I32_I16"}, // 15
                {amdgpu_gfx1250_op_S_BITSET0_B32,"S_BITSET0_B32"}, // 16
                {amdgpu_gfx1250_op_S_BITSET0_B64,"S_BITSET0_B64"}, // 17
                {amdgpu_gfx1250_op_S_BITSET1_B32,"S_BITSET1_B32"}, // 18
                {amdgpu_gfx1250_op_S_BITSET1_B64,"S_BITSET1_B64"}, // 19
                {amdgpu_gfx1250_op_S_BITREPLICATE_B64_B32,"S_BITREPLICATE_B64_B32"}, // 20
                {amdgpu_gfx1250_op_S_ABS_I32,"S_ABS_I32"}, // 21
                {amdgpu_gfx1250_op_S_BCNT0_I32_B32,"S_BCNT0_I32_B32"}, // 22
                {amdgpu_gfx1250_op_S_BCNT0_I32_B64,"S_BCNT0_I32_B64"}, // 23
                {amdgpu_gfx1250_op_S_BCNT1_I32_B32,"S_BCNT1_I32_B32"}, // 24
                {amdgpu_gfx1250_op_S_BCNT1_I32_B64,"S_BCNT1_I32_B64"}, // 25
                {amdgpu_gfx1250_op_S_QUADMASK_B32,"S_QUADMASK_B32"}, // 26
                {amdgpu_gfx1250_op_S_QUADMASK_B64,"S_QUADMASK_B64"}, // 27
                {amdgpu_gfx1250_op_S_WQM_B32,"S_WQM_B32"}, // 28
                {amdgpu_gfx1250_op_S_WQM_B64,"S_WQM_B64"}, // 29
                {amdgpu_gfx1250_op_S_NOT_B32,"S_NOT_B32"}, // 30
                {amdgpu_gfx1250_op_S_NOT_B64,"S_NOT_B64"}, // 31
                {amdgpu_gfx1250_op_S_AND_SAVEEXEC_B32,"S_AND_SAVEEXEC_B32"}, // 32
                {amdgpu_gfx1250_op_S_AND_SAVEEXEC_B64,"S_AND_SAVEEXEC_B64"}, // 33
                {amdgpu_gfx1250_op_S_OR_SAVEEXEC_B32,"S_OR_SAVEEXEC_B32"}, // 34
                {amdgpu_gfx1250_op_S_OR_SAVEEXEC_B64,"S_OR_SAVEEXEC_B64"}, // 35
                {amdgpu_gfx1250_op_S_XOR_SAVEEXEC_B32,"S_XOR_SAVEEXEC_B32"}, // 36
                {amdgpu_gfx1250_op_S_XOR_SAVEEXEC_B64,"S_XOR_SAVEEXEC_B64"}, // 37
                {amdgpu_gfx1250_op_S_NAND_SAVEEXEC_B32,"S_NAND_SAVEEXEC_B32"}, // 38
                {amdgpu_gfx1250_op_S_NAND_SAVEEXEC_B64,"S_NAND_SAVEEXEC_B64"}, // 39
                {amdgpu_gfx1250_op_S_NOR_SAVEEXEC_B32,"S_NOR_SAVEEXEC_B32"}, // 40
                {amdgpu_gfx1250_op_S_NOR_SAVEEXEC_B64,"S_NOR_SAVEEXEC_B64"}, // 41
                {amdgpu_gfx1250_op_S_XNOR_SAVEEXEC_B32,"S_XNOR_SAVEEXEC_B32"}, // 42
                {amdgpu_gfx1250_op_S_XNOR_SAVEEXEC_B64,"S_XNOR_SAVEEXEC_B64"}, // 43
                {amdgpu_gfx1250_op_S_AND_NOT0_SAVEEXEC_B32,"S_AND_NOT0_SAVEEXEC_B32"}, // 44
                {amdgpu_gfx1250_op_S_AND_NOT0_SAVEEXEC_B64,"S_AND_NOT0_SAVEEXEC_B64"}, // 45
                {amdgpu_gfx1250_op_S_OR_NOT0_SAVEEXEC_B32,"S_OR_NOT0_SAVEEXEC_B32"}, // 46
                {amdgpu_gfx1250_op_S_OR_NOT0_SAVEEXEC_B64,"S_OR_NOT0_SAVEEXEC_B64"}, // 47
                {amdgpu_gfx1250_op_S_AND_NOT1_SAVEEXEC_B32,"S_AND_NOT1_SAVEEXEC_B32"}, // 48
                {amdgpu_gfx1250_op_S_AND_NOT1_SAVEEXEC_B64,"S_AND_NOT1_SAVEEXEC_B64"}, // 49
                {amdgpu_gfx1250_op_S_OR_NOT1_SAVEEXEC_B32,"S_OR_NOT1_SAVEEXEC_B32"}, // 50
                {amdgpu_gfx1250_op_S_OR_NOT1_SAVEEXEC_B64,"S_OR_NOT1_SAVEEXEC_B64"}, // 51
                {amdgpu_gfx1250_op_S_AND_NOT0_WREXEC_B32,"S_AND_NOT0_WREXEC_B32"}, // 52
                {amdgpu_gfx1250_op_S_AND_NOT0_WREXEC_B64,"S_AND_NOT0_WREXEC_B64"}, // 53
                {amdgpu_gfx1250_op_S_AND_NOT1_WREXEC_B32,"S_AND_NOT1_WREXEC_B32"}, // 54
                {amdgpu_gfx1250_op_S_AND_NOT1_WREXEC_B64,"S_AND_NOT1_WREXEC_B64"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 59
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 60
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 61
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 64
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 65
                {amdgpu_gfx1250_op_S_MOVRELD_B32,"S_MOVRELD_B32"}, // 66
                {amdgpu_gfx1250_op_S_MOVRELD_B64,"S_MOVRELD_B64"}, // 67
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 68
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 69
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 72
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 73
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 74
                {amdgpu_gfx1250_op_S_ADD_PC_I64,"S_ADD_PC_I64"}, // 75
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 76
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 77
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 80
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 81
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 82
                {amdgpu_gfx1250_op_S_ALLOC_VGPR,"S_ALLOC_VGPR"}, // 83
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 84
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 85
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 86
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 87
                {amdgpu_gfx1250_op_S_SLEEP_VAR,"S_SLEEP_VAR"}, // 88
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 89
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 90
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 91
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 92
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 93
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 94
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 95
                {amdgpu_gfx1250_op_S_CEIL_F32,"S_CEIL_F32"}, // 96
                {amdgpu_gfx1250_op_S_FLOOR_F32,"S_FLOOR_F32"}, // 97
                {amdgpu_gfx1250_op_S_TRUNC_F32,"S_TRUNC_F32"}, // 98
                {amdgpu_gfx1250_op_S_RNDNE_F32,"S_RNDNE_F32"}, // 99
                {amdgpu_gfx1250_op_S_CVT_F32_I32,"S_CVT_F32_I32"}, // 100
                {amdgpu_gfx1250_op_S_CVT_F32_U32,"S_CVT_F32_U32"}, // 101
                {amdgpu_gfx1250_op_S_CVT_I32_F32,"S_CVT_I32_F32"}, // 102
                {amdgpu_gfx1250_op_S_CVT_U32_F32,"S_CVT_U32_F32"}, // 103
                {amdgpu_gfx1250_op_S_CVT_F16_F32,"S_CVT_F16_F32"}, // 104
                {amdgpu_gfx1250_op_S_CVT_F32_F16,"S_CVT_F32_F16"}, // 105
                {amdgpu_gfx1250_op_S_CVT_HI_F32_F16,"S_CVT_HI_F32_F16"}, // 106
                {amdgpu_gfx1250_op_S_CEIL_F16,"S_CEIL_F16"}, // 107
                {amdgpu_gfx1250_op_S_FLOOR_F16,"S_FLOOR_F16"}, // 108
                {amdgpu_gfx1250_op_S_TRUNC_F16,"S_TRUNC_F16"}, // 109
                {amdgpu_gfx1250_op_S_RNDNE_F16,"S_RNDNE_F16"}, // 110
            }; // end SOP1_INST_LITERAL_insn_table
            const amdgpu_gfx1250_insn_entry SOP1_INST_LITERAL64_insn_table[111] =
            {
                {amdgpu_gfx1250_op_S_MOV_B32,"S_MOV_B32"}, // 0
                {amdgpu_gfx1250_op_S_MOV_B64,"S_MOV_B64"}, // 1
                {amdgpu_gfx1250_op_S_CMOV_B32,"S_CMOV_B32"}, // 2
                {amdgpu_gfx1250_op_S_CMOV_B64,"S_CMOV_B64"}, // 3
                {amdgpu_gfx1250_op_S_BREV_B32,"S_BREV_B32"}, // 4
                {amdgpu_gfx1250_op_S_BREV_B64,"S_BREV_B64"}, // 5
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 6
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 7
                {amdgpu_gfx1250_op_S_CTZ_I32_B32,"S_CTZ_I32_B32"}, // 8
                {amdgpu_gfx1250_op_S_CTZ_I32_B64,"S_CTZ_I32_B64"}, // 9
                {amdgpu_gfx1250_op_S_CLZ_I32_U32,"S_CLZ_I32_U32"}, // 10
                {amdgpu_gfx1250_op_S_CLZ_I32_U64,"S_CLZ_I32_U64"}, // 11
                {amdgpu_gfx1250_op_S_CLS_I32,"S_CLS_I32"}, // 12
                {amdgpu_gfx1250_op_S_CLS_I32_I64,"S_CLS_I32_I64"}, // 13
                {amdgpu_gfx1250_op_S_SEXT_I32_I8,"S_SEXT_I32_I8"}, // 14
                {amdgpu_gfx1250_op_S_SEXT_I32_I16,"S_SEXT_I32_I16"}, // 15
                {amdgpu_gfx1250_op_S_BITSET0_B32,"S_BITSET0_B32"}, // 16
                {amdgpu_gfx1250_op_S_BITSET0_B64,"S_BITSET0_B64"}, // 17
                {amdgpu_gfx1250_op_S_BITSET1_B32,"S_BITSET1_B32"}, // 18
                {amdgpu_gfx1250_op_S_BITSET1_B64,"S_BITSET1_B64"}, // 19
                {amdgpu_gfx1250_op_S_BITREPLICATE_B64_B32,"S_BITREPLICATE_B64_B32"}, // 20
                {amdgpu_gfx1250_op_S_ABS_I32,"S_ABS_I32"}, // 21
                {amdgpu_gfx1250_op_S_BCNT0_I32_B32,"S_BCNT0_I32_B32"}, // 22
                {amdgpu_gfx1250_op_S_BCNT0_I32_B64,"S_BCNT0_I32_B64"}, // 23
                {amdgpu_gfx1250_op_S_BCNT1_I32_B32,"S_BCNT1_I32_B32"}, // 24
                {amdgpu_gfx1250_op_S_BCNT1_I32_B64,"S_BCNT1_I32_B64"}, // 25
                {amdgpu_gfx1250_op_S_QUADMASK_B32,"S_QUADMASK_B32"}, // 26
                {amdgpu_gfx1250_op_S_QUADMASK_B64,"S_QUADMASK_B64"}, // 27
                {amdgpu_gfx1250_op_S_WQM_B32,"S_WQM_B32"}, // 28
                {amdgpu_gfx1250_op_S_WQM_B64,"S_WQM_B64"}, // 29
                {amdgpu_gfx1250_op_S_NOT_B32,"S_NOT_B32"}, // 30
                {amdgpu_gfx1250_op_S_NOT_B64,"S_NOT_B64"}, // 31
                {amdgpu_gfx1250_op_S_AND_SAVEEXEC_B32,"S_AND_SAVEEXEC_B32"}, // 32
                {amdgpu_gfx1250_op_S_AND_SAVEEXEC_B64,"S_AND_SAVEEXEC_B64"}, // 33
                {amdgpu_gfx1250_op_S_OR_SAVEEXEC_B32,"S_OR_SAVEEXEC_B32"}, // 34
                {amdgpu_gfx1250_op_S_OR_SAVEEXEC_B64,"S_OR_SAVEEXEC_B64"}, // 35
                {amdgpu_gfx1250_op_S_XOR_SAVEEXEC_B32,"S_XOR_SAVEEXEC_B32"}, // 36
                {amdgpu_gfx1250_op_S_XOR_SAVEEXEC_B64,"S_XOR_SAVEEXEC_B64"}, // 37
                {amdgpu_gfx1250_op_S_NAND_SAVEEXEC_B32,"S_NAND_SAVEEXEC_B32"}, // 38
                {amdgpu_gfx1250_op_S_NAND_SAVEEXEC_B64,"S_NAND_SAVEEXEC_B64"}, // 39
                {amdgpu_gfx1250_op_S_NOR_SAVEEXEC_B32,"S_NOR_SAVEEXEC_B32"}, // 40
                {amdgpu_gfx1250_op_S_NOR_SAVEEXEC_B64,"S_NOR_SAVEEXEC_B64"}, // 41
                {amdgpu_gfx1250_op_S_XNOR_SAVEEXEC_B32,"S_XNOR_SAVEEXEC_B32"}, // 42
                {amdgpu_gfx1250_op_S_XNOR_SAVEEXEC_B64,"S_XNOR_SAVEEXEC_B64"}, // 43
                {amdgpu_gfx1250_op_S_AND_NOT0_SAVEEXEC_B32,"S_AND_NOT0_SAVEEXEC_B32"}, // 44
                {amdgpu_gfx1250_op_S_AND_NOT0_SAVEEXEC_B64,"S_AND_NOT0_SAVEEXEC_B64"}, // 45
                {amdgpu_gfx1250_op_S_OR_NOT0_SAVEEXEC_B32,"S_OR_NOT0_SAVEEXEC_B32"}, // 46
                {amdgpu_gfx1250_op_S_OR_NOT0_SAVEEXEC_B64,"S_OR_NOT0_SAVEEXEC_B64"}, // 47
                {amdgpu_gfx1250_op_S_AND_NOT1_SAVEEXEC_B32,"S_AND_NOT1_SAVEEXEC_B32"}, // 48
                {amdgpu_gfx1250_op_S_AND_NOT1_SAVEEXEC_B64,"S_AND_NOT1_SAVEEXEC_B64"}, // 49
                {amdgpu_gfx1250_op_S_OR_NOT1_SAVEEXEC_B32,"S_OR_NOT1_SAVEEXEC_B32"}, // 50
                {amdgpu_gfx1250_op_S_OR_NOT1_SAVEEXEC_B64,"S_OR_NOT1_SAVEEXEC_B64"}, // 51
                {amdgpu_gfx1250_op_S_AND_NOT0_WREXEC_B32,"S_AND_NOT0_WREXEC_B32"}, // 52
                {amdgpu_gfx1250_op_S_AND_NOT0_WREXEC_B64,"S_AND_NOT0_WREXEC_B64"}, // 53
                {amdgpu_gfx1250_op_S_AND_NOT1_WREXEC_B32,"S_AND_NOT1_WREXEC_B32"}, // 54
                {amdgpu_gfx1250_op_S_AND_NOT1_WREXEC_B64,"S_AND_NOT1_WREXEC_B64"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 59
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 60
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 61
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 64
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 65
                {amdgpu_gfx1250_op_S_MOVRELD_B32,"S_MOVRELD_B32"}, // 66
                {amdgpu_gfx1250_op_S_MOVRELD_B64,"S_MOVRELD_B64"}, // 67
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 68
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 69
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 72
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 73
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 74
                {amdgpu_gfx1250_op_S_ADD_PC_I64,"S_ADD_PC_I64"}, // 75
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 76
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 77
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 80
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 81
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 82
                {amdgpu_gfx1250_op_S_ALLOC_VGPR,"S_ALLOC_VGPR"}, // 83
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 84
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 85
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 86
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 87
                {amdgpu_gfx1250_op_S_SLEEP_VAR,"S_SLEEP_VAR"}, // 88
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 89
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 90
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 91
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 92
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 93
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 94
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 95
                {amdgpu_gfx1250_op_S_CEIL_F32,"S_CEIL_F32"}, // 96
                {amdgpu_gfx1250_op_S_FLOOR_F32,"S_FLOOR_F32"}, // 97
                {amdgpu_gfx1250_op_S_TRUNC_F32,"S_TRUNC_F32"}, // 98
                {amdgpu_gfx1250_op_S_RNDNE_F32,"S_RNDNE_F32"}, // 99
                {amdgpu_gfx1250_op_S_CVT_F32_I32,"S_CVT_F32_I32"}, // 100
                {amdgpu_gfx1250_op_S_CVT_F32_U32,"S_CVT_F32_U32"}, // 101
                {amdgpu_gfx1250_op_S_CVT_I32_F32,"S_CVT_I32_F32"}, // 102
                {amdgpu_gfx1250_op_S_CVT_U32_F32,"S_CVT_U32_F32"}, // 103
                {amdgpu_gfx1250_op_S_CVT_F16_F32,"S_CVT_F16_F32"}, // 104
                {amdgpu_gfx1250_op_S_CVT_F32_F16,"S_CVT_F32_F16"}, // 105
                {amdgpu_gfx1250_op_S_CVT_HI_F32_F16,"S_CVT_HI_F32_F16"}, // 106
                {amdgpu_gfx1250_op_S_CEIL_F16,"S_CEIL_F16"}, // 107
                {amdgpu_gfx1250_op_S_FLOOR_F16,"S_FLOOR_F16"}, // 108
                {amdgpu_gfx1250_op_S_TRUNC_F16,"S_TRUNC_F16"}, // 109
                {amdgpu_gfx1250_op_S_RNDNE_F16,"S_RNDNE_F16"}, // 110
            }; // end SOP1_INST_LITERAL64_insn_table
            const amdgpu_gfx1250_insn_entry ENC_SOP2_insn_table[86] =
            {
                {amdgpu_gfx1250_op_S_ADD_CO_U32,"S_ADD_CO_U32"}, // 0
                {amdgpu_gfx1250_op_S_SUB_CO_U32,"S_SUB_CO_U32"}, // 1
                {amdgpu_gfx1250_op_S_ADD_CO_I32,"S_ADD_CO_I32"}, // 2
                {amdgpu_gfx1250_op_S_SUB_CO_I32,"S_SUB_CO_I32"}, // 3
                {amdgpu_gfx1250_op_S_ADD_CO_CI_U32,"S_ADD_CO_CI_U32"}, // 4
                {amdgpu_gfx1250_op_S_SUB_CO_CI_U32,"S_SUB_CO_CI_U32"}, // 5
                {amdgpu_gfx1250_op_S_ABSDIFF_I32,"S_ABSDIFF_I32"}, // 6
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 7
                {amdgpu_gfx1250_op_S_LSHL_B32,"S_LSHL_B32"}, // 8
                {amdgpu_gfx1250_op_S_LSHL_B64,"S_LSHL_B64"}, // 9
                {amdgpu_gfx1250_op_S_LSHR_B32,"S_LSHR_B32"}, // 10
                {amdgpu_gfx1250_op_S_LSHR_B64,"S_LSHR_B64"}, // 11
                {amdgpu_gfx1250_op_S_ASHR_I32,"S_ASHR_I32"}, // 12
                {amdgpu_gfx1250_op_S_ASHR_I64,"S_ASHR_I64"}, // 13
                {amdgpu_gfx1250_op_S_LSHL1_ADD_U32,"S_LSHL1_ADD_U32"}, // 14
                {amdgpu_gfx1250_op_S_LSHL2_ADD_U32,"S_LSHL2_ADD_U32"}, // 15
                {amdgpu_gfx1250_op_S_LSHL3_ADD_U32,"S_LSHL3_ADD_U32"}, // 16
                {amdgpu_gfx1250_op_S_LSHL4_ADD_U32,"S_LSHL4_ADD_U32"}, // 17
                {amdgpu_gfx1250_op_S_MIN_I32,"S_MIN_I32"}, // 18
                {amdgpu_gfx1250_op_S_MIN_U32,"S_MIN_U32"}, // 19
                {amdgpu_gfx1250_op_S_MAX_I32,"S_MAX_I32"}, // 20
                {amdgpu_gfx1250_op_S_MAX_U32,"S_MAX_U32"}, // 21
                {amdgpu_gfx1250_op_S_AND_B32,"S_AND_B32"}, // 22
                {amdgpu_gfx1250_op_S_AND_B64,"S_AND_B64"}, // 23
                {amdgpu_gfx1250_op_S_OR_B32,"S_OR_B32"}, // 24
                {amdgpu_gfx1250_op_S_OR_B64,"S_OR_B64"}, // 25
                {amdgpu_gfx1250_op_S_XOR_B32,"S_XOR_B32"}, // 26
                {amdgpu_gfx1250_op_S_XOR_B64,"S_XOR_B64"}, // 27
                {amdgpu_gfx1250_op_S_NAND_B32,"S_NAND_B32"}, // 28
                {amdgpu_gfx1250_op_S_NAND_B64,"S_NAND_B64"}, // 29
                {amdgpu_gfx1250_op_S_NOR_B32,"S_NOR_B32"}, // 30
                {amdgpu_gfx1250_op_S_NOR_B64,"S_NOR_B64"}, // 31
                {amdgpu_gfx1250_op_S_XNOR_B32,"S_XNOR_B32"}, // 32
                {amdgpu_gfx1250_op_S_XNOR_B64,"S_XNOR_B64"}, // 33
                {amdgpu_gfx1250_op_S_AND_NOT1_B32,"S_AND_NOT1_B32"}, // 34
                {amdgpu_gfx1250_op_S_AND_NOT1_B64,"S_AND_NOT1_B64"}, // 35
                {amdgpu_gfx1250_op_S_OR_NOT1_B32,"S_OR_NOT1_B32"}, // 36
                {amdgpu_gfx1250_op_S_OR_NOT1_B64,"S_OR_NOT1_B64"}, // 37
                {amdgpu_gfx1250_op_S_BFE_U32,"S_BFE_U32"}, // 38
                {amdgpu_gfx1250_op_S_BFE_I32,"S_BFE_I32"}, // 39
                {amdgpu_gfx1250_op_S_BFE_U64,"S_BFE_U64"}, // 40
                {amdgpu_gfx1250_op_S_BFE_I64,"S_BFE_I64"}, // 41
                {amdgpu_gfx1250_op_S_BFM_B32,"S_BFM_B32"}, // 42
                {amdgpu_gfx1250_op_S_BFM_B64,"S_BFM_B64"}, // 43
                {amdgpu_gfx1250_op_S_MUL_I32,"S_MUL_I32"}, // 44
                {amdgpu_gfx1250_op_S_MUL_HI_U32,"S_MUL_HI_U32"}, // 45
                {amdgpu_gfx1250_op_S_MUL_HI_I32,"S_MUL_HI_I32"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_S_CSELECT_B32,"S_CSELECT_B32"}, // 48
                {amdgpu_gfx1250_op_S_CSELECT_B64,"S_CSELECT_B64"}, // 49
                {amdgpu_gfx1250_op_S_PACK_LL_B32_B16,"S_PACK_LL_B32_B16"}, // 50
                {amdgpu_gfx1250_op_S_PACK_LH_B32_B16,"S_PACK_LH_B32_B16"}, // 51
                {amdgpu_gfx1250_op_S_PACK_HH_B32_B16,"S_PACK_HH_B32_B16"}, // 52
                {amdgpu_gfx1250_op_S_PACK_HL_B32_B16,"S_PACK_HL_B32_B16"}, // 53
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 59
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 60
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 61
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_S_ADD_F32,"S_ADD_F32"}, // 64
                {amdgpu_gfx1250_op_S_SUB_F32,"S_SUB_F32"}, // 65
                {amdgpu_gfx1250_op_S_MIN_NUM_F32,"S_MIN_NUM_F32"}, // 66
                {amdgpu_gfx1250_op_S_MAX_NUM_F32,"S_MAX_NUM_F32"}, // 67
                {amdgpu_gfx1250_op_S_MUL_F32,"S_MUL_F32"}, // 68
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 69
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 70
                {amdgpu_gfx1250_op_S_FMAC_F32,"S_FMAC_F32"}, // 71
                {amdgpu_gfx1250_op_S_CVT_PK_RTZ_F16_F32,"S_CVT_PK_RTZ_F16_F32"}, // 72
                {amdgpu_gfx1250_op_S_ADD_F16,"S_ADD_F16"}, // 73
                {amdgpu_gfx1250_op_S_SUB_F16,"S_SUB_F16"}, // 74
                {amdgpu_gfx1250_op_S_MIN_NUM_F16,"S_MIN_NUM_F16"}, // 75
                {amdgpu_gfx1250_op_S_MAX_NUM_F16,"S_MAX_NUM_F16"}, // 76
                {amdgpu_gfx1250_op_S_MUL_F16,"S_MUL_F16"}, // 77
                {amdgpu_gfx1250_op_S_FMAC_F16,"S_FMAC_F16"}, // 78
                {amdgpu_gfx1250_op_S_MINIMUM_F32,"S_MINIMUM_F32"}, // 79
                {amdgpu_gfx1250_op_S_MAXIMUM_F32,"S_MAXIMUM_F32"}, // 80
                {amdgpu_gfx1250_op_S_MINIMUM_F16,"S_MINIMUM_F16"}, // 81
                {amdgpu_gfx1250_op_S_MAXIMUM_F16,"S_MAXIMUM_F16"}, // 82
                {amdgpu_gfx1250_op_S_ADD_NC_U64,"S_ADD_NC_U64"}, // 83
                {amdgpu_gfx1250_op_S_SUB_NC_U64,"S_SUB_NC_U64"}, // 84
                {amdgpu_gfx1250_op_S_MUL_U64,"S_MUL_U64"}, // 85
            }; // end ENC_SOP2_insn_table
            const amdgpu_gfx1250_insn_entry SOP2_INST_LITERAL_insn_table[86] =
            {
                {amdgpu_gfx1250_op_S_ADD_CO_U32,"S_ADD_CO_U32"}, // 0
                {amdgpu_gfx1250_op_S_SUB_CO_U32,"S_SUB_CO_U32"}, // 1
                {amdgpu_gfx1250_op_S_ADD_CO_I32,"S_ADD_CO_I32"}, // 2
                {amdgpu_gfx1250_op_S_SUB_CO_I32,"S_SUB_CO_I32"}, // 3
                {amdgpu_gfx1250_op_S_ADD_CO_CI_U32,"S_ADD_CO_CI_U32"}, // 4
                {amdgpu_gfx1250_op_S_SUB_CO_CI_U32,"S_SUB_CO_CI_U32"}, // 5
                {amdgpu_gfx1250_op_S_ABSDIFF_I32,"S_ABSDIFF_I32"}, // 6
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 7
                {amdgpu_gfx1250_op_S_LSHL_B32,"S_LSHL_B32"}, // 8
                {amdgpu_gfx1250_op_S_LSHL_B64,"S_LSHL_B64"}, // 9
                {amdgpu_gfx1250_op_S_LSHR_B32,"S_LSHR_B32"}, // 10
                {amdgpu_gfx1250_op_S_LSHR_B64,"S_LSHR_B64"}, // 11
                {amdgpu_gfx1250_op_S_ASHR_I32,"S_ASHR_I32"}, // 12
                {amdgpu_gfx1250_op_S_ASHR_I64,"S_ASHR_I64"}, // 13
                {amdgpu_gfx1250_op_S_LSHL1_ADD_U32,"S_LSHL1_ADD_U32"}, // 14
                {amdgpu_gfx1250_op_S_LSHL2_ADD_U32,"S_LSHL2_ADD_U32"}, // 15
                {amdgpu_gfx1250_op_S_LSHL3_ADD_U32,"S_LSHL3_ADD_U32"}, // 16
                {amdgpu_gfx1250_op_S_LSHL4_ADD_U32,"S_LSHL4_ADD_U32"}, // 17
                {amdgpu_gfx1250_op_S_MIN_I32,"S_MIN_I32"}, // 18
                {amdgpu_gfx1250_op_S_MIN_U32,"S_MIN_U32"}, // 19
                {amdgpu_gfx1250_op_S_MAX_I32,"S_MAX_I32"}, // 20
                {amdgpu_gfx1250_op_S_MAX_U32,"S_MAX_U32"}, // 21
                {amdgpu_gfx1250_op_S_AND_B32,"S_AND_B32"}, // 22
                {amdgpu_gfx1250_op_S_AND_B64,"S_AND_B64"}, // 23
                {amdgpu_gfx1250_op_S_OR_B32,"S_OR_B32"}, // 24
                {amdgpu_gfx1250_op_S_OR_B64,"S_OR_B64"}, // 25
                {amdgpu_gfx1250_op_S_XOR_B32,"S_XOR_B32"}, // 26
                {amdgpu_gfx1250_op_S_XOR_B64,"S_XOR_B64"}, // 27
                {amdgpu_gfx1250_op_S_NAND_B32,"S_NAND_B32"}, // 28
                {amdgpu_gfx1250_op_S_NAND_B64,"S_NAND_B64"}, // 29
                {amdgpu_gfx1250_op_S_NOR_B32,"S_NOR_B32"}, // 30
                {amdgpu_gfx1250_op_S_NOR_B64,"S_NOR_B64"}, // 31
                {amdgpu_gfx1250_op_S_XNOR_B32,"S_XNOR_B32"}, // 32
                {amdgpu_gfx1250_op_S_XNOR_B64,"S_XNOR_B64"}, // 33
                {amdgpu_gfx1250_op_S_AND_NOT1_B32,"S_AND_NOT1_B32"}, // 34
                {amdgpu_gfx1250_op_S_AND_NOT1_B64,"S_AND_NOT1_B64"}, // 35
                {amdgpu_gfx1250_op_S_OR_NOT1_B32,"S_OR_NOT1_B32"}, // 36
                {amdgpu_gfx1250_op_S_OR_NOT1_B64,"S_OR_NOT1_B64"}, // 37
                {amdgpu_gfx1250_op_S_BFE_U32,"S_BFE_U32"}, // 38
                {amdgpu_gfx1250_op_S_BFE_I32,"S_BFE_I32"}, // 39
                {amdgpu_gfx1250_op_S_BFE_U64,"S_BFE_U64"}, // 40
                {amdgpu_gfx1250_op_S_BFE_I64,"S_BFE_I64"}, // 41
                {amdgpu_gfx1250_op_S_BFM_B32,"S_BFM_B32"}, // 42
                {amdgpu_gfx1250_op_S_BFM_B64,"S_BFM_B64"}, // 43
                {amdgpu_gfx1250_op_S_MUL_I32,"S_MUL_I32"}, // 44
                {amdgpu_gfx1250_op_S_MUL_HI_U32,"S_MUL_HI_U32"}, // 45
                {amdgpu_gfx1250_op_S_MUL_HI_I32,"S_MUL_HI_I32"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_S_CSELECT_B32,"S_CSELECT_B32"}, // 48
                {amdgpu_gfx1250_op_S_CSELECT_B64,"S_CSELECT_B64"}, // 49
                {amdgpu_gfx1250_op_S_PACK_LL_B32_B16,"S_PACK_LL_B32_B16"}, // 50
                {amdgpu_gfx1250_op_S_PACK_LH_B32_B16,"S_PACK_LH_B32_B16"}, // 51
                {amdgpu_gfx1250_op_S_PACK_HH_B32_B16,"S_PACK_HH_B32_B16"}, // 52
                {amdgpu_gfx1250_op_S_PACK_HL_B32_B16,"S_PACK_HL_B32_B16"}, // 53
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 59
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 60
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 61
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_S_ADD_F32,"S_ADD_F32"}, // 64
                {amdgpu_gfx1250_op_S_SUB_F32,"S_SUB_F32"}, // 65
                {amdgpu_gfx1250_op_S_MIN_NUM_F32,"S_MIN_NUM_F32"}, // 66
                {amdgpu_gfx1250_op_S_MAX_NUM_F32,"S_MAX_NUM_F32"}, // 67
                {amdgpu_gfx1250_op_S_MUL_F32,"S_MUL_F32"}, // 68
                {amdgpu_gfx1250_op_S_FMAAK_F32,"S_FMAAK_F32"}, // 69
                {amdgpu_gfx1250_op_S_FMAMK_F32,"S_FMAMK_F32"}, // 70
                {amdgpu_gfx1250_op_S_FMAC_F32,"S_FMAC_F32"}, // 71
                {amdgpu_gfx1250_op_S_CVT_PK_RTZ_F16_F32,"S_CVT_PK_RTZ_F16_F32"}, // 72
                {amdgpu_gfx1250_op_S_ADD_F16,"S_ADD_F16"}, // 73
                {amdgpu_gfx1250_op_S_SUB_F16,"S_SUB_F16"}, // 74
                {amdgpu_gfx1250_op_S_MIN_NUM_F16,"S_MIN_NUM_F16"}, // 75
                {amdgpu_gfx1250_op_S_MAX_NUM_F16,"S_MAX_NUM_F16"}, // 76
                {amdgpu_gfx1250_op_S_MUL_F16,"S_MUL_F16"}, // 77
                {amdgpu_gfx1250_op_S_FMAC_F16,"S_FMAC_F16"}, // 78
                {amdgpu_gfx1250_op_S_MINIMUM_F32,"S_MINIMUM_F32"}, // 79
                {amdgpu_gfx1250_op_S_MAXIMUM_F32,"S_MAXIMUM_F32"}, // 80
                {amdgpu_gfx1250_op_S_MINIMUM_F16,"S_MINIMUM_F16"}, // 81
                {amdgpu_gfx1250_op_S_MAXIMUM_F16,"S_MAXIMUM_F16"}, // 82
                {amdgpu_gfx1250_op_S_ADD_NC_U64,"S_ADD_NC_U64"}, // 83
                {amdgpu_gfx1250_op_S_SUB_NC_U64,"S_SUB_NC_U64"}, // 84
                {amdgpu_gfx1250_op_S_MUL_U64,"S_MUL_U64"}, // 85
            }; // end SOP2_INST_LITERAL_insn_table
            const amdgpu_gfx1250_insn_entry SOP2_INST_LITERAL64_insn_table[86] =
            {
                {amdgpu_gfx1250_op_S_ADD_CO_U32,"S_ADD_CO_U32"}, // 0
                {amdgpu_gfx1250_op_S_SUB_CO_U32,"S_SUB_CO_U32"}, // 1
                {amdgpu_gfx1250_op_S_ADD_CO_I32,"S_ADD_CO_I32"}, // 2
                {amdgpu_gfx1250_op_S_SUB_CO_I32,"S_SUB_CO_I32"}, // 3
                {amdgpu_gfx1250_op_S_ADD_CO_CI_U32,"S_ADD_CO_CI_U32"}, // 4
                {amdgpu_gfx1250_op_S_SUB_CO_CI_U32,"S_SUB_CO_CI_U32"}, // 5
                {amdgpu_gfx1250_op_S_ABSDIFF_I32,"S_ABSDIFF_I32"}, // 6
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 7
                {amdgpu_gfx1250_op_S_LSHL_B32,"S_LSHL_B32"}, // 8
                {amdgpu_gfx1250_op_S_LSHL_B64,"S_LSHL_B64"}, // 9
                {amdgpu_gfx1250_op_S_LSHR_B32,"S_LSHR_B32"}, // 10
                {amdgpu_gfx1250_op_S_LSHR_B64,"S_LSHR_B64"}, // 11
                {amdgpu_gfx1250_op_S_ASHR_I32,"S_ASHR_I32"}, // 12
                {amdgpu_gfx1250_op_S_ASHR_I64,"S_ASHR_I64"}, // 13
                {amdgpu_gfx1250_op_S_LSHL1_ADD_U32,"S_LSHL1_ADD_U32"}, // 14
                {amdgpu_gfx1250_op_S_LSHL2_ADD_U32,"S_LSHL2_ADD_U32"}, // 15
                {amdgpu_gfx1250_op_S_LSHL3_ADD_U32,"S_LSHL3_ADD_U32"}, // 16
                {amdgpu_gfx1250_op_S_LSHL4_ADD_U32,"S_LSHL4_ADD_U32"}, // 17
                {amdgpu_gfx1250_op_S_MIN_I32,"S_MIN_I32"}, // 18
                {amdgpu_gfx1250_op_S_MIN_U32,"S_MIN_U32"}, // 19
                {amdgpu_gfx1250_op_S_MAX_I32,"S_MAX_I32"}, // 20
                {amdgpu_gfx1250_op_S_MAX_U32,"S_MAX_U32"}, // 21
                {amdgpu_gfx1250_op_S_AND_B32,"S_AND_B32"}, // 22
                {amdgpu_gfx1250_op_S_AND_B64,"S_AND_B64"}, // 23
                {amdgpu_gfx1250_op_S_OR_B32,"S_OR_B32"}, // 24
                {amdgpu_gfx1250_op_S_OR_B64,"S_OR_B64"}, // 25
                {amdgpu_gfx1250_op_S_XOR_B32,"S_XOR_B32"}, // 26
                {amdgpu_gfx1250_op_S_XOR_B64,"S_XOR_B64"}, // 27
                {amdgpu_gfx1250_op_S_NAND_B32,"S_NAND_B32"}, // 28
                {amdgpu_gfx1250_op_S_NAND_B64,"S_NAND_B64"}, // 29
                {amdgpu_gfx1250_op_S_NOR_B32,"S_NOR_B32"}, // 30
                {amdgpu_gfx1250_op_S_NOR_B64,"S_NOR_B64"}, // 31
                {amdgpu_gfx1250_op_S_XNOR_B32,"S_XNOR_B32"}, // 32
                {amdgpu_gfx1250_op_S_XNOR_B64,"S_XNOR_B64"}, // 33
                {amdgpu_gfx1250_op_S_AND_NOT1_B32,"S_AND_NOT1_B32"}, // 34
                {amdgpu_gfx1250_op_S_AND_NOT1_B64,"S_AND_NOT1_B64"}, // 35
                {amdgpu_gfx1250_op_S_OR_NOT1_B32,"S_OR_NOT1_B32"}, // 36
                {amdgpu_gfx1250_op_S_OR_NOT1_B64,"S_OR_NOT1_B64"}, // 37
                {amdgpu_gfx1250_op_S_BFE_U32,"S_BFE_U32"}, // 38
                {amdgpu_gfx1250_op_S_BFE_I32,"S_BFE_I32"}, // 39
                {amdgpu_gfx1250_op_S_BFE_U64,"S_BFE_U64"}, // 40
                {amdgpu_gfx1250_op_S_BFE_I64,"S_BFE_I64"}, // 41
                {amdgpu_gfx1250_op_S_BFM_B32,"S_BFM_B32"}, // 42
                {amdgpu_gfx1250_op_S_BFM_B64,"S_BFM_B64"}, // 43
                {amdgpu_gfx1250_op_S_MUL_I32,"S_MUL_I32"}, // 44
                {amdgpu_gfx1250_op_S_MUL_HI_U32,"S_MUL_HI_U32"}, // 45
                {amdgpu_gfx1250_op_S_MUL_HI_I32,"S_MUL_HI_I32"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_S_CSELECT_B32,"S_CSELECT_B32"}, // 48
                {amdgpu_gfx1250_op_S_CSELECT_B64,"S_CSELECT_B64"}, // 49
                {amdgpu_gfx1250_op_S_PACK_LL_B32_B16,"S_PACK_LL_B32_B16"}, // 50
                {amdgpu_gfx1250_op_S_PACK_LH_B32_B16,"S_PACK_LH_B32_B16"}, // 51
                {amdgpu_gfx1250_op_S_PACK_HH_B32_B16,"S_PACK_HH_B32_B16"}, // 52
                {amdgpu_gfx1250_op_S_PACK_HL_B32_B16,"S_PACK_HL_B32_B16"}, // 53
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 59
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 60
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 61
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_S_ADD_F32,"S_ADD_F32"}, // 64
                {amdgpu_gfx1250_op_S_SUB_F32,"S_SUB_F32"}, // 65
                {amdgpu_gfx1250_op_S_MIN_NUM_F32,"S_MIN_NUM_F32"}, // 66
                {amdgpu_gfx1250_op_S_MAX_NUM_F32,"S_MAX_NUM_F32"}, // 67
                {amdgpu_gfx1250_op_S_MUL_F32,"S_MUL_F32"}, // 68
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 69
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 70
                {amdgpu_gfx1250_op_S_FMAC_F32,"S_FMAC_F32"}, // 71
                {amdgpu_gfx1250_op_S_CVT_PK_RTZ_F16_F32,"S_CVT_PK_RTZ_F16_F32"}, // 72
                {amdgpu_gfx1250_op_S_ADD_F16,"S_ADD_F16"}, // 73
                {amdgpu_gfx1250_op_S_SUB_F16,"S_SUB_F16"}, // 74
                {amdgpu_gfx1250_op_S_MIN_NUM_F16,"S_MIN_NUM_F16"}, // 75
                {amdgpu_gfx1250_op_S_MAX_NUM_F16,"S_MAX_NUM_F16"}, // 76
                {amdgpu_gfx1250_op_S_MUL_F16,"S_MUL_F16"}, // 77
                {amdgpu_gfx1250_op_S_FMAC_F16,"S_FMAC_F16"}, // 78
                {amdgpu_gfx1250_op_S_MINIMUM_F32,"S_MINIMUM_F32"}, // 79
                {amdgpu_gfx1250_op_S_MAXIMUM_F32,"S_MAXIMUM_F32"}, // 80
                {amdgpu_gfx1250_op_S_MINIMUM_F16,"S_MINIMUM_F16"}, // 81
                {amdgpu_gfx1250_op_S_MAXIMUM_F16,"S_MAXIMUM_F16"}, // 82
                {amdgpu_gfx1250_op_S_ADD_NC_U64,"S_ADD_NC_U64"}, // 83
                {amdgpu_gfx1250_op_S_SUB_NC_U64,"S_SUB_NC_U64"}, // 84
                {amdgpu_gfx1250_op_S_MUL_U64,"S_MUL_U64"}, // 85
            }; // end SOP2_INST_LITERAL64_insn_table
            const amdgpu_gfx1250_insn_entry ENC_SOPC_insn_table[95] =
            {
                {amdgpu_gfx1250_op_S_CMP_EQ_I32,"S_CMP_EQ_I32"}, // 0
                {amdgpu_gfx1250_op_S_CMP_LG_I32,"S_CMP_LG_I32"}, // 1
                {amdgpu_gfx1250_op_S_CMP_GT_I32,"S_CMP_GT_I32"}, // 2
                {amdgpu_gfx1250_op_S_CMP_GE_I32,"S_CMP_GE_I32"}, // 3
                {amdgpu_gfx1250_op_S_CMP_LT_I32,"S_CMP_LT_I32"}, // 4
                {amdgpu_gfx1250_op_S_CMP_LE_I32,"S_CMP_LE_I32"}, // 5
                {amdgpu_gfx1250_op_S_CMP_EQ_U32,"S_CMP_EQ_U32"}, // 6
                {amdgpu_gfx1250_op_S_CMP_LG_U32,"S_CMP_LG_U32"}, // 7
                {amdgpu_gfx1250_op_S_CMP_GT_U32,"S_CMP_GT_U32"}, // 8
                {amdgpu_gfx1250_op_S_CMP_GE_U32,"S_CMP_GE_U32"}, // 9
                {amdgpu_gfx1250_op_S_CMP_LT_U32,"S_CMP_LT_U32"}, // 10
                {amdgpu_gfx1250_op_S_CMP_LE_U32,"S_CMP_LE_U32"}, // 11
                {amdgpu_gfx1250_op_S_BITCMP0_B32,"S_BITCMP0_B32"}, // 12
                {amdgpu_gfx1250_op_S_BITCMP1_B32,"S_BITCMP1_B32"}, // 13
                {amdgpu_gfx1250_op_S_BITCMP0_B64,"S_BITCMP0_B64"}, // 14
                {amdgpu_gfx1250_op_S_BITCMP1_B64,"S_BITCMP1_B64"}, // 15
                {amdgpu_gfx1250_op_S_CMP_EQ_U64,"S_CMP_EQ_U64"}, // 16
                {amdgpu_gfx1250_op_S_CMP_LG_U64,"S_CMP_LG_U64"}, // 17
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 18
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 19
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 20
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 21
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 22
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 23
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 24
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 25
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 26
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 27
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 28
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 29
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 30
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 31
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 32
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 33
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 34
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 35
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 36
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 42
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 49
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 50
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 51
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 52
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 53
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 59
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 60
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 61
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 64
                {amdgpu_gfx1250_op_S_CMP_LT_F32,"S_CMP_LT_F32"}, // 65
                {amdgpu_gfx1250_op_S_CMP_EQ_F32,"S_CMP_EQ_F32"}, // 66
                {amdgpu_gfx1250_op_S_CMP_LE_F32,"S_CMP_LE_F32"}, // 67
                {amdgpu_gfx1250_op_S_CMP_GT_F32,"S_CMP_GT_F32"}, // 68
                {amdgpu_gfx1250_op_S_CMP_LG_F32,"S_CMP_LG_F32"}, // 69
                {amdgpu_gfx1250_op_S_CMP_GE_F32,"S_CMP_GE_F32"}, // 70
                {amdgpu_gfx1250_op_S_CMP_O_F32,"S_CMP_O_F32"}, // 71
                {amdgpu_gfx1250_op_S_CMP_U_F32,"S_CMP_U_F32"}, // 72
                {amdgpu_gfx1250_op_S_CMP_NGE_F32,"S_CMP_NGE_F32"}, // 73
                {amdgpu_gfx1250_op_S_CMP_NLG_F32,"S_CMP_NLG_F32"}, // 74
                {amdgpu_gfx1250_op_S_CMP_NGT_F32,"S_CMP_NGT_F32"}, // 75
                {amdgpu_gfx1250_op_S_CMP_NLE_F32,"S_CMP_NLE_F32"}, // 76
                {amdgpu_gfx1250_op_S_CMP_NEQ_F32,"S_CMP_NEQ_F32"}, // 77
                {amdgpu_gfx1250_op_S_CMP_NLT_F32,"S_CMP_NLT_F32"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 80
                {amdgpu_gfx1250_op_S_CMP_LT_F16,"S_CMP_LT_F16"}, // 81
                {amdgpu_gfx1250_op_S_CMP_EQ_F16,"S_CMP_EQ_F16"}, // 82
                {amdgpu_gfx1250_op_S_CMP_LE_F16,"S_CMP_LE_F16"}, // 83
                {amdgpu_gfx1250_op_S_CMP_GT_F16,"S_CMP_GT_F16"}, // 84
                {amdgpu_gfx1250_op_S_CMP_LG_F16,"S_CMP_LG_F16"}, // 85
                {amdgpu_gfx1250_op_S_CMP_GE_F16,"S_CMP_GE_F16"}, // 86
                {amdgpu_gfx1250_op_S_CMP_O_F16,"S_CMP_O_F16"}, // 87
                {amdgpu_gfx1250_op_S_CMP_U_F16,"S_CMP_U_F16"}, // 88
                {amdgpu_gfx1250_op_S_CMP_NGE_F16,"S_CMP_NGE_F16"}, // 89
                {amdgpu_gfx1250_op_S_CMP_NLG_F16,"S_CMP_NLG_F16"}, // 90
                {amdgpu_gfx1250_op_S_CMP_NGT_F16,"S_CMP_NGT_F16"}, // 91
                {amdgpu_gfx1250_op_S_CMP_NLE_F16,"S_CMP_NLE_F16"}, // 92
                {amdgpu_gfx1250_op_S_CMP_NEQ_F16,"S_CMP_NEQ_F16"}, // 93
                {amdgpu_gfx1250_op_S_CMP_NLT_F16,"S_CMP_NLT_F16"}, // 94
            }; // end ENC_SOPC_insn_table
            const amdgpu_gfx1250_insn_entry SOPC_INST_LITERAL_insn_table[95] =
            {
                {amdgpu_gfx1250_op_S_CMP_EQ_I32,"S_CMP_EQ_I32"}, // 0
                {amdgpu_gfx1250_op_S_CMP_LG_I32,"S_CMP_LG_I32"}, // 1
                {amdgpu_gfx1250_op_S_CMP_GT_I32,"S_CMP_GT_I32"}, // 2
                {amdgpu_gfx1250_op_S_CMP_GE_I32,"S_CMP_GE_I32"}, // 3
                {amdgpu_gfx1250_op_S_CMP_LT_I32,"S_CMP_LT_I32"}, // 4
                {amdgpu_gfx1250_op_S_CMP_LE_I32,"S_CMP_LE_I32"}, // 5
                {amdgpu_gfx1250_op_S_CMP_EQ_U32,"S_CMP_EQ_U32"}, // 6
                {amdgpu_gfx1250_op_S_CMP_LG_U32,"S_CMP_LG_U32"}, // 7
                {amdgpu_gfx1250_op_S_CMP_GT_U32,"S_CMP_GT_U32"}, // 8
                {amdgpu_gfx1250_op_S_CMP_GE_U32,"S_CMP_GE_U32"}, // 9
                {amdgpu_gfx1250_op_S_CMP_LT_U32,"S_CMP_LT_U32"}, // 10
                {amdgpu_gfx1250_op_S_CMP_LE_U32,"S_CMP_LE_U32"}, // 11
                {amdgpu_gfx1250_op_S_BITCMP0_B32,"S_BITCMP0_B32"}, // 12
                {amdgpu_gfx1250_op_S_BITCMP1_B32,"S_BITCMP1_B32"}, // 13
                {amdgpu_gfx1250_op_S_BITCMP0_B64,"S_BITCMP0_B64"}, // 14
                {amdgpu_gfx1250_op_S_BITCMP1_B64,"S_BITCMP1_B64"}, // 15
                {amdgpu_gfx1250_op_S_CMP_EQ_U64,"S_CMP_EQ_U64"}, // 16
                {amdgpu_gfx1250_op_S_CMP_LG_U64,"S_CMP_LG_U64"}, // 17
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 18
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 19
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 20
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 21
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 22
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 23
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 24
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 25
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 26
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 27
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 28
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 29
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 30
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 31
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 32
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 33
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 34
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 35
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 36
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 42
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 49
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 50
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 51
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 52
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 53
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 59
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 60
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 61
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 64
                {amdgpu_gfx1250_op_S_CMP_LT_F32,"S_CMP_LT_F32"}, // 65
                {amdgpu_gfx1250_op_S_CMP_EQ_F32,"S_CMP_EQ_F32"}, // 66
                {amdgpu_gfx1250_op_S_CMP_LE_F32,"S_CMP_LE_F32"}, // 67
                {amdgpu_gfx1250_op_S_CMP_GT_F32,"S_CMP_GT_F32"}, // 68
                {amdgpu_gfx1250_op_S_CMP_LG_F32,"S_CMP_LG_F32"}, // 69
                {amdgpu_gfx1250_op_S_CMP_GE_F32,"S_CMP_GE_F32"}, // 70
                {amdgpu_gfx1250_op_S_CMP_O_F32,"S_CMP_O_F32"}, // 71
                {amdgpu_gfx1250_op_S_CMP_U_F32,"S_CMP_U_F32"}, // 72
                {amdgpu_gfx1250_op_S_CMP_NGE_F32,"S_CMP_NGE_F32"}, // 73
                {amdgpu_gfx1250_op_S_CMP_NLG_F32,"S_CMP_NLG_F32"}, // 74
                {amdgpu_gfx1250_op_S_CMP_NGT_F32,"S_CMP_NGT_F32"}, // 75
                {amdgpu_gfx1250_op_S_CMP_NLE_F32,"S_CMP_NLE_F32"}, // 76
                {amdgpu_gfx1250_op_S_CMP_NEQ_F32,"S_CMP_NEQ_F32"}, // 77
                {amdgpu_gfx1250_op_S_CMP_NLT_F32,"S_CMP_NLT_F32"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 80
                {amdgpu_gfx1250_op_S_CMP_LT_F16,"S_CMP_LT_F16"}, // 81
                {amdgpu_gfx1250_op_S_CMP_EQ_F16,"S_CMP_EQ_F16"}, // 82
                {amdgpu_gfx1250_op_S_CMP_LE_F16,"S_CMP_LE_F16"}, // 83
                {amdgpu_gfx1250_op_S_CMP_GT_F16,"S_CMP_GT_F16"}, // 84
                {amdgpu_gfx1250_op_S_CMP_LG_F16,"S_CMP_LG_F16"}, // 85
                {amdgpu_gfx1250_op_S_CMP_GE_F16,"S_CMP_GE_F16"}, // 86
                {amdgpu_gfx1250_op_S_CMP_O_F16,"S_CMP_O_F16"}, // 87
                {amdgpu_gfx1250_op_S_CMP_U_F16,"S_CMP_U_F16"}, // 88
                {amdgpu_gfx1250_op_S_CMP_NGE_F16,"S_CMP_NGE_F16"}, // 89
                {amdgpu_gfx1250_op_S_CMP_NLG_F16,"S_CMP_NLG_F16"}, // 90
                {amdgpu_gfx1250_op_S_CMP_NGT_F16,"S_CMP_NGT_F16"}, // 91
                {amdgpu_gfx1250_op_S_CMP_NLE_F16,"S_CMP_NLE_F16"}, // 92
                {amdgpu_gfx1250_op_S_CMP_NEQ_F16,"S_CMP_NEQ_F16"}, // 93
                {amdgpu_gfx1250_op_S_CMP_NLT_F16,"S_CMP_NLT_F16"}, // 94
            }; // end SOPC_INST_LITERAL_insn_table
            const amdgpu_gfx1250_insn_entry SOPC_INST_LITERAL64_insn_table[95] =
            {
                {amdgpu_gfx1250_op_S_CMP_EQ_I32,"S_CMP_EQ_I32"}, // 0
                {amdgpu_gfx1250_op_S_CMP_LG_I32,"S_CMP_LG_I32"}, // 1
                {amdgpu_gfx1250_op_S_CMP_GT_I32,"S_CMP_GT_I32"}, // 2
                {amdgpu_gfx1250_op_S_CMP_GE_I32,"S_CMP_GE_I32"}, // 3
                {amdgpu_gfx1250_op_S_CMP_LT_I32,"S_CMP_LT_I32"}, // 4
                {amdgpu_gfx1250_op_S_CMP_LE_I32,"S_CMP_LE_I32"}, // 5
                {amdgpu_gfx1250_op_S_CMP_EQ_U32,"S_CMP_EQ_U32"}, // 6
                {amdgpu_gfx1250_op_S_CMP_LG_U32,"S_CMP_LG_U32"}, // 7
                {amdgpu_gfx1250_op_S_CMP_GT_U32,"S_CMP_GT_U32"}, // 8
                {amdgpu_gfx1250_op_S_CMP_GE_U32,"S_CMP_GE_U32"}, // 9
                {amdgpu_gfx1250_op_S_CMP_LT_U32,"S_CMP_LT_U32"}, // 10
                {amdgpu_gfx1250_op_S_CMP_LE_U32,"S_CMP_LE_U32"}, // 11
                {amdgpu_gfx1250_op_S_BITCMP0_B32,"S_BITCMP0_B32"}, // 12
                {amdgpu_gfx1250_op_S_BITCMP1_B32,"S_BITCMP1_B32"}, // 13
                {amdgpu_gfx1250_op_S_BITCMP0_B64,"S_BITCMP0_B64"}, // 14
                {amdgpu_gfx1250_op_S_BITCMP1_B64,"S_BITCMP1_B64"}, // 15
                {amdgpu_gfx1250_op_S_CMP_EQ_U64,"S_CMP_EQ_U64"}, // 16
                {amdgpu_gfx1250_op_S_CMP_LG_U64,"S_CMP_LG_U64"}, // 17
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 18
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 19
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 20
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 21
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 22
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 23
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 24
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 25
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 26
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 27
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 28
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 29
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 30
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 31
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 32
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 33
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 34
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 35
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 36
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 42
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 49
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 50
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 51
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 52
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 53
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 59
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 60
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 61
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 64
                {amdgpu_gfx1250_op_S_CMP_LT_F32,"S_CMP_LT_F32"}, // 65
                {amdgpu_gfx1250_op_S_CMP_EQ_F32,"S_CMP_EQ_F32"}, // 66
                {amdgpu_gfx1250_op_S_CMP_LE_F32,"S_CMP_LE_F32"}, // 67
                {amdgpu_gfx1250_op_S_CMP_GT_F32,"S_CMP_GT_F32"}, // 68
                {amdgpu_gfx1250_op_S_CMP_LG_F32,"S_CMP_LG_F32"}, // 69
                {amdgpu_gfx1250_op_S_CMP_GE_F32,"S_CMP_GE_F32"}, // 70
                {amdgpu_gfx1250_op_S_CMP_O_F32,"S_CMP_O_F32"}, // 71
                {amdgpu_gfx1250_op_S_CMP_U_F32,"S_CMP_U_F32"}, // 72
                {amdgpu_gfx1250_op_S_CMP_NGE_F32,"S_CMP_NGE_F32"}, // 73
                {amdgpu_gfx1250_op_S_CMP_NLG_F32,"S_CMP_NLG_F32"}, // 74
                {amdgpu_gfx1250_op_S_CMP_NGT_F32,"S_CMP_NGT_F32"}, // 75
                {amdgpu_gfx1250_op_S_CMP_NLE_F32,"S_CMP_NLE_F32"}, // 76
                {amdgpu_gfx1250_op_S_CMP_NEQ_F32,"S_CMP_NEQ_F32"}, // 77
                {amdgpu_gfx1250_op_S_CMP_NLT_F32,"S_CMP_NLT_F32"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 80
                {amdgpu_gfx1250_op_S_CMP_LT_F16,"S_CMP_LT_F16"}, // 81
                {amdgpu_gfx1250_op_S_CMP_EQ_F16,"S_CMP_EQ_F16"}, // 82
                {amdgpu_gfx1250_op_S_CMP_LE_F16,"S_CMP_LE_F16"}, // 83
                {amdgpu_gfx1250_op_S_CMP_GT_F16,"S_CMP_GT_F16"}, // 84
                {amdgpu_gfx1250_op_S_CMP_LG_F16,"S_CMP_LG_F16"}, // 85
                {amdgpu_gfx1250_op_S_CMP_GE_F16,"S_CMP_GE_F16"}, // 86
                {amdgpu_gfx1250_op_S_CMP_O_F16,"S_CMP_O_F16"}, // 87
                {amdgpu_gfx1250_op_S_CMP_U_F16,"S_CMP_U_F16"}, // 88
                {amdgpu_gfx1250_op_S_CMP_NGE_F16,"S_CMP_NGE_F16"}, // 89
                {amdgpu_gfx1250_op_S_CMP_NLG_F16,"S_CMP_NLG_F16"}, // 90
                {amdgpu_gfx1250_op_S_CMP_NGT_F16,"S_CMP_NGT_F16"}, // 91
                {amdgpu_gfx1250_op_S_CMP_NLE_F16,"S_CMP_NLE_F16"}, // 92
                {amdgpu_gfx1250_op_S_CMP_NEQ_F16,"S_CMP_NEQ_F16"}, // 93
                {amdgpu_gfx1250_op_S_CMP_NLT_F16,"S_CMP_NLT_F16"}, // 94
            }; // end SOPC_INST_LITERAL64_insn_table
            const amdgpu_gfx1250_insn_entry ENC_SOPK_insn_table[21] =
            {
                {amdgpu_gfx1250_op_S_MOVK_I32,"S_MOVK_I32"}, // 0
                {amdgpu_gfx1250_op_S_VERSION,"S_VERSION"}, // 1
                {amdgpu_gfx1250_op_S_CMOVK_I32,"S_CMOVK_I32"}, // 2
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 3
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 4
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 5
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 6
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 7
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 8
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 9
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 10
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 11
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 12
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 13
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 14
                {amdgpu_gfx1250_op_S_ADDK_CO_I32,"S_ADDK_CO_I32"}, // 15
                {amdgpu_gfx1250_op_S_MULK_I32,"S_MULK_I32"}, // 16
                {amdgpu_gfx1250_op_S_GETREG_B32,"S_GETREG_B32"}, // 17
                {amdgpu_gfx1250_op_S_SETREG_B32,"S_SETREG_B32"}, // 18
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 19
                {amdgpu_gfx1250_op_S_CALL_I64,"S_CALL_I64"}, // 20
            }; // end ENC_SOPK_insn_table
            const amdgpu_gfx1250_insn_entry SOPK_INST_LITERAL_insn_table[20] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 1
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 2
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 3
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 4
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 5
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 6
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 7
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 8
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 9
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 10
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 11
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 12
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 13
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 17
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 18
                {amdgpu_gfx1250_op_S_SETREG_IMM32_B32,"S_SETREG_IMM32_B32"}, // 19
            }; // end SOPK_INST_LITERAL_insn_table
            const amdgpu_gfx1250_insn_entry ENC_SOPP_insn_table[76] =
            {
                {amdgpu_gfx1250_op_S_NOP,"S_NOP"}, // 0
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 1
                {amdgpu_gfx1250_op_S_SETHALT,"S_SETHALT"}, // 2
                {amdgpu_gfx1250_op_S_SLEEP,"S_SLEEP"}, // 3
                {amdgpu_gfx1250_op_S_MONITOR_SLEEP,"S_MONITOR_SLEEP"}, // 4
                {amdgpu_gfx1250_op_S_CLAUSE,"S_CLAUSE"}, // 5
                {amdgpu_gfx1250_op_S_SET_VGPR_MSB,"S_SET_VGPR_MSB"}, // 6
                {amdgpu_gfx1250_op_S_DELAY_ALU,"S_DELAY_ALU"}, // 7
                {amdgpu_gfx1250_op_S_WAIT_ALU,"S_WAIT_ALU"}, // 8
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 9
                {amdgpu_gfx1250_op_S_WAIT_IDLE,"S_WAIT_IDLE"}, // 10
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 11
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 12
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 13
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_S_TRAP,"S_TRAP"}, // 16
                {amdgpu_gfx1250_op_S_ROUND_MODE,"S_ROUND_MODE"}, // 17
                {amdgpu_gfx1250_op_S_DENORM_MODE,"S_DENORM_MODE"}, // 18
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 19
                {amdgpu_gfx1250_op_S_BARRIER_WAIT,"S_BARRIER_WAIT"}, // 20
                {amdgpu_gfx1250_op_S_BARRIER_LEAVE,"S_BARRIER_LEAVE"}, // 21
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 22
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 23
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 24
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 25
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 26
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 27
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 28
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 29
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 30
                {amdgpu_gfx1250_op_S_CODE_END,"S_CODE_END"}, // 31
                {amdgpu_gfx1250_op_S_BRANCH,"S_BRANCH"}, // 32
                {amdgpu_gfx1250_op_S_CBRANCH_SCC0,"S_CBRANCH_SCC0"}, // 33
                {amdgpu_gfx1250_op_S_CBRANCH_SCC1,"S_CBRANCH_SCC1"}, // 34
                {amdgpu_gfx1250_op_S_CBRANCH_VCCZ,"S_CBRANCH_VCCZ"}, // 35
                {amdgpu_gfx1250_op_S_CBRANCH_VCCNZ,"S_CBRANCH_VCCNZ"}, // 36
                {amdgpu_gfx1250_op_S_CBRANCH_EXECZ,"S_CBRANCH_EXECZ"}, // 37
                {amdgpu_gfx1250_op_S_CBRANCH_EXECNZ,"S_CBRANCH_EXECNZ"}, // 38
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 42
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_S_ENDPGM,"S_ENDPGM"}, // 48
                {amdgpu_gfx1250_op_S_ENDPGM_SAVED,"S_ENDPGM_SAVED"}, // 49
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 50
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 51
                {amdgpu_gfx1250_op_S_WAKEUP,"S_WAKEUP"}, // 52
                {amdgpu_gfx1250_op_S_SETPRIO,"S_SETPRIO"}, // 53
                {amdgpu_gfx1250_op_S_SENDMSG,"S_SENDMSG"}, // 54
                {amdgpu_gfx1250_op_S_SENDMSGHALT,"S_SENDMSGHALT"}, // 55
                {amdgpu_gfx1250_op_S_INCPERFLEVEL,"S_INCPERFLEVEL"}, // 56
                {amdgpu_gfx1250_op_S_DECPERFLEVEL,"S_DECPERFLEVEL"}, // 57
                {amdgpu_gfx1250_op_S_TTRACEDATA,"S_TTRACEDATA"}, // 58
                {amdgpu_gfx1250_op_S_TTRACEDATA_IMM,"S_TTRACEDATA_IMM"}, // 59
                {amdgpu_gfx1250_op_S_ICACHE_INV,"S_ICACHE_INV"}, // 60
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 61
                {amdgpu_gfx1250_op_S_SETPRIO_INC_WG,"S_SETPRIO_INC_WG"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_S_WAIT_LOADCNT,"S_WAIT_LOADCNT"}, // 64
                {amdgpu_gfx1250_op_S_WAIT_STORECNT,"S_WAIT_STORECNT"}, // 65
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 66
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 67
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 68
                {amdgpu_gfx1250_op_S_WAIT_XCNT,"S_WAIT_XCNT"}, // 69
                {amdgpu_gfx1250_op_S_WAIT_DSCNT,"S_WAIT_DSCNT"}, // 70
                {amdgpu_gfx1250_op_S_WAIT_KMCNT,"S_WAIT_KMCNT"}, // 71
                {amdgpu_gfx1250_op_S_WAIT_LOADCNT_DSCNT,"S_WAIT_LOADCNT_DSCNT"}, // 72
                {amdgpu_gfx1250_op_S_WAIT_STORECNT_DSCNT,"S_WAIT_STORECNT_DSCNT"}, // 73
                {amdgpu_gfx1250_op_S_WAIT_ASYNCCNT,"S_WAIT_ASYNCCNT"}, // 74
                {amdgpu_gfx1250_op_S_WAIT_TENSORCNT,"S_WAIT_TENSORCNT"}, // 75
            }; // end ENC_SOPP_insn_table
            const amdgpu_gfx1250_insn_entry ENC_VBUFFER_insn_table[93] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 1
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 2
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 3
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 4
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 5
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 6
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 7
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 8
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 9
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 10
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 11
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 12
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 13
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_BUFFER_LOAD_U8,"BUFFER_LOAD_U8"}, // 16
                {amdgpu_gfx1250_op_BUFFER_LOAD_I8,"BUFFER_LOAD_I8"}, // 17
                {amdgpu_gfx1250_op_BUFFER_LOAD_U16,"BUFFER_LOAD_U16"}, // 18
                {amdgpu_gfx1250_op_BUFFER_LOAD_I16,"BUFFER_LOAD_I16"}, // 19
                {amdgpu_gfx1250_op_BUFFER_LOAD_B32,"BUFFER_LOAD_B32"}, // 20
                {amdgpu_gfx1250_op_BUFFER_LOAD_B64,"BUFFER_LOAD_B64"}, // 21
                {amdgpu_gfx1250_op_BUFFER_LOAD_B96,"BUFFER_LOAD_B96"}, // 22
                {amdgpu_gfx1250_op_BUFFER_LOAD_B128,"BUFFER_LOAD_B128"}, // 23
                {amdgpu_gfx1250_op_BUFFER_STORE_B8,"BUFFER_STORE_B8"}, // 24
                {amdgpu_gfx1250_op_BUFFER_STORE_B16,"BUFFER_STORE_B16"}, // 25
                {amdgpu_gfx1250_op_BUFFER_STORE_B32,"BUFFER_STORE_B32"}, // 26
                {amdgpu_gfx1250_op_BUFFER_STORE_B64,"BUFFER_STORE_B64"}, // 27
                {amdgpu_gfx1250_op_BUFFER_STORE_B96,"BUFFER_STORE_B96"}, // 28
                {amdgpu_gfx1250_op_BUFFER_STORE_B128,"BUFFER_STORE_B128"}, // 29
                {amdgpu_gfx1250_op_BUFFER_LOAD_D16_U8,"BUFFER_LOAD_D16_U8"}, // 30
                {amdgpu_gfx1250_op_BUFFER_LOAD_D16_I8,"BUFFER_LOAD_D16_I8"}, // 31
                {amdgpu_gfx1250_op_BUFFER_LOAD_D16_B16,"BUFFER_LOAD_D16_B16"}, // 32
                {amdgpu_gfx1250_op_BUFFER_LOAD_D16_HI_U8,"BUFFER_LOAD_D16_HI_U8"}, // 33
                {amdgpu_gfx1250_op_BUFFER_LOAD_D16_HI_I8,"BUFFER_LOAD_D16_HI_I8"}, // 34
                {amdgpu_gfx1250_op_BUFFER_LOAD_D16_HI_B16,"BUFFER_LOAD_D16_HI_B16"}, // 35
                {amdgpu_gfx1250_op_BUFFER_STORE_D16_HI_B8,"BUFFER_STORE_D16_HI_B8"}, // 36
                {amdgpu_gfx1250_op_BUFFER_STORE_D16_HI_B16,"BUFFER_STORE_D16_HI_B16"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 42
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 49
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 50
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_SWAP_B32,"BUFFER_ATOMIC_SWAP_B32"}, // 51
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_CMPSWAP_B32,"BUFFER_ATOMIC_CMPSWAP_B32"}, // 52
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_ADD_U32,"BUFFER_ATOMIC_ADD_U32"}, // 53
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_SUB_U32,"BUFFER_ATOMIC_SUB_U32"}, // 54
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_SUB_CLAMP_U32,"BUFFER_ATOMIC_SUB_CLAMP_U32"}, // 55
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_MIN_I32,"BUFFER_ATOMIC_MIN_I32"}, // 56
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_MIN_U32,"BUFFER_ATOMIC_MIN_U32"}, // 57
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_MAX_I32,"BUFFER_ATOMIC_MAX_I32"}, // 58
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_MAX_U32,"BUFFER_ATOMIC_MAX_U32"}, // 59
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_AND_B32,"BUFFER_ATOMIC_AND_B32"}, // 60
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_OR_B32,"BUFFER_ATOMIC_OR_B32"}, // 61
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_XOR_B32,"BUFFER_ATOMIC_XOR_B32"}, // 62
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_INC_U32,"BUFFER_ATOMIC_INC_U32"}, // 63
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_DEC_U32,"BUFFER_ATOMIC_DEC_U32"}, // 64
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_SWAP_B64,"BUFFER_ATOMIC_SWAP_B64"}, // 65
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_CMPSWAP_B64,"BUFFER_ATOMIC_CMPSWAP_B64"}, // 66
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_ADD_U64,"BUFFER_ATOMIC_ADD_U64"}, // 67
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_SUB_U64,"BUFFER_ATOMIC_SUB_U64"}, // 68
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_MIN_I64,"BUFFER_ATOMIC_MIN_I64"}, // 69
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_MIN_U64,"BUFFER_ATOMIC_MIN_U64"}, // 70
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_MAX_I64,"BUFFER_ATOMIC_MAX_I64"}, // 71
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_MAX_U64,"BUFFER_ATOMIC_MAX_U64"}, // 72
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_AND_B64,"BUFFER_ATOMIC_AND_B64"}, // 73
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_OR_B64,"BUFFER_ATOMIC_OR_B64"}, // 74
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_XOR_B64,"BUFFER_ATOMIC_XOR_B64"}, // 75
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_INC_U64,"BUFFER_ATOMIC_INC_U64"}, // 76
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_DEC_U64,"BUFFER_ATOMIC_DEC_U64"}, // 77
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_COND_SUB_U32,"BUFFER_ATOMIC_COND_SUB_U32"}, // 80
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_MIN_NUM_F32,"BUFFER_ATOMIC_MIN_NUM_F32"}, // 81
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_MAX_NUM_F32,"BUFFER_ATOMIC_MAX_NUM_F32"}, // 82
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 83
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 84
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_ADD_F64,"BUFFER_ATOMIC_ADD_F64"}, // 85
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_ADD_F32,"BUFFER_ATOMIC_ADD_F32"}, // 86
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 87
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 88
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_PK_ADD_F16,"BUFFER_ATOMIC_PK_ADD_F16"}, // 89
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_PK_ADD_BF16,"BUFFER_ATOMIC_PK_ADD_BF16"}, // 90
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_MIN_NUM_F64,"BUFFER_ATOMIC_MIN_NUM_F64"}, // 91
                {amdgpu_gfx1250_op_BUFFER_ATOMIC_MAX_NUM_F64,"BUFFER_ATOMIC_MAX_NUM_F64"}, // 92
            }; // end ENC_VBUFFER_insn_table
            const amdgpu_gfx1250_insn_entry ENC_VDS_insn_table[256] =
            {
                {amdgpu_gfx1250_op_DS_ADD_U32,"DS_ADD_U32"}, // 0
                {amdgpu_gfx1250_op_DS_SUB_U32,"DS_SUB_U32"}, // 1
                {amdgpu_gfx1250_op_DS_RSUB_U32,"DS_RSUB_U32"}, // 2
                {amdgpu_gfx1250_op_DS_INC_U32,"DS_INC_U32"}, // 3
                {amdgpu_gfx1250_op_DS_DEC_U32,"DS_DEC_U32"}, // 4
                {amdgpu_gfx1250_op_DS_MIN_I32,"DS_MIN_I32"}, // 5
                {amdgpu_gfx1250_op_DS_MAX_I32,"DS_MAX_I32"}, // 6
                {amdgpu_gfx1250_op_DS_MIN_U32,"DS_MIN_U32"}, // 7
                {amdgpu_gfx1250_op_DS_MAX_U32,"DS_MAX_U32"}, // 8
                {amdgpu_gfx1250_op_DS_AND_B32,"DS_AND_B32"}, // 9
                {amdgpu_gfx1250_op_DS_OR_B32,"DS_OR_B32"}, // 10
                {amdgpu_gfx1250_op_DS_XOR_B32,"DS_XOR_B32"}, // 11
                {amdgpu_gfx1250_op_DS_MSKOR_B32,"DS_MSKOR_B32"}, // 12
                {amdgpu_gfx1250_op_DS_STORE_B32,"DS_STORE_B32"}, // 13
                {amdgpu_gfx1250_op_DS_STORE_2ADDR_B32,"DS_STORE_2ADDR_B32"}, // 14
                {amdgpu_gfx1250_op_DS_STORE_2ADDR_STRIDE64_B32,"DS_STORE_2ADDR_STRIDE64_B32"}, // 15
                {amdgpu_gfx1250_op_DS_CMPSTORE_B32,"DS_CMPSTORE_B32"}, // 16
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 17
                {amdgpu_gfx1250_op_DS_MIN_NUM_F32,"DS_MIN_NUM_F32"}, // 18
                {amdgpu_gfx1250_op_DS_MAX_NUM_F32,"DS_MAX_NUM_F32"}, // 19
                {amdgpu_gfx1250_op_DS_NOP,"DS_NOP"}, // 20
                {amdgpu_gfx1250_op_DS_ADD_F32,"DS_ADD_F32"}, // 21
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 22
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 23
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 24
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 25
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 26
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 27
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 28
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 29
                {amdgpu_gfx1250_op_DS_STORE_B8,"DS_STORE_B8"}, // 30
                {amdgpu_gfx1250_op_DS_STORE_B16,"DS_STORE_B16"}, // 31
                {amdgpu_gfx1250_op_DS_ADD_RTN_U32,"DS_ADD_RTN_U32"}, // 32
                {amdgpu_gfx1250_op_DS_SUB_RTN_U32,"DS_SUB_RTN_U32"}, // 33
                {amdgpu_gfx1250_op_DS_RSUB_RTN_U32,"DS_RSUB_RTN_U32"}, // 34
                {amdgpu_gfx1250_op_DS_INC_RTN_U32,"DS_INC_RTN_U32"}, // 35
                {amdgpu_gfx1250_op_DS_DEC_RTN_U32,"DS_DEC_RTN_U32"}, // 36
                {amdgpu_gfx1250_op_DS_MIN_RTN_I32,"DS_MIN_RTN_I32"}, // 37
                {amdgpu_gfx1250_op_DS_MAX_RTN_I32,"DS_MAX_RTN_I32"}, // 38
                {amdgpu_gfx1250_op_DS_MIN_RTN_U32,"DS_MIN_RTN_U32"}, // 39
                {amdgpu_gfx1250_op_DS_MAX_RTN_U32,"DS_MAX_RTN_U32"}, // 40
                {amdgpu_gfx1250_op_DS_AND_RTN_B32,"DS_AND_RTN_B32"}, // 41
                {amdgpu_gfx1250_op_DS_OR_RTN_B32,"DS_OR_RTN_B32"}, // 42
                {amdgpu_gfx1250_op_DS_XOR_RTN_B32,"DS_XOR_RTN_B32"}, // 43
                {amdgpu_gfx1250_op_DS_MSKOR_RTN_B32,"DS_MSKOR_RTN_B32"}, // 44
                {amdgpu_gfx1250_op_DS_STOREXCHG_RTN_B32,"DS_STOREXCHG_RTN_B32"}, // 45
                {amdgpu_gfx1250_op_DS_STOREXCHG_2ADDR_RTN_B32,"DS_STOREXCHG_2ADDR_RTN_B32"}, // 46
                {amdgpu_gfx1250_op_DS_STOREXCHG_2ADDR_STRIDE64_RTN_B32,"DS_STOREXCHG_2ADDR_STRIDE64_RTN_B32"}, // 47
                {amdgpu_gfx1250_op_DS_CMPSTORE_RTN_B32,"DS_CMPSTORE_RTN_B32"}, // 48
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 49
                {amdgpu_gfx1250_op_DS_MIN_NUM_RTN_F32,"DS_MIN_NUM_RTN_F32"}, // 50
                {amdgpu_gfx1250_op_DS_MAX_NUM_RTN_F32,"DS_MAX_NUM_RTN_F32"}, // 51
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 52
                {amdgpu_gfx1250_op_DS_SWIZZLE_B32,"DS_SWIZZLE_B32"}, // 53
                {amdgpu_gfx1250_op_DS_LOAD_B32,"DS_LOAD_B32"}, // 54
                {amdgpu_gfx1250_op_DS_LOAD_2ADDR_B32,"DS_LOAD_2ADDR_B32"}, // 55
                {amdgpu_gfx1250_op_DS_LOAD_2ADDR_STRIDE64_B32,"DS_LOAD_2ADDR_STRIDE64_B32"}, // 56
                {amdgpu_gfx1250_op_DS_LOAD_I8,"DS_LOAD_I8"}, // 57
                {amdgpu_gfx1250_op_DS_LOAD_U8,"DS_LOAD_U8"}, // 58
                {amdgpu_gfx1250_op_DS_LOAD_I16,"DS_LOAD_I16"}, // 59
                {amdgpu_gfx1250_op_DS_LOAD_U16,"DS_LOAD_U16"}, // 60
                {amdgpu_gfx1250_op_DS_CONSUME,"DS_CONSUME"}, // 61
                {amdgpu_gfx1250_op_DS_APPEND,"DS_APPEND"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_DS_ADD_U64,"DS_ADD_U64"}, // 64
                {amdgpu_gfx1250_op_DS_SUB_U64,"DS_SUB_U64"}, // 65
                {amdgpu_gfx1250_op_DS_RSUB_U64,"DS_RSUB_U64"}, // 66
                {amdgpu_gfx1250_op_DS_INC_U64,"DS_INC_U64"}, // 67
                {amdgpu_gfx1250_op_DS_DEC_U64,"DS_DEC_U64"}, // 68
                {amdgpu_gfx1250_op_DS_MIN_I64,"DS_MIN_I64"}, // 69
                {amdgpu_gfx1250_op_DS_MAX_I64,"DS_MAX_I64"}, // 70
                {amdgpu_gfx1250_op_DS_MIN_U64,"DS_MIN_U64"}, // 71
                {amdgpu_gfx1250_op_DS_MAX_U64,"DS_MAX_U64"}, // 72
                {amdgpu_gfx1250_op_DS_AND_B64,"DS_AND_B64"}, // 73
                {amdgpu_gfx1250_op_DS_OR_B64,"DS_OR_B64"}, // 74
                {amdgpu_gfx1250_op_DS_XOR_B64,"DS_XOR_B64"}, // 75
                {amdgpu_gfx1250_op_DS_MSKOR_B64,"DS_MSKOR_B64"}, // 76
                {amdgpu_gfx1250_op_DS_STORE_B64,"DS_STORE_B64"}, // 77
                {amdgpu_gfx1250_op_DS_STORE_2ADDR_B64,"DS_STORE_2ADDR_B64"}, // 78
                {amdgpu_gfx1250_op_DS_STORE_2ADDR_STRIDE64_B64,"DS_STORE_2ADDR_STRIDE64_B64"}, // 79
                {amdgpu_gfx1250_op_DS_CMPSTORE_B64,"DS_CMPSTORE_B64"}, // 80
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 81
                {amdgpu_gfx1250_op_DS_MIN_NUM_F64,"DS_MIN_NUM_F64"}, // 82
                {amdgpu_gfx1250_op_DS_MAX_NUM_F64,"DS_MAX_NUM_F64"}, // 83
                {amdgpu_gfx1250_op_DS_ADD_F64,"DS_ADD_F64"}, // 84
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 85
                {amdgpu_gfx1250_op_DS_ATOMIC_ASYNC_BARRIER_ARRIVE_B64,"DS_ATOMIC_ASYNC_BARRIER_ARRIVE_B64"}, // 86
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 87
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 88
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 89
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 90
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 91
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 92
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 93
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 94
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 95
                {amdgpu_gfx1250_op_DS_ADD_RTN_U64,"DS_ADD_RTN_U64"}, // 96
                {amdgpu_gfx1250_op_DS_SUB_RTN_U64,"DS_SUB_RTN_U64"}, // 97
                {amdgpu_gfx1250_op_DS_RSUB_RTN_U64,"DS_RSUB_RTN_U64"}, // 98
                {amdgpu_gfx1250_op_DS_INC_RTN_U64,"DS_INC_RTN_U64"}, // 99
                {amdgpu_gfx1250_op_DS_DEC_RTN_U64,"DS_DEC_RTN_U64"}, // 100
                {amdgpu_gfx1250_op_DS_MIN_RTN_I64,"DS_MIN_RTN_I64"}, // 101
                {amdgpu_gfx1250_op_DS_MAX_RTN_I64,"DS_MAX_RTN_I64"}, // 102
                {amdgpu_gfx1250_op_DS_MIN_RTN_U64,"DS_MIN_RTN_U64"}, // 103
                {amdgpu_gfx1250_op_DS_MAX_RTN_U64,"DS_MAX_RTN_U64"}, // 104
                {amdgpu_gfx1250_op_DS_AND_RTN_B64,"DS_AND_RTN_B64"}, // 105
                {amdgpu_gfx1250_op_DS_OR_RTN_B64,"DS_OR_RTN_B64"}, // 106
                {amdgpu_gfx1250_op_DS_XOR_RTN_B64,"DS_XOR_RTN_B64"}, // 107
                {amdgpu_gfx1250_op_DS_MSKOR_RTN_B64,"DS_MSKOR_RTN_B64"}, // 108
                {amdgpu_gfx1250_op_DS_STOREXCHG_RTN_B64,"DS_STOREXCHG_RTN_B64"}, // 109
                {amdgpu_gfx1250_op_DS_STOREXCHG_2ADDR_RTN_B64,"DS_STOREXCHG_2ADDR_RTN_B64"}, // 110
                {amdgpu_gfx1250_op_DS_STOREXCHG_2ADDR_STRIDE64_RTN_B64,"DS_STOREXCHG_2ADDR_STRIDE64_RTN_B64"}, // 111
                {amdgpu_gfx1250_op_DS_CMPSTORE_RTN_B64,"DS_CMPSTORE_RTN_B64"}, // 112
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 113
                {amdgpu_gfx1250_op_DS_MIN_NUM_RTN_F64,"DS_MIN_NUM_RTN_F64"}, // 114
                {amdgpu_gfx1250_op_DS_MAX_NUM_RTN_F64,"DS_MAX_NUM_RTN_F64"}, // 115
                {amdgpu_gfx1250_op_DS_ADD_RTN_F64,"DS_ADD_RTN_F64"}, // 116
                {amdgpu_gfx1250_op_DS_ATOMIC_BARRIER_ARRIVE_RTN_B64,"DS_ATOMIC_BARRIER_ARRIVE_RTN_B64"}, // 117
                {amdgpu_gfx1250_op_DS_LOAD_B64,"DS_LOAD_B64"}, // 118
                {amdgpu_gfx1250_op_DS_LOAD_2ADDR_B64,"DS_LOAD_2ADDR_B64"}, // 119
                {amdgpu_gfx1250_op_DS_LOAD_2ADDR_STRIDE64_B64,"DS_LOAD_2ADDR_STRIDE64_B64"}, // 120
                {amdgpu_gfx1250_op_DS_ADD_RTN_F32,"DS_ADD_RTN_F32"}, // 121
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 122
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 123
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 124
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 125
                {amdgpu_gfx1250_op_DS_CONDXCHG32_RTN_B64,"DS_CONDXCHG32_RTN_B64"}, // 126
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 127
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 128
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 129
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 130
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 131
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 132
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 133
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 134
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 135
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 136
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 137
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 138
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 139
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 140
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 141
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 142
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 143
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 144
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 145
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 146
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 147
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 148
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 149
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 150
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 151
                {amdgpu_gfx1250_op_DS_COND_SUB_U32,"DS_COND_SUB_U32"}, // 152
                {amdgpu_gfx1250_op_DS_SUB_CLAMP_U32,"DS_SUB_CLAMP_U32"}, // 153
                {amdgpu_gfx1250_op_DS_PK_ADD_F16,"DS_PK_ADD_F16"}, // 154
                {amdgpu_gfx1250_op_DS_PK_ADD_BF16,"DS_PK_ADD_BF16"}, // 155
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 156
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 157
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 158
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 159
                {amdgpu_gfx1250_op_DS_STORE_B8_D16_HI,"DS_STORE_B8_D16_HI"}, // 160
                {amdgpu_gfx1250_op_DS_STORE_B16_D16_HI,"DS_STORE_B16_D16_HI"}, // 161
                {amdgpu_gfx1250_op_DS_LOAD_U8_D16,"DS_LOAD_U8_D16"}, // 162
                {amdgpu_gfx1250_op_DS_LOAD_U8_D16_HI,"DS_LOAD_U8_D16_HI"}, // 163
                {amdgpu_gfx1250_op_DS_LOAD_I8_D16,"DS_LOAD_I8_D16"}, // 164
                {amdgpu_gfx1250_op_DS_LOAD_I8_D16_HI,"DS_LOAD_I8_D16_HI"}, // 165
                {amdgpu_gfx1250_op_DS_LOAD_U16_D16,"DS_LOAD_U16_D16"}, // 166
                {amdgpu_gfx1250_op_DS_LOAD_U16_D16_HI,"DS_LOAD_U16_D16_HI"}, // 167
                {amdgpu_gfx1250_op_DS_COND_SUB_RTN_U32,"DS_COND_SUB_RTN_U32"}, // 168
                {amdgpu_gfx1250_op_DS_SUB_CLAMP_RTN_U32,"DS_SUB_CLAMP_RTN_U32"}, // 169
                {amdgpu_gfx1250_op_DS_PK_ADD_RTN_F16,"DS_PK_ADD_RTN_F16"}, // 170
                {amdgpu_gfx1250_op_DS_PK_ADD_RTN_BF16,"DS_PK_ADD_RTN_BF16"}, // 171
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 172
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 173
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 174
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 175
                {amdgpu_gfx1250_op_DS_STORE_ADDTID_B32,"DS_STORE_ADDTID_B32"}, // 176
                {amdgpu_gfx1250_op_DS_LOAD_ADDTID_B32,"DS_LOAD_ADDTID_B32"}, // 177
                {amdgpu_gfx1250_op_DS_PERMUTE_B32,"DS_PERMUTE_B32"}, // 178
                {amdgpu_gfx1250_op_DS_BPERMUTE_B32,"DS_BPERMUTE_B32"}, // 179
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 180
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 181
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 182
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 183
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 184
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 185
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 186
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 187
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 188
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 189
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 190
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 191
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 192
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 193
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 194
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 195
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 196
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 197
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 198
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 199
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 200
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 201
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 202
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 203
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 204
                {amdgpu_gfx1250_op_DS_BPERMUTE_FI_B32,"DS_BPERMUTE_FI_B32"}, // 205
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 206
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 207
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 208
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 209
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 210
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 211
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 212
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 213
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 214
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 215
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 216
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 217
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 218
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 219
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 220
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 221
                {amdgpu_gfx1250_op_DS_STORE_B96,"DS_STORE_B96"}, // 222
                {amdgpu_gfx1250_op_DS_STORE_B128,"DS_STORE_B128"}, // 223
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 224
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 225
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 226
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 227
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 228
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 229
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 230
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 231
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 232
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 233
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 234
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 235
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 236
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 237
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 238
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 239
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 240
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 241
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 242
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 243
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 244
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 245
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 246
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 247
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 248
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 249
                {amdgpu_gfx1250_op_DS_LOAD_TR4_B64,"DS_LOAD_TR4_B64"}, // 250
                {amdgpu_gfx1250_op_DS_LOAD_TR6_B96,"DS_LOAD_TR6_B96"}, // 251
                {amdgpu_gfx1250_op_DS_LOAD_TR16_B128,"DS_LOAD_TR16_B128"}, // 252
                {amdgpu_gfx1250_op_DS_LOAD_TR8_B64,"DS_LOAD_TR8_B64"}, // 253
                {amdgpu_gfx1250_op_DS_LOAD_B96,"DS_LOAD_B96"}, // 254
                {amdgpu_gfx1250_op_DS_LOAD_B128,"DS_LOAD_B128"}, // 255
            }; // end ENC_VDS_insn_table
            const amdgpu_gfx1250_insn_entry ENC_VFLAT_insn_table[115] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 1
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 2
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 3
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 4
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 5
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 6
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 7
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 8
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 9
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 10
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 11
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 12
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 13
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_FLAT_LOAD_U8,"FLAT_LOAD_U8"}, // 16
                {amdgpu_gfx1250_op_FLAT_LOAD_I8,"FLAT_LOAD_I8"}, // 17
                {amdgpu_gfx1250_op_FLAT_LOAD_U16,"FLAT_LOAD_U16"}, // 18
                {amdgpu_gfx1250_op_FLAT_LOAD_I16,"FLAT_LOAD_I16"}, // 19
                {amdgpu_gfx1250_op_FLAT_LOAD_B32,"FLAT_LOAD_B32"}, // 20
                {amdgpu_gfx1250_op_FLAT_LOAD_B64,"FLAT_LOAD_B64"}, // 21
                {amdgpu_gfx1250_op_FLAT_LOAD_B96,"FLAT_LOAD_B96"}, // 22
                {amdgpu_gfx1250_op_FLAT_LOAD_B128,"FLAT_LOAD_B128"}, // 23
                {amdgpu_gfx1250_op_FLAT_STORE_B8,"FLAT_STORE_B8"}, // 24
                {amdgpu_gfx1250_op_FLAT_STORE_B16,"FLAT_STORE_B16"}, // 25
                {amdgpu_gfx1250_op_FLAT_STORE_B32,"FLAT_STORE_B32"}, // 26
                {amdgpu_gfx1250_op_FLAT_STORE_B64,"FLAT_STORE_B64"}, // 27
                {amdgpu_gfx1250_op_FLAT_STORE_B96,"FLAT_STORE_B96"}, // 28
                {amdgpu_gfx1250_op_FLAT_STORE_B128,"FLAT_STORE_B128"}, // 29
                {amdgpu_gfx1250_op_FLAT_LOAD_D16_U8,"FLAT_LOAD_D16_U8"}, // 30
                {amdgpu_gfx1250_op_FLAT_LOAD_D16_I8,"FLAT_LOAD_D16_I8"}, // 31
                {amdgpu_gfx1250_op_FLAT_LOAD_D16_B16,"FLAT_LOAD_D16_B16"}, // 32
                {amdgpu_gfx1250_op_FLAT_LOAD_D16_HI_U8,"FLAT_LOAD_D16_HI_U8"}, // 33
                {amdgpu_gfx1250_op_FLAT_LOAD_D16_HI_I8,"FLAT_LOAD_D16_HI_I8"}, // 34
                {amdgpu_gfx1250_op_FLAT_LOAD_D16_HI_B16,"FLAT_LOAD_D16_HI_B16"}, // 35
                {amdgpu_gfx1250_op_FLAT_STORE_D16_HI_B8,"FLAT_STORE_D16_HI_B8"}, // 36
                {amdgpu_gfx1250_op_FLAT_STORE_D16_HI_B16,"FLAT_STORE_D16_HI_B16"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 42
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 49
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 50
                {amdgpu_gfx1250_op_FLAT_ATOMIC_SWAP_B32,"FLAT_ATOMIC_SWAP_B32"}, // 51
                {amdgpu_gfx1250_op_FLAT_ATOMIC_CMPSWAP_B32,"FLAT_ATOMIC_CMPSWAP_B32"}, // 52
                {amdgpu_gfx1250_op_FLAT_ATOMIC_ADD_U32,"FLAT_ATOMIC_ADD_U32"}, // 53
                {amdgpu_gfx1250_op_FLAT_ATOMIC_SUB_U32,"FLAT_ATOMIC_SUB_U32"}, // 54
                {amdgpu_gfx1250_op_FLAT_ATOMIC_SUB_CLAMP_U32,"FLAT_ATOMIC_SUB_CLAMP_U32"}, // 55
                {amdgpu_gfx1250_op_FLAT_ATOMIC_MIN_I32,"FLAT_ATOMIC_MIN_I32"}, // 56
                {amdgpu_gfx1250_op_FLAT_ATOMIC_MIN_U32,"FLAT_ATOMIC_MIN_U32"}, // 57
                {amdgpu_gfx1250_op_FLAT_ATOMIC_MAX_I32,"FLAT_ATOMIC_MAX_I32"}, // 58
                {amdgpu_gfx1250_op_FLAT_ATOMIC_MAX_U32,"FLAT_ATOMIC_MAX_U32"}, // 59
                {amdgpu_gfx1250_op_FLAT_ATOMIC_AND_B32,"FLAT_ATOMIC_AND_B32"}, // 60
                {amdgpu_gfx1250_op_FLAT_ATOMIC_OR_B32,"FLAT_ATOMIC_OR_B32"}, // 61
                {amdgpu_gfx1250_op_FLAT_ATOMIC_XOR_B32,"FLAT_ATOMIC_XOR_B32"}, // 62
                {amdgpu_gfx1250_op_FLAT_ATOMIC_INC_U32,"FLAT_ATOMIC_INC_U32"}, // 63
                {amdgpu_gfx1250_op_FLAT_ATOMIC_DEC_U32,"FLAT_ATOMIC_DEC_U32"}, // 64
                {amdgpu_gfx1250_op_FLAT_ATOMIC_SWAP_B64,"FLAT_ATOMIC_SWAP_B64"}, // 65
                {amdgpu_gfx1250_op_FLAT_ATOMIC_CMPSWAP_B64,"FLAT_ATOMIC_CMPSWAP_B64"}, // 66
                {amdgpu_gfx1250_op_FLAT_ATOMIC_ADD_U64,"FLAT_ATOMIC_ADD_U64"}, // 67
                {amdgpu_gfx1250_op_FLAT_ATOMIC_SUB_U64,"FLAT_ATOMIC_SUB_U64"}, // 68
                {amdgpu_gfx1250_op_FLAT_ATOMIC_MIN_I64,"FLAT_ATOMIC_MIN_I64"}, // 69
                {amdgpu_gfx1250_op_FLAT_ATOMIC_MIN_U64,"FLAT_ATOMIC_MIN_U64"}, // 70
                {amdgpu_gfx1250_op_FLAT_ATOMIC_MAX_I64,"FLAT_ATOMIC_MAX_I64"}, // 71
                {amdgpu_gfx1250_op_FLAT_ATOMIC_MAX_U64,"FLAT_ATOMIC_MAX_U64"}, // 72
                {amdgpu_gfx1250_op_FLAT_ATOMIC_AND_B64,"FLAT_ATOMIC_AND_B64"}, // 73
                {amdgpu_gfx1250_op_FLAT_ATOMIC_OR_B64,"FLAT_ATOMIC_OR_B64"}, // 74
                {amdgpu_gfx1250_op_FLAT_ATOMIC_XOR_B64,"FLAT_ATOMIC_XOR_B64"}, // 75
                {amdgpu_gfx1250_op_FLAT_ATOMIC_INC_U64,"FLAT_ATOMIC_INC_U64"}, // 76
                {amdgpu_gfx1250_op_FLAT_ATOMIC_DEC_U64,"FLAT_ATOMIC_DEC_U64"}, // 77
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_FLAT_ATOMIC_COND_SUB_U32,"FLAT_ATOMIC_COND_SUB_U32"}, // 80
                {amdgpu_gfx1250_op_FLAT_ATOMIC_MIN_NUM_F32,"FLAT_ATOMIC_MIN_NUM_F32"}, // 81
                {amdgpu_gfx1250_op_FLAT_ATOMIC_MAX_NUM_F32,"FLAT_ATOMIC_MAX_NUM_F32"}, // 82
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 83
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 84
                {amdgpu_gfx1250_op_FLAT_ATOMIC_ADD_F64,"FLAT_ATOMIC_ADD_F64"}, // 85
                {amdgpu_gfx1250_op_FLAT_ATOMIC_ADD_F32,"FLAT_ATOMIC_ADD_F32"}, // 86
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 87
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 88
                {amdgpu_gfx1250_op_FLAT_ATOMIC_PK_ADD_F16,"FLAT_ATOMIC_PK_ADD_F16"}, // 89
                {amdgpu_gfx1250_op_FLAT_ATOMIC_PK_ADD_BF16,"FLAT_ATOMIC_PK_ADD_BF16"}, // 90
                {amdgpu_gfx1250_op_FLAT_ATOMIC_MIN_NUM_F64,"FLAT_ATOMIC_MIN_NUM_F64"}, // 91
                {amdgpu_gfx1250_op_FLAT_ATOMIC_MAX_NUM_F64,"FLAT_ATOMIC_MAX_NUM_F64"}, // 92
                {amdgpu_gfx1250_op_FLAT_PREFETCH_B8,"FLAT_PREFETCH_B8"}, // 93
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 94
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 95
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 96
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 97
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 98
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 99
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 100
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 101
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 102
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 103
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 104
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 105
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 106
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 107
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 108
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 109
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 110
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 111
                {amdgpu_gfx1250_op_FLAT_LOAD_MONITOR_B32,"FLAT_LOAD_MONITOR_B32"}, // 112
                {amdgpu_gfx1250_op_FLAT_LOAD_MONITOR_B64,"FLAT_LOAD_MONITOR_B64"}, // 113
                {amdgpu_gfx1250_op_FLAT_LOAD_MONITOR_B128,"FLAT_LOAD_MONITOR_B128"}, // 114
            }; // end ENC_VFLAT_insn_table
            const amdgpu_gfx1250_insn_entry ENC_VGLOBAL_insn_table[117] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 1
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 2
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 3
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 4
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 5
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 6
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 7
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 8
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 9
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 10
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 11
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 12
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 13
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_GLOBAL_LOAD_U8,"GLOBAL_LOAD_U8"}, // 16
                {amdgpu_gfx1250_op_GLOBAL_LOAD_I8,"GLOBAL_LOAD_I8"}, // 17
                {amdgpu_gfx1250_op_GLOBAL_LOAD_U16,"GLOBAL_LOAD_U16"}, // 18
                {amdgpu_gfx1250_op_GLOBAL_LOAD_I16,"GLOBAL_LOAD_I16"}, // 19
                {amdgpu_gfx1250_op_GLOBAL_LOAD_B32,"GLOBAL_LOAD_B32"}, // 20
                {amdgpu_gfx1250_op_GLOBAL_LOAD_B64,"GLOBAL_LOAD_B64"}, // 21
                {amdgpu_gfx1250_op_GLOBAL_LOAD_B96,"GLOBAL_LOAD_B96"}, // 22
                {amdgpu_gfx1250_op_GLOBAL_LOAD_B128,"GLOBAL_LOAD_B128"}, // 23
                {amdgpu_gfx1250_op_GLOBAL_STORE_B8,"GLOBAL_STORE_B8"}, // 24
                {amdgpu_gfx1250_op_GLOBAL_STORE_B16,"GLOBAL_STORE_B16"}, // 25
                {amdgpu_gfx1250_op_GLOBAL_STORE_B32,"GLOBAL_STORE_B32"}, // 26
                {amdgpu_gfx1250_op_GLOBAL_STORE_B64,"GLOBAL_STORE_B64"}, // 27
                {amdgpu_gfx1250_op_GLOBAL_STORE_B96,"GLOBAL_STORE_B96"}, // 28
                {amdgpu_gfx1250_op_GLOBAL_STORE_B128,"GLOBAL_STORE_B128"}, // 29
                {amdgpu_gfx1250_op_GLOBAL_LOAD_D16_U8,"GLOBAL_LOAD_D16_U8"}, // 30
                {amdgpu_gfx1250_op_GLOBAL_LOAD_D16_I8,"GLOBAL_LOAD_D16_I8"}, // 31
                {amdgpu_gfx1250_op_GLOBAL_LOAD_D16_B16,"GLOBAL_LOAD_D16_B16"}, // 32
                {amdgpu_gfx1250_op_GLOBAL_LOAD_D16_HI_U8,"GLOBAL_LOAD_D16_HI_U8"}, // 33
                {amdgpu_gfx1250_op_GLOBAL_LOAD_D16_HI_I8,"GLOBAL_LOAD_D16_HI_I8"}, // 34
                {amdgpu_gfx1250_op_GLOBAL_LOAD_D16_HI_B16,"GLOBAL_LOAD_D16_HI_B16"}, // 35
                {amdgpu_gfx1250_op_GLOBAL_STORE_D16_HI_B8,"GLOBAL_STORE_D16_HI_B8"}, // 36
                {amdgpu_gfx1250_op_GLOBAL_STORE_D16_HI_B16,"GLOBAL_STORE_D16_HI_B16"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 39
                {amdgpu_gfx1250_op_GLOBAL_LOAD_ADDTID_B32,"GLOBAL_LOAD_ADDTID_B32"}, // 40
                {amdgpu_gfx1250_op_GLOBAL_STORE_ADDTID_B32,"GLOBAL_STORE_ADDTID_B32"}, // 41
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 42
                {amdgpu_gfx1250_op_GLOBAL_INV,"GLOBAL_INV"}, // 43
                {amdgpu_gfx1250_op_GLOBAL_WB,"GLOBAL_WB"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 49
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 50
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_SWAP_B32,"GLOBAL_ATOMIC_SWAP_B32"}, // 51
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_CMPSWAP_B32,"GLOBAL_ATOMIC_CMPSWAP_B32"}, // 52
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_ADD_U32,"GLOBAL_ATOMIC_ADD_U32"}, // 53
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_SUB_U32,"GLOBAL_ATOMIC_SUB_U32"}, // 54
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_SUB_CLAMP_U32,"GLOBAL_ATOMIC_SUB_CLAMP_U32"}, // 55
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_MIN_I32,"GLOBAL_ATOMIC_MIN_I32"}, // 56
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_MIN_U32,"GLOBAL_ATOMIC_MIN_U32"}, // 57
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_MAX_I32,"GLOBAL_ATOMIC_MAX_I32"}, // 58
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_MAX_U32,"GLOBAL_ATOMIC_MAX_U32"}, // 59
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_AND_B32,"GLOBAL_ATOMIC_AND_B32"}, // 60
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_OR_B32,"GLOBAL_ATOMIC_OR_B32"}, // 61
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_XOR_B32,"GLOBAL_ATOMIC_XOR_B32"}, // 62
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_INC_U32,"GLOBAL_ATOMIC_INC_U32"}, // 63
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_DEC_U32,"GLOBAL_ATOMIC_DEC_U32"}, // 64
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_SWAP_B64,"GLOBAL_ATOMIC_SWAP_B64"}, // 65
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_CMPSWAP_B64,"GLOBAL_ATOMIC_CMPSWAP_B64"}, // 66
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_ADD_U64,"GLOBAL_ATOMIC_ADD_U64"}, // 67
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_SUB_U64,"GLOBAL_ATOMIC_SUB_U64"}, // 68
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_MIN_I64,"GLOBAL_ATOMIC_MIN_I64"}, // 69
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_MIN_U64,"GLOBAL_ATOMIC_MIN_U64"}, // 70
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_MAX_I64,"GLOBAL_ATOMIC_MAX_I64"}, // 71
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_MAX_U64,"GLOBAL_ATOMIC_MAX_U64"}, // 72
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_AND_B64,"GLOBAL_ATOMIC_AND_B64"}, // 73
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_OR_B64,"GLOBAL_ATOMIC_OR_B64"}, // 74
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_XOR_B64,"GLOBAL_ATOMIC_XOR_B64"}, // 75
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_INC_U64,"GLOBAL_ATOMIC_INC_U64"}, // 76
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_DEC_U64,"GLOBAL_ATOMIC_DEC_U64"}, // 77
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 78
                {amdgpu_gfx1250_op_GLOBAL_WBINV,"GLOBAL_WBINV"}, // 79
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_COND_SUB_U32,"GLOBAL_ATOMIC_COND_SUB_U32"}, // 80
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_MIN_NUM_F32,"GLOBAL_ATOMIC_MIN_NUM_F32"}, // 81
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_MAX_NUM_F32,"GLOBAL_ATOMIC_MAX_NUM_F32"}, // 82
                {amdgpu_gfx1250_op_GLOBAL_LOAD_BLOCK,"GLOBAL_LOAD_BLOCK"}, // 83
                {amdgpu_gfx1250_op_GLOBAL_STORE_BLOCK,"GLOBAL_STORE_BLOCK"}, // 84
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_ADD_F64,"GLOBAL_ATOMIC_ADD_F64"}, // 85
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_ADD_F32,"GLOBAL_ATOMIC_ADD_F32"}, // 86
                {amdgpu_gfx1250_op_GLOBAL_LOAD_TR16_B128,"GLOBAL_LOAD_TR16_B128"}, // 87
                {amdgpu_gfx1250_op_GLOBAL_LOAD_TR8_B64,"GLOBAL_LOAD_TR8_B64"}, // 88
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_PK_ADD_F16,"GLOBAL_ATOMIC_PK_ADD_F16"}, // 89
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_PK_ADD_BF16,"GLOBAL_ATOMIC_PK_ADD_BF16"}, // 90
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_MIN_NUM_F64,"GLOBAL_ATOMIC_MIN_NUM_F64"}, // 91
                {amdgpu_gfx1250_op_GLOBAL_ATOMIC_MAX_NUM_F64,"GLOBAL_ATOMIC_MAX_NUM_F64"}, // 92
                {amdgpu_gfx1250_op_GLOBAL_PREFETCH_B8,"GLOBAL_PREFETCH_B8"}, // 93
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 94
                {amdgpu_gfx1250_op_GLOBAL_LOAD_ASYNC_TO_LDS_B8,"GLOBAL_LOAD_ASYNC_TO_LDS_B8"}, // 95
                {amdgpu_gfx1250_op_GLOBAL_LOAD_ASYNC_TO_LDS_B32,"GLOBAL_LOAD_ASYNC_TO_LDS_B32"}, // 96
                {amdgpu_gfx1250_op_GLOBAL_LOAD_ASYNC_TO_LDS_B64,"GLOBAL_LOAD_ASYNC_TO_LDS_B64"}, // 97
                {amdgpu_gfx1250_op_GLOBAL_LOAD_ASYNC_TO_LDS_B128,"GLOBAL_LOAD_ASYNC_TO_LDS_B128"}, // 98
                {amdgpu_gfx1250_op_GLOBAL_STORE_ASYNC_FROM_LDS_B8,"GLOBAL_STORE_ASYNC_FROM_LDS_B8"}, // 99
                {amdgpu_gfx1250_op_GLOBAL_STORE_ASYNC_FROM_LDS_B32,"GLOBAL_STORE_ASYNC_FROM_LDS_B32"}, // 100
                {amdgpu_gfx1250_op_GLOBAL_STORE_ASYNC_FROM_LDS_B64,"GLOBAL_STORE_ASYNC_FROM_LDS_B64"}, // 101
                {amdgpu_gfx1250_op_GLOBAL_STORE_ASYNC_FROM_LDS_B128,"GLOBAL_STORE_ASYNC_FROM_LDS_B128"}, // 102
                {amdgpu_gfx1250_op_CLUSTER_LOAD_B32,"CLUSTER_LOAD_B32"}, // 103
                {amdgpu_gfx1250_op_CLUSTER_LOAD_B64,"CLUSTER_LOAD_B64"}, // 104
                {amdgpu_gfx1250_op_CLUSTER_LOAD_B128,"CLUSTER_LOAD_B128"}, // 105
                {amdgpu_gfx1250_op_CLUSTER_LOAD_ASYNC_TO_LDS_B8,"CLUSTER_LOAD_ASYNC_TO_LDS_B8"}, // 106
                {amdgpu_gfx1250_op_CLUSTER_LOAD_ASYNC_TO_LDS_B32,"CLUSTER_LOAD_ASYNC_TO_LDS_B32"}, // 107
                {amdgpu_gfx1250_op_CLUSTER_LOAD_ASYNC_TO_LDS_B64,"CLUSTER_LOAD_ASYNC_TO_LDS_B64"}, // 108
                {amdgpu_gfx1250_op_CLUSTER_LOAD_ASYNC_TO_LDS_B128,"CLUSTER_LOAD_ASYNC_TO_LDS_B128"}, // 109
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 110
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 111
                {amdgpu_gfx1250_op_GLOBAL_LOAD_MONITOR_B32,"GLOBAL_LOAD_MONITOR_B32"}, // 112
                {amdgpu_gfx1250_op_GLOBAL_LOAD_MONITOR_B64,"GLOBAL_LOAD_MONITOR_B64"}, // 113
                {amdgpu_gfx1250_op_GLOBAL_LOAD_MONITOR_B128,"GLOBAL_LOAD_MONITOR_B128"}, // 114
                {amdgpu_gfx1250_op_GLOBAL_LOAD_TR4_B64,"GLOBAL_LOAD_TR4_B64"}, // 115
                {amdgpu_gfx1250_op_GLOBAL_LOAD_TR6_B96,"GLOBAL_LOAD_TR6_B96"}, // 116
            }; // end ENC_VGLOBAL_insn_table
            const amdgpu_gfx1250_insn_entry ENC_VIMAGE_insn_table[198] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 1
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 2
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 3
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 4
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 5
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 6
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 7
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 8
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 9
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 10
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 11
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 12
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 13
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 17
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 18
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 19
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 20
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 21
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 22
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 23
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 24
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 25
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 26
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 27
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 28
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 29
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 30
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 31
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 32
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 33
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 34
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 35
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 36
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 42
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 49
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 50
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 51
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 52
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 53
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 59
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 60
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 61
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 64
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 65
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 66
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 67
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 68
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 69
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 72
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 73
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 74
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 75
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 76
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 77
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 80
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 81
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 82
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 83
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 84
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 85
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 86
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 87
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 88
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 89
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 90
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 91
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 92
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 93
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 94
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 95
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 96
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 97
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 98
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 99
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 100
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 101
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 102
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 103
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 104
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 105
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 106
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 107
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 108
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 109
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 110
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 111
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 112
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 113
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 114
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 115
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 116
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 117
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 118
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 119
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 120
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 121
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 122
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 123
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 124
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 125
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 126
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 127
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 128
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 129
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 130
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 131
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 132
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 133
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 134
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 135
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 136
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 137
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 138
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 139
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 140
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 141
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 142
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 143
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 144
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 145
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 146
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 147
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 148
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 149
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 150
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 151
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 152
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 153
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 154
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 155
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 156
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 157
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 158
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 159
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 160
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 161
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 162
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 163
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 164
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 165
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 166
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 167
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 168
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 169
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 170
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 171
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 172
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 173
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 174
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 175
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 176
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 177
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 178
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 179
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 180
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 181
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 182
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 183
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 184
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 185
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 186
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 187
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 188
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 189
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 190
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 191
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 192
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 193
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 194
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 195
                {amdgpu_gfx1250_op_TENSOR_LOAD_TO_LDS,"TENSOR_LOAD_TO_LDS"}, // 196
                {amdgpu_gfx1250_op_TENSOR_STORE_FROM_LDS,"TENSOR_STORE_FROM_LDS"}, // 197
            }; // end ENC_VIMAGE_insn_table
            const amdgpu_gfx1250_insn_entry ENC_VOP1_insn_table[128] =
            {
                {amdgpu_gfx1250_op_V_NOP,"V_NOP"}, // 0
                {amdgpu_gfx1250_op_V_MOV_B32,"V_MOV_B32"}, // 1
                {amdgpu_gfx1250_op_V_READFIRSTLANE_B32,"V_READFIRSTLANE_B32"}, // 2
                {amdgpu_gfx1250_op_V_CVT_I32_F64,"V_CVT_I32_F64"}, // 3
                {amdgpu_gfx1250_op_V_CVT_F64_I32,"V_CVT_F64_I32"}, // 4
                {amdgpu_gfx1250_op_V_CVT_F32_I32,"V_CVT_F32_I32"}, // 5
                {amdgpu_gfx1250_op_V_CVT_F32_U32,"V_CVT_F32_U32"}, // 6
                {amdgpu_gfx1250_op_V_CVT_U32_F32,"V_CVT_U32_F32"}, // 7
                {amdgpu_gfx1250_op_V_CVT_I32_F32,"V_CVT_I32_F32"}, // 8
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 9
                {amdgpu_gfx1250_op_V_CVT_F16_F32,"V_CVT_F16_F32"}, // 10
                {amdgpu_gfx1250_op_V_CVT_F32_F16,"V_CVT_F32_F16"}, // 11
                {amdgpu_gfx1250_op_V_CVT_NEAREST_I32_F32,"V_CVT_NEAREST_I32_F32"}, // 12
                {amdgpu_gfx1250_op_V_CVT_FLOOR_I32_F32,"V_CVT_FLOOR_I32_F32"}, // 13
                {amdgpu_gfx1250_op_V_CVT_OFF_F32_I4,"V_CVT_OFF_F32_I4"}, // 14
                {amdgpu_gfx1250_op_V_CVT_F32_F64,"V_CVT_F32_F64"}, // 15
                {amdgpu_gfx1250_op_V_CVT_F64_F32,"V_CVT_F64_F32"}, // 16
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE0,"V_CVT_F32_UBYTE0"}, // 17
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE1,"V_CVT_F32_UBYTE1"}, // 18
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE2,"V_CVT_F32_UBYTE2"}, // 19
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE3,"V_CVT_F32_UBYTE3"}, // 20
                {amdgpu_gfx1250_op_V_CVT_U32_F64,"V_CVT_U32_F64"}, // 21
                {amdgpu_gfx1250_op_V_CVT_F64_U32,"V_CVT_F64_U32"}, // 22
                {amdgpu_gfx1250_op_V_TRUNC_F64,"V_TRUNC_F64"}, // 23
                {amdgpu_gfx1250_op_V_CEIL_F64,"V_CEIL_F64"}, // 24
                {amdgpu_gfx1250_op_V_RNDNE_F64,"V_RNDNE_F64"}, // 25
                {amdgpu_gfx1250_op_V_FLOOR_F64,"V_FLOOR_F64"}, // 26
                {amdgpu_gfx1250_op_V_PIPEFLUSH,"V_PIPEFLUSH"}, // 27
                {amdgpu_gfx1250_op_V_MOV_B16,"V_MOV_B16"}, // 28
                {amdgpu_gfx1250_op_V_MOV_B64,"V_MOV_B64"}, // 29
                {amdgpu_gfx1250_op_V_TANH_F32,"V_TANH_F32"}, // 30
                {amdgpu_gfx1250_op_V_TANH_F16,"V_TANH_F16"}, // 31
                {amdgpu_gfx1250_op_V_FRACT_F32,"V_FRACT_F32"}, // 32
                {amdgpu_gfx1250_op_V_TRUNC_F32,"V_TRUNC_F32"}, // 33
                {amdgpu_gfx1250_op_V_CEIL_F32,"V_CEIL_F32"}, // 34
                {amdgpu_gfx1250_op_V_RNDNE_F32,"V_RNDNE_F32"}, // 35
                {amdgpu_gfx1250_op_V_FLOOR_F32,"V_FLOOR_F32"}, // 36
                {amdgpu_gfx1250_op_V_EXP_F32,"V_EXP_F32"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_V_LOG_F32,"V_LOG_F32"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_V_RCP_F32,"V_RCP_F32"}, // 42
                {amdgpu_gfx1250_op_V_RCP_IFLAG_F32,"V_RCP_IFLAG_F32"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_V_RSQ_F32,"V_RSQ_F32"}, // 46
                {amdgpu_gfx1250_op_V_RCP_F64,"V_RCP_F64"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_V_RSQ_F64,"V_RSQ_F64"}, // 49
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 50
                {amdgpu_gfx1250_op_V_SQRT_F32,"V_SQRT_F32"}, // 51
                {amdgpu_gfx1250_op_V_SQRT_F64,"V_SQRT_F64"}, // 52
                {amdgpu_gfx1250_op_V_SIN_F32,"V_SIN_F32"}, // 53
                {amdgpu_gfx1250_op_V_COS_F32,"V_COS_F32"}, // 54
                {amdgpu_gfx1250_op_V_NOT_B32,"V_NOT_B32"}, // 55
                {amdgpu_gfx1250_op_V_BFREV_B32,"V_BFREV_B32"}, // 56
                {amdgpu_gfx1250_op_V_CLZ_I32_U32,"V_CLZ_I32_U32"}, // 57
                {amdgpu_gfx1250_op_V_CTZ_I32_B32,"V_CTZ_I32_B32"}, // 58
                {amdgpu_gfx1250_op_V_CLS_I32,"V_CLS_I32"}, // 59
                {amdgpu_gfx1250_op_V_FREXP_EXP_I32_F64,"V_FREXP_EXP_I32_F64"}, // 60
                {amdgpu_gfx1250_op_V_FREXP_MANT_F64,"V_FREXP_MANT_F64"}, // 61
                {amdgpu_gfx1250_op_V_FRACT_F64,"V_FRACT_F64"}, // 62
                {amdgpu_gfx1250_op_V_FREXP_EXP_I32_F32,"V_FREXP_EXP_I32_F32"}, // 63
                {amdgpu_gfx1250_op_V_FREXP_MANT_F32,"V_FREXP_MANT_F32"}, // 64
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 65
                {amdgpu_gfx1250_op_V_MOVRELD_B32,"V_MOVRELD_B32"}, // 66
                {amdgpu_gfx1250_op_V_MOVRELS_B32,"V_MOVRELS_B32"}, // 67
                {amdgpu_gfx1250_op_V_MOVRELSD_B32,"V_MOVRELSD_B32"}, // 68
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 69
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_V_MOVRELSD_2_B32,"V_MOVRELSD_2_B32"}, // 72
                {amdgpu_gfx1250_op_V_PERMLANE16_SWAP_B32,"V_PERMLANE16_SWAP_B32"}, // 73
                {amdgpu_gfx1250_op_V_TANH_BF16,"V_TANH_BF16"}, // 74
                {amdgpu_gfx1250_op_V_PRNG_B32,"V_PRNG_B32"}, // 75
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 76
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 77
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_V_CVT_F16_U16,"V_CVT_F16_U16"}, // 80
                {amdgpu_gfx1250_op_V_CVT_F16_I16,"V_CVT_F16_I16"}, // 81
                {amdgpu_gfx1250_op_V_CVT_U16_F16,"V_CVT_U16_F16"}, // 82
                {amdgpu_gfx1250_op_V_CVT_I16_F16,"V_CVT_I16_F16"}, // 83
                {amdgpu_gfx1250_op_V_RCP_F16,"V_RCP_F16"}, // 84
                {amdgpu_gfx1250_op_V_SQRT_F16,"V_SQRT_F16"}, // 85
                {amdgpu_gfx1250_op_V_RSQ_F16,"V_RSQ_F16"}, // 86
                {amdgpu_gfx1250_op_V_LOG_F16,"V_LOG_F16"}, // 87
                {amdgpu_gfx1250_op_V_EXP_F16,"V_EXP_F16"}, // 88
                {amdgpu_gfx1250_op_V_FREXP_MANT_F16,"V_FREXP_MANT_F16"}, // 89
                {amdgpu_gfx1250_op_V_FREXP_EXP_I16_F16,"V_FREXP_EXP_I16_F16"}, // 90
                {amdgpu_gfx1250_op_V_FLOOR_F16,"V_FLOOR_F16"}, // 91
                {amdgpu_gfx1250_op_V_CEIL_F16,"V_CEIL_F16"}, // 92
                {amdgpu_gfx1250_op_V_TRUNC_F16,"V_TRUNC_F16"}, // 93
                {amdgpu_gfx1250_op_V_RNDNE_F16,"V_RNDNE_F16"}, // 94
                {amdgpu_gfx1250_op_V_FRACT_F16,"V_FRACT_F16"}, // 95
                {amdgpu_gfx1250_op_V_SIN_F16,"V_SIN_F16"}, // 96
                {amdgpu_gfx1250_op_V_COS_F16,"V_COS_F16"}, // 97
                {amdgpu_gfx1250_op_V_SAT_PK_U8_I16,"V_SAT_PK_U8_I16"}, // 98
                {amdgpu_gfx1250_op_V_CVT_NORM_I16_F16,"V_CVT_NORM_I16_F16"}, // 99
                {amdgpu_gfx1250_op_V_CVT_NORM_U16_F16,"V_CVT_NORM_U16_F16"}, // 100
                {amdgpu_gfx1250_op_V_SWAP_B32,"V_SWAP_B32"}, // 101
                {amdgpu_gfx1250_op_V_SWAP_B16,"V_SWAP_B16"}, // 102
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 103
                {amdgpu_gfx1250_op_V_SWAPREL_B32,"V_SWAPREL_B32"}, // 104
                {amdgpu_gfx1250_op_V_NOT_B16,"V_NOT_B16"}, // 105
                {amdgpu_gfx1250_op_V_CVT_I32_I16,"V_CVT_I32_I16"}, // 106
                {amdgpu_gfx1250_op_V_CVT_U32_U16,"V_CVT_U32_U16"}, // 107
                {amdgpu_gfx1250_op_V_CVT_F32_FP8,"V_CVT_F32_FP8"}, // 108
                {amdgpu_gfx1250_op_V_CVT_F32_BF8,"V_CVT_F32_BF8"}, // 109
                {amdgpu_gfx1250_op_V_CVT_PK_F32_FP8,"V_CVT_PK_F32_FP8"}, // 110
                {amdgpu_gfx1250_op_V_CVT_PK_F32_BF8,"V_CVT_PK_F32_BF8"}, // 111
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 112
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 113
                {amdgpu_gfx1250_op_V_CVT_F32_BF16,"V_CVT_F32_BF16"}, // 114
                {amdgpu_gfx1250_op_V_SAT_PK4_I4_I8,"V_SAT_PK4_I4_I8"}, // 115
                {amdgpu_gfx1250_op_V_SAT_PK4_U4_U8,"V_SAT_PK4_U4_U8"}, // 116
                {amdgpu_gfx1250_op_V_CVT_PK_F16_FP8,"V_CVT_PK_F16_FP8"}, // 117
                {amdgpu_gfx1250_op_V_CVT_PK_F16_BF8,"V_CVT_PK_F16_BF8"}, // 118
                {amdgpu_gfx1250_op_V_CVT_F16_FP8,"V_CVT_F16_FP8"}, // 119
                {amdgpu_gfx1250_op_V_CVT_F16_BF8,"V_CVT_F16_BF8"}, // 120
                {amdgpu_gfx1250_op_V_RCP_BF16,"V_RCP_BF16"}, // 121
                {amdgpu_gfx1250_op_V_SQRT_BF16,"V_SQRT_BF16"}, // 122
                {amdgpu_gfx1250_op_V_RSQ_BF16,"V_RSQ_BF16"}, // 123
                {amdgpu_gfx1250_op_V_LOG_BF16,"V_LOG_BF16"}, // 124
                {amdgpu_gfx1250_op_V_EXP_BF16,"V_EXP_BF16"}, // 125
                {amdgpu_gfx1250_op_V_SIN_BF16,"V_SIN_BF16"}, // 126
                {amdgpu_gfx1250_op_V_COS_BF16,"V_COS_BF16"}, // 127
            }; // end ENC_VOP1_insn_table
            const amdgpu_gfx1250_insn_entry ENC_VOP3_insn_table[886] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_V_CMP_LT_F16,"V_CMP_LT_F16"}, // 1
                {amdgpu_gfx1250_op_V_CMP_EQ_F16,"V_CMP_EQ_F16"}, // 2
                {amdgpu_gfx1250_op_V_CMP_LE_F16,"V_CMP_LE_F16"}, // 3
                {amdgpu_gfx1250_op_V_CMP_GT_F16,"V_CMP_GT_F16"}, // 4
                {amdgpu_gfx1250_op_V_CMP_LG_F16,"V_CMP_LG_F16"}, // 5
                {amdgpu_gfx1250_op_V_CMP_GE_F16,"V_CMP_GE_F16"}, // 6
                {amdgpu_gfx1250_op_V_CMP_O_F16,"V_CMP_O_F16"}, // 7
                {amdgpu_gfx1250_op_V_CMP_U_F16,"V_CMP_U_F16"}, // 8
                {amdgpu_gfx1250_op_V_CMP_NGE_F16,"V_CMP_NGE_F16"}, // 9
                {amdgpu_gfx1250_op_V_CMP_NLG_F16,"V_CMP_NLG_F16"}, // 10
                {amdgpu_gfx1250_op_V_CMP_NGT_F16,"V_CMP_NGT_F16"}, // 11
                {amdgpu_gfx1250_op_V_CMP_NLE_F16,"V_CMP_NLE_F16"}, // 12
                {amdgpu_gfx1250_op_V_CMP_NEQ_F16,"V_CMP_NEQ_F16"}, // 13
                {amdgpu_gfx1250_op_V_CMP_NLT_F16,"V_CMP_NLT_F16"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_V_CMP_LT_F32,"V_CMP_LT_F32"}, // 17
                {amdgpu_gfx1250_op_V_CMP_EQ_F32,"V_CMP_EQ_F32"}, // 18
                {amdgpu_gfx1250_op_V_CMP_LE_F32,"V_CMP_LE_F32"}, // 19
                {amdgpu_gfx1250_op_V_CMP_GT_F32,"V_CMP_GT_F32"}, // 20
                {amdgpu_gfx1250_op_V_CMP_LG_F32,"V_CMP_LG_F32"}, // 21
                {amdgpu_gfx1250_op_V_CMP_GE_F32,"V_CMP_GE_F32"}, // 22
                {amdgpu_gfx1250_op_V_CMP_O_F32,"V_CMP_O_F32"}, // 23
                {amdgpu_gfx1250_op_V_CMP_U_F32,"V_CMP_U_F32"}, // 24
                {amdgpu_gfx1250_op_V_CMP_NGE_F32,"V_CMP_NGE_F32"}, // 25
                {amdgpu_gfx1250_op_V_CMP_NLG_F32,"V_CMP_NLG_F32"}, // 26
                {amdgpu_gfx1250_op_V_CMP_NGT_F32,"V_CMP_NGT_F32"}, // 27
                {amdgpu_gfx1250_op_V_CMP_NLE_F32,"V_CMP_NLE_F32"}, // 28
                {amdgpu_gfx1250_op_V_CMP_NEQ_F32,"V_CMP_NEQ_F32"}, // 29
                {amdgpu_gfx1250_op_V_CMP_NLT_F32,"V_CMP_NLT_F32"}, // 30
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 31
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 32
                {amdgpu_gfx1250_op_V_CMP_LT_F64,"V_CMP_LT_F64"}, // 33
                {amdgpu_gfx1250_op_V_CMP_EQ_F64,"V_CMP_EQ_F64"}, // 34
                {amdgpu_gfx1250_op_V_CMP_LE_F64,"V_CMP_LE_F64"}, // 35
                {amdgpu_gfx1250_op_V_CMP_GT_F64,"V_CMP_GT_F64"}, // 36
                {amdgpu_gfx1250_op_V_CMP_LG_F64,"V_CMP_LG_F64"}, // 37
                {amdgpu_gfx1250_op_V_CMP_GE_F64,"V_CMP_GE_F64"}, // 38
                {amdgpu_gfx1250_op_V_CMP_O_F64,"V_CMP_O_F64"}, // 39
                {amdgpu_gfx1250_op_V_CMP_U_F64,"V_CMP_U_F64"}, // 40
                {amdgpu_gfx1250_op_V_CMP_NGE_F64,"V_CMP_NGE_F64"}, // 41
                {amdgpu_gfx1250_op_V_CMP_NLG_F64,"V_CMP_NLG_F64"}, // 42
                {amdgpu_gfx1250_op_V_CMP_NGT_F64,"V_CMP_NGT_F64"}, // 43
                {amdgpu_gfx1250_op_V_CMP_NLE_F64,"V_CMP_NLE_F64"}, // 44
                {amdgpu_gfx1250_op_V_CMP_NEQ_F64,"V_CMP_NEQ_F64"}, // 45
                {amdgpu_gfx1250_op_V_CMP_NLT_F64,"V_CMP_NLT_F64"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_V_CMP_LT_I16,"V_CMP_LT_I16"}, // 49
                {amdgpu_gfx1250_op_V_CMP_EQ_I16,"V_CMP_EQ_I16"}, // 50
                {amdgpu_gfx1250_op_V_CMP_LE_I16,"V_CMP_LE_I16"}, // 51
                {amdgpu_gfx1250_op_V_CMP_GT_I16,"V_CMP_GT_I16"}, // 52
                {amdgpu_gfx1250_op_V_CMP_NE_I16,"V_CMP_NE_I16"}, // 53
                {amdgpu_gfx1250_op_V_CMP_GE_I16,"V_CMP_GE_I16"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_V_CMP_LT_U16,"V_CMP_LT_U16"}, // 57
                {amdgpu_gfx1250_op_V_CMP_EQ_U16,"V_CMP_EQ_U16"}, // 58
                {amdgpu_gfx1250_op_V_CMP_LE_U16,"V_CMP_LE_U16"}, // 59
                {amdgpu_gfx1250_op_V_CMP_GT_U16,"V_CMP_GT_U16"}, // 60
                {amdgpu_gfx1250_op_V_CMP_NE_U16,"V_CMP_NE_U16"}, // 61
                {amdgpu_gfx1250_op_V_CMP_GE_U16,"V_CMP_GE_U16"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 64
                {amdgpu_gfx1250_op_V_CMP_LT_I32,"V_CMP_LT_I32"}, // 65
                {amdgpu_gfx1250_op_V_CMP_EQ_I32,"V_CMP_EQ_I32"}, // 66
                {amdgpu_gfx1250_op_V_CMP_LE_I32,"V_CMP_LE_I32"}, // 67
                {amdgpu_gfx1250_op_V_CMP_GT_I32,"V_CMP_GT_I32"}, // 68
                {amdgpu_gfx1250_op_V_CMP_NE_I32,"V_CMP_NE_I32"}, // 69
                {amdgpu_gfx1250_op_V_CMP_GE_I32,"V_CMP_GE_I32"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 72
                {amdgpu_gfx1250_op_V_CMP_LT_U32,"V_CMP_LT_U32"}, // 73
                {amdgpu_gfx1250_op_V_CMP_EQ_U32,"V_CMP_EQ_U32"}, // 74
                {amdgpu_gfx1250_op_V_CMP_LE_U32,"V_CMP_LE_U32"}, // 75
                {amdgpu_gfx1250_op_V_CMP_GT_U32,"V_CMP_GT_U32"}, // 76
                {amdgpu_gfx1250_op_V_CMP_NE_U32,"V_CMP_NE_U32"}, // 77
                {amdgpu_gfx1250_op_V_CMP_GE_U32,"V_CMP_GE_U32"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 80
                {amdgpu_gfx1250_op_V_CMP_LT_I64,"V_CMP_LT_I64"}, // 81
                {amdgpu_gfx1250_op_V_CMP_EQ_I64,"V_CMP_EQ_I64"}, // 82
                {amdgpu_gfx1250_op_V_CMP_LE_I64,"V_CMP_LE_I64"}, // 83
                {amdgpu_gfx1250_op_V_CMP_GT_I64,"V_CMP_GT_I64"}, // 84
                {amdgpu_gfx1250_op_V_CMP_NE_I64,"V_CMP_NE_I64"}, // 85
                {amdgpu_gfx1250_op_V_CMP_GE_I64,"V_CMP_GE_I64"}, // 86
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 87
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 88
                {amdgpu_gfx1250_op_V_CMP_LT_U64,"V_CMP_LT_U64"}, // 89
                {amdgpu_gfx1250_op_V_CMP_EQ_U64,"V_CMP_EQ_U64"}, // 90
                {amdgpu_gfx1250_op_V_CMP_LE_U64,"V_CMP_LE_U64"}, // 91
                {amdgpu_gfx1250_op_V_CMP_GT_U64,"V_CMP_GT_U64"}, // 92
                {amdgpu_gfx1250_op_V_CMP_NE_U64,"V_CMP_NE_U64"}, // 93
                {amdgpu_gfx1250_op_V_CMP_GE_U64,"V_CMP_GE_U64"}, // 94
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 95
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 96
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 97
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 98
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 99
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 100
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 101
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 102
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 103
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 104
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 105
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 106
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 107
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 108
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 109
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 110
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 111
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 112
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 113
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 114
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 115
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 116
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 117
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 118
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 119
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 120
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 121
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 122
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 123
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 124
                {amdgpu_gfx1250_op_V_CMP_CLASS_F16,"V_CMP_CLASS_F16"}, // 125
                {amdgpu_gfx1250_op_V_CMP_CLASS_F32,"V_CMP_CLASS_F32"}, // 126
                {amdgpu_gfx1250_op_V_CMP_CLASS_F64,"V_CMP_CLASS_F64"}, // 127
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 128
                {amdgpu_gfx1250_op_V_CMPX_LT_F16,"V_CMPX_LT_F16"}, // 129
                {amdgpu_gfx1250_op_V_CMPX_EQ_F16,"V_CMPX_EQ_F16"}, // 130
                {amdgpu_gfx1250_op_V_CMPX_LE_F16,"V_CMPX_LE_F16"}, // 131
                {amdgpu_gfx1250_op_V_CMPX_GT_F16,"V_CMPX_GT_F16"}, // 132
                {amdgpu_gfx1250_op_V_CMPX_LG_F16,"V_CMPX_LG_F16"}, // 133
                {amdgpu_gfx1250_op_V_CMPX_GE_F16,"V_CMPX_GE_F16"}, // 134
                {amdgpu_gfx1250_op_V_CMPX_O_F16,"V_CMPX_O_F16"}, // 135
                {amdgpu_gfx1250_op_V_CMPX_U_F16,"V_CMPX_U_F16"}, // 136
                {amdgpu_gfx1250_op_V_CMPX_NGE_F16,"V_CMPX_NGE_F16"}, // 137
                {amdgpu_gfx1250_op_V_CMPX_NLG_F16,"V_CMPX_NLG_F16"}, // 138
                {amdgpu_gfx1250_op_V_CMPX_NGT_F16,"V_CMPX_NGT_F16"}, // 139
                {amdgpu_gfx1250_op_V_CMPX_NLE_F16,"V_CMPX_NLE_F16"}, // 140
                {amdgpu_gfx1250_op_V_CMPX_NEQ_F16,"V_CMPX_NEQ_F16"}, // 141
                {amdgpu_gfx1250_op_V_CMPX_NLT_F16,"V_CMPX_NLT_F16"}, // 142
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 143
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 144
                {amdgpu_gfx1250_op_V_CMPX_LT_F32,"V_CMPX_LT_F32"}, // 145
                {amdgpu_gfx1250_op_V_CMPX_EQ_F32,"V_CMPX_EQ_F32"}, // 146
                {amdgpu_gfx1250_op_V_CMPX_LE_F32,"V_CMPX_LE_F32"}, // 147
                {amdgpu_gfx1250_op_V_CMPX_GT_F32,"V_CMPX_GT_F32"}, // 148
                {amdgpu_gfx1250_op_V_CMPX_LG_F32,"V_CMPX_LG_F32"}, // 149
                {amdgpu_gfx1250_op_V_CMPX_GE_F32,"V_CMPX_GE_F32"}, // 150
                {amdgpu_gfx1250_op_V_CMPX_O_F32,"V_CMPX_O_F32"}, // 151
                {amdgpu_gfx1250_op_V_CMPX_U_F32,"V_CMPX_U_F32"}, // 152
                {amdgpu_gfx1250_op_V_CMPX_NGE_F32,"V_CMPX_NGE_F32"}, // 153
                {amdgpu_gfx1250_op_V_CMPX_NLG_F32,"V_CMPX_NLG_F32"}, // 154
                {amdgpu_gfx1250_op_V_CMPX_NGT_F32,"V_CMPX_NGT_F32"}, // 155
                {amdgpu_gfx1250_op_V_CMPX_NLE_F32,"V_CMPX_NLE_F32"}, // 156
                {amdgpu_gfx1250_op_V_CMPX_NEQ_F32,"V_CMPX_NEQ_F32"}, // 157
                {amdgpu_gfx1250_op_V_CMPX_NLT_F32,"V_CMPX_NLT_F32"}, // 158
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 159
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 160
                {amdgpu_gfx1250_op_V_CMPX_LT_F64,"V_CMPX_LT_F64"}, // 161
                {amdgpu_gfx1250_op_V_CMPX_EQ_F64,"V_CMPX_EQ_F64"}, // 162
                {amdgpu_gfx1250_op_V_CMPX_LE_F64,"V_CMPX_LE_F64"}, // 163
                {amdgpu_gfx1250_op_V_CMPX_GT_F64,"V_CMPX_GT_F64"}, // 164
                {amdgpu_gfx1250_op_V_CMPX_LG_F64,"V_CMPX_LG_F64"}, // 165
                {amdgpu_gfx1250_op_V_CMPX_GE_F64,"V_CMPX_GE_F64"}, // 166
                {amdgpu_gfx1250_op_V_CMPX_O_F64,"V_CMPX_O_F64"}, // 167
                {amdgpu_gfx1250_op_V_CMPX_U_F64,"V_CMPX_U_F64"}, // 168
                {amdgpu_gfx1250_op_V_CMPX_NGE_F64,"V_CMPX_NGE_F64"}, // 169
                {amdgpu_gfx1250_op_V_CMPX_NLG_F64,"V_CMPX_NLG_F64"}, // 170
                {amdgpu_gfx1250_op_V_CMPX_NGT_F64,"V_CMPX_NGT_F64"}, // 171
                {amdgpu_gfx1250_op_V_CMPX_NLE_F64,"V_CMPX_NLE_F64"}, // 172
                {amdgpu_gfx1250_op_V_CMPX_NEQ_F64,"V_CMPX_NEQ_F64"}, // 173
                {amdgpu_gfx1250_op_V_CMPX_NLT_F64,"V_CMPX_NLT_F64"}, // 174
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 175
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 176
                {amdgpu_gfx1250_op_V_CMPX_LT_I16,"V_CMPX_LT_I16"}, // 177
                {amdgpu_gfx1250_op_V_CMPX_EQ_I16,"V_CMPX_EQ_I16"}, // 178
                {amdgpu_gfx1250_op_V_CMPX_LE_I16,"V_CMPX_LE_I16"}, // 179
                {amdgpu_gfx1250_op_V_CMPX_GT_I16,"V_CMPX_GT_I16"}, // 180
                {amdgpu_gfx1250_op_V_CMPX_NE_I16,"V_CMPX_NE_I16"}, // 181
                {amdgpu_gfx1250_op_V_CMPX_GE_I16,"V_CMPX_GE_I16"}, // 182
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 183
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 184
                {amdgpu_gfx1250_op_V_CMPX_LT_U16,"V_CMPX_LT_U16"}, // 185
                {amdgpu_gfx1250_op_V_CMPX_EQ_U16,"V_CMPX_EQ_U16"}, // 186
                {amdgpu_gfx1250_op_V_CMPX_LE_U16,"V_CMPX_LE_U16"}, // 187
                {amdgpu_gfx1250_op_V_CMPX_GT_U16,"V_CMPX_GT_U16"}, // 188
                {amdgpu_gfx1250_op_V_CMPX_NE_U16,"V_CMPX_NE_U16"}, // 189
                {amdgpu_gfx1250_op_V_CMPX_GE_U16,"V_CMPX_GE_U16"}, // 190
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 191
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 192
                {amdgpu_gfx1250_op_V_CMPX_LT_I32,"V_CMPX_LT_I32"}, // 193
                {amdgpu_gfx1250_op_V_CMPX_EQ_I32,"V_CMPX_EQ_I32"}, // 194
                {amdgpu_gfx1250_op_V_CMPX_LE_I32,"V_CMPX_LE_I32"}, // 195
                {amdgpu_gfx1250_op_V_CMPX_GT_I32,"V_CMPX_GT_I32"}, // 196
                {amdgpu_gfx1250_op_V_CMPX_NE_I32,"V_CMPX_NE_I32"}, // 197
                {amdgpu_gfx1250_op_V_CMPX_GE_I32,"V_CMPX_GE_I32"}, // 198
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 199
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 200
                {amdgpu_gfx1250_op_V_CMPX_LT_U32,"V_CMPX_LT_U32"}, // 201
                {amdgpu_gfx1250_op_V_CMPX_EQ_U32,"V_CMPX_EQ_U32"}, // 202
                {amdgpu_gfx1250_op_V_CMPX_LE_U32,"V_CMPX_LE_U32"}, // 203
                {amdgpu_gfx1250_op_V_CMPX_GT_U32,"V_CMPX_GT_U32"}, // 204
                {amdgpu_gfx1250_op_V_CMPX_NE_U32,"V_CMPX_NE_U32"}, // 205
                {amdgpu_gfx1250_op_V_CMPX_GE_U32,"V_CMPX_GE_U32"}, // 206
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 207
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 208
                {amdgpu_gfx1250_op_V_CMPX_LT_I64,"V_CMPX_LT_I64"}, // 209
                {amdgpu_gfx1250_op_V_CMPX_EQ_I64,"V_CMPX_EQ_I64"}, // 210
                {amdgpu_gfx1250_op_V_CMPX_LE_I64,"V_CMPX_LE_I64"}, // 211
                {amdgpu_gfx1250_op_V_CMPX_GT_I64,"V_CMPX_GT_I64"}, // 212
                {amdgpu_gfx1250_op_V_CMPX_NE_I64,"V_CMPX_NE_I64"}, // 213
                {amdgpu_gfx1250_op_V_CMPX_GE_I64,"V_CMPX_GE_I64"}, // 214
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 215
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 216
                {amdgpu_gfx1250_op_V_CMPX_LT_U64,"V_CMPX_LT_U64"}, // 217
                {amdgpu_gfx1250_op_V_CMPX_EQ_U64,"V_CMPX_EQ_U64"}, // 218
                {amdgpu_gfx1250_op_V_CMPX_LE_U64,"V_CMPX_LE_U64"}, // 219
                {amdgpu_gfx1250_op_V_CMPX_GT_U64,"V_CMPX_GT_U64"}, // 220
                {amdgpu_gfx1250_op_V_CMPX_NE_U64,"V_CMPX_NE_U64"}, // 221
                {amdgpu_gfx1250_op_V_CMPX_GE_U64,"V_CMPX_GE_U64"}, // 222
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 223
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 224
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 225
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 226
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 227
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 228
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 229
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 230
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 231
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 232
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 233
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 234
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 235
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 236
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 237
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 238
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 239
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 240
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 241
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 242
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 243
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 244
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 245
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 246
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 247
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 248
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 249
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 250
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 251
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 252
                {amdgpu_gfx1250_op_V_CMPX_CLASS_F16,"V_CMPX_CLASS_F16"}, // 253
                {amdgpu_gfx1250_op_V_CMPX_CLASS_F32,"V_CMPX_CLASS_F32"}, // 254
                {amdgpu_gfx1250_op_V_CMPX_CLASS_F64,"V_CMPX_CLASS_F64"}, // 255
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 256
                {amdgpu_gfx1250_op_V_CNDMASK_B32,"V_CNDMASK_B32"}, // 257
                {amdgpu_gfx1250_op_V_ADD_F64,"V_ADD_F64"}, // 258
                {amdgpu_gfx1250_op_V_ADD_F32,"V_ADD_F32"}, // 259
                {amdgpu_gfx1250_op_V_SUB_F32,"V_SUB_F32"}, // 260
                {amdgpu_gfx1250_op_V_SUBREV_F32,"V_SUBREV_F32"}, // 261
                {amdgpu_gfx1250_op_V_MUL_F64,"V_MUL_F64"}, // 262
                {amdgpu_gfx1250_op_V_MUL_DX9_ZERO_F32,"V_MUL_DX9_ZERO_F32"}, // 263
                {amdgpu_gfx1250_op_V_MUL_F32,"V_MUL_F32"}, // 264
                {amdgpu_gfx1250_op_V_MUL_I32_I24,"V_MUL_I32_I24"}, // 265
                {amdgpu_gfx1250_op_V_MUL_HI_I32_I24,"V_MUL_HI_I32_I24"}, // 266
                {amdgpu_gfx1250_op_V_MUL_U32_U24,"V_MUL_U32_U24"}, // 267
                {amdgpu_gfx1250_op_V_MUL_HI_U32_U24,"V_MUL_HI_U32_U24"}, // 268
                {amdgpu_gfx1250_op_V_MIN_NUM_F64,"V_MIN_NUM_F64"}, // 269
                {amdgpu_gfx1250_op_V_MAX_NUM_F64,"V_MAX_NUM_F64"}, // 270
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 271
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 272
                {amdgpu_gfx1250_op_V_MIN_I32,"V_MIN_I32"}, // 273
                {amdgpu_gfx1250_op_V_MAX_I32,"V_MAX_I32"}, // 274
                {amdgpu_gfx1250_op_V_MIN_U32,"V_MIN_U32"}, // 275
                {amdgpu_gfx1250_op_V_MAX_U32,"V_MAX_U32"}, // 276
                {amdgpu_gfx1250_op_V_MIN_NUM_F32,"V_MIN_NUM_F32"}, // 277
                {amdgpu_gfx1250_op_V_MAX_NUM_F32,"V_MAX_NUM_F32"}, // 278
                {amdgpu_gfx1250_op_V_FMAC_F64,"V_FMAC_F64"}, // 279
                {amdgpu_gfx1250_op_V_LSHLREV_B32,"V_LSHLREV_B32"}, // 280
                {amdgpu_gfx1250_op_V_LSHRREV_B32,"V_LSHRREV_B32"}, // 281
                {amdgpu_gfx1250_op_V_ASHRREV_I32,"V_ASHRREV_I32"}, // 282
                {amdgpu_gfx1250_op_V_AND_B32,"V_AND_B32"}, // 283
                {amdgpu_gfx1250_op_V_OR_B32,"V_OR_B32"}, // 284
                {amdgpu_gfx1250_op_V_XOR_B32,"V_XOR_B32"}, // 285
                {amdgpu_gfx1250_op_V_XNOR_B32,"V_XNOR_B32"}, // 286
                {amdgpu_gfx1250_op_V_LSHLREV_B64,"V_LSHLREV_B64"}, // 287
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 288
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 289
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 290
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 291
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 292
                {amdgpu_gfx1250_op_V_ADD_NC_U32,"V_ADD_NC_U32"}, // 293
                {amdgpu_gfx1250_op_V_SUB_NC_U32,"V_SUB_NC_U32"}, // 294
                {amdgpu_gfx1250_op_V_SUBREV_NC_U32,"V_SUBREV_NC_U32"}, // 295
                {amdgpu_gfx1250_op_V_ADD_NC_U64,"V_ADD_NC_U64"}, // 296
                {amdgpu_gfx1250_op_V_SUB_NC_U64,"V_SUB_NC_U64"}, // 297
                {amdgpu_gfx1250_op_V_MUL_U64,"V_MUL_U64"}, // 298
                {amdgpu_gfx1250_op_V_FMAC_F32,"V_FMAC_F32"}, // 299
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 300
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 301
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 302
                {amdgpu_gfx1250_op_V_CVT_PK_RTZ_F16_F32,"V_CVT_PK_RTZ_F16_F32"}, // 303
                {amdgpu_gfx1250_op_V_MIN_NUM_F16,"V_MIN_NUM_F16"}, // 304
                {amdgpu_gfx1250_op_V_MAX_NUM_F16,"V_MAX_NUM_F16"}, // 305
                {amdgpu_gfx1250_op_V_ADD_F16,"V_ADD_F16"}, // 306
                {amdgpu_gfx1250_op_V_SUB_F16,"V_SUB_F16"}, // 307
                {amdgpu_gfx1250_op_V_SUBREV_F16,"V_SUBREV_F16"}, // 308
                {amdgpu_gfx1250_op_V_MUL_F16,"V_MUL_F16"}, // 309
                {amdgpu_gfx1250_op_V_FMAC_F16,"V_FMAC_F16"}, // 310
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 311
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 312
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 313
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 314
                {amdgpu_gfx1250_op_V_LDEXP_F16,"V_LDEXP_F16"}, // 315
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 316
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 317
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 318
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 319
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 320
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 321
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 322
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 323
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 324
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 325
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 326
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 327
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 328
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 329
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 330
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 331
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 332
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 333
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 334
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 335
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 336
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 337
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 338
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 339
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 340
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 341
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 342
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 343
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 344
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 345
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 346
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 347
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 348
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 349
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 350
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 351
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 352
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 353
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 354
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 355
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 356
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 357
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 358
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 359
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 360
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 361
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 362
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 363
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 364
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 365
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 366
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 367
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 368
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 369
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 370
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 371
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 372
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 373
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 374
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 375
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 376
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 377
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 378
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 379
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 380
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 381
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 382
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 383
                {amdgpu_gfx1250_op_V_NOP,"V_NOP"}, // 384
                {amdgpu_gfx1250_op_V_MOV_B32,"V_MOV_B32"}, // 385
                {amdgpu_gfx1250_op_V_READFIRSTLANE_B32,"V_READFIRSTLANE_B32"}, // 386
                {amdgpu_gfx1250_op_V_CVT_I32_F64,"V_CVT_I32_F64"}, // 387
                {amdgpu_gfx1250_op_V_CVT_F64_I32,"V_CVT_F64_I32"}, // 388
                {amdgpu_gfx1250_op_V_CVT_F32_I32,"V_CVT_F32_I32"}, // 389
                {amdgpu_gfx1250_op_V_CVT_F32_U32,"V_CVT_F32_U32"}, // 390
                {amdgpu_gfx1250_op_V_CVT_U32_F32,"V_CVT_U32_F32"}, // 391
                {amdgpu_gfx1250_op_V_CVT_I32_F32,"V_CVT_I32_F32"}, // 392
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 393
                {amdgpu_gfx1250_op_V_CVT_F16_F32,"V_CVT_F16_F32"}, // 394
                {amdgpu_gfx1250_op_V_CVT_F32_F16,"V_CVT_F32_F16"}, // 395
                {amdgpu_gfx1250_op_V_CVT_NEAREST_I32_F32,"V_CVT_NEAREST_I32_F32"}, // 396
                {amdgpu_gfx1250_op_V_CVT_FLOOR_I32_F32,"V_CVT_FLOOR_I32_F32"}, // 397
                {amdgpu_gfx1250_op_V_CVT_OFF_F32_I4,"V_CVT_OFF_F32_I4"}, // 398
                {amdgpu_gfx1250_op_V_CVT_F32_F64,"V_CVT_F32_F64"}, // 399
                {amdgpu_gfx1250_op_V_CVT_F64_F32,"V_CVT_F64_F32"}, // 400
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE0,"V_CVT_F32_UBYTE0"}, // 401
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE1,"V_CVT_F32_UBYTE1"}, // 402
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE2,"V_CVT_F32_UBYTE2"}, // 403
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE3,"V_CVT_F32_UBYTE3"}, // 404
                {amdgpu_gfx1250_op_V_CVT_U32_F64,"V_CVT_U32_F64"}, // 405
                {amdgpu_gfx1250_op_V_CVT_F64_U32,"V_CVT_F64_U32"}, // 406
                {amdgpu_gfx1250_op_V_TRUNC_F64,"V_TRUNC_F64"}, // 407
                {amdgpu_gfx1250_op_V_CEIL_F64,"V_CEIL_F64"}, // 408
                {amdgpu_gfx1250_op_V_RNDNE_F64,"V_RNDNE_F64"}, // 409
                {amdgpu_gfx1250_op_V_FLOOR_F64,"V_FLOOR_F64"}, // 410
                {amdgpu_gfx1250_op_V_PIPEFLUSH,"V_PIPEFLUSH"}, // 411
                {amdgpu_gfx1250_op_V_MOV_B16,"V_MOV_B16"}, // 412
                {amdgpu_gfx1250_op_V_MOV_B64,"V_MOV_B64"}, // 413
                {amdgpu_gfx1250_op_V_TANH_F32,"V_TANH_F32"}, // 414
                {amdgpu_gfx1250_op_V_TANH_F16,"V_TANH_F16"}, // 415
                {amdgpu_gfx1250_op_V_FRACT_F32,"V_FRACT_F32"}, // 416
                {amdgpu_gfx1250_op_V_TRUNC_F32,"V_TRUNC_F32"}, // 417
                {amdgpu_gfx1250_op_V_CEIL_F32,"V_CEIL_F32"}, // 418
                {amdgpu_gfx1250_op_V_RNDNE_F32,"V_RNDNE_F32"}, // 419
                {amdgpu_gfx1250_op_V_FLOOR_F32,"V_FLOOR_F32"}, // 420
                {amdgpu_gfx1250_op_V_EXP_F32,"V_EXP_F32"}, // 421
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 422
                {amdgpu_gfx1250_op_V_LOG_F32,"V_LOG_F32"}, // 423
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 424
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 425
                {amdgpu_gfx1250_op_V_RCP_F32,"V_RCP_F32"}, // 426
                {amdgpu_gfx1250_op_V_RCP_IFLAG_F32,"V_RCP_IFLAG_F32"}, // 427
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 428
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 429
                {amdgpu_gfx1250_op_V_RSQ_F32,"V_RSQ_F32"}, // 430
                {amdgpu_gfx1250_op_V_RCP_F64,"V_RCP_F64"}, // 431
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 432
                {amdgpu_gfx1250_op_V_RSQ_F64,"V_RSQ_F64"}, // 433
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 434
                {amdgpu_gfx1250_op_V_SQRT_F32,"V_SQRT_F32"}, // 435
                {amdgpu_gfx1250_op_V_SQRT_F64,"V_SQRT_F64"}, // 436
                {amdgpu_gfx1250_op_V_SIN_F32,"V_SIN_F32"}, // 437
                {amdgpu_gfx1250_op_V_COS_F32,"V_COS_F32"}, // 438
                {amdgpu_gfx1250_op_V_NOT_B32,"V_NOT_B32"}, // 439
                {amdgpu_gfx1250_op_V_BFREV_B32,"V_BFREV_B32"}, // 440
                {amdgpu_gfx1250_op_V_CLZ_I32_U32,"V_CLZ_I32_U32"}, // 441
                {amdgpu_gfx1250_op_V_CTZ_I32_B32,"V_CTZ_I32_B32"}, // 442
                {amdgpu_gfx1250_op_V_CLS_I32,"V_CLS_I32"}, // 443
                {amdgpu_gfx1250_op_V_FREXP_EXP_I32_F64,"V_FREXP_EXP_I32_F64"}, // 444
                {amdgpu_gfx1250_op_V_FREXP_MANT_F64,"V_FREXP_MANT_F64"}, // 445
                {amdgpu_gfx1250_op_V_FRACT_F64,"V_FRACT_F64"}, // 446
                {amdgpu_gfx1250_op_V_FREXP_EXP_I32_F32,"V_FREXP_EXP_I32_F32"}, // 447
                {amdgpu_gfx1250_op_V_FREXP_MANT_F32,"V_FREXP_MANT_F32"}, // 448
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 449
                {amdgpu_gfx1250_op_V_MOVRELD_B32,"V_MOVRELD_B32"}, // 450
                {amdgpu_gfx1250_op_V_MOVRELS_B32,"V_MOVRELS_B32"}, // 451
                {amdgpu_gfx1250_op_V_MOVRELSD_B32,"V_MOVRELSD_B32"}, // 452
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 453
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 454
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 455
                {amdgpu_gfx1250_op_V_MOVRELSD_2_B32,"V_MOVRELSD_2_B32"}, // 456
                {amdgpu_gfx1250_op_V_PERMLANE16_SWAP_B32,"V_PERMLANE16_SWAP_B32"}, // 457
                {amdgpu_gfx1250_op_V_TANH_BF16,"V_TANH_BF16"}, // 458
                {amdgpu_gfx1250_op_V_PRNG_B32,"V_PRNG_B32"}, // 459
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 460
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 461
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 462
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 463
                {amdgpu_gfx1250_op_V_CVT_F16_U16,"V_CVT_F16_U16"}, // 464
                {amdgpu_gfx1250_op_V_CVT_F16_I16,"V_CVT_F16_I16"}, // 465
                {amdgpu_gfx1250_op_V_CVT_U16_F16,"V_CVT_U16_F16"}, // 466
                {amdgpu_gfx1250_op_V_CVT_I16_F16,"V_CVT_I16_F16"}, // 467
                {amdgpu_gfx1250_op_V_RCP_F16,"V_RCP_F16"}, // 468
                {amdgpu_gfx1250_op_V_SQRT_F16,"V_SQRT_F16"}, // 469
                {amdgpu_gfx1250_op_V_RSQ_F16,"V_RSQ_F16"}, // 470
                {amdgpu_gfx1250_op_V_LOG_F16,"V_LOG_F16"}, // 471
                {amdgpu_gfx1250_op_V_EXP_F16,"V_EXP_F16"}, // 472
                {amdgpu_gfx1250_op_V_FREXP_MANT_F16,"V_FREXP_MANT_F16"}, // 473
                {amdgpu_gfx1250_op_V_FREXP_EXP_I16_F16,"V_FREXP_EXP_I16_F16"}, // 474
                {amdgpu_gfx1250_op_V_FLOOR_F16,"V_FLOOR_F16"}, // 475
                {amdgpu_gfx1250_op_V_CEIL_F16,"V_CEIL_F16"}, // 476
                {amdgpu_gfx1250_op_V_TRUNC_F16,"V_TRUNC_F16"}, // 477
                {amdgpu_gfx1250_op_V_RNDNE_F16,"V_RNDNE_F16"}, // 478
                {amdgpu_gfx1250_op_V_FRACT_F16,"V_FRACT_F16"}, // 479
                {amdgpu_gfx1250_op_V_SIN_F16,"V_SIN_F16"}, // 480
                {amdgpu_gfx1250_op_V_COS_F16,"V_COS_F16"}, // 481
                {amdgpu_gfx1250_op_V_SAT_PK_U8_I16,"V_SAT_PK_U8_I16"}, // 482
                {amdgpu_gfx1250_op_V_CVT_NORM_I16_F16,"V_CVT_NORM_I16_F16"}, // 483
                {amdgpu_gfx1250_op_V_CVT_NORM_U16_F16,"V_CVT_NORM_U16_F16"}, // 484
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 485
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 486
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 487
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 488
                {amdgpu_gfx1250_op_V_NOT_B16,"V_NOT_B16"}, // 489
                {amdgpu_gfx1250_op_V_CVT_I32_I16,"V_CVT_I32_I16"}, // 490
                {amdgpu_gfx1250_op_V_CVT_U32_U16,"V_CVT_U32_U16"}, // 491
                {amdgpu_gfx1250_op_V_CVT_F32_FP8,"V_CVT_F32_FP8"}, // 492
                {amdgpu_gfx1250_op_V_CVT_F32_BF8,"V_CVT_F32_BF8"}, // 493
                {amdgpu_gfx1250_op_V_CVT_PK_F32_FP8,"V_CVT_PK_F32_FP8"}, // 494
                {amdgpu_gfx1250_op_V_CVT_PK_F32_BF8,"V_CVT_PK_F32_BF8"}, // 495
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 496
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 497
                {amdgpu_gfx1250_op_V_CVT_F32_BF16,"V_CVT_F32_BF16"}, // 498
                {amdgpu_gfx1250_op_V_SAT_PK4_I4_I8,"V_SAT_PK4_I4_I8"}, // 499
                {amdgpu_gfx1250_op_V_SAT_PK4_U4_U8,"V_SAT_PK4_U4_U8"}, // 500
                {amdgpu_gfx1250_op_V_CVT_PK_F16_FP8,"V_CVT_PK_F16_FP8"}, // 501
                {amdgpu_gfx1250_op_V_CVT_PK_F16_BF8,"V_CVT_PK_F16_BF8"}, // 502
                {amdgpu_gfx1250_op_V_CVT_F16_FP8,"V_CVT_F16_FP8"}, // 503
                {amdgpu_gfx1250_op_V_CVT_F16_BF8,"V_CVT_F16_BF8"}, // 504
                {amdgpu_gfx1250_op_V_RCP_BF16,"V_RCP_BF16"}, // 505
                {amdgpu_gfx1250_op_V_SQRT_BF16,"V_SQRT_BF16"}, // 506
                {amdgpu_gfx1250_op_V_RSQ_BF16,"V_RSQ_BF16"}, // 507
                {amdgpu_gfx1250_op_V_LOG_BF16,"V_LOG_BF16"}, // 508
                {amdgpu_gfx1250_op_V_EXP_BF16,"V_EXP_BF16"}, // 509
                {amdgpu_gfx1250_op_V_SIN_BF16,"V_SIN_BF16"}, // 510
                {amdgpu_gfx1250_op_V_COS_BF16,"V_COS_BF16"}, // 511
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 512
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 513
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 514
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 515
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 516
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 517
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 518
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 519
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 520
                {amdgpu_gfx1250_op_V_FMA_DX9_ZERO_F32,"V_FMA_DX9_ZERO_F32"}, // 521
                {amdgpu_gfx1250_op_V_MAD_I32_I24,"V_MAD_I32_I24"}, // 522
                {amdgpu_gfx1250_op_V_MAD_U32_U24,"V_MAD_U32_U24"}, // 523
                {amdgpu_gfx1250_op_V_CUBEID_F32,"V_CUBEID_F32"}, // 524
                {amdgpu_gfx1250_op_V_CUBESC_F32,"V_CUBESC_F32"}, // 525
                {amdgpu_gfx1250_op_V_CUBETC_F32,"V_CUBETC_F32"}, // 526
                {amdgpu_gfx1250_op_V_CUBEMA_F32,"V_CUBEMA_F32"}, // 527
                {amdgpu_gfx1250_op_V_BFE_U32,"V_BFE_U32"}, // 528
                {amdgpu_gfx1250_op_V_BFE_I32,"V_BFE_I32"}, // 529
                {amdgpu_gfx1250_op_V_BFI_B32,"V_BFI_B32"}, // 530
                {amdgpu_gfx1250_op_V_FMA_F32,"V_FMA_F32"}, // 531
                {amdgpu_gfx1250_op_V_FMA_F64,"V_FMA_F64"}, // 532
                {amdgpu_gfx1250_op_V_LERP_U8,"V_LERP_U8"}, // 533
                {amdgpu_gfx1250_op_V_ALIGNBIT_B32,"V_ALIGNBIT_B32"}, // 534
                {amdgpu_gfx1250_op_V_ALIGNBYTE_B32,"V_ALIGNBYTE_B32"}, // 535
                {amdgpu_gfx1250_op_V_MULLIT_F32,"V_MULLIT_F32"}, // 536
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 537
                {amdgpu_gfx1250_op_V_MIN3_I32,"V_MIN3_I32"}, // 538
                {amdgpu_gfx1250_op_V_MIN3_U32,"V_MIN3_U32"}, // 539
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 540
                {amdgpu_gfx1250_op_V_MAX3_I32,"V_MAX3_I32"}, // 541
                {amdgpu_gfx1250_op_V_MAX3_U32,"V_MAX3_U32"}, // 542
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 543
                {amdgpu_gfx1250_op_V_MED3_I32,"V_MED3_I32"}, // 544
                {amdgpu_gfx1250_op_V_MED3_U32,"V_MED3_U32"}, // 545
                {amdgpu_gfx1250_op_V_SAD_U8,"V_SAD_U8"}, // 546
                {amdgpu_gfx1250_op_V_SAD_HI_U8,"V_SAD_HI_U8"}, // 547
                {amdgpu_gfx1250_op_V_SAD_U16,"V_SAD_U16"}, // 548
                {amdgpu_gfx1250_op_V_SAD_U32,"V_SAD_U32"}, // 549
                {amdgpu_gfx1250_op_V_CVT_PK_U8_F32,"V_CVT_PK_U8_F32"}, // 550
                {amdgpu_gfx1250_op_V_DIV_FIXUP_F32,"V_DIV_FIXUP_F32"}, // 551
                {amdgpu_gfx1250_op_V_DIV_FIXUP_F64,"V_DIV_FIXUP_F64"}, // 552
                {amdgpu_gfx1250_op_V_MIN3_NUM_F32,"V_MIN3_NUM_F32"}, // 553
                {amdgpu_gfx1250_op_V_MAX3_NUM_F32,"V_MAX3_NUM_F32"}, // 554
                {amdgpu_gfx1250_op_V_MIN3_NUM_F16,"V_MIN3_NUM_F16"}, // 555
                {amdgpu_gfx1250_op_V_MAX3_NUM_F16,"V_MAX3_NUM_F16"}, // 556
                {amdgpu_gfx1250_op_V_MINIMUM3_F32,"V_MINIMUM3_F32"}, // 557
                {amdgpu_gfx1250_op_V_MAXIMUM3_F32,"V_MAXIMUM3_F32"}, // 558
                {amdgpu_gfx1250_op_V_MINIMUM3_F16,"V_MINIMUM3_F16"}, // 559
                {amdgpu_gfx1250_op_V_MAXIMUM3_F16,"V_MAXIMUM3_F16"}, // 560
                {amdgpu_gfx1250_op_V_MED3_NUM_F32,"V_MED3_NUM_F32"}, // 561
                {amdgpu_gfx1250_op_V_MED3_NUM_F16,"V_MED3_NUM_F16"}, // 562
                {amdgpu_gfx1250_op_V_BITOP3_B16,"V_BITOP3_B16"}, // 563
                {amdgpu_gfx1250_op_V_BITOP3_B32,"V_BITOP3_B32"}, // 564
                {amdgpu_gfx1250_op_V_MAD_U32,"V_MAD_U32"}, // 565
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 566
                {amdgpu_gfx1250_op_V_DIV_FMAS_F32,"V_DIV_FMAS_F32"}, // 567
                {amdgpu_gfx1250_op_V_DIV_FMAS_F64,"V_DIV_FMAS_F64"}, // 568
                {amdgpu_gfx1250_op_V_MSAD_U8,"V_MSAD_U8"}, // 569
                {amdgpu_gfx1250_op_V_QSAD_PK_U16_U8,"V_QSAD_PK_U16_U8"}, // 570
                {amdgpu_gfx1250_op_V_MQSAD_PK_U16_U8,"V_MQSAD_PK_U16_U8"}, // 571
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 572
                {amdgpu_gfx1250_op_V_MQSAD_U32_U8,"V_MQSAD_U32_U8"}, // 573
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 574
                {amdgpu_gfx1250_op_V_PERM_PK16_B4_U4,"V_PERM_PK16_B4_U4"}, // 575
                {amdgpu_gfx1250_op_V_XOR3_B32,"V_XOR3_B32"}, // 576
                {amdgpu_gfx1250_op_V_MAD_U16,"V_MAD_U16"}, // 577
                {amdgpu_gfx1250_op_V_PERM_PK16_B6_U4,"V_PERM_PK16_B6_U4"}, // 578
                {amdgpu_gfx1250_op_V_PERM_PK16_B8_U4,"V_PERM_PK16_B8_U4"}, // 579
                {amdgpu_gfx1250_op_V_PERM_B32,"V_PERM_B32"}, // 580
                {amdgpu_gfx1250_op_V_XAD_U32,"V_XAD_U32"}, // 581
                {amdgpu_gfx1250_op_V_LSHL_ADD_U32,"V_LSHL_ADD_U32"}, // 582
                {amdgpu_gfx1250_op_V_ADD_LSHL_U32,"V_ADD_LSHL_U32"}, // 583
                {amdgpu_gfx1250_op_V_FMA_F16,"V_FMA_F16"}, // 584
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 585
                {amdgpu_gfx1250_op_V_MIN3_I16,"V_MIN3_I16"}, // 586
                {amdgpu_gfx1250_op_V_MIN3_U16,"V_MIN3_U16"}, // 587
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 588
                {amdgpu_gfx1250_op_V_MAX3_I16,"V_MAX3_I16"}, // 589
                {amdgpu_gfx1250_op_V_MAX3_U16,"V_MAX3_U16"}, // 590
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 591
                {amdgpu_gfx1250_op_V_MED3_I16,"V_MED3_I16"}, // 592
                {amdgpu_gfx1250_op_V_MED3_U16,"V_MED3_U16"}, // 593
                {amdgpu_gfx1250_op_V_LSHL_ADD_U64,"V_LSHL_ADD_U64"}, // 594
                {amdgpu_gfx1250_op_V_MAD_I16,"V_MAD_I16"}, // 595
                {amdgpu_gfx1250_op_V_DIV_FIXUP_F16,"V_DIV_FIXUP_F16"}, // 596
                {amdgpu_gfx1250_op_V_ADD3_U32,"V_ADD3_U32"}, // 597
                {amdgpu_gfx1250_op_V_LSHL_OR_B32,"V_LSHL_OR_B32"}, // 598
                {amdgpu_gfx1250_op_V_AND_OR_B32,"V_AND_OR_B32"}, // 599
                {amdgpu_gfx1250_op_V_OR3_B32,"V_OR3_B32"}, // 600
                {amdgpu_gfx1250_op_V_MAD_U32_U16,"V_MAD_U32_U16"}, // 601
                {amdgpu_gfx1250_op_V_MAD_I32_I16,"V_MAD_I32_I16"}, // 602
                {amdgpu_gfx1250_op_V_PERMLANE16_B32,"V_PERMLANE16_B32"}, // 603
                {amdgpu_gfx1250_op_V_PERMLANEX16_B32,"V_PERMLANEX16_B32"}, // 604
                {amdgpu_gfx1250_op_V_CNDMASK_B16,"V_CNDMASK_B16"}, // 605
                {amdgpu_gfx1250_op_V_ADD_MAX_I32,"V_ADD_MAX_I32"}, // 606
                {amdgpu_gfx1250_op_V_ADD_MAX_U32,"V_ADD_MAX_U32"}, // 607
                {amdgpu_gfx1250_op_V_ADD_MIN_I32,"V_ADD_MIN_I32"}, // 608
                {amdgpu_gfx1250_op_V_ADD_MIN_U32,"V_ADD_MIN_U32"}, // 609
                {amdgpu_gfx1250_op_V_MAXMIN_U32,"V_MAXMIN_U32"}, // 610
                {amdgpu_gfx1250_op_V_MINMAX_U32,"V_MINMAX_U32"}, // 611
                {amdgpu_gfx1250_op_V_MAXMIN_I32,"V_MAXMIN_I32"}, // 612
                {amdgpu_gfx1250_op_V_MINMAX_I32,"V_MINMAX_I32"}, // 613
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 614
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 615
                {amdgpu_gfx1250_op_V_MINMAX_NUM_F32,"V_MINMAX_NUM_F32"}, // 616
                {amdgpu_gfx1250_op_V_MAXMIN_NUM_F32,"V_MAXMIN_NUM_F32"}, // 617
                {amdgpu_gfx1250_op_V_MINMAX_NUM_F16,"V_MINMAX_NUM_F16"}, // 618
                {amdgpu_gfx1250_op_V_MAXMIN_NUM_F16,"V_MAXMIN_NUM_F16"}, // 619
                {amdgpu_gfx1250_op_V_MINIMUMMAXIMUM_F32,"V_MINIMUMMAXIMUM_F32"}, // 620
                {amdgpu_gfx1250_op_V_MAXIMUMMINIMUM_F32,"V_MAXIMUMMINIMUM_F32"}, // 621
                {amdgpu_gfx1250_op_V_MINIMUMMAXIMUM_F16,"V_MINIMUMMAXIMUM_F16"}, // 622
                {amdgpu_gfx1250_op_V_MAXIMUMMINIMUM_F16,"V_MAXIMUMMINIMUM_F16"}, // 623
                {amdgpu_gfx1250_op_V_PERMLANE_BCAST_B32,"V_PERMLANE_BCAST_B32"}, // 624
                {amdgpu_gfx1250_op_V_PERMLANE_UP_B32,"V_PERMLANE_UP_B32"}, // 625
                {amdgpu_gfx1250_op_V_PERMLANE_DOWN_B32,"V_PERMLANE_DOWN_B32"}, // 626
                {amdgpu_gfx1250_op_V_PERMLANE_XOR_B32,"V_PERMLANE_XOR_B32"}, // 627
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 628
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 629
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 630
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 631
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 632
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 633
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 634
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 635
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 636
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 637
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 638
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 639
                {amdgpu_gfx1250_op_V_S_EXP_F32,"V_S_EXP_F32"}, // 640
                {amdgpu_gfx1250_op_V_S_EXP_F16,"V_S_EXP_F16"}, // 641
                {amdgpu_gfx1250_op_V_S_LOG_F32,"V_S_LOG_F32"}, // 642
                {amdgpu_gfx1250_op_V_S_LOG_F16,"V_S_LOG_F16"}, // 643
                {amdgpu_gfx1250_op_V_S_RCP_F32,"V_S_RCP_F32"}, // 644
                {amdgpu_gfx1250_op_V_S_RCP_F16,"V_S_RCP_F16"}, // 645
                {amdgpu_gfx1250_op_V_S_RSQ_F32,"V_S_RSQ_F32"}, // 646
                {amdgpu_gfx1250_op_V_S_RSQ_F16,"V_S_RSQ_F16"}, // 647
                {amdgpu_gfx1250_op_V_S_SQRT_F32,"V_S_SQRT_F32"}, // 648
                {amdgpu_gfx1250_op_V_S_SQRT_F16,"V_S_SQRT_F16"}, // 649
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 650
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 651
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 652
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 653
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 654
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 655
                {amdgpu_gfx1250_op_V_ASHR_PK_I8_I32,"V_ASHR_PK_I8_I32"}, // 656
                {amdgpu_gfx1250_op_V_ASHR_PK_U8_I32,"V_ASHR_PK_U8_I32"}, // 657
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 658
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 659
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 660
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 661
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 662
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK8_FP4_F32,"V_CVT_SCALEF32_SR_PK8_FP4_F32"}, // 663
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK8_FP8_F32,"V_CVT_SCALEF32_SR_PK8_FP8_F32"}, // 664
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK8_BF8_F32,"V_CVT_SCALEF32_SR_PK8_BF8_F32"}, // 665
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 666
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 667
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 668
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 669
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 670
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK8_F16_FP4,"V_CVT_SCALE_PK8_F16_FP4"}, // 671
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK8_BF16_FP4,"V_CVT_SCALE_PK8_BF16_FP4"}, // 672
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK8_F32_FP4,"V_CVT_SCALE_PK8_F32_FP4"}, // 673
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 674
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 675
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 676
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 677
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 678
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 679
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK8_F16_FP8,"V_CVT_SCALE_PK8_F16_FP8"}, // 680
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK8_BF16_FP8,"V_CVT_SCALE_PK8_BF16_FP8"}, // 681
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK8_F32_FP8,"V_CVT_SCALE_PK8_F32_FP8"}, // 682
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK8_F16_BF8,"V_CVT_SCALE_PK8_F16_BF8"}, // 683
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK8_BF16_BF8,"V_CVT_SCALE_PK8_BF16_BF8"}, // 684
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK8_F32_BF8,"V_CVT_SCALE_PK8_F32_BF8"}, // 685
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 686
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 687
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK8_FP4_F32,"V_CVT_SCALEF32_PK8_FP4_F32"}, // 688
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 689
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 690
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK8_FP4_F16,"V_CVT_SCALEF32_PK8_FP4_F16"}, // 691
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK8_FP8_BF16,"V_CVT_SCALEF32_PK8_FP8_BF16"}, // 692
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK8_BF8_BF16,"V_CVT_SCALEF32_PK8_BF8_BF16"}, // 693
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 694
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 695
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK8_FP4_BF16,"V_CVT_SCALEF32_PK8_FP4_BF16"}, // 696
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK8_FP4_F16,"V_CVT_SCALEF32_SR_PK8_FP4_F16"}, // 697
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 698
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 699
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK8_FP4_BF16,"V_CVT_SCALEF32_SR_PK8_FP4_BF16"}, // 700
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 701
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 702
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK8_FP8_F16,"V_CVT_SCALEF32_SR_PK8_FP8_F16"}, // 703
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK8_FP8_BF16,"V_CVT_SCALEF32_SR_PK8_FP8_BF16"}, // 704
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK8_BF8_F16,"V_CVT_SCALEF32_SR_PK8_BF8_F16"}, // 705
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK8_BF8_BF16,"V_CVT_SCALEF32_SR_PK8_BF8_BF16"}, // 706
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK8_FP8_F32,"V_CVT_SCALEF32_PK8_FP8_F32"}, // 707
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK8_FP8_F16,"V_CVT_SCALEF32_PK8_FP8_F16"}, // 708
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK8_BF8_F32,"V_CVT_SCALEF32_PK8_BF8_F32"}, // 709
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK8_BF8_F16,"V_CVT_SCALEF32_PK8_BF8_F16"}, // 710
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK16_F16_FP6,"V_CVT_SCALE_PK16_F16_FP6"}, // 711
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK16_BF16_FP6,"V_CVT_SCALE_PK16_BF16_FP6"}, // 712
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK16_F32_FP6,"V_CVT_SCALE_PK16_F32_FP6"}, // 713
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK16_F16_BF6,"V_CVT_SCALE_PK16_F16_BF6"}, // 714
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK16_BF16_BF6,"V_CVT_SCALE_PK16_BF16_BF6"}, // 715
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK16_F32_BF6,"V_CVT_SCALE_PK16_F32_BF6"}, // 716
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK16_FP6_F32,"V_CVT_SCALEF32_PK16_FP6_F32"}, // 717
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK16_BF6_F32,"V_CVT_SCALEF32_PK16_BF6_F32"}, // 718
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK16_FP6_F16,"V_CVT_SCALEF32_PK16_FP6_F16"}, // 719
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK16_BF6_F16,"V_CVT_SCALEF32_PK16_BF6_F16"}, // 720
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK16_FP6_BF16,"V_CVT_SCALEF32_PK16_FP6_BF16"}, // 721
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK16_BF6_BF16,"V_CVT_SCALEF32_PK16_BF6_BF16"}, // 722
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK16_FP6_F32,"V_CVT_SCALEF32_SR_PK16_FP6_F32"}, // 723
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK16_BF6_F32,"V_CVT_SCALEF32_SR_PK16_BF6_F32"}, // 724
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK16_FP6_F16,"V_CVT_SCALEF32_SR_PK16_FP6_F16"}, // 725
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK16_BF6_F16,"V_CVT_SCALEF32_SR_PK16_BF6_F16"}, // 726
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK16_FP6_BF16,"V_CVT_SCALEF32_SR_PK16_FP6_BF16"}, // 727
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK16_BF6_BF16,"V_CVT_SCALEF32_SR_PK16_BF6_BF16"}, // 728
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 729
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 730
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 731
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 732
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 733
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 734
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 735
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 736
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 737
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 738
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 739
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 740
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 741
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 742
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 743
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 744
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 745
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 746
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 747
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 748
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 749
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 750
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 751
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 752
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 753
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 754
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 755
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 756
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 757
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 758
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 759
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 760
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 761
                {amdgpu_gfx1250_op_V_MAD_NC_U64_U32,"V_MAD_NC_U64_U32"}, // 762
                {amdgpu_gfx1250_op_V_MAD_NC_I64_I32,"V_MAD_NC_I64_I32"}, // 763
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 764
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 765
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 766
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 767
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 768
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 769
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 770
                {amdgpu_gfx1250_op_V_ADD_NC_U16,"V_ADD_NC_U16"}, // 771
                {amdgpu_gfx1250_op_V_SUB_NC_U16,"V_SUB_NC_U16"}, // 772
                {amdgpu_gfx1250_op_V_MUL_LO_U16,"V_MUL_LO_U16"}, // 773
                {amdgpu_gfx1250_op_V_CVT_PK_I16_F32,"V_CVT_PK_I16_F32"}, // 774
                {amdgpu_gfx1250_op_V_CVT_PK_U16_F32,"V_CVT_PK_U16_F32"}, // 775
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 776
                {amdgpu_gfx1250_op_V_MAX_U16,"V_MAX_U16"}, // 777
                {amdgpu_gfx1250_op_V_MAX_I16,"V_MAX_I16"}, // 778
                {amdgpu_gfx1250_op_V_MIN_U16,"V_MIN_U16"}, // 779
                {amdgpu_gfx1250_op_V_MIN_I16,"V_MIN_I16"}, // 780
                {amdgpu_gfx1250_op_V_ADD_NC_I16,"V_ADD_NC_I16"}, // 781
                {amdgpu_gfx1250_op_V_SUB_NC_I16,"V_SUB_NC_I16"}, // 782
                {amdgpu_gfx1250_op_V_PERMLANE16_VAR_B32,"V_PERMLANE16_VAR_B32"}, // 783
                {amdgpu_gfx1250_op_V_PERMLANEX16_VAR_B32,"V_PERMLANEX16_VAR_B32"}, // 784
                {amdgpu_gfx1250_op_V_PACK_B32_F16,"V_PACK_B32_F16"}, // 785
                {amdgpu_gfx1250_op_V_CVT_PK_NORM_I16_F16,"V_CVT_PK_NORM_I16_F16"}, // 786
                {amdgpu_gfx1250_op_V_CVT_PK_NORM_U16_F16,"V_CVT_PK_NORM_U16_F16"}, // 787
                {amdgpu_gfx1250_op_V_PERMLANE_IDX_GEN_B32,"V_PERMLANE_IDX_GEN_B32"}, // 788
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 789
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 790
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 791
                {amdgpu_gfx1250_op_V_MIN_U64,"V_MIN_U64"}, // 792
                {amdgpu_gfx1250_op_V_MAX_U64,"V_MAX_U64"}, // 793
                {amdgpu_gfx1250_op_V_MIN_I64,"V_MIN_I64"}, // 794
                {amdgpu_gfx1250_op_V_MAX_I64,"V_MAX_I64"}, // 795
                {amdgpu_gfx1250_op_V_LDEXP_F32,"V_LDEXP_F32"}, // 796
                {amdgpu_gfx1250_op_V_BFM_B32,"V_BFM_B32"}, // 797
                {amdgpu_gfx1250_op_V_BCNT_U32_B32,"V_BCNT_U32_B32"}, // 798
                {amdgpu_gfx1250_op_V_MBCNT_LO_U32_B32,"V_MBCNT_LO_U32_B32"}, // 799
                {amdgpu_gfx1250_op_V_MBCNT_HI_U32_B32,"V_MBCNT_HI_U32_B32"}, // 800
                {amdgpu_gfx1250_op_V_CVT_PK_NORM_I16_F32,"V_CVT_PK_NORM_I16_F32"}, // 801
                {amdgpu_gfx1250_op_V_CVT_PK_NORM_U16_F32,"V_CVT_PK_NORM_U16_F32"}, // 802
                {amdgpu_gfx1250_op_V_CVT_PK_U16_U32,"V_CVT_PK_U16_U32"}, // 803
                {amdgpu_gfx1250_op_V_CVT_PK_I16_I32,"V_CVT_PK_I16_I32"}, // 804
                {amdgpu_gfx1250_op_V_SUB_NC_I32,"V_SUB_NC_I32"}, // 805
                {amdgpu_gfx1250_op_V_ADD_NC_I32,"V_ADD_NC_I32"}, // 806
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 807
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 808
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 809
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 810
                {amdgpu_gfx1250_op_V_LDEXP_F64,"V_LDEXP_F64"}, // 811
                {amdgpu_gfx1250_op_V_MUL_LO_U32,"V_MUL_LO_U32"}, // 812
                {amdgpu_gfx1250_op_V_MUL_HI_U32,"V_MUL_HI_U32"}, // 813
                {amdgpu_gfx1250_op_V_MUL_HI_I32,"V_MUL_HI_I32"}, // 814
                {amdgpu_gfx1250_op_V_TRIG_PREOP_F64,"V_TRIG_PREOP_F64"}, // 815
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 816
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 817
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 818
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 819
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 820
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 821
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 822
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 823
                {amdgpu_gfx1250_op_V_LSHLREV_B16,"V_LSHLREV_B16"}, // 824
                {amdgpu_gfx1250_op_V_LSHRREV_B16,"V_LSHRREV_B16"}, // 825
                {amdgpu_gfx1250_op_V_ASHRREV_I16,"V_ASHRREV_I16"}, // 826
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 827
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 828
                {amdgpu_gfx1250_op_V_LSHRREV_B64,"V_LSHRREV_B64"}, // 829
                {amdgpu_gfx1250_op_V_ASHRREV_I64,"V_ASHRREV_I64"}, // 830
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 831
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 832
                {amdgpu_gfx1250_op_V_MINIMUM_F64,"V_MINIMUM_F64"}, // 833
                {amdgpu_gfx1250_op_V_MAXIMUM_F64,"V_MAXIMUM_F64"}, // 834
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 835
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 836
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 837
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 838
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 839
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 840
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 841
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 842
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 843
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 844
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 845
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 846
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 847
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 848
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 849
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 850
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 851
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 852
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 853
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 854
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 855
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 856
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 857
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 858
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 859
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 860
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 861
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 862
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 863
                {amdgpu_gfx1250_op_V_READLANE_B32,"V_READLANE_B32"}, // 864
                {amdgpu_gfx1250_op_V_WRITELANE_B32,"V_WRITELANE_B32"}, // 865
                {amdgpu_gfx1250_op_V_AND_B16,"V_AND_B16"}, // 866
                {amdgpu_gfx1250_op_V_OR_B16,"V_OR_B16"}, // 867
                {amdgpu_gfx1250_op_V_XOR_B16,"V_XOR_B16"}, // 868
                {amdgpu_gfx1250_op_V_MINIMUM_F32,"V_MINIMUM_F32"}, // 869
                {amdgpu_gfx1250_op_V_MAXIMUM_F32,"V_MAXIMUM_F32"}, // 870
                {amdgpu_gfx1250_op_V_MINIMUM_F16,"V_MINIMUM_F16"}, // 871
                {amdgpu_gfx1250_op_V_MAXIMUM_F16,"V_MAXIMUM_F16"}, // 872
                {amdgpu_gfx1250_op_V_CVT_PK_FP8_F32,"V_CVT_PK_FP8_F32"}, // 873
                {amdgpu_gfx1250_op_V_CVT_PK_BF8_F32,"V_CVT_PK_BF8_F32"}, // 874
                {amdgpu_gfx1250_op_V_CVT_SR_FP8_F32,"V_CVT_SR_FP8_F32"}, // 875
                {amdgpu_gfx1250_op_V_CVT_SR_BF8_F32,"V_CVT_SR_BF8_F32"}, // 876
                {amdgpu_gfx1250_op_V_CVT_PK_BF16_F32,"V_CVT_PK_BF16_F32"}, // 877
                {amdgpu_gfx1250_op_V_CVT_SR_PK_BF16_F32,"V_CVT_SR_PK_BF16_F32"}, // 878
                {amdgpu_gfx1250_op_V_CVT_PK_F16_F32,"V_CVT_PK_F16_F32"}, // 879
                {amdgpu_gfx1250_op_V_CVT_SR_PK_F16_F32,"V_CVT_SR_PK_F16_F32"}, // 880
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 881
                {amdgpu_gfx1250_op_V_CVT_PK_FP8_F16,"V_CVT_PK_FP8_F16"}, // 882
                {amdgpu_gfx1250_op_V_CVT_PK_BF8_F16,"V_CVT_PK_BF8_F16"}, // 883
                {amdgpu_gfx1250_op_V_CVT_SR_FP8_F16,"V_CVT_SR_FP8_F16"}, // 884
                {amdgpu_gfx1250_op_V_CVT_SR_BF8_F16,"V_CVT_SR_BF8_F16"}, // 885
            }; // end ENC_VOP3_insn_table
            const amdgpu_gfx1250_insn_entry VOP1_INST_LITERAL_insn_table[128] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_V_MOV_B32,"V_MOV_B32"}, // 1
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 2
                {amdgpu_gfx1250_op_V_CVT_I32_F64,"V_CVT_I32_F64"}, // 3
                {amdgpu_gfx1250_op_V_CVT_F64_I32,"V_CVT_F64_I32"}, // 4
                {amdgpu_gfx1250_op_V_CVT_F32_I32,"V_CVT_F32_I32"}, // 5
                {amdgpu_gfx1250_op_V_CVT_F32_U32,"V_CVT_F32_U32"}, // 6
                {amdgpu_gfx1250_op_V_CVT_U32_F32,"V_CVT_U32_F32"}, // 7
                {amdgpu_gfx1250_op_V_CVT_I32_F32,"V_CVT_I32_F32"}, // 8
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 9
                {amdgpu_gfx1250_op_V_CVT_F16_F32,"V_CVT_F16_F32"}, // 10
                {amdgpu_gfx1250_op_V_CVT_F32_F16,"V_CVT_F32_F16"}, // 11
                {amdgpu_gfx1250_op_V_CVT_NEAREST_I32_F32,"V_CVT_NEAREST_I32_F32"}, // 12
                {amdgpu_gfx1250_op_V_CVT_FLOOR_I32_F32,"V_CVT_FLOOR_I32_F32"}, // 13
                {amdgpu_gfx1250_op_V_CVT_OFF_F32_I4,"V_CVT_OFF_F32_I4"}, // 14
                {amdgpu_gfx1250_op_V_CVT_F32_F64,"V_CVT_F32_F64"}, // 15
                {amdgpu_gfx1250_op_V_CVT_F64_F32,"V_CVT_F64_F32"}, // 16
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE0,"V_CVT_F32_UBYTE0"}, // 17
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE1,"V_CVT_F32_UBYTE1"}, // 18
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE2,"V_CVT_F32_UBYTE2"}, // 19
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE3,"V_CVT_F32_UBYTE3"}, // 20
                {amdgpu_gfx1250_op_V_CVT_U32_F64,"V_CVT_U32_F64"}, // 21
                {amdgpu_gfx1250_op_V_CVT_F64_U32,"V_CVT_F64_U32"}, // 22
                {amdgpu_gfx1250_op_V_TRUNC_F64,"V_TRUNC_F64"}, // 23
                {amdgpu_gfx1250_op_V_CEIL_F64,"V_CEIL_F64"}, // 24
                {amdgpu_gfx1250_op_V_RNDNE_F64,"V_RNDNE_F64"}, // 25
                {amdgpu_gfx1250_op_V_FLOOR_F64,"V_FLOOR_F64"}, // 26
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 27
                {amdgpu_gfx1250_op_V_MOV_B16,"V_MOV_B16"}, // 28
                {amdgpu_gfx1250_op_V_MOV_B64,"V_MOV_B64"}, // 29
                {amdgpu_gfx1250_op_V_TANH_F32,"V_TANH_F32"}, // 30
                {amdgpu_gfx1250_op_V_TANH_F16,"V_TANH_F16"}, // 31
                {amdgpu_gfx1250_op_V_FRACT_F32,"V_FRACT_F32"}, // 32
                {amdgpu_gfx1250_op_V_TRUNC_F32,"V_TRUNC_F32"}, // 33
                {amdgpu_gfx1250_op_V_CEIL_F32,"V_CEIL_F32"}, // 34
                {amdgpu_gfx1250_op_V_RNDNE_F32,"V_RNDNE_F32"}, // 35
                {amdgpu_gfx1250_op_V_FLOOR_F32,"V_FLOOR_F32"}, // 36
                {amdgpu_gfx1250_op_V_EXP_F32,"V_EXP_F32"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_V_LOG_F32,"V_LOG_F32"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_V_RCP_F32,"V_RCP_F32"}, // 42
                {amdgpu_gfx1250_op_V_RCP_IFLAG_F32,"V_RCP_IFLAG_F32"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_V_RSQ_F32,"V_RSQ_F32"}, // 46
                {amdgpu_gfx1250_op_V_RCP_F64,"V_RCP_F64"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_V_RSQ_F64,"V_RSQ_F64"}, // 49
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 50
                {amdgpu_gfx1250_op_V_SQRT_F32,"V_SQRT_F32"}, // 51
                {amdgpu_gfx1250_op_V_SQRT_F64,"V_SQRT_F64"}, // 52
                {amdgpu_gfx1250_op_V_SIN_F32,"V_SIN_F32"}, // 53
                {amdgpu_gfx1250_op_V_COS_F32,"V_COS_F32"}, // 54
                {amdgpu_gfx1250_op_V_NOT_B32,"V_NOT_B32"}, // 55
                {amdgpu_gfx1250_op_V_BFREV_B32,"V_BFREV_B32"}, // 56
                {amdgpu_gfx1250_op_V_CLZ_I32_U32,"V_CLZ_I32_U32"}, // 57
                {amdgpu_gfx1250_op_V_CTZ_I32_B32,"V_CTZ_I32_B32"}, // 58
                {amdgpu_gfx1250_op_V_CLS_I32,"V_CLS_I32"}, // 59
                {amdgpu_gfx1250_op_V_FREXP_EXP_I32_F64,"V_FREXP_EXP_I32_F64"}, // 60
                {amdgpu_gfx1250_op_V_FREXP_MANT_F64,"V_FREXP_MANT_F64"}, // 61
                {amdgpu_gfx1250_op_V_FRACT_F64,"V_FRACT_F64"}, // 62
                {amdgpu_gfx1250_op_V_FREXP_EXP_I32_F32,"V_FREXP_EXP_I32_F32"}, // 63
                {amdgpu_gfx1250_op_V_FREXP_MANT_F32,"V_FREXP_MANT_F32"}, // 64
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 65
                {amdgpu_gfx1250_op_V_MOVRELD_B32,"V_MOVRELD_B32"}, // 66
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 67
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 68
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 69
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 72
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 73
                {amdgpu_gfx1250_op_V_TANH_BF16,"V_TANH_BF16"}, // 74
                {amdgpu_gfx1250_op_V_PRNG_B32,"V_PRNG_B32"}, // 75
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 76
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 77
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_V_CVT_F16_U16,"V_CVT_F16_U16"}, // 80
                {amdgpu_gfx1250_op_V_CVT_F16_I16,"V_CVT_F16_I16"}, // 81
                {amdgpu_gfx1250_op_V_CVT_U16_F16,"V_CVT_U16_F16"}, // 82
                {amdgpu_gfx1250_op_V_CVT_I16_F16,"V_CVT_I16_F16"}, // 83
                {amdgpu_gfx1250_op_V_RCP_F16,"V_RCP_F16"}, // 84
                {amdgpu_gfx1250_op_V_SQRT_F16,"V_SQRT_F16"}, // 85
                {amdgpu_gfx1250_op_V_RSQ_F16,"V_RSQ_F16"}, // 86
                {amdgpu_gfx1250_op_V_LOG_F16,"V_LOG_F16"}, // 87
                {amdgpu_gfx1250_op_V_EXP_F16,"V_EXP_F16"}, // 88
                {amdgpu_gfx1250_op_V_FREXP_MANT_F16,"V_FREXP_MANT_F16"}, // 89
                {amdgpu_gfx1250_op_V_FREXP_EXP_I16_F16,"V_FREXP_EXP_I16_F16"}, // 90
                {amdgpu_gfx1250_op_V_FLOOR_F16,"V_FLOOR_F16"}, // 91
                {amdgpu_gfx1250_op_V_CEIL_F16,"V_CEIL_F16"}, // 92
                {amdgpu_gfx1250_op_V_TRUNC_F16,"V_TRUNC_F16"}, // 93
                {amdgpu_gfx1250_op_V_RNDNE_F16,"V_RNDNE_F16"}, // 94
                {amdgpu_gfx1250_op_V_FRACT_F16,"V_FRACT_F16"}, // 95
                {amdgpu_gfx1250_op_V_SIN_F16,"V_SIN_F16"}, // 96
                {amdgpu_gfx1250_op_V_COS_F16,"V_COS_F16"}, // 97
                {amdgpu_gfx1250_op_V_SAT_PK_U8_I16,"V_SAT_PK_U8_I16"}, // 98
                {amdgpu_gfx1250_op_V_CVT_NORM_I16_F16,"V_CVT_NORM_I16_F16"}, // 99
                {amdgpu_gfx1250_op_V_CVT_NORM_U16_F16,"V_CVT_NORM_U16_F16"}, // 100
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 101
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 102
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 103
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 104
                {amdgpu_gfx1250_op_V_NOT_B16,"V_NOT_B16"}, // 105
                {amdgpu_gfx1250_op_V_CVT_I32_I16,"V_CVT_I32_I16"}, // 106
                {amdgpu_gfx1250_op_V_CVT_U32_U16,"V_CVT_U32_U16"}, // 107
                {amdgpu_gfx1250_op_V_CVT_F32_FP8,"V_CVT_F32_FP8"}, // 108
                {amdgpu_gfx1250_op_V_CVT_F32_BF8,"V_CVT_F32_BF8"}, // 109
                {amdgpu_gfx1250_op_V_CVT_PK_F32_FP8,"V_CVT_PK_F32_FP8"}, // 110
                {amdgpu_gfx1250_op_V_CVT_PK_F32_BF8,"V_CVT_PK_F32_BF8"}, // 111
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 112
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 113
                {amdgpu_gfx1250_op_V_CVT_F32_BF16,"V_CVT_F32_BF16"}, // 114
                {amdgpu_gfx1250_op_V_SAT_PK4_I4_I8,"V_SAT_PK4_I4_I8"}, // 115
                {amdgpu_gfx1250_op_V_SAT_PK4_U4_U8,"V_SAT_PK4_U4_U8"}, // 116
                {amdgpu_gfx1250_op_V_CVT_PK_F16_FP8,"V_CVT_PK_F16_FP8"}, // 117
                {amdgpu_gfx1250_op_V_CVT_PK_F16_BF8,"V_CVT_PK_F16_BF8"}, // 118
                {amdgpu_gfx1250_op_V_CVT_F16_FP8,"V_CVT_F16_FP8"}, // 119
                {amdgpu_gfx1250_op_V_CVT_F16_BF8,"V_CVT_F16_BF8"}, // 120
                {amdgpu_gfx1250_op_V_RCP_BF16,"V_RCP_BF16"}, // 121
                {amdgpu_gfx1250_op_V_SQRT_BF16,"V_SQRT_BF16"}, // 122
                {amdgpu_gfx1250_op_V_RSQ_BF16,"V_RSQ_BF16"}, // 123
                {amdgpu_gfx1250_op_V_LOG_BF16,"V_LOG_BF16"}, // 124
                {amdgpu_gfx1250_op_V_EXP_BF16,"V_EXP_BF16"}, // 125
                {amdgpu_gfx1250_op_V_SIN_BF16,"V_SIN_BF16"}, // 126
                {amdgpu_gfx1250_op_V_COS_BF16,"V_COS_BF16"}, // 127
            }; // end VOP1_INST_LITERAL_insn_table
            const amdgpu_gfx1250_insn_entry VOP1_INST_LITERAL64_insn_table[128] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_V_MOV_B32,"V_MOV_B32"}, // 1
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 2
                {amdgpu_gfx1250_op_V_CVT_I32_F64,"V_CVT_I32_F64"}, // 3
                {amdgpu_gfx1250_op_V_CVT_F64_I32,"V_CVT_F64_I32"}, // 4
                {amdgpu_gfx1250_op_V_CVT_F32_I32,"V_CVT_F32_I32"}, // 5
                {amdgpu_gfx1250_op_V_CVT_F32_U32,"V_CVT_F32_U32"}, // 6
                {amdgpu_gfx1250_op_V_CVT_U32_F32,"V_CVT_U32_F32"}, // 7
                {amdgpu_gfx1250_op_V_CVT_I32_F32,"V_CVT_I32_F32"}, // 8
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 9
                {amdgpu_gfx1250_op_V_CVT_F16_F32,"V_CVT_F16_F32"}, // 10
                {amdgpu_gfx1250_op_V_CVT_F32_F16,"V_CVT_F32_F16"}, // 11
                {amdgpu_gfx1250_op_V_CVT_NEAREST_I32_F32,"V_CVT_NEAREST_I32_F32"}, // 12
                {amdgpu_gfx1250_op_V_CVT_FLOOR_I32_F32,"V_CVT_FLOOR_I32_F32"}, // 13
                {amdgpu_gfx1250_op_V_CVT_OFF_F32_I4,"V_CVT_OFF_F32_I4"}, // 14
                {amdgpu_gfx1250_op_V_CVT_F32_F64,"V_CVT_F32_F64"}, // 15
                {amdgpu_gfx1250_op_V_CVT_F64_F32,"V_CVT_F64_F32"}, // 16
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE0,"V_CVT_F32_UBYTE0"}, // 17
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE1,"V_CVT_F32_UBYTE1"}, // 18
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE2,"V_CVT_F32_UBYTE2"}, // 19
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE3,"V_CVT_F32_UBYTE3"}, // 20
                {amdgpu_gfx1250_op_V_CVT_U32_F64,"V_CVT_U32_F64"}, // 21
                {amdgpu_gfx1250_op_V_CVT_F64_U32,"V_CVT_F64_U32"}, // 22
                {amdgpu_gfx1250_op_V_TRUNC_F64,"V_TRUNC_F64"}, // 23
                {amdgpu_gfx1250_op_V_CEIL_F64,"V_CEIL_F64"}, // 24
                {amdgpu_gfx1250_op_V_RNDNE_F64,"V_RNDNE_F64"}, // 25
                {amdgpu_gfx1250_op_V_FLOOR_F64,"V_FLOOR_F64"}, // 26
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 27
                {amdgpu_gfx1250_op_V_MOV_B16,"V_MOV_B16"}, // 28
                {amdgpu_gfx1250_op_V_MOV_B64,"V_MOV_B64"}, // 29
                {amdgpu_gfx1250_op_V_TANH_F32,"V_TANH_F32"}, // 30
                {amdgpu_gfx1250_op_V_TANH_F16,"V_TANH_F16"}, // 31
                {amdgpu_gfx1250_op_V_FRACT_F32,"V_FRACT_F32"}, // 32
                {amdgpu_gfx1250_op_V_TRUNC_F32,"V_TRUNC_F32"}, // 33
                {amdgpu_gfx1250_op_V_CEIL_F32,"V_CEIL_F32"}, // 34
                {amdgpu_gfx1250_op_V_RNDNE_F32,"V_RNDNE_F32"}, // 35
                {amdgpu_gfx1250_op_V_FLOOR_F32,"V_FLOOR_F32"}, // 36
                {amdgpu_gfx1250_op_V_EXP_F32,"V_EXP_F32"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_V_LOG_F32,"V_LOG_F32"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_V_RCP_F32,"V_RCP_F32"}, // 42
                {amdgpu_gfx1250_op_V_RCP_IFLAG_F32,"V_RCP_IFLAG_F32"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_V_RSQ_F32,"V_RSQ_F32"}, // 46
                {amdgpu_gfx1250_op_V_RCP_F64,"V_RCP_F64"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_V_RSQ_F64,"V_RSQ_F64"}, // 49
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 50
                {amdgpu_gfx1250_op_V_SQRT_F32,"V_SQRT_F32"}, // 51
                {amdgpu_gfx1250_op_V_SQRT_F64,"V_SQRT_F64"}, // 52
                {amdgpu_gfx1250_op_V_SIN_F32,"V_SIN_F32"}, // 53
                {amdgpu_gfx1250_op_V_COS_F32,"V_COS_F32"}, // 54
                {amdgpu_gfx1250_op_V_NOT_B32,"V_NOT_B32"}, // 55
                {amdgpu_gfx1250_op_V_BFREV_B32,"V_BFREV_B32"}, // 56
                {amdgpu_gfx1250_op_V_CLZ_I32_U32,"V_CLZ_I32_U32"}, // 57
                {amdgpu_gfx1250_op_V_CTZ_I32_B32,"V_CTZ_I32_B32"}, // 58
                {amdgpu_gfx1250_op_V_CLS_I32,"V_CLS_I32"}, // 59
                {amdgpu_gfx1250_op_V_FREXP_EXP_I32_F64,"V_FREXP_EXP_I32_F64"}, // 60
                {amdgpu_gfx1250_op_V_FREXP_MANT_F64,"V_FREXP_MANT_F64"}, // 61
                {amdgpu_gfx1250_op_V_FRACT_F64,"V_FRACT_F64"}, // 62
                {amdgpu_gfx1250_op_V_FREXP_EXP_I32_F32,"V_FREXP_EXP_I32_F32"}, // 63
                {amdgpu_gfx1250_op_V_FREXP_MANT_F32,"V_FREXP_MANT_F32"}, // 64
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 65
                {amdgpu_gfx1250_op_V_MOVRELD_B32,"V_MOVRELD_B32"}, // 66
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 67
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 68
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 69
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 72
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 73
                {amdgpu_gfx1250_op_V_TANH_BF16,"V_TANH_BF16"}, // 74
                {amdgpu_gfx1250_op_V_PRNG_B32,"V_PRNG_B32"}, // 75
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 76
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 77
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_V_CVT_F16_U16,"V_CVT_F16_U16"}, // 80
                {amdgpu_gfx1250_op_V_CVT_F16_I16,"V_CVT_F16_I16"}, // 81
                {amdgpu_gfx1250_op_V_CVT_U16_F16,"V_CVT_U16_F16"}, // 82
                {amdgpu_gfx1250_op_V_CVT_I16_F16,"V_CVT_I16_F16"}, // 83
                {amdgpu_gfx1250_op_V_RCP_F16,"V_RCP_F16"}, // 84
                {amdgpu_gfx1250_op_V_SQRT_F16,"V_SQRT_F16"}, // 85
                {amdgpu_gfx1250_op_V_RSQ_F16,"V_RSQ_F16"}, // 86
                {amdgpu_gfx1250_op_V_LOG_F16,"V_LOG_F16"}, // 87
                {amdgpu_gfx1250_op_V_EXP_F16,"V_EXP_F16"}, // 88
                {amdgpu_gfx1250_op_V_FREXP_MANT_F16,"V_FREXP_MANT_F16"}, // 89
                {amdgpu_gfx1250_op_V_FREXP_EXP_I16_F16,"V_FREXP_EXP_I16_F16"}, // 90
                {amdgpu_gfx1250_op_V_FLOOR_F16,"V_FLOOR_F16"}, // 91
                {amdgpu_gfx1250_op_V_CEIL_F16,"V_CEIL_F16"}, // 92
                {amdgpu_gfx1250_op_V_TRUNC_F16,"V_TRUNC_F16"}, // 93
                {amdgpu_gfx1250_op_V_RNDNE_F16,"V_RNDNE_F16"}, // 94
                {amdgpu_gfx1250_op_V_FRACT_F16,"V_FRACT_F16"}, // 95
                {amdgpu_gfx1250_op_V_SIN_F16,"V_SIN_F16"}, // 96
                {amdgpu_gfx1250_op_V_COS_F16,"V_COS_F16"}, // 97
                {amdgpu_gfx1250_op_V_SAT_PK_U8_I16,"V_SAT_PK_U8_I16"}, // 98
                {amdgpu_gfx1250_op_V_CVT_NORM_I16_F16,"V_CVT_NORM_I16_F16"}, // 99
                {amdgpu_gfx1250_op_V_CVT_NORM_U16_F16,"V_CVT_NORM_U16_F16"}, // 100
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 101
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 102
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 103
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 104
                {amdgpu_gfx1250_op_V_NOT_B16,"V_NOT_B16"}, // 105
                {amdgpu_gfx1250_op_V_CVT_I32_I16,"V_CVT_I32_I16"}, // 106
                {amdgpu_gfx1250_op_V_CVT_U32_U16,"V_CVT_U32_U16"}, // 107
                {amdgpu_gfx1250_op_V_CVT_F32_FP8,"V_CVT_F32_FP8"}, // 108
                {amdgpu_gfx1250_op_V_CVT_F32_BF8,"V_CVT_F32_BF8"}, // 109
                {amdgpu_gfx1250_op_V_CVT_PK_F32_FP8,"V_CVT_PK_F32_FP8"}, // 110
                {amdgpu_gfx1250_op_V_CVT_PK_F32_BF8,"V_CVT_PK_F32_BF8"}, // 111
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 112
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 113
                {amdgpu_gfx1250_op_V_CVT_F32_BF16,"V_CVT_F32_BF16"}, // 114
                {amdgpu_gfx1250_op_V_SAT_PK4_I4_I8,"V_SAT_PK4_I4_I8"}, // 115
                {amdgpu_gfx1250_op_V_SAT_PK4_U4_U8,"V_SAT_PK4_U4_U8"}, // 116
                {amdgpu_gfx1250_op_V_CVT_PK_F16_FP8,"V_CVT_PK_F16_FP8"}, // 117
                {amdgpu_gfx1250_op_V_CVT_PK_F16_BF8,"V_CVT_PK_F16_BF8"}, // 118
                {amdgpu_gfx1250_op_V_CVT_F16_FP8,"V_CVT_F16_FP8"}, // 119
                {amdgpu_gfx1250_op_V_CVT_F16_BF8,"V_CVT_F16_BF8"}, // 120
                {amdgpu_gfx1250_op_V_RCP_BF16,"V_RCP_BF16"}, // 121
                {amdgpu_gfx1250_op_V_SQRT_BF16,"V_SQRT_BF16"}, // 122
                {amdgpu_gfx1250_op_V_RSQ_BF16,"V_RSQ_BF16"}, // 123
                {amdgpu_gfx1250_op_V_LOG_BF16,"V_LOG_BF16"}, // 124
                {amdgpu_gfx1250_op_V_EXP_BF16,"V_EXP_BF16"}, // 125
                {amdgpu_gfx1250_op_V_SIN_BF16,"V_SIN_BF16"}, // 126
                {amdgpu_gfx1250_op_V_COS_BF16,"V_COS_BF16"}, // 127
            }; // end VOP1_INST_LITERAL64_insn_table
            const amdgpu_gfx1250_insn_entry VOP1_VOP_DPP8_insn_table[128] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_V_MOV_B32,"V_MOV_B32"}, // 1
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 2
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 3
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 4
                {amdgpu_gfx1250_op_V_CVT_F32_I32,"V_CVT_F32_I32"}, // 5
                {amdgpu_gfx1250_op_V_CVT_F32_U32,"V_CVT_F32_U32"}, // 6
                {amdgpu_gfx1250_op_V_CVT_U32_F32,"V_CVT_U32_F32"}, // 7
                {amdgpu_gfx1250_op_V_CVT_I32_F32,"V_CVT_I32_F32"}, // 8
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 9
                {amdgpu_gfx1250_op_V_CVT_F16_F32,"V_CVT_F16_F32"}, // 10
                {amdgpu_gfx1250_op_V_CVT_F32_F16,"V_CVT_F32_F16"}, // 11
                {amdgpu_gfx1250_op_V_CVT_NEAREST_I32_F32,"V_CVT_NEAREST_I32_F32"}, // 12
                {amdgpu_gfx1250_op_V_CVT_FLOOR_I32_F32,"V_CVT_FLOOR_I32_F32"}, // 13
                {amdgpu_gfx1250_op_V_CVT_OFF_F32_I4,"V_CVT_OFF_F32_I4"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE0,"V_CVT_F32_UBYTE0"}, // 17
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE1,"V_CVT_F32_UBYTE1"}, // 18
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE2,"V_CVT_F32_UBYTE2"}, // 19
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE3,"V_CVT_F32_UBYTE3"}, // 20
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 21
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 22
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 23
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 24
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 25
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 26
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 27
                {amdgpu_gfx1250_op_V_MOV_B16,"V_MOV_B16"}, // 28
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 29
                {amdgpu_gfx1250_op_V_TANH_F32,"V_TANH_F32"}, // 30
                {amdgpu_gfx1250_op_V_TANH_F16,"V_TANH_F16"}, // 31
                {amdgpu_gfx1250_op_V_FRACT_F32,"V_FRACT_F32"}, // 32
                {amdgpu_gfx1250_op_V_TRUNC_F32,"V_TRUNC_F32"}, // 33
                {amdgpu_gfx1250_op_V_CEIL_F32,"V_CEIL_F32"}, // 34
                {amdgpu_gfx1250_op_V_RNDNE_F32,"V_RNDNE_F32"}, // 35
                {amdgpu_gfx1250_op_V_FLOOR_F32,"V_FLOOR_F32"}, // 36
                {amdgpu_gfx1250_op_V_EXP_F32,"V_EXP_F32"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_V_LOG_F32,"V_LOG_F32"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_V_RCP_F32,"V_RCP_F32"}, // 42
                {amdgpu_gfx1250_op_V_RCP_IFLAG_F32,"V_RCP_IFLAG_F32"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_V_RSQ_F32,"V_RSQ_F32"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 49
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 50
                {amdgpu_gfx1250_op_V_SQRT_F32,"V_SQRT_F32"}, // 51
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 52
                {amdgpu_gfx1250_op_V_SIN_F32,"V_SIN_F32"}, // 53
                {amdgpu_gfx1250_op_V_COS_F32,"V_COS_F32"}, // 54
                {amdgpu_gfx1250_op_V_NOT_B32,"V_NOT_B32"}, // 55
                {amdgpu_gfx1250_op_V_BFREV_B32,"V_BFREV_B32"}, // 56
                {amdgpu_gfx1250_op_V_CLZ_I32_U32,"V_CLZ_I32_U32"}, // 57
                {amdgpu_gfx1250_op_V_CTZ_I32_B32,"V_CTZ_I32_B32"}, // 58
                {amdgpu_gfx1250_op_V_CLS_I32,"V_CLS_I32"}, // 59
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 60
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 61
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 62
                {amdgpu_gfx1250_op_V_FREXP_EXP_I32_F32,"V_FREXP_EXP_I32_F32"}, // 63
                {amdgpu_gfx1250_op_V_FREXP_MANT_F32,"V_FREXP_MANT_F32"}, // 64
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 65
                {amdgpu_gfx1250_op_V_MOVRELD_B32,"V_MOVRELD_B32"}, // 66
                {amdgpu_gfx1250_op_V_MOVRELS_B32,"V_MOVRELS_B32"}, // 67
                {amdgpu_gfx1250_op_V_MOVRELSD_B32,"V_MOVRELSD_B32"}, // 68
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 69
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_V_MOVRELSD_2_B32,"V_MOVRELSD_2_B32"}, // 72
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 73
                {amdgpu_gfx1250_op_V_TANH_BF16,"V_TANH_BF16"}, // 74
                {amdgpu_gfx1250_op_V_PRNG_B32,"V_PRNG_B32"}, // 75
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 76
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 77
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_V_CVT_F16_U16,"V_CVT_F16_U16"}, // 80
                {amdgpu_gfx1250_op_V_CVT_F16_I16,"V_CVT_F16_I16"}, // 81
                {amdgpu_gfx1250_op_V_CVT_U16_F16,"V_CVT_U16_F16"}, // 82
                {amdgpu_gfx1250_op_V_CVT_I16_F16,"V_CVT_I16_F16"}, // 83
                {amdgpu_gfx1250_op_V_RCP_F16,"V_RCP_F16"}, // 84
                {amdgpu_gfx1250_op_V_SQRT_F16,"V_SQRT_F16"}, // 85
                {amdgpu_gfx1250_op_V_RSQ_F16,"V_RSQ_F16"}, // 86
                {amdgpu_gfx1250_op_V_LOG_F16,"V_LOG_F16"}, // 87
                {amdgpu_gfx1250_op_V_EXP_F16,"V_EXP_F16"}, // 88
                {amdgpu_gfx1250_op_V_FREXP_MANT_F16,"V_FREXP_MANT_F16"}, // 89
                {amdgpu_gfx1250_op_V_FREXP_EXP_I16_F16,"V_FREXP_EXP_I16_F16"}, // 90
                {amdgpu_gfx1250_op_V_FLOOR_F16,"V_FLOOR_F16"}, // 91
                {amdgpu_gfx1250_op_V_CEIL_F16,"V_CEIL_F16"}, // 92
                {amdgpu_gfx1250_op_V_TRUNC_F16,"V_TRUNC_F16"}, // 93
                {amdgpu_gfx1250_op_V_RNDNE_F16,"V_RNDNE_F16"}, // 94
                {amdgpu_gfx1250_op_V_FRACT_F16,"V_FRACT_F16"}, // 95
                {amdgpu_gfx1250_op_V_SIN_F16,"V_SIN_F16"}, // 96
                {amdgpu_gfx1250_op_V_COS_F16,"V_COS_F16"}, // 97
                {amdgpu_gfx1250_op_V_SAT_PK_U8_I16,"V_SAT_PK_U8_I16"}, // 98
                {amdgpu_gfx1250_op_V_CVT_NORM_I16_F16,"V_CVT_NORM_I16_F16"}, // 99
                {amdgpu_gfx1250_op_V_CVT_NORM_U16_F16,"V_CVT_NORM_U16_F16"}, // 100
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 101
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 102
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 103
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 104
                {amdgpu_gfx1250_op_V_NOT_B16,"V_NOT_B16"}, // 105
                {amdgpu_gfx1250_op_V_CVT_I32_I16,"V_CVT_I32_I16"}, // 106
                {amdgpu_gfx1250_op_V_CVT_U32_U16,"V_CVT_U32_U16"}, // 107
                {amdgpu_gfx1250_op_V_CVT_F32_FP8,"V_CVT_F32_FP8"}, // 108
                {amdgpu_gfx1250_op_V_CVT_F32_BF8,"V_CVT_F32_BF8"}, // 109
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 110
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 111
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 112
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 113
                {amdgpu_gfx1250_op_V_CVT_F32_BF16,"V_CVT_F32_BF16"}, // 114
                {amdgpu_gfx1250_op_V_SAT_PK4_I4_I8,"V_SAT_PK4_I4_I8"}, // 115
                {amdgpu_gfx1250_op_V_SAT_PK4_U4_U8,"V_SAT_PK4_U4_U8"}, // 116
                {amdgpu_gfx1250_op_V_CVT_PK_F16_FP8,"V_CVT_PK_F16_FP8"}, // 117
                {amdgpu_gfx1250_op_V_CVT_PK_F16_BF8,"V_CVT_PK_F16_BF8"}, // 118
                {amdgpu_gfx1250_op_V_CVT_F16_FP8,"V_CVT_F16_FP8"}, // 119
                {amdgpu_gfx1250_op_V_CVT_F16_BF8,"V_CVT_F16_BF8"}, // 120
                {amdgpu_gfx1250_op_V_RCP_BF16,"V_RCP_BF16"}, // 121
                {amdgpu_gfx1250_op_V_SQRT_BF16,"V_SQRT_BF16"}, // 122
                {amdgpu_gfx1250_op_V_RSQ_BF16,"V_RSQ_BF16"}, // 123
                {amdgpu_gfx1250_op_V_LOG_BF16,"V_LOG_BF16"}, // 124
                {amdgpu_gfx1250_op_V_EXP_BF16,"V_EXP_BF16"}, // 125
                {amdgpu_gfx1250_op_V_SIN_BF16,"V_SIN_BF16"}, // 126
                {amdgpu_gfx1250_op_V_COS_BF16,"V_COS_BF16"}, // 127
            }; // end VOP1_VOP_DPP8_insn_table
            const amdgpu_gfx1250_insn_entry VOP1_VOP_DPP16_insn_table[128] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_V_MOV_B32,"V_MOV_B32"}, // 1
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 2
                {amdgpu_gfx1250_op_V_CVT_I32_F64,"V_CVT_I32_F64"}, // 3
                {amdgpu_gfx1250_op_V_CVT_F64_I32,"V_CVT_F64_I32"}, // 4
                {amdgpu_gfx1250_op_V_CVT_F32_I32,"V_CVT_F32_I32"}, // 5
                {amdgpu_gfx1250_op_V_CVT_F32_U32,"V_CVT_F32_U32"}, // 6
                {amdgpu_gfx1250_op_V_CVT_U32_F32,"V_CVT_U32_F32"}, // 7
                {amdgpu_gfx1250_op_V_CVT_I32_F32,"V_CVT_I32_F32"}, // 8
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 9
                {amdgpu_gfx1250_op_V_CVT_F16_F32,"V_CVT_F16_F32"}, // 10
                {amdgpu_gfx1250_op_V_CVT_F32_F16,"V_CVT_F32_F16"}, // 11
                {amdgpu_gfx1250_op_V_CVT_NEAREST_I32_F32,"V_CVT_NEAREST_I32_F32"}, // 12
                {amdgpu_gfx1250_op_V_CVT_FLOOR_I32_F32,"V_CVT_FLOOR_I32_F32"}, // 13
                {amdgpu_gfx1250_op_V_CVT_OFF_F32_I4,"V_CVT_OFF_F32_I4"}, // 14
                {amdgpu_gfx1250_op_V_CVT_F32_F64,"V_CVT_F32_F64"}, // 15
                {amdgpu_gfx1250_op_V_CVT_F64_F32,"V_CVT_F64_F32"}, // 16
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE0,"V_CVT_F32_UBYTE0"}, // 17
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE1,"V_CVT_F32_UBYTE1"}, // 18
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE2,"V_CVT_F32_UBYTE2"}, // 19
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE3,"V_CVT_F32_UBYTE3"}, // 20
                {amdgpu_gfx1250_op_V_CVT_U32_F64,"V_CVT_U32_F64"}, // 21
                {amdgpu_gfx1250_op_V_CVT_F64_U32,"V_CVT_F64_U32"}, // 22
                {amdgpu_gfx1250_op_V_TRUNC_F64,"V_TRUNC_F64"}, // 23
                {amdgpu_gfx1250_op_V_CEIL_F64,"V_CEIL_F64"}, // 24
                {amdgpu_gfx1250_op_V_RNDNE_F64,"V_RNDNE_F64"}, // 25
                {amdgpu_gfx1250_op_V_FLOOR_F64,"V_FLOOR_F64"}, // 26
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 27
                {amdgpu_gfx1250_op_V_MOV_B16,"V_MOV_B16"}, // 28
                {amdgpu_gfx1250_op_V_MOV_B64,"V_MOV_B64"}, // 29
                {amdgpu_gfx1250_op_V_TANH_F32,"V_TANH_F32"}, // 30
                {amdgpu_gfx1250_op_V_TANH_F16,"V_TANH_F16"}, // 31
                {amdgpu_gfx1250_op_V_FRACT_F32,"V_FRACT_F32"}, // 32
                {amdgpu_gfx1250_op_V_TRUNC_F32,"V_TRUNC_F32"}, // 33
                {amdgpu_gfx1250_op_V_CEIL_F32,"V_CEIL_F32"}, // 34
                {amdgpu_gfx1250_op_V_RNDNE_F32,"V_RNDNE_F32"}, // 35
                {amdgpu_gfx1250_op_V_FLOOR_F32,"V_FLOOR_F32"}, // 36
                {amdgpu_gfx1250_op_V_EXP_F32,"V_EXP_F32"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_V_LOG_F32,"V_LOG_F32"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_V_RCP_F32,"V_RCP_F32"}, // 42
                {amdgpu_gfx1250_op_V_RCP_IFLAG_F32,"V_RCP_IFLAG_F32"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_V_RSQ_F32,"V_RSQ_F32"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 49
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 50
                {amdgpu_gfx1250_op_V_SQRT_F32,"V_SQRT_F32"}, // 51
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 52
                {amdgpu_gfx1250_op_V_SIN_F32,"V_SIN_F32"}, // 53
                {amdgpu_gfx1250_op_V_COS_F32,"V_COS_F32"}, // 54
                {amdgpu_gfx1250_op_V_NOT_B32,"V_NOT_B32"}, // 55
                {amdgpu_gfx1250_op_V_BFREV_B32,"V_BFREV_B32"}, // 56
                {amdgpu_gfx1250_op_V_CLZ_I32_U32,"V_CLZ_I32_U32"}, // 57
                {amdgpu_gfx1250_op_V_CTZ_I32_B32,"V_CTZ_I32_B32"}, // 58
                {amdgpu_gfx1250_op_V_CLS_I32,"V_CLS_I32"}, // 59
                {amdgpu_gfx1250_op_V_FREXP_EXP_I32_F64,"V_FREXP_EXP_I32_F64"}, // 60
                {amdgpu_gfx1250_op_V_FREXP_MANT_F64,"V_FREXP_MANT_F64"}, // 61
                {amdgpu_gfx1250_op_V_FRACT_F64,"V_FRACT_F64"}, // 62
                {amdgpu_gfx1250_op_V_FREXP_EXP_I32_F32,"V_FREXP_EXP_I32_F32"}, // 63
                {amdgpu_gfx1250_op_V_FREXP_MANT_F32,"V_FREXP_MANT_F32"}, // 64
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 65
                {amdgpu_gfx1250_op_V_MOVRELD_B32,"V_MOVRELD_B32"}, // 66
                {amdgpu_gfx1250_op_V_MOVRELS_B32,"V_MOVRELS_B32"}, // 67
                {amdgpu_gfx1250_op_V_MOVRELSD_B32,"V_MOVRELSD_B32"}, // 68
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 69
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_V_MOVRELSD_2_B32,"V_MOVRELSD_2_B32"}, // 72
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 73
                {amdgpu_gfx1250_op_V_TANH_BF16,"V_TANH_BF16"}, // 74
                {amdgpu_gfx1250_op_V_PRNG_B32,"V_PRNG_B32"}, // 75
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 76
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 77
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_V_CVT_F16_U16,"V_CVT_F16_U16"}, // 80
                {amdgpu_gfx1250_op_V_CVT_F16_I16,"V_CVT_F16_I16"}, // 81
                {amdgpu_gfx1250_op_V_CVT_U16_F16,"V_CVT_U16_F16"}, // 82
                {amdgpu_gfx1250_op_V_CVT_I16_F16,"V_CVT_I16_F16"}, // 83
                {amdgpu_gfx1250_op_V_RCP_F16,"V_RCP_F16"}, // 84
                {amdgpu_gfx1250_op_V_SQRT_F16,"V_SQRT_F16"}, // 85
                {amdgpu_gfx1250_op_V_RSQ_F16,"V_RSQ_F16"}, // 86
                {amdgpu_gfx1250_op_V_LOG_F16,"V_LOG_F16"}, // 87
                {amdgpu_gfx1250_op_V_EXP_F16,"V_EXP_F16"}, // 88
                {amdgpu_gfx1250_op_V_FREXP_MANT_F16,"V_FREXP_MANT_F16"}, // 89
                {amdgpu_gfx1250_op_V_FREXP_EXP_I16_F16,"V_FREXP_EXP_I16_F16"}, // 90
                {amdgpu_gfx1250_op_V_FLOOR_F16,"V_FLOOR_F16"}, // 91
                {amdgpu_gfx1250_op_V_CEIL_F16,"V_CEIL_F16"}, // 92
                {amdgpu_gfx1250_op_V_TRUNC_F16,"V_TRUNC_F16"}, // 93
                {amdgpu_gfx1250_op_V_RNDNE_F16,"V_RNDNE_F16"}, // 94
                {amdgpu_gfx1250_op_V_FRACT_F16,"V_FRACT_F16"}, // 95
                {amdgpu_gfx1250_op_V_SIN_F16,"V_SIN_F16"}, // 96
                {amdgpu_gfx1250_op_V_COS_F16,"V_COS_F16"}, // 97
                {amdgpu_gfx1250_op_V_SAT_PK_U8_I16,"V_SAT_PK_U8_I16"}, // 98
                {amdgpu_gfx1250_op_V_CVT_NORM_I16_F16,"V_CVT_NORM_I16_F16"}, // 99
                {amdgpu_gfx1250_op_V_CVT_NORM_U16_F16,"V_CVT_NORM_U16_F16"}, // 100
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 101
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 102
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 103
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 104
                {amdgpu_gfx1250_op_V_NOT_B16,"V_NOT_B16"}, // 105
                {amdgpu_gfx1250_op_V_CVT_I32_I16,"V_CVT_I32_I16"}, // 106
                {amdgpu_gfx1250_op_V_CVT_U32_U16,"V_CVT_U32_U16"}, // 107
                {amdgpu_gfx1250_op_V_CVT_F32_FP8,"V_CVT_F32_FP8"}, // 108
                {amdgpu_gfx1250_op_V_CVT_F32_BF8,"V_CVT_F32_BF8"}, // 109
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 110
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 111
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 112
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 113
                {amdgpu_gfx1250_op_V_CVT_F32_BF16,"V_CVT_F32_BF16"}, // 114
                {amdgpu_gfx1250_op_V_SAT_PK4_I4_I8,"V_SAT_PK4_I4_I8"}, // 115
                {amdgpu_gfx1250_op_V_SAT_PK4_U4_U8,"V_SAT_PK4_U4_U8"}, // 116
                {amdgpu_gfx1250_op_V_CVT_PK_F16_FP8,"V_CVT_PK_F16_FP8"}, // 117
                {amdgpu_gfx1250_op_V_CVT_PK_F16_BF8,"V_CVT_PK_F16_BF8"}, // 118
                {amdgpu_gfx1250_op_V_CVT_F16_FP8,"V_CVT_F16_FP8"}, // 119
                {amdgpu_gfx1250_op_V_CVT_F16_BF8,"V_CVT_F16_BF8"}, // 120
                {amdgpu_gfx1250_op_V_RCP_BF16,"V_RCP_BF16"}, // 121
                {amdgpu_gfx1250_op_V_SQRT_BF16,"V_SQRT_BF16"}, // 122
                {amdgpu_gfx1250_op_V_RSQ_BF16,"V_RSQ_BF16"}, // 123
                {amdgpu_gfx1250_op_V_LOG_BF16,"V_LOG_BF16"}, // 124
                {amdgpu_gfx1250_op_V_EXP_BF16,"V_EXP_BF16"}, // 125
                {amdgpu_gfx1250_op_V_SIN_BF16,"V_SIN_BF16"}, // 126
                {amdgpu_gfx1250_op_V_COS_BF16,"V_COS_BF16"}, // 127
            }; // end VOP1_VOP_DPP16_insn_table
            const amdgpu_gfx1250_insn_entry VOP3_INST_LITERAL_insn_table[886] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_V_CMP_LT_F16,"V_CMP_LT_F16"}, // 1
                {amdgpu_gfx1250_op_V_CMP_EQ_F16,"V_CMP_EQ_F16"}, // 2
                {amdgpu_gfx1250_op_V_CMP_LE_F16,"V_CMP_LE_F16"}, // 3
                {amdgpu_gfx1250_op_V_CMP_GT_F16,"V_CMP_GT_F16"}, // 4
                {amdgpu_gfx1250_op_V_CMP_LG_F16,"V_CMP_LG_F16"}, // 5
                {amdgpu_gfx1250_op_V_CMP_GE_F16,"V_CMP_GE_F16"}, // 6
                {amdgpu_gfx1250_op_V_CMP_O_F16,"V_CMP_O_F16"}, // 7
                {amdgpu_gfx1250_op_V_CMP_U_F16,"V_CMP_U_F16"}, // 8
                {amdgpu_gfx1250_op_V_CMP_NGE_F16,"V_CMP_NGE_F16"}, // 9
                {amdgpu_gfx1250_op_V_CMP_NLG_F16,"V_CMP_NLG_F16"}, // 10
                {amdgpu_gfx1250_op_V_CMP_NGT_F16,"V_CMP_NGT_F16"}, // 11
                {amdgpu_gfx1250_op_V_CMP_NLE_F16,"V_CMP_NLE_F16"}, // 12
                {amdgpu_gfx1250_op_V_CMP_NEQ_F16,"V_CMP_NEQ_F16"}, // 13
                {amdgpu_gfx1250_op_V_CMP_NLT_F16,"V_CMP_NLT_F16"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_V_CMP_LT_F32,"V_CMP_LT_F32"}, // 17
                {amdgpu_gfx1250_op_V_CMP_EQ_F32,"V_CMP_EQ_F32"}, // 18
                {amdgpu_gfx1250_op_V_CMP_LE_F32,"V_CMP_LE_F32"}, // 19
                {amdgpu_gfx1250_op_V_CMP_GT_F32,"V_CMP_GT_F32"}, // 20
                {amdgpu_gfx1250_op_V_CMP_LG_F32,"V_CMP_LG_F32"}, // 21
                {amdgpu_gfx1250_op_V_CMP_GE_F32,"V_CMP_GE_F32"}, // 22
                {amdgpu_gfx1250_op_V_CMP_O_F32,"V_CMP_O_F32"}, // 23
                {amdgpu_gfx1250_op_V_CMP_U_F32,"V_CMP_U_F32"}, // 24
                {amdgpu_gfx1250_op_V_CMP_NGE_F32,"V_CMP_NGE_F32"}, // 25
                {amdgpu_gfx1250_op_V_CMP_NLG_F32,"V_CMP_NLG_F32"}, // 26
                {amdgpu_gfx1250_op_V_CMP_NGT_F32,"V_CMP_NGT_F32"}, // 27
                {amdgpu_gfx1250_op_V_CMP_NLE_F32,"V_CMP_NLE_F32"}, // 28
                {amdgpu_gfx1250_op_V_CMP_NEQ_F32,"V_CMP_NEQ_F32"}, // 29
                {amdgpu_gfx1250_op_V_CMP_NLT_F32,"V_CMP_NLT_F32"}, // 30
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 31
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 32
                {amdgpu_gfx1250_op_V_CMP_LT_F64,"V_CMP_LT_F64"}, // 33
                {amdgpu_gfx1250_op_V_CMP_EQ_F64,"V_CMP_EQ_F64"}, // 34
                {amdgpu_gfx1250_op_V_CMP_LE_F64,"V_CMP_LE_F64"}, // 35
                {amdgpu_gfx1250_op_V_CMP_GT_F64,"V_CMP_GT_F64"}, // 36
                {amdgpu_gfx1250_op_V_CMP_LG_F64,"V_CMP_LG_F64"}, // 37
                {amdgpu_gfx1250_op_V_CMP_GE_F64,"V_CMP_GE_F64"}, // 38
                {amdgpu_gfx1250_op_V_CMP_O_F64,"V_CMP_O_F64"}, // 39
                {amdgpu_gfx1250_op_V_CMP_U_F64,"V_CMP_U_F64"}, // 40
                {amdgpu_gfx1250_op_V_CMP_NGE_F64,"V_CMP_NGE_F64"}, // 41
                {amdgpu_gfx1250_op_V_CMP_NLG_F64,"V_CMP_NLG_F64"}, // 42
                {amdgpu_gfx1250_op_V_CMP_NGT_F64,"V_CMP_NGT_F64"}, // 43
                {amdgpu_gfx1250_op_V_CMP_NLE_F64,"V_CMP_NLE_F64"}, // 44
                {amdgpu_gfx1250_op_V_CMP_NEQ_F64,"V_CMP_NEQ_F64"}, // 45
                {amdgpu_gfx1250_op_V_CMP_NLT_F64,"V_CMP_NLT_F64"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_V_CMP_LT_I16,"V_CMP_LT_I16"}, // 49
                {amdgpu_gfx1250_op_V_CMP_EQ_I16,"V_CMP_EQ_I16"}, // 50
                {amdgpu_gfx1250_op_V_CMP_LE_I16,"V_CMP_LE_I16"}, // 51
                {amdgpu_gfx1250_op_V_CMP_GT_I16,"V_CMP_GT_I16"}, // 52
                {amdgpu_gfx1250_op_V_CMP_NE_I16,"V_CMP_NE_I16"}, // 53
                {amdgpu_gfx1250_op_V_CMP_GE_I16,"V_CMP_GE_I16"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_V_CMP_LT_U16,"V_CMP_LT_U16"}, // 57
                {amdgpu_gfx1250_op_V_CMP_EQ_U16,"V_CMP_EQ_U16"}, // 58
                {amdgpu_gfx1250_op_V_CMP_LE_U16,"V_CMP_LE_U16"}, // 59
                {amdgpu_gfx1250_op_V_CMP_GT_U16,"V_CMP_GT_U16"}, // 60
                {amdgpu_gfx1250_op_V_CMP_NE_U16,"V_CMP_NE_U16"}, // 61
                {amdgpu_gfx1250_op_V_CMP_GE_U16,"V_CMP_GE_U16"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 64
                {amdgpu_gfx1250_op_V_CMP_LT_I32,"V_CMP_LT_I32"}, // 65
                {amdgpu_gfx1250_op_V_CMP_EQ_I32,"V_CMP_EQ_I32"}, // 66
                {amdgpu_gfx1250_op_V_CMP_LE_I32,"V_CMP_LE_I32"}, // 67
                {amdgpu_gfx1250_op_V_CMP_GT_I32,"V_CMP_GT_I32"}, // 68
                {amdgpu_gfx1250_op_V_CMP_NE_I32,"V_CMP_NE_I32"}, // 69
                {amdgpu_gfx1250_op_V_CMP_GE_I32,"V_CMP_GE_I32"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 72
                {amdgpu_gfx1250_op_V_CMP_LT_U32,"V_CMP_LT_U32"}, // 73
                {amdgpu_gfx1250_op_V_CMP_EQ_U32,"V_CMP_EQ_U32"}, // 74
                {amdgpu_gfx1250_op_V_CMP_LE_U32,"V_CMP_LE_U32"}, // 75
                {amdgpu_gfx1250_op_V_CMP_GT_U32,"V_CMP_GT_U32"}, // 76
                {amdgpu_gfx1250_op_V_CMP_NE_U32,"V_CMP_NE_U32"}, // 77
                {amdgpu_gfx1250_op_V_CMP_GE_U32,"V_CMP_GE_U32"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 80
                {amdgpu_gfx1250_op_V_CMP_LT_I64,"V_CMP_LT_I64"}, // 81
                {amdgpu_gfx1250_op_V_CMP_EQ_I64,"V_CMP_EQ_I64"}, // 82
                {amdgpu_gfx1250_op_V_CMP_LE_I64,"V_CMP_LE_I64"}, // 83
                {amdgpu_gfx1250_op_V_CMP_GT_I64,"V_CMP_GT_I64"}, // 84
                {amdgpu_gfx1250_op_V_CMP_NE_I64,"V_CMP_NE_I64"}, // 85
                {amdgpu_gfx1250_op_V_CMP_GE_I64,"V_CMP_GE_I64"}, // 86
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 87
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 88
                {amdgpu_gfx1250_op_V_CMP_LT_U64,"V_CMP_LT_U64"}, // 89
                {amdgpu_gfx1250_op_V_CMP_EQ_U64,"V_CMP_EQ_U64"}, // 90
                {amdgpu_gfx1250_op_V_CMP_LE_U64,"V_CMP_LE_U64"}, // 91
                {amdgpu_gfx1250_op_V_CMP_GT_U64,"V_CMP_GT_U64"}, // 92
                {amdgpu_gfx1250_op_V_CMP_NE_U64,"V_CMP_NE_U64"}, // 93
                {amdgpu_gfx1250_op_V_CMP_GE_U64,"V_CMP_GE_U64"}, // 94
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 95
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 96
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 97
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 98
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 99
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 100
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 101
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 102
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 103
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 104
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 105
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 106
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 107
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 108
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 109
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 110
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 111
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 112
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 113
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 114
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 115
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 116
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 117
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 118
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 119
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 120
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 121
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 122
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 123
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 124
                {amdgpu_gfx1250_op_V_CMP_CLASS_F16,"V_CMP_CLASS_F16"}, // 125
                {amdgpu_gfx1250_op_V_CMP_CLASS_F32,"V_CMP_CLASS_F32"}, // 126
                {amdgpu_gfx1250_op_V_CMP_CLASS_F64,"V_CMP_CLASS_F64"}, // 127
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 128
                {amdgpu_gfx1250_op_V_CMPX_LT_F16,"V_CMPX_LT_F16"}, // 129
                {amdgpu_gfx1250_op_V_CMPX_EQ_F16,"V_CMPX_EQ_F16"}, // 130
                {amdgpu_gfx1250_op_V_CMPX_LE_F16,"V_CMPX_LE_F16"}, // 131
                {amdgpu_gfx1250_op_V_CMPX_GT_F16,"V_CMPX_GT_F16"}, // 132
                {amdgpu_gfx1250_op_V_CMPX_LG_F16,"V_CMPX_LG_F16"}, // 133
                {amdgpu_gfx1250_op_V_CMPX_GE_F16,"V_CMPX_GE_F16"}, // 134
                {amdgpu_gfx1250_op_V_CMPX_O_F16,"V_CMPX_O_F16"}, // 135
                {amdgpu_gfx1250_op_V_CMPX_U_F16,"V_CMPX_U_F16"}, // 136
                {amdgpu_gfx1250_op_V_CMPX_NGE_F16,"V_CMPX_NGE_F16"}, // 137
                {amdgpu_gfx1250_op_V_CMPX_NLG_F16,"V_CMPX_NLG_F16"}, // 138
                {amdgpu_gfx1250_op_V_CMPX_NGT_F16,"V_CMPX_NGT_F16"}, // 139
                {amdgpu_gfx1250_op_V_CMPX_NLE_F16,"V_CMPX_NLE_F16"}, // 140
                {amdgpu_gfx1250_op_V_CMPX_NEQ_F16,"V_CMPX_NEQ_F16"}, // 141
                {amdgpu_gfx1250_op_V_CMPX_NLT_F16,"V_CMPX_NLT_F16"}, // 142
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 143
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 144
                {amdgpu_gfx1250_op_V_CMPX_LT_F32,"V_CMPX_LT_F32"}, // 145
                {amdgpu_gfx1250_op_V_CMPX_EQ_F32,"V_CMPX_EQ_F32"}, // 146
                {amdgpu_gfx1250_op_V_CMPX_LE_F32,"V_CMPX_LE_F32"}, // 147
                {amdgpu_gfx1250_op_V_CMPX_GT_F32,"V_CMPX_GT_F32"}, // 148
                {amdgpu_gfx1250_op_V_CMPX_LG_F32,"V_CMPX_LG_F32"}, // 149
                {amdgpu_gfx1250_op_V_CMPX_GE_F32,"V_CMPX_GE_F32"}, // 150
                {amdgpu_gfx1250_op_V_CMPX_O_F32,"V_CMPX_O_F32"}, // 151
                {amdgpu_gfx1250_op_V_CMPX_U_F32,"V_CMPX_U_F32"}, // 152
                {amdgpu_gfx1250_op_V_CMPX_NGE_F32,"V_CMPX_NGE_F32"}, // 153
                {amdgpu_gfx1250_op_V_CMPX_NLG_F32,"V_CMPX_NLG_F32"}, // 154
                {amdgpu_gfx1250_op_V_CMPX_NGT_F32,"V_CMPX_NGT_F32"}, // 155
                {amdgpu_gfx1250_op_V_CMPX_NLE_F32,"V_CMPX_NLE_F32"}, // 156
                {amdgpu_gfx1250_op_V_CMPX_NEQ_F32,"V_CMPX_NEQ_F32"}, // 157
                {amdgpu_gfx1250_op_V_CMPX_NLT_F32,"V_CMPX_NLT_F32"}, // 158
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 159
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 160
                {amdgpu_gfx1250_op_V_CMPX_LT_F64,"V_CMPX_LT_F64"}, // 161
                {amdgpu_gfx1250_op_V_CMPX_EQ_F64,"V_CMPX_EQ_F64"}, // 162
                {amdgpu_gfx1250_op_V_CMPX_LE_F64,"V_CMPX_LE_F64"}, // 163
                {amdgpu_gfx1250_op_V_CMPX_GT_F64,"V_CMPX_GT_F64"}, // 164
                {amdgpu_gfx1250_op_V_CMPX_LG_F64,"V_CMPX_LG_F64"}, // 165
                {amdgpu_gfx1250_op_V_CMPX_GE_F64,"V_CMPX_GE_F64"}, // 166
                {amdgpu_gfx1250_op_V_CMPX_O_F64,"V_CMPX_O_F64"}, // 167
                {amdgpu_gfx1250_op_V_CMPX_U_F64,"V_CMPX_U_F64"}, // 168
                {amdgpu_gfx1250_op_V_CMPX_NGE_F64,"V_CMPX_NGE_F64"}, // 169
                {amdgpu_gfx1250_op_V_CMPX_NLG_F64,"V_CMPX_NLG_F64"}, // 170
                {amdgpu_gfx1250_op_V_CMPX_NGT_F64,"V_CMPX_NGT_F64"}, // 171
                {amdgpu_gfx1250_op_V_CMPX_NLE_F64,"V_CMPX_NLE_F64"}, // 172
                {amdgpu_gfx1250_op_V_CMPX_NEQ_F64,"V_CMPX_NEQ_F64"}, // 173
                {amdgpu_gfx1250_op_V_CMPX_NLT_F64,"V_CMPX_NLT_F64"}, // 174
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 175
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 176
                {amdgpu_gfx1250_op_V_CMPX_LT_I16,"V_CMPX_LT_I16"}, // 177
                {amdgpu_gfx1250_op_V_CMPX_EQ_I16,"V_CMPX_EQ_I16"}, // 178
                {amdgpu_gfx1250_op_V_CMPX_LE_I16,"V_CMPX_LE_I16"}, // 179
                {amdgpu_gfx1250_op_V_CMPX_GT_I16,"V_CMPX_GT_I16"}, // 180
                {amdgpu_gfx1250_op_V_CMPX_NE_I16,"V_CMPX_NE_I16"}, // 181
                {amdgpu_gfx1250_op_V_CMPX_GE_I16,"V_CMPX_GE_I16"}, // 182
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 183
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 184
                {amdgpu_gfx1250_op_V_CMPX_LT_U16,"V_CMPX_LT_U16"}, // 185
                {amdgpu_gfx1250_op_V_CMPX_EQ_U16,"V_CMPX_EQ_U16"}, // 186
                {amdgpu_gfx1250_op_V_CMPX_LE_U16,"V_CMPX_LE_U16"}, // 187
                {amdgpu_gfx1250_op_V_CMPX_GT_U16,"V_CMPX_GT_U16"}, // 188
                {amdgpu_gfx1250_op_V_CMPX_NE_U16,"V_CMPX_NE_U16"}, // 189
                {amdgpu_gfx1250_op_V_CMPX_GE_U16,"V_CMPX_GE_U16"}, // 190
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 191
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 192
                {amdgpu_gfx1250_op_V_CMPX_LT_I32,"V_CMPX_LT_I32"}, // 193
                {amdgpu_gfx1250_op_V_CMPX_EQ_I32,"V_CMPX_EQ_I32"}, // 194
                {amdgpu_gfx1250_op_V_CMPX_LE_I32,"V_CMPX_LE_I32"}, // 195
                {amdgpu_gfx1250_op_V_CMPX_GT_I32,"V_CMPX_GT_I32"}, // 196
                {amdgpu_gfx1250_op_V_CMPX_NE_I32,"V_CMPX_NE_I32"}, // 197
                {amdgpu_gfx1250_op_V_CMPX_GE_I32,"V_CMPX_GE_I32"}, // 198
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 199
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 200
                {amdgpu_gfx1250_op_V_CMPX_LT_U32,"V_CMPX_LT_U32"}, // 201
                {amdgpu_gfx1250_op_V_CMPX_EQ_U32,"V_CMPX_EQ_U32"}, // 202
                {amdgpu_gfx1250_op_V_CMPX_LE_U32,"V_CMPX_LE_U32"}, // 203
                {amdgpu_gfx1250_op_V_CMPX_GT_U32,"V_CMPX_GT_U32"}, // 204
                {amdgpu_gfx1250_op_V_CMPX_NE_U32,"V_CMPX_NE_U32"}, // 205
                {amdgpu_gfx1250_op_V_CMPX_GE_U32,"V_CMPX_GE_U32"}, // 206
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 207
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 208
                {amdgpu_gfx1250_op_V_CMPX_LT_I64,"V_CMPX_LT_I64"}, // 209
                {amdgpu_gfx1250_op_V_CMPX_EQ_I64,"V_CMPX_EQ_I64"}, // 210
                {amdgpu_gfx1250_op_V_CMPX_LE_I64,"V_CMPX_LE_I64"}, // 211
                {amdgpu_gfx1250_op_V_CMPX_GT_I64,"V_CMPX_GT_I64"}, // 212
                {amdgpu_gfx1250_op_V_CMPX_NE_I64,"V_CMPX_NE_I64"}, // 213
                {amdgpu_gfx1250_op_V_CMPX_GE_I64,"V_CMPX_GE_I64"}, // 214
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 215
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 216
                {amdgpu_gfx1250_op_V_CMPX_LT_U64,"V_CMPX_LT_U64"}, // 217
                {amdgpu_gfx1250_op_V_CMPX_EQ_U64,"V_CMPX_EQ_U64"}, // 218
                {amdgpu_gfx1250_op_V_CMPX_LE_U64,"V_CMPX_LE_U64"}, // 219
                {amdgpu_gfx1250_op_V_CMPX_GT_U64,"V_CMPX_GT_U64"}, // 220
                {amdgpu_gfx1250_op_V_CMPX_NE_U64,"V_CMPX_NE_U64"}, // 221
                {amdgpu_gfx1250_op_V_CMPX_GE_U64,"V_CMPX_GE_U64"}, // 222
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 223
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 224
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 225
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 226
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 227
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 228
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 229
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 230
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 231
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 232
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 233
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 234
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 235
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 236
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 237
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 238
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 239
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 240
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 241
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 242
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 243
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 244
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 245
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 246
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 247
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 248
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 249
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 250
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 251
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 252
                {amdgpu_gfx1250_op_V_CMPX_CLASS_F16,"V_CMPX_CLASS_F16"}, // 253
                {amdgpu_gfx1250_op_V_CMPX_CLASS_F32,"V_CMPX_CLASS_F32"}, // 254
                {amdgpu_gfx1250_op_V_CMPX_CLASS_F64,"V_CMPX_CLASS_F64"}, // 255
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 256
                {amdgpu_gfx1250_op_V_CNDMASK_B32,"V_CNDMASK_B32"}, // 257
                {amdgpu_gfx1250_op_V_ADD_F64,"V_ADD_F64"}, // 258
                {amdgpu_gfx1250_op_V_ADD_F32,"V_ADD_F32"}, // 259
                {amdgpu_gfx1250_op_V_SUB_F32,"V_SUB_F32"}, // 260
                {amdgpu_gfx1250_op_V_SUBREV_F32,"V_SUBREV_F32"}, // 261
                {amdgpu_gfx1250_op_V_MUL_F64,"V_MUL_F64"}, // 262
                {amdgpu_gfx1250_op_V_MUL_DX9_ZERO_F32,"V_MUL_DX9_ZERO_F32"}, // 263
                {amdgpu_gfx1250_op_V_MUL_F32,"V_MUL_F32"}, // 264
                {amdgpu_gfx1250_op_V_MUL_I32_I24,"V_MUL_I32_I24"}, // 265
                {amdgpu_gfx1250_op_V_MUL_HI_I32_I24,"V_MUL_HI_I32_I24"}, // 266
                {amdgpu_gfx1250_op_V_MUL_U32_U24,"V_MUL_U32_U24"}, // 267
                {amdgpu_gfx1250_op_V_MUL_HI_U32_U24,"V_MUL_HI_U32_U24"}, // 268
                {amdgpu_gfx1250_op_V_MIN_NUM_F64,"V_MIN_NUM_F64"}, // 269
                {amdgpu_gfx1250_op_V_MAX_NUM_F64,"V_MAX_NUM_F64"}, // 270
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 271
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 272
                {amdgpu_gfx1250_op_V_MIN_I32,"V_MIN_I32"}, // 273
                {amdgpu_gfx1250_op_V_MAX_I32,"V_MAX_I32"}, // 274
                {amdgpu_gfx1250_op_V_MIN_U32,"V_MIN_U32"}, // 275
                {amdgpu_gfx1250_op_V_MAX_U32,"V_MAX_U32"}, // 276
                {amdgpu_gfx1250_op_V_MIN_NUM_F32,"V_MIN_NUM_F32"}, // 277
                {amdgpu_gfx1250_op_V_MAX_NUM_F32,"V_MAX_NUM_F32"}, // 278
                {amdgpu_gfx1250_op_V_FMAC_F64,"V_FMAC_F64"}, // 279
                {amdgpu_gfx1250_op_V_LSHLREV_B32,"V_LSHLREV_B32"}, // 280
                {amdgpu_gfx1250_op_V_LSHRREV_B32,"V_LSHRREV_B32"}, // 281
                {amdgpu_gfx1250_op_V_ASHRREV_I32,"V_ASHRREV_I32"}, // 282
                {amdgpu_gfx1250_op_V_AND_B32,"V_AND_B32"}, // 283
                {amdgpu_gfx1250_op_V_OR_B32,"V_OR_B32"}, // 284
                {amdgpu_gfx1250_op_V_XOR_B32,"V_XOR_B32"}, // 285
                {amdgpu_gfx1250_op_V_XNOR_B32,"V_XNOR_B32"}, // 286
                {amdgpu_gfx1250_op_V_LSHLREV_B64,"V_LSHLREV_B64"}, // 287
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 288
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 289
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 290
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 291
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 292
                {amdgpu_gfx1250_op_V_ADD_NC_U32,"V_ADD_NC_U32"}, // 293
                {amdgpu_gfx1250_op_V_SUB_NC_U32,"V_SUB_NC_U32"}, // 294
                {amdgpu_gfx1250_op_V_SUBREV_NC_U32,"V_SUBREV_NC_U32"}, // 295
                {amdgpu_gfx1250_op_V_ADD_NC_U64,"V_ADD_NC_U64"}, // 296
                {amdgpu_gfx1250_op_V_SUB_NC_U64,"V_SUB_NC_U64"}, // 297
                {amdgpu_gfx1250_op_V_MUL_U64,"V_MUL_U64"}, // 298
                {amdgpu_gfx1250_op_V_FMAC_F32,"V_FMAC_F32"}, // 299
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 300
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 301
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 302
                {amdgpu_gfx1250_op_V_CVT_PK_RTZ_F16_F32,"V_CVT_PK_RTZ_F16_F32"}, // 303
                {amdgpu_gfx1250_op_V_MIN_NUM_F16,"V_MIN_NUM_F16"}, // 304
                {amdgpu_gfx1250_op_V_MAX_NUM_F16,"V_MAX_NUM_F16"}, // 305
                {amdgpu_gfx1250_op_V_ADD_F16,"V_ADD_F16"}, // 306
                {amdgpu_gfx1250_op_V_SUB_F16,"V_SUB_F16"}, // 307
                {amdgpu_gfx1250_op_V_SUBREV_F16,"V_SUBREV_F16"}, // 308
                {amdgpu_gfx1250_op_V_MUL_F16,"V_MUL_F16"}, // 309
                {amdgpu_gfx1250_op_V_FMAC_F16,"V_FMAC_F16"}, // 310
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 311
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 312
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 313
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 314
                {amdgpu_gfx1250_op_V_LDEXP_F16,"V_LDEXP_F16"}, // 315
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 316
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 317
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 318
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 319
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 320
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 321
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 322
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 323
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 324
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 325
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 326
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 327
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 328
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 329
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 330
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 331
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 332
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 333
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 334
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 335
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 336
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 337
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 338
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 339
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 340
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 341
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 342
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 343
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 344
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 345
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 346
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 347
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 348
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 349
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 350
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 351
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 352
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 353
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 354
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 355
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 356
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 357
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 358
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 359
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 360
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 361
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 362
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 363
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 364
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 365
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 366
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 367
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 368
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 369
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 370
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 371
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 372
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 373
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 374
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 375
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 376
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 377
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 378
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 379
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 380
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 381
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 382
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 383
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 384
                {amdgpu_gfx1250_op_V_MOV_B32,"V_MOV_B32"}, // 385
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 386
                {amdgpu_gfx1250_op_V_CVT_I32_F64,"V_CVT_I32_F64"}, // 387
                {amdgpu_gfx1250_op_V_CVT_F64_I32,"V_CVT_F64_I32"}, // 388
                {amdgpu_gfx1250_op_V_CVT_F32_I32,"V_CVT_F32_I32"}, // 389
                {amdgpu_gfx1250_op_V_CVT_F32_U32,"V_CVT_F32_U32"}, // 390
                {amdgpu_gfx1250_op_V_CVT_U32_F32,"V_CVT_U32_F32"}, // 391
                {amdgpu_gfx1250_op_V_CVT_I32_F32,"V_CVT_I32_F32"}, // 392
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 393
                {amdgpu_gfx1250_op_V_CVT_F16_F32,"V_CVT_F16_F32"}, // 394
                {amdgpu_gfx1250_op_V_CVT_F32_F16,"V_CVT_F32_F16"}, // 395
                {amdgpu_gfx1250_op_V_CVT_NEAREST_I32_F32,"V_CVT_NEAREST_I32_F32"}, // 396
                {amdgpu_gfx1250_op_V_CVT_FLOOR_I32_F32,"V_CVT_FLOOR_I32_F32"}, // 397
                {amdgpu_gfx1250_op_V_CVT_OFF_F32_I4,"V_CVT_OFF_F32_I4"}, // 398
                {amdgpu_gfx1250_op_V_CVT_F32_F64,"V_CVT_F32_F64"}, // 399
                {amdgpu_gfx1250_op_V_CVT_F64_F32,"V_CVT_F64_F32"}, // 400
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE0,"V_CVT_F32_UBYTE0"}, // 401
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE1,"V_CVT_F32_UBYTE1"}, // 402
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE2,"V_CVT_F32_UBYTE2"}, // 403
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE3,"V_CVT_F32_UBYTE3"}, // 404
                {amdgpu_gfx1250_op_V_CVT_U32_F64,"V_CVT_U32_F64"}, // 405
                {amdgpu_gfx1250_op_V_CVT_F64_U32,"V_CVT_F64_U32"}, // 406
                {amdgpu_gfx1250_op_V_TRUNC_F64,"V_TRUNC_F64"}, // 407
                {amdgpu_gfx1250_op_V_CEIL_F64,"V_CEIL_F64"}, // 408
                {amdgpu_gfx1250_op_V_RNDNE_F64,"V_RNDNE_F64"}, // 409
                {amdgpu_gfx1250_op_V_FLOOR_F64,"V_FLOOR_F64"}, // 410
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 411
                {amdgpu_gfx1250_op_V_MOV_B16,"V_MOV_B16"}, // 412
                {amdgpu_gfx1250_op_V_MOV_B64,"V_MOV_B64"}, // 413
                {amdgpu_gfx1250_op_V_TANH_F32,"V_TANH_F32"}, // 414
                {amdgpu_gfx1250_op_V_TANH_F16,"V_TANH_F16"}, // 415
                {amdgpu_gfx1250_op_V_FRACT_F32,"V_FRACT_F32"}, // 416
                {amdgpu_gfx1250_op_V_TRUNC_F32,"V_TRUNC_F32"}, // 417
                {amdgpu_gfx1250_op_V_CEIL_F32,"V_CEIL_F32"}, // 418
                {amdgpu_gfx1250_op_V_RNDNE_F32,"V_RNDNE_F32"}, // 419
                {amdgpu_gfx1250_op_V_FLOOR_F32,"V_FLOOR_F32"}, // 420
                {amdgpu_gfx1250_op_V_EXP_F32,"V_EXP_F32"}, // 421
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 422
                {amdgpu_gfx1250_op_V_LOG_F32,"V_LOG_F32"}, // 423
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 424
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 425
                {amdgpu_gfx1250_op_V_RCP_F32,"V_RCP_F32"}, // 426
                {amdgpu_gfx1250_op_V_RCP_IFLAG_F32,"V_RCP_IFLAG_F32"}, // 427
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 428
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 429
                {amdgpu_gfx1250_op_V_RSQ_F32,"V_RSQ_F32"}, // 430
                {amdgpu_gfx1250_op_V_RCP_F64,"V_RCP_F64"}, // 431
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 432
                {amdgpu_gfx1250_op_V_RSQ_F64,"V_RSQ_F64"}, // 433
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 434
                {amdgpu_gfx1250_op_V_SQRT_F32,"V_SQRT_F32"}, // 435
                {amdgpu_gfx1250_op_V_SQRT_F64,"V_SQRT_F64"}, // 436
                {amdgpu_gfx1250_op_V_SIN_F32,"V_SIN_F32"}, // 437
                {amdgpu_gfx1250_op_V_COS_F32,"V_COS_F32"}, // 438
                {amdgpu_gfx1250_op_V_NOT_B32,"V_NOT_B32"}, // 439
                {amdgpu_gfx1250_op_V_BFREV_B32,"V_BFREV_B32"}, // 440
                {amdgpu_gfx1250_op_V_CLZ_I32_U32,"V_CLZ_I32_U32"}, // 441
                {amdgpu_gfx1250_op_V_CTZ_I32_B32,"V_CTZ_I32_B32"}, // 442
                {amdgpu_gfx1250_op_V_CLS_I32,"V_CLS_I32"}, // 443
                {amdgpu_gfx1250_op_V_FREXP_EXP_I32_F64,"V_FREXP_EXP_I32_F64"}, // 444
                {amdgpu_gfx1250_op_V_FREXP_MANT_F64,"V_FREXP_MANT_F64"}, // 445
                {amdgpu_gfx1250_op_V_FRACT_F64,"V_FRACT_F64"}, // 446
                {amdgpu_gfx1250_op_V_FREXP_EXP_I32_F32,"V_FREXP_EXP_I32_F32"}, // 447
                {amdgpu_gfx1250_op_V_FREXP_MANT_F32,"V_FREXP_MANT_F32"}, // 448
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 449
                {amdgpu_gfx1250_op_V_MOVRELD_B32,"V_MOVRELD_B32"}, // 450
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 451
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 452
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 453
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 454
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 455
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 456
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 457
                {amdgpu_gfx1250_op_V_TANH_BF16,"V_TANH_BF16"}, // 458
                {amdgpu_gfx1250_op_V_PRNG_B32,"V_PRNG_B32"}, // 459
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 460
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 461
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 462
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 463
                {amdgpu_gfx1250_op_V_CVT_F16_U16,"V_CVT_F16_U16"}, // 464
                {amdgpu_gfx1250_op_V_CVT_F16_I16,"V_CVT_F16_I16"}, // 465
                {amdgpu_gfx1250_op_V_CVT_U16_F16,"V_CVT_U16_F16"}, // 466
                {amdgpu_gfx1250_op_V_CVT_I16_F16,"V_CVT_I16_F16"}, // 467
                {amdgpu_gfx1250_op_V_RCP_F16,"V_RCP_F16"}, // 468
                {amdgpu_gfx1250_op_V_SQRT_F16,"V_SQRT_F16"}, // 469
                {amdgpu_gfx1250_op_V_RSQ_F16,"V_RSQ_F16"}, // 470
                {amdgpu_gfx1250_op_V_LOG_F16,"V_LOG_F16"}, // 471
                {amdgpu_gfx1250_op_V_EXP_F16,"V_EXP_F16"}, // 472
                {amdgpu_gfx1250_op_V_FREXP_MANT_F16,"V_FREXP_MANT_F16"}, // 473
                {amdgpu_gfx1250_op_V_FREXP_EXP_I16_F16,"V_FREXP_EXP_I16_F16"}, // 474
                {amdgpu_gfx1250_op_V_FLOOR_F16,"V_FLOOR_F16"}, // 475
                {amdgpu_gfx1250_op_V_CEIL_F16,"V_CEIL_F16"}, // 476
                {amdgpu_gfx1250_op_V_TRUNC_F16,"V_TRUNC_F16"}, // 477
                {amdgpu_gfx1250_op_V_RNDNE_F16,"V_RNDNE_F16"}, // 478
                {amdgpu_gfx1250_op_V_FRACT_F16,"V_FRACT_F16"}, // 479
                {amdgpu_gfx1250_op_V_SIN_F16,"V_SIN_F16"}, // 480
                {amdgpu_gfx1250_op_V_COS_F16,"V_COS_F16"}, // 481
                {amdgpu_gfx1250_op_V_SAT_PK_U8_I16,"V_SAT_PK_U8_I16"}, // 482
                {amdgpu_gfx1250_op_V_CVT_NORM_I16_F16,"V_CVT_NORM_I16_F16"}, // 483
                {amdgpu_gfx1250_op_V_CVT_NORM_U16_F16,"V_CVT_NORM_U16_F16"}, // 484
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 485
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 486
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 487
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 488
                {amdgpu_gfx1250_op_V_NOT_B16,"V_NOT_B16"}, // 489
                {amdgpu_gfx1250_op_V_CVT_I32_I16,"V_CVT_I32_I16"}, // 490
                {amdgpu_gfx1250_op_V_CVT_U32_U16,"V_CVT_U32_U16"}, // 491
                {amdgpu_gfx1250_op_V_CVT_F32_FP8,"V_CVT_F32_FP8"}, // 492
                {amdgpu_gfx1250_op_V_CVT_F32_BF8,"V_CVT_F32_BF8"}, // 493
                {amdgpu_gfx1250_op_V_CVT_PK_F32_FP8,"V_CVT_PK_F32_FP8"}, // 494
                {amdgpu_gfx1250_op_V_CVT_PK_F32_BF8,"V_CVT_PK_F32_BF8"}, // 495
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 496
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 497
                {amdgpu_gfx1250_op_V_CVT_F32_BF16,"V_CVT_F32_BF16"}, // 498
                {amdgpu_gfx1250_op_V_SAT_PK4_I4_I8,"V_SAT_PK4_I4_I8"}, // 499
                {amdgpu_gfx1250_op_V_SAT_PK4_U4_U8,"V_SAT_PK4_U4_U8"}, // 500
                {amdgpu_gfx1250_op_V_CVT_PK_F16_FP8,"V_CVT_PK_F16_FP8"}, // 501
                {amdgpu_gfx1250_op_V_CVT_PK_F16_BF8,"V_CVT_PK_F16_BF8"}, // 502
                {amdgpu_gfx1250_op_V_CVT_F16_FP8,"V_CVT_F16_FP8"}, // 503
                {amdgpu_gfx1250_op_V_CVT_F16_BF8,"V_CVT_F16_BF8"}, // 504
                {amdgpu_gfx1250_op_V_RCP_BF16,"V_RCP_BF16"}, // 505
                {amdgpu_gfx1250_op_V_SQRT_BF16,"V_SQRT_BF16"}, // 506
                {amdgpu_gfx1250_op_V_RSQ_BF16,"V_RSQ_BF16"}, // 507
                {amdgpu_gfx1250_op_V_LOG_BF16,"V_LOG_BF16"}, // 508
                {amdgpu_gfx1250_op_V_EXP_BF16,"V_EXP_BF16"}, // 509
                {amdgpu_gfx1250_op_V_SIN_BF16,"V_SIN_BF16"}, // 510
                {amdgpu_gfx1250_op_V_COS_BF16,"V_COS_BF16"}, // 511
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 512
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 513
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 514
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 515
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 516
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 517
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 518
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 519
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 520
                {amdgpu_gfx1250_op_V_FMA_DX9_ZERO_F32,"V_FMA_DX9_ZERO_F32"}, // 521
                {amdgpu_gfx1250_op_V_MAD_I32_I24,"V_MAD_I32_I24"}, // 522
                {amdgpu_gfx1250_op_V_MAD_U32_U24,"V_MAD_U32_U24"}, // 523
                {amdgpu_gfx1250_op_V_CUBEID_F32,"V_CUBEID_F32"}, // 524
                {amdgpu_gfx1250_op_V_CUBESC_F32,"V_CUBESC_F32"}, // 525
                {amdgpu_gfx1250_op_V_CUBETC_F32,"V_CUBETC_F32"}, // 526
                {amdgpu_gfx1250_op_V_CUBEMA_F32,"V_CUBEMA_F32"}, // 527
                {amdgpu_gfx1250_op_V_BFE_U32,"V_BFE_U32"}, // 528
                {amdgpu_gfx1250_op_V_BFE_I32,"V_BFE_I32"}, // 529
                {amdgpu_gfx1250_op_V_BFI_B32,"V_BFI_B32"}, // 530
                {amdgpu_gfx1250_op_V_FMA_F32,"V_FMA_F32"}, // 531
                {amdgpu_gfx1250_op_V_FMA_F64,"V_FMA_F64"}, // 532
                {amdgpu_gfx1250_op_V_LERP_U8,"V_LERP_U8"}, // 533
                {amdgpu_gfx1250_op_V_ALIGNBIT_B32,"V_ALIGNBIT_B32"}, // 534
                {amdgpu_gfx1250_op_V_ALIGNBYTE_B32,"V_ALIGNBYTE_B32"}, // 535
                {amdgpu_gfx1250_op_V_MULLIT_F32,"V_MULLIT_F32"}, // 536
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 537
                {amdgpu_gfx1250_op_V_MIN3_I32,"V_MIN3_I32"}, // 538
                {amdgpu_gfx1250_op_V_MIN3_U32,"V_MIN3_U32"}, // 539
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 540
                {amdgpu_gfx1250_op_V_MAX3_I32,"V_MAX3_I32"}, // 541
                {amdgpu_gfx1250_op_V_MAX3_U32,"V_MAX3_U32"}, // 542
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 543
                {amdgpu_gfx1250_op_V_MED3_I32,"V_MED3_I32"}, // 544
                {amdgpu_gfx1250_op_V_MED3_U32,"V_MED3_U32"}, // 545
                {amdgpu_gfx1250_op_V_SAD_U8,"V_SAD_U8"}, // 546
                {amdgpu_gfx1250_op_V_SAD_HI_U8,"V_SAD_HI_U8"}, // 547
                {amdgpu_gfx1250_op_V_SAD_U16,"V_SAD_U16"}, // 548
                {amdgpu_gfx1250_op_V_SAD_U32,"V_SAD_U32"}, // 549
                {amdgpu_gfx1250_op_V_CVT_PK_U8_F32,"V_CVT_PK_U8_F32"}, // 550
                {amdgpu_gfx1250_op_V_DIV_FIXUP_F32,"V_DIV_FIXUP_F32"}, // 551
                {amdgpu_gfx1250_op_V_DIV_FIXUP_F64,"V_DIV_FIXUP_F64"}, // 552
                {amdgpu_gfx1250_op_V_MIN3_NUM_F32,"V_MIN3_NUM_F32"}, // 553
                {amdgpu_gfx1250_op_V_MAX3_NUM_F32,"V_MAX3_NUM_F32"}, // 554
                {amdgpu_gfx1250_op_V_MIN3_NUM_F16,"V_MIN3_NUM_F16"}, // 555
                {amdgpu_gfx1250_op_V_MAX3_NUM_F16,"V_MAX3_NUM_F16"}, // 556
                {amdgpu_gfx1250_op_V_MINIMUM3_F32,"V_MINIMUM3_F32"}, // 557
                {amdgpu_gfx1250_op_V_MAXIMUM3_F32,"V_MAXIMUM3_F32"}, // 558
                {amdgpu_gfx1250_op_V_MINIMUM3_F16,"V_MINIMUM3_F16"}, // 559
                {amdgpu_gfx1250_op_V_MAXIMUM3_F16,"V_MAXIMUM3_F16"}, // 560
                {amdgpu_gfx1250_op_V_MED3_NUM_F32,"V_MED3_NUM_F32"}, // 561
                {amdgpu_gfx1250_op_V_MED3_NUM_F16,"V_MED3_NUM_F16"}, // 562
                {amdgpu_gfx1250_op_V_BITOP3_B16,"V_BITOP3_B16"}, // 563
                {amdgpu_gfx1250_op_V_BITOP3_B32,"V_BITOP3_B32"}, // 564
                {amdgpu_gfx1250_op_V_MAD_U32,"V_MAD_U32"}, // 565
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 566
                {amdgpu_gfx1250_op_V_DIV_FMAS_F32,"V_DIV_FMAS_F32"}, // 567
                {amdgpu_gfx1250_op_V_DIV_FMAS_F64,"V_DIV_FMAS_F64"}, // 568
                {amdgpu_gfx1250_op_V_MSAD_U8,"V_MSAD_U8"}, // 569
                {amdgpu_gfx1250_op_V_QSAD_PK_U16_U8,"V_QSAD_PK_U16_U8"}, // 570
                {amdgpu_gfx1250_op_V_MQSAD_PK_U16_U8,"V_MQSAD_PK_U16_U8"}, // 571
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 572
                {amdgpu_gfx1250_op_V_MQSAD_U32_U8,"V_MQSAD_U32_U8"}, // 573
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 574
                {amdgpu_gfx1250_op_V_PERM_PK16_B4_U4,"V_PERM_PK16_B4_U4"}, // 575
                {amdgpu_gfx1250_op_V_XOR3_B32,"V_XOR3_B32"}, // 576
                {amdgpu_gfx1250_op_V_MAD_U16,"V_MAD_U16"}, // 577
                {amdgpu_gfx1250_op_V_PERM_PK16_B6_U4,"V_PERM_PK16_B6_U4"}, // 578
                {amdgpu_gfx1250_op_V_PERM_PK16_B8_U4,"V_PERM_PK16_B8_U4"}, // 579
                {amdgpu_gfx1250_op_V_PERM_B32,"V_PERM_B32"}, // 580
                {amdgpu_gfx1250_op_V_XAD_U32,"V_XAD_U32"}, // 581
                {amdgpu_gfx1250_op_V_LSHL_ADD_U32,"V_LSHL_ADD_U32"}, // 582
                {amdgpu_gfx1250_op_V_ADD_LSHL_U32,"V_ADD_LSHL_U32"}, // 583
                {amdgpu_gfx1250_op_V_FMA_F16,"V_FMA_F16"}, // 584
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 585
                {amdgpu_gfx1250_op_V_MIN3_I16,"V_MIN3_I16"}, // 586
                {amdgpu_gfx1250_op_V_MIN3_U16,"V_MIN3_U16"}, // 587
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 588
                {amdgpu_gfx1250_op_V_MAX3_I16,"V_MAX3_I16"}, // 589
                {amdgpu_gfx1250_op_V_MAX3_U16,"V_MAX3_U16"}, // 590
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 591
                {amdgpu_gfx1250_op_V_MED3_I16,"V_MED3_I16"}, // 592
                {amdgpu_gfx1250_op_V_MED3_U16,"V_MED3_U16"}, // 593
                {amdgpu_gfx1250_op_V_LSHL_ADD_U64,"V_LSHL_ADD_U64"}, // 594
                {amdgpu_gfx1250_op_V_MAD_I16,"V_MAD_I16"}, // 595
                {amdgpu_gfx1250_op_V_DIV_FIXUP_F16,"V_DIV_FIXUP_F16"}, // 596
                {amdgpu_gfx1250_op_V_ADD3_U32,"V_ADD3_U32"}, // 597
                {amdgpu_gfx1250_op_V_LSHL_OR_B32,"V_LSHL_OR_B32"}, // 598
                {amdgpu_gfx1250_op_V_AND_OR_B32,"V_AND_OR_B32"}, // 599
                {amdgpu_gfx1250_op_V_OR3_B32,"V_OR3_B32"}, // 600
                {amdgpu_gfx1250_op_V_MAD_U32_U16,"V_MAD_U32_U16"}, // 601
                {amdgpu_gfx1250_op_V_MAD_I32_I16,"V_MAD_I32_I16"}, // 602
                {amdgpu_gfx1250_op_V_PERMLANE16_B32,"V_PERMLANE16_B32"}, // 603
                {amdgpu_gfx1250_op_V_PERMLANEX16_B32,"V_PERMLANEX16_B32"}, // 604
                {amdgpu_gfx1250_op_V_CNDMASK_B16,"V_CNDMASK_B16"}, // 605
                {amdgpu_gfx1250_op_V_ADD_MAX_I32,"V_ADD_MAX_I32"}, // 606
                {amdgpu_gfx1250_op_V_ADD_MAX_U32,"V_ADD_MAX_U32"}, // 607
                {amdgpu_gfx1250_op_V_ADD_MIN_I32,"V_ADD_MIN_I32"}, // 608
                {amdgpu_gfx1250_op_V_ADD_MIN_U32,"V_ADD_MIN_U32"}, // 609
                {amdgpu_gfx1250_op_V_MAXMIN_U32,"V_MAXMIN_U32"}, // 610
                {amdgpu_gfx1250_op_V_MINMAX_U32,"V_MINMAX_U32"}, // 611
                {amdgpu_gfx1250_op_V_MAXMIN_I32,"V_MAXMIN_I32"}, // 612
                {amdgpu_gfx1250_op_V_MINMAX_I32,"V_MINMAX_I32"}, // 613
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 614
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 615
                {amdgpu_gfx1250_op_V_MINMAX_NUM_F32,"V_MINMAX_NUM_F32"}, // 616
                {amdgpu_gfx1250_op_V_MAXMIN_NUM_F32,"V_MAXMIN_NUM_F32"}, // 617
                {amdgpu_gfx1250_op_V_MINMAX_NUM_F16,"V_MINMAX_NUM_F16"}, // 618
                {amdgpu_gfx1250_op_V_MAXMIN_NUM_F16,"V_MAXMIN_NUM_F16"}, // 619
                {amdgpu_gfx1250_op_V_MINIMUMMAXIMUM_F32,"V_MINIMUMMAXIMUM_F32"}, // 620
                {amdgpu_gfx1250_op_V_MAXIMUMMINIMUM_F32,"V_MAXIMUMMINIMUM_F32"}, // 621
                {amdgpu_gfx1250_op_V_MINIMUMMAXIMUM_F16,"V_MINIMUMMAXIMUM_F16"}, // 622
                {amdgpu_gfx1250_op_V_MAXIMUMMINIMUM_F16,"V_MAXIMUMMINIMUM_F16"}, // 623
                {amdgpu_gfx1250_op_V_PERMLANE_BCAST_B32,"V_PERMLANE_BCAST_B32"}, // 624
                {amdgpu_gfx1250_op_V_PERMLANE_UP_B32,"V_PERMLANE_UP_B32"}, // 625
                {amdgpu_gfx1250_op_V_PERMLANE_DOWN_B32,"V_PERMLANE_DOWN_B32"}, // 626
                {amdgpu_gfx1250_op_V_PERMLANE_XOR_B32,"V_PERMLANE_XOR_B32"}, // 627
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 628
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 629
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 630
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 631
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 632
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 633
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 634
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 635
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 636
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 637
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 638
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 639
                {amdgpu_gfx1250_op_V_S_EXP_F32,"V_S_EXP_F32"}, // 640
                {amdgpu_gfx1250_op_V_S_EXP_F16,"V_S_EXP_F16"}, // 641
                {amdgpu_gfx1250_op_V_S_LOG_F32,"V_S_LOG_F32"}, // 642
                {amdgpu_gfx1250_op_V_S_LOG_F16,"V_S_LOG_F16"}, // 643
                {amdgpu_gfx1250_op_V_S_RCP_F32,"V_S_RCP_F32"}, // 644
                {amdgpu_gfx1250_op_V_S_RCP_F16,"V_S_RCP_F16"}, // 645
                {amdgpu_gfx1250_op_V_S_RSQ_F32,"V_S_RSQ_F32"}, // 646
                {amdgpu_gfx1250_op_V_S_RSQ_F16,"V_S_RSQ_F16"}, // 647
                {amdgpu_gfx1250_op_V_S_SQRT_F32,"V_S_SQRT_F32"}, // 648
                {amdgpu_gfx1250_op_V_S_SQRT_F16,"V_S_SQRT_F16"}, // 649
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 650
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 651
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 652
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 653
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 654
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 655
                {amdgpu_gfx1250_op_V_ASHR_PK_I8_I32,"V_ASHR_PK_I8_I32"}, // 656
                {amdgpu_gfx1250_op_V_ASHR_PK_U8_I32,"V_ASHR_PK_U8_I32"}, // 657
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 658
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 659
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 660
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 661
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 662
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK8_FP4_F32,"V_CVT_SCALEF32_SR_PK8_FP4_F32"}, // 663
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK8_FP8_F32,"V_CVT_SCALEF32_SR_PK8_FP8_F32"}, // 664
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK8_BF8_F32,"V_CVT_SCALEF32_SR_PK8_BF8_F32"}, // 665
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 666
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 667
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 668
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 669
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 670
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK8_F16_FP4,"V_CVT_SCALE_PK8_F16_FP4"}, // 671
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK8_BF16_FP4,"V_CVT_SCALE_PK8_BF16_FP4"}, // 672
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK8_F32_FP4,"V_CVT_SCALE_PK8_F32_FP4"}, // 673
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 674
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 675
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 676
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 677
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 678
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 679
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK8_F16_FP8,"V_CVT_SCALE_PK8_F16_FP8"}, // 680
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK8_BF16_FP8,"V_CVT_SCALE_PK8_BF16_FP8"}, // 681
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK8_F32_FP8,"V_CVT_SCALE_PK8_F32_FP8"}, // 682
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK8_F16_BF8,"V_CVT_SCALE_PK8_F16_BF8"}, // 683
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK8_BF16_BF8,"V_CVT_SCALE_PK8_BF16_BF8"}, // 684
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK8_F32_BF8,"V_CVT_SCALE_PK8_F32_BF8"}, // 685
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 686
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 687
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK8_FP4_F32,"V_CVT_SCALEF32_PK8_FP4_F32"}, // 688
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 689
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 690
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK8_FP4_F16,"V_CVT_SCALEF32_PK8_FP4_F16"}, // 691
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK8_FP8_BF16,"V_CVT_SCALEF32_PK8_FP8_BF16"}, // 692
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK8_BF8_BF16,"V_CVT_SCALEF32_PK8_BF8_BF16"}, // 693
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 694
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 695
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK8_FP4_BF16,"V_CVT_SCALEF32_PK8_FP4_BF16"}, // 696
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK8_FP4_F16,"V_CVT_SCALEF32_SR_PK8_FP4_F16"}, // 697
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 698
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 699
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK8_FP4_BF16,"V_CVT_SCALEF32_SR_PK8_FP4_BF16"}, // 700
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 701
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 702
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK8_FP8_F16,"V_CVT_SCALEF32_SR_PK8_FP8_F16"}, // 703
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK8_FP8_BF16,"V_CVT_SCALEF32_SR_PK8_FP8_BF16"}, // 704
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK8_BF8_F16,"V_CVT_SCALEF32_SR_PK8_BF8_F16"}, // 705
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK8_BF8_BF16,"V_CVT_SCALEF32_SR_PK8_BF8_BF16"}, // 706
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK8_FP8_F32,"V_CVT_SCALEF32_PK8_FP8_F32"}, // 707
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK8_FP8_F16,"V_CVT_SCALEF32_PK8_FP8_F16"}, // 708
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK8_BF8_F32,"V_CVT_SCALEF32_PK8_BF8_F32"}, // 709
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK8_BF8_F16,"V_CVT_SCALEF32_PK8_BF8_F16"}, // 710
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK16_F16_FP6,"V_CVT_SCALE_PK16_F16_FP6"}, // 711
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK16_BF16_FP6,"V_CVT_SCALE_PK16_BF16_FP6"}, // 712
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK16_F32_FP6,"V_CVT_SCALE_PK16_F32_FP6"}, // 713
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK16_F16_BF6,"V_CVT_SCALE_PK16_F16_BF6"}, // 714
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK16_BF16_BF6,"V_CVT_SCALE_PK16_BF16_BF6"}, // 715
                {amdgpu_gfx1250_op_V_CVT_SCALE_PK16_F32_BF6,"V_CVT_SCALE_PK16_F32_BF6"}, // 716
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK16_FP6_F32,"V_CVT_SCALEF32_PK16_FP6_F32"}, // 717
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK16_BF6_F32,"V_CVT_SCALEF32_PK16_BF6_F32"}, // 718
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK16_FP6_F16,"V_CVT_SCALEF32_PK16_FP6_F16"}, // 719
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK16_BF6_F16,"V_CVT_SCALEF32_PK16_BF6_F16"}, // 720
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK16_FP6_BF16,"V_CVT_SCALEF32_PK16_FP6_BF16"}, // 721
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_PK16_BF6_BF16,"V_CVT_SCALEF32_PK16_BF6_BF16"}, // 722
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK16_FP6_F32,"V_CVT_SCALEF32_SR_PK16_FP6_F32"}, // 723
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK16_BF6_F32,"V_CVT_SCALEF32_SR_PK16_BF6_F32"}, // 724
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK16_FP6_F16,"V_CVT_SCALEF32_SR_PK16_FP6_F16"}, // 725
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK16_BF6_F16,"V_CVT_SCALEF32_SR_PK16_BF6_F16"}, // 726
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK16_FP6_BF16,"V_CVT_SCALEF32_SR_PK16_FP6_BF16"}, // 727
                {amdgpu_gfx1250_op_V_CVT_SCALEF32_SR_PK16_BF6_BF16,"V_CVT_SCALEF32_SR_PK16_BF6_BF16"}, // 728
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 729
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 730
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 731
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 732
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 733
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 734
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 735
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 736
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 737
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 738
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 739
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 740
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 741
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 742
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 743
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 744
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 745
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 746
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 747
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 748
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 749
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 750
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 751
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 752
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 753
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 754
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 755
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 756
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 757
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 758
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 759
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 760
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 761
                {amdgpu_gfx1250_op_V_MAD_NC_U64_U32,"V_MAD_NC_U64_U32"}, // 762
                {amdgpu_gfx1250_op_V_MAD_NC_I64_I32,"V_MAD_NC_I64_I32"}, // 763
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 764
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 765
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 766
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 767
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 768
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 769
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 770
                {amdgpu_gfx1250_op_V_ADD_NC_U16,"V_ADD_NC_U16"}, // 771
                {amdgpu_gfx1250_op_V_SUB_NC_U16,"V_SUB_NC_U16"}, // 772
                {amdgpu_gfx1250_op_V_MUL_LO_U16,"V_MUL_LO_U16"}, // 773
                {amdgpu_gfx1250_op_V_CVT_PK_I16_F32,"V_CVT_PK_I16_F32"}, // 774
                {amdgpu_gfx1250_op_V_CVT_PK_U16_F32,"V_CVT_PK_U16_F32"}, // 775
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 776
                {amdgpu_gfx1250_op_V_MAX_U16,"V_MAX_U16"}, // 777
                {amdgpu_gfx1250_op_V_MAX_I16,"V_MAX_I16"}, // 778
                {amdgpu_gfx1250_op_V_MIN_U16,"V_MIN_U16"}, // 779
                {amdgpu_gfx1250_op_V_MIN_I16,"V_MIN_I16"}, // 780
                {amdgpu_gfx1250_op_V_ADD_NC_I16,"V_ADD_NC_I16"}, // 781
                {amdgpu_gfx1250_op_V_SUB_NC_I16,"V_SUB_NC_I16"}, // 782
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 783
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 784
                {amdgpu_gfx1250_op_V_PACK_B32_F16,"V_PACK_B32_F16"}, // 785
                {amdgpu_gfx1250_op_V_CVT_PK_NORM_I16_F16,"V_CVT_PK_NORM_I16_F16"}, // 786
                {amdgpu_gfx1250_op_V_CVT_PK_NORM_U16_F16,"V_CVT_PK_NORM_U16_F16"}, // 787
                {amdgpu_gfx1250_op_V_PERMLANE_IDX_GEN_B32,"V_PERMLANE_IDX_GEN_B32"}, // 788
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 789
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 790
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 791
                {amdgpu_gfx1250_op_V_MIN_U64,"V_MIN_U64"}, // 792
                {amdgpu_gfx1250_op_V_MAX_U64,"V_MAX_U64"}, // 793
                {amdgpu_gfx1250_op_V_MIN_I64,"V_MIN_I64"}, // 794
                {amdgpu_gfx1250_op_V_MAX_I64,"V_MAX_I64"}, // 795
                {amdgpu_gfx1250_op_V_LDEXP_F32,"V_LDEXP_F32"}, // 796
                {amdgpu_gfx1250_op_V_BFM_B32,"V_BFM_B32"}, // 797
                {amdgpu_gfx1250_op_V_BCNT_U32_B32,"V_BCNT_U32_B32"}, // 798
                {amdgpu_gfx1250_op_V_MBCNT_LO_U32_B32,"V_MBCNT_LO_U32_B32"}, // 799
                {amdgpu_gfx1250_op_V_MBCNT_HI_U32_B32,"V_MBCNT_HI_U32_B32"}, // 800
                {amdgpu_gfx1250_op_V_CVT_PK_NORM_I16_F32,"V_CVT_PK_NORM_I16_F32"}, // 801
                {amdgpu_gfx1250_op_V_CVT_PK_NORM_U16_F32,"V_CVT_PK_NORM_U16_F32"}, // 802
                {amdgpu_gfx1250_op_V_CVT_PK_U16_U32,"V_CVT_PK_U16_U32"}, // 803
                {amdgpu_gfx1250_op_V_CVT_PK_I16_I32,"V_CVT_PK_I16_I32"}, // 804
                {amdgpu_gfx1250_op_V_SUB_NC_I32,"V_SUB_NC_I32"}, // 805
                {amdgpu_gfx1250_op_V_ADD_NC_I32,"V_ADD_NC_I32"}, // 806
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 807
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 808
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 809
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 810
                {amdgpu_gfx1250_op_V_LDEXP_F64,"V_LDEXP_F64"}, // 811
                {amdgpu_gfx1250_op_V_MUL_LO_U32,"V_MUL_LO_U32"}, // 812
                {amdgpu_gfx1250_op_V_MUL_HI_U32,"V_MUL_HI_U32"}, // 813
                {amdgpu_gfx1250_op_V_MUL_HI_I32,"V_MUL_HI_I32"}, // 814
                {amdgpu_gfx1250_op_V_TRIG_PREOP_F64,"V_TRIG_PREOP_F64"}, // 815
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 816
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 817
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 818
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 819
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 820
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 821
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 822
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 823
                {amdgpu_gfx1250_op_V_LSHLREV_B16,"V_LSHLREV_B16"}, // 824
                {amdgpu_gfx1250_op_V_LSHRREV_B16,"V_LSHRREV_B16"}, // 825
                {amdgpu_gfx1250_op_V_ASHRREV_I16,"V_ASHRREV_I16"}, // 826
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 827
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 828
                {amdgpu_gfx1250_op_V_LSHRREV_B64,"V_LSHRREV_B64"}, // 829
                {amdgpu_gfx1250_op_V_ASHRREV_I64,"V_ASHRREV_I64"}, // 830
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 831
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 832
                {amdgpu_gfx1250_op_V_MINIMUM_F64,"V_MINIMUM_F64"}, // 833
                {amdgpu_gfx1250_op_V_MAXIMUM_F64,"V_MAXIMUM_F64"}, // 834
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 835
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 836
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 837
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 838
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 839
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 840
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 841
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 842
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 843
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 844
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 845
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 846
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 847
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 848
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 849
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 850
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 851
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 852
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 853
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 854
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 855
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 856
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 857
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 858
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 859
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 860
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 861
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 862
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 863
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 864
                {amdgpu_gfx1250_op_V_WRITELANE_B32,"V_WRITELANE_B32"}, // 865
                {amdgpu_gfx1250_op_V_AND_B16,"V_AND_B16"}, // 866
                {amdgpu_gfx1250_op_V_OR_B16,"V_OR_B16"}, // 867
                {amdgpu_gfx1250_op_V_XOR_B16,"V_XOR_B16"}, // 868
                {amdgpu_gfx1250_op_V_MINIMUM_F32,"V_MINIMUM_F32"}, // 869
                {amdgpu_gfx1250_op_V_MAXIMUM_F32,"V_MAXIMUM_F32"}, // 870
                {amdgpu_gfx1250_op_V_MINIMUM_F16,"V_MINIMUM_F16"}, // 871
                {amdgpu_gfx1250_op_V_MAXIMUM_F16,"V_MAXIMUM_F16"}, // 872
                {amdgpu_gfx1250_op_V_CVT_PK_FP8_F32,"V_CVT_PK_FP8_F32"}, // 873
                {amdgpu_gfx1250_op_V_CVT_PK_BF8_F32,"V_CVT_PK_BF8_F32"}, // 874
                {amdgpu_gfx1250_op_V_CVT_SR_FP8_F32,"V_CVT_SR_FP8_F32"}, // 875
                {amdgpu_gfx1250_op_V_CVT_SR_BF8_F32,"V_CVT_SR_BF8_F32"}, // 876
                {amdgpu_gfx1250_op_V_CVT_PK_BF16_F32,"V_CVT_PK_BF16_F32"}, // 877
                {amdgpu_gfx1250_op_V_CVT_SR_PK_BF16_F32,"V_CVT_SR_PK_BF16_F32"}, // 878
                {amdgpu_gfx1250_op_V_CVT_PK_F16_F32,"V_CVT_PK_F16_F32"}, // 879
                {amdgpu_gfx1250_op_V_CVT_SR_PK_F16_F32,"V_CVT_SR_PK_F16_F32"}, // 880
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 881
                {amdgpu_gfx1250_op_V_CVT_PK_FP8_F16,"V_CVT_PK_FP8_F16"}, // 882
                {amdgpu_gfx1250_op_V_CVT_PK_BF8_F16,"V_CVT_PK_BF8_F16"}, // 883
                {amdgpu_gfx1250_op_V_CVT_SR_FP8_F16,"V_CVT_SR_FP8_F16"}, // 884
                {amdgpu_gfx1250_op_V_CVT_SR_BF8_F16,"V_CVT_SR_BF8_F16"}, // 885
            }; // end VOP3_INST_LITERAL_insn_table
            const amdgpu_gfx1250_insn_entry VOP3_VOP_DPP8_insn_table[886] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_V_CMP_LT_F16,"V_CMP_LT_F16"}, // 1
                {amdgpu_gfx1250_op_V_CMP_EQ_F16,"V_CMP_EQ_F16"}, // 2
                {amdgpu_gfx1250_op_V_CMP_LE_F16,"V_CMP_LE_F16"}, // 3
                {amdgpu_gfx1250_op_V_CMP_GT_F16,"V_CMP_GT_F16"}, // 4
                {amdgpu_gfx1250_op_V_CMP_LG_F16,"V_CMP_LG_F16"}, // 5
                {amdgpu_gfx1250_op_V_CMP_GE_F16,"V_CMP_GE_F16"}, // 6
                {amdgpu_gfx1250_op_V_CMP_O_F16,"V_CMP_O_F16"}, // 7
                {amdgpu_gfx1250_op_V_CMP_U_F16,"V_CMP_U_F16"}, // 8
                {amdgpu_gfx1250_op_V_CMP_NGE_F16,"V_CMP_NGE_F16"}, // 9
                {amdgpu_gfx1250_op_V_CMP_NLG_F16,"V_CMP_NLG_F16"}, // 10
                {amdgpu_gfx1250_op_V_CMP_NGT_F16,"V_CMP_NGT_F16"}, // 11
                {amdgpu_gfx1250_op_V_CMP_NLE_F16,"V_CMP_NLE_F16"}, // 12
                {amdgpu_gfx1250_op_V_CMP_NEQ_F16,"V_CMP_NEQ_F16"}, // 13
                {amdgpu_gfx1250_op_V_CMP_NLT_F16,"V_CMP_NLT_F16"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_V_CMP_LT_F32,"V_CMP_LT_F32"}, // 17
                {amdgpu_gfx1250_op_V_CMP_EQ_F32,"V_CMP_EQ_F32"}, // 18
                {amdgpu_gfx1250_op_V_CMP_LE_F32,"V_CMP_LE_F32"}, // 19
                {amdgpu_gfx1250_op_V_CMP_GT_F32,"V_CMP_GT_F32"}, // 20
                {amdgpu_gfx1250_op_V_CMP_LG_F32,"V_CMP_LG_F32"}, // 21
                {amdgpu_gfx1250_op_V_CMP_GE_F32,"V_CMP_GE_F32"}, // 22
                {amdgpu_gfx1250_op_V_CMP_O_F32,"V_CMP_O_F32"}, // 23
                {amdgpu_gfx1250_op_V_CMP_U_F32,"V_CMP_U_F32"}, // 24
                {amdgpu_gfx1250_op_V_CMP_NGE_F32,"V_CMP_NGE_F32"}, // 25
                {amdgpu_gfx1250_op_V_CMP_NLG_F32,"V_CMP_NLG_F32"}, // 26
                {amdgpu_gfx1250_op_V_CMP_NGT_F32,"V_CMP_NGT_F32"}, // 27
                {amdgpu_gfx1250_op_V_CMP_NLE_F32,"V_CMP_NLE_F32"}, // 28
                {amdgpu_gfx1250_op_V_CMP_NEQ_F32,"V_CMP_NEQ_F32"}, // 29
                {amdgpu_gfx1250_op_V_CMP_NLT_F32,"V_CMP_NLT_F32"}, // 30
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 31
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 32
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 33
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 34
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 35
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 36
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 42
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_V_CMP_LT_I16,"V_CMP_LT_I16"}, // 49
                {amdgpu_gfx1250_op_V_CMP_EQ_I16,"V_CMP_EQ_I16"}, // 50
                {amdgpu_gfx1250_op_V_CMP_LE_I16,"V_CMP_LE_I16"}, // 51
                {amdgpu_gfx1250_op_V_CMP_GT_I16,"V_CMP_GT_I16"}, // 52
                {amdgpu_gfx1250_op_V_CMP_NE_I16,"V_CMP_NE_I16"}, // 53
                {amdgpu_gfx1250_op_V_CMP_GE_I16,"V_CMP_GE_I16"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_V_CMP_LT_U16,"V_CMP_LT_U16"}, // 57
                {amdgpu_gfx1250_op_V_CMP_EQ_U16,"V_CMP_EQ_U16"}, // 58
                {amdgpu_gfx1250_op_V_CMP_LE_U16,"V_CMP_LE_U16"}, // 59
                {amdgpu_gfx1250_op_V_CMP_GT_U16,"V_CMP_GT_U16"}, // 60
                {amdgpu_gfx1250_op_V_CMP_NE_U16,"V_CMP_NE_U16"}, // 61
                {amdgpu_gfx1250_op_V_CMP_GE_U16,"V_CMP_GE_U16"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 64
                {amdgpu_gfx1250_op_V_CMP_LT_I32,"V_CMP_LT_I32"}, // 65
                {amdgpu_gfx1250_op_V_CMP_EQ_I32,"V_CMP_EQ_I32"}, // 66
                {amdgpu_gfx1250_op_V_CMP_LE_I32,"V_CMP_LE_I32"}, // 67
                {amdgpu_gfx1250_op_V_CMP_GT_I32,"V_CMP_GT_I32"}, // 68
                {amdgpu_gfx1250_op_V_CMP_NE_I32,"V_CMP_NE_I32"}, // 69
                {amdgpu_gfx1250_op_V_CMP_GE_I32,"V_CMP_GE_I32"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 72
                {amdgpu_gfx1250_op_V_CMP_LT_U32,"V_CMP_LT_U32"}, // 73
                {amdgpu_gfx1250_op_V_CMP_EQ_U32,"V_CMP_EQ_U32"}, // 74
                {amdgpu_gfx1250_op_V_CMP_LE_U32,"V_CMP_LE_U32"}, // 75
                {amdgpu_gfx1250_op_V_CMP_GT_U32,"V_CMP_GT_U32"}, // 76
                {amdgpu_gfx1250_op_V_CMP_NE_U32,"V_CMP_NE_U32"}, // 77
                {amdgpu_gfx1250_op_V_CMP_GE_U32,"V_CMP_GE_U32"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 80
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 81
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 82
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 83
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 84
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 85
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 86
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 87
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 88
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 89
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 90
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 91
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 92
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 93
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 94
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 95
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 96
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 97
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 98
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 99
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 100
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 101
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 102
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 103
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 104
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 105
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 106
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 107
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 108
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 109
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 110
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 111
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 112
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 113
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 114
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 115
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 116
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 117
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 118
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 119
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 120
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 121
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 122
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 123
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 124
                {amdgpu_gfx1250_op_V_CMP_CLASS_F16,"V_CMP_CLASS_F16"}, // 125
                {amdgpu_gfx1250_op_V_CMP_CLASS_F32,"V_CMP_CLASS_F32"}, // 126
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 127
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 128
                {amdgpu_gfx1250_op_V_CMPX_LT_F16,"V_CMPX_LT_F16"}, // 129
                {amdgpu_gfx1250_op_V_CMPX_EQ_F16,"V_CMPX_EQ_F16"}, // 130
                {amdgpu_gfx1250_op_V_CMPX_LE_F16,"V_CMPX_LE_F16"}, // 131
                {amdgpu_gfx1250_op_V_CMPX_GT_F16,"V_CMPX_GT_F16"}, // 132
                {amdgpu_gfx1250_op_V_CMPX_LG_F16,"V_CMPX_LG_F16"}, // 133
                {amdgpu_gfx1250_op_V_CMPX_GE_F16,"V_CMPX_GE_F16"}, // 134
                {amdgpu_gfx1250_op_V_CMPX_O_F16,"V_CMPX_O_F16"}, // 135
                {amdgpu_gfx1250_op_V_CMPX_U_F16,"V_CMPX_U_F16"}, // 136
                {amdgpu_gfx1250_op_V_CMPX_NGE_F16,"V_CMPX_NGE_F16"}, // 137
                {amdgpu_gfx1250_op_V_CMPX_NLG_F16,"V_CMPX_NLG_F16"}, // 138
                {amdgpu_gfx1250_op_V_CMPX_NGT_F16,"V_CMPX_NGT_F16"}, // 139
                {amdgpu_gfx1250_op_V_CMPX_NLE_F16,"V_CMPX_NLE_F16"}, // 140
                {amdgpu_gfx1250_op_V_CMPX_NEQ_F16,"V_CMPX_NEQ_F16"}, // 141
                {amdgpu_gfx1250_op_V_CMPX_NLT_F16,"V_CMPX_NLT_F16"}, // 142
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 143
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 144
                {amdgpu_gfx1250_op_V_CMPX_LT_F32,"V_CMPX_LT_F32"}, // 145
                {amdgpu_gfx1250_op_V_CMPX_EQ_F32,"V_CMPX_EQ_F32"}, // 146
                {amdgpu_gfx1250_op_V_CMPX_LE_F32,"V_CMPX_LE_F32"}, // 147
                {amdgpu_gfx1250_op_V_CMPX_GT_F32,"V_CMPX_GT_F32"}, // 148
                {amdgpu_gfx1250_op_V_CMPX_LG_F32,"V_CMPX_LG_F32"}, // 149
                {amdgpu_gfx1250_op_V_CMPX_GE_F32,"V_CMPX_GE_F32"}, // 150
                {amdgpu_gfx1250_op_V_CMPX_O_F32,"V_CMPX_O_F32"}, // 151
                {amdgpu_gfx1250_op_V_CMPX_U_F32,"V_CMPX_U_F32"}, // 152
                {amdgpu_gfx1250_op_V_CMPX_NGE_F32,"V_CMPX_NGE_F32"}, // 153
                {amdgpu_gfx1250_op_V_CMPX_NLG_F32,"V_CMPX_NLG_F32"}, // 154
                {amdgpu_gfx1250_op_V_CMPX_NGT_F32,"V_CMPX_NGT_F32"}, // 155
                {amdgpu_gfx1250_op_V_CMPX_NLE_F32,"V_CMPX_NLE_F32"}, // 156
                {amdgpu_gfx1250_op_V_CMPX_NEQ_F32,"V_CMPX_NEQ_F32"}, // 157
                {amdgpu_gfx1250_op_V_CMPX_NLT_F32,"V_CMPX_NLT_F32"}, // 158
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 159
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 160
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 161
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 162
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 163
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 164
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 165
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 166
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 167
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 168
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 169
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 170
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 171
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 172
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 173
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 174
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 175
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 176
                {amdgpu_gfx1250_op_V_CMPX_LT_I16,"V_CMPX_LT_I16"}, // 177
                {amdgpu_gfx1250_op_V_CMPX_EQ_I16,"V_CMPX_EQ_I16"}, // 178
                {amdgpu_gfx1250_op_V_CMPX_LE_I16,"V_CMPX_LE_I16"}, // 179
                {amdgpu_gfx1250_op_V_CMPX_GT_I16,"V_CMPX_GT_I16"}, // 180
                {amdgpu_gfx1250_op_V_CMPX_NE_I16,"V_CMPX_NE_I16"}, // 181
                {amdgpu_gfx1250_op_V_CMPX_GE_I16,"V_CMPX_GE_I16"}, // 182
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 183
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 184
                {amdgpu_gfx1250_op_V_CMPX_LT_U16,"V_CMPX_LT_U16"}, // 185
                {amdgpu_gfx1250_op_V_CMPX_EQ_U16,"V_CMPX_EQ_U16"}, // 186
                {amdgpu_gfx1250_op_V_CMPX_LE_U16,"V_CMPX_LE_U16"}, // 187
                {amdgpu_gfx1250_op_V_CMPX_GT_U16,"V_CMPX_GT_U16"}, // 188
                {amdgpu_gfx1250_op_V_CMPX_NE_U16,"V_CMPX_NE_U16"}, // 189
                {amdgpu_gfx1250_op_V_CMPX_GE_U16,"V_CMPX_GE_U16"}, // 190
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 191
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 192
                {amdgpu_gfx1250_op_V_CMPX_LT_I32,"V_CMPX_LT_I32"}, // 193
                {amdgpu_gfx1250_op_V_CMPX_EQ_I32,"V_CMPX_EQ_I32"}, // 194
                {amdgpu_gfx1250_op_V_CMPX_LE_I32,"V_CMPX_LE_I32"}, // 195
                {amdgpu_gfx1250_op_V_CMPX_GT_I32,"V_CMPX_GT_I32"}, // 196
                {amdgpu_gfx1250_op_V_CMPX_NE_I32,"V_CMPX_NE_I32"}, // 197
                {amdgpu_gfx1250_op_V_CMPX_GE_I32,"V_CMPX_GE_I32"}, // 198
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 199
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 200
                {amdgpu_gfx1250_op_V_CMPX_LT_U32,"V_CMPX_LT_U32"}, // 201
                {amdgpu_gfx1250_op_V_CMPX_EQ_U32,"V_CMPX_EQ_U32"}, // 202
                {amdgpu_gfx1250_op_V_CMPX_LE_U32,"V_CMPX_LE_U32"}, // 203
                {amdgpu_gfx1250_op_V_CMPX_GT_U32,"V_CMPX_GT_U32"}, // 204
                {amdgpu_gfx1250_op_V_CMPX_NE_U32,"V_CMPX_NE_U32"}, // 205
                {amdgpu_gfx1250_op_V_CMPX_GE_U32,"V_CMPX_GE_U32"}, // 206
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 207
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 208
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 209
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 210
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 211
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 212
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 213
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 214
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 215
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 216
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 217
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 218
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 219
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 220
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 221
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 222
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 223
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 224
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 225
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 226
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 227
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 228
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 229
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 230
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 231
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 232
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 233
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 234
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 235
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 236
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 237
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 238
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 239
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 240
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 241
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 242
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 243
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 244
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 245
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 246
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 247
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 248
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 249
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 250
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 251
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 252
                {amdgpu_gfx1250_op_V_CMPX_CLASS_F16,"V_CMPX_CLASS_F16"}, // 253
                {amdgpu_gfx1250_op_V_CMPX_CLASS_F32,"V_CMPX_CLASS_F32"}, // 254
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 255
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 256
                {amdgpu_gfx1250_op_V_CNDMASK_B32,"V_CNDMASK_B32"}, // 257
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 258
                {amdgpu_gfx1250_op_V_ADD_F32,"V_ADD_F32"}, // 259
                {amdgpu_gfx1250_op_V_SUB_F32,"V_SUB_F32"}, // 260
                {amdgpu_gfx1250_op_V_SUBREV_F32,"V_SUBREV_F32"}, // 261
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 262
                {amdgpu_gfx1250_op_V_MUL_DX9_ZERO_F32,"V_MUL_DX9_ZERO_F32"}, // 263
                {amdgpu_gfx1250_op_V_MUL_F32,"V_MUL_F32"}, // 264
                {amdgpu_gfx1250_op_V_MUL_I32_I24,"V_MUL_I32_I24"}, // 265
                {amdgpu_gfx1250_op_V_MUL_HI_I32_I24,"V_MUL_HI_I32_I24"}, // 266
                {amdgpu_gfx1250_op_V_MUL_U32_U24,"V_MUL_U32_U24"}, // 267
                {amdgpu_gfx1250_op_V_MUL_HI_U32_U24,"V_MUL_HI_U32_U24"}, // 268
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 269
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 270
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 271
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 272
                {amdgpu_gfx1250_op_V_MIN_I32,"V_MIN_I32"}, // 273
                {amdgpu_gfx1250_op_V_MAX_I32,"V_MAX_I32"}, // 274
                {amdgpu_gfx1250_op_V_MIN_U32,"V_MIN_U32"}, // 275
                {amdgpu_gfx1250_op_V_MAX_U32,"V_MAX_U32"}, // 276
                {amdgpu_gfx1250_op_V_MIN_NUM_F32,"V_MIN_NUM_F32"}, // 277
                {amdgpu_gfx1250_op_V_MAX_NUM_F32,"V_MAX_NUM_F32"}, // 278
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 279
                {amdgpu_gfx1250_op_V_LSHLREV_B32,"V_LSHLREV_B32"}, // 280
                {amdgpu_gfx1250_op_V_LSHRREV_B32,"V_LSHRREV_B32"}, // 281
                {amdgpu_gfx1250_op_V_ASHRREV_I32,"V_ASHRREV_I32"}, // 282
                {amdgpu_gfx1250_op_V_AND_B32,"V_AND_B32"}, // 283
                {amdgpu_gfx1250_op_V_OR_B32,"V_OR_B32"}, // 284
                {amdgpu_gfx1250_op_V_XOR_B32,"V_XOR_B32"}, // 285
                {amdgpu_gfx1250_op_V_XNOR_B32,"V_XNOR_B32"}, // 286
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 287
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 288
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 289
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 290
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 291
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 292
                {amdgpu_gfx1250_op_V_ADD_NC_U32,"V_ADD_NC_U32"}, // 293
                {amdgpu_gfx1250_op_V_SUB_NC_U32,"V_SUB_NC_U32"}, // 294
                {amdgpu_gfx1250_op_V_SUBREV_NC_U32,"V_SUBREV_NC_U32"}, // 295
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 296
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 297
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 298
                {amdgpu_gfx1250_op_V_FMAC_F32,"V_FMAC_F32"}, // 299
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 300
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 301
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 302
                {amdgpu_gfx1250_op_V_CVT_PK_RTZ_F16_F32,"V_CVT_PK_RTZ_F16_F32"}, // 303
                {amdgpu_gfx1250_op_V_MIN_NUM_F16,"V_MIN_NUM_F16"}, // 304
                {amdgpu_gfx1250_op_V_MAX_NUM_F16,"V_MAX_NUM_F16"}, // 305
                {amdgpu_gfx1250_op_V_ADD_F16,"V_ADD_F16"}, // 306
                {amdgpu_gfx1250_op_V_SUB_F16,"V_SUB_F16"}, // 307
                {amdgpu_gfx1250_op_V_SUBREV_F16,"V_SUBREV_F16"}, // 308
                {amdgpu_gfx1250_op_V_MUL_F16,"V_MUL_F16"}, // 309
                {amdgpu_gfx1250_op_V_FMAC_F16,"V_FMAC_F16"}, // 310
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 311
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 312
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 313
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 314
                {amdgpu_gfx1250_op_V_LDEXP_F16,"V_LDEXP_F16"}, // 315
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 316
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 317
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 318
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 319
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 320
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 321
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 322
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 323
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 324
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 325
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 326
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 327
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 328
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 329
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 330
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 331
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 332
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 333
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 334
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 335
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 336
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 337
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 338
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 339
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 340
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 341
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 342
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 343
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 344
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 345
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 346
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 347
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 348
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 349
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 350
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 351
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 352
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 353
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 354
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 355
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 356
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 357
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 358
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 359
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 360
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 361
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 362
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 363
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 364
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 365
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 366
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 367
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 368
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 369
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 370
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 371
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 372
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 373
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 374
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 375
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 376
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 377
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 378
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 379
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 380
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 381
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 382
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 383
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 384
                {amdgpu_gfx1250_op_V_MOV_B32,"V_MOV_B32"}, // 385
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 386
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 387
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 388
                {amdgpu_gfx1250_op_V_CVT_F32_I32,"V_CVT_F32_I32"}, // 389
                {amdgpu_gfx1250_op_V_CVT_F32_U32,"V_CVT_F32_U32"}, // 390
                {amdgpu_gfx1250_op_V_CVT_U32_F32,"V_CVT_U32_F32"}, // 391
                {amdgpu_gfx1250_op_V_CVT_I32_F32,"V_CVT_I32_F32"}, // 392
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 393
                {amdgpu_gfx1250_op_V_CVT_F16_F32,"V_CVT_F16_F32"}, // 394
                {amdgpu_gfx1250_op_V_CVT_F32_F16,"V_CVT_F32_F16"}, // 395
                {amdgpu_gfx1250_op_V_CVT_NEAREST_I32_F32,"V_CVT_NEAREST_I32_F32"}, // 396
                {amdgpu_gfx1250_op_V_CVT_FLOOR_I32_F32,"V_CVT_FLOOR_I32_F32"}, // 397
                {amdgpu_gfx1250_op_V_CVT_OFF_F32_I4,"V_CVT_OFF_F32_I4"}, // 398
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 399
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 400
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE0,"V_CVT_F32_UBYTE0"}, // 401
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE1,"V_CVT_F32_UBYTE1"}, // 402
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE2,"V_CVT_F32_UBYTE2"}, // 403
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE3,"V_CVT_F32_UBYTE3"}, // 404
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 405
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 406
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 407
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 408
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 409
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 410
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 411
                {amdgpu_gfx1250_op_V_MOV_B16,"V_MOV_B16"}, // 412
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 413
                {amdgpu_gfx1250_op_V_TANH_F32,"V_TANH_F32"}, // 414
                {amdgpu_gfx1250_op_V_TANH_F16,"V_TANH_F16"}, // 415
                {amdgpu_gfx1250_op_V_FRACT_F32,"V_FRACT_F32"}, // 416
                {amdgpu_gfx1250_op_V_TRUNC_F32,"V_TRUNC_F32"}, // 417
                {amdgpu_gfx1250_op_V_CEIL_F32,"V_CEIL_F32"}, // 418
                {amdgpu_gfx1250_op_V_RNDNE_F32,"V_RNDNE_F32"}, // 419
                {amdgpu_gfx1250_op_V_FLOOR_F32,"V_FLOOR_F32"}, // 420
                {amdgpu_gfx1250_op_V_EXP_F32,"V_EXP_F32"}, // 421
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 422
                {amdgpu_gfx1250_op_V_LOG_F32,"V_LOG_F32"}, // 423
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 424
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 425
                {amdgpu_gfx1250_op_V_RCP_F32,"V_RCP_F32"}, // 426
                {amdgpu_gfx1250_op_V_RCP_IFLAG_F32,"V_RCP_IFLAG_F32"}, // 427
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 428
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 429
                {amdgpu_gfx1250_op_V_RSQ_F32,"V_RSQ_F32"}, // 430
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 431
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 432
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 433
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 434
                {amdgpu_gfx1250_op_V_SQRT_F32,"V_SQRT_F32"}, // 435
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 436
                {amdgpu_gfx1250_op_V_SIN_F32,"V_SIN_F32"}, // 437
                {amdgpu_gfx1250_op_V_COS_F32,"V_COS_F32"}, // 438
                {amdgpu_gfx1250_op_V_NOT_B32,"V_NOT_B32"}, // 439
                {amdgpu_gfx1250_op_V_BFREV_B32,"V_BFREV_B32"}, // 440
                {amdgpu_gfx1250_op_V_CLZ_I32_U32,"V_CLZ_I32_U32"}, // 441
                {amdgpu_gfx1250_op_V_CTZ_I32_B32,"V_CTZ_I32_B32"}, // 442
                {amdgpu_gfx1250_op_V_CLS_I32,"V_CLS_I32"}, // 443
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 444
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 445
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 446
                {amdgpu_gfx1250_op_V_FREXP_EXP_I32_F32,"V_FREXP_EXP_I32_F32"}, // 447
                {amdgpu_gfx1250_op_V_FREXP_MANT_F32,"V_FREXP_MANT_F32"}, // 448
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 449
                {amdgpu_gfx1250_op_V_MOVRELD_B32,"V_MOVRELD_B32"}, // 450
                {amdgpu_gfx1250_op_V_MOVRELS_B32,"V_MOVRELS_B32"}, // 451
                {amdgpu_gfx1250_op_V_MOVRELSD_B32,"V_MOVRELSD_B32"}, // 452
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 453
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 454
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 455
                {amdgpu_gfx1250_op_V_MOVRELSD_2_B32,"V_MOVRELSD_2_B32"}, // 456
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 457
                {amdgpu_gfx1250_op_V_TANH_BF16,"V_TANH_BF16"}, // 458
                {amdgpu_gfx1250_op_V_PRNG_B32,"V_PRNG_B32"}, // 459
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 460
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 461
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 462
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 463
                {amdgpu_gfx1250_op_V_CVT_F16_U16,"V_CVT_F16_U16"}, // 464
                {amdgpu_gfx1250_op_V_CVT_F16_I16,"V_CVT_F16_I16"}, // 465
                {amdgpu_gfx1250_op_V_CVT_U16_F16,"V_CVT_U16_F16"}, // 466
                {amdgpu_gfx1250_op_V_CVT_I16_F16,"V_CVT_I16_F16"}, // 467
                {amdgpu_gfx1250_op_V_RCP_F16,"V_RCP_F16"}, // 468
                {amdgpu_gfx1250_op_V_SQRT_F16,"V_SQRT_F16"}, // 469
                {amdgpu_gfx1250_op_V_RSQ_F16,"V_RSQ_F16"}, // 470
                {amdgpu_gfx1250_op_V_LOG_F16,"V_LOG_F16"}, // 471
                {amdgpu_gfx1250_op_V_EXP_F16,"V_EXP_F16"}, // 472
                {amdgpu_gfx1250_op_V_FREXP_MANT_F16,"V_FREXP_MANT_F16"}, // 473
                {amdgpu_gfx1250_op_V_FREXP_EXP_I16_F16,"V_FREXP_EXP_I16_F16"}, // 474
                {amdgpu_gfx1250_op_V_FLOOR_F16,"V_FLOOR_F16"}, // 475
                {amdgpu_gfx1250_op_V_CEIL_F16,"V_CEIL_F16"}, // 476
                {amdgpu_gfx1250_op_V_TRUNC_F16,"V_TRUNC_F16"}, // 477
                {amdgpu_gfx1250_op_V_RNDNE_F16,"V_RNDNE_F16"}, // 478
                {amdgpu_gfx1250_op_V_FRACT_F16,"V_FRACT_F16"}, // 479
                {amdgpu_gfx1250_op_V_SIN_F16,"V_SIN_F16"}, // 480
                {amdgpu_gfx1250_op_V_COS_F16,"V_COS_F16"}, // 481
                {amdgpu_gfx1250_op_V_SAT_PK_U8_I16,"V_SAT_PK_U8_I16"}, // 482
                {amdgpu_gfx1250_op_V_CVT_NORM_I16_F16,"V_CVT_NORM_I16_F16"}, // 483
                {amdgpu_gfx1250_op_V_CVT_NORM_U16_F16,"V_CVT_NORM_U16_F16"}, // 484
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 485
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 486
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 487
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 488
                {amdgpu_gfx1250_op_V_NOT_B16,"V_NOT_B16"}, // 489
                {amdgpu_gfx1250_op_V_CVT_I32_I16,"V_CVT_I32_I16"}, // 490
                {amdgpu_gfx1250_op_V_CVT_U32_U16,"V_CVT_U32_U16"}, // 491
                {amdgpu_gfx1250_op_V_CVT_F32_FP8,"V_CVT_F32_FP8"}, // 492
                {amdgpu_gfx1250_op_V_CVT_F32_BF8,"V_CVT_F32_BF8"}, // 493
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 494
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 495
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 496
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 497
                {amdgpu_gfx1250_op_V_CVT_F32_BF16,"V_CVT_F32_BF16"}, // 498
                {amdgpu_gfx1250_op_V_SAT_PK4_I4_I8,"V_SAT_PK4_I4_I8"}, // 499
                {amdgpu_gfx1250_op_V_SAT_PK4_U4_U8,"V_SAT_PK4_U4_U8"}, // 500
                {amdgpu_gfx1250_op_V_CVT_PK_F16_FP8,"V_CVT_PK_F16_FP8"}, // 501
                {amdgpu_gfx1250_op_V_CVT_PK_F16_BF8,"V_CVT_PK_F16_BF8"}, // 502
                {amdgpu_gfx1250_op_V_CVT_F16_FP8,"V_CVT_F16_FP8"}, // 503
                {amdgpu_gfx1250_op_V_CVT_F16_BF8,"V_CVT_F16_BF8"}, // 504
                {amdgpu_gfx1250_op_V_RCP_BF16,"V_RCP_BF16"}, // 505
                {amdgpu_gfx1250_op_V_SQRT_BF16,"V_SQRT_BF16"}, // 506
                {amdgpu_gfx1250_op_V_RSQ_BF16,"V_RSQ_BF16"}, // 507
                {amdgpu_gfx1250_op_V_LOG_BF16,"V_LOG_BF16"}, // 508
                {amdgpu_gfx1250_op_V_EXP_BF16,"V_EXP_BF16"}, // 509
                {amdgpu_gfx1250_op_V_SIN_BF16,"V_SIN_BF16"}, // 510
                {amdgpu_gfx1250_op_V_COS_BF16,"V_COS_BF16"}, // 511
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 512
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 513
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 514
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 515
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 516
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 517
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 518
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 519
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 520
                {amdgpu_gfx1250_op_V_FMA_DX9_ZERO_F32,"V_FMA_DX9_ZERO_F32"}, // 521
                {amdgpu_gfx1250_op_V_MAD_I32_I24,"V_MAD_I32_I24"}, // 522
                {amdgpu_gfx1250_op_V_MAD_U32_U24,"V_MAD_U32_U24"}, // 523
                {amdgpu_gfx1250_op_V_CUBEID_F32,"V_CUBEID_F32"}, // 524
                {amdgpu_gfx1250_op_V_CUBESC_F32,"V_CUBESC_F32"}, // 525
                {amdgpu_gfx1250_op_V_CUBETC_F32,"V_CUBETC_F32"}, // 526
                {amdgpu_gfx1250_op_V_CUBEMA_F32,"V_CUBEMA_F32"}, // 527
                {amdgpu_gfx1250_op_V_BFE_U32,"V_BFE_U32"}, // 528
                {amdgpu_gfx1250_op_V_BFE_I32,"V_BFE_I32"}, // 529
                {amdgpu_gfx1250_op_V_BFI_B32,"V_BFI_B32"}, // 530
                {amdgpu_gfx1250_op_V_FMA_F32,"V_FMA_F32"}, // 531
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 532
                {amdgpu_gfx1250_op_V_LERP_U8,"V_LERP_U8"}, // 533
                {amdgpu_gfx1250_op_V_ALIGNBIT_B32,"V_ALIGNBIT_B32"}, // 534
                {amdgpu_gfx1250_op_V_ALIGNBYTE_B32,"V_ALIGNBYTE_B32"}, // 535
                {amdgpu_gfx1250_op_V_MULLIT_F32,"V_MULLIT_F32"}, // 536
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 537
                {amdgpu_gfx1250_op_V_MIN3_I32,"V_MIN3_I32"}, // 538
                {amdgpu_gfx1250_op_V_MIN3_U32,"V_MIN3_U32"}, // 539
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 540
                {amdgpu_gfx1250_op_V_MAX3_I32,"V_MAX3_I32"}, // 541
                {amdgpu_gfx1250_op_V_MAX3_U32,"V_MAX3_U32"}, // 542
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 543
                {amdgpu_gfx1250_op_V_MED3_I32,"V_MED3_I32"}, // 544
                {amdgpu_gfx1250_op_V_MED3_U32,"V_MED3_U32"}, // 545
                {amdgpu_gfx1250_op_V_SAD_U8,"V_SAD_U8"}, // 546
                {amdgpu_gfx1250_op_V_SAD_HI_U8,"V_SAD_HI_U8"}, // 547
                {amdgpu_gfx1250_op_V_SAD_U16,"V_SAD_U16"}, // 548
                {amdgpu_gfx1250_op_V_SAD_U32,"V_SAD_U32"}, // 549
                {amdgpu_gfx1250_op_V_CVT_PK_U8_F32,"V_CVT_PK_U8_F32"}, // 550
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 551
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 552
                {amdgpu_gfx1250_op_V_MIN3_NUM_F32,"V_MIN3_NUM_F32"}, // 553
                {amdgpu_gfx1250_op_V_MAX3_NUM_F32,"V_MAX3_NUM_F32"}, // 554
                {amdgpu_gfx1250_op_V_MIN3_NUM_F16,"V_MIN3_NUM_F16"}, // 555
                {amdgpu_gfx1250_op_V_MAX3_NUM_F16,"V_MAX3_NUM_F16"}, // 556
                {amdgpu_gfx1250_op_V_MINIMUM3_F32,"V_MINIMUM3_F32"}, // 557
                {amdgpu_gfx1250_op_V_MAXIMUM3_F32,"V_MAXIMUM3_F32"}, // 558
                {amdgpu_gfx1250_op_V_MINIMUM3_F16,"V_MINIMUM3_F16"}, // 559
                {amdgpu_gfx1250_op_V_MAXIMUM3_F16,"V_MAXIMUM3_F16"}, // 560
                {amdgpu_gfx1250_op_V_MED3_NUM_F32,"V_MED3_NUM_F32"}, // 561
                {amdgpu_gfx1250_op_V_MED3_NUM_F16,"V_MED3_NUM_F16"}, // 562
                {amdgpu_gfx1250_op_V_BITOP3_B16,"V_BITOP3_B16"}, // 563
                {amdgpu_gfx1250_op_V_BITOP3_B32,"V_BITOP3_B32"}, // 564
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 565
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 566
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 567
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 568
                {amdgpu_gfx1250_op_V_MSAD_U8,"V_MSAD_U8"}, // 569
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 570
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 571
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 572
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 573
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 574
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 575
                {amdgpu_gfx1250_op_V_XOR3_B32,"V_XOR3_B32"}, // 576
                {amdgpu_gfx1250_op_V_MAD_U16,"V_MAD_U16"}, // 577
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 578
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 579
                {amdgpu_gfx1250_op_V_PERM_B32,"V_PERM_B32"}, // 580
                {amdgpu_gfx1250_op_V_XAD_U32,"V_XAD_U32"}, // 581
                {amdgpu_gfx1250_op_V_LSHL_ADD_U32,"V_LSHL_ADD_U32"}, // 582
                {amdgpu_gfx1250_op_V_ADD_LSHL_U32,"V_ADD_LSHL_U32"}, // 583
                {amdgpu_gfx1250_op_V_FMA_F16,"V_FMA_F16"}, // 584
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 585
                {amdgpu_gfx1250_op_V_MIN3_I16,"V_MIN3_I16"}, // 586
                {amdgpu_gfx1250_op_V_MIN3_U16,"V_MIN3_U16"}, // 587
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 588
                {amdgpu_gfx1250_op_V_MAX3_I16,"V_MAX3_I16"}, // 589
                {amdgpu_gfx1250_op_V_MAX3_U16,"V_MAX3_U16"}, // 590
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 591
                {amdgpu_gfx1250_op_V_MED3_I16,"V_MED3_I16"}, // 592
                {amdgpu_gfx1250_op_V_MED3_U16,"V_MED3_U16"}, // 593
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 594
                {amdgpu_gfx1250_op_V_MAD_I16,"V_MAD_I16"}, // 595
                {amdgpu_gfx1250_op_V_DIV_FIXUP_F16,"V_DIV_FIXUP_F16"}, // 596
                {amdgpu_gfx1250_op_V_ADD3_U32,"V_ADD3_U32"}, // 597
                {amdgpu_gfx1250_op_V_LSHL_OR_B32,"V_LSHL_OR_B32"}, // 598
                {amdgpu_gfx1250_op_V_AND_OR_B32,"V_AND_OR_B32"}, // 599
                {amdgpu_gfx1250_op_V_OR3_B32,"V_OR3_B32"}, // 600
                {amdgpu_gfx1250_op_V_MAD_U32_U16,"V_MAD_U32_U16"}, // 601
                {amdgpu_gfx1250_op_V_MAD_I32_I16,"V_MAD_I32_I16"}, // 602
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 603
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 604
                {amdgpu_gfx1250_op_V_CNDMASK_B16,"V_CNDMASK_B16"}, // 605
                {amdgpu_gfx1250_op_V_ADD_MAX_I32,"V_ADD_MAX_I32"}, // 606
                {amdgpu_gfx1250_op_V_ADD_MAX_U32,"V_ADD_MAX_U32"}, // 607
                {amdgpu_gfx1250_op_V_ADD_MIN_I32,"V_ADD_MIN_I32"}, // 608
                {amdgpu_gfx1250_op_V_ADD_MIN_U32,"V_ADD_MIN_U32"}, // 609
                {amdgpu_gfx1250_op_V_MAXMIN_U32,"V_MAXMIN_U32"}, // 610
                {amdgpu_gfx1250_op_V_MINMAX_U32,"V_MINMAX_U32"}, // 611
                {amdgpu_gfx1250_op_V_MAXMIN_I32,"V_MAXMIN_I32"}, // 612
                {amdgpu_gfx1250_op_V_MINMAX_I32,"V_MINMAX_I32"}, // 613
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 614
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 615
                {amdgpu_gfx1250_op_V_MINMAX_NUM_F32,"V_MINMAX_NUM_F32"}, // 616
                {amdgpu_gfx1250_op_V_MAXMIN_NUM_F32,"V_MAXMIN_NUM_F32"}, // 617
                {amdgpu_gfx1250_op_V_MINMAX_NUM_F16,"V_MINMAX_NUM_F16"}, // 618
                {amdgpu_gfx1250_op_V_MAXMIN_NUM_F16,"V_MAXMIN_NUM_F16"}, // 619
                {amdgpu_gfx1250_op_V_MINIMUMMAXIMUM_F32,"V_MINIMUMMAXIMUM_F32"}, // 620
                {amdgpu_gfx1250_op_V_MAXIMUMMINIMUM_F32,"V_MAXIMUMMINIMUM_F32"}, // 621
                {amdgpu_gfx1250_op_V_MINIMUMMAXIMUM_F16,"V_MINIMUMMAXIMUM_F16"}, // 622
                {amdgpu_gfx1250_op_V_MAXIMUMMINIMUM_F16,"V_MAXIMUMMINIMUM_F16"}, // 623
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 624
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 625
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 626
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 627
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 628
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 629
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 630
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 631
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 632
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 633
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 634
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 635
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 636
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 637
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 638
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 639
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 640
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 641
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 642
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 643
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 644
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 645
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 646
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 647
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 648
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 649
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 650
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 651
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 652
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 653
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 654
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 655
                {amdgpu_gfx1250_op_V_ASHR_PK_I8_I32,"V_ASHR_PK_I8_I32"}, // 656
                {amdgpu_gfx1250_op_V_ASHR_PK_U8_I32,"V_ASHR_PK_U8_I32"}, // 657
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 658
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 659
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 660
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 661
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 662
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 663
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 664
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 665
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 666
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 667
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 668
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 669
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 670
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 671
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 672
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 673
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 674
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 675
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 676
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 677
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 678
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 679
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 680
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 681
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 682
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 683
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 684
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 685
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 686
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 687
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 688
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 689
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 690
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 691
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 692
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 693
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 694
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 695
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 696
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 697
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 698
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 699
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 700
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 701
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 702
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 703
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 704
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 705
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 706
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 707
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 708
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 709
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 710
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 711
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 712
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 713
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 714
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 715
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 716
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 717
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 718
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 719
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 720
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 721
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 722
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 723
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 724
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 725
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 726
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 727
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 728
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 729
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 730
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 731
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 732
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 733
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 734
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 735
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 736
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 737
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 738
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 739
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 740
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 741
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 742
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 743
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 744
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 745
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 746
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 747
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 748
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 749
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 750
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 751
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 752
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 753
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 754
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 755
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 756
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 757
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 758
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 759
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 760
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 761
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 762
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 763
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 764
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 765
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 766
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 767
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 768
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 769
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 770
                {amdgpu_gfx1250_op_V_ADD_NC_U16,"V_ADD_NC_U16"}, // 771
                {amdgpu_gfx1250_op_V_SUB_NC_U16,"V_SUB_NC_U16"}, // 772
                {amdgpu_gfx1250_op_V_MUL_LO_U16,"V_MUL_LO_U16"}, // 773
                {amdgpu_gfx1250_op_V_CVT_PK_I16_F32,"V_CVT_PK_I16_F32"}, // 774
                {amdgpu_gfx1250_op_V_CVT_PK_U16_F32,"V_CVT_PK_U16_F32"}, // 775
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 776
                {amdgpu_gfx1250_op_V_MAX_U16,"V_MAX_U16"}, // 777
                {amdgpu_gfx1250_op_V_MAX_I16,"V_MAX_I16"}, // 778
                {amdgpu_gfx1250_op_V_MIN_U16,"V_MIN_U16"}, // 779
                {amdgpu_gfx1250_op_V_MIN_I16,"V_MIN_I16"}, // 780
                {amdgpu_gfx1250_op_V_ADD_NC_I16,"V_ADD_NC_I16"}, // 781
                {amdgpu_gfx1250_op_V_SUB_NC_I16,"V_SUB_NC_I16"}, // 782
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 783
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 784
                {amdgpu_gfx1250_op_V_PACK_B32_F16,"V_PACK_B32_F16"}, // 785
                {amdgpu_gfx1250_op_V_CVT_PK_NORM_I16_F16,"V_CVT_PK_NORM_I16_F16"}, // 786
                {amdgpu_gfx1250_op_V_CVT_PK_NORM_U16_F16,"V_CVT_PK_NORM_U16_F16"}, // 787
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 788
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 789
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 790
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 791
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 792
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 793
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 794
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 795
                {amdgpu_gfx1250_op_V_LDEXP_F32,"V_LDEXP_F32"}, // 796
                {amdgpu_gfx1250_op_V_BFM_B32,"V_BFM_B32"}, // 797
                {amdgpu_gfx1250_op_V_BCNT_U32_B32,"V_BCNT_U32_B32"}, // 798
                {amdgpu_gfx1250_op_V_MBCNT_LO_U32_B32,"V_MBCNT_LO_U32_B32"}, // 799
                {amdgpu_gfx1250_op_V_MBCNT_HI_U32_B32,"V_MBCNT_HI_U32_B32"}, // 800
                {amdgpu_gfx1250_op_V_CVT_PK_NORM_I16_F32,"V_CVT_PK_NORM_I16_F32"}, // 801
                {amdgpu_gfx1250_op_V_CVT_PK_NORM_U16_F32,"V_CVT_PK_NORM_U16_F32"}, // 802
                {amdgpu_gfx1250_op_V_CVT_PK_U16_U32,"V_CVT_PK_U16_U32"}, // 803
                {amdgpu_gfx1250_op_V_CVT_PK_I16_I32,"V_CVT_PK_I16_I32"}, // 804
                {amdgpu_gfx1250_op_V_SUB_NC_I32,"V_SUB_NC_I32"}, // 805
                {amdgpu_gfx1250_op_V_ADD_NC_I32,"V_ADD_NC_I32"}, // 806
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 807
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 808
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 809
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 810
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 811
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 812
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 813
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 814
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 815
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 816
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 817
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 818
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 819
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 820
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 821
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 822
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 823
                {amdgpu_gfx1250_op_V_LSHLREV_B16,"V_LSHLREV_B16"}, // 824
                {amdgpu_gfx1250_op_V_LSHRREV_B16,"V_LSHRREV_B16"}, // 825
                {amdgpu_gfx1250_op_V_ASHRREV_I16,"V_ASHRREV_I16"}, // 826
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 827
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 828
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 829
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 830
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 831
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 832
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 833
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 834
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 835
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 836
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 837
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 838
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 839
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 840
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 841
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 842
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 843
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 844
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 845
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 846
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 847
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 848
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 849
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 850
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 851
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 852
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 853
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 854
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 855
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 856
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 857
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 858
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 859
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 860
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 861
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 862
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 863
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 864
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 865
                {amdgpu_gfx1250_op_V_AND_B16,"V_AND_B16"}, // 866
                {amdgpu_gfx1250_op_V_OR_B16,"V_OR_B16"}, // 867
                {amdgpu_gfx1250_op_V_XOR_B16,"V_XOR_B16"}, // 868
                {amdgpu_gfx1250_op_V_MINIMUM_F32,"V_MINIMUM_F32"}, // 869
                {amdgpu_gfx1250_op_V_MAXIMUM_F32,"V_MAXIMUM_F32"}, // 870
                {amdgpu_gfx1250_op_V_MINIMUM_F16,"V_MINIMUM_F16"}, // 871
                {amdgpu_gfx1250_op_V_MAXIMUM_F16,"V_MAXIMUM_F16"}, // 872
                {amdgpu_gfx1250_op_V_CVT_PK_FP8_F32,"V_CVT_PK_FP8_F32"}, // 873
                {amdgpu_gfx1250_op_V_CVT_PK_BF8_F32,"V_CVT_PK_BF8_F32"}, // 874
                {amdgpu_gfx1250_op_V_CVT_SR_FP8_F32,"V_CVT_SR_FP8_F32"}, // 875
                {amdgpu_gfx1250_op_V_CVT_SR_BF8_F32,"V_CVT_SR_BF8_F32"}, // 876
                {amdgpu_gfx1250_op_V_CVT_PK_BF16_F32,"V_CVT_PK_BF16_F32"}, // 877
                {amdgpu_gfx1250_op_V_CVT_SR_PK_BF16_F32,"V_CVT_SR_PK_BF16_F32"}, // 878
                {amdgpu_gfx1250_op_V_CVT_PK_F16_F32,"V_CVT_PK_F16_F32"}, // 879
                {amdgpu_gfx1250_op_V_CVT_SR_PK_F16_F32,"V_CVT_SR_PK_F16_F32"}, // 880
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 881
                {amdgpu_gfx1250_op_V_CVT_PK_FP8_F16,"V_CVT_PK_FP8_F16"}, // 882
                {amdgpu_gfx1250_op_V_CVT_PK_BF8_F16,"V_CVT_PK_BF8_F16"}, // 883
                {amdgpu_gfx1250_op_V_CVT_SR_FP8_F16,"V_CVT_SR_FP8_F16"}, // 884
                {amdgpu_gfx1250_op_V_CVT_SR_BF8_F16,"V_CVT_SR_BF8_F16"}, // 885
            }; // end VOP3_VOP_DPP8_insn_table
            const amdgpu_gfx1250_insn_entry VOP3_VOP_DPP16_insn_table[886] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_V_CMP_LT_F16,"V_CMP_LT_F16"}, // 1
                {amdgpu_gfx1250_op_V_CMP_EQ_F16,"V_CMP_EQ_F16"}, // 2
                {amdgpu_gfx1250_op_V_CMP_LE_F16,"V_CMP_LE_F16"}, // 3
                {amdgpu_gfx1250_op_V_CMP_GT_F16,"V_CMP_GT_F16"}, // 4
                {amdgpu_gfx1250_op_V_CMP_LG_F16,"V_CMP_LG_F16"}, // 5
                {amdgpu_gfx1250_op_V_CMP_GE_F16,"V_CMP_GE_F16"}, // 6
                {amdgpu_gfx1250_op_V_CMP_O_F16,"V_CMP_O_F16"}, // 7
                {amdgpu_gfx1250_op_V_CMP_U_F16,"V_CMP_U_F16"}, // 8
                {amdgpu_gfx1250_op_V_CMP_NGE_F16,"V_CMP_NGE_F16"}, // 9
                {amdgpu_gfx1250_op_V_CMP_NLG_F16,"V_CMP_NLG_F16"}, // 10
                {amdgpu_gfx1250_op_V_CMP_NGT_F16,"V_CMP_NGT_F16"}, // 11
                {amdgpu_gfx1250_op_V_CMP_NLE_F16,"V_CMP_NLE_F16"}, // 12
                {amdgpu_gfx1250_op_V_CMP_NEQ_F16,"V_CMP_NEQ_F16"}, // 13
                {amdgpu_gfx1250_op_V_CMP_NLT_F16,"V_CMP_NLT_F16"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_V_CMP_LT_F32,"V_CMP_LT_F32"}, // 17
                {amdgpu_gfx1250_op_V_CMP_EQ_F32,"V_CMP_EQ_F32"}, // 18
                {amdgpu_gfx1250_op_V_CMP_LE_F32,"V_CMP_LE_F32"}, // 19
                {amdgpu_gfx1250_op_V_CMP_GT_F32,"V_CMP_GT_F32"}, // 20
                {amdgpu_gfx1250_op_V_CMP_LG_F32,"V_CMP_LG_F32"}, // 21
                {amdgpu_gfx1250_op_V_CMP_GE_F32,"V_CMP_GE_F32"}, // 22
                {amdgpu_gfx1250_op_V_CMP_O_F32,"V_CMP_O_F32"}, // 23
                {amdgpu_gfx1250_op_V_CMP_U_F32,"V_CMP_U_F32"}, // 24
                {amdgpu_gfx1250_op_V_CMP_NGE_F32,"V_CMP_NGE_F32"}, // 25
                {amdgpu_gfx1250_op_V_CMP_NLG_F32,"V_CMP_NLG_F32"}, // 26
                {amdgpu_gfx1250_op_V_CMP_NGT_F32,"V_CMP_NGT_F32"}, // 27
                {amdgpu_gfx1250_op_V_CMP_NLE_F32,"V_CMP_NLE_F32"}, // 28
                {amdgpu_gfx1250_op_V_CMP_NEQ_F32,"V_CMP_NEQ_F32"}, // 29
                {amdgpu_gfx1250_op_V_CMP_NLT_F32,"V_CMP_NLT_F32"}, // 30
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 31
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 32
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 33
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 34
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 35
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 36
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 42
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_V_CMP_LT_I16,"V_CMP_LT_I16"}, // 49
                {amdgpu_gfx1250_op_V_CMP_EQ_I16,"V_CMP_EQ_I16"}, // 50
                {amdgpu_gfx1250_op_V_CMP_LE_I16,"V_CMP_LE_I16"}, // 51
                {amdgpu_gfx1250_op_V_CMP_GT_I16,"V_CMP_GT_I16"}, // 52
                {amdgpu_gfx1250_op_V_CMP_NE_I16,"V_CMP_NE_I16"}, // 53
                {amdgpu_gfx1250_op_V_CMP_GE_I16,"V_CMP_GE_I16"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_V_CMP_LT_U16,"V_CMP_LT_U16"}, // 57
                {amdgpu_gfx1250_op_V_CMP_EQ_U16,"V_CMP_EQ_U16"}, // 58
                {amdgpu_gfx1250_op_V_CMP_LE_U16,"V_CMP_LE_U16"}, // 59
                {amdgpu_gfx1250_op_V_CMP_GT_U16,"V_CMP_GT_U16"}, // 60
                {amdgpu_gfx1250_op_V_CMP_NE_U16,"V_CMP_NE_U16"}, // 61
                {amdgpu_gfx1250_op_V_CMP_GE_U16,"V_CMP_GE_U16"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 64
                {amdgpu_gfx1250_op_V_CMP_LT_I32,"V_CMP_LT_I32"}, // 65
                {amdgpu_gfx1250_op_V_CMP_EQ_I32,"V_CMP_EQ_I32"}, // 66
                {amdgpu_gfx1250_op_V_CMP_LE_I32,"V_CMP_LE_I32"}, // 67
                {amdgpu_gfx1250_op_V_CMP_GT_I32,"V_CMP_GT_I32"}, // 68
                {amdgpu_gfx1250_op_V_CMP_NE_I32,"V_CMP_NE_I32"}, // 69
                {amdgpu_gfx1250_op_V_CMP_GE_I32,"V_CMP_GE_I32"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 72
                {amdgpu_gfx1250_op_V_CMP_LT_U32,"V_CMP_LT_U32"}, // 73
                {amdgpu_gfx1250_op_V_CMP_EQ_U32,"V_CMP_EQ_U32"}, // 74
                {amdgpu_gfx1250_op_V_CMP_LE_U32,"V_CMP_LE_U32"}, // 75
                {amdgpu_gfx1250_op_V_CMP_GT_U32,"V_CMP_GT_U32"}, // 76
                {amdgpu_gfx1250_op_V_CMP_NE_U32,"V_CMP_NE_U32"}, // 77
                {amdgpu_gfx1250_op_V_CMP_GE_U32,"V_CMP_GE_U32"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 80
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 81
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 82
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 83
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 84
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 85
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 86
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 87
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 88
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 89
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 90
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 91
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 92
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 93
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 94
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 95
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 96
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 97
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 98
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 99
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 100
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 101
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 102
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 103
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 104
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 105
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 106
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 107
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 108
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 109
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 110
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 111
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 112
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 113
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 114
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 115
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 116
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 117
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 118
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 119
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 120
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 121
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 122
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 123
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 124
                {amdgpu_gfx1250_op_V_CMP_CLASS_F16,"V_CMP_CLASS_F16"}, // 125
                {amdgpu_gfx1250_op_V_CMP_CLASS_F32,"V_CMP_CLASS_F32"}, // 126
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 127
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 128
                {amdgpu_gfx1250_op_V_CMPX_LT_F16,"V_CMPX_LT_F16"}, // 129
                {amdgpu_gfx1250_op_V_CMPX_EQ_F16,"V_CMPX_EQ_F16"}, // 130
                {amdgpu_gfx1250_op_V_CMPX_LE_F16,"V_CMPX_LE_F16"}, // 131
                {amdgpu_gfx1250_op_V_CMPX_GT_F16,"V_CMPX_GT_F16"}, // 132
                {amdgpu_gfx1250_op_V_CMPX_LG_F16,"V_CMPX_LG_F16"}, // 133
                {amdgpu_gfx1250_op_V_CMPX_GE_F16,"V_CMPX_GE_F16"}, // 134
                {amdgpu_gfx1250_op_V_CMPX_O_F16,"V_CMPX_O_F16"}, // 135
                {amdgpu_gfx1250_op_V_CMPX_U_F16,"V_CMPX_U_F16"}, // 136
                {amdgpu_gfx1250_op_V_CMPX_NGE_F16,"V_CMPX_NGE_F16"}, // 137
                {amdgpu_gfx1250_op_V_CMPX_NLG_F16,"V_CMPX_NLG_F16"}, // 138
                {amdgpu_gfx1250_op_V_CMPX_NGT_F16,"V_CMPX_NGT_F16"}, // 139
                {amdgpu_gfx1250_op_V_CMPX_NLE_F16,"V_CMPX_NLE_F16"}, // 140
                {amdgpu_gfx1250_op_V_CMPX_NEQ_F16,"V_CMPX_NEQ_F16"}, // 141
                {amdgpu_gfx1250_op_V_CMPX_NLT_F16,"V_CMPX_NLT_F16"}, // 142
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 143
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 144
                {amdgpu_gfx1250_op_V_CMPX_LT_F32,"V_CMPX_LT_F32"}, // 145
                {amdgpu_gfx1250_op_V_CMPX_EQ_F32,"V_CMPX_EQ_F32"}, // 146
                {amdgpu_gfx1250_op_V_CMPX_LE_F32,"V_CMPX_LE_F32"}, // 147
                {amdgpu_gfx1250_op_V_CMPX_GT_F32,"V_CMPX_GT_F32"}, // 148
                {amdgpu_gfx1250_op_V_CMPX_LG_F32,"V_CMPX_LG_F32"}, // 149
                {amdgpu_gfx1250_op_V_CMPX_GE_F32,"V_CMPX_GE_F32"}, // 150
                {amdgpu_gfx1250_op_V_CMPX_O_F32,"V_CMPX_O_F32"}, // 151
                {amdgpu_gfx1250_op_V_CMPX_U_F32,"V_CMPX_U_F32"}, // 152
                {amdgpu_gfx1250_op_V_CMPX_NGE_F32,"V_CMPX_NGE_F32"}, // 153
                {amdgpu_gfx1250_op_V_CMPX_NLG_F32,"V_CMPX_NLG_F32"}, // 154
                {amdgpu_gfx1250_op_V_CMPX_NGT_F32,"V_CMPX_NGT_F32"}, // 155
                {amdgpu_gfx1250_op_V_CMPX_NLE_F32,"V_CMPX_NLE_F32"}, // 156
                {amdgpu_gfx1250_op_V_CMPX_NEQ_F32,"V_CMPX_NEQ_F32"}, // 157
                {amdgpu_gfx1250_op_V_CMPX_NLT_F32,"V_CMPX_NLT_F32"}, // 158
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 159
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 160
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 161
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 162
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 163
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 164
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 165
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 166
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 167
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 168
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 169
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 170
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 171
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 172
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 173
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 174
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 175
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 176
                {amdgpu_gfx1250_op_V_CMPX_LT_I16,"V_CMPX_LT_I16"}, // 177
                {amdgpu_gfx1250_op_V_CMPX_EQ_I16,"V_CMPX_EQ_I16"}, // 178
                {amdgpu_gfx1250_op_V_CMPX_LE_I16,"V_CMPX_LE_I16"}, // 179
                {amdgpu_gfx1250_op_V_CMPX_GT_I16,"V_CMPX_GT_I16"}, // 180
                {amdgpu_gfx1250_op_V_CMPX_NE_I16,"V_CMPX_NE_I16"}, // 181
                {amdgpu_gfx1250_op_V_CMPX_GE_I16,"V_CMPX_GE_I16"}, // 182
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 183
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 184
                {amdgpu_gfx1250_op_V_CMPX_LT_U16,"V_CMPX_LT_U16"}, // 185
                {amdgpu_gfx1250_op_V_CMPX_EQ_U16,"V_CMPX_EQ_U16"}, // 186
                {amdgpu_gfx1250_op_V_CMPX_LE_U16,"V_CMPX_LE_U16"}, // 187
                {amdgpu_gfx1250_op_V_CMPX_GT_U16,"V_CMPX_GT_U16"}, // 188
                {amdgpu_gfx1250_op_V_CMPX_NE_U16,"V_CMPX_NE_U16"}, // 189
                {amdgpu_gfx1250_op_V_CMPX_GE_U16,"V_CMPX_GE_U16"}, // 190
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 191
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 192
                {amdgpu_gfx1250_op_V_CMPX_LT_I32,"V_CMPX_LT_I32"}, // 193
                {amdgpu_gfx1250_op_V_CMPX_EQ_I32,"V_CMPX_EQ_I32"}, // 194
                {amdgpu_gfx1250_op_V_CMPX_LE_I32,"V_CMPX_LE_I32"}, // 195
                {amdgpu_gfx1250_op_V_CMPX_GT_I32,"V_CMPX_GT_I32"}, // 196
                {amdgpu_gfx1250_op_V_CMPX_NE_I32,"V_CMPX_NE_I32"}, // 197
                {amdgpu_gfx1250_op_V_CMPX_GE_I32,"V_CMPX_GE_I32"}, // 198
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 199
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 200
                {amdgpu_gfx1250_op_V_CMPX_LT_U32,"V_CMPX_LT_U32"}, // 201
                {amdgpu_gfx1250_op_V_CMPX_EQ_U32,"V_CMPX_EQ_U32"}, // 202
                {amdgpu_gfx1250_op_V_CMPX_LE_U32,"V_CMPX_LE_U32"}, // 203
                {amdgpu_gfx1250_op_V_CMPX_GT_U32,"V_CMPX_GT_U32"}, // 204
                {amdgpu_gfx1250_op_V_CMPX_NE_U32,"V_CMPX_NE_U32"}, // 205
                {amdgpu_gfx1250_op_V_CMPX_GE_U32,"V_CMPX_GE_U32"}, // 206
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 207
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 208
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 209
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 210
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 211
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 212
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 213
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 214
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 215
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 216
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 217
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 218
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 219
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 220
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 221
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 222
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 223
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 224
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 225
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 226
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 227
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 228
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 229
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 230
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 231
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 232
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 233
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 234
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 235
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 236
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 237
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 238
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 239
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 240
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 241
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 242
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 243
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 244
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 245
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 246
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 247
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 248
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 249
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 250
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 251
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 252
                {amdgpu_gfx1250_op_V_CMPX_CLASS_F16,"V_CMPX_CLASS_F16"}, // 253
                {amdgpu_gfx1250_op_V_CMPX_CLASS_F32,"V_CMPX_CLASS_F32"}, // 254
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 255
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 256
                {amdgpu_gfx1250_op_V_CNDMASK_B32,"V_CNDMASK_B32"}, // 257
                {amdgpu_gfx1250_op_V_ADD_F64,"V_ADD_F64"}, // 258
                {amdgpu_gfx1250_op_V_ADD_F32,"V_ADD_F32"}, // 259
                {amdgpu_gfx1250_op_V_SUB_F32,"V_SUB_F32"}, // 260
                {amdgpu_gfx1250_op_V_SUBREV_F32,"V_SUBREV_F32"}, // 261
                {amdgpu_gfx1250_op_V_MUL_F64,"V_MUL_F64"}, // 262
                {amdgpu_gfx1250_op_V_MUL_DX9_ZERO_F32,"V_MUL_DX9_ZERO_F32"}, // 263
                {amdgpu_gfx1250_op_V_MUL_F32,"V_MUL_F32"}, // 264
                {amdgpu_gfx1250_op_V_MUL_I32_I24,"V_MUL_I32_I24"}, // 265
                {amdgpu_gfx1250_op_V_MUL_HI_I32_I24,"V_MUL_HI_I32_I24"}, // 266
                {amdgpu_gfx1250_op_V_MUL_U32_U24,"V_MUL_U32_U24"}, // 267
                {amdgpu_gfx1250_op_V_MUL_HI_U32_U24,"V_MUL_HI_U32_U24"}, // 268
                {amdgpu_gfx1250_op_V_MIN_NUM_F64,"V_MIN_NUM_F64"}, // 269
                {amdgpu_gfx1250_op_V_MAX_NUM_F64,"V_MAX_NUM_F64"}, // 270
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 271
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 272
                {amdgpu_gfx1250_op_V_MIN_I32,"V_MIN_I32"}, // 273
                {amdgpu_gfx1250_op_V_MAX_I32,"V_MAX_I32"}, // 274
                {amdgpu_gfx1250_op_V_MIN_U32,"V_MIN_U32"}, // 275
                {amdgpu_gfx1250_op_V_MAX_U32,"V_MAX_U32"}, // 276
                {amdgpu_gfx1250_op_V_MIN_NUM_F32,"V_MIN_NUM_F32"}, // 277
                {amdgpu_gfx1250_op_V_MAX_NUM_F32,"V_MAX_NUM_F32"}, // 278
                {amdgpu_gfx1250_op_V_FMAC_F64,"V_FMAC_F64"}, // 279
                {amdgpu_gfx1250_op_V_LSHLREV_B32,"V_LSHLREV_B32"}, // 280
                {amdgpu_gfx1250_op_V_LSHRREV_B32,"V_LSHRREV_B32"}, // 281
                {amdgpu_gfx1250_op_V_ASHRREV_I32,"V_ASHRREV_I32"}, // 282
                {amdgpu_gfx1250_op_V_AND_B32,"V_AND_B32"}, // 283
                {amdgpu_gfx1250_op_V_OR_B32,"V_OR_B32"}, // 284
                {amdgpu_gfx1250_op_V_XOR_B32,"V_XOR_B32"}, // 285
                {amdgpu_gfx1250_op_V_XNOR_B32,"V_XNOR_B32"}, // 286
                {amdgpu_gfx1250_op_V_LSHLREV_B64,"V_LSHLREV_B64"}, // 287
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 288
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 289
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 290
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 291
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 292
                {amdgpu_gfx1250_op_V_ADD_NC_U32,"V_ADD_NC_U32"}, // 293
                {amdgpu_gfx1250_op_V_SUB_NC_U32,"V_SUB_NC_U32"}, // 294
                {amdgpu_gfx1250_op_V_SUBREV_NC_U32,"V_SUBREV_NC_U32"}, // 295
                {amdgpu_gfx1250_op_V_ADD_NC_U64,"V_ADD_NC_U64"}, // 296
                {amdgpu_gfx1250_op_V_SUB_NC_U64,"V_SUB_NC_U64"}, // 297
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 298
                {amdgpu_gfx1250_op_V_FMAC_F32,"V_FMAC_F32"}, // 299
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 300
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 301
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 302
                {amdgpu_gfx1250_op_V_CVT_PK_RTZ_F16_F32,"V_CVT_PK_RTZ_F16_F32"}, // 303
                {amdgpu_gfx1250_op_V_MIN_NUM_F16,"V_MIN_NUM_F16"}, // 304
                {amdgpu_gfx1250_op_V_MAX_NUM_F16,"V_MAX_NUM_F16"}, // 305
                {amdgpu_gfx1250_op_V_ADD_F16,"V_ADD_F16"}, // 306
                {amdgpu_gfx1250_op_V_SUB_F16,"V_SUB_F16"}, // 307
                {amdgpu_gfx1250_op_V_SUBREV_F16,"V_SUBREV_F16"}, // 308
                {amdgpu_gfx1250_op_V_MUL_F16,"V_MUL_F16"}, // 309
                {amdgpu_gfx1250_op_V_FMAC_F16,"V_FMAC_F16"}, // 310
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 311
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 312
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 313
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 314
                {amdgpu_gfx1250_op_V_LDEXP_F16,"V_LDEXP_F16"}, // 315
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 316
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 317
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 318
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 319
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 320
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 321
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 322
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 323
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 324
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 325
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 326
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 327
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 328
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 329
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 330
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 331
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 332
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 333
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 334
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 335
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 336
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 337
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 338
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 339
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 340
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 341
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 342
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 343
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 344
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 345
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 346
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 347
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 348
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 349
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 350
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 351
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 352
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 353
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 354
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 355
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 356
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 357
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 358
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 359
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 360
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 361
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 362
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 363
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 364
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 365
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 366
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 367
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 368
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 369
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 370
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 371
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 372
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 373
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 374
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 375
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 376
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 377
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 378
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 379
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 380
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 381
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 382
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 383
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 384
                {amdgpu_gfx1250_op_V_MOV_B32,"V_MOV_B32"}, // 385
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 386
                {amdgpu_gfx1250_op_V_CVT_I32_F64,"V_CVT_I32_F64"}, // 387
                {amdgpu_gfx1250_op_V_CVT_F64_I32,"V_CVT_F64_I32"}, // 388
                {amdgpu_gfx1250_op_V_CVT_F32_I32,"V_CVT_F32_I32"}, // 389
                {amdgpu_gfx1250_op_V_CVT_F32_U32,"V_CVT_F32_U32"}, // 390
                {amdgpu_gfx1250_op_V_CVT_U32_F32,"V_CVT_U32_F32"}, // 391
                {amdgpu_gfx1250_op_V_CVT_I32_F32,"V_CVT_I32_F32"}, // 392
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 393
                {amdgpu_gfx1250_op_V_CVT_F16_F32,"V_CVT_F16_F32"}, // 394
                {amdgpu_gfx1250_op_V_CVT_F32_F16,"V_CVT_F32_F16"}, // 395
                {amdgpu_gfx1250_op_V_CVT_NEAREST_I32_F32,"V_CVT_NEAREST_I32_F32"}, // 396
                {amdgpu_gfx1250_op_V_CVT_FLOOR_I32_F32,"V_CVT_FLOOR_I32_F32"}, // 397
                {amdgpu_gfx1250_op_V_CVT_OFF_F32_I4,"V_CVT_OFF_F32_I4"}, // 398
                {amdgpu_gfx1250_op_V_CVT_F32_F64,"V_CVT_F32_F64"}, // 399
                {amdgpu_gfx1250_op_V_CVT_F64_F32,"V_CVT_F64_F32"}, // 400
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE0,"V_CVT_F32_UBYTE0"}, // 401
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE1,"V_CVT_F32_UBYTE1"}, // 402
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE2,"V_CVT_F32_UBYTE2"}, // 403
                {amdgpu_gfx1250_op_V_CVT_F32_UBYTE3,"V_CVT_F32_UBYTE3"}, // 404
                {amdgpu_gfx1250_op_V_CVT_U32_F64,"V_CVT_U32_F64"}, // 405
                {amdgpu_gfx1250_op_V_CVT_F64_U32,"V_CVT_F64_U32"}, // 406
                {amdgpu_gfx1250_op_V_TRUNC_F64,"V_TRUNC_F64"}, // 407
                {amdgpu_gfx1250_op_V_CEIL_F64,"V_CEIL_F64"}, // 408
                {amdgpu_gfx1250_op_V_RNDNE_F64,"V_RNDNE_F64"}, // 409
                {amdgpu_gfx1250_op_V_FLOOR_F64,"V_FLOOR_F64"}, // 410
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 411
                {amdgpu_gfx1250_op_V_MOV_B16,"V_MOV_B16"}, // 412
                {amdgpu_gfx1250_op_V_MOV_B64,"V_MOV_B64"}, // 413
                {amdgpu_gfx1250_op_V_TANH_F32,"V_TANH_F32"}, // 414
                {amdgpu_gfx1250_op_V_TANH_F16,"V_TANH_F16"}, // 415
                {amdgpu_gfx1250_op_V_FRACT_F32,"V_FRACT_F32"}, // 416
                {amdgpu_gfx1250_op_V_TRUNC_F32,"V_TRUNC_F32"}, // 417
                {amdgpu_gfx1250_op_V_CEIL_F32,"V_CEIL_F32"}, // 418
                {amdgpu_gfx1250_op_V_RNDNE_F32,"V_RNDNE_F32"}, // 419
                {amdgpu_gfx1250_op_V_FLOOR_F32,"V_FLOOR_F32"}, // 420
                {amdgpu_gfx1250_op_V_EXP_F32,"V_EXP_F32"}, // 421
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 422
                {amdgpu_gfx1250_op_V_LOG_F32,"V_LOG_F32"}, // 423
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 424
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 425
                {amdgpu_gfx1250_op_V_RCP_F32,"V_RCP_F32"}, // 426
                {amdgpu_gfx1250_op_V_RCP_IFLAG_F32,"V_RCP_IFLAG_F32"}, // 427
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 428
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 429
                {amdgpu_gfx1250_op_V_RSQ_F32,"V_RSQ_F32"}, // 430
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 431
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 432
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 433
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 434
                {amdgpu_gfx1250_op_V_SQRT_F32,"V_SQRT_F32"}, // 435
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 436
                {amdgpu_gfx1250_op_V_SIN_F32,"V_SIN_F32"}, // 437
                {amdgpu_gfx1250_op_V_COS_F32,"V_COS_F32"}, // 438
                {amdgpu_gfx1250_op_V_NOT_B32,"V_NOT_B32"}, // 439
                {amdgpu_gfx1250_op_V_BFREV_B32,"V_BFREV_B32"}, // 440
                {amdgpu_gfx1250_op_V_CLZ_I32_U32,"V_CLZ_I32_U32"}, // 441
                {amdgpu_gfx1250_op_V_CTZ_I32_B32,"V_CTZ_I32_B32"}, // 442
                {amdgpu_gfx1250_op_V_CLS_I32,"V_CLS_I32"}, // 443
                {amdgpu_gfx1250_op_V_FREXP_EXP_I32_F64,"V_FREXP_EXP_I32_F64"}, // 444
                {amdgpu_gfx1250_op_V_FREXP_MANT_F64,"V_FREXP_MANT_F64"}, // 445
                {amdgpu_gfx1250_op_V_FRACT_F64,"V_FRACT_F64"}, // 446
                {amdgpu_gfx1250_op_V_FREXP_EXP_I32_F32,"V_FREXP_EXP_I32_F32"}, // 447
                {amdgpu_gfx1250_op_V_FREXP_MANT_F32,"V_FREXP_MANT_F32"}, // 448
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 449
                {amdgpu_gfx1250_op_V_MOVRELD_B32,"V_MOVRELD_B32"}, // 450
                {amdgpu_gfx1250_op_V_MOVRELS_B32,"V_MOVRELS_B32"}, // 451
                {amdgpu_gfx1250_op_V_MOVRELSD_B32,"V_MOVRELSD_B32"}, // 452
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 453
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 454
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 455
                {amdgpu_gfx1250_op_V_MOVRELSD_2_B32,"V_MOVRELSD_2_B32"}, // 456
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 457
                {amdgpu_gfx1250_op_V_TANH_BF16,"V_TANH_BF16"}, // 458
                {amdgpu_gfx1250_op_V_PRNG_B32,"V_PRNG_B32"}, // 459
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 460
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 461
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 462
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 463
                {amdgpu_gfx1250_op_V_CVT_F16_U16,"V_CVT_F16_U16"}, // 464
                {amdgpu_gfx1250_op_V_CVT_F16_I16,"V_CVT_F16_I16"}, // 465
                {amdgpu_gfx1250_op_V_CVT_U16_F16,"V_CVT_U16_F16"}, // 466
                {amdgpu_gfx1250_op_V_CVT_I16_F16,"V_CVT_I16_F16"}, // 467
                {amdgpu_gfx1250_op_V_RCP_F16,"V_RCP_F16"}, // 468
                {amdgpu_gfx1250_op_V_SQRT_F16,"V_SQRT_F16"}, // 469
                {amdgpu_gfx1250_op_V_RSQ_F16,"V_RSQ_F16"}, // 470
                {amdgpu_gfx1250_op_V_LOG_F16,"V_LOG_F16"}, // 471
                {amdgpu_gfx1250_op_V_EXP_F16,"V_EXP_F16"}, // 472
                {amdgpu_gfx1250_op_V_FREXP_MANT_F16,"V_FREXP_MANT_F16"}, // 473
                {amdgpu_gfx1250_op_V_FREXP_EXP_I16_F16,"V_FREXP_EXP_I16_F16"}, // 474
                {amdgpu_gfx1250_op_V_FLOOR_F16,"V_FLOOR_F16"}, // 475
                {amdgpu_gfx1250_op_V_CEIL_F16,"V_CEIL_F16"}, // 476
                {amdgpu_gfx1250_op_V_TRUNC_F16,"V_TRUNC_F16"}, // 477
                {amdgpu_gfx1250_op_V_RNDNE_F16,"V_RNDNE_F16"}, // 478
                {amdgpu_gfx1250_op_V_FRACT_F16,"V_FRACT_F16"}, // 479
                {amdgpu_gfx1250_op_V_SIN_F16,"V_SIN_F16"}, // 480
                {amdgpu_gfx1250_op_V_COS_F16,"V_COS_F16"}, // 481
                {amdgpu_gfx1250_op_V_SAT_PK_U8_I16,"V_SAT_PK_U8_I16"}, // 482
                {amdgpu_gfx1250_op_V_CVT_NORM_I16_F16,"V_CVT_NORM_I16_F16"}, // 483
                {amdgpu_gfx1250_op_V_CVT_NORM_U16_F16,"V_CVT_NORM_U16_F16"}, // 484
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 485
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 486
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 487
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 488
                {amdgpu_gfx1250_op_V_NOT_B16,"V_NOT_B16"}, // 489
                {amdgpu_gfx1250_op_V_CVT_I32_I16,"V_CVT_I32_I16"}, // 490
                {amdgpu_gfx1250_op_V_CVT_U32_U16,"V_CVT_U32_U16"}, // 491
                {amdgpu_gfx1250_op_V_CVT_F32_FP8,"V_CVT_F32_FP8"}, // 492
                {amdgpu_gfx1250_op_V_CVT_F32_BF8,"V_CVT_F32_BF8"}, // 493
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 494
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 495
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 496
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 497
                {amdgpu_gfx1250_op_V_CVT_F32_BF16,"V_CVT_F32_BF16"}, // 498
                {amdgpu_gfx1250_op_V_SAT_PK4_I4_I8,"V_SAT_PK4_I4_I8"}, // 499
                {amdgpu_gfx1250_op_V_SAT_PK4_U4_U8,"V_SAT_PK4_U4_U8"}, // 500
                {amdgpu_gfx1250_op_V_CVT_PK_F16_FP8,"V_CVT_PK_F16_FP8"}, // 501
                {amdgpu_gfx1250_op_V_CVT_PK_F16_BF8,"V_CVT_PK_F16_BF8"}, // 502
                {amdgpu_gfx1250_op_V_CVT_F16_FP8,"V_CVT_F16_FP8"}, // 503
                {amdgpu_gfx1250_op_V_CVT_F16_BF8,"V_CVT_F16_BF8"}, // 504
                {amdgpu_gfx1250_op_V_RCP_BF16,"V_RCP_BF16"}, // 505
                {amdgpu_gfx1250_op_V_SQRT_BF16,"V_SQRT_BF16"}, // 506
                {amdgpu_gfx1250_op_V_RSQ_BF16,"V_RSQ_BF16"}, // 507
                {amdgpu_gfx1250_op_V_LOG_BF16,"V_LOG_BF16"}, // 508
                {amdgpu_gfx1250_op_V_EXP_BF16,"V_EXP_BF16"}, // 509
                {amdgpu_gfx1250_op_V_SIN_BF16,"V_SIN_BF16"}, // 510
                {amdgpu_gfx1250_op_V_COS_BF16,"V_COS_BF16"}, // 511
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 512
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 513
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 514
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 515
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 516
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 517
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 518
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 519
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 520
                {amdgpu_gfx1250_op_V_FMA_DX9_ZERO_F32,"V_FMA_DX9_ZERO_F32"}, // 521
                {amdgpu_gfx1250_op_V_MAD_I32_I24,"V_MAD_I32_I24"}, // 522
                {amdgpu_gfx1250_op_V_MAD_U32_U24,"V_MAD_U32_U24"}, // 523
                {amdgpu_gfx1250_op_V_CUBEID_F32,"V_CUBEID_F32"}, // 524
                {amdgpu_gfx1250_op_V_CUBESC_F32,"V_CUBESC_F32"}, // 525
                {amdgpu_gfx1250_op_V_CUBETC_F32,"V_CUBETC_F32"}, // 526
                {amdgpu_gfx1250_op_V_CUBEMA_F32,"V_CUBEMA_F32"}, // 527
                {amdgpu_gfx1250_op_V_BFE_U32,"V_BFE_U32"}, // 528
                {amdgpu_gfx1250_op_V_BFE_I32,"V_BFE_I32"}, // 529
                {amdgpu_gfx1250_op_V_BFI_B32,"V_BFI_B32"}, // 530
                {amdgpu_gfx1250_op_V_FMA_F32,"V_FMA_F32"}, // 531
                {amdgpu_gfx1250_op_V_FMA_F64,"V_FMA_F64"}, // 532
                {amdgpu_gfx1250_op_V_LERP_U8,"V_LERP_U8"}, // 533
                {amdgpu_gfx1250_op_V_ALIGNBIT_B32,"V_ALIGNBIT_B32"}, // 534
                {amdgpu_gfx1250_op_V_ALIGNBYTE_B32,"V_ALIGNBYTE_B32"}, // 535
                {amdgpu_gfx1250_op_V_MULLIT_F32,"V_MULLIT_F32"}, // 536
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 537
                {amdgpu_gfx1250_op_V_MIN3_I32,"V_MIN3_I32"}, // 538
                {amdgpu_gfx1250_op_V_MIN3_U32,"V_MIN3_U32"}, // 539
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 540
                {amdgpu_gfx1250_op_V_MAX3_I32,"V_MAX3_I32"}, // 541
                {amdgpu_gfx1250_op_V_MAX3_U32,"V_MAX3_U32"}, // 542
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 543
                {amdgpu_gfx1250_op_V_MED3_I32,"V_MED3_I32"}, // 544
                {amdgpu_gfx1250_op_V_MED3_U32,"V_MED3_U32"}, // 545
                {amdgpu_gfx1250_op_V_SAD_U8,"V_SAD_U8"}, // 546
                {amdgpu_gfx1250_op_V_SAD_HI_U8,"V_SAD_HI_U8"}, // 547
                {amdgpu_gfx1250_op_V_SAD_U16,"V_SAD_U16"}, // 548
                {amdgpu_gfx1250_op_V_SAD_U32,"V_SAD_U32"}, // 549
                {amdgpu_gfx1250_op_V_CVT_PK_U8_F32,"V_CVT_PK_U8_F32"}, // 550
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 551
                {amdgpu_gfx1250_op_V_DIV_FIXUP_F64,"V_DIV_FIXUP_F64"}, // 552
                {amdgpu_gfx1250_op_V_MIN3_NUM_F32,"V_MIN3_NUM_F32"}, // 553
                {amdgpu_gfx1250_op_V_MAX3_NUM_F32,"V_MAX3_NUM_F32"}, // 554
                {amdgpu_gfx1250_op_V_MIN3_NUM_F16,"V_MIN3_NUM_F16"}, // 555
                {amdgpu_gfx1250_op_V_MAX3_NUM_F16,"V_MAX3_NUM_F16"}, // 556
                {amdgpu_gfx1250_op_V_MINIMUM3_F32,"V_MINIMUM3_F32"}, // 557
                {amdgpu_gfx1250_op_V_MAXIMUM3_F32,"V_MAXIMUM3_F32"}, // 558
                {amdgpu_gfx1250_op_V_MINIMUM3_F16,"V_MINIMUM3_F16"}, // 559
                {amdgpu_gfx1250_op_V_MAXIMUM3_F16,"V_MAXIMUM3_F16"}, // 560
                {amdgpu_gfx1250_op_V_MED3_NUM_F32,"V_MED3_NUM_F32"}, // 561
                {amdgpu_gfx1250_op_V_MED3_NUM_F16,"V_MED3_NUM_F16"}, // 562
                {amdgpu_gfx1250_op_V_BITOP3_B16,"V_BITOP3_B16"}, // 563
                {amdgpu_gfx1250_op_V_BITOP3_B32,"V_BITOP3_B32"}, // 564
                {amdgpu_gfx1250_op_V_MAD_U32,"V_MAD_U32"}, // 565
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 566
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 567
                {amdgpu_gfx1250_op_V_DIV_FMAS_F64,"V_DIV_FMAS_F64"}, // 568
                {amdgpu_gfx1250_op_V_MSAD_U8,"V_MSAD_U8"}, // 569
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 570
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 571
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 572
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 573
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 574
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 575
                {amdgpu_gfx1250_op_V_XOR3_B32,"V_XOR3_B32"}, // 576
                {amdgpu_gfx1250_op_V_MAD_U16,"V_MAD_U16"}, // 577
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 578
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 579
                {amdgpu_gfx1250_op_V_PERM_B32,"V_PERM_B32"}, // 580
                {amdgpu_gfx1250_op_V_XAD_U32,"V_XAD_U32"}, // 581
                {amdgpu_gfx1250_op_V_LSHL_ADD_U32,"V_LSHL_ADD_U32"}, // 582
                {amdgpu_gfx1250_op_V_ADD_LSHL_U32,"V_ADD_LSHL_U32"}, // 583
                {amdgpu_gfx1250_op_V_FMA_F16,"V_FMA_F16"}, // 584
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 585
                {amdgpu_gfx1250_op_V_MIN3_I16,"V_MIN3_I16"}, // 586
                {amdgpu_gfx1250_op_V_MIN3_U16,"V_MIN3_U16"}, // 587
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 588
                {amdgpu_gfx1250_op_V_MAX3_I16,"V_MAX3_I16"}, // 589
                {amdgpu_gfx1250_op_V_MAX3_U16,"V_MAX3_U16"}, // 590
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 591
                {amdgpu_gfx1250_op_V_MED3_I16,"V_MED3_I16"}, // 592
                {amdgpu_gfx1250_op_V_MED3_U16,"V_MED3_U16"}, // 593
                {amdgpu_gfx1250_op_V_LSHL_ADD_U64,"V_LSHL_ADD_U64"}, // 594
                {amdgpu_gfx1250_op_V_MAD_I16,"V_MAD_I16"}, // 595
                {amdgpu_gfx1250_op_V_DIV_FIXUP_F16,"V_DIV_FIXUP_F16"}, // 596
                {amdgpu_gfx1250_op_V_ADD3_U32,"V_ADD3_U32"}, // 597
                {amdgpu_gfx1250_op_V_LSHL_OR_B32,"V_LSHL_OR_B32"}, // 598
                {amdgpu_gfx1250_op_V_AND_OR_B32,"V_AND_OR_B32"}, // 599
                {amdgpu_gfx1250_op_V_OR3_B32,"V_OR3_B32"}, // 600
                {amdgpu_gfx1250_op_V_MAD_U32_U16,"V_MAD_U32_U16"}, // 601
                {amdgpu_gfx1250_op_V_MAD_I32_I16,"V_MAD_I32_I16"}, // 602
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 603
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 604
                {amdgpu_gfx1250_op_V_CNDMASK_B16,"V_CNDMASK_B16"}, // 605
                {amdgpu_gfx1250_op_V_ADD_MAX_I32,"V_ADD_MAX_I32"}, // 606
                {amdgpu_gfx1250_op_V_ADD_MAX_U32,"V_ADD_MAX_U32"}, // 607
                {amdgpu_gfx1250_op_V_ADD_MIN_I32,"V_ADD_MIN_I32"}, // 608
                {amdgpu_gfx1250_op_V_ADD_MIN_U32,"V_ADD_MIN_U32"}, // 609
                {amdgpu_gfx1250_op_V_MAXMIN_U32,"V_MAXMIN_U32"}, // 610
                {amdgpu_gfx1250_op_V_MINMAX_U32,"V_MINMAX_U32"}, // 611
                {amdgpu_gfx1250_op_V_MAXMIN_I32,"V_MAXMIN_I32"}, // 612
                {amdgpu_gfx1250_op_V_MINMAX_I32,"V_MINMAX_I32"}, // 613
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 614
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 615
                {amdgpu_gfx1250_op_V_MINMAX_NUM_F32,"V_MINMAX_NUM_F32"}, // 616
                {amdgpu_gfx1250_op_V_MAXMIN_NUM_F32,"V_MAXMIN_NUM_F32"}, // 617
                {amdgpu_gfx1250_op_V_MINMAX_NUM_F16,"V_MINMAX_NUM_F16"}, // 618
                {amdgpu_gfx1250_op_V_MAXMIN_NUM_F16,"V_MAXMIN_NUM_F16"}, // 619
                {amdgpu_gfx1250_op_V_MINIMUMMAXIMUM_F32,"V_MINIMUMMAXIMUM_F32"}, // 620
                {amdgpu_gfx1250_op_V_MAXIMUMMINIMUM_F32,"V_MAXIMUMMINIMUM_F32"}, // 621
                {amdgpu_gfx1250_op_V_MINIMUMMAXIMUM_F16,"V_MINIMUMMAXIMUM_F16"}, // 622
                {amdgpu_gfx1250_op_V_MAXIMUMMINIMUM_F16,"V_MAXIMUMMINIMUM_F16"}, // 623
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 624
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 625
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 626
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 627
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 628
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 629
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 630
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 631
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 632
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 633
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 634
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 635
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 636
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 637
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 638
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 639
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 640
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 641
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 642
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 643
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 644
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 645
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 646
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 647
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 648
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 649
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 650
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 651
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 652
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 653
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 654
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 655
                {amdgpu_gfx1250_op_V_ASHR_PK_I8_I32,"V_ASHR_PK_I8_I32"}, // 656
                {amdgpu_gfx1250_op_V_ASHR_PK_U8_I32,"V_ASHR_PK_U8_I32"}, // 657
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 658
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 659
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 660
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 661
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 662
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 663
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 664
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 665
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 666
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 667
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 668
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 669
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 670
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 671
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 672
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 673
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 674
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 675
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 676
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 677
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 678
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 679
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 680
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 681
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 682
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 683
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 684
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 685
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 686
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 687
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 688
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 689
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 690
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 691
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 692
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 693
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 694
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 695
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 696
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 697
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 698
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 699
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 700
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 701
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 702
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 703
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 704
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 705
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 706
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 707
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 708
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 709
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 710
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 711
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 712
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 713
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 714
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 715
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 716
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 717
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 718
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 719
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 720
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 721
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 722
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 723
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 724
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 725
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 726
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 727
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 728
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 729
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 730
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 731
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 732
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 733
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 734
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 735
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 736
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 737
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 738
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 739
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 740
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 741
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 742
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 743
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 744
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 745
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 746
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 747
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 748
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 749
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 750
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 751
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 752
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 753
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 754
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 755
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 756
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 757
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 758
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 759
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 760
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 761
                {amdgpu_gfx1250_op_V_MAD_NC_U64_U32,"V_MAD_NC_U64_U32"}, // 762
                {amdgpu_gfx1250_op_V_MAD_NC_I64_I32,"V_MAD_NC_I64_I32"}, // 763
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 764
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 765
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 766
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 767
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 768
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 769
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 770
                {amdgpu_gfx1250_op_V_ADD_NC_U16,"V_ADD_NC_U16"}, // 771
                {amdgpu_gfx1250_op_V_SUB_NC_U16,"V_SUB_NC_U16"}, // 772
                {amdgpu_gfx1250_op_V_MUL_LO_U16,"V_MUL_LO_U16"}, // 773
                {amdgpu_gfx1250_op_V_CVT_PK_I16_F32,"V_CVT_PK_I16_F32"}, // 774
                {amdgpu_gfx1250_op_V_CVT_PK_U16_F32,"V_CVT_PK_U16_F32"}, // 775
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 776
                {amdgpu_gfx1250_op_V_MAX_U16,"V_MAX_U16"}, // 777
                {amdgpu_gfx1250_op_V_MAX_I16,"V_MAX_I16"}, // 778
                {amdgpu_gfx1250_op_V_MIN_U16,"V_MIN_U16"}, // 779
                {amdgpu_gfx1250_op_V_MIN_I16,"V_MIN_I16"}, // 780
                {amdgpu_gfx1250_op_V_ADD_NC_I16,"V_ADD_NC_I16"}, // 781
                {amdgpu_gfx1250_op_V_SUB_NC_I16,"V_SUB_NC_I16"}, // 782
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 783
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 784
                {amdgpu_gfx1250_op_V_PACK_B32_F16,"V_PACK_B32_F16"}, // 785
                {amdgpu_gfx1250_op_V_CVT_PK_NORM_I16_F16,"V_CVT_PK_NORM_I16_F16"}, // 786
                {amdgpu_gfx1250_op_V_CVT_PK_NORM_U16_F16,"V_CVT_PK_NORM_U16_F16"}, // 787
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 788
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 789
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 790
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 791
                {amdgpu_gfx1250_op_V_MIN_U64,"V_MIN_U64"}, // 792
                {amdgpu_gfx1250_op_V_MAX_U64,"V_MAX_U64"}, // 793
                {amdgpu_gfx1250_op_V_MIN_I64,"V_MIN_I64"}, // 794
                {amdgpu_gfx1250_op_V_MAX_I64,"V_MAX_I64"}, // 795
                {amdgpu_gfx1250_op_V_LDEXP_F32,"V_LDEXP_F32"}, // 796
                {amdgpu_gfx1250_op_V_BFM_B32,"V_BFM_B32"}, // 797
                {amdgpu_gfx1250_op_V_BCNT_U32_B32,"V_BCNT_U32_B32"}, // 798
                {amdgpu_gfx1250_op_V_MBCNT_LO_U32_B32,"V_MBCNT_LO_U32_B32"}, // 799
                {amdgpu_gfx1250_op_V_MBCNT_HI_U32_B32,"V_MBCNT_HI_U32_B32"}, // 800
                {amdgpu_gfx1250_op_V_CVT_PK_NORM_I16_F32,"V_CVT_PK_NORM_I16_F32"}, // 801
                {amdgpu_gfx1250_op_V_CVT_PK_NORM_U16_F32,"V_CVT_PK_NORM_U16_F32"}, // 802
                {amdgpu_gfx1250_op_V_CVT_PK_U16_U32,"V_CVT_PK_U16_U32"}, // 803
                {amdgpu_gfx1250_op_V_CVT_PK_I16_I32,"V_CVT_PK_I16_I32"}, // 804
                {amdgpu_gfx1250_op_V_SUB_NC_I32,"V_SUB_NC_I32"}, // 805
                {amdgpu_gfx1250_op_V_ADD_NC_I32,"V_ADD_NC_I32"}, // 806
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 807
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 808
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 809
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 810
                {amdgpu_gfx1250_op_V_LDEXP_F64,"V_LDEXP_F64"}, // 811
                {amdgpu_gfx1250_op_V_MUL_LO_U32,"V_MUL_LO_U32"}, // 812
                {amdgpu_gfx1250_op_V_MUL_HI_U32,"V_MUL_HI_U32"}, // 813
                {amdgpu_gfx1250_op_V_MUL_HI_I32,"V_MUL_HI_I32"}, // 814
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 815
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 816
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 817
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 818
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 819
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 820
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 821
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 822
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 823
                {amdgpu_gfx1250_op_V_LSHLREV_B16,"V_LSHLREV_B16"}, // 824
                {amdgpu_gfx1250_op_V_LSHRREV_B16,"V_LSHRREV_B16"}, // 825
                {amdgpu_gfx1250_op_V_ASHRREV_I16,"V_ASHRREV_I16"}, // 826
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 827
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 828
                {amdgpu_gfx1250_op_V_LSHRREV_B64,"V_LSHRREV_B64"}, // 829
                {amdgpu_gfx1250_op_V_ASHRREV_I64,"V_ASHRREV_I64"}, // 830
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 831
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 832
                {amdgpu_gfx1250_op_V_MINIMUM_F64,"V_MINIMUM_F64"}, // 833
                {amdgpu_gfx1250_op_V_MAXIMUM_F64,"V_MAXIMUM_F64"}, // 834
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 835
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 836
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 837
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 838
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 839
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 840
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 841
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 842
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 843
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 844
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 845
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 846
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 847
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 848
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 849
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 850
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 851
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 852
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 853
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 854
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 855
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 856
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 857
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 858
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 859
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 860
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 861
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 862
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 863
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 864
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 865
                {amdgpu_gfx1250_op_V_AND_B16,"V_AND_B16"}, // 866
                {amdgpu_gfx1250_op_V_OR_B16,"V_OR_B16"}, // 867
                {amdgpu_gfx1250_op_V_XOR_B16,"V_XOR_B16"}, // 868
                {amdgpu_gfx1250_op_V_MINIMUM_F32,"V_MINIMUM_F32"}, // 869
                {amdgpu_gfx1250_op_V_MAXIMUM_F32,"V_MAXIMUM_F32"}, // 870
                {amdgpu_gfx1250_op_V_MINIMUM_F16,"V_MINIMUM_F16"}, // 871
                {amdgpu_gfx1250_op_V_MAXIMUM_F16,"V_MAXIMUM_F16"}, // 872
                {amdgpu_gfx1250_op_V_CVT_PK_FP8_F32,"V_CVT_PK_FP8_F32"}, // 873
                {amdgpu_gfx1250_op_V_CVT_PK_BF8_F32,"V_CVT_PK_BF8_F32"}, // 874
                {amdgpu_gfx1250_op_V_CVT_SR_FP8_F32,"V_CVT_SR_FP8_F32"}, // 875
                {amdgpu_gfx1250_op_V_CVT_SR_BF8_F32,"V_CVT_SR_BF8_F32"}, // 876
                {amdgpu_gfx1250_op_V_CVT_PK_BF16_F32,"V_CVT_PK_BF16_F32"}, // 877
                {amdgpu_gfx1250_op_V_CVT_SR_PK_BF16_F32,"V_CVT_SR_PK_BF16_F32"}, // 878
                {amdgpu_gfx1250_op_V_CVT_PK_F16_F32,"V_CVT_PK_F16_F32"}, // 879
                {amdgpu_gfx1250_op_V_CVT_SR_PK_F16_F32,"V_CVT_SR_PK_F16_F32"}, // 880
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 881
                {amdgpu_gfx1250_op_V_CVT_PK_FP8_F16,"V_CVT_PK_FP8_F16"}, // 882
                {amdgpu_gfx1250_op_V_CVT_PK_BF8_F16,"V_CVT_PK_BF8_F16"}, // 883
                {amdgpu_gfx1250_op_V_CVT_SR_FP8_F16,"V_CVT_SR_FP8_F16"}, // 884
                {amdgpu_gfx1250_op_V_CVT_SR_BF8_F16,"V_CVT_SR_BF8_F16"}, // 885
            }; // end VOP3_VOP_DPP16_insn_table
            const amdgpu_gfx1250_insn_entry ENC_VOP2_insn_table[61] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_V_CNDMASK_B32,"V_CNDMASK_B32"}, // 1
                {amdgpu_gfx1250_op_V_ADD_F64,"V_ADD_F64"}, // 2
                {amdgpu_gfx1250_op_V_ADD_F32,"V_ADD_F32"}, // 3
                {amdgpu_gfx1250_op_V_SUB_F32,"V_SUB_F32"}, // 4
                {amdgpu_gfx1250_op_V_SUBREV_F32,"V_SUBREV_F32"}, // 5
                {amdgpu_gfx1250_op_V_MUL_F64,"V_MUL_F64"}, // 6
                {amdgpu_gfx1250_op_V_MUL_DX9_ZERO_F32,"V_MUL_DX9_ZERO_F32"}, // 7
                {amdgpu_gfx1250_op_V_MUL_F32,"V_MUL_F32"}, // 8
                {amdgpu_gfx1250_op_V_MUL_I32_I24,"V_MUL_I32_I24"}, // 9
                {amdgpu_gfx1250_op_V_MUL_HI_I32_I24,"V_MUL_HI_I32_I24"}, // 10
                {amdgpu_gfx1250_op_V_MUL_U32_U24,"V_MUL_U32_U24"}, // 11
                {amdgpu_gfx1250_op_V_MUL_HI_U32_U24,"V_MUL_HI_U32_U24"}, // 12
                {amdgpu_gfx1250_op_V_MIN_NUM_F64,"V_MIN_NUM_F64"}, // 13
                {amdgpu_gfx1250_op_V_MAX_NUM_F64,"V_MAX_NUM_F64"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_V_MIN_I32,"V_MIN_I32"}, // 17
                {amdgpu_gfx1250_op_V_MAX_I32,"V_MAX_I32"}, // 18
                {amdgpu_gfx1250_op_V_MIN_U32,"V_MIN_U32"}, // 19
                {amdgpu_gfx1250_op_V_MAX_U32,"V_MAX_U32"}, // 20
                {amdgpu_gfx1250_op_V_MIN_NUM_F32,"V_MIN_NUM_F32"}, // 21
                {amdgpu_gfx1250_op_V_MAX_NUM_F32,"V_MAX_NUM_F32"}, // 22
                {amdgpu_gfx1250_op_V_FMAC_F64,"V_FMAC_F64"}, // 23
                {amdgpu_gfx1250_op_V_LSHLREV_B32,"V_LSHLREV_B32"}, // 24
                {amdgpu_gfx1250_op_V_LSHRREV_B32,"V_LSHRREV_B32"}, // 25
                {amdgpu_gfx1250_op_V_ASHRREV_I32,"V_ASHRREV_I32"}, // 26
                {amdgpu_gfx1250_op_V_AND_B32,"V_AND_B32"}, // 27
                {amdgpu_gfx1250_op_V_OR_B32,"V_OR_B32"}, // 28
                {amdgpu_gfx1250_op_V_XOR_B32,"V_XOR_B32"}, // 29
                {amdgpu_gfx1250_op_V_XNOR_B32,"V_XNOR_B32"}, // 30
                {amdgpu_gfx1250_op_V_LSHLREV_B64,"V_LSHLREV_B64"}, // 31
                {amdgpu_gfx1250_op_V_ADD_CO_CI_U32,"V_ADD_CO_CI_U32"}, // 32
                {amdgpu_gfx1250_op_V_SUB_CO_CI_U32,"V_SUB_CO_CI_U32"}, // 33
                {amdgpu_gfx1250_op_V_SUBREV_CO_CI_U32,"V_SUBREV_CO_CI_U32"}, // 34
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 35
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 36
                {amdgpu_gfx1250_op_V_ADD_NC_U32,"V_ADD_NC_U32"}, // 37
                {amdgpu_gfx1250_op_V_SUB_NC_U32,"V_SUB_NC_U32"}, // 38
                {amdgpu_gfx1250_op_V_SUBREV_NC_U32,"V_SUBREV_NC_U32"}, // 39
                {amdgpu_gfx1250_op_V_ADD_NC_U64,"V_ADD_NC_U64"}, // 40
                {amdgpu_gfx1250_op_V_SUB_NC_U64,"V_SUB_NC_U64"}, // 41
                {amdgpu_gfx1250_op_V_MUL_U64,"V_MUL_U64"}, // 42
                {amdgpu_gfx1250_op_V_FMAC_F32,"V_FMAC_F32"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_V_CVT_PK_RTZ_F16_F32,"V_CVT_PK_RTZ_F16_F32"}, // 47
                {amdgpu_gfx1250_op_V_MIN_NUM_F16,"V_MIN_NUM_F16"}, // 48
                {amdgpu_gfx1250_op_V_MAX_NUM_F16,"V_MAX_NUM_F16"}, // 49
                {amdgpu_gfx1250_op_V_ADD_F16,"V_ADD_F16"}, // 50
                {amdgpu_gfx1250_op_V_SUB_F16,"V_SUB_F16"}, // 51
                {amdgpu_gfx1250_op_V_SUBREV_F16,"V_SUBREV_F16"}, // 52
                {amdgpu_gfx1250_op_V_MUL_F16,"V_MUL_F16"}, // 53
                {amdgpu_gfx1250_op_V_FMAC_F16,"V_FMAC_F16"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_V_LDEXP_F16,"V_LDEXP_F16"}, // 59
                {amdgpu_gfx1250_op_V_PK_FMAC_F16,"V_PK_FMAC_F16"}, // 60
            }; // end ENC_VOP2_insn_table
            const amdgpu_gfx1250_insn_entry VOP2_INST_LITERAL_insn_table[61] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_V_CNDMASK_B32,"V_CNDMASK_B32"}, // 1
                {amdgpu_gfx1250_op_V_ADD_F64,"V_ADD_F64"}, // 2
                {amdgpu_gfx1250_op_V_ADD_F32,"V_ADD_F32"}, // 3
                {amdgpu_gfx1250_op_V_SUB_F32,"V_SUB_F32"}, // 4
                {amdgpu_gfx1250_op_V_SUBREV_F32,"V_SUBREV_F32"}, // 5
                {amdgpu_gfx1250_op_V_MUL_F64,"V_MUL_F64"}, // 6
                {amdgpu_gfx1250_op_V_MUL_DX9_ZERO_F32,"V_MUL_DX9_ZERO_F32"}, // 7
                {amdgpu_gfx1250_op_V_MUL_F32,"V_MUL_F32"}, // 8
                {amdgpu_gfx1250_op_V_MUL_I32_I24,"V_MUL_I32_I24"}, // 9
                {amdgpu_gfx1250_op_V_MUL_HI_I32_I24,"V_MUL_HI_I32_I24"}, // 10
                {amdgpu_gfx1250_op_V_MUL_U32_U24,"V_MUL_U32_U24"}, // 11
                {amdgpu_gfx1250_op_V_MUL_HI_U32_U24,"V_MUL_HI_U32_U24"}, // 12
                {amdgpu_gfx1250_op_V_MIN_NUM_F64,"V_MIN_NUM_F64"}, // 13
                {amdgpu_gfx1250_op_V_MAX_NUM_F64,"V_MAX_NUM_F64"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_V_MIN_I32,"V_MIN_I32"}, // 17
                {amdgpu_gfx1250_op_V_MAX_I32,"V_MAX_I32"}, // 18
                {amdgpu_gfx1250_op_V_MIN_U32,"V_MIN_U32"}, // 19
                {amdgpu_gfx1250_op_V_MAX_U32,"V_MAX_U32"}, // 20
                {amdgpu_gfx1250_op_V_MIN_NUM_F32,"V_MIN_NUM_F32"}, // 21
                {amdgpu_gfx1250_op_V_MAX_NUM_F32,"V_MAX_NUM_F32"}, // 22
                {amdgpu_gfx1250_op_V_FMAC_F64,"V_FMAC_F64"}, // 23
                {amdgpu_gfx1250_op_V_LSHLREV_B32,"V_LSHLREV_B32"}, // 24
                {amdgpu_gfx1250_op_V_LSHRREV_B32,"V_LSHRREV_B32"}, // 25
                {amdgpu_gfx1250_op_V_ASHRREV_I32,"V_ASHRREV_I32"}, // 26
                {amdgpu_gfx1250_op_V_AND_B32,"V_AND_B32"}, // 27
                {amdgpu_gfx1250_op_V_OR_B32,"V_OR_B32"}, // 28
                {amdgpu_gfx1250_op_V_XOR_B32,"V_XOR_B32"}, // 29
                {amdgpu_gfx1250_op_V_XNOR_B32,"V_XNOR_B32"}, // 30
                {amdgpu_gfx1250_op_V_LSHLREV_B64,"V_LSHLREV_B64"}, // 31
                {amdgpu_gfx1250_op_V_ADD_CO_CI_U32,"V_ADD_CO_CI_U32"}, // 32
                {amdgpu_gfx1250_op_V_SUB_CO_CI_U32,"V_SUB_CO_CI_U32"}, // 33
                {amdgpu_gfx1250_op_V_SUBREV_CO_CI_U32,"V_SUBREV_CO_CI_U32"}, // 34
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 35
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 36
                {amdgpu_gfx1250_op_V_ADD_NC_U32,"V_ADD_NC_U32"}, // 37
                {amdgpu_gfx1250_op_V_SUB_NC_U32,"V_SUB_NC_U32"}, // 38
                {amdgpu_gfx1250_op_V_SUBREV_NC_U32,"V_SUBREV_NC_U32"}, // 39
                {amdgpu_gfx1250_op_V_ADD_NC_U64,"V_ADD_NC_U64"}, // 40
                {amdgpu_gfx1250_op_V_SUB_NC_U64,"V_SUB_NC_U64"}, // 41
                {amdgpu_gfx1250_op_V_MUL_U64,"V_MUL_U64"}, // 42
                {amdgpu_gfx1250_op_V_FMAC_F32,"V_FMAC_F32"}, // 43
                {amdgpu_gfx1250_op_V_FMAMK_F32,"V_FMAMK_F32"}, // 44
                {amdgpu_gfx1250_op_V_FMAAK_F32,"V_FMAAK_F32"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_V_CVT_PK_RTZ_F16_F32,"V_CVT_PK_RTZ_F16_F32"}, // 47
                {amdgpu_gfx1250_op_V_MIN_NUM_F16,"V_MIN_NUM_F16"}, // 48
                {amdgpu_gfx1250_op_V_MAX_NUM_F16,"V_MAX_NUM_F16"}, // 49
                {amdgpu_gfx1250_op_V_ADD_F16,"V_ADD_F16"}, // 50
                {amdgpu_gfx1250_op_V_SUB_F16,"V_SUB_F16"}, // 51
                {amdgpu_gfx1250_op_V_SUBREV_F16,"V_SUBREV_F16"}, // 52
                {amdgpu_gfx1250_op_V_MUL_F16,"V_MUL_F16"}, // 53
                {amdgpu_gfx1250_op_V_FMAC_F16,"V_FMAC_F16"}, // 54
                {amdgpu_gfx1250_op_V_FMAMK_F16,"V_FMAMK_F16"}, // 55
                {amdgpu_gfx1250_op_V_FMAAK_F16,"V_FMAAK_F16"}, // 56
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_V_LDEXP_F16,"V_LDEXP_F16"}, // 59
                {amdgpu_gfx1250_op_V_PK_FMAC_F16,"V_PK_FMAC_F16"}, // 60
            }; // end VOP2_INST_LITERAL_insn_table
            const amdgpu_gfx1250_insn_entry VOP2_INST_LITERAL64_insn_table[61] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_V_CNDMASK_B32,"V_CNDMASK_B32"}, // 1
                {amdgpu_gfx1250_op_V_ADD_F64,"V_ADD_F64"}, // 2
                {amdgpu_gfx1250_op_V_ADD_F32,"V_ADD_F32"}, // 3
                {amdgpu_gfx1250_op_V_SUB_F32,"V_SUB_F32"}, // 4
                {amdgpu_gfx1250_op_V_SUBREV_F32,"V_SUBREV_F32"}, // 5
                {amdgpu_gfx1250_op_V_MUL_F64,"V_MUL_F64"}, // 6
                {amdgpu_gfx1250_op_V_MUL_DX9_ZERO_F32,"V_MUL_DX9_ZERO_F32"}, // 7
                {amdgpu_gfx1250_op_V_MUL_F32,"V_MUL_F32"}, // 8
                {amdgpu_gfx1250_op_V_MUL_I32_I24,"V_MUL_I32_I24"}, // 9
                {amdgpu_gfx1250_op_V_MUL_HI_I32_I24,"V_MUL_HI_I32_I24"}, // 10
                {amdgpu_gfx1250_op_V_MUL_U32_U24,"V_MUL_U32_U24"}, // 11
                {amdgpu_gfx1250_op_V_MUL_HI_U32_U24,"V_MUL_HI_U32_U24"}, // 12
                {amdgpu_gfx1250_op_V_MIN_NUM_F64,"V_MIN_NUM_F64"}, // 13
                {amdgpu_gfx1250_op_V_MAX_NUM_F64,"V_MAX_NUM_F64"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_V_MIN_I32,"V_MIN_I32"}, // 17
                {amdgpu_gfx1250_op_V_MAX_I32,"V_MAX_I32"}, // 18
                {amdgpu_gfx1250_op_V_MIN_U32,"V_MIN_U32"}, // 19
                {amdgpu_gfx1250_op_V_MAX_U32,"V_MAX_U32"}, // 20
                {amdgpu_gfx1250_op_V_MIN_NUM_F32,"V_MIN_NUM_F32"}, // 21
                {amdgpu_gfx1250_op_V_MAX_NUM_F32,"V_MAX_NUM_F32"}, // 22
                {amdgpu_gfx1250_op_V_FMAC_F64,"V_FMAC_F64"}, // 23
                {amdgpu_gfx1250_op_V_LSHLREV_B32,"V_LSHLREV_B32"}, // 24
                {amdgpu_gfx1250_op_V_LSHRREV_B32,"V_LSHRREV_B32"}, // 25
                {amdgpu_gfx1250_op_V_ASHRREV_I32,"V_ASHRREV_I32"}, // 26
                {amdgpu_gfx1250_op_V_AND_B32,"V_AND_B32"}, // 27
                {amdgpu_gfx1250_op_V_OR_B32,"V_OR_B32"}, // 28
                {amdgpu_gfx1250_op_V_XOR_B32,"V_XOR_B32"}, // 29
                {amdgpu_gfx1250_op_V_XNOR_B32,"V_XNOR_B32"}, // 30
                {amdgpu_gfx1250_op_V_LSHLREV_B64,"V_LSHLREV_B64"}, // 31
                {amdgpu_gfx1250_op_V_ADD_CO_CI_U32,"V_ADD_CO_CI_U32"}, // 32
                {amdgpu_gfx1250_op_V_SUB_CO_CI_U32,"V_SUB_CO_CI_U32"}, // 33
                {amdgpu_gfx1250_op_V_SUBREV_CO_CI_U32,"V_SUBREV_CO_CI_U32"}, // 34
                {amdgpu_gfx1250_op_V_FMAMK_F64,"V_FMAMK_F64"}, // 35
                {amdgpu_gfx1250_op_V_FMAAK_F64,"V_FMAAK_F64"}, // 36
                {amdgpu_gfx1250_op_V_ADD_NC_U32,"V_ADD_NC_U32"}, // 37
                {amdgpu_gfx1250_op_V_SUB_NC_U32,"V_SUB_NC_U32"}, // 38
                {amdgpu_gfx1250_op_V_SUBREV_NC_U32,"V_SUBREV_NC_U32"}, // 39
                {amdgpu_gfx1250_op_V_ADD_NC_U64,"V_ADD_NC_U64"}, // 40
                {amdgpu_gfx1250_op_V_SUB_NC_U64,"V_SUB_NC_U64"}, // 41
                {amdgpu_gfx1250_op_V_MUL_U64,"V_MUL_U64"}, // 42
                {amdgpu_gfx1250_op_V_FMAC_F32,"V_FMAC_F32"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_V_CVT_PK_RTZ_F16_F32,"V_CVT_PK_RTZ_F16_F32"}, // 47
                {amdgpu_gfx1250_op_V_MIN_NUM_F16,"V_MIN_NUM_F16"}, // 48
                {amdgpu_gfx1250_op_V_MAX_NUM_F16,"V_MAX_NUM_F16"}, // 49
                {amdgpu_gfx1250_op_V_ADD_F16,"V_ADD_F16"}, // 50
                {amdgpu_gfx1250_op_V_SUB_F16,"V_SUB_F16"}, // 51
                {amdgpu_gfx1250_op_V_SUBREV_F16,"V_SUBREV_F16"}, // 52
                {amdgpu_gfx1250_op_V_MUL_F16,"V_MUL_F16"}, // 53
                {amdgpu_gfx1250_op_V_FMAC_F16,"V_FMAC_F16"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_V_LDEXP_F16,"V_LDEXP_F16"}, // 59
                {amdgpu_gfx1250_op_V_PK_FMAC_F16,"V_PK_FMAC_F16"}, // 60
            }; // end VOP2_INST_LITERAL64_insn_table
            const amdgpu_gfx1250_insn_entry VOP2_VOP_DPP8_insn_table[60] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_V_CNDMASK_B32,"V_CNDMASK_B32"}, // 1
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 2
                {amdgpu_gfx1250_op_V_ADD_F32,"V_ADD_F32"}, // 3
                {amdgpu_gfx1250_op_V_SUB_F32,"V_SUB_F32"}, // 4
                {amdgpu_gfx1250_op_V_SUBREV_F32,"V_SUBREV_F32"}, // 5
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 6
                {amdgpu_gfx1250_op_V_MUL_DX9_ZERO_F32,"V_MUL_DX9_ZERO_F32"}, // 7
                {amdgpu_gfx1250_op_V_MUL_F32,"V_MUL_F32"}, // 8
                {amdgpu_gfx1250_op_V_MUL_I32_I24,"V_MUL_I32_I24"}, // 9
                {amdgpu_gfx1250_op_V_MUL_HI_I32_I24,"V_MUL_HI_I32_I24"}, // 10
                {amdgpu_gfx1250_op_V_MUL_U32_U24,"V_MUL_U32_U24"}, // 11
                {amdgpu_gfx1250_op_V_MUL_HI_U32_U24,"V_MUL_HI_U32_U24"}, // 12
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 13
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_V_MIN_I32,"V_MIN_I32"}, // 17
                {amdgpu_gfx1250_op_V_MAX_I32,"V_MAX_I32"}, // 18
                {amdgpu_gfx1250_op_V_MIN_U32,"V_MIN_U32"}, // 19
                {amdgpu_gfx1250_op_V_MAX_U32,"V_MAX_U32"}, // 20
                {amdgpu_gfx1250_op_V_MIN_NUM_F32,"V_MIN_NUM_F32"}, // 21
                {amdgpu_gfx1250_op_V_MAX_NUM_F32,"V_MAX_NUM_F32"}, // 22
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 23
                {amdgpu_gfx1250_op_V_LSHLREV_B32,"V_LSHLREV_B32"}, // 24
                {amdgpu_gfx1250_op_V_LSHRREV_B32,"V_LSHRREV_B32"}, // 25
                {amdgpu_gfx1250_op_V_ASHRREV_I32,"V_ASHRREV_I32"}, // 26
                {amdgpu_gfx1250_op_V_AND_B32,"V_AND_B32"}, // 27
                {amdgpu_gfx1250_op_V_OR_B32,"V_OR_B32"}, // 28
                {amdgpu_gfx1250_op_V_XOR_B32,"V_XOR_B32"}, // 29
                {amdgpu_gfx1250_op_V_XNOR_B32,"V_XNOR_B32"}, // 30
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 31
                {amdgpu_gfx1250_op_V_ADD_CO_CI_U32,"V_ADD_CO_CI_U32"}, // 32
                {amdgpu_gfx1250_op_V_SUB_CO_CI_U32,"V_SUB_CO_CI_U32"}, // 33
                {amdgpu_gfx1250_op_V_SUBREV_CO_CI_U32,"V_SUBREV_CO_CI_U32"}, // 34
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 35
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 36
                {amdgpu_gfx1250_op_V_ADD_NC_U32,"V_ADD_NC_U32"}, // 37
                {amdgpu_gfx1250_op_V_SUB_NC_U32,"V_SUB_NC_U32"}, // 38
                {amdgpu_gfx1250_op_V_SUBREV_NC_U32,"V_SUBREV_NC_U32"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 42
                {amdgpu_gfx1250_op_V_FMAC_F32,"V_FMAC_F32"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_V_CVT_PK_RTZ_F16_F32,"V_CVT_PK_RTZ_F16_F32"}, // 47
                {amdgpu_gfx1250_op_V_MIN_NUM_F16,"V_MIN_NUM_F16"}, // 48
                {amdgpu_gfx1250_op_V_MAX_NUM_F16,"V_MAX_NUM_F16"}, // 49
                {amdgpu_gfx1250_op_V_ADD_F16,"V_ADD_F16"}, // 50
                {amdgpu_gfx1250_op_V_SUB_F16,"V_SUB_F16"}, // 51
                {amdgpu_gfx1250_op_V_SUBREV_F16,"V_SUBREV_F16"}, // 52
                {amdgpu_gfx1250_op_V_MUL_F16,"V_MUL_F16"}, // 53
                {amdgpu_gfx1250_op_V_FMAC_F16,"V_FMAC_F16"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_V_LDEXP_F16,"V_LDEXP_F16"}, // 59
            }; // end VOP2_VOP_DPP8_insn_table
            const amdgpu_gfx1250_insn_entry VOP2_VOP_DPP16_insn_table[60] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_V_CNDMASK_B32,"V_CNDMASK_B32"}, // 1
                {amdgpu_gfx1250_op_V_ADD_F64,"V_ADD_F64"}, // 2
                {amdgpu_gfx1250_op_V_ADD_F32,"V_ADD_F32"}, // 3
                {amdgpu_gfx1250_op_V_SUB_F32,"V_SUB_F32"}, // 4
                {amdgpu_gfx1250_op_V_SUBREV_F32,"V_SUBREV_F32"}, // 5
                {amdgpu_gfx1250_op_V_MUL_F64,"V_MUL_F64"}, // 6
                {amdgpu_gfx1250_op_V_MUL_DX9_ZERO_F32,"V_MUL_DX9_ZERO_F32"}, // 7
                {amdgpu_gfx1250_op_V_MUL_F32,"V_MUL_F32"}, // 8
                {amdgpu_gfx1250_op_V_MUL_I32_I24,"V_MUL_I32_I24"}, // 9
                {amdgpu_gfx1250_op_V_MUL_HI_I32_I24,"V_MUL_HI_I32_I24"}, // 10
                {amdgpu_gfx1250_op_V_MUL_U32_U24,"V_MUL_U32_U24"}, // 11
                {amdgpu_gfx1250_op_V_MUL_HI_U32_U24,"V_MUL_HI_U32_U24"}, // 12
                {amdgpu_gfx1250_op_V_MIN_NUM_F64,"V_MIN_NUM_F64"}, // 13
                {amdgpu_gfx1250_op_V_MAX_NUM_F64,"V_MAX_NUM_F64"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_V_MIN_I32,"V_MIN_I32"}, // 17
                {amdgpu_gfx1250_op_V_MAX_I32,"V_MAX_I32"}, // 18
                {amdgpu_gfx1250_op_V_MIN_U32,"V_MIN_U32"}, // 19
                {amdgpu_gfx1250_op_V_MAX_U32,"V_MAX_U32"}, // 20
                {amdgpu_gfx1250_op_V_MIN_NUM_F32,"V_MIN_NUM_F32"}, // 21
                {amdgpu_gfx1250_op_V_MAX_NUM_F32,"V_MAX_NUM_F32"}, // 22
                {amdgpu_gfx1250_op_V_FMAC_F64,"V_FMAC_F64"}, // 23
                {amdgpu_gfx1250_op_V_LSHLREV_B32,"V_LSHLREV_B32"}, // 24
                {amdgpu_gfx1250_op_V_LSHRREV_B32,"V_LSHRREV_B32"}, // 25
                {amdgpu_gfx1250_op_V_ASHRREV_I32,"V_ASHRREV_I32"}, // 26
                {amdgpu_gfx1250_op_V_AND_B32,"V_AND_B32"}, // 27
                {amdgpu_gfx1250_op_V_OR_B32,"V_OR_B32"}, // 28
                {amdgpu_gfx1250_op_V_XOR_B32,"V_XOR_B32"}, // 29
                {amdgpu_gfx1250_op_V_XNOR_B32,"V_XNOR_B32"}, // 30
                {amdgpu_gfx1250_op_V_LSHLREV_B64,"V_LSHLREV_B64"}, // 31
                {amdgpu_gfx1250_op_V_ADD_CO_CI_U32,"V_ADD_CO_CI_U32"}, // 32
                {amdgpu_gfx1250_op_V_SUB_CO_CI_U32,"V_SUB_CO_CI_U32"}, // 33
                {amdgpu_gfx1250_op_V_SUBREV_CO_CI_U32,"V_SUBREV_CO_CI_U32"}, // 34
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 35
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 36
                {amdgpu_gfx1250_op_V_ADD_NC_U32,"V_ADD_NC_U32"}, // 37
                {amdgpu_gfx1250_op_V_SUB_NC_U32,"V_SUB_NC_U32"}, // 38
                {amdgpu_gfx1250_op_V_SUBREV_NC_U32,"V_SUBREV_NC_U32"}, // 39
                {amdgpu_gfx1250_op_V_ADD_NC_U64,"V_ADD_NC_U64"}, // 40
                {amdgpu_gfx1250_op_V_SUB_NC_U64,"V_SUB_NC_U64"}, // 41
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 42
                {amdgpu_gfx1250_op_V_FMAC_F32,"V_FMAC_F32"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_V_CVT_PK_RTZ_F16_F32,"V_CVT_PK_RTZ_F16_F32"}, // 47
                {amdgpu_gfx1250_op_V_MIN_NUM_F16,"V_MIN_NUM_F16"}, // 48
                {amdgpu_gfx1250_op_V_MAX_NUM_F16,"V_MAX_NUM_F16"}, // 49
                {amdgpu_gfx1250_op_V_ADD_F16,"V_ADD_F16"}, // 50
                {amdgpu_gfx1250_op_V_SUB_F16,"V_SUB_F16"}, // 51
                {amdgpu_gfx1250_op_V_SUBREV_F16,"V_SUBREV_F16"}, // 52
                {amdgpu_gfx1250_op_V_MUL_F16,"V_MUL_F16"}, // 53
                {amdgpu_gfx1250_op_V_FMAC_F16,"V_FMAC_F16"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_V_LDEXP_F16,"V_LDEXP_F16"}, // 59
            }; // end VOP2_VOP_DPP16_insn_table
            const amdgpu_gfx1250_insn_entry VOP3_SDST_ENC_insn_table[771] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 1
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 2
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 3
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 4
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 5
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 6
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 7
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 8
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 9
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 10
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 11
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 12
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 13
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 17
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 18
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 19
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 20
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 21
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 22
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 23
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 24
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 25
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 26
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 27
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 28
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 29
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 30
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 31
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 32
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 33
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 34
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 35
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 36
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 42
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 49
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 50
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 51
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 52
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 53
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 59
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 60
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 61
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 64
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 65
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 66
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 67
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 68
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 69
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 72
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 73
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 74
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 75
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 76
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 77
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 80
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 81
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 82
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 83
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 84
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 85
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 86
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 87
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 88
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 89
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 90
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 91
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 92
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 93
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 94
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 95
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 96
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 97
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 98
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 99
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 100
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 101
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 102
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 103
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 104
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 105
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 106
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 107
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 108
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 109
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 110
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 111
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 112
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 113
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 114
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 115
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 116
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 117
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 118
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 119
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 120
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 121
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 122
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 123
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 124
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 125
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 126
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 127
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 128
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 129
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 130
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 131
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 132
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 133
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 134
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 135
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 136
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 137
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 138
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 139
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 140
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 141
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 142
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 143
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 144
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 145
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 146
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 147
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 148
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 149
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 150
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 151
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 152
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 153
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 154
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 155
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 156
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 157
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 158
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 159
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 160
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 161
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 162
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 163
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 164
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 165
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 166
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 167
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 168
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 169
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 170
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 171
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 172
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 173
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 174
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 175
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 176
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 177
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 178
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 179
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 180
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 181
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 182
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 183
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 184
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 185
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 186
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 187
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 188
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 189
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 190
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 191
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 192
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 193
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 194
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 195
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 196
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 197
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 198
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 199
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 200
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 201
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 202
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 203
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 204
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 205
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 206
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 207
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 208
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 209
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 210
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 211
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 212
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 213
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 214
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 215
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 216
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 217
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 218
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 219
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 220
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 221
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 222
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 223
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 224
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 225
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 226
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 227
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 228
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 229
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 230
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 231
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 232
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 233
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 234
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 235
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 236
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 237
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 238
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 239
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 240
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 241
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 242
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 243
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 244
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 245
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 246
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 247
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 248
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 249
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 250
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 251
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 252
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 253
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 254
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 255
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 256
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 257
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 258
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 259
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 260
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 261
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 262
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 263
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 264
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 265
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 266
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 267
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 268
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 269
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 270
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 271
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 272
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 273
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 274
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 275
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 276
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 277
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 278
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 279
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 280
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 281
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 282
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 283
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 284
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 285
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 286
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 287
                {amdgpu_gfx1250_op_V_ADD_CO_CI_U32,"V_ADD_CO_CI_U32"}, // 288
                {amdgpu_gfx1250_op_V_SUB_CO_CI_U32,"V_SUB_CO_CI_U32"}, // 289
                {amdgpu_gfx1250_op_V_SUBREV_CO_CI_U32,"V_SUBREV_CO_CI_U32"}, // 290
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 291
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 292
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 293
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 294
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 295
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 296
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 297
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 298
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 299
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 300
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 301
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 302
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 303
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 304
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 305
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 306
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 307
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 308
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 309
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 310
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 311
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 312
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 313
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 314
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 315
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 316
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 317
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 318
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 319
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 320
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 321
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 322
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 323
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 324
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 325
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 326
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 327
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 328
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 329
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 330
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 331
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 332
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 333
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 334
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 335
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 336
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 337
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 338
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 339
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 340
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 341
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 342
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 343
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 344
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 345
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 346
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 347
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 348
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 349
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 350
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 351
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 352
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 353
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 354
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 355
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 356
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 357
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 358
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 359
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 360
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 361
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 362
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 363
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 364
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 365
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 366
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 367
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 368
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 369
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 370
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 371
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 372
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 373
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 374
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 375
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 376
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 377
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 378
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 379
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 380
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 381
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 382
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 383
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 384
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 385
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 386
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 387
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 388
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 389
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 390
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 391
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 392
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 393
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 394
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 395
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 396
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 397
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 398
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 399
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 400
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 401
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 402
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 403
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 404
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 405
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 406
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 407
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 408
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 409
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 410
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 411
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 412
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 413
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 414
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 415
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 416
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 417
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 418
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 419
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 420
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 421
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 422
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 423
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 424
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 425
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 426
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 427
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 428
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 429
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 430
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 431
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 432
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 433
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 434
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 435
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 436
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 437
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 438
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 439
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 440
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 441
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 442
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 443
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 444
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 445
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 446
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 447
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 448
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 449
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 450
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 451
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 452
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 453
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 454
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 455
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 456
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 457
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 458
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 459
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 460
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 461
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 462
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 463
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 464
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 465
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 466
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 467
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 468
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 469
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 470
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 471
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 472
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 473
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 474
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 475
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 476
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 477
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 478
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 479
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 480
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 481
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 482
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 483
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 484
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 485
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 486
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 487
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 488
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 489
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 490
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 491
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 492
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 493
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 494
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 495
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 496
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 497
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 498
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 499
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 500
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 501
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 502
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 503
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 504
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 505
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 506
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 507
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 508
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 509
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 510
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 511
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 512
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 513
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 514
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 515
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 516
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 517
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 518
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 519
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 520
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 521
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 522
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 523
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 524
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 525
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 526
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 527
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 528
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 529
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 530
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 531
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 532
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 533
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 534
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 535
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 536
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 537
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 538
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 539
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 540
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 541
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 542
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 543
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 544
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 545
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 546
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 547
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 548
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 549
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 550
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 551
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 552
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 553
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 554
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 555
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 556
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 557
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 558
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 559
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 560
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 561
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 562
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 563
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 564
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 565
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 566
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 567
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 568
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 569
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 570
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 571
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 572
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 573
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 574
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 575
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 576
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 577
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 578
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 579
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 580
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 581
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 582
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 583
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 584
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 585
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 586
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 587
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 588
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 589
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 590
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 591
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 592
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 593
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 594
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 595
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 596
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 597
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 598
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 599
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 600
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 601
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 602
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 603
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 604
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 605
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 606
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 607
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 608
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 609
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 610
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 611
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 612
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 613
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 614
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 615
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 616
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 617
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 618
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 619
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 620
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 621
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 622
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 623
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 624
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 625
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 626
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 627
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 628
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 629
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 630
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 631
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 632
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 633
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 634
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 635
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 636
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 637
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 638
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 639
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 640
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 641
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 642
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 643
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 644
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 645
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 646
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 647
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 648
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 649
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 650
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 651
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 652
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 653
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 654
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 655
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 656
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 657
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 658
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 659
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 660
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 661
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 662
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 663
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 664
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 665
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 666
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 667
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 668
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 669
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 670
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 671
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 672
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 673
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 674
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 675
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 676
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 677
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 678
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 679
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 680
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 681
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 682
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 683
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 684
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 685
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 686
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 687
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 688
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 689
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 690
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 691
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 692
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 693
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 694
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 695
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 696
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 697
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 698
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 699
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 700
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 701
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 702
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 703
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 704
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 705
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 706
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 707
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 708
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 709
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 710
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 711
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 712
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 713
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 714
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 715
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 716
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 717
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 718
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 719
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 720
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 721
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 722
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 723
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 724
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 725
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 726
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 727
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 728
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 729
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 730
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 731
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 732
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 733
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 734
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 735
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 736
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 737
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 738
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 739
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 740
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 741
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 742
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 743
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 744
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 745
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 746
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 747
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 748
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 749
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 750
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 751
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 752
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 753
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 754
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 755
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 756
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 757
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 758
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 759
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 760
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 761
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 762
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 763
                {amdgpu_gfx1250_op_V_DIV_SCALE_F32,"V_DIV_SCALE_F32"}, // 764
                {amdgpu_gfx1250_op_V_DIV_SCALE_F64,"V_DIV_SCALE_F64"}, // 765
                {amdgpu_gfx1250_op_V_MAD_CO_U64_U32,"V_MAD_CO_U64_U32"}, // 766
                {amdgpu_gfx1250_op_V_MAD_CO_I64_I32,"V_MAD_CO_I64_I32"}, // 767
                {amdgpu_gfx1250_op_V_ADD_CO_U32,"V_ADD_CO_U32"}, // 768
                {amdgpu_gfx1250_op_V_SUB_CO_U32,"V_SUB_CO_U32"}, // 769
                {amdgpu_gfx1250_op_V_SUBREV_CO_U32,"V_SUBREV_CO_U32"}, // 770
            }; // end VOP3_SDST_ENC_insn_table
            const amdgpu_gfx1250_insn_entry VOP3_SDST_ENC_INST_LITERAL_insn_table[771] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 1
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 2
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 3
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 4
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 5
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 6
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 7
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 8
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 9
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 10
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 11
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 12
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 13
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 17
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 18
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 19
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 20
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 21
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 22
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 23
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 24
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 25
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 26
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 27
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 28
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 29
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 30
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 31
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 32
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 33
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 34
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 35
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 36
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 42
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 49
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 50
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 51
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 52
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 53
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 59
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 60
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 61
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 64
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 65
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 66
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 67
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 68
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 69
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 72
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 73
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 74
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 75
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 76
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 77
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 80
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 81
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 82
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 83
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 84
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 85
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 86
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 87
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 88
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 89
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 90
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 91
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 92
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 93
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 94
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 95
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 96
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 97
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 98
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 99
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 100
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 101
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 102
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 103
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 104
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 105
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 106
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 107
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 108
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 109
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 110
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 111
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 112
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 113
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 114
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 115
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 116
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 117
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 118
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 119
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 120
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 121
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 122
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 123
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 124
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 125
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 126
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 127
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 128
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 129
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 130
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 131
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 132
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 133
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 134
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 135
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 136
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 137
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 138
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 139
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 140
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 141
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 142
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 143
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 144
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 145
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 146
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 147
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 148
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 149
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 150
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 151
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 152
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 153
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 154
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 155
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 156
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 157
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 158
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 159
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 160
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 161
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 162
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 163
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 164
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 165
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 166
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 167
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 168
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 169
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 170
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 171
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 172
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 173
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 174
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 175
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 176
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 177
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 178
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 179
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 180
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 181
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 182
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 183
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 184
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 185
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 186
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 187
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 188
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 189
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 190
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 191
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 192
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 193
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 194
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 195
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 196
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 197
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 198
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 199
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 200
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 201
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 202
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 203
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 204
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 205
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 206
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 207
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 208
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 209
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 210
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 211
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 212
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 213
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 214
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 215
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 216
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 217
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 218
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 219
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 220
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 221
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 222
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 223
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 224
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 225
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 226
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 227
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 228
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 229
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 230
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 231
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 232
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 233
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 234
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 235
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 236
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 237
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 238
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 239
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 240
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 241
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 242
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 243
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 244
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 245
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 246
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 247
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 248
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 249
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 250
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 251
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 252
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 253
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 254
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 255
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 256
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 257
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 258
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 259
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 260
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 261
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 262
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 263
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 264
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 265
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 266
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 267
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 268
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 269
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 270
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 271
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 272
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 273
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 274
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 275
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 276
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 277
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 278
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 279
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 280
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 281
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 282
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 283
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 284
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 285
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 286
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 287
                {amdgpu_gfx1250_op_V_ADD_CO_CI_U32,"V_ADD_CO_CI_U32"}, // 288
                {amdgpu_gfx1250_op_V_SUB_CO_CI_U32,"V_SUB_CO_CI_U32"}, // 289
                {amdgpu_gfx1250_op_V_SUBREV_CO_CI_U32,"V_SUBREV_CO_CI_U32"}, // 290
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 291
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 292
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 293
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 294
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 295
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 296
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 297
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 298
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 299
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 300
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 301
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 302
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 303
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 304
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 305
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 306
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 307
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 308
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 309
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 310
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 311
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 312
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 313
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 314
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 315
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 316
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 317
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 318
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 319
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 320
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 321
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 322
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 323
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 324
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 325
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 326
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 327
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 328
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 329
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 330
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 331
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 332
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 333
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 334
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 335
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 336
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 337
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 338
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 339
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 340
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 341
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 342
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 343
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 344
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 345
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 346
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 347
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 348
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 349
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 350
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 351
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 352
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 353
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 354
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 355
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 356
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 357
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 358
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 359
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 360
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 361
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 362
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 363
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 364
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 365
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 366
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 367
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 368
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 369
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 370
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 371
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 372
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 373
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 374
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 375
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 376
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 377
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 378
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 379
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 380
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 381
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 382
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 383
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 384
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 385
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 386
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 387
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 388
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 389
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 390
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 391
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 392
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 393
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 394
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 395
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 396
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 397
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 398
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 399
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 400
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 401
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 402
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 403
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 404
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 405
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 406
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 407
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 408
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 409
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 410
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 411
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 412
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 413
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 414
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 415
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 416
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 417
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 418
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 419
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 420
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 421
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 422
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 423
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 424
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 425
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 426
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 427
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 428
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 429
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 430
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 431
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 432
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 433
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 434
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 435
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 436
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 437
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 438
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 439
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 440
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 441
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 442
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 443
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 444
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 445
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 446
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 447
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 448
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 449
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 450
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 451
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 452
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 453
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 454
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 455
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 456
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 457
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 458
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 459
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 460
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 461
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 462
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 463
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 464
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 465
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 466
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 467
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 468
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 469
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 470
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 471
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 472
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 473
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 474
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 475
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 476
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 477
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 478
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 479
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 480
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 481
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 482
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 483
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 484
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 485
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 486
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 487
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 488
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 489
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 490
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 491
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 492
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 493
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 494
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 495
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 496
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 497
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 498
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 499
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 500
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 501
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 502
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 503
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 504
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 505
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 506
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 507
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 508
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 509
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 510
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 511
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 512
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 513
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 514
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 515
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 516
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 517
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 518
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 519
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 520
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 521
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 522
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 523
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 524
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 525
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 526
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 527
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 528
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 529
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 530
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 531
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 532
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 533
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 534
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 535
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 536
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 537
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 538
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 539
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 540
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 541
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 542
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 543
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 544
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 545
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 546
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 547
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 548
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 549
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 550
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 551
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 552
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 553
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 554
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 555
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 556
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 557
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 558
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 559
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 560
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 561
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 562
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 563
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 564
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 565
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 566
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 567
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 568
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 569
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 570
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 571
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 572
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 573
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 574
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 575
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 576
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 577
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 578
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 579
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 580
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 581
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 582
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 583
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 584
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 585
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 586
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 587
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 588
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 589
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 590
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 591
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 592
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 593
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 594
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 595
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 596
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 597
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 598
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 599
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 600
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 601
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 602
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 603
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 604
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 605
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 606
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 607
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 608
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 609
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 610
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 611
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 612
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 613
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 614
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 615
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 616
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 617
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 618
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 619
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 620
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 621
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 622
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 623
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 624
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 625
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 626
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 627
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 628
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 629
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 630
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 631
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 632
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 633
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 634
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 635
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 636
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 637
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 638
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 639
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 640
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 641
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 642
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 643
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 644
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 645
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 646
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 647
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 648
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 649
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 650
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 651
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 652
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 653
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 654
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 655
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 656
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 657
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 658
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 659
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 660
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 661
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 662
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 663
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 664
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 665
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 666
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 667
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 668
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 669
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 670
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 671
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 672
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 673
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 674
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 675
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 676
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 677
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 678
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 679
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 680
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 681
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 682
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 683
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 684
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 685
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 686
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 687
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 688
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 689
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 690
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 691
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 692
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 693
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 694
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 695
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 696
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 697
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 698
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 699
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 700
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 701
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 702
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 703
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 704
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 705
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 706
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 707
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 708
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 709
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 710
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 711
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 712
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 713
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 714
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 715
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 716
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 717
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 718
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 719
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 720
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 721
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 722
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 723
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 724
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 725
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 726
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 727
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 728
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 729
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 730
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 731
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 732
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 733
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 734
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 735
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 736
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 737
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 738
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 739
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 740
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 741
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 742
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 743
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 744
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 745
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 746
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 747
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 748
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 749
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 750
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 751
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 752
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 753
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 754
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 755
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 756
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 757
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 758
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 759
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 760
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 761
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 762
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 763
                {amdgpu_gfx1250_op_V_DIV_SCALE_F32,"V_DIV_SCALE_F32"}, // 764
                {amdgpu_gfx1250_op_V_DIV_SCALE_F64,"V_DIV_SCALE_F64"}, // 765
                {amdgpu_gfx1250_op_V_MAD_CO_U64_U32,"V_MAD_CO_U64_U32"}, // 766
                {amdgpu_gfx1250_op_V_MAD_CO_I64_I32,"V_MAD_CO_I64_I32"}, // 767
                {amdgpu_gfx1250_op_V_ADD_CO_U32,"V_ADD_CO_U32"}, // 768
                {amdgpu_gfx1250_op_V_SUB_CO_U32,"V_SUB_CO_U32"}, // 769
                {amdgpu_gfx1250_op_V_SUBREV_CO_U32,"V_SUBREV_CO_U32"}, // 770
            }; // end VOP3_SDST_ENC_INST_LITERAL_insn_table
            const amdgpu_gfx1250_insn_entry VOP3_SDST_ENC_VOP_DPP8_insn_table[771] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 1
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 2
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 3
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 4
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 5
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 6
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 7
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 8
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 9
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 10
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 11
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 12
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 13
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 17
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 18
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 19
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 20
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 21
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 22
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 23
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 24
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 25
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 26
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 27
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 28
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 29
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 30
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 31
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 32
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 33
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 34
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 35
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 36
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 42
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 49
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 50
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 51
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 52
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 53
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 59
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 60
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 61
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 64
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 65
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 66
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 67
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 68
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 69
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 72
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 73
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 74
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 75
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 76
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 77
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 80
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 81
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 82
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 83
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 84
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 85
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 86
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 87
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 88
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 89
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 90
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 91
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 92
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 93
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 94
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 95
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 96
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 97
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 98
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 99
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 100
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 101
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 102
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 103
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 104
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 105
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 106
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 107
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 108
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 109
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 110
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 111
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 112
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 113
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 114
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 115
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 116
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 117
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 118
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 119
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 120
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 121
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 122
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 123
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 124
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 125
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 126
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 127
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 128
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 129
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 130
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 131
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 132
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 133
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 134
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 135
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 136
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 137
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 138
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 139
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 140
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 141
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 142
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 143
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 144
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 145
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 146
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 147
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 148
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 149
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 150
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 151
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 152
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 153
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 154
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 155
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 156
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 157
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 158
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 159
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 160
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 161
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 162
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 163
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 164
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 165
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 166
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 167
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 168
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 169
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 170
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 171
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 172
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 173
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 174
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 175
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 176
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 177
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 178
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 179
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 180
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 181
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 182
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 183
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 184
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 185
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 186
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 187
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 188
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 189
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 190
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 191
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 192
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 193
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 194
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 195
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 196
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 197
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 198
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 199
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 200
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 201
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 202
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 203
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 204
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 205
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 206
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 207
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 208
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 209
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 210
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 211
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 212
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 213
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 214
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 215
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 216
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 217
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 218
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 219
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 220
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 221
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 222
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 223
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 224
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 225
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 226
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 227
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 228
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 229
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 230
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 231
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 232
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 233
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 234
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 235
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 236
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 237
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 238
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 239
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 240
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 241
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 242
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 243
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 244
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 245
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 246
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 247
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 248
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 249
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 250
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 251
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 252
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 253
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 254
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 255
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 256
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 257
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 258
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 259
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 260
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 261
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 262
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 263
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 264
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 265
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 266
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 267
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 268
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 269
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 270
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 271
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 272
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 273
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 274
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 275
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 276
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 277
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 278
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 279
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 280
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 281
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 282
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 283
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 284
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 285
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 286
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 287
                {amdgpu_gfx1250_op_V_ADD_CO_CI_U32,"V_ADD_CO_CI_U32"}, // 288
                {amdgpu_gfx1250_op_V_SUB_CO_CI_U32,"V_SUB_CO_CI_U32"}, // 289
                {amdgpu_gfx1250_op_V_SUBREV_CO_CI_U32,"V_SUBREV_CO_CI_U32"}, // 290
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 291
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 292
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 293
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 294
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 295
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 296
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 297
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 298
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 299
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 300
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 301
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 302
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 303
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 304
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 305
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 306
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 307
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 308
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 309
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 310
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 311
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 312
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 313
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 314
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 315
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 316
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 317
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 318
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 319
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 320
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 321
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 322
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 323
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 324
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 325
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 326
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 327
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 328
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 329
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 330
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 331
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 332
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 333
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 334
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 335
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 336
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 337
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 338
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 339
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 340
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 341
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 342
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 343
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 344
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 345
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 346
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 347
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 348
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 349
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 350
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 351
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 352
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 353
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 354
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 355
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 356
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 357
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 358
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 359
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 360
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 361
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 362
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 363
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 364
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 365
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 366
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 367
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 368
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 369
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 370
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 371
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 372
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 373
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 374
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 375
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 376
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 377
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 378
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 379
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 380
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 381
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 382
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 383
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 384
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 385
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 386
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 387
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 388
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 389
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 390
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 391
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 392
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 393
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 394
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 395
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 396
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 397
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 398
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 399
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 400
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 401
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 402
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 403
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 404
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 405
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 406
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 407
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 408
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 409
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 410
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 411
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 412
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 413
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 414
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 415
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 416
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 417
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 418
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 419
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 420
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 421
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 422
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 423
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 424
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 425
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 426
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 427
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 428
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 429
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 430
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 431
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 432
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 433
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 434
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 435
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 436
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 437
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 438
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 439
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 440
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 441
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 442
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 443
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 444
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 445
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 446
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 447
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 448
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 449
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 450
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 451
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 452
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 453
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 454
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 455
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 456
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 457
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 458
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 459
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 460
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 461
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 462
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 463
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 464
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 465
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 466
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 467
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 468
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 469
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 470
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 471
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 472
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 473
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 474
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 475
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 476
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 477
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 478
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 479
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 480
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 481
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 482
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 483
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 484
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 485
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 486
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 487
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 488
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 489
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 490
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 491
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 492
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 493
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 494
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 495
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 496
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 497
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 498
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 499
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 500
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 501
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 502
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 503
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 504
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 505
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 506
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 507
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 508
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 509
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 510
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 511
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 512
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 513
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 514
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 515
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 516
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 517
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 518
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 519
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 520
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 521
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 522
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 523
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 524
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 525
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 526
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 527
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 528
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 529
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 530
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 531
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 532
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 533
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 534
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 535
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 536
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 537
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 538
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 539
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 540
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 541
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 542
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 543
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 544
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 545
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 546
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 547
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 548
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 549
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 550
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 551
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 552
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 553
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 554
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 555
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 556
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 557
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 558
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 559
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 560
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 561
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 562
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 563
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 564
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 565
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 566
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 567
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 568
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 569
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 570
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 571
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 572
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 573
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 574
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 575
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 576
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 577
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 578
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 579
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 580
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 581
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 582
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 583
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 584
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 585
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 586
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 587
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 588
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 589
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 590
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 591
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 592
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 593
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 594
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 595
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 596
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 597
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 598
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 599
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 600
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 601
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 602
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 603
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 604
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 605
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 606
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 607
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 608
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 609
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 610
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 611
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 612
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 613
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 614
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 615
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 616
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 617
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 618
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 619
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 620
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 621
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 622
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 623
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 624
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 625
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 626
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 627
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 628
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 629
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 630
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 631
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 632
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 633
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 634
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 635
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 636
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 637
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 638
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 639
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 640
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 641
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 642
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 643
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 644
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 645
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 646
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 647
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 648
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 649
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 650
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 651
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 652
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 653
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 654
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 655
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 656
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 657
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 658
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 659
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 660
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 661
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 662
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 663
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 664
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 665
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 666
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 667
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 668
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 669
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 670
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 671
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 672
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 673
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 674
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 675
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 676
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 677
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 678
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 679
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 680
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 681
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 682
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 683
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 684
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 685
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 686
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 687
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 688
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 689
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 690
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 691
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 692
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 693
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 694
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 695
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 696
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 697
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 698
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 699
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 700
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 701
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 702
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 703
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 704
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 705
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 706
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 707
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 708
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 709
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 710
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 711
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 712
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 713
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 714
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 715
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 716
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 717
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 718
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 719
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 720
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 721
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 722
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 723
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 724
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 725
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 726
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 727
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 728
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 729
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 730
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 731
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 732
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 733
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 734
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 735
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 736
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 737
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 738
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 739
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 740
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 741
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 742
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 743
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 744
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 745
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 746
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 747
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 748
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 749
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 750
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 751
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 752
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 753
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 754
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 755
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 756
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 757
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 758
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 759
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 760
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 761
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 762
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 763
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 764
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 765
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 766
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 767
                {amdgpu_gfx1250_op_V_ADD_CO_U32,"V_ADD_CO_U32"}, // 768
                {amdgpu_gfx1250_op_V_SUB_CO_U32,"V_SUB_CO_U32"}, // 769
                {amdgpu_gfx1250_op_V_SUBREV_CO_U32,"V_SUBREV_CO_U32"}, // 770
            }; // end VOP3_SDST_ENC_VOP_DPP8_insn_table
            const amdgpu_gfx1250_insn_entry VOP3_SDST_ENC_VOP_DPP16_insn_table[771] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 1
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 2
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 3
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 4
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 5
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 6
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 7
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 8
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 9
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 10
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 11
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 12
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 13
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 17
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 18
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 19
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 20
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 21
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 22
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 23
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 24
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 25
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 26
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 27
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 28
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 29
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 30
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 31
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 32
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 33
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 34
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 35
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 36
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 42
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 49
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 50
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 51
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 52
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 53
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 59
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 60
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 61
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 64
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 65
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 66
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 67
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 68
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 69
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 72
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 73
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 74
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 75
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 76
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 77
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 80
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 81
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 82
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 83
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 84
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 85
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 86
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 87
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 88
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 89
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 90
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 91
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 92
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 93
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 94
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 95
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 96
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 97
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 98
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 99
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 100
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 101
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 102
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 103
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 104
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 105
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 106
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 107
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 108
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 109
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 110
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 111
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 112
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 113
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 114
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 115
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 116
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 117
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 118
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 119
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 120
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 121
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 122
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 123
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 124
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 125
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 126
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 127
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 128
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 129
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 130
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 131
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 132
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 133
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 134
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 135
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 136
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 137
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 138
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 139
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 140
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 141
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 142
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 143
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 144
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 145
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 146
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 147
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 148
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 149
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 150
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 151
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 152
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 153
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 154
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 155
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 156
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 157
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 158
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 159
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 160
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 161
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 162
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 163
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 164
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 165
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 166
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 167
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 168
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 169
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 170
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 171
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 172
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 173
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 174
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 175
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 176
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 177
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 178
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 179
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 180
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 181
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 182
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 183
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 184
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 185
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 186
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 187
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 188
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 189
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 190
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 191
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 192
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 193
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 194
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 195
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 196
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 197
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 198
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 199
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 200
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 201
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 202
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 203
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 204
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 205
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 206
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 207
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 208
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 209
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 210
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 211
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 212
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 213
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 214
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 215
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 216
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 217
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 218
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 219
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 220
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 221
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 222
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 223
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 224
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 225
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 226
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 227
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 228
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 229
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 230
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 231
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 232
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 233
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 234
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 235
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 236
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 237
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 238
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 239
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 240
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 241
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 242
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 243
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 244
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 245
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 246
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 247
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 248
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 249
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 250
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 251
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 252
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 253
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 254
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 255
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 256
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 257
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 258
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 259
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 260
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 261
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 262
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 263
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 264
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 265
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 266
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 267
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 268
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 269
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 270
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 271
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 272
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 273
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 274
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 275
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 276
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 277
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 278
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 279
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 280
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 281
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 282
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 283
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 284
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 285
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 286
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 287
                {amdgpu_gfx1250_op_V_ADD_CO_CI_U32,"V_ADD_CO_CI_U32"}, // 288
                {amdgpu_gfx1250_op_V_SUB_CO_CI_U32,"V_SUB_CO_CI_U32"}, // 289
                {amdgpu_gfx1250_op_V_SUBREV_CO_CI_U32,"V_SUBREV_CO_CI_U32"}, // 290
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 291
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 292
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 293
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 294
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 295
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 296
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 297
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 298
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 299
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 300
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 301
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 302
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 303
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 304
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 305
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 306
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 307
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 308
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 309
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 310
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 311
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 312
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 313
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 314
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 315
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 316
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 317
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 318
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 319
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 320
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 321
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 322
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 323
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 324
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 325
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 326
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 327
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 328
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 329
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 330
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 331
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 332
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 333
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 334
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 335
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 336
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 337
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 338
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 339
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 340
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 341
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 342
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 343
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 344
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 345
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 346
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 347
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 348
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 349
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 350
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 351
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 352
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 353
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 354
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 355
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 356
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 357
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 358
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 359
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 360
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 361
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 362
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 363
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 364
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 365
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 366
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 367
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 368
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 369
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 370
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 371
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 372
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 373
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 374
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 375
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 376
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 377
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 378
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 379
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 380
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 381
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 382
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 383
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 384
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 385
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 386
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 387
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 388
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 389
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 390
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 391
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 392
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 393
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 394
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 395
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 396
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 397
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 398
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 399
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 400
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 401
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 402
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 403
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 404
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 405
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 406
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 407
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 408
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 409
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 410
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 411
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 412
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 413
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 414
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 415
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 416
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 417
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 418
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 419
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 420
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 421
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 422
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 423
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 424
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 425
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 426
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 427
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 428
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 429
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 430
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 431
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 432
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 433
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 434
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 435
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 436
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 437
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 438
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 439
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 440
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 441
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 442
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 443
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 444
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 445
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 446
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 447
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 448
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 449
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 450
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 451
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 452
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 453
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 454
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 455
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 456
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 457
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 458
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 459
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 460
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 461
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 462
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 463
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 464
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 465
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 466
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 467
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 468
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 469
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 470
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 471
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 472
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 473
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 474
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 475
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 476
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 477
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 478
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 479
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 480
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 481
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 482
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 483
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 484
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 485
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 486
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 487
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 488
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 489
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 490
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 491
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 492
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 493
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 494
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 495
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 496
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 497
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 498
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 499
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 500
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 501
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 502
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 503
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 504
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 505
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 506
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 507
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 508
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 509
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 510
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 511
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 512
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 513
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 514
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 515
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 516
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 517
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 518
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 519
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 520
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 521
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 522
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 523
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 524
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 525
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 526
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 527
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 528
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 529
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 530
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 531
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 532
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 533
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 534
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 535
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 536
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 537
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 538
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 539
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 540
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 541
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 542
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 543
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 544
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 545
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 546
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 547
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 548
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 549
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 550
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 551
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 552
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 553
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 554
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 555
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 556
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 557
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 558
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 559
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 560
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 561
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 562
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 563
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 564
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 565
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 566
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 567
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 568
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 569
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 570
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 571
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 572
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 573
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 574
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 575
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 576
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 577
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 578
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 579
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 580
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 581
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 582
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 583
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 584
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 585
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 586
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 587
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 588
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 589
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 590
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 591
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 592
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 593
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 594
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 595
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 596
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 597
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 598
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 599
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 600
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 601
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 602
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 603
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 604
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 605
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 606
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 607
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 608
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 609
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 610
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 611
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 612
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 613
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 614
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 615
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 616
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 617
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 618
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 619
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 620
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 621
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 622
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 623
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 624
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 625
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 626
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 627
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 628
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 629
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 630
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 631
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 632
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 633
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 634
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 635
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 636
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 637
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 638
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 639
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 640
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 641
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 642
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 643
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 644
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 645
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 646
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 647
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 648
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 649
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 650
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 651
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 652
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 653
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 654
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 655
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 656
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 657
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 658
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 659
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 660
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 661
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 662
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 663
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 664
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 665
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 666
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 667
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 668
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 669
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 670
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 671
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 672
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 673
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 674
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 675
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 676
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 677
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 678
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 679
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 680
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 681
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 682
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 683
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 684
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 685
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 686
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 687
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 688
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 689
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 690
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 691
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 692
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 693
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 694
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 695
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 696
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 697
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 698
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 699
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 700
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 701
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 702
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 703
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 704
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 705
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 706
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 707
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 708
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 709
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 710
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 711
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 712
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 713
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 714
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 715
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 716
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 717
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 718
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 719
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 720
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 721
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 722
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 723
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 724
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 725
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 726
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 727
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 728
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 729
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 730
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 731
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 732
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 733
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 734
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 735
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 736
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 737
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 738
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 739
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 740
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 741
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 742
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 743
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 744
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 745
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 746
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 747
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 748
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 749
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 750
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 751
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 752
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 753
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 754
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 755
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 756
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 757
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 758
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 759
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 760
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 761
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 762
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 763
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 764
                {amdgpu_gfx1250_op_V_DIV_SCALE_F64,"V_DIV_SCALE_F64"}, // 765
                {amdgpu_gfx1250_op_V_MAD_CO_U64_U32,"V_MAD_CO_U64_U32"}, // 766
                {amdgpu_gfx1250_op_V_MAD_CO_I64_I32,"V_MAD_CO_I64_I32"}, // 767
                {amdgpu_gfx1250_op_V_ADD_CO_U32,"V_ADD_CO_U32"}, // 768
                {amdgpu_gfx1250_op_V_SUB_CO_U32,"V_SUB_CO_U32"}, // 769
                {amdgpu_gfx1250_op_V_SUBREV_CO_U32,"V_SUBREV_CO_U32"}, // 770
            }; // end VOP3_SDST_ENC_VOP_DPP16_insn_table
            const amdgpu_gfx1250_insn_entry ENC_VOP3P_insn_table[137] =
            {
                {amdgpu_gfx1250_op_V_PK_MAD_I16,"V_PK_MAD_I16"}, // 0
                {amdgpu_gfx1250_op_V_PK_MUL_LO_U16,"V_PK_MUL_LO_U16"}, // 1
                {amdgpu_gfx1250_op_V_PK_ADD_I16,"V_PK_ADD_I16"}, // 2
                {amdgpu_gfx1250_op_V_PK_SUB_I16,"V_PK_SUB_I16"}, // 3
                {amdgpu_gfx1250_op_V_PK_LSHLREV_B16,"V_PK_LSHLREV_B16"}, // 4
                {amdgpu_gfx1250_op_V_PK_LSHRREV_B16,"V_PK_LSHRREV_B16"}, // 5
                {amdgpu_gfx1250_op_V_PK_ASHRREV_I16,"V_PK_ASHRREV_I16"}, // 6
                {amdgpu_gfx1250_op_V_PK_MAX_I16,"V_PK_MAX_I16"}, // 7
                {amdgpu_gfx1250_op_V_PK_MIN_I16,"V_PK_MIN_I16"}, // 8
                {amdgpu_gfx1250_op_V_PK_MAD_U16,"V_PK_MAD_U16"}, // 9
                {amdgpu_gfx1250_op_V_PK_ADD_U16,"V_PK_ADD_U16"}, // 10
                {amdgpu_gfx1250_op_V_PK_SUB_U16,"V_PK_SUB_U16"}, // 11
                {amdgpu_gfx1250_op_V_PK_MAX_U16,"V_PK_MAX_U16"}, // 12
                {amdgpu_gfx1250_op_V_PK_MIN_U16,"V_PK_MIN_U16"}, // 13
                {amdgpu_gfx1250_op_V_PK_FMA_F16,"V_PK_FMA_F16"}, // 14
                {amdgpu_gfx1250_op_V_PK_ADD_F16,"V_PK_ADD_F16"}, // 15
                {amdgpu_gfx1250_op_V_PK_MUL_F16,"V_PK_MUL_F16"}, // 16
                {amdgpu_gfx1250_op_V_PK_FMA_BF16,"V_PK_FMA_BF16"}, // 17
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 18
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 19
                {amdgpu_gfx1250_op_V_PK_ADD_MAX_I16,"V_PK_ADD_MAX_I16"}, // 20
                {amdgpu_gfx1250_op_V_PK_ADD_MAX_U16,"V_PK_ADD_MAX_U16"}, // 21
                {amdgpu_gfx1250_op_V_DOT4_I32_IU8,"V_DOT4_I32_IU8"}, // 22
                {amdgpu_gfx1250_op_V_DOT4_U32_U8,"V_DOT4_U32_U8"}, // 23
                {amdgpu_gfx1250_op_V_DOT8_I32_IU4,"V_DOT8_I32_IU4"}, // 24
                {amdgpu_gfx1250_op_V_DOT8_U32_U4,"V_DOT8_U32_U4"}, // 25
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 26
                {amdgpu_gfx1250_op_V_PK_MIN_NUM_F16,"V_PK_MIN_NUM_F16"}, // 27
                {amdgpu_gfx1250_op_V_PK_MAX_NUM_F16,"V_PK_MAX_NUM_F16"}, // 28
                {amdgpu_gfx1250_op_V_PK_MINIMUM_F16,"V_PK_MINIMUM_F16"}, // 29
                {amdgpu_gfx1250_op_V_PK_MAXIMUM_F16,"V_PK_MAXIMUM_F16"}, // 30
                {amdgpu_gfx1250_op_V_PK_FMA_F32,"V_PK_FMA_F32"}, // 31
                {amdgpu_gfx1250_op_V_FMA_MIX_F32,"V_FMA_MIX_F32"}, // 32
                {amdgpu_gfx1250_op_V_FMA_MIXLO_F16,"V_FMA_MIXLO_F16"}, // 33
                {amdgpu_gfx1250_op_V_FMA_MIXHI_F16,"V_FMA_MIXHI_F16"}, // 34
                {amdgpu_gfx1250_op_V_PK_ADD_BF16,"V_PK_ADD_BF16"}, // 35
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 36
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 39
                {amdgpu_gfx1250_op_V_PK_MUL_F32,"V_PK_MUL_F32"}, // 40
                {amdgpu_gfx1250_op_V_PK_ADD_F32,"V_PK_ADD_F32"}, // 41
                {amdgpu_gfx1250_op_V_PK_MUL_BF16,"V_PK_MUL_BF16"}, // 42
                {amdgpu_gfx1250_op_V_PK_MIN_NUM_BF16,"V_PK_MIN_NUM_BF16"}, // 43
                {amdgpu_gfx1250_op_V_PK_MAX_NUM_BF16,"V_PK_MAX_NUM_BF16"}, // 44
                {amdgpu_gfx1250_op_V_PK_ADD_MIN_I16,"V_PK_ADD_MIN_I16"}, // 45
                {amdgpu_gfx1250_op_V_PK_ADD_MIN_U16,"V_PK_ADD_MIN_U16"}, // 46
                {amdgpu_gfx1250_op_V_PK_MAX3_I16,"V_PK_MAX3_I16"}, // 47
                {amdgpu_gfx1250_op_V_PK_MAX3_U16,"V_PK_MAX3_U16"}, // 48
                {amdgpu_gfx1250_op_V_PK_MIN3_I16,"V_PK_MIN3_I16"}, // 49
                {amdgpu_gfx1250_op_V_PK_MIN3_U16,"V_PK_MIN3_U16"}, // 50
                {amdgpu_gfx1250_op_V_WMMA_F32_16X16X128_F8F6F4,"V_WMMA_F32_16X16X128_F8F6F4"}, // 51
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 52
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 53
                {amdgpu_gfx1250_op_V_PK_MINIMUM3_F16,"V_PK_MINIMUM3_F16"}, // 54
                {amdgpu_gfx1250_op_V_PK_MAXIMUM3_F16,"V_PK_MAXIMUM3_F16"}, // 55
                {amdgpu_gfx1250_op_V_PK_MIN3_NUM_F16,"V_PK_MIN3_NUM_F16"}, // 56
                {amdgpu_gfx1250_op_V_PK_MAX3_NUM_F16,"V_PK_MAX3_NUM_F16"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 59
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 60
                {amdgpu_gfx1250_op_V_FMA_MIX_F32_BF16,"V_FMA_MIX_F32_BF16"}, // 61
                {amdgpu_gfx1250_op_V_FMA_MIXLO_BF16,"V_FMA_MIXLO_BF16"}, // 62
                {amdgpu_gfx1250_op_V_FMA_MIXHI_BF16,"V_FMA_MIXHI_BF16"}, // 63
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 64
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 65
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 66
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 67
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 68
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 69
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 72
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 73
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 74
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 75
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 76
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 77
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 80
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 81
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 82
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 83
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 84
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 85
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 86
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 87
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 88
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 89
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 90
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 91
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 92
                {amdgpu_gfx1250_op_V_WMMA_F32_16X16X4_F32,"V_WMMA_F32_16X16X4_F32"}, // 93
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 94
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 95
                {amdgpu_gfx1250_op_V_WMMA_F32_16X16X32_F16,"V_WMMA_F32_16X16X32_F16"}, // 96
                {amdgpu_gfx1250_op_V_WMMA_F16_16X16X32_F16,"V_WMMA_F16_16X16X32_F16"}, // 97
                {amdgpu_gfx1250_op_V_WMMA_F32_16X16X32_BF16,"V_WMMA_F32_16X16X32_BF16"}, // 98
                {amdgpu_gfx1250_op_V_WMMA_BF16_16X16X32_BF16,"V_WMMA_BF16_16X16X32_BF16"}, // 99
                {amdgpu_gfx1250_op_V_WMMA_BF16F32_16X16X32_BF16,"V_WMMA_BF16F32_16X16X32_BF16"}, // 100
                {amdgpu_gfx1250_op_V_SWMMAC_F32_16X16X64_F16,"V_SWMMAC_F32_16X16X64_F16"}, // 101
                {amdgpu_gfx1250_op_V_SWMMAC_F32_16X16X64_BF16,"V_SWMMAC_F32_16X16X64_BF16"}, // 102
                {amdgpu_gfx1250_op_V_SWMMAC_F16_16X16X64_F16,"V_SWMMAC_F16_16X16X64_F16"}, // 103
                {amdgpu_gfx1250_op_V_SWMMAC_BF16_16X16X64_BF16,"V_SWMMAC_BF16_16X16X64_BF16"}, // 104
                {amdgpu_gfx1250_op_V_SWMMAC_BF16F32_16X16X64_BF16,"V_SWMMAC_BF16F32_16X16X64_BF16"}, // 105
                {amdgpu_gfx1250_op_V_WMMA_F32_16X16X64_FP8_FP8,"V_WMMA_F32_16X16X64_FP8_FP8"}, // 106
                {amdgpu_gfx1250_op_V_WMMA_F32_16X16X64_FP8_BF8,"V_WMMA_F32_16X16X64_FP8_BF8"}, // 107
                {amdgpu_gfx1250_op_V_WMMA_F32_16X16X64_BF8_FP8,"V_WMMA_F32_16X16X64_BF8_FP8"}, // 108
                {amdgpu_gfx1250_op_V_WMMA_F32_16X16X64_BF8_BF8,"V_WMMA_F32_16X16X64_BF8_BF8"}, // 109
                {amdgpu_gfx1250_op_V_WMMA_F16_16X16X64_FP8_FP8,"V_WMMA_F16_16X16X64_FP8_FP8"}, // 110
                {amdgpu_gfx1250_op_V_WMMA_F16_16X16X64_FP8_BF8,"V_WMMA_F16_16X16X64_FP8_BF8"}, // 111
                {amdgpu_gfx1250_op_V_WMMA_F16_16X16X64_BF8_FP8,"V_WMMA_F16_16X16X64_BF8_FP8"}, // 112
                {amdgpu_gfx1250_op_V_WMMA_F16_16X16X64_BF8_BF8,"V_WMMA_F16_16X16X64_BF8_BF8"}, // 113
                {amdgpu_gfx1250_op_V_WMMA_I32_16X16X64_IU8,"V_WMMA_I32_16X16X64_IU8"}, // 114
                {amdgpu_gfx1250_op_V_SWMMAC_F32_16X16X128_FP8_FP8,"V_SWMMAC_F32_16X16X128_FP8_FP8"}, // 115
                {amdgpu_gfx1250_op_V_SWMMAC_F32_16X16X128_FP8_BF8,"V_SWMMAC_F32_16X16X128_FP8_BF8"}, // 116
                {amdgpu_gfx1250_op_V_SWMMAC_F32_16X16X128_BF8_FP8,"V_SWMMAC_F32_16X16X128_BF8_FP8"}, // 117
                {amdgpu_gfx1250_op_V_SWMMAC_F32_16X16X128_BF8_BF8,"V_SWMMAC_F32_16X16X128_BF8_BF8"}, // 118
                {amdgpu_gfx1250_op_V_SWMMAC_F16_16X16X128_FP8_FP8,"V_SWMMAC_F16_16X16X128_FP8_FP8"}, // 119
                {amdgpu_gfx1250_op_V_SWMMAC_F16_16X16X128_FP8_BF8,"V_SWMMAC_F16_16X16X128_FP8_BF8"}, // 120
                {amdgpu_gfx1250_op_V_SWMMAC_F16_16X16X128_BF8_FP8,"V_SWMMAC_F16_16X16X128_BF8_FP8"}, // 121
                {amdgpu_gfx1250_op_V_SWMMAC_F16_16X16X128_BF8_BF8,"V_SWMMAC_F16_16X16X128_BF8_BF8"}, // 122
                {amdgpu_gfx1250_op_V_SWMMAC_I32_16X16X128_IU8,"V_SWMMAC_I32_16X16X128_IU8"}, // 123
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 124
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 125
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 126
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 127
                {amdgpu_gfx1250_op_V_WMMA_F32_16X16X128_FP8_FP8,"V_WMMA_F32_16X16X128_FP8_FP8"}, // 128
                {amdgpu_gfx1250_op_V_WMMA_F32_16X16X128_FP8_BF8,"V_WMMA_F32_16X16X128_FP8_BF8"}, // 129
                {amdgpu_gfx1250_op_V_WMMA_F32_16X16X128_BF8_FP8,"V_WMMA_F32_16X16X128_BF8_FP8"}, // 130
                {amdgpu_gfx1250_op_V_WMMA_F32_16X16X128_BF8_BF8,"V_WMMA_F32_16X16X128_BF8_BF8"}, // 131
                {amdgpu_gfx1250_op_V_WMMA_F16_16X16X128_FP8_FP8,"V_WMMA_F16_16X16X128_FP8_FP8"}, // 132
                {amdgpu_gfx1250_op_V_WMMA_F16_16X16X128_FP8_BF8,"V_WMMA_F16_16X16X128_FP8_BF8"}, // 133
                {amdgpu_gfx1250_op_V_WMMA_F16_16X16X128_BF8_FP8,"V_WMMA_F16_16X16X128_BF8_FP8"}, // 134
                {amdgpu_gfx1250_op_V_WMMA_F16_16X16X128_BF8_BF8,"V_WMMA_F16_16X16X128_BF8_BF8"}, // 135
                {amdgpu_gfx1250_op_V_WMMA_F32_32X16X128_F4,"V_WMMA_F32_32X16X128_F4"}, // 136
            }; // end ENC_VOP3P_insn_table
            const amdgpu_gfx1250_insn_entry VOP3P_INST_LITERAL_insn_table[64] =
            {
                {amdgpu_gfx1250_op_V_PK_MAD_I16,"V_PK_MAD_I16"}, // 0
                {amdgpu_gfx1250_op_V_PK_MUL_LO_U16,"V_PK_MUL_LO_U16"}, // 1
                {amdgpu_gfx1250_op_V_PK_ADD_I16,"V_PK_ADD_I16"}, // 2
                {amdgpu_gfx1250_op_V_PK_SUB_I16,"V_PK_SUB_I16"}, // 3
                {amdgpu_gfx1250_op_V_PK_LSHLREV_B16,"V_PK_LSHLREV_B16"}, // 4
                {amdgpu_gfx1250_op_V_PK_LSHRREV_B16,"V_PK_LSHRREV_B16"}, // 5
                {amdgpu_gfx1250_op_V_PK_ASHRREV_I16,"V_PK_ASHRREV_I16"}, // 6
                {amdgpu_gfx1250_op_V_PK_MAX_I16,"V_PK_MAX_I16"}, // 7
                {amdgpu_gfx1250_op_V_PK_MIN_I16,"V_PK_MIN_I16"}, // 8
                {amdgpu_gfx1250_op_V_PK_MAD_U16,"V_PK_MAD_U16"}, // 9
                {amdgpu_gfx1250_op_V_PK_ADD_U16,"V_PK_ADD_U16"}, // 10
                {amdgpu_gfx1250_op_V_PK_SUB_U16,"V_PK_SUB_U16"}, // 11
                {amdgpu_gfx1250_op_V_PK_MAX_U16,"V_PK_MAX_U16"}, // 12
                {amdgpu_gfx1250_op_V_PK_MIN_U16,"V_PK_MIN_U16"}, // 13
                {amdgpu_gfx1250_op_V_PK_FMA_F16,"V_PK_FMA_F16"}, // 14
                {amdgpu_gfx1250_op_V_PK_ADD_F16,"V_PK_ADD_F16"}, // 15
                {amdgpu_gfx1250_op_V_PK_MUL_F16,"V_PK_MUL_F16"}, // 16
                {amdgpu_gfx1250_op_V_PK_FMA_BF16,"V_PK_FMA_BF16"}, // 17
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 18
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 19
                {amdgpu_gfx1250_op_V_PK_ADD_MAX_I16,"V_PK_ADD_MAX_I16"}, // 20
                {amdgpu_gfx1250_op_V_PK_ADD_MAX_U16,"V_PK_ADD_MAX_U16"}, // 21
                {amdgpu_gfx1250_op_V_DOT4_I32_IU8,"V_DOT4_I32_IU8"}, // 22
                {amdgpu_gfx1250_op_V_DOT4_U32_U8,"V_DOT4_U32_U8"}, // 23
                {amdgpu_gfx1250_op_V_DOT8_I32_IU4,"V_DOT8_I32_IU4"}, // 24
                {amdgpu_gfx1250_op_V_DOT8_U32_U4,"V_DOT8_U32_U4"}, // 25
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 26
                {amdgpu_gfx1250_op_V_PK_MIN_NUM_F16,"V_PK_MIN_NUM_F16"}, // 27
                {amdgpu_gfx1250_op_V_PK_MAX_NUM_F16,"V_PK_MAX_NUM_F16"}, // 28
                {amdgpu_gfx1250_op_V_PK_MINIMUM_F16,"V_PK_MINIMUM_F16"}, // 29
                {amdgpu_gfx1250_op_V_PK_MAXIMUM_F16,"V_PK_MAXIMUM_F16"}, // 30
                {amdgpu_gfx1250_op_V_PK_FMA_F32,"V_PK_FMA_F32"}, // 31
                {amdgpu_gfx1250_op_V_FMA_MIX_F32,"V_FMA_MIX_F32"}, // 32
                {amdgpu_gfx1250_op_V_FMA_MIXLO_F16,"V_FMA_MIXLO_F16"}, // 33
                {amdgpu_gfx1250_op_V_FMA_MIXHI_F16,"V_FMA_MIXHI_F16"}, // 34
                {amdgpu_gfx1250_op_V_PK_ADD_BF16,"V_PK_ADD_BF16"}, // 35
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 36
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 39
                {amdgpu_gfx1250_op_V_PK_MUL_F32,"V_PK_MUL_F32"}, // 40
                {amdgpu_gfx1250_op_V_PK_ADD_F32,"V_PK_ADD_F32"}, // 41
                {amdgpu_gfx1250_op_V_PK_MUL_BF16,"V_PK_MUL_BF16"}, // 42
                {amdgpu_gfx1250_op_V_PK_MIN_NUM_BF16,"V_PK_MIN_NUM_BF16"}, // 43
                {amdgpu_gfx1250_op_V_PK_MAX_NUM_BF16,"V_PK_MAX_NUM_BF16"}, // 44
                {amdgpu_gfx1250_op_V_PK_ADD_MIN_I16,"V_PK_ADD_MIN_I16"}, // 45
                {amdgpu_gfx1250_op_V_PK_ADD_MIN_U16,"V_PK_ADD_MIN_U16"}, // 46
                {amdgpu_gfx1250_op_V_PK_MAX3_I16,"V_PK_MAX3_I16"}, // 47
                {amdgpu_gfx1250_op_V_PK_MAX3_U16,"V_PK_MAX3_U16"}, // 48
                {amdgpu_gfx1250_op_V_PK_MIN3_I16,"V_PK_MIN3_I16"}, // 49
                {amdgpu_gfx1250_op_V_PK_MIN3_U16,"V_PK_MIN3_U16"}, // 50
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 51
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 52
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 53
                {amdgpu_gfx1250_op_V_PK_MINIMUM3_F16,"V_PK_MINIMUM3_F16"}, // 54
                {amdgpu_gfx1250_op_V_PK_MAXIMUM3_F16,"V_PK_MAXIMUM3_F16"}, // 55
                {amdgpu_gfx1250_op_V_PK_MIN3_NUM_F16,"V_PK_MIN3_NUM_F16"}, // 56
                {amdgpu_gfx1250_op_V_PK_MAX3_NUM_F16,"V_PK_MAX3_NUM_F16"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 59
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 60
                {amdgpu_gfx1250_op_V_FMA_MIX_F32_BF16,"V_FMA_MIX_F32_BF16"}, // 61
                {amdgpu_gfx1250_op_V_FMA_MIXLO_BF16,"V_FMA_MIXLO_BF16"}, // 62
                {amdgpu_gfx1250_op_V_FMA_MIXHI_BF16,"V_FMA_MIXHI_BF16"}, // 63
            }; // end VOP3P_INST_LITERAL_insn_table
            const amdgpu_gfx1250_insn_entry VOP3P_VOP_DPP8_insn_table[64] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 1
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 2
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 3
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 4
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 5
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 6
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 7
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 8
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 9
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 10
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 11
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 12
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 13
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 17
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 18
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 19
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 20
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 21
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 22
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 23
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 24
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 25
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 26
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 27
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 28
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 29
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 30
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 31
                {amdgpu_gfx1250_op_V_FMA_MIX_F32,"V_FMA_MIX_F32"}, // 32
                {amdgpu_gfx1250_op_V_FMA_MIXLO_F16,"V_FMA_MIXLO_F16"}, // 33
                {amdgpu_gfx1250_op_V_FMA_MIXHI_F16,"V_FMA_MIXHI_F16"}, // 34
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 35
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 36
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 42
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 49
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 50
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 51
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 52
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 53
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 59
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 60
                {amdgpu_gfx1250_op_V_FMA_MIX_F32_BF16,"V_FMA_MIX_F32_BF16"}, // 61
                {amdgpu_gfx1250_op_V_FMA_MIXLO_BF16,"V_FMA_MIXLO_BF16"}, // 62
                {amdgpu_gfx1250_op_V_FMA_MIXHI_BF16,"V_FMA_MIXHI_BF16"}, // 63
            }; // end VOP3P_VOP_DPP8_insn_table
            const amdgpu_gfx1250_insn_entry VOP3P_VOP_DPP16_insn_table[64] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 1
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 2
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 3
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 4
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 5
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 6
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 7
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 8
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 9
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 10
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 11
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 12
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 13
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 17
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 18
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 19
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 20
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 21
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 22
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 23
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 24
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 25
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 26
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 27
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 28
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 29
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 30
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 31
                {amdgpu_gfx1250_op_V_FMA_MIX_F32,"V_FMA_MIX_F32"}, // 32
                {amdgpu_gfx1250_op_V_FMA_MIXLO_F16,"V_FMA_MIXLO_F16"}, // 33
                {amdgpu_gfx1250_op_V_FMA_MIXHI_F16,"V_FMA_MIXHI_F16"}, // 34
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 35
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 36
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 42
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 49
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 50
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 51
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 52
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 53
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 59
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 60
                {amdgpu_gfx1250_op_V_FMA_MIX_F32_BF16,"V_FMA_MIX_F32_BF16"}, // 61
                {amdgpu_gfx1250_op_V_FMA_MIXLO_BF16,"V_FMA_MIXLO_BF16"}, // 62
                {amdgpu_gfx1250_op_V_FMA_MIXHI_BF16,"V_FMA_MIXHI_BF16"}, // 63
            }; // end VOP3P_VOP_DPP16_insn_table
            const amdgpu_gfx1250_insn_entry ENC_VOP3PX2_insn_table[137] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 1
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 2
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 3
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 4
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 5
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 6
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 7
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 8
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 9
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 10
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 11
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 12
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 13
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 17
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 18
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 19
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 20
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 21
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 22
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 23
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 24
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 25
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 26
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 27
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 28
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 29
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 30
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 31
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 32
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 33
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 34
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 35
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 36
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 42
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 49
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 50
                {amdgpu_gfx1250_op_V_WMMA_SCALE_F32_16X16X128_F8F6F4,"V_WMMA_SCALE_F32_16X16X128_F8F6F4"}, // 51
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 52
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 53
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 59
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 60
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 61
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 64
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 65
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 66
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 67
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 68
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 69
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 72
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 73
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 74
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 75
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 76
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 77
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 80
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 81
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 82
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 83
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 84
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 85
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 86
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 87
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 88
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 89
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 90
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 91
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 92
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 93
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 94
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 95
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 96
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 97
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 98
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 99
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 100
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 101
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 102
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 103
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 104
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 105
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 106
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 107
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 108
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 109
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 110
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 111
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 112
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 113
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 114
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 115
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 116
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 117
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 118
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 119
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 120
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 121
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 122
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 123
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 124
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 125
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 126
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 127
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 128
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 129
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 130
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 131
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 132
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 133
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 134
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 135
                {amdgpu_gfx1250_op_V_WMMA_SCALE_F32_32X16X128_F4,"V_WMMA_SCALE_F32_32X16X128_F4"}, // 136
            }; // end ENC_VOP3PX2_insn_table
            const amdgpu_gfx1250_insn_entry ENC_VOP3PX3_insn_table[137] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 1
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 2
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 3
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 4
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 5
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 6
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 7
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 8
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 9
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 10
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 11
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 12
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 13
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 17
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 18
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 19
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 20
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 21
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 22
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 23
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 24
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 25
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 26
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 27
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 28
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 29
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 30
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 31
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 32
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 33
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 34
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 35
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 36
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 42
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 49
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 50
                {amdgpu_gfx1250_op_V_WMMA_SCALE16_F32_16X16X128_F8F6F4,"V_WMMA_SCALE16_F32_16X16X128_F8F6F4"}, // 51
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 52
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 53
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 59
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 60
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 61
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 64
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 65
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 66
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 67
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 68
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 69
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 72
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 73
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 74
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 75
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 76
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 77
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 80
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 81
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 82
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 83
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 84
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 85
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 86
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 87
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 88
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 89
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 90
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 91
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 92
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 93
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 94
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 95
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 96
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 97
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 98
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 99
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 100
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 101
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 102
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 103
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 104
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 105
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 106
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 107
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 108
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 109
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 110
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 111
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 112
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 113
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 114
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 115
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 116
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 117
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 118
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 119
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 120
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 121
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 122
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 123
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 124
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 125
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 126
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 127
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 128
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 129
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 130
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 131
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 132
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 133
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 134
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 135
                {amdgpu_gfx1250_op_V_WMMA_SCALE16_F32_32X16X128_F4,"V_WMMA_SCALE16_F32_32X16X128_F4"}, // 136
            }; // end ENC_VOP3PX3_insn_table
            const amdgpu_gfx1250_insn_entry ENC_VOPC_insn_table[256] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_V_CMP_LT_F16,"V_CMP_LT_F16"}, // 1
                {amdgpu_gfx1250_op_V_CMP_EQ_F16,"V_CMP_EQ_F16"}, // 2
                {amdgpu_gfx1250_op_V_CMP_LE_F16,"V_CMP_LE_F16"}, // 3
                {amdgpu_gfx1250_op_V_CMP_GT_F16,"V_CMP_GT_F16"}, // 4
                {amdgpu_gfx1250_op_V_CMP_LG_F16,"V_CMP_LG_F16"}, // 5
                {amdgpu_gfx1250_op_V_CMP_GE_F16,"V_CMP_GE_F16"}, // 6
                {amdgpu_gfx1250_op_V_CMP_O_F16,"V_CMP_O_F16"}, // 7
                {amdgpu_gfx1250_op_V_CMP_U_F16,"V_CMP_U_F16"}, // 8
                {amdgpu_gfx1250_op_V_CMP_NGE_F16,"V_CMP_NGE_F16"}, // 9
                {amdgpu_gfx1250_op_V_CMP_NLG_F16,"V_CMP_NLG_F16"}, // 10
                {amdgpu_gfx1250_op_V_CMP_NGT_F16,"V_CMP_NGT_F16"}, // 11
                {amdgpu_gfx1250_op_V_CMP_NLE_F16,"V_CMP_NLE_F16"}, // 12
                {amdgpu_gfx1250_op_V_CMP_NEQ_F16,"V_CMP_NEQ_F16"}, // 13
                {amdgpu_gfx1250_op_V_CMP_NLT_F16,"V_CMP_NLT_F16"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_V_CMP_LT_F32,"V_CMP_LT_F32"}, // 17
                {amdgpu_gfx1250_op_V_CMP_EQ_F32,"V_CMP_EQ_F32"}, // 18
                {amdgpu_gfx1250_op_V_CMP_LE_F32,"V_CMP_LE_F32"}, // 19
                {amdgpu_gfx1250_op_V_CMP_GT_F32,"V_CMP_GT_F32"}, // 20
                {amdgpu_gfx1250_op_V_CMP_LG_F32,"V_CMP_LG_F32"}, // 21
                {amdgpu_gfx1250_op_V_CMP_GE_F32,"V_CMP_GE_F32"}, // 22
                {amdgpu_gfx1250_op_V_CMP_O_F32,"V_CMP_O_F32"}, // 23
                {amdgpu_gfx1250_op_V_CMP_U_F32,"V_CMP_U_F32"}, // 24
                {amdgpu_gfx1250_op_V_CMP_NGE_F32,"V_CMP_NGE_F32"}, // 25
                {amdgpu_gfx1250_op_V_CMP_NLG_F32,"V_CMP_NLG_F32"}, // 26
                {amdgpu_gfx1250_op_V_CMP_NGT_F32,"V_CMP_NGT_F32"}, // 27
                {amdgpu_gfx1250_op_V_CMP_NLE_F32,"V_CMP_NLE_F32"}, // 28
                {amdgpu_gfx1250_op_V_CMP_NEQ_F32,"V_CMP_NEQ_F32"}, // 29
                {amdgpu_gfx1250_op_V_CMP_NLT_F32,"V_CMP_NLT_F32"}, // 30
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 31
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 32
                {amdgpu_gfx1250_op_V_CMP_LT_F64,"V_CMP_LT_F64"}, // 33
                {amdgpu_gfx1250_op_V_CMP_EQ_F64,"V_CMP_EQ_F64"}, // 34
                {amdgpu_gfx1250_op_V_CMP_LE_F64,"V_CMP_LE_F64"}, // 35
                {amdgpu_gfx1250_op_V_CMP_GT_F64,"V_CMP_GT_F64"}, // 36
                {amdgpu_gfx1250_op_V_CMP_LG_F64,"V_CMP_LG_F64"}, // 37
                {amdgpu_gfx1250_op_V_CMP_GE_F64,"V_CMP_GE_F64"}, // 38
                {amdgpu_gfx1250_op_V_CMP_O_F64,"V_CMP_O_F64"}, // 39
                {amdgpu_gfx1250_op_V_CMP_U_F64,"V_CMP_U_F64"}, // 40
                {amdgpu_gfx1250_op_V_CMP_NGE_F64,"V_CMP_NGE_F64"}, // 41
                {amdgpu_gfx1250_op_V_CMP_NLG_F64,"V_CMP_NLG_F64"}, // 42
                {amdgpu_gfx1250_op_V_CMP_NGT_F64,"V_CMP_NGT_F64"}, // 43
                {amdgpu_gfx1250_op_V_CMP_NLE_F64,"V_CMP_NLE_F64"}, // 44
                {amdgpu_gfx1250_op_V_CMP_NEQ_F64,"V_CMP_NEQ_F64"}, // 45
                {amdgpu_gfx1250_op_V_CMP_NLT_F64,"V_CMP_NLT_F64"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_V_CMP_LT_I16,"V_CMP_LT_I16"}, // 49
                {amdgpu_gfx1250_op_V_CMP_EQ_I16,"V_CMP_EQ_I16"}, // 50
                {amdgpu_gfx1250_op_V_CMP_LE_I16,"V_CMP_LE_I16"}, // 51
                {amdgpu_gfx1250_op_V_CMP_GT_I16,"V_CMP_GT_I16"}, // 52
                {amdgpu_gfx1250_op_V_CMP_NE_I16,"V_CMP_NE_I16"}, // 53
                {amdgpu_gfx1250_op_V_CMP_GE_I16,"V_CMP_GE_I16"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_V_CMP_LT_U16,"V_CMP_LT_U16"}, // 57
                {amdgpu_gfx1250_op_V_CMP_EQ_U16,"V_CMP_EQ_U16"}, // 58
                {amdgpu_gfx1250_op_V_CMP_LE_U16,"V_CMP_LE_U16"}, // 59
                {amdgpu_gfx1250_op_V_CMP_GT_U16,"V_CMP_GT_U16"}, // 60
                {amdgpu_gfx1250_op_V_CMP_NE_U16,"V_CMP_NE_U16"}, // 61
                {amdgpu_gfx1250_op_V_CMP_GE_U16,"V_CMP_GE_U16"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 64
                {amdgpu_gfx1250_op_V_CMP_LT_I32,"V_CMP_LT_I32"}, // 65
                {amdgpu_gfx1250_op_V_CMP_EQ_I32,"V_CMP_EQ_I32"}, // 66
                {amdgpu_gfx1250_op_V_CMP_LE_I32,"V_CMP_LE_I32"}, // 67
                {amdgpu_gfx1250_op_V_CMP_GT_I32,"V_CMP_GT_I32"}, // 68
                {amdgpu_gfx1250_op_V_CMP_NE_I32,"V_CMP_NE_I32"}, // 69
                {amdgpu_gfx1250_op_V_CMP_GE_I32,"V_CMP_GE_I32"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 72
                {amdgpu_gfx1250_op_V_CMP_LT_U32,"V_CMP_LT_U32"}, // 73
                {amdgpu_gfx1250_op_V_CMP_EQ_U32,"V_CMP_EQ_U32"}, // 74
                {amdgpu_gfx1250_op_V_CMP_LE_U32,"V_CMP_LE_U32"}, // 75
                {amdgpu_gfx1250_op_V_CMP_GT_U32,"V_CMP_GT_U32"}, // 76
                {amdgpu_gfx1250_op_V_CMP_NE_U32,"V_CMP_NE_U32"}, // 77
                {amdgpu_gfx1250_op_V_CMP_GE_U32,"V_CMP_GE_U32"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 80
                {amdgpu_gfx1250_op_V_CMP_LT_I64,"V_CMP_LT_I64"}, // 81
                {amdgpu_gfx1250_op_V_CMP_EQ_I64,"V_CMP_EQ_I64"}, // 82
                {amdgpu_gfx1250_op_V_CMP_LE_I64,"V_CMP_LE_I64"}, // 83
                {amdgpu_gfx1250_op_V_CMP_GT_I64,"V_CMP_GT_I64"}, // 84
                {amdgpu_gfx1250_op_V_CMP_NE_I64,"V_CMP_NE_I64"}, // 85
                {amdgpu_gfx1250_op_V_CMP_GE_I64,"V_CMP_GE_I64"}, // 86
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 87
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 88
                {amdgpu_gfx1250_op_V_CMP_LT_U64,"V_CMP_LT_U64"}, // 89
                {amdgpu_gfx1250_op_V_CMP_EQ_U64,"V_CMP_EQ_U64"}, // 90
                {amdgpu_gfx1250_op_V_CMP_LE_U64,"V_CMP_LE_U64"}, // 91
                {amdgpu_gfx1250_op_V_CMP_GT_U64,"V_CMP_GT_U64"}, // 92
                {amdgpu_gfx1250_op_V_CMP_NE_U64,"V_CMP_NE_U64"}, // 93
                {amdgpu_gfx1250_op_V_CMP_GE_U64,"V_CMP_GE_U64"}, // 94
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 95
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 96
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 97
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 98
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 99
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 100
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 101
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 102
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 103
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 104
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 105
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 106
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 107
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 108
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 109
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 110
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 111
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 112
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 113
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 114
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 115
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 116
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 117
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 118
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 119
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 120
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 121
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 122
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 123
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 124
                {amdgpu_gfx1250_op_V_CMP_CLASS_F16,"V_CMP_CLASS_F16"}, // 125
                {amdgpu_gfx1250_op_V_CMP_CLASS_F32,"V_CMP_CLASS_F32"}, // 126
                {amdgpu_gfx1250_op_V_CMP_CLASS_F64,"V_CMP_CLASS_F64"}, // 127
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 128
                {amdgpu_gfx1250_op_V_CMPX_LT_F16,"V_CMPX_LT_F16"}, // 129
                {amdgpu_gfx1250_op_V_CMPX_EQ_F16,"V_CMPX_EQ_F16"}, // 130
                {amdgpu_gfx1250_op_V_CMPX_LE_F16,"V_CMPX_LE_F16"}, // 131
                {amdgpu_gfx1250_op_V_CMPX_GT_F16,"V_CMPX_GT_F16"}, // 132
                {amdgpu_gfx1250_op_V_CMPX_LG_F16,"V_CMPX_LG_F16"}, // 133
                {amdgpu_gfx1250_op_V_CMPX_GE_F16,"V_CMPX_GE_F16"}, // 134
                {amdgpu_gfx1250_op_V_CMPX_O_F16,"V_CMPX_O_F16"}, // 135
                {amdgpu_gfx1250_op_V_CMPX_U_F16,"V_CMPX_U_F16"}, // 136
                {amdgpu_gfx1250_op_V_CMPX_NGE_F16,"V_CMPX_NGE_F16"}, // 137
                {amdgpu_gfx1250_op_V_CMPX_NLG_F16,"V_CMPX_NLG_F16"}, // 138
                {amdgpu_gfx1250_op_V_CMPX_NGT_F16,"V_CMPX_NGT_F16"}, // 139
                {amdgpu_gfx1250_op_V_CMPX_NLE_F16,"V_CMPX_NLE_F16"}, // 140
                {amdgpu_gfx1250_op_V_CMPX_NEQ_F16,"V_CMPX_NEQ_F16"}, // 141
                {amdgpu_gfx1250_op_V_CMPX_NLT_F16,"V_CMPX_NLT_F16"}, // 142
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 143
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 144
                {amdgpu_gfx1250_op_V_CMPX_LT_F32,"V_CMPX_LT_F32"}, // 145
                {amdgpu_gfx1250_op_V_CMPX_EQ_F32,"V_CMPX_EQ_F32"}, // 146
                {amdgpu_gfx1250_op_V_CMPX_LE_F32,"V_CMPX_LE_F32"}, // 147
                {amdgpu_gfx1250_op_V_CMPX_GT_F32,"V_CMPX_GT_F32"}, // 148
                {amdgpu_gfx1250_op_V_CMPX_LG_F32,"V_CMPX_LG_F32"}, // 149
                {amdgpu_gfx1250_op_V_CMPX_GE_F32,"V_CMPX_GE_F32"}, // 150
                {amdgpu_gfx1250_op_V_CMPX_O_F32,"V_CMPX_O_F32"}, // 151
                {amdgpu_gfx1250_op_V_CMPX_U_F32,"V_CMPX_U_F32"}, // 152
                {amdgpu_gfx1250_op_V_CMPX_NGE_F32,"V_CMPX_NGE_F32"}, // 153
                {amdgpu_gfx1250_op_V_CMPX_NLG_F32,"V_CMPX_NLG_F32"}, // 154
                {amdgpu_gfx1250_op_V_CMPX_NGT_F32,"V_CMPX_NGT_F32"}, // 155
                {amdgpu_gfx1250_op_V_CMPX_NLE_F32,"V_CMPX_NLE_F32"}, // 156
                {amdgpu_gfx1250_op_V_CMPX_NEQ_F32,"V_CMPX_NEQ_F32"}, // 157
                {amdgpu_gfx1250_op_V_CMPX_NLT_F32,"V_CMPX_NLT_F32"}, // 158
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 159
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 160
                {amdgpu_gfx1250_op_V_CMPX_LT_F64,"V_CMPX_LT_F64"}, // 161
                {amdgpu_gfx1250_op_V_CMPX_EQ_F64,"V_CMPX_EQ_F64"}, // 162
                {amdgpu_gfx1250_op_V_CMPX_LE_F64,"V_CMPX_LE_F64"}, // 163
                {amdgpu_gfx1250_op_V_CMPX_GT_F64,"V_CMPX_GT_F64"}, // 164
                {amdgpu_gfx1250_op_V_CMPX_LG_F64,"V_CMPX_LG_F64"}, // 165
                {amdgpu_gfx1250_op_V_CMPX_GE_F64,"V_CMPX_GE_F64"}, // 166
                {amdgpu_gfx1250_op_V_CMPX_O_F64,"V_CMPX_O_F64"}, // 167
                {amdgpu_gfx1250_op_V_CMPX_U_F64,"V_CMPX_U_F64"}, // 168
                {amdgpu_gfx1250_op_V_CMPX_NGE_F64,"V_CMPX_NGE_F64"}, // 169
                {amdgpu_gfx1250_op_V_CMPX_NLG_F64,"V_CMPX_NLG_F64"}, // 170
                {amdgpu_gfx1250_op_V_CMPX_NGT_F64,"V_CMPX_NGT_F64"}, // 171
                {amdgpu_gfx1250_op_V_CMPX_NLE_F64,"V_CMPX_NLE_F64"}, // 172
                {amdgpu_gfx1250_op_V_CMPX_NEQ_F64,"V_CMPX_NEQ_F64"}, // 173
                {amdgpu_gfx1250_op_V_CMPX_NLT_F64,"V_CMPX_NLT_F64"}, // 174
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 175
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 176
                {amdgpu_gfx1250_op_V_CMPX_LT_I16,"V_CMPX_LT_I16"}, // 177
                {amdgpu_gfx1250_op_V_CMPX_EQ_I16,"V_CMPX_EQ_I16"}, // 178
                {amdgpu_gfx1250_op_V_CMPX_LE_I16,"V_CMPX_LE_I16"}, // 179
                {amdgpu_gfx1250_op_V_CMPX_GT_I16,"V_CMPX_GT_I16"}, // 180
                {amdgpu_gfx1250_op_V_CMPX_NE_I16,"V_CMPX_NE_I16"}, // 181
                {amdgpu_gfx1250_op_V_CMPX_GE_I16,"V_CMPX_GE_I16"}, // 182
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 183
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 184
                {amdgpu_gfx1250_op_V_CMPX_LT_U16,"V_CMPX_LT_U16"}, // 185
                {amdgpu_gfx1250_op_V_CMPX_EQ_U16,"V_CMPX_EQ_U16"}, // 186
                {amdgpu_gfx1250_op_V_CMPX_LE_U16,"V_CMPX_LE_U16"}, // 187
                {amdgpu_gfx1250_op_V_CMPX_GT_U16,"V_CMPX_GT_U16"}, // 188
                {amdgpu_gfx1250_op_V_CMPX_NE_U16,"V_CMPX_NE_U16"}, // 189
                {amdgpu_gfx1250_op_V_CMPX_GE_U16,"V_CMPX_GE_U16"}, // 190
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 191
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 192
                {amdgpu_gfx1250_op_V_CMPX_LT_I32,"V_CMPX_LT_I32"}, // 193
                {amdgpu_gfx1250_op_V_CMPX_EQ_I32,"V_CMPX_EQ_I32"}, // 194
                {amdgpu_gfx1250_op_V_CMPX_LE_I32,"V_CMPX_LE_I32"}, // 195
                {amdgpu_gfx1250_op_V_CMPX_GT_I32,"V_CMPX_GT_I32"}, // 196
                {amdgpu_gfx1250_op_V_CMPX_NE_I32,"V_CMPX_NE_I32"}, // 197
                {amdgpu_gfx1250_op_V_CMPX_GE_I32,"V_CMPX_GE_I32"}, // 198
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 199
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 200
                {amdgpu_gfx1250_op_V_CMPX_LT_U32,"V_CMPX_LT_U32"}, // 201
                {amdgpu_gfx1250_op_V_CMPX_EQ_U32,"V_CMPX_EQ_U32"}, // 202
                {amdgpu_gfx1250_op_V_CMPX_LE_U32,"V_CMPX_LE_U32"}, // 203
                {amdgpu_gfx1250_op_V_CMPX_GT_U32,"V_CMPX_GT_U32"}, // 204
                {amdgpu_gfx1250_op_V_CMPX_NE_U32,"V_CMPX_NE_U32"}, // 205
                {amdgpu_gfx1250_op_V_CMPX_GE_U32,"V_CMPX_GE_U32"}, // 206
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 207
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 208
                {amdgpu_gfx1250_op_V_CMPX_LT_I64,"V_CMPX_LT_I64"}, // 209
                {amdgpu_gfx1250_op_V_CMPX_EQ_I64,"V_CMPX_EQ_I64"}, // 210
                {amdgpu_gfx1250_op_V_CMPX_LE_I64,"V_CMPX_LE_I64"}, // 211
                {amdgpu_gfx1250_op_V_CMPX_GT_I64,"V_CMPX_GT_I64"}, // 212
                {amdgpu_gfx1250_op_V_CMPX_NE_I64,"V_CMPX_NE_I64"}, // 213
                {amdgpu_gfx1250_op_V_CMPX_GE_I64,"V_CMPX_GE_I64"}, // 214
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 215
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 216
                {amdgpu_gfx1250_op_V_CMPX_LT_U64,"V_CMPX_LT_U64"}, // 217
                {amdgpu_gfx1250_op_V_CMPX_EQ_U64,"V_CMPX_EQ_U64"}, // 218
                {amdgpu_gfx1250_op_V_CMPX_LE_U64,"V_CMPX_LE_U64"}, // 219
                {amdgpu_gfx1250_op_V_CMPX_GT_U64,"V_CMPX_GT_U64"}, // 220
                {amdgpu_gfx1250_op_V_CMPX_NE_U64,"V_CMPX_NE_U64"}, // 221
                {amdgpu_gfx1250_op_V_CMPX_GE_U64,"V_CMPX_GE_U64"}, // 222
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 223
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 224
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 225
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 226
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 227
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 228
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 229
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 230
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 231
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 232
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 233
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 234
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 235
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 236
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 237
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 238
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 239
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 240
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 241
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 242
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 243
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 244
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 245
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 246
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 247
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 248
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 249
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 250
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 251
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 252
                {amdgpu_gfx1250_op_V_CMPX_CLASS_F16,"V_CMPX_CLASS_F16"}, // 253
                {amdgpu_gfx1250_op_V_CMPX_CLASS_F32,"V_CMPX_CLASS_F32"}, // 254
                {amdgpu_gfx1250_op_V_CMPX_CLASS_F64,"V_CMPX_CLASS_F64"}, // 255
            }; // end ENC_VOPC_insn_table
            const amdgpu_gfx1250_insn_entry VOPC_INST_LITERAL_insn_table[256] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_V_CMP_LT_F16,"V_CMP_LT_F16"}, // 1
                {amdgpu_gfx1250_op_V_CMP_EQ_F16,"V_CMP_EQ_F16"}, // 2
                {amdgpu_gfx1250_op_V_CMP_LE_F16,"V_CMP_LE_F16"}, // 3
                {amdgpu_gfx1250_op_V_CMP_GT_F16,"V_CMP_GT_F16"}, // 4
                {amdgpu_gfx1250_op_V_CMP_LG_F16,"V_CMP_LG_F16"}, // 5
                {amdgpu_gfx1250_op_V_CMP_GE_F16,"V_CMP_GE_F16"}, // 6
                {amdgpu_gfx1250_op_V_CMP_O_F16,"V_CMP_O_F16"}, // 7
                {amdgpu_gfx1250_op_V_CMP_U_F16,"V_CMP_U_F16"}, // 8
                {amdgpu_gfx1250_op_V_CMP_NGE_F16,"V_CMP_NGE_F16"}, // 9
                {amdgpu_gfx1250_op_V_CMP_NLG_F16,"V_CMP_NLG_F16"}, // 10
                {amdgpu_gfx1250_op_V_CMP_NGT_F16,"V_CMP_NGT_F16"}, // 11
                {amdgpu_gfx1250_op_V_CMP_NLE_F16,"V_CMP_NLE_F16"}, // 12
                {amdgpu_gfx1250_op_V_CMP_NEQ_F16,"V_CMP_NEQ_F16"}, // 13
                {amdgpu_gfx1250_op_V_CMP_NLT_F16,"V_CMP_NLT_F16"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_V_CMP_LT_F32,"V_CMP_LT_F32"}, // 17
                {amdgpu_gfx1250_op_V_CMP_EQ_F32,"V_CMP_EQ_F32"}, // 18
                {amdgpu_gfx1250_op_V_CMP_LE_F32,"V_CMP_LE_F32"}, // 19
                {amdgpu_gfx1250_op_V_CMP_GT_F32,"V_CMP_GT_F32"}, // 20
                {amdgpu_gfx1250_op_V_CMP_LG_F32,"V_CMP_LG_F32"}, // 21
                {amdgpu_gfx1250_op_V_CMP_GE_F32,"V_CMP_GE_F32"}, // 22
                {amdgpu_gfx1250_op_V_CMP_O_F32,"V_CMP_O_F32"}, // 23
                {amdgpu_gfx1250_op_V_CMP_U_F32,"V_CMP_U_F32"}, // 24
                {amdgpu_gfx1250_op_V_CMP_NGE_F32,"V_CMP_NGE_F32"}, // 25
                {amdgpu_gfx1250_op_V_CMP_NLG_F32,"V_CMP_NLG_F32"}, // 26
                {amdgpu_gfx1250_op_V_CMP_NGT_F32,"V_CMP_NGT_F32"}, // 27
                {amdgpu_gfx1250_op_V_CMP_NLE_F32,"V_CMP_NLE_F32"}, // 28
                {amdgpu_gfx1250_op_V_CMP_NEQ_F32,"V_CMP_NEQ_F32"}, // 29
                {amdgpu_gfx1250_op_V_CMP_NLT_F32,"V_CMP_NLT_F32"}, // 30
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 31
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 32
                {amdgpu_gfx1250_op_V_CMP_LT_F64,"V_CMP_LT_F64"}, // 33
                {amdgpu_gfx1250_op_V_CMP_EQ_F64,"V_CMP_EQ_F64"}, // 34
                {amdgpu_gfx1250_op_V_CMP_LE_F64,"V_CMP_LE_F64"}, // 35
                {amdgpu_gfx1250_op_V_CMP_GT_F64,"V_CMP_GT_F64"}, // 36
                {amdgpu_gfx1250_op_V_CMP_LG_F64,"V_CMP_LG_F64"}, // 37
                {amdgpu_gfx1250_op_V_CMP_GE_F64,"V_CMP_GE_F64"}, // 38
                {amdgpu_gfx1250_op_V_CMP_O_F64,"V_CMP_O_F64"}, // 39
                {amdgpu_gfx1250_op_V_CMP_U_F64,"V_CMP_U_F64"}, // 40
                {amdgpu_gfx1250_op_V_CMP_NGE_F64,"V_CMP_NGE_F64"}, // 41
                {amdgpu_gfx1250_op_V_CMP_NLG_F64,"V_CMP_NLG_F64"}, // 42
                {amdgpu_gfx1250_op_V_CMP_NGT_F64,"V_CMP_NGT_F64"}, // 43
                {amdgpu_gfx1250_op_V_CMP_NLE_F64,"V_CMP_NLE_F64"}, // 44
                {amdgpu_gfx1250_op_V_CMP_NEQ_F64,"V_CMP_NEQ_F64"}, // 45
                {amdgpu_gfx1250_op_V_CMP_NLT_F64,"V_CMP_NLT_F64"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_V_CMP_LT_I16,"V_CMP_LT_I16"}, // 49
                {amdgpu_gfx1250_op_V_CMP_EQ_I16,"V_CMP_EQ_I16"}, // 50
                {amdgpu_gfx1250_op_V_CMP_LE_I16,"V_CMP_LE_I16"}, // 51
                {amdgpu_gfx1250_op_V_CMP_GT_I16,"V_CMP_GT_I16"}, // 52
                {amdgpu_gfx1250_op_V_CMP_NE_I16,"V_CMP_NE_I16"}, // 53
                {amdgpu_gfx1250_op_V_CMP_GE_I16,"V_CMP_GE_I16"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_V_CMP_LT_U16,"V_CMP_LT_U16"}, // 57
                {amdgpu_gfx1250_op_V_CMP_EQ_U16,"V_CMP_EQ_U16"}, // 58
                {amdgpu_gfx1250_op_V_CMP_LE_U16,"V_CMP_LE_U16"}, // 59
                {amdgpu_gfx1250_op_V_CMP_GT_U16,"V_CMP_GT_U16"}, // 60
                {amdgpu_gfx1250_op_V_CMP_NE_U16,"V_CMP_NE_U16"}, // 61
                {amdgpu_gfx1250_op_V_CMP_GE_U16,"V_CMP_GE_U16"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 64
                {amdgpu_gfx1250_op_V_CMP_LT_I32,"V_CMP_LT_I32"}, // 65
                {amdgpu_gfx1250_op_V_CMP_EQ_I32,"V_CMP_EQ_I32"}, // 66
                {amdgpu_gfx1250_op_V_CMP_LE_I32,"V_CMP_LE_I32"}, // 67
                {amdgpu_gfx1250_op_V_CMP_GT_I32,"V_CMP_GT_I32"}, // 68
                {amdgpu_gfx1250_op_V_CMP_NE_I32,"V_CMP_NE_I32"}, // 69
                {amdgpu_gfx1250_op_V_CMP_GE_I32,"V_CMP_GE_I32"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 72
                {amdgpu_gfx1250_op_V_CMP_LT_U32,"V_CMP_LT_U32"}, // 73
                {amdgpu_gfx1250_op_V_CMP_EQ_U32,"V_CMP_EQ_U32"}, // 74
                {amdgpu_gfx1250_op_V_CMP_LE_U32,"V_CMP_LE_U32"}, // 75
                {amdgpu_gfx1250_op_V_CMP_GT_U32,"V_CMP_GT_U32"}, // 76
                {amdgpu_gfx1250_op_V_CMP_NE_U32,"V_CMP_NE_U32"}, // 77
                {amdgpu_gfx1250_op_V_CMP_GE_U32,"V_CMP_GE_U32"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 80
                {amdgpu_gfx1250_op_V_CMP_LT_I64,"V_CMP_LT_I64"}, // 81
                {amdgpu_gfx1250_op_V_CMP_EQ_I64,"V_CMP_EQ_I64"}, // 82
                {amdgpu_gfx1250_op_V_CMP_LE_I64,"V_CMP_LE_I64"}, // 83
                {amdgpu_gfx1250_op_V_CMP_GT_I64,"V_CMP_GT_I64"}, // 84
                {amdgpu_gfx1250_op_V_CMP_NE_I64,"V_CMP_NE_I64"}, // 85
                {amdgpu_gfx1250_op_V_CMP_GE_I64,"V_CMP_GE_I64"}, // 86
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 87
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 88
                {amdgpu_gfx1250_op_V_CMP_LT_U64,"V_CMP_LT_U64"}, // 89
                {amdgpu_gfx1250_op_V_CMP_EQ_U64,"V_CMP_EQ_U64"}, // 90
                {amdgpu_gfx1250_op_V_CMP_LE_U64,"V_CMP_LE_U64"}, // 91
                {amdgpu_gfx1250_op_V_CMP_GT_U64,"V_CMP_GT_U64"}, // 92
                {amdgpu_gfx1250_op_V_CMP_NE_U64,"V_CMP_NE_U64"}, // 93
                {amdgpu_gfx1250_op_V_CMP_GE_U64,"V_CMP_GE_U64"}, // 94
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 95
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 96
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 97
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 98
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 99
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 100
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 101
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 102
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 103
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 104
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 105
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 106
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 107
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 108
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 109
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 110
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 111
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 112
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 113
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 114
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 115
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 116
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 117
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 118
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 119
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 120
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 121
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 122
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 123
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 124
                {amdgpu_gfx1250_op_V_CMP_CLASS_F16,"V_CMP_CLASS_F16"}, // 125
                {amdgpu_gfx1250_op_V_CMP_CLASS_F32,"V_CMP_CLASS_F32"}, // 126
                {amdgpu_gfx1250_op_V_CMP_CLASS_F64,"V_CMP_CLASS_F64"}, // 127
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 128
                {amdgpu_gfx1250_op_V_CMPX_LT_F16,"V_CMPX_LT_F16"}, // 129
                {amdgpu_gfx1250_op_V_CMPX_EQ_F16,"V_CMPX_EQ_F16"}, // 130
                {amdgpu_gfx1250_op_V_CMPX_LE_F16,"V_CMPX_LE_F16"}, // 131
                {amdgpu_gfx1250_op_V_CMPX_GT_F16,"V_CMPX_GT_F16"}, // 132
                {amdgpu_gfx1250_op_V_CMPX_LG_F16,"V_CMPX_LG_F16"}, // 133
                {amdgpu_gfx1250_op_V_CMPX_GE_F16,"V_CMPX_GE_F16"}, // 134
                {amdgpu_gfx1250_op_V_CMPX_O_F16,"V_CMPX_O_F16"}, // 135
                {amdgpu_gfx1250_op_V_CMPX_U_F16,"V_CMPX_U_F16"}, // 136
                {amdgpu_gfx1250_op_V_CMPX_NGE_F16,"V_CMPX_NGE_F16"}, // 137
                {amdgpu_gfx1250_op_V_CMPX_NLG_F16,"V_CMPX_NLG_F16"}, // 138
                {amdgpu_gfx1250_op_V_CMPX_NGT_F16,"V_CMPX_NGT_F16"}, // 139
                {amdgpu_gfx1250_op_V_CMPX_NLE_F16,"V_CMPX_NLE_F16"}, // 140
                {amdgpu_gfx1250_op_V_CMPX_NEQ_F16,"V_CMPX_NEQ_F16"}, // 141
                {amdgpu_gfx1250_op_V_CMPX_NLT_F16,"V_CMPX_NLT_F16"}, // 142
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 143
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 144
                {amdgpu_gfx1250_op_V_CMPX_LT_F32,"V_CMPX_LT_F32"}, // 145
                {amdgpu_gfx1250_op_V_CMPX_EQ_F32,"V_CMPX_EQ_F32"}, // 146
                {amdgpu_gfx1250_op_V_CMPX_LE_F32,"V_CMPX_LE_F32"}, // 147
                {amdgpu_gfx1250_op_V_CMPX_GT_F32,"V_CMPX_GT_F32"}, // 148
                {amdgpu_gfx1250_op_V_CMPX_LG_F32,"V_CMPX_LG_F32"}, // 149
                {amdgpu_gfx1250_op_V_CMPX_GE_F32,"V_CMPX_GE_F32"}, // 150
                {amdgpu_gfx1250_op_V_CMPX_O_F32,"V_CMPX_O_F32"}, // 151
                {amdgpu_gfx1250_op_V_CMPX_U_F32,"V_CMPX_U_F32"}, // 152
                {amdgpu_gfx1250_op_V_CMPX_NGE_F32,"V_CMPX_NGE_F32"}, // 153
                {amdgpu_gfx1250_op_V_CMPX_NLG_F32,"V_CMPX_NLG_F32"}, // 154
                {amdgpu_gfx1250_op_V_CMPX_NGT_F32,"V_CMPX_NGT_F32"}, // 155
                {amdgpu_gfx1250_op_V_CMPX_NLE_F32,"V_CMPX_NLE_F32"}, // 156
                {amdgpu_gfx1250_op_V_CMPX_NEQ_F32,"V_CMPX_NEQ_F32"}, // 157
                {amdgpu_gfx1250_op_V_CMPX_NLT_F32,"V_CMPX_NLT_F32"}, // 158
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 159
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 160
                {amdgpu_gfx1250_op_V_CMPX_LT_F64,"V_CMPX_LT_F64"}, // 161
                {amdgpu_gfx1250_op_V_CMPX_EQ_F64,"V_CMPX_EQ_F64"}, // 162
                {amdgpu_gfx1250_op_V_CMPX_LE_F64,"V_CMPX_LE_F64"}, // 163
                {amdgpu_gfx1250_op_V_CMPX_GT_F64,"V_CMPX_GT_F64"}, // 164
                {amdgpu_gfx1250_op_V_CMPX_LG_F64,"V_CMPX_LG_F64"}, // 165
                {amdgpu_gfx1250_op_V_CMPX_GE_F64,"V_CMPX_GE_F64"}, // 166
                {amdgpu_gfx1250_op_V_CMPX_O_F64,"V_CMPX_O_F64"}, // 167
                {amdgpu_gfx1250_op_V_CMPX_U_F64,"V_CMPX_U_F64"}, // 168
                {amdgpu_gfx1250_op_V_CMPX_NGE_F64,"V_CMPX_NGE_F64"}, // 169
                {amdgpu_gfx1250_op_V_CMPX_NLG_F64,"V_CMPX_NLG_F64"}, // 170
                {amdgpu_gfx1250_op_V_CMPX_NGT_F64,"V_CMPX_NGT_F64"}, // 171
                {amdgpu_gfx1250_op_V_CMPX_NLE_F64,"V_CMPX_NLE_F64"}, // 172
                {amdgpu_gfx1250_op_V_CMPX_NEQ_F64,"V_CMPX_NEQ_F64"}, // 173
                {amdgpu_gfx1250_op_V_CMPX_NLT_F64,"V_CMPX_NLT_F64"}, // 174
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 175
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 176
                {amdgpu_gfx1250_op_V_CMPX_LT_I16,"V_CMPX_LT_I16"}, // 177
                {amdgpu_gfx1250_op_V_CMPX_EQ_I16,"V_CMPX_EQ_I16"}, // 178
                {amdgpu_gfx1250_op_V_CMPX_LE_I16,"V_CMPX_LE_I16"}, // 179
                {amdgpu_gfx1250_op_V_CMPX_GT_I16,"V_CMPX_GT_I16"}, // 180
                {amdgpu_gfx1250_op_V_CMPX_NE_I16,"V_CMPX_NE_I16"}, // 181
                {amdgpu_gfx1250_op_V_CMPX_GE_I16,"V_CMPX_GE_I16"}, // 182
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 183
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 184
                {amdgpu_gfx1250_op_V_CMPX_LT_U16,"V_CMPX_LT_U16"}, // 185
                {amdgpu_gfx1250_op_V_CMPX_EQ_U16,"V_CMPX_EQ_U16"}, // 186
                {amdgpu_gfx1250_op_V_CMPX_LE_U16,"V_CMPX_LE_U16"}, // 187
                {amdgpu_gfx1250_op_V_CMPX_GT_U16,"V_CMPX_GT_U16"}, // 188
                {amdgpu_gfx1250_op_V_CMPX_NE_U16,"V_CMPX_NE_U16"}, // 189
                {amdgpu_gfx1250_op_V_CMPX_GE_U16,"V_CMPX_GE_U16"}, // 190
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 191
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 192
                {amdgpu_gfx1250_op_V_CMPX_LT_I32,"V_CMPX_LT_I32"}, // 193
                {amdgpu_gfx1250_op_V_CMPX_EQ_I32,"V_CMPX_EQ_I32"}, // 194
                {amdgpu_gfx1250_op_V_CMPX_LE_I32,"V_CMPX_LE_I32"}, // 195
                {amdgpu_gfx1250_op_V_CMPX_GT_I32,"V_CMPX_GT_I32"}, // 196
                {amdgpu_gfx1250_op_V_CMPX_NE_I32,"V_CMPX_NE_I32"}, // 197
                {amdgpu_gfx1250_op_V_CMPX_GE_I32,"V_CMPX_GE_I32"}, // 198
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 199
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 200
                {amdgpu_gfx1250_op_V_CMPX_LT_U32,"V_CMPX_LT_U32"}, // 201
                {amdgpu_gfx1250_op_V_CMPX_EQ_U32,"V_CMPX_EQ_U32"}, // 202
                {amdgpu_gfx1250_op_V_CMPX_LE_U32,"V_CMPX_LE_U32"}, // 203
                {amdgpu_gfx1250_op_V_CMPX_GT_U32,"V_CMPX_GT_U32"}, // 204
                {amdgpu_gfx1250_op_V_CMPX_NE_U32,"V_CMPX_NE_U32"}, // 205
                {amdgpu_gfx1250_op_V_CMPX_GE_U32,"V_CMPX_GE_U32"}, // 206
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 207
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 208
                {amdgpu_gfx1250_op_V_CMPX_LT_I64,"V_CMPX_LT_I64"}, // 209
                {amdgpu_gfx1250_op_V_CMPX_EQ_I64,"V_CMPX_EQ_I64"}, // 210
                {amdgpu_gfx1250_op_V_CMPX_LE_I64,"V_CMPX_LE_I64"}, // 211
                {amdgpu_gfx1250_op_V_CMPX_GT_I64,"V_CMPX_GT_I64"}, // 212
                {amdgpu_gfx1250_op_V_CMPX_NE_I64,"V_CMPX_NE_I64"}, // 213
                {amdgpu_gfx1250_op_V_CMPX_GE_I64,"V_CMPX_GE_I64"}, // 214
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 215
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 216
                {amdgpu_gfx1250_op_V_CMPX_LT_U64,"V_CMPX_LT_U64"}, // 217
                {amdgpu_gfx1250_op_V_CMPX_EQ_U64,"V_CMPX_EQ_U64"}, // 218
                {amdgpu_gfx1250_op_V_CMPX_LE_U64,"V_CMPX_LE_U64"}, // 219
                {amdgpu_gfx1250_op_V_CMPX_GT_U64,"V_CMPX_GT_U64"}, // 220
                {amdgpu_gfx1250_op_V_CMPX_NE_U64,"V_CMPX_NE_U64"}, // 221
                {amdgpu_gfx1250_op_V_CMPX_GE_U64,"V_CMPX_GE_U64"}, // 222
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 223
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 224
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 225
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 226
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 227
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 228
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 229
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 230
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 231
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 232
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 233
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 234
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 235
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 236
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 237
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 238
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 239
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 240
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 241
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 242
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 243
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 244
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 245
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 246
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 247
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 248
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 249
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 250
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 251
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 252
                {amdgpu_gfx1250_op_V_CMPX_CLASS_F16,"V_CMPX_CLASS_F16"}, // 253
                {amdgpu_gfx1250_op_V_CMPX_CLASS_F32,"V_CMPX_CLASS_F32"}, // 254
                {amdgpu_gfx1250_op_V_CMPX_CLASS_F64,"V_CMPX_CLASS_F64"}, // 255
            }; // end VOPC_INST_LITERAL_insn_table
            const amdgpu_gfx1250_insn_entry VOPC_INST_LITERAL64_insn_table[256] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_V_CMP_LT_F16,"V_CMP_LT_F16"}, // 1
                {amdgpu_gfx1250_op_V_CMP_EQ_F16,"V_CMP_EQ_F16"}, // 2
                {amdgpu_gfx1250_op_V_CMP_LE_F16,"V_CMP_LE_F16"}, // 3
                {amdgpu_gfx1250_op_V_CMP_GT_F16,"V_CMP_GT_F16"}, // 4
                {amdgpu_gfx1250_op_V_CMP_LG_F16,"V_CMP_LG_F16"}, // 5
                {amdgpu_gfx1250_op_V_CMP_GE_F16,"V_CMP_GE_F16"}, // 6
                {amdgpu_gfx1250_op_V_CMP_O_F16,"V_CMP_O_F16"}, // 7
                {amdgpu_gfx1250_op_V_CMP_U_F16,"V_CMP_U_F16"}, // 8
                {amdgpu_gfx1250_op_V_CMP_NGE_F16,"V_CMP_NGE_F16"}, // 9
                {amdgpu_gfx1250_op_V_CMP_NLG_F16,"V_CMP_NLG_F16"}, // 10
                {amdgpu_gfx1250_op_V_CMP_NGT_F16,"V_CMP_NGT_F16"}, // 11
                {amdgpu_gfx1250_op_V_CMP_NLE_F16,"V_CMP_NLE_F16"}, // 12
                {amdgpu_gfx1250_op_V_CMP_NEQ_F16,"V_CMP_NEQ_F16"}, // 13
                {amdgpu_gfx1250_op_V_CMP_NLT_F16,"V_CMP_NLT_F16"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_V_CMP_LT_F32,"V_CMP_LT_F32"}, // 17
                {amdgpu_gfx1250_op_V_CMP_EQ_F32,"V_CMP_EQ_F32"}, // 18
                {amdgpu_gfx1250_op_V_CMP_LE_F32,"V_CMP_LE_F32"}, // 19
                {amdgpu_gfx1250_op_V_CMP_GT_F32,"V_CMP_GT_F32"}, // 20
                {amdgpu_gfx1250_op_V_CMP_LG_F32,"V_CMP_LG_F32"}, // 21
                {amdgpu_gfx1250_op_V_CMP_GE_F32,"V_CMP_GE_F32"}, // 22
                {amdgpu_gfx1250_op_V_CMP_O_F32,"V_CMP_O_F32"}, // 23
                {amdgpu_gfx1250_op_V_CMP_U_F32,"V_CMP_U_F32"}, // 24
                {amdgpu_gfx1250_op_V_CMP_NGE_F32,"V_CMP_NGE_F32"}, // 25
                {amdgpu_gfx1250_op_V_CMP_NLG_F32,"V_CMP_NLG_F32"}, // 26
                {amdgpu_gfx1250_op_V_CMP_NGT_F32,"V_CMP_NGT_F32"}, // 27
                {amdgpu_gfx1250_op_V_CMP_NLE_F32,"V_CMP_NLE_F32"}, // 28
                {amdgpu_gfx1250_op_V_CMP_NEQ_F32,"V_CMP_NEQ_F32"}, // 29
                {amdgpu_gfx1250_op_V_CMP_NLT_F32,"V_CMP_NLT_F32"}, // 30
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 31
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 32
                {amdgpu_gfx1250_op_V_CMP_LT_F64,"V_CMP_LT_F64"}, // 33
                {amdgpu_gfx1250_op_V_CMP_EQ_F64,"V_CMP_EQ_F64"}, // 34
                {amdgpu_gfx1250_op_V_CMP_LE_F64,"V_CMP_LE_F64"}, // 35
                {amdgpu_gfx1250_op_V_CMP_GT_F64,"V_CMP_GT_F64"}, // 36
                {amdgpu_gfx1250_op_V_CMP_LG_F64,"V_CMP_LG_F64"}, // 37
                {amdgpu_gfx1250_op_V_CMP_GE_F64,"V_CMP_GE_F64"}, // 38
                {amdgpu_gfx1250_op_V_CMP_O_F64,"V_CMP_O_F64"}, // 39
                {amdgpu_gfx1250_op_V_CMP_U_F64,"V_CMP_U_F64"}, // 40
                {amdgpu_gfx1250_op_V_CMP_NGE_F64,"V_CMP_NGE_F64"}, // 41
                {amdgpu_gfx1250_op_V_CMP_NLG_F64,"V_CMP_NLG_F64"}, // 42
                {amdgpu_gfx1250_op_V_CMP_NGT_F64,"V_CMP_NGT_F64"}, // 43
                {amdgpu_gfx1250_op_V_CMP_NLE_F64,"V_CMP_NLE_F64"}, // 44
                {amdgpu_gfx1250_op_V_CMP_NEQ_F64,"V_CMP_NEQ_F64"}, // 45
                {amdgpu_gfx1250_op_V_CMP_NLT_F64,"V_CMP_NLT_F64"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_V_CMP_LT_I16,"V_CMP_LT_I16"}, // 49
                {amdgpu_gfx1250_op_V_CMP_EQ_I16,"V_CMP_EQ_I16"}, // 50
                {amdgpu_gfx1250_op_V_CMP_LE_I16,"V_CMP_LE_I16"}, // 51
                {amdgpu_gfx1250_op_V_CMP_GT_I16,"V_CMP_GT_I16"}, // 52
                {amdgpu_gfx1250_op_V_CMP_NE_I16,"V_CMP_NE_I16"}, // 53
                {amdgpu_gfx1250_op_V_CMP_GE_I16,"V_CMP_GE_I16"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_V_CMP_LT_U16,"V_CMP_LT_U16"}, // 57
                {amdgpu_gfx1250_op_V_CMP_EQ_U16,"V_CMP_EQ_U16"}, // 58
                {amdgpu_gfx1250_op_V_CMP_LE_U16,"V_CMP_LE_U16"}, // 59
                {amdgpu_gfx1250_op_V_CMP_GT_U16,"V_CMP_GT_U16"}, // 60
                {amdgpu_gfx1250_op_V_CMP_NE_U16,"V_CMP_NE_U16"}, // 61
                {amdgpu_gfx1250_op_V_CMP_GE_U16,"V_CMP_GE_U16"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 64
                {amdgpu_gfx1250_op_V_CMP_LT_I32,"V_CMP_LT_I32"}, // 65
                {amdgpu_gfx1250_op_V_CMP_EQ_I32,"V_CMP_EQ_I32"}, // 66
                {amdgpu_gfx1250_op_V_CMP_LE_I32,"V_CMP_LE_I32"}, // 67
                {amdgpu_gfx1250_op_V_CMP_GT_I32,"V_CMP_GT_I32"}, // 68
                {amdgpu_gfx1250_op_V_CMP_NE_I32,"V_CMP_NE_I32"}, // 69
                {amdgpu_gfx1250_op_V_CMP_GE_I32,"V_CMP_GE_I32"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 72
                {amdgpu_gfx1250_op_V_CMP_LT_U32,"V_CMP_LT_U32"}, // 73
                {amdgpu_gfx1250_op_V_CMP_EQ_U32,"V_CMP_EQ_U32"}, // 74
                {amdgpu_gfx1250_op_V_CMP_LE_U32,"V_CMP_LE_U32"}, // 75
                {amdgpu_gfx1250_op_V_CMP_GT_U32,"V_CMP_GT_U32"}, // 76
                {amdgpu_gfx1250_op_V_CMP_NE_U32,"V_CMP_NE_U32"}, // 77
                {amdgpu_gfx1250_op_V_CMP_GE_U32,"V_CMP_GE_U32"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 80
                {amdgpu_gfx1250_op_V_CMP_LT_I64,"V_CMP_LT_I64"}, // 81
                {amdgpu_gfx1250_op_V_CMP_EQ_I64,"V_CMP_EQ_I64"}, // 82
                {amdgpu_gfx1250_op_V_CMP_LE_I64,"V_CMP_LE_I64"}, // 83
                {amdgpu_gfx1250_op_V_CMP_GT_I64,"V_CMP_GT_I64"}, // 84
                {amdgpu_gfx1250_op_V_CMP_NE_I64,"V_CMP_NE_I64"}, // 85
                {amdgpu_gfx1250_op_V_CMP_GE_I64,"V_CMP_GE_I64"}, // 86
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 87
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 88
                {amdgpu_gfx1250_op_V_CMP_LT_U64,"V_CMP_LT_U64"}, // 89
                {amdgpu_gfx1250_op_V_CMP_EQ_U64,"V_CMP_EQ_U64"}, // 90
                {amdgpu_gfx1250_op_V_CMP_LE_U64,"V_CMP_LE_U64"}, // 91
                {amdgpu_gfx1250_op_V_CMP_GT_U64,"V_CMP_GT_U64"}, // 92
                {amdgpu_gfx1250_op_V_CMP_NE_U64,"V_CMP_NE_U64"}, // 93
                {amdgpu_gfx1250_op_V_CMP_GE_U64,"V_CMP_GE_U64"}, // 94
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 95
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 96
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 97
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 98
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 99
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 100
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 101
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 102
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 103
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 104
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 105
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 106
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 107
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 108
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 109
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 110
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 111
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 112
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 113
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 114
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 115
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 116
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 117
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 118
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 119
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 120
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 121
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 122
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 123
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 124
                {amdgpu_gfx1250_op_V_CMP_CLASS_F16,"V_CMP_CLASS_F16"}, // 125
                {amdgpu_gfx1250_op_V_CMP_CLASS_F32,"V_CMP_CLASS_F32"}, // 126
                {amdgpu_gfx1250_op_V_CMP_CLASS_F64,"V_CMP_CLASS_F64"}, // 127
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 128
                {amdgpu_gfx1250_op_V_CMPX_LT_F16,"V_CMPX_LT_F16"}, // 129
                {amdgpu_gfx1250_op_V_CMPX_EQ_F16,"V_CMPX_EQ_F16"}, // 130
                {amdgpu_gfx1250_op_V_CMPX_LE_F16,"V_CMPX_LE_F16"}, // 131
                {amdgpu_gfx1250_op_V_CMPX_GT_F16,"V_CMPX_GT_F16"}, // 132
                {amdgpu_gfx1250_op_V_CMPX_LG_F16,"V_CMPX_LG_F16"}, // 133
                {amdgpu_gfx1250_op_V_CMPX_GE_F16,"V_CMPX_GE_F16"}, // 134
                {amdgpu_gfx1250_op_V_CMPX_O_F16,"V_CMPX_O_F16"}, // 135
                {amdgpu_gfx1250_op_V_CMPX_U_F16,"V_CMPX_U_F16"}, // 136
                {amdgpu_gfx1250_op_V_CMPX_NGE_F16,"V_CMPX_NGE_F16"}, // 137
                {amdgpu_gfx1250_op_V_CMPX_NLG_F16,"V_CMPX_NLG_F16"}, // 138
                {amdgpu_gfx1250_op_V_CMPX_NGT_F16,"V_CMPX_NGT_F16"}, // 139
                {amdgpu_gfx1250_op_V_CMPX_NLE_F16,"V_CMPX_NLE_F16"}, // 140
                {amdgpu_gfx1250_op_V_CMPX_NEQ_F16,"V_CMPX_NEQ_F16"}, // 141
                {amdgpu_gfx1250_op_V_CMPX_NLT_F16,"V_CMPX_NLT_F16"}, // 142
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 143
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 144
                {amdgpu_gfx1250_op_V_CMPX_LT_F32,"V_CMPX_LT_F32"}, // 145
                {amdgpu_gfx1250_op_V_CMPX_EQ_F32,"V_CMPX_EQ_F32"}, // 146
                {amdgpu_gfx1250_op_V_CMPX_LE_F32,"V_CMPX_LE_F32"}, // 147
                {amdgpu_gfx1250_op_V_CMPX_GT_F32,"V_CMPX_GT_F32"}, // 148
                {amdgpu_gfx1250_op_V_CMPX_LG_F32,"V_CMPX_LG_F32"}, // 149
                {amdgpu_gfx1250_op_V_CMPX_GE_F32,"V_CMPX_GE_F32"}, // 150
                {amdgpu_gfx1250_op_V_CMPX_O_F32,"V_CMPX_O_F32"}, // 151
                {amdgpu_gfx1250_op_V_CMPX_U_F32,"V_CMPX_U_F32"}, // 152
                {amdgpu_gfx1250_op_V_CMPX_NGE_F32,"V_CMPX_NGE_F32"}, // 153
                {amdgpu_gfx1250_op_V_CMPX_NLG_F32,"V_CMPX_NLG_F32"}, // 154
                {amdgpu_gfx1250_op_V_CMPX_NGT_F32,"V_CMPX_NGT_F32"}, // 155
                {amdgpu_gfx1250_op_V_CMPX_NLE_F32,"V_CMPX_NLE_F32"}, // 156
                {amdgpu_gfx1250_op_V_CMPX_NEQ_F32,"V_CMPX_NEQ_F32"}, // 157
                {amdgpu_gfx1250_op_V_CMPX_NLT_F32,"V_CMPX_NLT_F32"}, // 158
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 159
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 160
                {amdgpu_gfx1250_op_V_CMPX_LT_F64,"V_CMPX_LT_F64"}, // 161
                {amdgpu_gfx1250_op_V_CMPX_EQ_F64,"V_CMPX_EQ_F64"}, // 162
                {amdgpu_gfx1250_op_V_CMPX_LE_F64,"V_CMPX_LE_F64"}, // 163
                {amdgpu_gfx1250_op_V_CMPX_GT_F64,"V_CMPX_GT_F64"}, // 164
                {amdgpu_gfx1250_op_V_CMPX_LG_F64,"V_CMPX_LG_F64"}, // 165
                {amdgpu_gfx1250_op_V_CMPX_GE_F64,"V_CMPX_GE_F64"}, // 166
                {amdgpu_gfx1250_op_V_CMPX_O_F64,"V_CMPX_O_F64"}, // 167
                {amdgpu_gfx1250_op_V_CMPX_U_F64,"V_CMPX_U_F64"}, // 168
                {amdgpu_gfx1250_op_V_CMPX_NGE_F64,"V_CMPX_NGE_F64"}, // 169
                {amdgpu_gfx1250_op_V_CMPX_NLG_F64,"V_CMPX_NLG_F64"}, // 170
                {amdgpu_gfx1250_op_V_CMPX_NGT_F64,"V_CMPX_NGT_F64"}, // 171
                {amdgpu_gfx1250_op_V_CMPX_NLE_F64,"V_CMPX_NLE_F64"}, // 172
                {amdgpu_gfx1250_op_V_CMPX_NEQ_F64,"V_CMPX_NEQ_F64"}, // 173
                {amdgpu_gfx1250_op_V_CMPX_NLT_F64,"V_CMPX_NLT_F64"}, // 174
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 175
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 176
                {amdgpu_gfx1250_op_V_CMPX_LT_I16,"V_CMPX_LT_I16"}, // 177
                {amdgpu_gfx1250_op_V_CMPX_EQ_I16,"V_CMPX_EQ_I16"}, // 178
                {amdgpu_gfx1250_op_V_CMPX_LE_I16,"V_CMPX_LE_I16"}, // 179
                {amdgpu_gfx1250_op_V_CMPX_GT_I16,"V_CMPX_GT_I16"}, // 180
                {amdgpu_gfx1250_op_V_CMPX_NE_I16,"V_CMPX_NE_I16"}, // 181
                {amdgpu_gfx1250_op_V_CMPX_GE_I16,"V_CMPX_GE_I16"}, // 182
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 183
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 184
                {amdgpu_gfx1250_op_V_CMPX_LT_U16,"V_CMPX_LT_U16"}, // 185
                {amdgpu_gfx1250_op_V_CMPX_EQ_U16,"V_CMPX_EQ_U16"}, // 186
                {amdgpu_gfx1250_op_V_CMPX_LE_U16,"V_CMPX_LE_U16"}, // 187
                {amdgpu_gfx1250_op_V_CMPX_GT_U16,"V_CMPX_GT_U16"}, // 188
                {amdgpu_gfx1250_op_V_CMPX_NE_U16,"V_CMPX_NE_U16"}, // 189
                {amdgpu_gfx1250_op_V_CMPX_GE_U16,"V_CMPX_GE_U16"}, // 190
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 191
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 192
                {amdgpu_gfx1250_op_V_CMPX_LT_I32,"V_CMPX_LT_I32"}, // 193
                {amdgpu_gfx1250_op_V_CMPX_EQ_I32,"V_CMPX_EQ_I32"}, // 194
                {amdgpu_gfx1250_op_V_CMPX_LE_I32,"V_CMPX_LE_I32"}, // 195
                {amdgpu_gfx1250_op_V_CMPX_GT_I32,"V_CMPX_GT_I32"}, // 196
                {amdgpu_gfx1250_op_V_CMPX_NE_I32,"V_CMPX_NE_I32"}, // 197
                {amdgpu_gfx1250_op_V_CMPX_GE_I32,"V_CMPX_GE_I32"}, // 198
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 199
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 200
                {amdgpu_gfx1250_op_V_CMPX_LT_U32,"V_CMPX_LT_U32"}, // 201
                {amdgpu_gfx1250_op_V_CMPX_EQ_U32,"V_CMPX_EQ_U32"}, // 202
                {amdgpu_gfx1250_op_V_CMPX_LE_U32,"V_CMPX_LE_U32"}, // 203
                {amdgpu_gfx1250_op_V_CMPX_GT_U32,"V_CMPX_GT_U32"}, // 204
                {amdgpu_gfx1250_op_V_CMPX_NE_U32,"V_CMPX_NE_U32"}, // 205
                {amdgpu_gfx1250_op_V_CMPX_GE_U32,"V_CMPX_GE_U32"}, // 206
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 207
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 208
                {amdgpu_gfx1250_op_V_CMPX_LT_I64,"V_CMPX_LT_I64"}, // 209
                {amdgpu_gfx1250_op_V_CMPX_EQ_I64,"V_CMPX_EQ_I64"}, // 210
                {amdgpu_gfx1250_op_V_CMPX_LE_I64,"V_CMPX_LE_I64"}, // 211
                {amdgpu_gfx1250_op_V_CMPX_GT_I64,"V_CMPX_GT_I64"}, // 212
                {amdgpu_gfx1250_op_V_CMPX_NE_I64,"V_CMPX_NE_I64"}, // 213
                {amdgpu_gfx1250_op_V_CMPX_GE_I64,"V_CMPX_GE_I64"}, // 214
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 215
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 216
                {amdgpu_gfx1250_op_V_CMPX_LT_U64,"V_CMPX_LT_U64"}, // 217
                {amdgpu_gfx1250_op_V_CMPX_EQ_U64,"V_CMPX_EQ_U64"}, // 218
                {amdgpu_gfx1250_op_V_CMPX_LE_U64,"V_CMPX_LE_U64"}, // 219
                {amdgpu_gfx1250_op_V_CMPX_GT_U64,"V_CMPX_GT_U64"}, // 220
                {amdgpu_gfx1250_op_V_CMPX_NE_U64,"V_CMPX_NE_U64"}, // 221
                {amdgpu_gfx1250_op_V_CMPX_GE_U64,"V_CMPX_GE_U64"}, // 222
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 223
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 224
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 225
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 226
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 227
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 228
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 229
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 230
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 231
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 232
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 233
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 234
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 235
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 236
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 237
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 238
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 239
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 240
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 241
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 242
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 243
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 244
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 245
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 246
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 247
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 248
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 249
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 250
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 251
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 252
                {amdgpu_gfx1250_op_V_CMPX_CLASS_F16,"V_CMPX_CLASS_F16"}, // 253
                {amdgpu_gfx1250_op_V_CMPX_CLASS_F32,"V_CMPX_CLASS_F32"}, // 254
                {amdgpu_gfx1250_op_V_CMPX_CLASS_F64,"V_CMPX_CLASS_F64"}, // 255
            }; // end VOPC_INST_LITERAL64_insn_table
            const amdgpu_gfx1250_insn_entry VOPC_VOP_DPP8_insn_table[255] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_V_CMP_LT_F16,"V_CMP_LT_F16"}, // 1
                {amdgpu_gfx1250_op_V_CMP_EQ_F16,"V_CMP_EQ_F16"}, // 2
                {amdgpu_gfx1250_op_V_CMP_LE_F16,"V_CMP_LE_F16"}, // 3
                {amdgpu_gfx1250_op_V_CMP_GT_F16,"V_CMP_GT_F16"}, // 4
                {amdgpu_gfx1250_op_V_CMP_LG_F16,"V_CMP_LG_F16"}, // 5
                {amdgpu_gfx1250_op_V_CMP_GE_F16,"V_CMP_GE_F16"}, // 6
                {amdgpu_gfx1250_op_V_CMP_O_F16,"V_CMP_O_F16"}, // 7
                {amdgpu_gfx1250_op_V_CMP_U_F16,"V_CMP_U_F16"}, // 8
                {amdgpu_gfx1250_op_V_CMP_NGE_F16,"V_CMP_NGE_F16"}, // 9
                {amdgpu_gfx1250_op_V_CMP_NLG_F16,"V_CMP_NLG_F16"}, // 10
                {amdgpu_gfx1250_op_V_CMP_NGT_F16,"V_CMP_NGT_F16"}, // 11
                {amdgpu_gfx1250_op_V_CMP_NLE_F16,"V_CMP_NLE_F16"}, // 12
                {amdgpu_gfx1250_op_V_CMP_NEQ_F16,"V_CMP_NEQ_F16"}, // 13
                {amdgpu_gfx1250_op_V_CMP_NLT_F16,"V_CMP_NLT_F16"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_V_CMP_LT_F32,"V_CMP_LT_F32"}, // 17
                {amdgpu_gfx1250_op_V_CMP_EQ_F32,"V_CMP_EQ_F32"}, // 18
                {amdgpu_gfx1250_op_V_CMP_LE_F32,"V_CMP_LE_F32"}, // 19
                {amdgpu_gfx1250_op_V_CMP_GT_F32,"V_CMP_GT_F32"}, // 20
                {amdgpu_gfx1250_op_V_CMP_LG_F32,"V_CMP_LG_F32"}, // 21
                {amdgpu_gfx1250_op_V_CMP_GE_F32,"V_CMP_GE_F32"}, // 22
                {amdgpu_gfx1250_op_V_CMP_O_F32,"V_CMP_O_F32"}, // 23
                {amdgpu_gfx1250_op_V_CMP_U_F32,"V_CMP_U_F32"}, // 24
                {amdgpu_gfx1250_op_V_CMP_NGE_F32,"V_CMP_NGE_F32"}, // 25
                {amdgpu_gfx1250_op_V_CMP_NLG_F32,"V_CMP_NLG_F32"}, // 26
                {amdgpu_gfx1250_op_V_CMP_NGT_F32,"V_CMP_NGT_F32"}, // 27
                {amdgpu_gfx1250_op_V_CMP_NLE_F32,"V_CMP_NLE_F32"}, // 28
                {amdgpu_gfx1250_op_V_CMP_NEQ_F32,"V_CMP_NEQ_F32"}, // 29
                {amdgpu_gfx1250_op_V_CMP_NLT_F32,"V_CMP_NLT_F32"}, // 30
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 31
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 32
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 33
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 34
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 35
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 36
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 42
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_V_CMP_LT_I16,"V_CMP_LT_I16"}, // 49
                {amdgpu_gfx1250_op_V_CMP_EQ_I16,"V_CMP_EQ_I16"}, // 50
                {amdgpu_gfx1250_op_V_CMP_LE_I16,"V_CMP_LE_I16"}, // 51
                {amdgpu_gfx1250_op_V_CMP_GT_I16,"V_CMP_GT_I16"}, // 52
                {amdgpu_gfx1250_op_V_CMP_NE_I16,"V_CMP_NE_I16"}, // 53
                {amdgpu_gfx1250_op_V_CMP_GE_I16,"V_CMP_GE_I16"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_V_CMP_LT_U16,"V_CMP_LT_U16"}, // 57
                {amdgpu_gfx1250_op_V_CMP_EQ_U16,"V_CMP_EQ_U16"}, // 58
                {amdgpu_gfx1250_op_V_CMP_LE_U16,"V_CMP_LE_U16"}, // 59
                {amdgpu_gfx1250_op_V_CMP_GT_U16,"V_CMP_GT_U16"}, // 60
                {amdgpu_gfx1250_op_V_CMP_NE_U16,"V_CMP_NE_U16"}, // 61
                {amdgpu_gfx1250_op_V_CMP_GE_U16,"V_CMP_GE_U16"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 64
                {amdgpu_gfx1250_op_V_CMP_LT_I32,"V_CMP_LT_I32"}, // 65
                {amdgpu_gfx1250_op_V_CMP_EQ_I32,"V_CMP_EQ_I32"}, // 66
                {amdgpu_gfx1250_op_V_CMP_LE_I32,"V_CMP_LE_I32"}, // 67
                {amdgpu_gfx1250_op_V_CMP_GT_I32,"V_CMP_GT_I32"}, // 68
                {amdgpu_gfx1250_op_V_CMP_NE_I32,"V_CMP_NE_I32"}, // 69
                {amdgpu_gfx1250_op_V_CMP_GE_I32,"V_CMP_GE_I32"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 72
                {amdgpu_gfx1250_op_V_CMP_LT_U32,"V_CMP_LT_U32"}, // 73
                {amdgpu_gfx1250_op_V_CMP_EQ_U32,"V_CMP_EQ_U32"}, // 74
                {amdgpu_gfx1250_op_V_CMP_LE_U32,"V_CMP_LE_U32"}, // 75
                {amdgpu_gfx1250_op_V_CMP_GT_U32,"V_CMP_GT_U32"}, // 76
                {amdgpu_gfx1250_op_V_CMP_NE_U32,"V_CMP_NE_U32"}, // 77
                {amdgpu_gfx1250_op_V_CMP_GE_U32,"V_CMP_GE_U32"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 80
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 81
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 82
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 83
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 84
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 85
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 86
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 87
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 88
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 89
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 90
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 91
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 92
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 93
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 94
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 95
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 96
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 97
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 98
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 99
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 100
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 101
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 102
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 103
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 104
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 105
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 106
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 107
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 108
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 109
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 110
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 111
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 112
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 113
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 114
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 115
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 116
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 117
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 118
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 119
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 120
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 121
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 122
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 123
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 124
                {amdgpu_gfx1250_op_V_CMP_CLASS_F16,"V_CMP_CLASS_F16"}, // 125
                {amdgpu_gfx1250_op_V_CMP_CLASS_F32,"V_CMP_CLASS_F32"}, // 126
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 127
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 128
                {amdgpu_gfx1250_op_V_CMPX_LT_F16,"V_CMPX_LT_F16"}, // 129
                {amdgpu_gfx1250_op_V_CMPX_EQ_F16,"V_CMPX_EQ_F16"}, // 130
                {amdgpu_gfx1250_op_V_CMPX_LE_F16,"V_CMPX_LE_F16"}, // 131
                {amdgpu_gfx1250_op_V_CMPX_GT_F16,"V_CMPX_GT_F16"}, // 132
                {amdgpu_gfx1250_op_V_CMPX_LG_F16,"V_CMPX_LG_F16"}, // 133
                {amdgpu_gfx1250_op_V_CMPX_GE_F16,"V_CMPX_GE_F16"}, // 134
                {amdgpu_gfx1250_op_V_CMPX_O_F16,"V_CMPX_O_F16"}, // 135
                {amdgpu_gfx1250_op_V_CMPX_U_F16,"V_CMPX_U_F16"}, // 136
                {amdgpu_gfx1250_op_V_CMPX_NGE_F16,"V_CMPX_NGE_F16"}, // 137
                {amdgpu_gfx1250_op_V_CMPX_NLG_F16,"V_CMPX_NLG_F16"}, // 138
                {amdgpu_gfx1250_op_V_CMPX_NGT_F16,"V_CMPX_NGT_F16"}, // 139
                {amdgpu_gfx1250_op_V_CMPX_NLE_F16,"V_CMPX_NLE_F16"}, // 140
                {amdgpu_gfx1250_op_V_CMPX_NEQ_F16,"V_CMPX_NEQ_F16"}, // 141
                {amdgpu_gfx1250_op_V_CMPX_NLT_F16,"V_CMPX_NLT_F16"}, // 142
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 143
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 144
                {amdgpu_gfx1250_op_V_CMPX_LT_F32,"V_CMPX_LT_F32"}, // 145
                {amdgpu_gfx1250_op_V_CMPX_EQ_F32,"V_CMPX_EQ_F32"}, // 146
                {amdgpu_gfx1250_op_V_CMPX_LE_F32,"V_CMPX_LE_F32"}, // 147
                {amdgpu_gfx1250_op_V_CMPX_GT_F32,"V_CMPX_GT_F32"}, // 148
                {amdgpu_gfx1250_op_V_CMPX_LG_F32,"V_CMPX_LG_F32"}, // 149
                {amdgpu_gfx1250_op_V_CMPX_GE_F32,"V_CMPX_GE_F32"}, // 150
                {amdgpu_gfx1250_op_V_CMPX_O_F32,"V_CMPX_O_F32"}, // 151
                {amdgpu_gfx1250_op_V_CMPX_U_F32,"V_CMPX_U_F32"}, // 152
                {amdgpu_gfx1250_op_V_CMPX_NGE_F32,"V_CMPX_NGE_F32"}, // 153
                {amdgpu_gfx1250_op_V_CMPX_NLG_F32,"V_CMPX_NLG_F32"}, // 154
                {amdgpu_gfx1250_op_V_CMPX_NGT_F32,"V_CMPX_NGT_F32"}, // 155
                {amdgpu_gfx1250_op_V_CMPX_NLE_F32,"V_CMPX_NLE_F32"}, // 156
                {amdgpu_gfx1250_op_V_CMPX_NEQ_F32,"V_CMPX_NEQ_F32"}, // 157
                {amdgpu_gfx1250_op_V_CMPX_NLT_F32,"V_CMPX_NLT_F32"}, // 158
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 159
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 160
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 161
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 162
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 163
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 164
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 165
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 166
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 167
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 168
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 169
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 170
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 171
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 172
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 173
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 174
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 175
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 176
                {amdgpu_gfx1250_op_V_CMPX_LT_I16,"V_CMPX_LT_I16"}, // 177
                {amdgpu_gfx1250_op_V_CMPX_EQ_I16,"V_CMPX_EQ_I16"}, // 178
                {amdgpu_gfx1250_op_V_CMPX_LE_I16,"V_CMPX_LE_I16"}, // 179
                {amdgpu_gfx1250_op_V_CMPX_GT_I16,"V_CMPX_GT_I16"}, // 180
                {amdgpu_gfx1250_op_V_CMPX_NE_I16,"V_CMPX_NE_I16"}, // 181
                {amdgpu_gfx1250_op_V_CMPX_GE_I16,"V_CMPX_GE_I16"}, // 182
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 183
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 184
                {amdgpu_gfx1250_op_V_CMPX_LT_U16,"V_CMPX_LT_U16"}, // 185
                {amdgpu_gfx1250_op_V_CMPX_EQ_U16,"V_CMPX_EQ_U16"}, // 186
                {amdgpu_gfx1250_op_V_CMPX_LE_U16,"V_CMPX_LE_U16"}, // 187
                {amdgpu_gfx1250_op_V_CMPX_GT_U16,"V_CMPX_GT_U16"}, // 188
                {amdgpu_gfx1250_op_V_CMPX_NE_U16,"V_CMPX_NE_U16"}, // 189
                {amdgpu_gfx1250_op_V_CMPX_GE_U16,"V_CMPX_GE_U16"}, // 190
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 191
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 192
                {amdgpu_gfx1250_op_V_CMPX_LT_I32,"V_CMPX_LT_I32"}, // 193
                {amdgpu_gfx1250_op_V_CMPX_EQ_I32,"V_CMPX_EQ_I32"}, // 194
                {amdgpu_gfx1250_op_V_CMPX_LE_I32,"V_CMPX_LE_I32"}, // 195
                {amdgpu_gfx1250_op_V_CMPX_GT_I32,"V_CMPX_GT_I32"}, // 196
                {amdgpu_gfx1250_op_V_CMPX_NE_I32,"V_CMPX_NE_I32"}, // 197
                {amdgpu_gfx1250_op_V_CMPX_GE_I32,"V_CMPX_GE_I32"}, // 198
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 199
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 200
                {amdgpu_gfx1250_op_V_CMPX_LT_U32,"V_CMPX_LT_U32"}, // 201
                {amdgpu_gfx1250_op_V_CMPX_EQ_U32,"V_CMPX_EQ_U32"}, // 202
                {amdgpu_gfx1250_op_V_CMPX_LE_U32,"V_CMPX_LE_U32"}, // 203
                {amdgpu_gfx1250_op_V_CMPX_GT_U32,"V_CMPX_GT_U32"}, // 204
                {amdgpu_gfx1250_op_V_CMPX_NE_U32,"V_CMPX_NE_U32"}, // 205
                {amdgpu_gfx1250_op_V_CMPX_GE_U32,"V_CMPX_GE_U32"}, // 206
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 207
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 208
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 209
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 210
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 211
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 212
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 213
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 214
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 215
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 216
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 217
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 218
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 219
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 220
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 221
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 222
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 223
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 224
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 225
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 226
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 227
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 228
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 229
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 230
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 231
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 232
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 233
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 234
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 235
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 236
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 237
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 238
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 239
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 240
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 241
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 242
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 243
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 244
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 245
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 246
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 247
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 248
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 249
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 250
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 251
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 252
                {amdgpu_gfx1250_op_V_CMPX_CLASS_F16,"V_CMPX_CLASS_F16"}, // 253
                {amdgpu_gfx1250_op_V_CMPX_CLASS_F32,"V_CMPX_CLASS_F32"}, // 254
            }; // end VOPC_VOP_DPP8_insn_table
            const amdgpu_gfx1250_insn_entry VOPC_VOP_DPP16_insn_table[255] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_V_CMP_LT_F16,"V_CMP_LT_F16"}, // 1
                {amdgpu_gfx1250_op_V_CMP_EQ_F16,"V_CMP_EQ_F16"}, // 2
                {amdgpu_gfx1250_op_V_CMP_LE_F16,"V_CMP_LE_F16"}, // 3
                {amdgpu_gfx1250_op_V_CMP_GT_F16,"V_CMP_GT_F16"}, // 4
                {amdgpu_gfx1250_op_V_CMP_LG_F16,"V_CMP_LG_F16"}, // 5
                {amdgpu_gfx1250_op_V_CMP_GE_F16,"V_CMP_GE_F16"}, // 6
                {amdgpu_gfx1250_op_V_CMP_O_F16,"V_CMP_O_F16"}, // 7
                {amdgpu_gfx1250_op_V_CMP_U_F16,"V_CMP_U_F16"}, // 8
                {amdgpu_gfx1250_op_V_CMP_NGE_F16,"V_CMP_NGE_F16"}, // 9
                {amdgpu_gfx1250_op_V_CMP_NLG_F16,"V_CMP_NLG_F16"}, // 10
                {amdgpu_gfx1250_op_V_CMP_NGT_F16,"V_CMP_NGT_F16"}, // 11
                {amdgpu_gfx1250_op_V_CMP_NLE_F16,"V_CMP_NLE_F16"}, // 12
                {amdgpu_gfx1250_op_V_CMP_NEQ_F16,"V_CMP_NEQ_F16"}, // 13
                {amdgpu_gfx1250_op_V_CMP_NLT_F16,"V_CMP_NLT_F16"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 16
                {amdgpu_gfx1250_op_V_CMP_LT_F32,"V_CMP_LT_F32"}, // 17
                {amdgpu_gfx1250_op_V_CMP_EQ_F32,"V_CMP_EQ_F32"}, // 18
                {amdgpu_gfx1250_op_V_CMP_LE_F32,"V_CMP_LE_F32"}, // 19
                {amdgpu_gfx1250_op_V_CMP_GT_F32,"V_CMP_GT_F32"}, // 20
                {amdgpu_gfx1250_op_V_CMP_LG_F32,"V_CMP_LG_F32"}, // 21
                {amdgpu_gfx1250_op_V_CMP_GE_F32,"V_CMP_GE_F32"}, // 22
                {amdgpu_gfx1250_op_V_CMP_O_F32,"V_CMP_O_F32"}, // 23
                {amdgpu_gfx1250_op_V_CMP_U_F32,"V_CMP_U_F32"}, // 24
                {amdgpu_gfx1250_op_V_CMP_NGE_F32,"V_CMP_NGE_F32"}, // 25
                {amdgpu_gfx1250_op_V_CMP_NLG_F32,"V_CMP_NLG_F32"}, // 26
                {amdgpu_gfx1250_op_V_CMP_NGT_F32,"V_CMP_NGT_F32"}, // 27
                {amdgpu_gfx1250_op_V_CMP_NLE_F32,"V_CMP_NLE_F32"}, // 28
                {amdgpu_gfx1250_op_V_CMP_NEQ_F32,"V_CMP_NEQ_F32"}, // 29
                {amdgpu_gfx1250_op_V_CMP_NLT_F32,"V_CMP_NLT_F32"}, // 30
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 31
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 32
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 33
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 34
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 35
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 36
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 42
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_V_CMP_LT_I16,"V_CMP_LT_I16"}, // 49
                {amdgpu_gfx1250_op_V_CMP_EQ_I16,"V_CMP_EQ_I16"}, // 50
                {amdgpu_gfx1250_op_V_CMP_LE_I16,"V_CMP_LE_I16"}, // 51
                {amdgpu_gfx1250_op_V_CMP_GT_I16,"V_CMP_GT_I16"}, // 52
                {amdgpu_gfx1250_op_V_CMP_NE_I16,"V_CMP_NE_I16"}, // 53
                {amdgpu_gfx1250_op_V_CMP_GE_I16,"V_CMP_GE_I16"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_V_CMP_LT_U16,"V_CMP_LT_U16"}, // 57
                {amdgpu_gfx1250_op_V_CMP_EQ_U16,"V_CMP_EQ_U16"}, // 58
                {amdgpu_gfx1250_op_V_CMP_LE_U16,"V_CMP_LE_U16"}, // 59
                {amdgpu_gfx1250_op_V_CMP_GT_U16,"V_CMP_GT_U16"}, // 60
                {amdgpu_gfx1250_op_V_CMP_NE_U16,"V_CMP_NE_U16"}, // 61
                {amdgpu_gfx1250_op_V_CMP_GE_U16,"V_CMP_GE_U16"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 64
                {amdgpu_gfx1250_op_V_CMP_LT_I32,"V_CMP_LT_I32"}, // 65
                {amdgpu_gfx1250_op_V_CMP_EQ_I32,"V_CMP_EQ_I32"}, // 66
                {amdgpu_gfx1250_op_V_CMP_LE_I32,"V_CMP_LE_I32"}, // 67
                {amdgpu_gfx1250_op_V_CMP_GT_I32,"V_CMP_GT_I32"}, // 68
                {amdgpu_gfx1250_op_V_CMP_NE_I32,"V_CMP_NE_I32"}, // 69
                {amdgpu_gfx1250_op_V_CMP_GE_I32,"V_CMP_GE_I32"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 72
                {amdgpu_gfx1250_op_V_CMP_LT_U32,"V_CMP_LT_U32"}, // 73
                {amdgpu_gfx1250_op_V_CMP_EQ_U32,"V_CMP_EQ_U32"}, // 74
                {amdgpu_gfx1250_op_V_CMP_LE_U32,"V_CMP_LE_U32"}, // 75
                {amdgpu_gfx1250_op_V_CMP_GT_U32,"V_CMP_GT_U32"}, // 76
                {amdgpu_gfx1250_op_V_CMP_NE_U32,"V_CMP_NE_U32"}, // 77
                {amdgpu_gfx1250_op_V_CMP_GE_U32,"V_CMP_GE_U32"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 80
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 81
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 82
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 83
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 84
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 85
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 86
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 87
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 88
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 89
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 90
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 91
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 92
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 93
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 94
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 95
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 96
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 97
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 98
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 99
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 100
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 101
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 102
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 103
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 104
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 105
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 106
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 107
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 108
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 109
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 110
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 111
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 112
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 113
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 114
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 115
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 116
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 117
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 118
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 119
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 120
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 121
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 122
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 123
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 124
                {amdgpu_gfx1250_op_V_CMP_CLASS_F16,"V_CMP_CLASS_F16"}, // 125
                {amdgpu_gfx1250_op_V_CMP_CLASS_F32,"V_CMP_CLASS_F32"}, // 126
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 127
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 128
                {amdgpu_gfx1250_op_V_CMPX_LT_F16,"V_CMPX_LT_F16"}, // 129
                {amdgpu_gfx1250_op_V_CMPX_EQ_F16,"V_CMPX_EQ_F16"}, // 130
                {amdgpu_gfx1250_op_V_CMPX_LE_F16,"V_CMPX_LE_F16"}, // 131
                {amdgpu_gfx1250_op_V_CMPX_GT_F16,"V_CMPX_GT_F16"}, // 132
                {amdgpu_gfx1250_op_V_CMPX_LG_F16,"V_CMPX_LG_F16"}, // 133
                {amdgpu_gfx1250_op_V_CMPX_GE_F16,"V_CMPX_GE_F16"}, // 134
                {amdgpu_gfx1250_op_V_CMPX_O_F16,"V_CMPX_O_F16"}, // 135
                {amdgpu_gfx1250_op_V_CMPX_U_F16,"V_CMPX_U_F16"}, // 136
                {amdgpu_gfx1250_op_V_CMPX_NGE_F16,"V_CMPX_NGE_F16"}, // 137
                {amdgpu_gfx1250_op_V_CMPX_NLG_F16,"V_CMPX_NLG_F16"}, // 138
                {amdgpu_gfx1250_op_V_CMPX_NGT_F16,"V_CMPX_NGT_F16"}, // 139
                {amdgpu_gfx1250_op_V_CMPX_NLE_F16,"V_CMPX_NLE_F16"}, // 140
                {amdgpu_gfx1250_op_V_CMPX_NEQ_F16,"V_CMPX_NEQ_F16"}, // 141
                {amdgpu_gfx1250_op_V_CMPX_NLT_F16,"V_CMPX_NLT_F16"}, // 142
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 143
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 144
                {amdgpu_gfx1250_op_V_CMPX_LT_F32,"V_CMPX_LT_F32"}, // 145
                {amdgpu_gfx1250_op_V_CMPX_EQ_F32,"V_CMPX_EQ_F32"}, // 146
                {amdgpu_gfx1250_op_V_CMPX_LE_F32,"V_CMPX_LE_F32"}, // 147
                {amdgpu_gfx1250_op_V_CMPX_GT_F32,"V_CMPX_GT_F32"}, // 148
                {amdgpu_gfx1250_op_V_CMPX_LG_F32,"V_CMPX_LG_F32"}, // 149
                {amdgpu_gfx1250_op_V_CMPX_GE_F32,"V_CMPX_GE_F32"}, // 150
                {amdgpu_gfx1250_op_V_CMPX_O_F32,"V_CMPX_O_F32"}, // 151
                {amdgpu_gfx1250_op_V_CMPX_U_F32,"V_CMPX_U_F32"}, // 152
                {amdgpu_gfx1250_op_V_CMPX_NGE_F32,"V_CMPX_NGE_F32"}, // 153
                {amdgpu_gfx1250_op_V_CMPX_NLG_F32,"V_CMPX_NLG_F32"}, // 154
                {amdgpu_gfx1250_op_V_CMPX_NGT_F32,"V_CMPX_NGT_F32"}, // 155
                {amdgpu_gfx1250_op_V_CMPX_NLE_F32,"V_CMPX_NLE_F32"}, // 156
                {amdgpu_gfx1250_op_V_CMPX_NEQ_F32,"V_CMPX_NEQ_F32"}, // 157
                {amdgpu_gfx1250_op_V_CMPX_NLT_F32,"V_CMPX_NLT_F32"}, // 158
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 159
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 160
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 161
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 162
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 163
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 164
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 165
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 166
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 167
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 168
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 169
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 170
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 171
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 172
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 173
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 174
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 175
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 176
                {amdgpu_gfx1250_op_V_CMPX_LT_I16,"V_CMPX_LT_I16"}, // 177
                {amdgpu_gfx1250_op_V_CMPX_EQ_I16,"V_CMPX_EQ_I16"}, // 178
                {amdgpu_gfx1250_op_V_CMPX_LE_I16,"V_CMPX_LE_I16"}, // 179
                {amdgpu_gfx1250_op_V_CMPX_GT_I16,"V_CMPX_GT_I16"}, // 180
                {amdgpu_gfx1250_op_V_CMPX_NE_I16,"V_CMPX_NE_I16"}, // 181
                {amdgpu_gfx1250_op_V_CMPX_GE_I16,"V_CMPX_GE_I16"}, // 182
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 183
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 184
                {amdgpu_gfx1250_op_V_CMPX_LT_U16,"V_CMPX_LT_U16"}, // 185
                {amdgpu_gfx1250_op_V_CMPX_EQ_U16,"V_CMPX_EQ_U16"}, // 186
                {amdgpu_gfx1250_op_V_CMPX_LE_U16,"V_CMPX_LE_U16"}, // 187
                {amdgpu_gfx1250_op_V_CMPX_GT_U16,"V_CMPX_GT_U16"}, // 188
                {amdgpu_gfx1250_op_V_CMPX_NE_U16,"V_CMPX_NE_U16"}, // 189
                {amdgpu_gfx1250_op_V_CMPX_GE_U16,"V_CMPX_GE_U16"}, // 190
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 191
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 192
                {amdgpu_gfx1250_op_V_CMPX_LT_I32,"V_CMPX_LT_I32"}, // 193
                {amdgpu_gfx1250_op_V_CMPX_EQ_I32,"V_CMPX_EQ_I32"}, // 194
                {amdgpu_gfx1250_op_V_CMPX_LE_I32,"V_CMPX_LE_I32"}, // 195
                {amdgpu_gfx1250_op_V_CMPX_GT_I32,"V_CMPX_GT_I32"}, // 196
                {amdgpu_gfx1250_op_V_CMPX_NE_I32,"V_CMPX_NE_I32"}, // 197
                {amdgpu_gfx1250_op_V_CMPX_GE_I32,"V_CMPX_GE_I32"}, // 198
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 199
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 200
                {amdgpu_gfx1250_op_V_CMPX_LT_U32,"V_CMPX_LT_U32"}, // 201
                {amdgpu_gfx1250_op_V_CMPX_EQ_U32,"V_CMPX_EQ_U32"}, // 202
                {amdgpu_gfx1250_op_V_CMPX_LE_U32,"V_CMPX_LE_U32"}, // 203
                {amdgpu_gfx1250_op_V_CMPX_GT_U32,"V_CMPX_GT_U32"}, // 204
                {amdgpu_gfx1250_op_V_CMPX_NE_U32,"V_CMPX_NE_U32"}, // 205
                {amdgpu_gfx1250_op_V_CMPX_GE_U32,"V_CMPX_GE_U32"}, // 206
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 207
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 208
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 209
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 210
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 211
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 212
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 213
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 214
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 215
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 216
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 217
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 218
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 219
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 220
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 221
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 222
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 223
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 224
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 225
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 226
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 227
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 228
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 229
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 230
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 231
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 232
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 233
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 234
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 235
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 236
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 237
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 238
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 239
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 240
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 241
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 242
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 243
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 244
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 245
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 246
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 247
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 248
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 249
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 250
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 251
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 252
                {amdgpu_gfx1250_op_V_CMPX_CLASS_F16,"V_CMPX_CLASS_F16"}, // 253
                {amdgpu_gfx1250_op_V_CMPX_CLASS_F32,"V_CMPX_CLASS_F32"}, // 254
            }; // end VOPC_VOP_DPP16_insn_table
            const amdgpu_gfx1250_insn_entry VOPD3XY_X_insn_table[37] =
            {
                {amdgpu_gfx1250_op_V_DUAL_FMAC_F32,"V_DUAL_FMAC_F32"}, // 0
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 1
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 2
                {amdgpu_gfx1250_op_V_DUAL_MUL_F32,"V_DUAL_MUL_F32"}, // 3
                {amdgpu_gfx1250_op_V_DUAL_ADD_F32,"V_DUAL_ADD_F32"}, // 4
                {amdgpu_gfx1250_op_V_DUAL_SUB_F32,"V_DUAL_SUB_F32"}, // 5
                {amdgpu_gfx1250_op_V_DUAL_SUBREV_F32,"V_DUAL_SUBREV_F32"}, // 6
                {amdgpu_gfx1250_op_V_DUAL_MUL_DX9_ZERO_F32,"V_DUAL_MUL_DX9_ZERO_F32"}, // 7
                {amdgpu_gfx1250_op_V_DUAL_MOV_B32,"V_DUAL_MOV_B32"}, // 8
                {amdgpu_gfx1250_op_V_DUAL_CNDMASK_B32,"V_DUAL_CNDMASK_B32"}, // 9
                {amdgpu_gfx1250_op_V_DUAL_MAX_NUM_F32,"V_DUAL_MAX_NUM_F32"}, // 10
                {amdgpu_gfx1250_op_V_DUAL_MIN_NUM_F32,"V_DUAL_MIN_NUM_F32"}, // 11
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 12
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 13
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_V_DUAL_ADD_NC_U32,"V_DUAL_ADD_NC_U32"}, // 16
                {amdgpu_gfx1250_op_V_DUAL_LSHLREV_B32,"V_DUAL_LSHLREV_B32"}, // 17
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 18
                {amdgpu_gfx1250_op_V_DUAL_FMA_F32,"V_DUAL_FMA_F32"}, // 19
                {amdgpu_gfx1250_op_V_DUAL_SUB_NC_U32,"V_DUAL_SUB_NC_U32"}, // 20
                {amdgpu_gfx1250_op_V_DUAL_LSHRREV_B32,"V_DUAL_LSHRREV_B32"}, // 21
                {amdgpu_gfx1250_op_V_DUAL_ASHRREV_I32,"V_DUAL_ASHRREV_I32"}, // 22
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 23
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 24
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 25
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 26
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 27
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 28
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 29
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 30
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 31
                {amdgpu_gfx1250_op_V_DUAL_FMA_F64,"V_DUAL_FMA_F64"}, // 32
                {amdgpu_gfx1250_op_V_DUAL_ADD_F64,"V_DUAL_ADD_F64"}, // 33
                {amdgpu_gfx1250_op_V_DUAL_MUL_F64,"V_DUAL_MUL_F64"}, // 34
                {amdgpu_gfx1250_op_V_DUAL_MAX_NUM_F64,"V_DUAL_MAX_NUM_F64"}, // 35
                {amdgpu_gfx1250_op_V_DUAL_MIN_NUM_F64,"V_DUAL_MIN_NUM_F64"}, // 36
            }; // end VOPD3XY_X_insn_table
            const amdgpu_gfx1250_insn_entry VOPD3XY_Y_insn_table[25] =
            {
                {amdgpu_gfx1250_op_V_DUAL_FMAC_F32,"V_DUAL_FMAC_F32"}, // 0
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 1
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 2
                {amdgpu_gfx1250_op_V_DUAL_MUL_F32,"V_DUAL_MUL_F32"}, // 3
                {amdgpu_gfx1250_op_V_DUAL_ADD_F32,"V_DUAL_ADD_F32"}, // 4
                {amdgpu_gfx1250_op_V_DUAL_SUB_F32,"V_DUAL_SUB_F32"}, // 5
                {amdgpu_gfx1250_op_V_DUAL_SUBREV_F32,"V_DUAL_SUBREV_F32"}, // 6
                {amdgpu_gfx1250_op_V_DUAL_MUL_DX9_ZERO_F32,"V_DUAL_MUL_DX9_ZERO_F32"}, // 7
                {amdgpu_gfx1250_op_V_DUAL_MOV_B32,"V_DUAL_MOV_B32"}, // 8
                {amdgpu_gfx1250_op_V_DUAL_CNDMASK_B32,"V_DUAL_CNDMASK_B32"}, // 9
                {amdgpu_gfx1250_op_V_DUAL_MAX_NUM_F32,"V_DUAL_MAX_NUM_F32"}, // 10
                {amdgpu_gfx1250_op_V_DUAL_MIN_NUM_F32,"V_DUAL_MIN_NUM_F32"}, // 11
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 12
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 13
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_V_DUAL_ADD_NC_U32,"V_DUAL_ADD_NC_U32"}, // 16
                {amdgpu_gfx1250_op_V_DUAL_LSHLREV_B32,"V_DUAL_LSHLREV_B32"}, // 17
                {amdgpu_gfx1250_op_V_DUAL_BITOP2_B32,"V_DUAL_BITOP2_B32"}, // 18
                {amdgpu_gfx1250_op_V_DUAL_FMA_F32,"V_DUAL_FMA_F32"}, // 19
                {amdgpu_gfx1250_op_V_DUAL_SUB_NC_U32,"V_DUAL_SUB_NC_U32"}, // 20
                {amdgpu_gfx1250_op_V_DUAL_LSHRREV_B32,"V_DUAL_LSHRREV_B32"}, // 21
                {amdgpu_gfx1250_op_V_DUAL_ASHRREV_I32,"V_DUAL_ASHRREV_I32"}, // 22
                {amdgpu_gfx1250_op_V_DUAL_MAX_I32,"V_DUAL_MAX_I32"}, // 23
                {amdgpu_gfx1250_op_V_DUAL_MIN_I32,"V_DUAL_MIN_I32"}, // 24
            }; // end VOPD3XY_Y_insn_table
            const amdgpu_gfx1250_insn_entry VOPDXY_X_insn_table[12] =
            {
                {amdgpu_gfx1250_op_V_DUAL_FMAC_F32,"V_DUAL_FMAC_F32"}, // 0
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 1
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 2
                {amdgpu_gfx1250_op_V_DUAL_MUL_F32,"V_DUAL_MUL_F32"}, // 3
                {amdgpu_gfx1250_op_V_DUAL_ADD_F32,"V_DUAL_ADD_F32"}, // 4
                {amdgpu_gfx1250_op_V_DUAL_SUB_F32,"V_DUAL_SUB_F32"}, // 5
                {amdgpu_gfx1250_op_V_DUAL_SUBREV_F32,"V_DUAL_SUBREV_F32"}, // 6
                {amdgpu_gfx1250_op_V_DUAL_MUL_DX9_ZERO_F32,"V_DUAL_MUL_DX9_ZERO_F32"}, // 7
                {amdgpu_gfx1250_op_V_DUAL_MOV_B32,"V_DUAL_MOV_B32"}, // 8
                {amdgpu_gfx1250_op_V_DUAL_CNDMASK_B32,"V_DUAL_CNDMASK_B32"}, // 9
                {amdgpu_gfx1250_op_V_DUAL_MAX_NUM_F32,"V_DUAL_MAX_NUM_F32"}, // 10
                {amdgpu_gfx1250_op_V_DUAL_MIN_NUM_F32,"V_DUAL_MIN_NUM_F32"}, // 11
            }; // end VOPDXY_X_insn_table
            const amdgpu_gfx1250_insn_entry VOPDXY_INST_LITERAL_X_insn_table[12] =
            {
                {amdgpu_gfx1250_op_V_DUAL_FMAC_F32,"V_DUAL_FMAC_F32"}, // 0
                {amdgpu_gfx1250_op_V_DUAL_FMAAK_F32,"V_DUAL_FMAAK_F32"}, // 1
                {amdgpu_gfx1250_op_V_DUAL_FMAMK_F32,"V_DUAL_FMAMK_F32"}, // 2
                {amdgpu_gfx1250_op_V_DUAL_MUL_F32,"V_DUAL_MUL_F32"}, // 3
                {amdgpu_gfx1250_op_V_DUAL_ADD_F32,"V_DUAL_ADD_F32"}, // 4
                {amdgpu_gfx1250_op_V_DUAL_SUB_F32,"V_DUAL_SUB_F32"}, // 5
                {amdgpu_gfx1250_op_V_DUAL_SUBREV_F32,"V_DUAL_SUBREV_F32"}, // 6
                {amdgpu_gfx1250_op_V_DUAL_MUL_DX9_ZERO_F32,"V_DUAL_MUL_DX9_ZERO_F32"}, // 7
                {amdgpu_gfx1250_op_V_DUAL_MOV_B32,"V_DUAL_MOV_B32"}, // 8
                {amdgpu_gfx1250_op_V_DUAL_CNDMASK_B32,"V_DUAL_CNDMASK_B32"}, // 9
                {amdgpu_gfx1250_op_V_DUAL_MAX_NUM_F32,"V_DUAL_MAX_NUM_F32"}, // 10
                {amdgpu_gfx1250_op_V_DUAL_MIN_NUM_F32,"V_DUAL_MIN_NUM_F32"}, // 11
            }; // end VOPDXY_INST_LITERAL_X_insn_table
            const amdgpu_gfx1250_insn_entry VOPDXY_Y_insn_table[25] =
            {
                {amdgpu_gfx1250_op_V_DUAL_FMAC_F32,"V_DUAL_FMAC_F32"}, // 0
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 1
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 2
                {amdgpu_gfx1250_op_V_DUAL_MUL_F32,"V_DUAL_MUL_F32"}, // 3
                {amdgpu_gfx1250_op_V_DUAL_ADD_F32,"V_DUAL_ADD_F32"}, // 4
                {amdgpu_gfx1250_op_V_DUAL_SUB_F32,"V_DUAL_SUB_F32"}, // 5
                {amdgpu_gfx1250_op_V_DUAL_SUBREV_F32,"V_DUAL_SUBREV_F32"}, // 6
                {amdgpu_gfx1250_op_V_DUAL_MUL_DX9_ZERO_F32,"V_DUAL_MUL_DX9_ZERO_F32"}, // 7
                {amdgpu_gfx1250_op_V_DUAL_MOV_B32,"V_DUAL_MOV_B32"}, // 8
                {amdgpu_gfx1250_op_V_DUAL_CNDMASK_B32,"V_DUAL_CNDMASK_B32"}, // 9
                {amdgpu_gfx1250_op_V_DUAL_MAX_NUM_F32,"V_DUAL_MAX_NUM_F32"}, // 10
                {amdgpu_gfx1250_op_V_DUAL_MIN_NUM_F32,"V_DUAL_MIN_NUM_F32"}, // 11
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 12
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 13
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_V_DUAL_ADD_NC_U32,"V_DUAL_ADD_NC_U32"}, // 16
                {amdgpu_gfx1250_op_V_DUAL_LSHLREV_B32,"V_DUAL_LSHLREV_B32"}, // 17
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 18
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 19
                {amdgpu_gfx1250_op_V_DUAL_SUB_NC_U32,"V_DUAL_SUB_NC_U32"}, // 20
                {amdgpu_gfx1250_op_V_DUAL_LSHRREV_B32,"V_DUAL_LSHRREV_B32"}, // 21
                {amdgpu_gfx1250_op_V_DUAL_ASHRREV_I32,"V_DUAL_ASHRREV_I32"}, // 22
                {amdgpu_gfx1250_op_V_DUAL_MAX_I32,"V_DUAL_MAX_I32"}, // 23
                {amdgpu_gfx1250_op_V_DUAL_MIN_I32,"V_DUAL_MIN_I32"}, // 24
            }; // end VOPDXY_Y_insn_table
            const amdgpu_gfx1250_insn_entry VOPDXY_INST_LITERAL_Y_insn_table[25] =
            {
                {amdgpu_gfx1250_op_V_DUAL_FMAC_F32,"V_DUAL_FMAC_F32"}, // 0
                {amdgpu_gfx1250_op_V_DUAL_FMAAK_F32,"V_DUAL_FMAAK_F32"}, // 1
                {amdgpu_gfx1250_op_V_DUAL_FMAMK_F32,"V_DUAL_FMAMK_F32"}, // 2
                {amdgpu_gfx1250_op_V_DUAL_MUL_F32,"V_DUAL_MUL_F32"}, // 3
                {amdgpu_gfx1250_op_V_DUAL_ADD_F32,"V_DUAL_ADD_F32"}, // 4
                {amdgpu_gfx1250_op_V_DUAL_SUB_F32,"V_DUAL_SUB_F32"}, // 5
                {amdgpu_gfx1250_op_V_DUAL_SUBREV_F32,"V_DUAL_SUBREV_F32"}, // 6
                {amdgpu_gfx1250_op_V_DUAL_MUL_DX9_ZERO_F32,"V_DUAL_MUL_DX9_ZERO_F32"}, // 7
                {amdgpu_gfx1250_op_V_DUAL_MOV_B32,"V_DUAL_MOV_B32"}, // 8
                {amdgpu_gfx1250_op_V_DUAL_CNDMASK_B32,"V_DUAL_CNDMASK_B32"}, // 9
                {amdgpu_gfx1250_op_V_DUAL_MAX_NUM_F32,"V_DUAL_MAX_NUM_F32"}, // 10
                {amdgpu_gfx1250_op_V_DUAL_MIN_NUM_F32,"V_DUAL_MIN_NUM_F32"}, // 11
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 12
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 13
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_V_DUAL_ADD_NC_U32,"V_DUAL_ADD_NC_U32"}, // 16
                {amdgpu_gfx1250_op_V_DUAL_LSHLREV_B32,"V_DUAL_LSHLREV_B32"}, // 17
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 18
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 19
                {amdgpu_gfx1250_op_V_DUAL_SUB_NC_U32,"V_DUAL_SUB_NC_U32"}, // 20
                {amdgpu_gfx1250_op_V_DUAL_LSHRREV_B32,"V_DUAL_LSHRREV_B32"}, // 21
                {amdgpu_gfx1250_op_V_DUAL_ASHRREV_I32,"V_DUAL_ASHRREV_I32"}, // 22
                {amdgpu_gfx1250_op_V_DUAL_MAX_I32,"V_DUAL_MAX_I32"}, // 23
                {amdgpu_gfx1250_op_V_DUAL_MIN_I32,"V_DUAL_MIN_I32"}, // 24
            }; // end VOPDXY_INST_LITERAL_Y_insn_table
            const amdgpu_gfx1250_insn_entry ENC_VSCRATCH_insn_table[85] =
            {
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 0
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 1
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 2
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 3
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 4
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 5
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 6
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 7
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 8
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 9
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 10
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 11
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 12
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 13
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 14
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 15
                {amdgpu_gfx1250_op_SCRATCH_LOAD_U8,"SCRATCH_LOAD_U8"}, // 16
                {amdgpu_gfx1250_op_SCRATCH_LOAD_I8,"SCRATCH_LOAD_I8"}, // 17
                {amdgpu_gfx1250_op_SCRATCH_LOAD_U16,"SCRATCH_LOAD_U16"}, // 18
                {amdgpu_gfx1250_op_SCRATCH_LOAD_I16,"SCRATCH_LOAD_I16"}, // 19
                {amdgpu_gfx1250_op_SCRATCH_LOAD_B32,"SCRATCH_LOAD_B32"}, // 20
                {amdgpu_gfx1250_op_SCRATCH_LOAD_B64,"SCRATCH_LOAD_B64"}, // 21
                {amdgpu_gfx1250_op_SCRATCH_LOAD_B96,"SCRATCH_LOAD_B96"}, // 22
                {amdgpu_gfx1250_op_SCRATCH_LOAD_B128,"SCRATCH_LOAD_B128"}, // 23
                {amdgpu_gfx1250_op_SCRATCH_STORE_B8,"SCRATCH_STORE_B8"}, // 24
                {amdgpu_gfx1250_op_SCRATCH_STORE_B16,"SCRATCH_STORE_B16"}, // 25
                {amdgpu_gfx1250_op_SCRATCH_STORE_B32,"SCRATCH_STORE_B32"}, // 26
                {amdgpu_gfx1250_op_SCRATCH_STORE_B64,"SCRATCH_STORE_B64"}, // 27
                {amdgpu_gfx1250_op_SCRATCH_STORE_B96,"SCRATCH_STORE_B96"}, // 28
                {amdgpu_gfx1250_op_SCRATCH_STORE_B128,"SCRATCH_STORE_B128"}, // 29
                {amdgpu_gfx1250_op_SCRATCH_LOAD_D16_U8,"SCRATCH_LOAD_D16_U8"}, // 30
                {amdgpu_gfx1250_op_SCRATCH_LOAD_D16_I8,"SCRATCH_LOAD_D16_I8"}, // 31
                {amdgpu_gfx1250_op_SCRATCH_LOAD_D16_B16,"SCRATCH_LOAD_D16_B16"}, // 32
                {amdgpu_gfx1250_op_SCRATCH_LOAD_D16_HI_U8,"SCRATCH_LOAD_D16_HI_U8"}, // 33
                {amdgpu_gfx1250_op_SCRATCH_LOAD_D16_HI_I8,"SCRATCH_LOAD_D16_HI_I8"}, // 34
                {amdgpu_gfx1250_op_SCRATCH_LOAD_D16_HI_B16,"SCRATCH_LOAD_D16_HI_B16"}, // 35
                {amdgpu_gfx1250_op_SCRATCH_STORE_D16_HI_B8,"SCRATCH_STORE_D16_HI_B8"}, // 36
                {amdgpu_gfx1250_op_SCRATCH_STORE_D16_HI_B16,"SCRATCH_STORE_D16_HI_B16"}, // 37
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 38
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 39
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 40
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 41
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 42
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 43
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 44
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 45
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 46
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 47
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 48
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 49
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 50
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 51
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 52
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 53
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 54
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 55
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 56
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 57
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 58
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 59
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 60
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 61
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 62
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 63
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 64
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 65
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 66
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 67
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 68
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 69
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 70
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 71
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 72
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 73
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 74
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 75
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 76
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 77
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 78
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 79
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 80
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 81
                {amdgpu_gfx1250_op_INVALID,"INVALID"}, // 82
                {amdgpu_gfx1250_op_SCRATCH_LOAD_BLOCK,"SCRATCH_LOAD_BLOCK"}, // 83
                {amdgpu_gfx1250_op_SCRATCH_STORE_BLOCK,"SCRATCH_STORE_BLOCK"}, // 84
            }; // end ENC_VSCRATCH_insn_table


        };

    }
}
#endif //INSTRUCTION_DECODER_GFX1250_H
