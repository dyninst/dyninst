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

/*
 * PCintern.h
 * 
 * Included by PC modules only
 *  
 * $Log: PCintern.h,v $
 * Revision 1.10  1997/03/29 02:04:41  sec
 * Changed the resource handle MsgTags to Messages
 *
 * Revision 1.9  1996/08/16 21:03:29  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.8  1996/05/06 04:35:11  karavan
 * Bug fix for asynchronous predicted cost changes.
 *
 * added new function find() to template classes dictionary_hash and
 * dictionary_lite.
 *
 * changed filteredDataServer::DataFilters to dictionary_lite
 *
 * changed normalized hypotheses to use activeProcesses:cf rather than
 * activeProcesses:tlf
 *
 * code cleanup
 *
 * Revision 1.7  1996/05/02 19:46:36  karavan
 * changed predicted data cost to be fully asynchronous within the pc.
 *
 * added predicted cost server which caches predicted cost values, minimizing
 * the number of calls to the data manager.
 *
 * added new batch version of ui->DAGconfigNode
 *
 * added hysteresis factor to cost threshold
 *
 * eliminated calls to dm->enable wherever possible
 *
 * general cleanup
 *
 * Revision 1.6  1996/04/30 06:26:53  karavan
 * change PC pause function so cost-related metric instances aren't disabled
 * if another phase is running.
 *
 * fixed bug in search node activation code.
 *
 * added change to treat activeProcesses metric differently in all PCmetrics
 * in which it is used; checks for refinement along process hierarchy and
 * if there is one, uses value "1" instead of enabling activeProcesses metric.
 *
 * changed costTracker:  we now use min of active Processes and number of
 * cpus, instead of just number of cpus; also now we average only across
 * time intervals rather than cumulative average.
 *
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
typedef enum filterType { averaging, nonfiltering}; 

// known or "base" resources -- these don't vary across applications
extern resourceHandle rootResource;
extern resourceHandle SyncObject;
extern resourceHandle Procedures;
extern resourceHandle Processes;
extern resourceHandle Machines;
extern resourceHandle Locks;
extern resourceHandle Barriers;
extern resourceHandle Semaphores;
extern resourceHandle Messages;

extern focus topLevelFocus;

class hypothesis;
class whyAxis;
class costServer;

extern whyAxis *PCWhyAxis;
extern hypothesis *const topLevelHypothesis;
extern const unsigned GlobalPhaseID;

#endif

