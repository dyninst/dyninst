//
// Created by ssunny on 5/26/16.
//

#ifndef DYNINST_REGISTERDESCRIPTOR_H
#define DYNINST_REGISTERDESCRIPTOR_H

#include <iostream>

/** Describes (part of) a physical CPU register.
 *
 *  Some architectures have multiple names for physical registers. For example, a amd64 has a 64-bit integer register, parts
 *  of which are referred to as "rax", "eax", "ax", "al", and "ah".  The purpose of a RegisterDescriptor is to describe what
 *  physical register (major and minor) is being referenced, and the part (offset and nbits) that's being referenced.
 *
 *  The reason for having a major and minor number to specify a register is to allow for different types of registers. For
 *  instance, an i686 has a set of 64-bit integer registers and a set of 80-bit floating point registers (among others).
 *  Having major and minor numbers allows the physical register set (such as defined by an instruction semantics policy) to
 *  be implemented as an array of 64-bit integers and an array of 80-bit floating points (among others). The array is selected
 *  by the major number while the element of the array is selected by the minor number.
 *
 *  The RegisterDescriptor type is part of a SgAsmRegisterReferenceExpression and also appears in the register dictionaries
 *  (RegisterDictionary) used in various places including the disassembler. */
struct RegisterDescriptor {
public:
    RegisterDescriptor()
            : majr(0), minr(0), offset(0), nbits(0) {}
    RegisterDescriptor(unsigned majr_, unsigned minr_, unsigned offset_, unsigned nbits_)
            : majr(majr_), minr(minr_), offset(offset_), nbits(nbits_) {}
    unsigned get_major() const {
        return majr;
    }
    bool is_valid() const {
        return nbits!=0;
    }
    RegisterDescriptor &set_major(unsigned majr_) {
        this->majr = majr_;
        return *this;
    }
    unsigned get_minor() const {
        return minr;
    }
    RegisterDescriptor &set_minor(unsigned minr_) {
        this->minr = minr_;
        return *this;
    }
    unsigned get_offset() const {
        return offset;
    }
    RegisterDescriptor &set_offset(unsigned offset_) {
        this->offset = offset_;
        return *this;
    }
    unsigned get_nbits() const {
        return nbits;
    }
    RegisterDescriptor &set_nbits(unsigned nbits_) {
        this->nbits = nbits_;
        return *this;
    }
    bool operator<(const RegisterDescriptor &other) const;
    bool operator==(const RegisterDescriptor &other) const;
    bool operator!=(const RegisterDescriptor &other) const;
    void print(std::ostream &o) const {
        o <<"{" <<majr <<"," <<minr <<"," <<offset <<"," <<nbits <<"}";
    }
    friend std::ostream& operator<<(std::ostream&, const RegisterDescriptor&); /*in Register.C*/
private:
    /* Strange names "majr" and "minr" are to avoid a conflict with "major" and "minor" macros in some non-standard headers. */
    unsigned majr;	/** Major type of register, such as integer, floating point, flag, etc. */
    unsigned minr;	/** Register number within major division. */
    unsigned offset;	/** Low-bit offset within the physical register. */
    unsigned nbits;	/** Number of bits referenced by this descriptor. */
};

#endif //DYNINST_REGISTERDESCRIPTOR_H
