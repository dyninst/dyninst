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

// $Id: DMpublic.C,v 1.143 2004/06/14 22:25:55 legendre Exp $

extern "C" {
#include <malloc.h>
}

#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "dataManager.thread.h"
#include "dataManager.thread.SRVR.h"
#include "dataManager.thread.CLNT.h"
#include "dyninstRPC.xdr.CLNT.h"
#include "visi.xdr.h"
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/Time.h"
#include "pdutil/h/pdSample.h"
#include "pdutil/h/pdDebugOstream.h"
#include "DMmetric.h"
#include "DMdaemon.h"
#include "DMresource.h"
#include "DMperfstream.h"
#include "DMphase.h"
#include "DMinclude.h"
#include "paradyn/src/DMthread/DVbufferpool.h"
#include "paradyn/src/pdMain/paradyn.h"
#include "CallGraph.h"
#include "DMmetricFocusReqBundle.h"

#if !defined(i386_unknown_nt4_0)
#include "termWin.xdr.CLNT.h"
#endif //  !defined(i386_unknown_nt4_0)

// the argument list passed to paradynds
pdvector<pdstring> paradynDaemon::args(0);
extern bool our_print_sample_arrival;

#if !defined(i386_unknown_nt4_0)
// client side of termWin igen interface
termWinUser* twUser = NULL;
#endif //  !defined(i386_unknown_nt4_0)

unsigned enable_pd_samplevalue_debug = 0;

#if ENABLE_DEBUG_CERR == 1
#define sampleVal_cerr if (enable_pd_samplevalue_debug) cerr
#else
#define sampleVal_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

void histDataCallBack(pdSample *buckets, relTimeStamp, int count, int first, 
		      void *callbackData)
{
    struct histCallbackData *callbackDataB = 
      static_cast<struct histCallbackData *>(callbackData);
    metricInstance *mi = callbackDataB->miPtr;
    bool globalFlag    = callbackDataB->globalFlag;

    if (our_print_sample_arrival){
      sampleVal_cerr << "histDataCallBack-  bucket:  " << first 
		     << "  value(1):" << buckets[0] << "  count: " << count 
		     << "   bucketwidth " << metricInstance::GetGlobalWidth()
		     <<"\n";
    }

    if(globalFlag)
      mi->globalPhaseDataCallback(buckets, count, first);
    else
      mi->currPhaseDataCallback(buckets, count, first);
    
    for(int i=first; i < count; i++){
        if(!buckets[i].isNaN() && buckets[i] < pdSample::Zero()) 
	  cerr << "bucket " << i << " : " << buckets[i] << "\n";
    }
}

//
// start_time specifies the phaseType (globalType starts at 0.0) 
//
void histFoldCallBack(const timeLength *_width, void *callbackData)
{
    struct histCallbackData *callbackDataB = 
      static_cast<struct histCallbackData *>(callbackData);
    bool globalFlag    = callbackDataB->globalFlag;

    timeLength width = *_width;
    sampleVal_cerr << "histFoldCallBack: " << width << ", globalFlag: " 
		   << globalFlag << "\n";

    if(globalFlag){
      // only notify clients if new bucket width is larger than previous one
      if(metricInstance::GetGlobalWidth() < width) {
	metricInstance::SetGlobalWidth(width);
	performanceStream::foldAll(width,GlobalPhase);
	// if a current phase exists, then use the current phase sampling
	// rate for the daemon sampling rate (probably because this could
	// be smaller)
	if(!metricInstance::numCurrHists()){  // change the sampling rate
	  newSampleRate(width);
	}
	metricInstance::updateAllAggIntervals();
      }
    }
    else {  // fold applies to current phase
        // only notify clients if new bucket width is larger than previous one
	if(metricInstance::GetCurrWidth() <  width) {
	    metricInstance::SetCurrWidth(width);
	    performanceStream::foldAll(width,CurrentPhase);
	    newSampleRate(width); // change sampling rate
	    metricInstance::updateAllAggIntervals();
	}
	phaseInfo::setCurrentBucketWidth(width);
    }
}

// trace data streams
void traceDataCallBack(const void *data,
                       int length,
                       void *arg)
{
    metricInstance *mi = (metricInstance *) arg;
    performanceStream *ps = 0;
    for(unsigned i=0; i < mi->trace_users.size(); i++) {
         ps = performanceStream::find(mi->trace_users[i]);
         if(ps) {
             ps->callTraceFunc(mi->getHandle(),
                               data,length);
         }
    }
}

void dataManager::setResourceSearchSuppress(resourceHandle res, bool newValue)
{
    resource *r = resource::handle_to_resource(res);
    if(r)
        r->setSuppress(newValue);
}

