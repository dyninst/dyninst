/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// Dictionary.h

#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_

#include "common/h/language.h"
#include "common/h/Vector.h" // takes care of redefining assert as ASSERT for _KERNEL
#include "common/h/Pair.h"

/************************************************************************
 * template<class K, class V> class dictionary_hash
 * Ari Tamches
 *
 * See Stroustrup, The C++ Programming Language, 3d edition, which
 * provided inspiration for this hash table class via an extended
 * example in section 17.6
 *
 * This class tries very hard to ensure that there are enough bins s.t.
 * hash collisions are few (though if you provide a bad hash function,
 * all bets are off, as usual).  Toward that end, the following invariant
 * is maintained at all times:
 *         #bins * max_bin_load >= total # items
 *         Note about "total # items": includes items that are marked as removed
 *         (after all, such items do indeed clutter up the bins and so should be
 *         taken into account for this performance invariant.)
 * max_bin_load is a user-settable parameter (see ctor).
 *
 * When adding a new hash table item would break the invariant,
 * #bins is multiplied by bin_grow_factor (another user-settable parameter in ctor),
 * and everything is rehashed (expensive, but not needed often, since growth
 * is exponential).  NOTE: since the performance invariant doesn't distinguish
 * between items marked as 'removed' and non-removed items, when we clear the
 * 'removed' flag of any item, the invariant cannot possibly be broken.
 *
 * Note: We don't make any effort to keep num_bins always a power of 2.
 *       If such an invariant were maintained, then "x % num_bins"
 *       simplifies to "x % (1U << lg_num_bins)-1".  We could get away
 *       with just storing lg_num_bins, and use "1U << lg_num_bins"
 *       to get num_bins at any time.
 * 
************************************************************************/

template<class K, class V> class dictionary_hash_iter;

template<class K, class V>
class dictionary_hash {
  typedef const V &RET;
  friend class dictionary_hash_iter<K,V>;

 public:
  typedef K key_type;
  typedef V value_type;
  typedef RET iter_value_return_type;
  typedef V& value_reference_type;

  typedef dictionary_hash_iter<K,V> const_iterator;
  typedef dictionary_hash_iter<K,V> iterator;
   
  dictionary_hash (unsigned (*hashfunc)(const K &),
		   unsigned nbins=101,
		   unsigned max_bin_load=70
		   // we keep #bins*max_bin_load >= total # items * 100, 
		   // to make sure that there are enough bins s.t. 
		   // (assuming a good hash function) collisions are few.
		   );
  ~dictionary_hash () {}

  // I'd rather not have provided these methods, but apparantly some code
  // in paradynd actually copies entire dictionaries (!)
  dictionary_hash (const dictionary_hash<K,V> &);
  dictionary_hash<K,V>&  operator= (const dictionary_hash<K,V> &);

  unsigned getMemUsage_exceptObjItself_AndExtraFromKeyOrValue() const {
    return all_elems.capacity() * sizeof(entry) + bins.capacity() * sizeof(unsigned);
  }

  unsigned size () const;

  V& operator[] (const K &);
  // If key doesn't exist, creates a new entry (using the default ctor for the
  // value).  Hence this can't be a const method; contrast with get().
  const V& operator[] (const K &) const;

  V& get(const K &);
  const V& get(const K &) const;
  // If key doesn't exist, then barfs.  Contrast with operator[].

  const V& get_and_remove(const K &);
  // returns a reference the to value part of the deleted item (possible because
  // in reality, deleting an item just sets the 'removed' flag).
   
  bool find_and_remove(const K &, V&);

  void set(const K &, const V &);
  // If key already exists, barfs.  Contrast with operator[].

  bool      find (const K &, V &) const;
  bool      find (const K &, const V **) const;

  iterator find(const K &);  // like STL hashmap find
  const_iterator find(const K &) const;  // like STL hashmap find

  bool      defines (const K &) const;
  void      undef (const K &); 

#ifndef _KERNEL
  // Sun's compiler can't handle the return types
  pdvector<K> keys () const;
  pdvector<V> values () const;
  pdvector< pdpair<K, V> > keysAndValues() const;
#endif

