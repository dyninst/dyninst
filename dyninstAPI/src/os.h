/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

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
 * Revision 1.11  1996/10/31 08:51:40  tamches
 * removed osForwardSignal
 *
 * Revision 1.10  1996/10/18 23:54:07  mjrg
 * Solaris/X86 port
 *
 * Revision 1.9  1996/08/16 21:19:28  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.8  1996/05/12 05:16:58  tamches
 * aix 4.1 commit
 *
 * Revision 1.7  1995/08/24 15:04:24  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.6  1995/05/30  05:05:00  krisna
 * upgrade from solaris-2.3 to solaris-2.4.
 * architecture-os based include protection of header files.
 * removed architecture-os dependencies in generic sources.
 * changed ST_* symbol names to PDST_* (to avoid conflict on HPUX)
 *
 * Revision 1.5  1995/05/25  16:08:47  markc
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
#elif defined(sparc_sun_solaris2_4) || defined(i386_unknown_solaris2_5)
#include "solaris.h"
#elif defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
#include "aix.h"
#elif defined(sparc_tmc_cmost7_3)
#include "cmost.h"
#elif defined(hppa1_1_hp_hpux)
#include "hpux.h"
#endif

#include "util/h/String.h"
#include "util/h/Types.h"

class OS {
public:
  static bool osAttach(pid_t process_id);
  static bool osStop(pid_t process_id);
  static bool osDumpCore(pid_t pid, string dumpTo);
  static bool osDumpImage(const string &, pid_t, Address);
//  static bool osForwardSignal(pid_t pid, int status);
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
