// $Id: test6.C,v 1.1 2001/10/30 21:02:52 gaburici Exp $
 
#include <stdio.h>
#include <string.h>
#include <assert.h>
#ifdef i386_unknown_nt4_0
#include <windows.h>
#include <winbase.h>
#else
#include <unistd.h>
#endif

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_memoryAccess_NP.h"

#include "test_util.h"

char *mutateeNameRoot = "test6.mutatee";

int inTest;		// current test #
#define DYNINST_NO_ERROR -1
int expectError = DYNINST_NO_ERROR;
int debugPrint = 0; // internal "mutator" tracing
int errorPrint = 0; // external "dyninst" tracing (via errorFunc)

bool forceRelocation = false;  // Force relocation upon instrumentation

#define dprintf if (debugPrint) printf

bool runAllTests = true;
const unsigned int MAX_TEST = 4;
bool runTest[MAX_TEST+1];
bool passedTest[MAX_TEST+1];
bool failedTest[MAX_TEST+1];
int threadCount = 0;
BPatch_thread *mythreads[25];

BPatch *bpatch;

/*
 * Given a string variable name and an expected value, lookup the varaible
 *    in the child process, and verify that the value matches.
 *
 */
bool verifyChildMemory(BPatch_thread *appThread, 
			      char *name, int expectedVal)
{
     BPatch_image *appImage = appThread->getImage();

     if (!appImage) {
	 dprintf("unable to locate image for %d\n", appThread->getPid());
	 return false;
     }

     BPatch_variableExpr *var = appImage->findVariable(name);
     if (!var) {
	 dprintf("unable to located variable %s in child\n", name);
	 return false;
     }

     int actualVal;
     var->readValue(&actualVal);

     if (expectedVal != actualVal) {
	 printf("*** for %s, expected val = %d, but actual was %d\n",
		name, expectedVal, actualVal);
	 return false;
     } else {
	 dprintf("verified %s was = %d\n", name, actualVal);
	 return true;
     }
}

/**************************************************************************
 * Error callback
 **************************************************************************/

void errorFunc(BPatchErrorLevel level, int num, const char **params)
{
  if (num == 0) {
    // conditional reporting of warnings and informational messages
    if (errorPrint) {
      if (level == BPatchInfo)
        { if (errorPrint > 1) printf("%s\n", params[0]); }
      else
        printf("%s", params[0]);
    }
  } else {
    // reporting of actual errors
    char line[256];
    const char *msg = bpatch->getEnglishErrorString(num);
    bpatch->formatErrorString(line, sizeof(line), msg, params);
    
    if (num != expectError) {
      printf("Error #%d (level %d): %s\n", num, level, line);
      
      // We consider some errors fatal.
      if (num == 101) {
        exit(-1);
      }
    }
  }
}

void instCall(BPatch_thread* bpthr, const char* fname,
              const BPatch_Vector<BPatch_point*>* res)
{
  char buf[30];
  snprintf(buf, 30, "count%s", fname);

  BPatch_Vector<BPatch_snippet*> callArgs;

  BPatch_function *countXXXFunc = bpthr->getImage()->findFunction(buf);

  BPatch_funcCallExpr countXXXCall(*countXXXFunc, callArgs);

  bpthr->insertSnippet(countXXXCall, *res);
}

#define MK_LD(imm, rs1, rs2, bytes) (new MemoryAccess(true, false, (bytes), (imm), (rs1), (rs2)))
#define MK_ST(imm, rs1, rs2, bytes) (new MemoryAccess(false, true, (bytes), (imm), (rs1), (rs2)))
#define MK_LS(imm, rs1, rs2, bytes) (new MemoryAccess(true, true, (bytes), (imm), (rs1), (rs2)))
#define MK_PF(imm, rs1, rs2, f) (new MemoryAccess(false, false, true, \
                                                  (imm), (rs1), (rs2), \
                                                   0, -1, -1, (f)))

// what we expect to find in the "loadsnstores" function; platform specific
#ifdef sparc_sun_solaris2_4

const unsigned int nloads = 15;
const unsigned int nstores = 13;
const unsigned int nprefes = 2;
const unsigned int naxses = 26;

