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

#include "InstructionDecoder-power.h"
#include <boost/assign/list_of.hpp>
#include "../../common/src/singleton_object_pool.h"

namespace Dyninst
{
  namespace InstructionAPI
  {
      typedef void (InstructionDecoder_power::*operandFactory)();
      typedef std::vector< operandFactory > operandSpec;
      typedef const power_entry&(InstructionDecoder_power::*nextTableFunc)();
      typedef std::map<unsigned int, power_entry> power_table;
      bool InstructionDecoder_power::foundDoubleHummerInsn = false;
      bool InstructionDecoder_power::foundQuadInsn = false;
      struct power_entry
      {
        power_entry(entryID o, const char* m, nextTableFunc next, operandSpec ops) :
                op(o), mnemonic(m), next_table(next), operands(ops)
                {}
        power_entry() :
                        op(power_op_INVALID), mnemonic("INVALID"), next_table(NULL)
                        {
                            operands.reserve(5);
                        }
        power_entry(const power_entry& o) :
                op(o.op), mnemonic(o.mnemonic), next_table(o.next_table), operands(o.operands)
                {}
                const power_entry& operator=(const power_entry& rhs)
                {
                    operands.reserve(rhs.operands.size());
                    op = rhs.op;
                    mnemonic = rhs.mnemonic;
                    next_table = rhs.next_table;
                    operands = rhs.operands;
                    return *this;
                }
                entryID op;
                const char* mnemonic;
                nextTableFunc next_table;
                operandSpec operands;
                static void buildTables();
                static bool built_tables;
                static std::vector<power_entry> main_opcode_table;
                static power_table extended_op_0;
                static power_table extended_op_4;
                static power_table extended_op_19;
                static power_table extended_op_30;
                static power_table extended_op_31;
                static power_table extended_op_58;
                static power_table extended_op_59;
                static power_table extended_op_63;
      };

    InstructionDecoder_power::InstructionDecoder_power(Architecture a)
      : InstructionDecoderImpl(a),
        insn(0), insn_in_progress(NULL),
	isRAWritten(false), invertBranchCondition(false),
        isFPInsn(false), bcIsConditional(false)
    {
        power_entry::buildTables();
    }
    InstructionDecoder_power::~InstructionDecoder_power()
    {
    }
    void InstructionDecoder_power::decodeOpcode(InstructionDecoder::buffer& b)
    {
      b.start += 4;
    }

    void InstructionDecoder_power::FRTP()
    {
        FRT();
        FRTS();
    }
    void InstructionDecoder_power::FRTS()
    {
        isFPInsn = true;
        MachRegister regID = makePowerRegID(ppc32::fsr0, field<6, 10>(insn));
        insn_in_progress->appendOperand(makeRegisterExpression(regID), false, true);
        isRAWritten = false;
        foundDoubleHummerInsn = true;
    }
    void InstructionDecoder_power::QFRTP()
    {
        QRT();
        QFRTS();
    }
    void InstructionDecoder_power::QFRTS()
    {
        isFPInsn = true;
        MachRegister regID = makePowerRegID(ppc64::fsr0, field<6, 10>(insn));
        insn_in_progress->appendOperand(makeRegisterExpression(regID), false, true);
        isRAWritten = false;
        foundQuadInsn = true;
    }
    void InstructionDecoder_power::FRSP()
    {
        FRS();
        FRSS();
    }
    void InstructionDecoder_power::FRSS()
    {
        isFPInsn = true;
        MachRegister regID = makePowerRegID(ppc32::fsr0, field<6, 10>(insn));
        insn_in_progress->appendOperand(makeRegisterExpression(regID), true, false);
        isRAWritten = true;
        foundDoubleHummerInsn = true;
    }
    void InstructionDecoder_power::QFRSP()
    {
        QFRS();
        QFRSS();
    }
    void InstructionDecoder_power::QFRSS()
    {
        isFPInsn = true;
        MachRegister regID = makePowerRegID(ppc64::fsr0, field<6, 10>(insn));
        insn_in_progress->appendOperand(makeRegisterExpression(regID), true, false);
        isRAWritten = true;
        foundQuadInsn = true;
    }
    void InstructionDecoder_power::FRAP()
    {
        FRA();
        FRAS();
    }
    void InstructionDecoder_power::FRAS()
    {
        isFPInsn = true;
        MachRegister regID = makePowerRegID(ppc32::fsr0, field<11, 15>(insn));
        insn_in_progress->appendOperand(makeRegisterExpression(regID), !isRAWritten, isRAWritten);
        foundDoubleHummerInsn = true;
    }
    void InstructionDecoder_power::QFRAP()
    {
        QFRA();
        QFRAS();
    }
    void InstructionDecoder_power::QFRAS()
    {
        isFPInsn = true;
        MachRegister regID = makePowerRegID(ppc64::fsr0, field<11, 15>(insn));
        insn_in_progress->appendOperand(makeRegisterExpression(regID), !isRAWritten, isRAWritten);
        foundQuadInsn = true;
    }
    void InstructionDecoder_power::FRBP()
    {
        FRB();
        FRBS();
    }
    void InstructionDecoder_power::QRB()
    {
        insn_in_progress->appendOperand(makeQRBExpr(), true, false);
        QRBS();
    }
    void InstructionDecoder_power::QRBS()
    {
        isFPInsn = true;
        MachRegister regID = makePowerRegID(ppc64::fsr0, field<16, 20>(insn));
        insn_in_progress->appendOperand(makeRegisterExpression(regID), true, false);
        foundQuadInsn = true;
    }
    void InstructionDecoder_power::FRBS()
    {
        isFPInsn = true;
        MachRegister regID = makePowerRegID(ppc32::fsr0, field<16, 20>(insn));
        insn_in_progress->appendOperand(makeRegisterExpression(regID), true, false);
        foundDoubleHummerInsn = true;
    }
    void InstructionDecoder_power::FRCP()
    {
        FRC();
        FRCS();
    }
    void InstructionDecoder_power::FRCS()
    {
        isFPInsn = true;
        MachRegister regID = makePowerRegID(ppc32::fsr0, field<21, 25>(insn));
        insn_in_progress->appendOperand(makeRegisterExpression(regID), true, false);
        foundDoubleHummerInsn = true;
    }

