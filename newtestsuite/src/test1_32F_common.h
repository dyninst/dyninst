	IMPLICIT NONE

	INTEGER globalVariable32_1, globalVariable32_2,
     & globalVariable32_3, globalVariable32_4, passedTest

! **********************************************************************
! The following common block (globals) has a corresponding c structure 
! (struct block_) defined in test1.mutateeFortC.c.  Make sure all changes
! to this structure are reflected in the other. (Including the size of
! each memeber defined above)
! **********************************************************************
	common /test1_32f_globals/ globalVariable32_1, globalVariable32_2, globalVariable32_3,
     & globalVariable32_4, passedTest

