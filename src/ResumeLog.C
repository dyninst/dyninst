/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#include <assert.h>
#include "test_results.h"
#include "test_info_new.h"
//#include "test_lib.h"
#include "ResumeLog.h"
#include <assert.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static bool enableLog = false;

#define RESULT_REPORTED -1
#define RESUME_POINT -2
#define RESULT_CRASHED -3

struct resumeLogEntry {
   resumeLogEntry(int gn, int tn, int rs, int res=0, bool use_res=false) :
      groupnum(gn),
      testnum(tn),
      runstate(rs),
      result(res),
      use_result(use_res)
   {}
   int groupnum;
   int testnum;
   int runstate;
   int result;
   bool use_result;
};

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
   set_resumelog_name(f);
}

void rebuild_resumelog(const std::vector<resumeLogEntry> &entries)
{
   if (!enableLog)
      return;

   FILE *f = fopen(get_resumelog_name(), "a");
   
   for (unsigned i=0; i<entries.size(); i++)
   {
      fprintf(f, "%d,%d,%d\n", entries[i].groupnum, entries[i].testnum, 
              entries[i].runstate);
      if (entries[i].use_result)
         fprintf(f, "%d\n", entries[i].result);
   }

   fclose(f);
}

static void log_line(int groupnum, int testnum, int runstate, bool append)
{
   if (!enableLog)
      return;

   FILE *f = fopen(get_resumelog_name(), append ? "a" : "w");
   if (!f) {
      fprintf(stderr, "Failed to update the resume log");
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

   FILE *f = fopen(get_resumelog_name(), "a");
   if (!f) {
      fprintf(stderr, "Failed to update the resume log");
      return;
   }
   fprintf(f, "%d\n", result);
   fclose(f);
}

void log_testreported(int groupnum, int testnum)
{
   log_line(groupnum, testnum, RESULT_REPORTED, true);
}

static std::vector<resumeLogEntry> recreate_entries;

void log_resumepoint(int groupnum, int testnum)
{
   log_line(groupnum, testnum, RESUME_POINT, false);
   rebuild_resumelog(recreate_entries);
}

void log_clear()
{
   if (!enableLog)
      return;
   FILE *f = fopen(get_resumelog_name(), "w");
   if (f)
      fclose(f);
}

void parse_resumelog(std::vector<RunGroup *> &groups)
{
   if (!enableLog)
      return;

   FILE *f = fopen(get_resumelog_name(), "r");
   if (!f) {
      return;
   }

   unsigned int groupnum, testnum;
   int runstate_int;
   test_runstate_t runstate;
   test_results_t result;
   
   for (;;)
   {
      int res = fscanf(f, "%d,%d,%d\n", &groupnum, &testnum, &runstate_int);
      if (res != 3)
         break;
      
      assert(groupnum >= 0 && groupnum < groups.size());
      assert(groups[groupnum]);
      assert(testnum >= 0);
      assert(testnum < groups[groupnum]->tests.size());
      if (runstate_int == RESULT_REPORTED)
      {
         groups[groupnum]->tests[testnum]->result_reported = true;
         recreate_entries.push_back(resumeLogEntry(groupnum, testnum, RESULT_REPORTED));
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
      recreate_entries.push_back(resumeLogEntry(groupnum, testnum, 
                                                runstate_int, result, true));

      if (res != 1)
         break;
   }

   rebuild_resumelog(recreate_entries);
}

void parse_mutateelog(RunGroup *group, char *logname)
{
   FILE *f = fopen(logname, "r");
   if (!f) {
      std::string alt_logname = std::string("../") + logname;
      f = fopen(alt_logname.c_str(), "r");
   }
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
      if (res == EOF)
         result = CRASHED;
      else if (passed == 1)
         result = PASSED;
      else if (passed == 0)
         result = FAILED;
      else {
         fprintf(stderr, "Error parsing mutatee log\n");
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

void clear_mutateelog(char *logname)
{
   FILE *f = fopen(logname, "w");
   if (!f) {
      std::string alt_logname = std::string("../") + logname;
      f = fopen(alt_logname.c_str(), "w");
   }
   if (!f) {
      fprintf(stderr, "Unable to reset mutatee log\n");
      exit(0);
   }
   fclose(f);
}

static char *resumelog_name = const_cast<char*>("resumelog");
char *get_resumelog_name() {
	return resumelog_name;
}

void set_resumelog_name(char *s) {
	resumelog_name = s;
}
