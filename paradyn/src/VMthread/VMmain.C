
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
/* Revision 1.12  1994/07/12 17:03:50  newhall
/* added error handling
/*
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

static int      currNumActiveVisis = 0;
thread_key_t visiThrd_key;
List<VMactiveVisi *>  activeVisis; 
List<VMvisis *> visiList;


extern void VISIthreadmain(visi_thread_args *args);

#define ERROR_MSG(s1, s2) \
   uiMgr->showError(s1,s2); \
   printf(s2);

VM_activeVisiInfo_Array VM::VMActiveVisis(){

  VM_activeVisiInfo_Array temp;
  VMactiveVisi *tempdata;
  int i;

  temp.count = activeVisis.count(); 
  if((temp.data  = 
      (VM_activeVisiInfo *) malloc(sizeof(VM_activeVisiInfo)*temp.count))
      == NULL){
    ERROR_MSG(18,"malloc in VMActiveVisis");
  }

  activeVisis.setCurrent();
  for(i=0; i < temp.count; i++){
     if((tempdata = activeVisis.getCurrent()) == 0){
       PARADYN_DEBUG(("Error in getCurrent() in VMActiveVisis")); 
       ERROR_MSG(20,"Error in getCurrent() in VMActiveVisis");
     } 
     if(tempdata != 0){
       temp.data[i].visiNum = tempdata->visiThreadId;
       temp.data[i].visiTypeId = tempdata->visiTypeId;
       if(tempdata->name != NULL){
         if((temp.data[i].name = strdup(tempdata->name)) == NULL){
	   ERROR_MSG(19,"strdup in VM::ActiveVisis");
	   tempdata->name = NULL;
         }
       }
       else{
	 temp.data[i].name = NULL;
	 printf("error #: informational, active visi %d name = NULL\n",i);
       }
     } 
     activeVisis.advanceCurrent();
  }
  return(temp);
}


VM_visiInfo_Array VM::VMAvailableVisis(){

  VM_visiInfo_Array  temp; 
  int i;
  VMvisis *temp2;

  PARADYN_DEBUG(("in VMAvailableVisis"));
  temp.count = visiList.count();
  if((temp.data  = (VM_visiInfo *)malloc(sizeof(VM_visiInfo)*temp.count))
       == NULL){ 
    ERROR_MSG(18, "malloc in VMAvailableVisis");
  }
  visiList.setCurrent();
  for(i=0; i < temp.count; i++){
    if((temp2 = visiList.getCurrent()) == 0){
       PARADYN_DEBUG(("Error in getCurrent() in VMAvailableVisis"));
       ERROR_MSG(20,"Error in getCurrent() in VMAvailableVisis");
    }
    else{
      temp.data[i].visiTypeId = temp2->Id;
      if(temp2->name != NULL){
        if((temp.data[i].name = strdup(temp2->name)) == NULL){
	  ERROR_MSG(19,"strdup in VM::AvailableVisis");
	  temp.data[i].name = NULL;
        }
      }
      else
	temp.data[i].name = NULL;
    }
    visiList.advanceCurrent();
  }
  return(temp);

}


int VM::VMAddNewVisualization(char *name,
			      int argc,
			      char *argv[]){

VMvisis *temp;
int i;
int id;

  id = visiList.count();
  id++;

  // create new VMvisis list element and add to visiList
  if((temp = (VMvisis *)malloc(sizeof(VMvisis))) == NULL){
    perror("malloc in VM::VMAddNewVisualization");
    ERROR_MSG(18,"malloc in VM::VMAddNewVisualization");
    return(VMERROR_MALLOC);
  }
  if((temp->argv = (char **)malloc(sizeof(char *)*argc)) == NULL){
    ERROR_MSG(18,"malloc in VM::VMAddNewVisualization");
    return(VMERROR_MALLOC);
  }

  // argv must be null terminated
  temp->argv[argc] = (char *) 0;
  for(i=0;i<argc;i++){
    if((temp->argv[i] = strdup(argv[i])) == NULL){
      ERROR_MSG(19,"strdup in VM::VMAddNewVisualization");
      return(VMERROR_MALLOC);
    }
  }
  if((temp->name = strdup(name)) == NULL){
    ERROR_MSG(19,"strdup in VM::VMAddNewVisualization");
    return(VMERROR_MALLOC);
  }
  temp->argc = argc;
  temp->Id = id;

  visiList.add(temp,(void *)id);

  return(VMOK); 
}


int  VM::VMCreateVisi(int visiTypeId){

thread_t  tid;
visi_thread_args *temp;
VMactiveVisi  *temp2;
VMvisis *visitemp;

  // get visi process command line to pass to visithread thr_create 
  if((visitemp = visiList.find((void *)visiTypeId))==NULL){
    PARADYN_DEBUG(("in VM::VMCreateVisi"));
    ERROR_MSG(20,"Error in find() in VM::VMCreateVisi");
    return(VMERROR_VISINOTFOUND);
  }

  temp = (visi_thread_args *)malloc(sizeof(visi_thread_args));
  temp->argc = visitemp->argc;
  temp->argv = (char **)visitemp->argv;
  temp->parent_tid = thr_self();


  // create a visi thread  
  thr_create(0,0,&VISIthreadmain,temp,0,&tid);

  // create a new visipointer
  if((temp2 = (VMactiveVisi  *)malloc(sizeof(VMactiveVisi))) == NULL){
    ERROR_MSG(18,"malloc in VM::VMCreateVisi");
    return(VMERROR_MALLOC);
  }

  temp2->visip = new VISIthreadUser(tid);

  // add  entry to active visi table 
   temp2->visiTypeId = visiTypeId;
   if((visitemp != 0) && (visitemp->name != NULL))
     if((temp2->name = strdup(visitemp->name)) == NULL){
       PARADYN_DEBUG(("strdup in VM::VMCreateVisi"));
       ERROR_MSG(19,"strdup in VM::VMCreateVisi");
       temp2->name = NULL;
     }
   else 
     temp2->name = NULL;
   temp2->visiThreadId = tid;
   activeVisis.add(temp2,(void *)tid);
 
  currNumActiveVisis++;

  return(VMOK);
}


void VM::VMDestroyVisi(int visiThreadId){

VMactiveVisi *temp;

  PARADYN_DEBUG(("VM::VMDestroyVisi: visiThreadId = %d",visiThreadId));
  PARADYN_DEBUG(("currNumActiveVisis = %d",currNumActiveVisis));

  // call visithread Kill_Visi routine (visithread will call thr_exit())
  if((temp = activeVisis.find((void *)visiThreadId)) != NULL){ 
     temp->visip->VISIKillVisi(); 
     // remove entry from active visi table 
     if((activeVisis.remove((void *)visiThreadId)) == FALSE){
       PARADYN_DEBUG(("remove in VM::VMDestroyVisi"));
       ERROR_MSG(20,"remove in VM::VMDestroyVisi");
     }
     else{
       // call destructor for visip 
       delete(temp->visip);
       currNumActiveVisis--;
     }
  }
  PARADYN_DEBUG(("VM::VMDestroyVisi: after temp->visip->VISIKillVisi"));
}


void VM::VMVisiDied(int visiThreadId){

  // remove visiId element from active list
  if(!(activeVisis.remove(&visiThreadId))){
    PARADYN_DEBUG(("remove in VM::VMVisiDied"));
    ERROR_MSG(20,"remove in VM::VMVisiDied");
  }
  currNumActiveVisis--;

}

void myfree(void* ptr) {
    (void) free(ptr);
}


// main loop for visualization manager thread
void *VMmain(int arg){

  unsigned tag;
  int      from;
  VM       *vmp; 
  UIMUser   *ump;
  performanceConsultantUser   *pcp; 
  int found;

  char  VMbuff[32];
  tag_t mtag;
  int   retVal;
  unsigned msgSize;
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
  // TODO: add  error code assoc with msg send and receive
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

  // free all malloced space

}