MemoryAccess* loadList[nloads];
MemoryAccess* storeList[nstores];
MemoryAccess* prefeList[nprefes];
MemoryAccess* axsesList[naxses];

void init_test_data()
{
  int k=-1;

  loadList[++k] = MK_LD(0,17,0,4);
  loadList[++k] = MK_LD(3,17,-1,1);
  loadList[++k] = MK_LD(2,17,-1,2);
  loadList[++k] = MK_LD(0,17,0,8);
  loadList[++k] = MK_LD(0,17,0,4);

  loadList[++k] = MK_LS(3,17,-1,1); // ldstub
  loadList[++k] = MK_LD(3,17,-1,1);

  loadList[++k] = MK_LS(0,17,18,4); // cas
  loadList[++k] = MK_LS(0,17,18,8); // casx
  loadList[++k] = MK_LS(0,17,0,4);  // swap

  loadList[++k] = MK_LD(0,17,0,4);
  loadList[++k] = MK_LD(0,17,0,4);
  loadList[++k] = MK_LD(0,17,0,8);
  loadList[++k] = MK_LD(0,17,0,8);
  loadList[++k] = MK_LD(0,17,0,16);

  k=-1;

  storeList[++k] = MK_LS(3,17,-1,1); // ldstub
  storeList[++k] = MK_LS(0,17,18,4); // cas
  storeList[++k] = MK_LS(0,17,18,8); // casx
  storeList[++k] = MK_LS(0,17,0,4);  // swap

  storeList[++k] = MK_ST(7,21,-1,1);
  storeList[++k] = MK_ST(6,21,-1,2);
  storeList[++k] = MK_ST(4,21,-1,4);
  storeList[++k] = MK_ST(0,21,0,8);
  storeList[++k] = MK_ST(0,17,0,4);
  storeList[++k] = MK_ST(0,17,0,8);
  storeList[++k] = MK_ST(0,18,0,16);
  storeList[++k] = MK_ST(4,21,-1,4);
  storeList[++k] = MK_ST(0,21,0,8);

  k=-1;

  prefeList[++k] = MK_PF(0,17,0,2);
  prefeList[++k] = MK_PF(0,20,0,0);

  k=-1;

  axsesList[++k] = MK_LD(0,17,0,4);
  axsesList[++k] = MK_LD(3,17,-1,1);
  axsesList[++k] = MK_LD(2,17,-1,2);
  axsesList[++k] = MK_LD(0,17,0,8);
  axsesList[++k] = MK_LD(0,17,0,4);

  axsesList[++k] = MK_LS(3,17,-1,1); // ldstub
  axsesList[++k] = MK_LD(3,17,-1,1);

  axsesList[++k] = MK_LS(0,17,18,4); // cas
  axsesList[++k] = MK_LS(0,17,18,8); // casx
  axsesList[++k] = MK_LS(0,17,0,4);  // swap

  axsesList[++k] = MK_LD(0,17,0,4);
  axsesList[++k] = MK_LD(0,17,0,4);
  axsesList[++k] = MK_LD(0,17,0,8);
  axsesList[++k] = MK_LD(0,17,0,8);
  axsesList[++k] = MK_LD(0,17,0,16);

  axsesList[++k] = MK_PF(0,17,0,2);
  axsesList[++k] = MK_PF(0,20,0,0);

  axsesList[++k] = MK_ST(7,21,-1,1);
  axsesList[++k] = MK_ST(6,21,-1,2);
  axsesList[++k] = MK_ST(4,21,-1,4);
  axsesList[++k] = MK_ST(0,21,0,8);
  axsesList[++k] = MK_ST(0,17,0,4);
  axsesList[++k] = MK_ST(0,17,0,8);
  axsesList[++k] = MK_ST(0,18,0,16);
  axsesList[++k] = MK_ST(4,21,-1,4);
  axsesList[++k] = MK_ST(0,21,0,8);
}

#endif

#ifdef rs6000_ibm_aix4_1
const unsigned int nloads = 38;
const unsigned int nstores = 32;
const unsigned int nprefes = 0;
const unsigned int naxses = 70;

MemoryAccess* loadList[nloads];
MemoryAccess* storeList[nstores];
MemoryAccess* prefeList[nprefes];
MemoryAccess* axsesList[naxses];

