/*
 * Copyright (c) 1996-1999 Barton P. Miller
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

// $Id: perfStream.C,v 1.121 2001/08/23 14:44:22 schendel Exp $

#ifdef PARADYND_PVM
extern "C" {
#include <pvm3.h>
}
#include "pvm_support.h"
#endif

#include "common/h/headers.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/dyninstP.h"
#include "paradynd/src/metric.h"
#include "dyninstAPI/src/util.h"
#include "paradynd/src/comm.h"
#include "dyninstAPI/src/stats.h"
#include "paradynd/src/debugger.h"
#include "paradynd/src/main.h"
#include "paradynd/src/association.h"
#include "paradynd/src/init.h"
#include "paradynd/src/context.h"
#include "paradynd/src/perfStream.h"
#include "paradynd/src/dynrpc.h"
#include "dyninstAPI/src/os.h"
#include "paradynd/src/mdld.h"
#include "dyninstAPI/src/showerror.h"
#include "paradynd/src/main.h"
#include "common/h/debugOstream.h"
#include "pdutil/h/pdDebugOstream.h"
#include "pdutil/h/airtStreambuf.h"

// trace data streams
#include "common/h/Dictionary.h"

//#define TEST_DEL_DEBUG 1

// The following were all defined in process.C (for no particular reason)
extern debug_ostream attach_cerr;
extern debug_ostream inferiorrpc_cerr;
extern debug_ostream shmsample_cerr;
extern debug_ostream forkexec_cerr;
extern debug_ostream signal_cerr;
extern debug_ostream sharedobj_cerr;
extern pdDebug_ostream sampleVal_cerr;

string traceSocketPath; /* file path for trace socket */
int traceConnectInfo;
int traceSocketPort;

static void createResource(int pid, traceHeader *header, struct _newresource *r);
bool firstSampleReceived = false;

/*removed for output redirection
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
	// end of file -- process exited 
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
*/

char errorLine[1024];

void logLineN(const char *line, int n) {
    static char fullLine[1024];
    //cerr << "logLineN: " << n << "- " << line << "\n";
    if (strlen(fullLine) + strlen(line) >= 1024) {
       tp->applicationIO(0, strlen(fullLine), fullLine);
       fullLine[0] = '\0';
    }

    assert(strlen(fullLine) + strlen(line) < 1024) ;
    int curlen=0;
    curlen = strlen(fullLine);
    strcat(fullLine, line);
    fullLine[curlen+n] = '\0';

       // Ack!  Possible overflow!  Possible bug!
       // If you put a '\n' at the end of every string passed to a call
       // to logLine (and the string is < 1000 chars) then you'll be okay.
       // Otherwise, watch out!
    //cerr << "checking line (" << fullLine << ") for nl\n";
    if (strstr(&fullLine[strlen(fullLine)-2],"\n")) {
      //cerr << "*logLineN - outputting: " << fullLine << "\n";
        tp->applicationIO(0, strlen(fullLine), fullLine);
	fullLine[0] = '\0';
    }
}

void logLine(const char *line) {
  logLineN(line, strlen(line));
}

void statusLineN(const char *line, int n) {
  //  cerr << "statusLineN: " << n << "- " << line << "\n"; 
  static char buff[300];
  if(n>299) n=299;
  strncpy(buff, line, n+1);
  tp->reportStatus(buff);
}

void statusLine(const char *line) {
  statusLineN(line, strlen(line));
}

airtStreambuf logLineStreamBuf(&logLineN);
airtStreambuf statusLineStreamBuf(&statusLineN);
ostream logStream(&logLineStreamBuf);
ostream statusStream(&logLineStreamBuf);


// New with paradynd-->paradyn buffering.  When true, it tells the
// buffering routine to flush (to paradyn); otherwise, it would flush
// only when the buffer was full, hurting response time.
extern bool BURST_HAS_COMPLETED;

extern vector<process*> processVec;
extern process* findProcess(int); // should become a static method of class process

