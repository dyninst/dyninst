/*
 * Copyright (c) 1996-2002 Barton P. Miller
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

// $Id: DMmain.C,v 1.143 2002/12/20 07:50:01 jaw Exp $

#include <assert.h>
extern "C" {
#include <malloc.h>
#include <stdio.h>
}

#include "paradyn/src/pdMain/paradyn.h"
#include "pdthread/h/thread.h"
#include "paradyn/src/TCthread/tunableConst.h"
#include "dataManager.thread.SRVR.h"
#include "dyninstRPC.xdr.CLNT.h"
#include "DMdaemon.h"
#include "DMmetric.h"
#include "DMperfstream.h"
#include "DMabstractions.h"
#include "paradyn/src/UIthread/Status.h"

#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/String.h"
#include "common/h/Time.h"
#include "common/h/timing.h"
// trace data streams
#include "pdutil/h/ByteArray.h"
#include "DMphase.h"

#include "common/h/Ident.h"

#include "CallGraph.h"

extern "C" const char V_paradyn[];
extern const Ident V_id;

#if defined(i386_unknown_nt4_0)
#define	S_ISDIR(m)	(((m)&0xF000) == _S_IFDIR)
#define	S_ISREG(m)	(((m)&0xF000) == _S_IFREG)
#endif // defined(i386_unknown_nt4_0)

typedef pdvector<string> blahType;

// bool parse_metrics(string metric_file);
bool metMain(string &userFile);

// this has to be declared before baseAbstr, cmfAbstr, and rootResource 
PDSOCKET dataManager::sock_desc;  
int dataManager::sock_port;  
int dataManager::termWin_port = -1;
PDSOCKET dataManager::termWin_sock= INVALID_PDSOCKET;
dataManager *dataManager::dm = NULL;  

dictionary_hash<string,abstraction*> abstraction::allAbstractions(string::hash);
abstraction *baseAbstr = new abstraction("BASE");
abstraction *cmfAbstr = new abstraction("CMF");

dictionary_hash<string,metric*> metric::allMetrics(string::hash);
dictionary_hash<metricInstanceHandle,metricInstance *> 
		metricInstance::allMetricInstances(metricInstance::mhash);
dictionary_hash<perfStreamHandle,performanceStream*>  
		performanceStream::allStreams(performanceStream::pshash);
dictionary_hash<string, resource*> resource::allResources(string::hash, 8192);
dictionary_hash<string,resourceList *> resourceList::allFoci(string::hash);

dictionary_hash<unsigned, resource*>resource::resources(uiHash);
pdvector<string> resource::lib_constraints;
pdvector<unsigned> resource::lib_constraint_flags;
pdvector< pdvector<string> > resource::func_constraints;
pdvector<unsigned> resource::func_constraint_flags;
bool resource::func_constraints_built = false;
bool resource::lib_constraints_built = false;

pdvector<metric*> metric::metrics;
pdvector<paradynDaemon*> paradynDaemon::allDaemons;
pdvector<daemonEntry*> paradynDaemon::allEntries;
pdvector<executable*> paradynDaemon::programs;
unsigned paradynDaemon::procRunning;
pdvector<resourceList *> resourceList::foci;
pdvector<phaseInfo *> phaseInfo::dm_phases;
u_int metricInstance::next_id = 1;
// u_int performanceStream::next_id = 0;
pdvector<DM_enableType*> paradynDaemon::outstanding_enables;  
u_int paradynDaemon::next_enable_id = 0;  
u_int paradynDaemon::count = 0;

// to distinguish the enableDataRequest calls only for samples 
// from those for both samples and traces 
// 0 is reserved for non-trace use
u_int performanceStream::next_id = 1;

resource *resource::rootResource = new resource();
timeLength metricInstance::curr_bucket_width = Histogram::getMinBucketWidth();
timeLength metricInstance::global_bucket_width =Histogram::getMinBucketWidth();
phaseHandle metricInstance::curr_phase_id;
u_int metricInstance::num_curr_hists = 0;
u_int metricInstance::num_global_hists = 0;

timeStamp paradynDaemon::earliestStartTime;
void newSampleRate(timeLength rate);

extern void histDataCallBack(pdSample *buckets, relTimeStamp, int count, 
			     int first, void *callbackData);
extern void histFoldCallBack(const timeLength *_width, void *callbackData);

//upcall from paradynd to notify the datamanager that the static
//portion of the call graph is completely filled in.
void dynRPCUser::CallGraphFillDone(string exe_name){
  CallGraph *cg;
  cg = CallGraph::FindCallGraph(exe_name);
  cg->CallGraphFillDone();
}

void dynRPCUser::CallGraphAddDynamicCallSiteCallback(string exe_name, string parent){
  CallGraph *cg;
  resource *r;
  cg = CallGraph::FindCallGraph(exe_name);
  assert(r = resource::string_to_resource(parent));
  cg->AddDynamicCallSite(r);
}


void dynRPCUser::CallGraphAddProgramCallback(string exe_name){
  CallGraph::AddProgram(exe_name);
}

// upcall from paradynd to register the entry function for a given
//  call graph.... 
//This function is called with a previously unseen program ID to create a new
// call graph.
void dynRPCUser::CallGraphSetEntryFuncCallback(string exe_name, 
					       string entry_func, int tid) {
    CallGraph *cg;
    resource *r;

    // get/create call graph corresponding to program....
    cg = CallGraph::FindCallGraph(exe_name);
    assert(cg);

    // resource whose name is passed in <resource> should have been previously
    //  registered w/ data manager....
    assert(r = resource::string_to_resource(entry_func));

    cg->SetEntryFunc(r, tid);
}

// upcall from paradynd to register new function resource with call graph....
// parameters are an integer corresponding to the program to which a node
// is being added, and a string that is the name of the function being added
void dynRPCUser::AddCallGraphNodeCallback(string exe_name, string r_name) {
    CallGraph *cg;
    resource *r;

    // get (or create) call graph corresponding to program....
    cg = CallGraph::FindCallGraph(exe_name);
    assert(cg);

    // resource whose name is passed in <resource> should have been previously
    //  registered w/ data manager....
    assert(r = resource::string_to_resource(r_name));

    cg->AddResource(r);
}

//Same as AddCallGraphNodeCallback, only adds multiple children at once,
//and associates these children with a give parent (r_name)
void dynRPCUser::AddCallGraphStaticChildrenCallback(string exe_name, 
						    string r_name, 
						    pdvector<string>children) {
    unsigned u;
    CallGraph *cg;
    resource *r, *child;
    pdvector <resource *> children_as_resources;

    // call graph for program <program> should have been previously defined....
    cg = CallGraph::FindCallGraph(exe_name);
    assert(cg);
    // resource whose name is passed in <resource> should have been previously
    //  registered w/ data manager....
    r = resource::string_to_resource(r_name);
    assert(r);

    // convert pdvector of resource names to pdvector of resource *'s....
    for(u=0;u<children.size();u++) {
      child = resource::string_to_resource(children[u]);
      assert(child);
      children_as_resources += child;
    }

    cg->SetChildren(r, children_as_resources);
}

void dynRPCUser::AddCallGraphDynamicChildCallback(string exe_name,
						  string parent, string child){
  resource *p, *c;
  CallGraph *cg;
  cg = CallGraph::FindCallGraph(exe_name);
  assert(cg);
  
  p = resource::string_to_resource(parent);
  assert(p);
  c = resource::string_to_resource(child);
  assert(c);
  perfConsult->notifyDynamicChild(p->getHandle(),c->getHandle());
  cg->AddDynamicallyDeterminedChild(p,c);
}





/*should be removed for output redirection
left untouched for paradynd log mesg use
*/
//
// IO from application processes.
//
void dynRPCUser::applicationIO(int,int,string data)
{

    // NOTE: this fixes a purify error with the commented out code (a memory
    // segment error occurs occasionally with the line "cout << rest << endl") 
    // this is problably not the best fix,  but I can't figure out why 
    // the error is occuring (rest is always '\0' terminated when this
    // error occurs)---tn 
    if( data.length() > 0 )
    {
		fprintf(stdout,data.c_str());
		fflush(stdout);
	}
	else
	{
		fprintf( stderr, "paradyn: warning: empty IO string sent from daemon...ignoring..." );
	}

#ifdef n_def
    char *ptr;
    char *rest;
    // extra should really be per process.
    static string extra;

    rest = P_strdup(data.c_str());

    char *tp = rest;
    ptr = P_strchr(rest, '\n');
    while (ptr) {
	*ptr = '\0';
	if (pid) {
	    printf("pid %d:", pid);
	} else {
	    printf("paradynd: ");
	}
	if (extra.length()) {
	    cout << extra;
	    extra = (char*) NULL;
	}
	cout << rest << endl;
	rest = ptr+1;
	if(rest)
	    ptr = P_strchr(rest, '\n');
        else
	    ptr = 0;
    }
    extra += rest;
    delete tp;
    rest = 0;
#endif
}