void init_test_data()
{
  int k=-1;

  loadList[++k] = MK_LD(0, 7, -1, 1);
  loadList[++k] = MK_LD(3, 7, -1, 1);

  loadList[++k] = MK_LD(0, 3, 8, 1);
  loadList[++k] = MK_LD(0, 3, 9, 1);

  loadList[++k] = MK_LD(0, 3, -1, 2);
  loadList[++k] = MK_LD(4, 3, -1, 2);
  loadList[++k] = MK_LD(2, 3, -1, 2);
  loadList[++k] = MK_LD(0, 3, -1, 2);

  loadList[++k] = MK_LD(0, 7, 9, 2);
  loadList[++k] = MK_LD(0, 7, 8, 2);
  loadList[++k] = MK_LD(0, 7, 9, 2);
  loadList[++k] = MK_LD(0, 7, 8, 2);

  loadList[++k] = MK_LD(0, 7, -1, 4);
  loadList[++k] = MK_LD(4, 7, -1, 4);
  loadList[++k] = MK_LD(0, 3, 9, 4);
  loadList[++k] = MK_LD(0, 3, 9, 4);

  loadList[++k] = MK_LD(4, 3, -1, 4);
  loadList[++k] = MK_LD(0, 7, 0, 4);  // fixme: 0 should be -1 for rs2...
  loadList[++k] = MK_LD(0, 7, 8, 4);

  loadList[++k] = MK_LD(0, 7, -1, 8);
  loadList[++k] = MK_LD(0, 3, -1, 8);
  loadList[++k] = MK_LD(0, 7, 9, 8);
  loadList[++k] = MK_LD(0, 3, 9, 8);

  loadList[++k] = MK_LD(0, 8, 3, 2);
  loadList[++k] = MK_LD(0, 9, 3, 4);

  loadList[++k] = MK_LD(-76, 1, -1, 76);
  loadList[++k] = MK_LD(0, 4, -1, 24);
  loadList[++k] = new MemoryAccess(true, false,
				   0, 1, 9,
				   0, 9999, -1); // 9999 means XER_25:31

  loadList[++k] = MK_LD(0, 0, 3, 4);
  loadList[++k] = MK_LD(0, 0, 7, 8);

  loadList[++k] = MK_LD(0, 4, -1, 4);
  loadList[++k] = MK_LD(0, 4, 6, 4);
  loadList[++k] = MK_LD(0, 4, -1, 4);
  loadList[++k] = MK_LD(0, 6, 4, 4);
  loadList[++k] = MK_LD(0, 6, -1, 8);
  loadList[++k] = MK_LD(0, 6, 9, 8);
  loadList[++k] = MK_LD(8, 6, -1, 8);
  loadList[++k] = MK_LD(0, 9, 7, 8);

  k=-1;

  storeList[++k] = MK_ST(3, 7, -1, 1);
  storeList[++k] = MK_ST(1, 7, -1, 1);
  storeList[++k] = MK_ST(0, 3, 8, 1);
  storeList[++k] = MK_ST(0, 8, 3, 1);
  storeList[++k] = MK_ST(2, 7, -1, 2);
  storeList[++k] = MK_ST(6, 7, -1, 2);
  storeList[++k] = MK_ST(0, 3, 9, 2);
  storeList[++k] = MK_ST(0, 9, 3, 2);
  storeList[++k] = MK_ST(0, 7, -1, 4);
  storeList[++k] = MK_ST(4, 7, -1, 4);
  storeList[++k] = MK_ST(0, 3, 9, 4);
  storeList[++k] = MK_ST(0, 9, 3, 4);
  storeList[++k] = MK_ST(0, 7, -1, 8);
  storeList[++k] = MK_ST(0, 7, -1, 8);
  storeList[++k] = MK_ST(0, 7, 8, 8);
  storeList[++k] = MK_ST(0, 8, 7, 8);

  storeList[++k] = MK_ST(0, 8, 7, 2);
  storeList[++k] = MK_ST(0, 9, 7, 4);

  storeList[++k] = MK_ST(-76, 1, -1, 76);
  storeList[++k] = MK_ST(0, 4, -1, 20);

  storeList[++k] = new MemoryAccess(false, true,
				    0, 1, 9,
				    0, 9999, -1); // 9999 means XER_25:31

  storeList[++k] = MK_ST(0, 0, 3, 4);
  storeList[++k] = MK_ST(0, 0, 7, 8);

  storeList[++k] = MK_ST(4, 4, -1, 4);
  storeList[++k] = MK_ST(0, 4, 0, 4);
  storeList[++k] = MK_ST(0, 4, -1, 4);
  storeList[++k] = MK_ST(0, 0, 4, 4);
  storeList[++k] = MK_ST(0, 6, -1, 8);
  storeList[++k] = MK_ST(0, 6, 9, 8);
  storeList[++k] = MK_ST(8, 6, -1, 8);
  storeList[++k] = MK_ST(0, 9, 7, 8);

  storeList[++k] = MK_ST(0, 0, 4, 4);

  k=-1;

  axsesList[++k] = MK_LD(0, 7, -1, 1);
  axsesList[++k] = MK_LD(3, 7, -1, 1);

  axsesList[++k] = MK_LD(0, 3, 8, 1);
  axsesList[++k] = MK_LD(0, 3, 9, 1);

  axsesList[++k] = MK_LD(0, 3, -1, 2);
  axsesList[++k] = MK_LD(4, 3, -1, 2);
  axsesList[++k] = MK_LD(2, 3, -1, 2);
  axsesList[++k] = MK_LD(0, 3, -1, 2);

  axsesList[++k] = MK_LD(0, 7, 9, 2);
  axsesList[++k] = MK_LD(0, 7, 8, 2);
  axsesList[++k] = MK_LD(0, 7, 9, 2);
  axsesList[++k] = MK_LD(0, 7, 8, 2);

  axsesList[++k] = MK_LD(0, 7, -1, 4);
  axsesList[++k] = MK_LD(4, 7, -1, 4);
  axsesList[++k] = MK_LD(0, 3, 9, 4);
  axsesList[++k] = MK_LD(0, 3, 9, 4);

  axsesList[++k] = MK_LD(4, 3, -1, 4);
  axsesList[++k] = MK_LD(0, 7, 0, 4);  // fixme: 0 should be -1 for rs2...
  axsesList[++k] = MK_LD(0, 7, 8, 4);

  axsesList[++k] = MK_LD(0, 7, -1, 8);
  axsesList[++k] = MK_LD(0, 3, -1, 8);
  axsesList[++k] = MK_LD(0, 7, 9, 8);
  axsesList[++k] = MK_LD(0, 3, 9, 8);

  axsesList[++k] = MK_ST(3, 7, -1, 1);
  axsesList[++k] = MK_ST(1, 7, -1, 1);
  axsesList[++k] = MK_ST(0, 3, 8, 1);
  axsesList[++k] = MK_ST(0, 8, 3, 1);
  axsesList[++k] = MK_ST(2, 7, -1, 2);
  axsesList[++k] = MK_ST(6, 7, -1, 2);
  axsesList[++k] = MK_ST(0, 3, 9, 2);
  axsesList[++k] = MK_ST(0, 9, 3, 2);
  axsesList[++k] = MK_ST(0, 7, -1, 4);
  axsesList[++k] = MK_ST(4, 7, -1, 4);
  axsesList[++k] = MK_ST(0, 3, 9, 4);
  axsesList[++k] = MK_ST(0, 9, 3, 4);
  axsesList[++k] = MK_ST(0, 7, -1, 8);
  axsesList[++k] = MK_ST(0, 7, -1, 8);
  axsesList[++k] = MK_ST(0, 7, 8, 8);
  axsesList[++k] = MK_ST(0, 8, 7, 8);

  axsesList[++k] = MK_LD(0, 8, 3, 2);
  axsesList[++k] = MK_LD(0, 9, 3, 4);

  axsesList[++k] = MK_ST(0, 8, 7, 2);
  axsesList[++k] = MK_ST(0, 9, 7, 4);

  axsesList[++k] = MK_ST(-76, 1, -1, 76);
  axsesList[++k] = MK_LD(-76, 1, -1, 76);

  axsesList[++k] = MK_LD(0, 4, -1, 24);
  axsesList[++k] = MK_ST(0, 4, -1, 20);

  axsesList[++k] = new MemoryAccess(true, false,
				   0, 1, 9,
				   0, 9999, -1); // 9999 means XER_25:31

  axsesList[++k] = new MemoryAccess(false, true,
				    0, 1, 9,
				    0, 9999, -1); // 9999 means XER_25:31

  axsesList[++k] = MK_LD(0, 0, 3, 4);
  axsesList[++k] = MK_ST(0, 0, 3, 4);

  axsesList[++k] = MK_LD(0, 0, 7, 8);
  axsesList[++k] = MK_ST(0, 0, 7, 8);

  axsesList[++k] = MK_LD(0, 4, -1, 4);
  axsesList[++k] = MK_LD(0, 4, 6, 4);
  axsesList[++k] = MK_LD(0, 4, -1, 4);
  axsesList[++k] = MK_LD(0, 6, 4, 4);
  axsesList[++k] = MK_LD(0, 6, -1, 8);
  axsesList[++k] = MK_LD(0, 6, 9, 8);
  axsesList[++k] = MK_LD(8, 6, -1, 8);
  axsesList[++k] = MK_LD(0, 9, 7, 8);

  axsesList[++k] = MK_ST(4, 4, -1, 4);
  axsesList[++k] = MK_ST(0, 4, 0, 4);
  axsesList[++k] = MK_ST(0, 4, -1, 4);
  axsesList[++k] = MK_ST(0, 0, 4, 4);
  axsesList[++k] = MK_ST(0, 6, -1, 8);
  axsesList[++k] = MK_ST(0, 6, 9, 8);
  axsesList[++k] = MK_ST(8, 6, -1, 8);
  axsesList[++k] = MK_ST(0, 9, 7, 8);

  axsesList[++k] = MK_ST(0, 0, 4, 4);
}
#endif

