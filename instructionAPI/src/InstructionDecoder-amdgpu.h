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

#include "InstructionDecoderImpl.h"
#include "RegisterPair.h"
#include <iostream>
#include "Immediate.h"
#include "dyn_regs.h"

namespace Dyninst {
    namespace InstructionAPI {

#if defined(__GNUC__)
#define insn_printf(format, ...) \
        do{ \
            printf("[%s:%u]insn_debug " format, FILE__, __LINE__, ## __VA_ARGS__); \
        }while(0)
#endif

        struct amdgpu_insn_entry;
        struct amdgpu_mask_entry;

        class InstructionDecoder_amdgpu : public InstructionDecoderImpl {
            friend struct amdgpu_insn_entry;
            friend struct amdgpu_mask_entry;
            enum DecodeFamily {sopp};

        public:
            InstructionDecoder_amdgpu(Architecture a);

            virtual ~InstructionDecoder_amdgpu();

            virtual void decodeOpcode(InstructionDecoder::buffer &b);

            virtual Instruction decode(InstructionDecoder::buffer &b);

            virtual void setMode(bool) { }

            virtual bool decodeOperands(const Instruction *insn_to_complete);
            virtual bool decodeOperands(const Instruction *insn_to_complete, const amdgpu_insn_entry & insn_entry);

            virtual void doDelayedDecode(const Instruction *insn_to_complete);

            static const std::array<std::string, 16> condNames;
            static MachRegister sysRegMap(unsigned int);
            static const char* bitfieldInsnAliasMap(entryID);
            static const char* condInsnAliasMap(entryID);

     

        private:
            virtual Result_Type makeSizeType(unsigned int opType);

            bool isPstateRead, isPstateWritten;
            bool isFPInsn, isSIMDInsn;
	        bool skipRn, skipRm;
            bool is64Bit;
            bool isValid;
            unsigned int insn_size;

            void mainDecode(InstructionDecoder::buffer &b);

            int findInsnTableIndex(unsigned int);

            /*members for handling operand re-ordering, will be removed later once a generic operand ordering method is incorporated*/
            int oprRotateAmt;
            bool hasb5;

            void reorderOperands();

            unsigned int insn;
            unsigned long long int insn_long;
            unsigned int insn_high;
            unsigned int imm32;
            unsigned int imm64;

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
                return (raw >> (start) & (0xFFFFFFFFFFFFFFFF >> (63 - (end - start))));
            }
 
            template<int start, int end>
            int rev_field(unsigned int raw) {
#if defined DEBUG_FIELD
                std::cerr << start << "-" << end << ":" << std::dec << (raw >> (start) &
                        (0xFFFFFFFF >> (31 - (end - start)))) << " ";
#endif
                unsigned int le = (raw >> (start) & (0xFFFFFFFF >> (31 - (end - start))));

                std::cerr << "operating on le " << std::hex << le << std::endl;
                unsigned int be = __builtin_bswap32(le);

                std::cerr << "operating on be " << std::hex << be << std::endl;
                return be >> (31 - (end-start));
            }


            int32_t sign_extend32(int size, int in) {
                int32_t val = 0 | in;

                return (val << (32 - size)) >> (32 - size);
            }

            int64_t sign_extend64(int size, int in) {
                int64_t val = 0 | in;

                return (val << (64 - size)) >> (64 - size);
            }

            uint32_t unsign_extend32(int size, int in) {
                uint32_t mask = ~0;

                return (mask >> (32 - size)) & in;
            }

            uint64_t unsign_extend64(int size, int in) {
                uint64_t mask = ~0;

                return (mask >> (64 - size)) & in;
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

            int op1Field, op2Field, crmField;

            void processSystemInsn();

            bool hasHw;
            int hwField;

            void processHwFieldInsn(int, int);

            bool hasShift;
            int shiftField;

            void processShiftFieldShiftedInsn(int, int);

            void processShiftFieldImmInsn(int, int);

            bool hasOption;
            int optionField;

            void processOptionFieldLSRegOffsetInsn();

            bool hasN;
            int immr, immrLen;
            int sField, nField, nLen;

            int immlo, immloLen;

            void makeBranchTarget(bool, bool, int, int);

            Expression::Ptr makeFallThroughExpr();

            int _szField, size;
            int _typeField;
            int cmode;
            int op;
            int simdAlphabetImm;

            void processAlphabetImm();

            void NOTHING();

            void set32Mode();

            void setRegWidth();

            void setFPMode();

            void setSIMDMode();

            bool isSinglePrec();

            bool fix_bitfieldinsn_alias(int, int);
	        void fix_condinsn_alias_and_cond(int &);
	        void modify_mnemonic_simd_upperhalf_insns();
            bool pre_process_checks(const amdgpu_insn_entry &);

            MachRegister makeAmdgpuRegID(MachRegister, unsigned int, unsigned int len = 1);

            MachRegister getLoadStoreSimdRegister(int encoding);

            Expression::Ptr makeRdExpr();

            Expression::Ptr makeRnExpr();

            Expression::Ptr makeRmExpr();

            Expression::Ptr makeRaExpr();

            Expression::Ptr makeRsExpr();

            Expression::Ptr makePstateExpr();

            Expression::Ptr makePCExpr();

            Expression::Ptr makeb40Expr();

            Expression::Ptr makeOptionExpression(int, int);

            template<typename T, Result_Type rT>
            Expression::Ptr fpExpand(int);

            Expression::Ptr makeRtExpr();

            Expression::Ptr makeRt2Expr();

            Expression::Ptr makeMemRefReg();

            Expression::Ptr makeMemRefReg_Rm();

            Expression::Ptr makeMemRefReg_ext();

            Expression::Ptr makeMemRefReg_amount();

            Expression::Ptr makeMemRefIndexLiteral();

            Expression::Ptr makeMemRefIndexUImm();

            Expression::Ptr makeMemRefIndexPre();

            Expression::Ptr makeMemRefIndexPost();

            Expression::Ptr makeMemRefIndex_addOffset9();

            Expression::Ptr makeMemRefIndex_offset9();

            Expression::Ptr makeMemRefPairPre();

            Expression::Ptr makeMemRefPairPost();

            Expression::Ptr makeMemRefPair_offset7();

            Expression::Ptr makeMemRefPair_addOffset7();

            Expression::Ptr makeMemRefEx();

            Expression::Ptr makeMemRefExPair();

            Expression::Ptr makeMemRefExPair2();

            Expression::Ptr makeMemRefSIMD_MULT();

            Expression::Ptr makeMemRefSIMD_SING();

            template<typename T>
            Expression::Ptr makeLogicalImm(int immr, int imms, int immsLen, Result_Type rT);


            void getMemRefIndexLiteral_OffsetLen(int &, int &);

            void getMemRefIndex_SizeSizelen(unsigned int &, unsigned int &);

            void getMemRefIndexPrePost_ImmImmlen(unsigned int &, unsigned int &);

            void getMemRefPair_ImmImmlen(unsigned int &immVal, unsigned int &immLen);

            void getMemRefEx_RT(Result_Type &rt);

            void getMemRefIndexLiteral_RT(Result_Type &);

            void getMemRefExPair_RT(Result_Type &rt);

            void getMemRefPair_RT(Result_Type &rt);

            void getMemRefIndex_RT(Result_Type &);

            void getMemRefIndexUImm_RT(Result_Type &);

            unsigned int getMemRefSIMD_MULT_T();

            unsigned int getMemRefSIMD_SING_T();

            void getMemRefSIMD_MULT_RT(Result_Type &);

            void getMemRefSIMD_SING_RT(Result_Type &);

            unsigned int get_SIMD_MULT_POST_imm();

            unsigned int get_SIMD_SING_POST_imm();

            void OPRRd();

            void OPRsf();

            template<unsigned int endBit, unsigned int startBit>
            void OPRoption();

            void OPRshift();

            void OPRhw();

            template<unsigned int endBit, unsigned int startBit>
            void OPRN();

            //for load store
            void LIndex();

            void STIndex();

            void OPRRn();

            void OPRRnL();

            void OPRRnLU();

            void OPRRnSU();

            void OPRRnS();

            void OPRRnU();

            void OPRRm();

            void OPRRt();

            void OPRRtL();

            void OPRRtS();

            void OPRRt2();

            void OPRRt2L();

            void OPRRt2S();

            void OPRop1();

            void OPRop2();

            template<unsigned int endBit, unsigned int startBit>
            void OPRcond();

            void OPRnzcv();

            void OPRCRm();

            void OPRCRn();

            template<unsigned int endBit, unsigned int startBit>
            void OPRS();

            void OPRRa();

            void OPRo0();

            void OPRb5();

            void OPRb40();

            template<unsigned int endBit, unsigned int startBit>
            void OPRsz();

            void OPRRs();

            template<unsigned int endBit, unsigned int startBit>
            void OPRimm();

            void OPRscale();

            template<unsigned int endBit, unsigned int startBit>
            void OPRtype();

            void OPRQ();

            void OPRL();

            //void OPRR();
            void OPRH() { }

            void OPRM() { }

            void OPRa();

            void OPRb();

            void OPRc();

            void OPRd();

            void OPRe();

            void OPRf();

            void OPRg();

            void OPRh();

            void OPRopc();

            void OPRopcode() { }

            void OPRlen();

            template<unsigned int endBit, unsigned int startBit>
            void OPRsize();

            void OPRcmode();

            void OPRrmode() { }

            void OPRop();

            void setFlags();

            unsigned int _Q;
            unsigned int _L;
            unsigned int _R;

            void getSIMD_MULT_RptSelem(unsigned int &rpt, unsigned int &selem);

            unsigned int getSIMD_SING_selem();

            // STARTING HERE IS THE MY IMPLEMENTATION FOR AMDGPU
            template<unsigned int endBit, unsigned int startBit>
            void OPRssrc();

            template<unsigned int endBit, unsigned int startBit>
            void OPRsdst();

            void insnSize(unsigned int insn_size );
            
            Expression::Ptr decodeSSRC(InstructionDecoder::buffer & b , unsigned int index);
            Expression::Ptr decodeVSRC(InstructionDecoder::buffer & b , unsigned int index);
            
            Expression::Ptr makeRegisterPairExpr(MachRegister & baseReg,unsigned int index,  unsigned length);
            Expression::Ptr decodeSGPRorM0(unsigned int offset);
            
            void decodeFLATOperands();
            void decodeSMEMOperands();
            void decodeMUBUFOperands();
            void decodeMTBUFOperands();
            void decodeSOP2Operands();
            void decodeSOP1Operands();
            void decodeVOP1Operands();
            bool useImm;
            bool setSCC;

#define IS_LD_ST() (isLoad || isStore )

            bool isSMEM;
            bool isLoad;
            bool isStore;
            bool isBuffer;
            bool isScratch;
            void setSMEM() {isSMEM = true;};
            unsigned int num_elements ; 
            
            template<unsigned int num_elements>
            void setLoad(){isLoad = true; this->num_elements = num_elements; };
            
            template<unsigned int num_elements>
            void setStore() {isStore = true;this->num_elements = num_elements;};

            void setScratch() {isScratch = true;};

            void setBuffer() {isBuffer = true;};
            
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
        
#include "amdgpu_decoder_impl_vega.h"
        };
    }
}
