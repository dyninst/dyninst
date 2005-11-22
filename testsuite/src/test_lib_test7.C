/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: test_lib_test7.C,v 1.2 2005/11/22 19:40:53 bpellin Exp $
#include "test_lib_test7.h"
#include "test_lib.h"
#include <sys/msg.h>

int setupMessaging(int *msgid) {
  key_t msgkey = 1234;

  if((*msgid = msgget(msgkey, 0666|IPC_CREAT)) == -1) {
    perror("Couldn't create messaging");
    return -1;
  }

  return 0;
}

bool doError(bool *passedTest, bool cond, const char *str) {
   if(cond == true) {
      fprintf(stderr, str);
      *passedTest = false;
      return true;
   }
   return false;
}

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

char *subTestNames[10] = {
    "<no test # 0>",
    "Delete snippet in parent",
    "Delete snippet in child",
    "Delete snippet in both parent and child",
    "Insert snippet in child - wo inherited snippets",
    "Add snippets to parent & child",
    "OneTimeCode in parent & child",
    "Memory allocation in parent & child",
    "Memory deallocate in child",
    "Memory deallocate in parent",
};

void showFinalResults(bool passedTest, int i) {
   if(passedTest==false) {
      printf("Failed test #%d (%s)\n", i, subTestNames[i]);
   } else {
      printf("Passed test #%d (%s)\n", i, subTestNames[i]);
   }
}
