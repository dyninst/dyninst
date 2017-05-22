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

	SUBROUTINE test1_31f_init_globals
#include "test1_31F_common.h"

	globalVariable31_1 = 0
	globalVariable31_2 = 0
	globalVariable31_3 = 0
	globalVariable31_4 = 0
	END

!------------------------------------------------------------------------------

	INTEGER FUNCTION test1_31_func1 ()
#include "test1_31F_common.h"

	globalVariable31_1 = 0
	globalVariable31_2 = 0
	globalVariable31_3 = 0
	globalVariable31_4 = 0

	CALL test1_31_func2

	if (globalVariable31_3 .eq. 1) then
		passedTest = 1
	endif

	if (passedTest .eq. 0) then
		print *, '**Failed** test #31 (non-recursive base tramp guard)'
		print *, '    globalVariable31_3 = ', globalVariable31_3,
     & ', should be 1 (no instrumentation got exectued?)'
		test1_31_func1 = 0
		RETURN
	endif

	if (globalVariable31_4 .eq. 0) then
		passedTest = 1
	endif

	if (passedTest .eq. 0) then
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
		test1_31_func1 = 0
		RETURN
	endif

	passedTest = 1

	test1_31_func1 = 1
	RETURN
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_31_func2
#include "test1_31F_common.h"

	globalVariable31_2 = 1
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_31_func3
#include "test1_31F_common.h"

	globalVariable31_3 = 1
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_31_func4 (value)
#include "test1_31F_common.h"
	INTEGER value

	if (value .eq. 0) then
		print *, 'test1_31_func4 called with calue = 0 !'
	endif
	globalVariable31_4 = globalVariable31_4 + value
	END

!------------------------------------------------------------------------------
