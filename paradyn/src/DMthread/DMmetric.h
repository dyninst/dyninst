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
#ifndef dmmetric_H
#define dmmetric_H
#include "util/h/sys.h"
#include "util/h/hist.h"
#include "dataManager.thread.h"
#include "dataManager.thread.SRVR.h"
#include <string.h>
#include "paradyn/src/UIthread/Status.h"
#include <stdlib.h>
#include "util/h/Vector.h"
#include "util/h/Dictionary.h"
#include "util/h/String.h"
#include "DMinclude.h"
#include "DMdaemon.h"


class metricInstance;

// a part of an mi.
//
class component {
    friend class metricInstance;
    friend class paradynDaemon;
    public:
	component(paradynDaemon *d, int i, metricInstance *mi) {
	    daemon = d;
	    id = i;
            // Is this add unique?
            assert(i >= 0);
	    d->activeMids[(unsigned)id] = mi;
	}
	~component() {
	    daemon->disableDataCollection(id);
            assert(id>=0);
            daemon->disabledMids += (unsigned) id;
            daemon->activeMids.undef((unsigned)id);
	}
    private:
	sampleInfo sample;
	paradynDaemon *daemon;
	int id;
};


//
//  info about a metric in the system
//
class metric {
    friend class dataManager;
    friend class metricInstance;
    friend class paradynDaemon;
    friend void addMetric(T_dyninstRPC::metricInfo &info);
    friend void histDataCallBack(sampleValue *buckets, int count, 
				 int first, void *arg);
    // TODO: remove these when PC is re-written ***************
    friend void PCmetricFunc(perfStreamHandle, const char *name,
			     int style, int aggregate,
			     const char *units, metricHandle m_handle);
    friend void PCmain(void* varg);
    friend class PCmetric;
    friend void PCnewData(perfStreamHandle,metricInstanceHandle,
			  sampleValue*,int,int);
    // ********************************************************
    public:
	metric(T_dyninstRPC::metricInfo i); 
	const T_dyninstRPC::metricInfo  *getInfo() { return(&info); }
	const char *getName() { return((info.name.string_of()));}
	const char *getUnits() { return((info.units.string_of()));}
	metricHandle getHandle() { return(info.handle);}
	metricStyle  getStyle() { return((metricStyle) info.style); }
        int   getAggregate() { return info.aggregate;}

	static unsigned  size(){return(metrics.size());}
	static const T_dyninstRPC::metricInfo  *getInfo(metricHandle handle);
	static const char *getName(metricHandle handle);
        static const metricHandle  *find(const string name); 
	static vector<string> *allMetricNames();
	static vector<met_name_id> *allMetricNamesIds();

    private:
	static dictionary_hash<string, metric*> allMetrics;
	static vector<metric*> metrics;  // indexed by metric id
	T_dyninstRPC::metricInfo info;

        static metric  *getMetric(metricHandle iName); 
};


class metricInstance {
    friend class dataManager;
    friend class metric;
    friend class paradynDaemon;
    friend void histDataCallBack(sampleValue *buckets, int count, 
				 int first, void *arg);
    // TODO: remove these when PC is re-written ***************
    friend void PCnewData(perfStreamHandle,metricInstanceHandle,
			  sampleValue,int,int);
    friend class datum;
    friend class PCmetric;
    friend void phaseInfo::startPhase(timeStamp start_Time, const string &name);
    // ********************************************************
    public:
	metricInstance(resourceListHandle rl, metricHandle m); 
	~metricInstance(); 
	float getValue() {
	    float ret;
	    if (!data) abort();
	    ret = data->getValue();
	    ret /= enabledTime;
	    return(ret);
	}
	float getTotValue() {
	    float ret;

	    if (!data) abort();
	    ret = data->getValue();
	    return(ret);
	}
	int getCount(){return(users.size());}
	int getSampleValues(sampleValue *buckets,
			    int numberOfBuckets,
			    int first){
	    if (!data) return (-1);
	    return(data->getBuckets(buckets, numberOfBuckets, first));
        }
        static unsigned  mhash(const metricInstanceHandle &val){
	    return((unsigned)val);
	}
	metricInstanceHandle getHandle(){ return(id); }
	metricHandle getMetricHandle(){ return(met); }
	resourceListHandle getFocusHandle(){ return(focus); }
	void addInterval(timeStamp s,timeStamp e,sampleValue v,bool b){
             data->addInterval(s,e,v,b);
	}
        void disableDataCollection(perfStreamHandle ps);
	void addUser(perfStreamHandle p); 
    private:
	metricHandle met;
	resourceListHandle focus;
	float enabledTime;
	vector<sampleInfo *> parts;
	vector<component *> components;
	vector<perfStreamHandle> users;
	Histogram *data;
	unsigned id;
	sampleInfo sample;
	static dictionary_hash<metricInstanceHandle,metricInstance *> 
	    allMetricInstances;

        // vector of ids...reuse ids of deleted metricInstances
	static vector<bool> nextId;

        static metricInstance *getMI(metricInstanceHandle);
	static metricInstance *find(metricHandle, resourceListHandle);
        void newDataCollection(metricStyle style, dataCallBack dcb,
			       foldCallBack fcb){
            data = new Histogram(style, dcb, fcb, this);
        }
};
#endif
