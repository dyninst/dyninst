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

	SUBROUTINE test1_25f_init_globals
#include "test1_25F_common.h"

	END

!------------------------------------------------------------------------------

	SUBROUTINE func25_1

#include "test1_25F_common.h"
	LOGICAL passed
	passed = .true.
	test1_25_globalVariable1 = 25000001
!	test1_25_globalVariable2 = (int *) 25000002
	test1_25_globalVariable3 = 25000003
	test1_25_globalVariable4 = 25000004
	test1_25_globalVariable5 = 25000005
	test1_25_globalVariable6 = -25000006
	test1_25_globalVariable7 = 25000007

	CALL test1_25_call1

	if (test1_25_globalVariable3 .ne. test1_25_globalVariable1) then
		if (passed .eqv. .true.) then
			print *, '**Failed** test #25 (unary operators)'
		endif
		passed = .false.
		print *, '    test1_25_globalVariable2 = ', test1_25_globalVariable3,
     & ', not ', test1_25_globalVariable1
	endif

	if (test1_25_globalVariable5 .ne. -1 * test1_25_globalVariable4) then
		if (passed .eqv. .true.) then
			print *, '**Failed** test #25 (unary operators)'
		endif
		passed = .false.
		print *, '    test1_25_globalVariable5 = ', test1_25_globalVariable5,
     & ', not ', -1 * test1_25_globalVariable4
	endif

	if (test1_25_globalVariable7 .ne. -1 * test1_25_globalVariable6) then
		if (passed .eqv. .true.) then
			print *, '**Failed** test #25 (unary operators)'
		endif
		passed = .false.
		print *, '    test1_25_globalVariable7 = ', test1_25_globalVariable7,
     & ', not ', -1 * test1_25_globalVariable6
	endif

	if (passed .eqv. .true.) then
		print *, 'Passed test #25 (unary operators)'
		passedTest = 1
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_25_call1
#include "test1_25F_common.h"
	integer place

	place = 23
	END

!------------------------------------------------------------------------------
