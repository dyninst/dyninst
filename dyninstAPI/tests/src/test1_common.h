	IMPLICIT NONE

#if !defined (F77)
	TYPE struct26_1
		INTEGER field1, field2
	END TYPE struct26_1

	TYPE struct26_2
		INTEGER field1, field2, field3 (10)
		TYPE (struct26_1) field4
	END TYPE struct26_2

	TYPE type27_1
		INTEGER field27_11
		REAL field27_12
	END TYPE type27_1

	TYPE type27_2
		INTEGER field27_21
		REAL field27_22
	END TYPE type27_2

	TYPE type27_3
		INTEGER field3 (10)
		TYPE (struct26_2) field4
	END TYPE type27_3

	TYPE type27_4
		INTEGER field3 (10)
		TYPE (struct26_2) field5
	END TYPE type27_4

	TYPE (type27_1) dummy1
	TYPE (type27_2) dummy2
	TYPE (type27_3) dummy3
	TYPE (type27_4) dummy4
#endif

	INTEGER globalVariable1_1, globalVariable3_1,
     & globalVariable4_1, globalVariable5_1, globalVariable5_2,
     & globalVariable6_1, globalVariable6_2, globalVariable6_3,
     & globalVariable6_4, globalVariable6_5, globalVariable6_6,
     & globalVariable6_1a, globalVariable6_2a,
     & globalVariable6_3a, globalVariable6_4a,
     & globalVariable6_5a, globalVariable6_6a, constVar0,
     & constVar1, constVar2, constVar3, constVar4, constVar5,
     & constVar6, constVar7, constVar9, constVar10, constVar60,
     & constVar64, constVar66, constVar67, globalVariable7_1,
     & globalVariable7_2,
     & globalVariable7_3, globalVariable7_4, globalVariable7_5,
     & globalVariable7_6, globalVariable7_7, globalVariable7_8,
     & globalVariable7_9, globalVariable7_10, globalVariable7_11,
     & globalVariable7_12, globalVariable7_13, globalVariable7_14,
     & globalVariable7_15, globalVariable7_16, globalVariable7_1a,
     & globalVariable7_2a, globalVariable7_3a, globalVariable7_4a,
     & globalVariable7_5a, globalVariable7_6a, globalVariable7_7a,
     & globalVariable7_8a, globalVariable7_9a, globalVariable7_10a,
     & globalVariable7_11a, globalVariable7_12a,
     & globalVariable7_13a, globalVariable7_14a,
     & globalVariable7_15a, globalVariable7_16a,
     & globalVariable8_1, globalVariable10_1, globalVariable10_2,
     & globalVariable10_3, globalVariable10_4, globalVariable11_1,
     & globalVariable11_2, globalVariable11_3, globalVariable11_4,
     & globalVariable11_5, globalVariable12_1, globalVariable13_1,
     & globalVariable14_1, globalVariable14_2, globalVariable15_1,
     & globalVariable15_2, globalVariable15_3, globalVariable15_4,
     & globalVariable16_1, globalVariable16_2, globalVariable16_3,
     & globalVariable16_4, globalVariable16_5, globalVariable16_6,
     & globalVariable16_7, globalVariable16_8, globalVariable16_9,
     & globalVariable16_10, globalVariable17_1, globalVariable17_2,
     & globalVariable18_1, globalVariable19_1, globalVariable19_2,
     & globalVariable20_1, globalVariable25_1,

#if defined(alpha_dec_osf4_0)
     & globalVariable25_2*8,
#else
     & globalVariable25_2,
#endif
     & globalVariable25_3, globalVariable25_4, globalVariable25_5,
     & globalVariable25_6, globalVariable25_7, globalVariable26_2,
     & globalVariable26_3, globalVariable26_4, globalVariable26_5,
     & globalVariable26_6, globalVariable26_7, globalVariable26_8,
     & globalVariable26_9, globalVariable26_10, 
     & globalVariable26_11, globalVariable26_12,
     & globalVariable26_13, globalVariable27_1,
     & globalVariable27_5 (10), globalVariable27_6 (10),
     & globalVariable29_1,
     & globalVariable31_1, globalVariable31_2, globalVariable31_3,
     & globalVariable31_4, globalVariable32_1, globalVariable32_2,
     & globalVariable32_3, globalVariable32_4,
     & globalVariable36_1, globalVariable36_2, globalVariable36_3,
     & globalVariable36_4, globalVariable36_5, globalVariable36_6,
     & globalVariable36_7, globalVariable36_8, globalVariable36_9,
     & globalVariable36_10

	LOGICAL passedTest (37)
	LOGICAL runTest (37)

	DOUBLE PRECISION globalVariable20_2

	REAL globalVariable27_7 (10), globalVariable27_8 (12)

