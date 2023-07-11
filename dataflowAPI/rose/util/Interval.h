// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




#ifndef Sawyer_Interval_H
#define Sawyer_Interval_H

#include "Assert.h"
#include "Sawyer.h"
#include <utility>
#include <boost/integer_traits.hpp>

namespace Sawyer {
namespace Container {

/** Range of values delimited by endpoints.
 *
 *  This class represents a range of contiguous values by specifying the lower and upper end points, both of which are included
 *  in the range. Alternatively, a range may be empty; the default constructor creates empty ranges.  The value type, @p T, is
 *  intended to be an unsigned integer type.  Signed integers may be used, but the caller should be prepared to handle negative
 *  sizes due to overflow (see @ref size).  Non-integer types are not recommended since some methods (e.g., @ref size) assume
 *  that <em>n</em> and <em>n+1</em> are adjacent values, which is not the case for floating point.
 *
 *  Values of this type are immutable except for the assignment operator; operations like intersection return a new object
 *  rather than modifying an existing object. */
template<class T>
class Interval {
public:
    typedef T Value;
private:
    T lo_, hi_;
public:
    /** Constructs an empty interval. */
    Interval(): lo_(1), hi_(0) {}

    /** Copy-constructs an interval. */
    Interval(const Interval &other): lo_(other.lo_), hi_(other.hi_) {}

    /** Constructs a singleton interval. */
    Interval(T value): lo_(value), hi_(value) {}

#if 0 // [Robb Matzke 2014-05-14]: too much confusion with "hi" vs. "size". Use either baseSize or hull instead.
    /** Constructs an interval from endpoints.
     *
     *  The first end point must be less than or equal to the second end point.  If both endpoints are equal then a singleton
     *  interval is constructed. */
    Interval(T lo, T hi): lo_(lo), hi_(hi) {
        ASSERT_require(lo <= hi);
    }
#endif

    /** Construct an interval from two endpoints.
     *
     *  Returns the smallest interal that contains both points. */
    static Interval hull(T v1, T v2) {
        Interval retval;
        retval.lo_ = std::min(v1, v2);
        retval.hi_ = std::max(v1, v2);
        return retval;
    }

    /** Construct an interval from one endpoint and a size.
     *
     *  Returns the smallest interval that contains @p lo (inclusive) through @p lo + @p size (exclusive).  If @p size is zero
     *  then an empty interval is created, in which case @p lo is irrelevant. */
    static Interval baseSize(T lo, T size) {
        ASSERT_require2(lo + size >= lo, "overflow");
        return 0==size ? Interval() : Interval::hull(lo, lo+size-1);
    }

    /** Construct an interval that covers the entire domain. */
    static Interval whole() {
        return hull(boost::integer_traits<T>::const_min, boost::integer_traits<T>::const_max);
    }

    /** Assignment from an interval. */
    Interval& operator=(const Interval &other) {
        lo_ = other.lo_;
        hi_ = other.hi_;
        return *this;
    }

    /** Assignment from a scalar. */
    Interval& operator=(T value) {
        lo_ = hi_ = value;
        return *this;
    }

    /** Returns lower limit. */
    T least() const {
        ASSERT_forbid(isEmpty());
        return lo_;
    }

    /** Returns upper limit. */
    T greatest() const {
        ASSERT_forbid(isEmpty());
        return hi_;
    }

    /** True if interval is empty. */
    bool isEmpty() const { return 1==lo_ && 0==hi_; }

    /** True if interval is a singleton. */
    bool isSingleton() const { return lo_ == hi_; }

    /** True if interval covers entire space. */
    bool isWhole() const { return lo_==boost::integer_traits<T>::const_min && hi_==boost::integer_traits<T>::const_max; }

    /** True if two intervals overlap.
     *
     *  An empty interval never overlaps with any other interval, empty or not. */
    bool isOverlapping(const Interval &other) const {
        return !intersection(other).isEmpty();
    }

    /** Containment predicate.
     *
     *  Returns true if this interval contains all of the @p other interval.  An empty interval is always contained in any
     *  other interval, even another empty interval. */
    bool isContaining(const Interval &other) const {
        return (other.isEmpty() ||
                (!isEmpty() && least()<=other.least() && greatest()>=other.greatest()));
    }

    /** Adjacency predicate.
     *
     *  Returns true if the two intervals are adjacent.  An empty interval is adjacent to all other intervals, including
     *  another empty interval.
     *
     *  @{ */
    bool isLeftAdjacent(const Interval &right) const {
        return isEmpty() || right.isEmpty() || (!isWhole() && greatest()+1 == right.least());
    }
    bool isRightAdjacent(const Interval &left) const {
        return isEmpty() || left.isEmpty() || (!left.isWhole() && left.greatest()+1 == least());
    }
    bool isAdjacent(const Interval &other) const {
        return (isEmpty() || other.isEmpty() ||
                (!isWhole() && greatest()+1 == other.least()) ||
                (!other.isWhole() && other.greatest()+1 == least()));
    }
    /** @} */

