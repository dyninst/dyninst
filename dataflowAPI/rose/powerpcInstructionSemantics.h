#ifndef ROSE_POWERPCINSTRUCTIONSEMANTICS_H
#define ROSE_POWERPCINSTRUCTIONSEMANTICS_H

//#include "rose.h"
#include "semanticsModule.h"
#include <cassert>
#include <cstdio>
#include <iostream>
#include "integerOps.h"

#include "SgAsmExpression.h"
#include "SgAsmPowerpcInstruction.h"
#include "conversions.h"

#ifdef Word
#error \
    "Having a macro called \"Word\" conflicts with powerpcInstructionSemantics.h"
#endif

template <typename Policy, template <size_t> class WordType,
          size_t word_size = 64>
struct PowerpcInstructionSemantics {
#define Word(Len) WordType<(Len)>
  Policy& policy;

  PowerpcInstructionSemantics(Policy& policy) : policy(policy) {}

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

  template <size_t Len>  // In bits
  Word(Len) readMemory(const Word(word_size) & addr, Word(1) cond) {
    return policy.template readMemory<Len>(addr, cond);
  }

  template <size_t Len>
  void writeMemory(const Word(word_size) & addr, const Word(Len) & data,
                   Word(1) cond) {
    policy.template writeMemory<Len>(addr, data, cond);
  }

  Word(32) read32(SgAsmExpression* e) {
    ROSE_ASSERT(e != NULL);
    switch (e->variantT()) {
      case V_SgAsmMemoryReferenceExpression:
        return readMemory<32>(readEffectiveAddress(e), policy.true_());
      case V_SgAsmPowerpcRegisterReferenceExpression: {
        SgAsmPowerpcRegisterReferenceExpression* ref =
            isSgAsmPowerpcRegisterReferenceExpression(e);
        ROSE_ASSERT(ref != NULL);
        switch (ref->get_register_class()) {
          case powerpc_regclass_gpr: {
            Word(32) val =
                extract<0, 32>(policy.readGPR(ref->get_register_number()));
            return val;
          }

          case powerpc_regclass_spr: {
            // printf ("Need support for reading SPR in policy! \n");
            // ROSE_ASSERT(false);
            // printf ("ref->get_register_number() = %d
            // \n",ref->get_register_number());
            Word(32) val =
                extract<0, 32>(policy.readSPR(ref->get_register_number()));
            return val;
          }

          default: {
            fprintf(stderr, "Bad register class %s\n",
                    regclassToString(ref->get_register_class()));
            abort();
          }
        }
      }
      default:
        assert(false);
    }
    return number<32>(0);
  }

  // DQ (10/20/2008): changed name of function from templated read version.
  Word(word_size) read_word_size(SgAsmExpression* e) {
    // This function does the address evaluation.

    ROSE_ASSERT(e != NULL);
    // printf ("In read32(): e = %p = %s \n",e,e->class_name().c_str());

    switch (e->variantT()) {
      // DQ (10/26/2008): Don't we need to handle the case of a
      // SgAsmMemoryReferenceExpression
      // so that readEffectiveAddress() will operate properly!

      case V_SgAsmBinaryAdd: {
        SgAsmBinaryAdd* binaryAdd = isSgAsmBinaryAdd(e);
        Word(word_size) lhs_value = read_word_size(binaryAdd->get_lhs());
        Word(word_size) rhs_value = read_word_size(binaryAdd->get_rhs());
        return policy.add(lhs_value, rhs_value);
      }

      case V_SgAsmMemoryReferenceExpression: {
        return readMemory<word_size>(readEffectiveAddress(e), policy.true_());
      }

      case V_SgAsmByteValueExpression:
      case V_SgAsmWordValueExpression:
      case V_SgAsmDoubleWordValueExpression: {
        uint64_t val =
            SageInterface::getAsmSignedConstant(isSgAsmValueExpression(e));
        // std::cerr << "read32 of immediate, returning " << (val &
        // 0xFFFFFFFFULL) << std::endl;
        return number<word_size>(val & 0xFFFFFFFFULL);
      }

      case V_SgAsmQuadWordValueExpression: {
        uint64_t val = isSgAsmQuadWordValueExpression(e)->get_value();
        return number<word_size>(val);
      }

      case V_SgAsmPowerpcRegisterReferenceExpression: {
        SgAsmPowerpcRegisterReferenceExpression* ref =
            isSgAsmPowerpcRegisterReferenceExpression(e);
        ROSE_ASSERT(ref != NULL);
        switch (ref->get_register_class()) {
          case powerpc_regclass_gpr: {
            Word(word_size) val = policy.readGPR(ref->get_register_number());
            return val;
          }

          case powerpc_regclass_spr: {
            // printf ("Need support for reading SPR in policy! \n");
            // ROSE_ASSERT(false);
            // printf ("ref->get_register_number() = %d
            // \n",ref->get_register_number());
            Word(word_size) val = policy.readSPR(ref->get_register_number());
            return val;
          }

          default: {
            fprintf(stderr, "Bad register class %s\n",
                    regclassToString(ref->get_register_class()));
            abort();
          }
        }
      }
      default:
        fprintf(stderr, "Bad variant %s in read32\n", e->class_name().c_str());
        abort();
    }
    return number<word_size>(0);
  }

  Word(word_size) readEffectiveAddress(SgAsmExpression* expr) {
    assert(isSgAsmMemoryReferenceExpression(expr));

    // This must be a SgAsmExpression that is supported by read32(), else it
    // will be an error.
    // The case of "D(RA)" as an operand is such a case...(the type is a
    // SgAsmBinaryAdd).
    return read_word_size(
        isSgAsmMemoryReferenceExpression(expr)->get_address());
  }

  Word(16) read16(SgAsmExpression* e) {
    ROSE_ASSERT(e != NULL);

    switch (e->variantT()) {
      case V_SgAsmMemoryReferenceExpression: {
        return readMemory<16>(readEffectiveAddress(e), policy.true_());
      }

      default:
        fprintf(stderr, "Bad variant %s in read16\n", e->class_name().c_str());
        abort();
    }
    return number<16>(0);
  }

  Word(8) read8(SgAsmExpression* e) {
    ROSE_ASSERT(e != NULL);

    switch (e->variantT()) {
      case V_SgAsmMemoryReferenceExpression: {
        return readMemory<8>(readEffectiveAddress(e), policy.true_());
      }

      default:
        fprintf(stderr, "Bad variant %s in read8\n", e->class_name().c_str());
        abort();
    }
    return number<8>(0);
  }

  void write8(SgAsmExpression* e, const Word(8) & value) {
    switch (e->variantT()) {
      case V_SgAsmMemoryReferenceExpression: {
        writeMemory<8>(readEffectiveAddress(e), value, policy.true_());
        break;
      }
      default:
        fprintf(stderr, "Bad variant %s in write8\n", e->class_name().c_str());
        abort();
    }
  }

  void write16(SgAsmExpression* e, const Word(16) & value) {
    switch (e->variantT()) {
      case V_SgAsmMemoryReferenceExpression: {
        writeMemory<16>(readEffectiveAddress(e), value, policy.true_());
        break;
      }
      default:
        fprintf(stderr, "Bad variant %s in write16\n", e->class_name().c_str());
        abort();
    }
  }

