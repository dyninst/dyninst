/*
 * Copyright (c) 1996-2011 Barton P. Miller
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

// $Id: test9.C,v 1.24 2008/04/11 23:30:39 legendre Exp $
//
// libdyninst validation suite test #9
//    Author: Chadd Williams (30 jun 2003) 
//        derived from a previous test by Bryan Buck & Jeff Hollingsworth
//

//  This program tests the save the world (dumpPatchedImage) features
//	of DyninstAPI.
//	The mutatee that goes with this file is test9.mutatee.c
//	
//  Naming conventions:
//      All functions, variables, etc are name funcXX_YY, exprXX_YY, etc.
//          XX is the test number
//          YY is the instance withing the test
//	    func1_2 is the second function used in test case #1.
//

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>
#include <stdarg.h>
#include <dirent.h>

#if defined(i386_unknown_linux2_0) \
 || defined(sparc_sun_solaris2_4)
#include <sys/types.h>
#include <sys/wait.h>
#endif

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "test_util.h"
#include "test1.h"

#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#define TEST1 "1"
#define TEST2 "2"
#define TEST3 "3"
#define TEST4 "4"
#define TEST5 "5"
#define TEST6 "6"
#define TEST7 "7"

int debugPrint = 0; // internal "mutator" tracing
int errorPrint = 0; // external "dyninst" tracing (via errorFunc)

bool forceRelocation = false; // force relocation of functions

int mutateeCplusplus = 0;
int mutateeFortran = 0;
int mutateeF77 = 0;
bool runAllTests = true;
const unsigned int MAX_TEST = 7;
bool runTest[MAX_TEST+1];
bool passedTest[MAX_TEST+1];

char *savedDirectories[MAX_TEST+1];

template class BPatch_Vector<BPatch_variableExpr*>;
template class BPatch_Set<int>;

BPatch *bpatch;

static const char *mutateeNameRoot = "test9.mutatee";

// control debug printf statements
#define dprintf	if (debugPrint) printf

/**************************************************************************
 * Error callback
 **************************************************************************/

#define DYNINST_NO_ERROR -1

int expectError = DYNINST_NO_ERROR;

void errorFunc(BPatchErrorLevel level, int num, const char * const *params)
{
    if (num == 0) {
        // conditional reporting of warnings and informational messages
        if (errorPrint) {
            if (level == BPatchInfo)
              { if (errorPrint > 1)fprintf(stderr,"%s\n", params[0]); }
            else
               fprintf(stderr,"%s", params[0]);
        }
    } else {
        // reporting of actual errors
        char line[256];
        const char *msg = bpatch->getEnglishErrorString(num);
        bpatch->formatErrorString(line, sizeof(line), msg, params);
        
        if (num != expectError) {
	  if(num != 112)
	   fprintf(stderr,"Error #%d (level %d): %s\n", num, level, line);
        
            // We consider some errors fatal.
            if (num == 101) {
               exit(-1);
            }
        }
    }
}

void createInstPointError(BPatchErrorLevel level, int num, const char **params)
{
    if (num != 117 && num != 118)
	errorFunc(level, num, params);
}

/**************************************************************************
 * Utility functions
 **************************************************************************/
