
/* Test application (Mutatee) */

/* $Id: test7.mutatee.c,v 1.1 2002/06/26 21:15:04 schendel Exp $ */

#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/msg.h>

#if defined(i386_unknown_nt4_0) && !defined(__GNUC__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#define getpid _getpid
#else
#include <unistd.h>
#endif

#ifdef __cplusplus
int mutateeCplusplus = 1;
#else
int mutateeCplusplus = 0;
#endif

#ifndef COMPILER
#define COMPILER ""
#endif
const char *Builder_id=COMPILER; /* defined on compile line */

/* control debug printf statements */
#define dprintf if (debugPrint) printf
int debugPrint = 0;

#define TRUE    1
#define FALSE   0

struct  msgSt {
  long  mtype;     /* message type */
  char  mtext[1];  /* message text */
};
typedef struct msgSt ipcMsg;

int msgid = -1;

void sendDoneMsg() {
  const long int DONEMSG = 4321;
  ipcMsg A;
  A.mtype = DONEMSG;
  /* fprintf(stderr,"sent done msg\n"); */
  if(msgsnd(msgid, &A, sizeof(char), 0) == -1) {
    perror("msgSend: msgsnd failed");
  }
}

void setupMessaging() {
  key_t msgkey = 1234;

  if((msgid = msgget(msgkey, 0666)) == -1) {
    perror("Couldn't get messaging");
    exit(1);
  }
}

int globalVariable7_1 = 123;
int globalVariable7_2 = 159;
int globalVariable7_3 = 246;
int globalVariable7_4 = 789;
int globalVariable7_5 = 7;
int globalVariable7_6 = 21;
int globalVariable7_7 = 1;
int globalVariable7_8 = 1;
int globalVariable7_9 = 1;
int dummyVal = 0;

void func7_1() { 
  dummyVal += 10;
}

void func7_2() { 
  dummyVal += 10;
}

void func7_3() { 
  dummyVal += 10;
}

void func7_4() { 
  dummyVal += 10;
}

void func7_5() { 
  dummyVal += 10;
}

void func7_6() { 
  dummyVal += 10;
}

void func7_7() { 
  dummyVal += 10;
}

void func7_8() { 
  dummyVal += 10;
}

void func7_9() { 
  dummyVal += 10;
}


void delay(int m) {
  int i,j;
  for(i=0; i<m; i++)
    for(j=0; j<m; j++) 
      ;
  assert(i>0 && j>0);
}

void mutateeMAIN()
{
#if defined(i386_unknown_nt4_0)
  return;
#endif
  int pid;
  setupMessaging();
  /* fprintf(stderr, "mutatee:  starting fork\n"); */
  pid = fork();
  /* fprintf(stderr, "mutatee:  stopping fork\n"); */

  /* mutatee will get paused here, temporarily, when the mutator receives
     the postForkCallback */

  if (pid == 0) {   /* child */
    /* fprintf(stderr, "child, done with fork\n"); */
    func7_1();
    func7_2();
    func7_3();
    func7_4();
    func7_5();
    func7_6();
    func7_7();
    func7_8();
    func7_9();
    /* delay(100);*/
    sendDoneMsg();
  } else if(pid > 0) {
    /* fprintf(stderr, "parent, done with fork\n"); */
    func7_1();
    func7_2();
    func7_3();
    func7_4();
    func7_5();
    func7_6();
    func7_7();
    func7_8();
    func7_9();
  } else if(pid < 0) {
    fprintf(stderr, "error on fork\n");
    exit(pid);  /* error case */
  }
  sleep(60);  /* will be killed by mutator */
  printf("dummyVal: %d\n",dummyVal);  /* so code doesn't get optimized away */
}

int main(int argc, char *argv[])
{                                       
    int i, j;

    for (i=1; i < argc; i++) {
      if (!strcmp(argv[i], "-verbose")) {
	debugPrint = TRUE;
      }
    }
    if(debugPrint) {
        printf("Mutatee %s [%s]:\"%s\"\n", argv[0], 
                mutateeCplusplus ? "C++" : "C", Builder_id);
    }
    mutateeMAIN();
    return 0;
}

