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

// $Id: DMmetric.C,v 1.42 2002/12/20 07:50:01 jaw Exp $

extern "C" {
#include <malloc.h>
#include <stdio.h>
}
#include "DMmetric.h"
#include <iostream.h>
#include "pdutil/h/pdDebugOstream.h"
#include "common/h/timing.h"

extern void histDataCallBack(pdSample*, relTimeStamp, int, int, void*);
extern void histFoldCallBack(const timeLength*, void*);

// trace data streams
extern void traceDataCallBack(const void*, int, void*);

extern pdDebug_ostream sampleVal_cerr;

metric::metric(T_dyninstRPC::metricInfo i){

    if(allMetrics.defines(i.name)) return;
    info.style = i.style;
    info.units = i.units;
    info.name = i.name;
    info.developerMode = i.developerMode;
    info.unitstype = i.unitstype;
    info.aggregate = i.aggregate;
    info.handle = metrics.size();
    metric *met = this;
    allMetrics[i.name] = met;
    metrics += met;
}

const T_dyninstRPC::metricInfo *metric::getInfo(metricHandle handle) { 
   
    if(handle < metrics.size()){
        metric *met = metrics[handle];
        return(met->getInfo());
    }
    else{
        return 0 ;
    }
}

const char *metric::getName(metricHandle handle){
     if(handle < metrics.size()){
	metric *met = metrics[handle];
        return(met->info.name.c_str());
     }
     else{
        return 0 ;
     }
}

const metricHandle *metric::find(const string iName){ 
     if(allMetrics.defines(iName)){
        return(&(allMetrics[iName])->info.handle);
     }
     else{
        return 0 ;
     }
}

metric *metric::getMetric(metricHandle handle){ 
     if(handle < metrics.size()){
	return(metrics[handle]);
     }
     else{
        return 0 ;
     }
}

pdvector<string> *metric::allMetricNames(bool all){

    pdvector<string> *temp = new pdvector<string>;
    string name;
    tunableBooleanConstant developerMode =
			   tunableConstantRegistry::findBoolTunableConstant(
			   "developerMode");
    bool developerModeActive = developerMode.getValue();
    for(unsigned i=0; i < metrics.size(); i++){
        metric *met = metrics[i];
        if (all || developerModeActive) {
	  string name = met->getName();
	  *temp += name;
        }
	else if (!met->isDeveloperMode()) {
	  string name = met->getName();
	  *temp += name;
        }
    }
    return(temp);
}

pdvector<met_name_id> *metric::allMetricNamesIds(bool all){

    pdvector<met_name_id> *temp = new pdvector<met_name_id>;
    met_name_id next;
    tunableBooleanConstant developerMode =
			   tunableConstantRegistry::findBoolTunableConstant(
		           "developerMode");
    bool developerModeActive = developerMode.getValue();

    for(unsigned i=0; i < metrics.size(); i++){
        metric *met = metrics[i];
	if (all || developerModeActive) {
          next.name = met->getName();
	  next.id = met->getHandle();
	  *temp += next; 
        }
        else if (!met->isDeveloperMode()) {
	  next.name = met->getName();
	  next.id = met->getHandle();
	  *temp += next;
        }
    }
    return(temp);
}

metricInstance::metricInstance(resourceListHandle rl, 
			       metricHandle m,
			       phaseHandle ph): 
  enabledTime(timeLength::Zero()), 
  aggregator(aggregateOp(metric::getMetric(m)->getAggregate()), 
	     determineAggInterval())
{
    met = m;
    focus = rl;
    //metric *mp = metric::getMetric(m);
    //sample.aggOp = mp->getAggregate();
    data = 0;
    global_data = 0;
    persistent_data = false;
    persistent_collection = false;
    phase_persistent_data = false;
    currEnablesWaiting = 0;
    globalEnablesWaiting = 0;
    enabled = false;
    currently_enabling = false;
    metricInstance::curr_phase_id = ph;
    id = next_id++;
    allMetricInstances[id] = this;

    // trace data streams
    traceFunc = 0;
}

metricInstance::~metricInstance() {
    for(unsigned i=0; i < components.size(); i++){
        delete (components[i]);    
    }
    // for(unsigned j=0; j < parts.size(); j++){
    //        delete (parts[j]);    
    // }
    for(unsigned k=0; k < old_data.size(); k++){
        delete (old_data[k]);    
    }
    if (data) delete(data);
    if (global_data) delete(global_data);
    // remove metricInstace from list of allMetricsInstances
    allMetricInstances.undef(id);
}

