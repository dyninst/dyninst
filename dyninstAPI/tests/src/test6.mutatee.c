/* $Id: test6.mutatee.c,v 1.15 2002/08/06 23:20:54 gaburici Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define dprintf	if (debugPrint) printf
int debugPrint = 0;

#define MAX_TEST 6

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

extern long loadsnstores(long, long, long); /* ILP32 & LP64 */
int result_of_loadsnstores;

unsigned int loadCnt = 0;
unsigned int storeCnt = 0;
unsigned int prefeCnt = 0;
unsigned int accessCnt = 0;

unsigned int accessCntEA = 0;
unsigned int accessCntBC = 0;

int doomEA = 0;
int doomBC = 0;

#ifdef sparc_sun_solaris2_4
/* const */ unsigned int loadExp=15;
/* const */ unsigned int storeExp=13;
/* const */ unsigned int prefeExp=2;
/* const */ unsigned int accessExp=26;

unsigned int bcExp[] = { 4,1,2,8,4,1,1,  4,8,4,  4,4,8,8,16,
                         0,0,  1,2,4,8,  4,8,16,4,8 };

int eaExpOffset[] =    { 0,3,2,0,0,3,3,  0,0,0,  0,0,0,0,0,
                         0,0,  7,6,4,0,  0,0,0,4,0 };


extern void* eaExp[]; /* forward */
extern int divarw;
extern float dfvars;
extern double dfvard;
extern long double dfvarq;

/* _inline */ void init_test_data()
{
  int i=0;

  /*
  printf("&divarw = %p\n", &divarw);
  printf("&dfvars = %p\n", &dfvars);
  printf("&dfvard = %p\n", &dfvard);
  printf("&dfvarq = %p\n", &dfvarq);
  */

  for(; i<10; ++i)
    eaExp[i] = (void*)((unsigned long)&divarw + eaExpOffset[i]);

  for(; i<12; ++i)
    eaExp[i] = (void*)((unsigned long)&dfvars + eaExpOffset[i]);

  for(; i<14; ++i)
    eaExp[i] = (void*)((unsigned long)&dfvard + eaExpOffset[i]);

  for(; i<17; ++i)
    eaExp[i] = (void*)((unsigned long)&dfvarq + eaExpOffset[i]);

  for(; i<21; ++i)
    eaExp[i] = (void*)((unsigned long)&divarw + eaExpOffset[i]);

  eaExp[i] = (void*)((unsigned long)&dfvars + eaExpOffset[i]);
  ++i;

  eaExp[i] = (void*)((unsigned long)&dfvard + eaExpOffset[i]);
  ++i;

  eaExp[i] = (void*)((unsigned long)&dfvarq + eaExpOffset[i]);
  ++i;

  for(; i<26; ++i)
    eaExp[i] = (void*)((unsigned long)&divarw + eaExpOffset[i]);
}
#endif

#ifdef rs6000_ibm_aix4_1
const unsigned int loadExp=41;
const unsigned int storeExp=32;
const unsigned int prefeExp=0;
const unsigned int accessExp=73;

unsigned int bcExp[] = { 4,  1,1,1,1,  2,2,2,2,  2,2,2,2,  4,4,4,4,
			 4,4,4,  8,8,8,8,  1,1,1,1,  2,2,2,2,
			 4,4,4,4,  8,8,8,8,  2,4,2,4,  76,76,24,20,
			 20,20,  4,4,8,8,  4,  4,4,4,4,  4,  8,8,8,8,
			 4,4,4,4,  8,8,8,8,  4 };

int eaExpOffset[] =    { 0, 17,3,1,2,  0,4,2,0,  2,2,2,2,  0,4,4,4,
			 4,12,2,  0,0,0,0,  3,1,1,1,  2,6,2,2,
			 0,4,4,4,  0,0,0,0,  0,0,0,0,  -76,-76,-24,-24,
			 -20,-20,    0,0,0,0,  0,  0,4,0,4,  0,  0,8,8,8,
			 4,4,0,0,  0,8,8,8,  0 };

extern void* eaExp[]; /* forward */
extern int divarw;
extern float dfvars;
extern double dfvard;

extern void* gettoc();
extern void* getsp();

#ifdef __GNUC__
#define _inline inline
#endif

