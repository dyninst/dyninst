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
#include <boost/atomic.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_queue.h>

namespace Dyninst {

template<typename K, typename V>
class dyn_c_hash_map : protected tbb::concurrent_hash_map<K, V,
    tbb::tbb_hash_compare<K>, std::allocator<std::pair<K,V>>> {

    typedef tbb::concurrent_hash_map<K, V,
        tbb::tbb_hash_compare<K>, std::allocator<std::pair<K,V>>> base;

    typedef std::pair<K,V> entry;

public:
    dyn_c_hash_map() : base() {};
    ~dyn_c_hash_map() {};

    class const_accessor : public base::const_accessor {
#ifdef DYNINST_VG_ANNOTATIONS
        bool owns;
#endif
    public:
        const_accessor() : base::const_accessor()
#ifdef DYNINST_VG_ANNOTATIONS
            , owns(true)
#endif
            {};
        ~const_accessor() { release(); }
        void acquire() {
#ifdef DYNINST_VG_ANNOTATIONS
            owns = true;
            ANNOTATE_HAPPENS_AFTER(base::const_accessor::operator->());
#endif
        }
        bool acquire(bool r) { acquire(); return r; }
        void release() {
#ifdef DYNINST_VG_ANNOTATIONS
            if(owns) {
                owns = false;
                ANNOTATE_HAPPENS_BEFORE(base::const_accessor::operator->() + 1);
            }
#endif
        }
    };
    class accessor : public base::accessor {
#ifdef DYNINST_VG_ANNOTATIONS
        bool owns;
#endif
    public:
        accessor() : base::accessor()
#ifdef DYNINST_VG_ANNOTATIONS
            , owns(true)
#endif
            {};
        ~accessor() { release(); }
        void acquire() {
#ifdef DYNINST_VG_ANNOTATIONS
            owns = true;
            ANNOTATE_HAPPENS_AFTER(base::accessor::operator->() + 1);
            ANNOTATE_HAPPENS_AFTER(base::accessor::operator->());
#endif
        }
        bool acquire(bool r) { acquire(); return r; }
        void release() {
#ifdef DYNINST_VG_ANNOTATIONS
            if(owns) {
                ANNOTATE_HAPPENS_BEFORE(base::accessor::operator->());
            }
#endif
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

    bool insert(const_accessor& ca, const K& k) {
        return ca.acquire(base::insert(ca, k)); }
    bool insert(accessor& a, const K& k) { return a.acquire(base::insert(a, k)); }
    bool insert(const K& k) { return base::insert(k); }
    bool insert(const_accessor& ca, const entry& e) {
        return ca.acquire(base::insert(ca, e)); }
    bool insert(accessor& a, const entry& e) { return a.acquire(base::insert(a, e)); }
    bool insert(const entry& e) { return base::insert(e); }

    bool erase(const_accessor& ca) { ca.release(); return base::erase(ca); }
    bool erase(accessor& a) { a.release(); return base::erase(a); }
    bool erase(const K& k) { return base::erase(k); }

    using base::clear;

    using base::iterator;
    using base::const_iterator;
    using base::begin;
    using base::end;
};

template<typename T>
using dyn_c_vector = tbb::concurrent_vector<T, std::allocator<T>>;

template<typename T>
using dyn_c_queue = tbb::concurrent_queue<T, std::allocator<T>>;

typedef boost::mutex dyn_mutex;

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
};

class COMMON_EXPORT dyn_thread {
    int myid;
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
    dyn_threadlocal() : cache(), base() {};
    dyn_threadlocal(const T& d) : cache(), base(d) {};
    ~dyn_threadlocal() {};

    T get() {
        lock.lock_shared();
        if(cache.size() > dyn_thread::me) {
            lock.unlock_shared();
            return cache[dyn_thread::me];
        }
        lock.unlock_shared();
        lock.lock();
        if(cache.size() <= dyn_thread::me)
            cache.insert(cache.end(), dyn_thread::threads() - cache.size(), base);
        lock.unlock();
        return base;
    }

    void set(const T& val) {
        lock.lock_shared();
        if(cache.size() > dyn_thread::me) {
            cache[dyn_thread::me] = val;
            lock.unlock_shared();
            return;
        }
        lock.unlock_shared();
        lock.lock();
        if(cache.size() <= dyn_thread::me)
            cache.insert(cache.end(), dyn_thread::threads() - cache.size(), base);
        cache[dyn_thread::me] = val;
        lock.unlock();
    }
};

} // namespace Dyninst

#endif
