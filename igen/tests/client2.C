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

#include <unistd.h>
#include <assert.h>
#include "test2.thread.CLNT.h"
#include "thread/h/thread.h"

pdstring str1 = "A Test String with server words in it";

extern void *serverMainFunc(void *);

bool s2Equal (T_test2::s2 &one, T_test2::s2 &two)
{
  if (one.i1 != two.i1)
    return false;
  else if (one.i2 != two.i2)
    return false;
  else 
    return true;
}
static int stid;
static bool asyncCalled = false;
static int ctid, dtid;
static unsigned asyncCount = 0;

void *clientMainFunc(void *parentId) {
  int i, total;
  T_test2::intStruct is;
  testUser *remote;
  pdstring *str2;

  thr_create(0, 0, serverMainFunc, (void*) thr_self(), 0, (thread_t*) &stid);
  printf("In client2 after thread create, %d\n", stid);

  remote = new testUser(stid);

  int tries;
  pdvector<int> vect;
  vect += 1; vect += 2; vect += 3; vect += 4; vect += 5; 
  vect += 6; vect += 7; vect += 8; vect += 9; vect += 10;

  scope::fruit fr1 = scope::orange;
  assert (fr1 == remote->echoFruit(fr1));

  other o, *op;  o.i = 3; o.k = 3.0;
  op = remote->testOther(&o);
  assert(op->i = 3);  assert(op->k == 3.0);
  delete op;

  for (tries=0; tries<100; tries++)
    remote->nullNull();
  cerr << "nullNull ok\n";

  for (tries=0; tries<100; tries++)
    assert(remote->intNull() == 0);
  cerr << "intNull ok\n";

  for (tries=0; tries<100; tries++)
    remote->nullStruct(is);
  cerr << "nullStruct ok\n";

  for (tries=0; tries<100; tries++) {
    int len = remote->intString(&str1);
    assert(str1.length() == len);
  }
  cerr << "intString ok\n";

  for (tries=0; tries<100; tries++) {
    str2 = remote->stringString(&str1);
    assert(*str2 == str1);
  }
  cerr << "stringString ok\n";

  for (tries=0; tries<100; tries++) {
    assert(remote->add(1, 1) == 2);
    assert(remote->add(-1, -13) == -14);
  }
  cerr << "add ok\n";

  total = 0;
  for (int s=0; s<vect.size(); s++)
    total += vect[s];

  assert(remote->sumVector(&vect) == total);
  cout << "sumVector ok\n";

  T_test2::s2 one, two, *p1;

  one.i1 = 3; one.i2 = 7;
  two.i1 = 3; two.i2 = 7;
  p1 = remote->isSp(&two);
  assert (s2Equal(*p1, two));
  cout << "struct equal ok\n";

  for (i=0; i<100; i++)
    remote->triggerAsyncUpcall(-10);
  cout << "triggerAsync ok\n";

  for (i=0; i < 100; i++)
    assert(remote->add(1, 0) == 1);
  cout << "add ok\n";

  unsigned tag = MSG_TAG_ANY; int from;
  while ((from = msg_poll(&tag, 0)) != THR_ERR) {
    if (remote->isValidTag((T_test2::message_tags)tag))
      if (remote->waitLoop(false, T_test2::last) == T_test2::error) {
	cerr << "error\n";
	assert(0);
      }
    tag = MSG_TAG_ANY;
  }

  cout << "at hangup\n";
  remote->hangup();

  if (asyncCalled)
    printf("ThreadPC test1 passed\n");
  else
    cerr << "error, async upcall not handled\n";
  delete remote;
  return 0;
}


main() {
  thread_t tid; void *ptr;
  thr_create(0, 0, clientMainFunc, (void*) thr_self(), 0, (thread_t*) &ctid);
  thr_join(ctid, &tid, &ptr);
  cout << "In main, all tests passed\n";
}


void testUser::asyncUpcall(int val) {
  asyncCalled = true;
  printf("asyncUpcall called with value = %d: count = %d\n", val, asyncCount);
  asyncCount++;
}
