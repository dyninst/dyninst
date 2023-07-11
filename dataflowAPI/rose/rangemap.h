// This interface will be deprecated at some point in the not-to-distant future. Use Sawyer::Container::IntervalMap instead.



#ifndef ROSE_RANGEMAP_H
#define ROSE_RANGEMAP_H

/* This file defines four main template classes:
 *    - RangeMap:      Time and space efficient storage of non-overlapping Range objects each associated with an arbitrary value.
 *    - Range:         Contiguous set of integers defined by the starting value and size.
 *    - RangeMapVoid:  Void value for RangeMap classes that don't store any useful value (just the ranges themselves).
 *    - RangeMapValue: Class for storing simple values in a RangeMap.
 */


/* There is no need to include "sage3basic.h"; this file defines all it needs. */

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stddef.h>
#include <utility>
#include <inttypes.h>

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

/* Define this if you want the class to do fairly extensive testing of the consistency of the map after every operation.  Note
 * that this substantially increases execution time.  The NDEBUG preprocessor symbol must not be defined, or else the check()
 * method won't really do anything. */
//#define RANGEMAP_CHECK

/******************************************************************************************************************************
 *                                      Range<>
 ******************************************************************************************************************************/

/** A contiguous range of values.  Represents a contiguous range of @p size values beginning at @p begin, and defines
 *  relationships between two ranges.  The ranges are designed such that they can represent unsigned values up to and
 *  including the maximum possible values for the data type.  However, this means that a range that represents all possible
 *  values will have a size() of zero due to overflow.
 *
 *  Floating point ranges are also possible (types "float" or "double") but the behavior of some methods differs slightly from
 *  integers.  For instance, an integer range containing a single value has size 1, but a floating point range with one value
 *  has size zero.  The differences are noted in the documentation for the particular methods affected. */
template<class T>
class Range {
public:
    typedef T Value;
    typedef std::pair<Range, Range> Pair;               /**< A pair of ranges. */

protected:
    Value r_first;                                      /**< First value in range. */
    Value r_last;                                       /**< Last value in range. */

public:
    /** Create an empty range.  Ranges may have an empty size, but empty ranges will never appear inside a RangeMap object.
     *  The @p begin value of an empty range is meaningless. */
    Range()
        : r_first(1), r_last(0) {} // see clear()

    /** Create a new range of unit size. The new range contains only the specified value. */
    explicit Range(const Value &first)
        : r_first(first), r_last(first) {}

    /** Create a new range of specified size. If @p size is zero then an empty range is created.  Note that  a zero size is also
     *  returned for a range that contains all values, but this is due to overflow.  Whether this is an integer range or a
     *  floating point range, if size is zero then the range is considered to be empty (but the @p first value is remembered).
     *  To create a floating point range with a single value, use the single-argument constructor. */
    Range(const Value &first, const Value &size)
        : r_first(first), r_last(first+size-1) {
        if (0==size) {
            r_last = first; // or else clear() is a no-op leaving invalid values
            clear();
        } else {
            assert(!empty()); // checks for overflow
        }
    }

    /** Create a new range from a different range. */
    template<class Other>
    explicit Range(const Other &other)
        : r_first(other.relaxed_first()), r_last(other.relaxed_last()) {}

    /** Create a new range by giving the first (inclusive) and last value (inclusive).  This is the only way to create a range
     * that contains all values since the size of such a range overflows the range's Value type.  If the two values are equal
     * then the created range contains only that value; if the first value is larger than the second then the range is
     * considered to be empty. */
    static Range inin(const Value &v1, const Value &v2) {
        assert(v1<=v2);
        Range retval;
        retval.first(v1);
        retval.last(v2);
        return retval;
    }

    /** Accessor for the first value of a range.  It does not make sense to ask for the first value of an empty range, and an
     *  assertion will fail if such a request is made.  However, relaxed_first() will return a value anyway.
     *  @{ */
    void first(const Value &first) {
        r_first = first;
    }
    const Value first() const {
        assert(!empty());
        return r_first;
    }

// DQ (9/3/2015): Intel v14 compiler warns that use of "const" is meaningless.
// I think this is correct since this is being returned by value.
//  const Value 
    Value 
    relaxed_first() const {
        if (!empty())
            return first();
        if (1==r_first-r_last)
            return r_first-1;
        return r_first;
    }
    /** @} */

    /** Accessor for the last value of a range.  It does not make sense to ask for the last value of an empty range, and an
     *  assertion will fail if such a request is made.  However, relaxed_last() will return a value anyway.
     *  @{ */
    void last(const Value &last) {
        r_last = last;
    }
    const Value last() const {
        assert(!empty());
        return r_last;
    }
    const Value relaxed_last() const {
        return empty() ? relaxed_first() : last();
    }
    /** @} */

    /** Returns the number of values represented by the range. Note that if the range contains all possible values then the
     *  returned size may be zero due to overflow, in which case the empty() method should also be called to make the
     *  determination.
     *
     *  Floating point range sizes are simply the last value minus the first value.  Therefore, a singleton floating point range
     *  will return a size of zero, while a singleton integer range will return a size of one.  This is actualy consistent
     *  behavior if you think of an integer value N as the floating point range [N,N+1), where N is included in the range but
     *  N+1 is not. */
    Value size() const {
        if (empty())
            return 0;
        return r_last + 1 - r_first;
    }

    /** Sets the range size by adjusting the maximum value.  It is an error to change the size of a range from zero to
     *  non-zero, but the relaxed_resize() is available for that purpose.
     *
     *  Setting the size to zero causes different behavior for integer ranges than it does for floating point ranges.  For
     *  integer ranges, setting the size to zero clears the range (makes it empty); for floating point ranges, setting the size
     *  to zero causes the range to contain only the starting value.  Floating point ranges can be cleared by setting the new
     *  size to a negative value.  The clear() method should be favored over resize() for making a range empty.
     *
     *  @{ */
    void resize(const Value &new_size) {
        assert(!empty());
        if (0==new_size) {
            clear();
        } else {
            r_last = r_first + new_size - 1;
            assert(!empty()); // this one is to check for overflow of r_last
        }
    }
    void relaxed_resize(const Value &new_size) {
        if (0==new_size) {
            clear();
        } else {
            r_first = relaxed_first();
            r_last = r_first + new_size - 1;
            assert(!empty()); // this one is to check for overflow of r_last
        }
    }
    /** @} */

    /** Split a range into two parts.  Returns a pair of adjacent ranges such that @p at is the first value of the second
     *  returned range.  The split point must be such that neither range is empty. */
    Pair split_range_at(const Value &at) const {
        assert(!empty());
        assert(at>first() && at<=last()); // neither half can be empty
        Range left(first(), at-first());
        Range right(at, last()+1-at);
        return std::make_pair(left, right);
    }

