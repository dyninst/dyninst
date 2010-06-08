/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

// $Id: test_driver.C,v 1.7 2008/10/30 19:16:50 legendre Exp $
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <string>
#include <errno.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <algorithm>
#include <sys/stat.h>
#include <time.h>
#include <limits.h>

#if defined(os_windows_test)
#define vsnprintf _vsnprintf
#define snprintf _snprintf
#pragma warning(disable:4786)
#include <direct.h>
#else
#include <fnmatch.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/times.h>
#include <strings.h>
#endif

#include "ParameterDict.h"
#include "test_lib.h"
#include "error.h"
#include "ResumeLog.h"
#include "TestOutputDriver.h"
#include "StdOutputDriver.h"
#include "comptester.h"
#include "help.h"

#define opt_none 1<<0
#define opt_low 1<<1
#define opt_high 1<<2
#define opt_max 1<<3
#define opt_all (opt_none|opt_low|opt_high|opt_max)

int testsRun = 0;

FILE *outlog = NULL;
FILE *errlog = NULL;

char *pdscrdir = NULL;
char *uw_pdscrdir = "/scratch/paradyn/builds/scripts";
char *umd_pdscrdir = "/fs/dyninst/dyninst/current/scripts";

int parseArgs(int argc, char *argv[]);
int setupLogs();

#if !defined(os_windows_test)
int runScript(const char *name, ...)
{
   char test[1024];
   int result;
   va_list ap;
   va_start(ap, name);
   vsnprintf(test, 1024, name, ap);
   va_end(ap);

   char test2[1024];
   if ((outlog != NULL) && (outlog != stdout)) {
      snprintf(test2, 1024, "sh -c \"%s\" >>%s 2>&1", test, logfilename);
   } else {
      snprintf(test2, 1024, "%s", test);
   }

   // Flush before/after script run
   flushOutputLog();
   flushErrorLog();
   result = system(test2);
   flushOutputLog();
   flushErrorLog();

   return result;
}
#else
int runScript(const char *name, ...) {
   getOutput()->log(STDERR, "runScript not implemented on Windows\n");
   assert(0);
   return -1;
}
#endif

#if !defined(os_windows_test)
static void *mem_start;
static struct tms start_time;

#include <unistd.h>

void measureStart()
{
   if (!measureMEMCPU)
      return;

   mem_start = sbrk(0);
   times(&start_time);
}

void measureEnd()
{
   if (!measureMEMCPU)
      return;

   void *mem_end = sbrk(0);
   struct tms end_time;
   times(&end_time);

   signed long mem_diff = ((char *) mem_end) - ((char *) mem_start);
   clock_t utime = end_time.tms_utime - start_time.tms_utime;
   clock_t stime = end_time.tms_stime - start_time.tms_stime;

   FILE *measure_file = NULL;
   bool using_stdout;
   if (strcmp(measureFileName, "-") == 0) {
      using_stdout = true;
      measure_file = stdout;
   }
   else {
      using_stdout = false;
      measure_file = fopen(measureFileName, "a");
   }
   if (!measure_file)
   {
      perror("Unable to open file for CPU/MEM measurement");
      return;
   }

   fprintf(measure_file, "mem=%ld\tutime=%ld\tstime=%ld\n",
           mem_diff, utime, stime);
   if (!using_stdout)
      fclose(measure_file);
}

void setupProcessGroup()
{
   if (!called_from_runTests)
      return;

   setpgrp();
}

#else

void measureStart()
{
}

void measureEnd()
{
}

void setupProcessGroup()
{
}

#endif

void printLogMutateeHeader(const char *mutatee, test_linktype_t linktype)
{
   flushOutputLog();
   flushErrorLog();
   // Mutatee Description
   // Mutatee Info
   if ( (mutatee != NULL) && (strcmp(mutatee, "") != 0) ) {
      getOutput()->log(LOGINFO, "[Tests with %s]\n", mutatee);
#if !defined(os_windows_test)
      if ( pdscrdir ) {
         runScript("ls -lLF %s", mutatee);
         if( linktype == DynamicLink ) 
            runScript("%s/ldd_PD %s", pdscrdir, mutatee);
         else
            getOutput()->log(LOGINFO, "%s: statically linked\n", mutatee);
      }
#endif
   } else {
      getOutput()->log(LOGINFO, "[Tests with none]\n");
   }

   getOutput()->log(LOGINFO, "\n");
   flushOutputLog();
   flushErrorLog();
}

