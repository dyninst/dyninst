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

// $Id: test_driver.C,v 1.6 2008/10/20 20:37:11 legendre Exp $
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <string>
#include <errno.h>
#include <vector>
#include <iostream>
#include <sstream>

#include <sys/stat.h>
#include <time.h>

#if defined(i386_unknown_nt4_0)
#define vsnprintf _vsnprintf
#define snprintf _snprintf
#pragma warning(disable:4786)
#else
#include <fnmatch.h>
#include <dirent.h>
#endif

#include "ParameterDict.h"
#include "test_lib.h"
#include "error.h"
#include "ResumeLog.h"
#include "TestOutputDriver.h"
#include "StdOutputDriver.h"
#include "comptester.h"

// Globals, should be eventually set by commandline options
bool runAllTests = true;
bool ranLastTest = false;
bool runAllMutatees = true;
bool runAllOptions = true;
bool runCreate = false;
bool runAttach = false;
bool useHumanLog = true;
bool enableLogging = true;
bool printLabels = false;
bool shouldDebugBreak = false;
bool called_from_runTests = false;
bool quietFormat = false;
int skipToTest = 0;
int skipToMutatee = 0;
int skipToOption = 0;
int testLimit = 0;
int testsRun = 0;
int errorPrint = 0;
int debugPrint = 0;
char *resumelog_name = "resumelog";
char *humanlog_name = "-";
char *crashlog_name = "crashlog";
std::vector<char *> mutatee_list;
std::vector<char *> test_list;

#if defined(os_windows)
char *logfilename = "NUL";
#else
char *logfilename = "/dev/null";
#endif
FILE *outlog = NULL;
FILE *errlog = NULL;

char *pdscrdir = NULL;
char *uw_pdscrdir = "/p/paradyn/builds/scripts";
char *umd_pdscrdir = "/fs/dyninst/dyninst/current/scripts";

int parseArgs(int argc, char *argv[]);
int setupLogs();

// Include test setup data
#include "test_info_new.h"

#if !defined(os_windows)
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

bool runOnThisPlatform(test_data_t &test)
{
#if defined(alpha_dec_osf4_0)
   return test.platforms.alpha_dec_osf5_1;
#elif defined(i386_unknown_linux2_0)
   return test.platforms.i386_unknown_linux2_4;
#elif defined(i386_unknown_nt4_0)
   return test.platforms._i386_unknown_nt4_0;
#elif defined(ia64_unknown_linux2_4) 
   return test.platforms._ia64_unknown_linux2_4;
#elif defined(x86_64_unknown_linux2_4)
   return test.platforms._x86_64_unknown_linux2_4;
#elif defined(mips_sgi_irix6_5)
   return test.platforms.mips_sgi_irix6_5;
#elif defined(rs6000_ibm_aix5_1) 
   return test.platforms._rs6000_ibm_aix5_1;
#elif defined(sparc_sun_solaris2_4)
   return test.platforms.sparc_sun_solaris2_8;
#else
   return true;
#endif
}

