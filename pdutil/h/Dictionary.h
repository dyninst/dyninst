/************************************************************************
 * Dictionary.h: implementation of template-based dictionaries.
************************************************************************/




#pragma interface

#if !defined(_Dictionary_h_)
#define _Dictionary_h_




/************************************************************************
 * header files.
************************************************************************/

#include "util/h/Pair.h"
#include "util/h/Vector.h"



#ifndef DO_INLINE
#define DO_INLINE
#endif


/************************************************************************
 * template<class K, class V> class dictionary
************************************************************************/

template<class K, class V>
class dictionary {
public:
             dictionary ();
    virtual ~dictionary ();

    virtual unsigned                  size ()                                                  const =0;
    virtual V&                  operator[] (const K &)                                         const =0;
    virtual bool                   defines (const K &)                                         const =0;
    virtual void                     undef (const K &)                                               =0;
    virtual void                     clear ()                                                        =0;
    virtual vector<K>                 keys ()                                                  const =0;
    virtual vector<V>               values ()                                                  const =0;
    virtual vector< pair<K,V> >      items ()                                                  const =0;
    virtual void                       app (void (*)(const K &, V &))                                =0;
    virtual void                    revapp (void (*)(const K &, V &))                                =0;
    virtual V                         fold (V (*)(const K &, const V &, const V &), const V &) const =0;
    virtual V                      revfold (V (*)(const K &, const V &, const V &), const V &) const =0;

private:
    dictionary                 (const dictionary<K,V> &); // not allowed
    dictionary<K,V>& operator= (const dictionary<K,V> &); // not allowed
};

template<class K, class V>
DO_INLINE
dictionary<K,V>::dictionary() {
}

template<class K, class V>
DO_INLINE
dictionary<K,V>::~dictionary() {
}





/************************************************************************
 * template<class K, class V> class dictionary_hash
************************************************************************/

static const unsigned DEFAULT_CHAIN_SIZE = 32;

template<class K, class V>
class dictionary_hash : public dictionary<K,V> {

public:
             dictionary_hash (unsigned (*)(const K &));
             dictionary_hash (unsigned (*)(const K &), unsigned);
             dictionary_hash (const dictionary_hash<K,V> &);
    virtual ~dictionary_hash ();

    dictionary_hash<K,V>&  operator= (const dictionary_hash<K,V> &);
    unsigned                    size ()                                                  const;
    V&                    operator[] (const K &)                                         const;
    bool                     defines (const K &)                                         const;
    void                       undef (const K &);
    void                       clear ();
    vector<K>                   keys ()                                                  const;
    vector<V>                 values ()                                                  const;
    vector< pair<K,V> >        items ()                                                  const;
    void                         app (void (*)(const K &, V &));
    void                      revapp (void (*)(const K &, V &));
    V                           fold (V (*)(const K &, const V &, const V &), const V &) const;
    V                        revfold (V (*)(const K &, const V &, const V &), const V &) const;

    unsigned (*hashf () const) (const K &);

private:
    unsigned chain_of (unsigned)                                      const;
    bool       locate (const K &, unsigned &, unsigned &, unsigned &) const;
    V&         insert (const K &, unsigned, unsigned)                 const;
    void       remove (unsigned, unsigned);
    void        split ();

    struct hash_pair : public pair<K,V> {
        unsigned hash_;

        hash_pair()                                   : pair<K,V>(),    hash_(0) {}
        hash_pair(const K& k, unsigned h)             : pair<K,V>(k),   hash_(h) {}
        hash_pair(const K& k, unsigned h, const V& v) : pair<K,V>(k,v), hash_(h) {}

