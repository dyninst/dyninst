/* Test application (Mutatee) */

/* $Id: test1.mutatee.c,v 1.81 2002/12/05 01:38:40 buck Exp $ */

#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include "mutatee_util.h"

#ifdef i386_unknown_nt4_0
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "test1.h"
#ifdef __cplusplus
#include "cpp_test.h"
#include <iostream.h>
#endif

#if defined(sparc_sun_solaris2_4) || \
    defined(alpha_dec_osf4_0) || \
    defined(i386_unknown_linux2_0) || \
    defined(i386_unknown_solaris2_5) || \
    defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */
#include <dlfcn.h> /* For replaceFunction test */
#endif

#include "test1.mutateeCommon.h"

#ifdef __cplusplus
int mutateeCplusplus = 1;
#else
int mutateeCplusplus = 0;
#endif

int mutateeFortran = 0;
int mutateeF77 = 0;


struct struct26_1 {
    int field1;
    int field2;
};

struct struct26_2 {
    int field1;
    int field2;
    int field3[10];
    struct struct26_1 field4;
};

int globalVariable1_1 = 0;
int globalVariable3_1 = 31;
int globalVariable4_1 = 41;
int globalVariable5_1 = 51;
int globalVariable5_2 = 51;

int constVar0 = 0;
int constVar1 = 1;
int constVar2 = 2;
int constVar3 = 3;
int constVar4 = 4;
int constVar5 = 5;
int constVar6 = 6;
int constVar7 = 7;
int constVar9 = 9;
int constVar10 = 10;
int constVar60 = 60;
int constVar64 = 64;
int constVar66 = 66;
int constVar67 = 67;

int globalVariable6_1 = (int)0xdeadbeef;
int globalVariable6_2 = (int)0xdeadbeef;
int globalVariable6_3 = (int)0xdeadbeef;
int globalVariable6_4 = (int)0xdeadbeef;
int globalVariable6_5 = (int)0xdeadbeef;
int globalVariable6_6 = (int)0xdeadbeef;
int globalVariable6_1a = (int)0xdeadbeef;
int globalVariable6_2a = (int)0xdeadbeef;
int globalVariable6_3a = (int)0xdeadbeef;
int globalVariable6_4a = (int)0xdeadbeef;
int globalVariable6_5a = (int)0xdeadbeef;
int globalVariable6_6a = (int)0xdeadbeef;

int globalVariable7_1 = 71, globalVariable7_2 = 71,
    globalVariable7_3 = 71, globalVariable7_4 = 71,
    globalVariable7_5 = 71, globalVariable7_6 = 71,
    globalVariable7_7 = 71, globalVariable7_8 = 71,
    globalVariable7_9 = 71, globalVariable7_10 = 71,
    globalVariable7_11 = 71, globalVariable7_12 = 71,
    globalVariable7_13 = 71, globalVariable7_14 = 71,
    globalVariable7_15 = 71, globalVariable7_16 = 71;

int globalVariable7_1a = 73, globalVariable7_2a = 73,
    globalVariable7_3a = 73, globalVariable7_4a = 73,
    globalVariable7_5a = 73, globalVariable7_6a = 73,
    globalVariable7_7a = 73, globalVariable7_8a = 73,
    globalVariable7_9a = 73, globalVariable7_10a = 73,
    globalVariable7_11a = 73, globalVariable7_12a = 73,
    globalVariable7_13a = 73, globalVariable7_14a = 73,
    globalVariable7_15a = 73, globalVariable7_16a = 73;

int globalVariable8_1 = 1;

int globalVariable10_1 = 0, globalVariable10_2 = 0,
    globalVariable10_3 = 0, globalVariable10_4 = 0;

int globalVariable11_1 = 0, globalVariable11_2 = 0,
    globalVariable11_3 = 0, globalVariable11_4 = 0, globalVariable11_5 = 0;

int globalVariable12_1 = 0;

int globalVariable13_1 = 0;

int globalVariable14_1 = 0;
int globalVariable14_2 = 0;

int globalVariable15_1 = 0;
int globalVariable15_2 = 0;
int globalVariable15_3 = 0;
int globalVariable15_4 = 0;

int globalVariable16_1 = 0;
int globalVariable16_2 = 0;
int globalVariable16_3 = 0;
int globalVariable16_4 = 0;
int globalVariable16_5 = 0;
int globalVariable16_6 = 0;
int globalVariable16_7 = 0;
int globalVariable16_8 = 0;
int globalVariable16_9 = 0;
int globalVariable16_10 = 0;

int globalVariable17_1 = 0;
int globalVariable17_2 = 0;

int globalVariable18_1 = 42;

int globalVariable19_1 = (int)0xdeadbeef;
int globalVariable19_2 = (int)0xdeadbeef;

int globalVariable20_1 = (int)0xdeadbeef;
double globalVariable20_2 = 0.0;

int globalVariable22_1 = 0;
int globalVariable22_2 = 0;
int globalVariable22_3 = 0;
int globalVariable22_4 = 0;

unsigned globalVariable30_1 = 0;
unsigned globalVariable30_2 = 0;

unsigned globalVariable30_3 = 0;
unsigned globalVariable30_4 = 0;
unsigned globalVariable30_5 = 0;
unsigned globalVariable30_6 = 0;

int globalVariable31_1 = 0;
int globalVariable31_2 = 0;
int globalVariable31_3 = 0;
int globalVariable31_4 = 0;

int globalVariable32_1 = 0;
int globalVariable32_2 = 0;
int globalVariable32_3 = 0;
int globalVariable32_4 = 0;

/*
 * Determine if two doubles are close to being equal (for our purposes, that
 * means to ten decimal places).
 */
int eq_doubles(double a, double b)
{
    double diff = a - b;

    if (diff < 0) diff = -diff;

    if (diff < 0.00000000001) return 1;
    else return 0;
}

void call1_1()
{
    dprintf("call1() called - setting globalVariable1_1 = 11\n");
    globalVariable1_1 = 11;
}

void call2_1(int arg1, int arg2, char *arg3, void *arg4)
{
    assert(TEST_PTR_SIZE == sizeof(void *));

    if ((arg1 == 1) && (arg2 == 2) && (!strcmp(arg3, "testString2_1")) &&
	(arg4 == TEST_PTR)) {
	printf("Passed test #2 (four parameter function)\n");
	passedTest[2] = TRUE;
    } else {
	printf("**Failed** test #2 (four parameter function)\n");
	if (arg1 != 1)
	    printf("    arg1 = %d, should be 1\n", arg1);
	if (arg2 != 2)
	    printf("    arg2 = %d, should be 2\n", arg2);
	if (strcmp(arg3, "testString2_1"))
	    printf("    arg3 = %s, should be \"testString2_1\"\n", arg3);
	if (arg4 != TEST_PTR)
	    printf("    arg4 = 0x%p, should be 0x%p\n", arg4, TEST_PTR);
    }
}

void call3_1(int arg1, int arg2)
{
    if ((arg1 == 31) && (arg2 == 32))  {
	printf("Passed test #3 (passing variables to functions)\n");
	passedTest[3] = TRUE;
    } else {
	printf("**Failed** test #3 (passing variables to functions)\n");
	printf("    arg1 = %d, should be 31\n", arg1);
	printf("    arg2 = %d, should be 32\n", arg2);
    }
}

int call9_1(int p1, int p2, int p3, int p4, int p5)
{
    int x;
    x = (((p1 + p2) + (p3 + p4) + (p5)));
    if (x != (91 + 92 + 93 + 94 + 95 )) {
      printf("**Failed** test case #9 (preserve registers - funcCall)\n");
      if (p1 != 91) printf("    call9_1 parameter 1 is %d not 91\n", p1);
      if (p2 != 92) printf("    call9_1 parameter 2 is %d not 92\n", p2);
      if (p3 != 93) printf("    call9_1 parameter 3 is %d not 93\n", p3);
      if (p4 != 94) printf("    call9_1 parameter 4 is %d not 94\n", p4);
      if (p5 != 95) printf("    call9_1 parameter 5 is %d not 95\n", p5);
      exit(-1);
    }
    dprintf("inside call9_1\n");
    return x;
}

void call10_1()
{
    if (globalVariable10_4 == 0) {
	globalVariable10_4 = 1;
	globalVariable10_1 = 1;
    }
}


void call10_2()
{
    if (globalVariable10_4 == 1) {
	globalVariable10_4 = 2;
	globalVariable10_2 = 1;
    }
}

void call10_3()
{
    if (globalVariable10_4 == 2) {
	globalVariable10_4 = 3;
	globalVariable10_3 = 1;
    }
}

void call11_1()
{
    if (globalVariable11_1 == 0) globalVariable11_2 = 1;
}

void call11_2()
{
    if (globalVariable11_1 == 1) globalVariable11_3 = 1;
}

void call11_3()
{
    if (globalVariable11_1 == 2) globalVariable11_4 = 1;
}

