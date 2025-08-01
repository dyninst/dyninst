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
#ifndef INSTRUCTION_DECODER_GFX940_H
#define INSTRUCTION_DECODER_GFX940_H
#include <array>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include "InstructionDecoderImpl.h"
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

        struct amdgpu_gfx940_insn_entry;

        class InstructionDecoder_amdgpu_gfx940 : public InstructionDecoderImpl {
            friend struct amdgpu_gfx940_insn_entry;
            friend struct amdgpu_mask_entry;

            public:
    		InstructionDecoder_amdgpu_gfx940(Architecture a) : InstructionDecoderImpl(a)  {}

            virtual ~InstructionDecoder_amdgpu_gfx940() = default;

            // decode one instruction starting from b.start
            // will advance b.start whenver a instruction is successfully decoded
            virtual Instruction decode(InstructionDecoder::buffer &b);

            virtual void setMode(bool)  { }

            virtual bool decodeOperands(const Instruction *insn_to_complete);

            bool decodeOperands(const amdgpu_gfx940_insn_entry & insn_entry);

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
                int field(unsigned int raw)  {
#if defined DEBUG_FIELD
                    std::cerr << start << "-" << end << ":" << std::dec << (raw >> (start) &
                            (0xFFFFFFFF >> (31 - (end - start)))) << " ";
#endif
                    return (raw >> (start) & (0xFFFFFFFF >> (31 - (end - start))));
                }

            template<int start, int end>
                int longfield(unsigned long long int raw)  {
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
            Expression::Ptr decodeOPR_SDWA();
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
            static bool IS_ENC_VOP3P(uint64_t I);
            static bool IS_ENC_VOP3(uint64_t I);
            static bool IS_ENC_DS(uint64_t I);
            static bool IS_ENC_MUBUF(uint64_t I);
            static bool IS_ENC_MTBUF(uint64_t I);
            static bool IS_ENC_FLAT(uint64_t I);
            static bool IS_ENC_FLAT_GLBL(uint64_t I);
            static bool IS_ENC_FLAT_SCRATCH(uint64_t I);
            static bool IS_SOPK_INST_LITERAL_(uint64_t I);
            static bool IS_ENC_VOP2_LITERAL(uint64_t I);
            static bool IS_ENC_VOP3B(uint64_t I);
            static bool IS_ENC_VOP3P_MFMA(uint64_t I);
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
                ENC_VOP3P = 9,
                ENC_VOP3 = 10,
                ENC_DS = 11,
                ENC_MUBUF = 12,
                ENC_MTBUF = 13,
                ENC_FLAT = 16,
                ENC_FLAT_GLBL = 17,
                ENC_FLAT_SCRATCH = 18,
                SOPK_INST_LITERAL_ = 19,
                ENC_VOP2_LITERAL = 20,
                ENC_VOP3B = 21,
                ENC_VOP3P_MFMA = 22,
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
                uint8_t GLC : 1;
                uint8_t IMM : 1;
                uint8_t NV : 1;
                uint32_t OFFSET : 21;
                uint8_t OP : 8;
                uint8_t SBASE : 7;
                uint8_t SDATA : 7;
                uint8_t SOFFSET : 7;
                uint8_t SOFFSET_EN : 1;
            };
            struct layout_ENC_VOP1 {
                uint8_t ENCODING : 7;
                uint8_t OP : 8;
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
            struct layout_ENC_VOP3P {
                uint8_t CLAMP : 1;
                uint16_t ENCODING : 9;
                uint8_t NEG : 3;
                uint8_t NEG_HI : 3;
                uint8_t OP : 7;
                uint8_t OP_SEL : 3;
                uint8_t OP_SEL_HI : 2;
                uint8_t OP_SEL_HI_2 : 1;
                uint16_t SRC0 : 9;
                uint16_t SRC1 : 9;
                uint16_t SRC2 : 9;
                uint8_t VDST : 8;
            };
            struct layout_ENC_VOP3 {
                uint8_t ABS : 3;
                uint8_t CLAMP : 1;
                uint8_t ENCODING : 6;
                uint8_t NEG : 3;
                uint8_t OMOD : 2;
                uint16_t OP : 10;
                uint8_t OP_SEL : 4;
                uint16_t SRC0 : 9;
                uint16_t SRC1 : 9;
                uint16_t SRC2 : 9;
                uint8_t VDST : 8;
            };
            struct layout_ENC_DS {
                uint8_t ACC : 1;
                uint8_t ADDR : 8;
                uint8_t DATA0 : 8;
                uint8_t DATA1 : 8;
                uint8_t ENCODING : 6;
                uint8_t GDS : 1;
                uint8_t OFFSET0 : 8;
                uint8_t OFFSET1 : 8;
                uint8_t OP : 8;
                uint8_t VDST : 8;
            };
            struct layout_ENC_MUBUF {
                uint8_t ACC : 1;
                uint8_t ENCODING : 6;
                uint8_t IDXEN : 1;
                uint8_t LDS : 1;
                uint8_t NT : 1;
                uint8_t OFFEN : 1;
                uint16_t OFFSET : 12;
                uint8_t OP : 7;
                uint8_t SC0 : 1;
                uint8_t SC1 : 1;
                uint8_t SOFFSET : 8;
                uint8_t SRSRC : 7;
                uint8_t VADDR : 8;
                uint8_t VDATA : 8;
            };
            struct layout_ENC_MTBUF {
                uint8_t ACC : 1;
                uint8_t DFMT : 4;
                uint8_t ENCODING : 6;
                uint8_t IDXEN : 1;
                uint8_t NFMT : 3;
                uint8_t NT : 1;
                uint8_t OFFEN : 1;
                uint16_t OFFSET : 12;
                uint8_t OP : 4;
                uint8_t SC0 : 1;
                uint8_t SC1 : 1;
                uint8_t SOFFSET : 8;
                uint8_t SRSRC : 7;
                uint8_t VADDR : 8;
                uint8_t VDATA : 8;
            };
            struct layout_ENC_FLAT {
                uint8_t ACC : 1;
                uint8_t ADDR : 8;
                uint8_t DATA : 8;
                uint8_t ENCODING : 6;
                uint8_t NT : 1;
                uint16_t OFFSET : 12;
                uint8_t OP : 7;
                uint8_t SADDR : 7;
                uint8_t SC0 : 1;
                uint8_t SC1 : 1;
                uint8_t SEG : 2;
                uint8_t SVE : 1;
                uint8_t VDST : 8;
            };
            struct layout_ENC_FLAT_GLBL {
                uint8_t ACC : 1;
                uint8_t ADDR : 8;
                uint8_t DATA : 8;
                uint8_t ENCODING : 6;
                uint8_t NT : 1;
                uint16_t OFFSET : 13;
                uint8_t OP : 7;
                uint8_t SADDR : 7;
                uint8_t SC0 : 1;
                uint8_t SC1 : 1;
                uint8_t SEG : 2;
                uint8_t SVE : 1;
                uint8_t VDST : 8;
            };
            struct layout_ENC_FLAT_SCRATCH {
                uint8_t ACC : 1;
                uint8_t ADDR : 8;
                uint8_t DATA : 8;
                uint8_t ENCODING : 6;
                uint8_t NT : 1;
                uint16_t OFFSET : 13;
                uint8_t OP : 7;
                uint8_t SADDR : 7;
                uint8_t SC0 : 1;
                uint8_t SC1 : 1;
                uint8_t SEG : 2;
                uint8_t SVE : 1;
                uint8_t VDST : 8;
            };
            struct layout_SOPK_INST_LITERAL_ {
                uint8_t ENCODING : 4;
                uint8_t OP : 5;
                uint8_t SDST : 7;
                uint16_t SIMM16 : 16;
                uint32_t SIMM32 : 32;
            };
            struct layout_ENC_VOP2_LITERAL {
                uint8_t ENCODING : 1;
                uint8_t OP : 6;
                uint32_t SIMM32 : 32;
                uint16_t SRC0 : 9;
                uint8_t VDST : 8;
                uint8_t VSRC1 : 8;
            };
            struct layout_ENC_VOP3B {
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
            struct layout_ENC_VOP3P_MFMA {
                uint8_t ABID : 4;
                uint8_t ACC : 2;
                uint8_t ACC_CD : 1;
                uint8_t BLGP : 3;
                uint8_t CBSZ : 3;
                uint16_t ENCODING : 9;
                uint8_t OP : 7;
                uint16_t SRC0 : 9;
                uint16_t SRC1 : 9;
                uint16_t SRC2 : 9;
                uint8_t VDST : 8;
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
                layout_ENC_VOP3P ENC_VOP3P;
                layout_ENC_VOP3 ENC_VOP3;
                layout_ENC_DS ENC_DS;
                layout_ENC_MUBUF ENC_MUBUF;
                layout_ENC_MTBUF ENC_MTBUF;
                layout_ENC_FLAT ENC_FLAT;
                layout_ENC_FLAT_GLBL ENC_FLAT_GLBL;
                layout_ENC_FLAT_SCRATCH ENC_FLAT_SCRATCH;
                layout_SOPK_INST_LITERAL_ SOPK_INST_LITERAL_;
                layout_ENC_VOP2_LITERAL ENC_VOP2_LITERAL;
                layout_ENC_VOP3B ENC_VOP3B;
                layout_ENC_VOP3P_MFMA ENC_VOP3P_MFMA;
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
            void decodeENC_VOP3P();
            void finalizeENC_VOP3POperands();
            void decodeENC_VOP3();
            void finalizeENC_VOP3Operands();
            void decodeENC_DS();
            void finalizeENC_DSOperands();
            void decodeENC_MUBUF();
            void finalizeENC_MUBUFOperands();
            void decodeENC_MTBUF();
            void finalizeENC_MTBUFOperands();
            void decodeENC_FLAT();
            void finalizeENC_FLATOperands();
            void decodeENC_FLAT_GLBL();
            void finalizeENC_FLAT_GLBLOperands();
            void decodeENC_FLAT_SCRATCH();
            void finalizeENC_FLAT_SCRATCHOperands();
            void decodeSOPK_INST_LITERAL_();
            void finalizeSOPK_INST_LITERAL_Operands();
            void decodeENC_VOP2_LITERAL();
            void finalizeENC_VOP2_LITERALOperands();
            void decodeENC_VOP3B();
            void finalizeENC_VOP3BOperands();
            void decodeENC_VOP3P_MFMA();
            void finalizeENC_VOP3P_MFMAOperands();
 
            
            Expression::Ptr decodeOPR_ACCVGPR(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_DSMEM(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_FLAT_SCRATCH(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_HWREG_ID(uint64_t input, uint32_t start, uint32_t end);
            Expression::Ptr decodeOPR_PC(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SDST(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SDST_EXEC(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SDST_M0(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SENDMSG_GSOP(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SENDMSG_MSG(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SRC(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SRC_ACCVGPR(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SRC_NOLDS(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SRC_NOLIT(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SRC_SIMPLE(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SRC_VGPR(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SRC_VGPR_OR_ACCVGPR(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SRC_VGPR_OR_ACCVGPR_OR_CONST(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SREG(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SREG_NOVCC(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SSRC(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SSRC_LANESEL(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SSRC_NOLIT(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_SSRC_SPECIAL_SCC(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_VCC(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_VGPR(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_VGPR_OR_ACCVGPR(uint64_t input, uint32_t output_vec_len = 1 );
            Expression::Ptr decodeOPR_VGPR_OR_LDS(uint64_t input, uint32_t output_vec_len = 1 );

            
            void appendOPR_LABEL(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_WAITCNT(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SIMM8(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SENDMSG(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_HWREG(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SIMM32(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SIMM16(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SIMM4(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_ACCVGPR(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_DSMEM(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_FLAT_SCRATCH(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_PC(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SDST(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SDST_EXEC(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SDST_M0(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void processOPR_SMEM_OFFSET(layout_ENC_SMEM & layout  );

            void appendOPR_SRC(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SRC_ACCVGPR(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SRC_NOLDS(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SRC_NOLIT(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SRC_SIMPLE(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SRC_VGPR(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SRC_VGPR_OR_ACCVGPR(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SRC_VGPR_OR_ACCVGPR_OR_CONST(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SREG(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SREG_NOVCC(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SSRC(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SSRC_LANESEL(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SSRC_NOLIT(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_SSRC_SPECIAL_SCC(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_VCC(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_VGPR(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_VGPR_OR_ACCVGPR(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);

            void appendOPR_VGPR_OR_LDS(uint64_t input, bool isRead, bool isWritten, uint32_t _num_elements = 1, bool isImplicit = false);



            struct amdgpu_gfx940_insn_entry {
                entryID op;
                const char *mnemonic;
            };

            
            const amdgpu_gfx940_insn_entry ENC_DS_insn_table[256] = 
            {
                {amdgpu_gfx940_op_DS_ADD_U32,"DS_ADD_U32"}, // 0
                {amdgpu_gfx940_op_DS_SUB_U32,"DS_SUB_U32"}, // 1
                {amdgpu_gfx940_op_DS_RSUB_U32,"DS_RSUB_U32"}, // 2
                {amdgpu_gfx940_op_DS_INC_U32,"DS_INC_U32"}, // 3
                {amdgpu_gfx940_op_DS_DEC_U32,"DS_DEC_U32"}, // 4
                {amdgpu_gfx940_op_DS_MIN_I32,"DS_MIN_I32"}, // 5
                {amdgpu_gfx940_op_DS_MAX_I32,"DS_MAX_I32"}, // 6
                {amdgpu_gfx940_op_DS_MIN_U32,"DS_MIN_U32"}, // 7
                {amdgpu_gfx940_op_DS_MAX_U32,"DS_MAX_U32"}, // 8
                {amdgpu_gfx940_op_DS_AND_B32,"DS_AND_B32"}, // 9
                {amdgpu_gfx940_op_DS_OR_B32,"DS_OR_B32"}, // 10
                {amdgpu_gfx940_op_DS_XOR_B32,"DS_XOR_B32"}, // 11
                {amdgpu_gfx940_op_DS_MSKOR_B32,"DS_MSKOR_B32"}, // 12
                {amdgpu_gfx940_op_DS_WRITE_B32,"DS_WRITE_B32"}, // 13
                {amdgpu_gfx940_op_DS_WRITE2_B32,"DS_WRITE2_B32"}, // 14
                {amdgpu_gfx940_op_DS_WRITE2ST64_B32,"DS_WRITE2ST64_B32"}, // 15
                {amdgpu_gfx940_op_DS_CMPST_B32,"DS_CMPST_B32"}, // 16
                {amdgpu_gfx940_op_DS_CMPST_F32,"DS_CMPST_F32"}, // 17
                {amdgpu_gfx940_op_DS_MIN_F32,"DS_MIN_F32"}, // 18
                {amdgpu_gfx940_op_DS_MAX_F32,"DS_MAX_F32"}, // 19
                {amdgpu_gfx940_op_DS_NOP,"DS_NOP"}, // 20
                {amdgpu_gfx940_op_DS_ADD_F32,"DS_ADD_F32"}, // 21
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 22
                {amdgpu_gfx940_op_DS_PK_ADD_F16,"DS_PK_ADD_F16"}, // 23
                {amdgpu_gfx940_op_DS_PK_ADD_BF16,"DS_PK_ADD_BF16"}, // 24
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 25
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 26
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 27
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 28
                {amdgpu_gfx940_op_DS_WRITE_ADDTID_B32,"DS_WRITE_ADDTID_B32"}, // 29
                {amdgpu_gfx940_op_DS_WRITE_B8,"DS_WRITE_B8"}, // 30
                {amdgpu_gfx940_op_DS_WRITE_B16,"DS_WRITE_B16"}, // 31
                {amdgpu_gfx940_op_DS_ADD_RTN_U32,"DS_ADD_RTN_U32"}, // 32
                {amdgpu_gfx940_op_DS_SUB_RTN_U32,"DS_SUB_RTN_U32"}, // 33
                {amdgpu_gfx940_op_DS_RSUB_RTN_U32,"DS_RSUB_RTN_U32"}, // 34
                {amdgpu_gfx940_op_DS_INC_RTN_U32,"DS_INC_RTN_U32"}, // 35
                {amdgpu_gfx940_op_DS_DEC_RTN_U32,"DS_DEC_RTN_U32"}, // 36
                {amdgpu_gfx940_op_DS_MIN_RTN_I32,"DS_MIN_RTN_I32"}, // 37
                {amdgpu_gfx940_op_DS_MAX_RTN_I32,"DS_MAX_RTN_I32"}, // 38
                {amdgpu_gfx940_op_DS_MIN_RTN_U32,"DS_MIN_RTN_U32"}, // 39
                {amdgpu_gfx940_op_DS_MAX_RTN_U32,"DS_MAX_RTN_U32"}, // 40
                {amdgpu_gfx940_op_DS_AND_RTN_B32,"DS_AND_RTN_B32"}, // 41
                {amdgpu_gfx940_op_DS_OR_RTN_B32,"DS_OR_RTN_B32"}, // 42
                {amdgpu_gfx940_op_DS_XOR_RTN_B32,"DS_XOR_RTN_B32"}, // 43
                {amdgpu_gfx940_op_DS_MSKOR_RTN_B32,"DS_MSKOR_RTN_B32"}, // 44
                {amdgpu_gfx940_op_DS_WRXCHG_RTN_B32,"DS_WRXCHG_RTN_B32"}, // 45
                {amdgpu_gfx940_op_DS_WRXCHG2_RTN_B32,"DS_WRXCHG2_RTN_B32"}, // 46
                {amdgpu_gfx940_op_DS_WRXCHG2ST64_RTN_B32,"DS_WRXCHG2ST64_RTN_B32"}, // 47
                {amdgpu_gfx940_op_DS_CMPST_RTN_B32,"DS_CMPST_RTN_B32"}, // 48
                {amdgpu_gfx940_op_DS_CMPST_RTN_F32,"DS_CMPST_RTN_F32"}, // 49
                {amdgpu_gfx940_op_DS_MIN_RTN_F32,"DS_MIN_RTN_F32"}, // 50
                {amdgpu_gfx940_op_DS_MAX_RTN_F32,"DS_MAX_RTN_F32"}, // 51
                {amdgpu_gfx940_op_DS_WRAP_RTN_B32,"DS_WRAP_RTN_B32"}, // 52
                {amdgpu_gfx940_op_DS_ADD_RTN_F32,"DS_ADD_RTN_F32"}, // 53
                {amdgpu_gfx940_op_DS_READ_B32,"DS_READ_B32"}, // 54
                {amdgpu_gfx940_op_DS_READ2_B32,"DS_READ2_B32"}, // 55
                {amdgpu_gfx940_op_DS_READ2ST64_B32,"DS_READ2ST64_B32"}, // 56
                {amdgpu_gfx940_op_DS_READ_I8,"DS_READ_I8"}, // 57
                {amdgpu_gfx940_op_DS_READ_U8,"DS_READ_U8"}, // 58
                {amdgpu_gfx940_op_DS_READ_I16,"DS_READ_I16"}, // 59
                {amdgpu_gfx940_op_DS_READ_U16,"DS_READ_U16"}, // 60
                {amdgpu_gfx940_op_DS_SWIZZLE_B32,"DS_SWIZZLE_B32"}, // 61
                {amdgpu_gfx940_op_DS_PERMUTE_B32,"DS_PERMUTE_B32"}, // 62
                {amdgpu_gfx940_op_DS_BPERMUTE_B32,"DS_BPERMUTE_B32"}, // 63
                {amdgpu_gfx940_op_DS_ADD_U64,"DS_ADD_U64"}, // 64
                {amdgpu_gfx940_op_DS_SUB_U64,"DS_SUB_U64"}, // 65
                {amdgpu_gfx940_op_DS_RSUB_U64,"DS_RSUB_U64"}, // 66
                {amdgpu_gfx940_op_DS_INC_U64,"DS_INC_U64"}, // 67
                {amdgpu_gfx940_op_DS_DEC_U64,"DS_DEC_U64"}, // 68
                {amdgpu_gfx940_op_DS_MIN_I64,"DS_MIN_I64"}, // 69
                {amdgpu_gfx940_op_DS_MAX_I64,"DS_MAX_I64"}, // 70
                {amdgpu_gfx940_op_DS_MIN_U64,"DS_MIN_U64"}, // 71
                {amdgpu_gfx940_op_DS_MAX_U64,"DS_MAX_U64"}, // 72
                {amdgpu_gfx940_op_DS_AND_B64,"DS_AND_B64"}, // 73
                {amdgpu_gfx940_op_DS_OR_B64,"DS_OR_B64"}, // 74
                {amdgpu_gfx940_op_DS_XOR_B64,"DS_XOR_B64"}, // 75
                {amdgpu_gfx940_op_DS_MSKOR_B64,"DS_MSKOR_B64"}, // 76
                {amdgpu_gfx940_op_DS_WRITE_B64,"DS_WRITE_B64"}, // 77
                {amdgpu_gfx940_op_DS_WRITE2_B64,"DS_WRITE2_B64"}, // 78
                {amdgpu_gfx940_op_DS_WRITE2ST64_B64,"DS_WRITE2ST64_B64"}, // 79
                {amdgpu_gfx940_op_DS_CMPST_B64,"DS_CMPST_B64"}, // 80
                {amdgpu_gfx940_op_DS_CMPST_F64,"DS_CMPST_F64"}, // 81
                {amdgpu_gfx940_op_DS_MIN_F64,"DS_MIN_F64"}, // 82
                {amdgpu_gfx940_op_DS_MAX_F64,"DS_MAX_F64"}, // 83
                {amdgpu_gfx940_op_DS_WRITE_B8_D16_HI,"DS_WRITE_B8_D16_HI"}, // 84
                {amdgpu_gfx940_op_DS_WRITE_B16_D16_HI,"DS_WRITE_B16_D16_HI"}, // 85
                {amdgpu_gfx940_op_DS_READ_U8_D16,"DS_READ_U8_D16"}, // 86
                {amdgpu_gfx940_op_DS_READ_U8_D16_HI,"DS_READ_U8_D16_HI"}, // 87
                {amdgpu_gfx940_op_DS_READ_I8_D16,"DS_READ_I8_D16"}, // 88
                {amdgpu_gfx940_op_DS_READ_I8_D16_HI,"DS_READ_I8_D16_HI"}, // 89
                {amdgpu_gfx940_op_DS_READ_U16_D16,"DS_READ_U16_D16"}, // 90
                {amdgpu_gfx940_op_DS_READ_U16_D16_HI,"DS_READ_U16_D16_HI"}, // 91
                {amdgpu_gfx940_op_DS_ADD_F64,"DS_ADD_F64"}, // 92
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 93
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 94
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 95
                {amdgpu_gfx940_op_DS_ADD_RTN_U64,"DS_ADD_RTN_U64"}, // 96
                {amdgpu_gfx940_op_DS_SUB_RTN_U64,"DS_SUB_RTN_U64"}, // 97
                {amdgpu_gfx940_op_DS_RSUB_RTN_U64,"DS_RSUB_RTN_U64"}, // 98
                {amdgpu_gfx940_op_DS_INC_RTN_U64,"DS_INC_RTN_U64"}, // 99
                {amdgpu_gfx940_op_DS_DEC_RTN_U64,"DS_DEC_RTN_U64"}, // 100
                {amdgpu_gfx940_op_DS_MIN_RTN_I64,"DS_MIN_RTN_I64"}, // 101
                {amdgpu_gfx940_op_DS_MAX_RTN_I64,"DS_MAX_RTN_I64"}, // 102
                {amdgpu_gfx940_op_DS_MIN_RTN_U64,"DS_MIN_RTN_U64"}, // 103
                {amdgpu_gfx940_op_DS_MAX_RTN_U64,"DS_MAX_RTN_U64"}, // 104
                {amdgpu_gfx940_op_DS_AND_RTN_B64,"DS_AND_RTN_B64"}, // 105
                {amdgpu_gfx940_op_DS_OR_RTN_B64,"DS_OR_RTN_B64"}, // 106
                {amdgpu_gfx940_op_DS_XOR_RTN_B64,"DS_XOR_RTN_B64"}, // 107
                {amdgpu_gfx940_op_DS_MSKOR_RTN_B64,"DS_MSKOR_RTN_B64"}, // 108
                {amdgpu_gfx940_op_DS_WRXCHG_RTN_B64,"DS_WRXCHG_RTN_B64"}, // 109
                {amdgpu_gfx940_op_DS_WRXCHG2_RTN_B64,"DS_WRXCHG2_RTN_B64"}, // 110
                {amdgpu_gfx940_op_DS_WRXCHG2ST64_RTN_B64,"DS_WRXCHG2ST64_RTN_B64"}, // 111
                {amdgpu_gfx940_op_DS_CMPST_RTN_B64,"DS_CMPST_RTN_B64"}, // 112
                {amdgpu_gfx940_op_DS_CMPST_RTN_F64,"DS_CMPST_RTN_F64"}, // 113
                {amdgpu_gfx940_op_DS_MIN_RTN_F64,"DS_MIN_RTN_F64"}, // 114
                {amdgpu_gfx940_op_DS_MAX_RTN_F64,"DS_MAX_RTN_F64"}, // 115
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 116
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 117
                {amdgpu_gfx940_op_DS_READ_B64,"DS_READ_B64"}, // 118
                {amdgpu_gfx940_op_DS_READ2_B64,"DS_READ2_B64"}, // 119
                {amdgpu_gfx940_op_DS_READ2ST64_B64,"DS_READ2ST64_B64"}, // 120
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 121
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 122
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 123
                {amdgpu_gfx940_op_DS_ADD_RTN_F64,"DS_ADD_RTN_F64"}, // 124
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 125
                {amdgpu_gfx940_op_DS_CONDXCHG32_RTN_B64,"DS_CONDXCHG32_RTN_B64"}, // 126
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 127
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 128
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 129
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 130
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 131
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 132
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 133
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 134
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 135
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 136
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 137
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 138
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 139
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 140
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 141
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 142
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 143
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 144
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 145
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 146
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 147
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 148
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 149
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 150
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 151
                {amdgpu_gfx940_op_DS_GWS_SEMA_RELEASE_ALL,"DS_GWS_SEMA_RELEASE_ALL"}, // 152
                {amdgpu_gfx940_op_DS_GWS_INIT,"DS_GWS_INIT"}, // 153
                {amdgpu_gfx940_op_DS_GWS_SEMA_V,"DS_GWS_SEMA_V"}, // 154
                {amdgpu_gfx940_op_DS_GWS_SEMA_BR,"DS_GWS_SEMA_BR"}, // 155
                {amdgpu_gfx940_op_DS_GWS_SEMA_P,"DS_GWS_SEMA_P"}, // 156
                {amdgpu_gfx940_op_DS_GWS_BARRIER,"DS_GWS_BARRIER"}, // 157
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 158
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 159
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 160
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 161
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 162
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 163
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 164
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 165
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 166
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 167
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 168
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 169
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 170
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 171
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 172
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 173
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 174
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 175
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 176
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 177
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 178
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 179
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 180
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 181
                {amdgpu_gfx940_op_DS_READ_ADDTID_B32,"DS_READ_ADDTID_B32"}, // 182
                {amdgpu_gfx940_op_DS_PK_ADD_RTN_F16,"DS_PK_ADD_RTN_F16"}, // 183
                {amdgpu_gfx940_op_DS_PK_ADD_RTN_BF16,"DS_PK_ADD_RTN_BF16"}, // 184
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 185
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 186
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 187
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 188
                {amdgpu_gfx940_op_DS_CONSUME,"DS_CONSUME"}, // 189
                {amdgpu_gfx940_op_DS_APPEND,"DS_APPEND"}, // 190
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 191
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 192
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 193
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 194
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 195
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 196
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 197
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 198
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 199
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 200
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 201
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 202
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 203
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 204
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 205
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 206
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 207
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 208
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 209
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 210
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 211
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 212
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 213
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 214
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 215
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 216
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 217
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 218
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 219
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 220
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 221
                {amdgpu_gfx940_op_DS_WRITE_B96,"DS_WRITE_B96"}, // 222
                {amdgpu_gfx940_op_DS_WRITE_B128,"DS_WRITE_B128"}, // 223
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 224
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 225
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 226
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 227
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 228
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 229
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 230
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 231
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 232
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 233
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 234
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 235
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 236
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 237
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 238
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 239
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 240
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 241
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 242
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 243
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 244
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 245
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 246
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 247
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 248
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 249
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 250
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 251
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 252
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 253
                {amdgpu_gfx940_op_DS_READ_B96,"DS_READ_B96"}, // 254
                {amdgpu_gfx940_op_DS_READ_B128,"DS_READ_B128"}, // 255
            }; // end ENC_DS_insn_table
            const amdgpu_gfx940_insn_entry ENC_FLAT_insn_table[109] = 
            {
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 0
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 1
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 2
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 3
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 4
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 5
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 6
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 7
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 8
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 9
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 10
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 11
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 12
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 13
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 14
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 15
                {amdgpu_gfx940_op_FLAT_LOAD_UBYTE,"FLAT_LOAD_UBYTE"}, // 16
                {amdgpu_gfx940_op_FLAT_LOAD_SBYTE,"FLAT_LOAD_SBYTE"}, // 17
                {amdgpu_gfx940_op_FLAT_LOAD_USHORT,"FLAT_LOAD_USHORT"}, // 18
                {amdgpu_gfx940_op_FLAT_LOAD_SSHORT,"FLAT_LOAD_SSHORT"}, // 19
                {amdgpu_gfx940_op_FLAT_LOAD_DWORD,"FLAT_LOAD_DWORD"}, // 20
                {amdgpu_gfx940_op_FLAT_LOAD_DWORDX2,"FLAT_LOAD_DWORDX2"}, // 21
                {amdgpu_gfx940_op_FLAT_LOAD_DWORDX3,"FLAT_LOAD_DWORDX3"}, // 22
                {amdgpu_gfx940_op_FLAT_LOAD_DWORDX4,"FLAT_LOAD_DWORDX4"}, // 23
                {amdgpu_gfx940_op_FLAT_STORE_BYTE,"FLAT_STORE_BYTE"}, // 24
                {amdgpu_gfx940_op_FLAT_STORE_BYTE_D16_HI,"FLAT_STORE_BYTE_D16_HI"}, // 25
                {amdgpu_gfx940_op_FLAT_STORE_SHORT,"FLAT_STORE_SHORT"}, // 26
                {amdgpu_gfx940_op_FLAT_STORE_SHORT_D16_HI,"FLAT_STORE_SHORT_D16_HI"}, // 27
                {amdgpu_gfx940_op_FLAT_STORE_DWORD,"FLAT_STORE_DWORD"}, // 28
                {amdgpu_gfx940_op_FLAT_STORE_DWORDX2,"FLAT_STORE_DWORDX2"}, // 29
                {amdgpu_gfx940_op_FLAT_STORE_DWORDX3,"FLAT_STORE_DWORDX3"}, // 30
                {amdgpu_gfx940_op_FLAT_STORE_DWORDX4,"FLAT_STORE_DWORDX4"}, // 31
                {amdgpu_gfx940_op_FLAT_LOAD_UBYTE_D16,"FLAT_LOAD_UBYTE_D16"}, // 32
                {amdgpu_gfx940_op_FLAT_LOAD_UBYTE_D16_HI,"FLAT_LOAD_UBYTE_D16_HI"}, // 33
                {amdgpu_gfx940_op_FLAT_LOAD_SBYTE_D16,"FLAT_LOAD_SBYTE_D16"}, // 34
                {amdgpu_gfx940_op_FLAT_LOAD_SBYTE_D16_HI,"FLAT_LOAD_SBYTE_D16_HI"}, // 35
                {amdgpu_gfx940_op_FLAT_LOAD_SHORT_D16,"FLAT_LOAD_SHORT_D16"}, // 36
                {amdgpu_gfx940_op_FLAT_LOAD_SHORT_D16_HI,"FLAT_LOAD_SHORT_D16_HI"}, // 37
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 38
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 39
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 40
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 41
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 42
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 43
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 44
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 45
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 46
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 47
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 48
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 49
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 50
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 51
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 52
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 53
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 54
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 55
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 56
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 57
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 58
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 59
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 60
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 61
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 62
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 63
                {amdgpu_gfx940_op_FLAT_ATOMIC_SWAP,"FLAT_ATOMIC_SWAP"}, // 64
                {amdgpu_gfx940_op_FLAT_ATOMIC_CMPSWAP,"FLAT_ATOMIC_CMPSWAP"}, // 65
                {amdgpu_gfx940_op_FLAT_ATOMIC_ADD,"FLAT_ATOMIC_ADD"}, // 66
                {amdgpu_gfx940_op_FLAT_ATOMIC_SUB,"FLAT_ATOMIC_SUB"}, // 67
                {amdgpu_gfx940_op_FLAT_ATOMIC_SMIN,"FLAT_ATOMIC_SMIN"}, // 68
                {amdgpu_gfx940_op_FLAT_ATOMIC_UMIN,"FLAT_ATOMIC_UMIN"}, // 69
                {amdgpu_gfx940_op_FLAT_ATOMIC_SMAX,"FLAT_ATOMIC_SMAX"}, // 70
                {amdgpu_gfx940_op_FLAT_ATOMIC_UMAX,"FLAT_ATOMIC_UMAX"}, // 71
                {amdgpu_gfx940_op_FLAT_ATOMIC_AND,"FLAT_ATOMIC_AND"}, // 72
                {amdgpu_gfx940_op_FLAT_ATOMIC_OR,"FLAT_ATOMIC_OR"}, // 73
                {amdgpu_gfx940_op_FLAT_ATOMIC_XOR,"FLAT_ATOMIC_XOR"}, // 74
                {amdgpu_gfx940_op_FLAT_ATOMIC_INC,"FLAT_ATOMIC_INC"}, // 75
                {amdgpu_gfx940_op_FLAT_ATOMIC_DEC,"FLAT_ATOMIC_DEC"}, // 76
                {amdgpu_gfx940_op_FLAT_ATOMIC_ADD_F32,"FLAT_ATOMIC_ADD_F32"}, // 77
                {amdgpu_gfx940_op_FLAT_ATOMIC_PK_ADD_F16,"FLAT_ATOMIC_PK_ADD_F16"}, // 78
                {amdgpu_gfx940_op_FLAT_ATOMIC_ADD_F64,"FLAT_ATOMIC_ADD_F64"}, // 79
                {amdgpu_gfx940_op_FLAT_ATOMIC_MIN_F64,"FLAT_ATOMIC_MIN_F64"}, // 80
                {amdgpu_gfx940_op_FLAT_ATOMIC_MAX_F64,"FLAT_ATOMIC_MAX_F64"}, // 81
                {amdgpu_gfx940_op_FLAT_ATOMIC_PK_ADD_BF16,"FLAT_ATOMIC_PK_ADD_BF16"}, // 82
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 83
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 84
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 85
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 86
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 87
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 88
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 89
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 90
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 91
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 92
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 93
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 94
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 95
                {amdgpu_gfx940_op_FLAT_ATOMIC_SWAP_X2,"FLAT_ATOMIC_SWAP_X2"}, // 96
                {amdgpu_gfx940_op_FLAT_ATOMIC_CMPSWAP_X2,"FLAT_ATOMIC_CMPSWAP_X2"}, // 97
                {amdgpu_gfx940_op_FLAT_ATOMIC_ADD_X2,"FLAT_ATOMIC_ADD_X2"}, // 98
                {amdgpu_gfx940_op_FLAT_ATOMIC_SUB_X2,"FLAT_ATOMIC_SUB_X2"}, // 99
                {amdgpu_gfx940_op_FLAT_ATOMIC_SMIN_X2,"FLAT_ATOMIC_SMIN_X2"}, // 100
                {amdgpu_gfx940_op_FLAT_ATOMIC_UMIN_X2,"FLAT_ATOMIC_UMIN_X2"}, // 101
                {amdgpu_gfx940_op_FLAT_ATOMIC_SMAX_X2,"FLAT_ATOMIC_SMAX_X2"}, // 102
                {amdgpu_gfx940_op_FLAT_ATOMIC_UMAX_X2,"FLAT_ATOMIC_UMAX_X2"}, // 103
                {amdgpu_gfx940_op_FLAT_ATOMIC_AND_X2,"FLAT_ATOMIC_AND_X2"}, // 104
                {amdgpu_gfx940_op_FLAT_ATOMIC_OR_X2,"FLAT_ATOMIC_OR_X2"}, // 105
                {amdgpu_gfx940_op_FLAT_ATOMIC_XOR_X2,"FLAT_ATOMIC_XOR_X2"}, // 106
                {amdgpu_gfx940_op_FLAT_ATOMIC_INC_X2,"FLAT_ATOMIC_INC_X2"}, // 107
                {amdgpu_gfx940_op_FLAT_ATOMIC_DEC_X2,"FLAT_ATOMIC_DEC_X2"}, // 108
            }; // end ENC_FLAT_insn_table
            const amdgpu_gfx940_insn_entry ENC_FLAT_GLBL_insn_table[109] = 
            {
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 0
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 1
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 2
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 3
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 4
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 5
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 6
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 7
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 8
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 9
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 10
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 11
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 12
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 13
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 14
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 15
                {amdgpu_gfx940_op_GLOBAL_LOAD_UBYTE,"GLOBAL_LOAD_UBYTE"}, // 16
                {amdgpu_gfx940_op_GLOBAL_LOAD_SBYTE,"GLOBAL_LOAD_SBYTE"}, // 17
                {amdgpu_gfx940_op_GLOBAL_LOAD_USHORT,"GLOBAL_LOAD_USHORT"}, // 18
                {amdgpu_gfx940_op_GLOBAL_LOAD_SSHORT,"GLOBAL_LOAD_SSHORT"}, // 19
                {amdgpu_gfx940_op_GLOBAL_LOAD_DWORD,"GLOBAL_LOAD_DWORD"}, // 20
                {amdgpu_gfx940_op_GLOBAL_LOAD_DWORDX2,"GLOBAL_LOAD_DWORDX2"}, // 21
                {amdgpu_gfx940_op_GLOBAL_LOAD_DWORDX3,"GLOBAL_LOAD_DWORDX3"}, // 22
                {amdgpu_gfx940_op_GLOBAL_LOAD_DWORDX4,"GLOBAL_LOAD_DWORDX4"}, // 23
                {amdgpu_gfx940_op_GLOBAL_STORE_BYTE,"GLOBAL_STORE_BYTE"}, // 24
                {amdgpu_gfx940_op_GLOBAL_STORE_BYTE_D16_HI,"GLOBAL_STORE_BYTE_D16_HI"}, // 25
                {amdgpu_gfx940_op_GLOBAL_STORE_SHORT,"GLOBAL_STORE_SHORT"}, // 26
                {amdgpu_gfx940_op_GLOBAL_STORE_SHORT_D16_HI,"GLOBAL_STORE_SHORT_D16_HI"}, // 27
                {amdgpu_gfx940_op_GLOBAL_STORE_DWORD,"GLOBAL_STORE_DWORD"}, // 28
                {amdgpu_gfx940_op_GLOBAL_STORE_DWORDX2,"GLOBAL_STORE_DWORDX2"}, // 29
                {amdgpu_gfx940_op_GLOBAL_STORE_DWORDX3,"GLOBAL_STORE_DWORDX3"}, // 30
                {amdgpu_gfx940_op_GLOBAL_STORE_DWORDX4,"GLOBAL_STORE_DWORDX4"}, // 31
                {amdgpu_gfx940_op_GLOBAL_LOAD_UBYTE_D16,"GLOBAL_LOAD_UBYTE_D16"}, // 32
                {amdgpu_gfx940_op_GLOBAL_LOAD_UBYTE_D16_HI,"GLOBAL_LOAD_UBYTE_D16_HI"}, // 33
                {amdgpu_gfx940_op_GLOBAL_LOAD_SBYTE_D16,"GLOBAL_LOAD_SBYTE_D16"}, // 34
                {amdgpu_gfx940_op_GLOBAL_LOAD_SBYTE_D16_HI,"GLOBAL_LOAD_SBYTE_D16_HI"}, // 35
                {amdgpu_gfx940_op_GLOBAL_LOAD_SHORT_D16,"GLOBAL_LOAD_SHORT_D16"}, // 36
                {amdgpu_gfx940_op_GLOBAL_LOAD_SHORT_D16_HI,"GLOBAL_LOAD_SHORT_D16_HI"}, // 37
                {amdgpu_gfx940_op_GLOBAL_LOAD_LDS_UBYTE,"GLOBAL_LOAD_LDS_UBYTE"}, // 38
                {amdgpu_gfx940_op_GLOBAL_LOAD_LDS_SBYTE,"GLOBAL_LOAD_LDS_SBYTE"}, // 39
                {amdgpu_gfx940_op_GLOBAL_LOAD_LDS_USHORT,"GLOBAL_LOAD_LDS_USHORT"}, // 40
                {amdgpu_gfx940_op_GLOBAL_LOAD_LDS_SSHORT,"GLOBAL_LOAD_LDS_SSHORT"}, // 41
                {amdgpu_gfx940_op_GLOBAL_LOAD_LDS_DWORD,"GLOBAL_LOAD_LDS_DWORD"}, // 42
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 43
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 44
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 45
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 46
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 47
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 48
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 49
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 50
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 51
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 52
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 53
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 54
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 55
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 56
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 57
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 58
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 59
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 60
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 61
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 62
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 63
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_SWAP,"GLOBAL_ATOMIC_SWAP"}, // 64
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_CMPSWAP,"GLOBAL_ATOMIC_CMPSWAP"}, // 65
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_ADD,"GLOBAL_ATOMIC_ADD"}, // 66
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_SUB,"GLOBAL_ATOMIC_SUB"}, // 67
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_SMIN,"GLOBAL_ATOMIC_SMIN"}, // 68
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_UMIN,"GLOBAL_ATOMIC_UMIN"}, // 69
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_SMAX,"GLOBAL_ATOMIC_SMAX"}, // 70
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_UMAX,"GLOBAL_ATOMIC_UMAX"}, // 71
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_AND,"GLOBAL_ATOMIC_AND"}, // 72
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_OR,"GLOBAL_ATOMIC_OR"}, // 73
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_XOR,"GLOBAL_ATOMIC_XOR"}, // 74
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_INC,"GLOBAL_ATOMIC_INC"}, // 75
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_DEC,"GLOBAL_ATOMIC_DEC"}, // 76
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_ADD_F32,"GLOBAL_ATOMIC_ADD_F32"}, // 77
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_PK_ADD_F16,"GLOBAL_ATOMIC_PK_ADD_F16"}, // 78
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_ADD_F64,"GLOBAL_ATOMIC_ADD_F64"}, // 79
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_MIN_F64,"GLOBAL_ATOMIC_MIN_F64"}, // 80
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_MAX_F64,"GLOBAL_ATOMIC_MAX_F64"}, // 81
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_PK_ADD_BF16,"GLOBAL_ATOMIC_PK_ADD_BF16"}, // 82
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 83
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 84
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 85
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 86
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 87
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 88
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 89
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 90
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 91
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 92
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 93
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 94
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 95
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_SWAP_X2,"GLOBAL_ATOMIC_SWAP_X2"}, // 96
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_CMPSWAP_X2,"GLOBAL_ATOMIC_CMPSWAP_X2"}, // 97
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_ADD_X2,"GLOBAL_ATOMIC_ADD_X2"}, // 98
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_SUB_X2,"GLOBAL_ATOMIC_SUB_X2"}, // 99
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_SMIN_X2,"GLOBAL_ATOMIC_SMIN_X2"}, // 100
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_UMIN_X2,"GLOBAL_ATOMIC_UMIN_X2"}, // 101
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_SMAX_X2,"GLOBAL_ATOMIC_SMAX_X2"}, // 102
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_UMAX_X2,"GLOBAL_ATOMIC_UMAX_X2"}, // 103
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_AND_X2,"GLOBAL_ATOMIC_AND_X2"}, // 104
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_OR_X2,"GLOBAL_ATOMIC_OR_X2"}, // 105
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_XOR_X2,"GLOBAL_ATOMIC_XOR_X2"}, // 106
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_INC_X2,"GLOBAL_ATOMIC_INC_X2"}, // 107
                {amdgpu_gfx940_op_GLOBAL_ATOMIC_DEC_X2,"GLOBAL_ATOMIC_DEC_X2"}, // 108
            }; // end ENC_FLAT_GLBL_insn_table
            const amdgpu_gfx940_insn_entry ENC_FLAT_SCRATCH_insn_table[43] = 
            {
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 0
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 1
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 2
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 3
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 4
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 5
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 6
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 7
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 8
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 9
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 10
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 11
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 12
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 13
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 14
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 15
                {amdgpu_gfx940_op_SCRATCH_LOAD_UBYTE,"SCRATCH_LOAD_UBYTE"}, // 16
                {amdgpu_gfx940_op_SCRATCH_LOAD_SBYTE,"SCRATCH_LOAD_SBYTE"}, // 17
                {amdgpu_gfx940_op_SCRATCH_LOAD_USHORT,"SCRATCH_LOAD_USHORT"}, // 18
                {amdgpu_gfx940_op_SCRATCH_LOAD_SSHORT,"SCRATCH_LOAD_SSHORT"}, // 19
                {amdgpu_gfx940_op_SCRATCH_LOAD_DWORD,"SCRATCH_LOAD_DWORD"}, // 20
                {amdgpu_gfx940_op_SCRATCH_LOAD_DWORDX2,"SCRATCH_LOAD_DWORDX2"}, // 21
                {amdgpu_gfx940_op_SCRATCH_LOAD_DWORDX3,"SCRATCH_LOAD_DWORDX3"}, // 22
                {amdgpu_gfx940_op_SCRATCH_LOAD_DWORDX4,"SCRATCH_LOAD_DWORDX4"}, // 23
                {amdgpu_gfx940_op_SCRATCH_STORE_BYTE,"SCRATCH_STORE_BYTE"}, // 24
                {amdgpu_gfx940_op_SCRATCH_STORE_BYTE_D16_HI,"SCRATCH_STORE_BYTE_D16_HI"}, // 25
                {amdgpu_gfx940_op_SCRATCH_STORE_SHORT,"SCRATCH_STORE_SHORT"}, // 26
                {amdgpu_gfx940_op_SCRATCH_STORE_SHORT_D16_HI,"SCRATCH_STORE_SHORT_D16_HI"}, // 27
                {amdgpu_gfx940_op_SCRATCH_STORE_DWORD,"SCRATCH_STORE_DWORD"}, // 28
                {amdgpu_gfx940_op_SCRATCH_STORE_DWORDX2,"SCRATCH_STORE_DWORDX2"}, // 29
                {amdgpu_gfx940_op_SCRATCH_STORE_DWORDX3,"SCRATCH_STORE_DWORDX3"}, // 30
                {amdgpu_gfx940_op_SCRATCH_STORE_DWORDX4,"SCRATCH_STORE_DWORDX4"}, // 31
                {amdgpu_gfx940_op_SCRATCH_LOAD_UBYTE_D16,"SCRATCH_LOAD_UBYTE_D16"}, // 32
                {amdgpu_gfx940_op_SCRATCH_LOAD_UBYTE_D16_HI,"SCRATCH_LOAD_UBYTE_D16_HI"}, // 33
                {amdgpu_gfx940_op_SCRATCH_LOAD_SBYTE_D16,"SCRATCH_LOAD_SBYTE_D16"}, // 34
                {amdgpu_gfx940_op_SCRATCH_LOAD_SBYTE_D16_HI,"SCRATCH_LOAD_SBYTE_D16_HI"}, // 35
                {amdgpu_gfx940_op_SCRATCH_LOAD_SHORT_D16,"SCRATCH_LOAD_SHORT_D16"}, // 36
                {amdgpu_gfx940_op_SCRATCH_LOAD_SHORT_D16_HI,"SCRATCH_LOAD_SHORT_D16_HI"}, // 37
                {amdgpu_gfx940_op_SCRATCH_LOAD_LDS_UBYTE,"SCRATCH_LOAD_LDS_UBYTE"}, // 38
                {amdgpu_gfx940_op_SCRATCH_LOAD_LDS_SBYTE,"SCRATCH_LOAD_LDS_SBYTE"}, // 39
                {amdgpu_gfx940_op_SCRATCH_LOAD_LDS_USHORT,"SCRATCH_LOAD_LDS_USHORT"}, // 40
                {amdgpu_gfx940_op_SCRATCH_LOAD_LDS_SSHORT,"SCRATCH_LOAD_LDS_SSHORT"}, // 41
                {amdgpu_gfx940_op_SCRATCH_LOAD_LDS_DWORD,"SCRATCH_LOAD_LDS_DWORD"}, // 42
            }; // end ENC_FLAT_SCRATCH_insn_table
            const amdgpu_gfx940_insn_entry ENC_MTBUF_insn_table[16] = 
            {
                {amdgpu_gfx940_op_TBUFFER_LOAD_FORMAT_X,"TBUFFER_LOAD_FORMAT_X"}, // 0
                {amdgpu_gfx940_op_TBUFFER_LOAD_FORMAT_XY,"TBUFFER_LOAD_FORMAT_XY"}, // 1
                {amdgpu_gfx940_op_TBUFFER_LOAD_FORMAT_XYZ,"TBUFFER_LOAD_FORMAT_XYZ"}, // 2
                {amdgpu_gfx940_op_TBUFFER_LOAD_FORMAT_XYZW,"TBUFFER_LOAD_FORMAT_XYZW"}, // 3
                {amdgpu_gfx940_op_TBUFFER_STORE_FORMAT_X,"TBUFFER_STORE_FORMAT_X"}, // 4
                {amdgpu_gfx940_op_TBUFFER_STORE_FORMAT_XY,"TBUFFER_STORE_FORMAT_XY"}, // 5
                {amdgpu_gfx940_op_TBUFFER_STORE_FORMAT_XYZ,"TBUFFER_STORE_FORMAT_XYZ"}, // 6
                {amdgpu_gfx940_op_TBUFFER_STORE_FORMAT_XYZW,"TBUFFER_STORE_FORMAT_XYZW"}, // 7
                {amdgpu_gfx940_op_TBUFFER_LOAD_FORMAT_D16_X,"TBUFFER_LOAD_FORMAT_D16_X"}, // 8
                {amdgpu_gfx940_op_TBUFFER_LOAD_FORMAT_D16_XY,"TBUFFER_LOAD_FORMAT_D16_XY"}, // 9
                {amdgpu_gfx940_op_TBUFFER_LOAD_FORMAT_D16_XYZ,"TBUFFER_LOAD_FORMAT_D16_XYZ"}, // 10
                {amdgpu_gfx940_op_TBUFFER_LOAD_FORMAT_D16_XYZW,"TBUFFER_LOAD_FORMAT_D16_XYZW"}, // 11
                {amdgpu_gfx940_op_TBUFFER_STORE_FORMAT_D16_X,"TBUFFER_STORE_FORMAT_D16_X"}, // 12
                {amdgpu_gfx940_op_TBUFFER_STORE_FORMAT_D16_XY,"TBUFFER_STORE_FORMAT_D16_XY"}, // 13
                {amdgpu_gfx940_op_TBUFFER_STORE_FORMAT_D16_XYZ,"TBUFFER_STORE_FORMAT_D16_XYZ"}, // 14
                {amdgpu_gfx940_op_TBUFFER_STORE_FORMAT_D16_XYZW,"TBUFFER_STORE_FORMAT_D16_XYZW"}, // 15
            }; // end ENC_MTBUF_insn_table
            const amdgpu_gfx940_insn_entry ENC_MUBUF_insn_table[109] = 
            {
                {amdgpu_gfx940_op_BUFFER_LOAD_FORMAT_X,"BUFFER_LOAD_FORMAT_X"}, // 0
                {amdgpu_gfx940_op_BUFFER_LOAD_FORMAT_XY,"BUFFER_LOAD_FORMAT_XY"}, // 1
                {amdgpu_gfx940_op_BUFFER_LOAD_FORMAT_XYZ,"BUFFER_LOAD_FORMAT_XYZ"}, // 2
                {amdgpu_gfx940_op_BUFFER_LOAD_FORMAT_XYZW,"BUFFER_LOAD_FORMAT_XYZW"}, // 3
                {amdgpu_gfx940_op_BUFFER_STORE_FORMAT_X,"BUFFER_STORE_FORMAT_X"}, // 4
                {amdgpu_gfx940_op_BUFFER_STORE_FORMAT_XY,"BUFFER_STORE_FORMAT_XY"}, // 5
                {amdgpu_gfx940_op_BUFFER_STORE_FORMAT_XYZ,"BUFFER_STORE_FORMAT_XYZ"}, // 6
                {amdgpu_gfx940_op_BUFFER_STORE_FORMAT_XYZW,"BUFFER_STORE_FORMAT_XYZW"}, // 7
                {amdgpu_gfx940_op_BUFFER_LOAD_FORMAT_D16_X,"BUFFER_LOAD_FORMAT_D16_X"}, // 8
                {amdgpu_gfx940_op_BUFFER_LOAD_FORMAT_D16_XY,"BUFFER_LOAD_FORMAT_D16_XY"}, // 9
                {amdgpu_gfx940_op_BUFFER_LOAD_FORMAT_D16_XYZ,"BUFFER_LOAD_FORMAT_D16_XYZ"}, // 10
                {amdgpu_gfx940_op_BUFFER_LOAD_FORMAT_D16_XYZW,"BUFFER_LOAD_FORMAT_D16_XYZW"}, // 11
                {amdgpu_gfx940_op_BUFFER_STORE_FORMAT_D16_X,"BUFFER_STORE_FORMAT_D16_X"}, // 12
                {amdgpu_gfx940_op_BUFFER_STORE_FORMAT_D16_XY,"BUFFER_STORE_FORMAT_D16_XY"}, // 13
                {amdgpu_gfx940_op_BUFFER_STORE_FORMAT_D16_XYZ,"BUFFER_STORE_FORMAT_D16_XYZ"}, // 14
                {amdgpu_gfx940_op_BUFFER_STORE_FORMAT_D16_XYZW,"BUFFER_STORE_FORMAT_D16_XYZW"}, // 15
                {amdgpu_gfx940_op_BUFFER_LOAD_UBYTE,"BUFFER_LOAD_UBYTE"}, // 16
                {amdgpu_gfx940_op_BUFFER_LOAD_SBYTE,"BUFFER_LOAD_SBYTE"}, // 17
                {amdgpu_gfx940_op_BUFFER_LOAD_USHORT,"BUFFER_LOAD_USHORT"}, // 18
                {amdgpu_gfx940_op_BUFFER_LOAD_SSHORT,"BUFFER_LOAD_SSHORT"}, // 19
                {amdgpu_gfx940_op_BUFFER_LOAD_DWORD,"BUFFER_LOAD_DWORD"}, // 20
                {amdgpu_gfx940_op_BUFFER_LOAD_DWORDX2,"BUFFER_LOAD_DWORDX2"}, // 21
                {amdgpu_gfx940_op_BUFFER_LOAD_DWORDX3,"BUFFER_LOAD_DWORDX3"}, // 22
                {amdgpu_gfx940_op_BUFFER_LOAD_DWORDX4,"BUFFER_LOAD_DWORDX4"}, // 23
                {amdgpu_gfx940_op_BUFFER_STORE_BYTE,"BUFFER_STORE_BYTE"}, // 24
                {amdgpu_gfx940_op_BUFFER_STORE_BYTE_D16_HI,"BUFFER_STORE_BYTE_D16_HI"}, // 25
                {amdgpu_gfx940_op_BUFFER_STORE_SHORT,"BUFFER_STORE_SHORT"}, // 26
                {amdgpu_gfx940_op_BUFFER_STORE_SHORT_D16_HI,"BUFFER_STORE_SHORT_D16_HI"}, // 27
                {amdgpu_gfx940_op_BUFFER_STORE_DWORD,"BUFFER_STORE_DWORD"}, // 28
                {amdgpu_gfx940_op_BUFFER_STORE_DWORDX2,"BUFFER_STORE_DWORDX2"}, // 29
                {amdgpu_gfx940_op_BUFFER_STORE_DWORDX3,"BUFFER_STORE_DWORDX3"}, // 30
                {amdgpu_gfx940_op_BUFFER_STORE_DWORDX4,"BUFFER_STORE_DWORDX4"}, // 31
                {amdgpu_gfx940_op_BUFFER_LOAD_UBYTE_D16,"BUFFER_LOAD_UBYTE_D16"}, // 32
                {amdgpu_gfx940_op_BUFFER_LOAD_UBYTE_D16_HI,"BUFFER_LOAD_UBYTE_D16_HI"}, // 33
                {amdgpu_gfx940_op_BUFFER_LOAD_SBYTE_D16,"BUFFER_LOAD_SBYTE_D16"}, // 34
                {amdgpu_gfx940_op_BUFFER_LOAD_SBYTE_D16_HI,"BUFFER_LOAD_SBYTE_D16_HI"}, // 35
                {amdgpu_gfx940_op_BUFFER_LOAD_SHORT_D16,"BUFFER_LOAD_SHORT_D16"}, // 36
                {amdgpu_gfx940_op_BUFFER_LOAD_SHORT_D16_HI,"BUFFER_LOAD_SHORT_D16_HI"}, // 37
                {amdgpu_gfx940_op_BUFFER_LOAD_FORMAT_D16_HI_X,"BUFFER_LOAD_FORMAT_D16_HI_X"}, // 38
                {amdgpu_gfx940_op_BUFFER_STORE_FORMAT_D16_HI_X,"BUFFER_STORE_FORMAT_D16_HI_X"}, // 39
                {amdgpu_gfx940_op_BUFFER_WBL2,"BUFFER_WBL2"}, // 40
                {amdgpu_gfx940_op_BUFFER_INV,"BUFFER_INV"}, // 41
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 42
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 43
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 44
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 45
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 46
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 47
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 48
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 49
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 50
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 51
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 52
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 53
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 54
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 55
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 56
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 57
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 58
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 59
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 60
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 61
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 62
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 63
                {amdgpu_gfx940_op_BUFFER_ATOMIC_SWAP,"BUFFER_ATOMIC_SWAP"}, // 64
                {amdgpu_gfx940_op_BUFFER_ATOMIC_CMPSWAP,"BUFFER_ATOMIC_CMPSWAP"}, // 65
                {amdgpu_gfx940_op_BUFFER_ATOMIC_ADD,"BUFFER_ATOMIC_ADD"}, // 66
                {amdgpu_gfx940_op_BUFFER_ATOMIC_SUB,"BUFFER_ATOMIC_SUB"}, // 67
                {amdgpu_gfx940_op_BUFFER_ATOMIC_SMIN,"BUFFER_ATOMIC_SMIN"}, // 68
                {amdgpu_gfx940_op_BUFFER_ATOMIC_UMIN,"BUFFER_ATOMIC_UMIN"}, // 69
                {amdgpu_gfx940_op_BUFFER_ATOMIC_SMAX,"BUFFER_ATOMIC_SMAX"}, // 70
                {amdgpu_gfx940_op_BUFFER_ATOMIC_UMAX,"BUFFER_ATOMIC_UMAX"}, // 71
                {amdgpu_gfx940_op_BUFFER_ATOMIC_AND,"BUFFER_ATOMIC_AND"}, // 72
                {amdgpu_gfx940_op_BUFFER_ATOMIC_OR,"BUFFER_ATOMIC_OR"}, // 73
                {amdgpu_gfx940_op_BUFFER_ATOMIC_XOR,"BUFFER_ATOMIC_XOR"}, // 74
                {amdgpu_gfx940_op_BUFFER_ATOMIC_INC,"BUFFER_ATOMIC_INC"}, // 75
                {amdgpu_gfx940_op_BUFFER_ATOMIC_DEC,"BUFFER_ATOMIC_DEC"}, // 76
                {amdgpu_gfx940_op_BUFFER_ATOMIC_ADD_F32,"BUFFER_ATOMIC_ADD_F32"}, // 77
                {amdgpu_gfx940_op_BUFFER_ATOMIC_PK_ADD_F16,"BUFFER_ATOMIC_PK_ADD_F16"}, // 78
                {amdgpu_gfx940_op_BUFFER_ATOMIC_ADD_F64,"BUFFER_ATOMIC_ADD_F64"}, // 79
                {amdgpu_gfx940_op_BUFFER_ATOMIC_MIN_F64,"BUFFER_ATOMIC_MIN_F64"}, // 80
                {amdgpu_gfx940_op_BUFFER_ATOMIC_MAX_F64,"BUFFER_ATOMIC_MAX_F64"}, // 81
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 82
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 83
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 84
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 85
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 86
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 87
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 88
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 89
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 90
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 91
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 92
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 93
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 94
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 95
                {amdgpu_gfx940_op_BUFFER_ATOMIC_SWAP_X2,"BUFFER_ATOMIC_SWAP_X2"}, // 96
                {amdgpu_gfx940_op_BUFFER_ATOMIC_CMPSWAP_X2,"BUFFER_ATOMIC_CMPSWAP_X2"}, // 97
                {amdgpu_gfx940_op_BUFFER_ATOMIC_ADD_X2,"BUFFER_ATOMIC_ADD_X2"}, // 98
                {amdgpu_gfx940_op_BUFFER_ATOMIC_SUB_X2,"BUFFER_ATOMIC_SUB_X2"}, // 99
                {amdgpu_gfx940_op_BUFFER_ATOMIC_SMIN_X2,"BUFFER_ATOMIC_SMIN_X2"}, // 100
                {amdgpu_gfx940_op_BUFFER_ATOMIC_UMIN_X2,"BUFFER_ATOMIC_UMIN_X2"}, // 101
                {amdgpu_gfx940_op_BUFFER_ATOMIC_SMAX_X2,"BUFFER_ATOMIC_SMAX_X2"}, // 102
                {amdgpu_gfx940_op_BUFFER_ATOMIC_UMAX_X2,"BUFFER_ATOMIC_UMAX_X2"}, // 103
                {amdgpu_gfx940_op_BUFFER_ATOMIC_AND_X2,"BUFFER_ATOMIC_AND_X2"}, // 104
                {amdgpu_gfx940_op_BUFFER_ATOMIC_OR_X2,"BUFFER_ATOMIC_OR_X2"}, // 105
                {amdgpu_gfx940_op_BUFFER_ATOMIC_XOR_X2,"BUFFER_ATOMIC_XOR_X2"}, // 106
                {amdgpu_gfx940_op_BUFFER_ATOMIC_INC_X2,"BUFFER_ATOMIC_INC_X2"}, // 107
                {amdgpu_gfx940_op_BUFFER_ATOMIC_DEC_X2,"BUFFER_ATOMIC_DEC_X2"}, // 108
            }; // end ENC_MUBUF_insn_table
            const amdgpu_gfx940_insn_entry ENC_SMEM_insn_table[173] = 
            {
                {amdgpu_gfx940_op_S_LOAD_DWORD,"S_LOAD_DWORD"}, // 0
                {amdgpu_gfx940_op_S_LOAD_DWORDX2,"S_LOAD_DWORDX2"}, // 1
                {amdgpu_gfx940_op_S_LOAD_DWORDX4,"S_LOAD_DWORDX4"}, // 2
                {amdgpu_gfx940_op_S_LOAD_DWORDX8,"S_LOAD_DWORDX8"}, // 3
                {amdgpu_gfx940_op_S_LOAD_DWORDX16,"S_LOAD_DWORDX16"}, // 4
                {amdgpu_gfx940_op_S_SCRATCH_LOAD_DWORD,"S_SCRATCH_LOAD_DWORD"}, // 5
                {amdgpu_gfx940_op_S_SCRATCH_LOAD_DWORDX2,"S_SCRATCH_LOAD_DWORDX2"}, // 6
                {amdgpu_gfx940_op_S_SCRATCH_LOAD_DWORDX4,"S_SCRATCH_LOAD_DWORDX4"}, // 7
                {amdgpu_gfx940_op_S_BUFFER_LOAD_DWORD,"S_BUFFER_LOAD_DWORD"}, // 8
                {amdgpu_gfx940_op_S_BUFFER_LOAD_DWORDX2,"S_BUFFER_LOAD_DWORDX2"}, // 9
                {amdgpu_gfx940_op_S_BUFFER_LOAD_DWORDX4,"S_BUFFER_LOAD_DWORDX4"}, // 10
                {amdgpu_gfx940_op_S_BUFFER_LOAD_DWORDX8,"S_BUFFER_LOAD_DWORDX8"}, // 11
                {amdgpu_gfx940_op_S_BUFFER_LOAD_DWORDX16,"S_BUFFER_LOAD_DWORDX16"}, // 12
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 13
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 14
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 15
                {amdgpu_gfx940_op_S_STORE_DWORD,"S_STORE_DWORD"}, // 16
                {amdgpu_gfx940_op_S_STORE_DWORDX2,"S_STORE_DWORDX2"}, // 17
                {amdgpu_gfx940_op_S_STORE_DWORDX4,"S_STORE_DWORDX4"}, // 18
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 19
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 20
                {amdgpu_gfx940_op_S_SCRATCH_STORE_DWORD,"S_SCRATCH_STORE_DWORD"}, // 21
                {amdgpu_gfx940_op_S_SCRATCH_STORE_DWORDX2,"S_SCRATCH_STORE_DWORDX2"}, // 22
                {amdgpu_gfx940_op_S_SCRATCH_STORE_DWORDX4,"S_SCRATCH_STORE_DWORDX4"}, // 23
                {amdgpu_gfx940_op_S_BUFFER_STORE_DWORD,"S_BUFFER_STORE_DWORD"}, // 24
                {amdgpu_gfx940_op_S_BUFFER_STORE_DWORDX2,"S_BUFFER_STORE_DWORDX2"}, // 25
                {amdgpu_gfx940_op_S_BUFFER_STORE_DWORDX4,"S_BUFFER_STORE_DWORDX4"}, // 26
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 27
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 28
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 29
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 30
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 31
                {amdgpu_gfx940_op_S_DCACHE_INV,"S_DCACHE_INV"}, // 32
                {amdgpu_gfx940_op_S_DCACHE_WB,"S_DCACHE_WB"}, // 33
                {amdgpu_gfx940_op_S_DCACHE_INV_VOL,"S_DCACHE_INV_VOL"}, // 34
                {amdgpu_gfx940_op_S_DCACHE_WB_VOL,"S_DCACHE_WB_VOL"}, // 35
                {amdgpu_gfx940_op_S_MEMTIME,"S_MEMTIME"}, // 36
                {amdgpu_gfx940_op_S_MEMREALTIME,"S_MEMREALTIME"}, // 37
                {amdgpu_gfx940_op_S_ATC_PROBE,"S_ATC_PROBE"}, // 38
                {amdgpu_gfx940_op_S_ATC_PROBE_BUFFER,"S_ATC_PROBE_BUFFER"}, // 39
                {amdgpu_gfx940_op_S_DCACHE_DISCARD,"S_DCACHE_DISCARD"}, // 40
                {amdgpu_gfx940_op_S_DCACHE_DISCARD_X2,"S_DCACHE_DISCARD_X2"}, // 41
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 42
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 43
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 44
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 45
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 46
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 47
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 48
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 49
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 50
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 51
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 52
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 53
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 54
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 55
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 56
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 57
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 58
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 59
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 60
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 61
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 62
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 63
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_SWAP,"S_BUFFER_ATOMIC_SWAP"}, // 64
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_CMPSWAP,"S_BUFFER_ATOMIC_CMPSWAP"}, // 65
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_ADD,"S_BUFFER_ATOMIC_ADD"}, // 66
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_SUB,"S_BUFFER_ATOMIC_SUB"}, // 67
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_SMIN,"S_BUFFER_ATOMIC_SMIN"}, // 68
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_UMIN,"S_BUFFER_ATOMIC_UMIN"}, // 69
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_SMAX,"S_BUFFER_ATOMIC_SMAX"}, // 70
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_UMAX,"S_BUFFER_ATOMIC_UMAX"}, // 71
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_AND,"S_BUFFER_ATOMIC_AND"}, // 72
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_OR,"S_BUFFER_ATOMIC_OR"}, // 73
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_XOR,"S_BUFFER_ATOMIC_XOR"}, // 74
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_INC,"S_BUFFER_ATOMIC_INC"}, // 75
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_DEC,"S_BUFFER_ATOMIC_DEC"}, // 76
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 77
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 78
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 79
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 80
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 81
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 82
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 83
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 84
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 85
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 86
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 87
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 88
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 89
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 90
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 91
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 92
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 93
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 94
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 95
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_SWAP_X2,"S_BUFFER_ATOMIC_SWAP_X2"}, // 96
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_CMPSWAP_X2,"S_BUFFER_ATOMIC_CMPSWAP_X2"}, // 97
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_ADD_X2,"S_BUFFER_ATOMIC_ADD_X2"}, // 98
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_SUB_X2,"S_BUFFER_ATOMIC_SUB_X2"}, // 99
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_SMIN_X2,"S_BUFFER_ATOMIC_SMIN_X2"}, // 100
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_UMIN_X2,"S_BUFFER_ATOMIC_UMIN_X2"}, // 101
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_SMAX_X2,"S_BUFFER_ATOMIC_SMAX_X2"}, // 102
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_UMAX_X2,"S_BUFFER_ATOMIC_UMAX_X2"}, // 103
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_AND_X2,"S_BUFFER_ATOMIC_AND_X2"}, // 104
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_OR_X2,"S_BUFFER_ATOMIC_OR_X2"}, // 105
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_XOR_X2,"S_BUFFER_ATOMIC_XOR_X2"}, // 106
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_INC_X2,"S_BUFFER_ATOMIC_INC_X2"}, // 107
                {amdgpu_gfx940_op_S_BUFFER_ATOMIC_DEC_X2,"S_BUFFER_ATOMIC_DEC_X2"}, // 108
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 109
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 110
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 111
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 112
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 113
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 114
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 115
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 116
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 117
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 118
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 119
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 120
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 121
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 122
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 123
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 124
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 125
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 126
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 127
                {amdgpu_gfx940_op_S_ATOMIC_SWAP,"S_ATOMIC_SWAP"}, // 128
                {amdgpu_gfx940_op_S_ATOMIC_CMPSWAP,"S_ATOMIC_CMPSWAP"}, // 129
                {amdgpu_gfx940_op_S_ATOMIC_ADD,"S_ATOMIC_ADD"}, // 130
                {amdgpu_gfx940_op_S_ATOMIC_SUB,"S_ATOMIC_SUB"}, // 131
                {amdgpu_gfx940_op_S_ATOMIC_SMIN,"S_ATOMIC_SMIN"}, // 132
                {amdgpu_gfx940_op_S_ATOMIC_UMIN,"S_ATOMIC_UMIN"}, // 133
                {amdgpu_gfx940_op_S_ATOMIC_SMAX,"S_ATOMIC_SMAX"}, // 134
                {amdgpu_gfx940_op_S_ATOMIC_UMAX,"S_ATOMIC_UMAX"}, // 135
                {amdgpu_gfx940_op_S_ATOMIC_AND,"S_ATOMIC_AND"}, // 136
                {amdgpu_gfx940_op_S_ATOMIC_OR,"S_ATOMIC_OR"}, // 137
                {amdgpu_gfx940_op_S_ATOMIC_XOR,"S_ATOMIC_XOR"}, // 138
                {amdgpu_gfx940_op_S_ATOMIC_INC,"S_ATOMIC_INC"}, // 139
                {amdgpu_gfx940_op_S_ATOMIC_DEC,"S_ATOMIC_DEC"}, // 140
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 141
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 142
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 143
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 144
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 145
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 146
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 147
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 148
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 149
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 150
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 151
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 152
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 153
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 154
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 155
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 156
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 157
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 158
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 159
                {amdgpu_gfx940_op_S_ATOMIC_SWAP_X2,"S_ATOMIC_SWAP_X2"}, // 160
                {amdgpu_gfx940_op_S_ATOMIC_CMPSWAP_X2,"S_ATOMIC_CMPSWAP_X2"}, // 161
                {amdgpu_gfx940_op_S_ATOMIC_ADD_X2,"S_ATOMIC_ADD_X2"}, // 162
                {amdgpu_gfx940_op_S_ATOMIC_SUB_X2,"S_ATOMIC_SUB_X2"}, // 163
                {amdgpu_gfx940_op_S_ATOMIC_SMIN_X2,"S_ATOMIC_SMIN_X2"}, // 164
                {amdgpu_gfx940_op_S_ATOMIC_UMIN_X2,"S_ATOMIC_UMIN_X2"}, // 165
                {amdgpu_gfx940_op_S_ATOMIC_SMAX_X2,"S_ATOMIC_SMAX_X2"}, // 166
                {amdgpu_gfx940_op_S_ATOMIC_UMAX_X2,"S_ATOMIC_UMAX_X2"}, // 167
                {amdgpu_gfx940_op_S_ATOMIC_AND_X2,"S_ATOMIC_AND_X2"}, // 168
                {amdgpu_gfx940_op_S_ATOMIC_OR_X2,"S_ATOMIC_OR_X2"}, // 169
                {amdgpu_gfx940_op_S_ATOMIC_XOR_X2,"S_ATOMIC_XOR_X2"}, // 170
                {amdgpu_gfx940_op_S_ATOMIC_INC_X2,"S_ATOMIC_INC_X2"}, // 171
                {amdgpu_gfx940_op_S_ATOMIC_DEC_X2,"S_ATOMIC_DEC_X2"}, // 172
            }; // end ENC_SMEM_insn_table
            const amdgpu_gfx940_insn_entry ENC_SOP1_insn_table[56] = 
            {
                {amdgpu_gfx940_op_S_MOV_B32,"S_MOV_B32"}, // 0
                {amdgpu_gfx940_op_S_MOV_B64,"S_MOV_B64"}, // 1
                {amdgpu_gfx940_op_S_CMOV_B32,"S_CMOV_B32"}, // 2
                {amdgpu_gfx940_op_S_CMOV_B64,"S_CMOV_B64"}, // 3
                {amdgpu_gfx940_op_S_NOT_B32,"S_NOT_B32"}, // 4
                {amdgpu_gfx940_op_S_NOT_B64,"S_NOT_B64"}, // 5
                {amdgpu_gfx940_op_S_WQM_B32,"S_WQM_B32"}, // 6
                {amdgpu_gfx940_op_S_WQM_B64,"S_WQM_B64"}, // 7
                {amdgpu_gfx940_op_S_BREV_B32,"S_BREV_B32"}, // 8
                {amdgpu_gfx940_op_S_BREV_B64,"S_BREV_B64"}, // 9
                {amdgpu_gfx940_op_S_BCNT0_I32_B32,"S_BCNT0_I32_B32"}, // 10
                {amdgpu_gfx940_op_S_BCNT0_I32_B64,"S_BCNT0_I32_B64"}, // 11
                {amdgpu_gfx940_op_S_BCNT1_I32_B32,"S_BCNT1_I32_B32"}, // 12
                {amdgpu_gfx940_op_S_BCNT1_I32_B64,"S_BCNT1_I32_B64"}, // 13
                {amdgpu_gfx940_op_S_FF0_I32_B32,"S_FF0_I32_B32"}, // 14
                {amdgpu_gfx940_op_S_FF0_I32_B64,"S_FF0_I32_B64"}, // 15
                {amdgpu_gfx940_op_S_FF1_I32_B32,"S_FF1_I32_B32"}, // 16
                {amdgpu_gfx940_op_S_FF1_I32_B64,"S_FF1_I32_B64"}, // 17
                {amdgpu_gfx940_op_S_FLBIT_I32_B32,"S_FLBIT_I32_B32"}, // 18
                {amdgpu_gfx940_op_S_FLBIT_I32_B64,"S_FLBIT_I32_B64"}, // 19
                {amdgpu_gfx940_op_S_FLBIT_I32,"S_FLBIT_I32"}, // 20
                {amdgpu_gfx940_op_S_FLBIT_I32_I64,"S_FLBIT_I32_I64"}, // 21
                {amdgpu_gfx940_op_S_SEXT_I32_I8,"S_SEXT_I32_I8"}, // 22
                {amdgpu_gfx940_op_S_SEXT_I32_I16,"S_SEXT_I32_I16"}, // 23
                {amdgpu_gfx940_op_S_BITSET0_B32,"S_BITSET0_B32"}, // 24
                {amdgpu_gfx940_op_S_BITSET0_B64,"S_BITSET0_B64"}, // 25
                {amdgpu_gfx940_op_S_BITSET1_B32,"S_BITSET1_B32"}, // 26
                {amdgpu_gfx940_op_S_BITSET1_B64,"S_BITSET1_B64"}, // 27
                {amdgpu_gfx940_op_S_GETPC_B64,"S_GETPC_B64"}, // 28
                {amdgpu_gfx940_op_S_SETPC_B64,"S_SETPC_B64"}, // 29
                {amdgpu_gfx940_op_S_SWAPPC_B64,"S_SWAPPC_B64"}, // 30
                {amdgpu_gfx940_op_S_RFE_B64,"S_RFE_B64"}, // 31
                {amdgpu_gfx940_op_S_AND_SAVEEXEC_B64,"S_AND_SAVEEXEC_B64"}, // 32
                {amdgpu_gfx940_op_S_OR_SAVEEXEC_B64,"S_OR_SAVEEXEC_B64"}, // 33
                {amdgpu_gfx940_op_S_XOR_SAVEEXEC_B64,"S_XOR_SAVEEXEC_B64"}, // 34
                {amdgpu_gfx940_op_S_ANDN2_SAVEEXEC_B64,"S_ANDN2_SAVEEXEC_B64"}, // 35
                {amdgpu_gfx940_op_S_ORN2_SAVEEXEC_B64,"S_ORN2_SAVEEXEC_B64"}, // 36
                {amdgpu_gfx940_op_S_NAND_SAVEEXEC_B64,"S_NAND_SAVEEXEC_B64"}, // 37
                {amdgpu_gfx940_op_S_NOR_SAVEEXEC_B64,"S_NOR_SAVEEXEC_B64"}, // 38
                {amdgpu_gfx940_op_S_XNOR_SAVEEXEC_B64,"S_XNOR_SAVEEXEC_B64"}, // 39
                {amdgpu_gfx940_op_S_QUADMASK_B32,"S_QUADMASK_B32"}, // 40
                {amdgpu_gfx940_op_S_QUADMASK_B64,"S_QUADMASK_B64"}, // 41
                {amdgpu_gfx940_op_S_MOVRELS_B32,"S_MOVRELS_B32"}, // 42
                {amdgpu_gfx940_op_S_MOVRELS_B64,"S_MOVRELS_B64"}, // 43
                {amdgpu_gfx940_op_S_MOVRELD_B32,"S_MOVRELD_B32"}, // 44
                {amdgpu_gfx940_op_S_MOVRELD_B64,"S_MOVRELD_B64"}, // 45
                {amdgpu_gfx940_op_S_CBRANCH_JOIN,"S_CBRANCH_JOIN"}, // 46
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 47
                {amdgpu_gfx940_op_S_ABS_I32,"S_ABS_I32"}, // 48
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 49
                {amdgpu_gfx940_op_S_SET_GPR_IDX_IDX,"S_SET_GPR_IDX_IDX"}, // 50
                {amdgpu_gfx940_op_S_ANDN1_SAVEEXEC_B64,"S_ANDN1_SAVEEXEC_B64"}, // 51
                {amdgpu_gfx940_op_S_ORN1_SAVEEXEC_B64,"S_ORN1_SAVEEXEC_B64"}, // 52
                {amdgpu_gfx940_op_S_ANDN1_WREXEC_B64,"S_ANDN1_WREXEC_B64"}, // 53
                {amdgpu_gfx940_op_S_ANDN2_WREXEC_B64,"S_ANDN2_WREXEC_B64"}, // 54
                {amdgpu_gfx940_op_S_BITREPLICATE_B64_B32,"S_BITREPLICATE_B64_B32"}, // 55
            }; // end ENC_SOP1_insn_table
            const amdgpu_gfx940_insn_entry ENC_SOP2_insn_table[53] = 
            {
                {amdgpu_gfx940_op_S_ADD_U32,"S_ADD_U32"}, // 0
                {amdgpu_gfx940_op_S_SUB_U32,"S_SUB_U32"}, // 1
                {amdgpu_gfx940_op_S_ADD_I32,"S_ADD_I32"}, // 2
                {amdgpu_gfx940_op_S_SUB_I32,"S_SUB_I32"}, // 3
                {amdgpu_gfx940_op_S_ADDC_U32,"S_ADDC_U32"}, // 4
                {amdgpu_gfx940_op_S_SUBB_U32,"S_SUBB_U32"}, // 5
                {amdgpu_gfx940_op_S_MIN_I32,"S_MIN_I32"}, // 6
                {amdgpu_gfx940_op_S_MIN_U32,"S_MIN_U32"}, // 7
                {amdgpu_gfx940_op_S_MAX_I32,"S_MAX_I32"}, // 8
                {amdgpu_gfx940_op_S_MAX_U32,"S_MAX_U32"}, // 9
                {amdgpu_gfx940_op_S_CSELECT_B32,"S_CSELECT_B32"}, // 10
                {amdgpu_gfx940_op_S_CSELECT_B64,"S_CSELECT_B64"}, // 11
                {amdgpu_gfx940_op_S_AND_B32,"S_AND_B32"}, // 12
                {amdgpu_gfx940_op_S_AND_B64,"S_AND_B64"}, // 13
                {amdgpu_gfx940_op_S_OR_B32,"S_OR_B32"}, // 14
                {amdgpu_gfx940_op_S_OR_B64,"S_OR_B64"}, // 15
                {amdgpu_gfx940_op_S_XOR_B32,"S_XOR_B32"}, // 16
                {amdgpu_gfx940_op_S_XOR_B64,"S_XOR_B64"}, // 17
                {amdgpu_gfx940_op_S_ANDN2_B32,"S_ANDN2_B32"}, // 18
                {amdgpu_gfx940_op_S_ANDN2_B64,"S_ANDN2_B64"}, // 19
                {amdgpu_gfx940_op_S_ORN2_B32,"S_ORN2_B32"}, // 20
                {amdgpu_gfx940_op_S_ORN2_B64,"S_ORN2_B64"}, // 21
                {amdgpu_gfx940_op_S_NAND_B32,"S_NAND_B32"}, // 22
                {amdgpu_gfx940_op_S_NAND_B64,"S_NAND_B64"}, // 23
                {amdgpu_gfx940_op_S_NOR_B32,"S_NOR_B32"}, // 24
                {amdgpu_gfx940_op_S_NOR_B64,"S_NOR_B64"}, // 25
                {amdgpu_gfx940_op_S_XNOR_B32,"S_XNOR_B32"}, // 26
                {amdgpu_gfx940_op_S_XNOR_B64,"S_XNOR_B64"}, // 27
                {amdgpu_gfx940_op_S_LSHL_B32,"S_LSHL_B32"}, // 28
                {amdgpu_gfx940_op_S_LSHL_B64,"S_LSHL_B64"}, // 29
                {amdgpu_gfx940_op_S_LSHR_B32,"S_LSHR_B32"}, // 30
                {amdgpu_gfx940_op_S_LSHR_B64,"S_LSHR_B64"}, // 31
                {amdgpu_gfx940_op_S_ASHR_I32,"S_ASHR_I32"}, // 32
                {amdgpu_gfx940_op_S_ASHR_I64,"S_ASHR_I64"}, // 33
                {amdgpu_gfx940_op_S_BFM_B32,"S_BFM_B32"}, // 34
                {amdgpu_gfx940_op_S_BFM_B64,"S_BFM_B64"}, // 35
                {amdgpu_gfx940_op_S_MUL_I32,"S_MUL_I32"}, // 36
                {amdgpu_gfx940_op_S_BFE_U32,"S_BFE_U32"}, // 37
                {amdgpu_gfx940_op_S_BFE_I32,"S_BFE_I32"}, // 38
                {amdgpu_gfx940_op_S_BFE_U64,"S_BFE_U64"}, // 39
                {amdgpu_gfx940_op_S_BFE_I64,"S_BFE_I64"}, // 40
                {amdgpu_gfx940_op_S_CBRANCH_G_FORK,"S_CBRANCH_G_FORK"}, // 41
                {amdgpu_gfx940_op_S_ABSDIFF_I32,"S_ABSDIFF_I32"}, // 42
                {amdgpu_gfx940_op_S_RFE_RESTORE_B64,"S_RFE_RESTORE_B64"}, // 43
                {amdgpu_gfx940_op_S_MUL_HI_U32,"S_MUL_HI_U32"}, // 44
                {amdgpu_gfx940_op_S_MUL_HI_I32,"S_MUL_HI_I32"}, // 45
                {amdgpu_gfx940_op_S_LSHL1_ADD_U32,"S_LSHL1_ADD_U32"}, // 46
                {amdgpu_gfx940_op_S_LSHL2_ADD_U32,"S_LSHL2_ADD_U32"}, // 47
                {amdgpu_gfx940_op_S_LSHL3_ADD_U32,"S_LSHL3_ADD_U32"}, // 48
                {amdgpu_gfx940_op_S_LSHL4_ADD_U32,"S_LSHL4_ADD_U32"}, // 49
                {amdgpu_gfx940_op_S_PACK_LL_B32_B16,"S_PACK_LL_B32_B16"}, // 50
                {amdgpu_gfx940_op_S_PACK_LH_B32_B16,"S_PACK_LH_B32_B16"}, // 51
                {amdgpu_gfx940_op_S_PACK_HH_B32_B16,"S_PACK_HH_B32_B16"}, // 52
            }; // end ENC_SOP2_insn_table
            const amdgpu_gfx940_insn_entry ENC_SOPC_insn_table[20] = 
            {
                {amdgpu_gfx940_op_S_CMP_EQ_I32,"S_CMP_EQ_I32"}, // 0
                {amdgpu_gfx940_op_S_CMP_LG_I32,"S_CMP_LG_I32"}, // 1
                {amdgpu_gfx940_op_S_CMP_GT_I32,"S_CMP_GT_I32"}, // 2
                {amdgpu_gfx940_op_S_CMP_GE_I32,"S_CMP_GE_I32"}, // 3
                {amdgpu_gfx940_op_S_CMP_LT_I32,"S_CMP_LT_I32"}, // 4
                {amdgpu_gfx940_op_S_CMP_LE_I32,"S_CMP_LE_I32"}, // 5
                {amdgpu_gfx940_op_S_CMP_EQ_U32,"S_CMP_EQ_U32"}, // 6
                {amdgpu_gfx940_op_S_CMP_LG_U32,"S_CMP_LG_U32"}, // 7
                {amdgpu_gfx940_op_S_CMP_GT_U32,"S_CMP_GT_U32"}, // 8
                {amdgpu_gfx940_op_S_CMP_GE_U32,"S_CMP_GE_U32"}, // 9
                {amdgpu_gfx940_op_S_CMP_LT_U32,"S_CMP_LT_U32"}, // 10
                {amdgpu_gfx940_op_S_CMP_LE_U32,"S_CMP_LE_U32"}, // 11
                {amdgpu_gfx940_op_S_BITCMP0_B32,"S_BITCMP0_B32"}, // 12
                {amdgpu_gfx940_op_S_BITCMP1_B32,"S_BITCMP1_B32"}, // 13
                {amdgpu_gfx940_op_S_BITCMP0_B64,"S_BITCMP0_B64"}, // 14
                {amdgpu_gfx940_op_S_BITCMP1_B64,"S_BITCMP1_B64"}, // 15
                {amdgpu_gfx940_op_S_SETVSKIP,"S_SETVSKIP"}, // 16
                {amdgpu_gfx940_op_S_SET_GPR_IDX_ON,"S_SET_GPR_IDX_ON"}, // 17
                {amdgpu_gfx940_op_S_CMP_EQ_U64,"S_CMP_EQ_U64"}, // 18
                {amdgpu_gfx940_op_S_CMP_LG_U64,"S_CMP_LG_U64"}, // 19
            }; // end ENC_SOPC_insn_table
            const amdgpu_gfx940_insn_entry ENC_SOPK_insn_table[22] = 
            {
                {amdgpu_gfx940_op_S_MOVK_I32,"S_MOVK_I32"}, // 0
                {amdgpu_gfx940_op_S_CMOVK_I32,"S_CMOVK_I32"}, // 1
                {amdgpu_gfx940_op_S_CMPK_EQ_I32,"S_CMPK_EQ_I32"}, // 2
                {amdgpu_gfx940_op_S_CMPK_LG_I32,"S_CMPK_LG_I32"}, // 3
                {amdgpu_gfx940_op_S_CMPK_GT_I32,"S_CMPK_GT_I32"}, // 4
                {amdgpu_gfx940_op_S_CMPK_GE_I32,"S_CMPK_GE_I32"}, // 5
                {amdgpu_gfx940_op_S_CMPK_LT_I32,"S_CMPK_LT_I32"}, // 6
                {amdgpu_gfx940_op_S_CMPK_LE_I32,"S_CMPK_LE_I32"}, // 7
                {amdgpu_gfx940_op_S_CMPK_EQ_U32,"S_CMPK_EQ_U32"}, // 8
                {amdgpu_gfx940_op_S_CMPK_LG_U32,"S_CMPK_LG_U32"}, // 9
                {amdgpu_gfx940_op_S_CMPK_GT_U32,"S_CMPK_GT_U32"}, // 10
                {amdgpu_gfx940_op_S_CMPK_GE_U32,"S_CMPK_GE_U32"}, // 11
                {amdgpu_gfx940_op_S_CMPK_LT_U32,"S_CMPK_LT_U32"}, // 12
                {amdgpu_gfx940_op_S_CMPK_LE_U32,"S_CMPK_LE_U32"}, // 13
                {amdgpu_gfx940_op_S_ADDK_I32,"S_ADDK_I32"}, // 14
                {amdgpu_gfx940_op_S_MULK_I32,"S_MULK_I32"}, // 15
                {amdgpu_gfx940_op_S_CBRANCH_I_FORK,"S_CBRANCH_I_FORK"}, // 16
                {amdgpu_gfx940_op_S_GETREG_B32,"S_GETREG_B32"}, // 17
                {amdgpu_gfx940_op_S_SETREG_B32,"S_SETREG_B32"}, // 18
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 19
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 20
                {amdgpu_gfx940_op_S_CALL_B64,"S_CALL_B64"}, // 21
            }; // end ENC_SOPK_insn_table
            const amdgpu_gfx940_insn_entry ENC_SOPP_insn_table[32] = 
            {
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 0
                {amdgpu_gfx940_op_S_ENDPGM,"S_ENDPGM"}, // 1
                {amdgpu_gfx940_op_S_BRANCH,"S_BRANCH"}, // 2
                {amdgpu_gfx940_op_S_WAKEUP,"S_WAKEUP"}, // 3
                {amdgpu_gfx940_op_S_CBRANCH_SCC0,"S_CBRANCH_SCC0"}, // 4
                {amdgpu_gfx940_op_S_CBRANCH_SCC1,"S_CBRANCH_SCC1"}, // 5
                {amdgpu_gfx940_op_S_CBRANCH_VCCZ,"S_CBRANCH_VCCZ"}, // 6
                {amdgpu_gfx940_op_S_CBRANCH_VCCNZ,"S_CBRANCH_VCCNZ"}, // 7
                {amdgpu_gfx940_op_S_CBRANCH_EXECZ,"S_CBRANCH_EXECZ"}, // 8
                {amdgpu_gfx940_op_S_CBRANCH_EXECNZ,"S_CBRANCH_EXECNZ"}, // 9
                {amdgpu_gfx940_op_S_BARRIER,"S_BARRIER"}, // 10
                {amdgpu_gfx940_op_S_SETKILL,"S_SETKILL"}, // 11
                {amdgpu_gfx940_op_S_WAITCNT,"S_WAITCNT"}, // 12
                {amdgpu_gfx940_op_S_SETHALT,"S_SETHALT"}, // 13
                {amdgpu_gfx940_op_S_SLEEP,"S_SLEEP"}, // 14
                {amdgpu_gfx940_op_S_SETPRIO,"S_SETPRIO"}, // 15
                {amdgpu_gfx940_op_S_SENDMSG,"S_SENDMSG"}, // 16
                {amdgpu_gfx940_op_S_SENDMSGHALT,"S_SENDMSGHALT"}, // 17
                {amdgpu_gfx940_op_S_TRAP,"S_TRAP"}, // 18
                {amdgpu_gfx940_op_S_ICACHE_INV,"S_ICACHE_INV"}, // 19
                {amdgpu_gfx940_op_S_INCPERFLEVEL,"S_INCPERFLEVEL"}, // 20
                {amdgpu_gfx940_op_S_DECPERFLEVEL,"S_DECPERFLEVEL"}, // 21
                {amdgpu_gfx940_op_S_TTRACEDATA,"S_TTRACEDATA"}, // 22
                {amdgpu_gfx940_op_S_CBRANCH_CDBGSYS,"S_CBRANCH_CDBGSYS"}, // 23
                {amdgpu_gfx940_op_S_CBRANCH_CDBGUSER,"S_CBRANCH_CDBGUSER"}, // 24
                {amdgpu_gfx940_op_S_CBRANCH_CDBGSYS_OR_USER,"S_CBRANCH_CDBGSYS_OR_USER"}, // 25
                {amdgpu_gfx940_op_S_CBRANCH_CDBGSYS_AND_USER,"S_CBRANCH_CDBGSYS_AND_USER"}, // 26
                {amdgpu_gfx940_op_S_ENDPGM_SAVED,"S_ENDPGM_SAVED"}, // 27
                {amdgpu_gfx940_op_S_SET_GPR_IDX_OFF,"S_SET_GPR_IDX_OFF"}, // 28
                {amdgpu_gfx940_op_S_SET_GPR_IDX_MODE,"S_SET_GPR_IDX_MODE"}, // 29
                {amdgpu_gfx940_op_S_ENDPGM_ORDERED_PS_DONE,"S_ENDPGM_ORDERED_PS_DONE"}, // 30
                {amdgpu_gfx940_op_S_SET_VALU_COEXEC_MODE,"S_SET_VALU_COEXEC_MODE"}, // 31
            }; // end ENC_SOPP_insn_table
            const amdgpu_gfx940_insn_entry ENC_VOP1_insn_table[88] = 
            {
                {amdgpu_gfx940_op_V_NOP,"V_NOP"}, // 0
                {amdgpu_gfx940_op_V_MOV_B32,"V_MOV_B32"}, // 1
                {amdgpu_gfx940_op_V_READFIRSTLANE_B32,"V_READFIRSTLANE_B32"}, // 2
                {amdgpu_gfx940_op_V_CVT_I32_F64,"V_CVT_I32_F64"}, // 3
                {amdgpu_gfx940_op_V_CVT_F64_I32,"V_CVT_F64_I32"}, // 4
                {amdgpu_gfx940_op_V_CVT_F32_I32,"V_CVT_F32_I32"}, // 5
                {amdgpu_gfx940_op_V_CVT_F32_U32,"V_CVT_F32_U32"}, // 6
                {amdgpu_gfx940_op_V_CVT_U32_F32,"V_CVT_U32_F32"}, // 7
                {amdgpu_gfx940_op_V_CVT_I32_F32,"V_CVT_I32_F32"}, // 8
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 9
                {amdgpu_gfx940_op_V_CVT_F16_F32,"V_CVT_F16_F32"}, // 10
                {amdgpu_gfx940_op_V_CVT_F32_F16,"V_CVT_F32_F16"}, // 11
                {amdgpu_gfx940_op_V_CVT_RPI_I32_F32,"V_CVT_RPI_I32_F32"}, // 12
                {amdgpu_gfx940_op_V_CVT_FLR_I32_F32,"V_CVT_FLR_I32_F32"}, // 13
                {amdgpu_gfx940_op_V_CVT_OFF_F32_I4,"V_CVT_OFF_F32_I4"}, // 14
                {amdgpu_gfx940_op_V_CVT_F32_F64,"V_CVT_F32_F64"}, // 15
                {amdgpu_gfx940_op_V_CVT_F64_F32,"V_CVT_F64_F32"}, // 16
                {amdgpu_gfx940_op_V_CVT_F32_UBYTE0,"V_CVT_F32_UBYTE0"}, // 17
                {amdgpu_gfx940_op_V_CVT_F32_UBYTE1,"V_CVT_F32_UBYTE1"}, // 18
                {amdgpu_gfx940_op_V_CVT_F32_UBYTE2,"V_CVT_F32_UBYTE2"}, // 19
                {amdgpu_gfx940_op_V_CVT_F32_UBYTE3,"V_CVT_F32_UBYTE3"}, // 20
                {amdgpu_gfx940_op_V_CVT_U32_F64,"V_CVT_U32_F64"}, // 21
                {amdgpu_gfx940_op_V_CVT_F64_U32,"V_CVT_F64_U32"}, // 22
                {amdgpu_gfx940_op_V_TRUNC_F64,"V_TRUNC_F64"}, // 23
                {amdgpu_gfx940_op_V_CEIL_F64,"V_CEIL_F64"}, // 24
                {amdgpu_gfx940_op_V_RNDNE_F64,"V_RNDNE_F64"}, // 25
                {amdgpu_gfx940_op_V_FLOOR_F64,"V_FLOOR_F64"}, // 26
                {amdgpu_gfx940_op_V_FRACT_F32,"V_FRACT_F32"}, // 27
                {amdgpu_gfx940_op_V_TRUNC_F32,"V_TRUNC_F32"}, // 28
                {amdgpu_gfx940_op_V_CEIL_F32,"V_CEIL_F32"}, // 29
                {amdgpu_gfx940_op_V_RNDNE_F32,"V_RNDNE_F32"}, // 30
                {amdgpu_gfx940_op_V_FLOOR_F32,"V_FLOOR_F32"}, // 31
                {amdgpu_gfx940_op_V_EXP_F32,"V_EXP_F32"}, // 32
                {amdgpu_gfx940_op_V_LOG_F32,"V_LOG_F32"}, // 33
                {amdgpu_gfx940_op_V_RCP_F32,"V_RCP_F32"}, // 34
                {amdgpu_gfx940_op_V_RCP_IFLAG_F32,"V_RCP_IFLAG_F32"}, // 35
                {amdgpu_gfx940_op_V_RSQ_F32,"V_RSQ_F32"}, // 36
                {amdgpu_gfx940_op_V_RCP_F64,"V_RCP_F64"}, // 37
                {amdgpu_gfx940_op_V_RSQ_F64,"V_RSQ_F64"}, // 38
                {amdgpu_gfx940_op_V_SQRT_F32,"V_SQRT_F32"}, // 39
                {amdgpu_gfx940_op_V_SQRT_F64,"V_SQRT_F64"}, // 40
                {amdgpu_gfx940_op_V_SIN_F32,"V_SIN_F32"}, // 41
                {amdgpu_gfx940_op_V_COS_F32,"V_COS_F32"}, // 42
                {amdgpu_gfx940_op_V_NOT_B32,"V_NOT_B32"}, // 43
                {amdgpu_gfx940_op_V_BFREV_B32,"V_BFREV_B32"}, // 44
                {amdgpu_gfx940_op_V_FFBH_U32,"V_FFBH_U32"}, // 45
                {amdgpu_gfx940_op_V_FFBL_B32,"V_FFBL_B32"}, // 46
                {amdgpu_gfx940_op_V_FFBH_I32,"V_FFBH_I32"}, // 47
                {amdgpu_gfx940_op_V_FREXP_EXP_I32_F64,"V_FREXP_EXP_I32_F64"}, // 48
                {amdgpu_gfx940_op_V_FREXP_MANT_F64,"V_FREXP_MANT_F64"}, // 49
                {amdgpu_gfx940_op_V_FRACT_F64,"V_FRACT_F64"}, // 50
                {amdgpu_gfx940_op_V_FREXP_EXP_I32_F32,"V_FREXP_EXP_I32_F32"}, // 51
                {amdgpu_gfx940_op_V_FREXP_MANT_F32,"V_FREXP_MANT_F32"}, // 52
                {amdgpu_gfx940_op_V_CLREXCP,"V_CLREXCP"}, // 53
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 54
                {amdgpu_gfx940_op_V_SCREEN_PARTITION_4SE_B32,"V_SCREEN_PARTITION_4SE_B32"}, // 55
                {amdgpu_gfx940_op_V_MOV_B64,"V_MOV_B64"}, // 56
                {amdgpu_gfx940_op_V_CVT_F16_U16,"V_CVT_F16_U16"}, // 57
                {amdgpu_gfx940_op_V_CVT_F16_I16,"V_CVT_F16_I16"}, // 58
                {amdgpu_gfx940_op_V_CVT_U16_F16,"V_CVT_U16_F16"}, // 59
                {amdgpu_gfx940_op_V_CVT_I16_F16,"V_CVT_I16_F16"}, // 60
                {amdgpu_gfx940_op_V_RCP_F16,"V_RCP_F16"}, // 61
                {amdgpu_gfx940_op_V_SQRT_F16,"V_SQRT_F16"}, // 62
                {amdgpu_gfx940_op_V_RSQ_F16,"V_RSQ_F16"}, // 63
                {amdgpu_gfx940_op_V_LOG_F16,"V_LOG_F16"}, // 64
                {amdgpu_gfx940_op_V_EXP_F16,"V_EXP_F16"}, // 65
                {amdgpu_gfx940_op_V_FREXP_MANT_F16,"V_FREXP_MANT_F16"}, // 66
                {amdgpu_gfx940_op_V_FREXP_EXP_I16_F16,"V_FREXP_EXP_I16_F16"}, // 67
                {amdgpu_gfx940_op_V_FLOOR_F16,"V_FLOOR_F16"}, // 68
                {amdgpu_gfx940_op_V_CEIL_F16,"V_CEIL_F16"}, // 69
                {amdgpu_gfx940_op_V_TRUNC_F16,"V_TRUNC_F16"}, // 70
                {amdgpu_gfx940_op_V_RNDNE_F16,"V_RNDNE_F16"}, // 71
                {amdgpu_gfx940_op_V_FRACT_F16,"V_FRACT_F16"}, // 72
                {amdgpu_gfx940_op_V_SIN_F16,"V_SIN_F16"}, // 73
                {amdgpu_gfx940_op_V_COS_F16,"V_COS_F16"}, // 74
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 75
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 76
                {amdgpu_gfx940_op_V_CVT_NORM_I16_F16,"V_CVT_NORM_I16_F16"}, // 77
                {amdgpu_gfx940_op_V_CVT_NORM_U16_F16,"V_CVT_NORM_U16_F16"}, // 78
                {amdgpu_gfx940_op_V_SAT_PK_U8_I16,"V_SAT_PK_U8_I16"}, // 79
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 80
                {amdgpu_gfx940_op_V_SWAP_B32,"V_SWAP_B32"}, // 81
                {amdgpu_gfx940_op_V_ACCVGPR_MOV_B32,"V_ACCVGPR_MOV_B32"}, // 82
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 83
                {amdgpu_gfx940_op_V_CVT_F32_FP8,"V_CVT_F32_FP8"}, // 84
                {amdgpu_gfx940_op_V_CVT_F32_BF8,"V_CVT_F32_BF8"}, // 85
                {amdgpu_gfx940_op_V_CVT_PK_F32_FP8,"V_CVT_PK_F32_FP8"}, // 86
                {amdgpu_gfx940_op_V_CVT_PK_F32_BF8,"V_CVT_PK_F32_BF8"}, // 87
            }; // end ENC_VOP1_insn_table
            const amdgpu_gfx940_insn_entry ENC_VOP3_insn_table[678] = 
            {
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 0
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 1
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 2
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 3
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 4
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 5
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 6
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 7
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 8
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 9
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 10
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 11
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 12
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 13
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 14
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 15
                {amdgpu_gfx940_op_V_CMP_CLASS_F32,"V_CMP_CLASS_F32"}, // 16
                {amdgpu_gfx940_op_V_CMPX_CLASS_F32,"V_CMPX_CLASS_F32"}, // 17
                {amdgpu_gfx940_op_V_CMP_CLASS_F64,"V_CMP_CLASS_F64"}, // 18
                {amdgpu_gfx940_op_V_CMPX_CLASS_F64,"V_CMPX_CLASS_F64"}, // 19
                {amdgpu_gfx940_op_V_CMP_CLASS_F16,"V_CMP_CLASS_F16"}, // 20
                {amdgpu_gfx940_op_V_CMPX_CLASS_F16,"V_CMPX_CLASS_F16"}, // 21
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 22
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 23
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 24
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 25
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 26
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 27
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 28
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 29
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 30
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 31
                {amdgpu_gfx940_op_V_CMP_F_F16,"V_CMP_F_F16"}, // 32
                {amdgpu_gfx940_op_V_CMP_LT_F16,"V_CMP_LT_F16"}, // 33
                {amdgpu_gfx940_op_V_CMP_EQ_F16,"V_CMP_EQ_F16"}, // 34
                {amdgpu_gfx940_op_V_CMP_LE_F16,"V_CMP_LE_F16"}, // 35
                {amdgpu_gfx940_op_V_CMP_GT_F16,"V_CMP_GT_F16"}, // 36
                {amdgpu_gfx940_op_V_CMP_LG_F16,"V_CMP_LG_F16"}, // 37
                {amdgpu_gfx940_op_V_CMP_GE_F16,"V_CMP_GE_F16"}, // 38
                {amdgpu_gfx940_op_V_CMP_O_F16,"V_CMP_O_F16"}, // 39
                {amdgpu_gfx940_op_V_CMP_U_F16,"V_CMP_U_F16"}, // 40
                {amdgpu_gfx940_op_V_CMP_NGE_F16,"V_CMP_NGE_F16"}, // 41
                {amdgpu_gfx940_op_V_CMP_NLG_F16,"V_CMP_NLG_F16"}, // 42
                {amdgpu_gfx940_op_V_CMP_NGT_F16,"V_CMP_NGT_F16"}, // 43
                {amdgpu_gfx940_op_V_CMP_NLE_F16,"V_CMP_NLE_F16"}, // 44
                {amdgpu_gfx940_op_V_CMP_NEQ_F16,"V_CMP_NEQ_F16"}, // 45
                {amdgpu_gfx940_op_V_CMP_NLT_F16,"V_CMP_NLT_F16"}, // 46
                {amdgpu_gfx940_op_V_CMP_TRU_F16,"V_CMP_TRU_F16"}, // 47
                {amdgpu_gfx940_op_V_CMPX_F_F16,"V_CMPX_F_F16"}, // 48
                {amdgpu_gfx940_op_V_CMPX_LT_F16,"V_CMPX_LT_F16"}, // 49
                {amdgpu_gfx940_op_V_CMPX_EQ_F16,"V_CMPX_EQ_F16"}, // 50
                {amdgpu_gfx940_op_V_CMPX_LE_F16,"V_CMPX_LE_F16"}, // 51
                {amdgpu_gfx940_op_V_CMPX_GT_F16,"V_CMPX_GT_F16"}, // 52
                {amdgpu_gfx940_op_V_CMPX_LG_F16,"V_CMPX_LG_F16"}, // 53
                {amdgpu_gfx940_op_V_CMPX_GE_F16,"V_CMPX_GE_F16"}, // 54
                {amdgpu_gfx940_op_V_CMPX_O_F16,"V_CMPX_O_F16"}, // 55
                {amdgpu_gfx940_op_V_CMPX_U_F16,"V_CMPX_U_F16"}, // 56
                {amdgpu_gfx940_op_V_CMPX_NGE_F16,"V_CMPX_NGE_F16"}, // 57
                {amdgpu_gfx940_op_V_CMPX_NLG_F16,"V_CMPX_NLG_F16"}, // 58
                {amdgpu_gfx940_op_V_CMPX_NGT_F16,"V_CMPX_NGT_F16"}, // 59
                {amdgpu_gfx940_op_V_CMPX_NLE_F16,"V_CMPX_NLE_F16"}, // 60
                {amdgpu_gfx940_op_V_CMPX_NEQ_F16,"V_CMPX_NEQ_F16"}, // 61
                {amdgpu_gfx940_op_V_CMPX_NLT_F16,"V_CMPX_NLT_F16"}, // 62
                {amdgpu_gfx940_op_V_CMPX_TRU_F16,"V_CMPX_TRU_F16"}, // 63
                {amdgpu_gfx940_op_V_CMP_F_F32,"V_CMP_F_F32"}, // 64
                {amdgpu_gfx940_op_V_CMP_LT_F32,"V_CMP_LT_F32"}, // 65
                {amdgpu_gfx940_op_V_CMP_EQ_F32,"V_CMP_EQ_F32"}, // 66
                {amdgpu_gfx940_op_V_CMP_LE_F32,"V_CMP_LE_F32"}, // 67
                {amdgpu_gfx940_op_V_CMP_GT_F32,"V_CMP_GT_F32"}, // 68
                {amdgpu_gfx940_op_V_CMP_LG_F32,"V_CMP_LG_F32"}, // 69
                {amdgpu_gfx940_op_V_CMP_GE_F32,"V_CMP_GE_F32"}, // 70
                {amdgpu_gfx940_op_V_CMP_O_F32,"V_CMP_O_F32"}, // 71
                {amdgpu_gfx940_op_V_CMP_U_F32,"V_CMP_U_F32"}, // 72
                {amdgpu_gfx940_op_V_CMP_NGE_F32,"V_CMP_NGE_F32"}, // 73
                {amdgpu_gfx940_op_V_CMP_NLG_F32,"V_CMP_NLG_F32"}, // 74
                {amdgpu_gfx940_op_V_CMP_NGT_F32,"V_CMP_NGT_F32"}, // 75
                {amdgpu_gfx940_op_V_CMP_NLE_F32,"V_CMP_NLE_F32"}, // 76
                {amdgpu_gfx940_op_V_CMP_NEQ_F32,"V_CMP_NEQ_F32"}, // 77
                {amdgpu_gfx940_op_V_CMP_NLT_F32,"V_CMP_NLT_F32"}, // 78
                {amdgpu_gfx940_op_V_CMP_TRU_F32,"V_CMP_TRU_F32"}, // 79
                {amdgpu_gfx940_op_V_CMPX_F_F32,"V_CMPX_F_F32"}, // 80
                {amdgpu_gfx940_op_V_CMPX_LT_F32,"V_CMPX_LT_F32"}, // 81
                {amdgpu_gfx940_op_V_CMPX_EQ_F32,"V_CMPX_EQ_F32"}, // 82
                {amdgpu_gfx940_op_V_CMPX_LE_F32,"V_CMPX_LE_F32"}, // 83
                {amdgpu_gfx940_op_V_CMPX_GT_F32,"V_CMPX_GT_F32"}, // 84
                {amdgpu_gfx940_op_V_CMPX_LG_F32,"V_CMPX_LG_F32"}, // 85
                {amdgpu_gfx940_op_V_CMPX_GE_F32,"V_CMPX_GE_F32"}, // 86
                {amdgpu_gfx940_op_V_CMPX_O_F32,"V_CMPX_O_F32"}, // 87
                {amdgpu_gfx940_op_V_CMPX_U_F32,"V_CMPX_U_F32"}, // 88
                {amdgpu_gfx940_op_V_CMPX_NGE_F32,"V_CMPX_NGE_F32"}, // 89
                {amdgpu_gfx940_op_V_CMPX_NLG_F32,"V_CMPX_NLG_F32"}, // 90
                {amdgpu_gfx940_op_V_CMPX_NGT_F32,"V_CMPX_NGT_F32"}, // 91
                {amdgpu_gfx940_op_V_CMPX_NLE_F32,"V_CMPX_NLE_F32"}, // 92
                {amdgpu_gfx940_op_V_CMPX_NEQ_F32,"V_CMPX_NEQ_F32"}, // 93
                {amdgpu_gfx940_op_V_CMPX_NLT_F32,"V_CMPX_NLT_F32"}, // 94
                {amdgpu_gfx940_op_V_CMPX_TRU_F32,"V_CMPX_TRU_F32"}, // 95
                {amdgpu_gfx940_op_V_CMP_F_F64,"V_CMP_F_F64"}, // 96
                {amdgpu_gfx940_op_V_CMP_LT_F64,"V_CMP_LT_F64"}, // 97
                {amdgpu_gfx940_op_V_CMP_EQ_F64,"V_CMP_EQ_F64"}, // 98
                {amdgpu_gfx940_op_V_CMP_LE_F64,"V_CMP_LE_F64"}, // 99
                {amdgpu_gfx940_op_V_CMP_GT_F64,"V_CMP_GT_F64"}, // 100
                {amdgpu_gfx940_op_V_CMP_LG_F64,"V_CMP_LG_F64"}, // 101
                {amdgpu_gfx940_op_V_CMP_GE_F64,"V_CMP_GE_F64"}, // 102
                {amdgpu_gfx940_op_V_CMP_O_F64,"V_CMP_O_F64"}, // 103
                {amdgpu_gfx940_op_V_CMP_U_F64,"V_CMP_U_F64"}, // 104
                {amdgpu_gfx940_op_V_CMP_NGE_F64,"V_CMP_NGE_F64"}, // 105
                {amdgpu_gfx940_op_V_CMP_NLG_F64,"V_CMP_NLG_F64"}, // 106
                {amdgpu_gfx940_op_V_CMP_NGT_F64,"V_CMP_NGT_F64"}, // 107
                {amdgpu_gfx940_op_V_CMP_NLE_F64,"V_CMP_NLE_F64"}, // 108
                {amdgpu_gfx940_op_V_CMP_NEQ_F64,"V_CMP_NEQ_F64"}, // 109
                {amdgpu_gfx940_op_V_CMP_NLT_F64,"V_CMP_NLT_F64"}, // 110
                {amdgpu_gfx940_op_V_CMP_TRU_F64,"V_CMP_TRU_F64"}, // 111
                {amdgpu_gfx940_op_V_CMPX_F_F64,"V_CMPX_F_F64"}, // 112
                {amdgpu_gfx940_op_V_CMPX_LT_F64,"V_CMPX_LT_F64"}, // 113
                {amdgpu_gfx940_op_V_CMPX_EQ_F64,"V_CMPX_EQ_F64"}, // 114
                {amdgpu_gfx940_op_V_CMPX_LE_F64,"V_CMPX_LE_F64"}, // 115
                {amdgpu_gfx940_op_V_CMPX_GT_F64,"V_CMPX_GT_F64"}, // 116
                {amdgpu_gfx940_op_V_CMPX_LG_F64,"V_CMPX_LG_F64"}, // 117
                {amdgpu_gfx940_op_V_CMPX_GE_F64,"V_CMPX_GE_F64"}, // 118
                {amdgpu_gfx940_op_V_CMPX_O_F64,"V_CMPX_O_F64"}, // 119
                {amdgpu_gfx940_op_V_CMPX_U_F64,"V_CMPX_U_F64"}, // 120
                {amdgpu_gfx940_op_V_CMPX_NGE_F64,"V_CMPX_NGE_F64"}, // 121
                {amdgpu_gfx940_op_V_CMPX_NLG_F64,"V_CMPX_NLG_F64"}, // 122
                {amdgpu_gfx940_op_V_CMPX_NGT_F64,"V_CMPX_NGT_F64"}, // 123
                {amdgpu_gfx940_op_V_CMPX_NLE_F64,"V_CMPX_NLE_F64"}, // 124
                {amdgpu_gfx940_op_V_CMPX_NEQ_F64,"V_CMPX_NEQ_F64"}, // 125
                {amdgpu_gfx940_op_V_CMPX_NLT_F64,"V_CMPX_NLT_F64"}, // 126
                {amdgpu_gfx940_op_V_CMPX_TRU_F64,"V_CMPX_TRU_F64"}, // 127
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 128
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 129
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 130
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 131
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 132
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 133
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 134
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 135
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 136
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 137
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 138
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 139
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 140
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 141
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 142
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 143
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 144
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 145
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 146
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 147
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 148
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 149
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 150
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 151
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 152
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 153
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 154
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 155
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 156
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 157
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 158
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 159
                {amdgpu_gfx940_op_V_CMP_F_I16,"V_CMP_F_I16"}, // 160
                {amdgpu_gfx940_op_V_CMP_LT_I16,"V_CMP_LT_I16"}, // 161
                {amdgpu_gfx940_op_V_CMP_EQ_I16,"V_CMP_EQ_I16"}, // 162
                {amdgpu_gfx940_op_V_CMP_LE_I16,"V_CMP_LE_I16"}, // 163
                {amdgpu_gfx940_op_V_CMP_GT_I16,"V_CMP_GT_I16"}, // 164
                {amdgpu_gfx940_op_V_CMP_NE_I16,"V_CMP_NE_I16"}, // 165
                {amdgpu_gfx940_op_V_CMP_GE_I16,"V_CMP_GE_I16"}, // 166
                {amdgpu_gfx940_op_V_CMP_T_I16,"V_CMP_T_I16"}, // 167
                {amdgpu_gfx940_op_V_CMP_F_U16,"V_CMP_F_U16"}, // 168
                {amdgpu_gfx940_op_V_CMP_LT_U16,"V_CMP_LT_U16"}, // 169
                {amdgpu_gfx940_op_V_CMP_EQ_U16,"V_CMP_EQ_U16"}, // 170
                {amdgpu_gfx940_op_V_CMP_LE_U16,"V_CMP_LE_U16"}, // 171
                {amdgpu_gfx940_op_V_CMP_GT_U16,"V_CMP_GT_U16"}, // 172
                {amdgpu_gfx940_op_V_CMP_NE_U16,"V_CMP_NE_U16"}, // 173
                {amdgpu_gfx940_op_V_CMP_GE_U16,"V_CMP_GE_U16"}, // 174
                {amdgpu_gfx940_op_V_CMP_T_U16,"V_CMP_T_U16"}, // 175
                {amdgpu_gfx940_op_V_CMPX_F_I16,"V_CMPX_F_I16"}, // 176
                {amdgpu_gfx940_op_V_CMPX_LT_I16,"V_CMPX_LT_I16"}, // 177
                {amdgpu_gfx940_op_V_CMPX_EQ_I16,"V_CMPX_EQ_I16"}, // 178
                {amdgpu_gfx940_op_V_CMPX_LE_I16,"V_CMPX_LE_I16"}, // 179
                {amdgpu_gfx940_op_V_CMPX_GT_I16,"V_CMPX_GT_I16"}, // 180
                {amdgpu_gfx940_op_V_CMPX_NE_I16,"V_CMPX_NE_I16"}, // 181
                {amdgpu_gfx940_op_V_CMPX_GE_I16,"V_CMPX_GE_I16"}, // 182
                {amdgpu_gfx940_op_V_CMPX_T_I16,"V_CMPX_T_I16"}, // 183
                {amdgpu_gfx940_op_V_CMPX_F_U16,"V_CMPX_F_U16"}, // 184
                {amdgpu_gfx940_op_V_CMPX_LT_U16,"V_CMPX_LT_U16"}, // 185
                {amdgpu_gfx940_op_V_CMPX_EQ_U16,"V_CMPX_EQ_U16"}, // 186
                {amdgpu_gfx940_op_V_CMPX_LE_U16,"V_CMPX_LE_U16"}, // 187
                {amdgpu_gfx940_op_V_CMPX_GT_U16,"V_CMPX_GT_U16"}, // 188
                {amdgpu_gfx940_op_V_CMPX_NE_U16,"V_CMPX_NE_U16"}, // 189
                {amdgpu_gfx940_op_V_CMPX_GE_U16,"V_CMPX_GE_U16"}, // 190
                {amdgpu_gfx940_op_V_CMPX_T_U16,"V_CMPX_T_U16"}, // 191
                {amdgpu_gfx940_op_V_CMP_F_I32,"V_CMP_F_I32"}, // 192
                {amdgpu_gfx940_op_V_CMP_LT_I32,"V_CMP_LT_I32"}, // 193
                {amdgpu_gfx940_op_V_CMP_EQ_I32,"V_CMP_EQ_I32"}, // 194
                {amdgpu_gfx940_op_V_CMP_LE_I32,"V_CMP_LE_I32"}, // 195
                {amdgpu_gfx940_op_V_CMP_GT_I32,"V_CMP_GT_I32"}, // 196
                {amdgpu_gfx940_op_V_CMP_NE_I32,"V_CMP_NE_I32"}, // 197
                {amdgpu_gfx940_op_V_CMP_GE_I32,"V_CMP_GE_I32"}, // 198
                {amdgpu_gfx940_op_V_CMP_T_I32,"V_CMP_T_I32"}, // 199
                {amdgpu_gfx940_op_V_CMP_F_U32,"V_CMP_F_U32"}, // 200
                {amdgpu_gfx940_op_V_CMP_LT_U32,"V_CMP_LT_U32"}, // 201
                {amdgpu_gfx940_op_V_CMP_EQ_U32,"V_CMP_EQ_U32"}, // 202
                {amdgpu_gfx940_op_V_CMP_LE_U32,"V_CMP_LE_U32"}, // 203
                {amdgpu_gfx940_op_V_CMP_GT_U32,"V_CMP_GT_U32"}, // 204
                {amdgpu_gfx940_op_V_CMP_NE_U32,"V_CMP_NE_U32"}, // 205
                {amdgpu_gfx940_op_V_CMP_GE_U32,"V_CMP_GE_U32"}, // 206
                {amdgpu_gfx940_op_V_CMP_T_U32,"V_CMP_T_U32"}, // 207
                {amdgpu_gfx940_op_V_CMPX_F_I32,"V_CMPX_F_I32"}, // 208
                {amdgpu_gfx940_op_V_CMPX_LT_I32,"V_CMPX_LT_I32"}, // 209
                {amdgpu_gfx940_op_V_CMPX_EQ_I32,"V_CMPX_EQ_I32"}, // 210
                {amdgpu_gfx940_op_V_CMPX_LE_I32,"V_CMPX_LE_I32"}, // 211
                {amdgpu_gfx940_op_V_CMPX_GT_I32,"V_CMPX_GT_I32"}, // 212
                {amdgpu_gfx940_op_V_CMPX_NE_I32,"V_CMPX_NE_I32"}, // 213
                {amdgpu_gfx940_op_V_CMPX_GE_I32,"V_CMPX_GE_I32"}, // 214
                {amdgpu_gfx940_op_V_CMPX_T_I32,"V_CMPX_T_I32"}, // 215
                {amdgpu_gfx940_op_V_CMPX_F_U32,"V_CMPX_F_U32"}, // 216
                {amdgpu_gfx940_op_V_CMPX_LT_U32,"V_CMPX_LT_U32"}, // 217
                {amdgpu_gfx940_op_V_CMPX_EQ_U32,"V_CMPX_EQ_U32"}, // 218
                {amdgpu_gfx940_op_V_CMPX_LE_U32,"V_CMPX_LE_U32"}, // 219
                {amdgpu_gfx940_op_V_CMPX_GT_U32,"V_CMPX_GT_U32"}, // 220
                {amdgpu_gfx940_op_V_CMPX_NE_U32,"V_CMPX_NE_U32"}, // 221
                {amdgpu_gfx940_op_V_CMPX_GE_U32,"V_CMPX_GE_U32"}, // 222
                {amdgpu_gfx940_op_V_CMPX_T_U32,"V_CMPX_T_U32"}, // 223
                {amdgpu_gfx940_op_V_CMP_F_I64,"V_CMP_F_I64"}, // 224
                {amdgpu_gfx940_op_V_CMP_LT_I64,"V_CMP_LT_I64"}, // 225
                {amdgpu_gfx940_op_V_CMP_EQ_I64,"V_CMP_EQ_I64"}, // 226
                {amdgpu_gfx940_op_V_CMP_LE_I64,"V_CMP_LE_I64"}, // 227
                {amdgpu_gfx940_op_V_CMP_GT_I64,"V_CMP_GT_I64"}, // 228
                {amdgpu_gfx940_op_V_CMP_NE_I64,"V_CMP_NE_I64"}, // 229
                {amdgpu_gfx940_op_V_CMP_GE_I64,"V_CMP_GE_I64"}, // 230
                {amdgpu_gfx940_op_V_CMP_T_I64,"V_CMP_T_I64"}, // 231
                {amdgpu_gfx940_op_V_CMP_F_U64,"V_CMP_F_U64"}, // 232
                {amdgpu_gfx940_op_V_CMP_LT_U64,"V_CMP_LT_U64"}, // 233
                {amdgpu_gfx940_op_V_CMP_EQ_U64,"V_CMP_EQ_U64"}, // 234
                {amdgpu_gfx940_op_V_CMP_LE_U64,"V_CMP_LE_U64"}, // 235
                {amdgpu_gfx940_op_V_CMP_GT_U64,"V_CMP_GT_U64"}, // 236
                {amdgpu_gfx940_op_V_CMP_NE_U64,"V_CMP_NE_U64"}, // 237
                {amdgpu_gfx940_op_V_CMP_GE_U64,"V_CMP_GE_U64"}, // 238
                {amdgpu_gfx940_op_V_CMP_T_U64,"V_CMP_T_U64"}, // 239
                {amdgpu_gfx940_op_V_CMPX_F_I64,"V_CMPX_F_I64"}, // 240
                {amdgpu_gfx940_op_V_CMPX_LT_I64,"V_CMPX_LT_I64"}, // 241
                {amdgpu_gfx940_op_V_CMPX_EQ_I64,"V_CMPX_EQ_I64"}, // 242
                {amdgpu_gfx940_op_V_CMPX_LE_I64,"V_CMPX_LE_I64"}, // 243
                {amdgpu_gfx940_op_V_CMPX_GT_I64,"V_CMPX_GT_I64"}, // 244
                {amdgpu_gfx940_op_V_CMPX_NE_I64,"V_CMPX_NE_I64"}, // 245
                {amdgpu_gfx940_op_V_CMPX_GE_I64,"V_CMPX_GE_I64"}, // 246
                {amdgpu_gfx940_op_V_CMPX_T_I64,"V_CMPX_T_I64"}, // 247
                {amdgpu_gfx940_op_V_CMPX_F_U64,"V_CMPX_F_U64"}, // 248
                {amdgpu_gfx940_op_V_CMPX_LT_U64,"V_CMPX_LT_U64"}, // 249
                {amdgpu_gfx940_op_V_CMPX_EQ_U64,"V_CMPX_EQ_U64"}, // 250
                {amdgpu_gfx940_op_V_CMPX_LE_U64,"V_CMPX_LE_U64"}, // 251
                {amdgpu_gfx940_op_V_CMPX_GT_U64,"V_CMPX_GT_U64"}, // 252
                {amdgpu_gfx940_op_V_CMPX_NE_U64,"V_CMPX_NE_U64"}, // 253
                {amdgpu_gfx940_op_V_CMPX_GE_U64,"V_CMPX_GE_U64"}, // 254
                {amdgpu_gfx940_op_V_CMPX_T_U64,"V_CMPX_T_U64"}, // 255
                {amdgpu_gfx940_op_V_CNDMASK_B32,"V_CNDMASK_B32"}, // 256
                {amdgpu_gfx940_op_V_ADD_F32,"V_ADD_F32"}, // 257
                {amdgpu_gfx940_op_V_SUB_F32,"V_SUB_F32"}, // 258
                {amdgpu_gfx940_op_V_SUBREV_F32,"V_SUBREV_F32"}, // 259
                {amdgpu_gfx940_op_V_FMAC_F64,"V_FMAC_F64"}, // 260
                {amdgpu_gfx940_op_V_MUL_F32,"V_MUL_F32"}, // 261
                {amdgpu_gfx940_op_V_MUL_I32_I24,"V_MUL_I32_I24"}, // 262
                {amdgpu_gfx940_op_V_MUL_HI_I32_I24,"V_MUL_HI_I32_I24"}, // 263
                {amdgpu_gfx940_op_V_MUL_U32_U24,"V_MUL_U32_U24"}, // 264
                {amdgpu_gfx940_op_V_MUL_HI_U32_U24,"V_MUL_HI_U32_U24"}, // 265
                {amdgpu_gfx940_op_V_MIN_F32,"V_MIN_F32"}, // 266
                {amdgpu_gfx940_op_V_MAX_F32,"V_MAX_F32"}, // 267
                {amdgpu_gfx940_op_V_MIN_I32,"V_MIN_I32"}, // 268
                {amdgpu_gfx940_op_V_MAX_I32,"V_MAX_I32"}, // 269
                {amdgpu_gfx940_op_V_MIN_U32,"V_MIN_U32"}, // 270
                {amdgpu_gfx940_op_V_MAX_U32,"V_MAX_U32"}, // 271
                {amdgpu_gfx940_op_V_LSHRREV_B32,"V_LSHRREV_B32"}, // 272
                {amdgpu_gfx940_op_V_ASHRREV_I32,"V_ASHRREV_I32"}, // 273
                {amdgpu_gfx940_op_V_LSHLREV_B32,"V_LSHLREV_B32"}, // 274
                {amdgpu_gfx940_op_V_AND_B32,"V_AND_B32"}, // 275
                {amdgpu_gfx940_op_V_OR_B32,"V_OR_B32"}, // 276
                {amdgpu_gfx940_op_V_XOR_B32,"V_XOR_B32"}, // 277
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 278
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 279
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 280
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 281
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 282
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 283
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 284
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 285
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 286
                {amdgpu_gfx940_op_V_ADD_F16,"V_ADD_F16"}, // 287
                {amdgpu_gfx940_op_V_SUB_F16,"V_SUB_F16"}, // 288
                {amdgpu_gfx940_op_V_SUBREV_F16,"V_SUBREV_F16"}, // 289
                {amdgpu_gfx940_op_V_MUL_F16,"V_MUL_F16"}, // 290
                {amdgpu_gfx940_op_V_MAC_F16,"V_MAC_F16"}, // 291
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 292
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 293
                {amdgpu_gfx940_op_V_ADD_U16,"V_ADD_U16"}, // 294
                {amdgpu_gfx940_op_V_SUB_U16,"V_SUB_U16"}, // 295
                {amdgpu_gfx940_op_V_SUBREV_U16,"V_SUBREV_U16"}, // 296
                {amdgpu_gfx940_op_V_MUL_LO_U16,"V_MUL_LO_U16"}, // 297
                {amdgpu_gfx940_op_V_LSHLREV_B16,"V_LSHLREV_B16"}, // 298
                {amdgpu_gfx940_op_V_LSHRREV_B16,"V_LSHRREV_B16"}, // 299
                {amdgpu_gfx940_op_V_ASHRREV_I16,"V_ASHRREV_I16"}, // 300
                {amdgpu_gfx940_op_V_MAX_F16,"V_MAX_F16"}, // 301
                {amdgpu_gfx940_op_V_MIN_F16,"V_MIN_F16"}, // 302
                {amdgpu_gfx940_op_V_MAX_U16,"V_MAX_U16"}, // 303
                {amdgpu_gfx940_op_V_MAX_I16,"V_MAX_I16"}, // 304
                {amdgpu_gfx940_op_V_MIN_U16,"V_MIN_U16"}, // 305
                {amdgpu_gfx940_op_V_MIN_I16,"V_MIN_I16"}, // 306
                {amdgpu_gfx940_op_V_LDEXP_F16,"V_LDEXP_F16"}, // 307
                {amdgpu_gfx940_op_V_ADD_U32,"V_ADD_U32"}, // 308
                {amdgpu_gfx940_op_V_SUB_U32,"V_SUB_U32"}, // 309
                {amdgpu_gfx940_op_V_SUBREV_U32,"V_SUBREV_U32"}, // 310
                {amdgpu_gfx940_op_V_DOT2C_F32_F16,"V_DOT2C_F32_F16"}, // 311
                {amdgpu_gfx940_op_V_DOT2C_I32_I16,"V_DOT2C_I32_I16"}, // 312
                {amdgpu_gfx940_op_V_DOT4C_I32_I8,"V_DOT4C_I32_I8"}, // 313
                {amdgpu_gfx940_op_V_DOT8C_I32_I4,"V_DOT8C_I32_I4"}, // 314
                {amdgpu_gfx940_op_V_FMAC_F32,"V_FMAC_F32"}, // 315
                {amdgpu_gfx940_op_V_PK_FMAC_F16,"V_PK_FMAC_F16"}, // 316
                {amdgpu_gfx940_op_V_XNOR_B32,"V_XNOR_B32"}, // 317
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 318
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 319
                {amdgpu_gfx940_op_V_NOP,"V_NOP"}, // 320
                {amdgpu_gfx940_op_V_MOV_B32,"V_MOV_B32"}, // 321
                {amdgpu_gfx940_op_V_READFIRSTLANE_B32,"V_READFIRSTLANE_B32"}, // 322
                {amdgpu_gfx940_op_V_CVT_I32_F64,"V_CVT_I32_F64"}, // 323
                {amdgpu_gfx940_op_V_CVT_F64_I32,"V_CVT_F64_I32"}, // 324
                {amdgpu_gfx940_op_V_CVT_F32_I32,"V_CVT_F32_I32"}, // 325
                {amdgpu_gfx940_op_V_CVT_F32_U32,"V_CVT_F32_U32"}, // 326
                {amdgpu_gfx940_op_V_CVT_U32_F32,"V_CVT_U32_F32"}, // 327
                {amdgpu_gfx940_op_V_CVT_I32_F32,"V_CVT_I32_F32"}, // 328
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 329
                {amdgpu_gfx940_op_V_CVT_F16_F32,"V_CVT_F16_F32"}, // 330
                {amdgpu_gfx940_op_V_CVT_F32_F16,"V_CVT_F32_F16"}, // 331
                {amdgpu_gfx940_op_V_CVT_RPI_I32_F32,"V_CVT_RPI_I32_F32"}, // 332
                {amdgpu_gfx940_op_V_CVT_FLR_I32_F32,"V_CVT_FLR_I32_F32"}, // 333
                {amdgpu_gfx940_op_V_CVT_OFF_F32_I4,"V_CVT_OFF_F32_I4"}, // 334
                {amdgpu_gfx940_op_V_CVT_F32_F64,"V_CVT_F32_F64"}, // 335
                {amdgpu_gfx940_op_V_CVT_F64_F32,"V_CVT_F64_F32"}, // 336
                {amdgpu_gfx940_op_V_CVT_F32_UBYTE0,"V_CVT_F32_UBYTE0"}, // 337
                {amdgpu_gfx940_op_V_CVT_F32_UBYTE1,"V_CVT_F32_UBYTE1"}, // 338
                {amdgpu_gfx940_op_V_CVT_F32_UBYTE2,"V_CVT_F32_UBYTE2"}, // 339
                {amdgpu_gfx940_op_V_CVT_F32_UBYTE3,"V_CVT_F32_UBYTE3"}, // 340
                {amdgpu_gfx940_op_V_CVT_U32_F64,"V_CVT_U32_F64"}, // 341
                {amdgpu_gfx940_op_V_CVT_F64_U32,"V_CVT_F64_U32"}, // 342
                {amdgpu_gfx940_op_V_TRUNC_F64,"V_TRUNC_F64"}, // 343
                {amdgpu_gfx940_op_V_CEIL_F64,"V_CEIL_F64"}, // 344
                {amdgpu_gfx940_op_V_RNDNE_F64,"V_RNDNE_F64"}, // 345
                {amdgpu_gfx940_op_V_FLOOR_F64,"V_FLOOR_F64"}, // 346
                {amdgpu_gfx940_op_V_FRACT_F32,"V_FRACT_F32"}, // 347
                {amdgpu_gfx940_op_V_TRUNC_F32,"V_TRUNC_F32"}, // 348
                {amdgpu_gfx940_op_V_CEIL_F32,"V_CEIL_F32"}, // 349
                {amdgpu_gfx940_op_V_RNDNE_F32,"V_RNDNE_F32"}, // 350
                {amdgpu_gfx940_op_V_FLOOR_F32,"V_FLOOR_F32"}, // 351
                {amdgpu_gfx940_op_V_EXP_F32,"V_EXP_F32"}, // 352
                {amdgpu_gfx940_op_V_LOG_F32,"V_LOG_F32"}, // 353
                {amdgpu_gfx940_op_V_RCP_F32,"V_RCP_F32"}, // 354
                {amdgpu_gfx940_op_V_RCP_IFLAG_F32,"V_RCP_IFLAG_F32"}, // 355
                {amdgpu_gfx940_op_V_RSQ_F32,"V_RSQ_F32"}, // 356
                {amdgpu_gfx940_op_V_RCP_F64,"V_RCP_F64"}, // 357
                {amdgpu_gfx940_op_V_RSQ_F64,"V_RSQ_F64"}, // 358
                {amdgpu_gfx940_op_V_SQRT_F32,"V_SQRT_F32"}, // 359
                {amdgpu_gfx940_op_V_SQRT_F64,"V_SQRT_F64"}, // 360
                {amdgpu_gfx940_op_V_SIN_F32,"V_SIN_F32"}, // 361
                {amdgpu_gfx940_op_V_COS_F32,"V_COS_F32"}, // 362
                {amdgpu_gfx940_op_V_NOT_B32,"V_NOT_B32"}, // 363
                {amdgpu_gfx940_op_V_BFREV_B32,"V_BFREV_B32"}, // 364
                {amdgpu_gfx940_op_V_FFBH_U32,"V_FFBH_U32"}, // 365
                {amdgpu_gfx940_op_V_FFBL_B32,"V_FFBL_B32"}, // 366
                {amdgpu_gfx940_op_V_FFBH_I32,"V_FFBH_I32"}, // 367
                {amdgpu_gfx940_op_V_FREXP_EXP_I32_F64,"V_FREXP_EXP_I32_F64"}, // 368
                {amdgpu_gfx940_op_V_FREXP_MANT_F64,"V_FREXP_MANT_F64"}, // 369
                {amdgpu_gfx940_op_V_FRACT_F64,"V_FRACT_F64"}, // 370
                {amdgpu_gfx940_op_V_FREXP_EXP_I32_F32,"V_FREXP_EXP_I32_F32"}, // 371
                {amdgpu_gfx940_op_V_FREXP_MANT_F32,"V_FREXP_MANT_F32"}, // 372
                {amdgpu_gfx940_op_V_CLREXCP,"V_CLREXCP"}, // 373
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 374
                {amdgpu_gfx940_op_V_SCREEN_PARTITION_4SE_B32,"V_SCREEN_PARTITION_4SE_B32"}, // 375
                {amdgpu_gfx940_op_V_MOV_B64,"V_MOV_B64"}, // 376
                {amdgpu_gfx940_op_V_CVT_F16_U16,"V_CVT_F16_U16"}, // 377
                {amdgpu_gfx940_op_V_CVT_F16_I16,"V_CVT_F16_I16"}, // 378
                {amdgpu_gfx940_op_V_CVT_U16_F16,"V_CVT_U16_F16"}, // 379
                {amdgpu_gfx940_op_V_CVT_I16_F16,"V_CVT_I16_F16"}, // 380
                {amdgpu_gfx940_op_V_RCP_F16,"V_RCP_F16"}, // 381
                {amdgpu_gfx940_op_V_SQRT_F16,"V_SQRT_F16"}, // 382
                {amdgpu_gfx940_op_V_RSQ_F16,"V_RSQ_F16"}, // 383
                {amdgpu_gfx940_op_V_LOG_F16,"V_LOG_F16"}, // 384
                {amdgpu_gfx940_op_V_EXP_F16,"V_EXP_F16"}, // 385
                {amdgpu_gfx940_op_V_FREXP_MANT_F16,"V_FREXP_MANT_F16"}, // 386
                {amdgpu_gfx940_op_V_FREXP_EXP_I16_F16,"V_FREXP_EXP_I16_F16"}, // 387
                {amdgpu_gfx940_op_V_FLOOR_F16,"V_FLOOR_F16"}, // 388
                {amdgpu_gfx940_op_V_CEIL_F16,"V_CEIL_F16"}, // 389
                {amdgpu_gfx940_op_V_TRUNC_F16,"V_TRUNC_F16"}, // 390
                {amdgpu_gfx940_op_V_RNDNE_F16,"V_RNDNE_F16"}, // 391
                {amdgpu_gfx940_op_V_FRACT_F16,"V_FRACT_F16"}, // 392
                {amdgpu_gfx940_op_V_SIN_F16,"V_SIN_F16"}, // 393
                {amdgpu_gfx940_op_V_COS_F16,"V_COS_F16"}, // 394
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 395
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 396
                {amdgpu_gfx940_op_V_CVT_NORM_I16_F16,"V_CVT_NORM_I16_F16"}, // 397
                {amdgpu_gfx940_op_V_CVT_NORM_U16_F16,"V_CVT_NORM_U16_F16"}, // 398
                {amdgpu_gfx940_op_V_SAT_PK_U8_I16,"V_SAT_PK_U8_I16"}, // 399
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 400
                {amdgpu_gfx940_op_V_SWAP_B32,"V_SWAP_B32"}, // 401
                {amdgpu_gfx940_op_V_ACCVGPR_MOV_B32,"V_ACCVGPR_MOV_B32"}, // 402
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 403
                {amdgpu_gfx940_op_V_CVT_F32_FP8,"V_CVT_F32_FP8"}, // 404
                {amdgpu_gfx940_op_V_CVT_F32_BF8,"V_CVT_F32_BF8"}, // 405
                {amdgpu_gfx940_op_V_CVT_PK_F32_FP8,"V_CVT_PK_F32_FP8"}, // 406
                {amdgpu_gfx940_op_V_CVT_PK_F32_BF8,"V_CVT_PK_F32_BF8"}, // 407
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 408
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 409
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 410
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 411
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 412
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 413
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 414
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 415
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 416
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 417
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 418
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 419
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 420
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 421
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 422
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 423
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 424
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 425
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 426
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 427
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 428
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 429
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 430
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 431
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 432
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 433
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 434
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 435
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 436
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 437
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 438
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 439
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 440
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 441
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 442
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 443
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 444
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 445
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 446
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 447
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 448
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 449
                {amdgpu_gfx940_op_V_MAD_I32_I24,"V_MAD_I32_I24"}, // 450
                {amdgpu_gfx940_op_V_MAD_U32_U24,"V_MAD_U32_U24"}, // 451
                {amdgpu_gfx940_op_V_CUBEID_F32,"V_CUBEID_F32"}, // 452
                {amdgpu_gfx940_op_V_CUBESC_F32,"V_CUBESC_F32"}, // 453
                {amdgpu_gfx940_op_V_CUBETC_F32,"V_CUBETC_F32"}, // 454
                {amdgpu_gfx940_op_V_CUBEMA_F32,"V_CUBEMA_F32"}, // 455
                {amdgpu_gfx940_op_V_BFE_U32,"V_BFE_U32"}, // 456
                {amdgpu_gfx940_op_V_BFE_I32,"V_BFE_I32"}, // 457
                {amdgpu_gfx940_op_V_BFI_B32,"V_BFI_B32"}, // 458
                {amdgpu_gfx940_op_V_FMA_F32,"V_FMA_F32"}, // 459
                {amdgpu_gfx940_op_V_FMA_F64,"V_FMA_F64"}, // 460
                {amdgpu_gfx940_op_V_LERP_U8,"V_LERP_U8"}, // 461
                {amdgpu_gfx940_op_V_ALIGNBIT_B32,"V_ALIGNBIT_B32"}, // 462
                {amdgpu_gfx940_op_V_ALIGNBYTE_B32,"V_ALIGNBYTE_B32"}, // 463
                {amdgpu_gfx940_op_V_MIN3_F32,"V_MIN3_F32"}, // 464
                {amdgpu_gfx940_op_V_MIN3_I32,"V_MIN3_I32"}, // 465
                {amdgpu_gfx940_op_V_MIN3_U32,"V_MIN3_U32"}, // 466
                {amdgpu_gfx940_op_V_MAX3_F32,"V_MAX3_F32"}, // 467
                {amdgpu_gfx940_op_V_MAX3_I32,"V_MAX3_I32"}, // 468
                {amdgpu_gfx940_op_V_MAX3_U32,"V_MAX3_U32"}, // 469
                {amdgpu_gfx940_op_V_MED3_F32,"V_MED3_F32"}, // 470
                {amdgpu_gfx940_op_V_MED3_I32,"V_MED3_I32"}, // 471
                {amdgpu_gfx940_op_V_MED3_U32,"V_MED3_U32"}, // 472
                {amdgpu_gfx940_op_V_SAD_U8,"V_SAD_U8"}, // 473
                {amdgpu_gfx940_op_V_SAD_HI_U8,"V_SAD_HI_U8"}, // 474
                {amdgpu_gfx940_op_V_SAD_U16,"V_SAD_U16"}, // 475
                {amdgpu_gfx940_op_V_SAD_U32,"V_SAD_U32"}, // 476
                {amdgpu_gfx940_op_V_CVT_PK_U8_F32,"V_CVT_PK_U8_F32"}, // 477
                {amdgpu_gfx940_op_V_DIV_FIXUP_F32,"V_DIV_FIXUP_F32"}, // 478
                {amdgpu_gfx940_op_V_DIV_FIXUP_F64,"V_DIV_FIXUP_F64"}, // 479
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 480
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 481
                {amdgpu_gfx940_op_V_DIV_FMAS_F32,"V_DIV_FMAS_F32"}, // 482
                {amdgpu_gfx940_op_V_DIV_FMAS_F64,"V_DIV_FMAS_F64"}, // 483
                {amdgpu_gfx940_op_V_MSAD_U8,"V_MSAD_U8"}, // 484
                {amdgpu_gfx940_op_V_QSAD_PK_U16_U8,"V_QSAD_PK_U16_U8"}, // 485
                {amdgpu_gfx940_op_V_MQSAD_PK_U16_U8,"V_MQSAD_PK_U16_U8"}, // 486
                {amdgpu_gfx940_op_V_MQSAD_U32_U8,"V_MQSAD_U32_U8"}, // 487
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 488
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 489
                {amdgpu_gfx940_op_V_MAD_LEGACY_F16,"V_MAD_LEGACY_F16"}, // 490
                {amdgpu_gfx940_op_V_MAD_LEGACY_U16,"V_MAD_LEGACY_U16"}, // 491
                {amdgpu_gfx940_op_V_MAD_LEGACY_I16,"V_MAD_LEGACY_I16"}, // 492
                {amdgpu_gfx940_op_V_PERM_B32,"V_PERM_B32"}, // 493
                {amdgpu_gfx940_op_V_FMA_LEGACY_F16,"V_FMA_LEGACY_F16"}, // 494
                {amdgpu_gfx940_op_V_DIV_FIXUP_LEGACY_F16,"V_DIV_FIXUP_LEGACY_F16"}, // 495
                {amdgpu_gfx940_op_V_CVT_PKACCUM_U8_F32,"V_CVT_PKACCUM_U8_F32"}, // 496
                {amdgpu_gfx940_op_V_MAD_U32_U16,"V_MAD_U32_U16"}, // 497
                {amdgpu_gfx940_op_V_MAD_I32_I16,"V_MAD_I32_I16"}, // 498
                {amdgpu_gfx940_op_V_XAD_U32,"V_XAD_U32"}, // 499
                {amdgpu_gfx940_op_V_MIN3_F16,"V_MIN3_F16"}, // 500
                {amdgpu_gfx940_op_V_MIN3_I16,"V_MIN3_I16"}, // 501
                {amdgpu_gfx940_op_V_MIN3_U16,"V_MIN3_U16"}, // 502
                {amdgpu_gfx940_op_V_MAX3_F16,"V_MAX3_F16"}, // 503
                {amdgpu_gfx940_op_V_MAX3_I16,"V_MAX3_I16"}, // 504
                {amdgpu_gfx940_op_V_MAX3_U16,"V_MAX3_U16"}, // 505
                {amdgpu_gfx940_op_V_MED3_F16,"V_MED3_F16"}, // 506
                {amdgpu_gfx940_op_V_MED3_I16,"V_MED3_I16"}, // 507
                {amdgpu_gfx940_op_V_MED3_U16,"V_MED3_U16"}, // 508
                {amdgpu_gfx940_op_V_LSHL_ADD_U32,"V_LSHL_ADD_U32"}, // 509
                {amdgpu_gfx940_op_V_ADD_LSHL_U32,"V_ADD_LSHL_U32"}, // 510
                {amdgpu_gfx940_op_V_ADD3_U32,"V_ADD3_U32"}, // 511
                {amdgpu_gfx940_op_V_LSHL_OR_B32,"V_LSHL_OR_B32"}, // 512
                {amdgpu_gfx940_op_V_AND_OR_B32,"V_AND_OR_B32"}, // 513
                {amdgpu_gfx940_op_V_OR3_B32,"V_OR3_B32"}, // 514
                {amdgpu_gfx940_op_V_MAD_F16,"V_MAD_F16"}, // 515
                {amdgpu_gfx940_op_V_MAD_U16,"V_MAD_U16"}, // 516
                {amdgpu_gfx940_op_V_MAD_I16,"V_MAD_I16"}, // 517
                {amdgpu_gfx940_op_V_FMA_F16,"V_FMA_F16"}, // 518
                {amdgpu_gfx940_op_V_DIV_FIXUP_F16,"V_DIV_FIXUP_F16"}, // 519
                {amdgpu_gfx940_op_V_LSHL_ADD_U64,"V_LSHL_ADD_U64"}, // 520
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 521
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 522
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 523
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 524
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 525
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 526
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 527
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 528
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 529
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 530
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 531
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 532
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 533
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 534
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 535
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 536
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 537
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 538
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 539
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 540
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 541
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 542
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 543
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 544
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 545
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 546
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 547
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 548
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 549
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 550
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 551
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 552
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 553
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 554
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 555
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 556
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 557
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 558
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 559
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 560
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 561
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 562
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 563
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 564
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 565
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 566
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 567
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 568
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 569
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 570
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 571
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 572
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 573
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 574
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 575
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 576
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 577
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 578
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 579
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 580
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 581
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 582
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 583
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 584
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 585
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 586
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 587
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 588
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 589
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 590
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 591
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 592
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 593
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 594
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 595
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 596
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 597
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 598
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 599
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 600
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 601
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 602
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 603
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 604
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 605
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 606
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 607
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 608
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 609
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 610
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 611
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 612
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 613
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 614
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 615
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 616
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 617
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 618
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 619
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 620
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 621
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 622
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 623
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 624
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 625
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 626
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 627
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 628
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 629
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 630
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 631
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 632
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 633
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 634
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 635
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 636
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 637
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 638
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 639
                {amdgpu_gfx940_op_V_ADD_F64,"V_ADD_F64"}, // 640
                {amdgpu_gfx940_op_V_MUL_F64,"V_MUL_F64"}, // 641
                {amdgpu_gfx940_op_V_MIN_F64,"V_MIN_F64"}, // 642
                {amdgpu_gfx940_op_V_MAX_F64,"V_MAX_F64"}, // 643
                {amdgpu_gfx940_op_V_LDEXP_F64,"V_LDEXP_F64"}, // 644
                {amdgpu_gfx940_op_V_MUL_LO_U32,"V_MUL_LO_U32"}, // 645
                {amdgpu_gfx940_op_V_MUL_HI_U32,"V_MUL_HI_U32"}, // 646
                {amdgpu_gfx940_op_V_MUL_HI_I32,"V_MUL_HI_I32"}, // 647
                {amdgpu_gfx940_op_V_LDEXP_F32,"V_LDEXP_F32"}, // 648
                {amdgpu_gfx940_op_V_READLANE_B32,"V_READLANE_B32"}, // 649
                {amdgpu_gfx940_op_V_WRITELANE_B32,"V_WRITELANE_B32"}, // 650
                {amdgpu_gfx940_op_V_BCNT_U32_B32,"V_BCNT_U32_B32"}, // 651
                {amdgpu_gfx940_op_V_MBCNT_LO_U32_B32,"V_MBCNT_LO_U32_B32"}, // 652
                {amdgpu_gfx940_op_V_MBCNT_HI_U32_B32,"V_MBCNT_HI_U32_B32"}, // 653
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 654
                {amdgpu_gfx940_op_V_LSHLREV_B64,"V_LSHLREV_B64"}, // 655
                {amdgpu_gfx940_op_V_LSHRREV_B64,"V_LSHRREV_B64"}, // 656
                {amdgpu_gfx940_op_V_ASHRREV_I64,"V_ASHRREV_I64"}, // 657
                {amdgpu_gfx940_op_V_TRIG_PREOP_F64,"V_TRIG_PREOP_F64"}, // 658
                {amdgpu_gfx940_op_V_BFM_B32,"V_BFM_B32"}, // 659
                {amdgpu_gfx940_op_V_CVT_PKNORM_I16_F32,"V_CVT_PKNORM_I16_F32"}, // 660
                {amdgpu_gfx940_op_V_CVT_PKNORM_U16_F32,"V_CVT_PKNORM_U16_F32"}, // 661
                {amdgpu_gfx940_op_V_CVT_PKRTZ_F16_F32,"V_CVT_PKRTZ_F16_F32"}, // 662
                {amdgpu_gfx940_op_V_CVT_PK_U16_U32,"V_CVT_PK_U16_U32"}, // 663
                {amdgpu_gfx940_op_V_CVT_PK_I16_I32,"V_CVT_PK_I16_I32"}, // 664
                {amdgpu_gfx940_op_V_CVT_PKNORM_I16_F16,"V_CVT_PKNORM_I16_F16"}, // 665
                {amdgpu_gfx940_op_V_CVT_PKNORM_U16_F16,"V_CVT_PKNORM_U16_F16"}, // 666
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 667
                {amdgpu_gfx940_op_V_ADD_I32,"V_ADD_I32"}, // 668
                {amdgpu_gfx940_op_V_SUB_I32,"V_SUB_I32"}, // 669
                {amdgpu_gfx940_op_V_ADD_I16,"V_ADD_I16"}, // 670
                {amdgpu_gfx940_op_V_SUB_I16,"V_SUB_I16"}, // 671
                {amdgpu_gfx940_op_V_PACK_B32_F16,"V_PACK_B32_F16"}, // 672
                {amdgpu_gfx940_op_V_MUL_LEGACY_F32,"V_MUL_LEGACY_F32"}, // 673
                {amdgpu_gfx940_op_V_CVT_PK_FP8_F32,"V_CVT_PK_FP8_F32"}, // 674
                {amdgpu_gfx940_op_V_CVT_PK_BF8_F32,"V_CVT_PK_BF8_F32"}, // 675
                {amdgpu_gfx940_op_V_CVT_SR_FP8_F32,"V_CVT_SR_FP8_F32"}, // 676
                {amdgpu_gfx940_op_V_CVT_SR_BF8_F32,"V_CVT_SR_BF8_F32"}, // 677
            }; // end ENC_VOP3_insn_table
            const amdgpu_gfx940_insn_entry ENC_VOP2_insn_table[62] = 
            {
                {amdgpu_gfx940_op_V_CNDMASK_B32,"V_CNDMASK_B32"}, // 0
                {amdgpu_gfx940_op_V_ADD_F32,"V_ADD_F32"}, // 1
                {amdgpu_gfx940_op_V_SUB_F32,"V_SUB_F32"}, // 2
                {amdgpu_gfx940_op_V_SUBREV_F32,"V_SUBREV_F32"}, // 3
                {amdgpu_gfx940_op_V_FMAC_F64,"V_FMAC_F64"}, // 4
                {amdgpu_gfx940_op_V_MUL_F32,"V_MUL_F32"}, // 5
                {amdgpu_gfx940_op_V_MUL_I32_I24,"V_MUL_I32_I24"}, // 6
                {amdgpu_gfx940_op_V_MUL_HI_I32_I24,"V_MUL_HI_I32_I24"}, // 7
                {amdgpu_gfx940_op_V_MUL_U32_U24,"V_MUL_U32_U24"}, // 8
                {amdgpu_gfx940_op_V_MUL_HI_U32_U24,"V_MUL_HI_U32_U24"}, // 9
                {amdgpu_gfx940_op_V_MIN_F32,"V_MIN_F32"}, // 10
                {amdgpu_gfx940_op_V_MAX_F32,"V_MAX_F32"}, // 11
                {amdgpu_gfx940_op_V_MIN_I32,"V_MIN_I32"}, // 12
                {amdgpu_gfx940_op_V_MAX_I32,"V_MAX_I32"}, // 13
                {amdgpu_gfx940_op_V_MIN_U32,"V_MIN_U32"}, // 14
                {amdgpu_gfx940_op_V_MAX_U32,"V_MAX_U32"}, // 15
                {amdgpu_gfx940_op_V_LSHRREV_B32,"V_LSHRREV_B32"}, // 16
                {amdgpu_gfx940_op_V_ASHRREV_I32,"V_ASHRREV_I32"}, // 17
                {amdgpu_gfx940_op_V_LSHLREV_B32,"V_LSHLREV_B32"}, // 18
                {amdgpu_gfx940_op_V_AND_B32,"V_AND_B32"}, // 19
                {amdgpu_gfx940_op_V_OR_B32,"V_OR_B32"}, // 20
                {amdgpu_gfx940_op_V_XOR_B32,"V_XOR_B32"}, // 21
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 22
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 23
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 24
                {amdgpu_gfx940_op_V_ADD_CO_U32,"V_ADD_CO_U32"}, // 25
                {amdgpu_gfx940_op_V_SUB_CO_U32,"V_SUB_CO_U32"}, // 26
                {amdgpu_gfx940_op_V_SUBREV_CO_U32,"V_SUBREV_CO_U32"}, // 27
                {amdgpu_gfx940_op_V_ADDC_CO_U32,"V_ADDC_CO_U32"}, // 28
                {amdgpu_gfx940_op_V_SUBB_CO_U32,"V_SUBB_CO_U32"}, // 29
                {amdgpu_gfx940_op_V_SUBBREV_CO_U32,"V_SUBBREV_CO_U32"}, // 30
                {amdgpu_gfx940_op_V_ADD_F16,"V_ADD_F16"}, // 31
                {amdgpu_gfx940_op_V_SUB_F16,"V_SUB_F16"}, // 32
                {amdgpu_gfx940_op_V_SUBREV_F16,"V_SUBREV_F16"}, // 33
                {amdgpu_gfx940_op_V_MUL_F16,"V_MUL_F16"}, // 34
                {amdgpu_gfx940_op_V_MAC_F16,"V_MAC_F16"}, // 35
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 36
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 37
                {amdgpu_gfx940_op_V_ADD_U16,"V_ADD_U16"}, // 38
                {amdgpu_gfx940_op_V_SUB_U16,"V_SUB_U16"}, // 39
                {amdgpu_gfx940_op_V_SUBREV_U16,"V_SUBREV_U16"}, // 40
                {amdgpu_gfx940_op_V_MUL_LO_U16,"V_MUL_LO_U16"}, // 41
                {amdgpu_gfx940_op_V_LSHLREV_B16,"V_LSHLREV_B16"}, // 42
                {amdgpu_gfx940_op_V_LSHRREV_B16,"V_LSHRREV_B16"}, // 43
                {amdgpu_gfx940_op_V_ASHRREV_I16,"V_ASHRREV_I16"}, // 44
                {amdgpu_gfx940_op_V_MAX_F16,"V_MAX_F16"}, // 45
                {amdgpu_gfx940_op_V_MIN_F16,"V_MIN_F16"}, // 46
                {amdgpu_gfx940_op_V_MAX_U16,"V_MAX_U16"}, // 47
                {amdgpu_gfx940_op_V_MAX_I16,"V_MAX_I16"}, // 48
                {amdgpu_gfx940_op_V_MIN_U16,"V_MIN_U16"}, // 49
                {amdgpu_gfx940_op_V_MIN_I16,"V_MIN_I16"}, // 50
                {amdgpu_gfx940_op_V_LDEXP_F16,"V_LDEXP_F16"}, // 51
                {amdgpu_gfx940_op_V_ADD_U32,"V_ADD_U32"}, // 52
                {amdgpu_gfx940_op_V_SUB_U32,"V_SUB_U32"}, // 53
                {amdgpu_gfx940_op_V_SUBREV_U32,"V_SUBREV_U32"}, // 54
                {amdgpu_gfx940_op_V_DOT2C_F32_F16,"V_DOT2C_F32_F16"}, // 55
                {amdgpu_gfx940_op_V_DOT2C_I32_I16,"V_DOT2C_I32_I16"}, // 56
                {amdgpu_gfx940_op_V_DOT4C_I32_I8,"V_DOT4C_I32_I8"}, // 57
                {amdgpu_gfx940_op_V_DOT8C_I32_I4,"V_DOT8C_I32_I4"}, // 58
                {amdgpu_gfx940_op_V_FMAC_F32,"V_FMAC_F32"}, // 59
                {amdgpu_gfx940_op_V_PK_FMAC_F16,"V_PK_FMAC_F16"}, // 60
                {amdgpu_gfx940_op_V_XNOR_B32,"V_XNOR_B32"}, // 61
            }; // end ENC_VOP2_insn_table
            const amdgpu_gfx940_insn_entry ENC_VOP2_LITERAL_insn_table[38] = 
            {
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 0
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 1
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 2
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 3
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 4
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 5
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 6
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 7
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 8
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 9
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 10
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 11
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 12
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 13
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 14
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 15
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 16
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 17
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 18
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 19
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 20
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 21
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 22
                {amdgpu_gfx940_op_V_FMAMK_F32,"V_FMAMK_F32"}, // 23
                {amdgpu_gfx940_op_V_FMAAK_F32,"V_FMAAK_F32"}, // 24
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 25
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 26
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 27
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 28
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 29
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 30
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 31
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 32
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 33
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 34
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 35
                {amdgpu_gfx940_op_V_MADMK_F16,"V_MADMK_F16"}, // 36
                {amdgpu_gfx940_op_V_MADAK_F16,"V_MADAK_F16"}, // 37
            }; // end ENC_VOP2_LITERAL_insn_table
            const amdgpu_gfx940_insn_entry ENC_VOP3B_insn_table[490] = 
            {
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 0
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 1
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 2
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 3
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 4
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 5
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 6
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 7
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 8
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 9
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 10
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 11
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 12
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 13
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 14
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 15
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 16
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 17
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 18
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 19
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 20
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 21
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 22
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 23
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 24
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 25
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 26
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 27
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 28
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 29
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 30
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 31
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 32
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 33
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 34
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 35
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 36
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 37
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 38
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 39
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 40
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 41
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 42
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 43
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 44
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 45
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 46
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 47
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 48
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 49
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 50
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 51
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 52
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 53
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 54
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 55
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 56
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 57
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 58
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 59
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 60
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 61
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 62
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 63
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 64
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 65
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 66
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 67
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 68
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 69
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 70
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 71
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 72
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 73
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 74
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 75
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 76
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 77
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 78
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 79
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 80
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 81
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 82
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 83
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 84
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 85
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 86
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 87
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 88
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 89
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 90
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 91
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 92
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 93
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 94
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 95
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 96
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 97
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 98
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 99
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 100
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 101
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 102
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 103
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 104
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 105
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 106
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 107
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 108
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 109
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 110
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 111
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 112
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 113
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 114
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 115
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 116
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 117
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 118
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 119
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 120
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 121
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 122
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 123
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 124
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 125
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 126
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 127
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 128
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 129
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 130
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 131
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 132
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 133
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 134
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 135
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 136
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 137
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 138
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 139
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 140
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 141
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 142
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 143
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 144
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 145
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 146
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 147
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 148
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 149
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 150
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 151
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 152
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 153
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 154
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 155
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 156
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 157
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 158
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 159
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 160
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 161
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 162
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 163
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 164
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 165
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 166
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 167
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 168
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 169
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 170
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 171
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 172
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 173
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 174
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 175
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 176
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 177
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 178
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 179
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 180
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 181
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 182
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 183
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 184
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 185
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 186
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 187
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 188
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 189
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 190
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 191
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 192
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 193
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 194
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 195
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 196
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 197
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 198
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 199
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 200
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 201
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 202
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 203
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 204
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 205
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 206
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 207
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 208
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 209
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 210
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 211
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 212
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 213
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 214
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 215
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 216
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 217
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 218
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 219
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 220
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 221
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 222
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 223
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 224
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 225
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 226
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 227
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 228
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 229
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 230
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 231
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 232
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 233
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 234
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 235
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 236
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 237
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 238
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 239
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 240
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 241
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 242
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 243
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 244
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 245
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 246
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 247
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 248
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 249
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 250
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 251
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 252
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 253
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 254
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 255
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 256
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 257
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 258
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 259
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 260
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 261
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 262
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 263
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 264
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 265
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 266
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 267
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 268
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 269
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 270
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 271
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 272
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 273
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 274
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 275
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 276
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 277
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 278
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 279
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 280
                {amdgpu_gfx940_op_V_ADD_CO_U32,"V_ADD_CO_U32"}, // 281
                {amdgpu_gfx940_op_V_SUB_CO_U32,"V_SUB_CO_U32"}, // 282
                {amdgpu_gfx940_op_V_SUBREV_CO_U32,"V_SUBREV_CO_U32"}, // 283
                {amdgpu_gfx940_op_V_ADDC_CO_U32,"V_ADDC_CO_U32"}, // 284
                {amdgpu_gfx940_op_V_SUBB_CO_U32,"V_SUBB_CO_U32"}, // 285
                {amdgpu_gfx940_op_V_SUBBREV_CO_U32,"V_SUBBREV_CO_U32"}, // 286
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 287
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 288
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 289
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 290
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 291
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 292
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 293
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 294
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 295
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 296
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 297
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 298
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 299
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 300
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 301
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 302
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 303
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 304
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 305
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 306
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 307
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 308
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 309
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 310
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 311
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 312
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 313
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 314
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 315
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 316
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 317
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 318
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 319
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 320
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 321
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 322
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 323
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 324
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 325
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 326
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 327
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 328
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 329
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 330
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 331
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 332
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 333
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 334
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 335
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 336
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 337
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 338
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 339
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 340
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 341
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 342
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 343
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 344
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 345
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 346
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 347
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 348
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 349
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 350
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 351
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 352
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 353
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 354
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 355
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 356
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 357
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 358
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 359
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 360
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 361
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 362
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 363
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 364
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 365
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 366
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 367
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 368
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 369
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 370
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 371
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 372
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 373
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 374
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 375
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 376
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 377
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 378
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 379
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 380
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 381
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 382
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 383
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 384
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 385
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 386
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 387
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 388
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 389
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 390
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 391
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 392
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 393
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 394
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 395
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 396
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 397
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 398
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 399
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 400
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 401
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 402
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 403
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 404
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 405
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 406
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 407
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 408
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 409
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 410
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 411
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 412
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 413
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 414
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 415
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 416
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 417
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 418
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 419
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 420
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 421
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 422
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 423
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 424
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 425
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 426
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 427
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 428
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 429
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 430
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 431
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 432
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 433
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 434
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 435
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 436
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 437
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 438
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 439
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 440
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 441
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 442
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 443
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 444
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 445
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 446
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 447
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 448
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 449
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 450
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 451
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 452
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 453
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 454
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 455
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 456
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 457
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 458
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 459
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 460
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 461
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 462
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 463
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 464
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 465
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 466
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 467
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 468
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 469
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 470
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 471
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 472
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 473
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 474
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 475
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 476
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 477
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 478
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 479
                {amdgpu_gfx940_op_V_DIV_SCALE_F32,"V_DIV_SCALE_F32"}, // 480
                {amdgpu_gfx940_op_V_DIV_SCALE_F64,"V_DIV_SCALE_F64"}, // 481
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 482
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 483
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 484
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 485
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 486
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 487
                {amdgpu_gfx940_op_V_MAD_U64_U32,"V_MAD_U64_U32"}, // 488
                {amdgpu_gfx940_op_V_MAD_I64_I32,"V_MAD_I64_I32"}, // 489
            }; // end ENC_VOP3B_insn_table
            const amdgpu_gfx940_insn_entry ENC_VOP3P_insn_table[90] = 
            {
                {amdgpu_gfx940_op_V_PK_MAD_I16,"V_PK_MAD_I16"}, // 0
                {amdgpu_gfx940_op_V_PK_MUL_LO_U16,"V_PK_MUL_LO_U16"}, // 1
                {amdgpu_gfx940_op_V_PK_ADD_I16,"V_PK_ADD_I16"}, // 2
                {amdgpu_gfx940_op_V_PK_SUB_I16,"V_PK_SUB_I16"}, // 3
                {amdgpu_gfx940_op_V_PK_LSHLREV_B16,"V_PK_LSHLREV_B16"}, // 4
                {amdgpu_gfx940_op_V_PK_LSHRREV_B16,"V_PK_LSHRREV_B16"}, // 5
                {amdgpu_gfx940_op_V_PK_ASHRREV_I16,"V_PK_ASHRREV_I16"}, // 6
                {amdgpu_gfx940_op_V_PK_MAX_I16,"V_PK_MAX_I16"}, // 7
                {amdgpu_gfx940_op_V_PK_MIN_I16,"V_PK_MIN_I16"}, // 8
                {amdgpu_gfx940_op_V_PK_MAD_U16,"V_PK_MAD_U16"}, // 9
                {amdgpu_gfx940_op_V_PK_ADD_U16,"V_PK_ADD_U16"}, // 10
                {amdgpu_gfx940_op_V_PK_SUB_U16,"V_PK_SUB_U16"}, // 11
                {amdgpu_gfx940_op_V_PK_MAX_U16,"V_PK_MAX_U16"}, // 12
                {amdgpu_gfx940_op_V_PK_MIN_U16,"V_PK_MIN_U16"}, // 13
                {amdgpu_gfx940_op_V_PK_FMA_F16,"V_PK_FMA_F16"}, // 14
                {amdgpu_gfx940_op_V_PK_ADD_F16,"V_PK_ADD_F16"}, // 15
                {amdgpu_gfx940_op_V_PK_MUL_F16,"V_PK_MUL_F16"}, // 16
                {amdgpu_gfx940_op_V_PK_MIN_F16,"V_PK_MIN_F16"}, // 17
                {amdgpu_gfx940_op_V_PK_MAX_F16,"V_PK_MAX_F16"}, // 18
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 19
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 20
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 21
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 22
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 23
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 24
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 25
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 26
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 27
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 28
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 29
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 30
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 31
                {amdgpu_gfx940_op_V_MAD_MIX_F32,"V_MAD_MIX_F32"}, // 32
                {amdgpu_gfx940_op_V_MAD_MIXLO_F16,"V_MAD_MIXLO_F16"}, // 33
                {amdgpu_gfx940_op_V_MAD_MIXHI_F16,"V_MAD_MIXHI_F16"}, // 34
                {amdgpu_gfx940_op_V_DOT2_F32_F16,"V_DOT2_F32_F16"}, // 35
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 36
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 37
                {amdgpu_gfx940_op_V_DOT2_I32_I16,"V_DOT2_I32_I16"}, // 38
                {amdgpu_gfx940_op_V_DOT2_U32_U16,"V_DOT2_U32_U16"}, // 39
                {amdgpu_gfx940_op_V_DOT4_I32_I8,"V_DOT4_I32_I8"}, // 40
                {amdgpu_gfx940_op_V_DOT4_U32_U8,"V_DOT4_U32_U8"}, // 41
                {amdgpu_gfx940_op_V_DOT8_I32_I4,"V_DOT8_I32_I4"}, // 42
                {amdgpu_gfx940_op_V_DOT8_U32_U4,"V_DOT8_U32_U4"}, // 43
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 44
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 45
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 46
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 47
                {amdgpu_gfx940_op_V_PK_FMA_F32,"V_PK_FMA_F32"}, // 48
                {amdgpu_gfx940_op_V_PK_MUL_F32,"V_PK_MUL_F32"}, // 49
                {amdgpu_gfx940_op_V_PK_ADD_F32,"V_PK_ADD_F32"}, // 50
                {amdgpu_gfx940_op_V_PK_MOV_B32,"V_PK_MOV_B32"}, // 51
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 52
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 53
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 54
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 55
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 56
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 57
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 58
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 59
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 60
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 61
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 62
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 63
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 64
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 65
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 66
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 67
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 68
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 69
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 70
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 71
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 72
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 73
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 74
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 75
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 76
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 77
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 78
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 79
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 80
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 81
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 82
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 83
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 84
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 85
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 86
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 87
                {amdgpu_gfx940_op_V_ACCVGPR_READ,"V_ACCVGPR_READ"}, // 88
                {amdgpu_gfx940_op_V_ACCVGPR_WRITE,"V_ACCVGPR_WRITE"}, // 89
            }; // end ENC_VOP3P_insn_table
            const amdgpu_gfx940_insn_entry ENC_VOP3P_MFMA_insn_table[128] = 
            {
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 0
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 1
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 2
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 3
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 4
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 5
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 6
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 7
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 8
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 9
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 10
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 11
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 12
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 13
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 14
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 15
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 16
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 17
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 18
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 19
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 20
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 21
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 22
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 23
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 24
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 25
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 26
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 27
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 28
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 29
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 30
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 31
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 32
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 33
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 34
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 35
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 36
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 37
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 38
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 39
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 40
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 41
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 42
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 43
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 44
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 45
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 46
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 47
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 48
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 49
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 50
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 51
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 52
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 53
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 54
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 55
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 56
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 57
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 58
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 59
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 60
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 61
                {amdgpu_gfx940_op_V_MFMA_F32_16X16X8_XF32,"V_MFMA_F32_16X16X8_XF32"}, // 62
                {amdgpu_gfx940_op_V_MFMA_F32_32X32X4_XF32,"V_MFMA_F32_32X32X4_XF32"}, // 63
                {amdgpu_gfx940_op_V_MFMA_F32_32X32X1_2B_F32,"V_MFMA_F32_32X32X1_2B_F32"}, // 64
                {amdgpu_gfx940_op_V_MFMA_F32_16X16X1_4B_F32,"V_MFMA_F32_16X16X1_4B_F32"}, // 65
                {amdgpu_gfx940_op_V_MFMA_F32_4X4X1_16B_F32,"V_MFMA_F32_4X4X1_16B_F32"}, // 66
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 67
                {amdgpu_gfx940_op_V_MFMA_F32_32X32X2_F32,"V_MFMA_F32_32X32X2_F32"}, // 68
                {amdgpu_gfx940_op_V_MFMA_F32_16X16X4_F32,"V_MFMA_F32_16X16X4_F32"}, // 69
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 70
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 71
                {amdgpu_gfx940_op_V_MFMA_F32_32X32X4_2B_F16,"V_MFMA_F32_32X32X4_2B_F16"}, // 72
                {amdgpu_gfx940_op_V_MFMA_F32_16X16X4_4B_F16,"V_MFMA_F32_16X16X4_4B_F16"}, // 73
                {amdgpu_gfx940_op_V_MFMA_F32_4X4X4_16B_F16,"V_MFMA_F32_4X4X4_16B_F16"}, // 74
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 75
                {amdgpu_gfx940_op_V_MFMA_F32_32X32X8_F16,"V_MFMA_F32_32X32X8_F16"}, // 76
                {amdgpu_gfx940_op_V_MFMA_F32_16X16X16_F16,"V_MFMA_F32_16X16X16_F16"}, // 77
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 78
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 79
                {amdgpu_gfx940_op_V_MFMA_I32_32X32X4_2B_I8,"V_MFMA_I32_32X32X4_2B_I8"}, // 80
                {amdgpu_gfx940_op_V_MFMA_I32_16X16X4_4B_I8,"V_MFMA_I32_16X16X4_4B_I8"}, // 81
                {amdgpu_gfx940_op_V_MFMA_I32_4X4X4_16B_I8,"V_MFMA_I32_4X4X4_16B_I8"}, // 82
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 83
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 84
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 85
                {amdgpu_gfx940_op_V_MFMA_I32_32X32X16_I8,"V_MFMA_I32_32X32X16_I8"}, // 86
                {amdgpu_gfx940_op_V_MFMA_I32_16X16X32_I8,"V_MFMA_I32_16X16X32_I8"}, // 87
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 88
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 89
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 90
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 91
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 92
                {amdgpu_gfx940_op_V_MFMA_F32_32X32X4_2B_BF16,"V_MFMA_F32_32X32X4_2B_BF16"}, // 93
                {amdgpu_gfx940_op_V_MFMA_F32_16X16X4_4B_BF16,"V_MFMA_F32_16X16X4_4B_BF16"}, // 94
                {amdgpu_gfx940_op_V_MFMA_F32_4X4X4_16B_BF16,"V_MFMA_F32_4X4X4_16B_BF16"}, // 95
                {amdgpu_gfx940_op_V_MFMA_F32_32X32X8_BF16,"V_MFMA_F32_32X32X8_BF16"}, // 96
                {amdgpu_gfx940_op_V_MFMA_F32_16X16X16_BF16,"V_MFMA_F32_16X16X16_BF16"}, // 97
                {amdgpu_gfx940_op_V_SMFMAC_F32_16X16X32_F16,"V_SMFMAC_F32_16X16X32_F16"}, // 98
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 99
                {amdgpu_gfx940_op_V_SMFMAC_F32_32X32X16_F16,"V_SMFMAC_F32_32X32X16_F16"}, // 100
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 101
                {amdgpu_gfx940_op_V_SMFMAC_F32_16X16X32_BF16,"V_SMFMAC_F32_16X16X32_BF16"}, // 102
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 103
                {amdgpu_gfx940_op_V_SMFMAC_F32_32X32X16_BF16,"V_SMFMAC_F32_32X32X16_BF16"}, // 104
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 105
                {amdgpu_gfx940_op_V_SMFMAC_I32_16X16X64_I8,"V_SMFMAC_I32_16X16X64_I8"}, // 106
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 107
                {amdgpu_gfx940_op_V_SMFMAC_I32_32X32X32_I8,"V_SMFMAC_I32_32X32X32_I8"}, // 108
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 109
                {amdgpu_gfx940_op_V_MFMA_F64_16X16X4_F64,"V_MFMA_F64_16X16X4_F64"}, // 110
                {amdgpu_gfx940_op_V_MFMA_F64_4X4X4_4B_F64,"V_MFMA_F64_4X4X4_4B_F64"}, // 111
                {amdgpu_gfx940_op_V_MFMA_F32_16X16X32_BF8_BF8,"V_MFMA_F32_16X16X32_BF8_BF8"}, // 112
                {amdgpu_gfx940_op_V_MFMA_F32_16X16X32_BF8_FP8,"V_MFMA_F32_16X16X32_BF8_FP8"}, // 113
                {amdgpu_gfx940_op_V_MFMA_F32_16X16X32_FP8_BF8,"V_MFMA_F32_16X16X32_FP8_BF8"}, // 114
                {amdgpu_gfx940_op_V_MFMA_F32_16X16X32_FP8_FP8,"V_MFMA_F32_16X16X32_FP8_FP8"}, // 115
                {amdgpu_gfx940_op_V_MFMA_F32_32X32X16_BF8_BF8,"V_MFMA_F32_32X32X16_BF8_BF8"}, // 116
                {amdgpu_gfx940_op_V_MFMA_F32_32X32X16_BF8_FP8,"V_MFMA_F32_32X32X16_BF8_FP8"}, // 117
                {amdgpu_gfx940_op_V_MFMA_F32_32X32X16_FP8_BF8,"V_MFMA_F32_32X32X16_FP8_BF8"}, // 118
                {amdgpu_gfx940_op_V_MFMA_F32_32X32X16_FP8_FP8,"V_MFMA_F32_32X32X16_FP8_FP8"}, // 119
                {amdgpu_gfx940_op_V_SMFMAC_F32_16X16X64_BF8_BF8,"V_SMFMAC_F32_16X16X64_BF8_BF8"}, // 120
                {amdgpu_gfx940_op_V_SMFMAC_F32_16X16X64_BF8_FP8,"V_SMFMAC_F32_16X16X64_BF8_FP8"}, // 121
                {amdgpu_gfx940_op_V_SMFMAC_F32_16X16X64_FP8_BF8,"V_SMFMAC_F32_16X16X64_FP8_BF8"}, // 122
                {amdgpu_gfx940_op_V_SMFMAC_F32_16X16X64_FP8_FP8,"V_SMFMAC_F32_16X16X64_FP8_FP8"}, // 123
                {amdgpu_gfx940_op_V_SMFMAC_F32_32X32X32_BF8_BF8,"V_SMFMAC_F32_32X32X32_BF8_BF8"}, // 124
                {amdgpu_gfx940_op_V_SMFMAC_F32_32X32X32_BF8_FP8,"V_SMFMAC_F32_32X32X32_BF8_FP8"}, // 125
                {amdgpu_gfx940_op_V_SMFMAC_F32_32X32X32_FP8_BF8,"V_SMFMAC_F32_32X32X32_FP8_BF8"}, // 126
                {amdgpu_gfx940_op_V_SMFMAC_F32_32X32X32_FP8_FP8,"V_SMFMAC_F32_32X32X32_FP8_FP8"}, // 127
            }; // end ENC_VOP3P_MFMA_insn_table
            const amdgpu_gfx940_insn_entry ENC_VOPC_insn_table[256] = 
            {
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 0
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 1
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 2
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 3
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 4
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 5
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 6
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 7
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 8
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 9
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 10
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 11
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 12
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 13
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 14
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 15
                {amdgpu_gfx940_op_V_CMP_CLASS_F32,"V_CMP_CLASS_F32"}, // 16
                {amdgpu_gfx940_op_V_CMPX_CLASS_F32,"V_CMPX_CLASS_F32"}, // 17
                {amdgpu_gfx940_op_V_CMP_CLASS_F64,"V_CMP_CLASS_F64"}, // 18
                {amdgpu_gfx940_op_V_CMPX_CLASS_F64,"V_CMPX_CLASS_F64"}, // 19
                {amdgpu_gfx940_op_V_CMP_CLASS_F16,"V_CMP_CLASS_F16"}, // 20
                {amdgpu_gfx940_op_V_CMPX_CLASS_F16,"V_CMPX_CLASS_F16"}, // 21
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 22
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 23
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 24
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 25
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 26
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 27
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 28
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 29
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 30
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 31
                {amdgpu_gfx940_op_V_CMP_F_F16,"V_CMP_F_F16"}, // 32
                {amdgpu_gfx940_op_V_CMP_LT_F16,"V_CMP_LT_F16"}, // 33
                {amdgpu_gfx940_op_V_CMP_EQ_F16,"V_CMP_EQ_F16"}, // 34
                {amdgpu_gfx940_op_V_CMP_LE_F16,"V_CMP_LE_F16"}, // 35
                {amdgpu_gfx940_op_V_CMP_GT_F16,"V_CMP_GT_F16"}, // 36
                {amdgpu_gfx940_op_V_CMP_LG_F16,"V_CMP_LG_F16"}, // 37
                {amdgpu_gfx940_op_V_CMP_GE_F16,"V_CMP_GE_F16"}, // 38
                {amdgpu_gfx940_op_V_CMP_O_F16,"V_CMP_O_F16"}, // 39
                {amdgpu_gfx940_op_V_CMP_U_F16,"V_CMP_U_F16"}, // 40
                {amdgpu_gfx940_op_V_CMP_NGE_F16,"V_CMP_NGE_F16"}, // 41
                {amdgpu_gfx940_op_V_CMP_NLG_F16,"V_CMP_NLG_F16"}, // 42
                {amdgpu_gfx940_op_V_CMP_NGT_F16,"V_CMP_NGT_F16"}, // 43
                {amdgpu_gfx940_op_V_CMP_NLE_F16,"V_CMP_NLE_F16"}, // 44
                {amdgpu_gfx940_op_V_CMP_NEQ_F16,"V_CMP_NEQ_F16"}, // 45
                {amdgpu_gfx940_op_V_CMP_NLT_F16,"V_CMP_NLT_F16"}, // 46
                {amdgpu_gfx940_op_V_CMP_TRU_F16,"V_CMP_TRU_F16"}, // 47
                {amdgpu_gfx940_op_V_CMPX_F_F16,"V_CMPX_F_F16"}, // 48
                {amdgpu_gfx940_op_V_CMPX_LT_F16,"V_CMPX_LT_F16"}, // 49
                {amdgpu_gfx940_op_V_CMPX_EQ_F16,"V_CMPX_EQ_F16"}, // 50
                {amdgpu_gfx940_op_V_CMPX_LE_F16,"V_CMPX_LE_F16"}, // 51
                {amdgpu_gfx940_op_V_CMPX_GT_F16,"V_CMPX_GT_F16"}, // 52
                {amdgpu_gfx940_op_V_CMPX_LG_F16,"V_CMPX_LG_F16"}, // 53
                {amdgpu_gfx940_op_V_CMPX_GE_F16,"V_CMPX_GE_F16"}, // 54
                {amdgpu_gfx940_op_V_CMPX_O_F16,"V_CMPX_O_F16"}, // 55
                {amdgpu_gfx940_op_V_CMPX_U_F16,"V_CMPX_U_F16"}, // 56
                {amdgpu_gfx940_op_V_CMPX_NGE_F16,"V_CMPX_NGE_F16"}, // 57
                {amdgpu_gfx940_op_V_CMPX_NLG_F16,"V_CMPX_NLG_F16"}, // 58
                {amdgpu_gfx940_op_V_CMPX_NGT_F16,"V_CMPX_NGT_F16"}, // 59
                {amdgpu_gfx940_op_V_CMPX_NLE_F16,"V_CMPX_NLE_F16"}, // 60
                {amdgpu_gfx940_op_V_CMPX_NEQ_F16,"V_CMPX_NEQ_F16"}, // 61
                {amdgpu_gfx940_op_V_CMPX_NLT_F16,"V_CMPX_NLT_F16"}, // 62
                {amdgpu_gfx940_op_V_CMPX_TRU_F16,"V_CMPX_TRU_F16"}, // 63
                {amdgpu_gfx940_op_V_CMP_F_F32,"V_CMP_F_F32"}, // 64
                {amdgpu_gfx940_op_V_CMP_LT_F32,"V_CMP_LT_F32"}, // 65
                {amdgpu_gfx940_op_V_CMP_EQ_F32,"V_CMP_EQ_F32"}, // 66
                {amdgpu_gfx940_op_V_CMP_LE_F32,"V_CMP_LE_F32"}, // 67
                {amdgpu_gfx940_op_V_CMP_GT_F32,"V_CMP_GT_F32"}, // 68
                {amdgpu_gfx940_op_V_CMP_LG_F32,"V_CMP_LG_F32"}, // 69
                {amdgpu_gfx940_op_V_CMP_GE_F32,"V_CMP_GE_F32"}, // 70
                {amdgpu_gfx940_op_V_CMP_O_F32,"V_CMP_O_F32"}, // 71
                {amdgpu_gfx940_op_V_CMP_U_F32,"V_CMP_U_F32"}, // 72
                {amdgpu_gfx940_op_V_CMP_NGE_F32,"V_CMP_NGE_F32"}, // 73
                {amdgpu_gfx940_op_V_CMP_NLG_F32,"V_CMP_NLG_F32"}, // 74
                {amdgpu_gfx940_op_V_CMP_NGT_F32,"V_CMP_NGT_F32"}, // 75
                {amdgpu_gfx940_op_V_CMP_NLE_F32,"V_CMP_NLE_F32"}, // 76
                {amdgpu_gfx940_op_V_CMP_NEQ_F32,"V_CMP_NEQ_F32"}, // 77
                {amdgpu_gfx940_op_V_CMP_NLT_F32,"V_CMP_NLT_F32"}, // 78
                {amdgpu_gfx940_op_V_CMP_TRU_F32,"V_CMP_TRU_F32"}, // 79
                {amdgpu_gfx940_op_V_CMPX_F_F32,"V_CMPX_F_F32"}, // 80
                {amdgpu_gfx940_op_V_CMPX_LT_F32,"V_CMPX_LT_F32"}, // 81
                {amdgpu_gfx940_op_V_CMPX_EQ_F32,"V_CMPX_EQ_F32"}, // 82
                {amdgpu_gfx940_op_V_CMPX_LE_F32,"V_CMPX_LE_F32"}, // 83
                {amdgpu_gfx940_op_V_CMPX_GT_F32,"V_CMPX_GT_F32"}, // 84
                {amdgpu_gfx940_op_V_CMPX_LG_F32,"V_CMPX_LG_F32"}, // 85
                {amdgpu_gfx940_op_V_CMPX_GE_F32,"V_CMPX_GE_F32"}, // 86
                {amdgpu_gfx940_op_V_CMPX_O_F32,"V_CMPX_O_F32"}, // 87
                {amdgpu_gfx940_op_V_CMPX_U_F32,"V_CMPX_U_F32"}, // 88
                {amdgpu_gfx940_op_V_CMPX_NGE_F32,"V_CMPX_NGE_F32"}, // 89
                {amdgpu_gfx940_op_V_CMPX_NLG_F32,"V_CMPX_NLG_F32"}, // 90
                {amdgpu_gfx940_op_V_CMPX_NGT_F32,"V_CMPX_NGT_F32"}, // 91
                {amdgpu_gfx940_op_V_CMPX_NLE_F32,"V_CMPX_NLE_F32"}, // 92
                {amdgpu_gfx940_op_V_CMPX_NEQ_F32,"V_CMPX_NEQ_F32"}, // 93
                {amdgpu_gfx940_op_V_CMPX_NLT_F32,"V_CMPX_NLT_F32"}, // 94
                {amdgpu_gfx940_op_V_CMPX_TRU_F32,"V_CMPX_TRU_F32"}, // 95
                {amdgpu_gfx940_op_V_CMP_F_F64,"V_CMP_F_F64"}, // 96
                {amdgpu_gfx940_op_V_CMP_LT_F64,"V_CMP_LT_F64"}, // 97
                {amdgpu_gfx940_op_V_CMP_EQ_F64,"V_CMP_EQ_F64"}, // 98
                {amdgpu_gfx940_op_V_CMP_LE_F64,"V_CMP_LE_F64"}, // 99
                {amdgpu_gfx940_op_V_CMP_GT_F64,"V_CMP_GT_F64"}, // 100
                {amdgpu_gfx940_op_V_CMP_LG_F64,"V_CMP_LG_F64"}, // 101
                {amdgpu_gfx940_op_V_CMP_GE_F64,"V_CMP_GE_F64"}, // 102
                {amdgpu_gfx940_op_V_CMP_O_F64,"V_CMP_O_F64"}, // 103
                {amdgpu_gfx940_op_V_CMP_U_F64,"V_CMP_U_F64"}, // 104
                {amdgpu_gfx940_op_V_CMP_NGE_F64,"V_CMP_NGE_F64"}, // 105
                {amdgpu_gfx940_op_V_CMP_NLG_F64,"V_CMP_NLG_F64"}, // 106
                {amdgpu_gfx940_op_V_CMP_NGT_F64,"V_CMP_NGT_F64"}, // 107
                {amdgpu_gfx940_op_V_CMP_NLE_F64,"V_CMP_NLE_F64"}, // 108
                {amdgpu_gfx940_op_V_CMP_NEQ_F64,"V_CMP_NEQ_F64"}, // 109
                {amdgpu_gfx940_op_V_CMP_NLT_F64,"V_CMP_NLT_F64"}, // 110
                {amdgpu_gfx940_op_V_CMP_TRU_F64,"V_CMP_TRU_F64"}, // 111
                {amdgpu_gfx940_op_V_CMPX_F_F64,"V_CMPX_F_F64"}, // 112
                {amdgpu_gfx940_op_V_CMPX_LT_F64,"V_CMPX_LT_F64"}, // 113
                {amdgpu_gfx940_op_V_CMPX_EQ_F64,"V_CMPX_EQ_F64"}, // 114
                {amdgpu_gfx940_op_V_CMPX_LE_F64,"V_CMPX_LE_F64"}, // 115
                {amdgpu_gfx940_op_V_CMPX_GT_F64,"V_CMPX_GT_F64"}, // 116
                {amdgpu_gfx940_op_V_CMPX_LG_F64,"V_CMPX_LG_F64"}, // 117
                {amdgpu_gfx940_op_V_CMPX_GE_F64,"V_CMPX_GE_F64"}, // 118
                {amdgpu_gfx940_op_V_CMPX_O_F64,"V_CMPX_O_F64"}, // 119
                {amdgpu_gfx940_op_V_CMPX_U_F64,"V_CMPX_U_F64"}, // 120
                {amdgpu_gfx940_op_V_CMPX_NGE_F64,"V_CMPX_NGE_F64"}, // 121
                {amdgpu_gfx940_op_V_CMPX_NLG_F64,"V_CMPX_NLG_F64"}, // 122
                {amdgpu_gfx940_op_V_CMPX_NGT_F64,"V_CMPX_NGT_F64"}, // 123
                {amdgpu_gfx940_op_V_CMPX_NLE_F64,"V_CMPX_NLE_F64"}, // 124
                {amdgpu_gfx940_op_V_CMPX_NEQ_F64,"V_CMPX_NEQ_F64"}, // 125
                {amdgpu_gfx940_op_V_CMPX_NLT_F64,"V_CMPX_NLT_F64"}, // 126
                {amdgpu_gfx940_op_V_CMPX_TRU_F64,"V_CMPX_TRU_F64"}, // 127
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 128
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 129
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 130
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 131
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 132
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 133
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 134
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 135
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 136
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 137
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 138
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 139
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 140
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 141
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 142
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 143
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 144
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 145
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 146
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 147
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 148
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 149
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 150
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 151
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 152
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 153
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 154
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 155
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 156
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 157
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 158
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 159
                {amdgpu_gfx940_op_V_CMP_F_I16,"V_CMP_F_I16"}, // 160
                {amdgpu_gfx940_op_V_CMP_LT_I16,"V_CMP_LT_I16"}, // 161
                {amdgpu_gfx940_op_V_CMP_EQ_I16,"V_CMP_EQ_I16"}, // 162
                {amdgpu_gfx940_op_V_CMP_LE_I16,"V_CMP_LE_I16"}, // 163
                {amdgpu_gfx940_op_V_CMP_GT_I16,"V_CMP_GT_I16"}, // 164
                {amdgpu_gfx940_op_V_CMP_NE_I16,"V_CMP_NE_I16"}, // 165
                {amdgpu_gfx940_op_V_CMP_GE_I16,"V_CMP_GE_I16"}, // 166
                {amdgpu_gfx940_op_V_CMP_T_I16,"V_CMP_T_I16"}, // 167
                {amdgpu_gfx940_op_V_CMP_F_U16,"V_CMP_F_U16"}, // 168
                {amdgpu_gfx940_op_V_CMP_LT_U16,"V_CMP_LT_U16"}, // 169
                {amdgpu_gfx940_op_V_CMP_EQ_U16,"V_CMP_EQ_U16"}, // 170
                {amdgpu_gfx940_op_V_CMP_LE_U16,"V_CMP_LE_U16"}, // 171
                {amdgpu_gfx940_op_V_CMP_GT_U16,"V_CMP_GT_U16"}, // 172
                {amdgpu_gfx940_op_V_CMP_NE_U16,"V_CMP_NE_U16"}, // 173
                {amdgpu_gfx940_op_V_CMP_GE_U16,"V_CMP_GE_U16"}, // 174
                {amdgpu_gfx940_op_V_CMP_T_U16,"V_CMP_T_U16"}, // 175
                {amdgpu_gfx940_op_V_CMPX_F_I16,"V_CMPX_F_I16"}, // 176
                {amdgpu_gfx940_op_V_CMPX_LT_I16,"V_CMPX_LT_I16"}, // 177
                {amdgpu_gfx940_op_V_CMPX_EQ_I16,"V_CMPX_EQ_I16"}, // 178
                {amdgpu_gfx940_op_V_CMPX_LE_I16,"V_CMPX_LE_I16"}, // 179
                {amdgpu_gfx940_op_V_CMPX_GT_I16,"V_CMPX_GT_I16"}, // 180
                {amdgpu_gfx940_op_V_CMPX_NE_I16,"V_CMPX_NE_I16"}, // 181
                {amdgpu_gfx940_op_V_CMPX_GE_I16,"V_CMPX_GE_I16"}, // 182
                {amdgpu_gfx940_op_V_CMPX_T_I16,"V_CMPX_T_I16"}, // 183
                {amdgpu_gfx940_op_V_CMPX_F_U16,"V_CMPX_F_U16"}, // 184
                {amdgpu_gfx940_op_V_CMPX_LT_U16,"V_CMPX_LT_U16"}, // 185
                {amdgpu_gfx940_op_V_CMPX_EQ_U16,"V_CMPX_EQ_U16"}, // 186
                {amdgpu_gfx940_op_V_CMPX_LE_U16,"V_CMPX_LE_U16"}, // 187
                {amdgpu_gfx940_op_V_CMPX_GT_U16,"V_CMPX_GT_U16"}, // 188
                {amdgpu_gfx940_op_V_CMPX_NE_U16,"V_CMPX_NE_U16"}, // 189
                {amdgpu_gfx940_op_V_CMPX_GE_U16,"V_CMPX_GE_U16"}, // 190
                {amdgpu_gfx940_op_V_CMPX_T_U16,"V_CMPX_T_U16"}, // 191
                {amdgpu_gfx940_op_V_CMP_F_I32,"V_CMP_F_I32"}, // 192
                {amdgpu_gfx940_op_V_CMP_LT_I32,"V_CMP_LT_I32"}, // 193
                {amdgpu_gfx940_op_V_CMP_EQ_I32,"V_CMP_EQ_I32"}, // 194
                {amdgpu_gfx940_op_V_CMP_LE_I32,"V_CMP_LE_I32"}, // 195
                {amdgpu_gfx940_op_V_CMP_GT_I32,"V_CMP_GT_I32"}, // 196
                {amdgpu_gfx940_op_V_CMP_NE_I32,"V_CMP_NE_I32"}, // 197
                {amdgpu_gfx940_op_V_CMP_GE_I32,"V_CMP_GE_I32"}, // 198
                {amdgpu_gfx940_op_V_CMP_T_I32,"V_CMP_T_I32"}, // 199
                {amdgpu_gfx940_op_V_CMP_F_U32,"V_CMP_F_U32"}, // 200
                {amdgpu_gfx940_op_V_CMP_LT_U32,"V_CMP_LT_U32"}, // 201
                {amdgpu_gfx940_op_V_CMP_EQ_U32,"V_CMP_EQ_U32"}, // 202
                {amdgpu_gfx940_op_V_CMP_LE_U32,"V_CMP_LE_U32"}, // 203
                {amdgpu_gfx940_op_V_CMP_GT_U32,"V_CMP_GT_U32"}, // 204
                {amdgpu_gfx940_op_V_CMP_NE_U32,"V_CMP_NE_U32"}, // 205
                {amdgpu_gfx940_op_V_CMP_GE_U32,"V_CMP_GE_U32"}, // 206
                {amdgpu_gfx940_op_V_CMP_T_U32,"V_CMP_T_U32"}, // 207
                {amdgpu_gfx940_op_V_CMPX_F_I32,"V_CMPX_F_I32"}, // 208
                {amdgpu_gfx940_op_V_CMPX_LT_I32,"V_CMPX_LT_I32"}, // 209
                {amdgpu_gfx940_op_V_CMPX_EQ_I32,"V_CMPX_EQ_I32"}, // 210
                {amdgpu_gfx940_op_V_CMPX_LE_I32,"V_CMPX_LE_I32"}, // 211
                {amdgpu_gfx940_op_V_CMPX_GT_I32,"V_CMPX_GT_I32"}, // 212
                {amdgpu_gfx940_op_V_CMPX_NE_I32,"V_CMPX_NE_I32"}, // 213
                {amdgpu_gfx940_op_V_CMPX_GE_I32,"V_CMPX_GE_I32"}, // 214
                {amdgpu_gfx940_op_V_CMPX_T_I32,"V_CMPX_T_I32"}, // 215
                {amdgpu_gfx940_op_V_CMPX_F_U32,"V_CMPX_F_U32"}, // 216
                {amdgpu_gfx940_op_V_CMPX_LT_U32,"V_CMPX_LT_U32"}, // 217
                {amdgpu_gfx940_op_V_CMPX_EQ_U32,"V_CMPX_EQ_U32"}, // 218
                {amdgpu_gfx940_op_V_CMPX_LE_U32,"V_CMPX_LE_U32"}, // 219
                {amdgpu_gfx940_op_V_CMPX_GT_U32,"V_CMPX_GT_U32"}, // 220
                {amdgpu_gfx940_op_V_CMPX_NE_U32,"V_CMPX_NE_U32"}, // 221
                {amdgpu_gfx940_op_V_CMPX_GE_U32,"V_CMPX_GE_U32"}, // 222
                {amdgpu_gfx940_op_V_CMPX_T_U32,"V_CMPX_T_U32"}, // 223
                {amdgpu_gfx940_op_V_CMP_F_I64,"V_CMP_F_I64"}, // 224
                {amdgpu_gfx940_op_V_CMP_LT_I64,"V_CMP_LT_I64"}, // 225
                {amdgpu_gfx940_op_V_CMP_EQ_I64,"V_CMP_EQ_I64"}, // 226
                {amdgpu_gfx940_op_V_CMP_LE_I64,"V_CMP_LE_I64"}, // 227
                {amdgpu_gfx940_op_V_CMP_GT_I64,"V_CMP_GT_I64"}, // 228
                {amdgpu_gfx940_op_V_CMP_NE_I64,"V_CMP_NE_I64"}, // 229
                {amdgpu_gfx940_op_V_CMP_GE_I64,"V_CMP_GE_I64"}, // 230
                {amdgpu_gfx940_op_V_CMP_T_I64,"V_CMP_T_I64"}, // 231
                {amdgpu_gfx940_op_V_CMP_F_U64,"V_CMP_F_U64"}, // 232
                {amdgpu_gfx940_op_V_CMP_LT_U64,"V_CMP_LT_U64"}, // 233
                {amdgpu_gfx940_op_V_CMP_EQ_U64,"V_CMP_EQ_U64"}, // 234
                {amdgpu_gfx940_op_V_CMP_LE_U64,"V_CMP_LE_U64"}, // 235
                {amdgpu_gfx940_op_V_CMP_GT_U64,"V_CMP_GT_U64"}, // 236
                {amdgpu_gfx940_op_V_CMP_NE_U64,"V_CMP_NE_U64"}, // 237
                {amdgpu_gfx940_op_V_CMP_GE_U64,"V_CMP_GE_U64"}, // 238
                {amdgpu_gfx940_op_V_CMP_T_U64,"V_CMP_T_U64"}, // 239
                {amdgpu_gfx940_op_V_CMPX_F_I64,"V_CMPX_F_I64"}, // 240
                {amdgpu_gfx940_op_V_CMPX_LT_I64,"V_CMPX_LT_I64"}, // 241
                {amdgpu_gfx940_op_V_CMPX_EQ_I64,"V_CMPX_EQ_I64"}, // 242
                {amdgpu_gfx940_op_V_CMPX_LE_I64,"V_CMPX_LE_I64"}, // 243
                {amdgpu_gfx940_op_V_CMPX_GT_I64,"V_CMPX_GT_I64"}, // 244
                {amdgpu_gfx940_op_V_CMPX_NE_I64,"V_CMPX_NE_I64"}, // 245
                {amdgpu_gfx940_op_V_CMPX_GE_I64,"V_CMPX_GE_I64"}, // 246
                {amdgpu_gfx940_op_V_CMPX_T_I64,"V_CMPX_T_I64"}, // 247
                {amdgpu_gfx940_op_V_CMPX_F_U64,"V_CMPX_F_U64"}, // 248
                {amdgpu_gfx940_op_V_CMPX_LT_U64,"V_CMPX_LT_U64"}, // 249
                {amdgpu_gfx940_op_V_CMPX_EQ_U64,"V_CMPX_EQ_U64"}, // 250
                {amdgpu_gfx940_op_V_CMPX_LE_U64,"V_CMPX_LE_U64"}, // 251
                {amdgpu_gfx940_op_V_CMPX_GT_U64,"V_CMPX_GT_U64"}, // 252
                {amdgpu_gfx940_op_V_CMPX_NE_U64,"V_CMPX_NE_U64"}, // 253
                {amdgpu_gfx940_op_V_CMPX_GE_U64,"V_CMPX_GE_U64"}, // 254
                {amdgpu_gfx940_op_V_CMPX_T_U64,"V_CMPX_T_U64"}, // 255
            }; // end ENC_VOPC_insn_table
            const amdgpu_gfx940_insn_entry SOPK_INST_LITERAL__insn_table[1] = 
            {
                {amdgpu_gfx940_op_S_NOP,"S_NOP"}, // 0
            }; // end SOPK_INST_LITERAL__insn_table


        };

    }
}
#endif //INSTRUCTION_DECODER_GFX940_H