#ifdef i386_unknown_linux2_0
void init_test_data()
{
}
#endif

#ifdef mips_sgi_irix6_4
void init_test_data()
{
}
#endif

#ifdef alpha_dec_osf4_0
void init_test_data()
{
}
#endif

#ifdef i386_unknown_nt4_0
void init_test_data()
{
}
#endif


#define skiptest(i,d) { printf("Skipping test #%d (%s)\n", (i), (d)); \
                        printf("    not implemented on this platform\n"); \
                        passedTest[(i)] = true; }

#define failtest(i,d,r) { fprintf(stderr, "**Failed** test #%d (%s)\n", (i), (d)); \
                          fprintf(stderr, "    %s\n", (r)); \
                          exit(1); }

#define dumpvect(res, msg) if(debugPrint) { \
                             printf("%s: %d\n", msg, res->size()); \
                             for(unsigned int i=0; i<res->size(); ++i) { \
                               BPatch_point *bpp = (*res)[i]; \
                               void* a = bpp->getAddress(); \
                               printf("%s[%d]: %p\n", msg, i, a); \
                             } \
                           }

bool validate(BPatch_Vector<BPatch_point*>* res, MemoryAccess* acc[], char* msg)
{
  bool ok = true;

  for(unsigned int i=0; i<res->size(); ++i) {
    BPatch_point* bpoint = (*res)[i];
    ok = (ok && bpoint->getMemoryAccess()->equals(acc[i]));
    if(!ok) {
      printf("Validation failed at %s #%d.\n", msg, i);
      return ok;
    }
  }
  return ok;
}


