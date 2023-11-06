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

#if !defined (_PATCHAPI_PC_ATOM_H_)
#define _PATCHAPI_PC_ATOM_H_

#include <string>

#include "dyn_register.h"
#include "Widget.h"
#include "dataflowAPI/h/Absloc.h"

class block_instance;
class func_instance;

namespace Dyninst {
namespace Relocation {

class PCWidget : public Widget {
 public:
   typedef boost::shared_ptr<PCWidget> Ptr;

   static Ptr create(InstructionAPI::Instruction insn,
		     Address addr,
		     Absloc a,
		     Address thunk = 0);
   virtual bool generate(const codeGen &, const RelocBlock *, CodeBuffer &);

   TrackerElement *tracker(const RelocBlock *t) const;

   virtual ~PCWidget() {}
   virtual std::string format() const;
   virtual unsigned size() const { return insn_.size(); }
   virtual Address addr() const { return addr_; }
   virtual InstructionAPI::Instruction insn() const { return insn_; }

 private:
   PCWidget(InstructionAPI::Instruction insn,
	 Address addr,
	 Absloc &a,
	 Address thunkAddr = 0) : 
   insn_(insn), 
     addr_(addr), 
     a_(a),
     thunkAddr_(thunkAddr) {}


   bool PCtoReturnAddr(const codeGen &templ, const RelocBlock *, CodeBuffer &);
   bool PCtoReg(const codeGen &templ, const RelocBlock *, CodeBuffer &);

   InstructionAPI::Instruction insn_;
   Address addr_;
   Absloc a_;

   Address thunkAddr_;
};

struct IPPatch : public Patch {
  typedef enum {
    Push, 
    Reg } Type;
 IPPatch(Type a, Address b, InstructionAPI::Instruction c,
	 block_instance *d, func_instance *e) : 
  type(a), addr(b), reg((Register)-1), 
    thunk(0), 
    insn(c), block(d), func(e) {}
 IPPatch(Type a, Address b, Register c, Address d,
	 InstructionAPI::Instruction e, block_instance *f, func_instance *g) :
  type(a), addr(b), reg(c), thunk(d), 
    insn(e), block(f), func(g) {}

  virtual bool apply(codeGen &gen, CodeBuffer *buf);
  virtual unsigned estimate(codeGen &templ);
  virtual ~IPPatch() {}
  
  Type type;
  Address addr;
  Register reg;
  Address thunk;
  // Necessary for live registers
  InstructionAPI::Instruction insn;
  block_instance *block;
  func_instance *func;
};


}
}
#endif
