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
#include <mutex>
#include "unaligned_memory_access.h"
#include "registers/ppc32_regs.h"
#include "registers/ppc64_regs.h"

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
                static std::vector<power_entry> main_opcode_table;
                static power_table extended_op_0;
                static power_table extended_op_4;
                static power_table extended_op_4_1409;
                static power_table extended_op_4_1538;
                static power_table extended_op_4_1921;
                static power_table extended_op_19;
                static power_table extended_op_30;
                static power_table extended_op_31;
                static power_table extended_op_57;
                static power_table extended_op_58;
                static power_table extended_op_59;
                static power_table extended_op_60;
                static power_table extended_op_60_specials;
                static power_table extended_op_60_347;
                static power_table extended_op_60_475;
                static power_table extended_op_61;
                static power_table extended_op_63;
                static power_table extended_op_63_583;
                static power_table extended_op_63_804;
                static power_table extended_op_63_836;

      };



    InstructionDecoder_power::InstructionDecoder_power(Architecture a)
      : InstructionDecoderImpl(a),
        insn(0),
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
    Instruction InstructionDecoder_power::decode(InstructionDecoder::buffer& b)
    {
      if(b.start > b.end) return Instruction();
      isRAWritten = false;
      isFPInsn = false;
      bcIsConditional = false;
      insn = Dyninst::read_memory_as<uint32_t>(b.start);
#if defined(DEBUG_RAW_INSN)        
        cout.width(0);
        cout << "0x";
        cout.width(8);
        cout.fill('0');
        cout << hex << insn << "\t";
#endif
        mainDecode();
        b.start += 4;
        return *(insn_in_progress.get());
    }

    bool InstructionDecoder_power::decodeOperands(const Instruction* insn_to_complete)
    {
				/* Yuhan's notes for implementation
					 For all instructions in opcode 60, the extended opcode is 21-29th bit.
					 special case for opcode 60: (Opcode table on Page 1190)
			  I. Decision Tree:
				  (1) For XX4 format, the 26&27 bits are 1. (only for opcde "xxsel" P773)
			    (2) Two special XX3 cases: 0..00 010.. & 0..01 010.., started from 21th bit. (manual 1208)
				  (3) Other instruction are all implemented normally, 
				    the "." bits are treated as 0 and 1 respectively in the opcode table
				II. The Rc bit for opcode 60 is the 21st bit

			   **	There are 2 XX1 instructions, the last bit of the extended opcodes are ignored (30th bit, which are both 0).
				*/

        isRAWritten = false;
        isFPInsn = false;
        bcIsConditional = false;
        insn = insn_to_complete->m_RawInsn.small_insn;
        const power_entry* current = &power_entry::main_opcode_table[field<0,5>(insn)];
        while(current->next_table)
        {
            current = &(std::mem_fn(current->next_table)(this));
        }
	if (findRAAndRS(current)) {
	    isRAWritten = true;
	}
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
            std::mem_fn(*curFn)(this);
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

        return true;
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


    /***** BEGIN: For new vector instructions *****/
    void InstructionDecoder_power::XT()
    {
        // TODO: Format DQ has a different encoding.
        //       The single T bit is at bit 28, instaed of bit 31.
        unsigned id = field<6, 10>(insn) + 32 * field<31, 31>(insn);
        insn_in_progress->appendOperand(
                        makeRegisterExpression(makePowerRegID(ppc64::vsr0, id)),
                        false, 
                        true);
    }
    void InstructionDecoder_power::XS()
    {
        // TODO: Format DQ has a different encoding.
        //       The single T bit is at bit 28, instaed of bit 31.
        unsigned id = field<6, 10>(insn) + 32 * field<31, 31>(insn);
        insn_in_progress->appendOperand(
                        makeRegisterExpression(makePowerRegID(ppc64::vsr0, id)),
                        true, 
                        false);
    }
    void InstructionDecoder_power::XA()
    {
        unsigned id = field<11, 15>(insn) + 32 * field<29, 29>(insn);
        insn_in_progress->appendOperand(
                        makeRegisterExpression(makePowerRegID(ppc64::vsr0, id)),
                        true, 
                        false);
    }
    void InstructionDecoder_power::XB()
    {
        unsigned id = field<16, 20>(insn) + 32 * field<30, 30>(insn);
        insn_in_progress->appendOperand(
                        makeRegisterExpression(makePowerRegID(ppc64::vsr0, id)),
                        true, 
                        false);
    }
    void InstructionDecoder_power::VRT()
    {
        unsigned id = field<6, 10>(insn) + 32;
        insn_in_progress->appendOperand(
                        makeRegisterExpression(makePowerRegID(ppc64::vsr0, id)),
                        false, 
                        true);
    }
    void InstructionDecoder_power::VRS()
    {
        unsigned id = field<6, 10>(insn) + 32;
        insn_in_progress->appendOperand(
                        makeRegisterExpression(makePowerRegID(ppc64::vsr0, id)),
                        true, 
                        false);
    }
    void InstructionDecoder_power::VRA()
    {
        unsigned id = field<11, 15>(insn) + 32;
        insn_in_progress->appendOperand(
                        makeRegisterExpression(makePowerRegID(ppc64::vsr0, id)),
                        true, 
                        false);
    }
    void InstructionDecoder_power::VRB()
    {
        unsigned id = field<16, 20>(insn) + 32 ;
        insn_in_progress->appendOperand(
                        makeRegisterExpression(makePowerRegID(ppc64::vsr0, id)),
                        true, 
                        false);
    }
    void InstructionDecoder_power::VRC()
    {
        unsigned id = field<21, 25>(insn) + 32 ;
        insn_in_progress->appendOperand(
                        makeRegisterExpression(makePowerRegID(ppc64::vsr0, id)),
                        true, 
                        false);
    }

    void InstructionDecoder_power::UIM()
    {
        //fprintf(stderr, "Unimplemented operand type UIM. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }

    void InstructionDecoder_power::BHRBE()
    {
        //fprintf(stderr, "Unimplemented operand type BHRBE. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }

    void InstructionDecoder_power::IH()
    {
        //fprintf(stderr, "Unimplemented operand type IH. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }

    void InstructionDecoder_power::SP()
    {
       // fprintf(stderr, "Unimplemented operand type SP. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }


    void InstructionDecoder_power::S()
    {
        //fprintf(stderr, "Unimplemented operand type S. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }


    void InstructionDecoder_power::TE()
    {
        //fprintf(stderr, "Unimplemented operand type TE. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }

    void InstructionDecoder_power::DGM()
    {
        //fprintf(stderr, "Unimplemented operand type DGM. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }

    void InstructionDecoder_power::DCM()
    {
        //fprintf(stderr, "Unimplemented operand type DCM. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }


    void InstructionDecoder_power::CT()
    {
//        fprintf(stderr, "Unimplemented operand type CT. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }
    void InstructionDecoder_power::RSP()
    {
//        fprintf(stderr, "Unimplemented operand type RSP. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }
    void InstructionDecoder_power::RTP()
    {
//        fprintf(stderr, "Unimplemented operand type RTP. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }
    void InstructionDecoder_power::EH()
    {
//        fprintf(stderr, "Unimplemented operand type EH. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }
    void InstructionDecoder_power::PRS()
    {
//        fprintf(stderr, "Unimplemented operand type PRS. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }

    void InstructionDecoder_power::A()
    {
//        fprintf(stderr, "Unimplemented operand type A. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }
    void InstructionDecoder_power::R()
    {
//        fprintf(stderr, "Unimplemented operand type R. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }
    void InstructionDecoder_power::BC()
    {
//        fprintf(stderr, "Unimplemented operand type BC. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }
    void InstructionDecoder_power::RC()
    {
//        fprintf(stderr, "Unimplemented operand type RC. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }
    void InstructionDecoder_power::RIC()
    {
//        fprintf(stderr, "Unimplemented operand type RIC. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }
    void InstructionDecoder_power::SIM()
    {
//        fprintf(stderr, "Unimplemented operand type SIM. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }

    void InstructionDecoder_power::DCMX()
    {
//        fprintf(stderr, "Unimplemented operand type DCMX. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }
    void InstructionDecoder_power::RO()
    {
//        fprintf(stderr, "Unimplemented operand type RO. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }
    void InstructionDecoder_power::RMC()
    {
//        fprintf(stderr, "Unimplemented operand type RMC. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }
    void InstructionDecoder_power::EX()
    {
//        fprintf(stderr, "Unimplemented operand type EX. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }
    void InstructionDecoder_power::SHB()
    {
        //fprintf(stderr, "Unimplemented operand type SHB. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }
    void InstructionDecoder_power::PS()
    {
        //fprintf(stderr, "Unimplemented operand type PS. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }
    void InstructionDecoder_power::CY()
    {
        //fprintf(stderr, "Unimplemented operand type CY. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }
    void InstructionDecoder_power::DRM()
    {
        //fprintf(stderr, "Unimplemented operand type DRM. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }

    void InstructionDecoder_power::SHW()
    {
        //fprintf(stderr, "Unimplemented operand type SHW. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }


    void InstructionDecoder_power::XC()
    {
        //fprintf(stderr, "Unimplemented operand type XC. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }

    void InstructionDecoder_power::DM()
    {
        //fprintf(stderr, "Unimplemented operand type DM. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }

    void InstructionDecoder_power::IMM8()
    {
        //fprintf(stderr, "Unimplemented operand type IMM8. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
    }

    /***** END: For new vector instructions *****/

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

        insn_in_progress = boost::shared_ptr<Instruction>(new Instruction(*insn_to_complete));
        decodeOperands(insn_in_progress.get());
        Instruction* iptr = const_cast<Instruction*>(insn_to_complete);
        *iptr = *(insn_in_progress.get());

    }
    MachRegister InstructionDecoder_power::makePowerRegID(MachRegister base, unsigned int encoding, int field_)
    {
        if(field_ != -1)
        {
            return makePowerRegID(base, encoding, field_ * 4, field_ * 4 + 3);
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
            const power_table::const_iterator entry_it = power_entry::extended_op_0.find(xo);
            if (entry_it == power_entry::extended_op_0.end())
                return invalid_entry;
            return entry_it->second;
        }
        const power_table::const_iterator entry_it = power_entry::extended_op_0.find(field<21, 30>(insn));
        if (entry_it == power_entry::extended_op_0.end())
            return invalid_entry;
        return entry_it->second;
    }

    const power_entry& InstructionDecoder_power::extended_op_4()
    {
        // Extended OpCode 4:
        //     First check bits 26-31. If there is a match were done.
        //     If not, XO is in bits 21-31. 
        power_table::const_iterator entry_it;

        switch (field<21, 31>(insn)) {
            case 1409:
                return extended_op_4_1409();
            case 1538:
                return extended_op_4_1538();
            case 1921:
                return extended_op_4_1921();
            default:
                break;
        }

        entry_it = power_entry::extended_op_4.find(field<21, 31>(insn));
        if (entry_it != power_entry::extended_op_4.end())
            return entry_it->second;

        entry_it = power_entry::extended_op_4.find(field<26, 31>(insn));
        if (entry_it != power_entry::extended_op_4.end())
            return entry_it->second;

        return invalid_entry;
    }

    const power_entry & InstructionDecoder_power::extended_op_4_1409() {

        const power_table::const_iterator entry_it = power_entry::extended_op_4_1409.find(field<11, 15>(insn));
        if (entry_it != power_entry::extended_op_4_1409.end())
            return entry_it->second;    
        return invalid_entry;

    }
    const power_entry & InstructionDecoder_power::extended_op_4_1538() {
        const power_table::const_iterator entry_it = power_entry::extended_op_4_1538.find(field<11, 15>(insn));
        if (entry_it != power_entry::extended_op_4_1538.end())
            return entry_it->second;    
        return invalid_entry;

    }

    const power_entry & InstructionDecoder_power::extended_op_4_1921() {
        const power_table::const_iterator entry_it = power_entry::extended_op_4_1921.find(field<11, 15>(insn));
        if (entry_it != power_entry::extended_op_4_1921.end())
            return entry_it->second;    
        return invalid_entry;
    }

    const power_entry& InstructionDecoder_power::extended_op_19()
    {
        const power_table::const_iterator entry_it = power_entry::extended_op_19.find(field<21, 30>(insn));
        if (entry_it == power_entry::extended_op_19.end())
            return invalid_entry;
        return entry_it->second;
    }
    const power_entry& InstructionDecoder_power::extended_op_30()
    {
	
        power_table::const_iterator entry_it;
	if (field<27,27>(insn) == 0)
	   entry_it = power_entry::extended_op_30.find(field<27, 29>(insn));
	else
	   entry_it = power_entry::extended_op_30.find(field<27, 30>(insn));
        if (entry_it == power_entry::extended_op_30.end())
            return invalid_entry;
        return entry_it->second;
    }
    const power_entry& InstructionDecoder_power::extended_op_31()
    {
        // sradi is a special instruction. Its xop is from 21 to 29 and its xop value is 413
        if (field<21,29>(insn) == 413) {
            const power_table::const_iterator entry_it = power_entry::extended_op_31.find(413);
            if (entry_it == power_entry::extended_op_31.end())
                return invalid_entry;
            return entry_it->second;
        }
        const power_entry* xoform_entry;
        const power_table::const_iterator entry_it = power_entry::extended_op_31.find(field<22, 30>(insn));
        if (entry_it == power_entry::extended_op_31.end())
            xoform_entry = &invalid_entry;
        else
            xoform_entry = &(entry_it->second);
        if(find(xoform_entry->operands.begin(), xoform_entry->operands.end(), &InstructionDecoder_power::OE)
           != xoform_entry->operands.end())
        {
            return *xoform_entry;
        }
        const power_table::const_iterator entry_it2 = power_entry::extended_op_31.find(field<21, 30>(insn));
        if (entry_it2 == power_entry::extended_op_31.end())
            return invalid_entry;
        return entry_it2->second;
    }
    // extended_op_57 needs revisiting
    const power_entry& InstructionDecoder_power::extended_op_57()
    {
        return power_entry::extended_op_57[field<30, 31>(insn)];
    }
    const power_entry& InstructionDecoder_power::extended_op_58()
    {
        const power_table::const_iterator entry_it = power_entry::extended_op_58.find(field<30, 31>(insn));
        if (entry_it == power_entry::extended_op_58.end())
            return invalid_entry;
        return entry_it->second;
    }
    const power_entry& InstructionDecoder_power::extended_op_59()
    {
        const power_table::const_iterator entry_it = power_entry::extended_op_59.find(field<21, 30>(insn));
        if (entry_it == power_entry::extended_op_59.end())
            return invalid_entry;
        return entry_it->second;
    }
    // extended_op_60 needs revisiting
    const power_entry& InstructionDecoder_power::extended_op_60_specials_check() {
	// If the power decoder is ever redone. Use masking to determine the instructions for 60.
	// Otherwise we are forced to do this fun hack....
	
	// Check for xxsel
	if (field<26,27>(insn) == 3)
		return power_entry::extended_op_60_specials[2];
	
	// xscmpexpdp
	if (field<21,28>(insn) == 59)
		return power_entry::extended_op_60_specials[5];
	// xscvuxddp	
	if (field<21,28>(insn) == 360)
		return power_entry::extended_op_60_specials[6];
	// xvdivsp
//	if (field<21,28>(insn) == 88) 
//		return extended_op_60_specials[1];

	// xvnmaddasp
	if (field<21,28>(insn) == 193) 
		return power_entry::extended_op_60_specials[4];
	// xvtdivsp
	if (field<21,28>(insn) == 93)
		return power_entry::extended_op_60_specials[1];

	// xxpermdi
	if (field<21,21>(insn) == 0 && field<24,28>(insn) == 10)
		return power_entry::extended_op_60_specials[0];

	if (field<21,21>(insn) == 0 && field<24,28>(insn) == 2)
		return power_entry::extended_op_60_specials[3];
	return invalid_entry;
    }
    const power_entry& InstructionDecoder_power::extended_op_60()
    {
	if (extended_op_60_specials_check().op != power_op_INVALID)
		return extended_op_60_specials_check();
        switch (field<21, 29>(insn)) {
            case 347:
                return extended_op_60_347();
            case 475:
                return extended_op_60_475();
            default:
                break;
        }

        const power_table::const_iterator entry_it = power_entry::extended_op_60.find(field<21, 29>(insn));
        if (entry_it == power_entry::extended_op_60.end())
            return invalid_entry;
        return entry_it->second;        
    }

    const power_entry& InstructionDecoder_power::extended_op_60_347() {
        const power_table::const_iterator entry_it = power_entry::extended_op_60_347.find(field<11, 15>(insn));
        if (entry_it == power_entry::extended_op_60_347.end())
            return invalid_entry;
        return entry_it->second;    

    }
    const power_entry& InstructionDecoder_power::extended_op_60_475() {
        const power_table::const_iterator entry_it = power_entry::extended_op_60_475.find(field<11, 15>(insn));
        if (entry_it == power_entry::extended_op_60_475.end())
            return invalid_entry;
        return entry_it->second;    
    }


    // extended_op_61 needs revisiting
    const power_entry& InstructionDecoder_power::extended_op_61()
    {
        unsigned int xo = field<26, 30>(insn);
        if(xo <= 31)
        {
            power_table::const_iterator found = power_entry::extended_op_61.find(xo);
            if(found != power_entry::extended_op_61.end())
                return found->second;
        }
        power_table::const_iterator found = power_entry::extended_op_61.find(field<21,30>(insn));
        if(found != power_entry::extended_op_61.end()) return found->second;
        return invalid_entry;
    }

    const power_entry& InstructionDecoder_power::extended_op_63()
    {
        switch (field<21, 30>(insn)) {
            case 583:
                return extended_op_63_583();
            case 804:
                return extended_op_63_804();
            case 836:
                return extended_op_63_836();
            default:
                break;
        }
        unsigned int xo = field<26, 26>(insn);
        if(xo == 1)
        {
            power_table::const_iterator found = power_entry::extended_op_63.find(field<26,30>(insn));
            if(found != power_entry::extended_op_63.end())
                return found->second;
        }
        const power_table::const_iterator entry_it = power_entry::extended_op_63.find(field<21, 30>(insn));
        if (entry_it == power_entry::extended_op_63.end())
            return invalid_entry;
        return entry_it->second;
    }
    const power_entry& InstructionDecoder_power::extended_op_63_583()
    { 
        const power_table::const_iterator entry_it = power_entry::extended_op_63_583.find(field<11, 15>(insn));
        if (entry_it == power_entry::extended_op_63_583.end())
            return invalid_entry;
        return entry_it->second;   
    }
    const power_entry& InstructionDecoder_power::extended_op_63_804()
    { 
        const power_table::const_iterator entry_it = power_entry::extended_op_63_804.find(field<11, 15>(insn));
        if (entry_it == power_entry::extended_op_63_804.end())
            return invalid_entry;
        return entry_it->second;   
    }
    const power_entry& InstructionDecoder_power::extended_op_63_836()
    { 
        const power_table::const_iterator entry_it = power_entry::extended_op_63_836.find(field<11, 15>(insn));
        if (entry_it == power_entry::extended_op_63_836.end())
            return invalid_entry;
        return entry_it->second;   
    }    
    void InstructionDecoder_power::FC() {
	// Used by lwat/ldat but usage is confusing 
	// 5 bits located at positions 16-21
	    
//        fprintf(stderr, "Unimplemented operand type FC. Please create an issue at https://github.com/dyninst/dyninst/issues\n");
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
            current = &(std::mem_fn(current->next_table)(this));
        }
        insn_in_progress = makeInstruction(current->op, current->mnemonic, 4, reinterpret_cast<unsigned char*>(&insn));
        if(current->op == power_op_b ||
          current->op == power_op_bc ||
          current->op == power_op_bclr ||
          current->op == power_op_bcctr)
        {
            // decode control-flow operands immediately; we're all but guaranteed to need them
            decodeOperands(insn_in_progress.get());
        }
	// FIXME in parsing
        insn_in_progress->arch_decoded_from = m_Arch;
        //insn_in_progress->arch_decoded_from = Arch_ppc32;
        if(field<0,5>(insn) == 0x04) {
            insn_in_progress->m_InsnOp.isVectorInsn = true;
        }
        return;
    }
  }
}



  
