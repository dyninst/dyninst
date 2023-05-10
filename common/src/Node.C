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


// Node class implementation

#include "Graph.h"
#include "Edge.h"
#include "Node.h"
#include <assert.h>

#include "common/src/NodeIterator.h"

// Nodes are quite simple; they have an Insn, an Absloc, and a set of Edges.

using namespace Dyninst;

const Address Node::INVALID_ADDR = (Address) -1;


void Node::addInEdge(const EdgePtr in) {
   ins_.insert(in);
}
 
void Node::addOutEdge(const EdgePtr out) {
   outs_.insert(out);
}   

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

std::string VirtualNode::defaultName("N_VIRTUAL");

Node::Ptr VirtualNode::createNode() {
    return Node::Ptr(new VirtualNode());
}

Node::Ptr VirtualNode::createNode(std::string name) {
    return Node::Ptr(new VirtualNode(name));
}

Node::Ptr VirtualNode::copy() {
    return Node::Ptr(new VirtualNode(name_));
}

std::string VirtualNode::format() const {
    return name_;
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
/*
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
*/

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
    if(this == &rhs) return *this;
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

Graph::Ptr Node::forwardSubgraph() {
    // We want to copy this node and every node reachable from
    // it along a forward direction. This node will become the
    // entry for a new subgraph. 

    // TODO: this is a generic graph by definition, as nodes
    // have no idea what type of graph they belong to. Is that
    // the right thing? 

    Graph::Ptr ret = Graph::createGraph();
    
    Node::Ptr newEntry = copy();
    ret->insertEntryNode(newEntry);
    
    std::set<Node::Ptr> visited;
    std::queue<std::pair<Node::Ptr, Node::Ptr> > worklist;

    // We don't have a shared pointer to the current node. 
    // However, we do have an edge, which has a weak pointer. 

    if (outs_.empty()) return ret;
    Node::Ptr thisPtr = (*outs_.begin())->source();

    worklist.push(std::make_pair(thisPtr, newEntry));

    while (!worklist.empty()) {
        // First is the original node, second the copy
        std::pair<Node::Ptr, Node::Ptr> src = worklist.front(); 
        worklist.pop();

        if (visited.find(src.first) != visited.end()) continue;
        visited.insert(src.first);

        NodeIterator b, e;
        src.first->outs(b, e);

        if (b == e) {
            ret->insertExitNode(src.second);
        }

        for (; b != e; ++b) {
            std::pair<Node::Ptr, Node::Ptr> targ = std::make_pair(*b, (*b)->copy());
            ret->insertPair(src.second, targ.second);
            worklist.push(targ);
        } 
    }
    return ret;
}

Graph::Ptr Node::backwardSubgraph() {
    // We want to copy this node and every node reachable from
    // it along a forward direction. This node will become the
    // entry for a new subgraph. 

    // TODO: this is a generic graph by definition, as nodes
    // have no idea what type of graph they belong to. Is that
    // the right thing? 

    Graph::Ptr ret = Graph::createGraph();
    
    Node::Ptr newExit = copy();
    ret->insertExitNode(newExit);
    
    std::set<Node::Ptr> visited;
    std::queue<std::pair<Node::Ptr, Node::Ptr> > worklist;
    
    // See comment in nearly-identical code above...
    if (ins_.empty()) return ret;
    Node::Ptr thisPtr = (*ins_.begin())->target();

    worklist.push(std::make_pair(thisPtr, newExit));

    while (!worklist.empty()) {
        // First is the original node, second the copy
        std::pair<Node::Ptr, Node::Ptr> targ = worklist.front(); 
        worklist.pop();

        if (visited.find(targ.first) != visited.end()) continue;
        visited.insert(targ.first);

        NodeIterator b, e;

        targ.first->ins(b, e);

        if (b == e) {
            ret->insertEntryNode(targ.second);
        }

        for (; b != e; ++b) {
            std::pair<Node::Ptr, Node::Ptr> src = std::make_pair(*b, (*b)->copy());
            ret->insertPair(src.second, targ.second);
            worklist.push(src);
        } 
    }
    return ret;
}

void Node::deleteInEdge(EdgeIterator e) {
  ins_.erase(*e);
}

void Node::deleteOutEdge(EdgeIterator e) {
  outs_.erase(*e);
}

