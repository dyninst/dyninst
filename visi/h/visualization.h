/* $Log: visualization.h,v $
/* Revision 1.2  1994/03/17 05:13:41  newhall
/* change callback type
/*
 * Revision 1.1  1994/03/14  20:27:33  newhall
 * changed visi subdirectory structure
 *  */ 
#ifndef _visualization_h
#define _visualization_h
#include "datagrid.h"
#include "mrlist.h"
#include "error.h" 
#include "visi.SRVR.h"

#define FILETABLESIZE  64
#define EVENTSIZE      FOLD+1

// global variables assoc. with histo datagrid
extern visi_DataGrid  dataGrid;
extern visi_MRList    metricList;
extern visi_MRList    resourceList;
extern int            lastBucketSent;

// file descriptor array: 1st file desc. is assoc. w/ Paradyn
// remaining file descriptors are assigned by user when user
// uses main routine provided by Paradyn
extern int fileDesc[FILETABLESIZE];

// array of callback routines assoc. with file descriptors when
// user uses main routine provided by Paradyn
extern int (*fileDescCallbacks[FILETABLESIZE])();


// array of procedure pointers for callback routines assoc.
// with paradyn events  (ex. DATAVALUES,INVALIDMETRICSRESOURCES...)
extern int (*eventCallbacks[EVENTSIZE])(int);

// for calling paradyn-visualization interface routines
extern visualization *vp;
extern int visi_callback();


////////////////////////////////////////////////////////////////
//   initialization routines 
////////////////////////////////////////////////////////////////
// connects to parent socket and registers the mainLoop routine
// as a callback on events on fileDesc[0]
// creates metric and resource lists from args
// args: type, nummetrics, metriclist,numresources,resourcelist
extern int  VisiInit();
extern int  initDone;
extern int  StartVisi(int argc,char *argv[]);

// registration callback routine for paradyn events
// sets eventCallbacks[event] to callback routine provided by user
extern int RegistrationCallback(msgTag event,int (*callBack)(int));

// fd registration and callback routine registration for user
// to register callback routines when they use the provided main routine
extern int RegFileDescriptors(int *fd, int (*callBack)()); 

// main loop provided by paradyn 
// not currently supported
extern void ParadynMain();

#endif
