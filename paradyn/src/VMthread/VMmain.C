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
/* $Log: VMmain.C,v $
/* Revision 1.40  1996/04/04 21:50:13  newhall
/* added mi_limit to VMAddNewVisualization
/*
 * Revision 1.39  1996/01/05 20:01:12  newhall
 * removed warnings
 *
 * Revision 1.38  1995/10/30  23:07:36  naim
 * Eliminating warning message - naim
 *
 * Revision 1.37  1995/09/26  20:27:46  naim
 * Minor changes in error messages
 *
 * Revision 1.36  1995/09/18  18:22:41  newhall
 * changes to avoid for-scope problem
 *
 * Revision 1.35  1995/09/08  19:51:27  krisna
 * stupid way to avoid the for-scope problem
 *
 * Revision 1.34  1995/08/12 22:28:48  newhall
 * Added VM_sequential_init and VM_post_thread_create_init. Changes to VMmain
 *
 * Revision 1.33  1995/08/11  21:51:39  newhall
 * added calls to VMmain to get initial set of visis from parsed PDL entries
 *
 * Revision 1.32  1995/08/01  02:18:50  newhall
 * changes to support phase interface
 *
 * Revision 1.31  1995/06/02  20:55:10  newhall
 * made code compatable with new DM interface
 * replaced List templates  with STL templates
 *
 * Revision 1.29  1995/02/16  08:23:18  markc
 * Changed Boolean to bool.
 * Changed wait loop code for igen messages
 *
 * Revision 1.28  1995/02/07  21:55:11  newhall
 * modified VMCreateVisi to get value for forceProcessStart from either
 * the caller or the visi table
 *
 * Revision 1.27  1995/01/26  17:59:17  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.26  1994/11/04  06:41:31  newhall
 * removed printfs
 *
 * Revision 1.25  1994/11/04  03:59:30  karavan
 * Removed status line displays
 *
 * Revision 1.24  1994/11/03  21:35:15  krisna
 * status lines for active visis.
 *
 * Revision 1.23  1994/10/10  02:51:52  newhall
 * purify fixes
 *
 * Revision 1.22  1994/09/30  21:20:10  newhall
 * added interface function VMStringToMetResPair
 * changed parameters to VMCreateVisi to take list of metrespair
 *
 * Revision 1.21  1994/09/25  01:53:03  newhall
 * updated to support the changes to the  visi, UI and VM interfaces having
 * to do with a new representation of metric/focus lists as a list of
 * metric/focus pairs.
 *
 * Revision 1.20  1994/09/22  01:21:32  markc
 * access tid using getTid() method
 *
 * Revision 1.19  1994/09/05  20:05:36  jcargill
 * Fixed read-before-write of thread stack data (spotted by purify)
 *
 * Revision 1.18  1994/08/17  01:44:55  markc
 * Correction, recommitted incorrect file, this should be the correct one.
 * Adds strdups to VMAddNewVisualization.
 *
 * Revision 1.17  1994/08/17  01:09:03  markc
 * VMAddNewVisualization now strdup's the strings passed to it.
 *
 * Revision 1.16  1994/08/16  16:35:53  markc
 * Removed uses of old string iterators.  Added new VMAddNewVisualization function to support config language.
 *
 * Revision 1.15  1994/08/13  20:54:26  newhall
 * visi_thread_args def. changed
 * VMCreateVisi arguments changed
 *
 * Revision 1.14  1994/08/01  17:20:32  newhall
 * bug fixes to VMVisiDied and VMCreateVisi
 *
 * Revision 1.13  1994/07/28  22:33:07  krisna
 * proper starting sequence for VMmain thread
 *
 * Revision 1.12  1994/07/12  17:03:50  newhall
 * added error handling
 *
 * Revision 1.11  1994/07/07  17:28:00  newhall
 * fixed compile warnings
 *
 * Revision 1.10  1994/07/07  03:27:00  markc
 * Removed reading of configuration file, which is now supported by the
 * configuration language.
 *
 * Revision 1.9  1994/06/29  02:56:47  hollings
 * AFS path changes
 *
 * Revision 1.8  1994/05/22  16:40:58  newhall
 * *** empty log message ***
 *
 * Revision 1.7  1994/05/22  01:58:53  newhall
 * fixed problem with thr_create arguments in VMCreateVisi
 *
 * Revision 1.6  1994/05/11  21:32:26  newhall
 * changed location of VMconfig.file
 *
 * Revision 1.5  1994/05/11  17:28:25  newhall
 * test version 3
 *
 * Revision 1.4  1994/05/09  21:16:53  hollings
 * added PARADYNCONFIG environment varaible for local spec. of visualizations.
 *
 * Revision 1.3  1994/04/28  22:08:34  newhall
 * test version 2
 *
 * Revision 1.2  1994/04/10  19:07:22  newhall
 * *** empty log message ***
 *
 * Revision 1.1  1994/04/09  21:23:47  newhall
 * test version
 * */
