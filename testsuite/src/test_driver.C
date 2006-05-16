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

// $Id: test_driver.C,v 1.31 2006/05/16 14:46:20 bpellin Exp $
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

// Globals, should be eventually set by commandline options
bool forceRelocation = false; // force relocation of functions
bool delayedParse = false;
bool enableLogging = false;
bool runAllTests = true;
bool runAllMutatees = true;
bool runAllOptions = true;
bool runCreate = false;
bool runAttach = false;
bool resumeLog = false;
bool useResume = false;
bool useHumanLog = false;
int skipToTest = 0;
int skipToMutatee = 0;
int skipToOption = 0;
int testLimit = 0;

char *resumelog_name = "resumelog";
char *humanlog_name = NULL;

char *pdscrdir = NULL;
char *uw_pdscrdir = "/p/paradyn/builds/scripts";
char *umd_pdscrdir = "/fs/dyninst/dyninst/current/scripts";

char *libRTname;
#if defined(m_abi)
char *libRTname_m_abi;
#endif

int saveTheWorld = 0;
int mergeTramp = 0;
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

int runScript(const char *name, ...)
{
   char test[1024];
   int result;
   va_list ap;
   va_start(ap, name);
   vsnprintf(test, 1024, name, ap);
   va_end(ap);

   // Flush before/after script run
   fflush(stdout);
   fflush(stderr);
   result = system(test);
   fflush(stdout);
   fflush(stderr);

   return result;
}

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
         do {
             appThread->continueExecution();
             bpatch->waitForStatusChange();
         } while (!appThread->isTerminated());

         int retVal;
         if(appThread->terminationStatus() == ExitedNormally) {
            int exitCode = appThread->getExitCode();
            if (exitCode || debugPrint)
               printf("Mutatee exit code 0x%x\n", exitCode);
            retVal = exitCode;
         } else if(appThread->terminationStatus() == ExitedViaSignal) {
            int signalNum = appThread->getExitSignal();
            if (signalNum || debugPrint)
               printf("Mutatee exited from signal 0x%x\n", signalNum);

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
			printf(" The mutated binary is stored in: %s\n",dirName);
			delete [] dirName;
		}else{
			printf("Error: No directory name returned\n");
		}
		//appThread->detach(false);
	}
#endif
}


void printLogTestHeader(char *name)
{
   fflush(stdout);
   fflush(stderr);
   // Test Header
   printf("*** dyninstAPI %s...\n", name);
   fflush(stdout);
   fflush(stderr);
}

void printLogMutateeHeader(char *mutatee)
{
   fflush(stdout);
   fflush(stderr);
   // Mutatee Description
   // Mutatee Info
   if ( mutatee != "" )
   {
      printf("[Tests with %s]\n", mutatee);
      if ( pdscrdir )
      {
         runScript("ls -lLF %s", mutatee);
         runScript("%s/ldd_PD %s", pdscrdir, mutatee);
      }
   }
   else
   {
      printf("[Tests with none]\n");
   }


   printf("\n");
   fflush(stdout);
   fflush(stderr);
}
   
void printLogOptionHeader(test_data_t &test, char *mutatee, bool useAttach)
{
   fflush(stdout);
   fflush(stderr);
   // Full test description
   printf("\"%s", test.name);
   if ( mutatee != "" )
   {
      printf(" -mutatee %s", mutatee);
   }
   else
   {
      printf(" -mutatee none");
   }
   if ( useAttach == USEATTACH )
   {
      printf(" -attach");
   }
   printf("\"\n");
   fflush(stdout);
   fflush(stderr);
}

void printHumanTestHeader(test_data_t &test, char *mutatee, bool useAttach)
{
   fflush(stdout);
   fflush(stderr);
   // Test Header
   printf("Running: %s", test.name);
   if ( mutatee != "" )
   {
      printf(" with mutatee: %s", mutatee);
   }
   if ( useAttach )
   {
      printf(" in attach mode");
   }
   printf(".\n");


   fflush(stdout);
   fflush(stderr);
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

}

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
      appThread = startMutateeTest(bpatch, mutatee, test.subtest, useAttach, proc_list);
      if (appThread == NULL) {

  	fprintf(stderr, "Unable to run test program: %s\n", mutatee);
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
   fprintf(stderr, "pre %s: sbrk %p\n", test.soname, mem_usage);
#endif

   // Run the test
   // result will hold the value of mutator success
   int result = loadLibRunTest(test, param);
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
       fflush(stdout);
       fflush(stderr);
       printf("=========================================================\n");
       printf("=== Exit code 0x%02x: %s\n", pos_result, test.name);
       printf("=========================================================\n");
       fflush(stdout);
       fflush(stderr);
   }
   // This is the string that the summary script uses to tell if a script passed
   // or not.  Some mutatees currently print this, so in some cases this will
   // cause a redundant print.
   if ( enableLogging && result == 0 )
   {
       fflush(stdout);
       fflush(stderr);
       printf("All tests passed\n");
       fflush(stdout);
       fflush(stderr);
   }

   //delete bpatch;
   return result;
}