void printLogOptionHeader(TestInfo *tinfo)
{
   if (!printMutateeLogHeader)
      return;
   flushOutputLog();
   flushErrorLog();
   // Full test description
   getOutput()->log(LOGINFO, "test-info: %s\n", tinfo->label);
   flushOutputLog();
   flushErrorLog();
}

void printHumanTestHeader(test_data_t &test, char *mutatee, bool useAttach)
{
   flushOutputLog();
   flushErrorLog();
   // Test Header
   getOutput()->log(LOGINFO, "Running: %s", test.name);
   if ( strcmp(mutatee, "") != 0 )
   {
      getOutput()->log(LOGINFO, " with mutatee: %s", mutatee);
   }
   if ( useAttach )
   {
      getOutput()->log(LOGINFO, " in attach mode");
   }
   getOutput()->log(LOGINFO, ".\n");


   flushOutputLog();
   flushErrorLog();
}

struct formatStrings
{
   formatStrings(const std::string& pass, const std::string& skip, const std::string& fail, const std::string& crash, 
                 const std::string& create, const std::string& attach, const std::string& testNameFormat) :
      passStr(pass), skipStr(skip), failStr(fail), crashStr(crash), createStr(create), attachStr(attach), testNameFormatStr(testNameFormat) 
   {
   }
   formatStrings() 
   {
   }
  
   std::string passStr;
   std::string skipStr;
   std::string failStr;
   std::string crashStr;
   std::string createStr;
   std::string attachStr;
   std::string testNameFormatStr;
};

static formatStrings verboseStrings("PASSED\n", "SKIPPED\n", "FAILED\n", "CRASHED\n", "create\tresult: ", "attach\tresult: ", "%s: mutatee: %s create_mode: ");
static formatStrings compactStrings(".", "S", "F", "C", "", "", "");

struct failureInfo
{
   std::string testName;
   bool createMode;
   bool wasCrash;
};

// Performs a wildcard string match on Unix-like systems, and a standard string
// match on Windows
// Returns true for match found, false for no match
bool nameMatches(const char *wcname, const char *tomatch) {
#if defined(os_windows_test)
   // Sadly, we can't assume the presence of fnmatch on Windows
   return (strcmp(wcname, tomatch) == 0);
#else
   // The other systems we support are unix-based and should provide fnmatch (?)
   return (fnmatch(wcname, tomatch, 0) == 0);
#endif
}

int numUnreportedTests(RunGroup *group)
{
   int num_unreported = 0;

   for (unsigned i=0; i<group->tests.size(); i++)
   {
      if (shouldRunTest(group, group->tests[i]))
      {
         num_unreported++;
      }
   }

   return num_unreported;
}

void executeTest(ComponentTester *tester,
                 RunGroup *group, TestInfo *test, 
                 ParameterDict param)
{
   std::map<std::string, std::string> attrs;
   TestOutputDriver::getAttributesMap(test, group, attrs);

   std::vector<ParamString*> new_params;
   int group_num = group->index;
   int test_num = test->index;

   std::map<std::string, std::string>::iterator i = attrs.begin();
   for (; i != attrs.end(); i++)
   {
      ParamString *pstr = new ParamString((*i).second.c_str());
      param[(*i).first] = pstr;
      new_params.push_back(pstr);
   }

   if (shouldRunTest(group, test))
   {
      log_teststart(group_num, test_num, test_setup_rs);
      test->results[test_setup_rs] = tester->test_setup(test, param);
      log_testresult(test->results[test_setup_rs]);
   }

   if (shouldRunTest(group, test))
   {
      log_teststart(group_num, test_num, test_execute_rs);
      test->results[test_execute_rs] = test->mutator->executeTest();
      log_testresult(test->results[test_execute_rs]);
   }

   if (shouldRunTest(group, test))
   {
      log_teststart(group_num, test_num, test_teardown_rs);
      test->results[test_teardown_rs] = tester->test_teardown(test, param);
      log_testresult(test->results[test_teardown_rs]);
   }

   for (unsigned j=0; j<new_params.size(); j++)
      delete new_params[j];
}

volatile int dont_optimize = 0;
void tests_breakpoint()
{
  dont_optimize++;
  //Breakpoint here to get a binary with test libraries loaded.
}

