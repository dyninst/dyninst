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


// Node class implementation

#include "Graph.h"
#include "Edge.h"
#include "Node.h"
#include "DepGraphNode.h"
#include <assert.h>



#include "BPatch_function.h"

using namespace Dyninst;
using namespace Dyninst::DepGraphAPI;

Node::Ptr OperationNode::createNode(Address addr, Absloc::Ptr absloc) {
    return Node::Ptr(new OperationNode(addr, absloc)); 
}

Node::Ptr OperationNode::copy() {
    return Node::Ptr(new OperationNode(addr(), absloc()));
}

std::string OperationNode::format() const {
    char buf[256];
    sprintf(buf,"N_0x%lx_%s_",
            addr(), absloc()->format().c_str());
    return std::string(buf);
}

Node::Ptr BlockNode::createNode(Block *b) {
    return Node::Ptr(new BlockNode(b));
}

Node::Ptr BlockNode::copy() {
    return Node::Ptr(new BlockNode(block()));
}

std::string BlockNode::format() const {
    char buf[256];
    sprintf(buf, "N_Block_0x%lx_",
            block_->getStartAddress());
    return std::string(buf);
}

BlockNode::BlockNode(Block *b) : Node(), block_(b) {};

Address BlockNode::addr() const {
    return (block_ != NULL ? (Address) block_->getStartAddress() : INVALID_ADDR);
}

Node::Ptr FormalParamNode::createNode(Absloc::Ptr absloc) {
    return Node::Ptr(new FormalParamNode(absloc)); 
}

Node::Ptr FormalParamNode::copy() {
    return Node::Ptr(new FormalParamNode(absloc()));
}


std::string FormalParamNode::format() const {
    char buf[256];
    sprintf(buf, "N_Param_%s_",
            absloc()->format().c_str());
    return std::string(buf);
}

Node::Ptr FormalReturnNode::createNode(Absloc::Ptr absloc) {
    return Node::Ptr(new FormalReturnNode(absloc)); 
}

Node::Ptr FormalReturnNode::copy() {
    return Node::Ptr(new FormalReturnNode(absloc()));
}


std::string FormalReturnNode::format() const {
    char buf[256];
    sprintf(buf, "N_Return_%s_",
            absloc()->format().c_str());
    return std::string(buf);
}


Node::Ptr ActualParamNode::createNode(Address addr,
                                      Function *func,
                                      Absloc::Ptr a) {
    return Node::Ptr(new ActualParamNode(addr, func, a));
}

Node::Ptr ActualParamNode::copy() {
    return Node::Ptr(new ActualParamNode(addr(), func(), absloc()));
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
            absloc()->format().c_str());
    return std::string(buf);
}

Node::Ptr ActualReturnNode::createNode(Address addr,
                                      Function *func,
                                      Absloc::Ptr a) {
    return Node::Ptr(new ActualReturnNode(addr, func, a));
}

Node::Ptr ActualReturnNode::copy() {
    return Node::Ptr(new ActualReturnNode(addr(), func(), absloc()));
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
            absloc()->format().c_str());
    return std::string(buf);
}

Node::Ptr CallNode::createNode(Function *func) {
    return Node::Ptr(new CallNode(func));
}

Node::Ptr CallNode::copy() {
    return Node::Ptr(new CallNode(func()));
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

