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

#ifdef PARADYND_PVM
extern "C" {
#include "pvm3.h"
}
#include "pvm_support.h"
#endif

#include "util/h/headers.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h" //XXX
#include "dyninstP.h"
#include "metric.h"
#include "util.h"
#include "comm.h"
#include "stats.h"
#include "debugger.h"
#include "main.h"
#include "association.h"
#include "init.h"
#include "context.h"
#include "perfStream.h"
#include "os.h"
#include "paradynd/src/mdld.h"
#include "showerror.h"
#include "main.h"
#include "util/h/debugOstream.h"

// The following were defined in process.C
extern debug_ostream attach_cerr;
extern debug_ostream inferiorrpc_cerr;
extern debug_ostream shmsample_cerr;
extern debug_ostream forkexec_cerr;
extern debug_ostream metric_cerr;
extern debug_ostream signal_cerr;

string traceSocketPath; /* file path for trace socket */

void createResource(int pid, traceHeader *header, struct _newresource *r);

bool firstSampleReceived = false;

double cyclesPerSecond = 0;
extern time64 firstRecordTime; // util.C

// Read output data from process curr. 
void processAppIO(process *curr)
{
    int ret;
    char lineBuf[256];

    ret = read(curr->ioLink, lineBuf, sizeof(lineBuf)-1);
    if (ret < 0) {
        //statusLine("read error");
	//showErrorCallback(23, "Read error");
	//cleanUpAndExit(-2);
        string msg = string("Read error on IO stream from PID=") +
	             string(curr->getPid()) + string(": ") +
		     string(sys_errlist[errno]) + 
		     string("\nNo more data will be received from this process.");
	showErrorCallback(23, msg);
	P_close(curr->ioLink);
	curr->ioLink = -1;
	return;
    } else if (ret == 0) {
	/* end of file -- process exited */
        P_close(curr->ioLink);
	curr->ioLink = -1;
	string msg = string("Process ") + string(curr->getPid()) + string(" exited");
	statusLine(msg.string_of());
        handleProcessExit(curr,0);
	return;
    }

    // null terminate it
    lineBuf[ret] = '\0';

    // forward the data to the paradyn process.
    tp->applicationIO(curr->getPid(), ret, lineBuf);
       // note: this is an async igen call, so the results may not appear right away.
}


char errorLine[1024];

void logLine(const char *line)
{
    static char fullLine[1024];

    strcat(fullLine, line);
       // Ack!  Possible overflow!  Possible bug!
       // If you put a '\n' at the end of every string passed to a call
       // to logLine (and the string is < 1000 chars) then you'll be okay.
       // Otherwise, watch out!

    if (fullLine[strlen(fullLine)-1] == '\n') {
	tp->applicationIO(0, strlen(fullLine), fullLine);
	fullLine[0] = '\0';
    }
}

void statusLine(const char *line)
{
  tp->reportStatus(line);
}

// New with paradynd-->paradyn buffering.  When true, it tells the
// buffering routine to flush (to paradyn); otherwise, it would flush
// only when the buffer was full, hurting response time.
extern bool BURST_HAS_COMPLETED;

extern vector<process*> processVec;
extern process* findProcess(int); // should become a static method of class process