// Wrapper function to find variables
// For Fortran, will look for lowercase variable, if mixed case not found
BPatch_variableExpr *findVariable(BPatch_image *appImage, const char* var,
                                  BPatch_Vector <BPatch_point *> *point = NULL)
{
  //BPatch_variableExpr *FortVar = NULL;
    BPatch_variableExpr *ret = NULL;
    int i, numchars = strlen (var);
    char *lowercase = new char [numchars];
    int temp = expectError;

    if (mutateeFortran && point) {
            strcpy (lowercase, var);
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

char* saveWorld(BPatch_thread *appThread){

	char *mutatedName = new char[strlen("test9_mutated") +1];
	memset(mutatedName, '\0',strlen("test9_mutated") +1);
	strcat(mutatedName, "test9_mutated");
   	char* dirName = appThread->dumpPatchedImage(mutatedName);
	if(!dirName){
		fprintf(stderr,"Error: No directory name returned\n");
	}

	return dirName;
}

/****** CHANGE PATH **********/
extern char**environ;

void changePath(char *path){

	char  *newPATH;

	newPATH= new char[strlen("PWD=") + strlen(path) +  1];
	newPATH[0] = '\0';
	strcpy(newPATH, "PWD=");
	strcat(newPATH,path); 

	for(int i=0;environ[i]!= '\0';i++){

		if( strstr(environ[i], "PWD=") ){
			environ[i] = newPATH;
		}
	}

}

/****************************/

int runMutatedBinary(char *path, char* fileName, char* testID){

   pid_t pid;
#if defined(sparc_sun_solaris2_4) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(i386_unknown_linux2_0) \
 || defined(rs6000_ibm_aix5_1)
   int status, died;
#endif
 	char *mutatedBinary;

#if defined(rs6000_ibm_aix4_1) \
 || defined(rs6000_ibm_aix5_1)
	char *aixBinary="dyninst_mutatedBinary";
#endif
	char *realfileName;

	realfileName = fileName;
#if defined(rs6000_ibm_aix4_1) \
 || defined(rs6000_ibm_aix5_1)
	realfileName = aixBinary;
#endif

	mutatedBinary= new char[strlen(path) + strlen(realfileName) + 1];

	memset(mutatedBinary, '\0', strlen(path) + strlen(realfileName) + 1);

	strcat(mutatedBinary, path);
	strcat(mutatedBinary, realfileName);

	switch((pid=fork())){
		case -1: 
		fprintf(stderr,"can't fork\n");
    	    		exit(-1);
		case 0 : 
			//child
			fprintf(stderr," running: %s %s %s\n", mutatedBinary, realfileName, testID);
#if defined(rs6000_ibm_aix5_1) \
 || defined(rs6000_ibm_aix4_1)
			changePath(path);
#endif

			execl(mutatedBinary, realfileName,"-run", testID, NULL); 
			fprintf(stderr,"ERROR!\n");
			perror("execl");
			exit(-1);

		default: 
			//parent
			delete [] mutatedBinary;
#if defined(sparc_sun_solaris2_4) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(i386_unknown_linux2_0) \
 || defined(rs6000_ibm_aix5_1)
			died= waitpid(pid, &status, 0); 
#endif
   	}
#if defined(sparc_sun_solaris2_4) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(i386_unknown_linux2_0) \
 || defined(rs6000_ibm_aix5_1)
	if(WIFEXITED(status)){
		int exitStatus = WEXITSTATUS(status);

		if(exitStatus == 0){
			return 1;
		}
	}else if(WIFSIGNALED(status)){
		fprintf(stderr," terminated with signal: %d \n", WTERMSIG(status));
	}
#endif
	return 0; 
	

}


static int preloadMutatedRT(const char *path)
{
    char *rt_lib_name = getenv("DYNINSTAPI_RT_LIB");
    if (rt_lib_name == 0) {
	fprintf(stderr, "preloadMutatedRT: DYNINSTAPI_RT_LIB is undefined\n");
	return (-1);
    }
    char *rt_lib_base = strrchr(rt_lib_name, '/');
    if (rt_lib_base == 0) {
	fprintf(stderr,
		"preloadMutatedRT: DYNINSTAPI_RT_LIB not a full path\n");
	return (-1);
    }
    const char *var_prefix = "LD_PRELOAD=";
    char *ld_preload = new char[strlen(var_prefix) + strlen(path) +
				strlen(rt_lib_base) + 1];
    strcpy(ld_preload, var_prefix);
    strcat(ld_preload, path);
    strcat(ld_preload, rt_lib_base); // starts with a slash
    if (putenv(ld_preload) < 0) {
	perror("preloadMutatedRT: putenv error");
	return (-1);
    }
    return 0;
}

int runMutatedBinaryLDLIBRARYPATH(char *path, char* fileName, char* testID){

   pid_t pid;
#if defined(sparc_sun_solaris2_4) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(i386_unknown_linux2_0) \
 || defined(rs6000_ibm_aix5_1)
   int status, died;
#endif

	char *mutatedBinary;
#if defined(rs6000_ibm_aix4_1) \
 || defined(rs6000_ibm_aix5_1)
	char *aixBinary="dyninst_mutatedBinary";
#endif
	char *realFileName;

	realFileName = fileName;
#if defined(rs6000_ibm_aix4_1) \
 || defined(rs6000_ibm_aix5_1)
	realFileName = aixBinary;
#endif
	char *currLDPATH, *newLDPATH;

	currLDPATH = getenv("LD_LIBRARY_PATH");
	newLDPATH= new char[strlen("LD_LIBRARY_PATH=") + strlen(currLDPATH) + strlen(":") +strlen(path) + 1];
	newLDPATH[0] = '\0';
	strcpy(newLDPATH, "LD_LIBRARY_PATH=");
	strcat(newLDPATH,path); 
	strcat(newLDPATH,":");
	strcat(newLDPATH, currLDPATH);

	mutatedBinary= new char[strlen(path) + strlen(realFileName) + 10];

	memset(mutatedBinary, '\0', strlen(path) + strlen(realFileName) + 10);

	strcat(mutatedBinary, path);
	strcat(mutatedBinary, realFileName);
	char *command = new char[strlen(mutatedBinary)+ strlen(realFileName) + strlen("-run") + strlen(testID)+10];
	sprintf(command,"%s -run %s", mutatedBinary, testID);

#if  defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4)
	int retVal =0;
#endif
	switch((pid=fork())){
		case -1: 
		fprintf(stderr,"can't fork\n");
    	    		exit(-1);
		case 0 : 
			//child
			fprintf(stderr," running: %s %s %s\n", mutatedBinary, realFileName, testID);

#if defined(rs6000_ibm_aix5_1) \
 || defined(rs6000_ibm_aix4_1)
			changePath(path);
#endif
			for(int i=0;environ[i]!= '\0';i++){

				if( strstr(environ[i], "LD_LIBRARY_PATH=") ){
					environ[i] = newLDPATH;
				}
			}
			if (preloadMutatedRT(path) < 0) {
			    return (-1);
			}
#if  defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4)
			struct stat buf;
			retVal = stat("/usr/bin/setarch", &buf);
			if(retVal != -1 ){
				execl("/usr/bin/setarch","setarch","i386",mutatedBinary, "-run", testID, NULL); 
			}else{
				fprintf(stderr," Running without /usr/bin/setarch\n");
				execl(mutatedBinary, realFileName,"-run", testID, NULL); 
			}
#else

			execl(mutatedBinary, realFileName,"-run", testID,0); 
#endif
			fprintf(stderr,"ERROR!\n");
			perror("execl");
			exit(-1);

		default: 
			//parent
			delete [] command;
			delete [] mutatedBinary;
#if defined(sparc_sun_solaris2_4) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(i386_unknown_linux2_0) \
 || defined(rs6000_ibm_aix5_1)
			died= waitpid(pid, &status, 0); 
#endif
   	}

#if defined(sparc_sun_solaris2_4) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(i386_unknown_linux2_0) \
 || defined(rs6000_ibm_aix5_1)
	if(WIFEXITED(status)){
		int exitStatus = WEXITSTATUS(status);

		if(exitStatus == 0){
			return 1;
		}
	}else if(WIFSIGNALED(status)){
		fprintf(stderr," terminated with signal: %d \n", WTERMSIG(status));
	}
#endif
	return 0; 
	

}

void createNewProcess(BPatch_thread *&appThread, BPatch_image *&appImage, char *pathname, const char** child_argv){


    appThread = bpatch->createProcess(pathname, child_argv,NULL);

    if (appThread == NULL) {
	fprintf(stderr, "Unable to run test program.\n");
	exit(1);
    }
    appThread->enableDumpPatchedImage();

    // Read the program's image and get an associated image object
    appImage = appThread->getImage();

    // determine whether mutatee is C or C++
    BPatch_variableExpr *isCxx = appImage->findVariable("mutateeCplusplus");
    if (isCxx == NULL) {
	fprintf(stderr, "  Unable to locate variable \"mutateeCplusplus\""
                 " -- assuming 0!\n");
    } else {
        isCxx->readValue(&mutateeCplusplus);
        dprintf("Mutatee is %s.\n", mutateeCplusplus ? "C++" : "C");
    }
}

/**************************************************************************
 * Tests
 **************************************************************************/


void instrumentToCallZeroArg(BPatch_thread *appThread, BPatch_image *appImage, char *instrumentee, char*patch, int testNo, char *testName){

  BPatch_Vector<BPatch_function *> found_funcs;
  if ((NULL == appImage->findFunction(instrumentee, found_funcs)) || !found_funcs.size()) {
    fprintf(stderr, "    Unable to find function %s\n",instrumentee);
    exit(1);
  }
  
  if (1 < found_funcs.size()) {
    fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs.size(), instrumentee);
  }
  
  BPatch_Vector<BPatch_point *> *point1_1 = found_funcs[0]->findPoint(BPatch_entry);


  if (!point1_1 || ((*point1_1).size() == 0)) {
    fprintf(stderr, "**Failed** test #%d (%s)\n", testNo,testName);
    fprintf(stderr, "    Unable to find entry point to \"%s.\"\n",instrumentee);
    exit(1);
  }

  BPatch_Vector<BPatch_function *> bpfv;
  if (NULL == appImage->findFunction(patch, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
    fprintf(stderr, "    Unable to find function %s\n", patch);
    exit(1);
  }
  BPatch_function *call1_func = bpfv[0];
  
  BPatch_Vector<BPatch_snippet *> call1_args;
  BPatch_funcCallExpr call1Expr(*call1_func, call1_args);
  
  dprintf("Inserted snippet2\n");
  appThread->insertSnippet(call1Expr, *point1_1);


}

void buildArgs(const char** child_argv, char *pathname, int testNo){
	int n=0;

	child_argv[n++] = pathname;
	if (debugPrint){
		child_argv[n++] = const_cast<char*>("-verbose");
	}
	child_argv[n++] = const_cast<char*>("-orig"); 

        child_argv[n++] = const_cast<char*>("-run");
       	char str[5];
       	sprintf(str, "%d",testNo);
       	child_argv[n++] = strdup(str);
	
	child_argv[n] = NULL;

}

int letOriginalMutateeFinish(BPatch_thread *appThread){
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
			fprintf(stderr,"Mutatee exited from signal 0x%x\n", signalNum);
		}
	       	retVal = signalNum;
    	}


//	fprintf(stderr,"Original mutatee has terminated\n\************************\n\n");

	return retVal;
}



