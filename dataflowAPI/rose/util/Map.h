// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




#ifndef Sawyer_Map_H
#define Sawyer_Map_H

#include "Interval.h"
#include "Optional.h"
#include "Sawyer.h"
#include <memory>
#include <stddef.h>
#include <utility>
#include <boost/range/iterator_range.hpp>
#include <map>
#include <stdexcept>

namespace Sawyer {

/** %Container classes that store user-defined values.
 *
 *  This library implements a number of container classes.  In general, these classes adhere to the naming scheme used
 *  throughout the library, which might be somewhat surprising compared with STL containers.  The major differences are:
 *
 *  @li All type names are CamelCase with an initial capital letter, such as <code>Iterator</code>, <code>ConstIterator</code>,
 *      etc.
 *  @li Some containers have multiple categories of iterator, such as iterators that visit the storage nodes of the container,
 *      and iterators that visit the values logically contained in the container.  For instance one can iterate over the keys
 *      alone in a key/value map, or the values alone, or the combined storage nodes that have a key/value pair.
 *  @li The containers have no direct <code>begin</code> and <code>end</code> methods, but rather return iterator ranges with
 *      methods like, for the key/value map container, <code>keys</code>, <code>values</code>, and <code>nodes</code>.  This
 *      allows containers return different types of iterators.  The iterator ranges have the usual <code>begin</code> and
 *      <code>end</code> methods.
 *  @li Most predicates start with the word "is", such as <code>isEmpty</code> so they cannot be confused with a similar verb
 *      (i.e., it is quite obvious that <code>isEmpty</code> does not empty the container).
 *  @li The <code>insert</code> methods come in different flavors depending on what's being inserted.  Inserters that insert
 *      a value are named <code>insert</code>; those that insert multiple values, like from another container or iterator
 *      range, are named <code>insertMultiple</code>.  This gives the API more flexibility and consistency across different
 *      containers.
 *  @li The <code>erase</code> methods also come in different flavors: <code>erase</code> is to erase a single specified value,
 *      <code>eraseAt</code> is for erasing based on location (i.e., iterators that point into that container), and
 *      <code>eraseMultiple</code> is to erase multiple values at once (e.g., values obtained from a different container).
 *      As with <code>insert</code>, this gives container designers more flexibility and at the same time improves
 *      consistency. */
namespace Container {

/** %Container associating values with keys.
 *
 *  This container is similar to the <code>std::map</code> container in the standard template library, but with these
 *  differences in addition to those described in the documentation for the Sawyer::Container name space:
 *
 *  @li It extends the interface with additional methods that return optional values (@ref Optional) and a few
 *      convenience methods (like @ref exists).
 *  @li It provides iterators over keys and values in addition to the STL-like iterator over nodes.
 *  @li The insert methods always insert the specified value(s) regardless of whether a node with the same key already
 *      existed (i.e., they do what their name says they do). If you need STL behavior, then use the <code>insertMaybe</code>
 *      methods.  */
template<class K,
         class T,
         class Cmp = std::less<K>,
         class Alloc = std::allocator<std::pair<const K, T> > >
class Map {
public:
    typedef K Key;                                      /**< Type for keys. */
    typedef T Value;                                    /**< Type for values associated with each key. */
    typedef Cmp Comparator;                             /**< Type of comparator, third template argument. */
    typedef Alloc Allocator;                            /**< Type of allocator, fourth template argument. */

private:
    typedef std::map<Key, Value, Comparator, Alloc> StlMap;
    StlMap map_;

public:
    /** Type for stored nodes.
     *
     *  A storage node contains the immutable key and its associated value. */
    class Node: private std::pair<const Key, Value> {
        // This class MUST BE binary compatible with its super class (see NodeIterator::operator* below)
    public:
        explicit Node(const std::pair<const Key, Value> &pair): std::pair<const Key, Value>(pair) {}
        Node(const Key &key, Value &value): std::pair<const Key, Value>(key, value) {}
    public:
        /** Key part of key/value node.
         *
         *  Returns the key part of a key/value node. Keys are not mutable when they are part of a map. */
        const Key& key() const { return this->first; }

