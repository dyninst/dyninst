/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: linux.h,v 1.15 2004/03/23 01:12:06 eli Exp $

#if !(defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4))
#error "invalid architecture-os inclusion"
#endif

#ifndef LINUX_PD_HDR
#define LINUX_PD_HDR

#include <sys/param.h>
#include "common/h/Types.h"

#if !defined( ia64_unknown_linux2_4 )
#define BYTES_TO_SAVE   256
#else
/* More than the number of bundles necessary for loadDYNINSTlib()'s code. */
#define CODE_BUFFER_SIZE	512
#define BYTES_TO_SAVE		(CODE_BUFFER_SIZE * 16)
#endif

#define EXIT_NAME "_exit"

#define SIGNAL_HANDLER	 "__restore"

typedef int handleT; // a /proc file descriptor

#if defined( ia64_unknown_linux2_4 )
#include "linux-ia64.h"
#elif defined( i386_unknown_linux2_0 )
#include "linux-x86.h"
#else
#error Invalid or unknown architecture-os inclusion
#endif

/* For linux.C */
Address getPC( int );
bool changePC( int pid, Address loc );
void generateBreakPoint( instruction & insn );
void printRegs( void *save );

class process;
class ptraceKludge {
private:
  static bool haltProcess(process *p);
  static void continueProcess(process *p, const bool halted);

public:
  static bool deliverPtrace(process *p, int req, Address addr, Address data);
  static int deliverPtraceReturn(process *p, int req, Address addr, Address data);
};

#include "linux-signals.h"

#endif
