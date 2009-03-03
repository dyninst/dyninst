/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// Graph class implementation

#include "Graph.h"
#include "Absloc.h"
#include "Edge.h"
#include "Node.h"
#include <assert.h>

using namespace Dyninst::DDG;

const Dyninst::Address Graph::INITIAL_ADDR = (Address) -1;

Graph::Graph() : entryNodesUpdated_(true) {};

Node::Ptr Graph::makeNode(Dyninst::InstructionAPI::Instruction &insn,
                          Address addr,
                          AbslocPtr absloc) {
    // First check to see if we already have this one
    if (allNodes_[addr].find(absloc) != allNodes_[addr].end()) { 
        return allNodes_[addr][absloc];
    }

    // Otherwise create a new Node and insert it into the
    // map. 

    Node::Ptr newNode = Node::createNode(addr, insn, absloc);
    
    // Update allNodes_ so that we only create a particular node once.
    allNodes_[addr][absloc] = newNode;
    
    // We created a node, so our list of entry nodes is _not_ up to date.
    entryNodesUpdated_ = false;

    return newNode;
}

Graph::NodePtr Graph::makeParamNode(Absloc::Ptr a) {
    fprintf(stderr, "Creating initial definition node for %s\n", a->name().c_str());
    
    // Create a known "initial" node and insert it into the
    // entryNodes_ structure.
    if (allNodes_[INITIAL_ADDR].find(a) == allNodes_[INITIAL_ADDR].end()) {
        NodePtr n = Node::createNode(a);
        allNodes_[INITIAL_ADDR][a] = n;
        return n;
    }

    return allNodes_[INITIAL_ADDR][a];
}
   

Graph::Ptr Graph::createGraph() {
    return Graph::Ptr(new Graph());
}

void Graph::insertPair(NodePtr source, NodePtr target) {
    fprintf(stderr, "Inserting <%s, %s>\n",
            source->name().c_str(), 
            target->name().c_str());

    // This asserts that we don't try to insert an unknown
    // node.
    assert(allNodes_[source->addr()].find(source->absloc()) 
           != allNodes_[source->addr()].end());

    allNodes_[target->addr()][target->absloc()] = target;

    Edge::Ptr e = Edge::createEdge(source, target);

    // Since we changed the graph we may have invalidated the list of
    // entry nodes.
    entryNodesUpdated_ = false;

    source->addOutEdge(e);
    target->addInEdge(e);
}

const Graph::NodeSet &Graph::entryNodes() {
    // If the list of entry nodes is up to date, return it.
    if (entryNodesUpdated_) return entryNodes_;

    // Otherwise look at all nodes and find those with no in-edges.
    // Those become our entry nodes.
    // Assertion: there are no true cycles in the DDG as there _must_
    // exist an initial definition.
    entryNodes_.clear();

    for (NodeMap::iterator i = allNodes_.begin(); i != allNodes_.end(); i++) {
        for (AbslocMap::iterator j = (*i).second.begin(); j != (*i).second.end(); j++) {
            entryNodes_.insert((*j).second);
        }
    }
    entryNodesUpdated_ = true;
    return entryNodes_;
}

void Graph::printDOT() {
    fprintf(stderr, "digraph G {\n");

    NodeSet visited;
    std::queue<Node::Ptr> worklist;

    const NodeSet &entries = entryNodes();

    // Initialize visitor worklist
    for (NodeSet::const_iterator i = entries.begin(); i != entries.end(); i++) {
        worklist.push(*i);
    }

    while (!worklist.empty()) {
        Node::Ptr source = worklist.front();
        worklist.pop();
        visited.insert(source);

        Node::EdgeSet outs; source->outs(outs);
        for (Node::EdgeSet::iterator e = outs.begin(); e != outs.end(); e++) {
            Node::Ptr target = (*e)->target();
            fprintf(stderr, "\t %s -> %s;\n", source->name().c_str(), target->name().c_str());
            if (visited.find(target) == visited.end()) {
                worklist.push(target);
            }
        }
    }
    fprintf(stderr, "}\n\n\n");
}

