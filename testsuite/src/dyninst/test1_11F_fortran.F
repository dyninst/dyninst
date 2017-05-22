 !
 ! See the dyninst/COPYRIGHT file for copyright information.
 ! 
 ! We provide the Paradyn Tools (below described as "Paradyn")
 ! on an AS IS basis, and do not warrant its validity or performance.
 ! We reserve the right to update, modify, or discontinue this
 ! software at any time.  We shall have no obligation to supply such
 ! updates or modifications or any other form of support to you.
 ! 
 ! By your use of Paradyn, you understand and agree that we (or any
 ! other person or entity with proprietary rights in Paradyn) are
 ! under no obligation to provide either maintenance services,
 ! update services, notices of latent defects, or correction of
 ! defects for Paradyn.
 ! 
 ! This library is free software; you can redistribute it and/or
 ! modify it under the terms of the GNU Lesser General Public
 ! License as published by the Free Software Foundation; either
 ! version 2.1 of the License, or (at your option) any later version.
 ! 
 ! This library is distributed in the hope that it will be useful,
 ! but WITHOUT ANY WARRANTY; without even the implied warranty of
 ! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 ! Lesser General Public License for more details.
 ! 
 ! You should have received a copy of the GNU Lesser General Public
 ! License along with this library; if not, write to the Free Software
 ! Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