// trace data streams
extern bool TRACE_BURST_HAS_COMPLETED;
unsigned mid_hash(const unsigned &mid) {return mid;}
dictionary_hash<unsigned, unsigned> traceOn(mid_hash);

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

        unsigned curr_bufStart = curr->bufStart;
	memcpy(&sid, &(curr->buffer[curr->bufStart]), sizeof(traceStream));
	curr->bufStart += sizeof(traceStream);

	memcpy(&header, &(curr->buffer[curr->bufStart]), sizeof(header));
	curr->bufStart += sizeof(header);

	curr->bufStart = ALIGN_TO_WORDSIZE(curr->bufStart);
	if (header.length % WORDSIZE != 0) {
	  sprintf(errorLine, "Warning: non-aligned length (%d) received"
                " on traceStream.  Type=%d\n", header.length, header.type);
	  logLine(errorLine);
	  showErrorCallback(36,(const char *) errorLine);
	}
	    
	if (curr->bufEnd - curr->bufStart < (unsigned)header.length) {
	    /* the whole record isn't here yet */
	    // curr->bufStart -= sizeof(traceStream) + sizeof(header);
            curr->bufStart = curr_bufStart;
	    break;
	}

	recordData = &(curr->buffer[curr->bufStart]);
	curr->bufStart +=  header.length;

	switch (header.type) {
#if defined(MT_THREAD)
	    case TR_THR_CREATE:
		// sprintf(errorLine, "paradynd received TR_THR_CREATE, curr=0x%x", curr) ;
		// cerr << errorLine <<endl ;
	        createThread((traceThread *) ((void*)recordData));
                break;
	    case TR_THR_SELF:
		// sprintf(errorLine, "paradynd received TR_THR_SELF, curr=0x%x", curr) ;
		// cerr << errorLine <<endl ;
	        updateThreadId((traceThread *) ((void*)recordData));
                break;
	    case TR_THR_DELETE:
	        deleteThread((traceThread *) ((void*)recordData));
	        break;
#endif
	    case TR_NEW_RESOURCE:
	      //cerr << "paradynd: received a new resource from pid " 
              //     << curr->getPid() << "; processing now" << endl;
		createResource(curr->getPid(), &header, (struct _newresource *) ((void*)recordData));
		   // createResource() is in this file, below
		break;

	    case TR_NEW_ASSOCIATION:
		a = (struct _association *) ((void*)recordData);
		newAssoc(curr, a->abstraction, a->type, a->key, a->value);
		break;

#ifndef SHM_SAMPLING
	    case TR_SAMPLE:
		// metric_cerr << "got something from pid " << curr->getPid() << endl;

		 // sprintf(errorLine, "Got data from process %d\n", curr->getPid());
		 // logLine(errorLine);
//		assert(curr->getFirstRecordTime());
		processSample(curr->getPid(), &header, (traceSample *) ((void*)recordData));
		   // in metric.C
		firstSampleReceived = true;
		break;
#endif

 
	    case TR_EXIT:
            {
		/* 03/09/2001 - Jeffrey Shergalis
		 * Under Optimization level 3 on SPARC this portion of
		 * code was breaking due to an unalligned address issue
		 * to fix this, we create an endStatsRec struct and memcopy
		 * all of the data into it, then pass the address of that
		 * struct to the printAppStats call
		 */
                struct endStatsRec r;
                sprintf(errorLine, "process %d exited\n", curr->getPid());
                logLine(errorLine);
                memcpy(&r, recordData, sizeof(r));
                printAppStats(&r);
                printDyninstStats();
                P_close(curr->traceLink);
                curr->traceLink = -1;
                handleProcessExit(curr, 0);
                break;
            }

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

	     case TR_DYNAMIC_CALLEE_FOUND:
	       {
		 pd_Function *caller, *callee;
		 resource *caller_res, *callee_res;
		 vector<shared_object *> *sh_objs = NULL;
		 image *symbols = curr->getImage();
		 callercalleeStruct *c = (struct callercalleeStruct *) 
		   ((void*)recordData);

		 //cerr << "DYNAMIC trace record received!!, caller = " << hex 
		 //   << c->caller << " callee = " << c->callee << dec << endl;
		 assert(symbols);	
		 if (curr->isDynamicallyLinked())
		   sh_objs  = curr->sharedObjects();
		 // Have to look in main image and (possibly) in shared objects
		 caller = symbols->findFuncByAddr(c->caller, curr);
		 if (!caller && sh_objs)
		   {
		     for(u_int j=0; j < sh_objs->size(); j++)
		       {
			 caller = ((*sh_objs)[j])->getImage()->findFuncByAddr(c->caller, curr);
			 if (caller) break;
		       }
		   }

		 callee = symbols->findFuncByAddr(c->callee, curr);
		 if (!callee && sh_objs)
		   {
		     for(u_int j=0; j < sh_objs->size(); j++)
		       {
			 callee = ((*sh_objs)[j])->getImage()->findFuncByAddr(c->callee, curr);
			 if (callee) break;
		       }
		   }
		 if(!callee || !caller){
		   cerr << "callee for addr " << hex << c->callee <<dec
			<< " not found, caller = " <<
		     caller->ResourceFullName() << endl;
		   break;
		 }
		 
		 /*If the callee's FuncResource isn't set, then
		   the callee must be uninstrumentable, so we don't
		   notify the front end.*/
		 if(callee->FuncResourceSet() && caller->FuncResourceSet()){
		   callee_res =  
		     resource::findResource(callee->ResourceFullName());
		   caller_res =
		     resource::findResource(caller->ResourceFullName());
		   assert(callee_res);
		   assert(caller_res);
		   tp->AddCallGraphDynamicChildCallback(symbols->file(),
						      caller_res->full_name(), 
						      callee_res->full_name());
		 }
		 break;
	       }
            case TR_DATA:
                extern void batchTraceData(int, int, int, char *);
                batchTraceData(0, sid, header.length, recordData);
                traceOn[sid] = 1;
                break;
 	    case TR_ERROR: // also used for warnings
	        { 

		  rtUIMsg *rtMsgPtr;
		  rtMsgPtr = (rtUIMsg *) recordData;
		  showErrorCallback(rtMsgPtr->errorNum, rtMsgPtr->msgString);
		  if(rtMsgPtr->msgType == rtError) {
		    // if need this, might want to add code
		    // to shut down paradyn and/or app
		  }
	        }
		break;
	    case TR_SYNC:
		// to eliminate a race condition, --Zhichen
		break ;
	    default:
		sprintf(errorLine, "Got unknown record type %d on sid %d\n", 
		    header.type, sid);
		logLine(errorLine);
		sprintf(errorLine, "Received bad trace data from process %d.", curr->getPid());
		showErrorCallback(37,(const char *) errorLine);
	}
    }
    BURST_HAS_COMPLETED = true; // will force a batch-flush very soon

    // trace data streams
    for (dictionary_hash_iter<unsigned,unsigned> iter=traceOn; iter; iter++) {
       const unsigned key = iter.currkey();
       unsigned val = iter.currval();
       
       if (val) {
             extern void batchTraceData(int, int, int, char *);
	     TRACE_BURST_HAS_COMPLETED = true;
	     // will force a trace-batch-flush very soon
             batchTraceData(0, key, 0, (char *)NULL);
             traceOn[key] = 0;
             //sprintf(errorLine, "$$$Tag burst with mid %d\n", k);
             //logLine(errorLine);
        }
    }

    /* copy those bits we have to the base */
    memcpy(curr->buffer, &(curr->buffer[curr->bufStart]), 
	curr->bufEnd - curr->bufStart);
    curr->bufEnd = curr->bufEnd - curr->bufStart;
}


