#ifndef ROSE_X86_64INSTRUCTIONSEMANTICS_H
#define ROSE_X86_64INSTRUCTIONSEMANTICS_H

//#include "rose.h"
#include <stddef.h>
#include <stdint.h>
#include <string>
#include "semanticsModule.h"
#include <cassert>
#include <cstdio>
#include <iostream>
#include "rose.h"

#include "integerOps.h"

#include "semantics/BaseSemantics2.h"

#include "SgAsmx86Instruction.h"
#include "SgAsmExpression.h"
#include "conversions.h"

#undef ROSE_ASSERT
#define ROSE_ASSERT(x) \
    if (!(x)) {\
        throw rose::BinaryAnalysis::InstructionSemantics2::BaseSemantics::Exception("", NULL); \
    }\

#ifndef ROSE_X86INSTRUCTIONSEMANTICS_H

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
#endif

template <typename Policy, template <size_t> class WordType>
struct X86_64InstructionSemantics {
#   ifdef Word
#       error "Having a macro called \"Word\" conflicts with x86InstructionSemantics.h"
#   else
#       define Word(Len) WordType<(Len)>
#   endif

    Policy& policy;

    X86_64InstructionSemantics(Policy& policy_)
        : policy(policy_)
        {}
    virtual ~X86_64InstructionSemantics() {}

    /* If the counter (cx register) is non-zero then decrement it. The return value is a flag indicating whether cx was
     * originally non-zero. */
    Word(1) stringop_setup_loop() {
        Word(1) ecxNotZero = policy.invert(policy.equalToZero(policy.readGPR(x86_gpr_cx)));
        policy.writeGPR(x86_gpr_cx,
                        policy.add(policy.readGPR(x86_gpr_cx),
                                   policy.ite(ecxNotZero, number<64>(-1), number<64>(0))));
        return ecxNotZero;
    }

    /* Instruction semantics for rep_stosN where N is 1(b), 2(w), or 4(d).
     * This method handles semantics for one iteration of stosN.
     * See https://siyobik.info/index.php?module=x86&id=279 */
    template<size_t N>
    void rep_stos_semantics(SgAsmx86Instruction *insn) {
        Word(1) ecxNotZero = policy.invert(policy.equalToZero(policy.readGPR(x86_gpr_cx)));

        /* Fill memory pointed to by ES:[DI] with contents of AX register if counter was not zero. */
        policy.writeMemory(x86_segreg_es,
                           policy.readGPR(x86_gpr_di),
                           extract<0, 8*N>(policy.readGPR(x86_gpr_ax)),
                           ecxNotZero);

        /* Update DI by incrementing or decrementing by number of bytes copied (only if counter is nonzero) */
        policy.writeGPR(x86_gpr_di,
                        policy.add(policy.readGPR(x86_gpr_di),
                                   policy.ite(ecxNotZero,
                                              policy.ite(policy.readFlag(x86_flag_df), number<64>(-N), number<64>(N)),
                                              number<64>(0))));

        /* Decrement the counter if not already zero. */
        policy.writeGPR(x86_gpr_cx,
                        policy.add(policy.readGPR(x86_gpr_cx),
                                   policy.ite(ecxNotZero, number<64>(-1), number<64>(0))));

        /* If counter is now nonzero then repeat this instruction, otherwise go to the next one. */
        ecxNotZero = policy.invert(policy.equalToZero(policy.readGPR(x86_gpr_cx)));
        policy.writeIP(policy.ite(ecxNotZero, number<64>((uint32_t)(insn->get_address())), policy.readIP()));
    }

    /* Instruction semantics for stosN where N is 1(b), 2(w), or 4(d) */
    template<size_t N>
    void stos_semantics(SgAsmx86Instruction *insn) {
        const SgAsmExpressionPtrList& operands = insn->get_operandList()->get_operands();
        ROSE_ASSERT(operands.size() == 0);
        ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_64);

        /* Fill memory pointed to by ES:[DI] with contents of AX. */
        policy.writeMemory(x86_segreg_es,
                           policy.readGPR(x86_gpr_di),
                           extract<0, 8*N>(policy.readGPR(x86_gpr_ax)),
                           policy.true_());

