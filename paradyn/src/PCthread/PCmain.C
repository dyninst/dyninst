
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
/* Revision 1.35  1995/10/13 22:09:19  newhall
/* added phaseType parameter to PCnewData
/*
 * Revision 1.34  1995/10/05  04:41:43  karavan
 * changes to UI::PC interface calls.
 *
 * Revision 1.33  1995/08/08  03:13:03  newhall
 * updates due to changes in DM: newPerfData, sampleDataCallbackFunc defs.
 *
 * Revision 1.32  1995/08/05  17:09:11  krisna
 * do not include <memory.h> in C++ programs, use <stdlib.h>
 *
 * Revision 1.31  1995/08/01 02:18:20  newhall
 * changes to support phase interface
 *
 * Revision 1.30  1995/06/02  20:50:09  newhall
 * made code compatable with new DM interface
 *
 * Revision 1.29  1995/02/27  19:17:31  tamches
 * Changes to code having to do with tunable constants.
 * First, header files have moved from util lib to TCthread.
 * Second, tunable constants may no longer be declared globally.
 * Third, accessing tunable constants is different.
 *
 * Revision 1.28  1995/02/16  08:19:11  markc
 * Changed Boolean to bool
 *
 * Revision 1.27  1995/01/26  17:58:39  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.26  1994/09/30  19:18:12  rbi
 * Abstraction interface change.
 *
 * Revision 1.25  1994/09/22  01:01:08  markc
 * Cast stringHandle to char* to view as text
 *
 * Revision 1.24  1994/09/06  08:32:30  karavan
 * Better control of PC output through tunable constants.
 *
 * Revision 1.23  1994/09/05  20:01:00  jcargill
 * Better control of PC output through tunable constants.
 *
 * Revision 1.22  1994/08/22  15:55:58  markc
 * Cast stringHandles to char* for printing.
 *
 * Revision 1.21  1994/08/05  16:04:12  hollings
 * more consistant use of stringHandle vs. char *.
 *
 * Revision 1.20  1994/08/03  19:09:49  hollings
 * split tunable constant into float and boolean types
 *
 * added tunable constant for printing tests as they avaluate.
 *
 * added code to compute the min interval data has been enabled for a single
 * test rather than using a global min.  This prevents short changes from
 * altering long term trends among high level hypotheses.
 *
 * Revision 1.19  1994/07/28  22:33:59  krisna
 * proper starting code for PCmain thread
 * stringCompare matches qsort prototype
 * changed infinity() to HUGE_VAL
 *
 * Revision 1.18  1994/07/25  04:47:05  hollings
 * Added histogram to PCmetric so we only use data for minimum interval
 * that all metrics for a current batch of requests has been enabled.
 *
 * added hypothsis to deal with the procedure level data correctly in
 * CPU bound programs.
 *
 * changed inst hypothesis to use observed cost metric not old procedure
 * call based one.
 *
 * Revision 1.17  1994/06/27  21:24:39  rbi
 * New abstraction parameter for performance streams
 *
 * Revision 1.16  1994/06/27  18:55:08  hollings
 * Added compiler flag to add SHG nodes to dag only on first evaluation.
 *
 * Revision 1.15  1994/06/22  22:58:19  hollings
 * Compiler warnings and copyrights.
 *
 * Revision 1.14  1994/06/17  00:12:28  hollings
 * fixed the init of the control callback structure.
 *
 * Revision 1.13  1994/06/12  22:40:49  karavan
 * changed printf's to calls to status display service.
 *
 * Revision 1.12  1994/05/18  00:48:53  hollings
 * Major changes in the notion of time to wait for a hypothesis.  We now wait
 * until the earlyestLastSample for a metrics used by a hypothesis is at
 * least sufficient observation time after the change was made.
 *
 * Revision 1.11  1994/05/12  23:34:06  hollings
 * made path to paradyn.h relative.
 *
 * Revision 1.10  1994/05/10  03:57:43  hollings
 * Changed data upcall to return array of buckets.
 *
 * Revision 1.9  1994/05/06  06:39:34  karavan
 * SHG now initialized only upon request
 *
 * Revision 1.8  1994/05/02  20:38:10  hollings
 * added pause search mode, and cleanedup global variable naming.
 *
 * Revision 1.7  1994/04/21  05:00:10  karavan
 * added global SHGid for visual display.
 *
 * Revision 1.6  1994/04/06  21:24:10  markc
 * First log message.
 * */

#include <assert.h>
#include <stdlib.h>


#include "thread/h/thread.h"
#include "../TCthread/tunableConst.h"
#include "dataManager.thread.CLNT.h"
#include "performanceConsultant.thread.SRVR.h"
#include "UI.thread.CLNT.h"
#include "PCglobals.h"
#include "PCmetric.h"
#include "../src/pdMain/paradyn.h"
#include "../src/DMthread/DMresource.h"

#include "../src/DMthread/DMinclude.h"

// TEMP until remove all ptrs from interface then include DMinclue.h
#include "paradyn/src/DMthread/DMmetric.h"
#include "paradyn/src/DMthread/DMresource.h"
// ***********************************


