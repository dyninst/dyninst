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
#include "registers/aarch64_regs.h"

namespace Dyninst {
    namespace InstructionAPI {
        typedef void (InstructionDecoder_aarch64::*operandFactory)();

        typedef aarch64_insn_entry aarch64_insn_table[];
        typedef aarch64_mask_entry aarch64_decoder_table[];

        const std::array<std::string, 16> InstructionDecoder_aarch64::condNames = { {
            "eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc", "hi", "ls", "ge",
            "lt", "gt", "le", "al", "nv",
        } };
        
        const char* InstructionDecoder_aarch64::bitfieldInsnAliasMap(entryID e) {
            switch(e) {
            case aarch64_op_bfi_bfm: return "bfi";
            case aarch64_op_bfxil_bfm: return "bfxil";
            case aarch64_op_sbfiz_sbfm: return "sbfiz";
            case aarch64_op_sbfx_sbfm: return "sbfx";
            case aarch64_op_ubfiz_ubfm: return "ubfiz";
            case aarch64_op_ubfx_ubfm: return "ubfx";
            case aarch64_op_sxtb_sbfm: return "sxtb";
            case aarch64_op_sxth_sbfm: return "sxth";
            case aarch64_op_sxtw_sbfm: return "sxtw";
            case aarch64_op_uxtb_ubfm: return "uxtb";
            case aarch64_op_uxth_ubfm: return "uxth";
            case aarch64_op_lsl_ubfm: return "lsl";
            case aarch64_op_lsr_ubfm: return "lsr";
            default: assert(!"no alias for entryID");
            };
        }
        const char* InstructionDecoder_aarch64::condInsnAliasMap(entryID e) {
            switch(e) {
            case aarch64_op_csinc: return "csinc";
            case aarch64_op_csinv: return "csinv";
            case aarch64_op_csneg: return "csneg";
            case aarch64_op_cinc_csinc: return "cinc";
            case aarch64_op_cset_csinc: return "cset";
            case aarch64_op_cinv_csinv: return "cinv";
            case aarch64_op_csetm_csinv: return "csetm";
            case aarch64_op_cneg_csneg: return "cneg";
            default: assert(!"no alias for entryID");
            };
        }
        
        struct aarch64_insn_entry {
            entryID op;
            const char *mnemonic;
            std::size_t operandCnt;
            const operandFactory* operands;

            static const aarch64_insn_table main_insn_table;
            static const operandFactory operandTable[];
        };

        struct aarch64_mask_entry {
            unsigned int mask;
            std::size_t branchCnt;
            const std::pair<unsigned int,unsigned int>* nodeBranches;
            int insnTableIndex;

            static const aarch64_decoder_table main_decoder_table;
            static const std::pair<unsigned int,unsigned int> branchTable[];
        };

        InstructionDecoder_aarch64::InstructionDecoder_aarch64(Architecture a)
                : InstructionDecoderImpl(a), isPstateRead(false), isPstateWritten(false), isFPInsn(false),
                  isSIMDInsn(false), skipRn(false), skipRm(false),
                  is64Bit(true), isValid(true), insn(0),
                  hasHw(false), hasShift(false), hasOption(false), hasN(false),
                  immr(0), immrLen(0), sField(0), nField(0), nLen(0),
                  immlo(0), immloLen(0), _szField(-1), size(-1),
                  cmode(0), op(0), simdAlphabetImm(0), _Q(1) {
        }

        InstructionDecoder_aarch64::~InstructionDecoder_aarch64() {
        }

        void InstructionDecoder_aarch64::decodeOpcode(InstructionDecoder::buffer &b) {
            b.start += 4;
        }

        using namespace std;

        Instruction InstructionDecoder_aarch64::decode(InstructionDecoder::buffer &b) {
            if (b.start > b.end)
                return Instruction();

            isPstateRead = isPstateWritten = false;
            isFPInsn = false;
            isSIMDInsn = false;
            skipRm = skipRn = false;
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

            _szField = size = -1;
            _Q = 1;

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

            return *(insn_in_progress.get());
        }

        /* replace this function with a more generic function, which is setRegWidth
    void InstructionDecoder_aarch64::set32Mode()
    {
        // NOTE: is64Bit is set by default.
		is64Bit = false;
    }
    */
        void InstructionDecoder_aarch64::NOTHING() {
        }

        void InstructionDecoder_aarch64::setFPMode() {
            // NOTE: if it is fp, only isFP is set.
            isFPInsn = true;
        }

        //TODO: consistency issue
        void InstructionDecoder_aarch64::setSIMDMode() {
            // NOTE: if it is SIMD insn, both isFP and isSIMD are set.
            //isFPInsn = true;
            isSIMDInsn = true;
        }

        template<unsigned int endBit, unsigned int startBit>
        void InstructionDecoder_aarch64::OPRtype() {
            _typeField = field<startBit, endBit>(insn);
        }

        void InstructionDecoder_aarch64::processHwFieldInsn(int len, int val) {
            Result_Type rT = is64Bit ? u64 : u32;

            unsigned int shiftAmount = hwField * 16;

            Expression::Ptr lhs = Immediate::makeImmediate(
                    Result(rT, rT == u32 ? unsign_extend32(len, val) : unsign_extend64(len, val)));
            Expression::Ptr rhs = Immediate::makeImmediate(Result(u32, unsign_extend32(6, shiftAmount)));

            insn_in_progress->appendOperand(makeLeftShiftExpression(lhs, rhs, rT), true, false);
        }

        void InstructionDecoder_aarch64::processShiftFieldShiftedInsn(int len, int val) {
            Result_Type rT;
            Expression::Ptr lhs, rhs;

            rT = is64Bit ? u64 : u32;

            lhs = makeRmExpr();
            rhs = Immediate::makeImmediate(Result(u32, unsign_extend32(len, val)));

            switch (shiftField)                                            //add-sub (shifted) and logical (shifted)
            {
                case 0:
                    insn_in_progress->appendOperand(makeLeftShiftExpression(lhs, rhs, rT), true, false);
                    break;
                case 1:
                    insn_in_progress->appendOperand(makeRightLogicalShiftExpression(lhs, rhs, rT), true, false);
                    break;
                case 2:
                    insn_in_progress->appendOperand(makeRightArithmeticShiftExpression(lhs, rhs, rT), true, false);
                    break;
                case 3:
                    if (IS_INSN_LOGICAL_SHIFT(
                            insn))                    //logical (shifted) -- not applicable to add-sub (shifted)
                        insn_in_progress->appendOperand(makeRightRotateExpression(lhs, rhs, rT), true, false);
                    else
                        isValid = false;
                    break;
            }
        }

        void InstructionDecoder_aarch64::processShiftFieldImmInsn(int len, int val) {
            if (shiftField == 0 || shiftField == 1)                        //add-sub (immediate)
            {
                Result_Type rT = is64Bit ? u64 : u32;

                unsigned int shiftAmount = shiftField * 12;

                Expression::Ptr lhs = Immediate::makeImmediate(
                        Result(rT, rT == u32 ? unsign_extend32(len, val) : unsign_extend64(len, val)));
                Expression::Ptr rhs = Immediate::makeImmediate(Result(u32, unsign_extend32(4, shiftAmount)));

                insn_in_progress->appendOperand(makeLeftShiftExpression(lhs, rhs, rT), true, false);
            }
            else {
                isValid = false;
            }

        }

        Expression::Ptr InstructionDecoder_aarch64::makeOptionExpression(int len, int val) {
            MachRegister reg;
            int encoding = field<16, 20>(insn);

            reg = ((optionField & 0x3) == 0x3) ? ((encoding == 31) ? aarch64::xzr : aarch64::x0) : ((encoding == 31)
                                                                                                   ? aarch64::wzr
                                                                                                   : aarch64::w0);
            if (encoding != 31)
                reg = makeAarch64RegID(reg, encoding);

            Expression::Ptr lhs;

            switch (optionField) {
                case 0:
                    lhs = makeRegisterExpression(reg, u8);
                    break;
                case 1:
                    lhs = makeRegisterExpression(reg, u16);
                    break;
                case 2:
                    lhs = makeRegisterExpression(reg, u32);
                    break;
                case 3:
                    lhs = makeRegisterExpression(reg, u64);
                    break;
                case 4:
                    lhs = makeRegisterExpression(reg, s8);
                    break;
                case 5:
                    lhs = makeRegisterExpression(reg, s16);
                    break;
                case 6:
                    lhs = makeRegisterExpression(reg, s32);
                    break;
                case 7:
                    lhs = makeRegisterExpression(reg, s64);
                    break;
                default:
                    isValid = false;   //invalid option field
            }

            Result_Type rT = is64Bit ? (optionField < 4 ? u64 : s64) : (optionField < 4 ? u32 : s32);

            return makeLeftShiftExpression(lhs, Immediate::makeImmediate(Result(u32, unsign_extend32(len, val))), rT);
        }

        void InstructionDecoder_aarch64::processOptionFieldLSRegOffsetInsn() {
            if (optionField == 0x3)            //option = LSL
            {
                int sizeVal = field<30, 31>(insn);

                if (field<23, 23>(insn) == 1)
                    sizeVal = 4;

                unsigned extend = sField * sizeVal;
                int extendSize = 31;
                while (extendSize >= 0 && ((extend << (31 - extendSize)) & 0x80000000) == 0)
                    extendSize--;

                //above values need to be used in a dereference expression
            }
            else {
                //sign-extend
                switch (optionField) {
                    case 0x2://UXTW
                        break;
                    case 0x6://SXTW
                        break;
                    case 0x7://SXTX
                        break;
                    default:
                        isValid = false;
                        break;
                }
            }
        }