void dataManager::setResourceSearchChildrenSuppress(resourceHandle res, 
						    bool newValue)
{
    resource *r = resource::handle_to_resource(res);
    if(r) r->setSuppressChildren(newValue);
}

void dataManager::setResourceInstSuppress(resourceHandle res, bool newValue)
{
    resource *r = resource::handle_to_resource(res);
    if (r) paradynDaemon::setInstSuppress(r, newValue);
}


//
// define a new entry for the daemon dictionary
//
bool dataManager::defineDaemon(const char *command,
			       const char *dir,
			       const char *login,
			       const char *name,
			       const char *machine,
				   const char *remote_shell,
			       const char *flavor)
{
  if(!name || !command)
      return false;
  return (paradynDaemon::defineDaemon(command, dir, login, name, machine, remote_shell, flavor));
}

bool dataManager::addExecutable(const char *machine,
				const char *login,
				const char *name,
				const char *dir,
				const pdvector<pdstring> *argv)
{
  bool added = false;

  // This is the implementation of an igen call...usually from the UI thread
  // when a new process is defined in the dialog box.
  pdstring m = machine;
  pdstring l = login;
  pdstring n = name;
  pdstring d = dir;


#if !defined(i386_unknown_nt4_0)
  if( twUser != NULL ) {
    // we have a termWin, so try to start the executable
    added = paradynDaemon::newExecutable(m, l, n, d, *argv);
  }
#else
  // Windows does not yet support the termWin
  added = paradynDaemon::newExecutable(m, l, n, d, *argv);
#endif

  return added;
}


bool dataManager::attach(const char *machine,
			 const char *user,
			 const char *cmd, // program name (full path)
			 const char *pidStr,
			 const char *daemonName,
			 int afterAttach // 0 --> as is, 1 --> pause, 2 --> run
			 ) {
   if (pidStr == NULL && cmd == NULL)
      return false;

   // cmd gives the full path to the executable...we use it just to read
   // the symbol table.

   int pid = pidStr ? atoi(pidStr) : -1;
   return (paradynDaemon::attachStub(machine, user, cmd, pid, daemonName, afterAttach));
      // "Stub" appended to name to avoid name clash with the actual remote igen call
      // attach()
}

bool dataManager::applicationDefined()
{
    return(paradynDaemon::applicationDefined());
}

bool dataManager::startApplication()
{
    return(paradynDaemon::startApplication());
}

bool dataManager::pauseApplication()
{
    return(paradynDaemon::pauseAll());
}

void dataManager::createCallGraph(){
  CallGraph::displayAllCallGraphs();
}

bool dataManager::pauseProcess(int pid)
{
    return(paradynDaemon::pauseProcess(pid));
}

bool dataManager::continueApplication()
{
    return(paradynDaemon::continueAll());
}

bool dataManager::detachApplication(bool pause)
{
   return(paradynDaemon::detachApplication(pause));
}

// 
// write all global data to files created in the dirname directory
// 
void dataManager::saveAllData (const char *dirname, SaveRequestType optionFlag) 
{
  int findex = 0;
  bool success = true;
  metricInstance *activeMI;
  pdstring dir = pdstring (dirname);
  dir += pdstring("/");

  // create index file
  pdstring indexFileName = dir + "index";

  std::ofstream indexFile (indexFileName.c_str(), std::ios::out);
  if (!indexFile) {
    success = false;
  } else {

    pdvector<metricInstanceHandle> allMIHs = 
      metricInstance::allMetricInstances.keys();
    for (unsigned i = 0; i < allMIHs.size(); i++) {
      // try to write data from one metric instance 
      activeMI = metricInstance::getMI(allMIHs[i]);
      if (activeMI == NULL)
	continue;
      if (! (activeMI->saveAllData (indexFile, findex, 
				   dir.c_str(), optionFlag))) {
	success = false;
	break;
      }
    }
    indexFile.close();
  }
  uiMgr->allDataSaved(success);
}

void
dataManager::saveAllResources (const char *dirname)
{
  bool success = true;
  pdstring dir = pdstring (dirname) + pdstring("/resources");

  std::ofstream saveFile (dir.c_str(), std::ios::out);
  if (!saveFile) {
    success = false;
  } else {
    resource::saveHierarchiesToFile(saveFile);
    saveFile.close();
  }
  uiMgr->resourcesSaved(success);
}

perfStreamHandle dataManager::createPerformanceStream(dataType dt,
						      dataCallback dc,
						      controlCallback cc)
{
    int td;
    performanceStream *ps;

    td = getRequestingThread();
    ps = new performanceStream(dt, dc, cc, td);
    return(ps->Handle());
}

