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
 * File containing lots of dynRPC function definitions for the paradynd..
 *
 * $Log: dynrpc.C,v $
 * Revision 1.51  1996/08/16 21:18:31  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.50  1996/08/12 16:27:18  mjrg
 * Code cleanup: removed cm5 kludges and some unused code
 *
 * Revision 1.49  1996/05/15 18:32:42  naim
 * Fixing bug in inferiorMalloc and adding some debugging information - naim
 *
 * Revision 1.48  1996/05/10  13:52:58  naim
 * Inserting temporal timing information collection - naim
 *
 * Revision 1.47  1996/05/08  17:04:14  tamches
 * added comments regarding how we are kludging the internal metric bucket_width
 * for now
 *
 * Revision 1.46  1996/05/01 18:07:21  newhall
 * added parameter to predicted cost call
 *
 * Revision 1.45  1996/04/30  18:58:38  newhall
 * changes to enableDataCollection calls and upcalls
 *
 */

#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "ast.h"
#include "util.h"
#include "dyninstP.h"
#include "metric.h"
#include "internalMetrics.h"
#include "dyninstRPC.xdr.SRVR.h"
#include "dyninst.h"
#include "stats.h"
#include "resource.h"
#include "paradynd/src/mdld.h"
#include "paradynd/src/init.h"
#include "paradynd/src/costmetrics.h"
#include "showerror.h"
#include "util/h/sys.h" 

#define ONEMILLION 1000000
// default to once a second.
float samplingRate = 1.0;
float currSamplingRate = BASEBUCKETWIDTH;

#ifdef TIMINGDEBUG
#define EVERY 100
#endif

void dynRPC::printStats(void)
{
  printDyninstStats();
}

// TODO -- use a different creation time
void dynRPC::addResource(u_int parent_id, u_int id, string name, u_int type)
{
  resource *parent = resource::findResource(parent_id);
  if (!parent) return;
  resource::newResource(parent, name, id, type);
}

void dynRPC::coreProcess(int id)
{
  process *proc = findProcess(id);
  if (proc)
    proc->dumpCore("core.out");
}

string dynRPC::getStatus(int id)
{
  process *proc = findProcess(id);
  if (!proc) {
    string ret = string("PID: ") + string(id);
    ret += string(" not found for getStatus\n");
    return (P_strdup(ret.string_of()));
  } else 
    return (proc->getProcessStatus());
}

vector<T_dyninstRPC::metricInfo> dynRPC::getAvailableMetrics(void) {
  vector<T_dyninstRPC::metricInfo> metInfo;
  unsigned size = internalMetric::allInternalMetrics.size();
  for (unsigned u=0; u<size; u++)
    metInfo += internalMetric::allInternalMetrics[u]->getInfo();
  for (unsigned u2=0; u2< costMetric::allCostMetrics.size(); u2++)
    metInfo += costMetric::allCostMetrics[u2]->getInfo();
  mdl_get_info(metInfo);
  return(metInfo);
}

void dynRPC::getPredictedDataCost(u_int id,
				  u_int req_id,
				  vector<u_int> focus, 
				  string metName,
				  u_int clientID)
{
    if (!metName.length()) 
      getPredictedDataCostCallback(id, req_id, 0.0,clientID);
    else{

#ifdef TIMINGDEBUG
  timeStamp t1,t2,current;
  static timeStamp total=0.0;
  static int counter=0;
  static timeStamp worst=0.0;
  t1=getCurrentTime(false);
#endif

      float cost = guessCost(metName, focus);

#ifdef TIMINGDEBUG
  t2=getCurrentTime(false);
  current=t2-t1;
  if (current > worst) worst=current;
  total += current;
  counter++;
  if (!(counter%EVERY)) {
    sprintf(errorLine,"************* TIMING getPredictedDataCost: current=%5.2f, avg=%5.2f, worst=%5.2f\n",current,total/counter,worst);
    logLine(errorLine);
  }
#endif

      getPredictedDataCostCallback(id, req_id, cost,clientID);
    }
}

