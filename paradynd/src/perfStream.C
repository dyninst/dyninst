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

// $Id: perfStream.C,v 1.183 2005/11/03 05:21:08 jaw Exp $

#include "common/h/headers.h"
#include "common/h/timing.h"
#include "common/h/debugOstream.h"
#include "common/h/int64iostream.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "paradynd/src/machineMetFocusNode.h"
#include "paradynd/src/comm.h"
#include "paradynd/src/main.h"
#include "paradynd/src/init.h"
#include "paradynd/src/context.h"
#include "paradynd/src/perfStream.h"
#include "paradynd/src/dynrpc.h"
#include "paradynd/src/mdld.h"
#include "paradynd/src/main.h"
#include "paradynd/src/pd_module.h"
#include "paradynd/src/pd_image.h"
#include "paradynd/src/processMgr.h"
#include "paradynd/src/pd_process.h"
#include "paradynd/src/debug.h"
#include "pdutil/h/airtStreambuf.h"
#include "pdutil/h/mdl_data.h"
#include "pdutil/h/pdDebugOstream.h"

#include "dyninstAPI/src/mapped_object.h"

// trace data streams
#include "common/h/Dictionary.h"

extern unsigned int metResPairsEnabled; // definied in metricFocusNode.C

//  As far as I can tell these vars are unused except in the
//  functions that print statistics...  kept for legacy purposes
//  (moved here from dyninstAPI/src/stats.C)
//  Unless future use of these is expected, they can probably be
//  deleted
unsigned int fociUsed = 0;
unsigned int metricsUsed = 0;
unsigned int samplesDelivered = 0;

static void createResource(int pid, traceHeader *header, struct _newresource *r);
static void updateResource(int pid, traceHeader *header, struct _updtresource *r);

extern bool isInfProcAttached;

char errorLine[1024];
void logLineN(const char *line, int n, bool /* force */) 
{
   if (frontendExited) {
      fprintf(stderr, "Skipping message, frontend exited:\n%s\n", line);
      return;
   }
   static char fullLine[1024];
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

void logLine(const char *line) {
  logLineN(line, strlen(line), true);
}

void statusLineN(const char *line, int n, bool) 
{
  if (frontendExited) {
    fprintf(stderr, "Skipping status line, frontend exited:\n%s\n", line);
    return;
  }

  static char buff[300];
  if(n>299) n=299;
  strncpy(buff, line, n+1);
  tp->reportStatus(buff);
  
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
void processTraceStream(BPatch_process *p, traceHeader *header, char *msg)
{
   pd_process *pd_p = getProcMgr().find_pd_process(p->getPid());

   switch (header->type) 
   {
      case TR_NEW_RESOURCE:
      {
         createResource(p->getPid(), header, (struct _newresource *) msg);
         break;
      }
      case TR_UPDATE_RESOURCE:
      {
         updateResource(p->getPid(), header,(struct _updtresource *) msg);
         break;
      }
      case TR_EXEC_FAILED:
      { 
         assert(0);
         break;
      }     
      case TR_DYNAMIC_CALLEE_FOUND:
      {
#if 0
         process *dproc
         int_function *caller, *callee;
         resource *caller_res, *callee_res;
         callercalleeStruct *c = (struct callercalleeStruct *) msg;
         
         
         //cerr << "DYNAMIC trace record received!!, caller = " << hex 
         //   << c->caller << " callee = " << c->callee << dec << endl;
         // Have to look in main image and (possibly) in shared objects
         codeRange *range;
         range = dproc->findCodeRangeByAddress(c->caller);
         caller = range->is_function();
         
         range = dproc->findCodeRangeByAddress(c->callee);
         callee = range->is_function();
         
         BPatch_process *bproc = pd_p->get_bprocess();
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
#endif
      }
      case TR_DATA:
      {
         extern void batchTraceData(int, int, int, char *);
         batchTraceData(0, p->getPid(), header->length, msg);
         traceOn[p->getPid()] = 1;
         break;
      }
      case TR_ERROR: // also used for warnings
      {
         rtUIMsg *rtMsgPtr;
         rtMsgPtr = (rtUIMsg *) msg;
         showErrorCallback(rtMsgPtr->errorNum, rtMsgPtr->msgString);
         if(rtMsgPtr->msgType == rtError) {
            // if need this, might want to add code
            // to shut down paradyn and/or app
         }
         break;
      }
      default:
      {
         sprintf(errorLine, "Got unknown record type %d\n", 
                 header->type);
         logLine(errorLine);
         sprintf(errorLine, "Received bad trace data from process %d.", 
                 pd_p->getPid());
         showErrorCallback(37,(const char *) errorLine);
      }
   }
}

void recvUserEvent(BPatch_process *p, void *buffer, unsigned size)
{
   traceHeader *header = (traceHeader *) buffer;
   char *msg = (char *) (header + 1); //The message follows the trace header
   processTraceStream(p, header, msg);
}

void DyninstRTMessageCB(BPatch_process *p, void *msg, unsigned msg_size)
{
   static traceHeader *header = NULL;
   static BPatch_process *last_proc = NULL;
   if (!header)
   {
      header = (traceHeader *) header;
      last_proc = p;
      assert(msg_size == sizeof(traceHeader));
      return;
   }

   assert(last_proc == p);
   processTraceStream(p, header, (char *) msg);
   header = NULL;
   last_proc = NULL;
}

extern pdvector<int> deferredMetricIDs;

static void doDeferredRPCs() {
    processMgr::procIter itr = getProcMgr().begin();
    while(itr != getProcMgr().end()) {
        pd_process *theProc = *itr;
        itr++;
        if (!theProc) continue;

        if (theProc->isTerminated()) continue;
        theProc->launchRPCs(!theProc->isStopped());
    }
}


static void doDeferredInstrumentation() {
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
#ifdef NOTDEF // PDSEP

      // we have moved this code at the beginning of the loop, so we will
      // process signals before igen requests: this is to avoid problems when
      // an inferiorRPC is waiting for a system call to complete and an igen
      // requests arrives at that moment - naim
      if( isInfProcAttached )
      {
         getBPatch().pollForStatusChange();
      } 
#endif
     
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

#if !defined(os_windows)
      if (FD_ISSET(tp->get_sock(), &errorSet)) {
         // Don't forward more messages to the frontend.
         frontendExited = true;
         // paradyn is gone so we go too.
         cleanUpAndExit(-1);
      }
#else          
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
#endif 
	 
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

static void updateResource(int pid, traceHeader *, struct _updtresource *r)
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
