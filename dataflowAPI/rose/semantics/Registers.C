#include "../util/StringUtility.h"
//#include "sage3basic.h"
#include "Registers.h"
#include "external/rose/armv8InstructionEnum.h"
#include "external/rose/amdgpuInstructionEnum.h"
#include "external/rose/rose-compat.h"
#include "external/rose/powerpcInstructionEnum.h"
#include <boost/foreach.hpp>
#include <mutex>

// These are here temporarily until the classes in this file can be moved into rose::BinaryAnalysis
using namespace rose;
using namespace rose::BinaryAnalysis;

std::ostream&
operator<<(std::ostream &o, const RegisterDictionary &dict)
{
    dict.print(o);
    return o;
}

std::ostream&
operator<<(std::ostream &o, const RegisterDescriptor &reg)
{
    reg.print(o);
    return o;
}


/*******************************************************************************************************************************
 *                                      RegisterDescriptor
 *******************************************************************************************************************************/

bool
RegisterDescriptor::operator==(const RegisterDescriptor &other) const 
{
    return majr==other.majr && minr==other.minr && offset==other.offset && nbits==other.nbits;
}

bool
RegisterDescriptor::operator!=(const RegisterDescriptor &other) const
{
    return !(*this==other);
}

bool
RegisterDescriptor::operator<(const RegisterDescriptor &other) const
{
    if (majr!=other.majr)
        return majr < other.majr;
    if (minr!=other.minr)
        return minr < other.minr;
    if (offset!=other.offset)
        return offset < other.offset;
    return nbits < other.nbits;
}


/*******************************************************************************************************************************
 *                                      RegisterNames
 *******************************************************************************************************************************/

std::string
RegisterNames::operator()(const RegisterDescriptor &rdesc, const RegisterDictionary *dict_/*=NULL*/) const
{
    if (!rdesc.is_valid())
        return prefix + (prefix==""?"":"_") + "NONE";

    const RegisterDictionary *dict = dict_ ? dict_ : dflt_dict;
    if (dict) {
        std::string name = dict->lookup(rdesc);
        if (!name.empty())
            return name;
    }

    std::ostringstream ss;
    ss <<prefix <<rdesc.get_major() <<"." <<rdesc.get_minor();
    if (show_offset>0 || (show_offset<0 && rdesc.get_offset()!=0))
        ss <<offset_prefix <<rdesc.get_offset() <<offset_suffix;
    if (show_size)
        ss <<size_prefix <<rdesc.get_nbits() <<size_suffix;
    ss <<suffix;
    return ss.str();
}


/*******************************************************************************************************************************
 *                                      RegisterDictionary
 *******************************************************************************************************************************/


/* class method */
uint64_t
RegisterDictionary::hash(const RegisterDescriptor &d)
{
    uint64_t h = d.get_major() << 24;
    h ^= d.get_minor() << 16;
    h ^= d.get_offset() << 8;
    h ^= d.get_nbits();
    return h;
}

void
RegisterDictionary::insert(const std::string &name, const RegisterDescriptor &rdesc) {
    /* Erase the name from the reverse lookup map, indexed by the old descriptor. */
    Entries::iterator fi = forward.find(name);
    if (fi!=forward.end()) {
        Reverse::iterator ri = reverse.find(hash(fi->second));
        ROSE_ASSERT(ri!=reverse.end());
        std::vector<std::string>::iterator vi=std::find(ri->second.begin(), ri->second.end(), name);
        ROSE_ASSERT(vi!=ri->second.end());
        ri->second.erase(vi);
    }

    /* Insert or replace old descriptor with a new one and insert reverse lookup info. */
    forward[name] = rdesc;
    reverse[hash(rdesc)].push_back(name);
}

void
RegisterDictionary::insert(const std::string &name, unsigned majr, unsigned minr, unsigned offset, unsigned nbits) {
    insert(name, RegisterDescriptor(majr, minr, offset, nbits));
}

void
RegisterDictionary::insert(const RegisterDictionary &other) {
    const Entries &entries = other.get_registers();
    for (Entries::const_iterator ei=entries.begin(); ei!=entries.end(); ++ei)
        insert(ei->first, ei->second);
}

void
RegisterDictionary::insert(const RegisterDictionary *other) {
    if (other)
        insert(*other);
}

const RegisterDescriptor *
RegisterDictionary::lookup(const std::string &name) const {
    Entries::const_iterator fi = forward.find(name);
    if (fi==forward.end())
        return NULL;
    return &(fi->second);
}

const std::string &
RegisterDictionary::lookup(const RegisterDescriptor &rdesc) const {
    Reverse::const_iterator ri = reverse.find(hash(rdesc));
    if (ri!=reverse.end()) {
        for (size_t i=ri->second.size(); i>0; --i) {
            const std::string &name = ri->second[i-1];
            Entries::const_iterator fi = forward.find(name);
            ROSE_ASSERT(fi!=forward.end());
            if (fi->second==rdesc)
                return name;
        }
    }

    static const std::string empty;
    return empty;
}

