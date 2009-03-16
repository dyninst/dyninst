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

Graph::Graph() {};

Graph::NodePtr Graph::makeNode(Dyninst::InstructionAPI::Instruction &insn,
                               Address addr,
                               AbslocPtr absloc) {
    // First check to see if we already have this one
    if (insnNodes_[addr].find(absloc) != insnNodes_[addr].end()) { 
        return insnNodes_[addr][absloc];
    }

    // Otherwise create a new Node and insert it into the
    // map. 

    Node::Ptr newNode = InsnNode::createNode(addr, insn, absloc);
    
    // Update insnNodes_ so that we only create a particular node once.
    insnNodes_[addr][absloc] = newNode;

    return newNode;
}

Graph::NodePtr Graph::makeParamNode(Absloc::Ptr a) {
    if (parameterNodes_.find(a) == parameterNodes_.end()) {
        NodePtr n = ParameterNode::createNode(a);
        parameterNodes_[a] = n;
        return n;
    }
    return parameterNodes_[a];
}

Graph::NodePtr Graph::makeVirtualNode() {
    if (virtualNode_) return virtualNode_;
    virtualNode_ = VirtualNode::createNode(); 
    return virtualNode_;
}

Graph::Ptr Graph::createGraph() {
    return Graph::Ptr(new Graph());
}

void Graph::insertPair(NodePtr source, NodePtr target) {
    // TODO handle parameter edge types.

    Edge::Ptr e = Edge::createEdge(source, target);

    source->addOutEdge(e);
    target->addInEdge(e);
}

const Graph::NodeSet Graph::entryNodes() const {
    NodeSet ret;
    
    for (AbslocMap::const_iterator iter = parameterNodes_.begin();
         iter != parameterNodes_.end(); iter++) {
        ret.insert(iter->second);
    }
    if (virtualNode_) {
        ret.insert(virtualNode_);
    }
    return ret;
}

void Graph::recordCall(Address callAddr,
                       const CNodeRec &callInfo) {
    callRecords_[callAddr] = callInfo;
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
        fprintf(file, "\t // %s\n", source->name().c_str());
        Node::EdgeSet outs; source->outs(outs);
        //fprintf(stderr, "\t with %d out-edges\n", outs.size());
        for (Node::EdgeSet::iterator e = outs.begin(); e != outs.end(); e++) {
            Node::Ptr target = (*e)->target();
            fprintf(file, "\t %s -> %s;\n", source->name().c_str(), target->name().c_str());
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

void Graph::debugCallInfo() {
    for (CNodeMap::const_iterator c_iter = callRecords_.begin();
         c_iter != callRecords_.end();
         c_iter++) {
        fprintf(stderr, "Call at 0x%lx has the following reaching defs:\n",
                c_iter->first);
        for (CNodeRec::const_iterator a_iter = c_iter->second.begin();
             a_iter != c_iter->second.end();
             a_iter++) {
            fprintf(stderr, "\t Absloc %s:\n",
                    a_iter->first->name().c_str());
            for (CNodeSet::const_iterator s_iter = a_iter->second.begin();
                 s_iter != a_iter->second.end(); 
                 s_iter++) {
                fprintf(stderr, "\t\t 0x%lx / %s\n",
                        s_iter->second,
                        s_iter->first->name().c_str());
            }
        }
    }
}
