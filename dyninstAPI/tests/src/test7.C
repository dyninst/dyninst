// $Id: test7.C,v 1.7 2003/07/18 15:44:11 schendel Exp $
//

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/msg.h>
#include <iostream>
#include <errno.h>
#ifdef i386_unknown_nt4_0
#include <windows.h>
#include <winbase.h>
#else
#include <unistd.h>
#endif

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "test_util.h"


const char *mutateeNameRoot = "test7.mutatee";

int inTest;		// current test #
int expectError;
int debugPrint = 0; // internal "mutator" tracing
int errorPrint = 0; // external "dyninst" tracing (via errorFunc)

bool forceRelocation = false;  // Force relocation upon instrumentation

#define dprintf if (debugPrint) printf

bool runAllTests = true;
const unsigned int MAX_TEST = 9;
bool runTest[MAX_TEST+1];
bool passedTest[MAX_TEST+1];

BPatch *bpatch;

typedef enum { Parent_p, Child_p } procType;
typedef enum { PreFork, PostFork } forkWhen;

const char *procName[2] = { "parent", "child" };

/*
 * Given a string variable name and an expected value, lookup the varaible
 *    in the process, and verify that the value matches.
 *
 */
bool verifyProcMemory(BPatch_thread *appThread, const char *name,
                      int expectedVal, procType proc_type)
{
   BPatch_image *appImage = appThread->getImage();

   if (!appImage) {
      dprintf("unable to locate image for %d\n", appThread->getPid());
      return false;
   }
   
   BPatch_variableExpr *var = appImage->findVariable(name);
   if (!var) {
      dprintf("unable to located variable %s in child\n", name);
      return false;
   }

   int actualVal;
   var->readValue(&actualVal);
   
   if (expectedVal != actualVal) {
      fprintf(stderr,"*** for %s (%s), expected val = %d, but actual was %d\n",
	      name, procName[proc_type], expectedVal, actualVal);
      return false;
   } else {
      dprintf("verified %s (%s) was = %d\n", name, procName[proc_type], 
	      actualVal);
      return true;
   }
}

/*
 * Given a string variable name and an expected value, lookup the varaible
 *    in the process, and verify that the value matches.
 *
 */
bool verifyProcMemory(const char *name, BPatch_variableExpr *var,
                      int expectedVal, procType proc_type)
{
   int actualVal;
   var->readValue(&actualVal);
   
   if (expectedVal != actualVal) {
      fprintf(stderr,"*** for %s (%s), expected val = %d, but actual was %d\n",
	      name, procName[proc_type], expectedVal, actualVal);
      return false;
   } else {
      dprintf("verified %s (%s) was = %d\n", name, procName[proc_type], 
	      actualVal);
      return true;
   }
}


bool doError(int testNum, bool cond, const char *str) {
   if(cond == true) {
      fprintf(stderr, str);
      passedTest[testNum] = false;
      return true;
   }
   return false;
}

struct  msgSt {
  long  mtype;     /* message type */
  char  mtext[1];  /* message text */
};
typedef struct msgSt ipcMsg;

int msgid = -1;

int checkForDoneMsg() {
  ipcMsg A;
  int bytesRead = 0, ret = 0;
  const long int DONEMSG = 4321;

  A.mtype = 0;
  if((bytesRead = msgrcv(msgid, &A, sizeof(char), DONEMSG,IPC_NOWAIT)) == -1) {
    if(errno == ENOMSG)
      return 0;
    else
      perror("msgRecv: msgrcv failed");
  }
  if(A.mtype == DONEMSG)
    ret = 1;
  else
    printf("Invalid message sent to mutator (%ld)\n",A.mtype);
  return ret;
}

void waitForDoneMsg() {
   while(! checkForDoneMsg());
}

void closeMessaging() {
  msgctl(msgid, IPC_RMID, NULL);   
}

void setupMessaging() {
  key_t msgkey = 1234;

  if((msgid = msgget(msgkey, 0666|IPC_CREAT)) == -1) {
    perror("Couldn't create messaging");
    exit(1);
  }
}

/* Make sure deleting a snippet in a parent process doesn't delete the
   snippet in the child process.

   parent: snippetHandleA  = insert snippetA at pointA
   child:  snippetHandleA' = pointA.getCurrentSnippets()
   --- fork ---
   parent: deleteSnippet( snippetHandleA )
   --- run  ---
   child:  verify snippetHandleA' still ran
*/

void prepareTestCase1(procType proc_type, BPatch_thread *thread, forkWhen when)
{
   static BPatchSnippetHandle *parSnippetHandle1;

   if(proc_type == Parent_p  &&  when == PreFork) {
      BPatch_image *parImage = thread->getImage();
      
      BPatch_Vector<BPatch_function *> found_funcs;
      const char *inFunction = "func7_1";
      if ((NULL == parImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
	fprintf(stderr, "    Unable to find function %s\n",
		inFunction);
	exit(1);
      }
      
      if (1 < found_funcs.size()) {
	fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
		__FILE__, __LINE__, found_funcs.size(), inFunction);
      }
      
      BPatch_Vector<BPatch_point *> *point7_1p = found_funcs[0]->findPoint(BPatch_entry);

      if(doError(1, !point7_1p || ((*point7_1p).size() == 0),
		 "  Unable to find entry point to \"func7_1\".\n")) return;

      BPatch_variableExpr *var7_1p = 
	 parImage->findVariable("globalVariable7_1");
      if(doError(1, (var7_1p==NULL),
		 "  Unable to locate variable globalVariable7_1\n")) return;

      BPatch_arithExpr expr7_1p(BPatch_assign, *var7_1p,BPatch_constExpr(321));

      parSnippetHandle1 =
	 thread->insertSnippet(expr7_1p, *point7_1p, BPatch_callBefore);
      if(doError(1, (parSnippetHandle1 == NULL),
		"  Unable to insert snippet into parent for test 1\n")) return;
   } else if(proc_type == Parent_p  &&  when == PostFork) {
      thread->deleteSnippet(parSnippetHandle1);
   }
}

