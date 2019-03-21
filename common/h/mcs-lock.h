//***************************************************************************
//
// File:
//   mcs_lock.h
//
// Purpose:
//   Define an API for the MCS lock: a fair queue-based lock.
//
// Reference:
//   John M. Mellor-Crummey and Michael L. Scott. 1991. Algorithms for scalable
//   synchronization on shared-memory multiprocessors. ACM Transactions on
//   Computing Systems 9, 1 (February 1991), 21-65.
//   http://doi.acm.org/10.1145/103727.103729
//***************************************************************************



#ifndef _mcs_lock_h_
#define _mcs_lock_h_

//******************************************************************************
// global includes
//******************************************************************************

#include <boost/atomic.hpp>


//******************************************************************************
// local includes
//******************************************************************************

#include "util.h"



//******************************************************************************
// types
//******************************************************************************

typedef struct mcs_node_s {
  boost::atomic<struct mcs_node_s*> next;
  boost::atomic<bool> blocked;
} mcs_node_t;


typedef struct {
  boost::atomic<mcs_node_t *> tail;
} mcs_lock_t;



//******************************************************************************
// constants
//******************************************************************************

#define mcs_nil (struct mcs_node_s*) 0

//******************************************************************************
// interface functions
//******************************************************************************

static inline void
mcs_init(mcs_lock_t &l)
{
  l.tail.store(mcs_nil);
}

COMMON_EXPORT void
mcs_lock(mcs_lock_t &l, mcs_node_t &me);


COMMON_EXPORT bool
mcs_trylock(mcs_lock_t &l, mcs_node_t &me);


COMMON_EXPORT void
mcs_unlock(mcs_lock_t &l, mcs_node_t &me);

#endif
