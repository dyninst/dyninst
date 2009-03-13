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

// Node class implementation

#include "Graph.h"
#include "Absloc.h"
#include "Edge.h"
#include "Node.h"
#include <assert.h>

#include "boost/lexical_cast.hpp"



// Nodes are quite simple; they have an Insn, an Absloc, and a set of Edges.

using namespace Dyninst;
using namespace Dyninst::DDG;

const Address Node::VIRTUAL_ADDR = (Address) -1;

Node::Ptr InsnNode::createNode(Address addr, InsnPtr insn, AbslocPtr absloc) {
    return Node::Ptr(new InsnNode(addr, insn, absloc)); 
}

Node::Ptr ParameterNode::createNode(AbslocPtr absloc) {
    return Node::Ptr(new ParameterNode(absloc)); 
}

Node::Ptr VirtualNode::createNode() {
    return Node::Ptr(new VirtualNode());
}

bool Node::returnEdges(const EdgeSet &local,
                       EdgeSet &ret) const {
    // Insert all edges in the "local" set into the
    // "ret" set.
    if (local.size() == 0) return false;

    ret.insert(local.begin(), local.end());
    return true;
}

std::string InsnNode::name() const {
    char buf[256];
    sprintf(buf,"N_0x%lx_%s_",
            addr(), absloc()->name().c_str());
    return std::string(buf);
}

std::string ParameterNode::name() const {
    char buf[256];
    sprintf(buf, "N_PARAM_%s_",
            absloc()->name().c_str());
    return std::string(buf);
}

std::string VirtualNode::name() const {
    return std::string("N_VIRTUAL");
}