int metricInstance::getArchiveValues(pdSample *buckets,int numOfBuckets,
				    int first,phaseHandle phase_id){

    // find histogram associated with phase_id
    for(unsigned i = 0; i < old_data.size(); i++){
        if((old_data[i])->phaseId == phase_id){
	    if((old_data[i])->data)
                return((old_data[i])->data->getBuckets(buckets,
						       numOfBuckets,first));
	}
    }
    return -1;
}

int metricInstance::getSampleValues(pdSample *buckets,int numOfBuckets,
				    int first,phaseType phase){
    if(phase == CurrentPhase){
        if (!data) return (-1);
        return(data->getBuckets(buckets, numOfBuckets, first));
    }
    else {
        if (!global_data) return (-1);
        return(global_data->getBuckets(buckets, numOfBuckets, first));
    }

}

void
metricInstance::saveOneMI_Histo (ofstream& fptr,
				 Histogram *hdata,
				 int phaseid)
{
  fptr << "histogram " << getMetricName() << endl <<  
    getFocusName() << endl;
	
  int numBins = hdata->getNumBins();
  pdSample *buckets = new pdSample[numBins];
  unsigned count = hdata->getBuckets(buckets, numBins, 0);
  timeLength width = hdata->getBucketWidth();
  relTimeStamp startTime = hdata->getStartTime();
  // header info:  numBuckets, bucketWidth, startTime, count, id 
  fptr << count << " " << width << " " << startTime << " " << 
    " " << phaseid << endl;
  // data
  for (unsigned k = 0; k < count; k++) {
    fptr << buckets[k] << endl;
  }
  delete buckets;
}

// 
// write out all data for a single histogram to specified file
// this routine assumes that fptr points to a valid file open for writing!  
//
bool 
metricInstance::saveAllData (ofstream& iptr, 
			     int &findex,
			     const char *dirname,
			     SaveRequestType oFlag)
{
  Histogram *hdata = NULL;
  int phaseid = 0;

  if ((oFlag == Phase) || (oFlag == All)) {
    // save all phase data

    // export archived (previous) phases, if any
    for (unsigned i = 0; i < old_data.size(); i++) {
      hdata = (old_data[i])->data;
      if ( hdata ) {

	string fileSuffix = string("hist_") + string(findex);
	string miFileName = string(dirname) + fileSuffix;
	
	ofstream fptr (miFileName.c_str(), ios::out);
	if (!fptr) {
	  return false;
	}
	phaseid = old_data[i]->phaseId;
	saveOneMI_Histo (fptr, hdata, phaseid);
	fptr.close();
	// update index file
	iptr << fileSuffix.c_str() << " " <<  
	  getMetricName() << " " << getFocusName() << " " << phaseid << endl;
	findex++;  // increment fileid
      }
    }

    // export current phase
    hdata = data;
    phaseid =  phaseInfo::CurrentPhaseHandle();
    if (hdata != NULL) {
      // check for histogram not created yet for this MI

      string fileSuffix = string("hist_") + string(findex);
      string miFileName = string(dirname) + fileSuffix;
      
      ofstream fptr (miFileName.c_str(), ios::out);
      if (!fptr) {
	return false;
      }
      saveOneMI_Histo (fptr, hdata, phaseid);
      fptr.close();
      // update index file
      iptr << fileSuffix.c_str() << " " <<  
	getMetricName() << " " << getFocusName() << " " << phaseid << endl;
      findex++;  // increment fileid
    }
  }

  if ((oFlag == Global)  || (oFlag == All)) {
    hdata = global_data;
    if (hdata != NULL) {
      // check for histogram not created yet for this MI
      //  (I think actually this is an error for the global phase - klk)
      string fileSuffix = string("hist_") + string(findex);
      string miFileName = string(dirname) + fileSuffix;
      
      ofstream fptr (miFileName.c_str(), ios::out);
      if (!fptr) {
	return false;
      }
      phaseid = -1;  // global phase id is set
      saveOneMI_Histo (fptr, hdata, phaseid);
      fptr.close();
      // update index file
      iptr << fileSuffix.c_str() << " " <<  
	getMetricName() << " " << getFocusName() << " -1" //global phaseid
	   << endl;
      findex++;  // increment fileid
    }
  }
  return true;
}

