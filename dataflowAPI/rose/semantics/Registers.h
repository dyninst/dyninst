#ifndef ROSE_BINARY_REGISTERS_H
#define ROSE_BINARY_REGISTERS_H

#include "../ExtentMap.h"
#include "../util/Map.h"
#include "RegisterParts.h"

#include <map>
#include <queue>
#include <string>

using namespace Sawyer::Container;

/** Defines registers available for a particular architecture.
 *
 *  The dictionary maps each register name to a RegisterDescriptor.  The RegisterDescriptor describes how a register name maps
 *  to (part of) a physical CPU register by providing a major number, minor number, bit offset, and number of bits.  The major
 *  number is typically a register class number and the minor number is typically an offset within the class.  For instance,
 *  for x86 the major numbers might indicate whether a register is a general purpose register, an 80-bit floating point
 *  register, a 128-bit MMX register, etc.
 *
 *  Users should not assume that the values of a RegisterDescriptor correspond to actual values found in the machine
 *  instructions that were disassembled.  What they can assume is that two unrelated registers (such as "eax" and "ebx") do not
 *  overlap in the RegisterDescriptor address space (major, minor, offset, size). They can also assume that two related
 *  registers (such as "eax" and "rax", the 32- and 64-bit versions of a single CPU register) do, in fact, overlap in the
 *  RegisterDescriptor address space and that the overlap indicates how the registers are related.
 *
 *  Users should not assume that RegisterDescriptor entries from two separate dictionaries are compatible. Looking up the "eax"
 *  register in one dictionary may return a different descriptor than "eax" looked up in a different dictionary.  Components of
 *  the ROSE binary support that generate RegisterDescriptors provide a mechanism for obtaining (and possibly setting) the
 *  register dictionary.  For instance, the Disassembler class has get_registers() and set_registers() methods. */
class RegisterDictionary {
public:
    typedef std::map<std::string/*name*/, RegisterDescriptor> Entries;
    typedef std::vector<RegisterDescriptor> RegisterDescriptors;

    /* Functions that return a dictionary for a particular machine architecute. (See implementation for documentation.) */
    /*static const RegisterDictionary *dictionary_i8086();                // Intel 8086
    static const RegisterDictionary *dictionary_i8088();                // Intel 8088
    static const RegisterDictionary *dictionary_i286();                 // Intel 80286
    static const RegisterDictionary *dictionary_i386();                 // Intel 80386
    static const RegisterDictionary *dictionary_i386_387();             // Intel 80386 with 80387 math coprocessor
    static const RegisterDictionary *dictionary_i486();                 // Intel 80486
    static const RegisterDictionary *dictionary_pentium();              // Intel Pentium
    static const RegisterDictionary *dictionary_pentiumiii();           // Intel Pentium III
    static const RegisterDictionary *dictionary_pentium4();             // Intel Pentium 4
    static const RegisterDictionary *dictionary_amd64();                // AMD Athlon 64*/
    static const RegisterDictionary *dictionary_armv8();                // ARMv8-A architecture
    static const RegisterDictionary *dictionary_amdgpu_vega();                // ARMv8-A architecture
    static const RegisterDictionary *dictionary_powerpc();

    RegisterDictionary(const std::string &name)
        :name(name) {}
    RegisterDictionary(const RegisterDictionary& other) {
        *this = other;
    }

    /** Obtain the name of the dictionary. */
    const std::string &get_architecture_name() const {
        return name;
    }

    /** Set the name of the dictionary. Dictionary names are generally architecture names.  Dictionaries created by one of the
     *  built-in static methods of this class have the same name as the method that created it. */
    void set_architecture_name(const std::string &name) {
        this->name = name;
    }

    /** Insert a definition into the dictionary.  If the name already exists in the dictionary then the new RegisterDescriptor
     *  will replace the one that already exists. */
    void insert(const std::string &name, const RegisterDescriptor&);

    /** Insert a definition into the dictionary.  If the name already exists in the dictionary then the new RegisterDescriptor
     *  will replace the one that already exists. */
    void insert(const std::string &name, unsigned majr, unsigned minr, unsigned offset, unsigned nbits);

    /** Inserts definitions from another dictionary into this dictionary. Names in the other dictionary that are the same as
     *  names in this dictionary will replace the definitions in this dictionary. */
    void insert(const RegisterDictionary*);

    /** Inserts definitions from another dictionary into this dictionary. Names in the other dictionary that are the same as
     *  names in this dictionary will replace the definitions in this dictionary. */
    void insert(const RegisterDictionary&);

    /** Changes the size of a register.  This is a common enough operation that we have a special method to do it.  To change
     *  other properties of a register you would look up the register descriptor, change the property, then re-insert the
     *  register into the dictionary using the new descriptor.  This method does exactly that. */
    void resize(const std::string &name, unsigned new_nbits);

    /** Returns a descriptor for a given register name. Returns the null pointer if the name is not found. It is not possible
     *  to modify a descriptor in the dictionary because doing so would interfere with the dictionary's data structures for
     *  reverse lookups. */
    const RegisterDescriptor *lookup(const std::string &name) const;

