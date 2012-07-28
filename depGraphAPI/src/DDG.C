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

#include "dynptr.h"
#include <set>
#include <list>
#include <queue>
#include "Annotatable.h"
#include "Instruction.h"
#include "Node.h"
#include "Absloc.h"
#include "Graph.h"
#include "DepGraphNode.h"
#include "DDG.h"
#include "analyzeDDG.h"

#include "BPatch_function.h"

#include "common/h/NodeIterator.h"
#include "DDGIterator.h"

using namespace Dyninst;
using namespace DepGraphAPI;

DDG::DDG(Function *func) :
    func_(func)
{}


DDG::Ptr DDG::analyze(BPatch_function *func) {
    DDGAnalyzer ddgA(func);

    DDG::Ptr ddg = ddgA.analyze();

    return ddg;
}

extern AnnotationClass <DDG::Ptr> DDGAnno;

void DDG::removeAnnotation() {
    assert(func_);
    
    // Note: removeAnnotation just NULLs it out, leading
    // to wasted memory...
    
    DDG::Ptr *ret;
    func_->getAnnotation(ret, DDGAnno);
    if (ret) {
        delete ret;
        func_->removeAnnotation(DDGAnno);
    }
}

void DDG::formalParamNodes(NodeIterator &begin, NodeIterator &end) {
    begin = NodeIterator(new FormalParamSetIter(formalParamNodes_.begin()));
    end = NodeIterator(new FormalParamSetIter(formalParamNodes_.end()));
}

void DDG::formalReturnNodes(NodeIterator &begin, NodeIterator &end) {
    begin = NodeIterator(new FormalReturnSetIter(formalReturnNodes_.begin()));
    end = NodeIterator(new FormalReturnSetIter(formalReturnNodes_.end()));
}

bool DDG::actualParamNodes(Address call, NodeIterator &begin, NodeIterator &end) {
    ActualParamNodeMap::iterator iter = actualParamNodes_.find(call + 1);
    if (iter == actualParamNodes_.end()) return false;

    begin = NodeIterator(new NodeIteratorSet(iter->second.begin()));
    end = NodeIterator(new NodeIteratorSet(iter->second.end()));
    return true;
}

bool DDG::actualReturnNodes(Address call, NodeIterator &begin, NodeIterator &end) {
    ActualReturnNodeMap::iterator iter = actualReturnNodes_.find(call + 1);
    if (iter == actualReturnNodes_.end()) return false;

    begin = NodeIterator(new NodeIteratorSet(iter->second.begin()));
    end = NodeIterator(new NodeIteratorSet(iter->second.end()));
    return true;
}

void DDG::entryNodes(NodeIterator &begin, NodeIterator &end) {
    // This is a little more tricky... we want to iterate over all
    // of the formal parameter nodes, then over the children of the
    // virtual node (if any exist)

    NodeIterator paramBegin(new FormalParamSetIter(formalParamNodes_.begin()));
    NodeIterator paramEnd(new FormalParamSetIter(formalParamNodes_.end()));
    NodeIterator immBegin(new NodeIteratorSet(immediateDefinitions_.begin()));
    NodeIterator immEnd(new NodeIteratorSet(immediateDefinitions_.end()));

    begin = NodeIterator(new DDGEntryIter(paramBegin, paramBegin, paramEnd,
                                          immBegin, immBegin, immEnd));
    end = NodeIterator(new DDGEntryIter(paramBegin, paramEnd, paramEnd,
                                        immBegin, immEnd, immEnd));

    

}


void DDG::exitNodes(NodeIterator &begin, NodeIterator &end) {
    NodeIterator returnBegin(new FormalReturnSetIter(formalReturnNodes_.begin()));
    NodeIterator returnEnd(new FormalReturnSetIter(formalReturnNodes_.end()));

    NodeIterator deadBegin(new NodeIteratorSet(deadDefinitions_.begin()));
    NodeIterator deadEnd(new NodeIteratorSet(deadDefinitions_.end()));


    begin = NodeIterator(new DDGEntryIter(returnBegin, returnBegin, returnEnd,
                                          deadBegin, deadBegin, deadEnd));
    end = NodeIterator(new DDGEntryIter(returnBegin, returnEnd, returnEnd,
                                        deadBegin, deadEnd, deadEnd));
}

