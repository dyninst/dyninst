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

#include "InstructionDecoder-aarch64.h"
#include "Immediate.h"
#include <boost/assign/list_of.hpp>
#include "../../common/src/singleton_object_pool.h"

namespace Dyninst
{
  namespace InstructionAPI
  {
    typedef void (InstructionDecoder_aarch64::*operandFactory)();
    typedef std::vector<operandFactory> operandSpec;
    typedef std::vector<aarch64_insn_entry> aarch64_insn_table;
    typedef std::map<unsigned int, aarch64_mask_entry> aarch64_decoder_table;
    typedef std::map<unsigned int, unsigned int> branchMap;

    struct aarch64_insn_entry
    {
        aarch64_insn_entry(entryID o, const char* m, operandSpec ops):
        op(o), mnemonic(m), operands(ops)
        {
        }

        aarch64_insn_entry():
        op(aarch64_op_INVALID), mnemonic("INVALID")
        {
            // TODO: why 5?
            // TODO: Is this needed here?
            operands.reserve(5);
        }

        aarch64_insn_entry(const aarch64_insn_entry& o) :
        op(o.op), mnemonic(o.mnemonic), operands(o.operands)
        {
        }

        const aarch64_insn_entry& operator=(const aarch64_insn_entry& rhs)
        {
            operands.reserve(rhs.operands.size());
            op = rhs.op;
            mnemonic = rhs.mnemonic;
            operands = rhs.operands;

            return *this;
        }

        entryID op;
        const char* mnemonic;
        operandSpec operands;

        static void buildInsnTable();
        static bool built_insn_table;

        static aarch64_insn_table main_insn_table;
    };

    struct aarch64_mask_entry
    {
		aarch64_mask_entry(unsigned int m, branchMap bm, int tabIndex):
		mask(m), nodeBranches(bm), insnTableIndex(tabIndex)
		{
		}

		aarch64_mask_entry():
		mask(0), nodeBranches(branchMap()), insnTableIndex(-1)
		{
		}

		aarch64_mask_entry(const aarch64_mask_entry& e):
		mask(e.mask), nodeBranches(e.nodeBranches), insnTableIndex(e.insnTableIndex)
		{
		}

		const aarch64_mask_entry& operator=(const aarch64_mask_entry& rhs)
		{
			mask = rhs.mask;
			nodeBranches = rhs.nodeBranches;
			insnTableIndex = rhs.insnTableIndex;

			return *this;
		}

		unsigned int mask;
		branchMap nodeBranches;
		int insnTableIndex;

		static void buildDecoderTable();
		static bool built_decoder_table;
		static aarch64_decoder_table main_decoder_table;
	};

    InstructionDecoder_aarch64::InstructionDecoder_aarch64(Architecture a)
      : InstructionDecoderImpl(a), insn(0), insn_in_progress(NULL),
	    isRAWritten(false), invertBranchCondition(false), isValid(true),
        isFPInsn(false), is64Bit(false), isSystemInsn(false),
        hasHw(false), hasShift(false), hasOption(false), hasN(false),
        sField(0), nField(0), nLen(0), immr(0), immrLen(0),
        bcIsConditional(false)
    {
        aarch64_insn_entry::buildInsnTable();
        aarch64_mask_entry::buildDecoderTable();
    }

    InstructionDecoder_aarch64::~InstructionDecoder_aarch64()
    {
    }

    void InstructionDecoder_aarch64::decodeOpcode(InstructionDecoder::buffer& b)
    {
      b.start += 4;
    }

