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
/* $Log: VISIthreadpublic.C,v $
/* Revision 1.11  1995/08/01 02:18:44  newhall
/* changes to support phase interface
/*
 * Revision 1.10  1995/06/02  20:54:36  newhall
 * made code compatable with new DM interface
 * replaced List templates  with STL templates
 *
 * Revision 1.9  1995/02/26  02:08:37  newhall
 * added some of the support for the phase interface
 * fix so that the vector of data values are being
 * correctly filled before call to BulkDataTransfer
 *
 * Revision 1.8  1995/02/16  19:10:59  markc
 * Removed start slash from comments
 * Removed start slash from comments
 *
 * Revision 1.7  1995/02/16  08:22:32  markc
 * Changed Boolean to bool
 * Changed wait loop code for igen messages - check for buffered messages
 * Changed char igen-array code to use strings/vectors for igen functions
 *
 * Revision 1.6  1995/01/26  17:59:14  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.5  1995/01/05  19:23:14  newhall
 * changed the size of the data buffer to be proportional
 * to the number of enabled metric/focus pairs.
 *
 * Revision 1.4  1994/11/04  06:41:05  newhall
 * removed printfs
 *
 * Revision 1.3  1994/09/25  01:52:10  newhall
 * updated to support the changes to the  visi, UI and VM interfaces having
 * to do with a new representation of metric/focus lists as a list of
 * metric/focus pairs.
 *
 * Revision 1.2  1994/09/22  01:20:20  markc
 * Changed "String" to "char*"
 *
 * Revision 1.1  1994/08/13  20:52:40  newhall
 * changed when a visualization process is started
 * added new file VISIthreadpublic.C
 * */
/////////////////////////////////////////////////////////////////////
// * visualizationUser routines:  GetMetricResource, StartPhase
//		StopMetricResource 
// * VISIthread server routines:  VISIKillVisi
/////////////////////////////////////////////////////////////////////
#include <signal.h>
#include <math.h>
#include "thread/h/thread.h"
#include "util/h/list.h"
#include "util/h/rpcUtil.h"
#include "VM.thread.CLNT.h"
#include "UI.thread.CLNT.h"
#include "dataManager.thread.CLNT.h"
#include "visi.xdr.CLNT.h"
#include "VISIthread.thread.SRVR.h"
#include "../VMthread/VMtypes.h"
#include "VISIthreadTypes.h"
#include "../pdMain/paradyn.h"
#include "dyninstRPC.xdr.CLNT.h"
#include "../DMthread/DMinclude.h"
#define  ERROR_MSG(s1, s2) \
	 uiMgr->showError(s1,s2); 

//////////////////////////////////////////////////
// VISIKillVisi:  VISIthread server routine 
//
//  called from VisiMgr, kills the visualization 
//  process and sets thread local variable "quit"
//  so that the VISIthread will die 
//////////////////////////////////////////////////
 void VISIthread::VISIKillVisi(){

 VISIthreadGlobals *ptr;

  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    PARADYN_DEBUG(("thr_getspecific in VISIthread::VISIKillVisi"));
    ERROR_MSG(13,"thr_getspecific in VISIthread::VISIKillVisi");
    return;
  }

  ptr->quit = 1;

}

void visualizationUser::GetPhaseInfo(){

 VISIthreadGlobals *ptr;

 PARADYN_DEBUG(("in visualizationUser::GetPhaseInfo"));
 if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    PARADYN_DEBUG(("thr_getspecific in visiUser::GetPhaseInfo"));
    ERROR_MSG(13,"thr_getspecific in VISIthread::GetPhaseInfo");
    return;
 }

 vector<T_visi::phase_info> *phases = ptr->dmp->getAllPhaseInfo();
 if((ptr->currPhaseHandle == -1) && (phases->size() > 0)){
      ptr->currPhaseHandle = (*phases)[(phases->size() -1)].handle;
 }

 ptr->visip->PhaseData(*phases);
 delete phases;

}