    void InstructionDecoder_power::QFRCP()
    {
        QFRC();
        QFRCS();
    }
    void InstructionDecoder_power::QFRCS()
    {
        isFPInsn = true;
        MachRegister regID = makePowerRegID(ppc64::fsr0, field<21, 25>(insn));
        insn_in_progress->appendOperand(makeRegisterExpression(regID), true, false);
        foundQuadInsn = true;
    }

    Expression::Ptr InstructionDecoder_power::makeFallThroughExpr()
    {
        return makeAddExpression(makeRegisterExpression(ppc32::pc), Immediate::makeImmediate(Result(u32, 4)), u32);
    }
    void InstructionDecoder_power::LI()
    {
        insn_in_progress->addSuccessor(makeIFormBranchTarget(), field<31, 31>(insn) == 1, false, false, false);
    }
    void InstructionDecoder_power::BD()
    {
        insn_in_progress->addSuccessor(makeBFormBranchTarget(), field<31, 31>(insn) == 1, false, bcIsConditional, false);
        if(bcIsConditional)
        {
            insn_in_progress->addSuccessor(makeFallThroughExpr(), false, false, false, true);
        }
    }
    
    using namespace std;
    Instruction::Ptr InstructionDecoder_power::decode(InstructionDecoder::buffer& b)
    {
      if(b.start > b.end) return Instruction::Ptr();
      isRAWritten = false;
      isFPInsn = false;
      bcIsConditional = false;
      insn = *((const uint32_t*)b.start);
#if defined(DEBUG_RAW_INSN)        
        cout.width(0);
        cout << "0x";
        cout.width(8);
        cout.fill('0');
        cout << hex << insn << "\t";
#endif
        mainDecode();
        b.start += 4;
        return make_shared(insn_in_progress);
    }