    using namespace std;
    Instruction::Ptr InstructionDecoder_aarch64::decode(InstructionDecoder::buffer& b)
    {
        insn_printf("### decoding\n");

     	if(b.start > b.end)
	    return Instruction::Ptr();

		isRAWritten = false;
        isFPInsn = false;
        bcIsConditional = false;
        isValid = true;
        is64Bit = false;

        hasHw = false;
        hw = 0;

        hasShift = false;
        shiftField = 0;
        
        hasOption = false;
        optionField = 0;
        
        hasN = false;
        sField = nField = nLen = 0;
        immr = immrLen = 0;
        
        isSystemInsn = false;
        op0 = op1 = op2 = crn = crm = 0;

        insn = b.start[0] << 24 | b.start[1] << 16 |
        b.start[2] << 8 | b.start[3];

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

    void InstructionDecoder_aarch64::setFPMode(bool )
    {
		isFPInsn = true;
    }

    bool InstructionDecoder_aarch64::decodeOperands(const Instruction *)
    {
		return false;
    }
	
	void InstructionDecoder_aarch64::processHwFieldInsn(int len, int val)
	{
		Result_Type rT = is64Bit?u64:u32;
				
		Expression::Ptr lhs = Immediate::makeImmediate(Result(rT, rT==u32?unsign_extend32<len>(val):unsign_extend64<len>(val)));
		Expression::Ptr rhs = Immediate::makeImmediate(Result(u32, 1<<(hw*16)));

		insn_in_progress->appendOperand(makeMultiplyExpression(lhs, rhs, rT), true, false);
	}
	
	void InstructionDecoder_aarch64::processShiftFieldShiftedInsn(int len, int val)
	{
		switch(shiftField)											//add-sub (shifted) and logical (shifted)
		{
			case 0:Result_Type rT = is64Bit?u64:u32;

				   Expression::Ptr lhs = makeRmExpr();
				   Expression::Ptr rhs = Immediate::makeImmediate(Result(u32, unsign_extend32<len>(val))));
			
				   insn_in_progress->appendOperand(makeMultiplyExpression(lhs, rhs, rT), true, false);
				   break;
			case 1://LSR #shiftAmount
				   break;
			case 2://ASR #shiftAmount
				   break;
			case 3:if(IS_INSN_LOGICAL_SHIFT(insn))					//logical (shifted) -- not applicable to add-sub (shifted)
						//ROR #shiftAmount
				   else
						isValid = false;
				   break;
		}
	}
	
	void InstructionDecoder_aarch64::processShiftFieldImmInsn(int len, int val)
	{
		if(shiftField == 0 || shiftField == 1)						//add-sub (immediate)
		{
			Result_Type rT = is64Bit?u64:u32;
			
			Expression::Ptr lhs = Immediate::makeImmediate(Result(rT, rT==u32?unsign_extend32<len>(val):unsign_extend64<len>(val)));
			Expression::Ptr rhs = Immediate::makeImmediate(Result(u32, 1<<(shiftField*12)));

			insn_in_progress->appendOperand(makeMultiplyExpression(lhs, rhs, rT), true, false);
		}
		else
		{
			isValid = false;
		}

	}
	
	void InstructionDecoder_aarch64::processOptionFieldExtendedInsn(int len, int val)
	{
		switch(optionField)
		{
			case 0://UXTB
					break;
			case 1://UXTH
					break;
			case 2://LSL(32) | UXTW
					break;
			case 3://LSL(64) | UXTX
					break;
			case 4://SXTB
					break;
			case 5://SXTH
					break;
			case 6://SXTW
					break;
			case 7://SXTX
					break;
		}
	}

	void InstructionDecoder_aarch64::processOptionFieldLSRegOffsetInsn()
	{
		if(optionField == 0x3)			//option = LSL
		{
			int sizeVal = field<30, 31>(insn), extend;

			if(field<23, 23>(insn) == 1)
				sizeVal = 4;

			extend = sField * sizeVal;
			int extendSize = 31;
			while(((extend << (31 - extendSize)) & 0x80000000) == 0)
				extendSize--;

			//above values need to be used in a dereference expression
		}
		else
		{
			//sign-extend
			switch(optionField)
			{
				case 0x2://UXTW
						 break;
				case 0x6://SXTW
						 break;
				case 0x7://SXTX
						 break;
				default:isValid = false;
						break;
			}
		}
	}

	void InstructionDecoder_aarch64::processSystemInsn()
	{
		if(op0 == 0)
		{
			if(crn == 3)			//clrex, dsb, dmb, isb
			{
				Expression::Ptr CRm = Immediate::makeImmediate(Result(u32, unsign_extend32<4>(crm)));

				insn_in_progress->appendOperand(CRm, true, false);
			}
			else if(crn == 2)
			{
				int immVal = (crm << 3)|(op2 & 7);		//hint

				Expression::Ptr imm = Immediate::makeImmediate(Result(u32, unsign_extend32<7>(immVal)));

				insn_in_progress->appendOperand(imm, true, false);
			}
			else if(crn == 4)
			{
				//msr immediate
				//affects pstate
			}
			else
			{
				isValid = false;
			}
		}
		else                  //sys, sysl, mrs, msr register
		{
			/*int immVal = op2 | (crm << 3) | (crn << 7) | (op1 << 11);
			immVal = (op0 >= 2)?(immVal << 1)|(op0 & 1):immVal;

			int immValLen = 14 + (op0 & 2);

			Expression::Ptr imm = Immediate::makeImmediate(Result(u32, unsign_extend32<immValLen>(immVal)));
				
			insn_in_progress->appendOperand(imm, true, false);*/
			assert(!"not implemented");

		}
	}

