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

/* $Id: dynrpc.C,v 1.120 2005/03/07 21:18:53 bernat Exp $ */

#include "paradynd/src/metricFocusNode.h"
#include "paradynd/src/machineMetFocusNode.h"
#include "paradynd/src/processMetFocusNode.h"
#include "paradynd/src/internalMetrics.h"
#include "dyninstRPC.xdr.SRVR.h"
#include "paradynd/src/resource.h"
#include "paradynd/src/mdld.h"
#include "paradynd/src/init.h"
#include "paradynd/src/costmetrics.h"
#include "paradynd/src/context.h"
#include "common/h/debugOstream.h"
#include "pdutil/h/hist.h"
#include "paradynd/src/pd_process.h"
#include "paradynd/src/processMgr.h"
#include "pdutil/h/mdlParse.h"
#include "paradynd/src/mdld_data.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/stats.h"

int StartOrAttach( void );
extern bool startOnReportSelfDone;
extern pdstring pd_flavor;

timer totalInstTime;

timeLength *imetricSamplingRate = NULL;
timeLength *currSamplingRate = NULL;

const timeLength &getIMetricSamplingRate() {
  if(imetricSamplingRate == NULL) {
    // default to once a second.
    imetricSamplingRate = new timeLength(timeLength::sec());
  }
  return *imetricSamplingRate;
}

void setCurrSamplingRate(timeLength tl) {
  if(currSamplingRate == NULL) {
    currSamplingRate = new timeLength(BASEBUCKETWIDTH_SECS, timeUnit::sec());
  }
  *currSamplingRate = tl;
}

const timeLength &getCurrSamplingRate() {
  if(currSamplingRate == NULL) {
    currSamplingRate = new timeLength(BASEBUCKETWIDTH_SECS, timeUnit::sec());
  }
  return *currSamplingRate;
}

void dynRPC::printStats(void)
{
  printDyninstStats();
}

void dynRPC::coreProcess(int id)
{
   pd_process *proc = getProcMgr().find_pd_process(id);
   if (proc)
      proc->dumpCore("core.out");
}

pdstring dynRPC::getStatus(int id)
{
   pd_process *proc = getProcMgr().find_pd_process(id);
   if (!proc) {
      pdstring ret = pdstring("PID: ") + pdstring(id);
      ret += pdstring(" not found for getStatus\n");
      return (P_strdup(ret.c_str()));
   } 

   if (proc->isTerminated()) return pdstring("exited");
   if (proc->isDetached()) return pdstring("detached"); // might just want "running" here
   if (proc->isStopped()) return pdstring("stopped"); // maybe example stopSignal here?
   return pdstring("running");
}

pdvector<T_dyninstRPC::metricInfo> dynRPC::getAvailableMetrics(void) {
  pdvector<T_dyninstRPC::metricInfo> metInfo;
  unsigned size = internalMetric::allInternalMetrics.size();
  for (unsigned u=0; u<size; u++)
    metInfo += internalMetric::allInternalMetrics[u]->getInfo();
  for (unsigned u2=0; u2< costMetric::allCostMetrics.size(); u2++)
    metInfo += costMetric::allCostMetrics[u2]->getInfo();
  mdl_get_info(metInfo);
  return(metInfo);
}

void dynRPC::getPredictedDataCost(u_int id, u_int req_id, pdvector<u_int> focus, 
				  pdstring metName, u_int clientID)
{
   if (!metName.length()) 
      getPredictedDataCostCallback(id, req_id, 0.0,clientID);
   else {
      timeLength cost = guessCost(metName, focus);
      // note: returns 0.0 in a variety of situations (if metric cannot be
      //       enabled, etc.)  Would we rather have a more explicit error
      //       return value?
      getPredictedDataCostCallback(id, req_id, 
                     static_cast<float>(cost.getD(timeUnit::sec())), clientID);
   }
}

extern pdvector<int> deferredMetricIDs;

pdvector<int> metricFocusesRequestedForDelete;

void dynRPC::disableDataCollection(int mid)
{
   metricFocusesRequestedForDelete.push_back(mid);
}


