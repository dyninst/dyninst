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

// $Id: test_driver.C,v 1.39 2006/12/19 04:51:45 rchen Exp $
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <vector>
#include <iostream>

#include <sys/stat.h>

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
bool fastAndLoose = false; // don't use a clean mutatee for each test
char *resumelog_name = "resumelog";
char *humanlog_name = "-";
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

// Include test setup data
#include "test_info.h"

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
   return isNameExt(name, "_m32", 4);
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
   fprintf(stderr, "runScript not implemented on Windows\n");
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
               logstatus("Mutatee exit code 0x%x\n", exitCode);
            retVal = exitCode;
         } else if(appThread->terminationStatus() == ExitedViaSignal) {
            int signalNum = appThread->getExitSignal();
            if (signalNum || debugPrint)
               logstatus("Mutatee exited from signal 0x%x\n", signalNum);

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
			logstatus(" The mutated binary is stored in: %s\n",dirName);
			delete [] dirName;
		}else{
			logstatus("Error: No directory name returned\n");
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
   logstatus("*** dyninstAPI %s...\n", name);
   flushOutputLog();
   flushErrorLog();
}

void printLogMutateeHeader(char *mutatee)
{
   flushOutputLog();
   flushErrorLog();
   // Mutatee Description
   // Mutatee Info
   if ( mutatee != "" )
   {
      logstatus("[Tests with %s]\n", mutatee);
#if !defined(os_windows)
      if ( pdscrdir )
      {
         runScript("ls -lLF %s", mutatee);
         runScript("%s/ldd_PD %s", pdscrdir, mutatee);
      }
#endif
   }
   else
   {
      logstatus("[Tests with none]\n");
   }


   logstatus("\n");
   flushOutputLog();
   flushErrorLog();
}
   
void printLogOptionHeader(test_data_t &test, char *mutatee, bool useAttach)
{
   flushOutputLog();
   flushErrorLog();
   // Full test description
   logstatus("\"%s", test.name);
   if ( strcmp(mutatee, "") != 0 )
   {
      logstatus(" -mutatee %s", mutatee);
   }
   else
   {
      logstatus(" -mutatee none");
   }
   if ( useAttach == USEATTACH )
   {
      logstatus(" -attach");
   }
   logstatus("\"\n");
   flushOutputLog();
   flushErrorLog();
}

void printHumanTestHeader(test_data_t &test, char *mutatee, bool useAttach)
{
   flushOutputLog();
   flushErrorLog();
   // Test Header
   logstatus("Running: %s", test.name);
   if ( strcmp(mutatee, "") != 0 )
   {
      logstatus(" with mutatee: %s", mutatee);
   }
   if ( useAttach )
   {
      logstatus(" in attach mode");
   }
   logstatus(".\n");


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
    fprintf(stderr, "Error finding function: %s\n", test.name);
  }
  return result;
}
#endif

