#ifdef os_windows
//needed for Sleep
#include <windows.h>
#define sleep(x) Sleep(x * 1000)
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
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>

// To prevent run away never run test_driver more the MAX_ITER times
#define MAX_ITER 1000

// Default name for the resume log file
#define DEFAULT_RESUMELOG "resumelog"
// Default name for the crash log file
#define DEFAULT_CRASHLOG "crashlog"

bool staticTests = false;
bool useLog = false;
string logfile;
string pdscrdir;

// Run No more than testLimit tests before re-exec'ing test_driver
int testLimit = 10;

vector<char *> child_argv;

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
      if ( strncmp(argv[i], "-log", 4) == 0 )
      {
         useLog = true;
         // Check to see if next line provides a logfile name
         if ((i + 1 < argc)
	     && ((argv[i+1][0] != '-' ) || (strcmp(argv[i+1], "-") == 0)))
         {
            logfile = argv[++i];
         }
      }
      else if (strncmp(argv[i], "-limit", 6) == 0) {
          if (i == (argc-1)) {
              fprintf(stderr, "Error: -limit requires a parameter\n");
          }
          else {
              unsigned int limit = atoi(argv[i+1]);
              testLimit = limit;
              i++;
          }
      } else if (strcmp(argv[i], "-help") == 0) {
	printf("Usage: runTests [-log [file]] [-limit <limit>] [-static | -dynamic] ...\n");
	printf("\t-log: enable logging, and use the file specified if one is provided\n");
	printf("\t\t(to log to standard output, use '-log -')\n");
	printf("runTests also passes parameters to test_driver:\n");
	system("test_driver -help");
	exit(0);
      } else if (strcmp(argv[i], "-static") == 0) {
	staticTests = true;
      } else if (strcmp(argv[i], "-dynamic") == 0) {
	staticTests = false;
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
     unlink(getenv("RESUMELOG"));
   } else if (isRegFile(string(DEFAULT_RESUMELOG))) {
     unlink(DEFAULT_RESUMELOG);
   }

   // Remove a stale crashlog, if it exists
   if (getenv("CRASHLOG") && isRegFile(string(getenv("CRASHLOG")))) {
     unlink(getenv("CRASHLOG"));
   } else if (isRegFile(string(DEFAULT_CRASHLOG))) {
     unlink(DEFAULT_CRASHLOG);
   }

   // Create a PIDs file, to track mutatee PIDs
   char pidFilename[32];
   initPIDFilename(pidFilename, 32);
   if (isRegFile(string(pidFilename))) {
     unlink(pidFilename); // Ensure that the file doesn't already exist
   }

   // result == 2 indicates that there are no more tests to run
   while ( result != NOTESTS && invocation < MAX_ITER )
   {
      result = RunTest(invocation, useLog, staticTests, logfile, testLimit,
		       child_argv, pidFilename);
      invocation++;
      // I want to kill any remaining mutatees now, to clean up.  I should also
      // set a timer in RunTest in case something goes weird with test_driver.
      // (I think we'd be better off moving away from timer.pl)
      cleanupMutatees(pidFilename);
      if (-3 == result) {
	// User interrupted the test run; allow them a couple of seconds to do
	// it again and kill runTests
	// TODO Make sure this is portable to Windows
	sleep(2);
      }
   }

   // Remove the PID file, now that we're done with it
   if (isRegFile(string(pidFilename))) {
     unlink(pidFilename);
   }

   return 0;
}
