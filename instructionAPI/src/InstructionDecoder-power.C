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

#include "InstructionDecoder-power.h"
#include "Immediate.h"
#include <boost/assign/list_of.hpp>
#include "../../common/h/singleton_object_pool.h"


namespace Dyninst
{
  namespace InstructionAPI
  {
      typedef void (InstructionDecoder_power::*operandFactory)();
      typedef std::vector< operandFactory > operandSpec;
      typedef const power_entry&(InstructionDecoder_power::*nextTableFunc)();
      typedef std::map<unsigned int, power_entry> power_table;
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
                static std::vector<power_entry> main_opcode_table;
                static power_table extended_op_0;
                static power_table extended_op_4;
                static power_table extended_op_19;
                static power_table extended_op_30;
                static power_table extended_op_31;
                static power_table extended_op_59;
                static power_table extended_op_63;
      };

    InstructionDecoder_power::InstructionDecoder_power(const unsigned char* buffer, unsigned int length)
          : InstructionDecoder(buffer, length), isRAWritten(false)
    {
    }
    InstructionDecoder_power::~InstructionDecoder_power()
    {
    }
    unsigned int InstructionDecoder_power::decodeOpcode()
    {
        assert(!"not used on power!");
        return 0;
    }

    void InstructionDecoder_power::FRTP()
    {
        FRT();
        FRTS();
    }
    void InstructionDecoder_power::FRTS()
    {
        isFPInsn = true;
        unsigned int regID = makePowerRegID(bank_fsr, field<6, 10>(insn));
        insn_in_progress->appendOperand(makeRegisterExpression(regID), false, true);
        isRAWritten = false;
    }
    void InstructionDecoder_power::FRSP()
    {
        FRS();
        FRSS();
    }
    void InstructionDecoder_power::FRSS()
    {
        isFPInsn = true;
        unsigned int regID = makePowerRegID(bank_fsr, field<6, 10>(insn));
        insn_in_progress->appendOperand(makeRegisterExpression(regID), true, false);
        isRAWritten = true;
    }
    void InstructionDecoder_power::FRAP()
    {
        FRA();
        FRAS();
    }
    void InstructionDecoder_power::FRAS()
    {
        isFPInsn = true;
        unsigned int regID = makePowerRegID(bank_fsr, field<11, 15>(insn));
        insn_in_progress->appendOperand(makeRegisterExpression(regID), !isRAWritten, isRAWritten);
    }
    void InstructionDecoder_power::FRBP()
    {
        FRB();
        FRBS();
    }
    void InstructionDecoder_power::FRBS()
    {
        isFPInsn = true;
        unsigned int regID = makePowerRegID(bank_fsr, field<16, 20>(insn));
        insn_in_progress->appendOperand(makeRegisterExpression(regID), true, false);
    }
    void InstructionDecoder_power::FRCP()
    {
        FRC();
        FRCS();
    }
    void InstructionDecoder_power::FRCS()
    {
        isFPInsn = true;
        unsigned int regID = makePowerRegID(bank_fsr, field<21, 25>(insn));
        insn_in_progress->appendOperand(makeRegisterExpression(regID), true, false);
    }


