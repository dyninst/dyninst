/*
 * Copyright (c) 1996-1999 Barton P. Miller
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

/* $Id: PCmain.C,v 1.71 2001/06/20 20:33:40 schendel Exp $ */

#include <assert.h>
#include <stdlib.h>

#include "PCintern.h"
#include "dataManager.thread.h"
#include "PCsearch.h"
#include "PCfilter.h"
#include "PCcostServer.h"

//
// for thread initialization
//
performanceConsultant *pc;   
extern thread_t MAINtid;
// 
extern void initPCconstants();
const unsigned GlobalPhaseID = 0;


//
// pc thread globals
//
float performanceConsultant::hysteresisRange = 0.0;
float performanceConsultant::predictedCostLimit = 0.0;
timeLength performanceConsultant::minObservationTime = timeLength::Zero();
timeLength performanceConsultant::sufficientTime = timeLength::Zero();
bool performanceConsultant::printSearchChanges = false;   
bool performanceConsultant::printDataCollection = false;   
bool performanceConsultant::printTestResults  = false;  
bool performanceConsultant::printDataTrace = false;   
bool performanceConsultant::collectInstrTimings = false;
bool performanceConsultant::useIndividualThresholds  = false;
bool performanceConsultant::useCallGraphSearch = true;
// 0 means no current phase defined  
unsigned performanceConsultant::currentPhase = 0; 
unsigned performanceConsultant::DMcurrentPhaseToken = 0;
filteredDataServer *performanceConsultant::globalRawDataServer = NULL;
filteredDataServer *performanceConsultant::currentRawDataServer = NULL;
PCmetricInstServer *performanceConsultant::globalPCMetricServer = NULL;
perfStreamHandle performanceConsultant::pstream = 0;
metricHandle performanceConsultant::normalMetric = 0;
bool performanceConsultant::PChyposDefined = false;
unsigned performanceConsultant::numMetrics = 0;
//costServer cs;


// filteredDataServers use the bin width to interpret performance stream data 
// so we need to pass fold notification to the appropriate server
void PCfold(perfStreamHandle,
	    timeLength *_newWidthPtr,
	    phaseType phase_type)
{
  timeLength newWidth = *_newWidthPtr;
  delete _newWidthPtr;
  // this callback may be invoked before we've initialized any searches, 
  // in which case we don't want to do anything at all.  (bin size is 
  // properly initialized when each search is created.)   
  filteredDataServer *rawInput;
  if (performanceConsultant::PChyposDefined) { 
    if (phase_type == GlobalPhase)
      rawInput = performanceConsultant::globalRawDataServer;
    else
      rawInput = performanceConsultant::currentRawDataServer;
    if (rawInput)
      rawInput-> newBinSize(newWidth);      
  }
}

//
// handle async reply from data manager providing predicted data cost
// for a requested metric/focus pair
//
void PCpredData(u_int tok, float f){
  costServer::newPredictedCostData(tok, f);			 
}
void PCenableDataCallback(vector<metricInstInfo> *bunchostuff,  u_int phaseID)
{
#ifdef PCDEBUG  
  cout << "PCenableDataCallback: phase" << phaseID << " ";
#endif
  filteredDataServer *rawInput = NULL;
  if (phaseID == GlobalPhase)
    rawInput = performanceConsultant::globalRawDataServer;
  else if (phaseID == performanceConsultant::currentPhase)
    rawInput = performanceConsultant::currentRawDataServer;
  if (rawInput)
    rawInput-> newDataEnabled(bunchostuff);
  delete bunchostuff;
}

//
// all new data arrives at the PC thread via this upcall
//
void PCnewDataCallback(vector<dataValueType> *values,
		       u_int num_values) 
{
#ifdef MYPCDEBUG
    timeStamp t1,t2;
    t1 = getCurrentTime();
#endif
    if (values->size() < num_values) num_values = values->size();
    dataValueType *curr;

    for(unsigned i=0; i < num_values;i++){
      curr = &((*values)[i]);

#ifdef PCDEBUG
      if (performanceConsultant::printDataCollection) {
	const char *metname = dataMgr->getMetricNameFromMI(curr->mi);
	const char *focname = dataMgr->getFocusNameFromMI(curr->mi);
	cout << "AR: " << curr->mi << " " << metname << " " << focname;
	cout << " value: " << curr->value;
	cout << " bin: " << curr->bucketNum << " " << curr->type << endl;
      }
#endif

      filteredDataServer *rawInput;
      if (curr->type == GlobalPhase)
	rawInput = performanceConsultant::globalRawDataServer;
      else
	rawInput = performanceConsultant::currentRawDataServer;
      assert (rawInput);
      rawInput-> newData(curr->mi, curr->value, curr->bucketNum, curr->type);
    }

#ifdef MYPCDEBUG
    t2 = getCurrentTime()
    if ((t2-t1) > timeLength::sec()) {
      cerr << "********** PCnewDataCallback took " << t2-t1 << "\n";
      const char *metname = dataMgr->getMetricNameFromMI(curr->mi);
      const char *focname = dataMgr->getFocusNameFromMI(curr->mi);
      if (metname && focname) 
        printf("********** metric=%s, focus=%s\n",metname,focname);
    }
#endif

    // dealloc dm buffer space
    // (leave this next line in or die a horrible slow memory leak death!) 
    datavalues_bufferpool.dealloc(values);
}

