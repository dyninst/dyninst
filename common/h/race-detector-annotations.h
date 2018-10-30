#ifndef __race_detector_annotations_h__
#define __race_detector_annotations_h__

#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ENABLE_RACE_DETECTION
  COMMON_EXPORT void race_detector_fake_lock_acquire(void *fake_lock);
  COMMON_EXPORT void race_detector_fake_lock_release(void *fake_lock);
  COMMON_EXPORT void race_detector_forget_access_history(void *loc, unsigned int nbytes);
  
#else
#define race_detector_fake_lock_acquire(fake_lock)
#define race_detector_fake_lock_release(fake_lock)
#define race_detector_forget_access_history(loc, nbytes) 
#endif

#define race_detector_fake_lock(lock) ((void *) &lock)

#ifdef __cplusplus
}
#endif

#endif
