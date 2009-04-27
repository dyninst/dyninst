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

// DDG Graph class
//
// This class is a container for the DDG graph. We keep indices to
// frequently requested information (e.g., a map of Instructions
// to subnodes), a list of Initial nodes (representing initial
// definitions), a list of Abslocs. 

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

namespace Dyninst {
namespace DepGraphAPI {

    class Dyninst::InstructionAPI::Instruction;
    class Edge;
    class Graph;
    class Absloc;
    class Node;
    
class Graph : public AnnotatableSparse {
    friend class Edge;
    friend class Node;
    friend class Creator;
    friend class Iterator;
    
 public:
    typedef dyn_detail::boost::shared_ptr<Graph> Ptr;

    typedef Node::Ptr NodePtr;
    typedef Node::Set NodeSet;
    
    typedef std::map<Address, NodeSet> NodeMap;
 public:    
    // We create an empty graph and then add nodes and edges.
    static Ptr createGraph();
    
    // We effectively build the graph by specifying all edges,
    // since it is meaningless to have a disconnected node. 
    void insertPair(NodePtr source, NodePtr target);

    void insertEntryNode(NodePtr entry);
    
    // If you want to traverse the graph start here.
    virtual const NodeSet &entryNodes() const { return entryNodes_; };
    
    // Get all nodes in the graph
    virtual const NodeSet &allNodes() const { return nodes_; };

    // Returns a new graph after copying the nodes and edges into this new graph.
    Graph::Ptr copyGraph();
    
    // Returns a set of nodes which are related to the instruction at
    // the given address.
    const NodeSet &getNodesAtAddr(Address addr);

    bool printDOT(const std::string fileName);

    virtual ~Graph() {};
    
 protected:
     
    void addNode(Node::Ptr node);
     
    static const Address INITIAL_ADDR;
    
    // Create graph, add nodes.
    Graph();
    
    // We also need to point to all Nodes to keep them alive; we can't 
    // pervasively use shared_ptr within the graph because we're likely
    // to have cycles.
    NodeSet nodes_;
    
    NodeMap nodesByAddr_;

    NodeSet entryNodes_;
};


class DDG : public Graph {
 public:
    typedef dyn_detail::boost::shared_ptr<DDG> Ptr;

    typedef std::map<Address, NodeSet> AddrMap;
    
    typedef std::set<FormalReturnNode::Ptr> FormalReturnNodeSet;
    typedef std::set<FormalParamNode::Ptr> FormalParamNodeSet;

    typedef std::set<ActualParamNode::Ptr> ActualParamNodeSet;
    typedef std::set<ActualReturnNode::Ptr> ActualReturnNodeSet;


    static Ptr createGraph();
    
    virtual ~DDG() {};

    const FormalParamNodeSet &formalParameterNodes() const { return formalParamNodes_; }
    const FormalReturnNodeSet &formalReturnNodes() const { return formalReturnNodes_; }
    
 private:

    DDG() {};

    // Assertion: only parameter nodes will have no in-edges,
    // by definition.
    FormalParamNodeSet formalParamNodes_;

    // Virtual nodes to represent locations defined by the function.
    FormalReturnNodeSet formalReturnNodes_;

    AddrMap callParamNodes_;
    AddrMap callReturnNodes_;
};


};
}
#endif

