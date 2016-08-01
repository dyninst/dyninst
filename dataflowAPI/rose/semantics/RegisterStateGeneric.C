#include "../util/StringUtility.h"
//#include <sage3basic.h>
//#include <Diagnostics.h>
#include <iomanip>
#include "RegisterStateGeneric.h"

// Define this if you want extra consistency checking before and after each mutator.  This slows things down considerably but
// can be useful for narrowing down logic errors in the implementation.
//#define RegisterStateGeneric_ExtraAssertions

// Define this if you want the readRegister behavior as it existed before 2015-09-24. This behavior was wrong in certain ways
// because it didn't always cause registers to spring into existence the first time they were read.
//#define RegisterStateGeneric_20150924

namespace rose {
namespace BinaryAnalysis {
namespace InstructionSemantics2 {
namespace BaseSemantics {

//using namespace rose::Diagnostics;

static bool
sortByOffset(const RegisterStateGeneric::RegPair &a, const RegisterStateGeneric::RegPair &b) {
    return a.desc.get_offset() < b.desc.get_offset();
}

void
RegisterStateGeneric::clear()
{
    registers_.clear();
    eraseWriters();
}

void
RegisterStateGeneric::zero()
{
    // We're initializing with a concrete value, so it really doesn't matter whether we initialize large registers
    // or small registers.  Constant folding will adjust things as necessary when we start reading registers.
    std::vector<RegisterDescriptor> regs = regdict->get_largest_registers();
    initialize_nonoverlapping(regs, true);
}

void
RegisterStateGeneric::initialize_large()
{
    std::vector<RegisterDescriptor> regs = regdict->get_largest_registers();
    initialize_nonoverlapping(regs, false);
}

void
RegisterStateGeneric::initialize_small()
{
    std::vector<RegisterDescriptor> regs = regdict->get_smallest_registers();
    initialize_nonoverlapping(regs, false);
}

void
RegisterStateGeneric::initialize_nonoverlapping(const std::vector<RegisterDescriptor> &regs, bool initialize_to_zero)
{
    clear();
    for (size_t i=0; i<regs.size(); ++i) {
        std::string name = regdict->lookup(regs[i]);
        SValuePtr val;
        if (initialize_to_zero) {
            val = protoval()->number_(regs[i].get_nbits(), 0);
        } else {
            val = protoval()->undefined_(regs[i].get_nbits());
            if (!name.empty() && val->get_comment().empty())
                val->set_comment(name+"_0");
        }
        registers_.insertMaybeDefault(regs[i]).push_back(RegPair(regs[i], val));
    }
}

static bool
has_null_value(const RegisterStateGeneric::RegPair &rp)
{
    return rp.value == NULL;
}

void
RegisterStateGeneric::assertStorageConditions(const std::string &when, const RegisterDescriptor &reg) const {
#if !defined(NDEBUG) && defined(RegisterStateGeneric_ExtraAssertions)
#if 1 // DEBUGGING [Robb P. Matzke 2015-09-28]
    static volatile size_t ncalls = 0;
    ++ncalls;
#endif
    std::ostringstream error;
    BOOST_FOREACH (const Registers::Node &rnode, registers_.nodes()) {
        Sawyer::Container::IntervalSet<BitRange> foundLocations;
        BOOST_FOREACH (const RegPair &regpair, rnode.value()) {
            if (!regpair.desc.is_valid()) {
                error <<"invalid register descriptor";
            } else if (regpair.desc.get_major() != rnode.key().majr || regpair.desc.get_minor() != rnode.key().minr) {
                error <<"register is in wrong list; register=" <<regpair.desc.get_major() <<"." <<regpair.desc.get_minor()
                      <<", list=" <<rnode.key().majr <<"." <<rnode.key().minr;
            } else if (regpair.value == NULL) {
                error <<"value is null for register " <<regpair.desc;
            } else if (regpair.value->get_width() != regpair.desc.get_nbits()) {
                error <<"value width (" <<regpair.value->get_width() <<") is incorrect for register " <<regpair.desc;
            } else if (foundLocations.isOverlapping(regpair.location())) {
                error <<"register " <<regpair.desc <<" is stored multiple times in the list";
            }
            foundLocations.insert(regpair.location());
            if (!error.str().empty())
                break;
        }

        if (!error.str().empty()) {
            //FIXME
            /*mlog[FATAL] <<when <<" register " <<reg <<":\n";
            mlog[FATAL] <<"  " <<error.str() <<"\n";
            mlog[FATAL] <<"  related registers:\n";
            BOOST_FOREACH (const RegPair &regpair, rnode.value()) {
                mlog[FATAL] <<"    " <<regpair.desc;
                if (regpair.value == NULL)
                    mlog[FATAL] <<"\tnull value";
                mlog[FATAL] <<"\n";
            }*/
            abort();
        }
    }
#endif
}

RegisterStateGeneric::RegPairs&
RegisterStateGeneric::scanAccessedLocations(const RegisterDescriptor &reg, RiscOperators *ops, bool markOverlapping,
                                            RegPairs &accessedParts /*out*/, RegPairs &preservedParts /*out*/) {
    BitRange accessedLocation = BitRange::baseSize(reg.get_offset(), reg.get_nbits());
    RegPairs &pairList = registers_.insertMaybeDefault(reg);
    BOOST_FOREACH (RegPair &regpair, pairList) {
        BitRange storedLocation = regpair.location();   // the thing that's already stored in this state
        BitRange overlap = storedLocation & accessedLocation;
        if (overlap.isEmpty())
            continue;

        // Low-order bits of stored part that are not needed for this access
        if (overlap.least() > storedLocation.least()) {
            size_t nbits = overlap.least() - storedLocation.least();
            RegisterDescriptor subreg(reg.get_major(), reg.get_minor(), storedLocation.least(), nbits);
            preservedParts.push_back(RegPair(subreg, ops->unsignedExtend(regpair.value, nbits)));
        }

        // Bits of the part that are needed for this access.
        {
            RegisterDescriptor subreg(reg.get_major(), reg.get_minor(), overlap.least(), overlap.size());
            size_t extractBegin = overlap.least() - storedLocation.least();
            size_t extractEnd = extractBegin + overlap.size();
            accessedParts.push_back(RegPair(subreg, ops->extract(regpair.value, extractBegin, extractEnd)));
        }

        // High-order bits of stored part that are not needed for this access
        if (overlap.greatest() < storedLocation.greatest()) {
            size_t nbits = storedLocation.greatest() - overlap.greatest();
            RegisterDescriptor subreg(reg.get_major(), reg.get_minor(), overlap.greatest()+1, nbits);
            size_t extractBegin = overlap.greatest()+1 - storedLocation.least();
            size_t extractEnd = extractBegin + nbits;
            preservedParts.push_back(RegPair(subreg, ops->extract(regpair.value, extractBegin, extractEnd)));
        }

        // If we're allowed to modify the storage locations so as to match the accessed register then mark this pair with a
        // null value so we can erase it in the next step.
        if (markOverlapping)
            regpair.value = SValuePtr();
    }
    return pairList;
}

SValuePtr
RegisterStateGeneric::readRegister(const RegisterDescriptor &reg, const SValuePtr &dflt, RiscOperators *ops)
{
    ASSERT_require(reg.is_valid());
    ASSERT_not_null(dflt);
    ASSERT_require(reg.get_nbits() == dflt->get_width());
    ASSERT_not_null(ops);
    assertStorageConditions("at start of read", reg);
    BitRange accessedLocation = BitRange::baseSize(reg.get_offset(), reg.get_nbits());
#ifdef RegisterStateGeneric_20150924
    const bool adjustLocations = false;
#else
    const bool adjustLocations = accessModifiesExistingLocations_;
#endif

    // Fast case: the state does not store this register or any register that might overlap with this register.
    if (!registers_.exists(reg)) {
        if (!accessCreatesLocations_)
            throw RegisterNotPresent(reg);
        SValuePtr newval = dflt->copy();
        std::string regname = regdict->lookup(reg);
        if (!regname.empty() && newval->get_comment().empty())
            newval->set_comment(regname + "_0");
        registers_.insertMaybeDefault(reg).push_back(RegPair(reg, newval));
        assertStorageConditions("at end of read", reg);
        return newval;
    }

    // Check that we're allowed to add storage locations if necessary.
    if (!accessCreatesLocations_) {
        size_t nBitsFound = 0;
        BOOST_FOREACH (const RegPair &regpair, registers_.getOrDefault(reg))
            nBitsFound += (regpair.location() & accessedLocation).size();
        ASSERT_require(nBitsFound <= accessedLocation.size());
        if (nBitsFound < accessedLocation.size())
            throw RegisterNotPresent(reg);
    }

    // Iterate over the storage/value pairs to figure out what parts of the register are already in existing storage locations,
    // and which parts of those overlapping storage locations are not accessed.
    RegPairs accessedParts;                             // parts of existing overlapping locations we access
    RegPairs preservedParts;                            // parts of existing overlapping locations we don't access
    RegPairs &pairList = scanAccessedLocations(reg, ops, adjustLocations, accessedParts /*out*/, preservedParts /*out*/);

    // Figure out which part of the access does not exist in the state
    typedef Sawyer::Container::IntervalSet<BitRange> Locations;
    Locations newLocations;
    newLocations.insert(accessedLocation);
    BOOST_FOREACH (const RegPair &regpair, accessedParts)
        newLocations -= regpair.location();

    // Create values for the parts of the accessed register that weren't stored in the state, but don't store them yet.
    RegPairs newParts;
    if (accessCreatesLocations_) {
        BOOST_FOREACH (const BitRange &newLocation, newLocations.intervals()) {
            RegisterDescriptor subreg(reg.get_major(), reg.get_minor(), newLocation.least(), newLocation.size());
            ASSERT_require(newLocation.least() >= reg.get_offset());
            SValuePtr newval = ops->extract(dflt,
                                            newLocation.least()-reg.get_offset(),
                                            newLocation.greatest()+1-reg.get_offset());
            newParts.push_back(RegPair(subreg, newval));
        }
    } else {
        ASSERT_require(newLocations.isEmpty());         // should have thrown a RegisterNotPresent exception already
    }

    // Construct the return value by combining all the parts we found or created.
    SValuePtr retval;
    RegPairs retvalParts = accessedParts;
    retvalParts.insert(retvalParts.end(), newParts.begin(), newParts.end());
    std::sort(retvalParts.begin(), retvalParts.end(), sortByOffset);
    BOOST_FOREACH (const RegPair &regpair, retvalParts)
        retval = retval ? ops->concat(retval, regpair.value) : regpair.value;
    ASSERT_require(retval->get_width() == reg.get_nbits());

    // Update the register state -- write parts that didn't exist and maybe combine or split some locations.
    if (adjustLocations) {
        // Combine all the accessed locations into a single location corresponding exactly to the return value, and split those
        // previously existing locations that partly overlap the returned location.
        pairList.erase(std::remove_if(pairList.begin(), pairList.end(), has_null_value), pairList.end());
        pairList.insert(pairList.end(), preservedParts.begin(), preservedParts.end());
        pairList.push_back(RegPair(reg, retval));
    } else {
        // Since this is a registerRead operation, the only thing we need to write back to the state are those parts of the
        // return value that had no locations allocated originally.
        pairList.insert(pairList.end(), newParts.begin(), newParts.end());
    }

    assertStorageConditions("at end of read", reg);
    return retval;
}

void
RegisterStateGeneric::writeRegister(const RegisterDescriptor &reg, const SValuePtr &value, RiscOperators *ops)
{
    ASSERT_not_null(value);
    ASSERT_require2(reg.get_nbits()==value->get_width(), "value written to register must be the same width as the register");
    ASSERT_not_null(ops);
    assertStorageConditions("at start of write", reg);
    BitRange accessedLocation = BitRange::baseSize(reg.get_offset(), reg.get_nbits());

    // Fast case: the state does not store this register or any register that might overlap with this register.
    if (!registers_.exists(reg)) {
        if (!accessCreatesLocations_)
            throw RegisterNotPresent(reg);
        registers_.insertMaybeDefault(reg).push_back(RegPair(reg, value));
        assertStorageConditions("at end of write", reg);
        return;
    }

    // Check that we're allowed to add storage locations if necessary.
    if (!accessCreatesLocations_) {
        size_t nBitsFound = 0;
        BOOST_FOREACH (const RegPair &regpair, registers_.getOrDefault(reg))
            nBitsFound += (regpair.location() & accessedLocation).size();
        ASSERT_require(nBitsFound <= accessedLocation.size());
        if (nBitsFound < accessedLocation.size())
            throw RegisterNotPresent(reg);
    }

    // Iterate over the storage/value pairs to figure out what parts of the register are already in existing storage locations,
    // and which parts of those overlapping storage locations are not accessed.
    RegPairs accessedParts;                             // parts of existing overlapping locations we access
    RegPairs preservedParts;                            // parts of existing overlapping locations we don't access
    RegPairs &pairList = scanAccessedLocations(reg, ops, accessModifiesExistingLocations_,
                                               accessedParts /*out*/, preservedParts /*out*/);

    // Figure out which part of the access does not exist in the state
    typedef Sawyer::Container::IntervalSet<BitRange> Locations;
    Locations newLocations;
    newLocations.insert(accessedLocation);
    BOOST_FOREACH (const RegPair &regpair, accessedParts)
        newLocations -= regpair.location();

    // Update the register state by writing to the accessed area.
    if (accessModifiesExistingLocations_) {
        // Combine all the accessed locations into a single location corresponding exactly to the written value, and split
        // those previously existing locations that partly overlap the written location.
        pairList.erase(std::remove_if(pairList.begin(), pairList.end(), has_null_value), pairList.end());
        pairList.insert(pairList.end(), preservedParts.begin(), preservedParts.end());
        pairList.push_back(RegPair(reg, value));
    } else {
        // Don't insert/erase locations that existed already -- only change their values.
        BOOST_FOREACH (RegPair &regpair, pairList) {
            BitRange storedLocation = regpair.location();
            if (BitRange overlap = storedLocation & accessedLocation) {
                SValuePtr valueToWrite;
                if (overlap.least() > storedLocation.least()) {
                    size_t nbits = overlap.least() - storedLocation.least();
                    SValuePtr loValue = ops->unsignedExtend(regpair.value, nbits);
                    valueToWrite = loValue;
                }
                {
                    size_t nbits = overlap.size();
                    size_t extractBegin = overlap.least() - accessedLocation.least();
                    size_t extractEnd = extractBegin + nbits;
                    SValuePtr midValue = ops->extract(value, extractBegin, extractEnd);
                    valueToWrite = valueToWrite ? ops->concat(valueToWrite, midValue) : midValue;
                }
                if (overlap.greatest() < storedLocation.greatest()) {
                    size_t nbits = storedLocation.greatest() - overlap.greatest();
                    size_t extractBegin = overlap.greatest()+1 - storedLocation.least();
                    size_t extractEnd = extractBegin + nbits;
                    SValuePtr hiValue = ops->extract(regpair.value, extractBegin, extractEnd);
                    valueToWrite = valueToWrite ? ops->concat(valueToWrite, hiValue) : hiValue;
                }
                ASSERT_not_null(valueToWrite);
                ASSERT_require(valueToWrite->get_width() == regpair.desc.get_nbits());
                regpair.value = valueToWrite;
            }
        }

        // Insert the parts that didn't exist before
        BOOST_FOREACH (const BitRange &newLocation, newLocations.intervals()) {
            size_t extractBegin = newLocation.least() - accessedLocation.least();
            size_t extractEnd = extractBegin + newLocation.size();
            SValuePtr valueToWrite = ops->extract(value, extractBegin, extractEnd);
            ASSERT_require(valueToWrite->get_width() == newLocation.size());
            RegisterDescriptor subreg(reg.get_major(), reg.get_minor(), newLocation.least(), newLocation.size());
            pairList.push_back(RegPair(subreg, valueToWrite));
        }
    }
    assertStorageConditions("at end of write", reg);
}

void
RegisterStateGeneric::updateWriteProperties(const RegisterDescriptor &reg, InputOutputProperty prop) {
    insertProperties(reg, prop);
    if (prop == IO_WRITE)
        eraseProperties(reg, IO_READ_AFTER_WRITE);
}

void
RegisterStateGeneric::updateReadProperties(const RegisterDescriptor &reg) {
    insertProperties(reg, IO_READ);
    BitProperties &props = properties_.insertMaybeDefault(reg);
    BitRange where = BitRange::baseSize(reg.get_offset(), reg.get_nbits());
    BOOST_FOREACH (BitProperties::Node &node, props.findAll(where)) {
        if (!node.value().exists(IO_WRITE)) {
            node.value().insert(IO_READ_BEFORE_WRITE);
            if (!node.value().exists(IO_INIT))
                node.value().insert(IO_READ_UNINITIALIZED);
        } else {
            node.value().insert(IO_READ_AFTER_WRITE);
        }
    }
}

void
RegisterStateGeneric::erase_register(const RegisterDescriptor &reg, RiscOperators *ops)
{
    ASSERT_require(reg.is_valid());
    ASSERT_not_null(ops);
    assertStorageConditions("at start of erase", reg);
    BitRange accessedLocation = BitRange::baseSize(reg.get_offset(), reg.get_nbits());

    // Fast case: the state does not store this register or any register that might overlap with this register
    if (!registers_.exists(reg))
        return;                                         // no part of register is stored in this state
    RegPairs &pairList = registers_[reg];

    // Look for existing registers that overlap with this register and remove them.  If the overlap was only partial, then we
    // need to eventually add the non-overlapping part back into the list.
    RegPairs nonoverlaps; // the non-overlapping parts of overlapping registers
    Extent need_extent(reg.get_offset(), reg.get_nbits());
    BOOST_FOREACH (RegPair &reg_val, pairList) {
        BitRange haveLocation = reg_val.location();
        if (BitRange intersection = accessedLocation & haveLocation) {
            if (haveLocation.least() < intersection.least()) {
                size_t leftSize = intersection.least() - haveLocation.least();
                RegisterDescriptor subreg(reg.get_major(), reg.get_minor(), haveLocation.least(), leftSize);
                nonoverlaps.push_back(RegPair(subreg, ops->unsignedExtend(reg_val.value, leftSize)));
            }
            if (intersection.greatest() < haveLocation.greatest()) {
                size_t rightSize = haveLocation.greatest() - intersection.greatest();
                size_t lobit = intersection.greatest()+1 - haveLocation.least();
                RegisterDescriptor subreg(reg.get_major(), reg.get_minor(), intersection.greatest()+1, rightSize);
                nonoverlaps.push_back(RegPair(subreg, ops->extract(reg_val.value, lobit, lobit+rightSize)));
            }
            reg_val.value = SValuePtr();
        }
    }

    // Remove marked pairs, then add the non-overlapping parts.
    pairList.erase(std::remove_if(pairList.begin(), pairList.end(), has_null_value), pairList.end());
    pairList.insert(pairList.end(), nonoverlaps.begin(), nonoverlaps.end());
    assertStorageConditions("at end of erase", reg);
}

RegisterStateGeneric::RegPairs
RegisterStateGeneric::get_stored_registers() const
{
    RegPairs retval;
    BOOST_FOREACH (const RegPairs &pairlist, registers_.values())
        retval.insert(retval.end(), pairlist.begin(), pairlist.end());
    return retval;
}

void
RegisterStateGeneric::traverse(Visitor &visitor)
{
    BOOST_FOREACH (RegPairs &pairlist, registers_.values()) {
        BOOST_FOREACH (RegPair &pair, pairlist) {
            if (SValuePtr newval = (visitor)(pair.desc, pair.value)) {
                ASSERT_require(newval->get_width() == pair.desc.get_nbits());
                pair.value = newval;
            }
        }
    }
}

void
RegisterStateGeneric::deep_copy_values()
{
    BOOST_FOREACH (RegPairs &pairlist, registers_.values()) {
        BOOST_FOREACH (RegPair &pair, pairlist)
            pair.value = pair.value->copy();
    }
}

bool
RegisterStateGeneric::is_partly_stored(const RegisterDescriptor &desc) const
{
    BitRange want = BitRange::baseSize(desc.get_offset(), desc.get_nbits());
    BOOST_FOREACH (const RegPair &pair, registers_.getOrDefault(desc)) {
        if (want & pair.location())
            return true;
    }
    return false;
}

bool
RegisterStateGeneric::is_wholly_stored(const RegisterDescriptor &desc) const
{
    Sawyer::Container::IntervalSet<BitRange> desired;
    desired.insert(BitRange::baseSize(desc.get_offset(), desc.get_nbits()));
    BOOST_FOREACH (const RegPair &pair, registers_.getOrDefault(desc))
        desired -= pair.location();
    return desired.isEmpty();
}

bool
RegisterStateGeneric::is_exactly_stored(const RegisterDescriptor &desc) const
{
    BOOST_FOREACH (const RegPair &pair, registers_.getOrDefault(desc)) {
        if (desc == pair.desc)
            return true;
    }
    return false;
}

ExtentMap
RegisterStateGeneric::stored_parts(const RegisterDescriptor &desc) const
{
    ExtentMap retval;
    Extent want(desc.get_offset(), desc.get_nbits());
    BOOST_FOREACH (const RegPair &pair, registers_.getOrDefault(desc)) {
        Extent have(pair.desc.get_offset(), pair.desc.get_nbits());
        retval.insert(want.intersect(have));
    }
    return retval;
}

RegisterStateGeneric::RegPairs
RegisterStateGeneric::overlappingRegisters(const RegisterDescriptor &needle) const {
    ASSERT_require(needle.is_valid());
    BitRange needleBits = BitRange::baseSize(needle.get_offset(), needle.get_nbits());
    RegPairs retval;
    BOOST_FOREACH (const RegPair &pair, registers_.getOrDefault(needle)) {
        if (needleBits & pair.location())
            retval.push_back(pair);
    }
    return retval;
}

bool
RegisterStateGeneric::insertWriters(const RegisterDescriptor &desc, const AddressSet &writerVas) {
    if (writerVas.isEmpty())
        return false;
    BitAddressSet &parts = writers_.insertMaybeDefault(desc);
    BitRange where = BitRange::baseSize(desc.get_offset(), desc.get_nbits());
    return parts.insert(where, writerVas);
}

void
RegisterStateGeneric::eraseWriters(const RegisterDescriptor &desc, const AddressSet &writerVas) {
    if (writerVas.isEmpty() || !writers_.exists(desc))
        return;
    BitAddressSet &parts = writers_[desc];
    BitRange where = BitRange::baseSize(desc.get_offset(), desc.get_nbits());
    parts.erase(where, writerVas);
    if (parts.isEmpty())
        writers_.erase(desc);
}

void
RegisterStateGeneric::setWriters(const RegisterDescriptor &desc, const AddressSet &writerVas) {
    if (writerVas.isEmpty()) {
        eraseWriters(desc);
    } else {
        BitAddressSet &parts = writers_.insertMaybeDefault(desc);
        BitRange where = BitRange::baseSize(desc.get_offset(), desc.get_nbits());
        parts.replace(where, writerVas);
    }
}

// [Robb P. Matzke 2015-08-07]: deprecated
void
RegisterStateGeneric::set_latest_writer(const RegisterDescriptor &desc, rose_addr_t writer_va)
{
    setWriters(desc, writer_va);
}

void
RegisterStateGeneric::eraseWriters(const RegisterDescriptor &desc) {
    if (!writers_.exists(desc))
        return;
    BitAddressSet &parts = writers_[desc];
    BitRange where = BitRange::baseSize(desc.get_offset(), desc.get_nbits());
    parts.erase(where);
    if (parts.isEmpty())
        writers_.erase(desc);
}

// [Robb P. Matzke 2015-08-07]: deprecated
void
RegisterStateGeneric::clear_latest_writer(const RegisterDescriptor &desc)
{
    eraseWriters(desc);
}

void
RegisterStateGeneric::eraseWriters() {
    writers_.clear();
}

// [Robb P. Matzke 2015-08-07]: deprecated
void
RegisterStateGeneric::clear_latest_writers()
{
    eraseWriters();
}

bool
RegisterStateGeneric::hasWritersAny(const RegisterDescriptor &desc) const {
    if (!writers_.exists(desc))
        return false;
    const BitAddressSet &parts = writers_[desc];
    BitRange where = BitRange::baseSize(desc.get_offset(), desc.get_nbits());
    return parts.isOverlapping(where);
}

bool
RegisterStateGeneric::hasWritersAll(const RegisterDescriptor &desc) const {
    if (!writers_.exists(desc))
        return false;
    const BitAddressSet &parts = writers_[desc];
    BitRange where = BitRange::baseSize(desc.get_offset(), desc.get_nbits());
    return parts.contains(where);
}

RegisterStateGeneric::AddressSet
RegisterStateGeneric::getWritersUnion(const RegisterDescriptor &desc) const {
    if (!writers_.exists(desc))
        return AddressSet();
    const BitAddressSet &parts = writers_[desc];
    BitRange where = BitRange::baseSize(desc.get_offset(), desc.get_nbits());
    return parts.getUnion(where);
}

RegisterStateGeneric::AddressSet
RegisterStateGeneric::getWritersIntersection(const RegisterDescriptor &desc) const {
    if (!writers_.exists(desc))
        return AddressSet();
    const BitAddressSet &parts = writers_[desc];
    BitRange where = BitRange::baseSize(desc.get_offset(), desc.get_nbits());
    return parts.getIntersection(where);
}

// [Robb P. Matzke 2015-08-07]: deprecated, use getWritersUnion instead
std::set<rose_addr_t>
RegisterStateGeneric::get_latest_writers(const RegisterDescriptor &desc) const
{
    AddressSet writerVas = getWritersUnion(desc);
    std::set<rose_addr_t> retval(writerVas.values().begin(), writerVas.values().end());
    return retval;
}

bool
RegisterStateGeneric::hasPropertyAny(const RegisterDescriptor &reg, InputOutputProperty prop) const {
    if (!properties_.exists(reg))
        return false;
    const BitProperties &bitProps = properties_[reg];
    BitRange where = BitRange::baseSize(reg.get_offset(), reg.get_nbits());
    return bitProps.existsAnywhere(where, prop);
}

bool
RegisterStateGeneric::hasPropertyAll(const RegisterDescriptor &reg, InputOutputProperty prop) const {
    if (!properties_.exists(reg))
        return false;
    const BitProperties &bitProps = properties_[reg];
    BitRange where = BitRange::baseSize(reg.get_offset(), reg.get_nbits());
    return bitProps.existsEverywhere(where, prop);
}

InputOutputPropertySet
RegisterStateGeneric::getPropertiesUnion(const RegisterDescriptor &reg) const {
    if (!properties_.exists(reg))
        return InputOutputPropertySet();
    const BitProperties &bitProps = properties_[reg];
    BitRange where = BitRange::baseSize(reg.get_offset(), reg.get_nbits());
    return bitProps.getUnion(where);
}

InputOutputPropertySet
RegisterStateGeneric::getPropertiesIntersection(const RegisterDescriptor &reg) const {
    if (!properties_.exists(reg))
        return InputOutputPropertySet();
    const BitProperties &bitProps = properties_[reg];
    BitRange where = BitRange::baseSize(reg.get_offset(), reg.get_nbits());
    return bitProps.getIntersection(where);
}

bool
RegisterStateGeneric::insertProperties(const RegisterDescriptor &reg, const InputOutputPropertySet &props) {
    if (props.isEmpty())
        return false;
    BitProperties &bitProps = properties_.insertMaybeDefault(reg);
    BitRange where = BitRange::baseSize(reg.get_offset(), reg.get_nbits());
    return bitProps.insert(where, props);
}

bool
RegisterStateGeneric::eraseProperties(const RegisterDescriptor &reg, const InputOutputPropertySet &props) {
    if (props.isEmpty() || !properties_.exists(reg))
        return false;
    BitProperties &bitProps = properties_[reg];
    BitRange where = BitRange::baseSize(reg.get_offset(), reg.get_nbits());
    bool changed = bitProps.erase(where, props);
    if (bitProps.isEmpty())
        properties_.erase(reg);
    return changed;
}

void
RegisterStateGeneric::setProperties(const RegisterDescriptor &reg, const InputOutputPropertySet &props) {
    if (props.isEmpty()) {
        eraseProperties(reg);
    } else {
        BitProperties &bitProps = properties_.insertMaybeDefault(reg);
        BitRange where = BitRange::baseSize(reg.get_offset(), reg.get_nbits());
        bitProps.replace(where, props);
    }
}

void
RegisterStateGeneric::eraseProperties(const RegisterDescriptor &reg) {
    if (!properties_.exists(reg))
        return;
    BitProperties &bitProps = properties_[reg];
    BitRange where = BitRange::baseSize(reg.get_offset(), reg.get_nbits());
    bitProps.erase(where);
    if (bitProps.isEmpty())
        properties_.erase(reg);
}

void
RegisterStateGeneric::eraseProperties() {
    properties_.clear();
}

std::vector<RegisterDescriptor>
RegisterStateGeneric::findProperties(const InputOutputPropertySet &required, const InputOutputPropertySet &prohibited) const {
    std::vector<RegisterDescriptor> retval;
    typedef Sawyer::Container::IntervalSet<BitRange> Bits;
    BOOST_FOREACH (const RegisterProperties::Node &regNode, properties_.nodes()) {
        unsigned majr = regNode.key().majr;
        unsigned minr = regNode.key().minr;
        Bits bits;
        BOOST_FOREACH (const BitProperties::Node &bitNode, regNode.value().nodes()) {
            if (bitNode.value().existsAll(required) && !bitNode.value().existsAny(prohibited))
                bits.insert(bitNode.key());
        }
        BOOST_FOREACH (const BitRange &bitRange, bits.intervals()) {
            RegisterDescriptor reg(majr, minr, bitRange.least(), bitRange.size());
            retval.push_back(reg);
        }
    }
    return retval;
}

bool
RegisterStateGeneric::merge(const BaseSemantics::RegisterStatePtr &other_, RiscOperators *ops) {
    ASSERT_not_null(ops);
    RegisterStateGenericPtr other = boost::dynamic_pointer_cast<RegisterStateGeneric>(other_);
    ASSERT_not_null(other);
    bool changed = false;

    // Merge values stored in registers.
    BOOST_FOREACH (const RegPair &otherRegVal, other->get_stored_registers()) {
        const RegisterDescriptor &otherReg = otherRegVal.desc;
        const BaseSemantics::SValuePtr &otherValue = otherRegVal.value;
        if (is_partly_stored(otherReg)) {
            BaseSemantics::SValuePtr dflt = ops->undefined_(otherReg.get_nbits());
            BaseSemantics::SValuePtr thisValue = readRegister(otherReg, dflt, ops);
            if (BaseSemantics::SValuePtr merged = thisValue->createOptionalMerge(otherValue, merger(),
                                                                                 ops->solver()).orDefault()) {
                writeRegister(otherReg, merged, ops);
                changed = true;
            }
        } else {
            writeRegister(otherReg, otherValue, ops);
            changed = true;
        }
    }

    // Merge writer sets.
    BOOST_FOREACH (const RegisterAddressSet::Node &wmNode, other->writers_.nodes()) {
        const BitAddressSet &otherWriters = wmNode.value();
        BitAddressSet &thisWriters = writers_.insertMaybeDefault(wmNode.key());
        BOOST_FOREACH (const BitAddressSet::Node &otherWritten, otherWriters.nodes()) {
            bool inserted = thisWriters.insert(otherWritten.key(), otherWritten.value());
            if (inserted)
                changed = true;
        }
    }

    // Merge property sets.
    BOOST_FOREACH (const RegisterProperties::Node &otherRegNode, other->properties_.nodes()) {
        const BitProperties &otherBitProps = otherRegNode.value();
        BitProperties &thisBitProps = properties_.insertMaybeDefault(otherRegNode.key());
        BOOST_FOREACH (const BitProperties::Node &otherBitNode, otherBitProps.nodes()) {
            bool inserted = thisBitProps.insert(otherBitNode.key(), otherBitNode.value());
            if (inserted)
                changed = true;
        }
    }

    return changed;
}

void
RegisterStateGeneric::print(std::ostream &stream, Formatter &fmt) const
{
    const RegisterDictionary *regdict = fmt.get_register_dictionary();
    if (!regdict)
        regdict = get_register_dictionary();
    RegisterNames regnames(regdict);

    // First pass is to get the maximum length of the register names; second pass prints
    FormatRestorer oflags(stream);
    size_t maxlen = 6; // use at least this many columns even if register names are short.
    for (int i=0; i<2; ++i) {
        BOOST_FOREACH (const RegPairs &pl, registers_.values()) {
            RegPairs regPairs = pl;
            std::sort(regPairs.begin(), regPairs.end(), sortByOffset);
            BOOST_FOREACH (const RegPair &pair, regPairs) {
                std::string regname = regnames(pair.desc);
                if (!fmt.get_suppress_initial_values() || pair.value->get_comment().empty() ||
                    0!=pair.value->get_comment().compare(regname+"_0")) {
                    if (0==i) {
                        maxlen = std::max(maxlen, regname.size());
                    } else {
                        stream <<fmt.get_line_prefix() <<std::setw(maxlen) <<std::left <<regname;
                        oflags.restore();
                        if (fmt.get_show_latest_writers()) {
                            // FIXME[Robb P. Matzke 2015-08-12]: This doesn't take into account that different writer sets can
                            // exist for different parts of the register.
                            AddressSet writers = getWritersUnion(pair.desc);
                            if (writers.size()==1) {
                                stream <<" [writer=" <<StringUtility::addrToString(*writers.values().begin()) <<"]";
                            } else if (!writers.isEmpty()) {
                                stream <<" [writers={";
                                for (Sawyer::Container::Set<rose_addr_t>::ConstIterator wi=writers.values().begin();
                                     wi!=writers.values().end(); ++wi) {
                                    stream <<(wi==writers.values().begin()?"":", ") <<StringUtility::addrToString(*wi);
                                }
                                stream <<"}]";
                            }
                        }

                        // FIXME[Robb P. Matzke 2015-08-12]: This doesn't take into account that different property sets can
                        // exist for different parts of the register.  It also doesn't take into account all combinations f
                        // properties -- just a few of the more common ones.
                        if (fmt.get_show_properties()) {
                            InputOutputPropertySet props = getPropertiesUnion(pair.desc);
                            if (props.exists(IO_READ_BEFORE_WRITE)) {
                                stream <<" read-before-write";
                            } else if (props.exists(IO_WRITE) && props.exists(IO_READ)) {
                                // nothing
                            } else if (props.exists(IO_READ)) {
                                stream <<" read-only";
                            } else if (props.exists(IO_WRITE)) {
                                stream <<" write-only";
                            }
                        }

                        stream <<" = ";
                        pair.value->print(stream, fmt);
                        stream <<"\n";
                    }
                }
            }
        }
    }
}



} // namespace
} // namespace
} // namespace
} // namespace