    bool InstructionDecoder_power::decodeOperands(const Instruction*)
    {
      assert(!"not implemented");
      return false;
    }
    void InstructionDecoder_power::OE()
    {
        if(field<21, 21>(insn))
        {
            // overflow and summary overflow bits; do we want full bit detail?
            insn_in_progress->appendOperand(makeRegisterExpression(ppc32::xer), false, true);
            insn_in_progress->getOperation().mnemonic += "o";
        }
    }
    template <Result_Type size>
    void InstructionDecoder_power::L()
    {
        insn_in_progress->appendOperand(makeMemRefNonIndex(size), true, false);
    }
    template <Result_Type size>
    void InstructionDecoder_power::ST()
    {
        insn_in_progress->appendOperand(makeMemRefNonIndex(size), false, true);
    }
    template <Result_Type size>
    void InstructionDecoder_power::LX()
    {
        insn_in_progress->appendOperand(makeMemRefIndex(size), true, false);
    }
    template <Result_Type size>
    void InstructionDecoder_power::STX()
    {
        insn_in_progress->appendOperand(makeMemRefIndex(size), false, true);
    }
    template <Result_Type size>
    void InstructionDecoder_power::LU()
    {
        L<size>();
        insn_in_progress->appendOperand(makeRAExpr(), false, true, true);
    }
    template <Result_Type size>
    void InstructionDecoder_power::STU()
    {
        ST<size>();
        insn_in_progress->appendOperand(makeRAExpr(), false, true, true);
    }
    template <Result_Type size>
    void InstructionDecoder_power::LUX()
    {
        LX<size>();
        insn_in_progress->appendOperand(makeRAExpr(), false, true, true);
    }
    template <Result_Type size>
    void InstructionDecoder_power::STUX()
    {
        STX<size>();
        insn_in_progress->appendOperand(makeRAExpr(), false, true, true);
    }
    void InstructionDecoder_power::LK()
    {
    }
    
