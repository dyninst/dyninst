#include <stdio.h>
#include <stdlib.h>

#define dprintf	if (debugPrint) printf
int debugPrint = 0;

#define MAX_TEST 4

int runTest[MAX_TEST+1];
int passedTest[MAX_TEST+1];

#define TRUE	1
#define FALSE	0

#define USAGE "Usage: test6.mutatee [-verbose] -run <num> .."

/*
 * Verify that a scalar value of a variable is what is expected
 *
 */
void verifyScalarValue(char *name, int a, int value, int testNum, char *testName)
{
    if (a != value) {
	if (passedTest[testNum])
	    printf("**Failed** test %d (%s)\n", testNum, testName);
	printf("  %s = %d, not %d\n", name, a, value);
	passedTest[testNum] = FALSE;
    }
}

extern int loadsnstores(int, int, int);
int result_of_loadsnstores;

unsigned int loadCnt = 0;
unsigned int storeCnt = 0;
unsigned int prefeCnt = 0;
unsigned int accessCnt = 0;

#ifdef sparc_sun_solaris2_4
const unsigned int loadExp = 15;
const unsigned int storeExp = 13;
const unsigned int prefeExp = 2;
const unsigned int accessExp = 26;
#endif

#ifdef rs6000_ibm_aix4_1
const unsigned int loadExp = 38;
const unsigned int storeExp = 32;
const unsigned int prefeExp = 0;
const unsigned int accessExp = 70;
#endif

#ifdef i386_unknown_linux2_0
const unsigned int loadExp = 0;
const unsigned int storeExp = 0;
const unsigned int prefeExp = 0;
const unsigned int accessExp = 0;

int loadsnstores(int x, int y, int z)
{
  return x + y + z;
}
#endif

#ifdef mips_sgi_irix6_4
const unsigned int loadExp = 0;
const unsigned int storeExp = 0;
const unsigned int prefeExp = 0;
const unsigned int accessExp = 0;

int loadsnstores(int x, int y, int z)
{
  return x + y + z;
}
#endif

#ifdef alpha_dec_osf4_0
const unsigned int loadExp = 0;
const unsigned int storeExp = 0;
const unsigned int prefeExp = 0;
const unsigned int accessExp = 0;

int loadsnstores(int x, int y, int z)
{
  return x + y + z;
}
#endif

#ifdef i386_unknown_nt4_0
const unsigned int loadExp = 0;
const unsigned int storeExp = 0;
const unsigned int prefeExp = 0;
const unsigned int accessExp = 0;

int loadsnstores(int x, int y, int z)
{
  return x + y + z;
}
#endif


#define passorfail(i,p,d,r) if((p)) { \
                              printf("Passed test #%d (%s)\n", (i), (d)); \
                              passedTest[(i)] = TRUE; \
                            } else { \
                              printf("\n**Failed** test #%d (%s): %s\n", (i), (d), (r)); \
                            }

void check0()
{
  passorfail(0, result_of_loadsnstores == 9, "function integrity", "function corrupted!");
}

void check1()
{
  passorfail(1, loadCnt == loadExp, "load instrumentation", "load counter seems wrong.");
}

void check2()
{
  passorfail(2, storeCnt == storeExp, "store instrumentation", "store counter seems wrong.");
}

void check3()
{
  passorfail(3, prefeCnt == prefeExp, "prefetch instrumentation", "prefetch counter seems wrong.");
}

void check4()
{
  passorfail(4, accessCnt == accessExp, "access instrumentation", "access counter seems wrong.");
}

/* functions called by the simple instrumentation points */
void countLoad()
{
  ++loadCnt;
}

void countStore()
{
  ++storeCnt;
}

void countPrefetch()
{
  ++prefeCnt;
}

void countAccess()
{
  ++accessCnt;
}


int main(int iargc, char *argv[])
{                                       /* despite different conventions */
  unsigned argc=(unsigned)iargc;      /* make argc consistently unsigned */
  unsigned int i, j;
  unsigned int testsFailed = 0;
  int x;
  
  for (j=0; j <= MAX_TEST; j++) {
    passedTest [j] = FALSE;
    runTest [j] = FALSE;
  }
  
  for (i=1; i < argc; i++) {
    if (!strcmp(argv[i], "-verbose")) {
      debugPrint = TRUE;
    } else if (!strcmp(argv[i], "-runall")) {
      dprintf("selecting all tests\n");
      for (j=1; j <= MAX_TEST; j++) runTest[j] = TRUE;
    } else if (!strcmp(argv[i], "-run")) {
      for (j=i+1; j < argc; j++) {
        unsigned int testId;
        if ((testId = atoi(argv[j]))) {
          if ((testId > 0) && (testId <= MAX_TEST)) {
            dprintf("selecting test %d\n", testId);
            runTest[testId] = TRUE;
          } else {
            printf("invalid test %d requested\n", testId);
            exit(-1);
          }
        } else {
          /* end of test list */
          break;
        }
      }
      i=j-1;
    } else {
      fprintf(stderr, "%s\n", USAGE);
      exit(-1);
    }
  }

  if (argc==1) exit(0);

  result_of_loadsnstores = loadsnstores(2,3,4);
  dprintf("0x%x %d %d %d %d\n", result_of_loadsnstores, loadCnt, storeCnt, prefeCnt, accessCnt);

  /* all test run anyway, we just validate them here */

  check0(x); /* integrity check */
  if (runTest[1]) check1();
  if (runTest[2]) check2();
  if (runTest[3]) check3();
  if (runTest[4]) check4();

  /* See how we did running the tests. */
  for (i=1; i <= MAX_TEST; i++) {
    if (runTest[i] && !passedTest[i]) testsFailed++;
  }

  if (!testsFailed) {
    printf("All tests passed\n");
  } else {
    printf("**Failed** %d test%c\n",testsFailed,(testsFailed>1)?'s':' ');
  }

  dprintf("Mutatee %s terminating.\n", argv[0]);
  return (testsFailed ? 127 : 0);
}
