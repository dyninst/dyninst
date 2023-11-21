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

#define INSIDE_INSTRUCTION_API

#include "InstructionDecoder-x86.h"
#include "Expression.h"
#include "common/src/arch-x86.h"
#include "Register.h"
#include "Dereference.h"
#include "Immediate.h" 
#include "BinaryFunction.h"
#include "common/src/singleton_object_pool.h"
#include "unaligned_memory_access.h"
#include "registers/x86_regs.h"
#include "registers/x86_64_regs.h"
#include "registers/abstract_regs.h"

// #define VEX_DEBUG

using namespace std;
using namespace NS_x86;
namespace Dyninst
{
    namespace InstructionAPI
    {
    
        bool readsOperand(unsigned int opsema, unsigned int i)
        {
            switch(opsema) {
                case s1R2R:
                    return (i == 0 || i == 1);
                case s1R:
                case s1RW:
                    return i == 0;
                case s1W:
                    return false;
                case s1W2RW:
                case s1W2R:   // second operand read, first operand written (e.g. mov)
                    return i == 1;
                case s1RW2R:  // two operands read, first written (e.g. add)
                case s1RW2RW: // e.g. xchg
                case s1R2RW:
                    return i == 0 || i == 1;
                case s1W2R3R: // e.g. imul
                case s1W2RW3R: // some mul
                case s1W2R3RW: // (stack) push & pop
                    return i == 1 || i == 2;
                case s1W2W3R: // e.g. les
                    return i == 2;
                case s1RW2R3RW:
                case s1RW2R3R: // shld/shrd
                case s1RW2RW3R: // [i]div, cmpxch8b
                case s1R2R3R:
                    return i == 0 || i == 1 || i == 2;
                case s1W2R3R4R:
                    return i == 1 || i == 2 || i == 3;
                case s1RW2R3R4R:
                    return i == 0 || i == 1 || i == 2 || i == 3;
                case sNONE:
                    return false;
                default:
                    return false;
                    // printf("OPSEMA: %d\n", opsema);
                    assert(!"Unknown opsema!");
                    return false;
            }
      
        }
      
        bool writesOperand(unsigned int opsema, unsigned int i)
        {
            switch(opsema) {
                case s1R2R:
                case s1R:
                    return false;
                case s1RW:
                case s1W:
                case s1W2R:   // second operand read, first operand written (e.g. mov)
                case s1RW2R:  // two operands read, first written (e.g. add)
                case s1W2R3R: // e.g. imul
                case s1RW2R3R: // shld/shrd
                case s1RW2R3R4R:
                  return i == 0;
                case s1R2RW:
                  return i == 1;
                case s1W2RW:
                case s1RW2RW: // e.g. xchg
                case s1W2RW3R: // some mul
                case s1W2W3R: // e.g. les
                case s1RW2RW3R: // [i]div, cmpxch8b
                  return i == 0 || i == 1;
                case s1W2R3RW: // (stack) push & pop
                  return i == 0 || i == 2;
                case s1RW2R3RW:
                  return i == 0 || i == 2;
                case sNONE:
                default:
                  return false;
                    // printf("OPSEMA: %d\n", opsema);
                  assert(!"Unknown opsema!");
                  return false;
            }
        }

        bool implicitOperand(unsigned int implicit_operands, unsigned int i)
        {
            return sGetImplicitOP(implicit_operands, i) != 0x0;
        }


    INSTRUCTION_EXPORT InstructionDecoder_x86::InstructionDecoder_x86(Architecture a) :
      InstructionDecoderImpl(a),
      locs(NULL),
      decodedInstruction(NULL),
      sizePrefixPresent(false),
      addrSizePrefixPresent(false)
    {
      if(a == Arch_x86_64) InstructionDecoder_x86::setMode(true);
      
    }
    INSTRUCTION_EXPORT InstructionDecoder_x86::~InstructionDecoder_x86()
    {
        free(decodedInstruction);
        free(locs);
    }
    static const unsigned char modrm_use_sib = 4;
    
    INSTRUCTION_EXPORT void InstructionDecoder_x86::setMode(bool is64)
    {
        InstructionDecoder_x86::is64BitMode = is64;
    }
    
      Expression::Ptr InstructionDecoder_x86::makeSIBExpression(const InstructionDecoder::buffer& b)
    {
        unsigned scale;
        Register index;
        Register base;
        Result_Type registerType = is64BitMode ? u64 : u32;

        int op_type = is64BitMode ? op_q : op_d;
        decode_SIB(locs->sib_byte, scale, index, base);

        Expression::Ptr scaleAST(make_shared(singleton_object_pool<Immediate>::construct(Result(u8, dword_t(scale)))));
        Expression::Ptr indexAST(make_shared(singleton_object_pool<RegisterAST>::construct(makeRegisterID(index, op_type,
                                    locs->rex_x))));
        Expression::Ptr baseAST;
        if(base == 0x05)
        {
            switch(locs->modrm_mod)
            {
                case 0x00:
                    baseAST = decodeImmediate(op_d, b.start + locs->sib_position + 1, true);
                    break;
                case 0x01: 
                case 0x02: 
                    baseAST = make_shared(singleton_object_pool<RegisterAST>::construct(makeRegisterID(base,
											       op_type,
											       locs->rex_b)));
                    break;
                case 0x03:
                default:
                    assert(0);
                    break;
            };
        }
        else
        {
            baseAST = make_shared(singleton_object_pool<RegisterAST>::construct(makeRegisterID(base,
											       op_type,
											       locs->rex_b)));
        }

        if(index == 0x04 && (!(is64BitMode) || !(locs->rex_x)))
        {
            return baseAST;
        }
        return makeAddExpression(baseAST, makeMultiplyExpression(indexAST, scaleAST, registerType), registerType);
    }

     Expression::Ptr InstructionDecoder_x86::makeModRMExpression(const InstructionDecoder::buffer& b,
								  unsigned int opType)
    {
       unsigned int regType = op_d;
       Result_Type aw;
       if(is64BitMode)
       {
          if(addrSizePrefixPresent)
          {
             aw = u32;
          } else {
             aw = u64;
             regType = op_q;
          }
       } else {
          if(!addrSizePrefixPresent)
          {
             aw = u32;
          } else {
             aw = u16;
             regType = op_w;
          }
       }

       if (opType == op_lea)
       {
          // For an LEA, aw (address width) is insufficient, use makeSizeType
          aw = makeSizeType(opType);
       }

       Expression::Ptr e =
          makeRegisterExpression(makeRegisterID(locs->modrm_rm, regType, locs->rex_b));
       switch(locs->modrm_mod)
       {
          case 0:
             /* modrm_rm == 0x4 is use SIB */
             if(locs->modrm_rm == modrm_use_sib)
                e = makeSIBExpression(b);
             else if(locs->modrm_rm == 0x5 && !addrSizePrefixPresent)
             {
                /* modrm_rm 00 0x5 is use 32 bit displacement only */
                assert(locs->opcode_position > -1);
                if(is64BitMode)
                {
                   e = makeAddExpression(makeRegisterExpression(x86_64::rip),
                         getModRMDisplacement(b), aw);
                } else {
                   e = getModRMDisplacement(b);
                }

             } else{
                e = makeRegisterExpression(makeRegisterID(locs->modrm_rm, regType, locs->rex_b));
             }

             if(opType == op_lea)
                return e;

             return makeDereferenceExpression(e, makeSizeType(opType));

          case 1:
          case 2:
             // Expression::Ptr disp_e = makeAddExpression(e, getModRMDisplacement(b), aw);

             /* Both MOD values 0b01 and 0b10 can have sibs */
             if(locs->modrm_rm == modrm_use_sib)
                e = makeSIBExpression(b);
             e = makeAddExpression(e, getModRMDisplacement(b), aw);

             if(opType == op_lea)
                return e;

             return makeDereferenceExpression(e, makeSizeType(opType));
          case 3:
             return makeRegisterExpression(makeRegisterID(locs->modrm_rm, opType, locs->rex_b));
          default:
             /* This should never happen */
             assert(0);
             return Expression::Ptr();

       };

       // can't get here, but make the compiler happy...
       assert(0);
       return Expression::Ptr();
    }

