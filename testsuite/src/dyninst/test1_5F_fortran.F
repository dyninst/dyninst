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

	SUBROUTINE test1_5f_init_globals
#include "test1_5F_common.h"

	test1_5_globalVariable5_1 = 51
	test1_5_globalVariable5_2 = 51
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_5_func1
#include "test1_5F_common.h"

	CALL test1_5_func2

	if (test1_5_globalVariable5_1 .eq. 51 .and. test1_5_globalVariable5_2 .eq. 53)
     & then
			passedTest = 1
		else
			print *, '**Failed** test #5 (if w.o. else)'
			if (test1_5_globalVariable5_1 .ne. 51) then
				print *, '      condition executed for false'
				print *, '		globalVariable5_1 = ', test1_5_globalVariable5_1
			endif
			if (test1_5_globalVariable5_2 .ne. 53) then
				print *, '      condition not executed for true'
				print *, '		globalVariable5_2 = ', test1_5_globalVariable5_2
			endif
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_5_func2
#include "test1_5F_common.h"
	END

!------------------------------------------------------------------------------
