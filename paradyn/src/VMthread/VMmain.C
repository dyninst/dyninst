
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
/* Revision 1.5  1994/05/11 17:28:25  newhall
/* test version 3
/*
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


VM_activeVisiInfo_Array VM::VMActiveVisis(){

  VM_activeVisiInfo_Array temp;
  VMactiveVisi *tempdata;
  int i;

  temp.count = activeVisis.count(); 
  if((temp.data  = 
      (VM_activeVisiInfo *) malloc(sizeof(VM_activeVisiInfo)*temp.count))
      == NULL){
    perror("malloc in VMActiveVisis");
    uiMgr->showError("malloc in VMActiveVisis");
    printf("error # : serious\n");
    return;
  }

  activeVisis.setCurrent();
  for(i=0; i < temp.count; i++){
     if((tempdata = activeVisis.getCurrent()) == 0){
       PARADYN_DEBUG(("Error in getCurrent() in VMActiveVisis")); 
       uiMgr->showError("getCurrent in VMActiveVisis");
       printf("error # : serious???\n");
     } 
     if(tempdata != 0){
       temp.data[i].visiNum = tempdata->visiThreadId;
       temp.data[i].visiTypeId = tempdata->visiTypeId;
       if(tempdata->name != NULL){
         if((temp.data[i].name = strdup(tempdata->name)) == NULL){
           perror("strdup in VM::VMActiveVisis");
           uiMgr->showError("strdup in VM::VMActiveVisis");
           printf("error # : not so serious???\n");
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
    perror("malloc in VMAvailableVisis");
    uiMgr->showError("malloc in VMAvailableVisis");
    printf("error # : serious\n");
    return;
  }
  visiList.setCurrent();
  for(i=0; i < temp.count; i++){
    if((temp2 = visiList.getCurrent()) == 0){
       PARADYN_DEBUG(("Error in getCurrent() in VMAvailableVisis"));
       uiMgr->showError("getCurrent in VMAvailableVisis");
       printf("error # : serious???\n");
    }
    else{
      temp.data[i].visiTypeId = temp2->Id;
      if(temp2->name != NULL){
        if((temp.data[i].name = strdup(temp2->name)) == NULL){
          perror("strdup in VM::VMAvailableVisis");
          uiMgr->showError("strdup in VM::VMAvailableVisis");
          printf("error # : not so serious???\n");
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
    uiMgr->showError("malloc in VM::VMAddNewVisualization");
    printf("error # : serious???\n");
    return(VMERROR_MALLOC);
  }
  if((temp->argv = (char **)malloc(sizeof(char *)*argc)) == NULL){
    perror("malloc in VM::VMAddNewVisualization");
    uiMgr->showError("malloc in VM::VMAddNewVisualization");
    printf("error # : serious???\n");
    return(VMERROR_MALLOC);
  }

  for(i=0;i<argc;i++){
    if((temp->argv[i] = strdup(argv[i])) == NULL){
      perror("strdup in VM::VMAddNewVisualization");
      uiMgr->showError("strdup in VM::VMAddNewVisualization");
      printf("error # : serious???\n");
      return(VMERROR_MALLOC);
    }
  }
  if((temp->name = strdup(name)) == NULL){
    perror("strdup in VM::VMAddNewVisualization");
    uiMgr->showError("strdup in VM::VMAddNewVisualization");
    printf("error # : serious???\n");
    return(VMERROR_MALLOC);
  }
  temp->argc = argc;
  temp->Id = id;

  visiList.add(temp,(void *)id);

  return(VMOK); 
}


int  VM::VMCreateVisi(int visiTypeId){

thread_t  tid;
visi_thread_args temp;
VMactiveVisi  *temp2;
VMvisis *visitemp;

  // get visi process command line to pass to visithread thr_create 
  if((visitemp = visiList.find((void *)visiTypeId))==NULL){
    PARADYN_DEBUG(("in VM::VMCreateVisi"));
    uiMgr->showError("fine in  VM::VMCreateVisi");
    printf("error # : serious???\n");
    return(VMERROR_VISINOTFOUND);
  }
  temp.argc = visitemp->argc;
  temp.argv = visitemp->argv;
  temp.parent_tid = thr_self();

  printf("before thread create: temp.arv[0] = %s visitemp->argv[0] = %s\n",
	 temp.argv[0],visitemp->argv[0]);

  // create a visi thread  
  thr_create(0,0,&VISIthreadmain,&temp,0,&tid);

  // create a new visipointer
  if((temp2 = (VMactiveVisi  *)malloc(sizeof(VMactiveVisi))) == NULL){
    perror("malloc in VM::VMAddNewVisualization");
    uiMgr->showError("malloc in VM::VMCreateVisi");
    printf("error # : serious???\n");
    return(VMERROR_MALLOC);
  }

  temp2->visip = new VISIthreadUser(tid);

  // add  entry to active visi table 
   temp2->visiTypeId = visiTypeId;
   if((visitemp != 0) && (visitemp->name != NULL))
     if((temp2->name = strdup(visitemp->name)) == NULL){
       PARADYN_DEBUG(("strdup in VM::VMCreateVisi"));
       uiMgr->showError("strdup in VM::VMCreateVisi");
       printf("error # : serious???\n");
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
int ok;

  PARADYN_DEBUG(("VM::VMDestroyVisi: visiThreadId = %d",visiThreadId));
  PARADYN_DEBUG(("currNumActiveVisis = %d",currNumActiveVisis));

  // call visithread Kill_Visi routine (visithread will call thr_exit())
  if((temp = activeVisis.find((void *)visiThreadId)) != NULL){ 
     temp->visip->VISIKillVisi(); 
     // remove entry from active visi table 
     if((activeVisis.remove((void *)visiThreadId)) == FALSE){
       PARADYN_DEBUG(("remove in VM::VMDestroyVisi"));
       uiMgr->showError("remove in VM::VMDestroyVisi");
       printf("error # : serious???\n");
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
    uiMgr->showError("remove in VM::VMVisiDied");
    printf("error # : serious???\n");
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
  char 	   *vmConfigFile;
  performanceConsultantUser   *pcp; 
  int i,j,k,found;
  int num,num2;
  char temp[128];
  int  c;
  FILE *fd;
  VMvisis *tempvals;

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
     uiMgr->showError("visiThrd_key in VM::VMmain");
     printf("error # : serious\n");
     thr_perror("visiThrd_key");
     return (void *)0;
  }

  // initialize VM data structures

  // for visilist need info. from config. file on visualization info.
  if (!(vmConfigFile = getenv("PARADYNCONFIG"))) {
     vmConfigFile = "/usr/home/paradyn/development/newhall/core/paradyn/src/VMthread/VMconfig.file";
  }

  if ((fd = fopen(vmConfigFile,"r")) == NULL){
     // call error routine from UIM
     PARADYN_DEBUG(("error in VMmain opening VMconfig.file"));
     uiMgr->showError("error in VMmain opening VMconfig.file");
     printf("error # : serious\n");
  }
  else {
   fscanf(fd,"%d",&num);   
   for(i=0;i<num;i++){
      if((tempvals = (VMvisis *)malloc(sizeof(VMvisis))) == NULL){
	perror("malloc in VMmain");
        PARADYN_DEBUG(("error in VMmain malloc"));
        uiMgr->showError("error in VMmain malloc");
        printf("error # : serious\n");
      }
      else{
        fscanf(fd,"%d",&num2);
	if((tempvals->argv=(char **)malloc(sizeof(char *)*(num2+1)))
	   == NULL){
	  perror("malloc in VMmain");
          PARADYN_DEBUG(("error in VMmain malloc"));
          uiMgr->showError("error in VMmain malloc");
          printf("error # : serious\n");
	}
	else{

	  fscanf(fd,"%s",temp);
	  if((tempvals->name = strdup(temp)) == NULL){
              PARADYN_DEBUG(("error in VMmain strdup"));
              uiMgr->showError("error in VMmain strdup");
              printf("error # : serious\n");
	  }

          for(j=0;j<num2;j++){
	    fscanf(fd,"%s",temp);
	    if((tempvals->argv[j] = strdup(temp)) == NULL){
              PARADYN_DEBUG(("error in VMmain strdup"));
              uiMgr->showError("error in VMmain strdup");
              printf("error # : serious\n");
	    }
	  }
          tempvals->argv[j++] = 0;
	  tempvals->argc = num2;
	  tempvals->Id   = i;

#ifdef DEBUG
	  printf("\nvisi %d:\n",i);
	  for(j=0;j<num2;j++){
           printf("arg %d: %s\n",j,tempvals->argv[j]); 
	  }
         printf("adding visi %d to the visi List\n",i);
#endif

          visiList.add(tempvals,(void *)i);
	}
      } // else
    } // for
  }


#ifdef DEBUG
visiList.setCurrent();
for(i=0;i<visiList.count();i++){
  tempvisi = visiList.getCurrent();
  printf("tempvisi %d: id = %d name = %s args = %d argv[0] = %s\n",i,
  tempvisi->Id,tempvisi->name, tempvisi->argc,tempvisi->argv[0]);
  visiList.advanceCurrent();
}
for(i=0;i<visiList.count();i++){
  if((tempvisi = visiList.find((void *)i)) != 0){
     printf("visi %d: id = %d name = %s args = %d argv[0] = %s\n",i,
     tempvisi->Id,tempvisi->name,tempvisi->argc,tempvisi->argv[0]);
  }
}
#endif

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
