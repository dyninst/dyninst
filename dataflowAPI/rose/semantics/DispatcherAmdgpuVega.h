#ifndef ROSE_DispatcherAMDGPU_VEGA_H
#define ROSE_DispatcherAMDGPU_VEGA_H

#include "BaseSemantics2.h"
#include "../SgAsmAmdgpuVegaInstruction.h"
#include "external/rose/amdgpuInstructionEnum.h"

namespace rose {
    namespace BinaryAnalysis {
        namespace InstructionSemantics2 {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Dispatcher
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Shared-ownership pointer to an AMDGPU VEGA instruction dispatcher. See @ref heap_object_shared_ownership. */
            typedef boost::shared_ptr<class DispatcherAmdgpuVega> DispatcherAmdgpuVegaPtr;

            class DispatcherAmdgpuVega : public BaseSemantics::Dispatcher {
            protected:
                // Prototypical constructor
                DispatcherAmdgpuVega()
                        : BaseSemantics::Dispatcher(64, RegisterDictionary::dictionary_amdgpu_vega()) { }

                // Prototypical constructor
                DispatcherAmdgpuVega(size_t addrWidth, const RegisterDictionary *regs/*=NULL*/)
                        : BaseSemantics::Dispatcher(addrWidth, regs ? regs : RegisterDictionary::dictionary_amdgpu_vega()) { }

                // Normal constructor
                DispatcherAmdgpuVega(const BaseSemantics::RiscOperatorsPtr &ops, size_t addrWidth,
                                const RegisterDictionary *regs)
                        : BaseSemantics::Dispatcher(ops, addrWidth,
                                                    regs ? regs : RegisterDictionary::dictionary_amdgpu_vega()) {
                    regcache_init();
                    iproc_init();
                    memory_init();
                }

            public:

                /** Loads the iproc table with instruction processing functors. This normally happens from the constructor. */
                void iproc_init();

                /** Load the cached register descriptors.  This happens at construction and on set_register_dictionary() calls. */
                void regcache_init();

                /** Make sure memory is set up correctly. For instance, byte order should be little endian. */
                void memory_init();

            public:
                /** Cached register. This register is cached so that there are not so many calls to Dispatcher::findRegister(). The
                 *  register descriptor is updated only when the register dictionary is changed (see set_register_dictionary()).
                 *
                 *  Register names like REG_anyAX have sizes that depend on the architecture: 16 bits for 16-bit architectures, 32 bits for
                 *  32-bit architectures, etc.  The other register names have specific sizes--such as REG_EAX being 32 bits--and are
                 *  defined only on architectures that support them.
                 *
                 * @{ */

                RegisterDescriptor REG_PC, REG_SCC;

                /** @}*/

                /** Construct a prototypical dispatcher.  The only thing this dispatcher can be used for is to create another dispatcher
                 *  with the virtual @ref create method. */
                static DispatcherAmdgpuVegaPtr instance() {
                    return DispatcherAmdgpuVegaPtr(new DispatcherAmdgpuVega);
                }

                /** Construct a prototyipcal dispatcher. Construct a prototypical dispatcher with a specified address size. The only thing
                 * this dispatcher can be used for is to create another dispatcher with the virtual @ref create method. */
                static DispatcherAmdgpuVegaPtr instance(size_t addrWidth, const RegisterDictionary *regs = NULL) {
                    return DispatcherAmdgpuVegaPtr(new DispatcherAmdgpuVega(addrWidth, regs));
                }

                /** Constructor. */
                static DispatcherAmdgpuVegaPtr instance(const BaseSemantics::RiscOperatorsPtr &ops, size_t addrWidth,
                                                   const RegisterDictionary *regs = NULL) {
                    return DispatcherAmdgpuVegaPtr(new DispatcherAmdgpuVega(ops, addrWidth, regs));
                }

                /** Virtual constructor. */
                virtual BaseSemantics::DispatcherPtr create(const BaseSemantics::RiscOperatorsPtr &ops,
                                                            size_t addrWidth = 0,
                                                            const RegisterDictionary *regs = NULL) const {
                    if (0 == addrWidth) {
			//TODO
                        //addressWidth(64);
                        addrWidth = 64;//addressWidth();
                    }

                    return instance(ops, addrWidth, regs);
                }

                /** Dynamic cast to a DispatcherAmdgpuVegaPtr with assertion. */
                static DispatcherAmdgpuVegaPtr promote(const BaseSemantics::DispatcherPtr &d) {
                    DispatcherAmdgpuVegaPtr retval = boost::dynamic_pointer_cast<DispatcherAmdgpuVega>(d);
                    assert(retval != NULL);
                    return retval;
                }

                virtual void set_register_dictionary(const RegisterDictionary *regdict);

                /** Get list of common registers. Returns a list of non-overlapping registers composed of the largest registers except
                 *  using individual flags for the fields of the FLAGS/EFLAGS register. */
                virtual RegisterDictionary::RegisterDescriptors get_usual_registers() const;

                virtual RegisterDescriptor instructionPointerRegister() const;

                virtual RegisterDescriptor stackPointerRegister() const;