        /** Value part of key/value node.
         *
         *  Returns a reference to the value part of a key/value node.
         *
         * @{ */
        Value& value() { return this->second; }
        const Value& value() const { return this->second; }
        /** @} */
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Iterators
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
    template<class Derived, class Value, class BaseIterator>
    class BidirectionalIterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = Value;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;
    protected:
        BaseIterator base_;
        BidirectionalIterator() {}
        BidirectionalIterator(const BaseIterator &base): base_(base) {}
    public:
        Derived& operator=(const Derived &other) { base_ = other.base_; return *derived(); }
        Derived& operator++() { ++base_; return *derived(); }
        Derived operator++(int) { Derived old=*derived(); ++*this; return old; }
        Derived& operator--() { --base_; return *derived(); }
        Derived operator--(int) { Derived old=*derived(); --*this; return old; }
        template<class OtherIter> bool operator==(const OtherIter &other) const { return base_ == other.base(); }
        template<class OtherIter> bool operator!=(const OtherIter &other) const { return base_ != other.base(); }
        const BaseIterator& base() const { return base_; }
    protected:
        Derived* derived() { return static_cast<Derived*>(this); }
        const Derived* derived() const { return static_cast<const Derived*>(this); }
    };

public:
    /** Bidirectional iterator over key/value nodes.
     *
     *  Dereferencing this iterator will return a Node from which both the key and the value can be obtained. Node iterators
     *  are implicitly convertible to both key and value iterators. */
    class NodeIterator: public BidirectionalIterator<NodeIterator, Node, typename StlMap::iterator> {
        typedef                BidirectionalIterator<NodeIterator, Node, typename StlMap::iterator> Super;
    public:
        NodeIterator() {}
        // std::map stores std::pair nodes, but we want to return Node, which must have the same layout.
        Node& operator*() const { return *(Node*)&*this->base_; }
        Node* operator->() const { return (Node*)&*this->base_; }
    private:
        friend class Map;
        NodeIterator(const typename StlMap::iterator &base): Super(base) {}
    };

    /** Bidirectional iterator over key/value nodes.
     *
     *  Dereferencing this iterator will return a Node from which both the key and the value can be obtained. Node iterators
     *  are implicitly convertible to both key and value iterators. */
    class ConstNodeIterator: public BidirectionalIterator<ConstNodeIterator, const Node, typename StlMap::const_iterator> {
        typedef                     BidirectionalIterator<ConstNodeIterator, const Node, typename StlMap::const_iterator> Super;
    public:
        ConstNodeIterator() {}
        ConstNodeIterator(const ConstNodeIterator &other): Super(other) {}
        ConstNodeIterator(const NodeIterator &other): Super(typename StlMap::const_iterator(other.base())) {}
        // std::map stores std::pair nodes, but we want to return Node, which must have the same layout.
        const Node& operator*() const { return *(const Node*)&*this->base_; }
        const Node* operator->() const { return (const Node*)&*this->base_; }
    private:
        friend class Map;
        ConstNodeIterator(const typename StlMap::const_iterator &base): Super(base) {}
        ConstNodeIterator(const typename StlMap::iterator &base): Super(typename StlMap::const_iterator(base)) {}
    };

    /** Bidirectional iterator over keys.
     *
     *  Dereferencing this iterator will return a reference to a const key.  Keys cannot be altered while they are a member of
     *  this container. */
    class ConstKeyIterator: public BidirectionalIterator<ConstKeyIterator, const Key, typename StlMap::const_iterator> {
        typedef                    BidirectionalIterator<ConstKeyIterator, const Key, typename StlMap::const_iterator> Super;
    public:
        ConstKeyIterator() {}
        ConstKeyIterator(const ConstKeyIterator &other): Super(other) {}
        ConstKeyIterator(const NodeIterator &other): Super(typename StlMap::const_iterator(other.base())) {}
        ConstKeyIterator(const ConstNodeIterator &other): Super(other.base()) {}
        const Key& operator*() const { return this->base()->first; }
        const Key* operator->() const { return &this->base()->first; }
    };

    /** Bidirectional iterator over values.
     *
     *  Dereferencing this iterator will return a reference to the user-defined value of the node.  Values may be altered
     *  in-place while they are members of a container. */
    class ValueIterator: public BidirectionalIterator<ValueIterator, Value, typename StlMap::iterator> {
        typedef                 BidirectionalIterator<ValueIterator, Value, typename StlMap::iterator> Super;
    public:
        ValueIterator() {}
        ValueIterator(const ValueIterator &other): Super(other) {}
        ValueIterator(const NodeIterator &other): Super(other.base()) {}
        Value& operator*() const { return this->base()->second; }
        Value* operator->() const { return &this->base()->second; }
    };

