#include <iostream>
#include <string>
#include <sstream>

#include "runTests-utils.h"

using namespace std;

void initPIDFilename(char *buffer, size_t len) {
  // FIXME Make this produce a name that is more likely to be unique to this
  // execution of runTests
  sprintf(buffer, len, "pids.windows");
}

void cleanupMutatees(char *pidFilename) {
  // TODO Fill this function in
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
// Sets up the command string to run the tests and executes test_driver
int RunTest(unsigned int iteration, bool useLog, bool staticTests,
	    string logfile, int testLimit, vector<char *> child_argv,
	    char *pidFilename) {
  string shellString;

  generateTestString(iteration > 0, useLog, staticTests, logfile,
		     testLimit, child_argv, shellString, pidFilename);

  int ret = executeTests(shellString);
  int ret = system(SimpleShellEscape(shellString).c_str());

  return WEXITSTATUS(ret);
}

void generateTestString(bool resume, bool useLog, bool staticTests,
			string &logfile, int testLimit,
			vector<char *>& child_argv, string& shellString,
			char *pidFilename)
{
   stringstream testString;
   testString << "test_driver.exe -under-runtests -enable-resume -limit " << testLimit;
   testString << " -pidfile " << pidFilename;

   if ( resume )
   {
      testString << " -use-resume";
   }
   if ( useLog )
   {
     testString << " -log -logfile " << logfile.c_str();
   }

   // Add child's arguments
   for ( unsigned int i = 0; i < child_argv.size(); i++ )
   {
      testString << " " << child_argv[i];
   }
   

   shellString = testString.str();
}

char *setResumeEnv()
{
   if ( getenv("RESUMELOG") != NULL )
      return NULL;
   
   stringstream tmp;
   tmp << "RESUMELOG=";
   if ( getenv("TMP") )
   {
      tmp << getenv("TMP") << "\\";
   }
   else
   {
      tmp << "C:\\";
   }

   tmp << "test_driver.resumelog." << getpid();

   char *r_tmp = strdup(tmp.str().c_str());
   putenv(r_tmp);

   return r_tmp;
}

// We don't need to do anything here, because on Windows . is
// explictly in the path.
char *setLibPath()
{
   return NULL;
}

void setupVars(bool useLog, string &logfile)
{
   if ( useLog && strlen(logfile.c_str()) == 0 )
   {
      cerr << "You must provide a logfile name after the -log option" << endl;
      exit(1);
   }
   return;
}
