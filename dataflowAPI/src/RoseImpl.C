/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <string>
#include <map>
#include <iostream>

#include "../rose/SgAsmx86Instruction.h"
#include "../rose/SgAsmPowerpcInstruction.h"
#include "../rose/SgAsmArmv8Instruction.h"
#include "../rose/SgAsmAMDGPUInstruction.h"
#include "external/rose/rose-compat.h"
#include "../rose/RegisterDescriptor.h"
#include "../rose/x86InstructionSemantics.h"

// defs for SgAsmNode
std::string SgAsmNode::class_name() const {
    return "SgAsmNode";
}

VariantT SgAsmNode::variantT() const {
    return V_SgAsmNode;
}

SgAsmNode::~SgAsmNode() {

}

SgAsmNode::SgAsmNode() :
        SgNode() {

}

// defs for SgAsmType
SgAsmType::SgAsmType() {

}

std::string SgAsmType::class_name() const {
    return "SgAsmType";
}

VariantT SgAsmType::variantT() const {
    return V_SgAsmType;
}

SgAsmType::~SgAsmType() {

}

// defs for SgAsmTypeByte
SgAsmTypeByte::SgAsmTypeByte() {

}

std::string SgAsmTypeByte::class_name() const {
    return "SgAsmTypeByte";
}

VariantT SgAsmTypeByte::variantT() const {
    return static_variant;
}

SgAsmTypeByte::~SgAsmTypeByte() {

}

// defs for SgAsmTypeWord
SgAsmTypeWord::SgAsmTypeWord() {

}

std::string SgAsmTypeWord::class_name() const {
    return "SgAsmTypeWord";
}

VariantT SgAsmTypeWord::variantT() const {
    return static_variant;
}

SgAsmTypeWord::~SgAsmTypeWord() {

}

// defs for SgAsmTypeDoubleWord
SgAsmTypeDoubleWord::SgAsmTypeDoubleWord() {

}

std::string SgAsmTypeDoubleWord::class_name() const {
    return "SgAsmTypeDoubleWord";
}

VariantT SgAsmTypeDoubleWord::variantT() const {
    return static_variant;
}

SgAsmTypeDoubleWord::~SgAsmTypeDoubleWord() {

}

// defs for SgAsmTypeQuadWord
SgAsmTypeQuadWord::SgAsmTypeQuadWord() {

}

std::string SgAsmTypeQuadWord::class_name() const {
    return "SgAsmTypeQuadWord";
}

VariantT SgAsmTypeQuadWord::variantT() const {
    return static_variant;
}

SgAsmTypeQuadWord::~SgAsmTypeQuadWord() {

}

// defs for SgAsmTypeSingleFloat
SgAsmTypeSingleFloat::SgAsmTypeSingleFloat() {

}

std::string SgAsmTypeSingleFloat::class_name() const {
    return "SgAsmTypeSingleFloat";
}

VariantT SgAsmTypeSingleFloat::variantT() const {
    return static_variant;
}

SgAsmTypeSingleFloat::~SgAsmTypeSingleFloat() {

}

// defs for SgAsmTypeDoubleFloat
SgAsmTypeDoubleFloat::SgAsmTypeDoubleFloat() {

}

std::string SgAsmTypeDoubleFloat::class_name() const {
    return "SgAsmTypeDoubleFloat";
}

VariantT SgAsmTypeDoubleFloat::variantT() const {
    return static_variant;
}

SgAsmTypeDoubleFloat::~SgAsmTypeDoubleFloat() {

}

// defs for SgAsmScalarType
std::string SgAsmScalarType::class_name() const {
    return "SgAsmScalarType";
}

VariantT SgAsmScalarType::variantT() const {
    return V_SgAsmScalarType;
}

SgAsmScalarType::~SgAsmScalarType() {
    p_minorOrder = ByteOrder::ORDER_UNSPECIFIED;
    p_majorOrder = ByteOrder::ORDER_UNSPECIFIED;
    p_majorNBytes = 0;
    p_nBits = 0;
}

SgAsmScalarType::SgAsmScalarType() :
        SgAsmType() {
    p_minorOrder = ByteOrder::ORDER_UNSPECIFIED;
    p_majorOrder = ByteOrder::ORDER_UNSPECIFIED;
    p_majorNBytes = 0;
    p_nBits = 0;
}

SgAsmScalarType::SgAsmScalarType(ByteOrder::Endianness sex, size_t nBits)
        : p_minorOrder(sex), p_majorOrder(ByteOrder::ORDER_UNSPECIFIED), p_majorNBytes(0), p_nBits(nBits) {
    size_t nBytes = (get_nBits() + 7) / 8;
    if (p_nBits <= 8)
        p_minorOrder = ByteOrder::ORDER_UNSPECIFIED;
    if (p_majorNBytes == 0 || p_majorNBytes <= nBytes)
        p_majorOrder = ByteOrder::ORDER_UNSPECIFIED;
}

size_t SgAsmScalarType::get_nBits() const {
    return p_nBits;
}

ByteOrder::Endianness SgAsmScalarType::get_minorOrder() const {
    return p_minorOrder;
}

ByteOrder::Endianness SgAsmScalarType::get_majorOrder() const {
    return p_majorOrder;
}

size_t SgAsmScalarType::get_majorNBytes() const {
    return p_majorNBytes;
}

// defs for SgAsmIntegerType
std::string SgAsmIntegerType::class_name() const {
    return "SgAsmIntegerType";
}

VariantT SgAsmIntegerType::variantT() const {
    return V_SgAsmIntegerType;
}

SgAsmIntegerType::~SgAsmIntegerType() {
    p_isSigned = false;
}

SgAsmIntegerType::SgAsmIntegerType() {
    p_isSigned = false;
}

SgAsmIntegerType::SgAsmIntegerType(ByteOrder::Endianness sex, size_t nBits, bool isSigned)
        : SgAsmScalarType(sex, nBits), p_isSigned(isSigned) {
    if (1 == nBits)
        isSigned = false;
}

bool
SgAsmIntegerType::get_isSigned() const {
    return p_isSigned;
}

// defs for SgAsmFloatType
std::string SgAsmFloatType::class_name() const {
    return "SgAsmFloatType";
}

VariantT SgAsmFloatType::variantT() const {
    return V_SgAsmFloatType;
}

SgAsmFloatType::SgAsmFloatType() :
        SgAsmScalarType() {
    p_significandOffset = (size_t)(-1);
    p_significandNBits = (size_t)(-1);
    p_signBitOffset = (size_t)(-1);
    p_exponentOffset = (size_t)(-1);
    p_exponentNBits = (size_t)(-1);
    p_exponentBias = 0;
    p_flags = 0;
}

SgAsmFloatType::~SgAsmFloatType() {
    p_significandOffset = (size_t)(-1);
    p_significandNBits = (size_t)(-1);
    p_signBitOffset = (size_t)(-1);
    p_exponentOffset = (size_t)(-1);
    p_exponentNBits = (size_t)(-1);
    p_exponentBias = 0;
    p_flags = 0;
}

