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

// This file contains a basic implementation of the MCS spin-based mutex.
// Reference:
//   John M. Mellor-Crummey and Michael L. Scott. 1991. Algorithms for scalable
//   synchronization on shared-memory multiprocessors. ACM Transactions on
//   Computing Systems 9, 1 (February 1991), 21-65.
//   http://doi.acm.org/10.1145/103727.103729

#include "vgannotations.h"
#include "locks.h"
#include <boost/memory_order.hpp>

thread_local dyn_mutex::node dyn_mutex::me;

dyn_mutex::dyn_mutex()
    : tail(NULL) {
    VALGRIND_HG_MUTEX_INIT_POST(this, 0 /* non-recursive */);
}

dyn_mutex::~dyn_mutex() {
    VALGRIND_HG_MUTEX_DESTROY_PRE(this);
}

void dyn_mutex::lock() {
    // HG should ignore the node, its races are handled via logic.
    VALGRIND_HG_DISABLE_CHECKING(&me, sizeof me);
    VALGRIND_HG_MUTEX_LOCK_PRE(this, 0 /* blocking */);

    // Make sure our node points nowhere.
    me.next.store(NULL, boost::memory_order_relaxed);

    // Install at the tail of the queue, and nab the predecessor.
    // Note: rel order fences the initialization of the node.
    node *predecessor = tail.exchange(&me, boost::memory_order_acq_rel);

    // Wait for my predecessor, if I have one.
    if (predecessor) {
        // Mark that we are in fact blocked.
        me.blocked.store(true);

        // Link with my predecessor. Rel ensures blocked is stored.
        predecessor->next.store(&me, boost::memory_order_release);

        // Spin until we actually acquire the lock.
        while (me.blocked.load(boost::memory_order_acquire));
    }

    // Tell Valgrind that we did things.
    VALGRIND_HG_MUTEX_LOCK_POST(this);
    ANNOTATE_HAPPENS_AFTER(this);
}

// The trylock is not currently used, it may never be. Disabled for now.
#if 0
bool dyn_mutex::trylock() {
    // HG should ignore the node, its races are handled via logic.
    VALGRIND_HG_DISABLE_CHECKING(&me, sizeof me);
    VALGRIND_HG_MUTEX_LOCK_PRE(this, 1 /* trylock form */);

    // Make sure our node points nowhere.
    me.next.store(NULL, boost::memory_order_relaxed);

    // Attempt to acquire the lock, but only if there was no one there.
    // If we fail, we don't need to enforce any memory order.
    node *tmp = NULL;
    bool locked = tail.compare_exchange_strong(tmp, &me,
                                           boost::memory_order_acq_rel,
                                           boost::memory_order_relaxed);

    // Tell Valgrind if we did it, and re-enable the node if we failed..
    if (locked) {
        VALGRIND_HG_MUTEX_LOCK_POST(this);
        ANNOTATE_HAPPENS_AFTER(this);
    } else VALGRIND_HG_ENABLE_CHECKING(&me, sizeof me);

    return locked;
}
#endif // 0

void dyn_mutex::unlock() {
    // Let Valgrind know what we are up to.
    VALGRIND_HG_MUTEX_UNLOCK_PRE(this);
    ANNOTATE_HAPPENS_BEFORE(this);

    node *successor = me.next.load(boost::memory_order_acquire);
    if (!successor) {
        // At (or was at) the tail of the queue. Try ripping myself out.
        node *tmp = &me;
        if (tail.compare_exchange_strong(tmp, NULL,
                                           boost::memory_order_release,
                                           boost::memory_order_relaxed)) {
            // We did it, we have no more connection with the lock. Carry on.
            VALGRIND_HG_MUTEX_UNLOCK_POST(this);
            return;
        }

        // Someone out there is fiddling with my node. Wait around for it.
        while (!(successor = me.next.load(boost::memory_order_acquire)));
    }

    // Pass the lock to my successor, I don't need it anymore.
    successor->blocked.store(false, boost::memory_order_release);

    // Let Valgrind know about our little expedition.
    VALGRIND_HG_MUTEX_UNLOCK_POST(this);
    VALGRIND_HG_ENABLE_CHECKING(&me, sizeof me);
}