                virtual BaseSemantics::SValuePtr effectiveAddress(SgAsmExpression *, size_t nbits = 0);

                virtual int iproc_key(SgAsmInstruction *insn_) const {
                    SgAsmAmdgpuVegaInstruction *insn = isSgAsmAmdgpuVegaInstruction(insn_);
                    assert(insn != NULL);
                    return insn->get_kind();
                }

                virtual void write(SgAsmExpression *e, const BaseSemantics::SValuePtr &value, size_t addr_nbits = 0);

                /** Architecture-specific read from register.
                 *
                 *  Similar to RiscOperators::readRegister, but might do additional architecture-specific things. */
                virtual BaseSemantics::SValuePtr readRegister(const RegisterDescriptor &);

                /** Architecture-specific write to register.
                 *
                 *  Similar to RiscOperators::writeRegister, but might do additional architecture-specific things. For instance, writing to
                 *  a 32-bit GPR such as "eax" on x86-64 will write zeros to the upper half of "rax". */
                virtual void writeRegister(const RegisterDescriptor &, const BaseSemantics::SValuePtr &result);

                /** Set parity, sign, and zero flags appropriate for result value. */
                virtual void setFlagsForResult(const BaseSemantics::SValuePtr &result,
                                               const BaseSemantics::SValuePtr &carries,
                                               bool invertCarries, size_t nbits,
                                               BaseSemantics::SValuePtr &n,
                                               BaseSemantics::SValuePtr &z,
                                               BaseSemantics::SValuePtr &c,
                                               BaseSemantics::SValuePtr &v);

                /** Returns true if byte @p v has an even number of bits set; false for an odd number */
                virtual BaseSemantics::SValuePtr parity(const BaseSemantics::SValuePtr &v);

                /** Conditionally invert the bits of @p value.  The bits are inverted if @p maybe is true, otherwise @p value is returned. */
                virtual BaseSemantics::SValuePtr invertMaybe(const BaseSemantics::SValuePtr &value, bool maybe);

                /** Adds two values and adjusts flags.  This method can be used for subtraction if @p b is two's complement and @p
                 *  invertCarries is set.  If @p cond is supplied, then the addition and flag adjustments are conditional.
                 * @{ */
                virtual BaseSemantics::SValuePtr doAddOperation(BaseSemantics::SValuePtr a, BaseSemantics::SValuePtr b,
                                                                bool invertCarries,
                                                                const BaseSemantics::SValuePtr &carryIn,
                                                                BaseSemantics::SValuePtr &n,
                                                                BaseSemantics::SValuePtr &z,
                                                                BaseSemantics::SValuePtr &c,
                                                                BaseSemantics::SValuePtr &v);

                //FIXME
                /** Implements the RCL, RCR, ROL, and ROR instructions for various operand sizes.  The rotate amount is always 8 bits wide
                 * in the instruction, but the semantics mask off all but the low-order bits, keeping 5 bits in 32-bit mode and 6 bits in
                 * 64-bit mode (indicated by the rotateSignificantBits argument). */
                /*virtual BaseSemantics::SValuePtr doRotateOperation(ARMv8InstructionKind kind,
                                                                   const BaseSemantics::SValuePtr &operand,
                                                                   const BaseSemantics::SValuePtr &total_rotate,
                                                                   size_t rotateSignificantBits);*/

                //FIXME
                /** Implements the SHR, SAR, SHL, SAL, SHRD, and SHLD instructions for various operand sizes.  The shift amount is always 8
                 *  bits wide in the instruction, but the semantics mask off all but the low-order bits, keeping 5 bits in 32-bit mode and
                 *  7 bits in 64-bit mode (indicated by the @p shiftSignificantBits argument).  The semantics of SHL and SAL are
                 *  identical (in fact, ROSE doesn't even define x86_sal). The @p source_bits argument contains the bits to be shifted into
                 *  the result and is used only for SHRD and SHLD instructions. */
                /*virtual BaseSemantics::SValuePtr doShiftOperation(ARMv8InstructionKind kind,
                                                                  const BaseSemantics::SValuePtr &operand,
                                                                  const BaseSemantics::SValuePtr &source_bits,
                                                                  const BaseSemantics::SValuePtr &total_shift,
                                                                  size_t shiftSignificantBits);*/

                /** Extend or truncate value to propert memory address width. */
                virtual BaseSemantics::SValuePtr fixMemoryAddress(const BaseSemantics::SValuePtr &address) const;

                /** Checks if the supplied value is or isn't equal to zero */
                virtual BaseSemantics::SValuePtr isZero(const BaseSemantics::SValuePtr &value);

                virtual BaseSemantics::SValuePtr ConditionHolds(const BaseSemantics::SValuePtr &cond);

                /** Inverts the passed in expression and returns the result */
                virtual BaseSemantics::SValuePtr NOT(const BaseSemantics::SValuePtr &expr);

                /** Execute a branch -- equivalent to writing the target address value to the PC */
                virtual void BranchTo(const BaseSemantics::SValuePtr &target);

                /** Returns a value that equals 0. nbits specifies what should be the bit-length of the value,
                 * but is irrelevant in practice as a 64-bit zero is returned anyway. */
                virtual BaseSemantics::SValuePtr Zeros(const unsigned int nbits);