bool inTestList(test_data_t &test, std::vector<char *> &test_list)
{
   for (unsigned int i = 0; i < test_list.size(); i++ )
   {
#if defined(i386_unknown_nt4_0)
      if ( strcmp(test_list[i], test.name) == 0 )
#else
      if ( fnmatch(test_list[i], test.name, 0) == 0 )
#endif
      {
         return true;
      }
   }

   return false;
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
void printCrashHumanLog(test_data_t test, char *mutatee, create_mode_t cm, off_t *init_pos)
{
   FILE *human;

   struct stat file_info;

   if ( stat(humanlog_name, &file_info) != 0 )
   {
      // File doesn't exist, so it has length 0
      if ( errno == ENOENT )
      {
         *init_pos = 0;
      } else {
         fprintf(stderr, "stat failed\n");
         exit(1);
      }
   } else {
       // Set inital position to the current file size
      *init_pos = file_info.st_size;
   }
   
   human = fopen(humanlog_name, "a");

   fprintf(human, "%s: mutatee: %s create_mode: ", test.name, mutatee);
   if ( cm == CREATE )
      fprintf(human, "create\tresult: ");
   else
      fprintf(human, "attach\tresult: ");

   fprintf(human, "CRASHED\n");
   fclose(human);

}

void printResultHumanLog(test_data_t test, char *mutatee, create_mode_t cm, int result, off_t init_pos)
{
   FILE *human;
   human = fopen(humanlog_name, "r+");
   // Backup stream to overwrite crash result
   if ( fseek(human, (long)init_pos, SEEK_SET) != 0 )
   {
      fprintf(stderr, "fseek failed\n");
	  exit(1);
   }
   fprintf(human, "%s: mutatee: %s create_mode: ", test.name, mutatee);
   if ( cm == CREATE )
      fprintf(human, "create\tresult: ");
   else
      fprintf(human, "attach\tresult: ");

   if ( result == 0 )
   {
      fprintf(human, "PASSED\n");
   } 
   else
   {
      fprintf(human, "FAILED\n");
   }

   off_t eof = ftell(human);
   
   fclose(human);

   // This code truncates using methods specific to Windows, and 
   // POSIX platforms
#if defined(i386_unknown_nt4_0)
   SECURITY_ATTRIBUTES sa;
   memset(&sa, 0, sizeof(sa));
   HANDLE h_file = CreateFile(humanlog_name, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
   SetFilePointer(h_file, eof, NULL, FILE_BEGIN);
   SetEndOfFile(h_file);
   CloseHandle(h_file);
#else
   truncate(humanlog_name, eof);
#endif
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

int getResumeLog(int *testnum, int *mutateenum, int *optionnum)
{
   FILE *resume;
   resume = fopen(resumelog_name, "r");
   if (!resume) {
	   return -1;
   }
   if ( fscanf(resume, "%d,%d,%d", testnum, mutateenum, optionnum) != 3 )
   {
      fprintf(stderr, "Unable to parse entry in the resume log\n");
      exit(NOTESTS);
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

int startTest(test_data_t tests[], unsigned int n_tests, std::vector<char *> &test_list, std::vector<char *> mutatee_list)
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
    
    param["pathname"] = &pathname;
    param["useAttach"] = &attach;
    param["errorPrint"] = &errorp;
    param["debugPrint"] = &debugp;
    param["mutateeXLC"] = &mutXLC;
    param["mergeTramp"] = &merget;

   // Print Test Log Header
   if ( enableLogging && skipToTest == 0 && skipToMutatee == 0 && skipToOption == 0 ) {
      printf("Commencing DyninstAPI test(s) ...\n");
      if ( pdscrdir )
      {
         runScript("date");
         runScript("uname -a");
      }
      printf("TESTDIR=%s\n", getenv("PWD"));
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
            
            off_t replay_position = 0;
            if ( useHumanLog )
            {
               // In case the follow test causes a program crash we 
               // premptively print a crash error in the human log.
               // If it does not crash, we'll use the replay position
               // to overwrite that entry with a correct one.
               printCrashHumanLog(tests[i], tests[i].mutatee[j], cm, &replay_position);
            }
            
            // Execute the test
            result = executeTest(bpatch, tests[i], tests[i].mutatee[j], cm, param);
            if ( useHumanLog )
            {
               printResultHumanLog(tests[i], tests[i].mutatee[j], cm, result, replay_position);
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
   
   delete bpatch;
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
    std::vector<char *> test_list;
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
       getResumeLog(&skipToTest,&skipToMutatee,&skipToOption);
    }

    if ( getenv("DYNINSTAPI_RT_LIB") ) 
    {
        char *temp = getenv("DYNINSTAPI_RT_LIB");
        int len = strlen(temp);
        libRTname = (char *) malloc(len+1);
        strncpy(libRTname, temp, len);
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
    } else
    {
       fprintf(stderr,"Warning: Environment variable DYNINSTAPI_RT_LIB_MABI undefined:\n"
                   "32 mutatees will not run\n");
    }
#endif

    return -startTest(tests, num_tests, test_list, mutatee_list);
}

