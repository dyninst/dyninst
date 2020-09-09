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

#include "Ternary.h"
#include "InstructionDecoder-amdgpu.h"

namespace Dyninst {
    namespace InstructionAPI {
        typedef void (InstructionDecoder_amdgpu::*operandFactory)();

        typedef amdgpu_insn_entry amdgpu_insn_table[];
        typedef amdgpu_mask_entry amdgpu_decoder_table[];

        const std::array<std::string, 16> InstructionDecoder_amdgpu::condNames = {
            "eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc", "hi", "ls", "ge",
            "lt", "gt", "le", "al", "nv",
        };

        const char* InstructionDecoder_amdgpu::bitfieldInsnAliasMap(entryID e) {
            switch(e) {
                default: assert(!"no alias for entryID");
            };
        }
        const char* InstructionDecoder_amdgpu::condInsnAliasMap(entryID e) {
            switch(e) {
                default: assert(!"no alias for entryID");
            };
        };

#include "amdgpu_insn_entry.h"
        /*struct amdgpu_insn_entry {
            entryID op;
            const char *mnemonic;
            std::size_t operandCnt;
            const operandFactory* operands;

            static const amdgpu_insn_table main_insn_table;
            static const amdgpu_insn_table smem_insn_table;
            static const operandFactory operandTable[];
        };*/

        struct amdgpu_mask_entry {
            unsigned int mask;
            std::size_t branchCnt;
            const std::pair<unsigned int,unsigned int>* nodeBranches;
            int insnTableIndex;

            static const amdgpu_decoder_table main_decoder_table;
            static const std::pair<unsigned int,unsigned int> branchTable[];
        };

#include "amdgpu_opcode_tables.C"

        InstructionDecoder_amdgpu::InstructionDecoder_amdgpu(Architecture a)
            : InstructionDecoderImpl(a), isPstateRead(false), isPstateWritten(false), isFPInsn(false),
            isSIMDInsn(false), skipRn(false), skipRm(false),
            is64Bit(true), isValid(true), insn(0), insn_in_progress(NULL),
            hasHw(false), hasShift(false), hasOption(false), hasN(false),
            immr(0), immrLen(0), sField(0), nField(0), nLen(0),
            immlo(0), immloLen(0), _szField(-1), size(-1),
            cmode(0), op(0), simdAlphabetImm(0), _Q(1),insn_size(0) {
            }

        InstructionDecoder_amdgpu::~InstructionDecoder_amdgpu() {
        }

        void InstructionDecoder_amdgpu::decodeOpcode(InstructionDecoder::buffer &b) {
            b.start += 4;
        }

        using namespace std;
        /* replace this function with a more generic function, which is setRegWidth
           void InstructionDecoder_amdgpu::set32Mode()
           {
        // NOTE: is64Bit is set by default.
        is64Bit = false;
        }
        */
        void InstructionDecoder_amdgpu::NOTHING() {
        }

        void InstructionDecoder_amdgpu::setFPMode() {
        }

        //TODO: consistency issue
        void InstructionDecoder_amdgpu::setSIMDMode() {
        }

        template<unsigned int endBit, unsigned int startBit>
            void InstructionDecoder_amdgpu::OPRtype() {
            }

        void InstructionDecoder_amdgpu::processHwFieldInsn(int len, int val) {
        }

        void InstructionDecoder_amdgpu::processShiftFieldShiftedInsn(int len, int val) {
        }

        void InstructionDecoder_amdgpu::processShiftFieldImmInsn(int len, int val) {
        }

        void InstructionDecoder_amdgpu::processOptionFieldLSRegOffsetInsn() {
        }

        void InstructionDecoder_amdgpu::processSystemInsn() {
        }

        Result_Type InstructionDecoder_amdgpu::makeSizeType(unsigned int) {
            assert(0); //not implemented
            return u32;
        }

        // ****************
        // decoding opcodes
        // ****************

        MachRegister InstructionDecoder_amdgpu::makeAmdgpuRegID(MachRegister base, unsigned int encoding , unsigned int len) {
            MachRegister realBase = base;
            if (base == amdgpu::sgpr0){
                switch(len){
                    case 2:
                        realBase = amdgpu::sgpr_vec2_0;
                        break;
                    case 4:
                        realBase = amdgpu::sgpr_vec4_0;
                        break;
                    case 8:
                        realBase = amdgpu::sgpr_vec8_0;
                        break;
                    case 16:
                        realBase = amdgpu::sgpr_vec16_0;
                        break;
                }
            }else if (base == amdgpu::vgpr0){
                switch(len){
                    case 2:
                        realBase = amdgpu::vgpr_vec2_0;
                        break;
                    case 4:
                        realBase = amdgpu::vgpr_vec4_0;
                        break;
                    case 8:
                        realBase = amdgpu::vgpr_vec8_0;
                        break;
                    case 16:
                        realBase = amdgpu::vgpr_vec16_0;
                        break;
                }

            }
            return MachRegister(realBase.val() + encoding);

        }