int dataManager::destroyPerformanceStream(perfStreamHandle handle){

    performanceStream *ps = performanceStream::find(handle);
    if(!ps) return(0);
    delete ps;
    return(1);
}

//
// If "all" is true, then all metrics will be passed regardless the mode.
// Otherwise, only those metrics corresponding to the current mode will be
// passed.
//
pdvector<pdstring> *dataManager::getAvailableMetrics(bool all)
{
    return(metric::allMetricNames(all));
}

//
// Same comments as for getAvailableMetrics
//
pdvector<met_name_id> *dataManager::getAvailableMetInfo(bool all)
{
    return(metric::allMetricNamesIds(all));
}


metricHandle *dataManager::findMetric(const char *name)
{
    pdstring n = name;
    const metricHandle *met = metric::find(n);
    if(met){
	metricHandle *ret = new metricHandle;
	*ret = *met;
	return(ret);
    }
    return 0;
}

pdvector<resourceHandle> *dataManager::getRootResources()
{
    return(resource::getRootResource()->getChildren());
}

resourceHandle *dataManager::getRootResource()
{
    resourceHandle *rh = new resourceHandle;
    *rh = resource::getRootResource()->getHandle();
    return(rh);
}

//
// make batched enable request to paradyn daemons
//
void DMdoEnableData(perfStreamHandle ps_handle,
                    perfStreamHandle pt_handle,
                    pdvector<metricRLType> *request,
                    u_int request_Id,
                    phaseType type,
                    phaseHandle phaseId,
                    u_int persistent_data,
                    u_int persistent_collection,
                    u_int phase_persistent_data)
{
   pdvector<metricInstance *> *miVec = new pdvector<metricInstance *>;

   for(u_int i=0; i < request->size(); i++){
      // does this metric/focus pair already exist?
      metricInstance *mi = metricInstance::find((*request)[i].met,
                                                (*request)[i].res);
      if(!mi){ // create new metricInstance 
         mi = new metricInstance((*request)[i].res,(*request)[i].met,phaseId);
      }

      if(mi) {
         (*miVec).push_back(mi);
      }
   }

   pdvector<paradynDaemon *> *matchingDaemons = new pdvector<paradynDaemon *>;

   // In the case of whole_prog_focus, I'm still copying daemons into
   // matchingDaemons, in case the number changes within setting up the
   // metricFocusReqBundle.  Since this is a MT program, we can't be
   // guaranteed that this won't be changed at any time.
   paradynDaemon::getMatchingDaemons(miVec, matchingDaemons);

   // if the tunable constant "persistentData" is true, it overrides 
   // individual requests; if false, go by individual request.
   tunableBooleanConstant allPersistentData = 
      tunableConstantRegistry::findBoolTunableConstant ("persistentData");
   u_int persistenceFeature = persistent_data;
   if ( allPersistentData.getValue()) persistenceFeature = 1;

   metricFocusReqBundle *new_bundle = 
      metricFocusReqBundle::createMetricFocusReqBundle(
                                 ps_handle, pt_handle, type, phaseId,
                                 request_Id, *miVec, matchingDaemons,
                                 persistenceFeature, persistent_collection,
                                 phase_persistent_data);

   new_bundle->enableWithDaemons();

#ifdef IS_THIS_NEEDED
   if(type == CurrentPhase){
      for(u_int k=0; k < miVec->size(); k++){
         if((*miVec)[k]) (*miVec)[k]->incrCurrWaiting(); 
      }
   } else{
      for(u_int k=0; k < miVec->size(); k++){
         if((*miVec)[k]) (*miVec)[k]->incrGlobalWaiting(); 
      }
   }
#endif
}