metricInstance *metricInstance::getMI(metricInstanceHandle mh){

    metricInstance* mih;
    bool found = allMetricInstances.find(mh, mih);
    if (found) {
        return mih;
    }
    return 0;
}

timeLength metricInstance::getBucketWidth(phaseType phase) {
    if(phase == CurrentPhase) {
        assert(data);
        return data->getBucketWidth();
    }
    else {
        assert(global_data);
        return global_data->getBucketWidth();
    }
}

void metricInstance::globalPhaseDataCallback(pdSample *buckets, int count, 
					     int first) {
  performanceStream *ps = 0;
  for(unsigned i=0; i < global_users.size(); i++) {
    perfStreamEntry *entry = &global_users[i];
    ps = performanceStream::find(entry->psHandle); 
    if(ps) {
      if(entry->sentInitActualVal == false) {
	pdSample initActualVal = getInitialActualValue();
	ps->callInitialActualValueFunc(getHandle(), initActualVal,GlobalPhase);
	entry->sentInitActualVal = true;
      }
      ps->callSampleFunc(getHandle(), buckets, count, first, GlobalPhase);
    }
  }
}

void metricInstance::currPhaseDataCallback(pdSample *buckets, int count, 
					   int first) {
  performanceStream *ps = 0;
  for(unsigned i=0; i < users.size(); i++) {
    perfStreamEntry *entry = &users[i];
    ps = performanceStream::find(entry->psHandle); 
    if(ps) {
      if(entry->sentInitActualVal == false) {
	metric *met = metric::getMetric(getMetricHandle());
	pdSample initActualVal;
	// I hope to replace the metric style or type variable in the
	// future with a per metric variable (set when a metric is
	// defined in the pcl file) named resetSampleValOnPhaseStart

	if(met->getStyle() == SampledFunction) {
	  initActualVal = global_data->getCurrentActualValue();
	} else if(met->getStyle() == EventCounter) {
	  // this should typically be zero
	  initActualVal = getInitialActualValue();
	}
	ps->callInitialActualValueFunc(getHandle(), initActualVal, 
				       CurrentPhase);
	entry->sentInitActualVal = true;
      }
      ps->callSampleFunc(getHandle(), buckets, count, first, CurrentPhase);
    }
  }
}
 
void metricInstance::updateAllAggIntervals() {
  pdvector<metricInstanceHandle> allMIHs;
  allMIHs = metricInstance::allMetricInstances.keys();

  // if a fold has occurred, then update the sampleAggregate object's
  // interval widths
  timeLength globalAggIntvl = determineAggInterval();
  for (unsigned i = 0; i < allMIHs.size(); i++) {
    metricInstance *mi = metricInstance::getMI(allMIHs[i]);
    mi->updateAggInterval(globalAggIntvl);
  }
}

timeLength metricInstance::determineAggInterval() {
  // This method will return the aggregation interval width to be used in the
  // sampleAggregator for the metricInstances.  We want to use the bucket
  // width for the current phase if one is running.  If not we'll use the
  // bucket width for the global phase.  If neither of these have been
  // initialized yet, then we'll use the global bucket width of the
  // histogram.

  // The current phase bucket width is guaranteed to be less than or equal to
  // the bucket width of the global phase.  We want to use the smaller bucket
  // width (ie. the current phase).  The sampleAggregator object for the
  // metricInstance handles aggregating the samples coming in and passes the
  // results to both the current phase and the global phase histograms.  If
  // we used the (possible) larger bucket width of the global phase this
  // would cause us to feed low precision samples (ie. large intervals) to
  // the current phase histogram.  Using the (possibly) smaller current phase
  // bucket width will cause us to pass more aggregated samples to the global
  // bucket width (because smaller intervals), but this won't cause any
  // problems and will not loose any precision.

  timeLength chosenWidth;
  if(numCurrHists() > 0) {
    chosenWidth = GetCurrWidth();
  }
  else {
    chosenWidth = GetGlobalWidth();
  }

  return chosenWidth;
}

// TODO: remove asserts
void metricInstance::dataDisable() {
    assert(!users.size());
    assert(!global_users.size());

    pdvector<component *>::iterator iter = components.end();
    while(iter != components.begin()) {
       iter--;
       removeComponent(*iter);
       components.erase(iter);  
    }
 
    enabled = false;
    assert(components.size()==0);
}