! **********************************************************************
! The following common block (globals) has a corresponding c structure 
! (struct block_) defined in test1.mutateeFortC.c.  Make sure all changes
! to this structure are reflected in the other. (Including the size of
! each memeber defined above)
! **********************************************************************
	common /globals/ globalVariable20_2, globalVariable1_1,
     & globalVariable3_1, globalVariable4_1, globalVariable5_1,
     & globalVariable5_2, globalVariable6_1, globalVariable6_2,
     & globalVariable6_3, globalVariable6_4, globalVariable6_5,
     & globalVariable6_6, globalVariable6_1a, globalVariable6_2a,
     & globalVariable6_3a, globalVariable6_4a, globalVariable6_5a,
     & globalVariable6_6a, constVar0, constVar1, constVar2,
     & constVar3,
     & constVar4, constVar5, constVar6, constVar7, constVar9,
     & constVar10, constVar60, constVar64, constVar66, constVar67,
     & globalVariable7_1, globalVariable7_2, globalVariable7_3,
     & globalVariable7_4, globalVariable7_5, globalVariable7_6,
     & globalVariable7_7, globalVariable7_8, globalVariable7_9,
     & globalVariable7_10, globalVariable7_11, globalVariable7_12,
     & globalVariable7_13, globalVariable7_14, globalVariable7_15,
     & globalVariable7_16, globalVariable7_1a, globalVariable7_2a,
     & globalVariable7_3a, globalVariable7_4a, globalVariable7_5a,
     & globalVariable7_6a, globalVariable7_7a, globalVariable7_8a,
     & globalVariable7_9a, globalVariable7_10a,
     & globalVariable7_11a, globalVariable7_12a,
     & globalVariable7_13a, globalVariable7_14a,
     & globalVariable7_15a, globalVariable7_16a, globalVariable8_1,
     & globalVariable10_1, globalVariable10_2, globalVariable10_3,
     & globalVariable10_4, globalVariable11_1, globalVariable11_2,
     & globalVariable11_3, globalVariable11_4, globalVariable11_5,
     & globalVariable12_1, globalVariable13_1, globalVariable14_1,
     & globalVariable14_2, globalVariable15_1, globalVariable15_2,
     & globalVariable15_3, globalVariable15_4, globalVariable16_1,
     & globalVariable16_2, globalVariable16_3, globalVariable16_4,
     & globalVariable16_5, globalVariable16_6, globalVariable16_7,
     & globalVariable16_8, globalVariable16_9, globalVariable16_10,
     & globalVariable17_1, globalVariable17_2, globalVariable18_1,
     & globalVariable19_1, globalVariable19_2, globalVariable20_1,
     & globalVariable25_1, globalVariable25_2, globalVariable25_3,
     & globalVariable25_4, globalVariable25_5, globalVariable25_6,
     & globalVariable25_7,
!	globalVariable26_1,
     & globalVariable26_2,
     & globalVariable26_3, globalVariable26_4, globalVariable26_5,
     & globalVariable26_6, globalVariable26_7, globalVariable26_8,
     & globalVariable26_9, globalVariable26_10, globalVariable26_11,
     & globalVariable26_12, globalVariable26_13, globalVariable27_1,
     & globalVariable27_5, globalVariable27_6, globalVariable27_7,
     & globalVariable27_8, globalVariable29_1, globalVariable31_1,
     & globalVariable31_2, globalVariable31_3, globalVariable31_4,
     & globalVariable32_1, globalVariable32_2, globalVariable32_3,
     & globalVariable32_4, 
     & globalVariable36_1, globalVariable36_2, globalVariable36_3,
     & globalVariable36_4, globalVariable36_5, globalVariable36_6,
     & globalVariable36_7, globalVariable36_8, globalVariable36_9,
     & globalVariable36_10,
     & passedTest, runTest

