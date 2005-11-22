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

// $Id: test_lib_mutateeStart.C,v 1.2 2005/11/22 19:41:23 bpellin Exp $
// Functions Dealing with mutatee Startup

#include "test_lib.h"
#include <stdlib.h>


BPatch_thread *startMutateeTestGeneric(BPatch *bpatch, char *pathname, const char **child_argv, bool useAttach)
{
   BPatch_thread *appThread;
    if (useAttach) {
        printf("Starting process for attach\n");
	int pid = startNewProcessForAttach(pathname, child_argv);
        printf("Started process for attach\n");
	if (pid < 0) {
	    printf("*ERROR*: unable to start tests due to error creating mutatee process\n");
            return NULL;
        } else {
            dprintf("New mutatee process pid %d started; attaching...\n", pid);
	}
        P_sleep(1); // let the mutatee catch its breath for a moment
        printf("Attaching to process: %s, %d\n", pathname, pid);
	appThread = bpatch->attachProcess(pathname, pid);
        printf("Attached to process\n");
    } else {
       /*
        printf("before createProcess, bpatch %x, %s\n", bpatch, pathname);
        for ( int i = 0; child_argv[i] != NULL; i++)
        {
           printf("%s ", child_argv[i]);
        }
        printf("\n");
        */
	appThread = bpatch->createProcess(pathname, child_argv,NULL);


        //printf("after createProcess: %x\n", appThread);
    }

    return appThread;
}

// Starts Mutatee and returns the proper appImage
BPatch_thread *startMutateeTest(BPatch *bpatch, char *pathname, int subtestno, bool useAttach, ProcessList &procList)
{
   BPatch_thread *result = startMutateeTest(bpatch, pathname, subtestno, useAttach);
   if ( result != NULL )
   {
      procList.insertThread(result);
   }

   return result;
}

BPatch_thread *startMutateeTestAll(BPatch *bpatch, char *pathname, bool useAttach, ProcessList &procList)
{

   BPatch_thread *appThread;
   const char *child_argv[5];

   // Start the mutatee
   dprintf("Starting \"%s\"\n", pathname);

   int n = 0;
   child_argv[n++] = pathname;
   child_argv[n++] = const_cast<char*>("-runall");
   child_argv[n] = NULL;

   appThread = startMutateeTestGeneric(bpatch, pathname, child_argv, useAttach);
   
    if ( appThread != NULL )
    {
       procList.insertThread(appThread);
    }
   
    return appThread;

}

BPatch_thread *startMutateeTest(BPatch *bpatch, char *pathname, int subtestno, bool useAttach)
{

   BPatch_thread *appThread;
   const char *child_argv[5];

   // Start the mutatee
   dprintf("Starting \"%s\"\n", pathname);

   int n = 0;
   child_argv[n++] = pathname;
   //child_argv[n++] = const_cast<char*>("-verbose");
   child_argv[n++] = const_cast<char*>("-run");
   char str[5];
   sprintf(str, "%d", subtestno);
   child_argv[n++] = str;
   child_argv[n++] = NULL;

   
   return startMutateeTestGeneric(bpatch, pathname, child_argv, useAttach);

}

BPatch_thread *startMutateeEnabledTests(BPatch *bpatch, char *pathname, bool useAttach, test_data_t tests[], unsigned int num_tests, int oldtest)
{
   int n = 0;
   const char **child_argv;
   child_argv = (const char **)calloc(num_tests,sizeof(const char*));
   child_argv[n++] = pathname;
   child_argv[n++] = const_cast<char*>("-run");
   for ( unsigned int i = 0; i < num_tests; i++ )
   {
      if ( tests[i].enabled && tests[i].oldtest == oldtest ) {
         char str[5];
         sprintf(str, "%d", tests[i].subtest);
         child_argv[n++] = const_cast<char*>(strdup(str));
      }
   }

   child_argv[n] = NULL;

   BPatch_thread *appThread;
   //printf("child_argv built\n");
   appThread = startMutateeTestGeneric(bpatch, pathname, child_argv, useAttach);

   //free(child_argv);

   return appThread;
}


