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

#if !defined(GRAPH_H)
#define GRAPH_H

#include "dyntypes.h"
#include "boost/shared_ptr.hpp"
#include <string>
#include <set>
#include <list>
#include <queue>
#include <map>
#include <unordered_set>
#include <unordered_map>

#include "Annotatable.h"
#include "Node.h"


namespace Dyninst {
class Edge;
class Graph;
class Node;
class NodeIterator;
class EdgeIterator;
    
class COMMON_EXPORT Graph : public AnnotatableSparse {
    friend class Edge;
    friend class Node;
    friend class Creator;
    friend class Iterator;
    
 protected:

    typedef boost::shared_ptr<Node> NodePtr;
    typedef boost::shared_ptr<Edge> EdgePtr;

    typedef std::unordered_set<NodePtr, Node::NodePtrHasher> NodeSet;
    typedef std::unordered_map<Address, NodeSet> NodeMap;

 public:    
    typedef boost::shared_ptr<Graph> Ptr;

    class NodePredicate {

    public:
        typedef boost::shared_ptr<NodePredicate> Ptr;
        virtual ~NodePredicate() {}
        virtual bool predicate(const NodePtr &node) = 0;
        static Ptr getPtr(NodePredicate *p) { 
            return Ptr(p);
        }
    };

    typedef bool (*NodePredicateFunc)(const NodePtr &node, void *user_arg);

    virtual void entryNodes(NodeIterator &begin, NodeIterator &end);

    virtual void exitNodes(NodeIterator &begin, NodeIterator &end);
    
    virtual void allNodes(NodeIterator &begin, NodeIterator &end);

    virtual bool find(Address addr, NodeIterator &begin, NodeIterator &end);

    virtual bool find(NodePredicate::Ptr, NodeIterator &begin, NodeIterator &end);
    virtual bool find(NodePredicateFunc, void *user_arg, NodeIterator &begin, NodeIterator &end);

    bool printDOT(const std::string& fileName);

    virtual ~Graph() {}
    
    static Ptr createGraph();
    
    void insertPair(NodePtr source, NodePtr target, EdgePtr edge = EdgePtr());

    virtual void insertEntryNode(NodePtr entry);
    virtual void insertExitNode(NodePtr exit);

    virtual void markAsEntryNode(NodePtr entry);
    virtual void markAsExitNode(NodePtr exit);

    void deleteNode(NodePtr node);

    void addNode(NodePtr node);

    virtual void removeAnnotation() {}

    bool isEntryNode(NodePtr node);
    bool isExitNode(NodePtr node);

    void clearEntryNodes();
    void clearExitNodes();
    void adjustEntryAndExitNodes();

    unsigned size() const;

 protected:
     
    static const Address INITIAL_ADDR;
    
    Graph();

    NodeSet nodes_;
    NodeMap nodesByAddr_;
    NodeSet entryNodes_;
    NodeSet exitNodes_;
};

}
#endif

