/* $Log: main.C,v $
/* Revision 1.1  1994/03/29 20:20:54  karavan
/* initial version for testing
/* */

/*
 * main.C - main routine for paradyn.  
 *   This routine creates DM, UIM, VM, and PC threads.
 */

#include <stdlib.h>
#include <stdio.h>
extern "C" {
#include "thread/h/thread.h"
}
#include "paradyn.h"
#include "performanceConsultant.CLNT.h"

extern void *UImain(CLargStruct *clargs);
extern void *DMmain(int arg);
extern void *PCmain(int arg);

extern thread_t UIMtid;
extern thread_t MAINtid;
extern thread_t PCtid;
extern thread_t DMtid;
extern applicationContext *context;

#define MBUFSIZE 256

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

  thr_trace_on();

// get tid of parent
  MAINtid = thr_self();
  printf ("parent = %d", MAINtid);

// values for global message tags
 
  MSG_TAG_UIM_READY = 1001;
  MSG_TAG_DM_READY = 1002;
  MSG_TAG_VM_READY = 1003;
  MSG_TAG_PC_READY = 1004;
  MSG_TAG_ALL_CHILDREN_READY = 1005;

// initialize UIM 
 
  clargs.clargc = argc;	
  clargs.clargv = argv;

  printf ("argc = %d\n", clargs.clargc);

  if (thr_create (NULL, 0, &UImain, &clargs, 0, &UIMtid) == THR_ERR) 
    exit(1);
  fprintf (stderr, "UI thread created\n");

// initialize DM

  if (thr_create(0, 0, DMmain, (void *) thr_self(), 0, 
		 (unsigned int *) &DMtid) == THR_ERR)
    exit(1);
  fprintf (stderr, "DM thread created\n");

// initialize PC

  if (thr_create(0, 0, PCmain, (void*) thr_self(), 0, 
		 (unsigned int *) &PCtid) == THR_ERR)
    exit(1);
  fprintf (stderr, "PC thread created\n");

    //get acks from each thread and signal ready 
/**
        msgsize = MBUFSIZE;
	mtag = MSG_TAG_VM_READY;
        msg_recv(&mtag, mbuf, &msgsize);
*/
  msgsize = MBUFSIZE;
  mtag = MSG_TAG_DM_READY;
  msg_recv(&mtag, mbuf, &msgsize);
  msgsize = MBUFSIZE;
  mtag = MSG_TAG_PC_READY;
  msg_recv(&mtag, mbuf, &msgsize);
  
  msgsize = MBUFSIZE;
  mtag = MSG_TAG_UIM_READY;
  msg_recv(&mtag, mbuf, &msgsize);

  fprintf (stderr, "all ACKS received\n");

/* send ALL_CHILDREN_READY to all child threads */

      msg_send (UIMtid, MSG_TAG_ALL_CHILDREN_READY, (char *) NULL, 0);
      msg_send (DMtid, MSG_TAG_ALL_CHILDREN_READY, (char *) NULL, 0);
      msg_send (PCtid, MSG_TAG_ALL_CHILDREN_READY, (char *) NULL, 0);
/**
      msg_send (VMtid, MSG_TAG_ALL_CHILDREN_READY, (char *) NULL, 0);
*/

/* wait for UIM thread to exit */

  thr_join (UIMtid, NULL, NULL);
	
  printf ("finished [test] main\n");
  thr_trace_off();

}

