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
 * perfStream.C - Manage performance streams.
 *
 * $Log: perfStream.C,v $
 * Revision 1.62  1996/08/20 19:02:21  lzheng
 * Implementation of moving multiple instructions sequence
 *
 * Revision 1.61  1996/08/16 21:19:31  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.60  1996/08/12 16:27:03  mjrg
 * Code cleanup: removed cm5 kludges and some unused code
 *
 * Revision 1.59  1996/05/13 15:44:48  mjrg
 * Check the pid of a TR_EXEC record
 *
 * Revision 1.58  1996/05/11 00:30:47  mjrg
 * Added new calls to handleProcessExit
 *
 * Revision 1.57  1996/05/08 23:54:59  mjrg
 * added support for handling fork and exec by an application
 * use /proc instead of ptrace on solaris
 * removed warnings
 *
 * Revision 1.56  1996/03/12 20:48:32  mjrg
 * Improved handling of process termination
 * New version of aggregateSample to support adding and removing components
 * dynamically
 * Added error messages
 *
 * Revision 1.55  1996/03/05 16:14:05  naim
 * Making enableDataCollection asynchronous in order to improve performance - naim
 *
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

// TODO: this eliminates a warning but creates a conflict when compiling
// paradyndCM5.
//extern "C" void bzero(char *b, int length);

void createResource(int pid, traceHeader *header, struct _newresource *r);

bool synchronousMode = false;
bool firstSampleReceived = false;

double cyclesPerSecond = 0;
time64 firstRecordTime = 0;

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
	/* end of file */
        P_close(curr->ioLink);
	curr->ioLink = -1;
	string msg = string("Process ") + string(curr->getPid()) + string(" exited");
	statusLine(msg.string_of());
        handleProcessExit(curr,0);
	return;
    }

    // null terminate it
    lineBuf[ret] = '\0';

    // forawrd the data to the paradyn process.
    tp->applicationIO(curr->pid, ret, lineBuf);

}


char errorLine[1024];

