#ifndef SG_ASM_RISCV64_INSN_H
#define SG_ASM_RISCV64_INSN_H

#include "external/rose/riscv64InstructionEnum.h"
#include "typedefs.h"
#include "SgAsmInstruction.h"

class SgAsmRiscv64Instruction : public SgAsmInstruction {
public:

/*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

/*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

/*! \brief static variant value */
    enum {
        static_variant = V_SgAsmRiscv64Instruction
    };

/* the generated cast function */
/*! \brief Casts pointer from base class to derived class */
    friend SgAsmRiscv64Instruction *isSgAsmRiscv64Instruction( SgNode * s );

/*! \brief Casts pointer from base class to derived class (for const pointers) */
     friend const SgAsmRiscv64Instruction *isSgAsmRiscv64Instruction(const SgNode *s);

public:
    Riscv64InstructionKind get_kind() const;

    void set_kind(Riscv64InstructionKind kind);

public:
    virtual ~SgAsmRiscv64Instruction();


public:
    SgAsmRiscv64Instruction(rose_addr_t address = 0, std::string mnemonic = "",
                          Riscv64InstructionKind kind = rose_riscv64_op_INVALID);


protected:
// Start of memberFunctionString
    Riscv64InstructionKind p_kind;

// End of memberFunctionString

};

#endif