extern status_line *DMstatus;

void dynRPCUser::resourceBatchMode(bool) // bool onNow
{
   printf("error calling virtual func: dynRPCUser::resourceBatchMode\n");
}

//
// upcalls from remote process.
//
void dynRPCUser::resourceInfoCallback(u_int , pdvector<string> ,
				      string , u_int) {

printf("error calling virtual func: dynRPCUser::resourceInfoCallback\n");

}

//
// upcalls from remote process.
//
void dynRPCUser::retiredResource(string) {
   printf("error calling virtual func: dynRPCUser::retiredResource\n");
}

void dynRPCUser::severalResourceInfoCallback(pdvector<T_dyninstRPC::resourceInfoCallbackStruct>) {
printf("error calling virtual func: dynRPCUser::severalResourceInfoCallback\n");
}

void dynRPCUser::mappingInfoCallback(int,
				     string abstraction, 
				     string type, 
				     string key,
				     string value)
{
  AMnewMapping(abstraction.c_str(),type.c_str(),key.c_str(),
	       value.c_str());    
}

class uniqueName {
  public:
    uniqueName(stringHandle base) { name = base; nextId = 0; }
    int nextId;
    stringHandle name;
};

//
// handles a completed enable response: updates metricInstance state
// and send the calling thread the response 
//
void DMenableResponse(DM_enableType &enable, pdvector<bool> &successful)
{
    pdvector<metricInstance *> &mis = (*enable.request);
    assert(successful.size() == mis.size());
    pdvector<metricInstInfo> *response = new pdvector<metricInstInfo>(mis.size()); 

    // update MI state and response pdvector
    for(u_int i=0; i < mis.size(); i++){
        if(mis[i] && successful[i]){  // this MI could be enabled
	  mis[i]->setEnabled();
	  metric *metricptr = metric::getMetric(mis[i]->getMetricHandle());
	  if(metricptr){

            if(enable.ph_type == CurrentPhase){
		u_int old_current = mis[i]->currUsersCount();
		bool  current_data = mis[i]->isCurrHistogram();
		mis[i]->newCurrDataCollection(histDataCallBack,
				              histFoldCallBack);
	        mis[i]->newGlobalDataCollection(histDataCallBack,
					        histFoldCallBack);
	        mis[i]->addCurrentUser(enable.ps_handle);

                // trace data streams
                mis[i]->newTraceDataCollection(traceDataCallBack);
                mis[i]->addTraceUser(enable.pt_handle);

		// set sample rate to match current phase hist. bucket width
	        if(!metricInstance::numCurrHists()){
	            timeLength rate = phaseInfo::GetLastBucketWidth();
	            newSampleRate(rate);
		}
		// new active curr. histogram added if there are no previous
		// curr. subscribers and either persistent_collection is clear
		// or there was no curr. histogram prior to this
		if((!old_current)
		    && (mis[i]->currUsersCount() == 1) && 
		    (!(mis[i]->isCollectionPersistent()) || (!current_data))){
                    metricInstance::incrNumCurrHists();
		}
		// new global histogram if this metricInstance was just enabled
		if(!((*enable.enabled)[i])){
		    metricInstance::incrNumGlobalHists();
		}
	    }
	    else {  // this is a global phase enable
	        mis[i]->newGlobalDataCollection(histDataCallBack,
					        histFoldCallBack);
	        mis[i]->addGlobalUser(enable.ps_handle);

                // trace data streams
                mis[i]->newTraceDataCollection(traceDataCallBack);
                mis[i]->addTraceUser(enable.pt_handle);

		// if this is first global histogram enabled and there are no
	        // curr hists, then set sample rate to global bucket width
	        if(!metricInstance::numCurrHists()){
	            if(!metricInstance::numGlobalHists()){
	                timeLength rate = Histogram::getGlobalBucketWidth();
	                newSampleRate(rate);
	        }}
		// new global hist added: update count
		if(!((*enable.enabled)[i])){
		    metricInstance::incrNumGlobalHists();
		}
	    }
	    // update response pdvector
	    (*response)[i].successfully_enabled = true;
	    (*response)[i].mi_id = mis[i]->getHandle(); 
	    (*response)[i].m_id = mis[i]->getMetricHandle();
	    (*response)[i].r_id = mis[i]->getFocusHandle();
	    (*response)[i].metric_name = mis[i]->getMetricName();
	    (*response)[i].focus_name = mis[i]->getFocusName();
	    (*response)[i].metric_units = metricptr->getUnits();
	    (*response)[i].units_type = metricptr->getUnitsType();

	    // update the persistence flags: the OR of new & previous values
	    if(enable.persistent_data){
		mis[i]->setPersistentData();
	    }
	    if(enable.persistent_collection){
		mis[i]->setPersistentCollection();
	    }
	    if(enable.phase_persistent_data){
		mis[i]->setPhasePersistentData();
	    }
	  }
	  else {
	      cout << "mis enabled but no metric handle: " 
		   << mis[i]->getMetricHandle() << endl;
	      assert(0);
	  }
	}
	else {  // was not successfully enabled
	    (*response)[i].successfully_enabled = false;
	    (*response)[i].mi_id = mis[i]->getHandle(); 
	    (*response)[i].m_id = mis[i]->getMetricHandle();
	    (*response)[i].r_id = mis[i]->getFocusHandle();
	    (*response)[i].metric_name = mis[i]->getMetricName();
	    (*response)[i].focus_name = mis[i]->getFocusName();
	}

//        if(mis[i]) {
//	    (*response)[i].mi_id = mis[i]->getHandle(); 
//	    (*response)[i].m_id = mis[i]->getMetricHandle();
//	    (*response)[i].r_id = mis[i]->getFocusHandle();
//	    (*response)[i].metric_name = mis[i]->getMetricName();
//	    (*response)[i].focus_name = mis[i]->getFocusName();
//        }
    }

    // make response call
    performanceStream::psIter_t allS = performanceStream::getAllStreamsIter();
    perfStreamHandle h; performanceStream *ps;
    while(allS.next(h,ps)){
	if(h == (perfStreamHandle)(enable.ps_handle)){
	    ps->callDataEnableFunc(response,enable.client_id);
            return;
    } }
    // trace data streams
    allS.reset();
    while(allS.next(h,ps)){
        if(h == (perfStreamHandle)(enable.pt_handle)){
            ps->callDataEnableFunc(response,enable.client_id);
            return;
    } }
    response = 0;
}

