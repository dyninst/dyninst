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


// Nodes are quite simple; they have an Insn, an Absloc, and a set of Edges.

using namespace Dyninst;

const Address Node::INVALID_ADDR = (Address) -1;

void Node::ins(NodeIterator &begin, NodeIterator &end) {
    assert(0);
}

void Node::outs(NodeIterator &begin, NodeIterator &end) {
    assert(0);
}

void Node::ins(EdgeIterator &begin, EdgeIterator &end) {
    assert(0);
}

void Node::outs(EdgeIterator &begin, EdgeIterator &end) {
    assert(0);
}

bool Node::hasInEdges() {
    return !ins_.empty();
}

bool Node::hasOutEdges() {
    return !outs_.empty();
}


Node::Ptr PhysicalNode::createNode(Address addr) {
    return Node::Ptr(new PhysicalNode(addr));
}

std::string PhysicalNode::format() const {
    char buf[256];
    sprintf(buf, "N_0x%lx", addr());
    return std::string(buf);
}

Node::Ptr VirtualNode::createNode() {
    return Node::Ptr(new VirtualNode());
}

std::string VirtualNode::format() const {
    return std::string("N_VIRTUAL");
}

void NodeIterator::operator++(int) {
    // Oops..
    assert(0);
}

void NodeIterator::operator++() {
    assert(0);
}

Node::Ptr NodeIterator::operator*() {
    assert(0);
    return Node::Ptr();
}

bool NodeIterator::operator!=(const NodeIterator &rhs) {
    assert(0);
    return false;
}

