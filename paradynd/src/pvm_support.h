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

/* $Log: pvm_support.h,v $
/* Revision 1.2  1996/08/16 21:19:45  tamches
/* updated copyright for release 1.1
/*
 * Revision 1.1  1995/12/15 22:27:01  mjrg
 * Merged paradynd and paradyndPVM
 * Get module name for functions from symbol table in solaris
 * Fixed code generation for multiple instrumentation statements
 * Changed syntax of MDL resource lists
 *
 * Revision 1.8  1995/11/22 00:13:58  mjrg
 * Updates for paradyndPVM on solaris
 * Fixed problem with wrong daemon getting connection to paradyn
 * Removed -f and -t arguments to paradyn
 * Added cleanUpAndExit to clean up and exit from pvm before we exit paradynd
 * Fixed bug in my previous commit
 *
 * Revision 1.7  1995/05/18  11:04:55  markc
 * added mdl source
 *
 * cleaned up pvm exports
 *
 * Revision 1.6  1995/02/26  22:41:19  markc
 * Updated to compile under new system.
 *
 * Revision 1.5  1994/11/01  16:28:54  markc
 * Stronger prototypes
 *
 * Revision 1.4  1994/05/11  15:53:30  markc
 * Added sys/time.h.
 *
 * Revision 1.3  1994/05/03  05:25:32  markc
 * Added Log.
 * */

#ifndef _pvm_support_h
#define _pvm_support_h

/*
   #include <netdb.h>
   #include <memory.h>
   #include <sys/time.h>
*/

#include "util/h/String.h"

#define PDYN_HOST_DELETE 8001

extern bool PDYND_report_to_paradyn (int pid, int arg, char **argv);
extern bool PDYN_register_as_starter();
extern bool PDYN_startProcess();
extern bool PDYN_reg_as_hoster();
extern bool PDYN_hoster();
extern bool PDYN_hostDelete();
extern int PDYN_get_pvmd_tid();
extern void PDYN_reportSIGCHLD (int pid, int exit_status);
extern bool PDYN_handle_pvmd_message();
extern bool PDYN_initForPVM(char**av, const string host, int sock, int flag);
extern void PDYN_exit_pvm(void);
#endif
