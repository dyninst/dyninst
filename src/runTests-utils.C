#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <cstring>

#include "runTests-utils.h"

#include <sys/types.h>
#include <dirent.h>

extern string pdscrdir;

int timeout = 1200;

string ReplaceAllWith(const string &in, const string &replace, const string &with);

void generateTestString(bool resume, bool useLog, bool staticTests,
      string &logfile,
      int testLimit, vector<char *>& child_argv, string& shellString)
{
   stringstream testString;
   if (staticTests) {
     testString << "test_driver_static";
   } else {
     testString << "test_driver";
   }
   testString << " -under-runtests -enable-resume -limit " << testLimit;

   if ( resume )
   {
      testString << " -use-resume";
   }
   if ( useLog )
   {
      testString << " -log";
   }
   if (useLog) {
     testString << " -logfile " << logfile;
   }

   // Add child's arguments
   for ( unsigned int i = 0; i < child_argv.size(); i++ )
   {
      testString << " " << child_argv[i];
   }
   
   stringstream timerString;
   if (pdscrdir.length()) 
      timerString << pdscrdir << "/timer.pl -t " << timeout << " ";
   shellString = timerString.str() + testString.str();
   
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

static bool dir_exists(const char *name) 
{
   DIR *dir = opendir(name);
   if (!dir) 
      return false;
   closedir(dir);
   return true;
}

void setupVars(bool useLog, string &logfile)
{
   string base_dir, tlog_dir;

#if defined(m_abi)
   if ( getenv("DYNINSTAPI_RT_LIB") )
   {
      char *rtlib = getenv("DYNINSTAPI_RT_LIB");

      string temp = ReplaceAllWith(rtlib, "libdyninstAPI_RT.so", "libdyninstAPI_RT_m32.so");
      char *rtlib_mabi = strdup(temp.c_str());
      setenv("DYNINSTAPI_RT_LIB_MABI", rtlib_mabi, 0);
   }
#endif
  
   // Determine Base Dir
   char *cstr_base_dir = NULL;
   cstr_base_dir = getenv("PARADYN_BASE");
   if (!cstr_base_dir || !dir_exists(cstr_base_dir))
      cstr_base_dir = getenv("DYNINST_ROOT");
   if (!cstr_base_dir || !dir_exists(cstr_base_dir))
      cstr_base_dir = "../../../";
   base_dir = string(cstr_base_dir);

   pdscrdir = base_dir + "/scripts";
   if (!dir_exists(pdscrdir.c_str()))
      pdscrdir = string("");
   
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