SgAsmFloatType::SgAsmFloatType(ByteOrder::Endianness sex, size_t nBits,
                               const BitRange &significandBits, const BitRange exponentBits, size_t signBit,
                               uint64_t exponentBias, unsigned flags)
        : SgAsmScalarType(sex, nBits), p_signBitOffset(signBit), p_exponentBias(exponentBias), p_flags(flags) {
    p_significandOffset = significandBits.least();
    p_significandNBits = significandBits.size();
    p_exponentOffset = exponentBits.least();
    p_exponentNBits = exponentBits.size();
}

SgAsmFloatType::BitRange
SgAsmFloatType::significandBits() const {
    return BitRange::baseSize(p_significandOffset, p_significandNBits);
}

SgAsmFloatType::BitRange
SgAsmFloatType::exponentBits() const {
    return BitRange::baseSize(p_exponentOffset, p_exponentNBits);
}

size_t
SgAsmFloatType::signBit() const {
    return p_signBitOffset;
}

uint64_t
SgAsmFloatType::exponentBias() const {
    return p_exponentBias;
}

unsigned
SgAsmFloatType::flags() const {
    return p_flags;
}

bool
SgAsmFloatType::gradualUnderflow() const {
    return 0 != (p_flags & GRADUAL_UNDERFLOW);
}

bool
SgAsmFloatType::normalizedSignificand() const {
    return 0 != (p_flags & NORMALIZED_SIGNIFICAND);
}

// defs for SgAsmExpression
SgAsmExpression::SgAsmExpression() :
        SgAsmNode() {
    p_type = NULL;
    p_replacement = "";
    p_comment = "";
}

std::string SgAsmExpression::class_name() const {
    return "SgAsmExpression";
}

VariantT SgAsmExpression::variantT() const {
    return V_SgAsmExpression;
}

SgAsmType *SgAsmExpression::get_type() const {
    return p_type;
}

void SgAsmExpression::set_type(SgAsmType *type) {
    p_type = type;
}

size_t SgAsmExpression::get_nBits() const {
    SgAsmType *type = get_type();
    ASSERT_not_null2(type, "expression has no type");
    return type->get_nBits();
}

std::string SgAsmExpression::get_replacement() const {
    return p_replacement;
}

void SgAsmExpression::set_replacement(std::string replacement) {
    p_replacement = replacement;
}

std::string SgAsmExpression::get_comment() const {
    return p_comment;
}

void SgAsmExpression::set_comment(std::string comment) {
    p_comment = comment;
}

SgAsmExpression::~SgAsmExpression() {
    p_type = NULL;
    p_replacement = "";
    p_comment = "";
}

// defs for SgAsmValueExpression
SgAsmValueExpression::SgAsmValueExpression() :
        SgAsmExpression() {
    p_unfolded_expression_tree = NULL;
    p_bit_offset = 0;
    p_bit_size = 0;
}

SgAsmType *SgAsmValueExpression::get_type() const {
    return new SgAsmType();
}

std::string SgAsmValueExpression::class_name() const {
    return "SgAsmValueExpression";
}

VariantT SgAsmValueExpression::variantT() const {
    return V_SgAsmValueExpression;
}

SgAsmValueExpression::~SgAsmValueExpression() {
    p_unfolded_expression_tree = NULL;
    p_bit_offset = 0;
    p_bit_size = 0;
}

unsigned short SgAsmValueExpression::get_bit_offset() const {
    return p_bit_offset;
}

void SgAsmValueExpression::set_bit_offset(unsigned short bit_offset) {
    p_bit_offset = bit_offset;
}

unsigned short SgAsmValueExpression::get_bit_size() const {
    return p_bit_size;
}

void SgAsmValueExpression::set_bit_size(unsigned short bit_size) {
    p_bit_size = bit_size;
}

SgAsmValueExpression *SgAsmValueExpression::get_unfolded_expression_tree() const {
    return p_unfolded_expression_tree;
}

void SgAsmValueExpression::set_unfolded_expression_tree(SgAsmValueExpression *unfolded_expression_tree) {
    p_unfolded_expression_tree = unfolded_expression_tree;
}

// defs for SgAsmByteValueExpression
SgAsmByteValueExpression::SgAsmByteValueExpression(unsigned char value) {
    p_value = value;
}

SgAsmType *SgAsmByteValueExpression::get_type() const {
    return new SgAsmTypeByte();
}

std::string SgAsmByteValueExpression::class_name() const {
    return "SgAsmByteValueExpression";
}

VariantT SgAsmByteValueExpression::variantT() const {
    return static_variant;
}

uint8_t SgAsmByteValueExpression::get_value() const {
    return p_value;
}

SgAsmByteValueExpression::~SgAsmByteValueExpression() {

}

// defs for SgAsmWordValueExpression
SgAsmWordValueExpression::SgAsmWordValueExpression(unsigned short value) {
    p_value = value;
}

SgAsmType *SgAsmWordValueExpression::get_type() const {
    return new SgAsmTypeWord();
}

std::string SgAsmWordValueExpression::class_name() const {
    return "SgAsmWordValueExpression";
}

VariantT SgAsmWordValueExpression::variantT() const {
    return static_variant;
}

uint16_t SgAsmWordValueExpression::get_value() const {
    return p_value;
}

SgAsmWordValueExpression::~SgAsmWordValueExpression() {

}

// defs for SgAsmDoubleWordValueExpression
SgAsmDoubleWordValueExpression::SgAsmDoubleWordValueExpression(unsigned int value) {
    p_value = value;
}

SgAsmType *SgAsmDoubleWordValueExpression::get_type() const {
    return new SgAsmTypeDoubleWord();
}

std::string SgAsmDoubleWordValueExpression::class_name() const {
    return "SgAsmDoubleWordValueExpression";
}

VariantT SgAsmDoubleWordValueExpression::variantT() const {
    return static_variant;
}

uint32_t SgAsmDoubleWordValueExpression::get_value() const {
    return p_value;
}

SgAsmDoubleWordValueExpression::~SgAsmDoubleWordValueExpression() {

}

// defs for SgAsmQuadWordValueExpression
SgAsmQuadWordValueExpression::SgAsmQuadWordValueExpression(uint64_t value) {
    p_value = value;
}

SgAsmType *SgAsmQuadWordValueExpression::get_type() const {
    return new SgAsmTypeQuadWord();
}

std::string SgAsmQuadWordValueExpression::class_name() const {
    return "SgAsmQuadWordValueExpression";
}

VariantT SgAsmQuadWordValueExpression::variantT() const {
    return static_variant;
}

uint64_t SgAsmQuadWordValueExpression::get_value() const {
    return p_value;
}

SgAsmQuadWordValueExpression::~SgAsmQuadWordValueExpression() {

}

