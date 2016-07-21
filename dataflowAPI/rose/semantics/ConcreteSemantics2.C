#include "../util/StringUtility.h"
//#include "sage3basic.h"
#include "ConcreteSemantics2.h"
#include "../integerOps.h"
#include "../util/BitVectorSupport.h"

using namespace Sawyer::Container;
typedef Sawyer::Container::BitVector::BitRange BitRange;

namespace rose {
namespace BinaryAnalysis {
namespace InstructionSemantics2 {
namespace ConcreteSemantics {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      SValue
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Sawyer::Optional<BaseSemantics::SValuePtr>
SValue::createOptionalMerge(const BaseSemantics::SValuePtr &other_, const BaseSemantics::MergerPtr&, SMTSolver*) const {
    // There's no official way to represent BOTTOM
    throw BaseSemantics::NotImplemented("SValue merging for ConcreteSemantics is not supported", NULL);
}

void
SValue::bits(const Sawyer::Container::BitVector &newBits) {
    ASSERT_require(newBits.size() == bits_.size());
    ASSERT_require(newBits.size() == get_width());
    bits_ = newBits;
}

bool
SValue::may_equal(const BaseSemantics::SValuePtr &other, SMTSolver*) const {
    return 0 == bits_.compare(SValue::promote(other)->bits());
}

bool
SValue::must_equal(const BaseSemantics::SValuePtr &other, SMTSolver*) const {
    return 0 == bits_.compare(SValue::promote(other)->bits());
}

void
SValue::set_width(size_t newWidth) {
    ASSERT_require(newWidth > 0);
    if (newWidth != get_width()) {
        bits_.resize(newWidth);
        BaseSemantics::SValue::set_width(newWidth);
        ASSERT_require(bits_.size() == get_width());
    }
}

uint64_t
SValue::get_number() const {
    return bits_.toInteger();
}

void
SValue::print(std::ostream &out, BaseSemantics::Formatter&) const {
    if (get_width() <= 64) {
        out <<StringUtility::toHex2(bits_.toInteger(), get_width());
    } else {
        out <<"0x" << bits_.toHex();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      MemoryState
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void
MemoryState::pageSize(rose_addr_t nBytes) {
    ASSERT_require(map_.isEmpty());
    pageSize_ = std::max(nBytes, (rose_addr_t)1);
}

void
MemoryState::allocatePage(rose_addr_t va) {
    rose_addr_t pageVa = alignDown(va, pageSize_);
    unsigned acc = MemoryMap::READABLE | MemoryMap::WRITABLE;
    map_.insert(AddressInterval::baseSize(pageVa, pageSize_),
                MemoryMap::Segment(MemoryMap::AllocatingBuffer::instance(pageSize_),
                                   0, acc, "ConcreteSemantics demand allocated"));
}

void
MemoryState::memoryMap(const MemoryMap &map, Sawyer::Optional<unsigned> padAccess) {
    map_ = map;
    rose_addr_t va = 0;
    while (map_.atOrAfter(va).next().assignTo(va)) {
        rose_addr_t pageVa = alignDown(va, pageSize_);

        // Mapped area must begin at a page boundary.
        if (pageVa < va) {
            unsigned acc = padAccess ? *padAccess : map_.get(va).accessibility();
            map_.insert(AddressInterval::hull(pageVa, va-1),
                        MemoryMap::Segment(MemoryMap::AllocatingBuffer::instance(va-pageVa), 0, acc, "padding"));
        }

        // Mapped area must end at the last byte before a page boundary.
        if (AddressInterval unused = map_.unmapped(va)) {
            va = unused.least();
            rose_addr_t nextPageVa = alignUp(va, pageSize_);
            if (nextPageVa > va) {
                unsigned acc = padAccess ? *padAccess : map_.get(va-1).accessibility();
                map_.insert(AddressInterval::hull(va, nextPageVa-1),
                            MemoryMap::Segment(MemoryMap::AllocatingBuffer::instance(nextPageVa-va), 0, acc, "padding"));
                va = nextPageVa - 1;
            }
        } else {
            break;
        }
        ++va;
    }
}

BaseSemantics::SValuePtr
MemoryState::readMemory(const BaseSemantics::SValuePtr &addr_, const BaseSemantics::SValuePtr &dflt_,
                        BaseSemantics::RiscOperators *addrOps, BaseSemantics::RiscOperators *valOps) {
    ASSERT_require2(8==dflt_->get_width(), "ConcreteSemantics::MemoryState requires memory cells contain 8-bit data");
    rose_addr_t addr = addr_->get_number();
    uint8_t dflt = dflt_->get_number();
    if (!map_.at(addr).exists()) {
        allocatePage(addr);
        map_.at(addr).limit(1).write(&dflt);
        return dflt_;
    }
    map_.at(addr).limit(1).read(&dflt);
    return dflt_->number_(8, dflt);
}

void
MemoryState::writeMemory(const BaseSemantics::SValuePtr &addr_, const BaseSemantics::SValuePtr &value_,
                         BaseSemantics::RiscOperators *addrOps, BaseSemantics::RiscOperators *valOps) {
    ASSERT_require2(8==value_->get_width(), "ConcreteSemantics::MemoryState requires memory cells contain 8-bit data");
    rose_addr_t addr = addr_->get_number();
    uint8_t value = value_->get_number();
    if (!map_.at(addr).exists())
        allocatePage(addr);
    map_.at(addr).limit(1).write(&value);
}

bool
MemoryState::merge(const BaseSemantics::MemoryStatePtr &other, BaseSemantics::RiscOperators *addrOps,
                   BaseSemantics::RiscOperators *valOps) {
    throw BaseSemantics::NotImplemented("MemoryState merging for ConcreteSemantics is not supported", NULL);
}

void
MemoryState::print(std::ostream &out, Formatter&) const {
    map_.dump(out);
    rose_addr_t pageVa = 0;
    uint8_t* page = new uint8_t[pageSize_];
    while (map_.atOrAfter(pageVa).next().assignTo(pageVa)) {
        size_t nread = map_.at(pageVa).limit(pageSize_).read(page).size();
        ASSERT_always_require(nread == pageSize_);
        HexdumpFormat fmt;
        SgAsmExecutableFileFormat::hexdump(out, pageVa, (const unsigned char*)page, pageSize_, fmt);
        out <<"\n";
        if (pageVa + (pageSize_-1) == map_.hull().greatest())
            break;
        pageVa += pageSize_;
    }
    delete [] page;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      RiscOperators
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SValuePtr
RiscOperators::svalue_number(const Sawyer::Container::BitVector &bits) {
    SValuePtr retval = SValue::promote(svalue_number(bits.size(), 0));
    retval->bits(bits);
    return retval;
}

void
RiscOperators::interrupt(int majr, int minr) {
    currentState()->clear();
}

BaseSemantics::SValuePtr
RiscOperators::and_(const BaseSemantics::SValuePtr &a_, const BaseSemantics::SValuePtr &b_) {
    BitVector result = SValue::promote(a_)->bits();
    result.bitwiseAnd(SValue::promote(b_)->bits());
    return svalue_number(result);
}

BaseSemantics::SValuePtr
RiscOperators::or_(const BaseSemantics::SValuePtr &a_, const BaseSemantics::SValuePtr &b_) {
    BitVector result = SValue::promote(a_)->bits();
    result.bitwiseOr(SValue::promote(b_)->bits());
    return svalue_number(result);
}

BaseSemantics::SValuePtr
RiscOperators::xor_(const BaseSemantics::SValuePtr &a_, const BaseSemantics::SValuePtr &b_) {
    BitVector result = SValue::promote(a_)->bits();
    result.bitwiseXor(SValue::promote(b_)->bits());
    return svalue_number(result);
}

BaseSemantics::SValuePtr
RiscOperators::invert(const BaseSemantics::SValuePtr &a_) {
    BitVector result = SValue::promote(a_)->bits();
    result.invert();
    return svalue_number(result);
}

BaseSemantics::SValuePtr
RiscOperators::extract(const BaseSemantics::SValuePtr &a_, size_t begin_bit, size_t end_bit) {
    ASSERT_require(end_bit <= a_->get_width());
    ASSERT_require(begin_bit < end_bit);
    BitVector result(end_bit - begin_bit);
    result.copy(result.hull(), SValue::promote(a_)->bits(), BitRange::hull(begin_bit, end_bit-1));
    return svalue_number(result);
}

BaseSemantics::SValuePtr
RiscOperators::concat(const BaseSemantics::SValuePtr &a_, const BaseSemantics::SValuePtr &b_) {
    size_t resultNBits = a_->get_width() + b_->get_width();
    BitVector result = SValue::promote(a_)->bits();
    result.resize(resultNBits);
    result.copy(BitRange::baseSize(a_->get_width(), b_->get_width()),
                SValue::promote(b_)->bits(), BitRange::baseSize(0, b_->get_width()));
    return svalue_number(result);
}

BaseSemantics::SValuePtr
RiscOperators::leastSignificantSetBit(const BaseSemantics::SValuePtr &a_) {
    uint64_t count = SValue::promote(a_)->bits().leastSignificantSetBit().orElse(0);
    return svalue_number(a_->get_width(), count);
}

BaseSemantics::SValuePtr
RiscOperators::mostSignificantSetBit(const BaseSemantics::SValuePtr &a_) {
    uint64_t count = SValue::promote(a_)->bits().mostSignificantSetBit().orElse(0);
    return svalue_number(a_->get_width(), count);
}

BaseSemantics::SValuePtr
RiscOperators::rotateLeft(const BaseSemantics::SValuePtr &a_, const BaseSemantics::SValuePtr &sa_) {
    BitVector result = SValue::promote(a_)->bits();
    result.rotateLeft(sa_->get_number());
    return svalue_number(result);
}

BaseSemantics::SValuePtr
RiscOperators::rotateRight(const BaseSemantics::SValuePtr &a_, const BaseSemantics::SValuePtr &sa_) {
    BitVector result = SValue::promote(a_)->bits();
    result.rotateRight(sa_->get_number());
    return svalue_number(result);
}

BaseSemantics::SValuePtr
RiscOperators::shiftLeft(const BaseSemantics::SValuePtr &a_, const BaseSemantics::SValuePtr &sa_) {
    BitVector result = SValue::promote(a_)->bits();
    result.shiftLeft(sa_->get_number());
    return svalue_number(result);
}

BaseSemantics::SValuePtr
RiscOperators::shiftRight(const BaseSemantics::SValuePtr &a_, const BaseSemantics::SValuePtr &sa_) {
    BitVector result = SValue::promote(a_)->bits();
    result.shiftRight(sa_->get_number());
    return svalue_number(result);
}

BaseSemantics::SValuePtr
RiscOperators::shiftRightArithmetic(const BaseSemantics::SValuePtr &a_, const BaseSemantics::SValuePtr &sa_) {
    BitVector result = SValue::promote(a_)->bits();
    result.shiftRightArithmetic(sa_->get_number());
    return svalue_number(result);
}

BaseSemantics::SValuePtr
RiscOperators::equalToZero(const BaseSemantics::SValuePtr &a_) {
    return svalue_boolean(SValue::promote(a_)->bits().isEqualToZero());
}

BaseSemantics::SValuePtr
RiscOperators::ite(const BaseSemantics::SValuePtr &sel_, const BaseSemantics::SValuePtr &a_, const BaseSemantics::SValuePtr &b_) {
    ASSERT_require(sel_->get_width() == 1);
    if (sel_->get_number()) {
        return a_->copy();
    } else {
        return b_->copy();
    }
}

BaseSemantics::SValuePtr
RiscOperators::unsignedExtend(const BaseSemantics::SValuePtr &a_, size_t new_width) {
    BitVector result = SValue::promote(a_)->bits();
    result.resize(new_width);
    return svalue_number(result);
}

BaseSemantics::SValuePtr
RiscOperators::signExtend(const BaseSemantics::SValuePtr &a_, size_t new_width) {
    BitVector result(new_width);
    result.signExtend(SValue::promote(a_)->bits());
    return svalue_number(result);
}

BaseSemantics::SValuePtr
RiscOperators::add(const BaseSemantics::SValuePtr &a_, const BaseSemantics::SValuePtr &b_) {
    BitVector result = SValue::promote(a_)->bits();
    result.add(SValue::promote(b_)->bits());
    return svalue_number(result);
}

BaseSemantics::SValuePtr
RiscOperators::addWithCarries(const BaseSemantics::SValuePtr &a_, const BaseSemantics::SValuePtr &b_,
                              const BaseSemantics::SValuePtr &c_, BaseSemantics::SValuePtr &carry_out/*out*/) {
    size_t nbits = a_->get_width();

    // Values extended by one bit
    BitVector   ae = SValue::promote(a_)->bits();   ae.resize(nbits+1);
    BitVector   be = SValue::promote(b_)->bits();   be.resize(nbits+1);
    BitVector   ce = SValue::promote(c_)->bits();   ce.resize(nbits+1);

    // Extended sum
    BitVector se = ae;
    se.add(be);
    se.add(ce);

    // Carry out
    BitVector co = ae;
    co.bitwiseXor(be);
    co.bitwiseXor(se);
    co.shiftRight(1);
    co.resize(nbits);
    carry_out = svalue_number(co);

    se.resize(nbits);
    return svalue_number(se);
}

BaseSemantics::SValuePtr
RiscOperators::negate(const BaseSemantics::SValuePtr &a_) {
    BitVector result = SValue::promote(a_)->bits();
    result.negate();
    return svalue_number(result);
}

BaseSemantics::SValuePtr
RiscOperators::signedDivide(const BaseSemantics::SValuePtr &a_, const BaseSemantics::SValuePtr &b_) {
    // FIXME[Robb P. Matzke 2015-03-31]: BitVector doesn't have a divide method
    if (a_->get_width() > 64 || b_->get_width() > 64) {
        throw BaseSemantics::Exception("signedDivide x[" + StringUtility::addrToString(a_->get_width()) +
                                       "] / y[" + StringUtility::addrToString(b_->get_width()) +
                                       "] is not implemented", currentInstruction());
    }
    int64_t a = IntegerOps::signExtend2(a_->get_number(), a_->get_width(), 64);
    int64_t b = IntegerOps::signExtend2(b_->get_number(), b_->get_width(), 64);
    if (0==b)
        throw BaseSemantics::Exception("division by zero", currentInstruction());
    return svalue_number(a_->get_width(), a/b);
}

BaseSemantics::SValuePtr
RiscOperators::signedModulo(const BaseSemantics::SValuePtr &a_, const BaseSemantics::SValuePtr &b_) {
    // FIXME[Robb P. Matzke 2015-03-31]: BitVector doesn't have a modulo method
    if (a_->get_width() > 64 || b_->get_width() > 64) {
        throw BaseSemantics::Exception("signedModulo x[" + StringUtility::addrToString(a_->get_width()) +
                                       "] % y[" + StringUtility::addrToString(b_->get_width()) +
                                       "] is not implemented", currentInstruction());
    }
    int64_t a = IntegerOps::signExtend2(a_->get_number(), a_->get_width(), 64);
    int64_t b = IntegerOps::signExtend2(b_->get_number(), b_->get_width(), 64);
    if (0==b)
        throw BaseSemantics::Exception("division by zero", currentInstruction());
    return svalue_number(b_->get_width(), a % b);
}

BaseSemantics::SValuePtr
RiscOperators::signedMultiply(const BaseSemantics::SValuePtr &a_, const BaseSemantics::SValuePtr &b_) {
    // FIXME[Robb P. Matzke 2015-03-31]: BitVector doesn't have a multiply method
    ASSERT_require2(a_->get_width() <= 64, "not implemented");
    ASSERT_require2(b_->get_width() <= 64, "not implemented");

    if (a_->get_width() == 64 && b_->get_width() == 64) {
        // Do long multiplication with 32 bits at a time in 64-bit variables (32 bits * 32 bits = 64 bits)
        // FIXME[Robb P. Matzke 2015-03-23]: use arbitrary width vector multiply in Sawyer when it's available.
        uint64_t a0 = a_->get_number() & 0xffffffff;
        uint64_t a1 = a_->get_number() >> 32;
        uint64_t b0 = b_->get_number() & 0xffffffff;
        uint64_t b1 = b_->get_number() >> 32;
        uint64_t c0 = a0 * b0;
        uint64_t c1 = (a0 * b1) + (a1 * b0) + (c0 >> 32);
        uint64_t c2 = (a1 * b1) + (c1 >> 32);
        c0 &= 0xffffffff;
        c1 &= 0xffffffff;
        //c2: use all 64 bits

        BitVector product(128);
        product.fromInteger(BitRange::baseSize( 0, 64), (c1 << 32) | c0);
        product.fromInteger(BitRange::baseSize(64, 64), c2);
        return svalue_number(product);
    } else if (a_->get_width() + b_->get_width() > 64) {
        throw BaseSemantics::Exception("signedMultiply x[" + StringUtility::addrToString(a_->get_width()) +
                                       "] * y[" + StringUtility::addrToString(b_->get_width()) +
                                       "] is not implemented", currentInstruction());
    } else {
        ASSERT_require2(a_->get_width() + b_->get_width() <= 64, "not implemented yet");
        int64_t a = IntegerOps::signExtend2(a_->get_number(), a_->get_width(), 64);
        int64_t b = IntegerOps::signExtend2(b_->get_number(), b_->get_width(), 64);
        return svalue_number(a_->get_width() + b_->get_width(), a * b);
    }
}

BaseSemantics::SValuePtr
RiscOperators::unsignedDivide(const BaseSemantics::SValuePtr &a_, const BaseSemantics::SValuePtr &b_) {
    SValuePtr a = SValue::promote(a_);
    SValuePtr b = SValue::promote(b_);

    // This is a common case on 64-bit architectures
    // FIXME[Robb P. Matzke 2015-06-24]: this will probably only compile with GCC >4.x on a 64-bit machine. The real solution
    // would be to either use a multi-precision library (sort of overkill) or implement division in Sawyer's BitVector class.
#ifdef __x86_64
    if (a->get_width() == 128 && b->get_width() == 64) {
        __uint128_t numerator = a->bits().toInteger(BitRange::baseSize(64, 64));
        numerator <<= 64;
        numerator |= a->bits().toInteger(BitRange::baseSize(0, 64));
        uint64_t denominator = b->bits().toInteger();
        if (0 == denominator)
            throw BaseSemantics::Exception("division by zero", currentInstruction());
        __uint128_t ratio = numerator / denominator;
        uint64_t ratio_lo = ratio;
        uint64_t ratio_hi = ratio >> 64;
        BitVector resultBits(128);
        resultBits.fromInteger(BitRange::baseSize(0, 64), ratio_lo);
        resultBits.fromInteger(BitRange::baseSize(64, 64), ratio_hi);
        return svalue_number(resultBits);
    }
#endif

    // FIXME[Robb P. Matzke 2015-03-31]: BitVector doesn't have a divide method
    if (a->get_width() > 64 || b->get_width() > 64) {
        throw BaseSemantics::Exception("unsignedDivide x[" + StringUtility::addrToString(a->get_width()) +
                                       "] / y[" + StringUtility::addrToString(b->get_width()) +
                                       "] is not implemented", currentInstruction());
    }

    uint64_t an = a->get_number();
    uint64_t bn = b->get_number();
    if (0==bn)
        throw BaseSemantics::Exception("division by zero", currentInstruction());
    return svalue_number(a->get_width(), an/bn);
}

BaseSemantics::SValuePtr
RiscOperators::unsignedModulo(const BaseSemantics::SValuePtr &a_, const BaseSemantics::SValuePtr &b_) {
    SValuePtr a = SValue::promote(a_);
    SValuePtr b = SValue::promote(b_);

    // This is a common case on 64-bit architectures
    // FIXME[Robb P. Matzke 2015-06-24]: this will probably only compile with GCC >4.x on a 64-bit machine. The real solution
    // would be to either use a multi-precision library (sort of overkill) or implement division in Sawyer's BitVector class.
#ifdef __x86_64
    if (a->get_width() == 128 && b->get_width() == 64) {
        __uint128_t numerator = a->bits().toInteger(BitRange::baseSize(64, 64));
        numerator <<= 64;
        numerator |= a->bits().toInteger(BitRange::baseSize(0, 64));
        uint64_t denominator = b->bits().toInteger();
        if (0 == denominator)
            throw BaseSemantics::Exception("division by zero", currentInstruction());
        uint64_t remainder = numerator % denominator;
        return svalue_number(64, remainder);
    }
#endif

    // FIXME[Robb P. Matzke 2015-03-31]: BitVector doesn't have a modulo method
    if (a->get_width() > 64 || b->get_width() > 64) {
        throw BaseSemantics::Exception("unsignedModulo x[" + StringUtility::addrToString(a->get_width()) +
                                       "] % y[" + StringUtility::addrToString(b->get_width()) +
                                       "] is not implemented", currentInstruction());
    }
    uint64_t an = a->get_number();
    uint64_t bn = b->get_number();
    if (0==bn)
        throw BaseSemantics::Exception("division by zero", currentInstruction());
    return svalue_number(b->get_width(), an % bn);
}

BaseSemantics::SValuePtr
RiscOperators::unsignedMultiply(const BaseSemantics::SValuePtr &a_, const BaseSemantics::SValuePtr &b_) {
    // FIXME[Robb P. Matzke 2015-03-31]: BitVector doesn't have a multiply method
    ASSERT_require2(a_->get_width() <= 64, "not implemented");
    ASSERT_require2(b_->get_width() <= 64, "not implemented");

    if (a_->get_width() == 64 && b_->get_width() == 64) {
        // Do long multiplication with 32 bits at a time in 64-bit variables (32 bits * 32 bits = 64 bits)
        // FIXME[Robb P. Matzke 2015-03-23]: use arbitrary width vector multiply in Sawyer when it's available.
        uint64_t a0 = a_->get_number() & 0xffffffff;
        uint64_t a1 = a_->get_number() >> 32;
        uint64_t b0 = b_->get_number() & 0xffffffff;
        uint64_t b1 = b_->get_number() >> 32;
        uint64_t c0 = a0 * b0;
        uint64_t c1 = (a0 * b1) + (a1 * b0) + (c0 >> 32);
        uint64_t c2 = (a1 * b1) + (c1 >> 32);
        c0 &= 0xffffffff;
        c1 &= 0xffffffff;
        //c2: use all 64 bits

        BitVector product(128);
        product.fromInteger(BitRange::baseSize( 0, 64), (c1 << 32) | c0);
        product.fromInteger(BitRange::baseSize(64, 64), c2);
        return svalue_number(product);
    } else if (a_->get_width() + b_->get_width() > 64) {
        throw BaseSemantics::Exception("unsignedMultiply x[" + StringUtility::addrToString(a_->get_width()) +
                                       "] * y[" + StringUtility::addrToString(b_->get_width()) +
                                       "] is not implemented", currentInstruction());
    } else {
        ASSERT_require2(a_->get_width() + b_->get_width() <= 64, "not implemented yet");
        uint64_t a = a_->get_number();
        uint64_t b = b_->get_number();
        return svalue_number(a_->get_width() + b_->get_width(), a * b);
    }
}

BaseSemantics::SValuePtr
RiscOperators::readMemory(const RegisterDescriptor &segreg, const BaseSemantics::SValuePtr &address,
                          const BaseSemantics::SValuePtr &dflt, const BaseSemantics::SValuePtr &cond) {
    size_t nbits = dflt->get_width();
    ASSERT_require(0 == nbits % 8);
    ASSERT_require(1==cond->get_width()); // FIXME: condition is not used
    if (cond->is_number() && !cond->get_number())
        return dflt;

    // Read the bytes and concatenate them together.
    BaseSemantics::SValuePtr retval;
    size_t nbytes = nbits/8;
    BaseSemantics::MemoryStatePtr mem = currentState()->memoryState();
    for (size_t bytenum=0; bytenum<nbits/8; ++bytenum) {
        size_t byteOffset = ByteOrder::ORDER_MSB==mem->get_byteOrder() ? nbytes-(bytenum+1) : bytenum;
        BaseSemantics::SValuePtr byte_dflt = extract(dflt, 8*byteOffset, 8*byteOffset+8);
        BaseSemantics::SValuePtr byte_addr = add(address, number_(address->get_width(), bytenum));

        // Use the lazily updated initial memory state if there is one.
        if (initialState())
            byte_dflt = initialState()->readMemory(byte_addr, byte_dflt, this, this);

        // Read the current memory state
        SValuePtr byte_value = SValue::promote(currentState()->readMemory(byte_addr, byte_dflt, this, this));
        if (0==bytenum) {
            retval = byte_value;
        } else if (ByteOrder::ORDER_MSB==mem->get_byteOrder()) {
            retval = concat(byte_value, retval);
        } else if (ByteOrder::ORDER_LSB==mem->get_byteOrder()) {
            retval = concat(retval, byte_value);
        } else {
            // See BaseSemantics::MemoryState::set_byteOrder
            throw BaseSemantics::Exception("multi-byte read with memory having unspecified byte order", currentInstruction());
        }
    }

    ASSERT_require(retval!=NULL && retval->get_width()==nbits);
    return retval;
}

void
RiscOperators::writeMemory(const RegisterDescriptor &segreg, const BaseSemantics::SValuePtr &address,
                           const BaseSemantics::SValuePtr &value_, const BaseSemantics::SValuePtr &cond) {
    ASSERT_require(1==cond->get_width()); // FIXME: condition is not used
    if (cond->is_number() && !cond->get_number())
        return;
    SValuePtr value = SValue::promote(value_->copy());
    size_t nbits = value->get_width();
    ASSERT_require(0 == nbits % 8);
    size_t nbytes = nbits/8;
    BaseSemantics::MemoryStatePtr mem = currentState()->memoryState();
    for (size_t bytenum=0; bytenum<nbytes; ++bytenum) {
        size_t byteOffset = 0;
        if (1 == nbytes) {
            // void
        } else if (ByteOrder::ORDER_MSB==mem->get_byteOrder()) {
            byteOffset = nbytes-(bytenum+1);
        } else if (ByteOrder::ORDER_LSB==mem->get_byteOrder()) {
            byteOffset = bytenum;
        } else {
            // See BaseSemantics::MemoryState::set_byteOrder
            throw BaseSemantics::Exception("multi-byte write with memory having unspecified byte order", currentInstruction());
        }

        BaseSemantics::SValuePtr byte_value = extract(value, 8*byteOffset, 8*byteOffset+8);
        BaseSemantics::SValuePtr byte_addr = add(address, number_(address->get_width(), bytenum));
        currentState()->writeMemory(byte_addr, byte_value, this, this);
    }
}

double
RiscOperators::exprToDouble(const BaseSemantics::SValuePtr &a, SgAsmFloatType *aType) {
    ASSERT_require(a->is_number());
    if (aType == SageBuilderAsm::buildIeee754Binary64()) {
        ASSERT_require(sizeof(double) == sizeof(int64_t));
        union {
            double fp;
            int64_t i;
        } u;
        u.i = a->get_number();
        return u.fp;
    } else if (aType == SageBuilderAsm::buildIeee754Binary32()) {
        ASSERT_require(sizeof(float) == sizeof(int32_t));
        union {
            float fp;
            int32_t i;
        } u;
        u.i = a->get_number();
        return u.fp;
    } else {
        throw BaseSemantics::NotImplemented("exprToDouble type not supported", currentInstruction());
    }
}

BaseSemantics::SValuePtr
RiscOperators::doubleToExpr(double d, SgAsmFloatType *retType) {
    if (retType == SageBuilderAsm::buildIeee754Binary64()) {
        ASSERT_require(sizeof(double) == sizeof(int64_t));
        union {
            double fp;
            int64_t i;
        } u;
        u.fp = d;
        return svalue_number(64, u.i);
    } else if (retType == SageBuilderAsm::buildIeee754Binary32()) {
        ASSERT_require(sizeof(float) == sizeof(int32_t));
        union {
            float fp;
            int32_t i;
        } u;
        u.fp = d;
        return svalue_number(32, u.i);
    } else {
        throw BaseSemantics::NotImplemented("doubleToExpr type not supported", currentInstruction());
    }
}
    
BaseSemantics::SValuePtr
RiscOperators::fpFromInteger(const BaseSemantics::SValuePtr &intValue, SgAsmFloatType *retType) {
    ASSERT_require(intValue->is_number());
    return doubleToExpr((double)intValue->get_number(), retType);
}

BaseSemantics::SValuePtr
RiscOperators::fpToInteger(const BaseSemantics::SValuePtr &a, SgAsmFloatType *aType, const BaseSemantics::SValuePtr &dflt) {
    double ad = exprToDouble(a, aType);
    size_t nBits = dflt->get_width();
    double minInt = -pow(2.0, nBits-1);
    double maxInt = pow(2.0, nBits-1);
    if (isnan(ad) || ad < minInt || ad >= maxInt)
        return dflt;
    int64_t ai = ad;
    return number_(nBits, ai);
}

BaseSemantics::SValuePtr
RiscOperators::fpAdd(const BaseSemantics::SValuePtr &a, const BaseSemantics::SValuePtr &b, SgAsmFloatType *fpType) {
    double ad = exprToDouble(a, fpType);
    double bd = exprToDouble(b, fpType);
    double result = ad + bd;
    return doubleToExpr(result, fpType);
}

BaseSemantics::SValuePtr
RiscOperators::fpSubtract(const BaseSemantics::SValuePtr &a, const BaseSemantics::SValuePtr &b, SgAsmFloatType *fpType) {
    double ad = exprToDouble(a, fpType);
    double bd = exprToDouble(b, fpType);
    double result = ad - bd;
    return doubleToExpr(result, fpType);
}

BaseSemantics::SValuePtr
RiscOperators::fpMultiply(const BaseSemantics::SValuePtr &a, const BaseSemantics::SValuePtr &b, SgAsmFloatType *fpType) {
    double ad = exprToDouble(a, fpType);
    double bd = exprToDouble(b, fpType);
    double result = ad * bd;
    return doubleToExpr(result, fpType);
}

BaseSemantics::SValuePtr
RiscOperators::fpRoundTowardZero(const BaseSemantics::SValuePtr &a, SgAsmFloatType *fpType) {
    double ad = exprToDouble(a, fpType);
    double result = trunc(ad);
    return doubleToExpr(result, fpType);
}

} // namespace
} // namespace
} // namespace
} // namespace
