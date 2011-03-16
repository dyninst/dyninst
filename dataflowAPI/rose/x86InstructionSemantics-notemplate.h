#ifndef ROSE_X86INSTRUCTIONSEMANTICS_H
#define ROSE_X86INSTRUCTIONSEMANTICS_H

//#include "rose.h"
#include "semanticsModule.h"
#include <cassert>
#include <cstdio>
#include <iostream>
#include "rose.h"

#include "integerOps.h"


#include "SgAsmx86Instruction.h"
#include "SgAsmExpression.h"
#include "conversions.h"

/* Returns the segment register corresponding to the specified register reference address expression. */
static inline X86SegmentRegister getSegregFromMemoryReference(SgAsmMemoryReferenceExpression* mr) {
    X86SegmentRegister segreg = x86_segreg_none;
    SgAsmx86RegisterReferenceExpression* seg = isSgAsmx86RegisterReferenceExpression(mr->get_segment());
    if (seg) {
        ROSE_ASSERT(seg->get_register_class() == x86_regclass_segment);
        segreg = (X86SegmentRegister)(seg->get_register_number());
    } else {
        ROSE_ASSERT(!"Bad segment expr");
    }
    if (segreg == x86_segreg_none) segreg = x86_segreg_ds;
    return segreg;
}


// I'm modifying this to automatically wrap whatever we were
// given in a shared_ptr (via boost)

