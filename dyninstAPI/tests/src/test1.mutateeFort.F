/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

! libdyninst validation suite test #1
! author: Jon Beyer (6/1/01)
!	derived from a previous test by Jeff Hollingsworth

!------------------------------------------------------------------------------

! AIX xlf90 does not insert underscores at end

#if	defined(sparc_sun_solaris2_4) || defined(alpha_dec_osf4_0)
#define stop_process_ stop_process
#endif

!------------------------------------------------------------------------------

	SUBROUTINE init_globals
#include "test1_common.h"

	globalVariable1_1 = 69
	globalVariable3_1 = 31
	globalVariable4_1 = 41
	globalVariable5_1 = 51
	globalVariable5_2 = 51

	globalVariable6_1 = Z'deadbeef'
	globalVariable6_2 = Z'deadbeef'
	globalVariable6_3 = Z'deadbeef'
	globalVariable6_4 = Z'deadbeef'
	globalVariable6_5 = Z'deadbeef'
	globalVariable6_6 = Z'deadbeef'
	globalVariable6_1a = Z'deadbeef'
	globalVariable6_2a = Z'deadbeef'
	globalVariable6_3a = Z'deadbeef'
	globalVariable6_4a = Z'deadbeef'
	globalVariable6_5a = Z'deadbeef'
	globalVariable6_6a = Z'deadbeef'

	constVar0 = 0
	constVar1 = 1
	constVar2 = 2
	constVar3 = 3
	constVar4 = 4
	constVar5 = 5
	constVar6 = 6
	constVar7 = 7
	constVar9 = 9
	constVar10 = 10
	constVar60 = 60
	constVar64 = 64
	constVar66 = 66
	constVar67 = 67

	globalVariable7_1 = 71
	globalVariable7_2 = 71
	globalVariable7_3 = 71
	globalVariable7_4 = 71
	globalVariable7_5 = 71
	globalVariable7_6 = 71
	globalVariable7_7 = 71
	globalVariable7_8 = 71
	globalVariable7_9 = 71
	globalVariable7_10 = 71
	globalVariable7_11 = 71
	globalVariable7_12 = 71
	globalVariable7_13 = 71
	globalVariable7_14 = 71
	globalVariable7_15 = 71
	globalVariable7_16 = 71

	globalVariable7_1a = 73
	globalVariable7_2a = 73
	globalVariable7_3a = 73
	globalVariable7_4a = 73
	globalVariable7_5a = 73
	globalVariable7_6a = 73
	globalVariable7_7a = 73
	globalVariable7_8a = 73
	globalVariable7_9a = 73
	globalVariable7_10a = 73
	globalVariable7_11a = 73
	globalVariable7_12a = 73
	globalVariable7_13a = 73
	globalVariable7_14a = 73
	globalVariable7_15a = 73
	globalVariable7_16a = 73

	globalVariable8_1 = 1

	globalVariable10_1 = 0
	globalVariable10_2 = 0
	globalVariable10_3 = 0
	globalVariable10_4 = 0

	globalVariable11_1 = 0
	globalVariable11_2 = 0
	globalVariable11_3 = 0
	globalVariable11_4 = 0
	globalVariable11_5 = 0

	globalVariable12_1 = 0

	globalVariable13_1 = 0

	globalVariable14_1 = 0
	globalVariable14_2 = 0

	globalVariable15_1 = 0
	globalVariable15_2 = 0
	globalVariable15_3 = 0
	globalVariable15_4 = 0

	globalVariable16_1 = 0
	globalVariable16_2 = 0
	globalVariable16_3 = 0
	globalVariable16_4 = 0
	globalVariable16_5 = 0
	globalVariable16_6 = 0
	globalVariable16_7 = 0
	globalVariable16_8 = 0
	globalVariable16_9 = 0
	globalVariable16_10 = 0

	globalVariable17_1 = 0
	globalVariable17_2 = 0

	globalVariable19_1 = Z'deadbeef'
	globalVariable19_2 = z'deadbeef'

	globalVariable20_1 = Z'deadbeef'
	globalVariable20_2 = 0.0

	globalVariable26_2 = 26000000
	globalVariable26_3 = 26000000
	globalVariable26_4 = 26000000
	globalVariable26_5 = 26000000
	globalVariable26_6 = 26000000
	globalVariable26_7 = 26000000
	globalVariable26_8 = 26000000
	globalVariable26_9 = 26000000
	globalVariable26_10 = 26000000
	globalVariable26_11 = 26000000
	globalVariable26_12 = 26000000
	globalVariable26_13 = 26000000

	globalVariable27_5(1) = 0
	globalVariable27_5(2) = 1
	globalVariable27_5(3) = 2
	globalVariable27_5(4) = 3
	globalVariable27_5(5) = 4
	globalVariable27_5(6) = 5
	globalVariable27_5(7) = 6
	globalVariable27_5(8) = 7
	globalVariable27_5(9) = 8
	globalVariable27_5(10) = 9
	globalVariable27_6(1) = 0
	globalVariable27_6(2) = 1
	globalVariable27_6(3) = 2
	globalVariable27_6(4) = 3
	globalVariable27_6(5) = 4
	globalVariable27_6(6) = 5
	globalVariable27_6(7) = 6
	globalVariable27_6(8) = 7
	globalVariable27_6(9) = 8
	globalVariable27_6(10) = 9
	globalVariable27_7(1) = 0.0
	globalVariable27_7(2) = 1.0
	globalVariable27_7(3) = 2.0
	globalVariable27_7(4) = 3.0
	globalVariable27_7(5) = 4.0
	globalVariable27_7(6) = 5.0
	globalVariable27_7(7) = 6.0
	globalVariable27_7(8) = 7.0
	globalVariable27_7(9) = 8.0
	globalVariable27_7(10) = 9.0

#if !defined (F77)
	dummy1%field27_11 = 10
	dummy2%field27_21 = 10
	dummy3%field3 (1) = 10
	dummy4%field3 (1) = 10
#endif

	globalVariable31_1 = 0
	globalVariable31_2 = 0
	globalVariable31_3 = 0
	globalVariable31_4 = 0

	globalVariable32_1 = 0
	globalVariable32_2 = 0
	globalVariable32_3 = 0
	globalVariable32_4 = 0

	globalVariable36_1 = 0
	globalVariable36_2 = 0
	globalVariable36_3 = 0
	globalVariable36_4 = 0
	globalVariable36_5 = 0
	globalVariable36_6 = 0
	globalVariable36_7 = 0
	globalVariable36_8 = 0
	globalVariable36_9 = 0
	globalVariable36_10 = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE func1_1
#include "test1_common.h"

	CALL func1_2

	if (globalVariable1_1 .eq. 11) then
		print *, 'F90: Passed test #1 (zero arg function call)'
		passedTest (1) = .true.
	else
		print *, '**Failed** test #1 (zero arg function call)'
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE func1_2
	INTEGER foo = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE call1_1
#include "test1_common.h"
	globalVariable1_1 = 11
	END

!------------------------------------------------------------------------------

	SUBROUTINE func2_1
	INTEGER foo = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE call2_1 (arg1, arg2, arg3)
#include "test1_common.h"
	INTEGER arg1
	INTEGER arg2
	CHARACTER arg3 *(*)

	if (arg1 .eq. 1 .and. arg2 .eq. 2 .and. arg3 .eq.
     & "testString2_1") then
		print *, 'Passed test #2 (four paramater function)'
		passedTest (2) = .true.
	else
		print *, '**Failed** test #2 (four parameter function)'
		if (arg1 .ne. 1) then
			print *, '	arg1 = ', arg1, ', should be 1'
		endif
		if (arg2 .ne. 2) then
			print *, '	arg2 = ', arg2, ', should be 2'
		endif
		if (arg3 .ne. 'testString2_1') then
			print *, '	arg3 = ', arg3, ', should be "testString2_1"'
		endif
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE func3_1
	INTEGER foo = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE call3_1 (arg1, arg2)
