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
  bool      defines (const K &) const;
  void      undef (const K &); 

#ifndef _KERNEL
  // Sun's compiler can't handle the return types
  vector<K> keys () const;
  vector<V> values () const;
  vector< pair<K, V> > keysAndValues() const;
#endif

  void      clear ();

  typedef dictionary_hash_iter<K,V> const_iterator;
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

    entry() {} // needed by vector class
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

  // The actual elements, in one big vector.  This enables certain important
  // operations, (specifically rehashing the entire dictionary) to be done
  // efficiently.  Plus, it's a clean approach to storage.
  // Since we use vector<>::operator+= on this vector, we're glad that class vector<>
  // preallocates some extra stuff when growing.
  vector<entry> all_elems;

  // The bins.  Note: since the number of bins doesn't change often (only when
  // growing), we use resize with the exact flag set.
  vector<unsigned> bins;
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
  const dictionary_hash<K,V> &dict;
  vector< dictionary_hash<K,V>::entry >::const_iterator i;
  vector< dictionary_hash<K,V>::entry >::const_iterator the_end;
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
  //dictionary_hash_iter(vector< dictionary_hash<K,V>::entry >::const_iterator ii,
  //		       vector< dictionary_hash<K,V>::entry >::const_iterator iend) :
  //                   i(ii), the_end(iend) {
  //make_valid_or_end();
  //}
  dictionary_hash_iter(const dictionary_hash<K,V> &idict) : dict(idict) {
    reset();
  }
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

  const K &currkey() const {
    return i->key;
  }
  RET currval() const {
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

// methods of this class implemented in .h so compiler has no trouble finding
// them for template instantiation

#include <limits.h> // UINT_MAX

template<class K, class V>
const unsigned dictionary_hash<K,V>::bin_grow_factor = 2;

template<class K, class V>
dictionary_hash<K,V>::dictionary_hash(unsigned (*hf)(const K &),
                                      unsigned nbins,
                                      unsigned imax_bin_load) {
  // we keep #bins*max_bin_load <= total # items * 100
  assert(imax_bin_load > 0);
  assert(imax_bin_load < 1000); // why would you want to allow so many
                                // collisions per bin?
  hasher = hf;

  // Note: all_elems[] starts off as an empty vector.

  // Pre-size the # of bins from parameter.  
  // Each bins starts off empty (UINT_MAX)
  assert(nbins > 0);
  bins.resize(nbins);
  for (unsigned binlcv=0; binlcv < bins.size(); binlcv++)
    bins[binlcv] = UINT_MAX;

  num_removed_elems = 0;

  max_bin_load = imax_bin_load;

  assert(enoughBins());
}

template<class K, class V>
dictionary_hash<K,V>::dictionary_hash(const dictionary_hash<K,V> &src) :
                                      all_elems(src.all_elems), bins(src.bins) {
  // copying an entire dictionary should be a rare occurance; we provide it only
  // for completeness (I'd prefer to leave it out, actually)
  hasher = src.hasher;
  num_removed_elems = src.num_removed_elems;
  max_bin_load = src.max_bin_load;

  assert(enoughBins());
}

template<class K, class V>
dictionary_hash<K,V> &
dictionary_hash<K,V>::operator=(const dictionary_hash<K,V> &src) {
  // assigning an entire dictionary should be a rare occurance; we provide it only
  // for completeness (I'd prefer to leave it out, actually)
  if (&src == this) return *this;

  hasher = src.hasher;
  all_elems = src.all_elems;
  bins = src.bins;

  num_removed_elems = src.num_removed_elems;
  max_bin_load = src.max_bin_load;

  assert(enoughBins());

  return *this;
}

template<class K, class V>
unsigned dictionary_hash<K,V>::size() const {
  assert(num_removed_elems <= all_elems.size());
  return all_elems.size() - num_removed_elems;
}

template<class K, class V>
V& dictionary_hash<K,V>::operator[](const K& key) {
  const unsigned ndx = locate_addIfNotFound(key);

  //assert(defines(key)); // WARNING: expensive assert!
   
  return all_elems[ndx].val;
}

template<class K, class V>
const V& dictionary_hash<K,V>::get(const K &key) const {
  const unsigned ndx = locate(key, false);
  // false: if removed, then it doesn't count

  if (ndx == UINT_MAX)
    assert(false && "dictionary_hash get() requires a hit");
   
  return all_elems[ndx].val;
}

template<class K, class V>
V& dictionary_hash<K,V>::get(const K &key) {
  const unsigned ndx = locate(key, false);
  // false: if removed, then it doesn't count

  if (ndx == UINT_MAX)
    assert(false && "dictionary_hash get() requires a hit");
   
  return all_elems[ndx].val;
}

template<class K, class V>
const V& dictionary_hash<K,V>::get_and_remove(const K &key) {
  const unsigned ndx = locate(key, false);
  // false: if removed, then it doesn't count

  if (ndx == UINT_MAX)
    assert(false && "dictionary_hash get_and_remove() requires a hit");

  const unsigned oldsize = size();

  entry &e = all_elems[ndx];

  assert(!e.removed);
  e.removed = true;
  num_removed_elems++;
  assert(num_removed_elems <= all_elems.size());

  assert(size() + 1 == oldsize);
   
  return e.val;
}

template<class K, class V>
bool dictionary_hash<K,V>::find_and_remove(const K &key, V &val) {
  const unsigned ndx = locate(key, false);
  // false: if removed, then it doesn't count

  if (ndx == UINT_MAX)
    return false;

  const unsigned oldsize = size();

  entry &e = all_elems[ndx];

  assert(!e.removed);
  e.removed = true;
  num_removed_elems++;
  assert(num_removed_elems <= all_elems.size());

  assert(size() + 1 == oldsize);

  val = e.val;

  return true;
}

template<class K, class V>
void dictionary_hash<K,V>::set(const K &key, const V &val) {
  const unsigned ndx = locate(key, true); // true --> if removed, count as a find

  if (ndx != UINT_MAX) {
    // Found an entry.  If this entry is actually 'removed', then un-remove it.
    // Otherwise, barf.
    entry &theEntry = all_elems[ndx];
    if (theEntry.removed) {
      assert(num_removed_elems > 0);
      theEntry.val = val;
      theEntry.removed = false;
      num_removed_elems--;

      // clearing the removed flag can't possibly invalidate the performance
      // invariant
    }
    else
      assert(false && "dictionary set(): an entry with that key already exists");
  }
  else {
    // Didn't find that entry.  Good.  Create that entry and add to the dictionary.
    add(key, val);
      
    //assert(defines(key)); // WARNING: expensive assert()
  }
}

template<class K, class V>
bool dictionary_hash<K,V>::find(const K& key, V& el) const {
  const unsigned ndx = locate(key, false); // false --> don't find removed items
  if (ndx == UINT_MAX)
    return false;
  else {
    el = all_elems[ndx].val;
    return true;
  }
}

template<class K, class V>
bool dictionary_hash<K,V>::defines(const K& key) const {
  const unsigned ndx = locate(key, false); // false --> don't find removed items
  return (ndx != UINT_MAX);
}

template<class K, class V>
void dictionary_hash<K,V>::undef(const K& key) {
  unsigned ndx = locate(key, false); // false --> don't find removed items
  if (ndx == UINT_MAX)
    return; // nothing to do...either doesn't exist, or already removed

#ifndef NDEBUG
  const unsigned oldsize = size();
#endif

  entry &e = all_elems[ndx];
  assert(!e.removed);
  e.removed = true;
  num_removed_elems++;

#ifndef NDEBUG
  assert(oldsize == size()+1);
  assert(num_removed_elems <= all_elems.size());
#endif
}

#ifndef _KERNEL
// Sun's compiler can't handle the return types

template<class K, class V>
vector<K>
dictionary_hash<K,V>::keys() const {
  // One can argue that this method (and values(), below) should be removed in
  // favor of using the dictionary iterator class.  I agree; it's here only for
  // backwards compatibility.
  vector<K> result;
  result.reserve(size());
   
  // note that the iterator class automatically skips elems tagged for removal
  const_iterator finish = end();
  for (const_iterator iter=begin(); iter != finish; iter++)
    result.push_back(iter.currkey());

  return result;
}

template<class K, class V>
vector<V>
dictionary_hash<K,V>::values() const {
  // One can argue that this method (and keys(), above) should be removed in
  // favor of using the dictionary iterator class.  I agree; it's here only for
  // backwards compatibility.
  vector<V> result;
  result.reserve(size());
   
  // note that the iterator class automatically skips elems tagged for removal
  const_iterator finish = end();
  for (const_iterator iter=begin(); iter != finish; ++iter)
    result.push_back(*iter);

  return result;
}

template<class K, class V>
vector< pair<K, V> >
dictionary_hash<K,V>::keysAndValues() const {
  vector< pair<K, V> > result;
  result.reserve(size());
   
  const_iterator finish = end();
  for (const_iterator iter = begin(); iter != finish; ++iter)
    result.push_back( make_pair(iter.currkey(), iter.currval()) );

  return result;
}

#endif //ifndef(_KERNEL)

template<class K, class V>
void dictionary_hash<K,V>::clear() {
  // max_bin_load and bin_grow_factor don't change.
  // Also, bins.size() doesn't change; is this best (perhaps we should shrink
  // bins down to its original size...not trivial since we didn't set that value
  // aside in the ctor.  In any event we don't lose sleep since calling clear() is
  // rare.)  Like class vector, we could also provide a zap() method which actually
  // does free up memory.

  all_elems.clear();

  for (unsigned lcv=0; lcv < bins.size(); lcv++)
    bins[lcv] = UINT_MAX;

  num_removed_elems = 0;

  assert(size() == 0);

  assert(enoughBins());
}

template<class K, class V>
unsigned
dictionary_hash<K,V>::locate(const K& key, bool evenIfRemoved) const {
  // An internal routine used by everyone.

  // call hasher(key), but make sure it fits in 31 bits:
  unsigned hashval = hasher(key) & 0x7fffffff;

  const unsigned bin = hashval % bins.size();
   
  unsigned elem_ndx = bins[bin];
  while (elem_ndx != UINT_MAX) {
    const entry &elem = all_elems[elem_ndx];

    // verify that this elem is in the right bin!
    assert(elem.key_hashval % bins.size() == bin);
      
    if (elem.key_hashval == hashval && elem.key == key) {
      // found it...unless it was removed
      if (elem.removed && !evenIfRemoved)
	elem_ndx = UINT_MAX;

      break;
    }
    else
      elem_ndx = elem.next;
  }

  return elem_ndx;
}


template<class K, class V>
unsigned
dictionary_hash<K,V>::add(const K& key, const V &val) {
  // internal routine called by locate_addIfNotFound() and by set()
  // returns ndx (within all_elems[]) of newly added item

  // before the insert, we should have enough bins to fit everything nicely...
  assert(enoughBins());

  if (!enoughBinsIf1MoreItemAdded()) {
    // ...but adding 1 more element would make things too big.  So, grow (add
    // some new bins) before adding.
         
    const unsigned new_numbins = (unsigned)(bins.size() * bin_grow_factor);
    assert(new_numbins > bins.size() && "bin_grow_factor too small");

    grow_numbins(new_numbins);

    // ...verify that we have enough bins after the grow:
    assert(enoughBinsIf1MoreItemAdded());

    // fall through...
  }
      
  // We don't need to grow.
  const unsigned hashval = hasher(key) & 0x7fffffff; // make sure it fits in 31 bits
  const unsigned bin     = hashval % bins.size();

  entry e(key, hashval, val, UINT_MAX);
  e.next = bins[bin];
  all_elems.push_back(e);
  //entry *e = all_elems.append_with_inplace_construction();
  //new((void*)e)entry(key, hashval, val, UINT_MAX);
  const unsigned new_entry_ndx = all_elems.size()-1;

  // Insert at the head of the bin (we could insert at the tail, but this
  // is a tad easier, and besides, position in the bin doesn't matter)
  bins[bin] = new_entry_ndx;

  //assert(defines(key)); // WARNING: expensive assert()
      
  assert(enoughBins());
  return new_entry_ndx;
}

template<class K, class V>
unsigned
dictionary_hash<K,V>::locate_addIfNotFound(const K& key) {
  // An internal routine used by everyone.
  // Returns ndx (within all_elems[]) of the item.

  unsigned result = locate(key, true); // true --> find even if 'removed' flag set
  // UINT_MAX if not found

  if (result == UINT_MAX)
    return add(key, V());
  else {
    // found the item.
    entry &e = all_elems[result];
    if (e.removed) {
      // Item has been removed.  We're gonna un-remove it.

      //assert(!defines(key)); // WARNING: expensive assert
      
      assert(num_removed_elems > 0);

      e.removed = false;
      e.val = V();
      num_removed_elems--;

      // Clearing the 'removed' flag of an entry can't possibly invalidate the
      // performance assertion
    }

    //assert(defines(key)); // WARNING: expensive assert
      
    return result;
  }
}

template<class K, class V>
void
dictionary_hash<K,V>::grow_numbins(unsigned new_numbins) {
  assert(new_numbins > bins.size() && "grow_numbins not adding any bins?");

  bins.resize(new_numbins, true); // true --> exact resize flag
  for (unsigned binlcv=0; binlcv < bins.size(); binlcv++)
    bins[binlcv] = UINT_MAX;

  // Look for elems to remove; shrink all_elems[] as appropriate
  if (num_removed_elems > 0) {
    for (unsigned lcv=0; lcv < all_elems.size(); ) {
      entry &e = all_elems[lcv];
      if (e.removed) {
	const unsigned oldsize = all_elems.size();
	assert(oldsize > 0);

	// remove item from vector by swap with last item & resizing/-1
	all_elems[lcv] = all_elems[oldsize-1]; // should be OK if lcv==oldsize-1
	all_elems.resize(oldsize-1);

	num_removed_elems--;
            
	// note: we DON'T bump up lcv in this case
      }
      else
	lcv++;
    }

    assert(num_removed_elems == 0);
  }

  // Now rehash everything.  Sounds awfully expensive, and I guess it is -- it's
  // linear in the total # of items in the hash table.  But take heart: beyond this
  // point, we don't need to make any copies of type KEY or VALUE, resize any vectors,
  // or recalculate any hash values.  We just need to assign to <n> unsigned
  // integers.

  const unsigned numbins = bins.size(); // loop invariant
  for (unsigned lcv=0; lcv < all_elems.size(); lcv++) {
    entry &e = all_elems[lcv];
    assert(!e.removed);

    // the entry's key_hashval stays the same, but it may go into a different bin:
    const unsigned bin = e.key_hashval % numbins;

    // prepend element to bin:
    unsigned &thebin = bins[bin];
      
    e.next = thebin;
    thebin = lcv;
  }

  // The invariant might not have held at the top of this routine, but it
  // should now hold.
  assert(enoughBins());
}

#endif
