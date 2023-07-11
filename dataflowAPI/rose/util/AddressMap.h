// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




#ifndef Sawyer_AddressMap_H
#define Sawyer_AddressMap_H

#include "Access.h"
#include "AddressSegment.h"
#include "Assert.h"
#include "BitVector.h"
#include "Callbacks.h"
#include "Interval.h"
#include "IntervalMap.h"
#include "IntervalSet.h"
#include "Sawyer.h"
#include <ostream>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/cstdint.hpp>
#include <boost/foreach.hpp>
#include <boost/integer_traits.hpp>
#include <boost/lexical_cast.hpp>

namespace Sawyer {
namespace Container {

template<class AddressMap>
struct AddressMapTraits {
    typedef typename AddressMap::NodeIterator NodeIterator;
    typedef typename AddressMap::SegmentIterator SegmentIterator;
};

template<class AddressMap>
struct AddressMapTraits<const AddressMap> {
    typedef typename AddressMap::ConstNodeIterator NodeIterator;
    typedef typename AddressMap::ConstSegmentIterator SegmentIterator;
};

/** Flags for matching constraints. */
typedef unsigned MatchFlags;
static const MatchFlags MATCH_BACKWARD      = 0x00000001; /**< Match in backward direction. */
static const MatchFlags MATCH_CONTIGUOUS    = 0x00000002; /**< Force contiguous matching for methods that default otherwise. */
static const MatchFlags MATCH_NONCONTIGUOUS = 0x00000004; /**< Allow non-contiguous matching for methods that are contiguous. */
static const MatchFlags MATCH_WHOLE         = 0x00000008; /**< Anchor at both specified ends of address interval. */


/** Base class for testing segment constraints. */
template<class A, class T>
class SegmentPredicate {
public:
    struct Args {
        const Sawyer::Container::Interval<A> &interval;
        const AddressSegment<A, T> &segment;
        Args(const Sawyer::Container::Interval<A> &interval, const AddressSegment<A, T> &segment)
            : interval(interval), segment(segment) {}
    };

    virtual ~SegmentPredicate() {}
    virtual bool operator()(bool chain, const Args &) = 0;
};

/** Constraints are used to select addresses from a memory map.
 *
 *  Users don't normally see this class since it's almost always created as a temporary. In fact, most of the public methods in
 *  this class are also present in the AddressMap class, and that's where they're documented.
 *
 *  The purpose of this class is to curry the arguments that would otherwise need to be passed to the various map I/O methods
 *  and which would significantly complicate the API since many of these arguments are optional. */
template<typename AddressMap>
class AddressMapConstraints {
public:
    typedef typename AddressMap::Address Address;
    typedef typename AddressMap::Value Value;
    typedef Sawyer::Container::Interval<Address> AddressInterval;
private:
    AddressMap *map_;                                   // AddressMap<> to which these constraints are bound
    bool never_;                                        // never match anything (e.g., when least_ > greatest_)
    // address constraints
    Optional<Address> least_;                           // least possible valid address
    Optional<Address> greatest_;                        // greatest possible valid address
    Optional<AddressInterval> anchored_;                // anchored least or greatest depending on direction
    // constraints requiring iteration
    size_t maxSize_;                                    // result size is limited
    bool singleSegment_;                                // do not cross a segment boundary
    unsigned requiredAccess_;                           // access bits that must be set in the segment
    unsigned prohibitedAccess_;                         // access bits that must be clear in the segment
    std::string nameSubstring_;                         // segment name must contain substring
    Callbacks<SegmentPredicate<Address, Value>*> segmentPredicates_; // user-supplied segment predicates
public:
    /** Construct a constraint that matches everything. */
    AddressMapConstraints(AddressMap *map)
        : map_(map), never_(false), maxSize_(size_t(-1)), singleSegment_(false), requiredAccess_(0), prohibitedAccess_(0) {}

    // Implicitly construct constraints for a const AddressMap from a non-const address map.
    operator AddressMapConstraints<const AddressMap>() const {
        AddressMapConstraints<const AddressMap> cc(map_);
        if (neverMatches())
            cc = cc.none();
        if (isAnchored()) {
            if (anchored().isSingleton()) {
                cc = cc.at(anchored().least());
            } else {
                cc = cc.at(anchored());
            }
        }
        if (least())
            cc = cc.atOrAfter(*least());
        if (greatest())
            cc = cc.atOrBefore(*greatest());
        cc = cc.limit(limit());
        if (isSingleSegment())
            cc = cc.singleSegment();
        cc = cc.require(required());
        cc = cc.prohibit(prohibited());
        cc = cc.substr(substr());
        cc = cc.segmentPredicates(segmentPredicates());
        return cc;
    }

    /** Print constraints in a human readable form. */
    void print(std::ostream &out) const {
        out <<"{map=" <<map_;
        if (never_)
            out <<", never";
        if (least_)
            out <<", least=" <<*least_;
        if (greatest_)
            out <<", greatest=" <<*greatest_;
        if (anchored_) {
            out <<", anchored=";
            AddressInterval a = *anchored_;
            if (a.isEmpty()) {
                out <<"empty";
            } else if (a.least()==a.greatest()) {
                out <<a.least();
            } else {
                out <<"{" <<a.least() <<".." <<a.greatest() <<"}";
            }
        }
        if (maxSize_ != size_t(-1))
            out <<", limit=" <<maxSize_;
        if (singleSegment_)
            out <<", single-segment";
        if (requiredAccess_!=0) {
            out <<", required="
                <<((requiredAccess_ & Access::READABLE) ? "r" : "")
                <<((requiredAccess_ & Access::WRITABLE) ? "w" : "")
                <<((requiredAccess_ & Access::EXECUTABLE) ? "x" : "")
                <<((requiredAccess_ & Access::IMMUTABLE) ? "i" : "");
            unsigned other = requiredAccess_ & ~(Access::READABLE|Access::WRITABLE|Access::EXECUTABLE|Access::IMMUTABLE);
            if (other)
                out <<"+0x" <<BitVector(8*sizeof requiredAccess_).fromInteger(other).toHex();
        }
        if (prohibitedAccess_!=0) {
            out <<", required="
                <<((prohibitedAccess_ & Access::READABLE) ? "r" : "")
                <<((prohibitedAccess_ & Access::WRITABLE) ? "w" : "")
                <<((prohibitedAccess_ & Access::EXECUTABLE) ? "x" : "")
                <<((prohibitedAccess_ & Access::IMMUTABLE) ? "i" : "");
            unsigned other = prohibitedAccess_ & ~(Access::READABLE|Access::WRITABLE|Access::EXECUTABLE|Access::IMMUTABLE);
            if (other)
                out <<"+0x" <<BitVector(8*sizeof prohibitedAccess_).fromInteger(other).toHex();
        }
        if (!nameSubstring_.empty()) {
            out <<", substr=\"";
            BOOST_FOREACH (char ch, nameSubstring_) {
                switch (ch) {
                    case '\a': out <<"\\a"; break;
                    case '\b': out <<"\\b"; break;
                    case '\t': out <<"\\t"; break;
                    case '\n': out <<"\\n"; break;
                    case '\v': out <<"\\v"; break;
                    case '\f': out <<"\\f"; break;
                    case '\r': out <<"\\r"; break;
                    case '\"': out <<"\\\""; break;
                    case '\\': out <<"\\\\"; break;
                    default:
                        if (isprint(ch)) {
                            out <<ch;
                        } else {
                            char buf[8];
                            sprintf(buf, "\\%03o", (unsigned)(unsigned char)ch);
                            out <<buf;
                        }
                        break;
                }
            }
        }
        if (!segmentPredicates_.isEmpty())
            out <<", user-def";
        out <<"}";
    }