//
// Request to enable a set of metric/focus pairs
// ps_handle - the perfStreamHandle of the calling thread
// pt_handle - the perfTraceStreamHandle of the calling thread
// request   - pdvector of metic/focus pairs to enable
// request_Id - identifier passed by calling thread
// type - which phase type to enable data for
// phaseId - the identifier of the phase for which data is requested
// persistent_data - if set data is not distroyed on last disable 
// persistent_collection - if set data collection isn't stoped on disable 
// phase_persistent_data - like persistent_data, but only valid for curr
//                         phase
//
void dataManager::enableDataRequest(perfStreamHandle ps_handle,
                                    perfStreamHandle pt_handle,
                                    pdvector<metric_focus_pair> *request,
                                    u_int request_Id,
                                    phaseType type,
                                    phaseHandle phaseId,
                                    u_int persistent_data,
                                    u_int persistent_collection,
                                    u_int phase_persistent_data)
{
   if((type == CurrentPhase) && (phaseId != phaseInfo::CurrentPhaseHandle())){
      // send enable failed response to calling thread
      pdvector<metricInstInfo> *response = 
         new pdvector<metricInstInfo>(request->size());
      for(u_int i=0; i < response->size();i++){
         (*response)[i].successfully_enabled = false;	    
      }

      // make response call
      performanceStream::psIter_t allS = 
         performanceStream::getAllStreamsIter();
      perfStreamHandle h; performanceStream *ps;
      while(allS.next(h,ps)){
         if(h == (perfStreamHandle)(ps_handle)){
            ps->callDataEnableFunc(response, request_Id, 1);
            break;
         } }
      // trace data streams
      allS.reset();
      while(allS.next(h,ps)){
         if(h == (perfStreamHandle)(pt_handle)){
            ps->callDataEnableFunc(response, request_Id, 1);
            break;
         }}
      delete request;
      response = 0;
      return;
   }

   // convert request to pdvector of metricRLType
   pdvector<metricRLType> *pairList = new pdvector<metricRLType>;
   for(u_int i=0; i < request->size(); i++){
      metricRLType newPair((*request)[i].met,
                           resourceList::getResourceList((*request)[i].res)); 
      *pairList += newPair; 
   }
   assert(request->size() == pairList->size());
   
   DMdoEnableData(ps_handle,pt_handle,pairList,request_Id,type,phaseId,
                  persistent_data,persistent_collection,phase_persistent_data);

   delete request;
   pairList = 0;
}

//
// same as enableDataRequest but with diff type for request 
//
void dataManager::enableDataRequest2(perfStreamHandle ps,
                                     pdvector<metricRLType> *request,
                                     u_int request_Id,
                                     phaseType type,
                                     phaseHandle phaseId,
                                     u_int persistent_data,
                                     u_int persistent_collection,
                                     u_int phase_persistent_data)
{
   // if currphase and phaseId != currentPhaseId then make approp.
   // response call to client
   if((type == CurrentPhase) && (phaseId != phaseInfo::CurrentPhaseHandle())){
      // send enable failed response to calling thread
      pdvector<metricInstInfo> *response = 
         new pdvector<metricInstInfo>(request->size());
      for(u_int i=0; i < response->size();i++){
         (*response)[i].successfully_enabled = false;	    
      }
      // make response call
      performanceStream::psIter_t allS = 
         performanceStream::getAllStreamsIter();
      perfStreamHandle h;
      performanceStream *ps;
      while(allS.next(h,ps)){
         if(h == (perfStreamHandle)(Address)(ps)){
            ps->callDataEnableFunc(response, request_Id, 1);
            break;
         }
      }
      delete request;
      response = 0;
      return;
   }
   
   // 0 is used as the second parameter for non-trace use
   DMdoEnableData(ps,0,request,request_Id,type,phaseId, persistent_data,
                  persistent_collection,phase_persistent_data);    
   
}


// data is really disabled when there are no current or global users and
// when the persistent_collection flag is clear
// when persistent_data flag is clear:
// current histogram is destroyed when there are no curr users 
// global histogram is destroyed whern there are no curr or gloabl users
// clear active flag on archived histograms rather than deleting them
void DMdisableRoutine(perfStreamHandle handle, 
		      perfStreamHandle pt_handle,
		      metricInstanceHandle mh, 
		      phaseType type)
{
    metricInstance *mi = metricInstance::getMI(mh);
    if (!mi) return;

    // if this mi is not enabled and there are no outstanding enable
    // requests then ignore this disable  
    if(!mi->isEnabled() && 
       !mi->isGlobalEnableOutstanding() && 
       !mi->isCurrEnableOutstanding()) return;

    u_int num_curr_users = mi->currUsersCount();

    // remove user from appropriate list
    if (type == CurrentPhase) {
        mi->removeCurrUser(handle); 
    }
    else {
        mi->removeGlobalUser(handle);
    }

    // trace data streams
    mi->removeTraceUser(pt_handle);

    if (mi->isCollectionPersistent()) {
        // just remove handle from appropriate client list and return
	return;
    }

    // really disable MI data collection?  
    // really disable data when there are no subscribers and
    // there are no outstanding global or curr enables for this MI      
    if (!(mi->currUsersCount()) 
	&& !mi->isGlobalEnableOutstanding() 
	&& !mi->isCurrEnableOutstanding()) {

	u_int num_curr_hists = metricInstance::numCurrHists();
	if (!(mi->isDataPersistent()) && !(mi->isPhaseDataPersistent())){
	    // remove histogram
	    if(mi->deleteCurrHistogram()){
		assert(metricInstance::numCurrHists());
		metricInstance::decrNumCurrHists();
	    }
	}
	else {  // clear active flag on current phase histogram
	    if(mi->data) {
	        mi->data->clearActive();
	        mi->data->setFoldOnInactive();
		if(num_curr_users){
		    assert(metricInstance::numCurrHists());
		    metricInstance::decrNumCurrHists();
                }
	}}

	if (!(mi->globalUsersCount())&& !mi->isGlobalEnableOutstanding()) {
	    mi->dataDisable();  // makes disable call to daemons
	    if (!(mi->isDataPersistent()) && !(mi->isPhaseDataPersistent())){
	        delete mi;	
		mi = 0;
		assert(metricInstance::numGlobalHists());
		metricInstance::decrNumGlobalHists();
	    }
	    else {
		if(mi->global_data) {
	            mi->global_data->clearActive();
	            mi->global_data->setFoldOnInactive();
		    assert(metricInstance::numGlobalHists());
		    metricInstance::decrNumGlobalHists();
	    }}
	}

	// if this was last curr histogram then set sampling rate to global
	if((num_curr_hists) && 
	   (!metricInstance::numCurrHists()) &&  
	   (metricInstance::numGlobalHists())){

            timeLength rate = Histogram::getGlobalBucketWidth();
            newSampleRate(rate);
        }
    }
    return;
}

