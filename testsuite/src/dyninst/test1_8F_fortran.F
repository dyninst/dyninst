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

	SUBROUTINE test1_8f_init_globals
#include "test1_8F_common.h"

	test1_8_globalVariable1 = 1
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_8_func1 (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)
#include "test1_8F_common.h"
	INTEGER p1, p2, p3, p4, p5, p6, p7, p8, p9, p10

	if (p1 .eq. 1 .and. p2 .eq. 2 .and. p3 .eq. 3 .and. p4 .eq. 4
     & .and. p5 .eq. 5 .and. p6 .eq. 6 .and. p7 .eq. 7 .and. p8 .eq. 8
     & .and. p9 .eq. 9 .and. p10 .eq. 10) then
		passedTest = 1
	else
		print *, '**Failed** test #8 (preserve registers - expr)'
		if (p1 .ne. 1) then
			print *, '    parameter #1 is ', p1, ' not 1'
		endif
		if (p2 .ne. 2) then
			print *, '    parameter #2 is ', p2, ' not 2'
		endif
		if (p3 .ne. 3) then
			print *, '    parameter #3 is ', p3, ' not 3'
		endif
		if (p4 .ne. 4) then
			print *, '    parameter #4 is ', p4, ' not 4'
		endif
		if (p5 .ne. 5) then
			print *, '    parameter #5 is ', p5, ' not 5'
		endif
		if (p6 .ne. 6) then
			print *, '    parameter #6 is ', p6, ' not 6'
		endif
		if (p7 .ne. 7) then
			print *, '    parameter #7 is ', p7, ' not 7'
		endif
		if (p8 .ne. 8) then
			print *, '    parameter #8 is ', p8, ' not 8'
		endif
		if (p9 .ne. 9) then
			print *, '    parameter #9 is ', p9, ' not 9'
		endif
		if (p10 .ne. 10) then
			print *, '    parameter #10 is ', p10, ' not 10'
		endif
	endif
	END

!------------------------------------------------------------------------------
