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

/* $Id: VMmain.C,v 1.53 2003/01/15 17:16:34 willb Exp $ */

#include "paradyn/src/pdMain/paradyn.h"
#include "pdthread/h/thread.h"
#include "VM.thread.SRVR.h"
#include "UI.thread.CLNT.h"
#include "performanceConsultant.thread.CLNT.h"
#include "VISIthread.thread.CLNT.h"
#include "VMtypes.h"
#include "paradyn/src/met/metParse.h"
#include "../DMthread/DMmetric.h"
/*
#include "../UIthread/Status.h"
*/

#define ERROR_MSG(s1, s2) \
   uiMgr->showError(s1,s2); 

static int      currNumActiveVisis = 0;
thread_key_t visiThrd_key;
pdvector<VMactiveVisi *> activeVisis; 
pdvector<VMvisis *> visiList;
extern void* VISIthreadmain(void *args);
VM *VM::vmp = NULL;

/////////////////////////////////////////////////////////////
//  VMActiveVisis: VM server routine, returns a list of all 
//        visualization processes that are currently active
/////////////////////////////////////////////////////////////
pdvector<VM_activeVisiInfo> *VM::VMActiveVisis(){

  pdvector<VM_activeVisiInfo> *temp;
  if (!(temp = new pdvector<VM_activeVisiInfo>)) {
      ERROR_MSG(18,"malloc failure in VMActiveVisis");
      return NULL;
  }

  for(unsigned i=0; i < activeVisis.size(); i++) {
      VM_activeVisiInfo newElm;
      newElm.visiNum = (activeVisis[i])->visiThreadId;
      newElm.visiTypeId = (activeVisis[i])->visiTypeId;
      newElm.name = activeVisis[i]->name;
      *temp += newElm;
  }
  return(temp);
}


/////////////////////////////////////////////////////////////
// VMAvailableVisis: VM server routine, returns a list of all
//                   available visualizations 
/////////////////////////////////////////////////////////////
pdvector<VM_visiInfo> *VM::VMAvailableVisis(){

  PARADYN_DEBUG(("in VMAvailableVisis"));
  pdvector<VM_visiInfo> *temp;
  if (!(temp = new pdvector<VM_visiInfo>)) {
      ERROR_MSG(18,"malloc in VMAvailableVisis");
      return NULL;
  }

  for(unsigned i=0; i < visiList.size(); i++) {
     VM_visiInfo newVal;
     newVal.name = visiList[i]->name; 
     newVal.visiTypeId = visiList[i]->Id; 
     *temp += newVal;
  }
  return(temp);
}

