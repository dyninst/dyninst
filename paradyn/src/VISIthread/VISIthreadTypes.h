/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: VISIthreadTypes.h,v 1.33 2004/03/23 01:12:32 eli Exp $ 

#ifndef VISI_thread_h
#define VISI_thread_h 

#include "common/h/Vector.h"
#include "pdthread/h/thread.h"
#include "VM.thread.CLNT.h"
#include "UI.thread.CLNT.h"
#include "dataManager.thread.CLNT.h"
#include "visi.xdr.CLNT.h"
#include "../pdMain/paradyn.h"
#include "paradyn/src/DMthread/DMinclude.h"

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

  pdvector<T_visi::dataValue> buffer;
  u_int buffer_next_insert_index; // same as old bufferSize

  // trace data streams
  perfStreamHandle pt_handle;
  pdvector<T_visi::traceDataValue> traceBuffer;

  thread_t vtid;
  int quit;
  timeLength bucketWidth;
  visi_thread_args* args;
  int start_up;
  int currPhaseHandle;
  unsigned fd_first;
  pdvector<metricInstInfo> mrlist;

  // for enable requests
  pdvector<metric_focus_pair> *request;  // list returned by UI menuing
  pdvector<metric_focus_pair> *retryList;  // list of unsuccessful enables 
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
    visiUser(PDSOCKET sock) :
	visualizationUser(sock, NULL, NULL, 0) {;};
    virtual void handle_error();
};

extern int VISIthreadchooseMetRes(pdvector<metric_focus_pair> *pairList);

#endif