    /** Joins two adjacent ranges.  This range must be the left range, and the argument is the right range. They must be
     *  adjacent without overlapping. If one of the ranges is empty, then the return value is the other range (which might also
     *  be empty). */
    Range join(const Range &right) const {
        if (empty())
            return right;
        if (right.empty())
            return *this;
        assert(abuts_lt(right));
        return Range::inin(first(), right.last());
    }

    /** Erase part of a range to return zero, one, or two new ranges.  The possible situations are:
     * <ol>
     *   <li>The range to erase can be a superset of this range, in which case this entire range is erased and nothing is
     *       returned.</li>
     *   <li>The range to erase can be empty, in which case this range is returned.</li>
     *   <li>The range to erase does not intersect this range, in which case this range is returned.</li>
     *   <li>The range to erase can overlap the low end of this range, in which case only the non-overlapping high end of this
     *       range is returned.</li>
     *   <li>The range to erase can overlap the high end of this range, in which case only the non-overlapping low end of this
     *       range is returned.</li>
     *   <li> The range  to erase overlaps only the middle part of this range, in which case two ranges are returned: the
     *        non-overlapping low end of this range, and the non-overlapping high end of this range.</li>
     * </ol>
     */
    std::vector<Range> erase(const Range &to_erase) const {
        std::vector<Range> retval;
        if (to_erase.empty() || distinct(to_erase)) {
            retval.push_back(*this);
        } else if (contained_in(to_erase)) {
            // void
        } else {
            if (begins_before(to_erase))
                retval.push_back(Range::inin(first(), to_erase.first()-1));
            if (ends_after(to_erase))
                retval.push_back(Range::inin(to_erase.last()+1, last()));
        }
        return retval;
    }

    /** Intersection of two ranges. */
    Range intersect(const Range &other) const {
        if (empty() || contains(other))
            return other;
        if (other.empty() || other.contains(*this))
            return *this;
        if (!overlaps(other))
            return Range();
        return Range::inin(
        std::max(first(), other.first()),
        std::min(last(), other.last()));
    }
    
    /** Returns true if this range is empty.  Note that many of the range comparison methods have special cases for empty
     *  ranges.  Note that due to overflow, the size() method may return zero for integer ranges if this range contains all
     *  possible values.  It follows, then that the expressions "empty()" and "0==size()" are not always equal. */
    bool empty() const {
        return r_last<r_first; // can't use first() or last() here because they're not valid for empty ranges.
    }

    /** Make a range empty.  An empty range is one in which r_first is greater than r_last.  We make special provisions here so
     *  that relaxed_first() will continue to return the same value as it did before the range was set to empty. */
    void clear() {
        if (!empty()) {
            if (r_first<maximum()) {
                r_first = r_first + 1;
                r_last = r_first - 1;
            } else {
                r_last = r_first - 2;
            }
            assert(empty());
        }
    }

    /** Do both ranges begin at the same place?
     *
     *  An empty range never begins with any other range, including other empty ranges. */
    bool begins_with(const Range &x) const {
        if (empty() || x.empty())
            return false;
        return first() == x.first();
    }

    /** Do both ranges end at the same place?
     *
     *  An empty range never ends with any other range, including other empty ranges. */
    bool ends_with(const Range &x) const {
        if (empty() || x.empty())
            return false;
        return last() == x.last();
    }

    /** Does this range begin (strictly) after the beginning of another range?
     *
     *  An empty range never begins after any other range, including other empty ranges. */
    bool begins_after(const Range &x, bool strict=true) const {
        if (empty() || x.empty())
            return false;
        return strict ? first() > x.first() : first() >= x.first();
    }

    /** Does this range begin (strictly) before the beginning of another range?
     *
     *  An empty range never begins before any other range, including other empty ranges. */
    bool begins_before(const Range &x, bool strict=true) const {
        if (empty() || x.empty())
            return false;
        return strict ? first() < x.first() : first() <= x.first();
    }

    /** Does this range end (strictly) after the end of another range?
     *
     *  An empty range never ends after any other range, including other empty ranges. */
    bool ends_after(const Range &x, bool strict=true) const {
        if (empty() || x.empty())
            return false;
        return strict ? last() > x.last() : last() >= x.last();
    }

    /** Does this range end (strictly) before the end of another range?
     *
     *  An empty range never ends before any other range, including other empty ranges. */
    bool ends_before(const Range &x, bool strict=true) const {
        if (empty() || x.empty())
            return false;
        return strict ? last() < x.last() : last() <= x.last();
    }

    /** Does this range contain the argument range?
     *
     *  The argument is contained in this range if the argument starts at or after the start of this range and ends at or
     *  before the end of this range.  If @p strict is true, then the comparisons do not include equality. An empty range does
     *  not contain any other range, including other empty ranges. */
    bool contains(const Range &x, bool strict=false) const {
        if (empty() || x.empty())
            return false;
        return strict ? x.first()>first() && x.last()<last() : x.first()>=first() && x.last()<=last();
    }

    /** Is this range contained in the argument range?
     *
     *  This range is contained in the argument range if this range starts at or after the start of the argument and ends at or
     *  before the end of the argument. If @p strict is true, then the comparisons do not include equality. An empty range does
     *  not contain any other range, including other empty ranges. */
    bool contained_in(const Range &x, bool strict=false) const {
        if (empty() || x.empty())
            return false;
        return strict ? first()>x.first() && last()<x.last() : first()>=x.first() && last()<=x.last();
    }

    /** Are two ranges equal?
     *
     *  They are equal if the start and end at the same place or if they are both empty. */
    bool congruent(const Range &x) const {
        if (empty() || x.empty())
            return empty() && x.empty();
        return first()==x.first() && last()==x.last();
    }

    /** Is this range left of the argument range?
     *
     *  This range is left of the argument range if this range ends before the start of the argument. They may adjoin, but must
     *  not overlap.  An empty range is never left of any other range, including other empty ranges. */
    bool left_of(const Range &x) const {
        if (empty() || x.empty())
            return false;
        return last() < x.first();
    }

    /** Is this range right of the argument range?
     *
     *  This range is right of the argument range if this range starts after the end of the argument range.  They may adjoin,
     *  but must not overlap.  An empty range is never right of any other range, including other empty ranges. */
    bool right_of(const Range &x) const {
        if (empty() || x.empty())
            return false;
        return first() > x.last();
    }

    /** Does this range overlap with the argument range?
     *
     *  An empty range does not overlap with any other rance, including other empty ranges. */
    bool overlaps(const Range &x) const {
        if (empty() || x.empty())
            return false;
        return !left_of(x) && !right_of(x);
    }

