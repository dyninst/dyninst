/*
 * Copyright (c) 2000 Barton P. Miller
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

/* $Id: RTsig.c,v 1.1 2000/08/08 15:02:55 wylie Exp $ */

/* Paradyn/dyninst only install their own runtime library signal handlers
   on x86 platforms, primarily to support trap-based instrumentation.
   These functions are therefore only relevant for x86/Solaris & x86/Linux.
 */
/*
   sigaction() in its various forms installs a new sigaction handler from
   "act" (with its last argument dealing with any previously installed
   handler not relevant to us): on entry, we stash "act" in our copy of the
   applications handler struct and set it to our own (already-installed)
   handler so that our handler won't be uninstalled (even briefly) and
   leaving them exposed to our instrumentation traps or our other signals;
   on exit, we restore the application's provided sigaction struct "act" 
   so that it may continue to be used by the application.
 */

#include <unistd.h>
#include <signal.h>

#include "dyninstAPI_RT/h/dyninstAPI_RT.h"

struct sigaction DYNINSTactTrap;
struct sigaction DYNINSTactTrapApp;
#ifdef DETACH_ON_THE_FLY 
struct sigaction DYNINSTactIll;
struct sigaction DYNINSTactIllApp;
#endif

void
DYNINSTdeferSigHandler (int sig, struct sigaction *act, struct sigaction *oact)
{
    if (act == NULL) return;
    RTprintf("DYNINSTdeferSigHandler(sig=%d, flag=%d, handler=0x%08X)\n", sig,
            act->sa_flags&SA_SIGINFO,
            act->sa_flags&SA_SIGINFO ? (void*)act->sa_sigaction
                                     : (void*)act->sa_handler);
    switch (sig) {
      case SIGTRAP: {
        /* stash the application's new act struct */
        DYNINSTactTrapApp.sa_handler    = act->sa_handler; 
        DYNINSTactTrapApp.sa_sigaction  = act->sa_sigaction; 
        DYNINSTactTrapApp.sa_mask       = act->sa_mask; 
        DYNINSTactTrapApp.sa_flags      = act->sa_flags; 
        
        /* return sigaction our own (already-installed) handler */
        act->sa_handler   = DYNINSTactTrap.sa_handler;
        act->sa_sigaction = DYNINSTactTrap.sa_sigaction;
        act->sa_mask      = DYNINSTactTrap.sa_mask;
        act->sa_flags     = DYNINSTactTrap.sa_flags;
        return;
      }
#ifdef DETACH_ON_THE_FLY
      case SIGILL: {
        /* stash the application's new act struct */
        DYNINSTactIllApp.sa_handler    = act->sa_handler; 
        DYNINSTactIllApp.sa_sigaction  = act->sa_sigaction; 
        DYNINSTactIllApp.sa_mask       = act->sa_mask; 
        DYNINSTactIllApp.sa_flags      = act->sa_flags; 
        
        /* return sigaction our own (already-installed) handler */
        act->sa_handler   = DYNINSTactIll.sa_handler;
        act->sa_sigaction = DYNINSTactIll.sa_sigaction;
        act->sa_mask      = DYNINSTactIll.sa_mask;
        act->sa_flags     = DYNINSTactIll.sa_flags;
        return;
      }
#endif
      default:
        /* no problem with handlers for other signals */
        break;
    }
    return;
}

void
DYNINSTresetSigHandler (int sig, struct sigaction *act, struct sigaction *oact)
{
    if (act == NULL) return;
    RTprintf("DYNINSTresetSigHandler(sig=%d, flag=%d, handler=0x%08X)\n", sig,
            act->sa_flags&SA_SIGINFO,
            act->sa_flags&SA_SIGINFO ? (void*)act->sa_sigaction
                                     : (void*)act->sa_handler);
    switch (sig) {
      case SIGTRAP: {
        /* restore given handler to sigaction (to return to application) */
        act->sa_handler   = DYNINSTactTrapApp.sa_handler;
        act->sa_sigaction = DYNINSTactTrapApp.sa_sigaction;
        act->sa_mask      = DYNINSTactTrapApp.sa_mask;
        act->sa_flags     = DYNINSTactTrapApp.sa_flags;
      }
#ifdef DETACH_ON_THE_FLY
      case SIGILL: {
        /* restore given handler to sigaction (to return to application) */
        act->sa_handler   = DYNINSTactIllApp.sa_handler;
        act->sa_sigaction = DYNINSTactIllApp.sa_sigaction;
        act->sa_mask      = DYNINSTactIllApp.sa_mask;
        act->sa_flags     = DYNINSTactIllApp.sa_flags;
      }
#endif
      default:
        /* nothing to be done with handlers for other signals */
        break;
    }
    return;
}