template <typename Policy, typename Word>
    class X86InstructionSemantics {
 public:

    Policy& policy;
    
    X86InstructionSemantics(Policy& policy)
        : policy(policy) {};
    
    virtual ~X86InstructionSemantics() {}
    
    /* If the counter (cx register) is non-zero then decrement it. The return value is a flag indicating whether cx was
     * originally non-zero. */
    Word stringop_setup_loop() {
        Word ecxNotZero = policy.invert(policy.equalToZero(policy.readGPR(x86_gpr_cx)));
        policy.writeGPR(x86_gpr_cx,
                        policy.add(policy.readGPR(x86_gpr_cx),
                                   policy.ite(ecxNotZero, number(-1), number(0))));
        return ecxNotZero;
    }
    
    /* Instruction semantics for rep_stosN where N is 1(b), 2(w), or 4(d).
     * This method handles semantics for one iteration of stosN.
     * See https://siyobik.info/index.php?module=x86&id=279 */
    void rep_stos_semantics(SgAsmx86Instruction *insn, size_t N) {
        Word ecxNotZero = policy.invert(policy.equalToZero(policy.readGPR(x86_gpr_cx)));

        /* Fill memory pointed to by ES:[DI] with contents of AX register if counter was not zero. */
        policy.writeMemory(x86_segreg_es,
                           policy.readGPR(x86_gpr_di),
                           extract(policy.readGPR(x86_gpr_ax), 0, 8*N),
                           ecxNotZero);

        /* Update DI by incrementing or decrementing by number of bytes copied (only if counter is nonzero) */
        policy.writeGPR(x86_gpr_di,
                        policy.add(policy.readGPR(x86_gpr_di),
                                   policy.ite(ecxNotZero,
                                              policy.ite(policy.readFlag(x86_flag_df), number(-N), number(N)),
                                              number(0))));

        /* Decrement the counter if not already zero. */
        policy.writeGPR(x86_gpr_cx,
                        policy.add(policy.readGPR(x86_gpr_cx),
                                   policy.ite(ecxNotZero, number(-1), number(0))));

        /* If counter is now nonzero then repeat this instruction, otherwise go to the next one. */
        ecxNotZero = policy.invert(policy.equalToZero(policy.readGPR(x86_gpr_cx)));
        policy.writeIP(policy.ite(ecxNotZero, number((uint32_t)(insn->get_address())), policy.readIP()));
    }

    /* Instruction semantics for stosN where N is 1(b), 2(w), or 4(d) */
    void stos_semantics(SgAsmx86Instruction *insn, size_t N) {
        const SgAsmExpressionPtrList& operands = insn->get_operandList()->get_operands();
        ROSE_ASSERT(operands.size() == 0);
        ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32);

        /* Fill memory pointed to by ES:[DI] with contents of AX. */
        policy.writeMemory(x86_segreg_es,
                           policy.readGPR(x86_gpr_di),
                           extract((policy.readGPR(x86_gpr_ax)), 0, 8*N),
                           policy.true_());
        
        /* Update DI */ /*FIXME: Is this correct? */
        policy.writeGPR(x86_gpr_di,
                        policy.add(policy.readGPR(x86_gpr_di),
                                   policy.ite(policy.readFlag(x86_flag_df), number(-N), number(N))));
    }
    
    Word invertMaybe(const Word& w, bool inv) {
        if (inv) {
            return policy.invert(w);
        } else {
            return w;
        }
    }

    Word number(uintmax_t v, size_t Len = 32) {
        return policy.number(v, Len);
    }

    Word extract(Word w, size_t From, size_t To) {
        return policy.extract(w, From, To);
    }

    Word signExtend(Word w, size_t From, size_t To) {
        return policy.template signExtend(w, From, To);
    }

    Word greaterOrEqualToTen(Word w, size_t Len) {
        Word carries = number(0, Len);
        policy.addWithCarries(w, number(6, Len), policy.false_(), carries, Len);
        return extract(carries, Len-1, Len);
    }

    Word readEflags() {
        return policy.concat(readFlags(), number(0x0000, 16));
    }

    Word readFlags() {
        return policy.concat(policy.readFlag((X86Flag)0 ),
                             policy.concat(policy.readFlag((X86Flag)1 ),
                             policy.concat(policy.readFlag((X86Flag)2 ),
                             policy.concat(policy.readFlag((X86Flag)3 ),
                             policy.concat(policy.readFlag((X86Flag)4 ),
                             policy.concat(policy.readFlag((X86Flag)5 ),
                             policy.concat(policy.readFlag((X86Flag)6 ),
                             policy.concat(policy.readFlag((X86Flag)7 ),
                             policy.concat(policy.readFlag((X86Flag)8 ),
                             policy.concat(policy.readFlag((X86Flag)9 ),
                             policy.concat(policy.readFlag((X86Flag)10),
                             policy.concat(policy.readFlag((X86Flag)11),
                             policy.concat(policy.readFlag((X86Flag)12),
                             policy.concat(policy.readFlag((X86Flag)13),
                             policy.concat(policy.readFlag((X86Flag)14),
                                           policy.readFlag((X86Flag)15))))))))))))))));
    }

    Word readMemory(X86SegmentRegister segreg, const Word& addr, Word cond, size_t Len) {
        return policy.readMemory(segreg, addr, cond, Len);
    }

    Word readEffectiveAddress(SgAsmExpression* expr) {
        assert (isSgAsmMemoryReferenceExpression(expr));
        return read32(isSgAsmMemoryReferenceExpression(expr)->get_address());
    }

    /* Returns an eight-bit value desribed by an instruction operand. */
    Word read8(SgAsmExpression* e) {
        switch (e->variantT()) {
        case V_SgAsmx86RegisterReferenceExpression: {
            SgAsmx86RegisterReferenceExpression* rre = isSgAsmx86RegisterReferenceExpression(e);
            switch (rre->get_register_class()) {
            case x86_regclass_gpr: {
                X86GeneralPurposeRegister reg = (X86GeneralPurposeRegister)(rre->get_register_number());
                Word rawValue = policy.readGPR(reg);
                switch (rre->get_position_in_register()) {
                case x86_regpos_low_byte: 
                    return extract(rawValue, 0, 8);
                case x86_regpos_high_byte: 
                    return extract(rawValue, 8, 16);
                default: ROSE_ASSERT(!"Bad position in register");
                }
            }
            default: {
                fprintf(stderr, "Bad register class %s\n", regclassToString(rre->get_register_class()));
                abort();
            }
            }
            break;
        }
        case V_SgAsmBinaryAdd: {
            return policy.add(read8(isSgAsmBinaryAdd(e)->get_lhs()), read8(isSgAsmBinaryAdd(e)->get_rhs()));
        }
        case V_SgAsmBinaryMultiply: {
            SgAsmByteValueExpression* rhs = isSgAsmByteValueExpression(isSgAsmBinaryMultiply(e)->get_rhs());
            ROSE_ASSERT(rhs);
            SgAsmExpression* lhs = isSgAsmBinaryMultiply(e)->get_lhs();
            return extract(policy.unsignedMultiply(read8(lhs), read8(rhs)), 0, 8);
        }
        case V_SgAsmMemoryReferenceExpression: {
            return readMemory(getSegregFromMemoryReference(isSgAsmMemoryReferenceExpression(e)),
                              readEffectiveAddress(e), policy.true_(), 8);
        }
        case V_SgAsmByteValueExpression:
        case V_SgAsmWordValueExpression:
        case V_SgAsmDoubleWordValueExpression:
        case V_SgAsmQuadWordValueExpression: {
            uint64_t val = getAsmSignedConstant(isSgAsmValueExpression(e));
            return number(val & 0xFFU, 8);
        }
        default: {
            fprintf(stderr, "Bad variant %s in read8\n", e->class_name().c_str());
            abort();
        }
        }
    }

    /* Returns a 16-bit value described by an instruction operand. */
    Word read16(SgAsmExpression* e) {
        switch (e->variantT()) {
        case V_SgAsmx86RegisterReferenceExpression: {
            SgAsmx86RegisterReferenceExpression* rre = isSgAsmx86RegisterReferenceExpression(e);
            ROSE_ASSERT(rre->get_position_in_register() == x86_regpos_word);
            switch (rre->get_register_class()) {
            case x86_regclass_gpr: {
                X86GeneralPurposeRegister reg = (X86GeneralPurposeRegister)(rre->get_register_number());
                Word rawValue = policy.readGPR(reg);
                return extract(rawValue, 0, 16);
            }
            case x86_regclass_segment: {
                X86SegmentRegister sr = (X86SegmentRegister)(rre->get_register_number());
                Word value = policy.readSegreg(sr);
                return value;
            }
            default: {
                fprintf(stderr, "Bad register class %s\n", regclassToString(rre->get_register_class()));
                abort();
            }
            }
            break;
        }
        case V_SgAsmBinaryAdd: {
            return policy.add(read16(isSgAsmBinaryAdd(e)->get_lhs()), read16(isSgAsmBinaryAdd(e)->get_rhs()));
        }
        case V_SgAsmBinaryMultiply: {
            SgAsmByteValueExpression* rhs = isSgAsmByteValueExpression(isSgAsmBinaryMultiply(e)->get_rhs());
            ROSE_ASSERT(rhs);
            SgAsmExpression* lhs = isSgAsmBinaryMultiply(e)->get_lhs();
            return extract(policy.unsignedMultiply(read16(lhs), read8(rhs)), 0, 16);
        }
        case V_SgAsmMemoryReferenceExpression: {
            return readMemory(getSegregFromMemoryReference(isSgAsmMemoryReferenceExpression(e)),
                              readEffectiveAddress(e), policy.true_(), 16);
        }
        case V_SgAsmByteValueExpression:
        case V_SgAsmWordValueExpression:
        case V_SgAsmDoubleWordValueExpression:
        case V_SgAsmQuadWordValueExpression: {
            uint64_t val = getAsmSignedConstant(isSgAsmValueExpression(e));
            return number(val & 0xFFFFU, 16);
        }
        default: {
            fprintf(stderr, "Bad variant %s in read16\n", e->class_name().c_str());
            abort();
        }
        }
    }

    /* Returns a 32-bit value described by an instruction operand. */
    Word read32(SgAsmExpression* e) {
        switch (e->variantT()) {
        case V_SgAsmx86RegisterReferenceExpression: {
            SgAsmx86RegisterReferenceExpression* rre = isSgAsmx86RegisterReferenceExpression(e);
            switch (rre->get_register_class()) {
            case x86_regclass_gpr: {
                X86GeneralPurposeRegister reg = (X86GeneralPurposeRegister)(rre->get_register_number());
                Word rawValue = policy.readGPR(reg);
                switch (rre->get_position_in_register()) {
                case x86_regpos_dword:
                case x86_regpos_all:
                    return rawValue;
                case x86_regpos_word:
                    return policy.concat(extract(rawValue,0,16), number(0,16));
                default:
                    ROSE_ASSERT(!"bad position in register");
                }
            }
            case x86_regclass_segment: {
                ROSE_ASSERT(rre->get_position_in_register() == x86_regpos_dword ||
                            rre->get_position_in_register() == x86_regpos_all);
                X86SegmentRegister sr = (X86SegmentRegister)(rre->get_register_number());
                Word value = policy.readSegreg(sr);
                return policy.concat(value, number(0,16));
            }
            default: {
                fprintf(stderr, "Bad register class %s\n", regclassToString(rre->get_register_class()));
                abort();
            }
            }
            break;
        }
        case V_SgAsmBinaryAdd: {
            return policy.add(read32(isSgAsmBinaryAdd(e)->get_lhs()), read32(isSgAsmBinaryAdd(e)->get_rhs()));
        }
        case V_SgAsmBinaryMultiply: {
            SgAsmByteValueExpression* rhs = isSgAsmByteValueExpression(isSgAsmBinaryMultiply(e)->get_rhs());
            ROSE_ASSERT(rhs);
            SgAsmExpression* lhs = isSgAsmBinaryMultiply(e)->get_lhs();
            return extract(policy.unsignedMultiply(read32(lhs), read8(rhs)), 0, 32);
        }
        case V_SgAsmMemoryReferenceExpression: {
            return readMemory(getSegregFromMemoryReference(isSgAsmMemoryReferenceExpression(e)),
                              readEffectiveAddress(e), policy.true_(), 32);
        }
        case V_SgAsmByteValueExpression:
        case V_SgAsmWordValueExpression:
        case V_SgAsmDoubleWordValueExpression:
        case V_SgAsmQuadWordValueExpression: {
            uint64_t val = getAsmSignedConstant(isSgAsmValueExpression(e));
            return number(val & 0xFFFFFFFFU);
        }
        default: {
            fprintf(stderr, "Bad variant %s in read32\n", e->class_name().c_str());
            abort();
        }
        }
    }

    /* Replaces the least significant byte of a general purpose register with a new value. */
    void updateGPRLowByte(X86GeneralPurposeRegister reg, const Word& value) {
        Word oldValue = policy.readGPR(reg);
        policy.writeGPR(reg, policy.concat(value, extract(oldValue,8,32)));
    }

    /* Replaces bits 8 through 15 of a 32-bit register with the specified 8-bit value. */
    void updateGPRHighByte(X86GeneralPurposeRegister reg, const Word& value) {
        Word oldValue = policy.readGPR(reg);
        policy.writeGPR(reg, policy.concat(extract(oldValue,0,8),
                                           policy.concat(value, extract(oldValue,16,32))));
    }

    /* Replaces the least significant 16 bits of a general purpose register with a new value. */
    void updateGPRLowWord(X86GeneralPurposeRegister reg, const Word& value) {
        Word oldValue = policy.readGPR(reg);
        policy.writeGPR(reg, policy.concat(value, extract(oldValue,16,32)));
    }

    /* Writes the specified eight-bit value to the location specified by an instruction operand. */
    void write8(SgAsmExpression* e, const Word& value) {
        switch (e->variantT()) {
        case V_SgAsmx86RegisterReferenceExpression: {
            SgAsmx86RegisterReferenceExpression* rre = isSgAsmx86RegisterReferenceExpression(e);
            switch (rre->get_register_class()) {
            case x86_regclass_gpr: {
                X86GeneralPurposeRegister reg = (X86GeneralPurposeRegister)(rre->get_register_number());
                switch (rre->get_position_in_register()) {
                case x86_regpos_low_byte: updateGPRLowByte(reg, value); break;
                case x86_regpos_high_byte: updateGPRHighByte(reg, value); break;
                default: ROSE_ASSERT(!"Bad position in register");
                }
                break;
            }
            default: {
                fprintf(stderr, "Bad register class %s\n", regclassToString(rre->get_register_class()));
                abort();
            }
            }
            break;
        }
        case V_SgAsmMemoryReferenceExpression: {
            policy.writeMemory(getSegregFromMemoryReference(isSgAsmMemoryReferenceExpression(e)),
                               readEffectiveAddress(e), value, policy.true_());
            break;
        }
        default: {
            fprintf(stderr, "Bad variant %s in write8\n", e->class_name().c_str());
            abort();
        }
        }
    }

    /* Writes the specified 16-bit value to the location specified by an instruction operand. */
    void write16(SgAsmExpression* e, const Word& value) {
        switch (e->variantT()) {
        case V_SgAsmx86RegisterReferenceExpression: {
            SgAsmx86RegisterReferenceExpression* rre = isSgAsmx86RegisterReferenceExpression(e);
            switch (rre->get_register_class()) {
            case x86_regclass_gpr: {
                X86GeneralPurposeRegister reg = (X86GeneralPurposeRegister)(rre->get_register_number());
                switch (rre->get_position_in_register()) {
                case x86_regpos_word: updateGPRLowWord(reg, value); break;
                default: ROSE_ASSERT(!"Bad position in register");
                }
                break;
            }
            case x86_regclass_segment: {
                X86SegmentRegister sr = (X86SegmentRegister)(rre->get_register_number());
                policy.writeSegreg(sr, value);
                break;
            }
            default: {
                fprintf(stderr, "Bad register class %s\n", regclassToString(rre->get_register_class()));
                abort();
            }
            }
            break;
        }
        case V_SgAsmMemoryReferenceExpression: {
            policy.writeMemory(getSegregFromMemoryReference(isSgAsmMemoryReferenceExpression(e)),
                               readEffectiveAddress(e), value, policy.true_());
            break;
        }
        default: {
            fprintf(stderr, "Bad variant %s in write16\n", e->class_name().c_str());
            abort();
        }
        }
    }

    /* Writes the specified 32-bit value to the location specified by an instruction operand. */
    void write32(SgAsmExpression* e, const Word& value) {
        switch (e->variantT()) {
        case V_SgAsmx86RegisterReferenceExpression: {
            SgAsmx86RegisterReferenceExpression* rre = isSgAsmx86RegisterReferenceExpression(e);
            switch (rre->get_register_class()) {
            case x86_regclass_gpr: {
                X86GeneralPurposeRegister reg = (X86GeneralPurposeRegister)(rre->get_register_number());
                switch (rre->get_position_in_register()) {
                case x86_regpos_dword:
                case x86_regpos_all: {
                    break;
                }
                default: ROSE_ASSERT(!"Bad position in register");
                }
                policy.writeGPR(reg, value);
                break;
            }
            case x86_regclass_segment: { // Used for pop of segment registers
                X86SegmentRegister sr = (X86SegmentRegister)(rre->get_register_number());
                policy.writeSegreg(sr, extract(value,0,16));
                break;
            }
            default: {
                fprintf(stderr, "Bad register class %s\n", regclassToString(rre->get_register_class()));
                abort();
            }
            }
            break;
        }
        case V_SgAsmMemoryReferenceExpression: {
            policy.writeMemory(getSegregFromMemoryReference(isSgAsmMemoryReferenceExpression(e)),
                               readEffectiveAddress(e), value, policy.true_());
            break;
        }
        default: {
            fprintf(stderr, "Bad variant %s in write32\n", e->class_name().c_str());
            abort();
        }
        }
    }

    /* Returns true if W has an even number of bits set; false for an odd number */
    Word parity(Word w) {
        Word p01 = policy.xor_(extract(w,0,1), extract(w,1,2));
        Word p23 = policy.xor_(extract(w,2,3), extract(w,3,4));
        Word p45 = policy.xor_(extract(w,4,5), extract(w,5,6));
        Word p67 = policy.xor_(extract(w,6,7), extract(w,7,8));
        Word p0123 = policy.xor_(p01, p23);
        Word p4567 = policy.xor_(p45, p67);
        return policy.invert(policy.xor_(p0123, p4567));
    }

    /* Sets flags: parity, sign, and zero */
    void setFlagsForResult(const Word& result, size_t Len) {
        policy.writeFlag(x86_flag_pf, parity(extract(result,0,8)));
        policy.writeFlag(x86_flag_sf, extract(result,Len-1,Len));
        policy.writeFlag(x86_flag_zf, policy.equalToZero(result));
    }

    /* Sets flags conditionally. Sets parity, sign, and zero flags if COND is true. */
    void setFlagsForResult(const Word& result, Word cond, size_t Len) {
        policy.writeFlag(x86_flag_pf, policy.ite(cond, parity(extract(result,0,8)), policy.readFlag(x86_flag_pf)));
        policy.writeFlag(x86_flag_sf, policy.ite(cond, extract(result,Len-1,Len), policy.readFlag(x86_flag_sf)));
        policy.writeFlag(x86_flag_zf, policy.ite(cond, policy.equalToZero(result), policy.readFlag(x86_flag_zf)));
    }

    /* Adds A and B and adjusts condition flags. Can be used for subtraction if B is two's complement and invertCarries is set. */
    Word doAddOperation(const Word& a, const Word& b, bool invertCarries, Word carryIn,size_t Len) {
        Word carries = number(0,Len);
        Word result = policy.addWithCarries(a, b, invertMaybe(carryIn, invertCarries), carries/*out*/, Len);
        setFlagsForResult(result,Len);
        policy.writeFlag(x86_flag_af, invertMaybe(extract(carries,3,4), invertCarries));
        policy.writeFlag(x86_flag_cf, invertMaybe(extract(carries,Len-1,Len), invertCarries));
        policy.writeFlag(x86_flag_of, policy.xor_(extract(carries,Len-1,Len), extract(carries,Len-2,Len-1)));
        return result;
    }

    /* Conditionally adds A and B and adjusts condition flags. Can be used for subtraction if B is two's complement and
     * invertCarries is set. Does nothing if COND is false. */ 
    Word doAddOperation(const Word& a, const Word& b, bool invertCarries, Word carryIn, Word cond, size_t Len) {
        Word carries = number(0, Len);
        Word result = policy.addWithCarries(a, b, invertMaybe(carryIn, invertCarries), carries/*out*/, Len);
        setFlagsForResult(result, cond, Len);
        policy.writeFlag(x86_flag_af,
                         policy.ite(cond,
                                    invertMaybe(extract(carries,3,4), invertCarries),
                                    policy.readFlag(x86_flag_af)));
        policy.writeFlag(x86_flag_cf,
                         policy.ite(cond,
                                    invertMaybe(extract(carries,Len-1,Len), invertCarries),
                                    policy.readFlag(x86_flag_cf)));
        policy.writeFlag(x86_flag_of,
                         policy.ite(cond,
                                    policy.xor_(extract(carries,Len-1,Len), 
                                                extract(carries,Len-2,Len-1)),
                                    policy.readFlag(x86_flag_of)));
        return result;
    }

    /* Does increment (decrement with DEC set), and adjusts condition flags. */
    Word doIncOperation(const Word& a, bool dec, bool setCarry, size_t Len) {
        Word carries = number(0,Len);
        Word result = policy.addWithCarries(a, number(dec ? -1 : 1,Len), policy.false_(), carries/*out*/, Len);
        setFlagsForResult(result, Len);
        policy.writeFlag(x86_flag_af, invertMaybe(extract(carries,3,4), dec));
        policy.writeFlag(x86_flag_of, policy.xor_(extract(carries,Len-1,Len), extract(carries,Len-2,Len-1)));
        if (setCarry)
            policy.writeFlag(x86_flag_cf, invertMaybe(extract(carries,Len-1,Len), dec));
        return result;
    }

    /* Virtual so that we can subclass X86InstructionSemantics and have an opportunity to override the translation of any
     * instruction. */
    virtual void translate(SgAsmx86Instruction* insn) {
        policy.writeIP(number((unsigned int)(insn->get_address() + insn->get_raw_bytes().size())));
        X86InstructionKind kind = insn->get_kind();
        const SgAsmExpressionPtrList& operands = insn->get_operandList()->get_operands();
        switch (kind) {
            
        case x86_mov: {
            ROSE_ASSERT(operands.size() == 2);
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: write8(operands[0], read8(operands[1])); break;
            case 2: write16(operands[0], read16(operands[1])); break;
            case 4: write32(operands[0], read32(operands[1])); break;
            default: ROSE_ASSERT("Bad size"); break;
            }
            break;
        }

        case x86_xchg: {
            ROSE_ASSERT(operands.size() == 2);
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: {
                Word temp = read8(operands[1]);
                write8(operands[1], read8(operands[0]));
                write8(operands[0], temp);
                break;
            }
            case 2: {
                Word temp = read16(operands[1]);
                write16(operands[1], read16(operands[0]));
                write16(operands[0], temp);
                break;
            }
            case 4: {
                Word temp = read32(operands[1]);
                write32(operands[1], read32(operands[0]));
                write32(operands[0], temp);
                break;
            }
            default:
                ROSE_ASSERT("Bad size");
                break;
            }
            break;
        }

        case x86_movzx: {
            ROSE_ASSERT(operands.size() == 2);
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 2: {
                write16(operands[0], policy.concat(read8(operands[1]), number(0,8)));
                break;
            }
            case 4: {
                switch (numBytesInAsmType(operands[1]->get_type())) {
                case 1: write32(operands[0], policy.concat(read8(operands[1]), number(0,24))); break;
                case 2: write32(operands[0], policy.concat(read16(operands[1]), number(0,16))); break;
                default: ROSE_ASSERT("Bad size");
                }
                break;
            }
            default:
                ROSE_ASSERT("Bad size");
                break;
            }
            break;
        }

        case x86_movsx: {
            ROSE_ASSERT(operands.size() == 2);
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 2: {
                Word op1 = read8(operands[1]);
                Word result = signExtend(op1,8,16);
                write16(operands[0], result);
                break;
            }
            case 4: {
                switch (numBytesInAsmType(operands[1]->get_type())) {
                case 1: {
                    Word op1 = read8(operands[1]);
                    Word result = signExtend(op1,8,32);
                    write32(operands[0], result);
                    break;
                }
                case 2: {
                    Word op1 = read16(operands[1]);
                    Word result = signExtend(op1,16,32);
                    write32(operands[0], result);
                    break;
                }
                default:
                    ROSE_ASSERT("Bad size");
                }
                break;
            }
            default:
                ROSE_ASSERT("Bad size");
                break;
            }
            break;
        }

        case x86_cbw: {
            ROSE_ASSERT(operands.size() == 0);
            updateGPRLowWord(x86_gpr_ax, signExtend(extract(policy.readGPR(x86_gpr_ax),0,8),8,16));
            break;
        }

        case x86_cwde: {
            ROSE_ASSERT(operands.size() == 0);
            policy.writeGPR(x86_gpr_ax, signExtend(extract(policy.readGPR(x86_gpr_ax),0,16),16,32));
            break;
        }

        case x86_cwd: {
            ROSE_ASSERT(operands.size() == 0);
            updateGPRLowWord(x86_gpr_dx, extract(signExtend(extract(policy.readGPR(x86_gpr_ax),0,16),16,32),16,32));
            break;
        }

        case x86_cdq: {
            ROSE_ASSERT(operands.size() == 0);
            policy.writeGPR(x86_gpr_dx, extract(signExtend(policy.readGPR(x86_gpr_ax),32,64),32,64));
            break;
        }

        case x86_lea: {
            ROSE_ASSERT(operands.size() == 2);
            write32(operands[0], readEffectiveAddress(operands[1]));
            break;
        }

        case x86_and: {
            ROSE_ASSERT(operands.size() == 2);
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: {
                Word result = policy.and_(read8(operands[0]), read8(operands[1]));
                setFlagsForResult(result,8);
                write8(operands[0], result);
                break;
            }
            case 2: {
                Word result = policy.and_(read16(operands[0]), read16(operands[1]));
                setFlagsForResult(result,16);
                write16(operands[0], result);
                break;
            }
            case 4: {
                Word result = policy.and_(read32(operands[0]), read32(operands[1]));
                setFlagsForResult(result,32);
                write32(operands[0], result);
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
                break;
            }
            policy.writeFlag(x86_flag_of, policy.false_());
            policy.writeFlag(x86_flag_af, policy.undefined_());
            policy.writeFlag(x86_flag_cf, policy.false_());
            break;
        }

        case x86_or: {
            ROSE_ASSERT(operands.size() == 2);
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: {
                Word result = policy.or_(read8(operands[0]), read8(operands[1]));
                setFlagsForResult(result,8);
                write8(operands[0], result);
                break;
            }
            case 2: {
                Word result = policy.or_(read16(operands[0]), read16(operands[1]));
                setFlagsForResult(result,16);
                write16(operands[0], result);
                break;
            }
            case 4: {
                Word result = policy.or_(read32(operands[0]), read32(operands[1]));
                setFlagsForResult(result,32);
                write32(operands[0], result);
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
                break;
            }
            policy.writeFlag(x86_flag_of, policy.false_());
            policy.writeFlag(x86_flag_af, policy.undefined_());
            policy.writeFlag(x86_flag_cf, policy.false_());
            break;
        }

        case x86_test: {
            ROSE_ASSERT(operands.size() == 2);
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: {
                Word result = policy.and_(read8(operands[0]), read8(operands[1]));
                setFlagsForResult(result,8);
                break;
            }
            case 2: {
                Word result = policy.and_(read16(operands[0]), read16(operands[1]));
                setFlagsForResult(result,16);
                break;
            }
            case 4: {
                Word result = policy.and_(read32(operands[0]), read32(operands[1]));
                setFlagsForResult(result,32);
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
                break;
            }
            policy.writeFlag(x86_flag_of, policy.false_());
            policy.writeFlag(x86_flag_af, policy.undefined_());
            policy.writeFlag(x86_flag_cf, policy.false_());
            break;
        }

        case x86_xor: {
            ROSE_ASSERT(operands.size() == 2);
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: {
                Word result = policy.xor_(read8(operands[0]), read8(operands[1]));
                setFlagsForResult(result,8);
                write8(operands[0], result);
                break;
            }
            case 2: {
                Word result = policy.xor_(read16(operands[0]), read16(operands[1]));
                setFlagsForResult(result,16);
                write16(operands[0], result);
                break;
            }
            case 4: {
                Word result = policy.xor_(read32(operands[0]), read32(operands[1]));
                setFlagsForResult(result,32);
                write32(operands[0], result);
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
                break;
            }
            policy.writeFlag(x86_flag_of, policy.false_());
            policy.writeFlag(x86_flag_af, policy.undefined_());
            policy.writeFlag(x86_flag_cf, policy.false_());
            break;
        }

        case x86_not: {
            ROSE_ASSERT(operands.size() == 1);
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: {
                Word result = policy.invert(read8(operands[0]));
                write8(operands[0], result);
                break;
            }
            case 2: {
                Word result = policy.invert(read16(operands[0]));
                write16(operands[0], result);
                break;
            }
            case 4: {
                Word result = policy.invert(read32(operands[0]));
                write32(operands[0], result);
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
                break;
            }
            break;
        }

        case x86_add: {
            ROSE_ASSERT(operands.size() == 2);
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: {
                Word result = doAddOperation(read8(operands[0]), read8(operands[1]), false, policy.false_(), 8);
                write8(operands[0], result);
                break;
            }
            case 2: {
                Word result = doAddOperation(read16(operands[0]), read16(operands[1]), false, policy.false_(), 16);
                write16(operands[0], result);
                break;
            }
            case 4: {
                Word result = doAddOperation(read32(operands[0]), read32(operands[1]), false, policy.false_(), 32);
                write32(operands[0], result);
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
                break;
            }
            break;
        }

        case x86_adc: {
            ROSE_ASSERT(operands.size() == 2);
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: {
                Word result = doAddOperation(read8(operands[0]), read8(operands[1]), false,
                                                policy.readFlag(x86_flag_cf), 8);
                write8(operands[0], result);
                break;
            }
            case 2: {
                Word result = doAddOperation(read16(operands[0]), read16(operands[1]), false,
                                                 policy.readFlag(x86_flag_cf), 16);
                write16(operands[0], result);
                break;
            }
            case 4: {
                Word result = doAddOperation(read32(operands[0]), read32(operands[1]), false,
                                                 policy.readFlag(x86_flag_cf), 32);
                write32(operands[0], result);
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
                break;
            }
            break;
        }

        case x86_sub: {
            ROSE_ASSERT(operands.size() == 2);
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: {
                Word result = doAddOperation(read8(operands[0]), policy.invert(read8(operands[1])), true,
                                                policy.false_(), 8);
                write8(operands[0], result);
                break;
            }
            case 2: {
                Word result = doAddOperation(read16(operands[0]), policy.invert(read16(operands[1])), true,
                                                 policy.false_(), 16);
                write16(operands[0], result);
                break;
            }
            case 4: {
                Word result = doAddOperation(read32(operands[0]), policy.invert(read32(operands[1])), true,
                                                 policy.false_(), 32);
                write32(operands[0], result);
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
                break;
            }
            break;
        }

        case x86_sbb: {
            ROSE_ASSERT(operands.size() == 2);
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: {
                Word result = doAddOperation(read8(operands[0]), policy.invert(read8(operands[1])), true,
                                                policy.readFlag(x86_flag_cf), 8);
                write8(operands[0], result);
                break;
            }
            case 2: {
                Word result = doAddOperation(read16(operands[0]), policy.invert(read16(operands[1])), true,
                                                 policy.readFlag(x86_flag_cf), 16);
                write16(operands[0], result);
                break;
            }
            case 4: {
                Word result = doAddOperation(read32(operands[0]), policy.invert(read32(operands[1])), true,
                                                 policy.readFlag(x86_flag_cf), 32);
                write32(operands[0], result);
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
                break;
            }
            break;
        }

        case x86_cmp: {
            ROSE_ASSERT(operands.size() == 2);
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: {
                doAddOperation(read8(operands[0]), policy.invert(read8(operands[1])), true, policy.false_(), 8);
                break;
            }
            case 2: {
                doAddOperation(read16(operands[0]), policy.invert(read16(operands[1])), true, policy.false_(), 16);
                break;
            }
            case 4: {
                doAddOperation(read32(operands[0]), policy.invert(read32(operands[1])), true, policy.false_(), 32);
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
                break;
            }
            break;
        }

        case x86_neg: {
            ROSE_ASSERT(operands.size() == 1);
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: {
                Word result = doAddOperation(number(0,8), policy.invert(read8(operands[0])), true,
                                                policy.false_(), 8);
                write8(operands[0], result);
                break;
            }
            case 2: {
                Word result = doAddOperation(number(0,16), policy.invert(read16(operands[0])), true,
                                                 policy.false_(), 16);
                write16(operands[0], result);
                break;
            }
            case 4: {
                Word result = doAddOperation(number(0,32), policy.invert(read32(operands[0])), true,
                                                 policy.false_(), 32);
                write32(operands[0], result);
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
                break;
            }
            break;
        }

        case x86_inc: {
            ROSE_ASSERT(operands.size() == 1);
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: {
                Word result = doIncOperation(read8(operands[0]), false, false, 8);
                write8(operands[0], result);
                break;
            }
            case 2: {
                Word result = doIncOperation(read16(operands[0]), false, false, 16);
                write16(operands[0], result);
                break;
            }
            case 4: {
                Word result = doIncOperation(read32(operands[0]), false, false, 32);
                write32(operands[0], result);
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
                break;
            }
            break;
        }

        case x86_dec: {
            ROSE_ASSERT(operands.size() == 1);
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: {
                Word result = doIncOperation(read8(operands[0]), true, false, 8);
                write8(operands[0], result);
                break;
            }
            case 2: {
                Word result = doIncOperation(read16(operands[0]), true, false, 16);
                write16(operands[0], result);
                break;
            }
            case 4: {
                Word result = doIncOperation(read32(operands[0]), true, false, 32);
                write32(operands[0], result);
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
                break;
            }
            break;
        }

        case x86_cmpxchg: {
            ROSE_ASSERT(operands.size() == 2);
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: {
                Word op0 = read8(operands[0]);
                Word oldAx = extract(policy.readGPR(x86_gpr_ax), 0, 8);
                doAddOperation(oldAx, policy.invert(op0), true, policy.false_(), 8);
                write8(operands[0], policy.ite(policy.readFlag(x86_flag_zf), read8(operands[1]), op0));
                updateGPRLowByte(x86_gpr_ax, policy.ite(policy.readFlag(x86_flag_zf), oldAx, op0));
                break;
            }
            case 2: {
                Word op0 = read16(operands[0]);
                Word oldAx = extract(policy.readGPR(x86_gpr_ax), 0, 16);
                doAddOperation(oldAx, policy.invert(op0), true, policy.false_(), 16);
                write16(operands[0], policy.ite(policy.readFlag(x86_flag_zf), read16(operands[1]), op0));
                updateGPRLowWord(x86_gpr_ax, policy.ite(policy.readFlag(x86_flag_zf), oldAx, op0));
                break;
            }
            case 4: {
                Word op0 = read32(operands[0]);
                Word oldAx = policy.readGPR(x86_gpr_ax);
                doAddOperation(oldAx, policy.invert(op0), true, policy.false_(), 32);
                write32(operands[0], policy.ite(policy.readFlag(x86_flag_zf), read32(operands[1]), op0));
                policy.writeGPR(x86_gpr_ax, policy.ite(policy.readFlag(x86_flag_zf), oldAx, op0));
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
                break;
            }
            break;
        }

        case x86_shl: {
            Word shiftCount = extract(read8(operands[1]), 0, 5);
            Word shiftCountZero = policy.equalToZero(shiftCount);
            policy.writeFlag(x86_flag_af, policy.ite(shiftCountZero, policy.readFlag(x86_flag_af), policy.undefined_()));
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: {
                Word op = read8(operands[0]);
                Word output = policy.shiftLeft(op, shiftCount);
                Word newCf = policy.ite(shiftCountZero,
                                           policy.readFlag(x86_flag_cf),
                                           extract(policy.shiftLeft(op, policy.add(shiftCount, number(7,5))),7,8));
                policy.writeFlag(x86_flag_cf, newCf);
                policy.writeFlag(x86_flag_of, policy.ite(shiftCountZero,
                                                         policy.readFlag(x86_flag_of),
                                                         policy.xor_(extract(output,7,8), newCf)));
                write8(operands[0], output);
                setFlagsForResult(output, policy.invert(shiftCountZero), 8);
                break;
            }
            case 2: {
                Word op = read16(operands[0]);
                Word output = policy.shiftLeft(op, shiftCount);
                Word newCf = policy.ite(shiftCountZero,
                                           policy.readFlag(x86_flag_cf),
                                           extract(policy.shiftLeft(op, policy.add(shiftCount, number(15, 5))),15,16));
                policy.writeFlag(x86_flag_cf, newCf);
                policy.writeFlag(x86_flag_of, policy.ite(shiftCountZero,
                                                         policy.readFlag(x86_flag_of),
                                                         policy.xor_(extract(output,15,16), newCf)));
                write16(operands[0], output);
                setFlagsForResult(output, policy.invert(shiftCountZero), 16);
                break;
            }
            case 4: {
                Word op = read32(operands[0]);
                Word output = policy.shiftLeft(op, shiftCount);
                Word newCf = policy.ite(shiftCountZero,
                                        policy.readFlag(x86_flag_cf),
                                        extract(policy.shiftLeft(op, policy.add(shiftCount, number(31,5))),31,32));
                policy.writeFlag(x86_flag_cf, newCf);
                policy.writeFlag(x86_flag_of, policy.ite(shiftCountZero,
                                                         policy.readFlag(x86_flag_of),
                                                         policy.xor_(extract(output,31,32), newCf)));
                write32(operands[0], output);
                setFlagsForResult(output, policy.invert(shiftCountZero), 32);
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
            }
            break;
        }

        case x86_shr: {
            Word shiftCount = extract(read8(operands[1]),0,5);
            Word shiftCountZero = policy.equalToZero(shiftCount);
            policy.writeFlag(x86_flag_af, policy.ite(shiftCountZero, policy.readFlag(x86_flag_af), policy.undefined_()));
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: {
                Word op = read8(operands[0]);
                Word output = policy.shiftRight(op, shiftCount);
                Word newCf = policy.ite(shiftCountZero,
                                           policy.readFlag(x86_flag_cf),
                                           extract(policy.shiftRight(op, policy.add(shiftCount, number(7,5))),0,1));
                policy.writeFlag(x86_flag_cf, newCf);
                policy.writeFlag(x86_flag_of, policy.ite(shiftCountZero, policy.readFlag(x86_flag_of), extract(op,7,8)));
                write8(operands[0], output);
                setFlagsForResult(output, policy.invert(shiftCountZero), 8);
                break;
            }
            case 2: {
                Word op = read16(operands[0]);
                Word output = policy.shiftRight(op, shiftCount);
                Word newCf = policy.ite(shiftCountZero,
                                           policy.readFlag(x86_flag_cf),
                                           extract(policy.shiftRight(op, policy.add(shiftCount, number(15,5))),0,1));
                policy.writeFlag(x86_flag_cf, newCf);
                policy.writeFlag(x86_flag_of,
                                 policy.ite(shiftCountZero, policy.readFlag(x86_flag_of), extract(op,15,16)));
                write16(operands[0], output);
                setFlagsForResult(output, policy.invert(shiftCountZero), 16);
                break;
            }
            case 4: {
                Word op = read32(operands[0]);
                Word output = policy.shiftRight(op, shiftCount);
                Word newCf = policy.ite(shiftCountZero,
                                           policy.readFlag(x86_flag_cf),
                                           extract(policy.shiftRight(op, policy.add(shiftCount, number(31,5))),0,1));
                policy.writeFlag(x86_flag_cf, newCf);
                policy.writeFlag(x86_flag_of, policy.ite(shiftCountZero,
                                                         policy.readFlag(x86_flag_of),
                                                         extract(op,31,32)));
                write32(operands[0], output);
                setFlagsForResult(output, policy.invert(shiftCountZero), 32);
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
            }
            break;
        }

        case x86_sar: {
            Word shiftCount = extract(read8(operands[1]),0,5);
            Word shiftCountZero = policy.equalToZero(shiftCount);
            Word shiftCountNotZero = policy.invert(shiftCountZero);
            policy.writeFlag(x86_flag_af, policy.ite(shiftCountZero, policy.readFlag(x86_flag_af), policy.undefined_()));
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: {
                Word op = read8(operands[0]);
                Word output = policy.shiftRightArithmetic(op, shiftCount);
                Word newCf = policy.ite(shiftCountZero,
                                           policy.readFlag(x86_flag_cf),
                                           extract(policy.shiftRight(op, policy.add(shiftCount, number(7,5))),0,1));
                policy.writeFlag(x86_flag_cf, newCf);
                /* No change with sc = 0, clear when sc = 1, undefined otherwise */
                policy.writeFlag(x86_flag_of, policy.ite(shiftCountZero,
                                                         policy.readFlag(x86_flag_of),
                                                         policy.false_()));
                write8(operands[0], output);
                setFlagsForResult(output, shiftCountNotZero, 8);
                break;
            }
            case 2: {
                Word op = read16(operands[0]);
                Word output = policy.shiftRightArithmetic(op, shiftCount);
                Word newCf = policy.ite(shiftCountZero,
                                           policy.readFlag(x86_flag_cf),
                                           extract(policy.shiftRight(op, policy.add(shiftCount, number(15,5))),0,1));
                policy.writeFlag(x86_flag_cf, newCf);
                /* No change with sc = 0, clear when sc = 1, undefined otherwise */
                policy.writeFlag(x86_flag_of, policy.ite(shiftCountZero,
                                                         policy.readFlag(x86_flag_of),
                                                         policy.false_()));
                write16(operands[0], output);
                setFlagsForResult(output, shiftCountNotZero, 16);
                break;
            }
            case 4: {
                Word op = read32(operands[0]);
                Word output = policy.shiftRightArithmetic(op, shiftCount);
                Word newCf = policy.ite(shiftCountZero,
                                           policy.readFlag(x86_flag_cf),
                                           extract(policy.shiftRight(op, policy.add(shiftCount, number(31,5))),0,1));
                policy.writeFlag(x86_flag_cf, newCf);
                /* No change with sc = 0, clear when sc = 1, undefined otherwise */
                policy.writeFlag(x86_flag_of, policy.ite(shiftCountZero,
                                                         policy.readFlag(x86_flag_of),
                                                         policy.false_()));
                write32(operands[0], output);
                setFlagsForResult(output, shiftCountNotZero, 32);
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
            }
            break;
        }

        case x86_rol: {
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: {
                Word op = read8(operands[0]);
                Word shiftCount = extract(read8(operands[1]),0,5);
                Word output = policy.rotateLeft(op, shiftCount);
                policy.writeFlag(x86_flag_cf, policy.ite(policy.equalToZero(shiftCount),
                                                         policy.readFlag(x86_flag_cf),
                                                         extract(output,0,1)));
                policy.writeFlag(x86_flag_of, policy.ite(policy.equalToZero(shiftCount),
                                                         policy.readFlag(x86_flag_of),
                                                         policy.xor_(extract(output,0,1), extract(output,7,8))));
                write8(operands[0], output);
                break;
            }
            case 2: {
                Word op = read16(operands[0]);
                Word shiftCount = extract(read8(operands[1]),0,5);
                Word output = policy.rotateLeft(op, shiftCount);
                policy.writeFlag(x86_flag_cf, policy.ite(policy.equalToZero(shiftCount),
                                                         policy.readFlag(x86_flag_cf),
                                                         extract(output,0,1)));
                policy.writeFlag(x86_flag_of, policy.ite(policy.equalToZero(shiftCount),
                                                         policy.readFlag(x86_flag_of),
                                                         policy.xor_(extract(output,0,1), extract(output,15,16))));
                write16(operands[0], output);
                break;
            }
            case 4: {
                Word op = read32(operands[0]);
                Word shiftCount = extract(read8(operands[1]),0,5);
                Word output = policy.rotateLeft(op, shiftCount);
                policy.writeFlag(x86_flag_cf, policy.ite(policy.equalToZero(shiftCount),
                                                         policy.readFlag(x86_flag_cf),
                                                         extract(output,0,1)));
                policy.writeFlag(x86_flag_of, policy.ite(policy.equalToZero(shiftCount),
                                                         policy.readFlag(x86_flag_of),
                                                         policy.xor_(extract(output,0,1), extract(output,31,32))));
                write32(operands[0], output);
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
            }
            break;
        }

        case x86_ror: {
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: {
                Word op = read8(operands[0]);
                Word shiftCount = extract(read8(operands[1]),0,5);
                Word output = policy.rotateRight(op, shiftCount);
                policy.writeFlag(x86_flag_cf, policy.ite(policy.equalToZero(shiftCount),
                                                         policy.readFlag(x86_flag_cf),
                                                         extract(output,7,8)));
                policy.writeFlag(x86_flag_of, policy.ite(policy.equalToZero(shiftCount),
                                                         policy.readFlag(x86_flag_of),
                                                         policy.xor_(extract(output,6,7), extract(output,7,8))));
                write8(operands[0], output);
                break;
            }
            case 2: {
                Word op = read16(operands[0]);
                Word shiftCount = extract(read8(operands[1]),0,5);
                Word output = policy.rotateRight(op, shiftCount);
                policy.writeFlag(x86_flag_cf, policy.ite(policy.equalToZero(shiftCount),
                                                         policy.readFlag(x86_flag_cf),
                                                         extract(output,15,16)));
                policy.writeFlag(x86_flag_of, policy.ite(policy.equalToZero(shiftCount),
                                                         policy.readFlag(x86_flag_of),
                                                         policy.xor_(extract(output,14,15), extract(output,15,16))));
                write16(operands[0], output);
                break;
            }
            case 4: {
                Word op = read32(operands[0]);
                Word shiftCount = extract(read8(operands[1]),0,5);
                Word output = policy.rotateRight(op, shiftCount);
                policy.writeFlag(x86_flag_cf, policy.ite(policy.equalToZero(shiftCount),
                                                         policy.readFlag(x86_flag_cf),
                                                         extract(output,31,32)));
                policy.writeFlag(x86_flag_of, policy.ite(policy.equalToZero(shiftCount),
                                                         policy.readFlag(x86_flag_of),
                                                         policy.xor_(extract(output,30,31), extract(output,31,32))));
                write32(operands[0], output);
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
            }
            break;
        }

        case x86_shld: {
            Word shiftCount = extract(read8(operands[2]),0,5);
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 2: {
                Word op1 = read16(operands[0]);
                Word op2 = read16(operands[1]);
                Word output1 = policy.shiftLeft(op1, shiftCount);
                Word output2 = policy.ite(policy.equalToZero(shiftCount),
                                              number(0,16),
                                              policy.shiftRight(op2, policy.negate(shiftCount)));
                Word output = policy.or_(output1, output2);
                Word newCf = policy.ite(policy.equalToZero(shiftCount),
                                           policy.readFlag(x86_flag_cf),
                                           extract(policy.shiftLeft(op1, policy.add(shiftCount, number(15,5))),15,16));
                policy.writeFlag(x86_flag_cf, newCf);
                Word newOf = policy.ite(policy.equalToZero(shiftCount),
                                           policy.readFlag(x86_flag_of), 
                                           policy.xor_(extract(output,15,16), newCf));
                policy.writeFlag(x86_flag_of, newOf);
                write16(operands[0], output);
                setFlagsForResult(output, 16);
                policy.writeFlag(x86_flag_af, policy.ite(policy.equalToZero(shiftCount),
                                                         policy.readFlag(x86_flag_af),
                                                         policy.undefined_()));
                break;
            }
            case 4: {
                Word op1 = read32(operands[0]);
                Word op2 = read32(operands[1]);
                Word shiftCount = extract(read8(operands[2]),0,5);
                Word output1 = policy.shiftLeft(op1, shiftCount);
                Word output2 = policy.ite(policy.equalToZero(shiftCount),
                                              number(0,32),
                                              policy.shiftRight(op2, policy.negate(shiftCount)));
                Word output = policy.or_(output1, output2);
                Word newCf = policy.ite(policy.equalToZero(shiftCount),
                                           policy.readFlag(x86_flag_cf),
                                           extract(policy.shiftLeft(op1, policy.add(shiftCount, number(31,5))),31,32));
                policy.writeFlag(x86_flag_cf, newCf);
                Word newOf = policy.ite(policy.equalToZero(shiftCount),
                                           policy.readFlag(x86_flag_of), 
                                           policy.xor_(extract(output,31,32), newCf));
                policy.writeFlag(x86_flag_of, newOf);
                write32(operands[0], output);
                setFlagsForResult(output, 32);
                policy.writeFlag(x86_flag_af, policy.ite(policy.equalToZero(shiftCount),
                                                         policy.readFlag(x86_flag_af),
                                                         policy.undefined_()));
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
            }
            break;
        }

        case x86_shrd: {
            Word shiftCount = extract(read8(operands[2]),0,5);
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 2: {
                Word op1 = read16(operands[0]);
                Word op2 = read16(operands[1]);
                Word output1 = policy.shiftRight(op1, shiftCount);
                Word output2 = policy.ite(policy.equalToZero(shiftCount),
                                              number(0,16),
                                              policy.shiftLeft(op2, policy.negate(shiftCount)));
                Word output = policy.or_(output1, output2);
                Word newCf = policy.ite(policy.equalToZero(shiftCount),
                                           policy.readFlag(x86_flag_cf),
                                           extract(policy.shiftRight(op1, policy.add(shiftCount, number(15,5))),0,1));
                policy.writeFlag(x86_flag_cf, newCf);
                Word newOf = policy.ite(policy.equalToZero(shiftCount),
                                           policy.readFlag(x86_flag_of), 
                                           policy.xor_(extract(output,15,16),
                                                       extract(op1,15,16)));
                policy.writeFlag(x86_flag_of, newOf);
                write16(operands[0], output);
                setFlagsForResult(output, 16);
                policy.writeFlag(x86_flag_af, policy.ite(policy.equalToZero(shiftCount),
                                                         policy.readFlag(x86_flag_af),
                                                         policy.undefined_()));
                break;
            }
            case 4: {
                Word op1 = read32(operands[0]);
                Word op2 = read32(operands[1]);
                Word output1 = policy.shiftRight(op1, shiftCount);
                Word output2 = policy.ite(policy.equalToZero(shiftCount),
                                              number(0,32),
                                              policy.shiftLeft(op2, policy.negate(shiftCount)));
                Word output = policy.or_(output1, output2);
                Word newCf = policy.ite(policy.equalToZero(shiftCount),
                                           policy.readFlag(x86_flag_cf),
                                           extract(policy.shiftRight(op1, policy.add(shiftCount, number(31,5))),0,1));
                policy.writeFlag(x86_flag_cf, newCf);
                Word newOf = policy.ite(policy.equalToZero(shiftCount),
                                           policy.readFlag(x86_flag_of), 
                                           policy.xor_(extract(output,31,32),
                                                       extract(op1,31,32)));
                policy.writeFlag(x86_flag_of, newOf);
                write32(operands[0], output);
                setFlagsForResult(output, 32);
                policy.writeFlag(x86_flag_af, policy.ite(policy.equalToZero(shiftCount),
                                                         policy.readFlag(x86_flag_af),
                                                         policy.undefined_()));
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
            }
            break;
        }

        case x86_bsf: {
            policy.writeFlag(x86_flag_of, policy.undefined_());
            policy.writeFlag(x86_flag_sf, policy.undefined_());
            policy.writeFlag(x86_flag_af, policy.undefined_());
            policy.writeFlag(x86_flag_pf, policy.undefined_());
            policy.writeFlag(x86_flag_cf, policy.undefined_());
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 2: {
                Word op = read16(operands[1]);
                policy.writeFlag(x86_flag_zf, policy.equalToZero(op));
                Word result = policy.ite(policy.readFlag(x86_flag_zf),
                                             read16(operands[0]),
                                             policy.leastSignificantSetBit(op));
                write16(operands[0], result);
                break;
            }
            case 4: {
                Word op = read32(operands[1]);
                policy.writeFlag(x86_flag_zf, policy.equalToZero(op));
                Word result = policy.ite(policy.readFlag(x86_flag_zf),
                                             read32(operands[0]),
                                             policy.leastSignificantSetBit(op));
                write32(operands[0], result);
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
            }
            break;
        }

        case x86_bsr: {
            policy.writeFlag(x86_flag_of, policy.undefined_());
            policy.writeFlag(x86_flag_sf, policy.undefined_());
            policy.writeFlag(x86_flag_af, policy.undefined_());
            policy.writeFlag(x86_flag_pf, policy.undefined_());
            policy.writeFlag(x86_flag_cf, policy.undefined_());
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 2: {
                Word op = read16(operands[1]);
                policy.writeFlag(x86_flag_zf, policy.equalToZero(op));
                Word result = policy.ite(policy.readFlag(x86_flag_zf),
                                             read16(operands[0]),
                                             policy.mostSignificantSetBit(op));
                write16(operands[0], result);
                break;
            }
            case 4: {
                Word op = read32(operands[1]);
                policy.writeFlag(x86_flag_zf, policy.equalToZero(op));
                Word result = policy.ite(policy.readFlag(x86_flag_zf),
                                             read32(operands[0]),
                                             policy.mostSignificantSetBit(op));
                write32(operands[0], result);
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
            }
            break;
        }

        case x86_bts: {
            ROSE_ASSERT(operands.size() == 2);
            /* All flags except CF are undefined */
            policy.writeFlag(x86_flag_of, policy.undefined_());
            policy.writeFlag(x86_flag_sf, policy.undefined_());
            policy.writeFlag(x86_flag_zf, policy.undefined_());
            policy.writeFlag(x86_flag_af, policy.undefined_());
            policy.writeFlag(x86_flag_pf, policy.undefined_());
            if (isSgAsmMemoryReferenceExpression(operands[0]) && isSgAsmx86RegisterReferenceExpression(operands[1])) {
                /* Special case allowing multi-word offsets into memory */
                Word addr = readEffectiveAddress(operands[0]);
                int numBytes = numBytesInAsmType(operands[1]->get_type());
                Word bitnum = numBytes == 2 ? signExtend(read16(operands[1]),16,32) : read32(operands[1]);
                Word adjustedAddr = policy.add(addr, signExtend(extract(bitnum,3,32),29,32));
                Word val = readMemory(getSegregFromMemoryReference(isSgAsmMemoryReferenceExpression(operands[0])),
                                         adjustedAddr, policy.true_(),8);
                Word bitval = extract(policy.rotateRight(val, extract(bitnum, 0, 3)), 0, 1);
                Word result = policy.or_(val, policy.rotateLeft(number(1,8), extract(bitnum,0,3)));
                policy.writeFlag(x86_flag_cf, bitval);
                policy.writeMemory(getSegregFromMemoryReference(isSgAsmMemoryReferenceExpression(operands[0])),
                                   adjustedAddr, result, policy.true_());
            } else {
                /* Simple case */
                switch (numBytesInAsmType(operands[0]->get_type())) {
                case 2: {
                    Word op0 = read16(operands[0]);
                    Word bitnum = extract(read16(operands[1]), 0, 4);
                    Word bitval = extract(policy.rotateRight(op0, bitnum), 0, 1);
                    Word result = policy.or_(op0, policy.rotateLeft(number(1,16), bitnum));
                    policy.writeFlag(x86_flag_cf, bitval);
                    write16(operands[0], result);
                    break;
                }
                case 4: {
                    Word op0 = read32(operands[0]);
                    Word bitnum = extract(read32(operands[1]), 0, 5);
                    Word bitval = extract(policy.rotateRight(op0, bitnum), 0, 1);
                    Word result = policy.or_(op0, policy.rotateLeft(number(1,32), bitnum));
                    policy.writeFlag(x86_flag_cf, bitval);
                    write32(operands[0], result);
                    break;
                }
                default:
                    ROSE_ASSERT(!"Bad size");
                }
            }
            break;
        }

        case x86_imul: {
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: {
                Word op0 = extract(policy.readGPR(x86_gpr_ax), 0, 8);
                Word op1 = read8(operands[0]);
                Word mulResult = policy.signedMultiply(op0, op1);
                updateGPRLowWord(x86_gpr_ax, mulResult);
                Word carry = policy.invert(policy.or_(policy.equalToZero(policy.invert(extract(mulResult, 7, 16))),
                                                         policy.equalToZero(extract(mulResult, 7, 16))));
                policy.writeFlag(x86_flag_cf, carry);
                policy.writeFlag(x86_flag_of, carry);
                break;
            }
            case 2: {
                Word op0 = operands.size() == 1 ?
                    extract(policy.readGPR(x86_gpr_ax), 0, 16) :
                    read16(operands[operands.size() - 2]);
                Word op1 = read16(operands[operands.size() - 1]);
                Word mulResult = policy.signedMultiply(op0, op1);
                if (operands.size() == 1) {
                    updateGPRLowWord(x86_gpr_ax, extract(mulResult, 0, 16));
                    updateGPRLowWord(x86_gpr_dx, extract(mulResult, 16, 32));
                } else {
                    write16(operands[0], extract(mulResult, 0, 16));
                }
                Word carry = policy.invert(policy.or_(policy.equalToZero(policy.invert(extract(mulResult,7,32))),
                                                         policy.equalToZero(extract(mulResult,7,32))));
                policy.writeFlag(x86_flag_cf, carry);
                policy.writeFlag(x86_flag_of, carry);
                break;
            }
            case 4: {
                Word op0 = operands.size() == 1 ? policy.readGPR(x86_gpr_ax) : read32(operands[operands.size() - 2]);
                Word op1 = read32(operands[operands.size() - 1]);
                Word mulResult = policy.signedMultiply(op0, op1);
                if (operands.size() == 1) {
                    policy.writeGPR(x86_gpr_ax, extract(mulResult,0,32));
                    policy.writeGPR(x86_gpr_dx, extract(mulResult,32,64));
                } else {
                    write32(operands[0], extract(mulResult,0,32));
                }
                Word carry = policy.invert(policy.or_(policy.equalToZero(policy.invert(extract(mulResult,7,64))),
                                                         policy.equalToZero(extract(mulResult,7,64))));
                policy.writeFlag(x86_flag_cf, carry);
                policy.writeFlag(x86_flag_of, carry);
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
            }
            policy.writeFlag(x86_flag_sf, policy.undefined_());
            policy.writeFlag(x86_flag_zf, policy.undefined_());
            policy.writeFlag(x86_flag_af, policy.undefined_());
            policy.writeFlag(x86_flag_pf, policy.undefined_());
            break;
        }

        case x86_mul: {
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: {
                Word op0 = extract(policy.readGPR(x86_gpr_ax),0,8);
                Word op1 = read8(operands[0]);
                Word mulResult = policy.unsignedMultiply(op0, op1);
                updateGPRLowWord(x86_gpr_ax, mulResult);
                Word carry = policy.invert(policy.equalToZero(extract(mulResult,8,16)));
                policy.writeFlag(x86_flag_cf, carry);
                policy.writeFlag(x86_flag_of, carry);
                break;
            }
            case 2: {
                Word op0 = extract(policy.readGPR(x86_gpr_ax),0,16);
                Word op1 = read16(operands[0]);
                Word mulResult = policy.unsignedMultiply(op0, op1);
                updateGPRLowWord(x86_gpr_ax, extract(mulResult,0,16));
                updateGPRLowWord(x86_gpr_dx, extract(mulResult,16,32));
                Word carry = policy.invert(policy.equalToZero(extract(mulResult,16,32)));
                policy.writeFlag(x86_flag_cf, carry);
                policy.writeFlag(x86_flag_of, carry);
                break;
            }
            case 4: {
                Word op0 = policy.readGPR(x86_gpr_ax);
                Word op1 = read32(operands[0]);
                Word mulResult = policy.unsignedMultiply(op0, op1);
                policy.writeGPR(x86_gpr_ax, extract(mulResult,0,32));
                policy.writeGPR(x86_gpr_dx, extract(mulResult,32,64));
                Word carry = policy.invert(policy.equalToZero(extract(mulResult,32,64)));
                policy.writeFlag(x86_flag_cf, carry);
                policy.writeFlag(x86_flag_of, carry);
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
            }
            policy.writeFlag(x86_flag_sf, policy.undefined_());
            policy.writeFlag(x86_flag_zf, policy.undefined_());
            policy.writeFlag(x86_flag_af, policy.undefined_());
            policy.writeFlag(x86_flag_pf, policy.undefined_());
            break;
        }

        case x86_idiv: {
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: {
                Word op0 = extract(policy.readGPR(x86_gpr_ax),0,16);
                Word op1 = read8(operands[0]);
                /* if op1 == 0, we should trap */
                Word divResult = policy.signedDivide(op0, op1);
                Word modResult = policy.signedModulo(op0, op1);
                /* if result overflows, we should trap */
                updateGPRLowWord(x86_gpr_ax, policy.concat(extract(divResult,0,8), modResult));
                break;
            }
            case 2: {
                Word op0 = policy.concat(extract(policy.readGPR(x86_gpr_ax),0,16),
                                             extract(policy.readGPR(x86_gpr_dx),0,16));
                Word op1 = read16(operands[0]);
                /* if op1 == 0, we should trap */
                Word divResult = policy.signedDivide(op0, op1);
                Word modResult = policy.signedModulo(op0, op1);
                /* if result overflows, we should trap */
                updateGPRLowWord(x86_gpr_ax, extract(divResult,0,16));
                updateGPRLowWord(x86_gpr_dx, modResult);
                break;
            }
            case 4: {
                Word op0 = policy.concat(policy.readGPR(x86_gpr_ax), policy.readGPR(x86_gpr_dx));
                Word op1 = read32(operands[0]);
                /* if op1 == 0, we should trap */
                Word divResult = policy.signedDivide(op0, op1);
                Word modResult = policy.signedModulo(op0, op1);
                /* if result overflows, we should trap */
                policy.writeGPR(x86_gpr_ax, extract(divResult,0,32));
                policy.writeGPR(x86_gpr_dx, modResult);
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
            }
            policy.writeFlag(x86_flag_sf, policy.undefined_());
            policy.writeFlag(x86_flag_zf, policy.undefined_());
            policy.writeFlag(x86_flag_af, policy.undefined_());
            policy.writeFlag(x86_flag_pf, policy.undefined_());
            policy.writeFlag(x86_flag_cf, policy.undefined_());
            policy.writeFlag(x86_flag_of, policy.undefined_());
            break;
        }

        case x86_div: {
            switch (numBytesInAsmType(operands[0]->get_type())) {
            case 1: {
                Word op0 = extract(policy.readGPR(x86_gpr_ax),0,16);
                Word op1 = read8(operands[0]);
                /* if op1 == 0, we should trap */
                Word divResult = policy.unsignedDivide(op0, op1);
                Word modResult = policy.unsignedModulo(op0, op1);
                /* if extract<8, 16> of divResult is non-zero (overflow), we should trap */
                updateGPRLowWord(x86_gpr_ax, policy.concat(extract(divResult,0,8), modResult));
                break;
            }
            case 2: {
                Word op0 = policy.concat(extract(policy.readGPR(x86_gpr_ax),0,16),
                                             extract(policy.readGPR(x86_gpr_dx),0,16));
                Word op1 = read16(operands[0]);
                /* if op1 == 0, we should trap */
                Word divResult = policy.unsignedDivide(op0, op1);
                Word modResult = policy.unsignedModulo(op0, op1);
                /* if extract<16, 32> of divResult is non-zero (overflow), we should trap */
                updateGPRLowWord(x86_gpr_ax, extract(divResult,0,16));
                updateGPRLowWord(x86_gpr_dx, modResult);
                break;
            }
            case 4: {
                Word op0 = policy.concat(policy.readGPR(x86_gpr_ax), policy.readGPR(x86_gpr_dx));
                Word op1 = read32(operands[0]);
                /* if op1 == 0, we should trap */
                Word divResult = policy.unsignedDivide(op0, op1);
                Word modResult = policy.unsignedModulo(op0, op1);
                /* if extract<32, 64> of divResult is non-zero (overflow), we should trap */
                policy.writeGPR(x86_gpr_ax, extract(divResult,0,32));
                policy.writeGPR(x86_gpr_dx, modResult);
                break;
            }
            default:
                ROSE_ASSERT(!"Bad size");
            }
            policy.writeFlag(x86_flag_sf, policy.undefined_());
            policy.writeFlag(x86_flag_zf, policy.undefined_());
            policy.writeFlag(x86_flag_af, policy.undefined_());
            policy.writeFlag(x86_flag_pf, policy.undefined_());
            policy.writeFlag(x86_flag_cf, policy.undefined_());
            policy.writeFlag(x86_flag_of, policy.undefined_());
            break;
        }

        case x86_aaa: {
            ROSE_ASSERT(operands.size() == 0);
            Word incAh = policy.or_(policy.readFlag(x86_flag_af),
                                    greaterOrEqualToTen(extract(policy.readGPR(x86_gpr_ax),0,4),4));
            updateGPRLowWord(x86_gpr_ax,
                             policy.concat(policy.add(policy.ite(incAh, number(6,4), number(0,4)),
                                                      extract(policy.readGPR(x86_gpr_ax),0,4)),
                                           policy.concat(number(0,4),
                                                         policy.add(policy.ite(incAh, number(1,8), number(0,1)),
                                                                    extract(policy.readGPR(x86_gpr_ax),8,16)))));
            policy.writeFlag(x86_flag_of, policy.undefined_());
            policy.writeFlag(x86_flag_sf, policy.undefined_());
            policy.writeFlag(x86_flag_zf, policy.undefined_());
            policy.writeFlag(x86_flag_pf, policy.undefined_());
            policy.writeFlag(x86_flag_af, incAh);
            policy.writeFlag(x86_flag_cf, incAh);
            break;
        }

        case x86_aas: {
            ROSE_ASSERT(operands.size() == 0);
            Word decAh = policy.or_(policy.readFlag(x86_flag_af),
                                    greaterOrEqualToTen(extract(policy.readGPR(x86_gpr_ax),0,4),4));
            updateGPRLowWord(x86_gpr_ax,
                             policy.concat(policy.add(policy.ite(decAh, number(-6,4), number(0,4)),
                                                      extract(policy.readGPR(x86_gpr_ax),0,4)),
                                           policy.concat(number(0,4),
                                                         policy.add(policy.ite(decAh, number(-1,8), number(0,8)),
                                                                    extract(policy.readGPR(x86_gpr_ax),8,16)))));
            policy.writeFlag(x86_flag_of, policy.undefined_());
            policy.writeFlag(x86_flag_sf, policy.undefined_());
            policy.writeFlag(x86_flag_zf, policy.undefined_());
            policy.writeFlag(x86_flag_pf, policy.undefined_());
            policy.writeFlag(x86_flag_af, decAh);
            policy.writeFlag(x86_flag_cf, decAh);
            break;
        }

        case x86_aam: {
            ROSE_ASSERT(operands.size() == 1);
            Word al = extract(policy.readGPR(x86_gpr_ax),0,8);
            Word divisor = read8(operands[0]);
            Word newAh = policy.unsignedDivide(al, divisor);
            Word newAl = policy.unsignedModulo(al, divisor);
            updateGPRLowWord(x86_gpr_ax, policy.concat(newAl, newAh));
            policy.writeFlag(x86_flag_of, policy.undefined_());
            policy.writeFlag(x86_flag_af, policy.undefined_());
            policy.writeFlag(x86_flag_cf, policy.undefined_());
            setFlagsForResult(newAl, 8);
            break;
        }

        case x86_aad: {
            ROSE_ASSERT(operands.size() == 1);
            Word al = extract(policy.readGPR(x86_gpr_ax),0,8);
            Word ah = extract(policy.readGPR(x86_gpr_ax),8,16);
            Word divisor = read8(operands[0]);
            Word newAl = policy.add(al, extract(policy.unsignedMultiply(ah, divisor),0,8));
            updateGPRLowWord(x86_gpr_ax, policy.concat(newAl, number(0,8)));
            policy.writeFlag(x86_flag_of, policy.undefined_());
            policy.writeFlag(x86_flag_af, policy.undefined_());
            policy.writeFlag(x86_flag_cf, policy.undefined_());
            setFlagsForResult(newAl, 8);
            break;
        }

        case x86_bswap: {
            ROSE_ASSERT(operands.size() == 1);
            Word oldVal = read32(operands[0]);
            Word newVal = policy.concat(extract(oldVal,24,32),
                                            policy.concat(extract(oldVal,16,24),
                                                          policy.concat(extract(oldVal,8,16),
                                                                        extract(oldVal,0,8))));
            write32(operands[0], newVal);
            break;
        }

        case x86_push: {
            ROSE_ASSERT(operands.size() == 1);
            ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32);
            ROSE_ASSERT(insn->get_operandSize() == x86_insnsize_32);
            Word oldSp = policy.readGPR(x86_gpr_sp);
            Word newSp = policy.add(oldSp, number(-4,32));
            policy.writeMemory(x86_segreg_ss, newSp, read32(operands[0]), policy.true_());
            policy.writeGPR(x86_gpr_sp, newSp);
            break;
        }

        case x86_pushad: {
            ROSE_ASSERT(operands.size() == 0);
            ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32);
            Word oldSp = policy.readGPR(x86_gpr_sp);
            Word newSp = policy.add(oldSp, number(-32));
            policy.writeMemory(x86_segreg_ss, newSp, policy.readGPR(x86_gpr_di), policy.true_());
            policy.writeMemory(x86_segreg_ss, policy.add(newSp, number(4)), policy.readGPR(x86_gpr_si), policy.true_());
            policy.writeMemory(x86_segreg_ss, policy.add(newSp, number(8)), policy.readGPR(x86_gpr_bp), policy.true_());
            policy.writeMemory(x86_segreg_ss, policy.add(newSp, number(12)), oldSp, policy.true_());
            policy.writeMemory(x86_segreg_ss, policy.add(newSp, number(16)), policy.readGPR(x86_gpr_bx), policy.true_());
            policy.writeMemory(x86_segreg_ss, policy.add(newSp, number(20)), policy.readGPR(x86_gpr_dx), policy.true_());
            policy.writeMemory(x86_segreg_ss, policy.add(newSp, number(24)), policy.readGPR(x86_gpr_cx), policy.true_());
            policy.writeMemory(x86_segreg_ss, policy.add(newSp, number(28)), policy.readGPR(x86_gpr_ax), policy.true_());
            policy.writeGPR(x86_gpr_sp, newSp);
            break;
        }

        case x86_pushfd: {
            ROSE_ASSERT(operands.size() == 0);
            ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32);
            Word oldSp = policy.readGPR(x86_gpr_sp);
            Word newSp = policy.add(oldSp, number(-4));
            policy.writeMemory(x86_segreg_ss, newSp, readEflags(), policy.true_());
            policy.writeGPR(x86_gpr_sp, newSp);
            break;
        }

        case x86_pop: {
            ROSE_ASSERT(operands.size() == 1);
            ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32);
            ROSE_ASSERT(insn->get_operandSize() == x86_insnsize_32);
            Word oldSp = policy.readGPR(x86_gpr_sp);
            Word newSp = policy.add(oldSp, number(4));
            write32(operands[0], readMemory(x86_segreg_ss, oldSp, policy.true_(),32));
            policy.writeGPR(x86_gpr_sp, newSp);
            break;
        }

        case x86_popad: {
            ROSE_ASSERT(operands.size() == 0);
            ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32);
            Word oldSp = policy.readGPR(x86_gpr_sp);
            Word newSp = policy.add(oldSp, number(32));
            policy.writeGPR(x86_gpr_di, readMemory(x86_segreg_ss, oldSp, policy.true_(), 32));
            policy.writeGPR(x86_gpr_si, readMemory(x86_segreg_ss, policy.add(oldSp, number(4)), policy.true_(), 32));
            policy.writeGPR(x86_gpr_bp, readMemory(x86_segreg_ss, policy.add(oldSp, number(8)), policy.true_(), 32));
            policy.writeGPR(x86_gpr_bx, readMemory(x86_segreg_ss, policy.add(oldSp, number(16)), policy.true_(), 32));
            policy.writeGPR(x86_gpr_dx, readMemory(x86_segreg_ss, policy.add(oldSp, number(20)), policy.true_(), 32));
            policy.writeGPR(x86_gpr_cx, readMemory(x86_segreg_ss, policy.add(oldSp, number(24)), policy.true_(), 32));
            policy.writeGPR(x86_gpr_ax, readMemory(x86_segreg_ss, policy.add(oldSp, number(28)), policy.true_(), 32));
            readMemory(x86_segreg_ss, policy.add(oldSp, number(12)), policy.true_(), 32);
            policy.writeGPR(x86_gpr_sp, newSp);
            break;
        }

        case x86_leave: {
            ROSE_ASSERT(operands.size() == 0);
            policy.writeGPR(x86_gpr_sp, policy.readGPR(x86_gpr_bp));
            ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32);
            ROSE_ASSERT(insn->get_operandSize() == x86_insnsize_32);
            Word oldSp = policy.readGPR(x86_gpr_sp);
            Word newSp = policy.add(oldSp, number(4));
            policy.writeGPR(x86_gpr_bp, readMemory(x86_segreg_ss, oldSp, policy.true_(), 32));
            policy.writeGPR(x86_gpr_sp, newSp);
            break;
        }

        case x86_call: {
            ROSE_ASSERT(operands.size() == 1);
            ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32);
            ROSE_ASSERT(insn->get_operandSize() == x86_insnsize_32);
            Word oldSp = policy.readGPR(x86_gpr_sp);
            Word newSp = policy.add(oldSp, number(-4));
            policy.writeMemory(x86_segreg_ss, newSp, policy.readIP(), policy.true_());
            policy.writeIP(policy.filterCallTarget(read32(operands[0])));
            policy.writeGPR(x86_gpr_sp, newSp);
            break;
        }

        case x86_ret: {
            ROSE_ASSERT(operands.size() <= 1);
            ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32);
            ROSE_ASSERT(insn->get_operandSize() == x86_insnsize_32);
            Word extraBytes = (operands.size() == 1 ? read32(operands[0]) : number(0));
            Word oldSp = policy.readGPR(x86_gpr_sp);
            Word newSp = policy.add(oldSp, policy.add(number(4), extraBytes));
            policy.writeIP(policy.filterReturnTarget(readMemory(x86_segreg_ss, oldSp, policy.true_(), 32)));
            policy.writeGPR(x86_gpr_sp, newSp);
            break;
        }

        case x86_loop: {
            ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32);
            ROSE_ASSERT(insn->get_operandSize() == x86_insnsize_32);
            ROSE_ASSERT(operands.size() == 1);
            Word oldCx = policy.readGPR(x86_gpr_cx);
            Word newCx = policy.add(number(-1), oldCx);
            policy.writeGPR(x86_gpr_cx, newCx);
            Word doLoop = policy.invert(policy.equalToZero(newCx));
            policy.writeIP(policy.ite(doLoop, read32(operands[0]), policy.readIP()));
            break;
        }
        case x86_loopz: {
            ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32);
            ROSE_ASSERT(insn->get_operandSize() == x86_insnsize_32);
            ROSE_ASSERT(operands.size() == 1);
            Word oldCx = policy.readGPR(x86_gpr_cx);
            Word newCx = policy.add(number(-1), oldCx);
            policy.writeGPR(x86_gpr_cx, newCx);
            Word doLoop = policy.and_(policy.invert(policy.equalToZero(newCx)), policy.readFlag(x86_flag_zf));
            policy.writeIP(policy.ite(doLoop, read32(operands[0]), policy.readIP()));
            break;
        }
        case x86_loopnz: {
            ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32);
            ROSE_ASSERT(insn->get_operandSize() == x86_insnsize_32);
            ROSE_ASSERT(operands.size() == 1);
            Word oldCx = policy.readGPR(x86_gpr_cx);
            Word newCx = policy.add(number(-1), oldCx);
            policy.writeGPR(x86_gpr_cx, newCx);
            Word doLoop = policy.and_(policy.invert(policy.equalToZero(newCx)),
                                         policy.invert(policy.readFlag(x86_flag_zf)));
            policy.writeIP(policy.ite(doLoop, read32(operands[0]), policy.readIP()));
            break;
        }

        case x86_jmp: {
            ROSE_ASSERT(operands.size() == 1);
            policy.writeIP(policy.filterIndirectJumpTarget(read32(operands[0])));
            break;
        }


            /* Flag expressions that must be true for a conditional jump to occur. */