    /** Is this range non-overlapping with the argument range?
     *
     *  In other words, do the two ranges represent distinct sets of values? An empty range is always distinct from all other
     *  ranges (including other empty ranges). */
    bool distinct(const Range &x) const {
        if (empty() || x.empty())
            return true;
        return !overlaps(x);
    }

    /** Is this range immediately left of the argument range?
     *
     *  Returns true if this range ends at the beginning of the argument, with no overlap and no space between them. An empty
     *  range does not abut any other range, including other empty ranges. */
    bool abuts_lt(const Range &x) const {
        if (empty() || x.empty())
            return false;
        return last()+1 == x.first();
    }

    /** Is this range immediately right of the argument range?
     *
     *  Returns true if this range begins at the end of the argument, with no overlap and no space between them.  An empty
     *  range does not abut any other range, including other empty ranges. */
    bool abuts_gt(const Range &x) const {
        if (empty() || x.empty())
            return false;
        return first() == x.last()+1;
    }

    /** Return the minimum possible value represented by this range. */
    static Value minimum() {
        return 0; // FIXME
    }

    /** Return the maximum possible value represented by this range. */
    static Value maximum() {
        return (Value)(-1); // FIXME
    }

    /** Return a range that covers all possible values. */
    static Range all() {
        return Range::inin(minimum(), maximum());
    }

    bool operator==(const Range &x) const {
        return congruent(x);
    }
    bool operator!=(const Range &x) const {
        return !congruent(x);
    }
    
    void print(std::ostream &o) const {
        if (empty()) {
            o <<"<empty>";
        } else if (first()==last()) {
            o <<first();
        } else {
            o <<first() <<".." <<last();
        }
    }

    friend std::ostream& operator<<(std::ostream &o, const Range &x) {
        x.print(o);
        return o;
    }
};

/******************************************************************************************************************************
 *                                      Specializations for Range<double>
 ******************************************************************************************************************************/

template<>
Range<double>::Range();

template<>
bool
Range<double>::empty() const;

template<>
void
Range<double>::clear();

template<>
// DQ (9/3/2015): Intel v14 compiler warns that use of "const" is meaningless.
// I think this is correct since this is being returned by value.
// const double
double
Range<double>::relaxed_first() const;

template<>
double
Range<double>::size() const;

template<>
void
Range<double>::resize(const double &new_size);

template<>
void
Range<double>::relaxed_resize(const double &new_size);

template<>
Range<double>::Pair
Range<double>::split_range_at(const double &at) const;

template<>
double
Range<double>::minimum();

template<>
double
Range<double>::maximum();

/******************************************************************************************************************************
 *                                      Specializations for Range<float>
 ******************************************************************************************************************************/

template<>
Range<float>::Range();

template<>
bool
Range<float>::empty() const;

template<>
void
Range<float>::clear();

template<>
// DQ (9/3/2015): Intel v14 compiler warns that use of "const" is meaningless.
// I think this is correct since this is being returned by value.
// const float
float
Range<float>::relaxed_first() const;

template<>
float
Range<float>::size() const;

template<>
void
Range<float>::resize(const float &new_size);

template<>
void
Range<float>::relaxed_resize(const float &new_size);

template<>
Range<float>::Pair
Range<float>::split_range_at(const float &at) const;

template<>
float
Range<float>::minimum();

template<>
float
Range<float>::maximum();

/******************************************************************************************************************************
 *                                      RangeMap void values
 ******************************************************************************************************************************/

/** Value type for a RangeMap with no useful data attached to the ranges.  This also serves as an example documenting the
 *  interface for the operations a RangeMap can perform on its values.  The single template parameter is the Range type. */
template<class R>
class RangeMapVoid {
public:
    typedef R Range;

    RangeMapVoid() {}

    template<class Other>
    explicit RangeMapVoid(const Other&) {}

    /** Remove a value from a RangeMap.  This method is invoked by RangeMap when it is removing a value from the map, such as
     *  during an erase() or clear() operation.  It is not called for the merge() argument after a successful merge. */
    void removing(const Range &my_range) {
        assert(!my_range.empty());
    }

    /** Truncate the RangeMap value.  This is similar to the removing() method, but only discards part of the value.  The @p
     *  new_end argument is the first value past the end of this range and must be such that the range would not become
     *  larger. */
    void truncate(const Range &my_range, const typename Range::Value &new_end) {
        assert(new_end>my_range.first() && new_end<=my_range.last());
    }

    /** Attempts to merge the specified range into this range.  The specified range must adjoin this range (on the left or
     *  right) but not overlap with this range.  The @p my_range argument is the range associated with this value and the
     *  @p other_range argument is the adjoining range for the value to be merged into this one.  The third argument is the
     *  value to be merged into this one.
     *
     *  Merging is optional.  If two values cannot be merged then they will be represented as distinct elements in the
     *  RangeMap.  However, merging can significantly reduce the size of large RangeMap objects.
     *
     *  If a merge occurs, then the removing() method of @p other_value is not invoked, but other_value will nonetheless be
     *  removed from the memory map.  Therefore, if its removing() method needs to do anything it should be called explicitly.
     *  It must be done this way in order to allow the merge operation the possibility doing something more efficient than
     *  copying and then deleting other_value.
     *
     *  Returns true if merging occurred, false otherwise. */
    bool merge(const Range &my_range, const Range &other_range, const RangeMapVoid &/*other_value*/) {
        assert(!my_range.empty() && !other_range.empty());
        return true;
    }

    /** Split a value into two parts.  This is the inverse of the merge() operation.  In effect, it truncates this value so it
     *  ends at @p new_end (exclusive), but rather than discarding the right part, it returns it as a new value.  The @p
     *  new_end must be inside @p my_range so that neither the modified nor returned ranges are empty.  The @p my_range
     *  argument is the value's range before it is split. */
    RangeMapVoid split(const Range &my_range, const typename Range::Value &new_end) {
        assert(my_range.contains(Range(new_end)));
        return RangeMapVoid();
    }

    void print(std::ostream &/*o*/) const {}
    friend std::ostream& operator<<(std::ostream &o, const RangeMapVoid &x) {
        x.print(o);
        return o;
    }
};

/******************************************************************************************************************************
 *                                      RangeMap numeric values
 ******************************************************************************************************************************/

/** Scalar value type for a RangeMap.  Values can be merged if they compare equal; splitting a value is done by copying it.
 *  The removing() and truncate() methods are no-ops.  See the RangeMapVoid class for full documentation. */
template<class R, class T>
class RangeMapNumeric /*final*/ {
public:
    typedef R Range;
    typedef T Value;