/* _inline */ void init_test_data()
{
  int i;

  void *toc = gettoc();
  void *sp  = getsp(1,2,3);

  dprintf("&divarw = %p\n", &divarw);
  dprintf("&dfvars = %p\n", &dfvars);
  dprintf("&dfvard = %p\n", &dfvard);

  dprintf("toc = %p\n", toc);
  dprintf("sp = %p\n", sp);

  eaExp[0] = toc; /* assuming that TOC entries are not reordered */

  for(i=1; i<44; ++i)
    eaExp[i] = (void*)((unsigned long)&divarw + eaExpOffset[i]);

  for(i=44; i<50; ++i)
    eaExp[i] = (void*)((unsigned long)sp + eaExpOffset[i]);; /* SP */
  
  for(i=50; i<54; ++i)
    eaExp[i] = (void*)((unsigned long)&divarw + eaExpOffset[i]);

  eaExp[54] = (void*)((unsigned long)toc + sizeof(void*)); /* TOC */

  for(i=55; i<59; ++i)
    eaExp[i] = (void*)((unsigned long)&dfvars + eaExpOffset[i]);

  eaExp[59] = (void*)((unsigned long)toc + 2*sizeof(void*)); /* TOC */

  for(i=60; i<64; ++i)
    eaExp[i] = (void*)((unsigned long)&dfvard + eaExpOffset[i]);

  for(i=64; i<68; ++i)
    eaExp[i] = (void*)((unsigned long)&dfvars + eaExpOffset[i]);

  for(i=68; i<72; ++i)
    eaExp[i] = (void*)((unsigned long)&dfvard + eaExpOffset[i]);
  
  eaExp[72] = (void*)((unsigned long)&dfvars + eaExpOffset[i]);
}
#endif

#if defined(i386_unknown_linux2_0) || defined(i386_unknown_nt4_0)
unsigned int loadExp=60;
unsigned int storeExp=23;
unsigned int prefeExp=0;
unsigned int accessExp=78;

struct reduction {
  unsigned int loadRed;
  unsigned int storeRed;
  unsigned int prefeRed;
  unsigned int axsRed;
  unsigned int axsShift;
};

const struct reduction mmxRed = { 2, 1, 0, 3, 48 };
const struct reduction sseRed = { 2, 0, 0, 2, 51 };
const struct reduction sse2Red = { 2, 0, 0, 2, 53 };
const struct reduction amdRed = { 2, 0, 0, 2, 55 };

int eaExpOffset[] =    { 0,0,0,0,  0,0,0,0,0,0,0,  4,8,4,8,4,8,4,  0,
                         0,4,8,12,0,4,8,  12,0,8,8,8,0,4,8,4,  0,  4,4,4,0,4,0,4,8,0,0,4,0,
                         0,8,0,  0,0,  0,0,  0,8,  0,12,0,0,  0,0,0,0,4,8,  0,0,0,2,4,8,  0,0,
                         0,0 };

extern void* eaExp[]; /* forward */

unsigned int bcExp[] = { 4,4,4,4,  4,4,4,4,4,4,4,  4,4,4,4,4,4,4,  4,
                         4,4,4,4,4,4,4,   4,4,4,4,4,4,4,4,4,   4,  4,4,1,1,4,4,4,4,4,1,4,4,
                         4,8,8,  16,4, 16,8, 8,8,  4,4,4,4,   4,8,10,2,4,8, 4,8,10,2,4,8, 2,2,
                         28,28,  4,4,4 };

extern int ia32features();
extern int amd_features();

extern int divarw;
extern float dfvars;
extern double dfvard;
extern long double dfvart; /* 10 byte hopefully, but it shouldn't matter... */
extern unsigned char dlarge[512];

#define CAP_MMX   (1<<23)
#define CAP_SSE   (1<<25)
#define CAP_SSE2  (1<<26)
#define CAP_3DNOW (1<<31)

void reduce(const struct reduction x)
{
  unsigned int i;

  loadExp  -= x.loadRed;
  storeExp -= x.storeRed;
  prefeExp -= x.prefeRed;

  for(i=x.axsShift; i<accessExp; ++i)
    eaExp[i] = eaExp[i+x.axsRed];

  for(i=x.axsShift; i<accessExp; ++i)
    bcExp[i] = bcExp[i+x.axsRed];

  accessExp -= x.axsRed;
}


