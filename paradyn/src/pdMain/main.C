/* $Log: main.C,v $
/* Revision 1.2  1994/04/05 04:36:48  karavan
/* Changed order of thread initialization to avoid deadlock.  Added global
/* user variables for data manager, uim, and performance consultant.
/*
 * Revision 1.1  1994/03/29  20:20:54  karavan
 * initial version for testing
 * */

/*
 * main.C - main routine for paradyn.  
 *   This routine creates DM, UIM, VM, and PC threads.
 */

#include <stdlib.h>
#include <stdio.h>

#include "paradyn.h"
#include "thread/h/thread.h"

extern void *UImain(CLargStruct *clargs);
extern void *DMmain(int arg);
extern void *PCmain(int arg);
// extern void *VMmain (int arg);

#define MBUFSIZE 256

thread_t UIMtid;
thread_t MAINtid;
thread_t PCtid;
thread_t DMtid;
thread_t VMtid;

char UIStack[32768];

applicationContext *context;
dataManagerUser *dataMgr;
performanceConsultantUser *perfConsult;
UIMUser *uiMgr;
// VMUser  *vmMgr;


int eFunction(int errno, char *message)
{
    printf("error: %s\b", message);
    abort();
    return(-1);
}

int
main (int argc, char *argv[])
{

  CLargStruct clargs;
  char mbuf[MBUFSIZE];
  unsigned int msgsize;
  tag_t mtag;


  if (argc < 3) {
    printf("usage: %s <paradynd> <program executable>\n", argv[0]);
    exit(-1);
  }

// get tid of parent
  MAINtid = thr_self();

     /* initialize the 4 main threads of paradyn: data manager, visi manager,
        user interface manager, performance consultant */
  
// initialize DM

  if (thr_create(0, 0, DMmain, (void *) thr_self(), 0, 
		 (unsigned int *) &DMtid) == THR_ERR)
    exit(1);
  fprintf (stderr, "DM thread created\n");

  msgsize = MBUFSIZE;
  mtag = MSG_TAG_DM_READY;
  msg_recv(&mtag, mbuf, &msgsize);
  msg_send (DMtid, MSG_TAG_ALL_CHILDREN_READY, (char *) NULL, 0);
  dataMgr = new dataManagerUser (DMtid);
  context = dataMgr->createApplicationContext(eFunction);

// initialize UIM 
 
  clargs.clargc = argc;	
  clargs.clargv = argv;

  if (thr_create (UIStack, sizeof(UIStack), &UImain, &clargs, 0, &UIMtid) == THR_ERR) 
    exit(1);
  fprintf (stderr, "UI thread created\n");

  msgsize = MBUFSIZE;
  mtag = MSG_TAG_UIM_READY;
  msg_recv(&mtag, mbuf, &msgsize);
  msg_send (UIMtid, MSG_TAG_ALL_CHILDREN_READY, (char *) NULL, 0);
  uiMgr = new UIMUser (UIMtid);

// initialize VM
/**
        msgsize = MBUFSIZE;
	mtag = MSG_TAG_VM_READY;
        msg_recv(&mtag, mbuf, &msgsize);
	msg_send (VMtid, MSG_TAG_ALL_CHILDREN_READY, (char *) NULL, 0);
	vmMgr = new VMUser (VMtid);
*/

// initialize PC

  if (thr_create(0, 0, PCmain, (void*) thr_self(), 0, 
		 (unsigned int *) &PCtid) == THR_ERR)
    exit(1);
  fprintf (stderr, "PC thread created\n");

  msgsize = MBUFSIZE;
  mtag = MSG_TAG_PC_READY;
  msg_recv(&mtag, mbuf, &msgsize);
  perfConsult = new performanceConsultantUser (PCtid);
  msg_send (PCtid, MSG_TAG_ALL_CHILDREN_READY, (char *) NULL, 0);

// wait for UIM thread to exit 

  thr_join (UIMtid, NULL, NULL);
	
  printf ("finished [test] main\n");
  thr_trace_off();

}

