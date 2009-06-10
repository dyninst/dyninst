	IMPLICIT NONE

	INTEGER globalVariable20_1, passedTest

	DOUBLE PRECISION globalVariable20_2

! **********************************************************************
! The following common block (globals) has a corresponding c structure 
! (struct block_) defined in test1.mutateeFortC.c.  Make sure all changes
! to this structure are reflected in the other. (Including the size of
! each memeber defined above)
! **********************************************************************
	common /test1_20f_globals/ globalVariable20_2, globalVariable20_1,
     & passedTest