// defs for SgAsmSingleFloatValueExpression
SgAsmSingleFloatValueExpression::SgAsmSingleFloatValueExpression(float value) {
    p_value = value;
}

SgAsmType *SgAsmSingleFloatValueExpression::get_type() const {
    return new SgAsmTypeSingleFloat();
}

std::string SgAsmSingleFloatValueExpression::class_name() const {
    return "SgAsmSingleFloatValueExpression";
}

VariantT SgAsmSingleFloatValueExpression::variantT() const {
    return static_variant;
}

SgAsmSingleFloatValueExpression::~SgAsmSingleFloatValueExpression() {

}

SgAsmDoubleFloatValueExpression::SgAsmDoubleFloatValueExpression(double value) {
    p_value = value;
}

// defs for SgAsmDoubleFloatValueExpression
SgAsmType *SgAsmDoubleFloatValueExpression::get_type() const {
    return new SgAsmTypeDoubleFloat();
}

std::string SgAsmDoubleFloatValueExpression::class_name() const {
    return "SgAsmDoubleFloatValueExpression";
}

VariantT SgAsmDoubleFloatValueExpression::variantT() const {
    return static_variant;
}

SgAsmDoubleFloatValueExpression::~SgAsmDoubleFloatValueExpression() {

}

// defs for SgAsmConstantExpression
std::string SgAsmConstantExpression::class_name() const {
    return "SgAsmConstantExpression";
}

VariantT SgAsmConstantExpression::variantT() const {
    return V_SgAsmConstantExpression;
}

SgAsmConstantExpression::SgAsmConstantExpression() :
        SgAsmValueExpression() {

}

SgAsmConstantExpression::~SgAsmConstantExpression() {

}

// defs for SgAsmIntegerValueExpression
std::string SgAsmIntegerValueExpression::class_name() const {
    return "SgAsmIntegerValueExpression";
}

VariantT SgAsmIntegerValueExpression::variantT() const {
    return V_SgAsmIntegerValueExpression;
}

SgAsmIntegerValueExpression::SgAsmIntegerValueExpression() :
        SgAsmConstantExpression() {
    p_baseNode = NULL;
}

SgAsmIntegerValueExpression::~SgAsmIntegerValueExpression() {
    p_baseNode = NULL;
}

SgNode *SgAsmIntegerValueExpression::get_baseNode() const {
    return p_baseNode;
}

void SgAsmIntegerValueExpression::set_baseNode(SgNode *baseNode) {
    p_baseNode = baseNode;
}

SgAsmIntegerValueExpression::SgAsmIntegerValueExpression(uint64_t value, SgAsmType *type)
        : p_baseNode(NULL) {
    set_type(type);
    p_bitVector.resize(type->get_nBits()).fromInteger(value);
}

SgAsmIntegerValueExpression::SgAsmIntegerValueExpression(const Sawyer::Container::BitVector &bv, SgAsmType *type)
        : p_baseNode(NULL) {
    set_type(type);
    p_bitVector = bv;
}

size_t
SgAsmIntegerValueExpression::get_significantBits() const {
    return p_bitVector.size();
}

void
SgAsmIntegerValueExpression::makeRelativeTo(SgNode *baseNode) {
    int64_t curValue = get_absoluteValue();
    //int64_t baseVa = virtualAddress(baseNode);
    int64_t baseVa = 0;
    int64_t offset = curValue - baseVa;
    set_baseNode(baseNode);

    // We don't want to change the size of the offset if we can help it.  But if we do need to widen it then use a power of two
    // bytes: 1, 2, 4, or 8
    size_t needWidth = 0;
    if (offset >= 0) {
        if ((uint64_t) offset > 0xffffffff) {
            needWidth = 64;
        } else if ((uint64_t) offset > 0xffff) {
            needWidth = 32;
        } else if ((uint64_t) offset > 0xff) {
            needWidth = 16;
        } else {
            needWidth = 8;
        }
    } else {
        if (offset < -4294967296ll) {
            needWidth = 64;
        } else if (offset < -65536) {
            needWidth = 32;
        } else if (offset < -256) {
            needWidth = 16;
        } else {
            needWidth = 8;
        }
    }

    size_t newWidth = std::max(get_significantBits(), needWidth);
    uint64_t uoffset = (uint64_t) offset & IntegerOps::genMask<uint64_t>(newWidth);
    set_relativeValue(uoffset, newWidth);
}

uint64_t
SgAsmIntegerValueExpression::get_baseAddress() const {
    //return virtualAddress(get_baseNode());
    return 0;
}

uint64_t
SgAsmIntegerValueExpression::get_absoluteValue(size_t nbits) const {
    if (0 == nbits)
        nbits = get_significantBits();
    uint64_t retval = get_baseAddress() + get_relativeValue();
    uint64_t mask = IntegerOps::genMask<uint64_t>(nbits);
    return retval & mask; // clear high-order bits
}

int64_t
SgAsmIntegerValueExpression::get_signedValue() const {
    int64_t retval = get_baseAddress() + get_relativeValue();
    return retval;
}

void
SgAsmIntegerValueExpression::set_absoluteValue(uint64_t v) {
    int64_t new_offset = v - get_baseAddress(); // mod 2^64
    set_relativeValue(new_offset);
}

int64_t
SgAsmIntegerValueExpression::get_relativeValue() const {
    size_t nbits = get_significantBits();
    uint64_t uv = p_bitVector.toInteger();
    int64_t sv = IntegerOps::signExtend2(uv, nbits, 8 * sizeof(int64_t));
    return sv;
}

void
SgAsmIntegerValueExpression::set_relativeValue(int64_t v, size_t nbits) {
    p_bitVector.resize(nbits).fromInteger(v);
}

// defs for SgAsmFloatValueExpression
std::string SgAsmFloatValueExpression::class_name() const {
    return "SgAsmFloatValueExpression";
}

VariantT SgAsmFloatValueExpression::variantT() const {
    return V_SgAsmFloatValueExpression;
}

SgAsmFloatValueExpression::~SgAsmFloatValueExpression() {

}

SgAsmFloatValueExpression::SgAsmFloatValueExpression(double value, SgAsmType * /*type*/) {
    p_nativeValue = value;
    p_nativeValueIsValid = true;
}

SgAsmFloatValueExpression::SgAsmFloatValueExpression(const Sawyer::Container::BitVector &bv, SgAsmType * /*type*/) {
    p_nativeValue = 0.0;
    p_nativeValueIsValid = false;
    p_bitVector = bv;
}

double
SgAsmFloatValueExpression::get_nativeValue() const {
    return p_nativeValue;
}

void
SgAsmFloatValueExpression::set_nativeValue(double value) {
    p_nativeValue = value;
    p_nativeValueIsValid = true;
}

