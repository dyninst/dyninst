#include <stdio.h>
#include <stdlib.h>
#include "test3.SRVR.h"
#include <string.h>

main(int argc, char *argv[])
{
    int fd;
    int eid;
    int ret;
    test *tp = 0;

    if (argc == 1) {
	fd = 0;
    } else {
	printf("remote start not supported\n");
	exit(-1);
    }
    tp = new test(0, NULL, NULL);

    // now go into main loop
    while(1) {
	ret = tp->mainLoop();	
	if (ret < 0) {
	    // assume the client has exited, and leave.
	    exit(-1);
	}
    }
}

int test::classArrayTest(sStruct_Array sArr)
{
  int i, sum = 0;

  for (i=0; i<sArr.count; ++i)
    {
      sum += sArr.data[i].computer;
      assert (!strcmp(sArr.data[i].name, "happy"));
    }
  return sum;
}
void test::nullNull()
{
    return;
}

int test::intNull()
{
    return(0);
}

float test::floatFloat(float f)
{
    return(f);
}

double test::doubleDouble(double d)
{
    return(d);
}

void test::nullStruct(intStruct s)
{
    return;
}

int test::VintString(String s)
{
    return(strlen(s));
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

sStruct test::msTest(sStruct input)
{
    sStruct ret;
    ret.computer = 1;
    ret.name = strdup("happy");
    return ret;
}

fStruct test::fTest(fStruct input)
{
    return input;
}
