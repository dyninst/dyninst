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
			std::string logfile, int testLimit, std::vector<char *> child_argv,
            char *pidFilename, char const * /*memcpu_file*/) {
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
	// -4 = we couldn't run test driver; thus if CreateProcess fails,
	// we want to return something meaningful that will keep us from infinite
	// looping
	DWORD exitcode = -4;
	if (CreateProcess(NULL, lpsz_shellString, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		// wait for process, or timeout
		WaitForSingleObject(pi.hProcess, timeout * 1000);
		GetExitCodeProcess(pi.hProcess, &exitcode);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		// TODO catch crash, etc
	} else {
		// TODO failed to create process, handle error
		fprintf(stderr, "command line: %s\n", lpsz_shellString);
		fprintf(stderr, "[%s:%u] - Error creating process: error code %ld\n",
			__FILE__, __LINE__, GetLastError());
	}

	free(lpsz_shellString);
	
	return exitcode;
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
