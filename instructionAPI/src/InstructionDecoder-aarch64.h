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

#include <stdint.h>
#include <string>
#include <array>
#include "InstructionDecoderImpl.h"
#include <iostream>
#include "Immediate.h"
#include "registers/MachRegister.h"

namespace Dyninst {
    namespace InstructionAPI {

#if defined(__GNUC__)
#define insn_printf(format, ...) \
        do{ \
            printf("[%s:%u]insn_debug " format, FILE__, __LINE__, ## __VA_ARGS__); \
        }while(0)
#endif

#define AARCH64_INSN_LENGTH    32

        struct aarch64_insn_entry;
        struct aarch64_mask_entry;

        class InstructionDecoder_aarch64 : public InstructionDecoderImpl {
            friend struct aarch64_insn_entry;
            friend struct aarch64_mask_entry;

        public:
            InstructionDecoder_aarch64(Architecture a);

            virtual ~InstructionDecoder_aarch64();

            virtual void decodeOpcode(InstructionDecoder::buffer &b);

            virtual Instruction decode(InstructionDecoder::buffer &b);

            virtual void setMode(bool) { }

            virtual bool decodeOperands(const Instruction *insn_to_complete);

            virtual void doDelayedDecode(const Instruction *insn_to_complete);

            static const std::array<std::string, 16> condNames;
            static MachRegister sysRegMap(unsigned int);
            static const char* bitfieldInsnAliasMap(entryID);
            static const char* condInsnAliasMap(entryID);

#define    IS_INSN_LOGICAL_SHIFT(I)        (field<24, 28>(I) == 0x0A)
#define    IS_INSN_ADDSUB_EXT(I)            (field<24, 28>(I) == 0x0B && field<21, 21>(I) == 1)
#define    IS_INSN_ADDSUB_SHIFT(I)            (field<24, 28>(I) == 0x0B && field<21, 21>(I) == 0)
#define    IS_INSN_ADDSUB_IMM(I)            (field<24, 28>(I) == 0x11)

            //----ld/st-----
#define IS_INSN_LDST(I)                 (field<25, 25>(I) == 0x00 && field<27, 27>(I) == 1)

#define IS_INSN_LDST_EX(I)              (field<24, 29>(I) == 0x08)
#define IS_INSN_ST_EX(I)                (field<24, 29>(I) == 0x08 && field<22, 22>(I) == 0)
#define IS_INSN_LDST_EX_PAIR(I)         (field<24, 29>(I) == 0x08 && field<21, 21>(I) ==0x01)

#define IS_INSN_LD_LITERAL(I)           (field<27, 29>(I) == 0x03 && field<24, 25>(I) == 0)

#define IS_INSN_LDST_PAIR(I)            (field<27, 29>(I) == 0x05 && field<25, 25>(I) == 0)
#define IS_INSN_LDST_PAIR_PRE(I)        (field<27, 29>(I) == 0x05 && field<23, 25>(I) == 0x03)
#define IS_INSN_LDST_PAIR_OFFSET(I)     (field<27, 29>(I) == 0x05 && field<23, 25>(I) == 0x02)
#define IS_INSN_LDST_PAIR_POST(I)       (field<27, 29>(I) == 0x05 && field<23, 25>(I) == 0x01)
#define IS_INSN_LDST_PAIR_NOALLOC(I)    (field<27, 29>(I) == 0x05 && field<23, 25>(I) == 0x00)

#define IS_INSN_LDST_UNSCALED(I)        (field<27, 29>(I) == 0x07 && field<24, 25>(I) == 0 && field<21, 21>(I) == 0 && field<10, 11>(I) == 0x00)
#define IS_INSN_LDST_POST(I)            (field<27, 29>(I) == 0x07 && field<24, 25>(I) == 0 && field<21, 21>(I) == 0 && field<10, 11>(I) == 0x01)
#define IS_INSN_LDST_UNPRIV(I)          (field<27, 29>(I) == 0x07 && field<24, 25>(I) == 0 && field<21, 21>(I) == 0 && field<10, 11>(I) == 0x02)
#define IS_INSN_LDST_PRE(I)             (field<27, 29>(I) == 0x07 && field<24, 25>(I) == 0 && field<21, 21>(I) == 0 && field<10, 11>(I) == 0x03)
#define    IS_INSN_LDST_REG(I)                (field<27, 29>(I) == 0x07 && field<24, 25>(I) == 0 && field<21, 21>(I) == 1 && field<10, 11>(I) == 0x02)
#define IS_INSN_LDST_UIMM(I)            (field<27, 29>(I) == 0x07 && field<24, 25>(I) == 1)

#define IS_INSN_LDST_SIMD_MULT(I)       (field<31, 31>(I) == 0x00 && field<23, 29>(I) == 0x18 && field<16, 21>(I) == 0)
#define IS_INSN_LDST_SIMD_MULT_POST(I)  (field<31, 31>(I) == 0x00 && field<23, 29>(I) == 0x19 && field<21, 21>(I) == 0)
#define IS_INSN_LDST_SIMD_SING(I)       (field<31, 31>(I) == 0x00 && field<23, 29>(I) == 0x1a && field<16, 20>(I) == 0)
#define IS_INSN_LDST_SIMD_SING_POST(I)  (field<31, 31>(I) == 0x00 && field<23, 29>(I) == 0x1b)

#define    IS_INSN_LOGICAL_IMM(I)            (field<23, 28>(I) == 0x24)
#define    IS_INSN_BITFIELD(I)                (field<23, 28>(I) == 0x26)
#define    IS_INSN_EXCEPTION(I)            (field<24, 31>(I) == 0xD4)
#define    IS_INSN_FP_COMPARE(I)            (field<24, 28>(I) == 0x1E && field<21, 21>(I) == 0x1 && field<10, 13>(I) == 0x8)
#define    IS_INSN_FP_CONV_FIX(I)            (field<24, 28>(I) == 0x1E && field<21, 21>(I) == 0x0)
#define    IS_INSN_FP_CONV_INT(I)            (field<24, 28>(I) == 0x1E && field<21, 21>(I) == 0x1 && field<10, 15>(I) == 0x0)
#define    IS_SOURCE_GP(I)                    (field<16, 18>(I) == 0x2  || field<16, 18>(I) == 0x3 || field<16, 18>(I) == 0x7)
#define    IS_INSN_FP_DATAPROC_ONESRC(I)    (field<24, 28>(I) == 0x1E && field<21, 21>(I) == 0x1 && field<10, 14>(I) == 0x10)
#define    IS_INSN_B_UNCOND(I)                (field<26, 30>(I) == 0x05)
#define    IS_INSN_B_UNCOND_REG(I)            (field<25, 31>(I) == 0x6B)
#define    IS_INSN_B_COMPARE(I)            (field<25, 30>(I) == 0x1A)
#define	   IS_INSN_COND_SELECT(I)	    (field<21, 28>(I) == 0xD4)
#define    IS_INSN_B_TEST(I)                (field<25, 30>(I) == 0x1B)
#define    IS_INSN_B_COND(I)                (field<25, 31>(I) == 0x2A)
#define    IS_INSN_PCREL_ADDR(I)            (field<24, 28>(I) == 0x10)
#define    IS_INSN_SYSTEM(I)                (field<22, 31>(I) == 0x354)
#define    IS_INSN_BRANCHING(I)            (IS_INSN_B_COND(I) || IS_INSN_B_UNCOND(I) || IS_INSN_B_UNCOND_REG(I) || IS_INSN_B_TEST(I)|| IS_INSN_B_COMPARE(I))

#define IS_INSN_SIMD_3SAME(I)           (field<31, 31>(I) == 0x0 && field<24, 28>(I) == 0xE && field<21, 21>(I) == 0x1 && field<10, 10>(I) == 0x1)
#define IS_INSN_SIMD_3DIFF(I)           (field<31, 31>(I) == 0x0 && field<24, 28>(I) == 0xe && field<21, 21>(I) == 0x1 && field<10, 11>(I) == 0x0)
#define IS_INSN_SIMD_2REG_MISC(I)	(field<31, 31>(I) == 0x0 && field<24, 28>(I) == 0xe && field<17, 21>(I) == 0x10 && field<10, 11>(I) == 0x2)
#define IS_INSN_SIMD_ACROSS(I)          (field<31, 31>(I) == 0x0 && field<24, 28>(I) == 0xe && field<17, 21>(I) == 0x18 && field<10, 11>(I) == 0x2)
#define IS_INSN_SIMD_COPY(I)            (field<31, 31>(I) == 0x0 && field<21, 28>(I) == 0x70 && field<15,15>(I) == 0x0 && field<10, 10>(I) == 0x1)
#define IS_INSN_SIMD_VEC_INDEX(I)       (field<31, 31>(I) == 0x0 && field<24, 28>(I) == 0xf && field<10, 10>(I) == 0x0)
#define IS_INSN_SIMD_MOD_IMM(I)         (field<31, 31>(I) == 0x0 && field<19, 28>(I) == 0x1e0 && field<10, 10>(I) == 0x1)
#define IS_INSN_SIMD_SHIFT_IMM(I)       (field<31, 31>(I) == 0x0 && field<23, 28>(I) == 0x1e && field<19, 22>(I) != 0x0 && field<10, 10>(I) == 0x1)
#define IS_INSN_SIMD_TAB_LOOKUP(I)      (field<31, 31>(I) == 0x0 && field<24, 29>(I) == 0xe && field<21, 21>(I) == 0x0 && field<15, 15>(I) == 0x0 && field<10, 11>(I) == 0x0)
#define IS_INSN_SIMD_EXTR(I)            (field<31, 31>(I) == 0x0 && field<24, 29>(I) == 0x2e && field<21, 21>(I) == 0x0 && field<15, 15>(I) == 0x0 && field<10, 10>(I) == 0x0)
#define IS_INSN_SCALAR_3SAME(I)         (field<30, 31>(I) == 0x1 && field<24, 28>(I) == 0x1e && field<21, 21>(I) == 0x1 && field<10, 10>(I) == 0x1)
#define IS_INSN_SCALAR_3DIFF(I)         (field<30, 31>(I) == 0x1 && field<24, 28>(I) == 0x1e && field<21, 21>(I) == 0x1 && field<10, 11>(I) == 0x0)
#define IS_INSN_SCALAR_2REG_MISC(I)     (field<30, 31>(I) == 0x1 && field<24, 28>(I) == 0x1e && field<17, 21>(I) == 0x10 && field<10, 11>(I) == 0x2)
#define IS_INSN_SCALAR_PAIR(I)          (field<30, 31>(I) == 0x1 && field<24, 28>(I) == 0x1e && field<17, 21>(I) == 0x18 && field<10, 11>(I) == 0x2)
#define IS_INSN_SCALAR_COPY(I)          (field<30, 31>(I) == 0x1 && field<21, 28>(I) == 0xf0 && field<15, 15>(I) == 0x0 && field<10, 10>(I) == 0x1)
#define IS_INSN_SCALAR_INDEX(I)         (field<30, 31>(I) == 0x1 && field<24, 28>(I) == 0x1f && field<10, 10>(I) == 0x0)
#define IS_INSN_SCALAR_SHIFT_IMM(I)     (field<30, 31>(I) == 0x1 && field<23, 28>(I) == 0x3e && field<10, 10>(I) == 0x1)

#define IS_INSN_CRYPT_3REG_SHA(I)       (field<24, 31>(I) == 0x5E && field<21, 21>(I) == 0 && field<15, 15>(I) == 0 && field<10, 11>(I) == 0)
#define IS_INSN_CRYPT_2REG_SHA(I)       (field<24, 31>(I) == 0x5E && field<17, 21>(I) == 0x14 && field<10, 11>(I) == 0x2)

#define    IS_FIELD_IMMR(S, E)                (S == 16 && E == 21)
#define    IS_FIELD_IMMS(S, E)                (S == 10 && E == 15)
#define    IS_FIELD_IMMLO(S, E)            (S == 29 && E == 30)
#define    IS_FIELD_IMMHI(S, E)            (S == 5 && E == 23)

        private:
            virtual Result_Type makeSizeType(unsigned int opType);

            bool isPstateRead{}, isPstateWritten{};
            bool isFPInsn{}, isSIMDInsn{};
	    bool skipRn{}, skipRm{};
            bool is64Bit{};
            bool isValid{};

            void mainDecode();

            int findInsnTableIndex(unsigned int);

            /*members for handling operand re-ordering, will be removed later once a generic operand ordering method is incorporated*/
            int oprRotateAmt{};
            bool hasb5{};

            void reorderOperands();

            unsigned int insn{};
            boost::shared_ptr<Instruction> insn_in_progress;

            template<int start, int end>
            int field(unsigned int raw) {
#if defined DEBUG_FIELD
                std::cerr << start << "-" << end << ":" << std::dec << (raw >> (start) &
                        (0xFFFFFFFF >> (31 - (end - start)))) << " ";
#endif
                return (raw >> (start) & (0xFFFFFFFF >> (31 - (end - start))));
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

            int op1Field{}, op2Field{}, crmField{};

            void processSystemInsn();

            bool hasHw{};
            int hwField{};

            void processHwFieldInsn(int, int);

            bool hasShift{};
            int shiftField{};

            void processShiftFieldShiftedInsn(int, int);

            void processShiftFieldImmInsn(int, int);

            bool hasOption{};
            int optionField{};

            void processOptionFieldLSRegOffsetInsn();

            bool hasN{};
            int immr, immrLen{};
            int sField{}, nField{}, nLen{};

            int immlo{}, immloLen{};

            void makeBranchTarget(bool, bool, int, int);

            Expression::Ptr makeFallThroughExpr();

            int _szField{}, size{};
            int _typeField{};
            int cmode{};
            int op{};
            int simdAlphabetImm{};

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
            bool pre_process_checks(const aarch64_insn_entry &);

            MachRegister makeAarch64RegID(MachRegister, unsigned int);

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

            unsigned int _Q{};
            unsigned int _L{};
            unsigned int _R{};

            void getSIMD_MULT_RptSelem(unsigned int &rpt, unsigned int &selem);

            unsigned int getSIMD_SING_selem();
        };
    }
}