// defs for SgAsmBinaryExpression
SgAsmBinaryExpression::SgAsmBinaryExpression(SgAsmExpression *lhs, SgAsmExpression *rhs) :
        SgAsmExpression() {
    p_lhs = lhs;
    p_rhs = rhs;
}

SgAsmType *SgAsmBinaryExpression::get_type() const {
    // TODO this might not be safe
    return p_lhs->get_type();
}

std::string SgAsmBinaryExpression::class_name() const {
    return "SgAsmBinaryExpression";
}

VariantT SgAsmBinaryExpression::variantT() const {
    return V_SgAsmBinaryExpression;
}

SgAsmExpression *SgAsmBinaryExpression::get_lhs() const {
    return p_lhs;
}

void SgAsmBinaryExpression::set_lhs(SgAsmExpression *lhs) {
    p_lhs = lhs;
}

SgAsmExpression *SgAsmBinaryExpression::get_rhs() const {
    return p_rhs;
}

void SgAsmBinaryExpression::set_rhs(SgAsmExpression *rhs) {
    p_rhs = rhs;
}

SgAsmBinaryExpression::~SgAsmBinaryExpression() {
    p_lhs = NULL;
    p_rhs = NULL;
}

// defs for SgAsmBinaryAdd
SgAsmBinaryAdd::SgAsmBinaryAdd(SgAsmExpression *lhs, SgAsmExpression *rhs)
        : SgAsmBinaryExpression(lhs, rhs) {

}

SgAsmType *SgAsmBinaryAdd::get_type() const {
    return SgAsmBinaryExpression::get_type();
}

std::string SgAsmBinaryAdd::class_name() const {
    return "SgAsmBinaryAdd";
}

VariantT SgAsmBinaryAdd::variantT() const {
    return V_SgAsmBinaryAdd;
}

SgAsmBinaryAdd::~SgAsmBinaryAdd() {

}

// defs for SgAsmBinarySubtract
SgAsmBinarySubtract::SgAsmBinarySubtract(SgAsmExpression *lhs, SgAsmExpression *rhs)
        : SgAsmBinaryExpression(lhs, rhs) {

}

std::string SgAsmBinarySubtract::class_name() const {
    return "SgAsmBinarySubtract";
}

VariantT SgAsmBinarySubtract::variantT() const {
    return V_SgAsmBinarySubtract;
}

SgAsmBinarySubtract::~SgAsmBinarySubtract() {

}

// defs for SgAsmBinaryMultiply
SgAsmBinaryMultiply::SgAsmBinaryMultiply(SgAsmExpression *lhs, SgAsmExpression *rhs)
        : SgAsmBinaryExpression(lhs, rhs) {

}

SgAsmType *SgAsmBinaryMultiply::get_type() const {
    return SgAsmBinaryExpression::get_type();
}

std::string SgAsmBinaryMultiply::class_name() const {
    return "SgAsmBinaryMultiply";
}

VariantT SgAsmBinaryMultiply::variantT() const {
    return V_SgAsmBinaryMultiply;
}

SgAsmBinaryMultiply::~SgAsmBinaryMultiply() {

}

// defs for SgAsmBinaryDivide
SgAsmBinaryDivide::SgAsmBinaryDivide(SgAsmExpression *lhs, SgAsmExpression *rhs)
        : SgAsmBinaryExpression(lhs, rhs) {

}

std::string SgAsmBinaryDivide::class_name() const {
    return "SgAsmBinaryDivide";
}

VariantT SgAsmBinaryDivide::variantT() const {
    return V_SgAsmBinaryDivide;
}

SgAsmBinaryDivide::~SgAsmBinaryDivide() {

}

// defs for SgAsmBinaryMod
SgAsmBinaryMod::SgAsmBinaryMod(SgAsmExpression *lhs, SgAsmExpression *rhs)
        : SgAsmBinaryExpression(lhs, rhs) {

}

std::string SgAsmBinaryMod::class_name() const {
    return "SgAsmBinaryMod";
}

VariantT SgAsmBinaryMod::variantT() const {
    return V_SgAsmBinaryMod;
}

SgAsmBinaryMod::~SgAsmBinaryMod() {

}

// defs for SgAsmBinaryAddPreupdate
SgAsmBinaryAddPreupdate::SgAsmBinaryAddPreupdate(SgAsmExpression *lhs, SgAsmExpression *rhs)
        : SgAsmBinaryExpression(lhs, rhs) {

}

std::string SgAsmBinaryAddPreupdate::class_name() const {
    return "SgAsmBinaryAddPreupdate";
}

VariantT SgAsmBinaryAddPreupdate::variantT() const {
    return V_SgAsmBinaryAddPreupdate;
}

SgAsmBinaryAddPreupdate::~SgAsmBinaryAddPreupdate() {

}

// defs for SgAsmBinarySubtractPreupdate
SgAsmBinarySubtractPreupdate::SgAsmBinarySubtractPreupdate(SgAsmExpression *lhs, SgAsmExpression *rhs)
        : SgAsmBinaryExpression(lhs, rhs) {

}

std::string SgAsmBinarySubtractPreupdate::class_name() const {
    return "SgAsmBinarySubtractPreupdate";
}

VariantT SgAsmBinarySubtractPreupdate::variantT() const {
    return V_SgAsmBinarySubtractPreupdate;
}

SgAsmBinarySubtractPreupdate::~SgAsmBinarySubtractPreupdate() {

}

// defs for SgAsmBinaryAddPostupdate
SgAsmBinaryAddPostupdate::SgAsmBinaryAddPostupdate(SgAsmExpression *lhs, SgAsmExpression *rhs)
        : SgAsmBinaryExpression(lhs, rhs) {

}

std::string SgAsmBinaryAddPostupdate::class_name() const {
    return "SgAsmBinaryAddPostupdate";
}

VariantT SgAsmBinaryAddPostupdate::variantT() const {
    return V_SgAsmBinaryAddPostupdate;
}

SgAsmBinaryAddPostupdate::~SgAsmBinaryAddPostupdate() {

}

// defs for SgAsmBinarySubtractPostupdate
SgAsmBinarySubtractPostupdate::SgAsmBinarySubtractPostupdate(SgAsmExpression *lhs, SgAsmExpression *rhs)
        : SgAsmBinaryExpression(lhs, rhs) {

}

std::string SgAsmBinarySubtractPostupdate::class_name() const {
    return "SgAsmBinarySubtractPostupdate";
}

VariantT SgAsmBinarySubtractPostupdate::variantT() const {
    return V_SgAsmBinarySubtractPostupdate;
}

SgAsmBinarySubtractPostupdate::~SgAsmBinarySubtractPostupdate() {

}

// defs for SgAsmBinaryLsl
SgAsmBinaryLsl::SgAsmBinaryLsl(SgAsmExpression *lhs, SgAsmExpression *rhs)
        : SgAsmBinaryExpression(lhs, rhs) {

}

std::string SgAsmBinaryLsl::class_name() const {
    return "SgAsmBinaryLsl";
}

