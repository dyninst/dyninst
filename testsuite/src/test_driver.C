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
#include <sys/time.h>
#include <sys/resource.h>
#include <strings.h>
#endif

#include "ParameterDict.h"
#include "test_lib.h"
#include "error.h"
#include "ResumeLog.h"
#include "TestOutputDriver.h"
#include "StdOutputDriver.h"
#include "comptester.h"
#include "CmdLine.h"
#include "module.h"
#include "MutateeStart.h"
#include "remotetest.h"

using namespace std;

int testsRun = 0;

FILE *outlog = NULL;
FILE *errlog = NULL;
char *logfilename;
FILE *debug_log;

int gargc;
char **gargv;

void initModuleIfNecessary(RunGroup *group, std::vector<RunGroup *> &groups, 
                           ParameterDict &params);
int LMONInvoke(RunGroup *, ParameterDict params, char *test_args[], char *daemon_args[], bool attach);

int setupLogs(ParameterDict &params);

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
//static void *mem_start;
//static struct tms start_time;

#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

#ifndef timersub
#define timersub(b, a, r) \
do { \
    (r)->tv_sec = (b)->tv_sec - (a)->tv_sec;\
    (r)->tv_usec = (b)->tv_usec -(a)->tv_usec;\
    if((r)->tv_usec < 0) {\
        (r)->tv_sec--;\
        (r)->tv_usec += 1000000;\
    } \
} while(0)
#endif

class testMetrics
{
    public:
    testMetrics(TestOutputDriver* d)
    : driver(d)
    {
        if (!measureMEMCPU || !d)
            return;

        getrusage(RUSAGE_SELF, &start_time);
    }
    ~testMetrics()
    {
        if (!measureMEMCPU || !driver)
            return;

        //unsigned long mem_end = get_mem_usage();
        struct rusage end_time;
        getrusage(RUSAGE_SELF, &end_time);

        //signed long mem_diff = mem_end - mem_start;
        timeval utime, stime;
        timersub(&end_time.ru_utime, &start_time.ru_utime, &utime);
        timersub(&end_time.ru_stime, &start_time.ru_stime, &stime);
        double ut, st;
        ut = (double)(utime.tv_sec) + (double)(utime.tv_usec)/1000000;
        st = (double)(stime.tv_sec) + (double)(stime.tv_usec)/1000000;
        // If Linux ever fills out the full rusage structure, we'll capture the difference in high-water mark here.
        // Right now, this should log zeroes for our memory usage...we're still getting the framework in at least.
        // Note that RSS is measured in kB, so we'll scale...
        driver->logMemory(1024 * (end_time.ru_maxrss - start_time.ru_maxrss));  
        driver->logTime(ut + st);
    }
    private:
        TestOutputDriver* driver;
        struct rusage start_time;
};

#else

class testMetrics
{
    public:
        testMetrics(TestOutputDriver*) {}
};

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
      runScript("ls -lLF %s", mutatee);
      if( linktype == DynamicLink ) 
         runScript("ldd %s", mutatee);
      else
         getOutput()->log(LOGINFO, "%s: statically linked\n", mutatee);
#endif
   } else {
      getOutput()->log(LOGINFO, "[Tests with none]\n");
   }

   getOutput()->log(LOGINFO, "\n");
   flushOutputLog();
   flushErrorLog();
}