void call11_4()
{
    if (globalVariable11_1 == 3) globalVariable11_5 = 1;

    if (globalVariable11_2 && globalVariable11_3 &&
	globalVariable11_4 && globalVariable11_5) {
        printf("Passed test #11 (snippets at entry,exit,call)\n");
	passedTest[11] = TRUE;
    } else {
        printf("**Failed test #11 (snippets at entry,exit,call)\n");
	if (!globalVariable11_2)
	    printf("    entry snippet not called at the correct time\n");
	if (!globalVariable11_3)
	    printf("    pre call snippet not called at the correct time\n");
	if (!globalVariable11_4)
	    printf("    post call snippet not called at the correct time\n");
	if (!globalVariable11_5)
	    printf("    exit snippet not called at the correct time\n");
    }
}

void call12_1()
{
    globalVariable12_1++;
}

void call13_1(int a1, int a2, int a3, int a4, int a5)
{
    if (a1 == 131) globalVariable13_1 |= 1;
    if (a2 == 132) globalVariable13_1 |= 2;
    if (a3 == 133) globalVariable13_1 |= 4;
    if (a4 == 134) globalVariable13_1 |= 8;
    if (a5 == 135) globalVariable13_1 |= 16;
    dprintf("a1 = %d\n", a1);
    dprintf("a2 = %d\n", a2);
    dprintf("a3 = %d\n", a3);
    dprintf("a4 = %d\n", a4);
    dprintf("a5 = %d\n", a5);
}

void call13_2(int ret)
{
    if (ret == RET13_1) globalVariable13_1 |= 32;
}

void call14_1()
{
    globalVariable14_1 = 1;
}

void call15_1()
{
    globalVariable15_1++;
}

void call15_2()
{
    globalVariable15_2++;
}

void call15_3()
{
    globalVariable15_3++;
}

int call17_1(int p1)
{
     /* make sure the function uses lots of registers */

     int a1, a2, a3, a4, a5, a6, a7;

     dprintf("call17_1 (p1=%d)\n", p1);
     assert(p1!=0); /* shouldn't try to divide by zero! */
     assert(p1==1); /* actually only expect calls with p1==1 */

     a1 = p1;
     a2 = a1 + p1;
     a3 = a1 * a2;
     a4 = a3 / p1;
     a5 = a4 + p1;
     a6 = a5 + a1;
     a7 = a6 + p1;

     dprintf("call17_1 (ret=%d)\n", a7);

     return a7;
}

int call17_2(int p1)
{
     /* make sure the function uses lots of registers */

     int a1, a2, a3, a4, a5, a6, a7;

     dprintf("call17_2 (p1=%d)\n", p1);
     assert(p1!=0); /* shouldn't try to divide by zero! */
     assert(p1==1); /* actually only expect calls with p1==1 */

     a1 = p1;
     a2 = a1 + p1;
     a3 = a1 * a2;
     a4 = a3 / p1;
     a5 = a4 + p1;
     a6 = a5 + a1;
     a7 = a6 + p1;
     globalVariable17_2 = RAN17_1;

     dprintf("call17_2 (ret=%d)\n", a7);

     return a7;
}

void call19_1()
{
    globalVariable19_1 = MAGIC19_1;
}

void call19_2()
{
    globalVariable19_2 = MAGIC19_2;
}

volatile int ta = TEST20_A;
volatile double tb = TEST20_B;

void call20_1()
{
    globalVariable20_1 = ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta+
			 (ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta+
			 (ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta+
			 (ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta+(ta
			 ))))))))))))))))))))))))))))))))))))))));

    globalVariable20_2 = tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb+
			 (tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb+
			 (tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb+
			 (tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb+(tb
			 ))))))))))))))))))))))))))))))))))))))));
}

void call22_1(int x)
{
     globalVariable22_1 += x;
     globalVariable22_1 += MAGIC22_1;
}

void call22_2(int x)
{
     globalVariable22_1 += x;
     globalVariable22_1 += MAGIC22_2;
}

void call22_3(int x)
{
     globalVariable22_2 += x;
     globalVariable22_2 += MAGIC22_3;
}

void call22_7(int x)
{
     globalVariable22_4 += x;
     globalVariable22_4 += MAGIC22_7;
}

#if defined(sparc_sun_solaris2_4) || \
    defined(i386_unknown_solaris2_5) || \
    defined(i386_unknown_linux2_0) || \
    defined(i386_unknown_nt4_0) ||\
    defined(rs6000_ibm_aix4_1) || \
    defined(alpha_dec_osf4_0) || \
    defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */

/* this function has to be only 1 line for test30 to pass */
/* these two lines has to be together otherwise test30 will fail */
unsigned globalVariable30_7 = __LINE__;
void call30_1(){ globalVariable30_1 = __LINE__; globalVariable30_2 = (unsigned)call30_1;}

#endif

/*
 * This is a series of nearly empty functions to attach code to
 */

/*
 * Start of Test #1
 */
void func1_1()
{
    void func1_2();

    dprintf("Value of globalVariable1_1 is %d.\n", globalVariable1_1);

    func1_2();

    dprintf("Value of globalVariable1_1 is now %d.\n", globalVariable1_1);

    if (globalVariable1_1 == 11) {
        printf("\nPassed test #1 (zero arg function call)\n");
		passedTest[1] = TRUE;
    } else {
        printf("\n**Failed** test #1 (zero arg function call)\n");
    }
}

void func1_2() { dprintf("func1_2 () called\n"); }

/*
 * Start of Test #2
 */
void func2_1() { dprintf("func2_1 () called\n"); }

/*
 * Start of Test #3
 */
void func3_1() { dprintf("func3_1 () called\n"); }

/*
 * Start of Test #4 - sequence
 *	Run two expressions and verify correct ordering.
 */
void func4_1()
{
    void func4_2();

    kludge = 1;	/* Here so that the following function call isn't the first
		   instruction */
    func4_2();
    if (globalVariable4_1 == 41) {
	printf("**Failed** test #4 (sequence)\n");
	printf("    none of the items were executed\n");
    } else if (globalVariable4_1 == 42) {
	printf("**Failed** test #4 (sequence)\n");
	printf("    first item was the last (or only) one to execute\n");
    } else if (globalVariable4_1 == 43) {
        printf("Passed test #4 (sequence)\n");
	passedTest[4] = TRUE;
    }
}

void func4_2() { dprintf("func4_1 () called\n"); }

/*
 * Start of Test #5 - if w.o. else
 *	Execute two if statements, one true and one false.
 */
void func5_1()
{
    void func5_2();
    func5_2();

    if ((globalVariable5_1 == 51) && (globalVariable5_2 == 53)) {
        printf("Passed test #5 (if w.o. else)\n");
		passedTest[5] = TRUE;
    } else {
	printf("**Failed** test #5 (if w.o. else)\n");
	if (globalVariable5_1 != 51) {
	    printf("    condition executed for false\n");
	}
	if (globalVariable5_2 != 53) {
	    printf("    condition not executed for true\n");
	}
    }
}

void func5_2() { dprintf("func5_1 () called\n"); }

/*
 * Start of Test #6 - arithmetic operators
 *	Verify arithmetic operators:
 *		constant integer addition
 *		constant integer subtraction
 *		constant integer divide (using large constants)
 *		constant integer divide (small consts)
 *		constant integer multiply
 *		constant comma operator
 *		variable integer addition
 *		variable integer subtraction
 *		variable integer divide (using large constants)
 *		variable integer divide (small consts)
 *		variable integer multiply
 *		variable comma operator
 *
 *	constant - use constant expressions
 *	variable - use variables in the expressions
 *
 */
void func6_1()
{
    void func6_2();
    func6_2();
    if ((globalVariable6_1 == 60+2) && (globalVariable6_2 == 64-1) &&
	(globalVariable6_3 == 66/3) && (globalVariable6_4 == 67/3) &&
	(globalVariable6_5 == 6 * 5) && (globalVariable6_6 == 3) &&
    	(globalVariable6_1a == 60+2) && (globalVariable6_2a == 64-1) &&
	(globalVariable6_3a == 66/3) && (globalVariable6_4a == 67/3) &&
	(globalVariable6_5a == 6 * 5) && (globalVariable6_6a == 3)) {
	printf("Passed test #6 (arithmetic operators)\n");
	passedTest[6] = TRUE;
    } else {
	printf("**Failed** test #6 (arithmetic operators)\n");
	if (globalVariable6_1 != 60+2)
	    printf("    addition error 60+2 got %d\n", globalVariable6_1);
	if (globalVariable6_2 != 64-1)
	    printf("    subtraction error 64-1 got %d\n", globalVariable6_2);
	if (globalVariable6_3 != 66/3)
	    printf("    division error 66/3 got %d\n", globalVariable6_3);
	if (globalVariable6_4 != 67/3)
	    printf("    division error 67/3 got %d\n", globalVariable6_4);
	if (globalVariable6_5 != 6 * 5)
	    printf("    mult error 6*5 got %d\n", globalVariable6_5);
	if (globalVariable6_6 != 3)
	    printf("    comma error 10,3 got %d\n", globalVariable6_6);
	if (globalVariable6_1a != 60+2)
	    printf("    addition error 60+2 got %d\n", globalVariable6_1a);
	if (globalVariable6_2a != 64-1)
	    printf("    subtraction error 64-1 got %d\n", globalVariable6_2a);
	if (globalVariable6_3a != 66/3)
	    printf("    division error 66/3 got %d\n", globalVariable6_3a);
	if (globalVariable6_4a != 67/3)
	    printf("    division error 67/3 got %d\n", globalVariable6_4a);
	if (globalVariable6_5a != 6 * 5)
	    printf("    mult error 6*5 got %d\n", globalVariable6_5a);
	if (globalVariable6_6a != 3)
	    printf("    comma error 10,3 got %d\n", globalVariable6_6a);
    }
}

