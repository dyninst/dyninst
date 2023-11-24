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

/** AMDGPU Registers
 * Scalar Registers : total 104 registers of 32 bits
 *
 */
const RegisterDictionary *
RegisterDictionary::dictionary_amdgpu() {
    static std::once_flag initialized;
    static RegisterDictionary *regs = NULL;

    std::call_once(initialized, []() {
        regs = new RegisterDictionary("AMDGPU");

        for (unsigned idx = 0; idx < 104; idx++) {
            regs->insert("sgpr" + StringUtility::numberToString(idx), amdgpu_regclass_sgpr, amdgpu_sgpr0 + idx, 0, 32);
        }

        regs->insert("pc_all", amdgpu_regclass_pc, 0, 0, 64);
        regs->insert("src_scc", amdgpu_regclass_hwr, amdgpu_status, 0, 1);
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
