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

#include "ast.h"
#include "common/src/dyn_register.h"
#include "patchAPI/h/Snippet.h"

// Low-level patchAPI snippet. Inserts the following sequence of instructions
// when dest = 94, base = 4, offset = 0xabc :
//
// s_load_dwordx2 s[94:95], s[4:5], 0xabc
// s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
//
class AmdgpuPrologueSnippet : public Dyninst::PatchAPI::Snippet {
public:
  AmdgpuPrologueSnippet(Dyninst::Register dest, Dyninst::Register base,
                        unsigned offset)
      : dest_(dest), base_(base), offset_(offset) {}

  bool generate(Dyninst::PatchAPI::Point *point, Dyninst::Buffer &buffer);

private:
  Dyninst::Register dest_;
  Dyninst::Register base_;
  unsigned offset_;
};

// The AST-based inteface that uses the above snippet.
// TODO : If possible, designate fixed registers for prologue to fit this with
// the register tracking mechanism.
class AmdgpuPrologueSnippetNode : public AstSnippetNode {
public:
  AmdgpuPrologueSnippetNode(Dyninst::PatchAPI::SnippetPtr p)
      : AstSnippetNode(p) {}
};
