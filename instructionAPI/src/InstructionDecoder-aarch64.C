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
    typedef uint32_t Bits_t;

    std::vector<std::string> InstructionDecoder_aarch64::condStringMap;
    std::map<unsigned int, MachRegister> InstructionDecoder_aarch64::sysRegMap;

    struct aarch64_insn_entry
    {
        aarch64_insn_entry(entryID o, const char* m, operandSpec ops):
        op(o), mnemonic(m), operands(ops)
        {
        }

        aarch64_insn_entry(entryID o, const char* m, operandSpec ops, Bits_t enb, Bits_t mb):
        op(o), mnemonic(m), operands(ops),_encodingBits(enb), _maskBits(mb)
        {
        }

        aarch64_insn_entry():
        op(aarch64_op_INVALID), mnemonic("INVALID")
        {
            operands.reserve(5);
        }

        aarch64_insn_entry(const aarch64_insn_entry& o) :
        op(o.op), mnemonic(o.mnemonic), operands(o.operands),
        _encodingBits(o._encodingBits),_maskBits(o._maskBits)
        {
        }

        const aarch64_insn_entry& operator=(const aarch64_insn_entry& rhs)
        {
            operands.reserve(rhs.operands.size());
            op = rhs.op;
            mnemonic = rhs.mnemonic;
            operands = rhs.operands;
            _encodingBits = rhs._encodingBits;
            _maskBits = rhs._maskBits;

            return *this;
        }

        entryID op;
        const char* mnemonic;
        operandSpec operands;

        Bits_t _encodingBits;
        Bits_t _maskBits;

        static void buildInsnTable();
        static bool built_insn_table;

        static aarch64_insn_table main_insn_table;
    };

    struct aarch64_mask_entry
    {
		aarch64_mask_entry(unsigned int m, branchMap bm, int tabIndex):
		mask(m), nodeBranches(bm), insnTableIndices(std::vector<int>()), insnTableIndex(tabIndex)
		{
		}

		aarch64_mask_entry(unsigned int m, branchMap bm, std::vector<int> tabIndices):
		mask(m), nodeBranches(bm), insnTableIndices(tabIndices), insnTableIndex(0)
		{
		}

		aarch64_mask_entry():
		mask(0), nodeBranches(branchMap()), insnTableIndices(std::vector<int>()), insnTableIndex(0)
		{
		}

		aarch64_mask_entry(const aarch64_mask_entry& e):
		mask(e.mask), nodeBranches(e.nodeBranches), insnTableIndices(e.insnTableIndices), insnTableIndex(e.insnTableIndex)
		{
		}

		const aarch64_mask_entry& operator=(const aarch64_mask_entry& rhs)
		{
			mask = rhs.mask;
			nodeBranches = rhs.nodeBranches;
			insnTableIndices = rhs.insnTableIndices;
            insnTableIndex = rhs.insnTableIndex;

			return *this;
		}

		unsigned int mask;
		branchMap nodeBranches;
        std::vector<int> insnTableIndices;
        int insnTableIndex;

		static void buildDecoderTable();
		static bool built_decoder_table;
		static bool isAliasWeakSolution;
		static aarch64_decoder_table main_decoder_table;
	};

    InstructionDecoder_aarch64::InstructionDecoder_aarch64(Architecture a)
      : InstructionDecoderImpl(a), isPstateRead(false), isPstateWritten(false), isFPInsn(false),isSIMDInsn(false),
        is64Bit(true), isValid(true), insn(0), insn_in_progress(NULL),
        hasHw(false), hasShift(false), hasOption(false), hasN(false),
        immr(0), immrLen(0), sField(0), nField(0), nLen(0),
        immlo(0), immloLen(0), _szField(-1), _Q(0),
	cmode(0), op(0), simdAlphabetImm(0)
    {
        aarch64_insn_entry::buildInsnTable();
        aarch64_mask_entry::buildDecoderTable();
        InstructionDecoder_aarch64::buildSysRegMap();

        std::string condArray[16] = {"eq","ne","cs","cc","mi","pl","vs","vc","hi","ls","ge","lt","gt","le","al","nv"};
        InstructionDecoder_aarch64::condStringMap.assign(&condArray[0], &condArray[0] + 16);
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

        op1Field = op2Field = crmField = 0;

        immlo = immloLen = 0;

        _szField = -1;
        _Q = 0;

	cmode = op = simdAlphabetImm = 0;

	oprRotateAmt = 0;
	hasb5 = false;

        insn = b.start[3] << 24 | b.start[2] << 16 |
        b.start[1] << 8 | b.start[0];

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
    void InstructionDecoder_aarch64::NOTHING(){
    }

    void InstructionDecoder_aarch64::setFPMode()
    {
        // NOTE: if it is fp, only isFP is set.
		isFPInsn = true;
    }

     //TODO: consistency issue
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
		MachRegister reg;
		int encoding = field<16, 20>(insn);

		reg = ((optionField & 0x3) == 0x3)?((encoding == 31)?aarch64::zr:aarch64::x0):((encoding == 31)?aarch64::wzr:aarch64::w0);
		if(encoding != 31)
			reg = makeAarch64RegID(reg, encoding);

		Expression::Ptr lhs;

		switch(optionField)
		{
			case 0:lhs = makeRegisterExpression(reg, u8);
					break;
			case 1:lhs = makeRegisterExpression(reg, u16);
					break;
			case 2:lhs = makeRegisterExpression(reg, u32);
					break;
			case 3:lhs = makeRegisterExpression(reg, u64);
					break;
			case 4:lhs = makeRegisterExpression(reg, s8);
					break;
			case 5:lhs = makeRegisterExpression(reg, s16);
					break;
			case 6:lhs = makeRegisterExpression(reg, s32);
					break;
			case 7:lhs = makeRegisterExpression(reg, s64);
					break;
			default: assert(!"invalid option field value");
		}

		Result_Type rT = is64Bit?(optionField<4?u64:s64):(optionField<4?u32:s32);

		return makeLeftShiftExpression(lhs, Immediate::makeImmediate(Result(u32, unsign_extend32(len, val))), rT);
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
			while(extendSize >= 0  && ((extend << (31 - extendSize)) & 0x80000000) == 0)
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
		int op0Field = field<19, 20>(insn), crnField = field<12, 15>(insn);

		if(op0Field == 0)
		{
			if(crnField == 3)			//clrex, dendBit, dmb, iendBit
			{
				Expression::Ptr CRm = Immediate::makeImmediate(Result(u8, unsign_extend32(4, crmField)));

				insn_in_progress->appendOperand(CRm, true, false);
			}
			else if(crnField == 2)                    //hint
			{
				int immVal = (crmField << 3)|(op2Field & 7);

				Expression::Ptr imm = Immediate::makeImmediate(Result(u8, unsign_extend32(7, immVal)));

				insn_in_progress->appendOperand(imm, true, false);
			}
			else if(crnField == 4)				//msr (immediate)
			{
				int pstatefield = (op1Field << 3) | (op2Field & 7);
				insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u8, unsign_extend32(6, pstatefield))), true, false);

				insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u8, unsign_extend32(4, crmField))), true, false);

				isPstateWritten = true;
			}
			else
			{
				isValid = false;
			}
		}
		else if(op0Field == 1)                  //sys, sysl
		{
			insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u8, unsign_extend32(3, op1Field))), true, false);
			insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u8, unsign_extend32(4, crnField))), true, false);
			insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u8, unsign_extend32(4, crmField))), true, false);
			insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u8, unsign_extend32(3, op2Field))), true, false);

			bool isRtRead = (field<21, 21>(insn) == 0);
			insn_in_progress->appendOperand(makeRtExpr(), isRtRead, !isRtRead);
		}
		else                   //mrs (register), msr
		{
			bool isRtRead = (field<21, 21>(insn) == 0);

			unsigned int systemRegEncoding = (op0Field << 14) | (op1Field << 11) | (crnField << 7) | (crmField << 3) | op2Field;
			if(InstructionDecoder_aarch64::sysRegMap.count(systemRegEncoding) <= 0)
				assert(!"tried to access system register not accessible in EL0");

			insn_in_progress->appendOperand(makeRegisterExpression(InstructionDecoder_aarch64::sysRegMap[systemRegEncoding]), !isRtRead, isRtRead);
			insn_in_progress->appendOperand(makeRtExpr(), isRtRead, !isRtRead);
			if(!isRtRead)
			    insn_in_progress->m_Operands.reverse();
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
    int encoding  = field<0, 4>(insn);
	MachRegister reg;

    if(isSIMDInsn)
    {
        if(IS_INSN_SIMD_ACROSS(insn))
        {
            int size = field<22, 23>(insn);

	    //fmaxnmv, fmaxv, fminnmv, fminv
	    if(field<14, 14>(insn) == 0x1)
	    {
		if((size & 0x1) == 0x0)
		    reg = aarch64::s0;
		else
		    isValid = true;
	    }
	    else
	    {
		int opcode = field<12 ,16>(insn);
		
		//saddlv and uaddlv with opcode field 0x03 use different sets of registers
		switch(size)
                {
	            case 0x0:
	                reg = (opcode == 0x03)?aarch64::h0:aarch64::b0;
		        break;
	            case 0x1:
		        reg = (opcode == 0x03)?aarch64::s0:aarch64::h0;
		        break;
                    case 0x2:
	                reg = (opcode == 0x03)?aarch64::d0:aarch64::s0;
	                break;
		    default:
		        assert(!"invalid encoding");
		}
	    }
        }
        else if(IS_INSN_SIMD_COPY(insn))
        {
            unsigned int op = field<29, 29>(insn);
            unsigned int imm4 = field<11, 14>(insn);
        
            if(op == 0x1)
                reg = aarch64::q0;
            else
	    {
                switch(imm4)
                {
                    case 0x5:
                    case 0x7:
                        reg = _Q == 0x1?aarch64::x0:aarch64::w0;
                        break;
                    default:
                        reg = _Q == 0x1?aarch64::q0:aarch64::d0;
                        break;
                }
            }
        }
        else if(IS_INSN_SIMD_VEC_INDEX(insn))
        {
            reg = aarch64::q0;
        }
	else if(IS_INSN_SIMD_MOD_IMM(insn) && _Q == 0 && op == 1 && cmode == 0xE)
	{
	    reg = aarch64::d0;	       
	}
	else if(IS_INSN_SIMD_3DIFF(insn))
	{
	    entryID op = insn_in_progress->getOperation().operationID;

	    if(op == aarch64_op_addhn_advsimd || op == aarch64_op_subhn_advsimd ||
	       op == aarch64_op_raddhn_advsimd || op == aarch64_op_rsubhn_advsimd)
		reg = _Q == 0x1?aarch64::hq0:aarch64::d0;
	    else
		reg = aarch64::q0;
	}
        // 3SAME, 2REG_MISC, EXTRACT
        else 
            reg = _Q == 0x1?aarch64::q0:aarch64::d0;

        reg = makeAarch64RegID(reg, encoding);
    } 
    else if(isFPInsn && !((IS_INSN_FP_CONV_FIX(insn) || (IS_INSN_FP_CONV_INT(insn))) && !IS_SOURCE_GP(insn)))
    {
	if(IS_INSN_FP_DATAPROC_ONESRC(insn))
	{
	    int opc = field<15, 16>(insn);
	    switch(opc)
	    {
		case 0: reg = aarch64::s0;
			break;
		case 1: reg = aarch64::d0;
			break;
		case 3: reg = aarch64::h0;
			break;
		default: assert(!"invalid destination register size");
	    }
	}
	else
	    reg = isSinglePrec()?aarch64::s0:aarch64::d0;
	    
	    reg = makeAarch64RegID(reg, encoding);
	}
	else
	{
	    reg = is64Bit?((encoding == 31)?aarch64::sp:aarch64::x0):((encoding == 31)?aarch64::wsp:aarch64::w0);
	    if(encoding != 31)
	    	reg = makeAarch64RegID(reg, encoding);
	}

	return makeRegisterExpression(reg);
}