//////////////////////////////////////////////////////////////////////
//  GetMetricResource: visualizationUser routine (called by visi process)
//  input: string of metric names, string of focus names, type of data
//         (0: histogram, 1: scalar) currently only 0 supported
//
// check if metric and resource lists have wild card chars 
// if so request metrics and resources form UIM (currently, the
// only option), else make enable data collection call to DM for each
// metric resource pair
//////////////////////////////////////////////////////////////////////
void visualizationUser::GetMetricResource(string mets_res,
					  int numElements,
					  int type){
 VISIthreadGlobals *ptr;

PARADYN_DEBUG(("in visualizationUser::GetMetricResource"));
 if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    PARADYN_DEBUG(("thr_getspecific in visiUser::GetMetricResource"));
    ERROR_MSG(13,"thr_getspecific in VISIthread::GetMetricResource");
    return;
 }
 // TODO: parse the mets_res list
 // if not empty and no wild cards convert mets_res to list of 
 // metrespairs representation and call VISIthreadchooseMetRes routine

 // otherwise initiate menuing request
 ptr->ump->chooseMetricsandResources((chooseMandRCBFunc)VISIthreadchooseMetRes,
				     NULL);
}


//////////////////////////////////////////////////////////////////////
//  StopMetricResource: visualizationUser routine (called by visi process)
//  input: metric and resource Ids 
//
//  if metricId and resourceId are valid, make disable data collection
//  call to dataManager for the pair, and remove the associated metric
//  instance from the threads local mrlist
//////////////////////////////////////////////////////////////////////
void visualizationUser::StopMetricResource(u_int metricId,
					   u_int resourceId){

  VISIthreadGlobals *ptr;
  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    PARADYN_DEBUG(("thr_getspecific in visualizationUser::StopMetricResource"));
    ERROR_MSG(13,"thr_getspecific in VISIthread::StopMetricResource");
    return;
  }


  // search metricList for matching metricId and resourceId
  // if found request DM to disable data collection of metricInstance
  unsigned size = ptr->mrlist.size();
  for (unsigned i=0; i < size; i++){
      if(( ptr->mrlist[i]->m_id == metricId) && 
	  (ptr->mrlist[i]->r_id == resourceId)){
          PARADYN_DEBUG(("in visualizationUser::StopMetricResource: mi found"));
	  metricInstanceHandle mi_handle = ptr->mrlist[i]->mi_id;
	  // make disable request to DM
          ptr->dmp->disableDataCollection(ptr->ps_handle,mi_handle,
					  ptr->args->phase_type);
	  // remove mi from mrlist
	  ptr->mrlist[i] = ptr->mrlist[size - 1];
	  ptr->mrlist.resize(size - 1);
          ptr->maxBufferSize--;
          assert(ptr->maxBufferSize >= 0);
          return;
      }
      PARADYN_DEBUG(("current list element: metId = %d resId = %d",
		     ptr->mrlist[i]->m_id,ptr->mrlist[i]->r_id));
  }

#ifdef DEBUG
  PARADYN_DEBUG(("visualizationUser::StopMetricResource: mi not found\n"));
  PARADYN_DEBUG(("metricId = %d resourceId = %d\n",metricId,resourceId));
#endif
}



///////////////////////////////////////////////////////////////////
//  StartPhase: visualizationUser routine (called by visi process)
//  input: name of phase, begining and ending timestamp for phase 
//
//  not currently implemented
///////////////////////////////////////////////////////////////////
void visualizationUser::StartPhase(double begin,
				   string name){

 VISIthreadGlobals *ptr;

  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    PARADYN_DEBUG(("thr_getspecific in visualizationUser::PhaseName"));
    ERROR_MSG(13,"thr_getspecific in VISIthread::PhaseName");
    return;
  }

  // call datamanager start phase routine
  ptr->dmp->StartPhase((timeStamp)-1.0, NULL);

}

