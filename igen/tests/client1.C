#include <unistd.h>
#include <assert.h>
#include "test1.CLNT.h"
#include <string.h>

char * str1 = "A Test string with server words in it";
char * str2 = "Different string";

int numbers[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
int_Array vect;

void echoA(char_PTR_Array &in) {
  int i;
  for (i=0; i<in.count; ++i)
    printf("Element %d = %s\n", i, in.data[i]);
}

void freeA(char_PTR_Array &in) {
  int i;
  for (i=0; i<in.count; ++i) {
    delete (in.data[i]);
    in.data[i] = 0;
  }
}

void echoCSA(charStruct_Array *in) {
  int i;
  for (i=0; i<in->count; ++i)
    printf("Item %d = %s\n", i, in->data[i].cp);
}

void freeCSA(charStruct_Array *in) {
  int i;
  for (i=0; i<in->count; ++i) {
    delete (in->data[i].cp);
    in->data[i].cp = 0;
  }
}

main()
{
    int i;
    int fd;
    int pid;
    int eid;
    int total;
    intStruct is;
    testUser *remote;

    fd = RPCprocessCreate(pid, "localhost", "", "server1");
    if (fd < 0) {
	perror("process Create");
	exit(-1);
    }

    remote = new testUser(fd, NULL, NULL);


    remote->nullNull();

    assert(remote->intNull() == 0);

    remote->nullStruct(is);

    assert(strlen(str1) == remote->intString(str1));

    str2 = remote->stringString(str1);
    assert(!strcmp(str2, str1));

    assert(remote->add(1, 1) == 2);
    assert(remote->add(-1, -13) == -14);

    vect.count = sizeof(numbers)/sizeof(int);
    vect.data = numbers;
    for (i=0, total = 0; i < vect.count; i++) {
	total += numbers[i];
    }
    assert(remote->sumVector(vect) == total);

    remote->triggerAsyncUpcall(-10);
    assert(-10 == remote->triggerSyncUpcall(-10));

    for (i=0; i < 10000; i++) {
	remote->add(1, i);
    }
    printf("RPC test1 passed\n");

    char_PTR_Array cpa, res;
    cpa.count = 2;
    cpa.data = new char*[2];
    cpa.data[0] = strdup("Happy");
    cpa.data[1] = strdup("Sad");
    echoA(cpa);
    res = remote->echoCPA(cpa);
    echoA(res);

    freeA(cpa);
    freeA(res);
    delete [] (cpa.data);
    delete [] res.data;

    charStruct cs, *csp, *csp1;
    charStruct_Array csa, *csap, *csap1;

    csp = new charStruct;
    cs.cp = strdup("Happy");
    *csp = remote->echoCS(cs);
    csp1 = remote->echoCSP(&cs);

    delete cs.cp;
    delete csp->cp;
    delete csp;
    delete csp1->cp;
    delete csp1;

    csa.count = 2;
    csa.data = new charStruct[2];

    csa.data[0].cp = strdup("Hospital");
    csa.data[1].cp = strdup("Accident");

    csap = new charStruct_Array;
    *csap = remote->echoCSA(csa);
    csap1 = remote->echoCSAP(csap);

    freeCSA(&csa);
    delete csa.data;
    freeCSA(csap1);
    freeCSA(csap);
    delete csap->data;
    delete csap1->data;
    delete csap;
    delete csap1;

    remote->asyncClient();
    sleep(3);
    delete remote;
}

void testUser::asyncUpcall(int val)
{
    printf("asyncUpcall called with value = %d\n", val);
}

int testUser::SyncUpcall(int val)
{
    printf("SyncUpcall called with value = %d\n", val);
    return val;
}