    /** Returns a register name for a given descriptor. If more than one register has the same descriptor then the name added
     *  latest is returned.  If no register is found then either return the empty string (default) or generate a generic name
     *  according to the optional supplied NameGenerator. */
    const std::string& lookup(const RegisterDescriptor&) const;

    /** Finds the first largest register with specified major and minor number.
     *
     *  Returns the first register with the largest width and having the specified major and minor numbers. Registers wider
     *  than @p maxWidth (if non-zero) are ignored. If no register is found with the major/minor number then an invalid
     *  (default-constructed) register is returned.
     *
     *  This function takes O(n) time where n is the number of registers defined. */
    RegisterDescriptor findLargestRegister(unsigned major, unsigned minor, size_t maxWidth=0) const;

    /** Returns all register parts.
     *
     *  Returns all parts of all registers in this dictionary without any regard for register boundaries.  For instance, if a
     *  diction contains the x86 AX and AL registers where AX is 16 bits and AL is its low-order eight bits, the return value
     *  will only contain the fact that the 16 bits corresponding to AX are stored, which also happens to contain the eight
     *  bits of AL, but it won't keep track that AX and AL were inserted separately. In other words, erasing AL from the
     *  returned container would also erase the low-order 8 bits of AX. */
    rose::BinaryAnalysis::RegisterParts getAllParts() const;

    /** Returns the list of all register definitions in the dictionary.
     * @{ */
    const Entries& get_registers() const;
    Entries& get_registers();
    /** @} */

    /** Returns the list of all register descriptors. The returned list may have overlapping register descriptors. The return
     *  value is similar to get_registers() except only the RegisterDescriptor part is returned, not the names. */
    RegisterDescriptors get_descriptors() const;

    /** Compares number of bits in register descriptors. This comparator is used to sort register descriptors in either
     *  ascending or descending order depending on the number of significant bits in the register. The default constructor
     *  uses a descending sort order.
     *
     *  For instance, to get a list of all descriptors sorted by descending number of significant bits, one could do:
     *
     * @code
     *  RegisterDescriptors regs = dictionary.get_descriptors();
     *  std::sort(regs.begin(), regs.end(), SortBySize());
     * @endcode
     */
    class SortBySize {
    public:
        enum Direction { ASCENDING, DESCENDING };
        explicit SortBySize(Direction d=DESCENDING): direction(d) {}
        bool operator()(const RegisterDescriptor &a, const RegisterDescriptor &b) const {
            return ASCENDING==direction ?
                a.get_nbits() < b.get_nbits() :
                a.get_nbits() > b.get_nbits();
        }
    protected:
        Direction direction;
    };

    /** Returns the list of non-overlapping registers or register parts.  The list of registers are processed in the reverse
     *  specified order and each register is added to the return value if its bits were not already added to the return value
     *  by a previous register.  If a register under consideration is only partially represented in the return value at the
     *  time it is considered, then it is either split into smaller parts to be reconsidered later, or not reconsidered at
     *  all. Thus, when @p reconsider_parts is true, the return value might contain register descriptors that were not part of
     *  the input @p reglist, and which might not even have entries/names in the register dictionary (see
     *  get_largest_registers() for more explaination).
     *
     *  For example, to get a list of the largest non-overlapping registers one could do the following:
     * @code
     *  RegisterDictionary::SortBySize largest_to_smallest(RegisterDictionary::SortBySize::DESCENDING);
     *  RegisterDictionary::RegisterDescriptors all_registers = ditionary.get_descriptors();
     *  RegisterDictionary::RegisterDescriptors list = dictionary.filter_nonoverlapping(all_registers, largest_to_smallest, true);
     * @endcode
     *
     *  Filtering largest to smallest and reconsidering parts is the default, so the example can be shortened to:
     * @code
     *  RegisterDictionary::RegisterDescriptors list = dictionary.filter_nonoverlapping(dictionary.get_descriptors());
     * @endcode
     *
     *  In fact, it can be shortened even more since this common operation is encapsulated in get_largest_registers():
     * @code
     *  RegisterDictionary::RegisterDescriptors list = dictionary.get_largest_registers();
     * @endcode
     */
    template<class Compare>
    static RegisterDescriptors filter_nonoverlapping(RegisterDescriptors reglist,
                                                     Compare order=SortBySize(),
                                                     bool reconsider_parts=true);

    /** Returns a list of the largest non-overlapping registers. For instance, for a 32-bit x86 dictionary the return value
     *  will contain registers EAX, EBX, etc. but not AX, AH, AL, BX, BH, BL, etc. Note that some of the returned descriptors
     *  might not correspond to actual names in the dictionary; this can happen when the dictionary contains two registers that
     *  partially overlap, but are themselves not subsets of a larger register, as in:
     *
     * @code
     *  |XXXXXXXXXXXX....| The "X" register occupies the first 12 bits of a 16-bit register
     *  |....YYYYYYYYYYYY| The "Y" register occupies the last 12 bits of a 16-bit register
     * @endcode
     *
     * If the 16-bit register has no name, and the high- and low-order four-bit parts have no name, then the return value
     * might consist of register "X" and the low four bits.  Or it could be register "Y" and the high four bits. */
    RegisterDescriptors get_largest_registers() const;

