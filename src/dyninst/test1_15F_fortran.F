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

	SUBROUTINE init_globals
#include "test1_15F_common.h"

	globalVariable15_1 = 0
	globalVariable15_2 = 0
	globalVariable15_3 = 0
	globalVariable15_4 = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_15_func2
	INTEGER foo = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_15_func3
#include "test1_15F_common.h"

	globalVariable15_3 = 100
	globalVariable15_4 = globalVariable15_4 + 1
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_15_func4
	INTEGER kludge

	kludge = 1
	CALL test1_15_func3
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_15_call1
#include "test1_15F_common.h"

	globalVariable15_1 = globalVariable15_1 + 1
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_15_call2
#include "test1_15F_common.h"

	globalVariable15_2 = globalVariable15_2 + 1
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_15_call3
#include "test1_15F_common.h"

	globalVariable15_3 = globalVariable15_3 + 1
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_15_func1
#include "test1_15F_common.h"
	LOGICAL failed

	failed = .false.

	CALL test1_15_func2
	CALL check15result ('globalVariable15_1', globalVariable15_1, 1,
     & 'after first call to instrumented function', failed)

	CALL test1_15_func4
	CALL check15result ('globalVariable15_3', globalVariable15_3, 1,
     & 'after first call to instrumented function', failed)

	CALL stop_process_
	CALL test1_15_func2
	CALL check15result ('globalVariable15_1', globalVariable15_1, 1,
     & 'after second call to instrumented function', failed)

	CALL test1_15_func4
	CALL check15result ('globalVariable15_3', globalVariable15_3,
     & 100, 'after second call to instrumented function', failed)

	CALL stop_process_
	CALL test1_15_func2
	CALL check15result ('globalVariable15_1', globalVariable15_1, 2,
     & 'after third call to instrumented function', failed)

	CALL test1_15_func4
	CALL check15result ('globalVariable15_3', globalVariable15_3,
     & 101, 'after third call to instrumented function', failed)

	if (failed .eqv. .false.) then
		passedTest = 1
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE check15result (varname, value, expected, errstr,
     & failed)
	CHARACTER *(*) varname
	INTEGER value, expected
	CHARACTER *(*) errstr
	LOGICAL failed

	if (value .ne. expected) then
		if (failed .eqv. .false.) then
			print *, '**Failed test #15 (setMutationsActive)'
		endif
		failed = .true.
		print *, '    ', varname, ' = ', value, ' ', errstr
	endif
	END	

!------------------------------------------------------------------------------
