#include <rpc/xdr.h>
#include <unistd.h>
#include <assert.h>
#include "testPVM.CLNT.h"

char * str1 = "A Test String with server words in it";
char * str2 = "Different String";

int numbers[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
int_Array vect;

main()
{
    int i;
    int fd;
    int eid;
    int total;
    intStruct is;
    testUser *remote;

    remote = new testUser(NULL, "serverPVM", NULL, 0);
    // remote = new testUser();
    
    if (remote->get_error() == -1)
      {
	printf ("client could not get parent id \n");
	exit(0);
      }

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

    for (i=0; i < 500; i++) {
	remote->add(1, i);
    }
    printf("RPC test1 passed\n");
    delete (remote);
    pvm_exit();
}


void testUser::asyncUpcall(int val)
{
    printf("asyncUpcall called with value = %d\n", val);
}
