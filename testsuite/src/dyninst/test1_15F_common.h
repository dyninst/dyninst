	IMPLICIT NONE

	INTEGER globalVariable15_1,
     & globalVariable15_2, globalVariable15_3, globalVariable15_4,
     & passedTest

! **********************************************************************
! The following common block (globals) has a corresponding c structure 
! (struct block_) defined in test1.mutateeFortC.c.  Make sure all changes
! to this structure are reflected in the other. (Including the size of
! each memeber defined above)
! **********************************************************************
	common /globals/ globalVariable15_1, globalVariable15_2,
     & globalVariable15_3, globalVariable15_4, passedTest