RegisterDescriptor
RegisterDictionary::findLargestRegister(unsigned major, unsigned minor, size_t maxWidth) const {
    RegisterDescriptor retval;
    for (Entries::const_iterator iter=forward.begin(); iter!=forward.end(); ++iter) {
        const RegisterDescriptor &reg = iter->second;
        if (major == reg.get_major() && minor == reg.get_minor()) {
            if (maxWidth > 0 && reg.get_nbits() > maxWidth) {
                // ignore
            } else if (!retval.is_valid()) {
                retval = reg;
            } else if (retval.get_nbits() < reg.get_nbits()) {
                retval = reg;
            }
        }
    }
    return retval;
}

void
RegisterDictionary::resize(const std::string &name, unsigned new_nbits) {
    const RegisterDescriptor *old_desc = lookup(name);
    ROSE_ASSERT(old_desc!=NULL);
    RegisterDescriptor new_desc = *old_desc;
    new_desc.set_nbits(new_nbits);
    insert(name, new_desc);
}

RegisterParts
RegisterDictionary::getAllParts() const {
    RegisterParts retval;
    BOOST_FOREACH (const Entries::value_type &node, forward)
        retval.insert(node.second);
    return retval;
}

const RegisterDictionary::Entries &
RegisterDictionary::get_registers() const {
    return forward;
}

RegisterDictionary::Entries &
RegisterDictionary::get_registers() {
    return forward;
}

RegisterDictionary::RegisterDescriptors
RegisterDictionary::get_descriptors() const
{
    const Entries &entries = get_registers();
    RegisterDescriptors retval;
    retval.reserve(entries.size());
    for (Entries::const_iterator ei=entries.begin(); ei!=entries.end(); ++ei)
        retval.push_back(ei->second);
    return retval;
}

RegisterDictionary::RegisterDescriptors
RegisterDictionary::get_largest_registers() const
{
    SortBySize order(SortBySize::ASCENDING);
    return filter_nonoverlapping(get_descriptors(), order, true);
}

RegisterDictionary::RegisterDescriptors
RegisterDictionary::get_smallest_registers() const
{
    SortBySize order(SortBySize::DESCENDING);
    return filter_nonoverlapping(get_descriptors(), order, true);
}

void
RegisterDictionary::print(std::ostream &o) const {
    o <<"RegisterDictionary \"" <<name <<"\" contains " <<forward.size() <<" " <<(1==forward.size()?"entry":"entries") <<"\n";
    for (Entries::const_iterator ri=forward.begin(); ri!=forward.end(); ++ri)
        o <<"  \"" <<ri->first <<"\" " <<StringUtility::addrToString(hash(ri->second)) <<" " <<ri->second <<"\n";

    for (Reverse::const_iterator ri=reverse.begin(); ri!=reverse.end(); ++ri) {
        o <<"  " <<StringUtility::addrToString(ri->first);
        for (std::vector<std::string>::const_iterator vi=ri->second.begin(); vi!=ri->second.end(); ++vi) {
            o <<" " <<*vi;
        }
        o <<"\n";
    }
}

/** Intel 8086 registers.
 *
 *  The Intel 8086 has fourteen 16-bit registers. Four of them (AX, BX, CX, DX) are general registers (although each may have
 *  an additional purpose; for example only CX can be used as a counter with the loop instruction). Each can be accessed as two
 *  separate bytes (thus BX's high byte can be accessed as BH and low byte as BL). Four segment registers (CS, DS, SS and ES)
 *  are used to form a memory address. There are two pointer registers. SP points to the bottom of the stack and BP which is
 *  used to point at some other place in the stack or the memory(Offset).  Two registers (SI and DI) are for array
 *  indexing. The FLAGS register contains flags such as carry flag, overflow flag and zero flag. Finally, the instruction
 *  pointer (IP) points to the next instruction that will be fetched from memory and then executed. */