void func6_2() { dprintf("func6_2 () called\n"); }

/*
 * Start Test Case #7 - relational operators
 *	Generate all relational operators (eq, gt, le, ne, ge, and, or)
 *	in both the true and false forms.
 */

void fail7Print(int tCase, int fCase, char *op)
{
    if (tCase != 72)
	printf(" operator %s was not true when it should be - const expr\n",
	    op);
    if (fCase != 71)
       printf(" operator %s was not false when it should be - const expr\n",
	    op);
}

void fail7aPrint(int tCase, int fCase, char *op)
{
    if (tCase != 74)
	printf(" operator %s was not true when it should be - var expr\n", op);
    if (fCase != 73)
	printf(" operator %s was not false when it should be - var expr\n",op);
}

void func7_1()
{
    void func7_2();

    func7_2();
    if ((globalVariable7_1 == 72) && (globalVariable7_2 == 71) &&
	(globalVariable7_3 == 72) && (globalVariable7_4 == 71) &&
	(globalVariable7_5 == 72) && (globalVariable7_6 == 71) &&
	(globalVariable7_7 == 72) && (globalVariable7_8 == 71) &&
	(globalVariable7_9 == 72) && (globalVariable7_10 == 71) &&
	(globalVariable7_11 == 72) && (globalVariable7_12 == 71) &&
	(globalVariable7_13 == 72) && (globalVariable7_14 == 71) &&
	(globalVariable7_15 == 72) && (globalVariable7_16 == 71) &&
        (globalVariable7_1a == 74) && (globalVariable7_2a == 73) &&
	(globalVariable7_3a == 74) && (globalVariable7_4a == 73) &&
	(globalVariable7_5a == 74) && (globalVariable7_6a == 73) &&
	(globalVariable7_7a == 74) && (globalVariable7_8a == 73) &&
	(globalVariable7_9a == 74) && (globalVariable7_10a == 73) &&
	(globalVariable7_11a == 74) && (globalVariable7_12a == 73) &&
	(globalVariable7_13a == 74) && (globalVariable7_14a == 73) &&
	(globalVariable7_15a == 74) && (globalVariable7_16a == 73)) {
	printf("Passed test #7 (relational operators)\n");
	passedTest[7] = TRUE;
    } else {
	printf("**Failed** test #7 (relational operators)\n");
	fail7Print(globalVariable7_1, globalVariable7_2, "BPatch_lt");
	fail7Print(globalVariable7_3, globalVariable7_4, "BPatch_eq");
	fail7Print(globalVariable7_5, globalVariable7_6, "BPatch_gt");
	fail7Print(globalVariable7_7, globalVariable7_8, "BPatch_le");
	fail7Print(globalVariable7_9, globalVariable7_10, "BPatch_ne");
	fail7Print(globalVariable7_11, globalVariable7_12, "BPatch_ge");
	fail7Print(globalVariable7_13, globalVariable7_14, "BPatch_and");
	fail7Print(globalVariable7_15, globalVariable7_16, "BPatch_or");

	fail7aPrint(globalVariable7_1a, globalVariable7_2a, "BPatch_lt");
	fail7aPrint(globalVariable7_3a, globalVariable7_4a, "BPatch_eq");
	fail7aPrint(globalVariable7_5a, globalVariable7_6a, "BPatch_gt");
	fail7aPrint(globalVariable7_7a, globalVariable7_8a, "BPatch_le");
	fail7aPrint(globalVariable7_9a, globalVariable7_10a, "BPatch_ne");
	fail7aPrint(globalVariable7_11a, globalVariable7_12a, "BPatch_ge");
	fail7aPrint(globalVariable7_13a, globalVariable7_14a, "BPatch_and");
	fail7aPrint(globalVariable7_15a, globalVariable7_16a, "BPatch_or");
    }
}

void func7_2() { dprintf("func7_2 () called\n"); }

/*
 * Test #8 - preserve registers - expr
 *	Verify the complex AST expressions do not clobber application
 *	paramter registers.
 */
void func8_1(int p1, int p2, int p3, int p4, int p5, int p6, int p7,
	     int p8, int p9, int p10)
{
    dprintf("func8_1 (...) called\n");
    if ((p1 == 1) && (p2 == 2) && (p3 == 3) && (p4 == 4) && (p5 == 5) &&
	(p6 == 6) && (p7 == 7) && (p8 == 8) && (p9 == 9) && (p10 == 10))  {
        printf("Passed test #8 (preserve registers - expr)\n");
	passedTest[8] = TRUE;
    } else {
	printf("**Failed** test #8 (preserve registers - expr )\n");
	if (p1 != 1)  printf("    parameter #1 is %d not 1\n", p1);
	if (p2 != 2)  printf("    parameter #2 is %d not 2\n", p2);
	if (p3 != 3)  printf("    parameter #3 is %d not 3\n", p3);
	if (p4 != 4)  printf("    parameter #4 is %d not 4\n", p4);
	if (p5 != 5)  printf("    parameter #5 is %d not 5\n", p5);
	if (p6 != 6)  printf("    parameter #6 is %d not 6\n", p6);
	if (p7 != 7)  printf("    parameter #7 is %d not 7\n", p7);
	if (p8 != 8)  printf("    parameter #8 is %d not 8\n", p8);
	if (p9 != 9)  printf("    parameter #9 is %d not 9\n", p9);
	if (p10 != 10)  printf("    parameter #10 is %d not 10\n", p10);
    }
}

/*
 * Test #9 - reserve registers - funcCall
 *	Verify the a snippet that calls a function does not clobber the
 *	the parameter registers.
 */
void func9_1(int p1, int p2, int p3, int p4, int p5, int p6, int p7,
	     int p8, int p9, int p10)
{
    dprintf("func9_1 (...) called\n");
    if ((p1 == 1) && (p2 == 2) && (p3 == 3) && (p4 == 4) && (p5 == 5) &&
	(p6 == 6) && (p7 == 7) && (p8 == 8) && (p9 == 9) && (p10 == 10))  {
        printf("Passed test #9 (preserve registers - funcCall)\n");
	passedTest[9] = TRUE;
    } else {
	printf("**Failed** test #9 (preserve registers - funcCall )\n");
	if (p1 != 1)  printf("    parameter #1 is %d not 1\n", p1);
	if (p2 != 2)  printf("    parameter #2 is %d not 2\n", p2);
	if (p3 != 3)  printf("    parameter #3 is %d not 3\n", p3);
	if (p4 != 4)  printf("    parameter #4 is %d not 4\n", p4);
	if (p5 != 5)  printf("    parameter #5 is %d not 5\n", p5);
	if (p6 != 6)  printf("    parameter #6 is %d not 6\n", p6);
	if (p7 != 7)  printf("    parameter #7 is %d not 7\n", p7);
	if (p8 != 8)  printf("    parameter #8 is %d not 8\n", p8);
	if (p9 != 9)  printf("    parameter #9 is %d not 9\n", p9);
	if (p10 != 10)  printf("    parameter #10 is %d not 10\n", p10);
    }
}

/*
 * Test #10 - insert snippet order
 *	Verify that a snippet are inserted in the requested order.  We insert
 *	one snippet and then request two more to be inserted.  One before
 *	the first snippet and one after it.
 */
void func10_1()
{
    if ((globalVariable10_1 == 1) && (globalVariable10_2 == 1) &&
	(globalVariable10_3 == 1) && (globalVariable10_4 == 3)) {
	printf("Passed test #10 (insert snippet order)\n");
	passedTest[10] = TRUE;
    } else {
	printf("** Failed test #10 (insert snippet order)\n");
	if (!globalVariable10_1)
	    printf("    call10_1 was not called first\n");
	if (!globalVariable10_2)
	    printf("    call10_2 was not called second\n");
	if (!globalVariable10_3)
	    printf("    call10_3 was not called third\n");
    }
}

/*
 * Test #11 - snippets at entry,exit,call
 */ 	
void func11_2() {
    globalVariable11_1 = 2;
}