        void InstructionDecoder_aarch64::processSystemInsn() {
            int op0Field = field<19, 20>(insn), crnField = field<12, 15>(insn);

            if (op0Field == 0) {
                if (crnField == 3)            //clrex, dendBit, dmb, iendBit
                {
                    Expression::Ptr CRm = Immediate::makeImmediate(Result(u8, unsign_extend32(4, crmField)));

                    insn_in_progress->appendOperand(CRm, true, false);
                }
                else if (crnField == 2)                    //hint
                {
                    int immVal = (crmField << 3) | (op2Field & 7);

                    Expression::Ptr imm = Immediate::makeImmediate(Result(u8, unsign_extend32(7, immVal)));

                    insn_in_progress->appendOperand(imm, true, false);
                }
                else if (crnField == 4)                //msr (immediate)
                {
                    int pstatefield = (op1Field << 3) | (op2Field & 7);
                    insn_in_progress->appendOperand(
                            Immediate::makeImmediate(Result(u8, unsign_extend32(6, pstatefield))), true, false);

                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u8, unsign_extend32(4, crmField))),
                                                    true, false);
                    isPstateWritten = true;
                }
                else {
                    isValid = false;
                }
            }
            else if (op0Field == 1)                  //sys, sysl
            {
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u8, unsign_extend32(3, op1Field))),
                                                true, false);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u8, unsign_extend32(4, crnField))),
                                                true, false);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u8, unsign_extend32(4, crmField))),
                                                true, false);
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u8, unsign_extend32(3, op2Field))),
                                                true, false);

                bool isRtRead = (field<21, 21>(insn) == 0);
                insn_in_progress->appendOperand(makeRtExpr(), isRtRead, !isRtRead);
            }
            else                   //mrs (register), msr
            {
                bool isRtRead = (field<21, 21>(insn) == 0);

                unsigned int systemRegEncoding =
                        (op0Field << 14) | (op1Field << 11) | (crnField << 7) | (crmField << 3) | op2Field;

                MachRegister reg;
                if ((op0Field & 0x3) == 0x3 && (crnField & 0x3) == 0x3 && (crnField & 0x8) == 0x8)
                    reg = aarch64::IMPLEMENTATION_DEFINED_SYSREG;
                else
                    reg = sysRegMap(systemRegEncoding);
                insn_in_progress->appendOperand(makeRegisterExpression(reg), !isRtRead, isRtRead);
                insn_in_progress->appendOperand(makeRtExpr(), isRtRead, !isRtRead);
                if (!isRtRead)
                    insn_in_progress->m_Operands.reverse();
            }
        }

        Result_Type InstructionDecoder_aarch64::makeSizeType(unsigned int) {
            assert(0); //not implemented
            return u32;
        }

        // ****************
        // decoding opcodes
        // ****************

        MachRegister InstructionDecoder_aarch64::makeAarch64RegID(MachRegister base, unsigned int encoding) {
            return MachRegister(base.val() + encoding);
        }

        template<unsigned int endBit, unsigned int startBit>
        void InstructionDecoder_aarch64::OPRsize() {
            size = field<startBit, endBit>(insn);

            entryID insnID = insn_in_progress->getOperation().operationID;

            if((insnID == aarch64_op_pmull_advsimd && (size == 0x1 || size == 0x2)) ||
               ((IS_INSN_SIMD_3DIFF(insn) || IS_INSN_SCALAR_3DIFF(insn)) && size == 0x3) ||
               ((insnID == aarch64_op_sqdmull_advsimd_vec || insnID == aarch64_op_sqdmlal_advsimd_vec || insnID == aarch64_op_sqdmlsl_advsimd_vec) && size == 0)) {
                isValid = false;
            } else if(IS_INSN_SCALAR_3SAME(insn)) {
		int opcode = field<11, 15>(insn);
		int S = opcode & 0x1;

		if(opcode != 0x1 && opcode != 0x5 && ((opcode >> 3) & 0x3) != 0x3) {
		    if(opcode == 0x16) {
			if(size == 0 || size == 0x3)
			    isValid = false;
		    } else {
			if(((opcode >> 2) & 0x2) == 0x2) {
			    if(S == 0 && size != 0x3)
				isValid = false;
			} else if(size != 0x3) {
			    isValid = false;
			}
		    }
		}
	    } else if(IS_INSN_SCALAR_2REG_MISC(insn)) {
		int opcode = field<12, 16>(insn);

		if(((opcode == 0x14 || opcode == 0x12) && size == 0x3) ||
		    (opcode >= 0x8 && opcode <= 0xB && size != 0x3))
		    isValid = false;
	    }
        }

        Expression::Ptr InstructionDecoder_aarch64::makeRdExpr() {
            int encoding = field<0, 4>(insn);
            MachRegister reg;

            if (isSIMDInsn) {
                if (IS_INSN_SIMD_ACROSS(insn)) {
                    //fmaxnmv, fmaxv, fminnmv, fminv
                    if (field<14, 14>(insn) == 0x1) {
                        if (_szField == 0x0)
                            reg = aarch64::s0;
                        else
                            isValid = false;
                    }
                    else {
                        int opcode = field<12, 16>(insn);

                        //saddlv and uaddlv with opcode field 0x03 use different sets of registers
                        switch (size) {
                            case 0x0:
                                reg = (opcode == 0x03) ? aarch64::h0 : aarch64::b0;
                                break;
                            case 0x1:
                                reg = (opcode == 0x03) ? aarch64::s0 : aarch64::h0;
                                break;
                            case 0x2:
                                reg = (opcode == 0x03) ? aarch64::d0 : aarch64::s0;
                                break;
                            default:
                                isValid = false;
                        }
                    }
                }
                else if (IS_INSN_SIMD_COPY(insn)) {
                    unsigned int op_ = field<29, 29>(insn);
                    unsigned int imm4 = field<11, 14>(insn);

                    if (op_ == 0x1)
                        reg = aarch64::q0;
                    else {
                        switch (imm4) {
                            case 0x5:
                            case 0x7:
                                reg = _Q == 0x1 ? aarch64::x0 : aarch64::w0;
                                break;
                            default:
                                reg = _Q == 0x1 ? aarch64::q0 : aarch64::d0;
                                break;
                        }
                    }
                }
                else if (IS_INSN_SCALAR_COPY(insn) || IS_INSN_SCALAR_SHIFT_IMM(insn)) {
                    int switchbit;

                    if (IS_INSN_SCALAR_COPY(insn))
                        switchbit = lowest_set_bit(field<16, 20>(insn));
                    else
                        switchbit = highest_set_bit(field<19, 22>(insn));

                    switch (switchbit) {
                        case 0x1:
                            reg = aarch64::b0;
                            break;
                        case 0x2:
                            reg = aarch64::h0;
                            break;
                        case 0x3:
                            reg = aarch64::s0;
                            break;
                        case 0x4:
                            reg = aarch64::d0;
                            break;
                        default:
                            isValid = false;
                    }
                }
                else if (IS_INSN_SCALAR_3DIFF(insn)) {
                    switch (size) {
                        case 0x1:
                            reg = aarch64::s0;
                            break;
                        case 0x2:
                            reg = aarch64::d0;
                            break;
                        default:
                            isValid = false;
                    }
                }
                else if (IS_INSN_SCALAR_INDEX(insn)) {
                    int opcode = field<12, 15>(insn);

                    //sqdmlal, sqdmlsl, sqdmull
                    if ((opcode & 0x3) == 0x3) {
                        switch (size) {
                            case 0x1:
                                reg = aarch64::s0;
                                break;
                            case 0x2:
                                reg = aarch64::d0;
                                break;
                            default:
                                isValid = false;
                        }
                    }
                        //sqdmulh, sqrdmulh
                    else if ((opcode & 0xC) == 0xC) {
                        switch (size) {
                            case 0x1:
                                reg = aarch64::h0;
                                break;
                            case 0x2:
                                reg = aarch64::s0;
                                break;
                            default:
                                isValid = false;
                        }
                    }
                        //fmla, fmls, fmul, fmulx
                    else if ((opcode & 0x3) == 0x1) {
                        switch (_szField) {
                            case 0x0:
                                reg = aarch64::s0;
                                break;
                            case 0x1:
                                reg = aarch64::d0;
                                break;
                            default:
                                isValid = false;
                        }
			if(_szField == 0x1 && field<21, 21>(insn) == 0x1)
			    isValid = false;
                    }
                    else
                        isValid = false;
                }
                else if (IS_INSN_SCALAR_2REG_MISC(insn) || IS_INSN_SCALAR_3SAME(insn)) {
                    //some instructions in this set rely on sz for choosing the register and some on size
                    //only one of them is set for an instruction, however
                    if (_szField == -1) {
                        switch (size) {
                            case 0x0:
                                reg = aarch64::b0;
                                break;
                            case 0x1:
                                reg = aarch64::h0;
                                break;
                            case 0x2:
                                reg = aarch64::s0;
                                break;
                            case 0x3:
                                reg = aarch64::d0;
                                break;
                            default:
                                isValid = false;
                        }
                    }
                    else {
                        entryID op_ = insn_in_progress->getOperation().operationID;
                        
			switch (_szField) {
                            case 0x0:
                                reg = aarch64::s0;
				if(op_ == aarch64_op_fcvtxn_advsimd)
				    isValid = false;
                                break;
                            case 0x1: {
                                reg = (op_ == aarch64_op_fcvtxn_advsimd) ? aarch64::s0 : aarch64::d0;
                            }
                                break;
                            default:
                                isValid = false;
                        }
                    }
                }
                else if (IS_INSN_SCALAR_PAIR(insn)) {
                    if (size != -1) {
                        if (size == 0x3)
                            reg = aarch64::d0;
                        else
                            isValid = false;
                    }
                    else if (_szField != -1) {
                        switch (_szField) {
                            case 0x0:
                                reg = aarch64::s0;
                                break;
                            case 0x1:
                                reg = aarch64::d0;
                                break;
                        }
                    }
                    else
                        isValid = false;
                }
                else if (IS_INSN_SIMD_MOD_IMM(insn) && _Q == 0 && op == 1 && cmode == 0xE) {
                    reg = aarch64::d0;
                }
                else if (IS_INSN_SIMD_VEC_INDEX(insn)) {
                    if (field<13, 13>(insn) == 0x1)
                        reg = aarch64::q0;
                    else
                        reg = _Q == 0x1 ? aarch64::q0 : aarch64::d0;
                }
                else if (IS_INSN_SIMD_3DIFF(insn)) {
                    entryID op_ = insn_in_progress->getOperation().operationID;

                    if (op_ == aarch64_op_addhn_advsimd || op_ == aarch64_op_subhn_advsimd ||
                        op_ == aarch64_op_raddhn_advsimd || op_ == aarch64_op_rsubhn_advsimd)
                        reg = _Q == 0x1 ? aarch64::hq0 : aarch64::d0;
                    else
                        reg = aarch64::q0;
                }
                    // 3SAME, 2REG_MISC, EXTRACT
                else
                    reg = _Q == 0x1 ? aarch64::q0 : aarch64::d0;

                reg = makeAarch64RegID(reg, encoding);
            }
            else if (isFPInsn && !((IS_INSN_FP_CONV_FIX(insn) || (IS_INSN_FP_CONV_INT(insn))) && !IS_SOURCE_GP(insn))) {
                if (insn_in_progress->getOperation().operationID == aarch64_op_fcvt_float) {
                    int opc = field<15, 16>(insn);

		    if(opc == _typeField || opc == 0x2)
			isValid = false;
		    else {
			switch (opc) {
			    case 0:
				reg = aarch64::s0;
				break;
			    case 1:
				reg = aarch64::d0;
				break;
			    case 3:
				reg = aarch64::h0;
				break;
			    default:
				isValid = false;
			}
		    }
                }
                else
                    reg = isSinglePrec() ? aarch64::s0 : aarch64::d0;

                reg = makeAarch64RegID(reg, encoding);
            }
            else {
                if (encoding == 31)
                    reg = ((IS_INSN_ADDSUB_IMM(insn) || IS_INSN_ADDSUB_EXT(insn) || IS_INSN_LOGICAL_IMM(insn)) &&
                           !isPstateWritten) ? (is64Bit ? aarch64::sp : aarch64::wsp) : (is64Bit ? aarch64::xzr
                                                                                                 : aarch64::wzr);
                else
                    reg = is64Bit ? aarch64::x0 : aarch64::w0;

                if (isValid && encoding != 31)
                    reg = makeAarch64RegID(reg, encoding);
            }

            return makeRegisterExpression(reg);
        }

        void InstructionDecoder_aarch64::OPRRd() {
            Expression::Ptr reg = makeRdExpr();
            int cmode_ = field<12, 15>(insn);

            bool isRdRead = false;
            if (((IS_INSN_SIMD_VEC_INDEX(insn) || IS_INSN_SCALAR_INDEX(insn)) && !(cmode_ & 0x8)) ||
                (IS_INSN_SIMD_MOD_IMM(insn) &&
                 (((cmode_ & 0x8) && !(cmode_ & 0x4) && (cmode_ & 0x1)) ||
                  (!(cmode_ & 0x8) && (cmode_ & 0x1)))))
                isRdRead = true;
            //for SIMD/Scalar vector indexed set, some instructions read Rd and some don't. This can be determined from the highest bit of the opcode field (bit 15)
            insn_in_progress->appendOperand(reg, isRdRead, true);
        }

        void InstructionDecoder_aarch64::OPRcmode() {
            cmode = field<12, 15>(insn);
        }

        void InstructionDecoder_aarch64::OPRop() {
            op = field<29, 29>(insn);
        }

        void InstructionDecoder_aarch64::OPRa() {
            simdAlphabetImm |= (simdAlphabetImm & 0x7F) | (field<18, 18>(insn) << 7);
        }

        void InstructionDecoder_aarch64::OPRb() {
            simdAlphabetImm |= (simdAlphabetImm & 0xBF) | (field<17, 17>(insn) << 6);
        }

        void InstructionDecoder_aarch64::OPRc() {
            simdAlphabetImm |= (simdAlphabetImm & 0xDF) | (field<16, 16>(insn) << 5);
        }

        void InstructionDecoder_aarch64::OPRd() {
            simdAlphabetImm |= (simdAlphabetImm & 0xEF) | (field<9, 9>(insn) << 4);
        }

        void InstructionDecoder_aarch64::OPRe() {
            simdAlphabetImm |= (simdAlphabetImm & 0xF7) | (field<8, 8>(insn) << 3);
        }

        void InstructionDecoder_aarch64::OPRf() {
            simdAlphabetImm |= (simdAlphabetImm & 0xFB) | (field<7, 7>(insn) << 2);
        }

        void InstructionDecoder_aarch64::OPRg() {
            simdAlphabetImm |= (simdAlphabetImm & 0xFD) | (field<6, 6>(insn) << 1);
        }

        void InstructionDecoder_aarch64::OPRh() {
            simdAlphabetImm |= (simdAlphabetImm & 0xFE) | (field<5, 5>(insn));
        }

        void InstructionDecoder_aarch64::OPRlen() {
            //reuse immlo
            immlo = field<13, 14>(insn);
        }

        Expression::Ptr InstructionDecoder_aarch64::makeRnExpr() {
            int encoding = field<5, 9>(insn);
            MachRegister reg;

            if (isSIMDInsn && !IS_INSN_LDST(insn)) {
                if (IS_INSN_SIMD_COPY(insn)) {
                    unsigned int op_ = field<29, 29>(insn);
                    unsigned int imm4 = field<11, 14>(insn);
                    unsigned int imm5 = field<16, 20>(insn);

                    //ins (element)
                    if (op_ == 0x1) {
                        reg = (imm4 & 0x8) ? aarch64::q0 : aarch64::d0;
                    }
                    else {
                        switch (imm4) {
                            //dup (element), smov, umov
                            case 0x0:
                            case 0x5:
                            case 0x7:
                                reg = (imm5 & 0x10) ? aarch64::q0 : aarch64::d0;
                                break;
                                //dup (general), ins (general)
                            case 0x1:
                            case 0x3:
                                if (imm5 & 0x1 || imm5 & 0x2 || imm5 & 0x4) {
                                    reg = encoding == 31 ? aarch64::wzr : aarch64::w0;
                                }
                                else {
                                    reg = encoding == 31 ? aarch64::xzr : aarch64::x0;
                                }
                                break;
                            default:
                                isValid = false;
                                break;
                        }
                    }
                }
                else if (IS_INSN_SCALAR_COPY(insn)) {
                    int imm5 = field<16, 20>(insn);

                    reg = (imm5 & 0x10) ? aarch64::q0 : aarch64::d0;
                }
                else if (IS_INSN_SCALAR_PAIR(insn)) {
                    if (size != -1) {
                        if (size == 0x3)
                            reg = aarch64::q0;
                        else
                            isValid = false;
                    }
                    else if (_szField != -1) {
                        switch (_szField) {
                            case 0x0:
                                reg = aarch64::d0;
                                break;
                            case 0x1:
                                reg = aarch64::q0;
                                break;
                        }
                    }
                    else
                        isValid = false;
                }
                else if (IS_INSN_SCALAR_SHIFT_IMM(insn)) {
                    int switchbit = highest_set_bit(field<19, 22>(insn));
                    bool isRnVa = false;
                    int opcode = field<11, 15>(insn);

                    if ((opcode & 0x1C) == 0x10)
                        isRnVa = true;

                    switch (switchbit) {
                        case 0x1:
                            reg = isRnVa ? aarch64::h0 : aarch64::b0;
                            break;
                        case 0x2:
                            reg = isRnVa ? aarch64::s0 : aarch64::h0;
                            break;
                        case 0x3:
                            reg = isRnVa ? aarch64::d0 : aarch64::s0;
                            break;
                        case 0x4:
                            isRnVa ? (isValid = false) : (reg = aarch64::d0);
                            break;
                        default:
                            isValid = false;
                    }
                }
                else if (IS_INSN_SCALAR_3DIFF(insn)) {
                    switch (size) {
                        case 0x1:
                            reg = aarch64::h0;
                            break;
                        case 0x2:
                            reg = aarch64::s0;
                            break;
                        default:
                            isValid = false;
                    }
                }
                else if (IS_INSN_SCALAR_INDEX(insn)) {
                    int opcode = field<12, 15>(insn);

                    //sqdmlal, sqdmlsl, sqdmull
                    //sqdmulh, sqrdmulh
                    if ((opcode & 0xC) == 0xC || (opcode & 0x3) == 0x3) {
                        switch (size) {
                            case 0x1:
                                reg = aarch64::h0;
                                break;
                            case 0x2:
                                reg = aarch64::s0;
                                break;
                            default:
                                isValid = false;
                        }
                    }
                        //fmla, fmls, fmul, fmulx
                    else if ((opcode & 0x3) == 0x1) {
                        switch (_szField) {
                            case 0x0:
                                reg = aarch64::s0;
                                break;
                            case 0x1:
                                reg = aarch64::d0;
                                break;
                            default:
                                isValid = false;
                        }
                    }
                    else
                        isValid = false;
                }
                else if (IS_INSN_SCALAR_2REG_MISC(insn) || IS_INSN_SCALAR_3SAME(insn)) {
                    //some instructions in this set rely on sz for choosing the register and some on size
                    //only one of them is set for an instruction, however
                    bool isRnVa = false;
                    int opcode = field<12, 16>(insn);
                    if (!IS_INSN_SCALAR_3SAME(insn) && (opcode & 0x18) == 0x10 && (opcode & 0x1) == 0x0)
                        isRnVa = true;

                    if (_szField == -1) {
                        switch (size) {
                            case 0x0:
                                reg = isRnVa ? aarch64::h0 : aarch64::b0;
                                break;
                            case 0x1:
                                reg = isRnVa ? aarch64::s0 : aarch64::h0;
                                break;
                            case 0x2:
                                reg = isRnVa ? aarch64::d0 : aarch64::s0;
                                break;
                            case 0x3:
                                isRnVa ? (isValid = false) : (reg = aarch64::d0);
                                break;
                            default:
                                isValid = false;
                        }
                    }
                    else {
                        switch (_szField) {
                            case 0x0:
                                isRnVa ? (isValid = false) : (reg = aarch64::s0);
                                break;
                            case 0x1:
                                reg = aarch64::d0;
                                break;
                            default:
                                isValid = false;
                        }
                    }
                }
                else if (IS_INSN_SIMD_VEC_INDEX(insn)) {
                    //the below two conditions can easily be combined into one, but would be difficult to understand
                    if (field<13, 13>(insn) == 0x1)
                        reg = _Q == 0x1 ? aarch64::hq0 : aarch64::d0;
                    else
                        //sqdmulh, mul, sqrdmulh, fmla, fmls, fmul, mla, mls, fmulx
                        reg = _Q == 0x1 ? aarch64::q0 : aarch64::d0;
                }
                else if (IS_INSN_SIMD_TAB_LOOKUP(insn)) {
                    reg = _Q == 1 ? aarch64::q0 : aarch64::d0;

                    for (int reg_index = immlo; reg_index > 0; reg_index--) {
                        insn_in_progress->appendOperand(
                                makeRegisterExpression(makeAarch64RegID(reg, (encoding + reg_index) % 32)), true,
                                false);
                    }
                }
                else if (IS_INSN_SIMD_3DIFF(insn)) {
                    entryID op_ = insn_in_progress->getOperation().operationID;

                    if (op_ == aarch64_op_saddw_advsimd || op_ == aarch64_op_ssubw_advsimd ||
                        op_ == aarch64_op_addhn_advsimd || op_ == aarch64_op_subhn_advsimd ||
                        op_ == aarch64_op_uaddw_advsimd || op_ == aarch64_op_usubw_advsimd ||
                        op_ == aarch64_op_raddhn_advsimd || op_ == aarch64_op_rsubhn_advsimd)
                        reg = aarch64::q0;
                    else
                        reg = _Q == 0x1 ? aarch64::hq0 : aarch64::d0;
                }
                else
                    reg = _Q == 0x1 ? aarch64::q0 : aarch64::d0;

                if (!(reg == aarch64::wzr || reg == aarch64::xzr))
                    reg = makeAarch64RegID(reg, encoding);
            }
            else if (isFPInsn && !((IS_INSN_FP_CONV_FIX(insn) || (IS_INSN_FP_CONV_INT(insn))) && IS_SOURCE_GP(insn))) {
		if(insn_in_progress->getOperation().operationID == aarch64_op_fcvt_float) {
                    switch (_typeField) {
	                case 0:
	                    reg = aarch64::s0;
		            break;
		        case 1:
			    reg = aarch64::d0;
			    break;
                        case 3:
	                    reg = aarch64::h0;
	                    break;
		        default:
		            isValid = false;
		    }
		} else {
		    reg = isSinglePrec()?aarch64::s0:aarch64::d0;
		}

                reg = makeAarch64RegID(reg, encoding);
            }
            else if (IS_INSN_LDST(insn)) {
                reg = encoding == 31 ? aarch64::sp : aarch64::x0;

                if (encoding != 31)
                    reg = makeAarch64RegID(reg, encoding);
            }
            else {
		bool use32for64bit = false;
		if(field<24, 28>(insn) == 0x1B && (field<21, 23>(insn) == 0x1 || field<21, 23>(insn) == 0x5))
		   use32for64bit = true; 

                if (encoding == 31)
                    reg = (IS_INSN_ADDSUB_IMM(insn) || IS_INSN_ADDSUB_EXT(insn)) ? (is64Bit ? aarch64::sp
                                                                                            : aarch64::wsp) : ((is64Bit && !use32for64bit)
                                                                                                               ? aarch64::xzr
                                                                                                               : aarch64::wzr);
                else
                    reg = (is64Bit && !use32for64bit) ? aarch64::x0 : aarch64::w0;

                if (isValid && encoding != 31)
                    reg = makeAarch64RegID(reg, encoding);
            }

            return makeRegisterExpression(reg);
        }

        Expression::Ptr InstructionDecoder_aarch64::makePCExpr() {
            MachRegister baseReg = aarch64::pc;

            return makeRegisterExpression(makeAarch64RegID(baseReg, 0));
        }

        Expression::Ptr InstructionDecoder_aarch64::makePstateExpr() {
            MachRegister baseReg = aarch64::pstate;

            return makeRegisterExpression(makeAarch64RegID(baseReg, 0));
        }


        void InstructionDecoder_aarch64::getMemRefIndexLiteral_OffsetLen(int &immVal, int &immLen) {
            immVal = field<5, 23>(insn);
            immVal = immVal << 2; //immVal:00
            immLen = (23 - 5 + 1) + 2;
            return;
        }

        void InstructionDecoder_aarch64::getMemRefIndexLiteral_RT(Result_Type &rt) {
            int size_ = field<30, 31>(insn);
            switch (size_) {
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
                    break;
                default:
                    isValid = false;
                    break;
            }
        }