void metricInstance::removeCurrUser(perfStreamHandle ps){

    // remove ps from pdvector of users
    unsigned size = users.size();
    for(unsigned i=0; i < size; i++){
	if(users[i].psHandle == ps){
	    users[i] = users[size-1];
	    users.resize(size-1);
	    assert(users.size() < size);
	    // decrease ps's data buffer size
	    performanceStream::removeCurrentUser(ps);
	    return;
    } }
}

void metricInstance::removeGlobalUser(perfStreamHandle ps){

    // remove ps from pdvector of users
    unsigned size = global_users.size();
    for(unsigned i=0; i < size; i++){
	if(global_users[i].psHandle == ps){
	    global_users[i] = global_users[size-1];
	    global_users.resize(size-1);
	    assert(global_users.size() < size);
	    // decrease ps's data buffer size
	    performanceStream::removeGlobalUser(ps);
	    return;
    } }
}

// trace data streams
void metricInstance::removeTraceUser(perfStreamHandle ps){

    // remove ps from pdvector of users
    unsigned size = trace_users.size();
    for(unsigned i=0; i < size; i++){
        if(trace_users[i] == ps){
            trace_users[i] = trace_users[size-1];
            trace_users.resize(size-1);
            assert(trace_users.size() < size);
            // decrease ps's data buffer size
            performanceStream::removeTraceUser(ps);
            return;
    } }
}

// returns true if histogram really was deleted
bool metricInstance::deleteCurrHistogram(){

    // if curr histogram exists and there are no users delete
    if(!(users.size()) && data) {
        delete data;
        data = 0;
	return true;
    }
    return false;
}

metricInstance *metricInstance::find(metricHandle mh, resourceListHandle rh){

    
    dictionary_hash_iter<metricInstanceHandle,metricInstance *> 
			allMI(allMetricInstances);
    metricInstanceHandle handle;
    metricInstance *mi;
    while(allMI.next(handle,mi)){
	if((mi->getMetricHandle() == mh) && (mi->getFocusHandle() == rh)){
	    return(mi);
	}
    }
    return 0;
}

pdvector<metricInstance*> *metricInstance::query(metric_focus_pair metfocus)
{
    resourceListHandle focus_handle=resourceList::getResourceList(metfocus.res);
    pdvector<metricInstance*> *result=new pdvector<metricInstance*>;

    dictionary_hash_iter<metricInstanceHandle,metricInstance *> 
			allMI(allMetricInstances);
    metricInstanceHandle handle;
    metricInstance *mi;
    while(allMI.next(handle,mi)){
    	if (metfocus.met != UNUSED_METRIC_HANDLE) {
   	    if((mi->getMetricHandle() == metfocus.met) && (mi->getFocusHandle() == focus_handle)){
	       *result += mi;
	       return result;
	    }
	}else {
	       pdvector<resourceHandle> *mi_focus=resourceList::getResourceHandles(mi->getFocusHandle());
	       assert(mi_focus != NULL);

	       string mi_focus_name = resource::DMcreateRLname(*mi_focus);
	       delete mi_focus;
	       string focus_name = resource::DMcreateRLname(metfocus.res);
	       char *focus_name_str = P_strdup(focus_name.c_str());

	       string focus_code("/Code");
	       string focus_machine("/Machine");
	       string focus_sync("/SyncObject");
	       int  index=0;
	       char *pos = strtok(const_cast<char *>(focus_name_str),",");
	       for (; pos != NULL; pos=strtok(NULL,","))
	       {
	       	    index++;
		    if (index == 1)
		    	focus_code=pos;
		    else if (index == 2)
		    	focus_machine=pos;
		    else if (index == 3)
		    	focus_sync=pos;
	       }
	       delete focus_name_str;

	       char *mi_focus_str = P_strdup(mi_focus_name.c_str());
	       string mi_code("/Code");
	       string mi_machine("/Machine");
	       string mi_sync("/SyncObject");
	       index=0;
	       pos = strtok(const_cast<char *>(mi_focus_str),",");
	       for (; pos; pos=strtok(NULL,","))
	       {
	       	    index++;
		    if (index == 1)
		    	mi_code=pos;
		    else if (index == 2)
		    	mi_machine=pos;
		    else if (index == 3)
		    	mi_sync=pos;
	       }
	       delete mi_focus_str;

	       if (focus_code == mi_code && focus_machine.prefix_of(mi_machine) && focus_sync.prefix_of(mi_sync))
	       		*result += mi;
	}
    }
    return result;

}
//
// clears the persistent_data flag and deletes any histograms without 
// subscribers.  The values for num_global_hists and num_curr_hists are
// not changed because these are for active histograms, and this routine
// should not delete any histograms that are active.
// returns true if the metric instance can be deleted
//
bool metricInstance::clearPersistentData(){
  
    phase_persistent_data = false;
    // if there are no outstanding enables for this MI and the flag was set 
    if(persistent_data && !globalEnablesWaiting && !currEnablesWaiting){ 
       // if persistent collection is false then may need to delete data
       if(!persistent_collection){
	   // if there are no current subscribers delete curr. hist and 
	   // archieved histograms
	   if(users.size() == 0){
	       if(data){
	           delete data;
	           data = 0;
	       }
	       // delete any archived histograms
               for(unsigned k=0; k < old_data.size(); k++){
                   delete (old_data[k]);    
		   old_data.resize(0);
               }
	   }
           // if there are no curr. or global data subscribers and if
           // persistent_collection is false then delete the metric instance
	   if(!enabled){
               if((global_users.size() == 0) && global_data){ 
		   delete global_data;
		   global_data = 0;
                   persistent_data = false;
		   return true;
	       }
	   }
       }
    }
    persistent_data = false;
    return false;
}

