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

/* $Log: VMtypes.h,v $
/* Revision 1.12  1996/08/16 21:09:25  tamches
/* updated copyright for release 1.1
/*
 * Revision 1.11  1996/04/04 21:50:15  newhall
 * added mi_limit to VMAddNewVisualization
 *
 * Revision 1.10  1995/08/01  02:18:52  newhall
 * changes to support phase interface
 *
 * Revision 1.9  1995/06/02  20:55:12  newhall
 * made code compatable with new DM interface
 * replaced List templates  with STL templates
 *
 * Revision 1.8  1995/01/26  17:59:19  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.7  1994/09/25  01:53:05  newhall
 * updated to support the changes to the  visi, UI and VM interfaces having
 * to do with a new representation of metric/focus lists as a list of
 * metric/focus pairs.
 *
 * Revision 1.6  1994/08/13  20:53:49  newhall
 * added fields to visi_thread_args type
 *
 */

#ifndef VMtypes_H
#define VMtypes_H
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "thread/h/thread.h"
#include "VISIthread.thread.CLNT.h"
#include "dyninstRPC.xdr.CLNT.h"
#include "util/h/Vector.h"
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
  char *matrix;  // string representation of initial set of metrics/foci
  int numMatrices;
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
