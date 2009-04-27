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

// Graph class implementation

#include "Graph.h"
#include "Edge.h"
#include "Node.h"
#include <assert.h>

using namespace Dyninst;

const Dyninst::Address Graph::INITIAL_ADDR = (Address) -1;

void Graph::entryNodes(NodeIterator &begin, NodeIterator &end) {
    assert(0);
    return;
}

void Graph::allNodes(NodeIterator &begin, NodeIterator &end) {
    assert(0);
    return;
}

bool Graph::find(Address addr, NodeIterator &begin, NodeIterator &end) {
    assert(0);
    return false;
}

Graph::Graph() {};

Graph::Ptr Graph::createGraph() {
    return Graph::Ptr(new Graph());
}

void Graph::insertPair(NodePtr source, NodePtr target) {
    // TODO handle parameter edge types.

    Edge::Ptr e = Edge::createEdge(source, target);

    addNode(source);
    addNode(target);

    source->addOutEdge(e);
    target->addInEdge(e);
}

void Graph::insertEntryNode(NodePtr entry) {
    addNode(entry);
    entryNodes_.insert(entry);
}

void Graph::addNode(Node::Ptr node) {
    if (node->hasInEdges() || node->hasOutEdges()) return;
    nodes_.insert(node);
    if (!node->isVirtual()) {
        nodesByAddr_[node->addr()].insert(node);
    }        
}

bool Graph::printDOT(const std::string fileName) {
    FILE *file = fopen(fileName.c_str(), "w");
    if (file == NULL) {
        return false;
    }

    fprintf(file, "digraph G {\n");

    NodeSet visited;
    std::queue<Node::Ptr> worklist;

    NodeIterator begin, end;
    entryNodes(begin, end);

    // Initialize visitor worklist
    for (; begin != end; begin++) {
        worklist.push(*begin);
    }

    while (!worklist.empty()) {
        Node::Ptr source = worklist.front();
        worklist.pop();

        //fprintf(stderr, "Considering node %s\n", source->name().c_str());
        
        // We may have already treated this node...
        if (visited.find(source) != visited.end()) {
            //fprintf(stderr, "\t skipping previously visited node\n");
            continue;
        }
        //fprintf(stderr, "\t inserting %s into visited set, %d elements pre-insert\n", source->name().c_str(), visited.size());
        visited.insert(source);
        fprintf(file, "\t // %s\n", source->format().c_str());

        NodeIterator outBegin, outEnd;
        source->outs(outBegin, outEnd);

        //fprintf(stderr, "\t with %d out-edges\n", outs.size());
        for (; outBegin != outEnd; outBegin++) {
            Node::Ptr target = *outBegin;
            fprintf(file, "\t %s -> %s;\n", source->format().c_str(), target->format().c_str());
            if (visited.find(target) == visited.end()) {
                //fprintf(stderr, "\t\t adding child %s\n", target->name().c_str());
                worklist.push(target);
            }
            else {
                //fprintf(stderr, "\t\t skipping previously visited child %s\n", 
                //target->name().c_str());
            }
        }
    }
    fprintf(file, "}\n\n\n");
    fclose(file);

    return true;
}
