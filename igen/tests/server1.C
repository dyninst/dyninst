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

string test::stringString(string s) {
    return(s);
}

int test::add(int a, int b) {
    return(a+b);
}

float test::fadd(float a, float b) {
    return(a+b);
}

int test::sumVector(vector<int> nums) {
   int total=0;
   for (int i=0; i < nums.size(); i++) {
       total += nums[i];
   }
   return(total);
}

int test::sumVectorPtr(vector<int> *nums) {
   int total=0;
   for (int i=0; i < (*nums).size(); i++) {
       total += (*nums)[i];
   }
   delete nums;
   return(total);
}

vector<int> test::retVector(int num, int start) {
    vector<int> retVal;

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

vector<string> test::echoCPA(vector<string> input) {
  return input;
}

vector<string> *test::echoCPAPtr(vector<string> *input) {
  return input;
}

T_test1::charStruct test::echoCS(T_test1::charStruct csin) {
  return csin;
}

vector<T_test1::charStruct> test::echoCSA(vector<T_test1::charStruct> csa) {
  return csa;
}

string test::boolToString(T_test1::boolStruct bs) {
  if (bs.b) {
    for (unsigned i=0; i<1000; i++)
      rapidUpcall(i);
    return "true";
  } else
    return "false";
}
