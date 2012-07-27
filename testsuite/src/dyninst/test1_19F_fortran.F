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

	SUBROUTINE test1_19f_init_globals
#include "test1_19F_common.h"

	globalVariable19_1 = Z'deadbeef'
	globalVariable19_2 = z'deadbeef'

	END

!------------------------------------------------------------------------------

	SUBROUTINE func19_1
#include "test1_19F_common.h"

	CALL stop_process_

	if (globalVariable19_1 .ne. 1900100) then
		print *, '**Failed test #19 (oneTimeCode)'
		print *, '     globalVariable19_1 contained ',
     & globalVariable19_1, ', not 1900100 as expected'
	endif

	CALL stop_process_

	if (globalVariable19_2 .eq. 1900200) then
		passedTest = 1
	else
		print *, '**Failed test #19 (oneTimeCode)'
		print *, '     globalVariable19_2 contained ',
     & globalVariable19_2, ', not 1900200 as expected'
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_19_call1
#include "test1_19F_common.h"

	globalVariable19_1 = 1900100
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_19_call2
#include "test1_19F_common.h"

	globalVariable19_2 = 1900200
	END

!------------------------------------------------------------------------------
