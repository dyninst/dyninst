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


metric::metric(T_dyninstRPC::metricInfo i){

    if(allMetrics.defines(i.name)) return;
    info.style = i.style;
    info.units = i.units;
    info.name = i.name;
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

vector<string> *metric::allMetricNames(){

    vector<string> *temp = new vector<string>;
    string name;
    for(unsigned i=0; i < metrics.size(); i++){
        metric *met = metrics[i];
        string name = met->getName();
	*temp += name; 

    }
    return(temp);
    temp = 0;
}

vector<met_name_id> *metric::allMetricNamesIds(){

    vector<met_name_id> *temp = new vector<met_name_id>;
    met_name_id next;
    for(unsigned i=0; i < metrics.size(); i++){
        metric *met = metrics[i];
        next.name = met->getName();
	next.id = met->getHandle();
	*temp += next; 
    }
    return(temp);
    temp = 0;
}

metricInstance::metricInstance(resourceListHandle rl, metricHandle m) {
    met = m;
    focus = rl;
    enabledTime = 0.0;
    metric *mp = metric::getMetric(m);
    sample.aggOp = mp->getAggregate();
    data = NULL;

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
    if (data) delete(data);
    nextId[id] = false;
    // remove metricInstace from list of allMetricsInstances
    allMetricInstances.undef(id);
}

metricInstance *metricInstance::getMI(metricInstanceHandle mh){

    if(allMetricInstances.defines(mh)){
        return(allMetricInstances[mh]);
    }
    return 0;
}

void metricInstance::disableDataCollection(perfStreamHandle ps){

    // remove ps from vector of users
    unsigned size = users.size();
    for(unsigned i=0; i < size; i++){
	if(users[i] == ps){
	    users[i] = users[size-1];
	    users.resize(size-1);
	    assert(users.size() < size);
	    return;
    } }
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

void metricInstance::addUser(perfStreamHandle p){

    for(unsigned i=0; i < users.size(); i++){
        if(users[i] == p) return;
    }
    users += p;
    assert(users.size());
}
