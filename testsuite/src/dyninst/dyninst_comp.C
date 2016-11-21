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

//#define COMPLIB_DLL_BUILD

#include <string>

#include "comptester.h"
#include "ParameterDict.h"
#include "Callbacks.h"

#include "BPatch.h"
#include "BPatch_process.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_point.h"
#include "test_lib.h"
#include "ResumeLog.h"
#include "dyninst_comp.h"
#include "MutateeStart.h"

using namespace std;

class DyninstComponent : public ComponentTester
{
private:
   BPatch *bpatch;
   char *libRTname;
   char *libRTname_m_abi;
   std::string err_msg;

   ParamPtr bpatch_ptr;
   ParamPtr bp_appThread;
   ParamPtr bp_appAddrSpace;
   ParamPtr bp_appProc;
   ParamPtr bp_appBinEdit;

   ParamInt is_xlc;
   BPatch_thread *appThread;
   BPatch_addressSpace *appAddrSpace;
   BPatch_process *appProc;
   BPatch_binaryEdit *appBinEdit;

public:
   DyninstComponent();
   virtual test_results_t program_setup(ParameterDict &params);
   virtual test_results_t program_teardown(ParameterDict &params);
   virtual test_results_t group_setup(RunGroup *group, ParameterDict &params);
   virtual test_results_t group_teardown(RunGroup *group, ParameterDict &params);
   virtual test_results_t test_setup(TestInfo *test, ParameterDict &parms);
   virtual test_results_t test_teardown(TestInfo *test, ParameterDict &parms);

   virtual std::string getLastErrorMsg();

   virtual ~DyninstComponent();
};

bool isMutateeMABI32(const char *name);
bool isMutateeXLC(const char *name);

DyninstComponent::DyninstComponent() :
   bpatch(NULL),
   libRTname(NULL),
   libRTname_m_abi(NULL),
   bp_appThread(NULL),
   is_xlc(0),
   appThread(NULL)
{
}

test_results_t DyninstComponent::program_setup(ParameterDict &params)
{
   if (measure) um_program.start();  // Measure resource usage.

   bpatch = new BPatch();
   if (!bpatch)
      return FAILED;
   bpatch_ptr.setPtr(bpatch);
   params["bpatch"] = &bpatch_ptr;
   setBPatch(bpatch);

   bpatch->registerErrorCallback(errorFunc);

   if (measure) um_program.end();  // Measure resource usage.

   ParamInt *debugprint = dynamic_cast<ParamInt *>(params["debugPrint"]);
   if (debugprint) {
      setDebugPrint(debugprint->getInt());
   }
   
   if ( getenv("DYNINSTAPI_RT_LIB") )
   {
      char *temp = getenv("DYNINSTAPI_RT_LIB");
      libRTname = strdup(temp);
   } else {
      getOutput()->log(STDERR, "Environment variable DYNINSTAPI_RT_LIB undefined:\n"
                       "  set it to the full pathname of libdyninstAPI_RT\n");
      return FAILED;
   }

#if defined(m_abi)
   if ( getenv("DYNINSTAPI_RT_LIB_MABI") ) {
      char *temp = getenv("DYNINSTAPI_RT_LIB_MABI");
      libRTname_m_abi = strdup(temp);
   } else {
      getOutput()->log(STDERR, "Warning: Environment variable DYNINSTAPI_RT_LIB_MABI undefined:\n"
                       "32 bit mutatees will not run\n");
   }
#endif

   return PASSED;
}

test_results_t DyninstComponent::program_teardown(ParameterDict &params)
{
   delete bpatch;
   bpatch = NULL;

   return PASSED;
}

test_results_t DyninstComponent::group_setup(RunGroup *group, 
                                             ParameterDict &params)
{
#if defined(m_abi)
   if (isMutateeMABI32(group->mutatee)) {
      if (NULL == libRTname_m_abi) {
         return -1;
      }
      setenv("DYNINSTAPI_RT_LIB", libRTname_m_abi, 1);
   } else {
      setenv("DYNINSTAPI_RT_LIB", libRTname, 1);
   }
#endif

   appThread = NULL;
   appProc = NULL;
   appAddrSpace = NULL;
   appBinEdit = NULL;
   char *mutatee_resumelog = params["mutatee_resumelog"]->getString();
   clear_mutateelog(mutatee_resumelog);

   is_xlc.setInt((int) isMutateeXLC(group->mutatee));
   params["mutateeXLC"] = &is_xlc;

   if (!group->mutatee || group->state == SELFSTART)
      return PASSED;
   if (measure) um_group.start(); // Measure resource usage.
   
   switch (group->createmode) {
      case CREATE:
      {
         std::string exec_name;
         std::vector<std::string> args;

         getMutateeParams(group, params, exec_name, args);
         char **argv = getCParams(string(""), args);
         appProc = BPatch::bpatch->processCreate(exec_name.c_str(), (const char**) argv, NULL);
         free(argv);
         if (!appProc) {
            logerror("Error creating process\n");
            return FAILED;
         }
         break;
      }
      case USEATTACH:
      {
         Dyninst::PID pid = getMutateePid(group);
         if (pid == NULL_PID) {
            std::string mutateeString = launchMutatee(group, params);
            if (mutateeString == string("")) {
               logerror("Error creating attach process\n");
               return FAILED;
            }
            registerMutatee(mutateeString);
            pid = getMutateePid(group);
         }
         assert(pid != NULL_PID);

         appProc = BPatch::bpatch->processAttach(group->mutatee, pid);
         if (!appProc) {
            logerror("Error attaching to process\n");
            return FAILED;
         }
         break;
      }
      case DISK:
      {
         appBinEdit = BPatch::bpatch->openBinary(group->mutatee, true);
         if (!appBinEdit) {
            logerror("Error opening binary for rewriting\n");
            return FAILED;
         }
         break;
      }
      case DESERIALIZE:
      {
         assert(0); //Don't know how to handle this;
         break;
      }
   }
   

   if (appProc) {
      BPatch_Vector<BPatch_thread *> thrds;
      appProc->getThreads(thrds);
      appThread = thrds[0];
      appAddrSpace = (BPatch_addressSpace *) appProc;
   }
   else if (appBinEdit) {
      appAddrSpace = (BPatch_addressSpace *) appBinEdit;
   }

   if (group->state == RUNNING && appProc) 
   {
      appProc->continueExecution();
   }

   if (measure) um_group.end(); // Measure resource usage.
   
   bp_appThread.setPtr(appThread);
   params["appThread"] = &bp_appThread;
   
   bp_appAddrSpace.setPtr(appAddrSpace);
   params["appAddrSpace"] = &bp_appAddrSpace;
   
   bp_appProc.setPtr(appProc);
   params["appProcess"] = &bp_appProc;
   
   bp_appBinEdit.setPtr(appBinEdit);
   params["appBinaryEdit"] = &bp_appBinEdit;

   return PASSED;
}

