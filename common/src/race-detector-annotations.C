//***************************************************************************
//
// File:
//   race-detector-annotations.C
//
// Purpose:
//   Inform a race detector about invariants known to the programmer 
//   that are relevant to race detection. Specifically, let the 
//   programmer know:
//   (1) a code region may be treated as if it were executed under
//       mutual exclusion. no accesses in the region will be endpoints
//       of race reports.
//   (2) forget about the prior access history associated with a region
//       of memory. this enables memory that has been allocated and freed
//       by one thread to be reused by another without the appearance of
//       any races.
//
//
// Description:
//   void fake_lock_acquire(void)
//
//   void fake_lock_release(void)
//
// Note:
//   the contents of this file will be ignored unless CILKSCREEN is defined
//
//***************************************************************************

#ifdef ENABLE_RACE_DETECTION

//****************************************************************************
// system include files
//****************************************************************************

#ifdef __INTEL_COMPILER
#include <cilktools/cilkscreen.h>
#endif



//****************************************************************************
// local include files
//****************************************************************************

#include "race-detector-annotations.h"



//****************************************************************************
// public operations
//****************************************************************************


void 
race_detector_fake_lock_acquire(void *fake_lock)
{
#ifdef __INTEL_COMPILER
  __cilkscreen_acquire_lock(fake_lock);
#endif
}


void 
race_detector_fake_lock_release(void *fake_lock)
{
#ifdef __INTEL_COMPILER
  __cilkscreen_release_lock(fake_lock);
#endif
}


void 
race_detector_forget_access_history(void *loc, unsigned int nbytes)
{
#ifdef __INTEL_COMPILER
  __cilkscreen_clean(loc, ((char *) loc) + nbytes);
#endif
}

#endif
