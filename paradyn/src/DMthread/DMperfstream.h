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
#include "paradyn/src/DMthread/BufferPool.h"
#include "DMphase.h"
#include "DMinclude.h"
#include "paradyn/src/DMthread/DVbufferpool.h"


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
	void callSampleFunc(metricInstanceHandle,
			    sampleValue*, int, int, phaseType);
	void callResourceFunc(resourceHandle parent, resourceHandle child, 
			      const char *name, const char *abstr);
	void callResourceBatchFunc(batchMode mode);
	void callFoldFunc(timeStamp width,phaseType phase_type);
	void callStateFunc(appState state);
	void callPhaseFunc(phaseInfo& phase);
	perfStreamHandle Handle(){return(handle);}
	void flushBuffer();   // send data to client thread
	static void notifyAllChange(appState state);
	static void ResourceBatchMode(batchMode mode);
	static void foldAll(timeStamp width, phaseType phase_type); 
	static performanceStream *find(perfStreamHandle psh);

	// these routines change the size of my_buffer 
	static void addCurrentUser(perfStreamHandle psh);
	static void addGlobalUser(perfStreamHandle psh);
	static void removeCurrentUser(perfStreamHandle psh);
	static void removeGlobalUser(perfStreamHandle psh);
	static void removeAllCurrUsers();

	// send data to client thread
	// static flushBuffer(perfStreamHandle psh);

	static unsigned pshash(const perfStreamHandle &val) {
		    return((unsigned)val);
	}
    private:
	dataType                type;   // Trace or Sample
	dataCallback            dataFunc;
	controlCallback         controlFunc;
	int 			threadId;
	perfStreamHandle	handle;
	u_int 			num_global_mis;  // num MI's for global phase
	u_int 			num_curr_mis;    // num MI's for curr phase
	u_int			my_buffer_size;  // total number of MI's enabled
 	u_int			next_buffer_loc;  // next buffer loc. to fill
	vector<dataValueType>	*my_buffer;	// buffer of dataValues
	static u_int 		next_id;
	// dictionary rather than vector since perfStreams can be destroyed
	static dictionary_hash<perfStreamHandle,performanceStream*> allStreams;
	bool			reallocBuffer();
};
#endif