test_results_t DyninstComponent::group_teardown(RunGroup *group,
                                                ParameterDict &params)
{

    if (group->customExecution) {
        // We don't care about pass/fail here but we most definitely care about mutatee cleanup.
        // Just kill the process...
        if(appProc)
        {
            //fprintf(stderr, "killing mutatee proc\n");
            appProc->terminateExecution();
        }      
        return PASSED;
    }
   bool someTestPassed = false;

   for (unsigned i=0; i<group->tests.size(); i++)
   {
      if (shouldRunTest(group, group->tests[i]))
      {
         someTestPassed = true;
      }
   }
   char *mutatee_resumelog = params["mutatee_resumelog"]->getString();

   if (group->createmode == DISK) {
      if (!someTestPassed)
         return FAILED;

      test_results_t test_result;
      runBinaryTest(group, params, test_result);
      return test_result;
   }

   if (!someTestPassed && appThread != NULL)
   {
      // All tests failed or skipped in executeTest(), so I didn't execute the
      // mutatee.  I should kill it so it doesn't run in attach mode
      appProc->terminateExecution();
      return FAILED;
   }
   if (appThread == NULL) {
       // I saw this happen during a broken resumeLog cleanup, so handle
       // it as an error... - bernat, 12JAN09
       return FAILED;
   }

   do {
       //fprintf(stderr, "continuing mutatee...\n");
      appProc->continueExecution();
      bpatch->waitForStatusChange();
   } while (appProc && !appProc->isTerminated());

   if (appProc->terminationStatus() == ExitedNormally &&
       appProc->getExitCode() == 0)
   {
      return PASSED;
   }

   bool mutateeExitedViaSignal = false;
   if(appProc->terminationStatus() == ExitedViaSignal) {
      mutateeExitedViaSignal = true;
      int signalNum = appProc->getExitSignal();
      getOutput()->log(LOGINFO, "Mutatee exited from signal 0x%x\n", signalNum);
   }
   else {
       int exitCode = appProc->getExitCode();
      getOutput()->log(LOGINFO, "Mutatee exit code 0x%x\n", exitCode);
   }

   parse_mutateelog(group, mutatee_resumelog);

   return UNKNOWN;
}

test_results_t DyninstComponent::test_setup(TestInfo *test, ParameterDict &parms)
{
   return PASSED;
}

test_results_t DyninstComponent::test_teardown(TestInfo *test, ParameterDict &parms)
{
    // Take care of the things the test can delete out from under us
    DyninstMutator* theMutator = dynamic_cast<DyninstMutator*>(test->mutator);
    if(theMutator->appThread == NULL) appThread = NULL;
    if(theMutator->appProc == NULL) appProc = NULL;
   return PASSED;
}

bool isNameExt(const char *name, const char *ext, int ext_len)
{
   int name_len = strlen(name);

   // Can't match
   if ( name_len < ext_len )
   {
      return false;
   }

   // If the last 4 characters match _xlc or _xlC
   // return true
   if ( strcmp(name + name_len - ext_len, ext) == 0 )
   {
      return true;
   } else
   {
      return false;
   }
}

bool isMutateeMABI32(const char *name)
{
   return (name != NULL) && isNameExt(name, "_m32", 4);
}

bool isMutateeXLC(const char *name)
{
   if (!name)
      return false;
   return isNameExt(name, "_xlc", 4) || isNameExt(name, "_xlC", 4);
}

std::string DyninstComponent::getLastErrorMsg()
{
   return err_msg;
}

extern "C" {
   COMPLIB_DLL_EXPORT ComponentTester *componentTesterFactory();
}

COMPLIB_DLL_EXPORT ComponentTester *componentTesterFactory() {
   return new DyninstComponent();
}

DyninstComponent::~DyninstComponent()
{
}

// All the constructor does is set the instance fields to NULL
DyninstMutator::DyninstMutator() :
    appThread(NULL),
	appAddrSpace(NULL),
	appBinEdit(NULL),
	appProc(NULL),
    appImage(NULL)
{
}

DyninstMutator::~DyninstMutator() {
}

// Standard setup; this does the setup for the "simple" class of tests.
// "Simple" tests are tests that only perform any processing in the executeTest
// stage.  Most just do some lookup in the mutatee and then optionally insert
// instrumentation and let the mutatee do its thing.

test_results_t DyninstMutator::setup(ParameterDict &param) 
{
  runmode = (create_mode_t) param["createmode"]->getInt();

  bool createmode = param["createmode"]->getInt() == USEATTACH;

  if (param["appThread"] == NULL)
  {
	  logerror("No app thread found.  Check test groups.\n");
	  return FAILED;
  }

  appThread = (BPatch_thread *)(param["appThread"]->getPtr());
  appProc = (BPatch_process *)(param["appProcess"]->getPtr());
  appBinEdit = (BPatch_binaryEdit *)(param["appBinaryEdit"]->getPtr());
  appAddrSpace = (BPatch_addressSpace *)(param["appAddrSpace"]->getPtr());

  // Read the program's image and get an associated image object
  appImage = appAddrSpace->getImage();

  if ( createmode ) 
  {
	  if ( ! signalAttached(appImage) )
	  {
		  return FAILED;
	  }
  }


  return PASSED;
}


int expectError = DYNINST_NO_ERROR;

//
// Wait for the mutatee to stop.
//
int waitUntilStopped(BPatch *bpatch, BPatch_process *appProc, int testnum,
                      const char *testname)
{
    while (!appProc->isStopped() && !appProc->isTerminated()) {
        bpatch->waitForStatusChange();
  }

  if (!appProc->isStopped()) {
        logerror("**Failed test #%d (%s)\n", testnum, testname);
        logerror("    process did not signal mutator via stop\n");
        logerror("thread is not stopped\n");
        return -1;
    }
#if defined(os_windows_test) //ccw 10 apr 2001
    else if (appProc->stopSignal() != EXCEPTION_BREAKPOINT && appProc->stopSignal() != -1) {
	logerror("**Failed test #%d (%s)\n", testnum, testname);
	logerror("    process stopped on signal %d, not SIGTRAP\n", 
                 appProc->stopSignal());
        return -1;
    }
#else
    else if ((appProc->stopSignal() != SIGSTOP) &&
	     (appProc->stopSignal() != SIGBUS) &&
	     (appProc->stopSignal() != SIGHUP)) {
	logerror("**Failed test #%d (%s)\n", testnum, testname);
 	logerror("    process stopped on signal %d, not SIGSTOP\n", 
                 appProc->stopSignal());
        return -1;
    }
#endif
    return 0;
}


//
// Signal the child that we've attached.  The child contains a function
// "checkIfAttached" which simply returns the value of the global variable
// "isAttached."  We add instrumentation to "checkIfAttached" to set
// "isAttached" to 1.
//
bool signalAttached(BPatch_image *appImage)
{
    BPatch_variableExpr *isAttached = appImage->findVariable("isAttached");
    if (isAttached == NULL) {
	logerror("*ERROR*: unable to start tests because variable \"isAttached\""
               " could not be found in the child process\n");
        return false;
    }

    int yes = 1;
    isAttached->writeValue(&yes);
    return true;
}

