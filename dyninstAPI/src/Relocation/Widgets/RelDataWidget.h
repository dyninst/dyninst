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

#if !defined (_PATCHAPI_REL_DATA_ATOM_H_)
#define _PATCHAPI_REL_DATA_ATOM_H_

#include <string>
#include "Widget.h"
class block_instance;

namespace Dyninst {
namespace Relocation {
// Represents generation for a PC-relative
// memory load/store

class RelDataWidget : public Widget {
 public:
   typedef boost::shared_ptr<RelDataWidget> Ptr;

   virtual bool generate(const codeGen &, const RelocBlock *, CodeBuffer &);

   TrackerElement *tracker(const RelocBlock *t) const;
  
   static Ptr create(InstructionAPI::Instruction insn,
                     Address addr,
                     Address target);

   virtual ~RelDataWidget() {}

   virtual std::string format() const;
   virtual unsigned size() const { return insn_.size(); }
   virtual Address addr() const { return addr_; }

 private:
   RelDataWidget(InstructionAPI::Instruction insn,
				 Address addr,
				 Address target) : insn_(insn), addr_(addr), target_(target) {}

   InstructionAPI::Instruction insn_;
   Address addr_;
   Address target_;
   // Read vs. write doesn't matter now but might
   // in the future.
};


struct RelDataPatch : public Patch {
  RelDataPatch(InstructionAPI::Instruction a, Address b, Address o) :
   orig_insn(a), target_addr(b), orig(o) {}
  
  virtual bool apply(codeGen &gen, CodeBuffer *buffer);
  virtual unsigned estimate(codeGen &templ);
  virtual ~RelDataPatch() {}

  void setFunc(func_instance *_func) { func = _func; }
  void setBlock(block_instance *_block) { block = _block; }
  
  InstructionAPI::Instruction orig_insn;
  Address target_addr{};
  Address orig{};

private:
  func_instance *func{};
  block_instance *block{};
};


}
}

#endif