  void write32(SgAsmExpression* e, const Word(32) & value) {
    switch (e->variantT()) {
      case V_SgAsmMemoryReferenceExpression: {
        writeMemory<32>(readEffectiveAddress(e), value, policy.true_());
        break;
      }
      case V_SgAsmPowerpcRegisterReferenceExpression: {
        SgAsmPowerpcRegisterReferenceExpression* ref =
            isSgAsmPowerpcRegisterReferenceExpression(e);
        switch (ref->get_register_class()) {
          case powerpc_regclass_gpr: {
            policy.writeGPR(ref->get_register_number(),
                            policy.concat(number<word_size - 32>(0), value));
            break;
          }

          case powerpc_regclass_spr: {
            // printf ("Need support for writing SPR in policy! \n");
            // ROSE_ASSERT(false);

            policy.writeSPR(ref->get_register_number(),
                            policy.concat(number<word_size - 32>(0), value));
            break;
          }

          default: {
            fprintf(stderr, "Bad register class %s\n",
                    regclassToString(ref->get_register_class()));
            abort();
          }
        }

        break;
      }
      default:
        fprintf(stderr, "Bad variant %s in write32\n", e->class_name().c_str());
        abort();
    }
  }
  void write_word_size(SgAsmExpression* e, const Word(word_size) & value) {
    switch (e->variantT()) {
      case V_SgAsmMemoryReferenceExpression: {
        writeMemory<word_size>(readEffectiveAddress(e), value, policy.true_());
        break;
      }
      case V_SgAsmPowerpcRegisterReferenceExpression: {
        SgAsmPowerpcRegisterReferenceExpression* ref =
            isSgAsmPowerpcRegisterReferenceExpression(e);
        switch (ref->get_register_class()) {
          case powerpc_regclass_gpr: {
            policy.writeGPR(ref->get_register_number(), value);
            break;
          }

          case powerpc_regclass_spr: {
            // printf ("Need support for writing SPR in policy! \n");
            // ROSE_ASSERT(false);

            policy.writeSPR(ref->get_register_number(), value);
            break;
          }

          default: {
            fprintf(stderr, "Bad register class %s\n",
                    regclassToString(ref->get_register_class()));
            abort();
          }
        }

        break;
      }
      default:
        fprintf(stderr, "Bad variant %s in write32\n", e->class_name().c_str());
        abort();
    }
  }

  void record(Word(word_size) result) {
    Word(3) c = policy.ite(policy.equalToZero(result), number<3>(1),
                           policy.ite(extract<word_size - 1, word_size>(result),
                                      number<3>(4), number<3>(2)));

    // This should be a helper function!
    Word(1) SO =
        extract<word_size - 1, word_size>(policy.readSPR(powerpc_spr_xer));

    // Put "SO" into the lower bits, and "c" into the higher order bits
    policy.writeCRField(0, policy.concat(SO, c));
  }

  uint32_t build_mask(uint8_t mb_value, uint8_t me_value) {
    // Builds mask of 1's from the bit value starting at mb_value to me_value.
    // See page 71 of PowerPC manual.

    uint32_t mask = 0;
    if (mb_value <= me_value) {
      // PowerPC counts bits from the left.
      for (int i = mb_value; i <= me_value; i++) mask |= (1 << (31 - i));
    } else {
      for (int i = mb_value; i <= 31; i++) mask |= (1 << (31 - i));
      for (int i = 0; i <= me_value; i++) mask |= (1 << (31 - i));
    }

    return mask;
  }

