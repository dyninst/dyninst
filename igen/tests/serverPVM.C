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

#include <stdio.h>
#include <stdlib.h>
#include "testPVM.SRVR.h"

main(int argc, char *argv[])
{
  int flag;
  int ret;
  int started = 0;
  test *tp;
  char *where = NULL;
  char *program = NULL;
  
  if (argc < 2)
    started = 1;

  program = argv[1];
  if (argc == 3)
    {
      where = argv[2];
      flag = 1;
    }
  else
    flag = 0;

  if (started)
    tp = new test();
  else
    tp = new test(where, program, NULL, flag);

  // now go into main loop
  while(1)
    {
      ret = tp->mainLoop();	
      if (ret < 0)
	{
	  // assume the client has exited, and leave.
	  exit(-1);
	}
    }
}


void test::nullNull()
{
    return;
}

int test::intNull()
{
    return(0);
}

void test::nullStruct(intStruct s)
{
    return;
}

int test::intString(char *s)
{
    return(strlen(s));
}

char *test::stringString(char *s)
{
    return(s);
}

int test::add(int a, int b)
{
    return(a+b);
}

int test::sumVector(int_Array nums)
{
   int i, total;

   for (i=0, total=0; i < nums.count; i++) {
       total += nums.data[i];
   }
   return(total);
}

int_Array test::retVector(int num, int start)
{
    int i;
    int_Array retVal;

    retVal.count = num;
    retVal.data = (int*) malloc(sizeof(int) * num);
    for (i=0; i < num; i++) {
	retVal.data[i] = start+i;
    }
    return(retVal);
}


void test::triggerAsyncUpcall(int val)
{
    asyncUpcall(val);
}
