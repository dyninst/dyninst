#include "../util/StringUtility.h"
//#include "sage3basic.h"
#include "Registers.h"
#include "RegisterParts.h"
#include <boost/foreach.hpp>

namespace rose {
namespace BinaryAnalysis {

void
RegisterParts::erase(const RegisterDescriptor &reg) {
    if (map_.exists(reg)) {
        BitSet &bits = map_[reg];
        bits.erase(bitRange(reg));
        if (bits.isEmpty())
            map_.erase(reg);
    }
}

RegisterParts&
RegisterParts::operator-=(const RegisterParts &other) {
    BOOST_FOREACH (const Map::Node &node, other.map_.nodes()) {
        BOOST_FOREACH (const BitRange &bits, node.value().intervals())
            erase(RegisterDescriptor(node.key().get_major(), node.key().get_minor(), bits.least(), bits.size()));
    }
    return *this;
}

RegisterParts
RegisterParts::operator-(const RegisterParts &other) const {
    RegisterParts retval = *this;
    retval -= other;
    return retval;
}

RegisterParts&
RegisterParts::operator|=(const RegisterParts &other) {
    BOOST_FOREACH (const Map::Node &node, other.map_.nodes())
        map_.insertMaybeDefault(node.key()).insertMultiple(node.value());
    return *this;
}

RegisterParts
RegisterParts::operator|(const RegisterParts &other) const {
    RegisterParts retval = *this;
    retval |= other;
    return retval;
}

RegisterParts&
RegisterParts::operator&=(const RegisterParts &other) {
    BOOST_FOREACH (const Map::Node &node, other.map_.nodes()) {
        if (map_.exists(node.key())) {
            BitSet &set = map_[node.key()];
            set.eraseMultiple(node.value());
            if (set.isEmpty())
                map_.erase(node.key());
        }
    }
    return *this;
}

RegisterParts
RegisterParts::operator&(const RegisterParts &other) const {
    RegisterParts retval = *this;
    retval &= other;
    return retval;
}

std::vector<RegisterDescriptor>
RegisterParts::extract(const RegisterDictionary *regDict, bool extractAll) {
    std::vector<RegisterDescriptor> retval, allRegs;
    if (isEmpty())
        return retval;

    if (regDict) {
        BOOST_FOREACH (const RegisterDictionary::Entries::value_type &pair, regDict->get_registers())
            allRegs.push_back(pair.second);
        std::sort(allRegs.begin(), allRegs.end(), RegisterDictionary::SortBySize(RegisterDictionary::SortBySize::DESCENDING));
        BOOST_FOREACH (const RegisterDescriptor &reg, allRegs) {
            if (existsAll(reg)) {
                retval.push_back(reg);
                erase(reg);
                if (isEmpty())
                    break;
            }
        }
    }

    if (!regDict || extractAll) {
        BOOST_FOREACH (const Map::Node &node, map_.nodes()) {
            BOOST_FOREACH (const BitRange &bits, node.value().intervals())
                retval.push_back(RegisterDescriptor(node.key().get_major(), node.key().get_minor(), bits.least(), bits.size()));
        }
        clear();
    }

    return retval;
}

std::vector<RegisterDescriptor>
RegisterParts::listAll(const RegisterDictionary *regDict) const {
    RegisterParts temp = *this;
    return temp.extract(regDict, true);
}

std::vector<RegisterDescriptor>
RegisterParts::listNamed(const RegisterDictionary *regDict) const {
    RegisterParts temp = *this;
    return temp.extract(regDict, false);
}

} // namespace
} // namespace
