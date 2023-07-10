// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




#ifndef Sawyer_Container_IntervalSetMap_H
#define Sawyer_Container_IntervalSetMap_H

#include <stddef.h>
#include <boost/foreach.hpp>
#include "IntervalMap.h"
#include "Sawyer.h"

namespace Sawyer {
namespace Container {

/** Mapping from integers to sets.
 *
 *  This container maps integer keys to sets of values and is optimized to store the same set across adjacent keys by using
 *  intervals of the key.  For instance, an IntervalSetMap that maps integer keys to sets of characters is declared like this:
 *
 * @code
 *  typedef Interval<int> IntRange;
 *  typedef Set<char> CharSet;
 *  typedef IntervalSetMap<IntRange, CharSet> IntCharMap;
 *  IntCharMap icmap;
 * @endcode
 *
 *  Such a map stores a set of characters for each integer key, but does so efficiently when the same set is stored across many
 *  consecutive keys.  For instance, one can store the set {'a', 'b'} across a few million keys and use very little storage:
 *
 * @code
 *  icmap.insert(IntRange::baseSize(0,5000000), 'a');
 *  icmap.insert(IntRange::baseSize(0,5000000), 'b');
 * @endcode
 *
 *  At this point @c icmap is storing 'a' and 'b' at every key from 0 through 4999999, inclusive.  This could also have been
 *  done by constructing the set first and then inserting the set.  The real power of this container comes from the fact that
 *  one can insert values without regard for what intervals currently exist.  For instance, we now insert a few more characters:
 *
 * @code
 *  icmap.insert(5, 'c'); // 5 is a singleton range
 *  icmap.insert(IntRange::hull(10,19), 'd');
 * @endcode
 *
 *  Now @p icmap stores {'a', 'b'} at keys 0 through 4, {'a', 'b', 'c'} at key 5, {'a', 'b'} at keys 6 through 9, {'a', 'b',
 *  'd'} at keys 10 through 19, and {'a', 'b'} at keys 20 through 4999999.
 *
 *  Erasing values works similarly: one can erase a character from an interval or single key without regard for what intervals
 *  already exist.  Attempting to erase a character from a set that doesn't contain the character is a no-op.  Here we erase
 *  'b' and 'e' from large parts of the map key space:
 *
 * @code
 *  icmap.erase(icmap.hull(), 'b'); // erase 'b' from everywhere
 *  icmap.erase(IntRange::hull(-1000000,1000000), 'e');
 * @endcode
 *
 *  Querying is also quite efficient. Here we obtain the set of all values stored in the map's sets:
 *
 * @code
 *  Set allValues = icmap.getUnion(icmap.hull());
 * @endcode
 *
 *  There are also predicates to determine whether a key or value is present in the map.
 *
 * @code
 *  icmap.contains(IntRange::hull(10,19)); // Do keys 10 through 19 all have non-empty sets?
 *  icmap.containsAnywhere(icmap.hull(), 'b'); // Is value 'b' present anywhere in the map?
 *  icmap.containsEverywhere(IntRange::hull(10,19), 'a'); // Is value 'a' present for all keys 10 through 19?
 * @endcode
 *
 *  The @p S template parameter is the set type and must implement the API defined by @ref Sawyer::Container::Set.
 *
 * @sa
 *
 *  See @ref IntervalMap for a similar container whose values don't act like sets. */
template<typename I, typename S>
class IntervalSetMap: public IntervalMap<I, S> {
    typedef IntervalMap<I, S> Super;
public:
    typedef I Interval;                                 /**< Interval type for keys. */
    typedef S Set;                                      /**< Set type for values. */

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Iterators
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Union of values over an interval of keys.
     *
     *  Returns the union of the sets stored across an interval of keys. */
    Set getUnion(const Interval &interval) const {
        Set retval;
        BOOST_FOREACH (const typename Super::Node &node, this->findAll(interval)) {
            BOOST_FOREACH (const typename Set::Value &member, node.value().values()) {
                retval.insert(member);
            }
        }
        return retval;
    }

