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

// This file contains a simple implementation of a condition variable-based
// shared lock. The algorithm is described below.

// Linearization is handled via two bitfields, rin and rout, which govern the
// behavior and motion of readers through the lock. These are arranged as:
// - Bit 0: PHASE. Determines which wakeup signal waiting readers use.
// - Bit 1: WRITER. If 1, a writer has the lock or is waiting for it.
// - Remaining bits: TICKET. Unique identifier for a reader.
// It is assumed the number of readers will never go above 2^14 (16384)

// Incoming readers obtain a ticket by a fetch_add on rin, and if
// WRITER is set will immediately wait on a condition variable rcond.
// The associated mutex inlock maintains consistent access with the two wakeup
// booleans, rwakeup[2], and the choice of variable is determined by the
// PHASE bit of the obtained ticket.

// Outgoing readers indicate their disinterest by applying a fetch_add
// on rout, which gives them a a secondary "outgoing" ticket. If WRITER
// is set the non-atomic field last contains the ticket of the final reader.
// The final reader signals a condition variable wcond to wake up one of
// the writers that may be waiting. The normal mutex outlock and boolean wwakeup
// ensure no wakeups are lost on the writer side.

// Writers are serialized amonst each other via a single normal mutex wlock.
// All arriving writers choose a final reader via fetch_xor on rin, and
// check for present readers via fetch_xor on rout. If readers are present,
// it then waits for a wakeup via the condition variable wcond, resetting
// wwakeup after this has occured successfully. All of this is done while wlock
// is held, and wlock continues to be held through the critical section.

// Leaving writers first ensure outgoing readers will not attempt to
// (erroneously) wake up a writer via a fetch_xor (equiv. fetch_and)
// on rout, permit incoming readers via the same on rin, and then wake up
// any waiting readers via a notification on rcond.

// Inspired by the previous implementation, which was based on the following:
//   Björn B. Brandenburg and James H. Anderson. 2010. Spin-based reader-writer
//   synchronization for multiprocessor real-time systems. Real-Time Systems
//   46(1):25-87 (September 2010).  http://dx.doi.org/10.1007/s11241-010-9097-2
// In addition to the above, the previous authors used a simplification where
// the readers span on a single shared atomic boolean. The version below
// replaces this with a condition variable, which is sufficient for the
// as the readers only serialize at the transitions between phases.

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