//
// Start Test Case #1 - (instrument one simple function call and save the world)
//
#if defined(sparc_sun_solaris2_4) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(i386_unknown_linux2_0) \
 || defined(rs6000_ibm_aix5_1)
void mutatorTest1(char *pathname)
#else
   void mutatorTest1(char * /* pathname */)
#endif
{
  char* testName = "instrument one simple function call and save the world";
  int testNo = 1;
 
#if defined(sparc_sun_solaris2_4) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(i386_unknown_linux2_0) \
 || defined(rs6000_ibm_aix5_1)

	const char* child_argv[MAX_TEST+5];
	
	buildArgs(child_argv, pathname, testNo);

	BPatch_thread *appThread;
	BPatch_image *appImage;

	createNewProcess(appThread, appImage, pathname, child_argv);
	
	instrumentToCallZeroArg(appThread, appImage, "func1_1", "call1_1", testNo, testName);

	char* dirname=saveWorld(appThread);	
	savedDirectories[testNo] = dirname;


	/* finish original mutatee to see if it runs */
	int retValue = letOriginalMutateeFinish(appThread);
	/***********/

	if( retValue == 0){
	
		passedTest[testNo] = runMutatedBinaryLDLIBRARYPATH(dirname, "test9_mutated", TEST1);
	}else{
		fprintf(stderr,"**Failed Test #%d: Original Mutatee failed subtest: %d\n\n", testNo,testNo);
	}

	//appThread->terminateExecution();
#else
	passedTest[testNo] = 1;
	fprintf(stderr,"Skipped Test #%d: not implemented on this platform\n",testNo);

#endif	
}