int executeTest(BPatch *bpatch, test_data_t &test, char *mutatee, create_mode_t attachMode, ParameterDict &param)
{
   BPatch_thread *appThread = NULL;
   ProcessList proc_list;
   bool useAttach;

#if defined(m_abi)
   // Set correct runtime lib
   if ( isMutateeMABI32(mutatee) )
   {
      if ( libRTname_m_abi == NULL )
         return -1;
      setenv("DYNINSTAPI_RT_LIB", libRTname_m_abi, 1);
   }
   else
   {
      setenv("DYNINSTAPI_RT_LIB", libRTname, 1);
   }
#endif

   if ( attachMode == CREATE )
      useAttach = false;
   else
      useAttach = true;
   
   // Print Per Test Header
   if ( enableLogging )
   {
     printLogOptionHeader(test, mutatee, useAttach);
   }
   else
   {
      printHumanTestHeader(test, mutatee, useAttach);
   }
   
   /* Setup */
   setupGeneralTest(bpatch);

   // Add bpatch pointer to test parameter's
   ParamPtr bp_ptr(bpatch);
   param["bpatch"] = &bp_ptr;
   ParamPtr bp_appThread;
   param["appThread"] = &bp_appThread;


   // If test requires a mutatee start it up for the test
   if ( test.state != SELFSTART )
   {
      appThread = startMutateeTest(bpatch, mutatee, test.subtest, useAttach, proc_list, logfilename);
      if (appThread == NULL) {

  	logerror("Unable to run test program: %s\n", mutatee);
        return -1;
      }
   }

   // Set up Test Specific parameters
   param["pathname"]->setString(mutatee);
   param["appThread"]->setPtr(appThread);
   param["useAttach"]->setInt(useAttach); 

   if ( isMutateeXLC(mutatee) ) {
      param["mutateeXLC"]->setInt(1);
   } else {
      param["mutateeXLC"]->setInt(0);
   }

    // Try to pass the output and error log files to the test mutator
    ParamPtr outlog_p(outlog);
    ParamPtr errlog_p(errlog);
    param["outlog"] = &outlog_p;
    param["errlog"] = &errlog_p;
    char logfile2[256];
    strncpy(logfile2, logfilename, 256);
    param["logfilename"]->setString(logfile2);

   // If mutatee needs to be run before the test is start, begin it
   if ( test.state == RUNNING )
   {
      appThread->continueExecution();
   }

   // Setup save the world
   setupSaveTheWorld(appThread, saveTheWorld);

   // Let's try to profile memory usage
#if defined(PROFILE_MEM_USAGE)
   void *mem_usage = sbrk(0);
   logerror("pre %s: sbrk %p\n", test.soname, mem_usage);
#endif

#if defined(STATIC_TEST_DRIVER)
   // Run the test
   int result = runStaticTest(test, param);
#else
   // Run the test
   // result will hold the value of mutator success
   int result = loadLibRunTest(test, param);
#endif
   dprintf("Mutator result = %d\n", result);
   
   // Execute save the world
   executeSaveTheWorld(appThread, saveTheWorld, mutatee);
   
   // Cleanup
   // result will hold the value of the complete test's success
   result = cleanup(bpatch, appThread, test, proc_list, result);

   if ( appThread != NULL && !(test.state == SELFSTART || test.cleanup == NONE ) )
   {
     delete appThread;
   }

   // Test Exit Status Log
   if ( enableLogging && result != 0 )
   {
       int pos_result = result;
       if ( result < 0 )
       {
          pos_result = -result;
       }
       flushOutputLog();
       flushErrorLog();
       logstatus("=========================================================\n");
       logstatus("=== Exit code 0x%02x: %s\n", pos_result, test.name);
       logstatus("=========================================================\n");
       flushOutputLog();
       flushErrorLog();
   }
   // This is the string that the summary script uses to tell if a script passed
   // or not.  Some mutatees currently print this, so in some cases this will
   // cause a redundant print.
   if ( enableLogging && result == 0 )
   {
       flushOutputLog();
       flushErrorLog();
       logstatus("All tests passed\n");
       flushOutputLog();
       flushErrorLog();
   }

   //delete bpatch;
   return result;
}

bool inMutateeList(char *mutatee, std::vector<char *> &mutatee_list)
{
   for (unsigned int i = 0; i < mutatee_list.size(); i++ )
   {
#if defined(i386_unknown_nt4_0)
      if ( strcmp(mutatee_list[i], mutatee) == 0 )
#else
      if ( fnmatch(mutatee_list[i], mutatee, 0) == 0 )
#endif
      {
         return true;
      }
   }

   return false;
}


// Human Readable Log Editing Functions
// void printCrashHumanLog(test_data_t test, char *mutatee, create_mode_t cm)
// {
//    FILE *human;

//    struct stat file_info;
   
//    if (!strcmp(humanlog_name, "-")) {
//      human = stdout;
//    } else {
//      if ( stat(humanlog_name, &file_info) != 0 )
//        {
// 	 // File doesn't exist, so it has length 0
// 	 if ( errno == ENOENT )
// 	   {
// 	     *init_pos = 0;
// 	   } else {
// 	     logerror("stat failed\n");
// 	     exit(1);
// 	   }
//        } else {
// 	 // Set inital position to the current file size
// 	 *init_pos = file_info.st_size;
//        }
   
//      human = fopen(humanlog_name, "a");
//      if (NULL == human) { // "handle" the error
//        human = stdout;
//      }
//    }

//    fprintf(human, "%s: mutatee: %s create_mode: ", test.name, mutatee);
//    if ( cm == CREATE )
//       fprintf(human, "create\tresult: ");
//    else
//       fprintf(human, "attach\tresult: ");

//    fprintf(human, "CRASHED\n");
//    if ((human != NULL) && (human != stdout)) {
//      fclose(human);
//    }
// }

// Human Readable Log Editing Functions
void printCrashHumanLog(test_data_t tests[], 
			int first_test, int last_test,
			char *mutatee, create_mode_t cm) {
   FILE *human;

   struct stat file_info;
   
   if (!strcmp(humanlog_name, "-")) {
     human = stdout;
   } else {
     human = fopen(humanlog_name, "a");
     if (NULL == human) { // "handle" the error
       human = stdout;
     }
   }

   for (int i = first_test; i < last_test; i++) {
     if (!runAllTests && !inTestList(tests[i], test_list)) {
       continue;
     }
     fprintf(human, "%s: mutatee: %s create_mode: ", tests[i].name, mutatee);
     if ( cm == CREATE ) {
       fprintf(human, "create\tresult: ");
     } else {
       fprintf(human, "attach\tresult: ");
     }
     fprintf(human, "CRASHED\n");
   }
   if ((human != NULL) && (human != stdout)) {
     fclose(human);
   }
}

