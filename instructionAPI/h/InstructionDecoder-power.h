/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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


#include "InstructionDecoder.h"
#include <iostream>

//#define DEBUG_FIELD

namespace Dyninst {
    namespace InstructionAPI
    {
        struct power_entry;

        enum power_RegisterIDs
        {
            power_GPR0 = 1000,
            power_GPR1,
            power_GPR2,
            power_GPR3,
            power_GPR4,
            power_GPR5,
            power_GPR6,
            power_GPR7,
            power_GPR8,
            power_GPR9,
            power_GPR10,
            power_GPR11,
            power_GPR12,
            power_GPR13,
            power_GPR14,
            power_GPR15,
            power_GPR16,
            power_GPR17,
            power_GPR18,
            power_GPR19,
            power_GPR20,
            power_GPR21,
            power_GPR22,
            power_GPR23,
            power_GPR24,
            power_GPR25,
            power_GPR26,
            power_GPR27,
            power_GPR28,
            power_GPR29,
            power_GPR30,
            power_GPR31,
            power_FPR0 = 1100,
            power_FPR1,
            power_FPR2,
            power_FPR3,
            power_FPR4,
            power_FPR5,
            power_FPR6,
            power_FPR7,
            power_FPR8,
            power_FPR9,
            power_FPR10,
            power_FPR11,
            power_FPR12,
            power_FPR13,
            power_FPR14,
            power_FPR15,
            power_FPR16,
            power_FPR17,
            power_FPR18,
            power_FPR19,
            power_FPR20,
            power_FPR21,
            power_FPR22,
            power_FPR23,
            power_FPR24,
            power_FPR25,
            power_FPR26,
            power_FPR27,
            power_FPR28,
            power_FPR29,
            power_FPR30,
            power_FPR31,
            power_FSR0 = 1200,
            power_FSR1,
            power_FSR2,
            power_FSR3,
            power_FSR4,
            power_FSR5,
            power_FSR6,
            power_FSR7,
            power_FSR8,
            power_FSR9,
            power_FSR10,
            power_FSR11,
            power_FSR12,
            power_FSR13,
            power_FSR14,
            power_FSR15,
            power_FSR16,
            power_FSR17,
            power_FSR18,
            power_FSR19,
            power_FSR20,
            power_FSR21,
            power_FSR22,
            power_FSR23,
            power_FSR24,
            power_FSR25,
            power_FSR26,
            power_FSR27,
            power_FSR28,
            power_FSR29,
            power_FSR30,
            power_FSR31,
            power_MQ = 1300,
            power_XER = 1301,
            power_LR = 1308,
            power_CTR = 1309,
            power_DSISR = 1318,
            power_DAR = 1319,
            power_DEC = 1322,
            power_SDR1 = 1325,
            power_SRR0,
            power_SRR1,
            power_SPRG0 = 1572,
            power_SPRG1,
            power_SPRG2,
            power_SPRG3,
            power_EAR = 1582,
            power_TBL = 1584,
            power_TBU = 1585,
            power_PVR = 1587,
            power_SEG0 = 1600,
            power_SEG1,
            power_SEG2,
            power_SEG3,
            power_SEG4,
            power_SEG5,
            power_SEG6,
            power_SEG7,
            power_CR0 = 1700,
            power_CR1,
            power_CR2,
            power_CR3,
            power_CR4,
            power_CR5,
            power_CR6,
            power_CR7,
            power_IBAT0U = 1828,
            power_IBAT0L,
            power_IBAT1U,
            power_IBAT1L,
            power_IBAT2U,
            power_IBAT2L,
            power_IBAT3U,
            power_IBAT3L,
            power_DBAT0U,
            power_DBAT0L,
            power_DBAT1U,
            power_DBAT1L,
            power_DBAT2U,
            power_DBAT2L,
            power_DBAT3U,
            power_DBAT3L,
            power_R_PC,            
            power_FPSCW,
            power_FPSCW0,
            power_FPSCW1,
            power_FPSCW2,
            power_FPSCW3,
            power_FPSCW4,
            power_FPSCW5,
            power_FPSCW6,
            power_FPSCW7,
            power_DUMMYSPR,
            power_MSR,
            power_IVPR,
            power_IVOR8,
            power_LAST_REG
        };
        enum powerXERBits
        {
            OF_bit,
            SOF_bit,
            CF_bit,
            BYTECNT_low_bit,
            BYTECNT_high_bit,
            last_power_xer_bit_id
        };
        class InstructionDecoder_power : public InstructionDecoder
        {

