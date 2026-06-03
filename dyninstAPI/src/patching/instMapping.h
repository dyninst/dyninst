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

#ifndef DYNINST_DYNINSTAPI_INSTMAPPING_H
#define DYNINST_DYNINSTAPI_INSTMAPPING_H

#include "ASTs/codeGenAST.h"
#include "instPoint.h"

#include <string>
#include <vector>

#define FUNC_ENTRY 0x1 /* entry to the function */
#define FUNC_EXIT 0x2  /* exit from function */
#define FUNC_CALL 0x4  /* subroutines called from func */
#define FUNC_ARG 0x8   /* use arg as argument */

namespace Dyninst { namespace DyninstAPI {

  enum callWhen {
    callPreInsn,
    callPostInsn,
    callBranchTargetInsn,
    callUnset
  };

  enum callOrder {
    orderFirstAtPoint,
    orderLastAtPoint
  };

  // Container class for "instrument this point with this function".
  class instMapping {

  public:
    instMapping(std::string f, std::string i, const int w, codeGenASTPtr a = codeGenASTPtr())
        : func(std::move(f)), inst(std::move(i)), where(w) {
      if(a) {
        args.push_back(a);
      }
    }

    // Fork
    instMapping(const instMapping *parIM, AddressSpace *child)
        : func(parIM->func), inst(parIM->inst), where(parIM->where), args(parIM->args),
          useTrampGuard(parIM->useTrampGuard), allow_trap(parIM->allow_trap) {
      for(auto instance : parIM->instances) {
        auto cMT = getChildInstance(instance, child);
        assert(cMT);
        instances.push_back(cMT);
      }
    }

    void dontUseTrampGuard() {
      useTrampGuard = false;
    }

    void canUseTrap(bool t) {
      allow_trap = t;
    }

    std::string func;                /* function to instrument */
    std::string inst;                /* inst. function to place at func */
    int where;                       /* FUNC_ENTRY, FUNC_EXIT, FUNC_CALL */
    std::vector<codeGenASTPtr> args; /* what to pass as arg0 ... n */
    bool useTrampGuard{true};
    bool allow_trap{false};
    std::vector<Dyninst::PatchAPI::InstancePtr> instances;
  };

}}

#endif
