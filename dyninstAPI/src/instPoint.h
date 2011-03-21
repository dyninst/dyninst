/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

// $Id: instPoint.h,v 1.49 2008/09/08 16:44:04 bernat Exp $
// Defines class instPoint

#ifndef _INST_POINT_H_
#define _INST_POINT_H_

#include <stdlib.h>
#include "common/h/Types.h"
#include "dyninstAPI/src/inst.h"
#include "common/h/arch.h" // instruction
#include "dyninstAPI/src/codeRange.h"
#include "common/h/stats.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/bitArray.h"

#include "dyninstAPI/src/baseTramp.h" // iterator

#include "arch-forward-decl.h" // instruction


class int_block;
class int_function;
class int_edge;
class baseTramp;

class instPoint;

namespace Dyninst {
   namespace ParseAPI {
      class Block;
   };
};

#if defined(cap_instruction_api)
#include "Instruction.h"
#include "InstructionDecoder.h"
#endif

class instPoint {
    friend class int_function;
    friend class int_block;
    friend class int_edge;

  public:
// Types of points
    typedef enum {
       None,
       FunctionEntry,
       FunctionExit,
       BlockEntry,
       Edge,
       PreInsn,
       PostInsn,
       PreCall,
       PostCall,
       OtherPoint
    } Type;

    typedef std::list<miniTramp *> Minitramps;

    typedef Minitramps::iterator iterator;
    typedef Minitramps::const_iterator const_iterator;

    // Creation methods are explicitly private, since you should never
    // be creating an instPoint without some element of the CFG to tie
    // it to - and all the CFG elements are friends. 
  private:

    // Function, block, instruction
    static instPoint *funcEntry(int_function *);
    static instPoint *funcExit(int_block *);
    static instPoint *blockEntry(int_block *);
    static instPoint *edge(int_edge *);
    static instPoint *preInsn(int_block *, InstructionAPI::Instruction::Ptr, Address, bool trusted = false);
    static instPoint *postInsn(int_block *, InstructionAPI::Instruction::Ptr, Address, bool trusted = false);
    static instPoint *preCall(int_block *);
    static instPoint *postCall(int_block *);
    // Abrupt end? Should just be edge instrumentation, right?
    
    instPoint(Type t, int_function *);
    instPoint(Type t, int_block *b);
    instPoint(Type t, int_edge *e);
    instPoint(Type t, int_block *b, InstructionAPI::Instruction::Ptr i, Address a);

    static instPoint *fork(instPoint *parent, AddressSpace *as);

    ~instPoint();


 public:
    
    iterator begin();
    iterator end();

    const_iterator begin() const;
    const_iterator end() const;
    baseTramp *tramp();

    AddressSpace *proc() const;
    int_function *func() const;
    // These are optional
    int_block *block() const { return block_; }
    int_edge *edge() const { return edge_; }

    // I'm commenting this out so that we don't reinvent the wheel. 
    // instPoints have two types of addresses. The first is "instrument
    // before or after the insn at this addr". The second is a best guess
    // as to the _next_ address that will execute. 

    //Address addr() const;
    Address insnAddr() const { return addr_; }
    InstructionAPI::Instruction::Ptr insn() const { return insn_; }

    // This is for address tracking... if we're between
    // blocks (e.g., post-call, function exit, or edge
    // instrumentation) and thus aren't strongly tied to 
    // a block give us the next block that will execute. 
    // Unlike block() above, this always works. 
    int_block *nextExecutedBlock() const;
    Address nextExecutedAddr() const;
    
    Type type() const { return type_; }

    miniTramp *push_front(AstNodePtr ast);
    miniTramp *push_back(AstNodePtr ast);
    miniTramp *insert(iterator, AstNodePtr);
    miniTramp *insert(callOrder order, AstNodePtr);
    void erase(iterator);
    void erase(miniTramp *);
    bool empty() const;

    bitArray liveRegisters();

    void setRecursionGuard(bool b) { recursive_ = b; }
    bool recursionGuard() { return recursive_; }

 private:
    void markModified();

    Type type_;

    int_function *func_;
    int_block *block_;
    int_edge *edge_;
    InstructionAPI::Instruction::Ptr insn_;
    Address addr_;
    
    Minitramps tramps_;
    
    bitArray liveRegs_;

    void calcLiveness();
    // Will fill in insn if it's NULL-equivalent
    static bool checkInsn(int_block *, 
                          InstructionAPI::Instruction::Ptr &insn,
                          Address a);

    bool recursive_; 

    baseTramp *baseTramp_;
};

#endif /* _INST_POINT_H_ */
