	IMPLICIT NONE

	INTEGER globalVariable11_1,
     & globalVariable11_2, globalVariable11_3, globalVariable11_4,
     & globalVariable11_5, passedTest

! **********************************************************************
! The following common block (globals) has a corresponding c structure 
! (struct block_) defined in test1.mutateeFortC.c.  Make sure all changes
! to this structure are reflected in the other. (Including the size of
! each memeber defined above)
! **********************************************************************
	common /test1_11f_globals/ globalVariable11_1, globalVariable11_2,
     & globalVariable11_3, globalVariable11_4, globalVariable11_5,
     & passedTest

