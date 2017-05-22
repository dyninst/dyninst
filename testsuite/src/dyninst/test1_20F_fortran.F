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

	SUBROUTINE test1_20f_init_globals
#include "test1_20F_common.h"

	globalVariable20_1 = Z'deadbeef'
	globalVariable20_2 = 0.0
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_20_call1
#include "test1_20F_common.h"
	INTEGER ta
	DOUBLE PRECISION tb

	ta = 3
	tb = 4.3

	globalVariable20_1 = ta + ta + ta + ta + ta + ta + ta + ta + ta +
     & ta + ta + ta + ta + ta + ta + ta + ta + ta + ta + ta + ta +
     & ta + ta + ta + ta + ta + ta + ta + ta + ta + ta + ta + ta +
     & ta + ta + ta + ta + ta + ta + ta + ta
	globalVariable20_2 = tb + tb + tb + tb + tb + tb + tb + tb + tb +
     & tb + tb + tb + tb + tb + tb + tb + tb + tb + tb + tb + tb +
     & tb + tb + tb + tb + tb + tb + tb + tb + tb + tb + tb + tb +
     & tb + tb + tb + tb + tb + tb + tb + tb
	END

!------------------------------------------------------------------------------

	INTEGER FUNCTION test1_20_func2 (int_val, double_val)
	INTEGER int_val
	DOUBLE PRECISION double_val
	INTEGER tc, i, ret
	DOUBLE PRECISION td

	tc = 7
	td = 6.4
	ret = 1

	int_val = tc + tc + tc + tc + tc + tc + tc + tc + tc + tc + tc +
     & tc + tc + tc + tc + tc + tc + tc + tc + tc + tc + tc + tc +
     & tc + tc + tc + tc + tc + tc + tc + tc + tc + tc + tc + tc +
     & tc + tc + tc + tc + tc + tc
	double_val = td + td + td + td + td + td + td + td + td + td +
     & td + td + td + td + td + td + td + td + td + td + td + td +
     & td + td + td + td + td + td + td + td + td + td + td + td +
     & td + td + td + td + td + td + td

	DO i = 0, 49, 1
		ret = ret * 3
		if (mod (i, 2) .eq. 1) then
			ret = ret * 5
		elseif (i .lt. 10) then
			ret = ret * 7
		elseif (i .gt. 20) then
			ret = ret * 11
		endif
	ENDDO

	test1_20_func2 = ret
	RETURN
	END

!------------------------------------------------------------------------------

	SUBROUTINE func20_1
#include "test1_20F_common.h"
	INTEGER ret, int_val, test1_20_func2, eq_doubles
	DOUBLE PRECISION double_val, temp1, temp2

	ret = test1_20_func2 (int_val, double_val)

	temp1 = 4.3 * 41
	temp2 = 6.4 * 41

	if (globalVariable20_1 .eq. 3 * 41 .and. eq_doubles
     & (globalVariable20_2, temp1) .eq. 1 .and. int_val .eq.
     & 7 * 41 .and. eq_doubles (double_val, temp2) .eq. 1
     & .and. ret .eq. 1088896211) then
		passedTest = 1
	else
		print *, '**Failed test #20 (instrument arbitrary points)'
		if (globalVariable20_1 .ne. (3 * 41)) then
			print *, '    globalVariable20_1 contained ',
     & globalVariable20_1, ', not ', 3 * 41, ' as expected'
		endif
		if (eq_doubles (globalVariable20_2, temp1) .ne. 1) then
			print *, '    globalVariable20_2 contained ',
     & globalVariable20_2, ', not ', temp1, ' as expected'
		endif
		if (int_val .ne. (7 * 41)) then
			print *, '    int_val contained ', int_val, ', not ',
     & (7 * 41), ' as expected'
		endif
		if (eq_doubles (double_val, temp2) .ne. 1) then
			print *, '    double_val contained', double_val, ', not ',
     & temp2, ' as expected'
		endif
		if (ret .ne. 1088896211) then
			print *, '    ret contained ', ret, ', not 1088896211 as',
     & ' expected'
		endif
	endif
	END

!------------------------------------------------------------------------------

	INTEGER FUNCTION eq_doubles (a, b)
	DOUBLE PRECISION a, b, diff

	diff = a - b

	if (diff .lt. 0) then
		diff = -1 * diff
	endif

	if (diff .lt. 0.01) then
		eq_doubles = 1
	else
		eq_doubles = 0
	endif
	RETURN
	END

!------------------------------------------------------------------------------
