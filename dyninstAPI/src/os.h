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
 * Revision 1.17  1998/08/25 19:35:17  buck
 * Initial commit of DEC Alpha port.
 *
 * Revision 1.1.1.3  1998/02/04  01:06:48  buck
 * Import latest changes from Wisconsin into Maryland repository.
 *
 * Revision 1.16  1997/12/01 02:30:19  tung
 * For Linux/X86 port
 *
 * Revision 1.15  1997/04/29 23:16:07  mjrg
 * Changes for WindowsNT port
 * Delayed check for DYNINST symbols to allow linking libdyninst dynamically
 * Changed way paradyn and paradynd generate resource ids
 * Changes to instPoint class in inst-x86.C to reduce size of objects
 * Added initialization for process->threads to fork and attach constructors
 *
 * Revision 1.14  1997/02/26 23:42:55  mjrg
 * First part on WindowsNT port: changes for compiling with Visual C++;
 * moved unix specific code to unix.C
 *
 * Revision 1.13  1997/02/21 20:13:41  naim
 * Moving files from paradynd to dyninstAPI + moving references to dataReqNode
 * out of the ast class. The is the first pre-dyninstAPI commit! - naim
 *
 * Revision 1.12  1996/11/05 20:33:19  tamches
 * some OS:: methods have changed to process:: methods
 *
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
 */

#if defined(sparc_sun_sunos4_1_3)
#include "dyninstAPI/src/sunos.h"
#elif defined(sparc_sun_solaris2_4) || defined(i386_unknown_solaris2_5)
#include "dyninstAPI/src/solaris.h"
#elif defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
#include "dyninstAPI/src/aix.h"
#elif defined(alpha_dec_osf4_0)
#include "dyninstAPI/src/alpha.h"
#elif defined(hppa1_1_hp_hpux)
#include "dyninstAPI/src/hpux.h"
#elif defined(i386_unknown_nt4_0)
#include "dyninstAPI/src/pdwinnt.h"
#elif defined(i386_unknown_linux2_0)
#include "dyninstAPI/src/linux.h"

#endif

#include "util/h/String.h"
#include "util/h/Types.h"

class OS {
public:
  static void osTraceMe(void);
  static void osDisconnect(void);
  static bool osKill(int);

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
