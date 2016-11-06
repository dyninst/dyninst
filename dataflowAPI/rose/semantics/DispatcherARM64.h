#ifndef ROSE_DispatcherARM64_H
#define ROSE_DispatcherARM64_H

#include "BaseSemantics2.h"
#include "../SgAsmArmv8Instruction.h"
#include "external/rose/armv8InstructionEnum.h"

namespace rose {
namespace BinaryAnalysis {
namespace InstructionSemantics2 {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Dispatcher
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Shared-ownership pointer to an ARM instruction dispatcher. See @ref
 * heap_object_shared_ownership. */
typedef boost::shared_ptr<class DispatcherARM64> DispatcherARM64Ptr;

class DispatcherARM64 : public BaseSemantics::Dispatcher {
 protected:
  // Prototypical constructor
  DispatcherARM64()
      : BaseSemantics::Dispatcher(64, RegisterDictionary::dictionary_armv8()) {}

  // Prototypical constructor
  DispatcherARM64(size_t addrWidth, const RegisterDictionary *regs /*=NULL*/)
      : BaseSemantics::Dispatcher(
            addrWidth, regs ? regs : RegisterDictionary::dictionary_armv8()) {}

  // Normal constructor
  DispatcherARM64(const BaseSemantics::RiscOperatorsPtr &ops, size_t addrWidth,
                  const RegisterDictionary *regs)
      : BaseSemantics::Dispatcher(
            ops, addrWidth,
            regs ? regs : RegisterDictionary::dictionary_armv8()) {
    regcache_init();
    iproc_init();
    memory_init();
  }

 public:
  /** Loads the iproc table with instruction processing functors. This normally
   * happens from the constructor. */
  void iproc_init();

  /** Load the cached register descriptors.  This happens at construction and on
   * set_register_dictionary() calls. */
  void regcache_init();

  /** Make sure memory is set up correctly. For instance, byte order should be
   * little endian. */
  void memory_init();

 public:
  /** Cached register. This register is cached so that there are not so many
   * calls to Dispatcher::findRegister(). The
   *  register descriptor is updated only when the register dictionary is
   * changed (see set_register_dictionary()).
   *
   *  Register names like REG_anyAX have sizes that depend on the architecture:
   * 16 bits for 16-bit architectures, 32 bits for
   *  32-bit architectures, etc.  The other register names have specific
   * sizes--such as REG_EAX being 32 bits--and are
   *  defined only on architectures that support them.
   *
   * @{ */

  RegisterDescriptor REG_PC, REG_NZCV, REG_SP;

  /** @}*/

  /** Construct a prototypical dispatcher.  The only thing this dispatcher can
   * be used for is to create another dispatcher
   *  with the virtual @ref create method. */
  static DispatcherARM64Ptr instance() {
    return DispatcherARM64Ptr(new DispatcherARM64);
  }

  /** Construct a prototyipcal dispatcher. Construct a prototypical dispatcher
   * with a specified address size. The only thing
   * this dispatcher can be used for is to create another dispatcher with the
   * virtual @ref create method. */
  static DispatcherARM64Ptr instance(size_t addrWidth,
                                     const RegisterDictionary *regs = NULL) {
    return DispatcherARM64Ptr(new DispatcherARM64(addrWidth, regs));
  }

  /** Constructor. */
  static DispatcherARM64Ptr instance(const BaseSemantics::RiscOperatorsPtr &ops,
                                     size_t addrWidth,
                                     const RegisterDictionary *regs = NULL) {
    return DispatcherARM64Ptr(new DispatcherARM64(ops, addrWidth, regs));
  }

  /** Virtual constructor. */
  virtual BaseSemantics::DispatcherPtr create(
      const BaseSemantics::RiscOperatorsPtr &ops, size_t addrWidth = 0,
      const RegisterDictionary *regs = NULL) const {
    if (0 == addrWidth) {
      // TODO
      // addressWidth(64);
      addrWidth = 64;  // addressWidth();
    }

    return instance(ops, addrWidth, regs);
  }

  /** Dynamic cast to a DispatcherARM64Ptr with assertion. */
  static DispatcherARM64Ptr promote(const BaseSemantics::DispatcherPtr &d) {
    DispatcherARM64Ptr retval = boost::dynamic_pointer_cast<DispatcherARM64>(d);
    assert(retval != NULL);
    return retval;
  }

  virtual void set_register_dictionary(const RegisterDictionary *regdict);

  /** Get list of common registers. Returns a list of non-overlapping registers
   * composed of the largest registers except
   *  using individual flags for the fields of the FLAGS/EFLAGS register. */
  virtual RegisterDictionary::RegisterDescriptors get_usual_registers() const;

  virtual RegisterDescriptor instructionPointerRegister() const;

  virtual RegisterDescriptor stackPointerRegister() const;

  virtual int iproc_key(SgAsmInstruction *insn_) const {
    SgAsmArmv8Instruction *insn = isSgAsmArmv8Instruction(insn_);
    assert(insn != NULL);
    return insn->get_kind();
  }

  virtual void write(SgAsmExpression *e, const BaseSemantics::SValuePtr &value,
                     size_t addr_nbits = 0);

  /** Architecture-specific read from register.
   *
   *  Similar to RiscOperators::readRegister, but might do additional
   * architecture-specific things. */
  virtual BaseSemantics::SValuePtr readRegister(const RegisterDescriptor &);