void checkTestCase1(procType proc_type, BPatch_thread *thread) {
   if(proc_type == Parent_p) {
      if(! verifyProcMemory(thread, "globalVariable7_1", 123, proc_type)) {
	 passedTest[1] = false;
      }
   } else if(proc_type == Child_p) {
      if(! verifyProcMemory(thread, "globalVariable7_1", 321, proc_type)) {
	 passedTest[1] = false;
      }
   }
}

/* Make sure deleting a snippet in a child process doesn't delete the
   snippet in the parent process.

   parent: snippetHandleA  = insert snippetA at pointA
   child:  snippetHandleA' = pointA.getCurrentSnippets()
   --- fork ---
   child:  deleteSnippet( snippetHandleA' )
   --- run  ---
   parent: verify snippetHandleA still ran
*/

void prepareTestCase2(procType proc_type, BPatch_thread *thread, forkWhen when)
{
   static BPatchSnippetHandle *parSnippetHandle2;

   if(proc_type == Parent_p  &&  when == PreFork) {
      BPatch_image *parImage = thread->getImage();
      
      BPatch_Vector<BPatch_function *> found_funcs;
      const char *inFunction = "func7_2";
      if ((NULL == parImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
	fprintf(stderr, "    Unable to find function %s\n",
		inFunction);
	exit(1);
      }
      
      if (1 < found_funcs.size()) {
	fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
		__FILE__, __LINE__, found_funcs.size(), inFunction);
      }
      
      BPatch_Vector<BPatch_point *> *points7_2p = found_funcs[0]->findPoint(BPatch_entry);

      if(doError(2, !points7_2p || ((*points7_2p).size() == 0),
		 "  Unable to find entry point to \"func7_2\".\n")) return;
      BPatch_point *point7_2p = (*points7_2p)[0];

      BPatch_variableExpr *var7_2p = 
	 parImage->findVariable("globalVariable7_2");
      if(doError(2, (var7_2p==NULL),
		 "  Unable to locate variable globalVariable7_2\n")) return;

      BPatch_arithExpr expr7_2p(BPatch_assign, *var7_2p,BPatch_constExpr(951));

      parSnippetHandle2 =
	 thread->insertSnippet(expr7_2p, *point7_2p, BPatch_callBefore);
   } else if(proc_type == Child_p  &&  when == PostFork) {
      BPatch_image *childImage = thread->getImage();      

      BPatch_Vector<BPatch_function *> found_funcs;
      const char *inFunction = "func7_2";
      if ((NULL == childImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
	fprintf(stderr, "    Unable to find function %s\n",
		inFunction);
	exit(1);
      }
      
      if (1 < found_funcs.size()) {
	fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
		__FILE__, __LINE__, found_funcs.size(), inFunction);
      }
      
      BPatch_Vector<BPatch_point *> *points7_2c = found_funcs[0]->findPoint(BPatch_entry);

      if(doError(2, !points7_2c || ((*points7_2c).size() == 0),
		 "  Unable to find entry point to \"func7_2\".\n")) return;
      BPatch_point *point7_2c = (*points7_2c)[0];

      BPatch_Vector<BPatchSnippetHandle *> childSnippets =
	 point7_2c->getCurrentSnippets();
      if(doError(2, (childSnippets.size()==0),
		 " No snippets were found at func7_2\n")) return;

      for(unsigned i=0; i<childSnippets.size(); i++) {
	 bool result = thread->deleteSnippet(childSnippets[i]);
	 if(result == false) {
	    fprintf(stderr, "  error, couldn't delete snippet\n");
	    passedTest[2] = false;
	    return;
	 }
      }
   }
}

void checkTestCase2(procType proc_type, BPatch_thread *thread) {
   if(proc_type == Parent_p) {
      if(! verifyProcMemory(thread, "globalVariable7_2", 951, proc_type)) {
	 passedTest[2] = false;
      }
   } else if(proc_type == Child_p) {
      if(! verifyProcMemory(thread, "globalVariable7_2", 159, proc_type)) {
	 passedTest[2] = false;
      }
   }
}

/* Delete both a parent and a child snippet, and make sure both are getting
   deleted.

   parent: snippetHandleA  = insert snippetA at pointA
   --- fork ---
   parent: deleteSnippet( snippetHandleA )
   child:  snippetHandleA' = pointA.getCurrentSnippets()
   child:  deleteSnippet( snippetHandleA' )
   --- run  ---
   parent: verify snippetHandleA didn't run
   child:  verify snippetHandleA didn't run
*/

