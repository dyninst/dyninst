#if !defined(SG_ASM_EXPR_H)
#define SG_ASM_EXPR_H

#include "external/rose/rose-compat.h"
#include "SgNode.h"
#include "SgAsmType.h"
#include "external/rose/powerpcInstructionEnum.h"


#if defined(_MSC_VER)
#include "external/stdint-win.h"
#include "external/inttypes-win.h"
#else
#include <stdint.h>
#include <inttypes.h>
#endif


class SgAsmExpression : public SgNode {
 public:
    virtual SgAsmType* get_type();

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;
    
    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal
    
    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmExpression;
    
    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmExpression* isSgAsmExpression(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmExpression* isSgAsmExpression( const SgNode * s );
    
    
    std::string get_replacement() const;
    void set_replacement(std::string replacement);
    
    std::string get_comment() const;
    void set_comment(std::string comment);

    virtual ~SgAsmExpression();
    
    SgAsmExpression(); 
    
 protected:
    std::string p_replacement;
    
    std::string p_comment;
};

// Class Definition for SgAsmValueExpression
class SgAsmValueExpression : public SgAsmExpression {
 public:
    virtual SgAsmType* get_type();

    virtual std::string class_name() const;
    
    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal
    
    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmValueExpression;
    
    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmValueExpression* isSgAsmValueExpression(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmValueExpression* isSgAsmValueExpression( const SgNode * s );
    
 public: 
    SgAsmValueExpression* get_unfolded_expression_tree() const;
    void set_unfolded_expression_tree(SgAsmValueExpression* unfolded_expression_tree);
    
 public: 
    unsigned short get_bit_offset() const;
    void set_bit_offset(unsigned short bit_offset);
    
 public: 
    unsigned short get_bit_size() const;
    void set_bit_size(unsigned short bit_size);
    
    
 public: 
    virtual ~SgAsmValueExpression();
    
    
 public: 
    SgAsmValueExpression(); 
    
 protected:
    // Start of memberFunctionString
    SgAsmValueExpression* p_unfolded_expression_tree;
    
    // End of memberFunctionString
    // Start of memberFunctionString
    unsigned short p_bit_offset;
    
    // End of memberFunctionString
    // Start of memberFunctionString
    unsigned short p_bit_size;  

    
    // End of memberFunctionString
};

class SgAsmByteValueExpression : public SgAsmValueExpression {
 public:
	virtual SgAsmType* get_type();
    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;
    
    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal
    
    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmByteValueExpression;
    
    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmByteValueExpression* isSgAsmByteValueExpression(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmByteValueExpression* isSgAsmByteValueExpression( const SgNode * s );
    
 public: 
    uint8_t get_value() const;
    void set_value(uint8_t value);
    
    
 public: 
    virtual ~SgAsmByteValueExpression();
    
    
 public: 
    SgAsmByteValueExpression(uint8_t value = 0x0); 
    
 protected:
    // Start of memberFunctionString
    uint8_t p_value;
};

class SgAsmWordValueExpression : public SgAsmValueExpression {
 public:
 	virtual SgAsmType* get_type();
   /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;
    
    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal
    
    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmWordValueExpression;
    
    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmWordValueExpression* isSgAsmWordValueExpression(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmWordValueExpression* isSgAsmWordValueExpression( const SgNode * s );
    
 public: 
    uint16_t get_value() const;
    void set_value(uint16_t value);
    
    
 public: 
    virtual ~SgAsmWordValueExpression();
    
    
 public: 
    SgAsmWordValueExpression(uint16_t value = 0x0); 
    
 protected:
    // Start of memberFunctionString
    uint16_t p_value;
};

// Class Definition for SgAsmDoubleWordValueExpression
class SgAsmDoubleWordValueExpression : public SgAsmValueExpression {
 public:
 	virtual SgAsmType* get_type();
   /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;
    
    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal
    
    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmDoubleWordValueExpression;
    
    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmDoubleWordValueExpression* isSgAsmDoubleWordValueExpression(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmDoubleWordValueExpression* isSgAsmDoubleWordValueExpression( const SgNode * s );
    
 public: 
    uint32_t get_value() const;
    void set_value(uint32_t value);
    
    
 public: 
    virtual ~SgAsmDoubleWordValueExpression();
    
    
 public: 
    SgAsmDoubleWordValueExpression(uint32_t value = 0x0); 
    
 protected:
    // Start of memberFunctionString
    uint32_t p_value;
    
};

class SgAsmQuadWordValueExpression : public SgAsmValueExpression {
 public:
 	virtual SgAsmType* get_type();
   
    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;
    
    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal
    
    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmQuadWordValueExpression;
    
    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmQuadWordValueExpression* isSgAsmQuadWordValueExpression(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmQuadWordValueExpression* isSgAsmQuadWordValueExpression( const SgNode * s );
    
    
    // End of memberFunctionString
    
 public: 
    uint64_t get_value() const;
    void set_value(uint64_t value);
    
    
 public: 
    virtual ~SgAsmQuadWordValueExpression();
    
    
 public: 
    SgAsmQuadWordValueExpression(uint64_t value = 0x0); 
    
 protected:
    // Start of memberFunctionString
    uint64_t p_value;
    
    // End of memberFunctionString
};

class SgAsmSingleFloatValueExpression : public SgAsmValueExpression {
 public:
	virtual SgAsmType* get_type();

