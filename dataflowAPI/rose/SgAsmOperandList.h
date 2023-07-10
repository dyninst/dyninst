// Class Definition for SgAsmOperandList
#if !defined(SG_ASM_OPERAND_LIST_H)
#define SG_ASM_OPERAND_LIST_H

#include <string>
#include "SgAsmType.h"
#include "typedefs.h"

class SgAsmOperandList : public SgAsmNode {
 public:

    void append_operand( SgAsmExpression* operand );
 public:

       // DQ (3/25/3006): I put this back in because it had the logic for where the copy function required 
       // and not required which is required to match the other aspects of the copy mechanism code generation.
       // Specifically it is a problem to declare the copy function everywhere because it is not implemented 
       // for the SgSymbol IR nodes. I'm not clear why it is not implemented for these IR nodes.
      /*! \brief Copies AST (whole subtree, depending on the SgCopyHelp class */
       // virtual SgNode* copy ( const SgCopyHelp & help) const;

      /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;
    
    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal
    
    /*! \brief static variant value */
    enum { static_variant = V_SgAsmOperandList };
    
    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmOperandList* isSgAsmOperandList(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmOperandList* isSgAsmOperandList( const SgNode * s );
    
 public: 
    const SgAsmExpressionPtrList&  get_operands() const;
    SgAsmExpressionPtrList& get_operands(); 
    
    
 public: 
    virtual ~SgAsmOperandList();
    
    
 public: 
    SgAsmOperandList(); 
    
 protected:
    // Start of memberFunctionString
    SgAsmExpressionPtrList p_operands;
};

#endif