void prepareTestCase3(procType proc_type, BPatch_thread *thread, forkWhen when)
{
   static BPatchSnippetHandle *parSnippetHandle3;

   if(proc_type == Parent_p  &&  when == PreFork) {
      BPatch_image *parImage = thread->getImage();

      BPatch_Vector<BPatch_function *> found_funcs;
      const char *inFunction = "func7_3";
      if ((NULL == parImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
	fprintf(stderr, "    Unable to find function %s\n",
		inFunction);
	exit(1);
      }
      
      if (1 < found_funcs.size()) {
	fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
		__FILE__, __LINE__, found_funcs.size(), inFunction);
      }
      
      BPatch_Vector<BPatch_point *> *points7_3p = found_funcs[0]->findPoint(BPatch_entry);

      if(doError(3, !points7_3p || ((*points7_3p).size() == 0),
		 "  Unable to find entry point to \"func7_3\".\n")) return;
      BPatch_point *point7_3p = (*points7_3p)[0];

      BPatch_variableExpr *var7_3p = 
	 parImage->findVariable("globalVariable7_3");
      if(doError(3, (var7_3p==NULL),
		 "  Unable to locate variable globalVariable7_3\n")) return;

      BPatch_arithExpr expr7_3p(BPatch_assign, *var7_3p,BPatch_constExpr(642));

      parSnippetHandle3 =
	 thread->insertSnippet(expr7_3p, *point7_3p, BPatch_callBefore);
   } else if(proc_type == Parent_p  &&  when == PostFork) {
      bool result = thread->deleteSnippet(parSnippetHandle3);
      if(result == false) {
	 fprintf(stderr, "  error, couldn't delete snippet\n");
	 passedTest[3] = false;
	 return;
      }
   } else if(proc_type == Child_p  &&  when == PostFork) {
      BPatch_image *childImage = thread->getImage();

      BPatch_Vector<BPatch_function *> found_funcs;
      const char *inFunction = "func7_3";
      if ((NULL == childImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
	fprintf(stderr, "    Unable to find function %s\n",
		inFunction);
	exit(1);
      }
      
      if (1 < found_funcs.size()) {
	fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
		__FILE__, __LINE__, found_funcs.size(), inFunction);
      }
      
      BPatch_Vector<BPatch_point *> *points7_3c = found_funcs[0]->findPoint(BPatch_entry);
      
      if(doError(3, !points7_3c || ((*points7_3c).size() == 0),
		 "  Unable to find entry point to \"func7_3\".\n")) return;
      BPatch_point *point7_3c = (*points7_3c)[0];

      BPatch_Vector<BPatchSnippetHandle *> childSnippets =
	 point7_3c->getCurrentSnippets();
      if(doError(3, (childSnippets.size()==0),
		 " No snippets were found at func7_3\n")) return;

      for(unsigned i=0; i<childSnippets.size(); i++) {
	 bool result = thread->deleteSnippet(childSnippets[i]);
	 if(result == false) {
	    fprintf(stderr, "  error, couldn't delete snippet\n");
	    passedTest[3] = false;
	    return;
	 }
      }
   }
}

void checkTestCase3(procType proc_type, BPatch_thread *thread) {
   if(proc_type == Parent_p) {
      if(! verifyProcMemory(thread, "globalVariable7_3", 246, proc_type)) {
	 passedTest[3] = false;
      }
   } else if(proc_type == Child_p) {
      if(! verifyProcMemory(thread, "globalVariable7_3", 246, proc_type)) {
	 passedTest[3] = false;
      }
   }
}

/* Insert a snippet into a child process, which hasn't inherited
   any snippets from the parent process. 
*/

/*
   var7_5 initial value = 789
   --- fork ---
   child:  insert snippet A, var7_5 += 211
   --- run  ---
   child:  verify snippet A ran, (ie. var7_5 == 1000)
*/

void prepareTestCase4(procType proc_type, BPatch_thread *thread, forkWhen when)
{
   const int TN = 4;
   static BPatchSnippetHandle *parSnippetHandle4;

   if(proc_type == Child_p  &&  when == PostFork) {
      BPatch_image *childImage = thread->getImage();

      BPatch_Vector<BPatch_function *> found_funcs;
      const char *inFunction = "func7_4";
      if ((NULL == childImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
	fprintf(stderr, "    Unable to find function %s\n",
		inFunction);
	exit(1);
      }
      
      if (1 < found_funcs.size()) {
	fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
		__FILE__, __LINE__, found_funcs.size(), inFunction);
      }
      
      BPatch_Vector<BPatch_point *> *points7_4c = found_funcs[0]->findPoint(BPatch_entry);
      if(doError(TN, !points7_4c || ((*points7_4c).size() == 0),
		 "  Unable to find entry point to \"func7_4\".\n")) return;
      BPatch_point *point7_4c = (*points7_4c)[0];

      BPatch_variableExpr *var7_4c = 
	 childImage->findVariable("globalVariable7_4");
      if(doError(TN, (var7_4c==NULL),
		 "  Unable to locate variable globalVariable7_4\n")) return;

      BPatch_arithExpr a_expr7_4c(BPatch_plus, *var7_4c,BPatch_constExpr(211));
      BPatch_arithExpr b_expr7_4c(BPatch_assign, *var7_4c, a_expr7_4c);
      parSnippetHandle4 =
	thread->insertSnippet(b_expr7_4c, *point7_4c, BPatch_callBefore);
   }
}

void checkTestCase4(procType proc_type, BPatch_thread *thread) {
   if(proc_type == Parent_p) {
      if(! verifyProcMemory(thread, "globalVariable7_4", 789, proc_type)) {
	 passedTest[4] = false;
      }
   } else if(proc_type == Child_p) {
      if(! verifyProcMemory(thread, "globalVariable7_4", 1000, proc_type)) {
	 passedTest[4] = false;
      }
   }
}


/* Add two snippets to an already existing snippet in the parent and child
   and see if all of the snippets get run.

   parent/child: globalVariable7_5 initial value = 7
   parent: snippetHandleA  = insert snippetA at pointA   += 9
   --- fork ---
   child:  snippetHandleA' = pointA.getCurrentSnippets()
   parent: add snippetB (+= 11);  add snippetC (+= 13);
   child:  add snippetB' (+= 5);  add snippetC' (+= 3);
   --- run  ---
   parent: verify snippetA, snippetB, and snippetC ran     7+9+11+13 == 40
   child:  verify snippetA', snippetB', and snippetC' ran  7+9+5+3   == 24
*/

