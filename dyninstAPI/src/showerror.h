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

#ifndef SHOWERROR_H
#define SHOWERROR_H

#include "common/h/String.h"
#include "common/h/Pair.h"

extern void showErrorCallback(int num, pdstring msg);

extern void showInfoCallback(pdstring msg);

#include "BPatch.h"
#define BPatch_reportError(a,b,c) BPatch::reportError(a,b,c)

extern int dyn_debug_signal;
extern int dyn_debug_infrpc;
extern int dyn_debug_startup;
extern int dyn_debug_parsing;
extern int dyn_debug_forkexec;
extern int dyn_debug_proccontrol;
extern int dyn_debug_stackwalk;

// C++ prototypes
#define signal_cerr       if (dyn_debug_signal) cerr
#define inferiorrpc_cerr  if (dyn_debug_infrpc) cerr
#define startup_cerr      if (dyn_debug_startup) cerr
#define parsing_cerr      if (dyn_debug_parsing) cerr
#define forkexec_cerr     if (dyn_debug_forkexec) cerr
#define proccontrol_cerr  if (dyn_debug_proccontrol) cerr
#define stackwalk_cerr    if (dyn_debug_stackwalk) cerr

// C prototypes
extern int signal_printf(const char *format, ...);
extern int inferiorrpc_printf(const char *format, ...);
extern int startup_printf(const char *format, ...);
extern int parsing_printf(const char *format, ...);
extern int forkexec_printf(const char *format, ...);
extern int proccontrol_printf(const char *format, ...);
extern int stackwalk_printf(const char *format, ...);

// And initialization
extern bool init_debug();

#endif /* SHOWERROR_H */
