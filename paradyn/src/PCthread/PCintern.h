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

/*
 * PCintern.h
 * 
 * Included by PC modules only
 *  
 * $Log: PCintern.h,v $
 * Revision 1.5  1996/04/07 21:29:33  karavan
 * split up search ready queue into two, one global one current, and moved to
 * round robin queue removal.
 *
 * eliminated startSearch(), combined functionality into activateSearch().  All
 * search requests are for a specific phase id.
 *
 * changed dataMgr->enableDataCollection2 to take phaseID argument, with needed
 * changes internal to PC to track phaseID, to avoid enable requests being handled
 * for incorrect current phase.
 *
 * added update of display when phase ends, so all nodes changed to inactive display
 * style.
 *
 * Revision 1.4  1996/03/18 07:13:02  karavan
 * Switched over to cost model for controlling extent of search.
 *
 * Added new TC PCcollectInstrTimings.
 *
 * Revision 1.3  1996/02/22 18:31:18  karavan
 * changed GUI node styles from #defines to enum
 *
 * Revision 1.2  1996/02/12 08:23:47  karavan
 * eliminated (ugh) full pathname from include
 *
 * Revision 1.1  1996/02/02 02:07:28  karavan
 * A baby Performance Consultant is born!
 *
 */

#ifndef PC_INTERN_H
#define PC_INTERN_H

#include <iostream.h>
#include "util/h/sys.h"
//sys.h defines the following:
//  typedef double timeStamp;
//  typedef float sampleValue;
//  typedef struct {
//     timeStamp start;
//     timeStamp end;
//      sampleValue value;
//  } Interval;
#define PCdataQSize 20
typedef struct Interval Interval;
ostream& operator <<(ostream &os, Interval &i);

#include "../pdMain/paradyn.h"
#include "util/h/list.h"
#include "util/h/PriorityQueue.h"
#include "thread/h/thread.h"
#include "dataManager.thread.CLNT.h"
#include "UI.thread.CLNT.h"
#include "../DMthread/DMinclude.h"
#include "../TCthread/tunableConst.h"
#include "performanceConsultant.thread.SRVR.h"

typedef resourceListHandle focus;
typedef metricInstanceHandle PCmetDataID;
class experiment;
typedef experiment* PCmetSubscriber;
typedef enum testResult {tfalse, ttrue, tunknown}; 


// known or "base" resources -- these don't vary across applications
extern resourceHandle rootResource;
extern resourceHandle SyncObject;
extern resourceHandle Procedures;
extern resourceHandle Processes;
extern resourceHandle Machines;
extern resourceHandle Locks;
extern resourceHandle Barriers;
extern resourceHandle Semaphores;
extern resourceHandle MsgTags;

extern focus topLevelFocus;

class hypothesis;
class whyAxis;

extern whyAxis *PCWhyAxis;
extern hypothesis *const topLevelHypothesis;
extern bool PChyposDefined;
extern const unsigned GlobalPhaseID;

struct pcglobals {
  bool PChyposDefined;
  hypothesis * topLevelHypothesis;
  whyAxis *PCWhyAxis;
  focus topLevelFocus;
};

extern struct pcglobals perfConsultant;

#endif