    Expression::Ptr InstructionDecoder_x86::decodeImmediate(unsigned int opType, const unsigned char* immStart, 
							    bool isSigned)
    {
        // rex_w indicates we need to sign-extend also.
        isSigned = isSigned || locs->rex_w;
	
        switch(opType)
        {
            case op_b:
                return Immediate::makeImmediate(Result(isSigned ? s8 : u8 ,*(const byte_t*)(immStart)));
                break;
            case op_d:
                return Immediate::makeImmediate(Result(isSigned ? s32 : u32,Dyninst::read_memory_as<dword_t>(immStart)));
            case op_w:
                return Immediate::makeImmediate(Result(isSigned ? s16 : u16,Dyninst::read_memory_as<word_t>(immStart)));
                break;
            case op_q:
                return Immediate::makeImmediate(Result(isSigned ? s64 : u64,Dyninst::read_memory_as<int64_t>(immStart)));
                break;
            case op_v:
                if (locs->rex_w || isDefault64Insn()) {
                    return Immediate::makeImmediate(Result(isSigned ? s64 : u64,Dyninst::read_memory_as<int64_t>(immStart)));
                }
                //if(!sizePrefixPresent)
                //{
                    return Immediate::makeImmediate(Result(isSigned ? s32 : u32,Dyninst::read_memory_as<dword_t>(immStart)));
		    //}
		    //else
		    //{
                    //return Immediate::makeImmediate(Result(isSigned ? s16 : u16,*(const word_t*)(immStart)));
		    //}
		break;
            case op_z:
                // 32 bit mode & no prefix, or 16 bit mode & prefix => 32 bit
                // 16 bit mode, no prefix or 32 bit mode, prefix => 16 bit
                //if(!addrSizePrefixPresent)
                //{
                    return Immediate::makeImmediate(Result(isSigned ? s32 : u32,Dyninst::read_memory_as<dword_t>(immStart)));
		    //}
		    //else
		    //{
                    //return Immediate::makeImmediate(Result(isSigned ? s16 : u16,*(const word_t*)(immStart)));
		    //}
                break;
            case op_p:
                // 32 bit mode & no prefix, or 16 bit mode & prefix => 48 bit
                // 16 bit mode, no prefix or 32 bit mode, prefix => 32 bit
                if(!sizePrefixPresent)
                {
                    return Immediate::makeImmediate(Result(isSigned ? s48 : u48,Dyninst::read_memory_as<int64_t>(immStart)));
                }
                else
                {
                    return Immediate::makeImmediate(Result(isSigned ? s32 : u32,Dyninst::read_memory_as<dword_t>(immStart)));
                }

                break;
            case op_a:
            case op_dq:
            case op_pd:
            case op_ps:
            case op_s:
            case op_si:
            case op_lea:
            case op_allgprs:
            case op_512:
            case op_c:
                assert(!"Can't happen: opType unexpected for valid ways to decode an immediate");
                return Expression::Ptr();
            default:
                assert(!"Can't happen: opType out of range");
                return Expression::Ptr();
        }
    }
    
    Expression::Ptr InstructionDecoder_x86::getModRMDisplacement(const InstructionDecoder::buffer& b)
    {
        int disp_pos;

        if(locs->sib_position != -1)
            disp_pos = locs->sib_position + 1;
        else disp_pos = locs->modrm_position + 1;

        switch(locs->modrm_mod)
        {
            case 1:
                return make_shared(singleton_object_pool<Immediate>::construct(Result(s8, Dyninst::read_memory_as<byte_t>(b.start +
                                        disp_pos))));
                break;
            case 2:
                if(0 && sizePrefixPresent)
                {
                    return make_shared(singleton_object_pool<Immediate>::construct(Result(s16, Dyninst::read_memory_as<word_t>(b.start +
                                            disp_pos))));
                }
                else
                {
                    return make_shared(singleton_object_pool<Immediate>::construct(Result(s32, Dyninst::read_memory_as<dword_t>(b.start +
                                            disp_pos))));
                }
                break;
            case 0:
                // In 16-bit mode, the word displacement is modrm r/m 6
                if(sizePrefixPresent && !is64BitMode)
                {
                    if(locs->modrm_rm == 6)
                    {
                        return make_shared(singleton_object_pool<Immediate>::construct(Result(s16,
                        	Dyninst::read_memory_as<dword_t>(b.start + disp_pos))));
                    }
                    // TODO FIXME; this was decoding wrong, but I'm not sure
                    // why...
                    else if (locs->modrm_rm == 5) {
                        assert(b.start + disp_pos + 4 <= b.end);
                        return make_shared(singleton_object_pool<Immediate>::construct(Result(s32,
                        	Dyninst::read_memory_as<dword_t>(b.start + disp_pos))));
                    } else {
                        assert(b.start + disp_pos + 1 <= b.end);
                        return make_shared(singleton_object_pool<Immediate>::construct(Result(s8, 0)));
                    }
                    break;
                }
                // ...and in 32-bit mode, the dword displacement is modrm r/m 5
                else
                {
                    if(locs->modrm_rm == 5)
                    {
                        if (b.start + disp_pos + 4 <= b.end)
                            return make_shared(singleton_object_pool<Immediate>::construct(Result(s32,
                        	    Dyninst::read_memory_as<dword_t>(b.start + disp_pos))));
                        else
                            return make_shared(singleton_object_pool<Immediate>::construct(Result()));
                    }
                    else
                    {
                        if (b.start + disp_pos + 1 <= b.end)
                            return make_shared(singleton_object_pool<Immediate>::construct(Result(s8, 0)));
                        else
                        {
                            return make_shared(singleton_object_pool<Immediate>::construct(Result()));
                        }
                    }
                    break;
                }
            default:
                assert(b.start + disp_pos + 1 <= b.end);
                return make_shared(singleton_object_pool<Immediate>::construct(Result(s8, 0)));
        }
    }

    enum intelRegBanks
    {
        b_8bitNoREX = 0,
        b_16bit,
        b_32bit,
        b_segment,
        b_64bit,
        b_xmm_set0, /* XMM0 -> XMM 7 */
        b_xmm_set1, /* XMM8 -> XMM 15 */
        b_xmm_set2, /* XMM16 -> XMM 23 */
        b_xmm_set3, /* XMM24 -> XMM 31 */
        b_ymm_set0, /* YMM0 -> YMM 7 */
        b_ymm_set1, /* YMM8 -> YMM 15 */
        b_ymm_set2, /* YMM16 -> YMM 23 */
        b_ymm_set3, /* YMM24 -> YMM 31 */
        b_zmm_set0, /* ZMM0 -> ZMM 7 */
        b_zmm_set1, /* ZMM8 -> ZMM 15 */
        b_zmm_set2, /* ZMM16 -> ZMM 23 */
        b_zmm_set3, /* ZMM24 -> ZMM 31 */
		b_kmask,
        b_mm,
        b_cr,
        b_dr,
        b_tr,
        b_amd64ext,
        b_8bitWithREX,
        b_fpstack,
	    amd64_ext_8,
	    amd64_ext_16,
	    amd64_ext_32,

        b_invalid /* should remain the final entry */
    };

    static MachRegister IntelRegTable32[][8] = {
        { x86::al, x86::cl, x86::dl, x86::bl, x86::ah, x86::ch, x86::dh, x86::bh }, /* b_8bitNoREX */
        { x86::ax, x86::cx, x86::dx, x86::bx, x86::sp, x86::bp, x86::si, x86::di }, /* b_16bit */
        { x86::eax, x86::ecx, x86::edx, x86::ebx, x86::esp, x86::ebp, x86::esi, x86::edi }, /* b_32bit */
        { x86::es, x86::cs, x86::ss, x86::ds, x86::fs, x86::gs, InvalidReg, InvalidReg }, /* b_segment */
        { x86_64::rax, x86_64::rcx, x86_64::rdx, x86_64::rbx, x86_64::rsp, x86_64::rbp, x86_64::rsi, x86_64::rdi }, /* b_64bit */
        { x86::xmm0, x86::xmm1, x86::xmm2, x86::xmm3, x86::xmm4, x86::xmm5, x86::xmm6, x86::xmm7 }, /* b_xmm_set0 */
        { x86_64::xmm8, x86_64::xmm9, x86_64::xmm10, x86_64::xmm11, x86_64::xmm12, x86_64::xmm13, x86_64::xmm14, x86_64::xmm15 }, /* b_xmm_set1 */
        { x86_64::xmm16, x86_64::xmm17, x86_64::xmm18, x86_64::xmm19, x86_64::xmm20, x86_64::xmm21, x86_64::xmm22, x86_64::xmm23 }, /* b_xmm_set2 */
        { x86_64::xmm24, x86_64::xmm25, x86_64::xmm26, x86_64::xmm27, x86_64::xmm28, x86_64::xmm29, x86_64::xmm30, x86_64::xmm31 }, /* b_xmm_set3 */
        { x86_64::ymm0, x86_64::ymm1, x86_64::ymm2, x86_64::ymm3, x86_64::ymm4, x86_64::ymm5, x86_64::ymm6, x86_64::ymm7 }, /* b_ymm_set0 */
        { x86_64::ymm8, x86_64::ymm9, x86_64::ymm10, x86_64::ymm11, x86_64::ymm12, x86_64::ymm13, x86_64::ymm14, x86_64::ymm15 }, /* b_ymm_set1 */
        { x86_64::ymm16, x86_64::ymm17, x86_64::ymm18, x86_64::ymm19, x86_64::ymm20, x86_64::ymm21, x86_64::ymm22, x86_64::ymm23 }, /* b_ymm_set2 */
        { x86_64::ymm24, x86_64::ymm25, x86_64::ymm26, x86_64::ymm27, x86_64::ymm28, x86_64::ymm29, x86_64::ymm30, x86_64::ymm31 }, /* b_ymm_set3 */
        { x86_64::zmm0, x86_64::zmm1, x86_64::zmm2, x86_64::zmm3, x86_64::zmm4, x86_64::zmm5, x86_64::zmm6, x86_64::zmm7 }, /* b_zmm_set0 */
        { x86_64::zmm8, x86_64::zmm9, x86_64::zmm10, x86_64::zmm11, x86_64::zmm12, x86_64::zmm13, x86_64::zmm14, x86_64::zmm15 }, /* b_zmm_set1 */
        { x86_64::zmm16, x86_64::zmm17, x86_64::zmm18, x86_64::zmm19, x86_64::zmm20, x86_64::zmm21, x86_64::zmm22, x86_64::zmm23 }, /* b_zmm_set2 */
        { x86_64::zmm24, x86_64::zmm25, x86_64::zmm26, x86_64::zmm27, x86_64::zmm28, x86_64::zmm29, x86_64::zmm30, x86_64::zmm31 }, /* b_zmm_set3 */
		{ x86_64::k0, x86_64::k1, x86_64::k2, x86_64::k3, x86_64::k4, x86_64::k5, x86_64::k6, x86_64::k7 },
        { x86::mm0, x86::mm1, x86::mm2, x86::mm3, x86::mm4, x86::mm5, x86::mm6, x86::mm7 },
        { x86::cr0, x86::cr1, x86::cr2, x86::cr3, x86::cr4, x86::cr5, x86::cr6, x86::cr7 },
        { x86::dr0, x86::dr1, x86::dr2, x86::dr3, x86::dr4, x86::dr5, x86::dr6, x86::dr7 },
        { x86::tr0, x86::tr1, x86::tr2, x86::tr3, x86::tr4, x86::tr5, x86::tr6, x86::tr7 },
        { x86_64::r8, x86_64::r9, x86_64::r10, x86_64::r11, x86_64::r12, x86_64::r13, x86_64::r14, x86_64::r15 },
        { x86_64::al, x86_64::cl, x86_64::dl, x86_64::bl, x86_64::spl, x86_64::bpl, x86_64::sil, x86_64::dil },
        { x86::st0, x86::st1, x86::st2, x86::st3, x86::st4, x86::st5, x86::st6, x86::st7 }
    };

