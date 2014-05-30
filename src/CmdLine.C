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

#include <cassert>
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

#if defined(os_windows_test)
#include <time.h>
#endif

#include "help.h"
#include "CmdLine.h"
#include "ResumeLog.h"
#include "test_info_new.h"
#include "error.h"

#if !defined(os_windows_test)
#include <fnmatch.h>
#endif

using namespace std;

#if defined(os_windows_test)
#define DEFAULT_LOGNAME "NUL"
#else
#define DEFAULT_LOGNAME "/dev/null"
#endif

enum Mode {
   explicitOn,
   explicitOff,
   defaultOn,
   defaultOff
};

struct ModeGroup {
   const char *option;
   int group;
   Mode mode;
};

static bool isModeParam(const char *param);
static void setAllOn(int groups, bool force);
static bool paramOn(const char *param);
static bool mutateeListContains(std::vector<char *> mutatee_list, const char *mutatee);
static bool testListContains(TestInfo * test, std::vector<char *> &testsn);
static int handleArgs(int argc, char *argv[]);
static void disableUnwantedTests(std::vector<RunGroup *> &groups);
static void sortGroups(std::vector<RunGroup *> &groups);
static void setIndexes(std::vector<RunGroup *> groups);
static void setupArgDictionary(ParameterDict &params);
static void setupGroupDictionary(ParameterDict &params);
static bool fileExists(std::string f);

#define END       0
#define COMPILERS (1 << 1)
#define RUNMODES  (1 << 2)
#define OPTLEVELS (1 << 3)
#define COMPS     (1 << 4)
#define ABI       (1 << 5)
#define THRDMODE  (1 << 6)
#define PROCMODE  (1 << 7)
#define LINKMODE  (1 << 8)
#define PICMODE   (1 << 9)
#define PLATMODE  (1 << 10)
//If you add more modes here, then update the lists in "-all" and "-full" parsing below

ModeGroup mode_args[] = {
   { "gcc",         COMPILERS, defaultOff },
   { "g++",         COMPILERS, defaultOn  },
   { "gfortran",    COMPILERS, defaultOn  },
   { "icc",         COMPILERS, defaultOff },
   { "icpc",        COMPILERS, defaultOff },
   { "pgcc",        COMPILERS, defaultOff },
   { "pgcxx",       COMPILERS, defaultOff },
   { "cc",          COMPILERS, defaultOff },
   { "CC",          COMPILERS, defaultOff },
   { "cxx",         COMPILERS, defaultOff },
   { "VC",          COMPILERS, defaultOff },
   { "VC++",        COMPILERS, defaultOn  },
   { "bg_gcc",      COMPILERS, defaultOff },
   { "bg_g++",      COMPILERS, defaultOn  },
   { "bg_gfortran", COMPILERS, defaultOff },
   { "bgq_gcc",     COMPILERS, defaultOff },
   { "bgq_g++",     COMPILERS, defaultOn  },
   { "bgq_gfortran",COMPILERS, defaultOff },
   { "suncc",       COMPILERS, defaultOff },
   { "xlc",         COMPILERS, defaultOff },
   { "xlC",         COMPILERS, defaultOff },
   { "ibmas",       COMPILERS, defaultOff },
   { "masm",        COMPILERS, defaultOff },
   { "nasm",        COMPILERS, defaultOff },
   { "bgxlc",       COMPILERS, defaultOff },
   { "bgxlc++",     COMPILERS, defaultOn  },
   { "nocompiler",  COMPILERS, defaultOn  },
   { "create",      RUNMODES,  defaultOn  },
   { "attach",      RUNMODES,  defaultOff },
   { "rewriter",    RUNMODES,  defaultOn  },
   { "serialize",   RUNMODES,  defaultOff },
   { "max",         OPTLEVELS, defaultOff },
   { "high",        OPTLEVELS, defaultOff },
   { "low",         OPTLEVELS, defaultOff },
   { "none",        OPTLEVELS, defaultOn  },
   { "dyninst",     COMPS,     defaultOn  },
   { "symtab",      COMPS,     defaultOn  },
   { "proccontrol", COMPS,     defaultOn  },
   { "instruction", COMPS,     defaultOn  },
   { "32",          ABI,       defaultOff },
   { "64",          ABI,       defaultOn  },
   { "st",          THRDMODE,  defaultOn  },
   { "mt",          THRDMODE,  defaultOff },
   { "sp",          PROCMODE,  defaultOn  },
   { "mp",          PROCMODE,  defaultOff },
   { "dynamiclink", LINKMODE,  defaultOn  },
   { "staticlink",  LINKMODE,  defaultOff },
   { "pic",         PICMODE,   defaultOff },
   { "nonpic",      PICMODE,   defaultOn  },
   { "smp",         PLATMODE,  defaultOn  },
   { "dual",        PLATMODE,  defaultOff },
   { "vn",          PLATMODE,  defaultOff },
   { NULL,          NONE,      defaultOff } };