void printResultHumanLog(test_data_t tests[], int first_test, int last_test,
			 char *mutatee, create_mode_t cm, int result)
{
   FILE *human;
   if (!strcmp(humanlog_name, "-")) {
     human = stdout;
   } else {
     human = fopen(humanlog_name, "a");
   }
   // Backup stream to overwrite crash result
   // No longer necessary
//    if (human != stdout) {
//      if ( fseek(human, (long)init_pos, SEEK_SET) != 0 )
//        {
// 	 logerror("fseek failed\n");
// 	 exit(1);
//        }
//    }
   for (int i = first_test; i < last_test; i++) {
     if (!runAllTests && !inTestList(tests[i], test_list)) {
       continue;
     }
     fprintf(human, "%s: mutatee: %s create_mode: ", tests[i].name, mutatee);
     if ( cm == CREATE ) {
       fprintf(human, "create\tresult: ");
     } else {
       fprintf(human, "attach\tresult: ");
     }

     if ( result == 0 ) {
       fprintf(human, "PASSED\n");
     } else {
       fprintf(human, "FAILED\n");
     }
   }
   // I think all this truncate stuff is just to erase the last couple
   // characters of the word CRASHED..  I'm removing it.
//    off_t eof = ftell(human);
   
//    if ((human != NULL) && (human != stdout)) {
//      fclose(human);
//    }

//    // This code truncates using methods specific to Windows, and 
//    // POSIX platforms
//    if(strcmp(humanlog_name, "-") != 0) { // Don't do it to stdout
// #if defined(i386_unknown_nt4_0)
//      SECURITY_ATTRIBUTES sa;
//      memset(&sa, 0, sizeof(sa));
//      HANDLE h_file = CreateFile(humanlog_name, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
//      SetFilePointer(h_file, eof, NULL, FILE_BEGIN);
//      SetEndOfFile(h_file);
//      CloseHandle(h_file);
// #else
//      truncate(humanlog_name, eof);
// #endif
//    }
}

void updateResumeLog(int testnum, int mutateenum, int optionnum)
{
   FILE *resume;
   resume = fopen(resumelog_name, "w");
   if ( resume == NULL )
   {
      perror("Failed to update the resume log");
      exit(NOTESTS);
   }
   fprintf(resume, "%d,%d,%d\n", testnum,mutateenum,optionnum);

   fclose(resume);
}

void updateResumeLogFastAndLoose(int first_test, int last_test,
				 int mutateenum, int optionnum) {
  FILE *resume;
  resume = fopen(resumelog_name, "w");
  if ( resume == NULL )
    {
      perror("Failed to update the resume log");
      exit(NOTESTS);
    }
  fprintf(resume, "f%d, %d, %d, %d\n", first_test, last_test,
	  mutateenum, optionnum);
  
  fclose(resume);
}

void updateResumeLogCompleted() {
  FILE *resume;
  resume = fopen(resumelog_name, "a");
  if (NULL == resume) {
    fprintf(stderr, "Failed to update the resume log");
    return;
  }
  fprintf(resume, "+\n");
  fclose(resume);
}

int getResumeLog(int *testnum, int *mutateenum, int *optionnum)
{
   FILE *resume;
   resume = fopen(resumelog_name, "r");
   if (!resume) {
	   return -1;
   }
   if ( fscanf(resume, "%d,%d,%d\n", testnum, mutateenum, optionnum) != 3 )
   {
      fprintf(stderr, "Unable to parse entry in the resume log\n");
      exit(NOTESTS);
   }
   char completed = '\0';
   if (fscanf(resume, "%c\n", &completed) != 1) {
     // TODO Print crashed message for the listed test
     printCrashHumanLog(tests, *testnum, *testnum + 1,
			tests[*testnum].mutatee[*mutateenum],
			*optionnum ? USEATTACH : CREATE);
   } else {
     if (completed != '+') {
       fprintf(stderr,
	       "Unable to parse (completed) entry in the resume log: '%c'\n",
	       completed);
     }
   }
   fclose(resume);

   // Update number to the next test
   (*optionnum)++;
   if ( *optionnum >= 2 )
   {
      *optionnum = 0;
      (*mutateenum)++;
   }
   if ( (unsigned int) *mutateenum >= tests[*testnum].mutatee.size() )
   {
      *mutateenum = 0;
      (*testnum)++;
   }

   return 0;
}