  void      clear ();

  const_iterator begin() const {
    //return const_iterator(all_elems.begin(), all_elems.end());
    return const_iterator(*this);
  }
  const_iterator end() const {
    //return const_iterator(all_elems.end(), all_elems.end());
    return const_iterator(*this).end();
  }

 private:
  bool enoughBins() const {
    //return bins.size() * max_bin_load >= size();
    // ABOVE WAS BUGGY: use all_elems.size(), not size()!
    return bins.size() * max_bin_load >= all_elems.size();
  }
  bool enoughBinsIf1MoreItemAdded() const {
    return bins.size() * max_bin_load >= all_elems.size() + 1;
  }

  unsigned add(const K &, const V &);
  // internal routine called by locate_addIfNotFound() and by set()
  // Assumes an entry with that key does not exist; adds to the dictionary.
  // returns ndx within all_elems[]
   
  unsigned locate_addIfNotFound(const K &);
  // returns ndx within all_elems[]; never returns UINT_MAX

  unsigned locate(const K &, bool even_if_removed) const;
  // never adds a new entry if not found.  Okay to call from const member fns

  void grow_numbins(unsigned new_numbins);

  // We keep a ptr to the hash function:
  unsigned (*hasher)(const K &);
  // NOTE: always squeeze the result into 31 bits before using it, since
  // key_hashval, below, is a 31-bit bitfield.

  // Important structure:
  struct entry {
    K key;
    V val;
    unsigned key_hashval : 31;
    // we could always rehash, since we have 'key', but this
    // is faster.
    unsigned removed : 1;
    // since removal would require a lot of shifting w/in all_elems[],
    // we use this flag instead, and only do the actual removal on
    // resize.
    unsigned next; // an index into 'all_elems', for hash collisions.  UINT_MAX
                   // implies end of collision chain for this bin

    entry() {} // needed by pdvector class
    entry(const K &ikey, unsigned ikey_hashval, const V &ival, unsigned inext) :
          key(ikey), val(ival), key_hashval(ikey_hashval), next(inext) {
      removed = false;
    }
    entry(const entry &src) : key(src.key), val(src.val) {
      key_hashval = src.key_hashval;
      removed = src.removed;
      next = src.next;
    }
    entry& operator=(const entry &src) {
      if (&src == this) return *this;

      key = src.key;
      val = src.val;
      key_hashval = src.key_hashval;
      removed = src.removed;
      next = src.next;
      return *this;
    }
  };

  // The actual elements, in one big pdvector.  This enables certain important
  // operations, (specifically rehashing the entire dictionary) to be done
  // efficiently.  Plus, it's a clean approach to storage.
  // Since we use pdvector<>::operator+= on this pdvector, we're glad that class pdvector<>
  // preallocates some extra stuff when growing.
  pdvector<entry> all_elems;

  // The bins.  Note: since the number of bins doesn't change often (only when
  // growing), we use resize with the exact flag set.
  pdvector<unsigned> bins;
  // each entry in 'bins' gives the index (w/in 'all_elems') of the first element
  // in the bin.  In other words, the first element in bin X is all_elems[bin[X]].
  // If bin[X] is UINT_MAX, then the bin is empty.

  unsigned num_removed_elems;
   
  // Notes:
  // 1) At any given time, all_elems.size()-num_removed_items gives us the
  //    total # of items in the dictionary.
  // 2) At any given time, bins.size() gives us the total # of bins in the dictionary.
  // 3) We keep the invariant #bins * max_bin_load >= total # items * 100,
  //    incrementing #bins (by a factor of bin_grow_factor) if needed to maintain it.

  unsigned max_bin_load;	// percentage of total number of items
  static const unsigned bin_grow_factor;
};