        unsigned operator==(const hash_pair& p) const {
            return ((hash_ == p.hash_) && ((*((pair<K,V> *) this)) == p));
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

    friend class dictionary_hash_iter<K,V>;
};

template<class K, class V>
DO_INLINE
dictionary_hash<K,V>::dictionary_hash(unsigned (*hf)(const K &))
    : hashf_(hf), data_(1), chain_size_(DEFAULT_CHAIN_SIZE),
    nbuckets_(data_.size()), llevel_(data_.size(), 0),
    glevel_(0), next_(0) {
}

template<class K, class V>
DO_INLINE
dictionary_hash<K,V>::dictionary_hash(unsigned (*hf)(const K &), unsigned s1)
    : hashf_(hf), data_(s1), chain_size_(s1), nbuckets_(data_.size()),
    llevel_(data_.size(), 0), glevel_(0), next_(0) {
    assert(s1 != 0);
}

template<class K, class V>
DO_INLINE
dictionary_hash<K,V>::dictionary_hash(const dictionary_hash<K,V>& d)
    : hashf_(d.hashf_), data_(d.data_), chain_size_(d.chain_size_),
    nbuckets_(d.nbuckets_), llevel_(d.llevel_), glevel_(d.glevel_),
    next_(d.next_) {
}

template<class K, class V>
DO_INLINE
dictionary_hash<K,V>::~dictionary_hash() {
}

template<class K, class V>
DO_INLINE
dictionary_hash<K,V>&
dictionary_hash<K,V>::operator=(const dictionary_hash<K,V>& d) {
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
DO_INLINE
unsigned
dictionary_hash<K,V>::size() const {
    unsigned sz = 0;
    for (unsigned chain = 0; chain < data_.size(); chain++) {
        sz += data_[chain].size();
    }
    return sz;
}

template<class K, class V>
DO_INLINE
V&
dictionary_hash<K,V>::operator[](const K& key) const {
    unsigned hash, chain, i;
    if (locate(key, hash, chain, i)) {
        return data_[chain][i].value;
    }
    return insert(key, hash, chain);
}

template<class K, class V>
DO_INLINE
bool
dictionary_hash<K,V>::defines(const K& key) const {
    unsigned hash, chain, i;
    return locate(key, hash, chain, i);
}

template<class K, class V>
DO_INLINE
void
dictionary_hash<K,V>::undef(const K& key) {
    unsigned hash, chain, i;
    if (locate(key, hash, chain, i)) {
        remove(chain, i);
    }
}

template<class K, class V>
DO_INLINE
void
dictionary_hash<K,V>::clear() {
    for (unsigned i = 0; i < data_.size(); i++) {
        data_[i].resize(0);
    }
}

template<class K, class V>
DO_INLINE
vector<K>
dictionary_hash<K,V>::keys() const {
    vector<K> k;
    for (unsigned i = 0; i < data_.size(); i++) {
        for (unsigned j = 0; j < data_[i].size(); j++) {
            k += data_[i][j].key;
        }
    }
    return k;
}

template<class K, class V>
DO_INLINE
vector<V>
dictionary_hash<K,V>::values() const {
    vector<V> v;
    for (unsigned i = 0; i < data_.size(); i++) {
        for (unsigned j = 0; j < data_[i].size(); j++) {
            v += data_[i][j].value;
        }
    }
    return v;
}

template<class K, class V>
DO_INLINE
vector< pair<K,V> >
dictionary_hash<K,V>::items() const {
    vector< pair<K,V> > p;
    for (unsigned i = 0; i < data_.size(); i++) {
        for (unsigned j = 0; j < data_[i].size(); j++) {
            p += pair<K,V>(data_[i][j].key, data_[i][j].value);
        }
    }
    return p;
}

template<class K, class V>
DO_INLINE
void
dictionary_hash<K,V>::app(void (*f)(const K &, V &)) {
    for (unsigned i = 0; i < data_.size(); i++) {
        for (unsigned j = 0; j < data_[i].size(); j++) {
            f(data_[i][j].key, data_[i][j].value);
        }
    }
}

template<class K, class V>
DO_INLINE
void
dictionary_hash<K,V>::revapp(void (*f)(const K &, V &)) {
    app(f);
}

template<class K, class V>
DO_INLINE
V
dictionary_hash<K,V>::fold(V (*f)(const K &, const V &, const V &),
    const V& arg0) const {
    V ret = arg0;
    for (unsigned i = 0; i < data_.size(); i++) {
        for (unsigned j = 0; j < data_[i].size(); j++) {
            ret = f(data_[i][j].key, data_[i][j].value, ret);
        }
    }
    return ret;
}

template<class K, class V>
DO_INLINE
V
dictionary_hash<K,V>::revfold(V (*f)(const K &, const V &, const V &),
    const V& arg0) const {
    return fold(f, arg0);
}

template<class K, class V>
DO_INLINE
unsigned
(*dictionary_hash<K,V>::hashf() const)(const K &) {
    return hashf_;
}

template<class K, class V>
DO_INLINE
unsigned
dictionary_hash<K,V>::chain_of(unsigned hash) const {
    unsigned chain = hash % nbuckets_;
    if (llevel_[chain] > glevel_) {
        chain = hash % (nbuckets_*2);
        assert(chain < data_.size());
    }
    return chain;
}

template<class K, class V>
DO_INLINE
bool
dictionary_hash<K,V>::locate(const K& key, unsigned& hash,
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
DO_INLINE
V&
dictionary_hash<K,V>::insert(const K& key, unsigned hash, unsigned chain) const {
    ((dictionary_hash<K,V> *) this)->data_[chain] += hash_pair(key, hash);
    unsigned i      = data_[chain].size()-1;
    unsigned nchain = chain;

    if (data_[chain].size() > chain_size_) {
        unsigned onext = next_;
        ((dictionary_hash<K,V> *) this)->split();
        if (chain == onext) {
            unsigned nhash;
            assert(locate(key, nhash, nchain, i));
            assert(hash == nhash);
        }
    }
    return data_[nchain][i].value;
}

template<class K, class V>
DO_INLINE
void
dictionary_hash<K,V>::remove(unsigned chain, unsigned i) {
    hash_chain&   ch   = data_[chain];
    unsigned last = ch.size()-1;
    if (i != last) {
        ch[i] = ch[last];
    }
    ch.resize(last);
}

template<class K, class V>
DO_INLINE
void
dictionary_hash<K,V>::split() {
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





/************************************************************************
 * template<class K, class V> class dictionary_iter
************************************************************************/

template<class K, class V>
class dictionary_iter {
public:
             dictionary_iter ();
    virtual ~dictionary_iter ();

    virtual bool  next (K &, V &) =0;
    virtual void reset ()         =0;

private:
    dictionary_iter                 (const dictionary_iter<K,V> &); // not allowed
    dictionary_iter<K,V>& operator= (const dictionary_iter<K,V> &); // not allowed
};

template<class K, class V>
DO_INLINE
dictionary_iter<K,V>::dictionary_iter() {
}

template<class K, class V>
DO_INLINE
dictionary_iter<K,V>::~dictionary_iter() {
}





/************************************************************************
 * template<class K, class V> class dictionary_hash_iter
************************************************************************/

template<class K, class V>
class dictionary_hash_iter : public dictionary_iter<K,V> {
public:
             dictionary_hash_iter (const dictionary_hash<K,V> &);
    virtual ~dictionary_hash_iter ();

    bool  next (K &, V &);
    void reset ();

private:
    const dictionary_hash<K,V>& dict_;
    unsigned                    chain_;
    unsigned                    curr_;

    dictionary_hash_iter                 (const dictionary_hash_iter<K,V> &); // not allowed
    dictionary_hash_iter<K,V>& operator= (const dictionary_hash_iter<K,V> &); // not allowed
};

template<class K, class V>
DO_INLINE
bool
dictionary_hash_iter<K,V>::next(K& a, V& b) {
    while (chain_ < dict_.data_.size()) {
        if (curr_ < dict_.data_[chain_].size()) {
            a = dict_.data_[chain_][curr_].key;
            b = dict_.data_[chain_][curr_].value;
            curr_++;
            return true;
        }

        chain_++;
        curr_ = 0;
    }

    return false;
}

template<class K, class V>
DO_INLINE
void
dictionary_hash_iter<K,V>::reset() {
    chain_ = curr_ = 0;
}

template<class K, class V>
DO_INLINE
dictionary_hash_iter<K,V>::dictionary_hash_iter(const dictionary_hash<K,V>& d)
    : dict_(d), chain_(0), curr_(0) {
}

template<class K, class V>
dictionary_hash_iter<K,V>::~dictionary_hash_iter() {
}





/************************************************************************
 * Iteration functions.
************************************************************************/

template<class K, class V1, class V2>
dictionary_hash<K,V2>
map(V2 (*f)(const V1 &), const dictionary_hash<K,V1>& d) {
    dictionary_hash<K,V2>      nd(d.hashf());
    dictionary_hash_iter<K,V1> di(d);
    K  k;
    V1 v1;
    while (di.next(k, v1)) {
        nd[k] = f(v1);
    }
    return nd;
}

template<class K, class V1, class V2>
V2
fold(V2 (*f)(const K &, const V1 &, const V2 &), const dictionary_hash<K,V1>& d, const V2& arg0) {
    dictionary_hash_iter<K,V1> di(d);
    K  k;
    V1 v1;
    V2 v2 = arg0;
    while (di.next(k, v1)) {
        v2 = f(k, v1, v2);
    }
    return v2;
}

template<class K, class V1, class V2>
V2
revfold(V2 (*f)(const K &, const V1 &, const V2 &), const dictionary_hash<K,V1>& d, const V2& arg0) {
    return fold(f, d, arg0);
}





#endif /* !defined(_Dictionary_h_) */
