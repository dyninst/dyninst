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
/* Revision 1.26  1994/11/04 06:41:31  newhall
/* removed printfs
/*
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
#include "VM.SRVR.h"
#include "UI.CLNT.h"
#include "performanceConsultant.CLNT.h"
#include "VISIthread.CLNT.h"
#include "VMtypes.h"
#include "../pdMain/paradyn.h"

/*
#include "../UIthread/Status.h"
*/

#define ERROR_MSG(s1, s2) \
   uiMgr->showError(s1,s2); 

static int      currNumActiveVisis = 0;
thread_key_t visiThrd_key;
List<VMactiveVisi *>  activeVisis; 
List<VMvisis *> visiList;
extern void* VISIthreadmain(void *args);


/////////////////////////////////////////////////////////////
//  VMActiveVisis: VM server routine, returns a list of all 
//        visualization processes that are currently active
/////////////////////////////////////////////////////////////
VM_activeVisiInfo_Array VM::VMActiveVisis(){

  VM_activeVisiInfo_Array temp;
  VMactiveVisi *tempdata;
  List<VMactiveVisi*> walk;
  int i;

  temp.count = activeVisis.count(); 
  if (!(temp.data = new VM_activeVisiInfo[temp.count])) {
    ERROR_MSG(18,"malloc in VMActiveVisis");
    temp.count = 0;
    temp.data = (VM_activeVisiInfo*) NULL;
    return temp;
  }

  for (walk=activeVisis, i=0; tempdata = *walk; i++, walk++) {
    temp.data[i].visiNum = tempdata->visiThreadId;
    temp.data[i].visiTypeId = tempdata->visiTypeId;
    if(tempdata->name) {
      if(!(temp.data[i].name = strdup(tempdata->name))) {
	ERROR_MSG(19,"strdup in VM::ActiveVisis");
      }
    } else
      temp.data[i].name = (char*) NULL;
  }
  return(temp);
}


