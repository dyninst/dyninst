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

#include "test1.xdr.CLNT.h"
#include "common/h/Timer.h"

pdstring str1 = "A Test string with server words in it";
pdstring str2 = "Different string";

void echoA(pdvector<pdstring> &in) {
  for (int i=0; i<in.size(); ++i)
    cout << "Element " << i << " = " << in[i] << endl;
}

void echoCSA(pdvector<T_test1::charStruct> &in) {
  for (int i=0; i<in.size(); ++i)
    cout << "Item " << i << " = " << in[i].cp << endl;
}

static bool seen_done = false;
static bool seen_rapid_upcall=false;
static bool seen_random_upcall = false;
static unsigned count = 0;
timer perf_timer;

T_test1::derClass::derClass() { }
T_test1::basicClass::basicClass() { }

main(int argc, char *argv[])
{
    int i, fd, pid, total;
    T_test1::intStruct is;
    testUser *remote;

    pdvector<pdstring> arg_list;
    // note -- starting on remote hosts will not work yet
    pdstring host = "localhost";
    if (argc > 1)
      host = argv[1];

    fd = RPCprocessCreate(pid, host, "", "server1", arg_list);
    if (fd < 0) {
	perror("process Create");
	exit(-1);
    }

    remote = new testUser(fd, NULL, NULL, false);

    int tries; 
    for (tries=0; tries<100; tries++)
      remote->nullNull();
    cerr << "nullNull ok\n";

    for (tries=0; tries<100; tries++)
      assert(remote->intNull() == 0);
    cerr << "intNull ok\n";

    for (tries=0; tries<100; tries++)
      remote->nullStruct(is);
    cerr << "nullStruct ok\n";

    pdstring longStr = "abacasdflkjasrlejrlkjavljadl;jasdjwe;rawojiasdvjal;jwer;lkjadsjasvl;jopejrl;jwel;jasdjasfjl;wejrl;asjd;jasdfjajfjer;lasdf;jasdfjasrjawrjas;jfajfl;ajf;lajwer;ljwaer;laflk;jasdf;jasdfjasdfjasdkfjwejrafjafjafjl;roaiwejfoifjasdjweroijwajasdjasvja;rjwrjasfjajvl;jowerjoiargj;dasgjl;l;kwrl;jfasdjfajfl;fj;sd;fj;fsdlksdfljkajforijsdogjlsl;ertokpsdrjioerjsfdjweiojsdflkjweojsdfojsdjerojsdjsdfjwejdsojsdggjgjsdfjsdfjerjegeigiolkdfsgasdgjerjerjwtgjodfsgjjasdjasfjajfl;ajgoierjtergjaejasdasjdlfjoarjioasdjdagjasoigjagoirghasdhasfjaofjpweasfjasdjasdjfwjfadfjoagjfoargjopasdfgjopjfof;jf;jasdf;jrwjaejadsojassjjsjasdjasdjarwjioaejasdjiasdjhashasdhasdhasdopfhasoidjfaodfjpfoasjdjasdjasfjofjodgjajasdjasdjajgogofjasdjasdjfasfjasofjojofioasdgjasgjasgjadgjadgjajgaopgjajgasjadjasdjasjasjasjasjaopjfasfasflkasjfgjdfjasdjfasjfoifoidfjasdjasd;jfjasdjasdjasjfojgofgjasjasdjasjfawjfgaojgl;fjagjasdjfas;jfajfl;f;jasdjasdjajflfjf;asdfaljfofglhasdfgalfl;asdjasdfjasdfja;fj;asf;asjdfl;jag;okahsdhasdfhasfofosdfljkasdlfjasjfafaffjaslkjajafjasdjkasd;jkasdfasdfasdf;jasdf;lkajsdjasdf;ljasdjasdjasdjasfja;fjadf;jasdjas;fjasjfajfl;f;jdfjasdfjasfjajfjfasdjasdjasd;jasddsslfg;fgj;fg;jasdf;dasjasdjasd;jasfjasjfas;fjasjasd;jasd;jasd;jasd;jasd;fjasd;jasdjasdjasdljk";

    assert(longStr.length() == remote->intString(longStr));
    cerr << "intString: long string test ok, length = " << longStr.length() << endl;

    pdstring strNull;
    assert(strNull.length() == remote->intString(strNull));
    cerr << "intString: null string test ok\n";
	
    for (tries=0; tries<100; tries++)
      assert(str1.length() == remote->intString(str1));
    cerr << "intString ok\n";

    str2 = remote->stringString(strNull);
    assert(str2 == strNull);
    cerr << "stringString: null string test ok\n";

    const unsigned max_tries = 500;

    cerr << "Performance testing: \n";
    perf_timer.clear();
    perf_timer.start();
    for (tries=0; tries<max_tries; tries++) 
      remote->nullNull();
    perf_timer.stop();
    cerr << max_tries << " null rpc's in " << perf_timer.wsecs() << " seconds\n";
    cerr << ((double)max_tries)/perf_timer.wsecs() << " null rpc's per second\n\n";

    pdstring echo_me = "happy", res_me;
    perf_timer.clear();
    perf_timer.start();
    for (tries=0; tries<max_tries; tries++) {
      res_me = remote->stringString(echo_me);
      assert(res_me == echo_me);
    }
    perf_timer.stop();
    cerr << max_tries << " echo string rpc's in " << perf_timer.wsecs() << " seconds\n";
    cerr << ((double)max_tries)/perf_timer.wsecs() << " echo string rpc's per second\n\n";

    perf_timer.clear();
    perf_timer.start();
    for (tries=0; tries<max_tries; tries++) {
      res_me = remote->stringStringRef(echo_me);
      assert(res_me == echo_me);
    }
    perf_timer.stop();
    cerr << max_tries << " echo string by ref rpc's in " << perf_timer.wsecs() << " seconds\n";
    cerr << ((double)max_tries)/perf_timer.wsecs() << " echo string by ref rpc's per second\n\n";

    perf_timer.clear();
    perf_timer.start();
    for (tries=0; tries<max_tries; tries++) {
      res_me = remote->stringStringRef(longStr);
      assert(res_me == longStr);
    }
    perf_timer.stop();
    cerr << max_tries << " echo long string rpc's in " << perf_timer.wsecs() << " seconds\n";
    cerr << ((double)max_tries)/perf_timer.wsecs() << " echo long string rpc's per second\n\n";

    perf_timer.clear();
    perf_timer.start();
    for (tries=0; tries<max_tries; tries++) {
      res_me = remote->stringStringRef(longStr);
      assert(res_me == longStr);
    }
    perf_timer.stop();
    cerr << max_tries << " echo long string by ref rpc's in " << perf_timer.wsecs() << " seconds\n";
    cerr << ((double)max_tries)/perf_timer.wsecs() << " echo long string by ref rpc's per second\n\n";

    pdvector<pdstring> arg, result;
    arg += "happy"; arg += "sad"; arg += "honest"; arg += "memory_hog";
    arg += "design"; arg += "good"; arg += "bad"; arg += "functional";
    arg += "efficient"; arg += "debug"; arg += "core_dump"; arg += "fault";
    arg += "colors"; arg += "widgets"; arg += "stupendous";
    for (int q=0; q<100; ++q)
      arg += "filler";

    perf_timer.clear();
    perf_timer.start();
    for (tries=0; tries<max_tries; tries++)
      result = remote->norefVector(arg);
    perf_timer.stop();
    cerr << max_tries << " echo pdvector<pdstring> rpc's in " << perf_timer.wsecs() << " seconds\n";
    cerr << ((double)max_tries)/perf_timer.wsecs() << " echo pdvector<pdstring> rpc's per second\n\n";

    perf_timer.clear();
    perf_timer.start();
    for (tries=0; tries<max_tries; tries++)
      result = remote->refVector(arg);
    perf_timer.stop();
    cerr << max_tries << " echo pdvector<pdstring> by ref rpc's in " << perf_timer.wsecs() << " seconds\n";
    cerr << ((double)max_tries)/perf_timer.wsecs() << " echo pdvector<pdstring> by ref rpc's per second\n\n";

    
    for (tries=0; tries<100; tries++) {
      assert(remote->add(1, 1) == 2);
      assert(remote->add(-1, -13) == -14);
    }
    cerr << "add ok\n";

    pdvector<int> vect;
    vect += 1; vect += 2; vect += 3; vect += 4; vect += 5;
    vect += 6; vect += 7; vect += 8; vect += 9; vect += 10;
    for (i=0, total = 0; i < vect.size(); i++) 
	total += vect[i];
    
    for (tries=0; tries<100; tries++) {
      assert(remote->sumVector(vect) == total);
      assert(remote->sumVectorPtr(&vect) == total);
    }
    cerr << "sumVector ok\n";

    remote->triggerAsyncUpcall(-10);
    remote->wait_for(T_test1::asyncUpcall_REQ);
    for (i=0; i < 10000; i++)
	assert(remote->add(1, i) == (1+i));
    printf("RPC test1 passed\n");

    pdvector<pdstring> cpa, res;
    cpa += "Happy"; cpa += "Sad";
    echoA(cpa);
    res = remote->echoCPA(cpa);
    echoA(res);

    assert(res.size() == cpa.size());
    for (int ea=0; ea<res.size(); ea++)
      assert(res[ea] == cpa[ea]);

    pdvector<pdstring> *vs = remote->echoCPAPtr(&cpa);
    assert(vs->size() == cpa.size());
    for (int l=0; l<vs->size(); l++)
      assert((*vs)[l] == cpa[l]);
    delete vs;

    T_test1::charStruct cs, csp;
    pdvector<T_test1::charStruct> csa, csap;

    cs.cp = "Happy";
    csp = remote->echoCS(cs);
    assert (csp.cp == cs.cp);

    csp.cp = "Hospital"; csa += csp;
    csp.cp = "Accident"; csa += csp;

    csap = remote->echoCSA(csa);
    assert (csap.size() == csa.size());
    for (ea=0; ea<csa.size(); ea++)
      assert(csap[ea].cp == csa[ea].cp);

    T_test1::charStruct *csptr;
    csptr = remote->echoCSP((T_test1::charStruct*)NULL);
    assert(!csptr);
    cerr << "Passed null structure ok\n";

    csptr = remote->echoCSP(&cs);
    assert(csptr->cp == cs.cp);
    cerr << "Passed structure pointer ok\n";
    delete csptr;

    T_test1::basicClass b, b1, *bp; T_test1::derClass d, d1, *dp;
    b.b = false; d.u = 3019; d.b = false;
    bp = NULL;
    bp = remote->echoClass((T_test1::basicClass*)NULL);
    assert(!bp);
    cerr << "Null class passed ok\n";
    bp = remote->echoClass(&b);
    assert(!bp->b);
    assert(bp->getId() == T_test1::basicClass_id);
    delete bp;
    cerr << "passing class pointers ok\n";

    b1 = remote->echoBClass(b);
    d1 = remote->echoDClass(d);
    assert(!b1.b); 
    assert(d1.u == 3019); 
    assert(!d.b);
    cerr << "passing classes ok\n";

    bp = remote->echoClass(&d1);
    assert(bp->getId() == T_test1::derClass_id);
    delete bp;
    cerr << "passing derived class ok\n";

    pdvector<pdstring> vs1, vs2;
    vs1 += "/Mark"; vs1 += "/Is"; vs1 += "/Bored";
    vs2 += "/What"; vs2 += "/To";
    pdvector<T_test1::resStruct> vres, answer;
    T_test1::resStruct restr;
    restr.parts = vs1;
    restr.handle = 0;
    vres += restr;
    restr.parts = vs2;
    vres += restr;
    cerr << "Echoing pdvector of structures\n";
    answer = remote->echoResStruct(vres);
    assert(answer[0].parts == vs1);
    assert(answer[1].parts == vs2);
    cerr << "Echoing pdvector of structures --> passed\n";
    
    cerr << "triggering async upcalls\n";
    T_test1::boolStruct bs;
    bs.b = true;
    assert (remote->boolToString(bs) == "true");
    bs.b = false;
    assert (!seen_rapid_upcall);
    assert (remote->boolToString(bs) == "false");
    assert (!seen_rapid_upcall);
    cerr << "handling async upcalls\n";
    while(remote->is_buffered(T_test1::rapidUpcall_REQ))
      remote->wait_for(T_test1::rapidUpcall_REQ);

    assert(count == 999);
    assert (!seen_random_upcall);

    bool one_there = true; 
    while (one_there) {
      one_there = false;
      if (remote->is_buffered(T_test1::up1_REQ)) {
	remote->wait_for(T_test1::up1_REQ); one_there = true;
      }
      if (remote->is_buffered(T_test1::up2_REQ)) {
	remote->wait_for(T_test1::up2_REQ); one_there = true;
      }
      if (remote->is_buffered(T_test1::up3_REQ)) {
	remote->wait_for(T_test1::up3_REQ); one_there = true;
      }
      if (remote->is_buffered(T_test1::up4_REQ)) {
	remote->wait_for(T_test1::up4_REQ); one_there = true;
      }
    }
    remote->wait_for(T_test1::up_done_REQ);
    remote->asyncClient();
    sleep(3);
    delete remote;
}

void testUser::rapidUpcall(unsigned val) {
  if (!seen_rapid_upcall) {
    seen_rapid_upcall = true;
    count = val;
  } else {
    assert (count == (val - 1));
    count++;
  }
}

void testUser::asyncUpcall(int val) {
  printf("asyncUpcall called with value = %d\n", val);
}

unsigned total = 0;

void testUser::up1() {
  total++;
  seen_random_upcall = true;
}

void testUser::up2() {
  total++;
  seen_random_upcall = true;
}

void testUser::up3() {
  total++;
  seen_random_upcall = true;
}

void testUser::up4() {
  total++;
  seen_random_upcall = true;
}

void testUser::up_done(u_int tot) {
  assert(tot == total);
  seen_done = true;
}
