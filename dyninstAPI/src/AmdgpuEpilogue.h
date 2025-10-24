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

#ifndef AMDGPU_EPILOGUE_H
#define AMDGPU_EPILOGUE_H

#include "BPatch_snippet.h"
#include "ast.h"
#include "common/src/dyn_register.h"
#include "patchAPI/h/Snippet.h"

// The epilogue writes back the contents of scalar data cache to corresponding global memory.
// PatchAPI snippet is used to insert the following epilogue:
//
//  s_dcache_wb
//
class AmdgpuEpilogue : public Dyninst::PatchAPI::Snippet {
public:
  bool generate(Dyninst::PatchAPI::Point *point, Dyninst::Buffer &buffer);
};

// The AST node for the above PatchAPI snippet.
class AmdgpuEpilogueNode : public AstSnippetNode {
public:
  AmdgpuEpilogueNode(Dyninst::PatchAPI::SnippetPtr p) : AstSnippetNode(p) {}
};

// BPatch_snippet that uses the above AST node.
class AmdgpuEpilogueSnippet : public BPatch_snippet {
public:
  AmdgpuEpilogueSnippet(const AstNodePtr &p) : BPatch_snippet(p) {}
};

#endif