// Find and instrument loads with a simple function call snippet
void mutatorTest1(BPatch_thread *bpthr, BPatch_image *bpimg,
                  int testnum = 1, char* testdesc = "load instrumentation")
{
#if !defined(sparc_sun_solaris2_4) && !defined(rs6000_ibm_aix4_1)
  skiptest(testnum, testdesc);
#else
  BPatch_Set<BPatch_opCode> loads;
  loads.insert(BPatch_opLoad);

  BPatch_Vector<BPatch_point*>* res1 = bpimg->findProcedurePoint("loadsnstores", loads);

  if(!res1)
    failtest(testnum, testdesc, "Unable to find function \"loadsnstores.\"\n");

  dumpvect(res1, "Load");

  if((*res1).size() != nloads)
    failtest(testnum, testdesc, "Number of loads seems wrong in function \"loadsnstores.\"\n");

  if(!validate(res1, loadList, "load"))
    failtest(testnum, testdesc, "Load sequence failed validation.\n");

  instCall(bpthr, "Load", res1);
#endif
}


// Find and instrument stores with a simple function call snippet
void mutatorTest2(BPatch_thread *bpthr, BPatch_image *bpimg,
                  int testnum = 2, char* testdesc = "store instrumentation")
{
#if !defined(sparc_sun_solaris2_4) && !defined(rs6000_ibm_aix4_1)
  skiptest(testnum, testdesc);
#else
  BPatch_Set<BPatch_opCode> stores;
  stores.insert(BPatch_opStore);

  BPatch_Vector<BPatch_point*>* res1 = bpimg->findProcedurePoint("loadsnstores", stores);

  if(!res1)
    failtest(testnum, testdesc, "Unable to find function \"loadsnstores.\"\n");

  dumpvect(res1, "Stores");

  if((*res1).size() != nstores)
    failtest(testnum, testdesc, "Number of stores seems wrong in function \"loadsnstores.\"\n");

  if(!validate(res1, storeList, "store"))
    failtest(testnum, testdesc, "Store sequence failed validation.\n");

  instCall(bpthr, "Store", res1);
#endif
}