void deleteMetricFocus(machineMetFocusNode *mi) {
#if defined(sparc_sun_solaris2_4) && defined(TIMINGDEBUG)
    begin_timing(1);
#endif

    if(mi == NULL) {
        // Already deleted
        return;
    }
    
    timeLength cost = mi->cost();
    
    if(cost > currentPredictedCost)
        setCurrentPredictedCost(timeLength::Zero());
    else 
        subCurrentPredictedCost(cost);

    mi->cancelPendingRPCs();
    // If this MID is on the deferred list, rip it out
    
    pdvector<int>::iterator itr = deferredMetricIDs.end();
    while (itr != deferredMetricIDs.begin()) {
        itr--;
        int defMID = *itr;
        if (defMID == mi->getMetricID()) {
            deferredMetricIDs.erase(itr);
        }
    }

    // This is a good idea -- let the frontend know that instrumentation was
    // removed and all that. But the frontend logic doesn't understand yet.

#ifdef BROKEN
    // Let the frontend know this instrumentation "failed"
    metFocInstResponse *cbi = mi->getMetricFocusResponse();
    if (cbi) {
        cbi->updateResponse(mi->getMetricID(), inst_insert_failure,
                            "Instrumentation cancelled");
        cbi->makeCallback();
    }
#endif
    delete mi;

#if defined(sparc_sun_solaris2_4) && defined(TIMINGDEBUG)
    end_timing(1,"disable");
#endif
}

void processInstrDeletionRequests() {
   pdvector<pd_process *> procsToCont;
   for(unsigned i=0; i<metricFocusesRequestedForDelete.size(); i++) {
      int curmid = metricFocusesRequestedForDelete[i];
      machineMetFocusNode *mi = machineMetFocusNode::lookupMachNode(curmid);
      if (!mi) {
	// Case: the backend has already deleted the node, and now the
	// frontend is duplicating our work.
	continue;
      }
      pdvector<processMetFocusNode*> procnodes = mi->getProcNodes();
      for(unsigned j=0; j<procnodes.size(); j++) {
         processMetFocusNode *cur_procnode = procnodes[j];
         pd_process *proc = cur_procnode->proc();
         proc->pause();
         procsToCont.push_back(proc);
      }
      
      deleteMetricFocus(mi);
   }
   metricFocusesRequestedForDelete.clear();

   for(unsigned k=0; k<procsToCont.size(); k++) {
      pd_process *proc = procsToCont[k];
      fprintf(stderr, "Continuing in instrDeletionRequests\n");
      proc->continueProc();
   }
}

bool dynRPC::setTracking(unsigned target, bool /* mode */)
{
    resource *res = resource::findResource(target);
    if (res) {
	if (res->isResourceDescendent(moduleRoot)) {
	    res->suppress(true);
	    return(true);
	} else {
	    // un-supported resource hierarchy.
	    return(false);
	}
    } else {
      // cout << "Set tracking target " << target << " not found\n";
      return(false);
    }
}

void dynRPC::resourceInfoResponse(pdvector<u_int> temporaryIds, 
			 	  pdvector<u_int> resourceIds) {
    assert(temporaryIds.size() == resourceIds.size());

    for (unsigned u = 0; u < temporaryIds.size(); u++) {
      resource *res = resource::findResource(temporaryIds[u]);
      assert(res);
      res->set_id(resourceIds[u]);
    }
}

void dynRPC::enableDataCollection(pdvector<T_dyninstRPC::focusStruct> focus, 
				  pdvector<pdstring> metric, pdvector<u_int> mi_ids, 
				  u_int daemon_id, u_int request_id) {
   processInstrDeletionRequests();

   assert(focus.size() == metric.size());
   totalInstTime.start();

   metFocInstResponse* cbi = new metFocInstResponse( request_id, daemon_id );

   for (u_int i=0; i<metric.size(); i++) {
        startCollecting( metric[i], focus[i].focus, mi_ids[i], cbi );
   }
   
   totalInstTime.stop();
   cbi->makeCallback();
}

// synchronous, for propogating metrics
T_dyninstRPC::instResponse
dynRPC::enableDataCollection2(pdvector<u_int> focus,
                                pdstring met,
                                int mid,
                                u_int daemon_id )
{
   processInstrDeletionRequests();

   totalInstTime.start();

   metFocInstResponse *cbi = new metFocInstResponse( mid, daemon_id );
   startCollecting( met, focus, mid, cbi );
   totalInstTime.stop();

   return *cbi;
}

