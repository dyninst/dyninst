/* Test application (Mutatee) */

/* $Id: test1.mutateeCommon.h,v 1.3 2003/01/02 19:52:01 schendel Exp $ */

/* Empty functions are sometimes compiled too tight for entry and exit
   points.  The following macro is used to flesh out these
   functions. (expanded to use on all platforms for non-gcc compilers jkh 10/99)
 */
static volatile int dummy3__;

#define DUMMY_FN_BODY \
  int dummy1__ = 1; \
  int dummy2__ = 2; \
  dummy3__ = dummy1__ + dummy2__

/* control debug printf statements */
#define dprintf	if (debugPrint) printf

#define MAX_TEST 35

extern int kludge;
extern int debugPrint;
extern int runTest[MAX_TEST+1];
extern int passedTest[MAX_TEST+1];

extern void verifyScalarValue(const char *name, int a, int value, int testNum,
                              const char *testName);
extern void verifyValue(const char *name, int *a, int index, int value, 
                        int tst, const char *tn);

#define TRUE	1
#define FALSE	0

#define RET13_1 1300100

#define RAN17_1 1701000

#define RET17_1 1700100
#define RET17_2 1700200

#define MAGIC19_1 1900100
#define MAGIC19_2 1900200

/* These are copied in libtestA.c and libtestB.c */
#define MAGIC22_1   2200100
#define MAGIC22_2   2200200
#define MAGIC22_3   2200300
#define MAGIC22_4   2200400
#define MAGIC22_5A  2200510
#define MAGIC22_5B  2200520
#define MAGIC22_6   2200600
#define MAGIC22_7   2200700

#define TEST20_A 3
#define TEST20_B 4.3
#define TEST20_C 7
#define TEST20_D 6.4
#define TEST20_TIMES 41


#if defined(sparc_sun_solaris2_4) || defined(alpha_dec_osf4_0)
#if defined(mips_sgi_irix6_4) && (_MIPS_SIM == _MIPS_SIM_NABI32)
static const char *libNameA = "libtestA_n32.so";
#else
static const char *libNameA = "libtestA.so";
#endif
#endif