// ****************************************
// load/store literal
// eg: load Xt, <literal>
// => offset = signextend(<literal>:00, 64)
// load from [PC + offset] to Xt
// ****************************************
        Expression::Ptr InstructionDecoder_aarch64::makeMemRefIndexLiteral() {
            int immVal, immLen;
            getMemRefIndexLiteral_OffsetLen(immVal, immLen);

            Expression::Ptr label = Immediate::makeImmediate(Result(s64, sign_extend64(immLen, immVal)));

            Result_Type rt = invalid_type;
            getMemRefIndexLiteral_RT(rt);

            return makeDereferenceExpression(makeAddExpression(makePCExpr(), label, u64), rt);
        }

// TODO potential bug: do we need to distinguish signed and unsigned here?
// this funciton is to get the mem ref size
// shared by ld/st uimm, post, pre, reg
        void InstructionDecoder_aarch64::getMemRefIndex_RT(Result_Type &rt) {
            unsigned int opc1 = field<23, 23>(insn);
            unsigned int size_ = field<30, 31>(insn);

            if (opc1 == 1) {
                switch (size_) {
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
                        isValid = false;
                        //should only be 2 or 3
                        break;
                }
            }
            else {
                switch (size_) {
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
                        isValid = false;
                        //should only be 2 or 3
                        break;
                }
            }
        }

        void InstructionDecoder_aarch64::getMemRefPair_RT(Result_Type &rt) {
            unsigned int isSigned = field<30, 30>(insn);
            unsigned int size_ = field<31, 31>(insn);

            // double the width
            switch (isSigned) {
                case 0:
                    switch (size_) {
                        case 0:
                            //rt = u32;
                            rt = u64;
                            break;
                        case 1:
                            //rt = u64;
                            rt = dbl128;
                            break;
                        default:
                            isValid = false;
                    }
                    break;
                case 1:
                    switch (size_) {
                        case 0:
                            //rt = s32;
                            rt = s64;
                            break;
                        case 1:
                        default:
                            isValid = false;
                    }
                    break;
                default:
                    isValid = false;
                    break;
            }
        }

        void InstructionDecoder_aarch64::getMemRefIndex_SizeSizelen(unsigned int &size_, unsigned int &sizeLen) {
            size_ = field<30, 31>(insn);
            if (isSIMDInsn && size_ == 0x0 && field<23, 23>(insn) == 0x1)
                size_ = 4;
            sizeLen = 31 - 30 + 1 + (size_ / 4);
            return;
        }

        void InstructionDecoder_aarch64::getMemRefIndexPrePost_ImmImmlen(unsigned int &immVal, unsigned int &immLen) {
            immVal = field<12, 20>(insn);
            immLen = 20 - 12 + 1;
            return;
        }

        void InstructionDecoder_aarch64::getMemRefPair_ImmImmlen(unsigned int &immVal, unsigned int &immLen) {
            immVal = field<15, 21>(insn);
            immLen = 21 - 15 + 1;
            return;
        }

// ****************************************
// load/store unsigned imm
// eg: load Xt, [Xn, #imm]
// => offset = unsignextend( imm , 64)
// load from [PC + offset] to Xt
// ****************************************
        Expression::Ptr InstructionDecoder_aarch64::makeMemRefIndexUImm() {
            assert(IS_INSN_LDST_UIMM(insn));

            int immVal = field<10, 21>(insn);
            int immLen = 21 - 10 + 1;

            unsigned int size_ = 0, sizeLen = 0;
            getMemRefIndex_SizeSizelen(size_, sizeLen);

            Expression::Ptr offset = Immediate::makeImmediate(
                    Result(u64, unsign_extend64(immLen + size_, immVal << size_)));

            Result_Type rt;
            getMemRefIndex_RT(rt);
            return makeDereferenceExpression(makeAddExpression(makeRnExpr(), offset, u64), rt);
        }

        Expression::Ptr InstructionDecoder_aarch64::makeMemRefIndex_offset9() {
            unsigned int immVal = 0, immLen = 0;
            getMemRefIndexPrePost_ImmImmlen(immVal, immLen);
            return Immediate::makeImmediate(Result(u32, sign_extend32(immLen, immVal)));
        }