    Expression::Ptr InstructionDecoder_power::makeMemRefIndex(Result_Type size)
    {
      return makeDereferenceExpression(makeAddExpression(makeRAorZeroExpr(), makeRBExpr(), s32), size);
    }
    Expression::Ptr InstructionDecoder_power::makeDSExpr()
    {
        return Immediate::makeImmediate(Result(s32, sign_extend<14>(field<16,29>(insn)) << 2));
    }
    Expression::Ptr InstructionDecoder_power::makeMemRefNonIndex(Result_Type size)
    {
        if(field<0,5>(insn) == 31 && (field<21,30>(insn) == 597 || field<21, 30>(insn) == 725))
        {
            return makeDereferenceExpression(makeRAorZeroExpr(), size);
        }
        if(size == u64 || size == s64 || size == dbl128 || field<0,5>(insn) == 58)
        {
            return makeDereferenceExpression(makeAddExpression(makeRAorZeroExpr(), makeDSExpr(), s32), size);
        }
        return makeDereferenceExpression(makeAddExpression(makeRAorZeroExpr(), makeDorSIExpr(), s32), size);
    }
    Expression::Ptr InstructionDecoder_power::makeRAExpr()
    {
        return makeRegisterExpression(makePowerRegID(ppc32::r0, field<11, 15>(insn)));
    }
    void InstructionDecoder_power::RA()
    {
        if(insn_in_progress->getOperation().getID() == power_op_addi)
        {
            if(field<11,15>(insn) == 0)
            {
                insn_in_progress->getOperation().mnemonic = "li";
                insn_in_progress->appendOperand(makeRAorZeroExpr(), !isRAWritten, isRAWritten);
                return;
            }
        }
        if(insn_in_progress->getOperation().getID() == power_op_addis)
        {
            if(field<11,15>(insn) == 0)
            {
                insn_in_progress->getOperation().mnemonic = "lis";
                insn_in_progress->appendOperand(makeRAorZeroExpr(), !isRAWritten, isRAWritten);
                return;
            }
        }
        insn_in_progress->appendOperand(makeRAExpr(), !isRAWritten, isRAWritten);
    }
    Expression::Ptr InstructionDecoder_power::makeRBExpr()
    {
        return makeRegisterExpression(makePowerRegID(ppc32::r0, field<16, 20>(insn)));
    }
    Expression::Ptr InstructionDecoder_power::makeFRAExpr()
    {
        isFPInsn = true;
        return makeRegisterExpression(makePowerRegID(ppc32::fpr0, field<11, 15>(insn)));
    }
    Expression::Ptr InstructionDecoder_power::makeQFRAExpr()
    {
        isFPInsn = true;
        return makeRegisterExpression(makePowerRegID(ppc64::fpr0, field<11, 15>(insn)));
    }
    Expression::Ptr InstructionDecoder_power::makeFRBExpr()
    {
        isFPInsn = true;
        return makeRegisterExpression(makePowerRegID(ppc32::fpr0, field<16, 20>(insn)));
    }
    Expression::Ptr InstructionDecoder_power::makeFRCExpr()
    {
        isFPInsn = true;
        return makeRegisterExpression(makePowerRegID(ppc32::fpr0, field<21, 25>(insn)));
    }
    Expression::Ptr InstructionDecoder_power::makeQFRCExpr()
    {
        isFPInsn = true;
        return makeRegisterExpression(makePowerRegID(ppc64::fpr0, field<21, 25>(insn)));
    }
    Expression::Ptr InstructionDecoder_power::makeQRBExpr()
    {
        isFPInsn = true;
        return makeRegisterExpression(makePowerRegID(ppc64::fpr0, field<16, 20>(insn)));
    }
    Expression::Ptr InstructionDecoder_power::makeQRTExpr()
    {
        isFPInsn = true;
        return makeRegisterExpression(makePowerRegID(ppc64::fpr0, field<6, 10>(insn)));
    }
    void InstructionDecoder_power::QRT()
    {
        insn_in_progress->appendOperand(makeQRTExpr(), false, true);
    }
    Expression::Ptr InstructionDecoder_power::makeFRTExpr()
    {
        isFPInsn = true;
        return makeRegisterExpression(makePowerRegID(ppc32::fpr0, field<6, 10>(insn)));
    }
    void InstructionDecoder_power::FRT()
    {
        insn_in_progress->appendOperand(makeFRTExpr(), false, true);
    }
    void InstructionDecoder_power::FRS()
    {
        insn_in_progress->appendOperand(makeFRTExpr(), true, false);
    }
    void InstructionDecoder_power::QFRS()
    {
        insn_in_progress->appendOperand(makeQRTExpr(), true, false);
    }
    void InstructionDecoder_power::FRT2()
    {
        MachRegister firstRegID = makePowerRegID(ppc32::fpr0, field<6,10>(insn));
        MachRegister secondRegID = makePowerRegID(ppc32::fpr0, field<6,10>(insn) + 1);
        insn_in_progress->appendOperand(makeRegisterExpression(firstRegID), false, true);
        insn_in_progress->appendOperand(makeRegisterExpression(secondRegID), false, true);
    }
    void InstructionDecoder_power::FRS2()
    {
        MachRegister firstRegID = makePowerRegID(ppc32::fpr0, field<6,10>(insn));
        MachRegister secondRegID = makePowerRegID(ppc32::fpr0, field<6,10>(insn) + 1);
        insn_in_progress->appendOperand(makeRegisterExpression(firstRegID), true, false);
        insn_in_progress->appendOperand(makeRegisterExpression(secondRegID), true, false);
    }
    void InstructionDecoder_power::RT()
    {
        insn_in_progress->appendOperand(makeRTExpr(), false, true);
    }
    void InstructionDecoder_power::RS()
    {
        insn_in_progress->appendOperand(makeRTExpr(), true, false);
        isRAWritten = true;
    }
    Expression::Ptr InstructionDecoder_power::makeRTExpr()
    {
        return makeRegisterExpression(makePowerRegID(ppc32::r0, field<6, 10>(insn)));
    }
    Expression::Ptr InstructionDecoder_power::makeIFormBranchTarget()
    {
        return makeBranchTarget<6, 29>();
    }
    Expression::Ptr InstructionDecoder_power::makeBFormBranchTarget()
    {
        return makeBranchTarget<16, 29>();
    }
    Expression::Ptr InstructionDecoder_power::makeRAorZeroExpr()
    {
        if(field<11, 15>(insn) == 0)
        {
            return Immediate::makeImmediate(Result(u32, 0));
        }
        else
        {
            return makeRAExpr();
        }
    } 
    Expression::Ptr InstructionDecoder_power::makeDorSIExpr()
    {
        return Immediate::makeImmediate(Result(s16, field<16, 31>(insn)));
    }
    void InstructionDecoder_power::SI()
    {
        insn_in_progress->appendOperand(makeDorSIExpr(), true, false);
    }
    void InstructionDecoder_power::D()
    {
        SI();
    }
    Expression::Ptr InstructionDecoder_power::makeBTExpr()
    {
        return makeRegisterExpression(makePowerRegID(ppc32::cr0, field<6, 10>(insn) >> 2));
    }
    Expression::Ptr InstructionDecoder_power::makeBAExpr()
    {
        return makeRegisterExpression(makePowerRegID(ppc32::cr0, field<11, 15>(insn) >> 2));
    }
    Expression::Ptr InstructionDecoder_power::makeBBExpr()
    {
        return makeRegisterExpression(makePowerRegID(ppc32::cr0, field<16, 20>(insn) >> 2));
    }
    Expression::Ptr InstructionDecoder_power::makeCR0Expr()
    {
        return makeRegisterExpression(ppc32::cr0);
    }
    Expression::Ptr InstructionDecoder_power::makeBIExpr()
    {
        switch(field<11, 15>(insn) & 0x03)
        {
            case 0:
                insn_in_progress->getOperation().mnemonic += (invertBranchCondition ? "ge" : "lt");
                break;
            case 1:
                insn_in_progress->getOperation().mnemonic += (invertBranchCondition ? "le" : "gt");
                break;
            case 2:
                insn_in_progress->getOperation().mnemonic += (invertBranchCondition ? "ne" : "eq");
                break;
            case 3:
                insn_in_progress->getOperation().mnemonic += (invertBranchCondition ? "ns" : "so");
                break;
            default:
                assert(!"can't happen");
                break;
        } 
        return makeRegisterExpression(makePowerRegID(ppc32::cr0, field<11, 15>(insn) >> 2));
    }
    
