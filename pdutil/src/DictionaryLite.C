// DictionaryLite.C

/*
 * $Log: DictionaryLite.C,v $
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
    if (locate(key, hash, chain, i)) {
        return data_[chain][i].value;
    }
    return insert(key, hash, chain);
}

template<class K, class V>
DO_INLINE_F
bool
dictionary_lite<K,V>::defines(const K& key) const {
    unsigned hash, chain, i;
    return locate(key, hash, chain, i);
}

template<class K, class V>
DO_INLINE_F
void
dictionary_lite<K,V>::undef(const K& key) {
    unsigned hash, chain, i;
    if (locate(key, hash, chain, i)) {
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

//template<class K, class V>
//DO_INLINE_F
//vector<K>
//dictionary_lite<K,V>::keys() const {
//    vector<K> k;
//    for (unsigned i = 0; i < data_.size(); i++) {
//        for (unsigned j = 0; j < data_[i].size(); j++) {
//            k += data_[i][j].key;
//        }
//    }
//    return k;
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
bool
dictionary_lite<K,V>::locate(const K& key, unsigned& hash,
    unsigned& chain, unsigned& i) const {
    hash           = hashf_(key);
    chain          = chain_of(hash);
    hash_chain& ch = data_[chain];

    for (i = 0; i < ch.size(); i++) {
        if ((hash == ch[i].hash_) && (key == ch[i].key)) {
            return true;
        }
    }
    return false;
}

template<class K, class V>
DO_INLINE_P
V&
dictionary_lite<K,V>::insert(const K& key, unsigned hash, unsigned chain) {
//    ((dictionary_lite<K,V> *) this)->data_[chain] += hash_pair(key, hash);
    data_[chain] += hash_pair(key, hash);
    unsigned i      = data_[chain].size()-1;
    unsigned nchain = chain;

    if (data_[chain].size() > chain_size_) {
        unsigned onext = next_;
        split();
        if (chain == onext) {
            unsigned nhash;
            assert(locate(key, nhash, nchain, i));
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