#include "test1_common.h"
	INTEGER arg1, arg2
	if (arg1 .eq. 31 .and. arg2 .eq. 32) then
		print *, 'Passed test #3 (passing Variables to functions)'
		passedTest (3) = .true.
	else
		print *, '**Failed** test #3 (passing Variables to functions)'
		print *, '	arg1 = ', arg1, ', should be 31'
		print *, '	arg2 = ', arg2, ', should be 32'
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE func4_1
#include "test1_common.h"
	CALL func4_2
	if (globalVariable4_1 .eq. 41) then
		print *, '**Failed** test #4 (sequence)'
		print *, '    none of the items were executed'
	else
		if (globalVariable4_1 .eq. 42) then
			print *, '**Failed** test #4 (sequence)'
			print *, '    first item was the last (or only) one to execute'
		else
			if (globalVariable4_1 .eq. 43) then
				print *, 'Passed test #4 (sequence)'
				passedTest (4) = .true.
			endif
		endif
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE func4_2
	INTEGER foo = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE func5_1
#include "test1_common.h"
	CALL func5_2

	if (globalVariable5_1 .eq. 51 .and. globalVariable5_2 .eq. 53)
     & then
			print *, 'Passed test #5 (if w.o. else)'
			passedTest (5) = .true.
		else
			print *, '**Failed** test #5 (if w.o. else)'
			if (globalVariable5_1 .ne. 51) then
				print *, '      condition executed for false'
				print *, '		globalVariable5_1 = ', globalVariable5_1
			endif
			if (globalVariable5_2 .ne. 53) then
				print *, '      condition not executed for true'
				print *, '		globalVariable5_2 = ', globalVariable5_2
			endif
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE func5_2
	INTEGER foo = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE func6_1
#include "test1_common.h"

	CALL func6_2

	if (globalVariable6_1 .eq. 60 + 2 .and.
     & globalVariable6_2 .eq. (64 - 1) .and.
     & globalVariable6_3 .eq. (66 / 3) .and.
     & globalVariable6_4 .eq. (67 / 3) .and.
     & globalVariable6_5 .eq. (6 * 5) .and.
     & globalVariable6_6 .eq. 3 .and.
     & globalVariable6_1a .eq. (60 + 2) .and.
     & globalVariable6_2a .eq. (64 - 1) .and.
     & globalVariable6_3a .eq. (66 / 3) .and.
     & globalVariable6_4a .eq. (67 / 3) .and.
     & globalVariable6_5a .eq. (6 * 5) .and.
     & globalVariable6_6a .eq. 3) then
		print *, 'Passed test #6 (arithmetic operators)'
		passedTest (6) = .true.
	else
		print *, '**Failed** test #6 (arithmetic operators)'

		if (globalVariable6_1 .ne. (60 + 2)) then
			print *, '     addition error 60+2 got ', globalVariable6_1
		endif
		if (globalVariable6_2 .ne. (64 - 1)) then
			print *, '     subtraction error 64-1 got ', globalVariable6_2
		endif
		if (globalVariable6_3 .ne. (66 / 3)) then
			print *, '     division error 66/3 got ', globalVariable6_3
		endif
		if (globalVariable6_4 .ne. (67 / 3)) then
			print *, '     division error 67/3 got ', globalVariable6_4
		endif
		if (globalVariable6_5 .ne. (6 * 5)) then
			print *, '     mult error 6*5 got ', globalVariable6_5
		endif
		if (globalVariable6_6 .ne. 3) then
			print *, '     comma error 10,3 got ', globalVariable6_6
		endif

		if (globalVariable6_1a .ne. (60 + 2)) then
			print *, '     addition error 60+2 got ', globalVariable6_1a
		endif
		if (globalVariable6_2a .ne. (64 - 1)) then
			print *, '     subtraction error 64-1 got ', globalVariable6_2a
		endif
		if (globalVariable6_3a .ne. (66 / 3)) then
			print *, '     division error 66/3 got ', globalVariable6_3a
		endif
		if (globalVariable6_4a .ne. (67 / 3)) then
			print *, '     division error 67/3 got ', globalVariable6_4a
		endif
		if (globalVariable6_5a .ne. (6 * 5)) then
			print *, '     mult error 6*5 got ', globalVariable6_5a
		endif
		if (globalVariable6_6a .ne. 3) then
			print *, '     comma error 10,3 got ', globalVariable6_6a
		endif
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE func6_2
	INTEGER foo = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE fail7Print (tCase, fCase, op)
	INTEGER tCase
	INTEGER fCase
	CHARACTER op *(*)

	if (tCase .ne. 72) then
		print *, ' operator ', op, ' was not true when it should be',
     & ' - const expr'
	endif
	if (fCase .ne. 71) then
		print *, ' operator ', op, ' was not false when it should be',
     & ' - const expr'
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE fail7aPrint (tCase, fCase, op)
	INTEGER tCase
	INTEGER fCase
	CHARACTER op *(*)

	if (tCase .ne. 74) then
		print *, ' operator ', op, ' was not true when it should be',
     & ' - var expr'
	endif
	if (fCase .ne. 73) then
		print *, ' operator ', op, ' was not false when it should be',
     & ' - var expr'
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE func7_1
#include "test1_common.h"

	CALL func7_2

	if (globalVariable7_1 .eq. 72 .and.
     & globalVariable7_2 .eq. 71 .and.
     & globalVariable7_3 .eq. 72 .and.
     & globalVariable7_4 .eq. 71 .and.
     & globalVariable7_5 .eq. 72 .and.
     & globalVariable7_6 .eq. 71 .and.
     & globalVariable7_7 .eq. 72 .and.
     & globalVariable7_8 .eq. 71 .and.
     & globalVariable7_9 .eq. 72 .and.
     & globalVariable7_10 .eq. 71 .and.
     & globalVariable7_11 .eq. 72 .and.
     & globalVariable7_12 .eq. 71 .and.
     & globalVariable7_13 .eq. 72 .and.
     & globalVariable7_14 .eq. 71 .and.
     & globalVariable7_15 .eq. 72 .and.
     & globalVariable7_16 .eq. 71 .and.
     & globalVariable7_1a .eq. 74 .and.
     & globalVariable7_2a .eq. 73 .and.
     & globalVariable7_3a .eq. 74 .and.
     & globalVariable7_4a .eq. 73 .and.
     & globalVariable7_5a .eq. 74 .and.
     & globalVariable7_6a .eq. 73 .and.
     & globalVariable7_7a .eq. 74 .and.
     & globalVariable7_8a .eq. 73 .and.
     & globalVariable7_9a .eq. 74 .and.
     & globalVariable7_10a .eq. 73 .and.
     & globalVariable7_11a .eq. 74 .and.
     & globalVariable7_12a .eq. 73 .and.
     & globalVariable7_13a .eq. 74 .and.
     & globalVariable7_14a .eq. 73 .and.
     & globalVariable7_15a .eq. 74 .and.
     & globalVariable7_16a .eq. 73) then
		print *, 'Passed test #7 (relational operators)'
		passedTest (7) = .true.
	else
		print *, '**Failed** test #7 (relational operators)'
		CALL fail7Print(globalVariable7_1, globalVariable7_2,
     & "BPatch_lt")
		CALL fail7Print(globalVariable7_3, globalVariable7_4,
     & "BPatch_eq")
		CALL fail7Print(globalVariable7_5, globalVariable7_6,
     & "BPatch_gt")
		CALL fail7Print(globalVariable7_7, globalVariable7_8,
     & "BPatch_le")
		CALL fail7Print(globalVariable7_9, globalVariable7_10,
     & "BPatch_ne")
		CALL fail7Print(globalVariable7_11, globalVariable7_12,
     & "BPatch_ge")
		CALL fail7Print(globalVariable7_13, globalVariable7_14,
     & "BPatch_and")
		CALL fail7Print(globalVariable7_15, globalVariable7_16,
     & "BPatch_or")

		CALL fail7aPrint(globalVariable7_1a, globalVariable7_2a,
     & "BPatch_lt")
		CALL fail7aPrint(globalVariable7_3a, globalVariable7_4a,
     & "BPatch_eq")
		CALL fail7aPrint(globalVariable7_5a, globalVariable7_6a,
     & "BPatch_gt")
		CALL fail7aPrint(globalVariable7_7a, globalVariable7_8a,
     & "BPatch_le")
		CALL fail7aPrint(globalVariable7_9a, globalVariable7_10a,
     & "BPatch_ne")
		CALL fail7aPrint(globalVariable7_11a, globalVariable7_12a,
     & "BPatch_ge")
		CALL fail7aPrint(globalVariable7_13a, globalVariable7_14a,
     & "BPatch_and")
		CALL fail7aPrint(globalVariable7_15a, globalVariable7_16a,
     & "BPatch_or")
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE func7_2
	INTEGER foo = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE func8_1 (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)
