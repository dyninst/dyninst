#ifndef ROSE_DispatcherRISCV64_H
#define ROSE_DispatcherRISCV64_H

#include "../SgAsmRiscv64Instruction.h"
#include "BaseSemantics2.h"
#include "external/rose/riscv64InstructionEnum.h"

// RISC-V is not supported by ROSE
// Instead, we're using SAIL (https://github.com/riscv/sail-riscv) for
// instruction semantics. For consistency, the class and function names are all
// ROSE-compatible, generated from the sail-to-rose pipeline

namespace rose {
namespace BinaryAnalysis {
namespace InstructionSemantics2 {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Dispatcher
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Shared-ownership pointer to an RISC-V instruction dispatcher. See @ref
 * heap_object_shared_ownership. */
typedef boost::shared_ptr<class DispatcherRiscv64> DispatcherRiscv64Ptr;

class DispatcherRiscv64 : public BaseSemantics::Dispatcher {
protected:
  // Prototypical constructor
  DispatcherRiscv64()
      : BaseSemantics::Dispatcher(64,
                                  RegisterDictionary::dictionary_riscv64()) {}

  // Prototypical constructor
  DispatcherRiscv64(size_t addrWidth, const RegisterDictionary *regs /*=NULL*/)
      : BaseSemantics::Dispatcher(
            addrWidth, regs ? regs : RegisterDictionary::dictionary_riscv64()) {
  }

