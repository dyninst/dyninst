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
/* Revision 1.5  1994/07/07 17:28:02  newhall
/* fixed compile warnings
/*
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
#include "VISIthread.CLNT.h"
#include "util/h/list.h"

#define VMOK                1 
#define VMERROR            -1
#define VMERROR_BASE	   -40
#define VMERROR_SUBSCRIPT  -41
#define VMERROR_MALLOC     -42
#define VMERROR_VISINOTFOUND -43
#define VMERROR_MAX	   -49


extern thread_key_t visiThrd_key;  // for VISIthread local storage
extern thread_t VM_tid; // visiManager tid 

struct VMactiveStruct {
  int visiTypeId;
  char *name;
  int visiThreadId;
  VISIthreadUser *visip;
};
typedef struct VMactiveStruct VMactiveVisi;

struct VMvisisStruct{

  char *name; 
  int  argc;    // number of command line arguments
  char **argv;  // command line arguments, 1st arg is name of executable
  int  Id;
};
typedef struct VMvisisStruct VMvisis;

struct visi_thread_argsStruct{
  int argc;
  char **argv;
  int  parent_tid;
};
typedef struct visi_thread_argsStruct visi_thread_args;

#endif