    /** Print constraints in a human readable form. */
    friend std::ostream& operator<<(std::ostream &out, const AddressMapConstraints &x) {
        x.print(out);
        return out;
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Constraint modifiers
public:
    /** Require certain access permissions. See @ref AddressMap::require. */
    AddressMapConstraints require(unsigned bits) const {
        AddressMapConstraints retval = *this;
        retval.requiredAccess_ |= bits;
        return retval;
    }

    /** Prohibit certain access permissions. See @ref AddressMap::prohibit. */
    AddressMapConstraints prohibit(unsigned bits) const {
        AddressMapConstraints retval = *this;
        retval.prohibitedAccess_ |= bits;
        return retval;
    }

    /** Require and prohibit certain access permissions. See @ref AddressMap::access. */
    AddressMapConstraints access(unsigned bits) const {
        return require(bits).prohibit(~bits);
    }

    /** Require certain segment names. See @ref AddressMap::substr. */
    AddressMapConstraints substr(const std::string &s) const {
        ASSERT_require(nameSubstring_.empty() || nameSubstring_==s);// substring conjunction not supported
        AddressMapConstraints retval = *this;
        retval.nameSubstring_ = s;
        return retval;
    }

    /** No constraints. See @ref AddressMap::any. */
    AddressMapConstraints any() const {
        return *this;
    }

    /** Constraints that match nothing. See @ref AddressMap::none. */
    AddressMapConstraints none() const {
        AddressMapConstraints retval = *this;
        retval.never_ = true;
        return retval;
    }

    /** Anchor at a certain address. See @ref AddressMap::at. */
    AddressMapConstraints at(Address x) const {
        AddressMapConstraints retval = *this;
        retval.anchored_ = anchored_ ? *anchored_ & AddressInterval(x) : AddressInterval(x);
        return retval.anchored_->isEmpty() ? retval.none() : retval;
    }

    /**  Anchor at a certain interval. See @ref AddressMap::at. */
    AddressMapConstraints at(const Sawyer::Container::Interval<Address> &x) const {
        AddressMapConstraints retval = *this;
        retval.anchored_ = anchored_ ? *anchored_ & x : x;
        return retval.anchored_->isEmpty() ? retval.none() : retval.atOrAfter(x.least()).atOrBefore(x.greatest());
    }

    /** Limit matching length. See @ref AddressMap::limit. */
    AddressMapConstraints limit(size_t x) const {
        AddressMapConstraints retval = *this;
        retval.maxSize_ = std::min(maxSize_, x);
        return 0 == retval.maxSize_ ? retval.none() : retval;
    }

    /** Limit addresses. See @ref AddressMap::atOrAfter. */
    AddressMapConstraints atOrAfter(Address least) const {
        AddressMapConstraints retval = *this;
        if (least_) {
            retval.least_ = std::max(*least_, least);
        } else {
            retval.least_ = least;
        }
        if (greatest_ && *greatest_ < *retval.least_)
            retval.none();
        return retval;
    }

    /**  Limit addresses. See @ref AddressMap::atOrBefore. */
    AddressMapConstraints atOrBefore(Address greatest) const {
        AddressMapConstraints retval = *this;
        if (greatest_) {
            retval.greatest_ = std::min(*greatest_, greatest);
        } else {
            retval.greatest_ = greatest;
        }
        if (least_ && *least_ > *retval.greatest_)
            retval.none();
        return retval;
    }

    /** Limit addresses.  See @ref AddressMap::within. */
    AddressMapConstraints within(const Sawyer::Container::Interval<Address> &x) const {
        return x.isEmpty() ? none() : atOrAfter(x.least()).atOrBefore(x.greatest());
    }

    /** Limit addresses.  See @ref AddressMap::within. */
    AddressMapConstraints within(Address lo, Address hi) const {
        return lo<=hi ? within(Sawyer::Container::Interval<Address>::hull(lo, hi)) : none();
    }

    /** Limit addresses.  See @ref AddressMap::baseSize. */
    AddressMapConstraints baseSize(Address base, Address size) const {
        return size>0 ? atOrAfter(base).atOrBefore(base+size-1) : none();
    }

    /** Limit addresses. See @ref AddressMap::after. */
    AddressMapConstraints after(Address x) const {
        return x==boost::integer_traits<Address>::const_max ? none() : atOrAfter(x+1);
    }

    /** Limit addreses.  See @ref AddressMap::before. */
    AddressMapConstraints before(Address x) const {
        return x==boost::integer_traits<Address>::const_min ? none() : atOrBefore(x-1);
    }

    /** Limit matching to single segment.  See @ref AddressMap::singleSegment. */
    AddressMapConstraints singleSegment() const {
        AddressMapConstraints retval = *this;
        retval.singleSegment_ = true;
        return retval;
    }

    /** Limit segments.  See @ref AddressMap::segmentPredicate. */
    AddressMapConstraints segmentPredicate(SegmentPredicate<Address, Value> *p) const {
        if (!p)
            return none();
        AddressMapConstraints retval = *this;
        retval.segmentPredicates_.append(p);
        return retval;
    }

    AddressMapConstraints segmentPredicates(const Callbacks<SegmentPredicate<Address, Value>*> &predicates) const {
        AddressMapConstraints retval = *this;
        retval.segmentPredicates_.append(predicates);
        return retval;
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Constraint queries
public:
    /** Accessibility bits that are required to be set. */
    unsigned required() const {
        return requiredAccess_;
    }

    /** Accessibility bits that are required to be clear. */
    unsigned prohibited() const {
        return prohibitedAccess_;
    }

    /** Section name substring.  Returns the string that must be present as a substring in a section name. */
    const std::string& substr() const {
        return nameSubstring_;
    }

    /** Returns true if the constraint is not allowed to match anything.  This returns true if the @ref none constraint is
     * set, but it doesn't necessarily return true if a combination of other constraints results in something that cannot
     * match. */
    bool neverMatches() const {
        return never_;
    }

    /** Determines whether constraints are anchored to an address. */
    bool isAnchored() const {
        return anchored_;
    }

    /** Returns the anchor points. Returns the anchor points as an interval, where the anchor(s) is an endpoint of the returned
     * interval.  For instance, if the @ref at method was called, then @ref anchored will return a singleton interval whose
     * value was the @ref at argument. The return value will be an empty interval if mutually exclusive anchors are set. This
     * method should not be called without first calling @ref isAnchored. */
    AddressInterval anchored() const {
        ASSERT_require(isAnchored());
        return *anchored_;
    }

    /** Size limit.  Returns the size limit or the maximum possible size_t value if not limit is in effect. */
    size_t limit() const {
        return maxSize_;
    }

    /** Least possible address.  Returns the least possible address or nothing. The least address is in effect for the @ref
     * atOrAfter constraint. */
    const Optional<Address>& least() const {
        return least_;
    }

    /** Greatest possible address. Returns the greatest possible address or nothing. The greatest address is in effect for the
     * @ref atOrBefore constraint. */
    const Optional<Address>& greatest() const {
        return greatest_;
    }

    /** Returns true if the single-segment constraint is set. */
    bool isSingleSegment() const {
        return singleSegment_;
    }

    /** Returns the segment predicates. */
    const Callbacks<SegmentPredicate<Address, Value>*> segmentPredicates() const {
        return segmentPredicates_;
    }

    /** Returns a pointer to the memory map for this constraint object. */
    AddressMap* map() const {
        return map_;
    }

    /** Determines whether non-address constraints are present.  The presence of non-address constraints requires iteration
     *  over matching addresses to find applicable regions of the address space. */
    bool hasNonAddressConstraints() const {
        return (!never_ &&
                (requiredAccess_ || prohibitedAccess_ || !nameSubstring_.empty() || maxSize_!=size_t(-1) ||
                 singleSegment_ || !segmentPredicates_.isEmpty()));
    }

    /** Construct new constraints from existing address constraints.  Constructs a new constraints object from an existing
     *  but using only the address constraints of the existing object and none of its non-address constraints. */
    AddressMapConstraints addressConstraints() const {
        AddressMapConstraints c(map_);
        c.least_ = least_;
        c.greatest_ = greatest_;
        c.anchored_ = anchored_;
        c.maxSize_ = maxSize_;
        return c;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Methods that directly call the AddressMap
public:
    boost::iterator_range<typename AddressMapTraits<AddressMap>::NodeIterator>
    nodes(MatchFlags flags=0) const {
        return map_->nodes(*this, flags);
    }

    boost::iterator_range<typename AddressMapTraits<AddressMap>::SegmentIterator>
    segments(MatchFlags flags=0) const {
        return map_->segments(*this, flags);
    }

    Optional<typename AddressMap::Address>
    next(MatchFlags flags=0) const {
        return map_->next(*this, flags);
    }

    Sawyer::Container::Interval<Address>
    available(MatchFlags flags=0) const {
        return map_->available(*this, flags);
    }

    bool
    exists(MatchFlags flags=0) const {
        return map_->exists(*this, flags);
    }

    typename AddressMapTraits<AddressMap>::NodeIterator
    findNode(MatchFlags flags=0) const {
        return map_->findNode(*this, flags);
    }

    template<typename Functor>
    void
    traverse(Functor &functor, MatchFlags flags=0) const {
        return map_->traverse(functor, *this, flags);
    }
    void
    traverse(typename AddressMap::Visitor &visitor, MatchFlags flags=0) const {
        return map_->template traverse<typename AddressMap::Visitor>(visitor, *this, flags);
    }
    
    Sawyer::Container::Interval<Address>
    read(typename AddressMap::Value *buf /*out*/, MatchFlags flags=0) const {
        return map_->read(buf, *this, flags);
    }

    Sawyer::Container::Interval<Address>
    read(std::vector<typename AddressMap::Value> &buf /*out*/,
         MatchFlags flags=0) const {
        return map_->read(buf, *this, flags);
    }

    Sawyer::Container::Interval<Address>
    write(const typename AddressMap::Value *buf, MatchFlags flags=0) const {
        return map_->write(buf, *this, flags);
    }

    Sawyer::Container::Interval<Address>
    write(const std::vector<typename AddressMap::Value> &buf, MatchFlags flags=0) {
        return map_->write(buf, *this, flags);
    }
    
    void
    prune(MatchFlags flags=0) const {
        return map_->prune(*this, flags);
    }

    void
    keep(MatchFlags flags=0) const {
        return map_->keep(*this, flags);
    }

    void
    changeAccess(unsigned requiredAccess, unsigned prohibitedAccess, MatchFlags flags=0) const {
        return map_->changeAccess(requiredAccess, prohibitedAccess, *this, flags);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Implementation details
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace AddressMapImpl {

// Used internally to split and merge segments
template<class A, class T>
class SegmentMergePolicy {
public:
    typedef A Address;
    typedef T Value;
    typedef AddressSegment<A, T> Segment;

    bool merge(const Sawyer::Container::Interval<Address> &leftInterval, Segment &leftSegment,
               const Sawyer::Container::Interval<Address> &rightInterval, Segment &rightSegment) {
        ASSERT_forbid(leftInterval.isEmpty());
        ASSERT_forbid(rightInterval.isEmpty());
        ASSERT_require(leftInterval.greatest() + 1 == rightInterval.least());
        return (leftSegment.accessibility() == rightSegment.accessibility() &&
                leftSegment.name() == rightSegment.name() &&
                leftSegment.buffer() == rightSegment.buffer() &&
                leftSegment.offset() + leftInterval.size() == rightSegment.offset());
    }

    Segment split(const Sawyer::Container::Interval<Address> &interval, Segment &segment, Address splitPoint) {
        ASSERT_forbid(interval.isEmpty());
        ASSERT_require(interval.isContaining(splitPoint));
        Segment right = segment;
        right.offset(segment.offset() + splitPoint - interval.least());
        return right;
    }

    void truncate(const Sawyer::Container::Interval<Address> &interval, Segment &segment, Address splitPoint) {
        ASSERT_forbid(interval.isEmpty());
        ASSERT_require(interval.isContaining(splitPoint));
    }
};

// The results for matching constraints
template<class AddressMap>
struct MatchedConstraints {
    typedef typename AddressMap::Address Address;
    Sawyer::Container::Interval<Address> interval_;
    typedef typename AddressMapTraits<AddressMap>::NodeIterator NodeIterator;
    boost::iterator_range<NodeIterator> nodes_;
};

// Finds the minimum possible address and node iterator for the specified constraints in this map and returns that
// iterator.  Returns the end iterator if the constraints match no address.  If a non-end iterator is returned then minAddr
// is adjusted to be the minimum address that satisfies the constraint (it will be an address within the returned node, but
// not necessarily the least address for the node).  If useAnchor is set and the constraints specify an anchor, then the
// anchor address must be present in the map and satisfy any address constraints that might also be present.
template<class AddressMap>
typename AddressMapTraits<AddressMap>::NodeIterator
constraintLowerBound(AddressMap &amap, const AddressMapConstraints<AddressMap> &c, bool useAnchor,
                     typename AddressMap::Address &minAddr) {
    typedef typename AddressMapTraits<AddressMap>::NodeIterator Iterator;
    if (amap.isEmpty() || c.neverMatches())
        return amap.nodes().end();

    if (useAnchor && c.isAnchored()) {                  // forward matching if useAnchor is set
        if ((c.least() && *c.least() > c.anchored().least()) || (c.greatest() && *c.greatest() < c.anchored().greatest()))
            return amap.nodes().end();                  // anchor is outside of allowed interval
        Iterator lb = amap.lowerBound(c.anchored().least());
        if (lb==amap.nodes().end() || c.anchored().least() < lb->key().least())
            return amap.nodes().end();                  // anchor is not present in this map
        minAddr = c.anchored().least();
        return lb;
    }

    if (c.least()) {
        Iterator lb = amap.lowerBound(*c.least());
        if (lb==amap.nodes().end())
            return lb;                                  // least is above all segments
        minAddr = std::max(*c.least(), lb->key().least());
        return lb;
    }

    Iterator lb = amap.nodes().begin();
    if (lb!=amap.nodes().end())
        minAddr = lb->key().least();
    return lb;
}

// Finds the maximum possible address and node for the specified constraints in this map, and returns an iterator to the
// following node.  Returns the begin iterator if the constraints match no address.  If a non-begin iterator is returned
// then maxAddr is adjusted to be the maximum address that satisfies the constraint (it will be an address that belongs to
// the node immediately prior to the one pointed to by the returned iterator, but not necessarily the greatest address for
// that node).  If useAnchor is set and the constraints specify an anchor, then the anchor address must be present in the
// map and satisfy any address constraints that might also be present.
template<class AddressMap>
typename AddressMapTraits<AddressMap>::NodeIterator
constraintUpperBound(AddressMap &amap, const AddressMapConstraints<AddressMap> &c, bool useAnchor,
                     typename AddressMap::Address &maxAddr) {
    typedef typename AddressMapTraits<AddressMap>::NodeIterator Iterator;
    if (amap.isEmpty() || c.neverMatches())
        return amap.nodes().begin();

    if (useAnchor && c.isAnchored()) {                  // backward matching if useAnchor is set
        if ((c.least() && *c.least() > c.anchored().least()) || (c.greatest() && *c.greatest() < c.anchored().greatest()))
            return amap.nodes().begin();                // anchor is outside allowed interval
        Iterator ub = amap.findPrior(c.anchored().greatest());
        if (ub==amap.nodes().end() || c.anchored().greatest() > ub->key().greatest())
            return amap.nodes().begin();                // anchor is not present in this map
        maxAddr = c.anchored().greatest();
        return ++ub;                                    // return node after the one containing the anchor
    }

    if (c.greatest()) {
        Iterator ub = amap.findPrior(*c.greatest());
        if (ub==amap.nodes().end())
            return amap.nodes().begin();            // greatest is below all segments
        maxAddr = std::min(ub->key().greatest(), *c.greatest());
        return ++ub;                                // return node after the one containing the maximum
    }

    maxAddr = amap.hull().greatest();
    return amap.nodes().end();
}

// Returns true if the segment satisfies the non-address constraints in c.
template<class AddressMap>
bool
isSatisfied(const typename AddressMap::Node &node, const AddressMapConstraints<AddressMap> &c) {
    typedef typename AddressMap::Address Address;
    typedef typename AddressMap::Value Value;
    typedef typename AddressMap::Segment Segment;
    const Segment &segment = node.value();
    if (!segment.isAccessible(c.required(), c.prohibited()))
        return false;                               // wrong segment permissions
    if (!boost::contains(segment.name(), c.substr()))
        return false;                               // wrong segment name
    if (!c.segmentPredicates().apply(true, typename SegmentPredicate<Address, Value>::Args(node.key(), node.value())))
        return false;                               // user-supplied predicates failed
    return true;
}

// Matches constraints against contiguous addresses in a forward direction.
template<class AddressMap>
MatchedConstraints<AddressMap>
matchForward(AddressMap &amap, const AddressMapConstraints<AddressMap> &c, MatchFlags flags) {
    typedef typename AddressMap::Address Address;
    typedef typename AddressMapTraits<AddressMap>::NodeIterator Iterator;
    MatchedConstraints<AddressMap> retval;
    retval.nodes_ = boost::iterator_range<Iterator>(amap.nodes().end(), amap.nodes().end());
    if (c.neverMatches() || amap.isEmpty())
        return retval;

    // Find a lower bound for the minimum address
    Address minAddr = 0;
    Iterator begin = constraintLowerBound(amap, c, true, minAddr /*out*/);
    if (begin == amap.nodes().end())
        return retval;

    // Find an upper bound for the maximum address.
    Address maxAddr = 0;
    Iterator end = constraintUpperBound(amap, c, false, maxAddr /*out*/);
    if (end==amap.nodes().begin())
        return retval;

    // Advance the lower-bound until it satisfies the other (non-address) constraints
    while (begin!=end && !isSatisfied(*begin, c)) {
        if (c.isAnchored())
            return retval;                          // match is anchored to minAddr
        ++begin;
    }
    if (begin==end)
        return retval;
    minAddr = std::max(minAddr, begin->key().least());

    // Iterate forward until the constraints are no longer satisfied
    if ((flags & MATCH_CONTIGUOUS)!=0 || c.hasNonAddressConstraints()) {
        Address addr = minAddr;
        Iterator iter = begin;
        size_t nElmtsFound = 0;
        for (/*void*/; iter!=end; ++iter) {
            if (iter!=begin) {                      // already tested the first node above
                if (c.isSingleSegment())
                    break;                          // we crossed a segment boundary
                if ((flags & MATCH_CONTIGUOUS)!=0 && addr+1 != iter->key().least())
                    break;                          // gap between segments
                if (!isSatisfied(*iter, c)) {
                    if ((flags & MATCH_WHOLE)!=0)
                        return retval;              // match is anchored to maxAddr
                    break;                          // segment does not satisfy constraints
                }
            }
            size_t nElmtsHere = iter->key().greatest() + 1 - std::max(minAddr, iter->key().least());
            if (nElmtsFound + nElmtsHere >= c.limit()) {
                size_t nNeed = c.limit() - nElmtsFound;
                addr = std::max(minAddr, iter->key().least()) + nNeed - 1;
                ++iter;
                break;                              // too many values
            }
            addr = iter->key().greatest();
            nElmtsFound += nElmtsHere;
        }
        end = iter;
        maxAddr = std::min(maxAddr, addr);
    }

    // Build the result
    retval.interval_ = Sawyer::Container::Interval<Address>::hull(minAddr, maxAddr);
    retval.nodes_ = boost::iterator_range<Iterator>(begin, end);
    return retval;
}

// Matches constraints against contiguous addresses in a backward direction.
template<class AddressMap>
MatchedConstraints<AddressMap>
matchBackward(AddressMap &amap, const AddressMapConstraints<AddressMap> &c, MatchFlags flags) {
    typedef typename AddressMap::Address Address;
    typedef typename AddressMapTraits<AddressMap>::NodeIterator Iterator;
    MatchedConstraints<AddressMap> retval;
    retval.nodes_ = boost::iterator_range<Iterator>(amap.nodes().end(), amap.nodes().end());
    if (c.neverMatches() || amap.isEmpty())
        return retval;

    // Find a lower bound for the minimum address
    Address minAddr = 0;
    Iterator begin = constraintLowerBound(amap, c, false, minAddr /*out*/);
    if (begin == amap.nodes().end())
        return retval;

    // Find an upper bound for the maximum address.
    Address maxAddr = 0;
    Iterator end = constraintUpperBound(amap, c, true, maxAddr /*out*/);
    if (end==amap.nodes().begin())
        return retval;

    // Decrement the upper bound until constraints are met. End always points to one-past the last matching node.
    while (end!=begin) {
        Iterator prev = end; --prev;
        if (isSatisfied(*prev, c)) {
            maxAddr = std::min(maxAddr, prev->key().greatest());
            break;
        }
        if (c.isAnchored())
            return retval;                          // match is anchored to maxAddr
        end = prev;
    }
    if (end==begin)
        return retval;

    // Iterate backward until the constraints are no longer satisfied. Within the loop, iter always points to on-past the
    // node in question.  When the loop exits, iter points to the first node satisfying constraints.
    if ((flags & MATCH_CONTIGUOUS)!=0 || c.hasNonAddressConstraints()) {
        Address addr = maxAddr;
        Iterator iter = end;
        size_t nElmtsFound = 0;
        for (/*void*/; iter!=begin; --iter) {
            Iterator prev = iter; --prev;           // prev points to the node in question
            if (iter!=end) {                        // already tested last node above
                if (c.isSingleSegment())
                    break;                          // we crossed a segment boundary
                if ((flags & MATCH_CONTIGUOUS)!=0 && prev->key().greatest()+1 != addr)
                    break;                          // gap between segments
                if (!isSatisfied(*prev, c)) {
                    if ((flags & MATCH_WHOLE)!=0)
                        return retval;              // match is anchored to minAddr
                    break;                          // segment does not satisfy constraints
                }
            }
            size_t nElmtsHere = std::min(maxAddr, prev->key().greatest()) - prev->key().least() + 1;
            if (nElmtsFound + nElmtsHere >= c.limit()) {
                size_t nNeed = c.limit() - nElmtsFound;
                addr = std::min(maxAddr, prev->key().greatest()) - nNeed + 1;
                iter = prev;
                break;
            }
            addr = prev->key().least();
            nElmtsFound += nElmtsHere;
        }
        begin = iter;                               // iter points to first matching node
        minAddr = std::max(minAddr, addr);
    }

    // Build the result
    retval.interval_ = Sawyer::Container::Interval<Address>::hull(minAddr, maxAddr);
    retval.nodes_ = boost::iterator_range<Iterator>(begin, end);
    return retval;
}

// Match constraints forward or backward
template<class AddressMap>
MatchedConstraints<AddressMap>
matchConstraints(AddressMap &amap, const AddressMapConstraints<AddressMap> &c, MatchFlags flags) {
    if ((flags & MATCH_BACKWARD) != 0)
        return matchBackward(amap, c, flags);
    return matchForward(amap, c, flags);
}

} // namespace


/** A mapping from address space to values.
 *
 *  This object maps addresses (actually, intervals thereof) to values. Addresses must be an integral unsigned type but values
 *  may be any type as long as they are default constructible and copyable.  The address type is usually an integral type whose
 *  width is the log base 2 of the size of the address space; the value type is often unsigned 8-bit bytes.
 *
 *  An address map accomplishes the mapping by inheriting from an @ref IntervalMap, whose intervals are
 *  <code>Interval<A></code> and whose values are <code>AddressSegment<A,T></code>. The @ref AddressSegment objects
 *  point to reference-counted @ref Buffer objects that hold the values.  The same values can be mapped at different addresses
 *  by inserting segments at those addresses that point to a common buffer.
 *
 *  An address map implements read and write concepts for copying values between user-supplied buffers and the storage areas
 *  referenced by the map. Many of the address map methods operate over a region of the map described by a set of constraints
 *  that are matched within the map.  Constraints are indicated by listing them before the map I/O operation, but they do not
 *  modify the map in any way--they exist outside the map.  Constraints are combined by logical conjunction. For instance, the
 *  @ref AddressMap::next method returns the lowest address that satisfies the given constraints, or nothing.  If we wanted
 *  to search for the lowest address beteen 1000 and 1999 inclusive, that has read access but not write access, we would
 *  do so like this:
 *
 * @code
 *  Optional<Address> x = map.within(1000,1999).require(READABLE).prohibit(WRITABLE).next();
 * @endcode
 *
 *  In fact, by making use of the @ref Sawyer::Optional API, we can write a loop that iterates over such addresses, although
 *  there may be more efficient ways to do this than one address at a time:
 *
 * @code
 *  for (Address x=1000; map.within(x,1999).require(READABLE).prohibit(WRITABLE).next().assignTo(x); ++x) ...
 * @endcode
 *
 *  The next example shows how to read a buffer's worth of values anchored at a particular starting value.  The @ref read
 *  method returns an address interval to indicate what addresses were accessed, but in this case we're only interested in the
 *  number of such addresses since we know the starting address.
 *
 * @code
 *  Value buf[10];
 *  size_t nAccessed = map.at(1000).limit(10).read(buf).size();
 * @endcode
 *
 *  An interval return value is more useful when we don't know where the operation occurs until after it occurs.  For instance,
 *  to read up to 10 values that are readable at or beyond some address:
 *
 * @code
 *  Value buf[10]
 *  Interval<Address> accessed = map.atOrAfter(1000).limit(10).require(READABLE).read(buf);
 * @endcode
 *
 *  Since all map operations take the same constraints, it is possible to rewrite the previous @c for loop so that instead of
 *  searching for an address it actually reads data.  Here's a loop that prints all the data that's readable but not writable
 *  and falls between two addresses, regardless of what other segments also exist in that same interval:
 *
 * @code
 *  static const size_t bufsz = 256;
 *  Value buf[bufsz];
 *  Address addr = 1000;
 *  while (Interval accessed = map.within(addr, 1999).require(READABLE).prohibit(WRITABLE).limit(bufsz).read(buf)) {
 *      for (Address i=accessed.least(); i<=access.greatest(); ++i)
 *          std::cout <<i <<": " <<buf[i-accessed.least()] <<"\n";
 *      addr += accessed.size();
 *  }
 * @endcode
 *
 *  Most I/O methods require that constraints match only contiguous addresses.  If there is an intervening address that does
 *  not satisfy the constraint, including addresses that are not mapped, then the matched range terminates at the non-matching
 *  address.  However, the @c MATCH_NONCONTIGUOUS flag can be used to relax this, in which case the matched interval of
 *  addresses may include addresses that are not mapped.  Regardless of whether contiguous addresses are required, the returned
 *  interval of addresses will never contain an address that is mapped and does not also satisfy the constraints. I/O
 *  operations (read and write) require contiguous addresses, but some other methods don't. For instance, the expressions
 *
 * @code
 *  Interval<Address> found1 = map.within(100,200).require(READABLE).available(MATCH_CONTIGUOUS);
 *  Interval<Address> found2 = map.within(100,200).require(READABLE).available(MATCH_NONCONTIGUOUS);
 * @endcode
 *
 *  are the same except the second one returns an interval that might include non-mapped addresses.  A few methods go even
 *  further and are able to operate across mapped addresses that don't satisfy the segment constraints, skipping over the
 *  addresses that don't satisfy the constraints.  For instance, the @ref prune and @ref keep functions operate this way so
 *  that a call like:
 *
 * @code
 *  map.within(100,200).require(READABLE).prohibit(WRITABLE).keep();
 * @endcode
 *
 *  will discard all addresses except keeping those which are between 100 and 200 (inclusive) and which are readable but not
 *  writable. The documentation for each method states whether the default is contiguous matching, non-contiguous matching, or
 *  skipping over whole segments, and the method can take a bit flag (@ref MatchFlags) to change its default behavior (with
 *  some restrictions).
 *
 *  One of the @ref MatchFlags bits indicates whether the constraint should match the lowest or highest possible addresses. The
 *  default is to match the constraint at the lowest possible addresses. Matching at the highest addresses is useful when
 *  iterating backward.  For instance, if one wants to read up to 1024 values that end at address 1023 but is not sure how many
 *  prior addresses are readable, he could use backward matching.  This is much more efficient than the alternative of
 *  searching backward one address at a time, and is simpler than doing an explicit binary search:
 *
 * @code
 *  Value buf[1024];
 *  Interval<Address> accessed = map.at(1023).limit(1024).read(buf, MATCH_BACKWARD);
 * @endcode
 *
 *  Backward and forward I/O behaves identically as far as which array element holds which value. In all cases array element
 *  zero contains the value at the lowest address read or written.
 *
 *  Here's an example that creates two buffers (they happen to point to arrays that the Buffer objects do not own), maps them
 *  at addresses in such a way that part of the smaller of the two buffers occludes the larger buffer, and then
 *  performs a write operation that touches parts of both buffers.  We then rewrite part of the mapping and do another write
 *  operation:
 *
 * @code
 *  using namespace Sawyer::Container;
 *
 *  typedef unsigned Address;
 *  typedef Interval<Address> Addresses;
 *  typedef Buffer<Address, char>::Ptr BufferPtr;
 *  typedef AddressSegment<Address, char> Segment;
 *  typedef AddressMap<Address, char> MemoryMap;
 *  
 *  // Create some buffer objects
 *  char data1[15];
 *  memcpy(data1, "---------------", 15);
 *  BufferPtr buf1 = Sawyer::Container::StaticBuffer<Address, char>::instance(data1, 15);
 *  char data2[5];
 *  memcpy(data2, "##########", 10);
 *  BufferPtr buf2 = Sawyer::Container::StaticBuffer<Address, char>::instance(data2, 5); // using only first 5 bytes
 *  
 *  // Map data2 into the middle of data1
 *  MemoryMap map;
 *  map.insert(Addresses::baseSize(1000, 15), Segment(buf1));
 *  map.insert(Addresses::baseSize(1005,  5), Segment(buf2)); 
 *  
 *  // Write across both buffers and check that data2 occluded data1
 *  Addresses accessed = map.at(1001).limit(13).write("bcdefghijklmn");
 *  ASSERT_always_require(accessed.size()==13);
 *  ASSERT_always_require(0==memcmp(data1, "-bcde-----klmn-", 15));
 *  ASSERT_always_require(0==memcmp(data2,      "fghij#####", 10));
 *  
 *  // Map the middle of data1 over the top of data2 again and check that the mapping has one element. I.e., the three
 *  // separate parts were recombined into a single entry since they are three consecutive areas of a single buffer.
 *  map.insert(Addresses::baseSize(1005, 5), Segment(buf1, 5));
 *  ASSERT_always_require(map.nSegments()==1);
 *  
 *  // Write some data again
 *  accessed = map.at(1001).limit(13).write("BCDEFGHIJKLMN");
 *  ASSERT_always_require(accessed.size()==13);
 *  ASSERT_always_require(0==memcmp(data1, "-BCDEFGHIJKLMN-", 15));
 *  ASSERT_always_require(0==memcmp(data2,      "fghij#####", 10));
 * @endcode
 *
 * @section errors Microsoft C++ compilers
 *
 * Microsoft Visual Studio 12 2013 (MSVC 18.0.30501.0) and possibly other versions look up non-dependent names in template base
 * classes in violation of C++ standards and apparently no switch to make their behavior compliant with the standard.  This
 * causes problems for AddressMap because unqualified references to <tt>%Interval</tt> should refer to the
 * Sawyer::Container::Interval class template, but instead they refer to the @ref IntervalMap::Interval "Interval" typedef in
 * the base class.  Our work around is to qualify all occurrences of <tt>%Interval</tt> where Microsoft compilers go wrong. */
template<class A, class T = boost::uint8_t>
class AddressMap: public IntervalMap<Interval<A>, AddressSegment<A, T>, AddressMapImpl::SegmentMergePolicy<A, T> > {
    // "Interval" is qualified to work around bug in Microsoft compilers. See doxygen note above.
    typedef IntervalMap<Sawyer::Container::Interval<A>, AddressSegment<A, T>, AddressMapImpl::SegmentMergePolicy<A, T> > Super;

public:
    typedef A Address;                                  /**< Type for addresses. This should be an unsigned type. */
    typedef T Value;                                    /**< Type of data stored in the address space. */
    typedef AddressSegment<A, T> Segment;               /**< Type of segments stored by this map. */
    typedef Sawyer::Container::Buffer<Address, Value> Buffer;
    typedef typename Super::Node Node;                  /**< Storage node containing interval/segment pair. */
    typedef typename Super::ValueIterator SegmentIterator; /**< Iterates over segments in the map. */
    typedef typename Super::ConstValueIterator ConstSegmentIterator; /**< Iterators over segments in the map. */
    typedef typename Super::ConstIntervalIterator ConstIntervalIterator; /**< Iterates over address intervals in the map. */
    typedef typename Super::NodeIterator NodeIterator;  /**< Iterates over address interval, segment pairs in the map. */
    typedef typename Super::ConstNodeIterator ConstNodeIterator; /**< Iterates over address interval/segment pairs in the map. */

    /** Constructs an empty address map. */
    AddressMap() {}

    /** Copy constructor.
     *
     *  The new address map has the same addresses mapped to the same buffers as the @p other map.  The buffers themselves are
     *  not copied since they are reference counted.
     *
     *  If @p copyOnWrite is set then the buffers are marked so that any subsequent write to that buffer via the @ref write
     *  method from any AddressMap object will cause a new copy to be created and used by the AddressMap that's doing the
     *  writing. One should be careful when buffers are intended to be shared because setting the copy-on-write bit on the
     *  buffer will cause the sharing to be broken.  For example, if map1 is created and then copied into map2 with the
     *  copy-on-write bit cleared, then any writes to the buffer via map1 will be visible when reading from map2 and vice
     *  versa.  However, if map3 is then created by copying either map1 or map2 with the copy-on-write bit set, then writes to
     *  any of the three maps will cause that map to obtain an independent copy of the buffer, effectively removing the sharing
     *  that was intended between map1 and map2.  Another thing to be aware of is that some buffer types will return a
     *  different buffer type when they're copied.  For instance, copying a @ref StaticBuffer or @ref MappedBuffer will return
     *  an @ref AllocatingBuffer. */
    AddressMap(const AddressMap &other, bool copyOnWrite=false): Super(other) {
        if (copyOnWrite) {
            BOOST_FOREACH (Segment &segment, this->values()) {
                if (const typename Buffer::Ptr &buffer = segment.buffer())
                    buffer->copyOnWrite(true);
            }
        }
    }

    /** Constraint: required access bits.
     *
     *  Constrains address to those that have all of the access bits that are set in @p x.
     *
     * @{ */
    AddressMapConstraints<const AddressMap> require(unsigned x) const {
        return AddressMapConstraints<const AddressMap>(this).require(x);
    }
    AddressMapConstraints<AddressMap> require(unsigned x) {
        return AddressMapConstraints<AddressMap>(this).require(x);
    }
    /** @} */

    /** Constraint: prohibited access bits.
     *
     *  Constrains addresses to those that have none of the access bits that are set in @p x.
     *
     * @{ */
    AddressMapConstraints<const AddressMap> prohibit(unsigned x) const {
        return AddressMapConstraints<const AddressMap>(this).prohibit(x);
    }
    AddressMapConstraints<AddressMap> prohibit(unsigned x) {
        return AddressMapConstraints<AddressMap>(this).prohibit(x);
    }
    /** @} */

    /** Constraint: required and prohibited access bits.
     *
     *  Constrains address to those that have the specified access bits. This method is the same as calling @ref require with
     *  the specified bit vector, and @ref prohibit with the inverted bit vector.
     *
     * @{ */
    AddressMapConstraints<const AddressMap> access(unsigned x) const {
        return AddressMapConstraints<const AddressMap>(this).access(x);
    }
    AddressMapConstraints<AddressMap> access(unsigned x) {
        return AddressMapConstraints<AddressMap>(this).access(x);
    }
    /** @} */

    /** Constraint: segment name substring.
     *
     *  Constrains addresses to those that belong to a segment that contains string @p x as part of its name.
     *
     * @{ */
    AddressMapConstraints<const AddressMap> substr(const std::string &x) const {
        return AddressMapConstraints<const AddressMap>(this).substr(x);
    }
    AddressMapConstraints<AddressMap> substr(const std::string &x) {
        return AddressMapConstraints<AddressMap>(this).substr(x);
    }
    /** @} */

    /** Constraint: anchor point.
     *
     *  Constrains addresses to a sequence that begins at @p x.  If address @p x is not part of the addresses matched by the
     *  other constraints, then no address matches.
     *
     * @{ */
    AddressMapConstraints<const AddressMap> at(Address x) const {
        return AddressMapConstraints<const AddressMap>(this).at(x);
    }
    AddressMapConstraints<AddressMap> at(Address x) {
        return AddressMapConstraints<AddressMap>(this).at(x);
    }
    /** @} */

    /** Constraint: anchored interval.
     *
     *  Constrains addresses so that the lowest or highest matched address is the specified anchor point.  When matching
     *  constraints in the forward direction (the default) then the anchor must be the lowest address, and when matching in the
     *  backward direction the anchor must be the highest address.  The direction is specified by an argument to the
     *  operation.
     *
     *  For instance:
     *
     * @code
     *  map.at(100).limit(10).read(buf);               // 1
     *  map.at(100).limit(10).read(buf, MATCH_BACKWARD); // 2
     * @endcode
     *
     *  Expression 1 reads up to 10 values such that the lowest value read is at address 100, while expression 2 reads up to 10
     *  values such that the highest value read is at address 100.  In both cases, if address 100 is not mapped (or otherwise
     *  does not satisfy the constraints) then nothing is read.
     *
     * @{ */
    AddressMapConstraints<const AddressMap> at(const Sawyer::Container::Interval<Address> &x) const {
        return AddressMapConstraints<const AddressMap>(this).at(x);
    }
    AddressMapConstraints<AddressMap> at(const Sawyer::Container::Interval<Address> &x) {
        return AddressMapConstraints<AddressMap>(this).at(x);
    }
    /** @} */

    /** Constraint: limit matched size.
     *
     *  Constrains the matched addresses so that at most @p x addresses match.  Forward matching matches the first @p x
     *  addresses while backward matching matches the last @p x addresses.
     *
     * @{ */
    AddressMapConstraints<const AddressMap> limit(size_t x) const {
        return AddressMapConstraints<const AddressMap>(this).limit(x);
    }
    AddressMapConstraints<AddressMap> limit(size_t x) {
        return AddressMapConstraints<AddressMap>(this).limit(x);
    }
    /** @} */

    /** Constraint: address lower bound.
     *
     *  Constrains matched addresses so that they are all greater than or equal to @p x.
     *
     * @{ */
    AddressMapConstraints<const AddressMap> atOrAfter(Address x) const {
        return AddressMapConstraints<const AddressMap>(this).atOrAfter(x);
    }
    AddressMapConstraints<AddressMap> atOrAfter(Address x) {
        return AddressMapConstraints<AddressMap>(this).atOrAfter(x);
    }
    /** @} */

    /** Constraint: address upper bound.
     *
     *  Constrains matched addresses so that they are all less than or equal to @p x.
     *
     * @{ */
    AddressMapConstraints<const AddressMap> atOrBefore(Address x) const {
        return AddressMapConstraints<const AddressMap>(this).atOrBefore(x);
    }
    AddressMapConstraints<AddressMap> atOrBefore(Address x) {
        return AddressMapConstraints<AddressMap>(this).atOrBefore(x);
    }
    /** @} */

    /** Constraint: address lower and upper bounds.
     *
     *  Constrains matched addresses so they are all within the specified interval.
     *
     * @{ */
    AddressMapConstraints<const AddressMap> within(const Sawyer::Container::Interval<Address> &x) const {
        return AddressMapConstraints<const AddressMap>(this).within(x);
    }
    AddressMapConstraints<AddressMap> within(const Sawyer::Container::Interval<Address> &x) {
        return AddressMapConstraints<AddressMap>(this).within(x);
    }
    /** @} */

    /** Constraint: address lower and upper bounds.
     *
     *  Constrains matched addresses so they are all greater than or equal to @p x and less than or equal to @p y.
     *
     * @{ */
    AddressMapConstraints<const AddressMap> within(Address x, Address y) const {
        return AddressMapConstraints<const AddressMap>(this).within(x, y);
    }
    AddressMapConstraints<AddressMap> within(Address x, Address y) {
        return AddressMapConstraints<AddressMap>(this).within(x, y);
    }
    /** @} */

    /** Constraint: address lower and upper bounds.
     *
     *  Specifies lower and upper bounds. The upper bound is specified indirectly by a size.
     *
     * @{ */
    AddressMapConstraints<const AddressMap> baseSize(Address base, Address size) const {
        return AddressMapConstraints<const AddressMap>(this).baseSize(base, size);
    }
    AddressMapConstraints<AddressMap> baseSize(Address base, Address size) {
        return AddressMapConstraints<AddressMap>(this).baseSize(base, size);
    }
    /** @} */

    /** Constraint: address lower bound.
     *
     *  Constrains matched addresses so that they are all greater than @p x.
     *
     * @{ */
    AddressMapConstraints<const AddressMap> after(Address x) const {
        return AddressMapConstraints<const AddressMap>(this).after(x);
    }
    AddressMapConstraints<AddressMap> after(Address x) {
        return AddressMapConstraints<AddressMap>(this).after(x);
    }
    /** @} */

    /** Constraint: address upper bound.
     *
     *  Constrains matched addresses so that they are all less than @p x.
     *
     * @{ */
    AddressMapConstraints<const AddressMap> before(Address x) const {
        return AddressMapConstraints<const AddressMap>(this).before(x);
    }
    AddressMapConstraints<AddressMap> before(Address x) {
        return AddressMapConstraints<AddressMap>(this).before(x);
    }
    /** @} */

    /** Constraint: single segment.
     *
     *  Constrains matched addresses so that they all come from the same segment.
     *
     * @{ */
    AddressMapConstraints<const AddressMap> singleSegment() const {
        return AddressMapConstraints<const AddressMap>(this).singleSegment();
    }
    AddressMapConstraints<AddressMap> singleSegment() {
        return AddressMapConstraints<AddressMap>(this).singleSegment();
    }
    /** @} */

    /** Constraint: arbitrary segment constraint.
     *
     *  Constraints matched addresses to those for which the chain of segment predicates return true.
     *
     * @{ */
    AddressMapConstraints<const AddressMap> segmentPredicate(SegmentPredicate<Address, Value> *p) const {
        return AddressMapConstraints<const AddressMap>(this).segmentPredicate(p);
    }
    AddressMapConstraints<AddressMap> segmentPredicate(SegmentPredicate<Address, Value> *p) {
        return AddressMapConstraints<AddressMap>(this).segmentPredicate(p);
    }
    /** @} */

    /** Constraint: matches anything.
     *
     *  The null constraint matches any mapped address.
     *
     * @{ */
    AddressMapConstraints<const AddressMap> any() const {
        return AddressMapConstraints<const AddressMap>(this);
    }
    AddressMapConstraints<AddressMap> any() {
        return AddressMapConstraints<AddressMap>(this);
    }
    /** @} */

    /** Constraint: matches nothing.
     *
     *  Constrains addresses so that none of them can match.
     *
     * @{ */
    AddressMapConstraints<const AddressMap> none() const {
        return AddressMapConstraints<const AddressMap>(this).none();
    }
    AddressMapConstraints<AddressMap> none() {
        return AddressMapConstraints<AddressMap>(this).none();
    }
    /** @} */

    /** Check map consistency.
     *
     *  Performs the following consistency checks and throws an <code>std::runtime_error</code> if something is wrong.
     *
     *  @li A segment should not have a null buffer pointer.
     *
     *  @li Checks that the buffers of the map are appropriate sizes for the address interval in which they're mapped. */
    void checkConsistency() const {
        BOOST_FOREACH (const Node &node, nodes()) {
            const Sawyer::Container::Interval<Address> &interval = node.key();
            const Segment &segment = node.value();
            if (segment.buffer()==NULL) {
                throw std::runtime_error("AddressMap null buffer for interval [" +
                                         boost::lexical_cast<std::string>(interval.least()) +
                                         "," + boost::lexical_cast<std::string>(interval.greatest()) + "]");
            }
            Address bufAvail = segment.buffer()->available(segment.offset());
            if (bufAvail < interval.size()) {
                throw std::runtime_error("AddressMap segment at [" + boost::lexical_cast<std::string>(interval.least()) +
                                         "," + boost::lexical_cast<std::string>(interval.greatest()) + "] points to only " +
                                         boost::lexical_cast<std::string>(bufAvail) + (1==bufAvail?" value":" values") +
                                         " but the interval size is " + boost::lexical_cast<std::string>(interval.size()));
            }
        }
    }
    
    /** Number of segments contained in the map.
     *
     *  Multiple segments may be pointing to the same underlying buffer, and the number of segments is not necessarily the same
     *  as the net number of segments inserted and erased.  For instance, if a segment is inserted for addresses [0,99] and
     *  then a different segment is inserted at [50,59], the map will contain three segments at addresses [0,49], [50,59], and
     *  [60,99], although the first and third segment point into different parts of the same buffer. */
    Address nSegments() const { return this->nIntervals(); }

    /** Iterator range for all segments.
     *
     *  This is just an alias for the @ref values method defined in the super class.
     *
     *  @{ */
    boost::iterator_range<SegmentIterator> segments() { return this->values(); }
    boost::iterator_range<ConstSegmentIterator> segments() const { return this->values(); }

    /** Segments that overlap with constraints.
     *
     *  Returns an iterator range for the first longest sequence of segments that all at least partly satisfy the specified
     *  constraints.  Constraints are always matched at the address level and the return value consists of those segments that
     *  contain at least one matched address. Constraints normally match contiguous addresses, and therefore the returned list
     *  will be segments that are contiguous. Disabling the contiguous constraint with the @c MATCH_NONCONTIGUOUS flag relaxes
     *  the requirement that addresses be contiguous, although it still enforces that the matched interval contains only
     *  addresses that satisfy the constraints or addresses that are not mapped.
     *
     *  The following example finds the first sequence of one or more segments having "IAT" as a substring in their name and
     *  returns the longest sequence at that position.  The sequence is then used to remove execute permission from each
     *  segment.
     *
     * @code
     *  typedef AddressMap<Address,Value>::Segment Segment;
     *  BOOST_FOREACH (Segment &segment, map.substr("IAT").segments(MATCH_NONCONTIGUOUS))
     *      segment.accessibility(segment.accessibility() & ~EXECUTABLE);
     * @endcode
     *
     * @{ */
    boost::iterator_range<ConstSegmentIterator>
    segments(const AddressMapConstraints<const AddressMap> &c, MatchFlags flags=0) const {
        using namespace AddressMapImpl;
        if (0==(flags & (MATCH_CONTIGUOUS|MATCH_NONCONTIGUOUS)))
            flags |= MATCH_CONTIGUOUS;
        MatchedConstraints<const AddressMap> m = matchConstraints(*this, c, flags);
        return boost::iterator_range<ConstSegmentIterator>(m.nodes_.begin(), m.nodes_.end());
    }

    boost::iterator_range<SegmentIterator>
    segments(const AddressMapConstraints<AddressMap> &c, MatchFlags flags=0) {
        using namespace AddressMapImpl;
        if (0==(flags & (MATCH_CONTIGUOUS|MATCH_NONCONTIGUOUS)))
            flags |= MATCH_CONTIGUOUS;
        MatchedConstraints<AddressMap> m = matchConstraints(*this, c, flags);
        return boost::iterator_range<SegmentIterator>(m.nodes_);
    }
    /** @} */

    /** Iterator range for nodes.
     *
     *  This is just an alias for the @ref nodes method defined in the super class.  See also the overloaded method of the same
     *  name that takes a constraint and thus returns only some nodes.
     *
     * @{ */
    boost::iterator_range<NodeIterator> nodes() { return Super::nodes(); }
    boost::iterator_range<ConstNodeIterator> nodes() const { return Super::nodes(); }
    /** @} */

    /** Nodes that overlap with constraints.
     *
     *  Returns an iterator range for the first longest sequence of interval/segment nodes that all at least partly satisfy the
     *  specified constraints.  Constraints are always matched at the address level and the return value consists of those
     *  nodes that contain at least one matched address. Constraints normally match contiguous addresses, and therefore the
     *  returned list will be nodes that are contiguous. Disabling the contiguous constraint with the @c MATCH_NONCONTIGUOUS
     *  flag relaxes the requirement that addresses be contiguous, although it still enforces that the matched interval
     *  contains only addresses that satisfy the constraints or addresses that are not mapped.
     *
     *  The following example finds the first sequence of one or more segments having addresses between 1000 and 2000 and "IAT"
     *  as part of their name and prints their address interval and name:
     *
     * @code
     *  typedef AddressMap<Address,Value>::Node Node;
     *  BOOST_FOREACH (const Node &node, map.within(1000,2000).substr("IAT").nodes(MATCH_NONCONTIGUOUS))
     *      std::cout <<"segment at " <<node.key() <<" named " <<node.value().name() <<"\n";
     * @endcode
     *
     * @{ */
    boost::iterator_range<ConstNodeIterator>
    nodes(const AddressMapConstraints<const AddressMap> &c, MatchFlags flags=0) const {
        using namespace AddressMapImpl;
        if (0==(flags & (MATCH_CONTIGUOUS|MATCH_NONCONTIGUOUS)))
            flags |= MATCH_CONTIGUOUS;
        MatchedConstraints<const AddressMap> m = matchConstraints(*this, c, flags);
        return m.nodes_;
    }

    boost::iterator_range<NodeIterator>
    nodes(const AddressMapConstraints<AddressMap> &c, MatchFlags flags=0) {
        using namespace AddressMapImpl;
        if (0==(flags & (MATCH_CONTIGUOUS|MATCH_NONCONTIGUOUS)))
            flags |= MATCH_CONTIGUOUS;
        MatchedConstraints<AddressMap> m = matchConstraints(*this, c, flags);
        return m.nodes_;
    }
    /** @} */

    /** Minimum or maximum address that satisfies constraints.
     *
     *  This method returns the minimum or maximum address that satisfies the constraints, depending on whether the
     *  direction is forward or backward.  It is named "next" because it is often used in loops that iterate over addresses.
     *  For instance, the following loop iterates over all readable addresses one at a time (there are more efficient ways to
     *  do this).
     *
     * @code
     *  typedef AddressMap<Address,Value> Map;
     *  Map map = ...;
     *  for (Address a=0; map.atOrAfter(a).require(READABLE).next().assignTo(a); ++a) {
     *      ...
     *      if (a == map.hull().greatest())
     *          break;
     *  }       
     * @endcode
     *
     *  The conditional break at the end of the loop is to handle the case where @c a is the largest possible address, and
     *  incrementing it would result in an overflow back to a smaller address.  The @ref hull method returns in constant time,
     *  but a slightly faster test (that is also more self-documenting) is:
     *
     * @code
     *  if (a == boost::integer_traits<Address>::const_max)
     *      break;
     * @endcode
     *
     *  Backward iterating is similar:
     *
     * @code
     *  typedef AddressMap<Address,Value> Map;
     *  Map map = ...;
     *  for (Address a=map.hull().greatest(); map.atOrBefore(a).require(READABLE).next(MATCH_BACKWARD).assignTo(a); --a) {
     *      ...
     *      if (a == map.hull().least())
     *          break;
     *  }
     * @endcode */
    Optional<Address>
    next(const AddressMapConstraints<const AddressMap> &c, MatchFlags flags=0) const {
        using namespace AddressMapImpl;
        if (0==(flags & (MATCH_CONTIGUOUS|MATCH_NONCONTIGUOUS)))
            flags |= MATCH_CONTIGUOUS;
        MatchedConstraints<const AddressMap> m = matchConstraints(*this, c.limit(1), flags);
        return m.interval_.isEmpty() ? Optional<Address>() : Optional<Address>(m.interval_.least());
    }

    /** Adress interval that satisfies constraints.
     *
     *  Returns the lowest or highest (depending on direction) largest address interval that satisfies the specified
     *  constraints.  The interval can be contiguous (the default), or it may contain unmapped addresses. In any case, all
     *  mapped addresses in the returned interval satisfy the constraints. */
    Sawyer::Container::Interval<Address>
    available(const AddressMapConstraints<const AddressMap> &c, MatchFlags flags=0) const {
        using namespace AddressMapImpl;
        if (0==(flags & (MATCH_CONTIGUOUS|MATCH_NONCONTIGUOUS)))
            flags |= MATCH_CONTIGUOUS;
        return matchConstraints(*this, c, flags).interval_;
    }

    /** Determines if an address exists with the specified constraints.
     *
     *  Checking for existence is just a wrapper around next.  For instance, these two statements both check whether the
     *  address 1000 exists and has execute permission:
     *
     * @code
     *  if (map.at(1000).require(EXECUTABLE).exists()) ...
     *  if (map.at(1000).require(EXECUTABLE).next()) ...
     * @endcode */
    bool exists(const AddressMapConstraints<const AddressMap> &c, MatchFlags flags=0) const {
        return next(c, flags);
    }

    /** Find node containing address.
     *
     *  Finds the node that contains the first (or last, depending on direction) address that satisfies the constraints.
     *
     * @{ */
    ConstNodeIterator findNode(const AddressMapConstraints<const AddressMap> &c, MatchFlags flags=0) const {
        return nodes(c.limit(1), flags).begin();
    }
    NodeIterator findNode(const AddressMapConstraints<AddressMap> &c, MatchFlags flags=0) {
        return nodes(c.limit(1), flags).begin();
    }
    /** @} */

    /** Find unmapped interval.
     *
     *  Searches for the lowest (or highest if direction is @c MATCH_BACKWARD) interval that is not mapped and returns its
     *  address and size.  The returned interval will not contain addresses that are less than (or greater than) than @p
     *  boundary.  If no such unmapped intervals exist then the empty interval is returned.
     *
     *  This method does not use constraints since it searches for addresses that do not exist in the map. */
    Sawyer::Container::Interval<Address>
    unmapped(Address boundary, MatchFlags flags=0) const {
        return (flags & MATCH_BACKWARD) != 0 ? this->lastUnmapped(boundary) : this->firstUnmapped(boundary);
    }

    /** Find free space.
     *
     *  Finds a suitable region of unmapped address space in which @p nValues values can be mapped.  The return value is either
     *  an address where the values can be mapped, or nothing if no such unmapped region is available.  The @p restriction can
     *  be used to restrict which addresses are considered.  The return value will have the specified alignment and will be
     *  either the lowest or highest possible address depending on whether direction is forward or backward.
     *
     *  This method does not use constraints since it searches for addresses that do not exist in the map. */
    Optional<Address>
    findFreeSpace(size_t nValues, size_t alignment=1,
                  Sawyer::Container::Interval<Address> restriction = Sawyer::Container::Interval<Address>::whole(),
                  MatchFlags flags=0) const {
        static const Sawyer::Container::Interval<Address> whole = Sawyer::Container::Interval<Address>::whole();
        ASSERT_forbid2(nValues == 0, "cannot determine if this is an overflow or intentional");

        if (restriction.isEmpty())
            return Nothing();

        if (0 == (flags & MATCH_BACKWARD)) {
            Address minAddr = restriction.least();
            while (minAddr <= restriction.greatest()) {
                Sawyer::Container::Interval<Address> interval = unmapped(minAddr, 0 /*forward*/);
                if (interval.isEmpty())
                    return Nothing();
                minAddr = alignUp(interval.least(), alignment);
                Address maxAddr = minAddr + (nValues-1);
                if ((nValues <= interval.size() || 0==interval.size()/*overflow*/) &&
                    minAddr >= interval.least()/*overflow*/ && maxAddr >= interval.least()/*overflow*/ &&
                    maxAddr <= interval.greatest()) {
                    return minAddr;
                }
                if (interval.greatest() == whole.greatest())
                    return Nothing();                   // to avoid overflow in next statement
                minAddr = interval.greatest() + 1;
            }
            return Nothing();
        }

        ASSERT_require((flags & MATCH_BACKWARD) != 0);
        Address maxAddr = restriction.greatest();
        while (maxAddr >= restriction.least()) {
            Sawyer::Container::Interval<Address> interval = unmapped(maxAddr, MATCH_BACKWARD);
            if (interval.isEmpty())
                return Nothing();
            Address minAddr = alignDown(interval.greatest() - (nValues-1), alignment);
            maxAddr = minAddr + (nValues-1);
            if ((nValues <= interval.size() || 0==interval.size()/*overflow*/) &&
                minAddr >= interval.least()/*overflow*/ && maxAddr >= interval.least()/*overflow*/ &&
                maxAddr <= interval.greatest()) {
                return minAddr;
            }
            if (interval.least() == whole.least())
                return Nothing();                       // to avoid overflow in next statement
            maxAddr = interval.least() - 1;
        }
        return Nothing();
    }

    /** Base class for traversals. */
    class Visitor {
    public:
        virtual ~Visitor() {}
        virtual bool operator()(const AddressMap&, const Sawyer::Container::Interval<Address>&) = 0;
    };

    /** Invoke a function on each address interval.
     *
     *  The functor is invoked with the following arguments: the memory map and an interval.  If the functor returns false then
     *  the traversal is terminated.  To facilitate the use of function-local types for the functor without requiring the use
     *  of explicit template parameters, one may pass a subclass of @ref Visitor as the functor.
     *
     *  This example shows one way to print the names of segments that overlap with a given interval:
     *
     * @code
     *  typedef AddressMap<unsigned, char> Map;
     *  struct: Visitor {
     *      bool operator()(const Map &map, const Interval<unsigned> &interval) {
     *          const Map::Segment &segment = map.at(interval.least()).findNode()->value();
     *          std::cerr <<"segment \"" <<segment.name() <<"\"\n";
     *          return true;
     *      }
     *  } visitor;
     *  Map map = ...;
     *  Interval<unsigned> where = ...;
     *  map.within(where).traverse(visitor);
     * @endcode
     *
     * @{ */
    template<typename Functor>
    void traverse(Functor &functor, const AddressMapConstraints<const AddressMap> &c, MatchFlags flags=0) const {
        using namespace AddressMapImpl;
        MatchedConstraints<const AddressMap> m = matchConstraints(*this, c, flags);
        BOOST_FOREACH (const Node &node, m.nodes_) {
            Sawyer::Container::Interval<Address> part = m.interval_ & node.key();
            if (!functor(*this, part))
                return;
        }
        return;
    }
    template<typename Functor>
    void traverse(Functor &functor, const AddressMapConstraints<AddressMap> &c, MatchFlags flags=0) {
        using namespace AddressMapImpl;
        MatchedConstraints<AddressMap> m = matchConstraints(*this, c, flags);
        BOOST_FOREACH (const Node &node, m.nodes_) {
            Sawyer::Container::Interval<Address> part = m.interval_ & node.key();
            if (!functor(*this, part))
                return;
        }
        return;
    }
    void traverse(Visitor &visitor, const AddressMapConstraints<const AddressMap> &c, MatchFlags flags=0) const {
        traverse<Visitor>(visitor, c, flags);
    }
    void traverse(Visitor &visitor, const AddressMapConstraints<AddressMap> &c, MatchFlags flags=0) {
        traverse<Visitor>(visitor, c, flags);
    }
    /** @} */

    /** Reads data into the supplied buffer.
     *
     *  Reads data into an array or STL vector according to the specified constraints.  If the array is a null pointer then no
     *  data is read or copied and the return value indicates what addresses would have been accessed. When the buffer is an
     *  STL vector the constraints are augmented by also limiting the number of items accessed; the caller must do that
     *  explicitly for arrays. The return value is the interval of addresses that were read.
     *
     *  The constraints are usually curried before the actual read call, as in this example that reads up to 10 values starting
     *  at some address and returns the number of values read:
     *
     * @code
     *  Value buf[10];
     *  size_t nRead = map.at(start).limit(10).read(buf).size();
     * @endcode
     *
     *  The following loop reads and prints all the readable values from a memory map using a large buffer for efficiency:
     *
     * @code
     *  std::vector<Value> buf(1024);
     *  while (Interval<Address> accessed = map.atOrAfter(a).read(buf)) {
     *      a = accessed.least();
     *      BOOST_FOREACH (const Value &v, buf)
     *          std::cout <<a++ <<": " <<v <<"\n";
     *      if (accessed.greatest()==map.hull().greatest())
     *          break; // to handle case when a++ overflowed
     *  }
     * @endcode
     *
     *  Reading can also be performed backward, such as this example that reads up to ten values such that the last value read
     *  is at address 999.  The buffer will always contain results in address order, with the first element of the buffer being
     *  the value that was read with the lowest address.
     *
     * @code
     *  Value buf[10];
     *  size_t nRead = map.at(999).limit(10).read(buf, MATCH_BACKWARD).size();
     * @endcode
     *
     * @{ */
    Sawyer::Container::Interval<Address>
    read(Value *buf /*out*/, const AddressMapConstraints<const AddressMap> &c, MatchFlags flags=0) const {
        using namespace AddressMapImpl;
        ASSERT_require2(0 == (flags & MATCH_NONCONTIGUOUS), "only contiguous addresses can be read");
        if (0==(flags & (MATCH_CONTIGUOUS|MATCH_NONCONTIGUOUS)))
            flags |= MATCH_CONTIGUOUS;
        MatchedConstraints<const AddressMap> m = matchConstraints(*this, c, flags);
        if (buf) {
            BOOST_FOREACH (const Node &node, m.nodes_) {
                Sawyer::Container::Interval<Address> part = m.interval_ & node.key(); // part of segment to read
                ASSERT_forbid(part.isEmpty());
                Address bufferOffset = part.least() - node.key().least() + node.value().offset();
                Address nValues = node.value().buffer()->read(buf, bufferOffset, part.size());
                if (nValues != part.size()) {
                    checkConsistency();
                    ASSERT_not_reachable("something is wrong with the memory map");
                }
                buf += nValues;
            }
        }
        return m.interval_;
    }

    Sawyer::Container::Interval<Address>
    read(std::vector<Value> &buf /*out*/, const AddressMapConstraints<const AddressMap> &c, MatchFlags flags=0) const {
        return buf.empty() ? Sawyer::Container::Interval<Address>() : read(&buf[0], c.limit(buf.size()), flags);
    }
    /** @} */
    
    /** Writes data from the supplied buffer.
     *
     *  Copies data from an array or STL vector into the underlying address map buffers corresponding to the specified
     *  constraints.  If the array is a null pointer then no data is written and the return value indicates what addresses
     *  would have been accessed.  The constraints are agumented by also requiring that the addresses be contiguous
     *  and lack the IMMUTABLE bit, and in the case of STL vectors that not more data is written thn what is in the vector.
     *  The return value is the interval of addresses that were written.
     *
     *  The Access::IMMUTABLE bit is usually used to indicate that a buffer cannot be modified (for instance, the buffer is
     *  memory allocated with read-only access by POSIX @c mmap).
     *
     *  The constraints are usually curried before the actual read call, as in this example that writes the vector's values
     *  into the map at the first writable address greater than or equal to 1000.
     *
     * @code
     *  std::vector<Value> buffer = {...};
     *  Interval<Address> written = map.atOrAfter(1000).require(WRITABLE).write(buffer);
     * @endcode
     *
     *  Writing can also be performed backward, such as this example that writes up to ten values such that the last value
     *  written is at address 999.  The buffer contains values in their address order.
     *
     * @code
     *  Value buf[10] = { ... };
     *  size_t nWritten = map.at(999).limit(10).write(buf, MATCH_BACKWARD).size();
     * @endcode
     *
     * @todo FIXME[Robb Matzke 2014-09-01]: The order of values in the buffer being written by AddressMap::write when writing
     * in a backward direction is not all that useful. Perhaps the write should consume values from the end of the buffer
     * instead of the beginning.
     *
     * @{ */
    Sawyer::Container::Interval<Address>
    write(const Value *buf, const AddressMapConstraints<AddressMap> &c, MatchFlags flags=0) {
        using namespace AddressMapImpl;
        ASSERT_require2(0 == (flags & MATCH_NONCONTIGUOUS), "only contiguous addresses can be written");
        if (0==(flags & (MATCH_CONTIGUOUS|MATCH_NONCONTIGUOUS)))
            flags |= MATCH_CONTIGUOUS;
        MatchedConstraints<AddressMap> m = matchConstraints(*this, c.prohibit(Access::IMMUTABLE), flags);
        if (buf) {
            BOOST_FOREACH (Node &node, m.nodes_) {
                Segment &segment = node.value();
                Sawyer::Container::Interval<Address> part = m.interval_ & node.key(); // part of segment to write
                ASSERT_forbid(part.isEmpty());
                typename Buffer::Ptr buffer = segment.buffer();
                ASSERT_not_null(buffer);

                if (buffer->copyOnWrite()) {
                    typename Buffer::Ptr newBuffer = buffer->copy(); // copyOnWrite is cleared in newBuffer
                    ASSERT_not_null(newBuffer);
                    for (NodeIterator iter=this->lowerBound(node.key().least()); iter!=nodes().end(); ++iter) {
                        if (iter->value().buffer() == buffer)
                            iter->value().buffer(newBuffer);
                    }
                    buffer = newBuffer;
                }

                Address bufferOffset = part.least() - node.key().least() + segment.offset();
                Address nValues = buffer->write(buf, bufferOffset, part.size());
                if (nValues != part.size()) {
                    checkConsistency();
                    ASSERT_not_reachable("something is wrong with the memory map");
                }
                buf += nValues;
            }
        }
        return m.interval_;
    }

    Sawyer::Container::Interval<Address>
    write(const std::vector<Value> &buf, const AddressMapConstraints<AddressMap> &c, MatchFlags flags=0) {
        return buf.empty() ? Sawyer::Container::Interval<Address>() : write(&buf[0], c.limit(buf.size()), flags);
    }
    /** @} */

    /** Prune away addresses that match constraints.
     *
     *  Removes all addresses for which the constraints match. The addresses need not be contiguous in memory (in fact,
     *  noncontiguous is the default), and the matching segments need not be consecutive segments.  In other words, the
     *  interval over which this function operates can include segments that do not satisfy the constraints (and are not
     *  pruned).  For instance, to remove all segments that are writable regardless of whether other segments are interspersed:
     *
     * @code
     *  map.require(WRITABLE).contiguous(false).prune();
     * @endcode
     *
     * @sa keep */
    void prune(const AddressMapConstraints<AddressMap> &c, MatchFlags flags=0) {
        using namespace AddressMapImpl;
        IntervalSet<Sawyer::Container::Interval<Address> > toErase;
        if (0==(flags & (MATCH_CONTIGUOUS|MATCH_NONCONTIGUOUS)))
            flags |= MATCH_NONCONTIGUOUS;
        MatchedConstraints<AddressMap> m = matchConstraints(*this, c.addressConstraints(), flags);
        BOOST_FOREACH (const Node &node, m.nodes_) {
            if (isSatisfied(node, c))
                toErase.insert(node.key() & m.interval_);
        }
        BOOST_FOREACH (const Sawyer::Container::Interval<Address> &interval, toErase.intervals())
            this->erase(interval);
    }

    /** Keep only addresses that match constraints.
     *
     *  Keeps only those addresses that satisfy the given constraints, discarding all others.  The addresses need not be
     *  contiguous (in fact, noncontiguous is the default), and the matching segments need not be consecutive segments.  In
     *  other words, the interval over which this function operates can include segments that do not satisfy the constraints
     *  (and are pruned). For instance, to remove all segments that are not writable regardless of whether other segments are
     *  interspersed:
     *
     * @code
     *  map.require(WRITABLE).contiguous(false).keep();
     * @endcode
     *
     * @sa prune */
    void keep(const AddressMapConstraints<AddressMap> &c, MatchFlags flags=0) {
        using namespace AddressMapImpl;
        if (0==(flags & (MATCH_CONTIGUOUS|MATCH_NONCONTIGUOUS)))
            flags |= MATCH_NONCONTIGUOUS;
        IntervalSet<Sawyer::Container::Interval<Address> > toKeep;
        MatchedConstraints<AddressMap> m = matchConstraints(*this, c.addressConstraints(), flags);
        BOOST_FOREACH (const Node &node, m.nodes_) {
            if (isSatisfied(node, c))
                toKeep.insert(node.key() & m.interval_);
        }
        toKeep.invert();
        BOOST_FOREACH (const Sawyer::Container::Interval<Address> &interval, toKeep.intervals())
            this->erase(interval);
    }

    /** Change access bits for addresses that match constraints.
     *
     *  For all addresses that satisfy the specified constraints, add the @p requiredAccess and remove the @p prohibitedAccess
     *  bits.  The addresses need not be contiguous (in fact, noncontiguous is the default), and the matching segments need not
     *  be consecutive segments. In other words, the interval over which this function operates can include addresses that do
     *  not satisfy the constraints and whose access bits are not modified.  For instance, to add execute permission and remove
     *  write permission for all segments containing the string ".text":
     *
     * @code
     *  map.substr(".text").changeAccess(EXECUTABLE, WRITABLE);
     * @endcode
     *
     *  To set access bits to a specific value, supply the complement as the second argument.  The following code changes all
     *  addresses between a specified range so that only the READABLE and WRITABLE bits are set and no others:
     *
     * @code
     *  unsigned newAccess = READABLE | WRITABLE;
     *  map.within(100,200).changeAccess(newAccess, ~newAccess);
     * @endcode */
    void changeAccess(unsigned requiredAccess, unsigned prohibitedAccess, const AddressMapConstraints<AddressMap> &c,
                      MatchFlags flags=0) {
        using namespace AddressMapImpl;
        if (0==(flags & (MATCH_CONTIGUOUS|MATCH_NONCONTIGUOUS)))
            flags |= MATCH_NONCONTIGUOUS;
        typedef std::pair<Sawyer::Container::Interval<Address>, Segment> ISPair;
        std::vector<ISPair> newSegments;
        MatchedConstraints<AddressMap> m = matchConstraints(*this, c.addressConstraints(), flags);
        BOOST_FOREACH (Node &node, m.nodes_) {
            Segment &segment = node.value();
            if (isSatisfied(node, c)) {
                unsigned newAccess = (segment.accessibility() | requiredAccess) & ~prohibitedAccess;
                Sawyer::Container::Interval<Address> toChange = node.key() & m.interval_;
                if (toChange == node.key()) {           // all addresses in segment are selected; change segment in place
                    segment.accessibility(newAccess);
                } else {                                // insert a new segment, replacing part of the existing one
                    Segment newSegment(segment);
                    newSegment.accessibility(newAccess);
                    newSegment.offset(segment.offset() + toChange.least() - node.key().least());
                    newSegments.push_back(ISPair(toChange, newSegment));
                }
            }
        }
        BOOST_FOREACH (const ISPair &pair, newSegments)
            this->insert(pair.first, pair.second);
    }
    
private:
    // Increment x if necessary so it is aligned.
    static Address alignUp(Address x, Address alignment) {
        return alignment>0 && x%alignment!=0 ? ((x+alignment-1)/alignment)*alignment : x;
    }

    static Address alignDown(Address x, Address alignment) {
        return alignment>0 && x%alignment!=0 ? (x/alignment)*alignment : x;
    }
};

} // namespace
} // namespace

#endif
