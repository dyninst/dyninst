#ifndef SG_ASM_AMDGPU_VEGA_INSN_H
#define SG_ASM_AMDGPU_VEGA_INSN_H

#include "external/rose/amdgpuInstructionEnum.h"
#include "typedefs.h"
#include "SgAsmInstruction.h"

class SgAsmAmdgpuVegaInstruction : public SgAsmInstruction {
public:

/*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

/*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

/*! \brief static variant value */
    enum {
        static_variant = V_SgAsmAmdgpuVegaInstruction
    };

/* the generated cast function */
/*! \brief Casts pointer from base class to derived class */
    friend SgAsmAmdgpuVegaInstruction *isSgAsmAmdgpuVegaInstruction( SgNode * s );

/*! \brief Casts pointer from base class to derived class (for const pointers) */
     friend const SgAsmAmdgpuVegaInstruction *isSgAsmAmdgpuVegaInstruction(const SgNode *s);

public:
    AmdgpuVegaInstructionKind get_kind() const;

    void set_kind(AmdgpuVegaInstructionKind kind);

public:
    virtual ~SgAsmAmdgpuVegaInstruction();


public:
    SgAsmAmdgpuVegaInstruction(rose_addr_t address = 0, std::string mnemonic = "",
                          AmdgpuVegaInstructionKind kind = rose_amdgpu_op_INVALID);


protected:
// Start of memberFunctionString
    AmdgpuVegaInstructionKind p_kind;

// End of memberFunctionString

};

#endif
