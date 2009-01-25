#ifdef os_windows_test
//needed for Sleep
#include <windows.h>
#define sleep(x) Sleep(x * 1000)
#define unlink _unlink
#endif

#include <stdlib.h>
#include <sstream>
#include <string>
#include <iostream>
#include <stdio.h>
#include <vector>

#include <string.h>
#include <errno.h>
#include <signal.h>

#include "runTests-utils.h"
#include "error.h"
#include "help.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>

// To prevent run away never run test_driver more the MAX_ITER times
#define MAX_ITER 1000

// Default name for the resume log file
#define DEFAULT_RESUMELOG "bresumelog"
// Default name for the crash log file
#define DEFAULT_CRASHLOG "crashlog"

#define MEMCPU_DEFAULT_LOG "memcpu_tmp.log";
const char *memcpu_name = NULL;
const char *memcpu_orig_name = NULL;

bool staticTests = false;
bool useLog = false;
string logfile;
string pdscrdir;

// Run No more than testLimit tests before re-exec'ing test_driver
int testLimit = 10;

vector<char *> child_argv;

void parseMEMCPUFile()
{
   if (!memcpu_name || !memcpu_orig_name)
      return;

   signed long mem_total = 0, utime_total = 0, stime_total = 0;
   FILE *f = fopen(memcpu_name, "r");
   if (!f)
      return;

   for (;;)
   {
      signed long mem, utime, stime;
      int res = fscanf(f, "mem=%ld\tutime=%ld\tstime=%ld\n",
                       &mem, &utime, &stime);
      if (res != 3)
         break;
      mem_total += mem;
      utime_total += utime;
      stime_total += stime;
   }
   fclose(f);
   unlink(memcpu_name);

   if (strcmp(memcpu_orig_name, "-") == 0)
   {
      f = stdout;
   }
   else {
      f = fopen(memcpu_orig_name, "w");
      if (!f)
         return;
   }
   
   fprintf(f, "mem=%ld\tutime=%ld\tstime=%ld\n",
           mem_total, utime_total, stime_total);
   if (f != stdout)
      fclose(f);
}

// isRegFile:
// Returns true if filename is a regular file
bool isRegFile(const string& filename)
{
   struct stat results;

   if ( stat(filename.c_str(), &results) !=  0 )
   {
      return false;
   }

   return S_ISREG(results.st_mode);
}

// isDir:
// Returns true if filename is a directory
bool isDir(const string& filename)
{
   struct stat results;

   if ( stat(filename.c_str(), &results) !=  0 )
   {
      return false;
   }

   return S_ISDIR(results.st_mode);
}

// getInput:
// Run filename and store stdout in the output string.
void getInput(const char *filename, string& output)
{
   FILE *text;
   char *result;
   result = (char *) calloc(1024,sizeof(char));
   text = popen(filename, "r");
   // Wait for program to terminate
   fscanf(text, "%s\n", result);
   pclose(text);

   output = result;

   free(result);
}

// parseParameters:
// Interpret arguments, and collect uninterpretted arguments to be passed
// to test_driver.
void parseParameters(int argc, char *argv[])
{
#if defined(STATIC_TEST_DRIVER)
   staticTests = true;
#endif

   for ( int i = 1; i < argc; i++ )
   {
      if (strncmp(argv[i], "-limit", 6) == 0) {
         if (i == (argc-1)) {
            fprintf(stderr, "Error: -limit requires a parameter\n");
         }
         else {
            unsigned int limit = atoi(argv[i+1]);
            testLimit = limit;
            i++;
         }
      } else if ((strcmp(argv[i], "-help") == 0) ||
                 (strcmp(argv[i], "--help") == 0)) {
         print_help();
         exit(0);
      } else if (strcmp(argv[i], "-static") == 0) {
         staticTests = true;
      } else if (strcmp(argv[i], "-dynamic") == 0) {
         staticTests = false;
      }
      else if ((strcmp(argv[i], "-memcpu") == 0) ||
               (strcmp(argv[i], "-cpumem") == 0))
      {
         memcpu_name = MEMCPU_DEFAULT_LOG;
         
         if ((i+1 < argc) &&
             (argv[i+1][0] != '-' || argv[i+1][1] == '\0'))
         {
            i++;
            memcpu_orig_name = argv[i];
         }
         else
         {
            memcpu_orig_name = "-";
         }
      }
      else
      {
         // Pass uninterpreted arguments to test_driver
         child_argv.push_back(argv[i]);
      }
   }
}

