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

	SUBROUTINE test1_36f_init_globals
#include "test1_36F_common.h"

	test1_36_globalVariable1 = 0
	test1_36_globalVariable2 = 0
	test1_36_globalVariable3 = 0
	test1_36_globalVariable4 = 0
	test1_36_globalVariable5 = 0
	test1_36_globalVariable6 = 0
	test1_36_globalVariable7 = 0
	test1_36_globalVariable8 = 0
	test1_36_globalVariable9 = 0
	test1_36_globalVariable10 = 0
	END

!------------------------------------------------------------------------------

	INTEGER FUNCTION test1_36_call1(y1, y2, y3, y4, y5, y6, y7, y8, y9, y10)
#include "test1_36F_common.h"
	INTEGER y1, y2, y3, y4, y5, y6, y7, y8, y9, y10
	test1_36_call1 = y1 + y2 + y3 + y4 + y5 + y6 + y7 + y8 + y9 + y10
	END FUNCTION test1_36_call1


	SUBROUTINE test1_36_func1
#include "test1_36F_common.h"
	INTEGER failure, result
	INTEGER test1_36_call1
	test1_36_globalVariable1 = 0
	test1_36_globalVariable2 = 0
	test1_36_globalVariable3 = 0
	test1_36_globalVariable4 = 0
	test1_36_globalVariable5 = 0
	test1_36_globalVariable6 = 0
	test1_36_globalVariable7 = 0
	test1_36_globalVariable8 = 0
	test1_36_globalVariable9 = 0
	test1_36_globalVariable10 = 0

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

	result = test1_36_call1(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
	failure = 0

	if (result .ne. 55) then
		print *, '  for test 36, expecting a value of 55 from test1_36_call1, '
		print *, 'got: ', result
		failure = 1
	endif

#if defined(rs6000_ibm_aix4_1) && !defined(AIX5)
	call xlf90_41_hack()
#endif

	if (test1_36_globalVariable1 .ne. 1) then
		print *, '  for test 36, expecting arg1 value of 1, got ',
     &		test1_36_globalVariable1
		failure = 1
	endif

	if (test1_36_globalVariable2 .ne. 2) then
		print *, '  for test 36, expecting arg2 value of 2, got ',
     &		test1_36_globalVariable2
		failure = 1
	endif

	if (test1_36_globalVariable3 .ne. 3) then
		print *, '  for test 36, expecting arg3 value of 3, got ',
     &		test1_36_globalVariable3
		failure = 1
	endif

	if (test1_36_globalVariable4 .ne. 4) then
		print *, '  for test 36, expecting arg4 value of 4, got ',
     &		test1_36_globalVariable4
		failure = 1
	endif

	if (test1_36_globalVariable5 .ne. 5) then
		print *, '  for test 36, expecting arg5 value of 5, got ',
     &		test1_36_globalVariable5
		failure = 1
	endif

	if (test1_36_globalVariable6 .ne. 6) then
		print *, '  for test 36, expecting arg6 value of 6, got ',
     &		test1_36_globalVariable6
		failure = 1
	endif

	if (test1_36_globalVariable7 .ne. 7) then
		print *, '  for test 36, expecting arg7 value of 7, got ',
     &		test1_36_globalVariable7
#if defined(alpha_dec_osf4_0)
		print *, '  not an error since it needs to be implemented',
     &		' on this platform'
#else
		failure = 1
#endif
	endif

	if (test1_36_globalVariable8 .ne. 8) then
		print *, '  for test 36, expecting arg8 value of 8, got ',
     &		test1_36_globalVariable8
#if defined(alpha_dec_osf4_0)
		print *, '  not an error since it needs to be implemented',
     &		' on this platform'
#else
		failure = 1
#endif
	endif

	if (test1_36_globalVariable9 .ne. 9) then
		print *, '  for test 36, expecting arg9 value of 9, got ',
     &		test1_36_globalVariable9
#if defined(sparc_sun_solaris2_4) \
 || defined(alpha_dec_osf4_0)
		print *, '  not an error since it needs to be implemented',
     &		' on this platform'
#else
		failure = 1
#endif
	endif

	if (test1_36_globalVariable10 .ne. 10) then
		print *, '  for test 36, expecting arg10 value of 10, got ',
     &		test1_36_globalVariable10
#if defined(sparc_sun_solaris2_4) \
 || defined(alpha_dec_osf4_0)
		print *, '  not an error since it needs to be implemented',
     &		' on this platform'
#else
		failure = 1
#endif
	endif

	if (failure .eq. 0) then
		passedTest = 1
		print *, 'Passed test #36 (callsite parameter referencing)'
	else
		passedTest = 0
		print *, '**Failed** test #36 (callsite parameter referencing)'
	endif

	END

!------------------------------------------------------------------------------
