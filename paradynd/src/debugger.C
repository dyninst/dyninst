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
 * $Log: debugger.C,v $
 * Revision 1.20  2002/10/15 17:11:38  schendel
 * create Paradyn specific pd_process and pd_thread classes  - - - - - - - -
 * changed to use paradyn specific pd_process class instead of (now dyninst)
 *   process class;
 * created processMgr to manage all pd_processes in place of processVec;
 *
 * Revision 1.19  1999/08/30 16:02:29  zhichen
 * Fixed bugs introduced to thread-aware daemon
 *
 * Revision 1.18  1998/04/22 02:37:26  buck
 * Moved showerror.h from paradynd directory to dyninstAPI directory.
 *
 * Revision 1.17  1997/02/26 23:46:28  mjrg
 * First part of WindowsNT port: changes for compiling with Visual C++;
 * moved unix specific code to unix.C file
 *
 * Revision 1.16  1997/02/21 20:15:41  naim
 * Moving files from paradynd to dyninstAPI + eliminating references to
 * dataReqNode from the ast class. This is the first pre-dyninstAPI commit! - naim
 *
 * Revision 1.15  1997/01/15 00:20:40  tamches
 * commented out some unused code
 *
 * Revision 1.14  1996/11/05 20:30:53  tamches
 * removed dumpProcessImage
 *
 * Revision 1.13  1996/10/31 08:38:34  tamches
 * changed call interface to osDumpImage
 *
 * Revision 1.12  1996/08/16 21:18:26  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.11  1995/09/26 20:17:43  naim
 * Adding error messages using showErrorCallback function for paradynd
 *
 * Revision 1.10  1995/05/18  10:31:21  markc
 * replace process dict with process map
 *
 * Revision 1.9  1995/02/16  08:53:05  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.8  1995/02/16  08:33:06  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.7  1994/11/02  11:03:24  markc
 * Moved os specific code to os-specific files.
 *
 */

//
// support for debugger style commands and interface.
//

#include "dyninstAPI/src/symtab.h"
#include "paradynd/src/pd_process.h"
#include "paradynd/src/processMgr.h"
#include "rtinst/h/rtinst.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/dyninst.h"
#include "dyninstAPI/src/dyninstP.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/showerror.h"


pd_process *defaultProcess=NULL;

void changeDefaultProcess(int pid)
{
   pd_process *def = getProcMgr().find_pd_process(pid);
   if (!def) {
      sprintf(errorLine, "Unable to find process %d\n", pid);
      logLine(errorLine);
      showErrorCallback(58, (const char *) errorLine);
   } else {
      defaultProcess = def;
   }
}

//// TODO what does this do
//void changeDefaultThread(int tid)
//{
//    int basePid;
//
//    if(getProcMgr().size()) {
//      basePid = (*getProcMgr().begin())->getPid();
//    } else {
//	sprintf(errorLine, "Internal error: no process defines to take thread of\n");
//	logLine(errorLine);
//	showErrorCallback(59,(const char *) errorLine);
//	return;
//    }
//
//    process *proc = findProcess(tid * MAXPID + basePid);
//    if (!proc) {
//      sprintf(errorLine, "Internal error: unable to find thread %d\n", tid);
//      logLine(errorLine);
//      showErrorCallback(60,(const char *) errorLine);
//    } else {
//      defaultProcess = proc;
//    }
//}
//
//process *getDefaultProcess()
//{
//    if (!defaultProcess) changeDefaultThread(0);
//    return(defaultProcess);
//}

extern void dummy (void) {
  cerr << "dummy"<< endl ;
}