//
// handle an enable response from a daemon. If all daemons have responded
// then make response callback to calling thread, and check the outstanding
// enables list to see if this enable response satisfies any waiting requests.
// and enable for an MI is successful if its done entry is true and if its
// MI* is not 0
//
void dynRPCUser::enableDataCallback(u_int daemon_id, 
				    pdvector<int> return_id,
				    pdvector<u_int> mi_ids,
				    u_int request_id)
{
    // find element in outstanding_enables corr. to request_id
    u_int which =0;
    DM_enableType *request_entry = 0;
    for(u_int i=0; i < paradynDaemon::outstanding_enables.size(); i++){
        if((paradynDaemon::outstanding_enables[i])->request_id == request_id){
	    which = i;
	    request_entry = paradynDaemon::outstanding_enables[i];
	    break;
    } }

    if(!request_entry){
	// a request entry can be removed if a new phase event occurs
	// between the enable request and response, so ignore the response
        return;
    }
    assert(daemon_id < paradynDaemon::allDaemons.size());
    paradynDaemon *pd = paradynDaemon::allDaemons[daemon_id];

    // for each mi in request update mi's components with new daemon if
    // it was successfully enabled
    assert(mi_ids.size() == return_id.size());
    for(u_int j=0; j< return_id.size(); j++){
	if(return_id[j] != -1){
            metricInstanceHandle mh =  mi_ids[j];     
	    metricInstance *mi = request_entry->findMI(mh);
	    assert(mi);
	    component *comp = new component(pd,return_id[j], mi);  
	    bool aflag;
	    aflag=(mi->addComponent(comp));
	    assert(aflag);
	    // if at least one daemon could enable, update done and enabled
            request_entry->setDone(mh);
    } }

    // update count of outstanding daemon responses
    assert(request_entry->how_many);
    request_entry->how_many--;

    // all daemons have responded to enable request, send result to caller 
    if(!request_entry->how_many) { 
	pdvector<bool> successful( request_entry->request->size());
	for(u_int k=0; k < request_entry->request->size(); k++){
	    // if MI is 0 or if done is false
	    if(!((*(request_entry->done))[k]) 
		|| !((*(request_entry->request))[k])){
                successful[k] = false;
	    }
	    else {
		successful[k] = true;
            }
	}
        // if all daemons have responded update state for request and send
        // result to caller
	// a successful enable has both the enabled flag set and an mi*

	// clear currentlyEnabling flag and decrement the count of 
	// waiting enables for all MI's
	for(u_int i1=0; i1 < request_entry->done->size(); i1++){
            if((*request_entry->request)[i1]){
	        ((*request_entry->request)[i1])->clearCurrentlyEnabling();
	        if(request_entry->ph_type == CurrentPhase){
		    ((*request_entry->request)[i1])->decrCurrWaiting();
	        }
	        else{
		    ((*request_entry->request)[i1])->decrGlobalWaiting();
	        }
        } }

	// update MI state for this entry and send response to caller
	DMenableResponse(*request_entry, successful);
        

	// remove this entry from the outstanding enables list 
	u_int size = paradynDaemon::outstanding_enables.size();
	paradynDaemon::outstanding_enables[which] =
			 paradynDaemon::outstanding_enables[size-1];
                paradynDaemon::outstanding_enables.resize(size-1);

        // for each element on outstanding_enables, check to see if there are
        // any outstatnding_enables that can be satisfied by this request
        // if so, update state, and for any outstanding_enables that are 
        // complete, send the result to the client thread  
        // update not_all_done 
	for(u_int i2=0; i2 < paradynDaemon::outstanding_enables.size(); i2++){
            DM_enableType *next_entry = paradynDaemon::outstanding_enables[i2];
	    next_entry->updateAny(*(request_entry->request),successful);
	}
	delete request_entry;
	request_entry = 0;

        if(paradynDaemon::outstanding_enables.size()){
	  bool done = false;
	  u_int i3 = 0;
	  while(!done){
	      if((paradynDaemon::outstanding_enables[i3])->not_all_done){
                  i3++;
	      }
	      else {  // this entry's request is complete
		   // update MI state for this entry and send response to caller
 	   	   DM_enableType *temp = paradynDaemon::outstanding_enables[i3];
		   successful.resize(temp->request->size());
	           for(u_int k2=0; k2 < successful.size(); k2++){
	               if(!((*(temp->done))[k2])) successful[k2] = false;
                       else successful[k2] = true;
	           }
		   // decrement the number of waiting for enables for 
		   // each MI in this response
		   for(u_int k3=0; k3 < temp->request->size(); k3++){
                       if((*temp->request)[k3]){
	                   if(temp->ph_type == CurrentPhase){
		               ((*temp->request)[k3])->decrCurrWaiting();
	                   }
	                   else{
		              ((*temp->request)[k3])->decrGlobalWaiting();
	                   }
		   } }

	           DMenableResponse(*temp, successful);

		   // remove entry from outstanding_enables list
		   u_int newsize=paradynDaemon::outstanding_enables.size()-1;
		   paradynDaemon::outstanding_enables[i3] =
			 paradynDaemon::outstanding_enables[newsize];
                   paradynDaemon::outstanding_enables.resize(newsize);
                   delete temp;
	      }
	      if(i3 >= paradynDaemon::outstanding_enables.size()) done = true;
	  }
	}
    }
}