//
// Start Test Case #2 - (instrument many function calls and save the world)
//
void mutatorTest2(char *pathname)
{
	char* testName = "instrument many function calls and save the world";
	int testNo = 2;
		//for(i in 1 to 1000)
	//	instrument func2_i to call call2_i
#if defined(sparc_sun_solaris2_4) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(i386_unknown_linux2_0) \
 || defined(rs6000_ibm_aix5_1)
	const char* child_argv[MAX_TEST+5];
	buildArgs(child_argv, pathname, testNo);

	char instrumentee[15];
	char patch[15];
	int i;
	BPatch_image *appImage;
	BPatch_thread *appThread;

	createNewProcess(appThread, appImage, pathname, child_argv);


	for( i = 1;i<1001; i++){
		sprintf(instrumentee, "func2_%d", i);
		sprintf(patch, "call2_%d", i);

		instrumentToCallZeroArg(appThread, appImage, instrumentee, patch, testNo, testName);
	}

	char * dirname = saveWorld(appThread);
	savedDirectories[testNo] = dirname;
	/* finish original mutatee to see if it runs */
	int retValue = letOriginalMutateeFinish(appThread);
	/***********/

	if( retValue == 0){
	
		passedTest[testNo] = runMutatedBinaryLDLIBRARYPATH(dirname, "test9_mutated", TEST2);
	}else{
		fprintf(stderr,"**Failed Test #%d: Original Mutatee failed subtest: %d\n\n", testNo,testNo);

	}
#else
	passedTest[testNo] = 1;
	fprintf(stderr,"Skipped Test #%d: not implemented on this platform\n",testNo);

#endif
}

