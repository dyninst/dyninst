#ifndef SG_ASM_ARMV6M_INSTRUCTION_H
#define SG_ASM_ARMV6M_INSTRUCTION_H

#include "external/rose/ARMv6MInstructionEnum.h"
#include "typedefs.h"
#include "SgAsmInstruction.h"


class SgAsmARMv6MInstruction : public SgAsmInstruction {
public:

/*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

/*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

/*! \brief static variant value */
    enum {
        static_variant = V_SgAsmARMv6MInstruction
    };

/* the generated cast function */
/*! \brief Casts pointer from base class to derived class */
    friend SgAsmARMv6MInstruction *isSgAsmARMv6MInstruction( SgNode * s );

/*! \brief Casts pointer from base class to derived class (for const pointers) */
     friend const SgAsmARMv6MInstruction *isSgAsmARMv6MInstruction(const SgNode *s);

public:
    ARMv6MInstructionKind get_kind() const;

    void set_kind(ARMv6MInstructionKind kind);

public:
    virtual ~SgAsmARMv6MInstruction();


public:
    SgAsmARMv6MInstruction(rose_addr_t address = 0, std::string mnemonic = "",
                          ARMv6MInstructionKind kind = rose_ARMv6M_op_INVALID);


protected:
// Start of memberFunctionString
    ARMv6MInstructionKind p_kind;

// End of memberFunctionString

};

#endif