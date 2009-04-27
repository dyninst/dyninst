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

using namespace Dyninst::DepGraphAPI;

const Dyninst::Address Graph::INITIAL_ADDR = (Address) -1;

Graph::Graph() {};

Graph::Ptr Graph::createGraph() {
    return Graph::Ptr(new Graph());
}

void Graph::insertPair(NodePtr source, NodePtr target) {
    // TODO handle parameter edge types.

    Edge::Ptr e = Edge::createEdge(source, target);

    source->addOutEdge(e);
    target->addInEdge(e);

    nodes_.insert(source);
    nodes_.insert(target);

    if (!source->isVirtual()) {
        nodesByAddr_[source->addr()].insert(source);
    }
    if (!target->isVirtual()) {
        nodesByAddr_[target->addr()].insert(target);
    }

}

const Graph::NodeSet &Graph::getNodesAtAddr(Address addr) {
    return nodesByAddr_[addr];
}

bool Graph::printDOT(const std::string fileName) {
    FILE *file = fopen(fileName.c_str(), "w");
    if (file == NULL) {
        return false;
    }

    fprintf(file, "digraph G {\n");

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

        //fprintf(stderr, "Considering node %s\n", source->name().c_str());
        
        // We may have already treated this node...
        if (visited.find(source) != visited.end()) {
            //fprintf(stderr, "\t skipping previously visited node\n");
            continue;
        }
        //fprintf(stderr, "\t inserting %s into visited set, %d elements pre-insert\n", source->name().c_str(), visited.size());
        visited.insert(source);
        fprintf(file, "\t // %s\n", source->format().c_str());
        Node::EdgeSet outs; source->outs(outs);
        //fprintf(stderr, "\t with %d out-edges\n", outs.size());
        for (Node::EdgeSet::iterator e = outs.begin(); e != outs.end(); e++) {
            Node::Ptr target = (*e)->target();
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

DDG::Ptr DDG::createGraph() {
    return Ptr(new DDG());
}

#if 0
/**
 * Returns a new graph after copying the nodes and edges into this new graph.
 */
DDG::Ptr DDG::copyGraph() {
  typedef NodeSet::iterator NodeIter;
  typedef std::set<Edge::Ptr> EdgeSet;
  typedef EdgeSet::iterator EdgeIter;
  
  DDG::Ptr newGraph = DDG::createGraph();
  NodePtr virtNode = newGraph->makeVirtualNode();
  NodeSet nodes;
  allNodes(nodes);
  for (NodeIter nodeIter = nodes.begin(); nodeIter != nodes.end(); nodeIter++) {
    NodePtr node = *nodeIter;
    // create a copy of the node and insert into the graph.
    NodePtr newNode = node->copyTo(newGraph); // copyNode(copy, node);
    newGraph->insertPair(virtNode, newNode);
    
    // process outgoing edges. no need to process incoming edges since they are
    // outgoing edges from another node and will be processed then.
    EdgeSet outEdges;
    node->outs(outEdges);
    for (EdgeIter iter = outEdges.begin(); iter != outEdges.end(); iter++) {
      NodePtr target = (*iter)->target();
      InstructionAPI::Instruction targetIns = target->insn();

      // create a new target for this edge.
      NodePtr newTarget = target->copyTo(newGraph); //copyNode(copy, target);
      // add edge from newNode to newTarget.
      newGraph->insertPair(newNode, newTarget);
    }
  }
  return newGraph;
}

#endif
