#ifndef VISI_thread_h
#define VISI_thread_h 
/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 * 
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */
#include "util/h/Vector.h"
#include "thread/h/thread.h"
#include "VM.thread.CLNT.h"
#include "UI.thread.CLNT.h"
#include "dataManager.thread.CLNT.h"
#include "visi.xdr.CLNT.h"
#include "../pdMain/paradyn.h"
#include "paradyn/src/DMthread/DMinclude.h"
#include "util/h/makenan.h"

#define SUM     0
#define AVE     1
#define  VISI_DEFAULT_FOCUS "Whole Program"
#define MIN(a,b) (((a)<(b))?(a):(b))

////////////////////////////////////////
//  for VISIthread local data  
///////////////////////////////////////
struct VISIGlobalsStruct {

  UIMUser *ump;
  VMUser *vmp;
  dataManagerUser *dmp;
  visualizationUser *visip; // where we talk to visi lib of the visi process
  perfStreamHandle ps_handle;

  vector<T_visi::dataValue> buffer;
  u_int buffer_next_insert_index; // same as old bufferSize

  int fd;
  int quit;
  double bucketWidth;
  visi_thread_args* args;
  int start_up;
  int currPhaseHandle;
  unsigned fd_first;
  vector<metricInstInfo> mrlist;

  // for enable requests
  vector<metric_focus_pair> *request;  // list returned by UI menuing
  vector<metric_focus_pair> *retryList;  // list of unsuccessful enables 
  u_int num_Enabled;  // number of successful enables in this request
  u_int next_to_enable;  // which element in request list to try next 
  u_int first_in_curr_request; // request vector id: start of curr. DM request

};
typedef struct VISIGlobalsStruct VISIthreadGlobals;

// IMPORTANT NOTE: whenever the visiUser constructor is used, it
// is the user's responsability to check whether the new object have been
// successfully created or not (i.e. by checking the public variable
// "bool errorConditionFound" in class visualizationUser). In this way, we 
// allow the user to take the appropriate error recovery actions instead of
// executing an "assert(0)" - naim
//
class visiUser : public visualizationUser
{
  public:
    visiUser(int fd) :
	visualizationUser(fd, NULL, NULL, false) {;};
    virtual void handle_error();
};

extern int VISIthreadchooseMetRes(vector<metric_focus_pair> *pairList);

#endif