//
// Upcall from daemon in response to getPredictedDataCost call
// id - perfStreamHandle assoc. with the call
// req_id - an identifier assoc. with the request 
// val - the cost of enabling the metric/focus pair
//
void dynRPCUser::getPredictedDataCostCallback(u_int id,
					      u_int req_id,
					      float val,
					      u_int clientID)
{
    // find the assoc. perfStream and update it's pred data cost value
    performanceStream::psIter_t allS = performanceStream::getAllStreamsIter();
    perfStreamHandle h; performanceStream *ps;
    while(allS.next(h,ps)){
	if(h == (perfStreamHandle)id){
            ps->predictedDataCostCallback(req_id,val,clientID);
	    return;
    } }
    // TODO: call correct routine
    assert(0);
}

//
// Display errors using showError function from the UIM class
// This function allows to display error messages from paradynd
// using the "upcall" or "call back" mechanism.
// Parameters: 	errCode = Error code
//		errString = Error message
//		hostName = Host name where the error occur
// Call: there is a macro defined in "showerror.h". This macro must be
//       used when calling this function. A typical call is:
//       showErrorCallback(99, "Erro message test"). This macro will
//       automatically insert the additional host info required.
//
void dynRPCUser::showErrorCallback(int errCode, 
				   string errString,
				   string hostName)
{
    string msg;

    if (errString.length() > 0) {
	if (hostName.length() > 0) {
    	    msg = string("<Msg from daemon on host ") + hostName + 
	          string("> ") + errString;
        }
	else { 
	    msg = string("<Msg from daemon on host ?> ") + errString; 
        }
        uiMgr->showError(errCode, P_strdup(msg.c_str()));
    }
    else {
        uiMgr->showError(errCode, ""); 
    }

    //
    // hostName.length() should always be > 0, otherwise
    // hostName is not defined (i.e. "?" will be used instead).
    // if errString.length()==0, (i.e. errString.c_str()==""),
    // then we will use the default error message in errorList.tcl
    // This message, however, will not include any info about the current
    // host name.
    //
}

