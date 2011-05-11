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

#if !defined(DEP_GRAPH_NODES_H)
#define DEP_GRAPH_NODES_H

#include "Node.h"
#include "Absloc.h"


class BPatch_function;
class BPatch_basicBlock;

namespace Dyninst {

namespace DepGraphAPI {

class Absloc;


typedef BPatch_function Function;

class OperationNode : public PhysicalNode {
 public:
    typedef dyn_detail::boost::shared_ptr<OperationNode> Ptr;
    
    static Node::Ptr createNode(Address addr, Absloc::Ptr absloc);
    
    Absloc::Ptr absloc() const { return absloc_; }
    Address addr() const { return addr_; }
    
    // We may use "virtual" nodes to represent initial definitions. These
    // have no associated instruction, which we represent as a NULL insn().
    //bool isVirtual() const { return insn(); }
    
    std::string format() const;
    
    bool isVirtual() const { return false; }

    virtual ~OperationNode() {};
    virtual Node::Ptr copy();

 
 private:
    OperationNode(Address addr, Absloc::Ptr absloc) :
        PhysicalNode(addr), absloc_(absloc) {};
    
    // We keep a "One True List" of abstract locations, so all instructions
    // that define a particular absloc will have the same pointer. 
    Absloc::Ptr absloc_;
};


class BlockNode : public Node {

 typedef BPatch_basicBlock Block;

 public:
    typedef dyn_detail::boost::shared_ptr<BlockNode> Ptr;
    
    static Node::Ptr createNode(Block *);
    
    // We may use "virtual" nodes to represent initial definitions. These
    // have no associated instruction, which we represent as a NULL insn().
    //bool isVirtual() const { return insn(); }
    
    std::string format() const;
    
    bool isVirtual() const { return false; }

    virtual ~BlockNode() {};
    
    virtual Address addr() const;
    
    virtual Block *block() { return block_; }
    virtual Node::Ptr copy();


 private:
    BlockNode(Block *b);
    
    Block *block_;
 };


class FormalNode : public VirtualNode {
    
    
 public:
    typedef dyn_detail::boost::shared_ptr<FormalNode> Ptr;
    
    Absloc::Ptr absloc() const { return absloc_; }
    
    // We may use "virtual" nodes to represent initial definitions. These
    // have no associated instruction, which we represent as a NULL insn().
    //bool isVirtual() const { return insn(); }
    
    virtual std::string format() const = 0;
        
    virtual ~FormalNode() {};
    
 protected:
    FormalNode(Absloc::Ptr a) :
        absloc_(a) {};
    
    Absloc::Ptr absloc_;
};

class FormalParamNode : public FormalNode {
 public:
    typedef dyn_detail::boost::shared_ptr<FormalParamNode> Ptr;
    
    static Node::Ptr createNode(Absloc::Ptr absloc);
    
    virtual std::string format() const;
    
    virtual ~FormalParamNode() {};
    virtual Node::Ptr copy();

    
 private:
    FormalParamNode(Absloc::Ptr a) :
        FormalNode(a) {};
};


class FormalReturnNode : public FormalNode {
 public:
    typedef dyn_detail::boost::shared_ptr<FormalReturnNode> Ptr;

    static Node::Ptr createNode(Absloc::Ptr absloc);
    
    virtual std::string format() const;
    
    virtual ~FormalReturnNode() {};
    virtual Node::Ptr copy();

    
 private:
    FormalReturnNode(Absloc::Ptr a) :
        FormalNode(a) {};
};


class ActualNode : public VirtualNode {

 public:
    typedef dyn_detail::boost::shared_ptr<ActualNode> Ptr;
    
    Address addr() const { return addr_; }
    Absloc::Ptr absloc() const { return absloc_; }
    Function *func() const { return func_; }

    virtual std::string format() const = 0;
 
    virtual bool isVirtual() const { return true; }
   
    virtual ~ActualNode() {};
    
 protected:
    ActualNode(Address addr, 
               Function *func,
               Absloc::Ptr a) :
        addr_(addr),
        func_(func),
        absloc_(a) {};
    
    Address addr_;
    Function *func_;
    Absloc::Ptr absloc_;
};

class ActualParamNode : public ActualNode {
    
    
 public:
    typedef dyn_detail::boost::shared_ptr<ActualParamNode> Ptr;
    
    static Node::Ptr createNode(Address addr, Function *func, Absloc::Ptr absloc);
    
    virtual std::string format() const;
    virtual ~ActualParamNode() {};
    virtual Node::Ptr copy();

 private:
    ActualParamNode(Address addr, 
                    Function *func,
                    Absloc::Ptr a) :
        ActualNode(addr, func, a) {};
};

class ActualReturnNode : public ActualNode {
    
 public:
    typedef dyn_detail::boost::shared_ptr<ActualReturnNode> Ptr;
    
    static Node::Ptr createNode(Address addr, Function *func, Absloc::Ptr absloc);
    
    virtual std::string format() const;
    virtual ~ActualReturnNode() {};
    virtual Node::Ptr copy();


 private:
    ActualReturnNode(Address addr, 
                     Function *func,
                     Absloc::Ptr a) :
        ActualNode(addr, func, a) {};
};

class CallNode : public VirtualNode {
 public:
    typedef dyn_detail::boost::shared_ptr<CallNode> Ptr;

    static Node::Ptr createNode(Function *func);
    
    virtual std::string format() const;
    virtual ~CallNode() {};
    Function *func() const { return func_; }    
    virtual Node::Ptr copy();


 private:
    CallNode(Function *func) : 
        func_(func) {};

    Function *func_;
};

};
};

#endif
