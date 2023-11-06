#ifndef INDEXED_SYMBOLS_HPP
#define INDEXED_SYMBOLS_HPP

#include "Symbol.h"
#include "concurrent.h"
#include <iterator>
#include <utility>
#include <vector>
#include "dyntypes.h"

namespace st = Dyninst::SymtabAPI;

struct indexed_symbols {
  typedef Dyninst::dyn_c_hash_map<st::Symbol *, Dyninst::Offset> master_t;
  typedef std::vector<st::Symbol *> symvec_t;
  typedef Dyninst::dyn_c_hash_map<Dyninst::Offset, symvec_t> by_offset_t;
  typedef Dyninst::dyn_c_hash_map<std::string, symvec_t> by_name_t;

  master_t master;
  by_offset_t by_offset;
  by_name_t by_mangled;
  by_name_t by_pretty;
  by_name_t by_typed;

  // Only inserts if not present. Returns whether it inserted.
  // Operations on the indexed_symbols compound table.
  bool insert(st::Symbol *s) {
    Dyninst::Offset o = s->getOffset();
    master_t::accessor a;
    if (master.insert(a, std::make_pair(s, o))) {
      {
        by_offset_t::accessor oa;
        by_offset.insert(oa, o);
        oa->second.push_back(s);
      }
      {
        by_name_t::accessor ma;
        by_mangled.insert(ma, s->getMangledName());
        ma->second.push_back(s);
      }
      {
        by_name_t::accessor pa;
        by_pretty.insert(pa, s->getPrettyName());
        pa->second.push_back(s);
      }
      {
        by_name_t::accessor ta;
        by_typed.insert(ta, s->getTypedName());
        ta->second.push_back(s);
      }

      return true;
    }
    return false;
  }

  // Clears the table. Do not use in parallel.
  void clear() {
    master.clear();
    by_offset.clear();
    by_mangled.clear();
    by_pretty.clear();
    by_typed.clear();
  }

  // Erases Symbols from the table. Do not use in parallel.
  void erase(st::Symbol *s) {
    if (master.erase(s)) {
      {
        by_offset_t::accessor oa;
        if (!by_offset.find(oa, s->getOffset())) {
          assert(!"by_offset.find(oa, s->getOffset())");
        }
        std::remove(oa->second.begin(), oa->second.end(), s);
      }
      {
        by_name_t::accessor ma;
        if (!by_mangled.find(ma, s->getMangledName())) {
          assert(!"by_mangled.find(ma, s->getMangledName())");
        }
        std::remove(ma->second.begin(), ma->second.end(), s);
      }
      {
        by_name_t::accessor pa;
        if (!by_pretty.find(pa, s->getPrettyName())) {
          assert(!"by_pretty.find(pa, s->getPrettyName())");
        }
        std::remove(pa->second.begin(), pa->second.end(), s);
      }
      {
        by_name_t::accessor ta;
        if (!by_typed.find(ta, s->getTypedName())) {
          assert(!"by_typed.find(ta, s->getTypedName())");
        }
        std::remove(ta->second.begin(), ta->second.end(), s);
      }
    }
  }

  // Iterator for the symbols. Do not use in parallel.
  class iterator {
    master_t::iterator m;

  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = st::Symbol *;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type *;
    using reference = value_type &;

    iterator(master_t::iterator i) : m(i) {}
    bool operator==(const iterator &x) const { return m == x.m; }
    bool operator!=(const iterator &x) const { return !operator==(x); }
    st::Symbol *const &operator*() const { return m->first; }
    st::Symbol *const *operator->() const { return &operator*(); }
    iterator &operator++() {
      ++m;
      return *this;
    }
    iterator operator++(int) {
      iterator old(m);
      operator++();
      return old;
    }
  };

  iterator begin() { return iterator(master.begin()); }
  iterator end() { return iterator(master.end()); }
};

#endif