void checkCost(BPatch_snippet snippet)
{
   float cost;
   BPatch_snippet copy;

   // test copy constructor too.
   copy = snippet;

   cost = snippet.getCost();
   dprintf("Snippet cost=%g\n", cost);
    if (cost < 0.0) {
        fprintf(stderr, "*Error*: negative snippet cost\n");
    } else if (cost > 0.01) {
        fprintf(stderr, "*Error*: snippet cost of %f, exceeds max expected of 0.1",
            cost);
    }
}

// Wrapper function to find variables
// For Fortran, will look for lowercase variable, if mixed case not found
BPatch_variableExpr *findVariable(BPatch_image *appImage, const char* var,
                                  BPatch_Vector <BPatch_point *> *point = NULL)
{
  //BPatch_variableExpr *FortVar = NULL;
    int mutateeFortran = isMutateeFortran(appImage);
    BPatch_variableExpr *ret = NULL;
    int i, numchars = strlen (var) + 1; // + 1 corrects for null-termination
    char *lowercase = new char [numchars];
    int temp = expectError;

    if (mutateeFortran && point) {
            strncpy (lowercase, var, numchars);
            expectError = 100;
            for (i = 0; i < numchars; i++)
                lowercase [i] = tolower (lowercase [i]);
            ret = appImage->findVariable (*(*point) [0], lowercase);
        if (!ret) {
            expectError = temp;
            ret = appImage->findVariable (*(*point) [0], var);
        }
    } else {
        ret = appImage->findVariable (var);
    }

    expectError = temp;
    delete [] lowercase;
    return ret;
}

int functionNameMatch(const char *gotName, const char *targetName) 
{
    if (!strcmp(gotName, targetName)) return 0;

    if (!strncmp(gotName, targetName, strlen(targetName)) &&
	(strlen(targetName) == strlen(gotName)-1) &&
	(gotName[strlen(targetName)] == '_'))
	return 0;

    return 1;
}

//
// Replace all calls in "inFunction" to "callTo" with calls to "replacement."
// If "replacement" is NULL, them use removeFunctionCall instead of
// replaceFunctionCall.
// Returns the number of replacements that were performed.
//
// TODO Fix error messages and refactor out the testNo parameter.  It's
// unnecessary in the new test suite structure.
int replaceFunctionCalls(BPatch_addressSpace *appAddrSpace, BPatch_image *appImage,
                         const char *inFunction, const char *callTo, 
                         const char *replacement, int testNo, 
                         const char *testName, int callsExpected = -1)
{
   int numReplaced = 0;

   BPatch_Vector<BPatch_function *> found_funcs;
   if ((NULL == appImage->findFunction(inFunction, found_funcs)) || 
       !found_funcs.size()) 
   {
      logerror("**Failed** test #%d (%s)\n", testNo, testName);
      logerror("    Unable to find function %s\n",
               inFunction);
      return -1;
   }

   if (1 < found_funcs.size()) {
      logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
               __FILE__, __LINE__, found_funcs.size(), inFunction);
   }

   BPatch_Vector<BPatch_point *> *points = found_funcs[0]->findPoint(BPatch_subroutine);

   if (!points || (!points->size() )) {
      logerror("**Failed** test #%d (%s)\n", testNo, testName);
      logerror("    %s[%d]: Unable to find point in %s - subroutine calls: pts = %p\n",
               __FILE__, __LINE__, inFunction,points);
      return -1;
   }

   BPatch_function *call_replacement = NULL;
   if (replacement != NULL) {
      
      BPatch_Vector<BPatch_function *> bpfv;
      /*
       * Include `uninstrumentable' functions here because function
       * call replacement works regardless of whether the target is
       * instrumentable or not. This is required to support targets
       * in static libraries, where all code is uinstrumentable by
       * default.
       */
      if (NULL == appImage->findFunction(replacement, bpfv,true,true,true) || !bpfv.size()
          || NULL == bpfv[0]){
         logerror("**Failed** test #%d (%s)\n", testNo, testName);
         logerror("    Unable to find function %s\n", replacement);
         return -1;
      }
      call_replacement = bpfv[0];
   }

   for (unsigned int n = 0; n < points->size(); n++) {
      BPatch_function *func;

      if ((func = (*points)[n]->getCalledFunction()) == NULL) continue;

      char fn[256];
      if (func->getName(fn, 256) == NULL) {
         logerror("**Failed** test #%d (%s)\n", testNo, testName);
         logerror("    Can't get name of called function in %s\n",
                  inFunction);
         return -1;
      }
      if (functionNameMatch(fn, callTo) == 0) {
         if (replacement == NULL)
            appAddrSpace->removeFunctionCall(*((*points)[n]));
         else {
            assert(call_replacement);
            appAddrSpace->replaceFunctionCall(*((*points)[n]),
                                           *call_replacement);
         }
         numReplaced++;
      }
   }

   if (callsExpected > 0 && callsExpected != numReplaced) {
      logerror("**Failed** test #%d (%s)\n", testNo, testName);
      logerror("    Expected to find %d %s to %s in %s, found %d\n",
               callsExpected, callsExpected == 1 ? "call" : "calls",
               callTo, inFunction, numReplaced);
      return -1;
   }


   return numReplaced;
}

//
// Return a pointer to a string identifying a BPatch_procedureLocation
//
const char *locationName(BPatch_procedureLocation l)
{
    switch(l) {
      case BPatch_entry:
	return "entry";
      case BPatch_exit:
	return "exit";
      case BPatch_subroutine:
	return "call points";
      case BPatch_longJump:
	return "long jump";
      case BPatch_allLocations:
	return "all";
      default:
	return "<invalid BPatch_procedureLocation>";
    };
}

//
// Insert "snippet" at the location "loc" in the function "inFunction."
// Returns the value returned by BPatch_thread::insertSnippet.
//
BPatchSnippetHandle *insertSnippetAt(BPatch_addressSpace *appAddrSpace,
                               BPatch_image *appImage, const char *inFunction, 
                               BPatch_procedureLocation loc, 
                               BPatch_snippet &snippet,
                               int testNo, const char *testName)
{
    // Find the point(s) we'll be instrumenting

    BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction(inFunction, found_funcs)) || !found_funcs.size()) {
      logerror("**Failed** test #%d (%s)\n", testNo, testName);
      logerror("    Unable to find function %s\n",
	      inFunction);
      return NULL;
    }

    if (1 < found_funcs.size()) {
      logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), inFunction);
    }

    BPatch_Vector<BPatch_point *> *points = found_funcs[0]->findPoint(loc);

    if (!points) {
	logerror("**Failed** test #%d (%s)\n", testNo, testName);
	logerror("    Unable to find point %s - %s\n",
		inFunction, locationName(loc));
        return NULL;
    }

    checkCost(snippet);
    return appAddrSpace->insertSnippet(snippet, *points);
}