    static MachRegister IntelRegTable64[][8] = {
        { x86_64::al, x86_64::cl, x86_64::dl, x86_64::bl, x86_64::ah, x86_64::ch, x86_64::dh, x86_64::bh },
        { x86_64::ax, x86_64::cx, x86_64::dx, x86_64::bx, x86_64::sp, x86_64::bp, x86_64::si, x86_64::di },
        { x86_64::eax, x86_64::ecx, x86_64::edx, x86_64::ebx, x86_64::esp, x86_64::ebp, x86_64::esi, x86_64::edi },
        { x86_64::es, x86_64::cs, x86_64::ss, x86_64::ds, x86_64::fs, x86_64::gs, InvalidReg, InvalidReg },
        { x86_64::rax, x86_64::rcx, x86_64::rdx, x86_64::rbx, x86_64::rsp, x86_64::rbp, x86_64::rsi, x86_64::rdi },
        { x86_64::xmm0, x86_64::xmm1, x86_64::xmm2, x86_64::xmm3, x86_64::xmm4, x86_64::xmm5, x86_64::xmm6, x86_64::xmm7 }, /* b_xmm_set0 */
        { x86_64::xmm8, x86_64::xmm9, x86_64::xmm10, x86_64::xmm11, x86_64::xmm12, x86_64::xmm13, x86_64::xmm14, x86_64::xmm15 }, /* b_xmm_set1 */
        { x86_64::xmm16, x86_64::xmm17, x86_64::xmm18, x86_64::xmm19, x86_64::xmm20, x86_64::xmm21, x86_64::xmm22, x86_64::xmm23 }, /* b_xmm_set2 */
        { x86_64::xmm24, x86_64::xmm25, x86_64::xmm26, x86_64::xmm27, x86_64::xmm28, x86_64::xmm29, x86_64::xmm30, x86_64::xmm31 }, /* b_xmm_set3 */
        { x86_64::ymm0, x86_64::ymm1, x86_64::ymm2, x86_64::ymm3, x86_64::ymm4, x86_64::ymm5, x86_64::ymm6, x86_64::ymm7 }, /* b_ymm_set0 */
        { x86_64::ymm8, x86_64::ymm9, x86_64::ymm10, x86_64::ymm11, x86_64::ymm12, x86_64::ymm13, x86_64::ymm14, x86_64::ymm15 }, /* b_ymm_set1 */
        { x86_64::ymm16, x86_64::ymm17, x86_64::ymm18, x86_64::ymm19, x86_64::ymm20, x86_64::ymm21, x86_64::ymm22, x86_64::ymm23 }, /* b_ymm_set2 */
        { x86_64::ymm24, x86_64::ymm25, x86_64::ymm26, x86_64::ymm27, x86_64::ymm28, x86_64::ymm29, x86_64::ymm30, x86_64::ymm31 }, /* b_ymm_set3 */
        { x86_64::zmm0, x86_64::zmm1, x86_64::zmm2, x86_64::zmm3, x86_64::zmm4, x86_64::zmm5, x86_64::zmm6, x86_64::zmm7 }, /* b_zmm_set0 */
        { x86_64::zmm8, x86_64::zmm9, x86_64::zmm10, x86_64::zmm11, x86_64::zmm12, x86_64::zmm13, x86_64::zmm14, x86_64::zmm15 }, /* b_zmm_set1 */
        { x86_64::zmm16, x86_64::zmm17, x86_64::zmm18, x86_64::zmm19, x86_64::zmm20, x86_64::zmm21, x86_64::zmm22, x86_64::zmm23 }, /* b_zmm_set2 */
        { x86_64::zmm24, x86_64::zmm25, x86_64::zmm26, x86_64::zmm27, x86_64::zmm28, x86_64::zmm29, x86_64::zmm30, x86_64::zmm31 }, /* b_zmm_set3 */
		{ x86_64::k0, x86_64::k1, x86_64::k2, x86_64::k3, x86_64::k4, x86_64::k5, x86_64::k6, x86_64::k7 },
        { x86_64::mm0, x86_64::mm1, x86_64::mm2, x86_64::mm3, x86_64::mm4, x86_64::mm5, x86_64::mm6, x86_64::mm7 },
        { x86_64::cr0, x86_64::cr1, x86_64::cr2, x86_64::cr3, x86_64::cr4, x86_64::cr5, x86_64::cr6, x86_64::cr7 },
        { x86_64::dr0, x86_64::dr1, x86_64::dr2, x86_64::dr3, x86_64::dr4, x86_64::dr5, x86_64::dr6, x86_64::dr7 },
        { x86_64::tr0, x86_64::tr1, x86_64::tr2, x86_64::tr3, x86_64::tr4, x86_64::tr5, x86_64::tr6, x86_64::tr7 },
        { x86_64::r8, x86_64::r9, x86_64::r10, x86_64::r11, x86_64::r12, x86_64::r13, x86_64::r14, x86_64::r15 },
        { x86_64::al, x86_64::cl, x86_64::dl, x86_64::bl, x86_64::spl, x86_64::bpl, x86_64::sil, x86_64::dil },
        { x86_64::st0, x86_64::st1, x86_64::st2, x86_64::st3, x86_64::st4, x86_64::st5, x86_64::st6, x86_64::st7 },
	    { x86_64::r8b, x86_64::r9b, x86_64::r10b, x86_64::r11b, x86_64::r12b, x86_64::r13b, x86_64::r14b, x86_64::r15b },
	    { x86_64::r8w, x86_64::r9w, x86_64::r10w, x86_64::r11w, x86_64::r12w, x86_64::r13w, x86_64::r14w, x86_64::r15w },
	    { x86_64::r8d, x86_64::r9d, x86_64::r10d, x86_64::r11d, x86_64::r12d, x86_64::r13d, x86_64::r14d, x86_64::r15d },
    };

  /* Uses the appropriate lookup table based on the 
     decoder architecture */
  class IntelRegTable_access {
    public:
        inline MachRegister operator()(Architecture arch,
                                       intelRegBanks bank,
                                       int index)
        {
            assert(index >= 0 && index < 8);
    
            if(arch == Arch_x86_64)
                return IntelRegTable64[bank][index];
            else if(arch == Arch_x86) 
	    {
	      if(bank > b_fpstack) return InvalidReg;
	      return IntelRegTable32[bank][index];
	    }
	    assert(0);
	    return InvalidReg;
        }

  };
  static IntelRegTable_access IntelRegTable;

      bool InstructionDecoder_x86::isDefault64Insn()
      {
	switch(m_Operation.getID())
	{
	case e_jmp:
	case e_pop:
	case e_push:
	case e_call:
	  return true;
	default:
	  return false;
	}
	
      }
      

