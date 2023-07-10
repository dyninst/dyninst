/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// Implementations of Node iterators

#if !defined(NODE_ITERATOR_H)
#define NODE_ITERATOR_H

#include <assert.h>
#include <deque>
#include <set>
#include <unordered_set>
#include "Node.h"
#include "Edge.h"

namespace Dyninst {

// This is a pure virtual interface class
class NodeIteratorImpl {
    friend class NodeIterator;
    
 public:
    virtual void inc() = 0;
//    virtual void dec() = 0;
    virtual Node::Ptr get() = 0;
    virtual bool equals(NodeIteratorImpl *) = 0;
    virtual NodeIteratorImpl *copy() = 0;

    virtual ~NodeIteratorImpl() {}
};

// Types of node iteration: over a set of nodes
class NodeIteratorSet : public NodeIteratorImpl {
 public:
    virtual void inc() { ++internal_; }
//    virtual void dec() { --internal_; }
    virtual Node::Ptr get() { return *internal_; }
    virtual bool equals(NodeIteratorImpl *rhs) {
        NodeIteratorSet *tmp = dynamic_cast<NodeIteratorSet *>(rhs);
        if (tmp == NULL) return false;
        return internal_ == tmp->internal_;
    }

    virtual NodeIteratorImpl *copy() {
        return new NodeIteratorSet(internal_);
    }

    virtual ~NodeIteratorSet() {
        // Nothing to do
    }
    
    NodeIteratorSet(const std::unordered_set<Node::Ptr, Node::NodePtrHasher>::iterator iter) : internal_(iter) {}

 private:
    std::unordered_set<Node::Ptr, Node::NodePtrHasher>::iterator internal_;
};

class NodeFromEdgeSet : public NodeIteratorImpl {
 public:
    typedef enum {
        target,
        source,
        unset } iterType;

    virtual void inc() { ++internal_; }
//    virtual void dec() { --internal_; }
    virtual Node::Ptr get() { 
        switch(type_) {
        case target:
            return (*internal_)->target();
        case source:
            return (*internal_)->source();
        default:
            return Node::Ptr();
        }
    }
    virtual bool equals(NodeIteratorImpl *rhs) {
        NodeFromEdgeSet *tmp = dynamic_cast<NodeFromEdgeSet *>(rhs);
        if (tmp == NULL) return false;

        return ((internal_ == tmp->internal_)  &&
                (type_ == tmp->type_));
    }
    virtual NodeIteratorImpl *copy () {
        NodeIteratorImpl *tmp = new NodeFromEdgeSet(internal_, type_);
        return tmp;
    }

    virtual ~NodeFromEdgeSet() {}

    NodeFromEdgeSet(const std::unordered_set<Edge::Ptr, Edge::EdgePtrHasher>::iterator iter,
                   iterType type) : internal_(iter), type_(type) {}

 private:
    std::unordered_set<Edge::Ptr, Edge::EdgePtrHasher>::iterator internal_;
    iterType type_;
};

class NodeSearchIterator : public NodeIteratorImpl{
    friend class NodeIterator;
 public:
    // Share implementation, as DFS/BFS are very similar
    typedef enum {
        depth,
        breadth} Type;
    typedef enum {
        in,
        out} Direction;

    // This iterator linearizes access to a tree/graph structure.
    // Since we operate over a graph there may be cycles. This
    // is handled by keeping a visited set; once a node is encountered
    // it is placed in the visited set. 
    //
    // The iterator is defined to be "end" if the following is true:
    // 1) The current pointer is NULL;
    // The iterator is one step from "end" (that is, iter->inc == end) if the 
    // following is true:
    // 2) Current is non-NULL and every element in the worklist has been visited. 
    // Due to 2) above, we _must_ describe the worklist and visited sets in terms
    // of nodes, rather than iterators; given an iterator we cannot determine 
    // (without deep inspection) whether it contains an unvisited node. Deep inspection
    // really obviates the point, here. 