        template<unsigned int endBit, unsigned int startBit>
            void InstructionDecoder_amdgpu::OPRsize() {
                size = field<startBit, endBit>(insn);

            }

        Expression::Ptr InstructionDecoder_amdgpu::makeRdExpr() {
            int encoding = field<0, 4>(insn);
            MachRegister reg;

            return makeRegisterExpression(reg);
        }

        void InstructionDecoder_amdgpu::OPRRd() {
        }

        void InstructionDecoder_amdgpu::OPRcmode() {
        }

        void InstructionDecoder_amdgpu::OPRop() {
        }

        void InstructionDecoder_amdgpu::OPRa() {
        }

        void InstructionDecoder_amdgpu::OPRb() {
        }

        void InstructionDecoder_amdgpu::OPRc() {
        }

        void InstructionDecoder_amdgpu::OPRd() {
        }

        void InstructionDecoder_amdgpu::OPRe() {
        }

        void InstructionDecoder_amdgpu::OPRf() {
            simdAlphabetImm |= (simdAlphabetImm & 0xFB) | (field<7, 7>(insn) << 2);
        }

        void InstructionDecoder_amdgpu::OPRg() {
            simdAlphabetImm |= (simdAlphabetImm & 0xFD) | (field<6, 6>(insn) << 1);
        }

        void InstructionDecoder_amdgpu::OPRh() {
            simdAlphabetImm |= (simdAlphabetImm & 0xFE) | (field<5, 5>(insn));
        }

        void InstructionDecoder_amdgpu::OPRlen() {
            //reuse immlo
        }

        Expression::Ptr InstructionDecoder_amdgpu::makeRnExpr() {

            MachRegister reg;// = amdgpu::pc;
            return makeRegisterExpression(reg);
        }

        Expression::Ptr InstructionDecoder_amdgpu::makePCExpr() {
            MachRegister baseReg;// = amdgpu::pc;

            return makeRegisterExpression(makeAmdgpuRegID(baseReg, 0));
        }

        Expression::Ptr InstructionDecoder_amdgpu::makePstateExpr() {
            MachRegister baseReg ;//= amdgpu::pstate;

            return makeRegisterExpression(makeAmdgpuRegID(baseReg, 0));
        }


        void InstructionDecoder_amdgpu::getMemRefIndexLiteral_OffsetLen(int &immVal, int &immLen) {
            immVal = field<5, 23>(insn);
            immVal = immVal << 2; //immVal:00
            immLen = (23 - 5 + 1) + 2;
            return;
        }