// Read trace data from process curr.
void processTraceStream(process *curr)
{
    int ret;
    traceStream sid;
    char *recordData;
    traceHeader header;
    struct _association *a;

    ret = read(curr->traceLink, &(curr->buffer[curr->bufEnd]), 
	       sizeof(curr->buffer)-curr->bufEnd);

    if (ret < 0) {
        //statusLine("read error, exiting");
        //showErrorCallback(23, "Read error");
	//curr->traceLink = -1;
	//cleanUpAndExit(-2);
        string msg = string("Read error on trace stream from PID=") +
	             string(curr->getPid()) + string(": ") +
		     string(sys_errlist[errno]) + 
		     string("\nNo more data will be received from this process");
	showErrorCallback(23, msg);
	P_close(curr->traceLink);
	curr->traceLink = -1;
	return;
    } else if (ret == 0) {
	/* end of file */
	// process exited unexpectedly
	//string buffer = string("Process ") + string(curr->pid);
	//buffer += string(" has exited unexpectedly");
	//statusLine(P_strdup(buffer.string_of()));
	//showErrorCallback(11, P_strdup(buffer.string_of()));
	string msg = string("Process ") + string(curr->getPid()) + string(" exited");
	statusLine(msg.string_of());
	P_close(curr->traceLink);
  	curr->traceLink = -1;
        handleProcessExit(curr,0);
	return;
    }

    curr->bufEnd += ret;
    curr->bufStart = 0;

    while (curr->bufStart < curr->bufEnd) {
	if (curr->bufEnd - curr->bufStart < (sizeof(traceStream) + sizeof(header))) {
	    break;
	}

	if (curr->bufStart % WORDSIZE != 0)     /* Word alignment check */
	    break;		        /* this will re-align by shifting */

	memcpy(&sid, &(curr->buffer[curr->bufStart]), sizeof(traceStream));
	curr->bufStart += sizeof(traceStream);

	memcpy(&header, &(curr->buffer[curr->bufStart]), sizeof(header));
	curr->bufStart += sizeof(header);

	curr->bufStart = ALIGN_TO_WORDSIZE(curr->bufStart);
	if (header.length % WORDSIZE != 0) {
	    sprintf(errorLine, "Warning: non-aligned length (%d) received on traceStream.  Type=%d\n", header.length, header.type);
	    logLine(errorLine);
	    showErrorCallback(36,(const char *) errorLine);
	}
	    
	if (curr->bufEnd - curr->bufStart < (unsigned)header.length) {
	    /* the whole record isn't here yet */
	    curr->bufStart -= sizeof(traceStream) + sizeof(header);
	    break;
	}

	recordData = &(curr->buffer[curr->bufStart]);
	curr->bufStart +=  header.length;

	if (!firstRecordTime)
	    firstRecordTime = header.wall;
            // firstRecordTime is used by getCurrentTime() in util.C when arg passed
            // is 'true', but noone seems to do that...so firstRecordTime is not a
            // terribly important vrble (but, for now at least, it's used in metric.C)

	// callback to paradyn (okay if this callback is done more than once; paradyn
        // detects this.  This is important since right now, we're also gonna do this
        // callback when starting a new process; needed since SHM_SAMPLING might well
        // start sending samples to paradyn before any trace records were received
        // here.)
	static bool done_yet = false;
	if (!done_yet) {
	   tp->firstSampleCallback(curr->getPid(), (double) (header.wall/1000000.0));
	   done_yet = true;
	}

	switch (header.type) {
	    case TR_NEW_RESOURCE:
//		cerr << "paradynd: received a new resource from pid " << curr->getPid() << "; processing now" << endl;
		createResource(curr->getPid(), &header, (struct _newresource *) ((void*)recordData));
		   // createResource() is in this file, below
		break;

	    case TR_NEW_ASSOCIATION:
		a = (struct _association *) ((void*)recordData);
		newAssoc(curr, a->abstraction, a->type, a->key, a->value);
		break;

#ifndef SHM_SAMPLING
	    case TR_SAMPLE:
		//metric_cerr << "got something from pid " << curr->getPid() << endl;

		// sprintf(errorLine, "Got data from process %d\n", curr->pid);
		// logLine(errorLine);
//		assert(curr->getFirstRecordTime());
		processSample(curr->getPid(), &header, (traceSample *) ((void*)recordData));
		   // in metric.C
		firstSampleReceived = true;
		break;
#endif

	    case TR_EXIT:
		sprintf(errorLine, "process %d exited\n", curr->getPid());
		logLine(errorLine);
		printAppStats((struct endStatsRec *) ((void*)recordData),
			      cyclesPerSecond);
		printDyninstStats();
  		P_close(curr->traceLink);
  		curr->traceLink = -1;
		handleProcessExit(curr, 0);
		break;

#ifndef SHM_SAMPLING
	    case TR_COST_UPDATE:
		processCost(curr, &header, (costUpdate *) ((void*)recordData));
		   // in metric.C
		break;
#endif

	    case TR_CP_SAMPLE:
		// critical path sample
		extern void processCP(process *, traceHeader *, cpSample *);
		processCP(curr, &header, (cpSample *) recordData);
		break;

	    case TR_EXEC_FAILED:
		{ int pid = *(int *)recordData;
		  process *p = findProcess(pid);
		  p->inExec = false;
		  p->execFilePath = string("");
		}
		break;

	    default:
		sprintf(errorLine, "Got unknown record type %d on sid %d\n", 
		    header.type, sid);
		logLine(errorLine);
		sprintf(errorLine, "Received bad trace data from process %d.", curr->getPid());
		showErrorCallback(37,(const char *) errorLine);
	}
    }
    BURST_HAS_COMPLETED = true; // will force a batch-flush very soon

    /* copy those bits we have to the base */
    memcpy(curr->buffer, &(curr->buffer[curr->bufStart]), 
	curr->bufEnd - curr->bufStart);
    curr->bufEnd = curr->bufEnd - curr->bufStart;
}