perfStreamHandle pc_ps_handle;
extern void initResources();
extern void PCevaluateWorld();
extern thread_t MAINtid;
extern timeStamp PCstartTransTime;
extern timeStamp PCendTransTime;

int SHGid;             // id needed for Search History Graph uim dag calls
static float PCbucketWidth;

void PCfold(perfStreamHandle handle,
	    timeStamp newWidth,
	    phaseType phase_type)
{
    if(phase_type == GlobalPhase) 
        PCbucketWidth = newWidth;
}

void PCnewData(perfStreamHandle handle,
	       metricInstanceHandle m_handle,
	       int bucketNumber,
	       sampleValue value,
	       phaseType type)
{
    // TODO: this should be removed and PC thread should not be accessing
    // metricInstance objects directly
    metricInstance *mi = metricInstance::getMI(m_handle);
    if(!mi)  return;

    sampleValue total = value*PCbucketWidth;

    datum *dp = miToDatumMap.find(mi);
    assert(dp);
    timeStamp start = PCbucketWidth * bucketNumber;
    timeStamp end = PCbucketWidth * (bucketNumber + 1);
#ifdef n_def
    if((bucketNumber % 50) == 0){
        cout << "PCdata:  bucketNumber:  " << bucketNumber << endl;
	cout << "    width:         " << PCbucketWidth << endl;
	cout << "    start:         " << start << endl;
	cout << "    end:           " << end << endl;
        cout << "    PCcurrentTime: " << PCcurrentTime << endl;
        cout << "    PCstartTransTime: " << PCstartTransTime << endl;
        cout << "    PCendTransTime: " << PCendTransTime << endl;
    }
#endif

  tunableBooleanConstant pcEvalPrint = tunableConstantRegistry::findBoolTunableConstant("pcEvalPrint");
  if (pcEvalPrint.getValue()) {
	cout << "AR: " << (char*)dp->metName << (char*)dp->resList->getName();
	cout << " = " << total;
	cout << " from " << start << " to " << end << "\n";
  }

    dp->newSample(start, end, total);

    // see if we should check for new state change.
    //  we wait until time moves, otherwise we evaluate hypotheses too often.
    if (end > PCcurrentTime) {
	PCcurrentTime = end;
	if (PCstartTransTime < PCendTransTime) {
	    PCevaluateWorld();
	}
    }
}

void PCnewInfo()
{
}

void PCmetricFunc(perfStreamHandle handle, 
		  const char *name,
		  int style,
		  int aggregate,
		  const char *units,
		  metricHandle m_handle)
{
    PCmetric *pcMet;
    stringHandle s_name;
    extern stringPool PCmetricStrings;

    // TODO: this should be removed and PC thread should not be accessing
    // metric objects directly
    metric *met = metric::getMetric(m_handle);
    if(!met) return;

    char *newName = strdup(name);
    s_name = PCmetricStrings.findAndAdd(newName);
    pcMet = (PCmetric *) allMetrics.find((char *)s_name);
    if (!pcMet) {
	// This warning was intended to make it easy to catch typos in metric
	//   names between paradynd and the PC.  However, there are now several
	//   metrics that paradynd defines that the PC doesn't need.
	//   - jkh 6/25/94
        // printf("WARNING performance consultant has no use for %s\n", name);
        pcMet = new PCmetric((char*)s_name);
    }
    pcMet->met = met;
}

void PCmain(void* varg)
{
    int arg; memcpy((void *) &arg, varg, sizeof arg);

    int i;
    int from;
    unsigned int tag;
    performanceConsultant *pc;
    union dataCallback dataHandlers;
    struct controlCallback controlHandlers;
    char PCbuff[64];
    unsigned int msgSize = 64;

    thr_name("PerformanceConsultant");
    // ??? do inits and waits.

    pc = new performanceConsultant(MAINtid);

    msg_send (MAINtid, MSG_TAG_PC_READY, (char *) NULL, 0);
    tag = MSG_TAG_ALL_CHILDREN_READY;
    msg_recv (&tag, PCbuff, &msgSize);
    // why are static resources initialized here?
    initResources();

    // make sure memory is clear.
    memset(&controlHandlers, '\0', sizeof(controlHandlers));
    controlHandlers.mFunc = PCmetricFunc;
    controlHandlers.fFunc = PCfold;

    dataHandlers.sample = PCnewData;
    pc_ps_handle = dataMgr->createPerformanceStream(Sample,
	dataHandlers, controlHandlers);

    PCbucketWidth = dataMgr->getCurrentBucketWidth();

    // now find about existing metrics.
    vector<string> *mets = dataMgr->getAvailableMetrics();
    for (i=0; i < mets->size(); i++) {
	metricHandle *m_handle = dataMgr->findMetric((*mets)[i].string_of());
	if (m_handle) {
	    metric *met = metric::getMetric(*m_handle);
	    if(met) 
		PCmetricFunc(pc_ps_handle, 
			     met->getName(),
			     met->getStyle(),
			     met->getAggregate(),
			     met->getUnits(),
			     met->getHandle());
        }
    }

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