void printLogOptionHeader(TestInfo *tinfo, ParameterDict &params)
{
   if (!params["printMutateeLogHeader"]->getInt())
      return;
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

void setupRemoteTests(vector<RunGroup *> &groups)
{
   for (unsigned i=0; i < groups.size(); i++) {
      if (groups[i]->disabled)
         continue;
      if (groups[i]->mutator_location == remote) {
         groups[i]->modname = "remote::" + groups[i]->modname;
      }
   }
}

int setupRemoteMutator(RunGroup *group)
{
   int count = 0;
   for (unsigned i=0; i<group->tests.size(); i++) {
      if (group->tests[i]->disabled)
         continue;
      group->tests[i]->mutator = RemoteTestFE::createRemoteTestFE(group->tests[i], getConnection());
      count++;
   }
   return count;
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

    {
        testMetrics m(getOutput());

        if(shouldRunTest(group, test))
        {
            log_teststart(group_num, test_num, test_setup_rs);
            test->results[test_setup_rs] = tester->test_setup(test, param);
            log_testresult(test->results[test_setup_rs]);
        }
        if(shouldRunTest(group, test))
        {
            log_teststart(group_num, test_num, test_execute_rs);
            test->results[test_execute_rs] = test->mutator->executeTest();
            log_testresult(test->results[test_execute_rs]);
        }
        if(shouldRunTest(group, test))
        {
            log_teststart(group_num, test_num, test_teardown_rs);
            test->results[test_teardown_rs] = tester->test_teardown(test, param);
            log_testresult(test->results[test_teardown_rs]);
        }
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

static void clearConnection()
{
   Connection *con = getConnection();
   if (!con)
      return;
   delete con;
   setConnection(NULL);
}

bool setupConnectionToRemote(RunGroup *group, ParameterDict &params)
{
   clearConnection();
   Connection *con = new Connection();
   setConnection(con);

   //Setup connection
   std::string hostname;
   int port;
   bool result = con->server_setup(hostname, port);
   if (!result) {
      fprintf(stderr, "Could not setup server socket\n");
      return false;
   }

   //Mutatee params
   string mutatee_exec;
   vector<string> mutatee_args;
   result = getMutateeParams(group, params, mutatee_exec, mutatee_args);
   if (!result) {
      fprintf(stderr, "Failed to collect mutatee params\n");
   }
   char **c_mutatee_args = getCParams(mutatee_exec, mutatee_args);

   //driver params;
   vector<string> driver_args;
#if defined(RUN_IN_VALGRIND) 
   string driver_exec = "valgrind";
   driver_args.push_back("--tool=memcheck");
   driver_args.push_back("testdriver_be");
#else
   string driver_exec = "/g/g22/legendre/pc_bluegene/dyninst/testsuite/ppc32_bgp_ion/testdriver_be";
#endif
   char port_s[32];
   snprintf(port_s, 32, "%d", port);
   driver_args.push_back("-hostname");
   driver_args.push_back(hostname);
   driver_args.push_back("-port");
   driver_args.push_back(port_s);
   for (unsigned i=0; i<gargc; i++) {
      driver_args.push_back(gargv[i]);
   }
   char **c_driver_args = getCParams(driver_exec, driver_args);

   bool attach_mode = (group->createmode == USEATTACH);
   int result_i = LMONInvoke(group, params, c_mutatee_args, c_driver_args, attach_mode);

   result = con->server_accept();
   if (!result) {
      fprintf(stderr, "Failed to accept connection from client\n");
      return false;
   }
}

void executeGroup(RunGroup *group,
                  vector<RunGroup *> &groups,
                  ParameterDict param)
{
   setMutateeDict(group, param);

   bool is_remote = (group->mutator_location == remote);
   if (is_remote && !group->disabled)
      setupConnectionToRemote(group, param);

   initModuleIfNecessary(group, groups, param);

   test_results_t result;
   // setupMutatorsForRunGroup creates TestMutator objects and
   // sets a pointer in the TestInfo object to point to the TestMutator for
   // each test.
   int tests_found;
   if (group->mutator_location == remote)
      tests_found = setupRemoteMutator(group);
   else
      tests_found = setupMutatorsForRunGroup(group);

   if (tests_found <= 0)
      return;
   tests_breakpoint();

   int groupnum = group->index;

   ParameterDict::iterator pdi = param.find("given_mutatee");
   if (pdi != param.end())
   {
      registerMutatee(pdi->second->getString());
   }

   for (unsigned i=0; i<group->tests.size(); i++)
   {
      if (shouldRunTest(group, group->tests[i]))
         testsRun++;
   }

   ComponentTester *tester = group->mod->tester;

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
      printLogOptionHeader(group->tests[i], param);

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

void initModuleIfNecessary(RunGroup *group, std::vector<RunGroup *> &groups, 
                           ParameterDict &params)
{
   if (group->disabled)
      return;

   bool is_remote = (group->mutator_location == remote);
   Module::registerGroupInModule(group->modname, group, is_remote);

   bool hasEnabledTest = false;
   for (unsigned j=0; j<group->tests.size(); j++) {
      if (!group->tests[j]->disabled)
         hasEnabledTest = true;
   }
   if (!hasEnabledTest)
      return;

   bool initModule = false;
   if (group->mod && !group->mod->setupRun())
      initModule = true;
   if (group->mutator_location == remote)
      initModule = true;
   if (!initModule)
      return;

   log_teststart(group->index, 0, program_setup_rs);
   test_results_t result = group->mod->tester->program_setup(params);
   log_testresult(result);

   group->mod->setSetupRun(true);

   for (unsigned i=0; i<groups.size(); i++) {
      if (groups[i]->disabled || groups[i]->modname != group->modname)
         continue;
      if (groups[i]->mutator_location == remote && groups[i] != group)
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

void startAllTests(std::vector<RunGroup *> &groups, ParameterDict &param)
{
   // Begin setting up test parameters
   unsigned i;
   bool aborted_group = false;

   // Print Test Log Header
   getOutput()->log(LOGINFO, "Commencing test(s) ...\n");
#if !defined(os_windows_test)
   runScript("date");
   runScript("uname -a");
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

   setupRemoteTests(groups);

   //measureStart();

   for (i = 0; i < groups.size(); i++) {
        if (groups[i]->disabled)
         continue;

      //If we fail then have the log resume us at this group
      log_resumepoint(i, 0);

      // Print mutatee (run group) header
      printLogMutateeHeader(groups[i]->mutatee, groups[i]->linktype);

      int before_group = numUnreportedTests(groups[i]);
      if (!before_group)
         continue;

      executeGroup(groups[i], groups, param);
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

      
      fprintf(stderr, "[%s:%u] - Remove here\n", __FILE__, __LINE__);
      break;
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
     if (param["limited_tests"]->getInt()) {
        int next_resume_group = param["next_resume_group"]->getInt();
        int next_resume_test = param["next_resume_test"]->getInt();
        if (next_resume_group != -1 && next_resume_test != -1)
           log_resumepoint(next_resume_group, next_resume_test);
        else
           log_clear();
     } else {
       log_clear();
     }
   }
      
   clearConnection();
   cleanPIDFile();

   return;
} // startAllTests()

void DebugPause() {
#if defined(os_windows_test)
   getOutput()->log(STDERR, "Waiting for attach by debugger\n");
   DebugBreak();
#else
   getOutput()->log(STDERR, "Waiting for attach by debugger to %d\n", getpid());
   static volatile int set_me = 0;
   while (!set_me)
     P_sleep(1);
#endif
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

   debug_log = stderr;

   ParameterDict params;
   int result = parseArgs(argc, argv, params);
   if (result)
      exit(result);

   gargc = argc;
   gargv = argv;

   if ( params["debugbreak"]->getInt() ) {
      DebugPause();
   }

   // Fill in tests vector with lists of test to run
   std::vector<RunGroup *> groups;
   getGroupList(groups, params);

   result = setupLogs(params);
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

   startAllTests(groups, params);

   if ((outlog != NULL) && (outlog != stdout)) {
      fclose(outlog);
   }
   fflush(stdout);

   if (!testsRemain(groups) && !params["limited_tests"]->getInt())
      return NOTESTS;
   return 0;
}

int setupLogs(ParameterDict &params)
{
   logfilename = params["logfilename"]->getString();
   // Set up the logging file..
   if (strcmp(logfilename, "-") == 0) 
   {
      outlog = stdout;
      errlog = stderr;
   }
   else
   {
      outlog = fopen(logfilename, "a");
      if (!outlog) {
         getOutput()->log(STDERR, "Error opening log file '%s'\n", logfilename);
         return NOTESTS;
      }
      errlog = outlog;
   } 

   setOutputLog(outlog);
   setErrorLog(errlog);
   setOutputLogFilename(logfilename);
   setErrorLogFilename(logfilename);

   if ((logfilename != NULL) && (strcmp(logfilename, "-") != 0)) {
      getOutput()->redirectStream(LOGINFO, logfilename);
      getOutput()->redirectStream(LOGERR, logfilename);
   }

   if (params["usehumanlog"]->getInt()) {
      getOutput()->redirectStream(HUMAN, params["humanlogname"]->getString());
   } else {
      getOutput()->redirectStream(HUMAN, NULL);
   }

   char *dbname = params["dboutput"]->getString();
   if (dbname) {
      TestOutputDriver *newoutput = loadOutputDriver(const_cast<char *>("DatabaseOutputDriver"), dbname);
      //make sure it loaded correctly before replacing default output
      if (newoutput != NULL) {
         setOutput(newoutput);
      }      
   }

   int unique_id = params["unique_id"]->getInt();
   if (unique_id) {
      char id_string[32];
      snprintf(id_string, 32, "%d", unique_id);
      std::string newname = std::string("resumelog.") +  std::string(id_string);
      set_resumelog_name(strdup(newname.c_str()));
   }
   else {
      set_resumelog_name(const_cast<char *>("resumelog"));
   }

   if (params["no_header"]->getInt()) {
      getOutput()->setNeedsHeader(false);
   }
   else {
      getOutput()->setNeedsHeader(true);
   }
   
   return 0;
}