    /** Constructor creates object whose underlying value is zero. */
    RangeMapNumeric(): value(0) {}

    /** Constructor creates object with specified value. */
    RangeMapNumeric(Value v): value(v) {} // implicit
    
    /** Accessor for the value actually stored here.
     *  @{ */
    void set(Value v) /*final*/ {
        value = v;
    }
    virtual Value get() const /*final*/ {
        return value;
    }
    /** @} */


    /** Called when this value is being removed from a RangeMap. */
    void removing(const Range &my_range) /*final*/ {
        assert(!my_range.empty());
    }

    /** Called when removing part of a value from a RangeMap. */
    void truncate(const Range &my_range, const typename Range::Value &new_end) /*final*/ {
        assert(new_end>my_range.first() && new_end<=my_range.last());
    }

    /** Called to merge two RangeMap values.  The values can be merged only if they compare equal. */
    bool merge(const Range &my_range, const Range &other_range, RangeMapNumeric other_value) /*final*/ {
        assert(!my_range.empty() && !other_range.empty());
        return get()==other_value.get();
    }

    /** Split a RangeMap value into two parts. */
    RangeMapNumeric split(const Range &my_range, typename Range::Value new_end) /*final*/ {
        assert(my_range.contains(Range(new_end)));
        return *this;
    }

    /** Print a RangeMap value.
     *  @{ */
    void print(std::ostream &o) const /*final*/ {
        o <<value;
    }
    friend std::ostream& operator<<(std::ostream &o, const RangeMapNumeric &x) {
        x.print(o);
        return o;
    }
    /** @} */
    
private:
    Value value;
};

/******************************************************************************************************************************
 *                                      RangeMap simple values
 ******************************************************************************************************************************/

/** Scalar value type for a RangeMap.  Values can be merged if they compare equal; splitting a value is done by copying it.
 *  The removing() and truncate() methods are no-ops.  This class is often used as a base class for other more sophisticated
 *  range maps.  See the RangeMapVoid class for full documentation. */
template<class R, class T>
class RangeMapValue {
public:
    typedef R Range;
    typedef T Value;

    /** Constructor creates object whose underlying value is default constructed. */
    RangeMapValue() {}

    /** Constructor creates object with specified value. */
    RangeMapValue(const Value &v) { // implicit
        value = v;
    }

    /* This class often serves as a base class, so we have some virtual methods. */
    virtual ~RangeMapValue() {}
    

    /** Accessor for the value actually stored here.
     *  @{ */
    virtual void set(const Value &v) {
        value = v;
    }
    virtual Value get() const {
        return value;
    }
    /** @} */


    /** Called when this value is being removed from a RangeMap. */
    virtual void removing(const Range &my_range) {
        assert(!my_range.empty());
    }

    /** Called when removing part of a value from a RangeMap. */
    virtual void truncate(const Range &my_range, const typename Range::Value &new_end) {
        assert(new_end>my_range.first() && new_end<=my_range.last());
    }

    /** Called to merge two RangeMap values.  The values can be merged only if they compare equal. */
    bool merge(const Range &my_range, const Range &other_range, const RangeMapValue &other_value) {
        assert(!my_range.empty() && !other_range.empty());
        return get()==other_value.get();
    }

#if 0 /* Must be implemented in the subclass due to return type. */
    /** Split a RangeMap value into two parts. */
    RangeMapValue split(const Range &my_range, const typename Range::Value &new_end) {
        assert(my_range.contains(Range(new_end)));
        return *this;
    }
#endif

    /** Print a RangeMap value.
     *  @{ */
    virtual void print(std::ostream &o) const {
        o <<value;
    }
    friend std::ostream& operator<<(std::ostream &o, const RangeMapValue &x) {
        x.print(o);
        return o;
    }
    /** @} */
    
protected:
    Value value;
};

/******************************************************************************************************************************
 *                                      RangeMap<>
 ******************************************************************************************************************************/

/** A container of ranges, somewhat like a set.  The container is able to hold non-overlapping ranges, each of which has some
 *  associated value attached to it.  Arbitrary ranges can be inserted and erased from the RangeMap without regard to the
 *  ranges that are present in the map, since this class can merge and split values (through methods defined on the value) as
 *  necessary in order to maintain the non-overlapping invariant.  Every attempt was made to optimize this class for storage
 *  and execution efficiency and usability.  The interface is similar to the std::map interface.
 *
 *  In the simple case, when no data is attached to the ranges in the map, the RangeMap acts somewhat like an std::set with the
 *  following differences:
 *  <ul>
 *    <li>The iterator points to a pair whose @p first member is a Range (the second member is a RangeMapVoid instance with no
 *        useful data).</li>
 *    <li>The RangeMap uses much less memory than an std::set when the members are mostly contiguous.</li>
 *    <li>The find() method is faster when members are mostly contiguous.  Both are O(log N) but N is much smaller for a
 *        RangeMap than an std::set due to the compression factor.</li>
 *  </ul>
 *
 *  Here's an example of using the RangeMap as a set.  For every CPU instruction in a binary specimen, it adds the addresses of
 *  the instruction to the set.  In some architectures, such as x86, the instructions might overlap; this approach correctly
 *  handles that.
 *
 *  @code
 *  struct InstructionAddresses: public AstSimpleProcessing {
 *      typedef Range<rose_addr_t> AddressRange;
 *      RangeMap<AddressRange> set;
 *      void visit(SgNode *node) {
 *          SgAsmInstruction *insn = isSgAsmInstruction(node);
 *          if (insn!=NULL) {
 *              rose_addr_t start = insn->get_address();
 *              size_t size = insn->get_size();
 *              set.insert(AddressRange(start, size));
 *          }
 *      }
 *  } instruction_addresses;
 *  instruction_addresses.traverse(project, preorder);
 *  std::cout <<"Instructions occupy " <<instruction_addresses.set.size() <<" bytes:\n"
 *            <<instruction_addresses.set;
 *  @endcode
 *
 *  A more complex example is using a RangeMap to store a value with each range.  A simple example follows, where we want to
 *  build a RangeMap that associates any address with the function that owns that address, even when functions are
 *  discontiguous in the address space.  The first step is to define the value type for the RangeMap we'll be using to store
 *  this:
 *
 *  @code
 *  typedef Range<rose_addr_t> AddressRange;
 *
 *  class FunctionRangeMapValue: public RangeMapValue<AddressRange, SgAsmFunction*> {
 *  public:
 *      FunctionRangeMapValue(): public RangeMapValue<AddressRange, SgAsmFunction*>(NULL) {}
 *      FunctionRangeMapValue(Function *f): public RangeMapValue<AddressRange, SgAsmFunction*>(f) {}
 *
 *      FunctionRangeMapValue split(const AddressRange&, const AddressRange::Value&) {
 *          return *this;
 *      }
 *
 *      void print(std::ostream &o) const {
 *          if (NULL==value) {
 *              o <<"(null)";
 *          } else {
 *              o <<"F" <<StringUtility::addrToString(get()->entry_va);
 *          }
 *      }
 *  };
 *
 *  typedef RangeMap<AddressRange, FunctionRangeMapValue> FunctionRangeMap; 
 *  @endcode
 *
 *  Define an AST traversal add each instruction to the RangeMap:
 *
 *  @code
 *  struct FindInstructions: public AstSimpleProcessing {
 *      FunctionRangeMap ranges;
 *      void visit(SgNode *node) {
 *          SgAsmInstruction *insn = isSgAsmInstruction(node);
 *          SgAsmFunction *func = SageInterface::getEnclosingNode<SgAsmFunction>(insn);
 *          if (insn && func) {
 *              rose_addr_t start = insn->get_address();
 *              size_t size = insn->get_size();
 *              ranges.insert(AddressRange(start, size), func);
 *          }
 *      }
 *  };
 *  @endcode
 *
 *  Finally, traverse the AST and print the result.  Because RangeMap merges adjacent ranges when possible, the output will
 *  contain the fewest number of ranges needed to describe the entire address space that's assigned to functions.  Note that
 *  it's possible for two or more functions to "own" the same part of the address space if their instructions overlap, but
 *  since we defined our RangeMap to hold only one function pointer per address we'll see only the function that was added last
 *  for overlapping ranges.
 *
 *  @code
 *  FindInstructions insn_finder;
 *  insn_finder.traverse(interpretation, preorder);
 *  o <<insn_finder.ranges;
 *  @endcode
 *
 *  The RangeMap class template can also be specialized to hold more complex values.  The value type defines how ranges can be
 *  merged and split. RangeMap value types must implement the interface described for RangeMapVoid.  Another example of a value
 *  type is RangeMapValue, that holds a simple scalar value and determines "mergeabiliy" and "splitability" based on the
 *  equality operator.  Eventually, MemoryMap might also be rewritten in terms of RangeMap, and will have much more complex
 *  rules for merging, splitting, truncating, and removing.
 */
