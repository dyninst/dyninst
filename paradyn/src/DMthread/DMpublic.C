extern "C" {
#include <malloc.h>
}

#include <assert.h>
#include "dataManager.thread.h"
#include "dataManager.thread.SRVR.h"
#include "dataManager.thread.CLNT.h"
#include "dyninstRPC.xdr.CLNT.h"
#include "visi.xdr.h"
#include "util/h/sys.h"
#include "util/h/Vector.h"
#include "util/h/Dictionary.h"
#include "util/h/makenan.h"
#include "DMmetric.h"
#include "DMdaemon.h"
#include "DMresource.h"
#include "DMperfstream.h"
#include "DMphase.h"
#include "DMinclude.h"
#include "paradyn/src/DMthread/DVbufferpool.h"

// the argument list passed to paradynds
vector<string> paradynDaemon::args = 0;
extern bool our_print_sample_arrival;

void histDataCallBack(sampleValue *buckets,
                      timeStamp start_time,
		      int count,
		      int first,
		      void *arg,
		      bool globalFlag)
{
    metricInstance *mi = (metricInstance *) arg;
    performanceStream *ps = 0;

#ifdef n_def
    // debug code that uses tunable constant printSampleArrival
    if (our_print_sample_arrival){
        cout << "bucket:  " << first << "value: "
	     << buckets[0] << "   bucketwidth " 
	     << metricInstance::GetGlobalWidth() <<  endl;
    }
#endif

    if(globalFlag) { 
	// update global data
        for(unsigned i=0; i < mi->global_users.size(); i++) {
	    ps = performanceStream::find(mi->global_users[i]); 
	    if(ps) {
	        ps->callSampleFunc(mi->getHandle(), 
				   buckets, count, first,GlobalPhase);
            }
        }
      }

    else {  // update just curr. phase data
        for(unsigned i=0; i < mi->users.size(); i++) {
	    ps = performanceStream::find(mi->users[i]); 
	    if(ps)
	        ps->callSampleFunc(mi->getHandle(), 
				   buckets, count, first,CurrentPhase);
        }
      }

    for(int i=first; i < count; i++){
        if(buckets[i] < 0) printf("bucket %d : %f \n",i,buckets[i]);
    }
}

