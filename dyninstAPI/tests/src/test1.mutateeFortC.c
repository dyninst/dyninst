/* Test application (Mutatee) */

/* $Id: test1.mutateeFortC.c,v 1.4 2003/04/28 19:30:24 hollings Exp $ */

#include <stdlib.h>
#include "test1.mutateeCommon.h"

struct struct26_1 {
    int field1_;
    int field2_;
};

struct struct26_2_ {
    int field1_;
    int field2_;
    int field3_[10];
    struct struct26_1 field4_;
};

struct block_ {
	double globalVariable20_2_;

	int globalVariable1_1, globalVariable3_1_, globalVariable4_1_, globalVariable5_1_, globalVariable5_2_;

	int globalVariable6_1, globalVariable6_2_, globalVariable6_3_, globalVariable6_4_, globalVariable6_5_,
		globalVariable6_6_, globalVariable6_1a_, globalVariable6_2a_, globalVariable6_3a_, globalVariable6_4a_,
		globalVariable6_5a_, globalVariable6_6a_;

	int constVar0_, constVar1_, constVar2_, constVar3_, constVar4_, constVar5_, constVar6_, constVar7_,
		constVar9_, constVar10_, constVar60_, constVar64_, constVar66_, constVar67_;
	int globalVariable7_1, globalVariable7_2_, globalVariable7_3_, globalVariable7_4_, globalVariable7_5_,
		globalVariable7_6_, globalVariable7_7_, globalVariable7_8_, globalVariable7_9_, globalVariable7_10_,
		globalVariable7_11_, globalVariable7_12_, globalVariable7_13_, globalVariable7_14_, globalVariable7_15_,
		globalVariable7_16_;

	int globalVariable7_1a_, globalVariable7_2a_, globalVariable7_3a_, globalVariable7_4a_, globalVariable7_5a_,
		globalVariable7_6a_, globalVariable7_7a_, globalVariable7_8a_, globalVariable7_9a_, globalVariable7_10a_,
	    globalVariable7_11a_, globalVariable7_12a_, globalVariable7_13a_, globalVariable7_14a_, globalVariable7_15a_,
		globalVariable7_16a__;

	int globalVariable8_1;

	int globalVariable10_1, globalVariable10_2_, globalVariable10_3_, globalVariable10_4_;

	int globalVariable11_1, globalVariable11_2_, globalVariable11_3_, globalVariable11_4_, globalVariable11_5_;

	int globalVariable12_1;

	int globalVariable13_1;

	int globalVariable14_1, globalVariable14_2_;

	int globalVariable15_1, globalVariable15_2_, globalVariable15_3_, globalVariable15_4_;

	int globalVariable16_1, globalVariable16_2_, globalVariable16_3_, globalVariable16_4_, globalVariable16_5_,
		globalVariable16_6_, globalVariable16_7_, globalVariable16_8_, globalVariable16_9_, globalVariable16_10_;

	int globalVariable17_1, globalVariable17_2_;

	int globalVariable18_1;

	int globalVariable19_1, globalVariable19_2_;

	int globalVariable20_1;

	int globalVariable25_1, globalVariable25_2_, globalVariable25_3_, globalVariable25_4_, globalVariable25_5_,
		globalVariable25_6_, globalVariable25_7_;

/*	struct struct26_2_ globalVariable26_1; */
	int globalVariable26_2_, globalVariable26_3_, globalVariable26_4_, globalVariable26_5_, globalVariable26_6_,
		globalVariable26_7_, globalVariable26_8_, globalVariable26_9_, globalVariable26_10_, globalVariable26_11_,
		globalVariable26_12_, globalVariable26_13_;

	int globalVariable27_1; int globalVariable27_5_[10]; int globalVariable27_6_[10];
	float globalVariable27_7_[10]; float globalVariable27_8_[12];

	int globalVariable29_1;

	int globalVariable31_1, globalVariable31_2_, globalVariable31_3_, globalVariable31_4_;

	int globalVariable32_1, globalVariable32_2_, globalVariable32_3_, globalVariable32_4_;

    int passedTest_ [36];
};

