
#include <stdio.h>
#include "class.xdr.CLNT.h"

int noSignal=1; 


int intStructEqual (intStruct one, intStruct two)
{
    return (one.style == two.style);
}

int twoStructEqual (twoStruct one, twoStruct two)
{
    return ((one.style == two.style) &&
	    intStructEqual(one.i,two.i));
}

int metricEqual (metric one, metric two)
{
    return ((one.i == two.i) &&
	    (one.j == two.j));
}

int metricAEqual (metricA one, metricA two)
{
  return ((one.x == two.x) &&
	  (one.y == two.y) &&
	  (one.i == two.i) &&
	  (one.j == two.j));
}

int metricAPEqual (metricA *one, metricA *two)
{
  return ((one->x == two->x) &&
	  (one->y == two->y) &&
	  (one->i == two->i) &&
	  (one->j == two->j));
}

int metricBEqual (metricB one, metricB two)
{
  return ((one.a == two.a) &&
	  (one.b == two.b) &&
	  (one.i == two.i) &&
	  (one.j == two.j));
}

main()
{
    int fd, pid;
    testUser *remote;
    twoStruct two, resTwo, *ts1, *ts2;
    intStruct one, resOne, *is1, *is2;
    metric oneM, twoM, *pM, *qM;
    metricA A1, A2, *pA, *p2A;
    metricB B1, B2, *pB;

    one.style = 3;
    two.style = 4;
    two.i.style = 5;
    oneM.i = 3;
    oneM.j = 'c';

    A1.x = 'c'; A1.y = 'f'; A1.i = 1; A1.j = 7;
    B1.a = 4; B1.c = 7; B1.b = 16; B1.i = 0; B1.j = 2;

    fd = RPCprocessCreate(pid, "localhost", "", "advS");

    remote = new testUser(fd, NULL, NULL);

    resOne = remote->echoIntStruct(one);
    printf("echoIntStruct %s\n",
	   intStructEqual(resOne, one) ? "OK" : "FAILS");

    is1 = remote->echoIntStructP(&resOne);
    printf("echoIntStructP %s\n",
	   intStructEqual(resOne, *is1) ? "OK" : "FAILS");
    delete is1;

    resTwo = remote->echoTwoStruct(two);
    printf("echoTwoStruct %s\n",
	   twoStructEqual(resTwo, two) ? "OK" : "FAILS");

    ts1 = remote->echoTwoStructP(&two);
    printf("echoTwoStructP %s\n",
	   twoStructEqual(*ts1, two) ? "OK" : "FAILS");
    delete ts1;

    twoM = remote->testClass(oneM);
    printf("testClass %s\n", metricEqual(oneM, twoM) ? "OK" : "FAILS");

    pM = remote->testClassPtr(&oneM);
    printf("testClassPtr %s\n", metricEqual(oneM, *pM) ? "OK" : "FAILS");
    delete(pM); pM = 0;

    A2 = remote->echoMetricA(A1);
    printf("echoMetricA %s\n", metricAEqual(A1, A2) ? "OK" : "FAILS");

    B2 = remote->echoMetricB(B1);
    printf("echoMetricB %s\n", metricBEqual(B1, B2) ? "OK" : "FAILS");

    pM = remote->echoMetricPtrStatic(oneM);
    printf("echoMetricPtrStatic %s\n", metricEqual(*pM, oneM) ? "OK" : "FAILS");
    delete(pM); pM = 0;

    pM = remote->echoMetricPtrPtr(&oneM);
    printf("echoMetricPtrPtr %s\n", metricEqual(*pM, oneM)
			       ? "OK" : "FAILS");

    pA = new metricA;
    pA->x = (char) 0;
    pA->y = (char) 1;
    pA->i = 2;
    pA->j = (unsigned char) 3;

    
    printf("0=metric(base), 1=metricA(derived), 2=metricB(derived)\n");
    printf("pM->do_it() = %d  -- should be 0\n", pM->do_it());
    printf("pA->do_it() = %d  -- should be 1\n", pA->do_it());
    qM = pA;
    printf("qM->do_it() = %d  -- should be 1\n\n", qM->do_it());

    qM = remote->testClassPtr(pM);
    printf("These should be equal (to 0) \n");
    printf("pM->do_it() = %d\n", pM->do_it());
    printf("qM->do_it() = %d\n\n", qM->do_it());
    delete (qM);

    qM = remote->testClassPtr(pA);
    printf("These should be equal (to 1) \n");
    printf("pA->do_it() = %d\n", pA->do_it());
    printf("qM->do_it() = %d\n\n", qM->do_it());

    delete (qM);
    delete(pM); pM = 0;
    delete(pA);

    remote->startUpcalls();
    while (noSignal)
      remote->awaitResponce(-1);
    

    remote->quit();
    printf("Bye now\n");
    delete(remote);
}

void testUser::signalUpcallsDone() {
  noSignal = 0;
}

void testUser::upcallOK() {
  printf("UPCALL OK\n");
}

void testUser::upcallNotOK() {
  printf("UPCALL NOT OK\n");
}

metric *testUser::echoMetricP(metric *get) {
  return get;
}

metric *testUser::echoMetricPF(metric *get) {
  metric *ret;
  ret = new metric;
  ret->i = get->i;
  ret->j = get->j;
  return ret;
}
