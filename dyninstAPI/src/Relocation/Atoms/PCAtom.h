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

#if !defined (_PATCHAPI_PC_ATOM_H_)
#define _PATCHAPI_PC_ATOM_H_

#include "Atom.h"

// Define where the PC value is supposed to go
#include "dataflowAPI/h/Absloc.h"

class block_instance;


namespace Dyninst {
namespace Relocation {

class PCAtom : public Atom {
 public:
   typedef dyn_detail::boost::shared_ptr<PCAtom> Ptr;

   static Ptr create(InstructionAPI::Instruction::Ptr insn,
		     Address addr,
		     Absloc a,
		     Address thunk = 0);
   virtual bool generate(const codeGen &, const Trace *, CodeBuffer &);

   TrackerElement *tracker(block_instance *) const;

   virtual ~PCAtom() {};
   virtual std::string format() const;
   virtual unsigned size() const { return insn_->size(); }
   virtual Address addr() const { return addr_; }
   virtual InstructionAPI::Instruction::Ptr insn() const { return insn_; }

 private:
   PCAtom(InstructionAPI::Instruction::Ptr insn,
	 Address addr,
	 Absloc &a,
	 Address thunkAddr = 0) : 
   insn_(insn), 
     addr_(addr), 
     a_(a),
     thunkAddr_(thunkAddr) {};


   bool PCtoStack(const codeGen &templ, const Trace *, CodeBuffer &);
   bool PCtoReg(const codeGen &templ, const Trace *, CodeBuffer &);

   InstructionAPI::Instruction::Ptr insn_;
   Address addr_;
   Absloc a_;

   Address thunkAddr_;
};

struct IPPatch : public Patch {
  typedef enum {
    Push, 
    Reg } Type;
 IPPatch(Type a, Address b) : 
  type(a), orig_value(b), reg((Register)-1), thunk(0) {};
 IPPatch(Type a, Address b, Register c, Address d) :
  type(a), orig_value(b), reg(c), thunk(d) {};

  virtual bool apply(codeGen &gen, CodeBuffer *buf);
  virtual unsigned estimate(codeGen &templ);
  virtual ~IPPatch() {};
  
  Type type;
  Address orig_value;
  Register reg;
  Address thunk;
};


};
};
#endif
