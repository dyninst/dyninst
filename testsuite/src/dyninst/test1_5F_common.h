	IMPLICIT NONE

	INTEGER test1_5_globalVariable5_1, test1_5_globalVariable5_2,
     & passedTest

! **********************************************************************
! The following common block (globals) has a corresponding c structure 
! (struct block_) defined in test1.mutateeFortC.c.  Make sure all changes
! to this structure are reflected in the other. (Including the size of
! each memeber defined above)
! **********************************************************************
	common /test1_5f_globals/ test1_5_globalVariable5_1,
     & test1_5_globalVariable5_2, passedTest

