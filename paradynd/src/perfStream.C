/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: perfStream.C,v 1.181 2005/09/02 00:45:29 bernat Exp $

#include "common/h/headers.h"
#include "common/h/timing.h" // getCyclesPerSecond
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "paradynd/src/machineMetFocusNode.h"
#include "paradynd/src/comm.h"
#include "paradynd/src/debugger.h"
#include "paradynd/src/main.h"
#include "paradynd/src/init.h"
#include "paradynd/src/context.h"
#include "paradynd/src/perfStream.h"
#include "paradynd/src/dynrpc.h"
#include "paradynd/src/mdld.h"
#include "paradynd/src/main.h"
#include "paradynd/src/pd_module.h"
#include "common/h/debugOstream.h"
#include "common/h/int64iostream.h"
#include "pdutil/h/pdDebugOstream.h"
#include "paradynd/src/processMgr.h"
#include "paradynd/src/pd_process.h"
#include "pdutil/h/airtStreambuf.h"
#include "pdutil/h/mdl_data.h"
#include "paradynd/src/debug.h"

#include "dyninstAPI/src/mapped_object.h"

// trace data streams
#include "common/h/Dictionary.h"

//#define TEST_DEL_DEBUG 1

// minimum interval between ReportSelf calls to front-end
#if defined(THROTTLE_RS)
#  define THROTTLE_RS_INTERVAL INT32_MAX
#endif // defined(THROTTLE_RS)

extern unsigned int metResPairsEnabled; // definied in metricFocusNode.C

#if 1
//  As far as I can tell these vars are unused except in the
//  functions that print statistics...  kept for legacy purposes
//  (moved here from dyninstAPI/src/stats.C)
//  Unless future use of these is expected, they can probably be
//  deleted
unsigned int fociUsed = 0;
unsigned int metricsUsed = 0;
unsigned int samplesDelivered = 0;
#endif

pdstring traceSocketPath; /* file path for trace socket */
int traceConnectInfo;
int traceSocketPort;

static void createResource(int pid, traceHeader *header, struct _newresource *r);
static void updateResource(int pid, traceHeader *header, struct _updtresource *r);
bool firstSampleReceived = false;

extern bool isInfProcAttached;

/* removed for output redirection
// Read output data from process curr. 
void processAppIO(process *curr)
{
   int ret;
   char lineBuf[256];
   
#if !defined(i386_unknown_nt4_0)
   ret = read(curr->ioLink, lineBuf, sizeof(lineBuf)-1);
#else
   ret = P_recv(curr->ioLink, lineBuf, sizeof(lineBuf)-1, 0);
#endif
   if (ret < 0) {
      //statusLine("read error");
      //showErrorCallback(23, "Read error");
      //cleanUpAndExit(-2);
      pdstring msg = pdstring("Read error on IO stream from PID=") +
         pdstring(curr->getPid()) + pdstring(": ") +
         pdstring(strerror(errno)) + 
         pdstring("\nNo more data will be received from this process.");
      showErrorCallback(23, msg);
      P_close(curr->ioLink);
      curr->ioLink = -1;
      return;
   } else if (ret == 0) {
      // end of file -- process exited 
      P_close(curr->ioLink);
      curr->ioLink = -1;
      string msg = pdstring("Process ") + pdstring(curr->getPid()) + pdstring(" exited");
      statusLine(msg.c_str());
      curr->handleProcessExit(0);
      
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

void logLineN(const char *line, int n, bool /* force */) {
  // Fix for daemon segfault: don't send messages to the frontend if 
  // it's gone. Happens if the frontend exits.
  if (frontendExited) {
    fprintf(stderr, "Skipping message, frontend exited:\n%s\n", line);
    return;
  }
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
       // If you put a '\n' at the end of every pdstring passed to a call
       // to logLine (and the string is < 1000 chars) then you'll be okay.
       // Otherwise, watch out!
    //cerr << "checking line (" << fullLine << ") for nl\n";
    if (strstr(&fullLine[strlen(fullLine)-2],"\n")) {
      //cerr << "*logLineN - outputting: " << fullLine << "\n";
        tp->applicationIO(0, strlen(fullLine), fullLine);
	fullLine[0] = '\0';
    }
}