VariantT SgAsmBinaryLsl::variantT() const {
    return V_SgAsmBinaryLsl;
}

SgAsmBinaryLsl::~SgAsmBinaryLsl() {

}

// defs for SgAsmBinaryLsr
SgAsmBinaryLsr::SgAsmBinaryLsr(SgAsmExpression *lhs, SgAsmExpression *rhs)
        : SgAsmBinaryExpression(lhs, rhs) {

}

std::string SgAsmBinaryLsr::class_name() const {
    return "SgAsmBinaryLsr";
}

VariantT SgAsmBinaryLsr::variantT() const {
    return V_SgAsmBinaryLsr;
}

SgAsmBinaryLsr::~SgAsmBinaryLsr() {

}

// defs for SgAsmBinaryAsr
SgAsmBinaryAsr::SgAsmBinaryAsr(SgAsmExpression *lhs, SgAsmExpression *rhs)
        : SgAsmBinaryExpression(lhs, rhs) {

}

std::string SgAsmBinaryAsr::class_name() const {
    return "SgAsmBinaryAsr";
}

VariantT SgAsmBinaryAsr::variantT() const {
    return V_SgAsmBinaryAsr;
}

SgAsmBinaryAsr::~SgAsmBinaryAsr() {

}

// defs for SgAsmBinaryRor
SgAsmBinaryRor::SgAsmBinaryRor(SgAsmExpression *lhs, SgAsmExpression *rhs)
        : SgAsmBinaryExpression(lhs, rhs) {

}

std::string SgAsmBinaryRor::class_name() const {
    return "SgAsmBinaryRor";
}

VariantT SgAsmBinaryRor::variantT() const {
    return V_SgAsmBinaryRor;
}

SgAsmBinaryRor::~SgAsmBinaryRor() {

}

// defs for SgAsmUnaryExpression
std::string SgAsmUnaryExpression::class_name() const {
    return "SgAsmUnaryExpression";
}

VariantT SgAsmUnaryExpression::variantT() const {
    return V_SgAsmUnaryExpression;
}

SgAsmUnaryExpression::SgAsmUnaryExpression(SgAsmExpression *operand) :
        SgAsmExpression() {
    p_operand = operand;
}

SgAsmUnaryExpression::~SgAsmUnaryExpression() {
    p_operand = NULL;
}

void SgAsmUnaryExpression::set_operand(SgAsmExpression *operand) {
    p_operand = operand;
}

SgAsmExpression *SgAsmUnaryExpression::get_operand() const {
    return p_operand;
}

// defs for SgAsmUnaryPlus
std::string SgAsmUnaryPlus::class_name() const {
    return "SgAsmUnaryPlus";
}

VariantT SgAsmUnaryPlus::variantT() const {
    return V_SgAsmUnaryPlus;
}

SgAsmUnaryPlus::SgAsmUnaryPlus(SgAsmExpression *operand) :
        SgAsmUnaryExpression(operand) {

}

SgAsmUnaryPlus::~SgAsmUnaryPlus() {

}

// defs for SgAsmUnaryMinus
std::string SgAsmUnaryMinus::class_name() const {
    return "SgAsmUnaryMinus";
}

VariantT SgAsmUnaryMinus::variantT() const {
    return V_SgAsmUnaryMinus;
}

SgAsmUnaryMinus::SgAsmUnaryMinus(SgAsmExpression *operand) :
        SgAsmUnaryExpression(operand) {

}

SgAsmUnaryMinus::~SgAsmUnaryMinus() {

}

// defs for SgAsmUnaryRrx
std::string SgAsmUnaryRrx::class_name() const {
    return "SgAsmUnaryRrx";
}

VariantT SgAsmUnaryRrx::variantT() const {
    return V_SgAsmUnaryRrx;
}

SgAsmUnaryRrx::SgAsmUnaryRrx(SgAsmExpression *operand) :
        SgAsmUnaryExpression(operand) {

}

SgAsmUnaryRrx::~SgAsmUnaryRrx() {

}

// defs for SgAsmMemoryReferenceExpression
SgAsmMemoryReferenceExpression::SgAsmMemoryReferenceExpression(SgAsmExpression *address, SgAsmExpression *segment) :
        SgAsmExpression() {
    p_address = address;
    p_segment = segment;
    p_type = NULL;
}

SgAsmType *SgAsmMemoryReferenceExpression::get_type() const {
    return p_type;
}

std::string SgAsmMemoryReferenceExpression::class_name() const {
    return "SgAsmMemoryReferenceExpression";
}

VariantT SgAsmMemoryReferenceExpression::variantT() const {
    return V_SgAsmMemoryReferenceExpression;
}

SgAsmExpression *SgAsmMemoryReferenceExpression::get_address() const {
    return p_address;
}

SgAsmExpression *SgAsmMemoryReferenceExpression::get_segment() const {
    return p_segment;
}

void SgAsmMemoryReferenceExpression::set_type(SgAsmType *type) {
    p_type = type;
}

void SgAsmMemoryReferenceExpression::set_address(SgAsmExpression *address) {
    p_address = address;
}

void SgAsmMemoryReferenceExpression::set_segment(SgAsmExpression *segment) {
    p_segment = segment;
}

SgAsmMemoryReferenceExpression::~SgAsmMemoryReferenceExpression() {
    p_address = NULL;
    p_segment = NULL;
    p_type = NULL;
}

// defs for SgAsmRegisterReferenceExpression
SgAsmRegisterReferenceExpression::SgAsmRegisterReferenceExpression() {
    p_type = NULL;
    p_adjustment = 0;
}

SgAsmType *SgAsmRegisterReferenceExpression::get_type() const {
    return p_type;
}

void SgAsmRegisterReferenceExpression::set_type(SgAsmType *type) {
    p_type = type;
}

std::string SgAsmRegisterReferenceExpression::class_name() const {
    return "SgAsmRegisterReferenceExpression";
}

VariantT SgAsmRegisterReferenceExpression::variantT() const {
    return V_SgAsmRegisterReferenceExpression;
}

SgAsmRegisterReferenceExpression::~SgAsmRegisterReferenceExpression() {
    p_type = NULL;
    p_adjustment = 0;
}

int SgAsmRegisterReferenceExpression::get_adjustment() const {
    return p_adjustment;
}

void SgAsmRegisterReferenceExpression::set_adjustment(int adjustment) {
    p_adjustment = adjustment;
}

RegisterDescriptor SgAsmRegisterReferenceExpression::get_descriptor() const {
    return p_descriptor;
}

void SgAsmRegisterReferenceExpression::set_descriptor(RegisterDescriptor
                                                      descriptor) {
    p_descriptor = descriptor;
}

SgAsmRegisterReferenceExpression::SgAsmRegisterReferenceExpression(RegisterDescriptor
                                                                   descriptor) :

        SgAsmExpression() {
    p_descriptor = descriptor;
    p_adjustment = 0;
}

