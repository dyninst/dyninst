//******************************************************************************
//
// File:
//   pfq_rwlock.c
//
// Purpose:
//   Implement the API for a fair, phased reader-writer lock with local spinning
//
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
//******************************************************************************



//******************************************************************************
// local includes
//******************************************************************************

#include "pfq-rwlock.h"
#include "race-detector-annotations.h"



//******************************************************************************
// macros
//******************************************************************************

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

//******************************************************************************
// interface operations
//******************************************************************************

void
pfq_rwlock_init(pfq_rwlock_t &l)
{
  atomic_init(&l.rin, 0U);
  atomic_init(&l.rout, 0U);
  atomic_init(&l.last, 0U);
  atomic_init(&l.writer_blocking_readers[0].bit, false);
  atomic_init(&l.writer_blocking_readers[1].bit, false);
  mcs_init(l.wtail);
  l.whead = mcs_nil;
}

void
pfq_rwlock_read_lock(pfq_rwlock_t &l)
{
  race_detector_fake_lock_acquire();
  uint32_t ticket = atomic_fetch_add_explicit(&l.rin, READER_INCREMENT, std::memory_order_acq_rel);

  if (ticket & WRITER_PRESENT) {
    uint32_t phase = ticket & PHASE_BIT;
    while (atomic_load_explicit(&l.writer_blocking_readers[phase].bit, std::memory_order_acquire));
  }
}


void
pfq_rwlock_read_unlock(pfq_rwlock_t &l)
{
  uint32_t ticket = atomic_fetch_add_explicit(&l.rout, READER_INCREMENT, std::memory_order_acq_rel);

  if (ticket & WRITER_PRESENT) {
    //----------------------------------------------------------------------------
    // finish reading counter before reading last
    //----------------------------------------------------------------------------
    if (ticket == atomic_load_explicit(&l.last, std::memory_order_acquire))
      atomic_store_explicit(&l.whead->blocked, false, std::memory_order_release);
  }
  race_detector_fake_lock_release();
}


void
pfq_rwlock_write_lock(pfq_rwlock_t &l, pfq_rwlock_node_t &me)
{
  //--------------------------------------------------------------------
  // use MCS lock to enforce mutual exclusion with other writers
  //--------------------------------------------------------------------
  mcs_lock(l.wtail, me);

  //--------------------------------------------------------------------
  // this may be false when at the head of the mcs queue
  //--------------------------------------------------------------------
  atomic_store_explicit(&me.blocked, true, std::memory_order_relaxed);

  //--------------------------------------------------------------------
  // announce myself as next writer
  //--------------------------------------------------------------------
  l.whead = &me;

  //--------------------------------------------------------------------
  // set writer_blocking_readers to block any readers in the next batch
  //--------------------------------------------------------------------
  uint32_t phase = atomic_load_explicit(&l.rin, std::memory_order_relaxed) & PHASE_BIT;
  atomic_store_explicit(&l.writer_blocking_readers[phase].bit, true, std::memory_order_release); 

  //----------------------------------------------------------------------------
  // store to writer_blocking_headers bit must complete before incrementing rin
  //----------------------------------------------------------------------------

  //--------------------------------------------------------------------
  // acquire an "in" sequence number to see how many readers arrived
  // set the WRITER_PRESENT bit so subsequent readers will wait
  //--------------------------------------------------------------------
  uint32_t in = atomic_fetch_or_explicit(&l.rin, WRITER_PRESENT, std::memory_order_acq_rel);

  //--------------------------------------------------------------------
  // save the ticket that the last reader will see
  //--------------------------------------------------------------------
  atomic_store_explicit(&l.last, in - READER_INCREMENT + WRITER_PRESENT, std::memory_order_release);

  //-------------------------------------------------------------
  // update to 'last' must complete before others see changed value of rout.
  // acquire an "out" sequence number to see how many readers left
  // set the WRITER_PRESENT bit so the last reader will know to signal
  // it is responsible for signaling the waiting writer
  //-------------------------------------------------------------
  uint32_t out = atomic_fetch_or_explicit(&l.rout, WRITER_PRESENT, std::memory_order_acq_rel);

  //--------------------------------------------------------------------
  // if any reads are active, wait for last reader to signal me
  //--------------------------------------------------------------------
  if (in != out) {
    while (atomic_load_explicit(&me.blocked, std::memory_order_acquire));
    // wait for active reads to drain

    //--------------------------------------------------------------------------
    // store to writer_blocking headers bit must complete before notifying
    // readers of writer
    //--------------------------------------------------------------------------
  }
}


void
pfq_rwlock_write_unlock(pfq_rwlock_t &l, pfq_rwlock_node_t &me)
{
  //--------------------------------------------------------------------
  // toggle phase and clear WRITER_PRESENT in rin. No synch issues
  // since there are no concurrent updates of the low-order byte
  //--------------------------------------------------------------------
  unsigned char *lsb = LSB_PTR(&l.rin);
  uint32_t phase = *lsb & PHASE_BIT;
  *lsb ^= WRITER_MASK;

  //--------------------------------------------------------------------
  // toggle phase and clear WRITER_PRESENT in rout. No synch issues
  // since the low-order byte modified here isn't modified again until
  // another writer has the mcs_lock.
  //--------------------------------------------------------------------
  lsb = LSB_PTR(&l.rout);
  *lsb ^= WRITER_MASK;

  //----------------------------------------------------------------------------
  // clearing writer present in rin can be reordered with writer_blocking_readers set below
  // because any arriving reader will see the cleared writer_blocking_readers and proceed.
  //----------------------------------------------------------------------------

  //--------------------------------------------------------------------
  // clear writer_blocking_readers to release waiting readers in the current read phase
  //--------------------------------------------------------------------
  atomic_store_explicit(&l.writer_blocking_readers[phase].bit, false, std::memory_order_release);

  //--------------------------------------------------------------------
  // pass writer lock to next writer
  //--------------------------------------------------------------------
  mcs_unlock(l.wtail, me);
}