static std::vector<char *> mutatee_list;
static std::vector<char *> test_list; 

static bool useHumanLog = true;
static bool shouldDebugBreak = false;
static bool called_from_runTests = false;
static bool printMutateeLogHeader = false;
static bool noclean = false;
static int testLimit = 0;
static int groupLimit = 0;
static bool limitedTests = false;
static bool debugPrint = false;
static int unique_id = 0;
static int max_unique_id = 0;
static int next_resume_group = -1;
static int next_resume_test = -1;
static bool no_header = false;
static bool measureMEMCPU = false;
static char *humanlog_name = NULL;
static char *logfilename = NULL;
static char *dbfilename = NULL;
static char *debug_out_filename = NULL;

static std::vector<RunGroup *> group_list;

static std::string given_mutatee = std::string("");
static int given_mutator = -1;

static bool in_runTests;

static int port = 0;
static string hostname;

int parseArgs(int argc, char *argv[], ParameterDict &params)
{
   //Parse args
   int result = handleArgs(argc, argv);
   if (result)
      return result;

   //Initialize dictionary parameters from args
   setupArgDictionary(params);

   return 0;
}

void getGroupList(std::vector<RunGroup *> &group_list, ParameterDict &params)
{
   //Initialize and enable tests according to options
   initialize_mutatees(group_list);
   sortGroups(group_list);
   disableUnwantedTests(group_list);
   setIndexes(group_list);

   setupGroupDictionary(params);
}
void setupArgDictionary(ParameterDict &params)
{
   params["usehumanlog"] = new ParamInt((int) useHumanLog);
   params["debugPrint"] = new ParamInt((int) debugPrint);
   params["noClean"] = new ParamInt((int) noclean);
   params["unique_id"] = new ParamInt((int) unique_id);
   params["debugbreak"] = new ParamInt((int) shouldDebugBreak);
   params["under_runtests"] = new ParamInt((int) called_from_runTests);
   params["in_runtests"] = new ParamInt((int) in_runTests);
   params["printMutateeLogHeader"] = new ParamInt((int) printMutateeLogHeader);
   params["no_header"] = new ParamInt((int) no_header);
   params["measureMEMCPU"] = new ParamInt((int) measureMEMCPU);
   
   if (!logfilename)
      logfilename = const_cast<char *>(DEFAULT_LOGNAME);
   if (!humanlog_name)
      humanlog_name = const_cast<char *>("-");

   params["logfilename"] = new ParamString(logfilename);
   params["mutatee_resumelog"] = new ParamString("mutatee_resumelog");
   params["humanlogname"] = new ParamString(humanlog_name);
   params["dboutput"] = new ParamString(dbfilename);

   if (given_mutatee != std::string("") && given_mutator != -1)
   {
      params["given_mutatee"] = new ParamString(given_mutatee.c_str());
      params["given_mutator"] = new ParamInt(given_mutator);
   }

   params["port"] = new ParamInt(port);
   params["hostname"] = new ParamString(hostname.c_str());
   params["redirect"] = new ParamString(debug_out_filename ? debug_out_filename : "");
}

