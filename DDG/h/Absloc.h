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

// DDG edges are simple: in node, out node, and a type. We can extend
// this as desired, although my prediction is that we will instead be
// heavily annotating with user data. 

#if !defined(DDG_ABSLOC_H)
#define DDG_ABSLOC_H

#include <values.h>

#include <boost/shared_ptr.hpp>
#include <set>
#include <string>
#include "Annotatable.h"
#include "Register.h"
#include "Result.h"
#include "Instruction.h"

// This is a parent class for all abslocs (abstract locations; e.g., registers/memory/etc)
// used by the DDG. We subclass off this class to provide an Absloc with a particular meaning,
// such as a physical register, stack element, heap element, input or output representation, etc.

class BPatch_function;

namespace Dyninst {
namespace DDG {

class Graph;
class Node;
class Edge;
 
class Absloc : public AnnotatableSparse {
    
    friend class Graph;
    friend class Node;
    friend class Edge;

    // Temporary typedef...
    typedef BPatch_function Function;
    
 public:
    // Get a list of all abstract locations currently defined
    // by the graph.
    typedef boost::shared_ptr<Absloc> Ptr;
    typedef std::set<Ptr> AbslocSet;
    
    static void getAbslocs(AbslocSet &locs);
    virtual std::string name() const = 0;
    
    //static Ptr createAbsloc(const std::string name);
    
    // Translate from an InstructionAPI Expression into an abstract
    // location.
    // We need some sort of "get state" function; for now we're doing
    // Function and address...
    static Absloc::Ptr getAbsloc(const InstructionAPI::Expression::Ptr exp,
                                 Function *func,
                                 Address addr);
    static Absloc::Ptr getAbsloc(const InstructionAPI::RegisterAST::Ptr reg);
    
    virtual ~Absloc() {};

    static void getUsedAbslocs(const InstructionAPI::Instruction insn,
                               Function *func,
                               Address addr,
                               AbslocSet &uses);
    static void getDefinedAbslocs(const InstructionAPI::Instruction insn,
                                  Function *func,
                                  Address addr,
                                  AbslocSet &defs);
    // Get the set of Abslocs that may "contain" the current one; that is,
    // may be included in a use or definition. For example, the "Stack"
    // absloc contains any specific stack slot.
    virtual AbslocSet getAliases() = 0;
 protected:

    static bool convertResultToAddr(const InstructionAPI::Result &res, Address &addr);
    static bool convertResultToSlot(const InstructionAPI::Result &res, int &slot);

    Absloc() {};

    static bool isFramePointer(const InstructionAPI::InstructionAST::Ptr &reg, Function *func, Address addr);
    static bool isStackPointer(const InstructionAPI::InstructionAST::Ptr &reg, Function *func, Address addr);

    static void bindFP(InstructionAPI::InstructionAST::Ptr &reg, Function *func, Address addr);
    static void bindSP(InstructionAPI::InstructionAST::Ptr &reg, Function *func, Address addr);

};

class RegisterLoc : public Absloc {
    friend class Graph;
    friend class Node;
    friend class Edge;

 public:
    typedef std::map<InstructionAPI::RegisterAST, RegisterLoc::Ptr> RegisterMap;

    typedef boost::shared_ptr<RegisterLoc> Ptr;
    virtual ~RegisterLoc() {};
    virtual std::string name() const { return reg_->format(); }
    static void getRegisterLocs(AbslocSet &locs);

    static Absloc::Ptr getRegLoc(const InstructionAPI::RegisterAST::Ptr reg);

    // We have precise information about all registers.
    virtual AbslocSet getAliases() { return AbslocSet(); }

 private:
    RegisterLoc(const InstructionAPI::RegisterAST::Ptr reg) : reg_(reg) {};
    
    const InstructionAPI::RegisterAST::Ptr reg_;
    static RegisterMap allRegLocs_;
};

class StackLoc : public Absloc {
    friend class Graph;
    friend class Node;
    friend class Edge;

 public:
    typedef std::map<int, StackLoc::Ptr> StackMap;

    typedef boost::shared_ptr<StackLoc> Ptr;
    virtual ~StackLoc() {};
    virtual std::string name() const;

    static void getStackLocs(AbslocSet &locs);
    static void getDefinedLocs(AbslocSet &locs);
    static void getBottomLocs(AbslocSet &locs);

    static Absloc::Ptr getStackLoc(int slot);
    static Absloc::Ptr getBottomLoc(Address addr);

    virtual AbslocSet getAliases();

    static const int STACK_GLOBAL;
 private:
    StackLoc(const int stackOffset) : slot_(stackOffset), id_(0) {};
    StackLoc() : slot_(STACK_GLOBAL), id_(0) {};

    const int slot_;

    // If we're a "bottom" location, we need some unique ID. Use
    // the address of the instruction as that ID.
    Address id_;

    static StackMap stackLocs_;
    static std::map<Address, StackLoc::Ptr> bottomLocs_;
};

#if 0
class HeapLoc : public Absloc {
    friend class Graph;
    friend class Node;
    friend class Edge;

 public:
    typedef boost::shared_ptr<HeapLoc> Ptr;
    virtual ~HeapLoc() {};
    virtual std::string name() const { return "HEAP"; }
    static void getHeapLocs(AbslocSet &locs);

    static Absloc::Ptr getHeapLoc(Address addr);

    virtual AbslocSet getAliases();

 private:
    HeapLoc() {};
    
    static Absloc::Ptr heapLoc_;
};
#endif

class MemLoc : public Absloc {
    friend class Graph;
    friend class Node;
    friend class Edge;

 public:
    typedef boost::shared_ptr<MemLoc> Ptr;
    typedef std::map<Address, MemLoc::Ptr> MemMap;
    typedef std::set<MemLoc::Ptr> MemSet;

    virtual ~MemLoc() {};
    virtual std::string name() const;
    static void getMemLocs(AbslocSet &locs);
    static void getDefinedLocs(AbslocSet &locs);
    static void getBottomLocs(AbslocSet &locs);

    // A specific word in memory
    static Absloc::Ptr getMemLoc(Address addr);
    // "Anywhere in memory"
    // The parameter here indicates a unique definition
    // so we can distinguish them.
    static Absloc::Ptr getBottomLoc(Address addr);

    static const Address MEM_GLOBAL;

    virtual AbslocSet getAliases();

 private:

    MemLoc(Address addr) : addr_(addr), id_(0) {};
    MemLoc() : addr_(MEM_GLOBAL), id_(0) {};

    Address addr_;
    Address id_;

    static MemMap memLocs_; 
    static MemMap bottomLocs_;
};

};
}

#endif

