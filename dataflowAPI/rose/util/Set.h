// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




#ifndef Sawyer_Container_Set_H
#define Sawyer_Container_Set_H

#include "Interval.h"
#include "Sawyer.h"

#include <memory>
#include <stddef.h>
#include <boost/foreach.hpp>
#include <boost/range/iterator_range.hpp>
#include <set>
#include <vector>

namespace Sawyer {
namespace Container {

/** Ordered set of values.
 *
 *  This container holds an ordered set of values. This container differs from std::set in the following ways:
 *
 *  @li The interface uses the Sawyer CamelCase naming scheme where types start with an upper-case letter and lack a "_type" or
 *      "_t" suffix and methods start with a lower-case letter.
 *
 *  @li Like other Sawyer containers, iterators are returned as a range rather than individual begin and end iterators.
 *
 *  @li Only const iterators are supported since it should not be possible to change the value pointed to by the iterator.
 *
 *  @li The "empty" predicate is named @ref isEmpty.
 *
 *  @li The existence predicate is named @ref exists instead of "count".
 *
 *  @li The insert and erase mutators are simplified. They don't take iterator hints and they return a Boolean indication of
 *      whether any operation occurred.
 *
 *  @li The container understands union, intersection, difference, and equality. Although this makes the library larger, it
 *      alleviates the user from needing to invoke a separate function for these operations. */
template<typename T, class C = std::less<T>, class A = std::allocator<T> >
class Set {
    typedef std::set<T, C, A> InternalSet;
    InternalSet set_;
public:
    typedef T Value;                                    /**< Type of values stored in this set. */
    typedef C Comparator;                               /**< How to compare values with each other. */
    typedef A Allocator;                                /**< How to allocate storge for new values. */
    typedef typename InternalSet::const_iterator ConstIterator;  /**< Iterator for traversing values stored in the set. */

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Construction
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Default constructor.
     *
     *  Constructs a new set containing no values. */
    explicit Set(const Comparator &comparator = Comparator(), const Allocator &allocator = Allocator())
        : set_(comparator, allocator) {}

    /** Singleton constructor.
     *
     *  Constructs a singleton set having only the specified value. */
    Set(const Value &value) /*implicit*/ {
        set_.insert(value);
    }

    /** Iterative constructor.
     *
     *  Constructs a new set and copies values into the set.  For instance, this can be used to initialize a set from a vector:
     *
     * @code
     *  Map<Key, int> v = ...;
     *  Set<int> set(v.values());
     * @endcode
     *
     * @{ */
    template<class InputIterator>
    Set(InputIterator begin, InputIterator end,
        const Comparator &comparator = Comparator(), const Allocator &allocator = Allocator())
        : set_(begin, end, comparator, allocator) {}

    template<class InputIterator>
    explicit Set(const boost::iterator_range<InputIterator> &range,
                 const Comparator &/*comparator*/ = Comparator(), const Allocator &/*allocator*/ = Allocator())
        : set_(range.begin(), range.end()) {}
    /** @} */

    /** Copy constructor. */
    Set(const Set &other)
        : set_(other.set_) {}