extern vector<defInst*> instrumentationToDo;
extern int startCollecting(string& metric_name, vector<u_int>& focus, 
                           int id, vector<process *> &procsToCont);

void doDeferredInstrumentation() {
  string metric;
  vector<u_int> focus; 
  int id;
 
  for (unsigned i=0; i < instrumentationToDo.size(); i++) {

    vector<process *> procsToContinue;
    metric = instrumentationToDo[i]->metric();
    focus  = instrumentationToDo[i]->focus();
    id     = instrumentationToDo[i]->id();

    bool instrumented = startCollecting(metric, focus, id, procsToContinue);

    // continue the processes that were stopped in start collecting
    for (unsigned u = 0; u < procsToContinue.size(); u++) {

#ifdef DETACH_ON_THE_FLY
      procsToContinue[u]->detachAndContinue();
#else
      procsToContinue[u]->continueProc();
#endif

    }

    if (instrumented) {

      int numDeferred = instrumentationToDo.size();
      // delete the defInst object for the deferred instrumentation that was 
      // successfully inserted and shift the defInst objects in the 
      // instrumentationToDo vector, resizing the vector
      delete instrumentationToDo[i];

      for (int j=i; j < numDeferred-1; j++) {
        instrumentationToDo[j] = instrumentationToDo[j+1];
      }

      instrumentationToDo.resize(numDeferred - 1); 
      
      // defInst objects were shifted, so index i has new deferred 
      // instrumentation to be inserted
      i--;
    }  
  }


}

void doDeferredRPCs() {
   // Any RPCs waiting to be performed?  If so, and if it's safe to
   // perform one, then launch one.
   for (unsigned lcv=0; lcv < processVec.size(); lcv++) {
      process *proc = processVec[lcv];
      if (proc == NULL) continue; // proc must've died and has itself cleaned up
      if (proc->status() == exited) continue;
      if (proc->status() == neonatal) continue; // not sure if this is appropriate
      
      bool wasLaunched = proc->launchRPCifAppropriate(proc->status() == running,
						      false);
#if defined(TEST_DEL_DEBUG)
   if (wasLaunched) logLine("***** inferiorRPC launched, perfStream.C\n");
#endif

      // do we need to do anything with 'wasLaunched'?
      if (wasLaunched)
 	 inferiorrpc_cerr << "fyi: launched an inferior RPC" << endl;
   }
}