void dynRPC::disableDataCollection(int mid)
{
    float cost;
    metricDefinitionNode *mi;

#ifdef TIMINGDEBUG
  timeStamp t1,t2,current;
  static timeStamp total=0.0;
  static int counter=0;
  static timeStamp worst=0.0;
  t1=getCurrentTime(false);
#endif

    if (!allMIs.defines(mid)) {
      // sprintf(errorLine, "Internal error: disableDataCollection mid %d not found\n", mid);
      // logLine(errorLine);
      // showErrorCallback(61,(const char *) errorLine);
      // because of async enables this can happen, so ignore it
      return;
    }

    mi = allMIs[mid];
    // cout << "disable of " << mi->getFullName() << endl; 

    cost = mi->originalCost();
    
    if(cost > currentPredictedCost)
	currentPredictedCost = 0.0;
    else 
        currentPredictedCost -= cost;

    mi->disable();
    allMIs.undef(mid);
    delete(mi);

#ifdef TIMINGDEBUG
  t2=getCurrentTime(false);
  current=t2-t1;
  if (current > worst) worst=current;
  total += current;
  counter++;
  if (!(counter%EVERY)) {
    sprintf(errorLine,"*********** TIMING disableDataCollection: current=%5.2f, avg=%5.2f, worst=%5.2f\n",current,total/counter,worst);
    logLine(errorLine);
  }
#endif

}

