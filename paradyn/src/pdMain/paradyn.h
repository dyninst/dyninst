/* $Log: paradyn.h,v $
/* Revision 1.1  1994/03/29 20:21:25  karavan
/* initial version for testing.
/* */
/* paradyn.h */

/* some global definitions for main.C */


#ifndef _paradyn_h
#define _paradyn_h

#include "dataManager.CLNT.h"

struct CLargStruct {
  int clargc;
  char **clargv;
};

typedef struct CLargStruct CLargStruct;


/* common MSG TAG definitions */

unsigned int MSG_TAG_UIM_READY;
unsigned int MSG_TAG_DM_READY;
unsigned int MSG_TAG_VM_READY;
unsigned int MSG_TAG_PC_READY;
unsigned int MSG_TAG_ALL_CHILDREN_READY;

thread_t UIMtid;
thread_t MAINtid;
thread_t PCtid;
thread_t DMtid;
applicationContext *context;

#endif
