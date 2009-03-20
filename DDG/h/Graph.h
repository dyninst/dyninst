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
namespace DDG {

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
    typedef std::set<NodePtr> NodeSet;

    typedef Absloc::Ptr AbslocPtr;
    typedef std::set<AbslocPtr> AbslocSet;
    
    typedef std::map<AbslocPtr, NodePtr> AbslocMap;
    typedef std::map<Address, AbslocMap> NodeMap;
    
    typedef std::map<AbslocPtr, NodePtr> InitialMap;
    
    // Store sufficient information to identify a
    // node (later) - used to record pre-call info.
    typedef std::pair<AbslocPtr, Address> CNode;
    typedef std::set<CNode> CNodeSet;
    typedef std::map<AbslocPtr, CNodeSet> CNodeRec;
    typedef std::map<Address, CNodeRec> CNodeMap;
    
 public:
    
    bool initialNodes(NodeSet &nodes) const;
    bool allNodes(NodeSet &nodes) const;
    
    // We create an empty graph and then add nodes and edges.
    static Ptr createGraph();
    
    // We effectively build the graph by specifying all edges,
    // since it is meaningless to have a disconnected node. 
    void insertPair(NodePtr source, NodePtr target);
    
    // Make a node in this graph. If the node already exists we return
    // it; otherwise we create a new Node and add it to allNodes_ (NOT
    // entryNodes_; that is populated by calls to insertPair above).
    NodePtr makeNode(Dyninst::InstructionAPI::Instruction &instruction,
                     Address addr,
                     AbslocPtr absloc);
    
    // Make a node that represents a parameter; that is, an initial 
    // definition that isn't explicit in the code but must exist.
    NodePtr makeParamNode(Absloc::Ptr a);
    
    // Make a node that represents a phantom "definition" to an
    // immediate value. We do this so that all nodes are reachable
    // from either a parameter node or this "immediate" node.
    NodePtr makeVirtualNode();
    
    bool printDOT(const std::string fileName);
    
    void debugCallInfo();

    const NodeSet entryNodes() const;
    
    // We record a "snapshot" of liveness at each call site; this
    // is used by the interprocedural iterator to hook together
    // the caller and callee. It is trivial to identify the parameters
    // in the callee, but we need to also identify the reaching def
    // sources (definitions) of those parameters.
    void recordCall(Address callAddr,
                    const CNodeRec &callInfo);
    
 private:
    static const Address INITIAL_ADDR;
    
    // Create graph, add nodes.
    Graph();
    
    // We also need to point to all Nodes to keep them alive; we can't 
    // pervasively use shared_ptr within the graph because we're likely
    // to have cycles.
    NodeMap insnNodes_;
    
    // Assertion: only parameter nodes will have no in-edges,
    // by definition.
    AbslocMap parameterNodes_;
    
    NodePtr virtualNode_;
    
    // Keep a snapshot of liveness information at each call
    // site so that we can determine formals later.
    CNodeMap callRecords_;

};
};
}
#endif