void init_test_data()
{
  int caps, i;

  dprintf("&divarw = %p\n", &divarw);
  dprintf("&dfvars = %p\n", &dfvars);
  dprintf("&dfvard = %p\n", &dfvard);
  dprintf("&dfvart = %p\n", &dfvart);
  dprintf("&dlarge = %p\n", &dlarge);

  for(i=4; i<15; ++i)
    eaExp[i] = (void*)((unsigned long)&divarw + eaExpOffset[i]); /* skip ebp for now */
  for(i=16; i<18; ++i)
    eaExp[i] = (void*)((unsigned long)&divarw + eaExpOffset[i]);
  for(i=19; i<26; ++i)
    eaExp[i] = (void*)((unsigned long)&divarw + eaExpOffset[i]);
  i=26;
  eaExp[i] = (void*)((unsigned long)&divarw + eaExpOffset[i]);
  for(i=28; i<35; ++i)
    eaExp[i] = (void*)((unsigned long)&divarw + eaExpOffset[i]);
  for(i=36; i<51; ++i)
    eaExp[i] = (void*)((unsigned long)&divarw + eaExpOffset[i]);
  for(i=51; i<53; ++i)
    eaExp[i] = (void*)((unsigned long)&dfvars + eaExpOffset[i]);
  for(i=53; i<55; ++i)
    eaExp[i] = (void*)((unsigned long)&dfvard + eaExpOffset[i]);
  for(i=55; i<57; ++i)
    eaExp[i] = (void*)((unsigned long)&dfvars + eaExpOffset[i]);
  for(i=57; i<60; ++i)
    eaExp[i] = (void*)((unsigned long)&divarw + eaExpOffset[i]);
  i=60;
  eaExp[i] = (void*)((unsigned long)&dfvars + eaExpOffset[i]);
  i=61;
  eaExp[i] = (void*)((unsigned long)&dfvars + eaExpOffset[i]);
  i=62;
  eaExp[i] = (void*)((unsigned long)&dfvard + eaExpOffset[i]);
  i=63;
  eaExp[i] = (void*)((unsigned long)&dfvart + eaExpOffset[i]);
  for(i=64; i<67; ++i)
    eaExp[i] = (void*)((unsigned long)&divarw + eaExpOffset[i]);
  i=67;
  eaExp[i] = (void*)((unsigned long)&dfvars + eaExpOffset[i]);
  i=68;
  eaExp[i] = (void*)((unsigned long)&dfvard + eaExpOffset[i]);
  i=69;
  eaExp[i] = (void*)((unsigned long)&dfvart + eaExpOffset[i]);
  for(i=70; i<75; ++i)
    eaExp[i] = (void*)((unsigned long)&divarw + eaExpOffset[i]);
  for(i=75; i<77; ++i)
    eaExp[i] = (void*)((unsigned long)&dlarge + eaExpOffset[i]);

  /* Order of reductions matters! */

  caps = amd_features();
  if(!(caps & CAP_3DNOW))
    reduce(amdRed);
  caps = ia32features();
  if(!(caps & CAP_SSE2))
    reduce(sse2Red);
  if(!(caps & CAP_SSE))
    reduce(sseRed);
  if(!(caps & CAP_MMX))
    reduce(mmxRed);
}
#endif


#ifdef ia64_unknown_linux2_4
#define loadExp 0
#define storeExp 0
#define prefeExp 0
#define accessExp 1

long loadsnstores(long x, long y, long z)
{
  return x + y + z;
}

unsigned int bcExp[] = { 0 };

void init_test_data()
{
}
#endif

#ifdef mips_sgi_irix6_4
#define loadExp 0
#define storeExp 0
#define prefeExp 0
#define accessExp 1

long loadsnstores(long x, long y, long z)
{
  return x + y + z;
}

unsigned int bcExp[] = { 0 };

void init_test_data()
{
}
#endif

#ifdef alpha_dec_osf4_0
#define loadExp 0
#define storeExp 0
#define prefeExp 0
#define accessExp 1

long loadsnstores(long x, long y, long z)
{
  return x + y + z;
}

unsigned int bcExp[] = { 0 };

void init_test_data()
{
}
#endif

void* eaList[1000];
unsigned int bcList[1000];

void* eaExp[1000];

