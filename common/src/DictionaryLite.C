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

// DictionaryLite.C

/*
 * $Log: DictionaryLite.C,v $
 * Revision 1.8  1996/11/26 16:09:32  naim
 * Fixing asserts - naim
 *
 * Revision 1.7  1996/10/31 07:32:31  tamches
 * locate() now returns V* instead of bool
 *
 * Revision 1.6  1996/08/16 21:31:45  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.5  1996/05/10 05:09:25  tamches
 * find() is a const member function
 *
 * Revision 1.4  1996/05/06 04:39:20  karavan
 * added new function find() to dictionary_lite class
 *
 * Revision 1.3  1995/12/26 19:57:17  tamches
 * made (empty) destructor inline.
 * removed unused member function clear()
 *
 * Revision 1.2  1995/12/16 00:24:12  tamches
 * commented out keys()
 *
 * Revision 1.1  1995/11/06 19:20:28  tamches
 * first version of DictionaryLite
 *
 */

/************************************************************************
 * DictionaryLite.C: same as Dictionary.h, but with some template baggage
 * removed.  Should lead to smaller binaries. --ari
************************************************************************/

#include "util/h/DictionaryLite.h"

/************************************************************************
 * template<class K, class V> class dictionary_lite
************************************************************************/

template<class K, class V>
DO_INLINE_F
dictionary_lite<K,V>::dictionary_lite(unsigned (*hf)(const K &))
    : hashf_(hf), data_(1), chain_size_(DEFAULT_LITE_CHAIN_SIZE),
    nbuckets_(data_.size()), llevel_(data_.size(), 0),
    glevel_(0), next_(0) {
}

template<class K, class V>
DO_INLINE_F
dictionary_lite<K,V>::dictionary_lite(unsigned (*hf)(const K &), unsigned s1)
    : hashf_(hf), data_(s1), chain_size_(s1), nbuckets_(data_.size()),
    llevel_(data_.size(), 0), glevel_(0), next_(0) {
    assert(s1 != 0);
}

template<class K, class V>
DO_INLINE_F
dictionary_lite<K,V>::dictionary_lite(const dictionary_lite<K,V>& d)
    : hashf_(d.hashf_), data_(d.data_), chain_size_(d.chain_size_),
    nbuckets_(d.nbuckets_), llevel_(d.llevel_), glevel_(d.glevel_),
    next_(d.next_) {
}

template<class K, class V>
DO_INLINE_F
dictionary_lite<K,V>&
dictionary_lite<K,V>::operator=(const dictionary_lite<K,V>& d) {
    if (this == &d) {
        return *this;
    }

    hashf_      = d.hashf_;
    data_       = d.data_;
    chain_size_ = d.chain_size_;
    nbuckets_   = d.nbuckets_;
    llevel_     = d.llevel_;
    glevel_     = d.glevel_;
    next_       = d.next_;

    return *this;
}

template<class K, class V>
DO_INLINE_F
unsigned
dictionary_lite<K,V>::size() const {
    unsigned sz = 0;
    for (unsigned chain = 0; chain < data_.size(); chain++) {
        sz += data_[chain].size();
    }
    return sz;
}

template<class K, class V>
DO_INLINE_F
V&
dictionary_lite<K,V>::operator[](const K& key) {
    unsigned hash, chain, i;
    V *value = locate(key, hash, chain, i);
    if (value != NULL)
        return *value;
    else
        return insert(key, hash, chain);
}

template<class K, class V>
DO_INLINE_F
bool 
dictionary_lite<K,V>::find(const K& key, V& el) const {
   unsigned hash, chain, i;
   V *value = locate(key, hash, chain, i);
   if (value != NULL) {
      el = *value; // makes a copy of type V; don't use this if V::operator=(const V&) is expensive
      return true;
   }
   else
      return false;
}

template<class K, class V>
DO_INLINE_F
bool
dictionary_lite<K,V>::defines(const K& key) const {
    unsigned hash, chain, i;
    return (locate(key, hash, chain, i) != NULL);
}

template<class K, class V>
DO_INLINE_F
void
dictionary_lite<K,V>::undef(const K& key) {
    unsigned hash, chain, i;
    if (locate(key, hash, chain, i) != NULL) {
        remove(chain, i);
    }
}

//template<class K, class V>
//DO_INLINE_F
//void
//dictionary_lite<K,V>::clear() {
//    for (unsigned i = 0; i < data_.size(); i++) {
//        data_[i].resize(0);
//    }
//}

template<class K, class V>
DO_INLINE_F
vector<V>
dictionary_lite<K,V>::values() const {
    vector<V> v;
    for (unsigned i = 0; i < data_.size(); i++) {
        for (unsigned j = 0; j < data_[i].size(); j++) {
            v += data_[i][j].value;
        }
    }
    return v;
}

template<class K, class V>
DO_INLINE_F
unsigned
(*dictionary_lite<K,V>::hashf() const)(const K &) {
    return hashf_;
}

template<class K, class V>
DO_INLINE_P
unsigned
dictionary_lite<K,V>::chain_of(unsigned hash) const {
    unsigned chain = hash % nbuckets_;
    if (llevel_[chain] > glevel_) {
        chain = hash % (nbuckets_*2);
        assert(chain < data_.size());
    }
    return chain;
}

template<class K, class V>
DO_INLINE_P
V*
dictionary_lite<K,V>::locate(const K& key, unsigned& hash,
			     unsigned& chain, unsigned& index) const {
    // returns ptr to value if found; else, returns NULL;

    hash           = hashf_(key);
    chain          = chain_of(hash);

    const hash_chain &ch = data_[chain];
    const unsigned hash_chain_size = ch.size();

    for (index = 0; index < hash_chain_size; index++) {
        hash_pair &thePair = ch[index];

        if (hash == thePair.hash_ && key == thePair.key)
            return &thePair.value;
    }
    return NULL;
}

template<class K, class V>
DO_INLINE_P
V&
dictionary_lite<K,V>::insert(const K& key, unsigned hash, unsigned chain) {
    data_[chain] += hash_pair(key, hash);
    unsigned i      = data_[chain].size()-1;
    unsigned nchain = chain;

    if (data_[chain].size() > chain_size_) {
        unsigned onext = next_;
        split();
        if (chain == onext) {
            unsigned nhash;
            bool aflag;
	    aflag=(locate(key, nhash, nchain, i) != NULL);
	    assert(aflag);
            assert(hash == nhash);
        }
    }
    return data_[nchain][i].value;
}

template<class K, class V>
DO_INLINE_P
void
dictionary_lite<K,V>::remove(unsigned chain, unsigned i) {
    hash_chain&   ch   = data_[chain];
    unsigned last = ch.size()-1;
    if (i != last) {
        ch[i] = ch[last];
    }
    ch.resize(last);
}

template<class K, class V>
DO_INLINE_P
void
dictionary_lite<K,V>::split() {
    unsigned nch = data_.size();
    unsigned nsz = nch+1;

    data_.resize(nsz);
    llevel_.resize(nsz); llevel_[nch] = ++llevel_[next_];

    hash_chain& ch = data_[next_];
    unsigned i = 0;
    while (i < ch.size()) {
        if (chain_of(ch[i].hash_) != next_) {
            data_[nch] += ch[i];
            remove(next_, i);
        }
        else {
            i++;
        }
    }

    next_++;
    if (next_ == nbuckets_) {
        next_ = 0;
        nbuckets_ *= 2;
        glevel_++;
    }
}
