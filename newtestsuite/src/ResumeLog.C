/*
 * Copyright (c) 1996-2008 Barton P. Miller
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

#include "test_results.h"
#include "test_info_new.h"
#include "test_lib.h"
#include "ResumeLog.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static bool enableLog = false;
static char *resumelog_name = "resumelog";

#define RESULT_REPORTED -1
#define RESUME_POINT -2

void enableResumeLog()
{
   enableLog = true;
}

bool isLogging()
{
   return enableLog;
}

void setLoggingFilename(char *f)
{
   resumelog_name = f;
}

static void log_line(int groupnum, int testnum, int runstate, bool append)
{
   if (!enableLog)
      return;

   FILE *f = fopen(resumelog_name, append ? "a" : "w");
   if (!f) {
      getOutput()->log(STDERR, "Failed to update the resume log");
      return;
   }
   fprintf(f, "%d,%d,%d\n", groupnum, testnum, runstate);
   fclose(f);
}

void log_teststart(int groupnum, int testnum, test_runstate_t runstate)
{
   log_line(groupnum, testnum, (int) runstate, true);
}

void log_testresult(test_results_t result)
{
   if (!enableLog)
      return;

   FILE *f = fopen(resumelog_name, "a");
   if (!f) {
      getOutput()->log(STDERR, "Failed to update the resume log");
      return;
   }
   fprintf(f, "%d\n", result);
   fclose(f);
}

void log_testreported(int groupnum, int testnum)
{
   log_line(groupnum, testnum, RESULT_REPORTED, true);
}

void log_resumepoint(int groupnum, int testnum)
{
   log_line(groupnum, testnum, RESUME_POINT, false);
}

void log_clear()
{
   if (!enableLog)
      return;
   FILE *f = fopen(resumelog_name, "w");
   if (f)
      fclose(f);
}

void parse_resumelog(std::vector<RunGroup *> &groups)
{
   if (!enableLog)
      return;

   FILE *f = fopen(resumelog_name, "r");
   if (!f) {
      return;
   }

   int groupnum, testnum, runstate_int;
   test_runstate_t runstate;
   test_results_t result;
   
   for (;;)
   {
      int res = fscanf(f, "%d,%d,%d\n", &groupnum, &testnum, &runstate_int);
      if (res != 3)
         break;

      assert(groupnum >= 0 && groupnum < groups.size());
      assert(testnum >= 0 && testnum < groups[groupnum]->tests.size());

      if (runstate_int == RESULT_REPORTED)
      {
         groups[groupnum]->tests[testnum]->disabled = true;
         continue;
      }
      if (runstate_int == RESUME_POINT)
      {
         for (unsigned i=0; i<groupnum; i++)
         {
            for (unsigned j=0; j<groups[i]->tests.size(); j++) {
               groups[i]->tests[j]->disabled = true;
            }
            groups[i]->disabled = true;
         }
         for (unsigned j=0; j<testnum; j++) {
            groups[groupnum]->tests[j]->disabled = true;
         }
         continue;
      }

      assert(runstate_int >= 0 && runstate_int < NUM_RUNSTATES);
      runstate = (test_runstate_t) runstate_int;

      res = fscanf(f, "%d\n", &result);
      if (res != 1) {
         result = CRASHED;
      }
      switch (runstate) {
         case test_setup_rs:
         case test_init_rs:
         case test_execute_rs:
         case test_teardown_rs:
            groups[groupnum]->tests[testnum]->results[runstate] = result;
            break;
         case group_setup_rs:
         case group_teardown_rs:
            for (unsigned i=0; i<groups[groupnum]->tests.size(); i++)
            {
               groups[groupnum]->tests[i]->results[runstate] = result;
            }
            break;
         case program_setup_rs:
         case program_teardown_rs:
            for (unsigned i=0; i<groups.size(); i++)
            {
               if (groups[i]->mod != groups[groupnum]->mod)
                  continue;
               for (unsigned j=0; j<groups[i]->tests.size(); j++)
               {
                  groups[i]->tests[j]->results[runstate] = result;
               }
            }
            break;
      }
      if (res != 1)
         break;
   }
}

char *mutatee_resumelog_name = "mutatee_resumelog";

void parse_mutateelog(RunGroup *group)
{
   FILE *f = fopen(mutatee_resumelog_name, "r");
   assert(f);
   char testname[256];
   for (;;)
   {
      test_results_t result = UNKNOWN;
      int res = fscanf(f, "%256s\n", testname);
      if (res != 1) {
         break;
      }
      
      int passed;
      res = fscanf(f, "%d\n", &passed);
      if (!res)
         result = CRASHED;
      else if (passed == 1)
         result = PASSED;
      else if (passed == 0)
         result = FAILED;
      else {
         getOutput()->log(STDERR, "Error parsing mutatee log\n");
         assert(0);
      }
      
      bool found = false;
      for (unsigned i=0; i<group->tests.size(); i++)
      {
         if (strcmp(group->tests[i]->name, testname) == 0)
         {
            group->tests[i]->results[group_teardown_rs] = result;
            found = true;
         }
      }
      assert(found);

      if (result == CRASHED)
         break;
   }
   fclose(f);
}

void clear_mutateelog()
{
   FILE *f = fopen(mutatee_resumelog_name, "w");
   if (!f) {
      getOutput()->log(STDERR, "Unable to reset mutatee log\n");
      exit(0);
   }
   fclose(f);
}
