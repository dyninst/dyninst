extern "C" {
#include <rpc/xdr.h>
}

#include <unistd.h>
#include <assert.h>
#include "test3.CLNT.h"
#include <string.h>

String str1 = "A Test String with server words in it";
String str2 = "Different String";
String str3 = "";

int numbers[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
int_Array vect;

main()
{
    sStruct Sret, Ssend;
    fStruct fret;
    int i;
    int fd;
    int eid;
    int total;
    intStruct is;
    testUser *remote = 0;
    float pracF = 0.3945;
    double pracD = 0.3945;
    is.style = 5;
    fd = RPCprocessCreate(&eid, "localhost", "", "server3");
    if (fd < 0) {
	perror("process Create");
	exit(-1);
    }

    remote = new testUser(fd, NULL, NULL);


    remote->nullNull();

    assert(remote->intNull() == 0);
    assert(remote->doubleDouble(pracD) == pracD);
    assert(remote->floatFloat(pracF) == pracF);
    fret.fval = pracF; fret.dval = pracD;
    fret = remote->fTest(fret);
    assert((fret.fval == pracF) && (fret.dval == pracD));
    remote->nullStruct(is);

    sStruct_Array sArr;
    sArr.count = 5;
    sArr.data = new sStruct[5];
    int j, sum1, sum2;
    sum1 = 0;
    for (j=0; j<sArr.count; ++j)
      {
	 sArr.data[j].computer = j;
	 sArr.data[j].name = strdup("happy");
         sum1 += sArr.data[j].computer;
      }

    sum2 = remote->classArrayTest(sArr);
    assert(sum1 == sum2);
    for (j=0; j<sArr.count; ++j)
       free( sArr.data[j].name);
    delete [] sArr.data;

    assert(strlen(str1) == remote->intString(str1));
    assert(strlen(str3) == remote->intString(str3));
    assert(-1 == remote->intString((char*) 0));

    str2 = remote->stringString(str1);
    assert(!strcmp(str2, str1));

    assert(remote->add(1, 1) == 2);
    assert(remote->add(-1, -13) == -14);

    Ssend.computer = 2;
    Ssend.name = strdup("happy");
    Sret = remote->msTest(Ssend);
    free(Sret.name);
    free(Ssend.name);

    vect.count = sizeof(numbers)/sizeof(int);
    vect.data = numbers;
    for (i=0, total = 0; i < vect.count; i++) {
	total += numbers[i];
    }
    assert(remote->sumVector(vect) == total);

    remote->triggerAsyncUpcall(-10);

    for (i=0; i < 10000; i++) {
	remote->add(1, i);
    }
    printf("RPC test1 passed\n");
    delete (remote);
}


void testUser::asyncUpcall(int val)
{
    printf("asyncUpcall called with value = %d\n", val);
}

void testUser::VasyncUpcall(int val)
{
    printf("asyncUpcall called with value = %d\n", val);
}