    /** Relative position predicate.
     *
     *  Returns true if the intervals do not overlap and one is positioned left or right of the other.  Empty intervals are
     *  considered to be both left and right of the other.
     *
     *  @{ */
    bool isLeftOf(const Interval &right) const {
        return isEmpty() || right.isEmpty() || greatest() < right.least();
    }
    bool isRightOf(const Interval &left) const {
        return isEmpty() || left.isEmpty() || left.greatest() < least();
    }
    /** @} */

    /** Size of interval.
     *
     *  If the interval is the whole space then the return value is zero due to overflow. */
    Value size() const { return isEmpty() ? 0 : hi_ - lo_ + 1; }

    /** Equality test.
     *
     *  Two intervals are equal if they have the same lower and upper bound, and unequal if either bound differs. Two empty
     *  ranges are considered to be equal.
     *
     *  @{ */
    bool operator==(const Interval &other) const {
        return lo_==other.lo_ && hi_==other.hi_;
    }
    bool operator!=(const Interval &other) const {
        return lo_!=other.lo_ || hi_!=other.hi_;
    }
    /** @} */

    /** Intersection.
     *
     *  Returns an interval which is the intersection of this interval with another.
     *
     * @{ */
    Interval intersection(const Interval &other) const {
        if (isEmpty() || other.isEmpty() || greatest()<other.least() || least()>other.greatest())
            return Interval();
        return Interval::hull(std::max(least(), other.least()), std::min(greatest(), other.greatest()));
    }
    Interval operator&(const Interval &other) const {
        return intersection(other);
    }
    /** @} */

    /** Hull.
     *
     *  Returns the smallest interval that contains both this interval and the @p other interval.
     *
     *  @sa join */
    Interval hull(const Interval &other) const {
        if (isEmpty()) {
            return other;
        } else if (other.isEmpty()) {
            return *this;
        } else {
            return Interval::hull(std::min(least(), other.least()), std::max(greatest(), other.greatest()));
        }
    }

    /** Hull.
     *
     *  Returns the smallest interval that contains both this interval and another value. */
    Interval hull(T value) const {
        if (isEmpty()) {
            return Interval(value);
        } else {
            return Interval::hull(std::min(least(), value), std::max(greatest(), value));
        }
    }

    /** Split interval in two.
     *
     *  Returns two interval by splitting this interval so that @p splitPoint is in the left returned interval.  If the split
     *  is not a member of this interval then one of the two returned intervals will be empty, depending on whether the split
     *  point is less than or greater than this interval.  If this interval is empty then both returned intervals will be empty
     *  regardless of the split point. */
    std::pair<Interval, Interval> split(T splitPoint) const {
        if (isEmpty()) {
            return std::make_pair(Interval(), Interval());
        } else if (splitPoint < least()) {
            return std::make_pair(Interval(), *this);
        } else if (splitPoint <= greatest()) {
            return std::make_pair(Interval(least(), splitPoint), Interval(splitPoint+1, greatest()));
        } else {
            return std::make_pair(*this, Interval());
        }
    }

    /** Creates an interval by joining two adjacent intervals.
     *
     *  Concatenates this interval with the @p right interval and returns the result.  This is similar to @ref hull except
     *  when neither interval is empty then the greatest value of this interval must be one less than the least value of the @p
     *  right interval.
     *
     *  @sa hull */
    Interval join(const Interval &right) const {
        if (isEmpty()) {
            return right;
        } else if (right.isEmpty()) {
            return *this;
        } else {
            ASSERT_require(greatest()+1 == right.least() && right.least() > greatest());
            return hull(right);
        }
    }

    // The following trickery is to allow things like "if (x)" to work but without having an implicit
    // conversion to bool which would cause no end of other problems.  This is fixed in C++11
private:
    typedef void(Interval::*unspecified_bool)() const;
    void this_type_does_not_support_comparisons() const {}
public:
    /** Type for Boolean context.
     *
     *  Implicit conversion to a type that can be used in a boolean context such as an <code>if</code> or <code>while</code>
     *  statement.  For instance:
     *
     * @code
     *  if (Interval<unsigned> x = doSomething(...)) {
     *     // this is reached only if x is non-empty
     *  }
     * @endcode
     *
     *  The inteval evaluates to true if it is non-empty, and false if it is empty. */
    operator unspecified_bool() const {
        return isEmpty() ? 0 : &Interval::this_type_does_not_support_comparisons;
    }
};

} // namespace
} // namespace

#endif
