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

#include <string>
#include <vector>

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
    instMapping(const std::string f, const std::string i, const int w, callWhen wn, callOrder o,
                Dyninst::DyninstAPI::codeGenASTPtr a = Dyninst::DyninstAPI::codeGenASTPtr(),
                std::string l = "")
        : func(f), inst(i), lib(l), where(w), when(wn), order(o), useTrampGuard(true),
          mt_only(false), allow_trap(false) {
      if(a != Dyninst::DyninstAPI::codeGenASTPtr()) {
        args.push_back(a);
      }
    }

    instMapping(const std::string f, const std::string i, const int w,
                Dyninst::DyninstAPI::codeGenASTPtr a = Dyninst::DyninstAPI::codeGenASTPtr(),
                std::string l = "")
        : func(f), inst(i), lib(l), where(w), when(callPreInsn), order(orderLastAtPoint),
          useTrampGuard(true), mt_only(false), allow_trap(false) {
      if(a != Dyninst::DyninstAPI::codeGenASTPtr()) {
        args.push_back(a);
      }
    }

    instMapping(const std::string f, const std::string i, const int w,
                std::vector<Dyninst::DyninstAPI::codeGenASTPtr> &aList, std::string l = "")
        : func(f), inst(i), lib(l), where(w), when(callPreInsn), order(orderLastAtPoint),
          useTrampGuard(true), mt_only(false), allow_trap(false) {
      for(auto ast : aList) {
        if(ast) {
          args.push_back(ast);
        }
      }
    }

    // Fork
    instMapping(const instMapping *parMapping, AddressSpace *child);

    ~instMapping() {}

  public:
    void dontUseTrampGuard() {
      useTrampGuard = false;
    }

    void markAs_MTonly() {
      mt_only = true;
    }

    void canUseTrap(bool t) {
      allow_trap = t;
    }

    bool is_MTonly() {
      return mt_only;
    }

    std::string func;                                     /* function to instrument */
    std::string inst;                                     /* inst. function to place at func */
    std::string lib;                                      /* library name */
    int where;                                            /* FUNC_ENTRY, FUNC_EXIT, FUNC_CALL */
    callWhen when;                                        /* callPreInsn, callPostInsn */
    callOrder order;                                      /* orderFirstAtPoint, orderLastAtPoint */
    std::vector<Dyninst::DyninstAPI::codeGenASTPtr> args; /* what to pass as arg0 ... n */
    bool useTrampGuard;
    bool mt_only;
    bool allow_trap;
    std::vector<Dyninst::PatchAPI::InstancePtr> instances;
  };

}}

#endif
