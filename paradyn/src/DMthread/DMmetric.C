/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 *
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */
extern "C" {
#include <malloc.h>
#include <stdio.h>
}
#include "DMmetric.h"

extern void histDataCallBack(sampleValue*, timeStamp, int, int, void*);
extern void histFoldCallBack(timeStamp, void*, timeStamp);

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
        return(met->info.name.string_of());
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

vector<string> *metric::allMetricNames(bool all){

    vector<string> *temp = new vector<string>;
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

vector<met_name_id> *metric::allMetricNamesIds(bool all){

    vector<met_name_id> *temp = new vector<met_name_id>;
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
			       phaseHandle ph){
    met = m;
    focus = rl;
    enabledTime = 0.0;
    metric *mp = metric::getMetric(m);
    sample.aggOp = mp->getAggregate();
    data = 0;
    global_data = 0;
    persistent_data = 0;
    persistent_collection = 0;
    enabled = false;
    metricInstance::curr_phase_id = ph;

    // find an Id for this metricInstance
    for(unsigned i=0; i < nextId.size(); i++){
        if((!nextId[i])){
	    id = i;
	    nextId[i] = true;
	    allMetricInstances[id] = this;
	    return;
    }}
    // if not found, add a new element to the id array
    id = nextId.size();
    nextId += true;
    assert(id < nextId.size());
    allMetricInstances[id] = this;
}

metricInstance::~metricInstance() {
    for(unsigned i=0; i < components.size(); i++){
        delete (components[i]);    
    }
    for(unsigned j=0; j < parts.size(); j++){
        delete (parts[j]);    
    }
    for(unsigned k=0; k < old_data.size(); k++){
        delete (old_data[k]);    
    }
    if (data) delete(data);
    if (global_data) delete(global_data);
    nextId[id] = false;
    // remove metricInstace from list of allMetricsInstances
    allMetricInstances.undef(id);
}

int metricInstance::getArchiveValues(sampleValue *buckets,int numOfBuckets,
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

int metricInstance::getSampleValues(sampleValue *buckets,int numOfBuckets,
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

metricInstance *metricInstance::getMI(metricInstanceHandle mh){

    if(allMetricInstances.defines(mh)){
        return(allMetricInstances[mh]);
    }
    return 0;
}

// TODO: remove asserts
void metricInstance::dataDisable(){
    
    assert(!users.size());
    assert(!global_users.size());
    for(unsigned i=0; i < components.size(); i++){
        delete (components[i]);  // this disables data collection  
    }
    components.resize(0);
    // deleteing components deletes parts as well
    // for(unsigned j=0; j < parts.size(); j++){
    //   delete (parts[j]);    
    // } 
    parts.resize(0);
    enabled = false;
    // if data is persistent this must be cleared 
    sample.firstSampleReceived = false;  
    assert(!components.size());
    assert(!parts.size());
}

void metricInstance::removeCurrUser(perfStreamHandle ps){

    // remove ps from vector of users
    unsigned size = users.size();
    for(unsigned i=0; i < size; i++){
	if(users[i] == ps){
	    users[i] = users[size-1];
	    users.resize(size-1);
	    assert(users.size() < size);
	    // decrease ps's data buffer size
	    performanceStream::removeCurrentUser(ps);
	    return;
    } }
}

void metricInstance::removeGlobalUser(perfStreamHandle ps){

    // remove ps from vector of users
    unsigned size = global_users.size();
    for(unsigned i=0; i < size; i++){
	if(global_users[i] == ps){
	    global_users[i] = global_users[size-1];
	    global_users.resize(size-1);
	    assert(global_users.size() < size);
	    // decrease ps's data buffer size
	    performanceStream::removeGlobalUser(ps);
	    return;
    } }
}

// returns true if histogram really was deleted
bool metricInstance::deleteCurrHistogram(){

    // if curr histogram exists and ther are no users delete
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

bool metricInstance::addComponent(component *new_comp){

    paradynDaemon *new_daemon =  new_comp->getDaemon();
    for (unsigned i=0; i < components.size(); i++){
         if((components[i])->getDaemon() == new_daemon) return false;
    }
    components += new_comp;
    return true;
}

bool metricInstance::addPart(sampleInfo *new_part){
    
    parts += new_part;
    return true;
}

// stops currentPhase data collection for all metricInstances
void metricInstance::stopAllCurrentDataCollection(phaseHandle last_phase_id) {

    dictionary_hash_iter<metricInstanceHandle,metricInstance *> 
			allMI(allMetricInstances);
    metricInstanceHandle handle;
    metricInstance *mi;
 
    vector<metricInstance *> remove_list;
    allMI.reset();
    while(allMI.next(handle,mi)){
	remove_list += mi;
    }
     
    assert(remove_list.size() == allMetricInstances.size());
    for(unsigned i=0; i < remove_list.size(); i++){
	mi = remove_list[i];
        // remove all users from user list
	mi->users.resize(0);
	assert(!(mi->users.size()));

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
	    metric *m = metric::getMetric(mi->met);
	    mi->newCurrDataCollection(m->getStyle(),
				      histDataCallBack,
				      histFoldCallBack);
	}
    }
    // reduce each performance stream's buffer size by the number
    // of current MI's it will no longer be receiving data for
    performanceStream::removeAllCurrUsers();
}


void metricInstance::addCurrentUser(perfStreamHandle p) {

    for(unsigned i=0; i < users.size(); i++){
        if(users[i] == p) return;
    }
    users += p;
    assert(users.size());
    // update buffersize for perfStream
    performanceStream::addCurrentUser(p);
}

void metricInstance::addGlobalUser(perfStreamHandle p) {

    for(unsigned i=0; i < global_users.size(); i++){
        if(global_users[i] == p) return;
    }
    global_users += p;
    assert(global_users.size());
    // update buffersize for perfStream
    performanceStream::addGlobalUser(p);
}

void metricInstance::newGlobalDataCollection(metricStyle style, 
					     dataCallBack dcb, 
					     foldCallBack fcb) {

    // histogram has already been created
    if (global_data) {
	global_data->setActive();
	global_data->clearFoldOnInactive();
	return;  
    }
    // call constructor for start time 0.0 
    global_data = new Histogram(style, dcb, fcb, this);
}

void metricInstance::newCurrDataCollection(metricStyle style, 
					   dataCallBack dcb, 
					   foldCallBack fcb) {

    // histogram has already been created
    if (data) {
	data->setActive();
	data->clearFoldOnInactive();
        return;  
    }
    // create new histogram
    timeStamp start_time = phaseInfo::GetLastPhaseStart();
    if(start_time == 0.0) {
        data = new Histogram(style, dcb, fcb, this);

    }
    else {
        data = new Histogram(start_time, style, dcb, fcb, this);
    }
}