void func11_1()
{
    globalVariable11_1 = 1;
    func11_2();
    globalVariable11_1 = 3;
}

/*
 * Test #12 - insert/remove and malloc/free
 */	
void func12_2()
{
    DUMMY_FN_BODY;
}

void func12_1()
{
    kludge = 1;	/* Here so that the following function call isn't the first
		   instruction */
    func12_2();
    stop_process_();
    func12_2();
    if (globalVariable12_1 == 1) {
        printf("Passed test #12 (insert/remove and malloc/free)\n");
	passedTest[12] = TRUE;
    } else {
        printf("**Failed test #12 (insert/remove and malloc/free)\n");
	printf("ZANDY: #12 failed because globalVariable12_1 == %d\n", globalVariable12_1);
    }
}

/*
 * Test #13 - paramExpr,retExpr,nullExpr
 *	Test various expressions
 */
int func13_2()
{
    return(RET13_1);
}

void func13_1(int p1, int p2, int p3, int p4, int p5)
{
    func13_2();

    if ((p1 == 131) && (p2 == 132) && (p3 == 133) &&
	(p4 == 134) && (p5 == 135) && (globalVariable13_1 == 63)) {
	printf("Passed test #13 (paramExpr,retExpr,nullExpr)\n");
	passedTest[13] = TRUE;
    } else {
	printf("**Failed test #13 (paramExpr,retExpr,nullExpr)\n");
	if (p1 != 131) printf("  parameter 1 is %d, not 131\n", p1);
	if (p2 != 132) printf("  parameter 2 is %d, not 132\n", p2);
	if (p3 != 133) printf("  parameter 3 is %d, not 133\n", p3);
	if (p4 != 134) printf("  parameter 4 is %d, not 134\n", p4);
	if (p5 != 135) printf("  parameter 5 is %d, not 135\n", p5);
	if (!(globalVariable13_1 & 1)) printf("    passed param a1 wrong\n");
	if (!(globalVariable13_1 & 2)) printf("    passed param a2 wrong\n");
	if (!(globalVariable13_1 & 4)) printf("    passed param a3 wrong\n");
	if (!(globalVariable13_1 & 8)) printf("    passed param a4 wrong\n");
	if (!(globalVariable13_1 & 16)) printf("    passed param a5 wrong\n");
	if (!(globalVariable13_1 & 32)) printf("    return value wrong\n");
    }
}

/*
 * Test #14 - replace function call
 */
void func14_2()
{
    globalVariable14_1 = 2;
}

void func14_3()
{
    globalVariable14_2 = 1;
}

void func14_1()
{
    kludge = 1;	/* Here so that the following function call isn't the first
		   instruction */

    func14_2();
    func14_3();

    if (globalVariable14_1 == 1 && globalVariable14_2 == 0) {
        printf("Passed test #14 (replace/remove function call)\n");
	passedTest[14] = TRUE;
    } else {
        printf("**Failed test #14 (replace/remove function call)\n");
	if (globalVariable14_1 != 1)
    	    printf("    call to func14_2() was not replaced\n");
	if (globalVariable14_2 != 0)
	    printf("    call to func14_3() was not removed\n");
    }
}

/*
 * Test #15 - setMutationsActive
 */
void check15result(char *varname, int value, int expected,
		   char *errstr, int *failed)
{
    if (value != expected) {
	if (!*failed)
	    printf("**failed test #15 (setMutationsActive)\n");
	*failed = TRUE;

	printf("    %s = %d %s\n", varname, value, errstr);
    }		
}


void func15_2()
{
  DUMMY_FN_BODY;
}

void func15_3()
{
    globalVariable15_3 = 100;
    /* increment a dummy variable to keep alpha code generator from assuming
       too many free registers on the call into func15_3. jkh 3/7/00 */
    globalVariable15_4++;
}

void func15_4()
{
    kludge = 1;	/* Here so that the following function call isn't the first
		   instruction */

    func15_3();
}

void func15_1()
{
    int failed = FALSE;

    func15_2();
    check15result("globalVariable15_1", globalVariable15_1, 1,
		  "after first call to instrumented function", &failed);

#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)
    /* Test a function that makes a system call (is a special case on Sparc) */
    access(".", R_OK);
    check15result("globalVariable15_2", globalVariable15_2, 2,
		  "after first call to instrumented function", &failed);
#endif

    func15_4();
    check15result("globalVariable15_3", globalVariable15_3, 1,
		  "after first call to instrumented function", &failed);

    /***********************************************************/

    stop_process_();

    func15_2();
    check15result("globalVariable15_1", globalVariable15_1, 1,
		  "after second call to instrumented function", &failed);

#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)
    access(".", R_OK);
    check15result("globalVariable15_2", globalVariable15_2, 2,
		  "after second call to instrumented function", &failed);
#endif

    func15_4();
    check15result("globalVariable15_3", globalVariable15_3, 100,
		  "after second call to instrumented function", &failed);

    /***********************************************************/

    stop_process_();

    func15_2();
    check15result("globalVariable15_1", globalVariable15_1, 2,
		  "after third call to instrumented function", &failed);

#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)
    access(".", R_OK);
    check15result("globalVariable15_2", globalVariable15_2, 4,
		  "after third call to instrumented function", &failed);
#endif

    func15_4();
    check15result("globalVariable15_3", globalVariable15_3, 101,
		  "after third call to instrumented function", &failed);

    if (!failed) {
        printf("Passed test #15 (setMutationsActive)\n");
	passedTest[15] = TRUE;
    }
}

/*
 * Test #16 - if-else
 *	Test if-then-else clauses
 */
void func16_2() { dprintf("func16_2 () called\n"); }
void func16_3() { dprintf("func16_3 () called\n"); }
void func16_4() { dprintf("func16_4 () called\n"); }

void func16_1()
{
    int failed = FALSE;

    func16_2();
    if (globalVariable16_1 != 1 || globalVariable16_2 != 0) {
        printf("**Failed test #16 (if-else)\n");
	if (globalVariable16_1 != 1)
	    printf("    True clause of first if should have been executed but was not.\n");
	if (globalVariable16_2 != 0)
	    printf("    False clause of first if should not have been executed but was.\n");
	failed = 1;
    }

    func16_3();
    if (globalVariable16_3 != 0 || globalVariable16_4 != 1) {
        printf("**Failed test #16 (if-else)\n");
	if (globalVariable16_3 != 1)
	    printf("    True clause of second if should not have been executed but was.\n");
	if (globalVariable16_4 != 0)
	    printf("    False clause of second if should have been executed but was not.\n");
	failed = 1;
    }

    func16_4();
    if ((globalVariable16_5 != 0 || globalVariable16_6 != 1) ||
        (globalVariable16_7 != 0 || globalVariable16_8 != 1) ||
        (globalVariable16_9 != 0 || globalVariable16_10 != 1)) {
	    printf("    failed large if clauses tests.\n");
	failed = 1;
    }

    if (!failed)
    	printf("Passed test #16 (if-else)\n");
	passedTest[16] = TRUE;
}

/*
 * Test #17 - return values from func calls
 *	See test1.C for a detailed comment
 */
void func17_1()
{
    int ret17_1;
    int func17_2();
    void func17_3();

    kludge = 1;	/* Here so that the following function call isn't the first
    		   instruction */

    ret17_1 = func17_2();
    func17_3();

    if ((ret17_1 != RET17_1) ||
	(globalVariable17_1 != RET17_2) ||
	(globalVariable17_2 != RAN17_1)) {
        printf("**Failed** test case #17 (return values from func calls)\n");
        if (ret17_1 != RET17_1) {
            printf("  return value was %d, not %d\n", ret17_1, RET17_1);
        }
        if (globalVariable17_1 != RET17_2) {
            printf("  return value was %d, not %d\n",
                globalVariable17_1, RET17_2);
        }
        if (globalVariable17_2 != RAN17_1) {
            printf("  function call17_2 was not inserted\n");
        }
    } else {
        printf("Passed test #17 (return values from func calls)\n");
	passedTest[17] = TRUE;
    }
}

int func17_2()
{
    return RET17_1;
}

int func17_4()
{
    return RET17_2;
}

void func17_3()
{
    globalVariable17_1 = func17_4();
    return;
}

/*
 * Test #18 - read/write a variable in the mutatee
 */
void func18_1()
{
    if (globalVariable18_1 == 17) {
    	printf("Passed test #18 (read/write a variable in the mutatee)\n");
	passedTest[18] = TRUE;
    } else {
	printf("**Failed test #18 (read/write a variable in the mutatee)\n");
	if (globalVariable18_1 == 42)
	    printf("    globalVariable18_1 still contains 42 (probably it was not written to)\n");
	else
	    printf("    globalVariable18_1 contained %d, not 17 as expected\n",
		    globalVariable18_1);
    }
}

/*
 * Test #19 - oneTimeCode
 */