#           define FLAGCOMBO_ne    policy.invert(policy.readFlag(x86_flag_zf))
#           define FLAGCOMBO_e     policy.readFlag(x86_flag_zf)
#           define FLAGCOMBO_no    policy.invert(policy.readFlag(x86_flag_of))
#           define FLAGCOMBO_o     policy.readFlag(x86_flag_of)
#           define FLAGCOMBO_ns    policy.invert(policy.readFlag(x86_flag_sf))
#           define FLAGCOMBO_s     policy.readFlag(x86_flag_sf)
#           define FLAGCOMBO_po    policy.invert(policy.readFlag(x86_flag_pf))
#           define FLAGCOMBO_pe    policy.readFlag(x86_flag_pf)
#           define FLAGCOMBO_ae    policy.invert(policy.readFlag(x86_flag_cf))
#           define FLAGCOMBO_b     policy.readFlag(x86_flag_cf)
#           define FLAGCOMBO_be    policy.or_(FLAGCOMBO_b, FLAGCOMBO_e)
#           define FLAGCOMBO_a     policy.and_(FLAGCOMBO_ae, FLAGCOMBO_ne)
#           define FLAGCOMBO_l     policy.xor_(policy.readFlag(x86_flag_sf), policy.readFlag(x86_flag_of))
#           define FLAGCOMBO_ge    policy.invert(policy.xor_(policy.readFlag(x86_flag_sf), policy.readFlag(x86_flag_of)))
#           define FLAGCOMBO_le    policy.or_(FLAGCOMBO_e, FLAGCOMBO_l)
#           define FLAGCOMBO_g     policy.and_(FLAGCOMBO_ge, FLAGCOMBO_ne)
#           define FLAGCOMBO_cxz   policy.equalToZero(extract(policy.readGPR(x86_gpr_cx), 0, 16))
#           define FLAGCOMBO_ecxz  policy.equalToZero(policy.readGPR(x86_gpr_cx))