// data is really disabled when there are no current or global users and
// when the persistent_collection flag is clear
// when persistent_data and phase_persistent_data flags are clear:
// current histogram is destroyed when there are no curr users 
// global histogram is destroyed whern there are no curr or gloabl users
// clear active flag on archived histograms rather than deleting them
void dataManager::disableDataCollection(perfStreamHandle handle, 
					perfStreamHandle pt_handle,
					metricInstanceHandle mh,
					phaseType type) {

    DMdisableRoutine(handle,pt_handle, mh,type);
}

//
// stop collecting data for the named metricInstance and clear the
// persistent data flag(s)
// ps_handle - a handle returned by createPerformanceStream
// mi_handle - a metricInstance returned by enableDataCollection.
// p_type - specifies either global or current phase data
// clear_persistent_data - if true, clear persistent_data flag before disabling
// clear_phase_persistent_data - if true, clear phase_persistent_data flag
//
void dataManager::disableDataAndClearPersistentData(perfStreamHandle ps_handle,
					perfStreamHandle pt_handle,
					metricInstanceHandle mi_handle,
				       	phaseType p_type,
				       	bool clear_persistent_data,
				       	bool clear_phase_persistent_data) {

    metricInstance *mi = metricInstance::getMI(mi_handle);
    if (!mi) return;
    if(clear_phase_persistent_data) mi->clearPhasePersistentData();
    if(clear_persistent_data) mi->clearPersistentData();
    DMdisableRoutine(ps_handle,pt_handle, mi_handle,p_type);
}

//
// This routine returns a list of foci which are the result of combining
// each child of resource rh with the remaining resources that make up rlh
// if the resource rh is a component of the focus rlh, otherwise it returns 0
//
pdvector<rlNameId> *dataManager::magnify(resourceHandle rh, 
				       resourceListHandle rlh, 
				       magnifyType m,
				       resourceHandle currentSearchPath) {
#ifdef PCDEBUG
   printf("Call made to datamanager:magnify, which is calling resourceList::magnify(rh, CallGraphSearch)\n");
#endif
   
   resourceList *rl = resourceList::getFocus(rlh);
   resource *res = resource::handle_to_resource(currentSearchPath);
   if(rl) {
      return rl->magnify(rh, m, res);
   }
   return 0;
}


//
// if resource rh is a decendent of a component of the focus, return a new
// focus consisting of rh replaced with it's corresponding entry in rlh,
// otherwise return the focus rlh
//
resourceListHandle *dataManager::constrain(resourceHandle rh,
					  resourceListHandle rlh){
    resourceList *rl = resourceList::getFocus(rlh);
    if (rl) {
	 resourceListHandle *return_handle = rl->constrain(rh);
	if(return_handle){
	    return return_handle;
	}
    }
    resourceListHandle *default_handle = new resourceListHandle;
    *default_handle = rlh;
    return default_handle;
}

//
// like constrain, except it returns 0 on failure
//
resourceListHandle *dataManager::morespecific(resourceHandle rh,
					      resourceListHandle rlh){
    resourceList *rl = resourceList::getFocus(rlh);
    if (rl) {
	return(rl->constrain(rh));
    }
    return 0;
}

//
// returns true if seppressSearch is true for this focus
//
bool dataManager::isSuppressed(resourceListHandle rlh){

    resourceList *rl = resourceList::getFocus(rlh);
    if (rl) {
        return(rl->isSuppressed());
    }
    return 0;
}