int getResumeLogFastAndLoose(int *first_test, int *last_test,
			     int *mutateenum, int *optionnum) {
   FILE *resume;
   resume = fopen(resumelog_name, "r");
   if (!resume) {
     return -1;
   }
   if (fscanf(resume, "f %d, %d, %d, %d\n", first_test, last_test,
	      mutateenum, optionnum) != 4)
   {
      fprintf(stderr, "Unable to parse entry in the resume log\n");
      exit(NOTESTS);
   }
   char completed = '\0';
   if (fscanf(resume, "%c\n", &completed) != 1) {
     // TODO Print crashed message for the listed test
     printCrashHumanLog(tests, *first_test, *last_test,
			tests[*first_test].mutatee[*mutateenum],
			*optionnum ? USEATTACH : CREATE);
   } else {
     if (completed != '+') {
       fprintf(stderr,
	       "Unable to parse (completed) entry in the resume log: '%c'\n",
	       completed);
     }
   }
   fclose(resume);

   // Update number to the next test
   (*optionnum)++;
   if ( *optionnum >= 2 )
   {
      *optionnum = 0;
      (*mutateenum)++;
   }
   if ( (unsigned int) *mutateenum >= tests[*first_test].mutatee.size() )
   {
      *mutateenum = 0;
      *first_test = *last_test;
   }

   return 0;
}

int startTest(test_data_t tests[], unsigned int n_tests, std::vector<char *> mutatee_list)
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
   if ( enableLogging && skipToTest == 0 && skipToMutatee == 0 && skipToOption == 0 ) {
      logstatus("Commencing DyninstAPI test(s) ...\n");
#if !defined(os_windows)
      if ( pdscrdir )
      {
         runScript("date");
         runScript("uname -a");
      }
#endif
      logstatus("TESTDIR=%s\n", getenv("PWD"));
   }

   int result = SUCCESS;
   int testsRun = 0;
   // Initial loop values for resume
   int i_init = skipToTest;
   int j_init = skipToMutatee;
   int k_init = skipToOption;
   // Loop over list of tests

   unsigned count = 0;
   for (unsigned int i = i_init; i < n_tests; i++ )
   {
      // Skip test if disabled on current platform
      if ( !tests[i].enabled || !runOnThisPlatform(tests[i]) )
         continue;
      // Skip if we specified a list of tests to run and this test
      // is not in it.
      if ( !runAllTests && !inTestList(tests[i], test_list) )
         continue;

      if ( enableLogging && j_init == 0 )
      {
         printLogTestHeader(tests[i].name);
      }
      // Run test over list of mutatees
      dprintf("%s has %d mutatees.\n", tests[i].name, tests[i].mutatee.size());
      //for (unsigned int j = j_init; j < tests[i].mutatee.size(); j++ )
      for (unsigned int j = j_init; j < tests[i].mutatee.size(); j++ )
      {
         if ( !runAllMutatees && !inMutateeList(tests[i].mutatee[j], mutatee_list) )
            continue;
         if ( enableLogging && k_init == 0 )
         {
            printLogMutateeHeader(tests[i].mutatee[j]);
         }

         // Iterate through useAttach options
         for (unsigned int k = k_init; k < 2; k++ )
         {
            // Skip if this test should not be run in CREATE mode
            if ( k == 0 )
            {
               if ( !runAllOptions && !runCreate )
                  continue;
               if ( tests[i].useAttach == USEATTACH )
                  continue;
            }
            // Skip if this test should not be run in USEATTACH mode
            if ( k == 1 )
            {
               if ( !runAllOptions && !runAttach )
                  continue;
               if ( tests[i].useAttach == CREATE )
                  continue;
            }

            if ( resumeLog)
            {
               updateResumeLog(i,j,k);
            }

            create_mode_t cm;
            if ( k == 0 )
            {
               cm = CREATE;
            } else if ( k == 1 )
            {
               cm = USEATTACH;
            }
            
//             if ( useHumanLog )
//             {
//                // In case the follow test causes a program crash we 
//                // premptively print a crash error in the human log.
//                // If it does not crash, we'll use the replay position
//                // to overwrite that entry with a correct one.
// 	      if (strcmp(humanlog_name, "-") != 0) {
// 		printCrashHumanLog(tests[i], tests[i].mutatee[j], cm);
// 	      }
//             }
            
            // Execute the test
            result = executeTest(bpatch, tests[i], tests[i].mutatee[j], cm, param);
	    if (resumeLog) {
	      updateResumeLogCompleted();
	    }

            if ( useHumanLog )
            {
               printResultHumanLog(tests, i, i + 1, tests[i].mutatee[j],
				   cm, result);
            }
            // End execution if we've set a limit and reached it
            testsRun++;
            if (count == 5)
                goto END_OF_TESTS;
            if ( testLimit != 0 && testsRun >= testLimit )
            {
               // Break out of all loops
               goto END_OF_TESTS;
            }
         }
         // On next iteration begin at the start regardless of resume settings
         k_init = 0;
      }
      j_init = 0;
   }
   // Target for inner break
   END_OF_TESTS:

   dprintf("Done.\n");

   //   fprintf(stderr, "/startTests logfilename = '%s', param[\"logfilename\"] = '%s'\n",
   //	   logfilename, param["logfilename"]);
   
   // We delay this; other threads can be running at this point.
   //delete bpatch;
   if ( testsRun == 0 )
   {
      // Return special code if no tests were run
      return -NOTESTS;
   }
   else
   {
      return result;
   }
}