void ioFunc()
{
     printf("in SIG child func\n");
     fflush(stdout);
}

#ifdef SHM_SAMPLING
static void checkAndDoShmSampling(timeLength *pollTime) {
   // We assume that nextShmSampleTime (synched to getCurrWallTime())
   // has already been set.  If the curr time is >= to this, then
   // we should sample immediately, and update nextShmSampleTime to
   // be nextShmSampleTime + sampleInterval, unless it is <= currTime,
   // in which case we set it to currTime + sampleInterval.

   // QUESTION: should we sample a given process while it's paused?  While it's
   //           in the middle of an inferiorRPC?  For now, the answer is no
   //           to both.
   static timeStamp nextMajorSampleTime = timeStamp::ts1970();
   static timeStamp nextMinorSampleTime = timeStamp::ts1970();
   const timeStamp currWallTime = getWallTime();
      // checks for rollback

   bool doMajorSample = false; // so far...
   bool doMinorSample = false; // so far...

   bool forNextTimeDoMinorSample = false; // so far...

   if (currWallTime >= nextMajorSampleTime)
      doMajorSample = true;
   else if (currWallTime >= nextMinorSampleTime)
      doMinorSample = true;
   else {
      // it's not time to do anything.
      return;
   }
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
   assert(getCurrSamplingRate() > timeLength::Zero());
   timeLength shmSamplingInterval = getCurrSamplingRate();

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
      nextMinorSampleTime = currWallTime + 20*timeLength::ms();
      if (nextMinorSampleTime > nextMajorSampleTime)
	 // oh, never mind, we'll just do the major sample which is going to
	 // happen first anyway.
	 nextMinorSampleTime = nextMajorSampleTime;
   }
   else {
      // we don't need to do a minor sample next time, so reset nextMinorSampleTime
      nextMinorSampleTime = nextMajorSampleTime;
   }

   timeStamp nextAnyKindOfSampleTime = nextMajorSampleTime;
   if (nextMinorSampleTime < nextAnyKindOfSampleTime)
      nextAnyKindOfSampleTime = nextMinorSampleTime;

   assert(nextAnyKindOfSampleTime >= currWallTime);
   const timeLength shmSamplingTimeout = nextAnyKindOfSampleTime - currWallTime;

   if (shmSamplingTimeout < *pollTime)
      *pollTime = shmSamplingTimeout;
}
#endif


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
    struct timeval pollTimeStruct;
    PDSOCKET traceSocket_fd;

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

#if !defined(i386_unknown_nt4_0)
      traceSocketPath = string(P_tmpdir) + "/" + string("paradynd.") + string(getpid());

    // unlink it, in case the file was left around from a previous run
    unlink(traceSocketPath.string_of());

    if (!RPC_setup_socket_un(traceSocket_fd, traceSocketPath.string_of())) {
      perror("paradynd -- can't setup socket");
      cleanUpAndExit(-1);
    }
    traceConnectInfo = getpid();
#else
    traceSocketPort = RPC_setup_socket(traceSocket_fd, PF_INET, SOCK_STREAM);
    if (traceSocketPort < 0) {
      perror("paradynd -- can't setup socket");
      cleanUpAndExit(-1);
    }
    traceConnectInfo = traceSocketPort;
#endif

    while (1) {
        // we have moved this code at the beginning of the loop, so we will
        // process signals before igen requests: this is to avoid problems when
        // an inferiorRPC is waiting for a system call to complete and an igen
        // requests arrives at that moment - naim
	extern void checkProcStatus(); // check status of inferior processes
	checkProcStatus();

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

	    //removed for output redirection
	    //if (processVec[u]->ioLink >= 0)
	    //  FD_SET(processVec[u]->ioLink, &readSet);
	    //if (processVec[u]->ioLink > width)
	    //  width = processVec[u]->ioLink;
	}

	// add traceSocket_fd, which accept()'s new connections (from processes
	// not launched via createProcess() [process.C], such as when a process
	// forks, or when we attach to an already-running process).
	if (traceSocket_fd != INVALID_PDSOCKET) FD_SET(traceSocket_fd, &readSet);
	if (traceSocket_fd > width) width = traceSocket_fd;

	// add our igen connection with the paradyn process.
	FD_SET(tp->get_sock(), &readSet);
	FD_SET(tp->get_sock(), &errorSet);

	// "width" is computed but ignored on Windows NT, where sockets 
	// are not represented by nice little file descriptors.
	if (tp->get_sock() > width) width = tp->get_sock();

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