//
// returns the name for the focus associated with this MI
// returns 0 on error
//
const char *dataManager::getFocusNameFromMI(metricInstanceHandle mh){
    metricInstance *mi = metricInstance::getMI(mh);
    if(mi){ 
	return resourceList::getName(mi->getFocusHandle()); 
    }
    return 0;
}

//
// setting and clearing persistentCollection or persistentData flags
// have no enable/disable side effects 
//
void dataManager::setPersistentCollection(metricInstanceHandle mh){
    metricInstance *mi = metricInstance::getMI(mh);
    if(!mi) return;
    mi->setPersistentCollection();
}
void dataManager::clearPersistentCollection(metricInstanceHandle mh){
    metricInstance *mi = metricInstance::getMI(mh);
    if(!mi) return;
    mi->clearPersistentCollection();
}
void dataManager::setPersistentData(metricInstanceHandle mh){
    metricInstance *mi = metricInstance::getMI(mh);
    if(!mi) return;
    mi->setPersistentData();

}

//
// clears both phase_persistent_data flag and persistent data flag
// this routine may result in the MI being deleted
//
void dataManager::clearPersistentData(metricInstanceHandle mh){
    metricInstance *mi = metricInstance::getMI(mh);
    if(!mi) return;
    mi->clearPhasePersistentData();
    if(mi->clearPersistentData()) delete mi;
}

metricHandle *dataManager::getMetric(metricInstanceHandle mh)
{
    metricInstance *mi = metricInstance::getMI(mh);
    if(!mi) return 0;

    metricHandle *handle = new metricHandle;
    *handle = mi->getMetricHandle();
    return(handle);
}

const char *dataManager::getMetricNameFromMI(metricInstanceHandle mh)
{
    metricInstance *mi = metricInstance::getMI(mh);
    if(mi){ 
	return(metric::getName(mi->getMetricHandle()));
    }
    return 0;
}

const char *dataManager::getMetricName(metricHandle m)
{
    const char *name = (metric::getName(m));
    if(name)
        return(name);
    return 0;
}

void dataManager::getMetricValue(metricInstanceHandle mh, 
				 pdSample *retSample)
{
    metricInstance *mi = metricInstance::getMI(mh);
    if(mi) {
      *retSample = mi->getValue();
    } else {
      *retSample = pdSample::NaN();
    }
}

void dataManager::getTotValue(metricInstanceHandle mh, pdSample *retSample)
{
    metricInstance *mi = metricInstance::getMI(mh);
    if(mi) {
      *retSample = mi->getTotValue();
    } else {
      *retSample = pdSample::NaN();
    }
}

//
// converts from a pdvector of resourceHandles to a resourceListHandle
//
resourceListHandle dataManager::getResourceList(const pdvector<resourceHandle> *h)
{
  
    resourceListHandle r = resourceList::getResourceList(*h);
    return r;
}

//
// returns the corresponding focus name for a given resourceHandle pdvector
//
const char *dataManager::getFocusName(const pdvector<resourceHandle> *rh)
{
  resourceListHandle rlh = resourceList::getResourceList(*rh);
  resourceList *rl = resourceList::getFocus(rlh);
  if (rl) 
    return(rl->getName());
  return 0;
}

//
// returns the name for the focus associated with this handle
// returns 0 on error
//
const char *dataManager::getFocusNameFromHandle(resourceListHandle rlh){
  return resourceList::getName(rlh); 
}


//
// converts from a resourceListHandle to a pdvector of resourceHandles
//
pdvector<resourceHandle> *dataManager::getResourceHandles(resourceListHandle h)
{
    return resourceList::getResourceHandles(h);
}

//
// converts from a resource name to a resourceHandle
//
resourceHandle *dataManager::findResource(const char *name){

    resourceHandle *rl = new resourceHandle;
    pdstring r_name = name;
    if(resource::string_to_handle(r_name,rl)){
        return(rl);
    }
    return 0;
}

//
// returns name of resource (this is not a unique representation of 
// the name instead it is the unique name trunctated)
// so for "/Code/blah.c/foo" this routine will return "foo"
//
const char *dataManager::getResourceLabelName(resourceHandle h){

     const char *s = resource::getName(h);
     if(s){
	 return(s);
     }
     return 0;
}

//
// returns full name of resource ie.  "/Code/blah.c/foo"
//
const char *dataManager::getResourceName(resourceHandle h){

     const char *s = resource::getFullName(h);
     if(s){
	 return(s);
     }
     return 0;
}

