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

	SUBROUTINE test1_11f_init_globals
#include "test1_11F_common.h"

	globalVariable11_1 = 0
	globalVariable11_2 = 0
	globalVariable11_3 = 0
	globalVariable11_4 = 0
	globalVariable11_5 = 0

	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_11_func1
#include "test1_11F_common.h"

	globalVariable11_1 = 1
	CALL test1_11_func2
	globalVariable11_1 = 3
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_11_func2
#include "test1_11F_common.h"

	globalVariable11_1 = 2
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_11_call1
#include "test1_11F_common.h"

	if (globalVariable11_1 .eq. 0) then
		globalVariable11_2 = 1
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_11_call2
#include "test1_11F_common.h"

	if (globalVariable11_1 .eq. 1) then
		globalVariable11_3 = 1
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_11_call3
#include "test1_11F_common.h"

	if (globalVariable11_1 .eq. 2) then
		globalVariable11_4 = 1
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_11_call4
#include "test1_11F_common.h"

	if (globalVariable11_1 .eq. 3) then
		globalVariable11_5 = 1
	endif
	if (globalVariable11_2 .ne. 0 .and. globalVariable11_3 .ne. 0
     & .and. globalVariable11_4 .ne. 0 .and. globalVariable11_5
     & .ne. 0) then
		print *, 'Passed test #11 (snippets at entry, exit, call)'
		passedTest = 1
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
