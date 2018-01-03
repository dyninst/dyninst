//******************************************************************************
//
// File:
//   mcs_lock.c
//
// Purpose:
//   Implement an API for the MCS lock: a fair queue-based lock.
//
// Reference:
//   John M. Mellor-Crummey and Michael L. Scott. 1991. Algorithms for scalable
//   synchronization on shared-memory multiprocessors. ACM Transactions on
//   Computing Systems 9, 1 (February 1991), 21-65.
//   http://doi.acm.org/10.1145/103727.103729
//******************************************************************************



//******************************************************************************
// local includes
//******************************************************************************

#include "mcs-lock.h"
#include "race-detector-annotations.h"

//******************************************************************************
// private operations
//******************************************************************************

//******************************************************************************
// interface operations
//******************************************************************************

void
mcs_lock(mcs_lock_t &l, mcs_node_t &me)
{
  race_detector_fake_lock_acquire();

  //--------------------------------------------------------------------
  // initialize my queue node
  //--------------------------------------------------------------------
  atomic_init(&me.next, mcs_nil);

  //--------------------------------------------------------------------
  // install my node at the tail of the lock queue.
  // determine my predecessor, if any.
  //
  // note: the rel aspect of the ordering below ensures that
  // initialization of me->next completes before anyone sees my node
  //--------------------------------------------------------------------
  mcs_node_t *predecessor =
    atomic_exchange_explicit(&l.tail, &me, std::memory_order_acq_rel);

  //--------------------------------------------------------------------
  // if I have a predecessor, wait until it signals me
  //--------------------------------------------------------------------
  if (predecessor != mcs_nil) {
    //------------------------------------------------------------------
    // prepare to block until signaled by my predecessor
    //------------------------------------------------------------------
    atomic_init(&me.blocked, true);

    //------------------------------------------------------------------
    // link behind my predecessor
    // note: use release to ensure that prior assignment to blocked
    //       occurs first
    //------------------------------------------------------------------
    atomic_store_explicit(&predecessor->next, &me, std::memory_order_release);

    //------------------------------------------------------------------
    // wait for my predecessor to clear my flag
    // note: use acquire order to ensure that reads or writes in the
    //       critical section will not occur until after blocked is
    //       cleared
    //------------------------------------------------------------------
    while (atomic_load_explicit(&me.blocked, std::memory_order_acquire));
  }
}


bool
mcs_trylock(mcs_lock_t &l, mcs_node_t &me)
{
  race_detector_fake_lock_acquire();
  //--------------------------------------------------------------------
  // initialize my queue node
  //--------------------------------------------------------------------
  atomic_store_explicit(&me.next, mcs_nil, std::memory_order_relaxed);

  //--------------------------------------------------------------------
  // if the tail pointer is nil, swap it with a pointer to me, which
  // acquires the lock and installs myself at the tail of the queue.
  // note: the acq_rel ordering ensures that
  // (1) rel: my store of me->next above completes before the exchange
  // (2) acq: any accesses after the exchange can't begin until after
  //     the exchange completes.
  //--------------------------------------------------------------------
  mcs_node_t *oldme = mcs_nil;
  bool locked = 
    atomic_compare_exchange_strong_explicit(&l.tail, &oldme, &me,
					    std::memory_order_acq_rel,
					    std::memory_order_relaxed);
  if (!locked) {
    race_detector_fake_lock_release();
  }
  return locked;
}


void
mcs_unlock(mcs_lock_t &l, mcs_node_t &me)
{
  mcs_node_t *successor = atomic_load_explicit(&me.next, std::memory_order_acquire);

  if (successor == mcs_nil) {
    //--------------------------------------------------------------------
    // I don't currently have a successor, so I may be at the tail
    //--------------------------------------------------------------------

    //--------------------------------------------------------------------
    // if my node is at the tail of the queue, attempt to remove myself
    // note: release order below on success guarantees that all accesses
    //       above the exchange must complete before the exchange if the
    //       exchange unlinks me from the tail of the queue
    //--------------------------------------------------------------------
    mcs_node_t *oldme = &me;

    if (atomic_compare_exchange_strong_explicit(&l.tail, &oldme, mcs_nil,
						std::memory_order_release,
						std::memory_order_relaxed)) {
      //------------------------------------------------------------------
      // I removed myself from the queue; I will never have a
      // successor, so I'm done
      //------------------------------------------------------------------
      race_detector_fake_lock_release();
      return;
    }

    //------------------------------------------------------------------
    // another thread is writing me->next to define itself as our successor;
    // wait for it to finish that
    //------------------------------------------------------------------
    while (mcs_nil == (successor = atomic_load_explicit(&me.next, std::memory_order_acquire)));
  }

  atomic_store_explicit(&successor->blocked, false, std::memory_order_release);
  race_detector_fake_lock_release();
}
