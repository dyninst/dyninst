	IMPLICIT NONE

	INTEGER test1_16_globalVariable16_1, test1_16_globalVariable16_2,
     & test1_16_globalVariable16_3,
     & test1_16_globalVariable16_4, test1_16_globalVariable16_5,
     & test1_16_globalVariable16_6,
     & test1_16_globalVariable16_7, test1_16_globalVariable16_8,
     & test1_16_globalVariable16_9,
     & test1_16_globalVariable16_10, passedTest

! **********************************************************************
! The following common block (globals) has a corresponding c structure 
! (struct block_) defined in test1.mutateeFortC.c.  Make sure all changes
! to this structure are reflected in the other. (Including the size of
! each memeber defined above)
! **********************************************************************
	common /test1_16f_globals/ test1_16_globalVariable16_1,
     & test1_16_globalVariable16_2, test1_16_globalVariable16_3,
     & test1_16_globalVariable16_4,
     & test1_16_globalVariable16_5, test1_16_globalVariable16_6,
     & test1_16_globalVariable16_7,
     & test1_16_globalVariable16_8, test1_16_globalVariable16_9,
     & test1_16_globalVariable16_10,
     & passedTest

