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

	SUBROUTINE test1_26f_init_globals
#include "test1_26F_common.h"

	test1_26_globalVariable2 = 26000000
	test1_26_globalVariable3 = 26000000
	test1_26_globalVariable4 = 26000000
	test1_26_globalVariable5 = 26000000
	test1_26_globalVariable6 = 26000000
	test1_26_globalVariable7 = 26000000
	test1_26_globalVariable8 = 26000000
	test1_26_globalVariable9 = 26000000
	test1_26_globalVariable10 = 26000000
	test1_26_globalVariable11 = 26000000
	test1_26_globalVariable12 = 26000000
	test1_26_globalVariable13 = 26000000

	END

!------------------------------------------------------------------------------

	SUBROUTINE func26_1
#include "test1_26F_common.h"

	print *, 'Skipped test #26 (field operators)'
	print *, '       - not implemented in Fortran'
	passedTest = 1
! What does this next line mean?  Is like #if 0?
#if notdef
	INTEGER i

        TYPE (struct26_2) test1_26_globalVariable1

	passedTest = 
	test1_26_globalVariable1%field1 = 26001001
	test1_26_globalVariable1%field2 = 26001002

	DO i = 1, 10, 1
	test1_26_globalVariable1%field3 (i) = 26001002 + i
	ENDDO

	test1_26_globalVariable1%field4%field1 = 26000013;
	test1_26_globalVariable1%field4%field2 = 26000014;

	CALL test1_26_call1

	CALL FverifyScalarValue ("test1_26_globalVariable2",
     & test1_26_globalVariable2, 26001001, 26, "field operators")
	CALL FverifyScalarValue ("test1_26_globalVariable3",
     & test1_26_globalVariable3, 26001002, 26, "field operators")
	CALL FverifyScalarValue ("test1_26_globalVariable4",
     & test1_26_globalVariable4, 26001003, 26, "field operators")
	CALL FverifyScalarValue ("test1_26_globalVariable5",
     & test1_26_globalVariable5, 26001003+5, 26, "field operators")
	CALL FverifyScalarValue ("test1_26_globalVariable6",
     & test1_26_globalVariable6, 26000013, 26, "field operators")
	CALL FverifyScalarValue ("test1_26_globalVariable7",
     & test1_26_globalVariable7, 26000014, 26, "field operators")

	CALL FverifyScalarValue ("test1_26_globalVariable8",
     & test1_26_globalVariable8, 26002001, 26, "field operators")
	CALL FverifyScalarValue ("test1_26_globalVariable9",
     & test1_26_globalVariable9, 26002002, 26, "field operators")
	CALL FverifyScalarValue ("test1_26_globalVariable10",
     & test1_26_globalVariable10, 26002003, 26, "field operators")
	CALL FverifyScalarValue ("test1_26_globalVariable11",
     & test1_26_globalVariable11, 26002003+5, 26, "field operators")
	CALL FverifyScalarValue ("test1_26_globalVariable12",
     & test1_26_globalVariable12, 26002013, 26, "field operators")
	CALL FverifyScalarValue ("test1_26_globalVariable13",
     & test1_26_globalVariable13, 26002014, 26, "field operators")

	if (passedTest .eq. 1) then
	endif
#else
#endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_26_call1
#include "test1_26F_common.h"

	INTEGER i

#if !defined (F77)
	TYPE (struct26_2) localVariable26_1

	localVariable26_1%field1 = 26002001
	localVariable26_1%field2 = 26002002

	DO i = 1, 10, 1
	localVariable26_1%field3 (i) = 26002002 + i
	ENDDO

	localVariable26_1%field4%field1 = 26002013
	localVariable26_1%field4%field2 = 26002014

	CALL test1_26_call2
#endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_26_call2
	INTEGER foo = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE FverifyScalarValue (name, a, value, testNum, testName)
#include "test1_26F_common.h"
	CHARACTER *(*) name, testName
	INTEGER a, value, testNum

	if (a .ne. value) then
		if (passedTest .eq. 1) then
			print *, '**Failed** test ', testNum, ' (', testName, ')'
		endif
		print *, '  ', name, ' = ', a, ', not ', value
		passedTest = 0
	endif
	END

!------------------------------------------------------------------------------
