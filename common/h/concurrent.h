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

typedef boost::mutex dyn_mutex;

template<typename K, typename V>
using dyn_c_hash_map = tbb::concurrent_hash_map<K, V, tbb::tbb_hash_compare<K>,
    std::allocator<std::pair<K,V>>>;

template<typename T>
using dyn_c_vector = tbb::concurrent_vector<T, std::allocator<T>>;

template<typename T>
using dyn_c_queue = tbb::concurrent_queue<T, std::allocator<T>>;

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

}

#endif