#           define JUMP(tag) {                                  \
                ROSE_ASSERT(operands.size() == 1);              \
                policy.writeIP(policy.ite(FLAGCOMBO_##tag,      \
                                          read32(operands[0]),  \
                                          policy.readIP()));    \
            }
        case x86_jne:   JUMP(ne);   break;
        case x86_je:    JUMP(e);    break;
        case x86_jno:   JUMP(no);   break;
        case x86_jo:    JUMP(o);    break;
        case x86_jpo:   JUMP(po);   break;
        case x86_jpe:   JUMP(pe);   break;
        case x86_jns:   JUMP(ns);   break;
        case x86_js:    JUMP(s);    break;
        case x86_jae:   JUMP(ae);   break;
        case x86_jb:    JUMP(b);    break;
        case x86_jbe:   JUMP(be);   break;
        case x86_ja:    JUMP(a);    break;
        case x86_jle:   JUMP(le);   break;
        case x86_jg:    JUMP(g);    break;
        case x86_jge:   JUMP(ge);   break;
        case x86_jl:    JUMP(l);    break;
        case x86_jcxz:  JUMP(cxz);  break;
        case x86_jecxz: JUMP(ecxz); break;
#           undef JUMP

#           define SET(tag) {                                           \
                ROSE_ASSERT(operands.size() == 1);                      \
                write8(operands[0], policy.concat(FLAGCOMBO_##tag, number(0,7))); \
            }
        case x86_setne: SET(ne); break;
        case x86_sete:  SET(e);  break;
        case x86_setno: SET(no); break;
        case x86_seto:  SET(o);  break;
        case x86_setpo: SET(po); break;
        case x86_setpe: SET(pe); break;
        case x86_setns: SET(ns); break;
        case x86_sets:  SET(s);  break;
        case x86_setae: SET(ae); break;
        case x86_setb:  SET(b);  break;
        case x86_setbe: SET(be); break;
        case x86_seta:  SET(a);  break;
        case x86_setle: SET(le); break;
        case x86_setg:  SET(g);  break;
        case x86_setge: SET(ge); break;
        case x86_setl:  SET(l);  break;
#           undef SET
                
#           define CMOV(tag) {                                          \
                ROSE_ASSERT(operands.size() == 2);                      \
                switch (numBytesInAsmType(operands[0]->get_type())) {   \
                case 2: write16(operands[0], policy.ite(FLAGCOMBO_##tag, read16(operands[1]), read16(operands[0]))); break; \
                case 4: write32(operands[0], policy.ite(FLAGCOMBO_##tag, read32(operands[1]), read32(operands[0]))); break; \
                default: ROSE_ASSERT("Bad size"); break;                \
                }                                                       \
            }
        case x86_cmovne:    CMOV(ne);       break;
        case x86_cmove:     CMOV(e);        break;
        case x86_cmovno:    CMOV(no);       break;
        case x86_cmovo:     CMOV(o);        break;
        case x86_cmovpo:    CMOV(po);       break;
        case x86_cmovpe:    CMOV(pe);       break;
        case x86_cmovns:    CMOV(ns);       break;
        case x86_cmovs:     CMOV(s);        break;
        case x86_cmovae:    CMOV(ae);       break;
        case x86_cmovb:     CMOV(b);        break;
        case x86_cmovbe:    CMOV(be);       break;
        case x86_cmova:     CMOV(a);        break;
        case x86_cmovle:    CMOV(le);       break;
        case x86_cmovg:     CMOV(g);        break;
        case x86_cmovge:    CMOV(ge);       break;
        case x86_cmovl:     CMOV(l);        break;
#           undef CMOV

            /* The flag expressions are no longer needed */
#           undef FLAGCOMBO_ne
#           undef FLAGCOMBO_e
#           undef FLAGCOMBO_ns
#           undef FLAGCOMBO_s
#           undef FLAGCOMBO_ae
#           undef FLAGCOMBO_b
#           undef FLAGCOMBO_be
#           undef FLAGCOMBO_a
#           undef FLAGCOMBO_l
#           undef FLAGCOMBO_ge
#           undef FLAGCOMBO_le
#           undef FLAGCOMBO_g
#           undef FLAGCOMBO_cxz
#           undef FLAGCOMBO_ecxz

        case x86_cld: {
            ROSE_ASSERT(operands.size() == 0);
            policy.writeFlag(x86_flag_df, policy.false_());
            break;
        }

        case x86_std: {
            ROSE_ASSERT(operands.size() == 0);
            policy.writeFlag(x86_flag_df, policy.true_());
            break;
        }

        case x86_clc: {
            ROSE_ASSERT(operands.size() == 0);
            policy.writeFlag(x86_flag_cf, policy.false_());
            break;
        }

        case x86_stc: {
            ROSE_ASSERT(operands.size() == 0);
            policy.writeFlag(x86_flag_cf, policy.true_());
            break;
        }

        case x86_cmc: {
            ROSE_ASSERT(operands.size() == 0);
            policy.writeFlag(x86_flag_cf, policy.invert(policy.readFlag(x86_flag_cf)));
            break;
        }

        case x86_nop:
            break;

#           define STRINGOP_LOAD_SI(len, cond)                          \
            readMemory((insn->get_segmentOverride() == x86_segreg_none ? \
                        x86_segreg_ds :                    \
                        insn->get_segmentOverride()),      \
                       policy.readGPR(x86_gpr_si),         \
                       (cond),8*len)
            
#           define STRINGOP_LOAD_DI(len, cond)                          \
            readMemory(x86_segreg_es, policy.readGPR(x86_gpr_di), (cond), 8*len)

            /* If true, repeat this instruction, otherwise go to the next one */
#           define STRINGOP_LOOP_E                                      \
            policy.writeIP(policy.ite(policy.and_(ecxNotZero, policy.readFlag(x86_flag_zf)), \
                                      number((uint32_t)(insn->get_address())), \
                                      policy.readIP()))

            /* If true, repeat this instruction, otherwise go to the next one */
#           define STRINGOP_LOOP_NE                                     \
            policy.writeIP(policy.ite(policy.and_(ecxNotZero, policy.invert(policy.readFlag(x86_flag_zf))), \
                                      number((uint32_t)(insn->get_address())), \
                                      policy.readIP()))

#           define REP_SCAS(suffix, len, repsuffix, loopmacro) {        \
                ROSE_ASSERT(operands.size() == 0);                      \
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32); \
                Word ecxNotZero = stringop_setup_loop();             \
                doAddOperation(extract(policy.readGPR(x86_gpr_ax),0,len*8), \
                                          policy.invert(STRINGOP_LOAD_DI(len, ecxNotZero)), \
                                          true,                         \
                                          policy.false_(),              \
                                          ecxNotZero,len*8);                 \
                policy.writeGPR(x86_gpr_di,                             \
                                policy.add(policy.readGPR(x86_gpr_di),  \
                                           policy.ite(policy.readFlag(x86_flag_df), number(-(len)), number(len)))); \
                loopmacro;                                              \
            }
        case x86_repne_scasb: REP_SCAS(b, 1, ne, STRINGOP_LOOP_NE); break;
        case x86_repne_scasw: REP_SCAS(w, 2, ne, STRINGOP_LOOP_NE); break;
        case x86_repne_scasd: REP_SCAS(d, 4, ne, STRINGOP_LOOP_NE); break;
        case x86_repe_scasb:  REP_SCAS(b, 1, e,  STRINGOP_LOOP_E);  break;
        case x86_repe_scasw:  REP_SCAS(w, 2, e,  STRINGOP_LOOP_E);  break;
        case x86_repe_scasd:  REP_SCAS(d, 4, e,  STRINGOP_LOOP_E);  break;
#           undef REP_SCAS

#           define SCAS(suffix, len) {                                  \
                ROSE_ASSERT(operands.size() == 0);                      \
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32); \
                doAddOperation(extract(policy.readGPR(x86_gpr_ax), 0, len*8), \
                                       policy.invert(STRINGOP_LOAD_DI(len, policy.true_())), \
                                       true,                         \
                                       policy.false_(),              \
                                       policy.true_(), len*8);             \
                policy.writeGPR(x86_gpr_di,                             \
                                policy.add(policy.readGPR(x86_gpr_di),  \
                                           policy.ite(policy.readFlag(x86_flag_df), number(-(len)), number(len)))); \
            }
        case x86_scasb: SCAS(b, 1); break;
        case x86_scasw: SCAS(w, 2); break;
        case x86_scasd: SCAS(d, 4); break;
