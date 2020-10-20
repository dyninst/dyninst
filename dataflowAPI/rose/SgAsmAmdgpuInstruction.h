#ifndef SG_ASM_AMDGPU_INSN_H
#define SG_ASM_AMDGPU_INSN_H

#include "external/rose/amdgpuInstructionEnum.h"
#include "typedefs.h"
#include "SgAsmInstruction.h"

class SgAsmAmdgpuInstruction : public SgAsmInstruction {
public:

/*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

/*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

/*! \brief static variant value */
    enum {
        static_variant = V_SgAsmAmdgpuInstruction
    };

/* the generated cast function */
/*! \brief Casts pointer from base class to derived class */
    friend SgAsmAmdgpuInstruction *isSgAsmAmdgpuInstruction( SgNode * s );

/*! \brief Casts pointer from base class to derived class (for const pointers) */
     friend const SgAsmAmdgpuInstruction *isSgAsmAmdgpuInstruction(const SgNode *s);

public:
    AMDGPUInstructionKind get_kind() const;

    void set_kind(AMDGPUInstructionKind kind);

public:
    virtual ~SgAsmAmdgpuInstruction();


public:
    SgAsmAmdgpuInstruction(rose_addr_t address = 0, std::string mnemonic = "",
                          AMDGPUInstructionKind kind = rose_amdgpu_op_INVALID);


protected:
// Start of memberFunctionString
    AMDGPUInstructionKind p_kind;

// End of memberFunctionString

};

#endif
