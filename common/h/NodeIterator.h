/*
 * Copyright (c) 2007-2008 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

// Implementations of Node iterators

#if !defined(NODE_ITERATOR_H)
#define NODE_ITERATOR_H

#include "Node.h"
#include "Edge.h"

namespace Dyninst {

// This is a pure virtual interface class
class NodeIteratorImpl {
    friend class NodeIterator;
    
 public:
    virtual void inc() = 0;
    virtual void dec() = 0;
    virtual Node::Ptr get() = 0;
    virtual bool equals(NodeIteratorImpl *) = 0;
    virtual NodeIteratorImpl *copy() = 0;

    virtual ~NodeIteratorImpl() {};
};

// Types of node iteration: over a set of nodes
class NodeIteratorSet : public NodeIteratorImpl {
 public:
    virtual void inc() { ++internal_; }
    virtual void dec() { --internal_; }
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
    
    NodeIteratorSet(const std::set<Node::Ptr>::iterator iter) : internal_(iter) {};

 private:
    std::set<Node::Ptr>::iterator internal_;
};

class NodeFromEdgeSet : public NodeIteratorImpl {
 public:
    typedef enum {
        target,
        source,
        unset } iterType;

    virtual void inc() { ++internal_; }
    virtual void dec() { --internal_; }
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

    virtual ~NodeFromEdgeSet() {};

    NodeFromEdgeSet(const std::set<Edge::Ptr>::iterator iter,
                   iterType type) : internal_(iter), type_(type) {};

 private:
    std::set<Edge::Ptr>::iterator internal_;
    iterType type_;
};

}

#endif