#include "test1_common.h"
	INTEGER p1, p2, p3, p4, p5, p6, p7, p8, p9, p10

	if (p1 .eq. 1 .and. p2 .eq. 2 .and. p3 .eq. 3 .and. p4 .eq. 4
     & .and. p5 .eq. 5 .and. p6 .eq. 6 .and. p7 .eq. 7 .and. p8 .eq. 8
     & .and. p9 .eq. 9 .and. p10 .eq. 10) then
		print *, 'Passed test #8 (preserve registers - expr)'
		passedTest (8) = .true.
	else
		print *, '**Failed** test #8 (preserve registers - expr)'
		if (p1 .ne. 1) then
			print *, '    parameter #1 is ', p1, ' not 1'
		endif
		if (p2 .ne. 2) then
			print *, '    parameter #2 is ', p2, ' not 2'
		endif
		if (p3 .ne. 3) then
			print *, '    parameter #3 is ', p3, ' not 3'
		endif
		if (p4 .ne. 4) then
			print *, '    parameter #4 is ', p4, ' not 4'
		endif
		if (p5 .ne. 5) then
			print *, '    parameter #5 is ', p5, ' not 5'
		endif
		if (p6 .ne. 6) then
			print *, '    parameter #6 is ', p6, ' not 6'
		endif
		if (p7 .ne. 7) then
			print *, '    parameter #7 is ', p7, ' not 7'
		endif
		if (p8 .ne. 8) then
			print *, '    parameter #8 is ', p8, ' not 8'
		endif
		if (p9 .ne. 9) then
			print *, '    parameter #9 is ', p9, ' not 9'
		endif
		if (p10 .ne. 10) then
			print *, '    parameter #10 is ', p10, ' not 10'
		endif
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE func9_1 (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)
#include "test1_common.h"
	INTEGER p1, p2, p3, p4, p5, p6, p7, p8, p9, p10

	if (p1 .eq. 1 .and. p2 .eq. 2 .and. p3 .eq. 3 .and. p4 .eq. 4
     & .and. p5 .eq. 5 .and. p6 .eq. 6 .and. p7 .eq. 7 .and. p8
     & .eq. 8 .and. p9 .eq. 9 .and. p10 .eq. 10) then
		print *, 'Passed test #9 (preserve registers - funcCall)'
		passedTest (9) = .true.
	else
		print *, '**Failed** test #9 (preserve registers - funcCall)'
		if (p1 .ne. 1) then
			print *, '	parameter #1 is ', p1, ' not 1'
		endif
		if (p2 .ne. 2) then
			print *, '	parameter #2 is ', p2, ' not 2'
		endif
		if (p3 .ne. 3) then
			print *, '	parameter #3 is ', p3, ' not 3'
		endif
		if (p4 .ne. 4) then
			print *, '	parameter #4 is ', p4, ' not 4'
		endif
		if (p5 .ne. 5) then
			print *, '	parameter #5 is ', p5, ' not 5'
		endif
		if (p6 .ne. 6) then
			print *, '	parameter #6 is ', p6, ' not 6'
		endif
		if (p7 .ne. 7) then
			print *, '	parameter #7 is ', p7, ' not 7'
		endif
		if (p8 .ne. 8) then
			print *, '	parameter #8 is ', p8, ' not 8'
		endif
		if (p9 .ne. 9) then
			print *, '	parameter #9 is ', p9, ' not 9'
		endif
		if (p10 .ne. 10) then
			print *, '	parameter #10 is ', p10, ' not 10'
		endif
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE call9_1 (p1, p2, p3, p4, p5)
	INTEGER p1, p2, p3, p4, p5
	INTEGER x

	x = p1 + p2 + p3 + p4 + p5
	if (x .ne. (91 + 92 + 93 + 94 + 95)) then
		print *, '**Failed** test #9 (preserve registers - funcCall)'
		if (p1 .ne. 91) then
			print *, '	call9_1 parameter 1 is ', p1, ' not 91'
		endif
		if (p2 .ne. 92) then
			print *, '	call9_1 parameter 2 is ', p2, ' not 92'
		endif
		if (p3 .ne. 93) then
			print *, '	call9_1 parameter 6 is ', p3, ' not 93'
		endif
		if (p4 .ne. 94) then
			print *, '	call9_1 parameter 4 is ', p4, ' not 94'
		endif
		if (p5 .ne. 95) then
			print *, '	call9_1 parameter 5 is ', p5, ' not 95'
		endif
	endif
	END

!-----------------------------------------------------------------------------

	SUBROUTINE func10_1
#include "test1_common.h"

	if (globalVariable10_1 .eq. 1 .and. globalVariable10_2 .eq. 1
     & .and. globalVariable10_3 .eq. 1 .and. globalVariable10_4 .eq.
     & 3) then
		print *, 'Passed test #10 (insert snippet order)'
		passedTest (10) = .true.
	else
		print *, '**Failed test #10 (insert snippet order)'
		if (globalVariable10_1 .ne. 1) then
			print *, '     call10_1 was not called first'
		endif
		if (globalVariable10_2 .ne. 1) then
			print *, '     call10_2 was not called second'
		endif
		if (globalVariable10_3 .ne. 1) then
			print *, '     call10_3 was not called second'
		endif
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE call10_1
#include "test1_common.h"

	if (globalVariable10_4 .eq. 0) then
		globalVariable10_4 = 1
		globalVariable10_1 = 1
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE call10_2
#include "test1_common.h"

	if (globalVariable10_4 .eq. 1) then
		globalVariable10_4 = 2
		globalVariable10_2 = 1
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE call10_3
#include "test1_common.h"

	if (globalVariable10_4 .eq. 2) then
		globalVariable10_4 = 3
		globalVariable10_3 = 1
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE func11_1
#include "test1_common.h"

	globalVariable11_1 = 1
	CALL func11_2
	globalVariable11_1 = 3
	END

!------------------------------------------------------------------------------

	SUBROUTINE func11_2
#include "test1_common.h"

	globalVariable11_1 = 2
	END

