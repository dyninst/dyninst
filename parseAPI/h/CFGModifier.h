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

#ifndef _CFG_MODIFIER_H_
#define _CFG_MODIFIER_H_

#include <vector>
#include "dyntypes.h"

// A collection of methods for user-triggered modification of a ParseAPI CFG. 
// Implemented here so that we can make it a friend class of the CFG; it also
// needs internal information about CFG objects. 

#include "CodeSource.h"


namespace Dyninst {
namespace ParseAPI {
   class Edge;
   class Block;
   class Function;
   class CodeObject;

   class InsertedRegion;

class CFGModifier {
  public:
   // These are all static methods as this class has no state; so really, 
   // it's just a namespace. 

   // Redirect the target of an existing edge. 
   PARSER_EXPORT static bool redirect(Edge *edge, Block *target);

   // Split a block at a provided point.; we double-check whether the address
   // is a valid instruction boundary unless trust is true. 
   // Newlast is the new "last insn" of the original block; provide it if
   // you don't want to waste time disassembling to figure it out.
   PARSER_EXPORT static Block *split(Block *, Address, bool trust = false, Address newlast = -1);
   
   // Parse and add a new region of code to a CodeObject
   // The void * becomes "owned" by the CodeObject, as it's used
   // as a backing store; it cannot be ephemeral.
   // Returns the new entry block. 
   PARSER_EXPORT static InsertedRegion *insert(CodeObject *obj, 
                                               Address base, void *data, 
                                               unsigned size);

   // Remove blocks from the CFG; the block must be unreachable
   // (that is, have no in-edges) unless force is true.
   PARSER_EXPORT static bool remove(std::vector<Block *> &, bool force = false);

   // As the above, but for functions. 
   PARSER_EXPORT static bool remove(Function *);

   // Label a block as the entry of a new function. If the block is already an
   // entry that function is returned; otherwise we create a new function and
   // return it.
   PARSER_EXPORT static Function *makeEntry(Block *);
};

class InsertedRegion : public CodeRegion {
  public:
   
   PARSER_EXPORT InsertedRegion(Address base, void *data, unsigned size, Architecture arch); 
   PARSER_EXPORT virtual ~InsertedRegion();
   
   
   // names: not overriden (as there are no names [yet])
   // findCatchBlock: there isn't one

   // Addresses are provided by the user, as Dyninst etc. have
   // well-known ways of allocating additional code by extending
   // the binary or allocating memory, etc. 
   PARSER_EXPORT Address low() const { return base_; }
   PARSER_EXPORT Address high() const { return base_ + size_; }

   /** InstructionSource implementation **/
   PARSER_EXPORT bool isValidAddress(const Address a) const { 
      return (a >= low() && a < high());
   }
   PARSER_EXPORT void* getPtrToInstruction(const Address a) const {
      if (!isValidAddress(a)) return NULL;
      return (void *)((char *)buf_ + (a - base_));
   }
   PARSER_EXPORT void* getPtrToData(const Address) const {
      return NULL; 
   }
   PARSER_EXPORT bool isCode(const Address a) const { return isValidAddress(a); }
   PARSER_EXPORT bool isData(const Address) const { return false; }
   PARSER_EXPORT bool isReadOnly(const Address) const { return false; }
   PARSER_EXPORT Address offset() const { return base_; }
   PARSER_EXPORT Address length() const { return size_; }
   PARSER_EXPORT unsigned int getAddressWidth() const {
      if (arch_ == Arch_ppc64 || arch_ == Arch_x86_64 || arch_ == Arch_aarch64) return 8;
      else return 4;
   }
   PARSER_EXPORT Architecture getArch() const { return arch_; }

   PARSER_EXPORT bool wasUserAdded() const { return true; }

  private:
    Address base_;
    void *buf_;
    unsigned size_;
    Architecture arch_;
};

}
}

#endif
   