//
// converts from a focus name to a resourceListHandle
//
resourceListHandle *dataManager::findResourceList(const char *name){

    pdstring n = name;
    const resourceListHandle *temp = resourceList::find(n);
    if(temp){
        resourceListHandle *h = new resourceListHandle;
	*h = *temp;
	return(h);
    }
    return 0;
}

//query for matched metric_focus_pair
pdvector<metric_focus_pair> *dataManager::matchMetFocus(metric_focus_pair *metfocus)
{
	pdvector<metric_focus_pair> *result = new pdvector<metric_focus_pair>;
	const pdvector<metricInstance*> *match_metInsts=metricInstance::query(*metfocus);
	//change results to metric_focus_pair
	for (unsigned i=0; i < match_metInsts->size(); i++)
	{
		metricInstance *mi=(*match_metInsts)[i];
		metricHandle mi_h = mi->getMetricHandle();
		pdvector<resourceHandle> *mi_focus=resourceList::getResourceHandles(mi->getFocusHandle());
                assert(mi_focus != NULL);

		*result += metric_focus_pair(mi_h,*mi_focus);
		delete mi_focus;
	}
	delete match_metInsts;
	if (result->size() == 0)
		return NULL;
	return result;
}

void dataManager::getPredictedDataCost(perfStreamHandle ps_handle,
       				       metricHandle m_handle,
				       resourceListHandle rl_handle,
				       u_int clientId)
{
    metric *m = metric::getMetric(m_handle);
    resourceList *rl = resourceList::getFocus(rl_handle);
    if(m && rl){
        paradynDaemon::getPredictedDataCostCall(ps_handle,m_handle,
						rl_handle,rl,m,clientId);
    }
    else {
      cerr << "Error in DMpublic.C, m=NULL\n";
      assert(0);
    }
}

// caller provides array of sampleValue to be filled
// returns number of buckets filled
int dataManager::getSampleValues(metricInstanceHandle mh,
				 pdSample *buckets,
				 int numberOfBuckets,
				 int first,
				 phaseType phase)
{
    metricInstance *mi = metricInstance::getMI(mh);
    if(mi) 
        return(mi->getSampleValues(buckets, numberOfBuckets, first, phase));
    return(0); 
}


// fill the passed array of buckets with the archived histogram values
// of the passed metricInstance
// returns number of buckets filled
int dataManager::getArchiveValues(metricInstanceHandle mh,
		     pdSample *buckets,
		     int numberOfBuckets,
		     int first,
		     phaseHandle phase_id){

    metricInstance *mi = metricInstance::getMI(mh);
    if(mi) 
        return(mi->getArchiveValues(buckets, numberOfBuckets, first, phase_id));
    return 0;
}


void dataManager::printResources()
{
    resource::printAllResources();
}

void dataManager::printStatus()
{
    paradynDaemon::printStatus();
}

void dataManager::coreProcess(int pid)
{
    paradynDaemon::dumpCore(pid);
}

void dataManager::StartPhase(const relTimeStamp *startTimePtr,const char *name,
			     bool with_new_pc, bool with_visis)
{
    pdstring n = name;
    
    if(startTimePtr == NULL)
      phaseInfo::startPhase(n, with_new_pc, with_visis);
    else 
      phaseInfo::startPhase(n, with_new_pc, with_visis, *startTimePtr);

    // change the sampling rate
    if(metricInstance::numCurrHists()){
      // set sampling rate to curr phase histogram bucket width 
      timeLength rate = phaseInfo::GetLastBucketWidth();
      newSampleRate(rate);
    }
    else {
      // set sampling rate to global phase histogram bucket width 
      timeLength rate = Histogram::getGlobalBucketWidth();
      newSampleRate(rate);
    }
}

pdvector<T_visi::phase_info> *dataManager::getAllPhaseInfo(){
    return(phaseInfo::GetAllPhaseInfo());
}

//
// Now for the upcalls.  We provide code that get called in the thread that
//   requested the call back.
//
void dataManagerUser::newMetricDefined(metricInfoCallback cb,
				  perfStreamHandle p_handle,
				  const char *name,
				  int style,
				  int aggregate,
				  const char *units,
				  metricHandle handle,
				  dm_MetUnitsType units_type)
{
    
    (cb)(p_handle, name, style, aggregate, units, handle, units_type);
}

void dataManagerUser::newResourceDefined(resourceInfoCallback cb,
					 perfStreamHandle handle,
					 resourceHandle parent,
					 resourceHandle newResource,
					 const char *name,
					 const char *abstr)
{
    (cb)(handle, parent, newResource, name, abstr);
}

