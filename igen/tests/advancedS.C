
#include "class.SRVR.h"


// I am started by the client
main()
{
  test *tp;

  // fd 0 is set up for the socket with the client
  tp = new test(0, NULL, NULL);

  while (1)
	tp->mainLoop();
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
  metric *p;

  p = new metric;
  *p = *parm;
  return p;
}