// Find and instrument prefetches with a simple function call snippet
void mutatorTest3(BPatch_thread *bpthr, BPatch_image *bpimg,
                  int testnum = 3, char* testdesc = "prefetch instrumentation")
{
#if !defined(sparc_sun_solaris2_4) && !defined(rs6000_ibm_aix4_1)
  skiptest(testnum, testdesc);
#else
  BPatch_Set<BPatch_opCode> prefes;
  prefes.insert(BPatch_opPrefetch);

  BPatch_Vector<BPatch_point*>* res1 = bpimg->findProcedurePoint("loadsnstores", prefes);

  if(!res1)
    failtest(testnum, testdesc, "Unable to find function \"loadsnstores.\"\n");

  dumpvect(res1, "Prefetches");

  if((*res1).size() != nprefes)
    failtest(testnum, testdesc,
             "Number of prefetches seems wrong in function \"loadsnstores.\"\n");

  if(!validate(res1, prefeList, "prefetch"))
    failtest(testnum, testdesc, "Prefetch sequence failed validation.\n");

  instCall(bpthr, "Prefetch", res1);
#endif
}


// Find and instrument all accesses with a simple function call snippet
void mutatorTest4(BPatch_thread *bpthr, BPatch_image *bpimg,
                  int testnum = 4, char* testdesc = "access instrumentation")
{
#if !defined(sparc_sun_solaris2_4) && !defined(rs6000_ibm_aix4_1)
  skiptest(testnum, testdesc);
#else
  BPatch_Set<BPatch_opCode> axs;
  axs.insert(BPatch_opLoad);
  axs.insert(BPatch_opStore);
  axs.insert(BPatch_opPrefetch);

  BPatch_Vector<BPatch_point*>* res1 = bpimg->findProcedurePoint("loadsnstores", axs);

  if(!res1)
    failtest(testnum, testdesc, "Unable to find function \"loadsnstores.\"\n");

  dumpvect(res1, "Accesses");

  if((*res1).size() != naxses)
    failtest(testnum, testdesc,
             "Number of accesses seems wrong in function \"loadsnstores.\"\n");

  if(!validate(res1, axsesList, "access"))
    failtest(testnum, testdesc, "Access sequence failed validation.\n");

  instCall(bpthr, "Access", res1);
#endif
}


