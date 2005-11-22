#include "BPatch.h"
#include "BPatch_thread.h"

/* Test7 Definitions */
typedef enum { Parent_p, Child_p } procType;
typedef enum { PreFork, PostFork } forkWhen;

struct  msgSt {
  long  mtype;     /* message type */
  char  mtext[1];  /* message text */
};
typedef struct msgSt ipcMsg;
typedef struct msgSt ipcMsg;

/* Test7 Functions */
int setupMessaging(int *msgid);
bool doError(bool *passedTest, bool cond, const char *str);
bool verifyProcMemory(BPatch_thread *appThread, const char *name,
                      int expectedVal, procType proc_type);
bool verifyProcMemory(const char *name, BPatch_variableExpr *var,
                      int expectedVal, procType proc_type);
void showFinalResults(bool passedTest, int i);

