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
/* $Log: VMtypes.h,v $
/* Revision 1.9  1995/06/02 20:55:12  newhall
/* made code compatable with new DM interface
/* replaced List templates  with STL templates
/*
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
 * Revision 1.5  1994/07/07  17:28:02  newhall
 * fixed compile warnings
 *
 * Revision 1.4  1994/05/11  17:28:27  newhall
 * test version 3
 *
 * Revision 1.3  1994/04/29  18:57:56  newhall
 * changed typedefs of structs to deal with g++/gdb bug
 *
 * Revision 1.2  1994/04/28  22:08:36  newhall
 * test version 2
 *
 * Revision 1.1  1994/04/09  21:23:59  newhall
 * test version
 * */
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
  vector<metric_focus_pair> *matrix;
};
typedef struct visi_thread_argsStruct visi_thread_args;

#endif
