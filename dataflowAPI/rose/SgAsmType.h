#if !defined(SG_ASM_TYPE_H)
#define SG_ASM_TYPE_H

#include <stddef.h>
#include <stdint.h>
#include <string>
#include "SgNode.h"
#include "util/Sawyer.h"
#include "util/BitVector.h"
#include "external/rose/rose-compat.h"

class SgAsmNode : public SgNode {
public:
    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    enum {
        static_variant = V_SgAsmNode
    };

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend SgAsmNode *isSgAsmNode(SgNode *s);

    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmNode *isSgAsmNode(const SgNode *s);

public:
    virtual ~SgAsmNode();

public:
    SgAsmNode();
    SgAsmNode(const SgAsmNode &) = default;
};

//TODO: check for other members
class SgAsmType : public SgAsmNode {
public:

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    enum {
        static_variant = V_SgAsmType
    };

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend SgAsmType *isSgAsmType(SgNode *s);

    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmType *isSgAsmType(const SgNode *s);

public:
    virtual ~SgAsmType();

    virtual size_t get_nBits() const { return (size_t) -1; }

public:
    SgAsmType();
    SgAsmType(const SgAsmType &) = default;

protected:


};

class SgAsmTypeByte : public SgAsmType {
public:

    static SgAsmTypeByte *createType();

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmTypeByte;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend SgAsmTypeByte *isSgAsmTypeByte(SgNode *s);

    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmTypeByte *isSgAsmTypeByte(const SgNode *s);

public:
    virtual ~SgAsmTypeByte();


public:
    SgAsmTypeByte();

protected:
    // Start of memberFunctionString
    static SgAsmTypeByte *p_builtin_type;

    // End of memberFunctionString


};

class SgAsmTypeWord : public SgAsmType {
public:


    static SgAsmTypeWord *createType();

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmTypeWord;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend SgAsmTypeWord *isSgAsmTypeWord(SgNode *s);

    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmTypeWord *isSgAsmTypeWord(const SgNode *s);

public:
    virtual ~SgAsmTypeWord();


public:
    SgAsmTypeWord();

protected:
    // Start of memberFunctionString
    static SgAsmTypeWord *p_builtin_type;

    // End of memberFunctionString


};

class SgAsmTypeDoubleWord : public SgAsmType {
public:

    static SgAsmTypeDoubleWord *createType();


    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmTypeDoubleWord;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend SgAsmTypeDoubleWord *isSgAsmTypeDoubleWord(SgNode *s);

    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmTypeDoubleWord *isSgAsmTypeDoubleWord(const SgNode *s);

public:
    virtual ~SgAsmTypeDoubleWord();


public:
    SgAsmTypeDoubleWord();

protected:
    // Start of memberFunctionString
    static SgAsmTypeDoubleWord *p_builtin_type;

    // End of memberFunctionString


};

class SgAsmTypeQuadWord : public SgAsmType {
public:

    static SgAsmTypeQuadWord *createType();


    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmTypeQuadWord;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend SgAsmTypeQuadWord *isSgAsmTypeQuadWord(SgNode *s);

    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmTypeQuadWord *isSgAsmTypeQuadWord(const SgNode *s);

public:
    virtual ~SgAsmTypeQuadWord();


public:
    SgAsmTypeQuadWord();

protected:
    // Start of memberFunctionString
    static SgAsmTypeQuadWord *p_builtin_type;

    // End of memberFunctionString


};

class SgAsmTypeDoubleQuadWord : public SgAsmType {
public:


    static SgAsmTypeDoubleQuadWord *createType();

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmTypeDoubleQuadWord;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend SgAsmTypeDoubleQuadWord *isSgAsmTypeDoubleQuadWord(SgNode *s);

    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmTypeDoubleQuadWord *isSgAsmTypeDoubleQuadWord(const SgNode *s);

public:
    virtual ~SgAsmTypeDoubleQuadWord();


public:
    SgAsmTypeDoubleQuadWord();

protected:
    // Start of memberFunctionString
    static SgAsmTypeDoubleQuadWord *p_builtin_type;

    // End of memberFunctionString


};

class SgAsmType80bitFloat : public SgAsmType {
public:

    static SgAsmType80bitFloat *createType();

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmType80bitFloat;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend SgAsmType80bitFloat *isSgAsmType80bitFloat(SgNode *s);

    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmType80bitFloat *isSgAsmType80bitFloat(const SgNode *s);

public:
    virtual ~SgAsmType80bitFloat();


public:
    SgAsmType80bitFloat();

protected:
    // Start of memberFunctionString
    static SgAsmType80bitFloat *p_builtin_type;

    // End of memberFunctionString


};

class SgAsmType128bitFloat : public SgAsmType {
public:
    static SgAsmType128bitFloat *createType();


    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmType128bitFloat;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend SgAsmType128bitFloat *isSgAsmType128bitFloat(SgNode *s);

    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmType128bitFloat *isSgAsmType128bitFloat(const SgNode *s);

public:
    virtual ~SgAsmType128bitFloat();


public:
    SgAsmType128bitFloat();

protected:
    // Start of memberFunctionString
    static SgAsmType128bitFloat *p_builtin_type;