void InstructionDecoder_aarch64::OPRRd()
{
    insn_in_progress->appendOperand(makeRdExpr(), false, true);
}

void InstructionDecoder_aarch64::OPRcmode()
{
    cmode = field<12, 15>(insn);
}

void InstructionDecoder_aarch64::OPRop()
{
    op = field<29, 29>(insn);
}

void InstructionDecoder_aarch64::OPRa()
{
    simdAlphabetImm = (simdAlphabetImm & 0x7F) | (field<18, 18>(insn)<<7);
}

void InstructionDecoder_aarch64::OPRb()
{
    simdAlphabetImm = (simdAlphabetImm & 0xBF) | (field<17, 17>(insn)<<6);
}

void InstructionDecoder_aarch64::OPRc()
{
    simdAlphabetImm = (simdAlphabetImm & 0xDF) | (field<16, 16>(insn)<<5);
}

void InstructionDecoder_aarch64::OPRd()
{
    simdAlphabetImm = (simdAlphabetImm & 0xEF) | (field<9, 9>(insn)<<4);
}

void InstructionDecoder_aarch64::OPRe()
{
    simdAlphabetImm = (simdAlphabetImm & 0xF7) | (field<8, 8>(insn)<<3);
}

void InstructionDecoder_aarch64::OPRf()
{
    simdAlphabetImm = (simdAlphabetImm & 0xFB) | (field<7, 7>(insn)<<2);
}