bool metricInstance::allComponentStartTimesReceived() {
  static bool cacheVal = false;
  if(cacheVal == false) {
    bool assignVal = true;
    for (unsigned i=0; i < components.size(); i++) {
      timeStamp compStartTime = components[i]->getDaemon()->getStartTime();
      if(! compStartTime.isInitialized()) assignVal = false;
    }
    cacheVal = assignVal;
  }
  return cacheVal;
}


bool metricInstance::addComponent(component *new_comp) {
    paradynDaemon *new_daemon =  new_comp->getDaemon();
    sampleVal_cerr << "for metricInstance (" << this << " adding aggComponent "
		   << new_comp << "\n";
    for (unsigned i=0; i < components.size(); i++){
         if((components[i])->getDaemon() == new_daemon) return false;
    }
    components.push_back(new_comp);
    new_comp->sample = aggregator.newComponent();

    return true;
}

// remove the component correspondent to daemon
// If there are no more components, flush aggregate samples, 
// and notify clients.
void metricInstance::removeComponent(paradynDaemon *daemon) {
   int lastIndex = static_cast<int>(components.size())-1;

   sampleVal_cerr << "metricInstance::removeComponent(dmn)-  daemon: " 
		  << daemon << "  num of components: " << lastIndex+1 << "\n";

   pdvector<component *>::iterator iter = components.end();
   while(iter != components.begin()) {
      iter--;
      if ((*iter)->getDaemon() == daemon) {
         removeComponent(*iter);
         components.erase(iter);
      }
   }   
}

void metricInstance::removeComponent(component *comp) {
  comp->sample->markAsFinished();
  delete (comp);
  
  if (components.size() == 1) {
    sampleVal_cerr << "Last component removed- ";
    // the last component was removed
    // flush aggregate samples
    struct sampleInterval aggSample;
    if(aggregator.aggregate(&aggSample)) {
      relTimeStamp relStartTime = relTimeStamp(aggSample.start - 
					       getEarliestFirstTime());
      relTimeStamp relEndTime = relTimeStamp(aggSample.end - 
					     getEarliestFirstTime());
      assert(relStartTime >= relTimeStamp::Zero());
      assert(relEndTime   >= relStartTime);
      enabledTime += relStartTime - relEndTime;

      sampleVal_cerr << " flush aggregation, start: " << relStartTime 
		     << "  end: " << relEndTime << "  value: " 
		     << aggSample.value << "\n";
      addInterval(relStartTime, relEndTime, aggSample.value);
    }
    if(data) data->flushUnsentBuckets();
    if(global_data) global_data->flushUnsentBuckets();
    flushPerfStreams();
  }
}