// defs for SgAsmDirectRegisterExpression
std::string SgAsmDirectRegisterExpression::class_name() const {
    return "SgAsmDirectRegisterExpression";
}

VariantT SgAsmDirectRegisterExpression::variantT() const {
    return V_SgAsmDirectRegisterExpression;
}

SgAsmDirectRegisterExpression::SgAsmDirectRegisterExpression(RegisterDescriptor descriptor) :
        SgAsmRegisterReferenceExpression(descriptor) {

}

SgAsmDirectRegisterExpression::~SgAsmDirectRegisterExpression() {

}

// defs for SgAsmIndirectRegisterExpression
SgAsmIndirectRegisterExpression::SgAsmIndirectRegisterExpression(RegisterDescriptor descriptor,
                                                                 RegisterDescriptor stride, RegisterDescriptor offset,
                                                                 size_t index, size_t modulus) : SgAsmRegisterReferenceExpression(descriptor) {
    p_stride = stride;
    p_offset = offset;
    p_index = index;
    p_modulus = modulus;
}

SgAsmIndirectRegisterExpression::~SgAsmIndirectRegisterExpression() {

}

std::string SgAsmIndirectRegisterExpression::class_name() const {
    return "SgAsmIndirectRegisterExpression";
}

VariantT SgAsmIndirectRegisterExpression::variantT() const {
    return V_SgAsmIndirectRegisterExpression;
}

RegisterDescriptor SgAsmIndirectRegisterExpression::get_stride() const {
    return p_stride;
}

void SgAsmIndirectRegisterExpression::set_stride(RegisterDescriptor stride) {
    p_stride = stride;
}

RegisterDescriptor SgAsmIndirectRegisterExpression::get_offset() const {
    return p_offset;
}

void SgAsmIndirectRegisterExpression::set_offset(RegisterDescriptor offset) {
    p_offset = offset;
}

size_t SgAsmIndirectRegisterExpression::get_index() const {
    return p_index;
}

void SgAsmIndirectRegisterExpression::set_index(size_t index) {
    p_index = index;
}

size_t SgAsmIndirectRegisterExpression::get_modulus() const {
    return p_modulus;
}

void SgAsmIndirectRegisterExpression::set_modulus(size_t modulus) {
    p_modulus = modulus;
}

// defs for SgAsmStatement
std::string SgAsmStatement::class_name() const {
    return "SgAsmStatement";
}

VariantT SgAsmStatement::variantT() const {
    return V_SgAsmStatement;
}

std::string SgAsmStatement::get_comment() const {
    return p_comment;
}

void SgAsmStatement::set_comment(std::string comment) {
    p_comment = comment;
}

rose_addr_t SgAsmStatement::get_address() const {
    return p_address;
}

void SgAsmStatement::set_address(rose_addr_t address) {
    p_address = address;
}

SgAsmStatement::SgAsmStatement(rose_addr_t address) :
        SgAsmNode() {
    p_address = address;
    p_comment = "";
}

SgAsmStatement::~SgAsmStatement() {
    p_address = 0;
    p_comment = "";
}

// defs for SgAsmInstruction
std::string SgAsmInstruction::class_name() const {
    return "SgAsmInstruction";
}

VariantT SgAsmInstruction::variantT() const {
    return V_SgAsmInstruction;
}

SgAsmInstruction::SgAsmInstruction(rose_addr_t address, std::string mnemonic) :
        SgAsmStatement(address) {
    p_mnemonic = mnemonic;
    p_operandList = NULL;
}

SgAsmInstruction::~SgAsmInstruction() {
    p_mnemonic = "";
    p_operandList = NULL;
}

SgAsmOperandList *SgAsmInstruction::get_operandList() const {
    return p_operandList;
}

std::string SgAsmInstruction::get_mnemonic() const {
    return p_mnemonic;
}

SgUnsignedCharList SgAsmInstruction::get_raw_bytes() const {
    return p_raw_bytes;
}

void SgAsmInstruction::set_operandList(SgAsmOperandList *operandList) {
    p_operandList = operandList;
}

void SgAsmInstruction::set_mnemonic(std::string mnemonic) {
    p_mnemonic = mnemonic;
}

void SgAsmInstruction::set_raw_bytes(SgUnsignedCharList raw_bytes) {
    p_raw_bytes = raw_bytes;
}

size_t SgAsmInstruction::get_size() const {
    return p_raw_bytes.size();
}

// defs for SgAsmAMDGPUInstruction
SgAsmAMDGPUInstruction *isSgAsmAMDGPUInstruction(SgNode *s) {
    return dynamic_cast<SgAsmAMDGPUInstruction *>(s);
}

std::string SgAsmAMDGPUInstruction::class_name() const {
    return "SgAsmAMDGPUGfx90aInstruction";
}

VariantT SgAsmAMDGPUInstruction::variantT() const {
    return V_SgAsmAMDGPUInstruction;
}

SgAsmAMDGPUInstruction::SgAsmAMDGPUInstruction(rose_addr_t address, std::string mnemonic, AMDGPUInstructionKind kind) :
        SgAsmInstruction(address, mnemonic) {
    p_kind = kind;
}

AMDGPUInstructionKind SgAsmAMDGPUInstruction::get_kind() const {
    return p_kind;
}

void SgAsmAMDGPUInstruction::set_kind(AMDGPUInstructionKind kind) {
    p_kind = kind;
}

SgAsmAMDGPUInstruction::~SgAsmAMDGPUInstruction() {
    p_kind = rose_amdgpu_op_INVALID;
}


// defs for SgAsmArmv8Instruction
std::string SgAsmArmv8Instruction::class_name() const {
    return "SgAsmArmv8Instruction";
}

VariantT SgAsmArmv8Instruction::variantT() const {
    return V_SgAsmArmv8Instruction;
}

SgAsmArmv8Instruction::SgAsmArmv8Instruction(rose_addr_t address, std::string mnemonic, ARMv8InstructionKind kind) :
        SgAsmInstruction(address, mnemonic) {
    p_kind = kind;
}

ARMv8InstructionKind SgAsmArmv8Instruction::get_kind() const {
    return p_kind;
}

void SgAsmArmv8Instruction::set_kind(ARMv8InstructionKind kind) {
    p_kind = kind;
}

SgAsmArmv8Instruction::~SgAsmArmv8Instruction() {
    p_kind = rose_aarch64_op_INVALID;
}

SgAsmx86RegisterReferenceExpression::SgAsmx86RegisterReferenceExpression(X86RegisterClass register_class,
                                                                         int register_number,
                                                                         X86PositionInRegister position_in_register) {
    p_register_class = register_class;
    p_register_number = register_number;
    p_position_in_register = position_in_register;

    // Need to fix for 64-bit
    // Right now semantics only accepts byte, word, doubleword

    switch (position_in_register) {
        case x86_regpos_low_byte:
        case x86_regpos_high_byte:
            p_type = new SgAsmTypeByte();
            break;
        case x86_regpos_word:
            p_type = new SgAsmTypeWord();
            break;
        case x86_regpos_all:
        case x86_regpos_dword:
            p_type = new SgAsmTypeDoubleWord();
            break;
        case x86_regpos_qword:
            p_type = new SgAsmTypeQuadWord();
            break;
        default:
            p_type = 0;
    }
}