//
// Start Test Case #3 - (instrument a function with arguments and save the world)
//
void mutatorTest3(char *pathname)
{
  const char* testName = "four parameter function";
  int testNo = 3;
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(rs6000_ibm_aix5_1) \
 || defined(rs6000_ibm_aix4_1)
//||  defined(rs6000_ibm_aix4_1) this fails on aix from the test case but the
//mutated binary works fine when it is run by hand 

    // Find the entry point to the procedure "func3_1"
	BPatch_image *appImage;
	BPatch_thread *appThread;

	const char* child_argv[MAX_TEST+5];
	buildArgs(child_argv, pathname, testNo);


	createNewProcess(appThread, appImage, pathname, child_argv);

  BPatch_Vector<BPatch_function *> found_funcs;
  if ((NULL == appImage->findFunction("func3_1", found_funcs)) || !found_funcs.size()) {
    fprintf(stderr, "    Unable to find function %s\n",
	    "func3_1");
    exit(1);
  }
  
  if (1 < found_funcs.size()) {
    fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs.size(), "func3_1");
  }
  
  BPatch_Vector<BPatch_point *> *point3_1 = found_funcs[0]->findPoint(BPatch_entry);

  if (!point3_1 || ((*point3_1).size() == 0)) {
    fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
    fprintf(stderr, "    Unable to find entry point to \"func3_1.\"\n");
    exit(1);
  }

  BPatch_Vector<BPatch_function *> bpfv;
  char *fn = "call3_1";
  if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
    fprintf(stderr, "    Unable to find function %s\n", fn);
    exit(1);
  }
  BPatch_function *call2_func = bpfv[0];

    void *ptr;

    /* For platforms where there is only one possible size for a pointer. */
    ptr = TEST_PTR;

    BPatch_Vector<BPatch_snippet *> call2_args;

    BPatch_constExpr expr3_1 (0), expr2_2 (0), expr2_3 (0), expr2_4 (0);

    if (mutateeFortran) {
        BPatch_variableExpr *expr2_5 = appThread->malloc (*appImage->findType ("int"));
        BPatch_variableExpr *expr2_6 = appThread->malloc (*appImage->findType ("int"));

        expr3_1 = expr2_5->getBaseAddr ();
        expr2_2 = expr2_6->getBaseAddr ();

        BPatch_arithExpr expr2_7 (BPatch_assign, *expr2_5, BPatch_constExpr(1));
        appThread->insertSnippet (expr2_7, *point3_1);

        BPatch_arithExpr expr2_8 (BPatch_assign, *expr2_6, BPatch_constExpr(2));
        appThread->insertSnippet (expr2_8, *point3_1);

        expr2_3 = "testString3_1";
        expr2_4 = 13;
    } else {
        expr3_1 = 1;
        expr2_2 = 2;
        expr2_3 = "testString3_1";
        expr2_4 = ptr;
    }

    call2_args.push_back(&expr3_1);
    call2_args.push_back(&expr2_2);
    call2_args.push_back(&expr2_3);
    call2_args.push_back(&expr2_4);

    BPatch_funcCallExpr call2Expr(*call2_func, call2_args);

    dprintf("Inserted snippet2\n");
    appThread->insertSnippet(call2Expr, *point3_1, BPatch_callBefore, BPatch_lastSnippet);

	char * dirname = saveWorld(appThread);
	savedDirectories[testNo] = dirname;	
	/* finish original mutatee to see if it runs */
	int retValue = letOriginalMutateeFinish(appThread);
	/***********/

	if( retValue == 0){
	
		passedTest[testNo] = runMutatedBinaryLDLIBRARYPATH(dirname, "test9_mutated", TEST3);
	}else{
		fprintf(stderr,"**Failed Test #%d: Original Mutatee failed subtest: %d\n\n", testNo,testNo);
	}

#else
	passedTest[testNo] = 1;
	fprintf(stderr,"Skipped Test #%d: not implemented on this platform\n",testNo);

#endif
}

//
// Start Test Case #4 - (call writeValue and save the world)
//
void mutatorTest4(char *pathname)
{
	int testNo = 4;

#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(rs6000_ibm_aix4_1)
//||  defined(rs6000_ibm_aix4_1) this fails on aix from the test case but the
//mutated binary works fine when it is run by hand 

	BPatch_image *appImage;
	BPatch_thread *appThread;

	const char* child_argv[MAX_TEST+5];
	buildArgs(child_argv, pathname, testNo);


	createNewProcess(appThread, appImage, pathname, child_argv);

  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction("func4_1", found_funcs)) || !found_funcs.size()) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "func4_1");
      exit(1);
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func4_1");
    }

    BPatch_Vector<BPatch_point *> *func4_1 = found_funcs[0]->findPoint(BPatch_subroutine);

    if (!func4_1 || ((*func4_1).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func4_1\".\n");
	exit(1);
    }

    BPatch_variableExpr *expr4_1 = findVariable(appImage, "globalVariable4_1", func4_1);

    if (expr4_1 == NULL) {
	fprintf(stderr, "**Failed** test #4 (read/write a variable in the mutatee)\n");
	fprintf(stderr, "    Unable to locate globalVariable4_1\n");
	exit(1);
    }

    int n;
    expr4_1->readValue(&n);

    if (n != 42) {
	fprintf(stderr, "**Failed** test #4 (read/write a variable in the mutatee)\n");
	fprintf(stderr, "    value read from globalVariable4_1 was %d, not 42 as expected\n", n);
	exit(1);
    }

    n = 17;
    expr4_1->writeValue(&n,true); //ccw 31 jul 2002


	char * dirname = saveWorld(appThread);
	savedDirectories[testNo]= dirname;
	/* finish original mutatee to see if it runs */
	int retValue = letOriginalMutateeFinish(appThread);
	/***********/

	if( retValue == 0){
	
		passedTest[testNo] = runMutatedBinaryLDLIBRARYPATH(dirname, "test9_mutated", TEST4);
	}else{
		fprintf(stderr,"**Failed Test #%d: Original Mutatee failed subtest: %d\n\n", testNo,testNo);
	}
