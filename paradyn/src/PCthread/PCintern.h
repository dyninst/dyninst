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
 * Revision 1.1  1996/02/02 02:07:28  karavan
 * A baby Performance Consultant is born!
 *
 */

#ifndef PC_INTERN_H
#define PC_INTERN_H

#include "/p/paradyn/core/util/h/sys.h"
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
extern int PCnumActiveExperiments;

// display styles for search GUI
#define INACTIVEUNKNOWNNODESTYLE 1
#define ACTIVEUNKNOWNNODESTYLE 4
#define ACTIVETRUENODESTYLE 3
#define ACTIVEFALSENODESTYLE 5
#define INACTIVETRUENODESTYLE 6
#define INACTIVEFALSENODESTYLE 2
#define WHEREEDGESTYLE 1
#define WHYEDGESTYLE 2
#define WHENEDGESTYLE 3

#endif