!------------------------------------------------------------------------------

	SUBROUTINE call11_1
#include "test1_common.h"

	if (globalVariable11_1 .eq. 0) then
		globalVariable11_2 = 1
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE call11_2
#include "test1_common.h"

	if (globalVariable11_1 .eq. 1) then
		globalVariable11_3 = 1
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE call11_3
#include "test1_common.h"

	if (globalVariable11_1 .eq. 2) then
		globalVariable11_4 = 1
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE call11_4
#include "test1_common.h"

	if (globalVariable11_1 .eq. 3) then
		globalVariable11_5 = 1
	endif
	if (globalVariable11_2 .ne. 0 .and. globalVariable11_3 .ne. 0
     & .and. globalVariable11_4 .ne. 0 .and. globalVariable11_5
     & .ne. 0) then
		print *, 'Passed test #11 (snippets at entry, exit, call)'
		passedTest (11) = .true.
	else
		print *, '**Failed test #11 (snippets at entry,exit,call)'
		if (globalVariable11_2 .ne. 0) then
			print *, '     entry snippet not called at the correct time'
		endif
		if (globalVariable11_3 .ne. 0) then
			print *, '     pre call snippet not called at the correct time'
		endif
		if (globalVariable11_4 .ne. 0) then
			print *, '     post call snippet not called at the correct time'
		endif
		if (globalVariable11_5 .ne. 0) then
			print *, '     exit snippet not called at the correct time'
		endif
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE func12_1
#include "test1_common.h"

	CALL func12_2
	CALL stop_process_
	CALL func12_2

	if (globalVariable12_1 .eq. 1) then
		print *, 'Passed test #12 (insert/remove and malloc/free)'
		passedTest (12) = .true.
	else
		print *, '**Failed test #12 (insert/remove and malloc/free)'
		print *, 'ZANDY: #12 failed because globalVariable12_1 = ',
     & globalVariable12_1
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE func12_2
	INTEGER foo = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE call12_1
#include "test1_common.h"

	globalVariable12_1 = globalVariable12_1 + 1
	END

!------------------------------------------------------------------------------

	SUBROUTINE func13_1 (p1, p2, p3, p4, p5)
#include "test1_common.h"
	INTRINSIC and
	INTEGER p1, p2, p3, p4, p5, temp, func13_2

	temp = func13_2 ()

	if (p1 .eq. 131 .and. p2 .eq. 132 .and. p3 .eq. 133 .and. p4 .eq.
     & 134 .and. p5 .eq. 135 .and. globalVariable13_1 .eq. 63) then
		print *, 'Passed test #13 (paramExpr,retExpr,nullExpr)'
		passedTest (13) = .true.
	else
		print *, '**Failed test#13 (paramExpr,retExpr,nullExpr)'
		print *, 'globalVariable13_1 = ', globalVariable13_1
		if (p1 .ne. 131) then
			print *, '  parameter 1 is ', p1, ', not 131'
		endif
		if (p2 .ne. 132) then
			print *, '  parameter 2 is ', p2, ', not 132'
		endif
		if (p3 .ne. 133) then
			print *, '  parameter 3 is ', p3, ', not 133'
		endif
		if (p4 .ne. 134) then
			print *, '  parameter 4 is ', p4, ', not 134'
		endif
		if (p5 .ne. 135) then
			print *, '  parameter 5 is ', p5, ', not 135'
		endif
		if (and (globalVariable13_1, 1) .eq. 0) then
			print *, '    passed param a1 wrong'
		endif
		if (and (globalVariable13_1, 2) .eq. 0) then
			print *, '    passed param a2 wrong'
		endif
		if (and (globalVariable13_1, 4) .eq. 0) then
			print *, '    passed param a3 wrong'
		endif
		if (and (globalVariable13_1, 8) .eq. 0) then
			print *, '    passed param a4 wrong'
		endif
		if (and (globalVariable13_1, 16) .eq. 0) then
			print *, '    passed param a5 wrong'
		endif
		if (and (globalVariable13_1, 32) .eq. 0) then
			print *, '    return value wrong'
		endif
	endif
	END

!------------------------------------------------------------------------------

	INTEGER FUNCTION func13_2 ()
	func13_2 = 1300100
	RETURN
	END

!------------------------------------------------------------------------------

	SUBROUTINE call13_1 (a1, a2, a3, a4, a5)
#include "test1_common.h"
	INTRINSIC or
	INTEGER a1, a2, a3, a4, a5

	if (a1 .eq. 131) then
		globalVariable13_1 = or (globalVariable13_1, 1)
	endif
	if (a2 .eq. 132) then
		globalVariable13_1 = or (globalVariable13_1, 2)
	endif
	if (a3 .eq. 133) then
		globalVariable13_1 = or (globalVariable13_1, 4)
	endif
	if (a4 .eq. 134) then
		globalVariable13_1 = or (globalVariable13_1, 8)
	endif
	if (a5 .eq. 135) then
		globalVariable13_1 = or (globalVariable13_1, 16)
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE call13_2 (ret)
#include "test1_common.h"
	INTRINSIC or
	INTEGER ret

	if (ret .eq. 1300100) then
		globalVariable13_1 = or (globalVariable13_1, 32)
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE func14_1
#include "test1_common.h"
	INTEGER kludge

	kludge = 1
	CALL func14_2
	CALL func14_3

	if (globalVariable14_1 .eq. 1 .and. globalVariable14_2
     & .eq. 0) then
		print *, 'Passed test #14 (replace/remove function call)'
		passedTest (14) = .true.
	else
		print *, '**Failed test #14 (replace/remove function call)'
		if (globalVariable14_1 .ne. 1) then
			print *, '     call to func14_2() was not replaced'
		endif
		if (globalVariable14_2 .ne. 0) then
			print *, '     call to func14_3() was not removed'
		endif
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE func14_2
#include "test1_common.h"

	globalVariable14_1 = 2
	END

!------------------------------------------------------------------------------

	SUBROUTINE func14_3
#include "test1_common.h"

	globalVariable14_2 = 1
	END

!------------------------------------------------------------------------------

	SUBROUTINE call14_1
#include "test1_common.h"

	globalVariable14_1 = 1
	END

!------------------------------------------------------------------------------

	SUBROUTINE func15_2
	INTEGER foo = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE func15_3
#include "test1_common.h"

	globalVariable15_3 = 100
	globalVariable15_4 = globalVariable15_4 + 1
	END

!------------------------------------------------------------------------------

	SUBROUTINE func15_4
	INTEGER kludge

	kludge = 1
	CALL func15_3
	END

!------------------------------------------------------------------------------

	SUBROUTINE call15_1
#include "test1_common.h"

	globalVariable15_1 = globalVariable15_1 + 1
	END

!------------------------------------------------------------------------------

	SUBROUTINE call15_2
#include "test1_common.h"

	globalVariable15_2 = globalVariable15_2 + 1
	END

!------------------------------------------------------------------------------

	SUBROUTINE call15_3
#include "test1_common.h"

	globalVariable15_3 = globalVariable15_3 + 1
	END

!------------------------------------------------------------------------------

	SUBROUTINE func15_1