void setupGroupDictionary(ParameterDict &params)
{
   params["limited_tests"] = new ParamInt((int) limitedTests);
   params["next_resume_group"] = new ParamInt((int) next_resume_group);
   params["next_resume_test"] = new ParamInt((int) next_resume_test);
}

static int handleArgs(int argc, char *argv[])
{
   char *exec_name = argv[0];
   char *file_exec_name = strrchr(exec_name, '/');
   if (!file_exec_name)
      file_exec_name = strrchr(exec_name, '\\');
   if (!file_exec_name)
      file_exec_name = exec_name;

   if (strstr(file_exec_name, "runTests")) {
      in_runTests = true;
   }
   else {
      assert(strstr(file_exec_name, "test_driver") || 
             strstr(file_exec_name, "testdriver_be"));
      in_runTests = false;         
   }


   for (int i=1; i < argc; i++ )
   {
      if ( strcmp(argv[i], "-test") == 0)
      {
         char *tests;
         char *name;

         if ( i + 1 >= argc )
         {
            fprintf(stderr, "-test must be followed by a testname\n");
            return NOTESTS;
         }

         tests = strdup(argv[++i]);

         name = strtok(tests, ",");
         test_list.push_back(name);
         while ( name != NULL )
         {
            name = strtok(NULL, ",");
            if ( name != NULL )
            {
               test_list.push_back(name);
            }
         }
      }
      else if ( strcmp(argv[i], "-run") == 0)
      {
         int j;
         for ( j = i+1; j < argc; j++ )
         {
            if ( argv[j][0] == '-' )
            {
               // end of test list
               break;
            }
            else
            {
               test_list.push_back(argv[j]);
            }
         }
         i = j - 1;
      }
      else if ( strcmp(argv[i], "-mutatee") == 0)
      {
         char *mutatees;
         char *name;

         if ( i + 1 >= argc )
         {
            fprintf(stderr, "-mutatee must be followed by mutatee names\n");
            return NOTESTS;
         }

         mutatees = strdup(argv[++i]);

         name = strtok(mutatees, ",");
         if (NULL == name) {
            // Special handling for a "" mutatee specified on the command line
            mutatee_list.push_back(strdup(""));
         } else {
            mutatee_list.push_back(name);
         }
         while ( name != NULL )
         {
            name = strtok(NULL, ",");
            if ( name != NULL )
            {
               mutatee_list.push_back(name);
            }
         }
      }
      else if (isModeParam(argv[i])) {
         //Nothing to do, handled in isModeParam
      }
      else if (strcmp(argv[i], "-allcompilers") == 0)
      {
         setAllOn(COMPILERS, true);
      }
      else if (strcmp(argv[i], "-allmode") == 0)
      {
         setAllOn(RUNMODES, true);
      }
      else if (strcmp(argv[i], "-all") == 0)
      {
         setAllOn(COMPILERS | RUNMODES | COMPS | ABI | THRDMODE | PROCMODE | LINKMODE | PICMODE | PLATMODE, false);
      }
      else if (strcmp(argv[i], "-full") == 0)
      {
         //Like -all, but with full optimization levels
         setAllOn(COMPILERS | OPTLEVELS | RUNMODES | COMPS | ABI | THRDMODE | PROCMODE | LINKMODE | PICMODE | PLATMODE, false);
      }
      else if (strcmp(argv[i], "-allcomp") == 0)
      {
         setAllOn(COMPS, true);
      }
      else if (strcmp(argv[i], "-allopt") == 0)
      {
         setAllOn(OPTLEVELS, true);
      }
      else if (strcmp(argv[i], "-allpmode") == 0)
      {
         setAllOn(PLATMODE, true);
      }
      else if ( strcmp(argv[i], "-noclean") == 0 )
      {
         noclean = true;
      }
      else if (strncmp(argv[i], "-verbose", 2) == 0)
      {
         debugPrint = true;
      }
      else if ( strcmp(argv[i], "-log")==0)
      {
         logfilename = const_cast<char *>("-");
         if ((i + 1) < argc) {
            if ((argv[i + 1][0] != '-') || (argv[i + 1][1] == '\0')) {
               i += 1;
               logfilename = argv[i];
            }
         }
      }
      else if ( strcmp(argv[i], "-logfile") == 0) 
      {
         fprintf(stderr, "WARNING: -logfile is a deprecated option; use -log instead\n");
         /* Store the log file name */
         if ((i + 1) >= argc) {
            fprintf(stderr, "Missing log file name\n");
            return NOTESTS;
         }
         i += 1;
         logfilename = argv[i];
      }
      else if ( strcmp(argv[i], "-debug")==0)
      {
         shouldDebugBreak = true;
      }
      else if ((strcmp(argv[i], "-enable-resume") == 0) ||
               (strcmp(argv[i], "-use-resume") == 0)) 
      {
         enableResumeLog();
      } 
      else if ( strcmp(argv[i], "-header") == 0 ) 
      {
         printMutateeLogHeader = true;
      } 
      else if ( strcmp(argv[i], "-limit") == 0 || strcmp(argv[i], "-test-limit") == 0) 
      {
         if (i + 1 < argc) {
            testLimit = strtol(argv[++i], NULL, 10);
         }
         if (!testLimit) {
            fprintf(stderr, "-test-limit must be followed by an integer limit\n");
            return NOTESTS;
         }
         if (testLimit && groupLimit) {
            fprintf(stderr, "-test-limit and -group-limit are mutually exclusive options\n");
            return NOTESTS;
         }
      }
      else if ( strcmp(argv[i], "-group-limit") == 0)
      {
         if (i + 1 < argc) {
            groupLimit = strtol(argv[++i], NULL, 10);
         }
         if (!groupLimit) {
            fprintf(stderr, "-group-limit must be followed by an integer limit\n");
            return NOTESTS;
         }
         if (testLimit && groupLimit) {
            fprintf(stderr, "-test-limit and -group-limit are mutually exclusive options\n");
            return NOTESTS;
         } 
      }
      else if (strcmp(argv[i], "-unique") == 0) 
      {
         if (i + 1 < argc) {
            unique_id = atoi(argv[++i]);
         }
         if (!unique_id) {
            fprintf(stderr, "-unique must be followed by a non-zero integer\n");
            return NOTESTS;
         }
      }
      else if (strcmp(argv[i], "-max-unique") == 0) 
      {
         if (i + 1 < argc) {
            max_unique_id = atoi(argv[++i]);
         }
         if (!max_unique_id) {
            fprintf(stderr, "-max_unique must be followed by a non-zero integer\n");
            return NOTESTS;
         }
      }
      else if (strcmp(argv[i], "-redirect-debug") == 0) {
         if (i + 1 >= argc) {
            fprintf(stderr, "-redirect-debug must be followed by a filename\n");
            return NOTESTS;
         }
         debug_out_filename = argv[++i];
      }
      else if ( strcmp(argv[i], "-humanlog") == 0 ) {
         // Verify that the following argument exists
         if ( i + 1 >= argc )
         {
            fprintf(stderr, "-humanlog must by followed by a filename\n");
            return NOTESTS;
         }

         useHumanLog = true;
         humanlog_name = argv[++i];
      } 
      else if (strcmp(argv[i], "-under-runtests") == 0) 
      {
         called_from_runTests = true;
      }
      else if (strcmp(argv[i], "-no-header") == 0)
      {
         no_header = true;
      }
      else if ((strcmp(argv[i], "-port") == 0)) {
         port = 0;
         if ((i + 1) < argc) {
            port = atoi(argv[++i]);
         }
         if (!port) {
            fprintf(stderr, "-port requires an integer argument\n");
            return NOTESTS;
         }
      }
      else if ((strcmp(argv[i], "-hostname") == 0)) {
         if ((i + 1) >= argc) {
            fprintf(stderr, "-hostname requires an argument\n");
            return NOTESTS;
         }
         hostname = argv[++i];
      }
      else if ((strcmp(argv[i], "-help") == 0) ||
               (strcmp(argv[i], "--help") == 0)) 
      {
         print_help();
         exit(-5);
      }
      else if (strcmp(argv[i], "-dboutput") == 0) {
         //check if a failed output file is specified
         if ((i + 1) < argc) {
            if (argv[i+1][0] != '-' || argv[i+1][1] == '\0') {
               //either it doesn't start with - or it's exactly -
               i++;
               dbfilename = argv[i];
            }
         }
         if (!dbfilename) {
            time_t rawtime;
            struct tm * timeinfo = (tm *)malloc(sizeof(struct tm));

            time(&rawtime);
            timeinfo = localtime(&rawtime);

            dbfilename = (char*)malloc(sizeof(char) * strlen("sql_dblog-xxxx-xx-xx0"));
            sprintf(dbfilename, "sql_dblog-%4d-%02d-%02d",
                    timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
         }
      }
      else if (strcmp(argv[i], "-given_mutatee") == 0) {
         if (i + 1 == argc) {
            fprintf(stderr, "-given_mutatee must be followed by a mutatee string\n");
            return NOTESTS;
         }
         if (given_mutatee != std::string("")) {
            fprintf(stderr, "-given_mutatee must be given only once\n");
            return NOTESTS;
         }
         given_mutatee = std::string(argv[++i]);
      }
      else if (strcmp(argv[i], "-given_mutator") == 0) {
         if (i + 1 == argc) {
            fprintf(stderr, "-given_mutator must be followed by a mutator number\n");
            return NOTESTS;
         }
         if (given_mutator != -1) {
            fprintf(stderr, "-given_mutator must be given only once\n");
            return NOTESTS;
         }
         given_mutator = atoi(argv[++i]);
      }
   }

   
   return 0;
}

static void setAllOn(int groups, bool force)
{
   for (unsigned i=0; mode_args[i].option != NULL; i++) {
      if (!(groups & mode_args[i].group))
         continue;
      if (force) {
         mode_args[i].mode = explicitOn;
      }
      else if (mode_args[i].mode == defaultOff) {
         mode_args[i].mode = defaultOn;
      }
   }
}

static bool isModeParam(const char *param) {
   if (param[0] == '-')
      param++;

   /**
    * The parameter passed gets set to explicitOn
    **/
   bool found_param = false;
   unsigned i;
   for (i=0; mode_args[i].option != NULL; i++) 
   {
      if (strcmp(mode_args[i].option, param) != 0) 
         continue;
      found_param = true;
      mode_args[i].mode = explicitOn;
      break;
   }
   if (!found_param)
      return false;

   /**
    * All parameters in the same group, which aren't explicitOn, are
    * set to defaultOff
    **/
   int group = mode_args[i].group;
   for (i=0; mode_args[i].option != NULL; i++) 
   {
      if (mode_args[i].group != group)
         continue;
      if (mode_args[i].mode == defaultOn || mode_args[i].mode == defaultOff)
         mode_args[i].mode = explicitOff;
   }
   return true;
}

static bool paramOn(const char *param)
{
   for (unsigned i=0; mode_args[i].option != NULL; i++) 
   {
      if (strcmp(mode_args[i].option, param) == 0) {
         return (mode_args[i].mode == defaultOn || mode_args[i].mode == explicitOn);
      }
   }
   assert(0);
   return false;
}

struct groupcmp 
{
   bool operator()(const RunGroup* lv, const RunGroup* rv)
   {
      if (lv->modname != rv->modname)
         return lv->modname < rv->modname;
      
      int mutatee_cmp = strcmp(lv->mutatee ? lv->mutatee : "", rv->mutatee ? rv->mutatee : "");
      if (mutatee_cmp)
         return mutatee_cmp < 0;
      
      if (lv->createmode != rv->createmode)
         return ((int) lv->createmode) < ((int) rv->createmode);

      if (lv->threadmode != rv->threadmode)
         return ((int) lv->threadmode) < ((int) rv->threadmode);
      
      if (lv->procmode != rv->procmode)
         return ((int) lv->procmode) < ((int) rv->procmode);

      int val = strcmp(lv->platmode, rv->platmode);
      if (val != 0)
         return (val < 0);
      
      return false;
   }
};

#define is_int(x) (x >= '0' && x <= '9')
static bool strint_lt(const char *lv, const char *rv)
{
   int i = 0;
   while (lv[i] != '\0' && rv[i] != '\0')
   {
      if (lv[i] == rv[i]) {
         i++;
         continue;
      }

      bool lint = is_int(lv[i]);
      bool rint = is_int(rv[i]);

      if (lint && !rint)
         return true;
      else if (!lint && rint)
         return false;
      else if (!lint && !rint)
         return (lv[i] < rv[i]);
      else {
         return atoi(lv+i) < atoi(rv+i);
      }
   }
   if (lv[i] == '\0' && rv[i] != '\0')
      return true;
   return false;
}

struct testcmp 
{
   bool operator()(const TestInfo* lv, const TestInfo* rv)
   {
      return strint_lt(lv->name, rv->name);
   }
};

static void sortGroups(std::vector<RunGroup *> &groups)
{
   std::sort(groups.begin(), groups.end(), groupcmp());
   for (unsigned i=0; i<groups.size(); i++)
      std::sort(groups[i]->tests.begin(), groups[i]->tests.end(), testcmp());
}

static void disableUnwantedTests(std::vector<RunGroup *> &groups)
{
   for (unsigned  i = 0; i < groups.size(); i++) 
   {
      //RunMode
      if (((groups[i]->createmode == CREATE) && !paramOn("create")) ||
          ((groups[i]->createmode == USEATTACH) && !paramOn("attach")) ||
          ((groups[i]->createmode == DISK) && !paramOn("rewriter")) ||
          ((groups[i]->createmode == DESERIALIZE) && !paramOn("serialize")))
      {
         groups[i]->disabled = true;
         continue;
      }
      //Component
      if (groups[i]->modname == std::string("") || 
          (groups[i]->modname == std::string("dyninst") && !paramOn("dyninst")) ||
          (groups[i]->modname == std::string("symtab") && !paramOn("symtab")) ||
          (groups[i]->modname == std::string("instruction") && !paramOn("instruction")) ||
          (groups[i]->modname == std::string("proccontrol") && !paramOn("proccontrol")))
      {
         groups[i]->disabled = true;
         continue;
      }
      //Mutatee list     
      if (!mutatee_list.empty() && !mutateeListContains(mutatee_list, groups[i]->mutatee))
      {
         groups[i]->disabled = true;
         continue;
      }
      //Compiler
      const char *compiler_name;
      if (!groups[i]->mutatee || strlen(groups[i]->mutatee) == 0)
         compiler_name = "nocompiler";
      else
         compiler_name = groups[i]->compiler;
      if (!paramOn(compiler_name)) {
         groups[i]->disabled = true;
         continue;
      }
      //Opt level
      if (groups[i]->optlevel && !paramOn(groups[i]->optlevel)) {
         groups[i]->disabled = true;
         continue;
      }
      //Thread mode
      if ((groups[i]->threadmode == MultiThreaded && !paramOn("mt")) ||
          (groups[i]->threadmode == SingleThreaded && !paramOn("st")))
      {
         groups[i]->disabled = true;
         continue;
      }
      //Process mode
      if ((groups[i]->procmode == MultiProcess && !paramOn("mp")) ||
          (groups[i]->procmode == SingleProcess && !paramOn("sp")))
      {
         groups[i]->disabled = true;
         continue;
      }
      //Link type
      if ((groups[i]->linktype == StaticLink && !paramOn("staticlink")) ||
          (groups[i]->linktype == DynamicLink && !paramOn("dynamiclink")))
      {
         groups[i]->disabled = true;
         continue;
      }
#if defined(cap_32_64_test)
      //ABI
      if (groups[i]->abi && !paramOn(groups[i]->abi)) {
         groups[i]->disabled = true;
         continue;
      }
#endif
      if ((groups[i]->pic == nonPIC && !paramOn("nonpic")) ||
          (groups[i]->pic == PIC && !paramOn("pic")))
      {
         groups[i]->disabled = true;
         continue;
      }

      if ((strcmp(groups[i]->platmode, "DUAL") == 0 && !paramOn("dual")) ||
          (strcmp(groups[i]->platmode, "VN") == 0 && !paramOn("vn")) ||
          (strcmp(groups[i]->platmode, "SMP") == 0 && !paramOn("smp"))) 
      {
         groups[i]->disabled = true;
         continue;
      }

      for (unsigned j=0; j<groups[i]->tests.size(); j++) 
      {
         if (!test_list.empty() && !testListContains(groups[i]->tests[j], test_list)) 
         {
            groups[i]->tests[j]->disabled = true;
         }
      }
   }

   if (unique_id && max_unique_id && testLimit) {
      unsigned cur_test = 0;
      unsigned cur_test_limitgroup = 0;
      for (unsigned i=0; i < groups.size(); i++) 
      {
         if (groups[i]->disabled) continue;
         for (unsigned j=0; j<groups[i]->tests.size(); j++) 
         {
            if (groups[i]->tests[j]->disabled) {
               continue;
            }
            if (cur_test && (cur_test % testLimit == 0)) {
               cur_test_limitgroup++;
            }
            cur_test++;

            if (cur_test_limitgroup % max_unique_id != (unique_id-1)) {
               groups[i]->tests[j]->disabled = true;
            }
         }       
      }
   }
   else if (unique_id && max_unique_id && groupLimit) {
      unsigned cur_group = 0;
      unsigned cur_limitgroup = 0;
      for (unsigned i=0; i < groups.size(); i++) 
      {
         if (groups[i]->disabled) continue;
         if (cur_group && (cur_group % groupLimit == 0)) {
            cur_limitgroup++;
         }
         cur_group++;

         if (cur_limitgroup % max_unique_id != (unique_id-1)) {
            groups[i]->disabled = true;
         }
      }
   }

   for (unsigned  i = 0; i < groups.size(); i++) 
   {
      if (groups[i]->disabled)
         continue;
      if (groups[i]->mutatee && groups[i]->mutatee[0] && !fileExists(groups[i]->mutatee)) {
         groups[i]->disabled = true;
      }
   }

   parse_resumelog(groups);

   if (testLimit) {
      int test_run = 0;
      for (unsigned  i = 0; i < groups.size(); i++) 
      {
         if (groups[i]->disabled)
            continue;
         for (unsigned j=0; j<groups[i]->tests.size(); j++) 
         {
            if (groups[i]->tests[j]->disabled)
               continue;
            if (test_run < testLimit) {
               test_run++;
               continue;
            }

            if (!groups[i]->tests[j]->disabled) {
               groups[i]->tests[j]->disabled = true;
               groups[i]->tests[j]->limit_disabled = true;
               limitedTests = true;
               if (next_resume_group == -1) {
                  next_resume_group = i;
                  next_resume_test = j;
               }
            }
         }      
      }
   }
   else if (groupLimit) {
      int groups_run = 0;
      for (unsigned  i = 0; i < groups.size(); i++) 
      {
         if (groups[i]->disabled)
            continue;
         if (groups_run < groupLimit) {
            groups_run++;
            continue;
         }

         if (!groups[i]->disabled) {
            for (unsigned j=0; j<groups[i]->tests.size(); j++) 
            {
               if (!groups[i]->tests[j]->disabled) {
                  groups[i]->tests[j]->disabled = true;
                  groups[i]->tests[j]->limit_disabled = true;
                  limitedTests = true;
                  if (next_resume_group == -1) {
                     next_resume_group = i;
                     next_resume_test = j;
                  }
               }
               groups[i]->disabled = true;
            }
         }
      }
   }
   if (given_mutator != -1) {
      for (unsigned  i = 0; i < groups.size(); i++) {
         if (i != given_mutator && !groups[i]->disabled) {
            for (unsigned j=0; j<groups[i]->tests.size(); j++) 
            {
               if (!groups[i]->tests[j]->disabled) {
                  groups[i]->tests[j]->disabled = true;
                  groups[i]->tests[j]->limit_disabled = true;
                  limitedTests = true;
               }
               groups[i]->disabled = true;
            }
         }
      }
   }
   for (unsigned  i = 0; i < groups.size(); i++) {
      if (groups[i]->disabled)
         continue;
      groups[i]->disabled = true;
      if (groups[i]->modname != std::string("")) {
         for (unsigned j=0; j<groups[i]->tests.size(); j++) {
            if (!groups[i]->tests[j]->disabled)
               groups[i]->disabled = false;
         }
      }
   }
}

static void setIndexes(std::vector<RunGroup *> groups)
{
   for (unsigned  i = 0; i < groups.size(); i++) {
      groups[i]->index = i;
      for (unsigned j=0; j<groups[i]->tests.size(); j++) {
         groups[i]->tests[j]->index = j;
         groups[i]->tests[j]->group_index = i;
      }
   }
}   


// Performs a wildcard string match on Unix-like systems, and a standard string
// match on Windows
// Returns true for match found, false for no match
static bool nameMatches(const char *wcname, const char *tomatch) {
#if defined(os_windows_test)
   // Sadly, we can't assume the presence of fnmatch on Windows
   return (strcmp(wcname, tomatch) == 0);
#else
   // The other systems we support are unix-based and should provide fnmatch (?)
   return (fnmatch(wcname, tomatch, 0) == 0);
#endif
}

// Returns true if the vector mutatee_list contains the string mutatee, and
// returns false if it does not
static bool mutateeListContains(std::vector<char *> mutatee_list, const char *mutatee) {
   if (NULL == mutatee) {
      return false;
   }
   for (size_t i = 0; i < mutatee_list.size(); i++) {
      if (nameMatches(mutatee_list[i], mutatee)) {
         return true;
      }
   }
   return false;
}

// Runs through all the test names in testsn, and enables any matching tests
// in testsv.  If any tests matched, returns true.  If there were no matching
// tests, returns false
// Okay, we don't actually enable any tests here; we just disable tests that
// don't match.  All tests start out enabled, and we previously disabled any
// that are crashing while we were parsing the resume log.
static bool testListContains(TestInfo * test, std::vector<char *> &testsn) {
   bool match_found = false;

   for (size_t i = 0; i < testsn.size(); i++) {
      if (nameMatches(testsn[i], test->name))
         return true;
   }
   return false;
}
 
#if !defined(os_windows_test)
#include <sys/types.h>
#include <unistd.h>
#endif

#include <sys/stat.h>
static bool fileExists(std::string f)
{
   struct stat data;
   int result = stat(f.c_str(), &data);

   return (result == 0);
}