    MachRegister InstructionDecoder_x86::makeRegisterID(unsigned int intelReg, unsigned int opType,
                                        bool isExtendedReg)
    {
        MachRegister retVal;
	

        if(isExtendedReg)
        {
	    switch(opType)
	    {
	        case op_q:  
		    retVal = IntelRegTable(m_Arch,b_amd64ext,intelReg);
		    break;
		case op_d:
		    retVal = IntelRegTable(m_Arch,amd64_ext_32,intelReg);
		    break;
		case op_w:
		    retVal = IntelRegTable(m_Arch,amd64_ext_16,intelReg);
		    break;
		case op_b:
		    retVal = IntelRegTable(m_Arch,amd64_ext_8,intelReg);
	            break;
		case op_v:
		    if (locs->rex_w || isDefault64Insn())
		        retVal = IntelRegTable(m_Arch, b_amd64ext, intelReg);
	            else if (!sizePrefixPresent)
		        retVal = IntelRegTable(m_Arch, amd64_ext_32, intelReg);
		    //else
		    //    retVal = IntelRegTable(m_Arch, amd64_ext_16, intelReg);
		    break;	
		case op_p:
		case op_z:
		    //		    if (!sizePrefixPresent)
		        retVal = IntelRegTable(m_Arch, amd64_ext_32, intelReg);
			//		    else
			//  retVal = IntelRegTable(m_Arch, amd64_ext_16, intelReg);
		    break;
	    case op_f:
	    case op_dbl:
		// extended reg ignored on FP regs
		retVal = IntelRegTable(m_Arch, b_fpstack,intelReg);
		break;
		default:
		    retVal = InvalidReg;
	    }
        }
        /* Promotion to 64-bit only applies to the operand types
           that are varible (c,v,z). Ignoring c and z because they
           do the right thing on 32- and 64-bit code.
        else if(locs->rex_w)
        {
            // AMD64 with 64-bit operands
            retVal = IntelRegTable[b_64bit][intelReg];
        }
        */
        else
        {
            switch(opType)
            {
                case op_v:
		  if(locs->rex_w || isDefault64Insn())
                        retVal = IntelRegTable(m_Arch,b_64bit,intelReg);
                    else
                        retVal = IntelRegTable(m_Arch,b_32bit,intelReg);
                    break;
                case op_b:
                    if (locs->rex_byte & 0x40) {
                        retVal = IntelRegTable(m_Arch,b_8bitWithREX,intelReg);
                    } else {
                        retVal = IntelRegTable(m_Arch,b_8bitNoREX,intelReg);
                    }
                    break;
                case op_q:
                    retVal = IntelRegTable(m_Arch,b_64bit,intelReg);
                    break;
                case op_w:
                    retVal = IntelRegTable(m_Arch,b_16bit,intelReg);
                    break;
                case op_f:
                case op_dbl:
                    retVal = IntelRegTable(m_Arch,b_fpstack,intelReg);
                    break;
                case op_d:
                case op_si:
                    retVal = IntelRegTable(m_Arch,b_32bit,intelReg);
                    break;
                default:
                    retVal = IntelRegTable(m_Arch,b_32bit,intelReg);
                    break;
            }
        }

        if (!is64BitMode) {
	  if ((retVal.val() & 0x00ffffff) == 0x0001000c)
	    assert(0);
	}

        return MachRegister((retVal.val() & ~retVal.getArchitecture()) | m_Arch);
    }
    
    Result_Type InstructionDecoder_x86::makeSizeType(unsigned int opType)
    {
        switch(opType)
        {
            case op_b:
            case op_c:
                return u8;
            case op_d:
            case op_ss:
            case op_allgprs:
            case op_si:
                return u32;
            case op_w:
            case op_a:
                return u16;
            case op_q:
            case op_sd:
                return u64;
            case op_v:
            case op_lea:
            case op_z:
                if (locs->rex_w) 
                {
                    return u64;
                }
		//if(is64BitMode || !sizePrefixPresent)
                //{
                    return u32;
		    //}
		    //else
		    //{
                    //return u16;
		    //}
                break;
            case op_y:
            	if(is64BitMode)
            		return u64;
            	else
            		return u32;
            	break;
            case op_p:
                // book says operand size; arch-x86 says word + word * operand size
                if(!is64BitMode ^ sizePrefixPresent)
                {
                    return u48;
                }
                else
                {
                    return u32;
                }
            case op_dq:
            case op_qq:
                return u64;
            case op_512:
                return m512;
            case op_pi:
            case op_ps:
            case op_pd:
                return dbl128;
            case op_s:
                return u48;
            case op_f:
                return sp_float;
            case op_dbl:
                return dp_float;
            case op_14:
                return m14;
            default:
                assert(!"Can't happen!");
                return u8;
        }
    }

    enum AVX_Regtype { AVX_XMM = 0, AVX_YMM, AVX_ZMM, AVX_NONE };
    #define AVX_TYPE_OKAY(type) ((type) >= AVX_XMM && (type) <= AVX_ZMM)
    /** 
     * Decode an avx register based on the type of prefix. Returns true if the
     * given configuration is invalid and should be rejected.
     */
    bool decodeAVX(intelRegBanks& bank, int* bank_index, int regnum, AVX_Regtype type, ia32_prefixes& pref, unsigned int admet)
    {

        /* Check to see if this is just a normal MMX register access */
        if(type >= AVX_NONE || type < 0)
        {
#ifdef VEX_DEBUG
            printf("VEX OPERAND:  REGNUM: %d  ", regnum);
            printf("REG_TYPE: AVX_NONE (%d)\n", type);
#endif
            /* Only registers XMM0 - XMM15 are usable */

            /* The register must be valid */
            if(regnum < 0) 
                return true;

            if(regnum < 8)
            {
                bank = b_xmm_set0;
                *bank_index = regnum;
            } else if(regnum < 16)
            {
                bank = b_xmm_set1;
                *bank_index = regnum - 8;
            } else {
                /* Value is out of the valid range */
                return true;
            }

            /* Return success */
            return false;
        }

        switch(admet)
        {
            case am_V: case am_YV: case am_XV:
                switch(pref.vex_type)
                {
                    case VEX_TYPE_EVEX:
                        regnum |= pref.vex_R << 4;
                        regnum |= pref.vex_r << 3;
                        break;
                    case VEX_TYPE_VEX2:
                    case VEX_TYPE_VEX3:
                        regnum |= pref.vex_r << 3;
                        break;
                    default:break;
                }
                break;

            case am_U: case am_YU: case am_XU:
            case am_W: case am_YW: case am_XW:
                switch(pref.vex_type)
                {
                    case VEX_TYPE_EVEX:
                        regnum |= pref.vex_x << 4;
                        regnum |= pref.vex_b << 3;
                        break;
                    case VEX_TYPE_VEX3:
                        regnum |= pref.vex_x << 4;
                        regnum |= pref.vex_b << 3;
                        break;
                    default: break;
                }
                break;

			case am_HK: case am_VK: case am_WK:
			    bank = b_kmask;
			    *bank_index = regnum;
			    if(*bank_index > 7)
				    *bank_index = 7;
			    else if(*bank_index < 0)
				    *bank_index = 0;
                return false; /* Return success */
            case am_B:
                bank = b_64bit;
                if(regnum > 7)
                {
                    regnum -= 8;
                    bank = b_amd64ext;
                }

                if(regnum < 0)
                    regnum = 0;
                if(regnum > 7)
                    regnum = 7;

                *bank_index = regnum;
                break;
            default:  break;/** SSE instruction */
        }

#ifdef VEX_DEBUG
        printf("VEX OPERAND:  REGNUM: %d  ", regnum);
#endif

        /* Operand is potentially XMM, YMM or ZMM */
        int setnum = 0;
        if(regnum < 8)
        {
            setnum = 0;
            *bank_index = regnum;
        } else if(regnum < 16)
        {
            setnum = 1;
            *bank_index = regnum - 8;
        } else if(regnum < 24)
        {
            setnum = 2;
            *bank_index = regnum - 16;
        } else if(regnum < 32){
            setnum = 3;
            *bank_index = regnum - 24;
        } else {
#ifdef VEX_DEBUG
            printf("AVX REGISTER NUMBER:   %d   is invalid!!\n", regnum);
#endif
            return false;
        }

        switch(type)
        {
            case AVX_XMM:
#ifdef VEX_DEBUG
                printf("REG_TYPE: AVX_XMM (%d)\n", type);
#endif
                if(setnum == 0)
                    bank = b_xmm_set0;
                else if(setnum == 1)
                    bank = b_xmm_set1;
                else if(setnum == 2)
                    bank = b_xmm_set2;
                else if(setnum == 3)
                    bank = b_xmm_set3;
                else return true;
                break;
            case AVX_YMM:
#ifdef VEX_DEBUG
                printf("REG_TYPE: AVX_YMM (%d)\n", type);
#endif
                if(setnum == 0)
                    bank = b_ymm_set0;
                else if(setnum == 1)
                    bank = b_ymm_set1;
                else if(setnum == 2)
                    bank = b_ymm_set2;
                else if(setnum == 3)
                    bank = b_ymm_set3;
                else return true;
                break;
            case AVX_ZMM:
#ifdef VEX_DEBUG
                printf("REG_TYPE: AVX_ZMM (%d)\n", type);
#endif
                if(setnum == 0)
                    bank = b_zmm_set0;
                else if(setnum == 1)
                    bank = b_zmm_set1;
                else if(setnum == 2)
                    bank = b_zmm_set2;
                else if(setnum == 3)
                    bank = b_zmm_set3;
                else return true;
                break;
            default:
                return true;
        }

        /* Return Success */
        return false;
    }

