
#include "class.SRVR.h"

int signal = 0;

int metricEq (metric *a, metric *b) {
  if (!a || !b)
    return 0;
  if (a->i != b->i)
    return 0;
  if (a->j != b->j)
    return 0;
  return 1;
}

// I am started by the client
main()
{
  test *tp;
  metric *pM, *rM, M;

  M.i = 9;  M.j = 4;
  // fd 0 is set up for the socket with the client
  tp = new test(0, NULL, NULL);

  while (1) {
    tp->mainLoop();
    // signal gets set on an upcall
    if (signal) {
      pM = tp->echoMetricP(&M);
      if (metricEq(pM, &M)) 
	tp->upcallOK();
      else
	tp->upcallNotOK();
      rM = tp->echoMetricPF(&M);
      if (metricEq(rM, &M)) 
	tp->upcallOK();
      else
	tp->upcallNotOK();
      delete pM;
      delete rM;
      tp->signalUpcallsDone();
    }
  }
}

intStruct test::echoIntStruct(intStruct parm)
{
	return (parm);
}

twoStruct test::echoTwoStruct(twoStruct parm)
{
	return parm;
}

intStruct *test::echoIntStructP(intStruct *parm)
{
	return (parm);
}

twoStruct *test::echoTwoStructP(twoStruct *parm)
{
	return parm;
}

metric test::testClass(metric parm)
{
    return parm;
}

metric *test::testClassPtr(metric_PTR parm)
{
    return (parm);
}

void test::quit()
{
    delete this;
    exit(0);
}

metricA test::echoMetricA(metricA parm)
{
  return parm;
}

metricB test::echoMetricB(metricB parm)
{
  return parm;
}

metric *test::echoMetricPtrStatic (metric parm)
{
  metric *p;

  p = new metric;
  *p = parm;
  return p;
}

metric *test::echoMetricPtrPtr (metric *parm)
{
  return parm;
}

void test::startUpcalls() {
  signal = 1;
}