    /** Bidirectional iterator over values.
     *
     *  Dereferencing this iterator will return a reference to the user-defined value of the node.  Values may be altered
     *  in-place while they are members of a container. */
    class ConstValueIterator: public BidirectionalIterator<ConstValueIterator, const Value, typename StlMap::const_iterator> {
        typedef BidirectionalIterator<ConstValueIterator, const Value, typename StlMap::const_iterator> Super;
    public:
        ConstValueIterator() {}
        ConstValueIterator(const ConstValueIterator &other): Super(other) {}
        ConstValueIterator(const ValueIterator &other): Super(typename StlMap::const_iterator(other.base())) {}
        ConstValueIterator(const ConstNodeIterator &other): Super(other.base()) {}
        ConstValueIterator(const NodeIterator &other): Super(typename StlMap::const_iterator(other.base())) {}
        const Value& operator*() const { return this->base()->second; }
        const Value* operator->() const { return &this->base()->second; }
    };


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Constructors
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:

    /** Default constructor.
     *
     *  Creates an empty map. */
    Map() {}

    /** Constructs an empty map.
     *
     *  This is like the default constructor, but a comparer and allocator can be specified. */
    explicit Map(const Comparator &comparator, const Allocator &allocator = Allocator())
        : map_(comparator, allocator) {}

    /** Copy constructor. */
    Map(const Map& other) {
        map_ = other.map_;
    }

    /** Copy constructor.
     *
     *  Initializes the new map with copies of the nodes of the @p other map.  The keys and values must be convertible from the
     *  other map to this map. */
    template<class Key2, class T2, class Cmp2, class Alloc2>
    Map(const Map<Key2, T2, Cmp2, Alloc2> &other) {
        typedef typename Map<Key2, T2, Cmp2, Alloc2>::ConstNodeIterator OtherIterator;
        boost::iterator_range<OtherIterator> otherNodes = other.nodes();
        for (OtherIterator otherIter=otherNodes.begin(); otherIter!=otherNodes.end(); ++otherIter)
            map_.insert(map_.end(), std::make_pair(Key(otherIter->key()), Value(otherIter->value())));
    }

