#include "test1.xdr.CLNT.h"

string str1 = "A Test string with server words in it";
string str2 = "Different string";

void echoA(vector<string> &in) {
  for (int i=0; i<in.size(); ++i)
    cout << "Element " << i << " = " << in[i] << endl;
}

void echoCSA(vector<T_test1::charStruct> &in) {
  for (int i=0; i<in.size(); ++i)
    cout << "Item " << i << " = " << in[i].cp << endl;
}

static bool seen_rapid_upcall=false;
static unsigned count = 0;

main()
{
    int i, fd, pid, total;
    T_test1::intStruct is;
    testUser *remote;

    vector<string> arg_list;
    fd = RPCprocessCreate(pid, "localhost", "", "server1", arg_list);
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

    string longStr = "abacasdflkjasrlejrlkjavljadl;jasdjwe;rawojiasdvjal;jwer;lkjadsjasvl;jopejrl;jwel;jasdjasfjl;wejrl;asjd;jasdfjajfjer;lasdf;jasdfjasrjawrjas;jfajfl;ajf;lajwer;ljwaer;laflk;jasdf;jasdfjasdfjasdkfjwejrafjafjafjl;roaiwejfoifjasdjweroijwajasdjasvja;rjwrjasfjajvl;jowerjoiargj;dasgjl;l;kwrl;jfasdjfajfl;fj;sd;fj;fsdlksdfljkajforijsdogjlsl;ertokpsdrjioerjsfdjweiojsdflkjweojsdfojsdjerojsdjsdfjwejdsojsdggjgjsdfjsdfjerjegeigiolkdfsgasdgjerjerjwtgjodfsgjjasdjasfjajfl;ajgoierjtergjaejasdasjdlfjoarjioasdjdagjasoigjagoirghasdhasfjaofjpweasfjasdjasdjfwjfadfjoagjfoargjopasdfgjopjfof;jf;jasdf;jrwjaejadsojassjjsjasdjasdjarwjioaejasdjiasdjhashasdhasdhasdopfhasoidjfaodfjpfoasjdjasdjasfjofjodgjajasdjasdjajgogofjasdjasdjfasfjasofjojofioasdgjasgjasgjadgjadgjajgaopgjajgasjadjasdjasjasjasjasjaopjfasfasflkasjfgjdfjasdjfasjfoifoidfjasdjasd;jfjasdjasdjasjfojgofgjasjasdjasjfawjfgaojgl;fjagjasdjfas;jfajfl;f;jasdjasdjajflfjf;asdfaljfofglhasdfgalfl;asdjasdfjasdfja;fj;asf;asjdfl;jag;okahsdhasdfhasfofosdfljkasdlfjasjfafaffjaslkjajafjasdjkasd;jkasdfasdfasdf;jasdf;lkajsdjasdf;ljasdjasdjasdjasfja;fjadf;jasdjas;fjasjfajfl;f;jdfjasdfjasfjajfjfasdjasdjasd;jasddsslfg;fgj;fg;jasdf;dasjasdjasd;jasfjasjfas;fjasjasd;jasd;jasd;jasd;jasd;fjasd;jasdjasdjasdljk";

    assert(longStr.length() == remote->intString(longStr));
    cerr << "intString: long string test ok, length = " << longStr.length() << endl;

    string strNull;
    assert(strNull.length() == remote->intString(strNull));
    cerr << "intString: null string test ok\n";
	
    for (tries=0; tries<100; tries++)
      assert(str1.length() == remote->intString(str1));
    cerr << "intString ok\n";

    str2 = remote->stringString(strNull);
    assert(str2 == strNull);
    cerr << "stringString: null string test ok\n";

    for (tries=0; tries<100; tries++) {
      str2 = remote->stringString(str1);
      assert(str2 == str1);
    }
    cerr << "stringString ok\n";

    for (tries=0; tries<100; tries++) {
      assert(remote->add(1, 1) == 2);
      assert(remote->add(-1, -13) == -14);
    }
    cerr << "add ok\n";

    vector<int> vect;
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

    for (i=0; i < 10000; i++)
	assert(remote->add(1, i) == (1+i));
    printf("RPC test1 passed\n");

    vector<string> cpa, res;
    cpa += "Happy"; cpa += "Sad";
    echoA(cpa);
    res = remote->echoCPA(cpa);
    echoA(res);

    assert(res.size() == cpa.size());
    for (int ea=0; ea<res.size(); ea++)
      assert(res[ea] == cpa[ea]);

    vector<string> *vs = remote->echoCPAPtr(&cpa);
    assert(vs->size() == cpa.size());
    for (int l=0; l<vs->size(); l++)
      assert((*vs)[l] == cpa[l]);
    delete vs;

    T_test1::charStruct cs, csp;
    vector<T_test1::charStruct> csa, csap;

    cs.cp = "Happy";
    csp = remote->echoCS(cs);
    assert (csp.cp == cs.cp);

    csp.cp = "Hospital"; csa += csp;
    csp.cp = "Accident"; csa += csp;

    csap = remote->echoCSA(csa);
    assert (csap.size() == csa.size());
    for (ea=0; ea<csa.size(); ea++)
      assert(csap[ea].cp == csa[ea].cp);

    cerr << "triggering async upcalls\n";
    T_test1::boolStruct bs;
    bs.b = true;
    assert (remote->boolToString(bs) == "true");
    bs.b = false;
    assert (!seen_rapid_upcall);
    assert (remote->boolToString(bs) == "false");

    cerr << "handling async upcalls\n";
    while (remote->buffered_requests())
      remote->process_buffered();
    assert(count == 999);

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

