// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




#include "Attribute.h"
#include "BiMap.h"
#include <boost/foreach.hpp>
#include <boost/version.hpp>

namespace Sawyer {
namespace Attribute {

const Id INVALID_ID(-1);

typedef Sawyer::Container::BiMap<Id, std::string> DefinedAttributes;
static DefinedAttributes definedAttributes;
static Id nextId = 0;

SAWYER_EXPORT Id
declare(const std::string &name) {
    Id retval = INVALID_ID;
    if (definedAttributes.reverse().getOptional(name).assignTo(retval))
        throw AlreadyExists(name, retval);
    retval = nextId++;
    definedAttributes.insert(retval, name);
    return retval;
}

SAWYER_EXPORT Id
id(const std::string &name) {
    return definedAttributes.reverse().getOptional(name).orElse(INVALID_ID);
}

SAWYER_EXPORT const std::string&
name(Id id) {
    return definedAttributes.forward().getOrDefault(id);
}

void
Storage::checkBoost() const {
    // We do not support boost 1.54 with C++11 because of boost ticket #9215 [https://svn.boost.org/trac/boost/ticket/9215]. It
    // is better if we check for this at runtime rather than compile time because many other features of boost 1.54 work
    // fine. If we allow the user to continue, then boost::any's move constructor will enter infinite recursion eventually
    // ending with a segmentation fault (although probably not occuring as a result of calling just Storage's c'tor).
    ASSERT_always_forbid2(BOOST_VERSION == 105400 && __cplusplus >= 201103L,
                          "boost::any move constructor has infinite recursion in boost-1.54");
}

SAWYER_EXPORT std::vector<Id>
Storage::attributeIds() const {
    std::vector<Id> retval;
    retval.reserve(values_.size());
    BOOST_FOREACH (Id id, values_.keys())
        retval.push_back(id);
    return retval;
}

} // namespace
} // namespace