void executeGroup(ComponentTester *tester, RunGroup *group, 
                  ParameterDict param)
{
   test_results_t result;
   // setupMutatorsForRunGroup creates TestMutator objects and
   // sets a pointer in the TestInfo object to point to the TestMutator for
   // each test.
   int tests_found = setupMutatorsForRunGroup(group);
   if (tests_found <= 0)
      return;
   tests_breakpoint();

   int groupnum = group->index;

   ParamString mutatee_prm(group->mutatee);
   ParamInt startstate_prm((int) group->state);
   ParamInt createmode_prm((int) group->useAttach);
   ParamInt customexecution_prm((int) group->customExecution);
   ParamInt selfstart_prm((int) group->selfStart);
   ParamInt threadmode_prm((int) group->threadmode);
   ParamInt procmode_prm((int) group->procmode);

   param["pathname"] = &mutatee_prm;
   param["startState"] = &startstate_prm;
   param["useAttach"] = &createmode_prm;
   param["customExecution"] = &customexecution_prm;
   param["selfStart"] = &selfstart_prm;
   param["threadMode"] = &threadmode_prm;
   param["processMode"] = &procmode_prm;

   for (unsigned i=0; i<group->tests.size(); i++)
   {
      if (shouldRunTest(group, group->tests[i]))
         testsRun++;
   }

   log_teststart(groupnum, 0, group_setup_rs);
   result = tester->group_setup(group, param);
   log_testresult(result);
   if (result != PASSED && result != SKIPPED && result != UNKNOWN) {
      getOutput()->log(LOGERR, "Group setup failed: %s\n", 
                       tester->getLastErrorMsg().c_str());
   }
   if (result != UNKNOWN) {
      for (unsigned int i = 0; i < group->tests.size(); i++) {
         group->tests[i]->results[group_setup_rs] = result;
      }
   }

   for(unsigned i = 0; i < group->tests.size(); i++)
   {
      // Print mutator log header
	  assert(group->tests[i]);
      printLogOptionHeader(group->tests[i]);

      if (shouldRunTest(group, group->tests[i])) {
         log_teststart(groupnum, i, test_init_rs);
         if(group->tests[i]->mutator)
         {
            test_results_t setup_res = group->tests[i]->mutator->setup(param);
            group->tests[i]->results[test_init_rs] = setup_res;
            if (setup_res != PASSED)
            {
               logerror("%s[%d]:  setup failed (%d) for test %s\n", 
                        FILE__, __LINE__, (int) setup_res, group->tests[i]->name);
            }
         }
         else
         {
            logerror("No mutator object found for test: %s\n", group->tests[i]->name);
            group->tests[i]->results[test_init_rs] = FAILED;
         }
         log_testresult(group->tests[i]->results[test_init_rs]);
      }
   }
      
   for (size_t i = 0; i < group->tests.size(); i++) {
      executeTest(tester, group, group->tests[i], param);
   }

   log_teststart(groupnum, 0, group_teardown_rs);
   result = tester->group_teardown(group, param);
   log_testresult(result);
   if (result != PASSED && result != SKIPPED && result != UNKNOWN) {
      getOutput()->log(LOGERR, "Group teardown failed: %s\n", 
                       tester->getLastErrorMsg().c_str());
   }
   if (result != UNKNOWN) {
      for (unsigned int i = 0; i < group->tests.size(); i++) {
         group->tests[i]->results[group_teardown_rs] = result;
      }
   }

   for (int i = 0; i < group->tests.size(); i++) {
      reportTestResult(group, group->tests[i]);
   }
}

#define is_int(x) (x >= '0' && x <= '9')
bool strint_lt(const char *lv, const char *rv)
{
   int i = 0;
   while (lv[i] != '\0' && rv[i] != '\0')
   {
      if (lv[i] == rv[i]) {
         i++;
         continue;
      }

      bool lint = is_int(lv[i]);
      bool rint = is_int(rv[i]);

      if (lint && !rint)
         return true;
      else if (!lint && rint)
         return false;
      else if (!lint && !rint)
         return (lv[i] < rv[i]);
      else {
         return atoi(lv+i) < atoi(rv+i);
      }
   }
   if (lv[i] == '\0' && rv[i] != '\0')
      return true;
   return false;
}

