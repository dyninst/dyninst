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

	SUBROUTINE test1_9_func1 (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)
#include "test1_9F_common.h"
	INTEGER p1, p2, p3, p4, p5, p6, p7, p8, p9, p10

	if (p1 .eq. 1 .and. p2 .eq. 2 .and. p3 .eq. 3 .and. p4 .eq. 4
     & .and. p5 .eq. 5 .and. p6 .eq. 6 .and. p7 .eq. 7 .and. p8
     & .eq. 8 .and. p9 .eq. 9 .and. p10 .eq. 10) then
		print *, 'Passed test #9 (preserve registers - funcCall)'
		passedTest = 1
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

	SUBROUTINE test1_9_call1 (p1, p2, p3, p4, p5)
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
