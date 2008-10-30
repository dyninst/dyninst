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

	SUBROUTINE test1_33_func1
#include "test1_33F_common.h"

	passedTest = 1
	print *, 'Passed test #33 (control flow graphs)'
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_33_func2 (x)
#include "test1_33F_common.h"
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

	INTEGER FUNCTION test1_33_func3 (x)
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

	test1_33_func3 = x
	RETURN
	END

!------------------------------------------------------------------------------
