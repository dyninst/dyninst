#include <stdio.h>
#include <stdlib.h>
#include "test2.SRVR.h"

void *serverMainFunc(void *parentId)
{
    int fd;
    int eid;
    int ret;
    test *tp;

    printf("In server2\n");
    tp = new test((int) parentId);

    printf("In server2 -- after test, before main loop\n");
    // now go into main loop
    while(1) {
	ret = tp->mainLoop();	
	if (ret < 0) {
            printf(" mainLoop returned < 0, bye bye\n");
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


void test::triggerAsyncUpcall(int val)
{
    asyncUpcall(val);
}