void printLogMutateeHeader(char *mutatee)
{
   if (!enableLogging)
      return;
   flushOutputLog();
   flushErrorLog();
   // Mutatee Description
   // Mutatee Info
   if ( (mutatee != NULL) && (strcmp(mutatee, "") != 0) ) {
      getOutput()->log(LOGINFO, "[Tests with %s]\n", mutatee);
#if !defined(os_windows)
      if ( pdscrdir ) {
         runScript("ls -lLF %s", mutatee);
         runScript("%s/ldd_PD %s", pdscrdir, mutatee);
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
#if defined(os_windows)
   // Sadly, we can't assume the presence of fnmatch on Windows
   return (strcmp(wcname, tomatch) == 0);
#else
   // The other systems we support are unix-based and should provide fnmatch (?)
   return (fnmatch(wcname, tomatch, 0) == 0);
#endif
}

// Returns true if the vector mutatee_list contains the string mutatee, and
// returns false if it does not
bool mutateeListContains(std::vector<char *> mutatee_list, char *mutatee) {
   if (NULL == mutatee) {
      return false;
   }
   for (size_t i = 0; i < mutatee_list.size(); i++) {
      if (nameMatches(mutatee_list[i], mutatee)) {
         return true;
      }
   }
   return false;
}

// Runs through all the test names in testsn, and enables any matching tests
// in testsv.  If any tests matched, returns true.  If there were no matching
// tests, returns false
// Okay, we don't actually enable any tests here; we just disable tests that
// don't match.  All tests start out enabled, and we previously disabled any
// that are crashing while we were parsing the resume log.
bool testListContains(TestInfo * test,
                      std::vector<char *> &testsn) {
   bool match_found = false;

   for (size_t i = 0; i < testsn.size(); i++) {
      if (nameMatches(testsn[i], test->name))
         return true;
   }
   return false;
}

void reportTestResult(RunGroup *group, TestInfo *test)
{
   if (test->result_reported || test->disabled)
      return;

   test_results_t result = UNKNOWN;

   for (unsigned i=0; i<NUM_RUNSTATES; i++)
   {
      if (test->results[i] == FAILED ||
          test->results[i] == CRASHED || 
          test->results[i] == SKIPPED) {
         result = test->results[i];
         break;
      }
      else if (test->results[i] == PASSED) {
         result = test->results[i];
      }
      else if (test->results[i] == UNKNOWN) {
         return;
      }
      else {
         assert(0 && "Unknown run state");
      }
   }
   assert(result != UNKNOWN);

   std::map<std::string, std::string> attrs;
   TestOutputDriver::getAttributesMap(test, group, attrs);
   getOutput()->startNewTest(attrs);
   getOutput()->logResult(result);
   getOutput()->finalizeOutput();

   log_testreported(group->index, test->index);
   test->result_reported = true;
}

int numUnreportedTests(RunGroup *group)
{
   int num_unreported = 0;

   for (unsigned i=0; i<group->tests.size(); i++)
   {
      if (shouldRunTest(group, group->tests[i]))
         num_unreported++;
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
      log_teststart(group_num, test_num, test_teardown_rs);
   }

   for (unsigned j=0; j<new_params.size(); j++)
      delete new_params[j];
}

void disableUnwantedTests(std::vector<RunGroup *> groups)
{
   if (!runAllOptions) {
      for (unsigned  i = 0; i < groups.size(); i++) {
         if (((groups[i]->useAttach == CREATE) && runAttach) ||
             ((groups[i]->useAttach == USEATTACH) && runCreate))
         {
            for (unsigned j=0; j<groups[i]->tests.size(); j++)
               groups[i]->tests[j]->disabled = true;
            groups[i]->disabled = true;
         }
      }
   }
   if (!runAllMutatees)
   {
      for (unsigned  i = 0; i < groups.size(); i++) {
         if (!mutateeListContains(mutatee_list, groups[i]->mutatee))
         {
            for (unsigned j=0; j<groups[i]->tests.size(); j++)
               groups[i]->tests[j]->disabled = true;
            groups[i]->disabled = true;
         }
      }
   }
   if (!runAllTests)
   {
      for (unsigned  i = 0; i < groups.size(); i++) {
         for (unsigned j=0; j<groups[i]->tests.size(); j++) {
            if (!testListContains(groups[i]->tests[j], test_list)) {
               groups[i]->tests[j]->disabled = true;
            }
         }
      }
   }

   parse_resumelog(groups);
   for (unsigned  i = 0; i < groups.size(); i++) {
      if (groups[i]->disabled)
         continue;
      groups[i]->disabled = true;
      for (unsigned j=0; j<groups[i]->tests.size(); j++) {
         if (!groups[i]->tests[j]->disabled)
            groups[i]->disabled = false;
      }
   }       
}

void setIndexes(std::vector<RunGroup *> groups)
{
   for (unsigned  i = 0; i < groups.size(); i++) {
      groups[i]->index = i;
      for (unsigned j=0; j<groups[i]->tests.size(); j++) {
         groups[i]->tests[j]->index = j;
      }
   }
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

   int groupnum = group->index;

   ParamString mutatee_prm(group->mutatee);
   ParamInt startstate_prm((int) group->state);
   ParamInt createmode_prm((int) group->useAttach);
   ParamInt customexecution_prm((int) group->customExecution);
   ParamInt selfstart_prm((int) group->selfStart);
   param["pathname"] = &mutatee_prm;
   param["startState"] = &startstate_prm;
   param["useAttach"] = &createmode_prm;
   param["customExecution"] = &customexecution_prm;
   param["selfStart"] = &selfstart_prm;
   

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

   for (int i = 0; i < group->tests.size(); i++) {
      // Print mutator log header
      printLogOptionHeader(group->tests[i]);

      if (shouldRunTest(group, group->tests[i])) {
         log_teststart(groupnum, i, test_init_rs);
         group->tests[i]->results[test_init_rs] = group->tests[i]->mutator->setup(param);
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

void startAllTests(std::vector<RunGroup *> &groups,
                   std::vector<char *> mutatee_list)
{

   // Begin setting up test parameters
   ParameterDict param;
   unsigned i;

   ParamString logfile_p(logfilename);
   ParamInt verbose_p((int) !quietFormat);
   ParamInt usehuman_p((int) useHumanLog);
   ParamInt printlabels_p((int) printLabels);
   ParamInt debugprint_p((int) debugPrint);
   ParamString humanname_p(humanlog_name);

   param["logfilename"] = &logfile_p;
   param["verbose"] = &verbose_p;
   param["usehumanlog"] = &usehuman_p;
   param["humanlogname"] = &humanname_p;
   param["printlabels"] = &printlabels_p;
   param["debugPrint"] = &debugprint_p;

   // Print Test Log Header
   getOutput()->log(LOGINFO, "Commencing DyninstAPI test(s) ...\n");
#if !defined(os_windows)
   if ( pdscrdir )
   {
      runScript("date");
      runScript("uname -a");
   }
#endif
   getOutput()->log(LOGINFO, "TESTDIR=%s\n", getenv("PWD"));

   // Sets the disable flag on groups and tests that weren't selected by
   // options or have alread been passed according to the resumelog
   setIndexes(groups);
   disableUnwantedTests(groups);

   //TODO: Once Bill has the group associated with a component, lookup and
   // load the specific component for each group.
   ComponentTester *component = componentTesterFactory();
   test_results_t result = component->program_setup(param);
   for (unsigned i=0; i<groups.size(); i++) {
      for (unsigned j=0; j<groups[i]->tests.size(); j++) {
         groups[i]->tests[j]->results[program_setup_rs] = result;
      }
   }
      
   
   // Now run each of the run groups in to_run
   for (i = 0; i < groups.size(); i++) {
      if (groups[i]->disabled)
         continue;

      //If we fail (or have the test limit reached), then have the log resume 
      // us at this group
      log_resumepoint(i, 0);

      if (testLimit && testsRun >= testLimit) {
         return;
      }

      // Print mutatee (run group) header
      printLogMutateeHeader(groups[i]->mutatee);

      executeGroup(component, groups[i], param);
       
      if (numUnreportedTests(groups[i])) {
         //This should be uncommon.  We have tests in this group that didn't
         // complete (neither failure nor success), mark these as failures.
         for (unsigned j=0; j<groups[i]->tests.size(); j++) {
            if (shouldRunTest(groups[i], groups[i]->tests[j]))
            {
               //Just pick a result to set as failed
               groups[i]->tests[j]->results[group_teardown_rs] = FAILED;
               reportTestResult(groups[i], groups[i]->tests[j]);
            }
         }
      }
   }

   if (i==groups.size())
      ranLastTest = true;

   log_clear();
    
   return;
} // startAllTests()

void DebugPause() {
   getOutput()->log(STDERR, "Waiting for attach by debugger\n");
#if defined(os_windows)
   DebugBreak();
#else
   sleep(10);
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
#if !defined(os_windows)
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
#if !defined(os_windows)
   // First, find the directory we reside in

   char *execpath;
   char pathname[PATH_MAX];
   getcwd(pathname, PATH_MAX);

   if (filename[0] == '/') {
      // If it begins with a slash, it's an absolute path
      execpath = strdup(filename);
   } else if (strchr(filename,'/')) {
      // If it contains slashes, it's a relative path
      char *filename_copy = strdup(filename);
      
      execpath = (char *) ::malloc(strlen(pathname) + strlen(filename_copy) + 2);
      strcpy(execpath,pathname);
      strcat(execpath,"/");
      strcat(execpath,filename_copy);
      ::free(filename_copy);
   } else {
      // If it's just a name, it was found in PATH
      const char *pathenv = getenv("PATH");
      execpath = searchPath(pathenv, filename);
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
#if defined(os_aix)
   envLibPath = getenv("LIBPATH");
#else
   envLibPath = getenv("LD_LIBRARY_PATH");
#endif
    
   envCopy = (char *) ::malloc(((envLibPath && strlen(envLibPath)) ? strlen(envLibPath) + 1 : 0) + strlen(execpath) + 17);
#if defined(os_aix)
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

   ::free(execpath);
#endif
}

int main(int argc, char *argv[]) {
   updateSearchPaths(argv[0]);

   setOutput(new StdOutputDriver(NULL));
   
   int result = parseArgs(argc, argv);
   if (result)
      exit(result);

   // Fill in tests vector with lists of test to run
   std::vector<RunGroup *> tests;
   initialize_mutatees(tests);  

   result = setupLogs();
   if (result)
      exit(result);

   if (test_list.empty()) {
      getOutput()->log(LOGINFO, "WARNING: No tests specified\n");
   }
   if (mutatee_list.empty()) {
      getOutput()->log(LOGINFO, "WARNING: No mutatees specified\n");
   }

   // Set the resume log name
   if ( getenv("RESUMELOG") ) {
      resumelog_name = getenv("RESUMELOG");
   }

   if ( shouldDebugBreak ) {
      DebugPause();
   }

   startAllTests(tests, mutatee_list);

   if ((outlog != NULL) && (outlog != stdout)) {
      fclose(outlog);
   }
   fflush(stdout);

   if (testsRun == 0 || ranLastTest)
      return NOTESTS;
   return 0;
}

int setupLogs()
{
   // Set the script dir if we require scripts
   if ( enableLogging )
   {
      setPDScriptDir(called_from_runTests);
   }

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

int parseArgs(int argc, char *argv[])
{
   for (unsigned i=1; i < argc; i++ )
   {

      if (strncmp(argv[i], "-v+", 3) == 0)
         errorPrint++;
      if (strncmp(argv[i], "-v++", 4) == 0)
         errorPrint++;
      if (strncmp(argv[i], "-verbose", 2) == 0)
         debugPrint = 1;
      else if (strcmp(argv[i], "-q") == 0)
      {
         getOutput()->log(STDERR, "[%s:%u] - Quiet format not yet enabled\n");
         quietFormat = true;
      }
      else if ( strcmp(argv[i], "-skipTo")==0)
         // TODO Remove this option
      {
         // skip to test i+1
         skipToTest = atoi(argv[++i]);
      }
      else if ( strcmp(argv[i], "-log")==0)
      {
         bool logfile_found = false;
         if ((i + 1) < argc) {
            // Check whether the next argument is a filename to log to
            if ((argv[i + 1][0] != '-') || (argv[i + 1][1] == '\0')) {
               // It either doesn't start with '-' or is exactly "-"
               i += 1;
               logfilename = argv[i];
               logfile_found = true;
            }
         }
      }
      else if ( strcmp(argv[i], "-logfile") == 0) {
         getOutput()->log(STDERR, "WARNING: -logfile is a deprecated option; use -log instead\n");
         /* Store the log file name */
         if ((i + 1) >= argc) {
            getOutput()->log(STDERR, "Missing log file name\n");
            return NOTESTS;
         }
         i += 1;
         logfilename = argv[i];
      }
      else if ( strcmp(argv[i], "-debug")==0)
      {
         shouldDebugBreak = true;
      }
      else if ( strcmp(argv[i], "-test") == 0)
      {
         char *tests;
         char *name;

         runAllTests = false;
         if ( i + 1 >= argc )
         {
            getOutput()->log(STDERR, "-test must be followed by a testname\n");
            return NOTESTS;
         }

         tests = strdup(argv[++i]);

         name = strtok(tests, ","); // FIXME Use strtok_r()
         test_list.push_back(name);
         while ( name != NULL )
         {
            name = strtok(NULL, ",");
            if ( name != NULL )
            {
               test_list.push_back(name);
            }
         }
      }
      else if ( strcmp(argv[i], "-mutatee") == 0)
      {
         char *mutatees;
         char *name;

         runAllMutatees = false;
         if ( i + 1 >= argc )
         {
            getOutput()->log(STDERR, "-mutatee must be followed by mutatee names\n");
            return NOTESTS;
         }

         mutatees = strdup(argv[++i]);

         name = strtok(mutatees, ","); // FIXME Use strtok_r()
         if (NULL == name) {
            // Special handling for a "" mutatee specified on the command line
            mutatee_list.push_back("");
         } else {
            mutatee_list.push_back(name);
         }
         while ( name != NULL )
         {
            name = strtok(NULL, ","); // FIXME Use strtok_r()
            if ( name != NULL )
            {
               mutatee_list.push_back(name);
            }
         }
      }
      // TODO: Remove the -run option, it is replaced by the -test option
      else if ( strcmp(argv[i], "-run") == 0)
      {
         unsigned int j;
         runAllTests = false;
         for ( j = i+1; j < argc; j++ )
         {
            if ( argv[j][0] == '-' )
            {
               // end of test list
               break;
            }
            else
            {
               test_list.push_back(argv[j]);
            }
         }
         i = j - 1;
      }
      // TODO -attach and -create are DyninstAPI specific
      else if ( strcmp(argv[i], "-attach") == 0 )
      {
         runAllOptions = false;
         runAttach = true;
      }
      else if ( strcmp(argv[i], "-create") == 0 )
      {
         runAllOptions = false;
         runCreate = true;
      }
      else if ((strcmp(argv[i], "-enable-resume") == 0) ||
               (strcmp(argv[i], "-use-resume") == 0)) {
         enableResumeLog();
      } else if ( strcmp(argv[i], "-limit") == 0 ) {
         if ( i + 1 >= argc ) {
            getOutput()->log(STDERR, "-limit must be followed by an integer limit\n");
            return NOTESTS;
         }
         testLimit = strtol(argv[++i], NULL, 10);
         if ((0 == testLimit) && (EINVAL == errno)) {
            getOutput()->log(STDERR, "-limit must be followed by an integer limit\n");
            return NOTESTS;
         }
      }
      else if ( strcmp(argv[i], "-humanlog") == 0 ) {
         // Verify that the following argument exists
         if ( i + 1 >= argc )
         {
            getOutput()->log(STDERR, "-humanlog must by followed by a filename\n");
            return NOTESTS;
         }

         useHumanLog = true;
         humanlog_name = argv[++i];
      } else if (strcmp(argv[i], "-under-runtests") == 0) {
         called_from_runTests = true;
      }
      else if ( strcmp(argv[i], "-help") == 0) {
         getOutput()->log(STDOUT, "Usage: %s [-skipTo <test_num>] [-humanlog filename] [-verbose]\n", argv[0]);
         getOutput()->log(STDOUT, "       [-log] [-test <name> ...]\n", argv[0]);
         return SUCCESS;
      }
      else if (strcmp(argv[i], "-pidfile") == 0) {
         char *pidFilename = NULL;
         if (i + 1 >= argc) {
            getOutput()->log(STDERR, "-pidfile must be followed by a filename\n");
            return NOTESTS;
         }
         i += 1;
         pidFilename = argv[i];
         setPIDFilename(pidFilename);
      }
      else if (strcmp(argv[i], "-dboutput") == 0) {
         char * failedOutputFile = NULL;
         //check if a failed output file is specified
         if ((i + 1) < argc) {
            if (argv[i+1][0] != '-' || argv[i+1][1] == '\0') {
               //either it doesn't start with - or it's exactly -
               i++;
               failedOutputFile = argv[i];
            }
         }

         if (NULL == failedOutputFile) {
            //TODO insert proper value
            time_t rawtime;
            struct tm * timeinfo = (tm *)malloc(sizeof(struct tm));

            time(&rawtime);
            timeinfo = localtime(&rawtime);

            failedOutputFile = (char*)malloc(sizeof(char) * strlen("sql_dblog-xxxx-xx-xx0"));
            if (failedOutputFile == NULL) {
               fprintf(stderr, "[%s:%u] - Out of memory!\n", __FILE__, __LINE__);
               // TODO Handle error;
            }
            sprintf(failedOutputFile, "sql_dblog-%4d-%02d-%02d",
                    timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);

            getOutput()->log(STDERR, "No 'SQL log file' found, using default %s\n", failedOutputFile);
         }

         std::string s_failedOutputFile (failedOutputFile);
         TestOutputDriver *newoutput = loadOutputDriver("DatabaseOutputDriver", &s_failedOutputFile);

         //make sure it loaded correctly before replacing default output
         if (newoutput != NULL) {
            setOutput(newoutput);
         }
      }
      else
      {
         getOutput()->log(STDOUT, "Unrecognized option: '%s'\n", argv[i]);
         return NOTESTS;
      }
   }
   return 0;
}
