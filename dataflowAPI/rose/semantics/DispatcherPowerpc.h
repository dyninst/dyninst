// Semantics for 32-bit Motorola-IBM PowerPC microprocessors using ROSE instruction semantics API2
// This code was derived from $ROSE/projects/assemblyToSourceAst/powerpcInstructionSemantics.h,
// which is mostly a copy of $ROSE/projects/SemanticSignatureVectors/powerpcInstructionSemantics.h
//
// The ROSE style guide indicates that PowerPC, when used as part of a symbol in ROSE source code,
// should be capitalized as "Powerpc" (e.g., "DispatcherPowerpc", the same rule that consistently
// capitializes x86 as "DispatcherX86").

#ifndef ROSE_DispatcherPpc_H
#define ROSE_DispatcherPpc_H

#include <assert.h>
#include <stddef.h>
#include "BaseSemantics2.h"
#include "../SgAsmPowerpcInstruction.h"
#include "external/rose/powerpcInstructionEnum.h"

namespace rose {
namespace BinaryAnalysis {
namespace InstructionSemantics2 {

/** Shared-ownership pointer to a PowerPC instruction dispatcher. See @ref heap_object_shared_ownership. */
typedef boost::shared_ptr<class DispatcherPowerpc> DispatcherPowerpcPtr;

class DispatcherPowerpc: public BaseSemantics::Dispatcher {
protected:
    // prototypical constructor
    DispatcherPowerpc(): BaseSemantics::Dispatcher(32, RegisterDictionary::dictionary_powerpc()) {}

    DispatcherPowerpc(const BaseSemantics::RiscOperatorsPtr &ops, size_t addrWidth, const RegisterDictionary *regs)
        : BaseSemantics::Dispatcher(ops, addrWidth, regs ? regs : RegisterDictionary::dictionary_powerpc()) {
//        ASSERT_require(32==addrWidth);
        regcache_init();
        iproc_init();
        memory_init();
    }

    /** Loads the iproc table with instruction processing functors. This normally happens from the constructor. */
    void iproc_init();

    /** Load the cached register descriptors.  This happens at construction and on set_register_dictionary() calls. */
    void regcache_init();

    /** Make sure memory is set up correctly. For instance, byte order should be little endian. */
    void memory_init();

public:
    /** Cached register. This register is cached so that there are not so many calls to Dispatcher::findRegister(). The
     *  register descriptor is updated only when the register dictionary is changed (see set_register_dictionary()).
     * @{ */
    RegisterDescriptor REG_IAR, REG_LR, REG_XER, REG_CR, REG_CR0, REG_CTR;
    /** @}*/

    /** Construct a prototypical dispatcher.  The only thing this dispatcher can be used for is to create another dispatcher
     *  with the virtual @ref create method. */
    static DispatcherPowerpcPtr instance() {
        return DispatcherPowerpcPtr(new DispatcherPowerpc);
    }
    
    /** Constructor. */
    static DispatcherPowerpcPtr instance(const BaseSemantics::RiscOperatorsPtr &ops, size_t addrWidth,
                                         const RegisterDictionary *regs=NULL) {
        return DispatcherPowerpcPtr(new DispatcherPowerpc(ops, addrWidth, regs));
    }

    /** Virtual constructor. */
    virtual BaseSemantics::DispatcherPtr create(const BaseSemantics::RiscOperatorsPtr &ops, size_t addrWidth=0,
                                                const RegisterDictionary *regs=NULL) const  {
        if (0==addrWidth)
            addrWidth = addressWidth();
        if (!regs)
            regs = get_register_dictionary();
        return instance(ops, addrWidth, regs);
    }

    /** Dynamic cast to a DispatcherPowerpcPtr with assertion. */
    static DispatcherPowerpcPtr promote(const BaseSemantics::DispatcherPtr &d) {
        DispatcherPowerpcPtr retval = boost::dynamic_pointer_cast<DispatcherPowerpc>(d);
        assert(retval!=NULL);
        return retval;
    }

    virtual void set_register_dictionary(const RegisterDictionary *regdict) ;

    virtual RegisterDescriptor instructionPointerRegister() const ;

    virtual RegisterDescriptor stackPointerRegister() const ;

    virtual int iproc_key(SgAsmInstruction *insn_) const  {
        SgAsmPowerpcInstruction *insn = isSgAsmPowerpcInstruction(insn_);
        assert(insn!=NULL);
        return insn->get_kind();
    }

    /** Write status flags for result. */
    virtual void record(const BaseSemantics::SValuePtr &result);
};
        
} // namespace
} // namespace
} // namespace

#endif