#else
	passedTest[testNo] = 1;
	fprintf(stderr,"Skipped Test #%d: not implemented on this platform\n",testNo);
#endif

}

//
// Start Test Case #5 - (call loadLibrary and save the world)
//
void mutatorTest5(char *pathname)
{
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(rs6000_ibm_aix4_1)
	int testNo = 5;
	BPatch_image *appImage;
	BPatch_thread *appThread;

	const char* child_argv[MAX_TEST+5];
	buildArgs(child_argv, pathname, testNo);


	createNewProcess(appThread, appImage, pathname, child_argv);
	if (! appThread->loadLibrary("libLoadMe.so", true)) {
	     fprintf(stderr, "**Failed test #5 (use loadLibrary)\n");
	     fprintf(stderr, "  Mutator couldn't load libLoadMe.so into mutatee\n");
	     exit(1);
	}

	char * dirname = saveWorld(appThread);
	savedDirectories[testNo] = dirname;

	/* finish original mutatee to see if it runs */
	int retValue = letOriginalMutateeFinish(appThread);
	/***********/

	if( retValue == 0){
	
		passedTest[testNo] = runMutatedBinaryLDLIBRARYPATH(dirname, "test9_mutated", TEST5);
	}else{
		fprintf(stderr,"**Failed Test #%d: Original Mutatee failed subtest: %d\n\n", testNo,testNo);
	}

#else
	passedTest[5]=1;
	fprintf(stderr,"Skipped Test #5: not implemented on this platform\n");
#endif

}

//
// Start Test Case #6 - (instrument a shared library and save the world)
//
void mutatorTest6(char *pathname)
{
#if defined(i386_unknown_linux2_0) \
 ||  defined(sparc_sun_solaris2_4) 

	int testNo = 6;
	char *testName = "instrument a shared library and save the world";
	BPatch_image *appImage;
	BPatch_thread *appThread;

	const char* child_argv[MAX_TEST+5];
	buildArgs(child_argv, pathname, testNo);

	createNewProcess(appThread, appImage, pathname, child_argv);


	/*instrument func6_2()  to call call6_2() 
	  each of this functions are in libInstMe.so
	*/

	instrumentToCallZeroArg(appThread, appImage, "func6_2", "call6_2", testNo, testName);

	char * dirname= saveWorld(appThread);
	savedDirectories[testNo]=dirname;

	/* finish original mutatee to see if it runs */
	int retValue = letOriginalMutateeFinish(appThread);
	/***********/

	if( retValue == 0){
	
		passedTest[testNo] = runMutatedBinaryLDLIBRARYPATH(dirname, "test9_mutated", TEST6);
	}else{
		fprintf(stderr,"**Failed Test #%d: Original Mutatee failed subtest: %d\n\n", testNo,testNo);
	}

#else
	passedTest[6]=1;
	fprintf(stderr,"Skipped Test #6: not implemented on this platform\n");
#endif

}