//
// Paradynd calls this igen fn when it starts a new process (more
// specifically, after it starts the new process and the new process
// has completed running DYNINSTinit).
//
void dynRPCUser::newProgramCallbackFunc(int pid,
					pdvector<string> argvString,
					string machine_name,
					bool calledFromExec,
					bool runMe)
{
    // there better be a paradynd running on this machine!
    paradynDaemon *last_match = 0;

    for (unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++) {
        paradynDaemon *pd = paradynDaemon::allDaemons[i];
	if (pd->machine.length() && (pd->machine == machine_name)) {
	    last_match = pd;
	}
    }
    if (last_match != 0) {
        if (!paradynDaemon::addRunningProgram(pid, argvString, last_match,
					      calledFromExec, runMe)) {
	    assert(false);
	}
	uiMgr->enablePauseOrRun();
    }
    else {
        // for now, abort if there is no paradynd, this should not happen
        fprintf(stderr, "process started on %s, can't find paradynd "
		"there\n", machine_name.c_str());
	fprintf(stderr,"paradyn error #1 encountered\n");
	//exit(-1);
    }
}

void dynRPCUser::newMetricCallback(T_dyninstRPC::metricInfo info)
{
    addMetric(info);
}

void dynRPCUser::setDaemonStartTime(int, double) 
{
  assert(0 && "Invalid virtual function");
}