void prepareTestCase5(procType proc_type, BPatch_thread *thread, forkWhen when)
{
   const int TN = 5;
   static BPatchSnippetHandle *parSnippetHandle5;

   if(proc_type == Parent_p  &&  when == PreFork) {
      BPatch_image *parImage = thread->getImage();

      BPatch_Vector<BPatch_function *> found_funcs;
      const char *inFunction = "func7_5";
      if ((NULL == parImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
	fprintf(stderr, "    Unable to find function %s\n",
		inFunction);
	exit(1);
      }
      
      if (1 < found_funcs.size()) {
	fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
		__FILE__, __LINE__, found_funcs.size(), inFunction);
      }
      
      BPatch_Vector<BPatch_point *> *points7_5p = found_funcs[0]->findPoint(BPatch_entry);

      if(doError(TN, !points7_5p || ((*points7_5p).size() == 0),
		 "  Unable to find entry point to \"func7_5\".\n")) return;
      BPatch_point *point7_5p = (*points7_5p)[0];

      BPatch_variableExpr *var7_5p = 
	 parImage->findVariable("globalVariable7_5");
      if(doError(TN, (var7_5p==NULL),
		 "  Unable to locate variable globalVariable7_5\n")) return;

      BPatch_arithExpr expr7_5p(BPatch_plus, *var7_5p, BPatch_constExpr(9));
      BPatch_arithExpr b_expr7_5p(BPatch_assign, *var7_5p, expr7_5p);
      parSnippetHandle5 =
	 thread->insertSnippet(b_expr7_5p, *point7_5p, BPatch_callBefore);
   } else if(proc_type == Parent_p  &&  when == PostFork) {
      BPatch_image *parImage = thread->getImage();

      BPatch_Vector<BPatch_function *> found_funcs;
      const char *inFunction = "func7_5";
      if ((NULL == parImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
	fprintf(stderr, "    Unable to find function %s\n",
		inFunction);
	exit(1);
      }
      
      if (1 < found_funcs.size()) {
	fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
		__FILE__, __LINE__, found_funcs.size(), inFunction);
      }
      
      BPatch_Vector<BPatch_point *> *points7_5p = found_funcs[0]->findPoint(BPatch_entry);

      if(doError(TN, !points7_5p || ((*points7_5p).size() == 0),
		 "  Unable to find entry point to \"func7_5\".\n")) return;
      BPatch_point *point7_5p = (*points7_5p)[0];

      BPatch_variableExpr *var7_5p = 
	 parImage->findVariable("globalVariable7_5");
      if(doError(TN, (var7_5p==NULL),
		 "  Unable to locate variable globalVariable7_5\n")) return;

      BPatch_arithExpr a_expr7_5p(BPatch_plus, *var7_5p, BPatch_constExpr(11));
      BPatch_arithExpr b_expr7_5p(BPatch_assign, *var7_5p, a_expr7_5p);
      parSnippetHandle5 =
	 thread->insertSnippet(b_expr7_5p, *point7_5p, BPatch_callBefore,
			       BPatch_lastSnippet);

      BPatch_arithExpr c_expr7_5p(BPatch_plus, *var7_5p, BPatch_constExpr(13));
      BPatch_arithExpr d_expr7_5p(BPatch_assign, *var7_5p, c_expr7_5p);
      parSnippetHandle5 =
	 thread->insertSnippet(d_expr7_5p, *point7_5p, BPatch_callBefore);
   } else if(proc_type == Child_p  &&  when == PostFork) {
      BPatch_image *childImage = thread->getImage();

      BPatch_Vector<BPatch_function *> found_funcs;
      const char *inFunction = "func7_5";
      if ((NULL == childImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
	fprintf(stderr, "    Unable to find function %s\n",
		inFunction);
	exit(1);
      }
      
      if (1 < found_funcs.size()) {
	fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
		__FILE__, __LINE__, found_funcs.size(), inFunction);
      }
      
      BPatch_Vector<BPatch_point *> *points7_5c = found_funcs[0]->findPoint(BPatch_entry);

      if(doError(TN, !points7_5c || ((*points7_5c).size() == 0),
		 "  Unable to find entry point to \"func7_5\".\n")) return;
      BPatch_point *point7_5c = (*points7_5c)[0];

      BPatch_variableExpr *var7_5c = 
	 childImage->findVariable("globalVariable7_5");
      if(doError(TN, (var7_5c==NULL),
		 "  Unable to locate variable globalVariable7_5\n")) return;

      BPatch_arithExpr a_expr7_5c(BPatch_plus, *var7_5c, BPatch_constExpr(5));
      BPatch_arithExpr b_expr7_5c(BPatch_assign, *var7_5c, a_expr7_5c);
      parSnippetHandle5 =
	thread->insertSnippet(b_expr7_5c, *point7_5c, BPatch_callBefore,
			      BPatch_lastSnippet);
      BPatch_arithExpr c_expr7_5c(BPatch_plus, *var7_5c, BPatch_constExpr(3));
      BPatch_arithExpr d_expr7_5c(BPatch_assign, *var7_5c, c_expr7_5c);
      parSnippetHandle5 =
	thread->insertSnippet(d_expr7_5c, *point7_5c, BPatch_callBefore);
   }
}

void checkTestCase5(procType proc_type, BPatch_thread *thread) {
   const int TN = 5;
   if(proc_type == Parent_p) {
      if(! verifyProcMemory(thread, "globalVariable7_5", 40, proc_type)) {
	 passedTest[TN] = false;
      }
   } else if(proc_type == Child_p) {
      if(! verifyProcMemory(thread, "globalVariable7_5", 24, proc_type)) {
	 passedTest[TN] = false;
      }
   }
}