#include "test1_common.h"
	LOGICAL failed

	failed = .false.

	CALL func15_2
	CALL check15result ('globalVariable15_1', globalVariable15_1, 1,
     & 'after first call to instrumented function', failed)

	CALL func15_4
	CALL check15result ('globalVariable15_3', globalVariable15_3, 1,
     & 'after first call to instrumented function', failed)

	CALL stop_process_
	CALL func15_2
	CALL check15result ('globalVariable15_1', globalVariable15_1, 1,
     & 'after second call to instrumented function', failed)

	CALL func15_4
	CALL check15result ('globalVariable15_3', globalVariable15_3,
     & 100, 'after second call to instrumented function', failed)

	CALL stop_process_
	CALL func15_2
	CALL check15result ('globalVariable15_1', globalVariable15_1, 2,
     & 'after third call to instrumented function', failed)

	CALL func15_4
	CALL check15result ('globalVariable15_3', globalVariable15_3,
     & 101, 'after third call to instrumented function', failed)

	if (failed .eqv. .false.) then
		print *, 'Passed test #15 (setMutationsActive)'
		passedTest (15) = .true.
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE check15result (varname, value, expected, errstr,
     & failed)
	CHARACTER *(*) varname
	INTEGER value, expected
	CHARACTER *(*) errstr
	LOGICAL failed

	if (value .ne. expected) then
		if (failed .eqv. .false.) then
			print *, '**Failed test #15 (setMutationsActive)'
		endif
		failed = .true.
		print *, '    ', varname, ' = ', value, ' ', errstr
	endif
	END	

!------------------------------------------------------------------------------

	SUBROUTINE func16_1
#include "test1_common.h"
	LOGICAL failed

	failed = .false.

	CALL func16_2
	if (globalVariable16_1 .ne. 1 .or. globalVariable16_2 .ne. 0) then
		print *, '**Failed test #16 in spot 1 (if-else)'
		if (globalVariable16_1 .ne. 1) then
			print *, '    True clause of first if should have been',
     & ' executed but was not'
		endif
		if (globalVariable16_2 .ne. 0) then
			print *, '    False clause of first if should not have',
     & ' been executed but was'
		endif
		failed = .true.
	endif

	CALL func16_3
	if (globalVariable16_3 .ne. 0 .or. globalVariable16_4 .ne. 1) then
		print *, '**Failed test #16 (if-else)'
		if (globalVariable16_3 .ne. 1) then
			print *, '    True clause of second if should not have',
     & ' been executed but was'
		endif
		if (globalVariable16_4 .ne. 0) then
			print *, '    False clause of second if should have',
     & ' been executed but was not'
		endif
		failed = .true.
	endif

	CALL func16_4
	if (globalVariable16_5 .ne. 0 .or. globalVariable16_6 .ne. 1
     & .or. globalVariable16_7 .ne. 0 .or. globalVariable16_8 .ne. 1
     & .or. globalVariable16_9 .ne. 0 .or. globalVariable16_10 .ne. 1)
     & then
			print *, '    failed large if clauses tests'
			failed = .true.
	endif

	if (failed .eqv. .false.) then
		print *, 'Passed test #16 (if-else)'
		passedTest (16) = .true.
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE func16_2
	INTEGER foo = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE func16_3
	INTEGER foo = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE func16_4
	INTEGER foo = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE func17_1
#include "test1_common.h"
	INTEGER ret17_1
	INTEGER func17_2

	ret17_1 = func17_2 ()
	CALL func17_3

	if (ret17_1 .ne. 1700100 .or. globalVariable17_1 .ne. 1700200 .or.
     & globalVariable17_2 .ne. 1701000) then
		print *, '**Failed** test case #17 (return values from func',
     & ' calls)'
		if (ret17_1 .ne. 1700100) then
			print *, '  return value was ', ret17_1, ', not 1700100'
		endif
		if (globalVariable17_1 .ne. 1700200) then
			print *, '  return value was ', globalVariable17_1,
     & ', not 1700200'
		endif
		if (globalVariable17_2 .ne. 1701000) then
			print *, '  function call17_2 was not inserted'
		endif
	else
		print *, 'Passed test #17 (return values from func calls)'
		passedTest (17) = .true.
	endif
	END

!------------------------------------------------------------------------------

	INTEGER FUNCTION func17_2 ()
	func17_2 = 1700100
	RETURN
	END

!------------------------------------------------------------------------------

	SUBROUTINE func17_3
#include "test1_common.h"
	INTEGER func17_4

	globalVariable17_1 = func17_4()
	END

!------------------------------------------------------------------------------
	INTEGER FUNCTION func17_4 ()
	func17_4 = 1700200
	RETURN
	END

!------------------------------------------------------------------------------

	INTEGER FUNCTION call17_1 (p1)
	INTEGER p1
	INTEGER a1, a2, a3, a4, a5, a6, a7

	a1 = p1
	a2 = a1 + p1
	a3 = a1 * a2
	a4 = a3 / p1
	a5 = a4 + p1
	a6 = a5 + a1
	a7 = a6 + p1
	call17_1 = a7
	return
	END

!------------------------------------------------------------------------------

	INTEGER FUNCTION call17_2 (p1)
#include "test1_common.h"
	INTEGER p1
	INTEGER a1, a2, a3, a4, a5, a6, a7

	a1 = p1
	a2 = a1 + p1
	a3 = a1 * a2
	a4 = a3 / p1
	a5 = a4 + p1
	a6 = a5 + a1
	a7 = a6 + p1
	globalVariable17_2 = 1701000
	call17_2 = a7
	return
	END

!------------------------------------------------------------------------------

	SUBROUTINE func18_1
#include "test1_common.h"

	if (globalVariable18_1 .eq. 17) then
		print *, 'Passed test #18 (read/write a value in the mutatee)'
		passedTest (18) = .true.
	else
		print *, '**Failed test #18 (read/write a value in the mutatee)'
		if (globalVariable18_1 .eq. 42) then
			print *, '    globalVariable18_1 still contains 42 (',
     & ' probably it was not written to)'
		else
			print *, '    globalVariable18_1 contained ',
     & globalVariable18_1, ', not 17 as expected'
		endif
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE func19_1
#include "test1_common.h"

	CALL stop_process_

	if (globalVariable19_1 .ne. 1900100) then
		print *, '**Failed test #19 (oneTimeCode)'
		print *, '     globalVariable19_1 contained ',
     & globalVariable19_1, ', not 1900100 as expected'
	endif

	CALL stop_process_

	if (globalVariable19_2 .eq. 1900200) then
		print *, 'Passed test #19 (oneTimeCode)'
		passedTest (19) = .true.
	else
		print *, '**Failed test #19 (oneTimeCode)'
		print *, '     globalVariable19_2 contained ',
     & globalVariable19_2, ', not 1900200 as expected'
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE call19_1
#include "test1_common.h"

	globalVariable19_1 = 1900100
	END

!------------------------------------------------------------------------------

	SUBROUTINE call19_2
#include "test1_common.h"

	globalVariable19_2 = 1900200
	END

!------------------------------------------------------------------------------

	SUBROUTINE call20_1
#include "test1_common.h"
	INTEGER ta
	DOUBLE PRECISION tb

	ta = 3
	tb = 4.3

	globalVariable20_1 = ta + ta + ta + ta + ta + ta + ta + ta + ta +
     & ta + ta + ta + ta + ta + ta + ta + ta + ta + ta + ta + ta +
     & ta + ta + ta + ta + ta + ta + ta + ta + ta + ta + ta + ta +
     & ta + ta + ta + ta + ta + ta + ta + ta
	globalVariable20_2 = tb + tb + tb + tb + tb + tb + tb + tb + tb +
     & tb + tb + tb + tb + tb + tb + tb + tb + tb + tb + tb + tb +
     & tb + tb + tb + tb + tb + tb + tb + tb + tb + tb + tb + tb +
     & tb + tb + tb + tb + tb + tb + tb + tb
	END