  // Normal constructor
  DispatcherRiscv64(const BaseSemantics::RiscOperatorsPtr &ops,
                    size_t addrWidth, const RegisterDictionary *regs)
      : BaseSemantics::Dispatcher(
            ops, addrWidth,
            regs ? regs : RegisterDictionary::dictionary_riscv64()) {
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
   * calls to Dispatcher::findRegister(). The register descriptor is updated
   * only when the register dictionary is changed (see
   * set_register_dictionary()).
   *
   *  Register names like REG_anyAX have sizes that depend on the architecture:
   * 16 bits for 16-bit architectures, 32 bits for 32-bit architectures, etc.
   * The other register names have specific sizes--such as REG_EAX being 32
   * bits--and are defined only on architectures that support them. */

  RegisterDescriptor REG_PC, REG_RA, REG_SP;

  /** Construct a prototypical dispatcher.  The only thing this dispatcher can
   * be used for is to create another dispatcher with the virtual @ref create
   * method. */
  static DispatcherRiscv64Ptr instance() {
    return DispatcherRiscv64Ptr(new DispatcherRiscv64);
  }

  /** Construct a prototyipcal dispatcher. Construct a prototypical dispatcher
   * with a specified address size. The only thing this dispatcher can be used
   * for is to create another dispatcher with the virtual @ref create method. */
  static DispatcherRiscv64Ptr instance(size_t addrWidth,
                                       const RegisterDictionary *regs = NULL) {
    return DispatcherRiscv64Ptr(new DispatcherRiscv64(addrWidth, regs));
  }

  /** Constructor. */
  static DispatcherRiscv64Ptr
  instance(const BaseSemantics::RiscOperatorsPtr &ops, size_t addrWidth,
           const RegisterDictionary *regs = NULL) {
    return DispatcherRiscv64Ptr(new DispatcherRiscv64(ops, addrWidth, regs));
  }

  /** Virtual constructor. */
  virtual BaseSemantics::DispatcherPtr
  create(const BaseSemantics::RiscOperatorsPtr &ops, size_t addrWidth = 0,
         const RegisterDictionary *regs = NULL) const {
    if (0 == addrWidth) {
      addrWidth = Dyninst::getArchAddressWidth(Dyninst::Arch_riscv64);
    }
    return instance(ops, addrWidth, regs);
  }

  /** Dynamic cast to a DispatcherRiscv64Ptr with assertion. */
  static DispatcherRiscv64Ptr promote(const BaseSemantics::DispatcherPtr &d) {
    DispatcherRiscv64Ptr retval =
        boost::dynamic_pointer_cast<DispatcherRiscv64>(d);
    assert(retval != NULL);
    return retval;
  }

  virtual void set_register_dictionary(const RegisterDictionary *regdict);

  /** Get list of common registers. Returns a list of non-overlapping registers
   * composed of the largest registers except using individual flags for the
   * fields of the FLAGS/EFLAGS register. */
  virtual RegisterDictionary::RegisterDescriptors get_usual_registers() const;

  virtual RegisterDescriptor instructionPointerRegister() const;

  virtual RegisterDescriptor stackPointerRegister() const;

  virtual BaseSemantics::SValuePtr effectiveAddress(SgAsmExpression *,
                                                    size_t nbits = 0);

  virtual int iproc_key(SgAsmInstruction *insn_) const {
    SgAsmRiscv64Instruction *insn = isSgAsmRiscv64Instruction(insn_);
    assert(insn != NULL);
    return insn->get_kind();
  }

  virtual BaseSemantics::SValuePtr
  read(SgAsmExpression *e, size_t value_nbits /*=0*/, size_t addr_nbits /*=0*/);

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
   * architecture-specific things. For instance, writing to a 32-bit GPR such as
   * "eax" on x86-64 will write zeros to the upper half of "rax". */
  virtual void writeRegister(const RegisterDescriptor &,
                             const BaseSemantics::SValuePtr &result);

  /** Extend or truncate value to propert memory address width. */
  virtual BaseSemantics::SValuePtr
  fixMemoryAddress(const BaseSemantics::SValuePtr &address) const;

  /** Checks if the supplied value is or isn't equal to zero */
  virtual BaseSemantics::SValuePtr
  isZero(const BaseSemantics::SValuePtr &value);

  /** Execute a branch -- equivalent to writing the target address value to the
   * PC */
  virtual void BranchTo(const BaseSemantics::SValuePtr &target);

  /** Returns the input value sign extended to the provided length. */
  virtual BaseSemantics::SValuePtr
  SignExtend(const BaseSemantics::SValuePtr &expr, size_t newsize);

  /** Returns the input value zero extended to the provided length. */
  virtual BaseSemantics::SValuePtr
  ZeroExtend(const BaseSemantics::SValuePtr &expr, size_t newsize);

  /** Replicates the value contained in expr to fill the full 64-bit width. */
  virtual BaseSemantics::SValuePtr
  Replicate(const BaseSemantics::SValuePtr &expr);

  /** Reads memory of size readSize bits from address addr. */
  BaseSemantics::SValuePtr readMemory(const BaseSemantics::SValuePtr &addr,
                                      size_t readSize);

  /** Writes value data of size writeSize bits to memory at address addr. */
  void writeMemory(const BaseSemantics::SValuePtr &addr, size_t writeSize,
                   const BaseSemantics::SValuePtr &data);

  /** Returns the register expression containing the target address for a
   * write-back in case of memory-access instructions. */
  SgAsmExpression *getWriteBackTarget(SgAsmExpression *expr);

  /** Applies a shift operation of type shiftType (defined in enum ShiftType)
   * and shift length of amount to src. */
  BaseSemantics::SValuePtr ShiftReg(const BaseSemantics::SValuePtr &src,
                                    int shiftType,
                                    const BaseSemantics::SValuePtr &amount);

  /** Returns an expression representing the number of leading 0s in expr. */
  BaseSemantics::SValuePtr
  CountLeadingZeroBits(const BaseSemantics::SValuePtr &expr);

  /** Returns an expression representing the number of leading contiguous bits
   * that match the sign bit in expr. */
  BaseSemantics::SValuePtr
  CountLeadingSignBits(const BaseSemantics::SValuePtr &expr);

  /** Rounds a number to zero (upwards if it is negative, downwards if it is
   * positive. */
  BaseSemantics::SValuePtr
  RoundTowardsZero(const BaseSemantics::SValuePtr &expr);

  /** Returns an expression representing the number of trailing 0s in expr. */
  BaseSemantics::SValuePtr
  CountTrailingSignBits(const BaseSemantics::SValuePtr &expr);

  /** Returns carryless multiplication (multiplication in GF2) */
  BaseSemantics::SValuePtr CarrylessMultiply(const BaseSemantics::SValuePtr &a,
                                             const BaseSemantics::SValuePtr &b);

  /** Returns carryless multiplication reverse (find the polynomial
   * reciprocal,gR in GF2) */
  BaseSemantics::SValuePtr
  CarrylessMultiplyReverse(const BaseSemantics::SValuePtr &a,
                           const BaseSemantics::SValuePtr &b);

  /** Count the number of 1s in expr */
  BaseSemantics::SValuePtr PopCount(const BaseSemantics::SValuePtr &expr);

  /** Reverses the order of bits within each individual byte, but maintains the
   * order of the bytes themselves. */
  BaseSemantics::SValuePtr Brev8(const BaseSemantics::SValuePtr &expr);

  /** Reverses the order of the bytes in expr */
  BaseSemantics::SValuePtr Rev8(const BaseSemantics::SValuePtr &expr);

  /** Returns the value of log2(expr) */
  BaseSemantics::SValuePtr Log2(const BaseSemantics::SValuePtr &expr);

  /** Returns signed unsigned multiplication of a and b */
  BaseSemantics::SValuePtr
  signedUnsignedMultiply(const BaseSemantics::SValuePtr &a,
                         const BaseSemantics::SValuePtr &b);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Instruction processors
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Riscv64 {

/** Base class for all Riscv64 instruction processors.
 *
 *  This class provides single-letter names for some types that are used in all
 * instructions: D, I, A, and Ops for the dispatcher raw pointer, instruction
 * pointer, argument list pointer, and RISC operators raw pointer.  It also
 * takes care of advancing the instruction pointer prior to handing the
 * instruction to the subclass, which by the way is done via
 *  @ref p method (short for "process").  See examples in DispatcherX86.C --
 * there are <em>lots</em> of them. */
class InsnProcessor : public BaseSemantics::InsnProcessor {
public:
  typedef DispatcherRiscv64 *D;
  typedef BaseSemantics::RiscOperators *Ops;
  typedef SgAsmRiscv64Instruction *I;
  typedef const SgAsmExpressionPtrList &A;
  typedef uint32_t B;

  virtual void p(D, Ops, I, A, B) = 0;

  virtual void process(const BaseSemantics::DispatcherPtr &,
                       SgAsmInstruction *);

  virtual void assert_args(I insn, A args, size_t nargs);

public:
  enum MemOp { MemOp_STORE, MemOp_LOAD };

  enum MoveWideOp { MoveWideOp_N, MoveWideOp_Z, MoveWideOp_K };

  enum LogicalOp { LogicalOp_AND, LogicalOp_ORR, LogicalOp_EOR };

  enum ShiftType { ShiftType_LSL, ShiftType_LSR, ShiftType_ASR, ShiftType_ROR };

  enum CountOp { CountOp_CLZ, CountOp_CLS, CountOp_CNT };
};

} // namespace Riscv64

} // namespace InstructionSemantics2
} // namespace BinaryAnalysis
} // namespace rose

#endif
