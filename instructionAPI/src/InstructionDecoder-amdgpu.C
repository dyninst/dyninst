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

        void InstructionDecoder_amdgpu::decodeSMEMOperands(){
            layout_smem & layout = insn_layout.smem;

            const amdgpu_insn_entry  &insn_entry = amdgpu_insn_entry::smem_insn_table[layout.op];
            if(!isScratch){
                // sgpr[124] == m0
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
                cout << "layout.sbase = " << std::hex << layout.sbase << endl;
                MachRegister mr = makeAmdgpuRegID(amdgpu::sgpr0,4,2);
                cout << " shouldn't it be " << amdgpu::sgpr_vec2_0 << " " << mr << endl; 
                Expression::Ptr sbase_expr = makeRegisterExpression(makeAmdgpuRegID(amdgpu::sgpr0,layout.sbase << 1,2));

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
            }else if (index == 128){

                return Immediate::makeImmediate(Result(u32,0));
            }else if(index == 0xff){
                unsigned int imm = get32bit(b,4);
                //std::cerr << "\nusing imm " << imm << std::endl;
                useImm = true;
                return Immediate::makeImmediate(Result(u32, unsign_extend32(32,imm )));
            } 

            MachRegister mr = makeAmdgpuRegID(amdgpu::sgpr0,0);
            return makeRegisterExpression(mr);
        }
#include "amdgpu_decoder_impl_vega.C"

