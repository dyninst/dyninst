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

#include "Point.h"
#include "Relocation/DynPointMaker.h"
#include "Relocation/DynCommon.h"

class block_instance;
class func_instance;
class edge_instance;
class baseTramp;

class instPoint;

namespace Dyninst {
   namespace ParseAPI {
      class Block;
   };
};

using Dyninst::PatchAPI::PatchMgrPtr;

#include "Instruction.h"
#include "InstructionDecoder.h"

class instPoint : public Dyninst::PatchAPI::Point {
  friend class func_instance;
  friend class block_instance;
  friend class edge_instance;
  friend class DynPointMaker;
  public:
    typedef std::list<miniTramp *> Minitramps;
    typedef Minitramps::iterator iterator;
    typedef Minitramps::const_iterator const_iterator;

    // The compleat list of instPoint creation methods
    static instPoint *funcEntry(func_instance *);
    static instPoint *funcExit(func_instance *, block_instance *exitPoint);
    // Now with added context!
    // We can restrict instrumentation to a particular instance of a block, edge,
    // or instruction by additionally specifying a function for context.
    static instPoint *blockEntry(func_instance *, block_instance *);
    static instPoint *blockExit(func_instance *, block_instance *);
    static instPoint *edge(func_instance *, edge_instance *);
    static instPoint *preInsn(func_instance *,
                        block_instance *,
                        Address,
                        InstructionAPI::Instruction::Ptr = InstructionAPI::Instruction::Ptr(),
                        bool trusted = false);
    static instPoint *postInsn(func_instance *,
                        block_instance *, Address,
                        InstructionAPI::Instruction::Ptr = InstructionAPI::Instruction::Ptr(),
                        bool trusted = false);
    static instPoint *preCall(func_instance *,
                              block_instance *);
    static instPoint *postCall(func_instance *,
                               block_instance *);

    static std::pair<instPoint *, instPoint *> getInstpointPair(instPoint *);
    static instPoint *fork(instPoint *parent, AddressSpace *as);
    ~instPoint();

  private:
    instPoint(Address,
              Type,
              PatchMgrPtr,
              func_instance*);
    instPoint(Address,
              Type,
              PatchMgrPtr,
              block_instance*);
    // to be cleared
    instPoint(Address,
              Type,
              PatchMgrPtr,
              edge_instance*,
              func_instance*);
    instPoint(Address,
              Type,
              PatchMgrPtr,
              block_instance*,
              InstructionAPI::Instruction::Ptr,
              func_instance*);

  public:

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    baseTramp *tramp();

    AddressSpace *proc() const;
    func_instance *func() const;
    // These are optional
    block_instance *block() const { return block_; }
    edge_instance *edge()  { return edge_; }

    // I'm commenting this out so that we don't reinvent the wheel.
    // instPoints have two types of addresses. The first is "instrument
    // before or after the insn at this addr". The second is a best guess
    // as to the _next_ address that will execute.

    //Address addr() const;
    Address insnAddr() const { return addr_; }
    InstructionAPI::Instruction::Ptr insn() { return insn_; }

    // This is for address tracking... if we're between
    // blocks (e.g., post-call, function exit, or edge
    // instrumentation) and thus aren't strongly tied to
    // a block give us the next block that will execute.
    // Unlike block() above, this always works.
    block_instance *nextExecutedBlock() const;
    Address nextExecutedAddr() const;

    miniTramp *push_front(AstNodePtr ast, bool recursive);
    miniTramp *push_back(AstNodePtr ast, bool recursive);
    miniTramp *insert(iterator, AstNodePtr, bool recursive);
    miniTramp *insert(callOrder order, AstNodePtr, bool recursive);
    void erase(iterator);
    void erase(miniTramp *);
    bool empty() const;
    unsigned size() const;

    bitArray liveRegisters();

    std::string format() const;

 private:
    void markModified();

    func_instance *func_;
    block_instance *block_;
    edge_instance *edge_;
    InstructionAPI::Instruction::Ptr insn_;
    Minitramps tramps_;

    bitArray liveRegs_;
    void calcLiveness();
    // Will fill in insn if it's NULL-equivalent
    static bool checkInsn(block_instance *,
                          InstructionAPI::Instruction::Ptr &insn,
                          Address a);

    baseTramp *baseTramp_;
};

typedef std::map<Address, instPoint *> InsnInstpoints;

struct BlockInstpoints {
   instPoint *entry;
   instPoint *exit;
   InsnInstpoints preInsn;
   InsnInstpoints postInsn;
   instPoint *preCall;
   instPoint *postCall;
BlockInstpoints() : entry(NULL), exit(NULL), preCall(NULL), postCall(NULL) {};
   ~BlockInstpoints();
};

struct EdgeInstpoints {
   instPoint *point;
EdgeInstpoints() : point(NULL) {};
   ~EdgeInstpoints() { if (point) delete point; };
};

struct FuncInstpoints {
   instPoint *entry;
   std::map<block_instance *, instPoint *> exits;
FuncInstpoints() : entry(NULL) {};
   ~FuncInstpoints();
};



#endif /* _INST_POINT_H_ */