    /** Assignment operator. */
    Set& operator=(const Set &other) {
        set_ = other.set_;
        return *this;
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Iterators
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Value iterator range.
     *
     *  Returns an iterator range that covers all values in the set. */
    boost::iterator_range<ConstIterator> values() const {
        return boost::iterator_range<ConstIterator>(set_.begin(), set_.end());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Predicates and queries
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Whether the set is empty.
     *
     *  Returns true if the set is empty, false if not empty. */
    bool isEmpty() const {
        return set_.empty();
    }

    /** Whether a value exists.
     *
     *  Returns true if @p value is a member of the set, false if not a member. */
    bool exists(const Value &value) const {
        return 1 == set_.count(value);
    }

    /** Whether any value exists.
     *
     *  Returns true if any of the specified values exist in this set. */
    bool existsAny(const Set &other) const {
        BOOST_FOREACH (const Value &otherValue, other.values()) {
            if (exists(otherValue))
                return true;
        }
        return false;
    }

    /** Whether all values exist.
     *
     *  Returns true if all specified values exist in this set. */
    bool existsAll(const Set &other) const {
        BOOST_FOREACH (const Value &otherValue, other.values()) {
            if (!exists(otherValue))
                return false;
        }
        return true;
    }
    
    /** Size of the set.
     *
     *  Returns the number of values that are members of this set. */
    size_t size() const {
        return set_.size();
    }

    /** Smallest member.
     *
     *  Returns the smallest member of the set. The set must not be empty. */
    Value least() const {
        ASSERT_forbid(isEmpty());
        return *set_.begin();
    }

    /** Largest member.
     *
     *  Returns the largest member of the set. The set must not be empty. */
    Value greatest() const {
        ASSERT_forbid(isEmpty());
        ConstIterator i = set_.end();
        --i;
        return *i;
    }

    /** Range of members.
     *
     *  Returns a range having the minimum and maximum members of the set. */
    Interval<Value> hull() const {
        if (isEmpty())
            return Interval<Value>();
        return Interval<Value>::hull(least(), greatest());
    }

    /** Whether two sets contain the same members.
     *
     *  Returns true if this set and @p other contain exactly the same members. */
    bool operator==(const Set &other) const {
        return set_.size() == other.set_.size() && std::equal(set_.begin(), set_.end(), other.set_.begin());
    }

    /** Whether two sets do not contain the same members.
     *
     *  Returns true if this set and the @p other set are not equal, although this method is faster than using
     *  <code>!(*this==other)</code>. */
    bool operator!=(const Set &other) const {
        return (set_.size() != other.set_.size() ||
                std::mismatch(set_.begin(), set_.end(), other.set_.begin()).first != set_.end());
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Mutators
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Insert a value.
     *
     *  Inserts @p value into the set. Returns true if the value was inserted, false if the value was already a member. */
    bool insert(const Value &value) {
        return set_.insert(value).second;
    }

    /** Insert multiple values.
     *
     *  Inserts all specified values into this set. Returns true if any value was inserted, false if all the values were
     *  already members of this set. */
    bool insert(const Set &values) {
        bool isInserted = false;
        BOOST_FOREACH (const Value &value, values.values()) {
            if (set_.insert(value).second)
                isInserted = true;
        }
        return isInserted;
    }

    /** Erase a value.
     *
     *  Erases @p value from the set. Returns true if the value was erased, false if the value was not a member. */
    bool erase(const Value &value) {
        return 1 == set_.erase(value);
    }

    /** Erase multiple values.
     *
     *  Erases all specified values from this set. Returns true if any value was erased, false if none of the specified values
     *  were members of this set. */
    bool erase(const Set &values) {
        bool isErased = false;
        BOOST_FOREACH (const Value &value, values.values()) {
            if (1 == set_.erase(value))
                isErased = true;
        }
        return isErased;
    }
    
    /** Erase all values.
     *
     *  Erases all values from the set so that the set becomes empty. */
    void clear() {
        set_.clear();
    }

    /** Intersects this set with another.
     *
     *  Removes those members of this set that are not in the @p other set. */
    Set& operator&=(const Set &other) {
        std::vector<Value> toErase;
        toErase.reserve(set_.size());
        BOOST_FOREACH (const Value &value, set_) {
            if (!other.exists(value))
                toErase.push_back(value);
        }
        BOOST_FOREACH (const Value &value, toErase)
            set_.erase(value);
        return *this;
    }

    /** Unions this set with another.
     *
     *  Adds those members of @p other that are not already members of this set. */
    Set& operator|=(const Set &other) {
        BOOST_FOREACH (const Value &v, other.values())
            set_.insert(v);
        return *this;
    }

    /** Differences two sets.
     *
     *  Removes those members of this set that are in the @p other set.  This is like the intersection of the complement but
     *  does not require computing a potentially large complement. */
    Set& operator-=(const Set &other) {
        std::vector<Value> toErase;
        toErase.reserve(set_.size());
        BOOST_FOREACH (const Value &value, set_) {
            if (other.exists(value))
                toErase.push_back(value);
        }
        BOOST_FOREACH (const Value &value, toErase)
            set_.erase(value);
        return *this;
    }

        
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Set-theoretic operations
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Compute the intersection of this set with another.
     *
     *  Returns a new set which has only those members that are common to this set and the @p other set. */
    Set operator&(const Set &other) const {
        Set retval = *this;
        retval &= other;
        return retval;
    }

    /** Compute the union of this set with another.
     *
     *  Returns a new set containing the union of all members of this set and the @p other set. */
    Set operator|(const Set &other) const {
        Set retval = *this;
        retval |= other;
        return retval;
    }

    /** Compute the difference of this set with another.
     *
     *  Returns a new set containing those elements of @p this set that are not members of the @p other set. */
    Set operator-(const Set &other) const {
        Set retval = *this;
        retval -= other;
        return retval;
    }
};

} // namespace
} // namespace

#endif