/* Run a oneTimeCode in both the parent and child and see if they both
   happen.

   parent/child: globalVariable7_6 initial value = 21

   parent: run one time code, value += 5
   child:  run one time code, value += 9
   parent: value == 26
   child:  value == 30
*/

void prepareTestCase6(procType proc_type, BPatch_thread *thread, forkWhen when)
{
   const int TN = 6;

   if(proc_type == Parent_p  &&  when == PostFork) {
      BPatch_image *parImage = thread->getImage();

      BPatch_variableExpr *var7_6p = 
	 parImage->findVariable("globalVariable7_6");
      if(doError(TN, (var7_6p==NULL),
		 "  Unable to locate variable globalVariable7_6\n")) return;

      BPatch_arithExpr a_expr7_6p(BPatch_plus, *var7_6p, BPatch_constExpr(5));
      BPatch_arithExpr b_expr7_6p(BPatch_assign, *var7_6p, a_expr7_6p);
      thread->oneTimeCode(b_expr7_6p);
   } else if(proc_type == Child_p  &&  when == PostFork) {
      BPatch_image *childImage = thread->getImage();

      BPatch_variableExpr *var7_6c = 
	 childImage->findVariable("globalVariable7_6");
      if(doError(TN, (var7_6c==NULL),
		 "  Unable to locate variable globalVariable7_6\n")) return;

      BPatch_arithExpr a_expr7_6c(BPatch_plus, *var7_6c, BPatch_constExpr(9));
      BPatch_arithExpr b_expr7_6c(BPatch_assign, *var7_6c, a_expr7_6c);
      thread->oneTimeCode(b_expr7_6c);
   }
}

void checkTestCase6(procType proc_type, BPatch_thread *thread) {
   const int TN = 6;
   if(proc_type == Parent_p) {
      if(! verifyProcMemory(thread, "globalVariable7_6", 26, proc_type)) {
	 passedTest[TN] = false;
      }
   } else if(proc_type == Child_p) {
      if(! verifyProcMemory(thread, "globalVariable7_6", 30, proc_type)) {
	 passedTest[TN] = false;
      }
   }
}

/* Verify that the memory created with malloc is per process and isn't
   being shared between a parent and child process.

   parent/child: malloc a variable
   parent/child: oneTimeCode(snippetA)  (malloced variable = 10)
   --- fork ---
   parent: insert snippet B  (malloced_var += 3);
   child:  insert snippet B' (malloced_var += 7);
   --- run  ---
   parent: verify malloced_var = 13
   child:  verify malloced_var = 17
*/

BPatch_variableExpr *var7_7p;
BPatch_variableExpr *var7_7c;

void prepareTestCase7(procType proc_type, BPatch_thread *thread, forkWhen when)
{
   const int TN = 7;

   if(proc_type == Parent_p  &&  when == PreFork) {
      BPatch_image *parImage = thread->getImage();
      var7_7p = thread->malloc(*(parImage->findType("int")));
      if(doError(TN, (var7_7p==NULL),
		 "  Unable to malloc variable in parent\n")) return;

      BPatch_arithExpr a_expr7_7p(BPatch_assign, *var7_7p,
				  BPatch_constExpr(10));
      thread->oneTimeCode(a_expr7_7p);
   } else if(proc_type == Parent_p  &&  when == PostFork) {
      BPatch_image *parImage = thread->getImage();

      BPatch_Vector<BPatch_function *> found_funcs;
      const char *inFunction = "func7_7";
      if ((NULL == parImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
	fprintf(stderr, "    Unable to find function %s\n",
		inFunction);
	exit(1);
      }
      
      if (1 < found_funcs.size()) {
	fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
		__FILE__, __LINE__, found_funcs.size(), inFunction);
      }
      
      BPatch_Vector<BPatch_point *> *points7_7p = found_funcs[0]->findPoint(BPatch_entry);

      if(doError(TN, !points7_7p || ((*points7_7p).size() == 0),
		 "  Unable to find entry point to \"func7_7\".\n")) return;
      BPatch_point *point7_7p = (*points7_7p)[0];

      BPatch_arithExpr a_expr7_7p(BPatch_plus, *var7_7p, BPatch_constExpr(3));
      BPatch_arithExpr b_expr7_7p(BPatch_assign, *var7_7p, a_expr7_7p);
      thread->insertSnippet(b_expr7_7p, *point7_7p, BPatch_callBefore);
   } else if(proc_type == Child_p  &&  when == PostFork) {
      var7_7c = thread->getInheritedVariable(*var7_7p);

      BPatch_image *childImage = thread->getImage();

      BPatch_Vector<BPatch_function *> found_funcs;
      const char *inFunction = "func7_7";
      if ((NULL == childImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
	fprintf(stderr, "    Unable to find function %s\n",
		inFunction);
	exit(1);
      }
      
      if (1 < found_funcs.size()) {
	fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
		__FILE__, __LINE__, found_funcs.size(), inFunction);
      }
      
      BPatch_Vector<BPatch_point *> *points7_7c = found_funcs[0]->findPoint(BPatch_entry);

      if(doError(TN, !points7_7c || ((*points7_7c).size() == 0),
		 "  Unable to find entry point to \"func7_7\".\n")) return;
      BPatch_point *point7_7c = (*points7_7c)[0];

      BPatch_arithExpr a_expr7_7c(BPatch_plus, *var7_7c, BPatch_constExpr(7));
      BPatch_arithExpr b_expr7_7c(BPatch_assign, *var7_7c, a_expr7_7c);

      thread->insertSnippet(b_expr7_7c, *point7_7c, BPatch_callBefore);
   }
}

