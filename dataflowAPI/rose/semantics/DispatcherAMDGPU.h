#ifndef ROSE_DispatcherAMDGPU_H
#define ROSE_DispatcherAMDGPU_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "BaseSemantics2.h"
#include "../SgAsmAMDGPUInstruction.h"
#include "external/rose/amdgpuInstructionEnum.h"



namespace rose {
    namespace BinaryAnalysis {
        namespace InstructionSemantics2 {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Dispatcher
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Shared-ownership pointer to an AMDGPU instruction dispatcher. See @ref heap_object_shared_ownership. */
            typedef boost::shared_ptr<class DispatcherAMDGPU> DispatcherAMDGPUPtr;

            class DispatcherAMDGPU : public BaseSemantics::Dispatcher {
            protected:
                // Prototypical constructor
                DispatcherAMDGPU()
                        : BaseSemantics::Dispatcher(64, RegisterDictionary::dictionary_amdgpu()) { }

                // Prototypical constructor
                DispatcherAMDGPU(size_t addrWidth, const RegisterDictionary *regs/*=NULL*/)
                        : BaseSemantics::Dispatcher(addrWidth, regs ? regs : RegisterDictionary::dictionary_amdgpu()) { }

                // Normal constructor
                DispatcherAMDGPU(const BaseSemantics::RiscOperatorsPtr &ops, size_t addrWidth,
                                const RegisterDictionary *regs)
                        : BaseSemantics::Dispatcher(ops, addrWidth,
                                                    regs ? regs : RegisterDictionary::dictionary_amdgpu()) {
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
                static DispatcherAMDGPUPtr instance() {
                    return DispatcherAMDGPUPtr(new DispatcherAMDGPU);
                }

                /** Construct a prototyipcal dispatcher. Construct a prototypical dispatcher with a specified address size. The only thing
                 * this dispatcher can be used for is to create another dispatcher with the virtual @ref create method. */
                static DispatcherAMDGPUPtr instance(size_t addrWidth, const RegisterDictionary *regs = NULL) {
                    return DispatcherAMDGPUPtr(new DispatcherAMDGPU(addrWidth, regs));
                }

                /** Constructor. */
                static DispatcherAMDGPUPtr instance(const BaseSemantics::RiscOperatorsPtr &ops, size_t addrWidth,
                                                   const RegisterDictionary *regs = NULL) {
                    return DispatcherAMDGPUPtr(new DispatcherAMDGPU(ops, addrWidth, regs));
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

                /** Dynamic cast to a DispatcherAMDGPUPtr with assertion. */
                static DispatcherAMDGPUPtr promote(const BaseSemantics::DispatcherPtr &d) {
                    DispatcherAMDGPUPtr retval = boost::dynamic_pointer_cast<DispatcherAMDGPU>(d);
                    assert(retval != NULL);
                    return retval;
                }

                virtual void set_register_dictionary(const RegisterDictionary *regdict);

                virtual RegisterDescriptor instructionPointerRegister() const;

                virtual RegisterDescriptor stackPointerRegister() const;

                virtual int iproc_key(SgAsmInstruction *insn_) const {
                    SgAsmAMDGPUInstruction *insn = isSgAsmAMDGPUInstruction(insn_);
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

                /** Returns the input value sign extended to the provided length. */
                virtual BaseSemantics::SValuePtr SignExtend(const BaseSemantics::SValuePtr &expr, size_t newsize);

                /** Returns the input value zero extended to the provided length. */
                virtual BaseSemantics::SValuePtr ZeroExtend(const BaseSemantics::SValuePtr &expr, size_t newsize);

            };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Instruction processors
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            namespace AMDGPU {

/** Base class for all AMDGPU  instruction processors.
 *
 *  This class provides single-letter names for some types that are used in all instructions: D, I, A, and Ops for the
 *  dispatcher raw pointer, instruction pointer, argument list pointer, and RISC operators raw pointer.  It also takes care
 *  of advancing the instruction pointer prior to handing the instruction to the subclass, which by the way is done via
 *  @ref p method (short for "process").  See examples in DispatcherX86.C -- there are <em>lots</em> of them. */
                class InsnProcessor : public BaseSemantics::InsnProcessor {
                public:
                    typedef DispatcherAMDGPU *D;
                    typedef BaseSemantics::RiscOperators *Ops;
                    typedef SgAsmAMDGPUInstruction *I;
                    typedef const SgAsmExpressionPtrList &A;
                    typedef uint32_t B;

                    virtual void p(D, Ops, I, A, B) = 0;

                    virtual void process(const BaseSemantics::DispatcherPtr &, SgAsmInstruction *);

                    virtual void assert_args(I insn, A args, size_t nargs);
                };

            } // namespace

        } // namespace
    } // namespace
} // namespace

#endif