//
// start_time specifies the phaseType (globalType starts at 0.0) 
//
void histFoldCallBack(timeStamp width, void *, bool globalFlag)
{

    // need to check if current phase also starts at 0.0
    // if it does, then fold applies to both global and curr phase
    if(globalFlag){
      if(metricInstance::GetGlobalWidth() != width) {
	metricInstance::SetGlobalWidth(width);
	performanceStream::foldAll(width,GlobalPhase);
	if(!metricInstance::numCurrHists()){  // change the sampling rate
	  newSampleRate(width);
	}
      }
    }
    else {  // fold applies to current phase
	if(metricInstance::GetCurrWidth() != width) {
	    metricInstance::SetCurrWidth(width);
	    performanceStream::foldAll(width,CurrentPhase);
	    newSampleRate(width); // change sampling rate
	}
	phaseInfo::setCurrentBucketWidth(width);
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

bool dataManager::addDaemon(const char *machine,
			    const char *login,
			    const char *name)
{
  // fix args so that if char * points to "" then it pts to NULL
  string m = 0;
  if(machine && strlen(machine))  m = machine;
  string l = 0;
  if(login && strlen(login)) l = login;
  string n = 0;
  if(!name) { 
      char *temp = 0; 
      if(!paradynDaemon::setDefaultArgs(temp))
	  return false;
      n = temp;
      delete temp;
  }
  else {
      n = name;
  }
  return (paradynDaemon::getDaemon(m, l, n));
}

//
// define a new entry for the daemon dictionary
//
bool dataManager::defineDaemon(const char *command,
			       const char *dir,
			       const char *login,
			       const char *name,
			       const char *machine,
			       const char *flavor)
{
  if(!name || !command)
      return false;
  return (paradynDaemon::defineDaemon(command, dir, login, name, machine, flavor));
}


bool dataManager::addExecutable(const char *machine,
				const char *login,
				const char *name,
				const char *dir,
				const vector<string> *argv)
{
    string m = machine;
    string l = login;
    string n = name;
    string d = dir;
    return(paradynDaemon::newExecutable(m, l, n, d, *argv));
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

bool dataManager::pauseProcess(int pid)
{
    return(paradynDaemon::pauseProcess(pid));
}

bool dataManager::continueApplication()
{
    return(paradynDaemon::continueAll());
}

bool dataManager::continueProcess(int pid)
{
    return(paradynDaemon::continueProcess(pid));
}

bool dataManager::detachApplication(bool pause)
{
   return(paradynDaemon::detachApplication(pause));
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
    ps = 0;
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
vector<string> *dataManager::getAvailableMetrics(bool all)
{
    return(metric::allMetricNames(all));
}

//
// Same comments as for getAvailableMetrics
//
vector<met_name_id> *dataManager::getAvailableMetInfo(bool all)
{
    return(metric::allMetricNamesIds(all));
}


metricHandle *dataManager::findMetric(const char *name)
{
    string n = name;
    const metricHandle *met = metric::find(n);
    if(met){
	metricHandle *ret = new metricHandle;
	*ret = *met;
	return(ret);
    }
    return 0;
}

vector<resourceHandle> *dataManager::getRootResources()
{
    return(resource::rootResource->getChildren());
}

resourceHandle *dataManager::getRootResource()
{
    resourceHandle *rh = new resourceHandle;
    *rh = resource::rootResource->getHandle();
    return(rh);
}

//
// called by DM enable routines
//
metricInstance *DMenableData(perfStreamHandle ps_handle,
			     metricHandle m, 
			     resourceListHandle rl,
			     phaseType type,
			     unsigned persistent_data, 
			     unsigned persistent_collection)
{
    // does this this metric/focus combination already exist? 
     metricInstance *mi = metricInstance::find(m,rl);

    if (!mi) {  // create new metricInstance
	if(!(mi = new metricInstance(rl,m,phaseInfo::CurrentPhaseHandle()))) {
            return 0;
    }}
    
    bool newly_enabled = false;
    if ( !(mi->isEnabled()) ){  // enable data collection for this MI
        if (!(paradynDaemon::enableData(rl,m,mi))) { 
	    return 0;
        }
	newly_enabled = true;
    }

    metric *metricptr = metric::getMetric(m);

    // update appropriate MI info. 
    if (type == CurrentPhase) {
	 u_int old_current = mi->currUsersCount();
	 bool current_data = mi->isCurrHistogram();
	 mi->newCurrDataCollection(metricptr->getStyle(),
				   histDataCallBack,
				   histFoldCallBack);
	 mi->newGlobalDataCollection(metricptr->getStyle(),
				   histDataCallBack,
				   histFoldCallBack);
         mi->addCurrentUser(ps_handle);

	 // set sample rate to match current phase hist. bucket width
	 if(!metricInstance::numCurrHists()){
	        float rate = phaseInfo::GetLastBucketWidth();
	        newSampleRate(rate);
	 }

	 // new active curr. histogram added if there are no previous
	 // curr. subscribers and either persistent_collection is clear
	 // or there was no curr. histogram prior to this
	 if((!old_current) 
	    && (mi->currUsersCount() == 1)
	    && ((!mi->isCollectionPersistent()) || (!current_data))){ 
	     metricInstance::incrNumCurrHists();
	 }
	 // new global histogram added if this metricInstance was just enabled
	 if(newly_enabled){ 
	     metricInstance::incrNumGlobalHists();
	 }
    }
    else {
	 mi->newGlobalDataCollection(metricptr->getStyle(),
				   histDataCallBack,
				   histFoldCallBack);
         mi->addGlobalUser(ps_handle);

	 // if this is first global histogram enabled and there are no
	 // curr hists, then set sample rate to match global hist. bucket width
	 if(!metricInstance::numCurrHists()){
	    if(!metricInstance::numGlobalHists()){ 
	        float rate = Histogram::getGlobalBucketWidth();
	        newSampleRate(rate);
	 }}

	 // new global hist added: update count
	 if(newly_enabled){  // new active global histogram added
	     metricInstance::incrNumGlobalHists();
	 }
    }

    // update persistence flags:  the OR of new and previous values
    if(persistent_data) {
	mi->setPersistentData();
    }
    if(persistent_collection) {
	mi->setPersistentCollection();
    }

    // cout << "num global hists " << metricInstance::numGlobalHists() << endl;
    // cout << "num curr hists " << metricInstance::numCurrHists() << endl;
    return mi;
}

vector<metricInstance *> *DMenableDataBatch(perfStreamHandle ps_handle,
			     vector<metricHandle> *m, 
			     vector<resourceListHandle> *rl,
			     phaseType type,
			     unsigned persistent_data, 
			     unsigned persistent_collection)
{
  assert((*m).size() == (*rl).size());
  vector<metricInstance *> *miVec;
  vector<bool> miVecEnabled;
  miVec = new vector<metricInstance *>((*m).size());
  miVecEnabled.resize((*m).size());
  assert((*miVec).size() == miVecEnabled.size());
 
  bool newly_enabled = false;
  for (unsigned int i=0;i<(*m).size();i++) {
    miVecEnabled[i] = false;
  
    // does this this metric/focus combination already exist? 
    metricInstance *mi = metricInstance::find((*m)[i],(*rl)[i]);

    if (!mi) {  // create new metricInstance
      if(!(mi = new metricInstance((*rl)[i],(*m)[i],
                                   phaseInfo::CurrentPhaseHandle()))) {
        (*miVec)[i] = NULL;
#ifdef DMDEBUG
        printf("TEST: DMenableDataBatch, miVec[%d] is NULL\n",i);
#endif
      }
      else {
        (*miVec)[i] = mi;
      }
    }
    else {
      (*miVec)[i] = mi;
    }
    if ((*miVec)[i]) {
      if ( !((*miVec)[i]->isEnabled()) ) {
        newly_enabled = true;
      }
      else {
        miVecEnabled[i] = true;
      }
    }
#ifdef DMDEBUG
    else printf("TEST: DMenableDataBatch, mi is NULL\n");
#endif
  }
    
  if (newly_enabled) {
    if (!(paradynDaemon::enableDataBatch(rl,m,miVec,miVecEnabled))) {
      return miVec;
    }
  }

  for (unsigned int i=0;i<(*m).size();i++) {
    if ((*miVec)[i]) {
#ifdef DMDEBUG
      printf("TEST: DMenableDataBatch, miVecEnable[%d] is %d\n",i,miVecEnabled[i]);
#endif
      metric *metricptr = metric::getMetric((*m)[i]);

      // update appropriate MI info. 
      if (type == CurrentPhase) {
	 u_int old_current = (*miVec)[i]->currUsersCount();
	 bool current_data = (*miVec)[i]->isCurrHistogram();
	 (*miVec)[i]->newCurrDataCollection(metricptr->getStyle(),
				   histDataCallBack,
				   histFoldCallBack);
	 (*miVec)[i]->newGlobalDataCollection(metricptr->getStyle(),
				   histDataCallBack,
				   histFoldCallBack);
         (*miVec)[i]->addCurrentUser(ps_handle);

	 // set sample rate to match current phase hist. bucket width
	 if(!metricInstance::numCurrHists()){
	   float rate = phaseInfo::GetLastBucketWidth();
	   newSampleRate(rate);
	 }

	 // new active curr. histogram added if there are no previous
	 // curr. subscribers and either persistent_collection is clear
	 // or there was no curr. histogram prior to this
	 if((!old_current) 
	    && ((*miVec)[i]->currUsersCount() == 1)
	    && ((!(*miVec)[i]->isCollectionPersistent()) || (!current_data))){ 
	     metricInstance::incrNumCurrHists();
	 }
	 // new global histogram added if this metricInstance was just enabled
	 if(!miVecEnabled[i]){ 
	     metricInstance::incrNumGlobalHists();
	 }
      }
      else {
	 (*miVec)[i]->newGlobalDataCollection(metricptr->getStyle(),
				   histDataCallBack,
				   histFoldCallBack);
         (*miVec)[i]->addGlobalUser(ps_handle);

	 // if this is first global histogram enabled and there are no
	 // curr hists, then set sample rate to match global hist. bucket width
	 if(!metricInstance::numCurrHists()){
	    if(!metricInstance::numGlobalHists()){ 
	      float rate = Histogram::getGlobalBucketWidth();
	      newSampleRate(rate);
	 }}

	 // new global hist added: update count
	 if(!miVecEnabled[i]){  // new active global histogram added
	     metricInstance::incrNumGlobalHists();
	 }
      }

      // update persistence flags:  the OR of new and previous values
      if(persistent_data) {
	(*miVec)[i]->setPersistentData();
      }
      if(persistent_collection) {
	(*miVec)[i]->setPersistentCollection();
      }
    }
#ifdef DMDEBUG
    else printf("TEST: DMenableDataBatch, miVec[%d] is NULL 2\n",i);
#endif
  }
  return miVec;
}

metricInstInfo *dataManager::enableDataCollection(perfStreamHandle ps_handle,
					const vector<resourceHandle> *focus, 
					metricHandle m,
					phaseType type,
					unsigned persistent_data,
					unsigned persistent_collection)
{
    if(!focus || !focus->size()){
        if(focus) 
	    printf("error in enableDataCollection size = %d\n",focus->size());
        else
	    printf("error in enableDataCollection focus is NULL\n");
        return 0;
    } 
    resourceListHandle rl = resourceList::getResourceList(*focus);

    metricInstance *mi = DMenableData(ps_handle,m,rl,type,persistent_data,
    				      persistent_collection);
    if(!mi) return 0;
    metricInstInfo *temp = new metricInstInfo;
    assert(temp);
    metric *metricptr = metric::getMetric(m);
    assert(metricptr);
    temp->mi_id = mi->getHandle();
    temp->m_id = m;
    temp->r_id = rl;
    resourceList *rl_temp = resourceList::getFocus(rl);
    temp->metric_name = metricptr->getName();
    temp->metric_units = metricptr->getUnits();
    temp->focus_name = rl_temp->getName();
    temp->units_type = metricptr->getUnitsType();
    return(temp);
    temp = 0;
}

vector<metricInstInfo *> 
*dataManager::enableDataCollectionBatch(perfStreamHandle ps_handle,
                                        vector<metric_focus_pair> *metResPair,
					phaseType type,
					unsigned persistent_data,
					unsigned persistent_collection)
{
    assert(metResPair);
    vector<resourceListHandle> rl;
    vector<metricHandle> met;
    rl.resize((*metResPair).size());
    met.resize((*metResPair).size());
    assert(rl.size());
    assert(met.size());
    for (unsigned int i=0;i<(*metResPair).size();i++) {
      if( !((*metResPair)[i].res).size() ) {
	printf("error in enableDataCollection size = %d\n",
               ((*metResPair)[i].res).size());
        return NULL;
      } 
      rl[i] = resourceList::getResourceList((*metResPair)[i].res);
      met[i] = (*metResPair)[i].met;
    }

    vector<metricInstance *> *mi = DMenableDataBatch(ps_handle,
                                              &met,
                                              &rl,type,persistent_data,
      			                      persistent_collection);
    if(!mi) return NULL;
    vector<metricInstInfo *> *temp;
    temp = new vector<metricInstInfo *> ((*metResPair).size());
    assert(temp);
    assert((*mi).size() == (*metResPair).size());
    for (unsigned int i=0;i<(*metResPair).size();i++) {
      if ((*mi)[i]) {
        metric *metricptr = metric::getMetric((*metResPair)[i].met);
        assert(metricptr);
        (*temp)[i] = new metricInstInfo;
        assert((*temp)[i]);
        (*temp)[i]->mi_id = (*mi)[i]->getHandle();
        (*temp)[i]->m_id = (*metResPair)[i].met;
        (*temp)[i]->r_id = rl[i];
        resourceList *rl_temp = resourceList::getFocus(rl[i]);
        (*temp)[i]->metric_name = metricptr->getName();
        (*temp)[i]->metric_units = metricptr->getUnits();
        (*temp)[i]->focus_name = rl_temp->getName();
        (*temp)[i]->units_type = metricptr->getUnitsType();
      }
      else {
#ifdef DMDEBUG
        printf("TEST: enableDataCollectionBatch, mi[%d] is NULL\n",i);
#endif
        // metric wasn't enabled
        (*temp)[i] = NULL;
      }
    }  
    return(temp);
}


//
// same as other enableDataCollection routine, except takes focus handle
// argument and returns metricInstanceHandle on successful enable 
//
metricInstanceHandle 
*dataManager::enableDataCollection2(perfStreamHandle ps,
				    resourceListHandle rlh,
				    metricHandle mh,
				    phaseType pType,
				    unsigned pID,
				    unsigned persistent_data,
				    unsigned persistent_collection)
{
  // it is possible for enable requests and new phase callbacks to cross,
  // so if this is a current phase request, we need to make sure its still
  // the current phase
  if ((pType == CurrentPhase) && (pID != phaseInfo::CurrentPhaseHandle()))
    return NULL;

    metricInstance *mi = DMenableData(ps,mh,rlh,pType,persistent_data,
				      persistent_collection);
    if(mi){
        metricInstanceHandle *mi_h = new metricInstanceHandle;
	*mi_h = mi->getHandle();
	return(mi_h);
    }
    return 0;
}

// data is really disabled when there are no current or global users and
// when the persistent_collection flag is clear
// when persistent_data flag is clear:
// current histogram is destroyed when there are no curr users 
// global histogram is destroyed whern there are no curr or gloabl users
// clear active flag on archived histograms rather than deleting them
void dataManager::disableDataCollection(perfStreamHandle handle, 
					metricInstanceHandle mh,
					phaseType type)
{

    // cout << " in dataManager::disableDataCollection: mh = " << mh << endl;
    metricInstance *mi = metricInstance::getMI(mh);
    if (!mi) return;

    // if this mi is not enabled then return
    if(!mi->isEnabled()) return;

    u_int num_curr_users = mi->currUsersCount();

    // remove user from appropriate list
    if (type == CurrentPhase) {
        mi->removeCurrUser(handle); 
    }
    else {
        mi->removeGlobalUser(handle);
    }

    if (mi->isCollectionPersistent()) {
        // just remove handle from appropriate client list and return
	return;
    }

    // really disable MI data collection?  
    if (!(mi->currUsersCount())) {
	u_int num_curr_hists = metricInstance::numCurrHists();
	if (!(mi->isDataPersistent())){
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

	if (!(mi->globalUsersCount())) {
	    mi->dataDisable();  // makes disable call to daemons
	    if (!(mi->isDataPersistent())){
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

            float rate = Histogram::getGlobalBucketWidth();
            newSampleRate(rate);
        }
    }
    // cout << "num global hists " << metricInstance::numGlobalHists() << endl;
    // cout << "num curr hists " << metricInstance::numCurrHists() << endl;
    return;
}

//
// This routine returns a list of foci which are the result of combining
// each child of resource rh with the remaining resources that make up rlh
// if the resource rh is a component of the focus rlh, otherwise it returns 0
//
vector<rlNameId> *dataManager::magnify(resourceHandle rh, 
				       resourceListHandle rlh){

    resourceList *rl = resourceList::getFocus(rlh);
    if(rl){
	return(rl->magnify(rh));
    }
    return 0;
}


//
// This routine returns a list of foci each of which is the result of combining
// a child of one of the resources with the remaining resource components of
// rlh, this routine returns 0 if no resource components of rlh have children
// The DM allocates the vector, the client is responsible for deallocation
//
vector<rlNameId> *dataManager::magnify2(resourceListHandle rlh){
    resourceList *rl = resourceList::getFocus(rlh);
    if(rl){
	return (rl->magnify());
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
void dataManager::clearPersistentData(metricInstanceHandle mh){
    metricInstance *mi = metricInstance::getMI(mh);
    if(!mi) return;
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

sampleValue dataManager::getMetricValue(metricInstanceHandle mh)
{
    metricInstance *mi = metricInstance::getMI(mh);
    if(mi) 
	return(mi->getValue());
    float ret = PARADYN_NaN;
    return(ret);
}

sampleValue dataManager::getTotValue(metricInstanceHandle mh)
{
    metricInstance *mi = metricInstance::getMI(mh);
    if(mi) 
	return(mi->getTotValue());
    float ret = PARADYN_NaN;
    return(ret);
}

//
// converts from a vector of resourceHandles to a resourceListHandle
//
resourceListHandle dataManager::getResourceList(const vector<resourceHandle> *h)
{
  
    resourceListHandle r = resourceList::getResourceList(*h);
    return r;
}

//
// returns the corresponding focus name for a given resourceHandle vector
//
const char *dataManager::getFocusName(const vector<resourceHandle> *rh)
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
// converts from a resourceListHandle to a vector of resourceHandles
//
vector<resourceHandle> *dataManager::getResourceHandles(resourceListHandle h)
{
    return resourceList::getResourceHandles(h);
}

//
// converts from a resource name to a resourceHandle
//
resourceHandle *dataManager::findResource(const char *name){

    resourceHandle *rl = new resourceHandle;
    string r_name = name;
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

    string n = name;
    const resourceListHandle *temp = resourceList::find(n);
    if(temp){
        resourceListHandle *h = new resourceListHandle;
	*h = *temp;
	return(h);
    }
    return 0;
}


void dataManager::getPredictedDataCost(perfStreamHandle ps_handle,
       				       metricHandle m_handle,
				       resourceListHandle rl_handle)
{
    metric *m = metric::getMetric(m_handle);
    resourceList *rl = resourceList::getFocus(rl_handle);
    if(m && rl){
        paradynDaemon::getPredictedDataCostCall(ps_handle,m_handle,
						rl_handle,rl,m);
    }
    else {
      // TODO: call approp. callback routine
      cerr << "Error in DMpublic.C, m=NULL\n";
      assert(0);
    }
}

// caller provides array of sampleValue to be filled
// returns number of buckets filled
int dataManager::getSampleValues(metricInstanceHandle mh,
				 sampleValue *buckets,
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
		     sampleValue *buckets,
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
    printAllResources();
}

void dataManager::printStatus()
{
    paradynDaemon::printStatus();
}

void dataManager::coreProcess(int pid)
{
    paradynDaemon::dumpCore(pid);
}

void dataManager::StartPhase(timeStamp start_Time, 
			     const char *name,
			     bool with_new_pc,
			     bool with_visis)
{
    string n = name;
    phaseInfo::startPhase(start_Time,n,with_new_pc,with_visis);
    // cout << "in dataManager::StartPhase " << endl;
    // change the sampling rate
    if(metricInstance::numCurrHists()){
       // set sampling rate to curr phase histogram bucket width 
       float rate = phaseInfo::GetLastBucketWidth();
       newSampleRate(rate);
    }
    else {
       // set sampling rate to global phase histogram bucket width 
       float rate = Histogram::getGlobalBucketWidth();
       newSampleRate(rate);
    }
    // cout << "num global hists " << metricInstance::numGlobalHists() << endl;
    // cout << "num curr hists " << metricInstance::numCurrHists() << endl;

}

vector<T_visi::phase_info> *dataManager::getAllPhaseInfo(){
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

void dataManagerUser::changeResourceBatchMode(resourceBatchModeCallback cb,
					 perfStreamHandle handle,
					 batchMode mode)
{
    (cb)(handle, mode);
}

void dataManagerUser::histFold(histFoldCallback cb,
			       perfStreamHandle handle,
			       timeStamp width,
			       phaseType phase_type)
{
    (cb)(handle, width, phase_type);
}

void dataManagerUser::changeState(appStateChangeCallback cb,
			          perfStreamHandle handle,
			          appState state)
{
    (cb)(handle, state);
}

void dataManagerUser::newPerfData(sampleDataCallbackFunc func, 
				  vector<dataValueType> *data,
				  u_int num_data_values){

    (func)(data, num_data_values);
}

void dataManagerUser::predictedDataCost(predDataCostCallbackFunc func, 
				  metricHandle m_handle,
				  resourceListHandle rl_handle,
				  float cost){

    (func)(m_handle,rl_handle,cost);
}

void dataManagerUser::newPhaseInfo(newPhaseCallback cb,
				   perfStreamHandle handle,
				   const char *name,
				   phaseHandle phase,
				   timeStamp begin,
				   timeStamp end,
				   float bucketwidth,
				   bool with_new_pc,
				   bool with_visis) {

    (cb)(handle,name,phase,begin,end,bucketwidth,with_new_pc,with_visis);
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

#ifdef n_def
resourceHandle dataManager::newResource(resourceHandle parent, 
					const char *newResource) {
    // rbi: kludge 
    // calls to this method should specify an abstraction,
    // but that involves a bunch of other changes that I don't want 
    // to make right now.
    // the kludge works because we know that all calls to this method 
    // are for BASE abstraction resources.  

    // TEMP: until this routine is called with vector of strings for new res
    string res = resource::resources[parent]->getFullName();
    res += string("/");
    res += string(newResource);
    char *word = strdup(res.string_of());
    string next;
    vector<string> temp;
    unsigned j=1;
    for(unsigned i=1; i < res.length(); i++){
	if(word[i] == '/'){
	    word[i] = '\0';
	    next = &word[j];
	    temp += next;
	    j = i+1;
        }
    }
    next = &word[j];
    temp += next;
    string base = string("BASE");
    resourceHandle r = createResource(parent, temp, res, base);  
    paradynDaemon::tellDaemonsOfResource(res.string_of(),newResource);
    return(r);
}
#endif

resourceHandle dataManager::newResource(resourceHandle parent,
			                const char *name, u_int type)
{

    // rbi: kludge
    // calls to this method should specify an abstraction,
    // but that involves a bunch of other changes that I don't want
    // to make right now.
    // the kludge works because we know that all calls to this method
    // are for BASE abstraction resources.
    
    string abs = "BASE";
    resource *parent_res = resource::handle_to_resource(parent);
    vector<string> res_name = parent_res->getParts();
    res_name += name;
    resourceHandle child = createResource(res_name,abs, type);
    paradynDaemon::tellDaemonsOfResource(parent_res->getHandle(), 
			       		 child, 
			                 name, type);
    return(child);

}

timeStamp dataManager::getGlobalBucketWidth()
{
    return(Histogram::getGlobalBucketWidth());
}

timeStamp dataManager::getCurrentBucketWidth()
{
    return(phaseInfo::GetLastBucketWidth());
}

timeStamp dataManager::getCurrentStartTime() 
{
    return(phaseInfo::GetLastPhaseStart());
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

vector<string> *dataManager::getAvailableDaemons()
{
    return(paradynDaemon::getAvailableDaemons());
}