    /** Intersection of values over an interval of keys.
     *
     *  Returns the set of values that are present for all keys in the interval. */
    Set getIntersection(const Interval &interval) const {
        Set retval;
        size_t nNodes = 0;
        BOOST_FOREACH (const typename Super::Node &node, this->findAll(interval)) {
            const Set &set = this->get(node.key().least());
            if (1 == ++nNodes) {
                retval = set;
            } else {
                retval &= set;
            }
        }
        return retval;
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Predicates and queries
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Determines if values are stored for an interval.
     *
     *  Returns true if <code>get(interval)</code> would return a non-empty set. */
    bool exists(const Interval &interval) const {
        return Super::contains(interval);
    }

    /** Determines if a particular value is stored in an interval.
     *
     *  Returns true if <code>getUnion(interval)</code> would return a set containing @p value as a member. In particular, this
     *  returns false if the @p interval is empty. This is more efficient than calling <code>getUnion(interval)</code> and
     *  checking whether it contains @p value. */
    bool existsAnywhere(const Interval &interval, const typename Set::Value &value) const {
        BOOST_FOREACH (const typename Super::Node &node, this->findAll(interval)) {
            if (node.value().exists(value))
                return true;
        }
        return false;
    }

    /** Determines if a particular value is stored everywhere in the interval.
     *
     *  Returns true if <code>getIntersection(interval)</code> would return a set containing @p value as a member. In
     *  particular, this returns false if the @p interval is empty. This is more efficient than calling
     *  <code>getIntersection(interval)</code> and checking whether it contains @p value. */
    bool existsEverywhere(const Interval &interval, const typename Set::Value &value) const {
        if (interval.isEmpty())
            return false;
        BOOST_FOREACH (const typename Super::Node &node, this->findAll(interval)) {
            if (!node.value().exists(value))
                return false;
        }
        return true;
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Mutators
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:

    /** Erase sets for an interval.
     *
     *  Erases the sets associated with the given interval of keys. */
    void erase(const Interval &interval) {
        Super::erase(interval);
    }

    /** Erases one value from a set over an interval.
     *
     *  Erases the specified @p value from all sets over the specified @p interval of keys. Any sets that become empty are
     *  removed from the map as if @c erase had been called on that sub-interval. */
    bool erase(const Interval &interval, const typename Set::Value &value) {
        Set set;
        set.insert(value);
        return erase(interval, set);
    }

    /** Erase specified values from the sets of an interval.
     *
     *  Erases the specified values from all sets over the specified @p interval of keys. Any sets that become empty are
     *  removed from the map as if single-argument @c erase hd been called on that sub-interval.  Returns true if any values
     *  were erased, false if none of the values were members of the affected sets. */
    bool erase(const Interval &interval, const Set &values) {
        bool isErased = false;
        Interval worklist = interval;
        while (!worklist.isEmpty()) {
            typename Super::ConstNodeIterator iter = this->findFirstOverlap(worklist);
            if (iter == this->nodes().end()) {
                break;
            } else if (worklist.least() < iter->key().least()) {
                worklist = Interval::hull(iter->key().least(), worklist.greatest());
            } else {
                Interval work = worklist.intersection(iter->key());
                Set set = this->get(work.least());
                if (set.erase(values)) {
                    replace(work, set);
                    isErased = true;
                }
                if (work == worklist)
                    break;
                worklist = Interval::hull(work.greatest()+1, worklist.greatest());
            }
        }
        return isErased;
    }

    /** Insert one value to the sets of an interval.
     *
     *  Inserts the specified @p value to all sets in the @p interval of keys. Returns true if the value was inserted anywhere,
     *  false if the value already existed everywhere. */
    bool insert(const Interval &interval, const typename Set::Value &value) {
        Set set;
        set.insert(value);
        return insert(interval, set);
    }

    /** Insert a set of values into the sets of an interval.
     *
     *  Inserts the specified values into all sets in the @p interval of keys.  Returns true if any value was inserted
     *  anywhere, false if all values already existed in the sets of all specified keys. */
    bool insert(const Interval &interval, const Set &values) {
        bool isInserted = false;
        Interval worklist = interval;
        while (!worklist.isEmpty()) {
            typename Super::ConstNodeIterator iter = this->findFirstOverlap(worklist);
            Set set;
            Interval work;
            if (iter == this->nodes().end()) {
                work = worklist;
            } else if (worklist.least() < iter->key().least()) {
                work = Interval::hull(worklist.least(), iter->key().least() - 1);
            } else {
                work = worklist.intersection(iter->key());
                set = this->get(work.least());
            }
            if (set.insert(values)) {
                Super::insert(work, set);
                isInserted = true;
            }
            if (work == worklist)
                break;
            worklist = Interval::hull(work.greatest()+1, worklist.greatest());
        }
        return isInserted;
    }
    
    /** Replace sets with a new set.
     *
     *  Replaces sets for keys in the specified @p interval with the specified @p set. */
    void replace(const Interval &interval, const Set &set) {
        if (set.isEmpty()) {
            Super::erase(interval);
        } else {
            Super::insert(interval, set);
        }
    }
};

} // namespace
} // namespace

#endif