void metricInstance::updateComponent(paradynDaemon *dmn,timeStamp newStartTime,
				    timeStamp newEndTime, pdSample value) {
  if(components.size() == 0) return;

  // find the right component.
  component *part = NULL;
  for(unsigned i=0; i < components.size(); i++) {
    if(components[i]->daemon == dmn) {
      part = components[i];
      break;
      }
  }
  assert(part != NULL);

  // update the aggComponent value associated with the daemon that sent the
  // value, note: using a timeStamp (timeline in sync with real dates) for
  // aggregation, converting to a relTimeStamp when values get bucketed
  aggComponent *aggComp = part->sample;  
  assert(aggComp);
  if(! aggComp->isInitialStartTimeSet()) {
    aggComp->setInitialStartTime(newStartTime);
  }
  aggComp->addSamplePt(newEndTime, value);
}

void metricInstance::doAggregation() {
  // don't aggregate if this metric is still being enabled (we may 
  // not have received replies for the enable requests from all the daemons)
  if (isCurrentlyEnabling())  return;
  
  // if we haven't received all of the component start times yet, return
  if(! allComponentStartTimesReceived()) return;

  // update the metric instance sample value if there is a new interval with
  // data for all parts, otherwise this routine returns false
  // and the data cannot be bucketed by the histograms yet (not all
  // components have sent data for this interval)

  struct sampleInterval aggSample;
  while(aggregator.aggregate(&aggSample)) {
    if(getInitialActualValue().isNaN()) {
      setInitialActualValue(aggregator.getInitialActualValue());
    }
    relTimeStamp relStartTime = 
      relTimeStamp(aggSample.start - paradynDaemon::getEarliestStartTime());
    relTimeStamp relEndTime = 
      relTimeStamp(aggSample.end - paradynDaemon::getEarliestStartTime());
    assert(relStartTime >= relTimeStamp::Zero());
    assert(relEndTime   >= relTimeStamp::Zero());
    enabledTime += relEndTime - relStartTime;

    sampleVal_cerr << "calling addInterval- st:" << aggSample.start 
		   << "  end: " << aggSample.end << "  val: " 
		   << aggSample.value << "\n";
    addInterval(relStartTime, relEndTime, aggSample.value);
  }
}


void metricInstance::flushPerfStreams() {
    sampleVal_cerr << "flushPerfStreams - \n";
    unsigned i;
    for(i=0; i < users.size(); i++){
        performanceStream *pstr = performanceStream::find(users[i].psHandle);
        if(pstr != NULL) {
	  sampleVal_cerr << "  flushing (Curr) perfStream  index: " << i 
			 << "  totalPS: " << users.size() << "  addr: " 
			 << pstr << "\n";
	  pstr->flushBuffer();
	  // flush the VISIthread buffer also 
	  pstr->signalToFlush();
	}
    }
    for(i=0; i < global_users.size(); i++){
        performanceStream *pstr = 
	  performanceStream::find(global_users[i].psHandle);
        if(pstr != NULL) {
	  sampleVal_cerr << "  flushing (Glob) perfStream  index: " << i 
			 << "  totalPS: " << global_users.size() << "  addr: " 
			 << pstr << "\n";
	  pstr->flushBuffer();
	  // flush the VISIthread buffer also 
	  pstr->signalToFlush();
	}
    }
    sampleVal_cerr << "Leaving flushPerfStreams\n";    
}

#ifdef notdef
bool metricInstance::addPart(aggComponent *new_part){
    parts += new_part;
    u_int new_size = num_procs_per_part.size() + 1;
    num_procs_per_part.resize(new_size);
    assert(parts.size() == num_procs_per_part.size());
    return true;
}
#endif

