/* $Log: paradyn.h,v $
/* Revision 1.9  1995/02/27 19:12:23  tamches
/* added TCtid for the new tunable constants thread
/*
 * Revision 1.8  1995/01/26  17:59:22  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.7  1994/07/19  23:53:03  markc
 * Moved "include "metricExt.h"" to main.C from paradyn.h to remove false
 * dependencies.
 *
 * Revision 1.6  1994/07/07  03:26:25  markc
 * Added calls to parser routines.
 *
 * Revision 1.5  1994/04/28  22:07:40  newhall
 * added PARADYN_DEBUG macro: prints debug message if PARADYNDEBUG
 * environment variable has value >= 1
 *
 * Revision 1.4  1994/04/10  19:16:19  newhall
 * added VM definitions
 *
 * Revision 1.3  1994/04/05  20:05:37  karavan
 * Reinstated User Interface.
 *
 * Revision 1.1  1994/03/29  20:21:25  karavan
 * initial version for testing.
 * */
/* paradyn.h */

/* some global definitions for main.C */


#ifndef _paradyn_h
#define _paradyn_h

#include "thread/h/thread.h"

#include "dataManager.thread.CLNT.h"
#include "performanceConsultant.thread.CLNT.h"
#include "UI.thread.CLNT.h"
#include "VM.thread.CLNT.h"

struct CLargStruct {
  int clargc;
  char **clargv;
};

typedef struct CLargStruct CLargStruct;


/* common MSG TAG definitions */

#define MSG_TAG_UIM_READY 1001
#define MSG_TAG_DM_READY 1002
#define MSG_TAG_VM_READY 1003
#define MSG_TAG_PC_READY 1004
#define MSG_TAG_ALL_CHILDREN_READY 1005
#define MSG_TAG_TC_READY 1006

extern thread_t UIMtid;
extern thread_t MAINtid;
extern thread_t PCtid;
extern thread_t DMtid;
extern thread_t VMtid;
extern thread_t TCtid; // tunable constant
extern applicationContext *context;

extern dataManagerUser *dataMgr;
extern performanceConsultantUser *perfConsult;
extern UIMUser *uiMgr;
extern VMUser  *vmMgr;


// PARADYN_DEBUG: if PARADYNDEBUG environment variable 
//     has value > 0, prints msg to stdout
extern void print_debug_macro(const char* format, ...);
#define PARADYN_DEBUG(x) print_debug_macro x

#endif