void func19_1()
{
    stop_process_();

    if (globalVariable19_1 != MAGIC19_1) {
	printf("**Failed test #19 (oneTimeCode)\n");
	printf("    globalVariable19_1 contained %d, not %d as expected\n",
		globalVariable19_1, MAGIC19_1);
    }

    stop_process_();

    if (globalVariable19_2 == MAGIC19_2) {
	printf("Passed test #19 (oneTimeCode)\n");
	passedTest[19] = TRUE;
    } else {
	printf("**Failed test #19 (oneTimeCode)\n");
	printf("    globalVariable19_2 contained %d, not %d as expected\n",
		globalVariable19_2, MAGIC19_2);
    }
}

/*
 * Test #20 - instrumentation at arbitrary points
 */
int func20_3()
{
    static int n = 1;

    return n++;
}

volatile int tc = TEST20_C;
volatile double td = TEST20_D;
int test20_iter = 50;

int func20_2(int *int_val, double *double_val)
{
    int i, ret = 1;

    *int_val = tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+
	       (tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+
	       (tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+
	       (tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc+(tc
	       ))))))))))))))))))))))))))))))))))))))));

    *double_val = td+(td+(td+(td+(td+(td+(td+(td+(td+(td+
		  (td+(td+(td+(td+(td+(td+(td+(td+(td+(td+
		  (td+(td+(td+(td+(td+(td+(td+(td+(td+(td+
		  (td+(td+(td+(td+(td+(td+(td+(td+(td+(td+(td
		  ))))))))))))))))))))))))))))))))))))))));

    for (i = 0; i < test20_iter; i++) {
	ret *= 3;
	if (i % 2 == 1) {
	    ret *= 5;
	} else if (i < 10) {
	    ret *= 7;
	} else if (i > 20) {
	    ret *= 11;
	}
    }

/* The answer we expect from the above is: */
#define TEST20_ANSWER 1088896211

    return ret;
}

void func20_1()
{
    int ret;
    int int_val;
    double double_val;

    ret = func20_2(&int_val, &double_val);

    if (globalVariable20_1 == (TEST20_A * TEST20_TIMES) &&
	eq_doubles(globalVariable20_2, (TEST20_B * (double)TEST20_TIMES)) &&
	int_val == (TEST20_C * TEST20_TIMES) &&
	eq_doubles(double_val, (TEST20_D * (double)TEST20_TIMES)) &&
	ret == TEST20_ANSWER) {
	printf("Passed test #20 (instrument arbitrary points)\n");
	passedTest[20] = TRUE;
    } else {
	printf("**Failed test #20 (instrument arbitrary points)\n");
	if (globalVariable20_1 != (TEST20_A * TEST20_TIMES))
    	    printf("    globalVariable20_1 contained %d, not %d as expected\n",
		   globalVariable20_1, TEST20_A * TEST20_TIMES);
	if (!eq_doubles(globalVariable20_2, (TEST20_B * (double)TEST20_TIMES)))
    	    printf("    globalVariable20_2 contained %g, not %g as expected\n",
		   globalVariable20_2, TEST20_B * (double)TEST20_TIMES);
	if (int_val != (TEST20_C * TEST20_TIMES))
    	    printf("    int_val contained %d, not %d as expected\n",
		   int_val, TEST20_C * TEST20_TIMES);
	if (!eq_doubles(double_val, (TEST20_D * (double)TEST20_TIMES)))
    	    printf("    double_val contained %g, not %g as expected\n",
		   double_val, TEST20_D * (double)TEST20_TIMES);
	if (ret != TEST20_ANSWER)
    	    printf("    ret contained %d, not %d as expected\n",
		   ret, TEST20_ANSWER);
    }
}

/*
 * Test #21 - findFunction in module
 */
void func21_1()
{
     /* Nothing for the mutatee to do in this test (findFunction in module) */
#if defined(sparc_sun_solaris2_4) \
 || defined(mips_sgi_irix6_4) \
 || defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_linux2_0) \
 || defined(alpha_dec_osf4_0) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */

     printf("Passed test #21 (findFunction in module)\n");
     passedTest[21] = TRUE;
#else
    printf("Skipped test #21 (findFunction in module)\n");
    printf("\t- not implemented on this platform\n");
    passedTest[21] = TRUE;
#endif
}

/*
 * Test #22 - replace function
 *
 * These are defined in libtestA.so
 */
extern void call22_5A(int);
extern void call22_6(int);

volatile int _unused;	/* move decl here to dump compiler warning - jkh */

void func22_1()
{
#if !defined(sparc_sun_solaris2_4)
/*    !defined(alpha_dec_osf4_0) - temporary jkh 3/27/02 */

    printf("Skipped test #22 (replace function)\n");
    printf("\t- not implemented on this platform\n");
    passedTest[22] = TRUE;
#else
    /* libtestA.so should already be loaded (by the mutator), but we
       need to use the dl interface to get pointers to the functions
       it defines. */
    void (*call22_5)(int);
    void (*call22_6)(int);

    void *handleA;
    char dlopenName[128];
#if defined(sparc_sun_solaris2_4)
    int dlopenMode = RTLD_NOW | RTLD_GLOBAL;
#else
    int dlopenMode = RTLD_NOW;
#endif
    _unused = sprintf(dlopenName, "./%s", libNameA);
    handleA = dlopen(dlopenName, dlopenMode);
    if (! handleA) {
	 printf("**Failed test #22 (replaceFunction)\n");
	 printf("  Mutatee couldn't get handle for %s\n", libNameA);
    }

    call22_5 = (void(*)(int))dlsym(handleA, "call22_5");
    if (! call22_5) {
	 printf("**Failed test #22 (replaceFunction)\n");
	 printf("  Mutatee couldn't get handle for call22_5 in %s\n", libNameA);
    }
    call22_6 = (void(*)(int))dlsym(handleA, "call22_6");
    if (! call22_6) {
	 printf("**Failed test #22 (replaceFunction)\n");
	 printf("  Mutatee couldn't get handle for call22_6 in %s\n", libNameA);
    }

    /* Call functions that have been replaced by the mutator.  The
       side effects of these calls (replaced, not replaced, or
       otherwise) are independent of each other. */
    call22_1(10);  /* replaced by call22_2 */
    if (globalVariable22_1 != 10 + MAGIC22_2) {
	 printf("**Failed test #22 (replace function) (a.out -> a.out)\n");
	 return;
    }
    call22_3(20);  /* replaced by call22_4 */
    if (globalVariable22_2 != 20 + MAGIC22_4) {
	 printf("**Failed test #22 (replace function) (a.out -> shlib)\n");
	 return;
    }
    call22_5(30);  /* replaced by call22_5 (in libtestB) */
    if (globalVariable22_3 != 30 + MAGIC22_5B) {
	 printf("globalVariable22_3 = %d\n", globalVariable22_3);
	 printf("30 + MAGIC22_5B = %d\n", 30 + MAGIC22_5B);
	 printf("**Failed test #22 (replace function) (shlib -> shlib)\n");
	 return;
    }
    call22_6(40);  /* replaced by call22_6 */
    if (globalVariable22_4 != 40 + MAGIC22_7) {
	 printf("**Failed test #22 (replace function) (shlib -> a.out)\n");
	 return;
    }
    printf("Passed test #22 (replace function)\n");
    passedTest[22] = TRUE;
#endif
}

/*
 * Test #23 - local variables
 */
int shadowVariable23_1 = 2300010;
int shadowVariable23_2 = 2300020;
int globalShadowVariable23_1 = (int)0xdeadbeef;
int globalShadowVariable23_2 = (int)0xdeadbeef;
int globalVariable23_1 = 2300000;

void verifyScalarValue23(char *name, int a, int value)
{
    verifyScalarValue(name, a, value, 23, "local variables");
}

void call23_2()
{
    /* copy shadowed global variables to visible global variables to permit
     *    checking their values
     */
    globalShadowVariable23_1 = shadowVariable23_1;
    globalShadowVariable23_2 = shadowVariable23_2;
}

void call23_1()
{
    int localVariable23_1 = 2300019;
    int shadowVariable23_1 = 2300011;
    int shadowVariable23_2 = 2300021;

    call23_2();			/* place to manipulate local variables */

    passedTest[23] = TRUE;

    /* snippet didn't update local variable */
    verifyScalarValue23("localVariable23_1", localVariable23_1, 2300001);

    /* did snippet update shadow variable (in the global scope) */
    verifyScalarValue23("globalShadowVariable23_1", globalShadowVariable23_1,
	2300010);

    /* did snippet correctly update shadow variable call23_2 */
    verifyScalarValue23("shadowVariable23_1", shadowVariable23_1, 2300012);

    /* did snippet correctly update shadow variable via global
       scope in call23_2 */
    verifyScalarValue23("shadowVariable23_2", shadowVariable23_2, 2300021);

    /* did snippet correctly update shadow variable via global
       scope in call23_2 */
    verifyScalarValue23("globalShadowVariable23_2", globalShadowVariable23_2,
	2300023);

    /* did snippet correctly read local variable in call23_2 */
    verifyScalarValue23("globalVariable23_1", globalVariable23_1, 2300001);
}