void checkTestCase7(procType proc_type, BPatch_thread */*thread*/) {
   const int TN = 7;
   char varname[50];
   sprintf(varname,"test%d malloced var",TN);
   if(proc_type == Parent_p) {
      if(! verifyProcMemory(varname, var7_7p, 13, proc_type)) {
	 passedTest[TN] = false;
      }
   } else if(proc_type == Child_p) {
      if(! verifyProcMemory(varname, var7_7c, 17, proc_type)) {
	 passedTest[TN] = false;
      }
   }
}

/* Verify that if a variable in the child process is freed with 
   BPatch_thread::free, the corresponding variable in the parent process
   isn't also deleted.

   parent/child: malloc a variable
   parent/child: oneTimeCode(snippetA)  (malloced variable = 10)
   --- fork ---
   parent: insert snippet B  (malloced_var += 3);
   child:  free(getInheritedVariable(malloced_var))
   --- run  ---
   parent: verify malloced_var = 13
   (no way to verify the child variable has indeed been freed)
*/

BPatch_variableExpr *var7_8p;

void prepareTestCase8(procType proc_type, BPatch_thread *thread, forkWhen when)
{
   const int TN = 8;

   if(proc_type == Parent_p  &&  when == PreFork) {
      BPatch_image *parImage = thread->getImage();
      var7_8p = thread->malloc(*(parImage->findType("int")));
      if(doError(TN, (var7_8p==NULL),
		 "  Unable to malloc variable in parent\n")) return;

      BPatch_arithExpr a_expr7_8p(BPatch_assign, *var7_8p,
				  BPatch_constExpr(10));
      thread->oneTimeCode(a_expr7_8p);
   } else if(proc_type == Parent_p  &&  when == PostFork) {
      BPatch_image *parImage = thread->getImage();

      BPatch_Vector<BPatch_function *> found_funcs;
      const char *inFunction = "func7_8";
      if ((NULL == parImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
	fprintf(stderr, "    Unable to find function %s\n",
		inFunction);
	exit(1);
      }
      
      if (1 < found_funcs.size()) {
	fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
		__FILE__, __LINE__, found_funcs.size(), inFunction);
      }
      
      BPatch_Vector<BPatch_point *> *points7_8p = found_funcs[0]->findPoint(BPatch_entry);

      if(doError(TN, !points7_8p || ((*points7_8p).size() == 0),
		 "  Unable to find entry point to \"func7_8\".\n")) return;
      BPatch_point *point7_8p = (*points7_8p)[0];

      BPatch_arithExpr a_expr7_8p(BPatch_plus, *var7_8p, BPatch_constExpr(3));
      BPatch_arithExpr b_expr7_8p(BPatch_assign, *var7_8p, a_expr7_8p);

      thread->insertSnippet(b_expr7_8p, *point7_8p, BPatch_callBefore);
   } else if(proc_type == Child_p  &&  when == PostFork) {
      BPatch_variableExpr *var7_8c = thread->getInheritedVariable(*var7_8p);
      thread->free(*var7_8c);
   }
}

void checkTestCase8(procType proc_type, BPatch_thread */*thread*/) {
   const int TN = 8;
   char varname[50];
   sprintf(varname,"test%d malloced var",TN);
   if(proc_type == Parent_p) {
      if(! verifyProcMemory(varname, var7_8p, 13, proc_type)) {
	 passedTest[TN] = false;
      }
   }
}

/* Verify that if a variable in the parent process is freed with 
   BPatch_thread::free, the corresponding variable in the child process
   isn't also deleted.

   parent/child: malloc a variable
   parent/child: oneTimeCode(snippetA)  (malloced variable = 10)
   --- fork ---
   child:  insert snippet B  (malloced_var += 5);
   parent: free(getInheritedVariable(malloced_var))

   --- run  ---
   parent: verify malloced_var = 15
   (no way to verify the child variable has indeed been freed)
*/

BPatch_variableExpr *var7_9p;
BPatch_variableExpr *var7_9c;
BPatch_thread *parentThread = NULL;

void prepareTestCase9(procType proc_type, BPatch_thread *thread, forkWhen when)
{
   const int TN = 9;

   if(proc_type == Parent_p  &&  when == PreFork) {
      BPatch_image *parImage = thread->getImage();
      var7_9p = thread->malloc(*(parImage->findType("int")));
      if(doError(TN, (var7_9p==NULL),
		 "  Unable to malloc variable in parent\n")) return;

      BPatch_arithExpr a_expr7_9p(BPatch_assign, *var7_9p,
				  BPatch_constExpr(10));
      thread->oneTimeCode(a_expr7_9p);
   } else if(proc_type == Parent_p  &&  when == PostFork) {
      parentThread = thread;
      // can't delete var7_9p here, since then the getInheritedVariable
      // would be operating on a freed variable
   } else if(proc_type == Child_p  &&  when == PostFork) {
      var7_9c = thread->getInheritedVariable(*var7_9p);
      parentThread->free(*var7_9p);

      BPatch_image *childImage = thread->getImage();

      BPatch_Vector<BPatch_function *> found_funcs;
      const char *inFunction = "func7_9";
      if ((NULL == childImage->findFunction(inFunction, found_funcs, 1)) || !found_funcs.size()) {
	fprintf(stderr, "    Unable to find function %s\n",
		inFunction);
	exit(1);
      }
      
      if (1 < found_funcs.size()) {
	fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
		__FILE__, __LINE__, found_funcs.size(), inFunction);
      }
      
      BPatch_Vector<BPatch_point *> *points7_9c = found_funcs[0]->findPoint(BPatch_entry);

      if(doError(TN, !points7_9c || ((*points7_9c).size() == 0),
		 "  Unable to find entry point to \"func7_9\".\n")) return;
      BPatch_point *point7_9c = (*points7_9c)[0];

      BPatch_arithExpr a_expr7_9c(BPatch_plus, *var7_9c, BPatch_constExpr(5));
      BPatch_arithExpr b_expr7_9c(BPatch_assign, *var7_9c, a_expr7_9c);

      thread->insertSnippet(b_expr7_9c, *point7_9c, BPatch_callBefore);
   }
}

