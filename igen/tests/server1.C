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

#include "test1.xdr.SRVR.h"

main(int argc, char *argv[])
{
    int fd;

    int ret;
    test *tp;

    // int z=1;
    // while(z) ;

    if (argc == 1) {
	fd = 0;
    } else {
	printf("remote start not supported\n");
        abort();
	exit(-1);
    }
    tp = new test(0, NULL, NULL, false);

    // now go into main loop
    while(1) {
	ret = tp->waitLoop();	
	if (ret < 0) {
	    // assume the client has exited, and leave.
	    exit(-1);
	}
    }
}


void test::nullNull() {
    return;
}

int test::intNull() {
    return(0);
}

void test::nullStruct(T_test1::intStruct s) {
    return;
}

int test::intString(pdstring s) {
    return(s.length());
}

pdvector<pdstring> test::refVector(pdvector<pdstring> &vec) {
  return vec;
}
pdvector<pdstring> test::norefVector(pdvector<pdstring> vec) {
  return vec;
}

pdstring test::stringString(pdstring s) {
    return(s);
}

pdstring test::stringStringRef(pdstring &s) {
    return(s);
}

int test::add(int a, int b) {
    return(a+b);
}

float test::fadd(float a, float b) {
    return(a+b);
}

int test::sumVector(pdvector<int> nums) {
   int total=0;
   for (int i=0; i < nums.size(); i++) {
       total += nums[i];
   }
   return(total);
}

int test::sumVectorPtr(pdvector<int> *nums) {
   int total=0;
   for (int i=0; i < (*nums).size(); i++) {
       total += (*nums)[i];
   }
   delete nums;
   return(total);
}

pdvector<int> test::retVector(int num, int start) {
    pdvector<int> retVal;

    for (int i=0; i<num; i++)
      retVal += start++;
    return(retVal);
}


void test::triggerAsyncUpcall(int val) {
    asyncUpcall(val);
}

void exitNow(test *me) {
  delete me;
  exit(1);
}

void test::asyncClient() {
  printf("In test::asyncClient(), goodbye!\n");
  exitNow(this);
}

pdvector<pdstring> test::echoCPA(pdvector<pdstring> input) {
  return input;
}

pdvector<pdstring> *test::echoCPAPtr(pdvector<pdstring> *input) {
  return input;
}

T_test1::charStruct test::echoCS(T_test1::charStruct csin) {
  return csin;
}

pdvector<T_test1::charStruct> test::echoCSA(pdvector<T_test1::charStruct> csa) {
  return csa;
}

unsigned total = 0;

pdstring test::boolToString(T_test1::boolStruct bs) {
  if (bs.b) {

    for (unsigned i=0; i<1000; i++) {
      up1(); total++;
      up2(); total++;
      up3(); total++;
      up4(); total++;
    }

    for (i=0; i<1000; i++)
      rapidUpcall(i);

    for (i=0; i<1000; i++) {
      up1(); total++;
      up2(); total++;
      up3(); total++;
      up4(); total++;
    }

    up_done(total);

    return "true";
  } else
    return "false";
}

T_test1::basicClass *test::echoClass(T_test1::basicClass *b) {
  return b;
}

T_test1::basicClass test::echoBClass(T_test1::basicClass b) {
  return b;
}

T_test1::derClass test::echoDClass(T_test1::derClass d) {
  return d;
}

T_test1::charStruct *test::echoCSP(T_test1::charStruct *cs) {
  return cs;
}

T_test1::basicClass::basicClass() { }
T_test1::derClass::derClass() { }

pdvector<T_test1::resStruct> test::echoResStruct(pdvector<T_test1::resStruct> rs) {
  return rs;
}