void InstructionDecoder_aarch64::OPRg()
{
    simdAlphabetImm = (simdAlphabetImm & 0xFD) | (field<6, 6>(insn)<<1);
}

void InstructionDecoder_aarch64::OPRh()
{
    simdAlphabetImm = (simdAlphabetImm & 0xFE) | (field<5, 5>(insn));
}

void InstructionDecoder_aarch64::OPRlen()
{
    //reuse immlo
    immlo = field<13, 14>(insn);
}

Expression::Ptr InstructionDecoder_aarch64::makeRnExpr()
{
    int encoding  = field<5, 9>(insn);
	MachRegister reg;

    if(isSIMDInsn && !IS_INSN_LDST(insn))
    {
        if(IS_INSN_SIMD_COPY(insn) )
        {
            unsigned int op = field<29, 29>(insn);
            unsigned int imm4 = field<11, 14>(insn);
            unsigned int imm5 = field<16, 20>(insn);

	    //ins (element)
            if(op == 0x1)
            {
                reg = (imm4 & 0x8)?aarch64::q0:aarch64::d0;
            }
            else
            {
                switch(imm4)
                {
		    //dup (element), smov, umov
		    case 0x0:
		    case 0x5:
		    case 0x7:
			reg = (imm5 & 0x10)?aarch64::q0:aarch64::d0;
			break;
		    //dup (general), ins (general)
                    case 0x1:
                    case 0x3:
                        if(imm5 & 0x1 || imm5 & 0x2 || imm5 & 0x4)
			{
                            reg = encoding==31?aarch64::wzr:aarch64::w0;
                        }
			else
			{
                            reg = encoding==31?aarch64::zr:aarch64::x0;
                        }
                        break;
                    default:
			isValid = true;
                        break;
                }
            }
        }
        else if( IS_INSN_SIMD_VEC_INDEX(insn) )
        {
            reg = _Q == 0x1?aarch64::hq0:aarch64::d0;
        }
	else if(IS_INSN_SIMD_TAB_LOOKUP(insn))
	{
	    reg = _Q==1?aarch64::q0:aarch64::d0;
	    
	    for(int reg_index = immlo; reg_index >= 0; reg_index++)
	    {
		insn_in_progress->appendOperand(makeRegisterExpression(makeAarch64RegID(reg, (encoding+reg_index)%32)), true, false);
	    }
	}
	else if(IS_INSN_SIMD_3DIFF(insn))
	{
	    entryID op = insn_in_progress->getOperation().operationID;

	    if(op == aarch64_op_saddw_advsimd || op == aarch64_op_ssubw_advsimd ||
	       op == aarch64_op_addhn_advsimd || op == aarch64_op_subhn_advsimd ||
	       op == aarch64_op_uaddw_advsimd || op == aarch64_op_usubw_advsimd ||
	       op == aarch64_op_raddhn_advsimd || op == aarch64_op_rsubhn_advsimd)
		reg = aarch64::q0;
	    else
		reg = _Q == 0x1?aarch64::hq0:aarch64::d0;
	}
        else
            reg = _Q == 0x1?aarch64::q0:aarch64::d0;

        reg = makeAarch64RegID(reg, encoding);
    } 
    else if(isFPInsn && !((IS_INSN_FP_CONV_FIX(insn) || (IS_INSN_FP_CONV_INT(insn))) && IS_SOURCE_GP(insn)))
    {
	switch(_typeField)
	{
	    case 0: reg = aarch64::s0;
	       	break;
	    case 1: reg = aarch64::d0;
		break;
	    case 3: reg = aarch64::h0;
			break;
	    default: assert(!"invalid source register size");
	}
	reg = makeAarch64RegID(reg, encoding);
    }
    else if(IS_INSN_LDST(insn))
    {
        reg = encoding == 31?aarch64::sp:aarch64::x0;
		if(encoding != 31)
			reg = makeAarch64RegID(reg, encoding);
    }
    else
    {
    	reg = is64Bit?((encoding == 31)?aarch64::sp:aarch64::x0):((encoding == 31)?aarch64::wsp:aarch64::w0);
    	if(encoding != 31)
    		reg = makeAarch64RegID(reg, encoding);
    }

    return makeRegisterExpression(reg);
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

    Expression::Ptr label = Immediate::makeImmediate(Result(s32, sign_extend32(immLen, immVal)));

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

    Result_Type rt;
    getMemRefIndex_RT(rt);
    return makeDereferenceExpression( makeAddExpression(makeRnExpr(), offset, u64), rt);
}

Expression::Ptr InstructionDecoder_aarch64::makeMemRefIndex_offset9(){
    unsigned int immVal = 0, immLen = 0;
    getMemRefIndexPrePost_ImmImmlen(immVal, immLen);
    return Immediate::makeImmediate(Result(u32, sign_extend32(immLen, immVal)));
}

