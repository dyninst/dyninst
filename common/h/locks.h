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

#ifndef _LOCKS_H_
#define _LOCKS_H_

#include "util.h"
#include <boost/atomic.hpp>
#include <inttypes.h>

class COMMON_EXPORT dyn_mutex {
    friend class dyn_rwlock;
    struct node {
        boost::atomic<node*> next;
        boost::atomic<bool> blocked;
        node() : next(NULL), blocked(false) {};
    };

    static thread_local node me;

    boost::atomic<node*> tail;
public:
    dyn_mutex();
    ~dyn_mutex();

    void lock();
    void unlock();
};

#define cache_aligned __attribute__((aligned(128)))

class COMMON_EXPORT dyn_rwlock {
    struct bigbool {
        boost::atomic<bool> bit cache_aligned;
    };

    // Reader management members
    boost::atomic<uint_least32_t> rin cache_aligned;
    boost::atomic<uint_least32_t> rout cache_aligned;
    boost::atomic<uint_least32_t> last cache_aligned;
    bigbool writer_blocking_readers[2];

    // Writer management members
    dyn_mutex wtail cache_aligned;
    dyn_mutex::node *whead cache_aligned;
public:
    dyn_rwlock();
    ~dyn_rwlock();

    void lock_shared();
    void unlock_shared();
    void lock();
    void unlock();
};

#endif
