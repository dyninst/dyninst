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
/* $Log: VISIthreadTypes.h,v $
/* Revision 1.19  1996/01/05 20:00:58  newhall
/* removed warnings
/*
 * Revision 1.18  1995/12/03  21:32:57  newhall
 * changes to support new sampleDataCallbackFunc
 *
 * Revision 1.17  1995/11/20  03:32:18  tamches
 * changed BUFFERSIZE from 64 to 1024, though it won't make much difference
 * since actual buffer size is still min(BUFFERSIZE, num m/f pairs).
 * changed buffer vrbles (now a vector<>)
 *
 * Revision 1.16  1995/10/12 19:44:45  naim
 * Adding some comments about error recovery to the visiUser class - naim
 *
 * Revision 1.15  1995/06/02  20:54:32  newhall
 * made code compatable with new DM interface
 * replaced List templates  with STL templates
 *
 * Revision 1.14  1995/02/26  02:08:32  newhall
 * added some of the support for the phase interface
 * fix so that the vector of data values are being
 * correctly filled before call to BulkDataTransfer
 *
 * Revision 1.13  1995/02/16  19:10:52  markc
 * Removed start slash from comments
 * Removed start slash from comments
 *
 * Revision 1.12  1995/02/16  08:22:26  markc
 * Changed Boolean to bool
 * Changed wait loop code for igen messages - check for buffered messages
 * Changed char igen-array code to use strings/vectors for igen functions
 *
 * Revision 1.11  1995/01/26  17:59:08  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.10  1995/01/05  19:23:07  newhall
 * changed the size of the data buffer to be proportional
 * to the number of enabled metric/focus pairs.
 *
 * Revision 1.9  1994/09/25  01:52:06  newhall
 * updated to support the changes to the  visi, UI and VM interfaces having
 * to do with a new representation of metric/focus lists as a list of
 * metric/focus pairs.
 *
 * Revision 1.8  1994/08/13  20:52:36  newhall
 * changed when a visualization process is started
 * added new file VISIthreadpublic.C
 *
 * Revision 1.7  1994/08/11  02:19:22  newhall
 * added call to dataManager routine destroyPerformanceStream
 *
 * Revision 1.6  1994/06/07  18:16:29  newhall
 * support for adding metrics/resources to an existing set
 *
 * Revision 1.5  1994/06/03  18:22:50  markc
 * Changes to support igen error handling.
 *
 * Revision 1.4  1994/05/11  17:21:29  newhall
 * Changes to handle multiple curves on one visualization
 * and multiple visualizations.  Fixed problems with folding
 * and resource name string passed to visualization.  Changed
 * data type from double to float.
 *
 * Revision 1.3  1994/04/29  18:57:35  newhall
 * changed typedefs of structs to deal with g++/gdb bug
 *
 * Revision 1.2  1994/04/28  22:08:07  newhall
 * test version 2
 *
 * Revision 1.1  1994/04/09  21:23:04  newhall
 * test version
 * */
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
  int pid;
  int quit;
  double bucketWidth;
  visi_thread_args* args;
  int start_up;
  int currPhaseHandle;
  vector<metricInstInfo *> mrlist;

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
