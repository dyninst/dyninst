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

#include "common/src/Types.h"
#include "InstructionDecoder-x86.h"
#include "Expression.h"
#include "common/src/arch-x86.h"
#include "Register.h"
#include "Dereference.h"
#include "Immediate.h" 
#include "BinaryFunction.h"
#include "common/src/singleton_object_pool.h"

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
                case s1RW2R3R: // shld/shrd
                case s1RW2RW3R: // [i]div, cmpxch8b
                case s1R2R3R:
                    return i == 0 || i == 1 || i == 2;
                    break;
                case sNONE:
                default:
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
                case sNONE:
                default:
                    return false;
            }
        }


    
    INSTRUCTION_EXPORT InstructionDecoder_x86::InstructionDecoder_x86(Architecture a) :
      InstructionDecoderImpl(a),
    locs(NULL),
    decodedInstruction(NULL),
    sizePrefixPresent(false),
    addrSizePrefixPresent(false)
    {
      if(a == Arch_x86_64) setMode(true);
      
    }
    INSTRUCTION_EXPORT InstructionDecoder_x86::~InstructionDecoder_x86()
    {
        if(decodedInstruction) decodedInstruction->~ia32_instruction();
        free(decodedInstruction);
        if(locs) locs->~ia32_locations();
        free(locs);

    }
    static const unsigned char modrm_use_sib = 4;
    
    INSTRUCTION_EXPORT void InstructionDecoder_x86::setMode(bool is64)
    {
        ia32_set_mode_64(is64);
    }
    
      Expression::Ptr InstructionDecoder_x86::makeSIBExpression(const InstructionDecoder::buffer& b)
    {
        unsigned scale;
        Register index;
        Register base;
        Result_Type registerType = ia32_is_mode_64() ? u64 : u32;

        decode_SIB(locs->sib_byte, scale, index, base);

        Expression::Ptr scaleAST(make_shared(singleton_object_pool<Immediate>::construct(Result(u8, dword_t(scale)))));
        Expression::Ptr indexAST(make_shared(singleton_object_pool<RegisterAST>::construct(makeRegisterID(index, registerType,
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
											       registerType,
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
											       registerType,
											       locs->rex_b)));
        }

        if(index == 0x04 && (!(ia32_is_mode_64()) || !(locs->rex_x)))
        {
            return baseAST;
        }
        return makeAddExpression(baseAST, makeMultiplyExpression(indexAST, scaleAST, registerType), registerType);
    }

      Expression::Ptr InstructionDecoder_x86::makeModRMExpression(const InstructionDecoder::buffer& b,
								  unsigned int opType)
    {
       unsigned int regType = op_d;
        Result_Type aw = ia32_is_mode_64() ? u32 : u64;
        if(ia32_is_mode_64())
        {
            regType = op_q;
        }
        Expression::Ptr e =
            makeRegisterExpression(makeRegisterID(locs->modrm_rm, regType, locs->rex_b));
        switch(locs->modrm_mod)
        {
            case 0:
                if(locs->modrm_rm == modrm_use_sib) {
                    e = makeSIBExpression(b);
                }
                if(locs->modrm_rm == 0x5 && !addrSizePrefixPresent)
                {
                    assert(locs->opcode_position > -1);
                    if(ia32_is_mode_64())
                    {
                        e = makeAddExpression(makeRegisterExpression(x86_64::rip),
                                            getModRMDisplacement(b), aw);
                    }
                    else
                    {
                        e = getModRMDisplacement(b);
                    }
        
                }
                if(locs->modrm_rm == 0x6 && addrSizePrefixPresent)
                {
                    e = getModRMDisplacement(b);
                }
                if(opType == op_lea)
                {
                    return e;
                }
                return makeDereferenceExpression(e, makeSizeType(opType));
                assert(0);
                break;
            case 1:
            case 2:
            {
                if(locs->modrm_rm == modrm_use_sib) {
                    e = makeSIBExpression(b);
                }
                Expression::Ptr disp_e = makeAddExpression(e, getModRMDisplacement(b), aw);
                if(opType == op_lea)
                {
                    return disp_e;
                }
                return makeDereferenceExpression(disp_e, makeSizeType(opType));
            }
            assert(0);
            break;
            case 3:
                return makeRegisterExpression(makeRegisterID(locs->modrm_rm, opType, locs->rex_b));
            default:
                return Expression::Ptr();
        
        };
        // can't get here, but make the compiler happy...
        assert(0);
        return Expression::Ptr();
    }

    Expression::Ptr InstructionDecoder_x86::decodeImmediate(unsigned int opType, const unsigned char* immStart, 
							    bool isSigned)
    {
#if 0
        /* See "2.2.1.5 Immediates" from the Intel Manual" */
        bool is_64 = ia32_is_mode_64();
        fprintf(stderr, "decodeImmediate: ia32_is_mode_64 returned %s\n", (is_64 ? "true" : "false"));
        if (is_64) {
            fprintf(stderr, "setting isSigned to true (was %s)\n", (isSigned ? "true" : "false"));
            isSigned = true;
        }
#endif

        // rex_w indicates we need to sign-extend also.
        isSigned = isSigned || locs->rex_w;

        switch(opType)
        {
            case op_b:
                return Immediate::makeImmediate(Result(isSigned ? s8 : u8 ,*(const byte_t*)(immStart)));
                break;
            case op_d:
                return Immediate::makeImmediate(Result(isSigned ? s32 : u32,*(const dword_t*)(immStart)));
            case op_w:
                return Immediate::makeImmediate(Result(isSigned ? s16 : u16,*(const word_t*)(immStart)));
                break;
            case op_q:
                return Immediate::makeImmediate(Result(isSigned ? s64 : u64,*(const int64_t*)(immStart)));
                break;
            case op_v:
                if (locs->rex_w) {
                    /* Check with valgrind--if uninit reads go away--this was probably wrong (was 64, change to 32) */
//                    return Immediate::makeImmediate(Result(isSigned ? s64 : u64,*(const int32_t*)(immStart))); // TODO: signed, read the first 32
                    return Immediate::makeImmediate(Result(isSigned ? s64 : u64,*(const int64_t*)(immStart)));
                }
                //FALLTHROUGH
            case op_z:
                // 32 bit mode & no prefix, or 16 bit mode & prefix => 32 bit
                // 16 bit mode, no prefix or 32 bit mode, prefix => 16 bit
                if(!sizePrefixPresent)
                {
                    return Immediate::makeImmediate(Result(isSigned ? s32 : u32,*(const dword_t*)(immStart)));
                }
                else
                {
                    return Immediate::makeImmediate(Result(isSigned ? s16 : u16,*(const word_t*)(immStart)));
                }
                break;
            case op_p:
                // 32 bit mode & no prefix, or 16 bit mode & prefix => 48 bit
                // 16 bit mode, no prefix or 32 bit mode, prefix => 32 bit
                if(!sizePrefixPresent)
                {
                    return Immediate::makeImmediate(Result(isSigned ? s48 : u48,*(const int64_t*)(immStart)));
                }
                else
                {
                    return Immediate::makeImmediate(Result(isSigned ? s32 : u32,*(const dword_t*)(immStart)));
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
        {
            disp_pos = locs->sib_position + 1;
        }
        else
        {
            disp_pos = locs->modrm_position + 1;
        }
        switch(locs->modrm_mod)
        {
            case 1:
                return make_shared(singleton_object_pool<Immediate>::construct(Result(s8, (*(const byte_t*)(b.start +
                        disp_pos)))));
                break;
            case 2:
                if(sizePrefixPresent)
                {
                    return make_shared(singleton_object_pool<Immediate>::construct(Result(s16, *((const word_t*)(b.start +
                            disp_pos)))));
                }
                else
                {
                    return make_shared(singleton_object_pool<Immediate>::construct(Result(s32, *((const dword_t*)(b.start +
                            disp_pos)))));
                }
                break;
            case 0:
                // In 16-bit mode, the word displacement is modrm r/m 6
                if(sizePrefixPresent)
                {
                    if(locs->modrm_rm == 6)
                    {
                        return make_shared(singleton_object_pool<Immediate>::construct(Result(s16,
                                           *((const dword_t*)(b.start + disp_pos)))));
                    }
                    // TODO FIXME; this was decoding wrong, but I'm not sure
                    // why...
                    else if (locs->modrm_rm == 5) {
                        assert(b.start + disp_pos + 4 <= b.end);
                        return make_shared(singleton_object_pool<Immediate>::construct(Result(s32,
                                           *((const dword_t*)(b.start + disp_pos)))));
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
                        assert(b.start + disp_pos + 4 <= b.end);
                        return make_shared(singleton_object_pool<Immediate>::construct(Result(s32,
                                           *((const dword_t*)(b.start + disp_pos)))));
                    }
                    else
                    {
                        assert(b.start + disp_pos + 1 <= b.end);
                        return make_shared(singleton_object_pool<Immediate>::construct(Result(s8, 0)));
                    }
                    break;
                }
            default:
                assert(b.start + disp_pos + 1 <= b.end);
                return make_shared(singleton_object_pool<Immediate>::construct(Result(s8, 0)));
                break;
        }
    }

    enum intelRegBanks
    {
        b_8bitNoREX = 0,
        b_16bit,
        b_32bit,
        b_segment,
        b_64bit,
        b_xmm,
        b_xmmhigh,
        b_mm,
        b_cr,
        b_dr,
        b_tr,
        b_amd64ext,
        b_8bitWithREX,
        b_fpstack,
	amd64_ext_8,
	amd64_ext_16,
	amd64_ext_32
    };
    static MachRegister IntelRegTable32[][8] = {
        {
            x86::al, x86::cl, x86::dl, x86::bl, x86::ah, x86::ch, x86::dh, x86::bh
        },
        {
            x86::ax, x86::cx, x86::dx, x86::bx, x86::sp, x86::bp, x86::si, x86::di
        },
        {
            x86::eax, x86::ecx, x86::edx, x86::ebx, x86::esp, x86::ebp, x86::esi, x86::edi
        },
        {
           x86::es, x86::cs, x86::ss, x86::ds, x86::fs, x86::gs, InvalidReg, InvalidReg
        },
        {
            x86_64::rax, x86_64::rcx, x86_64::rdx, x86_64::rbx, x86_64::rsp, x86_64::rbp, x86_64::rsi, x86_64::rdi
        },
        {
            x86::xmm0, x86::xmm1, x86::xmm2, x86::xmm3, x86::xmm4, x86::xmm5, x86::xmm6, x86::xmm7
        },
        {
            x86_64::xmm8, x86_64::xmm9, x86_64::xmm10, x86_64::xmm11, x86_64::xmm12, x86_64::xmm13, x86_64::xmm14, x86_64::xmm15
        },
        {
            x86::mm0, x86::mm1, x86::mm2, x86::mm3, x86::mm4, x86::mm5, x86::mm6, x86::mm7
        },
        {
            x86::cr0, x86::cr1, x86::cr2, x86::cr3, x86::cr4, x86::cr5, x86::cr6, x86::cr7
        },
        {
            x86::dr0, x86::dr1, x86::dr2, x86::dr3, x86::dr4, x86::dr5, x86::dr6, x86::dr7
        },
        {
            x86::tr0, x86::tr1, x86::tr2, x86::tr3, x86::tr4, x86::tr5, x86::tr6, x86::tr7
        },
        {
            x86_64::r8, x86_64::r9, x86_64::r10, x86_64::r11, x86_64::r12, x86_64::r13, x86_64::r14, x86_64::r15
        },
        {
            x86_64::al, x86_64::cl, x86_64::dl, x86_64::bl, x86_64::spl, x86_64::bpl, x86_64::sil, x86_64::dil
        },
        {
            x86::st0, x86::st1, x86::st2, x86::st3, x86::st4, x86::st5, x86::st6, x86::st7
        }

    };
    static MachRegister IntelRegTable64[][8] = {
        {
            x86_64::al, x86_64::cl, x86_64::dl, x86_64::bl, x86_64::ah, x86_64::ch, x86_64::dh, x86_64::bh
        },
        {
            x86_64::ax, x86_64::cx, x86_64::dx, x86_64::bx, x86_64::sp, x86_64::bp, x86_64::si, x86_64::di
        },
        {
            x86_64::eax, x86_64::ecx, x86_64::edx, x86_64::ebx, x86_64::esp, x86_64::ebp, x86_64::esi, x86_64::edi
        },
        {
            x86_64::es, x86_64::cs, x86_64::ss, x86_64::ds, x86_64::fs, x86_64::gs, InvalidReg, InvalidReg
        },
        {
            x86_64::rax, x86_64::rcx, x86_64::rdx, x86_64::rbx, x86_64::rsp, x86_64::rbp, x86_64::rsi, x86_64::rdi
        },
        {
            x86_64::xmm0, x86_64::xmm1, x86_64::xmm2, x86_64::xmm3, x86_64::xmm4, x86_64::xmm5, x86_64::xmm6, x86_64::xmm7
        },
        {
            x86_64::xmm8, x86_64::xmm9, x86_64::xmm10, x86_64::xmm11, x86_64::xmm12, x86_64::xmm13, x86_64::xmm14, x86_64::xmm15
        },
        {
            x86_64::mm0, x86_64::mm1, x86_64::mm2, x86_64::mm3, x86_64::mm4, x86_64::mm5, x86_64::mm6, x86_64::mm7
        },
        {
            x86_64::cr0, x86_64::cr1, x86_64::cr2, x86_64::cr3, x86_64::cr4, x86_64::cr5, x86_64::cr6, x86_64::cr7
        },
        {
            x86_64::dr0, x86_64::dr1, x86_64::dr2, x86_64::dr3, x86_64::dr4, x86_64::dr5, x86_64::dr6, x86_64::dr7
        },
        {
            x86_64::tr0, x86_64::tr1, x86_64::tr2, x86_64::tr3, x86_64::tr4, x86_64::tr5, x86_64::tr6, x86_64::tr7
        },
        {
            x86_64::r8, x86_64::r9, x86_64::r10, x86_64::r11, x86_64::r12, x86_64::r13, x86_64::r14, x86_64::r15
        },
        {
            x86_64::al, x86_64::cl, x86_64::dl, x86_64::bl, x86_64::spl, x86_64::bpl, x86_64::sil, x86_64::dil
        },
        {
            x86_64::st0, x86_64::st1, x86_64::st2, x86_64::st3, x86_64::st4, x86_64::st5, x86_64::st6, x86_64::st7
        },
	{
	    x86_64::r8b, x86_64::r9b, x86_64::r10b, x86_64::r11b, x86_64::r12b, x86_64::r13b, x86_64::r14b, x86_64::r15b 
	},
	{
	    x86_64::r8w, x86_64::r9w, x86_64::r10w, x86_64::r11w, x86_64::r12w, x86_64::r13w, x86_64::r14w, x86_64::r15w 
	},
	{
	    x86_64::r8d, x86_64::r9d, x86_64::r10d, x86_64::r11d, x86_64::r12d, x86_64::r13d, x86_64::r14d, x86_64::r15d 
	}

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
	switch(m_Operation->getID())
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
		    else
		        retVal = IntelRegTable(m_Arch, amd64_ext_16, intelReg);
		    break;	
		case op_p:
		case op_z:
		    if (!sizePrefixPresent)
		        retVal = IntelRegTable(m_Arch, amd64_ext_32, intelReg);
		    else
		        retVal = IntelRegTable(m_Arch, amd64_ext_16, intelReg);
		    break;
		default:
		    fprintf(stderr, "%d\n", opType);
		    fprintf(stderr, "%s\n",  decodedInstruction->getEntry()->name(locs));
		    assert(0 && "opType=" && opType);
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
                    if (locs->rex_position == -1) {
                        retVal = IntelRegTable(m_Arch,b_8bitNoREX,intelReg);
                    } else {
                        retVal = IntelRegTable(m_Arch,b_8bitWithREX,intelReg);
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

        if (!ia32_is_mode_64()) {
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
	      if(ia32_is_mode_64() || !sizePrefixPresent)
                {
                    return u32;
                }
                else
                {
                    return u16;
                }
                break;
            case op_y:
            	if(ia32_is_mode_64())
            		return u64;
            	else
            		return u32;
            	break;
            case op_p:
                // book says operand size; arch-x86 says word + word * operand size
                if(!ia32_is_mode_64() ^ sizePrefixPresent)
                {
                    return u48;
                }
                else
                {
                    return u32;
                }
            case op_dq:
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


    bool InstructionDecoder_x86::decodeOneOperand(const InstructionDecoder::buffer& b,
						  const ia32_operand& operand,
						  int & imm_index, /* immediate operand index */
						  const Instruction* insn_to_complete, 
						  bool isRead, bool isWritten)
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
      if (cat == c_BranchInsn && insn_to_complete->getOperation().getID() != e_jmp) {
	isConditional = true;
      }

      unsigned int optype = operand.optype;
      if (sizePrefixPresent && 
	  ((optype == op_v) ||
	   (optype == op_z))) {
	optype = op_w;
      }
      if(optype == op_y) {
    	  if(ia32_is_mode_64() && locs->rex_w)
    		  optype = op_q;
    	  else
    		  optype = op_d;
      }
                switch(operand.admet)
                {
                    case 0:
                    // No operand
                    {
/*                        fprintf(stderr, "ERROR: Instruction with mismatched operands. Raw bytes: ");
                        for(unsigned int i = 0; i < decodedInstruction->getSize(); i++) {
                            fprintf(stderr, "%x ", b.start[i]);
                        }
                        fprintf(stderr, "\n");*/
                        assert(!"Mismatched number of operands--check tables");
                        return false;
                    }
                    case am_A:
                    {
                        // am_A only shows up as a far call/jump.  Position 1 should be universally safe.
                        Expression::Ptr addr(decodeImmediate(optype, b.start + 1));
                        insn_to_complete->addSuccessor(addr, isCall, false, false, false);
                    }
                    break;
                    case am_C:
                    {
                        Expression::Ptr op(makeRegisterExpression(IntelRegTable(m_Arch,b_cr,locs->modrm_reg)));
                        insn_to_complete->appendOperand(op, isRead, isWritten);
                    }
                    break;
                    case am_D:
                    {
                        Expression::Ptr op(makeRegisterExpression(IntelRegTable(m_Arch,b_dr,locs->modrm_reg)));
                        insn_to_complete->appendOperand(op, isRead, isWritten);
                    }
                    break;
                    case am_E:
                    // am_M is like am_E, except that mod of 0x03 should never occur (am_M specified memory,
                    // mod of 0x03 specifies direct register access).
                    case am_M:
                    // am_R is the inverse of am_M; it should only have a mod of 3
                    case am_R:
                    // can be am_R or am_M	
                    case am_RM:	
                        if(isCFT)
                        {
			  insn_to_complete->addSuccessor(makeModRMExpression(b, optype), isCall, true, false, false);
                        }
                        else
                        {
			  insn_to_complete->appendOperand(makeModRMExpression(b, optype), isRead, isWritten);
                        }
                    break;
                    case am_F:
                    {
                        Expression::Ptr op(makeRegisterExpression(x86::flags));
                        insn_to_complete->appendOperand(op, isRead, isWritten);
                    }
                    break;
                    case am_G:
                    {
                        Expression::Ptr op(makeRegisterExpression(makeRegisterID(locs->modrm_reg,
                                optype, locs->rex_r)));
                        insn_to_complete->appendOperand(op, isRead, isWritten);
                    }
                    break;
                    case am_I:
                        insn_to_complete->appendOperand(decodeImmediate(optype, b.start + 
									locs->imm_position[imm_index++]), 
							isRead, isWritten);
                        break;
                    case am_J:
                    {
                        Expression::Ptr Offset(decodeImmediate(optype, 
							       b.start + locs->imm_position[imm_index++], 
							       true));
                        Expression::Ptr EIP(makeRegisterExpression(MachRegister::getPC(m_Arch)));
                        Expression::Ptr InsnSize(make_shared(singleton_object_pool<Immediate>::construct(Result(u8,
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
                                // closest I can get to "will be address size by default"
                                pseudoOpType = op_v;
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
                                decodeImmediate(pseudoOpType, b.start + offset_position), makeSizeType(optype)), 
							isRead, isWritten);
                    }
                    break;
                    case am_P:
                        insn_to_complete->appendOperand(makeRegisterExpression(IntelRegTable(m_Arch,b_mm,locs->modrm_reg)),
                                isRead, isWritten);
                        break;
                    case am_Q:
        
                        switch(locs->modrm_mod)
                        {
                            // direct dereference
                            case 0x00:
                            case 0x01:
                            case 0x02:
			      insn_to_complete->appendOperand(makeModRMExpression(b, optype), isRead, isWritten);
                                break;
                            case 0x03:
                                // use of actual register
                                insn_to_complete->appendOperand(makeRegisterExpression(IntelRegTable(m_Arch,b_mm,locs->modrm_rm)),
                                                               isRead, isWritten);
                                break;
                            default:
                                assert(!"2-bit value modrm_mod out of range");
                                break;
                        };
                        break;
                    case am_S:
                    // Segment register in modrm reg field.
                        insn_to_complete->appendOperand(makeRegisterExpression(IntelRegTable(m_Arch,b_segment,locs->modrm_reg)),
                                isRead, isWritten);
                        break;
                    case am_T:
                        // test register in modrm reg; should only be tr6/tr7, but we'll decode any of them
                        // NOTE: this only appears in deprecated opcodes
                        insn_to_complete->appendOperand(makeRegisterExpression(IntelRegTable(m_Arch,b_tr,locs->modrm_reg)),
                                                       isRead, isWritten);
                        break;
                    case am_UM:
                    	switch(locs->modrm_mod)
                    	{
                    	// direct dereference
                    	case 0x00:
                    	case 0x01:
                    	case 0x02:
                    		insn_to_complete->appendOperand(makeModRMExpression(b, makeSizeType(optype)),
                    				isRead, isWritten);
                    		break;
                    	case 0x03:
                    		// use of actual register
                    		{
                    			insn_to_complete->appendOperand(makeRegisterExpression(IntelRegTable(m_Arch,
                    					locs->rex_b ? b_xmmhigh : b_xmm, locs->modrm_rm)),
                    					isRead, isWritten);
                    			break;
                    		}
                    	default:
                    		assert(!"2-bit value modrm_mod out of range");
                    		break;
                    	};
                    	break;
                    case am_V:
                       
                        insn_to_complete->appendOperand(makeRegisterExpression(IntelRegTable(m_Arch,
                                locs->rex_r ? b_xmmhigh : b_xmm,locs->modrm_reg)),
                                    isRead, isWritten);
                        break;
                    case am_W:
                        switch(locs->modrm_mod)
                        {
                            // direct dereference
                            case 0x00:
                            case 0x01:
                            case 0x02:
			      insn_to_complete->appendOperand(makeModRMExpression(b, makeSizeType(optype)),
                                                               isRead, isWritten);
                                break;
                            case 0x03:
                            // use of actual register
                            {
                                insn_to_complete->appendOperand(makeRegisterExpression(IntelRegTable(m_Arch,
                                        locs->rex_b ? b_xmmhigh : b_xmm, locs->modrm_rm)),
                                        isRead, isWritten);
                                break;
                            }
                            default:
                                assert(!"2-bit value modrm_mod out of range");
                                break;
                        };
                        break;
                    case am_X:
                    {
			MachRegister si_reg;
			if(m_Arch == Arch_x86)
			{
				if(addrSizePrefixPresent)
				{
					si_reg = x86::si;
				} else
				{
					si_reg = x86::esi;
				}
			}
			else
			{
				if(addrSizePrefixPresent)
				{
					si_reg = x86_64::esi;
				} else
				{
					si_reg = x86_64::rsi;
				}
			}
                        Expression::Ptr ds(makeRegisterExpression(m_Arch == Arch_x86 ? x86::ds : x86_64::ds));
                        Expression::Ptr si(makeRegisterExpression(si_reg));
                        Expression::Ptr segmentOffset(make_shared(singleton_object_pool<Immediate>::construct(
                                Result(u32, 0x10))));
                        Expression::Ptr ds_segment = makeMultiplyExpression(ds, segmentOffset, u32);
                        Expression::Ptr ds_si = makeAddExpression(ds_segment, si, u32);
                        insn_to_complete->appendOperand(makeDereferenceExpression(ds_si, makeSizeType(optype)),
                                                       isRead, isWritten);
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
				} else
				{
					di_reg = x86::edi;
				}
			}
			else
			{
				if(addrSizePrefixPresent)
				{
					di_reg = x86_64::edi;
				} else
				{
					di_reg = x86_64::rdi;
				}
			}
                        Expression::Ptr es(makeRegisterExpression(m_Arch == Arch_x86 ? x86::es : x86_64::es));
                        Expression::Ptr di(makeRegisterExpression(di_reg));
                        Expression::Ptr es_segment = makeMultiplyExpression(es,
                            make_shared(singleton_object_pool<Immediate>::construct(Result(u32, 0x10))), u32);
                        Expression::Ptr es_di = makeAddExpression(es_segment, di, u32);
                        insn_to_complete->appendOperand(makeDereferenceExpression(es_di, makeSizeType(optype)),
                                                       isRead, isWritten);
                    }
                    break;
                    case am_tworeghack:
                    {
                        if(optype == op_edxeax)
                        {
                            Expression::Ptr edx(makeRegisterExpression(m_Arch == Arch_x86 ? x86::edx : x86_64::edx));
                            Expression::Ptr eax(makeRegisterExpression(m_Arch == Arch_x86 ? x86::eax : x86_64::eax));
                            Expression::Ptr highAddr = makeMultiplyExpression(edx,
                                    Immediate::makeImmediate(Result(u64, 2^32)), u64);
                            Expression::Ptr addr = makeAddExpression(highAddr, eax, u64);
                            Expression::Ptr op = makeDereferenceExpression(addr, u64);
                            insn_to_complete->appendOperand(op, isRead, isWritten);
                        }
                        else if (optype == op_ecxebx)
                        {
                            Expression::Ptr ecx(makeRegisterExpression(m_Arch == Arch_x86 ? x86::ecx : x86_64::ecx));
                            Expression::Ptr ebx(makeRegisterExpression(m_Arch == Arch_x86 ? x86::ebx : x86_64::ebx));
                            Expression::Ptr highAddr = makeMultiplyExpression(ecx,
                                    Immediate::makeImmediate(Result(u64, 2^32)), u64);
                            Expression::Ptr addr = makeAddExpression(highAddr, ebx, u64);
                            Expression::Ptr op = makeDereferenceExpression(addr, u64);
                            insn_to_complete->appendOperand(op, isRead, isWritten);
                        }
                    }
                    break;
                    
                    case am_reg:
                    {
                        MachRegister r(optype);
                        r = MachRegister((r.val() & ~r.getArchitecture()) | m_Arch);
                        entryID entryid = decodedInstruction->getEntry()->getID(locs);
                        if(locs->rex_b && insn_to_complete->m_Operands.empty() && 
			    (entryid == e_push || entryid == e_pop || entryid == e_xchg || ((*(b.start + locs->opcode_position) & 0xf0) == 0xb0) ) )
                        {
                            // FP stack registers are not affected by the rex_b bit in AM_REG.
                           if(r.regClass() != (unsigned) x86::MMX)
                            {
                                r = MachRegister((r.val()) | x86_64::r8.val());
                            }
                        }
                        if(sizePrefixPresent)
                        {
                            r = MachRegister((r.val() & ~x86::FULL) | x86::W_REG);
                        }
                        Expression::Ptr op(makeRegisterExpression(r));
                        insn_to_complete->appendOperand(op, isRead, isWritten);
                    }
                    break;
                case am_stackH:
                case am_stackP:
                // handled elsewhere
                    break;
                case am_allgprs:
                {
                    if(m_Arch == Arch_x86)
                    {
                        insn_to_complete->appendOperand(makeRegisterExpression(x86::eax), isRead, isWritten);
                        insn_to_complete->appendOperand(makeRegisterExpression(x86::ecx), isRead, isWritten);
                        insn_to_complete->appendOperand(makeRegisterExpression(x86::edx), isRead, isWritten);
                        insn_to_complete->appendOperand(makeRegisterExpression(x86::ebx), isRead, isWritten);
                        insn_to_complete->appendOperand(makeRegisterExpression(x86::esp), isRead, isWritten);
                        insn_to_complete->appendOperand(makeRegisterExpression(x86::ebp), isRead, isWritten);
                        insn_to_complete->appendOperand(makeRegisterExpression(x86::esi), isRead, isWritten);
                        insn_to_complete->appendOperand(makeRegisterExpression(x86::edi), isRead, isWritten);
                    }
                    else
                    {
                        insn_to_complete->appendOperand(makeRegisterExpression(x86_64::eax), isRead, isWritten);
                        insn_to_complete->appendOperand(makeRegisterExpression(x86_64::ecx), isRead, isWritten);
                        insn_to_complete->appendOperand(makeRegisterExpression(x86_64::edx), isRead, isWritten);
                        insn_to_complete->appendOperand(makeRegisterExpression(x86_64::ebx), isRead, isWritten);
                        insn_to_complete->appendOperand(makeRegisterExpression(x86_64::esp), isRead, isWritten);
                        insn_to_complete->appendOperand(makeRegisterExpression(x86_64::ebp), isRead, isWritten);
                        insn_to_complete->appendOperand(makeRegisterExpression(x86_64::esi), isRead, isWritten);
                        insn_to_complete->appendOperand(makeRegisterExpression(x86_64::edi), isRead, isWritten);
                    }
                }
                    break;
		case am_ImplImm: {
		  insn_to_complete->appendOperand(Immediate::makeImmediate(Result(makeSizeType(optype), 1)), isRead, isWritten);
		  break;
		}

                default:
                    printf("decodeOneOperand() called with unknown addressing method %d\n", operand.admet);
                        break;
                };
                return true;
            }

    extern ia32_entry invalid;
    
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
        ia32_decode(IA32_DECODE_PREFIXES, b.start, *decodedInstruction);
        sizePrefixPresent = (decodedInstruction->getPrefix()->getOperSzPrefix() == 0x66);
        if (decodedInstruction->getPrefix()->rexW()) {
           // as per 2.2.1.2 - rex.w overrides 66h
           sizePrefixPresent = false;
        }
        addrSizePrefixPresent = (decodedInstruction->getPrefix()->getAddrSzPrefix() == 0x67);
    }
    
    void InstructionDecoder_x86::decodeOpcode(InstructionDecoder::buffer& b)
    {
        static ia32_entry invalid = { e_No_Entry, 0, 0, true, { {0,0}, {0,0}, {0,0} }, 0, 0 };
        doIA32Decode(b);
        if(decodedInstruction->getEntry()) {
            m_Operation = make_shared(singleton_object_pool<Operation>::construct(decodedInstruction->getEntry(),
                                    decodedInstruction->getPrefix(), locs, m_Arch));
            
        }
        else
        {
                // Gap parsing can trigger this case; in particular, when it encounters prefixes in an invalid order.
                // Notably, if a REX prefix (0x40-0x48) appears followed by another prefix (0x66, 0x67, etc)
                // we'll reject the instruction as invalid and send it back with no entry.  Since this is a common
                // byte sequence to see in, for example, ASCII strings, we want to simply accept this and move on, not
                // yell at the user.
            m_Operation = make_shared(singleton_object_pool<Operation>::construct(&invalid,
                                    decodedInstruction->getPrefix(), locs, m_Arch));
        }
        b.start += decodedInstruction->getSize();
    }
    
      bool InstructionDecoder_x86::decodeOperands(const Instruction* insn_to_complete)
    {
       int imm_index = 0; // handle multiple immediate operands
        if(!decodedInstruction) return false;
        unsigned int opsema = decodedInstruction->getEntry()->opsema & 0xFF;
	InstructionDecoder::buffer b(insn_to_complete->ptr(), insn_to_complete->size());

        if (decodedInstruction->getEntry()->getID() == e_ret_near ||
            decodedInstruction->getEntry()->getID() == e_ret_far) {
           Expression::Ptr ret_addr = makeDereferenceExpression(makeRegisterExpression(ia32_is_mode_64() ? x86_64::rsp : x86::esp), 
                                                                ia32_is_mode_64() ? u64 : u32);
           insn_to_complete->addSuccessor(ret_addr, false, true, false, false);
	}

        for(unsigned i = 0; i < 3; i++)
        {
            if(decodedInstruction->getEntry()->operands[i].admet == 0 && 
	       decodedInstruction->getEntry()->operands[i].optype == 0)
                return true;
            if(!decodeOneOperand(b,
				 decodedInstruction->getEntry()->operands[i], 
				 imm_index, 
				 insn_to_complete, 
				 readsOperand(opsema, i),
				 writesOperand(opsema, i)))
            {
                return false;
            }
        }
    
        return true;
    }

    
      INSTRUCTION_EXPORT Instruction::Ptr InstructionDecoder_x86::decode(InstructionDecoder::buffer& b)
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
    
};
};