void func23_1()
{
/*    !defined(alpha_dec_osf4_0) && \ - temporaarly removed jkh 3/27/02 */
#if !defined(sparc_sun_solaris2_4) && \
    !defined(rs6000_ibm_aix4_1) && \
    !defined(i386_unknown_linux2_0) && \
    !defined(i386_unknown_solaris2_5) && \
    !defined(i386_unknown_nt4_0) && \
    !defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */

    printf("Skipped test #23 (local variables)\n");
    printf("\t- not implemented on this platform\n");
    passedTest[23] = TRUE;
#else
    call23_1();

    if (passedTest[23]) printf("Passed test #23 (local variables)\n");
#endif
}

/*
 * Test #24 - arrary variables
 */
int dummy;
int foo;

int globalVariable24_1[100];
int globalVariable24_2 = 53;
int globalVariable24_3;
int globalVariable24_4 = 83;
int globalVariable24_5;

/* to hold values from local array */
int globalVariable24_6;
int globalVariable24_7;

/* for 2-d arrays - array is not square and we avoid using diagonal elements
 *    to make sure we test address computation
 */
int globalVariable24_8[10][15];
int globalVariable24_9;

void verifyValue24(char *name, int *a, int index, int value)
{
    verifyValue(name, a, index, value, 24, "array variables");
}

void verifyScalarValue24(char *name, int a, int value)
{
    verifyScalarValue(name, a, value, 24, "array variables");
}

void call24_2()
{
}

void call24_1()
{
#ifdef sparc_sun_solaris2_4
    unsigned i=0; /* hack to prevent g++'s optimizer making func uninstr'uble */
#else
    unsigned i;
#endif
    int localVariable24_1[100];

    for (i=0; i < 100; i++) localVariable24_1[i] = 2400000;

    localVariable24_1[79] = 2400007;
    localVariable24_1[83] = 2400008;

    call24_2();

    verifyValue24("localVariable24_1", localVariable24_1, 1, 2400005);
    verifyValue24("localVariable24_1", localVariable24_1, 53, 2400006);
}

void func24_1()
{
#if !defined(sparc_sun_solaris2_4) && \
    !defined(rs6000_ibm_aix4_1) && \
    !defined(alpha_dec_osf4_0) && \
    !defined(i386_unknown_linux2_0) && \
    !defined(i386_unknown_solaris2_5) && \
    !defined(i386_unknown_nt4_0) && \
    !defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM. */

    printf("Skipped test #24 (arrary variables)\n");
    printf("\t- not implemented on this platform\n");
    passedTest[24] = TRUE;
#else
    int i, j;

    passedTest[24] = TRUE;


    for (i=0; i < 100; i++) globalVariable24_1[i] = 2400000;
    globalVariable24_1[79] = 2400003;
    globalVariable24_1[83] = 2400004;

    for (i=0; i < 10; i++) {
	for (j=0; j < 15; j++) {
	    globalVariable24_8[i][j] = 2400010;
	}
    }
    globalVariable24_8[7][9] = 2400012;

    /* inst code we put into this function:
     *  At Call:
     *     globalVariable24_1[1] = 2400001
     *     globalVariable24_1[globalVariable24_2] = 2400002
     *	   globalVariable24_3 = globalVariable24_1[79]
     *	   globalVariable24_5 = globalVariable24_1[globalVariable24_4]
     *     localVariable24_1[1] = 2400001
     *     localVariable24_1[globalVariable24_2] = 2400002
     *	   globalVariable24_8[2][3] = 2400011
     *	   globalVariable24_6 = localVariable24_1[79]
     *	   globalVariable24_7 = localVariable24_1[globalVariable24_4]
     */
    call24_1();

    for (i=0; i < 100; i++) {
	if (i == 1) {
	    /* 1st element should be modified by the snippet (constant index) */
	    verifyValue24("globalVariable24_1", globalVariable24_1, 1, 2400001);
	} else if (i == 53) {
	    /* 53rd element should be modified by the snippet (variable index) */
	    verifyValue24("globalVariable24_1", globalVariable24_1, 53, 2400002);
	} else if (i == 79) {
	    /* 79th element was modified by us  */
	    verifyValue24("globalVariable24_1", globalVariable24_1, 79, 2400003);
	} else if (i == 83) {
	    /* 83rd element was modified by us  */
	    verifyValue24("globalVariable24_1", globalVariable24_1, 83, 2400004);
	} else if (globalVariable24_1[i] != 2400000) {
	    /* rest should still be the original value */
	    verifyValue24("globalVariable24_1", globalVariable24_1, i, 2400000);
	}
    }

    verifyScalarValue24("globalVariable24_3", globalVariable24_3, 2400003);
    verifyScalarValue24("globalVariable24_5", globalVariable24_5, 2400004);

    /* now for the two elements read from the local variable */
    verifyScalarValue24("globalVariable24_6", globalVariable24_6, 2400007);
    verifyScalarValue24("globalVariable24_7", globalVariable24_7, 2400008);

    /* verify 2-d element use */
    verifyScalarValue24("globalVariable24_8[2][3]", globalVariable24_8[2][3],
	 2400011);
    verifyScalarValue24("globalVariable24_9", globalVariable24_9, 2400012);

    if (passedTest[24]) printf("Passed test #24 (array variables)\n");
#endif
}

/*
 * Test #25 - unary operators
 */
int globalVariable25_1;
int *globalVariable25_2;	/* used to hold addres of globalVariable25_1 */
int globalVariable25_3;
int globalVariable25_4;
int globalVariable25_5;
int globalVariable25_6;
int globalVariable25_7;

void call25_1()
{
  DUMMY_FN_BODY;
}

void func25_1()
{
#if defined(mips_sgi_irix6_4) || \
    defined(alpha_dec_osf4_0) 	/* temporarty disabled jkh 3/27/02 */
    printf("Skipped test #25 (unary operators)\n");
    printf("\t- not implemented on this platform\n");
    passedTest[25] = TRUE;
#else

    passedTest[25] = TRUE;

    globalVariable25_1 = 25000001;
    globalVariable25_2 = (int *) 25000002;
    globalVariable25_3 = 25000003;
    globalVariable25_4 = 25000004;
    globalVariable25_5 = 25000005;
    globalVariable25_6 = -25000006;
    globalVariable25_7 = 25000007;

    /* inst code we put into this function:
     *  At Entry:
     *     globalVariable25_2 = &globalVariable25_1
     *     globalVariable25_3 = *globalVariable25_2
     *     globalVariable25_5 = -globalVariable25_4
     *     globalVariable25_7 = -globalVariable25_6
     */

    call25_1();

    if ((int *) globalVariable25_2 != &globalVariable25_1) {
	if (passedTest[25]) printf("**Failed** test #25 (unary operators)\n");
	passedTest[25] = FALSE;
	printf("    globalVariable25_2 = %p, not %p\n",
	    globalVariable25_2, (void *) &globalVariable25_1);
    }

    if (globalVariable25_3 != globalVariable25_1) {
	if (passedTest[25]) printf("**Failed** test #25 (unary operators)\n");
	passedTest[25] = FALSE;
	printf("    globalVariable25_3 = %d, not %d\n",
	    globalVariable25_3, globalVariable25_1);
    }

    if (globalVariable25_5 != -globalVariable25_4) {
	if (passedTest[25]) printf("**Failed** test #25 (unary operators)\n");
	passedTest[25] = FALSE;
	printf("    globalVariable25_5 = %d, not %d\n",
	    globalVariable25_5, -globalVariable25_4);
    }

    if (globalVariable25_7 != -globalVariable25_6) {
	if (passedTest[25]) printf("**Failed** test #25 (unary operators)\n");
	passedTest[25] = FALSE;
	printf("    globalVariable25_7 = %d, not %d\n",
	    globalVariable25_7, -globalVariable25_6);
    }

    if (passedTest[25]) printf("Passed test #25 (unary operators)\n");
#endif
}

/*
 * Test #26 - field operators
 */
struct struct26_2 globalVariable26_1;
int globalVariable26_2 = 26000000;
int globalVariable26_3 = 26000000;
int globalVariable26_4 = 26000000;
int globalVariable26_5 = 26000000;
int globalVariable26_6 = 26000000;
int globalVariable26_7 = 26000000;

int globalVariable26_8 = 26000000;
int globalVariable26_9 = 26000000;
int globalVariable26_10 = 26000000;
int globalVariable26_11 = 26000000;
int globalVariable26_12 = 26000000;
int globalVariable26_13 = 26000000;

void verifyScalarValue26(char *name, int a, int value)
{
    verifyScalarValue(name, a, value, 26, "field operators");
}

void call26_2()
{
}

