/* $Log: VISIthreadTypes.h,v $
/* Revision 1.1  1994/04/09 21:23:04  newhall
/* test version
/* */
#ifndef VISI_thread_h
#define VISI_thread_h 
#include "thread/h/thread.h"
#include "VM.CLNT.h"
#include "UI.CLNT.h"
#include "dataManager.CLNT.h"
#include "visi.CLNT.h"
#include "../pdMain/paradyn.h"


#define BUFFERSIZE 1024
#define SUM     0
#define AVE     1



typedef struct {

  UIMUser *ump;
  VMUser *vmp;
  dataManagerUser *dmp;
  visualizationUser *visip;
  performanceStream *perStream;
  dataValue buffer[BUFFERSIZE];
  int bufferSize;
  int fd;
  int pid;
  int quit;
  List<metricInstance *> mrlist;  // data and key are metricInstance *

} VISIthreadGlobals;
#endif
