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

#include "InstructionDecoder-x86.h"
#include "../h/Expression.h"
#include "../src/arch-x86.h"
#include "../h/Register.h"
#include "../h/Dereference.h"
#include "../h/Immediate.h"
#include "../h/BinaryFunction.h"
#include "../../common/h/singleton_object_pool.h"

using namespace std;
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


    
    INSTRUCTION_EXPORT InstructionDecoder_x86::InstructionDecoder_x86(const unsigned char* buffer, size_t size,
                                                                     Architecture arch) :
            InstructionDecoder(buffer, size, arch),
    locs(NULL),
    decodedInstruction(NULL),
    is32BitMode(true),
    sizePrefixPresent(false)
    {
    }
    INSTRUCTION_EXPORT InstructionDecoder_x86::InstructionDecoder_x86() :
            InstructionDecoder(),
            locs(NULL),
    decodedInstruction(NULL),
    is32BitMode(true),
    sizePrefixPresent(false)
    {
    }
#if 0    
    INSTRUCTION_EXPORT InstructionDecoder_x86::InstructionDecoder_x86(const InstructionDecoder_x86& o) :
            InstructionDecoder(o),
            locs(NULL),
    decodedInstruction(NULL),
    is32BitMode(o.is32BitMode),
    sizePrefixPresent(o.sizePrefixPresent)
    {
    }
