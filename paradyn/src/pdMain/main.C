/* $Log: main.C,v $
/* Revision 1.5  1994/04/28 22:07:39  newhall
/* added PARADYN_DEBUG macro: prints debug message if PARADYNDEBUG
/* environment variable has value >= 1
/*
 * Revision 1.4  1994/04/21  23:25:19  hollings
 * changed to no initial paradynd being defined.
 *
 * Revision 1.3  1994/04/10  19:08:48  newhall
 * added visualization manager thread create
 *
 * Revision 1.2  1994/04/05  04:36:48  karavan
 * Changed order of thread initialization to avoid deadlock.  Added global
 * user variables for data manager, uim, and performance consultant.
 *
 * Revision 1.1  1994/03/29  20:20:54  karavan
 * initial version for testing
 * */

/*
 * main.C - main routine for paradyn.  
 *   This routine creates DM, UIM, VM, and PC threads.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "paradyn.h"
#include "thread/h/thread.h"

extern void *UImain(CLargStruct *clargs);
extern void *DMmain(int arg);
extern void *PCmain(int arg);
extern void *VMmain (int arg);

#define MBUFSIZE 256
#define DEBUGBUFSIZE	4096

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
VMUser  *vmMgr;
int paradyn_debug;
char debug_buf[DEBUGBUFSIZE];

#define PRINT_DEBUG_MACRO				\
do {							\
	va_list args;					\
	va_start(args,format);				\
	(void) fflush(stdout);				\
	(void) vsprintf(debug_buf, format, args);	\
	(void) fprintf(stdout,"THREAD %d: %s\n",	\
	       thr_self(), debug_buf);			\
	(void) fflush(stdout);                          \
	va_end(args);					\
} while (0)

void print_debug_macro(const char* format, ...){
  if(paradyn_debug > 0)
     PRINT_DEBUG_MACRO;
  return;
}

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
  char *temp;


  if (argc != 1) {
    printf("usage: %s\n", argv[0]);
    exit(-1);
  }

  // get paradyn_debug environment var PARADYNDEBUG, if its value
  // is > 1, then PARADYN_DEBUG msgs will be printed to stdout
  if((temp = (char *) getenv("PARADYNDEBUG"))){
     paradyn_debug = atoi(temp);
  }
  else {
    paradyn_debug = 0;
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


// initialize PC

  if (thr_create(0, 0, PCmain, (void*) thr_self(), 0, 
		 (unsigned int *) &PCtid) == THR_ERR)
    exit(1);
  fprintf (stderr, "PC thread created\n");

  msgsize = MBUFSIZE;
  mtag = MSG_TAG_PC_READY;
  msg_recv(&mtag, mbuf, &msgsize);
  msg_send (PCtid, MSG_TAG_ALL_CHILDREN_READY, (char *) NULL, 0);
  perfConsult = new performanceConsultantUser (PCtid);

// initialize VM
  if (thr_create(0, 0, VMmain, (void*) thr_self(), 0, 
		 (unsigned int *) &VMtid) == THR_ERR)
    exit(1);

  fprintf (stderr, "VM thread created\n");
  msgsize = MBUFSIZE;
  mtag = MSG_TAG_VM_READY;
  msg_recv(&mtag, mbuf, &msgsize);
  msg_send (VMtid, MSG_TAG_ALL_CHILDREN_READY, (char *) NULL, 0);
  vmMgr = new VMUser (VMtid);

// wait for UIM thread to exit 

  thr_join (UIMtid, NULL, NULL);
	
  printf ("finished [test] main\n");
  thr_trace_off();

}