/* Sun Forte/WorkShop cc releases older than 6.2 do not like these defines: */
#if !defined(__SUNPRO_C) || (__SUNPRO_C >= 0x530)
#define passorfail(i,p,d,r) if((p)) { \
                              printf("Passed test #%d (%s)\n", (i), (d)); \
                              passedTest[(i)] = TRUE; \
                            } else { \
                              printf("\n**Failed** test #%d (%s): %s\n", (i), (d), (r)); \
                            }

#define skiptest(i,d) { printf("Skipping test #%d (%s)\n", (i), (d)); \
                        printf("    not implemented on this platform\n"); \
                        passedTest[(i)] = TRUE; }
#else
void passorfail(int i, int p, char* d, char* r)
{
  if(p) {
    printf("Passed test #%d (%s)\n", (i), (d));
    passedTest[(i)] = TRUE;
  } else {
    printf("\n**Failed** test #%d (%s): %s\n", (i), (d), (r));
  }
}

void skiptest(int i, char* d)
{
  printf("Skipping test #%d (%s)\n", (i), (d));
  printf("    not implemented on this platform\n");
  passedTest[(i)] = TRUE;
}
#endif

int validateEA(void* ea1[], void* ea2[])
{
  int ok = 1;
  unsigned int i=0;

  for(; i<accessExp; ++i) {
    ok = (ok && ((ea1[i] == ea2[i]) || ea1[i] == NULL));
    if(!ok) {
      printf("Validation failed at access #%d. Expecting: %x. Got: %x.\n", i+1, ea1[i], ea2[i]);
      return 0;
    }
  }
  return 1;
}

int validateBC(unsigned int bc1[], unsigned int bc2[])
{
  int ok = 1;
  unsigned int i=0;

  for(; i<accessExp; ++i) {
    ok = (ok && (bc1[i] == bc2[i]));
    if(!ok) {
printf("Validation failed at access #%d. Expecting: %d. Got: %d.\n", i+1, bc1[i], bc2[i]);
      return 0;
    }
  }
  return 1;
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

void check5()
{
#if !defined(sparc_sun_solaris2_4) && !defined(rs6000_ibm_aix4_1) && !defined(i386_unknown_linux2_0) && !defined(i386_unknown_nt4_0)
  skiptest(5, "instrumentation w/effective address snippet");
#else
  passorfail(5, !doomEA && validateEA(eaExp, eaList),
	     "effective address snippet", "address sequences are different");
#endif
}

void check6()
{
#if !defined(sparc_sun_solaris2_4) && !defined(rs6000_ibm_aix4_1) && !defined(i386_unknown_linux2_0) && !defined(i386_unknown_nt4_0)
  skiptest(6, "instrumentation w/byte count snippet");
#else
  passorfail(6, !doomBC && validateBC(bcExp, bcList),
	     "count accessed bytes snippet", "count sequences are different");
#endif
}


/* functions called by the simple instrumentation points */
void countLoad()
{
  /* fprintf(stderr, "Load!\n"); */
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


/* functions called by the effective address/byte count instrumentation points */
void listEffAddr(void* addr)
{
  if(accessCntEA < accessExp)
    eaList[accessCntEA] = addr;
  else
    doomEA = 1;
  accessCntEA++;
  dprintf("EA[%d]:%p ", accessCntEA, addr);
}

void listByteCnt(unsigned int count)
{
  if(accessCntBC < accessExp)
    bcList[accessCntBC] = count;
  else
    doomBC = 1;
  accessCntBC++;
  dprintf("BC[%d]:%d ", accessCntBC, count);
}


int main(int iargc, char *argv[])
{                                       /* despite different conventions */
  unsigned argc=(unsigned)iargc;      /* make argc consistently unsigned */
  unsigned int i, j;
  unsigned int testsFailed = 0;
  
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

  loadCnt = 0;
  storeCnt = 0;
  prefeCnt = 0;
  accessCnt = 0;

  result_of_loadsnstores = loadsnstores(2,3,4);
  dprintf("\nresult=0x%x loads=%d stores=%d prefetches=%d accesses=%d\n",
          result_of_loadsnstores, loadCnt, storeCnt, prefeCnt, accessCnt);

  init_test_data();

  /* check0(); integrity check skipped needs more work to be really usefull */
  if (runTest[1]) check1();
  if (runTest[2]) check2();
  if (runTest[3]) check3();
  if (runTest[4]) check4();
  if (runTest[5]) check5();
  if (runTest[6]) check6();

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
