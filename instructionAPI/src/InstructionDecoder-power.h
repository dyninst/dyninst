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
#include <iostream>
#include <stddef.h>
#include <string>
#include "Immediate.h"
#include "registers/ppc32_regs.h"
//#define DEBUG_FIELD

namespace Dyninst {
    namespace InstructionAPI
    {
        struct power_entry;

        class InstructionDecoder_power : public InstructionDecoderImpl
        {
            friend struct power_entry;
            public:
                InstructionDecoder_power(Architecture a);
                virtual ~InstructionDecoder_power();
                virtual void decodeOpcode(InstructionDecoder::buffer& b);
                virtual Instruction decode(InstructionDecoder::buffer& b);
		virtual void setMode(bool) 
		{
		}
                virtual bool decodeOperands(const Instruction* insn_to_complete);
                virtual void doDelayedDecode(const Instruction* insn_to_complete);
                static bool foundDoubleHummerInsn;
                static bool foundQuadInsn;
                using InstructionDecoderImpl::makeRegisterExpression;
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
                Expression::Ptr makeQFRCExpr();
                Expression::Ptr makeFRTExpr();
                Expression::Ptr makeIFormBranchTarget();
                Expression::Ptr makeBFormBranchTarget();
                Expression::Ptr makeRAorZeroExpr();
                Expression::Ptr makeDorSIExpr();
                Expression::Ptr makeBTExpr();
                Expression::Ptr makeBAExpr();
                Expression::Ptr makeQRBExpr();
                Expression::Ptr makeQRTExpr();
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
                Expression::Ptr makeQFRAExpr();
                template <Result_Type size> void L();
                template <Result_Type size> void ST();
                template <Result_Type size> void LX();
                template <Result_Type size> void STX();
                template <Result_Type size> void LU();
                template <Result_Type size> void STU();
                template <Result_Type size> void LUX();
                template <Result_Type size> void STUX();
                
        		void FC();
        		void BHRBE();
        		void CT();
        		void RSP();
        		void RTP();
        		void EH();
        		void PRS();
        		void RIC();
        		void A();
        		void RC();
        		void BC();
                void IH();
                void SP();
                void S();
                void TE();
                void DCM();
                void DGM();
                void DRM();
                void SHW();

                void XC();
                void DM();
                void IMM8();

        		void LK();
                void LI();
                void FRT();
                void FRS();
                void FRT2();
                void FRS2();
                void QRT();
                void QTT();
                void QVD();
                void QRB();
                void QRBS();
                void QGPC();
                void QFRTS();
                void QFRSP();
                void QFRTP();
                void QFRS();
                void QFRSS();
                void QFRCP();
                void QFRC();
                void QFRCS();
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
                void QFRA();
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
                void QFRAP();
                void QFRAS();
                void FRBP();
                void FRBS();
                void FRCP();
                void FRCS();
                void WC();
                void syscall();
                void setFPMode();
                void L() {} // non-templated version for some zero fields
                void XT();
                void XS();
                void XA();
                void XB();
                void VRT();
                void VRA();
                void VRB();
                void VRC();
                void UIM();
                void SIM();
                void DCMX();
                void VRS();
                void RO();
                void R();
                void RMC();
                void EX();
                void SHB();
                void SIX();
                void PS();
                void CY();



                const power_entry& extended_op_0();
                const power_entry& extended_op_4();
                const power_entry& extended_op_4_1409();
                const power_entry& extended_op_4_1538();
                const power_entry& extended_op_4_1921();
                const power_entry& extended_op_19();
                const power_entry& extended_op_30();
                const power_entry& extended_op_31();
                const power_entry& extended_op_57();
                const power_entry& extended_op_58();
                const power_entry& extended_op_59();
                const power_entry& extended_op_60();
                const power_entry& extended_op_60_347();
                const power_entry& extended_op_60_475();
		const power_entry& extended_op_60_specials_check();
                const power_entry& extended_op_61();
                const power_entry& extended_op_63();
                const power_entry& extended_op_63_583();
                const power_entry& extended_op_63_804();
                const power_entry& extended_op_63_836();

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

                
                template<unsigned int low, unsigned int high, unsigned int base = ppc32::icr0,
                    unsigned int curCR = high - low>
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
                                    MachRegister(base), curCR)), !m_dec.isRAWritten, m_dec.isRAWritten);
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
                                    MachRegister(base), 0)), !m_dec.isRAWritten, m_dec.isRAWritten);
                        }
                    }
                    InstructionDecoder_power& m_dec;
                };
                

                template< int lowBit, int highBit>
                        Expression::Ptr makeBranchTarget()
                        {
                            size_t where = insn_in_progress->getOperation().mnemonic.rfind('+');
                            if(where == std::string::npos) {
                                where = insn_in_progress->getOperation().mnemonic.rfind('-');
                            }
                            if(where == std::string::npos) {
                                where = insn_in_progress->getOperation().mnemonic.size();
                            }
                            if(field<31, 31>(insn) == 1)
                            {
                                insn_in_progress->getOperation().mnemonic.insert(where, "l");
                                where++;
                                insn_in_progress->appendOperand(makeRegisterExpression(ppc32::lr), false, true);
                            }
        // absolute address
                            if(field<30, 30>(insn) == 1)
                            {
                                insn_in_progress->getOperation().mnemonic.insert(where, "a");
                                return Immediate::makeImmediate(Result(u32,
                                        sign_extend<(highBit - lowBit + 1)>(field<lowBit, highBit>(insn)) << 2));
                            }
                            else
                            {
                                Expression::Ptr displacement = Immediate::makeImmediate(Result(s32,
                                        sign_extend<(highBit - lowBit + 1)>(field<lowBit, highBit>(insn)) << 2));
                                return makeAddExpression(makeRegisterExpression(ppc32::pc), displacement, s32);
                            }
                        }
                
                MachRegister makePowerRegID(MachRegister base, unsigned int encoding, int field = -1);
                MachRegister makePowerRegID(MachRegister base, unsigned int encoding,
                        unsigned int lowBit, unsigned int highBit);
                
                Expression::Ptr makeFallThroughExpr();

                unsigned int insn;
                boost::shared_ptr<Instruction> insn_in_progress;
                bool isRAWritten;
                bool invertBranchCondition;
                bool isFPInsn;
                bool bcIsConditional;
		bool findRAAndRS(const struct power_entry*);
        };
    }
}
