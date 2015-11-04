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
      : InstructionDecoderImpl(a), isPstateRead(false), isPstateWritten(false), isFPInsn(false),isSIMDInsn(false),
	    is64Bit(true), isValid(true), insn(0), insn_in_progress(NULL), isSystemInsn(false),
        hasHw(false), hasShift(false), hasOption(false), hasN(false),
        immr(0), immrLen(0), sField(0), nField(0), nLen(0),
        immlo(0), immloLen(0), _szField(-1)
    {
        aarch64_insn_entry::buildInsnTable();
        aarch64_mask_entry::buildDecoderTable();

        invalid_insn = makeInstruction(aarch64_op_INVALID, "INVALID", 4, reinterpret_cast<unsigned char*>(&insn));
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
        //insn_printf("### decoding\n");

     	if(b.start > b.end)
	    return Instruction::Ptr();

		isPstateRead = isPstateWritten = false;
        isFPInsn = false;
        isSIMDInsn = false;
        isValid = true;
        is64Bit = true;

        hasHw = false;
        hwField = 0;

        hasShift = false;
        shiftField = 0;

        hasOption = false;
        optionField = 0;

        hasN = false;
        sField = nField = nLen = 0;
        immr = immrLen = 0;

        isSystemInsn = false;
        op0Field = op1Field = op2Field = crnField = crmField = 0;

        immlo = immloLen = 0;

        _szField = -1;

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

    /* replace this function with a more generic function, which is setRegWidth
    void InstructionDecoder_aarch64::set32Mode()
    {
        // NOTE: is64Bit is set by default.
		is64Bit = false;
    }
    */

    void InstructionDecoder_aarch64::setFPMode()
    {
        // NOTE: if it is fp, only isFP is set.
		isFPInsn = true;
    }

    void InstructionDecoder_aarch64::setSIMDMode()
    {
        // NOTE: if it is SIMD insn, both isFP and isSIMD are set.
		isFPInsn = true;
		isSIMDInsn = true;
    }

	template<unsigned int endBit, unsigned int startBit>
    void InstructionDecoder_aarch64::OPRtype(){
        _typeField = field<startBit, endBit>(insn);
    }

    bool InstructionDecoder_aarch64::decodeOperands(const Instruction *)
    {
		return false;
    }

	void InstructionDecoder_aarch64::processHwFieldInsn(int len, int val)
	{
		Result_Type rT = is64Bit?u64:u32;

		unsigned int shiftAmount = hwField*16;

		Expression::Ptr lhs = Immediate::makeImmediate(Result(rT, rT==u32?unsign_extend32(len, val):unsign_extend64(len, val)));
		Expression::Ptr rhs = Immediate::makeImmediate(Result(u32, unsign_extend32(6, shiftAmount)));

		insn_in_progress->appendOperand(makeLeftShiftExpression(lhs, rhs, rT), true, false);
	}

	void InstructionDecoder_aarch64::processShiftFieldShiftedInsn(int len, int val)
	{
		Result_Type rT;
		Expression::Ptr lhs, rhs;

		rT = is64Bit?u64:u32;

  	    lhs = makeRmExpr();
	    rhs = Immediate::makeImmediate(Result(u32, unsign_extend32(len, val)));

		switch(shiftField)											//add-sub (shifted) and logical (shifted)
		{
			case 0:insn_in_progress->appendOperand(makeLeftShiftExpression(lhs, rhs, rT), true, false);
				   break;
			case 1:insn_in_progress->appendOperand(makeRightLogicalShiftExpression(lhs, rhs, rT), true, false);
				   break;
			case 2:insn_in_progress->appendOperand(makeRightArithmeticShiftExpression(lhs, rhs, rT), true, false);
				   break;
			case 3:if(IS_INSN_LOGICAL_SHIFT(insn))					//logical (shifted) -- not applicable to add-sub (shifted)
						insn_in_progress->appendOperand(makeRightRotateExpression(lhs, rhs, rT), true, false);
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

			unsigned int shiftAmount = shiftField * 12;

			Expression::Ptr lhs = Immediate::makeImmediate(Result(rT, rT==u32?unsign_extend32(len, val):unsign_extend64(len, val)));
			Expression::Ptr rhs = Immediate::makeImmediate(Result(u32, unsign_extend32(4, shiftAmount)));

			insn_in_progress->appendOperand(makeLeftShiftExpression(lhs, rhs, rT), true, false);
		}
		else
		{
			isValid = false;
		}

	}

	Expression::Ptr InstructionDecoder_aarch64::makeOptionExpression(int len, int val)
	{
		MachRegister baseReg = isFPInsn?
            (isSinglePrec()?aarch64::s0:aarch64::d0):
            (is64Bit?aarch64::x0:aarch64::w0);

		int encoding = field<16, 20>(insn);
		if(!isFPInsn && field<16, 20>(insn) == 31)
			baseReg = aarch64::zr;
		else
			baseReg = MachRegister(baseReg.val() + encoding);

		switch(optionField)
		{
			case 0:return makeRegisterExpression(baseReg, u8);
			case 1:return makeRegisterExpression(baseReg, u16);
			case 2:if(!is64Bit && (field<0, 4>(insn) == 31 || field<5, 9>(insn) == 31) && val != 0)
					   return makeLeftShiftExpression(makeRegisterExpression(baseReg), Immediate::makeImmediate(Result(u32, unsign_extend32(len, val))), u32);
				   else
				   	   return makeRegisterExpression(baseReg, u32);
			case 3:if(is64Bit && (field<0, 4>(insn) == 31 || field<5, 9>(insn) == 31) && val != 0)
					   return makeLeftShiftExpression(makeRegisterExpression(baseReg), Immediate::makeImmediate(Result(u32, unsign_extend32(len, val))), u64);
				   else
					   return makeRegisterExpression(baseReg, u64);
			case 4:return makeRegisterExpression(baseReg, s8);
			case 5:return makeRegisterExpression(baseReg, s16);
			case 6:return makeRegisterExpression(baseReg, s32);
			case 7:return makeRegisterExpression(baseReg, s64);
			default: assert(!"invalid option field value");
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
		if(op0Field == 0)
		{
			if(crnField == 3)			//clrex, dendBit, dmb, iendBit
			{
				Expression::Ptr CRm = Immediate::makeImmediate(Result(u32, unsign_extend32(4, crmField)));

				insn_in_progress->appendOperand(CRm, true, false);
			}
			else if(crnField == 2)
			{
				int immVal = (crmField << 3)|(op2Field & 7);		//hint

				Expression::Ptr imm = Immediate::makeImmediate(Result(u32, unsign_extend32(7, immVal)));

				insn_in_progress->appendOperand(imm, true, false);
			}
			else if(crnField == 4)
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

    Result_Type InstructionDecoder_aarch64::makeSizeType(unsigned int)
    {
        assert(0); //not implemented
        return u32;
    }

    // ****************
    // decoding opcodes
    // ****************

#define fn(x) (&InstructionDecoder_aarch64::x)
#define	COMMA	,

MachRegister InstructionDecoder_aarch64::makeAarch64RegID(MachRegister base, unsigned int encoding)
{
    return MachRegister(base.val() + encoding);
}

Expression::Ptr InstructionDecoder_aarch64::makeRdExpr()
{
	MachRegister baseReg = (isFPInsn && !IS_INSN_FP_CONV_INT(insn))?
        (isSinglePrec()?aarch64::s0:aarch64::d0):
        (is64Bit?aarch64::x0:aarch64::w0);

    int encoding = field<0, 4>(insn);
	if(encoding == 31)
		return makeRegisterExpression(makeAarch64RegID(is64Bit?aarch64::sp:aarch64::wsp, 0));
	else
		return makeRegisterExpression(makeAarch64RegID(baseReg, encoding));
}


void InstructionDecoder_aarch64::OPRRd()
{
    insn_in_progress->appendOperand(makeRdExpr(), false, true);
}

Expression::Ptr InstructionDecoder_aarch64::makeRnExpr()
{
	MachRegister baseReg = (isFPInsn && !IS_INSN_FP_CONV_FIX(insn))?
        (isSinglePrec()?aarch64::s0:aarch64::d0):
        (is64Bit?aarch64::x0:aarch64::w0);

	int encoding = field<5, 9>(insn);
	if(encoding == 31)
		return makeRegisterExpression(makeAarch64RegID(is64Bit?aarch64::sp:aarch64::wsp, 0));
	else
		return makeRegisterExpression(makeAarch64RegID(baseReg, encoding));
}

Expression::Ptr InstructionDecoder_aarch64::makePCExpr()
{
	MachRegister baseReg = aarch64::pc;

	return makeRegisterExpression(makeAarch64RegID(baseReg, 0));
}

Expression::Ptr InstructionDecoder_aarch64::makePstateExpr()
{
	MachRegister baseReg = aarch64::pstate;

	return makeRegisterExpression(makeAarch64RegID(baseReg, 0));
}


void InstructionDecoder_aarch64::getMemRefIndexLiteral_OffsetLen(int &immVal, int &immLen){
    immVal = field<5, 23>(insn);
    immVal = immVal << 2; //immVal:00
    immLen = (23 - 5 + 1) + 2;
    return;
}

void InstructionDecoder_aarch64::getMemRefIndexLiteral_RT(Result_Type &rt){
    int size = field<30, 31>(insn);
    switch(size){
        case 0x0:
            rt = u32;
            break;
        case 0x1:
            rt = u64;
            break;
        case 0x2:
            rt = s32;
            break;
        case 0x3:
        default:
            assert(0);
            break;
    }
    return;
}

// ****************************************
// load/store literal
// eg: load Xt, <literal>
// => offset = signextend(<literal>:00, 64)
// load from [PC + offset] to Xt
// ****************************************
Expression::Ptr InstructionDecoder_aarch64::makeMemRefIndexLiteral()
{
    int immVal, immLen;
    getMemRefIndexLiteral_OffsetLen(immVal, immLen);

    Expression::Ptr label = Immediate::makeImmediate(Result(s64, sign_extend64(immLen, immVal)));

    Result_Type rt;
    getMemRefIndexLiteral_RT(rt);

    return makeDereferenceExpression(makeAddExpression(label, makePCExpr(), u64), rt);
}

// TODO potential bug: do we need to distinguish signed and unsigned here?
// this funciton is to get the mem ref size
// shared by ld/st uimm, post, pre, reg
void InstructionDecoder_aarch64::getMemRefIndex_RT(Result_Type &rt){
    unsigned int opc1 = field<23, 23>(insn);
    unsigned int size = field<30, 31>(insn);

    if(opc1 == 1){
        switch(size){
            case 0:
                rt = s8;
                break;
            case 1:
                rt = s16;
                break;
            case 2:
                rt = s32;
                break;
            case 3:
                rt = s64;
                break;
            default:
                assert(0);
                //should only be 2 or 3
                break;
        }
    }else{
        switch(size){
            case 0:
                rt = u8;
                break;
            case 1:
                rt = u16;
                break;
            case 2:
                rt = u32;
                break;
            case 3:
                rt = u64;
                break;
            default:
                assert(0);
                //should only be 2 or 3
                break;
        }
    }

    return;
}

void InstructionDecoder_aarch64::getMemRefPair_RT(Result_Type &rt){
    unsigned int isSigned = field<30,30>(insn);
    unsigned int size = field<31, 31>(insn);

    // double the width
    switch(isSigned){
        case 0:
            switch(size){
                case 0:
                    //rt = u32;
                    rt = u64;
                    break;
                case 1:
                    //rt = u64;
                    rt = dbl128;
                    break;
                default:
                    assert(0);
            }
            break;
        case 1:
            switch(size){
                case 0:
                    //rt = s32;
                    rt = s64;
                    break;
                case 1:
                default:
                    assert(0);
            }
            break;
        default:
            assert(0);
            break;
    }

    return;
}

void InstructionDecoder_aarch64::getMemRefIndex_SizeSizelen(unsigned int &size, unsigned int &sizeLen){
    size = field<30, 31>(insn);
    sizeLen = 31-30+1;
    return;
}

void InstructionDecoder_aarch64::getMemRefIndexPrePost_ImmImmlen(unsigned int &immVal, unsigned int &immLen){
    immVal = field<12, 20>(insn);
    immLen = 20 - 12 + 1;
    return;
}

void InstructionDecoder_aarch64::getMemRefPair_ImmImmlen(unsigned int &immVal, unsigned int &immLen){
    immVal = field<15, 21>(insn);
    immLen = 21-15 + 1;
    return;
}

// ****************************************
// load/store unsigned imm
// eg: load Xt, [Xn, #imm]
// => offset = unsignextend( imm , 64)
// load from [PC + offset] to Xt
// ****************************************
Expression::Ptr InstructionDecoder_aarch64::makeMemRefIndexUImm()
{
    assert( IS_INSN_LDST_UIMM(insn) );

    int immVal = field<10, 21>(insn);
    int immLen = 21 - 10 + 1;

    unsigned int size = 0, sizeLen = 0;
    getMemRefIndex_SizeSizelen(size, sizeLen);

    Expression::Ptr offset = Immediate::makeImmediate( Result(u64, unsign_extend64( immLen+size, immVal<<size) ) );

    /*
    Expression::Ptr scaleForUImm = Immediate::makeImmediate(Result(u32, unsign_extend32(sizeLen + size, 1<<size)));
    Expression::Ptr offset = makeMultiplyExpression(imm, scaleForUImm, u64);
    */

    Result_Type rt;
    getMemRefIndex_RT(rt);
    return makeDereferenceExpression( makeAddExpression(makeRnExpr(), offset, u64), rt);
}

Expression::Ptr InstructionDecoder_aarch64::makeMemRefIndex_offset9(){
    unsigned int immVal = 0, immLen = 0;
    getMemRefIndexPrePost_ImmImmlen(immVal, immLen);
    return Immediate::makeImmediate(Result(u64, sign_extend64(immLen, immVal)));
}

// scale = 2 + opc<1>
// scale = 1<<scale
// LSL(sign_ex(imm7 , 64), scale)
Expression::Ptr InstructionDecoder_aarch64::makeMemRefPair_offset7(){
    unsigned int scaleVal = field<31, 31>(insn);
    unsigned int scaleLen = 1;
    scaleVal += 2;

    Expression::Ptr scale = Immediate::makeImmediate(Result(u32, unsign_extend32(scaleLen, 1<<scaleVal)));

    unsigned int immVal = 0, immLen = 0;
    getMemRefPair_ImmImmlen(immVal, immLen);

    Expression::Ptr imm7 = Immediate::makeImmediate(Result(u64, sign_extend64(immLen, immVal)));

    return makeMultiplyExpression(imm7, scale, s64);
}

Expression::Ptr InstructionDecoder_aarch64::makeMemRefIndex_addOffset9(){
    Expression::Ptr offset = makeMemRefIndex_offset9();
    return makeAddExpression(makeRnExpr(), offset, u64);
}

Expression::Ptr InstructionDecoder_aarch64::makeMemRefPair_addOffset7(){
    Expression::Ptr offset = makeMemRefPair_offset7();
    return makeAddExpression(makeRnExpr(), offset, u64);
}

Expression::Ptr InstructionDecoder_aarch64::makeMemRefIndexPre(){
    Result_Type rt;
    getMemRefIndex_RT(rt);
    return makeDereferenceExpression(makeMemRefIndex_addOffset9(), rt);
}

Expression::Ptr InstructionDecoder_aarch64::makeMemRefIndexPost(){
    Result_Type rt;
    getMemRefIndex_RT(rt);
    return makeDereferenceExpression(makeRnExpr(), rt);
}

Expression::Ptr InstructionDecoder_aarch64::makeMemRefPairPre(){
    Result_Type rt;
    getMemRefPair_RT(rt);
    return makeDereferenceExpression(makeMemRefPair_addOffset7(), rt);
}

/*
Expression::Ptr InstructionDecoder_aarch64::makeMemRefPairPre2(){
    // datasize = 64/32 /8;
    unsigned int opc1 = field<31, 31>(insn);
    unsigned int sizeVal = 1<<(2+opc1); //4 or 8 bytes

    Expression::Ptr dataSize = Immediate::makeImmediate(Result(u32, unsign_extend32( 32, sizeVal ) ) );
    Expression::Ptr pair2 = makeAddExpression( makeMemRefPair_addOffset7(), dataSize, u64);

    Result_Type rt;
    getMemRefPair_RT(rt);
    return makeDereferenceExpression(pair2, rt);
}
*/

Expression::Ptr InstructionDecoder_aarch64::makeMemRefPairPost(){
    Result_Type rt;
    getMemRefPair_RT(rt);
    return makeDereferenceExpression(makeRnExpr(), rt);
}

/*
Expression::Ptr InstructionDecoder_aarch64::makeMemRefPairPost2(){
    // datasize = 64/32 /8;
    unsigned int opc1 = field<31, 31>(insn);
    unsigned int sizeVal = 1<<(2+opc1); //4 or 8 bytes

    Expression::Ptr dataSize = Immediate::makeImmediate(Result(u32, unsign_extend32( 32, sizeVal ) ) );
    Expression::Ptr pair2 = makeAddExpression( makeRnExpr(), dataSize, u64);

    Result_Type rt;
    getMemRefPair_RT(rt);
    return makeDereferenceExpression(pair2, rt);
}
*/

void InstructionDecoder_aarch64::getMemRefEx_RT(Result_Type &rt){
    unsigned int sz = field<30, 31>(insn);
    switch(sz){
        case 0x00: //B
            rt = u8;
            break;
        case 0x01: //H
            rt = u16;
            break;
        case 0x10: //32b
            rt = u32;
            break;
        case 0x11: //64b
            rt = u64;
            break;
        default:
            rt = u64;
            insn_printf("[ERROR]: 0x%x\n", insn);
            //assert(0);// shouldn't reach here;
    }
    return;
}

Expression::Ptr InstructionDecoder_aarch64::makeMemRefEx(){
    Result_Type rt;
    getMemRefEx_RT(rt);
    return makeDereferenceExpression(makeRnExpr(), rt);
}

void InstructionDecoder_aarch64::getMemRefExPair_RT(Result_Type &rt){
    int size = field<30, 30>(insn);
    switch(size){
        case 0:
            rt = u64;
            break;
        case 1:
            rt = dbl128;
            break;
        default:
            assert(!"should not reach here");
            break;
    }
    return;
}

Expression::Ptr InstructionDecoder_aarch64::makeMemRefExPair(){
    Result_Type rt;
    getMemRefExPair_RT(rt);
    return makeDereferenceExpression(makeRnExpr(), rt);
}

/*
Expression::Ptr InstructionDecoder_aarch64::makeMemRefExPair16B(){
    return makeDereferenceExpression(makeRnExpr(), dbl128);
}
*/

/*
Expression::Ptr InstructionDecoder_aarch64::makeMemRefExPair2(){
    unsigned int immLen = 4, immVal = 8;
    Expression::Ptr offset = Immediate::makeImmediate(Result(u32, unsign_extend32(immLen, immVal)));
    return makeDereferenceExpression(makeAddExpression(makeRnExpr(), offset, u64) , u64);
}
*/

Expression::Ptr InstructionDecoder_aarch64::makeMemRefReg_Rm(){
    unsigned int option = field<13, 15>(insn);
    if((option&2) != 2) assert(0);
    MachRegister baseReg = ((option & 0x3)==0x2) ? aarch64::w0 : aarch64::x0;
    unsigned int encoding = field<16, 20>(insn);

	if( encoding == 31 )//zero register
		return makeRegisterExpression(makeAarch64RegID(aarch64::zr, 0));
    else
        return makeRegisterExpression(makeAarch64RegID(baseReg, encoding));
}

Expression::Ptr InstructionDecoder_aarch64::makeMemRefReg_amount(){
    unsigned int S =  field<12, 12>(insn);
    unsigned int amountVal = is64Bit?(S==0?0:2):(S==0?0:3);
    unsigned int amountLen = 2;

    return Immediate::makeImmediate(Result(u32, unsign_extend32(amountLen, amountVal)) ) ;
}

Expression::Ptr InstructionDecoder_aarch64::makeMemRefReg_ext(){
    //TODO
    /*
			int sizeVal = field<30, 31>(insn), extend;

			if(field<23, 23>(insn) == 1)
				sizeVal = 4;

			extend = sField * sizeVal;
			int extendSize = 31;
			while(((extend << (31 - extendSize)) & 0x80000000) == 0)
				extendSize--;
    */


    int immLen = 2;
    int immVal = 0; //for amount

    int S = field<12,12>(insn);
    int size = field<30,31>(insn);

    if( size == 2 ){ //32bit
        immVal = S==0?0:(S==1?2:-1);
        if( immVal==-1 ) assert(0);
    }else if( size == 3 ){ //64bit
        immVal = S==0?0:(S==1?3:-1);
        if( immVal==-1) assert(0);
    }else{
        //insn_printf("[WARN] unhandled case: 0x%x\n", insn);
        //assert(0); //unregconized val
    }

    Expression::Ptr ext = makeOptionExpression(immLen, immVal);
    return ext; /*extended ptr*/
}

/**********************
// ld/st reg memReg AST:
//    memRef
//      |
//     ADD
//    /    \
//  [Rn]   EXT
//        /   \
//      [Rm] [amount]
**********************/
Expression::Ptr InstructionDecoder_aarch64::makeMemRefReg(){

    Expression::Ptr ext = makeMemRefReg_ext();
    Expression::Ptr xn = makeRnExpr();
    Expression::Ptr add = makeAddExpression(xn, ext, u64);

    Result_Type rt;
    getMemRefIndex_RT(rt);
    return makeDereferenceExpression(add,rt);
}

void InstructionDecoder_aarch64::LIndex()
{
    if( IS_INSN_LD_LITERAL(insn)){
        insn_in_progress->appendOperand(makeMemRefIndexLiteral(), true, false);
        return;
    }

    // ******************
    // ld/st register offset
    // ******************
    else if( IS_INSN_LDST_REG(insn)){
        insn_in_progress->appendOperand(makeMemRefReg(), true, false);
        return;
    }

    // ******************
    // ld/st unsigned imm
    // ******************
    else if( IS_INSN_LDST_UIMM(insn)){
        insn_in_progress->appendOperand(makeMemRefIndexUImm(), true, false);
        return;
    }

    // ******************
    // ld/st pre and post, unscaled and unprivlidged
    // ******************
    else if( IS_INSN_LDST_PRE(insn)
        || IS_INSN_LDST_UNPRIV(insn)
        || IS_INSN_LDST_UNSCALED(insn) ){
        insn_in_progress->appendOperand(makeMemRefIndexPre(), true, false);
        return;
    }

    else if( IS_INSN_LDST_POST(insn) ){
        insn_in_progress->appendOperand(makeMemRefIndexPost(), true, false);
        return;
    }

    // ****************************
    // ld/st PAIR pre, post, offset
    // ****************************
    else if( IS_INSN_LDST_PAIR_PRE(insn) ){
        insn_in_progress->appendOperand(makeMemRefPairPre(), true, false);
        return;
    }

    else if( IS_INSN_LDST_PAIR_POST(insn)
        || IS_INSN_LDST_PAIR_OFFSET(insn)
        || IS_INSN_LDST_PAIR_NOALLOC(insn) ){
        insn_in_progress->appendOperand(makeMemRefPairPost(), true, false);
        return;
    }

    // ****************************
    // ld/st exclusive instructions
    // ****************************
    else if( IS_INSN_LDST_EX(insn) ){
        if( !IS_INSN_LDST_EX_PAIR(insn) ){ // Rt2 field == 31, non-pair op
            insn_in_progress->appendOperand( makeMemRefEx(), true, false);
        } else { // pair
            insn_in_progress->appendOperand( makeMemRefExPair(), true, false);
        }
        return;
    }

    assert(0); //un-handled case

}

void InstructionDecoder_aarch64::STIndex()
{
    if( IS_INSN_LD_LITERAL(insn))
        assert(0); // only load literal, no store literal

    // ******************
    // ld/st register offset
    // ******************
    else if( IS_INSN_LDST_REG(insn)){
        insn_in_progress->appendOperand(makeMemRefReg(), false, true);
        return;
    }

    else if( IS_INSN_LDST_UIMM(insn)){
        insn_in_progress->appendOperand(makeMemRefIndexUImm(), false, true);
        return;
    }

    // ******************
    // ld/st pre and post, unscaled and unprivilidged
    // ******************
    else if( IS_INSN_LDST_PRE(insn)
        || IS_INSN_LDST_UNPRIV(insn)
        || IS_INSN_LDST_UNSCALED(insn) ){
        insn_in_progress->appendOperand(makeMemRefIndexPre(), false, true);
        return;
    }

    else if( IS_INSN_LDST_POST(insn) ){
        insn_in_progress->appendOperand(makeMemRefIndexPost(), false, true);
        return;
    }

    // ****************************
    // ld/st PAIR pre, post, offset
    // ****************************
    else if( IS_INSN_LDST_PAIR_PRE(insn) ){
        insn_in_progress->appendOperand(makeMemRefPairPre(), false, true);
        return;
    }

    else if( IS_INSN_LDST_PAIR_POST(insn)
        || IS_INSN_LDST_PAIR_OFFSET(insn)
        || IS_INSN_LDST_PAIR_NOALLOC(insn) ){
        insn_in_progress->appendOperand(makeMemRefPairPost(), false, true);
        return;
    }

    // ****************************
    // ld/st exclusive instructions
    // ****************************
    else if( IS_INSN_LDST_EX(insn) ){
        if( !IS_INSN_LDST_EX_PAIR(insn) ){ // Rt2 field == 31, non-pair op
            insn_in_progress->appendOperand( makeMemRefEx(), false, true);
        } else { // pair
            insn_in_progress->appendOperand( makeMemRefExPair(), false, true);
        }
        return;
    }

    assert(0); //un-handled case

}

// This function is for non-writeback
void InstructionDecoder_aarch64::OPRRn()
{
	if(IS_INSN_B_UNCOND_REG(insn))										//unconditional branch (register)
	{
		int branchType = field<21, 22>(insn);
		bool branchIsCall = false;

		if(branchType == 0x1)
		{
			branchIsCall = true;
			makeLinkForBranch();
		}

		insn_in_progress->appendOperand(makePCExpr(), false, true);
		insn_in_progress->addSuccessor(makeRnExpr(), branchIsCall, false, false, false);
	}
	else
		insn_in_progress->appendOperand(makeRnExpr(), true, false);
}

void InstructionDecoder_aarch64::OPRRnL()
{
	//insn_in_progress->appendOperand(makeRnExpr(), true, false);
    LIndex();
}

void InstructionDecoder_aarch64::OPRRnS()
{
	//insn_in_progress->appendOperand(makeRnExpr(), true, false);
    STIndex();
}

void InstructionDecoder_aarch64::OPRRnU()
{
    assert(0);
    /* this functions is useless
	insn_in_progress->appendOperand(makeRnExpr(), true, true);
    */
}

void InstructionDecoder_aarch64::OPRRnLU()
{
    if( IS_INSN_LDST_PRE(insn) ){
	    //insn_in_progress->appendOperand(makeRnExpr(), true, true);
        LIndex();
        return;
    }

    if( IS_INSN_LDST_POST(insn) ){
        LIndex();
	    //insn_in_progress->appendOperand(makeRnExpr(), true, true);
        return;
    }

    if( IS_INSN_LDST_PAIR_PRE(insn) ){
	    //insn_in_progress->appendOperand(makeRnExpr(), true, true);
        LIndex();
        return;
    }

    if( IS_INSN_LDST_PAIR_POST(insn) ){
        LIndex();
	    //insn_in_progress->appendOperand(makeRnExpr(), true, true);
        return;
    }

}

void InstructionDecoder_aarch64::OPRRnSU()
{
    if( IS_INSN_LDST_PRE(insn) ){
	    //insn_in_progress->appendOperand(makeRnExpr(), true, true);
        STIndex();
        return;
    }

    if( IS_INSN_LDST_POST(insn) ){
        STIndex();
	    //insn_in_progress->appendOperand(makeRnExpr(), true, true);
        return;
    }

    if( IS_INSN_LDST_PAIR_PRE(insn) ){
	    //insn_in_progress->appendOperand(makeRnExpr(), true, true);
        STIndex();
        return;
    }

    if( IS_INSN_LDST_PAIR_POST(insn) ){
        STIndex();
	    //insn_in_progress->appendOperand(makeRnExpr(), true, true);
        return;
    }
}

Expression::Ptr InstructionDecoder_aarch64::makeRmExpr()
{
	MachRegister baseReg = isFPInsn?
        (isSinglePrec()?aarch64::s0:aarch64::d0):
        (is64Bit?aarch64::x0:aarch64::w0);

	int encoding = field<16, 20>(insn);
	if(!isFPInsn && field<16, 20>(insn) == 31)							//zero register applicable only for non-floating point instructions
		return makeRegisterExpression(makeAarch64RegID(aarch64::zr, 0));
	else
		return makeRegisterExpression(makeAarch64RegID(baseReg, encoding));
}

void InstructionDecoder_aarch64::OPRRm()
{
	if(IS_INSN_LDST_REG(insn) ||
	   IS_INSN_ADDSUB_EXT(insn) ||
	   IS_INSN_ADDSUB_SHIFT(insn) ||
	   IS_INSN_LOGICAL_SHIFT(insn))
		return;

	insn_in_progress->appendOperand(makeRmExpr(), true, false);
}

void InstructionDecoder_aarch64::OPRsf()
{
	if(field<31, 31>(insn) == 0)
		is64Bit = false;
}

template<unsigned int endBit, unsigned int startBit>
void InstructionDecoder_aarch64::OPRoption()
{
	hasOption = true;
	optionField = field<startBit, endBit>(insn);
}

void InstructionDecoder_aarch64::OPRshift()
{
	hasShift = true;
	shiftField = field<22, 23>(insn);
}

void InstructionDecoder_aarch64::OPRhw()
{
	hasHw = true;
	hwField = field<21, 22>(insn);
}

template<unsigned int endBit, unsigned int startBit>
void InstructionDecoder_aarch64::OPRN()
{
	nField = field<startBit, endBit>(insn);
	nLen = endBit - startBit + 1;
}

// the call of this function should be generated by decoder gen for ld/st class
// except for ld/st{s, h, w} subclass
void InstructionDecoder_aarch64::setRegWidth(){
    //is64Bit is by default set to TRUE
    unsigned int opc = 0x3 & field<22, 23>(insn);
    unsigned int opc0 = opc & 0x1;
    unsigned int opc1 = (opc & 0x2)>>1;
    unsigned int sz  = 0x3 & field<30, 31>(insn);

    if(IS_INSN_LDST(insn)){
        if(   IS_INSN_LDST_UIMM(insn)   || IS_INSN_LDST_UNSCALED(insn)
                || IS_INSN_LDST_UNPRIV(insn) || IS_INSN_LDST_POST(insn)
                || IS_INSN_LDST_PRE(insn)    || IS_INSN_LDST_REG(insn)){
                if(opc1 == 0){
                    if(field<30, 31>(insn) != 3)
                        is64Bit = false;
                    return;
                }else{
                    if( sz == 3){
                        if( opc0 == 1) assert(!"unallocated insn");
                    }
                    else{
                        if(sz == 2 && opc0 == 1) assert(!"unallocated insn");
                        if(opc0 == 1)
                            is64Bit = false;
                    }
                    return;
                }
        }
        else if(IS_INSN_LDST_EX(insn)){
            switch(sz){
                case 2:
                    is64Bit = false;
                    break;
                case 0:
                case 1:
                case 3:
                default:
                    return;
            }
        }
        else if(IS_INSN_LDST_PAIR(insn)){
            switch(sz){
                case 0:
                    is64Bit = false;
                    break;
                case 1:
                case 2:
                case 3:
                default:
                    return;
            }
        }
        else if(IS_INSN_LD_LITERAL(insn)){
            switch(sz){
                case 0:
                    is64Bit = false;
                case 1:
                case 2:
                case 3:
                default:
                    return;
            }
        }
        else{
            assert(!"not implemented for SIMD");
        }
    }else{
        assert(0);
    }
    return;
}

Expression::Ptr InstructionDecoder_aarch64::makeRtExpr()
{
	MachRegister baseReg = isFPInsn?
        (isSinglePrec()?aarch64::s0:aarch64::d0):
        (is64Bit ? aarch64::x0 : aarch64::w0);

	return makeRegisterExpression(makeAarch64RegID(baseReg, field<0, 4>(insn)));
}

void InstructionDecoder_aarch64::OPRRt()
{
	//sys, sysl, msr reg, mrs
	//others
}

void InstructionDecoder_aarch64::OPRRtL()
{
	insn_in_progress->appendOperand(makeRtExpr(), false, true);
}

void InstructionDecoder_aarch64::OPRRtS()
{
	insn_in_progress->appendOperand(makeRtExpr(), true, false);
}

Expression::Ptr InstructionDecoder_aarch64::makeRt2Expr()
{
	MachRegister baseReg = isFPInsn?
        (isSinglePrec()?aarch64::s0:aarch64::d0):
        (is64Bit?aarch64::x0 : aarch64::w0);

	return makeRegisterExpression(makeAarch64RegID(baseReg, field<10, 14>(insn)));
}

void InstructionDecoder_aarch64::OPRRt2()
{
    assert(0);
}

void InstructionDecoder_aarch64::OPRRt2L()
{
	insn_in_progress->appendOperand(makeRt2Expr(), false, true);
}

void InstructionDecoder_aarch64::OPRRt2S()
{
	insn_in_progress->appendOperand(makeRt2Expr(), true, false);
}

template<unsigned int endBit, unsigned int startBit>
void InstructionDecoder_aarch64::OPRcond()
{
	unsigned char condVal = static_cast<unsigned char>(field<startBit, endBit>(insn));
	Expression::Ptr cond = Immediate::makeImmediate(Result(u8, condVal));
	insn_in_progress->appendOperand(cond, true, false);

	isPstateRead = true;
}

void InstructionDecoder_aarch64::OPRnzcv()
{
	uint64_t nzcvVal = (static_cast<uint64_t>(field<0, 3>(insn)))<<60;
	Expression::Ptr nzcv = Immediate::makeImmediate(Result(u64, nzcvVal));
	insn_in_progress->appendOperand(nzcv, true, false);

	isPstateWritten = true;
}

void InstructionDecoder_aarch64::OPRop1()
{
	op1Field = field<16, 18>(insn);
}

void InstructionDecoder_aarch64::OPRop2()
{
	op2Field = field<5, 7>(insn);
}

void InstructionDecoder_aarch64::OPRCRm()
{
	isSystemInsn = true;
	crmField = field<8 ,11>(insn);
}

void InstructionDecoder_aarch64::OPRCRn()
{
	crnField = field<12, 15>(insn);
}

template<unsigned int endBit, unsigned int startBit>
void InstructionDecoder_aarch64::OPRS()
{
	sField = field<startBit, endBit>(insn);
}

void InstructionDecoder_aarch64::OPRscale()
{
	int scaleVal = 64 - field<10, 15>(insn);

	Expression::Ptr scale = Immediate::makeImmediate(Result(u32, unsign_extend32(6, scaleVal)));
	insn_in_progress->appendOperand(scale, true, false);
}

Expression::Ptr InstructionDecoder_aarch64::makeRaExpr()
{
	MachRegister baseReg = isFPInsn?
        (isSinglePrec()?aarch64::s0:aarch64::d0):
        (is64Bit?aarch64::x0:aarch64::w0);

	return makeRegisterExpression(makeAarch64RegID(baseReg, field<10, 14>(insn)));
}

void InstructionDecoder_aarch64::OPRRa()
{
	insn_in_progress->appendOperand(makeRaExpr(), true, false);
}

void InstructionDecoder_aarch64::OPRo0()
{
	op0Field = field<19, 20>(insn);
}

void InstructionDecoder_aarch64::OPRb5()
{
	OPRsf();
}

void InstructionDecoder_aarch64::OPRb40()
{
	int b40Val = field<19, 23>(insn);
	int bitpos = ((is64Bit?1:0)<<5) | b40Val;

	insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32, unsign_extend32(6, bitpos))), true, false);
}

template<unsigned int endBit, unsigned int startBit>
void InstructionDecoder_aarch64::OPRsz()
{
    _szField = field<startBit, endBit>(insn);
}

bool InstructionDecoder_aarch64::isSinglePrec() {
    if( isFPInsn && !isSIMDInsn ){
        if(_typeField == -1){
            //TODO if the type field is not set, do sth else
            OPRtype<23, 22>();
        }
        return _typeField==0?true:false;
    }else if( isSIMDInsn ){
        assert(0); //not implemeted yet
    }
    return false;
}

Expression::Ptr InstructionDecoder_aarch64::makeRsExpr()
{
	MachRegister baseReg = isFPInsn?
        (isSinglePrec()?aarch64::s0:aarch64::d0):
        (is64Bit?aarch64::x0:aarch64::w0);

	return makeRegisterExpression(makeAarch64RegID(baseReg, field<16, 20>(insn)));
}

void InstructionDecoder_aarch64::OPRRs()
{
	insn_in_progress->appendOperand(makeRsExpr(), false, true);
}

void InstructionDecoder_aarch64::makeLinkForBranch()
{
	insn_in_progress->appendOperand(makeRegisterExpression(makeAarch64RegID(aarch64::x30, 0)), false, true);
}

void InstructionDecoder_aarch64::makeBranchTarget(bool branchIsCall, bool bIsConditional, int immVal, int immLen)
{
	Expression::Ptr lhs = makePCExpr();

	int offset = sign_extend64(immLen + 2, immVal*4);
	Expression::Ptr rhs = Immediate::makeImmediate(Result(s64, offset));

	if(branchIsCall)
	{
		makeLinkForBranch();
	}

	insn_in_progress->addSuccessor(makeAddExpression(lhs, rhs, s64), branchIsCall, false, bIsConditional, false);
}

Expression::Ptr InstructionDecoder_aarch64::makeFallThroughExpr()
{
	return makeAddExpression(makePCExpr(), Immediate::makeImmediate(Result(u64, unsign_extend64(3, 4))), u64);
}

template<typename T, Result_Type rT>
Expression::Ptr InstructionDecoder_aarch64::fpExpand(int val)
{
	int N, E, F, sign, exp;
	T frac, expandedImm;

	N = (rT == s32)?32:64;
	E = (N == 32)?8:11;
	F = N - E - 1;

	sign = (val & 0x80) >> 7;

	int val6 = ((~val) & 0x40) >> 6, val6mask = (1 << (E - 3)) - 1;
	exp = (val6 << (E - 1)) | ((val6?val6mask:0) << 2) | ((val & 0x30) >> 4);

	frac = (val & 0xF) << (F - 4);

	expandedImm = (sign << (E + F)) | (exp << F) | frac;

	return Immediate::makeImmediate(Result(rT, expandedImm));
}

template<unsigned int endBit, unsigned int startBit>
void InstructionDecoder_aarch64::OPRimm()
{
	int immVal = field<startBit, endBit>(insn);
	unsigned int immLen = endBit - startBit + 1;

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
				Expression::Ptr imm = Immediate::makeImmediate(Result(u32, unsign_extend32(immrLen, immr)));
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
				imm = Immediate::makeImmediate(Result(rT, rT==u32?unsign_extend32(immLen, immVal):unsign_extend64(immLen, immVal)));
			}
			else
				imm = Immediate::makeImmediate(Result(u32, unsign_extend32(immLen, immVal)));

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
		    Expression::Ptr expr = makeOptionExpression(immLen, immVal);

		    insn_in_progress->appendOperand(expr, true, false);
		}
		else
		{
			isValid = false;
		}
	}
	else if(IS_INSN_BRANCHING(insn) && !IS_INSN_B_UNCOND_REG(insn))
	{		//unconditional branch (immediate), test and branch, compare and branch, conditional branch
		bool bIsConditional = false;
		if(!(IS_INSN_B_UNCOND(insn)))
			bIsConditional = true;

		bool branchIsCall = bIsConditional?false:(field<31, 31>(insn) == 1);

		insn_in_progress->appendOperand(makePCExpr(), false, true);
		makeBranchTarget(branchIsCall, bIsConditional, immVal, immLen);

		if(bIsConditional)
			insn_in_progress->addSuccessor(makeFallThroughExpr(), false, false, false, true);
	}
	else if(IS_INSN_PCREL_ADDR(insn))									//pc-relative addressing
	{
		if(IS_FIELD_IMMLO(startBit, endBit))
		{
			immlo = immVal;
			immloLen = endBit - startBit + 1;
		}
		else if(IS_FIELD_IMMHI(startBit, endBit))
		{
			bool page = (field<31, 31>(insn) == 1);
			int offset = (immVal<<immloLen) | immlo;

			Expression::Ptr imm;

			insn_in_progress->appendOperand(makePCExpr(), true, false);
			if(page)
				imm = makeLeftShiftExpression(Immediate::makeImmediate(Result(s64, sign_extend64(immloLen + immLen, offset))),
											  Immediate::makeImmediate(Result(u32, unsign_extend32(4, 12))), s64);
			else
				imm = Immediate::makeImmediate(Result(s64, sign_extend64(immloLen + immLen, offset)));

			insn_in_progress->appendOperand(imm, true, false);
		}
		else
			isValid = false;
	}
    // STEVE added for ld literal
    else if( IS_INSN_LD_LITERAL(insn) ){
			Expression::Ptr imm = Immediate::makeImmediate(Result(u32, sign_extend32(immLen+2, immVal<<2) ));
			insn_in_progress->appendOperand(imm, true, false);
    }
    // TODO: Sunny need a default final else do nothing for ld/st imm fields
    // ld/st will call OPRimm and should do nothing.
    // However, now it will fall into the final else for exception and Cbranch
	else
	{
		if(IS_INSN_FP_IMM(insn))
		{
			if(isSinglePrec())
				insn_in_progress->appendOperand(fpExpand<uint32_t, s32>(immVal), true, false);
			else
				insn_in_progress->appendOperand(fpExpand<uint64_t, s64>(immVal), true, false);
		}
		else                                                            //exception, conditional compare (immediate)
		{
			Result_Type rT = is64Bit?u64:u32;

			Expression::Ptr imm = Immediate::makeImmediate(Result(rT, rT==u32?unsign_extend32(immLen, immVal):unsign_extend64(immLen, immVal)));
			insn_in_progress->appendOperand(imm, true, false);
		}
	}
}