#include "thread/h/thread.h"
#include "VM.thread.SRVR.h"
#include "UI.thread.CLNT.h"
#include "performanceConsultant.thread.CLNT.h"
#include "VISIthread.thread.CLNT.h"
#include "VMtypes.h"
#include "paradyn/src/pdMain/paradyn.h"
#include "paradyn/src/met/metParse.h"

/*
#include "../UIthread/Status.h"
*/

#define ERROR_MSG(s1, s2) \
   uiMgr->showError(s1,s2); 

static int      currNumActiveVisis = 0;
thread_key_t visiThrd_key;
vector<VMactiveVisi *> activeVisis; 
vector<VMvisis *> visiList;
extern void* VISIthreadmain(void *args);
VM *VM::vmp = NULL;


/////////////////////////////////////////////////////////////
//  VMActiveVisis: VM server routine, returns a list of all 
//        visualization processes that are currently active
/////////////////////////////////////////////////////////////
vector<VM_activeVisiInfo> *VM::VMActiveVisis(){

  vector<VM_activeVisiInfo> *temp;
  if (!(temp = new vector<VM_activeVisiInfo>)) {
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
vector<VM_visiInfo> *VM::VMAvailableVisis(){

  PARADYN_DEBUG(("in VMAvailableVisis"));
  vector<VM_visiInfo> *temp;
  if (!(temp = new vector<VM_visiInfo>)) {
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
			      vector<string> *arg_str,
			      int  forceProcessStart,
			      int  mi_limit,
			      char *matrix,
			      int numMatrices){

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
  if((temp->argv = new (char*)[size+1]) == NULL){
      ERROR_MSG(18,"malloc in VM::VMAddNewVisualization");
      return(VMERROR);
  }

  // argv must be null terminated
  temp->argv[size] = (char *) 0;
  unsigned a_size = arg_str->size();
  for(unsigned i1=0; i1<a_size; i1++){
      if((temp->argv[i1] = strdup((*arg_str)[i1].string_of())) == NULL){
          ERROR_MSG(19,"strdup in VM::VMAddNewVisualization");
          return(VMERROR);
      }
  }
  temp->name = name;

  if (matrix){
      if((temp->matrix = strdup(matrix)) == NULL){
          ERROR_MSG(19,"strdup in VM::VMAddNewVisualization");
          return(VMERROR);
      }
  }
  else {
      temp->matrix = NULL;
  }
  temp->numMatrices = numMatrices;
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
			      vector<string> *arg_str,
			      int  forceProcessStart,
			      int  mi_limit,
			      char *matrix,
			      int numMatrices){

    return(VM_AddNewVisualization(name, arg_str, forceProcessStart,
				 mi_limit, matrix, numMatrices));
}

/////////////////////////////////////////////////////////////
//  VMStringToMetResPair: VM server routine, converts a string
//      representation of a metric/focus pair list to the 
//      internal representation
//  return value NULL indicates an error in parsing the string
/////////////////////////////////////////////////////////////
 vector<metric_focus_pair> *VM::VMStringToMetResPair(const char *){

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
		      vector<metric_focus_pair> *matrix){

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
      temp->bucketWidth = dataMgr->getGlobalBucketWidth();
      temp->start_time = 0.0;
      temp->my_phaseId = 0;
  }
  else {
      temp->bucketWidth = dataMgr->getCurrentBucketWidth();
      temp->start_time = dataMgr->getCurrentStartTime();
      temp->my_phaseId = dataMgr->getCurrentPhaseId();
   }
  // make sure bucket width is positive 
  if(temp->bucketWidth <= 0.0) {
      PARADYN_DEBUG(("bucketWidth is <= 0.0\n"));
      temp->bucketWidth = 0.1; 
  }
      

  if(matrix != NULL){
      temp->matrix = matrix;
  }
  else {
      // TODO: check active visi list to see if visi has
      // pre-defined set of metric/focus pairs, if so
      // parse the string representation into metrespair rep.
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
	  vector<string> argv;
	  assert(RPCgetArg(argv, next_visi->command().string_of()));
	  VM_AddNewVisualization(next_visi->name().string_of(), &argv, 
				next_visi->force(),next_visi->limit(),NULL, 0);
      }

  }

  char  VMbuff[32];
  // global synchronization
  int retVal = msg_send (MAINtid, MSG_TAG_VM_READY,(char *)NULL,0);
  tag_t mtag   = MSG_TAG_ALL_CHILDREN_READY;
  unsigned msgSize = 0;
  retVal = msg_recv (&mtag, VMbuff, &msgSize);
  return 1;
}



// main loop for visualization manager thread
void *VMmain(void* varg) {

  int arg; memcpy((void *) &arg, varg, sizeof arg);

  VM::VM_post_thread_create_init();


  while(1){
      unsigned tag = MSG_TAG_ANY;
      int from = msg_poll(&tag, 1);
      assert(from != THR_ERR);
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