//const RegisterDictionary *
//RegisterDictionary::dictionary_i8086() {
//    static RegisterDictionary *regs = NULL;
//    if (!regs) {
//        regs = new RegisterDictionary("i8086");
//
//        /*  16-bit general purpose registers. Each has three names depending on which bytes are reference. */
//        regs->insert("al", x86_regclass_gpr, x86_gpr_ax, 0, 8);
//        regs->insert("ah", x86_regclass_gpr, x86_gpr_ax, 8, 8);
//        regs->insert("ax", x86_regclass_gpr, x86_gpr_ax, 0, 16);
//
//        regs->insert("bl", x86_regclass_gpr, x86_gpr_bx, 0, 8);
//        regs->insert("bh", x86_regclass_gpr, x86_gpr_bx, 8, 8);
//        regs->insert("bx", x86_regclass_gpr, x86_gpr_bx, 0, 16);
//
//        regs->insert("cl", x86_regclass_gpr, x86_gpr_cx, 0, 8);
//        regs->insert("ch", x86_regclass_gpr, x86_gpr_cx, 8, 8);
//        regs->insert("cx", x86_regclass_gpr, x86_gpr_cx, 0, 16);
//
//        regs->insert("dl", x86_regclass_gpr, x86_gpr_dx, 0, 8);
//        regs->insert("dh", x86_regclass_gpr, x86_gpr_dx, 8, 8);
//        regs->insert("dx", x86_regclass_gpr, x86_gpr_dx, 0, 16);
//
//        /*  16-bit segment registers */
//        regs->insert("cs", x86_regclass_segment, x86_segreg_cs, 0, 16);
//        regs->insert("ds", x86_regclass_segment, x86_segreg_ds, 0, 16);
//        regs->insert("ss", x86_regclass_segment, x86_segreg_ss, 0, 16);
//        regs->insert("es", x86_regclass_segment, x86_segreg_es, 0, 16);
//
//        /* 16-bit pointer registers */
//        regs->insert("sp", x86_regclass_gpr, x86_gpr_sp, 0, 16);        /* stack pointer */
//        regs->insert("spl", x86_regclass_gpr, x86_gpr_sp, 0, 8);
//
//        regs->insert("bp", x86_regclass_gpr, x86_gpr_bp, 0, 16);        /* base pointer */
//        regs->insert("bpl", x86_regclass_gpr, x86_gpr_bp, 0, 8);
//
//        regs->insert("ip", x86_regclass_ip, 0, 0, 16);                  /* instruction pointer */
//        regs->insert("ipl", x86_regclass_ip, 0, 0, 8);
//
//        /* Array indexing registers */
//        regs->insert("si", x86_regclass_gpr, x86_gpr_si, 0, 16);
//        regs->insert("sil", x86_regclass_gpr, x86_gpr_si, 0, 8);
//
//        regs->insert("di", x86_regclass_gpr, x86_gpr_di, 0, 16);
//        regs->insert("dil", x86_regclass_gpr, x86_gpr_di, 0, 8);
//
//        /* Flags with official names. */
//        regs->insert("flags", x86_regclass_flags, x86_flags_status,  0, 16); /* all flags */
//        regs->insert("cf",    x86_regclass_flags, x86_flags_status,  0,  1); /* carry status flag */
//        regs->insert("pf",    x86_regclass_flags, x86_flags_status,  2,  1); /* parity status flag */
//        regs->insert("af",    x86_regclass_flags, x86_flags_status,  4,  1); /* adjust status flag */
//        regs->insert("zf",    x86_regclass_flags, x86_flags_status,  6,  1); /* zero status flag */
//        regs->insert("sf",    x86_regclass_flags, x86_flags_status,  7,  1); /* sign status flag */
//        regs->insert("tf",    x86_regclass_flags, x86_flags_status,  8,  1); /* trap system flag */
//        regs->insert("if",    x86_regclass_flags, x86_flags_status,  9,  1); /* interrupt enable system flag */
//        regs->insert("df",    x86_regclass_flags, x86_flags_status, 10,  1); /* direction control flag */
//        regs->insert("of",    x86_regclass_flags, x86_flags_status, 11,  1); /* overflow status flag */
//        regs->insert("nt",    x86_regclass_flags, x86_flags_status, 14,  1); /* nested task system flag */
//
//        /* Flags without names */
//        regs->insert("f1",    x86_regclass_flags, x86_flags_status,  1,  1);
//        regs->insert("f3",    x86_regclass_flags, x86_flags_status,  3,  1);
//        regs->insert("f5",    x86_regclass_flags, x86_flags_status,  5,  1);
//        regs->insert("f12",   x86_regclass_flags, x86_flags_status, 12,  1);
//        regs->insert("f13",   x86_regclass_flags, x86_flags_status, 13,  1);
//        regs->insert("f15",   x86_regclass_flags, x86_flags_status, 15,  1);
//    }
//    return regs;
//}
//
///** Intel 8088 registers.
// *
// *  Intel 8088 has the same set of registers as Intel 8086. */
//const RegisterDictionary *
//RegisterDictionary::dictionary_i8088()
//{
//    static RegisterDictionary *regs = NULL;
//    if (!regs) {
//        regs = new RegisterDictionary("i8088");
//        regs->insert(dictionary_i8086());
//    }
//    return regs;
//}
//
///** Intel 80286 registers.
// *
// *  The 80286 has the same registers as the 8086 but adds two new flags to the "flags" register. */
//const RegisterDictionary *
//RegisterDictionary::dictionary_i286()
//{
//    static RegisterDictionary *regs = NULL;
//    if (!regs) {
//        regs = new RegisterDictionary("i286");
//        regs->insert(dictionary_i8086());
//        regs->insert("iopl", x86_regclass_flags, x86_flags_status, 12, 2); /*  I/O privilege level flag */
//        regs->insert("nt",   x86_regclass_flags, x86_flags_status, 14, 1); /*  nested task system flag */
//    }
//    return regs;
//}
//
///** Intel 80386 registers.
// *
// *  The 80386 has the same registers as the 80286 but extends the general-purpose registers, base registers, index registers,
// *  instruction pointer, and flags register to 32 bits.  Register names from the 80286 refer to the same offsets and sizes while
// *  the full 32 bits are accessed by names prefixed with "e" as in "eax" (the "e" means "extended"). Two new segment registers
// *  (FS and GS) were added and all segment registers remain 16 bits. */
//const RegisterDictionary *
//RegisterDictionary::dictionary_i386()
//{
//    static RegisterDictionary *regs = NULL;
//    if (!regs) {
//        regs = new RegisterDictionary("i386");
//        regs->insert(dictionary_i286());
//
//        /* Additional 32-bit registers */
//        regs->insert("eax", x86_regclass_gpr, x86_gpr_ax, 0, 32);
//        regs->insert("ebx", x86_regclass_gpr, x86_gpr_bx, 0, 32);
//        regs->insert("ecx", x86_regclass_gpr, x86_gpr_cx, 0, 32);
//        regs->insert("edx", x86_regclass_gpr, x86_gpr_dx, 0, 32);
//        regs->insert("esp", x86_regclass_gpr, x86_gpr_sp, 0, 32);
//        regs->insert("ebp", x86_regclass_gpr, x86_gpr_bp, 0, 32);
//        regs->insert("eip", x86_regclass_ip, 0, 0, 32);
//        regs->insert("esi", x86_regclass_gpr, x86_gpr_si, 0, 32);
//        regs->insert("edi", x86_regclass_gpr, x86_gpr_di, 0, 32);
//        regs->insert("eflags", x86_regclass_flags, x86_flags_status, 0, 32);
//
//        /* Additional 16-bit segment registers */
//        regs->insert("fs", x86_regclass_segment, x86_segreg_fs, 0, 16);
//        regs->insert("gs", x86_regclass_segment, x86_segreg_gs, 0, 16);
//
//        /* Additional flags */
//        regs->insert("rf", x86_regclass_flags, x86_flags_status, 16, 1); /* resume system flag */
//        regs->insert("vm", x86_regclass_flags, x86_flags_status, 17, 1); /* virtual 8086 mode flag */
//
//        /* Additional flag bits that have no official names */
//        for (unsigned i=18; i<32; ++i)
//            regs->insert("f"+StringUtility::numberToString(i), x86_regclass_flags, x86_flags_status, i, 1);
//
//        /* Control registers */
//        regs->insert("cr0", x86_regclass_cr, 0, 0, 32);
//        regs->insert("cr1", x86_regclass_cr, 1, 0, 32);
//        regs->insert("cr2", x86_regclass_cr, 2, 0, 32);
//        regs->insert("cr3", x86_regclass_cr, 3, 0, 32);
//        regs->insert("cr4", x86_regclass_cr, 4, 0, 32);
//
//        /* Debug registers */
//        regs->insert("dr0", x86_regclass_dr, 0, 0, 32);
//        regs->insert("dr1", x86_regclass_dr, 1, 0, 32);
//        regs->insert("dr2", x86_regclass_dr, 2, 0, 32);
//        regs->insert("dr3", x86_regclass_dr, 3, 0, 32);                 /* dr4 and dr5 are reserved */
//        regs->insert("dr6", x86_regclass_dr, 6, 0, 32);
//        regs->insert("dr7", x86_regclass_dr, 7, 0, 32);
//
//    }
//    return regs;
//}
//
///** Intel 80386 with 80387 math co-processor. */
//const RegisterDictionary *
//RegisterDictionary::dictionary_i386_387()
//{
//    static RegisterDictionary *regs = NULL;
//    if (!regs) {
//        regs = new RegisterDictionary("i386 w/387");
//        regs->insert(dictionary_i386());
//
//        // The 387 contains eight floating-point registers that have no names (we call them "st0" through "st7"), and defines
//        // expressions of the form "st(n)" to refer to the current nth register from the top of a circular stack.  These
//        // expressions are implemented usng SgAsmIndexedRegisterExpression IR nodes, which have a base register which is
//        // "st0", a stride which increments the minor number, an offset which is the current top-of-stack value, an index
//        // which is the value "n" in the expression "st(n)", and a modulus of eight.  The current top-of-stack value is held in
//        // the three-bit register "fpstatus_top", which normally has a concrete value.
//        regs->insert("st0",     x86_regclass_st, x86_st_0,   0, 80);
//        regs->insert("st1",     x86_regclass_st, x86_st_1,   0, 80);
//        regs->insert("st2",     x86_regclass_st, x86_st_2,   0, 80);
//        regs->insert("st3",     x86_regclass_st, x86_st_3,   0, 80);
//        regs->insert("st4",     x86_regclass_st, x86_st_4,   0, 80);
//        regs->insert("st5",     x86_regclass_st, x86_st_5,   0, 80);
//        regs->insert("st6",     x86_regclass_st, x86_st_6,   0, 80);
//        regs->insert("st7",     x86_regclass_st, x86_st_7,   0, 80);
//
//        // Floating-point tag registers, two bits per ST register.
//        regs->insert("fptag",     x86_regclass_flags, x86_flags_fptag,  0, 16); // all tags
//        regs->insert("fptag_st0", x86_regclass_flags, x86_flags_fptag,  0,  2); // tag for st0
//        regs->insert("fptag_st1", x86_regclass_flags, x86_flags_fptag,  2,  2); // tag for st1
//        regs->insert("fptag_st2", x86_regclass_flags, x86_flags_fptag,  4,  2); // tag for st2
//        regs->insert("fptag_st3", x86_regclass_flags, x86_flags_fptag,  6,  2); // tag for st3
//        regs->insert("fptag_st4", x86_regclass_flags, x86_flags_fptag,  8,  2); // tag for st4
//        regs->insert("fptag_st5", x86_regclass_flags, x86_flags_fptag, 10,  2); // tag for st5
//        regs->insert("fptag_st6", x86_regclass_flags, x86_flags_fptag, 12,  2); // tag for st6
//        regs->insert("fptag_st7", x86_regclass_flags, x86_flags_fptag, 14,  2); // tag for st7
//
//        // Floating-point status register
//        regs->insert("fpstatus",     x86_regclass_flags, x86_flags_fpstatus,  0, 16);
//        regs->insert("fpstatus_ie",  x86_regclass_flags, x86_flags_fpstatus,  0,  1); // invalid operation
//        regs->insert("fpstatus_de",  x86_regclass_flags, x86_flags_fpstatus,  1,  1); // denormalized operand
//        regs->insert("fpstatus_ze",  x86_regclass_flags, x86_flags_fpstatus,  2,  1); // zero divide
//        regs->insert("fpstatus_oe",  x86_regclass_flags, x86_flags_fpstatus,  3,  1); // overflow
//        regs->insert("fpstatus_ue",  x86_regclass_flags, x86_flags_fpstatus,  4,  1); // underflow
//        regs->insert("fpstatus_pe",  x86_regclass_flags, x86_flags_fpstatus,  5,  1); // precision
//        regs->insert("fpstatus_ir",  x86_regclass_flags, x86_flags_fpstatus,  7,  1); // interrupt request
//        regs->insert("fpstatus_c4",  x86_regclass_flags, x86_flags_fpstatus,  8,  1); // condition code
//        regs->insert("fpstatus_c1",  x86_regclass_flags, x86_flags_fpstatus,  9,  1); // condition code
//        regs->insert("fpstatus_c2",  x86_regclass_flags, x86_flags_fpstatus, 10,  1); // condition code
//        regs->insert("fpstatus_top", x86_regclass_flags, x86_flags_fpstatus, 11,  3); // top of stack
//        regs->insert("fpstatus_c3",  x86_regclass_flags, x86_flags_fpstatus, 14,  1); // condition code
//        regs->insert("fpstatus_b",   x86_regclass_flags, x86_flags_fpstatus, 15,  1); // busy
//
//        // Floating-point control register
//        regs->insert("fpctl",    x86_regclass_flags, x86_flags_fpctl,  0, 16);
//        regs->insert("fpctl_im", x86_regclass_flags, x86_flags_fpctl,  0,  1); // invalid operation
//        regs->insert("fpctl_dm", x86_regclass_flags, x86_flags_fpctl,  1,  1); // denormalized operand
//        regs->insert("fpctl_zm", x86_regclass_flags, x86_flags_fpctl,  2,  1); // zero divide
//        regs->insert("fpctl_om", x86_regclass_flags, x86_flags_fpctl,  3,  1); // overflow
//        regs->insert("fpctl_um", x86_regclass_flags, x86_flags_fpctl,  4,  1); // underflow
//        regs->insert("fpctl_pm", x86_regclass_flags, x86_flags_fpctl,  5,  1); // precision
//        regs->insert("fpctl_m",  x86_regclass_flags, x86_flags_fpctl,  7,  1); // interrupt mask
//        regs->insert("fpctl_pc", x86_regclass_flags, x86_flags_fpctl,  8,  2); // precision control
//        regs->insert("fpctl_rc", x86_regclass_flags, x86_flags_fpctl, 10,  2); // rounding control
//        regs->insert("fpctl_ic", x86_regclass_flags, x86_flags_fpctl, 12,  1); // infinity control
//    }
//    return regs;
//}
//
//
///** Intel 80486 registers.
// *
// *  The 80486 has the same registers as the 80386 with '387 co-processor but adds a new flag to the "eflags" register. */
//const RegisterDictionary *
//RegisterDictionary::dictionary_i486()
//{
//    static RegisterDictionary *regs = NULL;
//    if (!regs) {
//        regs = new RegisterDictionary("i486");
//        regs->insert(dictionary_i386_387());
//        regs->insert("ac", x86_regclass_flags, x86_flags_status, 18, 1); /* alignment check system flag */
//    }
//    return regs;
//}
//
///** Intel Pentium registers.
// *
// *  The Pentium has the same registers as the 80486 but adds a few flags to the "eflags" register and MMX registers. */
//const RegisterDictionary *
//RegisterDictionary::dictionary_pentium()
//{
//    static RegisterDictionary *regs = NULL;
//    if (!regs) {
//        regs = new RegisterDictionary("pentium");
//        regs->insert(dictionary_i486());
//
//        /* Additional flags */
//        regs->insert("vif", x86_regclass_flags, x86_flags_status, 19, 1); /* virtual interrupt flag */
//        regs->insert("vip", x86_regclass_flags, x86_flags_status, 20, 1); /* virt interrupt pending */
//        regs->insert("id",  x86_regclass_flags, x86_flags_status, 21, 1); /* ident system flag */
//
//        /* The MMi registers are aliases for the ST(i) registers but are absolute rather than relative to the top of the
//         * stack. We're creating the static definitions, so MMi will point to the same storage as ST(i) for 0<=i<=7. Note that
//         * a write to one of the 64-bit MMi registers causes the high-order 16 bits of the corresponding ST(j) register to be
//         * set to all ones to indicate a NaN value. */
//        regs->insert("mm0", x86_regclass_st, x86_st_0, 0, 64);
//        regs->insert("mm1", x86_regclass_st, x86_st_1, 0, 64);
//        regs->insert("mm2", x86_regclass_st, x86_st_2, 0, 64);
//        regs->insert("mm3", x86_regclass_st, x86_st_3, 0, 64);
//        regs->insert("mm4", x86_regclass_st, x86_st_4, 0, 64);
//        regs->insert("mm5", x86_regclass_st, x86_st_5, 0, 64);
//        regs->insert("mm6", x86_regclass_st, x86_st_6, 0, 64);
//        regs->insert("mm7", x86_regclass_st, x86_st_7, 0, 64);
//    }
//    return regs;
//}
//
///** Intel Pentium III registers.
// *
// *  The Pentium III has the same register set as the Pentium but adds the xmm0 through xmm7 registers for the SSE instruction
// *  set. */
//const RegisterDictionary *
//RegisterDictionary::dictionary_pentiumiii()
//{
//    static RegisterDictionary *regs = NULL;
//    if (!regs) {
//        regs = new RegisterDictionary("pentiumiii");
//        regs->insert(dictionary_pentium());
//        regs->insert("xmm0", x86_regclass_xmm, 0, 0, 128);
//        regs->insert("xmm1", x86_regclass_xmm, 1, 0, 128);
//        regs->insert("xmm2", x86_regclass_xmm, 2, 0, 128);
//        regs->insert("xmm3", x86_regclass_xmm, 3, 0, 128);
//        regs->insert("xmm4", x86_regclass_xmm, 4, 0, 128);
//        regs->insert("xmm5", x86_regclass_xmm, 5, 0, 128);
//        regs->insert("xmm6", x86_regclass_xmm, 6, 0, 128);
//        regs->insert("xmm7", x86_regclass_xmm, 7, 0, 128);
//
//        /** SSE status and control register. */
//        regs->insert("mxcsr",     x86_regclass_flags, x86_flags_mxcsr,  0, 32);
//        regs->insert("mxcsr_ie",  x86_regclass_flags, x86_flags_mxcsr,  0,  1); // invalid operation flag
//        regs->insert("mxcsr_de",  x86_regclass_flags, x86_flags_mxcsr,  1,  1); // denormal flag
//        regs->insert("mxcsr_ze",  x86_regclass_flags, x86_flags_mxcsr,  2,  1); // divide by zero flag
//        regs->insert("mxcsr_oe",  x86_regclass_flags, x86_flags_mxcsr,  3,  1); // overflow flag
//        regs->insert("mxcsr_ue",  x86_regclass_flags, x86_flags_mxcsr,  4,  1); // underflow flag
//        regs->insert("mxcsr_pe",  x86_regclass_flags, x86_flags_mxcsr,  5,  1); // precision flag
//        regs->insert("mxcsr_daz", x86_regclass_flags, x86_flags_mxcsr,  6,  1); // denormals are zero
//        regs->insert("mxcsr_im",  x86_regclass_flags, x86_flags_mxcsr,  7,  1); // invalid operation mask
//        regs->insert("mxcsr_dm",  x86_regclass_flags, x86_flags_mxcsr,  8,  1); // denormal mask
//        regs->insert("mxcsr_zm",  x86_regclass_flags, x86_flags_mxcsr,  9,  1); // divide by zero mask
//        regs->insert("mxcsr_om",  x86_regclass_flags, x86_flags_mxcsr, 10,  1); // overflow mask
//        regs->insert("mxcsr_um",  x86_regclass_flags, x86_flags_mxcsr, 11,  1); // underflow mask
//        regs->insert("mxcsr_pm",  x86_regclass_flags, x86_flags_mxcsr, 12,  1); // precision mask
//        regs->insert("mxcsr_r",   x86_regclass_flags, x86_flags_mxcsr, 13,  2); // rounding mode
//        regs->insert("mxcsr_fz",  x86_regclass_flags, x86_flags_mxcsr, 15,  1); // flush to zero
//    }
//    return regs;
//}
//
///** Intel Pentium 4 registers. */
//const RegisterDictionary *
//RegisterDictionary::dictionary_pentium4()
//{
//    static RegisterDictionary *regs = NULL;
//    if (!regs) {
//        regs = new RegisterDictionary("pentium4");
//        regs->insert(dictionary_pentiumiii());
//    }
//    return regs;
//}
//