    // virtual SgNode* copy ( const SgCopyHelp & help) const;

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmSingleFloatValueExpression;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmSingleFloatValueExpression* isSgAsmSingleFloatValueExpression(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmSingleFloatValueExpression* isSgAsmSingleFloatValueExpression( const SgNode * s );

 public: 
    float get_value() const;
    void set_value(float value);


 public: 
    virtual ~SgAsmSingleFloatValueExpression();


 public: 
    SgAsmSingleFloatValueExpression(float value = 0.0F); 

 protected:
    // Start of memberFunctionString
    float p_value;
};


class SgAsmDoubleFloatValueExpression : public SgAsmValueExpression
{
 public:
	virtual SgAsmType* get_type();

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmDoubleFloatValueExpression;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmDoubleFloatValueExpression* isSgAsmDoubleFloatValueExpression(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmDoubleFloatValueExpression* isSgAsmDoubleFloatValueExpression( const SgNode * s );
 public: 
    double get_value() const;
    void set_value(double value);


 public: 
    virtual ~SgAsmDoubleFloatValueExpression();


 public: 
    SgAsmDoubleFloatValueExpression(double value = 0.0); 

 protected:
    // Start of memberFunctionString
    double p_value;
};

class SgAsmType;

class SgAsmVectorValueExpression : public SgAsmValueExpression
{
 public:
	virtual SgAsmType* get_type();

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmVectorValueExpression;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmVectorValueExpression* isSgAsmVectorValueExpression(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmVectorValueExpression* isSgAsmVectorValueExpression( const SgNode * s );

 public: 
    unsigned int get_size() const;
    void set_size(unsigned int size);

 public: 
    SgAsmType* get_type() const;
    void set_type(SgAsmType* type);


 public: 
    virtual ~SgAsmVectorValueExpression();


 public: 
    SgAsmVectorValueExpression(); 

 protected:
    // Start of memberFunctionString
    unsigned int p_size;
          
    // End of memberFunctionString
    // Start of memberFunctionString
    SgAsmType* p_type;
          
    // End of memberFunctionString
};

class SgAsmBinaryExpression : public SgAsmExpression
{
 public:

	virtual SgAsmType* get_type();

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmBinaryExpression;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmBinaryExpression* isSgAsmBinaryExpression(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmBinaryExpression* isSgAsmBinaryExpression( const SgNode * s );

 public: 
    SgAsmExpression* get_lhs() const;
    void set_lhs(SgAsmExpression* lhs);

 public: 
    SgAsmExpression* get_rhs() const;
    void set_rhs(SgAsmExpression* rhs);


 public: 
    virtual ~SgAsmBinaryExpression();


 public: 
    SgAsmBinaryExpression(SgAsmExpression* lhs = NULL, SgAsmExpression* rhs = NULL); 

 protected:
    // Start of memberFunctionString
    SgAsmExpression* p_lhs;
          
    // End of memberFunctionString
    // Start of memberFunctionString
    SgAsmExpression* p_rhs;
          
    // End of memberFunctionString


};

class SgAsmBinaryAdd : public SgAsmBinaryExpression
{
 public:


    virtual SgAsmType* get_type();

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmBinaryAdd;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmBinaryAdd* isSgAsmBinaryAdd(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmBinaryAdd* isSgAsmBinaryAdd( const SgNode * s );

 public: 
    virtual ~SgAsmBinaryAdd();


 public: 
    SgAsmBinaryAdd(SgAsmExpression* lhs = NULL, SgAsmExpression* rhs = NULL); 

 protected:
};

class SgAsmBinarySubtract : public SgAsmBinaryExpression
{
 public:
    virtual SgAsmType* get_type();

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmBinarySubtract;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmBinarySubtract* isSgAsmBinarySubtract(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmBinarySubtract* isSgAsmBinarySubtract( const SgNode * s );
 public: 
    virtual ~SgAsmBinarySubtract();


 public: 
    SgAsmBinarySubtract(SgAsmExpression* lhs = NULL, SgAsmExpression* rhs = NULL); 

 protected:




};

// Class Definition for SgAsmBinaryMultiply
class SgAsmBinaryMultiply : public SgAsmBinaryExpression
{
 public:
    virtual SgAsmType* get_type();

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmBinaryMultiply;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmBinaryMultiply* isSgAsmBinaryMultiply(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmBinaryMultiply* isSgAsmBinaryMultiply( const SgNode * s );

 public: 
    virtual ~SgAsmBinaryMultiply();


 public: 
    SgAsmBinaryMultiply(SgAsmExpression* lhs = NULL, SgAsmExpression* rhs = NULL); 
};

class SgAsmBinaryDivide : public SgAsmBinaryExpression
{
 public:
    virtual SgAsmType* get_type();

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmBinaryDivide;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmBinaryDivide* isSgAsmBinaryDivide(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmBinaryDivide* isSgAsmBinaryDivide( const SgNode * s );

 public: 
    virtual ~SgAsmBinaryDivide();


 public: 
    SgAsmBinaryDivide(SgAsmExpression* lhs = NULL, SgAsmExpression* rhs = NULL); 

 protected:


};

// Class Definition for SgAsmBinaryMod
class SgAsmBinaryMod : public SgAsmBinaryExpression
{
 public:    
    virtual SgAsmType* get_type();
    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmBinaryMod;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmBinaryMod* isSgAsmBinaryMod(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmBinaryMod* isSgAsmBinaryMod( const SgNode * s );

 public: 
    virtual ~SgAsmBinaryMod();


 public: 
    SgAsmBinaryMod(SgAsmExpression* lhs = NULL, SgAsmExpression* rhs = NULL); 
};

class SgAsmBinaryAddPreupdate : public SgAsmBinaryExpression
{
 public:
    virtual SgAsmType* get_type();

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmBinaryAddPreupdate;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmBinaryAddPreupdate* isSgAsmBinaryAddPreupdate(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmBinaryAddPreupdate* isSgAsmBinaryAddPreupdate( const SgNode * s );

 public: 
    virtual ~SgAsmBinaryAddPreupdate();


 public: 
    SgAsmBinaryAddPreupdate(SgAsmExpression* lhs = NULL, SgAsmExpression* rhs = NULL); 


};

class SgAsmBinarySubtractPreupdate : public SgAsmBinaryExpression
{
 public:
    virtual SgAsmType* get_type();

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmBinarySubtractPreupdate;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmBinarySubtractPreupdate* isSgAsmBinarySubtractPreupdate(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmBinarySubtractPreupdate* isSgAsmBinarySubtractPreupdate( const SgNode * s );


 public: 
    virtual ~SgAsmBinarySubtractPreupdate();


 public: 
    SgAsmBinarySubtractPreupdate(SgAsmExpression* lhs = NULL, SgAsmExpression* rhs = NULL); 

};

class SgAsmBinaryAddPostupdate : public SgAsmBinaryExpression
{
 public:

    virtual SgAsmType* get_type();
    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmBinaryAddPostupdate;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmBinaryAddPostupdate* isSgAsmBinaryAddPostupdate(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmBinaryAddPostupdate* isSgAsmBinaryAddPostupdate( const SgNode * s );

 public: 
    virtual ~SgAsmBinaryAddPostupdate();


 public: 
    SgAsmBinaryAddPostupdate(SgAsmExpression* lhs = NULL, SgAsmExpression* rhs = NULL); 


};

class SgAsmBinarySubtractPostupdate : public SgAsmBinaryExpression
{
 public:

    virtual SgAsmType* get_type();
    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmBinarySubtractPostupdate;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmBinarySubtractPostupdate* isSgAsmBinarySubtractPostupdate(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmBinarySubtractPostupdate* isSgAsmBinarySubtractPostupdate( const SgNode * s );

 public: 
    virtual ~SgAsmBinarySubtractPostupdate();


 public: 
    SgAsmBinarySubtractPostupdate(SgAsmExpression* lhs = NULL, SgAsmExpression* rhs = NULL); 

};

class SgAsmBinaryLsl : public SgAsmBinaryExpression
{
 public:
    virtual SgAsmType* get_type();
    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmBinaryLsl;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmBinaryLsl* isSgAsmBinaryLsl(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmBinaryLsl* isSgAsmBinaryLsl( const SgNode * s );



 public: 
    virtual ~SgAsmBinaryLsl();


 public: 
    SgAsmBinaryLsl(SgAsmExpression* lhs = NULL, SgAsmExpression* rhs = NULL); 

};

class SgAsmBinaryLsr : public SgAsmBinaryExpression
{
 public:
    virtual SgAsmType* get_type();


    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmBinaryLsr;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmBinaryLsr* isSgAsmBinaryLsr(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmBinaryLsr* isSgAsmBinaryLsr( const SgNode * s );

 public: 
    virtual ~SgAsmBinaryLsr();


 public: 
    SgAsmBinaryLsr(SgAsmExpression* lhs = NULL, SgAsmExpression* rhs = NULL); 

};

class SgAsmBinaryAsr : public SgAsmBinaryExpression
{
 public:
    virtual SgAsmType* get_type();


    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmBinaryAsr;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmBinaryAsr* isSgAsmBinaryAsr(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmBinaryAsr* isSgAsmBinaryAsr( const SgNode * s );

 public: 
    virtual ~SgAsmBinaryAsr();


 public: 
    SgAsmBinaryAsr(SgAsmExpression* lhs = NULL, SgAsmExpression* rhs = NULL); 

};

class SgAsmBinaryRor : public SgAsmBinaryExpression
{
 public:      
    virtual SgAsmType* get_type();
    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmBinaryRor;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmBinaryRor* isSgAsmBinaryRor(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmBinaryRor* isSgAsmBinaryRor( const SgNode * s );


 public: 
    virtual ~SgAsmBinaryRor();


 public: 
    SgAsmBinaryRor(SgAsmExpression* lhs = NULL, SgAsmExpression* rhs = NULL); 

 
};

class SgAsmUnaryExpression : public SgAsmExpression
{
 public:
    virtual SgAsmType* get_type();
    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmUnaryExpression;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmUnaryExpression* isSgAsmUnaryExpression(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmUnaryExpression* isSgAsmUnaryExpression( const SgNode * s );

 public: 
    SgAsmExpression* get_operand() const;
    void set_operand(SgAsmExpression* operand);


 public: 
    virtual ~SgAsmUnaryExpression();


 public: 
    SgAsmUnaryExpression(SgAsmExpression* operand = NULL); 

 protected:
    // Start of memberFunctionString
    SgAsmExpression* p_operand;
          
    // End of memberFunctionString


};

class SgAsmUnaryPlus : public SgAsmUnaryExpression
{
 public:
    virtual SgAsmType* get_type();

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmUnaryPlus;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmUnaryPlus* isSgAsmUnaryPlus(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmUnaryPlus* isSgAsmUnaryPlus( const SgNode * s );

 public: 
    virtual ~SgAsmUnaryPlus();


 public: 
    SgAsmUnaryPlus(SgAsmExpression* operand = NULL); 

 protected:


};

class SgAsmUnaryMinus : public SgAsmUnaryExpression
{
 public:
    virtual SgAsmType* get_type();

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmUnaryMinus;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmUnaryMinus* isSgAsmUnaryMinus(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmUnaryMinus* isSgAsmUnaryMinus( const SgNode * s );


 public: 
    virtual ~SgAsmUnaryMinus();


 public: 
    SgAsmUnaryMinus(SgAsmExpression* operand = NULL); 

 protected:


};

// Class Definition for SgAsmUnaryRrx
class SgAsmUnaryRrx : public SgAsmUnaryExpression
{
 public:

    virtual SgAsmType* get_type();

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmUnaryRrx;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmUnaryRrx* isSgAsmUnaryRrx(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmUnaryRrx* isSgAsmUnaryRrx( const SgNode * s );

 public: 
    virtual ~SgAsmUnaryRrx();


 public: 
    SgAsmUnaryRrx(SgAsmExpression* operand = NULL); 

 protected:


};

// Class Definition for SgAsmMemoryReferenceExpression
class SgAsmMemoryReferenceExpression : public SgAsmExpression
{
 public:
    virtual SgAsmType* get_type();

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmMemoryReferenceExpression;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmMemoryReferenceExpression* isSgAsmMemoryReferenceExpression(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmMemoryReferenceExpression* isSgAsmMemoryReferenceExpression( const SgNode * s );

 public: 
    SgAsmExpression* get_address() const;
    void set_address(SgAsmExpression* address);

 public: 
    SgAsmExpression* get_segment() const;
    void set_segment(SgAsmExpression* segment);

 public: 
    SgAsmType* get_type() const;
    void set_type(SgAsmType* type);


 public: 
    virtual ~SgAsmMemoryReferenceExpression();


 public: 
    SgAsmMemoryReferenceExpression(SgAsmExpression* address = NULL, SgAsmExpression* segment = NULL); 

 protected:
    // Start of memberFunctionString
    SgAsmExpression* p_address;
          
    // End of memberFunctionString
    // Start of memberFunctionString
    SgAsmExpression* p_segment;
          
    // End of memberFunctionString
    // Start of memberFunctionString
    SgAsmType* p_type;
          
    // End of memberFunctionString


};

// Class Definition for SgAsmRegisterReferenceExpression
class SgAsmRegisterReferenceExpression : public SgAsmExpression
{
 public:
    virtual SgAsmType* get_type();


    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmRegisterReferenceExpression;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmRegisterReferenceExpression* isSgAsmRegisterReferenceExpression(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmRegisterReferenceExpression* isSgAsmRegisterReferenceExpression( const SgNode * s );

 public: 
    virtual void set_type(SgAsmType* type);


 public: 
    virtual ~SgAsmRegisterReferenceExpression();


 public: 
    SgAsmRegisterReferenceExpression(); 

 protected:
    // Start of memberFunctionString
    SgAsmType* p_type;
          
    // End of memberFunctionString


};

class SgAsmx86RegisterReferenceExpression : public SgAsmRegisterReferenceExpression
{
 public:


    //! Get a unique identifier for this particular register
    std::pair<X86RegisterClass, int> get_identifier() const;

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmx86RegisterReferenceExpression;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend       SgAsmx86RegisterReferenceExpression* isSgAsmx86RegisterReferenceExpression(       SgNode * s );
    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmx86RegisterReferenceExpression* isSgAsmx86RegisterReferenceExpression( const SgNode * s );

 public: 
    X86RegisterClass get_register_class() const;
    void set_register_class(X86RegisterClass register_class);

 public: 
    int get_register_number() const;
    void set_register_number(int register_number);

 public: 
    X86PositionInRegister get_position_in_register() const;
    void set_position_in_register(X86PositionInRegister position_in_register);


 public: 
    virtual ~SgAsmx86RegisterReferenceExpression();


 public: 
    SgAsmx86RegisterReferenceExpression(X86RegisterClass register_class = x86_regclass_unknown, int register_number = 0, X86PositionInRegister position_in_register = x86_regpos_unknown); 

 protected:
    // Start of memberFunctionString
    X86RegisterClass p_register_class;
          
    // End of memberFunctionString
    // Start of memberFunctionString
    int p_register_number;
          
    // End of memberFunctionString
    // Start of memberFunctionString
    X86PositionInRegister p_position_in_register;
          
    // End of memberFunctionString
};

// Class Definition for SgAsmPowerpcRegisterReferenceExpression
class SgAsmPowerpcRegisterReferenceExpression : public SgAsmRegisterReferenceExpression
{
    public:
        enum powerpc_register_enum { // The exact numbers here are important
    undefined_powerpc_register = 0, /*!< unknown (error or unitialized value) */
    reg0 = 1,
    reg1 = 2,
    reg2 = 3,
    reg3 = 4,
    reg4 = 5,
    reg5 = 6,
    reg6 = 7,
    reg7 = 8,
    last_powerpc_register
        };

    public:

        /*! \brief returns a string representing the class name */
        virtual std::string class_name() const { return "SgAsmPowerpcRegisterReferenceExpression"; }

        /*! \brief returns new style SageIII enum values */
        virtual VariantT variantT() const { return static_variant; } 

        /*! \brief static variant value */
        static const VariantT static_variant = V_SgAsmPowerpcRegisterReferenceExpression;

        /* the generated cast function */
        /*! \brief Casts pointer from base class to derived class */
        friend       SgAsmPowerpcRegisterReferenceExpression* isSgAsmPowerpcRegisterReferenceExpression(       SgNode * s );
        /*! \brief Casts pointer from base class to derived class (for const pointers) */
        friend const SgAsmPowerpcRegisterReferenceExpression* isSgAsmPowerpcRegisterReferenceExpression( const SgNode * s );

    public:
        PowerpcRegisterClass get_register_class() const;
        void set_register_class(PowerpcRegisterClass register_class);
        int get_register_number() const;
        void set_register_number(int register_number);
        PowerpcConditionRegisterAccessGranularity get_conditionRegisterGranularity() const;
        void set_conditionRegisterGranularity(PowerpcConditionRegisterAccessGranularity conditionRegisterGranularity);
        virtual ~SgAsmPowerpcRegisterReferenceExpression();
        SgAsmPowerpcRegisterReferenceExpression(PowerpcRegisterClass register_class = powerpc_regclass_unknown, int
                register_number = 0, PowerpcConditionRegisterAccessGranularity conditionRegisterGranularity =
                        powerpc_condreggranularity_whole);

    protected:
        PowerpcRegisterClass p_register_class;
        int p_register_number;
        PowerpcConditionRegisterAccessGranularity p_conditionRegisterGranularity;
};

uint64_t getAsmSignedConstant(SgAsmValueExpression *e);


#endif