        void InstructionDecoder_amdgpu::getMemRefIndexLiteral_RT(Result_Type &rt) {
            int size = field<30, 31>(insn);
            switch (size) {
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
        Expression::Ptr InstructionDecoder_amdgpu::makeMemRefIndexLiteral() {
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
        void InstructionDecoder_amdgpu::getMemRefIndex_RT(Result_Type &rt) {
            unsigned int opc1 = field<23, 23>(insn);
            unsigned int size = field<30, 31>(insn);

            if (opc1 == 1) {
                switch (size) {
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
                switch (size) {
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

        void InstructionDecoder_amdgpu::getMemRefPair_RT(Result_Type &rt) {
            unsigned int isSigned = field<30, 30>(insn);
            unsigned int size = field<31, 31>(insn);

            // double the width
            switch (isSigned) {
                case 0:
                    switch (size) {
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
                    switch (size) {
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

        void InstructionDecoder_amdgpu::getMemRefIndex_SizeSizelen(unsigned int &size, unsigned int &sizeLen) {
            size = field<30, 31>(insn);
            if (isSIMDInsn && size == 0x0 && field<23, 23>(insn) == 0x1)
                size = 4;
            sizeLen = 31 - 30 + 1 + (size / 4);
            return;
        }

        void InstructionDecoder_amdgpu::getMemRefIndexPrePost_ImmImmlen(unsigned int &immVal, unsigned int &immLen) {
            immVal = field<12, 20>(insn);
            immLen = 20 - 12 + 1;
            return;
        }

        void InstructionDecoder_amdgpu::getMemRefPair_ImmImmlen(unsigned int &immVal, unsigned int &immLen) {
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
        Expression::Ptr InstructionDecoder_amdgpu::makeMemRefIndexUImm() {
            int immVal = field<10, 21>(insn);
            int immLen = 21 - 10 + 1;

            unsigned int size = 0, sizeLen = 0;
            getMemRefIndex_SizeSizelen(size, sizeLen);

            Expression::Ptr offset = Immediate::makeImmediate(
                    Result(u64, unsign_extend64(immLen + size, immVal << size)));

            Result_Type rt;
            getMemRefIndex_RT(rt);
            return makeDereferenceExpression(makeAddExpression(makeRnExpr(), offset, u64), rt);
        }

        Expression::Ptr InstructionDecoder_amdgpu::makeMemRefIndex_offset9() {
            unsigned int immVal = 0, immLen = 0;
            getMemRefIndexPrePost_ImmImmlen(immVal, immLen);
            return Immediate::makeImmediate(Result(u32, sign_extend32(immLen, immVal)));
        }

        // scale = 2 + opc<1>
        // scale = 1<<scale
        // LSL(sign_ex(imm7 , 64), scale)
        Expression::Ptr InstructionDecoder_amdgpu::makeMemRefPair_offset7() {
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

        Expression::Ptr InstructionDecoder_amdgpu::makeMemRefIndex_addOffset9() {
            Expression::Ptr offset = makeMemRefIndex_offset9();
            return makeAddExpression(makeRnExpr(), offset, u64);
        }

        Expression::Ptr InstructionDecoder_amdgpu::makeMemRefPair_addOffset7() {
            Expression::Ptr offset = makeMemRefPair_offset7();
            return makeAddExpression(makeRnExpr(), offset, u64);
        }

        Expression::Ptr InstructionDecoder_amdgpu::makeMemRefIndexPre() {
            Result_Type rt;
            getMemRefIndex_RT(rt);
            return makeDereferenceExpression(makeMemRefIndex_addOffset9(), rt);
        }

        Expression::Ptr InstructionDecoder_amdgpu::makeMemRefIndexPost() {
            Result_Type rt;
            getMemRefIndex_RT(rt);
            return makeDereferenceExpression(makeRnExpr(), rt);
        }

        Expression::Ptr InstructionDecoder_amdgpu::makeMemRefPairPre() {
            Result_Type rt;
            getMemRefPair_RT(rt);
            return makeDereferenceExpression(makeMemRefPair_addOffset7(), rt);
        }

        Expression::Ptr InstructionDecoder_amdgpu::makeMemRefPairPost() {
            Result_Type rt;
            getMemRefPair_RT(rt);

            return makeDereferenceExpression(makeRnExpr(), rt);
        }

        void InstructionDecoder_amdgpu::getMemRefEx_RT(Result_Type &rt) {
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

        Expression::Ptr InstructionDecoder_amdgpu::makeMemRefEx() {
            Result_Type rt;
            getMemRefEx_RT(rt);
            return makeDereferenceExpression(makeRnExpr(), rt);
        }

        void InstructionDecoder_amdgpu::OPRQ() {
            _Q = field<30, 30>(insn);
        }

        void InstructionDecoder_amdgpu::OPRL() {
            _L = field<30, 30>(insn);
        }

        unsigned int InstructionDecoder_amdgpu::getMemRefSIMD_MULT_T() {
            unsigned int Q = field<30, 30>(insn);
            return Q ? 128 : 64;
        }

        void InstructionDecoder_amdgpu::getMemRefSIMD_MULT_RT(Result_Type &rt) {
            unsigned int tmpSize = getMemRefSIMD_MULT_T();
            unsigned int rpt = 0, selem = 0;
            //getSIMD_MULT_RptSelem(rpt, selem);
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

        unsigned int InstructionDecoder_amdgpu::getSIMD_SING_selem() {
            return (((field<13, 13>(insn) << 1) & 0x2) | (field<21, 21>(insn) & 0x1)) + 0x1;
        }

        void InstructionDecoder_amdgpu::getMemRefSIMD_SING_RT(Result_Type &rt) {
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

        unsigned int InstructionDecoder_amdgpu::getMemRefSIMD_SING_T() {
            unsigned int opcode = field<14, 15>(insn);
            unsigned int S = field<12, 12>(insn);
            unsigned int size = field<10, 11>(insn);

            switch (opcode) {
                case 0x0:
                    return 8;
                case 0x1:
                    if ((size & 0x1) == 0x0)
                        return 16;
                    else
                        isValid = false;
                    break;
                case 0x2:
                    if (size == 0x0)
                        return 32;
                    else if (size == 0x1 && S == 0)
                        return 64;
                    else
                        isValid = false;
                    break;
                case 0x3:
                    return 8 << size;
                default:
                    isValid = false;
            }

            return 0;
        }

        Expression::Ptr InstructionDecoder_amdgpu::makeMemRefSIMD_MULT() {
            Result_Type rt = invalid_type;
            getMemRefSIMD_MULT_RT(rt);
            return makeDereferenceExpression(makeRnExpr(), rt);
        }

        Expression::Ptr InstructionDecoder_amdgpu::makeMemRefSIMD_SING() {
            Result_Type rt = invalid_type;
            getMemRefSIMD_SING_RT(rt);
            return makeDereferenceExpression(makeRnExpr(), rt);
        }

        void InstructionDecoder_amdgpu::getMemRefExPair_RT(Result_Type &rt) {
            int size = field<30, 30>(insn);
            switch (size) {
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

        Expression::Ptr InstructionDecoder_amdgpu::makeMemRefExPair() {
            Result_Type rt;
            getMemRefExPair_RT(rt);
            return makeDereferenceExpression(makeRnExpr(), rt);
        }

        /*
           Expression::Ptr InstructionDecoder_amdgpu::makeMemRefExPair16B(){
           return makeDereferenceExpression(makeRnExpr(), dbl128);
           }
           */

        /*
           Expression::Ptr InstructionDecoder_amdgpu::makeMemRefExPair2(){
           unsigned int immLen = 4, immVal = 8;
           Expression::Ptr offset = Immediate::makeImmediate(Result(u32, unsign_extend32(immLen, immVal)));
           return makeDereferenceExpression(makeAddExpression(makeRnExpr(), offset, u64) , u64);
           }
           */

        Expression::Ptr InstructionDecoder_amdgpu::makeMemRefReg_Rm() {
            unsigned int option = field<13, 15>(insn);
            if ((option & 2) != 2)
                isValid = false;
            MachRegister baseReg;// = ((option & 0x3) == 0x2) ? amdgpu::w0 : amdgpu::x0;
            unsigned int encoding = field<16, 20>(insn);

            return makeRegisterExpression(makeAmdgpuRegID(baseReg, encoding));
        }

        Expression::Ptr InstructionDecoder_amdgpu::makeMemRefReg_amount() {
            unsigned int S = field<12, 12>(insn);
            unsigned int amountVal = is64Bit ? (S == 0 ? 0 : 2) : (S == 0 ? 0 : 3);
            unsigned int amountLen = 2;

            return Immediate::makeImmediate(Result(u32, unsign_extend32(amountLen, amountVal)));
        }

        Expression::Ptr InstructionDecoder_amdgpu::makeMemRefReg_ext() {
            return NULL;
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
        Expression::Ptr InstructionDecoder_amdgpu::makeMemRefReg() {
            Expression::Ptr ext = makeMemRefReg_ext();
            Expression::Ptr xn = makeRnExpr();
            Expression::Ptr add = makeAddExpression(xn, ext, u64);

            Result_Type rt;
            getMemRefIndex_RT(rt);
            return makeDereferenceExpression(add, rt);
        }

        void InstructionDecoder_amdgpu::LIndex() {
        }

        void InstructionDecoder_amdgpu::STIndex() {

        }

        // This function is for non-writeback
        void InstructionDecoder_amdgpu::OPRRn() {
        }

        void InstructionDecoder_amdgpu::OPRRnL() {
        }

        void InstructionDecoder_amdgpu::OPRRnS() {
        }

        void InstructionDecoder_amdgpu::OPRRnU() {
            assert(0);
            /* this functions is useless
               insn_in_progress->appendOperand(makeRnExpr(), true, true);
               */
        }

        void InstructionDecoder_amdgpu::OPRRnLU() {
        }

        void InstructionDecoder_amdgpu::OPRRnSU() {
        }

        unsigned int InstructionDecoder_amdgpu::get_SIMD_MULT_POST_imm() {
            unsigned int rpt = 0, selem = 0;
            //getSIMD_MULT_RptSelem(rpt, selem);
            unsigned int numReg = rpt * selem;
            return _Q == 0x1 ? numReg << 4 : numReg << 3;
        }

        unsigned int InstructionDecoder_amdgpu::get_SIMD_SING_POST_imm() {
            return (getMemRefSIMD_SING_T() >> 3) * getSIMD_SING_selem();
        }

        Expression::Ptr InstructionDecoder_amdgpu::makeRmExpr() {
            int encoding = field<16, 20>(insn);
            MachRegister reg;

            return makeRegisterExpression(reg);
        }

        template<unsigned int endBit, unsigned int startBit>
            void InstructionDecoder_amdgpu::OPRoption() {
                hasOption = true;
                optionField = field<startBit, endBit>(insn);
            }

        MachRegister InstructionDecoder_amdgpu::getLoadStoreSimdRegister(int encoding) {
            MachRegister reg;

            return makeAmdgpuRegID(reg, encoding);
        }

        Expression::Ptr InstructionDecoder_amdgpu::makeRtExpr() {
            int encoding = field<0, 4>(insn);
            MachRegister reg;

            return makeRegisterExpression(reg);
        }

        Expression::Ptr InstructionDecoder_amdgpu::makeRt2Expr() {
            MachRegister baseReg;
            return makeRegisterExpression(baseReg);
        }

        Expression::Ptr InstructionDecoder_amdgpu::makeRaExpr() {
            int encoding = field<10, 14>(insn);
            MachRegister reg;

            return makeRegisterExpression(reg);

        }

        Expression::Ptr InstructionDecoder_amdgpu::makeb40Expr() {
            int b40Val = field<19, 23>(insn);
            int bitpos = ((is64Bit ? 1 : 0) << 5) | b40Val;

            return Immediate::makeImmediate(Result(u32, unsign_extend32(6, bitpos)));
        }

        bool InstructionDecoder_amdgpu::isSinglePrec() {
            return false;
        }

        Expression::Ptr InstructionDecoder_amdgpu::makeRsExpr() {
            MachRegister baseReg;
            return makeRegisterExpression(makeAmdgpuRegID(baseReg, field<16, 20>(insn)));
        }

        Expression::Ptr InstructionDecoder_amdgpu::makeFallThroughExpr() {
            return makeAddExpression(makePCExpr(), Immediate::makeImmediate(Result(u64, unsign_extend64(3, 4))), u64);
        }

        template<typename T, Result_Type rT>
            Expression::Ptr InstructionDecoder_amdgpu::fpExpand(int val) {
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
            Expression::Ptr InstructionDecoder_amdgpu::makeLogicalImm(int immr, int imms, int immsLen, Result_Type rT) {
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
                int R = immr & levels;

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

        bool InstructionDecoder_amdgpu::fix_bitfieldinsn_alias(int immr, int imms) {
            entryID modifiedID = insn_in_progress->getOperation().operationID;
            bool do_further_processing = true;

            return do_further_processing;
        }

        void InstructionDecoder_amdgpu::fix_condinsn_alias_and_cond(int &cond) {
        }

        void InstructionDecoder_amdgpu::modify_mnemonic_simd_upperhalf_insns() {
        }

        template<unsigned int endBit, unsigned int startBit>
            void InstructionDecoder_amdgpu::OPRimm() {
            }

        void InstructionDecoder_amdgpu::reorderOperands() {
        }

        void InstructionDecoder_amdgpu::processAlphabetImm() {
        }


        void InstructionDecoder_amdgpu::doDelayedDecode(const Instruction *insn_to_complete) {
            //cout << " callign do delayed decode " << endl;
            //InstructionDecoder::buffer b(insn_to_complete->ptr(), insn_to_complete->size());
            //insn_to_complete->m_Operands.reserve(4);
            //decode(b);
            //decodeOperands(insn_to_complete);
            //TODO : not sure why is this needed
        }

        bool InstructionDecoder_amdgpu::pre_process_checks(const amdgpu_insn_entry &entry) {
            bool ret = false;
            return ret;
        }

        bool InstructionDecoder_amdgpu::decodeOperands(const Instruction *insn_to_complete) {
            return true;
        }

        Expression::Ptr InstructionDecoder_amdgpu::decodeSGPRorM0(unsigned int offset){
            if( offset <= 104)
                return makeRegisterExpression(makeAmdgpuRegID(amdgpu::sgpr0,offset));
            if (offset == 124)
                return makeRegisterExpression(amdgpu::m0);
            assert(0 && "shouldn't reach here");
        }
       
        static Expression::Ptr conditionalAssignment(unsigned int cond, Expression::Ptr first, Result_Type result_type){
            if(cond)
                return first;
            return Immediate::makeImmediate(Result(result_type,0));
        }

        void InstructionDecoder_amdgpu::decodeFLATOperands(){
            layout_flat & layout = insn_layout.flat;
            //const amdgpu_insn_entry & insn_entry = amdgpu::flat_insn_table[layout.op];

            Expression::Ptr addr_ast = 
                makeTernaryExpression(
                        makeRegisterExpression(amdgpu::address_mode_32), // TODO: type needs to be fixed
                        makeRegisterExpression(makeAmdgpuRegID(amdgpu::vgpr0,layout.addr)),
                        makeRegisterExpression(makeAmdgpuRegID(amdgpu::vgpr0,layout.addr,2)),
                        u64
                        );
            switch(layout.seg){
                case 1:
                    insn_in_progress->getOperation().mnemonic += "_scratch";
                    break;
                case 2:
                    insn_in_progress->getOperation().mnemonic += "_global";
                    break;
                default:
                    break;
            }

            insn_in_progress->appendOperand(addr_ast,false,true);

        }
        void InstructionDecoder_amdgpu::decodeMUBUFOperands(){
            layout_mubuf & layout = insn_layout.mubuf;
            const amdgpu_insn_entry  &insn_entry = amdgpu_insn_entry::mubuf_insn_table[layout.op];

            MachRegister vsharp = makeAmdgpuRegID(amdgpu::sgpr0,layout.srsrc<<2,4);
            Expression::Ptr const_base_ast   = makeRegisterExpression(vsharp,0,47); 
            Expression::Ptr const_stride_ast = makeRegisterExpression(vsharp,48,65); 
            Expression::Ptr num_records = makeRegisterExpression(vsharp,66,97); 
            Expression::Ptr add_tid_enable = makeRegisterExpression(vsharp,98,98); 
            Expression::Ptr swizzle_enable = makeRegisterExpression(vsharp,99,99); 
            Expression::Ptr element_size = makeRegisterExpression(vsharp,100,101); 
            Expression::Ptr index_stride = makeRegisterExpression(vsharp,102,103); 
            
            Expression::Ptr offset_expr = Immediate::makeImmediate(Result(u32,0)) ;
            Expression::Ptr index_expr = Immediate::makeImmediate(Result(u32,0)) ;

            if(layout.idxen){
                if(layout.offen){
                    index_expr =  makeRegisterExpression(makeAmdgpuRegID(amdgpu::vgpr0,layout.vaddr));
                    offset_expr =  makeRegisterExpression(makeAmdgpuRegID(amdgpu::vgpr0,layout.vaddr+1));
                }else{
                    index_expr =  makeRegisterExpression(makeAmdgpuRegID(amdgpu::vgpr0,layout.vaddr));
                }

            }else{
                if(layout.offen){
                    offset_expr =  makeRegisterExpression(makeAmdgpuRegID(amdgpu::vgpr0,layout.vaddr));
                }else{
                    // do nothing
                }
            }


            Expression::Ptr index_ast = 
                makeAddExpression(
                    index_expr
                    ,
                    makeTernaryExpression(
                        add_tid_enable,
                        makeRegisterExpression(amdgpu::tid),
                        Immediate::makeImmediate(Result(u32,0)),
                        u32
                    ),
                    u32
                );
            
            Expression::Ptr offset_ast = 
                makeAddExpression(
                    offset_expr,
                    Immediate::makeImmediate(Result(u32,layout.offset)),
                    u32
                );

            Expression::Ptr buffer_offset = makeAddExpression(
                    offset_ast,
                    makeMultiplyExpression(
                        const_stride_ast,
                        index_ast,
                        u64
                     ),
                    u64         
                );

            Expression::Ptr sgpr_offset_ast = decodeSGPRorM0(layout.soffset);
            Expression::Ptr addr_ast = makeAddExpression(
                    makeAddExpression(
                        const_base_ast,
                        sgpr_offset_ast,
                        u64),
                    buffer_offset,
                    u64
                    );
            
            Expression::Ptr vdata_ast = makeRegisterExpression(
                    makeAmdgpuRegID(amdgpu::vgpr0,layout.vdata,
                        num_elements// TODO: This depends on number of elements, which is available from opcode
                        ));

            insn_in_progress->appendOperand(addr_ast,true,true);
            insn_in_progress->appendOperand(vdata_ast,true,true);
            
            if(layout.lds){
                Expression::Ptr lds_offset_ast = makeRegisterExpression(amdgpu::m0,0,15);
                Expression::Ptr mem_offset_ast = makeRegisterExpression(
                        makeAmdgpuRegID(amdgpu::sgpr0,layout.soffset));
                Expression::Ptr inst_offset_ast = Immediate::makeImmediate(Result(u32,layout.offset));
                Expression::Ptr lds_addr_ast =
                    makeAddExpression(
                        makeAddExpression(
                            makeAddExpression(
                                makeRegisterExpression(amdgpu::lds_base),
                                lds_offset_ast,
                                u32
                            ),
                            inst_offset_ast,
                            u32
                        ),
                        makeMultiplyExpression(
                            makeRegisterExpression(amdgpu::tid),
                            Immediate::makeImmediate(Result(u16,4)),
                            u32
                        ),
                        u32
                    );
                insn_in_progress->getOperation().mnemonic+="_lds";
                insn_in_progress->appendOperand(lds_addr_ast,false,true);

            }
        }


        void InstructionDecoder_amdgpu::decodeMTBUFOperands(){
            layout_mtbuf & layout = insn_layout.mtbuf;
            const amdgpu_insn_entry  &insn_entry = amdgpu_insn_entry::mtbuf_insn_table[layout.op];

            MachRegister vsharp = makeAmdgpuRegID(amdgpu::sgpr0,layout.srsrc<<2,4);
            Expression::Ptr const_base_ast   = makeRegisterExpression(vsharp,0,47); 
            Expression::Ptr const_stride_ast = makeRegisterExpression(vsharp,48,65); 
            Expression::Ptr num_records = makeRegisterExpression(vsharp,66,97); 
            Expression::Ptr add_tid_enable = makeRegisterExpression(vsharp,98,98); 
            Expression::Ptr swizzle_enable = makeRegisterExpression(vsharp,99,99); 
            Expression::Ptr element_size = makeRegisterExpression(vsharp,100,101); 
            Expression::Ptr index_stride = makeRegisterExpression(vsharp,102,103); 
            
            Expression::Ptr offset_expr = Immediate::makeImmediate(Result(u32,0)) ;
            Expression::Ptr index_expr = Immediate::makeImmediate(Result(u32,0)) ;

            if(layout.idxen){
                if(layout.offen){
                    index_expr =  makeRegisterExpression(makeAmdgpuRegID(amdgpu::vgpr0,layout.vaddr));
                    offset_expr =  makeRegisterExpression(makeAmdgpuRegID(amdgpu::vgpr0,layout.vaddr+1));
                }else{
                    index_expr =  makeRegisterExpression(makeAmdgpuRegID(amdgpu::vgpr0,layout.vaddr));
                }

            }else{
                if(layout.offen){
                    offset_expr =  makeRegisterExpression(makeAmdgpuRegID(amdgpu::vgpr0,layout.vaddr));
                }else{
                    // do nothing
                }
            }


            Expression::Ptr index_ast = 
                makeAddExpression(
                    index_expr
                    ,
                    makeTernaryExpression(
                        add_tid_enable,
                        makeRegisterExpression(amdgpu::tid),
                        Immediate::makeImmediate(Result(u32,0)),
                        u32
                    ),
                    u32
                );
            
            Expression::Ptr offset_ast = 
                makeAddExpression(
                    offset_expr,
                    Immediate::makeImmediate(Result(u32,layout.offset)),
                    u32
                );

            Expression::Ptr buffer_offset = makeAddExpression(
                    offset_ast,
                    makeMultiplyExpression(
                        const_stride_ast,
                        index_ast,
                        u64
                     ),
                    u64         
                );

            Expression::Ptr sgpr_offset_ast = decodeSGPRorM0(layout.soffset);
            Expression::Ptr addr_ast = makeAddExpression(
                    makeAddExpression(
                        const_base_ast,
                        sgpr_offset_ast,
                        u64),
                    buffer_offset,
                    u64
                    );
            
            Expression::Ptr vdata_ast = makeRegisterExpression(
                    makeAmdgpuRegID(amdgpu::vgpr0,layout.vdata,
                        num_elements// TODO: This depends on number of elements, which is available from opcode
                        ));

            insn_in_progress->appendOperand(addr_ast,true,true);
            insn_in_progress->appendOperand(vdata_ast,true,true);

            
        }
        void InstructionDecoder_amdgpu::decodeVOP1Operands(){

            layout_vop1 & layout = insn_layout.vop1;
            const amdgpu_insn_entry  &insn_entry = amdgpu_insn_entry::vop1_insn_table[layout.op];

            InstructionDecoder::buffer *tmp = new InstructionDecoder::buffer("",0);
            insn_in_progress->appendOperand(decodeSSRC(*tmp,layout.vdst),false,true);
            insn_in_progress->appendOperand(decodeSSRC(*tmp,layout.src0),true,false);

        }

       
        void InstructionDecoder_amdgpu::decodeSOP1Operands(){

            layout_sop1 & layout = insn_layout.sop1;
            const amdgpu_insn_entry  &insn_entry = amdgpu_insn_entry::sop1_insn_table[layout.op];

            InstructionDecoder::buffer *tmp = new InstructionDecoder::buffer("",0);
            insn_in_progress->appendOperand(decodeSSRC(*tmp,layout.sdst),false,true);
            insn_in_progress->appendOperand(decodeSSRC(*tmp,layout.ssrc0),true,false);

        }

        void InstructionDecoder_amdgpu::decodeSOP2Operands(){

            layout_sop2 & layout = insn_layout.sop2;
            const amdgpu_insn_entry  &insn_entry = amdgpu_insn_entry::sop2_insn_table[layout.op];

            InstructionDecoder::buffer *tmp = new InstructionDecoder::buffer("",0);
            insn_in_progress->appendOperand(decodeSSRC(*tmp,layout.sdst),false,true);
            insn_in_progress->appendOperand(decodeSSRC(*tmp,layout.ssrc1),true,false);
            insn_in_progress->appendOperand(decodeSSRC(*tmp,layout.ssrc0),true,false);

        }
        void InstructionDecoder_amdgpu::decodeSMEMOperands(){
            layout_smem & layout = insn_layout.smem;

            const amdgpu_insn_entry  &insn_entry = amdgpu_insn_entry::smem_insn_table[layout.op];
            if(IS_LD_ST()){
                Expression::Ptr offset_expr ;
                Expression::Ptr inst_offset_expr ;
                if(layout.imm==0){
                    MachRegister mr;
                    unsigned int indexing_offset = layout.soe ? layout.soffset : layout.offset;
                    offset_expr = decodeSGPRorM0(indexing_offset);
                    inst_offset_expr = Immediate::makeImmediate(Result(u64,0));
                }else{
                    inst_offset_expr = Immediate::makeImmediate(Result(u32,layout.offset));
                    offset_expr = layout.soe ? 
                        decodeSGPRorM0(layout.soffset) : Immediate::makeImmediate(Result(u64,0));
                }
//                cout << "layout.sbase = " << std::hex << layout.sbase << endl;
                MachRegister mr = makeAmdgpuRegID(amdgpu::sgpr0,4,2);
//                cout << " shouldn't it be " << amdgpu::sgpr_vec2_0 << " " << mr << endl; 
                Expression::Ptr sbase_expr = makeRegisterExpression(makeAmdgpuRegID(amdgpu::sgpr0,layout.sbase << 1,2));
                if(isScratch)
                    offset_expr = makeMultiplyExpression(offset_expr,
                            Immediate::makeImmediate(Result(u64,64)),u64);

                Expression::Ptr remain_expr = makeAddExpression(inst_offset_expr,offset_expr,u64);
                Expression::Ptr addr_expr = makeDereferenceExpression(makeAddExpression(sbase_expr,remain_expr,u64),u64);


                InstructionDecoder::buffer *tmp = new InstructionDecoder::buffer("",0);
                
                Expression::Ptr sdata_expr = makeRegisterExpression(makeAmdgpuRegID(amdgpu::sgpr0,layout.sdata,num_elements));


                insn_in_progress->appendOperand(sdata_expr,true,false);

                insn_in_progress->appendOperand(addr_expr,false,true);

            }

        }

        bool InstructionDecoder_amdgpu::decodeOperands(const Instruction *insn_to_complete, const amdgpu_insn_entry & insn_entry) {
            if(insn_entry.operandCnt!=0){
                for (std::size_t i =0 ; i < insn_entry.operandCnt; i++){
                    std::mem_fun(insn_entry.operands[i])(this);
                }
            }
            return true;
        }


        int InstructionDecoder_amdgpu::findInsnTableIndex(unsigned int decoder_table_index) {
            return 0;
        }
        inline unsigned int get32bit(InstructionDecoder::buffer &b,unsigned int offset ){
            assert(offset %4 ==0 );
            return b.start[offset+3] << 24 | b.start[offset + 2] << 16 | b.start[offset +1 ] << 8 | b.start [offset];
        }

        template<unsigned int endBit, unsigned int startBit>
            void InstructionDecoder_amdgpu::OPRssrc() {

            }
        void InstructionDecoder_amdgpu::insnSize(unsigned int insn_size) {
            this->insn_size = insn_size;
        }


        void decodeMemory(unsigned int base, unsigned offset , unsigned int m0){
        }


        Expression::Ptr InstructionDecoder_amdgpu::decodeVSRC(InstructionDecoder::buffer &b, unsigned int index){
            if (index > 255)
                index = 0;
            MachRegister mr = makeAmdgpuRegID(amdgpu::vgpr0,index & 0xff);
            return makeRegisterExpression(mr);
        } 

        Expression::Ptr InstructionDecoder_amdgpu::decodeSSRC(InstructionDecoder::buffer &b, unsigned int index){
            if (index < 102){
                MachRegister mr = makeAmdgpuRegID(amdgpu::sgpr0,index);
                return makeRegisterExpression(mr);
            }else if ( 106 == index ){
                return makeRegisterExpression(amdgpu::vcc_lo);
            }else if ( 107 == index ){

                return makeRegisterExpression(amdgpu::vcc_hi);
            }else if (124 == index ){
                
                return makeRegisterExpression(amdgpu::m0);
            }else if ( 126 == index ){
                return makeRegisterExpression(amdgpu::exec_lo);
            }else if ( 127 == index ){

                return makeRegisterExpression(amdgpu::exec_hi);
            }else if( 128 <= index && index <= 192){
                return Immediate::makeImmediate(Result(u32, unsign_extend32(32,index - 128 )));
            }else if( 193 <= index && index <= 208 ){
                return Immediate::makeImmediate(Result(s32, sign_extend32(32,-(index - 192) )));
            }else if( 209 <= index && index <= 234){
                assert ( 0 && "reserved index " ) ;
            }else if (240 == index){
                return Immediate::makeImmediate(Result(sp_float, 0.5) );
            }else if (241 == index){
                return Immediate::makeImmediate(Result(sp_float, -0.5) );
            }else if (242 == index){
                return Immediate::makeImmediate(Result(sp_float, 1.0) );
            }else if (243 == index){
                return Immediate::makeImmediate(Result(sp_float, -1.0) );
            }else if (244 == index){
                return Immediate::makeImmediate(Result(sp_float, 2.0) );
            }else if (245 == index){
                return Immediate::makeImmediate(Result(sp_float, -2.0) );
            }else if (246 == index){
                return Immediate::makeImmediate(Result(sp_float, 4.0) );
            }else if (247 == index){
                return Immediate::makeImmediate(Result(sp_float, -4.0) );
            }else if (248 == index){
                return Immediate::makeImmediate(Result(dp_float, (1.0f / (3.1415926535*2)) ) );
            }else if (249 == index  || 250 == index){
                assert ( 0 && "reserved index " ) ;
            }else if ( 251 == index ){
                return makeRegisterExpression(amdgpu::vccz);
            }else if ( 252 == index ){
                return makeRegisterExpression(amdgpu::execz);
            }else if ( 254 == index ){
                assert ( 0 && "reserved index " ) ;
            }
            else if(index == 0xff){

                unsigned int imm = get32bit(b,4);
                //std::cerr << "\nusing imm " << imm << std::endl;
                useImm = true;
                return Immediate::makeImmediate(Result(u32, unsign_extend32(32,imm )));
            } 
            cerr << std::dec << index << endl;
            
            assert(0 && "unknown register value ");
            MachRegister mr = makeAmdgpuRegID(amdgpu::sgpr0,0);
            return makeRegisterExpression(mr);
        }
#include "amdgpu_decoder_impl_vega.C"

        Instruction InstructionDecoder_amdgpu::decode(InstructionDecoder::buffer &b) {
            num_elements = 1;
            useImm = false;
            if (b.start > b.end)
                return Instruction();
            insn = insn_high = insn_long = 0;

            insn = get32bit(b,0);
            insn_high = 0;

            if(b.start + 4 <= b.end)
                insn_long = get32bit(b,4);
            insn_high = insn_long;

            insn_long = (insn_long << 32) | insn;


            //insn = b.start[3] << 24 | b.start[2] << 16 | b.start[1] << 8 | b.start[0];

            mainDecode(b);


            return *insn_in_progress;
        }

    };
};