int startTestFastAndLoose(test_data_t tests[], unsigned int n_tests,
			  std::vector<char *> mutatee_list) {
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
   if (enableLogging && skipToTest == 0 && skipToMutatee == 0
       && skipToOption == 0 ) {
     logstatus("Commencing DyninstAPI test(s) ...\n");
#if !defined(os_windows)
     if ( pdscrdir ) {
       runScript("date");
       runScript("uname -a");
     }
#endif
     logstatus("TESTDIR=%s\n", getenv("PWD"));
   }

   int result = SUCCESS;
   // Initial loop values for resume
   int i_init = skipToTest;
   int j_init = skipToMutatee;
   int k_init = skipToOption;
   
   int tests_run = 0;

   // Loop over list of tests

   //   unsigned count = 0;
   // I need to pick a test and run all the other tests that use the same
   // mutatee..
   unsigned int first_test = skipToTest;
   unsigned int last_test = first_test;
   while (first_test < num_tests) {
     // If there are no tests to run in the range [first_test, num_tests) then
     // I need to break out of this loop.
     if (!runAllTests) {
       int i;
       for (i = first_test; i < num_tests; i++) {
	 if (inTestList(tests[i], test_list))
	   break;
       }
       if (num_tests == i) {
	 // Didn't find any tests to run
	 break; // out of the while loop
       }
     }
     // This for loop find a contiguous set of tests that we can run in a
     // group.  All the mutations will be applied to the mutatee before
     // starting it.
     if ((SOLO == tests[first_test].grouped)
	 || (SELFSTART == tests[first_test].state)) {
       // Run SELFSTART tests in their own groups
       last_test = first_test + 1;
     } else {
       for (last_test = first_test; last_test < num_tests; last_test++) {
	 if (&(tests[last_test].mutatee) != &(tests[first_test].mutatee)) {
	   // last_test uses a different set of mutatees, so it can't run in
	   // the same group as first_test
	   break;
	 }
	 if (tests[last_test].state != tests[first_test].state) {
	   // last_test uses a different state as first_test, so it can't run
	   // in the same group as first_test
	   // Is that correct?
	   break;
	 }
	 if (tests[last_test].useAttach != tests[first_test].useAttach) {
	   // Don't run last_test in the same group as first_test if they use
	   // different attach modes
	   break;
	 }
	 if (tests[last_test].cleanup != tests[first_test].cleanup) {
	   // Don't run tests with different cleanup modes in the same group;
	   // the test driver will probably crash if you do
	   break;
	 }
       }
     }
     // I need to confirm that I'm actually going to run a test in the range
     // [first_test, last_test)..
     if (!runAllTests) {
       int i;
       for (i = first_test; i < last_test; i++) {
	 if (inTestList(tests[i], test_list)) {
	   break;
	 }
       }
       if (i == last_test) {
	 // I'm not running any tests in this group; so skip the group
	 first_test = last_test;
	 continue;
       }
     }

     // Now I've found the right value for last_test
     for (unsigned int j = j_init; j < tests[first_test].mutatee.size(); j++) {
       // Run the tests in [first_test, last_test) on each mutatee
       if (!runAllMutatees
	   && !inMutateeList(tests[first_test].mutatee[j], mutatee_list)) {
	 continue;
       }
       if (enableLogging && k_init == 0) {
	 printLogMutateeHeader(tests[first_test].mutatee[j]);
       }
       char *mutatee = tests[first_test].mutatee[j];
       BPatch_thread *appThread = NULL;

       for (int k = k_init; k < 2; k++) {
	 create_mode_t cm;
	 if (0 == k) {
	   if (!runAllOptions && !runCreate) {
	     continue;
	   }
	   if (USEATTACH == tests[first_test].useAttach) {
	     continue;
	   }
	   cm = CREATE;
	 } else if (1 == k) {
	   if (!runAllOptions && !runAttach) {
	     continue;
	   }
	   if (CREATE == tests[first_test].useAttach) {
	     continue;
	   }
	   cm = USEATTACH;
	 }

	 /* Setup */
	 setupGeneralTest(bpatch);

	 // Add bpatch pointer to test parameter's
	 ParamPtr bp_ptr(bpatch);
	 param["bpatch"] = &bp_ptr;
	 ParamPtr bp_appThread;
	 param["appThread"] = &bp_appThread;

	 ProcessList proc_list;

	 // If test requires a mutatee start it up for the test
	 // TODO Figure out how to deal with this (SELFSTART)
	 if ( tests[first_test].state != SELFSTART )
	   {
	     // I need to change the invocation below to start the mutatee and
	     // tell it to run the correct set of tests..
	     appThread = startMutateeTestSet(bpatch, mutatee, tests,
					     first_test, last_test,
					     cm, proc_list, logfilename,
					     runAllTests, test_list);
	     if (appThread == NULL) {
	       logerror("Unable to run test program: %s\n", mutatee);
	       return -1;
	     }
	   }

	 // Setup save the world
	 setupSaveTheWorld(appThread, saveTheWorld);

	 if (resumeLog) {
	   updateResumeLogFastAndLoose(first_test, last_test, j, k);
	 }

	 // Apply the tests in [first_test, last_test) to the mutatee
	 for (unsigned int i = first_test; i < last_test; i++) {
	   if (!tests[i].enabled || !runOnThisPlatform(tests[i])) {
	     continue;
	   }
	   if (!runAllTests && !inTestList(tests[i], test_list)) {
	     continue;
	   }

	   // Set up Test Specific parameters
	   param["pathname"]->setString(mutatee);
	   param["appThread"]->setPtr(appThread);
	   param["useAttach"]->setInt(cm); 

	   if ( isMutateeXLC(mutatee) ) {
	     param["mutateeXLC"]->setInt(1);
	   } else {
	     param["mutateeXLC"]->setInt(0);
	   }

	   // Try to pass the output and error log files to the test mutator
	   ParamPtr outlog_p(outlog);
	   ParamPtr errlog_p(errlog);
	   param["outlog"] = &outlog_p;
	   param["errlog"] = &errlog_p;
	   param["logfilename"]->setString(logfilename);

	   if (enableLogging) {
	     printLogTestHeader(tests[i].name);
	   }
	   // TODO Excise this replay_position stuff.  We don't need it any more
	   // Done.
// 	   if (useHumanLog) {
// 	     if (strcmp(humanlog_name, "-") != 0) {
// 	       printCrashHumanLog(tests[i], tests[i].mutatee[j], cm);
// 	     }
// 	   }

	   test_data_t test = tests[i];

#if defined(m_abi)
	   // Set correct runtime lib
	   if ( isMutateeMABI32(mutatee) )
	     {
	       if ( libRTname_m_abi == NULL ) {
		 fprintf(stderr, "test_driver: exiting because libRTname_m_abi == NULL\n");
		 return -1;
	       }
	       setenv("DYNINSTAPI_RT_LIB", libRTname_m_abi, 1);
	     }
	   else
	     {
	       setenv("DYNINSTAPI_RT_LIB", libRTname, 1);
	     }
#endif

	   // Print Per Test Header
	   if ( enableLogging ) {
	     printLogOptionHeader(test, mutatee, cm);
	   } else {
	     printHumanTestHeader(test, mutatee, cm);
	   }

	   // If mutatee needs to be run before the test is start, begin it
	   // 	 if ( test.state == RUNNING ) {
	   // 	   dprintf("Running mutatee as per test.state RUNNING\n");
	   // 	   appThread->continueExecution();
	   // 	 }
   
	   // Let's try to profile memory usage
#if defined(PROFILE_MEM_USAGE)
	   void *mem_usage = sbrk(0);
	   logerror("pre %s: sbrk %p\n", test.soname, mem_usage);
#endif

	   // FIXME I need to loop through all the tests I'm running right here.
	   // Once I get to the cleanup function below, that's where it checks
	   // to see whether or not the tests worked.
#if defined(STATIC_TEST_DRIVER)
	   // Run the test
	   result = runStaticTest(test, param);
#else
	   // Run the test
	   // result will hold the value of mutator success
	   result = loadLibRunTest(test, param);
#endif
	   dprintf("Mutator result = %d\n", result);
	   
	   tests_run += 1;
	 } // for (each mutator)

	 // Execute save the world
	 executeSaveTheWorld(appThread, saveTheWorld, mutatee);
	 // Run the mutatee and collect results

	 // Cleanup
	 // result will hold the value of the complete test's success
	 result = cleanup(bpatch, appThread, tests[first_test],
			  proc_list, result);

	 // FIXME Not sure if this block below is correct
	 if (appThread != NULL
	     && !(tests[first_test].state == SELFSTART
		  || tests[first_test].cleanup == NONE ) ) {
	   delete appThread;
	 }

	 // Test Exit Status Log
	 if ( enableLogging && result != 0 )
	   {
	     int pos_result = result;
	     if ( result < 0 )
	       {
		 pos_result = -result;
	       }
	     flushOutputLog();
	     flushErrorLog();
	     logstatus("=========================================================\n");
	     logstatus("=== Exit code 0x%02x: %s\n", pos_result,
		       tests[first_test].name);
	     logstatus("=========================================================\n");
	     flushOutputLog();
	     flushErrorLog();
	   }
	 // This is the string that the summary script uses to tell if a script passed
	 // or not.  Some mutatees currently print this, so in some cases this will
	 // cause a redundant print.
	 if ( enableLogging && result == 0 )
	   {
	     flushOutputLog();
	     flushErrorLog();
	     logstatus("All tests passed\n");
	     flushOutputLog();
	     flushErrorLog();
	   }

	 if (resumeLog) {
	   // TODO I need to change updateResumeLogCompleted to take the last
	   // test run as a parameter..  Or rather, it needs to write enough
	   // information to the resume log for me to resume testing at the
	   // correct point.
	   updateResumeLogCompleted();
	 }
	 if (useHumanLog) {
	   // TODO Change the human log mechanisms to output useful information
	   // for fast & loose mode.
	   printResultHumanLog(tests, first_test, last_test,
			       tests[first_test].mutatee[j], cm, result);
	 }
	 // Since we're going for speed here, we won't check whether or not
	 // we've reached our test limit until we're out of the innermost
	 // (test) loop
	 // TODO Add a check for how many tests we've run
	 if (testLimit != 0 && (tests_run >= testLimit)) {
	   goto END_OF_TESTS_FL;
	 }

       } // for (each attach mode)
       // The next mutatee should start k at 0 rather than the resume log value
       k_init = 0;
     } // for (each mutatee)
     first_test = last_test;
     // The next time 'round should start with the first mutatee, not the
     // resumelog value (?)
     j_init = 0;
   } // while (first_test < num_tests)
 END_OF_TESTS_FL:
   // FIXME this computation doesn't say how many tests we actually ran..
   if (0 == tests_run) {
     return -NOTESTS;
   } else {
     return result;
   }
} // startTestFastAndLoose()

