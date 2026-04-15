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

#ifndef DYNINST_DYNINSTAPI_FUNCTION_CACHE_H
#define DYNINST_DYNINSTAPI_FUNCTION_CACHE_H

#include "function.h"

#include <vector>

namespace Dyninst { namespace DyninstAPI {

  class function_cache final {
    std::vector<func_instance*> funcs;
  public:
    function_cache() {
      funcs.reserve(10);
    }
    bool contains(func_instance *f) const {
      auto cmp = [f](func_instance *c) {
        return c->addr() == f->addr() &&
               c->typedName() == f->typedName();
      };
      return std::find_if(funcs.begin(), funcs.end(), cmp) == funcs.end();
    }
    void insert(func_instance *f) {
      funcs.push_back(f);
    }
  };

}}

#endif
