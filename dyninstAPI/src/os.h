
#ifndef _OS_HDR
#define _OS_HDR

/*
 * This is an initial attempt at providing an os abstraction for the paradynd
 * I am doing this so I can compile the paradynd on solaris
 *
 * This should enforce the abstract os operations
 */ 

/*
 * $Log: os.h,v $
 * Revision 1.5  1995/05/25 16:08:47  markc
 * Include files for solaris 2.4
 *
 * Revision 1.4  1995/05/18  10:39:54  markc
 * Added calls to getrusage
 *
 * Revision 1.3  1995/02/16  08:53:52  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.2  1995/02/16  08:34:17  markc
 * Changed igen interfaces to use strings/vectors rather than charigen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.1  1994/11/01  16:50:03  markc
 * Abstract os support.  No os specific code here.  Includes os specific
 * file.
 *
 */

#if defined(sparc_sun_sunos4_1_3)
#include "sunos.h"
#elif defined(sparc_sun_solaris2_3)
#include "solaris.h"
#elif defined(sparc_sun_solaris2_4)
#include "solaris.h"
#elif defined(sparc_tmc_cmost7_3)
#include "cmost.h"
#endif

#include "util/h/String.h"
#include "util/h/Types.h"

class OS {
public:
  static bool osAttach(pid_t process_id);
  static bool osStop(pid_t process_id);
  static bool osDumpCore(pid_t pid, const string dumpTo);
  static bool osDumpImage(const string &imageFileName, pid_t pid, const Address a);
  static bool osForwardSignal(pid_t pid, int status);
  static void osTraceMe(void);
  static void osDisconnect(void);

  // getrusage is a bsd system call, sunos, solaris, and hp seem to support it
  // TODO -- what if this sys call is unsupported
  static float compute_rusage_cpu();
  static float compute_rusage_sys();
  static float compute_rusage_min();
  static float compute_rusage_maj();
  static float compute_rusage_swap();
  static float compute_rusage_io_in();
  static float compute_rusage_io_out();
  static float compute_rusage_msg_send();
  static float compute_rusage_msg_recv();
  static float compute_rusage_sigs();
  static float compute_rusage_vol_cs();
  static float compute_rusage_inv_cs();
};

#endif
