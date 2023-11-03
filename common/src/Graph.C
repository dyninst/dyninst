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

// Graph class implementation

#include "Graph.h"
#include "Edge.h"
#include "Node.h"
#include <assert.h>

#include "NodeIterator.h"
#include <iostream>

using namespace Dyninst;

const Dyninst::Address Graph::INITIAL_ADDR = (Dyninst::Address) -1;



void Graph::entryNodes(NodeIterator &begin, NodeIterator &end) {
    begin = NodeIterator(new NodeIteratorSet(entryNodes_.begin()));
    end = NodeIterator(new NodeIteratorSet(entryNodes_.end()));
    return;
}

void Graph::exitNodes(NodeIterator &begin, NodeIterator &end) {
	// std::cerr << "exitNodes called: " << exitNodes_.size() << " nodes" << std::endl;
    begin = NodeIterator(new NodeIteratorSet(exitNodes_.begin()));
    end = NodeIterator(new NodeIteratorSet(exitNodes_.end()));
    return;
}

void Graph::allNodes(NodeIterator &begin, NodeIterator &end) {
    // std::cerr << "AllNodes called: " << nodes_.size() << " nodes" << std::endl;
    begin = NodeIterator(new NodeIteratorSet(nodes_.begin()));
	end = NodeIterator(new NodeIteratorSet(nodes_.end()));
	return;
}

bool Graph::find(Address addr, NodeIterator &begin, NodeIterator &end) {
    NodeMap::iterator iter = nodesByAddr_.find(addr);
    if (iter == nodesByAddr_.end()) return false;

    begin = NodeIterator(new NodeIteratorSet(iter->second.begin()));
    end = NodeIterator(new NodeIteratorSet(iter->second.end()));
    return true;
}

Graph::Graph() {}

Graph::Ptr Graph::createGraph() {
    return Graph::Ptr(new Graph());
}

void Graph::insertPair(NodePtr source, NodePtr target, EdgePtr e) {
    // TODO handle parameter edge types.
   if (!e) {
      e = Edge::createEdge(source, target);
   }
   else {
      e->setSource(source);
      e->setTarget(target);
   }
   
   source->addOutEdge(e);
   target->addInEdge(e);
   
   addNode(source);
   addNode(target);
}

void Graph::insertEntryNode(NodePtr entry) {
    addNode(entry);
    entryNodes_.insert(entry);
}

void Graph::insertExitNode(NodePtr exit) {
    addNode(exit);
    exitNodes_.insert(exit);
}

void Graph::markAsEntryNode(NodePtr entry) {
    entryNodes_.insert(entry);
}

void Graph::markAsExitNode(NodePtr exit) {
    exitNodes_.insert(exit);
}

void Graph::addNode(Node::Ptr node) {
  //if (node->hasInEdges() || node->hasOutEdges()) return;
    nodes_.insert(node);
    if (!node->isVirtual()) {
        nodesByAddr_[node->addr()].insert(node);
    }        
    // std::cerr << "\t\t After addNode: " << nodes_.size() << " nodes" << std::endl;
    // std::cerr << "\t\t\t adding " << node << std::endl;
}

void Graph::clearEntryNodes() {
    entryNodes_.clear();
}

void Graph::clearExitNodes() {
    exitNodes_.clear();
}
// Fancy-shmancy predicate based search methods...

bool Graph::find(NodePredicate::Ptr pred, NodeIterator &begin, NodeIterator &end) {
    // We could try to be lazy here, but you have a hard time determining if you're
    // done a priori. So instead we pre-look-up and hand that set into a copy-based
    // set iterator. 
    NodeIterator allBegin, allEnd;
    allNodes(allBegin, allEnd);
    begin = NodeIterator(new NodeIteratorPredicateObj(pred, allBegin, allEnd));
    end = NodeIterator(new NodeIteratorPredicateObj(pred, allEnd, allEnd));
    return (begin != end);
}
                         
bool Graph::find(NodePredicateFunc pred, void *user_arg, NodeIterator &begin, NodeIterator &end) {
    // We could try to be lazy here, but you have a hard time determining if you're
    // done a priori. So instead we pre-look-up and hand that set into a copy-based
    // set iterator. 
    NodeIterator allBegin, allEnd;
    allNodes(allBegin, allEnd);
    begin = NodeIterator(new NodeIteratorPredicateFunc(pred, user_arg, allBegin, allEnd));
    end = NodeIterator(new NodeIteratorPredicateFunc(pred, user_arg, allEnd, allEnd));
    return (begin != end);
}

void Graph::deleteNode(NodePtr node) {
  //std::cerr << "Deleting node " << node << std::endl;
  // Remove from the graph map
  // Remove from any edges that point to this one

  nodes_.erase(node);

  NodeMap::iterator iter2 = nodesByAddr_.find(node->addr());
  if (iter2 != nodesByAddr_.end()) {
    iter2->second.erase(node);
  }

  entryNodes_.erase(node);
  exitNodes_.erase(node);

  EdgeIterator e1, e2;
  node->ins(e1, e2);
  for (; e1 != e2; ++e1) {
    (*e1)->source()->deleteOutEdge(e1);
  }

  node->outs(e1, e2);
  for (; e1 != e2; ++e1) {
    (*e1)->target()->deleteInEdge(e1);
  }
}
    
bool Graph::isEntryNode(NodePtr node) {
  return (entryNodes_.find(node) != entryNodes_.end());
}


bool Graph::isExitNode(NodePtr node) {
  return (exitNodes_.find(node) != exitNodes_.end());
}

unsigned Graph::size() const {
   return nodes_.size();
}

void Graph::adjustEntryAndExitNodes() {
    entryNodes_.clear();
    exitNodes_.clear();
    NodeIterator gbegin, gend;
    allNodes(gbegin, gend);
    for (; gbegin != gend; ++gbegin) {
        Node::Ptr ptr = *gbegin;
	if (!ptr->hasInEdges()) insertEntryNode(ptr);
	if (!ptr->hasOutEdges()) insertExitNode(ptr);
    }

}
