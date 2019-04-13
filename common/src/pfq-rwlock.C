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

// This file contains a simplified implementation of a spin-based shared lock.
// Reference:
//   Björn B. Brandenburg and James H. Anderson. 2010. Spin-based reader-writer
//   synchronization for multiprocessor real-time systems. Real-Time Systems
//   46(1):25-87 (September 2010).  http://dx.doi.org/10.1007/s11241-010-9097-2
//
// Notes:
//   the reference uses a queue for arriving readers. on a cache coherent
//   machine, the local spinning property for waiting readers can be achieved
//   by simply using a cacheable flag. the implementation here uses that
//   simplification.
//

#include "vgannotations.h"
#include "locks.h"

#define READER_INCREMENT 0x100U

#define PHASE_BIT        0x001U
#define WRITER_PRESENT   0x002U

#define WRITER_MASK      (PHASE_BIT | WRITER_PRESENT)
#define TICKET_MASK      ~(WRITER_MASK)

//------------------------------------------------------------------
// define a macro to point to the low-order byte of an integer type
// in a way that will work on both big-endian and little-endian 
// processors
//------------------------------------------------------------------
#ifdef DYNINST_BIG_ENDIAN
#define LSB_PTR(p) (((unsigned char *) p) + (sizeof(*p) - 1))
#endif


#ifdef DYNINST_LITTLE_ENDIAN
#define LSB_PTR(p) ((unsigned char *) p)
#endif

#ifndef LSB_PTR
#error "endianness must be configured. " \
       "use --enable-endian to force configuration"
#endif

dyn_rwlock::dyn_rwlock() {
    rin.store(0U);
    rout.store(0U);
    last.store(0U);
    writer_blocking_readers[0].bit.store(false);
    writer_blocking_readers[1].bit.store(false);

    // This algorithm does a number of weird and probably invalid tricks.
    // Tell HG to just... look the other way for now.
    VALGRIND_HG_DISABLE_CHECKING(rin, sizeof rin);
    VALGRIND_HG_DISABLE_CHECKING(rout, sizeof rout);
    VALGRIND_HG_DISABLE_CHECKING(last, sizeof last);
    VALGRIND_HG_DISABLE_CHECKING(writer_blocking_readers,
        sizeof writer_blocking_readers);

    ANNOTATE_RWLOCK_CREATE(this);
}

dyn_rwlock::~dyn_rwlock() {
    ANNOTATE_RWLOCK_DESTROY(this);
}

void dyn_rwlock::lock_shared() {
    uint32_t ticket = rin.fetch_add(READER_INCREMENT, boost::memory_order_acq_rel);

    if (ticket & WRITER_PRESENT) {
        uint32_t phase = ticket & PHASE_BIT;
        while (writer_blocking_readers[phase].bit.load(boost::memory_order_acquire));
    }

    ANNOTATE_RWLOCK_ACQUIRED(this, 0 /* reader mode */);
    ANNOTATE_HAPPENS_AFTER(&last);
}

void dyn_rwlock::unlock_shared() {
    ANNOTATE_HAPPENS_BEFORE(&last);
    ANNOTATE_RWLOCK_RELEASED(this, 0 /* reader mode */);

    uint32_t ticket = rout.fetch_add(READER_INCREMENT, boost::memory_order_acq_rel);

    if (ticket & WRITER_PRESENT) {
        if (ticket == last.load(boost::memory_order_acquire))
            whead->blocked.store(false, boost::memory_order_release);
    }
}

void dyn_rwlock::lock() {
    // Use our full-blown mutex to order with any other writers.
    wtail.lock();

    // MCS has a case where the mutex isn't blocked. We handle that here.
    dyn_mutex::me.blocked.store(true, boost::memory_order_relaxed);

    // Mark myself as the next writer for whenever the readers are done.
    whead = &dyn_mutex::me;

    // Stop any new readers from coming in by marking the corrosponding bit.
    // Rel because we need to ensure our position at the head is secure.
    uint32_t phase = rin.load(boost::memory_order_relaxed) & PHASE_BIT;
    writer_blocking_readers[phase].bit.store(true, boost::memory_order_release);

    // Get a sequence number for the final reader, and tell them to wait.
    // Rel because we need the above to finish before we tell new readers.
    uint32_t in = rin.fetch_or(WRITER_PRESENT, boost::memory_order_acq_rel);

    // Tell the readers who the last reader will be.
    last.store((in - READER_INCREMENT) | WRITER_PRESENT, boost::memory_order_release);

    // Let the leaving readers know that we are present.
    // Rel because we need to ensure that they see the correct value for last.
    uint32_t out = rout.fetch_or(WRITER_PRESENT, boost::memory_order_acq_rel);

    // If there is still a reader left out there, wait for them to tap me.
    if (in != out)
        while (dyn_mutex::me.blocked.load(boost::memory_order_acquire));

    // Tell Valgrind all about it.
    ANNOTATE_RWLOCK_ACQUIRED(this, 1 /* writer mode */);
    ANNOTATE_HAPPENS_AFTER(&last);
    ANNOTATE_HAPPENS_AFTER(&rout);
}

void dyn_rwlock::unlock() {
    ANNOTATE_HAPPENS_BEFORE(&rout);

    // Toggle the phase to let new readers in.
    // Note: Apparently this is "safe" because the low-order byte is locked.
    // Last I remember C++11's memory model was based on entire objects,
    // not bytes. Which might be why Valgrind was screaming. Caveat emptor.
    unsigned char *lsb = LSB_PTR(&rin);
    uint32_t phase = *lsb & PHASE_BIT;
    *lsb ^= WRITER_MASK;

    // Toggle the phase to let the readers out.
    // Note: Apparently "safe" for the same reason. Caveat emptor.
    lsb = LSB_PTR(&rout);
    *lsb ^= WRITER_MASK;

    // Let any readers that were standing around come in.
    writer_blocking_readers[phase].bit.store(false, boost::memory_order_release);

    // Let Valgrind know about our little quest.
    ANNOTATE_RWLOCK_RELEASED(this, 1 /* writer mode */);

    // And pawn our responsibilities off to the next writer.
    wtail.unlock();
}
