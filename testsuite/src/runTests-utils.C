#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <string>

#include "runTests-utils.h"

extern string pdscrdir;

int timeout = 120;

void generateTestString(bool resume, bool useLog, string& logfile,
      int testLimit, vector<char *>& child_argv, string& shellString)
{
   stringstream testString;
   testString << "test_driver -enable-resume -limit " << testLimit;

   if ( resume )
   {
      testString << " -use-resume";
   }
   if ( useLog )
   {
      testString << " -log";
   }

   // Add child's arguments
   for ( unsigned int i = 0; i < child_argv.size(); i++ )
   {
      testString << " " << child_argv[i];
   }
   
   stringstream timerString;
   timerString << pdscrdir << "/timer.pl -t " << timeout;
   shellString = timerString.str() + " " + testString.str();

   if ( useLog )
   {
      shellString += " >> " + logfile + " 2>&1";
   }

}

char *setResumeEnv()
{
   if ( getenv("RESUMELOG") != NULL )
      return NULL;

   stringstream tmp;
   tmp << "RESUMELOG=";
   if ( getenv("TMP") )
   {
      tmp << getenv("TMP") << "/";
   }
   else
   {
      tmp << "/tmp/";
   }

   tmp << "test_driver.resumelog." << getpid();

   char *r_tmp = strdup(tmp.str().c_str());
   putenv(r_tmp);

   return r_tmp;
}

char *setLibPath()
{
   stringstream tmp;
   tmp << "LD_LIBRARY_PATH=.:";

   if ( getenv("LD_LIBRARY_PATH") )
   {
      tmp << getenv("LD_LIBRARY_PATH");
   }

   char *l_tmp = strdup(tmp.str().c_str());
   putenv(l_tmp);

   return l_tmp;
}

void setupVars(bool useLog, string& logfile)
{
   string base_dir, tlog_dir;
  
   // Determine Base Dir
   if ( getenv("PARADYN_BASE") == NULL )
   {
      if ( getenv("DYNINST_ROOT") == NULL )
      {
         cerr << "DYNINST_ROOT or PARADYN_BASE must be set." << endl;
         exit(1);
      }
      else
      {
         base_dir = getenv("DYNINST_ROOT");
      }
   } else
   {
      base_dir = getenv("PARADYN_BASE");
   }

   pdscrdir = base_dir + "/scripts";
   if ( ! isDir(pdscrdir) )
   {
      cerr << pdscrdir << " does not exist.  Paradyn scripts dir required." 
         << endl;
      exit(1);
   }

   // Determine Test log dir
   char *pdtst = getenv("PDTST");
   if ( pdtst == NULL )
   {
      tlog_dir = base_dir + "/log/tests";
   } else
   {
      tlog_dir = pdtst;
   }
   tlog_dir += "2";

   // Determin Test log file
   string buildnum, build_id;
   buildnum = pdscrdir + "/buildnum";
   if ( getenv("PARADYN_BASE") != NULL && isRegFile(buildnum) )
   {
      getInput(buildnum.c_str(), build_id); 
   } else
   {
      getInput("date '+%Y-%m-%d'", build_id);
   }

   if ( useLog ) 
   {

      if ( logfile == "" )
      {
         logfile = tlog_dir + "/" + getenv("PLATFORM") + "/" + build_id;
      }

      cout << "   ... output to " << logfile << endl;

      string testslogdir, cmd;
      cmd = "dirname " + logfile;
      getInput(cmd.c_str(), testslogdir);
   
      if ( ! isDir(testslogdir) )
      {
         cout << testslogdir << "does not exist (yet)!" << endl;
         cmd = "mkdir -p " + testslogdir;
         system(cmd.c_str());
         if ( ! isDir(testslogdir) )
         {
            cout << testslogdir << " creation failed - aborting!" << endl;
            exit(1);
         } else {
            cout << testslogdir << " create for test logs!" << endl;
         }
      }

      if ( isRegFile(logfile) )
      {
         cout << "File exists" << endl;
      }
      else
      {
         cmd = "touch " + logfile;
         system(cmd.c_str());
      }
   
      cmd = logfile + ".gz";
      if ( isRegFile(cmd) )
      {
         cout << "File.gz exists" << endl;
      }
   }
}
