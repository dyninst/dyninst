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

int test::intString(pdstring *s) {
  return(s->length());
}

pdstring *test::stringString(pdstring *s) { return(s); }

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
