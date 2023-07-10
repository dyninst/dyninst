// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




#ifndef Sawyer_IntervalSet_H
#define Sawyer_IntervalSet_H

#include "IntervalMap.h"
#include "Optional.h"
#include "Sawyer.h"

#include <stddef.h>
#include <utility>
#include <boost/integer_traits.hpp>
#include <boost/iterator/iterator_facade.hpp>

namespace Sawyer {
namespace Container {

/** A container holding a set of values.
 *
 *  This container is somewhat like the STL <code>std::set</code> container except it is optimized for the case when large
 *  numbers of values are contiguous.  It adds the ability to insert and erase intervals as well as scalars, and provides a
 *  mechanism to iterate over the storage nodes (intervals) rather than over the scalar values.
 *
 *  An interval set always maintains a list containing a minimum number of largest possible intervals regardless of what
 *  intervals are inserted and erased.  This feature makes for a very convenient way to compute address ranges for small
 *  things, or things that might overlap.  For instance, if one has a list of intervals corresponding to machine instructions
 *  that belong to a single function in an executable (some of which might even overlap), we can easily compute whether the
 *  function is contiguous in memory, and whether any instructions overlap with each other:
 *
 * @code
 *  // Input is a list of intervals for instructions in a function
 *  typedef Sawyer::Container::Interval<uint32_t> AddressInterval;
 *  std::vector<AddressInterval> instructionIntervals = ...;
 *
 *  // Build the functionExtent and count total instruction size
 *  Sawyer::Container::IntervalSet<AddressInterval> functionExtent;
 *  uint32_t insnTotalSize = 0;
 *  BOOST_FOREACH (const AddressInterval &insnInterval, instructionIntervals) {
 *      functionExtent.insert(insnInterval);
 *      insnTotalSize += insnInterval.size();
 *  }
 *
 *  // Final results
 *  bool isFunctionContiguous = functionExtent.nIntervals() <= 1;
 *  bool isInsnsDisjoint = functionExtent.size() == insnTotalSize;
 * @endcode
 *
 *  The @p Interval template parameter must implement the Sawyer::Container::Interval API, at least to some extent. */
template<class I>
class IntervalSet {
    // We use an IntervalMap to do all our work, always storing int(0) as the value.
    typedef IntervalMap<I, int> Map;
    Map map_;
public:
    typedef I Interval;
    typedef typename I::Value Scalar;                   /**< Type of scalar values stored in this set. */

    /** Interval iterator.
     *
     *  Iterates over the intervals of the container, which are the Interval type provided as a class template
     *  parameter. Dereferencing the iterator will return a const reference to an interval (possibly a singlton interval). */
    class ConstIntervalIterator: public boost::iterator_facade<ConstIntervalIterator, const Interval,
                                                               boost::bidirectional_traversal_tag> {
        typedef typename IntervalMap<Interval, int>::ConstNodeIterator MapNodeIterator;
        MapNodeIterator iter_;
    public:
        ConstIntervalIterator() {}
    private:
        friend class boost::iterator_core_access;
        friend class IntervalSet;
        explicit ConstIntervalIterator(MapNodeIterator iter): iter_(iter) {}
        const Interval& dereference() const { return iter_->key(); }
        bool equal(const ConstIntervalIterator &other) const { return iter_ == other.iter_; }
        void increment() { ++iter_; }
        void decrement() { --iter_; }
        MapNodeIterator base() const { return iter_; }
    };

