#include <iostream>
#include <string>
#include <sstream>

#include <windows.h>
#include "runTests-utils.h"

using namespace std;

// configuration: timeout in seconds
int timeout = 1200;

void initPIDFilename(char *buffer, size_t len) {
  _snprintf(buffer, len, "pids.windows.%lu", GetCurrentProcessId());
}

void cleanupMutatees(char *pidFilename) {
  // TODO Fill this function in
}

//located in runTests.C
string ReplaceAllWith(const string &in, const string &replace, const string &with);

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
	
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	si.cb = sizeof(si);
	
	// CreateProcess requires that this not be const
	char * lpsz_shellString = (char *)malloc((shellString.length() + 1) * sizeof(char));
	//TODO handle out of memory error
	strcpy(lpsz_shellString, shellString.c_str());
	
	//execute
	if (CreateProcess(NULL, lpsz_shellString, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		// wait for process, or timeout
		WaitForSingleObject(pi.hProcess, timeout * 1000);
		
		// TODO catch crash, etc
	} else {
		// TODO failed to create process, handle error
		fprintf(stderr, "command line: %s\n", lpsz_shellString);
		fprintf(stderr, "[%s:%u] - Error creating process: error code %ld\n",
			__FILE__, __LINE__, GetLastError());
	}

	free(lpsz_shellString);
	
//	int ret = executeTests(shellString);
//	int ret = system(SimpleShellEscape(shellString).c_str());

//	return WEXITSTATUS(ret);
	//TODO actual value here
	return 0;
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