#           undef SCAS

#           define REP_CMPS(suffix, len, repsuffix, loopmacro) {        \
                ROSE_ASSERT(operands.size() == 0);                      \
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32); \
                Word ecxNotZero = stringop_setup_loop();             \
                doAddOperation(STRINGOP_LOAD_SI(len, ecxNotZero), \
                                          policy.invert(STRINGOP_LOAD_DI(len, ecxNotZero)), \
                                          true,                         \
                                          policy.false_(),              \
                                          ecxNotZero, len*8);                 \
                policy.writeGPR(x86_gpr_si,                             \
                                policy.add(policy.readGPR(x86_gpr_si),  \
                                           policy.ite(ecxNotZero,       \
                                                      policy.ite(policy.readFlag(x86_flag_df), \
                                                                 number(-(len)), \
                                                                 number(len)), \
                                                      number(0)))); \
                policy.writeGPR(x86_gpr_di,                             \
                                policy.add(policy.readGPR(x86_gpr_di),  \
                                           policy.ite(ecxNotZero,       \
                                                      policy.ite(policy.readFlag(x86_flag_df), \
                                                                 number(-(len)), \
                                                                 number(len)), \
                                                      number(0)))); \
                loopmacro;                                              \
            }
        case x86_repne_cmpsb: REP_CMPS(b, 1, ne, STRINGOP_LOOP_NE); break;
        case x86_repne_cmpsw: REP_CMPS(w, 2, ne, STRINGOP_LOOP_NE); break;
        case x86_repne_cmpsd: REP_CMPS(d, 4, ne, STRINGOP_LOOP_NE); break;
        case x86_repe_cmpsb:  REP_CMPS(b, 1, e,  STRINGOP_LOOP_E);  break;
        case x86_repe_cmpsw:  REP_CMPS(w, 2, e,  STRINGOP_LOOP_E);  break;
        case x86_repe_cmpsd:  REP_CMPS(d, 4, e,  STRINGOP_LOOP_E);  break;