    /** Scalar value iterator.
     *
     *  Scalar value iterators iterate over each scalar value in the set. Two caveats to beware of:
     *
     *  @li The set can hold a very large number of values, even the entire value space, in which case iterating over values
     *      rather than storage nodes could take a very long time.
     *  @li Iterating over values for a non-integral type is most likely nonsensical. */
    class ConstScalarIterator: public boost::iterator_facade<ConstScalarIterator, const typename Interval::Value,
                                                             boost::bidirectional_traversal_tag> {
        ConstIntervalIterator iter_;
        typename Interval::Value offset_;
        mutable typename Interval::Value value_;        // so dereference() can return a reference
    public:
        ConstScalarIterator(): offset_(0) {}
        ConstScalarIterator(ConstIntervalIterator iter): iter_(iter), offset_(0) {}
    private:
        friend class boost::iterator_core_access;
        friend class IntervalSet;
        const typename Interval::Value& dereference() const {
            ASSERT_require2(iter_->least() <= iter_->greatest(), "stored interval cannot be empty");
            ASSERT_require(iter_->least() + offset_ <= iter_->greatest());
            value_ = iter_->least() + offset_;
            return value_;                              // must return a reference
        }
        bool equal(const ConstScalarIterator &other) const {
            return iter_ == other.iter_ && offset_ == other.offset_;
        }
        void increment() {
            ASSERT_require2(iter_->least() <= iter_->greatest(), "stored interval cannot be empty");
            if (iter_->least() + offset_ == iter_->greatest()) {
                ++iter_;
                offset_ = 0;
            } else {
                ++offset_;
            }
        }
        void decrement() {
            ASSERT_require2(iter_->least() <= iter_->greatest(), "stored interval cannot be empty");
            if (0==offset_) {
                --iter_;
                offset_ = width(*iter_);
            } else {
                --offset_;
            }
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Constructors
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /** Default constructor.
     *
     *  Creates an empty set. */
    IntervalSet() {}

    /** Copy constructor.
     *
     *  The newly constructed set will contain copies of the nodes from @p other. */
    template<class Interval2>
    IntervalSet(const IntervalSet<Interval2> &other) {
        typedef typename IntervalSet<Interval2>::ConstIntervalIterator OtherIntervalIterator;
        for (OtherIntervalIterator otherIter=other.intervals().begin(); otherIter!=other.intervals().end(); ++otherIter)
            insert(*otherIter);
    }

    /** Construct from an IntervalMap.
     *
     *  The newly constructed set will contain copies of the intervals from the specified @ref IntervalMap.  The map's
     *  intervals must be convertible to the set's interval type. The map's values are not used. */
    template<class Interval2, class T, class Policy>
    explicit IntervalSet(const IntervalMap<Interval2, T, Policy> &other) {
        typedef typename IntervalMap<Interval2, T, Policy>::ConstNodeIterator OtherNodeIterator;
        for (OtherNodeIterator otherIter=other.nodes().begin(); otherIter!=other.nodes().end(); ++otherIter)
            insert(otherIter->key());
    }

    /** Construct from an iterator range.
     *
     *  The newly constructed set will contain copies of the intervals from the specified iterator range.  The range's
     *  dereferenced iterators must be convertible to the set's interval type. */
    template<class Iterator>
    IntervalSet(const boost::iterator_range<Iterator> &intervals) {
        for (Iterator iter=intervals.begin(); iter!=intervals.end(); ++iter)
            insert(*iter);
    }
    
    /** Assignment from another set.
     *
     *  Causes this set to contain the same intervals as the @p other set. The other set's intervals must be convertible to
     *  this set's interval type. */
    template<class Interval2>
    IntervalSet& operator=(const IntervalSet<Interval2> &other) {
        clear();
        typedef typename IntervalSet<Interval2>::ConstIntervalIterator OtherIntervalIterator;
        for (OtherIntervalIterator otherIter=other.intervals().begin(); otherIter!=other.intervals().end(); ++otherIter)
            insert(*otherIter);
        return *this;
    }

    /** Assignment from an IntervalMap.
     *
     *  Causes this set to contain the same intervals as the specified map.  The map's intervals must be convertible to this
     *  set's interval type.  Since sets and maps have different requirements regarding merging of neighboring intervals, the
     *  returned container might not have node-to-node correspondence with the map, but both will contain the same logical
     *  intervals. */
    template<class Interval2, class T, class Policy>
    IntervalSet& operator=(const IntervalMap<Interval2, T, Policy> &other) {
        clear();
        typedef typename IntervalMap<Interval2, T, Policy>::ConstNodeIterator OtherNodeIterator;
        for (OtherNodeIterator otherIter=other.nodes().begin(); otherIter!=other.nodes().end(); ++otherIter)
            insert(otherIter->key());
        return *this;
    }

    /** Assignment from an iterator range.
     *
     *  The newly constructed set will contain copies of the intervals from the specified iterator range.  The range's
     *  dereferenced iterators must be convertible to the set's interval type. */
    template<class Iterator>
    IntervalSet& operator=(const boost::iterator_range<Iterator> &intervals) {
        clear();
        for (Iterator iter=intervals.begin(); iter!=intervals.end(); ++iter)
            insert(*iter);
        return *this;
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Iteration
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /** Iterator range for all intervals actually stored by this set. */
    boost::iterator_range<ConstIntervalIterator> intervals() const {
        return boost::iterator_range<ConstIntervalIterator>(ConstIntervalIterator(map_.nodes().begin()),
                                                            ConstIntervalIterator(map_.nodes().end()));
    }

    /** Iterator range for all scalar values logically represented by this set. */
    boost::iterator_range<ConstScalarIterator> scalars() const {
        return boost::iterator_range<ConstScalarIterator>(ConstScalarIterator(intervals().begin()),
                                                          ConstScalarIterator(intervals().end()));
    }

    /** Find the first node whose interval ends at or above the specified scalar key.
     *
     *  Returns an iterator to the node, or the end iterator if no such node exists. */
    ConstIntervalIterator lowerBound(const typename Interval::Value &scalar) const {
        return ConstIntervalIterator(map_.lowerBound(scalar));
    }

    /** Find the first node whose interval begins above the specified scalar key.
     *
     *  Returns an iterator to the node, or the end iterator if no such node exists. */
    ConstIntervalIterator upperBound(const typename Interval::Value &scalar) const {
        return ConstIntervalIterator(map_.upperBound(scalar));
    }

    /** Find the last node whose interval starts at or below the specified scalar key.
     *
     *  Returns an iterator to the node, or the end iterator if no such node exists. */
    ConstIntervalIterator findPrior(const typename Interval::Value &scalar) const {
        return ConstIntervalIterator(map_.findPrior(scalar));
    }

    /** Find the node containing the specified scalar key.
     *
     *  Returns an iterator to the matching node, or the end iterator if no such node exists. */
    ConstIntervalIterator find(const typename Interval::Value &scalar) const {
        return ConstIntervalIterator(map_.find(scalar));
    }

    /** Finds all nodes overlapping the specified interval.
     *
     *  Returns an iterator range that enumerates the nodes that overlap with the specified interval. */
    boost::iterator_range<ConstIntervalIterator> findAll(const Interval &interval) const {
        boost::iterator_range<typename Map::ConstNodeIterator> range = map_.findAll(interval);
        return boost::iterator_range<ConstIntervalIterator>(ConstIntervalIterator(range.begin()),
                                                            ConstIntervalIterator(range.end()));
    }

    /** Finds first node that overlaps with the specified interval.
     *
     *  Returns an iterator to the matching node, or the end iterator if no such node exists. */
    ConstIntervalIterator findFirstOverlap(const Interval &interval) const {
        return ConstIntervalIterator(map_.findFirstOverlap(interval));
    }
    /** @} */

    /** Find first nodes that overlap.
     *
     *  Given two ranges of iterators for two sets, advance the iterators to the closest nodes that overlap with each other,
     *  and return the result as two iterators.  If no overlaps can be found then the return value is two end iterators. */
    std::pair<ConstIntervalIterator, ConstIntervalIterator>
    findFirstOverlap(ConstIntervalIterator thisIter, const IntervalSet &other, ConstIntervalIterator otherIter) const {
        std::pair<typename Map::ConstNodeIterator, typename Map::ConstNodeIterator> found =
            map_.findFirstOverlap(thisIter.base(), other.map_, otherIter.base());
        return std::make_pair(ConstIntervalIterator(found.first), ConstIntervalIterator(found.second));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Size
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /** Determine whether the container is empty.
     *
     *  Returns true only if this set contains no elements. */
    bool isEmpty() const {
        return map_.isEmpty();
    }

    /** Number of scalar elements represented.
     *
     *  Returns the number of scalar elements (not intervals or storage nodes) contained in this set.  Since the return type is
     *  the same as the type used in the interval end points, this function can return overflowed values.  For instance, a set
     *  that contains all possible values in the value space is likely to return zero. */
    typename Interval::Value size() const {
        return map_.size();
    }

    /** Number of storage nodes.
     *
     *  Returns the number of nodes stored in this container, which for sets is always the number of maximally contiguous
     *  intervals.  Most algorithms employed by IntervalSet methods are either logarithmic or scalar in this number. */
    size_t nIntervals() const {
        return map_.nIntervals();
    }

    /** Returns the range of values in this map. */
    Interval hull() const {
        return map_.hull();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Predicates
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /** Determines whether this set overlaps with the argument.
     *
     *  Returns true if this set contains any values that are also present in the argument.
     *
     * @{ */
    template<class Interval2>
    bool isOverlapping(const Interval2 &interval) const {
        return map_.isOverlapping(interval);
    }

    template<class Interval2>
    bool isOverlapping(const IntervalSet<Interval2> &other) const {
        return map_.isOverlapping(other.map_);
    }

    template<class Interval2, class T2, class Policy2>
    bool isOverlapping(const IntervalMap<Interval2, T2, Policy2> &other) const {
        return map_.isOverlapping(other);
    }
    /** @} */

    /** Determines whether this set is distinct from the argument.
     *
     *  Returns true if none of the values of this set are equal to any value in the argument.
     *
     * @{ */
    template<class Interval2>
    bool isDistinct(const Interval2 &/*interval*/) const {
        return !isOverlapping();
    }

    template<class Interval2>
    bool isDistinct(const IntervalSet<Interval2> &other) const {
        return !isOverlapping(other);
    }

    template<class Interval2, class T2, class Policy2>
    bool isDistinct(const IntervalMap<Interval2, T2, Policy2> &other) const {
        return !isOverlapping(other);
    }
    /** @} */

    /** Determines if a value exists in the set.
     *
     *  Returns true if the specified value is a member of the set. */
    bool exists(const typename Interval::Value &scalar) const {
        return find(scalar)!=intervals().end();
    }
    
    /** Determines whether this set fully contains the argument.
     *
     *  Returns true if this set contains all values represented by the argument.
     *
     * @{ */
    bool contains(const typename Interval::Value &scalar) const {
        return exists(scalar);
    }

    template<class Interval2>
    bool contains(const Interval2 &interval) const {
        return map_.contains(interval);
    }

    template<class Interval2>
    bool contains(const IntervalSet<Interval2> &other) const {
        return map_.contains(other.map_);
    }

    template<class Interval2, class T2, class Policy2>
    bool contains(const IntervalMap<Interval2, T2, Policy2> &other) const {
        return map_.contains(other);
    }
    /** @} */

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Searching
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    /** Returns the minimum scalar contained in this set. */
    Scalar least() const {
        ASSERT_forbid(isEmpty());
        return map_.least();
    }

    /** Returns the maximum scalar contained in this set. */
    Scalar greatest() const {
        ASSERT_forbid(isEmpty());
        return map_.greatest();
    }

    /** Returns the limited-minimum scalar contained in this set.
     *
     *  Returns the minimum scalar that exists in this set and which is greater than or equal to @p lowerLimit.  If no such
     *  value exists then nothing is returned. */
    Optional<Scalar> least(Scalar lowerLimit) const {
        return map_.least(lowerLimit);
    }

    /** Returns the limited-maximum scalar contained in this set.
     *
     *  Returns the maximum scalar that exists in this set and which is less than or equal to @p upperLimit.  If no such
     *  value exists then nothing is returned. */
    Optional<Scalar> greatest(Scalar upperLimit) const {
        return map_.greatest(upperLimit);
    }

    /** Returns the limited-minimum scalar not contained in this set.
     *
     *  Returns the minimum scalar equal to or greater than the @p lowerLimit which is not in this set.  If no such value
     *  exists then nothing is returned. */
    Optional<Scalar> leastNonExistent(Scalar lowerLimit) const {
        return map_.leastUnmapped(lowerLimit);
    }

    /** Returns the limited-maximum scalar not contained in this set.
     *
     *  Returns the maximum scalar equal to or less than the @p upperLimit which is not in this set.  If no such value exists
     *  then nothing is returned. */
    Optional<Scalar> greatestNonExistent(Scalar upperLimit) const {
        return map_.greatestUnmapped(upperLimit);
    }

    /** Find the first fit node at or after a starting point.
     *
     *  Finds the first node of contiguous values beginning at or after the specified starting iterator, @p start, and which is
     *  at least as large as the desired @p size.  If there are no such nodes then the end iterator is returned.
     *
     *  Caveat emptor: The @p size argument has the name type as the interval end points. If the end points have a signed type,
     *  then it is entirely likely that the size will overflow.  In fact, it is also possible that unsigned sizes overflow
     *  since, for example, an 8-bit unsigned size cannot hold the size of an interval representing the entire 8-bit space.
     *  Therefore, use this method with care. */
    ConstIntervalIterator firstFit(const typename Interval::Value &size, ConstIntervalIterator start) const {
        return ConstIntervalIterator(map_.firstFit(size, start.iter_));
    }

    /** Find the best fit node at or after a starting point.
     *
     *  Finds a node of contiguous values beginning at or after the specified starting iterator, @p start, and which is at
     *  least as large as the desired @p size.  If there is more than one such node, then the first smallest such node is
     *  returned.  If there are no such nodes then the end iterator is returned.
     *
     *  Caveat emptor: The @p size argument has the name type as the interval end points. If the end points have a signed type,
     *  then it is entirely likely that the size will overflow.  In fact, it is also possible that unsigned sizes overflow
     *  since, for example, an 8-bit unsigned size cannot hold the size of an interval representing the entire 8-bit space.
     *  Therefore, use this method with care. */
    ConstIntervalIterator bestFit(const typename Interval::Value &size, ConstIntervalIterator start) const {
        return ConstIntervalIterator(map_.bestFit(size, start.iter_));
    }



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Modifiers
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /** Remove all values.
     *
     *  All values are removed from this set and the set becomes empty. */
    void clear() {
        map_.clear();
    }

    /** Invert.
     *
     *  Invert this set in place. */
    void invert() {
        invert(Interval::whole());
    }

    /** Invert and intersect.
     *
     *  Inverts this set and then intersects it with @p restricted. */
    void invert(const Interval &restricted) {
        IntervalSet inverted;
        if (!restricted.isEmpty()) {
            typename Interval::Value pending = restricted.least();
            bool insertTop = true;
            for (typename Map::ConstIntervalIterator iter=map_.lowerBound(restricted.least());
                 iter!=map_.intervals().end(); ++iter) {
                if (iter->least() > restricted.greatest())
                    break;
                if (pending < iter->least())
                    inverted.insert(Interval::hull(pending, iter->least()-1));
                if (iter->greatest() < restricted.greatest()) {
                    pending = iter->greatest() + 1;
                } else {
                    insertTop = false;
                    break;
                }
            }
            if (insertTop)
                inverted.insert(Interval::hull(pending, restricted.greatest()));
        }
        std::swap(map_, inverted.map_);
    }

    /** Insert specified values.
     *
     *  The values can be specified by a interval (or scalar if the interval has an implicit constructor), another set whose
     *  interval type is convertible to this set's interval type, or an IntervalMap whose intervals are convertible.
     *
     * @{ */
    template<class Interval2>
    void insert(const Interval2 &interval) {
        map_.insert(interval, 0);
    }

    template<class Interval2>
    void insertMultiple(const IntervalSet<Interval2> &other) {
        typedef typename IntervalSet<Interval2>::ConstIntervalIterator OtherIterator;
        for (OtherIterator otherIter=other.intervals().begin(); otherIter!=other.intervals().end(); ++otherIter)
            map_.insert(*otherIter, 0);
    }

    template<class Interval2, class T, class Policy>
    void insertMultiple(const IntervalMap<Interval2, T, Policy> &other) {
        typedef typename IntervalMap<Interval2, T, Policy>::ConstIntervalIterator OtherIterator;
        for (OtherIterator otherIter=other.intervals().begin(); otherIter!=other.intervals().end(); ++otherIter)
            map_.insert(*otherIter, 0);
    }
    /** @} */

    /** Remove specified values.
     *
     *  The values can be specified by an interval (or scalar if the interval has an implicit constructor), another set whose
     *  interval type is convertible to this set's interval type, or an IntervalMap whose intervals are convertible.
     *
     * @{ */
    template<class Interval2>
    void erase(const Interval2 &interval) {
        map_.erase(interval);
    }

    template<class Interval2>
    void eraseMultiple(const IntervalSet<Interval2> &other) {
        ASSERT_forbid2((void*)&other==(void*)this, "use IntervalSet::clear() instead");
        typedef typename IntervalSet<Interval2>::ConstIntervalIterator OtherIntervalIterator;
        for (OtherIntervalIterator otherIter=other.intervals().begin(); otherIter!=other.intervals().end(); ++otherIter)
            map_.erase(*otherIter);
    }

    template<class Interval2, class T, class Policy>
    void eraseMultiple(const IntervalMap<Interval2, T, Policy> &other) {
        typedef typename IntervalMap<Interval2, T, Policy>::ConstIntervalIterator OtherIntervalIterator;
        for (OtherIntervalIterator otherIter=other.intervals().begin(); otherIter!=other.intervals().end(); ++otherIter)
            map_.erase(otherIter->first);
    }
    /** @} */

    /** Interset with specified values.
     *
     *  Computes in place intersection of this container with the specified argument.  The argument may be an interval (or
     *  scalar if the interval has an implicit constructor), or another set whose interval type is convertible to this set's
     *  interval type.
     *
     * @{ */
    template<class Interval2>
    void intersect(const Interval2 &interval) {
        if (isEmpty())
            return;
        if (interval.isEmpty()) {
            clear();
            return;
        }
        if (hull().least() < interval.least())
            map_.erase(Interval::hull(hull().least(), interval.least()-1));
        if (hull().greatest() > interval.greatest())
            map_.erase(Interval::hull(interval.greatest(), hull().greatest()));
    }

    template<class Interval2>
    void intersect(IntervalSet<Interval2> other) {
        other.invert(hull());
        map_.eraseMultiple(other.map_);
    }

    template<class Interval2, class T, class Policy>
    void intersect(const IntervalMap<Interval2, T, Policy> &other);// FIXME[Robb Matzke 2014-04-12]: not implemented yet
    /** @} */


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Operators
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /** Determines if two sets contain the same elements. */
    bool operator==(const IntervalSet &other) const {
        return !(*this!=other);
    }

    /** Determines if two sets contain different elements. */
    bool operator!=(const IntervalSet &other) const {
        if (map_.nIntervals()!=other.map_.nIntervals())
            return true;
        for (ConstIntervalIterator ai=intervals().begin(), bi=other.intervals().begin(); ai!=intervals().end(); ++ai, ++bi) {
            if (*ai!=*bi)
                return true;
        }
        return false;
    }
        
    /** Return inverse of specified set. */
    IntervalSet operator~() const {
        IntervalSet tmp = *this;
        tmp.invert();
        return tmp;
    }

    /** In-place union.
     *
     *  Inserts the members of @p other set into @p this set. */
    IntervalSet& operator|=(const IntervalSet &other) {
        insertMultiple(other);
        return *this;
    }
    
    /** In-place union with interval.
     *
     *  Inserts the @p interval into @p this set. */
    IntervalSet& operator|=(const Interval &interval) {
        insert(interval);
        return *this;
    }

    /** Union of two sets. */
    IntervalSet operator|(const IntervalSet &other) const {
        if (nIntervals() < other.nIntervals()) {
            IntervalSet tmp = other;
            tmp.insertMultiple(*this);
            return tmp;
        }
        IntervalSet tmp = *this;
        tmp.insertMultiple(other);
        return tmp;
    }

    /** Union of set with interval.
     *
     *  It's probably more efficient to insert the interval in place, but this method is sometimes convenient. */
    IntervalSet operator|(const Interval &interval) const {
        IntervalSet retval = *this;
        retval.insert(interval);
        return retval;
    }

    /** In-place intersection.
     *
     *  Modifies @p this set so it contains only those members that are also in the @p other set. */
    IntervalSet& operator&=(const IntervalSet &other) {
        intersect(other);
        return *this;
    }

    /** In-place intersection with an interval.
     *
     *  Modifies @p this set so it contains only those members that are also in @p interval. */
    IntervalSet& operator&=(const Interval &interval) {
        intersect(interval);
        return *this;
    }

    /** Intersection of two sets. */
    IntervalSet operator&(const IntervalSet &other) const {
        if (nIntervals() < other.nIntervals()) {
            IntervalSet tmp = other;
            tmp.intersect(*this);
            return tmp;
        }
        IntervalSet tmp = *this;
        tmp.intersect(other);
        return tmp;
    }

    /** Intersection of set with interval. */
    IntervalSet operator&(const Interval &interval) const {
        IntervalSet retval = *this;
        retval.intersect(interval);
        return retval;
    }

    /** In-place subtraction.
     *
     *  Subtracts from @p this set those members of the @p other set. */
    IntervalSet& operator-=(const IntervalSet &other) {
        eraseMultiple(other);
        return *this;
    }

    /** In-place subtraction of an interval.
     *
     *  Removes the specified @p interval of values from @p this set. */
    IntervalSet& operator-=(const Interval &interval) {
        erase(interval);
        return *this;
    }
    
    /** Subtract another set from this one.
     *
     *  <code>A-B</code> is equivalent to <code>A & ~B</code> but perhaps faster. */
    IntervalSet operator-(const IntervalSet &other) const {
        IntervalSet tmp = *this;
        tmp.eraseMultiple(other);
        return tmp;
    }
};

} // namespace
} // namespace

#endif
