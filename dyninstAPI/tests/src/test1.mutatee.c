
/* Test application (Mutatee) */

/* $Id: test1.mutatee.c,v 1.21 1999/07/13 04:31:31 csserra Exp $ */

#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)
#include <unistd.h>
#endif
#ifdef i386_unknown_nt4_0
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#if defined(sparc_sun_solaris2_4) || defined(alpha_dec_osf4_0)
#include <dlfcn.h> // For replaceFunction test
#endif

/* XXX Currently, there's a bug in the library that prevents a subroutine call
 * instrumentation point from being recognized if it is the first instruction
 * in a function.  The following variable is used in this program in a number
 * of kludges to get around this.
 */
int kludge;

/* control debug printf statements */
#define dprintf	if (debugPrint) printf
int debugPrint = 0;

#define TRUE	1
#define FALSE	0

int runAllTests = TRUE;
#define MAX_TEST 22
int runTest[MAX_TEST+1];
int passedTest[MAX_TEST+1];

#if defined(mips_sgi_irix6_4) && (_MIPS_SIM == _MIPS_SIM_NABI32)
static char *libNameA = "libtestA_n32.so";
#else
static char *libNameA = "libtestA.so";
#endif

#define RET13_1 1300100

#define RAN17_1 1701000

#define RET17_1 1700100
#define RET17_2 1700200

#define MAGIC19_1 1900100
#define MAGIC19_2 1900200

// These are copied in libtestA.c and libtestB.c
#define MAGIC22_1   2200100
#define MAGIC22_2   2200200
#define MAGIC22_3   2200300
#define MAGIC22_4   2200400
#define MAGIC22_5A  2200510
#define MAGIC22_5B  2200520
#define MAGIC22_6   2200600
#define MAGIC22_7   2200700

int isAttached = 0;

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

int globalVariable6_1 = 0xdeadbeef;
int globalVariable6_2 = 0xdeadbeef;
int globalVariable6_3 = 0xdeadbeef;
int globalVariable6_4 = 0xdeadbeef;
int globalVariable6_5 = 0xdeadbeef;
int globalVariable6_6 = 0xdeadbeef;
int globalVariable6_1a = 0xdeadbeef;
int globalVariable6_2a = 0xdeadbeef;
int globalVariable6_3a = 0xdeadbeef;
int globalVariable6_4a = 0xdeadbeef;
int globalVariable6_5a = 0xdeadbeef;
int globalVariable6_6a = 0xdeadbeef;

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

int globalVariable19_1 = 0xdeadbeef;
int globalVariable19_2 = 0xdeadbeef;

int globalVariable20_1 = 0xdeadbeef;
double globalVariable20_2 = 0.0;

int globalVariable22_1 = 0;
int globalVariable22_2 = 0;
int globalVariable22_3 = 0;
int globalVariable22_4 = 0;

#define TEST20_A 3
#define TEST20_B 4.3
#define TEST20_C 7
#define TEST20_D 6.4
#define TEST20_TIMES 41

/*
 * Check to see if the mutator has attached to us.
 */
int checkIfAttached()
{
    return isAttached;
}

/*
 * Stop the process (in order to wait for the mutator to finish what it's
 * doing and restart us).
 */
#ifdef alpha_dec_osf4_0
static long long int  beginFP;
#endif
void stop_process()
{
#ifdef i386_unknown_nt4_0
    DebugBreak();
#else

#ifdef alpha_dec_osf4_0
    register long int fp asm("15");

    beginFP = fp;
#endif

    kill(getpid(), SIGSTOP);

#ifdef alpha_dec_osf4_0
    fp = beginFP;
#endif

#endif
}

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