//
// Start Test Case #7 - (instrument entry point of main and first basic block in main)
//
void mutatorTest7(char *pathname)
{
  char* testName = "instrument entry point of main and first basic block in main";
  int testNo = 7;
 
#if defined(sparc_sun_solaris2_4) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(i386_unknown_linux2_0) \
 || defined(rs6000_ibm_aix5_1)

	const char* child_argv[MAX_TEST+5];
	
	buildArgs(child_argv, pathname, testNo);

	BPatch_thread *appThread;
	BPatch_image *appImage;
	createNewProcess(appThread, appImage, pathname, child_argv);
	
	BPatch_Vector<BPatch_function *> found_funcs;
	if ((NULL == appImage->findFunction("main", found_funcs)) || !found_funcs.size()) {
		fprintf(stderr, "    Unable to find function main\n");
		exit(1);
	}
  
	if (1 < found_funcs.size()) {
		fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named main.  Using the first.\n", 
			__FILE__, __LINE__, found_funcs.size());
	}
  
	BPatch_point *point=NULL;

	const BPatch_Vector<BPatch_point*>* points = found_funcs[0]->findPoint(BPatch_subroutine);
	char buff[1024];
	bool foundIt = false;

	for(unsigned int i=0; !foundIt && i< points->size(); i++){

		(*points)[i]->getCalledFunction()->getName(buff,1023);
		if( !strncmp(buff,"firstBasicBlock",15)){
			foundIt = true;
			point = (*points)[i];
		}
	}

	if (!point ){
		fprintf(stderr, "**Failed** test #%d (%s)\n", testNo,testName);
		fprintf(stderr, "    Unable to find call to firstBasicBlock() in main()\n");
		exit(1);
	}

	BPatch_Vector<BPatch_function *> bpfv;
	if (NULL == appImage->findFunction("funcIncrGlobalMain", bpfv) || !bpfv.size()
     		|| NULL == bpfv[0]){
		fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
		fprintf(stderr, "    Unable to find function funcIncrGlobalMain\n");
		exit(1);
	}
	BPatch_function *call1_func = bpfv[0];
  
	BPatch_Vector<BPatch_snippet *> call1_args;
	BPatch_funcCallExpr call1Expr(*call1_func, call1_args);
  
	appThread->insertSnippet(call1Expr, *point);

	instrumentToCallZeroArg(appThread, appImage, "main", "funcIncrGlobalMainBy2", testNo, testName);
	char* dirname=saveWorld(appThread);	
	savedDirectories[testNo] = dirname;

	/* finish original mutatee to see if it runs */
	int retValue = letOriginalMutateeFinish(appThread);
	/***********/

	if( retValue == 0 ){
	
		passedTest[testNo] = runMutatedBinaryLDLIBRARYPATH(dirname, "test9_mutated", TEST7);
	}else{
		fprintf(stderr,"**Failed Test #%d: Original Mutatee failed subtest: %d\n\n", testNo,testNo);
	}

	//appThread->terminateExecution();
#else
	passedTest[testNo] = 1;
	fprintf(stderr,"Skipped Test #%d: not implemented on this platform\n",testNo);

#endif	
}


void dynFunc (BPatch_thread * /* thr */, BPatch_module *mod, bool /* load */){
	char name[4096];
	mod->getName(name, 4096);	
	fprintf(stderr,"LOADED: %s\n",name);

}


/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

int mutatorMAIN(char *pathname)
{

    // Create an instance of the BPatch library
    bpatch = new BPatch;

    // Force functions to be relocated
    if (forceRelocation) {
      bpatch->setForcedRelocation_NP(true);
    }

    // Register a callback function that prints any error messages
    bpatch->registerErrorCallback(errorFunc);

	bpatch->registerDynLibraryCallback(dynFunc);
    // Start the mutatee
   fprintf(stderr,"Starting \"%s\"\n", pathname);

    if (runTest[1]) mutatorTest1(pathname);
    if (runTest[2]) mutatorTest2(pathname);
    if (runTest[3]) mutatorTest3(pathname);
    if (runTest[4]) mutatorTest4(pathname);
    if (runTest[5]) mutatorTest5(pathname);
    if (runTest[6]) mutatorTest6(pathname);
    if (runTest[7]) mutatorTest7(pathname);
    // Start of code to continue the process.  All mutations made
    // above will be in place before the mutatee begins its tests.

    return(1);
}

void removeDirectoryAndAllFiles ( char* dirname) {

  	DIR* dirPtr = opendir ( dirname);
	struct dirent *dirEntry, *result;

        if (!dirPtr) {
                return;
        }
        dirEntry  = (struct dirent*)malloc( sizeof(*dirEntry) + pathconf(dirname,_PC_NAME_MAX)+ 1 );
        result = dirEntry;

	readdir_r ( dirPtr, dirEntry, &result);
        while ( result ){ 
                int len = strlen(dirEntry->d_name);
                char fullname[1024];
                if( dirEntry->d_name[len-1] != '.'){
                        sprintf(fullname, "%s/%s", dirname,dirEntry->d_name);
                        remove(fullname);
                }
		readdir_r ( dirPtr, dirEntry, &result);
        }
        closedir(dirPtr);
	remove(dirname);
	free(dirEntry);
}


