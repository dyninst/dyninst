#include <unistd.h>
#include <assert.h>
#include "test2.CLNT.h"
#include "thread/h/thread.h"

char *str1 = "A Test String with server words in it";
char *str2 = "Different String";

int numbers[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
int_Array vect;

extern void *serverMainFunc(void *);

int s2Equal (s2 one, s2 two)
{
  if (one.i1 != two.i1)
    return 0;
  else if (one.i2 != two.i2)
    return 0;
  else 
    return 1;
}

main()
{
    int i;
    int fd;
    int eid;
    int tid;
    int total;
    intStruct is;
    testUser *remote;

    printf("In client2 before thread create\n");
    // do a thread create???
    thr_create(0, 0, serverMainFunc, (void *) thr_self(), (unsigned int) 0, 
	(unsigned int *) &tid);

    printf("In client2 after thread create\n");
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

    s2 one, two, three, *p1, *p2, *p3;

    one.i1 = 3; one.i2 = 7;
    two.i1 = 3; two.i2 = 7;
    three = remote->isS(one);
    assert(s2Equal(three, two));
    p2 = &one;
    p1 = remote->isSp(p2);
    assert (s2Equal(*p1, *p2));
    remote->triggerAsyncUpcall(-10);

    for (i=0; i < 10000; i++) {
	remote->add(1, 0);
    }

    printf("ThreadPC test1 passed\n");
    delete remote;
}


void testUser::asyncUpcall(int val)
{
    printf("asyncUpcall called with value = %d\n", val);
}