    Result_Type InstructionDecoder_power::makeSizeType(unsigned int)
    {
      assert(!"not implemented");
      return u32;
    }
    Expression::Ptr InstructionDecoder_power::makeSHExpr()
    {
        // For sradi instruction, the SH field is bit30 || bit16-20
        if (field<0,5>(insn) == 31 && field<21,29>(insn) == 413) {
            unsigned shift = ((field<30, 30>(insn)) << 5) | (field<16,20>(insn));
            return Immediate::makeImmediate(Result(u32, shift));
        }
        return Immediate::makeImmediate(Result(u32, (field<16, 20>(insn))));
    }
    Expression::Ptr InstructionDecoder_power::makeMBExpr()
    {
        return Immediate::makeImmediate(Result(u8, field<21, 25>(insn)));
    }
    Expression::Ptr InstructionDecoder_power::makeMEExpr()
    {
        return Immediate::makeImmediate(Result(u8, field<26, 30>(insn)));
    }
    Expression::Ptr InstructionDecoder_power::makeFLMExpr()
    {
        return Immediate::makeImmediate(Result(u32, field<7, 14>(insn)));
    }
    Expression::Ptr InstructionDecoder_power::makeTOExpr()
    {
        return Immediate::makeImmediate(Result(u32, field<6, 10>(insn)));
    }
    void InstructionDecoder_power::TO()
    {
        insn_in_progress->appendOperand(makeTOExpr(), true, false);
    }
    Expression::Ptr InstructionDecoder_power::makeSPRExpr()
    {
        int sprIDlo = field<11, 15>(insn);
        int sprIDhi = field<16, 20>(insn);
        int sprID = (sprIDhi << 5) + sprIDlo;

        // This is mftb, which is equivalent to mfspr Rx, 268 
        if (field<0,5>(insn) == 31 && field<21,30>(insn) == 371) {
            sprID = 268;
        }
        return makeRegisterExpression(makePowerRegID(ppc32::mq, sprID));
    }

