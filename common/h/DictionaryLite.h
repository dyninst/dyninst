// DictionaryLite.h

/*
 * $Log: DictionaryLite.h,v $
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
    DO_INLINE_F ~dictionary_lite ();

    DO_INLINE_F dictionary_lite<K,V>&  operator= (const dictionary_lite<K,V> &);
    DO_INLINE_F unsigned                    size ()                       const;
    DO_INLINE_F V&                    operator[] (const K &);
    DO_INLINE_F bool                     defines (const K &)              const;
    DO_INLINE_F void                       undef (const K &);
    DO_INLINE_F void                       clear ();
    DO_INLINE_F vector<K>                   keys ()                       const;
    DO_INLINE_F vector<V>                 values ()                       const;

    unsigned (*hashf () const) (const K &);

private:
    DO_INLINE_P unsigned chain_of (unsigned)                                      const;
    DO_INLINE_P bool       locate (const K &, unsigned &, unsigned &, unsigned &) const;
    DO_INLINE_P V&         insert (const K &, unsigned, unsigned);
    DO_INLINE_P void       remove (unsigned, unsigned);
    DO_INLINE_P void        split ();

    struct hash_pair : public pair<K,V> {
        unsigned hash_;

        hash_pair()                                   : pair<K,V>(),    hash_(0) {}
        hash_pair(const K& k, unsigned h)             : pair<K,V>(k),   hash_(h) {}
        hash_pair(const K& k, unsigned h, const V& v) : pair<K,V>(k,v), hash_(h) {}

	unsigned operator==(const hash_pair& p) const {
	  return ((hash_ == p.hash_) && pair<K,V>::operator==(p));
	  // ((*((pair<K,V> *) ((void*)this))) == p));
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
