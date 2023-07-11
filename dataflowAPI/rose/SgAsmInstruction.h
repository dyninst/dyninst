// This information is cut from a generated SAGEIII file. 
// Since we only need the data structure definition I've
// extracted that from the larger generated file.

// All methods that do not appear to be used by x86InstructionSemantics.h
// have been commented out. 

#if !defined(SG_ASM_INSN_H)
#define SG_ASM_INSN_H

#include "external/rose/rose-compat.h"
#include "typedefs.h"
#include "SgNode.h"

#include "SgAsmOperandList.h"
#include <stddef.h>
#include <string>

class SgAsmStatement : public SgAsmNode {
public:
    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    enum { static_variant = V_SgAsmStatement };

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmStatement* isSgAsmStatement(       SgNode * s );

    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmStatement* isSgAsmStatement( const SgNode * s );

public:
    rose_addr_t get_address() const;
    void set_address(rose_addr_t address);

public:
    std::string get_comment() const;
    void set_comment(std::string comment);

public:
    virtual ~SgAsmStatement();

public:
    SgAsmStatement(rose_addr_t address = 0);

protected:
    rose_addr_t p_address;

    std::string p_comment;

};

class SgAsmInstruction : public SgAsmStatement {
public:
    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    enum { static_variant = V_SgAsmInstruction };

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmInstruction* isSgAsmInstruction(       SgNode * s );

    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmInstruction* isSgAsmInstruction( const SgNode * s );

public:

    std::string get_mnemonic() const;
    void set_mnemonic(std::string mnemonic);

    SgUnsignedCharList get_raw_bytes() const;
    void set_raw_bytes(SgUnsignedCharList raw_bytes);

    SgAsmOperandList* get_operandList() const;
    void set_operandList(SgAsmOperandList* operandList);

    virtual size_t get_size() const;

public:
    SgAsmInstruction(rose_addr_t address = 0, std::string mnemonic = "");

public:
    ~SgAsmInstruction();

protected:
    std::string p_mnemonic;

    SgUnsignedCharList p_raw_bytes;

    SgAsmOperandList* p_operandList;
};

#endif