    // End of memberFunctionString


};

class SgAsmTypeSingleFloat : public SgAsmType {
public:


    static SgAsmTypeSingleFloat *createType();

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmTypeSingleFloat;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend SgAsmTypeSingleFloat *isSgAsmTypeSingleFloat(SgNode *s);

    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmTypeSingleFloat *isSgAsmTypeSingleFloat(const SgNode *s);

public:
    virtual ~SgAsmTypeSingleFloat();


public:
    SgAsmTypeSingleFloat();

protected:
    // Start of memberFunctionString
    static SgAsmTypeSingleFloat *p_builtin_type;

    // End of memberFunctionString


};

class SgAsmTypeDoubleFloat : public SgAsmType {
public:

    static SgAsmTypeDoubleFloat *createType();

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmTypeDoubleFloat;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend SgAsmTypeDoubleFloat *isSgAsmTypeDoubleFloat(SgNode *s);

    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmTypeDoubleFloat *isSgAsmTypeDoubleFloat(const SgNode *s);

public:
    virtual ~SgAsmTypeDoubleFloat();


public:
    SgAsmTypeDoubleFloat();

protected:
    // Start of memberFunctionString
    static SgAsmTypeDoubleFloat *p_builtin_type;

    // End of memberFunctionString


};

class SgAsmTypeVector : public SgAsmType {
public:

    static SgAsmTypeVector *createType(int elementCount, SgAsmType *elementType);

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    static const VariantT static_variant = V_SgAsmTypeVector;

    /* the generated cast function */
    /*! \brief Casts pointer from base class to derived class */
    friend SgAsmTypeVector *isSgAsmTypeVector(SgNode *s);

    /*! \brief Casts pointer from base class to derived class (for const pointers) */
    friend const SgAsmTypeVector *isSgAsmTypeVector(const SgNode *s);

public:
    int get_elementCount() const;

    void set_elementCount(int elementCount);

public:
    SgAsmType *get_elementType() const;

    void set_elementType(SgAsmType *elementType);


public:
    virtual ~SgAsmTypeVector();


public:
    SgAsmTypeVector(int elementCount = 0, SgAsmType *elementType = NULL);

protected:
    // Start of memberFunctionString
    int p_elementCount;

    // End of memberFunctionString
    // Start of memberFunctionString
    SgAsmType *p_elementType;

    // End of memberFunctionString
};

class SgAsmScalarType : public SgAsmType {
protected:
    SgAsmScalarType(ByteOrder::Endianness, size_t nBits);

public:
    virtual size_t get_nBits() const;

    ByteOrder::Endianness get_minorOrder() const;

    ByteOrder::Endianness get_majorOrder() const;

    size_t get_majorNBytes() const;

public:
    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    enum {
        static_variant = V_SgAsmScalarType
    };

public:
    virtual ~SgAsmScalarType();

public:
    SgAsmScalarType();

protected:
    ByteOrder::Endianness p_minorOrder;
    ByteOrder::Endianness p_majorOrder;
    size_t p_majorNBytes;
    size_t p_nBits;
};

class SgAsmIntegerType : public SgAsmScalarType {
public:
    SgAsmIntegerType(ByteOrder::Endianness, size_t nBits, bool isSigned);

    bool get_isSigned() const;

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    enum {
        static_variant = V_SgAsmIntegerType
    };

public:
    virtual ~SgAsmIntegerType();

public:
    SgAsmIntegerType();

protected:
    bool p_isSigned;
};

class SgAsmFloatType : public SgAsmScalarType {
public:
    enum {
        GRADUAL_UNDERFLOW = 0x00000001,
        NORMALIZED_SIGNIFICAND = 0x00000002
    };

    typedef Sawyer::Container::BitVector::BitRange BitRange;

    SgAsmFloatType(ByteOrder::Endianness, size_t nBits,
                   const BitRange &significandBits, const BitRange exponentBits, size_t signBit,
                   uint64_t exponentBias, unsigned flags);

    // These return the position of the significand, exponent, and sign bit fields.
    BitRange significandBits() const;

    BitRange exponentBits() const;

    size_t signBit() const;

    uint64_t exponentBias() const;

    // Boolean properties (same names as their enum constants)
    unsigned flags() const;

    bool gradualUnderflow() const;

    bool normalizedSignificand() const;

    /*! \brief returns a string representing the class name */
    virtual std::string class_name() const;

    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const; // MS: new variant used in tree traversal

    /*! \brief static variant value */
    enum {
        static_variant = V_SgAsmFloatType
    };

public:
    virtual ~SgAsmFloatType();

public:
    SgAsmFloatType();

protected:
    size_t p_significandOffset;
    size_t p_significandNBits;
    size_t p_signBitOffset;
    size_t p_exponentOffset;
    size_t p_exponentNBits;
    uint64_t p_exponentBias;
    unsigned p_flags;
};

#endif
