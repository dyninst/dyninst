/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
	IMPLICIT NONE

	INTEGER test1_6_globalVariable1, test1_6_globalVariable2, test1_6_globalVariable3,
     & test1_6_globalVariable4, test1_6_globalVariable5,
     & test1_6_globalVariable6, test1_6_globalVariable1a,
     & test1_6_globalVariable2a,
     & test1_6_globalVariable3a, test1_6_globalVariable4a,
     & test1_6_globalVariable5a, test1_6_globalVariable6a,
     & test1_6_constVar0,
     & test1_6_constVar1, test1_6_constVar2, test1_6_constVar3,
     & test1_6_constVar4, test1_6_constVar5,
     & test1_6_constVar6, test1_6_constVar7, test1_6_constVar9,
     & test1_6_constVar10, test1_6_constVar60,
     & test1_6_constVar64, test1_6_constVar66, test1_6_constVar67,
     & passedTest

! **********************************************************************
! The following common block (globals) has a corresponding c structure 
! (struct block_) defined in test1.mutateeFortC.c.  Make sure all changes
! to this structure are reflected in the other. (Including the size of
! each memeber defined above)
! **********************************************************************
	common /test1_6f_globals/ test1_6_globalVariable1, test1_6_globalVariable2,
     & test1_6_globalVariable3, test1_6_globalVariable4,
     & test1_6_globalVariable5,
     & test1_6_globalVariable6, test1_6_globalVariable1a,
     & test1_6_globalVariable2a,
     & test1_6_globalVariable3a, test1_6_globalVariable4a,
     & test1_6_globalVariable5a,
     & test1_6_globalVariable6a, test1_6_constVar0, test1_6_constVar1,
     & test1_6_constVar2,
     & test1_6_constVar3,
     & test1_6_constVar4, test1_6_constVar5, test1_6_constVar6,
     & test1_6_constVar7, test1_6_constVar9,
     & test1_6_constVar10, test1_6_constVar60, test1_6_constVar64,
     & test1_6_constVar66, test1_6_constVar67,
     & passedTest

