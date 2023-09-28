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

#ifndef SYMTAB_INDEXED_MODULES
#define SYMTAB_INDEXED_MODULES

#include "Module.h"

#include <boost/container_hash/hash.hpp>
#include <tbb/concurrent_unordered_set.h>

namespace Dyninst { namespace SymtabAPI {

  namespace detail {
    // A Module is uniquely identified by its name and offset
    struct hash {
      size_t operator()(Module *m) const {
        size_t seed{0UL};
        boost::hash_combine(seed, m->fileName());
        boost::hash_combine(seed, m->addr());
        return seed;
      }
    };

    struct equal {
      bool operator()(Module *m1, Module *m2) const {
        return m1->fileName() == m2->fileName() && m1->addr() == m2->addr();
      }
    };
  }

  class indexed_modules {
    tbb::concurrent_unordered_set<Module *, detail::hash, detail::equal> index;

  public:
    void insert(Module *m) { index.insert(m); }

    bool contains(Module *m) const { return index.count(m) != 0UL; }

    std::vector<Module *> find(std::string const& name) const {
      std::vector<Module *> mods;
      std::copy_if(index.begin(), index.end(), std::back_inserter(mods),
                   [&name](Module *m) { return m->fileName() == name; });
      return mods;
    }

    Module *find(Dyninst::Offset offset) const {
      for (auto *m : index) {
        if (m->addr() == offset)
          return m;
      }
      return nullptr;
    }

    bool empty() const { return index.empty(); }

    decltype(index)::iterator begin() { return index.begin(); }

    decltype(index)::iterator end() { return index.end(); }

    decltype(index)::const_iterator cbegin() const { return index.cbegin(); }

    decltype(index)::const_iterator cend() const { return index.cend(); }
  };
}}

#endif
