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


// DDG Nodes consist of an Instruction:Absloc pair. This is guaranteed to
// be a single definition on all platforms. We don't convert to an abstract
// language because we (assume) we need to keep the instruction information
// around, and this is lost in a conversion process. 

#if !defined(DDG_NODE_H)
#define DDG_NODE_H

#include "dyn_detail/boost/shared_ptr.hpp"
#include <set>
#include <string>
#include "Annotatable.h"
#include "Instruction.h"

#include "Absloc.h"
#include "Graph.h"

namespace Dyninst {
namespace DDG {

class Dyninst::InstructionAPI::Instruction;
class Edge;
 
class Absloc;

typedef BPatch_function Function;

class Node : public AnnotatableSparse {
    friend class Edge;
    friend class Graph;
    friend class Creator;
    
 public:
    typedef dyn_detail::boost::shared_ptr<Node> Ptr;
    //typedef boost::shared_ptr<InstructionAPI::Instruction> InsnPtr;
    
    typedef InstructionAPI::Instruction InsnPtr; 
    typedef dyn_detail::boost::shared_ptr<Edge> EdgePtr;
    typedef Absloc::Ptr AbslocPtr;
    typedef std::set<EdgePtr> EdgeSet;
    typedef dyn_detail::boost::shared_ptr<Graph> GraphPtr;
    
    bool ins(EdgeSet &edges) const { return returnEdges(ins_, edges); }
    bool outs(EdgeSet &edges) const { return returnEdges(outs_, edges); }
    
    virtual InsnPtr insn() const { return InsnPtr(); };
    virtual AbslocPtr absloc() const { return AbslocPtr(); };
    virtual Address addr() const { return VIRTUAL_ADDR; };
    
    virtual std::string name() const = 0;

    virtual Node::Ptr copyTo(GraphPtr graph) = 0;
    
    virtual bool isVirtual() const = 0;
    
    virtual ~Node() {};

 protected:
    Node() {};

    EdgeSet ins_;
    EdgeSet outs_;
    
    // Do the work of ins() and outs() above
    bool returnEdges(const EdgeSet &local,
                     EdgeSet &ret) const;
    
    // For Creator-class methods to use: add a new edge to 
    // a node.
    void addInEdge(const EdgePtr in) { ins_.insert(in); }
    void addOutEdge(const EdgePtr out) { outs_.insert(out); }

    static const Address VIRTUAL_ADDR;
};


class InsnNode : public Node {
    friend class Edge;
    friend class Graph;
    friend class Creator;
    
 public:
    typedef dyn_detail::boost::shared_ptr<InsnNode> Ptr;
    //typedef dyn_detail::boost::shared_ptr<InstructionAPI::Instruction> InsnPtr;
    
    static Node::Ptr createNode(Address addr, InsnPtr insn, AbslocPtr absloc);
    
    InsnPtr insn() const { return insn_; }
    AbslocPtr absloc() const { return absloc_; }
    Address addr() const { return addr_; }
    
    // We may use "virtual" nodes to represent initial definitions. These
    // have no associated instruction, which we represent as a NULL insn().
    //bool isVirtual() const { return insn(); }
    
    std::string name() const;
    Node::Ptr copyTo(GraphPtr graph);
    
    bool isVirtual() const { return false; }

    virtual ~InsnNode() {};
    
 private:
    InsnNode(Address addr, InsnPtr insn, AbslocPtr absloc) :
        addr_(addr), insn_(insn), absloc_(absloc) {};
    
    // Instructions don't include their address, so we include this for
    // unique info. We could actually remove and recalculate the Insn
    // based on the address, but I'll add that later. 
    Address addr_;
    
    InsnPtr insn_;
    
    // We keep a "One True List" of abstract locations, so all instructions
    // that define a particular absloc will have the same pointer. 
    AbslocPtr absloc_;
};

class ParameterNode : public Node {
    friend class Edge;
    friend class Graph;
    friend class Creator;
    
 public:
    typedef dyn_detail::boost::shared_ptr<ParameterNode> Ptr;
    
    static Node::Ptr createNode(AbslocPtr absloc);
    
    AbslocPtr absloc() const { return absloc_; }
    
    // We may use "virtual" nodes to represent initial definitions. These
    // have no associated instruction, which we represent as a NULL insn().
    //bool isVirtual() const { return insn(); }
    
    virtual std::string name() const;
    Node::Ptr copyTo(GraphPtr graph);
    
    virtual bool isVirtual() const { return true; } 
    
    virtual ~ParameterNode() {};
    
 private:
    ParameterNode(AbslocPtr a) :
        absloc_(a) {};
    
    AbslocPtr absloc_;
};


class VirtualNode : public Node {
    friend class Edge;
    friend class Graph;
    friend class Creator;
    
 public:
    typedef dyn_detail::boost::shared_ptr<VirtualNode> Ptr;
    
    static Node::Ptr createNode();
    
    virtual std::string name() const;
    Node::Ptr copyTo(GraphPtr graph);
    
    virtual bool isVirtual() const { return true; }
    
    virtual ~VirtualNode() {};

 private:
    VirtualNode() {};
};

class CallNode : public Node {
    friend class Edge;
    friend class Graph;
    friend class Creator;

 public:
    typedef dyn_detail::boost::shared_ptr<CallNode> Ptr;

    static Node::Ptr createNode(Function *func);
    Node::Ptr copyTo(GraphPtr graph);
    
    virtual std::string name() const;
    virtual bool isVirtual() const { return true; }
    virtual ~CallNode() {};
    
 private:
    CallNode(Function *func) : 
        func_(func) {};

    Function *func_;
};

class SimpleNode : public Node {
  friend class Edge;
  friend class Graph;
  friend class Creator;
  
public:
   typedef dyn_detail::boost::shared_ptr<SimpleNode> Ptr;
   
   static Node::Ptr createNode(Address addr, InsnPtr insn);
   
   InsnPtr insn() const { return insn_; }
   Address addr() const { return addr_; }
   
   std::string name() const;
   Node::Ptr copyTo(GraphPtr graph);

   bool isVirtual() const { return false; }

   virtual ~SimpleNode() {};
   
private:
   SimpleNode(Address addr, InsnPtr insn) : addr_(addr), insn_(insn) {};
   
   // Instructions don't include their address, so we include this for
   // unique info. We could actually remove and recalculate the Insn
   // based on the address, but I'll add that later. 
   Address addr_;
   
   InsnPtr insn_;
};

};
}
#endif
