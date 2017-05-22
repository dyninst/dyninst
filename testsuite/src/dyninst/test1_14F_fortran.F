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

	SUBROUTINE test1_14f_init_globals
#include "test1_14F_common.h"

	globalVariable14_1 = 0
	globalVariable14_2 = 0
	globalVariable14_3 = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_14_func1
#include "test1_14F_common.h"
	INTEGER kludge

	kludge = 1
	CALL test1_14_func2
	CALL test1_14_func3
	CALL test1_14_func4(globalVariable14_3)

	if (globalVariable14_1 .eq. 1 .and. globalVariable14_2
     & .eq. 0 .and. globalVariable14_3 .eq. 2) then
		passedTest = 1
	else
		print *, '**Failed test #14 (replace/remove function call)'
		if (globalVariable14_1 .ne. 1) then
			print *, '     call to test1_14_func2() was not replaced'
		endif
		if (globalVariable14_2 .ne. 0) then
			print *, '     call to test1_14_func3() was not removed'
		endif
		if (globalVariable14_3 .ne. 2) then
			print *, '     call to test1_14_func4() was not replaced'
		endif
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_14_func2
#include "test1_14F_common.h"

	globalVariable14_1 = 2
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_14_func3
#include "test1_14F_common.h"

	globalVariable14_2 = 1
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_14_call1
#include "test1_14F_common.h"

	globalVariable14_1 = 1
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_14_func4 (p1)
#include "test1_14F_common.h"
	INTEGER p1

	p1 = 3;
	END

!------------------------------------------------------------------------------