//
// computes new sample multiple value, and modifies the value of the
// symbol _DYNINSTsampleMultiple which will affect the frequency with
// which performance data is sent to the paradyn process 
//
void dynRPC::setSampleRate(double sampleInterval)
{
    // TODO: implement this:
    // want to change value of DYNINSTsampleMultiple to corr. to new
    // sampleInterval (sampleInterval % baseSampleInterval) 
    // if the sampleInterval is less than the BASESAMPLEINTERVAL ignore
    // use currSamplingRate to determine if the change to DYNINSTsampleMultiple
    // needs to be made
    timeLength newSampleRate(timeLength(sampleInterval, timeUnit::sec()));

    if(newSampleRate != getCurrSamplingRate()){
      // sample_multiple:  .2 sec  => 1 ;  .4 sec => 2 ;  .8 sec => 4
         int sample_multiple = static_cast<int>(
	   ((newSampleRate.getD(timeUnit::sec())+.01) / BASEBUCKETWIDTH_SECS));

//	 char buffer[200];
//	 sprintf(buffer, "ari fold; sampleInterval=%g so sample_multiple now %d\n",
//		 sampleInterval, *sample_multiple);
//	 logLine(buffer);
         
	// setSampleMultiple(sample_multiple);
	// set the sample multiple in all processes
	 processMgr::procIter itr = getProcMgr().begin();
	 while(itr != getProcMgr().end()) {
	    pd_process *proc = *itr++;
	    if (!proc)
	       continue;
	    if(!proc->isTerminated()) {
              BPatch_image *appImage = proc->get_dyn_process()->getImage();
              assert(appImage);

              const char *vname = "DYNINSTsampleMultiple";
              BPatch_variableExpr *v_dsm = appImage->findVariable(vname);
              if (! v_dsm) {
                fprintf(stderr, "%s[%d]:  could not find var named %s\n", 
                        __FILE__, __LINE__, vname);
                assert(0  && "fatal internal error");
              }
              if (! v_dsm->writeValue((void *) &sample_multiple)) {
                 fprintf(stderr, "%s[%d]:  could not write var named %s\n", 
                         __FILE__, __LINE__, vname);
                 assert(0  && "fatal internal error");
              }
	    }
	 }
	 setCurrSamplingRate(newSampleRate);
	 machineMetFocusNode::updateAllAggInterval(newSampleRate);
    }
    return;
}

bool dynRPC::detachProgram(int program, bool pause)
{
   pd_process *proc = getProcMgr().find_pd_process(program);
   if (proc)
      return(proc->detachProcess(pause));
   else
      return false;
}

//
// Continue all processes
//
void dynRPC::continueApplication(void)
{
    continueAllProcesses();
    statusLine("application running");
}

//
// Continue a process
//
void dynRPC::continueProgram(int pid)
{

  fprintf(stderr, "dynRPC->continueProgram\n");
   pd_process *proc = getProcMgr().find_pd_process(pid);
   if (!proc) {
      if(getProcMgr().hasProcessExited(pid)) {
         // do nothing, a possible and reasonable condition
         // a process forks, the front-end registers the new forked process
         // the forked process exits (eg. if the process was "ssh cmd &")
         // the front-end signals the daemon to continue the forked process
         // however, the forked process is now exited
         return;
      } else {
         sprintf(errorLine,
                 "Internal error: cannot continue PID %d\n", pid);
         logLine(errorLine);
         showErrorCallback(62,(const char *) errorLine,
                           machineResource->part_name());
         return;
      }
   }
   if( proc->isStopped()) {
     if (proc->isTerminated()) {
         sprintf(errorLine,
                 "%s[%d]: Internal error: PID %d terminated\n", 
                 __FILE__, __LINE__, pid);
         logLine(errorLine);
         showErrorCallback(62,(const char *) errorLine,
                           machineResource->part_name());

     }
     else
       proc->continueProc();
   }
   // we are no longer paused, are we?
   statusLine("application running");
   if (!markApplicationRunning()) {
       return;
   }
}

//
//  Stop all processes 
//
bool dynRPC::pauseApplication(void)
{
  fprintf(stderr, "dynRPC::pauseApp\n");
    pauseAllProcesses();
    return true;
}
 