bool dynRPC::setTracking(unsigned target, bool mode)
{
    resource *res = resource::findResource(target);
    if (res) {
	if (res->isResourceDescendent(moduleRoot)) {
	    image::changeLibFlag(res, (bool) mode);
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

void dynRPC::resourceInfoResponse(vector<string> resource_name, u_int resource_id) {
  resource *res = resource::findResource(resource_name);
  if (res)
    res->set_id(resource_id);
}

// TODO -- startCollecting  Returns -1 on failure ?
void dynRPC::enableDataCollection(vector<T_dyninstRPC::focusStruct> focus, 
                              vector<string> metric,
			      vector<u_int> mi_ids, 
		 	      u_int daemon_id,
			      u_int request_id){
#ifdef TIMINGDEBUG
  timeStamp t1,t2,current;
  static timeStamp total=0.0;
  static int counter=0;
  static int anotherCounter=0;
  static timeStamp worst=0.0;
  static string metricName;
  t1=getCurrentTime(false);
#endif

    vector<int> return_id;
    assert(focus.size() == metric.size());
    return_id.resize(metric.size());
    totalInstTime.start();
    for (u_int i=0;i<metric.size();i++) {
#ifdef TIMINGDEBUG
  t1=getCurrentTime(false);
#endif
        return_id[i] = startCollecting(metric[i], focus[i].focus, mi_ids[i]);
#ifdef TIMINGDEBUG
  t2=getCurrentTime(false);
  current=t2-t1;
  if (current > worst) {
    worst=current;
    metricName=metric[i];
  }
  total += current;
  counter++;
#endif
    }
    totalInstTime.stop();

#ifdef TIMINGDEBUG
  if (!(anotherCounter%EVERY)) {
    sprintf(errorLine,"************* TIMING enableDataCollection: current=%5.2f, avg=%5.2f, worst=%5.2f, metric=%s\n",current,total/counter,worst,metricName.string_of());
    logLine(errorLine);
  }
  anotherCounter++;
#endif  

    enableDataCallback(daemon_id,return_id,mi_ids,request_id);
}

int dynRPC::enableDataCollection2(vector<u_int> focus, string met, int gid)
{
  int id;

#ifdef TIMINGDEBUG
  timeStamp t1,t2,current;
  static timeStamp total=0.0;
  static int counter=0;
  static timeStamp worst=0.0;
  t1=getCurrentTime(false);
#endif

  totalInstTime.start();
  id = startCollecting(met, focus, gid);
  totalInstTime.stop();
  // cout << "Enabled " << met << " = " << id << endl;

#ifdef TIMINGDEBUG
  t2=getCurrentTime(false);
  current=t2-t1;
  if (current > worst) worst=current;
  total += current;
  counter++;
  if (!(counter%EVERY)) {
    sprintf(errorLine,"************* TIMING enableDataCollection2: current=%5.2f, avg=%5.2f, worst=%5.2f\n",current,total/counter,worst);
    logLine(errorLine);
  }
#endif

  return(id);
}

//
// computes new sample multiple value, and modifies the value of the
// symbol _DYNINSTsampleMultiple which will affect the frequency with
// which performance data is sent to the paradyn process 
//
#ifdef sparc_tmc_cmost7_3
extern float SAMPLEnodes ;
int SAMPLE_MULTIPLE ;
extern int getNumberOfJobs(void) ;
#endif


void dynRPC::setSampleRate(double sampleInterval)
{
    // TODO: implement this:
    // want to change value of DYNINSTsampleMultiple to corr. to new
    // sampleInterval (sampleInterval % baseSampleInterval) 
    // if the sampleInterval is less than the BASESAMPLEINTERVAL ignore
    // use currSamplingRate to determine if the change to DYNINSTsampleMultiple
    // needs to be made

    // TODO:
    // Update the value of bucket_width, an internal metric.
    // (Code used to be here to update the value, but it wasn't quite enough.
    //  In particular, if there were no enabled instances of bucket_width, then
    //  no updating would be done; thus the update could get lost.
    //  Example: put up table with active_processes; run program 5 seconds;
    //           then add bucket_width to the table.  The bucket width will be 0
    //           because the routine that updated bucket width (i.e. right here)
    //           was called only after active_processes was added; not after
    //           bucket_width was added.
    // In metric.C, we work around the problem by putting in a kludge for reporting
    // the internal metric bucket_width; we simply ignore the value stored in the
    // internalMetrics class and instead return the exterm float "sampleInterval" --ari
    //
    // We keep the following code because it's harmless; but remember, it's also
    // not really being used at this time:
    if (bucket_width->num_enabled_instances() > 0)
       bucket_width->getEnabledInstance(0).setValue(sampleInterval);

    if(sampleInterval != currSamplingRate){
         int *sample_multiple = new int; 
	*sample_multiple = 
	    (int)(((sampleInterval)*ONEMILLION)/BASESAMPLEINTERVAL);
         
#ifdef sparc_tmc_cmost7_3 
	int number_of_jobs = getNumberOfJobs() ;
	sprintf(errorLine, "FOLD, sample_multiple=%d\n", *sample_multiple) ;
	SAMPLE_MULTIPLE = *sample_multiple ;
	// adjust the snarf rate accordingly
	SAMPLEnodes = ((float) BASEBUCKETWIDTH)* 
		      ((float)(SAMPLE_MULTIPLE))*
		      number_of_jobs ;

	if(SAMPLEnodes < 1.0) 
		SAMPLEnodes =  1.0 ;
#endif
	// setSampleMultiple(sample_multiple);
	// set the sample multiple in all processes
	unsigned p_size = processVec.size();
	for (unsigned u=0; u<p_size; u++){
	  if (processVec[u]->status() != exited) {
            internalSym *ret_sym = 0; 
            if(!(ret_sym = processVec[u]->symbols->findInternalSymbol(
				"DYNINSTsampleMultiple",true))){
                sprintf(errorLine, "error2 in dynRPC::setSampleRate\n");
                logLine(errorLine);
		P_abort();
	    }
	    Address addr = ret_sym->getAddr();
            processVec[u]->writeDataSpace((caddr_t)addr,sizeof(int),
					  (caddr_t)sample_multiple);
	  }
        }
        currSamplingRate = sampleInterval;
    }
    return;
}

bool dynRPC::detachProgram(int program, bool pause)
{
  process *proc = findProcess(program);
  if (proc)
    return(proc->detach(pause));
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
void dynRPC::continueProgram(int program)
{
    process *proc = findProcess(program);
    if (!proc) {
      sprintf(errorLine, "Internal error: cannot continue PID %d\n", program);
      logLine(errorLine);
      showErrorCallback(62,(const char *) errorLine);
    }
    proc->continueProc();
    statusLine("application running");
}

//
//  Stop all processes 
//
bool dynRPC::pauseApplication(void)
{
    pauseAllProcesses();
    return true;
}

//
//  Stop a single process
//
bool dynRPC::pauseProgram(int program)
{
    process *proc = findProcess(program);
    if (!proc) {
      sprintf(errorLine, "Internal error: cannot pause PID %d\n", program);
      logLine(errorLine);
      showErrorCallback(63,(const char *) errorLine);
      return false;
    }
    return (proc->pause());
}

bool dynRPC::startProgram(int program)
{
    statusLine("starting application");
    continueAllProcesses();
    return(false);
}

//
// This is not implemented yet.
//
bool dynRPC::attachProgram(int id)
{
    return(false);
}

//
// start a new program for the tool.
//
int dynRPC::addExecutable(vector<string> argv, string dir)
{
  vector<string> envp;
  return(addProcess(argv, envp, dir));
}


//
// report the current time 
//
double dynRPC::getTime() {
  return getCurrentTime(false);
}

