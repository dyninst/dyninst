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

int test::intString(string s) {
    return(s.length());
}

pdvector<string> test::refVector(pdvector<string> &vec) {
  return vec;
}
pdvector<string> test::norefVector(pdvector<string> vec) {
  return vec;
}

string test::stringString(string s) {
    return(s);
}

string test::stringStringRef(string &s) {
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

pdvector<string> test::echoCPA(pdvector<string> input) {
  return input;
}

pdvector<string> *test::echoCPAPtr(pdvector<string> *input) {
  return input;
}

T_test1::charStruct test::echoCS(T_test1::charStruct csin) {
  return csin;
}

pdvector<T_test1::charStruct> test::echoCSA(pdvector<T_test1::charStruct> csa) {
  return csa;
}

unsigned total = 0;

string test::boolToString(T_test1::boolStruct bs) {
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