// scale = 2 + opc<1>
// scale = 1<<scale
// LSL(sign_ex(imm7 , 64), scale)
        Expression::Ptr InstructionDecoder_aarch64::makeMemRefPair_offset7() {
            /*
    unsigned int scaleVal = field<31, 31>(insn);
    unsigned int scaleLen = 8;
    scaleVal += 2;
    Expression::Ptr scale = Immediate::makeImmediate(Result(u32, unsign_extend32(scaleLen, 1<<scaleVal)));
    */

            unsigned int immVal = 0, immLen = 0;
            getMemRefPair_ImmImmlen(immVal, immLen);

            int scale = 2;
            if (isSIMDInsn)
                scale += field<30, 31>(insn);
            else
                scale += field<31, 31>(insn);

            //return makeMultiplyExpression(imm7, scale, s64);
            return Immediate::makeImmediate(Result(s64, sign_extend64(immLen, immVal) << scale));
        }

        Expression::Ptr InstructionDecoder_aarch64::makeMemRefIndex_addOffset9() {
            Expression::Ptr offset = makeMemRefIndex_offset9();
            return makeAddExpression(makeRnExpr(), offset, u64);
        }

        Expression::Ptr InstructionDecoder_aarch64::makeMemRefPair_addOffset7() {
            Expression::Ptr offset = makeMemRefPair_offset7();
            return makeAddExpression(makeRnExpr(), offset, u64);
        }

        Expression::Ptr InstructionDecoder_aarch64::makeMemRefIndexPre() {
            Result_Type rt;
            getMemRefIndex_RT(rt);
            return makeDereferenceExpression(makeMemRefIndex_addOffset9(), rt);
        }

        Expression::Ptr InstructionDecoder_aarch64::makeMemRefIndexPost() {
            Result_Type rt;
            getMemRefIndex_RT(rt);
            return makeDereferenceExpression(makeRnExpr(), rt);
        }

        Expression::Ptr InstructionDecoder_aarch64::makeMemRefPairPre() {
            Result_Type rt{};
            getMemRefPair_RT(rt);
            return makeDereferenceExpression(makeMemRefPair_addOffset7(), rt);
        }

        Expression::Ptr InstructionDecoder_aarch64::makeMemRefPairPost() {
            Result_Type rt{};
            getMemRefPair_RT(rt);

            return makeDereferenceExpression(makeRnExpr(), rt);
        }

        void InstructionDecoder_aarch64::getMemRefEx_RT(Result_Type &rt) {
            unsigned int sz = field<30, 31>(insn);
            switch (sz) {
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
        }

        Expression::Ptr InstructionDecoder_aarch64::makeMemRefEx() {
            Result_Type rt;
            getMemRefEx_RT(rt);
            return makeDereferenceExpression(makeRnExpr(), rt);
        }

        void InstructionDecoder_aarch64::OPRQ() {
            _Q = field<30, 30>(insn);
        }

        void InstructionDecoder_aarch64::OPRL() {
            _L = field<30, 30>(insn);
        }

        unsigned int InstructionDecoder_aarch64::getMemRefSIMD_MULT_T() {
            unsigned int Q = field<30, 30>(insn);
            return Q ? 128 : 64;
        }

        void InstructionDecoder_aarch64::getMemRefSIMD_MULT_RT(Result_Type &rt) {
            unsigned int tmpSize = getMemRefSIMD_MULT_T();
            unsigned int rpt = 0, selem = 0;
            getSIMD_MULT_RptSelem(rpt, selem);
            tmpSize = tmpSize * rpt * selem;
            switch (tmpSize) {
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
                    isValid = false;
                    return;
            }
        }

        unsigned int InstructionDecoder_aarch64::getSIMD_SING_selem() {
            return (((field<13, 13>(insn) << 1) & 0x2) | (field<21, 21>(insn) & 0x1)) + 0x1;
        }

        void InstructionDecoder_aarch64::getMemRefSIMD_SING_RT(Result_Type &rt) {
            unsigned int tmpSize = getMemRefSIMD_SING_T();
            unsigned int selem = getSIMD_SING_selem();
            switch (selem * tmpSize) {
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
                    isValid = false;
                    break;
            }
        }

        unsigned int InstructionDecoder_aarch64::getMemRefSIMD_SING_T() {
            unsigned int opcode = field<14, 15>(insn);
            unsigned int S = field<12, 12>(insn);
            unsigned int size_ = field<10, 11>(insn);

            switch (opcode) {
                case 0x0:
                    return 8;
                case 0x1:
                    if ((size_ & 0x1) == 0x0)
                        return 16;
                    else
                        isValid = false;
                    break;
                case 0x2:
                    if (size_ == 0x0)
                        return 32;
                    else if (size_ == 0x1 && S == 0)
                        return 64;
                    else
                        isValid = false;
                    break;
                case 0x3:
                    return 8 << size_;
                default:
                    isValid = false;
            }

            return 0;
        }

        Expression::Ptr InstructionDecoder_aarch64::makeMemRefSIMD_MULT() {
            Result_Type rt = invalid_type;
            getMemRefSIMD_MULT_RT(rt);
            return makeDereferenceExpression(makeRnExpr(), rt);
        }

        Expression::Ptr InstructionDecoder_aarch64::makeMemRefSIMD_SING() {
            Result_Type rt = invalid_type;
            getMemRefSIMD_SING_RT(rt);
            return makeDereferenceExpression(makeRnExpr(), rt);
        }

        void InstructionDecoder_aarch64::getMemRefExPair_RT(Result_Type &rt) {
            int size_ = field<30, 30>(insn);
            switch (size_) {
                case 0:
                    rt = u64;
                    break;
                case 1:
                    rt = dbl128;
                    break;
                default:
                    isValid = false;
                    break;
            }
        }

        Expression::Ptr InstructionDecoder_aarch64::makeMemRefExPair() {
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

        Expression::Ptr InstructionDecoder_aarch64::makeMemRefReg_Rm() {
            unsigned int option = field<13, 15>(insn);
            if ((option & 2) != 2)
                isValid = false;
            MachRegister baseReg = ((option & 0x3) == 0x2) ? aarch64::w0 : aarch64::x0;
            unsigned int encoding = field<16, 20>(insn);

            if (encoding == 31)//zero register
                return makeRegisterExpression(makeAarch64RegID(aarch64::xzr, 0));
            else
                return makeRegisterExpression(makeAarch64RegID(baseReg, encoding));
        }

        Expression::Ptr InstructionDecoder_aarch64::makeMemRefReg_amount() {
            unsigned int S = field<12, 12>(insn);
            unsigned int amountVal = is64Bit ? (S == 0 ? 0 : 2) : (S == 0 ? 0 : 3);
            unsigned int amountLen = 2;

            return Immediate::makeImmediate(Result(u32, unsign_extend32(amountLen, amountVal)));
        }

        Expression::Ptr InstructionDecoder_aarch64::makeMemRefReg_ext() {
            int immLen = 2;
            int immVal = 0; //for amount

            int size_ = field<30, 31>(insn);

            if(sField == 1)
                immVal = size_;
            else
                immVal = 0;

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
        Expression::Ptr InstructionDecoder_aarch64::makeMemRefReg() {
            Expression::Ptr ext = makeMemRefReg_ext();
            Expression::Ptr xn = makeRnExpr();
            Expression::Ptr add = makeAddExpression(xn, ext, u64);

            Result_Type rt;
            getMemRefIndex_RT(rt);
            return makeDereferenceExpression(add, rt);
        }

        void InstructionDecoder_aarch64::LIndex() {
            // never be called
            if (IS_INSN_LD_LITERAL(insn)) {
                insn_in_progress->appendOperand(makeMemRefIndexLiteral(), true, false);
            }
                // ******************
                // load register offset
                // ******************
            else if (IS_INSN_LDST_REG(insn)) {
                insn_in_progress->appendOperand(makeMemRefReg(), true, false);
            }
                // ******************
                // load unsigned imm
                // ******************
            else if (IS_INSN_LDST_UIMM(insn)) {
                insn_in_progress->appendOperand(makeMemRefIndexUImm(), true, false);
            }
                // ******************
                // load pre, unscaled and unprivlidged
                // ******************
            else if (IS_INSN_LDST_PRE(insn)
                     || IS_INSN_LDST_UNPRIV(insn)
                     || IS_INSN_LDST_UNSCALED(insn)) {
                insn_in_progress->appendOperand(makeMemRefIndexPre(), true, false);
            }
            else if (IS_INSN_LDST_POST(insn)) {
                insn_in_progress->appendOperand(makeMemRefIndexPost(), true, false);
            }
                // ****************************
                // load PAIR pre, post, offset
                // ****************************
            else if (IS_INSN_LDST_PAIR_PRE(insn)
                     || IS_INSN_LDST_PAIR_NOALLOC(insn)
                     || IS_INSN_LDST_PAIR_OFFSET(insn)) {
                insn_in_progress->appendOperand(makeMemRefPairPre(), true, false);
            }
            else if (IS_INSN_LDST_PAIR_POST(insn)) {
                insn_in_progress->appendOperand(makeMemRefPairPost(), true, false);
            }
                // ****************************
                // load exclusive instructions
                // ****************************
            else if (IS_INSN_LDST_EX(insn)) {
                if (!IS_INSN_LDST_EX_PAIR(insn)) { // Rt2 field == 31, non-pair op
                    insn_in_progress->appendOperand(makeMemRefEx(), true, false);
                }
                else { // pair
                    insn_in_progress->appendOperand(makeMemRefExPair(), true, false);
                }
            }
                // ****************************
                // load SIMD multiple structures &
                // load SIMD multiple structures post increment
                // ****************************
            else if (IS_INSN_LDST_SIMD_MULT(insn)
                     || IS_INSN_LDST_SIMD_MULT_POST(insn)) {
                insn_in_progress->appendOperand(makeMemRefSIMD_MULT(), true, false);
            }
                // ****************************
                // load SIMD single structure &
                // load SIMD single structure post increment
                // ****************************
            else if (IS_INSN_LDST_SIMD_SING(insn)
                     || IS_INSN_LDST_SIMD_SING_POST(insn)) {
                insn_in_progress->appendOperand(makeMemRefSIMD_SING(), true, false);
            }
            else
                assert(0);
        }

        void InstructionDecoder_aarch64::STIndex() {
            if (IS_INSN_LD_LITERAL(insn))
                assert(0); // only load literal, no store literal
                // ******************
                // ld/st register offset
                // ******************
            else if (IS_INSN_LDST_REG(insn)) {
                insn_in_progress->appendOperand(makeMemRefReg(), false, true);
            }
            else if (IS_INSN_LDST_UIMM(insn)) {
                insn_in_progress->appendOperand(makeMemRefIndexUImm(), false, true);
            }
                // ******************
                // ld/st pre and post, unscaled and unprivilidged
                // ******************
            else if (IS_INSN_LDST_PRE(insn)
                     || IS_INSN_LDST_UNPRIV(insn)
                     || IS_INSN_LDST_UNSCALED(insn)) {
                insn_in_progress->appendOperand(makeMemRefIndexPre(), false, true);
            }
            else if (IS_INSN_LDST_POST(insn)) {
                insn_in_progress->appendOperand(makeMemRefIndexPost(), false, true);
            }
                // ****************************
                // ld/st PAIR pre, post, offset
                // ****************************
            else if (IS_INSN_LDST_PAIR_PRE(insn)
                     || IS_INSN_LDST_PAIR_NOALLOC(insn)
                     || IS_INSN_LDST_PAIR_OFFSET(insn)) {
                insn_in_progress->appendOperand(makeMemRefPairPre(), false, true);
            }
            else if (IS_INSN_LDST_PAIR_POST(insn)) {
                insn_in_progress->appendOperand(makeMemRefPairPost(), false, true);
            }
                // ****************************
                // ld/st exclusive instructions
                // ****************************
            else if (IS_INSN_LDST_EX(insn)) {
                if (!IS_INSN_LDST_EX_PAIR(insn)) { // Rt2 field == 31, non-pair op
                    insn_in_progress->appendOperand(makeMemRefEx(), false, true);
                }
                else { // pair
                    insn_in_progress->appendOperand(makeMemRefExPair(), false, true);
                }
            }
                // ****************************
                // store SIMD multiple structures &
                // store SIMD multiple structures post increment
                // ****************************
            else if (IS_INSN_LDST_SIMD_MULT(insn)
                     || IS_INSN_LDST_SIMD_MULT_POST(insn)) {
                insn_in_progress->appendOperand(makeMemRefSIMD_MULT(), false, true);
            }
            else if (IS_INSN_LDST_SIMD_SING(insn)
                     || IS_INSN_LDST_SIMD_SING_POST(insn)) {
                insn_in_progress->appendOperand(makeMemRefSIMD_SING(), false, true);
            }
            else
                assert(0); //un-handled case

        }

// This function is for non-writeback
        void InstructionDecoder_aarch64::OPRRn() {
	    if(skipRn)
		return;

            if (IS_INSN_B_UNCOND_REG(insn))                                        //unconditional branch (register)
            {
                int branchType = field<21, 22>(insn);

                insn_in_progress->appendOperand(makePCExpr(), false, true, true);
                insn_in_progress->addSuccessor(makeRnExpr(), field<21, 21>(insn) == 1, true, false, false);

                if (branchType == 0x1) {
                    insn_in_progress->addSuccessor(makeFallThroughExpr(), false, false, false, true);
                }
            }
            else
                insn_in_progress->appendOperand(makeRnExpr(), true, false);
        }

        void InstructionDecoder_aarch64::OPRRnL() {
            LIndex();
        }

        void InstructionDecoder_aarch64::OPRRnS() {
            STIndex();
        }

        void InstructionDecoder_aarch64::OPRRnU() {
            assert(0);
            /* this functions is useless
	insn_in_progress->appendOperand(makeRnExpr(), true, true);
    */
        }

        void InstructionDecoder_aarch64::OPRRnLU() {
            if (IS_INSN_LDST_PRE(insn) || IS_INSN_LDST_POST(insn) || IS_INSN_LDST_PAIR_PRE(insn) ||
                IS_INSN_LDST_PAIR_POST(insn))
                LIndex();
        }

        void InstructionDecoder_aarch64::OPRRnSU() {
            if (IS_INSN_LDST_PRE(insn) || IS_INSN_LDST_POST(insn) || IS_INSN_LDST_PAIR_PRE(insn) ||
                IS_INSN_LDST_PAIR_POST(insn))
                STIndex();
        }

        unsigned int InstructionDecoder_aarch64::get_SIMD_MULT_POST_imm() {
            unsigned int rpt = 0, selem = 0;
            getSIMD_MULT_RptSelem(rpt, selem);
            unsigned int numReg = rpt * selem;
            return _Q == 0x1 ? numReg << 4 : numReg << 3;
        }

        unsigned int InstructionDecoder_aarch64::get_SIMD_SING_POST_imm() {
            return (getMemRefSIMD_SING_T() >> 3) * getSIMD_SING_selem();
        }

        Expression::Ptr InstructionDecoder_aarch64::makeRmExpr() {
            int encoding = field<16, 20>(insn);
            MachRegister reg;

            if (isSIMDInsn) {
                if (IS_INSN_LDST_SIMD_MULT_POST(insn)) {
                    if (encoding == 31) {
                        unsigned int immVal = get_SIMD_MULT_POST_imm();
                        unsigned int immLen = 8;

                        return Immediate::makeImmediate(Result(u32, unsign_extend32(immLen, immVal)));
                    }
                    else
                        reg = aarch64::x0;
                }
                else if (IS_INSN_LDST_SIMD_SING_POST(insn)) {
                    if (encoding == 31) {
                        unsigned int immVal = get_SIMD_SING_POST_imm();
                        unsigned int immLen = 8;

                        return Immediate::makeImmediate(Result(u32, unsign_extend32(immLen, immVal)));
                    }
                    else
                        reg = aarch64::x0;
                }
                else if (IS_INSN_SIMD_VEC_INDEX(insn) || IS_INSN_SCALAR_INDEX(insn)) {
                    reg = field<11, 11>(insn) == 0x1 ? aarch64::q0 : aarch64::d0;

                    if (size == 0x0 || size == 0x3)
                        isValid = false;
                    else if (size == 0x1)
                        encoding = encoding & 0xF;
                }
                else if (IS_INSN_SCALAR_3DIFF(insn)) {
                    switch (size) {
                        case 0x1:
                            reg = aarch64::h0;
                            break;
                        case 0x2:
                            reg = aarch64::s0;
                            break;
                        default:
                            isValid = false;
                    }
                }
                else if (IS_INSN_SIMD_3DIFF(insn)) {
                    entryID op_ = insn_in_progress->getOperation().operationID;

                    if (op_ == aarch64_op_addhn_advsimd || op_ == aarch64_op_subhn_advsimd ||
                        op_ == aarch64_op_raddhn_advsimd || op_ == aarch64_op_rsubhn_advsimd)
                        reg = aarch64::q0;
                    else
                        reg = _Q == 0x1 ? aarch64::hq0 : aarch64::d0;
                }
                else if (IS_INSN_SCALAR_3SAME(insn)) {
                    if (size != -1) {
                        switch (size) {
                            case 0x0:
                                reg = aarch64::b0;
                                break;
                            case 0x1:
                                reg = aarch64::h0;
                                break;
                            case 0x2:
                                reg = aarch64::s0;
                                break;
                            case 0x3:
                                reg = aarch64::d0;
                                break;
                            default:
                                isValid = false;
                        }
                    }
                    else if (_szField != -1) {
                        switch (_szField) {
                            case 0x0:
                                reg = aarch64::s0;
                                break;
                            case 0x1:
                                reg = aarch64::d0;
                                break;
                            default:
                                isValid = false;
                        }
                    }
                    else
                        isValid = false;
                }
                else
                    reg = _Q == 0x1 ? aarch64::q0 : aarch64::d0;

                reg = makeAarch64RegID(reg, encoding);

                return makeRegisterExpression(reg);
            }
            else if (isFPInsn) {
                reg = isSinglePrec() ? aarch64::s0 : aarch64::d0;
                reg = makeAarch64RegID(reg, encoding);
            }
            else {
		bool use32for64bit = false;
		if(field<24, 28>(insn) == 0x1B && (field<21, 23>(insn) == 0x1 || field<21, 23>(insn) == 0x5))
		   use32for64bit = true; 
                
		reg = (is64Bit && !use32for64bit) ? ((encoding == 31) ? aarch64::xzr : aarch64::x0) : ((encoding == 31) ? aarch64::wzr
                                                                                                   : aarch64::w0);
                if (encoding != 31)
                    reg = makeAarch64RegID(reg, encoding);
            }

            return makeRegisterExpression(reg);
        }

        void InstructionDecoder_aarch64::OPRRm() {
	    if(skipRm)
		return;

            if (IS_INSN_FP_COMPARE(insn) && field<3, 3>(insn) == 1)
                insn_in_progress->appendOperand(
                        Immediate::makeImmediate(Result(isSinglePrec() ? sp_float : dp_float, 0.0)), true, false);
            else
                insn_in_progress->appendOperand(makeRmExpr(), true, false);
        }

        void InstructionDecoder_aarch64::OPRsf() {
            if (field<31, 31>(insn) == 0)
                is64Bit = false;
        }

        template<unsigned int endBit, unsigned int startBit>
        void InstructionDecoder_aarch64::OPRoption() {
            hasOption = true;
            optionField = field<startBit, endBit>(insn);
        }

        void InstructionDecoder_aarch64::OPRshift() {
            hasShift = true;
            shiftField = field<22, 23>(insn);
        }

        void InstructionDecoder_aarch64::OPRhw() {
            hasHw = true;
            hwField = field<21, 22>(insn);

            if(!is64Bit && ((hwField >> 1) & 0x1) == 1)
                isValid = false;
        }

        template<unsigned int endBit, unsigned int startBit>
        void InstructionDecoder_aarch64::OPRN() {
            hasN = true;
            nField = field<startBit, endBit>(insn);
            nLen = endBit - startBit + 1;
        }

// the call of this function should be generated by decoder gen for ld/st class
// except for ld/st{s, h, w} subclass
        void InstructionDecoder_aarch64::setRegWidth() {
            //is64Bit is by default set to TRUE
            unsigned int opc = 0x3 & field<22, 23>(insn);
            unsigned int opc0 = opc & 0x1;
            unsigned int opc1 = (opc & 0x2) >> 1;
            unsigned int sz = 0x3 & field<30, 31>(insn);

            if (IS_INSN_LDST(insn)) {
                if (IS_INSN_LDST_UIMM(insn) || IS_INSN_LDST_UNSCALED(insn)
                    || IS_INSN_LDST_UNPRIV(insn) || IS_INSN_LDST_POST(insn)
                    || IS_INSN_LDST_PRE(insn) || IS_INSN_LDST_REG(insn)) {
                    if (opc1 == 0) {
                        if (field<30, 31>(insn) != 3)
                            is64Bit = false;
                        return;
                    } else {
                        if (sz == 3) {
                            if (opc0 == 1)
                                isValid = false;
                        }
                        else {
                            if (sz == 2 && opc0 == 1)
                                isValid = false;
                            if (opc0 == 1)
                                is64Bit = false;
                        }
                        return;
                    }
                }
                else if (IS_INSN_LDST_EX(insn)) {
                    switch (sz) {
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
                else if (IS_INSN_LDST_PAIR(insn)) {
                    switch (sz) {
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
                else if (IS_INSN_LD_LITERAL(insn)) {
                    switch (sz) {
                        case 0:
                            is64Bit = false;
                        case 1:
                        case 2:
                        case 3:
                        default:
                            return;
                    }
                }
                else {
                    isValid = false;
                }
            } else {
                isValid = false;
            }
            return;
        }

        MachRegister InstructionDecoder_aarch64::getLoadStoreSimdRegister(int encoding) {
            MachRegister reg;

            if (size != -1) {
                switch (size) {
                    case 0x0:
                        reg = (field<23, 23>(insn) == 0x1) ? aarch64::q0 : aarch64::b0;
                        break;
                    case 0x1:
                        reg = aarch64::h0;
                        break;
                    case 0x2:
                        reg = aarch64::s0;
                        break;
                    case 0x3:
                        reg = aarch64::d0;
                        break;
                }
            }
            else {
                switch (field<30, 31>(insn)) {
                    case 0x0:
                        reg = aarch64::s0;
                        break;
                    case 0x1:
                        reg = aarch64::d0;
                        break;
                    case 0x2:
                        reg = aarch64::q0;
                        break;
                    case 0x3:
                        isValid = false;
                        break;
                }
            }

            return makeAarch64RegID(reg, encoding);
        }

        Expression::Ptr InstructionDecoder_aarch64::makeRtExpr() {
            int encoding = field<0, 4>(insn);
            MachRegister reg;

            if (isFPInsn) {
                reg = makeAarch64RegID(isSinglePrec() ? aarch64::s0 : aarch64::d0, encoding);
            }
            else if (isSIMDInsn) {
                reg = getLoadStoreSimdRegister(encoding);
            }
            else {
                reg = is64Bit ? ((encoding == 31) ? aarch64::xzr : aarch64::x0) : ((encoding == 31) ? aarch64::wzr
                                                                                                   : aarch64::w0);
                if (encoding != 31)
                    reg = makeAarch64RegID(reg, encoding);
            }

            return makeRegisterExpression(reg);
        }

        void InstructionDecoder_aarch64::getSIMD_MULT_RptSelem(unsigned int &rpt, unsigned int &selem) {
            unsigned opcode = field<12, 15>(insn);
            switch (opcode) {
                case 0x0:
                    rpt = 1;
                    selem = 4;
                    break;
                case 0x2:
                    rpt = 4;
                    selem = 1;
                    break;
                case 0x4:
                    rpt = 1;
                    selem = 3;
                    break;
                case 0x6:
                    rpt = 3;
                    selem = 1;
                    break;
                case 0x7:
                    rpt = 1;
                    selem = 1;
                    break;
                case 0x8:
                    rpt = 1;
                    selem = 2;
                    break;
                case 0xa:
                    rpt = 2;
                    selem = 1;
                    break;
                default:
                    rpt = 0;
                    selem = 0;
                    isValid = false;
                    return;
            }
            if (rpt == 0 || selem == 0)
                isValid = false;
            return;
        }

        void InstructionDecoder_aarch64::OPRRt() {
            int encoding = field<0, 4>(insn);
            entryID op_ = insn_in_progress->getOperation().operationID;

            if (IS_INSN_BRANCHING(insn)) {
                if (encoding == 31)
                    insn_in_progress->appendOperand(makeRegisterExpression(is64Bit ? aarch64::xzr : aarch64::wzr), true,
                                                    false);
                else
                    insn_in_progress->appendOperand(
                            makeRegisterExpression(makeAarch64RegID(is64Bit ? aarch64::x0 : aarch64::w0, encoding)),
                            true, false);
            }
            else if (op_ == aarch64_op_prfm_imm || op_ == aarch64_op_prfm_lit || op_ == aarch64_op_prfm_reg || op_ == aarch64_op_prfum) {
                Expression::Ptr prfop;
		Result arg = Result(u32, unsign_extend32(5, encoding));
		
		if((encoding & 0x1E) == 0x6 || (encoding & 0x1E) == 0xE || (encoding & 0x1E) == 0x16 || (encoding & 0x18) == 0x18)
		    prfop = Immediate::makeImmediate(arg);
		else
		    prfop = ArmPrfmTypeImmediate::makeArmPrfmTypeImmediate(arg);

                insn_in_progress->appendOperand(prfop, true, false);
            }
        }

        void InstructionDecoder_aarch64::OPRRtL() {
            int encoding = field<0, 4>(insn);

            if (IS_INSN_LDST_SIMD_MULT(insn) || IS_INSN_LDST_SIMD_MULT_POST(insn)) {
                unsigned int rpt, selem;
                getSIMD_MULT_RptSelem(rpt, selem);
                if (!isValid)  {
                    return;
                }
                MachRegister reg = _Q == 0x1 ? aarch64::q0 : aarch64::d0;
                for (int it_rpt = rpt * selem - 1; it_rpt >= 0; it_rpt--) {
                    insn_in_progress->appendOperand(
                            makeRegisterExpression(makeAarch64RegID(reg, (encoding + it_rpt) % 32)), false, true);
                }
            }
            else if (IS_INSN_LDST_SIMD_SING(insn) || IS_INSN_LDST_SIMD_SING_POST(insn)) {
                unsigned int selem = getSIMD_SING_selem();

                MachRegister reg = _Q == 0x1 ? aarch64::q0 : aarch64::d0;

                for (int it_selem = selem - 1; it_selem >= 0; it_selem--) {
                    insn_in_progress->appendOperand(
                            makeRegisterExpression(makeAarch64RegID(reg, (encoding + it_selem) % 32)), false, true);
                }
            }
            else
                insn_in_progress->appendOperand(makeRtExpr(), false, true);
        }

        void InstructionDecoder_aarch64::OPRRtS() {
            int encoding = field<0, 4>(insn);

            if (IS_INSN_LDST_SIMD_MULT(insn) || IS_INSN_LDST_SIMD_MULT_POST(insn)) {
                unsigned int rpt, selem;
                getSIMD_MULT_RptSelem(rpt, selem);
                if (!isValid)  {
                    return;
                }

                MachRegister reg = _Q == 0x1 ? aarch64::q0 : aarch64::d0;

                for (int it_rpt = rpt * selem - 1; it_rpt >= 0; it_rpt--) {
                    insn_in_progress->appendOperand(
                            makeRegisterExpression(makeAarch64RegID(reg, (encoding + it_rpt) % 32)), true, false);
                }
            }
            else if (IS_INSN_LDST_SIMD_SING(insn) || IS_INSN_LDST_SIMD_SING_POST(insn)) {
                unsigned int selem = getSIMD_SING_selem();

                MachRegister reg = _Q == 0x1 ? aarch64::q0 : aarch64::d0;

                for (int it_selem = selem - 1; it_selem >= 0; it_selem--) {
                    insn_in_progress->appendOperand(
                            makeRegisterExpression(makeAarch64RegID(reg, (encoding + it_selem) % 32)), true, false);
                }
            }
            else
                insn_in_progress->appendOperand(makeRtExpr(), true, false);
        }

        Expression::Ptr InstructionDecoder_aarch64::makeRt2Expr() {
            MachRegister baseReg;
            int encoding = field<10, 14>(insn);

            if (isFPInsn) {
                baseReg = makeAarch64RegID(isSinglePrec() ? aarch64::s0 : aarch64::d0, encoding);
            }
            else if (isSIMDInsn) {
                baseReg = getLoadStoreSimdRegister(encoding);
            }
            else {
                baseReg = is64Bit ? ((encoding == 31) ? aarch64::xzr : aarch64::x0) : ((encoding == 31) ? aarch64::wzr
                                                                                                   : aarch64::w0);
                if (encoding != 31)
                    baseReg = makeAarch64RegID(baseReg, encoding);
            }

            return makeRegisterExpression(baseReg);
        }

        void InstructionDecoder_aarch64::OPRRt2() {
            assert(0);
        }

        void InstructionDecoder_aarch64::OPRRt2L() {
            insn_in_progress->appendOperand(makeRt2Expr(), false, true);
        }

        void InstructionDecoder_aarch64::OPRRt2S() {
            insn_in_progress->appendOperand(makeRt2Expr(), true, false);
        }

        template<unsigned int endBit, unsigned int startBit>
        void InstructionDecoder_aarch64::OPRcond() {
            int condVal = field<startBit, endBit>(insn);
            if (IS_INSN_B_COND(insn)) {
                insn_in_progress->getOperation().mnemonic += ".";
                insn_in_progress->getOperation().mnemonic += condNames[condVal];
            }
            else {
		if(IS_INSN_COND_SELECT(insn))
		    fix_condinsn_alias_and_cond(condVal);
		else
		    oprRotateAmt++;

                Expression::Ptr cond = ArmConditionImmediate::makeArmConditionImmediate(Result(u8, condVal));
                insn_in_progress->appendOperand(cond, true, false);
            }

            isPstateRead = true;
        }

        void InstructionDecoder_aarch64::OPRnzcv() {
            if(!isFPInsn && field<4, 4>(insn) == 1) {
                isValid = false;
            } else {
                unsigned int nzcvVal = field<0, 3>(insn);
                Expression::Ptr nzcv = Immediate::makeImmediate(Result(u8, nzcvVal));
                insn_in_progress->appendOperand(nzcv, true, false);

                isPstateWritten = true;
                oprRotateAmt++;
            }
        }

        void InstructionDecoder_aarch64::OPRop1() {
            op1Field = field<16, 18>(insn);
        }

        void InstructionDecoder_aarch64::OPRop2() {
            op2Field = field<5, 7>(insn);
        }

        void InstructionDecoder_aarch64::OPRCRm() {
            crmField = field<8, 11>(insn);
        }

        void InstructionDecoder_aarch64::OPRCRn() {
        }

        template<unsigned int endBit, unsigned int startBit>
        void InstructionDecoder_aarch64::OPRS() {
            sField = field<startBit, endBit>(insn);
        }

	void InstructionDecoder_aarch64::OPRopc() {
	    int opcVal = field<30, 31>(insn);
	    int lopc = (field<22, 22>(insn) << 1) | (opcVal & 0x1);

	    if((IS_INSN_LDST_PAIR_NOALLOC(insn) && !isSIMDInsn && (opcVal & 0x1) == 0x1) ||
	       (IS_INSN_LDST_PAIR(insn) && (opcVal == 0x3 || lopc == 0x1)))
		isValid = false;
	}

        void InstructionDecoder_aarch64::OPRscale() {
            int scaleVal = field<10, 15>(insn);

	    if(!is64Bit && ((scaleVal >> 0x5) & 0x1) == 0x0)
		isValid = false;
	    else {
		Expression::Ptr scale = Immediate::makeImmediate(Result(u32, unsign_extend32(6 + is64Bit, 64 - scaleVal)));
		insn_in_progress->appendOperand(scale, true, false);
	    }
        }

        Expression::Ptr InstructionDecoder_aarch64::makeRaExpr() {
            int encoding = field<10, 14>(insn);
            MachRegister reg;

            if (isFPInsn) {
                reg = makeAarch64RegID(isSinglePrec() ? aarch64::s0 : aarch64::d0, encoding);
            }
            else {
                reg = is64Bit ? ((encoding == 31) ? aarch64::xzr : aarch64::x0) : ((encoding == 31) ? aarch64::wzr
                                                                                                   : aarch64::w0);
                if (encoding != 31)
                    reg = makeAarch64RegID(reg, encoding);
            }

            return makeRegisterExpression(reg);

        }

        void InstructionDecoder_aarch64::OPRRa() {
            insn_in_progress->appendOperand(makeRaExpr(), true, false);

            oprRotateAmt++;
        }

        void InstructionDecoder_aarch64::OPRo0() {

        }

        void InstructionDecoder_aarch64::OPRb5() {
            OPRsf();
            hasb5 = true;
        }

        void InstructionDecoder_aarch64::OPRb40() {

        }

        Expression::Ptr InstructionDecoder_aarch64::makeb40Expr() {
            int b40Val = field<19, 23>(insn);
            int bitpos = ((is64Bit ? 1 : 0) << 5) | b40Val;

            return Immediate::makeImmediate(Result(u32, unsign_extend32(6, bitpos)));
        }

        template<unsigned int endBit, unsigned int startBit>
        void InstructionDecoder_aarch64::OPRsz() {
            _szField = field<startBit, endBit>(insn);

	    if(IS_INSN_SIMD_VEC_INDEX(insn)) {
		int L = field<21, 21>(insn);

		if(_szField == 0x1 && (L == 0x1 || _Q == 0))
		    isValid = false;
	    }
        }

        bool InstructionDecoder_aarch64::isSinglePrec() {
            if (isFPInsn && !isSIMDInsn) {
                if (_typeField == -1) {
                    //TODO if the type field is not set, do sth else
                    OPRtype<23, 22>();
                }
		if(_typeField > 1)
		    isValid = false;

		//return false even if type has an invalid value, it won't matter because isValid is set to false and thus the instruction will be marked invalid
                return _typeField == 0 ? true : false;
            } else if (isSIMDInsn) {
                isValid = false; //not implemeted yet
            }
            return false;
        }

        Expression::Ptr InstructionDecoder_aarch64::makeRsExpr() {
            MachRegister baseReg = isFPInsn ?
                                   (isSinglePrec() ? aarch64::s0 : aarch64::d0) :
                                   (is64Bit ? aarch64::x0 : aarch64::w0);

            if (IS_INSN_LDST(insn)) {
                baseReg = aarch64::w0;
            }
            return makeRegisterExpression(makeAarch64RegID(baseReg, field<16, 20>(insn)));
        }

        void InstructionDecoder_aarch64::OPRRs() {
            insn_in_progress->appendOperand(makeRsExpr(), false, true);
        }

        void InstructionDecoder_aarch64::makeBranchTarget(bool branchIsCall, bool bIsConditional, int immVal,
                                                          int immLen) {
            Expression::Ptr lhs = makePCExpr();

            int64_t offset = sign_extend64(immLen + 2, immVal * 4);
            Expression::Ptr rhs = Immediate::makeImmediate(Result(s64, offset));

            insn_in_progress->addSuccessor(makeAddExpression(lhs, rhs, s64), branchIsCall, false, bIsConditional,
                                           false);
            if (branchIsCall) {
                insn_in_progress->addSuccessor(makeFallThroughExpr(), false, false, false, true);
            }

        }

        Expression::Ptr InstructionDecoder_aarch64::makeFallThroughExpr() {
            return makeAddExpression(makePCExpr(), Immediate::makeImmediate(Result(u64, unsign_extend64(3, 4))), u64);
        }

        template<typename T, Result_Type rT>
        Expression::Ptr InstructionDecoder_aarch64::fpExpand(int val) {
            int N, E, F;
            T frac, expandedImm, sign, exp;

            N = (rT == s32) ? 32 : 64;
            E = (N == 32) ? 8 : 11;
            F = N - E - 1;

            sign = (val & 0x80) >> 7;

            int val6 = ((~val) & 0x40) >> 6, val6mask = (1 << (E - 3)) - 1;
            exp = (val6 << (E - 1)) | ((val6 ? val6mask : 0) << 2) | ((val & 0x30) >> 4);

            frac = (val & 0xF) << (F - 4);

            expandedImm = (sign << (E + F)) | (exp << F) | frac;

            return Immediate::makeImmediate(Result(rT, expandedImm));
        }

        template<typename T>
        Expression::Ptr InstructionDecoder_aarch64::makeLogicalImm(int immr_, int imms, int immsLen, Result_Type rT) {
            int len = highest_set_bit((nField << immsLen) | (~imms & ((1 << immsLen) - 1))) - 1;
            int finalsize = (rT == u32 ? 32 : 64);

            if (len < 1 || ((1 << len) > finalsize)) {
                isValid = false;
                return Immediate::makeImmediate(Result(u32, 0));
            }
            int levels = (1 << len) - 1;

            int S = imms & levels;
            if (S == levels) {
                isValid = false;
                return Immediate::makeImmediate(Result(u32, 0));
            }
            int R = immr_ & levels;

            int esize = 1 << len;
            T welem = (((T) 1) << (S + 1)) - 1;

            T wmaskarg = welem;
            if (R != 0) {
                T low = welem & (((T) 1 << R) - 1), high = welem & ((((T) 1 << (esize - R)) - 1) << R);
                wmaskarg = (low << (esize - R)) | (high >> R);
            }

            int idx;
            T wmask = wmaskarg;

            for (idx = 1; idx < finalsize / esize; idx++) {
                wmask |= (wmaskarg << (esize * idx));
            }

            return Immediate::makeImmediate(Result(rT, wmask));
        }

        bool InstructionDecoder_aarch64::fix_bitfieldinsn_alias(int immr_, int imms) {
            entryID modifiedID = insn_in_progress->getOperation().operationID;
            bool do_further_processing = true;

            switch (field<27, 30>(insn)) {
                case 0x6:
                    modifiedID = (imms < immr_) ? aarch64_op_bfi_bfm : aarch64_op_bfxil_bfm;
                    break;
                case 0x2:
                    if (immr_ == 0 && (imms == 7 || imms == 15 || imms == 31)) {
                        do_further_processing = false;
                        switch (imms) {
                            case 7:
                                modifiedID = aarch64_op_sxtb_sbfm;
                                break;
                            case 15:
                                modifiedID = aarch64_op_sxth_sbfm;
                                break;
                            case 31:
                                modifiedID = aarch64_op_sxtw_sbfm;
                                break;
                        }
                    }
                    else
                        modifiedID = (imms < immr) ? aarch64_op_sbfiz_sbfm : aarch64_op_sbfx_sbfm;
                    break;
                case 0xA:
                    if (immr == 0 && (imms == 7 || imms == 15)) {
                        do_further_processing = false;
                        switch (imms) {
                            case 7:
                                modifiedID = aarch64_op_uxtb_ubfm;
                                break;
                            case 15:
                                modifiedID = aarch64_op_uxth_ubfm;
                                break;
                        }
                    }
		    else if((imms + 1) == immr)
			modifiedID = aarch64_op_lsl_ubfm;
		    else if((imms & 0x1F) == 0x1F)
			modifiedID = aarch64_op_lsr_ubfm;
                    else
                        modifiedID = (imms < immr) ? aarch64_op_ubfiz_ubfm : aarch64_op_ubfx_ubfm;
                    break;
                default:
                    isValid = false;
            }

            insn_in_progress->getOperation().operationID = modifiedID;
            insn_in_progress->getOperation().mnemonic = bitfieldInsnAliasMap(modifiedID);

            return do_further_processing;
        }

        void InstructionDecoder_aarch64::fix_condinsn_alias_and_cond(int &cond) {
            entryID modifiedID = insn_in_progress->getOperation().operationID;
            if (modifiedID == aarch64_op_csel)
                return;

            int Rm = field<16, 20>(insn), Rn = field<5, 9>(insn);

            if (Rm == Rn && (cond & 0xE) != 0xE && modifiedID == aarch64_op_csneg) {
                modifiedID = aarch64_op_cneg_csneg;
		skipRm = true;
            } else if (Rm != 31 && Rn != 31 && (cond & 0xE) != 0xE && Rm == Rn) {
                switch (modifiedID) {
                    case aarch64_op_csinc:
                        modifiedID = aarch64_op_cinc_csinc;
                        break;
                    case aarch64_op_csinv:
                        modifiedID = aarch64_op_cinv_csinv;
                        break;
                    default:
                        isValid = false;
                }

		skipRm = true;
            } else if (Rm == 31 && Rn == 31 && (cond & 0xE) != 0xE) {
                switch (modifiedID) {
                    case aarch64_op_csinc:
                        modifiedID = aarch64_op_cset_csinc;
                        break;
                    case aarch64_op_csinv:
                        modifiedID = aarch64_op_csetm_csinv;
                        break;
                    default:
                        isValid = false;
                }

		skipRn = skipRm = true;
            }

            insn_in_progress->getOperation().operationID = modifiedID;
            insn_in_progress->getOperation().mnemonic = condInsnAliasMap(modifiedID);
	    if(skipRm)
		cond = ((cond % 2) == 0) ? (cond + 1) : (cond - 1);
        }
	
	void InstructionDecoder_aarch64::modify_mnemonic_simd_upperhalf_insns() {
        if (field<30, 30>(insn) != 1 || field<28, 28>(insn) != 0)
            return;

        string cur_mnemonic = insn_in_progress->getOperation().mnemonic;
        bool add2 = false;

        if (IS_INSN_SIMD_3DIFF(insn) || IS_INSN_SCALAR_3DIFF(insn))
            add2 = true;
        else if (IS_INSN_SCALAR_2REG_MISC(insn) || IS_INSN_SIMD_2REG_MISC(insn)) {
            int checkval = (field<12, 16>(insn) >> 2) & 0x7;
            if (checkval == 0x4 || checkval == 0x5)
                add2 = true;
        } else if (IS_INSN_SIMD_SHIFT_IMM(insn) || IS_INSN_SCALAR_SHIFT_IMM(insn)) {
            int checkval = (field<11, 15>(insn) >> 2) & 0x7;
            if (checkval == 0x4 || (IS_INSN_SIMD_SHIFT_IMM(insn) && checkval == 0x5))
                add2 = true;
        } else if (IS_INSN_SIMD_VEC_INDEX(insn) || IS_INSN_SCALAR_INDEX(insn)) {
            int checkval = field<12, 15>(insn) & 0x3;
            if ((checkval >> 1) == 0x1 || (IS_INSN_SCALAR_INDEX(insn) && (checkval & 0x1) == 0x1))
                add2 = true;
        }

        if (add2)
            insn_in_progress->getOperation().mnemonic = cur_mnemonic + "2";
	}

        template<unsigned int endBit, unsigned int startBit>
        void InstructionDecoder_aarch64::OPRimm() {
            int immVal = field<startBit, endBit>(insn);
            unsigned int immLen = endBit - startBit + 1;

            if (IS_INSN_LDST(insn)) {
                if (IS_INSN_LD_LITERAL(insn)) {
                    Expression::Ptr literal = makeMemRefIndexLiteral();
                    insn_in_progress->appendOperand(literal, true, false);
                }
                else if (IS_INSN_LDST_POST(insn)) {
                    Expression::Ptr offset = makeMemRefIndex_offset9();
                    insn_in_progress->appendOperand(offset, true, false);
                }
                else if (IS_INSN_LDST_PAIR_POST(insn)) {
                    Expression::Ptr offset = makeMemRefPair_offset7();
                    insn_in_progress->appendOperand(offset, true, false);
                }

                return;
            }

            if (hasHw) {
                processHwFieldInsn(immLen, immVal);
            }
            else if (hasN)        //logical (immediate), bitfield, extract
            {
                if (IS_FIELD_IMMR(startBit, endBit)) {
                    immr = immVal;
                    immrLen = immLen;
                }
                else if (IS_FIELD_IMMS(startBit, endBit)) {
                    Expression::Ptr imm;
		    bool isLsrLsl = false;

                    if (IS_INSN_LOGICAL_IMM(insn)) {
                        if (is64Bit)
                            imm = makeLogicalImm<uint64_t>(immr, immVal, immLen, u64);
                        else
                            imm = makeLogicalImm<uint32_t>(immr, immVal, immLen, u32);
			
			insn_in_progress->appendOperand(imm, true, false);
                    }
                    else {
			entryID curID;

                        if (IS_INSN_BITFIELD(insn)) {
                            if((is64Bit && nField != 0x1) ||
                               (!is64Bit && (nField != 0 || (immr & 0x20) != 0 || (immVal & 0x20) != 0))) {
                                isValid = false;
                                return;
                            }

                            if (!fix_bitfieldinsn_alias(immr, immVal))
                                return;

			    curID = insn_in_progress->getOperation().operationID;
			    if(curID == aarch64_op_lsl_ubfm || curID == aarch64_op_lsr_ubfm)
				isLsrLsl = true;
                            
			    if (curID != aarch64_op_lsr_ubfm && immVal < immr) {
                                int divisor = is64Bit ? 64 : 32, factor = 0;
                                while ((divisor * factor) < immr)
                                    factor += 1;
                                immr = (divisor * factor) - immr;
                                immrLen += 1;

				immVal += 1;
                            }
                            else
                                immVal = immVal + 1 - immr;

                            immLen += 1;
                        }

			if(!isLsrLsl) {
			    imm = Immediate::makeImmediate(Result(u32, unsign_extend32(immLen, immVal)));
			    insn_in_progress->appendOperand(imm, true, false);
			    oprRotateAmt++;
			}
                    }

                    if (IS_INSN_BITFIELD(insn)) {
                        imm = Immediate::makeImmediate(Result(u32, unsign_extend32(immrLen, immr)));
                        insn_in_progress->appendOperand(imm, true, false);
			if(!isLsrLsl)
			    oprRotateAmt--;
                    }
                }
                else
                    isValid = false;
            }
            else if (hasShift) {
                if (IS_INSN_ADDSUB_SHIFT(insn) || IS_INSN_LOGICAL_SHIFT(insn))    //add-sub shifted | logical shifted
                {
                    if(IS_INSN_LOGICAL_SHIFT(insn) && shiftField == 0 && field<5, 9>(insn) == 0x1F && immVal == 0) {
                        insn_in_progress->getOperation().operationID = aarch64_op_mov_orr_log_shift;
                        insn_in_progress->getOperation().mnemonic = "mov";
                        skipRn = true;

                        insn_in_progress->appendOperand(makeRmExpr(), true, false);
                    } else {
                        processShiftFieldShiftedInsn(immLen, immVal);
                        
                        if((IS_INSN_ADDSUB_SHIFT(insn) && shiftField == 0x3) || (!is64Bit && ((immVal >> 5) & 0x1) == 0x1))
                            isValid = false;
                    }
                }
                else if (IS_INSN_ADDSUB_IMM(insn))        //add-sub (immediate)
                {
                    processShiftFieldImmInsn(immLen, immVal);
                }
                else
                    isValid = false;
            }
            else if (hasOption) {
                if (IS_INSN_ADDSUB_EXT(insn))                                        //add-sub extended
                {
                    if(immVal > 4 || ((insn >> 21) & 0x7) != 0x1) {
                        isValid = false;
                        return;
                    }
                    Expression::Ptr expr = makeOptionExpression(immLen, immVal);

                    insn_in_progress->appendOperand(expr, true, false);
                }
                else {
                    isValid = false;
                }
            }
            else if (IS_INSN_BRANCHING(insn) && !IS_INSN_B_UNCOND_REG(
                    insn)) {        //unconditional branch (immediate), test and branch, compare and branch, conditional branch
                bool bIsConditional = false;
                if (!(IS_INSN_B_UNCOND(insn)))
                    bIsConditional = true;

                bool branchIsCall = bIsConditional ? false : (field<31, 31>(insn) == 1);

                insn_in_progress->appendOperand(makePCExpr(), false, true, true);
                makeBranchTarget(branchIsCall, bIsConditional, immVal, immLen);

                if (hasb5)
                    insn_in_progress->appendOperand(makeb40Expr(), true, false);

                if (bIsConditional)
                    insn_in_progress->addSuccessor(makeFallThroughExpr(), false, false, true, true);
            }
            else if (IS_INSN_PCREL_ADDR(insn))                                    //pc-relative addressing
            {
                if (IS_FIELD_IMMLO(startBit, endBit)) {
                    immlo = immVal;
                    immloLen = endBit - startBit + 1;
                }
                else if (IS_FIELD_IMMHI(startBit, endBit)) {
                    int page = field<31, 31>(insn);
                    int64_t offset = (immVal << immloLen) | immlo;
                    offset = offset << (page * 12);
                    int size_ = immloLen + immLen + (page * 12);

                    //insn_in_progress->appendOperand(makePCExpr(), true, false);
                    Expression::Ptr imm = Immediate::makeImmediate(Result(s64, (offset << (64 - size_)) >> (64 - size_)));

                    insn_in_progress->appendOperand(makeAddExpression(makePCExpr(), imm, u64), true, false);
                }
                else
                    isValid = false;
            }
            else if (isFPInsn) {
                if (isSinglePrec())
                    insn_in_progress->appendOperand(fpExpand<int32_t, s32>(immVal), true, false);
                else
                    insn_in_progress->appendOperand(fpExpand<int64_t, s64>(immVal), true, false);
            }
            else if (IS_INSN_EXCEPTION(insn)) {
                Expression::Ptr imm = Immediate::makeImmediate(Result(u16, immVal));
                insn_in_progress->appendOperand(imm, true, false);
                isPstateRead = true;
            }
            else if (isSIMDInsn) {
                if (IS_INSN_SIMD_EXTR(insn)) {
                    if (_Q == 0) {
                        if ((immVal & 0x8) == 0) {
                            Expression::Ptr imm = Immediate::makeImmediate(
                                    Result(u32, unsign_extend32(immLen - 1, immVal & 0x7)));
                            insn_in_progress->appendOperand(imm, true, false);
                            oprRotateAmt++;
                        }
                        else
                            isValid = false;
                    }
                    else {
                        Expression::Ptr imm = Immediate::makeImmediate(Result(u32, unsign_extend32(immLen, immVal)));
            insn_in_progress->appendOperand(imm, true, false);
                    }
                }
                else if (IS_INSN_SIMD_SHIFT_IMM(insn) || IS_INSN_SCALAR_SHIFT_IMM(insn)) {
                    //immh
                    if (startBit == 19 && endBit == 22) {
                        immlo = immVal;
                        immloLen = endBit - startBit + 1;

			int immh_3 = (immlo >> 3) & 0x1, opcode = field<11, 15>(insn);

			if(IS_INSN_SIMD_SHIFT_IMM(insn)) {
			    if(((opcode >> 3) & 0x3) == 0x3 && ((immlo >> 2) & 0x3) == 0)
				isValid = false;

			    if(immh_3 == 1) {
				if(((opcode >> 4) & 0x1) == 0x1 || _Q == 0)
				    isValid = false;
			    }
			} else {
			   if(((opcode >> 2) & 0x7) == 0x3) {
			       if(immlo == 0)
				   isValid = false;
			   } else if(((opcode >> 2) & 0x7) == 0x4) {
			       if(immlo == 0 || immh_3 == 1)
				   isValid = false;
			   } else if(((opcode >> 3) & 0x3) == 0x3) {
			       if(((immlo >> 2) & 0x3) == 0)
				   isValid = false;
			   } else {
			       if(immh_3 != 1)
				   isValid = false;
			   }
			}
                    }
                        //immb
                    else if (startBit == 16 && endBit == 18) {
                        int opcode = field<11, 15>(insn);
                        int shift = 0, isRightShift = 1, elemWidth = (immlo << immLen) | immVal;
                        entryID insnID = insn_in_progress->getOperation().operationID;
                        bool isScalar = field<28, 28>(insn) ? true : false;

                        //check if shift is left; if it is, the immediate has to be processed in a different manner.
                        //unfortunately, determining whether the instruction will do a left or right shift cannot be determined in any way other than checking the instruction's opcode
                        if (insnID == aarch64_op_shl_advsimd || insnID == aarch64_op_sqshl_advsimd_imm ||
                            insnID == aarch64_op_sshll_advsimd ||
                            insnID == aarch64_op_sli_advsimd || insnID == aarch64_op_sqshlu_advsimd ||
                            insnID == aarch64_op_uqshl_advsimd_imm || insnID == aarch64_op_ushll_advsimd)
                            isRightShift = -1;

                        switch (highest_set_bit(immlo)) {
                            case 0x1:
                                (!isScalar || (opcode & 0x1C) == 0x0C || (opcode & 0x1C) == 0x10)
                                ? (shift = isRightShift * (16 - elemWidth) + (isRightShift > 0 ? 0 : 8))
                                : (isValid = false);
                                break;
                            case 0x2:
                                (!isScalar || (opcode & 0x1C) == 0x0C || (opcode & 0x1C) == 0x10)
                                ? (shift = isRightShift * (32 - elemWidth) + (isRightShift > 0 ? 0 : 16))
                                : (isValid = false);
                                break;
                            case 0x3:
                                (!isScalar || opcode > 0x0A) ? (shift = isRightShift * (64 - elemWidth) +
                                                                        (isRightShift > 0 ? 0 : 32))
                                                             : (isValid = false);
                                break;
                            case 0x4:
                                shift = isRightShift * (128 - elemWidth) + (isRightShift > 0 ? 0 : 64);
                                break;
                            default:
                                isValid = false;
                        }

                        if (isValid) {
                            Expression::Ptr imm = Immediate::makeImmediate(
                                    Result(u32, unsign_extend32(immloLen + immLen, shift)));
                            insn_in_progress->appendOperand(imm, true, false);
                        }
                    }
                    else
                        isValid = false;
                }
            }
            else                                                            //conditional compare (immediate)
            {
                Result_Type rT = is64Bit ? u64 : u32;

                Expression::Ptr imm = Immediate::makeImmediate(
                        Result(rT, rT == u32 ? unsign_extend32(immLen, immVal) : unsign_extend64(immLen, immVal)));
                insn_in_progress->appendOperand(imm, true, false);
            }
        }

        void InstructionDecoder_aarch64::reorderOperands() {
            if (oprRotateAmt) {
                std::vector<Operand> curOperands;
                insn_in_progress->getOperands(curOperands);

                if (curOperands.empty())
                    assert(!"empty operand list found while re-ordering operands");

                std::swap(curOperands[1], curOperands[3]);

                while (oprRotateAmt--)
                    std::rotate(curOperands.begin(), curOperands.begin() + 1, curOperands.begin() + 3);

                insn_in_progress->m_Operands.assign(curOperands.begin(), curOperands.end());
            }
            else if (IS_INSN_LDST_POST(insn) || IS_INSN_LDST_PAIR_POST(insn)) {
                std::vector<Operand> curOperands;
                insn_in_progress->getOperands(curOperands);
                std::iter_swap(curOperands.begin(), curOperands.end() - 1);
                insn_in_progress->m_Operands.assign(curOperands.begin(), curOperands.end());
            }
            else if (IS_INSN_LDST_PAIR(insn)) {
                std::vector<Operand> curOperands;
                insn_in_progress->getOperands(curOperands);
                assert(curOperands.size() == 4 || curOperands.size() == 3);
                if (curOperands.size() == 3) {
                    curOperands.insert(curOperands.begin(), curOperands.back());
                    curOperands.pop_back();
                } else if (curOperands.size() == 4) {
                    std::iter_swap(curOperands.begin(), curOperands.end() - 1);
                }
                insn_in_progress->m_Operands.assign(curOperands.begin(), curOperands.end());
            }
            else if (IS_INSN_LDST_EX_PAIR(insn)) {
                std::vector<Operand> curOperands;
                insn_in_progress->getOperands(curOperands);
                if (curOperands.size() == 3) {
                    curOperands.insert(curOperands.begin(), curOperands.back());
                    curOperands.pop_back();
                }
                else if (curOperands.size() == 4) {
                    curOperands.insert(curOperands.begin() + 1, curOperands.back());
                    curOperands.pop_back();
                }
                insn_in_progress->m_Operands.assign(curOperands.begin(), curOperands.end());
            }
            else if (IS_INSN_ST_EX(insn)) {
                std::vector<Operand> curOperands;
                insn_in_progress->getOperands(curOperands);
                if (curOperands.size() == 3) {
                    curOperands.insert(curOperands.begin() + 1, curOperands.back());
                    curOperands.pop_back();
                    insn_in_progress->m_Operands.assign(curOperands.begin(), curOperands.end());
                }
                else
                    insn_in_progress->m_Operands.reverse();
            }
            else
                insn_in_progress->m_Operands.reverse();
        }

        void InstructionDecoder_aarch64::processAlphabetImm() {
            if (op == 1 && cmode == 0xE) {
                uint64_t imm = 0;

                for (int imm_index = 0; imm_index < 8; imm_index++)
                    imm |= (simdAlphabetImm & (1 << imm_index)) ? (0xFFULL << (imm_index * 8)) : 0;

                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u64, imm)), true, false);
            }
            else if (cmode == 0xF) {
                //fmov (vector, immediate)
                //TODO: check with Bill if this is fine
                insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u8, simdAlphabetImm)), true, false);
            }
            else {
                int shiftAmt = 0;

                //16-bit shifted immediate
                if ((cmode & 0xC) == 0x8)
                    shiftAmt = ((cmode & 0x2) >> 1) * 8;
                    //32-bit shifted immediate
                else if ((cmode & 0x8) == 0x0)
                    shiftAmt = ((cmode & 0x6) >> 1) * 8;
                    //32-bit shifting ones
                else if ((cmode & 0xE) == 0xC)
                    shiftAmt = ((cmode & 0x0) + 1) * 8;

                Expression::Ptr lhs = Immediate::makeImmediate(Result(u32, unsign_extend32(8, simdAlphabetImm)));
                Expression::Ptr rhs = Immediate::makeImmediate(Result(u32, unsign_extend32(5, shiftAmt)));
                Expression::Ptr imm = makeLeftShiftExpression(lhs, rhs, u64);

                insn_in_progress->appendOperand(imm, true, false);
            }
        }

#include "aarch64_opcode_tables.C"

        void InstructionDecoder_aarch64::doDelayedDecode(const Instruction *insn_to_complete) {
            InstructionDecoder::buffer b(insn_to_complete->ptr(), insn_to_complete->size());
            //insn_to_complete->m_Operands.reserve(4);
            decode(b);
            decodeOperands(insn_in_progress.get());

            Instruction* iptr = const_cast<Instruction*>(insn_to_complete);
            *iptr = *(insn_in_progress.get());
        }

        bool InstructionDecoder_aarch64::pre_process_checks(const aarch64_insn_entry &entry) {
            bool ret = false;
            entryID insnID = entry.op;
            const string& mnemonic = entry.mnemonic;

            vector<entryID> simdCompareRegInsns = {aarch64_op_cmeq_advsimd_reg, aarch64_op_cmge_advsimd_reg, aarch64_op_cmgt_advsimd_reg, aarch64_op_cmhi_advsimd, aarch64_op_cmhs_advsimd, aarch64_op_cmtst_advsimd},
                            simdCompareZeroInsns = {aarch64_op_cmeq_advsimd_zero, aarch64_op_cmge_advsimd_zero, aarch64_op_cmgt_advsimd_zero, aarch64_op_cmle_advsimd, aarch64_op_cmlt_advsimd};
            vector<entryID> fpCompareRegInsns = {aarch64_op_fcmeq_advsimd_reg, aarch64_op_fcmge_advsimd_reg, aarch64_op_fcmgt_advsimd_reg},
                            fpCompareZeroInsns = {aarch64_op_fcmeq_advsimd_zero, aarch64_op_fcmge_advsimd_zero, aarch64_op_fcmgt_advsimd_zero, aarch64_op_fcmle_advsimd, aarch64_op_fcmlt_advsimd};

            if(insnID == aarch64_op_sqshl_advsimd_imm) {
                if(!IS_INSN_SIMD_SHIFT_IMM(insn) && !IS_INSN_SCALAR_SHIFT_IMM(insn))
                    ret = true;
                else if(((insn >> 11) & 0x1) != 0)
                    ret = true;
            } else if((find(simdCompareRegInsns.begin(), simdCompareRegInsns.end(), insnID) != simdCompareRegInsns.end() ||
                      (find(fpCompareRegInsns.begin(), fpCompareRegInsns.end(), insnID) != fpCompareRegInsns.end()))
                      && !(IS_INSN_SCALAR_3SAME(insn) || IS_INSN_SIMD_3SAME(insn))) {
                ret = true;
            } else if((find(simdCompareZeroInsns.begin(), simdCompareZeroInsns.end(), insnID) != simdCompareZeroInsns.end() ||
                       find(fpCompareZeroInsns.begin(), fpCompareZeroInsns.end(), insnID) != fpCompareZeroInsns.end() ||
                       insnID == aarch64_op_rev64_advsimd)
                      && !(IS_INSN_SIMD_2REG_MISC(insn) || IS_INSN_SCALAR_2REG_MISC(insn))) {
                ret = true;
            } else if((mnemonic.find("sha1") != string::npos || mnemonic.find("sha2") != string::npos)
                      && !(IS_INSN_CRYPT_2REG_SHA(insn) || IS_INSN_CRYPT_3REG_SHA(insn))) {
                ret = true;
            }

            return ret;
        }

        bool InstructionDecoder_aarch64::decodeOperands(const Instruction *insn_to_complete) {
            int insn_table_index = findInsnTableIndex(0);
            isValid = !pre_process_checks(aarch64_insn_entry::main_insn_table[insn_table_index]);
            const auto& insn_table_entry = isValid
                ? aarch64_insn_entry::main_insn_table[insn_table_index]
                : aarch64_insn_entry::main_insn_table[0];

            insn = insn_to_complete->m_RawInsn.small_insn;

            if (IS_INSN_LDST_REG(insn) ||
                IS_INSN_ADDSUB_EXT(insn) ||
                IS_INSN_ADDSUB_SHIFT(insn) ||
                IS_INSN_LOGICAL_SHIFT(insn))
                skipRm = true;

            for (std::size_t i = 0; i < insn_table_entry.operandCnt; i++) {
                std::mem_fn(insn_table_entry.operands[i])(this);
            }

            if (insn_table_index == 0)
                isValid = false;

            if (!isValid) {
                insn_in_progress->getOperation().mnemonic = INVALID_ENTRY.mnemonic;
                insn_in_progress->getOperation().operationID = INVALID_ENTRY.op;
                insn_in_progress->m_Operands.clear();
                insn_in_progress->m_Successors.clear();
            } else {
                reorderOperands();

                if (IS_INSN_SYSTEM(insn)) {
                    processSystemInsn();
                }

                if (IS_INSN_SIMD_MOD_IMM(insn)) {
                    processAlphabetImm();
                }

                entryID insnID = insn_in_progress->getOperation().operationID;
                vector<entryID> zeroInsnIDs = {aarch64_op_cmeq_advsimd_zero, aarch64_op_cmge_advsimd_zero, aarch64_op_cmgt_advsimd_zero, aarch64_op_cmle_advsimd, aarch64_op_cmlt_advsimd,
                                               aarch64_op_fcmeq_advsimd_zero, aarch64_op_fcmge_advsimd_zero, aarch64_op_fcmgt_advsimd_zero, aarch64_op_fcmle_advsimd, aarch64_op_fcmlt_advsimd};
                if(find(zeroInsnIDs.begin(), zeroInsnIDs.end(), insnID) != zeroInsnIDs.end())
                    insn_in_progress->appendOperand(Immediate::makeImmediate(Result(u32, 0)), true, false);

                if (IS_INSN_LDST_SIMD_MULT_POST(insn) || IS_INSN_LDST_SIMD_SING_POST(insn))
                    insn_in_progress->appendOperand(makeRnExpr(), false, true, true);

                if (isPstateWritten || isPstateRead)
                    insn_in_progress->appendOperand(makePstateExpr(), isPstateRead, isPstateWritten, true);
            }

            return true;
        }


        int InstructionDecoder_aarch64::findInsnTableIndex(unsigned int decoder_table_index) {
            const auto& cur_entry = aarch64_mask_entry::main_decoder_table[decoder_table_index];
            unsigned int cur_mask = cur_entry.mask;

            if (cur_mask == 0) {
                int insn_table_index = cur_entry.insnTableIndex;
                if (insn_table_index == -1)
                    assert(!"no instruction table entry found for current instruction");
                else
                    return insn_table_index;
            }

            unsigned int insn_iter_index = 0, map_key_index = 0, branch_map_key = 0;

            while (insn_iter_index < AARCH64_INSN_LENGTH) {
                if (((cur_mask >> insn_iter_index) & 1) == 1) {
                    branch_map_key = branch_map_key | (((insn >> insn_iter_index) & 1) << map_key_index);
                    map_key_index++;
                }
                insn_iter_index++;
            }

            const auto& cur_branches = cur_entry.nodeBranches;
            for (std::size_t i = 0; i < cur_entry.branchCnt; i++)
                if (cur_branches[i].first == branch_map_key)
                    return findInsnTableIndex(cur_branches[i].second);
            return 0;
        }

        void InstructionDecoder_aarch64::setFlags() {
            isPstateWritten = true;
        }

        void InstructionDecoder_aarch64::mainDecode() {
            int insn_table_index = findInsnTableIndex(0);
            const auto& insn_table_entry = aarch64_insn_entry::main_insn_table[insn_table_index];

            insn_in_progress = makeInstruction(insn_table_entry.op, insn_table_entry.mnemonic, 4,
                                               reinterpret_cast<unsigned char *>(&insn));

            modify_mnemonic_simd_upperhalf_insns();

            if (IS_INSN_BRANCHING(insn)) {
                decodeOperands(insn_in_progress.get());
            }

            insn_in_progress->arch_decoded_from = Arch_aarch64;
            if (insn_table_entry.operands[0] != nullptr) {
                insn_in_progress->m_InsnOp.isVectorInsn =
                    (insn_table_entry.operands[0] == &InstructionDecoder_aarch64::setSIMDMode);
            }
            return;
        }
    }
}



