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

#include "symtab.h"
#include "process.h"
#include "rtinst/h/rtinst.h"
#include "inst.h"
#include "instP.h"
#include "dyninst.h"
#include "dyninstP.h"
#include "util.h"
#include "os.h"
#include "showerror.h"

#include <sys/param.h>

process *defaultProcess=NULL;

extern process *findProcess(int); // should become a static method of class process

void changeDefaultProcess(int pid)
{
    process *def = findProcess(pid);
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
//    if (processVec.size()) {
//      basePid = processVec[0]->getPid();
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