/////////////////////////////////////////////////////////////
// VMAvailableVisis: VM server routine, returns a list of all
//                   available visualizations 
/////////////////////////////////////////////////////////////
VM_visiInfo_Array VM::VMAvailableVisis(){

  VM_visiInfo_Array  temp; 
  int i;
  VMvisis *temp2;
  List<VMvisis*> vlist;

  PARADYN_DEBUG(("in VMAvailableVisis"));
  temp.count = visiList.count();
  if((temp.count = visiList.count()) > 0){
      if(!(temp.data = new VM_visiInfo[temp.count])) {
        ERROR_MSG(18, "malloc in VMAvailableVisis");
        temp.count = 0;
        temp.data = (VM_visiInfo*) NULL;
        return temp;
      }
  }
  else{
      temp.data = NULL;
  }


  for (vlist=visiList, i=0; temp2 = *vlist; i++, vlist++) {
    temp.data[i].visiTypeId = temp2->Id;
    if (temp2->name) {
      if(!(temp.data[i].name = strdup(temp2->name))) {
	ERROR_MSG(19,"strdup in VM::AvailableVisis");
	
      }
    }
    else
      temp.data[i].name = (char*) NULL;
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
int VM::VMAddNewVisualization(char *name,
			      int argc,
			      char *argv[],
			      int  forceProcessStart,
			      char *matrix,
			      int numMatrices){

VMvisis *temp = (VMvisis*) NULL;
int id,i;
List<VMvisis *> walk;

  // walk the list to determine if a visi with
  // this name is on the list
  for (walk = visiList; temp = *walk; walk++) {
    if (!strcmp(temp->name, name)) {
      break;
    } else
      temp = (VMvisis*) NULL;
  }

  if (!temp) {
    // create new VMvisis list element and add to visiList
    id = visiList.count();
    id++;
    if (!(temp = new VMvisis)) {
      perror("malloc in VM::VMAddNewVisualization");
      ERROR_MSG(18,"malloc in VM::VMAddNewVisualization");
      return(VMERROR);
    }
    visiList.add(temp,(void *)id);
  } else {
    // redefine an existing entry
    if (temp->argv) {
      int i=0;
      while (temp->argv[i]) {
	delete (temp->argv[i]);
	i++;
      }
      delete (temp->argv);
    }
  }

  // update info. for new entry 
  if((temp->argv = (char **)malloc(sizeof(char *)*(argc+1))) == NULL){
    ERROR_MSG(18,"malloc in VM::VMAddNewVisualization");
    return(VMERROR);
  }

  // argv must be null terminated
  temp->argv[argc] = (char *) 0;
  for(i=0;i<argc;i++){
    if((temp->argv[i] = strdup(argv[i])) == NULL){
        ERROR_MSG(19,"strdup in VM::VMAddNewVisualization");
        return(VMERROR);
    }
  }

  if((temp->name = strdup(name)) == NULL){
    ERROR_MSG(19,"strdup in VM::VMAddNewVisualization");
    return(VMERROR);
  }
  
  if (matrix){
      if((temp->matrix = strdup(matrix)) == NULL){
        ERROR_MSG(19,"strdup in VM::VMAddNewVisualization");
        return(VMERROR);
      }
  }
  else {
      matrix = NULL;
  }
  temp->numMatrices = numMatrices;
  temp->argc = argc;
  temp->Id = id;
  temp->forceProcessStart = forceProcessStart;
  return(VMOK); 
}


/////////////////////////////////////////////////////////////
//  VMStringToMetResPair: VM server routine, converts a string
//      representation of a metric/focus pair list to the 
//      internal representation
//  metrespair_Array values count = 0, and data = NULL indicate
//  an error in parsing the string list
/////////////////////////////////////////////////////////////
metrespair_Array  VM::VMStringToMetResPair(char *metresString){

metrespair_Array temp;

   temp.count = 0;
   temp.data  = NULL;
   return(temp);
}

/*
static
void
update_active_visis(VM* thisptr) {
	VM_activeVisiInfo_Array active_visis;
	char buf[1024];

	active_visis = thisptr->VMActiveVisis();
	buf[0] = '\0';
	for (unsigned i = 0; i < active_visis.count; i++) {
		strcat(buf, active_visis.data[i].name);
		strcat(buf, " ");
	}
	if (active_visis.count == 0) {
		sprintf(buf, "(none)");
	}

	free(active_visis.data);
}
*/
/////////////////////////////////////////////////////////////
// VMCreateVisi: VM server routine, starts a visualization process
//
// remenuFlag: if set, remenuing request made by visithread when
//             a set of metrics and resource choices can't be enabled
// forceProcessStart: if set, the first menuing request is skiped
//                     (visi process is started w/o menuing first)
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
		      metrespair *matrix,
		      int numMatrices){

thread_t  tid;
visi_thread_args *temp;
VMactiveVisi  *temp2;
VMvisis *visitemp;

  // get visi process command line to pass to visithread thr_create 
  if((visitemp = visiList.find((void *)visiTypeId))==NULL){
      PARADYN_DEBUG(("in VM::VMCreateVisi"));
      ERROR_MSG(20,"Error in find() in VM::VMCreateVisi");
      return(VMERROR);
  }

  temp = (visi_thread_args *)malloc(sizeof(visi_thread_args));
  temp->argc = visitemp->argc;
  temp->argv = (char **)visitemp->argv;
  temp->parent_tid = thr_self();

  if(matrix != NULL){

      temp->matrix = matrix;
      temp->numMatrices = numMatrices;
  }
  else {
      // TODO: check active visi list to see if visi has
      // pre-defined set of metric/focus pairs, if so
      // parse the string representation into metrespair rep.

      temp->matrix = NULL;
      temp->numMatrices = 0;
  }
  temp->remenuFlag = remenuFlag;
  temp->forceProcessStart = forceProcessStart;


  // create a visi thread  
  thr_create(0,0,&VISIthreadmain,temp,0,&tid);

  // create a new visipointer
  if((temp2 = (VMactiveVisi  *)malloc(sizeof(VMactiveVisi))) == NULL){
      ERROR_MSG(18,"malloc in VM::VMCreateVisi");
      return(VMERROR);
  }

  temp2->visip = new VISIthreadUser(tid);

  // add  entry to active visi table 
   temp2->visiTypeId = visiTypeId;
   if((visitemp != 0) && (visitemp->name != NULL)){
       if((temp2->name = strdup(visitemp->name)) == NULL){
          PARADYN_DEBUG(("strdup in VM::VMCreateVisi"));
          ERROR_MSG(19,"strdup in VM::VMCreateVisi");
          temp2->name = NULL;
       }
   }
   else {
       temp2->name = NULL;
   }
   temp2->visiThreadId = tid;
   activeVisis.add(temp2,(void *)tid);
 
  PARADYN_DEBUG(("in VM::VMCreateVisi: tid = %d added to list",tid));
  currNumActiveVisis++;
/*
	update_active_visis(this);
*/
  return(VMOK);
}

/////////////////////////////////////////////////////////////
// VMDestroyVisi: VM server routine, kills a visualization 
// visiThreadId: thread identifier associated with the visi to kill
/////////////////////////////////////////////////////////////
void VM::VMDestroyVisi(int visiThreadId){

VMactiveVisi *temp;

  PARADYN_DEBUG(("VM::VMDestroyVisi: visiThreadId = %d",visiThreadId));
  PARADYN_DEBUG(("currNumActiveVisis = %d",currNumActiveVisis));

  // call visithread Kill_Visi routine (visithread will call thr_exit())
  if((temp = activeVisis.find((void *)visiThreadId)) != NULL){ 
      temp->visip->VISIKillVisi(); 
      free(temp->name);
      // remove entry from active visi table 
      if((activeVisis.remove((void *)visiThreadId)) == FALSE){
         PARADYN_DEBUG(("remove in VM::VMDestroyVisi"));
         ERROR_MSG(20,"remove in VM::VMDestroyVisi");
         PARADYN_DEBUG(("in VM::VMDestroyVisi: tid = %d can't be removed",getTid()));
      }
      else{
         // call destructor for visip 
         delete(temp->visip);
	 delete(temp);
         currNumActiveVisis--;
         PARADYN_DEBUG(("in VM::VMDestroyVisi: tid = %d removed",getTid()));
      }
  }
/*
  update_active_visis(this);
*/
  PARADYN_DEBUG(("VM::VMDestroyVisi: after temp->visip->VISIKillVisi"));
}


/////////////////////////////////////////////////////////////
//  VMVisiDied: VM server routine, notification of dead visi 
//  visiThreadId: thread identifier of visithread that has died
/////////////////////////////////////////////////////////////
void VM::VMVisiDied(int visiThreadId){

VMactiveVisi *temp;

  if(temp = activeVisis.find((void *)visiThreadId)){
     free(temp->name);
  }
  // remove visiId element from active list
  if(!(activeVisis.remove((void *)visiThreadId))){
      PARADYN_DEBUG(("remove in VM::VMVisiDied"));
      ERROR_MSG(20,"remove in VM::VMVisiDied");
      PARADYN_DEBUG(("in VM::VMVisiDied: tid = %d can't be removed",getTid));
  }
  delete(temp->visip);
  delete(temp);
  currNumActiveVisis--;
/*
  update_active_visis(this);
*/
}

void myfree(void* ptr) {
    (void) free(ptr);
}

// main loop for visualization manager thread
void *VMmain(void* varg) {

  int arg; memcpy((void *) &arg, varg, sizeof arg);

  unsigned tag;
  int      from;
  VM       *vmp; 
  UIMUser   *ump;
  performanceConsultantUser   *pcp; 
  int found;
  char  VMbuff[32];
  tag_t mtag;
  int   retVal;
  unsigned msgSize = 0;
#ifdef DEBUG
  VMvisis *tempvisi;
#endif

  thr_name("Visualization Manager");
  VMtid = thr_self();

  // create key for VISIthread local storage
  if (thr_keycreate(&visiThrd_key, myfree) != THR_OKAY) {
     PARADYN_DEBUG(("visiThrd_key in VM::VMmain"));
     ERROR_MSG(20,"visiThrd_key in VM::VMmain");
     return (void *)0;
  }

  // visis are defined in the configuration language and
  // reported using VMAddNewVisualization

  vmp = new VM(MAINtid);

  // global synchronization
  retVal = msg_send (MAINtid, MSG_TAG_VM_READY,(char *)NULL,0);
  mtag   = MSG_TAG_ALL_CHILDREN_READY;
  retVal = msg_recv (&mtag, VMbuff, &msgSize);

  ump = uiMgr;
  pcp = perfConsult;

  PARADYN_DEBUG(("before loop in VMmain"));
  while(1){
      found = 0;
      tag = MSG_TAG_ANY;
      from = msg_poll(&tag, 1);
      if (ump->isValidUpCall(tag)) {
          ump->awaitResponce(-1);
      }
      else if (pcp->isValidUpCall(tag)) {
          pcp->awaitResponce(-1);
      }
      else { // check for incomming client calls
         vmp->mainLoop();
      }
  }
  return((void *)0);
}