void DebugPause() {
    fprintf(stderr, "Waiting for attach by debugger\n");
#if defined(i386_unknown_nt4_0)
    DebugBreak();
#else
    sleep(10);
#endif
}
void setPDScriptDir()
{
   pdscrdir = getenv("PDSCRDIR");
   if ( pdscrdir == NULL )
   {
#if defined(i386_unknown_nt4_0)
#else
      // Environment variable not set, try default wisc/umd directories
      DIR *dir;
      dir = opendir(uw_pdscrdir);
      if ( dir != NULL )
      {
         closedir(dir);
         pdscrdir = uw_pdscrdir;
         return;
      }
      dir = opendir(umd_pdscrdir);
      if ( dir != NULL )
      {
         closedir(dir);
         pdscrdir = umd_pdscrdir;
         return;
      }

      fprintf(stderr, "Unabled to find paradyn script dir, please set PDSCRDIR\n");
      exit(NOTESTS);
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

int main(unsigned int argc, char *argv[]) {
    unsigned int i;
    bool shouldDebugBreak = false;
    std::vector<char *> mutatee_list;

    updateSearchPaths(argv[0]);
    
    for (i=1; i < argc; i++ )
    {
       
       if (strncmp(argv[i], "-v+", 3) == 0)
          errorPrint++;
       if (strncmp(argv[i], "-v++", 4) == 0)   
          errorPrint++;
       if (strncmp(argv[i], "-verbose", 2) == 0) 
          debugPrint = 1;
       // skip to test i+1
       else if ( strcmp(argv[i], "-skipTo")==0)
       {
          skipToTest = atoi(argv[++i]);
       }
       else if ( strcmp(argv[i], "-log")==0)
       {
          enableLogging = true;
       }
       else if ( strcmp(argv[i], "-logfile") == 0) {
	 /* Store the log file name */
	 if ((i + 1) >= argc) {
	   fprintf(stderr, "Missing log file name\n");
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
             fprintf(stderr, "-test must be followed by a testname\n");
             exit(NOTESTS);
          }

          tests = strdup(argv[++i]);

          name = strtok(tests, ",");
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
             fprintf(stderr, "-mutatee must be followed by mutatee names\n");
             exit(NOTESTS);
          }

          mutatees = strdup(argv[++i]);

          name = strtok(mutatees, ",");
          mutatee_list.push_back(name);
          while ( name != NULL )
          {
             name = strtok(NULL, ",");
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
       else if ( strcmp(argv[i], "-enable-resume") == 0 )
       {
          resumeLog = true;
       } else if ( strcmp(argv[i], "-use-resume") == 0 )
       {
          useResume = true;
       } else if ( strcmp(argv[i], "-limit") == 0 )
       {
          if ( i + 1 >= argc )
          {
            fprintf(stderr, "-limit must be followed by an integer limit\n");
            exit(NOTESTS);
          }
          testLimit = atoi(argv[++i]);
       } 
       else if ( strcmp(argv[i], "-humanlog") == 0 )
       {
          // Verify that the following argument exists
          if ( i + 1 >= argc ) 
          {
             fprintf(stderr, "-humanlog must by followed by a filename\n");
             exit(NOTESTS);
          }

          useHumanLog = true;
          humanlog_name = argv[++i];
       }
       else if ( strcmp(argv[i], "-help") == 0)
       {
          printf("Usage: %s [-skipTo <test_num>] [-humanlog filename] [-verbose]\n", argv[0]);
          printf("       [-log] [-test <name> ...]\n", argv[0]);
          exit(SUCCESS);
       } else if (strcmp(argv[i], "-fast") == 0) {
	 fastAndLoose = true;
       }
       else
       {
          printf("Unrecognized option: %s\n", argv[i]);
          exit(NOTESTS);
       }
    }

    // Set up mutatees
    initialize_mutatees();

    // Set the script dir if we require scripts
    if ( enableLogging )
    {
       setPDScriptDir();
    }

    // Set up the logging file..
    if ((logfilename != NULL) && (strcmp(logfilename, "-") != 0)) {
      outlog = fopen(logfilename, "a");
      if (NULL == outlog) {
	fprintf(stderr, "Error opening log file '%s'\n", logfilename);
	exit(NOTESTS);
      }
      errlog = outlog;
    } else {
      outlog = stdout;
      errlog = stderr;
    }
    setOutputLog(outlog);
    setErrorLog(errlog);

    // Set the resume log name
    if ( getenv("RESUMELOG") )
    {
       resumelog_name = getenv("RESUMELOG");
    }

    if ( shouldDebugBreak )
    {
        DebugPause();
    }

    // Set the initial test to the value in the resume log
    if ( useResume )
    {
      if (fastAndLoose) {
	int garbage;
	getResumeLogFastAndLoose(&skipToTest, &garbage,
				 &skipToMutatee, &skipToOption);
      } else {
	getResumeLog(&skipToTest,&skipToMutatee,&skipToOption);
      }
    }

    if ( getenv("DYNINSTAPI_RT_LIB") ) 
    {
        char *temp = getenv("DYNINSTAPI_RT_LIB");
        int len = strlen(temp);
        libRTname = (char *) malloc(len+1);
        strncpy(libRTname, temp, len);
        libRTname[len] = '\0';
    }
    else {
       fprintf(stderr,"Environment variable DYNINSTAPI_RT_LIB undefined:\n"
#if defined(i386_unknown_nt4_0)
		 "    using standard search strategy for libdyninstAPI_RT.dll\n");
#else
	         "    set it to the full pathname of libdyninstAPI_RT\n");
       exit(-1);
#endif
    }

#if defined(m_abi)
    if ( getenv("DYNINSTAPI_RT_LIB_MABI") )
    {
        char *temp = getenv("DYNINSTAPI_RT_LIB_MABI");
        int len = strlen(temp);
        libRTname_m_abi = (char *) malloc(len+1);
        strncpy(libRTname_m_abi, temp, len);
        libRTname_m_abi[len] = '\0';
    } else
    {
       fprintf(stderr,"Warning: Environment variable DYNINSTAPI_RT_LIB_MABI undefined:\n"
                   "32 mutatees will not run\n");
    }
#endif

    int return_code;
    if (fastAndLoose) {
      return_code = -startTestFastAndLoose(tests, num_tests, mutatee_list);
    } else {
      return_code = -startTest(tests, num_tests, mutatee_list);
    }

    if ((outlog != NULL) && (outlog != stdout)) {
      fclose(outlog);
    }

    return return_code;
}

