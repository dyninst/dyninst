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

// DictionaryLite.h

/*
 * $Log: DictionaryLite.h,v $
 * Revision 1.6  1996/08/16 21:29:59  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.5  1996/05/10 05:09:03  tamches
 * find() is a const member function
 *
 * Revision 1.4  1996/05/06 04:38:40  karavan
 * added new function find() to classes dictionary_hash and dictionary_lite
 *
 * Revision 1.3  1995/12/26 19:57:05  tamches
 * made (empty) destructor inline.
 * removed unused member function clear()
 *
 * Revision 1.2  1995/12/16 00:23:37  tamches
 * removed keys() member function which in turn removes the need for vector<KEY>
 * template instantiation.
 * Removed inheritance of pair<K,V> which in turn removes the need for
 * pair<K,V> template.
 *
 * Revision 1.1  1995/11/06 19:19:15  tamches
 * first version of DictionaryLite
 *
 */

/************************************************************************
 * DictionaryLite.h: same as Dictionary.h, but with some template baggage
 * removed.  Should lead to smaller binaries. --ari
 * example: no class Dictionary, which wasn't being used anyway.
 * example: no class Dictionary_Iter, which wasn't being used.
 * example: don't need to instantiate vector< pair<K,V> >, because the
 *          seldom-if-ever used routine items() is gone.
 * example: no more iteration class
 * example: no virtual functions anywhere anymore
 * example: no keys() or values() members anymore so no need for vector<K>
 *             or for vector<V> to be instantiated.
 *             (this is still being worked on; so far, values() is needed in TC)
 * example: no map(), app, revapp() etc. fluff member functions anymore
************************************************************************/

#if defined(external_templates)
#pragma interface
#endif

#ifndef _DICTIONARY_LITE_H_
#define _DICTIONARY_LITE_H_

#include "util/h/Pair.h"
#include "util/h/Vector.h"

#if !defined(DO_INLINE_P)
#define DO_INLINE_P 
#endif

#if !defined(DO_INLINE_F)
#define DO_INLINE_F
#endif


/************************************************************************
 * template<class K, class V> class dictionary_lite
************************************************************************/

static const unsigned DEFAULT_LITE_CHAIN_SIZE = 32;

template<class K, class V>
class dictionary_lite {

public:
    DO_INLINE_F  dictionary_lite (unsigned (*)(const K &));
    DO_INLINE_F  dictionary_lite (unsigned (*)(const K &), unsigned);
    DO_INLINE_F  dictionary_lite (const dictionary_lite<K,V> &);
    DO_INLINE_F ~dictionary_lite () {}

    DO_INLINE_F dictionary_lite<K,V>&  operator= (const dictionary_lite<K,V> &);
    DO_INLINE_F unsigned                    size ()                       const;
    DO_INLINE_F V&                    operator[] (const K &);
    DO_INLINE_F bool                        find (const K &, V &)         const;
    DO_INLINE_F bool                     defines (const K &)              const;
    DO_INLINE_F void                       undef (const K &);
//    DO_INLINE_F void                       clear ();
    DO_INLINE_F vector<V>                 values ()                       const;

    unsigned (*hashf () const) (const K &);

private:
    DO_INLINE_P unsigned chain_of (unsigned)                                      const;
    DO_INLINE_P bool       locate (const K &, unsigned &, unsigned &, unsigned &) const;
    DO_INLINE_P V&         insert (const K &, unsigned, unsigned);
    DO_INLINE_P void       remove (unsigned, unsigned);
    DO_INLINE_P void        split ();

    struct hash_pair {
       K key;
       V value;
       unsigned hash_;

       hash_pair() {}
       hash_pair(const K &theKey, unsigned theHash) : key(theKey), hash_(theHash) {}
       hash_pair(const K &theKey, const V &theValue, unsigned theHash) : key(theKey), value(theValue), hash_(theHash) {}
 
       bool operator==(const hash_pair &other) const {
          return (hash_ == other.hash_ && key == other.key && value == other.value);
       }
    };
    typedef vector<hash_pair> hash_chain;

    unsigned           (*hashf_)(const K &);
    vector<hash_chain> data_;
    unsigned           chain_size_;
    unsigned           nbuckets_;
    vector<unsigned>   llevel_;
    unsigned           glevel_;
    unsigned           next_;
};

#endif /* !defined(_Dictionary_h_) */