void dynRPCUser::setInitialActualValueFE(int, double) 
{
  assert(0 && "Invalid virtual function");
}

void dynRPCUser::cpDataCallbackFunc(int,double,int,double,double)
{
    assert(0 && "Invalid virtual function");
}

// batch the sample delivery
void dynRPCUser::batchSampleDataCallbackFunc(int,
		    pdvector<T_dyninstRPC::batch_buffer_entry>)
{
    assert(0 && "Invalid virtual function");
}

// batch the trace delivery
void dynRPCUser::batchTraceDataCallbackFunc(int,
                    pdvector<T_dyninstRPC::trace_batch_buffer_entry>)
{
    assert(0 && "Invalid virtual function");
}

//
// When a paradynd is started remotely, ie not by paradyn, this upcall
// reports the information for that paradynd to paradyn
//
void 
dynRPCUser::reportSelf (string , string , int , string)
{
  assert(0);
  return;
}

void 
dynRPCUser::reportStatus (string)
{
    assert(0 && "Invalid virtual function");
}

void
dynRPCUser::processStatus(int, u_int)
{
    assert(0 && "Invalid virtual function");
}

void
dynRPCUser::endOfDataCollection(int)
{
  assert(0 && "Invalid virtual function");
}


// 
// establish socket that will be advertised to paradynd's
// this socket will allow paradynd's to connect to paradyn for pvm
//
static void
DMsetupSocket (PDSOCKET &sock)
{
  // setup "well known" socket for pvm paradynd's to connect to
  dataManager::dm->sock_port = RPC_setup_socket (sock, AF_INET, SOCK_STREAM);
  assert(dataManager::dm->sock_port != (-1));

  // bind fd for this thread
  thread_t stid;
  msg_bind_socket (sock, true, NULL, NULL, &stid);
}


void dataManager::displayParadynGeneralInfo()
{
  uiMgr->showError(104, "\0");
}

void dataManager::displayParadynLicenseInfo()
{
  uiMgr->showError(105, "\0");
}

void dataManager::displayParadynReleaseInfo()
{
  uiMgr->showError(106, "\0");
}

void dataManager::displayParadynVersionInfo()
{
  string msg = string("Paradyn Version Identifier:\n")
    + string(V_paradyn) + string("\n")
    + string("\n");
  static char buf[1000];
  sprintf(buf, "%s", msg.c_str());
  uiMgr->showError(107, buf);
}

// displayDaemonStartInfo() presents the information necessary to manually
// start up a Paradyn daemon and have it connect to this Paradyn front-end.

void dataManager::displayDaemonStartInfo() 
{
    const string machine = getNetworkName();
    const string port    = string(dataManager::dm->sock_port);
    string command = string("paradynd -z<flavor> -l2")
                         + string(" -m") + machine + string(" -p") + port;
#if !defined(i386_unknown_nt4_0)
    string term_port = string(dataManager::termWin_port);
    command += string(" -P");
    command += term_port;
#endif
    static char buf[1000];

    string msg = string("To start a paradyn daemon on a remote machine,")
      + string(" login to that machine and run paradynd")
      + string(" with the following arguments:\n\n    ") + command
      + string("\n\n(where flavor is one of: unix, mpi, winnt).\n");
    
    sprintf(buf, "%s", msg.c_str());
    uiMgr->showError(99, buf);
    //fprintf(stderr, msg.c_str());
}

// printDaemonStartInfo() provides the information necessary to manually
// start up a Paradyn daemon and have it connect to this Paradyn front-end,
// writing this information to the file whose name is provided as argument
// (after doing some sanity checks to ensure that the file can be written
// and that it (probably) won't overwrite another file mistakenly).

