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

	SUBROUTINE test1_6f_init_globals
#include "test1_6F_common.h"

	test1_6_globalVariable1 = Z'deadbeef'
	test1_6_globalVariable2 = Z'deadbeef'
	test1_6_globalVariable3 = Z'deadbeef'
	test1_6_globalVariable4 = Z'deadbeef'
	test1_6_globalVariable5 = Z'deadbeef'
	test1_6_globalVariable6 = Z'deadbeef'
	test1_6_globalVariable1a = Z'deadbeef'
	test1_6_globalVariable2a = Z'deadbeef'
	test1_6_globalVariable3a = Z'deadbeef'
	test1_6_globalVariable4a = Z'deadbeef'
	test1_6_globalVariable5a = Z'deadbeef'
	test1_6_globalVariable6a = Z'deadbeef'

	test1_6_constVar0 = 0
	test1_6_constVar1 = 1
	test1_6_constVar2 = 2
	test1_6_constVar3 = 3
	test1_6_constVar4 = 4
	test1_6_constVar5 = 5
	test1_6_constVar6 = 6
	test1_6_constVar7 = 7
	test1_6_constVar9 = 9
	test1_6_constVar10 = 10
	test1_6_constVar60 = 60
	test1_6_constVar64 = 64
	test1_6_constVar66 = 66
	test1_6_constVar67 = 67

	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_6_func1
#include "test1_6F_common.h"

	CALL test1_6_func2

	if (test1_6_globalVariable1 .eq. 60 + 2 .and.
     & test1_6_globalVariable2 .eq. (64 - 1) .and.
     & test1_6_globalVariable3 .eq. (66 / 3) .and.
     & test1_6_globalVariable4 .eq. (67 / 3) .and.
     & test1_6_globalVariable5 .eq. (6 * 5) .and.
     & test1_6_globalVariable6 .eq. 3 .and.
     & test1_6_globalVariable1a .eq. (60 + 2) .and.
     & test1_6_globalVariable2a .eq. (64 - 1) .and.
     & test1_6_globalVariable3a .eq. (66 / 3) .and.
     & test1_6_globalVariable4a .eq. (67 / 3) .and.
     & test1_6_globalVariable5a .eq. (6 * 5) .and.
     & test1_6_globalVariable6a .eq. 3) then
		passedTest = 1
	else
		print *, '**Failed** test #6 (arithmetic operators)'

		if (test1_6_globalVariable1 .ne. (60 + 2)) then
			print *, '     addition error 60+2 got ', test1_6_globalVariable1
		endif
		if (test1_6_globalVariable2 .ne. (64 - 1)) then
			print *, '     subtraction error 64-1 got ', test1_6_globalVariable2
		endif
		if (test1_6_globalVariable3 .ne. (66 / 3)) then
			print *, '     division error 66/3 got ', test1_6_globalVariable3
		endif
		if (test1_6_globalVariable4 .ne. (67 / 3)) then
			print *, '     division error 67/3 got ', test1_6_globalVariable4
		endif
		if (test1_6_globalVariable5 .ne. (6 * 5)) then
			print *, '     mult error 6*5 got ', test1_6_globalVariable5
		endif
		if (test1_6_globalVariable6 .ne. 3) then
			print *, '     comma error 10,3 got ', test1_6_globalVariable6
		endif

		if (test1_6_globalVariable1a .ne. (60 + 2)) then
			print *, '     addition error 60+2 got ', test1_6_globalVariable1a
		endif
		if (test1_6_globalVariable2a .ne. (64 - 1)) then
			print *, '     subtraction error 64-1 got ', test1_6_globalVariable2a
		endif
		if (test1_6_globalVariable3a .ne. (66 / 3)) then
			print *, '     division error 66/3 got ', test1_6_globalVariable3a
		endif
		if (test1_6_globalVariable4a .ne. (67 / 3)) then
			print *, '     division error 67/3 got ', test1_6_globalVariable4a
		endif
		if (test1_6_globalVariable5a .ne. (6 * 5)) then
			print *, '     mult error 6*5 got ', test1_6_globalVariable5a
		endif
		if (test1_6_globalVariable6a .ne. 3) then
			print *, '     comma error 10,3 got ', test1_6_globalVariable6a
		endif
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_6_func2
#include "test1_6F_common.h"


	END

!------------------------------------------------------------------------------
