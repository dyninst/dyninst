#include <stdio.h>
#include <stdlib.h>
#include "test2.thread.SRVR.h"
#include "thread/h/thread.h"

static bool seen_async = false;

void *serverMainFunc(void *parentId)
{
  T_test2::message_tags ret;
  test *tp;
  
  tp = new test((int) parentId);
  cerr << "Server: test created\n"; cerr.flush();

  // now go into main loop
  while(1) {
    ret = tp->waitLoop(false, T_test2::last);	
    if (ret == T_test2::error) {
      printf(" mainLoop returned < 0, bye bye\n");
      // assume the client has exited, and leave.
      assert(0);
      thr_exit(-1);
    }
  }
  return 0;
}

void test::nullNull() { return; }

int test::intNull() { return(0); }

void test::nullStruct(T_test2::intStruct s) { return; }

int test::intString(string *s) {
  return(s->length());
}

string *test::stringString(string *s) { return(s); }

int test::add(const int a, const int b) { return(a+b); }

int test::sumVector(pdvector<int> *nums) {
  int i, total;
  for (i=0, total=0; i < nums->size(); i++)
    total += (*nums)[i];
  return(total);
}

pdvector<int> *test::retVector(int num, int start) {
  int i;
  pdvector<int> *retVal;

  retVal = new pdvector<int>;
  for (i=0; i < num; i++)
    (*retVal)[i] = start+i;

  return(retVal);
}

void test::hangup() {
  seen_async = true;
}

void test::triggerAsyncUpcall(int val) {
  asyncUpcall(val);
}

T_test2::s2 *test::isSp(T_test2::s2 *parm) {
  return parm;
}

other *test::testOther(other *o) {
  other *op = new other;
  op->i = o->i;
  op->k = o->k;
  delete o;
  return (op);
}

scope::fruit test::echoFruit(scope::fruit f) {
  return f;
}