// scale = 2 + opc<1>
// scale = 1<<scale
// LSL(sign_ex(imm7 , 64), scale)
Expression::Ptr InstructionDecoder_aarch64::makeMemRefPair_offset7(){
    /*
    unsigned int scaleVal = field<31, 31>(insn);
    unsigned int scaleLen = 8;
    scaleVal += 2;
    Expression::Ptr scale = Immediate::makeImmediate(Result(u32, unsign_extend32(scaleLen, 1<<scaleVal)));
    */

    unsigned int immVal = 0, immLen = 0;
    getMemRefPair_ImmImmlen(immVal, immLen);

    immVal = is64Bit?immVal<<3:immVal<<2;
    Expression::Ptr imm7 = Immediate::makeImmediate(Result(u64, sign_extend64(immLen, immVal)));

    //return makeMultiplyExpression(imm7, scale, s64);
    return imm7;
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

Expression::Ptr InstructionDecoder_aarch64::makeMemRefPairPost(){
    Result_Type rt;
    getMemRefPair_RT(rt);
    return makeDereferenceExpression(makeRnExpr(), rt);
}

void InstructionDecoder_aarch64::getMemRefEx_RT(Result_Type &rt){
    unsigned int sz = field<30, 31>(insn);
    switch(sz){
        case 0x00: //B
            rt = u8;
            break;
        case 0x01: //H
            rt = u16;
            break;
        case 0x02: //32b
            rt = u32;
            break;
        case 0x03: //64b
            rt = u64;
            break;
        default:
            rt = u64;
    }
    return;
}

Expression::Ptr InstructionDecoder_aarch64::makeMemRefEx(){
    Result_Type rt;
    getMemRefEx_RT(rt);
    return makeDereferenceExpression(makeRnExpr(), rt);
}

void InstructionDecoder_aarch64::OPRQ()
{
    _Q = field<30, 30>(insn);
}

void InstructionDecoder_aarch64::OPRL()
{
    _L = field<30, 30>(insn);
}

unsigned int InstructionDecoder_aarch64::getMemRefSIMD_MULT_T(){
    unsigned int Q = field<30, 30>(insn);
    return Q?128:64;
}

void InstructionDecoder_aarch64::getMemRefSIMD_MULT_RT(Result_Type &rt){
    unsigned int tmpSize = getMemRefSIMD_MULT_T();
    unsigned int rpt = 0, selem = 0;
    getSIMD_MULT_RptSelem(rpt, selem);
    tmpSize = tmpSize * rpt * selem;
    switch(tmpSize){
        case 64:
            rt = u64;
            return;
        case 128:
            rt = dbl128;
            return;
        case 192:
            rt = m192;
            return;
        case 256:
            rt = m256;
            return;
        case 384:
            rt = m384;
            return;
        case 512:
            rt = m512;
            return;
        default:
            assert(!"not implemented");
            return;
    }
}

unsigned int InstructionDecoder_aarch64::getSIMD_SING_selem(){
    return ( ((field<13, 13>(insn)<<1)&0x2) | (field<21,21>(insn)&0x1) ) + 0x1;
}

void InstructionDecoder_aarch64::getMemRefSIMD_SING_RT(Result_Type &rt){
    unsigned int tmpSize = getMemRefSIMD_SING_T();
    unsigned int selem = getSIMD_SING_selem();
    switch(selem*tmpSize){
        case 8:
            rt = u8;
            break;
        case 16:
            rt = u16;
            break;
        case 24:
            rt = u24;
            break;
        case 32:
            rt = u32;
            break;
        case 48:
            rt = u48;
            break;
        case 64:
            rt = u64;
            break;
        case 96:
            rt = m96;
            break;
        case 128:
            rt = dbl128;
            break;
        case 192:
            rt = m192;
            break;
        case 256:
            rt = m256;
            break;
        default:
            assert(!"not implemented");
            break;
    }
}

unsigned int InstructionDecoder_aarch64::getMemRefSIMD_SING_T(){
    unsigned int opcode = field<13,15>(insn);
    unsigned int S = field<12, 12>(insn);
    unsigned int size = field<10, 11>(insn);

    if(opcode == 0x0){
        return 8;
    }else
    if(opcode == 0x2 && (size & 0x1)==0x0){
        return 16;
    }else
    if(opcode == 0x4 && size == 0x0){
        return 32;
    }else
    if(opcode == 0x4 && S == 0 && size == 0x1){
        return 64;
    }
    else
        assert(!"not implemented");
    return 0;
}

Expression::Ptr InstructionDecoder_aarch64::makeMemRefSIMD_MULT(){
    Result_Type rt;
    getMemRefSIMD_MULT_RT(rt);
    return makeDereferenceExpression(makeRnExpr(), rt);
}

Expression::Ptr InstructionDecoder_aarch64::makeMemRefSIMD_SING(){
    Result_Type rt;
    getMemRefSIMD_SING_RT(rt);
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
    // never be called
    if( IS_INSN_LD_LITERAL(insn)){
        insn_in_progress->appendOperand(makeMemRefIndexLiteral(), true, false);
        return;
    }

    // ******************
    // load register offset
    // ******************
    else if( IS_INSN_LDST_REG(insn)){
        insn_in_progress->appendOperand(makeMemRefReg(), true, false);
        return;
    }

    // ******************
    // load unsigned imm
    // ******************
    else if( IS_INSN_LDST_UIMM(insn)){
        insn_in_progress->appendOperand(makeMemRefIndexUImm(), true, false);
        return;
    }

    // ******************
    // load pre, unscaled and unprivlidged
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
    // load PAIR pre, post, offset
    // ****************************
    else if( IS_INSN_LDST_PAIR_PRE(insn)
        || IS_INSN_LDST_PAIR_NOALLOC(insn) ){
        insn_in_progress->appendOperand( makeMemRefPairPre(), true, false);
        return;
    }

    else if( IS_INSN_LDST_PAIR_POST(insn)
        || IS_INSN_LDST_PAIR_OFFSET(insn) ){
        insn_in_progress->appendOperand( makeMemRefPairPost(), true, false);
        return;
    }

    // ****************************
    // load exclusive instructions
    // ****************************
    else if( IS_INSN_LDST_EX(insn) ){
        if( !IS_INSN_LDST_EX_PAIR(insn) ){ // Rt2 field == 31, non-pair op
            insn_in_progress->appendOperand( makeMemRefEx(), true, false);
        } else { // pair
            insn_in_progress->appendOperand( makeMemRefExPair(), true, false);
        }
        return;
    }

    // ****************************
    // load SIMD multiple structures &
    // load SIMD multiple structures post increment
    // ****************************
    else if( IS_INSN_LDST_SIMD_MULT(insn)
            || IS_INSN_LDST_SIMD_MULT_POST(insn) ){
        insn_in_progress->appendOperand( makeMemRefSIMD_MULT(), true, false);
        return;
    }
    // ****************************
    // load SIMD single structure &
    // load SIMD single structure post increment
    // ****************************
    else if( IS_INSN_LDST_SIMD_SING(insn)
            || IS_INSN_LDST_SIMD_SING_POST(insn) ){
        insn_in_progress->appendOperand( makeMemRefSIMD_SING(), true, false);
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
        insn_in_progress->appendOperand( makeMemRefReg(), false, true);
        return;
    }

    else if( IS_INSN_LDST_UIMM(insn)){
        insn_in_progress->appendOperand( makeMemRefIndexUImm(), false, true);
        return;
    }

    // ******************
    // ld/st pre and post, unscaled and unprivilidged
    // ******************
    else if( IS_INSN_LDST_PRE(insn)
        || IS_INSN_LDST_UNPRIV(insn)
        || IS_INSN_LDST_UNSCALED(insn) ){
        insn_in_progress->appendOperand( makeMemRefIndexPre(), false, true);
        return;
    }

    else if( IS_INSN_LDST_POST(insn) ){
        insn_in_progress->appendOperand( makeMemRefIndexPost(), false, true);
        return;
    }

    // ****************************
    // ld/st PAIR pre, post, offset
    // ****************************
    else if( IS_INSN_LDST_PAIR_PRE(insn)
        || IS_INSN_LDST_PAIR_NOALLOC(insn) ){
        insn_in_progress->appendOperand( makeMemRefPairPre(), false, true);
        return;
    }
    else if( IS_INSN_LDST_PAIR_POST(insn)
        || IS_INSN_LDST_PAIR_OFFSET(insn)) {
        insn_in_progress->appendOperand( makeMemRefPairPost(), false, true);
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

    // ****************************
    // store SIMD multiple structures &
    // store SIMD multiple structures post increment
    // ****************************
    else if( IS_INSN_LDST_SIMD_MULT(insn)
            ||IS_INSN_LDST_SIMD_MULT_POST(insn) ){
        insn_in_progress->appendOperand( makeMemRefSIMD_MULT(), false, true);
        return;
    }
    else if( IS_INSN_LDST_SIMD_SING(insn)
            ||IS_INSN_LDST_SIMD_SING_POST(insn) ){
        insn_in_progress->appendOperand( makeMemRefSIMD_SING(), false, true);
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

		insn_in_progress->appendOperand(makePCExpr(), false, true);
		insn_in_progress->addSuccessor(makeRnExpr(), field<21, 21>(insn) == 1, true, false, false);

		if(branchType == 0x1)
		{
			insn_in_progress->addSuccessor(makeFallThroughExpr(), false, false, false, true);
		}
	}
	else
		insn_in_progress->appendOperand(makeRnExpr(), true, false);
}

void InstructionDecoder_aarch64::OPRRnL()
{
    LIndex();
}

void InstructionDecoder_aarch64::OPRRnS()
{
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
        LIndex();
        return;
    }

    if( IS_INSN_LDST_POST(insn) ){
        LIndex();
        return;
    }

    if( IS_INSN_LDST_PAIR_PRE(insn) ){
        LIndex();
        return;
    }

    if( IS_INSN_LDST_PAIR_POST(insn) ){
        LIndex();
        return;
    }

}

void InstructionDecoder_aarch64::OPRRnSU()
{
    if( IS_INSN_LDST_PRE(insn) ){
        STIndex();
        return;
    }

    if( IS_INSN_LDST_POST(insn) ){
        STIndex();
        return;
    }

    if( IS_INSN_LDST_PAIR_PRE(insn) ){
        STIndex();
        return;
    }

    if( IS_INSN_LDST_PAIR_POST(insn) ){
        STIndex();
        return;
    }
}

unsigned int InstructionDecoder_aarch64::get_SIMD_MULT_POST_imm(){
    unsigned int Q = field<30, 30>(insn);
    unsigned int rpt = 0, selem = 0;
    getSIMD_MULT_RptSelem(rpt, selem);
    unsigned int numReg = rpt*selem;
    return Q?numReg<<3:numReg<<4;
}

unsigned int InstructionDecoder_aarch64::get_SIMD_SING_POST_imm()
{
    return getMemRefSIMD_SING_T()>>3;
}

Expression::Ptr InstructionDecoder_aarch64::makeRmExpr()
{
    int encoding  = field<16, 20>(insn);
    MachRegister reg;

    if(isSIMDInsn)
    {
        if(IS_INSN_LDST_SIMD_MULT_POST(insn) && encoding == 0x1F)
        {
            unsigned int immVal = get_SIMD_MULT_POST_imm();
            unsigned int immLen = 8; // max #64
            return Immediate::makeImmediate( Result(u32, unsign_extend32(immLen, immVal)) );
        }
        else if(IS_INSN_LDST_SIMD_SING_POST(insn) && encoding == 0x1F)
        {
            unsigned int immVal = get_SIMD_SING_POST_imm();
            unsigned int immLen = 4; // max #8
            return Immediate::makeImmediate( Result(u32, unsign_extend32(immLen, immVal) ) );
        }
        else if(IS_INSN_SIMD_VEC_INDEX(insn))
        {
            reg = field<11, 11>(insn)==0x1?aarch64::q0:aarch64::d0;
        }
	else if(IS_INSN_SIMD_3DIFF(insn))
	{
	    entryID op = insn_in_progress->getOperation().operationID;

	    if(op == aarch64_op_addhn_advsimd || op == aarch64_op_subhn_advsimd ||
	       op == aarch64_op_raddhn_advsimd || op == aarch64_op_rsubhn_advsimd)
		reg = aarch64::q0;
	    else
		reg = _Q == 0x1?aarch64::hq0:aarch64::d0;
	}
        else
	    reg = _Q == 0x1?aarch64::q0:aarch64::d0;
        
        reg = makeAarch64RegID(reg, encoding);
    
    	return makeRegisterExpression(reg);
    }
    else if(isFPInsn)
    {
	reg = isSinglePrec()?aarch64::s0:aarch64::d0;
	reg = makeAarch64RegID(reg, encoding);
    }
    else
    {
	reg = is64Bit?((encoding == 31)?aarch64::zr:aarch64::x0):((encoding == 31)?aarch64::wzr:aarch64::w0);
	if(encoding != 31)
	    reg = makeAarch64RegID(reg, encoding);
    }

    return makeRegisterExpression(reg);
}

void InstructionDecoder_aarch64::OPRRm()
{
	if(IS_INSN_LDST_REG(insn) ||
	   IS_INSN_ADDSUB_EXT(insn) ||
	   IS_INSN_ADDSUB_SHIFT(insn) ||
	   IS_INSN_LOGICAL_SHIFT(insn))
		return;

	if(IS_INSN_FP_COMPARE(insn) && field<3, 3>(insn) == 1)
		insn_in_progress->appendOperand(Immediate::makeImmediate(Result(isSinglePrec()?sp_float:dp_float, 0.0)), true, false);
	else
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
        hasN = true;
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
                            
                    }
                    return;
                }
        }
        else if(IS_INSN_LDST_EX(insn)){
            switch(sz){
                case 2:
                case 0:
                case 1:
                    is64Bit = false;
                    break;
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
	int encoding  = field<0, 4>(insn);
	MachRegister reg;

	if(isFPInsn)
	{
		reg = makeAarch64RegID(isSinglePrec()?aarch64::s0:aarch64::d0, encoding);
	}
	else
	{
		reg = is64Bit?((encoding == 31)?aarch64::zr:aarch64::x0):((encoding == 31)?aarch64::wzr:aarch64::w0);
		if(encoding != 31)
			reg = makeAarch64RegID(reg, encoding);
	}

	return makeRegisterExpression(reg);
}

void InstructionDecoder_aarch64::getSIMD_MULT_RptSelem(unsigned int &rpt, unsigned int &selem){
    unsigned opcode = field<12, 15>(insn);
    switch(opcode){
        case 0x0:
            rpt = 1; selem = 4;
            break;
        case 0x2:
            rpt = 4; selem = 1;
            break;
        case 0x4:
            rpt = 1; selem = 3;
            break;
        case 0x6:
            rpt = 3; selem = 1;
            break;
        case 0x7:
            rpt = 1; selem = 1;
            break;
        case 0x8:
            rpt = 1; selem = 2;
            break;
        case 0xa:
            rpt = 2; selem = 1;
            break;
        default:
            assert(!"unallocated encoding");
            return;
    }
    assert(rpt!=0 && selem!=0);
    return;
}

void InstructionDecoder_aarch64::OPRRt()
{
	int encoding = field<0, 4>(insn);
	if(IS_INSN_BRANCHING(insn))
	{
		if(encoding == 31)
			insn_in_progress->appendOperand(makeRegisterExpression(is64Bit?aarch64::zr:aarch64::wzr), true, false);
		else
			insn_in_progress->appendOperand(makeRegisterExpression(makeAarch64RegID(is64Bit?aarch64::x0:aarch64::w0, encoding)), true, false);
	}
}

void InstructionDecoder_aarch64::OPRRtL()
{
	int encoding = field<0, 4>(insn);
    if( IS_INSN_LDST_SIMD_MULT(insn) ){
        unsigned int rpt, selem;
        getSIMD_MULT_RptSelem(rpt, selem);
        for(unsigned int it_rpt = 0; it_rpt < rpt*selem; it_rpt++){
			insn_in_progress->appendOperand(makeRegisterExpression(makeAarch64RegID(aarch64::q0, (encoding + it_rpt)%0x1f )), false, true);
        }
    }
    else
    if( IS_INSN_LDST_SIMD_SING(insn) ){
        unsigned int selem =  getSIMD_SING_selem();
        for(unsigned int it_selem = 0; it_selem < selem; it_selem++){
			insn_in_progress->appendOperand(makeRegisterExpression(makeAarch64RegID(aarch64::q0, (encoding + it_selem)%0x1f )), false, true);
        }
    }
    else
	    insn_in_progress->appendOperand(makeRtExpr(), false, true);
}

void InstructionDecoder_aarch64::OPRRtS()
{
	int encoding = field<0, 4>(insn);
    if( IS_INSN_LDST_SIMD_MULT(insn) ){
        unsigned int rpt, selem;
        getSIMD_MULT_RptSelem(rpt, selem);
        for(unsigned int it_rpt = 0; it_rpt < rpt; it_rpt++){
			insn_in_progress->appendOperand(makeRegisterExpression(makeAarch64RegID(aarch64::q0, (encoding + it_rpt)%0x1f )), true, false);
        }
    }
    else
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
	if(IS_INSN_B_COND(insn))
	{
		insn_in_progress->getOperation().mnemonic += ".";
		insn_in_progress->getOperation().mnemonic += condStringMap[condVal];
	}
	else
	{
		Expression::Ptr cond = Immediate::makeImmediate(Result(u8, condVal));
		insn_in_progress->appendOperand(cond, true, false);
		oprRotateAmt++;
	}

	isPstateRead = true;
}

void InstructionDecoder_aarch64::OPRnzcv()
{
	unsigned int nzcvVal = field<0, 3>(insn)<<28;
	Expression::Ptr nzcv = Immediate::makeImmediate(Result(u32, nzcvVal));
	insn_in_progress->appendOperand(nzcv, true, false);

	isPstateWritten = true;
	oprRotateAmt++;
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
	crmField = field<8 ,11>(insn);
}

void InstructionDecoder_aarch64::OPRCRn()
{
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
	int encoding  = field<10, 14>(insn);
	MachRegister reg;

	if(isFPInsn)
	{
		reg = makeAarch64RegID(isSinglePrec()?aarch64::s0:aarch64::d0, encoding);
	}
	else
	{
		reg = is64Bit?((encoding == 31)?aarch64::sp:aarch64::x0):((encoding == 31)?aarch64::wsp:aarch64::w0);
		if(encoding != 31)
			reg = makeAarch64RegID(reg, encoding);
	}

	return makeRegisterExpression(reg);

}

void InstructionDecoder_aarch64::OPRRa()
{
	insn_in_progress->appendOperand(makeRaExpr(), true, false);

	oprRotateAmt++;
}

void InstructionDecoder_aarch64::OPRo0()
{

}

void InstructionDecoder_aarch64::OPRb5()
{
	OPRsf();
	hasb5 = true;
}

void InstructionDecoder_aarch64::OPRb40()
{

}

Expression::Ptr InstructionDecoder_aarch64::makeb40Expr()
{
	int b40Val = field<19, 23>(insn);
	int bitpos = ((is64Bit?1:0)<<5) | b40Val;

	return Immediate::makeImmediate(Result(u32, unsign_extend32(6, bitpos)));
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

    if(IS_INSN_LDST(insn)){
        baseReg = aarch64::w0;
    }
	return makeRegisterExpression(makeAarch64RegID(baseReg, field<16, 20>(insn)));
}

void InstructionDecoder_aarch64::OPRRs()
{
	insn_in_progress->appendOperand(makeRsExpr(), false, true);
}

void InstructionDecoder_aarch64::makeBranchTarget(bool branchIsCall, bool bIsConditional, int immVal, int immLen)
{
	Expression::Ptr lhs = makePCExpr();

	int offset = sign_extend64(immLen + 2, immVal*4);
	Expression::Ptr rhs = Immediate::makeImmediate(Result(s64, offset));

	insn_in_progress->addSuccessor(makeAddExpression(lhs, rhs, s64), branchIsCall, false, bIsConditional, false);
	if(branchIsCall)
	{
		insn_in_progress->addSuccessor(makeFallThroughExpr(), false, false, false, true);
	}

}

Expression::Ptr InstructionDecoder_aarch64::makeFallThroughExpr()
{
	return makeAddExpression(makePCExpr(), Immediate::makeImmediate(Result(u64, unsign_extend64(3, 4))), u64);
}

template<typename T, Result_Type rT>
Expression::Ptr InstructionDecoder_aarch64::fpExpand(int val)
{
	int N, E, F;
	T frac, expandedImm, sign, exp;

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

	if(IS_INSN_LDST(insn)){
        if(IS_INSN_LD_LITERAL(insn) ){
			Expression::Ptr literal = makeMemRefIndexLiteral();
			insn_in_progress->appendOperand(literal, true, false);
        }
        else if(IS_INSN_LDST_POST(insn)){
			Expression::Ptr offset = makeMemRefIndex_offset9();
			insn_in_progress->appendOperand(offset, true, false);
        }
        else if(IS_INSN_LDST_PAIR_POST(insn)
                || IS_INSN_LDST_PAIR_OFFSET(insn) ){
			Expression::Ptr offset = makeMemRefPair_offset7();
			insn_in_progress->appendOperand(offset, true, false);
        }
		return;
    }

	if(hasHw)
	{
		processHwFieldInsn(immLen, immVal);
	}
	else if(hasN)		//logical (immediate), bitfield, extract
	{
		if(IS_FIELD_IMMR(startBit, endBit))
		{
			immr = immVal;
			immrLen = immLen;
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
			{
				imm = Immediate::makeImmediate(Result(u32, unsign_extend32(immLen, immVal)));
				oprRotateAmt++;
			}

			insn_in_progress->appendOperand(imm, true, false);
			if(IS_INSN_BITFIELD(insn))
			{
				imm = Immediate::makeImmediate(Result(u32, unsign_extend32(immrLen, immr)));
				insn_in_progress->appendOperand(imm, true, false);
				oprRotateAmt--;
			}
		}
		else
		    isValid = false;
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
			isValid = false;
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

		if(hasb5)
			insn_in_progress->appendOperand(makeb40Expr(), true, false);

		if(bIsConditional)
			insn_in_progress->addSuccessor(makeFallThroughExpr(), false, false, true, true);
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
			int page = field<31, 31>(insn);
			int offset = (immVal<<immloLen) | immlo;

			insn_in_progress->appendOperand(makePCExpr(), true, false);
			Expression::Ptr imm = Immediate::makeImmediate(Result(s64, sign_extend64(immloLen + immLen + page*12, offset<<(page*12))));

			insn_in_progress->appendOperand(imm, true, false);
		}
		else
			isValid = false;
	}
	else if(isFPInsn)
	{
		if(isSinglePrec())
			insn_in_progress->appendOperand(fpExpand<int32_t, s32>(immVal), true, false);
		else
			insn_in_progress->appendOperand(fpExpand<int64_t, s64>(immVal), true, false);
	}
	else if(IS_INSN_EXCEPTION(insn))
	{
		Expression::Ptr imm = Immediate::makeImmediate(Result(u16, immVal));
		insn_in_progress->appendOperand(imm, true, false);
		isPstateRead = true;
	}
	else if(isSIMDInsn)
	{
	    if(IS_INSN_SIMD_EXTR(insn))
	    {
		if(_Q == 0)
		{
		    if((immVal & 0x8) == 0)
		    {
			Expression::Ptr imm = Immediate::makeImmediate(Result(u32, unsign_extend32(immLen - 1, immVal & 0x7)));
			insn_in_progress->appendOperand(imm, true, false);
		    }
		    else
			isValid = true;
		}
		else
		{
		    Expression::Ptr imm = Immediate::makeImmediate(Result(u32, unsign_extend32(immLen, immVal)));\
		    insn_in_progress->appendOperand(imm, true, false);
		}
	    }
	    else if(IS_INSN_SIMD_SHIFT_IMM(insn))
	    {
		//immh
		if(startBit == 19 && endBit == 22)
		{
		    immlo = immVal;
		    immloLen = endBit - startBit + 1;
		}
		//immb
		else if(startBit == 16 && endBit == 18)
		{
		    int shift, isRightShift = 1, elemWidth = (immlo << immLen) | immVal;
		    
		    entryID insnID = insn_in_progress->getOperation().operationID;
		    
		    //check if shift is left; if it is, the immediate has to be processed in a different manner.
		    //unfortunately, determining whether the instruction will do a left or right shift cannot be determined in any way other than checking the instruction's opcode
		    if(insnID == aarch64_op_shl_advsimd || insnID == aarch64_op_sqshl_advsimd_imm || insnID == aarch64_op_sshll_advsimd ||
		       insnID == aarch64_op_sli_advsimd || insnID == aarch64_op_sqshlu_advsimd || insnID == aarch64_op_uqshl_advsimd_imm || insnID == aarch64_op_ushll_advsimd)
			isRightShift = -1;	

		    if(immlo & 0x1)
			shift = isRightShift*(16 - elemWidth) + isRightShift>0?0:8;
		    else if(immlo & 0x2)
			shift = isRightShift*(32 - elemWidth) + isRightShift>0?0:16;
		    else if(immlo & 0x4)
			shift = isRightShift*(64 - elemWidth) + isRightShift>0?0:32;
		    else
			shift = isRightShift*(128 - elemWidth) + isRightShift>0?0:64;

		    Expression::Ptr imm = Immediate::makeImmediate(Result(u32, unsign_extend32(immloLen + immLen, shift)));
		    insn_in_progress->appendOperand(imm, true, false);
		}
		else
		    isValid = false;
	    }
	}
	else                                                            //conditional compare (immediate)
	{
		Result_Type rT = is64Bit?u64:u32;

		Expression::Ptr imm = Immediate::makeImmediate(Result(rT, rT==u32?unsign_extend32(immLen, immVal):unsign_extend64(immLen, immVal)));
		insn_in_progress->appendOperand(imm, true, false);
	}
}

	void InstructionDecoder_aarch64::reorderOperands()
	{
	    if(oprRotateAmt)
	    {
			std::vector<Operand> curOperands;
			insn_in_progress->getOperands(curOperands);

			if(curOperands.empty())
				assert(!"empty operand list found while re-ordering operands");

			std::swap(curOperands[1], curOperands[3]);

			while(oprRotateAmt--)
				std::rotate(curOperands.begin(), curOperands.begin()+1, curOperands.begin()+3);

			insn_in_progress->m_Operands.assign(curOperands.begin(), curOperands.end());
	    }
        else if( IS_INSN_LDST_POST(insn) || IS_INSN_LDST_PAIR_POST(insn) ){
	        std::vector<Operand> curOperands;
	        insn_in_progress->getOperands(curOperands);
            std::iter_swap( curOperands.begin(), curOperands.end()-1 );
		    insn_in_progress->m_Operands.assign(curOperands.begin(), curOperands.end());
        }
        else if( IS_INSN_LDST_PAIR(insn) ){
		    std::vector<Operand> curOperands;
		    insn_in_progress->getOperands(curOperands);
            assert(curOperands.size() == 4 || curOperands.size() == 3);
            if(curOperands.size() == 3){
                curOperands.insert(curOperands.begin(), curOperands.back());
                curOperands.pop_back();
            }else if(curOperands.size() == 4){
                std::iter_swap( curOperands.begin(), curOperands.end()-1 );
            }
		    insn_in_progress->m_Operands.assign(curOperands.begin(), curOperands.end());
        }
        else if( IS_INSN_LDST_EX_PAIR(insn) ){
		    std::vector<Operand> curOperands;
		    insn_in_progress->getOperands(curOperands);
            if(curOperands.size() == 3) {
                curOperands.insert(curOperands.begin(), curOperands.back());
                curOperands.pop_back();
            }
            else if( curOperands.size() == 4) {
                curOperands.insert(curOperands.begin()+1, curOperands.back());
                curOperands.pop_back();
            }
		    insn_in_progress->m_Operands.assign(curOperands.begin(), curOperands.end());
        }
        else if( IS_INSN_ST_EX(insn) ){
		    std::vector<Operand> curOperands;
		    insn_in_progress->getOperands(curOperands);
            if(curOperands.size() == 3){
                curOperands.insert(curOperands.begin()+1, curOperands.back());
                curOperands.pop_back();
		        insn_in_progress->m_Operands.assign(curOperands.begin(), curOperands.end());
            }
            else
		        insn_in_progress->m_Operands.reverse();
        }
	    else
		    insn_in_progress->m_Operands.reverse();
	}