    /** Make this map be a copy of another map.
     *
     *  The keys and values of the @p other map must be convertible to the types used for this map. */
    template<class Key2, class T2, class Cmp2, class Alloc2>
    Map& operator=(const Map<Key2, T2, Cmp2, Alloc2> &other) {
        typedef typename Map<Key2, T2, Cmp2, Alloc2>::ConstNodeIterator OtherIterator;
        clear();
        boost::iterator_range<OtherIterator> otherNodes = other.nodes();
        for (OtherIterator otherIter=otherNodes.begin(); otherIter!=otherNodes.end(); ++otherIter)
            map_.insert(map_.end(), std::make_pair(Key(otherIter->key()), Value(otherIter->value())));
    }
        

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Iteration
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Iterators for container nodes.
     *
     *  This returns a range of node-iterators that will traverse all nodes (key/value pairs) of this container.
     *
     * @{ */
    boost::iterator_range<NodeIterator> nodes() {
        return boost::iterator_range<NodeIterator>(NodeIterator(map_.begin()), NodeIterator(map_.end()));
    }
    boost::iterator_range<ConstNodeIterator> nodes() const {
        return boost::iterator_range<ConstNodeIterator>(ConstNodeIterator(map_.begin()), ConstNodeIterator(map_.end()));
    }
    /** @} */

    /** Iterators for container keys.
     *
     *  Returns a range of key-iterators that will traverse the keys of this container.
     *
     * @{ */
    boost::iterator_range<ConstKeyIterator> keys() {
        return boost::iterator_range<ConstKeyIterator>(NodeIterator(map_.begin()), NodeIterator(map_.end()));
    }
    boost::iterator_range<ConstKeyIterator> keys() const {
        return boost::iterator_range<ConstKeyIterator>(ConstNodeIterator(map_.begin()), ConstNodeIterator(map_.end()));
    }
    /** @} */

    /** Iterators for container values.
     *
     *  Returns a range of iterators that will traverse the user-defined values of this container.  The values are iterated in
     *  key order, although the keys are not directly available via these iterators.
     *
     * @{ */
    boost::iterator_range<ValueIterator> values() {
        return boost::iterator_range<ValueIterator>(NodeIterator(map_.begin()), NodeIterator(map_.end()));
    }
    boost::iterator_range<ConstValueIterator> values() const {
        return boost::iterator_range<ConstValueIterator>(ConstNodeIterator(map_.begin()), ConstNodeIterator(map_.end()));
    }
    /** @} */


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Size and capacity
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:

    /** Determines whether this container is empty.
     *
     *  Returns true if the container is empty, and false if it has at least one node. This method executes in constant time. */
    bool isEmpty() const {
        return map_.empty();
    }

    /** Number of nodes, keys, or values in this container.
     *
     *  Returns the number of nodes (elements) in this container.  This method executes in constant time. */
    size_t size() const {
        return map_.size();
    }

    /** Returns the minimum key.  The map must not be empty. */
    Key least() const {
        ASSERT_forbid(isEmpty());
        return map_.begin()->first;
    }

    /** Returns the maximum key.  The map must not be empty. */
    Key greatest() const {
        ASSERT_forbid(isEmpty());
        typename StlMap::const_iterator last = map_.end();
        --last;
        return last->first;
    }

    /** Returns the range of keys in this map.
     *
     *  The return value is an interval containing the least and greatest keys, inclusive.  If the map is empty then an empty
     *  interval is returned.
     *
     *  This method is only defined when the map key type is appropriate for the Interval class template (such as when the keys
     *  are an integer type). */
    Interval<Key> hull() const {
        return isEmpty() ? Interval<Key>() : Interval<Key>::hull(least(), greatest());
    }

    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Searching
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:

    /** Find a node by key.
     *
     *  Looks for a node whose key is equal to the specified @p key and returns an iterator to that node, or the end
     *  iterator if no such node exists.  Two keys are considered equal if this container's Comparator object returns false
     *  reflexively. The method executes logorithmic time based on the number of nodes in the container.
     *
     * @{ */
    NodeIterator find(const Key &key) {
        return map_.find(key);
    }
    ConstNodeIterator find(const Key &key) const {
        return map_.find(key);
    }
    /** @} */

    /** Determine if a key exists.
     *
     *  Looks for a node whose key is equal to the specified @p key and returns true if found, or false if no such node exists.
     *  Two keys are considered equal if this container's @ref Comparator object returns false reflexively. The method executes
     *  in logorithmic time based on the number of nodes in the container. */
    bool exists(const Key &key) const {
        return map_.find(key)!=map_.end();
    }

    /** Find a node close to a key.
     *
     *  Finds the first node whose key is equal to or larger than the specified key and returns an iterator to that node. If no
     *  such node exists, then the end iterator is returned.  The internal @ref Comparator object is used to make the
     *  comparison. This method executes in logorithmic time based on the number of nodes in the container.
     *
     *  @sa upperBound
     *
     * @{ */
    NodeIterator lowerBound(const Key &key) {
        return map_.lower_bound(key);
    }
    ConstNodeIterator lowerBound(const Key &key) const {
        return map_.lower_bound(key);
    }
    /** @} */

    /** Find a node close to a key.
     *
     *  Finds the first node whose key is larger than the specified key and returns an iterator to that node. If no such node
     *  exists, then the end iterator is returned.  The internal @ref Comparator object is used to make the
     *  comparison. This method executes in logorithmic time based on the number of nodes in the container.
     *
     *  @sa lowerBound
     *
     * @{ */
    NodeIterator upperBound(const Key &key) {
        return map_.upper_bound(key);
    }
    ConstNodeIterator upperBound(const Key &key) const {
        return map_.upper_bound(key);
    }
    /** @} */


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Accessors
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:

    /** Return a reference to an existing value.
     *
     *  Returns a reference to the value at the node with the specified @p key.  Unlike <code>std::map</code>, this container
     *  does not instantiate a new key/value pair if the @p key is not in the map's domain.  In other words, the array operator
     *  for this class is more like an array operator on arrays or vectors--such objects are not automatically extended if
     *  dereferenced with an operand that is outside the domain.
     *
     *  If the @p key is not part of this map's domain then an <code>std:domain_error</code> is thrown.
     *
     *  @sa insert insertDefault
     *
     *  @{ */
    Value& operator[](const Key &key) {
        return get(key);
    }
    const Value& operator[](const Key &key) const {
        return get(key);
    }
    /** @} */

    /** Lookup and return an existing value.
     *
     *  Returns a reference to the value at the node with the specified @p key, which must exist. If the @p key is not part of
     *  this map's domain then an <code>std:domain_error</code> is thrown.
     *
     *  @sa insert insertDefault
     *
     *  @{ */
    Value& get(const Key &key) {
        typename StlMap::iterator found = map_.find(key);
        if (found==map_.end())
            throw std::domain_error("key lookup failure; key is not in map domain");
        return found->second;
    }
    const Value& get(const Key &key) const {
        typename StlMap::const_iterator found = map_.find(key);
        if (found==map_.end())
            throw std::range_error("key lookup failure; key is not in map domain");
        return found->second;
    }
    /** @} */

    /** Lookup and return a value or nothing.
     *
     *  Looks up the node with the specified key and returns either a copy of its value, or nothing. This method executes in
     *  logorithmic time.
     *
     *  Here's an example of one convenient way to use this:
     *
     * @code
     *  Map<std::string, FileInfo> files;
     *  ...
     *  if (Optional<FileInfo> fileInfo = files.getOptional(fileName))
     *      std::cout <<"file info for \"" <<fileName <<"\" is " <<*fileInfo <<"\n";
     * @endcode
     *
     *  The equivalent STL approach is:
     *
     * @code
     *  std::map<std::string, FileInfo> files;
     *  ...
     *  std::map<std::string, FileInfo>::const_iterator filesIter = files.find(fileName);
     *  if (fileIter != files.end())
     *      std::cout <<"file info for \"" <<fileName <<"\" is " <<filesIter->second <<"\n";
     * @endcode */
    Optional<Value> getOptional(const Key &key) const {
        typename StlMap::const_iterator found = map_.find(key);
        return found == map_.end() ? Optional<Value>() : Optional<Value>(found->second);
    }

    /** Lookup and return a value or something else.
     *
     *  This is similar to the @ref get method, except a default can be provided.  If a node with the specified @p key is
     *  present in this container, then a reference to that node's value is returned, otherwise the (reference to) supplied
     *  default is returned.
     *
     * @{ */
    Value& getOrElse(const Key &key, Value &dflt) {
        typename StlMap::iterator found = map_.find(key);
        return found == map_.end() ? dflt : found->second;
    }
    const Value& getOrElse(const Key &key, const Value &dflt) const {
        typename StlMap::const_iterator found = map_.find(key);
        return found == map_.end() ? dflt : found->second;
    }
    /** @} */


    /** Lookup and return a value or a default.
     *
     *  This is similar to the @ref getOrElse method except when the key is not present in the map, a reference to a const,
     *  default-constructed value is returned. */
    const Value& getOrDefault(const Key &key) const {
        static const Value dflt;
        typename StlMap::const_iterator found = map_.find(key);
        return found==map_.end() ? dflt : found->second;
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Mutators
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:

    /** Insert or update a key/value pair.
     *
     *  Inserts the key/value pair into the container. If a previous node already had the same key then it is replaced by the
     *  new node.  This method executes in logorithmic time.
     *
     *  @sa insertDefault insertMaybe insertMaybeDefault insertMultiple insertMaybeMultiple */
    Map& insert(const Key &key, const Value &value) {
        std::pair<typename StlMap::iterator, bool> inserted = map_.insert(std::make_pair(key, value));
        if (!inserted.second)
            inserted.first->second = value;
        return *this;
    }

    /** Insert or update a key with a default value.
     *
     *  The value associated with @p key in the map is replaced with a default-constructed value.  If the key does not exist
     *  then it is inserted with a default value.  This operation is similar to the array operator of <code>std::map</code>.
     *
     *  @sa insert insertMaybe insertMaybeDefault insertMultiple insertMaybeMultiple */
    Map& insertDefault(const Key &key) {
        map_[key] = T();
        return *this;
    }
    
    /** Insert multiple values.
     *  
     *  Inserts copies of the nodes in the specified node iterator range. The iterators must iterate over objects that have
     *  <code>key</code> and <code>value</code> methods that return keys and values that are convertible to the types used by
     *  this container.
     *
     *  The normal way to insert the contents of one map into another is:
     *
     * @code
     *  Map<...> source = ...;
     *  Map<...> destination = ...;
     *  destination.insertMultiple(source.nodes());
     * @endcode
     *
     * @sa insert insertDefault insertMaybe insertMaybeDefault insertMaybeMultiple
     *
     * @{ */
    template<class OtherNodeIterator>
    Map& insertMultiple(const OtherNodeIterator &begin, const OtherNodeIterator &end) {
        for (OtherNodeIterator otherIter=begin; otherIter!=end; ++otherIter)
            insert(Key(otherIter->key()), Value(otherIter->value()));
        return *this;
    }
    template<class OtherNodeIterator>
    Map& insertMultiple(const boost::iterator_range<OtherNodeIterator> &range) {
        return insertMultiple(range.begin(), range.end());
    }
    /** @} */
        
    /** Conditionally insert a new key/value pair.
     *
     *  Inserts the key/value pair into the container if the container does not yet have a node with the same key.  This method
     *  executes in logarithmic time.  The return value is a reference to the value that is in the container, either the value
     *  that previously existed or a copy of the specified @p value.
     *
     *  @sa insert insertDefault insertMaybeDefault insertMultiple insertMaybeMultiple */
    Value& insertMaybe(const Key &key, const Value &value) {
        return map_.insert(std::make_pair(key, value)).first->second;
    }

    /** Conditionally insert a new key with default value.
     *
     *  Inserts a key/value pair into the container if the container does not yet have a node with the same key. The value is
     *  default-constructed.  This method executes in logarithmic time.  The return value is a reference to the value that is
     *  in the container, either the value that previously existed or the new default-constructed value.
     *
     *  @sa insert insertDefault insertMultiple insertMaybeMultiple */
    Value& insertMaybeDefault(const Key &key) {
        return map_.insert(std::make_pair(key, T())).first->second;
    }

    /** Conditionally insert multiple key/value pairs.
     *
     *  Inserts each of the specified key/value pairs into this container where this container does not already contain a value
     *  for the key.  The return value is a reference to the container itself so that this method can be chained with others.
     *
     *  @sa insert insertDefault insertMaybe insertMaybeDefault insertMultiple */
    template<class OtherNodeIterator>
    Map& insertMaybeMultiple(const boost::iterator_range<OtherNodeIterator> &range) {
        for (OtherNodeIterator otherIter=range.begin(); otherIter!=range.end(); ++otherIter)
            insert(Key(otherIter->key()), Value(otherIter->value()));
        return *this;
    }

    /** Remove all nodes.
     *
     *  All nodes are removed from this container. This method executes in linear time in the number of nodes in this
     *  container. */
    Map& clear() {
        map_.clear();
        return *this;
    }

    /** Remove a node with specified key.
     *
     *  Removes the node whose key is equal to the specified key, or does nothing if no such node exists.  Two keys are
     *  considered equal if this container's @ref Comparator object returns false reflexively. The method executes in
     *  logorithmic time based on the number of nodes in the container.
     *
     *  @sa eraseMultiple eraseAt eraseAtMultiple */
    Map& erase(const Key &key) {
        map_.erase(key);
        return *this;
    }

    /** Remove keys stored in another Map.
     *
     *  All nodes of this container whose keys are equal to any key in the @p other container are removed from this container.
     *  The keys of the other container must be convertible to the types used by this container, and two keys are considered
     *  equal if this container's @ref Comparator object returns false reflexively. The method executes in <em>N log(N+M)</em>
     *  time.
     *
     *  @sa erase eraseAt eraseAtMultiple */
    template<class OtherKeyIterator>
    Map& eraseMultiple(const boost::iterator_range<OtherKeyIterator> &range) {
        for (OtherKeyIterator otherIter=range.begin(); otherIter!=range.end(); ++otherIter)
            map_.erase(Key(*otherIter));
        return *this;
    }

    /** Remove a node by iterator.
     *
     *  Removes the node referenced by @p iter. The iterator must reference a valid node in this container.
     *
     *  @sa erase eraseMultiple eraseAtMultiple
     *
     *  @{ */
    Map& eraseAt(const NodeIterator &iter) {
        map_.erase(iter.base());
        return *this;
    }
    Map& eraseAt(const ConstKeyIterator &iter) {
        map_.erase(iter.base());
        return *this;
    }
    Map& eraseAt(const ValueIterator &iter) {
        map_.erase(iter.base());
        return *this;
    }
    /** @} */

    /** Remove multiple nodes by iterator range.
     *
     *  The iterator range must contain iterators that point into this container.
     *
     *  @sa erase eraseAt eraseMultiple
     *
     * @{ */
    template<class Iter>
    Map& eraseAtMultiple(const Iter &begin, const Iter &end) {
        map_.erase(begin.base(), end.base());
        return *this;
    }
    template<class Iter>
    Map& eraseAtMultiple(const boost::iterator_range<Iter> &range) {
        map_.erase(range.begin().base(), range.end().base());
        return *this;
    }
    /** @} */

};

} // namespace
} // namespace

#endif
