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
#include "Edge.h"

#include "Absloc.h"

#include "dyntypes.h"

//#include "Graph.h"

class BPatch_function;
class BPatch_basicBlock;

namespace Dyninst {
namespace DepGraphAPI {

class Dyninst::InstructionAPI::Instruction;
class Edge;
class Absloc;
class Graph;


typedef BPatch_function Function;

class Node : public AnnotatableSparse {
    friend class Edge;
    friend class Graph;
 public:
    typedef dyn_detail::boost::shared_ptr<Node> Ptr;
    typedef std::set<Node::Ptr> Set;

    //typedef boost::shared_ptr<InstructionAPI::Instruction> InsnPtr;
    
    typedef InstructionAPI::Instruction InsnPtr; 
    typedef dyn_detail::boost::shared_ptr<Edge> EdgePtr;
    typedef Absloc::Ptr AbslocPtr;
    typedef Edge::Set EdgeSet;
    typedef dyn_detail::boost::shared_ptr<Graph> GraphPtr;

    bool ins(EdgeSet &edges) const { return returnEdges(ins_, edges); }
    bool outs(EdgeSet &edges) const { return returnEdges(outs_, edges); }
    
    bool hasInEdges() const { return ins_.size() != 0; }
    bool hasOutEdges() const { return ins_.size() != 0; }
    
    virtual Address addr() const { return INVALID_ADDR; }
    
    virtual std::string format() const = 0;

    //virtual Node::Ptr copyTo(GraphPtr graph) = 0;
    
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

    static const Address INVALID_ADDR;
};
 
class PhysicalNode : public Node {
    typedef dyn_detail::boost::shared_ptr<PhysicalNode> Ptr;
    
 public:
    static Node::Ptr createNode(Address addr);
    
    virtual Address addr() const { return addr_; }
    
    virtual std::string format() const;
    
    virtual bool isVirtual() const { return false; }
    
    virtual ~PhysicalNode() {};
    
    // virtual Node::Ptr copyTo(GraphPtr graph);

 protected:
    PhysicalNode(Address addr) : addr_(addr) {};
    
    Address addr_; 
};

class VirtualNode : public Node {
    friend class Edge;
    friend class Graph;
    
    
 public:
    typedef dyn_detail::boost::shared_ptr<VirtualNode> Ptr;
    
    static Node::Ptr createNode();
    
    virtual std::string format() const;
    //Node::Ptr copyTo(GraphPtr graph);
    
    virtual bool isVirtual() const { return true; }
    
    virtual ~VirtualNode() {};

 private:
    VirtualNode() {};
};


class OperationNode : public PhysicalNode {
    friend class Edge;
    friend class Graph;
    
    
 public:
    typedef dyn_detail::boost::shared_ptr<OperationNode> Ptr;
    
    static Node::Ptr createNode(Address addr, AbslocPtr absloc);
    
    AbslocPtr absloc() const { return absloc_; }
    Address addr() const { return addr_; }
    
    // We may use "virtual" nodes to represent initial definitions. These
    // have no associated instruction, which we represent as a NULL insn().
    //bool isVirtual() const { return insn(); }
    
    std::string format() const;
    //Node::Ptr copyTo(GraphPtr graph);
    
    bool isVirtual() const { return false; }

    virtual ~OperationNode() {};
    
 private:
    OperationNode(Address addr, AbslocPtr absloc) :
        PhysicalNode(addr), absloc_(absloc) {};
    
    // We keep a "One True List" of abstract locations, so all instructions
    // that define a particular absloc will have the same pointer. 
    AbslocPtr absloc_;
};


class BlockNode : public PhysicalNode {
     friend class Edge;
     friend class Graph;

 typedef BPatch_basicBlock Block;

 public:
    typedef dyn_detail::boost::shared_ptr<BlockNode> Ptr;
    
    static Node::Ptr createNode(Block *);
    
    // We may use "virtual" nodes to represent initial definitions. These
    // have no associated instruction, which we represent as a NULL insn().
    //bool isVirtual() const { return insn(); }
    
    std::string format() const;
    //Node::Ptr copyTo(GraphPtr graph);
    
    bool isVirtual() const { return false; }

    virtual ~BlockNode() {};
    
 private:
    BlockNode(Block *b);
    
