/************************************************************************
 * Pair.h: definition of pairs for dictionaries and sets.
************************************************************************/



#if defined(external_templates)
#pragma interface
#endif


#if !defined(_Pair_h_)
#define _Pair_h_




/************************************************************************
 * template<class K, class V> struct pair
************************************************************************/

template<class K, class V>
struct pair {
    K key;
    V value;

    bool operator== (const pair<K,V>& p) const {
        return ((key == p.key) && (value == p.value));
    }

    pair ()                                          {}
    pair (const K& k) : key(k), value(0)             {}
    pair (const K& k, const V& v) : key(k), value(v) {}
};





#endif /* !defined(_Pair_h_) */