  void translate(SgAsmPowerpcInstruction* insn) {
    policy.writeIP(policy.template number<word_size>(
        (unsigned int)(insn->get_address() + 4)));
    PowerpcInstructionKind kind = insn->get_kind();
    const SgAsmExpressionPtrList& operands =
        insn->get_operandList()->get_operands();
    switch (kind) {
      // General questions:
      //    1) What is the role of the SgAsmExpression's vs. uint32_t vs.
      //    Word(word_size).
      //    2) The write32() uses the readEffectiveAddress() to write to memory,
      //    but
      //       when the address is a computed value (e.g. "D(RA)" it is not
      //       clear
      //       where this should be evaluated, unless we should be generating a
      //

      case powerpc_nor: {
        ROSE_ASSERT(operands.size() == 3);
        write_word_size(operands[0],
                        policy.invert(policy.or_(read_word_size(operands[1]),
                                                 read_word_size(operands[2]))));
        break;
      }

      case powerpc_or: {
        ROSE_ASSERT(operands.size() == 3);
        write_word_size(operands[0], policy.or_(read_word_size(operands[1]),
                                                read_word_size(operands[2])));
        break;
      }
      case powerpc_orc: {
        ROSE_ASSERT(operands.size() == 3);
        write_word_size(operands[0],
                        policy.or_(read_word_size(operands[1]),
                                   policy.invert(read_word_size(operands[2]))));
        break;
      }
      case powerpc_fmr: {
        ROSE_ASSERT(operands.size() == 2);
        write_word_size(operands[0], read_word_size(operands[1]));
        break;
      }

      case powerpc_or_record: {
        ROSE_ASSERT(operands.size() == 3);
        Word(word_size) result = policy.or_(read_word_size(operands[1]),
                                            read_word_size(operands[2]));
        write_word_size(operands[0], result);
        record(result);
        break;
      }

      case powerpc_ori: {
        ROSE_ASSERT(operands.size() == 3);
        write_word_size(operands[0], policy.or_(read_word_size(operands[1]),
                                                read_word_size(operands[2])));
        break;
      }

      case powerpc_oris: {
        ROSE_ASSERT(operands.size() == 3);
        write_word_size(
            operands[0],
            policy.or_(read_word_size(operands[1]),
                       policy.concat(number<word_size - 16>(0),
                                     extract<0, 16>(read32(operands[2])))));
        break;
      }

      case powerpc_xor: {
        ROSE_ASSERT(operands.size() == 3);
        write_word_size(operands[0], policy.xor_(read_word_size(operands[1]),
                                                 read_word_size(operands[2])));
        break;
      }
      case powerpc_xor_record: {
        ROSE_ASSERT(operands.size() == 3);
        Word(word_size) result = policy.xor_(read_word_size(operands[1]),
                                             read_word_size(operands[2]));
        write_word_size(operands[0], result);
        record(result);
        break;
      }

      case powerpc_xori: {
        ROSE_ASSERT(operands.size() == 3);
        write_word_size(operands[0], policy.xor_(read_word_size(operands[1]),
                                                 read_word_size(operands[2])));
        break;
      }

      case powerpc_xoris: {
        ROSE_ASSERT(operands.size() == 3);
        write_word_size(
            operands[0],
            policy.xor_(
                read_word_size(operands[1]),
                policy.concat(number<word_size - 16>(0),
                              extract<0, 16>(read_word_size(operands[2])))));
        break;
      }

      case powerpc_rlwinm: {
        ROSE_ASSERT(operands.size() == 5);
        Word(word_size) RS = read_word_size(operands[1]);
        Word(5) SH = extract<0, 5>(read_word_size(operands[2]));

        SgAsmByteValueExpression* MB = isSgAsmByteValueExpression(operands[3]);
        ROSE_ASSERT(MB != NULL);
        int mb_value = MB->get_value();

        SgAsmByteValueExpression* ME = isSgAsmByteValueExpression(operands[4]);
        ROSE_ASSERT(ME != NULL);

        int me_value = ME->get_value();
        uint32_t mask = build_mask(mb_value, me_value);

        Word(word_size) rotatedReg = policy.rotateLeft(RS, SH);
        Word(word_size) bitMask = number<word_size>(mask);

        write_word_size(operands[0], policy.and_(rotatedReg, bitMask));
        break;
      }

      case powerpc_rlwinm_record: {
        ROSE_ASSERT(operands.size() == 5);
        Word(word_size) RS = read_word_size(operands[1]);
        Word(5) SH = extract<0, 5>(read_word_size(operands[2]));

        SgAsmByteValueExpression* MB = isSgAsmByteValueExpression(operands[3]);
        ROSE_ASSERT(MB != NULL);
        int mb_value = MB->get_value();

        SgAsmByteValueExpression* ME = isSgAsmByteValueExpression(operands[4]);
        ROSE_ASSERT(ME != NULL);

        int me_value = ME->get_value();
        uint32_t mask = build_mask(mb_value, me_value);

        Word(word_size) rotatedReg = policy.rotateLeft(RS, SH);
        Word(word_size) bitMask = number<word_size>(mask);
        Word(word_size) result = policy.and_(rotatedReg, bitMask);
        write_word_size(operands[0], result);
        record(result);
        break;
      }

      case powerpc_rlwimi: {
        ROSE_ASSERT(operands.size() == 5);
        Word(word_size) RS = read_word_size(operands[0]);
        Word(word_size) RA = read_word_size(operands[1]);
        Word(5) SH = extract<0, 5>(read_word_size(operands[2]));

        SgAsmByteValueExpression* MB = isSgAsmByteValueExpression(operands[3]);
        ROSE_ASSERT(MB != NULL);
        int mb_value = MB->get_value();

        SgAsmByteValueExpression* ME = isSgAsmByteValueExpression(operands[4]);
        ROSE_ASSERT(ME != NULL);

        int me_value = ME->get_value();
        uint32_t mask = build_mask(mb_value, me_value);

        Word(word_size) rotatedReg = policy.rotateLeft(RS, SH);
        Word(word_size) bitMask = number<word_size>(mask);

        write_word_size(operands[0],
                        policy.or_(policy.and_(rotatedReg, bitMask),
                                   policy.and_(RA, policy.invert(bitMask))));
        break;
      }

      case powerpc_addi: {
        ROSE_ASSERT(operands.size() == 3);
        Word(word_size) RA = read_word_size(operands[1]);

        // The disassembler should have built this as a DWord with a sign
        // extended value
        Word(word_size) signExtended_SI = signExtend<16, word_size>(
            extract<0, 16>(read_word_size(operands[2])));

        // "ite" is "if then else"
        // Word(32) result = policy.ite(policy.equalToZero(RA), signExtended_SI,
        // policy.add(RA,signExtended_SI));
        write_word_size(operands[0], policy.add(RA, signExtended_SI));
        break;
      }

      case powerpc_stwu:
      case powerpc_stwux:  // implemented as a memory reference instead of a 3
                           // operand instruction.
      {
        ROSE_ASSERT(operands.size() == 2);
        SgAsmMemoryReferenceExpression* memoryReference =
            isSgAsmMemoryReferenceExpression(operands[1]);
        SgAsmBinaryAdd* binaryAdd =
            isSgAsmBinaryAdd(memoryReference->get_address());
        ROSE_ASSERT(binaryAdd != NULL);

        SgAsmExpression* RA = binaryAdd->get_lhs();

        Word(word_size) effectiveAddress = readEffectiveAddress(operands[1]);
        writeMemory<word_size>(effectiveAddress, read_word_size(operands[0]),
                               policy.true_());
        write_word_size(RA, effectiveAddress);
        break;
      }

      case powerpc_and: {
        ROSE_ASSERT(operands.size() == 3);
        write_word_size(operands[0], policy.and_(read_word_size(operands[1]),
                                                 read_word_size(operands[2])));
        break;
      }
      case powerpc_andc: {
        ROSE_ASSERT(operands.size() == 3);
        write_word_size(
            operands[0],
            policy.and_(read_word_size(operands[1]),
                        policy.invert(read_word_size(operands[2]))));
        break;
      }
      case powerpc_and_record: {
        ROSE_ASSERT(operands.size() == 3);
        Word(word_size) result = policy.and_(read_word_size(operands[1]),
                                             read_word_size(operands[2]));
        write_word_size(operands[0], result);
        record(result);
        break;
      }
      case powerpc_andc_record: {
        ROSE_ASSERT(operands.size() == 3);
        Word(word_size) result =
            policy.and_(read_word_size(operands[1]),
                        policy.invert(read_word_size(operands[2])));
        write_word_size(operands[0], result);
        record(result);
        break;
      }

      case powerpc_mfspr: {
        ROSE_ASSERT(operands.size() == 2);

        // This is only a valid instruction if bits 0-4 or SPR are: 1, 8, or 9.
        // Should we be checking
        // this here or in the disassembler?
        write_word_size(operands[0], read_word_size(operands[1]));
        break;
      }

      case powerpc_mtspr: {
        ROSE_ASSERT(operands.size() == 2);

        // This is only a valid instruction if bits 0-4 or SPR are: 1, 8, or 9.
        // Should we be checking
        // this here or in the disassembler?
        write_word_size(operands[0], read_word_size(operands[1]));
        break;
      }

      case powerpc_stw:
      case powerpc_stwx:
      case powerpc_stwcx_record: {
        ROSE_ASSERT(operands.size() == 2);
        Word(word_size) result = read_word_size(operands[0]);
        write_word_size(operands[1], result);
        if (kind == powerpc_stwcx_record) record(result);
        break;
      }

      case powerpc_stb:
      case powerpc_stbx: {
        ROSE_ASSERT(operands.size() == 2);
        write8(operands[1], extract<0, 8>(read_word_size(operands[0])));
        break;
      }

      case powerpc_sth: {
        ROSE_ASSERT(operands.size() == 2);
        write16(operands[1], extract<0, 16>(read_word_size(operands[0])));
        break;
      }

      case powerpc_addis: {
        ROSE_ASSERT(operands.size() == 3);
        write_word_size(
            operands[0],
            policy.add(
                read_word_size(operands[1]),
                policy.concat(number<word_size - 16>(0),
                              extract<0, 16>(read_word_size(operands[2])))));
        break;
      }

      case powerpc_lwzu: {
        ROSE_ASSERT(operands.size() == 2);
        SgAsmMemoryReferenceExpression* memoryReference =
            isSgAsmMemoryReferenceExpression(operands[1]);
        SgAsmBinaryAdd* binaryAdd =
            isSgAsmBinaryAdd(memoryReference->get_address());
        ROSE_ASSERT(binaryAdd != NULL);

        SgAsmExpression* RA = binaryAdd->get_lhs();

        Word(word_size) effectiveAddress = readEffectiveAddress(operands[1]);
        write_word_size(operands[0], readMemory<word_size>(effectiveAddress,
                                                           policy.true_()));
        write_word_size(RA, effectiveAddress);
        break;
      }

      case powerpc_add: {
        ROSE_ASSERT(operands.size() == 3);
        write_word_size(operands[0], policy.add(read_word_size(operands[1]),
                                                read_word_size(operands[2])));
        break;
      }

      case powerpc_bl: {
        ROSE_ASSERT(operands.size() == 1);
        policy.writeSPR(powerpc_spr_lr,
                        number<word_size>(insn->get_address() + 4));
        Word(word_size) target =
            (policy.add(read_word_size(operands[0]),
                        number<word_size>(insn->get_address())));
        policy.writeIP(target);
        break;
      }

      case powerpc_b: {
        ROSE_ASSERT(operands.size() == 1);
        Word(word_size) target =
            (policy.add(read_word_size(operands[0]),
                        number<word_size>(insn->get_address())));
        policy.writeIP(target);
        break;
      }
      case powerpc_bla: {
        ROSE_ASSERT(operands.size() == 1);
        policy.writeSPR(powerpc_spr_lr,
                        number<word_size>(insn->get_address() + 4));
        policy.writeIP(read_word_size(operands[0]));
        break;
      }
      case powerpc_ba: {
        ROSE_ASSERT(operands.size() == 1);
        policy.writeIP(read_word_size(operands[0]));
        break;
      }
      case powerpc_lwz:
      case powerpc_lwzx:
      case powerpc_lwarx: {
        ROSE_ASSERT(operands.size() == 2);
        write_word_size(operands[0], read_word_size(operands[1]));
        break;
      }

      case powerpc_addc: {
        ROSE_ASSERT(operands.size() == 3);
        Word(word_size) carries = number<word_size>(0);
        Word(word_size) result = policy.addWithCarries(
            read_word_size(operands[1]), read_word_size(operands[2]),
            policy.false_(), carries);

        // Policy class bit numbering is opposite ordering from powerpc (based
        // on x86).
        Word(1) carry_out = extract<word_size - 1, word_size>(carries);
        write_word_size(operands[0], result);

        // This should be a helper function to read/write CA (and other flags)
        // The value 0xDFFFFFFFU is the mask for the Carry (CA) flag
        policy.writeSPR(
            powerpc_spr_xer,
            policy.or_(policy.and_(policy.readSPR(powerpc_spr_xer),
                                   number<word_size>(0xDFFFFFFFU)),
                       policy.ite(carry_out, number<word_size>(0x20000000U),
                                  number<word_size>(0x0))));
        break;
      }

      case powerpc_addic: {
        ROSE_ASSERT(operands.size() == 3);
        Word(word_size) carries = number<word_size>(0);
        Word(word_size) result = policy.addWithCarries(
            read_word_size(operands[1]),
            signExtend<16, word_size>(
                extract<0, 16>(read_word_size(operands[2]))),
            policy.false_(), carries);

        // Policy class bit numbering is opposite ordering from powerpc (based
        // on x86).
        Word(1) carry_out = extract<word_size - 1, word_size>(carries);
        write_word_size(operands[0], result);

        // This should be a helper function to read/write CA (and other flags)
        // The value 0xDFFFFFFFU is the mask for the Carry (CA) flag
        policy.writeSPR(
            powerpc_spr_xer,
            policy.or_(policy.and_(policy.readSPR(powerpc_spr_xer),
                                   number<word_size>(0xDFFFFFFFU)),
                       policy.ite(carry_out, number<word_size>(0x20000000U),
                                  number<word_size>(0x0))));
        break;
      }

      case powerpc_addic_record: {
        ROSE_ASSERT(operands.size() == 3);
        Word(word_size) carries = number<word_size>(0);
        Word(word_size) result = policy.addWithCarries(
            read_word_size(operands[1]),
            signExtend<16, word_size>(
                extract<0, 16>(read_word_size(operands[2]))),
            policy.false_(), carries);

        // Policy class bit numbering is opposite ordering from powerpc (based
        // on x86).
        Word(1) carry_out = extract<word_size - 1, word_size>(carries);
        write_word_size(operands[0], result);

        // This should be a helper function to read/write CA (and other flags)
        // The value 0xDFFFFFFFU is the mask for the Carry (CA) flag
        policy.writeSPR(
            powerpc_spr_xer,
            policy.or_(policy.and_(policy.readSPR(powerpc_spr_xer),
                                   number<word_size>(0xDFFFFFFFU)),
                       policy.ite(carry_out, number<word_size>(0x20000000U),
                                  number<word_size>(0x0))));

        record(result);
        break;
      }

      case powerpc_subfe: {
        ROSE_ASSERT(operands.size() == 3);

        // This should be a helper function to read CA (and other flags)
        Word(1) carry_in = extract<29, 30>(policy.readSPR(powerpc_spr_xer));

        Word(word_size) carries = number<word_size>(0);
        Word(word_size) result = policy.addWithCarries(
            policy.invert(read_word_size(operands[1])),
            read_word_size(operands[2]), carry_in, carries);

        // Policy class bit numbering is opposite ordering from powerpc (based
        // on x86).
        Word(1) carry_out = extract<word_size - 1, word_size>(carries);
        write_word_size(operands[0], result);

        // This should be a helper function to read/write CA (and other flags)
        // The value 0xDFFFFFFFU is the mask for the Carry (CA) flag
        policy.writeSPR(
            powerpc_spr_xer,
            policy.or_(policy.and_(policy.readSPR(powerpc_spr_xer),
                                   number<word_size>(0xDFFFFFFFU)),
                       policy.ite(carry_out, number<word_size>(0x20000000U),
                                  number<word_size>(0x0))));
        break;
      }
      case powerpc_si: {
        ROSE_ASSERT(operands.size() == 3);
        Word(word_size) RA = read_word_size(operands[1]);
        // The disassembler should have built this as a DWord with a sign
        // extended value
        Word(word_size) signExtended_SI = signExtend<16, word_size>(
            extract<0, 16>(read_word_size(operands[2])));

        write_word_size(operands[0],
                        policy.add(RA, policy.invert(signExtended_SI)));

        break;
      }

      case powerpc_subfze: {
        ROSE_ASSERT(operands.size() == 2);

        // This should be a helper function to read CA (and other flags)
        Word(1) carry_in = extract<29, 30>(policy.readSPR(powerpc_spr_xer));

        Word(word_size) carries = number<word_size>(0);
        Word(word_size) result =
            policy.addWithCarries(policy.invert(read_word_size(operands[1])),
                                  number<word_size>(0x0), carry_in, carries);

        // Policy class bit numbering is opposite ordering from powerpc (based
        // on x86).
        Word(1) carry_out = extract<word_size - 1, word_size>(carries);
        write_word_size(operands[0], result);

        // This should be a helper function to read/write CA (and other flags)
        // The value 0xDFFFFFFFU is the mask for the Carry (CA) flag
        policy.writeSPR(
            powerpc_spr_xer,
            policy.or_(policy.and_(policy.readSPR(powerpc_spr_xer),
                                   number<word_size>(0xDFFFFFFFU)),
                       policy.ite(carry_out, number<word_size>(0x20000000U),
                                  number<word_size>(0x0))));
        break;
      }

      case powerpc_subfc: {
        ROSE_ASSERT(operands.size() == 3);
        Word(word_size) carries = number<word_size>(0);
        Word(word_size) result = policy.addWithCarries(
            policy.invert(read_word_size(operands[1])),
            read_word_size(operands[2]), policy.true_(), carries);

        // Policy class bit numbering is opposite ordering from powerpc (based
        // on x86).
        Word(1) carry_out = extract<word_size - 1, word_size>(carries);
        write_word_size(operands[0], result);

        // This should be a helper function to read/write CA (and other flags)
        // The value 0xDFFFFFFFU is the mask for the Carry (CA) flag
        policy.writeSPR(
            powerpc_spr_xer,
            policy.or_(policy.and_(policy.readSPR(powerpc_spr_xer),
                                   number<word_size>(0xDFFFFFFFU)),
                       policy.ite(carry_out, number<word_size>(0x20000000U),
                                  number<word_size>(0x0))));
        break;
      }

      case powerpc_subfic: {
        ROSE_ASSERT(operands.size() == 3);
        Word(word_size) carries = number<word_size>(0);

        // To do the subtraction we invert the first operand and add.  To add
        // "1" we set the carry in to true.
        Word(word_size) result = policy.addWithCarries(
            policy.invert(read_word_size(operands[1])),
            signExtend<16, word_size>(
                extract<0, 16>(read_word_size(operands[2]))),
            policy.true_(), carries);

        // Policy class bit numbering is opposite ordering from powerpc (based
        // on x86).
        Word(1) carry_out = extract<word_size - 1, word_size>(carries);
        write_word_size(operands[0], result);

        // This should be a helper function to read/write CA (and other flags)
        // The value 0xDFFFFFFFU is the mask for the Carry (CA) flag
        policy.writeSPR(
            powerpc_spr_xer,
            policy.or_(policy.and_(policy.readSPR(powerpc_spr_xer),
                                   number<word_size>(0xDFFFFFFFU)),
                       policy.ite(carry_out, number<word_size>(0x20000000U),
                                  number<word_size>(0x0))));
        break;
      }

      case powerpc_lbz:
      case powerpc_lbzx: {
        ROSE_ASSERT(operands.size() == 2);
        write_word_size(operands[0], policy.concat(read8(operands[1]),
                                                   number<word_size - 8>(0)));
        break;
      }

      case powerpc_lbzu:
      case powerpc_lbzux: {
        ROSE_ASSERT(operands.size() == 2);
        SgAsmMemoryReferenceExpression* memoryReference =
            isSgAsmMemoryReferenceExpression(operands[1]);
        SgAsmBinaryAdd* binaryAdd =
            isSgAsmBinaryAdd(memoryReference->get_address());
        ROSE_ASSERT(binaryAdd != NULL);

        SgAsmExpression* RA = binaryAdd->get_lhs();

        Word(word_size) effectiveAddress = readEffectiveAddress(operands[1]);
        write_word_size(operands[0], policy.concat(read8(operands[1]),
                                                   number<word_size - 8>(0)));
        write_word_size(RA, effectiveAddress);
        break;
      }

      case powerpc_lha:
      case powerpc_lhax: {
        ROSE_ASSERT(operands.size() == 2);
        write_word_size(operands[0],
                        signExtend<16, word_size>(read16(operands[1])));
        break;
      }

      case powerpc_lhz:
      case powerpc_lhzx: {
        ROSE_ASSERT(operands.size() == 2);
        write_word_size(operands[0], policy.concat(read16(operands[1]),
                                                   number<word_size - 16>(0)));
        break;
      }

      case powerpc_cmpli: {
        ROSE_ASSERT(operands.size() == 4);
        // For 32-bit case we can ignore value of L

        Word(word_size) RA = read_word_size(operands[2]);
        Word(word_size) UI = read_word_size(operands[3]);

        Word(word_size) carries = number<word_size>(0);

        // Need to check if policy.false_() or policy.true_() should be used!
        // policy.invert(RA) yields "(-RA)-1"
        // Check if UI + (-RA) - 1 >= 0, test for UI > RA
        policy.addWithCarries(policy.invert(RA), UI, policy.false_(), carries);

        Word(3) c =
            policy.ite(policy.equalToZero(policy.xor_(RA, UI)), number<3>(1),
                       policy.ite(extract<word_size - 1, word_size>(carries),
                                  number<3>(4), number<3>(2)));

        SgAsmPowerpcRegisterReferenceExpression* bf =
            isSgAsmPowerpcRegisterReferenceExpression(operands[0]);
        ROSE_ASSERT(bf != NULL);
        ROSE_ASSERT(bf->get_register_class() == powerpc_regclass_cr);
        ROSE_ASSERT(bf->get_conditionRegisterGranularity() ==
                    powerpc_condreggranularity_field);

        // This should be a helper function!
        Word(1) SO =
            extract<word_size - 1, word_size>(policy.readSPR(powerpc_spr_xer));

        policy.writeCRField(bf->get_register_number(), policy.concat(SO, c));
        break;
      }

      case powerpc_cmpl: {
        // This is same as powerpc_cmpli (UI --> RB)

        ROSE_ASSERT(operands.size() == 4);
        // For 32-bit case we can ignore value of L

        Word(word_size) RA = read_word_size(operands[2]);
        Word(word_size) RB = read_word_size(operands[3]);

        Word(word_size) carries = number<word_size>(0);

        // Need to check if policy.false_() or policy.true_() should be used!
        // policy.invert(RA) yields "(-RA)-1"
        // Check if UI + (-RA) - 1 >= 0, test for UI > RA
        policy.addWithCarries(policy.invert(RA), RB, policy.false_(), carries);

        Word(3) c =
            policy.ite(policy.equalToZero(policy.xor_(RA, RB)), number<3>(1),
                       policy.ite(extract<word_size - 1, word_size>(carries),
                                  number<3>(4), number<3>(2)));

        SgAsmPowerpcRegisterReferenceExpression* bf =
            isSgAsmPowerpcRegisterReferenceExpression(operands[0]);
        ROSE_ASSERT(bf != NULL);
        ROSE_ASSERT(bf->get_register_class() == powerpc_regclass_cr);
        ROSE_ASSERT(bf->get_conditionRegisterGranularity() ==
                    powerpc_condreggranularity_field);

        // This should be a helper function!
        Word(1) SO =
            extract<word_size - 1, word_size>(policy.readSPR(powerpc_spr_xer));

        policy.writeCRField(bf->get_register_number(), policy.concat(SO, c));
        break;
      }

      case powerpc_bcl:
        policy.writeSPR(powerpc_spr_lr, policy.readIP());
      // fall through
      case powerpc_bc: {
        ROSE_ASSERT(operands.size() == 3);
        SgAsmByteValueExpression* byteValue =
            isSgAsmByteValueExpression(operands[0]);
        ROSE_ASSERT(byteValue != NULL);
        uint8_t boConstant = byteValue->get_value();

        // bool BO_4 = boConstant & 0x1;
        bool BO_3 = boConstant & 0x2;
        bool BO_2 = boConstant & 0x4;
        bool BO_1 = boConstant & 0x8;
        bool BO_0 = boConstant & 0x10;

        if (!BO_2) {
          policy.writeSPR(powerpc_spr_ctr,
                          policy.add(policy.readSPR(powerpc_spr_ctr),
                                     number<word_size>(-1)));
        }

        Word(1) CTR_ok =
            BO_2 ? policy.true_()
                 : BO_3 ? policy.equalToZero(policy.readSPR(powerpc_spr_ctr))
                        : policy.invert(policy.equalToZero(
                              policy.readSPR(powerpc_spr_ctr)));

        SgAsmPowerpcRegisterReferenceExpression* BI =
            isSgAsmPowerpcRegisterReferenceExpression(operands[1]);
        ROSE_ASSERT(BI != NULL);
        ROSE_ASSERT(BI->get_register_class() == powerpc_regclass_cr);
        ROSE_ASSERT(BI->get_conditionRegisterGranularity() ==
                    powerpc_condreggranularity_bit);

        // This needs a collection of helpfer functions!
        int bi_value = BI->get_register_number();
        Word(4) CR_field = policy.readCRField(bi_value / 4);
        Word(1) CR_bi = extract<0, 1>(
            policy.shiftRight(CR_field, number<2>(3 - bi_value % 4)));
        Word(1) COND_ok =
            BO_0 ? policy.true_() : BO_1 ? CR_bi : policy.invert(CR_bi);
        Word(word_size) target =
            (policy.add(read_word_size(operands[2]),
                        number<word_size>(insn->get_address())));
        policy.writeIP(
            policy.ite(policy.and_(CTR_ok, COND_ok), target, policy.readIP()));
        break;
      }

      case powerpc_bcla:
        policy.writeSPR(powerpc_spr_lr, policy.readIP());
      // fall through
      case powerpc_bca: {
        ROSE_ASSERT(operands.size() == 3);
        SgAsmByteValueExpression* byteValue =
            isSgAsmByteValueExpression(operands[0]);
        ROSE_ASSERT(byteValue != NULL);
        uint8_t boConstant = byteValue->get_value();

        // bool BO_4 = boConstant & 0x1;
        bool BO_3 = boConstant & 0x2;
        bool BO_2 = boConstant & 0x4;
        bool BO_1 = boConstant & 0x8;
        bool BO_0 = boConstant & 0x10;

        if (!BO_2) {
          policy.writeSPR(powerpc_spr_ctr,
                          policy.add(policy.readSPR(powerpc_spr_ctr),
                                     number<word_size>(-1)));
        }

        Word(1) CTR_ok =
            BO_2 ? policy.true_()
                 : BO_3 ? policy.equalToZero(policy.readSPR(powerpc_spr_ctr))
                        : policy.invert(policy.equalToZero(
                              policy.readSPR(powerpc_spr_ctr)));

        SgAsmPowerpcRegisterReferenceExpression* BI =
            isSgAsmPowerpcRegisterReferenceExpression(operands[1]);
        ROSE_ASSERT(BI != NULL);
        ROSE_ASSERT(BI->get_register_class() == powerpc_regclass_cr);
        ROSE_ASSERT(BI->get_conditionRegisterGranularity() ==
                    powerpc_condreggranularity_bit);

        // This needs a collection of helpfer functions!
        int bi_value = BI->get_register_number();
        Word(4) CR_field = policy.readCRField(bi_value / 4);
        Word(1) CR_bi = extract<0, 1>(
            policy.shiftRight(CR_field, number<2>(3 - bi_value % 4)));
        Word(1) COND_ok =
            BO_0 ? policy.true_() : BO_1 ? CR_bi : policy.invert(CR_bi);
        policy.writeIP(policy.ite(policy.and_(CTR_ok, COND_ok),
                                  read_word_size(operands[2]),
                                  policy.readIP()));
        break;
      }

      case powerpc_subf: {
        ROSE_ASSERT(operands.size() == 3);
        write_word_size(operands[0],
                        policy.add(policy.negate(read_word_size(operands[1])),
                                   read_word_size(operands[2])));
        break;
      }

      case powerpc_subf_record: {
        ROSE_ASSERT(operands.size() == 3);

        Word(word_size) result =
            policy.add(policy.negate(read_word_size(operands[1])),
                       read_word_size(operands[2]));
        write_word_size(operands[0], result);

        record(result);
        break;
      }

      case powerpc_bclrl:
        policy.writeSPR(powerpc_spr_lr,
                        number<word_size>(insn->get_address() + 4));
      // fall through to non-linking case
      case powerpc_bclr: {
        ROSE_ASSERT(operands.size() == 3);
        SgAsmByteValueExpression* byteValue =
            isSgAsmByteValueExpression(operands[0]);
        ROSE_ASSERT(byteValue != NULL);
        uint8_t boConstant = byteValue->get_value();

        // bool BO_4 = boConstant & 0x1;
        bool BO_3 = boConstant & 0x2;
        bool BO_2 = boConstant & 0x4;
        bool BO_1 = boConstant & 0x8;
        bool BO_0 = boConstant & 0x10;

        if (!BO_2) {
          policy.writeSPR(powerpc_spr_ctr,
                          policy.add(policy.readSPR(powerpc_spr_ctr),
                                     number<word_size>(-1)));
        }

        Word(1) CTR_ok =
            BO_2 ? policy.true_()
                 : BO_3 ? policy.equalToZero(policy.readSPR(powerpc_spr_ctr))
                        : policy.invert(policy.equalToZero(
                              policy.readSPR(powerpc_spr_ctr)));

        SgAsmPowerpcRegisterReferenceExpression* BI =
            isSgAsmPowerpcRegisterReferenceExpression(operands[1]);
        ROSE_ASSERT(BI != NULL);
        ROSE_ASSERT(BI->get_register_class() == powerpc_regclass_cr);
        ROSE_ASSERT(BI->get_conditionRegisterGranularity() ==
                    powerpc_condreggranularity_bit);

        // This needs a collection of helpfer functions!
        int bi_value = BI->get_register_number();
        Word(4) CR_field = policy.readCRField(bi_value / 4);
        Word(1) CR_bi = extract<0, 1>(
            policy.shiftRight(CR_field, number<2>(3 - bi_value % 4)));
        Word(1) COND_ok =
            BO_0 ? policy.true_() : BO_1 ? CR_bi : policy.invert(CR_bi);
        policy.writeIP(policy.ite(policy.and_(CTR_ok, COND_ok),
                                  policy.and_(policy.readSPR(powerpc_spr_lr),
                                              number<word_size>(0xFFFFFFFC)),
                                  policy.readIP()));
        break;
      }

      case powerpc_cmpi: {
        ROSE_ASSERT(operands.size() == 4);
        // For 32-bit case we can ignore value of L

        Word(word_size) RA = read_word_size(operands[2]);
        Word(word_size) SI = signExtend<16, word_size>(
            extract<0, 16>(read_word_size(operands[3])));

        Word(word_size) carries = number<word_size>(0);

        // Need to check if policy.false_() or policy.true_() should be used!
        // Bias both sides and use unsigned compare.
        // policy.invert(policy.xor_(RA,number<word_size>(0x80000000U))) yields
        // "(RA+bias)-1"
        // Check if UI + (-RA) - 1 >= 0, test for UI > RA
        policy.addWithCarries(
            policy.invert(policy.xor_(RA, number<word_size>(0x80000000U))),
            policy.xor_(SI, number<word_size>(0x80000000U)), policy.false_(),
            carries);

        Word(3) c =
            policy.ite(policy.equalToZero(policy.xor_(RA, SI)), number<3>(1),
                       policy.ite(extract<word_size - 1, word_size>(carries),
                                  number<3>(4), number<3>(2)));

        SgAsmPowerpcRegisterReferenceExpression* bf =
            isSgAsmPowerpcRegisterReferenceExpression(operands[0]);
        ROSE_ASSERT(bf != NULL);
        ROSE_ASSERT(bf->get_register_class() == powerpc_regclass_cr);
        ROSE_ASSERT(bf->get_conditionRegisterGranularity() ==
                    powerpc_condreggranularity_field);

        // This should be a helper function!
        Word(1) SO =
            extract<word_size - 1, word_size>(policy.readSPR(powerpc_spr_xer));

        policy.writeCRField(bf->get_register_number(), policy.concat(SO, c));
        break;
      }
      // This is an actual 32/64 difference: low multiply fills the register on
      // 64, and high multiply fills the high bits.
      // Always write word size, but high multiply may be shifting left 32
      // first.
      case powerpc_mulhwu: {
        ROSE_ASSERT(operands.size() == 3);
        write_word_size(
            operands[0],
            policy.concat(extract<32, 64>(policy.unsignedMultiply(
                              read32(operands[1]), read32(operands[2]))),
                          number<word_size - 32>(0)));
        break;
      }

      case powerpc_mulhw: {
        ROSE_ASSERT(operands.size() == 3);
        write_word_size(
            operands[0],
            policy.concat(extract<32, 64>(policy.signedMultiply(
                              read32(operands[1]), read32(operands[2]))),
                          number<word_size - 32>(0)));
        break;
      }

      case powerpc_mullw: {
        ROSE_ASSERT(operands.size() == 3);
        write_word_size(operands[0],
                        extract<0, word_size>(policy.signedMultiply(
                            read32(operands[1]), read32(operands[2]))));
        break;
      }

      case powerpc_mulli: {
        ROSE_ASSERT(operands.size() == 3);
        write_word_size(operands[0],
                        extract<0, word_size>(policy.signedMultiply(
                            read32(operands[1]), read32(operands[2]))));
        break;
      }

      case powerpc_divw: {
        ROSE_ASSERT(operands.size() == 3);
        write_word_size(operands[0],
                        policy.signedDivide(read_word_size(operands[1]),
                                            read_word_size(operands[2])));
        break;
      }

      case powerpc_divwu: {
        ROSE_ASSERT(operands.size() == 3);
        write_word_size(operands[0],
                        policy.unsignedDivide(read_word_size(operands[1]),
                                              read_word_size(operands[2])));
        break;
      }

      case powerpc_cmp: {
        ROSE_ASSERT(operands.size() == 4);
        // For 32-bit case we can ignore value of L

        Word(word_size) RA = read_word_size(operands[2]);
        Word(word_size) RB = read_word_size(operands[3]);

        Word(word_size) carries = number<word_size>(0);

        // Need to check if policy.false_() or policy.true_() should be used!
        // Bias both sides and use unsigned compare.
        // policy.invert(policy.xor_(RA,number<word_size>(0x80000000U))) yields
        // "(RA+bias)-1"
        // Check if UI + (-RA) - 1 >= 0, test for UI > RA
        policy.addWithCarries(
            policy.invert(policy.xor_(RA, number<word_size>(0x80000000U))),
            policy.xor_(RB, number<word_size>(0x80000000U)), policy.false_(),
            carries);

        Word(3) c =
            policy.ite(policy.equalToZero(policy.xor_(RA, RB)), number<3>(1),
                       policy.ite(extract<word_size - 1, word_size>(carries),
                                  number<3>(4), number<3>(2)));

        SgAsmPowerpcRegisterReferenceExpression* bf =
            isSgAsmPowerpcRegisterReferenceExpression(operands[0]);
        ROSE_ASSERT(bf != NULL);
        ROSE_ASSERT(bf->get_register_class() == powerpc_regclass_cr);
        ROSE_ASSERT(bf->get_conditionRegisterGranularity() ==
                    powerpc_condreggranularity_field);

        // This should be a helper function!
        Word(1) SO =
            extract<word_size - 1, word_size>(policy.readSPR(powerpc_spr_xer));

        policy.writeCRField(bf->get_register_number(), policy.concat(SO, c));
        break;
      }

      case powerpc_addze: {
        ROSE_ASSERT(operands.size() == 2);

        // This should be a helper function to read CA (and other flags)
        Word(1) carry_in = extract<29, 30>(policy.readSPR(powerpc_spr_xer));

        Word(word_size) carries = number<word_size>(0);
        Word(word_size) result =
            policy.addWithCarries(read_word_size(operands[1]),
                                  number<word_size>(0x0), carry_in, carries);

        // Policy class bit numbering is opposite ordering from powerpc (based
        // on x86).
        Word(1) carry_out = extract<word_size - 1, word_size>(carries);
        write_word_size(operands[0], result);

        // This should be a helper function to read/write CA (and other flags)
        // The value 0xDFFFFFFFU is the mask for the Carry (CA) flag
        policy.writeSPR(
            powerpc_spr_xer,
            policy.or_(policy.and_(policy.readSPR(powerpc_spr_xer),
                                   number<word_size>(0xDFFFFFFFU)),
                       policy.ite(carry_out, number<word_size>(0x20000000U),
                                  number<word_size>(0x0))));
        break;
      }

      case powerpc_addme: {
        ROSE_ASSERT(operands.size() == 2);

        // This should be a helper function to read CA (and other flags)
        Word(1) carry_in = extract<29, 30>(policy.readSPR(powerpc_spr_xer));

        Word(word_size) carries = number<word_size>(0);
        Word(word_size) result = policy.addWithCarries(
            read_word_size(operands[1]), number<word_size>(0xFFFFFFFFU),
            carry_in, carries);

        // Policy class bit numbering is opposite ordering from powerpc (based
        // on x86).
        Word(1) carry_out = extract<word_size - 1, word_size>(carries);
        write_word_size(operands[0], result);

        // This should be a helper function to read/write CA (and other flags)
        // The value 0xDFFFFFFFU is the mask for the Carry (CA) flag
        policy.writeSPR(
            powerpc_spr_xer,
            policy.or_(policy.and_(policy.readSPR(powerpc_spr_xer),
                                   number<word_size>(0xDFFFFFFFU)),
                       policy.ite(carry_out, number<word_size>(0x20000000U),
                                  number<word_size>(0x0))));
        break;
      }

      case powerpc_adde: {
        ROSE_ASSERT(operands.size() == 3);

        // This should be a helper function to read CA (and other flags)
        Word(1) carry_in = extract<29, 30>(policy.readSPR(powerpc_spr_xer));

        Word(word_size) carries = number<word_size>(0);
        Word(word_size) result = policy.addWithCarries(
            read_word_size(operands[1]), read_word_size(operands[2]), carry_in,
            carries);

        // Policy class bit numbering is opposite ordering from powerpc (based
        // on x86).
        Word(1) carry_out = extract<word_size - 1, word_size>(carries);
        write_word_size(operands[0], result);

        // This should be a helper function to read/write CA (and other flags)
        // The value 0xDFFFFFFFU is the mask for the Carry (CA) flag
        policy.writeSPR(
            powerpc_spr_xer,
            policy.or_(policy.and_(policy.readSPR(powerpc_spr_xer),
                                   number<word_size>(0xDFFFFFFFU)),
                       policy.ite(carry_out, number<word_size>(0x20000000U),
                                  number<word_size>(0x0))));
        break;
      }

      case powerpc_andi_record: {
        ROSE_ASSERT(operands.size() == 3);

        Word(word_size) result = policy.and_(read_word_size(operands[1]),
                                             read_word_size(operands[2]));
        write_word_size(operands[0], result);

        record(result);
        break;
      }

      case powerpc_andis_record: {
        ROSE_ASSERT(operands.size() == 3);

        Word(word_size) result = policy.and_(
            read_word_size(operands[1]),
            policy.concat(number<word_size - 16>(0),
                          extract<0, 16>(read_word_size(operands[2]))));
        write_word_size(operands[0], result);

        record(result);
        break;
      }

      case powerpc_neg: {
        ROSE_ASSERT(operands.size() == 2);
        write_word_size(operands[0],
                        policy.negate(read_word_size(operands[1])));
        break;
      }

      case powerpc_srawi: {
        ROSE_ASSERT(operands.size() == 3);

        Word(word_size) RS = read_word_size(operands[1]);

        // An alternative might be: uint8_t sh_value =
        // read_word_size(operands[1]);
        Word(5) SH = extract<0, 5>(read_word_size(operands[2]));

        Word(1) negative = extract<word_size - 1, word_size>(RS);
        Word(word_size) mask =
            policy.invert(policy.shiftLeft(number<word_size>(-1), SH));
        Word(1) hasValidBits =
            policy.invert(policy.equalToZero(policy.and_(RS, mask)));
        Word(1) carry_out = policy.and_(hasValidBits, negative);

        write_word_size(operands[0], policy.shiftRightArithmetic(RS, SH));
        policy.writeSPR(
            powerpc_spr_xer,
            policy.or_(policy.and_(policy.readSPR(powerpc_spr_xer),
                                   number<word_size>(0xDFFFFFFFU)),
                       policy.ite(carry_out, number<word_size>(0x20000000U),
                                  number<word_size>(0x0))));
        break;
      }

      case powerpc_bcctr: {
        ROSE_ASSERT(operands.size() == 3);
        SgAsmByteValueExpression* byteValue =
            isSgAsmByteValueExpression(operands[0]);
        ROSE_ASSERT(byteValue != NULL);
        uint8_t boConstant = byteValue->get_value();

        bool BO_1 = boConstant & 0x8;
        bool BO_0 = boConstant & 0x10;

        SgAsmPowerpcRegisterReferenceExpression* BI =
            isSgAsmPowerpcRegisterReferenceExpression(operands[1]);
        ROSE_ASSERT(BI != NULL);
        ROSE_ASSERT(BI->get_register_class() == powerpc_regclass_cr);
        ROSE_ASSERT(BI->get_conditionRegisterGranularity() ==
                    powerpc_condreggranularity_bit);

        // This needs a collection of helpfer functions!
        int bi_value = BI->get_register_number();
        Word(4) CR_field = policy.readCRField(bi_value / 4);
        Word(1) CR_bi = extract<0, 1>(
            policy.shiftRight(CR_field, number<2>(3 - bi_value % 4)));
        Word(1) COND_ok =
            BO_0 ? policy.true_() : BO_1 ? CR_bi : policy.invert(CR_bi);

        policy.writeIP(policy.ite(COND_ok,
                                  policy.and_(policy.readSPR(powerpc_spr_ctr),
                                              number<word_size>(0xFFFFFFFC)),
                                  policy.readIP()));

        break;
      }
      case powerpc_bcctrl: {
        ROSE_ASSERT(operands.size() == 3);
        SgAsmByteValueExpression* byteValue =
            isSgAsmByteValueExpression(operands[0]);
        ROSE_ASSERT(byteValue != NULL);
        uint8_t boConstant = byteValue->get_value();

        bool BO_1 = boConstant & 0x8;
        bool BO_0 = boConstant & 0x10;

        SgAsmPowerpcRegisterReferenceExpression* BI =
            isSgAsmPowerpcRegisterReferenceExpression(operands[1]);
        ROSE_ASSERT(BI != NULL);
        ROSE_ASSERT(BI->get_register_class() == powerpc_regclass_cr);
        ROSE_ASSERT(BI->get_conditionRegisterGranularity() ==
                    powerpc_condreggranularity_bit);

        // This needs a collection of helpfer functions!
        int bi_value = BI->get_register_number();
        Word(4) CR_field = policy.readCRField(bi_value / 4);
        Word(1) CR_bi = extract<0, 1>(
            policy.shiftRight(CR_field, number<2>(3 - bi_value % 4)));
        Word(1) COND_ok =
            BO_0 ? policy.true_() : BO_1 ? CR_bi : policy.invert(CR_bi);

        // Write the incremented IP value to the link register so that function
        // can return.
        policy.writeSPR(powerpc_spr_lr, policy.readIP());

        policy.writeIP(policy.ite(COND_ok,
                                  policy.and_(policy.readSPR(powerpc_spr_ctr),
                                              number<word_size>(0xFFFFFFFC)),
                                  policy.readIP()));

        break;
      }

      case powerpc_sc: {
        ROSE_ASSERT(operands.size() == 1);
        SgAsmByteValueExpression* bv = isSgAsmByteValueExpression(operands[0]);
        ROSE_ASSERT(bv);
        policy.systemCall(bv->get_value());
        break;
      }

      case powerpc_stmw: {
        ROSE_ASSERT(operands.size() == 2);

        Word(word_size) effectiveAddress = readEffectiveAddress(operands[1]);

        SgAsmPowerpcRegisterReferenceExpression* RS =
            isSgAsmPowerpcRegisterReferenceExpression(operands[0]);
        ROSE_ASSERT(RS != NULL);
        ROSE_ASSERT(RS->get_register_class() == powerpc_regclass_gpr);

        uint8_t r = RS->get_register_number();
        uint32_t offset = 0;

        while (r <= 31) {
          writeMemory<word_size>(
              policy.add(effectiveAddress, number<word_size>(offset)),
              policy.readGPR(r), policy.true_());
          offset += 4;
          ++r;
        }
        break;
      }

      case powerpc_lmw: {
        ROSE_ASSERT(operands.size() == 2);

        Word(word_size) effectiveAddress = readEffectiveAddress(operands[1]);

        SgAsmPowerpcRegisterReferenceExpression* RT =
            isSgAsmPowerpcRegisterReferenceExpression(operands[0]);
        ROSE_ASSERT(RT != NULL);
        ROSE_ASSERT(RT->get_register_class() == powerpc_regclass_gpr);

        uint8_t r = RT->get_register_number();
        uint32_t offset = 0;

        while (r <= 31) {
          policy.writeGPR(
              r, readMemory<word_size>(
                     policy.add(effectiveAddress, number<word_size>(offset)),
                     policy.true_()));
          offset += 4;
          ++r;
        }
        break;
      }

      case powerpc_cntlzw: {
        ROSE_ASSERT(operands.size() == 2);
        Word(word_size) RS = read_word_size(operands[1]);

        // Using xor to do the subtract from 31
        Word(word_size) result =
            policy.ite(policy.equalToZero(RS), number<word_size>(32),
                       policy.xor_(policy.mostSignificantSetBit(RS),
                                   number<word_size>(31)));

        write_word_size(operands[0], result);
        break;
      }

      case powerpc_mfcr: {
        ROSE_ASSERT(operands.size() == 1);
        write32(operands[0], policy.readCR());
        break;
      }

      case powerpc_slw: {
        ROSE_ASSERT(operands.size() == 3);
        Word(6) shiftCount = extract<0, 6>(read_word_size(operands[2]));
        write_word_size(
            operands[0],
            policy.ite(extract<5, 6>(shiftCount), number<word_size>(0),
                       policy.shiftLeft(read_word_size(operands[1]),
                                        extract<0, 5>(shiftCount))));
        break;
      }

      case powerpc_srw: {
        ROSE_ASSERT(operands.size() == 3);
        Word(6) shiftCount = extract<0, 6>(read_word_size(operands[2]));
        write_word_size(
            operands[0],
            policy.ite(extract<5, 6>(shiftCount), number<word_size>(0),
                       policy.shiftRight(read_word_size(operands[1]),
                                         extract<0, 5>(shiftCount))));
        break;
      }

      default: {
        policy.undefinedInstruction(insn);
        break;
      }
    }
  }

  void processInstruction(SgAsmPowerpcInstruction* insn) {
    ROSE_ASSERT(insn);
    policy.startInstruction(insn);
    translate(insn);
    policy.finishInstruction(insn);
  }
};

#undef Word

#endif  // ROSE_POWERPCINSTRUCTIONSEMANTICS_H