    bool InstructionDecoder_x86::decodeOneOperand(const InstructionDecoder::buffer& b,
						  const ia32_operand& operand,
						  int & imm_index, /* immediate operand index */
						  const Instruction* insn_to_complete,
                          bool isRead, bool isWritten, bool isImplicit)
    {
        bool isCFT = false;
        bool isCall = false;
        bool isConditional = false;
        InsnCategory cat = insn_to_complete->getCategory();

        if(cat == c_BranchInsn || cat == c_CallInsn)
        {
            isCFT = true;
            if(cat == c_CallInsn)
            {
                isCall = true;
            }
        }

        if(cat == c_BranchInsn && insn_to_complete->getOperation().getID() != e_jmp)
        {
            isConditional = true;
        }

        /* There must be a decoded instruction */
        if(!decodedInstruction)
            assert(!"No decoded instruction!\n");

        unsigned int optype = operand.optype;
        AVX_Regtype avx_type = AVX_NONE; /* The AVX register type (if VEX prefixed) */
        intelRegBanks bank = b_invalid; /* Specifies an AVX bank to use for register decoding */
        int bank_index = -1; /* Specifies a bank index for an AVX register */
        ia32_prefixes& pref = *decodedInstruction->getPrefix();
        int regnum; /* Used to keep track of some register positions */

        if (sizePrefixPresent
                && ((optype == op_v) || (optype == op_z))
                && (operand.admet != am_J))
        {
            optype = op_w;
        }

        if(pref.vex_present)
        {
            /* Get the AVX type from the prefix */
            avx_type = (AVX_Regtype)pref.vex_ll;
        }

        if (sizePrefixPresent && ((optype == op_v) 
                    || (optype == op_z)) && (operand.admet != am_J))
        {
            optype = op_w;
        }

        if(optype == op_y) 
        {
            if(is64BitMode && locs->rex_w)
            {
                optype = op_q;
            } else {
                optype = op_d;
            }
        }

        switch(operand.admet)
        {
            case 0:
                // No operand
                {
                    // fprintf(stderr, "ERROR: Instruction with mismatched operands. Raw bytes: ");
                    // for(unsigned int i = 0; i < decodedInstruction->getSize(); i++) {
                    //	fprintf(stderr, "%x ", b.start[i]);
                    // }
                    // fprintf(stderr, "\n");*/
                    assert(!"Mismatched number of operands--check tables");
                    return false;
                }
                return false;
            case am_A:
                {
                    // am_A only shows up as a far call/jump.  Position 1 should be universally safe.
                    Expression::Ptr addr(decodeImmediate(optype, b.start + 1));
                    insn_to_complete->addSuccessor(addr, isCall, false, false, false);
                }
                break;

            case am_B:
                {
                    // Selects a general purpose register from VEX.vvvv (VEX3 or EVEX)
                    if(!pref.vex_present)
                    {
                        // assert(!"Non VEX3 or EVEX instruction with am_B addressing mode!");
                        return false;
                    }

                    /* Grab the correct bank and bank index for this type of register */
                    if(decodeAVX(bank, &bank_index, pref.vex_vvvv_reg,
                                avx_type, pref, operand.admet))
                        return false;

                    /* Append the operand */
                    insn_to_complete->appendOperand(makeRegisterExpression(
                                IntelRegTable(m_Arch, bank, bank_index)),
                            isRead, isWritten, isImplicit);

                    // Expression::Ptr op(makeRegisterExpression(
                    // makeRegisterID(pref.vex_vvvv_reg, optype, locs->rex_r)));
                    // insn_to_complete->appendOperand(op, isRead, isWritten);
                }
                break;

            case am_C:
                {
                    Expression::Ptr op(makeRegisterExpression(
                                IntelRegTable(m_Arch,b_cr,locs->modrm_reg)));
                    insn_to_complete->appendOperand(op, isRead, isWritten, isImplicit);
                }
                break;

            case am_D:
                {
                    Expression::Ptr op(makeRegisterExpression(
                                IntelRegTable(m_Arch,b_dr,locs->modrm_reg)));
                    insn_to_complete->appendOperand(op, isRead, isWritten, isImplicit);
                }
                break;

            case am_E:
                // am_M is like am_E, except that mod of 0x03 should 
                // never occur (am_M specified memory,
                // mod of 0x03 specifies direct register access).
            case am_M:
                // am_R is the inverse of am_M; it should only have a mod of 3
            case am_R:
                // can be am_R or am_M	
            case am_RM:
                if(isCFT)
                {
                    insn_to_complete->addSuccessor(
                            makeModRMExpression(b, optype), 
                            isCall, true, false, false);
                } else {
                    insn_to_complete->appendOperand(
                            makeModRMExpression(b, optype), 
                            isRead, isWritten, isImplicit);
                }
                break;

            case am_F:
                {
                    Expression::Ptr op(makeRegisterExpression(x86::flags));
                    insn_to_complete->appendOperand(op, isRead, isWritten, isImplicit);
                }
                break;

            case am_G:
                {
                    Expression::Ptr op(makeRegisterExpression(
                                makeRegisterID(locs->modrm_reg, optype, locs->rex_r)));
                    insn_to_complete->appendOperand(op, isRead, isWritten, isImplicit);
                }
                break;
            case am_L:
                /* Use Imm byte to encode XMM. Seen in FMA4*/
                 if(decodeAVX(bank, &bank_index, 
                             (*(const uint8_t*)(b.start + locs->imm_position[imm_index++])) >> 4, 
                             avx_type, pref, operand.admet))
                    return false;

                /* Append the operand */
                insn_to_complete->appendOperand(makeRegisterExpression(
                            IntelRegTable(m_Arch, bank, bank_index)),
                        isRead, isWritten, isImplicit);
                break;

            case am_H: /* Could be XMM, YMM or ZMM */
                /* Make sure this register class is valid for VEX */
                if(!AVX_TYPE_OKAY(avx_type) || !pref.vex_present)
                    return false;

                /* Grab the correct bank and bank index for this type of register */
                if(decodeAVX(bank, &bank_index, pref.vex_vvvv_reg, avx_type, pref, operand.admet))
                    return false;

                /* Append the operand */
                insn_to_complete->appendOperand(makeRegisterExpression(
                            IntelRegTable(m_Arch, bank, bank_index)),
                        isRead, isWritten, isImplicit);
                break;

            case am_HK: /* Could be XMM, YMM or ZMM */
                /* Make sure this register class is valid for VEX */
                if(!AVX_TYPE_OKAY(avx_type) || !pref.vex_present)
                    return false;

                /* Grab the correct bank and bank index for this type of register */
                if(decodeAVX(bank, &bank_index, pref.vex_vvvv_reg, avx_type, pref, operand.admet))
                    return false;

                /* Append the operand */
                insn_to_complete->appendOperand(makeRegisterExpression(
                            IntelRegTable(m_Arch, bank, bank_index)),
                        isRead, isWritten, isImplicit);
                break;

            case am_I:
                insn_to_complete->appendOperand(decodeImmediate(optype, b.start +
                            locs->imm_position[imm_index++]),
                        isRead, isWritten, isImplicit);
                break;
            case am_J:
                {
                    Expression::Ptr Offset(decodeImmediate(optype,
                                b.start + locs->imm_position[imm_index++],
                                true));
                    Expression::Ptr EIP(makeRegisterExpression(MachRegister::getPC(m_Arch)));
                    Expression::Ptr InsnSize(
                            make_shared(singleton_object_pool<Immediate>::construct(Result(u8,
                                        decodedInstruction->getSize()))));
                    Expression::Ptr postEIP(makeAddExpression(EIP, InsnSize, u32));
                    Expression::Ptr op(makeAddExpression(Offset, postEIP, u32));
                    insn_to_complete->addSuccessor(op, isCall, false, isConditional, false);
                    if (isConditional)
                        insn_to_complete->addSuccessor(postEIP, false, false, true, true);
                }
                break;
            case am_O:
                {
                    // Address/offset width, which is *not* what's encoded by the optype...
                    // The deref's width is what's actually encoded here.
                    int pseudoOpType;
                    switch(locs->address_size)
                    {
                        case 1:
                            pseudoOpType = op_b;
                            break;
                        case 2:
                            pseudoOpType = op_w;
                            break;
                        case 4:
                            pseudoOpType = op_d;
                            break;
                        case 0:
                            if(m_Arch == Arch_x86_64) {
                                if(!addrSizePrefixPresent)
                                    pseudoOpType = op_q;
                                else
                                    pseudoOpType = op_d;
                            } else {
                                pseudoOpType = op_v;
                            }
                            break;
                        default:
                            assert(!"Bad address size, should be 0, 1, 2, or 4!");
                            pseudoOpType = op_b;
                            break;
                    }

                    int offset_position = locs->opcode_position;
                    if(locs->modrm_position > offset_position && locs->modrm_operand <
                            (int)(insn_to_complete->m_Operands.size()))
                    {
                        offset_position = locs->modrm_position;
                    }

                    if(locs->sib_position > offset_position)
                    {
                        offset_position = locs->sib_position;
                    }

                    offset_position++;
                    insn_to_complete->appendOperand(makeDereferenceExpression(
                                decodeImmediate(pseudoOpType, b.start + offset_position),
                                makeSizeType(optype)), isRead, isWritten, isImplicit);
                }
                break;

            case am_P:
                insn_to_complete->appendOperand(makeRegisterExpression(
                            IntelRegTable(m_Arch,b_mm,locs->modrm_reg)),
                        isRead, isWritten, isImplicit);
                break;

            case am_Q:
                switch(locs->modrm_mod)
                {
                    // direct dereference
                    case 0x00:
                    case 0x01:
                    case 0x02:
                        insn_to_complete->appendOperand(makeModRMExpression(b, optype),
                                isRead, isWritten, isImplicit);
                        break;
                    case 0x03:
                        // use of actual register
                        insn_to_complete->appendOperand(makeRegisterExpression(
                                    IntelRegTable(m_Arch,b_mm,locs->modrm_rm)),
                                isRead, isWritten, isImplicit);
                        break;
                    default:
                        assert(!"2-bit value modrm_mod out of range");
                        break;
                };
                break;

            case am_S:
                // Segment register in modrm reg field.
                insn_to_complete->appendOperand(makeRegisterExpression(
                            IntelRegTable(m_Arch,b_segment,locs->modrm_reg)),
                        isRead, isWritten, isImplicit);
                break;
            case am_T:
                // test register in modrm reg; should only be tr6/tr7, but we'll decode any of them
                // NOTE: this only appears in deprecated opcodes
                insn_to_complete->appendOperand(makeRegisterExpression(
                            IntelRegTable(m_Arch,b_tr,locs->modrm_reg)),
                        isRead, isWritten, isImplicit);
                break;

            case am_UM:
                switch(locs->modrm_mod)
                {
                    // direct dereference
                    case 0x00:
                    case 0x01:
                    case 0x02:
                        insn_to_complete->appendOperand(
                                makeModRMExpression(b, makeSizeType(optype)),
                                isRead, isWritten, isImplicit);
                        break;
                    case 0x03:
                        // use of actual register (am_U)
                        {
                            /* Is this a vex prefixed instruction? */
                            if(pref.vex_present)
                            {
                                if(!AVX_TYPE_OKAY(avx_type))
                                    return false;
                            }

                            /* Grab the register bank and index */
                            if(decodeAVX(bank, &bank_index, locs->modrm_rm, AVX_XMM,
                                        pref, operand.admet))
                                return false;

                            /* Append the operand */
                            insn_to_complete->appendOperand(makeRegisterExpression(
                                        IntelRegTable(m_Arch, bank, bank_index)),
                                    isRead, isWritten, isImplicit);
                            break;
                        }
                    default:
                        assert(!"2-bit value modrm_mod out of range");
                        break;
                };
                break;

            case am_XH: /* Must be XMM */
                /* Make sure we are using a valid VEX register class */
                if(!AVX_TYPE_OKAY(avx_type) || !pref.vex_present)
                    return false;

                /* Constrain register type to only the XMM banks */
                avx_type = AVX_XMM;

                /* Grab the correct bank and bank index for this type of register */
                if(decodeAVX(bank, &bank_index, pref.vex_vvvv_reg, avx_type, pref, operand.admet))
                    return false;

                insn_to_complete->appendOperand(makeRegisterExpression(
                            IntelRegTable(m_Arch, bank, bank_index)),
                        isRead, isWritten, isImplicit);
                break;

            case am_YH: /* Could be XMM or YMM */
                /* Make sure we are using a valid VEX register class */
                if(!AVX_TYPE_OKAY(avx_type) || !pref.vex_present)
                    return false;

                /* Constrain to only XMM or YMM registers */
                if(avx_type != AVX_XMM && avx_type != AVX_YMM)
                    avx_type = AVX_YMM;

                /* Grab the correct bank and bank index for this type of register */
                if(decodeAVX(bank, &bank_index, pref.vex_vvvv_reg, avx_type, pref, operand.admet))
                    return false;

                /* Append the operand */
                insn_to_complete->appendOperand(makeRegisterExpression(
                            IntelRegTable(m_Arch, bank, bank_index)),
                        isRead, isWritten, isImplicit);
                break;

            case am_U: /* Could be XMM, YMM, or ZMM (or possibly non VEX)*/

                /* Is this a vex prefixed instruction? */
                if(pref.vex_present)
                {
                    if(!AVX_TYPE_OKAY(avx_type))
                        return false;
                }

                /* Grab the register bank and index */
                if(decodeAVX(bank, &bank_index, locs->modrm_rm, AVX_XMM, pref, operand.admet))
                    return false;

                /* Append the operand */
                insn_to_complete->appendOperand(makeRegisterExpression(
                            IntelRegTable(m_Arch, bank, bank_index)),
                        isRead, isWritten, isImplicit);
                break;
            case am_XU: /* Must be XMM (must be VEX) */
                /* Make sure this register class is valid */
                if(!AVX_TYPE_OKAY(avx_type) || !pref.vex_present)
                    return false;

                /* Constrain register to XMM banks only */        
                avx_type = AVX_XMM;

                /* Get the register bank and index for this register */
                if(decodeAVX(bank, &bank_index, locs->modrm_rm, avx_type, pref, operand.admet))
                    return false;

                /* Append the operand */
                insn_to_complete->appendOperand(makeRegisterExpression(
                            IntelRegTable(m_Arch, bank, bank_index)),
                        isRead, isWritten, isImplicit);
                break;
            case am_YU: /* Must be XMM or YMM (must be VEX) */
                /* Make sure this register class is valid */
                if(!AVX_TYPE_OKAY(avx_type) || !pref.vex_present)
                    return false;

                /* Constrain to either XMM or YMM registers */
                if(avx_type != AVX_XMM && avx_type != AVX_YMM)
                    avx_type = AVX_YMM;

                /* Get the register bank and index */
                if(decodeAVX(bank, &bank_index, locs->modrm_rm, avx_type, pref, operand.admet))
                    return false;

                /* Append the operand */
                insn_to_complete->appendOperand(makeRegisterExpression(
                            IntelRegTable(m_Arch, bank, bank_index)),
                        isRead, isWritten, isImplicit);
                break;
            case am_V: /* Could be XMM, YMM or ZMM (possibly non VEX)*/
                /* Is this a vex prefixed instruction? */
                if(pref.vex_present && !AVX_TYPE_OKAY(avx_type))
                    return false;

                /* Get the base register number */
                regnum = locs->modrm_reg;

                /* Get the register bank and the index */
                if(decodeAVX(bank, &bank_index, regnum, avx_type, pref, operand.admet))
                    return false;

                /* Append the operand */
                insn_to_complete->appendOperand(makeRegisterExpression(
                            IntelRegTable(m_Arch, bank, bank_index)),
                        isRead, isWritten, isImplicit);
                break;
            case am_XV: /* Must be XMM (must be VEX) */

                if(!AVX_TYPE_OKAY(avx_type) || !pref.vex_present)
                    return false;

                regnum = locs->modrm_reg;

                /* Get the register bank and the index */
                if(decodeAVX(bank, &bank_index, regnum, avx_type, pref, operand.admet))
                    return false;

                /* Append the operand */
                insn_to_complete->appendOperand(makeRegisterExpression(IntelRegTable(
                                m_Arch, bank, bank_index)), isRead, isWritten, isImplicit);
                break;
            case am_YV: /* Must be XMM or YMM (must be VEX) */
                /* Make sure this register class is valid */
                if(!AVX_TYPE_OKAY(avx_type) || !pref.vex_present)
                    return false;

                regnum = locs->modrm_reg;

                /* Constrain to either XMM or YMM registers */
                if(avx_type != AVX_XMM && avx_type != AVX_YMM)
                    avx_type = AVX_YMM;

                /* Get the register bank and index */
                if(decodeAVX(bank, &bank_index, regnum, avx_type, pref, operand.admet))
                    return false;

                /* Append the operand */
                insn_to_complete->appendOperand(makeRegisterExpression(
                            IntelRegTable(m_Arch, bank, bank_index)),
                        isRead, isWritten, isImplicit);
                break;
            case am_VK: /* A KMasking register defined in the reg of a Mod/RM*/
                /* Is this a vex prefixed instruction? */
                if(pref.vex_present && !AVX_TYPE_OKAY(avx_type))
                    return false;

                /* Get the base register number */
                regnum = locs->modrm_reg;

                /* Get the register bank and the index */
                if(decodeAVX(bank, &bank_index, regnum, avx_type, pref, operand.admet))
                    return false;

                /* Append the operand */
                insn_to_complete->appendOperand(makeRegisterExpression(
                            IntelRegTable(m_Arch, bank, bank_index)),
                        isRead, isWritten, isImplicit);
                break;
            case am_WK: /* Could be a K mask register or memory address*/
            case am_W: /* Could be XMM, YMM, or ZMM (or possibly not VEX) */

                if(pref.vex_present)
                {
                    if(!AVX_TYPE_OKAY(avx_type))
                        return false;
                }

                // if(operand.admet == am_WK)
                // printf("modrm_mod: %d modrm_reg: %d  modrm_rm: %d\n", 
                // locs->modrm_mod, locs->modrm_reg, locs->modrm_rm);

                switch(locs->modrm_mod)
                {
                    /* Direct dereference */
                    case 0x00:
                    case 0x01:
                    case 0x02:
                        insn_to_complete->appendOperand(
                                makeModRMExpression(b, makeSizeType(optype)),
                                isRead, isWritten, isImplicit);
                        break;
                    case 0x03:
                        /* Just the register is used */
                        if(decodeAVX(bank, &bank_index, locs->modrm_rm,
                                    avx_type, pref, operand.admet))
                            return false;

                        insn_to_complete->appendOperand(
                                makeRegisterExpression(IntelRegTable(
                                        m_Arch, bank, bank_index)),
                                isRead, isWritten, isImplicit);
                        break;
                    default:
                        assert(!"2-bit value modrm_mod out of range");
                        break;
                }
                break;
            case am_XW: /* Must be XMM (must be VEX) */

                /* Make sure this vex is okay */
                if(!AVX_TYPE_OKAY(avx_type) || !pref.vex_present)
                    return false;

                /* Constrain to the XMM banks */ 
                avx_type = AVX_XMM;

                switch(locs->modrm_mod)
                {
                    /* Direct dereference */
                    case 0x00:
                    case 0x01:
                    case 0x02:
                        insn_to_complete->appendOperand(makeModRMExpression(b,
                                    makeSizeType(optype)),
                                isRead, isWritten, isImplicit);
                        break;
                    case 0x03:
                        /* Just the register is used */
                        if(decodeAVX(bank, &bank_index, locs->modrm_rm, avx_type, pref, operand.admet))
                            return false;
                        insn_to_complete->appendOperand(
                                makeRegisterExpression(IntelRegTable
                                    (m_Arch, bank, bank_index)),
                                isRead, isWritten, isImplicit);
                        break;
                    default:
                        assert(!"2-bit value modrm_mod out of range");
                        break;
                }
                break;
            case am_YW: /* Must be either YMM or XMM (must be VEX) */

                /* Make sure the register class is okay and we have a vex prefix */
                if(!AVX_TYPE_OKAY(avx_type) || !pref.vex_present)
                    return false;

                /* Constrain to either XMM or YMM registers */
                if(avx_type != AVX_XMM && avx_type != AVX_YMM)
                    avx_type = AVX_YMM;

                switch(locs->modrm_mod)
                {
                    /* Direct dereference */
                    case 0x00:
                    case 0x01:
                    case 0x02:
                        insn_to_complete->appendOperand(makeModRMExpression(
                                    b, makeSizeType(optype)),
                                isRead, isWritten, isImplicit);
                        break;
                    case 0x03:
                        /* Just the register is used */
                        if(decodeAVX(bank, &bank_index, locs->modrm_rm, avx_type, pref, operand.admet))
                            return false;

                        /* Append the operand */
                        insn_to_complete->appendOperand(makeRegisterExpression(
                                    IntelRegTable(m_Arch, bank, bank_index)),
                                isRead, isWritten, isImplicit);
                        break;
                    default:
                        assert(!"2-bit value modrm_mod out of range");
                        break;
                }
                break;
            case am_X:
                {
                    MachRegister si_reg;
                    if(m_Arch == Arch_x86)
                    {
                        if(addrSizePrefixPresent)
                        {
                            si_reg = x86::si;
                        } else {
                            si_reg = x86::esi;
                        }
                    } else {
                        if(addrSizePrefixPresent)
                        {
                            si_reg = x86_64::esi;
                        } else {
                            si_reg = x86_64::rsi;
                        }
                    }

                    Expression::Ptr ds(makeRegisterExpression(
                                m_Arch == Arch_x86 ? x86::ds : x86_64::ds));
                    Expression::Ptr si(makeRegisterExpression(si_reg));
                    Expression::Ptr segmentOffset(make_shared
                            (singleton_object_pool<Immediate>::construct(Result(u32, 0x10))));
                    Expression::Ptr ds_segment = makeMultiplyExpression(
                            ds, segmentOffset, u32);
                    Expression::Ptr ds_si = makeAddExpression(ds_segment, si, u32);
                    insn_to_complete->appendOperand(
                            makeDereferenceExpression(ds_si, makeSizeType(optype)),
                            isRead, isWritten, isImplicit);
                }
                break;
            case am_Y:
                {
                    MachRegister di_reg;
                    if(m_Arch == Arch_x86)
                    {
                        if(addrSizePrefixPresent)
                        {
                            di_reg = x86::di;
                        } else {
                            di_reg = x86::edi;
                        }
                    } else {
                        if(addrSizePrefixPresent)
                        {
                            di_reg = x86_64::edi;
                        } else {
                            di_reg = x86_64::rdi;
                        }
                    }

                    Expression::Ptr es(makeRegisterExpression(
                                m_Arch == Arch_x86 ? x86::es : x86_64::es));
                    Expression::Ptr di(makeRegisterExpression(di_reg));

                    Immediate::Ptr imm(make_shared(
                                singleton_object_pool<Immediate>::construct(Result(u32, 0x10))));
                    Expression::Ptr es_segment(
                            makeMultiplyExpression(es,imm, u32));
                    Expression::Ptr es_di(makeAddExpression(es_segment, di, u32));
                    insn_to_complete->appendOperand(
                            makeDereferenceExpression(es_di, makeSizeType(optype)),
                            isRead, isWritten, isImplicit);

                }
                break;
            case am_tworeghack:
                if(optype == op_edxeax)
                {
                    Expression::Ptr edx(makeRegisterExpression(m_Arch == Arch_x86 ? x86::edx : x86_64::edx));
                    Expression::Ptr eax(makeRegisterExpression(m_Arch == Arch_x86 ? x86::eax : x86_64::eax));
                    Expression::Ptr highAddr = makeMultiplyExpression(edx, Immediate::makeImmediate(Result(u64, 1LL << 32)), u64);
                    Expression::Ptr addr = makeAddExpression(highAddr, eax, u64);
                    Expression::Ptr op = makeDereferenceExpression(addr, u64);
                    insn_to_complete->appendOperand(op, isRead, isWritten, isImplicit);
                } else if (optype == op_ecxebx)
                {
                    Expression::Ptr ecx(makeRegisterExpression(m_Arch == Arch_x86 ? x86::ecx : x86_64::ecx));
                    Expression::Ptr ebx(makeRegisterExpression(m_Arch == Arch_x86 ? x86::ebx : x86_64::ebx));
                    Expression::Ptr highAddr = makeMultiplyExpression(ecx,
                            Immediate::makeImmediate(Result(u64, 1LL << 32)), u64);
                    Expression::Ptr addr = makeAddExpression(highAddr, ebx, u64);
                    Expression::Ptr op = makeDereferenceExpression(addr, u64);
                    insn_to_complete->appendOperand(op, isRead, isWritten, isImplicit);
                }
                break;

            case am_reg:
                {
                    MachRegister r(optype);
                    int size = r.size();
                    if((m_Arch == Arch_x86_64) && (r.regClass() == (unsigned int)x86::GPR) && (size == 4))
                    {
                        int reg_size = isDefault64Insn() ? op_q : op_v;
                        if(sizePrefixPresent)
                        {
                            reg_size = op_w;
                        }
                        // implicit regs are not extended
                        r = makeRegisterID((r.val() & 0xFF), reg_size, false);
                        entryID entryid = decodedInstruction->getEntry()->getID(locs);
                        if(locs->rex_b && insn_to_complete->m_Operands.empty() &&
                                (entryid == e_push || entryid == e_pop || entryid == e_xchg || ((*(b.start + locs->opcode_position) & 0xf0) == 0xb0)))
                        {
                            r = MachRegister((r.val()) | x86_64::r8.val());
                            assert(r.name() != "<INVALID_REG>");
                        }
                    } else {
                        r = MachRegister((r.val() & ~r.getArchitecture()) | m_Arch);

                        entryID entryid = decodedInstruction->getEntry()->getID(locs);
                        if(insn_to_complete->m_Operands.empty() &&
                                (entryid == e_push || entryid == e_pop || entryid == e_xchg || ((*(b.start + locs->opcode_position) & 0xf0) == 0xb0) ) )
                        {
                            unsigned int opcode_byte = *(b.start+locs->opcode_position);
                            unsigned int reg_id = (opcode_byte & 0x07);
                            if(locs->rex_b)
                            {
                                // FP stack registers are not affected by the rex_b bit in AM_REG.
                                if(r.regClass() == (unsigned) x86::GPR)
                                {
                                    int reg_op_type = op_d;
                                    switch(size)
                                    {
                                        case 1:
                                            reg_op_type = op_b;
                                            break;
                                        case 2:
                                            reg_op_type = op_w;
                                            break;
                                        case 8:
                                            reg_op_type = op_q;
                                            break;
                                        default:
                                            break;
                                    }

                                    r = makeRegisterID(reg_id, reg_op_type, true);
                                    assert(r.name() != "<INVALID_REG>");
                                }
                            } else if((r.size() == 1) && (locs->rex_byte & 0x40))
                            {
                                r = makeRegisterID(reg_id, op_b, false);
                                assert(r.name() != "<INVALID_REG>");
                            }
                        }

                        if(sizePrefixPresent && (r.regClass() == (unsigned int)x86::GPR) && r.size() >= 4)
                        {
                            r = MachRegister((r.val() & ~x86::FULL) | x86::W_REG);
                            assert(r.name() != "<INVALID_REG>");
                        }
                    }
                    Expression::Ptr op(makeRegisterExpression(r));
                    insn_to_complete->appendOperand(op, isRead, isWritten, isImplicit);
                }
                break;
            case am_stackH:
            case am_stackP:
                // handled elsewhere
                break;
            case am_allgprs:
                if(m_Arch == Arch_x86)
                {
                    insn_to_complete->appendOperand(makeRegisterExpression(x86::eax), isRead, isWritten, isImplicit);
                    insn_to_complete->appendOperand(makeRegisterExpression(x86::ecx), isRead, isWritten, isImplicit);
                    insn_to_complete->appendOperand(makeRegisterExpression(x86::edx), isRead, isWritten, isImplicit);
                    insn_to_complete->appendOperand(makeRegisterExpression(x86::ebx), isRead, isWritten, isImplicit);
                    insn_to_complete->appendOperand(makeRegisterExpression(x86::esp), isRead, isWritten, isImplicit);
                    insn_to_complete->appendOperand(makeRegisterExpression(x86::ebp), isRead, isWritten, isImplicit);
                    insn_to_complete->appendOperand(makeRegisterExpression(x86::esi), isRead, isWritten, isImplicit);
                    insn_to_complete->appendOperand(makeRegisterExpression(x86::edi), isRead, isWritten, isImplicit);
                } else {
                    insn_to_complete->appendOperand(makeRegisterExpression(x86_64::eax), isRead, isWritten, isImplicit);
                    insn_to_complete->appendOperand(makeRegisterExpression(x86_64::ecx), isRead, isWritten, isImplicit);
                    insn_to_complete->appendOperand(makeRegisterExpression(x86_64::edx), isRead, isWritten, isImplicit);
                    insn_to_complete->appendOperand(makeRegisterExpression(x86_64::ebx), isRead, isWritten, isImplicit);
                    insn_to_complete->appendOperand(makeRegisterExpression(x86_64::esp), isRead, isWritten, isImplicit);
                    insn_to_complete->appendOperand(makeRegisterExpression(x86_64::ebp), isRead, isWritten, isImplicit);
                    insn_to_complete->appendOperand(makeRegisterExpression(x86_64::esi), isRead, isWritten, isImplicit);
                    insn_to_complete->appendOperand(makeRegisterExpression(x86_64::edi), isRead, isWritten, isImplicit);
                }
                break;
            case am_ImplImm:
                insn_to_complete->appendOperand(Immediate::makeImmediate(Result(makeSizeType(optype), 1)), isRead, isWritten, isImplicit);
                break;
            default:
                printf("decodeOneOperand() called with unknown addressing method %u\n", operand.admet);
                // assert(0);
                return false;
        }

        return true;
    }

