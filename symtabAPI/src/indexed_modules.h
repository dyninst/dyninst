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
