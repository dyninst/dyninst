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

#include "vgannotations.h"
#include "concurrent.h"

static const unsigned int PHASE = 0x1;
static const unsigned int WRITER = 0x2;
static const unsigned int TICKET = 0x4;

using namespace Dyninst;

dyn_rwlock::dyn_rwlock()
    : rin(0), rout(0), last(0), rwakeup{false,false}, wwakeup(false) {
    ANNOTATE_RWLOCK_CREATE(this);
}

dyn_rwlock::~dyn_rwlock() {
    ANNOTATE_RWLOCK_DESTROY(this);
}

void dyn_rwlock::lock_shared() {
    // Register a ticket, and check for writers.
    unsigned int ticket = rin.fetch_add(TICKET, boost::memory_order_acquire);
    unsigned int phase = ticket & PHASE;

    if (ticket & WRITER) {
        // There is a writer present, try to wait.
        boost::unique_lock<boost::mutex> l(inlock);
        rcond.wait(l, [this,&phase](){ return rwakeup[phase]; });
    }

    // Tell Valgrind all about it
    ANNOTATE_RWLOCK_ACQUIRED(this, 0 /* reader mode */);
    ANNOTATE_HAPPENS_AFTER(&wwakeup);
}

void dyn_rwlock::unlock_shared() {
    // Tell Valgrind what we're up to
    ANNOTATE_HAPPENS_BEFORE(&rwakeup);
    ANNOTATE_RWLOCK_RELEASED(this, 0 /* reader mode */);

    // Pull off an outgoing ticket and see if we're the last reader.
    unsigned int ticket = rout.fetch_add(TICKET, boost::memory_order_acq_rel);

    if (ticket & WRITER && ticket == last) {
        // Wake up the writer, its our job.
        boost::unique_lock<dyn_mutex> l(outlock);
        wwakeup = true;
        wcond.notify_one();
    }
}

void dyn_rwlock::lock() {
    // Synchronize with other writers via the normal mutex.
    wlock.lock();

    // Choose the final reader, and make sure no others come in.
    unsigned int lr = rin.fetch_xor(PHASE|WRITER, boost::memory_order_acquire);
    last = (lr - TICKET) ^ (PHASE | WRITER);

    // Let the last reader know that they should wake me up.
    // Rel to "release" the previous write to last.
    unsigned int cr = rout.fetch_xor(PHASE|WRITER, boost::memory_order_acq_rel);

    if (cr != lr) {
        // There actually was a reader inside. Wait for him to leave.
        boost::unique_lock<boost::mutex> l(outlock);
        wcond.wait(l, [this](){ return wwakeup; });
        wwakeup = false;
    }

    // Now there are no readers waiting on the previous phase, clear the flag.
    rwakeup[lr & PHASE] = false;

    // Tell Valgrind all about it.
    ANNOTATE_RWLOCK_ACQUIRED(this, 1 /* writer mode */);
    ANNOTATE_HAPPENS_AFTER(&rwakeup);
}

void dyn_rwlock::unlock() {
    // Let Valgrind know what we are up to.
    ANNOTATE_HAPPENS_BEFORE(&wwakeup);
    ANNOTATE_RWLOCK_RELEASED(this, 1 /* writer mode */);

    // Make sure any speedy readers don't try to wake me up again.
    rout.fetch_xor(WRITER, boost::memory_order_relaxed);

    // Let in any new readers, since we're done with it at the moment.
    unsigned int phase = rin.fetch_xor(WRITER, boost::memory_order_acq_rel);
    phase &= PHASE;

    // Wake up any readers that were waiting for us.
    {
        boost::unique_lock<dyn_mutex> l(inlock);
        rwakeup[phase] = true;
        rcond.notify_all();
    }

    // We're done, pass ownership off to the next writer.
    wlock.unlock();
}