!------------------------------------------------------------------------------

	INTEGER FUNCTION func20_2 (int_val, double_val)
	INTEGER int_val
	DOUBLE PRECISION double_val
	INTEGER tc, i, ret
	DOUBLE PRECISION td

	tc = 7
	td = 6.4
	ret = 1

	int_val = tc + tc + tc + tc + tc + tc + tc + tc + tc + tc + tc +
     & tc + tc + tc + tc + tc + tc + tc + tc + tc + tc + tc + tc +
     & tc + tc + tc + tc + tc + tc + tc + tc + tc + tc + tc + tc +
     & tc + tc + tc + tc + tc + tc
	double_val = td + td + td + td + td + td + td + td + td + td +
     & td + td + td + td + td + td + td + td + td + td + td + td +
     & td + td + td + td + td + td + td + td + td + td + td + td +
     & td + td + td + td + td + td + td

	DO i = 0, 49, 1
		ret = ret * 3
		if (mod (i, 2) .eq. 1) then
			ret = ret * 5
		elseif (i .lt. 10) then
			ret = ret * 7
		elseif (i .gt. 20) then
			ret = ret * 11
		endif
	ENDDO

	func20_2 = ret
	RETURN
	END

!------------------------------------------------------------------------------

	SUBROUTINE func20_1
#include "test1_common.h"
	INTEGER ret, int_val, func20_2, eq_doubles
	DOUBLE PRECISION double_val, temp1, temp2

	ret = func20_2 (int_val, double_val)

	temp1 = 4.3 * 41
	temp2 = 6.4 * 41

	if (globalVariable20_1 .eq. 3 * 41 .and. eq_doubles
     & (globalVariable20_2, temp1) .eq. 1 .and. int_val .eq.
     & 7 * 41 .and. eq_doubles (double_val, temp2) .eq. 1
     & .and. ret .eq. 1088896211) then
		print *, 'Passed test #20 (instrument arbitrary points)'
		passedTest (20) = .true.
	else
		print *, '**Failed test #20 (instrument arbitrary points)'
		if (globalVariable20_1 .ne. (3 * 41)) then
			print *, '    globalVariable20_1 contained ',
     & globalVariable20_1, ', not ', 3 * 41, ' as expected'
		endif
		if (eq_doubles (globalVariable20_2, temp1) .ne. 1) then
			print *, '    globalVariable20_2 contained ',
     & globalVariable20_2, ', not ', temp1, ' as expected'
		endif
		if (int_val .ne. (7 * 41)) then
			print *, '    int_val contained ', int_val, ', not ',
     & (7 * 41), ' as expected'
		endif
		if (eq_doubles (double_val, temp2) .ne. 1) then
			print *, '    double_val contained', double_val, ', not ',
     & temp2, ' as expected'
		endif
		if (ret .ne. 1088896211) then
			print *, '    ret contained ', ret, ', not 1088896211 as',
     & ' expected'
		endif
	endif
	END

!------------------------------------------------------------------------------

	INTEGER FUNCTION eq_doubles (a, b)
	DOUBLE PRECISION a, b, diff

	diff = a - b

	if (diff .lt. 0) then
		diff = -1 * diff
	endif

	if (diff .lt. 0.01) then
		eq_doubles = 1
	else
		eq_doubles = 0
	endif
	RETURN
	END

!------------------------------------------------------------------------------

	SUBROUTINE func21_1
#include "test1_common.h"

	print *, 'Passed test #21 (findFunction in module)'
	passedTest (21) = .true.
	END

!------------------------------------------------------------------------------

	SUBROUTINE func22_1
#include "test1_common.h"
#if	!defined(sparc_sun_solaris2_4) && \
	!defined(alpha_dec_osf4_0)

	print *, 'Skipped test #22 (replace function)'
	print *, '       - not implemented on this platform'
#endif
	passedTest (22) = .true.
	END

!------------------------------------------------------------------------------

	SUBROUTINE func23_1
#include "test1_common.h"

	print *, 'Skipped test #23 (local Variables)'
	print *, '       - not implemented for Fortran'
	passedTest (23) = .true.
	END

!------------------------------------------------------------------------------

	SUBROUTINE FverifyScalarValue (name, a, value, testNum, testName)
#include "test1_common.h"
	CHARACTER *(*) name, testName
	INTEGER a, value, testNum

	if (a .ne. value) then
		if (passedTest (testNum + 1) .eqv. .true.) then
			print *, '**Failed** test ', testNum, ' (', testName, ')'
		endif
		print *, '  ', name, ' = ', a, ', not ', value
		passedTest (testNum + 1) = .false.
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE func24_1
#include "test1_common.h"

	print *, 'Skipped test #24 (arrary Variables)'
	print *, '       - not implemented for Fortran'
	passedTest (24) = .true.
	END

!------------------------------------------------------------------------------

	SUBROUTINE func25_1

#include "test1_common.h"
#if defined(mips_sgi_irix6_4)
	print *, 'Skipped test #25 (unary operators)'
	print *, '       - not implemented on this platform'
	passedTest (25) = .true.
#else
	passedTest (25) = .true.
	globalVariable25_1 = 25000001
!	globalVariable25_2 = (int *) 25000002
	globalVariable25_3 = 25000003
	globalVariable25_4 = 25000004
	globalVariable25_5 = 25000005
	globalVariable25_6 = -25000006
	globalVariable25_7 = 25000007

	CALL call25_1

	if (globalVariable25_3 .ne. globalVariable25_1) then
		if (passedTest (25) .eqv. .true.) then
			print *, '**Failed** test #25 (unary operators)'
		endif
		passedTest (25) = .false.
		print *, '    globalVariable25_2 = ', globalVariable25_3,
     & ', not ', globalVariable25_1
	endif

	if (globalVariable25_5 .ne. -1 * globalVariable25_4) then
		if (passedTest (25) .eqv. .true.) then
			print *, '**Failed** test #25 (unary operators)'
		endif
		passedTest (25) = .false.
		print *, '    globalVariable25_5 = ', globalVariable25_5,
     & ', not ', -1 * globalVariable25_4
	endif

	if (globalVariable25_7 .ne. -1 * globalVariable25_6) then
		if (passedTest (25) .eqv. .true.) then
			print *, '**Failed** test #25 (unary operators)'
		endif
		passedTest (25) = .false.
		print *, '    globalVariable25_7 = ', globalVariable25_7,
     & ', not ', -1 * globalVariable25_6
	endif

	if (passedTest (25) .eqv. .true.) then
		print *, 'Passed test #25 (unary operators)'
	endif
#endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE call25_1
#include "test1_common.h"
	integer place

	place = 23
	END

!------------------------------------------------------------------------------

	SUBROUTINE func26_1
#include "test1_common.h"

	print *, 'Skipped test #26 (field operators)'
	print *, '       - not implemented in Fortran'
	passedTest (26) = .true.
