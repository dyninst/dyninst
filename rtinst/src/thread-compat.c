/*
 * Thread compatibility code file
 */

#if defined(rs6000_ibm_aix4_1)
/* ... */
#endif

#if defined(sparc_sun_solaris2_4)
#include <thread.h>
#include "thread-compat.h"

inline void *P_thread_getspecific(dyninst_key_t key)
{
  void *result;
  thr_getspecific(key, &result);
  return result;
}
#endif
