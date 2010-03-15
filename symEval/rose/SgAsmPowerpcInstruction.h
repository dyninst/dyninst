//
// C++ Interface: SgAsmPowerpcInstruction
//
// Description: 
//
//
// Author: Bill Williams <bill@follis.cs.wisc.edu>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//

#if !defined(SG_ASM_POWERPC_INSN_H)
#define SG_ASM_POWERPC_INSN_H

#include "external/rose/rose-compat.h"
#include "typedefs.h"
#include "SgNode.h"
#include "SgAsmOperandList.h"
#include "external/rose/powerpcInstructionEnum.h"

// Class Definition for SgAsmPowerpcInstruction
class SgAsmPowerpcInstruction : public SgNode
{

    public:
        rose_addr_t get_address() const;
        void set_address(rose_addr_t addr);
        void set_mnemonic(std::string mnem);
        std::string get_mnemonic() const;
        void set_operandSize(X86InstructionSize) {}
        void set_addressSize(X86InstructionSize) {}
        void set_raw_bytes(std::vector<unsigned char> bytes);
        SgUnsignedCharList get_raw_bytes() const;
        virtual SgAsmOperandList* get_operandList() const;
        void set_operandList(SgAsmOperandList* operands);
        /*! \brief returns a string representing the class name */
        virtual std::string class_name() const;

        /*! \brief returns new style SageIII enum values */
        virtual VariantT variantT() const; // MS: new variant used in tree traversal

        /*! \brief static variant value */
        static const VariantT static_variant = V_SgAsmPowerpcInstruction;

        /* the generated cast function */
        /*! \brief Casts pointer from base class to derived class */
        friend       SgAsmPowerpcInstruction* isSgAsmPowerpcInstruction(       SgNode * s );
        /*! \brief Casts pointer from base class to derived class (for const pointers) */
        friend const SgAsmPowerpcInstruction* isSgAsmPowerpcInstruction( const SgNode * s );

    public:
        PowerpcInstructionKind get_kind() const;
        void set_kind(PowerpcInstructionKind kind);


    public:
        virtual ~SgAsmPowerpcInstruction() {}


    public:
        SgAsmPowerpcInstruction(rose_addr_t address = 0, std::string mnemonic = "", PowerpcInstructionKind kind =
                powerpc_unknown_instruction);

    protected:
        unsigned int p_address;
        PowerpcInstructionKind p_kind;
        SgAsmOperandList* p_operandList;
        std::string p_mnemonic;
        SgUnsignedCharList p_raw_bytes;
};

   const char* regclassToString(PowerpcRegisterClass n);

   
#endif //!defined(SG_ASM_POWERPC_INSN_H)