//
// main - decide our role and call the correct "main"
//
int
main(int argc, char *argv[])
{
    char mutateeName[128];
    char libRTname[256];
	int removeAllDirs =0, saveAllDirs=0;

    strcpy(mutateeName,mutateeNameRoot);
    libRTname[0]='\0';
    if (!getenv("DYNINSTAPI_RT_LIB")) {
	 fprintf(stderr,"Environment variable DYNINSTAPI_RT_LIB undefined:\n"
#if defined(i386_unknown_nt4_0)
		 "    using standard search strategy for libdyninstAPI_RT.dll\n");
#else
	         "    set it to the full pathname of libdyninstAPI_RT\n");
         exit(-1);
#endif
    } else
         strcpy((char *)libRTname, (char *)getenv("DYNINSTAPI_RT_LIB"));

    updateSearchPaths(argv[0]);

    unsigned int i;
    // by default run all tests
    for (i=1; i <= MAX_TEST; i++) {
        runTest[i] = true;
        passedTest[i] = false;
	savedDirectories[i]=NULL;
    }

    for (i=1; i < argc; i++) {
        if (strncmp(argv[i], "-v+", 3) == 0)    errorPrint++;
        if (strncmp(argv[i], "-v++", 4) == 0)   errorPrint++;
	
	if (strcmp(argv[i], "-rmdirs") == 0){
		removeAllDirs = 1;
	}else if (strcmp(argv[i], "-savedirs") == 0){
		saveAllDirs = 1;
	}else if (strncmp(argv[i], "-verbose", 2) == 0) {
	    debugPrint = 1;
	} else if (!strcmp(argv[i], "-V")) {
            fprintf (stdout, "%s\n", V_libdyninstAPI);
            if (libRTname[0])
                fprintf (stdout, "DYNINSTAPI_RT_LIB=%s\n", libRTname);
            fflush(stdout);
	} else if (!strcmp(argv[i], "-skip")) {
	    unsigned int j;
	    runAllTests = false;
            for (j=i+1; j < argc; j++) {
                unsigned int testId;
                if ((testId = atoi(argv[j]))) {
                    if ((testId > 0) && (testId <= MAX_TEST)) {
                        runTest[testId] = false;
                    } else {
                       fprintf(stderr,"invalid test %d requested\n", testId);
                        exit(-1);
                    }
                } else {
                    // end of test list
		    break;
                }
            }
            i=j-1;
	} else if (!strcmp(argv[i], "-run")) {
	    unsigned int j;
	    runAllTests = false;
            for (j=0; j <= MAX_TEST; j++) runTest[j] = false;
            for (j=i+1; j < argc; j++) {
                unsigned int testId;
                if ((testId = atoi(argv[j]))) {
                    if ((testId > 0) && (testId <= MAX_TEST)) {
                        runTest[testId] = true;
                    } else {
                       fprintf(stderr,"invalid test %d requested\n", testId);
                        exit(-1);
                    }
                } else {
                    // end of test list
		    break;
                }
            }
            i=j-1;
	} else if (!strcmp(argv[i], "-mutatee")) {
	    i++;
            if (*argv[i]=='_')
                strcat(mutateeName,argv[i]);
            else
                strcpy(mutateeName,argv[i]);
#if defined(i386_unknown_nt4_0) \
 || defined(i386_unknown_linux2_0) \
 || defined(sparc_sun_solaris2_4)
	} else if (!strcmp(argv[i], "-relocate")) {
            forceRelocation = true;
#endif
	} else {
	    fprintf(stderr, "Usage: test9 "
		    "[-V] [-verbose]  "
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(rs6000_ibm_aix4_1)
		    "[-saveworld] "
#endif 
                    "[-mutatee <test1.mutatee>] "
		    "[-run <test#> <test#> ...] "
		    "[-skip <test#> <test#> ...]\n");
            fprintf(stderr, "%d subtests\n", MAX_TEST);
	    exit(-1);
	}
    }

    if (!runAllTests) {
       fprintf(stderr,"Running Tests: ");
	for (unsigned int j=1; j <= MAX_TEST; j++) {
	    if (runTest[j])fprintf(stderr,"%d ", j);
	}
	fprintf(stderr,"\n");
    }

    // patch up the default compiler in mutatee name (if necessary)
    if (!strstr(mutateeName, "_"))
#if defined(i386_unknown_nt4_0)
        strcat(mutateeName,"_VC");
#else
        strcat(mutateeName,"_gcc");
#endif
    int exitCode = mutatorMAIN(mutateeName);

	int testsFailed=0;

    /* See how we did running the tests. */
    for (i=1; i <= MAX_TEST; i++) {
        if (runTest[i] && !passedTest[i]) {
            fprintf(stderr,"failure on %d\n", i);
            testsFailed++;
		if(  removeAllDirs && savedDirectories[i] ){
			/* remove failed test here */
			removeDirectoryAndAllFiles(savedDirectories[i]);
		}
        }else if( !saveAllDirs && savedDirectories[i]) {
		/* remove successful test here */
 		removeDirectoryAndAllFiles(savedDirectories[i]);
	}
    }

    if (!testsFailed) {
        fprintf(stderr,"All tests passed\n");
    } else {
        fprintf(stderr,"**Failed** %d test%c\n",testsFailed,(testsFailed>1)?'s':' ');
    }


    return exitCode;
}
