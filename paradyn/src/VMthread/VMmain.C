/* $Log: VMmain.C,v $
/* Revision 1.2  1994/04/10 19:07:22  newhall
/* *** empty log message ***
/*
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

#define DEBUG



// static VMvisiList      visiList;
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
  temp.data  = (VM_activeVisiInfo *) malloc(sizeof(VM_activeVisiInfo)*temp.count);
  activeVisis.setCurrent();
  for(i=0; i < temp.count; i++){
     tempdata = activeVisis.getCurrent();
     if(tempdata != 0){
       temp.data[i].visiNum = tempdata->visiThreadId;
       temp.data[i].visiTypeId = tempdata->visiTypeId;
       if(tempdata->name != NULL)
         temp.data[i].name = strdup(tempdata->name);
       else
	 temp.data[i].name = NULL;
     } 
     activeVisis.advanceCurrent();
  }
  return(temp);
}


VM_visiInfo_Array VM::VMAvailableVisis(){

  VM_visiInfo_Array  temp; 
  int i;
  VMvisis *temp2;

#ifdef DEBUG
  printf("in VMAvailableVisis");
#endif
  temp.count = visiList.count();
  temp.data  = (VM_visiInfo *)malloc(sizeof(VM_visiInfo)*temp.count); 
  visiList.setCurrent();
  for(i=0; i < temp.count; i++){
    temp.data[i].visiTypeId = i;
    temp2 = visiList.getCurrent();
    if(temp2 != 0)
      temp.data[i].name = strdup(temp2->argv[0]);
    visiList.advanceCurrent();
  }
  return(temp);

}


int VM::VMAddNewVisualization(char *name,int argc,char *argv[]){

VMvisis *temp;
int i;
int id;

  id = visiList.count();
  id++;
  // create new VMvisis list element and add to visiList
  if((temp = (VMvisis *)malloc(sizeof(VMvisis))) == NULL){
    perror("malloc in VM::VMAddNewVisualization");
    return(VMERROR_MALLOC);
  }
  if((temp->argv = (char **)malloc(sizeof(char *)*argc)) == NULL){
    perror("malloc in VM::VMAddNewVisualization");
    return(VMERROR_MALLOC);
  }

  for(i=0;i<argc;i++){
    if((temp->argv[i] = strdup(argv[i])) == NULL){
      perror("strdup in VM::VMAddNewVisualization");
      return(VMERROR_MALLOC);
    }
  }
  if((temp->name = strdup(name)) == NULL){
    perror("strdup in VM::VMAddNewVisualization");
    return(VMERROR_MALLOC);
  }
  temp->argc = argc;

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
    perror("in VM::VMCreateVisi");
    return(VMERROR_VISINOTFOUND);
  }
  temp.argc = visitemp->argc;
  temp.argv = visitemp->argv;
  temp.parent_tid = thr_self();

  // create a visi thread  
  thr_create(0,0,&VISIthreadmain,&temp,0,&tid);

  // create a new visipointer
  temp2 = (VMactiveVisi  *)malloc(sizeof(VMactiveVisi));
  temp2->visip = new VISIthreadUser(tid);

  // add  entry to active visi table 
   temp2->visiTypeId = visiTypeId;
   if((visitemp != 0) && (visitemp->name != NULL))
     temp2->name = strdup(visitemp->name);
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

printf("in VM::VMDestroyVisi:  visiThreadId = %d\n",visiThreadId);
printf("in VM::VMDestroyVisi:  currNumActiveVisis = %d\n",currNumActiveVisis);

  // call visithread Kill_Visi routine (visithread will call thr_exit())
  if((temp = activeVisis.find((void *)visiThreadId)) != NULL){ 
     temp->visip->VISIKillVisi(); 
     printf("blahb albhalhblahlkdjf\n");
  }
printf("in VM::VMDestroyVisi: after temp->visip->VISIKillVisi\n"); 

  // remove entry from active visi table 
  if((activeVisis.remove((void *)visiThreadId)) == FALSE){
    perror("VM::VMDestroyVisi");
  }
  else{
    // call destructor for visip 
printf("in VM::VMDestroyVisi: before call to delete(temp->visip)\n"); 
    delete(temp->visip);
printf("in VM::VMDestroyVisi: after call to delete(temp->visip)\n"); 
    currNumActiveVisis--;
  }
}


void VM::VMVisiDied(int visiThreadId){

  // remove visiId element from active list
  if(!(activeVisis.remove(&visiThreadId))){
    perror("VM::VMDestroyVisi");
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

  thr_name("Visualization Manager");
  VMtid = thr_self();

  // create key for VISIthread local storage
  if (thr_keycreate(&visiThrd_key, myfree) != THR_OKAY) {
     thr_perror("visiThrd_key");
     return (void *)0;
  }

  // initialize VM data structures

  // for visilist need info. from config. file on visualization info.
  if ((fd = fopen("/usr/home/paradyn/development/newhall/core/paradyn/src/VMthread/VMconfig.file","r")) == NULL){
     // call error routine from UIM
     fprintf(stderr,"error in VMmain opening VMconfig.file\n");
  }
  else {
   fscanf(fd,"%d",&num);   
   for(i=0;i<num;i++){
      if((tempvals = (VMvisis *)malloc(sizeof(VMvisis))) == NULL){
	perror("malloc in VMmain");
      }
      else{
        fscanf(fd,"%d",&num2);
	if((tempvals->argv=(char **)malloc(sizeof(char *)*(num2+1)))==NULL){
	  perror("malloc in VMmain");
	}
	else{

          for(j=0;j<num2;j++){
	    fscanf(fd,"%s",temp);
	    tempvals->argv[j] = strdup(temp);
	  }
          tempvals->argv[j++] = 0;

/*
	  printf("\nvisi %d:\n",i);
	  for(j=0;j<num2;j++){
           printf("arg %d: %s\n",j,tempvals->argv[j]); 
	  }
*/

          visiList.add(tempvals,(void *)i);
	}
      } // else
    } // for
  }

  vmp = new VM(MAINtid);

  // global synchronization
  retVal = msg_send (MAINtid, MSG_TAG_VM_READY,(char *)NULL,0);
  mtag   = MSG_TAG_ALL_CHILDREN_READY;
  retVal = msg_recv (&mtag, VMbuff, &msgSize);

/*  TESTING
  ump = new UIMUser(UIMtid);
  pcp = new performanceConsultantUser(PCtid);

*/

  ump = uiMgr;
  pcp = perfConsult;

#ifdef DEBUG
	fprintf(stderr,"before loop in VMmain\n");
#endif
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