template<class R, class T=RangeMapVoid<R> >
class RangeMap {
public:
    typedef R Range;                    /** A type having the Range interface, used as keys in the underlying std::map. */
    typedef T Value;                    /** The value attached to each range in this RangeMap. */

protected:
    /* The keys of the underlying map are sorted by their last value rather than beginning value.  This allows us to use the
     * map's lower_bound() method to find the range to which an address might belong.  Since the ranges in the map are
     * non-overlapping, sorting by ending address has the same effect as sorting by starting address. */
    struct RangeCompare {
        bool operator()(const Range &a, const Range &b) const {
            return a.last() < b.last();
        }
    };

    typedef std::pair<Range, Range> RangePair;
    typedef std::pair<Range, Value> MapPair;
    typedef std::map<Range, Value, RangeCompare> Map;
    Map ranges;

public:
    typedef typename Map::iterator iterator;
    typedef typename Map::const_iterator const_iterator;
    typedef typename Map::reverse_iterator reverse_iterator;
    typedef typename Map::const_reverse_iterator const_reverse_iterator;

    /**************************************************************************************************************************
     *                                  Constructors
     **************************************************************************************************************************/
public:

    /** Create a new, empty map. */
    RangeMap() {}

    /** Create a new map from an existing map. */
    template<class Other>
    explicit RangeMap(const Other &other) {
        for (typename Other::const_iterator ri=other.begin(); ri!=other.end(); ++ri) {
            Range new_range(ri->first);
            Value new_value(ri->second);
            insert(new_range, new_value);
        }
    }

    /**************************************************************************************************************************
     *                                  Iterators and searching
     **************************************************************************************************************************/
public:

    /** First-item iterator.  Returns an iterator for the first item, or the end iterator if the RangeMap is empty.  The
     *  iterator is valid until any operation that changes the RangeMap, such as an insert or erase.
     *
     *  @{ */
    iterator begin() {
        return ranges.begin();
    }
    const_iterator begin() const {
        return ranges.begin();
    }
    /** @} */

    /** End-item iterator.  Returns an iterator to the one-past-last item of the RangeMap, regardless of whether the range map
     *  is empty.  The iterator is valid until any operation that changes the RangeMap, such as an insert or erase.
     *
     *  @{ */
    iterator end() {
        return ranges.end();
    }
    const_iterator end() const {
        return ranges.end();
    }
    /** @} */

    /** Returns a reverse iterator referring to the last item of the map, the rend() iterator if the RangeMap is empty.  The
     *  iterator is valid until any operation that changes the RangeMap, such as an insert or erase.
     *
     *  @{ */
    reverse_iterator rbegin() {
        return ranges.rbegin();
    }
    const_reverse_iterator rbegin() const {
        return ranges.rbegin();
    }
    /** @} */

    /** Returns a reverse iterator referring to the element right before the first element in the map, which is considered its
     * reverse end.  Notice that rend() does not refer to the same element as begin(), but to the element right before it.  The
     * iterator is valid until any operation that changes the RangeMap, such as an insert or erase.
     *
     *  @{ */
    reverse_iterator rend() {
        return ranges.rend();
    }
    const_reverse_iterator rend() const {
        return ranges.rend();
    }
    /** @} */

    /** Find the range containing specified value.  Returns an iterator to the Range containing the specified value, or the
     *  end() iterator if no such range exists.
     *
     *  @{ */
    iterator find(const typename Range::Value &addr) {
        iterator ei = lower_bound(addr);
        if (ei==end() || Range(addr).left_of(ei->first))
            return end();
        return ei;
    }
    const_iterator find(const typename Range::Value &addr) const {
        const_iterator ei = lower_bound(addr);
        if (ei==end() || Range(addr).left_of(ei->first))
            return end();
        return ei;
    }
    /** @} */

    /** Finds the first range ending above the specified value.  This is similar to the find() method, except it does not
     *  return the end() iterator if a range exists above the specified value.
     *
     *  @{ */
    iterator lower_bound(const typename Range::Value &addr) {
        return ranges.lower_bound(Range(addr));
    }
    const_iterator lower_bound(const typename Range::Value &addr) const {
        return ranges.lower_bound(Range(addr));
    }
    /** @} */