    void InstructionDecoder_x86::doIA32Decode(InstructionDecoder::buffer& b)
    {
        if(decodedInstruction == NULL)
        {
            decodedInstruction = reinterpret_cast<ia32_instruction*>(malloc(sizeof(ia32_instruction)));
            assert(decodedInstruction);
        }
        if(locs == NULL)
        {
            locs = reinterpret_cast<ia32_locations*>(malloc(sizeof(ia32_locations)));
            assert(locs);
        }
        locs = new(locs) ia32_locations; //reinit();
        assert(locs->sib_position == -1);
        decodedInstruction = new (decodedInstruction) ia32_instruction(NULL, NULL, locs);
        ia32_decode(IA32_DECODE_PREFIXES, b.start, *decodedInstruction, is64BitMode);

        static ia32_entry invalid = { e_No_Entry, 0, 0, false, { {0,0}, {0,0}, {0,0} }, 0, 0, 0 };
        if(decodedInstruction->getLegacyType() == ILLEGAL) {
        	m_Operation = Operation(&invalid, nullptr, nullptr, m_Arch);
        	return;
        }

        sizePrefixPresent = (decodedInstruction->getPrefix()->getOperSzPrefix() == 0x66);
        if (decodedInstruction->getPrefix()->rexW()) {
            // as per 2.2.1.2 - rex.w overrides 66h
            sizePrefixPresent = false;
        }
        addrSizePrefixPresent = (decodedInstruction->getPrefix()->getAddrSzPrefix() == 0x67);

        if(decodedInstruction->getEntry()) {
            // check prefix validity
            // lock prefix only allowed on certain insns.
            // TODO: refine further to check memory written operand
            if(decodedInstruction->getPrefix()->getPrefix(0) == PREFIX_LOCK)
            {
                switch(decodedInstruction->getEntry()->id)
                {
                    case e_add:
                    case e_adc:
                    case e_and:
                    case e_btc:
                    case e_btr:
                    case e_bts:
                    case e_cmpxchg:
                    case e_cmpxchg8b:
                    case e_dec:
                    case e_inc:
                    case e_neg:
                    case e_not:
                    case e_or:
                    case e_sbb:
                    case e_sub:
                    case e_xor:
                    case e_xadd:
                    case e_xchg:
                        break;
                    default:
                        m_Operation = Operation(&invalid,
                                    decodedInstruction->getPrefix(), locs, m_Arch);
                        return;
                }
            } else if (decodedInstruction->getPrefix()->getPrefix(0) == PREFIX_REP &&
                        *(b.start+1) == (unsigned char)(0x0F) && *(b.start+2) == (unsigned char)(0x1E)) {
                // handling ENDBR family
                if (*(b.start+3) == (unsigned char)(0xFB)) {
                    m_Operation = Operation(e_endbr32, entryNames_IAPI[e_endbr32], m_Arch);
                    return;
                } else if (*(b.start+3) == (unsigned char)(0xFA)) {
                    m_Operation = Operation(e_endbr64, entryNames_IAPI[e_endbr64], m_Arch);
                    return;
                }
            } 
            m_Operation = Operation(decodedInstruction->getEntry(),
                        decodedInstruction->getPrefix(), locs, m_Arch);

        } else {
            // Gap parsing can trigger this case; in particular, when it encounters prefixes in an invalid order.
            // Notably, if a REX prefix (0x40-0x48) appears followed by another prefix (0x66, 0x67, etc)
            // we'll reject the instruction as invalid and send it back with no entry.  Since this is a common
            // byte sequence to see in, for example, ASCII strings, we want to simply accept this and move on, not
            // yell at the user.
            m_Operation = Operation(&invalid,
                        decodedInstruction->getPrefix(), locs, m_Arch);
        }

    }
    