void initModuleIfNecessary(RunGroup *group, std::vector<RunGroup *> &groups, 
                           ParameterDict &params)
{
   bool initModule = false;
   if (group->disabled)
      return;
   for (unsigned j=0; j<group->tests.size(); j++)
   {
      if (group->mod && !group->mod->setupRun() && !group->tests[j]->disabled)
         initModule = true;
   }
   if (!initModule)
      return;

   log_teststart(group->index, 0, program_setup_rs);
   test_results_t result = group->mod->tester->program_setup(params);
   log_testresult(result);

   group->mod->setSetupRun(true);

   for (unsigned i=0; i<groups.size(); i++) {
      if (groups[i]->disabled || groups[i]->mod != group->mod)
         continue;
      for (unsigned j=0; j<groups[i]->tests.size(); j++) {
         if (groups[i]->tests[j]->disabled)
            continue;
         groups[i]->tests[j]->results[program_setup_rs] = result;
         if (result != PASSED)
            reportTestResult(groups[i], groups[i]->tests[j]);
      }
   }
}
         

void startAllTests(std::vector<RunGroup *> &groups,
                   std::vector<char *> mutatee_list)
{
   // Begin setting up test parameters
   ParameterDict param;
   unsigned i;
   bool aborted_group = false;

   ParamString logfile_p(logfilename);
   ParamInt verbose_p((int) !quietFormat);
   ParamInt usehuman_p((int) useHumanLog);
   ParamInt debugprint_p((int) debugPrint);
   ParamInt noclean_p((int) noclean);
   ParamInt uniqueid_p((int) unique_id);
   ParamString humanname_p(humanlog_name);
   std::string mutateeresume_name = std::string("mutatee_") + get_resumelog_name();
   ParamString resumelog_p(mutateeresume_name.c_str());

   param["logfilename"] = &logfile_p;
   param["verbose"] = &verbose_p;
   param["usehumanlog"] = &usehuman_p;
   param["humanlogname"] = &humanname_p;
   param["debugPrint"] = &debugprint_p;
   param["noClean"] = &noclean_p;
   param["mutatee_resumelog"] = &resumelog_p;
   param["unique_id"] = &uniqueid_p;

   // Print Test Log Header
   getOutput()->log(LOGINFO, "Commencing test(s) ...\n");
#if !defined(os_windows_test)
   if ( pdscrdir )
   {
      runScript("date");
      runScript("uname -a");
   }
   getOutput()->log(LOGINFO, "TESTDIR=%s\n", getenv("PWD"));
#else
	char* cwd = _getcwd(NULL, 0);
	if(cwd) {
	   getOutput()->log(LOGINFO, "TESTDIR=%s\n", cwd);
	} else {
		getOutput()->log(LOGERR, "Couldn't get working directory!\n");
	}
	free(cwd);
#endif


   std::vector<Module *> modules;
   Module::getAllModules(modules);

   measureStart();

   for (i = 0; i < groups.size(); i++) {
      if (groups[i]->disabled)
         continue;

      //If we fail then have the log resume us at this group
      log_resumepoint(i, 0);

      initModuleIfNecessary(groups[i], groups, param);

      // Print mutatee (run group) header
      printLogMutateeHeader(groups[i]->mutatee, groups[i]->linktype);

      int before_group = numUnreportedTests(groups[i]);
      if (!before_group)
         continue;

      executeGroup(groups[i]->mod->tester, groups[i], param);
      int after_group = numUnreportedTests(groups[i]);
   
      if (after_group) {
         if (before_group == after_group) {
            //This should be uncommon.  We made no forward progress
            // running tests in the group, and we have tests that didn't run.
            // Mark the group as failed, as we don't want to just spin here.
            for (unsigned j=0; j<groups[i]->tests.size(); j++) {
               if (!shouldRunTest(groups[i], groups[i]->tests[j]))
                  continue;
               groups[i]->tests[j]->results[group_teardown_rs] = FAILED;
               reportTestResult(groups[i], groups[i]->tests[j]);
            }
         }
         else {
            aborted_group = true;
            break;
         }
      }
   }

   unsigned final_group = i;
   
   for (i = 0; i < final_group; i++) {
     Module *mod = groups[i]->mod;
     if (!mod || !mod->isInitialized() || groups[i]->disabled)
       continue;
     
     log_teststart(groups[i]->index, 0, program_teardown_rs);
     test_results_t result = mod->tester->program_teardown(param);
     log_testresult(result);

     for (unsigned j=0; j < groups[i]->tests.size(); j++)
     {
       if (!groups[i]->tests[j]->disabled) {
          groups[i]->tests[j]->results[program_teardown_rs] = result;
       }
       reportTestResult(groups[i], groups[i]->tests[j]);
     }
     mod->setInitialized(false);
   }

   if (!aborted_group) {
     if (limitSkippedTests) {
       log_resumepoint(limitResumeGroup, limitResumeTest);
     } else {
       log_clear();
     }
   }
      
   measureEnd();

   cleanPIDFile();
   return;
} // startAllTests()