/////////////////////////////////////////////////////////////
//  VMAddNewVisualization:  VM server routine
//  name: visualization's name (used in menuing)  
//  args: the command line arguments for the visualization
//        argv[0] is the name of the executable
// Note - this may add the visi to the list, or update an entry
//        in the list
/////////////////////////////////////////////////////////////
int VM_AddNewVisualization(const char *name,
			      pdvector<string> *arg_str,
			      int  forceProcessStart,
			      int  mi_limit,
			      pdvector<string> *matrix
			      ){

   if (!arg_str || !name) {
       // TODO -- is this error number correct
       ERROR_MSG(20,"parameters in VM::VMAddNewVisualization");
       return(VMERROR);
   }

  // walk the list to determine if a visi with  this name is on the list
  string temp_name = name;
  VMvisis *temp = NULL;
  for(unsigned i=0; i < visiList.size(); i++){
      if(visiList[i]->name == temp_name){
	  temp = visiList[i]; 
	  break;
      }
  }
  if(!temp){
      // create new VMvisis list element and add to visiList
      if (!(temp = new VMvisis)) {
          perror("malloc in VM::VMAddNewVisualization");
          ERROR_MSG(18,"malloc in VM::VMAddNewVisualization");
          return(VMERROR);
      }
      temp->Id = visiList.size();
      visiList += temp; 
      PARADYN_DEBUG(("visi added %s forcestart %d",name,forceProcessStart));
  }
  else { // redefine an existing entry
      if(temp->argv){
	  unsigned i = 0;
          while(temp->argv[i]){
              delete (temp->argv[i++]);
	  }
      }
      delete (temp->argv);
  }

  unsigned size = arg_str->size();
  // update info. for new entry 
  if((temp->argv = new char*[size+1]) == NULL){
      ERROR_MSG(18,"malloc in VM::VMAddNewVisualization");
      return(VMERROR);
  }

  // argv must be null terminated
  temp->argv[size] = (char *) 0;
  unsigned a_size = arg_str->size();
  for(unsigned i1=0; i1<a_size; i1++){
      if((temp->argv[i1] = strdup((*arg_str)[i1].c_str())) == NULL){
          ERROR_MSG(19,"strdup in VM::VMAddNewVisualization");
          return(VMERROR);
      }
  }
  temp->name = name;

  if (matrix){
      if((temp->matrix = new pdvector<string>(*matrix)) == NULL){
          ERROR_MSG(19,"strdup in VM::VMAddNewVisualization");
          return(VMERROR);
      }
  }
  else {
      temp->matrix = NULL;
  }
  delete matrix;
  temp->argc = size;
  temp->forceProcessStart = forceProcessStart;
  if(mi_limit > 0)
      temp->mi_limit = mi_limit;
  else 
      temp->mi_limit = 0;

  return(VMOK); 
}

/////////////////////////////////////////////////////////////
//  VMAddNewVisualization:  VM server routine
//  name: visualization's name (used in menuing)  
//  args: the command line arguments for the visualization
//        argv[0] is the name of the executable
// Note - this may add the visi to the list, or update an entry
//        in the list
/////////////////////////////////////////////////////////////
int VM::VMAddNewVisualization(const char *name,
			      pdvector<string> *arg_str,
			      int  forceProcessStart,
			      int  mi_limit,
			      pdvector<string> *matrix
			      ){

    return(VM_AddNewVisualization(name, arg_str, forceProcessStart,
				 mi_limit, matrix));
}

/////////////////////////////////////////////////////////////
//  VMStringToMetResPair: VM server routine, converts a string
//      representation of a metric/focus pair list to the 
//      internal representation
//  return value NULL indicates an error in parsing the string
/////////////////////////////////////////////////////////////
 pdvector<metric_focus_pair> *VM::VMStringToMetResPair(const char *){

   return(NULL);
}