#if defined(THROTTLE_RS)
static struct timeval throttleRS_SavedTimestamp;

inline
double 
TimevalDiff( struct timeval& begin, struct timeval& end )
{
    double dbegin = (double)begin.tv_sec + ((double)begin.tv_usec/1000000);
    double dend = (double)end.tv_sec + ((double)end.tv_usec/1000000);

    if( dend < dbegin )
    {
        dend = dbegin;
    }

    return (dend - dbegin);
}

#endif // defined(THROTTLE_RS)

void logLine(const char *line) {
  logLineN(line, strlen(line), true);
}

void statusLineN(const char *line, int n, bool force) {
  if (frontendExited) {
    fprintf(stderr, "Skipping status line, frontend exited:\n%s\n", line);
    return;
  }

#if defined(THROTTLE_RS)
    bool doit = force;

    if( !force )
    {
        struct timeval tv;

        gettimeofday( &tv, NULL );
        if( TimevalDiff( throttleRS_SavedTimestamp, tv ) > THROTTLE_RS_INTERVAL )
        {
            doit = true;
            throttleRS_SavedTimestamp = tv;
        }
    }

    if( doit )
    {
#endif // defined(THROTTLE_RS)

    //  cerr << "statusLineN: " << n << "- " << line << "\n"; 
    static char buff[300];
    if(n>299) n=299;
    strncpy(buff, line, n+1);
    tp->reportStatus(buff);

#if defined(THROTTLE_RS)
    }
#endif // defined(THROTTLE_RS)
}

void statusLine(const char *line, bool force) {
  statusLineN(line, strlen(line), force);
}

airtStreambuf logLineStreamBuf(&logLineN);
airtStreambuf statusLineStreamBuf(&statusLineN);
ostream logStream(&logLineStreamBuf);
ostream statusStream(&logLineStreamBuf);


// New with paradynd-->paradyn buffering.  When true, it tells the
// buffering routine to flush (to paradyn); otherwise, it would flush
// only when the buffer was full, hurting response time.
extern bool BURST_HAS_COMPLETED;

// trace data streams
extern bool TRACE_BURST_HAS_COMPLETED;
unsigned mid_hash(const unsigned &mid) {return mid;}
dictionary_hash<unsigned, unsigned> traceOn(mid_hash);


