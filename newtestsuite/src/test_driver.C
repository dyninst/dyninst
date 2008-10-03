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

// $Id: test_driver.C,v 1.5 2008/10/03 21:12:45 legendre Exp $
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
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
#include "Callbacks.h"
#include "error.h"
#include "BPatch.h"

#include "TestOutputDriver.h"
#include "StdOutputDriver.h"

#if defined(STATIC_TEST_DRIVER)
#include "static_test.h"
#endif

// Globals, should be eventually set by commandline options
bool forceRelocation = false; // force relocation of functions
bool delayedParse = false;
bool enableLogging = true;
bool runAllTests = true;
bool runAllMutatees = true;
bool runAllOptions = true;
bool runCreate = false;
bool runAttach = false;
bool resumeLog = false;
bool useResume = false;
bool useHumanLog = true;
int skipToTest = 0;
int skipToMutatee = 0;
int skipToOption = 0;
int testLimit = 0;
int testsRun = 0;
bool fastAndLoose = false; // don't use a clean mutatee for each test
char *resumelog_name = "resumelog";
char *humanlog_name = "-";
char *crashlog_name = "crashlog";
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

char *libRTname;
#if defined(m_abi)
char *libRTname_m_abi;
#endif

int saveTheWorld = 0;
int mergeTramp = 1;
int debugPrint = 0;
int errorPrint = 0;

/* */

enum resume_stage_t {
  PRESETUP = -1,
  SETUP,
  PREEXECUTION,
  EXECUTION,
  INEXECUTION,
  POSTEXECUTION,
  TEARDOWN,
};

// Include test setup data
#include "test_info_new.h"

bool isNameExt(char *name, char *ext, int ext_len)
{
  int name_len = strlen(name);

  // Can't match
  if ( name_len < ext_len )
  {
    return false;
  }

  // If the last 4 characters match _xlc or _xlC
  // return true
  if ( strcmp(name + name_len - ext_len, ext) == 0 )
  {
    return true;
  } else
  {
    return false;
  }
}

bool isMutateeXLC(char *name)
{
  return isNameExt(name, "_xlc", 4) || isNameExt(name, "_xlC", 4);
}

bool isMutateeMABI32(char *name)
{
   return (name != NULL) && isNameExt(name, "_m32", 4);
}

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

// Test Functions
int cleanup(BPatch *bpatch, BPatch_thread *appThread, test_data_t &test, ProcessList &proc_list, int result)
{
  if ( test.cleanup == COLLECT_EXITCODE ) 
  {
    if ( result < 0 ) {
      // Test failed in the mutator
      // Terminate the mutatees
      dprintf("Test failed, calling cleanup\n");

      // Reset Error callback
      //bpatch->registerErrorCallback(errorFunc);

      proc_list.terminateAllThreads();
      return result;
    }
    else
    {
      // The mutator did not detect a failure.
      // The next step is to pull the result from the mutatee
       
      // Start of code to continue the process.  All mutations made
      // above will be in place before the mutatee begins its tests.
      if ( test.state != SELFSTART )
      {
	dprintf("starting program execution.\n");

	// Reset Error callback
	//bpatch->registerErrorCallback(errorFunc);

	// Test poll for status change
	while (!appThread->isTerminated()) {
	  appThread->continueExecution();
	  bpatch->waitForStatusChange();
	}

	int retVal;
	if(appThread->terminationStatus() == ExitedNormally) {
	  int exitCode = appThread->getExitCode();
	  if (exitCode || debugPrint)
	    getOutput()->log(LOGINFO, "Mutatee exit code 0x%x\n", exitCode);
	  retVal = exitCode;
	} else if(appThread->terminationStatus() == ExitedViaSignal) {
	  int signalNum = appThread->getExitSignal();
	  if (signalNum || debugPrint)
	    getOutput()->log(LOGINFO, "Mutatee exited from signal 0x%x\n", signalNum);

	  retVal = signalNum;
	}
	return retVal;
      }
    }
  }
  else if ( test.cleanup == KILL_MUTATEE )
  {
    if ( !appThread->isTerminated() )
    {
      int pid = appThread->getPid();
      appThread->terminateExecution();
      dprintf("Mutatee process %d killed.\n", pid);
    }

  }

  return result;
}

void setupSaveTheWorld(BPatch_thread *appThread, int saveTheWorld)
{
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(rs6000_ibm_aix4_1)
  /* this is only supported on sparc solaris  and linux*/
  /* this call tells the process to collect data for the 
     save the world functionality
  */	
  if(saveTheWorld){
    appThread->enableDumpPatchedImage();
  }
#endif
}

void executeSaveTheWorld(BPatch_thread *appThread, int saveTheWorld, char *pathname)
{
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(rs6000_ibm_aix4_1)
  char *dirName;
  /* this is only supported on sparc solaris and linux*/

  if( saveTheWorld ) {
    char *mutatedName = new char[strlen(pathname) + strlen("_mutated") +1];
    strcpy(mutatedName, pathname);
    strcat(mutatedName, "_mutated");
    dirName = appThread->dumpPatchedImage(mutatedName);
    delete mutatedName;
    if(dirName){
      getOutput()->log(LOGINFO, " The mutated binary is stored in: %s\n",dirName);
      delete [] dirName;
    }else{
      getOutput()->log(LOGINFO, "Error: No directory name returned\n");
    }
    //appThread->detach(false);
  }
#endif
}


void printLogTestHeader(char *name)
{
  flushOutputLog();
  flushErrorLog();
  // Test Header
  getOutput()->log(LOGINFO, "*** dyninstAPI %s...\n", name);
  flushOutputLog();
  flushErrorLog();
}