//
// If a new phase is announced by the data manager, the CurrentPhase search
// is ended, if one exists.  We don't automatically start searching the new 
// phase; the user must explicitly request a new search.
//
void PCphase (perfStreamHandle, 
	      const char *name, 
	      phaseHandle phase,
	      relTimeStamp *beginPtr, 
	      relTimeStamp *endPtr,          // for future use only
	      timeLength *bucketWidthPtr,     // we get bucketwidth later
	      bool searchFlag,       
	      bool)            // used by UI only
{
  relTimeStamp begin = *beginPtr;
  delete beginPtr;
  delete endPtr;
  delete bucketWidthPtr;
#ifdef PCDEBUG
  if (performanceConsultant::printSearchChanges) {
    cout << "NEWPH: " << phase << ":" << name << " started at:" << begin 
	<< endl;
  }
#endif
  // Guard here against case that no search has ever been initialized, 
  // in which case we don't want to do anything.
  if (performanceConsultant::PChyposDefined) {
    PCsearch::updateCurrentPhase(phase+1, begin);
  }
  // we always keep track of the current phase, cause we never know when 
  // we'll get a request to start a new current phase search.
  //
  // we can't use the dm token really, cause there's no dm token for 
  // global search, which throws our part of the universe into total 
  // confusion.  So, internally and in communication with the UI, we always
  // use dm's number plus one, and 0 for global phase.  We still need to 
  // keep track of dm number for all dm communication.
  performanceConsultant::DMcurrentPhaseToken = phase;
  //
  // notify UI of new phase 
  uiMgr->newPhaseNotification ( phase+1, name, searchFlag);
}

inline void readTag(unsigned tag)
{
  if (dataMgr->isValidTag((T_dataManager::message_tags)tag)) {
    if (dataMgr->waitLoop(true, (T_dataManager::message_tags)tag) ==
	                         T_dataManager::error) 
    {
      cerr << "Error in PCmain.C, needs to be handled\n";
      assert(0);
    }
  } else if (pc->isValidTag((T_performanceConsultant::message_tags)tag)) {
    if (pc->waitLoop(true, (T_performanceConsultant::message_tags)tag) ==
	                    T_performanceConsultant::error) 
    {
      cerr << "Error in PCmain.C, needs to be handled\n";
      assert(0);
    }
  } else {
    cerr << "Message sent that is not recognized in PCmain.C\n";
    assert(0);
  }
}

void* PCmain(void* varg)
{
    int arg; memcpy((void *) &arg, varg, sizeof arg);

    thread_t from;
    unsigned int tag;
    char PCbuff[64];
    unsigned int msgSize = 64;
	int err;


    // define all tunable constants used by the performance Consultant
    // tunable constants must be defined here in the sequential section
    // of the code, or values specified in pcl files won't be handled 
    // properly.
    initPCconstants();

    // thread startup
    thr_name("PerformanceConsultant");
    pc = new performanceConsultant(MAINtid);
    msg_send (MAINtid, MSG_TAG_PC_READY, (char *) NULL, 0);
	from = MAINtid;
    tag = MSG_TAG_ALL_CHILDREN_READY;
    msg_recv (&from, &tag, PCbuff, &msgSize);
	assert( from == MAINtid );

    // register performance stream with data manager
    union dataCallback dataHandlers;
    struct controlCallback controlHandlers;
    memset(&controlHandlers, '\0', sizeof(controlHandlers));
    controlHandlers.fFunc = PCfold;
    controlHandlers.pFunc = PCphase;

    // The PC has to register a callback routine for predictedDataCost callbacks
    // even though there is a kludge in the PC to receive the msg before the
    // callback routine is called (PCpredData will never execute).  This is 
    // to maintain consistency in how the DM handles all callback functions.
    controlHandlers.cFunc = PCpredData;

    // The PC has to register a callback routine for enableDataRequest callbacks
    // even though there is a kludge in the PC to receive the msg before the
    // callback routine is called (PCenableDataCallback will never execute).  
    controlHandlers.eFunc  = PCenableDataCallback;
    // don't ask for a signal to flush our data
    controlHandlers.flFunc= 0;

    dataHandlers.sample = PCnewDataCallback;
    // the performance stream is used to identify this thread to the 
    // data manager
    performanceConsultant::pstream = dataMgr->createPerformanceStream
      (Sample, dataHandlers, controlHandlers);

    // Note: remaining initialization is application- and/or phase-specific and
    // is done after the user requests a search.

#ifdef MYPCDEBUG
    timeStamp t1 = getCurrentTime();
    timeStamp t2;
    timeLength TIME_TO_CHECK = timeLength::sec() * 2;
#endif
    while (1) {
#ifdef MYPCDEBUG
        t2 =  getCurrentTime();
        if ((t2-t1) > TIME_TO_CHECK) {
          unsigned loopLimit, loopStart;
          for (unsigned j=1;j<=1;j++) {
            if (j==1) {
              loopStart = (unsigned)T_performanceConsultant::verify + 1;
              loopLimit = (unsigned)T_performanceConsultant::last;
            }
            else {
              loopStart = (unsigned)T_dataManager::verify + 1;
              loopLimit = (unsigned)T_dataManager::last;
            }
            for (unsigned i=loopStart;i<loopLimit;i++) {
              tag = i;
              //printf("********** waiting for tag=%d\n",tag);
			  from = THR_TID_UNSPEC;
              if (msg_poll(&from, &tag, false) != THR_ERR) {
                readTag(tag);
              }
            }
          }
          t1=TESTgetTime();
        }
        else {
			from = THR_TID_UNSPEC;
			tag = MSG_TAG_THREAD;
			err = msg_poll(&from, &tag, true);
			assert(err != THR_ERR);
			readTag(tag);
        }
#else
		from = THR_TID_UNSPEC;
		tag = MSG_TAG_THREAD;
		err = msg_poll(&from, &tag, true);
		assert(err != THR_ERR);
		readTag(tag);
#endif
    }
	return NULL;
}
