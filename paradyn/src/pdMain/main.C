/* $Log: main.C,v $
/* Revision 1.13  1994/11/01 22:27:22  karavan
/* Changed debugging printfs to PARADYN_DEBUG calls.
/*
 * Revision 1.12  1994/09/22  01:22:48  markc
 * Gave correct signature for signal
 *
 * Revision 1.11  1994/08/22  15:54:49  markc
 * Added command line argument to specify application config file.
 *
 * Revision 1.10  1994/07/28  22:31:42  krisna
 * proper prototypes and starting code for thread main functions
 *
 * Revision 1.9  1994/07/19  23:52:58  markc
 * Moved "include "metricExt.h"" to main.C from paradyn.h to remove false
 * dependencies.
 *
 * Revision 1.8  1994/07/07  03:26:24  markc
 * Added calls to parser routines.
 *
 * Revision 1.7  1994/05/18  00:51:03  hollings
 * We don't want SIGPIPEs to kill us.
 *
 * Revision 1.6  1994/05/10  03:57:54  hollings
 * Changed data upcall to return array of buckets.
 *
 * Revision 1.5  1994/04/28  22:07:39  newhall
 * added PARADYN_DEBUG macro: prints debug message if PARADYNDEBUG
 * environment variable has value >= 1
 *
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
extern "C" {
#include <signal.h>
// void (*signal())(int, void (*)());
};


#include "paradyn.h"
#include "thread/h/thread.h"
#include "paradyn/src/met/metricExt.h"

extern void *UImain(void *);
extern void *DMmain(void *);
extern void *PCmain(void *);
extern void *VMmain (void *);

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

void eFunction(int errno, char *message)
{
    printf("error: %s\b", message);
    abort();
}

int
main (int argc, char *argv[])
{

  CLargStruct* clargs;
  char mbuf[MBUFSIZE];
  unsigned int msgsize;
  tag_t mtag;
  char *temp;


  //
  // We check our own read/write events.
  //
  signal(SIGPIPE, SIG_IGN);

  if (argc != 1 && argc != 3) {
    printf("usage: %s [-f <filename>]\n", argv[0]);
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

// parse the configuration files
  int parseResult, a_ct=0;
  char *fname=0;
  while (argv[a_ct + 1]) {
    if (!strcmp(argv[a_ct], "-f")) {
      fname = argv[a_ct+1];
      break;
    }
    a_ct++;
  }
  parseResult = metMain(fname);


// get tid of parent
  MAINtid = thr_self();

     /* initialize the 4 main threads of paradyn: data manager, visi manager,
        user interface manager, performance consultant */
  
// initialize DM

  if (thr_create(0, 0, DMmain, (void *) &MAINtid, 0, 
		 (unsigned int *) &DMtid) == THR_ERR)
    exit(1);
  PARADYN_DEBUG (("DM thread created\n"));

  msgsize = MBUFSIZE;
  mtag = MSG_TAG_DM_READY;
  msg_recv(&mtag, mbuf, &msgsize);
  msg_send (DMtid, MSG_TAG_ALL_CHILDREN_READY, (char *) NULL, 0);
  dataMgr = new dataManagerUser (DMtid);
  context = dataMgr->createApplicationContext(eFunction);

// initialize UIM 
 
  if ((clargs = (CLargStruct *) malloc(sizeof(CLargStruct))) == 0) {
    perror("malloc(ClargsStruct)");
    return -1;
  }
  clargs->clargc = argc;	
  clargs->clargv = argv; // this is still dangerous, argv lives on
			 // main's stack

  if (thr_create (UIStack, sizeof(UIStack), &UImain, (void *) clargs, 0, &UIMtid) == THR_ERR) 
    exit(1);
  PARADYN_DEBUG (("UI thread created\n"));

  msgsize = MBUFSIZE;
  mtag = MSG_TAG_UIM_READY;
  msg_recv(&mtag, mbuf, &msgsize);
  msg_send (UIMtid, MSG_TAG_ALL_CHILDREN_READY, (char *) NULL, 0);
  uiMgr = new UIMUser (UIMtid);


// initialize PC

  if (thr_create(0, 0, PCmain, (void*) &MAINtid, 0, 
		 (unsigned int *) &PCtid) == THR_ERR)
    exit(1);
  PARADYN_DEBUG (("PC thread created\n"));

  msgsize = MBUFSIZE;
  mtag = MSG_TAG_PC_READY;
  msg_recv(&mtag, mbuf, &msgsize);
  msg_send (PCtid, MSG_TAG_ALL_CHILDREN_READY, (char *) NULL, 0);
  perfConsult = new performanceConsultantUser (PCtid);

// initialize VM
  if (thr_create(0, 0, VMmain, (void *) &MAINtid, 0, 
		 (unsigned int *) &VMtid) == THR_ERR)
    exit(1);

  PARADYN_DEBUG (("VM thread created\n"));
  msgsize = MBUFSIZE;
  mtag = MSG_TAG_VM_READY;
  msg_recv(&mtag, mbuf, &msgsize);
  msg_send (VMtid, MSG_TAG_ALL_CHILDREN_READY, (char *) NULL, 0);
  vmMgr = new VMUser (VMtid);

// take actions based on the parsed configuration files
  metDoDaemon();
  metDoTunable();
  metDoProcess();
  metDoVisi();
  PARADYN_DEBUG (("past metric parsing\n"));


// wait for UIM thread to exit 

  thr_join (UIMtid, NULL, NULL);
}