//
//  Stop a single process
//
bool dynRPC::pauseProgram(int program)
{
  fprintf(stderr, "dynRPC::pauseProg\n");
   pd_process *proc = getProcMgr().find_pd_process(program);
   if (!proc) {
      sprintf(errorLine, "Internal error: cannot pause PID %d\n", program);
      logLine(errorLine);
      showErrorCallback(63,(const char *) errorLine,
		        machineResource->part_name());
      return false;
   }
   return proc->pause();
}

bool dynRPC::startProgram(int )
{
  fprintf(stderr, "dynRPC::startProgram\n");
    statusLine("starting application");
    continueAllProcesses();
    return(false);
}

//
// Monitor the dynamic call sites contained in function <function_name>
//
void dynRPC::MonitorDynamicCallSites(pdstring function_name){
  processMgr::procIter itr = getProcMgr().begin();
  while(itr != getProcMgr().end()) {
     pd_process *p = *itr++;
     if (!p)
       continue;
     p->MonitorDynamicCallSites(function_name);
  }
}

//
// start a new program for the tool.
//
int dynRPC::addExecutable(pdvector<pdstring> argv, pdstring dir, 
                          bool parse_loops)
{
  pd_process *p = pd_createProcess(argv, dir, parse_loops);
  metricFocusNode::handleNewProcess(p);
  if (p) {
    return 1;
  }
  else
    return -1;
}

//
// Attach is the other way to start a process (application?)
// path gives the full path name to the executable, used _only_ to read
// the symbol table off disk.
// values for 'afterAttach': 1 --> pause, 2 --> run, 0 --> leave as is
//
bool dynRPC::attach(pdstring progpath, int pid, int afterAttach, 
                    bool parse_loops)
{
  startup_cerr << "WELCOME to dynRPC::attach" << endl;
  startup_cerr << "progpath=" << progpath << endl;
  startup_cerr << "pid=" << pid << endl;
  startup_cerr << "afterAttach=" << afterAttach << endl;

    pd_process *p = pd_attachProcess(progpath, pid, parse_loops);
    if (!p) return false;
    
    metricFocusNode::handleNewProcess(p);

    return true;
}


//
// report the current time 
//
double dynRPC::getTime() {
  return getWallTime().getD(timeUnit::sec(), timeBase::bStd());
}


void
dynRPC::reportSelfDone( void )
{
    if( startOnReportSelfDone )
    {
        // The front-end is done handling our reportSelf request
        // We can start our process now
        StartOrAttach();
    }
}


void 
dynRPC::send_mdl( pdvector<T_dyninstRPC::rawMDL> /*mdlBufs*/ )
{
    // should never be called; pdRPC::send_mdl should be called instead.
    assert( false );
}

void
pdRPC::send_mdl( pdvector<T_dyninstRPC::rawMDL> mdlBufs )
{
    assert( !saw_mdl );

    // parse the MDL data we've been given
    for( pdvector<T_dyninstRPC::rawMDL>::const_iterator iter = mdlBufs.begin();
        iter != mdlBufs.end();
        iter++ )
    {
        mdlBufPtr = (const char*)iter->buf.c_str();
        mdlBufRemaining = iter->buf.length();

        int pret = mdlparse();
        if( pret != 0 )
        {
            // indicate the error
#if READY
            // how to indicate error?
#else
            cerr << "failed to parse MDL"
                << endl;
#endif // READY

            break;
        }
    }

    // we've now parsed all MDL data -
    // process it as the front-end processed it
    if( !mdl_apply() )
    {
        // indicate the error
#if READY
        // how to indicate the error?
#else
        cerr << "failed to apply MDL" << endl;
#endif // READY
    }
    else if( !mdl_check_node_constraints() )
    {
        // indicate the error
#if READY
        // how to indicate the error?
#else
        cerr << "MDL node constraint check failed" << endl;
#endif // READY
    }

    // now we've done just what the front-end had done before sending us
    // the raw MDL.
    // 
    // now we process the MDL within our own context
    mdl_data* fe_context = mdl_data::cur_mdl_data;
    mdl_data::cur_mdl_data = new mdld_data();
    mdl_init_be( pd_flavor );

    send_stmts( &(fe_context->stmts) );
    send_constraints( &(fe_context->all_constraints) );
    send_metrics( &(fe_context->all_metrics) );
    if( fe_context->lib_constraints.size() > 0 )
    {
        send_libs( &(fe_context->lib_constraints) );
    }
    else
    {
        send_no_libs();
    }

    saw_mdl = true;
}