    void InstructionDecoder_power::setFPMode()
    {
        isFPInsn = true;
    }
    void InstructionDecoder_power::doDelayedDecode(const Instruction* insn_to_complete)
    {
        isRAWritten = false;
        isFPInsn = false;
        bcIsConditional = false;
        insn = insn_to_complete->m_RawInsn.small_insn;
        const power_entry* current = &power_entry::main_opcode_table[field<0,5>(insn)];
        while(current->next_table)
        {
            current = &(std::mem_fun(current->next_table)(this));
        }
	if (findRAAndRS(current)) {
	    isRAWritten = true;
	}
        insn_in_progress = const_cast<Instruction*>(insn_to_complete);
        if(current->op == power_op_b ||
           current->op == power_op_bc ||
           current->op == power_op_bclr ||
           current->op == power_op_bcctr)
        {
            insn_in_progress->appendOperand(makeRegisterExpression(ppc32::pc), false, true);
        }
        
        for(operandSpec::const_iterator curFn = current->operands.begin();
            curFn != current->operands.end();
            ++curFn)
        {
            std::mem_fun(*curFn)(this);
        }
        if(current->op == power_op_bclr)
        {
	  // blrl is in practice a return-and-link, not a call-through-LR
	  // so we'll treat it as such
            insn_in_progress->addSuccessor(makeRegisterExpression(ppc32::lr),
                                           /*field<31,31>(insn) == 1*/ false, true, 
					   bcIsConditional, false);
            if(bcIsConditional)
            {
                insn_in_progress->addSuccessor(makeFallThroughExpr(), false, false, false, true);
            }
        }
        if(current->op == power_op_bcctr)
        {
            insn_in_progress->addSuccessor(makeRegisterExpression(ppc32::ctr),
                                           field<31,31>(insn) == 1, true, bcIsConditional, false);
            if(bcIsConditional)
            {
                insn_in_progress->addSuccessor(makeFallThroughExpr(), false, false, false, true);
            }
        }
        if(current->op == power_op_addic_rc ||
           current->op == power_op_andi_rc ||
           current->op == power_op_andis_rc ||
           current->op == power_op_stwcx_rc ||
           current->op == power_op_stdcx_rc)
        {
            insn_in_progress->appendOperand(makeCR0Expr(), false, true);
        }

        return;
    }
    MachRegister InstructionDecoder_power::makePowerRegID(MachRegister base, unsigned int encoding, int field)
    {
        if(field != -1)
        {
            return makePowerRegID(base, encoding, field * 4, field * 4 + 3);
        }
        return makePowerRegID(base, encoding, 0, 31);
    }
    MachRegister InstructionDecoder_power::makePowerRegID(MachRegister base, unsigned int encoding,
                                                         unsigned int, unsigned int)
    {
        return MachRegister(base.val() + encoding);
    }

#define fn(x) (&InstructionDecoder_power::x)

using namespace boost::assign;
    
#include "power_opcode_tables.C"


    const power_entry& InstructionDecoder_power::extended_op_0()
    {
        unsigned int xo = field<26, 30>(insn);
        if(xo <= 31)
        {
            return power_entry::extended_op_0[xo];
        }
        return power_entry::extended_op_0[field<21, 30>(insn)];
    }

    const power_entry& InstructionDecoder_power::extended_op_4()
    {
        return power_entry::extended_op_4[field<26, 30>(insn)];
    }
    const power_entry& InstructionDecoder_power::extended_op_19()
    {
        return power_entry::extended_op_19[field<21, 30>(insn)];
    }
    const power_entry& InstructionDecoder_power::extended_op_30()
    {
        return power_entry::extended_op_30[field<27, 29>(insn)];
    }
    const power_entry& InstructionDecoder_power::extended_op_31()
    {
        // sradi is a special instruction. Its xop is from 21 to 29 and its xop value is 413
        if (field<21,29>(insn) == 413) {
            return power_entry::extended_op_31[413];
        }
        const power_entry& xoform_entry = power_entry::extended_op_31[field<22, 30>(insn)];
        if(find(xoform_entry.operands.begin(), xoform_entry.operands.end(), &InstructionDecoder_power::OE)
           != xoform_entry.operands.end())
        {
            return xoform_entry;
        }
        return power_entry::extended_op_31[field<21, 30>(insn)];
    }
    const power_entry& InstructionDecoder_power::extended_op_58()
    {
        return power_entry::extended_op_58[field<30, 31>(insn)];
    }
    const power_entry& InstructionDecoder_power::extended_op_59()
    {
        return power_entry::extended_op_59[field<26, 30>(insn)];
    }
    const power_entry& InstructionDecoder_power::extended_op_63()
    {
        unsigned int xo = field<26, 30>(insn);
        if(xo <= 31)
        {
            power_table::const_iterator found = power_entry::extended_op_63.find(xo);
            if(found != power_entry::extended_op_63.end())
                return found->second;
        }
        return power_entry::extended_op_63[field<21, 30>(insn)];
    }
    