#           undef REP_CMPS

#           define CMPS(suffix, len) {                                  \
                ROSE_ASSERT(operands.size() == 0);                      \
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32); \
                doAddOperation(STRINGOP_LOAD_SI(len, policy.true_()), \
                                          policy.invert(STRINGOP_LOAD_DI(len, policy.true_())), \
                                          true,                         \
                                          policy.false_(),              \
                                          policy.true_(), len*8);             \
                policy.writeGPR(x86_gpr_si,                             \
                                policy.add(policy.readGPR(x86_gpr_si),  \
                                           policy.ite(policy.readFlag(x86_flag_df), number(-(len)), number(len)))); \
                policy.writeGPR(x86_gpr_di,                             \
                                policy.add(policy.readGPR(x86_gpr_di),  \
                                           policy.ite(policy.readFlag(x86_flag_df), number(-(len)), number(len)))); \
            }
        case x86_cmpsb: CMPS(b, 1); break;
        case x86_cmpsw: CMPS(w, 2); break;
        case x86_cmpsd: CMPS(d, 4); break;
#           undef CMPS

#           define MOVS(suffix, len) {                                  \
                ROSE_ASSERT(operands.size() == 0);                      \
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32); \
                policy.writeMemory(x86_segreg_es,                       \
                                   policy.readGPR(x86_gpr_di),          \
                                   STRINGOP_LOAD_SI(len, policy.true_()), \
                                   policy.true_());                     \
                policy.writeGPR(x86_gpr_si,                             \
                                policy.add(policy.readGPR(x86_gpr_si),  \
                                           policy.ite(policy.readFlag(x86_flag_df), number(-(len)), number(len)))); \
                policy.writeGPR(x86_gpr_di,                             \
                                policy.add(policy.readGPR(x86_gpr_di),  \
                                           policy.ite(policy.readFlag(x86_flag_df), number(-(len)), number(len)))); \
            }
        case x86_movsb: MOVS(b, 1); break;
        case x86_movsw: MOVS(w, 2); break;
        case x86_movsd: MOVS(d, 4); break;
