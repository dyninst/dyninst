#include <stdlib.h>
#include <sstream>
#include <string>
#include <iostream>
#include <stdio.h>
#include <vector>

#include "runTests-utils.h"
#include "error.h"
#include <sys/stat.h>
#include <sys/types.h>

// To prevent run away never run test_driver more the MAX_ITER times
#define MAX_ITER 1000

bool useLog = false;
string logfile = "";
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
   for ( int i = 1; i < argc; i++ )
   {
      if ( strncmp(argv[i], "-log", 4) == 0 )
      {
         useLog = true;
         // Check to see if next line provides a logfile name
         if ( i + 1 < argc && argv[i+1][0] != '-' )
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

// SimpleShellEscape:
// Escapes some characters in the input string 
string SimpleShellEscape(string &str)
{
   string tmp;
   tmp = ReplaceAllWith(str, "*", "\\*");
   tmp = ReplaceAllWith(tmp, "?", "\\?");
   return tmp;
}

// RunTest:
// Sets up the command string to run a single test
int RunTest(unsigned int iteration)
{
   string shellString;

   generateTestString(iteration > 0, useLog, logfile, 
                      testLimit, child_argv, shellString);

   int ret = system(SimpleShellEscape(shellString).c_str());

   return WEXITSTATUS(ret);
}

int main(int argc, char *argv[])
{
   parseParameters(argc, argv);
   setupVars(useLog, logfile);

   setLibPath();
   

   int result = 0;
   int invocation = 0;

   // result == 2 indicates that there are no more tests to run
   while ( result != NOTESTS && invocation < MAX_ITER )
   {
      result = RunTest(invocation);
      invocation++;
   }

   if ( getenv("RESUMELOG") && isRegFile(string(getenv("RESUMELOG"))) )
   {
      unlink(getenv("RESUMELOG"));
   }
   
}