/////////////////////////////////////////////////////////////
// VMCreateVisi: VM server routine, starts a visualization process
//
// remenuFlag: if set, remenuing request made by visithread when
//             a set of metrics and resource choices can't be enabled
// forceProcessStart: if 1, the first menuing request is skiped
//                     (visi process is started w/o menuing first)
//		      if 0, menuing is done before starting the visi
//		      if -1, the force value is obtained from the visi table
// visiTypeId: identifier indicating wch visi to start
// matrix: a string representation of an initail set 
//         of metrics and/or resources for the visi 
// 	   (this has to be a string because it can be 
// 	    invoked due to a create visi command from the
// 	    config. file with a list of mets and res ) 
/////////////////////////////////////////////////////////////
int  VM::VMCreateVisi(int remenuFlag,
                      int forceProcessStart,
		      int visiTypeId,
		      phaseType phase_type,
		      pdvector<metric_focus_pair> *matrix){

  // get visi process command line to pass to visithread thr_create 
  if(visiTypeId >= (int)visiList.size()){
      PARADYN_DEBUG(("in VM::VMCreateVisi"));
      ERROR_MSG(20,"visi Id out of range in VM::VMCreateVisi");
      return(VMERROR);
  }
  VMvisis *visitemp = visiList[visiTypeId];
  visi_thread_args *temp =  new visi_thread_args;
  temp->argc = visitemp->argc;
  temp->argv = (char **)visitemp->argv;
  temp->parent_tid = thr_self();
  temp->phase_type = phase_type;
  if(phase_type == GlobalPhase){
      dataMgr->getGlobalBucketWidth(&temp->bucketWidth);
      temp->start_time = relTimeStamp::Zero();
      temp->my_phaseId = 0;
  }
  else {
      dataMgr->getCurrentBucketWidth(&temp->bucketWidth);
      dataMgr->getCurrentStartTime(&temp->start_time);
      temp->my_phaseId = dataMgr->getCurrentPhaseId();
   }
  // make sure bucket width is positive 
  if(temp->bucketWidth <= timeLength::Zero()) {
      PARADYN_DEBUG(("bucketWidth is <= 0.0\n"));
      temp->bucketWidth = timeLength::ms() * 100; 
  }
      
  if(matrix != NULL){
      temp->matrix = new pdvector<metric_focus_pair>;
      for (unsigned i=0; i < matrix->size(); i++)
      {
      	  metric_focus_pair &metfocus = (*matrix)[i];
	  if (metfocus.met == UNUSED_METRIC_HANDLE)
	  {
		pdvector<metric_focus_pair> *match_matrix = dataMgr->matchMetFocus(&metfocus);
		if (match_matrix)
		{	*temp->matrix += *match_matrix;
			delete match_matrix;
		}
	  }else *temp->matrix += metfocus;
      }
      delete matrix;
  }
  else {
      temp->matrix = NULL;
  }
  //wxd visitemp->pdvector<string> == > pdvector<metric_focus_pair>
      // check active visi list to see if visi has
      // pre-defined set of metric/focus pairs, if so
      // parse the string representation into metrespair rep.

  if (visitemp->matrix)
  {
  	if (temp->matrix == NULL)
		temp->matrix = new pdvector<metric_focus_pair>;
	for (unsigned i=0;i < visitemp->matrix->size();i++)
	{
		metricHandle metric_h=UNUSED_METRIC_HANDLE;
		
		string metfocus_item((*visitemp->matrix)[i].c_str());
		char *metfocus_str=const_cast<char *>(
					 P_strdup(metfocus_item.c_str()));
		string *metric_name=NULL;
		string *code_name=NULL;
		string *machine_name=NULL;
		string *sync_name=NULL;
		for (char *pos=strtok(metfocus_str,", \t");pos != NULL;pos=strtok(NULL,", \t"))
		{
			if (!strcmp(pos,""))
				continue;
			if (metric_name == NULL)
				metric_name = new string(pos);
			else if (code_name== NULL)
				code_name = new string(pos);
			else if (machine_name == NULL)
				machine_name= new string(pos);
			else if (sync_name == NULL)
				sync_name = new string(pos);
		}
		delete metfocus_str;

		if (metric_name == NULL)
			metric_name = new string("*");
		if (code_name== NULL)
			code_name = new string("/Code");
		if (machine_name == NULL)
			machine_name= new string("/Machine");
		if (sync_name == NULL)
			sync_name = new string("/SyncObject");
	
		int	legal_metfocus=1;

		if (*metric_name == "*")
			metric_h = UNUSED_METRIC_HANDLE;
		else {
			metricHandle *result = dataMgr->findMetric(metric_name->c_str());
			if (result == NULL)
				legal_metfocus = 0;
			else metric_h = *result;
		}

		resourceHandle code_h,machine_h,sync_h;
		resourceHandle *rl=NULL;
		if (legal_metfocus)
		{
			rl=dataMgr->findResource(code_name->c_str());
			if (rl == NULL)
				legal_metfocus = 0;
			else code_h=*rl;
		}
        
		if (legal_metfocus)
		{
			rl=dataMgr->findResource(machine_name->c_str());
			if (rl == NULL)
				legal_metfocus = 0;
			else machine_h=*rl;
		}
        
		if (legal_metfocus)
		{
			rl=dataMgr->findResource(sync_name->c_str());
			if (rl == NULL)
				legal_metfocus = 0;
			else sync_h=*rl;
		}

		delete code_name;
		delete machine_name;
		delete sync_name;

		pdvector<resourceHandle> focus;
		focus += code_h;
		focus += machine_h;
		focus += sync_h;
		metric_focus_pair metfocus(metric_h,focus);
		if (metric_h == UNUSED_METRIC_HANDLE)
		{
			pdvector<metric_focus_pair> *match_matrix = dataMgr->matchMetFocus(&metfocus);
			if (match_matrix)
			{	*temp->matrix += *match_matrix;
				delete match_matrix;
			}
		}else {
			if (legal_metfocus)
				(*temp->matrix) += metfocus;
			else {
				string err_msg("invalid metric/focus ");
				err_msg += (*visitemp->matrix)[i].c_str();
				ERROR_MSG(120,P_strdup(err_msg.c_str()));
      				return(VMERROR);
			}
		}
	}
  }
  if (temp->matrix != NULL && temp->matrix->size() == 0)
  {
  	delete temp->matrix;
	temp->matrix = NULL;
  }

  temp->remenuFlag = remenuFlag;
  if(forceProcessStart == -1)
      temp->forceProcessStart = visitemp->forceProcessStart;
  else
      temp->forceProcessStart = forceProcessStart;

  temp->mi_limit = visitemp->mi_limit;
  PARADYN_DEBUG(("forceProcessStart = %d\n",temp->forceProcessStart));
  // create a visi thread  
  thread_t tid;
  thr_create(0,0,&VISIthreadmain,temp,0,&tid);

  // create a new visipointer
  VMactiveVisi *temp2 = new VMactiveVisi;
  if(temp2 == NULL){
      ERROR_MSG(18,"new in VM::VMCreateVisi");
      return(VMERROR);
  }

  temp2->visip = new VISIthreadUser(tid);

  // add  entry to active visi table 
   temp2->name = visitemp->name;
   temp2->visiThreadId = tid;
   activeVisis += temp2;
 
  PARADYN_DEBUG(("in VM::VMCreateVisi: tid = %d added to list",tid));
  currNumActiveVisis++;
  return(VMOK);
}

