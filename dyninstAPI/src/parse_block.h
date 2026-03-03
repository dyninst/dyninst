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

#ifndef DYNINST_DYNINSTAPI_PARSE_BLOCK_H
#define DYNINST_DYNINSTAPI_PARSE_BLOCK_H

#include "CFG.h"
#include "codeRange.h"
#include "dyntypes.h"

#include <utility>

class parse_func;

class parse_block : public codeRange, public Dyninst::ParseAPI::Block {
public:
  parse_block(parse_func *, Dyninst::ParseAPI::CodeRegion *, Dyninst::Address);
  parse_block(Dyninst::ParseAPI::CodeObject *, Dyninst::ParseAPI::CodeRegion *, Dyninst::Address);

  ~parse_block() = default;

  // cfg access & various predicates
  bool isShared() const {
    return containingFuncs() > 1;
  }

  bool isExitBlock();
  bool isCallBlock();
  bool isIndirectTailCallBlock();
  bool isEntryBlock(parse_func *f) const;

  // func starting with this bock
  parse_func *getEntryFunc() const;

  bool unresolvedCF() const {
    return unresolvedCF_;
  }

  bool abruptEnd() const {
    return abruptEnd_;
  }

  void setUnresolvedCF(bool newVal);

  void setAbruptEnd(bool newVal) {
    abruptEnd_ = newVal;
  }

  int id() const {
    return blockNumber_;
  }

  // Returns the address of our callee (if we're a call block, of course)
  std::pair<bool, Dyninst::Address> callTarget();

  // instrumentation-related
  bool needsRelocation() const {
    return needsRelocation_;
  }

  void markAsNeedingRelocation() {
    needsRelocation_ = true;
  }

  // codeRange implementation
  void *getPtrToInstruction(Dyninst::Address addr) const override;

  Dyninst::Address get_address() const override {
    return start();
  }

  unsigned get_size() const override {
    return size();
  }

private:
  bool needsRelocation_;
  int blockNumber_;

  bool unresolvedCF_;
  bool abruptEnd_;
};

#endif
