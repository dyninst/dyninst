
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include "classT.thread.CLNT.h"

extern void serverMainFunc(void *pid);

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
    int fd, tid;
    testTUser *remote;
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

    A1.x = 'c'; A1.y = 'f';
    B1.c = 7; B1.b = 16;


    printf("In client2 before thread create\n");
    // do a thread create???
    thr_create(0, 0, serverMainFunc, (void *) thr_self(), (unsigned int) 0, 
	(unsigned int *) &tid);

    printf("In client after thread create\n");
    remote = new testTUser(tid);

    resOne = remote->echoIntStruct(one);
    printf("echoIntStruct %s\n",
	   intStructEqual(resOne, one) ? "OK" : "FAILS");

    is1 = remote->echoIntStructP(&resOne);
    printf("echoIntStructP %s\n",
	   intStructEqual(resOne, *is1) ? "OK" : "FAILS");

    resTwo = remote->echoTwoStruct(two);
    printf("echoTwoStruct %s\n",
	   twoStructEqual(resTwo, two) ? "OK" : "FAILS");

    ts1 = remote->echoTwoStructP(&two);
    printf("echoTwoStructP %s\n",
	   twoStructEqual(*ts1, two) ? "OK" : "FAILS");

    pM = remote->testClassPtr(&oneM);
    printf("testClassPtr %s\n", metricEqual(oneM, *pM) ? "OK" : "FAILS");

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

    qM = remote->testClassPtr(pA);
    printf("These should be equal (to 1) \n");
    printf("pA->do_it() = %d\n", pA->do_it());
    printf("qM->do_it() = %d\n\n", qM->do_it());

    remote->quit();
    printf("Bye now\n");
    delete(remote);
}
