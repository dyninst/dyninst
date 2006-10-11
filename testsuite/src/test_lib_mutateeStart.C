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

// $Id: test_lib_mutateeStart.C,v 1.4 2006/10/11 21:54:32 cooksey Exp $
// Functions Dealing with mutatee Startup

#include "test_lib.h"
#include <stdlib.h>


BPatch_thread *startMutateeTestGeneric(BPatch *bpatch, char *pathname, const char **child_argv, bool useAttach, char *logfilename)
{
   BPatch_thread *appThread;
    if (useAttach) {
        FILE *outlog = stdout;
	FILE *errlog = stderr;
        if ((logfilename != NULL) && (strcmp(logfilename, "-") != 0)) {
	   outlog = fopen(logfilename, "a");
	   if (outlog == NULL) {
	     outlog = stdout;
	   } else {
	     errlog = outlog;
	   }
        }
	int pid = startNewProcessForAttach(pathname, child_argv,
					   outlog, errlog);
	if (pid < 0) {
	    fprintf(stderr, "*ERROR*: unable to start tests due to error creating mutatee process\n");
            return NULL;
        } else {
            dprintf("New mutatee process pid %d started; attaching...\n", pid);
	}
        P_sleep(1); // let the mutatee catch its breath for a moment
        dprintf("Attaching to process: %s, %d\n", pathname, pid);
	appThread = bpatch->attachProcess(pathname, pid);
        dprintf("Attached to process\n");
	// Is if safe to close the files now?
	if ((outlog != NULL) && (outlog != stdout)) {
	  fclose(outlog);
	}
    } else {
       /*
        printf("before createProcess, bpatch %x, %s\n", bpatch, pathname);
        for ( int i = 0; child_argv[i] != NULL; i++)
        {
           printf("%s ", child_argv[i]);
        }
        printf("\n");
        */
      if ((logfilename != NULL) && (strcmp(logfilename, "-") != 0)) {
	FILE *logfile = fopen(logfilename, "a");
	int logfile_fd = 1;
	if (logfile != NULL) {
	  logfile_fd = fileno(logfile);
	}
	appThread = bpatch->createProcess(pathname, child_argv, NULL,
					  0, logfile_fd, logfile_fd);
      } else {
	appThread = bpatch->createProcess(pathname, child_argv, NULL);
      }

        //printf("after createProcess: %x\n", appThread);
    }

    return appThread;
}

// Starts Mutatee and returns the proper appImage
BPatch_thread *startMutateeTest(BPatch *bpatch, char *pathname, int subtestno, bool useAttach, ProcessList &procList, char *logfilename)
{
   BPatch_thread *result = startMutateeTest(bpatch, pathname, subtestno, useAttach, logfilename);
   if ( result != NULL )
   {
      procList.insertThread(result);
   }

   return result;
}

// TODO Need to add log file support to this function
BPatch_thread *startMutateeTestAll(BPatch *bpatch, char *pathname, bool useAttach, ProcessList &procList, char *logfilename)
{

   BPatch_thread *appThread;
   const char *child_argv[5];

   // Start the mutatee
   dprintf("Starting \"%s\"\n", pathname);

   int n = 0;
   child_argv[n++] = pathname;
   child_argv[n++] = const_cast<char*>("-runall");
   child_argv[n] = NULL;

   appThread = startMutateeTestGeneric(bpatch, pathname, child_argv, useAttach, logfilename);
   
    if ( appThread != NULL )
    {
       procList.insertThread(appThread);
    }
   
    return appThread;

}

BPatch_thread *startMutateeTest(BPatch *bpatch, char *pathname, int subtestno, bool useAttach, char *logfilename)
{

   BPatch_thread *appThread;
   const char *child_argv[7];

   // Start the mutatee
   dprintf("Starting \"%s\"\n", pathname);

   int n = 0;
   child_argv[n++] = pathname;
   //child_argv[n++] = const_cast<char*>("-verbose");
   child_argv[n++] = const_cast<char*>("-run");
   char str[5];
   sprintf(str, "%d", subtestno);
   child_argv[n++] = str;
   if (logfilename != NULL) {
     child_argv[n++] = const_cast<char *>("-log");
     child_argv[n++] = logfilename;
   }
   child_argv[n++] = NULL;
   
   return startMutateeTestGeneric(bpatch, pathname, child_argv, useAttach,
				  logfilename);

}

// This function added to support the fast and loose test mode
BPatch_thread *startMutateeTestSet(BPatch *bpatch, char *pathname,
				   test_data_t tests[],
				   int first_test, int last_test,
				   bool useAttach, ProcessList &procList,
				   char *logfilename, bool runAllTests,
				   std::vector<char *>test_list) {
   BPatch_thread *appThread;
   int start_testset;
   // Need child_argv to have enough room for all the tests in the set..
   int test_count = 0;
   if (runAllTests) {
     test_count = last_test - first_test;
   } else {
     for (int i = first_test; i < last_test; i++) {
       if (inTestList(tests[i], test_list)) {
	 test_count += 1;
       }
     }
   }
   const char **child_argv = new const char *[6 + test_count];

   // Start the mutatee
   dprintf("Starting \"%s\"\n", pathname);

   int n = 0;
   child_argv[n++] = pathname;
   child_argv[n++] = "-fast";
   //child_argv[n++] = const_cast<char*>("-verbose");
   child_argv[n++] = const_cast<char*>("-run");
   // Need to add a string for each test to the array..
   start_testset = n;
   for (int i = first_test; i < last_test; i++) {
     // Only want to run the tests that we're supposed to run
     if (runAllTests || inTestList(tests[i], test_list)) {
       char *str = new char[5];
       sprintf(str, "%d", tests[i].subtest);
       child_argv[n++] = str;
     }
   }
   if (logfilename != NULL) {
     child_argv[n++] = const_cast<char *>("-log");
     child_argv[n++] = logfilename;
   }
   child_argv[n++] = NULL;
   
   appThread = startMutateeTestGeneric(bpatch, pathname, child_argv, useAttach,
				       logfilename);
   for (int i = start_testset; i < (start_testset + test_count); i++) {
     delete [] child_argv[i];
   }
   delete [] child_argv;
   if (appThread != NULL) {
     procList.insertThread(appThread);
   }
   return appThread;
}

BPatch_thread *startMutateeEnabledTests(BPatch *bpatch, char *pathname, bool useAttach, test_data_t tests[], unsigned int num_tests, int oldtest, char *logfilename)
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
   appThread = startMutateeTestGeneric(bpatch, pathname, child_argv, useAttach, logfilename);

   //free(child_argv);

   return appThread;
}


