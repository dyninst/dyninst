/*
 * Copyright (c) 1996-2002 Barton P. Miller
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

/////////////////////////////////////////////////////////////////////
// * visualizationUser routines:  GetMetricResource, StartPhase
//		StopMetricResource 
// * VISIthread server routines:  VISIKillVisi
/////////////////////////////////////////////////////////////////////
#include <signal.h>
#include <math.h>
#include "../pdMain/paradyn.h"
#include "pdthread/h/thread.h"
#include "common/h/List.h"
#include "pdutil/h/rpcUtil.h"
#include "VM.thread.CLNT.h"
#include "UI.thread.CLNT.h"
#include "dataManager.thread.CLNT.h"
#include "visi.xdr.CLNT.h"
#include "VISIthread.thread.SRVR.h"
#include "../VMthread/VMtypes.h"
#include "VISIthreadTypes.h"
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

 pdvector<T_visi::phase_info> *phases = ptr->dmp->getAllPhaseInfo();
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
void visualizationUser::GetMetricResource(string,int,int){
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

extern void flush_buffer_if_nonempty(VISIGlobalsStruct *);
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
      if(( ptr->mrlist[i].m_id == metricId) && 
	  (ptr->mrlist[i].r_id == resourceId)){
          PARADYN_DEBUG(("in visualizationUser::StopMetricResource: mi found"));
	  metricInstanceHandle mi_handle = ptr->mrlist[i].mi_id;
	  // make disable request to DM
          ptr->dmp->disableDataCollection(ptr->ps_handle,
					  ptr->pt_handle, mi_handle,
					  ptr->args->phase_type);

          // new; avoids losing data when we shrink buffer
          flush_buffer_if_nonempty(ptr);
	  assert(ptr->buffer_next_insert_index == 0);

	  // remove mi from mrlist
	  ptr->mrlist[i] = ptr->mrlist[size - 1];
	  ptr->mrlist.resize(size - 1);

          assert(ptr->buffer.size() > 0);
	  if(ptr->mrlist.size() < ptr->buffer.size()){
              unsigned newMaxBufferSize = ptr->buffer.size() - 1;
              ptr->buffer.resize(newMaxBufferSize);
              ptr->traceBuffer.resize(10);
	  }
          return;
      }
      PARADYN_DEBUG(("current list element: metId = %d resId = %d",
		     ptr->mrlist[i].m_id,ptr->mrlist[i].r_id));
  }
  if(!size)
       ptr->traceBuffer.resize(0);

#ifdef DEBUG
  PARADYN_DEBUG(("visualizationUser::StopMetricResource: mi not found\n"));
  PARADYN_DEBUG(("metricId = %d resourceId = %d\n",metricId,resourceId));
#endif
}

//
// showError: visualizationUser routine called by a visi process to
// display error messages
//
void visualizationUser::showError(int code, string msg)
{
  uiMgr->showError(code,P_strdup(msg.c_str()));
}

///////////////////////////////////////////////////////////////////
//  StartPhase: visualizationUser routine (called by visi process)
//  input: name of phase, begining and ending timestamp for phase 
//
//  not currently implemented
///////////////////////////////////////////////////////////////////
void visualizationUser::StartPhase(double, string,
				   bool withPerfConsult,
				   bool withVisis) {

 VISIthreadGlobals *ptr;

  if (thr_getspecific(visiThrd_key, (void **) &ptr) != THR_OKAY) {
    PARADYN_DEBUG(("thr_getspecific in visualizationUser::PhaseName"));
    ERROR_MSG(13,"thr_getspecific in VISIthread::PhaseName");
    return;
  }

  // call datamanager start phase routine
  ptr->dmp->StartPhase(NULL, NULL, withPerfConsult, withVisis);
}
