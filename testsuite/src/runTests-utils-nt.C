#include <iostream>
#include <string>
#include <sstream>

#include "runTests-utils.h"

using namespace std;

void generateTestString(bool resume, bool useLog, bool staticTests, string &logfile,
      int testLimit, vector<char *>& child_argv, string& shellString)
{
   stringstream testString;
   testString << "test_driver.exe -enable-resume -limit " << testLimit;

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
   

//    if ( useLog )
//    {
//       testString << " >> " << logfile << " 2>&1";
//    }

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
