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
