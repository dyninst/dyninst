
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

/* $Log: PCmain.C,v $
/* Revision 1.48  1996/03/05 16:13:15  naim
/* Minor changes for debugging purposes - naim
/*
 * Revision 1.47  1996/02/22  18:30:56  karavan
 * removed some debugging printing
 *
 * Revision 1.46  1996/02/12 20:00:53  karavan
 * part one of change to newDataCallback, streamlining here since this is
 * the critical path for the PC.
 *
 * Revision 1.45  1996/02/09 20:57:30  karavan
 * Added performanceConsultant::globalRawDataServer and
 * performanceConsultant::currentRawDataServer to streamline new data
 * storage.
 *
 * Revision 1.44  1996/02/09 05:30:51  karavan
 * changes to support multiple per phase searching.
 *
 * Revision 1.43  1996/02/08 19:52:43  karavan
 * changed performance consultant's use of tunable constants:  added 3 new
 * user-level TC's, PC_CPUThreshold, PC_IOThreshold, PC_SyncThreshold, which
 * are used for all hypotheses for the respective categories.  Also added
 * PC_useIndividualThresholds, which switches thresholds back to use hypothesis-
 * specific, rather than categorical, thresholds.
 *
 * Moved all TC initialization to PCconstants.C.
 *
 * Switched over to callbacks for TC value updates.
 *
 * Revision 1.42  1996/02/05 18:51:35  newhall
 * Change to DM interface: StartPhase and newPhaseCallback
 *
 * Revision 1.41  1996/02/02  02:06:38  karavan
 * A baby Performance Consultant is born!
 *
 */

#include <assert.h>
#include <stdlib.h>

#include "PCintern.h"
#include "PCsearch.h"
#include "PCfilter.h"

#ifdef PCDEBUG
FILE *TESTfp;
#endif

extern thread_t MAINtid;
extern void initPCconstants();
float performanceConsultant::hysteresisRange = 0.0;
float performanceConsultant::predictedCostLimit = 0.0;
float performanceConsultant::minObservationTime = 0.0;
float performanceConsultant::sufficientTime = 0.0;
bool performanceConsultant::printSearchChanges = false;   
bool performanceConsultant::printDataCollection = false;   
bool performanceConsultant::printTestResults  = false;  
bool performanceConsultant::printDataTrace = false;   
bool performanceConsultant::useIndividualThresholds  = false;
// 0 means no current phase defined  
unsigned performanceConsultant::currentPhase = 0; 
unsigned performanceConsultant::DMcurrentPhaseToken = 0;
filteredDataServer *performanceConsultant::globalRawDataServer = NULL;
filteredDataServer *performanceConsultant::currentRawDataServer = NULL;

// filteredDataServers use the bin width to interpret performance stream data 
// so we need to pass fold notification to the appropriate server
void PCfold(perfStreamHandle,
	    timeStamp newWidth,
	    phaseType phase_type)
{
  // this callback may be invoked before we've initialized any searches, 
  // in which case we don't want to do anything at all.  (bin size is 
  // properly initialized when each search is created.)   
  if (PChyposDefined) { 
    PCsearch *search = PCsearch::findSearch (phase_type);
    if (search)
      search->changeBinSize(newWidth);
  }
}

//
// all new data arrives at the PC thread via this upcall
//
void PCnewDataCallback(vector<dataValueType> *values,
		       u_int num_values) 
{
    if (values->size() < num_values) num_values = values->size();
    dataValueType *curr;
//**
//    cout << "AR: new data CB NUM VALUES = " << num_values << endl;
    for(unsigned i=0; i < num_values;i++){
      curr = &((*values)[i]);
#ifdef PCDEBUG
      if (performanceConsultant::printDataTrace) {
	const char *metname = dataMgr->getMetricNameFromMI((*values)[i].mi);
	const char *focname = dataMgr->getFocusNameFromMI((*values)[i].mi);
	cout << "AR: " << metname << " " << focname;
	cout << " value: " << (*values)[i].value;
	cout << " bin: " << (*values)[i].bucketNum << " " << (*values)[i].type
	  << endl;
      }
#endif
/*
      filteredDataServer *rawInput;
      if (curr->type == GlobalPhase)
	rawInput = performanceConsultant::globalRawDataServer;
      else
	rawInput = performanceConsultant::currentRawDataServer;
      assert (rawInput);
      rawInput-> newData(curr->mi, curr->value, curr->bucketNum);      
*/
      PCsearch *search = PCsearch::findSearch (curr->type);
      assert (search);
      search-> newData(curr->mi, curr->value, curr->bucketNum);

    }
    // dealloc buffer space
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
	      timeStamp begin, 
	      timeStamp,             // for future use only
	      float bucketwidth,
	      bool, bool)            // used by UI only
{
#ifdef PCDEBUG
  if (performanceConsultant::printSearchChanges) {
    cout << "NEWPH: " << phase << ":" << name << " started at:" << begin 
	<< " width: " << bucketwidth << endl;
  }
#endif
  // Guard here against case that no search has ever been initialized, 
  // in which case we don't want to do anything.
  if (PChyposDefined) {
    PCsearch::updateCurrentPhase(begin);
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
}

void PCmain(void* varg)
{
    int arg; memcpy((void *) &arg, varg, sizeof arg);

    int from;
    unsigned int tag;
    performanceConsultant *pc;
    char PCbuff[64];
    unsigned int msgSize = 64;

#ifdef PCDEBUG
    TESTfp = fopen("TESTresult.out","w");
#endif

    // define all tunable constants used by the performance Consultant
    // tunable constants must be defined here in the sequential section
    // of the code, or values specified in pcl files won't be handled 
    // properly.
    initPCconstants();

    // thread startup
    thr_name("PerformanceConsultant");
    pc = new performanceConsultant(MAINtid);
    msg_send (MAINtid, MSG_TAG_PC_READY, (char *) NULL, 0);
    tag = MSG_TAG_ALL_CHILDREN_READY;
    msg_recv (&tag, PCbuff, &msgSize);

    // register performance stream with data manager
    union dataCallback dataHandlers;
    struct controlCallback controlHandlers;
    memset(&controlHandlers, '\0', sizeof(controlHandlers));
    controlHandlers.fFunc = PCfold;
    controlHandlers.pFunc = PCphase;
    dataHandlers.sample = PCnewDataCallback;
    filteredDataServer::initPStoken(dataMgr->createPerformanceStream(Sample,
					       dataHandlers, controlHandlers));

    // Note: remaining initialization is application- and/or phase-specific and
    // is done after the user requests a search.

    while (1) {
	tag = MSG_TAG_ANY;
	from = msg_poll(&tag, true);
	assert(from != THR_ERR);
	if (dataMgr->isValidTag((T_dataManager::message_tags)tag)) {
	  if (dataMgr->waitLoop(true, (T_dataManager::message_tags)tag) ==
	      T_dataManager::error) {
	    // handle error
	    // TODO
	    cerr << "Error in PCmain.C, needs to be handled\n";
	    assert(0);
	  }
	} else if (pc->isValidTag((T_performanceConsultant::message_tags)tag)) {
	  if (pc->waitLoop(true, (T_performanceConsultant::message_tags)tag) ==
	      T_performanceConsultant::error) {
	    // handle error
	    // TODO
	    cerr << "Error in PCmain.C, needs to be handled\n";
	    assert(0);
	  }
	} else {
	  // TODO
	  cerr << "Message sent that is not recognized in PCmain.C\n";
	  assert(0);
	}
   }
}
