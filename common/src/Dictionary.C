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

// Dictionary.C

#include "common/h/Dictionary.h"

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
const V& dictionary_hash<K,V>::operator[](const K& key) const {
  const unsigned ndx = locate(key, false);
  assert(ndx != UINT_MAX);
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
dictionary_hash<K,V>::iterator dictionary_hash<K,V>::find(const K& key) {
  const unsigned ndx = locate(key, false); // false -->don't find removed items
  if (ndx == UINT_MAX) {
    return end();
  }
  else {
    dictionary_hash<K,V>::iterator dictIter(*this, all_elems.getIter(ndx));
    return dictIter;
  }
}

template<class K, class V>
dictionary_hash<K,V>::const_iterator dictionary_hash<K,V>::find(const K& key) 
     const {
  const unsigned ndx = locate(key, false); // false -->don't find removed items
  if (ndx == UINT_MAX) {
    return end();
  }
  else {
    dictionary_hash<K,V>::const_iterator dictIter(*this, 
						  all_elems.getIter(ndx));
    return dictIter;
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
bool dictionary_hash<K,V>::find(const K& key, const V **el) const {
  const unsigned ndx = locate(key, false); // false --> don't find removed items
  if (ndx == UINT_MAX)
    return false;
  else {
    (*el) = &all_elems[ndx].val;
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
  if(hasher == NULL) {
    cerr << "hasher == NULL\n";
    assert(false);
  }
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

