/*
 * Copyright (c) 1996-2011 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
 
#if !defined(DDG_GRAPH_H)
#define DDG_GRAPH_H

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

class BPatch_function;

namespace Dyninst {
    class InstructionAPI::Instruction;
    class Edge;
    class Graph;
    class Node;

namespace DepGraphAPI {

    class Absloc;
    class DDGAnalyzer;

class DDG : public Graph {
    friend class DDGAnalyzer;
 public:
    typedef dyn_detail::boost::shared_ptr<DDG> Ptr;

 protected:
    typedef BPatch_function Function;

    typedef std::set<FormalReturnNode::Ptr> FormalReturnNodeSet;
    typedef std::set<FormalParamNode::Ptr> FormalParamNodeSet;

    typedef std::map<Address, std::set<Node::Ptr> > ActualParamNodeMap;
    typedef std::map<Address, std::set<Node::Ptr> > ActualReturnNodeMap;

 public:

    static DDG::Ptr analyze(Function *func);
    
    virtual ~DDG() {};

    void formalParamNodes(NodeIterator &begin, NodeIterator &end);
    void formalReturnNodes(NodeIterator &begin, NodeIterator &end);

    void immediateDefinitions(NodeIterator &begin, NodeIterator &end);
    void deadDefinitions(NodeIterator &begin, NodeIterator &end);

    virtual void entryNodes(NodeIterator &begin, NodeIterator &end);
    virtual void exitNodes(NodeIterator &begin, NodeIterator &end);

    bool actualParamNodes(Address call, NodeIterator &begin, NodeIterator &end);
    bool actualReturnNodes(Address call, NodeIterator &begin, NodeIterator &end);

    static Ptr createGraph(Function *func) { return DDG::Ptr(new DDG(func)); }

    virtual void removeAnnotation();

    DDG::Ptr removeDeadNodes();
    
 private:

    void insertFormalParamNode(Node::Ptr node);
    void insertFormalReturnNode(Node::Ptr node);
    void insertActualParamNode(Node::Ptr node);
    void insertActualReturnNode(Node::Ptr node);

    void insertVirtualEdges();

    DDG(Function *func);

    // Assertion: only parameter nodes will have no in-edges,
    // by definition.
    FormalParamNodeSet formalParamNodes_;

    // Virtual nodes to represent locations defined by the function.
    FormalReturnNodeSet formalReturnNodes_;

    // Definitions that use nothing (and therefore don't have a path
    // to a formal parameter)
    NodeSet immediateDefinitions_;

    // Definitions that are killed before any uses
    NodeSet deadDefinitions_;

    ActualParamNodeMap actualParamNodes_;
    ActualReturnNodeMap actualReturnNodes_;


    Function *func_;
};

};
}
#endif