    Expression::Ptr InstructionDecoder_power::makeFallThroughExpr()
    {
        return makeAddExpression(makeRegisterExpression(power::sprpc), Immediate::makeImmediate(Result(u32, 4)), u32);
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
    
    Instruction::Ptr InstructionDecoder_power::decode(const unsigned char* buffer)
    {
        rawInstruction = buffer;
        return decode();
    }
    using namespace std;
    Instruction::Ptr InstructionDecoder_power::decode()
    {
        if(rawInstruction < bufferBegin || rawInstruction >= bufferBegin + bufferSize) return Instruction::Ptr();
        isRAWritten = false;
        isFPInsn = false;
        bcIsConditional = false;
        insn = InstructionDecoder::rawInstruction[0] << 24 | InstructionDecoder::rawInstruction[1] << 16 |
                InstructionDecoder::rawInstruction[2] << 8 | InstructionDecoder::rawInstruction[3];
#if defined(DEBUG_RAW_INSN)        
        cout.width(0);
        cout << "0x";
        cout.width(8);
        cout.fill('0');
        cout << hex << insn << "\t";
#endif        
        mainDecode();
        rawInstruction += 4;
        return make_shared(insn_in_progress);
    }
    void InstructionDecoder_power::setMode(bool is64)
    {
      //assert(!"not implemented");
    }

    bool InstructionDecoder_power::decodeOperands(const Instruction* insn_to_complete)
    {
      assert(!"not implemented");
      return false;
    }
    void InstructionDecoder_power::OE()
    {
        if(field<21, 21>(insn))
        {
            insn_in_progress->appendOperand(makeRegisterExpression(makePowerRegID(0, power::sprxer,
                                            power::OF_bit, power::OF_bit)), false, true);
            insn_in_progress->appendOperand(makeRegisterExpression(makePowerRegID(0, power::sprxer,
                                            power::SOF_bit, power::SOF_bit)), false, true);
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
        insn_in_progress->appendOperand(makeRAExpr(), false, true);
    }
    template <Result_Type size>
    void InstructionDecoder_power::STU()
    {
        ST<size>();
        insn_in_progress->appendOperand(makeRAExpr(), false, true);
    }
    template <Result_Type size>
    void InstructionDecoder_power::LUX()
    {
        LX<size>();
        insn_in_progress->appendOperand(makeRAExpr(), false, true);
    }
    template <Result_Type size>
    void InstructionDecoder_power::STUX()
    {
        STX<size>();
        insn_in_progress->appendOperand(makeRAExpr(), false, true);
    }
    void InstructionDecoder_power::LK()
    {
        if(field<31, 31>(insn))
        {
            insn_in_progress->appendOperand(makeRegisterExpression(power::sprlr), false, true);
            size_t where = insn_in_progress->getOperation().mnemonic.rfind('+');
            if(where == std::string::npos) {
                where = insn_in_progress->getOperation().mnemonic.rfind('-');
            }
            if(where == std::string::npos) {
                where = insn_in_progress->getOperation().mnemonic.size();
            }
            
            insn_in_progress->getOperation().mnemonic.insert(where, "l");
        }
    }
    
    Expression::Ptr InstructionDecoder_power::makeMemRefIndex(Result_Type size)
    {
      return makeDereferenceExpression(makeAddExpression(makeRAorZeroExpr(), makeRBExpr(), s32), size);
    }
    Expression::Ptr InstructionDecoder_power::makeDSExpr()
    {
        return Immediate::makeImmediate(Result(s32, sign_extend<14>(field<16,29>(insn))));
    }
    Expression::Ptr InstructionDecoder_power::makeMemRefNonIndex(Result_Type size)
    {
        if(size == u64 || size == s64 || size == dbl128)
        {
            return makeDereferenceExpression(makeAddExpression(makeRAorZeroExpr(), makeDSExpr(), s32), size);
        }
        return makeDereferenceExpression(makeAddExpression(makeRAorZeroExpr(), makeDorSIExpr(), s32), size);
    }
    Expression::Ptr InstructionDecoder_power::makeRAExpr()
    {
        return makeRegisterExpression(makePowerRegID(bank_gpr, field<11, 15>(insn)));
    }
    void InstructionDecoder_power::RA()
    {
        if(insn_in_progress->getOperation().getID() == power_op_addi)
        {
            if(field<11,15>(insn) == 0)
            {
                insn_in_progress->getOperation().mnemonic = "li";
                return;
            }
        }
        if(insn_in_progress->getOperation().getID() == power_op_addis)
        {
            if(field<11,15>(insn) == 0)
            {
                insn_in_progress->getOperation().mnemonic = "lis";
                return;
            }
        }
        insn_in_progress->appendOperand(makeRAExpr(), !isRAWritten, isRAWritten);
    }
    Expression::Ptr InstructionDecoder_power::makeRBExpr()
    {
        return makeRegisterExpression(makePowerRegID(bank_gpr, field<16, 20>(insn)));
    }
    Expression::Ptr InstructionDecoder_power::makeFRAExpr()
    {
        isFPInsn = true;
        return makeRegisterExpression(makePowerRegID(bank_fpr, field<11, 15>(insn)));
    }
    Expression::Ptr InstructionDecoder_power::makeFRBExpr()
    {
        isFPInsn = true;
        return makeRegisterExpression(makePowerRegID(bank_fpr, field<16, 20>(insn)));
    }
    Expression::Ptr InstructionDecoder_power::makeFRCExpr()
    {
        isFPInsn = true;
        return makeRegisterExpression(makePowerRegID(bank_fpr, field<21, 25>(insn)));
    }
    Expression::Ptr InstructionDecoder_power::makeFRTExpr()
    {
        isFPInsn = true;
        return makeRegisterExpression(makePowerRegID(bank_fpr, field<6, 10>(insn)));
    }
    void InstructionDecoder_power::FRT()
    {
        insn_in_progress->appendOperand(makeFRTExpr(), false, true);
    }
    void InstructionDecoder_power::FRS()
    {
        insn_in_progress->appendOperand(makeFRTExpr(), true, false);
    }
    void InstructionDecoder_power::FRT2()
    {
        int firstRegID = makePowerRegID(bank_fpr, field<6,10>(insn));
        int secondRegID = makePowerRegID(bank_fpr, field<6,10>(insn) + 1);
        insn_in_progress->appendOperand(makeRegisterExpression(firstRegID), false, true);
        insn_in_progress->appendOperand(makeRegisterExpression(secondRegID), false, true);
    }
    void InstructionDecoder_power::FRS2()
    {
        int firstRegID = makePowerRegID(bank_fpr, field<6,10>(insn));
        int secondRegID = makePowerRegID(bank_fpr, field<6,10>(insn) + 1);
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
        return makeRegisterExpression(makePowerRegID(bank_gpr, field<6, 10>(insn)));
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
        return makeRegisterExpression(makePowerRegID(bank_cond, field<6, 10>(insn) >> 2));
    }
    Expression::Ptr InstructionDecoder_power::makeBAExpr()
    {
        return makeRegisterExpression(makePowerRegID(bank_cond, field<11, 15>(insn) >> 2));
    }
    Expression::Ptr InstructionDecoder_power::makeBBExpr()
    {
        return makeRegisterExpression(makePowerRegID(bank_cond, field<16, 20>(insn) >> 2));
    }
    Expression::Ptr InstructionDecoder_power::makeCR0Expr()
    {
        return makeRegisterExpression(power::sprcr0);
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
        return makeRegisterExpression(makePowerRegID(bank_cond, field<11, 15>(insn) >> 2));
    }
    
    Result_Type InstructionDecoder_power::makeSizeType(unsigned int opType)
    {
      assert(!"not implemented");
      return u32;
    }
    Expression::Ptr InstructionDecoder_power::makeSHExpr()
    {
        return Immediate::makeImmediate(Result(u32, (field<16, 20>(insn))));
    }
    Expression::Ptr InstructionDecoder_power::makeMBExpr()
    {
        return Immediate::makeImmediate(Result(u32, field<21, 25>(insn)));
    }
    Expression::Ptr InstructionDecoder_power::makeMEExpr()
    {
        return Immediate::makeImmediate(Result(u32, field<26, 30>(insn)));
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
        return makeRegisterExpression(makePowerRegID(bank_spr, sprID));
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
        insn_in_progress = const_cast<Instruction*>(insn_to_complete);
        if(current->op == power_op_b ||
           current->op == power_op_bc ||
           current->op == power_op_bclr ||
           current->op == power_op_bcctr)
        {
            insn_in_progress->appendOperand(makeRegisterExpression(makePowerRegID(0, power::sprpc)), false, true);
        }
        
        for(operandSpec::const_iterator curFn = current->operands.begin();
            curFn != current->operands.end();
            ++curFn)
        {
            std::mem_fun(*curFn)(this);
        }
        if(current->op == power_op_bclr)
        {
            insn_in_progress->addSuccessor(makeRegisterExpression(makePowerRegID(0, power::sprlr)),
                                           field<31,31>(insn) == 1, true, bcIsConditional, false);
        }
        if(current->op == power_op_bcctr)
        {
            insn_in_progress->addSuccessor(makeRegisterExpression(makePowerRegID(0, power::sprctr)),
                                           field<31,31>(insn) == 1, true, bcIsConditional, false);
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
    unsigned int InstructionDecoder_power::makePowerRegID(unsigned int bank, unsigned int encoding, int field)
    {
        if(field != -1)
        {
            return makePowerRegID(bank, encoding, field * 4, field * 4 + 3);
        }
        return makePowerRegID(bank, encoding, 0, 31);
    }
    unsigned int InstructionDecoder_power::makePowerRegID(unsigned int bank, unsigned int encoding,
                                                         unsigned int lowBit, unsigned int highBit)
    {
        return bank + encoding;
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
        const power_entry& xoform_entry = power_entry::extended_op_31[field<22, 30>(insn)];
        if(find(xoform_entry.operands.begin(), xoform_entry.operands.end(), &InstructionDecoder_power::OE)
           != xoform_entry.operands.end())
        {
            return xoform_entry;
        }
        return power_entry::extended_op_31[field<21, 30>(insn)];
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
        int base_reg = isFPInsn ? power::sprfpscw0 : power::sprcr0;
        Expression::Ptr condReg = makeRegisterExpression(makePowerRegID(base_reg, field<6, 10>(insn) >> 2));
        insn_in_progress->appendOperand(condReg, false, true);
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
            Expression::Ptr ctr = makeRegisterExpression(makePowerRegID(0, power::sprctr));
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
            insn_in_progress->getOperation().mnemonic.erase(found, 1);
            bcIsConditional = false;
        }
        else
        {
            bool taken = (field<6,6>(insn) && field<8,8>(insn)) || field<16,16>(insn);
            taken ^= field<10,10>(insn);
            insn_in_progress->getOperation().mnemonic += (taken ? "+" : "-");
        }
            
        return;
    }
    void InstructionDecoder_power::syscall()
    {
        insn_in_progress->appendOperand(makeRegisterExpression(power::sprmsr), true, true);
        insn_in_progress->appendOperand(makeRegisterExpression(power::sprsrr0), false, true);
        insn_in_progress->appendOperand(makeRegisterExpression(power::sprsrr1), false, true);
        insn_in_progress->appendOperand(makeRegisterExpression(power::sprpc), true, true);
        insn_in_progress->appendOperand(makeRegisterExpression(power::sprivpr), false, true);
        insn_in_progress->appendOperand(makeRegisterExpression(power::sprivor8), false, true);
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
                insn_in_progress->appendOperand(makeRegisterExpression(makePowerRegID(0, power::sprfpscw)), false, true);
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
                return;
            }
        }
        else if(insn_in_progress->getOperation().getID() == power_op_nor)
        {
            if(field<16,20>(insn) == field<6,10>(insn))
            {
                insn_in_progress->getOperation().mnemonic = "not";
                return;
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
    void InstructionDecoder_power::BI()
    {
        insn_in_progress->appendOperand(makeRegisterExpression(makePowerRegID(bank_cond, field<11,13>(insn))), true, false);
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
        Expression::Ptr condReg = makeRegisterExpression(makePowerRegID(bank_cond, field<11, 15>(insn) >> 2));
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
        (translateBitFieldToCR<12, 19, power::sprcr0, 7>(*this))();
        return;
    }
    void InstructionDecoder_power::spr()
    {
        insn_in_progress->appendOperand(makeSPRExpr(), true, true);
        return;
    }
    void InstructionDecoder_power::SR()
    {
        insn_in_progress->appendOperand(makeRegisterExpression(makePowerRegID(bank_seg, field<11, 15>(insn) >> 2)),
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
        (translateBitFieldToCR<7, 14, power::sprfpscw0, 7>(*this))();
        return;
    }

    void InstructionDecoder_power::mainDecode()
    {
        const power_entry* current = &power_entry::main_opcode_table[field<0,5>(insn)];
        while(current->next_table)
        {
            current = &(std::mem_fun(current->next_table)(this));
        }
        insn_in_progress = makeInstruction(current->op, current->mnemonic, 4, reinterpret_cast<unsigned char*>(&insn));
        insn_in_progress->arch_decoded_from = power;
        return;
    }
  };
};



  