    void InstructionDecoder_power::BF()
    {
        MachRegister base_reg = isFPInsn ? ppc32::fpscw0 : ppc32::cr0;
        Expression::Ptr condReg = makeRegisterExpression(makePowerRegID(base_reg, field<6, 10>(insn) >> 2));
        insn_in_progress->appendOperand(condReg, false, true);
        return;
    }
    void InstructionDecoder_power::QTT()
    {
        Expression::Ptr imm = Immediate::makeImmediate(Result(u8, field<21, 24>(insn)));
        insn_in_progress->appendOperand(imm, true, false);
        return;
    }
    void InstructionDecoder_power::QVD()
    {
        Expression::Ptr imm = Immediate::makeImmediate(Result(u8, field<21, 22>(insn)));
        insn_in_progress->appendOperand(imm, true, false);
        return;
    }
    void InstructionDecoder_power::QGPC()
    {
        Expression::Ptr imm = Immediate::makeImmediate(Result(u8, field<11, 22>(insn)));
        insn_in_progress->appendOperand(imm, true, false);
        return;
    }
    void InstructionDecoder_power::UI()
    {
        Expression::Ptr imm = Immediate::makeImmediate(Result(u32, field<16, 31>(insn)));
        insn_in_progress->appendOperand(imm, true, false);
        return;
    }
    void InstructionDecoder_power::BO()
    {
        bcIsConditional = true;
#if defined(DEBUG_BO_FIELD)        
        cout << "BO: " << field<6,6>(insn) << field<7,7>(insn) << field<8,8>(insn) << field<9,9>(insn) << field<10,10>(insn)
                << endl;
#endif
        invertBranchCondition = false;
        if(!field<8, 8>(insn))
        {
            Expression::Ptr ctr = makeRegisterExpression(makePowerRegID(ppc32::ctr, 0));
            if(field<9, 9>(insn))
            {
                insn_in_progress->getOperation().mnemonic = "bdz";
            }
            else
            {
                insn_in_progress->getOperation().mnemonic = "bdn";
            }
            insn_in_progress->appendOperand(ctr, true, true);
        }
        if(!(field<6, 6>(insn)))
        {
            invertBranchCondition = !field<7,7>(insn);
            if(insn_in_progress->getOperation().mnemonic == "bc")
            {
                insn_in_progress->getOperation().mnemonic = "b";            
            }
            insn_in_progress->appendOperand(makeBIExpr(), true, false);
        }
        if(field<8,8>(insn) && field<6,6>(insn))
        {
            size_t found = insn_in_progress->getOperation().mnemonic.rfind("c");
            if(found != std::string::npos)
            {
                insn_in_progress->getOperation().mnemonic.erase(found, 1);
            }
            bcIsConditional = false;
        }
        else
        {
            bool taken = (field<6,6>(insn) && field<8,8>(insn)) || field<16,16>(insn);
			taken ^= field<10,10>(insn) ? true : false;
            insn_in_progress->getOperation().mnemonic += (taken ? "+" : "-");
        }
#if defined(DEBUG_BO_FIELD)
        cout << "bcIsConditional = " << (bcIsConditional ? "true" : "false") << endl;
#endif
        return;
    }
    void InstructionDecoder_power::syscall()
    {
        insn_in_progress->appendOperand(makeRegisterExpression(ppc32::msr), true, true);
        insn_in_progress->appendOperand(makeRegisterExpression(ppc32::srr0), false, true);
        insn_in_progress->appendOperand(makeRegisterExpression(ppc32::srr1), false, true);
        insn_in_progress->appendOperand(makeRegisterExpression(ppc32::pc), true, true);
        insn_in_progress->appendOperand(makeRegisterExpression(ppc32::ivpr), false, true);
        insn_in_progress->appendOperand(makeRegisterExpression(ppc32::ivor8), false, true);
        return;
    }
    void InstructionDecoder_power::LL()
    {
        LI();
        return;
    }
    void InstructionDecoder_power::SH()
    {
        insn_in_progress->appendOperand(makeSHExpr(), true, false);
        return;
    }
    void InstructionDecoder_power::Rc()
    {
        if(field<31, 31>(insn))
        {
            if(isFPInsn)
            {
                insn_in_progress->appendOperand(makeRegisterExpression(ppc32::fpscw), false, true, true);
            }
            else
            {
                insn_in_progress->appendOperand(makeCR0Expr(), false, true);
            }
            insn_in_progress->getOperation().mnemonic += ".";
        }
        return;
    }
    void InstructionDecoder_power::RB()
    {
        if(insn_in_progress->getOperation().getID() == power_op_or)
        {
            if(field<16,20>(insn) == field<6,10>(insn))
            {
                insn_in_progress->getOperation().mnemonic = "mr";
            }
        }
        else if(insn_in_progress->getOperation().getID() == power_op_nor)
        {
            if(field<16,20>(insn) == field<6,10>(insn))
            {
                insn_in_progress->getOperation().mnemonic = "not";
            }
        }
        insn_in_progress->appendOperand(makeRBExpr(), true, false);
        return;
    }
    void InstructionDecoder_power::FRA()
    {
        insn_in_progress->appendOperand(makeFRAExpr(), !isRAWritten, isRAWritten);
        return;
    }
    void InstructionDecoder_power::QFRA()
    {
        insn_in_progress->appendOperand(makeQFRAExpr(), !isRAWritten, isRAWritten);
        return;
    }
    void InstructionDecoder_power::FRB()
    {
        insn_in_progress->appendOperand(makeFRBExpr(), true, false);
        return;
    }
    void InstructionDecoder_power::FRC()
    {
        insn_in_progress->appendOperand(makeFRCExpr(), true, false);
        return;
    }
    void InstructionDecoder_power::QFRC()
    {
        insn_in_progress->appendOperand(makeQFRCExpr(), true, false);
        return;
    }
    void InstructionDecoder_power::BI()
    {
        return;
    }
    void InstructionDecoder_power::ME()
    {
        insn_in_progress->appendOperand(makeMEExpr(), true, false);
        return;
    }
    void InstructionDecoder_power::MB()
    {
        insn_in_progress->appendOperand(makeMBExpr(), true, false);
        return;
    }
    void InstructionDecoder_power::BFA()
    {
        Expression::Ptr condReg = makeRegisterExpression(makePowerRegID(ppc32::cr0, field<11, 15>(insn) >> 2));
        insn_in_progress->appendOperand(condReg, true, false);
        return;
    }
    void InstructionDecoder_power::BT()
    {
        insn_in_progress->appendOperand(makeBTExpr(), false, true);
        return;
    }
    void InstructionDecoder_power::BA()
    {
        insn_in_progress->appendOperand(makeBAExpr(), true, false);
        return;
    }
    void InstructionDecoder_power::BB()
    {
        insn_in_progress->appendOperand(makeBBExpr(), true, false);
        return;
    }