void InstructionDecoder_aarch64::processAlphabetImm()
{
   if(op == 1 && cmode == 0xE)
   {
	uint64_t imm = 0;

	imm      = (simdAlphabetImm & 0x1)?((1<<8)  -  (simdAlphabetImm & 0x1)):0;
	uint64_t immg = (simdAlphabetImm & 0x2)?((1<<8)  - ((simdAlphabetImm & 0x2)>>1)):0;
	uint64_t immf = (simdAlphabetImm & 0x4)?((1<<8)  - ((simdAlphabetImm & 0x4)>>2)):0;
	uint64_t imme = (simdAlphabetImm & 0x8)?((1<<8)  - ((simdAlphabetImm & 0x8)>>3)):0;
	uint64_t immd = (simdAlphabetImm & 0x10)?((1<<8) - ((simdAlphabetImm & 0x10)>>4)):0;
	uint64_t immc = (simdAlphabetImm & 0x20)?((1<<8) - ((simdAlphabetImm & 0x20)>>5)):0;
	uint64_t immb = (simdAlphabetImm & 0x40)?((1<<8) - ((simdAlphabetImm & 0x40)>>6)):0;
	uint64_t imma = (simdAlphabetImm & 0x80)?((1<<8) - ((simdAlphabetImm & 0x80)>>7)):0;

	imm = (imma << 55) | (immb << 47) | (immc << 39) | (immd << 31) | (imme << 23) | (immf << 15) | (immg << 7) | imm;

	insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64, imm)), true, false);
   }
   else if(cmode == 0xF)
   {
       //fmov (vector, immediate)
       //TODO: check with Bill if this is fine
      insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u8, simdAlphabetImm)), true, false); 
   }
   else
   {
	int shiftAmt = 0;

	//16-bit shifted immediate
	if((cmode & 0xC) == 0x2)
	    shiftAmt = (cmode & 0x2) * 8;
	//32-bit shifted immediate
	else if((cmode & 0x8) == 0x0)
	    shiftAmt = (cmode & 0x6) * 8;
	//32-bit shifting ones
	else if((cmode & 0xE) == 0x6)
	    shiftAmt = ((cmode & 0x0) + 1) * 8;
	
	Expression::Ptr lhs = Immediate::makeImmediate(Result(u32, unsign_extend32(8, simdAlphabetImm)));
	Expression::Ptr rhs = Immediate::makeImmediate(Result(u32, unsign_extend32(5, shiftAmt)));
	Expression::Ptr imm = makeLeftShiftExpression(lhs, rhs, u64);

	insn_in_progress->appendOperand(imm, true, false);
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

        for(operandSpec::const_iterator fn = insn_table_entry->operands.begin(); fn != insn_table_entry->operands.end(); fn++)
        {
			std::mem_fun(*fn)(this);
		}

		reorderOperands();

		if(IS_INSN_SYSTEM(insn))
		{
		    processSystemInsn();
		}

		if(IS_INSN_SIMD_MOD_IMM(insn))
		{
		    processAlphabetImm();
		}

		if(isPstateWritten || isPstateRead)
			insn_in_progress->appendOperand(makePstateExpr(), isPstateRead, isPstateWritten);

		if(!isValid)
		{
			insn_in_progress->getOperation().mnemonic = INVALID_ENTRY.mnemonic;
			insn_in_progress->getOperation().operationID = INVALID_ENTRY.op;
			insn_in_progress->m_Operands.clear();
			insn_in_progress->m_Successors.clear();
		}
    }
	int InstructionDecoder_aarch64::findInsnTableIndex(unsigned int decoder_table_index)
	{
		aarch64_mask_entry *cur_entry = &aarch64_mask_entry::main_decoder_table[decoder_table_index];
		unsigned int cur_mask = cur_entry->mask;

		if(cur_mask == 0)
		{
			/*if(cur_entry->insnTableIndices.size() == 1)
				return cur_entry->insnTableIndices[0];

			int insn_table_index = -1;
			for(std::vector<int>::iterator itr = cur_entry->insnTableIndices.begin(); itr != cur_entry->insnTableIndices.end(); itr++)
			{
				aarch64_insn_entry *nextEntry = &aarch64_insn_entry::main_insn_table[*itr];
				if((insn & nextEntry->_maskBits) == nextEntry->_encodingBits)
				{
					insn_table_index = *itr;
					break;
				}
			}*/

			int insn_table_index = cur_entry->insnTableIndex;
			if(insn_table_index == -1)
				assert(!"no instruction table entry found for current instruction");
			else
				return insn_table_index;
		}

		unsigned int insn_iter_index = 0, map_key_index = 0, branch_map_key = 0;
		branchMap cur_branches = cur_entry->nodeBranches;

		while(insn_iter_index < AARCH64_INSN_LENGTH)
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

	void InstructionDecoder_aarch64::setFlags()
	{
		isPstateWritten = true;
	}

    void InstructionDecoder_aarch64::mainDecode()
    {
		int insn_table_index = findInsnTableIndex(0);
		aarch64_insn_entry *insn_table_entry = &aarch64_insn_entry::main_insn_table[insn_table_index];

        insn_in_progress = makeInstruction(insn_table_entry->op, insn_table_entry->mnemonic, 4, reinterpret_cast<unsigned char*>(&insn));
        //insn_printf("ARM: %s\n", insn_table_entry->mnemonic);

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