            enum registerBank_power
            {
                bank_gpr = 1000,
                bank_fpr = 1100,
                bank_fsr = 1200,
                bank_spr = 1300,
                bank_seg = 1600,
                bank_cond = 1700
            };
            friend struct power_entry;
            public:
                InstructionDecoder_power(const unsigned char* buffer, unsigned int length);
                virtual ~InstructionDecoder_power();
                virtual unsigned int decodeOpcode();
                virtual Instruction::Ptr decode();
                virtual Instruction::Ptr decode(const unsigned char* buffer);
                virtual void setMode(bool is64);
                virtual bool decodeOperands(const Instruction* insn_to_complete);
                virtual void doDelayedDecode(const Instruction* insn_to_complete);
            private:
                virtual Result_Type makeSizeType(unsigned int opType);
                Expression::Ptr makeMemRefIndex(Result_Type size);
                Expression::Ptr makeMemRefNonIndex(Result_Type size);
                Expression::Ptr makeRAExpr();
                Expression::Ptr makeRBExpr();
                Expression::Ptr makeRTExpr(); 
                Expression::Ptr makeFRAExpr();
                Expression::Ptr makeFRBExpr();
                Expression::Ptr makeFRCExpr();
                Expression::Ptr makeFRTExpr();
                Expression::Ptr makeIFormBranchTarget();
                Expression::Ptr makeBFormBranchTarget();
                Expression::Ptr makeRAorZeroExpr();
                Expression::Ptr makeDorSIExpr();
                Expression::Ptr makeBTExpr();
                Expression::Ptr makeBAExpr();
                Expression::Ptr makeBBExpr();
                Expression::Ptr makeCR0Expr();
                Expression::Ptr makeBIExpr();
                Expression::Ptr makeSHExpr();
                Expression::Ptr makeMBExpr();
                Expression::Ptr makeMEExpr();
                Expression::Ptr makeFLMExpr();
                Expression::Ptr makeSPRExpr();
                Expression::Ptr makeTOExpr();
                Expression::Ptr makeDSExpr();
                template <Result_Type size> void L();
                template <Result_Type size> void ST();
                template <Result_Type size> void LX();
                template <Result_Type size> void STX();
                template <Result_Type size> void LU();
                template <Result_Type size> void STU();
                template <Result_Type size> void LUX();
                template <Result_Type size> void STUX();
                void LK();
                void LI();
                void FRT();
                void FRS();
                void FRT2();
                void FRS2();
                void RT();
                void RS();
                void BD();
                void TO();
                void RA();
                void SI();
                void D();
                void mainDecode();
                void BF();
                void UI();
                void BO();
                void AA() {}
                void LL();
                void SH();
                void Rc();
                void RB();
                void FRA();
                void FRB();
                void FRC();
                void BI();
                void ME();
                void MB();
                void BFA();
                void BT();
                void BA();
                void BB();
                void OE();
                void FXM();
                void spr();
                void SR();
                void NB();
                void U();
                void FLM();
                void FRTP();
                void FRTS();
                void FRSP();
                void FRSS();
                void FRAP();
                void FRAS();
                void FRBP();
                void FRBS();
                void FRCP();
                void FRCS();
                void syscall();
                void setFPMode();
                void L() {}; // non-templated version for some zero fields
                const power_entry& extended_op_0();
                const power_entry& extended_op_4();
                const power_entry& extended_op_19();
                const power_entry& extended_op_30();
                const power_entry& extended_op_31();
                const power_entry& extended_op_59();
                const power_entry& extended_op_63();
                template <int start, int end>
                int field(unsigned int raw) {
#if defined DEBUG_FIELD
                    std::cerr << start << "-" << end << ":" << std::dec << (raw >> (31 - (end)) &
                            (0xFFFFFFFF >> (31 - (end - start)))) << " ";
#endif
                    return (raw >> (31 - (end)) & (0xFFFFFFFF >> (31 - (end - start))));
                }
                template <int size>
                int sign_extend(int in)
                {
                    return (in << (32 - size)) >> (32 - size);
                }

                
                template<unsigned int low, unsigned int high, unsigned int base = power_CR0, unsigned int curCR = high - low>
                struct translateBitFieldToCR
                {
                    translateBitFieldToCR(InstructionDecoder_power& dec) :
                            m_dec(dec) {}
                    void operator()()
                    {
                        (translateBitFieldToCR<low, high - 1, base, curCR - 1>(m_dec))();
                        if(m_dec.field<high, high>(m_dec.insn))
                        {
                            m_dec.insn_in_progress->appendOperand(m_dec.makeRegisterExpression(m_dec.makePowerRegID(
                                    base, curCR)), !m_dec.isRAWritten, m_dec.isRAWritten);
                        }
                    }
                    InstructionDecoder_power& m_dec;
                };
                template<unsigned int low, unsigned int high, unsigned int base>
                        struct translateBitFieldToCR<low, high, base, 0>
                        {
                    translateBitFieldToCR(InstructionDecoder_power& dec) :
                            m_dec(dec) {}
                    void operator()()
                    {
                        if(m_dec.field<high, high>(m_dec.insn))
                        {
                            m_dec.insn_in_progress->appendOperand(m_dec.makeRegisterExpression(m_dec.makePowerRegID(
                                    base, 0)), !m_dec.isRAWritten, m_dec.isRAWritten);
                        }
                    }
                    InstructionDecoder_power& m_dec;
                };
                
                unsigned int makePowerRegID(unsigned int bank, unsigned int encoding, int field = -1);
                unsigned int makePowerRegID(unsigned int bank, unsigned int encoding,
                        unsigned int lowBit, unsigned int highBit);
                
                Expression::Ptr makeFallThroughExpr();

                unsigned int insn;
                Instruction* insn_in_progress;
                bool isRAWritten;
                bool invertBranchCondition;
                bool isFPInsn;
                bool bcIsConditional;
        };
    }
}