#if notdef
	INTEGER i

        TYPE (struct26_2) globalVariable26_1

	passedTest (26) = .true.
	globalVariable26_1%field1 = 26001001
	globalVariable26_1%field2 = 26001002

	DO i = 1, 10, 1
	globalVariable26_1%field3 (i) = 26001002 + i
	ENDDO

	globalVariable26_1%field4%field1 = 26000013;
	globalVariable26_1%field4%field2 = 26000014;

	CALL call26_1

	CALL FverifyScalarValue ("globalVariable26_2",
     & globalVariable26_2, 26001001, 26, "field operators")
	CALL FverifyScalarValue ("globalVariable26_3",
     & globalVariable26_3, 26001002, 26, "field operators")
	CALL FverifyScalarValue ("globalVariable26_4",
     & globalVariable26_4, 26001003, 26, "field operators")
	CALL FverifyScalarValue ("globalVariable26_5",
     & globalVariable26_5, 26001003+5, 26, "field operators")
	CALL FverifyScalarValue ("globalVariable26_6",
     & globalVariable26_6, 26000013, 26, "field operators")
	CALL FverifyScalarValue ("globalVariable26_7",
     & globalVariable26_7, 26000014, 26, "field operators")

	CALL FverifyScalarValue ("globalVariable26_8",
     & globalVariable26_8, 26002001, 26, "field operators")
	CALL FverifyScalarValue ("globalVariable26_9",
     & globalVariable26_9, 26002002, 26, "field operators")
	CALL FverifyScalarValue ("globalVariable26_10",
     & globalVariable26_10, 26002003, 26, "field operators")
	CALL FverifyScalarValue ("globalVariable26_11",
     & globalVariable26_11, 26002003+5, 26, "field operators")
	CALL FverifyScalarValue ("globalVariable26_12",
     & globalVariable26_12, 26002013, 26, "field operators")
	CALL FverifyScalarValue ("globalVariable26_13",
     & globalVariable26_13, 26002014, 26, "field operators")

	if (passedTest (26) .eqv. .true.) then
		print *, 'Passed test #26 (field operators)'
	endif
#else
#endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE call26_1
#include "test1_common.h"

	INTEGER i

#if !defined (F77)
	TYPE (struct26_2) localVariable26_1

	localVariable26_1%field1 = 26002001
	localVariable26_1%field2 = 26002002

	DO i = 1, 10, 1
	localVariable26_1%field3 (i) = 26002002 + i
	ENDDO

	localVariable26_1%field4%field1 = 26002013
	localVariable26_1%field4%field2 = 26002014

	CALL call26_2
#endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE call26_2
	INTEGER foo = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE func27_1
#include "test1_common.h"

	passedTest (27) = .true.
	print *, 'Skipped test #27 (type compatibility)'
	print *, '       - not implemented for Fortran'
	END

!------------------------------------------------------------------------------

	SUBROUTINE func28_1
#include "test1_common.h"
	passedTest (28) = .true.
	print *, 'Skipped test #28 (user defined fields)'
	print *, '       - not implemented for Fortran'
	END

!------------------------------------------------------------------------------

	SUBROUTINE func29_1
#include "test1_common.h"

	if (globalVariable29_1 .eq. 1) then
		passedTest (29) = .true.
		print *, 'Passed test #29 (BPatch_srcObj class)'
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE func30_1
#include "test1_common.h"
	passedTest (30) = .true.
	print *, 'Skipped test #30 (line information)'
	print *, '       - not implemented for Fortran'
	END

!------------------------------------------------------------------------------

	INTEGER FUNCTION func31_1 ()
#include "test1_common.h"

#if defined(alpha_dec_osf4_0)
        print *, 'Skipped test #31 (non-recursive base tramp guard)'
        print *, '       - not implemented on this platform.'
	passedTest (31) = .true.
        END
#else

	globalVariable31_1 = 0
	globalVariable31_2 = 0
	globalVariable31_3 = 0
	globalVariable31_4 = 0

	CALL func31_2

	if (globalVariable31_3 .eq. 1) then
		passedTest (31) = .true.
	endif

	if (passedTest (31) .eqv. .false.) then
		print *, '**Failed** test #31 (non-recursive base tramp guard)'
		print *, '    globalVariable31_3 = ', globalVariable31_3,
     & ', should be 1 (no instrumentation got exectued?)'
		func31_1 = 0
		RETURN
	endif

	if (globalVariable31_4 .eq. 0) then
		passedTest (31) = .true.
	endif

	if (passedTest (31) .eqv. .false.) then
		print *, '**Failed** test #31 (non-recursive base tramp guard)'
		print *, '    globalVariable31_4 = ', globalVariable31_4,
     & ', should be 0.'
		if (globalVariable31_4 .eq. 0) then
			print *, '    Recursive guard works find.'
		elseif (globalVariable31_4 .eq. 1) then
			print *, '    Pre-instr recursive guard does not work.'
		elseif (globalVariable31_4 .eq. 2) then
			print *, '    Post-instr recursive guard does not work.'
		elseif (globalVariable31_4 .eq. 3) then
			print *, '    None of the recursive guards work.'
		else
			print *, '    Something is really wrong.'
		endif
		func31_1 = 0
		RETURN
	endif

	passedTest (31) = .true.
	print *, 'Passed test #31 (non-recursive base tramp guard)'

	func31_1 = 1
	RETURN
	END
#endif

!------------------------------------------------------------------------------

	SUBROUTINE func31_2
#include "test1_common.h"

	globalVariable31_2 = 1
	END

!------------------------------------------------------------------------------

	SUBROUTINE func31_3
#include "test1_common.h"

	globalVariable31_3 = 1
	END

!------------------------------------------------------------------------------

	SUBROUTINE func31_4 (value)
#include "test1_common.h"
	INTEGER value

	if (value .eq. 0) then
		print *, 'func31_4 called with calue = 0 !'
	endif
	globalVariable31_4 = globalVariable31_4 + value
	END

!------------------------------------------------------------------------------

	INTEGER FUNCTION func32_1 ()
#include "test1_common.h"

	globalVariable32_1 = 0
	globalVariable32_2 = 0
	globalVariable32_3 = 0
	globalVariable32_4 = 0

	CALL func32_2

	if (globalVariable32_3 .eq. 1) then
		passedTest (32) = .true.
	endif

	if (passedTest (32) .eqv. .false.) then
		print *, '**Failed** test #32 (non-recursive base tramp guard)'
		print *, '    globalVariable32_3 = ', globalVariable32_3,
     & ', should be 1 (no instrumentation got executed?)'
		func32_1 = 0
		RETURN
	endif

	if (globalVariable32_4 .eq. 3) then
		passedTest (32) = .true.
	endif

	if (passedTest (32) .eqv. .false.) then
		print *, '**Failed** test #32 (non-recursive base tramp guard)'
		print *, '    globalVariable32_4 = ', globalVariable32_4,
     & ', should be 3.'
		if (globalVariable32_4 .eq. 0) then
			print *, '    Recursive guard works fine.'
		elseif (globalVariable32_4 .eq. 1) then
			print *, '    Pre-instr recursive guard does not work.'
		elseif (globalVariable32_4 .eq. 2) then
			print *, '    Post-instr recursive guard does not work.'
		elseif (globalVariable32_4 .eq. 1) then
			print *, '    None of the recursive guards work.'
		else
			print *, '    Something is really wrong.'
		endif

		func32_1 = 0
		RETURN
	endif

	passedTest (32) = .true.
	print *, 'Passed test #32 (recursive base tramp guard)'

	func32_1 = 1
	RETURN
	END

!------------------------------------------------------------------------------

	SUBROUTINE func32_2
#include "test1_common.h"

	globalVariable32_2 = 1
	END

!------------------------------------------------------------------------------

	SUBROUTINE func32_3
#include "test1_common.h"

	globalVariable32_3 = 1
	END

!------------------------------------------------------------------------------

	SUBROUTINE func32_4 (value)
#include "test1_common.h"
	INTEGER value

	if (value .eq. 0) then
		print *, 'func32_4 called with value = 0 !'
	endif

	globalVariable32_4 = globalVariable32_4 + value
	END

!------------------------------------------------------------------------------

	SUBROUTINE func33_1
#include "test1_common.h"

	passedTest (33) = .true.
	print *, 'Passed test #33 (control flow graphs)'
	END

!------------------------------------------------------------------------------

	SUBROUTINE func33_2 (x)