void dataManagerUser::retireResource_(resourceRetireCallback cb,
                                      perfStreamHandle handle,
                                      resourceHandle uniqueID,
                                      const char *name,
                                      const char *abstr)
{
   (cb)(handle, uniqueID, name, abstr);
}

void dataManagerUser::changeResourceBatchMode(resourceBatchModeCallback cb,
					 perfStreamHandle handle,
					 batchMode mode)
{
    (cb)(handle, mode);
}

//
// response to enableDataRequest call
// func - callback function registered by client thread on createPerfStream
// response - pdvector of enable reponses to enable
// request_id - identifier passed by client to enableDataRequest
//
void dataManagerUser::enableDataCallback(enableDataCallbackFunc func,
                                         pdvector<metricInstInfo> *response,
                                         u_int request_id,
                                         u_int last_cb_for_request) {
    (func)(response, request_id, last_cb_for_request);
}


// the callback function must do a delete on widthPtr
void dataManagerUser::histFold(histFoldCallback cb,
			       perfStreamHandle handle,
			       timeLength *widthPtr,
			       phaseType phase_type)
{
    (cb)(handle, widthPtr, phase_type);
}

// the callback function must do a delete on widthPtr
void dataManagerUser::setInitialActualValue(initActValCallback cb,
					    perfStreamHandle handle,
					    metricHandle mi,
					    pdSample *initActValPtr,
					    phaseType phase_type)
{
    (cb)(handle, mi, initActValPtr, phase_type);
}

void dataManagerUser::changeState(appStateChangeCallback cb,
			          perfStreamHandle handle,
			          appState state)
{
    (cb)(handle, state);
}

void dataManagerUser::newPerfData(sampleDataCallbackFunc func, 
				  pdvector<dataValueType> *data,
				  u_int num_data_values){

    (func)(data, num_data_values);
}

void dataManagerUser::forceFlush(forceFlushCallbackFunc func){
    (func)();
}

// trace data streams
void dataManagerUser::newTracePerfData(traceDataCallbackFunc func,
                                  pdvector<traceDataValueType> *traceData,
                                  u_int num_traceData_values){

    timeStamp *heapTime = new timeStamp;
    (func)(0, 0, heapTime, num_traceData_values,traceData);
}

void dataManagerUser::predictedDataCost(predDataCostCallbackFunc func, 
				  metricHandle,
				  resourceListHandle,
				  float cost,
				  u_int clientID){

    (func)(clientID, cost);
}

void dataManagerUser::newPhaseInfo(newPhaseCallback cb,
				   perfStreamHandle handle,
				   const char *name,
				   phaseHandle phase,
				   relTimeStamp *beginPtr,
				   relTimeStamp *endPtr,
				   timeLength *bucketwidthPtr,
				   bool with_new_pc,
				   bool with_visis) {

    (cb)(handle, name, phase, beginPtr, endPtr, bucketwidthPtr, with_new_pc,
	 with_visis);
}


T_dyninstRPC::metricInfo *dataManager::getMetricInfo(metricHandle m_handle) {

    const T_dyninstRPC::metricInfo *met = metric::getInfo(m_handle);
    if(met){ 
	T_dyninstRPC::metricInfo *copy = new T_dyninstRPC::metricInfo;
	copy->style = met->style;
	copy->units = met->units;
	copy->name = met->name;
	copy->aggregate = met->aggregate;
	copy->handle = met->handle;
	copy->unitstype = met->unitstype;
	return(copy);
    }
    return 0;
}


timeLength*
dataManager::getBucketWidth( metricInstanceHandle mih, phaseType ptype )
{
    timeLength* ret = NULL;

    metricInstance* mi = metricInstance::allMetricInstances[mih];
    if( (mi != NULL) && (mi->hasData(ptype)) )
    {
        ret = new timeLength( mi->getBucketWidth( ptype ) );
    }
    return ret;
}


void dataManager::getGlobalBucketWidth(timeLength *widthVal)
{
    *widthVal = Histogram::getGlobalBucketWidth();
}

void dataManager::getCurrentBucketWidth(timeLength *widthVal)
{
    *widthVal = phaseInfo::GetLastBucketWidth();
}

void dataManager::getCurrentStartTime(relTimeStamp *getTimeVal) 
{
    *getTimeVal = phaseInfo::GetLastPhaseStart();
}

u_int dataManager::getCurrentPhaseId() 
{
    return(phaseInfo::CurrentPhaseHandle());
}



int dataManager::getMaxBins()
{
    return(Histogram::getNumBins());
}

void dataManager::printDaemons()
{
  paradynDaemon::printDaemons();
}

pdvector<pdstring> *dataManager::getAvailableDaemons()
{
    return(paradynDaemon::getAvailableDaemons());
}
