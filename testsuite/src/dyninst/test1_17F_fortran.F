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

	SUBROUTINE test1_17f_init_globals
#include "test1_17F_common.h"

	globalVariable17_1 = 0
	globalVariable17_2 = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_17_func1
#include "test1_17F_common.h"
	INTEGER ret17_1
	INTEGER test1_17_func2

	ret17_1 = test1_17_func2 ()
	CALL test1_17_func3

	if (ret17_1 .ne. 1700100 .or. globalVariable17_1 .ne. 1700200 .or.
     & globalVariable17_2 .ne. 1701000) then
		print *, '**Failed** test case #17 (return values from func',
     & ' calls)'
		if (ret17_1 .ne. 1700100) then
			print *, '  return value was ', ret17_1, ', not 1700100'
		endif
		if (globalVariable17_1 .ne. 1700200) then
			print *, '  return value was ', globalVariable17_1,
     & ', not 1700200'
		endif
		if (globalVariable17_2 .ne. 1701000) then
			print *, '  function test1_17_call2 was not inserted'
		endif
	else
		passedTest = 1
	endif
	END

!------------------------------------------------------------------------------

	INTEGER FUNCTION test1_17_func2 ()
	test1_17_func2 = 1700100
	RETURN
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_17_func3
#include "test1_17F_common.h"
	INTEGER test1_17_func4

	globalVariable17_1 = test1_17_func4()
	END

!------------------------------------------------------------------------------
	INTEGER FUNCTION test1_17_func4 ()
	test1_17_func4 = 1700200
	RETURN
	END

!------------------------------------------------------------------------------

	INTEGER FUNCTION test1_17_call1 (p1)
	INTEGER p1
	INTEGER a1, a2, a3, a4, a5, a6, a7

	a1 = p1
	a2 = a1 + p1
	a3 = a1 * a2
	a4 = a3 / p1
	a5 = a4 + p1
	a6 = a5 + a1
	a7 = a6 + p1
	test1_17_call1 = a7
	return
	END

!------------------------------------------------------------------------------

	INTEGER FUNCTION test1_17_call2 (p1)
#include "test1_17F_common.h"
	INTEGER p1
	INTEGER a1, a2, a3, a4, a5, a6, a7

	a1 = p1
	a2 = a1 + p1
	a3 = a1 * a2
	a4 = a3 / p1
	a5 = a4 + p1
	a6 = a5 + a1
	a7 = a6 + p1
	globalVariable17_2 = 1701000
	test1_17_call2 = a7
	return
	END

!------------------------------------------------------------------------------