/** AMDGPU Registers
 * Scalar Registers : total 104 registers of 32 bits
 *
 */
const RegisterDictionary *
RegisterDictionary::dictionary_amdgpu_vega() {
    static std::once_flag initialized;
    static RegisterDictionary *regs = NULL;

    std::call_once(initialized, []() {
        regs = new RegisterDictionary("amdgpu_vega");

        /* All 60 variations (32- and 64-bit) of the 32 general purpose registers  */
        for (unsigned idx = 0; idx < 104; idx++) {
            regs->insert("sgpr" + StringUtility::numberToString(idx), amdgpu_regclass_sgpr, amdgpu_sgpr0 + idx, 0, 32);
        }

        /* 64-bit program counter register */
        regs->insert("pc", amdgpu_regclass_pc, 0, 0, 64);

        regs->insert("scc", amdgpu_regclass_hwr, amdgpu_status, 0, 1);


        /* 32-bit pstate register and the four relevant flags.*/
        /* Each flag is added as a separate register for individual access. Only allowed minor is 0 (since there is only one pstate register);
         * the different offsets indicate the positions of the flags within the pstate register. */
    });
    return regs;
}

/** ARMv8-A registers.
 * There are a total of 32 general purpose registers each 64 bits wide. Each of these registers can be addressed as its 32-bit or 64-bit form. The former are named with the prefix W and the latter with a prefix X. The 32nd register is not a physical register but the zero register and referred to as WZR/ZR. */
