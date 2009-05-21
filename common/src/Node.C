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


// Node class implementation

#include "Graph.h"
#include "Edge.h"
#include "Node.h"
#include <assert.h>

#include "common/h/NodeIterator.h"

// Nodes are quite simple; they have an Insn, an Absloc, and a set of Edges.

using namespace Dyninst;

const Address Node::INVALID_ADDR = (Address) -1;

void Node::ins(NodeIterator &begin, NodeIterator &end) {
    // Ins and outs are currently stored as sets of edges.
    // We need an iterator that translates them...
    begin = NodeIterator(new NodeFromEdgeSet(ins_.begin(), NodeFromEdgeSet::source));
    end = NodeIterator(new NodeFromEdgeSet(ins_.end(), NodeFromEdgeSet::source));
}

void Node::outs(NodeIterator &begin, NodeIterator &end) {
    begin = NodeIterator(new NodeFromEdgeSet(outs_.begin(), NodeFromEdgeSet::target));
    end = NodeIterator(new NodeFromEdgeSet(outs_.end(), NodeFromEdgeSet::target));
}

void Node::ins(EdgeIterator &begin, EdgeIterator &end) {
    // Iteration over a pre-existing set is easy...
    begin = EdgeIterator(new EdgeIteratorSet(ins_.begin()));
    end = EdgeIterator(new EdgeIteratorSet(ins_.end()));
}

void Node::outs(EdgeIterator &begin, EdgeIterator &end) {
    // Iteration over a pre-existing set is easy...
    begin = EdgeIterator(new EdgeIteratorSet(outs_.begin()));
    end = EdgeIterator(new EdgeIteratorSet(outs_.end()));
}

bool Node::hasInEdges() {
    return !ins_.empty();
}

bool Node::hasOutEdges() {
    return !outs_.empty();
}

void Node::forwardClosure(NodeIterator &begin, NodeIterator &end) {
    end = NodeIterator(new NodeSearchIterator());

    if (!hasOutEdges()) {
        begin = end;
    }
    else {
        NodeIterator outBegin, outEnd;
        outs(outBegin, outEnd);
        begin = NodeIterator(new NodeSearchIterator(outBegin, outEnd, NodeSearchIterator::out, NodeSearchIterator::breadth));
    }
}

void Node::backwardClosure(NodeIterator &begin, NodeIterator &end) {
    end = NodeIterator(new NodeSearchIterator());

    if (!hasInEdges()) {
        begin = end;
    }
    else {
        NodeIterator inBegin, inEnd;
        ins(inBegin, inEnd);
        begin = NodeIterator(new NodeSearchIterator(inBegin, inEnd, NodeSearchIterator::in, NodeSearchIterator::breadth));
    }
}


Node::Ptr PhysicalNode::createNode(Address addr) {
    return Node::Ptr(new PhysicalNode(addr));
}

Node::Ptr PhysicalNode::copy() {
    return Node::Ptr(new PhysicalNode(addr()));
}

std::string PhysicalNode::format() const {
    char buf[256];
    sprintf(buf, "N_0x%lx", addr());
    return std::string(buf);
}

Node::Ptr VirtualNode::createNode() {
    return Node::Ptr(new VirtualNode());
}

Node::Ptr VirtualNode::copy() {
    return Node::Ptr(new VirtualNode());
}

std::string VirtualNode::format() const {
    return std::string("N_VIRTUAL");
}

// Prefix...
NodeIterator &NodeIterator::operator++() {
    if (!iter_) return *this;
    
    iter_->inc();
    return *this;
}

// Postfix...
NodeIterator NodeIterator::operator++(int) {
    NodeIterator ret = *this;
    ++(*this);
    return ret;    
}

// Prefix...
NodeIterator &NodeIterator::operator--() {
    if (!iter_) return *this;
    
    iter_->dec();
    return *this;
}

// Postfix...
NodeIterator NodeIterator::operator--(int) {
    NodeIterator ret = *this;
    --(*this);
    return ret;    
}


Node::Ptr NodeIterator::operator*() const {
    if (!iter_) return Node::Ptr();

    return iter_->get();
}

bool NodeIterator::operator!=(const NodeIterator &rhs) const {
    if (!iter_) {
        return (rhs.iter_ != NULL);
    }
    return !iter_->equals(rhs.iter_);
}

bool NodeIterator::operator==(const NodeIterator &rhs) const {
    if (!iter_) {
        return (rhs.iter_ == NULL);
    }
    return iter_->equals(rhs.iter_);
}

NodeIterator &NodeIterator::operator=(const NodeIterator &rhs) {
    if (iter_) delete iter_;
    iter_ = rhs.copy();
    return *this;
}

NodeIterator::NodeIterator() : iter_(NULL) {
}

NodeIterator::NodeIterator(NodeIteratorImpl *iter) :
    iter_(iter) {
}

NodeIterator::NodeIterator(const NodeIterator &rhs) :
    iter_(rhs.copy()) {
}

NodeIterator::~NodeIterator() { 
    if (iter_) delete iter_;
}

NodeIteratorImpl *NodeIterator::copy() const {
    if (iter_ == NULL) return NULL;
    return iter_->copy();
}
