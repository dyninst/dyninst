/*
 * Copyright (c) 1996-2000 Barton P. Miller
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
#include <limits.h> // UINT_MAX

template<class K, class V>
dictionary_hash<K,V>::dictionary_hash(unsigned (*hf)(const K &),
                                    unsigned nbins,
                                    unsigned int imax_bin_load,
                                       // we keep #bins*max_bin_load <= total # items * 100
                                    unsigned int ibin_grow_factor
                                       // when we have to grow, by how much?
   ) {
   assert(ibin_grow_factor > 1);
   assert(ibin_grow_factor < 10); // let's not go nuts and grow like crazy!
   assert(imax_bin_load > 0);
   assert(imax_bin_load < 1000); // why would you want to allow so many
                                 // collisions per bin?

   hasher = hf;

   // Note: all_elems[] starts off as an empty vector.

   // Pre-size the # of bins from parameter.  Each bins starts off empty (UINT_MAX)
   assert(nbins > 0);
   bins.resize(nbins);
   for (unsigned binlcv=0; binlcv < bins.size(); binlcv++)
      bins[binlcv] = UINT_MAX;

   num_removed_elems = 0;

   max_bin_load = imax_bin_load;
   bin_grow_factor = ibin_grow_factor;
}


template<class K, class V>
dictionary_hash<K,V>::dictionary_hash(const dictionary_hash &src) :
                              all_elems(src.all_elems),
                              bins(src.bins) {
   // copying an entire dictionary should be a rare occurance; we provide it only
   // for completeness (I'd prefer to leave it out, actually)
   hasher = src.hasher;
   num_removed_elems = src.num_removed_elems;
   max_bin_load = src.max_bin_load;
   bin_grow_factor = src.bin_grow_factor;
}

template<class K, class V>
dictionary_hash<K,V> &
dictionary_hash<K,V>::operator=(const dictionary_hash &src) {
   // assigning an entire dictionary should be a rare occurance; we provide it only
   // for completeness (I'd prefer to leave it out, actually)
   if (&src == this) return *this;

   hasher = src.hasher;
   all_elems = src.all_elems;
   bins = src.bins;

   num_removed_elems = src.num_removed_elems;
   max_bin_load = src.max_bin_load;
   bin_grow_factor = src.bin_grow_factor;

   return *this;
}

template<class K, class V>
unsigned
dictionary_hash<K,V>::size() const {
   assert(num_removed_elems <= all_elems.size());
   return all_elems.size() - num_removed_elems;
}

template<class K, class V>
V&
dictionary_hash<K,V>::operator[](const K& key) {
   const unsigned ndx = locate_addIfNotFound(key);

//   assert(defines(key)); // WARNING: expensive assert!
   
   return all_elems[ndx].val;
}

template<class K, class V>
bool
dictionary_hash<K,V>::find(const K& key, V& el) const {
   const unsigned ndx = locate(key, false); // false --> don't find removed items
   if (ndx == UINT_MAX)
      return false;
   else {
      el = all_elems[ndx].val;
      return true;
   }
}

template<class K, class V>
bool
dictionary_hash<K,V>::defines(const K& key) const {
   const unsigned ndx = locate(key, false); // false --> don't find removed items
   return (ndx != UINT_MAX);
}

template<class K, class V>
void
dictionary_hash<K,V>::undef(const K& key) {
   unsigned ndx = locate(key, false); // false --> don't find removed items
   if (ndx == UINT_MAX)
      return; // nothing to do...either doesn't exist, or already removed

   const unsigned oldsize = size();
   entry &e = all_elems[ndx];
   assert(!e.removed);
   e.removed = true;
   num_removed_elems++;
   assert(oldsize == size()+1);
   assert(num_removed_elems <= all_elems.size());
}

template<class K, class V>
vector<K>
dictionary_hash<K,V>::keys() const {
   // One can argue that this method (and values(), below) should be removed in
   // favor of using the dictionary iterator class.  I agree; it's here only for
   // backwards compatibility.
   vector<K> result(size());
   unsigned ndx=0;
   for (unsigned i=0; i < all_elems.size(); i++)
      if (!all_elems[i].removed)
         result[ndx++] = all_elems[i].key;
   assert(ndx == size());
   return result;
}

template<class K, class V>
vector<V>
dictionary_hash<K,V>::values() const {
   // One can argue that this method (and keys(), above) should be removed in
   // favor of using the dictionary iterator class.  I agree; it's here only for
   // backwards compatibility.
   vector<V> result(size());
   unsigned ndx=0;
   for (unsigned i=0; i < all_elems.size(); i++)
      if (!all_elems[i].removed)
         result[ndx++] = all_elems[i].val;
   assert(ndx == size());
   return result;
}

template<class K, class V>
void dictionary_hash<K,V>::clear() {
   // max_bin_load and bin_grow_factor don't change.
   // Also, bins.size() doesn't change; is this best (perhaps we should shrink
   // bins down to its original size...not trivial since we didn't set that value
   // aside in the ctor.  In any event we don't lose sleep since calling clear() is
   // rare.)

   all_elems.resize(0);

   for (unsigned lcv=0; lcv < bins.size(); lcv++)
      bins[lcv] = UINT_MAX;

   num_removed_elems = 0;

   assert(size() == 0);
}

template<class K, class V>
unsigned
dictionary_hash<K,V>::locate(const K& key, bool evenIfRemoved) const {
   // An internal routine used by everyone.
   unsigned elem_ndx = UINT_MAX;

   if( bins.size() > 0 )
   {
       const unsigned hashval = hasher(key);
       const unsigned bin     = hashval % bins.size();
   
       elem_ndx = bins[bin];
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
   }
   return elem_ndx;
}


template<class K, class V>
unsigned
dictionary_hash<K,V>::locate_addIfNotFound(const K& key) {
   // An internal routine used by everyone.
   // Returns ndx (within all_elems[]) of the item.

   unsigned result = locate(key, true); // true --> find even if 'removed' flag set
      // UINT_MAX if not found

   if (result == UINT_MAX) {
      // didn't find the item.  We're gonna insert.
      // we know that the item's removed flag isn't set; if it was, result wouldn't
      // have been UINT_MAX.  So we truly need to add an item.

      // before the insert, we should have enough bins to fit everything nicely...
      assert( (bins.size() * max_bin_load) >= (all_elems.size() * 100) );
      
      if ( (bins.size() * max_bin_load) < ((all_elems.size() + 1) * 100) ) {
         // ...but adding 1 more element would make things too big.  So, grow (add
         // some new bins) before adding.
         
         const unsigned new_numbins = (unsigned)(bins.size() * bin_grow_factor);
         assert(new_numbins > bins.size() && "grow factor negative or barely > 1.00?");

         // ... after the grow, we should have enough bins:
         assert(new_numbins * max_bin_load >= ((all_elems.size() + 1) * 100));

         grow_numbins(new_numbins);

         // ...verify that we have enough bins after the grow:
         assert( (bins.size() * max_bin_load) >= ((all_elems.size() + 1) * 100));

         // fall through...
      }
      
      // We don't need to grow.
      const unsigned hashval = hasher(key);
      const unsigned bin     = hashval % bins.size();

      all_elems += entry(key, hashval, V(), UINT_MAX);
      const unsigned new_entry_ndx = all_elems.size()-1;

      // Insert at the head of the bin (we could insert at the tail, but this
      // is a tad easier, and besides, position in the bin doesn't matter)
      entry &e = all_elems[new_entry_ndx];
      e.next = bins[bin];
      bins[bin] = new_entry_ndx;

//      assert(defines(key)); // WARNING: expensive assert()

      assert( (bins.size() * max_bin_load) >= (all_elems.size() * 100) );  // Check invariant again

      return new_entry_ndx;
   }
   else {
      // found the item.
     
      assert( (bins.size() * max_bin_load) >= (all_elems.size() * 100) );  // Check invariant first

      entry &e = all_elems[result];
      if (e.removed) {
         // Item has been removed.  We're gonna un-remove it.

//         assert(!defines(key)); // WARNING: expensive assert
      
         assert(num_removed_elems > 0);

         e.removed = false;
         e.val = V();
         num_removed_elems--;

         if (! ( (bins.size() * max_bin_load) >= (all_elems.size() * 100) )) {
           // Oops, the un-remove just broke the invariant!
           // Grow some bins to re-establish the invariant.

           const unsigned new_numbins = (unsigned)(bins.size() * bin_grow_factor);
           assert(new_numbins > bins.size() && "grow factor negative or barely > 1.00?");

           // ... after the grow, we should have enough bins:
           assert( (new_numbins * max_bin_load) >= (all_elems.size() * 100) );

           grow_numbins(new_numbins);

           // ...verify that we have enough bins after the grow:
           assert( (bins.size() * max_bin_load) >= (all_elems.size() * 100) );

           result = locate(key, true); // true --> find even if 'removed' flag set
           assert(result != UINT_MAX); // should exist
        }
      }

//      assert(defines(key)); // WARNING: expensive assert

      assert( (bins.size() * max_bin_load) >= (all_elems.size() * 100) );  // Check invariant again

      return result;
   }
}

template<class K, class V>
void
dictionary_hash<K,V>::grow_numbins(unsigned new_numbins) {
   assert(new_numbins > bins.size() && "grow_numbins not adding any bins?");

   bins.resize(new_numbins);
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

   // the invariant should now hold
   assert( (bins.size() * max_bin_load) >= (all_elems.size() * 100) );
}
