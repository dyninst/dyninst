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

/* $Id: pvm_support.h,v 1.5 2000/07/28 17:22:33 pcroth Exp $ */

#ifndef _pvm_support_h
#define _pvm_support_h

/*
   #include <netdb.h>
   #include <memory.h>
   #include <sys/time.h>
*/

#include "common/h/String.h"

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