void logLine(const char *line)
{
    static char fullLine[1024];

    strcat(fullLine, line);
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

	if (curr->bufStart % WORDSIZE != 0)	/* Word alignment check */
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

	/*
	 * convert header to time based on first record.
	 *
	 */
	if (!curr->getFirstRecordTime()) {
	    double st;

	    curr->setFirstRecordTime(header.wall);
	    st = curr->getFirstRecordTime() / 1000000.0;
	    /* sprintf(errorLine, "started at %f\n", st);*/
	    /* logLine(errorLine);*/
	    // report sample here
	    
	    tp->firstSampleCallback(curr->getPid(), (double) (header.wall/1000000.0));

	}
	// header.wall -= curr->firstRecordTime();
	switch (header.type) {
	    case TR_FORK:
		forkProcess((traceFork *) ((void*)recordData));
		break;

	    case TR_NEW_RESOURCE:
		createResource(curr->getPid(), &header, (struct _newresource *) ((void*)recordData));
		break;

	    case TR_NEW_ASSOCIATION:
		a = (struct _association *) ((void*)recordData);
		newAssoc(curr, a->abstraction, a->type, a->key, a->value);
		break;

	    case TR_SAMPLE:
		// sprintf(errorLine, "Got data from process %d\n", curr->pid);
		// logLine(errorLine);
		assert(curr->getFirstRecordTime());
		processSample(&header, (traceSample *) ((void*)recordData));
		firstSampleReceived = true;
		break;

	    case TR_EXIT:
		sprintf(errorLine, "process %d exited\n", curr->pid);
		logLine(errorLine);
		printAppStats((struct endStatsRec *) ((void*)recordData),
			      cyclesPerSecond);
		printDyninstStats();
  		P_close(curr->traceLink);
  		curr->traceLink = -1;
		handleProcessExit(curr, 0);
		break;

	    case TR_COST_UPDATE:
		processCost(curr, &header, (costUpdate *) ((void*)recordData));
		break;

	    case TR_CP_SAMPLE:
		extern void processCP(process *, traceHeader *, cpSample *);
		processCP(curr, &header, (cpSample *) recordData);
		break;

	    case TR_EXEC:
		{ traceExec *execData = (traceExec *) recordData;
		  if (execData->pid == curr->getPid()) {
		    curr->inExec = true;
		    curr->execFilePath = string(execData->path);
		  }
		  else {
		    sprintf(errorLine, "Received inconsistent trace data from process %d", curr->getPid());
		    showErrorCallback(37, (const char *) errorLine);
		  }
		}
		break;

	    case TR_EXEC_FAILED:
		{ int pid = *(int *)recordData;
		  process *p = findProcess(pid);
		  p->inExec = false;
		  p->execFilePath = string("");
		}
		break;

	    default:
		sprintf(errorLine, "Got record type %d on sid %d\n", 
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

// TODO -- make this a process method
int handleSigChild(int pid, int status)
{
    int sig;
    string buffer;

    // ignore signals from unknown processes
    process *curr = findProcess(pid);
    if (!curr) return -1;

    if (WIFSTOPPED(status)) {
	sig = WSTOPSIG(status);
	switch (sig) {

	    case SIGTSTP:
		sprintf(errorLine, "process %d got SIGTSTP", pid);
		statusLine(errorLine);
		curr->Stopped();
		break;

	    case SIGTRAP:
		/* trap at the start of a ptraced process 
		 *   continue past it.
		 */
		//If the list is not empty, it means some previous
		//instrumentation has yet need to be finished.
		if (!(instWList.empty())) {
		    curr -> cleanUpInstrumentation();
		}
		//else, do what we are doing before   
		else {
		    if (curr->inExec) {
			// the process has executed a succesful exec, and is now
			// stopped at the exit of the exec call.
			curr->handleExec();
			curr->inExec = false;
			curr->reachedFirstBreak = false;
		    }
		    
		    // the process is stopped as a result of the initial SIGTRAP
		    curr->status_ = stopped;
		    
		    // query default instrumentation here - not done yet
		    // We must check that this is the first trap since on machines
		    //   where we use ptrace detach, a TRAP is generated on a pause.
		    //   - jkh 7/7/95
		    if (!curr->reachedFirstBreak) {
			buffer = string("PID=") + string(pid);
			buffer += string(", passed trap at start of program");
			statusLine(P_strdup(buffer.string_of()));
			installDefaultInst(curr, initialRequests);
			curr->reachedFirstBreak = 1;
			
			// propagate any metric that is already enabled to the new
			// process
			vector<metricDefinitionNode *> MIs = allMIs.values();
			for (unsigned j = 0; j < MIs.size(); j++) {
			    MIs[j]->propagateMetricInstance(curr);
			}
			costMetric::addProcessToAll(curr);
			
			tp->newProgramCallbackFunc(pid, curr->arg_list, 
						   machineResource->part_name());
			
		    }
		}
		break;

	    case SIGSTOP:
	    case SIGINT:

		if (curr->reachedFirstBreak == false && curr->status() == neonatal) {
		  // forked process
		  curr->reachedFirstBreak = true;
		  curr->status_ = stopped;
		  metricDefinitionNode::handleFork(curr->parent, curr);
		  tp->newProgramCallbackFunc(pid, curr->arg_list,
					     machineResource->part_name());
		  break;
		}

		curr->Stopped();
		curr->reachedFirstBreak = 1;

		// The Unix process should be stopped already, 
		// since it's blocked on ptrace and we didn't forward
		// received the SIGSTOP...
		// But we need to pause the rest of the application
		//pauseAllProcesses();
		//statusLine("application paused");
		break;

	    case SIGIOT:
	    case SIGBUS:
	    case SIGILL:
		curr->status_ = stopped;
		dumpProcessImage(curr, true);
		OS::osDumpCore(pid, "core.real");
		//handleProcessExit(curr);
		// ???
		// should really log this to the error reporting system.
		// jkh - 6/25/96
		// now forward it to the process.
		OS::osForwardSignal(pid, WSTOPSIG(status));
		break;

	    case SIGCHLD:
	    case SIGUSR1:
	    case SIGUSR2:
	    case SIGALRM:
	    case SIGVTALRM:
	    case SIGCONT:
	    case SIGSEGV:	// treadmarks needs this signal
		// printf("caught signal, forwarding...  (sig=%d)\n", 
		//       WSTOPSIG(status));
		if (!OS::osForwardSignal(pid, WSTOPSIG(status))) {
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
		if (!OS::osForwardSignal(pid, WSTOPSIG(status))) {
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
	sprintf(errorLine, "Process %d has terminated\n", curr->pid);
	statusLine(errorLine);
	logLine(errorLine);

	printDyninstStats();
        handleProcessExit(curr, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
	sprintf(errorLine, "process %d has terminated on signal %d\n", curr->pid,
	    WTERMSIG(status));
	logLine(errorLine);
	statusLine(errorLine);
	handleProcessExit(curr, WTERMSIG(status));
    } else {
	sprintf(errorLine, "Unknown state %d from process %d\n", status, curr->pid);
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


/*
 * Wait for a data from one of the inferriors or a request to come in.
 *
 */
void controllerMainLoop(bool check_buffer_first)
{
    int ct;
    int pid;
    int width;
    int status;
    fd_set readSet;
    fd_set errorSet;
    struct timeval pollTime;

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

    while (1) {
	FD_ZERO(&readSet);
	FD_ZERO(&errorSet);
	width = 0;
	unsigned p_size = processVec.size();
	for (unsigned u=0; u<p_size; u++) {
	    if (processVec[u]->traceLink >= 0)
	      FD_SET(processVec[u]->traceLink, &readSet);
	    if (processVec[u]->traceLink > width)
	      width = processVec[u]->traceLink;
	    if (processVec[u]->ioLink >= 0)
	      FD_SET(processVec[u]->ioLink, &readSet);
	    if (processVec[u]->ioLink > width)
	      width = processVec[u]->ioLink;
	}

	// add trace 
	extern int traceSocket_fd;
	if (traceSocket_fd > 0) FD_SET(traceSocket_fd, &readSet);
	if (traceSocket_fd > width) width = traceSocket_fd;

	// add connection to paradyn process.
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

	// poll for IO
	pollTime.tv_sec = 0;
	pollTime.tv_usec = 50000;

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
	      // to get a connection for the trace stream. 
	      // Accept the connection.
	      int fd = RPC_getConnect(traceSocket_fd);
	      assert(fd >= 0);
	      int pid;
	      if (read(fd, &pid, sizeof(pid)) != sizeof(pid))
		abort();
	      fprintf(stderr, "Connected to process %d\n", pid);
	      process *p = findProcess(pid);
	      assert(p);
	      p->traceLink = fd;
	    }

            unsigned p_size = processVec.size();
	    for (unsigned u=0; u<p_size; u++) {
		if ((processVec[u]->traceLink >= 0) && 
		    FD_ISSET(processVec[u]->traceLink, &readSet)) {
		    processTraceStream(processVec[u]);
		    /* clear it in case another process is sharing it */
		    if (processVec[u]->traceLink >= 0) 
			FD_CLR(processVec[u]->traceLink, &readSet);
		}

		if ((processVec[u]->ioLink >= 0) && 
		    FD_ISSET(processVec[u]->ioLink, &readSet)) {
		    processAppIO(processVec[u]);
		    /* clear it in case another process is sharing it */
		    if (processVec[u]->ioLink >= 0) 
			FD_CLR(processVec[u]->ioLink, &readSet);
		}
	    }
	    if (FD_ISSET(tp->get_fd(), &errorSet)) {
		// paradyn is gone so we got too.
	        cleanUpAndExit(-1);
	    }
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
	processArchDependentTraceStream();

	/* generate internal metrics */
	reportInternalMetrics();

	/* check for status change on inferrior processes */
	pid = process::waitProcs(&status);
	if (pid > 0) {
	    handleSigChild(pid, status);
	}
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