// Typical way to use an iterator for this class:
// for (dictionary_hash<K,V>::iterator iter=thedict.begin();
//      iter != thedict.end(); iter++) {
//    V &v = *iter; // or use iter-> directly to access the value.
//    // You can write to iter-> to change the value.  You can't change or even access
//    // the key right now.
// }

template<class K, class V>
class dictionary_hash_iter {
 private:
  typedef const V &RET; // RET: type returned by operator*()
  dictionary_hash<K,V> &dict;
  TYPENAME pdvector< TYPENAME dictionary_hash<K,V>::entry >::iterator i;
  TYPENAME pdvector< TYPENAME dictionary_hash<K,V>::entry >::iterator the_end;

  // too bad we need to store the_end (for make_valid_or_end())
   
  void move_to_next() {
    i++;
    make_valid_or_end();
  }
  void make_valid_or_end() {
    while (i != the_end && i->removed)
      i++;
  }
   
 public:
  //dictionary_hash_iter(pdvector< dictionary_hash<K,V>::entry >::const_iterator ii,
  //		       pdvector< dictionary_hash<K,V>::entry >::const_iterator iend) :
  //                   i(ii), the_end(iend) {
  //make_valid_or_end();
  //}
  dictionary_hash_iter(dictionary_hash<K,V> &idict) : dict(idict) {
    reset();
  }
  dictionary_hash_iter(const dictionary_hash<K,V> &idict) : 
    dict(const_cast< dictionary_hash<K,V>& >(idict)) {
    reset();
  }
  dictionary_hash_iter(dictionary_hash<K,V> &idict,
      TYPENAME pdvector< TYPENAME dictionary_hash<K,V>::entry>::iterator curi) 
    : dict(idict), i(curi), the_end(dict.all_elems.end()) {
  }
  dictionary_hash_iter(const dictionary_hash<K,V> &idict,
      TYPENAME pdvector< TYPENAME dictionary_hash<K,V>::entry>::const_iterator curi) 
    : dict(const_cast< dictionary_hash<K,V>& >(idict)), 
    i(const_cast< TYPENAME pdvector< TYPENAME dictionary_hash<K,V>::entry >::iterator>(curi)), 
    the_end(const_cast< TYPENAME pdvector< TYPENAME dictionary_hash<K,V>::entry >::iterator>(
                                                                                           dict.all_elems.end()))
  {  }

  dictionary_hash_iter(const dictionary_hash_iter<K,V> &src) :
                       dict(src.dict), i(src.i), the_end(src.the_end) { }
  //dictionary_hash_iter& operator=(const dictionary_hash_iter<K,V> &src) {
  //  dict = src.dict;
  //  i = src.i;
  //  the_end = src.the_end;
  //  return *this;
  //}

  dictionary_hash_iter operator++(int) {
    dictionary_hash_iter result = *this;
    move_to_next();
    return result;
  }
  dictionary_hash_iter operator++() { // prefix
    move_to_next();
    return *this;
  }

  bool next(K &k, V &v) {
    for (; i != the_end; i++) {
      if (!i->removed) {
	k = i->key;
	v = i->val;
	i++;
	return true;
      }
    }
    return false;
  }

  void reset() {
    // start the iterator off at the first non-removed item now:
    if (dict.all_elems.size() == 0) {
      i = the_end = NULL;
    }
    else {
      i = dict.all_elems.begin();
      the_end = dict.all_elems.end();

      while (i != the_end && i->removed)
	i++;
    }
  }

  dictionary_hash_iter end() {
    i = the_end;
    return *this;
  }

  RET operator*() {
    return i->val;
  }
//   RET *operator->() {
//      return &i->val;
//   }

  K currkey() {
    return i->key;
  }
  const K &currkey() const {
    return i->key;
  }
  RET currval() const {
    return i->val;
  }
  V &currval() {
    return i->val;
  }

  bool operator==(const dictionary_hash_iter &other) const {
    return i == other.i; // no need to check the_end, right?
  }
  bool operator!=(const dictionary_hash_iter &other) const {
    return i != other.i; // no need to check the_end, right?
  }

  operator bool() const {return i < the_end;}
};


#endif