const RegisterDictionary *
RegisterDictionary::dictionary_armv8() {
    static std::once_flag initialized;
    static RegisterDictionary *regs = NULL;

    std::call_once(initialized, []() {
        regs = new RegisterDictionary("armv8");

        /* All 60 variations (32- and 64-bit) of the 32 general purpose registers  */
        for (unsigned idx = 0; idx < 31; idx++) {
            regs->insert("x" + StringUtility::numberToString(idx), armv8_regclass_gpr, armv8_gpr_r0 + idx, 0, 64);
            regs->insert("w" + StringUtility::numberToString(idx), armv8_regclass_gpr, armv8_gpr_r0 + idx, 0, 32);
        }

        /* We have 32 SIMD/FP registers V0 - V31. In ARMv8-A, all of these are 128 bits wide.
         * Not considering those instructions that treat each of these registers as an array of fixed-length units,
         * each SIMD/FP register is divided can be accessed as the following:
         * - A 128-bit register (Q0 - Q31)
         * - A 64-bit register with its LSB being bit 0 of the register (D0 - D31)
         * - A 64-bit register with its LDB being bit 64 of the register (HQ0 - HQ31)
         * - A 32-bit register (S0 - S31)
         * - A 16-bit register (H0 - H31)
         * - An 8-bit register (B0 - B31) */
        for(unsigned idx = 0; idx < 32; idx++) {
            /* 128-bit parts of V0 - V31 */
            regs->insert("q" + StringUtility::numberToString(idx), armv8_regclass_simd_fpr, armv8_simdfpr_v0 + idx, 0, 128);

            /* 64-bit parts of V0 - V31 */
            regs->insert("d" + StringUtility::numberToString(idx), armv8_regclass_simd_fpr, armv8_simdfpr_v0 + idx, 0, 64);
            regs->insert("hq" + StringUtility::numberToString(idx), armv8_regclass_simd_fpr, armv8_simdfpr_v0 + idx, 64, 64);

            /* 32-bit parts of V0 - V31 */
            regs->insert("s" + StringUtility::numberToString(idx), armv8_regclass_simd_fpr, armv8_simdfpr_v0 + idx, 0, 32);

            /* 16-bit parts of V0 - V31 */
            regs->insert("h" + StringUtility::numberToString(idx), armv8_regclass_simd_fpr, armv8_simdfpr_v0 + idx, 0, 16);

            /* 8-bit parts of V0 - V31 */
            regs->insert("b" + StringUtility::numberToString(idx), armv8_regclass_simd_fpr, armv8_simdfpr_v0 + idx, 0, 8);
        }

        /* 32-bit section of the stack pointer register */
        regs->insert("wsp", armv8_regclass_sp, 0, 0, 32);
        /* Complete stack pointer regiser */
        regs->insert("sp", armv8_regclass_sp, 0, 0, 64);

        /* 64-bit program counter register */
        regs->insert("pc", armv8_regclass_pc, 0, 0, 64);

        /* 32-bit pstate register and the four relevant flags.*/
        /* Each flag is added as a separate register for individual access. Only allowed minor is 0 (since there is only one pstate register);
         * the different offsets indicate the positions of the flags within the pstate register. */
        regs->insert("pstate", armv8_regclass_pstate, 0, armv8_pstatefield_pstate, 32);
        regs->insert("n", armv8_regclass_pstate, 0, armv8_pstatefield_n, 1);
        regs->insert("z", armv8_regclass_pstate, 0, armv8_pstatefield_z, 1);
        regs->insert("c", armv8_regclass_pstate, 0, armv8_pstatefield_c, 1);
        regs->insert("v", armv8_regclass_pstate, 0, armv8_pstatefield_v, 1);
    });
    return regs;
}