#if defined (sparc_sun_solaris2_4)
#define func1_1 func1_1_
#define func2_1 func2_1_
#define func3_1 func3_1_
#define func4_1 func4_1_
#define func5_1 func5_1_
#define func6_1 func6_1_
#define func7_1 func7_1_
#define func8_1 func8_1_
#define func9_1 func9_1_
#define func10_1 func10_1_
#define func11_1 func11_1_
#define func12_1 func12_1_
#define func13_1 func13_1_
#define func14_1 func14_1_
#define func15_1 func15_1_
#define func16_1 func16_1_
#define func17_1 func17_1_
#define func18_1 func18_1_
#define func19_1 func19_1_
#define func20_1 func20_1_
#define func21_1 func21_1_
#define func22_1 func22_1_
#define func23_1 func23_1_
#define func24_1 func24_1_
#define func25_1 func25_1_
#define func26_1 func26_1_
#define func27_1 func27_1_
#define func28_1 func28_1_
#define func29_1 func29_1_
#define func30_1 func30_1_
#define func31_1 func31_1_
#define func32_1 func32_1_
#define func33_1 func33_1_
#define func34_1 func34_1_
#define func35_1 func35_1_
#define init_globals init_globals_
#define globals globals_
#endif

extern struct block_ globals;

extern void init_globals ();
extern void func1_1 ();
extern void func2_1 ();
extern void func3_1 ();
extern void func4_1 ();
extern void func5_1 ();
extern void func6_1 ();
extern void func7_1 ();
extern void func8_1 ();
extern void func9_1 ();
extern void func10_1 ();
extern void func11_1 ();
extern void func12_1 ();
extern void func13_1 ();
extern void func14_1 ();
extern void func15_1 ();
extern void func16_1 ();
extern void func17_1 ();
extern void func18_1 ();
extern void func19_1 ();
extern void func20_1 ();
extern void func21_1 ();
extern void func22_1 ();
extern void func23_1 ();
extern void func24_1 ();
extern void func25_1 ();
extern void func26_1 ();
extern void func27_1 ();
extern void func28_1 ();
extern void func29_1 ();
extern void func30_1 ();
extern void func31_1 ();
extern void func32_1 ();
extern void func33_1 ();
extern void func34_1 ();
extern void func35_1 ();

int mutateeFortran = 1;
int mutateeCplusplus = 0;

#ifdef F77
int mutateeF77 = 1;
#else
int mutateeF77 = 0;
#endif

void runTests()
{
    int i, j;

    int *pp1, *pp2, *pp3, *pp4, *pp5, *pp6, *pp7, *pp8, *pp9, *pp10;

    for (j=0; j <= MAX_TEST; j++) {
	globals.passedTest_[j] = FALSE;
    }

    pp1 = (int*) malloc (sizeof (int));
    pp2 = (int*) malloc (sizeof (int));
    pp3 = (int*) malloc (sizeof (int));
    pp4 = (int*) malloc (sizeof (int));
    pp5 = (int*) malloc (sizeof (int));
    pp6 = (int*) malloc (sizeof (int));
    pp7 = (int*) malloc (sizeof (int));
    pp8 = (int*) malloc (sizeof (int));
    pp9 = (int*) malloc (sizeof (int));
    pp10 = (int*) malloc (sizeof (int));

    *pp1 = 1; *pp2 = 2; *pp3 = 3; *pp4 = 4; *pp5 = 5;
    *pp6 = 6; *pp7 = 7; *pp8 = 8; *pp9 = 9; *pp10 = 10;

    init_globals();

    if (runTest[1]) func1_1();
    if (runTest[2]) func2_1();
    if (runTest[3]) func3_1();
    if (runTest[4]) func4_1();
    if (runTest[5]) func5_1();
    if (runTest[6]) func6_1();
    if (runTest[7]) func7_1();
    if (runTest[8]) func8_1(pp1, pp2, pp3, pp4, pp5, pp6, pp7, pp8, pp9, pp10);
    if (runTest[9]) func9_1(pp1, pp2, pp3, pp4, pp5, pp6, pp7, pp8, pp9, pp10);
    if (runTest[10]) func10_1();
    if (runTest[11]) func11_1();
    if (runTest[12]) func12_1();

    *pp1 = 131; *pp2 = 132; *pp3 = 133; *pp4 = 134; *pp5 = 135;

    if (runTest[13]) func13_1(pp1, pp2, pp3, pp4, pp5);
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
    if (runTest[34]) func34_1();
    if (runTest[35]) func35_1();

    /* Combine fortran passedTest with C passedTest */
    for (i=1; i <= MAX_TEST; i++)
	if (globals.passedTest_ [i-1]) passedTest [i] = TRUE;
}