void DebugPause() {
   getOutput()->log(STDERR, "Waiting for attach by debugger\n");
#if defined(os_windows_test)
   DebugBreak();
#else
   static volatile int set_me = 0;
   while (!set_me)
     P_sleep(1);
#endif
}

// Find the paradyn scripts directory.  It can be specified in the environment
// variable 'PDSCRDIR', or we pick up a default location for the UW and UMD
// sites, or we generate a default based on the DYNINST_ROOT directory, or
// we use a default '../../../scripts'.
void setPDScriptDir(bool skip_warning)
{
   pdscrdir = getenv("PDSCRDIR");
   if ( pdscrdir == NULL )
   {
#if !defined(os_windows_test)
      // Environment variable not set, try default wisc/umd directories
      DIR *dir;
      dir = opendir(uw_pdscrdir);
      if ( dir != NULL ) {
         closedir(dir);
         pdscrdir = uw_pdscrdir;
         return;
      }
      dir = opendir(umd_pdscrdir);
      if ( dir != NULL ) {
         closedir(dir);
         pdscrdir = umd_pdscrdir;
         return;
      }

      // Environment variable not set and default UW/UMD directories missing.
      // Derive default from DYNINST_ROOT
      char *basedir = getenv("DYNINST_ROOT");
      if (NULL == basedir) {
         // DYNINST_ROOT not set.  Print a warning, and default it to "../../.."
         if (!skip_warning) {
            getOutput()->log(STDERR, "** WARNING: DYNINST_ROOT not set.  Please set the environment variable\n");
            getOutput()->log(STDERR, "\tto the path for the top of the Dyninst library installation.\n");
            getOutput()->log(STDERR, "\tUsing default: '../../..'\n");
         }
         basedir = "../../..";
      }
      int basedir_len = strlen(basedir);
      int pdscrdir_len = basedir_len + strlen("/scripts") + 1;
      // BUG This allocated array lasts for the lifetime of the program, so it's
      // currently not worth worrying about freeing the memory.
      pdscrdir = new char[pdscrdir_len];
      strncpy(pdscrdir, basedir, basedir_len + 1);
      strcat(pdscrdir, "/scripts");

      dir = opendir(pdscrdir);
      if (!dir) {
         pdscrdir = NULL;
      } else {
         closedir(dir);
      }

#endif
   }
}
   
