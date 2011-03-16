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

// DDG edges are simple: in node, out node, and a type. We can extend
// this as desired, although my prediction is that we will instead be
// heavily annotating with user data. 

#if !defined(DDG_ABSLOC_H)
#define DDG_ABSLOC_H

#include <values.h>

#include "dyn_detail/boost/shared_ptr.hpp"

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

namespace DepGraphAPI {
 
class Absloc : public AnnotatableSparse {
    // Temporary typedef...
    typedef BPatch_function Function;
    
 public:
    // Get a list of all abstract locations currently defined
    // by the graph.
    typedef dyn_detail::boost::shared_ptr<Absloc> Ptr;
    typedef std::set<Ptr> AbslocSet;

    static void getAbslocs(AbslocSet &locs);
    
    virtual std::string format() const = 0;

    virtual ~Absloc() {};

    // Get the set of Abslocs that may "contain" the current one; that is,
    // may be included in a use or definition. For example, the "Stack"
    // absloc contains any specific stack slot.
    virtual AbslocSet getAliases() const = 0;
    virtual bool isPrecise() const = 0;
 protected:


    Absloc() {};

};

class RegisterLoc : public Absloc {

 public:
    typedef std::map<InstructionAPI::RegisterAST, RegisterLoc::Ptr> RegisterMap;
    typedef dyn_detail::boost::shared_ptr<RegisterLoc> Ptr;
    

    virtual ~RegisterLoc() {};
    virtual std::string format() const { return reg_->format(); }
    static void getRegisterLocs(AbslocSet &locs);

    static Absloc::Ptr getRegLoc(const InstructionAPI::RegisterAST::Ptr reg);

    // We have precise information about all registers.
    virtual AbslocSet getAliases() const { return AbslocSet(); }
    virtual bool isPrecise() const { return true; }
    const InstructionAPI::RegisterAST::Ptr getReg() { return reg_; }

    // Convenience methods
    bool isSP() const;
    bool isPC() const; 
    bool isFlag() const;

    static bool isSP(InstructionAPI::RegisterAST::Ptr reg);
    static bool isPC(InstructionAPI::RegisterAST::Ptr reg);
    static bool isFlag(InstructionAPI::RegisterAST::Ptr reg);

    // More convenience functions. InstructionAPI doesn't mention the
    // PC unless it's explicitly set (branch) or used (call)... thus,
    // we need to fake it. 
    static Absloc::Ptr makePC();

 private:
    RegisterLoc(const InstructionAPI::RegisterAST::Ptr reg) : reg_(reg) {};
    
    const InstructionAPI::RegisterAST::Ptr reg_;
    static RegisterMap allRegLocs_;
    static Absloc::Ptr pc_;
};

class StackLoc : public Absloc {
 public:
    typedef std::map<std::pair<int, int>, StackLoc::Ptr> StackMap;
    typedef dyn_detail::boost::shared_ptr<StackLoc> Ptr;

    int slot() const { return slot_; }
    int region() const { return region_; }

    virtual ~StackLoc() {};
    virtual std::string format() const;

    static void getStackLocs(AbslocSet &locs);

    static Absloc::Ptr getStackLoc(int slot, int region);
    static Absloc::Ptr getStackLoc();

    virtual AbslocSet getAliases() const;
    virtual bool isPrecise() const { return slot_ != STACK_GLOBAL; }

    static const int STACK_GLOBAL;
    static const int STACK_REGION_DEFAULT;
 private:
    StackLoc(const int stackOffset, const int stackRegion) : 
        slot_(stackOffset),
        region_(stackRegion) 
        {};
    StackLoc() : slot_(STACK_GLOBAL), region_(STACK_REGION_DEFAULT) {};

    const int slot_;
    const int region_;

    static StackMap stackLocs_;
};

class MemLoc : public Absloc {
 public:
    typedef dyn_detail::boost::shared_ptr<MemLoc> Ptr;
    typedef std::map<Address, MemLoc::Ptr> MemMap;
    typedef std::set<MemLoc::Ptr> MemSet;

    virtual ~MemLoc() {};
    virtual std::string format() const;
    static void getMemLocs(AbslocSet &locs);

    // A specific word in memory
    static Absloc::Ptr getMemLoc(Address addr);
    // "Anywhere in memory"
    // The parameter here indicates a unique definition
    // so we can distinguish them.
    static Absloc::Ptr getMemLoc();

    static const Address MEM_GLOBAL;

    virtual AbslocSet getAliases() const;
    virtual bool isPrecise() const { return addr_ != MEM_GLOBAL; }

 private:

    MemLoc(Address addr) : addr_(addr) {};
    MemLoc() : addr_(MEM_GLOBAL) {};

    Address addr_;

    static MemMap memLocs_; 
};

// We now have a virtual absloc for "uses" of immediates. We really
// want every node in the graph to be reachable from a small set
// of well-known nodes (e.g., "parameter" nodes). However, an instruction
// such as "EAX = 5" doesn't use anything and thus isn't reachable from
// a parameter node. We handle this by creating an "Immediate" absloc
// and an "Immediate" node. 

class ImmLoc : public Absloc { 
 public:
    typedef dyn_detail::boost::shared_ptr<ImmLoc> Ptr;
    
    virtual ~ImmLoc() {};
    virtual std::string format() const;
    static void getImmLocs(AbslocSet &) {};

    static Absloc::Ptr getImmLoc();

    virtual AbslocSet getAliases() const { return AbslocSet(); }
    virtual bool isPrecise() const { return true; }

 private:
    ImmLoc() {};

    static ImmLoc::Ptr immLoc_;
};    

};
};

#endif

