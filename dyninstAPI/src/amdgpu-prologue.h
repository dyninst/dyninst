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

#ifndef AMDGPU_PROLOGUE_H
#define AMDGPU_PROLOGUE_H

#include "BPatch_snippet.h"
#include "ast.h"
#include "common/src/dyn_register.h"
#include "patchAPI/h/Snippet.h"

// The prologue loads a register pair with address of the buffer containing instrumentation
// variables.
// PatchAPI snippet is used to insert the following prologue:
//
// Example when dest = 94, base = 4, offset = 0xabc; address_of_buffer is at s[4:5] + 0xabc.
// Load it into s[94:95].
//
// s_load_dwordx2 s[94:95], s[4:5], 0xabc
// s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
//
class AmdgpuPrologue : public Dyninst::PatchAPI::Snippet {
public:
  AmdgpuPrologue(Dyninst::Register dest, Dyninst::Register base, unsigned offset)
      : dest_(dest), base_(base), offset_(offset) {}

  bool generate(Dyninst::PatchAPI::Point *point, Dyninst::Buffer &buffer);

private:
  Dyninst::Register dest_;
  Dyninst::Register base_;
  unsigned offset_;
};

// The AST node for the above PatchAPI snippet.
class AmdgpuPrologueNode : public AstSnippetNode {
public:
  AmdgpuPrologueNode(Dyninst::PatchAPI::SnippetPtr p) : AstSnippetNode(p) {}
};

// BPatch_snippet that uses the above AST node.
class AmdgpuPrologueSnippet : public BPatch_snippet {
public:
  AmdgpuPrologueSnippet(const AstNodePtr &p) : BPatch_snippet(p) {}
};

#endif