/** PowerPC registers. */
const RegisterDictionary *
RegisterDictionary::dictionary_powerpc()
{
    static std::once_flag initialized;
    static RegisterDictionary *regs = NULL;

    std::call_once(initialized, []() {
        regs = new RegisterDictionary("powerpc");

        /**********************************************************************************************************************
         * General purpose and floating point registers
         **********************************************************************************************************************/
        for (unsigned i=0; i<32; i++) {
            regs->insert("r"+StringUtility::numberToString(i), powerpc_regclass_gpr, i, 0, 32);
            regs->insert("f"+StringUtility::numberToString(i), powerpc_regclass_fpr, i, 0, 64);
        }

        /**********************************************************************************************************************
         * State, status, condition, control registers
         **********************************************************************************************************************/

        /* Machine state register */
        regs->insert("msr", powerpc_regclass_msr, 0, 0, 32);

        /* Floating point status and control register */
        regs->insert("fpscr", powerpc_regclass_fpscr, 0, 0, 32);

        /* Condition Register. This register is grouped into eight fields, where each field is 4 bits. Many PowerPC
         * instructions define bit 31 of the instruction encoding as the Rc bit, and some instructions imply an Rc value equal
         * to 1. When Rc is equal to 1 for integer operations, the CR field 0 is set to reflect the result of the instruction's
         * operation: Equal (EQ), Greater Than (GT), Less Than (LT), and Summary Overflow (SO). When Rc is equal to 1 for
         * floating-point operations, the CR field 1 is set to reflect the state of the exception status bits in the FPSCR: FX,
         * FEX, VX, and OX. Any CR field can be the target of an integer or floating-point comparison instruction. The CR field
         * 0 is also set to reflect the result of a conditional store instruction (stwcx or stdcx). There is also a set of
         * instructions that can manipulate a specific CR bit, a specific CR field, or the entire CR, usually to combine
         * several conditions into a single bit for testing. */
        regs->insert("cr", powerpc_regclass_cr, 0, 0, 32);
        for (unsigned i=0; i<32; i++) {
            switch (i%4) {
                case 0:
                    regs->insert("cr"+StringUtility::numberToString(i/4), powerpc_regclass_cr, 0, i, 4);
                    regs->insert("cr"+StringUtility::numberToString(i/4)+"*4+lt", powerpc_regclass_cr, 0, i, 1);
                    break;
                case 1:
                    regs->insert("cr"+StringUtility::numberToString(i/4)+"*4+gt", powerpc_regclass_cr, 0, i, 1);
                    break;
                case 2:
                    regs->insert("cr"+StringUtility::numberToString(i/4)+"*4+eq", powerpc_regclass_cr, 0, i, 1);
                    break;
                case 3:
                    regs->insert("cr"+StringUtility::numberToString(i/4)+"*4+so", powerpc_regclass_cr, 0, i, 1);
                    break;
            }
        }

        /* The processor version register is a 32-bit read-only register that identifies the version and revision level of the
         * processor. Processor versions are assigned by the PowerPC architecture process. Revision levels are implementation
         * defined. Access to the register is privileged, so that an application program can determine the processor version
         * only with the help of an operating system function. */
        regs->insert("pvr", powerpc_regclass_pvr, 0, 0, 32);

        /**********************************************************************************************************************
         * The instruction address register is a pseudo register. It is not directly available to the user other than through a
         * "branch and link" instruction. It is primarily used by debuggers to show the next instruction to be executed.
         **********************************************************************************************************************/
        regs->insert("iar", powerpc_regclass_iar, 0, 0, 32);

        /**********************************************************************************************************************
         * Special purpose registers. There are 1024 of these, some of which have special names.  We name all 1024 consistently
         * and create aliases for the special ones. This allows the disassembler to look them up generically.  Because the
         * special names appear after the generic names, a reverse lookup will return the special name.
         **********************************************************************************************************************/
        /* Generic names for them all */
        for (unsigned i=0; i<1024; i++)
            regs->insert("spr"+StringUtility::numberToString(i), powerpc_regclass_spr, i, 0, 32);

        /* The link register contains the address to return to at the end of a function call.  Each branch instruction encoding
         * has an LK bit. If the LK bit is 1, the branch instruction moves the program counter to the link register. Also, the
         * conditional branch instruction BCLR branches to the value in the link register. */
        regs->insert("lr", powerpc_regclass_spr, powerpc_spr_lr, 0, 32);

        /* The fixed-point exception register contains carry and overflow information from integer arithmetic operations. It
         * also contains carry input to certain integer arithmetic operations and the number of bytes to transfer during load
         * and store string instructions, lswx and stswx. */
        regs->insert("xer", powerpc_regclass_spr, powerpc_spr_xer, 0, 32);

        /* The count register contains a loop counter that is decremented on certain branch operations. Also, the conditional
         * branch instruction bcctr branches to the value in the CTR. */
        regs->insert("ctr", powerpc_regclass_spr, powerpc_spr_ctr, 0, 32);

        /* Other special purpose registers. */
        regs->insert("dsisr", powerpc_regclass_spr, powerpc_spr_dsisr, 0, 32);
        regs->insert("dar", powerpc_regclass_spr, powerpc_spr_dar, 0, 32);
        regs->insert("dec", powerpc_regclass_spr, powerpc_spr_dec, 0, 32);

        /**********************************************************************************************************************
         * Time base registers. There are 1024 of these, some of which have special names. We name all 1024 consistently and
         * create aliases for the special ones. This allows the disassembler to look them up generically.  Because the special
         * names appear after the generic names, a reverse lookup will return the special name.
         **********************************************************************************************************************/
        for (unsigned i=0; i<1024; i++)
            regs->insert("tbr"+StringUtility::numberToString(i), powerpc_regclass_tbr, i, 0, 32);

        regs->insert("tbl", powerpc_regclass_tbr, powerpc_tbr_tbl, 0, 32);      /* time base lower */
        regs->insert("tbu", powerpc_regclass_tbr, powerpc_tbr_tbu, 0, 32);      /* time base upper */
    });
    return regs;
}