void dataManager::printDaemonStartInfo(const char *filename)
{
    const string machine = getNetworkName();
    const string port  = string(dataManager::dm->sock_port);
    string command = string("paradynd -z<flavor> -l2")
                     + string(" -m") + machine + string(" -p") + port;
#if !defined(i386_unknown_nt4_0)
    string term_port = string(dataManager::termWin_port);
    command += string(" -P");
    command += term_port;
#endif
    static char buf[1000];

    assert (filename && (filename[0]!='\0'));
    //cerr << "dataManager::printDaemonStartInfo(" << filename << ")" << endl;

    struct stat statBuf;
    int rd = stat(filename, &statBuf);
    if (rd == 0) {                                      // file already exists
        if (S_ISDIR(statBuf.st_mode)) { // got a directory!
            string msg = string("Paradyn connect file \"") + string(filename)
                + string("\" is a directory! - skipped.\n");
            sprintf(buf, "%s", msg.c_str());
            uiMgr->showError(103, buf);
            return;             
        } else if (S_ISREG(statBuf.st_mode) && (statBuf.st_size > 0)) {
            FILE *fp = fopen(filename, "r");        // check whether file exists
            if (fp) {
                int n=fscanf(fp, "paradynd");       // look for daemon info
                if (n<0) {
                    string msg = string("Aborted overwrite of unrecognized \"") 
                      + string(filename)
                      + string("\" contents with daemon start-up information.");
                    sprintf(buf, "%s", msg.c_str());
                    uiMgr->showError(103, buf);
                    fclose(fp);
                    return;
                } else {
                    //fprintf(stderr,"Overwriting daemon start-up information!\n");
                }
                fclose(fp);
            }
        }
    }
    FILE *fp = fopen(filename, "w");
    if (fp) {
        // go ahead and actually (re-)write the connect file
        fprintf(fp, "%s\n", command.c_str());;
        fclose(fp);
    } else {
        string msg = string("Unable to open file \"") + string(filename)
          + string("\" to write daemon start information.");
        sprintf(buf, "%s", msg.c_str());
        uiMgr->showError(103, buf);
    }
}

static void
DMnewParadynd ()
{
  // accept the connection
  PDSOCKET new_sock = RPC_getConnect(dataManager::dm->sock_desc);
  if (new_sock != PDSOCKET_ERROR)
  {
		// add new daemon to dictionary of all deamons
		paradynDaemon::addDaemon(new_sock);
  }
  else
  {
		uiMgr->showError(4, "");
  }
}

bool dataManager::DM_sequential_init(const char* met_file){
   string mfile = met_file;
   return(metMain(mfile)); 
}

#if !defined(i386_unknown_nt4_0)
void prepare_TermWin();
#endif

int dataManager::DM_post_thread_create_init(thread_t tid) {

    thr_name("Data Manager");
    dataManager::dm = new dataManager(tid);

    // supports argv passed to paradynDaemon
    // new paradynd's may try to connect to well known port
    DMsetupSocket (dataManager::dm->sock_desc);

#if !defined(i386_unknown_nt4_0)
    prepare_TermWin();
#endif

    bool aflag;
#if !defined(i386_unknown_nt4_0)
    aflag=(RPC_make_arg_list(paradynDaemon::args,
  	 	             dataManager::dm->sock_port,dataManager::termWin_port, 1, 1, "", false));
#else
    aflag=(RPC_make_arg_list(paradynDaemon::args,
  	 	             dataManager::dm->sock_port, 1, 1, "", false));
#endif
    assert(aflag);

    // start initial phase
    string dm_phase0 = "phase_0";
    phaseInfo::startPhase(dm_phase0, false, false);

    char DMbuff[64];
    unsigned int msgSize = 64;
    msg_send (MAINtid, MSG_TAG_DM_READY, (char *) NULL, 0);
    tag_t tag = MSG_TAG_ALL_CHILDREN_READY;
	thread_t from = MAINtid;
    msg_recv (&from, &tag, DMbuff, &msgSize);
	assert( from == MAINtid );

    return 1;
}