    /** Finds the last range starting at or below the specified value.  Returns the end iterator if there is no range
     *  containing a value less than or equal to the specified value.
     *  @{ */
    iterator find_prior(const typename Range::Value &addr) {
        if (empty())
            return end();
        iterator lb = lower_bound(addr);
        if (lb!=end() && lb->first.begins_before(Range(addr), false/*non-strict*/))
            return lb;
        if (lb==begin())
            return end();
        return --lb;
    }
    const_iterator find_prior(const typename Range::Value &addr) const {
        if (empty())
            return end();
        const_iterator lb = lower_bound(addr);
        if (lb!=end() && lb->first.begins_before(Range(addr), false/*non-strict*/))
            return lb;
        if (lb==begin())
            return end();
        return --lb;
    }
    /** @} */

    /** Find range with closest size.  Returns an iterator pointing to the first range at or after the specified @p start
     *  iterator whose size is at least as large as the specified size.  Returns the end iterator if no such range exists.
     *  Note that this is an O(N) algorithm.
     *
     * @{ */
    iterator best_fit(const typename Range::Value &size, iterator start) {
        iterator best = end();
        for (iterator ri=start; ri!=end(); ++ri) {
            if (ri->first.size()==size)
                return ri;
            if (ri->first.size()>size && (best==end() || ri->first.size()<best->first.size()))
                best = ri;
        }
        return best;
    }
    const_iterator best_fit(const typename Range::Value &size, const_iterator start) const {
        const_iterator best = end();
        for (const_iterator ri=start; ri!=end(); ++ri) {
            if (ri->first.size()==size)
                return ri;
            if (ri->first.size()>size && (best==end() || ri->first.size()<best->first.size()))
                best = ri;
        }
        return best;
    }
    /** @} */

    /** Find first range of larger size.  Returns an iterator to the first range at least as large as the specified @p size and
     *  at or after @p start.  Returns the end iterator if no range is found.  Note that this is an O(N) algorithm.
     *
     *  @{ */
    iterator first_fit(const typename Range::Value &size, iterator start) {
        for (iterator ri=start; ri!=end(); ++ri) {
            if (ri->first.size()>=size)
                return ri;
        }
        return end();
    }
    const_iterator first_fit(const typename Range::Value &size, const_iterator start) {
        for (const_iterator ri=start; ri!=end(); ++ri) {
            if (ri->first.size()>=size)
                return ri;
        }
        return end();
    }
    /** @} */

    /**************************************************************************************************************************
     *                                  Capacity
     **************************************************************************************************************************/
public:

    /** Returns true if this RangeMap is empty. */
    bool empty() const {
        return ranges.empty();
    }

    /** Returns the number of ranges in the range map.  This is the number of Range objects one would encounter if they iterate
     *  over this RangeMap from begin() to end(). */
    size_t nranges() const {
        return ranges.size();
    }

    /** Returns the number of values represented by this RangeMap.  The number of values does not typically correlate with the
     *  amount of memory used by the RangeMap since each element of the underlying std::map represents an arbitrary number of
     *  values.  Note that if the range occupies the entire possible set of values then the size might be returned as zero due
     *  to overflow, and it will be necessary to call empty() to make the determination. */
    typename Range::Value size() const {
        typename Range::Value retval = 0;
        for (const_iterator ei=begin(); ei!=end(); ++ei)
            retval += ei->first.size();
        return retval;
    }

    /** Returns the minimum value in an extent map.  The extent map must not be empty. */
    typename Range::Value min() const {
        assert(!empty());
        return ranges.begin()->first.first();
    }

    /** Returns the maximum value in an extent map.  The extent map must not be empty. */
    typename Range::Value max() const {
        assert(!empty());
        return ranges.rbegin()->first.last();
    }

    /** Returns the range of values in this map. */
    Range minmax() const {
        typename Range::Value lt=min(), rt=max();
        return Range::inin(lt, rt);
    }

    /**************************************************************************************************************************
     *                                  Low-level support functions
     **************************************************************************************************************************/
protected:

    /**************************************************************************************************************************
     *                                  Modifiers
     **************************************************************************************************************************/
public:

    /** Clears the map.  Removes all entries from the map.  If @p notify is true then also call the removing() method of each
     *  value. */
    void clear(bool notify=true) {
        if (notify) {
            for (iterator ei=begin(); ei!=end(); ++ei)
                ei->second.removing(ei->first);
        }
        ranges.clear();
    }

    /** Erases the specified range from this map.  The range to remove can span multiple existing ranges and/or parts of
     *  ranges, or no ranges at all.  It would be nice to be able to return an iterator to the next item since we have that in
     *  hand.  Unfortunately, limitations of std::map make this impractical.  If you need an iterator, just make another call
     *  to lower_bound(). */
    void erase(const Range &erase_range) {
        /* This loop figures out what needs to be removed and calls the elements' removing(), truncate(), etc. methods but does
         * not actually erase them from the underlying map yet.  We must not erase them yet because the std::map::erase()
         * method doesn't return any iterator.  Instead, we create a list (via an iterator range) of elements that will need to
         * be erased from the underlying map.
         *
         * This loop also creates a list of items that need to be inserted into the underlying map.  Even though the
         * std::map::insert() can return an iterator, we can't call it inside the loop because then our erasure iterators will
         * become invalid. */
        if (erase_range.empty())
            return;
        Map insertions;
        iterator erase_begin=end();
        iterator ei;
        for (ei=lower_bound(erase_range.first()); ei!=end() && !erase_range.left_of(ei->first); ++ei) {
            Range found_range = ei->first;
            Value &v = ei->second;
            if (erase_range.contains(found_range)) {
                /* Erase entire found range. */
                if (erase_begin==end())
                    erase_begin = ei;
                v.removing(found_range);
            } else if (erase_range.contained_in(found_range, true/*strict*/)) {
                /* Erase middle of found range. */
                assert(erase_begin==end());
                erase_begin = ei;
                RangePair rt = found_range.split_range_at(erase_range.last()+1);
                insertions[rt.second] = v.split(found_range, rt.second.first());
                RangePair lt = rt.first.split_range_at(erase_range.first());
                v.truncate(rt.first, erase_range.first());
                insertions[lt.first] = v;
            } else if (erase_range.begins_after(found_range, true/*strict*/)) {
                /* Erase right part of found range. */
                assert(erase_begin==end());
                erase_begin = ei;
                RangePair halves = found_range.split_range_at(erase_range.first());
                v.truncate(found_range, erase_range.first());
                insertions[halves.first] = v;
            } else if (erase_range.ends_before(found_range, true/*strict*/)) {
                /* Erase left part of found range. */
                if (erase_begin==end())
                    erase_begin = ei;
                RangePair halves = found_range.split_range_at(erase_range.last()+1);
                insertions[halves.second] = v.split(found_range, halves.second.first());
                v.removing(halves.first);
            }
        }

        /* Inserting is easy here because we already know that no merging is necessary. */
        if (erase_begin!=end())
            ranges.erase(erase_begin, ei);
        ranges.insert(insertions.begin(), insertions.end());
#ifdef RANGEMAP_CHECK
        check();
#endif
    }

