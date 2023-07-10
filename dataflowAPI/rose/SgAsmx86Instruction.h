#if !defined(SG_ASM_X86_INSN_H)
#define SG_ASM_X86_INSN_H
// This information is cut from a generated SAGEIII file. 
// Since we only need the data structure definition I've
// extracted that from the larger generated file.

// All methods that do not appear to be used by x86InstructionSemantics.h
// have been commented out. 

#include <stdint.h>
#include <string>
#include "external/rose/rose-compat.h"
#include "typedefs.h"
#include "SgAsmInstruction.h"

class SgAsmx86Instruction : public SgAsmInstruction {
 public:
    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmx86Instruction;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmx86Instruction* isSgAsmx86Instruction(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmx86Instruction* isSgAsmx86Instruction( const SgNode * s );

    X86InstructionKind get_kind() const;
    void set_kind(X86InstructionKind kind);

    X86InstructionSize get_baseSize() const;
    void set_baseSize(X86InstructionSize baseSize);

    X86InstructionSize get_operandSize() const;
    void set_operandSize(X86InstructionSize operandSize);

    X86InstructionSize get_addressSize() const;
    void set_addressSize(X86InstructionSize addressSize);

    bool get_lockPrefix() const;
    void set_lockPrefix(bool lockPrefix);

    X86RepeatPrefix get_repeatPrefix() const;
    void set_repeatPrefix(X86RepeatPrefix repeatPrefix);

    X86BranchPrediction get_branchPrediction() const;
    void set_branchPrediction(X86BranchPrediction branchPrediction);

    X86SegmentRegister get_segmentOverride() const;
    void set_segmentOverride(X86SegmentRegister segmentOverride);

    virtual ~SgAsmx86Instruction();

    SgAsmx86Instruction(rose_addr_t address = 0,
                        std::string mnemonic = "",
                        X86InstructionKind kind = x86_unknown_instruction,
                        X86InstructionSize baseSize = x86_insnsize_none,
                        X86InstructionSize operandSize = x86_insnsize_none,
                        X86InstructionSize addressSize = x86_insnsize_none);

 protected:

    X86InstructionKind p_kind;

    X86InstructionSize p_baseSize;

    X86InstructionSize p_operandSize;

    X86InstructionSize p_addressSize;

    bool p_lockPrefix;

    X86RepeatPrefix p_repeatPrefix;

    X86BranchPrediction p_branchPrediction;

    X86SegmentRegister p_segmentOverride;

    // End of memberFunctionString

};

// from  src/frontend/BinaryDisassembly/x86InstructionProperties.h

bool x86InstructionIsConditionalFlagControlTransfer(SgAsmx86Instruction* inst);
bool x86InstructionIsConditionalFlagDataTransfer(SgAsmx86Instruction* inst);
bool x86InstructionIsConditionalControlTransfer(SgAsmx86Instruction* inst);
bool x86InstructionIsConditionalDataTransfer(SgAsmx86Instruction* inst);

bool x86InstructionIsConditionalFlagBitAndByte(SgAsmx86Instruction* inst);

bool x86InstructionIsControlTransfer(SgAsmx86Instruction* inst);
bool x86InstructionIsUnconditionalBranch(SgAsmx86Instruction* inst);
bool x86InstructionIsConditionalBranch(SgAsmx86Instruction* inst);
bool x86InstructionIsDataTransfer(SgAsmx86Instruction* inst);
bool x86GetKnownBranchTarget(SgAsmx86Instruction* insn, uint64_t& addr);
// SgAsmx86Instruction* x86GetInstructionDestination(SgAsmx86Instruction* inst); // Returns non-fallthrough destination
// std::vector<SgAsmx86Instruction*> x86GetInstructionOutEdges(SgAsmx86Instruction* inst); // Returns all possible targets and fallthrough

const char* regclassToString(X86RegisterClass n);
const char* gprToString(X86GeneralPurposeRegister n);
const char* segregToString(X86SegmentRegister n);
const char* flagToString(X86Flag n);


#endif
