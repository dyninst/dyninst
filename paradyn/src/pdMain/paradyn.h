/* $Log: paradyn.h,v $
/* Revision 1.4  1994/04/10 19:16:19  newhall
/* added VM definitions
/*
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

#include "dataManager.CLNT.h"
#include "performanceConsultant.CLNT.h"
#include "UI.CLNT.h"
#include "VM.CLNT.h"

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

extern thread_t UIMtid;
extern thread_t MAINtid;
extern thread_t PCtid;
extern thread_t DMtid;
extern thread_t VMtid;
extern applicationContext *context;

extern dataManagerUser *dataMgr;
extern performanceConsultantUser *perfConsult;
extern UIMUser *uiMgr;
extern VMUser  *vmMgr;

#endif


