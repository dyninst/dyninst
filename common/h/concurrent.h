/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _CONCURRENT_H_
#define _CONCURRENT_H_

#include "util.h"
#include <memory>
#include <stddef.h>
#include <vector>
#include <boost/atomic.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_queue.h>
#include <boost/functional/hash.hpp>

namespace Dyninst {

namespace dyn_c_annotations {
    void COMMON_EXPORT rwinit(void*);
    void COMMON_EXPORT rwdeinit(void*);
    void COMMON_EXPORT wlock(void*);
    void COMMON_EXPORT wunlock(void*);
    void COMMON_EXPORT rlock(void*);
    void COMMON_EXPORT runlock(void*);
}

namespace concurrent {
  template <typename K>
  struct hasher {
    size_t operator()(K const& k) const {
      return boost::hash<K>{}(k);
    }
  };

  namespace detail {
    template <bool, typename Key>
    class hash_compare;

    // New style tbb_hash_compare concept (TBB_VERSION_MAJOR >= 2021)
    template<typename Key>
    class hash_compare<true, Key> {
    	hasher<Key> my_hasher;
    public:
    	size_t hash(Key const& k) const {
    		return my_hasher(k);
    	}
    	bool equal(Key const& k1, Key const& k2) const {
    		return k1 == k2;
    	}
    };

    // Old style tbb_hash_compare concept
    template<typename Key>
    class hash_compare<false, Key> {
    public:
    	static size_t hash(Key const& k) {
    		return hasher<Key>{}(k);
    	}
    	static bool equal(Key const& k1, Key const& k2) {
    		return k1 == k2;
    	}
    };
  }
}

template<typename K, typename V>
class dyn_c_hash_map : protected tbb::concurrent_hash_map<K, V,
    concurrent::detail::hash_compare<TBB_VERSION_MAJOR >= 2021, K>> {

	using base = tbb::concurrent_hash_map<K, V,
			concurrent::detail::hash_compare<TBB_VERSION_MAJOR >= 2021, K>>;

public:
    using typename base::value_type;
    using typename base::mapped_type;
    using typename base::key_type;

    class const_accessor : public base::const_accessor {
        friend class dyn_c_hash_map<K,V>;
    public:
        ~const_accessor() { release_ann(); }
        void acquire() { dyn_c_annotations::rlock(this->my_node); }
        void release() { release_ann(); base::const_accessor::release(); }
    private:
        void release_ann() {
            if(this->my_node) dyn_c_annotations::runlock(this->my_node);
        }
    };
    class accessor : public base::accessor {
        friend class dyn_c_hash_map<K,V>;
    public:
        ~accessor() { release_ann(); }
        void acquire() { dyn_c_annotations::wlock(this->my_node); }
        void release() { release_ann(); base::accessor::release(); }
    private:
        void release_ann() {
            if(this->my_node) dyn_c_annotations::wunlock(this->my_node);
        }
    };

    bool find(const_accessor& ca, const K& k) const {
        bool r = base::find(ca, k);
        if(r) ca.acquire();
        return r;
    }
    bool find(accessor& a, const K& k) {
        bool r = base::find(a, k);
        if(r) a.acquire();
        return r;
    }

    int contains(const K& k) { return base::count(k) == 1; }

    bool insert(const_accessor& ca, const K& k) {
        bool r = base::insert(ca, k);
        if(r) dyn_c_annotations::rwinit(ca.my_node);
        ca.acquire();
        return r;
    }
    bool insert(accessor& a, const K& k) {
        bool r = base::insert(a, k);
        if(r) dyn_c_annotations::rwinit(a.my_node);
        a.acquire();
        return r;
    }
    bool insert(const_accessor& ca, const value_type& e) {
        bool r = base::insert(ca, e);
        if(r) dyn_c_annotations::rwinit(ca.my_node);
        ca.acquire();
        return r;
    }
    bool insert(accessor& a, const value_type& e) {
        bool r = base::insert(a, e);
        if(r) dyn_c_annotations::rwinit(a.my_node);
        a.acquire();
        return r;
    }
    bool insert(const value_type& e) { return base::insert(e); }

    bool erase(const_accessor& ca) {
        void* n = ca.my_node;
        ca.release_ann();
        bool r = base::erase(ca);
        if(r) dyn_c_annotations::rwdeinit(n);
        return r;
    }
    bool erase(accessor& a) {
        void* n = a.my_node;
        a.release_ann();
        bool r = base::erase(a);
        if(r) dyn_c_annotations::rwdeinit(n);
        return r;
    }
    bool erase(const K& k) { return base::erase(k); }

    int size() const { return base::size(); }

    void rehash( int n = 0 ) { base::rehash(n); }

    using base::clear;

    using typename base::iterator;
    using typename base::const_iterator;
    using base::begin;
    using base::end;
};

template<typename T>
using dyn_c_vector = tbb::concurrent_vector<T, std::allocator<T>>;

template<typename T>
using dyn_c_queue = tbb::concurrent_queue<T, std::allocator<T>>;

class dyn_mutex : public boost::mutex {
public:
    using unique_lock = boost::unique_lock<dyn_mutex>;
};

class COMMON_EXPORT dyn_rwlock {
    // Reader management members
    boost::atomic<unsigned int> rin;
    boost::atomic<unsigned int> rout;
    unsigned int last;
    dyn_mutex inlock;
    boost::condition_variable rcond;
    bool rwakeup[2];

    // Writer management members
    dyn_mutex wlock;
    dyn_mutex outlock;
    boost::condition_variable wcond;
    bool wwakeup;
public:
    dyn_rwlock();
    ~dyn_rwlock();

    void lock_shared();
    void unlock_shared();
    void lock();
    void unlock();

    using unique_lock = boost::unique_lock<dyn_rwlock>;
    using shared_lock = boost::shared_lock<dyn_rwlock>;
};

class COMMON_EXPORT dyn_thread {
public:
    dyn_thread();
    unsigned int getId();
    static unsigned int threads();

    operator unsigned int() { return getId(); }

    static thread_local dyn_thread me;
};

template<typename T>
class dyn_threadlocal {
    std::vector<T> cache;
    T base;
    dyn_rwlock lock;
public:
    dyn_threadlocal() : cache(), base() {}
    dyn_threadlocal(const T& d) : cache(), base(d) {}
    ~dyn_threadlocal() {}

    T get() {
        {
            dyn_rwlock::shared_lock l(lock);
            if(cache.size() > dyn_thread::me)
                return cache[dyn_thread::me];
        }
        {
            dyn_rwlock::unique_lock l(lock);
            if(cache.size() <= dyn_thread::me)
                cache.insert(cache.end(), dyn_thread::threads() - cache.size(), base);
        }
        return base;
    }

    void set(const T& val) {
        {
            dyn_rwlock::shared_lock l(lock);
            if(cache.size() > dyn_thread::me) {
                cache[dyn_thread::me] = val;
                return;
            }
        }
        {
            dyn_rwlock::unique_lock l(lock);
            if(cache.size() <= dyn_thread::me)
                cache.insert(cache.end(), dyn_thread::threads() - cache.size(), base);
            cache[dyn_thread::me] = val;
        }
    }
};

} // namespace Dyninst

#endif
