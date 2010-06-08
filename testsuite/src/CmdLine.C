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

#include <cassert>
#include <vector>
#include <string>

#include "CmdLine.h"
#include "ResumeLog.h"
#include "test_info_new.h"
#include "error.h"

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

#define END       0
#define COMPILERS (1 << 1)
#define RUNMODES  (1 << 2)
#define OPTLEVELS (1 << 3)
#define COMPS     (1 << 4)
#define ABI       (1 << 5)
#define THRDMODE  (1 << 6)
#define PROCMODE  (1 << 7)
#define LINKMODE  (1 << 8)
//If you add more modes here, then update the lists in "-all" and "-full" parsing below

ModeGroup mode_args[] = {
   { "gcc",         COMPILERS, defaultOff },
   { "g++",         COMPILERS, defaultOn  },
   { "gfortran",    COMPILERS, defaultOn  },
   { "icc",         COMPILERS, defaultOff },
   { "icpc",        COMPILERS, defaultOff },
   { "pgcc",        COMPILERS, defaultOff },
   { "pgCC",        COMPILERS, defaultOff },
   { "cc",          COMPILERS, defaultOff },
   { "CC",          COMPILERS, defaultOff },
   { "cxx",         COMPILERS, defaultOff },
   { "VC",          COMPILERS, defaultOff },
   { "VC++",        COMPILERS, defaultOn  },
   { "suncc",       COMPILERS, defaultOff },
   { "xlc",         COMPILERS, defaultOff },
   { "xlC",         COMPILERS, defaultOff },
   { "ibmas",       COMPILERS, defaultOff },
   { "masm",        COMPILERS, defaultOff },
   { "nasm",        COMPILERS, defaultOff },
   { "nocompiler",  COMPILERS, defaultOn  },
   { "create",      RUNMODES,  defaultOn  },
   { "attach",      RUNMODES,  defaultOff },
   { "rewriter",    RUNMODES,  defaultOff },
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
   { "st"           THRDMODE,  defaultOn  },
   { "mt"           THRDMODE,  defaultOff },
   { "sp"           PROCMODE,  defaultOn  },
   { "mp"           PROCMODE,  defaultOff },
   { "dynamiclink", LINKMODE,  defaultOff },
   { "staticlink",  LINKMODE,  defaultOff },
   { NULL,          NONE,      defaultOff } };

static std::vector<char *> mutatee_list;
static std::vector<char *> test_list; 

static bool useHumanLog = true;
static char *humanlog_name = "-";
static bool shouldDebugBreak = false;
static bool called_from_runTests = false;
static bool printMutateeLogHeader = false;
static bool measureMEMCPU = false;
static char *measureFileName = "-";
static bool limitSkippedTests = false;
static int limitResumeGroup = -1;
static int limitResumeTest = -1;
static bool noclean = false;
static int testLimit = 0;
static bool debugPrint = false;
static int unique_id = 0;
static int max_unique_id = 0;
static char *logfilename = DEFAULT_LOGNAME;

static std::vector<RunGroup *> groups;             

int parseArgs(int argc, char *argv[], ParameterDict &params)
{
   //Parse args
   int result = handleArgs(argc, argv);
   if (result)
      return result;

   //Initialize and enable tests according to options
   initialize_mutatees(group_list);
   sortGroups(group_list);
   disableUnwantedTests(group_list);
   setIndexes(groups);   

   //Initialize dictionary parameters from args
   setupDictionary(params); //TODO here
}

