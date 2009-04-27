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

#include "BPatch_function.h"


// Nodes are quite simple; they have an Insn, an Absloc, and a set of Edges.

using namespace Dyninst;
using namespace Dyninst::DepGraphAPI;

const Address Node::INVALID_ADDR = (Address) -1;

bool Node::returnEdges(const EdgeSet &local,
                       EdgeSet &ret) const {
    // Insert all edges in the "local" set into the
    // "ret" set.
    if (local.size() == 0) return false;

    ret.insert(local.begin(), local.end());
    return true;
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

Node::Ptr OperationNode::createNode(Address addr, AbslocPtr absloc) {
    return Node::Ptr(new OperationNode(addr, absloc)); 
}

std::string OperationNode::format() const {
    char buf[256];
    sprintf(buf,"N_0x%lx_%s_",
            addr(), absloc()->name().c_str());
    return std::string(buf);
}

Node::Ptr BlockNode::createNode(Block *b) {
    return Node::Ptr(new BlockNode(b));
}

std::string BlockNode::format() const {
    char buf[256];
    sprintf(buf, "N_Block_0x%lx_",
            block_->getStartAddress());
    return std::string(buf);
}

BlockNode::BlockNode(Block *b) : PhysicalNode(b->getStartAddress()), block_(b) {};


Node::Ptr FormalParamNode::createNode(AbslocPtr absloc) {
    return Node::Ptr(new FormalParamNode(absloc)); 
}

std::string FormalParamNode::format() const {
    char buf[256];
    sprintf(buf, "N_Param_%s_",
            absloc()->name().c_str());
    return std::string(buf);
}

Node::Ptr FormalReturnNode::createNode(AbslocPtr absloc) {
    return Node::Ptr(new FormalReturnNode(absloc)); 
}

std::string FormalReturnNode::format() const {
    char buf[256];
    sprintf(buf, "N_Return_%s_",
            absloc()->name().c_str());
    return std::string(buf);
}


Node::Ptr ActualParamNode::createNode(Address addr,
                                      Function *func,
                                      AbslocPtr a) {
    return Node::Ptr(new ActualParamNode(addr, func, a));
}

std::string ActualParamNode::format() const {
    char funcname[256];
    if (func()) 
        func()->getName(funcname, 256);
    else
        sprintf(funcname, "<UNKNOWN>");

    char buf[512];

    sprintf(buf, "N_%s_Arg_%s_",
            funcname,
            absloc()->name().c_str());
    return std::string(buf);
}

Node::Ptr ActualReturnNode::createNode(Address addr,
                                      Function *func,
                                      AbslocPtr a) {
    return Node::Ptr(new ActualReturnNode(addr, func, a));
}

std::string ActualReturnNode::format() const {
    char funcname[256];
    if (func()) 
        func()->getName(funcname, 256);
    else
        sprintf(funcname, "<UNKNOWN>");
    char buf[512];

    sprintf(buf, "N_%s_Ret_%s_",
            funcname,
            absloc()->name().c_str());
    return std::string(buf);
}

Node::Ptr CallNode::createNode(Function *func) {
    return Node::Ptr(new CallNode(func));
}



std::string CallNode::format() const {
    char buf[512];
    func_->getName(buf, 512);
    return std::string(buf);
}

#if 0
Node::Ptr OperationNode::copyTo(Graph::Ptr graph) {
  return graph->makeNode(insn_, addr(), absloc());
}

Node::Ptr FormalParamNode::copyTo(Graph::Ptr graph) {
    return graph->makeFormalParamNode(absloc());
}

Node::Ptr FormalReturnNode::copyTo(Graph::Ptr graph) {
    return graph->makeFormalReturnNode(absloc());
}

Node::Ptr ActualParamNode::copyTo(Graph::Ptr graph) {
    return graph->makeCallParamNode(addr(), func(), absloc());
}

Node::Ptr ActualReturnNode::copyTo(Graph::Ptr graph) {
    return graph->makeCallReturnNode(addr(), func(), absloc());
}

Node::Ptr VirtualNode::copyTo(Graph::Ptr graph) {
  return graph->makeVirtualNode();
}

Node::Ptr CallNode::copyTo(Graph::Ptr graph) {
  assert(0 && "Not Supported!");
  return graph->makeVirtualNode();
}

Node::Ptr SimpleNode::copyTo(Graph::Ptr graph) {
  return graph->makeSimpleNode(insn_, addr());
}

#endif
