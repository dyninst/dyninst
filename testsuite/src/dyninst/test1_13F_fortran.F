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

	SUBROUTINE test1_13f_init_globals
#include "test1_13F_common.h"

	globalVariable13_1 = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_13_func1 (p1, p2, p3, p4, p5)
#include "test1_13F_common.h"
#ifdef __GNUC__
	INTRINSIC iand
#else
	INTRINSIC and
#endif
	INTEGER p1, p2, p3, p4, p5, temp, test1_13_func2

	temp = test1_13_func2 ()

	if (p1 .eq. 131 .and. p2 .eq. 132 .and. p3 .eq. 133 .and. p4 .eq.
     & 134 .and. p5 .eq. 135 .and. globalVariable13_1 .eq. 63) then
		passedTest = 1
	else
		print *, '**Failed test#13 (paramExpr,retExpr,nullExpr)'
		print *, 'globalVariable13_1 = ', globalVariable13_1
		if (p1 .ne. 131) then
			print *, '  parameter 1 is ', p1, ', not 131'
		endif
		if (p2 .ne. 132) then
			print *, '  parameter 2 is ', p2, ', not 132'
		endif
		if (p3 .ne. 133) then
			print *, '  parameter 3 is ', p3, ', not 133'
		endif
		if (p4 .ne. 134) then
			print *, '  parameter 4 is ', p4, ', not 134'
		endif
		if (p5 .ne. 135) then
			print *, '  parameter 5 is ', p5, ', not 135'
		endif
		if (AND (globalVariable13_1, 1) .eq. 0) then
			print *, '    passed param a1 wrong'
		endif
		if (AND (globalVariable13_1, 2) .eq. 0) then
			print *, '    passed param a2 wrong'
		endif
		if (AND (globalVariable13_1, 4) .eq. 0) then
			print *, '    passed param a3 wrong'
		endif
		if (AND (globalVariable13_1, 8) .eq. 0) then
			print *, '    passed param a4 wrong'
		endif
		if (AND (globalVariable13_1, 16) .eq. 0) then
			print *, '    passed param a5 wrong'
		endif
		if (AND (globalVariable13_1, 32) .eq. 0) then
			print *, '    return value wrong'
		endif
	endif
	END

!------------------------------------------------------------------------------

	INTEGER FUNCTION test1_13_func2 ()
	test1_13_func2 = 1300100
	RETURN
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_13_call1 (a1, a2, a3, a4, a5)
#include "test1_13F_common.h"
#ifdef __GNUC__
	INTRINSIC ior
#else
	INTRINSIC or
#endif
	INTEGER a1, a2, a3, a4, a5

	if (a1 .eq. 131) then
		globalVariable13_1 = OR (globalVariable13_1, 1)
	endif
	if (a2 .eq. 132) then
		globalVariable13_1 = OR (globalVariable13_1, 2)
	endif
	if (a3 .eq. 133) then
		globalVariable13_1 = OR (globalVariable13_1, 4)
	endif
	if (a4 .eq. 134) then
		globalVariable13_1 = OR (globalVariable13_1, 8)
	endif
	if (a5 .eq. 135) then
		globalVariable13_1 = OR (globalVariable13_1, 16)
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_13_call2 (ret)
#include "test1_13F_common.h"
#ifdef __GNUC__
	INTRINSIC ior
#else
	INTRINSIC or
#endif
	INTEGER ret

	if (ret .eq. 1300100) then
		globalVariable13_1 = OR (globalVariable13_1, 32)
	endif
	END

!------------------------------------------------------------------------------