/////////////////////////////////////////////////////////////
// VMDestroyVisi: VM server routine, kills a visualization 
// visiThreadId: thread identifier associated with the visi to kill
/////////////////////////////////////////////////////////////
void VM::VMDestroyVisi(thread_t visiThreadId){

  PARADYN_DEBUG(("VM::VMDestroyVisi: visiThreadId = %d",visiThreadId));
  PARADYN_DEBUG(("currNumActiveVisis = %d",currNumActiveVisis));

  // kill visiThread associated with visi
  for(unsigned i = 0; i < activeVisis.size(); i++){
      if(activeVisis[i]->visiThreadId == visiThreadId){
          // remove entry from active visi table
	  VMactiveVisi *temp = activeVisis[i];
	  temp->visip->VISIKillVisi();  
	  activeVisis[i] = activeVisis[activeVisis.size() - 1];
	  activeVisis.resize(activeVisis.size() - 1);
	  delete(temp->visip);
	  delete(temp);
	  currNumActiveVisis--;
          PARADYN_DEBUG(("in VM::VMDestroyVisi: tid = %d removed",getTid()));
	  break;
  } }
  PARADYN_DEBUG(("VM::VMDestroyVisi: after temp->visip->VISIKillVisi"));
}


/////////////////////////////////////////////////////////////
//  VMVisiDied: VM server routine, notification of dead visi 
//  visiThreadId: thread identifier of visithread that has died
/////////////////////////////////////////////////////////////
void VM::VMVisiDied(thread_t visiThreadId){

  for(unsigned i=0; i < activeVisis.size(); i++){
      if(activeVisis[i]->visiThreadId == visiThreadId){
	  VMactiveVisi *temp = activeVisis[i];
	  // remove element from activeVisis list
	  activeVisis[i] = activeVisis[activeVisis.size() - 1];
	  activeVisis.resize(activeVisis.size() - 1);
	  delete(temp->visip);
	  delete(temp);
          currNumActiveVisis--;
  } }
}