#           undef MOVS

#           define REP_MOVS(suffix, len) {                              \
                ROSE_ASSERT(operands.size() == 0);                      \
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32); \
                Word ecxNotZero = stringop_setup_loop();             \
                policy.writeMemory(x86_segreg_es, policy.readGPR(x86_gpr_di), STRINGOP_LOAD_SI(len, ecxNotZero), ecxNotZero); \
                policy.writeGPR(x86_gpr_si,                             \
                                policy.add(policy.readGPR(x86_gpr_si),  \
                                           policy.ite(ecxNotZero,       \
                                                      policy.ite(policy.readFlag(x86_flag_df), \
                                                                 number(-(len)), \
                                                                 number(len)), \
                                                      number(0)))); \
                policy.writeGPR(x86_gpr_di,                             \
                                policy.add(policy.readGPR(x86_gpr_di),  \
                                           policy.ite(ecxNotZero,       \
                                                      policy.ite(policy.readFlag(x86_flag_df), \
                                                                 number(-(len)), \
                                                                 number(len)), \
                                                      number(0)))); \
                policy.writeIP(policy.ite(ecxNotZero, /* If true, repeat this instruction, otherwise go to the next one */ \
                                          number((uint32_t)(insn->get_address())), \
                                          policy.readIP()));            \
            }
        case x86_rep_movsb: REP_MOVS(b, 1); break;
        case x86_rep_movsw: REP_MOVS(w, 2); break;
        case x86_rep_movsd: REP_MOVS(d, 4); break;