#if defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0)
        doDeferredInstrumentation();
#endif

	extern void doDeferedRPCasyncXDRWrite();
	doDeferedRPCasyncXDRWrite();

#if !defined(i386_unknown_nt4_0)
	timeLength pollTime(50, timeUnit::ms());
           // this is the time (rather arbitrarily) chosen fixed time length
           // in which to check for signals, etc.
#else
	// Windows NT wait happens in WaitForDebugEvent (in pdwinnt.C)
	timeLength pollTime = timeLength::Zero();
#endif

#ifdef SHM_SAMPLING
        checkAndDoShmSampling(&pollTime);
           // does shm sampling of each process, as appropriate.
           // may update pollTimeUSecs.
#endif 

#if defined(i386_unknown_linux2_0)
#ifndef DETACH_ON_THE_FLY
        pollTime = timeLength::Zero(); // hack for fairer trap servicing on Linux

        // Linux select has a granularity of 100Hz, i.e., 10000usecs
        // meaning that it can at best service 100 traps/second
	// when we're attached to the inferior.
#endif /* not DETACH_ON_THE_FLY */
#endif

	pollTimeStruct.tv_sec  = static_cast<long>(pollTime.getI(
							   timeUnit::sec()));
	pollTimeStruct.tv_usec = static_cast<long>(pollTime.getI(
                                                           timeUnit::us()));

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
	ct = P_select(width+1, &readSet, NULL, &errorSet, &pollTimeStruct);

	if (ct > 0) {

	    if (traceSocket_fd >= 0 && FD_ISSET(traceSocket_fd, &readSet)) {
	      // Either (1) a process we're measuring has forked, and the child
	      // process is asking for a new connection, or (2) a process we've
	      // attached to is asking for a new connection.

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

		/* removed for output redirection
		if (processVec[u] && processVec[u]->ioLink >= 0 && 
    		       FD_ISSET(processVec[u]->ioLink, &readSet)) {
		    processAppIO(processVec[u]);

		    // app can (conceivably) die in processAppIO(), resulting
		    // in a processVec[u] to NULL.

		    // clear it in case another process is sharing it 
		    if (processVec[u] && processVec[u]->ioLink >= 0)
		       // may have been set to -1
		       FD_CLR(processVec[u]->ioLink, &readSet);
		}
		*/
	    }

#if !defined(i386_unknown_nt4_0)
	    if (FD_ISSET(tp->get_sock(), &errorSet)) {
		// paradyn is gone so we go too.
	        cleanUpAndExit(-1);
	    }
#else // !defined(i386_unknown_nt4_0)

        // WinSock indicates the socket closed
        // as a read event.  When reading on
        // the socket, the number of bytes available
        // is zero.

        if( FD_ISSET( tp->get_sock(), &readSet ))
        {
            int junk;
            int nbytes = recv( tp->get_sock(),
                                (char*)&junk,
                                sizeof(junk),
                                MSG_PEEK );
            if( nbytes == 0 )
            {
                // paradyn is gone so we go too
                cleanUpAndExit(-1);
            }
        }
#endif // !defined(i386_unknown_nt4_0)

            bool delayIGENrequests=false;
	    for (unsigned u1=0; u1<p_size; u1++) {
	      if (processVec[u1] == NULL)
	        continue; // process structure has been deallocated
 
              if (processVec[u1]->isRPCwaitingForSysCallToComplete() &&
		  processVec[u1]->status() == running) {
		delayIGENrequests=true;
		break;
	      }
	    }

            // if we are waiting for a system call to complete in order to
            // launch an inferiorRPC, we will avoid processing any igen
	    // request - naim
	    if (!delayIGENrequests) {
              // Check if something has arrived from Paradyn on our igen link.
	      if (FD_ISSET(tp->get_sock(), &readSet)) {
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
    }
}


static void createResource(int pid, traceHeader *header, struct _newresource *r)
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

    timeStamp trWall(timeStamp::ts1970());
    trWall = getWallTimeMgr().units2timeStamp(header->wall);

    if ((parent = resource::findResource(parent_name)) && name != r->name) {
      resource::newResource(parent, NULL, r->abstraction, name,
			    trWall, "", type, true);
    }
    else {
      string msg = string("Unknown resource '") + string(r->name) +
	           string("' reported on trace stream from PID=") +
		   string(pid);
      showErrorCallback(36,msg);
    }

}