std::string SgAsmx86RegisterReferenceExpression::class_name() const {
    return "SgAsmx86RegisterReferenceExpression";
}

VariantT SgAsmx86RegisterReferenceExpression::variantT() const {
    return static_variant;
}

int SgAsmx86RegisterReferenceExpression::get_register_number() const {
    return p_register_number;
}

X86RegisterClass SgAsmx86RegisterReferenceExpression::get_register_class() const {
    return p_register_class;
}

X86PositionInRegister SgAsmx86RegisterReferenceExpression::get_position_in_register() const {
    return p_position_in_register;
}

SgAsmx86RegisterReferenceExpression::~SgAsmx86RegisterReferenceExpression() {

}

uint64_t getAsmSignedConstant(SgAsmValueExpression *valexp) {
    switch (valexp->variantT()) {
        case V_SgAsmByteValueExpression:
            return (uint64_t)((int8_t)(static_cast<SgAsmByteValueExpression *>(valexp)->get_value()));
        case V_SgAsmWordValueExpression:
            return (uint64_t)((int16_t)(static_cast<SgAsmWordValueExpression *>(valexp)->get_value()));
        case V_SgAsmDoubleWordValueExpression:
            return (uint64_t)((int32_t)(static_cast<SgAsmDoubleWordValueExpression *>(valexp)->get_value()));
        case V_SgAsmQuadWordValueExpression:
            return (uint64_t)((int64_t)(static_cast<SgAsmQuadWordValueExpression *>(valexp)->get_value()));
        case V_SgAsmIntegerValueExpression:
            return (uint64_t)(static_cast<SgAsmIntegerValueExpression *>(valexp)->get_value());
        default:
            return 0; // error
    }
}

// conversions.h

uint64_t SageInterface::getAsmSignedConstant(SgAsmValueExpression *valexp) {
    return ::getAsmSignedConstant(valexp);
}

SgAsmExpression *isSgAsmExpression(SgNode *s) {
    return dynamic_cast<SgAsmExpression *>(s);
}

SgAsmValueExpression *isSgAsmValueExpression(SgNode *s) {
    return dynamic_cast<SgAsmValueExpression *>(s);
}

SgAsmBinaryExpression *isSgAsmBinaryExpression(SgNode *s) {
    return dynamic_cast<SgAsmBinaryExpression *>(s);
}

SgAsmBinaryAdd *isSgAsmBinaryAdd(SgNode *s) {
    return dynamic_cast<SgAsmBinaryAdd *>(s);
}

SgAsmBinarySubtract *isSgAsmBinarySubtract(SgNode *s) {
    return dynamic_cast<SgAsmBinarySubtract *>(s);
}

SgAsmBinaryMultiply *isSgAsmBinaryMultiply(SgNode *s) {
    return dynamic_cast<SgAsmBinaryMultiply *>(s);
}

SgAsmBinaryDivide *isSgAsmBinaryDivide(SgNode *s) {
    return dynamic_cast<SgAsmBinaryDivide *>(s);
}

SgAsmBinaryMod *isSgAsmBinaryMod(SgNode *s) {
    return dynamic_cast<SgAsmBinaryMod *>(s);
}

SgAsmBinaryAddPreupdate *isSgAsmBinaryAddPreupdate(SgNode *s) {
    return dynamic_cast<SgAsmBinaryAddPreupdate *>(s);
}

SgAsmBinaryAddPostupdate *isSgAsmBinaryAddPostupdate(SgNode *s) {
    return dynamic_cast<SgAsmBinaryAddPostupdate *>(s);
}

SgAsmBinarySubtractPreupdate *isSgAsmBinarySubtractPreupdate(SgNode *s) {
    return dynamic_cast<SgAsmBinarySubtractPreupdate *>(s);
}

SgAsmBinarySubtractPostupdate *isSgAsmBinarySubtractPostupdate(SgNode *s) {
    return dynamic_cast<SgAsmBinarySubtractPostupdate *>(s);
}

SgAsmBinaryLsl *isSgAsmBinaryLsl(SgNode *s) {
    return dynamic_cast<SgAsmBinaryLsl *>(s);
}

SgAsmBinaryLsr *isSgAsmBinaryLsr(SgNode *s) {
    return dynamic_cast<SgAsmBinaryLsr *>(s);
}

SgAsmBinaryAsr *isSgAsmBinaryAsr(SgNode *s) {
    return dynamic_cast<SgAsmBinaryAsr *>(s);
}

SgAsmBinaryRor *isSgAsmBinaryRor(SgNode *s) {
    return dynamic_cast<SgAsmBinaryRor *>(s);
}

SgAsmUnaryExpression *isSgAsmUnaryExpression(SgNode *s) {
    return dynamic_cast<SgAsmUnaryExpression *>(s);
}

SgAsmUnaryPlus *isSgAsmUnaryPlus(SgNode *s) {
    return dynamic_cast<SgAsmUnaryPlus *>(s);
}

SgAsmUnaryMinus *isSgAsmUnaryMinus(SgNode *s) {
    return dynamic_cast<SgAsmUnaryMinus *>(s);
}

SgAsmUnaryRrx *isSgAsmUnaryRrx(SgNode *s) {
    return dynamic_cast<SgAsmUnaryRrx *>(s);
}

SgAsmMemoryReferenceExpression *isSgAsmMemoryReferenceExpression(SgNode *s) {
    return dynamic_cast<SgAsmMemoryReferenceExpression *>(s);
}

SgAsmRegisterReferenceExpression *isSgAsmRegisterReferenceExpression(SgNode *s) {
    return dynamic_cast<SgAsmRegisterReferenceExpression *>(s);
}

SgAsmDirectRegisterExpression *isSgAsmDirectRegisterExpression(SgNode *s) {
    return dynamic_cast<SgAsmDirectRegisterExpression *>(s);
}

SgAsmIndirectRegisterExpression *isSgAsmIndirectRegisterExpression(SgNode *s) {
    return dynamic_cast<SgAsmIndirectRegisterExpression *>(s);
}

SgAsmx86RegisterReferenceExpression *isSgAsmx86RegisterReferenceExpression(SgNode *n) {
    return dynamic_cast<SgAsmx86RegisterReferenceExpression *>(n);
}

SgAsmPowerpcRegisterReferenceExpression *isSgAsmPowerpcRegisterReferenceExpression(SgNode *s) {
    return dynamic_cast<SgAsmPowerpcRegisterReferenceExpression *>(s);
}

