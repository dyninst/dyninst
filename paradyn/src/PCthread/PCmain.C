
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
/* Revision 1.41  1996/02/02 02:06:38  karavan
/* A baby Performance Consultant is born!
/*
 */

#include <assert.h>
#include <stdlib.h>

#include "PCintern.h"
#include "PCsearch.h"
#include "PCfilter.h"

extern thread_t MAINtid;
extern void initPCconstants();

// ** note this homeless function moved here from PCauto.C
// 25% to start
bool predictedCostLimitValidChecker(float newVal) {
   // checker function for the tunable constant "predictedCostLimit",
   // whose declaration has moved to pdMain/main.C
   if (newVal < 0.0)
      return false; // no good
   else
      return true; // okay
}

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
    search->changeBinSize(newWidth);
  }
}

// all new data on any active search arrives via this upcall from the 
// data manager.  Data arrival is what triggers experiment evaluation 
// and search updates.
void PCnewData(metricInstanceHandle m_handle,
	       int bucketNum,
	       sampleValue value,
	       phaseType phase_type)
{
#ifdef PCDEBUG
  tunableBooleanConstant pcEvalPrint = 
    tunableConstantRegistry::findBoolTunableConstant("PCprintDataTrace");
  if (pcEvalPrint.getValue()) {
    const char *metname = dataMgr->getMetricNameFromMI(m_handle);
    const char *focname = dataMgr->getFocusNameFromMI(m_handle);
    cout << "AR: " << metname << " " << focname;
    cout << " value: " << value;
    cout << " bin: " << bucketNum << endl;
  }
#endif

  PCsearch *search = PCsearch::findSearch (phase_type);
  assert (search);
  search-> newData(m_handle, value, bucketNum);
}

void PCnewDataCallback(vector<dataValueType> *values,
		       u_int num_values) {

    if (values->size() < num_values) num_values = values->size();
    for(unsigned i=0; i < num_values;i++){
	PCnewData((*values)[i].mi,
	    	  (*values)[i].bucketNum,
		  (*values)[i].value,
    		  (*values)[i].type);
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
	      timeStamp end, 
	      float bucketwidth)
{
  // Guard here against case that no search has ever been initialized, 
  // in which case we don't want to do anything.
  if (PChyposDefined) {
    PCsearch::updateCurrentPhase();
    tunableBooleanConstant pcEvalPrint = 
      tunableConstantRegistry::findBoolTunableConstant("pcEvalPrint");
    if (pcEvalPrint.getValue()) {
      cout << "NEWPH: " << phase << ":" << name << "from:" << begin 
	<< " to:" << end
	  << " width: " << bucketwidth << endl;
    }
  }
}

void PCmain(void* varg)
{
    int arg; memcpy((void *) &arg, varg, sizeof arg);

    int from;
    unsigned int tag;
    performanceConsultant *pc;
    char PCbuff[64];
    unsigned int msgSize = 64;

    // define all tunable constants used by hypotheses
    // tunable constants must be defined here in the sequential section
    // of the code, or values specified in pcl files won't be handled 
    // properly.
    // Remaining initialization is application- and/or phase-specific and
    // is done after the user requests a search.

    tunableFloatConstantDeclarator pcl 
      ("predictedCostLimit",
       "Max. allowable perturbation of the application.",
       20.0, // initial value
       predictedCostLimitValidChecker, // validation function 
       NULL, // callback routine
       userConstant);

    tunableFloatConstantDeclarator hysRange
      ("hysteresisRange",
       "Fraction above and below threshold that a test should use.",
       0.15, // initial
       0.0, // min
       1.0, // max
       NULL, // callback
       developerConstant);

    tunableFloatConstantDeclarator minObsTime
      ("minObservationTime",
       "min. time (in seconds) to wait after changing inst to start try hypotheses.",
       1.0, // initial
       0.0, // min
       60.0, // max
       NULL, // callback
       userConstant);

    tunableFloatConstantDeclarator sufficientTime
      ("sufficientTime",
       "How long to wait (in seconds) before we can conclude a hypothesis is false.",
       6.0, // initial
       0.0, // min
       1000.0, // max
       NULL,
       userConstant);

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