#endif    
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
    
    Expression::Ptr InstructionDecoder_x86::makeSIBExpression(unsigned int opType)
    {
        unsigned scale;
        Register index;
        Register base;
        Result_Type aw = ia32_is_mode_64() ? u32 : u64;

        decode_SIB(locs->sib_byte, scale, index, base);

        Expression::Ptr scaleAST(make_shared(singleton_object_pool<Immediate>::construct(Result(u8, dword_t(scale)))));
        Expression::Ptr indexAST(make_shared(singleton_object_pool<RegisterAST>::construct(makeRegisterID(index, opType,
                                    locs->rex_x))));
        Expression::Ptr baseAST;
        if(base == 0x05)
        {
            switch(locs->modrm_mod)
            {
                case 0x00:
                    baseAST = decodeImmediate(op_d, locs->sib_position + 1);
                    break;
                    case 0x01: {
                        MachRegister reg;
                        if (locs->rex_b)
                            reg = x86_64::r13;
                        else
			  reg = MachRegister::getFramePointer(m_Arch);
			
                        baseAST = makeAddExpression(make_shared(singleton_object_pool<RegisterAST>::construct(reg)),
						    decodeImmediate(op_b, locs->sib_position + 1), 
						    aw);
                        break;
                    }
                    case 0x02: {
                        MachRegister reg;
                        if (locs->rex_b)
                            reg = x86_64::r13;
                        else
                            reg = MachRegister::getFramePointer(m_Arch);

                        baseAST = makeAddExpression(make_shared(singleton_object_pool<RegisterAST>::construct(reg)), 
						    decodeImmediate(op_d, locs->sib_position + 1),
						    aw);
                        break;
                    }
                case 0x03:
                default:
                    assert(0);
                    break;
            };
        }
        else
        {
            baseAST = make_shared(singleton_object_pool<RegisterAST>::construct(makeRegisterID(base, 
											       opType,
											       locs->rex_b)));
        }
        if(index == 0x04 && (!(ia32_is_mode_64()) || !(locs->rex_x)))
        {
            return baseAST;
        }
        return makeAddExpression(baseAST, makeMultiplyExpression(indexAST, scaleAST, aw), aw);
    }

    Expression::Ptr InstructionDecoder_x86::makeModRMExpression(unsigned int opType)
    {
        unsigned int regType = op_d;
        if(ia32_is_mode_64())
        {
            regType = op_q;
        }
        Result_Type aw = ia32_is_mode_64() ? u32 : u64;
        Expression::Ptr e =
	  makeRegisterExpression(makeRegisterID(locs->modrm_rm, regType, (locs->rex_b == 1)));
        switch(locs->modrm_mod)
        {
            case 0:
                if(locs->modrm_rm == modrm_use_sib) {
                    e = makeSIBExpression(opType);
                }
                if(locs->modrm_rm == 0x5)
                {
                    assert(locs->opcode_position > -1);
                    unsigned char opcode = rawInstruction[locs->opcode_position];
        // treat FP decodes as legacy mode since it appears that's what we've got in our
        // old code...
                    if(ia32_is_mode_64() && (opcode < 0xD8 || opcode > 0xDF))
                    {
                        e = makeAddExpression(makeRegisterExpression(x86_64::rip),
                                            getModRMDisplacement(), aw);
                    }
                    else
                    {
                        e = getModRMDisplacement();
                    }
        
                }
                if(opType == op_lea)
                {
                    return e;
                }
                return makeDereferenceExpression(e, makeSizeType(opType));
            case 1:
            case 2:
            {
                if(locs->modrm_rm == modrm_use_sib) {
                    e = makeSIBExpression(opType);
                }
                Expression::Ptr disp_e = makeAddExpression(e, getModRMDisplacement(), aw);
                if(opType == op_lea)
                {
                    return disp_e;
                }
                return makeDereferenceExpression(disp_e, makeSizeType(opType));
            }
            case 3:
	      return makeRegisterExpression(makeRegisterID(locs->modrm_rm, opType, (locs->rex_b == 1)));
            default:
                return Expression::Ptr();
        
        };
        
    }

    Expression::Ptr InstructionDecoder_x86::decodeImmediate(unsigned int opType, unsigned int position, bool isSigned)
    {
        const unsigned char* bufferEnd = bufferBegin + (bufferSize ? bufferSize : 16);
        assert(position != (unsigned int)(-1));
        switch(opType)
        {
            case op_b:
                assert(rawInstruction + position < bufferEnd);
                return Immediate::makeImmediate(Result(isSigned ? s8 : u8 ,*(const byte_t*)(rawInstruction + position)));
                break;
            case op_d:
                assert(rawInstruction + position + 3 < bufferEnd);
                return Immediate::makeImmediate(Result(isSigned ? s32 : u32,*(const dword_t*)(rawInstruction + position)));
            case op_w:
                assert(rawInstruction + position + 1 < bufferEnd);
                return Immediate::makeImmediate(Result(isSigned ? s16 : u16,*(const word_t*)(rawInstruction + position)));
                break;
            case op_q:
                assert(rawInstruction + position + 7 < bufferEnd);
                return Immediate::makeImmediate(Result(isSigned ? s64 : u64,*(const int64_t*)(rawInstruction + position)));
                break;
            case op_v:
            case op_z:
        // 32 bit mode & no prefix, or 16 bit mode & prefix => 32 bit
        // 16 bit mode, no prefix or 32 bit mode, prefix => 16 bit
                if(!sizePrefixPresent)
                {
                    assert(rawInstruction + position + 3 < bufferEnd);
                    return Immediate::makeImmediate(Result(isSigned ? s32 : u32,*(const dword_t*)(rawInstruction + position)));
                }
                else
                {
                    assert(rawInstruction + position + 1 < bufferEnd);
                    return Immediate::makeImmediate(Result(isSigned ? s16 : u16,*(const word_t*)(rawInstruction + position)));
                }
        
                break;
            case op_p:
        // 32 bit mode & no prefix, or 16 bit mode & prefix => 48 bit
        // 16 bit mode, no prefix or 32 bit mode, prefix => 32 bit
                if(!sizePrefixPresent)
                {
                    assert(rawInstruction + position + 5< bufferEnd);
                    return Immediate::makeImmediate(Result(isSigned ? s48 : u48,*(const int64_t*)(rawInstruction + position)));
                }
                else
                {
                    assert(rawInstruction + position + 3 < bufferEnd);
                    return Immediate::makeImmediate(Result(isSigned ? s32 : u32,*(const dword_t*)(rawInstruction + position)));
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
    
    Expression::Ptr InstructionDecoder_x86::getModRMDisplacement()
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
        const unsigned char* bufferEnd = bufferBegin + bufferSize;
        switch(locs->modrm_mod)
        {
            case 1:
                assert(rawInstruction + disp_pos + 1 <= bufferEnd);
                return make_shared(singleton_object_pool<Immediate>::construct(Result(s8, (*(const byte_t*)(rawInstruction +
                        disp_pos)))));
                break;
            case 2:
                if(sizePrefixPresent)
                {
                    assert(rawInstruction + disp_pos + 2 <= bufferEnd);
                    return make_shared(singleton_object_pool<Immediate>::construct(Result(s16, *((const word_t*)(rawInstruction +
                            disp_pos)))));
                }
                else
                {
                    assert(rawInstruction + disp_pos + 4 <= bufferEnd);
                    return make_shared(singleton_object_pool<Immediate>::construct(Result(s32, *((const dword_t*)(rawInstruction +
                            disp_pos)))));
                }
                break;
            case 0:
                // In 16-bit mode, the word displacement is modrm r/m 6
                if(sizePrefixPresent)
                {
                    if(locs->modrm_rm == 6)
                    {
                        assert(rawInstruction + disp_pos + 4 <= bufferEnd);
                        return make_shared(singleton_object_pool<Immediate>::construct(Result(s32,
                                           *((const dword_t*)(rawInstruction + disp_pos)))));
                    }
                    else
                    {
                        assert(rawInstruction + disp_pos + 1 <= bufferEnd);
                        return make_shared(singleton_object_pool<Immediate>::construct(Result(s8, 0)));
                    }
                    break;
                }
                // ...and in 32-bit mode, the dword displacement is modrm r/m 5
                else
                {
                    if(locs->modrm_rm == 5)
                    {
                        assert(rawInstruction + disp_pos + 4 <= bufferEnd);
                        return make_shared(singleton_object_pool<Immediate>::construct(Result(s32,
                                           *((const dword_t*)(rawInstruction + disp_pos)))));
                    }
                    else
                    {
                        assert(rawInstruction + disp_pos + 1 <= bufferEnd);
                        return make_shared(singleton_object_pool<Immediate>::construct(Result(s8, 0)));
                    }
                    break;
                }
            default:
                assert(rawInstruction + disp_pos + 1 <= bufferEnd);
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
        b_mm,
        b_cr,
        b_dr,
        b_tr,
        b_amd64ext,
        b_8bitWithREX
    };
    using namespace x86;
    
    static MachRegister IntelRegTable[][8] = {
        {
            al, cl, dl, bl, ah, ch, dh, bh
        },
        {
            ax, cx, dx, bx, sp, bp, si, di
        },
        {
            eax, ecx, edx, ebx, esp, ebp, esi, edi
        },
        {
            es, cs, ss, ds, fs, gs, InvalidReg, InvalidReg
        },
        {
            x86_64::rax, x86_64::rcx, x86_64::rdx, x86_64::rbx, x86_64::rsp, x86_64::rbp, x86_64::rsi, x86_64::rdi
        },
        {
            xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7
        },
        {
            mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7
        },
        {
            cr0, cr1, cr2, cr3, cr4, cr5, cr6, cr7
        },
        {
            dr0, dr1, dr2, dr3, dr4, dr5, dr6, dr7
        },
        {
            tr0, tr1, tr2, tr3, tr4, tr5, tr6, tr7
        },
        {
            x86_64::r8, x86_64::r9, x86_64::r10, x86_64::r11, x86_64::r12, x86_64::r13, x86_64::r14, x86_64::r15
        },
        {
            x86_64::al, x86_64::cl, x86_64::dl, x86_64::bl, x86_64::spl, x86_64::bpl, x86_64::sil, x86_64::dil
        }

    };

    MachRegister InstructionDecoder_x86::makeRegisterID(unsigned int intelReg, unsigned int opType,
                                        bool isExtendedReg)
    {
        MachRegister retVal;
        if(isExtendedReg)
        {
            retVal = IntelRegTable[b_amd64ext][intelReg];
        }
        else if(locs->rex_w)
        {
            // AMD64 with 64-bit operands
            retVal = IntelRegTable[b_64bit][intelReg];
        }
        else
        {
            switch(opType)
            {
                case op_b:
                    if (locs->rex_position == -1) {
                        retVal = IntelRegTable[b_8bitNoREX][intelReg];
                    } else {
                        retVal = IntelRegTable[b_8bitWithREX][intelReg];
                    }
                    break;
                case op_q:
                    retVal = IntelRegTable[b_64bit][intelReg];
                    break;
                case op_d:
                case op_si:
                case op_w:
                default:
                    retVal = IntelRegTable[b_32bit][intelReg];
                    break;
            }
        }
	if (m_Arch == Arch_x86) {
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
                if(is32BitMode ^ sizePrefixPresent)
                {
                    return u32;
                }
                else
                {
                    return u16;
                }
                break;
            case op_p:
                // book says operand size; arch-x86 says word + word * operand size
                if(is32BitMode ^ sizePrefixPresent)
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


    bool InstructionDecoder_x86::decodeOneOperand(const ia32_operand& operand,
            int & imm_index, /* immediate operand index */
            const Instruction* insn_to_complete, bool isRead, bool isWritten)
            {
                unsigned int regType = op_d;
                if(ia32_is_mode_64())
                {
                    regType = op_q;
                }
                bool isCFT = false;
                bool isCall = false;
                InsnCategory cat = insn_to_complete->getCategory();
                if(cat == c_BranchInsn || cat == c_CallInsn)
                {
                    isCFT = true;
                    if(cat == c_CallInsn)
                    {
                        isCall = true;
                    }
                }
                        
                switch(operand.admet)
                {
                    case 0:
                    // No operand
                    {
                        fprintf(stderr, "ERROR: Instruction with mismatched operands. Raw bytes: ");
                        for(unsigned int i = 0; i < decodedInstruction->getSize(); i++) {
                            fprintf(stderr, "%x ", rawInstruction[i]);
                        }
                        fprintf(stderr, "\n");
                        assert(!"Mismatched number of operands--check tables");
                        return false;
                    }
                    case am_A:
                    {
                        // am_A only shows up as a far call/jump.  Position 1 should be universally safe.
                        Expression::Ptr addr(decodeImmediate(operand.optype, 1));
                        Expression::Ptr op(makeDereferenceExpression(addr, makeSizeType(operand.optype)));
                        insn_to_complete->addSuccessor(op, isCall, false, false, false);
                    }
                    break;
                    case am_C:
                    {
                        Expression::Ptr op(makeRegisterExpression(IntelRegTable[b_cr][locs->modrm_reg]));
                        insn_to_complete->appendOperand(op, isRead, isWritten);
                    }
                    break;
                    case am_D:
                    {
                        Expression::Ptr op(makeRegisterExpression(IntelRegTable[b_dr][locs->modrm_reg]));
                        insn_to_complete->appendOperand(op, isRead, isWritten);
                    }
                    break;
                    case am_E:
                    // am_M is like am_E, except that mod of 0x03 should never occur (am_M specified memory,
                    // mod of 0x03 specifies direct register access).
                    case am_M:
                    // am_R is the inverse of am_M; it should only have a mod of 3
                    case am_R:
                        if(isCFT)
                        {
                            insn_to_complete->addSuccessor(makeModRMExpression(operand.optype), isCall, true, false, false);
                        }
                        else
                        {
                            insn_to_complete->appendOperand(makeModRMExpression(operand.optype), isRead, isWritten);
                        }
                    break;
                    case am_F:
                    {
                        Expression::Ptr op(makeRegisterExpression(flags));
                        insn_to_complete->appendOperand(op, isRead, isWritten);
                    }
                    break;
                    case am_G:
                    {
                        Expression::Ptr op(makeRegisterExpression(makeRegisterID(locs->modrm_reg,
                                operand.optype, locs->rex_r)));
                        insn_to_complete->appendOperand(op, isRead, isWritten);
                    }
                    break;
                    case am_I:
                        insn_to_complete->appendOperand(decodeImmediate(operand.optype, locs->imm_position[imm_index++]), isRead, isWritten);
                        break;
                    case am_J:
                    {
                        Expression::Ptr Offset(decodeImmediate(operand.optype, locs->imm_position[imm_index++], true));
                        Expression::Ptr EIP(makeRegisterExpression(MachRegister::getPC(m_Arch)));
                        Expression::Ptr InsnSize(make_shared(singleton_object_pool<Immediate>::construct(Result(u8,
                            decodedInstruction->getSize()))));
                        Expression::Ptr postEIP(makeAddExpression(EIP, InsnSize, u32));

                        Expression::Ptr op(makeAddExpression(Offset, postEIP, u32));
                        insn_to_complete->addSuccessor(op, isCall, false, false, false);
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
                                decodeImmediate(pseudoOpType, offset_position), makeSizeType(operand.optype)), isRead, isWritten);
                    }
                    break;
                    case am_P:
                        insn_to_complete->appendOperand(makeRegisterExpression(IntelRegTable[b_mm][locs->modrm_reg]),
                                isRead, isWritten);
                        break;
                    case am_Q:
        
                        switch(locs->modrm_mod)
                        {
                            // direct dereference
                            case 0x00:
                            case 0x01:
                            case 0x02:
                                insn_to_complete->appendOperand(makeModRMExpression(operand.optype), isRead, isWritten);
                                break;
                            case 0x03:
                                // use of actual register
                                insn_to_complete->appendOperand(makeRegisterExpression(IntelRegTable[b_mm][locs->modrm_rm]),
                                                               isRead, isWritten);
                                break;
                            default:
                                assert(!"2-bit value modrm_mod out of range");
                                break;
                        };
                        break;
                    case am_S:
                    // Segment register in modrm reg field.
                        insn_to_complete->appendOperand(makeRegisterExpression(IntelRegTable[b_segment][locs->modrm_reg]),
                                isRead, isWritten);
                        break;
                    case am_T:
                        // test register in modrm reg; should only be tr6/tr7, but we'll decode any of them
                        // NOTE: this only appears in deprecated opcodes
                        insn_to_complete->appendOperand(makeRegisterExpression(IntelRegTable[b_tr][locs->modrm_reg]),
                                                       isRead, isWritten);
                        break;
                    case am_V:
                        insn_to_complete->appendOperand(makeRegisterExpression(IntelRegTable[b_xmm][locs->modrm_reg]),
                                                       isRead, isWritten);
                        break;
                    case am_W:
                        switch(locs->modrm_mod)
                        {
                            // direct dereference
                            case 0x00:
                            case 0x01:
                            case 0x02:
                                insn_to_complete->appendOperand(makeModRMExpression(makeSizeType(operand.optype)),
                                                               isRead, isWritten);
                                break;
                            case 0x03:
                            // use of actual register
                            {
                                insn_to_complete->appendOperand(makeRegisterExpression(IntelRegTable[b_xmm][locs->modrm_rm]),
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
                        Expression::Ptr ds(makeRegisterExpression(m_Arch == Arch_x86 ? x86::ds : x86_64::ds));
                        Expression::Ptr si(makeRegisterExpression(m_Arch == Arch_x86 ? x86::si : x86_64::si));
                        Expression::Ptr segmentOffset(make_shared(singleton_object_pool<Immediate>::construct(
                                Result(u32, 0x10))));
                        Expression::Ptr ds_segment = makeMultiplyExpression(ds, segmentOffset, u32);
                        Expression::Ptr ds_si = makeAddExpression(ds_segment, si, u32);
                        insn_to_complete->appendOperand(makeDereferenceExpression(ds_si, makeSizeType(operand.optype)),
                                                       isRead, isWritten);
                    }
                    break;
                    case am_Y:
                    {
                        Expression::Ptr es(makeRegisterExpression(m_Arch == Arch_x86 ? x86::es : x86_64::es));
                        Expression::Ptr di(makeRegisterExpression(m_Arch == Arch_x86 ? x86::di : x86_64::di));
                        Expression::Ptr es_segment = makeMultiplyExpression(es,
                            make_shared(singleton_object_pool<Immediate>::construct(Result(u32, 0x10))), u32);
                        Expression::Ptr es_di = makeAddExpression(es_segment, di, u32);
                        insn_to_complete->appendOperand(makeDereferenceExpression(es_di, makeSizeType(operand.optype)),
                                                       isRead, isWritten);
                    }
                    break;
                    case am_tworeghack:
                    {
                        if(operand.optype == r_EDXEAX)
                        {
                            Expression::Ptr edx(makeRegisterExpression(m_Arch == Arch_x86 ? x86::edx : x86_64::edx));
                            Expression::Ptr eax(makeRegisterExpression(m_Arch == Arch_x86 ? x86::eax : x86_64::eax));
                            Expression::Ptr highAddr = makeMultiplyExpression(edx,
                                    Immediate::makeImmediate(Result(u64, 2^32)), u64);
                            Expression::Ptr addr = makeAddExpression(highAddr, eax, u64);
                            Expression::Ptr op = makeDereferenceExpression(addr, u64);
                            insn_to_complete->appendOperand(op, isRead, isWritten);
                        }
                        else if (operand.optype == r_ECXEBX)
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
                        MachRegister r(operand.optype);
                        r = MachRegister(r.val() & ~r.getArchitecture() | m_Arch);
                        if(locs->rex_b)
                        {
                            r = MachRegister((r.val()) | x86_64::r8.val());
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
                default:
                    printf("decodeOneOperand() called with unknown addressing method %d\n", operand.admet);
                        break;
                };
                return true;
            }

    extern ia32_entry invalid;
    
    void InstructionDecoder_x86::doIA32Decode()
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
    
        locs = new(locs) ia32_locations();
        assert(locs);
        decodedInstruction = new (decodedInstruction) ia32_instruction(NULL, NULL, locs);
        assert(locs);
        ia32_decode(IA32_DECODE_PREFIXES, rawInstruction, *decodedInstruction);
        sizePrefixPresent = (decodedInstruction->getPrefix()->getOperSzPrefix() == 0x66);
        addrSizePrefixPresent = (decodedInstruction->getPrefix()->getAddrSzPrefix() == 0x67);
    }
    
    unsigned int InstructionDecoder_x86::decodeOpcode()
    {
        static ia32_entry invalid = { e_No_Entry, 0, 0, true, { {0,0}, {0,0}, {0,0} }, 0, 0 };
        doIA32Decode();
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
        return decodedInstruction->getSize();
    }
    
    bool InstructionDecoder_x86::decodeOperands(const Instruction* insn_to_complete)
    {
        int imm_index = 0; // handle multiple immediate operands
        if(!decodedInstruction) return false;
        assert(locs);
        unsigned int opsema = decodedInstruction->getEntry()->opsema & 0xFF;
    
        for(unsigned i = 0; i < 3; i++)
        {
            if(decodedInstruction->getEntry()->operands[i].admet == 0 && decodedInstruction->getEntry()->operands[i].optype == 0)
                return true;
            if(!decodeOneOperand(decodedInstruction->getEntry()->operands[i], 
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

    Instruction::Ptr InstructionDecoder_x86::decode()
    {
        Instruction::Ptr ret(InstructionDecoder::decode());
        if(ret)
        {
            assert(m_Arch != Arch_none);
            ret->arch_decoded_from = m_Arch;
        }
        return ret;
    }
    
    INSTRUCTION_EXPORT Instruction::Ptr InstructionDecoder_x86::decode(const unsigned char* buffer)
    {
        Instruction::Ptr ret(InstructionDecoder::decode(buffer));
        if(ret)
        {
            assert(m_Arch != Arch_none);
            ret->arch_decoded_from = m_Arch;
        }
        return ret;
    }
    void InstructionDecoder_x86::doDelayedDecode(const Instruction* insn_to_complete)
    {
        setBuffer(reinterpret_cast<const unsigned char*>(insn_to_complete->ptr()), insn_to_complete->size());
        insn_to_complete->m_Operands.reserve(4);
        doIA32Decode();        
        decodeOperands(insn_to_complete);
        resetBuffer();
    }
    
};
};