//
// Create a snippet that calls the function "funcName" with no arguments
//
BPatch_snippet *makeCallSnippet(BPatch_image *appImage, const char *funcName,
                                int testNo, const char *testName)
{
  BPatch_Vector<BPatch_function *> bpfv;
  if (NULL == appImage->findFunction(funcName, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    logerror("**Failed** test #%d (%s)\n", testNo, testName);
    logerror("    Unable to find function %s\n", funcName);
    return NULL;
  }
  BPatch_function *call_func = bpfv[0];
 
    BPatch_Vector<BPatch_snippet *> nullArgs;
    BPatch_snippet *ret = new BPatch_funcCallExpr(*call_func, nullArgs);

    if (ret == NULL) {
	logerror("**Failed** test #%d (%s)\n", testNo, testName);
	logerror("    Unable to create snippet to call %s\n", funcName);
        return NULL;
    }

    return ret;
}


//
// Insert a snippet to call function "funcName" with no arguments into the
// procedure "inFunction" at the points given by "loc."
//
int insertCallSnippetAt(BPatch_addressSpace *appAddrSpace,
                            BPatch_image *appImage, const char *inFunction,
                            BPatch_procedureLocation loc, const char *funcName,
                            int testNo, const char *testName)
{
    BPatch_snippet *call_expr =
       makeCallSnippet(appImage, funcName, testNo, testName);
    RETURNONNULL(call_expr);

    BPatchSnippetHandle *ret = insertSnippetAt(appAddrSpace, appImage,
					       inFunction, loc, *call_expr,
					       testNo, testName);
    if (ret == NULL) {
	logerror("**Failed** test #%d (%s)\n", testNo, testName);
	logerror("    Unable to insert snippet to call function %s\n",
		funcName);
        return -1;
    }

    delete call_expr;

    return 0;
}

BPatch_Vector<BPatch_snippet *> genLongExpr(BPatch_arithExpr *tail)
{
    BPatch_Vector<BPatch_snippet *> ret;
    
    for (int i=0; i < 1000; i++) {
        ret.push_back(tail);
    }
    return ret;
}

// Build Architecture specific libname
// FIXME Is this used any more?  Is it necessary?
void addLibArchExt(char *dest, unsigned int dest_max_len, int psize)
{
   int dest_len;

   dest_len = strlen(dest);

   // Patch up alternate ABI filenames
#if defined(rs6000_ibm_aix64_test)
   if(psize == 4) {
     strncat(dest, "_32", dest_max_len - dest_len);
     dest_len += 3;
   }
#endif

#if defined(arch_x86_64_test)
   if (psize == 4) {
      strncat(dest,"_m32", dest_max_len - dest_len);
      dest_len += 4;   
   }
#endif

#if defined(os_windows_test)
   strncat(dest, ".dll", dest_max_len - dest_len);
   dest_len += 4;
#else
   strncat(dest, ".so", dest_max_len - dest_len);
   dest_len += 3;
#endif
}

int pointerSize(BPatch_image *img) {
#if defined(mips_sgi_irix6_4_test) || defined(arch_x86_64_test)
   BPatch_variableExpr *pointerSizeVar = img->findVariable("pointerSize");

   if (!pointerSizeVar) {
      logerror("**Failed** test #2 (four parameter function)\n");
      logerror("    Unable to locate variable pointerSize\n");
      return -1;
   }

   int pointerSize;
   if (!pointerSizeVar->readValue(&pointerSize)) {
      logerror("**Failed** test #2 (four parameter function)\n");
      logerror("    Unable to read value of variable pointerSize\n");
      return -1;
   }

   return pointerSize;
#else
   return sizeof(void*);
#endif
}

typedef BPatch_Vector<BPatch_point * > point_vector;

void instrument_entry_points( BPatch_addressSpace * app_thread,
		BPatch_image * ,
		BPatch_function * func,
		BPatch_snippet * code )
{
	assert( func != 0 );
	assert( code != 0 );

	//   handle_vector * list_of_handles = new handle_vector;

	int null_entry_point_count = 0;
	int failed_snippet_insertion_count = 0;

	point_vector * entries = func->findPoint( BPatch_entry );
	assert( entries != 0 );

	for( unsigned int i = 0; i < entries->size(); i++ )
	{
		BPatch_point * point = ( * entries )[ i ];
		if( point == 0 )
		{
			null_entry_point_count++;
		}
		else
		{
			BPatchSnippetHandle * result =
				app_thread->insertSnippet( * code,
						* point, BPatch_callBefore, BPatch_firstSnippet );
			if( result == 0 )
			{
				failed_snippet_insertion_count++;
			}
		}
	}

	delete code;
}

void instrument_exit_points( BPatch_addressSpace * app_thread,
		BPatch_image * ,
		BPatch_function * func,
		BPatch_snippet * code )
{
	assert( func != 0 );
	assert( code != 0 );

	//   handle_vector * list_of_handles = new handle_vector;

	int null_exit_point_count = 0;
	int failed_snippet_insertion_count = 0;

	point_vector * exits = func->findPoint( BPatch_exit );
	assert( exits != 0 );

	for( unsigned int i = 0; i < exits->size(); i++ )
	{
		BPatch_point * point = ( * exits )[ i ];
		if( point == 0 )
		{
			null_exit_point_count++;
		}
		else
		{
			BPatchSnippetHandle * result =
				app_thread->insertSnippet( * code,
						* point, BPatch_callAfter, BPatch_firstSnippet );
			if( result == 0 )
			{
				failed_snippet_insertion_count++;
			}
		}
	}

	delete code;
}

// NOTE: What are the benefits of this over appThread->terminateProcess?
void killMutatee(BPatch_process *appProc)
{
    int pid = appProc->getPid();

    appProc->terminateExecution();
    dprintf("Mutatee process %d killed.\n", pid);
    return;
}

// Tests to see if the mutatee has defined the mutateeCplusplus flag
int isMutateeCxx(BPatch_image *appImage) 
{
	// determine whether mutatee is C or C++
	BPatch_variableExpr *isCxx = appImage->findVariable("mutateeCplusplus");
	if (isCxx == NULL) {
		return 0;
	} else {
		int mutateeCplusplus;
		isCxx->readValue(&mutateeCplusplus);
		dprintf("Mutatee is %s.\n", mutateeCplusplus ? "C++" : "C");
		return mutateeCplusplus;
	}
}
// Tests to see if the mutatee has defined the mutateeFortran flag
int isMutateeFortran(BPatch_image *appImage) {
	// determine whether mutatee is Fortran
	BPatch_variableExpr *isF = appImage->findVariable("mutateeFortran");
	if (isF == NULL) {
		return 0;
	} else {
		int mutateeFortran;
		isF->readValue(&mutateeFortran);
		dprintf("Mutatee is %s.\n", mutateeFortran ? "Fortran" : "C/C++");
		return mutateeFortran;
	}

}

// Tests to see if the mutatee has defined the mutateeF77 flag
int isMutateeF77(BPatch_image *appImage) {
	// determine whether mutatee is F77
	BPatch_variableExpr *isF77 = appImage->findVariable("mutateeF77");
	if (isF77 == NULL) {
		return 0;
	} else {
		int mutateeF77;
		isF77->readValue(&mutateeF77);
		dprintf("Mutatee is %s.\n", mutateeF77 ? "F77" : "not F77");
		return mutateeF77;
	}
}


void MopUpMutatees(const int mutatees, BPatch_process *appProc[])
{
  int n=0;
  dprintf("MopUpMutatees(%d)\n", mutatees);
  for (n=0; n<mutatees; n++) {
    if (appProc[n]) {
      if (appProc[n]->terminateExecution()) {
	assert(appProc[n]->terminationStatus() == ExitedViaSignal);
	int signalNum = appProc[n]->getExitSignal();
	dprintf("Mutatee terminated from signal 0x%x\n", signalNum);
      } else {
	fprintf(stderr, "Failed to mop up mutatee %d (pid=%d)!\n",
		n, appProc[n]->getPid());
      }
    } else {
      fprintf(stderr, "Mutatee %d already terminated?\n", n);
    }
  }
  dprintf("MopUpMutatees(%d) done\n", mutatees);
}

TEST_DLL_EXPORT void contAndWaitForAllProcs(BPatch *bpatch, BPatch_process *appProc,
		BPatch_process **myprocs, int *threadCount)
{

    dprintf("Proc %d is pointer %p\n", *threadCount, appProc);
	myprocs[(*threadCount)++] = appProc;
        appProc->continueExecution();

	while (1) {
		int i;
		dprintf("Checking %d threads for terminated status\n", *threadCount);
		for (i=0; i < *threadCount; i++) {
                    if (!myprocs[i]->isTerminated()) {
				dprintf("Thread %d is not terminated\n", i);
				break;
			}
		}

		// see if all exited
		if (i== *threadCount) {
			dprintf("All threads terminated\n");
			break;
		}

		bpatch->waitForStatusChange();

		for (i=0; i < *threadCount; i++) {
                    if (myprocs[i]->isStopped()) {
				dprintf("Thread %d marked stopped, continuing\n", i);
                                myprocs[i]->continueExecution();
			}
		}
	}

	*threadCount = 0;
}

/*
 * Given a string variable name and an expected value, lookup the varaible
 *    in the child process, and verify that the value matches.
 *
 */
bool verifyChildMemory(BPatch_process *appThread,
		const char *name, int expectedVal)
{
	BPatch_image *appImage = appThread->getImage();

	if (!appImage) {
		logerror("unable to locate image for %d\n", appThread->getPid());
		return false;
	}

	BPatch_variableExpr *var = appImage->findVariable(name);
	if (!var) {
		logerror("unable to located variable %s in child\n", name);
		return false;
	}

	int actualVal;
	var->readValue(&actualVal);

	if (expectedVal != actualVal) {
		logerror("*** for %s, expected val = %d, but actual was %d\n",
				name, expectedVal, actualVal);
		return false;
	} else {
		logstatus("verified %s was = %d\n", name, actualVal);
		return true;
	}
}


void dumpvect(BPatch_Vector<BPatch_point*>* res, const char* msg)
{
  if(!debugPrint())
		return;

	printf("%s: %ld\n", msg, res->size());
	for(unsigned int i=0; i<res->size(); ++i) {
		BPatch_point *bpp = (*res)[i];
		const BPatch_memoryAccess* ma = bpp->getMemoryAccess();
		const BPatch_addrSpec_NP& as = ma->getStartAddr_NP();
		const BPatch_countSpec_NP& cs = ma->getByteCount_NP();
		if(ma->getNumberOfAccesses() == 1) {
			if(ma->isConditional_NP())
				printf("%s[%d]: @[r%d+r%d<<%d+%ld] #[r%d+r%d+%ld] ?[%X]\n", msg, i+1,
						as.getReg(0), as.getReg(1), as.getScale(), as.getImm(),
						cs.getReg(0), cs.getReg(1), cs.getImm(), ma->conditionCode_NP());
			else
				printf("%s[%d]: @[r%d+r%d<<%d+%ld] #[r%d+r%d+%ld]\n", msg, i+1,
						as.getReg(0), as.getReg(1), as.getScale(), as.getImm(),
						cs.getReg(0), cs.getReg(1), cs.getImm());
		}
		else {
			const BPatch_addrSpec_NP& as2 = ma->getStartAddr_NP(1);
			const BPatch_countSpec_NP& cs2 = ma->getByteCount_NP(1);
			printf("%s[%d]: @[r%d+r%d<<%d+%ld] #[r%d+r%d+%ld] && "
					"@[r%d+r%d<<%d+%ld] #[r%d+r%d+%ld]\n", msg, i+1,
					as.getReg(0), as.getReg(1), as.getScale(), as.getImm(),
					cs.getReg(0), cs.getReg(1), cs.getImm(),
					as2.getReg(0), as2.getReg(1), as2.getScale(), as2.getImm(),
					cs2.getReg(0), cs2.getReg(1), cs2.getImm());
		}
	}
}

static inline void dumpxpct(const BPatch_memoryAccess* exp[], unsigned int size, const char* msg)
{
  //	if(!debugPrint())
  //	return;

	printf("%s: %d\n", msg, size);

	for(unsigned int i=0; i<size; ++i) {
		const BPatch_memoryAccess* ma = exp[i];
		if(!ma)
			continue;
		const BPatch_addrSpec_NP& as = ma->getStartAddr_NP();
		const BPatch_countSpec_NP& cs = ma->getByteCount_NP();
		if(ma->getNumberOfAccesses() == 1)
			printf("%s[%d]: @[r%d+r%d<<%d+%ld] #[r%d+r%d+%ld]\n", msg, i+1,
					as.getReg(0), as.getReg(1), as.getScale(), as.getImm(),
					cs.getReg(0), cs.getReg(1), cs.getImm());
		else {
			const BPatch_addrSpec_NP& as2 = ma->getStartAddr_NP(1);
			const BPatch_countSpec_NP& cs2 = ma->getByteCount_NP(1);
			printf("%s[%d]: @[r%d+r%d<<%d+%ld] #[r%d+r%d+%ld] && "
					"@[r%d+r%d<<%d+%ld] #[r%d+r%d+%ld]\n", msg, i+1,
					as.getReg(0), as.getReg(1), as.getScale(), as.getImm(),
					cs.getReg(0), cs.getReg(1), cs.getImm(),
					as2.getReg(0), as2.getReg(1), as2.getScale(), as2.getImm(),
					cs2.getReg(0), cs2.getReg(1), cs2.getImm());
		}
	}
}
bool validate(BPatch_Vector<BPatch_point*>* res,
		BPatch_memoryAccess* acc[], const char* msg)
{
	bool ok = true;

	for(unsigned int i=0; i<res->size(); ++i) {
		if (acc[i] != NULL) {
			BPatch_point* bpoint = (*res)[i];
                        const BPatch_memoryAccess* expected_ma = acc[i];
                        const BPatch_memoryAccess* actual_ma = bpoint->getMemoryAccess();
                        ok = (ok && actual_ma->equals(expected_ma));
			if(!ok) {
				logerror("Validation failed at %s #%d.\n", msg, i+1);
                                dumpxpct(&expected_ma, 1, "Expected");
                                dumpxpct(&actual_ma, 1, "Actual");
                                return ok;
			}
		}
	}
	return ok;
}

BPatch_callWhen instrumentWhere(  const BPatch_memoryAccess* memAccess){

	BPatch_callWhen whenToCall;
	if(memAccess != NULL){
		if(memAccess->hasALoad()){
			whenToCall = BPatch_callBefore;
		}else if(memAccess->hasAStore()){
			whenToCall = BPatch_callAfter;
		}else if(memAccess->hasAPrefetch_NP() ){
			whenToCall = BPatch_callBefore;
		}else{
			whenToCall = BPatch_callBefore;
		}
	}else{
		whenToCall = BPatch_callBefore;
	}
	return whenToCall;
}

int instCall(BPatch_addressSpace* as, const char* fname,
		const BPatch_Vector<BPatch_point*>* res)
{
	char buf[256];
	BPatch_callWhen whenToCall = BPatch_callBefore;

	snprintf(buf, 256, "count%s", fname);

	BPatch_Vector<BPatch_snippet*> callArgs;
	BPatch_image *appImage = as->getImage();

	BPatch_Vector<BPatch_function *> bpfv;
	if (NULL == appImage->findFunction(buf, bpfv) || !bpfv.size()
			|| NULL == bpfv[0]){
		logerror("    Unable to find function %s\n", buf);
		return -1;
	}
	BPatch_function *countXXXFunc = bpfv[0];  

	BPatch_funcCallExpr countXXXCall(*countXXXFunc, callArgs);

	for(unsigned int i=0;i<(*res).size();i++){

#if defined(os_aix_test)
		const BPatch_memoryAccess* memAccess;
		memAccess = (*res)[i]->getMemoryAccess() ;

		whenToCall = instrumentWhere( memAccess);

#endif
		as->insertSnippet(countXXXCall, *((*res)[i]),whenToCall);
	}

	return 0;
}

int instEffAddr(BPatch_addressSpace* as, const char* fname,
		const BPatch_Vector<BPatch_point*>* res,
		bool conditional)
{
	char buf[30];
	snprintf(buf, 30, "list%s%s", fname, (conditional ? "CC" : ""));
	dprintf("CALLING: %s\n", buf);

	//BPatch_Vector<BPatch_snippet*> listArgs;
	//BPatch_effectiveAddressExpr eae;
        //listArgs.push_back(&eae);

	BPatch_image *appImage = as->getImage();

	BPatch_Vector<BPatch_function *> bpfv;
	if (NULL == appImage->findFunction(buf, bpfv) || !bpfv.size()
			|| NULL == bpfv[0]){
		logerror("    Unable to find function %s\n", buf);
		return -1;
	}
	BPatch_function *listXXXFunc = bpfv[0];  


	BPatch_callWhen whenToCall = BPatch_callBefore;
	for(unsigned int i=0;i<(*res).size();i++){
#if defined(os_aix_test)
		const BPatch_memoryAccess* memAccess;

		memAccess = (*res)[i]->getMemoryAccess() ;

		whenToCall = instrumentWhere( memAccess);
#endif
                BPatch_Vector<BPatch_snippet*> listArgs;
                BPatch_effectiveAddressExpr eae;
                BPatch_constExpr insn_str((*res)[i]->getInsnAtPoint()->format().c_str());
                listArgs.push_back(&insn_str);
                listArgs.push_back(&eae);
                BPatch_funcCallExpr listXXXCall(*listXXXFunc, listArgs);
		
                if(!conditional)
			as->insertSnippet(listXXXCall, *((*res)[i]), whenToCall, BPatch_lastSnippet);
		else {
			BPatch_ifMachineConditionExpr listXXXCallCC(listXXXCall);
			as->insertSnippet(listXXXCallCC, *((*res)[i]), whenToCall, BPatch_lastSnippet);
		}
	}

#if defined(i386_unknown_linux2_0_test) \
	|| defined(x86_64_unknown_linux2_4_test) /* Blind duplication - Ray */ \
	|| defined(i386_unknown_nt4_0_test) \
        || defined(amd64_unknown_freebsd7_0_test) \
        || defined(i386_unknown_freebsd7_0_test)
	BPatch_effectiveAddressExpr eae2(1);
	const BPatch_Vector<BPatch_point*>* res2 = BPatch_memoryAccess::filterPoints(*res, 2);

        if(!conditional) {
            for(unsigned int i = 0; i < (*res2).size(); i++)
            {
                BPatch_Vector<BPatch_snippet*> listArgs2;
                BPatch_constExpr insn_str2((*res2)[i]->getInsnAtPoint()->format().c_str());
                listArgs2.push_back(&insn_str2);
                listArgs2.push_back(&eae2);
                BPatch_funcCallExpr listXXXCall2(*listXXXFunc, listArgs2);
                as->insertSnippet(listXXXCall2, *((*res2)[i]), BPatch_lastSnippet);
            }

        }
        else {
            for(int i = 0; i < (*res2).size(); i++)
            {
                BPatch_Vector<BPatch_snippet*> listArgs2;
                std::string insn = (*res2)[i]->getInsnAtPoint()->format();
                BPatch_constExpr insn_str2(insn.c_str());
                listArgs2.push_back(&insn_str2);
                listArgs2.push_back(&eae2);
                BPatch_funcCallExpr listXXXCall2(*listXXXFunc, listArgs2);
                BPatch_ifMachineConditionExpr listXXXCallCC2(listXXXCall2);
                as->insertSnippet(listXXXCallCC2, *((*res2)[i]), BPatch_lastSnippet);
            }
        }
#endif

	return 0;
}


int instByteCnt(BPatch_addressSpace* as, const char* fname,
		const BPatch_Vector<BPatch_point*>* res,
		bool conditional)
{
	char buf[30];
	snprintf(buf, 30, "list%s%s", fname, (conditional ? "CC" : ""));
	dprintf("CALLING: %s\n", buf);


	BPatch_image *appImage = as->getImage();

	BPatch_Vector<BPatch_function *> bpfv;
	if (NULL == appImage->findFunction(buf, bpfv) || !bpfv.size()
			|| NULL == bpfv[0]){
		logerror("    Unable to find function %s\n", buf);
		return -1;
	}
	BPatch_function *listXXXFunc = bpfv[0];  

	BPatch_callWhen whenToCall = BPatch_callBefore;

	for(unsigned int i=0;i<(*res).size();i++){
            BPatch_Vector<BPatch_snippet*> listArgs;

#if defined(os_aix_test)
		const BPatch_memoryAccess* memAccess;
		memAccess = (*res)[i]->getMemoryAccess() ;

		whenToCall = instrumentWhere( memAccess);

#endif
                BPatch_bytesAccessedExpr bae;
                std::string insn = (*res)[i]->getInsnAtPoint()->format();
                BPatch_constExpr insn_str(insn.c_str());
                listArgs.push_back(&insn_str);
                listArgs.push_back(&bae);
                BPatch_funcCallExpr listXXXCall(*listXXXFunc, listArgs);
		if(!conditional)
			as->insertSnippet(listXXXCall, *((*res)[i]), whenToCall, BPatch_lastSnippet);
		else {
			BPatch_ifMachineConditionExpr listXXXCallCC(listXXXCall);
			as->insertSnippet(listXXXCallCC, *((*res)[i]), whenToCall, BPatch_lastSnippet);
		}
	}

#if defined(i386_unknown_linux2_0_test) \
	|| defined(x86_64_unknown_linux2_4_test) /* Blind duplication - Ray */ \
	|| defined(i386_unknown_nt4_0_test) \
        || defined(amd64_unknown_freebsd7_0_test) \
        || defined(i386_unknown_freebsd7_0_test)

        BPatch_bytesAccessedExpr bae2(1);
	const BPatch_Vector<BPatch_point*>* res2 = BPatch_memoryAccess::filterPoints(*res, 2);
	if(!conditional) {
            for(unsigned int i = 0; i < (*res2).size(); i++)
            {
                BPatch_Vector<BPatch_snippet*> listArgs2;
                std::string insn2 = (*res2)[i]->getInsnAtPoint()->format();
                BPatch_constExpr insn_str2(insn2.c_str());
                listArgs2.push_back(&insn_str2);
                listArgs2.push_back(&bae2);
                BPatch_funcCallExpr listXXXCall2(*listXXXFunc, listArgs2);
                as->insertSnippet(listXXXCall2, *((*res2)[i]), BPatch_lastSnippet);
            }

        }
	else {
            for(unsigned int i = 0; i < (*res2).size(); i++)
            {
                BPatch_Vector<BPatch_snippet*> listArgs2;
                std::string insn = (*res2)[i]->getInsnAtPoint()->format();
                BPatch_constExpr insn_str2(insn.c_str());
                listArgs2.push_back(&insn_str2);
                listArgs2.push_back(&bae2);
                BPatch_funcCallExpr listXXXCall2(*listXXXFunc, listArgs2);
                BPatch_ifMachineConditionExpr listXXXCallCC2(listXXXCall2);
                as->insertSnippet(listXXXCallCC2, *((*res2)[i]), BPatch_lastSnippet);
            }
	}
#endif

	return 0;
}

// From Test8
const char *frameTypeString(BPatch_frameType frameType)
{
	switch (frameType) {
		case BPatch_frameNormal:
			return "BPatch_frameNormal";
		case BPatch_frameSignal:
			return "BPatch_frameSignal";
		case BPatch_frameTrampoline:
			return "BPatch_frameTrampoline";
		default:
			break;
	};

	return "UNKNOWN";
}

bool hasExtraUnderscores(const char *str)
{
	assert( str );
	int len = strlen(str) - 1;
	return (str[0] == '_' || str[len] == '_');
}

/* WARNING: This function is not thread safe. */
const char *fixUnderscores(const char *str)
{
	static char buf[256];

	assert( str );
	assert( strlen(str) < sizeof(buf) );

	while (*str == '_') ++str;
	strncpy(buf, str, 256);

	char *ptr = buf + strlen(buf) - 1;
	while (ptr > buf && *ptr == '_') *(ptr--) = 0;

	return buf;
}

bool checkStack(BPatch_thread *appThread,
		const frameInfo_t correct_frame_info[],
		unsigned num_correct_names,
		int test_num, const char *test_name)
{
	unsigned i, j;

	const int name_max = 256;
	bool failed = false;

	BPatch_Vector<BPatch_frame> stack;
	appThread->getCallStack(stack);

	if (1) {
		dprintf("Stack in test %d (%s):\n", test_num, test_name);
		for( unsigned i = 0; i < stack.size(); i++) {
			char name[name_max];
			BPatch_function *func = stack[i].findFunction();
			if (func == NULL)
				strcpy(name, "[UNKNOWN]");
			else
				func->getName(name, name_max);
			dprintf("  %10p: %s, fp = %p, type %s\n",
					stack[i].getPC(),
					name,
					stack[i].getFP(),
					frameTypeString(stack[i].getFrameType()));

		}
		dprintf("End of stack dump.\n");
	}

	if (stack.size() < num_correct_names) {
		logerror("**Failed** test %d (%s)\n", test_num, test_name);
		logerror("    Stack trace should contain more frames.\n");
		failed = true;
	}

	for (i = 0, j = 0; i < num_correct_names; i++, j++) {
#if !defined(i386_unknown_nt4_0_test)
		if (stack.size() && j < stack.size()-1 && stack[j].getFP() == 0) {
			logerror("**Failed** test %d (%s)\n", test_num, test_name);
			logerror("    A stack frame other than the lowest has a null FP.\n");
			failed = true;
			break;
		}
#endif

		if (stack.size() >= j)
			break;
		if (correct_frame_info[i].valid) {
			char name[name_max], name2[name_max];

			BPatch_function *func = stack[j].findFunction();
			if (func != NULL)
				func->getName(name, name_max);

			BPatch_function *func2 =
                                appThread->getProcess()->findFunctionByAddr(stack[j].getPC());
			if (func2 != NULL)
				func2->getName(name2, name_max);

			if ((func == NULL && func2 != NULL) ||
					(func != NULL && func2 == NULL)) {
				logerror("**Failed** test %d (%s)\n", test_num, test_name);
				logerror("    frame->findFunction() disagrees with thread->findFunctionByAddr()\n");
				logerror("    frame->findFunction() returns %s\n",
						name);
				logerror("    thread->findFunctionByAddr() return %s\n",
						name2);
				failed = true;
				break;
			} else if (func!=NULL && func2!=NULL && strcmp(name, name2)!=0) {
				logerror("**Failed** test %d (%s)\n", test_num, test_name);
				logerror("    BPatch_frame::findFunction disagrees with BPatch_thread::findFunctionByAddr\n");
				failed = true;
				break;
			}

			if (correct_frame_info[i].type != stack[j].getFrameType()) {
				logerror("**Failed** test %d (%s)\n", test_num, test_name);
				logerror("    Stack frame #%d has wrong type, is %s, should be %s\n", i+1, frameTypeString(stack[i].getFrameType()), frameTypeString(correct_frame_info[i].type));
				logerror("    Stack frame 0x%lx, 0x%lx\n", stack[i].getPC(), stack[i].getFP() );
				failed = true;
				break;
			}

			if (stack[j].getFrameType() == BPatch_frameSignal ||
					stack[j].getFrameType() == BPatch_frameTrampoline) {
				// No further checking for these types right now
			} else {
				if (func == NULL) {
					logerror("**Failed** test %d (%s)\n",
							test_num, test_name);
					logerror("    Stack frame #%d refers to an unknown function, should refer to %s\n", j+1, correct_frame_info[i].function_name);
					failed = true;
					break;
				} else { /* func != NULL */
					if (!hasExtraUnderscores(correct_frame_info[i].function_name))
						strncpy(name, fixUnderscores(name), name_max);

					if (strcmp(name, correct_frame_info[i].function_name) != 0) {
						if (correct_frame_info[i].optional) {
							j--;
							continue;
						}
						logerror("**Failed** test %d (%s)\n", test_num, test_name);
						logerror("    Stack frame #%d refers to function %s, should be %s\n", j+1, name, correct_frame_info[i].function_name);
						failed = true;
						break;
					}
				}
			}
		}
	}

	return !failed;
}

/* End Test8 Specific */

/* Begin Test9 Specific */
void buildArgs(const char** child_argv, char *pathname, int testNo){
	int n=0;

	child_argv[n++] = pathname;
	if (debugPrint()){
		child_argv[n++] = const_cast<char*>("-verbose");
	}
	child_argv[n++] = const_cast<char*>("-orig"); 

	child_argv[n++] = const_cast<char*>("-run");
	char str[16];
	snprintf(str, 16, "test_stw_%d",testNo);
	child_argv[n++] = strdup(str);

	child_argv[n] = NULL;

}

int instrumentToCallZeroArg(BPatch_process *appThread, BPatch_image *appImage, char *instrumentee,
                            char*patch, int testNo, char *testName)
{

	BPatch_Vector<BPatch_function *> found_funcs;
	if ((NULL == appImage->findFunction(instrumentee, found_funcs)) || !found_funcs.size()) {
		logerror("    Unable to find function %s\n",instrumentee);
		return -1;
	}

	if (1 < found_funcs.size()) {
		logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
				__FILE__, __LINE__, found_funcs.size(), instrumentee);
	}

	BPatch_Vector<BPatch_point *> *point1_1 = found_funcs[0]->findPoint(BPatch_entry);


	if (!point1_1 || ((*point1_1).size() == 0)) {
		logerror("**Failed** test #%d (%s)\n", testNo,testName);
		logerror("    Unable to find entry point to \"%s.\"\n",instrumentee);
		return -1;
	}

	BPatch_Vector<BPatch_function *> bpfv;
	if (NULL == appImage->findFunction(patch, bpfv) || !bpfv.size()
			|| NULL == bpfv[0]){
		logerror("**Failed** test #%d (%s)\n", testNo, testName);
		logerror("    Unable to find function %s\n", patch);
		return -1;
	}
	BPatch_function *call1_func = bpfv[0];

	BPatch_Vector<BPatch_snippet *> call1_args;
	BPatch_funcCallExpr call1Expr(*call1_func, call1_args);

	dprintf("Inserted snippet2\n");
	checkCost(call1Expr);
	appThread->insertSnippet(call1Expr, *point1_1);

	return 0;
}

