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

#ifndef _PATCH_MODIFIER_H_
#define _PATCH_MODIFIER_H_

#include <set>
#include <vector>
#include "dyntypes.h"

#include "PatchMgr.h"
#include "PatchCommon.h"


namespace Dyninst {
namespace PatchAPI {
   class PatchEdge;
   class PatchBlock;
   class PatchFunction;
   class PatchModifier;

class PATCHAPI_EXPORT InsertedCode {
   friend class PatchModifier;

  public:
  InsertedCode() : entry_(NULL) {}

   typedef boost::shared_ptr<InsertedCode> Ptr;
   PatchBlock *entry() { return entry_; }
   const std::vector<PatchEdge *> &exits() { return exits_;}
   const std::set<PatchBlock *> &blocks() { return blocks_; }
  private:
   
   PatchBlock *entry_;
   std::vector<PatchEdge *> exits_;
   std::set<PatchBlock *> blocks_;
};   
   

class PATCHAPI_EXPORT PatchModifier {
  public:

   static bool redirect(PatchEdge *edge, PatchBlock *target);

   static PatchBlock *split(PatchBlock *, Address, 
                                            bool trust = false, 
                                            Address newlast = (Address)-1);
   
   static bool remove(std::vector<PatchBlock *> &blocks, bool force = false);

   static bool remove(PatchFunction *);

   static InsertedCode::Ptr insert(PatchObject *, SnippetPtr snip, Point *point);
   static InsertedCode::Ptr insert(PatchObject *, void *start, unsigned size);

  private:
   static InsertedCode::Ptr insert(PatchObject *, void *start, unsigned size, Address base);
};

}
}

#endif
   