        /* Update DI */ /*FIXME: Is this correct? */
        policy.writeGPR(x86_gpr_di,
                        policy.add(policy.readGPR(x86_gpr_di),
                                   policy.ite(policy.readFlag(x86_flag_df), number<64>(-N), number<64>(N))));
    }
    

    template <typename W>
    W invertMaybe(const W& w, bool inv) {
        if (inv) {
            return policy.invert(w);
        } else {
            return w;
        }
    }

    template <size_t Len>
    Word(Len) number(uintmax_t v) {
        return policy.template number<Len>(v);
    }

    template <size_t From, size_t To, typename W>
        Word(To - From) extract(W w) {
        return policy.template extract<From, To>(w);
    }

    template <size_t From, size_t To>
        Word(To) signExtend(Word(From) w) {
        return policy.template signExtend<From, To>(w);
    }

    template <size_t Len>
    Word(1) greaterOrEqualToTen(Word(Len) w) {
        Word(Len) carries = number<Len>(0);
        policy.addWithCarries(w, number<Len>(6), policy.false_(), carries);
        return extract<Len - 1, Len>(carries);
    }

    Word(32) readEflags() {
        return policy.concat(readFlags(), number<16>(0x0000));
    }

    Word(16) readFlags() {
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

    template <size_t Len/*bits*/>
    Word(Len) readMemory(X86SegmentRegister segreg, const Word(64)& addr, Word(1) cond) {
        return policy.template readMemory<Len>(segreg, addr, cond);
    }

    Word(64) readEffectiveAddress(SgAsmExpression* expr) {
      assert (isSgAsmMemoryReferenceExpression(expr));
      return read64(isSgAsmMemoryReferenceExpression(expr)->get_address());
    }

    /* Returns an eight-bit value desribed by an instruction operand. */
    Word(8) read8(SgAsmExpression* e) {
        switch (e->variantT()) {
            case V_SgAsmx86RegisterReferenceExpression: {
                SgAsmx86RegisterReferenceExpression* rre = isSgAsmx86RegisterReferenceExpression(e);
                switch (rre->get_register_class()) {
                    case x86_regclass_gpr: {
                        X86GeneralPurposeRegister reg = (X86GeneralPurposeRegister)(rre->get_register_number());
                        Word(64) rawValue = policy.readGPR(reg);
                        switch (rre->get_position_in_register()) {
                            case x86_regpos_low_byte: return extract<0, 8>(rawValue);
                            case x86_regpos_high_byte: return extract<8, 16>(rawValue);
                            default: ROSE_ASSERT(!"Bad position in register");
                        }
                    }
                    break;
                    default: {
                        fprintf(stderr, "Bad register class %s\n", regclassToString(rre->get_register_class()));
                        throw rose::BinaryAnalysis::InstructionSemantics2::BaseSemantics::Exception("", NULL);;
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
                return extract<0, 8>(policy.unsignedMultiply(read8(lhs), read8(rhs)));
            }
            case V_SgAsmMemoryReferenceExpression: {
                return readMemory<8>(getSegregFromMemoryReference(isSgAsmMemoryReferenceExpression(e)),
                                     readEffectiveAddress(e), policy.true_());
            }
            case V_SgAsmByteValueExpression:
            case V_SgAsmWordValueExpression:
            case V_SgAsmDoubleWordValueExpression:
            case V_SgAsmQuadWordValueExpression: {
                uint64_t val = SageInterface::getAsmSignedConstant(isSgAsmValueExpression(e));
                return number<8>(val & 0xFFU);
            }
            default: {
                fprintf(stderr, "Bad variant %s in read8\n", e->class_name().c_str());
                throw rose::BinaryAnalysis::InstructionSemantics2::BaseSemantics::Exception("", NULL);;
            }
        }
        return number<8>(0); // error case
    }

    /* Returns a 16-bit value described by an instruction operand. */
    Word(16) read16(SgAsmExpression* e) {
        switch (e->variantT()) {
            case V_SgAsmx86RegisterReferenceExpression: {
                SgAsmx86RegisterReferenceExpression* rre = isSgAsmx86RegisterReferenceExpression(e);
                // Xiaozhu: Not sure why we 16-bit is different from other lengths of read,
		// but for read8 or read32, there is no such assert.
		// ROSE_ASSERT(rre->get_position_in_register() == x86_regpos_word);
                switch (rre->get_register_class()) {
                    case x86_regclass_gpr: {
                        X86GeneralPurposeRegister reg = (X86GeneralPurposeRegister)(rre->get_register_number());
                        Word(64) rawValue = policy.readGPR(reg);
                        return extract<0, 16>(rawValue);
                    }
                    case x86_regclass_segment: {
                        X86SegmentRegister sr = (X86SegmentRegister)(rre->get_register_number());
                        Word(16) value = extract<0,16>(policy.readSegreg(sr));
                        return value;
                    }
                    default: {
                        fprintf(stderr, "Bad register class %s\n", regclassToString(rre->get_register_class()));
                        throw rose::BinaryAnalysis::InstructionSemantics2::BaseSemantics::Exception("", NULL);;
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
                return extract<0, 16>(policy.unsignedMultiply(read16(lhs), read8(rhs)));
            }
            case V_SgAsmMemoryReferenceExpression: {
                return readMemory<16>(getSegregFromMemoryReference(isSgAsmMemoryReferenceExpression(e)),
                                      readEffectiveAddress(e), policy.true_());
            }
            case V_SgAsmByteValueExpression:
            case V_SgAsmWordValueExpression:
            case V_SgAsmDoubleWordValueExpression:
            case V_SgAsmQuadWordValueExpression: {
                uint64_t val = SageInterface::getAsmSignedConstant(isSgAsmValueExpression(e));
                return number<16>(val & 0xFFFFU);
            }
            default: {
                fprintf(stderr, "Bad variant %s in read16\n", e->class_name().c_str());
                throw rose::BinaryAnalysis::InstructionSemantics2::BaseSemantics::Exception("", NULL); 
            }
        }
        return number<16>(0); //error case
    }

    /* Returns a 32-bit value described by an instruction operand. */
    Word(32) read32(SgAsmExpression* e) {
        switch (e->variantT()) {
            case V_SgAsmx86RegisterReferenceExpression: {
               SgAsmx86RegisterReferenceExpression* rre = isSgAsmx86RegisterReferenceExpression(e);
                switch (rre->get_register_class()) {
                    case x86_regclass_gpr: {
                        X86GeneralPurposeRegister reg = (X86GeneralPurposeRegister)(rre->get_register_number());
                        Word(64) rawValue = policy.readGPR(reg);
                        switch (rre->get_position_in_register()) {
                            case x86_regpos_dword:
                            case x86_regpos_all:
                                return extract<0,32>(rawValue);
                            case x86_regpos_word:
                                return policy.concat(extract<0, 16>(rawValue), number<16>(0));
                            default:
			      fprintf(stderr, "Error: position %d\n", rre->get_position_in_register());
			      ROSE_ASSERT(!"bad position in register");
                        }
                    }
                    break;
                    case x86_regclass_segment: {
                        ROSE_ASSERT(rre->get_position_in_register() == x86_regpos_dword ||
                                    rre->get_position_in_register() == x86_regpos_all);
                        X86SegmentRegister sr = (X86SegmentRegister)(rre->get_register_number());
                        Word(64) value = policy.readSegreg(sr);
                        return extract<0,32>(value);
                    }
                    default: {
                        std::string mesg = "Bad register class\n";
                        throw rose::BinaryAnalysis::InstructionSemantics2::BaseSemantics::Exception(mesg, NULL); 
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
                return extract<0, 32>(policy.unsignedMultiply(read32(lhs), read8(rhs)));
            }
            case V_SgAsmMemoryReferenceExpression: {
                return readMemory<32>(getSegregFromMemoryReference(isSgAsmMemoryReferenceExpression(e)),
                                      readEffectiveAddress(e), policy.true_());
            }
            case V_SgAsmByteValueExpression:
            case V_SgAsmWordValueExpression:
            case V_SgAsmDoubleWordValueExpression:
            case V_SgAsmQuadWordValueExpression: {
                uint64_t val = SageInterface::getAsmSignedConstant(isSgAsmValueExpression(e));
                return number<32>(val & 0xFFFFFFFFU);
            }
            default: {
                fprintf(stderr, "Bad variant %s in read32\n", e->class_name().c_str());
                throw rose::BinaryAnalysis::InstructionSemantics2::BaseSemantics::Exception("", NULL);;
            }
        }
        return number<32>(0); //error case
    }

    /* Returns a 64-bit value described by an instruction operand. */
    Word(64) read64(SgAsmExpression* e) {
        switch (e->variantT()) {
            case V_SgAsmx86RegisterReferenceExpression: {
                SgAsmx86RegisterReferenceExpression* rre = isSgAsmx86RegisterReferenceExpression(e);
                switch (rre->get_register_class()) {
                    case x86_regclass_gpr: {
                        X86GeneralPurposeRegister reg = (X86GeneralPurposeRegister)(rre->get_register_number());
                        Word(64) rawValue = policy.readGPR(reg);
                        switch (rre->get_position_in_register()) {
                            case x86_regpos_dword:
                            case x86_regpos_all:
                                return policy.concat(extract<0, 32>(rawValue), number<32>(0));
                            case x86_regpos_qword:
                                return rawValue;
                            case x86_regpos_word:
                                return policy.concat(extract<0, 16>(rawValue), number<48>(0));
                            default:
                                fprintf(stderr, "Error: position %d\n", rre->get_position_in_register());
                                        ROSE_ASSERT(!"bad position in register");
                        }
                    }
                    break;
                    case x86_regclass_segment: {
                                ROSE_ASSERT(rre->get_position_in_register() == x86_regpos_dword ||
                                            rre->get_position_in_register() == x86_regpos_all);
                        X86SegmentRegister sr = (X86SegmentRegister)(rre->get_register_number());
                        Word(64) value = policy.readSegreg(sr);
                        return value;
                    }
                    default: {
                        fprintf(stderr, "Bad register class %s\n", regclassToString(rre->get_register_class()));
                        throw rose::BinaryAnalysis::InstructionSemantics2::BaseSemantics::Exception("", NULL);;
                    }
                }
                break;
            }
            case V_SgAsmBinaryAdd: {
                return policy.add(read64(isSgAsmBinaryAdd(e)->get_lhs()), read64(isSgAsmBinaryAdd(e)->get_rhs()));
            }
            case V_SgAsmBinaryMultiply: {
                SgAsmByteValueExpression* rhs = isSgAsmByteValueExpression(isSgAsmBinaryMultiply(e)->get_rhs());
                        ROSE_ASSERT(rhs);
                SgAsmExpression* lhs = isSgAsmBinaryMultiply(e)->get_lhs();
                //TODO: Ask Bill
                return extract<0, 64>(policy.unsignedMultiply(read64(lhs), read8(rhs)));
            }
            case V_SgAsmMemoryReferenceExpression: {
                return readMemory<64>(getSegregFromMemoryReference(isSgAsmMemoryReferenceExpression(e)),
                                      readEffectiveAddress(e), policy.true_());
            }
            case V_SgAsmByteValueExpression:
            case V_SgAsmWordValueExpression:
            case V_SgAsmDoubleWordValueExpression:
            case V_SgAsmQuadWordValueExpression: {
                uint64_t val = SageInterface::getAsmSignedConstant(isSgAsmValueExpression(e));
                return number<64>(val & 0xFFFFFFFFFFFFFFFFLU);
            }
            default: {
                fprintf(stderr, "Bad variant %s in read32\n", e->class_name().c_str());
                throw rose::BinaryAnalysis::InstructionSemantics2::BaseSemantics::Exception("", NULL);;
            }
        }
        return number<64>(0); //error case
    }

    /* Replaces the least significant byte of a general purpose register with a new value. */
    void updateGPRLowByte(X86GeneralPurposeRegister reg, const Word(8)& value) {
        Word(64) oldValue = policy.readGPR(reg);
        policy.writeGPR(reg, policy.concat(value, extract<8, 64>(oldValue)));
    }

    /* Replaces bits 8 through 15 of a 32-bit register with the specified 8-bit value. */
    void updateGPRHighByte(X86GeneralPurposeRegister reg, const Word(8)& value) {
        Word(64) oldValue = policy.readGPR(reg);
        policy.writeGPR(reg, policy.concat(extract<0, 8>(oldValue),
                                           policy.concat(value, extract<16, 64>(oldValue))));
    }

    /* Replaces the least significant 16 bits of a general purpose register with a new value. */
    void updateGPRLowWord(X86GeneralPurposeRegister reg, const Word(16)& value) {
        Word(64) oldValue = policy.readGPR(reg);
        policy.writeGPR(reg, policy.concat(value, extract<16, 64>(oldValue)));
    }

    /* Replaces the least significant 32 bits of a general purpose register with a new value. */
    void updateGPRLowDWord(X86GeneralPurposeRegister reg, const Word(32)& value) {
        Word(64) oldValue = policy.readGPR(reg);
        policy.writeGPR(reg, policy.concat(value, extract<32, 64>(oldValue)));
    }

    /* Writes the specified eight-bit value to the location specified by an instruction operand. */
    void write8(SgAsmExpression* e, const Word(8)& value) {
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
                        throw rose::BinaryAnalysis::InstructionSemantics2::BaseSemantics::Exception("", NULL);;
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
                throw rose::BinaryAnalysis::InstructionSemantics2::BaseSemantics::Exception("", NULL);;
            }
        }
    }

    /* Writes the specified 16-bit value to the location specified by an instruction operand. */
    void write16(SgAsmExpression* e, const Word(16)& value) {
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
		    // TODO: Ask Bill, not sure how segment registers can be accessed
                    case x86_regclass_segment: {
                        X86SegmentRegister sr = (X86SegmentRegister)(rre->get_register_number());
                        policy.writeSegreg(sr, value);
                        break;
                    }
                    default: {
                        fprintf(stderr, "Bad register class %s\n", regclassToString(rre->get_register_class()));
                        throw rose::BinaryAnalysis::InstructionSemantics2::BaseSemantics::Exception("", NULL);;
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
                throw rose::BinaryAnalysis::InstructionSemantics2::BaseSemantics::Exception("", NULL);;
            }
        }
    }

    /* Writes the specified 32-bit value to the location specified by an instruction operand. */
    void write32(SgAsmExpression* e, const Word(32)& value) {
        switch (e->variantT()) {
            case V_SgAsmx86RegisterReferenceExpression: {
                SgAsmx86RegisterReferenceExpression* rre = isSgAsmx86RegisterReferenceExpression(e);
                switch (rre->get_register_class()) {
                    case x86_regclass_gpr: {
                        X86GeneralPurposeRegister reg = (X86GeneralPurposeRegister)(rre->get_register_number());
                        switch (rre->get_position_in_register()) {
                            case x86_regpos_dword:
			    case x86_regpos_all: updateGPRLowDWord(reg, value); break;
                            default: ROSE_ASSERT(!"Bad position in register");
                        }
                        break;
                    }
                    case x86_regclass_segment: { // Used for pop of segment registers
                        X86SegmentRegister sr = (X86SegmentRegister)(rre->get_register_number());
                        policy.writeSegreg(sr, extract<0, 16>(value));
                        break;
                    }
                    default: {
                        fprintf(stderr, "Bad register class %s\n", regclassToString(rre->get_register_class()));
                        throw rose::BinaryAnalysis::InstructionSemantics2::BaseSemantics::Exception("", NULL);;
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
                throw rose::BinaryAnalysis::InstructionSemantics2::BaseSemantics::Exception("", NULL);;
            }
        }
    }

    /* Writes the specified 64-bit value to the location specified by an instruction operand. */
    void write64(SgAsmExpression* e, const Word(64)& value) {
        switch (e->variantT()) {
            case V_SgAsmx86RegisterReferenceExpression: {
                SgAsmx86RegisterReferenceExpression* rre = isSgAsmx86RegisterReferenceExpression(e);
                switch (rre->get_register_class()) {
                    case x86_regclass_gpr: {
                        X86GeneralPurposeRegister reg = (X86GeneralPurposeRegister)(rre->get_register_number());
                        switch (rre->get_position_in_register()) {
                            case x86_regpos_qword: {
                                break;
                            }
                            default: ROSE_ASSERT(!"Bad position in register");
                        }
                        policy.writeGPR(reg, value);
                        break;
                    }
                    case x86_regclass_segment: { // Used for pop of segment registers
                        X86SegmentRegister sr = (X86SegmentRegister)(rre->get_register_number());
                        policy.writeSegreg(sr, extract<0, 16>(value));
                        break;
                    }
                    default: {
                        fprintf(stderr, "Bad register class %s\n", regclassToString(rre->get_register_class()));
                        throw rose::BinaryAnalysis::InstructionSemantics2::BaseSemantics::Exception("", NULL);;
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
                throw rose::BinaryAnalysis::InstructionSemantics2::BaseSemantics::Exception("", NULL);;
            }
        }
    }

    /* Returns true if W has an even number of bits set; false for an odd number */
    Word(1) parity(Word(8) w) {
        Word(1) p01 = policy.xor_(extract<0, 1>(w), extract<1, 2>(w));
        Word(1) p23 = policy.xor_(extract<2, 3>(w), extract<3, 4>(w));
        Word(1) p45 = policy.xor_(extract<4, 5>(w), extract<5, 6>(w));
        Word(1) p67 = policy.xor_(extract<6, 7>(w), extract<7, 8>(w));
        Word(1) p0123 = policy.xor_(p01, p23);
        Word(1) p4567 = policy.xor_(p45, p67);
        return policy.invert(policy.xor_(p0123, p4567));
    }

    /* Sets flags: parity, sign, and zero */
    template <size_t Len>
    void setFlagsForResult(const Word(Len)& result) {
        policy.writeFlag(x86_flag_pf, parity(extract<0, 8>(result)));
        policy.writeFlag(x86_flag_sf, extract<Len - 1, Len>(result));
        policy.writeFlag(x86_flag_zf, policy.equalToZero(result));
    }

    /* Sets flags conditionally. Sets parity, sign, and zero flags if COND is true. */
    template <size_t Len>
    void setFlagsForResult(const Word(Len)& result, Word(1) cond) {
        policy.writeFlag(x86_flag_pf, policy.ite(cond, parity(extract<0, 8>(result)), policy.readFlag(x86_flag_pf)));
        policy.writeFlag(x86_flag_sf, policy.ite(cond, extract<Len - 1, Len>(result), policy.readFlag(x86_flag_sf)));
        policy.writeFlag(x86_flag_zf, policy.ite(cond, policy.equalToZero(result), policy.readFlag(x86_flag_zf)));
    }

    /* Adds A and B and adjusts condition flags. Can be used for subtraction if B is two's complement and invertCarries is set. */
    template <size_t Len>
    Word(Len) doAddOperation(const Word(Len)& a, const Word(Len)& b, bool invertCarries, Word(1) carryIn) {
        Word(Len) carries = number<Len>(0);
        Word(Len) result = policy.addWithCarries(a, b, invertMaybe(carryIn, invertCarries), carries/*out*/);
        setFlagsForResult<Len>(result);
        policy.writeFlag(x86_flag_af, invertMaybe(extract<3, 4>(carries), invertCarries));
        policy.writeFlag(x86_flag_cf, invertMaybe(extract<Len - 1, Len>(carries), invertCarries));
        policy.writeFlag(x86_flag_of, policy.xor_(extract<Len - 1, Len>(carries), extract<Len - 2, Len - 1>(carries)));
        return result;
    }

    /* Conditionally adds A and B and adjusts condition flags. Can be used for subtraction if B is two's complement and
     * invertCarries is set. Does nothing if COND is false. */ 
    template <size_t Len>
    Word(Len) doAddOperation(const Word(Len)& a, const Word(Len)& b, bool invertCarries, Word(1) carryIn, Word(1) cond) {
        Word(Len) carries = number<Len>(0);
        Word(Len) result = policy.addWithCarries(a, b, invertMaybe(carryIn, invertCarries), carries/*out*/);
        setFlagsForResult<Len>(result, cond);
        policy.writeFlag(x86_flag_af,
                         policy.ite(cond,
                                    invertMaybe(extract<3, 4>(carries), invertCarries),
                                    policy.readFlag(x86_flag_af)));
        policy.writeFlag(x86_flag_cf,
                         policy.ite(cond,
                                    invertMaybe(extract<Len - 1, Len>(carries), invertCarries),
                                    policy.readFlag(x86_flag_cf)));
        policy.writeFlag(x86_flag_of,
                         policy.ite(cond,
                                    policy.xor_(extract<Len - 1, Len>(carries), extract<Len - 2, Len - 1>(carries)),
                                    policy.readFlag(x86_flag_of)));
        return result;
    }

    /* Does increment (decrement with DEC set), and adjusts condition flags. */
    template <size_t Len>
    Word(Len) doIncOperation(const Word(Len)& a, bool dec, bool setCarry) {
        Word(Len) carries = number<Len>(0);
        Word(Len) result = policy.addWithCarries(a, number<Len>(dec ? -1 : 1), policy.false_(), carries/*out*/);
        setFlagsForResult<Len>(result);
        policy.writeFlag(x86_flag_af, invertMaybe(extract<3, 4>(carries), dec));
        policy.writeFlag(x86_flag_of, policy.xor_(extract<Len - 1, Len>(carries), extract<Len - 2, Len - 1>(carries)));
        if (setCarry)
            policy.writeFlag(x86_flag_cf, invertMaybe(extract<Len - 1, Len>(carries), dec));
        return result;
    }

    /* Virtual so that we can subclass X86InstructionSemantics and have an opportunity to override the translation of any
     * instruction. */
    virtual void translate(SgAsmx86Instruction* insn) {
      // BERNAT, 16MAR10 - keep the IP symbolic.
      // PATCH
      policy.writeIP(policy.add(policy.readIP(), number<64>(insn->get_raw_bytes().size())));
      //policy.writeIP(number<32>((unsigned int)(insn->get_address() + insn->get_raw_bytes().size())));
      // END PATCH
        X86InstructionKind kind = insn->get_kind();
        const SgAsmExpressionPtrList& operands = insn->get_operandList()->get_operands();
        switch (kind) {

            case x86_mov: {
                ROSE_ASSERT(operands.size() == 2);
                switch (numBytesInAsmType(operands[0]->get_type())) {
                    case 1: write8(operands[0], read8(operands[1])); break;
                    case 2: write16(operands[0], read16(operands[1])); break;
                    case 4: write32(operands[0], read32(operands[1])); break;
                    case 8: write64(operands[0], read64(operands[1])); break;
                    default: ROSE_ASSERT("Bad size"); break;
                }
                break;
            }

            case x86_xchg: {
                ROSE_ASSERT(operands.size() == 2);
                switch (numBytesInAsmType(operands[0]->get_type())) {
                    case 1: {
                        Word(8) temp = read8(operands[1]);
                        write8(operands[1], read8(operands[0]));
                        write8(operands[0], temp);
                        break;
                    }
                    case 2: {
                        Word(16) temp = read16(operands[1]);
                        write16(operands[1], read16(operands[0]));
                        write16(operands[0], temp);
                        break;
                    }
                    case 4: {
                        Word(32) temp = read32(operands[1]);
                        write32(operands[1], read32(operands[0]));
                        write32(operands[0], temp);
                        break;
                    }
                    case 8: {
                        Word(64) temp = read64(operands[1]);
                        write64(operands[1], read64(operands[0]));
                        write64(operands[0], temp);
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
                        write16(operands[0], policy.concat(read8(operands[1]), number<8>(0)));
                        break;
                    }
                    case 4: {
                        switch (numBytesInAsmType(operands[1]->get_type())) {
                            case 1: write32(operands[0], policy.concat(read8(operands[1]), number<24>(0))); break;
                            case 2: write32(operands[0], policy.concat(read16(operands[1]), number<16>(0))); break;
                            default: ROSE_ASSERT("Bad size");
                        }
                        break;
                    }
		    case 8: {
                        switch (numBytesInAsmType(operands[1]->get_type())) {
                            case 1: write64(operands[0], policy.concat(read8(operands[1]), number<56>(0))); break;
                            case 2: write64(operands[0], policy.concat(read16(operands[1]), number<48>(0))); break;
			    case 4: write64(operands[0], policy.concat(read32(operands[1]), number<32>(0))); break;

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

            case x86_movsx:
	    case x86_movsxd: {
                ROSE_ASSERT(operands.size() == 2);
                switch (numBytesInAsmType(operands[0]->get_type())) {
                    case 2: {
                        Word(8) op1 = read8(operands[1]);
                        Word(16) result = signExtend<8, 16>(op1);
                        write16(operands[0], result);
                        break;
                    }
                    case 4: {
                        switch (numBytesInAsmType(operands[1]->get_type())) {
                            case 1: {
                                Word(8) op1 = read8(operands[1]);
                                Word(32) result = signExtend<8, 32>(op1);
                                write32(operands[0], result);
                                break;
                            }
                            case 2: {
                                Word(16) op1 = read16(operands[1]);
                                Word(32) result = signExtend<16, 32>(op1);
                                write32(operands[0], result);
                                break;
                            }
                            default:
			        break;
//                                ROSE_ASSERT(!"Bad size");
                        }
                        break;
                    }
                    case 8: {
                        switch (numBytesInAsmType(operands[1]->get_type())) {
                            case 1: {
                                Word(8) op1 = read8(operands[1]);
                                Word(64) result = signExtend<8, 64>(op1);
                                write64(operands[0], result);
                                break;
                            }
                            case 2: {
                                Word(16) op1 = read16(operands[1]);
                                Word(64) result = signExtend<16, 64>(op1);
                                write64(operands[0], result);
                                break;
                            }
			    case 4: {
                                Word(32) op1 = read32(operands[1]);
                                Word(64) result = signExtend<32, 64>(op1);
                                write64(operands[0], result);
                                break;
			    }
                            default:
                                ROSE_ASSERT(!"Bad size");
                        }
                        break;
                    }

                    default:
                        ROSE_ASSERT(!"Bad size");
                        break;
                }
                break;
            }
/*
            case x86_cbw: {
                ROSE_ASSERT(operands.size() == 0);
                updateGPRLowWord(x86_gpr_ax, signExtend<8, 16>(extract<0, 8>(policy.readGPR(x86_gpr_ax))));
                break;
            }
*/
            case x86_cwde: {
                ROSE_ASSERT(operands.size() == 0);
                policy.writeGPR(x86_gpr_ax, signExtend<16, 64>(extract<0, 16>(policy.readGPR(x86_gpr_ax))));
                break;
            }
/*
            case x86_cwd: {
                ROSE_ASSERT(operands.size() == 0);
                updateGPRLowWord(x86_gpr_dx, extract<16, 32>(signExtend<16, 32>(extract<0, 16>(policy.readGPR(x86_gpr_ax)))));
                break;
            }

            case x86_cdq: {
                ROSE_ASSERT(operands.size() == 0);
                policy.writeGPR(x86_gpr_dx, extract<32, 64>(signExtend<32, 64>(policy.readGPR(x86_gpr_ax))));
                break;
            }
*/
            case x86_lea: {
                ROSE_ASSERT(operands.size() == 2);
                switch (numBytesInAsmType(operands[0]->get_type())) {
		    case 4: {
		        write32(operands[0], extract<0,32>(readEffectiveAddress(operands[1])));
			break;
		    }
		    case 8: {
		        write64(operands[0], readEffectiveAddress(operands[1]));
			break;
		    }
		    default:
		        ROSE_ASSERT(!"Bad size!");
		}
                break;
            }

            case x86_and: {
                ROSE_ASSERT(operands.size() == 2);
                switch (numBytesInAsmType(operands[0]->get_type())) {
                    case 1: {
                        Word(8) result = policy.and_(read8(operands[0]), read8(operands[1]));
                        setFlagsForResult<8>(result);
                        write8(operands[0], result);
                        break;
                    }
                    case 2: {
                        Word(16) result = policy.and_(read16(operands[0]), read16(operands[1]));
                        setFlagsForResult<16>(result);
                        write16(operands[0], result);
                        break;
                    }
                    case 4: {
                        Word(32) result = policy.and_(read32(operands[0]), read32(operands[1]));
                        setFlagsForResult<32>(result);
                        write32(operands[0], result);
                        break;
                    }
                    case 8: {
                        Word(64) result = policy.and_(read64(operands[0]), read64(operands[1]));
                        setFlagsForResult<64>(result);
                        write64(operands[0], result);
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
                        Word(8) result = policy.or_(read8(operands[0]), read8(operands[1]));
                        setFlagsForResult<8>(result);
                        write8(operands[0], result);
                        break;
                    }
                    case 2: {
                        Word(16) result = policy.or_(read16(operands[0]), read16(operands[1]));
                        setFlagsForResult<16>(result);
                        write16(operands[0], result);
                        break;
                    }
                    case 4: {
                        Word(32) result = policy.or_(read32(operands[0]), read32(operands[1]));
                        setFlagsForResult<32>(result);
                        write32(operands[0], result);
                        break;
                    }
                    case 8: {
                        Word(64) result = policy.or_(read64(operands[0]), read64(operands[1]));
                        setFlagsForResult<64>(result);
                        write64(operands[0], result);
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
                        Word(8) result = policy.and_(read8(operands[0]), read8(operands[1]));
                        setFlagsForResult<8>(result);
                        break;
                    }
                    case 2: {
                        Word(16) result = policy.and_(read16(operands[0]), read16(operands[1]));
                        setFlagsForResult<16>(result);
                        break;
                    }
                    case 4: {
                        Word(32) result = policy.and_(read32(operands[0]), read32(operands[1]));
                        setFlagsForResult<32>(result);
                        break;
                    }
                    case 8: {
                        Word(64) result = policy.and_(read64(operands[0]), read64(operands[1]));
                        setFlagsForResult<64>(result);
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
                        Word(8) result = policy.xor_(read8(operands[0]), read8(operands[1]));
                        setFlagsForResult<8>(result);
                        write8(operands[0], result);
                        break;
                    }
                    case 2: {
                        Word(16) result = policy.xor_(read16(operands[0]), read16(operands[1]));
                        setFlagsForResult<16>(result);
                        write16(operands[0], result);
                        break;
                    }
                    case 4: {
                        Word(32) result = policy.xor_(read32(operands[0]), read32(operands[1]));
                        setFlagsForResult<32>(result);
                        write32(operands[0], result);
                        break;
                    }
                    case 8: {
                        Word(64) result = policy.xor_(read64(operands[0]), read64(operands[1]));
                        setFlagsForResult<64>(result);
                        write64(operands[0], result);
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
/*
            case x86_not: {
                ROSE_ASSERT(operands.size() == 1);
                switch (numBytesInAsmType(operands[0]->get_type())) {
                    case 1: {
                        Word(8) result = policy.invert(read8(operands[0]));
                        write8(operands[0], result);
                        break;
                    }
                    case 2: {
                        Word(16) result = policy.invert(read16(operands[0]));
                        write16(operands[0], result);
                        break;
                    }
                    case 4: {
                        Word(32) result = policy.invert(read32(operands[0]));
                        write32(operands[0], result);
                        break;
                    }
                    default:
                        ROSE_ASSERT(!"Bad size");
                        break;
                }
                break;
            }
*/
            case x86_add: {
                ROSE_ASSERT(operands.size() == 2);
                switch (numBytesInAsmType(operands[0]->get_type())) {
                    case 1: {
                        Word(8) result = doAddOperation<8>(read8(operands[0]), read8(operands[1]), false, policy.false_());
                        write8(operands[0], result);
                        break;
                    }
                    case 2: {
                        Word(16) result = doAddOperation<16>(read16(operands[0]), read16(operands[1]), false, policy.false_());
                        write16(operands[0], result);
                        break;
                    }
                    case 4: {
                        Word(32) result = doAddOperation<32>(read32(operands[0]), read32(operands[1]), false, policy.false_());
                        write32(operands[0], result);
                        break;
                    }
                    case 8: {
                        Word(64) result = doAddOperation<64>(read64(operands[0]), read64(operands[1]), false, policy.false_());
                        write64(operands[0], result);
                        break;
                    }
 
                    default:
                        ROSE_ASSERT(!"Bad size");
                        break;
                }
                break;
            }
/*
            case x86_adc: {
                ROSE_ASSERT(operands.size() == 2);
                switch (numBytesInAsmType(operands[0]->get_type())) {
                    case 1: {
                        Word(8) result = doAddOperation<8>(read8(operands[0]), read8(operands[1]), false,
                                                           policy.readFlag(x86_flag_cf));
                        write8(operands[0], result);
                        break;
                    }
                    case 2: {
                        Word(16) result = doAddOperation<16>(read16(operands[0]), read16(operands[1]), false,
                                                             policy.readFlag(x86_flag_cf));
                        write16(operands[0], result);
                        break;
                    }
                    case 4: {
                        Word(32) result = doAddOperation<32>(read32(operands[0]), read32(operands[1]), false,
                                                             policy.readFlag(x86_flag_cf));
                        write32(operands[0], result);
                        break;
                    }
                    default:
                        ROSE_ASSERT(!"Bad size");
                        break;
                }
                break;
            }
*/
            case x86_sub: {
                ROSE_ASSERT(operands.size() == 2);
                switch (numBytesInAsmType(operands[0]->get_type())) {
                    case 1: {
                        Word(8) result = doAddOperation<8>(read8(operands[0]), policy.invert(read8(operands[1])), true,
                                                           policy.false_());
                        write8(operands[0], result);
                        break;
                    }
                    case 2: {
                        Word(16) result = doAddOperation<16>(read16(operands[0]), policy.invert(read16(operands[1])), true,
                                                             policy.false_());
                        write16(operands[0], result);
                        break;
                    }
                    case 4: {
                        Word(32) result = doAddOperation<32>(read32(operands[0]), policy.invert(read32(operands[1])), true,
                                                             policy.false_());
                        write32(operands[0], result);
                        break;
                    }
                    case 8: {
                        Word(64) result = doAddOperation<64>(read64(operands[0]), policy.invert(read64(operands[1])), true,
                                                             policy.false_());
                        write64(operands[0], result);
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
                        Word(8) result = doAddOperation<8>(read8(operands[0]), policy.invert(read8(operands[1])), true,
                                                           policy.readFlag(x86_flag_cf));
                        write8(operands[0], result);
                        break;
                    }
                    case 2: {
                        Word(16) result = doAddOperation<16>(read16(operands[0]), policy.invert(read16(operands[1])), true,
                                                             policy.readFlag(x86_flag_cf));
                        write16(operands[0], result);
                        break;
                    }
                    case 4: {
                        Word(32) result = doAddOperation<32>(read32(operands[0]), policy.invert(read32(operands[1])), true,
                                                             policy.readFlag(x86_flag_cf));
                        write32(operands[0], result);
                        break;
                    }
                    case 8: {
                        Word(64) result = doAddOperation<64>(read64(operands[0]), policy.invert(read64(operands[1])), true,
                                                             policy.readFlag(x86_flag_cf));
                        write64(operands[0], result);
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
                        doAddOperation<8>(read8(operands[0]), policy.invert(read8(operands[1])), true, policy.false_());
                        break;
                    }
                    case 2: {
                        doAddOperation<16>(read16(operands[0]), policy.invert(read16(operands[1])), true, policy.false_());
                        break;
                    }
                    case 4: {
                        doAddOperation<32>(read32(operands[0]), policy.invert(read32(operands[1])), true, policy.false_());
                        break;
                    }
                    case 8: {
                        doAddOperation<64>(read64(operands[0]), policy.invert(read64(operands[1])), true, policy.false_());
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
                        Word(8) result = doAddOperation<8>(number<8>(0), policy.invert(read8(operands[0])), true,
                                                           policy.false_());
                        write8(operands[0], result);
                        break;
                    }
                    case 2: {
                        Word(16) result = doAddOperation<16>(number<16>(0), policy.invert(read16(operands[0])), true,
                                                             policy.false_());
                        write16(operands[0], result);
                        break;
                    }
                    case 4: {
                        Word(32) result = doAddOperation<32>(number<32>(0), policy.invert(read32(operands[0])), true,
                                                             policy.false_());
                        write32(operands[0], result);
                        break;
                    }
                    case 8: {
                        Word(64) result = doAddOperation<64>(number<64>(0), policy.invert(read64(operands[0])), true,
                                                             policy.false_());
                        write64(operands[0], result);
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
                        Word(8) result = doIncOperation<8>(read8(operands[0]), false, false);
                        write8(operands[0], result);
                        break;
                    }
                    case 2: {
                        Word(16) result = doIncOperation<16>(read16(operands[0]), false, false);
                        write16(operands[0], result);
                        break;
                    }
                    case 4: {
                        Word(32) result = doIncOperation<32>(read32(operands[0]), false, false);
                        write32(operands[0], result);
                        break;
                    }
                    case 8: {
                        Word(64) result = doIncOperation<64>(read64(operands[0]), false, false);
                        write64(operands[0], result);
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
                        Word(8) result = doIncOperation<8>(read8(operands[0]), true, false);
                        write8(operands[0], result);
                        break;
                    }
                    case 2: {
                        Word(16) result = doIncOperation<16>(read16(operands[0]), true, false);
                        write16(operands[0], result);
                        break;
                    }
                    case 4: {
                        Word(32) result = doIncOperation<32>(read32(operands[0]), true, false);
                        write32(operands[0], result);
                        break;
                    }
                    case 8: {
                        Word(64) result = doIncOperation<64>(read64(operands[0]), true, false);
                        write64(operands[0], result);
                        break;
                    }

                    default:
                        ROSE_ASSERT(!"Bad size");
                        break;
                }
                break;
            }
/*
            case x86_cmpxchg: {
                ROSE_ASSERT(operands.size() == 2);
                switch (numBytesInAsmType(operands[0]->get_type())) {
                    case 1: {
                        Word(8) op0 = read8(operands[0]);
                        Word(8) oldAx = extract<0, 8>(policy.readGPR(x86_gpr_ax));
                        doAddOperation<8>(oldAx, policy.invert(op0), true, policy.false_());
                        write8(operands[0], policy.ite(policy.readFlag(x86_flag_zf), read8(operands[1]), op0));
                        updateGPRLowByte(x86_gpr_ax, policy.ite(policy.readFlag(x86_flag_zf), oldAx, op0));
                        break;
                    }
                    case 2: {
                        Word(16) op0 = read16(operands[0]);
                        Word(16) oldAx = extract<0, 16>(policy.readGPR(x86_gpr_ax));
                        doAddOperation<16>(oldAx, policy.invert(op0), true, policy.false_());
                        write16(operands[0], policy.ite(policy.readFlag(x86_flag_zf), read16(operands[1]), op0));
                        updateGPRLowWord(x86_gpr_ax, policy.ite(policy.readFlag(x86_flag_zf), oldAx, op0));
                        break;
                    }
                    case 4: {
                        Word(32) op0 = read32(operands[0]);
                        Word(32) oldAx = policy.readGPR(x86_gpr_ax);
                        doAddOperation<32>(oldAx, policy.invert(op0), true, policy.false_());
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
*/
            case x86_shl: {
	        //TODO : ask bill, not sure about the shiftCount bound
                Word(5) shiftCount = extract<0, 5>(read8(operands[1]));
                Word(1) shiftCountZero = policy.equalToZero(shiftCount);
                policy.writeFlag(x86_flag_af, policy.ite(shiftCountZero, policy.readFlag(x86_flag_af), policy.undefined_()));
                switch (numBytesInAsmType(operands[0]->get_type())) {
                    case 1: {
                        Word(8) op = read8(operands[0]);
                        Word(8) output = policy.shiftLeft(op, shiftCount);
                        Word(1) newCf = policy.ite(shiftCountZero,
                                                   policy.readFlag(x86_flag_cf),
                                                   extract<7, 8>(policy.shiftLeft(op, policy.add(shiftCount, number<5>(7)))));
                        policy.writeFlag(x86_flag_cf, newCf);
                        policy.writeFlag(x86_flag_of, policy.ite(shiftCountZero,
                                                                 policy.readFlag(x86_flag_of),
                                                                 policy.xor_(extract<7, 8>(output), newCf)));
                        write8(operands[0], output);
                        setFlagsForResult<8>(output, policy.invert(shiftCountZero));
                        break;
                    }
                    case 2: {
                        Word(16) op = read16(operands[0]);
                        Word(16) output = policy.shiftLeft(op, shiftCount);
                        Word(1) newCf = policy.ite(shiftCountZero,
                                                   policy.readFlag(x86_flag_cf),
                                                   extract<15, 16>(policy.shiftLeft(op, policy.add(shiftCount, number<5>(15)))));
                        policy.writeFlag(x86_flag_cf, newCf);
                        policy.writeFlag(x86_flag_of, policy.ite(shiftCountZero,
                                                                 policy.readFlag(x86_flag_of),
                                                                 policy.xor_(extract<15, 16>(output), newCf)));
                        write16(operands[0], output);
                        setFlagsForResult<16>(output, policy.invert(shiftCountZero));
                        break;
                    }
                    case 4: {
                        Word(32) op = read32(operands[0]);
                        Word(32) output = policy.shiftLeft(op, shiftCount);
                        Word(1) newCf = policy.ite(shiftCountZero,
                                                   policy.readFlag(x86_flag_cf),
                                                   extract<31, 32>(policy.shiftLeft(op, policy.add(shiftCount, number<5>(31)))));
                        policy.writeFlag(x86_flag_cf, newCf);
                        policy.writeFlag(x86_flag_of, policy.ite(shiftCountZero,
                                                                 policy.readFlag(x86_flag_of),
                                                                 policy.xor_(extract<31, 32>(output), newCf)));
                        write32(operands[0], output);
                        setFlagsForResult<32>(output, policy.invert(shiftCountZero));
                        break;
                    }
                    case 8: {
                        Word(64) op = read64(operands[0]);
                        Word(64) output = policy.shiftLeft(op, shiftCount);
                        Word(1) newCf = policy.ite(shiftCountZero,
                                                   policy.readFlag(x86_flag_cf),
                                                   extract<63, 64>(policy.shiftLeft(op, policy.add(shiftCount, number<5>(63)))));
                        policy.writeFlag(x86_flag_cf, newCf);
                        policy.writeFlag(x86_flag_of, policy.ite(shiftCountZero,
                                                                 policy.readFlag(x86_flag_of),
                                                                 policy.xor_(extract<63, 64>(output), newCf)));
                        write64(operands[0], output);
                        setFlagsForResult<64>(output, policy.invert(shiftCountZero));
                        break;
                    }

                    default:
                        ROSE_ASSERT(!"Bad size");
                }
                break;
            }

            case x86_shr: {
                Word(5) shiftCount = extract<0, 5>(read8(operands[1]));
                Word(1) shiftCountZero = policy.equalToZero(shiftCount);
                policy.writeFlag(x86_flag_af, policy.ite(shiftCountZero, policy.readFlag(x86_flag_af), policy.undefined_()));
                switch (numBytesInAsmType(operands[0]->get_type())) {
                    case 1: {
                        Word(8) op = read8(operands[0]);
                        Word(8) output = policy.shiftRight(op, shiftCount);
                        Word(1) newCf = policy.ite(shiftCountZero,
                                                   policy.readFlag(x86_flag_cf),
                                                   extract<0, 1>(policy.shiftRight(op, policy.add(shiftCount, number<5>(7)))));
                        policy.writeFlag(x86_flag_cf, newCf);
                        policy.writeFlag(x86_flag_of, policy.ite(shiftCountZero, policy.readFlag(x86_flag_of), extract<7, 8>(op)));
                        write8(operands[0], output);
                        setFlagsForResult<8>(output, policy.invert(shiftCountZero));
                        break;
                    }
                    case 2: {
                        Word(16) op = read16(operands[0]);
                        Word(16) output = policy.shiftRight(op, shiftCount);
                        Word(1) newCf = policy.ite(shiftCountZero,
                                                   policy.readFlag(x86_flag_cf),
                                                   extract<0, 1>(policy.shiftRight(op, policy.add(shiftCount, number<5>(15)))));
                        policy.writeFlag(x86_flag_cf, newCf);
                        policy.writeFlag(x86_flag_of,
                                         policy.ite(shiftCountZero, policy.readFlag(x86_flag_of), extract<15, 16>(op)));
                        write16(operands[0], output);
                        setFlagsForResult<16>(output, policy.invert(shiftCountZero));
                        break;
                    }
                    case 4: {
                        Word(32) op = read32(operands[0]);
                        Word(32) output = policy.shiftRight(op, shiftCount);
                        Word(1) newCf = policy.ite(shiftCountZero,
                                                   policy.readFlag(x86_flag_cf),
                                                   extract<0, 1>(policy.shiftRight(op, policy.add(shiftCount, number<5>(31)))));
                        policy.writeFlag(x86_flag_cf, newCf);
                        policy.writeFlag(x86_flag_of, policy.ite(shiftCountZero,
                                                                 policy.readFlag(x86_flag_of),
                                                                 extract<31, 32>(op)));
                        write32(operands[0], output);
                        setFlagsForResult<32>(output, policy.invert(shiftCountZero));
                        break;
                    }
                    case 8: {
                        Word(64) op = read64(operands[0]);
                        Word(64) output = policy.shiftRight(op, shiftCount);
                        Word(1) newCf = policy.ite(shiftCountZero,
                                                   policy.readFlag(x86_flag_cf),
                                                   extract<0, 1>(policy.shiftRight(op, policy.add(shiftCount, number<5>(63)))));
                        policy.writeFlag(x86_flag_cf, newCf);
                        policy.writeFlag(x86_flag_of, policy.ite(shiftCountZero,
                                                                 policy.readFlag(x86_flag_of),
                                                                 extract<63, 64>(op)));
                        write64(operands[0], output);
                        setFlagsForResult<64>(output, policy.invert(shiftCountZero));
                        break;
                    }

                    default:
                        ROSE_ASSERT(!"Bad size");
                }
                break;
            }

            case x86_sar: {
                Word(5) shiftCount = extract<0, 5>(read8(operands[1]));
                Word(1) shiftCountZero = policy.equalToZero(shiftCount);
                Word(1) shiftCountNotZero = policy.invert(shiftCountZero);
                policy.writeFlag(x86_flag_af, policy.ite(shiftCountZero, policy.readFlag(x86_flag_af), policy.undefined_()));
                switch (numBytesInAsmType(operands[0]->get_type())) {
                    case 1: {
                        Word(8) op = read8(operands[0]);
                        Word(8) output = policy.shiftRightArithmetic(op, shiftCount);
                        Word(1) newCf = policy.ite(shiftCountZero,
                                                   policy.readFlag(x86_flag_cf),
                                                   extract<0, 1>(policy.shiftRight(op, policy.add(shiftCount, number<5>(7)))));
                        policy.writeFlag(x86_flag_cf, newCf);
                        // No change with sc = 0, clear when sc = 1, undefined otherwise 
                        policy.writeFlag(x86_flag_of, policy.ite(shiftCountZero,
                                                                 policy.readFlag(x86_flag_of),
                                                                 policy.false_()));
                        write8(operands[0], output);
                        setFlagsForResult<8>(output, shiftCountNotZero);
                        break;
                    }
                    case 2: {
                        Word(16) op = read16(operands[0]);
                        Word(16) output = policy.shiftRightArithmetic(op, shiftCount);
                        Word(1) newCf = policy.ite(shiftCountZero,
                                                   policy.readFlag(x86_flag_cf),
                                                   extract<0, 1>(policy.shiftRight(op, policy.add(shiftCount, number<5>(15)))));
                        policy.writeFlag(x86_flag_cf, newCf);
                        // No change with sc = 0, clear when sc = 1, undefined otherwise 
                        policy.writeFlag(x86_flag_of, policy.ite(shiftCountZero,
                                                                 policy.readFlag(x86_flag_of),
                                                                 policy.false_()));
                        write16(operands[0], output);
                        setFlagsForResult<16>(output, shiftCountNotZero);
                        break;
                    }
                    case 4: {
                        Word(32) op = read32(operands[0]);
                        Word(32) output = policy.shiftRightArithmetic(op, shiftCount);
                        Word(1) newCf = policy.ite(shiftCountZero,
                                                   policy.readFlag(x86_flag_cf),
                                                   extract<0, 1>(policy.shiftRight(op, policy.add(shiftCount, number<5>(31)))));
                        policy.writeFlag(x86_flag_cf, newCf);
                        // No change with sc = 0, clear when sc = 1, undefined otherwise 
                        policy.writeFlag(x86_flag_of, policy.ite(shiftCountZero,
                                                                 policy.readFlag(x86_flag_of),
                                                                 policy.false_()));
                        write32(operands[0], output);
                        setFlagsForResult<32>(output, shiftCountNotZero);
                        break;
                    }
                    case 8: {
                        Word(64) op = read64(operands[0]);
                        Word(64) output = policy.shiftRightArithmetic(op, shiftCount);
                        Word(1) newCf = policy.ite(shiftCountZero,
                                                   policy.readFlag(x86_flag_cf),
                                                   extract<0, 1>(policy.shiftRight(op, policy.add(shiftCount, number<5>(63)))));
                        policy.writeFlag(x86_flag_cf, newCf);
                        // No change with sc = 0, clear when sc = 1, undefined otherwise 
                        policy.writeFlag(x86_flag_of, policy.ite(shiftCountZero,
                                                                 policy.readFlag(x86_flag_of),
                                                                 policy.false_()));
                        write64(operands[0], output);
                        setFlagsForResult<64>(output, shiftCountNotZero);
                        break;
                    }

                    default:
                        ROSE_ASSERT(!"Bad size");
                }
                break;
            }
/*
            case x86_rol: {
                switch (numBytesInAsmType(operands[0]->get_type())) {
                    case 1: {
                        Word(8) op = read8(operands[0]);
                        Word(5) shiftCount = extract<0, 5>(read8(operands[1]));
                        Word(8) output = policy.rotateLeft(op, shiftCount);
                        policy.writeFlag(x86_flag_cf, policy.ite(policy.equalToZero(shiftCount),
                                                                 policy.readFlag(x86_flag_cf),
                                                                 extract<0, 1>(output)));
                        policy.writeFlag(x86_flag_of, policy.ite(policy.equalToZero(shiftCount),
                                                                 policy.readFlag(x86_flag_of),
                                                                 policy.xor_(extract<0, 1>(output), extract<7, 8>(output))));
                        write8(operands[0], output);
                        break;
                    }
                    case 2: {
                        Word(16) op = read16(operands[0]);
                        Word(5) shiftCount = extract<0, 5>(read8(operands[1]));
                        Word(16) output = policy.rotateLeft(op, shiftCount);
                        policy.writeFlag(x86_flag_cf, policy.ite(policy.equalToZero(shiftCount),
                                                                 policy.readFlag(x86_flag_cf),
                                                                 extract<0, 1>(output)));
                        policy.writeFlag(x86_flag_of, policy.ite(policy.equalToZero(shiftCount),
                                                                 policy.readFlag(x86_flag_of),
                                                                 policy.xor_(extract<0, 1>(output), extract<15, 16>(output))));
                        write16(operands[0], output);
                        break;
                    }
                    case 4: {
                        Word(32) op = read32(operands[0]);
                        Word(5) shiftCount = extract<0, 5>(read8(operands[1]));
                        Word(32) output = policy.rotateLeft(op, shiftCount);
                        policy.writeFlag(x86_flag_cf, policy.ite(policy.equalToZero(shiftCount),
                                                                 policy.readFlag(x86_flag_cf),
                                                                 extract<0, 1>(output)));
                        policy.writeFlag(x86_flag_of, policy.ite(policy.equalToZero(shiftCount),
                                                                 policy.readFlag(x86_flag_of),
                                                                 policy.xor_(extract<0, 1>(output), extract<31, 32>(output))));
                        write32(operands[0], output);
                        break;
                    }
                    default:
                        ROSE_ASSERT(!"Bad size");
                }
                break;
            }
*/
            case x86_ror: {
                switch (numBytesInAsmType(operands[0]->get_type())) {
                    case 1: {
                        Word(8) op = read8(operands[0]);
                        Word(5) shiftCount = extract<0, 5>(read8(operands[1]));
                        Word(8) output = policy.rotateRight(op, shiftCount);
                        policy.writeFlag(x86_flag_cf, policy.ite(policy.equalToZero(shiftCount),
                                                                 policy.readFlag(x86_flag_cf),
                                                                 extract<7, 8>(output)));
                        policy.writeFlag(x86_flag_of, policy.ite(policy.equalToZero(shiftCount),
                                                                 policy.readFlag(x86_flag_of),
                                                                 policy.xor_(extract<6, 7>(output), extract<7, 8>(output))));
                        write8(operands[0], output);
                        break;
                    }
                    case 2: {
                        Word(16) op = read16(operands[0]);
                        Word(5) shiftCount = extract<0, 5>(read8(operands[1]));
                        Word(16) output = policy.rotateRight(op, shiftCount);
                        policy.writeFlag(x86_flag_cf, policy.ite(policy.equalToZero(shiftCount),
                                                                 policy.readFlag(x86_flag_cf),
                                                                 extract<15, 16>(output)));
                        policy.writeFlag(x86_flag_of, policy.ite(policy.equalToZero(shiftCount),
                                                                 policy.readFlag(x86_flag_of),
                                                                 policy.xor_(extract<14, 15>(output), extract<15, 16>(output))));
                        write16(operands[0], output);
                        break;
                    }
                    case 4: {
                        Word(32) op = read32(operands[0]);
                        Word(5) shiftCount = extract<0, 5>(read8(operands[1]));
                        Word(32) output = policy.rotateRight(op, shiftCount);
                        policy.writeFlag(x86_flag_cf, policy.ite(policy.equalToZero(shiftCount),
                                                                 policy.readFlag(x86_flag_cf),
                                                                 extract<31, 32>(output)));
                        policy.writeFlag(x86_flag_of, policy.ite(policy.equalToZero(shiftCount),
                                                                 policy.readFlag(x86_flag_of),
                                                                 policy.xor_(extract<30, 31>(output), extract<31, 32>(output))));
                        write32(operands[0], output);
                        break;
                    }
                    case 8: {
                        Word(64) op = read64(operands[0]);
                        Word(5) shiftCount = extract<0, 5>(read8(operands[1]));
                        Word(64) output = policy.rotateRight(op, shiftCount);
                        policy.writeFlag(x86_flag_cf, policy.ite(policy.equalToZero(shiftCount),
                                                                 policy.readFlag(x86_flag_cf),
                                                                 extract<63, 64>(output)));
                        policy.writeFlag(x86_flag_of, policy.ite(policy.equalToZero(shiftCount),
                                                                 policy.readFlag(x86_flag_of),
                                                                 policy.xor_(extract<62, 63>(output), extract<63, 64>(output))));
                        write64(operands[0], output);
                        break;
                    }

                    default:
                        ROSE_ASSERT(!"Bad size");
                }
                break;
            }
/*
            case x86_shld: {
                Word(5) shiftCount = extract<0, 5>(read8(operands[2]));
                switch (numBytesInAsmType(operands[0]->get_type())) {
                    case 2: {
                        Word(16) op1 = read16(operands[0]);
                        Word(16) op2 = read16(operands[1]);
                        Word(16) output1 = policy.shiftLeft(op1, shiftCount);
                        Word(16) output2 = policy.ite(policy.equalToZero(shiftCount),
                                                      number<16>(0),
                                                      policy.shiftRight(op2, policy.negate(shiftCount)));
                        Word(16) output = policy.or_(output1, output2);
                        Word(1) newCf = policy.ite(policy.equalToZero(shiftCount),
                                                   policy.readFlag(x86_flag_cf),
                                                   extract<15, 16>(policy.shiftLeft(op1, policy.add(shiftCount, number<5>(15)))));
                        policy.writeFlag(x86_flag_cf, newCf);
                        Word(1) newOf = policy.ite(policy.equalToZero(shiftCount),
                                                   policy.readFlag(x86_flag_of), 
                                                   policy.xor_(extract<15, 16>(output), newCf));
                        policy.writeFlag(x86_flag_of, newOf);
                        write16(operands[0], output);
                        setFlagsForResult<16>(output);
                        policy.writeFlag(x86_flag_af, policy.ite(policy.equalToZero(shiftCount),
                                                                 policy.readFlag(x86_flag_af),
                                                                 policy.undefined_()));
                        break;
                    }
                    case 4: {
                        Word(32) op1 = read32(operands[0]);
                        Word(32) op2 = read32(operands[1]);
                        Word(5) shiftCount = extract<0, 5>(read8(operands[2]));
                        Word(32) output1 = policy.shiftLeft(op1, shiftCount);
                        Word(32) output2 = policy.ite(policy.equalToZero(shiftCount),
                                                      number<32>(0),
                                                      policy.shiftRight(op2, policy.negate(shiftCount)));
                        Word(32) output = policy.or_(output1, output2);
                        Word(1) newCf = policy.ite(policy.equalToZero(shiftCount),
                                                   policy.readFlag(x86_flag_cf),
                                                   extract<31, 32>(policy.shiftLeft(op1, policy.add(shiftCount, number<5>(31)))));
                        policy.writeFlag(x86_flag_cf, newCf);
                        Word(1) newOf = policy.ite(policy.equalToZero(shiftCount),
                                                   policy.readFlag(x86_flag_of), 
                                                   policy.xor_(extract<31, 32>(output), newCf));
                        policy.writeFlag(x86_flag_of, newOf);
                        write32(operands[0], output);
                        setFlagsForResult<32>(output);
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
                Word(5) shiftCount = extract<0, 5>(read8(operands[2]));
                switch (numBytesInAsmType(operands[0]->get_type())) {
                    case 2: {
                        Word(16) op1 = read16(operands[0]);
                        Word(16) op2 = read16(operands[1]);
                        Word(16) output1 = policy.shiftRight(op1, shiftCount);
                        Word(16) output2 = policy.ite(policy.equalToZero(shiftCount),
                                                      number<16>(0),
                                                      policy.shiftLeft(op2, policy.negate(shiftCount)));
                        Word(16) output = policy.or_(output1, output2);
                        Word(1) newCf = policy.ite(policy.equalToZero(shiftCount),
                                                   policy.readFlag(x86_flag_cf),
                                                   extract<0, 1>(policy.shiftRight(op1, policy.add(shiftCount, number<5>(15)))));
                        policy.writeFlag(x86_flag_cf, newCf);
                        Word(1) newOf = policy.ite(policy.equalToZero(shiftCount),
                                                   policy.readFlag(x86_flag_of), 
                                                   policy.xor_(extract<15, 16>(output),
                                                               extract<15, 16>(op1)));
                        policy.writeFlag(x86_flag_of, newOf);
                        write16(operands[0], output);
                        setFlagsForResult<16>(output);
                        policy.writeFlag(x86_flag_af, policy.ite(policy.equalToZero(shiftCount),
                                                                 policy.readFlag(x86_flag_af),
                                                                 policy.undefined_()));
                        break;
                    }
                    case 4: {
                        Word(32) op1 = read32(operands[0]);
                        Word(32) op2 = read32(operands[1]);
                        Word(32) output1 = policy.shiftRight(op1, shiftCount);
                        Word(32) output2 = policy.ite(policy.equalToZero(shiftCount),
                                                      number<32>(0),
                                                      policy.shiftLeft(op2, policy.negate(shiftCount)));
                        Word(32) output = policy.or_(output1, output2);
                        Word(1) newCf = policy.ite(policy.equalToZero(shiftCount),
                                                   policy.readFlag(x86_flag_cf),
                                                   extract<0, 1>(policy.shiftRight(op1, policy.add(shiftCount, number<5>(31)))));
                        policy.writeFlag(x86_flag_cf, newCf);
                        Word(1) newOf = policy.ite(policy.equalToZero(shiftCount),
                                                   policy.readFlag(x86_flag_of), 
                                                   policy.xor_(extract<31, 32>(output),
                                                               extract<31, 32>(op1)));
                        policy.writeFlag(x86_flag_of, newOf);
                        write32(operands[0], output);
                        setFlagsForResult<32>(output);
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
	    */
            case x86_bsf: {
                policy.writeFlag(x86_flag_of, policy.undefined_());
                policy.writeFlag(x86_flag_sf, policy.undefined_());
                policy.writeFlag(x86_flag_af, policy.undefined_());
                policy.writeFlag(x86_flag_pf, policy.undefined_());
                policy.writeFlag(x86_flag_cf, policy.undefined_());
                switch (numBytesInAsmType(operands[0]->get_type())) {
                    case 2: {
                        Word(16) op = read16(operands[1]);
                        policy.writeFlag(x86_flag_zf, policy.equalToZero(op));
                        Word(16) result = policy.ite(policy.readFlag(x86_flag_zf),
                                                     read16(operands[0]),
                                                     policy.leastSignificantSetBit(op));
                        write16(operands[0], result);
                        break;
                    }
                    case 4: {
                        Word(32) op = read32(operands[1]);
                        policy.writeFlag(x86_flag_zf, policy.equalToZero(op));
                        Word(32) result = policy.ite(policy.readFlag(x86_flag_zf),
                                                     read32(operands[0]),
                                                     policy.leastSignificantSetBit(op));
                        write32(operands[0], result);
                        break;
                    }
		    case 8: {
                        Word(64) op = read64(operands[1]);
                        policy.writeFlag(x86_flag_zf, policy.equalToZero(op));
                        Word(64) result = policy.ite(policy.readFlag(x86_flag_zf),
                                                     read64(operands[0]),
                                                     policy.leastSignificantSetBit(op));
                        write64(operands[0], result);
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
                        Word(16) op = read16(operands[1]);
                        policy.writeFlag(x86_flag_zf, policy.equalToZero(op));
                        Word(16) result = policy.ite(policy.readFlag(x86_flag_zf),
                                                     read16(operands[0]),
                                                     policy.mostSignificantSetBit(op));
                        write16(operands[0], result);
                        break;
                    }
                    case 4: {
                        Word(32) op = read32(operands[1]);
                        policy.writeFlag(x86_flag_zf, policy.equalToZero(op));
                        Word(32) result = policy.ite(policy.readFlag(x86_flag_zf),
                                                     read32(operands[0]),
                                                     policy.mostSignificantSetBit(op));
                        write32(operands[0], result);
                        break;
                    }
		    case 8: {
                        Word(64) op = read64(operands[1]);
                        policy.writeFlag(x86_flag_zf, policy.equalToZero(op));
                        Word(64) result = policy.ite(policy.readFlag(x86_flag_zf),
                                                     read64(operands[0]),
                                                     policy.mostSignificantSetBit(op));
                        write64(operands[0], result);
			break;
		    }
                    default:
                        ROSE_ASSERT(!"Bad size");
                }
                break;
            }
	    /*
            case x86_bts: {
                ROSE_ASSERT(operands.size() == 2);
                // All flags except CF are undefined 
                policy.writeFlag(x86_flag_of, policy.undefined_());
                policy.writeFlag(x86_flag_sf, policy.undefined_());
                policy.writeFlag(x86_flag_zf, policy.undefined_());
                policy.writeFlag(x86_flag_af, policy.undefined_());
                policy.writeFlag(x86_flag_pf, policy.undefined_());
                if (isSgAsmMemoryReferenceExpression(operands[0]) && isSgAsmx86RegisterReferenceExpression(operands[1])) {
                    // Special case allowing multi-word offsets into memory 
                    Word(32) addr = readEffectiveAddress(operands[0]);
                    int numBytes = numBytesInAsmType(operands[1]->get_type());
                    Word(32) bitnum = numBytes == 2 ? signExtend<16, 32>(read16(operands[1])) : read32(operands[1]);
                    Word(32) adjustedAddr = policy.add(addr, signExtend<29, 32>(extract<3, 32>(bitnum)));
                    Word(8) val = readMemory<8>(getSegregFromMemoryReference(isSgAsmMemoryReferenceExpression(operands[0])),
                                                adjustedAddr, policy.true_());
                    Word(1) bitval = extract<0, 1>(policy.rotateRight(val, extract<0, 3>(bitnum)));
                    Word(8) result = policy.or_(val, policy.rotateLeft(number<8>(1), extract<0, 3>(bitnum)));
                    policy.writeFlag(x86_flag_cf, bitval);
                    policy.writeMemory(getSegregFromMemoryReference(isSgAsmMemoryReferenceExpression(operands[0])),
                                       adjustedAddr, result, policy.true_());
                } else {
                    // Simple case 
                    switch (numBytesInAsmType(operands[0]->get_type())) {
                        case 2: {
                            Word(16) op0 = read16(operands[0]);
                            Word(4) bitnum = extract<0, 4>(read16(operands[1]));
                            Word(1) bitval = extract<0, 1>(policy.rotateRight(op0, bitnum));
                            Word(16) result = policy.or_(op0, policy.rotateLeft(number<16>(1), bitnum));
                            policy.writeFlag(x86_flag_cf, bitval);
                            write16(operands[0], result);
                            break;
                        }
                        case 4: {
                            Word(32) op0 = read32(operands[0]);
                            Word(5) bitnum = extract<0, 5>(read32(operands[1]));
                            Word(1) bitval = extract<0, 1>(policy.rotateRight(op0, bitnum));
                            Word(32) result = policy.or_(op0, policy.rotateLeft(number<32>(1), bitnum));
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
*/
            case x86_imul: {
                switch (numBytesInAsmType(operands[0]->get_type())) {
                    case 1: {
                        Word(8) op0 = extract<0, 8>(policy.readGPR(x86_gpr_ax));
                        Word(8) op1 = read8(operands[0]);
                        Word(16) mulResult = policy.signedMultiply(op0, op1);
                        updateGPRLowWord(x86_gpr_ax, mulResult);
                        Word(1) carry = policy.invert(policy.or_(policy.equalToZero(policy.invert(extract<7, 16>(mulResult))),
                                                                 policy.equalToZero(extract<7, 16>(mulResult))));
                        policy.writeFlag(x86_flag_cf, carry);
                        policy.writeFlag(x86_flag_of, carry);
                        break;
                    }
                    case 2: {
                        Word(16) op0 = operands.size() == 1 ?
                                       extract<0, 16>(policy.readGPR(x86_gpr_ax)) :
                                       read16(operands[operands.size() - 2]);
                        Word(16) op1 = read16(operands[operands.size() - 1]);
                        Word(32) mulResult = policy.signedMultiply(op0, op1);
                        if (operands.size() == 1) {
                            updateGPRLowWord(x86_gpr_ax, extract<0, 16>(mulResult));
                            updateGPRLowWord(x86_gpr_dx, extract<16, 32>(mulResult));
                        } else {
                            write16(operands[0], extract<0, 16>(mulResult));
                        }
                        Word(1) carry = policy.invert(policy.or_(policy.equalToZero(policy.invert(extract<7, 32>(mulResult))),
                                                                 policy.equalToZero(extract<7, 32>(mulResult))));
                        policy.writeFlag(x86_flag_cf, carry);
                        policy.writeFlag(x86_flag_of, carry);
                        break;
                    }
                    case 4: {
                        Word(32) op0 = (operands.size() == 1) ? extract<0,32>(policy.readGPR(x86_gpr_ax)) : read32(operands[operands.size() - 2]);
                        Word(32) op1 = read32(operands[operands.size() - 1]);
                        Word(64) mulResult = policy.signedMultiply(op0, op1);
                        if (operands.size() == 1) {
                            updateGPRLowDWord(x86_gpr_ax, extract<0, 32>(mulResult));
                            updateGPRLowDWord(x86_gpr_dx, extract<32, 64>(mulResult));
                        } else {
                            write32(operands[0], extract<0, 32>(mulResult));
                        }
                        Word(1) carry = policy.invert(policy.or_(policy.equalToZero(policy.invert(extract<7, 64>(mulResult))),
                                                                 policy.equalToZero(extract<7, 64>(mulResult))));
                        policy.writeFlag(x86_flag_cf, carry);
                        policy.writeFlag(x86_flag_of, carry);
                        break;
                    }
                    case 8: {
                        Word(64) op0 = operands.size() == 1 ? policy.readGPR(x86_gpr_ax) : read64(operands[operands.size() - 2]);
                        Word(64) op1 = read64(operands[operands.size() - 1]);
                        Word(128) mulResult = policy.signedMultiply(op0, op1);
                        if (operands.size() == 1) {
                            policy.writeGPR(x86_gpr_ax, extract<0, 64>(mulResult));
                            policy.writeGPR(x86_gpr_dx, extract<64, 128>(mulResult));
                        } else {
                            write64(operands[0], extract<0, 64>(mulResult));
                        }
                        Word(1) carry = policy.invert(policy.or_(policy.equalToZero(policy.invert(extract<7, 128>(mulResult))),
                                                                 policy.equalToZero(extract<7, 128>(mulResult))));
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
                        Word(8) op0 = extract<0, 8>(policy.readGPR(x86_gpr_ax));
                        Word(8) op1 = read8(operands[0]);
                        Word(16) mulResult = policy.unsignedMultiply(op0, op1);
                        updateGPRLowWord(x86_gpr_ax, mulResult);
                        Word(1) carry = policy.invert(policy.equalToZero(extract<8, 16>(mulResult)));
                        policy.writeFlag(x86_flag_cf, carry);
                        policy.writeFlag(x86_flag_of, carry);
                        break;
                    }
                    case 2: {
                        Word(16) op0 = extract<0, 16>(policy.readGPR(x86_gpr_ax));
                        Word(16) op1 = read16(operands[0]);
                        Word(32) mulResult = policy.unsignedMultiply(op0, op1);
                        updateGPRLowWord(x86_gpr_ax, extract<0, 16>(mulResult));
                        updateGPRLowWord(x86_gpr_dx, extract<16, 32>(mulResult));
                        Word(1) carry = policy.invert(policy.equalToZero(extract<16, 32>(mulResult)));
                        policy.writeFlag(x86_flag_cf, carry);
                        policy.writeFlag(x86_flag_of, carry);
                        break;
                    }
                    case 4: {
                        Word(32) op0 = extract<0,32>(policy.readGPR(x86_gpr_ax));
                        Word(32) op1 = read32(operands[0]);
                        Word(64) mulResult = policy.unsignedMultiply(op0, op1);
                        updateGPRLowDWord(x86_gpr_ax, extract<0, 32>(mulResult));
                        updateGPRLowDWord(x86_gpr_dx, extract<32, 64>(mulResult));
                        Word(1) carry = policy.invert(policy.equalToZero(extract<32, 64>(mulResult)));
                        policy.writeFlag(x86_flag_cf, carry);
                        policy.writeFlag(x86_flag_of, carry);
                        break;
                    }
                    case 8: {
                        Word(64) op0 = policy.readGPR(x86_gpr_ax);
                        Word(64) op1 = read64(operands[0]);
                        Word(128) mulResult = policy.unsignedMultiply(op0, op1);
                        policy.writeGPR(x86_gpr_ax, extract<0, 64>(mulResult));
                        policy.writeGPR(x86_gpr_dx, extract<64, 128>(mulResult));
                        Word(1) carry = policy.invert(policy.equalToZero(extract<64, 128>(mulResult)));
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
/*
            case x86_idiv: {
                switch (numBytesInAsmType(operands[0]->get_type())) {
                    case 1: {
                        Word(16) op0 = extract<0, 16>(policy.readGPR(x86_gpr_ax));
                        Word(8) op1 = read8(operands[0]);
                        // if op1 == 0, we should trap 
                        Word(16) divResult = policy.signedDivide(op0, op1);
                        Word(8) modResult = policy.signedModulo(op0, op1);
                        // if result overflows, we should trap 
                        updateGPRLowWord(x86_gpr_ax, policy.concat(extract<0, 8>(divResult), modResult));
                        break;
                    }
                    case 2: {
                        Word(32) op0 = policy.concat(extract<0, 16>(policy.readGPR(x86_gpr_ax)),
                                                     extract<0, 16>(policy.readGPR(x86_gpr_dx)));
                        Word(16) op1 = read16(operands[0]);
                        // if op1 == 0, we should trap 
                        Word(32) divResult = policy.signedDivide(op0, op1);
                        Word(16) modResult = policy.signedModulo(op0, op1);
                        // if result overflows, we should trap 
                        updateGPRLowWord(x86_gpr_ax, extract<0, 16>(divResult));
                        updateGPRLowWord(x86_gpr_dx, modResult);
                        break;
                    }
                    case 4: {
                        Word(64) op0 = policy.concat(policy.readGPR(x86_gpr_ax), policy.readGPR(x86_gpr_dx));
                        Word(32) op1 = read32(operands[0]);
                        // if op1 == 0, we should trap
                        Word(64) divResult = policy.signedDivide(op0, op1);
                        Word(32) modResult = policy.signedModulo(op0, op1);
                        // if result overflows, we should trap
                        policy.writeGPR(x86_gpr_ax, extract<0, 32>(divResult));
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
                        Word(16) op0 = extract<0, 16>(policy.readGPR(x86_gpr_ax));
                        Word(8) op1 = read8(operands[0]);
                        // if op1 == 0, we should trap 
                        Word(16) divResult = policy.unsignedDivide(op0, op1);
                        Word(8) modResult = policy.unsignedModulo(op0, op1);
                        // if extract<8, 16> of divResult is non-zero (overflow), we should trap 
                        updateGPRLowWord(x86_gpr_ax, policy.concat(extract<0, 8>(divResult), modResult));
                        break;
                    }
                    case 2: {
                        Word(32) op0 = policy.concat(extract<0, 16>(policy.readGPR(x86_gpr_ax)),
                                                     extract<0, 16>(policy.readGPR(x86_gpr_dx)));
                        Word(16) op1 = read16(operands[0]);
                        // if op1 == 0, we should trap 
                        Word(32) divResult = policy.unsignedDivide(op0, op1);
                        Word(16) modResult = policy.unsignedModulo(op0, op1);
                        // if extract<16, 32> of divResult is non-zero (overflow), we should trap 
                        updateGPRLowWord(x86_gpr_ax, extract<0, 16>(divResult));
                        updateGPRLowWord(x86_gpr_dx, modResult);
                        break;
                    }
                    case 4: {
                        Word(64) op0 = policy.concat(policy.readGPR(x86_gpr_ax), policy.readGPR(x86_gpr_dx));
                        Word(32) op1 = read32(operands[0]);
                        // if op1 == 0, we should trap 
                        Word(64) divResult = policy.unsignedDivide(op0, op1);
                        Word(32) modResult = policy.unsignedModulo(op0, op1);
                        // if extract<32, 64> of divResult is non-zero (overflow), we should trap 
                        policy.writeGPR(x86_gpr_ax, extract<0, 32>(divResult));
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
                Word(1) incAh = policy.or_(policy.readFlag(x86_flag_af),
                                           greaterOrEqualToTen<4>(extract<0, 4>(policy.readGPR(x86_gpr_ax))));
                updateGPRLowWord(x86_gpr_ax,
                                 policy.concat(policy.add(policy.ite(incAh, number<4>(6), number<4>(0)),
                                                          extract<0, 4>(policy.readGPR(x86_gpr_ax))),
                                               policy.concat(number<4>(0),
                                                             policy.add(policy.ite(incAh, number<8>(1), number<8>(0)),
                                                                        extract<8, 16>(policy.readGPR(x86_gpr_ax))))));
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
                Word(1) decAh = policy.or_(policy.readFlag(x86_flag_af),
                                           greaterOrEqualToTen<4>(extract<0, 4>(policy.readGPR(x86_gpr_ax))));
                updateGPRLowWord(x86_gpr_ax,
                                 policy.concat(policy.add(policy.ite(decAh, number<4>(-6), number<4>(0)),
                                                          extract<0, 4>(policy.readGPR(x86_gpr_ax))),
                                               policy.concat(number<4>(0),
                                                             policy.add(policy.ite(decAh, number<8>(-1), number<8>(0)),
                                                                        extract<8, 16>(policy.readGPR(x86_gpr_ax))))));
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
                Word(8) al = extract<0, 8>(policy.readGPR(x86_gpr_ax));
                Word(8) divisor = read8(operands[0]);
                Word(8) newAh = policy.unsignedDivide(al, divisor);
                Word(8) newAl = policy.unsignedModulo(al, divisor);
                updateGPRLowWord(x86_gpr_ax, policy.concat(newAl, newAh));
                policy.writeFlag(x86_flag_of, policy.undefined_());
                policy.writeFlag(x86_flag_af, policy.undefined_());
                policy.writeFlag(x86_flag_cf, policy.undefined_());
                setFlagsForResult<8>(newAl);
                break;
            }

            case x86_aad: {
                ROSE_ASSERT(operands.size() == 1);
                Word(8) al = extract<0, 8>(policy.readGPR(x86_gpr_ax));
                Word(8) ah = extract<8, 16>(policy.readGPR(x86_gpr_ax));
                Word(8) divisor = read8(operands[0]);
                Word(8) newAl = policy.add(al, extract<0, 8>(policy.unsignedMultiply(ah, divisor)));
                updateGPRLowWord(x86_gpr_ax, policy.concat(newAl, number<8>(0)));
                policy.writeFlag(x86_flag_of, policy.undefined_());
                policy.writeFlag(x86_flag_af, policy.undefined_());
                policy.writeFlag(x86_flag_cf, policy.undefined_());
                setFlagsForResult<8>(newAl);
                break;
            }

            case x86_bswap: {
                ROSE_ASSERT(operands.size() == 1);
                Word(32) oldVal = read32(operands[0]);
                Word(32) newVal = policy.concat(extract<24, 32>(oldVal),
                                                policy.concat(extract<16, 24>(oldVal),
                                                              policy.concat(extract<8, 16>(oldVal),
                                                                            extract<0, 8>(oldVal))));
                write32(operands[0], newVal);
                break;
            }
*/
            case x86_push: {
                ROSE_ASSERT(operands.size() == 1);
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_64);
                ROSE_ASSERT(insn->get_operandSize() == x86_insnsize_64);

                // Override hack for push <segreg>
		// TODO: ask bill
                if (operands[0]->variantT() == V_SgAsmx86RegisterReferenceExpression) {
                   SgAsmx86RegisterReferenceExpression* rre = isSgAsmx86RegisterReferenceExpression(operands[0]);
                   if (rre->get_register_class() == x86_regclass_segment) {
                      Word(64) oldSp = policy.readGPR(x86_gpr_sp);
                      Word(64) newSp = policy.add(oldSp, number<64>(-2));
                      policy.writeMemory(x86_segreg_ss, newSp, read16(operands[0]), policy.true_());
                      policy.writeGPR(x86_gpr_sp, newSp);
                      break;
                   }
                }
                Word(64) oldSp = policy.readGPR(x86_gpr_sp);
                Word(64) newSp = policy.add(oldSp, number<64>(-8));
                policy.writeMemory(x86_segreg_ss, newSp, read64(operands[0]), policy.true_());
                policy.writeGPR(x86_gpr_sp, newSp);                      

                break;
            }
/*
            case x86_pushad: {
                ROSE_ASSERT(operands.size() == 0);
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32);
                Word(32) oldSp = policy.readGPR(x86_gpr_sp);
                Word(32) newSp = policy.add(oldSp, number<32>(-32));
                policy.writeMemory(x86_segreg_ss, newSp, policy.readGPR(x86_gpr_di), policy.true_());
                policy.writeMemory(x86_segreg_ss, policy.add(newSp, number<32>(4)), policy.readGPR(x86_gpr_si), policy.true_());
                policy.writeMemory(x86_segreg_ss, policy.add(newSp, number<32>(8)), policy.readGPR(x86_gpr_bp), policy.true_());
                policy.writeMemory(x86_segreg_ss, policy.add(newSp, number<32>(12)), oldSp, policy.true_());
                policy.writeMemory(x86_segreg_ss, policy.add(newSp, number<32>(16)), policy.readGPR(x86_gpr_bx), policy.true_());
                policy.writeMemory(x86_segreg_ss, policy.add(newSp, number<32>(20)), policy.readGPR(x86_gpr_dx), policy.true_());
                policy.writeMemory(x86_segreg_ss, policy.add(newSp, number<32>(24)), policy.readGPR(x86_gpr_cx), policy.true_());
                policy.writeMemory(x86_segreg_ss, policy.add(newSp, number<32>(28)), policy.readGPR(x86_gpr_ax), policy.true_());
                policy.writeGPR(x86_gpr_sp, newSp);
                break;
            }

            case x86_pushfd: {
                ROSE_ASSERT(operands.size() == 0);
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32);
                Word(32) oldSp = policy.readGPR(x86_gpr_sp);
                Word(32) newSp = policy.add(oldSp, number<32>(-4));
                policy.writeMemory(x86_segreg_ss, newSp, readEflags(), policy.true_());
                policy.writeGPR(x86_gpr_sp, newSp);
                break;
            }
*/

            case x86_pop: {
                ROSE_ASSERT(operands.size() == 1);
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_64);
                ROSE_ASSERT(insn->get_operandSize() == x86_insnsize_64);
                Word(64) oldSp = policy.readGPR(x86_gpr_sp);
                Word(64) newSp = policy.add(oldSp, number<64>(8));
                switch (numBytesInAsmType(operands[0]->get_type())) {
                    case 1: {
                        write8(operands[0], readMemory<8>(x86_segreg_ss, oldSp, policy.true_()));
                        break;
                    }
                    case 2: {
                        write16(operands[0], readMemory<16>(x86_segreg_ss, oldSp, policy.true_()));
                        break;
                    }
                    case 4: {
                        write32(operands[0], readMemory<32>(x86_segreg_ss, oldSp, policy.true_()));
                        break;
                    }
                    case 8: {
                        write64(operands[0], readMemory<64>(x86_segreg_ss, oldSp, policy.true_()));
                        break;
                    }
                    default:
                        ROSE_ASSERT(!"Bad size");
                        break;
                }
                policy.writeGPR(x86_gpr_sp, newSp);

                break;
            }
/*
            case x86_popad: {
                ROSE_ASSERT(operands.size() == 0);
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32);
                Word(32) oldSp = policy.readGPR(x86_gpr_sp);
                Word(32) newSp = policy.add(oldSp, number<32>(32));
                policy.writeGPR(x86_gpr_di, readMemory<32>(x86_segreg_ss, oldSp, policy.true_()));
                policy.writeGPR(x86_gpr_si, readMemory<32>(x86_segreg_ss, policy.add(oldSp, number<32>(4)), policy.true_()));
                policy.writeGPR(x86_gpr_bp, readMemory<32>(x86_segreg_ss, policy.add(oldSp, number<32>(8)), policy.true_()));
                policy.writeGPR(x86_gpr_bx, readMemory<32>(x86_segreg_ss, policy.add(oldSp, number<32>(16)), policy.true_()));
                policy.writeGPR(x86_gpr_dx, readMemory<32>(x86_segreg_ss, policy.add(oldSp, number<32>(20)), policy.true_()));
                policy.writeGPR(x86_gpr_cx, readMemory<32>(x86_segreg_ss, policy.add(oldSp, number<32>(24)), policy.true_()));
                policy.writeGPR(x86_gpr_ax, readMemory<32>(x86_segreg_ss, policy.add(oldSp, number<32>(28)), policy.true_()));
                readMemory<32>(x86_segreg_ss, policy.add(oldSp, number<32>(12)), policy.true_());
                policy.writeGPR(x86_gpr_sp, newSp);
                break;
            }

            case x86_leave: {
                ROSE_ASSERT(operands.size() == 0);
                policy.writeGPR(x86_gpr_sp, policy.readGPR(x86_gpr_bp));
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32);
                ROSE_ASSERT(insn->get_operandSize() == x86_insnsize_32);
                Word(32) oldSp = policy.readGPR(x86_gpr_sp);
                Word(32) newSp = policy.add(oldSp, number<32>(4));
                policy.writeGPR(x86_gpr_bp, readMemory<32>(x86_segreg_ss, oldSp, policy.true_()));
                policy.writeGPR(x86_gpr_sp, newSp);
                break;
            }
*/
            case x86_call: {
                ROSE_ASSERT(operands.size() == 1);
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_64);
                ROSE_ASSERT(insn->get_operandSize() == x86_insnsize_64);
                Word(64) oldSp = policy.readGPR(x86_gpr_sp);
                Word(64) newSp = policy.add(oldSp, number<64>(-4));
                policy.writeMemory(x86_segreg_ss, newSp, policy.readIP(), policy.true_());
                policy.writeIP(policy.filterCallTarget(read64(operands[0])));
                policy.writeGPR(x86_gpr_sp, newSp);
                break;
            }
/*
            case x86_ret: {
                ROSE_ASSERT(operands.size() <= 1);
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32);
                ROSE_ASSERT(insn->get_operandSize() == x86_insnsize_32);
				Word(32) extraBytes = (operands.size() == 1 ? read32(operands[0]) : number<32>(0));
                Word(32) oldSp = policy.readGPR(x86_gpr_sp);
                Word(32) newSp = policy.add(oldSp, policy.add(number<32>(4), extraBytes));
                policy.writeIP(policy.filterReturnTarget(readMemory<32>(x86_segreg_ss, oldSp, policy.true_())));
                policy.writeGPR(x86_gpr_sp, newSp);
                break;
            }

            case x86_loop: {
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32);
                ROSE_ASSERT(insn->get_operandSize() == x86_insnsize_32);
                ROSE_ASSERT(operands.size() == 1);
                Word(32) oldCx = policy.readGPR(x86_gpr_cx);
                Word(32) newCx = policy.add(number<32>(-1), oldCx);
                policy.writeGPR(x86_gpr_cx, newCx);
                Word(1) doLoop = policy.invert(policy.equalToZero(newCx));
                policy.writeIP(policy.ite(doLoop, read32(operands[0]), policy.readIP()));
                break;
            }
            case x86_loopz: {
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32);
                ROSE_ASSERT(insn->get_operandSize() == x86_insnsize_32);
                ROSE_ASSERT(operands.size() == 1);
                Word(32) oldCx = policy.readGPR(x86_gpr_cx);
                Word(32) newCx = policy.add(number<32>(-1), oldCx);
                policy.writeGPR(x86_gpr_cx, newCx);
                Word(1) doLoop = policy.and_(policy.invert(policy.equalToZero(newCx)), policy.readFlag(x86_flag_zf));
                policy.writeIP(policy.ite(doLoop, read32(operands[0]), policy.readIP()));
                break;
            }
            case x86_loopnz: {
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_32);
                ROSE_ASSERT(insn->get_operandSize() == x86_insnsize_32);
                ROSE_ASSERT(operands.size() == 1);
                Word(32) oldCx = policy.readGPR(x86_gpr_cx);
                Word(32) newCx = policy.add(number<32>(-1), oldCx);
                policy.writeGPR(x86_gpr_cx, newCx);
                Word(1) doLoop = policy.and_(policy.invert(policy.equalToZero(newCx)),
                                             policy.invert(policy.readFlag(x86_flag_zf)));
                policy.writeIP(policy.ite(doLoop, read32(operands[0]), policy.readIP()));
                break;
            }
*/
            case x86_jmp: {
                ROSE_ASSERT(operands.size() == 1);
                policy.writeIP(policy.filterIndirectJumpTarget(read64(operands[0])));
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
#           define FLAGCOMBO_cxz   policy.equalToZero(extract<0, 16>(policy.readGPR(x86_gpr_cx)))
#           define FLAGCOMBO_ecxz  policy.equalToZero(policy.readGPR(x86_gpr_cx))

#           define JUMP(tag) {                                                                                                 \
                ROSE_ASSERT(operands.size() == 1);                                                                             \
                policy.writeIP(policy.ite(FLAGCOMBO_##tag,                                                                     \
                                          read64(operands[0]),                                                                 \
                                          policy.readIP()));                                                                   \
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

#           define SET(tag) {                                                                                                  \
                ROSE_ASSERT(operands.size() == 1);                                                                             \
                write8(operands[0], policy.concat(FLAGCOMBO_##tag, number<7>(0)));                                             \
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
                
#           define CMOV(tag) {                                                                                                 \
                ROSE_ASSERT(operands.size() == 2);                                                                             \
                switch (numBytesInAsmType(operands[0]->get_type())) {                                                          \
                    case 2: write16(operands[0], policy.ite(FLAGCOMBO_##tag, read16(operands[1]), read16(operands[0]))); break; \
                    case 4: write32(operands[0], policy.ite(FLAGCOMBO_##tag, read32(operands[1]), read32(operands[0]))); break; \
                    case 8: write64(operands[0], policy.ite(FLAGCOMBO_##tag, read64(operands[1]), read64(operands[0]))); break; \
                    default: ROSE_ASSERT(!"Bad size"); break;                                                                   \
                }                                                                                                              \
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

#           define STRINGOP_LOAD_SI(len, cond)                                                                                 \
                readMemory<(8 * (len))>((insn->get_segmentOverride() == x86_segreg_none ?                                      \
                                         x86_segreg_ds :                                                                       \
                                         insn->get_segmentOverride()),                                                         \
                                        policy.readGPR(x86_gpr_si),                                                            \
                                        (cond))
            
#           define STRINGOP_LOAD_DI(len, cond)                                                                                 \
                readMemory<(8 * (len))>(x86_segreg_es, policy.readGPR(x86_gpr_di), (cond))

            /* If true, repeat this instruction, otherwise go to the next one */
#           define STRINGOP_LOOP_E                                                                                             \
                policy.writeIP(policy.ite(policy.and_(ecxNotZero, policy.readFlag(x86_flag_zf)),                               \
                                          number<64>((uint64_t)(insn->get_address())),                                         \
                                          policy.readIP()))

            /* If true, repeat this instruction, otherwise go to the next one */
#           define STRINGOP_LOOP_NE                                                                                            \
                policy.writeIP(policy.ite(policy.and_(ecxNotZero, policy.invert(policy.readFlag(x86_flag_zf))),                \
                                          number<64>((uint64_t)(insn->get_address())),                                         \
                                          policy.readIP()))

#           define REP_SCAS(suffix, len, repsuffix, loopmacro) {                                                               \
                ROSE_ASSERT(operands.size() == 0);                                                                             \
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_64);                                                       \
                Word(1) ecxNotZero = stringop_setup_loop();                                                                    \
                doAddOperation<(len * 8)>(extract<0, (len * 8)>(policy.readGPR(x86_gpr_ax)),                                   \
                                          policy.invert(STRINGOP_LOAD_DI(len, ecxNotZero)),                                    \
                                          true,                                                                                \
                                          policy.false_(),                                                                     \
                                          ecxNotZero);                                                                         \
                policy.writeGPR(x86_gpr_di,                                                                                    \
                                policy.add(policy.readGPR(x86_gpr_di),                                                         \
                                           policy.ite(policy.readFlag(x86_flag_df), number<64>(-(len)), number<64>(len))));    \
                loopmacro;                                                                                                     \
            }
            case x86_repne_scasb: REP_SCAS(b, 1, ne, STRINGOP_LOOP_NE); break;
            case x86_repne_scasw: REP_SCAS(w, 2, ne, STRINGOP_LOOP_NE); break;
            case x86_repne_scasd: REP_SCAS(d, 4, ne, STRINGOP_LOOP_NE); break;
            case x86_repe_scasb:  REP_SCAS(b, 1, e,  STRINGOP_LOOP_E);  break;
            case x86_repe_scasw:  REP_SCAS(w, 2, e,  STRINGOP_LOOP_E);  break;
            case x86_repe_scasd:  REP_SCAS(d, 4, e,  STRINGOP_LOOP_E);  break;
#           undef REP_SCAS

#           define SCAS(suffix, len) {                                                                                         \
                ROSE_ASSERT(operands.size() == 0);                                                                             \
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_64);                                                       \
                doAddOperation<(len * 8)>(extract<0, (len * 8)>(policy.readGPR(x86_gpr_ax)),                                   \
                                          policy.invert(STRINGOP_LOAD_DI(len, policy.true_())),                                \
                                          true,                                                                                \
                                          policy.false_(),                                                                     \
                                          policy.true_());                                                                     \
                policy.writeGPR(x86_gpr_di,                                                                                    \
                                policy.add(policy.readGPR(x86_gpr_di),                                                         \
                                           policy.ite(policy.readFlag(x86_flag_df), number<64>(-(len)), number<64>(len))));    \
            }
            case x86_scasb: SCAS(b, 1); break;
            case x86_scasw: SCAS(w, 2); break;
            case x86_scasd: SCAS(d, 4); break;
#           undef SCAS

#           define REP_CMPS(suffix, len, repsuffix, loopmacro) {                                                               \
                ROSE_ASSERT(operands.size() == 0);                                                                             \
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_64);                                                       \
                Word(1) ecxNotZero = stringop_setup_loop();                                                                    \
                doAddOperation<(len * 8)>(STRINGOP_LOAD_SI(len, ecxNotZero),                                                   \
                                          policy.invert(STRINGOP_LOAD_DI(len, ecxNotZero)),                                    \
                                          true,                                                                                \
                                          policy.false_(),                                                                     \
                                          ecxNotZero);                                                                         \
                policy.writeGPR(x86_gpr_si,                                                                                    \
                                policy.add(policy.readGPR(x86_gpr_si),                                                         \
                                           policy.ite(ecxNotZero,                                                              \
                                                      policy.ite(policy.readFlag(x86_flag_df),                                 \
                                                                 number<64>(-(len)),                                           \
                                                                 number<64>(len)),                                             \
                                                      number<64>(0))));                                                        \
                policy.writeGPR(x86_gpr_di,                                                                                    \
                                policy.add(policy.readGPR(x86_gpr_di),                                                         \
                                           policy.ite(ecxNotZero,                                                              \
                                                      policy.ite(policy.readFlag(x86_flag_df),                                 \
                                                                 number<64>(-(len)),                                           \
                                                                 number<64>(len)),                                             \
                                                      number<64>(0))));                                                        \
                loopmacro;                                                                                                     \
            }
            case x86_repne_cmpsb: REP_CMPS(b, 1, ne, STRINGOP_LOOP_NE); break;
            case x86_repne_cmpsw: REP_CMPS(w, 2, ne, STRINGOP_LOOP_NE); break;
            case x86_repne_cmpsd: REP_CMPS(d, 4, ne, STRINGOP_LOOP_NE); break;
            case x86_repe_cmpsb:  REP_CMPS(b, 1, e,  STRINGOP_LOOP_E);  break;
            case x86_repe_cmpsw:  REP_CMPS(w, 2, e,  STRINGOP_LOOP_E);  break;
            case x86_repe_cmpsd:  REP_CMPS(d, 4, e,  STRINGOP_LOOP_E);  break;
#           undef REP_CMPS

#           define CMPS(suffix, len) {                                                                                         \
                ROSE_ASSERT(operands.size() == 0);                                                                             \
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_64);                                                       \
                doAddOperation<(len * 8)>(STRINGOP_LOAD_SI(len, policy.true_()),                                               \
                                          policy.invert(STRINGOP_LOAD_DI(len, policy.true_())),                                \
                                          true,                                                                                \
                                          policy.false_(),                                                                     \
                                          policy.true_());                                                                     \
                policy.writeGPR(x86_gpr_si,                                                                                    \
                                policy.add(policy.readGPR(x86_gpr_si),                                                         \
                                           policy.ite(policy.readFlag(x86_flag_df), number<64>(-(len)), number<64>(len))));    \
                policy.writeGPR(x86_gpr_di,                                                                                    \
                                policy.add(policy.readGPR(x86_gpr_di),                                                         \
                                           policy.ite(policy.readFlag(x86_flag_df), number<64>(-(len)), number<64>(len))));    \
            }
            case x86_cmpsb: CMPS(b, 1); break;
            case x86_cmpsw: CMPS(w, 2); break;
            case x86_cmpsd: CMPS(d, 4); break;
#           undef CMPS

#           define MOVS(suffix, len) {                                                                                         \
                ROSE_ASSERT(operands.size() == 0);                                                                             \
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_64);                                                       \
                policy.writeMemory(x86_segreg_es,                                                                              \
                                   policy.readGPR(x86_gpr_di),                                                                 \
                                   STRINGOP_LOAD_SI(len, policy.true_()),                                                      \
                                   policy.true_());                                                                            \
                policy.writeGPR(x86_gpr_si,                                                                                    \
                                policy.add(policy.readGPR(x86_gpr_si),                                                         \
                                           policy.ite(policy.readFlag(x86_flag_df), number<64>(-(len)), number<64>(len))));    \
                policy.writeGPR(x86_gpr_di,                                                                                    \
                                policy.add(policy.readGPR(x86_gpr_di),                                                         \
                                           policy.ite(policy.readFlag(x86_flag_df), number<64>(-(len)), number<64>(len))));    \
            }
            case x86_movsb: MOVS(b, 1); break;
            case x86_movsw: MOVS(w, 2); break;
            case x86_movsd: MOVS(d, 4); break;
#           undef MOVS

#           define REP_MOVS(suffix, len) {                                                                                     \
                ROSE_ASSERT(operands.size() == 0);                                                                             \
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_64);                                                       \
                Word(1) ecxNotZero = stringop_setup_loop();                                                                    \
                policy.writeMemory(x86_segreg_es, policy.readGPR(x86_gpr_di), STRINGOP_LOAD_SI(len, ecxNotZero), ecxNotZero);  \
                policy.writeGPR(x86_gpr_si,                                                                                    \
                                policy.add(policy.readGPR(x86_gpr_si),                                                         \
                                           policy.ite(ecxNotZero,                                                              \
                                                      policy.ite(policy.readFlag(x86_flag_df),                                 \
                                                                 number<64>(-(len)),                                           \
                                                                 number<64>(len)),                                             \
                                                      number<64>(0))));                                                        \
                policy.writeGPR(x86_gpr_di,                                                                                    \
                                policy.add(policy.readGPR(x86_gpr_di),                                                         \
                                           policy.ite(ecxNotZero,                                                              \
                                                      policy.ite(policy.readFlag(x86_flag_df),                                 \
                                                                 number<64>(-(len)),                                           \
                                                                 number<64>(len)),                                             \
                                                      number<64>(0))));                                                        \
                policy.writeIP(policy.ite(ecxNotZero, /* If true, repeat this instruction, otherwise go to the next one */     \
                                          number<64>((uint32_t)(insn->get_address())),                                         \
                                          policy.readIP()));                                                                   \
            }
            case x86_rep_movsb: REP_MOVS(b, 1); break;
            case x86_rep_movsw: REP_MOVS(w, 2); break;
            case x86_rep_movsd: REP_MOVS(d, 4); break;
#           undef REP_MOVS

            case x86_stosb: {
                stos_semantics<1>(insn);
                break;
            }

            case x86_rep_stosb: {
                rep_stos_semantics<1>(insn);
                break;
            }

            case x86_stosw: {
                stos_semantics<2>(insn);
                break;
            }

            case x86_rep_stosw: {
                rep_stos_semantics<2>(insn);
                break;
            }

            case x86_stosd: {
                stos_semantics<4>(insn);
                break;
            }

            case x86_rep_stosd: {
                rep_stos_semantics<4>(insn);
                break;
            }

#           define LODS(suffix, len, regupdate) {                                                                              \
                ROSE_ASSERT(operands.size() == 0);                                                                             \
                ROSE_ASSERT(insn->get_addressSize() == x86_insnsize_64);                                                       \
                regupdate(x86_gpr_ax, STRINGOP_LOAD_SI(len, policy.true_()));                                                  \
                policy.writeGPR(x86_gpr_si,                                                                                    \
                                policy.add(policy.readGPR(x86_gpr_si),                                                         \
                                           policy.ite(policy.readFlag(x86_flag_df), number<64>(-(len)), number<64>(len))));    \
            }
            case x86_lodsb: LODS(b, 1, updateGPRLowByte); break;
            case x86_lodsw: LODS(w, 2, updateGPRLowWord); break;
            case x86_lodsd: LODS(d, 4, updateGPRLowDWord);  break;
#           undef LODS

#undef STRINGOP_LOAD_SI
#undef STRINGOP_LOAD_DI
#undef STRINGOP_UPDATE_CX
#undef STRINGOP_LOOP_E
#undef STRINGOP_LOOP_NE

            case x86_hlt: {
                ROSE_ASSERT(operands.size() == 0);
                policy.hlt();
                policy.writeIP(number<64>((uint64_t)(insn->get_address())));
                break;
            }
/*
            case x86_rdtsc: {
                ROSE_ASSERT(operands.size() == 0);
                Word(64) tsc = policy.rdtsc();
                policy.writeGPR(x86_gpr_ax, extract<0, 32>(tsc));
                policy.writeGPR(x86_gpr_dx, extract<32, 64>(tsc));
                break;
            }

            case x86_int: {
                ROSE_ASSERT(operands.size() == 1);
                SgAsmByteValueExpression* bv = isSgAsmByteValueExpression(operands[0]);
                ROSE_ASSERT(bv);
                policy.interrupt(bv->get_value());
                break;
            }
*/
                /* This is a dummy version that should be replaced later FIXME */
/*            case x86_fnstcw: {
                ROSE_ASSERT(operands.size() == 1);
                write16(operands[0], number<16>(0x37f));
                break;
            }

            case x86_fldcw: {
                ROSE_ASSERT(operands.size() == 1);
                read16(operands[0]); // To catch access control violations 
                break;
            }
*/
            default: {

//                std::cerr <<"Bad instruction [0x" <<std::hex <<insn->get_address() <<": "// <<unparseInstruction(insn) <<"]"
//                          <<" (skipping semantic analysis)\n";
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

    /*
    void processBlock(const SgAsmStatementPtrList& stmts, size_t begin, size_t end) {
        if (begin == end) return;
        policy.startBlock(stmts[begin]->get_address());
        for (size_t i = begin; i < end; ++i) {
            processInstruction(isSgAsmx86Instruction(stmts[i]));
        }
        policy.finishBlock(stmts[begin]->get_address());
    }
    */

    static bool isRepeatedStringOp(SgAsmx86Instruction* insn) {
        //SgAsmx86Instruction* insn = isSgAsmx86Instruction(s);
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
        //SgAsmx86Instruction* insn = isSgAsmx86Instruction(s);
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

#undef Word

#endif // ROSE_X86_64INSTRUCTIONSEMANTICS_H
