#include "os.h"
#include "metric.h"
#include "dyninst.h"
#include "rtinst/h/trace.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "ast.h"
#include "ptrace_emul.h"
#include "dyninstRPC.xdr.SRVR.h"
#include "util.h"
#include "stats.h"
#include "main.h"
#include "perfStream.h"
#include "context.h"
#include "showerror.h"

string process::getProcessStatus() const {
   char ret[80];

   switch (status()) {
	case running:
	    sprintf(ret, "%d running", pid);
	    break;
	case neonatal:
	    sprintf(ret, "%d neonatal", pid);
	    break;
	case stopped:
	    sprintf(ret, "%d stopped", pid);
	    break;
	case exited:
	    sprintf(ret, "%d exited", pid);
	    break;
	default:
	    sprintf(ret, "%d UNKNOWN State", pid);
	    break;
    }
    return(ret);
}

//
// someone should make the proper measurements on hpux
//
void initPrimitiveCost()
{
    /* Need to add code here to collect values for other machines */

    // TODO: these values need to be updated - naim (02-13-96)

    // these happen async of the rest of the system.
    primitiveCosts["DYNINSTalarmExpire"] = 1;
    primitiveCosts["DYNINSTsampleValues"] = 1;
    primitiveCosts["DYNINSTreportTimer"] = 1;
    primitiveCosts["DYNINSTreportCounter"] = 1;
    primitiveCosts["DYNINSTreportCost"] = 1;
    primitiveCosts["DYNINSTreportNewTags"] = 1;
    primitiveCosts["DYNINSTprintCost"] = 1;

    // this doesn't really take any time
    primitiveCosts["DYNINSTbreakPoint"] = 1;

    // this happens before we start keeping time.
    primitiveCosts["DYNINSTinit"] = 1;

    primitiveCosts["DYNINSTincrementCounter"] = 16;
    primitiveCosts["DYNINSTdecrementCounter"] = 16;

    primitiveCosts["DYNINSTstartWallTimer"] = 1988;
    primitiveCosts["DYNINSTstopWallTimer"] = 4084;
    primitiveCosts["DYNINSTstartProcessTimer"] = 51;
    primitiveCosts["DYNINSTstopProcessTimer"] = 111;
}

int flushPtrace()
{
    return(0);
}

void forkNodeProcesses(process *curr, traceHeader *hr, traceFork *fr)
{
    printf("why is forkNodeProcesses being called on HPUX\n");

    int childPid;
    char command[256];
    char application[256];
    char app_pid[20];
    char num_nodes[20];	
 
    process *parent = findProcess(fr->ppid);
    if (!parent) {
      sprintf(errorLine, "In forkNodeProcesses, parent id %d unknown", fr->ppid);
      statusLine(errorLine);
      showErrorCallback(51, (const char *) errorLine);
      return;
    }

    /* Build arglist */
    sprintf (command, "%sCM5", process::programName.string_of());
    sprintf (application, "%s", (curr->symbols->file()).string_of());
    sprintf (app_pid, "%d", curr->pid);
    sprintf (num_nodes, "%d", fr->npids);

    /*
     * It would be nice if this weren't sensitive to the size of
     * arg_list.  For the moment, only arg_list[0] --> arg_list[6]
     * are written by RPC_make_arg_list
     * This is a small-time hack.
     */

    char *argv[20];
    argv[0] = command;
    argv[1] = application;
    argv[2] = app_pid;
    argv[3] = num_nodes;

    // IF these are change, check out the delete below
    argv[4] = P_strdup(process::arg_list[0].string_of());
    argv[5] = P_strdup(process::arg_list[1].string_of());
    argv[6] = P_strdup(process::arg_list[2].string_of());
    argv[7] = P_strdup(process::arg_list[3].string_of());
    argv[8] = P_strdup(process::arg_list[4].string_of());
    argv[9] = NULL;

    if ((childPid=fork()) == 0) {		/* child */
      P_execvp (command, argv);
      logLine("Exec failed in paradynd to start paradyndCM5\n");
      showErrorCallback(56, "");
      P_abort();
    } else {			/* parent */
      sprintf (errorLine, "forked child process (pid=%d)", childPid);
      statusLine(errorLine);
    }

    for (int di=4; di<9; di++)
      delete argv[di];

    // There is no need to stop the process here, since the process stops itself
    // after calling forkNodeProcess.

    /* Mark the cm-process as running now */
    // curr->status = running;
    //    pauseAllProcesses();
}


/*
 * Define the various classes of library functions to inst. 
 *
 */
void initLibraryFunctions()
{
    /* should record waiting time in read/write, but have a conflict with
     *   use of these functions by our inst code.
     *   This happens when a CPUtimer that is stopped is stopped again by the
     *   write.  It is then started again at the end of the write and should
     *   not be running then.  We could let timers go negative, but this
     *   causes a problem when inst is inserted into already running code.
     *   Not sure what the best fix is - jkh 10/4/93
     *
     */
#ifdef notdef
    tagDict["write"] = TAG_LIB_FUNC | TAG_IO_OUT;
    tagDict["read"] = TAG_LIB_FUNC | TAG_IO_IN;

    tagDict["send"] = TAG_LIB_FUNC | TAG_CPU_STATE | TAG_MSG_SEND;
    tagDict["sendmsg"] = TAG_LIB_FUNC | TAG_CPU_STATE | TAG_MSG_SEND;
    tagDict["sendto"] = TAG_LIB_FUNC | TAG_CPU_STATE | TAG_MSG_SEND;

    tagDict["rev"] = TAG_LIB_FUNC | TAG_CPU_STATE | TAG_MSG_RECV;
    tagDict["recvmsg"] = TAG_LIB_FUNC | TAG_CPU_STATE | TAG_MSG_RECV;
    tagDict["recvfrom"] = TAG_LIB_FUNC | TAG_CPU_STATE | TAG_MSG_RECV;

    tagDict["DYNINSTalarmExpire"] = TAG_LIB_FUNC;
    tagDict["DYNINSTsampleValues"] = TAG_LIB_FUNC;
    tagDict[EXIT_NAME] = TAG_LIB_FUNC;
    tagDict["fork"] = TAG_LIB_FUNC;

    tagDict["cmmd_debug"] = TAG_LIB_FUNC;
    tagDict["CMRT_init"] = TAG_LIB_FUNC;
    tagDict["CMMD_send"] = TAG_LIB_FUNC;
    tagDict["CMMD_receive"] = TAG_LIB_FUNC;
    tagDict["CMMD_receive_block"] = TAG_LIB_FUNC;
    tagDict["CMMD_send_block"] = TAG_LIB_FUNC;
    tagDict["CMMD_send_async"] = TAG_LIB_FUNC;
    tagDict["CMMD_send_async"] = TAG_LIB_FUNC;

    tagDict["main"] = 0;
#endif
}
 
float computePauseTimeMetric()
{
    timeStamp now;
    timeStamp elapsed=0.0;

    now = getCurrentTime(false);
    if (firstRecordTime) {
	elapsed = elapsedPauseTime;
	if (isApplicationPaused())
	    elapsed += now - startPause;

	assert(elapsed >= 0.0); 
	return(elapsed);
    } else {
	return(0.0);
    }
}

void osDependentInst(process *proc) {
}
