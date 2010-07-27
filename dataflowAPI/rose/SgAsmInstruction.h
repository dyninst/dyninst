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
#include <string>

class SgAsmInstruction : public SgNode {
 public:

    std::string get_mnemonic() const;
    void set_mnemonic(std::string mnemonic);
    
    SgUnsignedCharList get_raw_bytes() const;
    void set_raw_bytes(SgUnsignedCharList raw_bytes);
    
    SgAsmOperandList* get_operandList() const;
    void set_operandList(SgAsmOperandList* operandList);
    

    std::string get_comment() const;
    void set_comment(std::string comment);
    
    rose_addr_t get_address() const;
    void set_address(rose_addr_t address);

 protected:
    std::string p_mnemonic;
    
    SgUnsignedCharList p_raw_bytes;
    
    SgAsmOperandList* p_operandList;

    rose_addr_t p_address;
          
    std::string p_comment;
};

#endif