    void InstructionDecoder_power::FXM()
    {
        (translateBitFieldToCR<12, 19, ppc32::icr0, 7>(*this))();
        return;
    }
    void InstructionDecoder_power::spr()
    {
        insn_in_progress->appendOperand(makeSPRExpr(), !isRAWritten, isRAWritten);
        return;
    }
    void InstructionDecoder_power::SR()
    {
        insn_in_progress->appendOperand(makeRegisterExpression(makePowerRegID(ppc32::seg0, field<11, 15>(insn) >> 2)),
                                        true, true);
        return;
    }
    void InstructionDecoder_power::NB()
    {
        insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u8, field<16, 20>(insn))), true, false);
        return;
    }
    void InstructionDecoder_power::U()
    {
        insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u8, field<16, 20>(insn) >> 1)), true, false);
        return;
    }
    void InstructionDecoder_power::FLM()
    {
        isRAWritten = true;
        (translateBitFieldToCR<7, 14, ppc32::ifpscw0, 7>(*this))();
        return;
    }
     void InstructionDecoder_power::WC()
    {
        insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u8, field<9, 10>(insn))), true, false);
        return;
    }
   
    bool InstructionDecoder_power::findRAAndRS(const power_entry* cur) {
        bool findRA = false;
	bool findRS = false;
	for (auto oit = cur->operands.begin(); oit != cur->operands.end(); ++oit) {
	    if ((*oit) == &InstructionDecoder_power::RA) findRA = true;
	    if ((*oit) == &InstructionDecoder_power::RS) findRS = true;
	}
	return findRA && findRS;
    }

    void InstructionDecoder_power::mainDecode()
    {
        const power_entry* current = &power_entry::main_opcode_table[field<0,5>(insn)];
        while(current->next_table)
        {
            current = &(std::mem_fun(current->next_table)(this));
        }
        insn_in_progress = makeInstruction(current->op, current->mnemonic, 4, reinterpret_cast<unsigned char*>(&insn));
        if(current->op == power_op_b ||
          current->op == power_op_bc ||
          current->op == power_op_bclr ||
          current->op == power_op_bcctr)
        {
            // decode control-flow operands immediately; we're all but guaranteed to need them
            doDelayedDecode(insn_in_progress);
        }
	// FIXME in parsing
        insn_in_progress->arch_decoded_from = m_Arch;
        //insn_in_progress->arch_decoded_from = Arch_ppc32;
        return;
    }
  };
};



  
