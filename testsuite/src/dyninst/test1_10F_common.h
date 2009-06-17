	IMPLICIT NONE

	INTEGER globalVariable10_1, globalVariable10_2,
     & globalVariable10_3, globalVariable10_4, passedTest

! **********************************************************************
! The following common block (globals) has a corresponding c structure 
! (struct block_) defined in test1.mutateeFortC.c.  Make sure all changes
! to this structure are reflected in the other. (Including the size of
! each memeber defined above)
! **********************************************************************
	common /test1_10f_globals/ globalVariable10_1, globalVariable10_2,
     & globalVariable10_3,
     & globalVariable10_4, passedTest