void mutatorMAIN(char *pathname)
{
  // Create an instance of the BPatch library
  bpatch = new BPatch;

  // Register a callback function that prints any error messages
  // Mental note: this needs to be done BEFORE thread creation...
  bpatch->registerErrorCallback(errorFunc);

  // XXX: On Solaris we need an empty argv
  char *argv[] = { NULL };
  BPatch_thread *bpthr = bpatch->createProcess(pathname, argv);

  // Force functions to be relocated
  // VG: not sure why we need this...
  //if (forceRelocation) {
  //bpatch->setForcedRelocation_NP(true);
  //}

  // Start the mutatee
  dprintf("Starting \"%s\"\n", pathname);

  char *child_argv[MAX_TEST+5];

  int n = 0;
  child_argv[n++] = pathname;
  if (debugPrint) child_argv[n++] = "-verbose";

  if (runAllTests) {
    child_argv[n++] = "-runall"; // signifies all tests
  } else {
    child_argv[n++] = "-run";
    for (unsigned int j=1; j <= MAX_TEST; j++) {
      if (runTest[j]) {
        char str[5];
        sprintf(str, "%d", j);
        child_argv[n++] = strdup(str);
      }
    }
  }

  child_argv[n] = NULL;

  bpthr = bpatch->createProcess(pathname, child_argv, NULL);

  if (bpthr == NULL) {
    fprintf(stderr, "Unable to run test program.\n");
    exit(1);
  }

  BPatch_image *bpimg = bpthr->getImage();

  init_test_data();

  if (runTest[1]) mutatorTest1(bpthr, bpimg);
  if (runTest[2]) mutatorTest2(bpthr, bpimg);
  if (runTest[3]) mutatorTest3(bpthr, bpimg);
  if (runTest[4]) mutatorTest4(bpthr, bpimg);

  dprintf("starting program execution.\n");
  bpthr->continueExecution();

  unsigned int testsFailed = 0;
  for (unsigned int i=1; i <= MAX_TEST; i++) {
    if (runTest[i] && !passedTest[i]) testsFailed++;
  }

  if (!testsFailed) {
    if (runAllTests) {
      printf("All tests passed\n");
    } else {
      printf("All requested tests passed\n");
    }
  }
}

//
// main - decide our role and call the correct "main"
//
int
main(unsigned int argc, char *argv[])
{
    unsigned int i;
    bool N32ABI=false;
    char mutateeName[128];
    char libRTname[256];

    strcpy(mutateeName, mutateeNameRoot);
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

    // by default run all tests
    for (i=1; i <= MAX_TEST; i++) {
        runTest[i] = true;
        passedTest[i] = false;
    }

    for (i=1; i < argc; i++) {
        if (strncmp(argv[i], "-v+", 3) == 0)    errorPrint++;
        if (strncmp(argv[i], "-v++", 4) == 0)   errorPrint++;
	if (strncmp(argv[i], "-verbose", 2) == 0) {
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
                        printf("invalid test %d requested\n", testId);
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
                        printf("invalid test %d requested\n", testId);
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
#if defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0) || defined(sparc_sun_solaris2_4)
	} else if (!strcmp(argv[i], "-relocate")) {
            forceRelocation = true;
#endif
#if defined(mips_sgi_irix6_4)
	} else if (!strcmp(argv[i], "-n32")) {
	    N32ABI=true;
#endif
	} else {
	    fprintf(stderr, "Usage: test6 "
		    "[-V] [-verbose] "
#if defined(mips_sgi_irix6_4)
		    "[-n32] "
#endif
                    "[-mutatee <test4a.mutatee>] "
		    "[-run <test#> <test#> ...] "
		    "[-skip <test#> <test#> ...]\n");
            fprintf(stderr, "%d subtests\n", MAX_TEST);
	    exit(-1);
	}
    }

    if (!runAllTests) {
        printf("Running Tests: ");
	for (unsigned int j=1; j <= MAX_TEST; j++) {
	    if (runTest[j]) printf("%d ", j);
	}
	printf("\n");
    }

    // patch up the default compiler in mutatee name (if necessary)
    if (!strstr(mutateeName, "_"))
#if defined(i386_unknown_nt4_0)
        strcat(mutateeName,"_VC");
#else
        strcat(mutateeName,"_gcc");
#endif
    if (N32ABI || strstr(mutateeName,"_n32")) {
        // patch up file names based on alternate ABI (as necessary)
        if (!strstr(mutateeName, "_n32")) strcat(mutateeName,"_n32");
    }
    // patch up the platform-specific filename extensions
#if defined(i386_unknown_nt4_0)
    if (!strstr(mutateeName, ".exe")) strcat(mutateeName,".exe");
#endif

    mutatorMAIN(mutateeName);

    return 0;
}
