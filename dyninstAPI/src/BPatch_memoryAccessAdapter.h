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
#if !defined(BPMAA_H)
#define BPMAA_H

#include "Visitor.h"
#include "Instruction.h"
#include "dyntypes.h"

class BPatch_memoryAccess;

class BPatch_memoryAccessAdapter : public Dyninst::InstructionAPI::Visitor
{
 public:
     BPatch_memoryAccessAdapter() :
        bytes(0), imm(0), ra(-1), rb(-1), sc(0),
        setImm(false) {
  }
  
  virtual ~BPatch_memoryAccessAdapter() {
  }
  
  BPatch_memoryAccess* convert(Dyninst::InstructionAPI::Instruction insn,
			       Dyninst::Address current, bool is64);
  virtual void visit(Dyninst::InstructionAPI::BinaryFunction* b);
  virtual void visit(Dyninst::InstructionAPI::Dereference* d);
  virtual void visit(Dyninst::InstructionAPI::RegisterAST* r);
  virtual void visit(Dyninst::InstructionAPI::Immediate* i);
    private:
        unsigned int bytes;
        long imm;
        int ra;
        int rb;
 		int sc;
        bool setImm;
};

#endif // !defined(BPMAA_H)