int letOriginalMutateeFinish(BPatch_process *appThread){
	/* finish original mutatee to see if it runs */

	/*fprintf(stderr,"\n************************\n");	
	  fprintf(stderr,"Running the original mutatee\n\n");*/
	appThread->continueExecution();

	while( !appThread->isTerminated());

	int retVal;

	if(appThread->terminationStatus() == ExitedNormally) {
		retVal = appThread->getExitCode();
	} else if(appThread->terminationStatus() == ExitedViaSignal) {
		int signalNum = appThread->getExitSignal();
		if (signalNum){
			logerror("Mutatee exited from signal 0x%x\n", signalNum);
		}
		retVal = signalNum;
	}


	//	fprintf(stderr,"Original mutatee has terminated\n\************************\n\n");

	return retVal;
}

// Begin Test12 Library functions

BPatch_function *findFunction(const char *fname, BPatch_image *appImage, int testno, const char *testname)
{
	BPatch_Vector<BPatch_function *> bpfv;
	if (NULL == appImage->findFunction(fname, bpfv) || (bpfv.size() != 1)) {

		logerror("**Failed test #%d (%s)\n", testno, testname);
		logerror("  Expected 1 functions matching %s, got %d\n",
				fname, bpfv.size());
		return NULL;
	}
	return bpfv[0];
}

