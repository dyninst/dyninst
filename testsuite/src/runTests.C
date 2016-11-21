/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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
#ifdef os_windows_test
//needed for Sleep
#include <winsock2.h>
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
#include <assert.h>

#include "runTests-utils.h"
#include "error.h"
#include "help.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>
#include <vector>

// Default name for the resume log file
#define DEFAULT_RESUMELOG "resumelog"
// Default name for the crash log file
#define DEFAULT_CRASHLOG "crashlog"

bool staticTests = false;
bool useLog = false;
string logfile;
string pdscrdir;

int parallel_copies = 0;
vector<test_driver_t> test_drivers;
vector<std::string> hosts;

// Run No more than testLimit tests before re-exec'ing test_driver
int testLimit = 10;

vector<char *> child_argv;
char *scriptname;

char *outputlog_name;
int outputlog_pos = -1;
FILE *outputlog_file = NULL;

#if !defined(os_linux_test) && !defined(os_bgq_test)
int getline(char **line, size_t *line_size, FILE *f)
{
   if (*line == NULL) {
      *line = (char *) malloc(4096);
      *line_size = 4096;
   }
   char *result = fgets(*line, *line_size, f);
   if (!result)
      return -1;
   return 0;
}
#endif

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
      } else if (strcmp(argv[i], "-j") == 0) {
         if (i+1 != argc)
            parallel_copies = atoi(argv[++i]);
         if (!parallel_copies) {
            fprintf(stderr, "Error: -j requires a non-zero integer argument\n");
            exit(-1);
         }
      }
      else if (strcmp(argv[i], "-hosts") == 0)
      {
         for (;;)
         {
            i++;
            if (i == argc)
               break;
            if (argv[i][0] == '-') {
               i--;
               break;
            }
            hosts.push_back(std::string(argv[i]));
         }
      }
      else if (strcmp(argv[i], "-humanlog") == 0 || strcmp(argv[i], "-dboutput") == 0) {
         child_argv.push_back(argv[i]);
         if (i+1 != argc && (argv[i+1][0] != '-' || argv[i+1][1] == '\0')) {
            i++;
            outputlog_name = argv[i];
            outputlog_pos = child_argv.size();
            child_argv.push_back(argv[i]);
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

static void clear_resumelog()
{
   for (int  i=0; i<parallel_copies; i++)
   {
      char s[32];
      snprintf(s, 32, "%d", i+1);
    
      std::string resumename = std::string(DEFAULT_RESUMELOG) + std::string(".") + std::string(s);
      unlink(resumename.c_str());

      std::string mresumename = std::string("mutatee_") + resumename;
      unlink(mresumename.c_str());
   }
   unlink(DEFAULT_RESUMELOG);
   std::string mresumename = std::string("mutatee_") + DEFAULT_RESUMELOG;
   unlink(mresumename.c_str());
}

int main(int argc, char *argv[])
{
   parseParameters(argc, argv);
   setupVars(useLog, logfile);

   setLibPath();
   int numFailed = 0;
   int result = 0;
   int invocation = 0;

   // Create a PIDs file, to track mutatee PIDs
   //char *pidFilename = new char[80];
   //initPIDFilename(pidFilename, 80);
   char *pidFilename = NULL;

   char *line = NULL;
   size_t line_size = 0;


   if (!parallel_copies)
      parallel_copies = 1;
   
   clear_resumelog();
   
   char *parallel_copies_cs = NULL;
   {
      char s[32];
      snprintf(s, 32, "%d", parallel_copies);
      parallel_copies_cs = strdup(s);
   }

   test_drivers.resize(parallel_copies);
   for (unsigned i=0; i<parallel_copies; i++) {
      char unique_cs[32];
      int unique = i+1;
      snprintf(unique_cs, 32, "%d", unique);
      std::string unique_s(unique_cs);

      test_drivers[i].pid = 0;
      test_drivers[i].last_result = 0;
      test_drivers[i].unique = unique;
      test_drivers[i].useLog = useLog;
      test_drivers[i].staticTests = staticTests;
      test_drivers[i].logfile = logfile + std::string(".") + unique_s;
      test_drivers[i].testLimit = testLimit;
      test_drivers[i].child_argv = child_argv;
      if (pidFilename)
         test_drivers[i].pidFilename = pidFilename + std::string(".") + unique_s;

      if (parallel_copies > 1)
      {
         test_drivers[i].child_argv.push_back(const_cast<char*>("-unique"));
         test_drivers[i].child_argv.push_back(strdup(unique_cs));
         test_drivers[i].child_argv.push_back(const_cast<char*>("-max-unique"));
         test_drivers[i].child_argv.push_back(parallel_copies_cs);

         test_drivers[i].outputlog = std::string("par_outputlog.") + unique_s;
         unlink(test_drivers[i].outputlog.c_str());
         
         if (outputlog_pos != -1) {
            test_drivers[i].child_argv[outputlog_pos] = const_cast<char *>(test_drivers[i].outputlog.c_str());
         }
         else {
            test_drivers[i].child_argv.push_back(const_cast<char*>("-humanlog"));
            test_drivers[i].child_argv.push_back(const_cast<char *>(test_drivers[i].outputlog.c_str()));
         }
         if (outputlog_name && (strcmp(outputlog_name, "-") != 0)) {
            outputlog_file = fopen(outputlog_name, "w");
         }
         else {
            outputlog_file = stdout;
         }         
      }
      if (hosts.size()) {
         test_drivers[i].hostname = hosts[i % hosts.size()];
      }
   }

   if (hosts.size()) {
      scriptname = createParallelScript();
   }

   bool done = false;
   bool timeout = false;  // This should be pushed into test_driver.
   for (;;)
   {
      done = true;
      for (int i=0; i<parallel_copies; i++) {
         if (test_drivers[i].last_result == NOTESTS || timeout) {
            //This invocation is done or produced an error
            continue;
         }
         done = false;
         if (test_drivers[i].pid) {
            //Still running
            continue;
         }

         test_drivers[i].pid = RunTest(invocation,
                                       test_drivers[i].useLog,
                                       test_drivers[i].staticTests,
                                       test_drivers[i].logfile,
                                       test_drivers[i].testLimit,
                                       test_drivers[i].child_argv,
                                       test_drivers[i].pidFilename.c_str(),
                                       test_drivers[i].hostname);
         invocation++;
         if (test_drivers[i].pid < 0) {
            fprintf(stderr, "Could not execute test_driver\n");
            return -1;
         }
      }
      if (done)
         break;

      int driver = CollectTestResults(test_drivers, parallel_copies);
      if (driver == -3) {
         // User interrupted the test run; allow them a couple of seconds to do
         // it again and kill runTests, or restart test_drivers.
         // TODO Make sure this is portable to Windows
         fprintf(stderr, "Press ctrl-c again within 2 seconds to abort runTests.\n");
         sleep(2);
      }
      if (driver == -2) {
          // We apparently have no children.  This may not be a possibility
          // anymore after we added the timeout flag.  I'm not sure what to
          // do in this case, though.  Both continuing and breaking are
          // problematic.
          assert(0 && "No children returned from waitpid.");
      }
      if (driver == -1) {
          // Timeout was encountered, and children were reaped.
          timeout = true;
          for (int idx=0; idx < parallel_copies; idx++) {
             test_drivers[idx].last_result = -1;
          }
		  break;
      }
      if (test_drivers[driver].last_result == -4) {
         //Exec error
         fprintf(stderr, "Failed to exec test_driver\n");
         break;
      }
      if (test_drivers[driver].last_result == -5) {
         //Help
         break;
      }

      if (parallel_copies > 1)
      {
         FILE *f = fopen(test_drivers[driver].outputlog.c_str(), "r");
         if (f) {
            for (;;) {
               int result = (int) getline(&line, &line_size, f);
               if (result == -1)
                  break;
               fprintf(outputlog_file, "%s", line);
            }
            fclose(f);
            unlink(test_drivers[driver].outputlog.c_str());
         }
      }
   }

   // Remove the PID file, now that we're done with it
   if (pidFilename && isRegFile(string(pidFilename))) {
      unlink(pidFilename);
   }
   if (scriptname) {
      unlink(scriptname);
   }
   
   for (unsigned i=0; i<parallel_copies; i++)
   {
      char s[32];
      snprintf(s, 32, "%d", i+1);
    
      std::string resumename = std::string(DEFAULT_RESUMELOG) + std::string(".") + std::string(s);
      unlink(resumename.c_str());

      std::string mresumename = std::string("mutatee_") + resumename;
      unlink(mresumename.c_str());
   }

   clear_resumelog();

   return 0;
}