void doDeferredRPCs() {
   // Any RPCs waiting to be performed?  If so, and if it's safe to
   // perform one, then launch one.
   for (unsigned lcv=0; lcv < processVec.size(); lcv++) {
      process *proc = processVec[lcv];
      if (proc == NULL) continue; // proc must've died and has itself cleaned up
      if (proc->status() == exited) continue;
      if (proc->status() == neonatal) continue; // not sure if this is appropriate
      
      bool wasLaunched = proc->launchRPCifAppropriate(proc->status() == running);
      // do we need to do anything with 'wasLaunched'?
      if (wasLaunched) {
	 inferiorrpc_cerr << "launched an inferior RPC";
	 inferiorrpc_cerr << endl;
      }
   }
}

// TODO -- make this a process method
int handleSigChild(int pid, int status)
{

#ifdef rs6000_ibm_aix4_1
    // On AIX, we get sigtraps on fork and load, and must handle
    // these cases specially
    extern bool handleAIXsigTraps(int, int);
    if (handleAIXsigTraps(pid, status)) {
      return 0;
    }
    /* else check for regular traps and signals */
#endif

    // ignore signals from unknown processes
    process *curr = findProcess(pid);
    if (!curr) {
       forkexec_cerr << "handleSigChild pid " << pid << " is an unknown process." << endl;
       forkexec_cerr << "WIFSTOPPED=" << (WIFSTOPPED(status) ? "true" : "false");
       if (WIFSTOPPED(status)) {
	  forkexec_cerr << "WSTOPSIG=" << WSTOPSIG(status);
       }
       forkexec_cerr << endl << flush;
       return -1;
    }

    if (WIFSTOPPED(status)) {
	int sig = WSTOPSIG(status);
	switch (sig) {

	    case SIGTSTP:
		sprintf(errorLine, "process %d got SIGTSTP", pid);
		statusLine(errorLine);
		curr->Stopped();
		break;

	    case SIGTRAP: {
		// Note that there are now several uses for SIGTRAPs in paradynd.
		// The original use was to detect when a ptraced process had
		// started up.  Several have been added.  We must be careful
		// to make sure that uses of SIGTRAPs do not conflict.

	        signal_cerr << "welcome to SIGTRAP for pid " << curr->getPid() << endl;
		const bool wasRunning = (curr->status() == running);
		curr->status_ = stopped; // probably was 'neonatal'

		//If the list is not empty, it means some previous
		//instrumentation has yet need to be finished.
		if (instWList.size() != 0) {
	            // cerr << "instWList is full" << endl;
		    if(curr -> cleanUpInstrumentation(wasRunning)){
		        break; // successfully processed the SIGTRAP
                    }
		}

		if (curr->handleTrapIfDueToRPC()) {
		   inferiorrpc_cerr << "processed RPC response in SIGTRAP" << endl;
		   break;
		}

		if (curr->inExec) {
		   // the process has executed a succesful exec, and is now
		   // stopped at the exit of the exec call.

		   forkexec_cerr << "SIGTRAP: inExec is true, so doing process::handleExec()!" << endl;
		   string buffer = string("process ") + string(curr->getPid()) +
		                   " has performed exec() syscall";
		   statusLine(buffer.string_of());

		   // call handleExec to clean our internal data structures, reparse
		   // symbol table.  handleExec does not insert instrumentation or do
		   // any propagation.  Why not?  Because it wouldn't be safe -- the
		   // process hasn't yet been bootstrapped (run DYNINST, etc.)  Only
		   // when we get the breakpoint at the end of DYNINSTinit() is it safe
		   // to insert instrumentation and allocate timers & counters...
		   curr->handleExec();

		   // need to start resourceBatchMode here because we will call
		   //  tryToReadAndProcessBootstrapInfo which
		   // calls tp->resourceBatchMode(false)
		   tp->resourceBatchMode(true);

		   // set reachedFirstBreak to false here, so we execute
		   // the code below, and insert the initial instrumentation
		   // in the new image. (Actually, this is already done by handleExec())
		   curr->reachedFirstBreak = false;

		   // fall through...
		}

                // Now we expect that this TRAP is the initial trap sent when a ptrace'd
		// process completes startup via exec (or when an exec syscall was
		// executed in an already-running process).
                // But we must query 'reachedFirstBreak' because on machines where we
		// attach/detach on pause/continue, a TRAP is generated on each pause!
		if (!curr->reachedFirstBreak) { // vrble should be renamed 'reachedFirstTrap'
	           // cerr << "!reachedFirstBreak" << endl;
		   string buffer = string("PID=") + string(pid);
		   buffer += string(", passed trap at start of program");
		   statusLine(buffer.string_of());

//		   (void)(curr->findDynamicLinkingInfo()); // SHOULD THIS BE HERE???

		   curr->reachedFirstBreak = true;

		   buffer=string("PID=") + string(pid) + ", installing call to DYNINSTinit()";
		   statusLine(buffer.string_of());

		   curr->installBootstrapInst();
			
		   // now, let main() and then DYNINSTinit() get invoked.  As it
		   // completes, DYNINSTinit() does a DYNINSTbreakPoint, at which time
		   // we read bootstrap information "sent back" (in a global vrble),
		   // propagate metric instances, do tp->newProgramCallbackFunc(), etc.
		   if (!curr->continueProc()) {
		      cerr << "continueProc after installBootstrapInst() failed, so DYNINSTinit() isn't being invoked yet, which it should!" << endl;
		   }
		   else {
		      buffer=string("PID=") + string(pid) + ", running DYNINSTinit()...";
		      statusLine(buffer.string_of());
		   }
		}
		else {
		   forkexec_cerr << "SIGTRAP not handled for pid " << pid << " so just leaving process in stopped state" << endl << flush;
		}

		break;
	    }

	    case SIGSTOP:
	    case SIGINT: {
	        signal_cerr << "welcome to SIGSTOP/SIGINT for proc pid " << curr->getPid() << endl;

		const processState prevStatus = curr->status_;
		// assert(prevStatus != stopped); (bombs on sunos and AIX, when lots of spurious sigstops are delivered)

		curr->status_ = stopped;
		   // the following routines expect (and assert) this status.

		if (curr->procStopFromDYNINSTinit()) {
		   // grabs data from DYNINST_bootstrap_info
		   assert(curr->status_ == stopped);
		     // DYNINSTinit() after normal startup, after fork, or after exec
		     // syscall was made by a running program; leave paused
		     // (tp->newProgramCallback() to paradyn will result in the process
		     // being continued soon enough, assuming the applic was running,
		     // which is true in all cases except when an applic is just being
		     // started up).  Fall through (we want the status line to change)

		   forkexec_cerr << "processed SIGSTOP from DYNINSTinit for pid " << curr->getPid() << endl << flush;
		}
		else if (curr->handleTrapIfDueToRPC()) {
		   inferiorrpc_cerr << "processed RPC response in SIGSTOP" << endl;
		   break; // don't want to execute ->Stopped() which changes status line
		}
		else if (curr->handleStopDueToExecEntry()) {
		   // grabs data from DYNINST_bootstrap_info
		   forkexec_cerr << "fork/exec -- handled stop before exec" << endl;
		   string buffer = string("process ") + string(curr->getPid()) +
		                   " performing exec() syscall...";
		   statusLine(buffer.string_of());

		   // note: status will now be 'running', since handleStopDueToExec()
		   // did a continueProc() to let the exec() syscall go forward.
		   assert(curr->status_ == running);
		      // would neonatal be better? or exited?

		   break; // don't want to change status line in conventional way
		}
		else {
		   forkexec_cerr << "unhandled SIGSTOP for pid " << curr->getPid() << " so just leaving process in paused state." << endl << flush;
		}

		curr->status_ = prevStatus; // so Stopped() below won't be a nop
		curr->Stopped();

		curr->reachedFirstBreak = true;
		   // probably not needed; should already be true

		break;
	    }

	    case SIGILL:
	       signal_cerr << "welcome to SIGILL" << endl << flush;
	       curr->status_ = stopped;

	       if (curr->handleTrapIfDueToRPC()) {
		  inferiorrpc_cerr << "processed RPC response in SIGILL" << endl; cerr.flush();

		  break; // we don't forward the signal -- on purpose
	       }
	       else
		  // fall through, on purpose
		  ;

	    case SIGIOT:
	    case SIGBUS:
		signal_cerr << "caught signal, dying...  (sig="
                            << WSTOPSIG(status) << ")" << endl << flush;

		curr->status_ = stopped;
		curr->dumpImage();

//		OS::osDumpCore(pid, "core.real");
		//handleProcessExit(curr);
		// ???
		// should really log this to the error reporting system.
		// jkh - 6/25/96
		// now forward it to the process, who will proceed to core dump.
		curr->continueWithForwardSignal(WSTOPSIG(status));
		break;

	    case SIGALRM:
#ifndef SHM_SAMPLING
		// Due to the DYNINSTin_sample variable, it's safest to launch
		// inferior-RPCs only when we know that the inferior is not in the
		// middle of processing an alarm-expire.  Otherwise, code that does
		// stuff like call DYNINSTstartWallTimer will appear to do nothing
		// (DYNINSTstartWallTimer will be invoked but will see that
		//  DYNINSTin_sample is set and so bails out!!!)
		// Ick.
		if (curr->existsRPCreadyToLaunch()) {
		   curr -> status_ = stopped;
		   (void)curr->launchRPCifAppropriate(true);
		   break; // sure, we lose the SIGALARM, but so what.
		}
		else
		   ; // no break, on purpose
#endif

	    case SIGCHLD:
	    case SIGUSR1:
	    case SIGUSR2:
	    case SIGVTALRM:
	    case SIGCONT:
	    case SIGSEGV:	// treadmarks needs this signal
//		 printf("caught signal, forwarding...  (sig=%d)\n", 
//		       WSTOPSIG(status)); fflush(stdout);

		if (!curr->continueWithForwardSignal(WSTOPSIG(status))) {
                     logLine("error  in forwarding  signal\n");
		     showErrorCallback(38, "Error  in forwarding  signal");
                     //P_abort();
                }

		break;

#ifdef notdef
	    // XXXX for debugging
	    case SIGSEGV:	// treadmarks needs this signal
		sprintf(errorLine, "DEBUG: forwarding signal (sig=%d, pid=%d)\n"
			, WSTOPSIG(status), pid);
		logLine(errorLine);
#endif
	    default:
		if (!curr->continueWithForwardSignal(WSTOPSIG(status))) {
                     logLine("error  in forwarding  signal\n");
                     P_abort();
                }
		break;

	}
    } else if (WIFEXITED(status)) {
#ifdef PARADYND_PVM
        //  if (pvm_running) {
        //         PDYN_reportSIGCHLD (pid, WEXITSTATUS(status));
	//}
#endif
	sprintf(errorLine, "Process %d has terminated\n", curr->getPid());
	statusLine(errorLine);
	logLine(errorLine);

	printDyninstStats();
        handleProcessExit(curr, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
	sprintf(errorLine, "process %d has terminated on signal %d\n", curr->getPid(),
	    WTERMSIG(status));
	logLine(errorLine);
	statusLine(errorLine);
	handleProcessExit(curr, WTERMSIG(status));
    } else {
	sprintf(errorLine, "Unknown state %d from process %d\n", status, curr->getPid());
	logLine(errorLine);
	showErrorCallback(39,(const char *) errorLine);
    }
    return(0);
}

void ioFunc()
{
     printf("in SIG child func\n");
     fflush(stdout);
}

#ifdef SHM_SAMPLING
static void checkAndDoShmSampling(unsigned long long &pollTimeUSecs) {
   // We assume that nextShmSampleTime (synched to getCurrWallTime())
   // has already been set.  If the curr time is >= to this, then
   // we should sample immediately, and update nextShmSampleTime to
   // be nextShmSampleTime + sampleInterval, unless it is <= currTime,
   // in which case we set it to currTime + sampleInterval.

   // QUESTION: should we sample a given process while it's paused?  While it's
   //           in the middle of an inferiorRPC?  For now, the answer is no
   //           to both.

   static unsigned long long nextMajorSampleTime = 0;
   static unsigned long long nextMinorSampleTime = 0;

   const unsigned long long currWallTime = getCurrWallTimeULL();
      // checks for rollback

   bool doMajorSample = false; // so far...
   bool doMinorSample = false; // so far...

   bool forNextTimeDoMinorSample = false; // so far...

   if (currWallTime >= nextMajorSampleTime)
      doMajorSample = true;
   else if (currWallTime >= nextMinorSampleTime)
      doMinorSample = true;
   else
      // it's not time to do anything.
      return;

   // Do shared memory sampling (for all processes) now!

   // Loop thru all processes.  For each, process inferiorIntCounters,
   // inferiorWallTimers, and inferiorProcessTimers.  But don't
   // sample while an inferiorRPC is pending for that process, or for
   // a non-running process.

   for (unsigned lcv=0; lcv < processVec.size(); lcv++) {
      process *theProc = processVec[lcv];
      if (theProc == NULL)
	 continue; // proc died & had its structures cleaned up

      // Don't sample paused/exited/neonatal processes, or even running processes
      // that haven't been bootstrapped yet (i.e. haven't called DYNINSTinit yet),
      // or processes that are in the middle of an inferiorRPC (we like for
      // inferiorRPCs to finish up quickly).
      if (theProc->status_ != running) {
	 //shmsample_cerr << "(-" << theProc->getStatusAsString() << "-)";
	 continue;
      }
      else if (!theProc->isBootstrappedYet()) {
	 //shmsample_cerr << "(-*-)" << endl;
	 continue;
      }
      else if (theProc->existsRPCinProgress()) {
	 //shmsample_cerr << "(-~-)" << endl;
	 continue;
      }

      if (doMajorSample) {
	 //shmsample_cerr << "(-Y-)" << endl;

	 if (!theProc->doMajorShmSample(currWallTime)) {
	    // The major sample didn't complete all of its work, so we
	    // schedule a minor sample for sometime in the near future
	    // (before the next major sample)

	    shmsample_cerr << "a minor sample will be needed" << endl;

	    forNextTimeDoMinorSample = true;
	 }
      }
      else if (doMinorSample) {
	 shmsample_cerr << "trying needed minor sample..."; cerr.flush();

	 if (!theProc->doMinorShmSample()) {
	    // The minor sample didn't complete all of its work, so
	    // schedule another one.
	    forNextTimeDoMinorSample = true;

	    shmsample_cerr << "it failed" << endl; cerr.flush();
	 }
	 else {
	    shmsample_cerr << "it succeeded" << endl; cerr.flush();
	 }
      }
   } // loop thru the processes

   // And now, do the internal metrics
   if (doMajorSample)
      reportInternalMetrics(true);

   // Here, we should probably flush the batch buffer (whether for a major
   // sample or a minor one)
   extern void flush_batch_buffer(); // metric.C (should be in this file)
   flush_batch_buffer();

   // Take currSamplingRate (which has values such as 0.2, 0.4, 0.8, 1.6, etc.)
   // and multiply by a million to get the # of usecs per sample.
   extern float currSamplingRate; // dynrpc.C
   assert(currSamplingRate > 0);
   const unsigned long long shmSamplingInterval =
	      (unsigned long long)((double)currSamplingRate * 1000000.0);

   if (doMajorSample) {
      // If we just did a major sample, then we schedule the next major sample,
      // and reset the next minor sample time.
      nextMajorSampleTime += shmSamplingInterval;
      if (nextMajorSampleTime <= currWallTime)
	 nextMajorSampleTime = currWallTime + shmSamplingInterval;
   }

   if (forNextTimeDoMinorSample) {
      // If a minor sample is needed, then we schedule it.  For now, let's
      // assume that a minor sample is always scheduled for now plus
      // one-fourth of the original sampling rate...i.e. for now + (0.2 sec/4) =
      // now + (0.05 sec), i.e. now + 50 milliseconds.
// temp: one-tenth of original sample rate...i.e. for now + 0.02 sec (+20 millisec)

//      nextMinorSampleTime = currWallTime + 50000; // 50ms = 50000us
      nextMinorSampleTime = currWallTime + 20000; // 20ms = 20000us
      if (nextMinorSampleTime > nextMajorSampleTime)
	 // oh, never mind, we'll just do the major sample which is going to
	 // happen first anyway.
	 nextMinorSampleTime = nextMajorSampleTime;
   }
   else {
      // we don't need to do a minor sample next time, so reset nextMinorSampleTime
      nextMinorSampleTime = nextMajorSampleTime;
   }

   unsigned long long nextAnyKindOfSampleTime = nextMajorSampleTime;
   if (nextMinorSampleTime < nextAnyKindOfSampleTime)
      nextAnyKindOfSampleTime = nextMinorSampleTime;

   assert(nextAnyKindOfSampleTime >= currWallTime);
   const unsigned long long shmSamplingTimeout = nextAnyKindOfSampleTime - currWallTime;

   if (shmSamplingTimeout < pollTimeUSecs)
      pollTimeUSecs = shmSamplingTimeout;
}
#endif

void doSignals() {
   /* check for status change on inferior processes, or the arrival of
      a signal. */

   int wait_status;
   int wait_pid = process::waitProcs(&wait_status);
   if (wait_pid > 0) {
      if (handleSigChild(wait_pid, wait_status) < 0) {
	 cerr << "handleSigChild failed for pid " << wait_pid << endl;
      }
   }
}

/*
 * Wait for a data from one of the inferiors or a request to come in.
 *
 */

void controllerMainLoop(bool check_buffer_first)
{
    int ct;
    int width;
    fd_set readSet;
    fd_set errorSet;
    struct timeval pollTime;
    int traceSocket_fd;

    // TODO - i am the guilty party - this will go soon - mdc
#ifdef PARADYND_PVM
#ifdef notdef
    int fd_num, *fd_ptr;
    if (pvm_mytid() < 0)
      {
	printf("pvm not working\n");
	_exit(-1);
      }
    fd_num = pvm_getfds(&fd_ptr);
    assert(fd_num == 1);
#endif
#endif

//    cerr << "welcome to controllerMainLoop...pid=" << getpid() << endl;
//    kill(getpid(), SIGSTOP);
//    cerr << "doing controllerMainLoop..." << endl;


    /***
       set up a socket to be used to create a trace link
       by inferior processes that are not forked 
       directly by this daemon.
       This is a unix domain socket, which is bound to the file
          <P_tmpdir>/paradynd.<pid>
       where <P_tmpdir> is a constant defined in stdio.h (usually "/tmp" or
       "/usr/tmp"), and <pid> is the pid of the paradynd process.

       This socket is currently being used in two cases: when a
       process forks and when we attach to a running process.  In the
       fork case, the socket path can be passed in the environment (so
       any name for the file would be ok), but in the attach case the
       name is passed as an argument to DYNINSTinit. Since we
       currently can only pass integer values as arguments, we use the
       file name paradynd.<pid>, so that we need only to pass the pid
       as the argument to DYNINSTinit, which can then determine the
       full file name.

       traceSocket_fd is the file descriptor of a socket, ready to receive
       connections.
       It represents a socket created with socket(); listen()
       In other words, one which we intend to call accept() on.
       (See perfStream.C -- the call to RPC_getConnect(traceSocket_fd))
    ***/

    traceSocketPath = string(P_tmpdir) + string("paradynd.") + string(getpid());
    // unlink it, in case the file was left around from a previous run
    unlink(traceSocketPath.string_of());

    if (!RPC_setup_socket_un(traceSocket_fd, traceSocketPath.string_of())) {
      perror("paradynd -- can't setup socket");
      cleanUpAndExit(-1);
    }


    while (1) {
//        doSignals();

	FD_ZERO(&readSet);
	FD_ZERO(&errorSet);
	width = 0;
	unsigned p_size = processVec.size();
	for (unsigned u=0; u<p_size; u++) {
	    if (processVec[u] == NULL)
	       continue;

	    if (processVec[u]->traceLink >= 0)
	      FD_SET(processVec[u]->traceLink, &readSet);
	    if (processVec[u]->traceLink > width)
	      width = processVec[u]->traceLink;

	    if (processVec[u]->ioLink >= 0)
	      FD_SET(processVec[u]->ioLink, &readSet);
	    if (processVec[u]->ioLink > width)
	      width = processVec[u]->ioLink;
	}

	// add traceSocket_fd, which accept()'s new connections (from processes
	// not launched via createProcess() [process.C], such as when a process
	// forks, or when we attach to an already-running process).
	if (traceSocket_fd > 0) FD_SET(traceSocket_fd, &readSet);
	if (traceSocket_fd > width) width = traceSocket_fd;

	// add our igen connection with the paradyn process.
	FD_SET(tp->get_fd(), &readSet);
	FD_SET(tp->get_fd(), &errorSet);
	if (tp->get_fd() > width) width = tp->get_fd();

#ifdef PARADYND_PVM
	// add connection to pvm daemon.
	/***
	  There is a problem here since pvm_getfds is not implemented on 
	  libpvmshmem which we use on solaris (a call to pvm_getfds returns
	  PvmNotImpl).
	  If we cannot use pvm_getfds here, the only alternative is to use polling.
	  To keep the code simple, I am using polling in all cases.
	***/
#ifdef notdef // not in use because pvm_getfds is not implemented on all platforms
	fd_num = pvm_getfds(&fd_ptr);
	assert(fd_num == 1);
	FD_SET(fd_ptr[0], &readSet);
	if (fd_ptr[0] > width)
	  width = fd_ptr[0];
#endif
#endif

#ifdef SHM_SAMPLING
// When _not_ shm sampling, rtinst defines a global vrble called
// DYNINSTin_sample, which is set to true while the application samples
// itself due to an alarm-expire.  When this variable is set, a call to
// DYNINSTstartProcessTimer() et al. will return immediately, taking
// no action.  This is of course a bad thing to happen.
// So: when not shm sampling, we mustn't do an inferiorRPC here.
// So we only do inferiorRPC here when SHM_SAMPLING.
// (What do we do when non-shm-sampling?  We wait until we're sure
// that we're not in the middle of processing a timer.  One way to do
// that is to manually reset DYNINSTin_sample when doing an RPC, and
// then restoring its initial value when done.  Instead, we wait for an
// ALARM signal to be delivered, and do pending RPCs just before we forward
// the signal.  Assuming ALARM signals aren't recursive, this should do the
// trick.  Ick...yet another reason to kill the ALARM signal and go with shm
// sampling.

	doDeferredRPCs();
#endif

	unsigned long long pollTimeUSecs = 50000;
           // this is the time (rather arbitrarily) chosen fixed time length
           // in which to check for signals, etc.

#ifdef SHM_SAMPLING
        checkAndDoShmSampling(pollTimeUSecs);
           // does shm sampling of each process, as appropriate.
           // may update pollTimeUSecs.
#endif 

	pollTime.tv_sec  = pollTimeUSecs / 1000000;
	pollTime.tv_usec = pollTimeUSecs % 1000000;

	// This fd may have been read from prior to entering this loop
	// There may be some bytes lying around
	if (check_buffer_first) {
	  bool no_stuff_there = P_xdrrec_eof(tp->net_obj());
	  while (!no_stuff_there) {
	    T_dyninstRPC::message_tags ret = tp->waitLoop();
	    if (ret == T_dyninstRPC::error) {
	      // assume the client has exited, and leave.
	      cleanUpAndExit(-1);
	    }
	    no_stuff_there = P_xdrrec_eof(tp->net_obj());
	  }
	}

	// TODO - move this into an os dependent area
	ct = P_select(width+1, &readSet, NULL, &errorSet, &pollTime);

	if (ct > 0) {

	    if (traceSocket_fd >= 0 && FD_ISSET(traceSocket_fd, &readSet)) {
	      // a process forked by a process we are tracing is trying
	      // to get a connection for its trace stream.
	      // NOTE: This will probably be the gateway whereby we implement
	      //       attaching to a new process.  (NOT YET IMPLEMENTED)

	      processNewTSConnection(traceSocket_fd); // context.C
	    }

            unsigned p_size = processVec.size();
	    for (unsigned u=0; u<p_size; u++) {
	        if (processVec[u] == NULL)
		   continue; // process structure has been deallocated

		if (processVec[u] && processVec[u]->traceLink >= 0 && 
		       FD_ISSET(processVec[u]->traceLink, &readSet)) {
		    processTraceStream(processVec[u]);

		    /* in the meantime, the process may have died, setting
		       processVec[u] to NULL */

		    /* clear it in case another process is sharing it */
		    if (processVec[u] &&
			processVec[u]->traceLink >= 0)
		           // may have been set to -1
		       FD_CLR(processVec[u]->traceLink, &readSet);
		}

		if (processVec[u] && processVec[u]->ioLink >= 0 && 
    		       FD_ISSET(processVec[u]->ioLink, &readSet)) {
		    processAppIO(processVec[u]);

		    // app can (conceivably) die in processAppIO(), resulting
		    // in a processVec[u] to NULL.

		    /* clear it in case another process is sharing it */
		    if (processVec[u] && processVec[u]->ioLink >= 0)
		       // may have been set to -1
		       FD_CLR(processVec[u]->ioLink, &readSet);
		}
	    }

	    if (FD_ISSET(tp->get_fd(), &errorSet)) {
		// paradyn is gone so we go too.
	        cleanUpAndExit(-1);
	    }

            // Check if something has arrived from Paradyn on our igen link.
	    if (FD_ISSET(tp->get_fd(), &readSet)) {
	      bool no_stuff_there = false;
	      while(!no_stuff_there) {
		T_dyninstRPC::message_tags ret = tp->waitLoop();
		if (ret == T_dyninstRPC::error) {
		  // assume the client has exited, and leave.
		  cleanUpAndExit(-1);
		}
		no_stuff_there = P_xdrrec_eof(tp->net_obj());
	      }
	    }
	    while (tp->buffered_requests()) {
	      T_dyninstRPC::message_tags ret = tp->process_buffered();
	      if (ret == T_dyninstRPC::error)
		cleanUpAndExit(-1);
	    }

#ifdef PARADYND_PVM
#ifdef notdef // not in use because of the problems with pvm_getfds. See comment above.
	    // message on pvmd channel
	    int res;
            fd_num = pvm_getfds(&fd_ptr);
	    assert(fd_num == 1);
	    if (FD_ISSET(fd_ptr[0], &readSet)) {
		// res == -1 --> error
		res = PDYN_handle_pvmd_message();
		// handle pvm message
	    }
#endif
#endif
	}

#ifdef PARADYND_PVM
	// poll for messages from the pvm daemon, and handle the message if 
	// there is one.
	// See comments above on the problems with pvm_getfds.
	if (pvm_running) {
	  PDYN_handle_pvmd_message();
	}
#endif

#ifndef SHM_SAMPLING
	// the ifdef is here because when shm sampling, reportInternalMetrics is
	// already done.
	reportInternalMetrics(false);
#endif

	doSignals();
    }
}


void createResource(int pid, traceHeader *header, struct _newresource *r)
{
    char *tmp;
    char *name;
    // resource *res;
    vector<string> parent_name;
    resource *parent = NULL;
    unsigned type;
    
    switch (r->type) {
    case RES_TYPE_STRING: type = MDL_T_STRING; break;
    case RES_TYPE_INT:    type = MDL_T_INT; break;
    default: 
      string msg = string("Invalid resource type reported on trace stream from PID=")
	           + string(pid);
      showErrorCallback(36,msg);
      return;
    }

    name = r->name;
    do {
	tmp = strchr(name, '/');
	if (tmp) {
	    *tmp = '\0';
	    tmp++;
	    parent_name += name;
	    name = tmp;
	}
    } while (tmp);

    if ((parent = resource::findResource(parent_name)) && name != r->name) {
      resource::newResource(parent, NULL, r->abstraction, name,
			    header->wall, "", type);
    }
    else {
      string msg = string("Unknown resource '") + string(r->name) +
	           string("' reported on trace stream from PID=") +
		   string(pid);
      showErrorCallback(36,msg);
    }

}