SgAsmByteValueExpression *isSgAsmByteValueExpression(SgNode *s) {
    return dynamic_cast<SgAsmByteValueExpression *>(s);
}

SgAsmWordValueExpression *isSgAsmWordValueExpression(SgNode *s) {
    return dynamic_cast<SgAsmWordValueExpression *>(s);
}

SgAsmDoubleWordValueExpression *isSgAsmDoubleWordValueExpression(SgNode *s) {
    return dynamic_cast<SgAsmDoubleWordValueExpression *>(s);
}

SgAsmQuadWordValueExpression *isSgAsmQuadWordValueExpression(SgNode *s) {
    return dynamic_cast<SgAsmQuadWordValueExpression *>(s);
}

SgAsmConstantExpression *isSgAsmConstantExpression(SgNode *s) {
    return dynamic_cast<SgAsmConstantExpression *>(s);
}

SgAsmIntegerValueExpression *isSgAsmIntegerValueExpression(SgNode *s) {
    return dynamic_cast<SgAsmIntegerValueExpression *>(s);
}

SgAsmFloatValueExpression *isSgAsmFloatValueExpression(SgNode *s) {
    return dynamic_cast<SgAsmFloatValueExpression *>(s);
}

SgAsmArmv8Instruction *isSgAsmArmv8Instruction(SgNode *s) {
    return dynamic_cast<SgAsmArmv8Instruction *>(s);
}


SgAsmPowerpcInstruction *isSgAsmPowerpcInstruction(SgNode *s) {
    return dynamic_cast<SgAsmPowerpcInstruction *>(s);
}

SgAsmNode *isSgAsmNode(SgNode *s) {
    return dynamic_cast<SgAsmNode *>(s);
}

const char *regclassToString(X86RegisterClass) {
    return "NOT IMPLEMENTED";
}

const char *regclassToString(PowerpcRegisterClass c) {
    switch (c) {
        case powerpc_regclass_gpr:
            return "GPR";
        case powerpc_regclass_spr:
            return "SPR";
        case powerpc_regclass_fpr:
            return "FPR";
        case powerpc_regclass_sr:
            return "SR";
        default:
            return "unexpected register class--not gpr, spr, segment, or fpr";
    }
}

// SgAsmx86Instruction.h
SgAsmPowerpcInstruction::SgAsmPowerpcInstruction(rose_addr_t address, std::string mnemonic,
                                                 PowerpcInstructionKind kind) :
        SgAsmInstruction(address, mnemonic) {
    p_kind = kind;
    p_operandList = NULL;
}

SgAsmx86Instruction::SgAsmx86Instruction(rose_addr_t address,
                                         std::string mnemonic,
                                         X86InstructionKind kind,
                                         X86InstructionSize baseSize,
                                         X86InstructionSize operandSize,
                                         X86InstructionSize addressSize) :
        SgAsmInstruction(address, mnemonic) {
    p_kind = kind;
    p_baseSize = baseSize;
    p_operandSize = operandSize;
    p_addressSize = addressSize;

    // these are as-yet unknown, but should still be initialized
    p_lockPrefix = false;
    p_repeatPrefix = x86_repeat_none;
    p_branchPrediction = x86_branch_prediction_none;
    p_segmentOverride = x86_segreg_none;
}

std::string SgAsmx86Instruction::class_name() const {
    return "SgAsmx86Instruction";
}

std::string SgAsmPowerpcInstruction::class_name() const {
    return "SgAsmPowerpcInstruction";
}

VariantT SgAsmx86Instruction::variantT() const {
    return static_variant;
}

VariantT SgAsmPowerpcInstruction::variantT() const {
    return static_variant;
}

SgAsmx86Instruction::~SgAsmx86Instruction() {

}

X86InstructionKind SgAsmx86Instruction::get_kind() const {
    return p_kind;
}

PowerpcInstructionKind SgAsmPowerpcInstruction::get_kind() const {
    return p_kind;
}

X86InstructionSize SgAsmx86Instruction::get_addressSize() const {
    return p_addressSize;
}

X86InstructionSize SgAsmx86Instruction::get_operandSize() const {
    return p_operandSize;
}

X86SegmentRegister SgAsmx86Instruction::get_segmentOverride() const {
    return p_segmentOverride;
}

void SgAsmx86Instruction::set_kind(X86InstructionKind kind) {
    p_kind = kind;
}

void SgAsmPowerpcInstruction::set_kind(PowerpcInstructionKind kind) {
    p_kind = kind;
}

void SgAsmx86Instruction::set_addressSize(X86InstructionSize size) {
    p_addressSize = size;
}

void SgAsmx86Instruction::set_operandSize(X86InstructionSize size) {
    p_operandSize = size;
}

// SgAsmOperandList.h

SgAsmOperandList::SgAsmOperandList() :
        SgAsmNode() {

}

std::string SgAsmOperandList::class_name() const {
    return "SgAsmOperandList";
}

VariantT SgAsmOperandList::variantT() const {
    return V_SgAsmOperandList;
}

SgAsmExpressionPtrList &SgAsmOperandList::get_operands() {
    return p_operands;
}

void SgAsmOperandList::append_operand(SgAsmExpression *operand) {
    p_operands.push_back(operand);
}

SgAsmOperandList::~SgAsmOperandList() {

}

//        PowerpcRegisterClass p_register_class;
//        int p_register_number;
//        PowerpcConditionRegisterAccessGranularity p_conditionRegisterGranularity;

PowerpcRegisterClass SgAsmPowerpcRegisterReferenceExpression::get_register_class() const {
    return p_register_class;
}

void SgAsmPowerpcRegisterReferenceExpression::set_register_class(PowerpcRegisterClass register_class) {
    p_register_class = register_class;
}

int SgAsmPowerpcRegisterReferenceExpression::get_register_number() const {
    return p_register_number;
}

void SgAsmPowerpcRegisterReferenceExpression::set_register_number(int register_number) {
    p_register_number = register_number;
}

PowerpcConditionRegisterAccessGranularity
SgAsmPowerpcRegisterReferenceExpression::get_conditionRegisterGranularity() const {
    return p_conditionRegisterGranularity;
}

void SgAsmPowerpcRegisterReferenceExpression::set_conditionRegisterGranularity(
        PowerpcConditionRegisterAccessGranularity conditionRegisterGranularity) {
    p_conditionRegisterGranularity = conditionRegisterGranularity;
}

SgAsmPowerpcRegisterReferenceExpression::~SgAsmPowerpcRegisterReferenceExpression() {
}

SgAsmPowerpcRegisterReferenceExpression::SgAsmPowerpcRegisterReferenceExpression(
        PowerpcRegisterClass register_class,
        int register_number,
        PowerpcConditionRegisterAccessGranularity conditionRegisterGranularity)
        : p_register_class(register_class), p_register_number(register_number),
          p_conditionRegisterGranularity(conditionRegisterGranularity) {
}