    void InstructionDecoder_aarch64::doDelayedDecode(const Instruction *insn_to_complete)
    {
		int insn_table_index = findInsnTableIndex(0);
		aarch64_insn_entry *insn_table_entry = &aarch64_insn_entry::main_insn_table[insn_table_index];
		
		insn = insn_to_complete->m_RawInsn.small_insn;
		insn_in_progress = const_cast<Instruction*>(insn_to_complete);
		
		//only a small subset of instructions modify pstate and the following are the ones for whom it cannot be detected from any operand that they do modify pstate
		//it thus needs to be read from the manual..yay
		if(((IS_INSN_ADDSUB_EXT(insn) || IS_INSN_ADDSUB_SHIFT(insn) || IS_INSN_ADDSUB_IMM(insn) || IS_INSN_ADDSUB_CARRY(insn)) 
			 && field<29, 29>(insn) == 0x1) ||
		   ((IS_INSN_LOGICAL_SHIFT(insn) || IS_INSN_LOGICAL_IMM(insn)) 
			 && field<29, 30>(insn) == 0x3) ||
		   (IS_INSN_FP_COMPARE(insn))
		   insn_in_progress->appendOperand(MakeRegisterExpression(makeAarch64RegID(aarch64::pstate, 0)), false, true);
		   
		
        for(operandSpec::const_iterator fn = insn_table_entry->operands.begin(); fn != insn_table_entry->operands.end(); fn++)
        {
			std::mem_fun(*fn)(this);
		}
		
		if(isSystemInsn)
		{
			processSystemInsn();
		}

		if(!isValid)
		{
			insn_in_progress = makeInstruction(aarch64_op_unallocated, "unallocated", 4, reinterpret_cast<unsigned char*>(0));
		}
    }

    Result_Type InstructionDecoder_aarch64::makeSizeType(unsigned int)
    {
        assert(0); //not implemented
        return u32;
    }

    // ****************
    // decoding opcodes
    // ****************

#define fn(x) (&InstructionDecoder_aarch64::x)

MachRegister InstructionDecoder_aarch64::makeAarch64RegID(MachRegister base, unsigned int encoding)
{
    return MachRegister(base.val() + encoding);
}

Expression::Ptr InstructionDecoder_aarch64::makeRdExpr()
{
	MachRegister baseReg = isFPinsn?aarch64::q0:aarch64::x0;
	
    int encoding = field<0, 4>(insn);
	if(encoding == 31)
		return makeRegisterExpression(makeAarch64RegID(aarch64::sp, 0));
	else
		return makeRegisterExpression(makeAarch64RegID(baseReg, encoding));
}


void InstructionDecoder_aarch64::Rd()
{
    insn_in_progress->appendOperand(makeRdExpr(), false, true);
}

Expression::Ptr InstructionDecoder_aarch64::makeRnExpr()
{
	MachRegister baseReg = isFPinsn?aarch64::q0:aarch64::x0;
	
	int encoding = field<5, 9>(insn);
	if(encoding == 31)
		return makeRegisterExpression(makeAarch64RegID(aarch64::sp, 0));
	else
		return makeRegisterExpression(makeAarch64RegID(baseReg, encoding));
}

Expression::Ptr InstructionDecoder_aarch64::makePCExpr()
{
	MachRegister baseReg = aarch64::pc;

	return makeRegisterExpression(makeAarch64RegID(baseReg, 0));
}

// load/store literal
// eg: load Xt, <literal>
// => offset = signextend(<literal>:00, 64)
// load from PC + offset
Expression::Ptr InstructionDecoder_aarch64::makeMemRefIndexLiteral(Result_Type size)
{

    // TODO: Sunny what does size in sign_extend64 mean? how much should I pass?
    int immVal = field<5, 23>(insn);
    Expression::Ptr imm = Immediate::makeImmediate(Result(s64, sign_extend64<64>(immVal<<2)));

    return makeDereferenceExpression(makeAddExpression(makePCExpr(), imm, s64), size);
}

template <Result_Type size>
void InstructionDecoder_aarch64::LIndex()
{
    if( IS_INSN_LOADSTORE_LITERAL(insn))
        insn_in_progress->appendOperand(makeMemRefIndexLiteral(size), true, false);
    // TODO if load  reg
    // TODO if load  imm
}

template <Result_Type size>
void InstructionDecoder_aarch64::STIndex()
{
    if( IS_INSN_LOADSTORE_LITERAL(insn))
        insn_in_progress->appendOperand(makeMemRefIndexLiteral(size), false, true);
    // TODO if store reg
    // TODO if store imm
}

// This function is for non-writeback
template<Result_Type size = u64>
void InstructionDecoder_aarch64::Rn()
{
    assert(0);
    /* this functions is useless
	insn_in_progress->appendOperand(makeRnExpr(), true, false);
    */
}

template<Result_Type size = u64>
void InstructionDecoder_aarch64::RnL()
{
    // TODO make a mem ref here
    // need to know the size of the mem ref
    LIndex<size>();
	insn_in_progress->appendOperand(makeRnExpr(), true, false);
}

template<Result_Type size = u64>
void InstructionDecoder_aarch64::RnS()
{
    // TODO make a mem ref here
    // need to know the size of the mem ref
    STIndex<size>();
	insn_in_progress->appendOperand(makeRnExpr(), true, false);
}

template<Result_Type size = u64>
void InstructionDecoder_aarch64::RnU()
{
    assert(0);
    /* this functions is useless
	insn_in_progress->appendOperand(makeRnExpr(), true, true);
    */
}

template<Result_Type size = u64>
void InstructionDecoder_aarch64::RnLU()
{
    // TODO make a mem ref here
    LIndex<size>();
	insn_in_progress->appendOperand(makeRnExpr(), true, true);
}

template<Result_Type size = u64>
void InstructionDecoder_aarch64::RnSU()
{
    // TODO make a mem ref here
    STIndex<size>();
	insn_in_progress->appendOperand(makeRnExpr(), true, true);
}

Expression::Ptr InstructionDecoder_aarch64::makeRmExpr()
{
	MachRegister baseReg = isFPinsn?aarch64::q0:aarch64::x0;

	return MakeRegisterExpression(makeAarch64RegID(baseReg, field<16, 20>(insn)));
}

void InstructionDecoder_aarch64::Rm()
{
	if(IS_INSN_LOADSTORE_REG(insn) ||
	   IS_INSN_ADDSUB_EXT(insn) ||
	   IS_INSN_ADDSUB_SHIFT(insn) ||
	   IS_INSN_LOGICAL_SHIFT(insn))
		return;
	
	//TODO: zero register (rm = 31, opc = 01 for FP)
		

	insn_in_progress->appendOperand(makeRmExpr(), true, false);
}

void InstructionDecoder_aarch64::sf()
{
	if(field<31>(insn) == 1)
		is64Bit = true;
}

template<unsigned int startBit, unsigned int endBit>
void InstructionDecoder_aarch64::option()
{
	hasOption = true;
	optionField = field<startBit, endBit>(insn);
}

void InstructionDecoder_aarch64::shift()
{
	hasShift = true;
	shiftField = field<22, 23>(insn);
}

void InstructionDecoder_aarch64::hw()
{
	hasHw = true;
	hw = field<21, 22>(insn);
}

template<unsigned int startBit, unsigned int endBit>
void InstructionDecoder_aarch64::N()
{
	nField = field<startBit, endBit>(insn);
	nLen = endBit - startBit + 1;
}

Expression::Ptr InstructionDecoder_aarch64::makeRtExpr()
{
	MachRegister baseReg = isFPinsn?aarch64::q0:aarch64::x0;

	return MakeRegisterExpression(makeAarch64RegID(baseReg, field<0, 4>(insn)));
}

void InstructionDecoder_aarch64::Rt()
{
	//sys, sysl, msr reg, mrs
	//others
}

void InstructionDecoder_aarch64::RtL()
{
	insn_in_progress->appendOperand(makeRtExpr(), false, true);
}

void InstructionDecoder_aarch64::RtS()
{
	insn_in_progress->appendOperand(makeRtExpr(), true, false);
}

Expression::Ptr InstructionDecoder_aarch64::makeRt2Expr()
{
	MachRegister baseReg = isFPinsn?aarch64::q0:aarch64::x0;

	return MakeRegisterExpression(makeAarch64RegID(baseReg, field<10, 14>(insn)));
}

void InstructionDecoder_aarch64::Rt2()
{
    assert(0);
}

void InstructionDecoder_aarch64::Rt2L()
{
	insn_in_progress->appendOperand(makeRt2Expr(), false, true);
}

void InstructionDecoder_aarch64::Rt2S()
{
	insn_in_progress->appendOperand(makeRt2Expr(), true, false);
}

template<unsigned int startBit, unsigned int endBit>
void InstructionDecoder_aarch64::cond()
{
}

void InstructionDecoder_aarch64::nzcv()
{
	MachRegister baseReg = makeAarch64RegID(aarch64::pstate, 0);
	insn_in_progress->appendOperand(MakeRegisterExpression(baseReg, true, true);
	
	unsigned char nzcvVal = static_cast<unsigned char>(field<0, 3>(insn) & 0xF);
	Expression::Ptr nzcv = Immediate::makeImmediate(Result(u8, nzcvVal));
	insn_in_progress->appendOperand(nzcv, true, false);
}

void InstructionDecoder_aarch64::op1()
{
	op1 = field<16, 18>(insn);
}

void InstructionDecoder_aarch64::op2()
{
	op2 = field<5, 7>(insn);
}

void InstructionDecoder_aarch64::CRm()
{
	isSystemInsn = true;
	crm = field<8 ,11>(insn);
}

void InstructionDecoder_aarch64::CRn()
{
	crn = field<12, 15>(insn);
}

template<unsigned int startBit, unsigned int endBit>
void InstructionDecoder_aarch64::S()
{
	sField = field<startBit, endBit>(insn);
}

void InstructionDecoder_aarch64::scale()
{
	int scaleVal = 64 - field<10, 15>(insn);
	
	Expression::Ptr scale = Immediate::makeImmediate(Result(u32, unsign_extend32<6>(scaleVal)));
	insn_in_progress->appendOperand(scale, true, false);
}

Expression::Ptr makeRaExpr()
{
	MachRegister baseReg = isFPinsn?aarch64::q0:aarch64::x0;

	return makeRegisterExpression(makeAarch64RegID(baseReg, field<10, 14>(insn)));
}

void InstructionDecoder_aarch64::Ra()
{
	insn_in_progress->appendOperand(makeRaExpr(), true, false);
}

void InstructionDecoder_aarch64::o0()
{
	op0 = field<19, 20>(insn);
}

void InstructionDecoder_aarch64::b5()
{
	sf();
}

void InstructionDecoder_aarch64::b40()
{
}

/*void InstructionDecoder_aarch64::sz()
{
}*/

Expression::Ptr makeRsExpr()
{
	MachRegister baseReg = isFPinsn?aarch64::q0:aarch64::x0;

	return makeRegisterExpression(makeAarch64RegID(baseReg, field<16, 20>(insn)));
}

void InstructionDecoder_aarch64::Rs()
{
	insn_in_progress->appendOperand(makeRsExpr(), false, true);
}

template<unsigned int startBit, unsigned int endBit>
void InstructionDecoder_aarch64::imm()
{
	int immVal = field<startBit, endBit>(insn);
	int immLen = endBit - startBit + 1;
	
	if(hasHw)
	{
		if(IS_INSN_MOVEWIDE_IMM(insn))		//movewide (immediate)
		{
			processHwFieldInsn(immLen, immVal);
		}
		else
		{
			isValid = false;
		}
	}
	else if(hasN)		//logical (immediate), bitfield, extract
	{
		if(IS_FIELD_IMMR(startBit, endBit))
		{
			immr = immVal;
			immrLen = immLen;
			
			if(IS_INSN_BITFIELD(insn))
			{
				Expression::Ptr imm = Immediate::makeImmediate(Result(u32, unsign_extend32<immrLen>(immr)));
				insn_in_progress->appendOperand(imm, true, false);
			}
		}
		else if(IS_FIELD_IMMS(startBit, endBit))
		{
			Expression::Ptr imm;
			
			if(IS_INSN_LOGICAL_IMM(insn))
			{
				immVal |= (immr << immLen);
				immVal |= (nField << (immLen + immrLen));
				
				immLen += nLen + immrLen;
				
				Result_Type rT = is64Bit?u64:u32;
				imm = Immediate::makeImmediate(Result(rT, rT==u32?unsign_extend32<immLen>(immVal):unsign_extend64<immLen>(immVal)));
			}
			else
				imm = Immediate::makeImmediate(Result(u32, unsign_extend32<immLen>(immVal)));
			
			insn_in_progress->appendOperand(imm, true, false);
		}
	}
	else if(hasShift)
	{
		if(IS_INSN_ADDSUB_SHIFT(insn) || IS_INSN_LOGICAL_SHIFT(insn))	//add-sub shifted | logical shifted
		{
			processShiftFieldShiftedInsn(immLen, immVal);
		}
		else if(IS_INSN_ADDSUB_IMM(insn))		//add-sub (immediate)
		{
			processShiftFieldImmInsn(immLen, immVal);
		}
		else
		{
			isValid = false;
		}
	}
	else if(hasOption)
	{
		if(IS_INSN_ADDSUB_EXT(insn))										//add-sub extended
		{
			processOptionFieldExtendedInsn(immLen, immVal);
		}
		else if(IS_INSN_LOADSTORE_REG(insn))	//load-store register offset
		{
			//STEVE: You can handle load store here by calling processOptionFieldLSRegOffset, or modify it as required
		}
		else
		{
			isValid = false;
		}
	}
	else
	{	
		if(IS_FP_IMM(insn))
		{

		}
		else                                                            //exception, conditional compare (immediate)
		{
			Result_Type rT = is64Bit?u64:u32;

			Expression::Ptr imm = Immediate::makeImmediate(Result(rT, rT==u32?unsign_extend32<immLen>(immVal):unsign_extend64<immLen>(immVal)));
			insn_in_progress->appendOperand(imm, true, false);
		}
	}
}


using namespace boost::assign;

#include "aarch64_opcode_tables.C"

	int InstructionDecoder_aarch64::findInsnTableIndex(unsigned int decoder_table_index)
	{
		aarch64_mask_entry *cur_entry = &aarch64_mask_entry::main_decoder_table[decoder_table_index];
		unsigned int cur_mask = cur_entry->mask;

		if(cur_mask == 0)
			return cur_entry->insnTableIndex;

		unsigned int insn_iter_index = 0, map_key_index = 0, branch_map_key = 0;
		branchMap cur_branches = cur_entry->nodeBranches;
		
		while(insn_iter_index <= AARCH64_INSN_LENGTH)
		{
			if(((cur_mask>>insn_iter_index) & 1) == 1)
			{
				branch_map_key = branch_map_key | (((insn>>insn_iter_index) & 1)<<map_key_index);
				map_key_index++;
			}
			insn_iter_index++;
		}

		if(cur_branches.count(branch_map_key) <= 0)
			branch_map_key = 0;

		return findInsnTableIndex(cur_branches[branch_map_key]);
	}

    void InstructionDecoder_aarch64::mainDecode()
    {
		int insn_table_index = findInsnTableIndex(0);

		aarch64_insn_entry *insn_table_entry = &aarch64_insn_entry::main_insn_table[insn_table_index];

        insn_in_progress = makeInstruction(insn_table_entry->op, insn_table_entry->mnemonic, 4, reinterpret_cast<unsigned char*>(&insn));
        insn_printf("ARM: %s\n", insn_table_entry->mnemonic);

        // control flow operations?
        /* tmp commented this part
        if(current->op == power_op_b ||
          current->op == power_op_bc ||
          current->op == power_op_bclr ||
          current->op == power_op_bcctr)
        {
            // decode control-flow operands immediately; we're all but guaranteed to need them
            doDelayedDecode(insn_in_progress);
        }
        */

        //insn_in_progress->arch_decoded_from = m_Arch;
        insn_in_progress->arch_decoded_from = Arch_aarch64;
        return;
    }
  };
};



