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

#if !defined(PATCHAPI_ATOM_H_)
#define PATCHAPI_ATOM_H_

#include "instructionAPI/h/Instruction.h" // Instruction::Ptr
#include <string>
#include <list> // stl::list

class baseTramp;
class codeGen;

namespace Dyninst {

namespace Relocation {

class Transformer;
class Widget;
class RelocInsn;
class Inst;
class CFWidget;
class RelocBlock;

struct Patch;
class TrackerElement;
class CodeTracker;
class CodeBuffer;

// Widget code generation class
class Widget {
  friend class Transformer;
 public:
  typedef boost::shared_ptr<Widget> Ptr;
  typedef boost::shared_ptr<RelocBlock> RelocBlockPtr;

  Widget() {}

  // A default value to make sure things don't go wonky.
  virtual Address addr() const { return 0; }
  virtual unsigned size() const { return 0; }
  virtual InstructionAPI::Instruction insn() const {
    return InstructionAPI::Instruction();
  }

  // Make binary from the thing
  // Current address (if we need it)
  // is in the codeGen object.
  virtual bool generate(const codeGen &templ,
                        const RelocBlock *trace,
                        CodeBuffer &buffer) = 0;

  virtual std::string format() const = 0;

  virtual ~Widget() {}
};

 // A generic code patching mechanism
struct Patch {
   virtual bool apply(codeGen &gen, CodeBuffer *buf) = 0;
   virtual unsigned estimate(codeGen &templ) = 0;
   virtual ~Patch() {}
};


}
}
#endif