// ReplaceAllWith:
// Replaces all occurances of 'replace' in string 'in' with the text in string
// 'with', the result is returned.
string ReplaceAllWith(const string &in, const string &replace, const string &with)
{
   string cur;
   
   string::size_type match_pos = in.find(replace);
   string::size_type r_len = replace.length();
   if ( match_pos == string::npos )
   {
      return in;
   } 
   else
   {
      return in.substr(0,match_pos) + with + 
         ReplaceAllWith(in.substr(match_pos + r_len, in.length()), replace, with);
   }
   
   
}

int main(int argc, char *argv[])
{
   parseParameters(argc, argv);
   setupVars(useLog, logfile);

   setLibPath();

   int result = 0;
   int invocation = 0;

   // Remove a stale resumelog, if it exists
   if ( getenv("RESUMELOG") && isRegFile(string(getenv("RESUMELOG"))) )
   {
	   if(unlink(getenv("RESUMELOG")) == -1) {
		   fprintf(stderr, "Couldn't delete resume log: %s\n", getenv("RESUMELOG"));
	   }
	   else {
		   fprintf(stderr, "Cleaned up resume log OK: %s\n", getenv("RESUMELOG"));
	   }
   } else if (isRegFile(string(DEFAULT_RESUMELOG))) {
	   if(unlink(DEFAULT_RESUMELOG) == -1) {
		   fprintf(stderr, "Couldn't delete resume log: %s\n", DEFAULT_RESUMELOG);
	   }
	   else {
		   fprintf(stderr, "Cleaned up resume log OK: %s\n", DEFAULT_RESUMELOG);
	   }
   }

   // Remove a stale crashlog, if it exists
   if (getenv("CRASHLOG") && isRegFile(string(getenv("CRASHLOG")))) {
	   if(unlink(getenv("CRASHLOG")) == -1) {
		   fprintf(stderr, "Couldn't delete crash log: %s\n", getenv("CRASHLOG"));
	   };
      unlink(getenv("CRASHLOG"));
   } else if (isRegFile(string(DEFAULT_CRASHLOG))) {
	   if(unlink(DEFAULT_CRASHLOG) == -1) {
		   fprintf(stderr, "Couldn't delete crash log: %s\n", DEFAULT_CRASHLOG);
	   };
   }

   // Create a PIDs file, to track mutatee PIDs
   char *pidFilename = new char[80];
   initPIDFilename(pidFilename, 80);
   // result == 2 indicates that there are no more tests to run
   while ( result != NOTESTS && invocation < MAX_ITER )
   {
      result = RunTest(invocation, useLog, staticTests, logfile, testLimit,
                       child_argv, pidFilename, memcpu_name);
      invocation++;
      // I want to kill any remaining mutatees now, to clean up.  I should also
      // set a timer in RunTest in case something goes weird with test_driver.
      // (I think we'd be better off moving away from timer.pl)
      cleanupMutatees(pidFilename);
      if (-3 == result) {
         // User interrupted the test run; allow them a couple of seconds to do
         // it again and kill runTests
         // TODO Make sure this is portable to Windows
         fprintf(stderr, "Press ctrl-c again with-in 2 seconds to abort runTests.\n");
         sleep(2);
      }
      if (-4 == (signed char) result) {
         fprintf(stderr, "Could not execute test_driver\n");
         break;
      }
      if (-5 == (signed char) result) {
         break;
      }
   }

   // Remove the PID file, now that we're done with it
   if (pidFilename && isRegFile(string(pidFilename))) {
      unlink(pidFilename);
   }
   unlink(DEFAULT_RESUMELOG);
   unlink(getenv("RESUMELOG"));
   

   parseMEMCPUFile();
   return 0;
}
