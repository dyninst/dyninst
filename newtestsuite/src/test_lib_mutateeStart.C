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

// $Id: test_lib_mutateeStart.C,v 1.3 2008/06/18 19:58:32 carl Exp $
// Functions Dealing with mutatee Startup

#include "test_lib.h"
#include <stdlib.h>


BPatch_thread *startMutateeTestGeneric(BPatch *bpatch, char *pathname, const char **child_argv, bool useAttach)
{
   BPatch_thread *appThread;
    if (useAttach) {
	// I should be able to remove the outlog and errlog parameters from
	// startNewProcessForAttach without harming anything.
	int pid = startNewProcessForAttach(pathname, child_argv,
					   NULL, NULL);
	if (pid < 0) {
	    fprintf(stderr, "*ERROR*: unable to start tests due to error creating mutatee process\n");
            return NULL;
        } else {
            dprintf("New mutatee process pid %d started; attaching...\n", pid);
	    registerPID(pid); // Register PID for cleanup
	}
        P_sleep(1); // let the mutatee catch its breath for a moment
        dprintf("Attaching to process: %s, %d\n", pathname, pid);
	appThread = bpatch->attachProcess(pathname, pid);
        dprintf("Attached to process\n");
	dprintf("appThread == %lu\n", (unsigned long) appThread);
    } else {
      appThread = bpatch->createProcess(pathname, child_argv, NULL);
      if (appThread != NULL) {
	int pid = appThread->getProcess()->getPid();
	registerPID(pid); // Register PID for cleanup
      }
    }

    return appThread;
}

// Starts Mutatee and returns the proper appImage
BPatch_thread *startMutateeTest(BPatch *bpatch, RunGroup *group,
				ProcessList &procList, char *logfilename,
				char *humanlogname, bool verboseFormat,
				bool printLabels, int debugPrint,
				char *pidfilename)
{
   BPatch_thread *result = startMutateeTest(bpatch, group, logfilename,
					    humanlogname, verboseFormat,
					    printLabels, debugPrint,
					    pidfilename);
   if ( result != NULL )
   {
      procList.insertThread(result);
   }

   return result;
}

BPatch_thread *startMutateeTestAll(BPatch *bpatch, char *pathname,
				   bool useAttach, ProcessList &procList,
				   char *logfilename, char *humanlogname)
{

   BPatch_thread *appThread;
   const char *child_argv[7];

   // Start the mutatee
   dprintf("Starting \"%s\"\n", pathname);

   int n = 0;
   child_argv[n++] = pathname;
   child_argv[n++] = const_cast<char*>("-runall");
   if (logfilename != NULL) {
     child_argv[n++] = const_cast<char *>("-log");
     child_argv[n++] = logfilename;
   }
   if (humanlogname != NULL) {
     child_argv[n++] = const_cast<char *>("-humanlog");
     child_argv[n++] = humanlogname;
   }
   child_argv[n] = NULL;

   appThread = startMutateeTestGeneric(bpatch, pathname, child_argv, useAttach);
   
    if ( appThread != NULL )
    {
       procList.insertThread(appThread);
    }
   
    return appThread;

}

BPatch_thread *startMutateeTest(BPatch *bpatch, char *mutatee, char *testname,
				bool useAttach, char *logfilename,
				char *humanlogname)
{
  std::vector<std::string> mutateeArgs;
  getOutput()->getMutateeArgs(*&mutateeArgs); // mutateeArgs is an output parameter
  const char **child_argv = new const char *[8 + mutateeArgs.size()];
  if (NULL == child_argv) {
    return NULL;
  }

  // Start the mutatee
  dprintf("Starting \"%s\"\n", mutatee);

  int n = 0;
  child_argv[n++] = mutatee;
  if (logfilename != NULL) {
    child_argv[n++] = const_cast<char *>("-log");
    child_argv[n++] = logfilename;
  }
  if (humanlogname != NULL) {
    child_argv[n++] = const_cast<char *>("-humanlog");
    child_argv[n++] = humanlogname;
  }
  child_argv[n++] = const_cast<char *>("-run");
  child_argv[n++] = testname;
  for (int i = 0; i < mutateeArgs.size(); i++) {
    child_argv[n++] = mutateeArgs[i].c_str();
  }
  child_argv[n] = NULL;

  BPatch_thread *retval = startMutateeTestGeneric(bpatch, mutatee, child_argv,
						  useAttach);
  delete [] child_argv;
  return retval;
}

BPatch_thread *startMutateeTest(BPatch *bpatch, RunGroup *group,
				char *logfilename, char *humanlogname,
				bool verboseFormat, bool printLabels,
				int debugPrint, char *pidfilename)
{
  std::vector<std::string> mutateeArgs;
  getOutput()->getMutateeArgs(*&mutateeArgs); // mutateeArgs is an output parameter
   BPatch_thread *appThread;
   const char **child_argv = new const char *[12 + (4 * group->tests.size())
					      + mutateeArgs.size()];
   if (NULL == child_argv) {
     return NULL;
   }

   // Start the mutatee
   dprintf("Starting \"%s\"\n", group->mutatee);

   int n = 0;
   child_argv[n++] = group->mutatee;
   //child_argv[n++] = const_cast<char*>("-verbose");
   if (logfilename != NULL) {
     child_argv[n++] = const_cast<char *>("-log");
     child_argv[n++] = logfilename;
   }
   if (humanlogname != NULL) {
     child_argv[n++] = const_cast<char *>("-humanlog");
     child_argv[n++] = humanlogname;
   }
   if (false == verboseFormat) {
     child_argv[n++] = const_cast<char *>("-q");
     // TODO I'll also want to pass a parameter specifying a file to write
     // postponed messages to
   }
   if (debugPrint != 0) {
     child_argv[n++] = const_cast<char *>("-verbose");
   }
   if (pidfilename != NULL) {
     child_argv[n++] = const_cast<char *>("-pidfile");
     child_argv[n++] = pidfilename;
   }
   for (int i = 0; i < group->tests.size(); i++) {
     child_argv[n++] = const_cast<char*>("-run");
     child_argv[n++] = group->tests[i]->name;
   }
   if (printLabels) {
     for (int i = 0; i < group->tests.size(); i++) {
       child_argv[n++] = const_cast<char *>("-label");
       child_argv[n++] = group->tests[i]->label;
     }
     child_argv[n++] = const_cast<char *>("-print-labels");
   }
   for (int i = 0; i < mutateeArgs.size(); i++) {
     child_argv[n++] = mutateeArgs[i].c_str();
   }
   child_argv[n] = NULL;
   
   BPatch_thread *retval = startMutateeTestGeneric(bpatch, group->mutatee,
						   child_argv,
						   group->useAttach);
   delete [] child_argv;
   return retval;
}
