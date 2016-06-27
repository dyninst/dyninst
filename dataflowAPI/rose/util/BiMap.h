// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




#ifndef Sawyer_BiMap_H
#define Sawyer_BiMap_H

#include "Map.h"
#include "Sawyer.h"
#include <boost/foreach.hpp>

namespace Sawyer {
namespace Container {

/** One-to-one mapping between source and target values.
 *
 *  This container holds a one-to-one mapping from values in a domain to values in a range and can look up values in the
 *  forward or reverse direction in log time.  It does so by consistently maintaining two maps: a forward map and a reverse
 *  map.  The forward map maps values in the domain to values in the range, while the reverse map goes the other direction. The
 *  @ref forward and @ref reverse methods return const references to these maps.  The BiMap container provides methods for
 *  modifying the mapping. */
template<class S, class T>
class BiMap {
public:
    typedef S Source;                                   /**< Type of values in the domain. */
    typedef T Target;                                   /**< Type of values in the range. */
    typedef Sawyer::Container::Map<Source, Target> Forward; /**< Type for domain-to-range map. */
    typedef Sawyer::Container::Map<Target, Source> Reverse; /**< Type for range-to-domain map. */
private:
    Forward forward_;
    Reverse reverse_;
public:
    /** Default constructor.
     *
     *  Constructs an empty mapping. */
    BiMap() {}

    /** Copy constructor. */
    BiMap(const BiMap &other);

    /** Construct a new map by composition of two maps.
     *
     *  Given two BiMap objects where the range type of the first is the domain type of the second, construct a new BiMap
     *  from the domain of the first to the range of the second.  The new map will contain only those domain/range pairs that
     *  map across both input maps. */
    template<class U>
    BiMap(const BiMap<Source, U> &a, const BiMap<U, Target> &b) {
        BOOST_FOREACH (const typename Forward::Node &anode, a.forward_.nodes()) {
            if (b.forward_.exists(anode.value())) {
                const Target &target = b.forward_[anode.value()];
                forward_.insert(anode.key(), target);
                reverse_.insert(target, anode.key());
            }
        }
    }

    /** Erase all mappings.
     *
     *  This results in a map whose state is the same as a default-constructed BiMap. */
    void clear() {
        forward_.clear();
        reverse_.clear();
    }
    
    /** Erase domain value and its associated range value.
     *
     *  If @p source exists in this map's domain, then it is removed along with the corresponding value in the range. Otherwise
     *  the map is not modified.  Returns true if the map was modified, false if not. */
    bool eraseSource(const Source &source) {
        if (!forward_.exists(source))
            return false;
        reverse_.erase(forward_[source]);
        forward_.erase(source);
        return true;
    }

    /** Erase range value and its associated domain value.
     *
     *  If @p target exists in this map's range, then it is removed along with the corresponding value in the domain. Otherwise
     *  the map is not modified. Returns true if the map was modified, false if not. */
    bool eraseTarget(const Target &target) {
        if (!reverse_.exists(target))
            return false;
        forward_.erase(reverse_[target]);
        reverse_.erase(target);
        return true;
    }

    /** Erase a specific map entry.
     *
     *  Erases the 1:1 mapping between @p source and @p target if present.  This is a no-op unless @p source is a member of the
     *  domain and @p target is a member of the range and @p source maps to @p target (and vice versa). Returns true if the map
     *  was modified, false if not. */
    bool erase(const Source &source, const Target &target) {
        if (!forward_.exists(source) || forward_[source]!=target)
            return false;
        forward_.erase(source);
        reverse_.erase(target);
        return true;
    }

    /** Insert a new mapping.
     *
     *  Creates a new mapping from @p source in the domain to @p target in the range.  If @p source is already a member of the
     *  domain then it is first erased along with its corresponding value in the range; likewise, if @p target is already a
     *  member of the range then it is first erased along with its corresponding value in the domain. */
    void insert(const Source &source, const Target &target) {
        eraseSource(source);
        eraseTarget(target);
        forward_.insert(source, target);
        reverse_.insert(target, source);
    }

    /** Return forward mapping.
     *
     *  The keys of the forward map are values in the domain, and its values are in the range. */
    const Forward& forward() const {
        return forward_;
    }

    /** Return reverse mapping.
     *
     *  The keys of the reverse map are values in the range, and its values are in the domain. */
    const Reverse& reverse() const {
        return reverse_;
    }
};

} // namespace
} // namespace

#endif
