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

int test::intString(String s)
{
    return(strlen(s));
}

String test::stringString(String s)
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

void test::triggerSyncUpcall(int val)
{
    syncUpcall(val);
}


void test::triggerAsyncUpcall(int val)
{
    asyncUpcall(val);
}
