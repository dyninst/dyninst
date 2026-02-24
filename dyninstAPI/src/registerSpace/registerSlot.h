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

#ifndef DYNINST_DYNINSTAPI_REGISTERSPACE_REGISTERSLOT_H
#define DYNINST_DYNINSTAPI_REGISTERSPACE_REGISTERSLOT_H

#include "dyn_register.h"

#include <cassert>
#include <string>

/*
 * Information about where a register can be found
 *
 * It can be in one of the following states:
 *
 *  1) Unsaved, and available via the register itself
 *
 *  2) Saved in a frame (e.g., a base tramp)
 *
 *  3) Pushed on the stack at a relative offset from the
 *     current stack pointer.
 *
 *
 * Terminology:
 *
 *  Live - contains a value outside of instrumentation; must be saved before use
 *  Used - used by instrumentation code
 */

/*  TODO
 *
 *  1. We could subclass this and make "get me the current value" a member
 *     function. Not sure it's really worth it for the minimal amount of
 *     memory multiple types will use.
 *
 *  2. We also need a better way of tracking what state a register is in.
 *     Here are some possibilities, not at all mutually independent:
 *
 *     a) Live at the start of instrumentation, or dead;
 *     b) Used during the generation of a subexpression
 *     c) Currently reserved by another AST (could recalculate if necessary)
 *     d) At a function call node, is it carrying a value?
 */

class registerSlot {
public:
  const Dyninst::Register number{Dyninst::Null_Register};
  const std::string name{"DEFAULT REGISTER"};

  typedef enum { deadAlways, deadABI, liveAlways } initialLiveness_t;
  const initialLiveness_t initialState{deadAlways};

  // Are we off limits for allocation in this particular instance?
  bool offLimits{true};

  typedef enum { invalid, GPR, FPR, SPR, SGPR, VGPR, realReg } regType_t;
  const regType_t type{invalid};

  ////////// Code generation

  // == 0 if free
  int refCount{};

  typedef enum { live, spilled, dead } livenessState_t;
  livenessState_t liveState{live};

  // Are we keeping this (as long as we can) to save the pre-calculated value?
  // Note: refCount can be 0 and this still set.
  bool keptValue{false};

  // Has this register been used by generated code?
  bool beenUsed{false};

  // New version of "if we were saved, then where?" It's a pair - true/false,
  // then offset from the "zeroed" stack pointer.
  typedef enum { unspilled, framePointer } spillReference_t;
  spillReference_t spilledState{unspilled};

  /*
   * Offset where this register can be retrieved
   *
   *  AMD-64: this is the number of words
   *  POWER: this is the number of bytes
   *
   *  I know it's inconsistent, but it's easier this way since
   *  POWER has some funky math.
   */
  int saveOffset{-1};

  //////// Member functions

  unsigned encoding() const;

  void cleanSlot();

  void markUsed(bool incRefCount) {
    assert(offLimits == false);
    assert(refCount == 0);
    assert(liveState != live);

    if (incRefCount)
      refCount = 1;
    beenUsed = true;
  }

  void debugPrint(const char *str = NULL);

  registerSlot(Dyninst::Register num, std::string name_, bool offLimits_,
               initialLiveness_t initial, regType_t type_)
      : number(num), name(std::move(name_)), initialState(initial),
        offLimits(offLimits_), type(type_) {}
};

#endif