    void InstructionDecoder_x86::decodeOpcode(InstructionDecoder::buffer& b)
    {
        doIA32Decode(b);

        // Do not move through the buffer if a bad instruction was encountered
        if(m_Operation.getID() == e_No_Entry) return;

        b.start += decodedInstruction->getSize();
    }
    
	bool InstructionDecoder_x86::decodeOperands(const Instruction* insn_to_complete)
    {
        int imm_index = 0; // handle multiple immediate operands
        if(!decodedInstruction || !decodedInstruction->getEntry()) return false;
        unsigned int opsema = decodedInstruction->getEntry()->opsema;
        unsigned int semantics = opsema & 0xFF;
        unsigned int implicit_operands =
            sGetImplicitOPs(decodedInstruction->getEntry()->impl_dec);
        InstructionDecoder::buffer b(insn_to_complete->ptr(), insn_to_complete->size());

        if (decodedInstruction->getEntry()->getID() == e_ret_near ||
            decodedInstruction->getEntry()->getID() == e_ret_far) {
           Expression::Ptr ret_addr = makeDereferenceExpression(makeRegisterExpression(is64BitMode ? x86_64::rsp : x86::esp),
                                                                is64BitMode ? u64 : u32);
           insn_to_complete->addSuccessor(ret_addr, false, true, false, false, true);
	    }
        if (insn_to_complete->getOperation().getID() == e_endbr32 ||
            insn_to_complete->getOperation().getID() == e_endbr64) {
            insn_to_complete->m_Operands.clear();
            return true;
        }

        for(int i = 0; i < 3; i++)
        {
            if(decodedInstruction->getEntry()->operands[i].admet == 0 && 
                    decodedInstruction->getEntry()->operands[i].optype == 0)
                break;

            if(!decodeOneOperand(b,
                        decodedInstruction->getEntry()->operands[i],
                        imm_index,
                        insn_to_complete,
                        readsOperand(semantics, i),
                        writesOperand(semantics, i),
                        implicitOperand(implicit_operands, i)))
            {
                return false;
            }
        }

        /* Does this instruction have a 4th operand? */
        if(semantics >= s4OP)
        {
            if (decodedInstruction->getEntry()->operands[3].admet != 0 ||
                decodedInstruction->getEntry()->operands[3].optype != 0) {
                // Special handling for FMA4 instructions
              if(!decodeOneOperand(b,
                          decodedInstruction->getEntry()->operands[3],
                          imm_index,
                          insn_to_complete,
                          readsOperand(semantics, 3),
                          writesOperand(semantics, 3),
                          implicitOperand(implicit_operands, 3)))
              {
                  return false;
              }
            } else if(!decodeOneOperand(b,
                        {am_I, op_b}, /* This is always an IMM8 */
                        imm_index,
                        insn_to_complete,
                        readsOperand(semantics, 3),
                        writesOperand(semantics, 3),
                        implicitOperand(implicit_operands, 3)))
            {
                return false;
            }
        }

        ia32_prefixes& pref = *decodedInstruction->getPrefix();
        /* Is this an EVEX prefixed instruction? */
        if(pref.vex_type == VEX_TYPE_EVEX)
        {
            insn_to_complete->appendOperand(makeMaskRegisterExpression(
                        IntelRegTable(m_Arch, b_kmask, pref.vex_aaa)), 
                    true, false);
        }

        return true;
    }

    
      INSTRUCTION_EXPORT Instruction InstructionDecoder_x86::decode(InstructionDecoder::buffer& b)
    {
        return InstructionDecoderImpl::decode(b);
    }
    void InstructionDecoder_x86::doDelayedDecode(const Instruction* insn_to_complete)
    {
      InstructionDecoder::buffer b(insn_to_complete->ptr(), insn_to_complete->size());
      //insn_to_complete->m_Operands.reserve(4);
      doIA32Decode(b);        
      decodeOperands(insn_to_complete);
    }
    
}
}

