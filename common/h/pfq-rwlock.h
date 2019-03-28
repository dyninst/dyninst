//******************************************************************************
//
// File:
//   pfq_rwlock.h
//
// Purpose:
//   Define the API for a fair, phased reader-writer lock with local spinning
//
// Reference:
//   Björn B. Brandenburg and James H. Anderson. 2010. Spin-based reader-writer
//   synchronization for multiprocessor real-time systems. Real-Time Systems
//   46(1):25-87 (September 2010).  http://dx.doi.org/10.1007/s11241-010-9097-2
//
// Notes:
//   the reference uses a queue for arriving readers. on a cache coherent
//   machine, the local spinning property for waiting readers can be achieved
//   by simply using a cacheble flag. the implementation here uses that
//   simplification.
//
//******************************************************************************

#ifndef __pfq_rwlock_h__
#define __pfq_rwlock_h__

#include <boost/atomic.hpp>
#include <boost/thread/shared_mutex.hpp>
#include "mcs-lock.h"

typedef int pfq_rwlock_node_t;
typedef boost::shared_mutex pfq_rwlock_t;

#define pfq_rwlock_init(X)
#define pfq_rwlock_read_lock(_l) (_l).lock_shared()
#define pfq_rwlock_read_unlock(_l) (_l).unlock_shared()
#define pfq_rwlock_write_lock(_l, _n) ((_l).lock(),_n=0,_n++)
#define pfq_rwlock_write_unlock(_l, _n) ((_l).unlock(),_n=0,_n++)

#if 0

//******************************************************************************
// global includes
//******************************************************************************

#include <inttypes.h>


//******************************************************************************
// local includes
//******************************************************************************

#include "mcs-lock.h"


//******************************************************************************
// macros
//******************************************************************************

// align a variable at the start of a cache line
#define cache_aligned __attribute__((aligned(128)))



//******************************************************************************
// types
//******************************************************************************

typedef mcs_node_t pfq_rwlock_node_t;

typedef struct bigbool {
  boost::atomic<bool> bit cache_aligned;
} bigbool;

typedef struct {
  //----------------------------------------------------------------------------
  // reader management
  //----------------------------------------------------------------------------
  boost::atomic<uint_least32_t> rin cache_aligned;  // = 0
  boost::atomic<uint_least32_t> rout cache_aligned;  // = 0
  boost::atomic<uint_least32_t> last cache_aligned;  // = WRITER_PRESENT
  bigbool writer_blocking_readers[2]; // false

  //----------------------------------------------------------------------------
  // writer management
  //----------------------------------------------------------------------------
  mcs_lock_t wtail cache_aligned;  // init
  mcs_node_t *whead cache_aligned;  // null

} pfq_rwlock_t;



//******************************************************************************
// interface operations
//******************************************************************************

COMMON_EXPORT void 
pfq_rwlock_init(pfq_rwlock_t &l);

COMMON_EXPORT void 
pfq_rwlock_read_lock(pfq_rwlock_t &l);

COMMON_EXPORT void 
pfq_rwlock_read_unlock(pfq_rwlock_t &l);

COMMON_EXPORT void 
pfq_rwlock_write_lock(pfq_rwlock_t &l, pfq_rwlock_node_t &me);

COMMON_EXPORT void 
pfq_rwlock_write_unlock(pfq_rwlock_t &l, pfq_rwlock_node_t &me);
#endif // 0


#endif