void DDG::insertFormalParamNode(Node::Ptr node) {
    FormalParamNode::Ptr formal = dynamic_pointer_cast<FormalParamNode>(node);

    // Currently we should only insert a formal parameter...
    assert(formal);
    formalParamNodes_.insert(formal);
}

void DDG::insertFormalReturnNode(Node::Ptr node) {
    FormalReturnNode::Ptr formal = dynamic_pointer_cast<FormalReturnNode>(node);

    // Currently we should only insert a formal parameter...
    assert(formal);
    formalReturnNodes_.insert(formal);
}

void DDG::insertActualParamNode(Node::Ptr node) {
    ActualParamNode::Ptr actual = dynamic_pointer_cast<ActualParamNode>(node);

    assert(actual);
    actualParamNodes_[actual->addr()].insert(actual);
}


void DDG::insertActualReturnNode(Node::Ptr node) {
    ActualReturnNode::Ptr actual = dynamic_pointer_cast<ActualReturnNode>(node);

    assert(actual);
    actualReturnNodes_[actual->addr()].insert(actual);
}

void DDG::insertVirtualEdges() {
    // By the definition of a graph all nodes must be reachable from an entry
    // and must reach an exit node. We initially use formal parameters/formal
    // returns as entry and exit sets. However, this is insufficient in two cases.
    // Entry: the use of immediate values to define a node.
    // Exit: a definition that is killed before function exit. 
    //
    // We get around this by doing a separate pass over the entire graph;
    // nodes with no in-edges have an edge added to "virtEntryNode_"; 
    // nodes with no out-edges have an edge added to "virtExitNode_".

    NodeIterator b, e;
    allNodes(b, e);
    for(; b != e; ++b) {
        if (!(*b)->hasInEdges() && !(dynamic_pointer_cast<FormalParamNode>(*b))) {
            immediateDefinitions_.insert(*b);
        }
        if (!(*b)->hasOutEdges() && (!dynamic_pointer_cast<FormalReturnNode>(*b))) {
            deadDefinitions_.insert(*b);
        }
    }
}

// Return a (derived) graph in which all nodes that do not reach a formal return node are removed. 

DDG::Ptr DDG::removeDeadNodes() {
    DDG::Ptr ret = DDG::createGraph(func_);

    // We build the graph backwards. Basically, this is the
    // union of all backward slices from formal return nodes...

    NodeIterator fB, fE;
    formalReturnNodes(fB, fE);

    std::set<Node::Ptr> visited;
    std::queue<Node::Ptr> worklist;
    std::map<Node::Ptr, Node::Ptr> copies;

    for (; fB != fE; ++fB) {
        Node::Ptr newNode = (*fB)->copy();
        copies[*fB] = newNode;
        worklist.push(*fB);
    }

    while (!worklist.empty()) {
        Node::Ptr cur = worklist.front();
        worklist.pop();

        if (visited.find(cur) != visited.end()) {
            continue;
        }
        visited.insert(cur);

        Node::Ptr newCur = copies[cur];

        // Insert this node into the graph and add its children to
        // the worklist.
        
        NodeIterator b, e;
        cur->ins(b, e);
 
        // Handle special node types...
        if (dynamic_pointer_cast<FormalReturnNode>(cur)) {
            ret->insertFormalReturnNode(newCur);
        }
        else if (dynamic_pointer_cast<FormalParamNode>(cur)) {
            ret->insertFormalParamNode(newCur);
        }
        else if (dynamic_pointer_cast<ActualReturnNode>(cur)) {
            ret->insertActualReturnNode(newCur);
        }
        else if (dynamic_pointer_cast<ActualParamNode>(cur)) {
            ret->insertActualParamNode(newCur);
        }
       
        for (; b != e; ++b) {
            Node::Ptr next = *b;
            Node::Ptr newNext = copies[next];
            if (!newNext) {
                newNext = next->copy();
                copies[next] = newNext;
            }
            worklist.push(next);
            ret->insertPair(newNext, newCur);
        }
    }

    ret->insertVirtualEdges();
    return ret;
}
