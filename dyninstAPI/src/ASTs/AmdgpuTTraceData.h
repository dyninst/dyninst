/*
 * See the dyninst/COPYRIGHT file for copyright information.
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

#ifndef AMDGPU_TTRACEDATA_H
#define AMDGPU_TTRACEDATA_H

#include "BPatch_snippet.h"
#include "ASTs/ast.h"
#include "patchAPI/h/Snippet.h"
#include <cstdint>

namespace Dyninst { namespace DyninstAPI {

// A register-only, NO-CALL marker snippet. Emits `s_mov_b32 m0, <marker>; s_ttracedata` at the point:
// the SQ streams `marker` into the SQTT/ATT hardware thread-trace stream ONLY while a trace session is
// active (host-side), so this is a NOP with zero overhead when tracing is off. Unlike the call-based
// instrumentation path it needs no scratch frame, inferior call, or memory — it rides the base insertion
// path. Prototype simplification: ASSUMES M0 IS DEAD at the insertion point (no save/restore).
class AmdgpuTTraceData : public Dyninst::PatchAPI::Snippet {
public:
  // Constant-marker form: `s_mov_b32 m0, <marker>; s_ttracedata`.
  explicit AmdgpuTTraceData(uint32_t marker) : marker_(marker) {}

  // Arch-register form: `s_mov_b32 m0, <srcReg>; s_ttracedata` — streams a live hardware register
  // (src_reg = AMDGPU SSRC encoding: s0..s101 = 0..101, exec_lo = 126, ...) into the thread trace.
  struct FromReg {};
  AmdgpuTTraceData(FromReg, uint32_t src_reg) : fromReg_(true), srcReg_(src_reg) {}

  bool generate(Dyninst::PatchAPI::Point *point, Dyninst::Buffer &buffer) override;

private:
  uint32_t marker_ = 0;
  bool fromReg_ = false;
  uint32_t srcReg_ = 0;
};

// AST node wrapping the PatchAPI snippet (mirrors AmdgpuPrologueNode).
class AmdgpuTTraceDataNode : public snippetAST {
public:
  explicit AmdgpuTTraceDataNode(Dyninst::PatchAPI::SnippetPtr p) : snippetAST(p) {}
};

// BPatch_snippet wrapping the AST node (mirrors AmdgpuPrologueSnippet).
class AmdgpuTTraceDataSnippet : public BPatch_snippet {
public:
  explicit AmdgpuTTraceDataSnippet(const codeGenASTPtr &p) : BPatch_snippet(p) {}
};

}}

#endif