void printLogMutateeHeader(char *mutatee)
{
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

void setupGeneralTest(BPatch *bpatch)
{
  // Set Test Library flags
  setBPatch(bpatch);
  setDebugPrint(debugPrint);

  // Register a callback function that prints any error messages
  bpatch->registerErrorCallback(errorFunc);

  setErrorPrint(errorPrint);
  setExpectErrors(false);

  // Force functions to be relocated
  if (forceRelocation) {
    bpatch->setForcedRelocation_NP(true);
  }
  if (delayedParse) {
    bpatch->setDelayedParsing(true);
  }

  if (mergeTramp)
    bpatch->setMergeTramp(true);
  else
    bpatch->setMergeTramp(false);

}

// The static test driver is deprecated and related code will be torn out
// soon.
#if defined(STATIC_TEST_DRIVER)
int runStaticTest(test_data_t &test, ParameterDict &param) {
  int result = -1;
  /* Find the test in the static_mutators array */
  int i;
  for(i = 0; i < static_mutators_count; i++) {
    if (strcmp(test.name, static_mutators[i].test_name) == 0) {
      break;
    }
  }
  if (i < static_mutators_count) {
    // Found the test
    int (*mutatorMAIN)(ParameterDict &) = static_mutators[i].mutator;
    result = mutatorMAIN(param);
  } else {
    getOutput()->log(STDERR, "Error finding function: %s\n", test.name);
  }
  return result;
}
#endif

#include <string>

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

std::vector< failureInfo > gFailureReport;

// Human Readable Log Editing Functions
void printCrashHumanLog(std::vector<RunGroup *> &tests, int groupnum,
			int testnum, bool useVerbose) {
  FILE *human;

  if (!strcmp(humanlog_name, "-")) {
    human = stdout;
  } else {
    human = fopen(humanlog_name, "a");
    if (NULL == human) { // "handle" the error
      human = stdout;
    }
  }

  formatStrings &outputFormat = useVerbose ? verboseStrings : compactStrings;

  getOutput()->log(HUMAN, outputFormat.testNameFormatStr.c_str(),
	  tests[groupnum]->tests[testnum]->name, tests[groupnum]->mutatee);

  if (CREATE == tests[groupnum]->useAttach) {
    getOutput()->log(HUMAN, outputFormat.createStr.c_str());
  } else {
    getOutput()->log(HUMAN, outputFormat.attachStr.c_str());
  }
  getOutput()->log(HUMAN, outputFormat.crashStr.c_str());
  failureInfo myCrash;
  myCrash.testName += tests[groupnum]->tests[testnum]->name;
  myCrash.testName += ": mutatee : ";
  myCrash.testName += tests[groupnum]->mutatee;
  myCrash.testName += " ";
  myCrash.createMode = (tests[groupnum]->useAttach == CREATE);
  myCrash.wasCrash = true;
  gFailureReport.push_back(myCrash);

  if (stdout == human) {
    fflush(human);
  } else {
    fclose(human);
  }
}

void printResultHumanLog(RunGroup *group, const char *testname,
			 test_results_t result, bool useVerbose)
{
  FILE *human;
  if (!strcmp(humanlog_name, "-")) {
    human = stdout;
  } else {
    human = fopen(humanlog_name, "a");
  }

  formatStrings &outputFormat = useVerbose ? verboseStrings : compactStrings;

  getOutput()->log(HUMAN, outputFormat.testNameFormatStr.c_str(), testname,
	  group->mutatee);
	  
  if (CREATE == group->useAttach) {
    getOutput()->log(HUMAN, outputFormat.createStr.c_str());
  } else {
    getOutput()->log(HUMAN, outputFormat.attachStr.c_str());
  }
  switch (result) {
  case PASSED:
    getOutput()->log(HUMAN, outputFormat.passStr.c_str());
    break;

  case SKIPPED:
    getOutput()->log(HUMAN, outputFormat.skipStr.c_str());
    break;

  case FAILED:
    getOutput()->log(HUMAN, outputFormat.failStr.c_str());
    failureInfo myFailure;
    myFailure.testName += testname;
    myFailure.testName += ": mutatee : ";
    myFailure.testName += group->mutatee;
    myFailure.testName += " ";
    myFailure.createMode = (group->useAttach == CREATE);
    myFailure.wasCrash = false;
    gFailureReport.push_back(myFailure);
    break;
  }
  
  if (stdout == human) {
    fflush(human);
  } else {
    fclose(human);
  } 

}

void updateResumeLog(int groupnum, int testnum, resume_stage_t stage)
{
  FILE *resume;
  resume = fopen(resumelog_name, "w");
  if ( resume == NULL ) {
    perror("Failed to update the resume log");
    exit(NOTESTS);
  }
  fprintf(resume, "%d,%d,%d\n", groupnum, testnum, (int)stage);

  fclose(resume);
}

void updateResumeLogCompleted() {
  FILE *resume;
  resume = fopen(resumelog_name, "a");
  if (NULL == resume) {
    getOutput()->log(STDERR, "Failed to update the resume log");
    return;
  }
  fprintf(resume, "+\n");
  fclose(resume);
}

int getResumeLog(std::vector<RunGroup *> &tests, bool verboseFormat) {
  FILE *resume;
  FILE *crashlog;
  int groupnum;
  int testnum;
  resume_stage_t stage;
  bool disable_rungroup = false;

  resume = fopen(resumelog_name, "r");
  if (!resume) {
    return -1;
  }

  if ((fscanf(resume, "%d,%d,%d\n", &groupnum, &testnum, &stage) != 3)
      || (groupnum >= tests.size())
      || (testnum >= tests[groupnum]->tests.size())
      || (stage > TEARDOWN)) {
    getOutput()->log(STDERR, "Unable to parse entry in the resume log\n");
    exit(NOTESTS);
  }

  // Disable all groups up to groupnum
  for (int g = 0; g < groupnum; g++) {
    for (int t = 0; t < tests[g]->tests.size(); t++) {
      tests[g]->tests[t]->disabled = true;
    }
  }

  char completed = '\0';
  if (fscanf(resume, "%c\n", &completed) != 1) {
    // This group didn't finish executing

    if (EXECUTION == stage) {
      // If the crash occurred during the EXECUTION stage, then read the
      // mutatee resume log to determine which test crashed
      char *mutatee_resumelog_name = "mutatee_resumelog";
      FILE *mutatee_resumelog = fopen(mutatee_resumelog_name, "r");
      if (mutatee_resumelog != NULL) {
	char crashed_test[128];
	int items_read;
	items_read = fscanf(mutatee_resumelog, "%127s\n", &crashed_test);
	if (items_read != 1) {
	  // TODO Handle error
	
	}
	// Now I need to scan the list of tests to find the right one to
	// disable
	for (int i = 0; i < tests[groupnum]->tests.size(); i++) {
	  if (!strcmp(tests[groupnum]->tests[i]->name, crashed_test)) {
	    testnum = i; // This is the test that crashed
	    break;
	  }
	}
	fclose(mutatee_resumelog);
      } else { // Error opening mutatee resumelog
	if (ENOENT == errno) {
	  // File doesn't exist; the crash was before the mutatee driver
	  // started the mutatee function..
	  // If the mutatee crashed before running any tests, I'm going to
	  // disable the whole run group and report the mutatee as crashing,
	  // rather than any particular test.
	  disable_rungroup = true;
	} else {
	  // TODO Handle error opening file
	}
      }
    } // Crash during EXECUTION stage

    // Mark test (groupnum, testnum) as having crashed
    crashlog = fopen(crashlog_name, "a");
    if (NULL == crashlog) {
      getOutput()->log(STDERR, "Error opening crashlog: %s\n", strerror(errno));
      exit(NOTESTS);
    }
    if (disable_rungroup) {
      // If mutatee driver crashed outside of a test, mark all tests for this
      // run group as having crashed
      for (int i = 0; i < tests[groupnum]->tests.size(); i++) {
	fprintf(crashlog, "%d,%d\n", groupnum, i);
      }
    } else {
      fprintf(crashlog, "%d,%d\n", groupnum, testnum);
    }
    fclose(crashlog);

    // FIXME This needs to be adjusted for multiple tests in a rungroup
    // Print crashed message for the listed test
    printCrashHumanLog(tests, groupnum, testnum, verboseFormat);

  } else {
    if (completed == '+') {
      // Handle a limit reached.  We need to continue from the next test
      // after the one specified by the numbers
      // I need to mark (groupnum, testnum + 1) as the next test to run...

      // Disable all tests in group #groupnum up through testnum
      for (int t = 0; t <= testnum; t++) {
	tests[groupnum]->tests[t]->disabled = true;
      }
      // If that was the last test in group #groupnum, then we need to
      // start with the next group after groupnum
      if (testnum == (tests[groupnum]->tests.size() - 1)) {
	// Disable test #testnum in group #groupnum
	// That's redundant; we do it above.  What we really want to do here I
	// think is move on to the next group and update the testnum
	//tests[groupnum]->tests[testnum]->enabled = false;
      }
    } else {
      getOutput()->log(STDERR,
	      "Unable to parse (completed) entry in the resume log: '%c'\n",
	      completed);
    }
  }
  fclose(resume);

  // Disable any crashed tests
  crashlog = fopen(crashlog_name, "r");
  if ((NULL == crashlog) && (errno != ENOENT)) {
    getOutput()->log(STDERR, "Error opening crashlog: %s\n", strerror(errno));
    exit(NOTESTS);
  }
  // FIXME Need to deal with no crashlog yet
  if (crashlog != NULL) {
    if (feof(crashlog)) {
    }
    while (!feof(crashlog)) {
      // Disable all the tests that have crashed
      // FIXME Check fscanf return code for errors
      fscanf(crashlog, "%d,%d\n", &groupnum, &testnum);
      tests[groupnum]->tests[testnum]->disabled = true;
    }
    fclose(crashlog);
  }
  return 0;
}

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
bool enableIntersectingTests(std::vector<TestInfo *> &testsv,
			     std::vector<char *> &testsn) {
  bool match_found = false;

  if (runAllTests) {
     for (size_t i = 0; i < testsv.size(); i++) {
       if (false == testsv[i]->disabled) {
	 testsv[i]->enabled = true;
	 match_found = true;
       }
     }
  } else {
    for (size_t i = 0; i < testsn.size(); i++) {
      for (size_t j = 0; j < testsv.size(); j++) {
	if ((false == testsv[j]->disabled)
	    && nameMatches(testsn[i], testsv[j]->name)) { // Change to fnmatch?
	  match_found = true;
	  testsv[j]->enabled = true;
	}
      }
    }
  }
  return match_found;
}

int executeRunGroupCustomExecutionPath(BPatch *bpatch, RunGroup *group,
				       ParameterDict &param, bool resumelog,
				       int groupnum, bool verboseFormat,
				       bool printLabels) {
  return 0;
}

int executeRunGroup(BPatch *bpatch, RunGroup *group, ParameterDict &param,
		    bool resumeLog, int groupnum, bool verboseFormat,
		    bool printLabels)
{

  BPatch_thread *appThread = NULL;
  ProcessList proc_list;
  int retval;

  test_results_t result;
  int mutatee_retval = 0;
  bool mutateeExitedViaSignal = false;

  if (group->customExecution) {
    executeRunGroupCustomExecutionPath(bpatch, group, param, resumeLog,
				       groupnum, verboseFormat, printLabels);
    // Set up runtime library if necessary
#if defined(m_abi)
    if (isMutateeMABI32(group->mutatee)) {
      if (NULL == libRTname_m_abi) {
	return -1;
      }
      setenv("DYNINSTAPI_RT_LIB", libRTname_m_abi, 1);
    } else {
      setenv("DYNINSTAPI_RT_LIB", libRTname, 1);
    }
#endif

    if (enableLogging) {
      // TODO Print mutatee header (?)
    }

    // Some general test setup.  This can probably be moved out to the caller
    // of executeRunGroup
    setupGeneralTest(bpatch);

    // Add bpatch pointer to test parameters
    ParamPtr bp_ptr(bpatch);
    param["bpatch"] = &bp_ptr;
    ParamPtr bp_appThread;
    param["appThread"] = &bp_appThread;

    // Get a list of all the tests (TestInfo objects) we're going to run in
    // RunGroup.  getMutatorsForRunGroup also creates TestMutator objects and
    // sets a pointer in the TestInfo object to point to the TestMutator for
    // each test.
    std::vector<TestInfo *> group_tests;
    if (getMutatorsForRunGroup(group, *&group_tests)) {
      // TODO Handle error getting list of test mutators
    }

    if (NULL == group->mutatee) {
      appThread = NULL;
    } else {
      if (group->state != SELFSTART) {
	// If test requires mutatee, start it up for the test
	// The mutatee doesn't need to print a test label for complex tests
	if (resumeLog) {
	  updateResumeLog(group->index, 0, PRESETUP);
	}
	appThread = startMutateeTest(bpatch, group, proc_list, logfilename,
				     (useHumanLog) ? humanlog_name : NULL,
				     verboseFormat, false, debugPrint,
				     getPIDFilename());
	if (resumeLog) {
	  updateResumeLogCompleted();
	}
	if (NULL == appThread) {
	  // DyninstAPI failed to launch the mutatee program; mark all tests
	  // in this group as failed
	  getOutput()->log(LOGERR, "Unable to run test program: %s\n", group->mutatee);
	  // FIXME This needs to mark the test as a failure.
	  // TODO Mark all tests in group as FAILED
	  for (unsigned int i = 0; i < group_tests.size(); i++) {
	    std::map<std::string, std::string> *attrs;
	    attrs = TestOutputDriver::getAttributesMap(group_tests[i], group);
	    getOutput()->startNewTest(*attrs);
	    delete attrs;
	    getOutput()->logResult(FAILED);
	    getOutput()->finalizeOutput();
	    if (resumeLog) {
	      // FIXME What do I send to the resume log before the completed mark?
	      updateResumeLog(group->index, group_tests[i]->index, SETUP);
	      updateResumeLogCompleted();
	    }
	    delete group_tests[i]->mutator;
	    group_tests[i]->mutator = NULL;
	    group_tests.erase(group_tests.begin() + i);
	    i -= 1;
	  }
	  return -1; // What is the significance of this return value?
	}
      }
    } // group->mutatee != NULL

    if (testLimit > 0) {
      // We want to increment the number of tests run here because we'll erase
      // tests from group_tests if they fail, so if we increment later our
      // count may be wrong.
      testsRun += group_tests.size();
    }

    // If mutatee needs to be run before the test is start, begin it
    // We check for group->mutatee == NULL because we may have a test with no
    // mutatee
    if ((group->mutatee != NULL) && (RUNNING == group->state)) {
      appThread->continueExecution();
    }

    // Setup save the world
    setupSaveTheWorld(appThread, saveTheWorld);

#if defined(PROFILE_MEM_USAGE)
    // Let's try to profile memory usage
    void *mem_usage = sbrk(0);
    getOutput()->log(LOGINFO, "pre %s: sbrk %p\n", test.soname, mem_usage);
#endif

    // Initialize each test
    bool someSetupPassed = false;
    for (int i = 0; i < group_tests.size(); i++) {
      param["pathname"]->setString(group->mutatee);
      param["appThread"]->setPtr(appThread);
      param["useAttach"]->setInt(group->useAttach);
      if (isMutateeXLC(group->mutatee)) {
	param["mutateeXLC"]->setInt(1);
      } else {
	param["mutateeXLC"]->setInt(0);
      }
      param["logfilename"]->setString(logfilename);

      // Inform output driver that we're starting a new test
      std::map<std::string, std::string> *attrs;
      attrs = TestOutputDriver::getAttributesMap(group_tests[i], group);
      getOutput()->startNewTest(*attrs);
      if (attrs != NULL) {
	delete attrs;
      }

      // Print mutator log header
      printLogOptionHeader(group_tests[i]);
      if (resumeLog) {
	updateResumeLog(group->index, group_tests[i]->index, SETUP);
      }
      if (printLabels) {
	getOutput()->log(LOGINFO, "1: %s\n", group_tests[i]->label);
      }
      result = group_tests[i]->mutator->setup(param);
      if (PASSED == result) {
	someSetupPassed = true;
      } else {
	// This test either failed or is being skipped; print out a message
	// to that effect and remove the test from the list of tests we're
	// running.
	getOutput()->logResult(result);
	delete group_tests[i]->mutator;
	group_tests[i]->mutator = NULL;
	group_tests.erase(group_tests.begin() + i);
	i -= 1;
	if (resumeLog) {
	  updateResumeLogCompleted(); // Don't mistake this failure for a crash
	}
	getOutput()->finalizeOutput();
	continue;
      }
      if (resumeLog) {
	updateResumeLogCompleted();
      }
    }

    // Up to here everything works about the same for group vs. non-group
    // mutatees

    // FIXME I think this condition is wrong.  It's just checking the result
    // of the last test in the group's setup()
    if (someSetupPassed) {
      // group_tests.size() is almost certainly 1.
      // This is not true for the 'none' mutatee.  Should that one use the
      // simple execution path instead?
      assert (group_tests.size() == 1);

      for (size_t i = 0; i < group_tests.size(); i++) {

	// Set up output driver for this test
	std::map<std::string, std::string> *attrs;
	attrs = TestOutputDriver::getAttributesMap(group_tests[i], group);
	getOutput()->startNewTest(*attrs);
	delete attrs;

	if (group_tests[i]->mutator->hasCustomExecutionPath()) {
	  if (resumeLog) {
	    updateResumeLog(group->index, group_tests[i]->index, EXECUTION);
	  }
	  result = group_tests[i]->mutator->execute();
	  getOutput()->logResult(result);
	  if (resumeLog) {
	    updateResumeLogCompleted();
	  }
	  delete group_tests[i]->mutator;
	  group_tests[i]->mutator = NULL;
	  getOutput()->finalizeOutput();
	} else { // non-group simple execution path

	  // TODO Group has customExecution flag set, but mutator doesn't
	  // These are non-group mutatees with standard execution path mutators
	  // I've got the mutator already
	  // 1. run mutator->preExecution()
	  if (resumeLog) {
	    updateResumeLog(group->index, group_tests[i]->index, PREEXECUTION);
	  }

	  result = group_tests[i]->mutator->preExecution();
	  if (resumeLog) {
	    updateResumeLogCompleted();
	  }
	  if (result != PASSED) {
	    // Deal with failure.  The mutatee will not be printing result
	    // codes
	    getOutput()->logResult(result);
	    getOutput()->finalizeOutput();
	    delete group_tests[i]->mutator;
	    group_tests[i]->mutator = NULL;
	    i -= 1;
	  } else { // preExecution passed
	    // TODO 2. run mutatee
	    dprintf("starting program execution.\n");
	    if (resumeLog) {
	      updateResumeLog(group->index, 0, EXECUTION);
	    }

	    do {
	      appThread->continueExecution();
	      bpatch->waitForStatusChange();
	    } while (!appThread->isTerminated());

	    // TODO 3. check results
	    if (appThread->terminationStatus() == ExitedNormally) {
	      int exitCode = appThread->getExitCode();
	      if (exitCode || debugPrint) {
		getOutput()->log(LOGINFO, "Mutatee exit code 0x%x\n", exitCode);
	      }
	      mutatee_retval = exitCode;
	    } else if (appThread->terminationStatus() == ExitedViaSignal) {
	      mutateeExitedViaSignal = true;
	      int signalNum = appThread->getExitSignal();
	      if (signalNum || debugPrint) {
		getOutput()->log(LOGINFO, "Mutatee exited from signal 0x%x\n", signalNum);
	      }
	      mutatee_retval = signalNum;
	    }

	    if (0 == mutatee_retval) {
	      // Test was successful
	      getOutput()->logResult(PASSED);
	    } else {
	      // Test failed
	      getOutput()->logResult(FAILED);
	    }
	    getOutput()->finalizeOutput();

	    if (resumeLog) {
	      updateResumeLogCompleted();
	    }

	    // Clean up TestInfo, mutator, etc.
	    delete group_tests[i]->mutator;
	    group_tests[i]->mutator = NULL;
	    group_tests.erase(group_tests.begin() + i);
	    i -= 1;

	  } // if (preExecution passed)
	} // if (non-group simple execution path)
      } // for (all tests in group)
    } // if (setup for all tests in group passed)

    retval = 0; // FIXME Make this the correct return value
  } else { // Standard execution path

    // ************************************************************
    // ************************************************************
    // *** BEGIN Standard execution path                        ***
    // ************************************************************
    // ************************************************************

    // Set up runtime library if necessary
#if defined(m_abi)
    if (isMutateeMABI32(group->mutatee)) {
      if (NULL == libRTname_m_abi) {
	return -1;
      }
      setenv("DYNINSTAPI_RT_LIB", libRTname_m_abi, 1);
    } else {
      setenv("DYNINSTAPI_RT_LIB", libRTname, 1);
    }
#endif

    if (enableLogging) {
      // TODO Print mutatee header (?)
    }

    // Some general test setup.  This can probably be moved out to the caller
    // of executeRunGroup
    setupGeneralTest(bpatch);

    // Add bpatch pointer to test parameters
    ParamPtr bp_ptr(bpatch);
    param["bpatch"] = &bp_ptr;
    ParamPtr bp_appThread;
    param["appThread"] = &bp_appThread;

    // Get a list of all the tests (TestInfo objects) we're going to run in
    // RunGroup.  getMutatorsForRunGroup also creates TestMutator objects and
    // sets a pointer in the TestInfo object to point to the TestMutator for
    // each test.
    // We're doing this before we start the mutatee because if we fail to start
    // the mutatee, we want to report all of these tests as FAILED
    std::vector<TestInfo *> group_tests;

    if (getMutatorsForRunGroup(group, *&group_tests)) {
      // TODO Handle error getting list of test mutators
    }

    // If test requires mutatee, start it up for the test
    if (resumeLog) {
      updateResumeLog(group->index, 0, PRESETUP);
    }
    appThread = startMutateeTest(bpatch, group, proc_list, logfilename,
				 (useHumanLog) ? humanlog_name : NULL,
				 verboseFormat, printLabels, debugPrint,
				 getPIDFilename());
    if (resumeLog) {
      updateResumeLogCompleted();
    }
    if (NULL == appThread) {
      getOutput()->log(LOGERR, "Unable to run test program: %s\n", group->mutatee);
      // FIXME This needs to mark the test as a failure.
      // TODO Mark all tests in group as FAILED
      for (unsigned int i = 0; i < group_tests.size(); i++) {
	std::map<std::string, std::string> *attrs;
	attrs = TestOutputDriver::getAttributesMap(group_tests[i], group);
	getOutput()->startNewTest(*attrs);
	delete attrs;
	getOutput()->logResult(FAILED);
	getOutput()->finalizeOutput();
	if (resumeLog) {
	  // FIXME What do I send to the resume log before the completed mark?
	  updateResumeLog(group->index, group_tests[i]->index, SETUP);
	  updateResumeLogCompleted();
	}
	delete group_tests[i]->mutator;
	group_tests[i]->mutator = NULL;
	group_tests.erase(group_tests.begin() + i);
	i -= 1;
      }
      return -1; // What is the significance of this return value?
    }

    if (testLimit > 0) {
      // Increment the number of tests we've run now, before we start pulling
      // failed tests out of group_tests
      testsRun += group_tests.size();
    }

    // If mutatee needs to be run before the test is start, begin it
    if (RUNNING == group->state) {
      appThread->continueExecution();
    }

    // Setup save the world
    setupSaveTheWorld(appThread, saveTheWorld);

    // Let's try to profile memory usage
#if defined(PROFILE_MEM_USAGE)
    void *mem_usage = sbrk(0);
    getOutput()->log(LOGINFO, "pre %s: sbrk %p\n", test.soname, mem_usage);
#endif

    TestInfo *lastLabelFrom = NULL;

    // The failed_preExecution list holds tests that failed before the mutatee
    // was run.  If all tests fail before the mutatee is run, then the test
    // driver needs to print failure messages for them
    std::vector<TestInfo *> failed_preExecution;

    // Initialize each test
    for (int i = 0; i < group_tests.size(); i++) {
      param["pathname"]->setString(group->mutatee);
      param["appThread"]->setPtr(appThread);
      param["useAttach"]->setInt(group->useAttach);
      if (isMutateeXLC(group->mutatee)) {
	param["mutateeXLC"]->setInt(1);
      } else {
	param["mutateeXLC"]->setInt(0);
      }

      // Print mutator log header
      printLogOptionHeader(group_tests[i]);
      if (resumeLog) {
	// Update resume log
	updateResumeLog(group->index, group_tests[i]->index, SETUP);
      }
      if (printLabels) {
	lastLabelFrom = group_tests[i];
	getOutput()->log(LOGINFO, "2: %s\n", group_tests[i]->label);
      }
      std::map<std::string, std::string> *attrs;
      attrs = TestOutputDriver::getAttributesMap(group_tests[i], group);
      getOutput()->startNewTest(*attrs);
      delete attrs;
      result = group_tests[i]->mutator->setup(param);
      if (result != PASSED) {
	// This test either failed or is being skipped; print out a message
	// to that effect and remove the test from the list of tests we're
	// running.
	failed_preExecution.push_back(group_tests[i]);
	group_tests.erase(group_tests.begin() + i);
	i -= 1;
	if (resumeLog) {
	  updateResumeLogCompleted(); // Don't mistake this failure for a crash
	}
	continue;
      }
      if (resumeLog) {
	// Update resume log
	updateResumeLogCompleted();
      }
      flushOutputLog();
      flushErrorLog();
    }

    // Up to here everything works about the same for group vs. non-group
    // mutatees

    bool allFailedOrSkipped = true;
    // Run preExecution for each test
    for (int i = 0; i < group_tests.size(); i++) {
      if (resumeLog) {
	updateResumeLog(group->index, group_tests[i]->index, PREEXECUTION);
      }
      std::map<std::string, std::string> *attrs;
      attrs = TestOutputDriver::getAttributesMap(group_tests[i], group);
      getOutput()->startNewTest(*attrs);
      delete attrs;
      if (printLabels && (lastLabelFrom != group_tests[i])) {
	lastLabelFrom = group_tests[i];
	getOutput()->log(LOGINFO, "3: %s\n", group_tests[i]->label);
      }
      result = group_tests[i]->mutator->preExecution();
      if (PASSED == result) {
	allFailedOrSkipped = false;
      } else {
	// preExecution failed; save this test's info in case we need to print
	// a failure message from the test driver
	failed_preExecution.push_back(group_tests[i]);
	group_tests.erase(group_tests.begin() + i);
	i -= 1;
	if (resumeLog) {
	  updateResumeLogCompleted(); // Don't mistake this failure for a crash
	}
	continue;
      }
      if (resumeLog) {
	// Update resume log
	updateResumeLogCompleted();
      }

    }

    // Run the mutatee
    if (false == allFailedOrSkipped) {
      // SELFSTART tests are, by definition, custom execution paths so I don't
      // need to worry about them here
      dprintf("starting program execution.\n");
      // TODO Modify this loop to allow for tests to do things in mid-execution
      // No, don't do that.
      do {
	if (resumeLog) {
	  updateResumeLog(group->index, 0, EXECUTION);
	  // The mutatee driver will log which test was executing when the
	  // crash occurred
	}
	appThread->continueExecution();
	bpatch->waitForStatusChange();
	if (!appThread->isTerminated()) {
	  // I think I can insert a call to TestMutator::inExecution() here..
	  // TODO Remove call to inExecution
	  for (int i = 0; i < group_tests.size(); i++) {
	    if (resumeLog) {
	      updateResumeLog(group->index, group_tests[i]->index,
			      INEXECUTION);
	    }
	    std::map<std::string, std::string> *attrs;
	    attrs = TestOutputDriver::getAttributesMap(group_tests[i], group);
	    getOutput()->startNewTest(*attrs);
	    delete attrs;
	    if (printLabels && (lastLabelFrom != group_tests[i])) {
	      lastLabelFrom = group_tests[i];
	      getOutput()->log(LOGINFO, "4: %s\n", group_tests[i]->label);
	    }
	    result = group_tests[i]->mutator->inExecution();
	    if (result != PASSED) {
	      std::map<std::string, std::string> *attrs;
	      attrs = TestOutputDriver::getAttributesMap(group_tests[i],
							 group);
	      getOutput()->startNewTest(*attrs);
	      delete attrs;
	      // The mutatee has provided us with the output to finalize
	      getOutput()->finalizeOutput();
	      delete group_tests[i]->mutator;
	      group_tests[i]->mutator = NULL;
	      group_tests.erase(group_tests.begin() + i);
	      i -= 1;
	      if (resumeLog) {
		updateResumeLogCompleted(); // Don't mistake failure for crash
	      }
	      continue;
	    }
	    if (resumeLog) {
	      // Update resume log
	      updateResumeLogCompleted();
	    }

	    flushOutputLog();
	    flushErrorLog();
	  }
	} // endif !appThread->isTerminated()
      } while (!appThread->isTerminated());

      if (resumeLog) {
	updateResumeLogCompleted(); // Completed mutatee execution
      }

      if (appThread->terminationStatus() == ExitedNormally) {
	int exitCode = appThread->getExitCode();
	if (exitCode || debugPrint)
	  getOutput()->log(LOGINFO, "Mutatee exit code 0x%x\n", exitCode);
	mutatee_retval = exitCode;
      } else if(appThread->terminationStatus() == ExitedViaSignal) {
	mutateeExitedViaSignal = true;
	int signalNum = appThread->getExitSignal();
	if (signalNum || debugPrint)
	  getOutput()->log(LOGINFO, "Mutatee exited from signal 0x%x\n", signalNum);
	mutatee_retval = signalNum;
      }

      // I think it's too late to run postExecution after the mutatee has
      // actually exited.  I think I need to use postExecution as a callback
      // for the mutatee exit event.

      // I still need to figure out what should be done if the mutatee exits
      // abnormally or with a non-zero error code..

      if (0 == mutatee_retval) {
	for (int i = 0; i < group_tests.size(); i++) {
	    // Here I'm printing the final batch of mutatee output for a
	    // non-crashing mutatee run
	  std::map<std::string, std::string> *attrs;
	  attrs = TestOutputDriver::getAttributesMap(group_tests[i], group);
	  getOutput()->startNewTest(*attrs);
	  delete attrs;
	  if (printLabels && (lastLabelFrom != group_tests[i])) {
	    lastLabelFrom = group_tests[i];
	    getOutput()->log(LOGINFO, "6: %s\n", group_tests[i]->label);
	  }
	  getOutput()->finalizeOutput();
	  delete group_tests[i]->mutator;
	  group_tests[i]->mutator = NULL;
	  group_tests.erase(group_tests.begin() + i);
	  i -= 1;

	}
      } else { // mutatee_retval != 0
	// Some test has failed.  Nothing to do here but update that we've run
	// the tests.  The mutatee driver prints the test results output.
	// FIXME If the mutatee driver exits via a signal, it most likely
	// didn't print any results.
	if (mutateeExitedViaSignal && useHumanLog) {
	  // TODO Print failed message for any tests that were left to run?
	  // Here is where I'd like to have a functioning mutatee crashlog
	  for (int i = 0; i < group_tests.size(); i++) {
	    std::map<std::string, std::string> *attrs;
	    attrs = TestOutputDriver::getAttributesMap(group_tests[i], group);
	    getOutput()->startNewTest(*attrs);
	    delete attrs;
	    getOutput()->logResult(FAILED);
	    getOutput()->finalizeOutput();
	  }
	} else {
	  // I need to finish logging tests where the mutatee didn't fail
	  for (int i = 0; i < group_tests.size(); i++) {
	    std::map<std::string, std::string> *attrs;
	    attrs = TestOutputDriver::getAttributesMap(group_tests[i], group);
	    getOutput()->startNewTest(*attrs);
	    delete attrs;
	    // This should hopefully pick up the test result from what the
	    // mutatee's written into the log
	    getOutput()->finalizeOutput();
	    delete group_tests[i]->mutator;
	    group_tests[i]->mutator = NULL;
	    group_tests.erase(group_tests.begin() + i);
	    i -= 1;
	  }
	}
	if (resumeLog) {
	  updateResumeLogCompleted();
	}
      }

      // Delete the list of tests that failed before we executed the mutatee
      //for (unsigned int i = 0; i < failed_preExecution.size(); i++) {
      while (failed_preExecution.size() > 0) {
	delete failed_preExecution[0]->mutator;
	failed_preExecution[0]->mutator = NULL;
	failed_preExecution.erase(failed_preExecution.begin());
      }
    } else {
      // All tests failed or skipped in preExecution(), so I didn't execute the
      // mutatee.  I should kill it so it doesn't run in attach mode
      if (appThread != NULL) {
	appThread->getProcess()->terminateExecution();
      }
      // FIXME I also need to print failure messages for all the tests that I
      // would have run if they hadn't already failed so I skipped running the
      // mutatee
      // FIXME Don't use group_tests here; it's empty
      for (int i = 0; i < failed_preExecution.size(); i++) {
	std::map<std::string, std::string> *attrs;
	attrs = TestOutputDriver::getAttributesMap(failed_preExecution[i],
						   group);
	getOutput()->startNewTest(*attrs);
	delete attrs;
	getOutput()->logResult(FAILED);
	getOutput()->finalizeOutput();
	delete failed_preExecution[i]->mutator;
	failed_preExecution[i]->mutator = NULL;
	failed_preExecution.erase(failed_preExecution.begin() + i);
	i -= 1;
      }
    } // if !(false == allTestsFailedOrSkipped)
    if (appThread != NULL) {
      delete appThread;
      appThread = NULL;
    }

    flushOutputLog();
    flushErrorLog();

    // FIXME I think I can remove the stuff below..  I need to make sure about
    // the mutatee exit code stuff first.

    if (enableLogging && (mutatee_retval != 0)) {
      int pos_result = mutatee_retval;
      if ( mutatee_retval < 0 )
	{
          pos_result = -mutatee_retval;
	}
      getOutput()->log(LOGINFO, "=========================================================\n");
      getOutput()->log(LOGINFO, "=== Exit code 0x%02x: %s\n", pos_result, group->mutatee);
      getOutput()->log(LOGINFO, "=========================================================\n");
      flushOutputLog();
      flushErrorLog();
    }
    // This is the string that the summary script uses to tell if a script passed
    // or not.  Some mutatees currently print this, so in some cases this will
    // cause a redundant print.
    if ( enableLogging && (result != FAILED) && (mutatee_retval == 0) ) {
      flushOutputLog();
      flushErrorLog();
      getOutput()->log(LOGINFO, "All tests passed\n");
      flushOutputLog();
      flushErrorLog();
    }
    
    retval = mutatee_retval;
  } // Standard execution path

  return retval; // FIXME (?) Return the correct value
}

int startTest(std::vector<RunGroup *> &tests,
	      std::vector<char *> mutatee_list, bool verboseFormat,
	      bool printLabels)
{

    BPatch *bpatch;
    // Create an instance of the BPatch library
    bpatch = new BPatch;

    // Begin setting up test parameters
    //ParameterDict param(pdstring::hash);
    ParameterDict param;
    // Setup parameters
    ParamInt attach(0);
    ParamInt errorp(errorPrint);
    ParamInt debugp(debugPrint);
    ParamInt merget(mergeTramp);
    ParamInt mutXLC(0);
    ParamString pathname;
    ParamString logfile_p;

    param["pathname"] = &pathname;
    param["useAttach"] = &attach;
    param["errorPrint"] = &errorp;
    param["debugPrint"] = &debugp;
    param["mutateeXLC"] = &mutXLC;
    param["mergeTramp"] = &merget;
    param["logfilename"] = &logfile_p;

    // Print Test Log Header
    // if ( enableLogging && skipToTest == 0 && skipToMutatee == 0 && skipToOption == 0 ) {
    // TODO Figure out a way to print the header only on new runs of the test
    // suite
    getOutput()->log(LOGINFO, "Commencing DyninstAPI test(s) ...\n");
#if !defined(os_windows)
    if ( pdscrdir )
      {
	runScript("date");
	runScript("uname -a");
      }
#endif
    getOutput()->log(LOGINFO, "TESTDIR=%s\n", getenv("PWD"));
    // }

    // Build list of run groups that we're going to execute.  It should include
    // only those run groups that involve a mutatee in mutatee_list and include
    // a test in test_list.  It should also include all of those run groups.
    std::vector<RunGroup *> to_run;
    for (size_t i = 0; i < tests.size(); i++) {
      if ((BOTH == tests[i]->useAttach)
	  || runAllOptions
	  || ((CREATE == tests[i]->useAttach) && runCreate)
	  || ((USEATTACH == tests[i]->useAttach) && runAttach)) {
	// If the RunGroup tests[i] uses a mutatee in mutatee_list..
	if (runAllMutatees
	    || mutateeListContains(mutatee_list, tests[i]->mutatee)) {
	  if (enableIntersectingTests(tests[i]->tests, test_list)) {
	    to_run.push_back(tests[i]);
	  }
	}
      }
    }

    if (to_run.size() == 0) {
      // Figure out why and print an error message, then return to main()
      if (test_list.size() > 0) { // Tests specified on command line
	for (size_t tl_ndx = 0; tl_ndx < test_list.size(); tl_ndx++) {
	  size_t rg_ndx;
	  for (rg_ndx = 0; rg_ndx < tests.size(); rg_ndx++) {
	    for (size_t rgt_ndx = 0; rgt_ndx < tests[rg_ndx]->tests.size();
		 rgt_ndx++) {
	      if (nameMatches(test_list[tl_ndx],
			      tests[rg_ndx]->tests[rgt_ndx]->name)) {
		// found a match, skip this rungroup
		goto sT_t_escape;
	      }
	    }
	  }
	sT_t_escape:
	  if (rg_ndx < tests.size()) {
	    // Match found; continue to next test
	    continue;
	  } else {
	    // No match found; print error message
	    if ((strcmp(logfilename, "NUL") == 0)
		|| (strcmp(logfilename, "/dev/null") == 0)) {
	      // We still want to print this message if logging is off
	      getOutput()->log(STDERR, "Error: test '%s' does not exist or is not enabled for this platform\n", test_list[tl_ndx]);
	    } else {
	      getOutput()->log(LOGERR, "Error: test '%s' does not exist or is not enabled for this platform\n", test_list[tl_ndx]);
	    }
	  }
	}
      }
      if (mutatee_list.size() > 0) { // Mutatees specified on command line
	for (size_t ml_ndx = 0; ml_ndx < mutatee_list.size(); ml_ndx++) {
	  // TODO* Do these mutatees exist?
	  size_t rg_ndx;
	  for (rg_ndx = 0; rg_ndx < tests.size(); rg_ndx++) {
	    if (nameMatches(mutatee_list[ml_ndx], tests[rg_ndx]->mutatee)) {
	      break;
	    }
	  }
	  if (rg_ndx < tests.size()) {
	    // Match found; continue to next mutatee
	    continue;
	  } else {
	    // No match found for this mutatee; print error message
	    if ((strcmp(logfilename, "NUL") == 0)
		|| (strcmp(logfilename, "/dev/null") == 0)) {
	      getOutput()->log(STDERR, "Error: mutatee '%s' does not exist or is not enabled for this platform\n", mutatee_list[ml_ndx]);
	    } else {
	      getOutput()->log(LOGERR, "Error: mutatee '%s' does not exist or is not enabled for this platform\n", mutatee_list[ml_ndx]);
	    }
	  }
	}
      }
      return -NOTESTS; // startTest() failed, ran no tests
    } // End no tests to run

    // Now run each of the run groups in to_run
    for (size_t i = 0; i < to_run.size(); i++) {
      if (enableLogging) {
	// Print mutatee (run group) header
	printLogMutateeHeader(to_run[i]->mutatee);
      }
      // TODO Notice the return code from executeRunGroup and do something
      // useful with it
      executeRunGroup(bpatch, to_run[i], param, resumeLog, i, verboseFormat,
		      printLabels);

      if ((testLimit > 0) && (testsRun >= testLimit)) {
	return 0; // FIXME Is this the right return value?
      }

    }
    
    return 0;
} // startTest()

void DebugPause() {
  getOutput()->log(STDERR, "Waiting for attach by debugger\n");
#if defined(i386_unknown_nt4_0)
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
  unsigned int i;
  bool shouldDebugBreak = false;
  bool called_from_runTests = false;
  std::vector<char *> mutatee_list;
  bool quietFormat = false;
  bool printLabels = false;

  updateSearchPaths(argv[0]);

  setOutput(new StdOutputDriver(NULL));

  for (i=1; i < argc; i++ )
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
	enableLogging = true;
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

	if (!logfile_found) {
	  getOutput()->log(STDERR, "Missing log file name; defaulting to standard output\n");
	  logfilename = "-";
	}
      }
    else if ( strcmp(argv[i], "-logfile") == 0) {
      getOutput()->log(STDERR, "WARNING: -logfile is a deprecated option; use -log instead\n");
      /* Store the log file name */
      if ((i + 1) >= argc) {
	getOutput()->log(STDERR, "Missing log file name\n");
	exit(NOTESTS);
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
	    exit(NOTESTS);
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
	    exit(NOTESTS);
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
    else if ( strcmp(argv[i], "-enable-resume") == 0 ) {
      resumeLog = true;
    } else if ( strcmp(argv[i], "-use-resume") == 0 ) {
      useResume = true;
    } else if ( strcmp(argv[i], "-limit") == 0 ) {
      if ( i + 1 >= argc ) {
	getOutput()->log(STDERR, "-limit must be followed by an integer limit\n");
	exit(NOTESTS);
      }
      testLimit = strtol(argv[++i], NULL, 10);
      if ((0 == testLimit) && (EINVAL == errno)) {
	getOutput()->log(STDERR, "-limit must be followed by an integer limit\n");
	exit(NOTESTS);
      }
    }
    else if ( strcmp(argv[i], "-humanlog") == 0 ) {
      // Verify that the following argument exists
      if ( i + 1 >= argc )
	{
	  getOutput()->log(STDERR, "-humanlog must by followed by a filename\n");
	  exit(NOTESTS);
	}

      useHumanLog = true;
      humanlog_name = argv[++i];
    } else if (strcmp(argv[i], "-under-runtests") == 0) {
      called_from_runTests = true;
    }
    else if ( strcmp(argv[i], "-help") == 0) {
      getOutput()->log(STDOUT, "Usage: %s [-skipTo <test_num>] [-humanlog filename] [-verbose]\n", argv[0]);
      getOutput()->log(STDOUT, "       [-log] [-test <name> ...]\n", argv[0]);
      exit(SUCCESS);
    }
    else if (strcmp(argv[i], "-fast") == 0) {
      fastAndLoose = true;
    }
    else if (strcmp(argv[i], "-print-labels") == 0) {
      printLabels = true;
    }
    else if (strcmp(argv[i], "-pidfile") == 0) {
      char *pidFilename = NULL;
      if (i + 1 >= argc) {
	getOutput()->log(STDERR, "-pidfile must be followed by a filename\n");
	exit(NOTESTS);
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
	exit(NOTESTS);
      }
  }

  // Set up mutatees
  std::vector<RunGroup *> tests;
  initialize_mutatees(*&tests);  // Fill in tests vector with lists of tests
                                 // to run

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
      exit(NOTESTS);
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
  // I don't think I need to redirect stdout and stderr

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

  // Set the initial test to the value in the resume log
  if ( useResume ) {
    getResumeLog(*&tests, !quietFormat);
  }

  if ( getenv("DYNINSTAPI_RT_LIB") )
  {
    char *temp = getenv("DYNINSTAPI_RT_LIB");
    int len = strlen(temp);
    libRTname = (char *) malloc(len + 1);
    strncpy(libRTname, temp, len);
    libRTname[len] = '\0';
  } else {
    getOutput()->log(STDERR, "Environment variable DYNINSTAPI_RT_LIB undefined:\n"
#if defined(i386_unknown_nt4_0)
	    "    using standard search strategy for libdyninstAPI_RT.dll\n");
#else
            "    set it to the full pathname of libdyninstAPI_RT\n");
    exit(-1);
#endif
  }

#if defined(m_abi)
  if ( getenv("DYNINSTAPI_RT_LIB_MABI") ) {
    char *temp = getenv("DYNINSTAPI_RT_LIB_MABI");
    int len = strlen(temp);
    libRTname_m_abi = (char *) malloc(len+1);
    strncpy(libRTname_m_abi, temp, len);
    libRTname_m_abi[len] = '\0';
  } else {
    getOutput()->log(STDERR, "Warning: Environment variable DYNINSTAPI_RT_LIB_MABI undefined:\n"
	    "32 bit mutatees will not run\n");
  }
#endif

  int return_code;
  return_code = -startTest(*&tests, mutatee_list, !quietFormat, printLabels);

  if ((outlog != NULL) && (outlog != stdout)) {
    fclose(outlog);
  }
  if(quietFormat) {
    if(!gFailureReport.empty()) {
      fprintf(stdout, "\nFAILURES:\n");
  
      for(int i = 0; i < gFailureReport.size(); i++) {
	fprintf(stdout, "Failure: %s, create mode: %s, %s\n", gFailureReport[i].testName.c_str(), gFailureReport[i].createMode ? "create" : "attach",
		gFailureReport[i].wasCrash ? "crashed" : "failed");
      }
    }
  
  }
  fflush(stdout);

  return return_code;
}

