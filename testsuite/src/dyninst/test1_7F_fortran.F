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

	SUBROUTINE test1_7f_init_globals
#include "test1_7F_common.h"

	test1_7_constVar0 = 0
	test1_7_constVar1 = 1
	test1_7_constVar2 = 2
	test1_7_constVar3 = 3
	test1_7_constVar4 = 4
	test1_7_constVar5 = 5
	test1_7_constVar6 = 6
	test1_7_constVar7 = 7
	test1_7_constVar9 = 9

	test1_7_globalVariable1 = 71
	test1_7_globalVariable2 = 71
	test1_7_globalVariable3 = 71
	test1_7_globalVariable4 = 71
	test1_7_globalVariable5 = 71
	test1_7_globalVariable6 = 71
	test1_7_globalVariable7 = 71
	test1_7_globalVariable8 = 71
	test1_7_globalVariable9 = 71
	test1_7_globalVariable10 = 71
	test1_7_globalVariable11 = 71
	test1_7_globalVariable12 = 71
	test1_7_globalVariable13 = 71
	test1_7_globalVariable14 = 71
	test1_7_globalVariable15 = 71
	test1_7_globalVariable16 = 71

	test1_7_globalVariable1a = 73
	test1_7_globalVariable2a = 73
	test1_7_globalVariable3a = 73
	test1_7_globalVariable4a = 73
	test1_7_globalVariable5a = 73
	test1_7_globalVariable6a = 73
	test1_7_globalVariable7a = 73
	test1_7_globalVariable8a = 73
	test1_7_globalVariable9a = 73
	test1_7_globalVariable10a = 73
	test1_7_globalVariable11a = 73
	test1_7_globalVariable12a = 73
	test1_7_globalVariable13a = 73
	test1_7_globalVariable14a = 73
	test1_7_globalVariable15a = 73
	test1_7_globalVariable16a = 73

	END

!------------------------------------------------------------------------------

	SUBROUTINE fail7Print (tCase, fCase, op)
	INTEGER tCase
	INTEGER fCase
	CHARACTER op *(*)

	if (tCase .ne. 72) then
		print *, ' operator ', op, ' was not true when it should be',
     & ' - const expr'
	endif
	if (fCase .ne. 71) then
		print *, ' operator ', op, ' was not false when it should be',
     & ' - const expr'
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE fail7aPrint (tCase, fCase, op)
	INTEGER tCase
	INTEGER fCase
	CHARACTER op *(*)

	if (tCase .ne. 74) then
		print *, ' operator ', op, ' was not true when it should be',
     & ' - var expr'
	endif
	if (fCase .ne. 73) then
		print *, ' operator ', op, ' was not false when it should be',
     & ' - var expr'
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_7_func1
#include "test1_7F_common.h"

	CALL test1_7_func2

	if (test1_7_globalVariable1 .eq. 72 .and.
     & test1_7_globalVariable2 .eq. 71 .and.
     & test1_7_globalVariable3 .eq. 72 .and.
     & test1_7_globalVariable4 .eq. 71 .and.
     & test1_7_globalVariable5 .eq. 72 .and.
     & test1_7_globalVariable6 .eq. 71 .and.
     & test1_7_globalVariable7 .eq. 72 .and.
     & test1_7_globalVariable8 .eq. 71 .and.
     & test1_7_globalVariable9 .eq. 72 .and.
     & test1_7_globalVariable10 .eq. 71 .and.
     & test1_7_globalVariable11 .eq. 72 .and.
     & test1_7_globalVariable12 .eq. 71 .and.
     & test1_7_globalVariable13 .eq. 72 .and.
     & test1_7_globalVariable14 .eq. 71 .and.
     & test1_7_globalVariable15 .eq. 72 .and.
     & test1_7_globalVariable16 .eq. 71 .and.
     & test1_7_globalVariable1a .eq. 74 .and.
     & test1_7_globalVariable2a .eq. 73 .and.
     & test1_7_globalVariable3a .eq. 74 .and.
     & test1_7_globalVariable4a .eq. 73 .and.
     & test1_7_globalVariable5a .eq. 74 .and.
     & test1_7_globalVariable6a .eq. 73 .and.
     & test1_7_globalVariable7a .eq. 74 .and.
     & test1_7_globalVariable8a .eq. 73 .and.
     & test1_7_globalVariable9a .eq. 74 .and.
     & test1_7_globalVariable10a .eq. 73 .and.
     & test1_7_globalVariable11a .eq. 74 .and.
     & test1_7_globalVariable12a .eq. 73 .and.
     & test1_7_globalVariable13a .eq. 74 .and.
     & test1_7_globalVariable14a .eq. 73 .and.
     & test1_7_globalVariable15a .eq. 74 .and.
     & test1_7_globalVariable16a .eq. 73) then
		passedTest = 1
	else
		print *, '**Failed** test #7 (relational operators)'
		CALL fail7Print(test1_7_globalVariable1, test1_7_globalVariable2,
     & "BPatch_lt")
		CALL fail7Print(test1_7_globalVariable3, test1_7_globalVariable4,
     & "BPatch_eq")
		CALL fail7Print(test1_7_globalVariable5, test1_7_globalVariable6,
     & "BPatch_gt")
		CALL fail7Print(test1_7_globalVariable7, test1_7_globalVariable8,
     & "BPatch_le")
		CALL fail7Print(test1_7_globalVariable9, test1_7_globalVariable10,
     & "BPatch_ne")
		CALL fail7Print(test1_7_globalVariable11, test1_7_globalVariable12,
     & "BPatch_ge")
		CALL fail7Print(test1_7_globalVariable13, test1_7_globalVariable14,
     & "BPatch_and")
		CALL fail7Print(test1_7_globalVariable15, test1_7_globalVariable16,
     & "BPatch_or")

		CALL fail7aPrint(test1_7_globalVariable1a, test1_7_globalVariable2a,
     & "BPatch_lt")
		CALL fail7aPrint(test1_7_globalVariable3a, test1_7_globalVariable4a,
     & "BPatch_eq")
		CALL fail7aPrint(test1_7_globalVariable5a, test1_7_globalVariable6a,
     & "BPatch_gt")
		CALL fail7aPrint(test1_7_globalVariable7a, test1_7_globalVariable8a,
     & "BPatch_le")
		CALL fail7aPrint(test1_7_globalVariable9a, test1_7_globalVariable10a,
     & "BPatch_ne")
		CALL fail7aPrint(test1_7_globalVariable11a, test1_7_globalVariable12a,
     & "BPatch_ge")
		CALL fail7aPrint(test1_7_globalVariable13a, test1_7_globalVariable14a,
     & "BPatch_and")
		CALL fail7aPrint(test1_7_globalVariable15a, test1_7_globalVariable16a,
     & "BPatch_or")
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_7_func2
#include "test1_7F_common.h"
	END

!------------------------------------------------------------------------------