    // TODO: reverse iteration. 

    virtual void inc() {
        // If current is NULL, we're done
        if (!current) return;

        Node::Ptr next;
        // Iterate over the worklist and see if any element there
        // has not been visited.
        while (next == NULL && (!worklist.empty())) {
            next = popWorklist();
        }
        // If the worklist is empty, then set to end and
        // return
        if (worklist.empty()) {
            setToEnd();
            return;
        }
        assert(next != NULL);

        current = next;
        
        updateVisited(current);
        NodeIterator begin, end;
        getNext(begin, end);
        updateWorklist(begin, end);
    }
/*    
    virtual void dec() {
        // Unsupported
        assert(0); return;
    }
*/
    virtual Node::Ptr get() { return current; }
    // Equality
    // 1) End == end, even if the internal data is not the same
    // 2) Otherwise, identical iterators are equal.
    virtual bool equals(NodeIteratorImpl *rhs) {
        NodeSearchIterator *tmp = dynamic_cast<NodeSearchIterator *>(rhs);
        if (!tmp) return false;

        if (end() && tmp->end()) return true;
        return ((current == tmp->current) &&
                (type == tmp->type) &&
                (direction == tmp->direction) &&
                (worklist == tmp->worklist) &&
                (visited == tmp->visited));
    }

    virtual NodeIteratorImpl *copy() {
        return new NodeSearchIterator(current, direction, type, worklist, visited);
    }

    NodeSearchIterator() : direction(in), type(depth) {}
    NodeSearchIterator(Node::Ptr cur, Direction d, Type t) : current(cur), direction(d), type(t) {
        updateVisited(current);
        NodeIterator begin, end;
        getNext(begin,end);
        updateWorklist(begin,end);
    }
    NodeSearchIterator(Node::Ptr cur, Direction d, Type t, std::deque<Node::Ptr> wl, std::set<Node::Ptr> v) :
        current(cur), direction(d), type(t), worklist(wl), visited(v) {}

    NodeSearchIterator(NodeIterator &rangeBegin, NodeIterator &rangeEnd, Direction d, Type t) :
        direction(d), type(t) {
        updateWorklist(rangeBegin, rangeEnd);

        if (worklist.empty()) return;
        current = popWorklist();
        updateVisited(current);
        NodeIterator begin, end;
        getNext(begin, end);
        updateWorklist(begin, end);
    }

    virtual ~NodeSearchIterator() {}

 private:

    bool end() {
        if (current == NULL) return true;
        return false;
    }

    void updateVisited(Node::Ptr &node) {
        assert(node);
        visited.insert(node);
    }

    // This is where the direction and type of iterator come in.
    // Direction: select whether we access the in set or out set of nodes.
    // Type: DFS: push to the front of the worklist; BFS: push to the back.
    void getNext(NodeIterator &begin, NodeIterator &end) {
        assert(current);
        switch (direction) {
        case in:
            current->ins(begin, end);
            break;
        case out:
            current->outs(begin, end);
            break;
        default:
            assert(0);
        }
    }
    void updateWorklist(NodeIterator &begin, NodeIterator &end) {
        for (; begin != end; begin++) {
            if (visited.find(*begin) == visited.end()) {
                switch (type) {
                case depth:
                    worklist.push_front(*begin);
                    break;
                case breadth:
                    worklist.push_back(*begin);
                    break;
                default:
                    assert(0);
                }
            }
        }
    }

    Node::Ptr popWorklist() {
        if (worklist.empty()) return Node::Ptr();
        Node::Ptr tmp = worklist.front();
        worklist.pop_front();
        return tmp;
    }

    void setToEnd() {
        current = Node::Ptr();
        worklist.clear();
        visited.clear();
    }