void call26_1()
{
    int i;
    struct struct26_2 localVariable26_1;

    localVariable26_1.field1 = 26002001;
    localVariable26_1.field2 = 26002002;
    for (i=0; i < 10; i++) localVariable26_1.field3[i] = 26002003 + i;
    localVariable26_1.field4.field1 = 26002013;
    localVariable26_1.field4.field2 = 26002014;

    /* check local variables at this point (since we known locals are still
       on the stack here. */
    call26_2();

}

void func26_1()
{
    /* !defined(alpha_dec_osf4_0) && \ - temporarly disabled jkh 3/27/02 */
#if !defined(sparc_sun_solaris2_4) && \
    !defined(rs6000_ibm_aix4_1) && \
    !defined(i386_unknown_linux2_0) && \
    !defined(i386_unknown_solaris2_5) && \
    !defined(i386_unknown_nt4_0) && \
    !defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM. */

    printf("Skipped test #26 (struct elements)\n");
    printf("\t- not implemented on this platform\n");
    passedTest[26] = TRUE;
#else
    int i;

    passedTest[26] = TRUE;

    globalVariable26_1.field1 = 26001001;
    globalVariable26_1.field2 = 26001002;
    for (i=0; i < 10; i++) globalVariable26_1.field3[i] = 26001003 + i;
    globalVariable26_1.field4.field1 = 26000013;
    globalVariable26_1.field4.field2 = 26000014;

    call26_1();

    verifyScalarValue26("globalVariable26_2", globalVariable26_2, 26001001);
    verifyScalarValue26("globalVariable26_3", globalVariable26_3, 26001002);
    verifyScalarValue26("globalVariable26_4", globalVariable26_4, 26001003);
    verifyScalarValue26("globalVariable26_5", globalVariable26_5, 26001003+5);
    verifyScalarValue26("globalVariable26_6", globalVariable26_6, 26000013);
    verifyScalarValue26("globalVariable26_7", globalVariable26_7, 26000014);

    /* local variables */
    verifyScalarValue26("globalVariable26_8", globalVariable26_8, 26002001);
    verifyScalarValue26("globalVariable26_9", globalVariable26_9, 26002002);
    verifyScalarValue26("globalVariable26_10", globalVariable26_10, 26002003);
    verifyScalarValue26("globalVariable26_11", globalVariable26_11, 26002003+5);
    verifyScalarValue26("globalVariable26_12", globalVariable26_12, 26002013);
    verifyScalarValue26("globalVariable26_13", globalVariable26_13, 26002014);

    if (passedTest[26]) printf("Passed test #26 (field operators)\n");
#endif
}

/*
 * Test #27 - type compatibility
 */
typedef struct {
    /* void *field27_11; */
    int field27_11;
    float field27_12;
} type27_1;

typedef struct {
    /* void *field27_21; */
    int field27_21;
    float field27_22;
} type27_2;

typedef struct {
    int field3[10];
    struct struct26_2 field4;
} type27_3;

typedef struct {
    int field3[10];
    struct struct26_2 field4;
} type27_4;

int globalVariable27_1;
/* Note for future reference: -Wl,-bgcbypass:3 is NECESSARY for
   compilation (gcc) on AIX. Damn efficient linkers. */