using namespace boost::assign;

#include "aarch64_opcode_tables.C"

	void InstructionDecoder_aarch64::doDelayedDecode(const Instruction *insn_to_complete)
    {
		int insn_table_index = findInsnTableIndex(0);
		aarch64_insn_entry *insn_table_entry = &aarch64_insn_entry::main_insn_table[insn_table_index];

		insn = insn_to_complete->m_RawInsn.small_insn;
		insn_in_progress = const_cast<Instruction*>(insn_to_complete);

		//only a small subset of instructions modify nzcv and the following are the ones for whom it cannot be detected from any operand that they do modify pstate
		//it thus needs to be read from the manual..yay
		if(((IS_INSN_ADDSUB_EXT(insn) || IS_INSN_ADDSUB_SHIFT(insn) || IS_INSN_ADDSUB_IMM(insn) || IS_INSN_ADDSUB_CARRY(insn))
			 && field<29, 29>(insn) == 0x1) ||
		   ((IS_INSN_LOGICAL_SHIFT(insn) || IS_INSN_LOGICAL_IMM(insn))
			 && field<29, 30>(insn) == 0x3) ||
		   (IS_INSN_FP_COMPARE(insn))
            )
		   isPstateWritten = true;


        for(operandSpec::const_iterator fn = insn_table_entry->operands.begin(); fn != insn_table_entry->operands.end(); fn++)
        {
			std::mem_fun(*fn)(this);
		}

		if(isPstateWritten || isPstateRead)
			insn_in_progress->appendOperand(makePstateExpr(), isPstateRead, isPstateWritten);

		if(isSystemInsn)
		{
			processSystemInsn();
		}

		if(!isValid)
		{
			insn_in_progress = invalid_insn;
		}
    }

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
        //insn_printf("ARM: %s\n", insn_table_entry->mnemonic);
        cout << insn_in_progress->format() << endl;

        if(IS_INSN_BRANCHING(insn))
        {
            // decode control-flow operands immediately; we're all but guaranteed to need them
            doDelayedDecode(insn_in_progress);
        }

        //insn_in_progress->arch_decoded_from = m_Arch;
        insn_in_progress->arch_decoded_from = Arch_aarch64;
        return;
    }
  };
};