    Node::Ptr current;
    Direction direction;
    Type type;
    std::deque<Node::Ptr> worklist;
    std::set<Node::Ptr> visited;
};

// And given a set to hold internally until the iterator goes away
class NodeIteratorPredicateObj : public NodeIteratorImpl {
 public:
    virtual void inc() { 
        if (cur == end) return;
        cur = next;
        setNext();
    }
//    virtual void dec() {  return; }
    virtual Node::Ptr get() { return *cur; }
    virtual bool equals(NodeIteratorImpl *rhs) {
        NodeIteratorPredicateObj *tmp = dynamic_cast<NodeIteratorPredicateObj *>(rhs);
        if (tmp == NULL) return false;
        return ((pred == tmp->pred) &&
                (cur == tmp->cur) &&
                (next == tmp->next) &&
                (end == tmp->end));
    }

    virtual NodeIteratorImpl *copy() {
        return new NodeIteratorPredicateObj(pred, cur, next, end);
    }

    virtual ~NodeIteratorPredicateObj() {
        // Nothing to do
    }
    
    NodeIteratorPredicateObj(Graph::NodePredicate::Ptr p,
                    NodeIterator &c,
                    NodeIterator &n,
                    NodeIterator &e) :
        pred(p),
        cur(c), next(n), end(e) {}
    NodeIteratorPredicateObj(Graph::NodePredicate::Ptr p,
                             NodeIterator &b,
                             NodeIterator &e) :
        pred(p), cur(b), next(b), end(e) {
        setNext();
        // next is now a matching node. If the start wasn't,
        // then we need to increment...
        if ((cur != end) && !pred->predicate(*cur)) {
        	NodeIteratorPredicateObj::inc();
        }
    }
    void setNext() {
        // Set next to the, well, next match
        if (next == end) return;
        do {
            ++next; 
        } while (next != end && !(pred->predicate(*next)));
    }

 private:
    Graph::NodePredicate::Ptr pred;
    // We're not allowing reverse iteration over this, since we really
    // don't want to suffer copy overhead. 
    NodeIterator cur, next, end;
};

// And given a set to hold internally until the iterator goes away
class NodeIteratorPredicateFunc : public NodeIteratorImpl {
 public:
    virtual void inc() { 
        if (cur == end) return;
        cur = next;
        setNext();
    }
//    virtual void dec() {  return; }
    virtual Node::Ptr get() { return *cur; }
    virtual bool equals(NodeIteratorImpl *rhs) {
        NodeIteratorPredicateFunc *tmp = dynamic_cast<NodeIteratorPredicateFunc *>(rhs);
        if (tmp == NULL) return false;
        return ((pred == tmp->pred) &&
                (user_arg == tmp->user_arg) &&
                (cur == tmp->cur) &&
                (next == tmp->next) &&
                (end == tmp->end));
    }

    virtual NodeIteratorImpl *copy() {
        return new NodeIteratorPredicateFunc(pred, user_arg, cur, next, end);
    }

    virtual ~NodeIteratorPredicateFunc() {
        // Nothing to do
    }
    
    NodeIteratorPredicateFunc(Graph::NodePredicateFunc p,
                              void *u, 
                              NodeIterator &c,
                              NodeIterator &n,
                              NodeIterator &e) :
        pred(p),
        user_arg(u),
        cur(c), next(n), end(e) {}
    NodeIteratorPredicateFunc(Graph::NodePredicateFunc p,
                              void *u,
                              NodeIterator &b,
                              NodeIterator &e) :
        pred(p), user_arg(u), cur(b), next(b), end(e) {
        setNext();
        // next is now a matching node. If the start wasn't,
        // then we need to increment...
        if ((cur != end) && !pred(*cur, user_arg)) {
        	NodeIteratorPredicateFunc::inc();
        }
    }
    void setNext() {
        // Set next to the, well, next match
        if (next == end) return;
        ++next;
        do {
            ++next; 
        } while (next != end && !(pred(*next, user_arg)));
    }

 private:
    Graph::NodePredicateFunc pred;
    void *user_arg;
    // We're not allowing reverse iteration over this, since we really
    // don't want to suffer copy overhead. 
    NodeIterator begin, cur, next, end;
};

}

#endif