void updateSearchPaths(const char *filename) {
#if !defined(os_windows_test)
   // First, find the directory we reside in

    bool include_cwd_always = false;
#if defined(os_aix_test)
    // AIX strips a ./ from the start of argv[0], so
    // we will execute ./test_driver and see test_driver

    include_cwd_always = true;
#endif

   char *execpath;
   char pathname[PATH_MAX];
   getcwd(pathname, PATH_MAX);

   if (filename[0] == '/') {
      // If it begins with a slash, it's an absolute path
      execpath = strdup(filename);
   } else if (strchr(filename,'/') || include_cwd_always) {
      // If it contains slashes, it's a relative path
      char *filename_copy = strdup(filename);
      
      execpath = (char *) ::malloc(strlen(pathname) + strlen(filename_copy) + 2);
      strcpy(execpath,pathname);
      strcat(execpath,"/");
      strcat(execpath,filename_copy);
      ::free(filename_copy);
   } else {
      // If it's just a name, it was found in PATH. 
      // Add current directory to the search path
      const char *pathenv = getenv("PATH");
      char *newpath = (char *) malloc (strlen(pathenv)+3);
      strcat(newpath, pathenv);
      strcat(newpath, ":.");
      execpath = searchPath(newpath, filename);
      if(execpath == NULL) {
         //  Not found in PATH - we'll assume it should be in CWD
         return;
      }
   }

   *strrchr(execpath, '/') = '\0';
   // Now update PATH and LD_LIBRARY_PATH/LIBPATH

   char *envCopy;

   char *envPath = getenv("PATH");
   envCopy = (char *) ::malloc(((envPath && strlen(envPath)) ? strlen(envPath) + 1 : 0) + strlen(execpath) + 6);
   strcpy(envCopy, "PATH=");
   if (envPath && strlen(envPath)) {
      strcat(envCopy, envPath);
      strcat(envCopy, ":");
   }
   strcat(envCopy, execpath);
   assert(!putenv(envCopy));
    
   char *envLibPath;
#if defined(os_aix_test)
   envLibPath = getenv("LIBPATH");
#else
   envLibPath = getenv("LD_LIBRARY_PATH");
#endif
    
   envCopy = (char *) ::malloc(((envLibPath && strlen(envLibPath)) ? strlen(envLibPath) + 1 : 0) + strlen(execpath) + 17);
#if defined(os_aix_test)
   strcpy(envCopy, "LIBPATH=");
#else
   strcpy(envCopy, "LD_LIBRARY_PATH=");
#endif
   if (envLibPath && strlen(envLibPath)) {
      strcat(envCopy, envLibPath);
      strcat(envCopy, ":");
   }
   strcat(envCopy, execpath);
   assert(!putenv(envCopy));

   //fprintf(stderr, "%s[%d]:  set LD_LIBRARY_PATH to %s\n", FILE__, __LINE__, getenv("LD_LIBRARY_PATH"));
   ::free(execpath);
#endif
}

bool testsRemain(std::vector<RunGroup *> &groups)
{
   for (unsigned  i = 0; i < groups.size(); i++) {
      if (groups[i]->disabled)
         continue;
      for (unsigned j=0; j<groups[i]->tests.size(); j++) {
         if (shouldRunTest(groups[i], groups[i]->tests[j]))
            return true;
      }
   }
   return false;
}

int main(int argc, char *argv[]) {
   updateSearchPaths(argv[0]);

   setOutput(new StdOutputDriver(NULL));
   
   int result = parseArgs(argc, argv);
   if (result)
      exit(result);

   if (unique_id) {
      char id_string[32];
      snprintf(id_string, 32, "%d", unique_id);
      std::string newname = std::string(get_resumelog_name()) + std::string(".") + std::string(id_string);
      set_resumelog_name(strdup(newname.c_str()));
   }

   // Fill in tests vector with lists of test to run
   std::vector<RunGroup *> tests;
   initialize_mutatees(tests);  

   result = setupLogs();
   if (result)
      exit(result);

   setupProcessGroup();

   // Set the resume log name
   if ( getenv("RESUMELOG") ) {
      set_resumelog_name(getenv("RESUMELOG"));
   }

   if ( shouldDebugBreak ) {
      DebugPause();
   }

   startAllTests(tests, mutatee_list);

   if ((outlog != NULL) && (outlog != stdout)) {
      fclose(outlog);
   }
   fflush(stdout);

   if (!testsRemain(tests) && !limitSkippedTests)
      return NOTESTS;
   return 0;
}

int setupLogs()
{
   // Set the script dir if we require scripts
   setPDScriptDir(true);

   // Set up the logging file..
   if ((logfilename != NULL) && (strcmp(logfilename, "-") != 0)) {
      outlog = fopen(logfilename, "a");
      if (NULL == outlog) {
         getOutput()->log(STDERR, "Error opening log file '%s'\n", logfilename);
         return NOTESTS;
      }
      errlog = outlog;
   } else {
      outlog = stdout;
      errlog = stderr;
   }
   setOutputLog(outlog);
   setErrorLog(errlog);
   setOutputLogFilename(logfilename);
   setErrorLogFilename(logfilename);

   if ((logfilename != NULL) && (strcmp(logfilename, "-") != 0)) {
      getOutput()->redirectStream(LOGINFO, logfilename);
      getOutput()->redirectStream(LOGERR, logfilename);
   }
   if (useHumanLog) {
      getOutput()->redirectStream(HUMAN, humanlog_name);
   } else {
      getOutput()->redirectStream(HUMAN, NULL);
   }
   return 0;
}

