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

