#include <rpc/xdr.h>
#include <unistd.h>
#include <assert.h>
#include "test4.CLNT.h"

String str1 = "A Test String with server words in it";
String str2 = "Different String";

int numbers[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
int_Array vect;

extern void serverMainFunc(int);

main()
{
    int i;
    int fd;
    int eid;
    int tid;
    int total;
    intStruct is;
    testUser *remote;

    // do a thread create???
    thr_create(0, 0, serverMainFunc, thr_self(), 0, (unsigned int *) &tid);

    remote = new testUser(tid);

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

    for (i=0; i < 10000; i++) {
	remote->add(1, 0);
    }

    printf("ThreadPC test1 passed\n");
    delete (remote);
}


void testUser::asyncUpcall(int val)
{
    printf("asyncUpcall called with value = %d\n", val);
}
