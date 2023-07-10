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

#if !defined(INTERVAL_TREE_H)
#define INTERVAL_TREE_H

#include <assert.h>
#include <stdio.h>
#include <utility>
#include <vector>
#include <map>

template <class K, class V>
class IntervalTree {
  typedef typename std::map<K, std::pair<K, V> > Tree;
  typedef typename Tree::const_iterator c_iter;
  typedef typename Tree::const_reverse_iterator c_r_iter;
  typedef typename Tree::iterator Iter;

 public:
  typedef typename std::pair<K, K> Range;
  typedef typename std::pair<Range, V> Entry;
  typedef typename Tree::iterator iterator;
  typedef typename Tree::const_iterator const_iterator;

  iterator begin() { return tree_.begin(); }
  iterator end() { return tree_.end(); }
  c_r_iter rbegin() { return tree_.rbegin(); }
  c_r_iter rend() { return tree_.rend(); }
  const_iterator begin() const { return tree_.begin(); }
  const_iterator end() const { return tree_.end(); }

  int size() const { return tree_.size(); }
  bool empty() const { return tree_.empty(); }
  void insert(K lb, K ub, V v) {
    tree_[lb] = std::make_pair(ub, v);
  }

  void remove(K lb) {
     erase(lb);
  }

  void erase(K lb) {
     tree_.erase(lb);
  }

  bool find(K key, V &value) const {
    K lb = 0;
    K ub = 0;
    V val;
    if (!precessor(key, lb, ub, val))
      return false;
    if (key < lb) return false;
    if (key >= ub) return false;
    value = val;
    return true;
  }

  bool find(K key, K &l, K &u, V &value) const {
    if (!precessor(key, l, u, value))
      return false;
    if (key < l) return false;
    if (key >= u) return false;
    return true;
  }

  bool precessor(K key, K &l, K &u, V &v) const {
    Entry e;
    if (!precessor(key, e)) {
      return false;
    }
    l = lb(e);
    u = ub(e);
    v = value(e);
    return true;
  }

  bool precessor(K key, Entry &e) const {
    if (tree_.empty()) return false;
    c_iter iter = tree_.lower_bound(key);
    if ((iter == tree_.end()) ||
	(iter->first != key)) {
      if (iter == tree_.begin()) {
	return false;
      }
      --iter;
    }
    if (iter->first > key) return false;

    lb(e) = iter->first;
    ub(e) = iter->second.first;
    value(e) = iter->second.second;

    assert(lb(e) <= key);
    if (ub(e) <= key) return false;
    return true;
  }
  void elements(std::vector<Entry> &buffer) const {
    buffer.clear();
    for (c_iter iter = tree_.begin();
	 iter != tree_.end(); ++iter) {
      Entry e;
      lb(e) = iter->first;
      ub(e) = iter->second.first;
      value(e) = iter->second.second;
      buffer.push_back(e);
    }
  }

  K lowest() const { 
    if (tree_.empty()) return 0;
    c_iter iter = tree_.begin();
    return iter->first;
  }

  K highest() const { 
    if (tree_.empty()) return 0;
    c_r_iter iter = tree_.rbegin();
    return iter->second.first;
  }

  void clear() { tree_.clear(); }

  bool empty() { return tree_.empty(); }

  bool update(K lb, K newUB) {
     Iter iter = tree_.find(lb);
     if (iter == tree_.end()) return false;
     iter->second.first = newUB;
     return true;
  }

  bool updateValue(K lb, V newVal) {
      Iter iter = tree_.find(lb);
      if (iter == tree_.end()) return false;
      iter->second.second = newVal;
      return true;
  }

 private:
  
  static V &value(Entry &e) { return e.second; }
  static K &lb(Entry &e) { return e.first.first; }
  static K &ub(Entry &e) { return e.first.second; }

  Tree tree_;
};
#endif