                /** Returns the input value sign extended to the provided length. */
                virtual BaseSemantics::SValuePtr SignExtend(const BaseSemantics::SValuePtr &expr, size_t newsize);

                /** Returns the input value zero extended to the provided length. */
                virtual BaseSemantics::SValuePtr ZeroExtend(const BaseSemantics::SValuePtr &expr, size_t newsize);

                /** Returns the input value right rotated by the provided amount. */
                virtual BaseSemantics::SValuePtr ROR(const BaseSemantics::SValuePtr &expr, const BaseSemantics::SValuePtr &amt);

                /** Replicates the value contained in expr to fill the full 64-bit width. */
                virtual BaseSemantics::SValuePtr Replicate(const BaseSemantics::SValuePtr &expr);

                BaseSemantics::SValuePtr getBitfieldMask(int immr, int imms, int N, bool iswmask, int datasize);

                size_t getRegSize(uint32_t raw);

                size_t ldStrLiteralAccessSize(uint32_t raw);

                bool inzero(uint32_t raw);

                bool extend(uint32_t raw);

                int op(uint32_t raw);

                bool setflags(uint32_t raw);

                int getDatasize(uint32_t raw);

                int getShiftType(uint32_t raw);

                int getConditionVal(uint32_t raw);

                int opcode(uint32_t raw);

                bool subop(uint32_t raw);

                /** Reads memory of size readSize bits from address addr. */
                BaseSemantics::SValuePtr readMemory(const BaseSemantics::SValuePtr &addr, size_t readSize);

                /** Writes value data of size writeSize bits to memory at address addr. */
                void writeMemory(const BaseSemantics::SValuePtr &addr, size_t writeSize, const BaseSemantics::SValuePtr &data);

                /** Returns the register expression containing the target address for a write-back in case of memory-access instructions. */
                SgAsmExpression *getWriteBackTarget(SgAsmExpression *expr);

                /** Returns an expression that is an unsigned representation of expr. */
                BaseSemantics::SValuePtr UInt(const BaseSemantics::SValuePtr &expr);

                /** Applies a shift operation of type shiftType (defined in enum ShiftType) and shift length of amount to src. */
                BaseSemantics::SValuePtr ShiftReg(const BaseSemantics::SValuePtr &src, int shiftType, const BaseSemantics::SValuePtr &amount);

                /** Returns an expression representing the number of leading 0s in expr. */
                BaseSemantics::SValuePtr CountLeadingZeroBits(const BaseSemantics::SValuePtr &expr);

                /** Returns an expression representing the number of leading contiguous bits that match the sign bit in expr. */
                BaseSemantics::SValuePtr CountLeadingSignBits(const BaseSemantics::SValuePtr &expr);

                /** Returns an expression that is a signed representation of expr if unSigned is false and calls UInt if unSigned is true. */
                BaseSemantics::SValuePtr Int(const BaseSemantics::SValuePtr &expr, bool isUnsigned);

                /** Rounds a number to zero (upwards if it is negative, downwards if it is positive. */
                BaseSemantics::SValuePtr RoundTowardsZero(const BaseSemantics::SValuePtr &expr);
            };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Instruction processors
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            namespace AmdgpuVega {

/** Base class for all Amdgpu Vega instruction processors.
 *
 *  This class provides single-letter names for some types that are used in all instructions: D, I, A, and Ops for the
 *  dispatcher raw pointer, instruction pointer, argument list pointer, and RISC operators raw pointer.  It also takes care
 *  of advancing the instruction pointer prior to handing the instruction to the subclass, which by the way is done via
 *  @ref p method (short for "process").  See examples in DispatcherX86.C -- there are <em>lots</em> of them. */
                class InsnProcessor : public BaseSemantics::InsnProcessor {
                public:
                    typedef DispatcherAmdgpuVega *D;
                    typedef BaseSemantics::RiscOperators *Ops;
                    typedef SgAsmAmdgpuVegaInstruction *I;
                    typedef const SgAsmExpressionPtrList &A;
                    typedef uint32_t B;

                    virtual void p(D, Ops, I, A, B) = 0;

                    virtual void process(const BaseSemantics::DispatcherPtr &, SgAsmInstruction *);

                    virtual void assert_args(I insn, A args, size_t nargs);
                    //void check_arg_width(D d, I insn, A args);

                public:
                    enum MemOp {
                        MemOp_STORE,
                        MemOp_LOAD
                    };

                    enum MoveWideOp {
                        MoveWideOp_N,
                        MoveWideOp_Z,
                        MoveWideOp_K
                    };

                    enum LogicalOp {
                        LogicalOp_AND,
                        LogicalOp_ORR,
                        LogicalOp_EOR
                    };

                    enum ShiftType {
                        ShiftType_LSL,
                        ShiftType_LSR,
                        ShiftType_ASR,
                        ShiftType_ROR
                    };

                    enum CountOp {
                        CountOp_CLZ,
                        CountOp_CLS,
                        CountOp_CNT
                    };
                };

            } // namespace

        } // namespace
    } // namespace
} // namespace

#endif
