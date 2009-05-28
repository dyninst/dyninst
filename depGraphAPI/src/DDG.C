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

#include "dyn_detail/boost/shared_ptr.hpp"
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

DDG::DDG() :
    virtEntryNode_(VirtualNode::createNode())
{}


DDG::Ptr DDG::analyze(BPatch_function *func) {
    DDGAnalyzer ddgA(func);

    DDG::Ptr ddg = ddgA.analyze();

    return ddg;
}

void DDG::insertEntryNode(NodePtr entry) {
    FormalParamNode::Ptr formal = dyn_detail::boost::dynamic_pointer_cast<FormalParamNode>(entry);

    // Currently we should only insert a formal parameter...
    assert(formal);
    formalParamNodes_.insert(formal);
}


void DDG::formalParameterNodes(NodeIterator &begin, NodeIterator &end) {
    begin = NodeIterator(new FormalParamSetIter(formalParamNodes_.begin()));
    end = NodeIterator(new FormalParamSetIter(formalParamNodes_.end()));
}

void DDG::formalReturnNodes(NodeIterator &begin, NodeIterator &end) {
    begin = NodeIterator(new FormalReturnSetIter(formalReturnNodes_.begin()));
    end = NodeIterator(new FormalReturnSetIter(formalReturnNodes_.end()));
}

bool DDG::actualParamNodes(Address call, NodeIterator &begin, NodeIterator &end) {
    // We use call+1 to make sure it's unique...
    Address addr = call+1;
    NodeMap::iterator iter = callParamNodes_.find(addr);
    if (iter == callParamNodes_.end()) return false;

    begin = NodeIterator(new NodeIteratorSet(iter->second.begin()));
    end = NodeIterator(new NodeIteratorSet(iter->second.end()));
    return true;
}


bool DDG::actualReturnNodes(Address call, NodeIterator &begin, NodeIterator &end) {
    // We use call+1 to make sure it's unique...
    Address addr = call+1;
    NodeMap::iterator iter = callReturnNodes_.find(addr);
    if (iter == callReturnNodes_.end()) return false;

    begin = NodeIterator(new NodeIteratorSet(iter->second.begin()));
    end = NodeIterator(new NodeIteratorSet(iter->second.end()));
    return true;
}

void DDG::entryNodes(NodeIterator &begin, NodeIterator &end) {
    // This is a little more tricky... we want to iterate over all
    // of the formal parameter nodes, then over the children of the
    // virtual node (if any exist)
    NodeIterator virtBegin, virtEnd;
    virtEntryNode_->outs(virtBegin, virtEnd);
    NodeIterator paramBegin(new FormalParamSetIter(formalParamNodes_.begin()));
    NodeIterator paramEnd(new FormalParamSetIter(formalParamNodes_.end()));

    begin = NodeIterator(new DDGEntryIter(paramBegin, paramBegin, paramEnd,
                                          virtBegin, virtBegin, virtEnd));
    end = NodeIterator(new DDGEntryIter(paramBegin, paramEnd, paramEnd,
                                        virtBegin, virtEnd, virtEnd));

    

}