void call2_1(int arg1, int arg2, char *arg3)
{
    if ((arg1 == 1) && (arg2 == 2) && (!strcmp(arg3, "testString2_1"))) {
	printf("Passed test #2 (three parameter function)\n");
	passedTest[2] = TRUE;
    } else {
	printf("**Failed** test #2 (three parameter function)\n");
	if (arg1 != 1) 
	    printf("    arg1 = %d, should be 1\n", arg1);
	if (arg1 != 2) 
	    printf("    arg2 = %d, should be 2\n", arg2);
	if (strcmp(arg3, "testString2_1")) 
	    printf("    arg3 = %s, should be \"testString2_1\"\n", arg3);
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
      if (p3 != 93) printf("    call9_1 parameter 2 is %d not 93\n", p3);
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
	globalVariable10_4 = 1;	/* update marker of call order */
	globalVariable10_1 = 1;	/* flag that this was called first */
    }
}


void call10_2()
{
    if (globalVariable10_4 == 1) {
	globalVariable10_4 = 2;	/* update marker of call order */
	globalVariable10_2 = 1;	/* flag that this was called first */
    }
}

void call10_3()
{
    if (globalVariable10_4 == 2) {
	globalVariable10_4 = 3;	/* update marker of call order */
	globalVariable10_3 = 1;	/* flag that this was called first */
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
     assert(p1!=0); // shouldn't try to divide by zero!
     assert(p1==1); // actually only expect calls with p1==1

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
     assert(p1!=0); // shouldn't try to divide by zero!
     assert(p1==1); // actually only expect calls with p1==1

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


/*
 * This is a series of nearly empty functions to attach code to 
 */

//
// Start of Test #1
//
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


//
// Start of Test #2
//
void func2_1() { dprintf("func2_1 () called\n"); }

//
// Start of Test #3
//
void func3_1() { dprintf("func3_1 () called\n"); }

//
// Start of Test #4 - sequence
//	Run two expressions and verify correct ordering.
//
void func4_1()
{
    void func4_2();
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


// 
// Start of Test #5 - if w.o. else
//	Execute two if statements, one true and one false.
//
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


//
// Start of Test #6 - arithmetic operators
//	Verify arithmetic operators:
//		constant integer addition
//		constant integer subtraction
//		constant integer divide (using large constants)
//		constant integer divide (small consts)
//		constant integer multiply 
//		constant comma operator
//		variable integer addition
//		variable integer subtraction
//		variable integer divide (using large constants)
//		variable integer divide (small consts)
//		variable integer multiply 
//		variable comma operator
//
//	constant - use constant expressions
//	variable - use variables in the expressions
//
//
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


//
// Start Test Case #7 - relational operators
//	Generate all relational operators (eq, gt, le, ne, ge, and, or) 
//	in both the true and false forms.
//

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

//
// Test #8 - preserve registers - expr
//	Verify the complex AST expressions do not clobber application
//	paramter registers.
//
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

//
// Test #9 - reserve registers - funcCall
//	Verify the a snippet that calls a function does not clobber the
//	the parameter registers.
//
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

//
// Test #10 - insert snippet order
//	Verify that a snippet are inserted in the requested order.  We insert
//	one snippet and then request two more to be inserted.  One before
//	the first snippet and one after it.
//
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

//
// Test #11 - snippets at entry,exit,call
// 	
void func11_2() {
    globalVariable11_1 = 2;
}

void func11_1()
{
    globalVariable11_1 = 1;
    func11_2();
    globalVariable11_1 = 3;

}

//
// Test #12 - insert/remove and malloc/free
//	
void func12_2()
{
}

void func12_1()
{
    func12_2();
    stop_process();
    func12_2();
    if (globalVariable12_1 == 1) {
        printf("Passed test #12 (insert/remove and malloc/free)\n");
	passedTest[12] = TRUE;
    } else {
        printf("**Failed test #12 (insert/remove and malloc/free)\n");
    }
}

//
// Test #13 - paramExpr,retExpr,nullExpr
//	Test various expressions
//
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


//
// Test #14 - replace function call
//
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


//
// Test #15 - setMutationsActive
//
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
}

void func15_3()
{
    globalVariable15_3 = 100;
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

    stop_process();

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

    stop_process();

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


// 
// Test #16 - if-else
//	Test if-then-else clauses
//

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


//
// Test #17 - return values from func calls
//	See test1.C for a detailed comment
//
void func17_1()
{
    int ret17_1;
    int func17_2();
    void func17_3();

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

//
// Test #18 - read/write a variable in the mutatee
//
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

//
// Test #19 - oneTimeCode
//
void func19_1()
{
    stop_process();

    if (globalVariable19_1 == MAGIC19_1) {
	printf("Passed test #19 (oneTimeCode)\n");
	passedTest[19] = TRUE;
    } else {
	printf("**Failed test #19 (oneTimeCode)\n");
	printf("    globalVariable19_1 contained %d, not %d as expected\n",
		globalVariable19_1, MAGIC19_1);
    }
}


//
// Test #20 - instrumentation at arbitrary points
//

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

// The answer we expect from the above is:
#define TEST20_ANSWER 1088896211

    return ret;
}

void func20_1()
{
#if !defined(rs6000_ibm_aix4_1) && !defined(alpha_dec_osf4_0)
    printf("Skipped test #20 (instrument arbitrary points)\n");
    printf("\t- not implemented on this platform\n");
    passedTest[20] = TRUE;
#else
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
#endif
}

//
// Test #21 - findFunction in module
//
void func21_1()
{
     // Nothing for the mutatee to do in this test (findFunction in module)
#if defined(sparc_sun_solaris2_4) || defined(mips_sgi_irix6_4) || defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0) || defined(alpha_dec_osf4_0)
     printf("Passed test #21 (findFunction in module)\n");
     passedTest[21] = TRUE;
#else
    printf("Skipped test #21 (findFunction in module)\n");
    printf("\t- not implemented on this platform\n");
    passedTest[21] = TRUE;
#endif
}

//
// Test #22 - replace function
//
// These are defined in libtestA.so
extern void call22_5A(int);  
extern void call22_6(int);

void func22_1()
{
#if !defined(sparc_sun_solaris2_4) && !defined(alpha_dec_osf4_0)
    printf("Skipped test #22 (replace function)\n");
    printf("\t- not implemented on this platform\n");
    passedTest[22] = TRUE;
#else
    // libtestA.so should already be loaded (by the mutator), but we
    // need to use the dl interface to get pointers to the functions
    // it defines.
    void (*call22_5)(int);
    void (*call22_6)(int);

    char dlopenName[128];
    int _unused = sprintf(dlopenName, "./%s", libNameA);
#if defined(sparc_sun_solaris2_4)
    int dlopenMode = RTLD_NOW | RTLD_GLOBAL;
#else
    int dlopenMode = RTLD_NOW;
#endif
    void *handleA = dlopen(dlopenName, dlopenMode);
    if (! handleA) {
	 printf("**Failed test #22 (replaceFunction)\n");
	 printf("  Mutatee couldn't get handle for %s\n", libNameA);
    }

    call22_5 = dlsym(handleA, "call22_5");
    if (! call22_5) {
	 printf("**Failed test #22 (replaceFunction)\n");
	 printf("  Mutatee couldn't get handle for call22_5 in %s\n", libNameA);
    }
    call22_6 = dlsym(handleA, "call22_6");
    if (! call22_6) {
	 printf("**Failed test #22 (replaceFunction)\n");
	 printf("  Mutatee couldn't get handle for call22_6 in %s\n", libNameA);
    }

    // Call functions that have been replaced by the mutator.  The
    // side effects of these calls (replaced, not replaced, or
    // otherwise) are independent of each other.
    call22_1(10);  // replaced by call22_2
    if (globalVariable22_1 != 10 + MAGIC22_2) {
	 printf("**Failed test #22 (replace function) (a.out -> a.out)\n");
	 return;
    }
    call22_3(20);  // replaced by call22_4
    if (globalVariable22_2 != 20 + MAGIC22_4) {
	 printf("**Failed test #22 (replace function) (a.out -> shlib)\n");
	 return;
    } 
    call22_5(30);  // replaced by call22_5 (in libtestB)
    if (globalVariable22_3 != 30 + MAGIC22_5B) {
	 printf("**Failed test #22 (replace function) (shlib -> shlib)\n");
	 return;
    } 
    call22_6(40);  // replaced by call22_6
    if (globalVariable22_4 != 40 + MAGIC22_7) {
	 printf("**Failed test #22 (replace function) (shlib -> a.out)\n");
	 return;
    }
    printf("Passed test #22 (replace function)\n");
    passedTest[22] = TRUE;
#endif
}

#ifdef i386_unknown_nt4_0
#define USAGE "Usage: test1 [-attach] [-verbose]"
#else
#define USAGE "Usage: test1 [-attach <fd>] [-verbose]"
#endif

int main(int iargc, char *argv[])
{                                       // despite different conventions
    unsigned argc=(unsigned)iargc;      // make argc consistently unsigned
    unsigned int i, j;
    int allPassed;
    int useAttach = FALSE;
#ifndef i386_unknown_nt4_0
    int pfd;
#endif
 
    for (j=0; j <= MAX_TEST; j++) {
	passedTest[j] = FALSE;
	if (!useAttach) runTest[j] = TRUE;
    }

    for (i=1; i < argc; i++) {
        if (!strcmp(argv[i], "-verbose")) {
            debugPrint = TRUE;
        } else if (!strcmp(argv[i], "-attach")) {
            useAttach = TRUE;
#ifndef i386_unknown_nt4_0
	    if (++i >= argc) {
		fprintf(stderr, "%s\n", USAGE);
		exit(-1);
	    }
	    pfd = atoi(argv[i]);
#endif
        } else if (!strcmp(argv[i], "-run")) {
            runAllTests = FALSE;
            for (j=0; j <= MAX_TEST; j++) runTest[j] = FALSE;
            for (j=i+1; j < argc; j++) {
                unsigned int testId;
                if ((testId = atoi(argv[j]))) {
                    if ((testId > 0) && (testId <= MAX_TEST)) {
                        runTest[testId] = TRUE;
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
        } else {
            fprintf(stderr, "%s\n", USAGE);
            exit(-1);
        }
    }

    if (useAttach) {
#ifndef i386_unknown_nt4_0
	char ch = 'T';
	if (write(pfd, &ch, sizeof(char)) != sizeof(char)) {
	    fprintf(stderr, "*ERROR*: Writing to pipe\n");
	    exit(-1);
	}
	close(pfd);
#endif
	printf("Waiting for mutator to attach...\n");
    	while (!checkIfAttached()) ;
	printf("Mutator attached.  Mutatee continuing.\n");
    }

    if (runTest[1]) func1_1();
    if (runTest[2]) func2_1();
    if (runTest[3]) func3_1();
    if (runTest[4]) func4_1();
    if (runTest[5]) func5_1();
    if (runTest[6]) func6_1();
    if (runTest[7]) func7_1();
    if (runTest[8]) func8_1(1,2,3,4,5,6,7,8,9,10);
    if (runTest[9]) func9_1(1,2,3,4,5,6,7,8,9,10);
    if (runTest[10]) func10_1();
    if (runTest[11]) func11_1();
    if (runTest[12]) func12_1();
    if (runTest[13]) func13_1(131,132,133,134,135);
    if (runTest[14]) func14_1();
    if (runTest[15]) func15_1();
    if (runTest[16]) func16_1();
    if (runTest[17]) func17_1();
    if (runTest[18]) func18_1();
    if (runTest[19]) func19_1();
    if (runTest[20]) func20_1();
    if (runTest[21]) func21_1();
    if (runTest[22]) func22_1();

    // See how we did running the tests.
    allPassed = TRUE;
    for (i=1; i <= MAX_TEST; i++) {
        if (runTest[i] && !passedTest[i]) allPassed = FALSE;
    }

    if (allPassed) {
        if (runAllTests) {
            printf("All tests passed\n");
        } else {
            printf("All requested tests passed\n");
        }
    } else {
        printf("**Some requested tests failed**\n");
    }

    return(0);
}
