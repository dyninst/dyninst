
#include <stdio.h>
#include <stdlib.h>
#include "classT.thread.SRVR.h"

void *serverMainFunc(void *parentId)
{
  int fd;
  int ret;
  testT *tp;

  printf("In server2\n");
  tp = new testT((int) parentId);
  
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

intStruct testT::echoIntStruct(intStruct parm)
{
	return (parm);
}

twoStruct testT::echoTwoStruct(twoStruct parm)
{
	return parm;
}

intStruct *testT::echoIntStructP(intStruct *parm)
{
	return (parm);
}

twoStruct *testT::echoTwoStructP(twoStruct *parm)
{
	return parm;
}

metric *testT::testClassPtr(metric_PTR parm)
{
    return (parm);
}

void testT::quit()
{
    exit(0);
}

metric *testT::echoMetricPtrPtr (metric *parm)
{
  metric *p;

  p = new metric;
  *p = *parm;
  return p;
}