    Block *block_;
 };


class FormalNode : public Node {
    friend class Edge;
    friend class Graph;
    
    
 public:
    typedef dyn_detail::boost::shared_ptr<FormalNode> Ptr;
    
    AbslocPtr absloc() const { return absloc_; }
    
    // We may use "virtual" nodes to represent initial definitions. These
    // have no associated instruction, which we represent as a NULL insn().
    //bool isVirtual() const { return insn(); }
    
    virtual std::string format() const = 0;
    //Node::Ptr copyTo(GraphPtr graph) = 0;
    
    virtual bool isVirtual() const { return true; } 
    
    virtual ~FormalNode() {};
    
 protected:
    FormalNode(AbslocPtr a) :
        absloc_(a) {};
    
    AbslocPtr absloc_;
};

class FormalParamNode : public FormalNode {
    friend class Edge;
    friend class Graph;
    
    
 public:
    typedef dyn_detail::boost::shared_ptr<FormalParamNode> Ptr;
    
    static Node::Ptr createNode(AbslocPtr absloc);
    
    virtual std::string format() const;
    // virtual Node::Ptr copyTo(GraphPtr graph);
    
    virtual ~FormalParamNode() {};
    
 private:
    FormalParamNode(AbslocPtr a) :
        FormalNode(a) {};
};


class FormalReturnNode : public FormalNode {
    friend class Edge;
    friend class Graph;
    
    
 public:
    typedef dyn_detail::boost::shared_ptr<FormalReturnNode> Ptr;

    static Node::Ptr createNode(AbslocPtr absloc);
    
    virtual std::string format() const;
    // virtual Node::Ptr copyTo(GraphPtr graph);
    
    virtual ~FormalReturnNode() {};
    
 private:
    FormalReturnNode(AbslocPtr a) :
        FormalNode(a) {};
};


class ActualNode : public Node {
    friend class Edge;
    friend class Graph;
    
    
 public:
    typedef dyn_detail::boost::shared_ptr<ActualNode> Ptr;
    
    Address addr() const { return addr_; }
    AbslocPtr absloc() const { return absloc_; }
    Function *func() const { return func_; }

    virtual std::string format() const = 0;
    // virtual Node::Ptr copyTo(GraphPtr graph);
 
    virtual bool isVirtual() const { return true; }
   
    virtual ~ActualNode() {};
    
 protected:
    ActualNode(Address addr, 
               Function *func,
               AbslocPtr a) :
        addr_(addr),
        func_(func),
        absloc_(a) {};
    
    Address addr_;
    Function *func_;
    AbslocPtr absloc_;
};

class ActualParamNode : public ActualNode {
    friend class Edge;
    friend class Graph;
    
    
 public:
    typedef dyn_detail::boost::shared_ptr<ActualParamNode> Ptr;
    
    static Node::Ptr createNode(Address addr, Function *func, AbslocPtr absloc);
    
    virtual std::string format() const;
    //Node::Ptr copyTo(GraphPtr graph);
    virtual ~ActualParamNode() {};
    
 private:
    ActualParamNode(Address addr, 
                    Function *func,
                    AbslocPtr a) :
        ActualNode(addr, func, a) {};
};

class ActualReturnNode : public ActualNode {
    friend class Edge;
    friend class Graph;
    
    
 public:
    typedef dyn_detail::boost::shared_ptr<ActualReturnNode> Ptr;
    
    static Node::Ptr createNode(Address addr, Function *func, AbslocPtr absloc);
    
    virtual std::string format() const;
    //Node::Ptr copyTo(GraphPtr graph);
    virtual ~ActualReturnNode() {};
    
 private:
    ActualReturnNode(Address addr, 
                     Function *func,
                     AbslocPtr a) :
        ActualNode(addr, func, a) {};
};

class CallNode : public Node {
    friend class Edge;
    friend class Graph;
    

 public:
    typedef dyn_detail::boost::shared_ptr<CallNode> Ptr;

    static Node::Ptr createNode(Function *func);
    //Node::Ptr copyTo(GraphPtr graph);
    
    virtual std::string format() const;
    virtual bool isVirtual() const { return true; }
    virtual ~CallNode() {};
    
 private:
    CallNode(Function *func) : 
        func_(func) {};

    Function *func_;
};

};
}
#endif