#include "test1_common.h"
	INTEGER x

	print *, 'Hello'

	if (x .eq. 1) then
		print *, 'Goodbye.'
	else
		print *, 'See you.'
	endif

	print *, 'That"s all.'
	END

!------------------------------------------------------------------------------

	INTEGER FUNCTION func33_3 (x)
	INTEGER x
C	INTRINSIC IEOR, IAND, IOR

	if (x .eq. 1) then
		print *, '1'
		x = x + 10
	elseif (x .eq. 2) then
		print *, '2'
		x = x -12
	elseif (x .eq. 3) then
		print *, '3'
		x = x * 33
	elseif (x .eq. 4) then
		print *, '4'
		x = x / 42
	elseif (x .eq. 5) then
		print *, '5'
		x = mod (x, 57)
	elseif (x .eq. 6) then
		print *, '6'
		x = mod (x, 2)
	elseif (x .eq. 7) then
		print *, '7'
		x = mod (x, 3)
	elseif (x .eq. 8) then
		print *, '8'
		x = mod (x, 4)
	elseif (x .eq. 9) then
		print *, '9'
		x = mod (x, 5)
	elseif (x .eq. 10) then
		print *, '10'
		x = mod (x, 6)
	endif

	print *, 'Exit'

	func33_3 = x
	RETURN
	END

!------------------------------------------------------------------------------

	SUBROUTINE func34_1
#include "test1_common.h"

	passedTest (34) = .true.
	print *, 'Passed test #34 (loop information)'
	END

!------------------------------------------------------------------------------

	SUBROUTINE func34_2
	INTEGER i, j, k, kludge

	kludge = 1
	DO i = 0, 9, 1
		print *, 'i = ', i
		DO j = 0, 9, 1
			print *, 'j = ', j
			k = 0

10			if (k .ge. 10) GOTO 20
				print *, 'k = ', k
				k = k + 1
			GOTO 10
20			CONTINUE
		ENDDO

30		CONTINUE
		j = j + 1
		print *, 'j = ', j
		if (j .lt. 10) GOTO 30
	ENDDO
	END

!------------------------------------------------------------------------------

	SUBROUTINE func35_1
#include "test1_common.h"
	passedTest (35) = .true.
#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0) || defined(sparc_sun_solaris2_4) || defined(ia64_unknown_linux2_4)
	print *, 'Skipped test #35 (function relocation)'
	print *, '       - not tested for Fortran'
#else
	print *, 'Skipped test #35 (function relocation)'
	print *, '       - not implemented on this platform'
#endif
	END

!------------------------------------------------------------------------------

	INTEGER FUNCTION call36_1(y1, y2, y3, y4, y5, y6, y7, y8, y9, y10)
#include "test1_common.h"
	INTEGER y1, y2, y3, y4, y5, y6, y7, y8, y9, y10
	call36_1 = y1 + y2 + y3 + y4 + y5 + y6 + y7 + y8 + y9 + y10
	END FUNCTION call36_1


	SUBROUTINE func36_1
#include "test1_common.h"
	INTEGER failure, result
	INTEGER call36_1
	globalVariable36_1 = 0
	globalVariable36_2 = 0
	globalVariable36_3 = 0
	globalVariable36_4 = 0
	globalVariable36_5 = 0
	globalVariable36_6 = 0
	globalVariable36_7 = 0
	globalVariable36_8 = 0
	globalVariable36_9 = 0
	globalVariable36_10 = 0

!  ==================================================

!  This was subtest was skipped for Fortran because
!  findVariable(globalVariable36_x) fails when called by the mutator.
!  Every other subtest that does a findVariable on some variable in
!  this Fortran mutatee is skipped, so this must be an unimplemented
!  feature on Fortran.  As soon as this is implemented, this test
!  should be able to be activated.

!	passedTest (36) = .true.
!	print *, 'Skipped test #36 (callsite parameter referencing)'
!	print *, '       - not implemented for Fortran'
!	RETURN
!  ==================================================

	result = call36_1(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
	failure = 0

	if (result .ne. 55) then
		print *, '  for test 36, expecting a value of 55 from call36_1, '
		print *, 'got: ', result
		failure = 1
	endif

#if	defined(rs6000_ibm_aix4_1) && !defined(AIX5)
	call xlf90_41_hack()
#endif

	if (globalVariable36_1 .ne. 1) then
		print *, '  for test 36, expecting arg1 value of 1, got ',
     &		globalVariable36_1
		failure = 1
	endif

	if (globalVariable36_2 .ne. 2) then
		print *, '  for test 36, expecting arg2 value of 2, got ',
     &		globalVariable36_2
		failure = 1
	endif

	if (globalVariable36_3 .ne. 3) then
		print *, '  for test 36, expecting arg3 value of 3, got ',
     &		globalVariable36_3
		failure = 1
	endif

	if (globalVariable36_4 .ne. 4) then
		print *, '  for test 36, expecting arg4 value of 4, got ',
     &		globalVariable36_4
		failure = 1
	endif

	if (globalVariable36_5 .ne. 5) then
		print *, '  for test 36, expecting arg5 value of 5, got ',
     &		globalVariable36_5
		failure = 1
	endif

	if (globalVariable36_6 .ne. 6) then
		print *, '  for test 36, expecting arg6 value of 6, got ',
     &		globalVariable36_6
		failure = 1
	endif

	if (globalVariable36_7 .ne. 7) then
		print *, '  for test 36, expecting arg7 value of 7, got ',
     &		globalVariable36_7
#if     defined(alpha_dec_osf4_0)
		print *, '  not an error since it needs to be implemented',
     &		' on this platform'
#else
		failure = 1
#endif
	endif

	if (globalVariable36_8 .ne. 8) then
		print *, '  for test 36, expecting arg8 value of 8, got ',
     &		globalVariable36_8
#if     defined(alpha_dec_osf4_0)
		print *, '  not an error since it needs to be implemented',
     &		' on this platform'
#else
		failure = 1
#endif
	endif

	if (globalVariable36_9 .ne. 9) then
		print *, '  for test 36, expecting arg9 value of 9, got ',
     &		globalVariable36_9
#if	defined(sparc_sun_solaris2_4) || defined(alpha_dec_osf4_0)
		print *, '  not an error since it needs to be implemented',
     &		' on this platform'
#else
		failure = 1
#endif
	endif

	if (globalVariable36_10 .ne. 10) then
		print *, '  for test 36, expecting arg10 value of 10, got ',
     &		globalVariable36_10
#if	defined(sparc_sun_solaris2_4) || defined(alpha_dec_osf4_0)
		print *, '  not an error since it needs to be implemented',
     &		' on this platform'
#else
		failure = 1
#endif
	endif

	if (failure .eq. 0) then
		passedTest (36) = .true.
		print *, 'Passed test #36 (callsite parameter referencing)'
	else
		passedTest (36) = .false.
		print *, '**Failed** test #36 (callsite parameter referencing)'
	endif

	END

!------------------------------------------------------------------------------

	SUBROUTINE func37_1
#include "test1_common.h"
	passedTest (37) = .true.
	print *, 'Skipped test #37 (instrument loops)'
	print *, '       - not implemented for Fortran'
	END

!------------------------------------------------------------------------------

	SUBROUTINE func38_1
#include "test1_common.h"
	passedTest (38) = .true.
	print *, 'Skipped test #38 (basic block addresses)'
	print *, '       - not implemented for Fortran'
	END

!------------------------------------------------------------------------------

	SUBROUTINE func39_1
#include "test1_common.h"
	passedTest (39) = .true.
	print *, 'Passed test #39 (regex function search)'
	END

!------------------------------------------------------------------------------
