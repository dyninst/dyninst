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

/*
 * $Log: paradyn.h,v $
 * Revision 1.16  1999/04/15 21:24:00  nash
 * Removed annoying '/* within comment' warnings from a couple files
 *
 * Revision 1.15  1996/08/16 21:13:04  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.14  1996/01/29 22:13:01  mjrg
 * Added metric propagation when new processes start
 * Adjust time to account for clock differences between machines
 * Daemons don't enable internal metrics when they are not running any processes
 * Changed CM5 start (paradynd doesn't stop application at first breakpoint;
 * the application stops only after it starts the CM5 daemon)
 * Added -default_host option to paradyn
 *
 * Revision 1.13  1995/08/24  15:02:57  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.12  1995/08/12  22:27:37  newhall
 * moved def. of init_struct to dataManager.I
 *
 * Revision 1.11  1995/06/02  20:56:00  newhall
 * made code compatable with new DM interface
 *
 * Revision 1.10  1995/05/18  11:00:46  markc
 * added mdl hooks
 *
 */

/* paradyn.h */

/* some global definitions for main.C */

#ifndef _paradyn_h
#define _paradyn_h

#include "thread/h/thread.h"

#include "dataManager.thread.CLNT.h"
#include "performanceConsultant.thread.CLNT.h"
#include "UI.thread.CLNT.h"
#include "VM.thread.CLNT.h"

struct CLargStruct {
  int clargc;
  char **clargv;
};

typedef struct CLargStruct CLargStruct;


/* common MSG TAG definitions */

/* changed these to be MSG_TAG_USERrelative (for AIX) jkh 8/14/95 */
#define MSG_TAG_UIM_READY MSG_TAG_USER+1
#define MSG_TAG_DM_READY MSG_TAG_USER+2
#define MSG_TAG_VM_READY MSG_TAG_USER+3
#define MSG_TAG_PC_READY MSG_TAG_USER+4
#define MSG_TAG_ALL_CHILDREN_READY MSG_TAG_USER+5
#define MSG_TAG_TC_READY MSG_TAG_USER+6

extern thread_t UIMtid;
extern thread_t MAINtid;
extern thread_t PCtid;
extern thread_t DMtid;
extern thread_t VMtid;
extern thread_t TCtid; // tunable constant
// extern applicationContext *context;

extern dataManagerUser *dataMgr;
extern performanceConsultantUser *perfConsult;
extern UIMUser *uiMgr;
extern VMUser  *vmMgr;


// PARADYN_DEBUG: if PARADYNDEBUG environment variable 
//     has value > 0, prints msg to stdout
extern void print_debug_macro(const char* format, ...);
#define PARADYN_DEBUG(x) print_debug_macro x

// Structure used to pass initial arguments to data manager
//typedef struct {
//  int tid;
//  char *met_file;
//} init_struct;

// default_host defines the host where programs run when no host is
// specified in a PCL process definition, or in the process definition window.
extern string default_host;


#endif


