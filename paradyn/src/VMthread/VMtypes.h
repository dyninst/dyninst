/* $Log: VMtypes.h,v $
/* Revision 1.1  1994/04/09 21:23:59  newhall
/* test version
/* */
#ifndef VMtypes_H
#define VMtypes_H
#include <stdio.h>
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

typedef struct {
  int visiTypeId;
  char *name;
  int visiThreadId;
  VISIthreadUser *visip;
} VMactiveVisi;


typedef struct {

  char *name; 
  int  argc;    // number of command line arguments
  char **argv;  // command line arguments, 1st arg is name of executable
} VMvisis;


typedef struct {
  int argc;
  char **argv;
  int  parent_tid;
} visi_thread_args;

#endif