int VM::VM_sequential_init(){
  return 1;
}


void myfree(void* ptr) {
    (void) free(ptr);
}
extern unsigned metVisiSize();
extern visiMet *metgetVisi(unsigned);

int VM::VM_post_thread_create_init(){

  thr_name("Visualization Manager");
  VMtid = thr_self();

  // create key for VISIthread local storage
  if (thr_keycreate(&visiThrd_key, myfree) != THR_OKAY) {
     PARADYN_DEBUG(("visiThrd_key in VM::VMmain"));
     ERROR_MSG(20,"visiThrd_key in VM::VMmain");
     return 0;
  }

  // visis are defined in the configuration language and
  // reported using VMAddNewVisualization

  VM::vmp = new VM(MAINtid);

  // Get PDL visi entries
  for(unsigned u=0; u < metVisiSize(); u++){
      visiMet *next_visi = metgetVisi(u);
      if(next_visi){
	  pdvector<string> argv;
	  bool aflag;
	  aflag=(RPCgetArg(argv, next_visi->command().c_str()));
	  assert(aflag);
	  
	  VM_AddNewVisualization(next_visi->name().c_str(), &argv, 
				next_visi->force(),next_visi->limit(),next_visi->metfocus());
      }

  }

  char  VMbuff[32];
  // global synchronization
  int retVal = msg_send (MAINtid, MSG_TAG_VM_READY,(char *)NULL,0);
  tag_t mtag   = MSG_TAG_ALL_CHILDREN_READY;
  thread_t mtid = MAINtid;
  unsigned msgSize = 0;
  retVal = msg_recv (&mtid, &mtag, VMbuff, &msgSize);
  assert( mtid == MAINtid );

  // register valid visis with the UI
  pdvector<VM_visiInfo> *temp = new pdvector<VM_visiInfo>;
  for(unsigned i=0; i < visiList.size(); i++) {
     VM_visiInfo newVal;
     newVal.name = visiList[i]->name; 
     newVal.visiTypeId = visiList[i]->Id; 
     *temp += newVal;
  }
  uiMgr->registerValidVisis(temp);

  return 1;
}


// main loop for visualization manager thread
void *VMmain(void* varg) {

  int arg; memcpy((void *) &arg, varg, sizeof arg);

  VM::VM_post_thread_create_init();


  while(1){
	  thread_t tid = THR_TID_UNSPEC;
      unsigned tag = MSG_TAG_ANY;
      int err = msg_poll(&tid, &tag, 1);
      assert(err != THR_ERR);

      if (tag == MSG_TAG_DO_EXIT_CLEANLY) {
          thr_exit(0);
      }

      if (uiMgr->isValidTag((T_UI::message_tags)tag)) {
	if (uiMgr->waitLoop(true, (T_UI::message_tags)tag) == T_UI::error) {
	  // TODO
	  cerr << "Error in VMmain.C, needs to be handled\n";
	  assert(0);
	}
      } else if (perfConsult->isValidTag(
		 (T_performanceConsultant::message_tags)tag)) {
	if (perfConsult->waitLoop(true, 
	   (T_performanceConsultant::message_tags)tag) ==
	   T_performanceConsultant::error) {
	  // TODO
	  cerr << "Error in VMmain.C, needs to be handled\n";
	  assert(0);
	}
      } else if (VM::vmp->isValidTag((T_VM::message_tags) tag)) {
	// check for incoming client calls
	if (VM::vmp->waitLoop(true, (T_VM::message_tags)tag) == T_VM::error) {
	  // TODO
	  cerr << "Error in VMmain.C, needs to be handled\n";
	  assert(0);
	}
      } else {
	// TODO
	cerr << "Message sent that is not recognized in PCmain.C\n";
	assert(0);
      }
  }
  for(unsigned i=0; i < visiList.size(); i++){
      for(int j=0; j < (*visiList[i]).argc; j++){
          free((*visiList[i]).argv[j]);
      }
  }
  return((void *)0);
}