    /** Erase ranges from this map.  Every range in the @p other map is erased from this map.  The maps need not be the same
     *  type as long as their ranges are the same type.  The values of the @p other map are not used--only its ranges. */
    template<class OtherMap>
    void erase_ranges(const OtherMap &other) {
        assert((const void*)&other!=(const void*)this);
        for (typename OtherMap::const_iterator ri=other.begin(); ri!=other.end(); ++ri)
            erase(Range::inin(ri->first.first(), ri->first.last()));
    }

    /** Insert a range/value pair into the map.  If @p make_hole is true then the new range is allowed to replace existing
     *  ranges (or parts thereof), otherwise if the new range conflicts with eixsting ranges the new extent is not inserted and
     *  no change is made to the map.  If @p merge is true then we attempt to merge the new range into adjacent ranges.
     *  Returns an iterator to the new map element, or if merged, to the element that contains the new value.  Returns the end
     *  iterator if the range was not inserted. */
    iterator insert(Range new_range, Value new_value=Value(), bool make_hole=true) {
        if (new_range.empty())
            return end();

        if (make_hole) {
            erase(new_range);
        } else {
            iterator found = lower_bound(new_range.first());
            if (found!=end() && new_range.overlaps(found->first))
                return end();
        }

        /* Attempt to merge with a left and/or right value. */
        iterator left = new_range.first()>Range::minimum() ? find(new_range.first()-1) : end();
        if (left!=end() && new_range.abuts_gt(left->first) && new_value.merge(new_range, left->first, left->second)) {
            new_range = left->first.join(new_range);
            ranges.erase(left);
        }
        iterator right = new_range.last()<new_range.maximum() ? find(new_range.last()+1) : end();
        if (right!=end() && new_range.abuts_lt(right->first) && new_value.merge(new_range, right->first, right->second)) {
            new_range = new_range.join(right->first);
            ranges.erase(right);
        }

        iterator retval = ranges.insert(end(), std::make_pair(new_range, new_value));
#ifdef RANGEMAP_CHECK
        check();
#endif
        return retval;
    }

    /** Insert one rangemap into another. */
    void insert_ranges(const RangeMap &x, bool make_hole=true) {
        assert(&x!=this);
        insert_ranges(x.begin(), x.end(), make_hole);
    }

    /** Insert part of one rangemap into another. The ranges from @p start (inclusive) to @p stop (exclusive) are inserted into
     *  this range map.  The @p start and @p stop iterators should not be iterators of this map, but some other. */
    void insert_ranges(const_iterator start, const_iterator stop, bool make_hole=true) {
        for (const_iterator ri=start; ri!=stop; ri++)
            insert(ri->first, ri->second, make_hole);
    }

    /**************************************************************************************************************************
     *                                  Predicates
     **************************************************************************************************************************/
public:

    /** Determines if two range maps overlap.  Returns true iff any ranges of this map overlap with any ranges of map @p x. */
    bool overlaps(const RangeMap &x) const {
        return first_overlap(x)!=end();
    }

    /** Determines if a range map overlaps with a specified range.  Returns true iff any part of the range @p r is present in
     *  the map.  A RangeMap never overlaps with an empty range. */
    bool overlaps(const Range &r) const {
        if (r.empty())
            return false;
        const_iterator found = lower_bound(r.first());
        return found!=end() && r.overlaps(found->first);
    }

    /** Determines if a range map does not contain any part of the specified range.  Returns false if any part of the range
     *  @p r is present in the map.  An empty range is always distinct from the map. */
    bool distinct(const Range &r) const {
        return !overlaps(r);
    }

    /** Determines if two range maps are distinct.  Returns true iff there is no range in this map that overlaps with any range
     *  of map @p x. */
    bool distinct(const RangeMap &x) const {
        return first_overlap(x)==end();
    }

    /** Determines if this range map contains all of the specified range.  If the specified range is empty then this function
     * returns true: the map contains all empty ranges. */
    bool contains(Range need) const {
        if (need.empty())
            return true;
        const_iterator found=find(need.first());
        while (1) {
            if (found==end())
                return false;
            if (need.begins_before(found->first))
                return false;
            assert(need.overlaps(found->first));
            if (need.ends_before(found->first, false/*non-strict*/))
                return true;
            need = need.split_range_at(found->first.last()+1).second;
            ++found;
        }
        assert(!"should not be reached");
        return true;
    }

    /** Determins if this range map contains all of some other range map.  Returns true iff each range in @p x is contained in
     *  some range of this map. If @p x is empty this function returns true: a RangeMap contains all empty ranges. */
    bool contains(const RangeMap &x) const {
        if (x.empty())
            return true;
        for (const_iterator xi=x.begin(); xi!=x.end(); ++xi) {
            if (!contains(xi->first))
                return false;
        }
        return true;
    }

    /**************************************************************************************************************************
     *                                  Comparisons
     **************************************************************************************************************************/
public:

    /** Find the first overlap between two RangeMap objects.  Returns an iterator for this map that points to the first range
     *  that overlaps with some range in the other map, @p x.  Returns the end iterator if no overlap is found.
     *
     *  @{ */
    iterator find_overlap(const RangeMap &x) {
        return find_overlap(begin(), end(), x);
    }
    const_iterator first_overlap(const RangeMap &x) const {
        return find_overlap(begin(), end(), x);
    }
    /** @} */

    /** Find an overlap between two RangeMap objects.  Returns an iterator for this map that points to the first range that
     *  overlaps with some range in the other map, @p x.  The returned iterator will be between @p start (inclusive) and @p
     *  stop (exclusive), which must obviously be iterators for this RangeMap, not @p x.  Returns the end iterator if there is
     *  no overlap within the restricted ranges.
     *
     *  @{ */
    iterator find_overlap(iterator start, iterator stop, const RangeMap &x) {
        if (start==stop)
            return end();

        iterator ia = start;
        const_iterator ib = x.lower_bound(start->first.first());
        while (ia!=stop && ib!=x.end() && ia->first.distinct(ib->first)) {
            while (ia!=stop && ia->first.left_of(ib->first))
                ++ia;
            while (ib!=x.end() && ib->first.left_of(ia->first))
                ++ib;
        }

        return ia!=stop && ib!=x.end() && ia->first.overlaps(ib->first);
    }
    const_iterator find_overlap(const_iterator start, const_iterator stop, const RangeMap &x) const {
        if (start==stop)
            return end();

        const_iterator ia = start;
        const_iterator ib = x.lower_bound(start->first.first());
        while (ia!=stop && ib!=x.end() && ia->first.distinct(ib->first)) {
            while (ia!=stop && ia->first.left_of(ib->first))
                ++ia;
            while (ib!=x.end() && ib->first.left_of(ia->first))
                ++ib;
        }

        return ia!=stop && ib!=x.end() && ia->first.overlaps(ib->first) ? ia : end();
    }
    /** @} */
            