void checkTestCase9(procType proc_type, BPatch_thread */*thread*/) {
   const int TN = 9;
   char varname[50];
   sprintf(varname,"test%d malloced var",TN);

   if(proc_type == Child_p) {
      if(! verifyProcMemory(varname, var7_9c, 15, proc_type)) {
	 passedTest[TN] = false;
      }
   }
}


void prepareTests(procType proc_type, BPatch_thread *thread, forkWhen when)
{
  //cerr << "prepareTests, proc_type: " << proc_type << "\n";
  for(unsigned testNum=1; testNum<=MAX_TEST; testNum++) {
    //cerr << "testNum: " << testNum << "\n";
      if(! runTest[testNum] || passedTest[testNum]==false) continue;
      switch(testNum) {
	case 1:  prepareTestCase1(proc_type, thread, when);  break;
	case 2:  prepareTestCase2(proc_type, thread, when);  break;
	case 3:  prepareTestCase3(proc_type, thread, when);  break;
	case 4:  prepareTestCase4(proc_type, thread, when);  break;
	case 5:  prepareTestCase5(proc_type, thread, when);  break;
	case 6:  prepareTestCase6(proc_type, thread, when);  break;
	case 7:  prepareTestCase7(proc_type, thread, when);  break;
	case 8:  prepareTestCase8(proc_type, thread, when);  break;
	case 9:  prepareTestCase9(proc_type, thread, when);  break;
	default: 
	   fprintf(stderr, "prepareTest: Invalid test num type\n");
	   exit(0);
	   break;
      }
   }
}

void checkTests(procType proc_type, BPatch_thread *thread)
{
  //cerr << "checkTests, proc_type: " << proc_type << "\n";
   for(unsigned testNum=1; testNum<=MAX_TEST; testNum++) {
     //cerr << "testNum: " << testNum << "\n";
      if(! runTest[testNum] || passedTest[testNum]==false) continue;
      switch(testNum) {
	case 1:  checkTestCase1(proc_type, thread);  break;
	case 2:  checkTestCase2(proc_type, thread);  break;
	case 3:  checkTestCase3(proc_type, thread);  break;
	case 4:  checkTestCase4(proc_type, thread);  break;
	case 5:  checkTestCase5(proc_type, thread);  break;
	case 6:  checkTestCase6(proc_type, thread);  break;
	case 7:  checkTestCase7(proc_type, thread);  break;
	case 8:  checkTestCase8(proc_type, thread);  break;
	case 9:  checkTestCase9(proc_type, thread);  break;
	default: 
	   fprintf(stderr, "checkTest: Invalid test num type\n");
	   exit(0);
	   break;
      }
   }
}

void errorFunc(BPatchErrorLevel level, int num, const char **params)
{
    if (num == 0) {
        // conditional reporting of warnings and informational messages
        if (errorPrint) {
            if (level == BPatchInfo)
              { if (errorPrint > 1) printf("%s\n", params[0]); }
            else
                printf("%s", params[0]);
        }
    } else {
        // reporting of actual errors
        char line[256];
        const char *msg = bpatch->getEnglishErrorString(num);
        bpatch->formatErrorString(line, sizeof(line), msg, params);
        
        if (num != expectError) {
            printf("Error #%d (level %d): %s\n", num, level, line);
        
            // We consider some errors fatal.
            if (num == 101) {
               exit(-1);
            }
        }
    }
}

void showFinalResults() {
   unsigned int testsFailed = 0;
   for (unsigned int i=1; i <= MAX_TEST; i++) {
      if (runTest[i]==false) {
	 printf("Test #%d () => skipped\n", i);	 
      } else if(passedTest[i]==false) {
	 testsFailed++;
	 printf("Test #%d () => FAILED\n", i);
      } else {
	 printf("Test #%d () => passed\n", i); 
      }
   }
   
   if (!testsFailed) {
      if (runAllTests) {
	 printf("All tests passed\n");
      } else {
	 printf("All requested tests passed\n");
      }
   }
}

/* Here's the sequence:
   MUTATOR                      ParentProc        ChildProc
   insert initial 
       instrumentation
   parent->continue

   registerPostFork
   parent->continue                    
                                fork()
   postFork-callback()
   insert instrumentation
   (parent & child)->continue   
                                instr is run      instr is run
                                kill(STOP)        kill(STOP)
   if( isStopped())
      checkInstrResults()
   terminate parent, child
                                exit              exit
*/

void initialPreparation(BPatch_thread *parent)
{
   //cerr << "in initialPreparation\n";
   assert(parent->isStopped());

   //cerr << "ok, inserting instr\n";
   prepareTests(Parent_p, parent, PreFork);
}

void postForkFunc(BPatch_thread *parent, BPatch_thread *child)
{
   //fprintf(stderr, "in postForkFunc\n");
   prepareTests(Parent_p, parent, PostFork);
   prepareTests(Child_p,  child,  PostFork);
   
   /* let the parent and child mutatee processes execute the instrumentation */
   //cerr << "parent->cont\n";
   parent->continueExecution();
   //cerr << "child->cont\n";
   child->continueExecution();
   
   waitForDoneMsg();
   checkTests(Child_p, child);
   checkTests(Parent_p, parent);

   child->terminateExecution();
   parent->terminateExecution();

   showFinalResults();
   closeMessaging();
   exit(0);
}


