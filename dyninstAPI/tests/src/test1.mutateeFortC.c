/* Test application (Mutatee) */

/* $Id: test1.mutateeFortC.c,v 1.2 2002/04/02 16:11:12 pcroth Exp $ */

#include "test1.mutateeCommon.h"

struct struct26_1_ {
    int field1_;
    int field2_;
};

struct struct26_2_ {
    int field1_;
    int field2_;
    int field3_[10];
    struct struct26_1_ field4_;
};

struct block_ {
	double globalVariable20_2_;

	int globalVariable1_1_, globalVariable3_1_, globalVariable4_1_, globalVariable5_1_, globalVariable5_2_;

	int globalVariable6_1_, globalVariable6_2_, globalVariable6_3_, globalVariable6_4_, globalVariable6_5_,
		globalVariable6_6_, globalVariable6_1a_, globalVariable6_2a_, globalVariable6_3a_, globalVariable6_4a_,
		globalVariable6_5a_, globalVariable6_6a_;

	int constVar0_, constVar1_, constVar2_, constVar3_, constVar4_, constVar5_, constVar6_, constVar7_,
		constVar9_, constVar10_, constVar60_, constVar64_, constVar66_, constVar67_;
	int globalVariable7_1_, globalVariable7_2_, globalVariable7_3_, globalVariable7_4_, globalVariable7_5_,
		globalVariable7_6_, globalVariable7_7_, globalVariable7_8_, globalVariable7_9_, globalVariable7_10_,
		globalVariable7_11_, globalVariable7_12_, globalVariable7_13_, globalVariable7_14_, globalVariable7_15_,
		globalVariable7_16_;

	int globalVariable7_1a_, globalVariable7_2a_, globalVariable7_3a_, globalVariable7_4a_, globalVariable7_5a_,
		globalVariable7_6a_, globalVariable7_7a_, globalVariable7_8a_, globalVariable7_9a_, globalVariable7_10a_,
	    globalVariable7_11a_, globalVariable7_12a_, globalVariable7_13a_, globalVariable7_14a_, globalVariable7_15a_,
		globalVariable7_16a__;

	int globalVariable8_1_;

	int globalVariable10_1_, globalVariable10_2_, globalVariable10_3_, globalVariable10_4_;

	int globalVariable11_1_, globalVariable11_2_, globalVariable11_3_, globalVariable11_4_, globalVariable11_5_;

	int globalVariable12_1_;

	int globalVariable13_1_;

	int globalVariable14_1_, globalVariable14_2_;

	int globalVariable15_1_, globalVariable15_2_, globalVariable15_3_, globalVariable15_4_;

	int globalVariable16_1_, globalVariable16_2_, globalVariable16_3_, globalVariable16_4_, globalVariable16_5_,
		globalVariable16_6_, globalVariable16_7_, globalVariable16_8_, globalVariable16_9_, globalVariable16_10_;

	int globalVariable17_1_, globalVariable17_2_;

	int globalVariable18_1_;

	int globalVariable19_1_, globalVariable19_2_;

	int globalVariable20_1_;

	int globalVariable25_1_, globalVariable25_2_, globalVariable25_3_, globalVariable25_4_, globalVariable25_5_,
		globalVariable25_6_, globalVariable25_7_;

/*	struct struct26_2_ globalVariable26_1_; */
	int globalVariable26_2_, globalVariable26_3_, globalVariable26_4_, globalVariable26_5_, globalVariable26_6_,
		globalVariable26_7_, globalVariable26_8_, globalVariable26_9_, globalVariable26_10_, globalVariable26_11_,
		globalVariable26_12_, globalVariable26_13_;

	int globalVariable27_1_; int globalVariable27_5_[10]; int globalVariable27_6_[10];
	float globalVariable27_7_[10]; float globalVariable27_8_[12];

	int globalVariable29_1_;

	int globalVariable31_1_, globalVariable31_2_, globalVariable31_3_, globalVariable31_4_;

	int globalVariable32_1_, globalVariable32_2_, globalVariable32_3_, globalVariable32_4_;

    int passedTest_ [36];
};

extern struct block_ globals_;

extern void init_globals_ ();
extern void func1_1_ ();
extern void func2_1_ ();
extern void func3_1_ ();
extern void func4_1_ ();
extern void func5_1_ ();
extern void func6_1_ ();
extern void func7_1_ ();
extern void func8_1_ ();
extern void func9_1_ ();
extern void func10_1_ ();
extern void func11_1_ ();
extern void func12_1_ ();
extern void func13_1_ ();
extern void func14_1_ ();
extern void func15_1_ ();
extern void func16_1_ ();
extern void func17_1_ ();
extern void func18_1_ ();
extern void func19_1_ ();
extern void func20_1_ ();
extern void func21_1_ ();
extern void func22_1_ ();
extern void func23_1_ ();
extern void func24_1_ ();
extern void func25_1_ ();
extern void func26_1_ ();
extern void func27_1_ ();
extern void func28_1_ ();
extern void func29_1_ ();
extern void func30_1_ ();
extern void func31_1_ ();
extern void func32_1_ ();
extern void func33_1_ ();
extern void func34_1_ ();
extern void func35_1_ ();

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
	globals_.passedTest_[j] = FALSE;
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

    init_globals_();

    if (runTest[1]) func1_1_();
    if (runTest[2]) func2_1_();
    if (runTest[3]) func3_1_();
    if (runTest[4]) func4_1_();
    if (runTest[5]) func5_1_();
    if (runTest[6]) func6_1_();
    if (runTest[7]) func7_1_();
    if (runTest[8]) func8_1_(pp1, pp2, pp3, pp4, pp5, pp6, pp7, pp8, pp9, pp10);
    if (runTest[9]) func9_1_(pp1, pp2, pp3, pp4, pp5, pp6, pp7, pp8, pp9, pp10);
    if (runTest[10]) func10_1_();
    if (runTest[11]) func11_1_();
    if (runTest[12]) func12_1_();

    *pp1 = 131; *pp2 = 132; *pp3 = 133; *pp4 = 134; *pp5 = 135;

    if (runTest[13]) func13_1_(pp1, pp2, pp3, pp4, pp5);
    if (runTest[14]) func14_1_();
    if (runTest[15]) func15_1_();
    if (runTest[16]) func16_1_();
    if (runTest[17]) func17_1_();
    if (runTest[18]) func18_1_();
    if (runTest[19]) func19_1_();
    if (runTest[20]) func20_1_();
    if (runTest[21]) func21_1_();
    if (runTest[22]) func22_1_();
    if (runTest[23]) func23_1_();
    if (runTest[24]) func24_1_();
    if (runTest[25]) func25_1_();
    if (runTest[26]) func26_1_();
    if (runTest[27]) func27_1_();
    if (runTest[28]) func28_1_();
    if (runTest[29]) func29_1_();
    if (runTest[30]) func30_1_();

    if (runTest[31]) func31_1_();
    if (runTest[32]) func32_1_();

    if (runTest[33]) func33_1_();
    if (runTest[34]) func34_1_();
    if (runTest[35]) func35_1_();

    /* Combine fortran passedTest with C passedTest */
    for (i=1; i <= MAX_TEST; i++)
	if (globals_.passedTest_ [i-1]) passedTest [i] = TRUE;
}