    /** Returns a list of the smallest non-overlapping registers.  For instance, for a 32-bit x86 dictionary the return value
     * will contain AX, AH, AL, BX, BH, BL, etc. rather than EAX and EBX.  It will also return the high 16-bit parts of EAX and
     * EBX even though they're not represented with explicit definitions in the dictionary.
     *
     * This is a one-liner in terms of filter_nonoverlapping.  If you don't want the unnamed parts to appear in the return
     * value (e.g., the high-order 16 bits of EAX), then call filter_nonoverlapping() like this:
     * @code
     *  SortBySize order(SortBySize::ASCENDING);
     *  RegisterDescriptors smallest = dict.filter_nonoverlapping(dict.get_descriptors(), order, false);
     * @endcode
     */
    RegisterDescriptors get_smallest_registers() const;

    /** Prints the contents of this register dictionary.  The first line of output contains the dictionary name. One additional
     *  line of output will be generated for each entry in the dictionary. */
    void print(std::ostream&) const;
    friend std::ostream& operator<<(std::ostream&, const RegisterDictionary&);

    /** Return the number of entries in the dictionary. */
    size_t size() const { return forward.size(); }

private:
    typedef std::map<uint64_t/*desc_hash*/, std::vector<std::string> > Reverse;
    static uint64_t hash(const RegisterDescriptor&);
    std::string name; /*name of the dictionary, usually an architecture name like 'i386'*/
    Entries forward;
    Reverse reverse;
};

/** Prints a register name even when no dictionary is available or when the dictionary doesn't contain an entry for the
 *  specified descriptor. */
class RegisterNames {
public:
    /** Constructor. A RegisterDictionary can be supplied to the constructor, or to each operator() call. */
    explicit RegisterNames(const RegisterDictionary *dict=NULL)
        : dflt_dict(dict), prefix("REG"), show_offset(-1), offset_prefix("@"), show_size(false) {}

    /** Obtain a name for a register descriptor.  If a dictionary is supplied, then it will be used instead of the dictionary
     *  that was supplied to the constructor. */
    std::string operator()(const RegisterDescriptor&, const RegisterDictionary *dict=NULL) const;

    const RegisterDictionary *dflt_dict;/**< Dictionary supplied to the constructor. */
    std::string prefix;                 /**< The leading part of a register name. */
    std::string suffix;                 /**< String to print at the very end of the generated name. */
    int show_offset;                    /**< 0=>never show offset; positive=>always show; negative=>show only when non-zero */
    std::string offset_prefix;          /**< String printed before the offset when the offset is shown. */
    std::string offset_suffix;          /**< String printed after the offset when the offset is shown. */
    bool show_size;                     /**< Whether to show the size in bits of the register. */
    std::string size_prefix;            /**< String printed prior to the size when the size is printed. */
    std::string size_suffix;            /**< String printed after the size when the size is printed. */
};

/*******************************************************************************************************************************
 *                                      Template definitions
 *******************************************************************************************************************************/



template<class Compare>
RegisterDictionary::RegisterDescriptors
RegisterDictionary::filter_nonoverlapping(RegisterDescriptors desc, Compare order, bool reconsider_parts)
{
    RegisterDescriptors retval;
    Map<std::pair<int/*major*/, int/*minor*/>, ExtentMap> have_bits; // the union of the bits of descriptors we will return

    // Process the descriptors in the order specified.
    std::priority_queue<RegisterDescriptor, RegisterDescriptors, Compare> heap(order, desc);
    while (!heap.empty()) {
        const RegisterDescriptor cur_desc = heap.top();
        heap.pop();
        const std::pair<int, int> cur_majmin(cur_desc.get_major(), cur_desc.get_minor());
        const Extent cur_extent(cur_desc.get_offset(), cur_desc.get_nbits());
        ExtentMap &have_extents = have_bits[cur_majmin];
        if (have_extents.distinct(cur_extent)) {
            // We're not returning any of these bits yet, so add the whole descriptor
            retval.push_back(cur_desc);
            have_extents.insert(cur_extent);
        } else if (reconsider_parts) {
            // We're already returning some (or all) of these bits. Split the cur_extent into sub-parts by subtracting the
            // stuff we're already returning.
            ExtentMap parts;
            parts.insert(cur_extent);
            parts.erase_ranges(have_extents);
            for (ExtentMap::iterator pi=parts.begin(); pi!=parts.end(); ++pi) {
                const Extent &part = pi->first;
                RegisterDescriptor part_desc(cur_desc.get_major(), cur_desc.get_minor(), part.first(), part.size());
                heap.push(part_desc);
            }
        }
    }
    return retval;
}
    
#endif /*!ROSE_BINARY_REGISTERRS_H*/