//
// Main loop for the dataManager thread.
//
void *DMmain(void* varg)
{
	thread_t mainTid = *(thread_t*)varg;

    unsigned fd_first = 0;
    // We declare the "printChangeCollection" tunable constant here; it will
    // last for the lifetime of this function, which is pretty much forever.
    // (used to be declared as global in DMappContext.C.  Globally declared
    //  tunables are now a no-no).  Note that the variable name (printCC) is
    // unimportant.   -AT
    tunableBooleanConstantDeclarator printCC("printChangeCollection", 
	      "Print the name of metric/focus when enabled or disabled",
	      false, // initial value
	      NULL, // callback
	      developerConstant);

    // Now the same for "printSampleArrival"
    extern bool our_print_sample_arrival;
    our_print_sample_arrival = false;
    extern void printSampleArrivalCallback(bool);
    tunableBooleanConstantDeclarator printSA("printSampleArrival", 
              "Print out status lines to show the arrival of samples",
	      our_print_sample_arrival, // init val
	      printSampleArrivalCallback,
	      developerConstant);

    // Now the same for "PersistentData"
    tunableBooleanConstantDeclarator persData("persistentData",
	      "Don't delete internal paradyn data when instrumentation disabled",
	      false, // init val
	      NULL,
	      userConstant);

    dataManager::DM_post_thread_create_init(mainTid);

    thread_t tid;
    unsigned int tag;
    paradynDaemon *pd = NULL;
    int err;

    while (1) {
        for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
	  pd = paradynDaemon::allDaemons[i]; 
	  // handle up to max async requests that may have been buffered
	  // while blocking on a sync request
	  while (pd->buffered_requests()){
	    if(pd->process_buffered() == T_dyninstRPC::error) {
	      cout << "error on paradyn daemon\n";
	      paradynDaemon::removeDaemon(pd, true);
	    }
	  }
	}

		// wait for next message from anyone, blocking till available
	tid = THR_TID_UNSPEC;
	tag = MSG_TAG_ANY;
	err = msg_poll_preference(&tid, &tag, true,fd_first);
	assert(err != THR_ERR);
	fd_first = !fd_first;
	
	if (tag == MSG_TAG_SOCKET) {
	  // must be an upcall on something speaking the dynRPC protocol.
	  PDSOCKET fromSock = thr_socket( tid );
	  assert(fromSock != INVALID_PDSOCKET);
	  
	  if (fromSock == dataManager::dm->sock_desc){
	    DMnewParadynd(); // set up a new daemon
	  }
	  else {
	    
	    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
	      pd = paradynDaemon::allDaemons[i]; 
	      if(pd->get_sock() == fromSock){
		
		if(pd->waitLoop() == T_dyninstRPC::error) {
		  cout << "error on paradyn daemon\n";
		  paradynDaemon::removeDaemon(pd, true);
		}
	      }
	      
	      // handle async requests that may have been buffered
	      // while blocking on a sync request
	      while(pd->buffered_requests()){
		if(pd->process_buffered() == T_dyninstRPC::error) {
		  cout << "error on paradyn daemon\n";
		  paradynDaemon::removeDaemon(pd, true);
		}
	      }
	    }
	  }
	} else if (dataManager::dm->isValidTag
		   ((T_dataManager::message_tags)tag)) {
	  if (dataManager::dm->waitLoop(true, (T_dataManager::message_tags)tag) 
	      == T_dataManager::error) {
				// handle error
	    assert(0);
	  }
	} else {
	  cerr << "Unrecognized message in DMmain.C: tag = "
	       << tag << ", tid = "
	       << tid << '\n';
	  assert(0);
	}
    }

    return NULL;
}


void addMetric(T_dyninstRPC::metricInfo &info)
{
    // if metric already exists return
    if(metric::allMetrics.defines(info.name)){
        return;
    }
    metric *met = new metric(info);

    // now tell all perfStreams
    performanceStream::psIter_t allS = performanceStream::getAllStreamsIter();
    perfStreamHandle h;
    performanceStream *ps;
    while(allS.next(h,ps)){
       controlCallback controlData = ps->getControlCallbackData();
	if(controlData.mFunc) {
	    // set the correct destination thread.
	    dataManager::dm->setTid(ps->getThreadID());
	    dataManager::dm->newMetricDefined(controlData.mFunc, 
					      ps->Handle(),
					      met->getName(),
					      met->getStyle(),
					      met->getAggregate(),
					      met->getUnits(),
					      met->getHandle(),
					      met->getUnitsType());
	}
    }
}

void ps_retiredResource(string resource_name) {
   resource *res = resource::string_to_resource(resource_name);
   if(! res) {
      cerr << "Couldn't find resource " << resource_name << "\n";
      return;
   }
   res->markAsRetired();

   /* inform others about it if they need to know */
   performanceStream::psIter_t allS = performanceStream::getAllStreamsIter();
   perfStreamHandle h;
   performanceStream *ps;
   resourceHandle r_handle = res->getHandle();
   assert(res->getAbstractionName());
   while(allS.next(h,ps)) {
      ps->callResourceRetireFunc(r_handle, res->getFullName(), 
                                 res->getAbstractionName());
   }
}

void newSampleRate(timeLength rate)
{
    paradynDaemon *pd = NULL;
    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
        pd = paradynDaemon::allDaemons[i]; 
	pd->setSampleRate(rate.getD(timeUnit::sec()));
    }
}

#ifdef ndef
// Note - the metric parser has been moved into the dataManager
bool parse_metrics(string metric_file) {
     bool parseResult = metMain(metric_file);
    return parseResult;
}
#endif

#if !defined(i386_unknown_nt4_0)
void prepare_TermWin()
{
    int serv_port = RPC_setup_socket(dataManager::termWin_sock,AF_INET,SOCK_STREAM);
    assert(dataManager::termWin_sock != INVALID_PDSOCKET);
    dataManager::termWin_port = serv_port;
}
#endif

