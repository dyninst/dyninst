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

	SUBROUTINE test1_10f_init_globals
#include "test1_10F_common.h"

	globalVariable10_1 = 0
	globalVariable10_2 = 0
	globalVariable10_3 = 0
	globalVariable10_4 = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_10_func1
#include "test1_10F_common.h"

	if (globalVariable10_1 .eq. 1 .and. globalVariable10_2 .eq. 1
     & .and. globalVariable10_3 .eq. 1 .and. globalVariable10_4 .eq.
     & 3) then
		print *, 'Passed test #10 (insert snippet order)'
		passedTest = 1
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

	SUBROUTINE test1_10_call1
#include "test1_10F_common.h"

	if (globalVariable10_4 .eq. 0) then
		globalVariable10_4 = 1
		globalVariable10_1 = 1
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_10_call2
#include "test1_10F_common.h"

	if (globalVariable10_4 .eq. 1) then
		globalVariable10_4 = 2
		globalVariable10_2 = 1
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_10_call3
#include "test1_10F_common.h"

	if (globalVariable10_4 .eq. 2) then
		globalVariable10_4 = 3
		globalVariable10_3 = 1
	endif
	END

!------------------------------------------------------------------------------
