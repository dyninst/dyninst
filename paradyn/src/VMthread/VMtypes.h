/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

/* $Id: VMtypes.h,v 1.15 2001/04/25 18:41:37 wxd Exp $ */

#ifndef VMtypes_H
#define VMtypes_H
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "thread/h/thread.h"
#include "VISIthread.thread.CLNT.h"
#include "dyninstRPC.xdr.CLNT.h"
#include "common/h/Vector.h"
#include "paradyn/src/DMthread/DMinclude.h"


#define VMOK                  1 
#define VMERROR               0 

extern thread_key_t visiThrd_key;  // for VISIthread local storage
extern thread_t VM_tid; // visiManager tid 
class metric;
class resourceList;

struct VMactiveStruct {
  int visiTypeId;
  string name;
  thread_t visiThreadId;
  VISIthreadUser *visip;
};
typedef struct VMactiveStruct VMactiveVisi;

struct VMvisisStruct{
  string name; 
  int  argc;    // number of command line arguments
  char **argv;  // command line arguments, 1st arg is name of executable
  int  Id;
  int forceProcessStart; // if set, visi proc. started before initial menuing
  int mi_limit; // an upper bound on the number of mi's that can be enabled 
  vector<string> *matrix;  // string representation of initial set of metrics/foci
};
typedef struct VMvisisStruct VMvisis;

struct visi_thread_argsStruct{
  int  argc;
  char **argv;
  int  parent_tid;
  int  remenuFlag;
  int  forceProcessStart;
  int  mi_limit;
  phaseType phase_type;
  timeStamp bucketWidth;
  timeStamp start_time;
  unsigned  my_phaseId;
  vector<metric_focus_pair> *matrix;
};
typedef struct visi_thread_argsStruct visi_thread_args;

#endif
