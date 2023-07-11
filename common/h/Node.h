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

#if !defined(NODE_H)
#define NODE_H

#include <set>
#include <string>
#include <stddef.h>
#include <unordered_set>
#include "Edge.h"
#include "Annotatable.h"

#include "dyntypes.h"
#include "boost/shared_ptr.hpp"


class BPatch_function;
class BPatch_basicBlock;

namespace Dyninst {

class Edge;
class Graph;
class NodeIterator;
class EdgeIterator;

class COMMON_EXPORT Node  {
    friend class Edge;
    friend class Graph;
    
	typedef boost::shared_ptr<Edge> EdgePtr;
	typedef boost::shared_ptr<Graph> GraphPtr;
    typedef std::unordered_set<EdgePtr, Edge::EdgePtrHasher> EdgeSet;

 public:
	 typedef boost::shared_ptr<Node> Ptr;
	 struct NodePtrHasher {
	     size_t operator() (const Ptr &n) const noexcept {
	         return (size_t)n.get();
	     }
	 };

    void ins(EdgeIterator &begin, EdgeIterator &end);
    void outs(EdgeIterator &begin, EdgeIterator &end);

    void ins(NodeIterator &begin, NodeIterator &end);
    void outs(NodeIterator &begin, NodeIterator &end);

    bool hasInEdges(); 
    bool hasOutEdges();

    void deleteInEdge(EdgeIterator e);
    void deleteOutEdge(EdgeIterator e);

    void forwardClosure(NodeIterator &begin, NodeIterator &end);
    void backwardClosure(NodeIterator &begin, NodeIterator &end);

    GraphPtr forwardSubgraph();
    GraphPtr backwardSubgraph();
    
    virtual Address addr() const { return INVALID_ADDR; }
    
    virtual std::string format() const = 0;

    virtual Node::Ptr copy() = 0;
    
    virtual bool isVirtual() const = 0;
    
    virtual ~Node() {}

    // DOT output methods...
    virtual std::string DOTshape() const;
    virtual std::string DOTrank() const;
    virtual std::string DOTname() const;
    virtual bool DOTinclude() const { return true; }

 protected:
    Node() {}

    EdgeSet ins_;
    EdgeSet outs_;
    
    void addInEdge(const EdgePtr in);
    void addOutEdge(const EdgePtr out);

    static const Address INVALID_ADDR;
};
 
class COMMON_EXPORT PhysicalNode : public Node {
public:
	typedef boost::shared_ptr<PhysicalNode> Ptr;
     
    static Node::Ptr createNode(Address addr);
    
    virtual Address addr() const { return addr_; }
    
    virtual std::string format() const;
    
    virtual bool isVirtual() const { return false; }
    
    virtual ~PhysicalNode() {}
    
    virtual Node::Ptr copy();

 protected:
    PhysicalNode(Address addr) : addr_(addr) {}
    
    Address addr_; 
};

class  COMMON_EXPORT VirtualNode : public Node {
    friend class Edge;
    friend class Graph;

 public:
    typedef boost::shared_ptr<VirtualNode> Ptr;
    
    static Node::Ptr createNode();
    static Node::Ptr createNode(std::string name); 

    virtual std::string format() const;
    virtual Node::Ptr copy();
    
    virtual bool isVirtual() const { return true; }
    
    virtual  ~VirtualNode() {}

    VirtualNode(std::string name) : name_(name) {}
    VirtualNode() : name_(defaultName) {}

    static std::string defaultName;

    // DOT output methods...
    virtual std::string DOTshape() const;
 private:
    std::string name_;
};

 class NodeIteratorImpl;

class COMMON_EXPORT NodeIterator {
    friend class Node;
    friend class Graph;
    friend class Edge;

 public:

    NodeIterator &operator++();
    NodeIterator operator++(int);

    NodeIterator &operator--();
    NodeIterator operator--(int);

    Node::Ptr operator*() const;

    bool operator==(const NodeIterator &rhs) const;
    bool operator!=(const NodeIterator &rhs) const;

    NodeIterator &operator=(const NodeIterator &rhs);


    // For code such as:
    // NodeIterator begin, end;
    // graph->entryNodes(begin, end);
    NodeIterator();

    // Main constructor
    // The iter parameter becomes owned by the iterator and will be destroyed
    // when the iterator is destroyed.
    NodeIterator(NodeIteratorImpl *iter);

    // Aaaand let's _really_ not forget the copy constructor
    NodeIterator(const NodeIterator &rhs);

    ~NodeIterator();

 protected:

    // We hide the internal iteration behavior behind a pointer. 
    // This allows us to override (yay for virtual functions).
    NodeIteratorImpl *iter_;

    NodeIteratorImpl *copy() const;

};

}
#endif