BPatch_function *findFunction(const char *fname, BPatch_module *inmod, int testno, const char *testname)
{
	BPatch_Vector<BPatch_function *> bpfv;
	if (NULL == inmod->findFunction(fname, bpfv) || (bpfv.size() != 1)) {

		logerror("**Failed test #%d (%s)\n", testno, testname);
		logerror("  Expected 1 functions matching %s, got %d\n",
				fname, bpfv.size());
		return NULL;
	}
	return bpfv[0];
}

// Internal Function for setVar and getVar
void dumpVars(BPatch_image *appImage)
{
	BPatch_Vector<BPatch_variableExpr *> vars;
	appImage->getVariables(vars);
	for (unsigned int i = 0; i < vars.size(); ++i) {
		fprintf(stderr, "\t%s\n", vars[i]->getName());
	}
}

bool setVar(BPatch_image *appImage, const char *vname, void *addr, int testno, const char *testname)
{
	BPatch_variableExpr *v;
	int addr_size = appImage->getProcess()->getAddressWidth();
	void *buf = addr;
	if (NULL == (v = appImage->findVariable(vname))) {
		logerror("**Failed test #%d (%s)\n", testno, testname);
		logerror("  cannot find variable %s, avail vars:\n", vname);
		dumpVars(appImage);
		return false;
	}

	if (! v->writeValue(buf, addr_size, true)) {
		logerror("**Failed test #%d (%s)\n", testno, testname);
		logerror("  failed to write call site var to mutatee\n");
		return false;
	}

	return true;
}

bool getVar(BPatch_image *appImage, const char *vname, void *addr, int testno, const char *testname)
{
	BPatch_variableExpr *v;
	int addr_size = appImage->getProcess()->getAddressWidth();
	if (NULL == (v = appImage->findVariable(vname))) {
		logerror("**Failed test #%d (%s)\n", testno, testname);
		logerror("  cannot find variable %s: avail vars:\n", vname);
		dumpVars(appImage);
		return false;
	}

	if (! v->readValue(addr, addr_size)) {
		logerror("**Failed test #%d (%s)\n", testno, testname);
		logerror("  failed to read var in mutatee\n");
		return false;
	}

	return true;
}


// End Test12 Library functions
