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
#ifndef dmperfstream_H
#define dmperfstream_H
#include "util/h/sys.h"
#include "dataManager.thread.h"
#include "dataManager.thread.SRVR.h"
#include "util/h/aggregateSample.h"
#include <string.h>
#include "paradyn/src/UIthread/Status.h"
#include <stdlib.h>
#include "util/h/Vector.h"
#include "util/h/Dictionary.h"
#include "DMphase.h"
#include "DMinclude.h"


class metricInstance;
class metric;
class resourceList;
//
// A consumer of performance data.
//
class performanceStream {
	friend class paradynDaemon;
	friend void phaseInfo::startPhase(timeStamp, const string&);
	friend void addMetric(T_dyninstRPC::metricInfo &info);
	friend resourceHandle createResource(vector<string>&, string& );
    public:
	performanceStream(dataType t, dataCallback dc,
			  controlCallback cc, int tid); 
	~performanceStream();

	// TODO remove this
	void setSampleRate(timeStamp rate) { sampleRate = rate; }

	void callSampleFunc(metricInstanceHandle, sampleValue*, int, int);
	void callResourceFunc(resourceHandle parent, resourceHandle child, 
			      const char *name, const char *abstr);
	void callResourceBatchFunc(batchMode mode);
	void callFoldFunc(timeStamp width);
	void callStateFunc(appState state);
	void callPhaseFunc(phaseInfo& phase);
	perfStreamHandle Handle(){return(handle);}
	static void notifyAllChange(appState state);
	static void ResourceBatchMode(batchMode mode);
	static void foldAll(timeStamp width); 
	static performanceStream *find(perfStreamHandle psh);
	static unsigned pshash(const perfStreamHandle &val) {
		    return((unsigned)val);
	}
    private:
	dataType                type;   // Trace or Sample
	dataCallback            dataFunc;
	controlCallback         controlFunc;
	int 			threadId;
	perfStreamHandle	handle;
	static vector<bool>     nextId;
	// dictionary rather than vector since perfStreams can be destroyed
	static dictionary_hash<perfStreamHandle,performanceStream*> allStreams;

	// TODO these should go
	timeStamp               sampleRate;     /* sample sampleRate usec */

};
#endif