  /** Architecture-specific write to register.
   *
   *  Similar to RiscOperators::writeRegister, but might do additional
   * architecture-specific things. For instance, writing to
   *  a 32-bit GPR such as "eax" on x86-64 will write zeros to the upper half of
   * "rax". */
  virtual void writeRegister(const RegisterDescriptor &,
                             const BaseSemantics::SValuePtr &result);

  /** Set parity, sign, and zero flags appropriate for result value. */
  virtual void setFlagsForResult(const BaseSemantics::SValuePtr &result,
                                 const BaseSemantics::SValuePtr &carries,
                                 bool invertCarries, size_t nbits,
                                 BaseSemantics::SValuePtr &nzcv);

  /** Returns true if byte @p v has an even number of bits set; false for an odd
   * number */
  virtual BaseSemantics::SValuePtr parity(const BaseSemantics::SValuePtr &v);

  /** Conditionally invert the bits of @p value.  The bits are inverted if @p
   * maybe is true, otherwise @p value is returned. */
  virtual BaseSemantics::SValuePtr invertMaybe(
      const BaseSemantics::SValuePtr &value, bool maybe);

  /** Adds two values and adjusts flags.  This method can be used for
   * subtraction if @p b is two's complement and @p
   *  invertCarries is set.  If @p cond is supplied, then the addition and flag
   * adjustments are conditional.
   * @{ */
  virtual BaseSemantics::SValuePtr doAddOperation(
      BaseSemantics::SValuePtr a, BaseSemantics::SValuePtr b,
      bool invertCarries, const BaseSemantics::SValuePtr &carryIn,
      BaseSemantics::SValuePtr &nzcv);

  // FIXME
  /** Implements the RCL, RCR, ROL, and ROR instructions for various operand
   * sizes.  The rotate amount is always 8 bits wide
   * in the instruction, but the semantics mask off all but the low-order bits,
   * keeping 5 bits in 32-bit mode and 6 bits in
   * 64-bit mode (indicated by the rotateSignificantBits argument). */
  /*virtual BaseSemantics::SValuePtr doRotateOperation(ARMv8InstructionKind
     kind,
                                                     const
     BaseSemantics::SValuePtr &operand,
                                                     const
     BaseSemantics::SValuePtr &total_rotate,
                                                     size_t
     rotateSignificantBits);*/

  // FIXME
  /** Implements the SHR, SAR, SHL, SAL, SHRD, and SHLD instructions for various
   * operand sizes.  The shift amount is always 8
   *  bits wide in the instruction, but the semantics mask off all but the
   * low-order bits, keeping 5 bits in 32-bit mode and
   *  7 bits in 64-bit mode (indicated by the @p shiftSignificantBits argument).
   * The semantics of SHL and SAL are
   *  identical (in fact, ROSE doesn't even define x86_sal). The @p source_bits
   * argument contains the bits to be shifted into
   *  the result and is used only for SHRD and SHLD instructions. */
  /*virtual BaseSemantics::SValuePtr doShiftOperation(ARMv8InstructionKind kind,
                                                    const
     BaseSemantics::SValuePtr &operand,
                                                    const
     BaseSemantics::SValuePtr &source_bits,
                                                    const
     BaseSemantics::SValuePtr &total_shift,
                                                    size_t
     shiftSignificantBits);*/

  /** Extend or truncate value to propert memory address width. */
  virtual BaseSemantics::SValuePtr fixMemoryAddress(
      const BaseSemantics::SValuePtr &address) const;

  /** Checks if the supplied value is or isn't equal to zero */
  virtual BaseSemantics::SValuePtr isZero(
      const BaseSemantics::SValuePtr &value);

  virtual BaseSemantics::SValuePtr ConditionHolds(
      const BaseSemantics::SValuePtr &cond);

  /** Inverts the passed in expression and returns the result */
  virtual BaseSemantics::SValuePtr NOT(const BaseSemantics::SValuePtr &expr);

  /** Execute a branch -- equivalent to writing the target address value to the
   * PC */
  virtual void BranchTo(const BaseSemantics::SValuePtr &target);

  /** Returns a value that equals 0. nbits specifies what should be the
   * bit-length of the value,
   * but is irrelevant in practice as a 64-bit zero is returned anyway. */
  virtual BaseSemantics::SValuePtr Zeros(const unsigned int nbits);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Instruction processors
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ARM64 {

/** Base class for all x86 instruction processors.
 *
 *  This class provides single-letter names for some types that are used in all
 * instructions: D, I, A, and Ops for the
 *  dispatcher raw pointer, instruction pointer, argument list pointer, and RISC
 * operators raw pointer.  It also takes care
 *  of advancing the instruction pointer prior to handing the instruction to the
 * subclass, which by the way is done via
 *  @ref p method (short for "process").  See examples in DispatcherX86.C --
 * there are <em>lots</em> of them. */
class InsnProcessor : public BaseSemantics::InsnProcessor {
 public:
  typedef DispatcherARM64 *D;
  typedef BaseSemantics::RiscOperators *Ops;
  typedef SgAsmArmv8Instruction *I;
  typedef const SgAsmExpressionPtrList &A;
  typedef uint32_t B;

  virtual void p(D, Ops, I, A, B) = 0;

  virtual void process(const BaseSemantics::DispatcherPtr &,
                       SgAsmInstruction *);

  virtual void assert_args(I insn, A args, size_t nargs);
  // void check_arg_width(D d, I insn, A args);
};

}  // namespace

}  // namespace
}  // namespace
}  // namespace

#endif