    /**************************************************************************************************************************
     *                                  Operators
     **************************************************************************************************************************/
public:

    /** Create an inverse of a range map. The values of the result are default constructed. */
    template<class ResultMap>
    ResultMap invert() const {
        return invert_within<ResultMap>(Range::all());
    }

    /** Create a range map that's the inverse of some other map.  The returned map's ranges will be limited according to the
     *  specified @p limits. The values of the result are default constructed. */
    template<class ResultMap>
    ResultMap invert_within(const Range &limits) const {
        ResultMap retval;
        if (limits.empty())
            return retval;
        typename Range::Value pending = limits.first();
        for (const_iterator ri=lower_bound(limits.first()); ri!=end() && !limits.left_of(ri->first); ++ri) {
            if (pending < ri->first.first())
                retval.insert(Range::inin(pending, ri->first.first()-1));
            pending = ri->first.last()+1;
        }
        if (pending <= limits.last())
            retval.insert(Range::inin(pending, limits.last()));
        if (!retval.empty())
            assert(retval.minmax().contained_in(limits, false));
        return retval;
    }

    /** Select ranges overlapping selector range.  Returns a new range map whose ranges are those ranges of this map that
     *  overlap with the specified @p selector range. */
    RangeMap select_overlapping_ranges(const Range &selector) const {
        RangeMap retval;
        if (!selector.empty()) {
            for (const_iterator ri=lower_bound(selector.start()); ri!=end() && !selector.left_of(ri->first); ++ri) {
                if (selector.overlaps(ri->first))
                    retval.insert(ri->first, ri->second);
            }
        }
        return retval;
    }

    /**************************************************************************************************************************
     *                                  Debugging
     **************************************************************************************************************************/
public:

    void check() const {
// DQ (11/8/2011): Commented out as part of ROSE compiling this ROSE source code (EDG frontend complains about errors).
#ifndef USE_ROSE
#ifndef NDEBUG
#define RANGEMAP_CHECK(EXPR) if (!(EXPR)) {                                                                                    \
            std::cerr <<"RangeMap::check() failed at r1=" <<r1 <<" r2=" <<r2 <<": " #EXPR "\n";                                \
            std::cerr <<"Entire range map at point of failure:\n";                                                             \
            print(std::cerr, "    ");                                                                                          \
            assert(EXPR);                                                                                                      \
        }
            
        for (const_iterator i1=begin(); i1!=end(); ++i1) {
            const Range &r1 = i1->first;
            const_iterator i2 = i1; ++i2;
            if (i2!=end()) {
                const Range &r2 = i2->first;

                RANGEMAP_CHECK(!r2.empty());

                RANGEMAP_CHECK(!r1.begins_with(r2));
                RANGEMAP_CHECK(!r2.begins_with(r1));

                RANGEMAP_CHECK(!r1.ends_with(r2));
                RANGEMAP_CHECK(!r2.ends_with(r1));

                RANGEMAP_CHECK(!r1.begins_after(r2, false));
                RANGEMAP_CHECK(!r1.begins_after(r2, true));
                RANGEMAP_CHECK(r2.begins_after(r1, false));
                RANGEMAP_CHECK(r2.begins_after(r1, true));

                RANGEMAP_CHECK(r1.begins_before(r2, false));
                RANGEMAP_CHECK(r1.begins_before(r2, true));
                RANGEMAP_CHECK(!r2.begins_before(r1, false));
                RANGEMAP_CHECK(!r2.begins_before(r1, true));

                RANGEMAP_CHECK(!r1.ends_after(r2, false));
                RANGEMAP_CHECK(!r1.ends_after(r2, true));
                RANGEMAP_CHECK(r2.ends_after(r1, false));
                RANGEMAP_CHECK(r2.ends_after(r1, true));

                RANGEMAP_CHECK(r1.ends_before(r2, false));
                RANGEMAP_CHECK(r1.ends_before(r2, true));
                RANGEMAP_CHECK(!r2.ends_before(r1, false));
                RANGEMAP_CHECK(!r2.ends_before(r1, true));

                RANGEMAP_CHECK(!r1.contains(r2, false));
                RANGEMAP_CHECK(!r1.contains(r2, true));
                RANGEMAP_CHECK(!r2.contains(r1, false));
                RANGEMAP_CHECK(!r2.contains(r1, true));

                RANGEMAP_CHECK(!r1.contained_in(r2, false));
                RANGEMAP_CHECK(!r1.contained_in(r2, true));
                RANGEMAP_CHECK(!r2.contained_in(r1, false));
                RANGEMAP_CHECK(!r2.contained_in(r1, true));

                RANGEMAP_CHECK(!r1.congruent(r2));
                RANGEMAP_CHECK(!r2.congruent(r1));

                RANGEMAP_CHECK(r1.left_of(r2));
                RANGEMAP_CHECK(!r2.left_of(r1));

                RANGEMAP_CHECK(!r1.right_of(r2));
                RANGEMAP_CHECK(r2.right_of(r1));

                RANGEMAP_CHECK(!r1.overlaps(r2));
                RANGEMAP_CHECK(!r2.overlaps(r1));

                RANGEMAP_CHECK(r1.distinct(r2));
                RANGEMAP_CHECK(r2.distinct(r1));

                RANGEMAP_CHECK(!r1.abuts_gt(r2)); // r1.abuts_lt(r2) is possible
                RANGEMAP_CHECK(!r2.abuts_lt(r1)); // r2.abuts_gt(r1) is possible
            }
        }
#undef RANGEMAP_CHECK
#endif
#endif
    }

    /** Prints unformatted RangeMap on a single line */
    void print(std::ostream &o) const {
        for (const_iterator ri=begin(); ri!=end(); ++ri) {
            std::ostringstream s;
            s <<ri->second;
            o <<(ri==begin()?"":", ") <<ri->first;
            if (!s.str().empty())
                o <<" {" <<s.str() <<"}";
        }
    }

    friend std::ostream& operator<<(std::ostream &o, const RangeMap &rmap) {
        rmap.print(o);
        return o;
    }

};

#endif