// stops currentPhase data collection for all metricInstances
void metricInstance::stopAllCurrentDataCollection(phaseHandle last_phase_id) {

    dictionary_hash_iter<metricInstanceHandle,metricInstance *> 
			allMI(allMetricInstances);
    metricInstanceHandle handle;
    metricInstance *mi;
 
    pdvector<metricInstance *> remove_list;
    allMI.reset();
    while(allMI.next(handle,mi)){
	remove_list += mi;
    }
     
    assert(remove_list.size() == allMetricInstances.size());
    for(unsigned i=0; i < remove_list.size(); i++){
	mi = remove_list[i];
	mi->currEnablesWaiting = 0;
        // remove all users from user list
	mi->users.resize(0);
	assert(!(mi->users.size()));
	// clear the persistent flag that is only valid within a phase
	mi->clearPhasePersistentData();

	bool was_deleted = false;
        // if persistent data archive curr histogram
	if(mi->isDataPersistent()){
	    if (mi->data) {
	        if(mi->data->isActive()) was_deleted = true;	
	        mi->data->clearActive();
	        mi->data->clearFoldOnInactive();
	        ArchiveType *temp = new ArchiveType;
	        temp->data = mi->data;
	        temp->phaseId = last_phase_id; 
	        mi->old_data += temp;
	        mi->data = 0;
	        temp = 0; 
	    }
	}
	else { // else delete curr histogram
            was_deleted = mi->deleteCurrHistogram();
	}


        // if not persistent collection
	if (!(mi->isCollectionPersistent())){
	    // really disable data collection 
	    if((mi->isEnabled()) && (!mi->globalUsersCount())) { 
		// disable MI data collection 
		mi->dataDisable();
		if(!(mi->isDataPersistent())){
		    delete(mi);
		}
		metricInstance::decrNumGlobalHists();
            }
	    if(was_deleted)
	        metricInstance::decrNumCurrHists();
	}
	else { // else, create new curr histogram with empty curr users list
	    mi->newCurrDataCollection(histDataCallBack,
				      histFoldCallBack);
	}
    }
    // reduce each performance stream's buffer size by the number
    // of current MI's it will no longer be receiving data for
    performanceStream::removeAllCurrUsers();
}

timeStamp metricInstance::getEarliestFirstTime() {
  unsigned size = components.size();
  timeStamp earliestTime = timeStamp::ts2200();
  for(unsigned i=0; i<size; i++) {
    timeStamp pdETime = components[i]->getDaemon()->getEarliestStartTime();
    if(pdETime < earliestTime) earliestTime = pdETime;
  }
  return earliestTime;
}

void metricInstance::addCurrentUser(perfStreamHandle p) {

    for(unsigned i=0; i < users.size(); i++){
        if(users[i].psHandle == p) return;
    }
    perfStreamEntry curEntry;
    curEntry.sentInitActualVal = false;
    curEntry.psHandle = p;
    users.push_back(curEntry);
    assert(users.size());
    // update buffersize for perfStream
    performanceStream::addCurrentUser(p);
}

void metricInstance::addGlobalUser(perfStreamHandle p) {

    for(unsigned i=0; i < global_users.size(); i++){
        if(global_users[i].psHandle == p) return;
    }
    perfStreamEntry curEntry;
    curEntry.sentInitActualVal = false;
    curEntry.psHandle = p;
    global_users.push_back(curEntry);
    assert(global_users.size());
    // update buffersize for perfStream
    performanceStream::addGlobalUser(p);
}

// trace data streams
void metricInstance::addTraceUser(perfStreamHandle p) {

    for(unsigned i=0; i < trace_users.size(); i++){
        if(trace_users[i] == p) return;
    }
    trace_users += p;
    assert(trace_users.size());
    // update buffersize for perfStream
    performanceStream::addTraceUser(p);

}

// trace data streams
void metricInstance::newTraceDataCollection(dataCallBack2 dcb) {
    assignTraceFunc(dcb);
}

void metricInstance::newGlobalDataCollection(dataCallBack dcb, 
					     foldCallBack fcb) 
{
    // histogram has already been created
    if (global_data) {
	global_data->setActive();
	global_data->clearFoldOnInactive();
	return;  
    }
    struct histCallbackData *callbackData = new struct histCallbackData;
    callbackData->miPtr = this;
    callbackData->globalFlag = true;
 
    global_data = new Histogram(relTimeStamp::Zero(), dcb, fcb, callbackData);
}

void metricInstance::newCurrDataCollection(dataCallBack dcb, 
					   foldCallBack fcb) 
{
    // histogram has already been created
    if (data) {
	data->setActive();
	data->clearFoldOnInactive();
        return;  
    }
    // create new histogram
    relTimeStamp start_time = phaseInfo::GetLastPhaseStart();

    struct histCallbackData *callbackData = new struct histCallbackData;
    callbackData->miPtr = this;
    callbackData->globalFlag = false;

    data = new Histogram(start_time, dcb, fcb, callbackData);
    assert(data);
    phaseInfo::setCurrentBucketWidth(data->getBucketWidth());
}