#           undef MOVS

        case x86_stosb: {
            stos_semantics(insn, 1);
            break;
        }

        case x86_rep_stosb: {
            rep_stos_semantics(insn, 1);
            break;
        }

        case x86_stosw: {
            stos_semantics(insn, 2);
            break;
        }

        case x86_rep_stosw: {
            rep_stos_semantics(insn, 2);
            break;
        }

        case x86_stosd: {
            stos_semantics(insn, 4);
            break;
        }

        case x86_rep_stosd: {
            rep_stos_semantics(insn, 4);
            break;
        }

#           define LODS(suffix, len, regupdate) {                       \
                ROSE_ASSERT(operands.size() == 0);                      \
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32); \
                regupdate(x86_gpr_ax, STRINGOP_LOAD_SI(len, policy.true_())); \
                policy.writeGPR(x86_gpr_si,                             \
                                policy.add(policy.readGPR(x86_gpr_si),  \
                                           policy.ite(policy.readFlag(x86_flag_df), number(-(len)), number(len)))); \
            }
        case x86_lodsb: LODS(b, 1, updateGPRLowByte); break;
        case x86_lodsw: LODS(w, 2, updateGPRLowWord); break;
        case x86_lodsd: LODS(d, 4, policy.writeGPR);  break;
#           undef LODS

#undef STRINGOP_LOAD_SI
#undef STRINGOP_LOAD_DI
#undef STRINGOP_UPDATE_CX
#undef STRINGOP_LOOP_E
#undef STRINGOP_LOOP_NE

        case x86_hlt: {
            ROSE_ASSERT(operands.size() == 0);
            policy.hlt();
            policy.writeIP(number((uint32_t)(insn->get_address())));
            break;
        }

        case x86_rdtsc: {
            ROSE_ASSERT(operands.size() == 0);
            Word tsc = policy.rdtsc();
            policy.writeGPR(x86_gpr_ax, extract(tsc, 0, 32));
            policy.writeGPR(x86_gpr_dx, extract(tsc, 32, 64));
            break;
        }

        case x86_int: {
            ROSE_ASSERT(operands.size() == 1);
            SgAsmByteValueExpression* bv = isSgAsmByteValueExpression(operands[0]);
            ROSE_ASSERT(bv);
            policy.interrupt(bv->get_value());
            break;
        }

            /* This is a dummy version that should be replaced later FIXME */
        case x86_fnstcw: {
            ROSE_ASSERT(operands.size() == 1);
            write16(operands[0], number(0x37f,16));
            break;
        }

        case x86_fldcw: {
            ROSE_ASSERT(operands.size() == 1);
            read16(operands[0]); /* To catch access control violations */
            break;
        }

        default: {
            std::cerr <<"Bad instruction [0x" <<std::hex <<insn->get_address() <<": " << insn->get_mnemonic() <<"]"
                      <<" (skipping semantic analysis)\n";
            break;
        }
        }
    }

    void processInstruction(SgAsmx86Instruction* insn) {
        ROSE_ASSERT(insn);
        policy.startInstruction(insn);
        translate(insn);
        policy.finishInstruction(insn);
    }

#if 0
    void processBlock(const SgAsmStatementPtrList& stmts, size_t begin, size_t end) {
        if (begin == end) return;
        policy.startBlock(stmts[begin]->get_address());
        for (size_t i = begin; i < end; ++i) {
            processInstruction(isSgAsmx86Instruction(stmts[i]));
        }
        policy.finishBlock(stmts[begin]->get_address());
    }
#endif

    static bool isRepeatedStringOp(SgAsmx86Instruction* insn) {
        if (!insn) return false;
        switch (insn->get_kind()) {
        case x86_repe_cmpsb: return true;
        case x86_repe_cmpsd: return true;
        case x86_repe_cmpsq: return true;
        case x86_repe_cmpsw: return true;
        case x86_repe_scasb: return true;
        case x86_repe_scasd: return true;
        case x86_repe_scasq: return true;
        case x86_repe_scasw: return true;
        case x86_rep_insb: return true;
        case x86_rep_insd: return true;
        case x86_rep_insw: return true;
        case x86_rep_lodsb: return true;
        case x86_rep_lodsd: return true;
        case x86_rep_lodsq: return true;
        case x86_rep_lodsw: return true;
        case x86_rep_movsb: return true;
        case x86_rep_movsd: return true;
        case x86_rep_movsq: return true;
        case x86_rep_movsw: return true;
        case x86_repne_cmpsb: return true;
        case x86_repne_cmpsd: return true;
        case x86_repne_cmpsq: return true;
        case x86_repne_cmpsw: return true;
        case x86_repne_scasb: return true;
        case x86_repne_scasd: return true;
        case x86_repne_scasq: return true;
        case x86_repne_scasw: return true;
        case x86_rep_outsb: return true;
        case x86_rep_outsd: return true;
        case x86_rep_outsw: return true;
        case x86_rep_stosb: return true;
        case x86_rep_stosd: return true;
        case x86_rep_stosq: return true;
        case x86_rep_stosw: return true;
        default: return false;
        }
    }

    static bool isHltOrInt(SgAsmx86Instruction* insn) {
        if (!insn) return false;
        switch (insn->get_kind()) {
        case x86_hlt: return true;
        case x86_int: return true;
        default: return false;
        }
    }

#if 0
    void processBlock(SgAsmBlock* b) {
        const SgAsmStatementPtrList& stmts = b->get_statementList();
        if (stmts.empty()) return;
        if (!isSgAsmInstruction(stmts[0])) return; /* A block containing functions or something */
        size_t i = 0;
        while (i < stmts.size()) {
            size_t oldI = i;
            /* Advance until either i points to a repeated string op or it is just after a hlt or int */
            while (i < stmts.size() && !isRepeatedStringOp(stmts[i]) && (i == oldI || !isHltOrInt(stmts[i - 1]))) ++i;
            processBlock(stmts, oldI, i);
            if (i >= stmts.size()) break;
            if (isRepeatedStringOp(stmts[i])) {
                processBlock(stmts, i, i + 1);
                ++i;
            }
            ROSE_ASSERT(i != oldI);
        }
    }
#endif


};

#endif // ROSE_X86INSTRUCTIONSEMANTICS_H