static int handleArgs(int argc, char *argv[])
{
   for (unsigned i=1; i < argc; i++ )
   {
      if ( strcmp(argv[i], "-test") == 0)
      {
         char *tests;
         char *name;

         runAllTests = false;
         if ( i + 1 >= argc )
         {
            getOutput()->log(STDERR, "-test must be followed by a testname\n");
            return NOTESTS;
         }

         tests = strdup(argv[++i]);

         name = strtok(tests, ","); // FIXME Use strtok_r()
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
         unsigned int j;
         runAllTests = false;
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

         runAllMutatees = false;
         if ( i + 1 >= argc )
         {
            getOutput()->log(STDERR, "-mutatee must be followed by mutatee names\n");
            return NOTESTS;
         }

         mutatees = strdup(argv[++i]);

         name = strtok(mutatees, ","); // FIXME Use strtok_r()
         if (NULL == name) {
            // Special handling for a "" mutatee specified on the command line
            mutatee_list.push_back("");
         } else {
            mutatee_list.push_back(name);
         }
         while ( name != NULL )
         {
            name = strtok(NULL, ","); // FIXME Use strtok_r()
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
         setAllOn(COMPILERS || RUNMODES || COMPS || ABI || THRDMODE || PROCMODE || LINKMODE, false);
      }
      else if (strcmp(argv[i], "-full") == 0)
      {
         //Like -all, but with full optimization levels
         setAllOn(COMPILERS || OPTLEVELS || RUNMODES || COMPS || ABI || THRDMODE || PROCMODE || LINKMODE, false);
      }
      else if (strcmp(argv[i], "-allcomp") == 0)
      {
         setAllOn(COMPS, true);
      }
      else if (strcmp(argv[i], "-allopt") == 0)
      {
         setAllOn(OPTLEVELS, true);
      }
      else if ( strcmp(argv[i], "-noclean") == 0 )
      {
         noclean = true;
      }
      else if ((strcmp(argv[i], "-cpumem") == 0) || (strcmp(argv[i], "-memcpu") == 0))
      {
         measureMEMCPU = true;
         if (i+1 < argc)
         {
            if (argv[i+1][0] != '-')
            {
               i++;
               measureFileName = argv[i];
            }
            else if (argv[i+1][1] == '\0')
            {
               i++;
               measureFileName = "-";
            }
         }
      }
      else if (strncmp(argv[i], "-verbose", 2) == 0)
      {
         debugPrint = true;
      }
      else if ( strcmp(argv[i], "-log")==0)
      {
         logfilename = "-";
         if ((i + 1) < argc) {
            if ((argv[i + 1][0] != '-') || (argv[i + 1][1] == '\0')) {
               i += 1;
               logfilename = argv[i];
            }
         }
      }
      else if ( strcmp(argv[i], "-logfile") == 0) 
      {
         getOutput()->log(STDERR, "WARNING: -logfile is a deprecated option; use -log instead\n");
         /* Store the log file name */
         if ((i + 1) >= argc) {
            getOutput()->log(STDERR, "Missing log file name\n");
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
      else if ( strcmp(argv[i], "-limit") == 0 ) 
      {
         if ( i + 1 >= argc ) {
            getOutput()->log(STDERR, "-limit must be followed by an integer limit\n");
            return NOTESTS;
         }
         testLimit = strtol(argv[++i], NULL, 10);
         if ((0 == testLimit) && (EINVAL == errno)) {
            getOutput()->log(STDERR, "-limit must be followed by an integer limit\n");
            return NOTESTS;
         }
      }
      else if (strcmp(argv[i], "-unique") == 0) 
      {
         if (i + 1 < argc) {
            unique_id = atoi(argv[++i]);
         }
         if (!unique_id) {
            getOutput()->log(STDERR, "-unique must be followed by a non-zero integer\n");
            return NOTESTS;
         }
      }
      else if (strcmp(argv[i], "-max-unique") == 0) 
      {
         if (i + 1 < argc) {
            max_unique_id = atoi(argv[++i]);
         }
         if (!max_unique_id) {
            getOutput()->log(STDERR, "-max_unique must be followed by a non-zero integer\n");
            return NOTESTS;
         }
      }
      else if ( strcmp(argv[i], "-humanlog") == 0 ) {
         // Verify that the following argument exists
         if ( i + 1 >= argc )
         {
            getOutput()->log(STDERR, "-humanlog must by followed by a filename\n");
            return NOTESTS;
         }

         useHumanLog = true;
         humanlog_name = argv[++i];
      } 
      else if (strcmp(argv[i], "-under-runtests") == 0) 
      {
         called_from_runTests = true;
      }
      else if ((strcmp(argv[i], "-help") == 0) ||
               (strcmp(argv[i], "--help") == 0)) 
      {
         print_help();
         exit(-5);
      }
      else if (strcmp(argv[i], "-dboutput") == 0) {
         char * failedOutputFile = NULL;
         //check if a failed output file is specified
         if ((i + 1) < argc) {
            if (argv[i+1][0] != '-' || argv[i+1][1] == '\0') {
               //either it doesn't start with - or it's exactly -
               i++;
               failedOutputFile = argv[i];
            }
         }

         if (NULL == failedOutputFile) {
            //TODO insert proper value
            time_t rawtime;
            struct tm * timeinfo = (tm *)malloc(sizeof(struct tm));

            time(&rawtime);
            timeinfo = localtime(&rawtime);

            failedOutputFile = (char*)malloc(sizeof(char) * strlen("sql_dblog-xxxx-xx-xx0"));
            if (failedOutputFile == NULL) {
               fprintf(stderr, "[%s:%u] - Out of memory!\n", __FILE__, __LINE__);
               // TODO Handle error;
            }
            sprintf(failedOutputFile, "sql_dblog-%4d-%02d-%02d",
                    timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);

            getOutput()->log(STDERR, "No 'SQL log file' found, using default %s\n", failedOutputFile);
         }

         std::string s_failedOutputFile(failedOutputFile);
         TestOutputDriver *newoutput = loadOutputDriver("DatabaseOutputDriver", &s_failedOutputFile);

         //make sure it loaded correctly before replacing default output
         if (newoutput != NULL) {
            setOutput(newoutput);
         }
      }
   }

   
   return 0;
}

static bool setAllOn(int groups, bool force)
{
   for (unsigned i=0; mode_args[i].option != NULL; i++) {
      if (!(groups & mode_args[i].group))
         continue;
      if (force && mode_args[i].mode == defaultOff) {
         mode_args[i].mode = defaultOn;
      }
      else if (!force && (mode_args[i].mode == defaultOff ||
                          mode_args[i].mode == explicitOff)) {
         mode_args[i].mode = explicitOn;
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
      if (strcmp(option, param) != 0) 
         continue;
      found_param = true;
      mode_args[i].mode = explicitOn;
      break;
   }
   if (!found_param)
      return false;

   /**
    * All parameters in the same group, which aren't explicitOn, are
    * set to defaultOn
    **/
   int group = mode_args[i].group;
   for (i=0; mode_args[i].option != NULL; i++) 
   {
      if (mode_args[i].group != group)
         continue;
      if (mode_args[i].mode == defaultOn || mode_args[i] == defaultOff)
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
}

struct groupcmp 
{
   bool operator()(const RunGroup* lv, const RunGroup* rv)
   {
      int mod_cmp = strcmp(lv->mod ? lv->mod->name : "", rv->mod ? rv->mod->name : "");
      if (mod_cmp)
         return mod_cmp == -1;
      
      int mutatee_cmp = strcmp(lv->mutatee ? lv->mutatee : "", rv->mutatee ? rv->mutatee : "");
      if (mutatee_cmp)
         return mutatee_cmp == -1;
      
      if (lv->createmode != rv->createmode)
         return ((int) lv->createmode) < ((int) rv->createmode);

      if (lv->threadmode != rv->threadmode)
         return ((int) lv->threadmode) != ((int) rv->threadmode);
      
      if (lv->procmode != rv->procmode)
         return ((int) lv->procmode) < ((int) rv->procmode);
      
      return false;
   }
};

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
      if (((groups[i]->useAttach == CREATE) && !paramOn("create")) ||
          ((groups[i]->useAttach == USEATTACH) && !paramOn("attach")) ||
          ((groups[i]->useAttach == DISK) && !paramOn("rewriter")) ||
          ((groups[i]->useAttach == DESERIALIZE) && !paramOn("serialize")))
      {
         groups[i]->disabled = true;
         continue;
      }
      //Component
      if (!groups[i]->mod || 
          (groups[i]->mod->name == std::string("dyninst") && !paramOn("dyninst")) ||
          (groups[i]->mod->name == std::string("symtab") && !paramOn("symtab")) ||
          (groups[i]->mod->name == std::string("instruction") && !paramOn("instruction")) ||
          (groups[i]->mod->name == std::string("proccontrol") && !paramOn("proccontrol")))
      {
         groups[i]->disabled = true;
         continue;
      }
      //Mutatee list
      if (!mutateeListContains(mutatee_list, groups[i]->mutatee))
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
      if ((groups[i]->threadmode == MultiProcess && !paramOn("mp")) ||
          (groups[i]->threadmode == SingleProcess && !paramOn("sp")))
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
      for (unsigned j=0; j<groups[i]->tests.size(); j++) 
      {
         if (!testListContains(groups[i]->tests[j], test_list)) 
         {
            groups[i]->tests[j]->disabled = true;
         }
      }      
   }

   if (unique_id && max_unique_id) {
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
            if (cur_test && cur_test % testLimit == 0) {
               cur_test_limitgroup++;
            }
            cur_test++;

            if (cur_test_limitgroup % max_unique_id != (unique_id-1)) {
               groups[i]->tests[j]->disabled = true;
            }
         }       
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

            groups[i]->tests[j]->disabled = true;
            if (!limitSkippedTests) {
               limitSkippedTests = true;
               limitResumeGroup = i;
               limitResumeTest = j;
            }
         }      
      }
   }

   for (unsigned  i = 0; i < groups.size(); i++) {
      if (groups[i]->disabled)
         continue;
      groups[i]->disabled = true;
      if (groups[i]->mod) {
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
      }
   }
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