// Read trace data from process proc.
void processTraceStream(process *dproc)
{
    int ret;
    traceStream sid;
    char *recordData;
    traceHeader header;

#if !defined(i386_unknown_nt4_0)
    ret = read(dproc->traceLink, &(dproc->buffer[dproc->bufEnd]), 
               sizeof(dproc->buffer) - dproc->bufEnd);
#else
    ret = recv(dproc->traceLink, &(dproc->buffer[dproc->bufEnd]), 
               sizeof(dproc->buffer) - dproc->bufEnd, 0);
#endif

    if (ret < 0) {
       //statusLine("read error, exiting");
       //showErrorCallback(23, "Read error");
       //dproc->traceLink = -1;
       //cleanUpAndExit(-2);
       pdstring msg = pdstring("Read error on trace stream from PID=") +
          pdstring(dproc->getPid()) + pdstring(": ") +
          pdstring(strerror(errno)) + 
          pdstring("\nNo more data will be received from this process");
       showErrorCallback(23, msg);
       P_close(dproc->traceLink);
       dproc->traceLink = -1;
       return;
    } else if (ret == 0) {
       /* end of file */
       // process exited unexpectedly
       //string buffer = pdstring("Process ") + pdstring(proc->pid);
       //buffer += pdstring(" has exited unexpectedly");
       //statusLine(P_strdup(buffer.c_str()));
       //showErrorCallback(11, P_strdup(buffer.c_str()));
       pdstring msg = pdstring("Process ") + pdstring(dproc->getPid()) + 
          pdstring(" exited");
       statusLine(msg.c_str());
       P_close(dproc->traceLink);
       dproc->traceLink = -1;
       dproc->handleProcessExit();
       return;
    }

    dproc->bufEnd += ret;
    dproc->bufStart = 0;
    
    while(dproc->bufStart < dproc->bufEnd) {
       if(dproc->bufEnd - dproc->bufStart < 
          (sizeof(traceStream) + sizeof(header))) {
          break;
       }
       
       if(dproc->bufStart % WORDSIZE != 0)     /* Word alignment check */
          break;		        /* this will re-align by shifting */
       
       unsigned curr_bufStart = dproc->bufStart;
       memcpy(&sid, &(dproc->buffer[dproc->bufStart]), sizeof(traceStream));
       dproc->bufStart += sizeof(traceStream);
       
       memcpy(&header, &(dproc->buffer[dproc->bufStart]), sizeof(header));
       dproc->bufStart += sizeof(header);

       dproc->bufStart = ALIGN_TO_WORDSIZE(dproc->bufStart);
       if (header.length % WORDSIZE != 0) {
          sprintf(errorLine, "Warning: non-aligned length (%d) received"
                  " on traceStream.  Type=%d\n", header.length, header.type);
          logLine(errorLine);
          showErrorCallback(36,(const char *) errorLine);
       }
       
       if(dproc->bufEnd - dproc->bufStart < (unsigned)header.length) {
          /* the whole record isn't here yet */
          // dproc->bufStart -= sizeof(traceStream) + sizeof(header);
          dproc->bufStart = curr_bufStart;
          break;
       }
       
       recordData = &(dproc->buffer[dproc->bufStart]);
       dproc->bufStart +=  header.length;

       switch (header.type) {
         case TR_THR_CREATE:
            // cerr << "paradynd received TR_THR_CREATE, dproc: " << dproc
            //      << endl;
            createThread((traceThread *) ((void*)recordData));
            break;
         case TR_THR_SELF:
            // cerr << "paradynd received TR_THR_SELF, dproc: " << dproc
            //      << endl;
            updateThreadId((traceThread *) ((void*)recordData));
            break;
         case TR_THR_DELETE:
            deleteThread((traceThread *) ((void*)recordData));
            break;
         case TR_NEW_RESOURCE:
            //cerr << "paradynd: received a new resource from pid " 
            //     << dproc->getPid() << "; dprocessing now" << endl;
            createResource(dproc->getPid(), &header, 
                           (struct _newresource *) ((void*)recordData));
            // createResource() is in this file, below
            break;
         case TR_UPDATE_RESOURCE:
           //  cerr << "paradynd: received update resource cmd from pid "
           //      << dproc->getPid() << "; dprocessing now" << endl;
            updateResource(dproc->getPid(), &header,
                           (struct _updtresource *) ((void*)recordData));
            // updateResource() is in this file, below
            break;
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
               sprintf(errorLine, "dprocess %d exited\n", dproc->getPid());
               logLine(errorLine);
               memcpy(&r, recordData, sizeof(r));
               printAppStats(&r);
               sprintf(errorLine, "    %d metric/resource pairs enabled\n",
                       metResPairsEnabled);
               logLine(errorLine);
               BPatch_stats &dyn_stats = getBPatch().getBPatchStatistics(); 
               printDyninstStats(dyn_stats);
               P_close(dproc->traceLink);
               dproc->traceLink = -1;
               dproc->handleProcessExit();
               break;
            }

         case TR_CP_SAMPLE: {
            // critical path sample
            extern void processCP(pd_process *, traceHeader *, cpSample *);
            pd_process *p = getProcMgr().find_pd_process(dproc->getPid());
            processCP(p, &header, (cpSample *) recordData);
            break;
         }
         case TR_EXEC_FAILED: 
            { 
#if 0
                int pid = *(int *)recordData;
                pd_process *p = getProcMgr().find_pd_process(pid);
                p->get_dyn_process()->lowlevel_process()->inExec = false;
                p->get_dyn_process()->lowlevel_process()->execFilePath =
                    pdstring("");
#endif
                // Shouldn't happen... asserting to be sure
                assert(0);
            }
            break;
            
         case TR_DYNAMIC_CALLEE_FOUND:
            {
               int_function *caller, *callee;
               resource *caller_res, *callee_res;
               callercalleeStruct *c = (struct callercalleeStruct *) 
                  ((void*)recordData);
               
               //cerr << "DYNAMIC trace record received!!, caller = " << hex 
               //   << c->caller << " callee = " << c->callee << dec << endl;
               // Have to look in main image and (possibly) in shared objects
               codeRange *range;
               range = dproc->findCodeRangeByAddress(c->caller);
               caller = range->is_function();
               
               range = dproc->findCodeRangeByAddress(c->callee);
               callee = range->is_function();

               pd_process *pdp = getProcMgr().find_pd_process(dproc->getPid());
               BPatch_process *bproc = pdp->get_bprocess();
               if (callee)
               {
                  BPatch_function *f = bproc->get_function(callee);
                  callee_res = pd_module::getFunctionResource(f);
               }
               if (caller)
               {
                  BPatch_function *f = bproc->get_function(caller);
                  caller_res = pd_module::getFunctionResource(f);
               }

               if(!callee || !caller){
                  cerr << "callee for addr " << ostream::hex << c->callee 
                       << ostream::dec << " not found\n";
                  if(caller)
                     cerr << "   caller = " << caller_res->full_name()
                          << endl;
                  break;
               }
		 
               /*If the callee's FuncResource isn't set, then
                 the callee must be uninstrumentable, so we don't
                 notify the front end.*/
               if (callee_res && caller_res)
               {
                  tp->AddCallGraphDynamicChildCallback(dproc->getAOut()->fileName(),
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
            sprintf(errorLine, "Received bad trace data from process %d.", 
                    dproc->getPid());
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
    memcpy(dproc->buffer, &(dproc->buffer[dproc->bufStart]), 
           dproc->bufEnd - dproc->bufStart);
    dproc->bufEnd = dproc->bufEnd - dproc->bufStart;
}


extern pdvector<int> deferredMetricIDs;

void doDeferredRPCs() {
    processMgr::procIter itr = getProcMgr().begin();
    while(itr != getProcMgr().end()) {
        pd_process *theProc = *itr;
        itr++;
        if (!theProc) continue;

        if (theProc->isTerminated()) continue;
#ifdef NOTDEF // PDSEP
        // PDSEP note:  "neonatal" is not a concept that is
        //  exported by dyninstAPI, not sure, however, if it is
        //  really a necessary concept here.
        if (status == exited) continue;
        if (status == neonatal) continue;
#endif
        theProc->launchRPCs(!theProc->isStopped());
    }
}


void doDeferredInstrumentation() {
   pdvector<int>::iterator itr = deferredMetricIDs.end();
   while(itr != deferredMetricIDs.begin()) {
      itr--;
      int mid = *itr;

      machineMetFocusNode *machNode;
      machNode = machineMetFocusNode::lookupMachNode(mid);

      if(machNode == NULL) {
          // Machine node went away without removing this
          // ID from the deferred list.
          deferredMetricIDs.erase(itr);
          continue;
      }

      inst_insert_result_t insert_status = machNode->insertInstrumentation();
      metFocInstResponse* cbi = machNode->getMetricFocusResponse();
      assert( cbi != NULL );
      
      if(insert_status == inst_insert_success) {
         deferredMetricIDs.erase(itr);
         machNode->initializeForSampling(getWallTime(), pdSample::Zero());
         
         if(cbi != NULL) {
            cbi->updateResponse( mid, inst_insert_success );
            cbi->makeCallback();
         }
      } else if(insert_status == inst_insert_failure) {
         deferredMetricIDs.erase(itr);

         if(cbi != NULL) {
            cbi->updateResponse( mid,
                                inst_insert_failure,
                                mdl_data::cur_mdl_data->env->getSavedErrorString() );
            cbi->makeCallback();
         }
         delete machNode;
      } // else insert_status == inst_insert_deferred
   }  
}

void ioFunc()
{
     printf("in SIG child func\n");
     fflush(stdout);
}

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

   processMgr::procIter itr = getProcMgr().begin();
   while(itr != getProcMgr().end()) {
       pd_process *theProc = *itr;
       itr++;
       
       if (theProc == NULL)
           continue; // proc died & had its structures cleaned up
       
       // Don't sample paused/exited/neonatal processes, or even running processes
       // that haven't been bootstrapped yet (i.e. haven't called DYNINSTinit yet),
       // or processes that are in the middle of an inferiorRPC (we like for
       // inferiorRPCs to finish up quickly).
       if (theProc->isStopped() || theProc->isTerminated()) {
	 // I don't feel like reimplementing getStatusAsString for pd_process
	 //sample_cerr << "(-" << theProc->getStatusAsString() << "-)";
           continue;
       }
       else if (!theProc->isBootstrappedYet() || !theProc->isPARADYNBootstrappedYet()) { //ccw 1 may 2002 : SPLIT
	 sample_cerr << "(-*-)" << endl;
           continue;
       }

       if (theProc == NULL)
           continue; // proc died & had its structures cleaned up
       
       // Don't sample paused/exited/neonatal processes, or even running
       // processes that haven't been bootstrapped yet (i.e. haven't called
       // DYNINSTinit yet), or processes that are in the middle of an
       // inferiorRPC (we like for inferiorRPCs to finish up quickly).
       if (theProc->isStopped() || theProc->isTerminated()) {
	 //sample_cerr << "(-" << theProc->getStatusAsString() << "-)";
           continue;
       }
       else if (!theProc->isBootstrappedYet() || !theProc->isPARADYNBootstrappedYet()) { //ccw 1 may 2002 : SPLIT
           sample_cerr << "(-*-)" << endl;
           continue;
       }
       
       if (doMajorSample) {
           sample_cerr << "(-Y-)" << endl;
           if (!theProc->doMajorShmSample()) {
               // The major sample didn't complete all of its work, so we
               // schedule a minor sample for sometime in the near future
               // (before the next major sample)
               sample_cerr << "a minor sample will be needed" << endl;
               
               forNextTimeDoMinorSample = true;
           }
       }
       else if (doMinorSample) {
           sample_cerr << "trying needed minor sample..."; cerr.flush();
           
           if (!theProc->doMinorShmSample()) {
               // The minor sample didn't complete all of its work, so
               // schedule another one.
               forNextTimeDoMinorSample = true;
               sample_cerr << "it failed" << endl; cerr.flush();
           }
           else {
               sample_cerr << "it succeeded" << endl; cerr.flush();
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
      // one-fourth of the original sampling rate...i.e. for now + (0.2
      // sec/4) = now + (0.05 sec), i.e. now + 50 milliseconds.  temp:
      // one-tenth of original sample rate...i.e. for now + 0.02 sec (+20
      // millisec)

      //      nextMinorSampleTime = currWallTime + 50000; // 50ms = 50000us
      nextMinorSampleTime = currWallTime + 20*timeLength::ms();
      if (nextMinorSampleTime > nextMajorSampleTime)
         // oh, never mind, we'll just do the major sample which is going to
         // happen first anyway.
         nextMinorSampleTime = nextMajorSampleTime;
   }
   else {
      // we don't need to do a minor sample next time, so reset
      // nextMinorSampleTime
      nextMinorSampleTime = nextMajorSampleTime;
   }

   timeStamp nextAnyKindOfSampleTime = nextMajorSampleTime;
   if (nextMinorSampleTime < nextAnyKindOfSampleTime)
      nextAnyKindOfSampleTime = nextMinorSampleTime;

   assert(nextAnyKindOfSampleTime >= currWallTime);
   const timeLength shmSamplingTimeout = nextAnyKindOfSampleTime -currWallTime;
   if (shmSamplingTimeout < *pollTime)
      *pollTime = shmSamplingTimeout;
}

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

  PDSOCKET traceSocket_fd = INVALID_PDSOCKET;
void setupTraceSocket()
{

#if !defined(i386_unknown_nt4_0)
  traceSocketPath = pdstring(P_tmpdir) + "/" + pdstring("paradynd.") + pdstring(getpid());
  
  // unlink it, in case the file was left around from a previous run
  unlink(traceSocketPath.c_str());
  
  if (!RPC_setup_socket_un(traceSocket_fd, traceSocketPath.c_str())) {
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
   struct timeval pollTimeStruct;
   
   while (1) {
      // we have moved this code at the beginning of the loop, so we will
      // process signals before igen requests: this is to avoid problems when
      // an inferiorRPC is waiting for a system call to complete and an igen
      // requests arrives at that moment - naim
      if( isInfProcAttached )
      {
          pdvector <procevent *> events = getSH()->checkForAndHandleProcessEvents(false);
          if (events.size()) {
              // Unhandled events... we don't want this, as we don't
              // expect to have signals etc. occur outside of the
              // process (object)-layer code
              for (unsigned i = 0; i < events.size(); i++) {
#if 0
		fprintf(stderr, "Unhandled event: (why %d, what %d) on process %d\n",
                          events[i]->why,
                          events[i]->what,
                          events[i]->proc->getPid());
#endif
#if !defined(os_windows)
                  if (events[i]->why == procSignalled)
                      forwardSigToProcess(*(events[i]));
#endif
              }
          }
          
      } 
     
      FD_ZERO(&readSet);
      FD_ZERO(&errorSet);
      width = 0;

      processMgr::procIter itrA = getProcMgr().begin();
      while(itrA != getProcMgr().end()) {
         pd_process *curProc = *itrA++;
         if(curProc == NULL)
            continue;
	 
         if(curProc->getTraceLink() >= 0)
            FD_SET(curProc->getTraceLink(), &readSet);
         if(curProc->getTraceLink() > width)
            width = curProc->getTraceLink();
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

      // Clean up any inferior RPCs that might still be queued do to a failure
      // to start them
      doDeferredRPCs();

#if defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */
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
      
      checkAndDoShmSampling(&pollTime);
      // does shm sampling of each process, as appropriate.
      // may update pollTimeUSecs.

      pollTimeStruct.tv_sec  = 
         static_cast<long>(pollTime.getI(timeUnit::sec()));
      pollTimeStruct.tv_usec = 
         static_cast<long>(pollTime.getI(timeUnit::us()));

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

      if (ct <= 0)   continue;

      if (traceSocket_fd >= 0 && FD_ISSET(traceSocket_fd, &readSet)) {
         // Either (1) a process we're measuring has forked, and the child
         // process is asking for a new connection, or (2) a process we've
         // attached to is asking for a new connection.
         processNewTSConnection(traceSocket_fd); // context.C
      }
      
      processMgr::procIter itr = getProcMgr().begin();
      while(itr != getProcMgr().end()) {
         pd_process *curProc = *itr++;
         if(curProc == NULL)
            continue; // process structure has been deallocated
         if(curProc && curProc->getTraceLink() >= 0 && 
            FD_ISSET(curProc->getTraceLink(), &readSet)) {
            processTraceStream(curProc->get_dyn_process()->lowlevel_process());
            /* in the meantime, the process may have died, setting
               curProc to NULL */
            
            /* clear it in case another process is sharing it */
            if (curProc && curProc->getTraceLink() >= 0) {
               // may have been set to -1
               FD_CLR(curProc->getTraceLink(), &readSet);
            }
         }
      }
#if !defined(i386_unknown_nt4_0)
      if (FD_ISSET(tp->get_sock(), &errorSet)) {
	// Don't forward more messages to the frontend.
	frontendExited = true;
	// paradyn is gone so we go too.
	cleanUpAndExit(-1);
      }
#else // !defined(i386_unknown_nt4_0)
         
      // WinSock indicates the socket closed as a read event.  When
      // reading on the socket, the number of bytes available is zero.
	 
      if( FD_ISSET( tp->get_sock(), &readSet )) {
         int junk;
         int nbytes = recv(tp->get_sock(), (char*)&junk, sizeof(junk),
                           MSG_PEEK );
         if( nbytes == 0 ) {
	   // No more messages to Daddy
	   frontendExited = true;
            // paradyn is gone so we go too
            cleanUpAndExit(-1);
         }
      }
#endif // !defined(i386_unknown_nt4_0)
	 
      bool delayIGENrequests=false;

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
   }
}

static void createResource(int pid, traceHeader *header, struct _newresource *r)
{
   char *tmp;
   char *name;
   // resource *res;
   pdvector<pdstring> parent_name;
   resource *parent = NULL;
   unsigned mdlType;
   //cerr << "in createResource pid: " << pid << endl;
   switch (r->mdlType) {
     case RES_TYPE_STRING: mdlType = MDL_T_STRING; break;
     case RES_TYPE_INT:    mdlType = MDL_T_INT; break;
     default: 
        pdstring msg = pdstring("Invalid resource type reported on trace stream from PID=")
           + pdstring(pid);
        showErrorCallback(36,msg);
        cerr << "cr - ret A\n";
        return;
   }

   name = r->name;
   ResourceType type = (ResourceType)r->btype;
   //cerr << "cr - a, name: " << name << endl;
   do {
      tmp = strchr(name, '/');
      //cerr << "  tmp at " << tmp << endl;
      if (tmp) {
         *tmp = '\0';
         tmp++;
         parent_name += name;
         name = tmp;
      }
   } while (tmp);
   //cerr << "cr - b\n";
   timeStamp trWall(timeStamp::ts1970());
   trWall = getWallTimeMgr().units2timeStamp(header->wall);
   //cerr << "cr - c\n";
   if ((parent = resource::findResource(parent_name)) && name != r->name) {
      resource::newResource(parent, NULL, r->abstraction, name,
                            trWall, "", 
                            type,
                            mdlType,
                            true);
   }
   else {
      pdstring msg = pdstring("Unknown resource '") + pdstring(r->name) +
         pdstring("' reported on trace stream from PID=") +
		   pdstring(pid);
      showErrorCallback(36,msg);
   }
   //cerr << "cr - return normal\n";
}

static void updateResource(int pid, traceHeader *header,struct _updtresource *r)
{
   char *tmp;
   char *name;
   char *displayname;
   resource *res = NULL;
   pdvector<pdstring> the_name;
   pdvector<pdstring> disp_name;
   int retired = 0;
   unsigned type;

   //cerr << "in updateResource pid: " << pid << endl;
   switch (r->mdlType) {
     case RES_TYPE_STRING: type = MDL_T_STRING; break;
     case RES_TYPE_INT:    type = MDL_T_INT; break;
     default:
        pdstring msg = pdstring("Invalid resource type reported on trace stream from PID=")
           + pdstring(pid);
        showErrorCallback(36,msg);
        cerr << "cr - ret A\n";
        return;
   }
   name = r->name;
   displayname = r->displayname;
   retired = r->retired;
   //cerr << "cr - a, name: " << name << endl;
   //cerr << "cr - a, displayname: " << displayname << endl;

/* make pdvectors of the strings because they're easier to work with*/
  do {
      tmp = strchr(name, '/');
      //cerr << "  tmp at " << tmp << endl;
      if (tmp) {
         *tmp = '\0';
         tmp++;
         the_name += name;
        name = tmp;
     }
  } while (tmp);
   the_name += name;
   do {
      tmp = strchr(displayname, '/');
      //cerr << "  tmp at " << tmp << endl;
      if (tmp) {
         *tmp = '\0';
         tmp++;
         disp_name += displayname;
         displayname = tmp;
      }
   } while (tmp);
   disp_name += displayname;
   if ((res = resource::findResource(the_name))) {
      resource::updateResource(res, r->abstraction, the_name,
                             (ResourceType*)&type, disp_name, retired);
   }
   else {
      pdstring msg = pdstring("Unknown resource '") + pdstring(r->name) +
         pdstring("' reported on trace stream from PID=") +
                   pdstring(pid);
      showErrorCallback(36,msg);
   }
   //for(int i = 0; i < the_name.size(); ++i)
      //cerr<<"the_name["<<i<<"]="<<the_name[i]<< " ";
   //cerr << "cr - return normal\n";
}

void printAppStats(struct endStatsRec *stats)
{
  if (stats) {
    logStream << "    DYNINSTtotalAlaramExpires: " << stats->alarms <<"\n";
#ifdef notdef
    logStream << "    DYNINSTnumReported: " << stats->numReported << "\n";
#endif
    logStream << "    Raw cycle count: " << (int64_t) stats->instCycles << "\n";

    timeUnit cps = getCyclesPerSecond();
    logStream << "    Cycle rate: " << cps << " units/nanoseconds" << "\n";

    timeLength instTime(stats->instCycles, cps);
    logStream << "    Total instrumentation cost: " << instTime << "\n";
    // variable only defined if using profiler, see RTinst.C
    //logStream << "    Total inst (via prof): " << stats->instTicks << "\n";

    timeLength cpuTime(stats->totalCpuTime, cps);

    logStream << "    Total cpu time of program: " << cpuTime << "\n";

    // variable only defined if using profiler, see RTinst.C
    //logStream << "    Total cpu time (via prof): ",stats->userTicks);

    timeLength wallTime(stats->totalWallTime, cps);
    logStream << "    Total wall time of program: " << wallTime << "\n";

    logStream << "    Total data samples: " << stats->samplesReported << "\n";
#if defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4)
    logStream <<  "    Total traps hit: " << stats->totalTraps << "\n";
#endif
    sprintf(errorLine, "    %d metrics used\n", metricsUsed);
    logLine(errorLine);
    sprintf(errorLine, "    %d foci used\n", fociUsed);
    logLine(errorLine);
    sprintf(errorLine, "    %d samples delivered\n", samplesDelivered);
    logLine(errorLine);
  }
}

void printDyninstStats(BPatch_stats &st)
{
    sprintf(errorLine, "    %d total points used\n", st.pointsUsed);
    logLine(errorLine);
    sprintf(errorLine, "    %d mini-tramps used\n", st.totalMiniTramps);
    logLine(errorLine);
    sprintf(errorLine, "    %d tramp bytes\n", st.trampBytes);
    logLine(errorLine);
    sprintf(errorLine, "    %d ptrace other calls\n", st.ptraceOtherOps);
    logLine(errorLine);
    sprintf(errorLine, "    %d ptrace write calls\n", st.ptraceOps-st.ptraceOtherOps);
    logLine(errorLine);
    sprintf(errorLine, "    %d ptrace bytes written\n", st.ptraceBytes);
    logLine(errorLine);
    sprintf(errorLine, "    %d instructions generated\n", st.insnGenerated);
    logLine(errorLine);
}