int globalVariable27_5[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
int globalVariable27_6[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
float globalVariable27_7[10] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0,
				6.0, 7.0, 8.0, 9.0};
float globalVariable27_8[12];

/* need this variables or some compilers (AIX xlc) will removed unused
   typedefs - jkh 10/13/99 */
type27_1 dummy1;
type27_2 dummy2;
type27_3 dummy3;
type27_4 dummy4;

void func27_1()
{
#if !defined(sparc_sun_solaris2_4) && \
    !defined(rs6000_ibm_aix4_1) && \
    !defined(alpha_dec_osf4_0) && \
    !defined(i386_unknown_linux2_0) && \
    !defined(i386_unknown_solaris2_5) && \
    !defined(i386_unknown_nt4_0) && \
    !defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM. */

    printf("Skipped test #27 (type compatibility)\n");
    printf("\t- not implemented on this platform\n");
    passedTest[27] = TRUE;
#else
    passedTest[27] = (globalVariable27_1 == 1);

    if (passedTest[27]) printf("Passed test #27 (type compatibility)\n");
#endif
}

/*
 * Test #28 - field operators
 */
struct struct28_1 {
    int field1;
    int field2;
};

struct struct28_2 {
    int field1;
    int field2;
    int field3[10];
    struct struct28_1 field4;
};

char globalVariable28_1[sizeof(struct struct28_2)];
struct struct28_2 *temp;
int globalVariable28_2 = 28000000;
int globalVariable28_3 = 28000000;
int globalVariable28_4 = 28000000;
int globalVariable28_5 = 28000000;
int globalVariable28_6 = 28000000;
int globalVariable28_7 = 28000000;

void verifyScalarValue28(char *name, int a, int value)
{
    verifyScalarValue(name, a, value, 28, "user defined fields");
}

void call28_1()
{
    int i = 42;

    int j = i;

    for (j=0; j < 400; j++);
}

void func28_1()
{
    int i;

    passedTest[28] = TRUE;


    temp = (struct struct28_2 *) globalVariable28_1;

    temp->field1 = 28001001;
    temp->field2 = 28001002;
    for (i=0; i < 10; i++) temp->field3[i] = 28001003 + i;
    temp->field4.field1 = 28000013;
    temp->field4.field2 = 28000014;

    call28_1();

    verifyScalarValue28("globalVariable28_2", globalVariable28_2, 28001001);
    verifyScalarValue28("globalVariable28_3", globalVariable28_3, 28001002);
    verifyScalarValue28("globalVariable28_4", globalVariable28_4, 28001003);
    verifyScalarValue28("globalVariable28_5", globalVariable28_5, 28001003+5);
    verifyScalarValue28("globalVariable28_6", globalVariable28_6, 28000013);
    verifyScalarValue28("globalVariable28_7", globalVariable28_7, 28000014);

    if (passedTest[28]) printf("Passed test #28 (user defined fields)\n");
}

int globalVariable29_1;

int func29_1()
{
    passedTest[29] = (globalVariable29_1 == 1);

    if (passedTest[29]) printf("Passed test #29 (BPatch_srcObj class)\n");

    return 0;
}

void func30_2()
{
    DUMMY_FN_BODY;
}

/* variables to keep the base addr and last addr of call30_1 */
unsigned globalVariable30_8 = 0;
unsigned globalVariable30_9 = 0;
int func30_1()
{
    kludge = 1;	/* Here so that the following function call isn't the first
		   instruction */

#if defined(sparc_sun_solaris2_4) || \
    defined(i386_unknown_solaris2_5) || \
    defined(i386_unknown_linux2_0) || \
    defined(i386_unknown_nt4_0) ||\
    defined(rs6000_ibm_aix4_1) || \
    defined(alpha_dec_osf4_0) || \
    defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM. */

    func30_2();

    passedTest[30] = !globalVariable30_3 ||
#if defined(rs6000_ibm_aix4_1)
		     ((globalVariable30_8 <= globalVariable30_3) &&
#else
		     ((globalVariable30_2 <= globalVariable30_3) &&
#endif
		      (globalVariable30_3 <= globalVariable30_9));
    if (!passedTest[30]){
    	printf("**Failed** test #30 (line information) in %s[%d]\n", __FILE__, __LINE__ );
	return 0;
    }

    passedTest[30] = !globalVariable30_4 ||
#if defined(rs6000_ibm_aix4_1)
		     ((globalVariable30_8 <= globalVariable30_4) &&
#else
		     ((globalVariable30_2 <= globalVariable30_4) &&
#endif
		      (globalVariable30_4 <= globalVariable30_9));
    if (!passedTest[30]){
    	printf("**Failed** test #30 (line information) in %s[%d]\n", __FILE__, __LINE__ );
	return 0;
    }

    passedTest[30] = !globalVariable30_5 ||
#if defined(rs6000_ibm_aix4_1)
		     ((globalVariable30_8 <= globalVariable30_5) &&
#else
		     ((globalVariable30_2 <= globalVariable30_5) &&
#endif
		      (globalVariable30_5 <= globalVariable30_9));
    if (!passedTest[30]){
    	printf("**Failed** test #30 (line information) in %s[%d]\n", __FILE__, __LINE__ );
	return 0;
    }

    passedTest[30] = !globalVariable30_6 ||
		     (globalVariable30_1 == globalVariable30_6);
    if (!passedTest[30]){
    	printf("**Failed** test #30 (line information) in %s[%d]\n", __FILE__, __LINE__ );
	return 0;
    }

    printf("Passed test #30 (line information)\n");

#else
    printf("Skipped test #30 (line information)\n");
    printf("\t- not implemented on this platform\n");
    passedTest[30] = TRUE;
#endif
    return 1;
}

void func31_2()
{
  globalVariable31_2 = 1;
}

void func31_3()
{
  globalVariable31_3 = 1;
}

void func31_4( int value )
{
  if( value == 0 )
    {
      printf( "func_31_4 called with value = 0 !\n" );
    }

  globalVariable31_4 += value;
}

int func31_1()
{
#if defined(alpha_dec_osf4_0)
    printf( "Skipped test #31 (non-recursive base tramp guard)\n" );
    printf( "\t- not implemented on this platform\n" );
    passedTest[ 31 ] = TRUE;

    return 1;
#else
  globalVariable31_1 = 0;
  globalVariable31_2 = 0;
  globalVariable31_3 = 0;
  globalVariable31_4 = 0;

  func31_2();

  passedTest[ 31 ] = ( globalVariable31_3 == 1 );
  if( ! passedTest[ 31 ] )
    {
      printf( "**Failed** test #31 (non-recursive base tramp guard)\n" );
      printf( "    globalVariable31_3 = %d, should be 1 (no instrumentation got executed?).\n",
	      globalVariable31_3 );
      return 0;
    }

  passedTest[ 31 ] = ( globalVariable31_4 == 0 );
  if( ! passedTest[ 31 ] )
    {
      printf( "**Failed** test #31 (non-recursive base tramp guard)\n" );
      printf( "    globalVariable31_4 = %d, should be 0.\n",
	      globalVariable31_4 );
      switch( globalVariable31_4 )
	{
	case 0: printf( "    Recursive guard works fine.\n" ); break;
	case 1: printf( "    Pre-instr recursive guard does not work.\n" ); break;
	case 2: printf( "    Post-instr recursive guard does not work.\n" ); break;
	case 3: printf( "    None of the recursive guards work.\n" ); break;
	default: printf( "    Something is really wrong.\n" ); break;
	}
      return 0;
    }

  passedTest[ 31 ] = TRUE;
  printf( "Passed test #31 (non-recursive base tramp guard)\n" );

  return 1;
#endif
}

void func32_2()
{
  globalVariable32_2 = 1;
}

void func32_3()
{
  globalVariable32_3 = 1;
}

void func32_4( int value )
{
  if( value == 0 )
    {
      printf( "func_32_4 called with value = 0 !\n" );
    }

  globalVariable32_4 += value;
}

int func32_1()
{
  globalVariable32_1 = 0;
  globalVariable32_2 = 0;
  globalVariable32_3 = 0;
  globalVariable32_4 = 0;

  func32_2();

  passedTest[ 32 ] = ( globalVariable32_3 == 1 );
  if( ! passedTest[ 32 ] )
    {
      printf( "**Failed** test #32 (non-recursive base tramp guard)\n" );
      printf( "    globalVariable32_3 = %d, should be 1 (no instrumentation got executed?).\n",
	      globalVariable32_3 );
      return 0;
    }

  passedTest[ 32 ] = ( globalVariable32_4 == 3 );
  if( ! passedTest[ 32 ] )
    {
      printf( "**Failed** test #32 (non-recursive base tramp guard)\n" );
      printf( "    globalVariable32_4 = %d, should be 3.\n",
	      globalVariable32_4 );
      switch( globalVariable32_4 )
	{
	case 0: printf( "    Recursive guard works fine.\n" ); break;
	case 1: printf( "    Pre-instr recursive guard does not work.\n" ); break;
	case 2: printf( "    Post-instr recursive guard does not work.\n" ); break;
	case 3: printf( "    None of the recursive guards work.\n" ); break;
	default: printf( "    Something is really wrong.\n" ); break;
	}
      return 0;
    }

  passedTest[ 32 ] = TRUE;
  printf( "Passed test #32 (recursive base tramp guard)\n" );

  return 1;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void func33_2(int x)
{
    printf("Hello\n");

    if (x == 1) {
	printf("Goodbye.\n");
    } else {
	printf("See you.\n");
    }

    printf("That's all.\n");
}

int func33_3(int x)
{
    printf("Entry.\n");

    switch (x) {
      case 1:
	printf("1\n");
	x += 10;
	break;
      case 2:
	printf("2\n");
	x-= 12;
	break;
      case 3:
	printf("3\n");
	x *= 33;
	break;
      case 4:
	printf("4\n");
	x /= 42;
	break;
      case 5:
	printf("5\n");
	x %= 57;
	break;
      case 6:
	printf("6\n");
	x <<= 2;
	break;
      case 7:
	printf("7\n");
	x >>= 3;
	break;
      case 8:
	printf("8\n");
	x ^= 0xfe;
	break;
      case 9:
	printf("9\n");
	x &= 0x44;
	break;
      case 10:
	printf("10\n");
	x |= 0x11;
	break;
    };

    printf("Exit.\n");

    return x;
}

void func33_1()
{
    /* The only possible failures occur in the mutator. */

    passedTest[ 33 ] = TRUE;
    printf( "Passed test #33 (control flow graphs)\n" );
}

/*
 * Nested loops.
 */
void func34_2()
{
    int i, j, k;

    /* There are four loops in total. */
    for (i = 0; i < 10; i++) { /* Contains two loops. */
	printf("i = %d\n", i);

	for (j = 0; j < 10; j++) { /* Contains one loop. */
	    printf("j = %d\n", j);

	    k = 0;
	    while (k < 10) {
		printf("k = %d\n", k);
		k++;
	    }
	}

	do {
	    j++;
	    printf("j = %d\n", j);

	} while (j < 10);
    }
}

void func34_1()
{
    /* The only possible failures occur in the mutator. */

    passedTest[ 34 ] = TRUE;
    printf( "Passed test #34 (loop information)\n" );
}

#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0) || defined(sparc_sun_solaris2_4) || defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */
#ifndef Fortran
#ifdef __cplusplus
extern "C" int call35_1();
#else
extern int call35_1();
#endif
#endif
#endif

void func35_1()
{
#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0) || defined(sparc_sun_solaris2_4) || defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */
#if !defined Fortran

    int value;
    value = call35_1();

    if( value != 35 )
    {
      printf( "**Failed** test #35 (function relocation)\n" );

      printf( "    total = %d, should be 35.\n", value );

      switch(value)
      {
        case 1: printf( "    Entry instrumentation did not work.\n" ); break;
        case 2: printf( "    Exit instrumentation did not work.\n" ); break;
        default: printf("    Take a look at the call to call35_2.\n" ); break;
      }
      passedTest[ 35 ] = FALSE;
      return;
    }

    passedTest[ 35 ] = TRUE;
    printf( "Passed test #35 (function relocation)\n" );
#endif
#else
    passedTest[ 35 ] = TRUE;
    printf( "Skipped test #35 (function relocation)\n" );
#if defined(i386_unknown_nt4_0)
    printf( "\t- test not implemented for this platform\n" );
#else
    printf( "\t- not implemented on this platform\n" );
#endif
#endif
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/* "replaceFunctionCall()" on Irix usually requires that the new
   callee have a GOT entry.  This dummy function forces GOT entries to
   be created for these functions. */
#if defined(mips_sgi_irix6_4)
void dummy_force_got_entries()
{
    call14_1();
    call15_3();
}
#endif

#ifdef i386_unknown_nt4_0
#define USAGE "Usage: test1.mutatee [-attach] [-verbose] -run <num> .."
#else
#define USAGE "Usage: test1.mutatee [-attach <fd>] [-verbose] -run <num> .."
#endif


#ifdef __cplusplus
extern "C" void runTests();
#endif

void runTests()
{
    int j;

    for (j=0; j <= MAX_TEST; j++) {
	passedTest [j] = FALSE;
    }

    if (runTest[1]) func1_1();
    if (runTest[2]) func2_1();
    if (runTest[3]) func3_1();
    if (runTest[4]) func4_1();
    if (runTest[5]) func5_1();
    if (runTest[6]) func6_1();
    if (runTest[7]) func7_1();
    if (runTest[8]) func8_1(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    if (runTest[9]) func9_1(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    if (runTest[10]) func10_1();
    if (runTest[11]) func11_1();
    if (runTest[12]) func12_1();

    if (runTest[13]) func13_1(131, 132, 133, 134, 135);
    if (runTest[14]) func14_1();
    if (runTest[15]) func15_1();
    if (runTest[16]) func16_1();
    if (runTest[17]) func17_1();
    if (runTest[18]) func18_1();
    if (runTest[19]) func19_1();
    if (runTest[20]) func20_1();
    if (runTest[21]) func21_1();
    if (runTest[22]) func22_1();
    if (runTest[23]) func23_1();
    if (runTest[24]) func24_1();
    if (runTest[25]) func25_1();
    if (runTest[26]) func26_1();
    if (runTest[27]) func27_1();
    if (runTest[28]) func28_1();
    if (runTest[29]) func29_1();
    if (runTest[30]) func30_1();

    if (runTest[31]) func31_1();
    if (runTest[32]) func32_1();

    if (runTest[33]) func33_1();
    if (runTest[33]) func34_1();
    if (runTest[35]) func35_1();
    
}
