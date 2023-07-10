#ifndef SG_ASM_ARMV8_INSN_H
#define SG_ASM_ARMV8_INSN_H

#include <string>
#include "external/rose/armv8InstructionEnum.h"
#include "typedefs.h"
#include "SgAsmInstruction.h"

class SgAsmArmv8Instruction : public SgAsmInstruction {
public:

/*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

/*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

/*! \brief static variant value */
    enum {
        static_variant = V_SgAsmArmv8Instruction
    };

/* the generated cast function */
/*! \brief Casts pointer from base class to derived class */
    friend SgAsmArmv8Instruction *isSgAsmArmv8Instruction( SgNode * s );

/*! \brief Casts pointer from base class to derived class (for const pointers) */
     friend const SgAsmArmv8Instruction *isSgAsmArmv8Instruction(const SgNode *s);

public:
    ARMv8InstructionKind get_kind() const;

    void set_kind(ARMv8InstructionKind kind);

public:
    virtual ~SgAsmArmv8Instruction();


public:
    SgAsmArmv8Instruction(rose_addr_t address = 0, std::string mnemonic = "",
                          ARMv8InstructionKind kind = rose_aarch64_op_INVALID);


protected:
// Start of memberFunctionString
    ARMv8InstructionKind p_kind;

// End of memberFunctionString

};

#endif