/*        void InstructionDecoder_amdgpu::decodeSOP2(InstructionDecoder::buffer &b){
            unsigned int insn_size = 4;
            layout_sop2 & sop2 = insn_layout.sop2;
            sop2.op = longfield<23,29>(insn_long);
            sop2.sdst = longfield<16,22>(insn_long);
            sop2.ssrc1 = longfield<8,15>(insn_long);
            sop2.ssrc0 = longfield<0,7>(insn_long);

            Expression::Ptr dst_expr = decodeSSRC(b,sop2.sdst );
            Expression::Ptr src1_expr = decodeSSRC(b,sop2.ssrc1);
            Expression::Ptr src0_expr = decodeSSRC(b,sop2.ssrc0);
            if(useImm)
                insn_size += 4;

            const amdgpu_insn_entry  &insn_entry = sop2_insn_table[sop2.op];

            this->insn_in_progress =makeInstruction(insn_entry.op, insn_entry.mnemonic , insn_size ,reinterpret_cast<unsigned char *>(&insn));


            //this->insn_in_progress =makeInstruction(amdgpu_op_sop1_nop, "sop2", insn_size ,reinterpret_cast<unsigned char *>(&insn));

            this->insn_in_progress->appendOperand(src1_expr,true,false);
            this->insn_in_progress->appendOperand(dst_expr,false,true);
            this->insn_in_progress->appendOperand(src0_expr,true,false);


        }

        void InstructionDecoder_amdgpu::decodeSOP1(InstructionDecoder::buffer &b){
            layout_sop1 & sop1 = insn_layout.sop1;
            sop1.sdst = longfield<16,22>(insn_long); 
            sop1.op = longfield<8,15>(insn_long); 
            sop1.ssrc0 = longfield<0,7>(insn_long); 

            unsigned int insn_size = 4;

            Expression::Ptr dst_expr = decodeSSRC(b,sop1.sdst);
            Expression::Ptr src_expr = decodeSSRC(b,sop1.ssrc0);

            // TODO use a lookup table to make sure we simply parse the srcfield and get a expression, simplify logic
            if (useImm){
                insn_size += 4;
            } 

            const amdgpu_insn_entry  &insn_entry = sop1_insn_table[sop1.op];

            this->insn_in_progress =makeInstruction(insn_entry.op, insn_entry.mnemonic , insn_size ,reinterpret_cast<unsigned char *>(&insn));

            //this->insn_in_progress =makeInstruction(amdgpu_op_sop1_nop, "sop1", insn_size ,reinterpret_cast<unsigned char *>(&insn));
            this->insn_in_progress->appendOperand(dst_expr,false,true);
            this->insn_in_progress->appendOperand(src_expr,true,false);

        }
        void InstructionDecoder_amdgpu::decodeSOPP(InstructionDecoder::buffer &b){
            layout_sopp & sopp = insn_layout.sopp;
            sopp.op = longfield<16,22>(insn_long); 
            sopp.simm16 = longfield<0,15>(insn_long); 
            unsigned int insn_size = 4;
            Expression::Ptr simm_expr =  Immediate::makeImmediate(Result(u32, unsign_extend32(32,sopp.simm16 )));

            const amdgpu_insn_entry  &insn_entry = sopp_insn_table[sopp.op];

            this->insn_in_progress =makeInstruction(insn_entry.op, insn_entry.mnemonic , insn_size ,reinterpret_cast<unsigned char *>(&insn));
            this->insn_in_progress->appendOperand(simm_expr,true,false);

        }

        void InstructionDecoder_amdgpu::decodeSOPK(InstructionDecoder::buffer &b){
            layout_sopk & sopk = insn_layout.sopk;
            sopk.op = longfield<23,27>(insn_long);
            sopk.sdst = longfield<16,22>(insn_long);
            sopk.simm16 = longfield<0,15>(insn_long);

            const amdgpu_insn_entry  &insn_entry = sopp_insn_table[sopk.op];
            this->insn_in_progress =makeInstruction(insn_entry.op, insn_entry.mnemonic , insn_size ,reinterpret_cast<unsigned char *>(&insn));
            //this->insn_in_progress->appendOperand(simm_expr,true,false);



        }
        void InstructionDecoder_amdgpu::decodeSOPC(InstructionDecoder::buffer &b){
            layout_sopc & sopc = insn_layout.sopc;

        }


        void InstructionDecoder_amdgpu::decodeSALU(InstructionDecoder::buffer &b){
            if(IS_SOP1(this->insn)){
                decodeSOP1(b);
                return;
            } 
            else if(IS_SOP2(this->insn)){
                decodeSOP2(b);
                return;
            } 

            else iflayoutSOPC(this->insn)){

                decodeSOPC(b);
                return;
            } 
            else if(IS_SOPP(this->insn)){
                decodeSOPP(b);return;
            } 
            else if(IS_SOPK(this->insn)){
                decodeSOPK(b);
                return;
            }else{
                assert(0 && "shouldn't reach here" ) ;
            }        
        }

        void InstructionDecoder_amdgpu::decodeSMEM(InstructionDecoder::buffer &b){
            unsigned insn_size = 8 ;
            layout_smem & layout = insn_layout.smem;
            layout.soffset = longfield<57,63>(insn_long);
            layout.offset = longfield<32,52>(insn_long);
            layout.op = longfield<18,25>(insn_long);
            layout.imm = longfield<17,17>(insn_long);
            layout.glc = longfield<16,16>(insn_long); // TODO : for cache coherence, not related for now
            layout.nv = longfield<15,15>(insn_long);
            layout.soe = longfield<14,14>(insn_long);
            layout.sdata =longfield<6,12>(insn_long);
            layout.sbase =longfield<0,5>(insn_long);
            
            const amdgpu_insn_entry  &insn_entry = smem_insn_table[layout.op];
            this->insn_in_progress =makeInstruction(insn_entry.op, insn_entry.mnemonic , insn_size ,reinterpret_cast<unsigned char *>(&insn));

            //cout<< "insn = " << std::hex << insn_long << "offsset = " << std::hex << layout.offset << " soffset = " << std::hex << layout.soffset  << "end" << endl; 

            Expression::Ptr offset_expr ;
            if(layout.imm==0){
                MachRegister mr;
                if(layout.soe == 0){
                    //cout << " SGPR [base ] + SGPR[offset] " << endl;
                    mr = makeAmdgpuRegID(amdgpu::sgpr0,layout.offset); // TODO : We should be dealing with a pair/quad of SGPRs instead of single one
                }else{

                    //cout << " SGPR [base ] + SGPR[soffset] " << endl;
                    mr = makeAmdgpuRegID(amdgpu::sgpr0,layout.soffset); // TODO : We should be dealing with a pair/quad of SGPRs instead of single one
                }
                offset_expr = makeRegisterExpression(mr);
            }else{
                if(layout.soe ==0){
                    offset_expr = Immediate::makeImmediate(Result(u32,layout.offset));

                    //cout << " SGPR [base ] + IMM[soffset] " << endl;
                }else{
                    // BinaryFunction(); addition of two expressions
                    // 
                    cout << " SGPR [base ] + +IMM[offset] + SGPR[soffset] " << endl;
                    offset_expr = makeAddExpression(
                            Immediate::makeImmediate(Result(u32,layout.offset)),
                            makeRegisterExpression(makeAmdgpuRegID(amdgpu::sgpr0,layout.soffset)),u64);
                }
            }

            Expression::Ptr soffset_expr = Immediate::makeImmediate(Result(u32,layout.soffset));
            Expression::Ptr sdata_expr = decodeSSRC(b,layout.sdata);
            Expression::Ptr sbase_expr = makeRegisterPairExpr(amdgpu::sgpr0,layout.sbase << 1,2);
            if (layout.op == 2){
                sdata_expr = makeRegisterPairExpr(amdgpu::sgpr0,layout.sdata,4);
            } 
            Expression::Ptr addr_expr = makeAddExpression(sbase_expr,offset_expr,u64); 


            this->insn_in_progress->appendOperand(sdata_expr,true,false);
            //this->insn_in_progress->appendOperand(sbase_expr,false,true);

            this->insn_in_progress->appendOperand(addr_expr,false,true);
            //this->insn_in_progress->appendOperand(soffset_expr,false,true);
            //this->insn_in_progress->appendOperand(offset_expr,false,true);



        }

        void InstructionDecoder_amdgpu::decodeVOP1(InstructionDecoder::buffer &b){
            this->insn_in_progress =makeInstruction(e_nop, "vop1", 4,reinterpret_cast<unsigned char *>(&insn));
            insn_high = get32bit(b,4);
            unsigned int vdst_index = field<17,24>(insn);
            unsigned int src_index = field<0,8>(insn);

            Expression::Ptr vdst_expr = decodeVSRC(b,vdst_index);
            Expression::Ptr src_expr = decodeSSRC(b,src_index);
            this->insn_in_progress->appendOperand(vdst_expr,false,true);
            this->insn_in_progress->appendOperand(src_expr,true,false);


        }
        void InstructionDecoder_amdgpu::decodeVOP2(InstructionDecoder::buffer &b){
            unsigned insn_size = 4;
            layout_vop2 & vop2 = insn_layout.vop2;
            vop2.op = longfield<25,30>(insn_long);
            vop2.vdst = longfield<17,24>(insn_long);
            vop2.vsrc1 = longfield<9,16>(insn_long);
            vop2.src0 = longfield<0,8>(insn_long);

            const amdgpu_insn_entry  &insn_entry = vop2_insn_table[vop2.op];
            this->insn_in_progress =makeInstruction(insn_entry.op, insn_entry.mnemonic , insn_size ,reinterpret_cast<unsigned char *>(&insn));


            insn_high = get32bit(b,4);
            unsigned int vdst_index = field<17,24>(insn);
            unsigned int vsrc1_index = field<9,16>(insn);
            unsigned int src0_index = field<0,8>(insn);

            Expression::Ptr vdst_expr = decodeVSRC(b,vdst_index);
            Expression::Ptr vsrc1_expr = decodeVSRC(b,vsrc1_index);
            Expression::Ptr src0_expr = decodeSSRC(b,src0_index);

            this->insn_in_progress->appendOperand(vdst_expr,false,true);
            this->insn_in_progress->appendOperand(vsrc1_expr,true,false);
            this->insn_in_progress->appendOperand(src0_expr,true,false);


        }
        void InstructionDecoder_amdgpu::decodeVOPC(InstructionDecoder::buffer &b){
            this->insn_in_progress =makeInstruction(e_nop, "vopc", 4,reinterpret_cast<unsigned char *>(&insn));
            insn_high = get32bit(b,4);
            unsigned int vsrc_index = field<9,16>(insn);
            unsigned int src0_index = field<0,8>(insn);

            Expression::Ptr vsrc_expr = decodeVSRC(b,vsrc_index);
            Expression::Ptr src0_expr = decodeSSRC(b,src0_index);

            this->insn_in_progress->appendOperand(vsrc_expr,true,false);
            this->insn_in_progress->appendOperand(src0_expr,true,false);


        }
        void InstructionDecoder_amdgpu::decodeVOP3A(InstructionDecoder::buffer &b){
            this->insn_in_progress =makeInstruction(e_nop, "vop3a", 4,reinterpret_cast<unsigned char *>(&insn));
        }
        void InstructionDecoder_amdgpu::decodeVOP3B(InstructionDecoder::buffer &b){
            this->insn_in_progress =makeInstruction(e_nop, "vop3b", 4,reinterpret_cast<unsigned char *>(&insn));
        }


        void InstructionDecoder_amdgpu::decodeVOP3AB(InstructionDecoder::buffer &b){
            // 480, 481, 488 ,489 is for VOP3B
            unsigned int vop3ab_op = field<16,25>(insn);
            switch(vop3ab_op){
                case 480:
                case 481:
                case 488:
                case 489:
                    decodeVOP3B(b);
                    break;
                default:
                    decodeVOP3A(b);
                    break;
            }
        }
        void InstructionDecoder_amdgpu::decodeVOP3P(InstructionDecoder::buffer &b){
            this->insn_in_progress =makeInstruction(e_nop, "vop3p", 4,reinterpret_cast<unsigned char *>(&insn));
        }
        void InstructionDecoder_amdgpu::decodeFLAT(InstructionDecoder::buffer &b){
            this->insn_in_progress =makeInstruction(e_nop, "flat", 8,reinterpret_cast<unsigned char *>(&insn));
        }





        void InstructionDecoder_amdgpu::mainDecode(InstructionDecoder::buffer & b) {

            if (IS_SALU(this->insn)){
                decodeSALU(b);
            }else if (IS_SMEM(this->insn)){
                decodeSMEM(b);
            }else if (IS_VOP1(this->insn)){
                decodeVOP1(b);
            }else if(IS_VOP2(this->insn)){
                decodeVOP2(b);
            }else if(IS_VOPC(this->insn)){
                decodeVOPC(b);
            }else if(IS_VOP3AB(this->insn)){
                decodeVOP3AB(b);
            }else if(IS_VOP3P(this->insn)){
                decodeVOP3P(b);

            }else if(IS_FLAT(this->insn)){
                decodeFLAT(b);
            }else 

                this->insn_in_progress =makeInstruction(e_nop, "else", 4,reinterpret_cast<unsigned char *>(&insn));
            return;
        }
*/
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



