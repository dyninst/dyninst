
/*
 * $Log: debugger.C,v $
 * Revision 1.10  1995/05/18 10:31:21  markc
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
 * Revision 1.6  1994/10/13  07:24:34  krisna
 * solaris porting and updates
 *
 * Revision 1.5  1994/09/22  01:50:14  markc
 * cast stringHandle to char*
 * cast args for ptrace
 *
 * Revision 1.4  1994/06/27  18:56:40  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.3  1994/06/22  03:46:30  markc
 * Removed compiler warnings.
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

#include <sys/param.h>

process *defaultProcess=NULL;

void changeDefaultProcess(int pid)
{
    process *def = findProcess(pid);
    if (!def) {
	sprintf(errorLine, "unable to find process %d\n", pid);
	logLine(errorLine);
    } else {
        defaultProcess = def;
    }
}

// TODO what does this do
void changeDefaultThread(int tid)
{
    int basePid;

    if (processVec.size()) {
      basePid = processVec[0]->getPid();
    } else {
	sprintf(errorLine, "no process.defines to take thread of\n");
	logLine(errorLine);
	return;
    }

    process *proc = findProcess(tid * MAXPID + basePid);
    if (!proc) {
      sprintf(errorLine, "unable to find thread %d\n", tid);
      logLine(errorLine);
    } else {
      defaultProcess = proc;
    }
}

process *getDefaultProcess()
{
    if (!defaultProcess) changeDefaultThread(0);
    return(defaultProcess);
}

void dumpProcessImage(process *proc, bool stopped) {
  OS::osDumpImage(proc->symbols->file(), proc->pid, proc->symbols->codeOffset());
}
