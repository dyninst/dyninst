 !
 ! Copyright (c) 1996-2004 Barton P. Miller
 ! 
 ! We provide the Paradyn Parallel Performance Tools (below
 ! described as "Paradyn") on an AS IS basis, and do not warrant its
 ! validity or performance.  We reserve the right to update, modify,
 ! or discontinue this software at any time.  We shall have no
 ! obligation to supply such updates or modifications or any other
 ! form of support to you.
 ! 
 ! This license is for research uses.  For such uses, there is no
 ! charge. We define "research use" to mean you may freely use it
 ! inside your organization for whatever purposes you see fit. But you
 ! may not re-distribute Paradyn or parts of Paradyn, in any form
 ! source or binary (including derivatives), electronic or otherwise,
 ! to any other organization or entity without our permission.
 ! 
 ! (for other uses, please contact us at paradyn@cs.wisc.edu)
 ! 
 ! All warranties, including without limitation, any warranty of
 ! merchantability or fitness for a particular purpose, are hereby
 ! excluded.
 ! 
 ! By your use of Paradyn, you understand and agree that we (or any
 ! other person or entity with proprietary rights in Paradyn) are
 ! under no obligation to provide either maintenance services,
 ! update services, notices of latent defects, or correction of
 ! defects for Paradyn.
 ! 
 ! Even if advised of the possibility of such damages, under no
 ! circumstances shall we (or any other person or entity with
 ! proprietary rights in the software licensed hereunder) be liable
 ! to you or any third party for direct, indirect, or consequential
 ! damages of any character regardless of type of action, including,
 ! without limitation, loss of profits, loss of use, loss of good
 ! will, or computer failure or malfunction.  You agree to indemnify
 ! us (and any other person or entity with proprietary rights in the
 ! software licensed hereunder) for any and all liability it may
 ! incur to third parties resulting from your use of Paradyn.
 !

! libdyninst validation suite test #1
! author: Jon Beyer (6/1/01)
!	derived from a previous test by Jeff Hollingsworth

!------------------------------------------------------------------------------

! AIX xlf90 does not insert underscores at end

#if !defined(XLF)
#define stop_process_ stop_process
#endif

#ifdef __GNUC__
#define AND iand
#define OR ior
#else
#define AND and
#define OR or
#endif

!------------------------------------------------------------------------------

	SUBROUTINE test1_16f_init_globals
#include "test1_16F_common.h"

	test1_16_globalVariable16_1 = 0
	test1_16_globalVariable16_2 = 0
	test1_16_globalVariable16_3 = 0
	test1_16_globalVariable16_4 = 0
	test1_16_globalVariable16_5 = 0
	test1_16_globalVariable16_6 = 0
	test1_16_globalVariable16_7 = 0
	test1_16_globalVariable16_8 = 0
	test1_16_globalVariable16_9 = 0
	test1_16_globalVariable16_10 = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_16_func1
#include "test1_16F_common.h"
	LOGICAL failed

	failed = .false.

	CALL test1_16_func2
	if (test1_16_globalVariable16_1 .ne. 1 .or. test1_16_globalVariable16_2 .ne. 0) then
		print *, '**Failed test #16 in spot 1 (if-else)'
		if (test1_16_globalVariable16_1 .ne. 1) then
			print *, '    True clause of first if should have been',
     & ' executed but was not'
		endif
		if (test1_16_globalVariable16_2 .ne. 0) then
			print *, '    False clause of first if should not have',
     & ' been executed but was'
		endif
		failed = .true.
	endif

	CALL test1_16_func3
	if (test1_16_globalVariable16_3 .ne. 0 .or. test1_16_globalVariable16_4 .ne. 1) then
		print *, '**Failed test #16 (if-else)'
		if (test1_16_globalVariable16_3 .ne. 1) then
			print *, '    True clause of second if should not have',
     & ' been executed but was'
		endif
		if (test1_16_globalVariable16_4 .ne. 0) then
			print *, '    False clause of second if should have',
     & ' been executed but was not'
		endif
		failed = .true.
	endif

	CALL test1_16_func4
	if (test1_16_globalVariable16_5 .ne. 0 .or. test1_16_globalVariable16_6 .ne. 1
     & .or. test1_16_globalVariable16_7 .ne. 0
     & .or. test1_16_globalVariable16_8 .ne. 1
     & .or. test1_16_globalVariable16_9 .ne. 0
     & .or. test1_16_globalVariable16_10 .ne. 1)
     & then
			print *, '    failed large if clauses tests'
			failed = .true.
	endif

	if (failed .eqv. .false.) then
		print *, 'Passed test #16 (if-else)'
		passedTest = 1
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_16_func2
	INTEGER foo = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_16_func3
	INTEGER foo = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_16_func4
	INTEGER foo = 0
	END

!------------------------------------------------------------------------------