void mutatorMAIN(char *pathname)
{
    // Create an instance of the BPatch library
    bpatch = new BPatch;

    // Force functions to be relocated
    if (forceRelocation) {
      bpatch->setForcedRelocation_NP(true);
    }

    // Register a callback function that prints any error messages
    bpatch->registerErrorCallback(errorFunc);
    //bpatch->registerPreForkCallback(preForkFunc);    
    bpatch->registerPostForkCallback(postForkFunc);
    for(unsigned i=1; i<=MAX_TEST; i++)  passedTest[i] = true;

    int n = 0;
    char *child_argv[MAX_TEST+5];
	
    dprintf("in mutatorTest1\n");

    inTest = 1;
    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");

    child_argv[n] = NULL;

    // Start the mutatee
    printf("Starting \"%s\"\n", pathname);
    setupMessaging();

    BPatch_thread *appThread = bpatch->createProcess(pathname, child_argv, 
						     NULL);
    if(appThread == NULL) {
	fprintf(stderr, "Unable to run test program.\n");
	exit(1);
    }
    initialPreparation(appThread);
    /* ok, do the fork */;
    appThread->continueExecution();

    /* the rest of the execution occurs in postForkFunc() */
    while(1)  bpatch->waitForStatusChange();
}

//
// main - decide our role and call the correct "main"
//
int
main(unsigned int argc, char *argv[])
{
    unsigned int i;
    bool N32ABI=false;
    char mutateeName[128];
    char libRTname[256];

    strcpy(mutateeName,mutateeNameRoot);
    libRTname[0]='\0';

    if (!getenv("DYNINSTAPI_RT_LIB")) {
	 fprintf(stderr,"Environment variable DYNINSTAPI_RT_LIB undefined:\n"
#if defined(i386_unknown_nt4_0)
		 "    using standard search strategy for libdyninstAPI_RT.dll\n");
#else
	         "    set it to the full pathname of libdyninstAPI_RT\n");   
         exit(-1);
#endif
    } else
         strcpy((char *)libRTname, (char *)getenv("DYNINSTAPI_RT_LIB"));

    // by default run all tests
    for (i=1; i <= MAX_TEST; i++) {
        runTest[i] = true;
        passedTest[i] = false;
    }

    for (i=1; i < argc; i++) {
        if (strncmp(argv[i], "-v+", 3) == 0)    errorPrint++;
        if (strncmp(argv[i], "-v++", 4) == 0)   errorPrint++;
	if (strncmp(argv[i], "-verbose", 2) == 0) {
	    debugPrint = 1;
	} else if (!strcmp(argv[i], "-V")) {
            fprintf (stdout, "%s\n", V_libdyninstAPI);
            if (libRTname[0]) 
                fprintf (stdout, "DYNINSTAPI_RT_LIB=%s\n", libRTname);
            fflush(stdout);
	} else if (!strcmp(argv[i], "-skip")) {
	    unsigned int j;
	    runAllTests = false;
            for (j=i+1; j < argc; j++) {
                unsigned int testId;
                if ((testId = atoi(argv[j]))) {
                    if ((testId > 0) && (testId <= MAX_TEST)) {
                        runTest[testId] = false;
                    } else {
                        printf("invalid test %d requested\n", testId);
                        exit(-1);
                    }
                } else {
                    // end of test list
		    break;
                }
            }
            i=j-1;
	} else if (!strcmp(argv[i], "-run")) {
	    unsigned int j;
	    runAllTests = false;
            for (j=0; j <= MAX_TEST; j++) runTest[j] = false;
            for (j=i+1; j < argc; j++) {
                unsigned int testId;
                if ((testId = atoi(argv[j]))) {
                    if ((testId > 0) && (testId <= MAX_TEST)) {
                        runTest[testId] = true;
                    } else {
                        printf("invalid test %d requested\n", testId);
                        exit(-1);
                    }
                } else {
                    // end of test list
		    break;
                }
            }
            i=j-1;
	} else if (!strcmp(argv[i], "-mutatee")) {
	    i++;
            if (*argv[i]=='_')
                strcat(mutateeName,argv[i]);
            else
                strcpy(mutateeName,argv[i]);
#if defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0) || defined(sparc_sun_solaris2_4) || defined(ia64_unknown_linux2_4)
	} else if (!strcmp(argv[i], "-relocate")) {
            forceRelocation = true;
#endif
#if defined(mips_sgi_irix6_4)
	} else if (!strcmp(argv[i], "-n32")) {
	    N32ABI=true;
#endif
	} else {
	    fprintf(stderr, "Usage: test7 "
		    "[-V] [-verbose] "
#if defined(mips_sgi_irix6_4)
		    "[-n32] "
#endif
                    "[-mutatee <test7.mutatee>] "
		    "[-run <test#> <test#> ...] "
		    "[-skip <test#> <test#> ...]\n");
            fprintf(stderr, "%d subtests\n", MAX_TEST);
	    exit(-1);
	}
    }

    if (!runAllTests) {
        printf("Running Tests: ");
	for (unsigned int j=1; j <= MAX_TEST; j++) {
	    if (runTest[j]) printf("%d ", j);
	}
	printf("\n");
    }

    // patch up the default compiler in mutatee name (if necessary)
    if (!strstr(mutateeName, "_"))
#if defined(i386_unknown_nt4_0)
        strcat(mutateeName,"_VC");
#else
        strcat(mutateeName,"_gcc");
#endif
    if (N32ABI || strstr(mutateeName,"_n32")) {
        // patch up file names based on alternate ABI (as necessary)
        if (!strstr(mutateeName, "_n32")) strcat(mutateeName,"_n32");
    }
    // patch up the platform-specific filename extensions
#if defined(i386_unknown_nt4_0)
    if (!strstr(mutateeName, ".exe")) strcat(mutateeName,".exe");
#endif

    mutatorMAIN(mutateeName);

    return 0;
}
